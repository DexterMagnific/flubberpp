#include "flubberpp.h"

#include <list>
#include <set>
#include <cmath>
#include <algorithm>

#include "earcut.hpp"

// Make earcut library able to use our Point type
namespace mapbox {
namespace util {

template <>
struct nth<0, flubberpp::Point> {
    inline static auto get(const flubberpp::Point &t) {
        return t.x;
    };
};
template <>
struct nth<1, flubberpp::Point> {
    inline static auto get(const flubberpp::Point &t) {
        return t.y;
    };
};

} // namespace util
} // namespace mapbox

namespace flubberpp {

SingleInterpolator::SingleInterpolator(const VectorShape &from, const VectorShape &to, float maxSegmentLength)
  : mMsl(maxSegmentLength)
  , dirty(false)
{
  setStartShape(from);
  setEndShape(to);
}

SingleInterpolator::SingleInterpolator(float maxSegmentLength)
  : mMsl(maxSegmentLength)
{
}

void SingleInterpolator::setStartShape(const VectorShape &s)
{
  // switch to lists to alter shapes
  mFromList = ListShape(s.cbegin(),s.cend());
  mFromList.normalize(mMsl);
  dirty = true;
}

void SingleInterpolator::setEndShape(const VectorShape &s)
{
  // switch to lists to alter shapes
  mToList = ListShape(s.cbegin(),s.cend());
  mToList.normalize(mMsl);
  dirty = true;
}

const VectorShape &SingleInterpolator::at(float dt)
{
  if ( dirty )
    setup();

  auto it2 = mTo.cbegin();
  auto it3 = mCur.begin();

  for (auto it=mFrom.cbegin(); it!=mFrom.cend(); ++it, ++it2, ++it3) {
    const Point &a = *it;
    const Point &b = *it2;
    *it3 = Point {
        a.x + (b.x-a.x)*dt,
        a.y + (b.y-a.y)*dt
    };
  }

  return mCur;
}

void SingleInterpolator::setup()
{
  if ( dirty ) {
    if ( mFromList.size() > mToList.size() ) {
      mToList.addPoints(mFromList.size() - mToList.size());
    } else {
      mFromList.addPoints(mToList.size() - mFromList.size());
    }

    rotate(mFromList, mToList);

    // we keep a mCur list so that we don't perform an allocate each time at() is called
    // back to vectors
    mCur = mFrom = VectorShape(mFromList.cbegin(),mFromList.cend());
    mTo = VectorShape(mToList.cbegin(),mToList.cend());

    // we no longer need the lists
    mFromList.clear();
    mToList.clear();
  }
  dirty = false;
}

VectorShapeSet SingleInterpolator::triangulate(const VectorShape &s) const
{
  VectorShapeSet res;

  // earcut expects a polygon = main shape + hole1 + hole2 + ...
  // we assume we don't have holes
  std::vector<VectorShape> polygon;
  polygon.push_back(s);
  auto indices = mapbox::earcut(polygon);

  for (auto i=0; i<indices.size(); i+=3) {
    // Note: we have no holes so all indices lie within polygon[0]
    res.insert({ polygon[0][i], polygon[0][i+1], polygon[0][i+2]}); // sorted insert
  }

  return res;
}

void SingleInterpolator::rotate(ListShape &from, const ListShape &to)
{
  // unfortunately we need to change the beginning of the 'from' list.
  // so a vector is required
  const std::vector<Point> v(from.begin(),from.end());
  auto bestOffset = from.begin();
  unsigned offset = 0;
  float minDist = std::numeric_limits<float>::max();

  for (auto it=from.begin(); it!=from.end(); ++it, ++offset) {
    // for each start of the v list, check distances to 'to' list
    float dist = 0.f;
    unsigned i=0;
    for (auto it2=to.cbegin(); it2!=to.cend(); ++it2,++i) {
      const auto d = v[(offset+i)%from.size()].distance(*it2);
      dist += d*d;
    }

    if ( dist < minDist ) {
      minDist = dist;
      bestOffset = it;
    }
  }

  if ( bestOffset != from.begin() ) {
    std::rotate(from.begin(),bestOffset,from.end());
  }
}

}
