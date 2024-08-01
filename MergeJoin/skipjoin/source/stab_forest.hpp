/**
 *
 * Copyright (c) 2017 Jelle Hellings.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY JELLE HELLINGS ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO
 * EVENT SHALL THE CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */
#ifndef INCLUDE_STAB_FOREST_HPP
#define INCLUDE_STAB_FOREST_HPP

#include <algorithm>
#include <vector>
#include "algorithm.hpp"
#include "block_list.hpp"
#include "interval.hpp"
#include "raw_array.hpp"

/**
 * Stab-forward policy enabling that every stab-forward operation is performed
 * exclusively using the index.
 */
struct stab_forward_index
{
};

/**
 * Stab-forward policy enabling that every stab-forward operation is performed
 * exclusively using the event-list.
 */
struct stab_forward_list
{
};

/**
 * Stab-forward policy enabling that every stab-forward operation is performed
 * using either the index or the event-list, depending on a threshold factor
 * check.
 */
struct stab_forward_check
{
    template <class StabForest>
    stab_forward_check(const StabForest &forest, const std::size_t c) : threshold(c * std::max<std::size_t>(1u, forest.index_height())) {}

    stab_forward_check(const stab_forward_check &other) : threshold(other.threshold) {}

    const std::size_t threshold;
};

/**
 * Basic operations on event lists.
 */
template <class EventList>
class basic_event_list
{
protected:
    /* The list-like container holding the event-list. */
    using event_list_type = EventList;

public:
    using event = typename event_list_type::value_type;
    using timestamp = typename event::unsigned_type;
    using const_iterator = typename event_list_type::const_iterator;
    using size_type = std::size_t;

protected:
    /**
     * Default-constructor.
     */
    basic_event_list() : event_list() {}

public:
    /**
     * Return iterator to the begin of the event-list.
     */
    const_iterator begin() const
    {
        return event_list.cbegin();
    }
    const_iterator cbegin() const
    {
        return event_list.cbegin();
    }

    /**
     * Return iterator to the one-past-end of the event-list.
     */
    const_iterator end() const
    {
        return event_list.cend();
    }
    const_iterator cend() const
    {
        return event_list.cend();
    }

    /**
     * Return true if the stab-forest does not hold data.
     */
    bool empty() const
    {
        return event_list.empty();
    }

    /**
     * Return the number of events in the stab-forest.
     */
    size_type size() const
    {
        return event_list.size();
    }

protected:
    /* The event-list. */
    event_list_type event_list;
};

/**
 * Use standard std::vector to represent the event-list. The event-list is
 * appended to, hence, the standard iterators are not stable. We use indices as
 * stable pointers.
 */
template <class TimeStampType>
class vector_event_list : public basic_event_list<std::vector<interval<TimeStampType>>>
{
protected:
    using bel = basic_event_list<std::vector<interval<TimeStampType>>>;
    using event_list_type = typename bel::event_list_type;
    using const_iterator = typename bel::const_iterator;
    using stable_event_pointer = typename event_list_type::size_type;

    /**
     * Default-constructor.
     */
    vector_event_list() : bel() {}

    /**
     * Return a stable pointer pointing to the same element as the provided
     * iterator.
     */
    stable_event_pointer stabilize_iterator(const const_iterator it) const
    {
        return std::distance(this->cbegin(), it);
    }

    /**
     * Return an iterator pointing to the same element as the provided
     * stable pointer.
     */
    const_iterator unstabilize_pointer(const stable_event_pointer p) const
    {
        return std::next(this->cbegin(), p);
    }
};

/**
 * Use a block-list to represent the event-list. Compared to a std::vector, this
 * yields faster appends and slower traversals. The current implementation does
 * not provide the stab-forward jump-optimization.
 */
template <class TimeStampType>
class block_event_list : public basic_event_list<block_list<interval<TimeStampType>>>
{
protected:
    using bel = basic_event_list<block_list<interval<TimeStampType>>>;
    using event_list_type = typename bel::event_list_type;
    using const_iterator = typename bel::const_iterator;
    using stable_event_pointer = typename bel::const_iterator;

    /**
     * Default-constructor.
     */
    block_event_list() : bel() {}

    /**
     * Return a stable pointer pointing to the same element as the provided
     * iterator.
     */
    stable_event_pointer stabilize_iterator(const const_iterator it) const
    {
        return it;
    }

    /**
     * Return an iterator pointing to the same element as the provided
     * stable pointer.
     */
    const_iterator unstabilize_pointer(const stable_event_pointer p) const
    {
        return this->event_list.repoint_iterator(p);
    }
};

/**
 * The stab forest for the specified timestamp type and the specified underlying
 * implementation of the event-list (vector_event_list or block_event_list).
 */
