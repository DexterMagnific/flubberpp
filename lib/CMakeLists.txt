add_library(libflubberpp STATIC
  flubberpp.cpp
  flubberpp.h
  shape.h
  earcut.hpp
  example.cpp
)

target_compile_definitions(libflubberpp PRIVATE FLUBBERPP_LIBRARY)
