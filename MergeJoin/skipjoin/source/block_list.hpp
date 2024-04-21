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
#ifndef INCLUDE_BLOCK_LIST_HPP
#define INCLUDE_BLOCK_LIST_HPP

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <iterator>
#include <type_traits>
#include <utility>


/**
 * The block-list provides a simple list-like container that with constant-time
 * stack-like append-semantics and high-performance value traversal. Internally,
 * the container is a list, in which each list element holds a block of at most
 * C values. The container guarantees a worst-case complexity of O(1) for LIFO
 * append/removal operations and high-performance traversal.
 *
 * Block-lists provide strong stability guarantees with respect to pointers,
 * references, and iterators. Pointers and references to values stored in a
 * block-list remain valid until values are deleted. An iterator pointing to
 * the n-th value in the block-list will, at any point in time, remain pointing
 * to the n-th value (given that such a value exists). We observe that this
 * iterator stability only holds for iterators that point to values (which
 * excludes end() and, when the list is empty, begin()). Using the method
 * repoint_iterator(...), iterators obtained from begin() can be remapped to the
 * current-first element in the list and iterators obtained from end()---when
 * the list had n elements---can be remapped to the current n+1-th element (if
 * such an element exists). Using this method, iterators obtained from begin()
 * and end() can be turned into stable-like iterators.
 *
 * This container provides a less-general list interface as std::vector<> and
 * std::deque<>, but does provide lower value append times as both data
 * structures and slightly faster traversal than std::deque<>.
 */
template<class Type, std::size_t C = 1024>
class block_list final
{
public:
    /* Container-style typedefs (the block-list is a minimal container, not all
     * generic list operators are supported). */
    using value_type = Type;
    using list_type = block_list<value_type, C>;

    using size_type = std::size_t;
    using difference_type = std::ptrdiff_t;
    using pointer = value_type*;
    using const_pointer = const value_type*;
    using reference = value_type&;
    using const_reference = const value_type&;

private:
    /**
     * The block structure is basically a placeholder: we never properly
     * construct a block. We use it as a POD-type, set the previous/next
     * pointers manually, and construct values in-place. The block-list
     * maintains information that determines which values are constructed. The
     * block-list also performs all construction, deconstruction, and memory
     * management.
     */
    struct block
    {
        using block_pointer = block*;
        using const_block_pointer = const block*;

        block_pointer previous;
        block_pointer next;
        value_type values[C];
    };

    using block_pointer = typename block::block_pointer;
    using const_block_pointer = typename block::const_block_pointer;

    
    /* Forward declaration of the iterator type. */
    template<class BlockPointer, class ValueType,
             class ReferenceType, class ConstReferenceType,
             class PointerType, class ConstPointerType>
    class iterator_base;

    /* Evaluates to std::true_type if we need deconstruction of individual
     * values, otherwise to std::false_type. */
    using deconstruction_type = std::integral_constant<bool, std::is_trivially_destructible<value_type>::value>;

public:
    using iterator = iterator_base<block_pointer, value_type,
                                   reference, const_reference,
                                   pointer, const_pointer>;
    using const_iterator = iterator_base<const_block_pointer, const value_type,
                                         const_reference, const_reference,
                                         const_pointer, const_pointer>;

private:
    /* Pointer to the first list-block, the last list-block, and the number of
     * elements in each block. All blocks before the last block are completely
     * filled with C values. The last block is filled with only size() % C. Any
     * blocks after last will only exists if the block-list shrunk, and these
     * blocks will be entirely empty. */
    block_pointer first;
    block_pointer current;
    size_type data_size;
    size_type current_offset;

public:
    /**
     * Default-construct: empty block-list.
     */
    block_list() : first(nullptr), current(nullptr),
                   data_size(0), current_offset(0) { }

    /**
     * Move-construct.
     */
    block_list(list_type&& other) : block_list()
    {
        swap(other);
    }

    /**
     * Destructor.
     */
    ~block_list()
    {
        deconstruct_all(first, data_size, deconstruction_type());
        while (first != nullptr) {
            block_pointer pointer = first;
            first = first->next;
            ::operator delete(pointer);
        }
    }


