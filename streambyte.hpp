/* Copyright 2019 Alexandre Leblanc
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to 
 * deal in the Software without restriction, including without limitation the 
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or 
 * sell copies of the Software, and to permit persons to whom the Software is 
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in 
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR 
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, 
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE 
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER 
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING 
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

#ifndef MRT_STREAMBYTE_HPP_
#define MRT_STREAMBYTE_HPP_

#include <array>
#include <cstddef>
#include <istream>
#include <iterator>
#include <streambuf>

#ifdef _MSC_VER
#define MRT_HARDWARE_CI_SIZE std::hardware_constructive_interference_size
#else
#define MRT_HARDWARE_CI_SIZE 64
#endif

namespace mrt {

// Iterator for byte reading from istream.
// Optimized to read by blocks.
template <std::size_t buf_size = MRT_HARDWARE_CI_SIZE>
class istreambyte_iterator {
public:
    using byte_type = std::byte;
    using iterator_category = std::input_iterator_tag;
    using value_type = byte_type;
    using pointer = const byte_type*;
    using reference = byte_type;
    using difference_type = char;

    using int_type = char;
    using traits_type = std::char_traits<int_type>;
    using streambuf_type = std::basic_streambuf<int_type, traits_type>;
    using istream_type = std::basic_istream<int_type, traits_type>;

private:
    using buf_pos_type = std::size_t;
    using array_type = std::array<int_type, buf_size>;

private:
    class istreambyte_proxy {
    public:
        [[nodiscard]] constexpr
        byte_type operator*() const noexcept {
            return m_buf[m_buf_pos];
        }

    private:
        friend istreambyte_iterator;
        istreambyte_proxy(buf_pos_type to_read, buf_pos_type buf_pos, 
            const array_type& buf, streambuf_type* streambuf) noexcept
            : m_to_read{to_read}, m_buf_pos{buf_pos}, m_streambuf{streambuf}, m_buf{buf}
        { }
        
        buf_pos_type m_to_read;
        buf_pos_type m_buf_pos;
        streambuf_type* m_streambuf;
        array_type& m_buf;
    };

public:
    constexpr
    istreambyte_iterator() noexcept
        : m_value_valid{false}, m_to_read{0}, m_buf_pos{0}, m_streambuf{nullptr}, m_buf{0}
    { }

    istreambyte_iterator(istream_type& stream)
        : istreambyte_iterator(stream.rdbuf())
    { }

    istreambyte_iterator(streambuf_type* sb)
        :  m_value_valid{false}, m_to_read{0}, m_buf_pos{0}, m_streambuf{sb}, m_buf{0}
    { 
        if (m_streambuf) {
            _read_block();
        }
    }

    istreambyte_iterator(const istreambyte_proxy& proxy) noexcept
        : m_value_valid{proxy.m_streambuf == nullptr}, m_to_read{proxy.m_to_read}, m_buf_pos{proxy.m_buf_pos}, m_streambuf{proxy.m_streambuf}
    { }

public:
    [[nodiscard]]
    byte_type operator*() const {
        if (!m_value_valid) {
            _peek();
        }

        return _value();
    }

    istreambyte_iterator& operator++() {
        _increment();

        return *this;
    }

    istreambyte_proxy operator++(int) {
        if (!m_value_valid) {
            _peek();
        }

        istreambyte_proxy tmp{ --m_to_read, m_buf_pos, m_buf,  m_streambuf };
        ++*this;

        return tmp;
    }

    [[nodiscard]]
    bool equal(const istreambyte_iterator& lrs) const {	
        if (!m_value_valid)
            _peek();

        if (!lrs.m_value_valid)
            lrs._peek();

        return (m_to_read == 0 && m_streambuf == nullptr && lrs.m_streambuf == nullptr) 
            || (m_streambuf != nullptr && lrs.m_streambuf != nullptr);
    }

private:
    // Next value to deliver
    [[nodiscard]] constexpr
    byte_type _value() const noexcept {
        return static_cast<byte_type>(_current());
    }

    // Current value (sgetc equivalent)
    [[nodiscard]] constexpr
    int_type _current() const noexcept {
        return m_buf[m_buf_pos];
    }

    // Equivalent to sbumpc but for a block.
    [[nodiscard]]
    int_type _bump() {
        ++m_buf_pos;
        --m_to_read;

        if (m_buf_pos >= buf_size) {
            _read_block();
        }

        return _current();
    }

    void _increment() {	
        if ( (m_streambuf == nullptr && m_to_read == 0) || traits_type::eq_int_type(traits_type::eof(), _bump())) {
            m_streambuf = nullptr;
            m_value_valid = true;
        } else {
            m_value_valid = false;
        }
    }

    void _peek() const noexcept {
        if ( (m_streambuf == nullptr && m_to_read == 0) || traits_type::eq_int_type(traits_type::eof(), _current())) {
            m_streambuf = nullptr;
        }
        
        m_value_valid = true;
    }

    void _read_block() const {
        m_buf_pos = 0;
        m_to_read = static_cast<buf_pos_type>(m_streambuf->sgetn(m_buf.data(), buf_size));
        
        if (m_to_read < buf_size) {
            m_streambuf = nullptr;
        }
    }

private:
    mutable bool m_value_valid;
    mutable buf_pos_type m_to_read;
    mutable buf_pos_type m_buf_pos;
    mutable streambuf_type* m_streambuf;
    mutable array_type m_buf;
};

template<std::size_t buf_size>
inline
bool operator==(const istreambyte_iterator<buf_size>& lhs, const istreambyte_iterator<buf_size>& lrs) noexcept
{
    return lhs.equal(lrs);
}

template<std::size_t buf_size>
inline
bool operator!=(const istreambyte_iterator<buf_size>& lhs, const istreambyte_iterator<buf_size>& lrs) noexcept
{
    return !lhs.equal(lrs);
}

// Outbut streambyte iterator to write to a streambuf. 
// Uses an internal buffer and flushes only when full or on object deallocation
template<std::size_t buf_size = MRT_HARDWARE_CI_SIZE>
class ostreambyte_iterator {
public:
    using iterator_category = std::output_iterator_tag;
    using value_type = void;
    using difference_type = void;
    using pointer = void;
    using reference = void;

    using char_type = char;
    using traits_type = std::char_traits<char_type>;
    using streambuf_type = std::basic_streambuf<char_type, traits_type>;
    using ostream_type = std::basic_ostream<char_type, traits_type>;

private:
    using byte_type = std::byte;
    using array_type = std::array<char_type, buf_size>;
    using size_type = std::size_t;

public:
    ostreambyte_iterator(streambuf_type* streambuf) noexcept
        : m_failure{false}, m_streambuf{streambuf}, m_buf_pos{0}, m_buf{}
    { }

    ostreambyte_iterator(ostream_type& stream) noexcept
        : ostreambyte_iterator{stream.rdbuf()}
    { }

    ostreambyte_iterator(const ostreambyte_iterator& rhs)
        : m_failure{rhs.m_failure}, m_streambuf{rhs.m_streambuf}, m_buf_pos{0}, m_buf{}
    {
        const_cast<ostreambyte_iterator&>(rhs)._commit();
        const_cast<ostreambyte_iterator&>(rhs)._clear_buffer();
    }

    ~ostreambyte_iterator() {
        _commit();
    }

    ostreambyte_iterator& operator=(byte_type rhs) {
        if (m_streambuf == nullptr || traits_type::eq_int_type(traits_type::eof(), _put(static_cast<char_type>(rhs)))) {
            m_failure = true;
        }

        return *this;
    }

    [[nodiscard]]
    ostreambyte_iterator& operator*() noexcept {
        return *this;
    }

    ostreambyte_iterator& operator++() noexcept {
        return *this;
    }

    ostreambyte_iterator& operator++(int) noexcept {
        return *this;
    }

    [[nodiscard]] constexpr
    bool failed() const noexcept {
        return m_failure;
    }

private:
    bool _commit() {
        std::streamsize current_size = static_cast<std::streamsize>(_buf_size());
        return m_streambuf->sputn(m_buf.data(), current_size) == current_size;
    }

    char_type _put(char_type c) {
        m_buf[m_buf_pos] = c;
        ++m_buf_pos;

        if (_buf_size() == buf_size) {
            if (!_commit()) {
                c = static_cast<char_type>(traits_type::eof());
            }

            _clear_buffer();
        }

        return c;
    }

    constexpr
    void _clear_buffer() noexcept {
        m_buf_pos = 0;
    }

    [[nodiscard]] constexpr
    size_type _buf_size() const noexcept {
        return m_buf_pos;
    }

private:
    bool m_failure;
    streambuf_type* m_streambuf;
    size_type m_buf_pos;
    array_type m_buf;
};

}

#endif