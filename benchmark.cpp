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
#include <chrono>
#include <cmath>
#include <cstddef>
#include <fstream>
#include <functional>
#include <iostream>
#include <iterator>
#include <numeric>
#include <sstream>
#include <vector>

// Tests parameters
static constexpr const int g_tests_batch_count = 100;
static constexpr const int g_bytes_count = 500'000;

static
void print_abstract() {
    std::cout << "This utility tests against:\r\n\t"
        << "- i/ostream_iterator<char>\r\n\t"
        << "- i/ostreambuf_iterator<char>\r\n\t"
        << "- i/ostreambyte_iterator<32>\r\n\t"
        << "- i/ostreambyte_iterator<64>\r\n\t"
        << "- i/ostreambyte_iterator<128>\r\n\t"
        << "- i/ostreambyte_iterator<256>\r\n\t"
        << "- i/ostreambyte_iterator<512>\r\n\t"
        << "- i/ostreambyte_iterator<1024>\r\n"
        << std::endl;
    
    std::cout << "Note that, istreambyte does no formatting, unlike i/ostream_iterator.\r\n" << std::endl;
}

// Testing utilities (timing, batching)
namespace utils {

    void print_result(std::uint64_t median, std::uint64_t sum, std::uint64_t average) {
        std::cout << "average=" << average 
            << ", sum=" << sum 
            << ", median=" << median;
    }

    template<typename Function>
    auto test(Function&& f) {
        using namespace std;

        auto start = chrono::high_resolution_clock::now();
        f();
        auto end = chrono::high_resolution_clock::now();

        return chrono::duration_cast<chrono::nanoseconds>(end - start).count();
    }

    template<typename Function>
    void batch_test() {
        std::vector<std::uint64_t> durations;
        durations.reserve(g_tests_batch_count);

        std::generate_n(std::back_inserter(durations), g_tests_batch_count, []() { return test(Function{}); });

        // Calculations
        std::sort(std::begin(durations), std::end(durations));

        auto median = static_cast<std::uint64_t>( (durations[std::ceil(g_tests_batch_count / 2)] + durations[std::floor(g_tests_batch_count / 2)]) / 2 );
        std::uint64_t sum = std::accumulate(std::begin(durations), std::end(durations), 0, std::plus<std::uint64_t>{});
        auto average = static_cast<std::uint64_t>(sum / g_tests_batch_count);

        print_result(median, sum, average);
    }

}

// Actual operations
namespace operation {
    static std::vector<std::byte> data_to_write;

    std::vector<std::byte> get_data_to_write() {
        if (data_to_write.empty()) {
            data_to_write.reserve(g_bytes_count);

            // Byte: 0..9
            for (auto i = 0; i < g_bytes_count; ++i) {
                data_to_write.push_back( static_cast<std::byte>(i % 10) );
            }
        }

        return data_to_write;
    }

    struct ostream {
        std::vector<char> m_data;
        std::ofstream m_stream;

        ostream() : m_stream("ostream.testfile", std::ios_base::binary | std::ios_base::trunc) {
            auto data = get_data_to_write();
            m_data.reserve(data.size());
            for (auto c : data) {
                m_data.push_back(static_cast<char>(c));
            }
        }

        void operator()() {
            std::copy(std::begin(m_data),
                std::end(m_data), 
                std::ostream_iterator<char>(m_stream));
        }
    };

    struct ostreambuf {
        std::vector<char> m_data;
        std::ofstream m_stream;

        ostreambuf() : m_stream("ostreambuf.testfile", std::ios_base::binary | std::ios_base::trunc) {
            auto data = get_data_to_write();
            m_data.reserve(data.size());
            for (auto c : data) {
                m_data.push_back(static_cast<char>(c));
            }
        }

        void operator()() {
            std::copy(std::begin(m_data),
                std::end(m_data), 
                std::ostreambuf_iterator<char>(m_stream));
        }
    };

    struct ostreambyte_32 {
        std::vector<std::byte> m_data;
        std::ofstream m_stream;
        constexpr static const std::size_t m_size = 32;

        ostreambyte_32() : m_stream("ostreambyte_32.testfile", std::ios_base::binary | std::ios_base::trunc) {
            m_data = get_data_to_write();
        }

        void operator()() {
            std::copy(std::begin(m_data),
                std::end(m_data), 
                mrt::ostreambyte_iterator<m_size>(m_stream));
        }
    };

    struct ostreambyte_64 {
        std::vector<std::byte> m_data;
        std::ofstream m_stream;
        constexpr static const std::size_t m_size = 64;

        ostreambyte_64() : m_stream("ostreambyte_64.testfile", std::ios_base::binary | std::ios_base::trunc) {
            m_data = get_data_to_write();
        }

        void operator()() {
            std::copy(std::begin(m_data),
                std::end(m_data), 
                mrt::ostreambyte_iterator<m_size>(m_stream));
        }
    };