template <class TimeStampType, template <class> class EventList = vector_event_list>
class stab_forest : public EventList<TimeStampType>
{
public:
    using event_list_base = EventList<TimeStampType>;
    using stab_forest_type = stab_forest<TimeStampType, EventList>;

    using timestamp = typename event_list_base::timestamp;
    using event = typename event_list_base::event;
    using const_iterator = typename event_list_base::const_iterator;
    using stable_event_pointer = typename event_list_base::stable_event_pointer;
    using size_type = typename event_list_base::size_type;

    /* Forward declaration of the stab-forward helper. */
    template <class OutputIterator, class JumpPolicy>
    class stab_forward_helper;

private:
    using event_list_base::event_list;
    using event_list_base::stabilize_iterator;
    using event_list_base::unstabilize_pointer;

public:
    /**
     * Default-constructor.
     */
    stab_forest() : event_list_base(), nodes(), index(),
                    tail_pointer(stabilize_iterator(event_list.cend())),
                    min_key(std::numeric_limits<timestamp>::max()) {}

    /**
     * Append an event to the stab forest. The new event must be at-or-after, in
     * lexicographic (start, end)-time order, the last event appended.
     */
    void append_event(const event current)
    {
        if (!event_list.empty())
        {
            if (current.start != event_list.back().start)
            {
                build_leaf_forest_point();
            }
            event_list.emplace_back(current);
        }
        else
        {
            min_key = current.start;
            event_list.emplace_back(current);
        }
    }

    void append_event(const timestamp start, const timestamp end)
    {
        append_event(event{start, end});
    }

    /**
     * Perform a stab and search: copy all events active at value to output and
     * return an iterator pointing to the first event that starts strictly after
     * value.
     */
    template <class OutputIterator>
    const_iterator stab_search(const timestamp value, OutputIterator output) const
    {
        stab_operations<OutputIterator> operations{*this, output};
        navigate_index(value, operations);
        return operations.next_it;
    }

    /**
     * Return a stab-forward helper that allows for repeated stab and scan
     * operations (on increasing start-times). Can be used to answer
     * multi-stab-queries and multi-window-queries. The end-user can fine tune
     * the stabbing behavior by setting the jump policy used (index jumps with
     * a specified threshold factor or event-list jumps).
     */
    template <class OutputIterator, class JumpPolicy>
    stab_forward_helper<OutputIterator, JumpPolicy> stab_forward_search(OutputIterator output,
                                                                        const JumpPolicy &policy) const
    {
        return stab_forward_helper<OutputIterator, JumpPolicy>{*this, output, policy};
    }

    template <class OutputIterator, class JumpPolicy>
    std::shared_ptr<stab_forward_helper<OutputIterator, JumpPolicy>> stab_forward_search_shared(OutputIterator output,
        const JumpPolicy& policy) const
    {
        return std::shared_ptr<stab_forward_helper<OutputIterator, JumpPolicy>>(new stab_forward_helper<OutputIterator, JumpPolicy>(*this, output, policy));
    }

    /**
     * Return the hieght of the index.
     */
    size_type index_height() const
    {
        return (index.empty()) ? 0 : index.front().height;
    }

private:
    using event_array = raw_array<event>;

    /*
     * Implementation details:
     * We represent stab-tree nodes by straightforward structures holding all
     * annotations necessary. The only exception are the parts of the left-list.
     * To save on the number of allocations, which can be a hugely limiting
     * factor for insert performance, and to increase locality of traversing
     * both descending end-time ordered parts of the left-list (data key and
     * navigation key), the left-list parts are represented by a single list of
     * events. The first nll_size events are the ascending start-time ordered
     * navigation key left-list. The following ll_size - nll_size events are the
     * descending end-time ordered data key left-list. The last nll_size events
     * are the descending end-time ordered navigation key left-list.
     *
     * We represent each forest-point (T, m) by a stab-tree node, the left child
     * being T, the left-list being max-list (and, strictly speaking, not a
     * valid left-list), and the forest-point itself being node m. The right
     * child is the next forest-point in the index. In this way, we can perform
     * forest-point navigation and stab-tree navigation in a uniform manner.
     *
     * We use block-lists to allocate and construct forest-points and stab-tree
     * nodes in. This serves four purposes. First, we eliminate the need for
     * explicit memory management on our side. Second, the block-list performs
     * slab-allocation (doing only a single allocation per many constructed
     * values). Third, due to the block allocation and the order in which
     * forest-points are constructed, the sequence of forest-points is stored in
     * consecutive memory (following right child pointers on forest-points will
     * go to the next element in the list). Finally, observe that each
     * forest-point (T, n, max-list) also points to a stab-tree node n'. This
     * node will replace node n whenever the forest-point is merged. By doing
     * so, we can construct all stab-tree nodes on increasing start-time order
     * (as n' is created in the list of stab-tree nodes). Due to the block
     * allocation used by the block-lists, this way to constructing stab-tree
     * nodes guarantees high locality for the deeper parts of the index and
     * allows us to do an in-order traversal of stab-trees by simply traversing
     * the block-list nodes. Combined, these properties benefit both insertion
     * and query performance.
     *
     * We observe that in the below, every leaf node has height zero and every
     * forest-point has height at least one (due to forest-points always having
     * a left-hand child, even if this is a mere dummy).
     */

