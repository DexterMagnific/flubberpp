#pragma once

#ifdef FLUBBERPP_LIBRARY
#define FLUBBERPP_EXPORT __attribute__((visibility("default")))
#else
#define FLUBBERPP_EXPORT
#endif

#include "shape.h"

namespace flubberpp {

/** One to One shape interpolator */
class FLUBBERPP_EXPORT SingleInterpolator {
  public:
    /** Builds a shape interpolator starting from shape 'from' and ending in shape 'to' */
    SingleInterpolator(const VectorShape &from, const VectorShape &to, float maxSegmentLength = 10.f);

    SingleInterpolator(float maxSegmentLength = 10.f);

    void setStartShape(const VectorShape &s);
    void setEndShape(const VectorShape &s);

    /** Returns the interpolated shape at time dt between 0 and 1 */
    const VectorShape &at(float dt);

  private:
    void setup();

    /** Rotates the 'from' shape so as to minimize the sum of square distances
     *  between its points and the points of the 'to' shape
     *  This is used to reorder the points of the 'from' shape
     *  so that overall the distance traveled between points of the
     *  'from' shape to reach 'to' shape is minimal
     */
    void rotate(ListShape &from, const ListShape &to);

    /** Cuts the shape into triangles using the earcut method. Returns a sorted set
     *  wrt to areas */
    VectorShapeSet triangulate(const VectorShape &s) const;

    float mMsl;
    ListShape mFromList, mToList;
    VectorShape mFrom, mTo, mCur;
    bool dirty;
};

};
