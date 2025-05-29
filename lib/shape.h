#pragma once

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4251)
#endif

#ifdef FLUBBERPP_LIBRARY
#if _WIN32
#define FLUBBERPP_EXPORT __declspec(dllexport)
#else
#define FLUBBERPP_EXPORT __attribute__((visibility("default")))
#endif
#else
#define FLUBBERPP_EXPORT
#endif

#include <list>
#include <vector>
#include <set>
#include <cmath>
#include <type_traits>

namespace flubberpp {

/** A 2D point */
struct FLUBBERPP_EXPORT Point {
    float x,y;

    /** Distance to another point */
    float distance(const Point &other) const { return std::sqrt((other.x-x)*(other.x-x) + (other.y-y)*(other.y-y)); }
    /** Point between this and another point, from 0 to 1 */
    Point pointAlong(const Point &b, float dt) const {
      return Point {
        x + (b.x-x)*dt,
        y + (b.y-y)*dt
      };
    }
    /** Nearly same point */
    bool operator ==(const Point &other) const { return this->distance(other) < 1e-4f; }
    /** Compare  points */
    bool operator < (const Point& other) const { return std::tie(x, y) < std::tie(other.x, other.y); }
};

inline Point operator+(const Point &a, const Point &b) { return Point{a.x+b.x,a.y+b.y}; }
inline Point operator-(const Point &a, const Point &b) { return Point{a.x-b.x,a.y-b.y}; }

/** A shape that store its points using the provided template container.
 *  (e.g. Shape<std::vector> will store the points as a vector
 */
template< template<typename,typename> class Container >
struct FLUBBERPP_EXPORT Shape : Container<Point, std::allocator<Point>> {
    using ContainerType = Container<Point, std::allocator<Point>>;
    using IteratorCategory = typename std::iterator_traits<typename ContainerType::iterator>::iterator_category;
    using random = std::is_same<
      IteratorCategory,
      std::random_access_iterator_tag>;

    // same ctors
    using Container<Point, std::allocator<Point>>::Container;

    /** Shape area */
    float area() const {
      if ( this->size() <= 2 )
        return 0.f;

      float area = 0.f;
      Point a;
      Point b = *(--this->cend());

      for (auto it=this->cbegin(); it!=this->cend(); ++it) {
        a = b;
        b = *it;
        area += a.y*b.x - a.x*b.y;
      }
      return area / 2.f;
    }
    /** Shape perimeter */
    float length() const {
      if ( this->size() <= 1 )
        return 0.f;

      float peri = 0.f;

      auto it = this->cbegin();
      for (auto i=0; i<this->size()-1; i++) {
        peri += it->distance(*std::next(it));
        ++it;
      }
      peri += (--this->cend())->distance(*this->cbegin());
      return peri;
    }
    /** Add nb points to the shape, uniformly distributed among its length
     *  Only available for non random access containers
     */
    void addPoints(unsigned nb) {
      static_assert(!random::value, "Random access containers are not supported by this method");

      if constexpr(!random::value) {
        const auto target = this->size() + nb;
        const auto step = this->length() / nb;

        float cursor = 0.f;
        float insertAt = step / 2.f;

        auto it = this->cbegin();
        while (this->size() < target ) {
          const Point &a = *it;
          const Point &b = it == --this->cend() ? *this->cbegin() : *std::next(it);
          const float segment = a.distance(b);
          if ( insertAt <= cursor+segment ) {
            this->insert(std::next(it), segment ? a.pointAlong(b, (insertAt-cursor)/segment) : *this->cbegin());
            insertAt += step;
            continue;
          }

          cursor += segment;
          ++it;
        }
      }
    }
    /** Make each segment of the shape at most @c seglen length by breaking
     *  long segments into smaller ones
     */
    void normalize(float seglen) {
      static_assert(!random::value, "Random access containers are not supported by this method");

      if constexpr(!random::value) {
        const auto area = this->area();
        if ( area < 0 ) {
          this->reverse();
        }

        for (auto it=this->cbegin(); it!=this->cend(); ++it) {
          const Point &a = *it;
          Point b = it == --this->cend() ? *this->cbegin() : *std::next(it); // next point or loop to first

          while ( a.distance(b) > seglen ) {
            b = a.pointAlong(b, 0.5f);
            this->insert(std::next(it), b);
          }
        }
      }
    }
};

template <typename T>
inline bool lessArea(const T &s1, const T &s2) {
  return s1.area() < s2.area();
}

/** A Shape whose underlying storage is a std::vector of points. Usually used by the user */
using VectorShape = Shape<std::vector>;
/** A Shape whose underlying storage is a std::list of points. Used internally to alter the shapes */
using ListShape   = Shape<std::list>;

template <typename T> using ShapeSet = std::multiset<T, decltype(lessArea<T>)*>;
/** A set of shapes sorted wrt their areas, vector or list */
using VectorShapeSet = ShapeSet<VectorShape>;
using ListShapeSet   = ShapeSet<ListShape>;

};