    /**
     * The stab-tree node.
     */
    struct stab_tree_node
    {
        using node_pointer = stab_tree_node *;
        using const_node_pointer = const stab_tree_node *;

        /* The navigation key and data key of this internal node.*/
        timestamp nkey;
        timestamp dkey;

        /* The left and right child. */
        node_pointer left_ptr;
        node_pointer right_ptr;

        /* Total height of this node. */
        size_type height;

        /* Stable iterator pointing to the first event with the start-time dkey
         * and to the one-past-last event with the start-time dkey. */
        stable_event_pointer data_begin;
        stable_event_pointer data_end;

        /* The left-lists of this node. When the node is used as a tree root in
         * the tree list, these left-lists represent max-lists. Use the
         * ll_-family of functions to access (details of) the left-list. */
        size_type nll_size;
        size_type ll_size;
        event_array ll_raw_data;
    };

    using node_pointer = typename stab_tree_node::node_pointer;
    using const_node_pointer = typename stab_tree_node::const_node_pointer;

    /**
     * The forest-point. Observe that the left-list of this forest-point
     * represents the max-list.
     */
    struct forest_point : public stab_tree_node
    {
        /**
         * Construct the forest-point. The first argument is the stab-tree node
         * that will replace this forest-point (when merged). The remaining
         * arguments are used to construct the node representing all information
         * of the forest-point (see the implementation details above)
         */
        template <class... Args>
        forest_point(const node_pointer replacement_node, Args &&...args) : stab_tree_node{std::forward<Args>(args)...}, replacement_node(replacement_node) {}

        /* Pointer to the normal node that is set to replace this forest node
         * when this forest is merged into another forest. */
        node_pointer replacement_node;
    };

    /* Return the begin() and end() iterators to the left-list (navigation key),
     * sorted on ascending start-time order. */
    template <class Node>
    static auto nll_sa_begin(Node &node) { return node.ll_raw_data.data(); }
    template <class Node>
    static auto nll_sa_end(Node &node) { return node.ll_raw_data.data() + node.nll_size; }

    /* Return the begin() and end() iterators to the left-list (data key),
     * sorted on descending end-time order. */
    template <class Node>
    static auto dll_ed_begin(Node &node) { return node.ll_raw_data.data() + node.nll_size; }
    template <class Node>
    static auto dll_ed_end(Node &node) { return node.ll_raw_data.data() + node.ll_size; }

    /* Return the begin() and end() iterators to the left-list (navigation key),
     * sorted on descending end-time order. */
    template <class Node>
    static auto nll_ed_begin(Node &node) { return node.ll_raw_data.data() + node.ll_size; }
    template <class Node>
    static auto nll_ed_end(Node &node) { return node.ll_raw_data.data() + node.ll_size + node.nll_size; }

    /**
     * Copy events in [first, last) to output that start before the specified
     * value v (and start at-or-after mstart) from an ascending start-time
     * ordered list of events; return an iterator pointing to the first value
     * not copied. Observe that these functions do not return output iterators,
     * making these functions work differently form the std::copy-family.
     */
    template <class InIt, class OutIt>
    static InIt copy_start_asc(InIt first, InIt last, OutIt output, const timestamp v)
    {
        while (first != last && first->start <= v)
        {
            *output++ = *first++;
        }
        return first;
    }
    template <class InIt, class OutIt>
    static InIt copy_start_asc(InIt first, InIt last, OutIt output, const timestamp v, const timestamp mstart)
    {
        while (first != last && first->start < mstart)
        {
            ++first;
        }
        return copy_start_asc(first, last, output, v);
    }

    /**
     * Copy events in [first, last) to output that end after the specified
     * value v (and start at-or-after mstart) from an descending end-time
     * ordered list of events; return an iterator pointing to the first value
     * not copied. Observe that these functions do not return output iterators,
     * making these functions work differently form the std::copy-family.
     */
    template <class InIt, class OutIt>
    static InIt copy_end_dec(InIt first, InIt last, OutIt output, const timestamp v)
    {
        while (first != last && first->end >= v)
        {
            *output++ = *first++;
        }
        return first;
    }
    template <class InIt, class OutIt>
    static InIt copy_end_dec(InIt first, InIt last, OutIt output, const timestamp v, const timestamp mstart)
    {
        while (first != last && first->end >= v)
        {
            if (mstart <= first->start)
            {
                *output++ = *first;
            }
            ++first;
        }
        return first;
    }