    /**
     * Copy-assignment: not provided.
     */
    list_type& operator=(const list_type& other) = delete;

    /**
     * Move-assignment.
     */
    list_type& operator=(list_type&& other)
    {
        swap(other);
        return *this;
    }


    /**
     * Return iterator to the begin of the block-list.
     */
    iterator begin()
    {
        return { first, 0 };
    }
    const_iterator begin() const
    {
        return { first, 0 };
    }
    const_iterator cbegin() const
    {
        return begin();
    }

    /**
     * Return iterator to the end of the block-list.
     */
    iterator end()
    {
        return { current, current_offset };
    }
    const_iterator end() const
    {
        return { current, current_offset };
    }
    const_iterator cend() const
    {
        return end();
    }


    /**
     * Return the first element in the block-list.
     */
    reference front()
    {
        return *begin();;
    }
    const_reference front() const
    {
        return *begin();
    }

    /**
     * Return the last element in the block-list.
     */
    reference back()
    {
        return *--end();
    }
    const_reference back() const
    {
        return *--end();
    }


    /**
     * Construct-and-append value to the end of the block-list. Return a
     * reference to the appended value.
     */
    template<class... Args> reference emplace_back(Args&&... args)
    {
        if (first == nullptr) {
            block_pointer block = create_block();
            first = current = block;
            current_offset = 0;
        }
        else if (current_offset == C) {
            if (current->next == nullptr) {
                block_pointer block = create_block();
                current->next = block;
                block->previous = current;
            }
            current = current->next;
            current_offset = 0;
        }
        reference value = construct(current->values + current_offset,
                                    std::forward<Args>(args)...);
        ++current_offset;
        ++data_size;
        return value;
    }

    /**
     * Append value to the end of the block-list. Return a reference to the
     * appended value.
     */
    reference push_back(const_reference value)
    {
        return emplace_back(value);
    }

    /**
     * Remove the last value.
     */
    void pop_back()
    {
        assert(data_size > 0);
        if (current_offset == 0) {
            current_offset = C;
            current = current->previous;
        }
        --current_offset;
        deconstruct(current->values + current_offset, deconstruction_type());
        --data_size;
    }

    /**
     * Return the size of the block-list.
     */
    size_type size() const
    {
        return data_size;
    }

    /**
     * Retrun true if the block-list is empty.
     */
    bool empty() const
    {
        return size() == 0;
    }

    /**
     * Swapping block-lists.
     */
    void swap(list_type& other)
    {
        std::swap(other.first, first);
        std::swap(other.current, current);
        std::swap(other.data_size, data_size);
        std::swap(other.current_offset, current_offset);
    }

    /**
     * Repoint an iterator.
     */
    const_iterator repoint_iterator(const const_iterator it) const
    {
        /* begin() or end() in a list that never had a value. */
        if (it.data_pointer == nullptr) {
            return cbegin();
        }

        /* end() in a list in which the last block was completely filled. */
        else if (it.last == it.current) {
            const_iterator copy(it);
            --copy;
            ++copy;
            return copy;
        }

        /* Should already have pointed to a valid value. */
        else {
            return it;
        }
    }


private:
    /**
     * Create a block.
     */
    block_pointer create_block()
    {
        void* storage = ::operator new(sizeof(block));
        block_pointer pointer = static_cast<block_pointer>(storage);
        pointer->next = pointer->previous = nullptr;
        return pointer;
    }

    /**
     * Construct a value at the specified pointer, return reference to the
     * constructed value.
     */
    template<class... Args>
    reference construct(const pointer pointer, Args&&... args)
    {
        void* void_pointer = static_cast<void*>(pointer);
        ::new (void_pointer) value_type{std::forward<Args>(args)...};
        return *pointer;
    }

    /**
     * Deconstruct num values, start at the values in the first block
     * (deconstruction of the values is not necessary).
     */
    static void deconstruct_all(const block_pointer first, const size_type num, const std::true_type&)
    {
    }

