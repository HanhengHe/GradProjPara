#include <iostream>
#include <memory>
#include <iterator>

template <typename Type>
class DoublyLinkedList
{
private:
    struct Node
    {
        Type value;
        std::unique_ptr<Node> next;
        Node *prev;

        Node(Type val, Node *prv, Node *nxt = nullptr)
            : value(val), prev(prv), next(nullptr)
        {
            if (next != nullptr)
            {
                next->prev = this;
            }
        }
    };

    std::unique_ptr<Node> head;
    Node *tail = nullptr;

public:
    // Default constructor
    DoublyLinkedList() {}

    // Copy constructor
    DoublyLinkedList(const DoublyLinkedList &other)
    {
        for (Node *curr = other.head.get(); curr != nullptr; curr = curr->next.get())
        {
            push_back(curr->value);
        }
    }

    // Move constructor
    DoublyLinkedList(DoublyLinkedList &&other) noexcept
        : head(std::move(other.head)), tail(other.tail)
    {
        other.tail = nullptr;
    }

    // Copy assignment
    DoublyLinkedList &operator=(const DoublyLinkedList &other)
    {
        if (this != &other)
        {
            DoublyLinkedList tmp(other);
            std::swap(head, tmp.head);
            std::swap(tail, tmp.tail);
        }
        return *this;
    }

    // Move assignment
    DoublyLinkedList &operator=(DoublyLinkedList &&other) noexcept
    {
        if (this != &other)
        {
            head = std::move(other.head);
            tail = other.tail;
            other.tail = nullptr;
        }
        return *this;
    }

    ~DoublyLinkedList() {}

    class iterator : public std::iterator<std::bidirectional_iterator_tag, T>
    {
        Node *ptr;

    public:
        explicit iterator(Node *p) : ptr(p) {}

        iterator &operator++()
        {
            ptr = ptr->next.get();
            return *this;
        }

        bool operator==(const iterator &other) const
        {
            return ptr == other.ptr;
        }

        bool operator!=(const iterator &other) const
        {
            return !(ptr == other.ptr);
        }

        Type &operator*() const
        {
            return ptr->value;
        }
    };

    iterator begin()
    {
        return iterator(head.get());
    }

    iterator end()
    {
        return iterator(nullptr);
    }

    void push_front(const Type &value)
    {
        std::unique_ptr<Node> new_node = std::make_unique<Node>(value, nullptr, head.get());
        if (head != nullptr)
        {
            head->prev = new_node.get();
        }
        head = std::move(new_node);
        if (tail == nullptr)
        {
            tail = head.get();
        }
    }

    void push_back(const Type &value)
    {
        if (tail == nullptr)
        { // List is empty
            push_front(value);
            return;
        }
        tail->next = std::make_unique<Node>(value, tail);
        tail = tail->next.get();
    }

    Type pop_front()
    {
        if (head == nullptr)
            throw std::runtime_error("List is empty");
        Type value = head->value;
        head = std::move(head->next);
        if (head != nullptr)
        {
            head->prev = nullptr;
        }
        else
        {
            tail = nullptr; // List became empty
        }
        return value;
    }

    Type pop_back()
    {
        if (tail == nullptr)
            throw std::runtime_error("List is empty");
        Type value = tail->value;
        if (tail->prev != nullptr)
        {
            tail = tail->prev;
            tail->next.reset();
        }
        else
        {
            head.reset();
            tail = nullptr;
        }
        return value;
    }
};