    /**
     * Query and navigate the stab-forest using the index. This will search for
     * the node for which nkey <= value <= dkey holds. This method uses
     * callbacks to perform the necessary actions on each visited forest-point
     * and stab-tree node. The following methods should be provided:
     *
     *   op.before_trees(value, other...) is called when value is smaller or
     *     equal to the smallest start-time present in the stab-forest.
     *   op.after_trees(value, other...) is called when value is larger than the
     *     largest start-time present in the stab-forest (but not necessary
     *     larger than the start-times of events in the event-list tail).
     *   op.left_child(node, value, other...) is called to indicate that the
     *     search will navigate to the left child of the specified node.
     *   op.right_child(node, value, other...) is analogous to left_child.
     *   op.select_node(node, value, other...) is called when node is the node
     *     for which nkey <= value <= dkey holds.
     *
     * The methods before_trees, after_trees, and select_node are only called
     * when the search has finished. The other... parameters are simply passed
     * on to the callback functions.
     */
    template <class NavigateOperations, class... Other>
    void navigate_index(const timestamp value,
                        NavigateOperations &op, Other &...other) const
    {
        /* Value not in the tree or the left-most timestamp. */
        if (value <= min_key)
        {
            op.before_trees(value, other...);
        }

        /* Value is indexed by the forest. */
        else if (!index.empty() && value <= index.back().dkey)
        {
            navigate_stab_tree_node(&index.front(), value, op, other...);
        }

        /* Value after the last start time in the last tree. */
        else
        {
            op.after_trees(value, other...);
        }
    }

    /**
     * Query and navigate a stab-tree. This method assumes that node has a
     * descendant for which nkey <= value <= dkey holds. See navigate_index
     * on the details of the navigation operations (we only use op.left_child(),
     * op.right_child(), and op.select_node()).
     */
    template <class NavigateOperations, class... Other>
    void navigate_stab_tree_node(const_node_pointer node, timestamp value,
                                 NavigateOperations &op, Other &...other) const
    {
        /* Traverse the forest until the node is found for which value is
         * active at [nkey, dkey]. */
        while (!(node->nkey <= value && value <= node->dkey))
        {
            if (value < node->nkey)
            {
                op.left_child(*node, value, other...);
                node = node->left_ptr;
            }
            else
            {
                op.right_child(*node, value, other...);
                node = node->right_ptr;
            }
        }

        /* The node is found. */
        op.select_node(*node, value, other...);
    }

    /* Classes supporting stab-forest querying and navigation (implementing
     * NavigateOperations as used by the navigate_index and
     * navigate_stab_tree_node functions). */
    template <class OutputIterator>
    struct stab_operations;
    struct probe_operations;

    /**
     * Construct a new forest-point representing a leaf (when the current
     * event-list tail needs to be incorporated into the index).
     */
    void build_leaf_forest_point()
    {
        /* Collect left-list details. */
        auto first = unstabilize_pointer(tail_pointer);
        auto last = event_list.cend();
        auto stable_end = stabilize_iterator(event_list.cend());
        timestamp key = first->start;

        /* Make the new leaf node and tree root. */
        auto nkey = (!index.empty()) ? index.back().dkey + 1 : key;
        auto &leaf = nodes.emplace_back(nkey, key, nullptr, nullptr, 0u,
                                        tail_pointer, stable_end, 0u, 0u, 0u);

        /* Add a root for this new tree. */
        auto ml_rbegin = std::make_reverse_iterator(last);
        auto ml_rend = std::make_reverse_iterator(first);

        size_type size = std::distance(first, last);
        auto &fp = index.emplace_back(&leaf,
                                      nkey, key, nullptr, nullptr, 0u,
                                      tail_pointer, stable_end, 0u, size, size);

        std::copy(ml_rbegin, ml_rend, dll_ed_begin(fp));

        tail_pointer = stable_end;
        maintain_index();
    }

    /**
     * Maintain the index.
     */
    void maintain_index()
    {
        /* Maintain the list of forest-points. Merge forest-points of equal
         * height; which must necessary be the last two forest-points. */
        while (index.size() >= 2)
        {
            auto it = index.end();
            auto &right_fp = *--it;
            auto &left_fp = *--it;

            /* If maintenance has finished, then relink the last two trees. */
            if (left_fp.height != right_fp.height)
            {
                left_fp.right_ptr = &right_fp;
                return;
            }

            /* If not finished, then merge the last two trees. */
            merge_forest_points(left_fp, right_fp);
        }
    }

