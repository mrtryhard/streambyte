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
#include <cassert>
#include <cstddef>
#include <sstream>
#include <string>
#include <vector>

constexpr const auto MAX_BYTES = 50;

std::vector<std::byte> get_expected() {
    std::vector<std::byte> expected_bytes;
    expected_bytes.reserve(MAX_BYTES);

    for (auto i = 0; i < MAX_BYTES; ++i) {
        expected_bytes.push_back( static_cast<std::byte>(i % 10) );
    }

    return expected_bytes;
}

int main() {
    // Case 1: ostreambyte_iterator
    {
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

        assert(resulting_bytes.size() == MAX_BYTES && "ostreambyte_iterator: Expected result do not match length.");
        assert(are_equals && "ostreambyte_iterator: Expected results mismatch.");
    }

    // Case 2: istreambyte_iterator
    {
        std::stringstream ss;
        ss << "012345674444234567890";
        const std::string expected_bytes = ss.str();

        std::vector<std::byte> resulting_bytes(mrt::istreambyte_iterator{ss},
            mrt::istreambyte_iterator{});
        
        auto are_equals = std::equal(
            std::begin(expected_bytes), std::end(expected_bytes),
            std::begin(resulting_bytes), std::end(resulting_bytes), 
            [](const char& left, const std::byte& right) {
                return left == static_cast<int>(right);
            }
        );

        assert(resulting_bytes.size() == expected_bytes.size() && "istreambyte_iterator: Expected result do not match length.");
        assert(are_equals && "istreambyte_iterator: Expected results mismatch.");
    }

    return 0;
}