// Part of the Jade Engine -- Copyright (c) Christian Neumüller 2012--2013
// This file is subject to the terms of the BSD 2-Clause License.
// See LICENSE.txt or http://opensource.org/licenses/BSD-2-Clause

#include "base64.hpp"

#include <array>
#include <algorithm>
#include <cassert>
#include <cstdint>
#include <cstring>

namespace {
static base64::byte charToByte(base64::byte c)
{
    switch(c) {
        case '+': return 62;
        case '/': return 63;
        case '=': return -1;
        default:
            if (c >= '0' && c <= '9')
                return c - '0' + 52;
            if (c >= 'A' && c <= 'Z')
                return c - 'A';
            if (c >= 'a' && c <= 'z')
                return c - 'a' + 26;
            throw base64::InvalidCharacter();
        }
}
} // anonymous namespace

namespace base64 {

std::vector<byte> decode(char const* encoded)
{
    return decode(encoded, std::strlen(encoded));
}

std::vector<byte> decode(std::vector<byte> const& encoded)
{
    if (encoded.empty())
        return std::vector<byte>();
    return decode(&encoded[0], encoded.size());
}

std::vector<byte> decode(std::string const& encoded)
{
    return decode(encoded.data(), encoded.size());
}


// Could be optimized: call transform once on encoded
std::vector<byte> decode(byte const* encoded, std::size_t byteCount)
{
    std::vector<byte> result;

    if (byteCount == 0)
        return result;

    assert(encoded);

    if (byteCount % 4 != 0)
        throw InvalidLength();

    result.reserve(byteCount * 3 / 4 + 2);
    for (std::size_t i = 0; i < byteCount; i += 4) {

        std::array<byte, 4> inBytes;
        std::transform(
            encoded + i, encoded + i + 4,
            inBytes.begin(),
            charToByte);

        std::array<byte, 3> outBytes;
        outBytes[0] = inBytes[0] << '\2' | inBytes[1] >> '\4';
        outBytes[1] = inBytes[1] << '\4' | inBytes[2] >> '\2';
        outBytes[2] = inBytes[2] << '\6' | inBytes[3];

        if (i + 4 == byteCount) {
            std::size_t discard = 0;
            if (inBytes[2] == -1) {
                ++discard;
                if (inBytes[1] == -1)
                    ++discard;
            } else if (inBytes[1] == -1) {
                throw InvalidCharacter();
            }
            result.insert(result.end(), outBytes.begin(), outBytes.end() - discard);
        } else if (inBytes[0] == -1 || inBytes[1] == -1) {
            throw InvalidCharacter();
        } else {
            result.insert(result.end(), outBytes.begin(), outBytes.end());
        }
    }
    return result;
}

} // namespace base64