    /**
     * Deconstruct num values, start at the values in the first block
     * (deconstruction of the values is necessary).
     */
    static void deconstruct_all(const block_pointer first, const size_type num, const std::false_type&)
    {
        block_pointer pointer = first;
        size_type offset = 0;
        size_type deleted = 0;
        while (deleted < num) {
            deconstruct(pointer->values + offset, deconstruction_type());
            ++offset;
            ++deleted;
            if (offset == C) {
                offset = 0;
                pointer = pointer->next;
            }
        }
    }

    /**
     * Deconstruct a value (deconstruction of the values is not necessary).
     */
    static void deconstruct(const pointer pointer, const std::true_type&)
    {
    }

    /**
     * Deconstruct a value (deconstruction of the values is necessary).
     */
    static void deconstruct(const pointer pointer, const std::false_type&)
    {
        pointer->~value_type();
    }
};


/**
 * Iterator base class used by the iterator and const_iterator types.
 */
template<class Type, std::size_t C>
template<class BlockPointer, class ValueType,
         class ReferenceType, class ConstReferenceType,
         class PointerType, class ConstPointerType>
class block_list<Type, C>::iterator_base
{
private:
    using block_pointer = BlockPointer;

public:
    using iterator_category = std::bidirectional_iterator_tag;
    using value_type = ValueType;
    using difference_type = std::ptrdiff_t;
    using reference = ReferenceType;
    using const_reference = ConstReferenceType;
    using pointer = PointerType;
    using const_pointer = ConstPointerType;
    using iterator_type = iterator_base<block_pointer, value_type,
                                        reference, const_reference,
                                        pointer, const_pointer>;

private:
    pointer first;
    pointer current;
    pointer last;
    block_pointer data_pointer;

    /**
     * Value-construct. To be used only by the block-list itself.
     */
    iterator_base(const block_pointer data_pointer, const size_type offset) :
                first(nullptr), current(nullptr), last(nullptr),
                data_pointer(data_pointer)
    {
        if (data_pointer != nullptr) {
            first = data_pointer->values;
            current = first + offset;
            last = first + C;
        }
    }

    /* Assure that the block-list can call the above value-constructor. */
    friend list_type;
    
public:
    /**
     * Default-construct.
     */
    iterator_base() { }

    /**
     * Copy-construct.
     */
    iterator_base(const iterator_type& other) :
                first(other.first), current(other.current), last(other.last),
                data_pointer(other.data_pointer) { }

    /**
     * Copy-assignment.
     */
    iterator_type& operator=(const iterator_type& other)
    {
        first = other.first;
        current = other.current;
        last = other.last;
        data_pointer = other.data_pointer;
        return *this;
    }

    /**
     * Move to the next value.
     */
    iterator_type& operator++()
    {
        ++current;
        if (current == last && data_pointer->next != nullptr) {
            data_pointer = data_pointer->next;
            first = current = data_pointer->values;
            last = first + C;
        }
        return *this;
    }
    iterator_type operator++(int)
    {
        auto it(*this);
        ++*this;
        return it;
    }

    /**
     * Move to the previous value.
     */
    iterator_type& operator--()
    {
        if (current == first && data_pointer->previous != nullptr) {
            data_pointer = data_pointer->previous;
            first = data_pointer->values;
            last = current = first + C;
        }
        --current;
        return *this;
    }
    iterator_type operator--(int)
    {
        auto it(*this);
        --*this;
        return it;
    }

    /**
     * Read and write values.
     */
    reference operator*()
    {
        return *current;
    }
    const_reference operator*() const
    {
        return *current;
    }

    /**
     * Pointer-like access.
     */
    pointer operator->()
    {
        return current;
    }
    const_pointer operator->() const
    {
        return current;
    }
    
    /**
     * Iterator comparisons.
     */
    bool operator==(const iterator_type& other) const
    {
        return current == other.current;
    }
    bool operator!=(const iterator_type& other) const
    {
        return !(*this == other);
    }
};

#endif