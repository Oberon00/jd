#ifndef BASE64_HPP_INCLUDED
#define BASE64_HPP_INCLUDED BASE64_HPP_INCLUDED

#include <vector>
#include <cstdint>
#include <stdexcept>
#include <string>

namespace base64 {
    class Error: public std::runtime_error {
    public:
        Error(char const* what): std::runtime_error(what) { }
    };

    class InvalidLength: public Error {
    public:
        InvalidLength(): Error("Invalid base64 data: Lenght not divisible by 4.") { }
    };

    class InvalidCharacter: public Error {
    public:
        InvalidCharacter(): Error("Invalid base64 data: Invalid character.") { }
    };


    typedef char byte;
    std::vector<byte> decode(byte const* encoded, std::size_t byteCount);

    std::vector<byte> decode(char const* encoded); // encoded must be null-terminated
    std::vector<byte> decode(std::vector<byte> const& encoded);
    std::vector<byte> decode(std::string const& encoded);
} // namespace base64


#endif