    /**
     * Merge the forest-points.
     */
    void merge_forest_points(forest_point &left, forest_point &right)
    {
        node_pointer root = left.replacement_node;
        node_pointer fp_node = right.replacement_node;

        /* Basic properties of the merged tree. By the append-only structure,
         * the requirements on nkey and dkey are maintained. As dkey is not
         * updated, the data_pointer remains valid. */
        root->left_ptr = left.left_ptr;
        root->right_ptr = right.left_ptr;
        root->height = left.height;

        /* Determine how many events from the max-list of left need to go into
         * the new max-list; this by searching in both the navigation key
         * max-list and the data key max-list of left. */
        auto dll_it = std::upper_bound(dll_ed_begin(left), dll_ed_end(left), fp_node->nkey,
                                       event::end_compare(std::greater<>()));
        auto nll_it = std::upper_bound(nll_ed_begin(left), nll_ed_end(left), fp_node->nkey,
                                       event::end_compare(std::greater<>()));

        /* Compute the sizes of the new left-list and max-list. */
        size_type dll_size = std::distance(dll_it, dll_ed_end(left));
        size_type nll_size = std::distance(nll_it, nll_ed_end(left));
        size_type ml_add_size = left.ll_size - (dll_size + nll_size);

        /* Construct new raw arrays to hold the data. */
        event_array raw_left_list(dll_size + 2 * nll_size);
        event_array raw_max_list(right.nll_size + right.ll_size + 2 * ml_add_size);

        /* Split the ascending start-time ordered max-list (navigation key) of
         * left into the left-list of root and the max-list of fp. */
        auto p = std::partition_copy(nll_sa_begin(left), nll_sa_end(left),
                                     raw_max_list.data(), raw_left_list.data(),
                                     event::end_predicate(fp_node->nkey, std::greater_equal<>()));

        /* Copy the remaining max-list (data key) of left to the left-list into
         * the left-list of root. */
        auto ll_it = std::copy(dll_it, dll_ed_end(left), p.second);

        /* Copy the remaining descending end-time ordered max-list (navigation
         * key) of left into the left-list of root. */
        std::copy(nll_it, nll_ed_end(left), ll_it);

        /* Append the max-list (data key) of left to the max_list of fp. */
        auto rbegin = std::make_reverse_iterator(dll_it);
        auto rend = std::make_reverse_iterator(dll_ed_begin(left));
        auto ml_it = std::copy(rbegin, rend, p.first);

        /* Copy the ascending start-time ordered max-list (navigation key) of
         * the right tree to the max-list of fp. */
        ml_it = std::copy(nll_sa_begin(right), nll_sa_end(right), ml_it);

        /* Copy the max-list (data key) of right to the max-list of fp. */
        ml_it = std::copy(dll_ed_begin(right), dll_ed_end(right), ml_it);

        /* We merge the parts of the end-time ordered max-lists (navigation
         * key and data key) of left that are moved to the max-list of fp and
         * the max-list of right into the max-list of fp (descending end-time
         * ordered). */
        merge_three_way(nll_ed_begin(left), nll_it,
                        dll_ed_begin(left), dll_it,
                        nll_ed_begin(right), nll_ed_end(right),
                        ml_it, event::end_compare(std::greater<>()));

        /* Remove the old forest-points and construct the new forest-point. */
        index.pop_back();
        index.pop_back();
        index.emplace_back(fp_node,
                           fp_node->nkey, fp_node->dkey, root, nullptr, root->height + 1,
                           fp_node->data_begin, fp_node->data_end,
                           right.nll_size + ml_add_size, right.ll_size + ml_add_size, std::move(raw_max_list));
        root->nll_size = nll_size;
        root->ll_size = nll_size + dll_size;
        root->ll_raw_data.swap(raw_left_list);
    }

    /* The stab-tree nodes used in the stab-forest index. */
    block_list<stab_tree_node> nodes;

    /* The stab-forest index. */
    block_list<forest_point, 32> index;

    /* The tail pointer. */
    stable_event_pointer tail_pointer;

    /* The start-time of the first event in the event list. */
    timestamp min_key;
};

/**
 * The stab-forward helper will allow forward iteration over the stab-forest.
 * Additionally, this helper also allows one to jump forward to a provided
 * timestamp in the stab-forest. Such a jump will perform a stab with the
 * provided timestamp: any event that is active during the provided timestamp
 * and starts at-or-after the current start-time is written to the a provided
 * output iterator. The stab-forward helper is only valid when the underlying
 * stab-forest is still in scope and no additional events have been appended to
 * the stab-forest.
 */
