#ifndef SFUTIL_HPP_INCLUDED
#define SFUTIL_HPP_INCLUDED SFUTIL_HPP_INCLUDED

#include <SFML/Graphics/Rect.hpp>
#include <SFML/System/Vector2.hpp>
#include <SFML/System/Vector3.hpp>
#include <SFML/Graphics/View.hpp>

#include <functional> // std::hash
#include <string>

namespace jd {

std::string const keyName(int keyCode);

// Taken from boost (not available in recent versions (non-standard))
template<typename T, typename H>
inline void hash_combine(
    std::size_t& seed,
    const T& t,
    const H h = std::hash<T>())
{
    seed ^= h(t) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
}

// Various math functions //

namespace math
{
    double const pi = 3.14159265;

    template <typename T>
    inline T rad(T degval)
    {
        return static_cast<T>(degval * pi / 180);
    }

    template <typename T>
    inline T deg(T radval)
    {
        return static_cast<T>(radval * 180 / pi);
    }

    template <typename T>
    inline T abs(sf::Vector2<T> v)
    {
        return std::sqrt(v.x * v.x + v.y * v.y);
    }

    template <typename T>
    inline T abs(sf::Vector3<T> v)
    {
        return std::sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
    }

    template <typename T>
    inline T angle(sf::Vector2<T> a, sf::Vector2<T> b = sf::Vector2<T>(1, 0))
    {
        return atan2(a.y, a.x) - atan2(b.y, b.x);
    }

    template <typename T>
    inline T operator* (sf::Vector2<T> v1, sf::Vector2<T> v2)
    {
        return scalar_product(v1, v2);
    }

