# flubberpp
This is a C++ port of the flubber (https://github.com/veltman/flubber) shape interpolation library.

![Qt demo app](https://github.com/DexterMagnific/flubberpp/assets/2777588/6879d4dd-bfe0-4413-be13-1750792371b8)

# Contents
* flubberpp library: C++ only that provides shape interpolation, but no visualization
* a Qt5 demo app: a demo app that uses flubberpp library to render interpolations

# Requirements
* C++17 compiler
* Qt5 development libraries: only if you want ot build the demo app

# Limitations
Currently only single shape to single shape interpolation is implemented.

# Build
```bash
mkdir build
cd build

# flubberpp library only
cmake .. -DCMAKE_BUILD_QT_DEMO=No
# flubberpp library and demo app
cmake ..

make

# run demo
./qtdemo/qtdemo
```

# Usage
```C++
#include "flubberpp.h"
```

## One to One interpolation
```C++
// VectorShape is a standard std::vector<flubberpp::Point>
flubberpp::VectorShape from({ {0,0}, {100,0}, {100,100}, {0,100} });
flubberpp::VectorShape to({ {0,50}, {100,50}, {50,100}, {0,50} });

// interpolator
flubberpp::SingleInterpolator interp(from,to);

// get the interpolated shape at some point in time,  between 0 and 1 using at()
const auto &s = interp.at(0.5);

// traverse interpolated shape and draw line segments
for (const auto &p: s) {
    drawLineTo(p.x,p.y);
}
```

## One to Many/Many to One interpolation

NYI

## M to N interpolation

NYI

# Embedding
Take the contents of the ``lib`` folder and add it to your project. If you are using CMake for building, you can reuse the included ``CMakeLists.txt``.

# Author
Saïd Lankri

# Credits
* Noah Veltman for the original [flubber library](https://github.com/veltman/flubber)
* Mapbox for [earcut](https://github.com/mapbox/earcut) and its C++ port
