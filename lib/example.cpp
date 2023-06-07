#include "flubberpp.h"

static void one_to_one() {
  // VectorShape is a standard std::vector<flubberpp::Point>
  flubberpp::VectorShape from({ {0,0}, {100,0}, {100,100}, {0,100} });
  flubberpp::VectorShape to({ {0,50}, {100,50}, {50,100}, {0,50} });

  // interpolator
  flubberpp::SingleInterpolator interp(from,to);

  // get the interpolated shape at some point in time,  between 0 and 1 using at()
  const auto &s = interp.at(0.5);
}
