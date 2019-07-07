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

#include "streambyte.hpp"
 
#include <algorithm>
#include <cstddef>
#include <iostream>
#include <sstream>
#include <string>
#include <string_view>
#include <vector>

#define expect(x, y) expect_impl(x, y, __LINE__)

static int g_final_result = 0;

constexpr const auto MAX_BYTES = 50;

std::vector<std::byte> get_expected() {
    std::vector<std::byte> expected_bytes;
    expected_bytes.reserve(MAX_BYTES);

    for (auto i = 0; i < MAX_BYTES; ++i) {
        expected_bytes.push_back( static_cast<std::byte>(i % 10) );
    }

    return expected_bytes;
}

static
void expect_impl(bool expression, std::string_view error_msg, int line) {
    if (!expression) {
        std::cerr << "Failure at line " << line << ":\r\n\t" << error_msg << std::endl;
        ++g_final_result;
    }
}

static
auto status() {
    if (g_final_result == 0) {
        std::cout << "success";
    } else {
        std::cout << "failure";
    }

    std::cout << std::endl;
}

// This tests the integrity of the iterators.
int main() {
    std::cout << "Starting testing..." << std::endl;

    // Case 1: ostreambyte_iterator
    {
        std::cout << "ostreambyte_iterator ";
        const std::vector<std::byte> expected_bytes = get_expected();

        std::stringstream stream;
        std::copy(std::begin(expected_bytes),
            std::end(expected_bytes), 
            mrt::ostreambyte_iterator(stream));

        std::string resulting_bytes = stream.str();

        auto are_equals = std::equal(
            std::begin(expected_bytes), std::end(expected_bytes),
            std::begin(resulting_bytes), std::end(resulting_bytes), 
            [](const std::byte& left, const char& right) {
                return right == static_cast<int>(left);
            }
        );

        expect(resulting_bytes.size() == MAX_BYTES, "ostreambyte_iterator: Expected result do not match length.");
        expect(are_equals, "ostreambyte_iterator: Expected results mismatch.");

        status();
    }

    // Case 2: istreambyte_iterator
    {
        std::cout << "istreambyte_iterator ";
        std::stringstream ss;
        ss << "012345674444234567890";
        const std::string expected_bytes = ss.str();

        std::vector<std::byte> resulting_bytes(mrt::istreambyte_iterator<>{ss}, mrt::istreambyte_iterator<>{});

        auto are_equals = std::equal(
            std::begin(expected_bytes), std::end(expected_bytes),
            std::begin(resulting_bytes), std::end(resulting_bytes), 
            [](const char& left, const std::byte& right) {
                return left == static_cast<int>(right);
            }
        );

        expect(resulting_bytes.size() == expected_bytes.size(), "istreambyte_iterator: Expected result do not match length.");
        expect(are_equals, "istreambyte_iterator: Expected results mismatch.");

        status();
    }

    // Case 3: istreambyte_iterator with count of elements (e.g. copy_n)
    //         Since istreambyte uses a buffer, if you copy_n(6), you'll still read up to buf_size bytes internally.
    //         You need to restore those bytes on iterator death.
    {
        // Setup
        std::cout << "istreambyte_iterator: lower than buffer length count usage\r\n";
        const std::size_t expected_count = 6;
        std::stringstream ss;
        ss << "012345674444234567890";

        std::vector<std::byte> resulting_bytes;
        std::string expected_bytes;
        expected_bytes.reserve(expected_count);
        resulting_bytes.reserve(expected_count);

        // Fill buffers.
        std::copy_n(std::istreambuf_iterator(ss), expected_count, std::back_inserter(expected_bytes));
        ss.seekg(0);

        std::copy_n(mrt::istreambyte_iterator<>{ss}, expected_count, std::back_inserter(resulting_bytes));
        const auto seek_position = ss.tellg();

        auto are_equals = std::equal(
            std::begin(expected_bytes), std::end(expected_bytes),
            std::begin(resulting_bytes), std::end(resulting_bytes),
            [](const char& left, const std::byte& right) {
                return left == static_cast<int>(right);
            }
        );

        expect(seek_position == expected_count, "Stream seek position should've been restored to right position.");
        expect(resulting_bytes.size() == expected_bytes.size(), "istreambyte_iterator: Expected result do not match length.");
        expect(are_equals, "istreambyte_iterator: Expected results mismatch.");

        status();
    }

    return g_final_result == 0 ? 0 : -g_final_result;
}