    struct ostreambyte_128 {
        std::vector<std::byte> m_data;
        std::ofstream m_stream;
        constexpr static const std::size_t m_size = 128;

        ostreambyte_128() : m_stream("ostreambyte_128.testfile", std::ios_base::binary | std::ios_base::trunc) {
            m_data = get_data_to_write();
        }

        void operator()() {
            std::copy(std::begin(m_data),
                std::end(m_data), 
                mrt::ostreambyte_iterator<m_size>(m_stream));
        }
    };

    struct ostreambyte_256 {
        std::vector<std::byte> m_data;
        std::ofstream m_stream;
        constexpr static const std::size_t m_size = 256;

        ostreambyte_256() : m_stream("ostreambyte_256.testfile", std::ios_base::binary | std::ios_base::trunc) {
            m_data = get_data_to_write();
        }

        void operator()() {
            std::copy(std::begin(m_data),
                std::end(m_data), 
                mrt::ostreambyte_iterator<m_size>(m_stream));
        }
    };

    struct ostreambyte_512 {
        std::vector<std::byte> m_data;
        std::ofstream m_stream;
        constexpr static const std::size_t m_size = 512;

        ostreambyte_512() : m_stream("ostreambyte_512.testfile", std::ios_base::binary | std::ios_base::trunc) {
            m_data = get_data_to_write();
        }

        void operator()() {
            std::copy(std::begin(m_data),
                std::end(m_data), 
                mrt::ostreambyte_iterator<m_size>(m_stream));
        }
    };

    struct ostreambyte_1024 {
        std::vector<std::byte> m_data;
        std::ofstream m_stream;
        constexpr static const std::size_t m_size = 1024;

        ostreambyte_1024() : m_stream("ostreambyte_1024.testfile", std::ios_base::binary | std::ios_base::trunc) {
            m_data = get_data_to_write();
        }

        void operator()() {
            std::copy(std::begin(m_data),
                std::end(m_data), 
                mrt::ostreambyte_iterator<m_size>(m_stream));
        }
    };

    // istream - Requires ostream to run beforehand
    struct istream {
        std::vector<char> m_data;
        std::ifstream m_stream;

        istream() : m_stream("ostream.testfile", std::ios_base::binary) {
            m_data.reserve(g_bytes_count);
        }

        void operator()() {
            std::copy(std::istream_iterator<char>(m_stream), 
                std::istream_iterator<char>(), 
                std::back_inserter(m_data));
        }
    };

    struct istreambuf {
        std::vector<char> m_data;
        std::ifstream m_stream;

        istreambuf() : m_stream("ostreambuf.testfile", std::ios_base::binary) {
            m_data.reserve(g_bytes_count);
        }

        void operator()() {
            std::copy(std::istreambuf_iterator<char>(m_stream), 
                std::istreambuf_iterator<char>(), 
                std::back_inserter(m_data));
        }
    };

    struct istreambyte_32 {
        std::vector<std::byte> m_data;
        std::ifstream m_stream;
        constexpr static const std::size_t m_size = 32;

        istreambyte_32() : m_stream("ostreambyte_32.testfile", std::ios_base::binary) {
            m_data.reserve(g_bytes_count);
        }

        void operator()() {
            std::copy(mrt::istreambyte_iterator<m_size>(m_stream), 
                mrt::istreambyte_iterator<m_size>(), 
                std::back_inserter(m_data));
        }
    };

    struct istreambyte_64 {
        std::vector<std::byte> m_data;
        std::ifstream m_stream;
        constexpr static const std::size_t m_size = 64;

        istreambyte_64() : m_stream("ostreambyte_64.testfile", std::ios_base::binary) {
            m_data.reserve(g_bytes_count);
        }

        void operator()() {
            std::copy(mrt::istreambyte_iterator<m_size>(m_stream), 
                mrt::istreambyte_iterator<m_size>(), 
                std::back_inserter(m_data));
        }
    };

    struct istreambyte_128 {
        std::vector<std::byte> m_data;
        std::ifstream m_stream;
        constexpr static const std::size_t m_size = 128;

        istreambyte_128() : m_stream("ostreambyte_128.testfile", std::ios_base::binary) {
            m_data.reserve(g_bytes_count);
        }

        void operator()() {
            std::copy(mrt::istreambyte_iterator<m_size>(m_stream), 
                mrt::istreambyte_iterator<m_size>(), 
                std::back_inserter(m_data));
        }
    };

    struct istreambyte_256 {
        std::vector<std::byte> m_data;
        std::ifstream m_stream;
        constexpr static const std::size_t m_size = 256;

        istreambyte_256() : m_stream("ostreambyte_256.testfile", std::ios_base::binary) {
            m_data.reserve(g_bytes_count);
        }

        void operator()() {
            std::copy(mrt::istreambyte_iterator<m_size>(m_stream), 
                mrt::istreambyte_iterator<m_size>(), 
                std::back_inserter(m_data));
        }
    };

