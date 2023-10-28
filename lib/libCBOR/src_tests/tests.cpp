// tests.cpp is part of libCBOR.
// (c) 2017 Shawn Silverman

#include "tests.h"

// C++ includes
#ifdef __has_include
#if __has_include(<cmath>)
#include <cmath>
#else
#include <math.h>
#endif
#if __has_include(<cstdint>)
#include <cstdint>
#else
#include <stdint.h>
#endif
#else
#include <cmath>
#include <cstdint>
#endif

// Other includes
#include <Arduino.h>
#include <ArduinoUnit.h>

// Project includes
#include "CBOR.h"
#include "CBOR_parsing.h"
#include "CBOR_streams.h"

namespace cbor = ::qindesign::cbor;

#include "tests/stream.inc"
#include "tests/syntax.inc"
#include "tests/unsigned.inc"
#include "tests/negative.inc"
#include "tests/simple_value.inc"
#include "tests/floating_point.inc"
#include "tests/bytes.inc"
#include "tests/tag.inc"
#include "tests/array.inc"
#include "tests/map.inc"
#include "tests/write.inc"
#include "tests/well_formed.inc"
#include "tests/api.inc"
#include "tests/parsing.inc"
#include "tests/int.inc"

// ***************************************************************************
//  Main program
// ***************************************************************************

void setup() {
  Serial.begin(9600);
  while (!Serial) {}
  delay(4000);
}

void loop() {
  Test::run();
}
