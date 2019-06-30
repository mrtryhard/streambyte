[![Build Status](https://travis-ci.com/mrtryhard/streambyte.svg?branch=master)](https://travis-ci.com/mrtryhard/streambyte)

# I/O stream iterators surpport for C++ std::bytes
C++ has introduced the type `std::byte`. However, it is tedious to read / write to file using this type.
This library leverage the iterators case and also offer a buffering system, rather than a byte-to-byte
approach.

## Example
### Copying file content into vector of bytes
```
std::ifstream file("test", std::ios_base::binary);
std::vector<std::byte> vec(mrt::istreambyte_iterator(file), mrt::istreambyte_iterator());
```

### Copying the content of a vector into a file
```
std::vector<std::byte> my_vec;
//...

ofstream file("test", std::ios_base::binary | std::ios_base::trunc);
std::copy(std::begin(my_vec), std::end(my_vec), mrt::ostreambyte_iterator(file));
```

## License
See LICENSE.md. Spoilers: it's MIT.

