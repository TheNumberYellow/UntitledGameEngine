#pragma once

#include <cstddef>

#include <functional>

#include "Math/Geometry.h"

namespace Hash
{
    template <typename T>
    struct Hasher
    {
        size_t operator()(const T& v) const
        {
            return Hash_Value(v);
        }
    };

    // Stolen from Boost
    inline size_t Combine(size_t h, size_t k)
    {
        const size_t m = (size_t(0xc6a4a793) << 32) + 0x5bd1e995;
        const int r = 47;

        k *= m;
        k ^= k >> r;
        k *= m;

        h ^= k;
        h *= m;

        // Completely arbitrary number, to prevent 0's
        // from hashing to 0.
        h += 0xe6546b64;

        return h;
    }

    template <typename T>
    inline size_t Hash_Value(T v)
    {
        return std::hash<T>{}(v);
    }

    inline size_t Hash_Value(Vec2f v)
    {
        size_t h = Hash_Value(v.x);
        return Combine(h, Hash_Value(v.y));
    }

    inline size_t Hash_Value(Vec3f v)
    {
        size_t h = Hash_Value(v.x);
        h = Combine(h, Hash_Value(v.y));
        return Combine(h, Hash_Value(v.z));
    }

    inline size_t Hash_Value(Rect v)
    {
        size_t h = Hash_Value(v.location);
        return Combine(h, Hash_Value(v.size));
    }
}