    struct istreambyte_512 {
        std::vector<std::byte> m_data;
        std::ifstream m_stream;
        constexpr static const std::size_t m_size = 512;

        istreambyte_512() : m_stream("ostreambyte_512.testfile", std::ios_base::binary) {
            m_data.reserve(g_bytes_count);
        }

        void operator()() {
            std::copy(mrt::istreambyte_iterator<m_size>(m_stream), 
                mrt::istreambyte_iterator<m_size>(), 
                std::back_inserter(m_data));
        }
    };

    struct istreambyte_1024 {
        std::vector<std::byte> m_data;
        std::ifstream m_stream;
        constexpr static const std::size_t m_size = 1024;

        istreambyte_1024() : m_stream("ostreambyte_1024.testfile", std::ios_base::binary) {
            m_data.reserve(g_bytes_count);
        }

        void operator()() {
            std::copy(mrt::istreambyte_iterator<m_size>(m_stream), 
                mrt::istreambyte_iterator<m_size>(), 
                std::back_inserter(m_data));
        }
    };
}

// Ease of use bootstrap to call from main
namespace bootstrap {
    void ostream() {
        std::cout << "Operation: ostream_iterator:\r\n\t";
        utils::batch_test<operation::ostream>();
        std::cout << std::endl;
    }

    void ostreambuf() {
        std::cout << "Operation: ostreambuf:\r\n\t";
        utils::batch_test<operation::ostreambuf>();
        std::cout << std::endl;
    }

    void ostreambyte_32() {
        std::cout << "Operation: ostreambyte_32:\r\n\t";
        utils::batch_test<operation::ostreambyte_32>();
        std::cout << std::endl;
    }

    void ostreambyte_64() {
        std::cout << "Operation: ostreambyte_64:\r\n\t";
        utils::batch_test<operation::ostreambyte_64>();
        std::cout << std::endl;
    }

    void ostreambyte_128() {
        std::cout << "Operation: ostreambyte_128:\r\n\t";
        utils::batch_test<operation::ostreambyte_128>();
        std::cout << std::endl;
    }

    void ostreambyte_256() {
        std::cout << "Operation: ostreambyte_256:\r\n\t";
        utils::batch_test<operation::ostreambyte_256>();
        std::cout << std::endl;
    }

    void ostreambyte_512() {
        std::cout << "Operation: ostreambyte_512:\r\n\t";
        utils::batch_test<operation::ostreambyte_512>();
        std::cout << std::endl;
    }

    void ostreambyte_1024() {
        std::cout << "Operation: ostreambyte_1024:\r\n\t";
        utils::batch_test<operation::ostreambyte_1024>();
        std::cout << std::endl;
    }

    void istream() {
        std::cout << "Operation: istream:\r\n\t";
        utils::batch_test<operation::istream>();
        std::cout << std::endl;
    }

    void istreambuf() {
        std::cout << "Operation: istreambuf:\r\n\t";
        utils::batch_test<operation::istreambuf>();
        std::cout << std::endl;
    }

    void istreambyte_32() {
        std::cout << "Operation: istreambyte_32:\r\n\t";
        utils::batch_test<operation::istreambyte_32>();
        std::cout << std::endl;
    }

    void istreambyte_64() {
        std::cout << "Operation: istreambyte_64:\r\n\t";
        utils::batch_test<operation::istreambyte_64>();
        std::cout << std::endl;
    }

    void istreambyte_128() {
        std::cout << "Operation: istreambyte_128:\r\n\t";
        utils::batch_test<operation::istreambyte_128>();
        std::cout << std::endl;
    }

    void istreambyte_256() {
        std::cout << "Operation: istreambyte_256:\r\n\t";
        utils::batch_test<operation::istreambyte_256>();
        std::cout << std::endl;
    }

    void istreambyte_512() {
        std::cout << "Operation: istreambyte_512:\r\n\t";
        utils::batch_test<operation::istreambyte_512>();
        std::cout << std::endl;
    }

    void istreambyte_1024() {
        std::cout << "Operation: istreambyte_1024:\r\n\t";
        utils::batch_test<operation::istreambyte_1024>();
        std::cout << std::endl;
    }
}

int main() {
    print_abstract();

    bootstrap::ostream();
    bootstrap::ostreambuf();
    bootstrap::ostreambyte_32();
    bootstrap::ostreambyte_64();
    bootstrap::ostreambyte_128();
    bootstrap::ostreambyte_256();
    bootstrap::ostreambyte_512();
    bootstrap::ostreambyte_1024();

    std::cout << "\r\n" << std::endl;

    bootstrap::istream();
    bootstrap::istreambuf();
    bootstrap::istreambyte_32();
    bootstrap::istreambyte_64();
    bootstrap::istreambyte_128();
    bootstrap::istreambyte_256();
    bootstrap::istreambyte_512();
    bootstrap::istreambyte_1024();

    return 0;
}