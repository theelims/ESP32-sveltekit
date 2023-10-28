# Changelog for libCBOR

This document details the changes between each release.

## [1.6.1]

### Changed
* Only compile EEPROM things if EEPROM.h (and `__has_include`) exists.

## [1.6.0]

### Added
* Added new `expectDefiniteBytes` and `expectDefiniteText` functions
  to `CBOR_parsing`. These test for a specific sequence of bytes or text.

### Fixed
* Added a header guard to `CBOR_streams.h`.

## [1.5.4]

### Changed
* Tested with
  [ArduinoUnit v3.0.2](https://github.com/mmurdoch/arduinounit/releases/tag/v3.0.2).

### Fixed
* Fixed the map part of the `bytes_unavailable` test in `stream.inc`. It now
  ensures that the value part of the map entry is there and not some
  random value.
* Compiles on more platforms (listed using PlatformIO board names): `teensylc`,
  `teensy35`, `teensy36`, `esp12e`, `nucleo_f302r8`, `nucleo_f303k8`, and
  `nucleo_f103rb`. This is in addition to just compiling and testing on
  `teensy31`, `huzzah`, and `featheresp32`.
* Now making use of PlatformIO's `build_unflags` to ensure that the
  `-fsingle-precision-constant` does not take effect; the compiler was
  complaining that some float literals were still overflowing, even though
  `-fno-single-precision-constant` was being passed in `build_flags`.

## [1.5.3]

### Fixed
* Compilation for ESP32.

## [1.5.2]

### Added
* keywords.txt file.

## [1.5.1]

### Fixed
* Removed the C-style comment line from `library.json`.

## [1.5.0]

### Added
* `Reader::bytesAvailable()` function that returns the number of bytes
  available in the current Bytes or Text data item.

### Changed
* Changed the behaviour of `readByte()` and `readBytes` in `Reader` to only
  read up to the number of bytes available in the current Bytes or Text
  data item.
* `Reader::getDataType()` is now `const`.

## [1.4.1]

### Changed
* `expectFloatValue` and `expectDoubleValue` now treat zero and negative
  zero as equal.

## [1.4.0]

### Added
* `Reader::getDataType()` function that returns the current data type without
  advancing the stream.
* Parsing helper functions that expect specific values for: unsigned ints,
  ints, bytes with length, text with length, arrays with length, maps with
  length, and tags. These join the other `expectXXXValue` functions.
* More tests, including ones for detecting negative zero being different than
  positive zero.

### Changed
* The length is now explicitly set to zero when parsing indefinite-length
  items: bytes, text, arrays, maps.
* Updated docs.

### Fixed
* Changed all `Reader` parameters in the parsing helper functions to
  references instead of copies. Now the internal state is correctly
  maintained across helper function calls.

## [1.3.1]

### Changed
* Moved the tests to a folder named `src_tests` for now. This makes it easier
  to directly reference this project as an Arduino and PlatformIO library.

## [1.3.0]

### Added
* `expectBoolean` helper function, for consistency.
* `expectBytesOrBreak` and `expectTextOrBreak` helper functions for reading
  indefinite-length Bytes or Text chunks.
* `isFalse()`, `isTrue()`, `isNull()`, `isUndefined()`, and `isBreak()`
  functions in `Reader`.
* `isNegativeOverflow()` and `isUnsigned()` functions in `Reader` help
  disambiguate some special cases when reading and expecting integer values
  via `Reader::getInt()` and `expectInt`.
* `isIndefinite` output parameter in `expectBytes`, `expectText`,
  `expectArray`, and `expectMap` for discovering indefinite-length data
  items.

### Changed
* The `readFully` helper function now returns a `size_t` instead of an
  `unsigned int`.
* `Reader::getInt()` and the `expectInt` helper function can now read and
  expect both signed and unsigned integers. New `isNegativeOverflow()` and
  `isUnsigned()` functions in `Reader` help disambiguate some special cases.
* Documentation updates.

### Removed
* `expectIndefiniteBytes` and `expectIndefiniteText` functions because
  there's no need to separate the syntactic difference. There's a new
  `isIndefinite` output parameter in `expectBytes`, `expectText`,
  `expectArray`, and `expectMap` for discovering indefinite-length data
  items.

### Fixed
* `expectDoubleValue` was using a `float` internally.
* `Reader::isWellFormed()` allowed nested indefinite-length bytes or text.
* `Reader::getBoolean()` was allowing certain 1-byte simple values to
  function as booleans.

## libCBOR v1.2.0

This release updates the API to include a way to access the read size and
write size, and also makes the Reader and Writer classes implement Stream
and Print, respectively. As well, the byte array and EEPROM Stream and Print
implementations now provide a way to access the internal index (or address,
in the case of EEPROM), for ease of determining how far reading or writing
has progressed.

## First libCBOR release

Tag: v1.1.0

This is the first release of the libCBOR library. It includes functions for
reading and writing, and helper functions for parsing and for processing
EEPROM.

---

Copyright (c) 2017-2019 Shawn Silverman