template <class TimeStampType, template <class> class EventList>
template <class OutputIterator, class JumpPolicy>
class stab_forest<TimeStampType, EventList>::stab_forward_helper : private JumpPolicy
{
private:
    /**
     * Construct an initial stab-forward helper. This constructor is used by the
     * stab-forest itself. After initial construction, this class can be moved
     * using the move-constructor.
     */
    stab_forward_helper(const stab_forest_type &forest, OutputIterator output, const JumpPolicy &policy) : JumpPolicy(policy),
                                                                                                           forest(forest),
                                                                                                           output(output),
                                                                                                           event_list_it(forest.cbegin()),
                                                                                                           went_left(false),
                                                                                                           first_left_parent(nullptr),
                                                                                                           visited_nodes(forest.index.empty() ? 0u : forest.index_height() + 1),
                                                                                                           start_asc_it(visited_nodes.size()) {}

    friend stab_forest_type;

public:
    /**
     * Move-constructor.
     */
    stab_forward_helper(stab_forward_helper &&other) : JumpPolicy(other),
                                                       forest(other.forest),
                                                       output(std::move(output)),
                                                       event_list_it(other.event_list_it),
                                                       went_left(other.went_left),
                                                       first_left_parent(other.first_left_parent),
                                                       visited_nodes(std::move(other.visited_nodes)),
                                                       start_asc_it(std::move(other.start_asc_it)) {}

    /**
     * No copy-constructor.
     */
    stab_forward_helper(const stab_forward_helper &other) = delete;

    /**
     * No assignment.
     */
    stab_forward_helper &operator=(const stab_forward_helper &other) = delete;

    /* Forward-iterator-like interface. */
    using value_type = event;

    std::size_t size(const_iterator it) const
    {
        return std::distance(event_list_it, it);
    }

    /**
     * Move to the next event in the event-list.
     */
    auto &operator++()
    {
        ++event_list_it;
        return *this;
    }

    /**
     * Read the current event in the event-list.
     */
    const value_type &operator*() const
    {
        return *event_list_it;
    }
    const value_type *operator->() const
    {
        return &*event_list_it;
    }

    /**
     * visit event list by index
     */
    const timestamp operator[](std::size_t const &idx) const
    {
        return std::next(event_list_it, idx)->start;
    }

    /**
     * Iterator comparisons (one can use a normal forest.end() as the end of the
     * event-list).
     */
    bool operator==(const const_iterator &other) const
    {
        return event_list_it == other;
    }
    bool operator!=(const const_iterator &other) const
    {
        return event_list_it != other;
    }

    /**
     * Return iterator to the current event in the event list.
     */
    const_iterator get_iterator() const
    {
        return event_list_it;
    }

    /**
     * Jump forward in the event-list. Also stab with the specified value by
     * writing all events active during value and that start at-or-after the
     * start-time of the current event to the output. (We assume that the
     * iterator currently points to the first event with the start-time of the
     * current event, hence, all events that start at the current start-time are
     * subject to inclusion in the stab-result).
     */
    void stab_forward(const timestamp value)
    {
        policy_stab_forward(value, *this);
    }

    void stab_forward(const timestamp value, const_iterator* it)
    {
        policy_stab_forward(value, *this, it);
    }

    /**
     * Return forest.stab_search, see stab_forest::stab_search
     */
    template <class OutputIterator>
    const_iterator stab_search(const timestamp value, OutputIterator output) const
    {
        return forest.stab_search(value, output);
    }

private:
    /* The underlying stab-forest. */
    const stab_forest_type &forest;

    /* The output iterator where stab results are written to. */
    OutputIterator output;

    /* The current position in the event-list. */
    const_iterator event_list_it;

    /* If set to true, then the last stab-forward operation traversed the forest
     * by navigated to the left child of some node. */
    bool went_left;

    /* If went_left, then this pointer points to first node at which the last
     * stab-forward operation navigated to the left child. */
    const_node_pointer first_left_parent;

    /* Pointers to the nodes visited during the last stab-forward operation. At
     * visited_nodes[i], the node with height i is stored. Observe that children
     * of parent p do not necessary have height p.height - 1. Hence, not all
     * consecutive values in visited_nodes[i] are relevant. These values are
     * only used to check if, starting at first_left_parent, to which point we
     * follow the same path through the stab-forest as during the last stab-
     * forward operation, as only this influences processing of left-lists. */
    std::vector<const_node_pointer> visited_nodes;

    /* For those nodes visited_nodes[i] that represent a node at which we went
     * to the left child during the previous stab-forward operation; we keep
     * track of the first event in the left-list (ordered on ascending
     * start-time) that we have not yet outputted by storing an iterator
     * pointing to this first event in start_asc_it[i]. */
    std::vector<const event *> start_asc_it;

    /**
     * Jump policy based choice of answering the stab-forward query.
     */
    void policy_stab_forward(const timestamp value, const stab_forward_index&)
    {
        index_stab_forward(value, &event_list_it);
    }

    void policy_stab_forward(const timestamp value, const stab_forward_index&, const_iterator* it)
    {
        index_stab_forward(value, it);
    }

