//
//  BMD split
//

#pragma once

#include <cstdint>
#include <vector>

union IntFloat64 {
    uint64_t i;
    double   f;
};

template <class T>
inline uint32_t encodeInt(std::vector<uint8_t>& buffer, uint32_t size, T value)
{
    for (uint32_t i = 0; i < size; ++i)
    {
        buffer.push_back(static_cast<uint8_t>(value >> 8 * (size - i - 1)));
    }

    return size;
}

inline uint32_t encodeDouble(std::vector<uint8_t>& buffer, double value)
{
    IntFloat64 intFloat64;
    intFloat64.f = value;

    uint64_t data = intFloat64.i;

    for (uint32_t i = 0; i < sizeof(double); ++i)
    {
        buffer.push_back(static_cast<uint8_t>(data >> 8 * (sizeof(value) - i - 1)));
    }
    
    return sizeof(double);
}