    template <typename T>
    inline T scalar_product (sf::Vector2<T> v1, sf::Vector2<T> v2)
    {
        return v1.x * v2.x + v1.y * v2.y;
    }
} // namespace math


// sf::Rect utilty //

template <typename T>
inline T right(sf::Rect<T> const& r) { return r.left + r.width; }

template <typename T>
inline T bottom(sf::Rect<T> const& r) { return r.top + r.height; }
    
template <typename T>
inline sf::Vector2<T> topLeft(sf::Rect<T> const& r)
{
    return sf::Vector2<T>(r.left, r.top);
}

template <typename T>
inline sf::Vector2<T> topRight(sf::Rect<T> const& r)
{
    return sf::Vector2<T>(right(r), r.top);
}


template <typename T>
inline sf::Vector2<T> bottomRight(sf::Rect<T> const& r)
{
    return sf::Vector2<T>(right(r), bottom(r));
}

template <typename T>
inline sf::Vector2<T> bottomLeft(sf::Rect<T> const& r)
{
    return sf::Vector2<T>(r.left, bottom(r));
}


template <typename T>
inline sf::Vector2<T> size(sf::Rect<T> const& r)
{
    return sf::Vector2<T>(r.width, r.height);
}

template <typename T>
inline sf::Vector2<T> center(sf::Rect<T> const& r)
{
    return sf::Vector2<T>(r.left + r.width / 2, r.top + r.height / 2);
}

template <typename T>
inline sf::Rect<T> pointsToRect(sf::Vector2<T> topLeft, sf::Vector2<T> bottomRight)
{
    return sf::Rect<T>(topLeft, topLeft + bottomRight);
}

// sf::Vector utility //

template <typename T>
inline bool isZero(sf::Vector2<T> v)
{
    return v.x == 0 && v.y == 0;
}

template <typename T>
inline bool isZero(sf::Vector3<T> v)
{
    return v.x == 0 && v.y == 0 && v.z == 0;
}

template <typename T>
inline sf::Vector3<T> vec2to3(sf::Vector2<T> xy, T z = 0)
{
    return sf::Vector3<T>(xy.x, xy.y, z);
}

template <typename T>
inline sf::Vector2<T> vec3to2(sf::Vector3<T> xyz)
{
    return sf::Vector2<T>(xyz.x, xyz.y);
}

template<typename Target, typename Source>
inline sf::Vector2<Target> vec_cast(sf::Vector2<Source> source)
{
    return sf::Vector2<Target>(
        static_cast<Target>(source.x),
        static_cast<Target>(source.y)
    );
}

template<typename Target, typename Source>
inline sf::Vector3<Target> vec_cast(sf::Vector3<Source> source)
{
    return sf::Vector3<Target>(
        static_cast<Target>(source.x),
        static_cast<Target>(source.y),
        static_cast<Target>(source.z)
    );
}

template <template<typename> class V, typename T>
inline T distance(V<T> a, V<T> b)
{
    return math::abs(b - a);
}


template <template<typename> class V, typename T>
inline T manhattanDistance(V<T> a, V<T> b)
{
    auto const d = b - a;
    return std::abs(d.x) + std::abs(d.y);
}

template <typename T>
sf::Vector2<T> nearestPoint(sf::Rect<T> in, sf::Vector2<T> to)
{
    sf::Vector2<T> result = to;
    if (to.x < in.left)
        result.x = in.left;
    else if (to.x > right(in))
        result.x = right(in);
    if (to.y < in.top)
        to.y = in.top;
    else if (to.y > bottom(in))
        to.y = bottom(in);
    return result;
}

template <typename T>
sf::Vector2<T> outermostPoint(sf::Rect<T> in, sf::Vector2<T> d, sf::Vector2<T> from)
{
    return sf::Vector2<T>(
        d.x == 0 ? from.x : d.x < 0 ? in.left : right(in),
        d.y == 0 ? from.y : d.y < 0 ? in.top  : bottom(in));
}

template <typename T>
sf::Vector2<T> outermostPoint(sf::Rect<T> in, sf::Vector2<T> d, sf::Rect<T> from)
{
    return sf::Vector2<T>(
        d.x == 0 ? from.left : d.x < 0 ? in.left : right(in)  - from.width,
        d.y == 0 ? from.top  : d.y < 0 ? in.top  : bottom(in) - from.height);
}



// Clipping and line-rectangle intersection //
// (using Cohen–Sutherland algorithm)
namespace clip {
    enum T {left  = 1, right = 2, lower = 4, upper = 8};
}

template <typename T>
unsigned clipflags(sf::Vector2<T> p, sf::Rect<T> const& r)
{
    unsigned k = 0;

    if      (p.y > bottom(r)) k  = clip::lower;
    else if (p.y < r.top    ) k  = clip::upper;
    if      (p.x < r.left   ) k |= clip::left ;
    else if (p.x > right(r) ) k |= clip::right;

    return k;
}

template <typename T>
void clipPoint(sf::Vector2<T>& p, sf::Vector2<T> d, sf::Rect<T> r, unsigned& k)
{
    if (k & clip::left) {
        p.y += (r.left    - p.x) * d.y / d.x;
        p.x  =  r.left;
    } else if (k & clip::right) {
        p.y += (right(r)  - p.x) * d.y / d.x;
        p.x  =  right(r);
    }
    if (k & clip::lower) {
        p.x += (bottom(r) - p.y) * d.x / d.y;
        p.y  =  bottom(r);
    } else if (k & clip::upper) {
        p.x += (r.top     - p.y) * d.x / d.y;
        p.y  =  r.top;
    }
    k = clipflags(p, r);
}

template <typename T>
bool clipLine(
    sf::Vector2<T>& p1, sf::Vector2<T>& p2,
    sf::Rect<T> r)
{
    
    unsigned k1 = clipflags(p1, r), k2 = clipflags(p2, r);

    sf::Vector2<T> const d = p2 - p1;
    while (k1 || k2) { // at most two cycles
        if (k1 & k2) // both outside on same side(s) ?
            return false;
        
        if (k1) {
            clipPoint(p1, d, r, k1);
            if (k1 & k2)
                return false;
        }
        if (k2)
            clipPoint(p2, d, r, k2);
    }
    return true;
}

template<typename T>
bool intersection(
    sf::Vector2<T> p1, sf::Vector2<T> p2,
    sf::Rect<T> r)
{
    sf::Vector2<T> inside1 = p1, inside2 = p2;
    return clipLine(p1, p2, r);
}

// sf::View utility //

sf::FloatRect viewRect(sf::View const& view);
void setViewRect(sf::View& view, sf::FloatRect const& newRect);


} // namespace jd

// std::hash support for sf::Vector2<T>, sf::Vector3<T> and sf::Rect<T>

namespace std {
template <typename T>
struct hash<sf::Rect<T>>: public unary_function<sf::Rect<T>, size_t> {
    size_t operator() (sf::Rect<T> const& r) const
    {
        hash<T> hasher;
        size_t result = hasher(r.left);
        jd::hash_combine(result, r.top, hasher);
        jd::hash_combine(result, r.width, hasher);
        jd::hash_combine(result, r.height, hasher);
        return result;
    }
};

template<typename T>
struct hash<sf::Vector2<T>>: public unary_function<sf::Vector2<T>, size_t> {
    size_t operator() (sf::Vector2<T> const& v) const
    {
        hash<T> hasher;
        size_t result = hasher(v.x);
        jd::hash_combine(result, v.y, hasher); 
        return result;
    }
};

template<typename T>
struct hash<sf::Vector3<T>>: public unary_function<sf::Vector3<T>, size_t> {
    size_t operator() (sf::Vector3<T> const& v) const
    {
        hash<T> hasher;
        size_t result = hasher(v.x);
        jd::hash_combine(result, v.y, hasher); 
        jd::hash_combine(result, v.z, hasher); 
        return result;
    }
};

} // namespace std

#endif // include guard