    void policy_stab_forward(const timestamp value, const stab_forward_list&)
    {
        list_stab_forward(value, &event_list_it);
    }

    void policy_stab_forward(const timestamp value, const stab_forward_list&, const_iterator* it)
    {
        list_stab_forward(value, it);
    }

    void policy_stab_forward(const timestamp value, const stab_forward_check& c)
    {
        policy_stab_forward(value, c, &event_list_it);
    }

    void policy_stab_forward(const timestamp value, const stab_forward_check&, const_iterator* it)
    {
        auto end = forest.cend();
        size_type d = std::distance(it, end);
        if (d <= this->threshold || value <= it[this->threshold].start)
        {
            list_stab_forward(value, &it);
        }
        else
        {
            index_stab_forward(value, &it);
        }
    }

    /**
     * Perform stab-forward using the index.
     */
    void index_stab_forward(const timestamp value, const_iterator* it)
    {
        went_left = false;

        /* We have not yet visited anything, hence, initialize a stab. */
        if (first_left_parent == nullptr)
        {
            if (*it == forest.cbegin())
            {
                forest.navigate_index(value, *this);
            }
            else
            {
                forest.navigate_index(value, *this, (*it)->start);
            }
        }

        /* Continue after all data collected during the previous stab. */
        else
        {
            auto start_at_after = (*it)->start;

            /* Continue stabbing the tree. */
            if (value <= forest.index.back().dkey)
            {
                forest.navigate_stab_tree_node(first_left_parent, value, *this, start_at_after);
            }

            /* We will not end up in the tree, jump out of the tree. */
            else
            {
                after_trees(value, start_at_after);
            }
        }
    }

    /**
     * Perform stab-forward using the event-list.
     */
    void list_stab_forward(const timestamp value, const_iterator* it)
    {
        auto end = forest.cend();
        while (((*it) != end) && ((*it)->start <= value))
        {
            if (value <= (*it)->end)
            {
                *output++ = **it;
            }
            it->operator++();
        }
    }

    /* The stab_forward_helper uses the navigate_index and
     * navigate_stab_tree_node methods for performing the underlying stab-forest
     * navigation. Following is the callback interface necessary for these
     * methods.
     *
     * Observe that each method can be called with a single optional argument.
     * This argument is used to pass through the start_at_after parameter that
     * is used in consecutive stabs to filter the left-lists to only include
     * not-yet-visited events. To a large degree, these methods follow the same
     * principles as those in stab_operations.
     *
     * We observe that start_at_after is at least the value that was used during
     * the previous stab_forward operation. Hence, we use start_at_after to
     * determine if we went left or right at a given node (and to eliminate
     * unnecessary left-list traversals when navigating to a right child). */

    template <class... Other>
    void before_trees(const timestamp value, Other... other)
    {
        event_list_it = copy_start_asc(event_list_it, forest.event_list.cend(),
                                       output, value, other...);
    }

    template <class... Other>
    void after_trees(const timestamp value, Other... other)
    {
        /* Search stab results in the max-lists of tree roots. */
        if (!forest.index.empty())
        {
            /* If we have not yet visited any trees in the forest, then we visit
             * all of them. */
            if (first_left_parent == nullptr)
            {
                first_left_parent = &forest.index.front();
            }

            /* Visit the trees. */
            while (first_left_parent != nullptr)
            {
                right_child(*first_left_parent, value, other...);
                first_left_parent = first_left_parent->right_ptr;
            }

            /* Set the last forest-point as the first_left_parent to prevent
             * further traversals of the entire index. */
            first_left_parent = &forest.index.back();
        }

        /* See if we need to include the event-list tail. */
        auto tail_begin = forest.unstabilize_pointer(forest.tail_pointer);
        if (!forest.empty() && (value < forest.event_list.back().start))
        {
            event_list_it = tail_begin;
        }
        else
        {
            auto rbegin = std::make_reverse_iterator(forest.event_list.cend());
            auto rend = std::make_reverse_iterator(tail_begin);
            copy_end_dec(rbegin, rend, output, value, other...);
            event_list_it = forest.cend();
        }
    }

    template <class... Other>
    void left_child(const stab_tree_node &node, const timestamp value, Other... other)
    {
        /* Set the first_left_parent if we did not yet navigate to a left child
         * during this stab-forward operation. */
        if (!went_left)
        {
            went_left = true;
            first_left_parent = &node;
        }

        /* If we have not yet visited this node, then initialize the left-list
         * iterator to point to the first event. */
        auto begin = nll_sa_begin(node);
        if (visited_nodes[node.height] != &node)
        {
            visited_nodes[node.height] = &node;
            start_asc_it[node.height] = begin;
        }

        /* Perform the stab operation. */
        start_asc_it[node.height] = copy_start_asc(start_asc_it[node.height], nll_sa_end(node),
                                                   output, value, other...);
    }

