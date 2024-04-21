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
#ifndef INCLUDE_RAW_ARRAY_HPP
#define INCLUDE_RAW_ARRAY_HPP

#include <limits>
#include <new>
#include <type_traits>

/**
 * This class provides a minimalistic wrapper around fixed-size array pointer to
 * a trivial type with minimal overhead: no data value in the list will be
 * constructed or destructed. The user of this class is responsible to
 * initialize values in this list, manage access to the values in this list, and
 * keep track of the size of this list.
 */
template<class Type>
class raw_array final
{
public:
    using value_type = Type;
    using array_type = raw_array<value_type>;

    using size_type = std::size_t;
    using pointer = value_type*;
    using const_pointer = const value_type*;

    static_assert(std::is_trivial<value_type>::value, "type must be trivial");


    /**
     * Default-construct: empty array.
     */
    raw_array() : data_pointer(nullptr) { }

    /**
     * Construct array that can hold n values.
     */
    raw_array(const size_type n) : data_pointer()
    {
        if (n > 0) {
            allocate_size(n);
        }
    }

    /**
     * Move-construct.
     */
    raw_array(array_type&& other) : data_pointer()
    {
        swap(other);
    }

    /**
     * Destructor.
     */
    ~raw_array()
    {
        ::operator delete(data_pointer);
    }


    /**
     * Return data pointer.
     */
    pointer data()
    {
        return data_pointer;
    }
    const_pointer data() const
    {
        return data_pointer;
    }

    
    /**
     * Swapping arrays.
     */
    void swap(array_type& other)
    {
        std::swap(other.data_pointer, data_pointer);
    }

private:
    static constexpr size_type max_size = std::numeric_limits<size_type>::max() / sizeof(value_type);

    /**
     * Allocate the raw storage for the array, throws bad_alloc when allocation
     * fails.
     */
    void allocate_size(const size_type n)
    {
        if (max_size < n) {
            throw std::bad_alloc();
        }

        void* p = ::operator new(n * sizeof(value_type));
        data_pointer = (pointer) p;
    }

    pointer data_pointer;
};

#endif