    void right_child(const stab_tree_node &node, timestamp value)
    {
        /* This must be the first stab. Hence, we did not navigate to the right
         * child yet. Process the left-list ordered on descending end-times. */
        copy_end_dec(dll_ed_begin(node), dll_ed_end(node), output, value);
        copy_end_dec(nll_ed_begin(node), nll_ed_end(node), output, value);
        visited_nodes[node.height] = &node;
    }

    void right_child(const stab_tree_node &node, timestamp value, const timestamp start_at_after)
    {
        /* This is not the first stab. If we did navigate to the left child of
         * this node previously and did not yet traverse all possible events in
         * that left-list, then we need to process the left-list ordered on
         * descending end-times once. */
        visited_nodes[node.height] = &node;
        if (start_at_after <= node.dkey)
        {
            copy_end_dec(dll_ed_begin(node), dll_ed_end(node), output, value, start_at_after);
        }
        if (node.nkey != 0 && (start_at_after < node.nkey - 1))
        {
            copy_end_dec(nll_ed_begin(node), nll_ed_end(node), output, value, start_at_after);
        }
    }

    void select_node(const stab_tree_node &node, const timestamp value)
    {
        /* This must be the first stab. Hence, we did not navigate to the right
         * child yet. Process the left-list ordered on descending end-times. */
        visited_nodes[node.height] = &node;
        if (value == node.dkey)
        {
            copy_end_dec(dll_ed_begin(node), dll_ed_end(node), output, value);
        }
        copy_end_dec(nll_ed_begin(node), nll_ed_end(node), output, value);
        event_list_it = (value < node.dkey) ? forest.unstabilize_pointer(node.data_begin)
                                            : forest.unstabilize_pointer(node.data_end);
    }

    template <class... Other>
    void select_node(const stab_tree_node &node, const timestamp value, const timestamp start_at_after)
    {
        /* This is not the first stab. If we did navigate to the left child of
         * this node previously and did not yet traverse all possible events in
         * that left-list, then we need to process the left-list ordered on
         * descending end-times once. */
        visited_nodes[node.height] = &node;
        if (value == node.dkey)
        {
            copy_end_dec(dll_ed_begin(node), dll_ed_end(node), output, value, start_at_after);
        }
        if (node.nkey != 0 && (start_at_after < node.nkey - 1))
        {
            copy_end_dec(nll_ed_begin(node), nll_ed_end(node), output, value, start_at_after);
        }
        event_list_it = (value < node.dkey) ? forest.unstabilize_pointer(node.data_begin)
                                            : forest.unstabilize_pointer(node.data_end);
    }
};

/**
 * The navigate_index callback structure used by stab_search.
 */
template <class TimeStampType, template <class> class EventList>
template <class OutputIterator>
struct stab_forest<TimeStampType, EventList>::stab_operations
{
    const stab_forest_type &forest;
    OutputIterator output;
    const_iterator next_it;

    void before_trees(const timestamp value)
    {
        next_it = copy_start_asc(forest.event_list.cbegin(), forest.event_list.cend(), output, value);
    }

    void after_trees(const timestamp value)
    {
        /* Search stab results in the max-lists of tree roots. */
        for (auto &fp : forest.index)
        {
            right_child(fp, value);
        }

        /* See if we need to include the event-list tail. */
        auto tail_begin = forest.unstabilize_pointer(forest.tail_pointer);
        if (!forest.empty() && (value < forest.event_list.back().start))
        {
            next_it = tail_begin;
        }
        else
        {
            auto rbegin = std::make_reverse_iterator(forest.event_list.cend());
            auto rend = std::make_reverse_iterator(tail_begin);
            copy_end_dec(rbegin, rend, output, value);
            next_it = forest.cend();
        }
    }

    void left_child(const stab_tree_node &node, const timestamp value)
    {
        copy_start_asc(nll_sa_begin(node), nll_sa_end(node), output, value);
    }

    void right_child(const stab_tree_node &node, timestamp value)
    {
        copy_end_dec(dll_ed_begin(node), dll_ed_end(node), output, value);
        copy_end_dec(nll_ed_begin(node), nll_ed_end(node), output, value);
    }

    void select_node(const stab_tree_node &node, const timestamp value)
    {
        if (value == node.dkey)
        {
            copy_end_dec(dll_ed_begin(node), dll_ed_end(node), output, value);
        }
        copy_end_dec(nll_ed_begin(node), nll_ed_end(node), output, value);
        next_it = (value < node.dkey) ? forest.unstabilize_pointer(node.data_begin)
                                      : forest.unstabilize_pointer(node.data_end);
    }
};


#endif