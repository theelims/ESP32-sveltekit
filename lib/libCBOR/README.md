# Readme for libCBOR v1.6.1

This is a library for processing CBOR
[RFC 7049](https://tools.ietf.org/html/rfc7049) data.

## Features

Notable features of this library:

1. Handles and identifies well-formed and not-well-formed CBOR data.
2. Provides utility functions to make it easier to parse data.
3. Provides some Stream and Print implementations to make it easy to
   interface with byte arrays and the EEPROM.

## How to use

The classes you'll need are in the `qindesign::cbor` namespace:
`Reader` and `Writer`. A complete example of how to use them are in
`StructInBytes`.

The main class documentation can be found in `src/CBOR.h`. Other documentation
can be found as follows:

* Utility functions: `src/CBOR_utils.h`
* `Stream` and `Print` implementations: `src/CBOR_streams.h`
* Parsing helpers: `src/CBOR_parsing.h`

## Installing as an Arduino library

Not all the files in this project are necessary in an installed library.
Only the following files and directories need to be there:

* `*.md`
* `LICENSE`
* `examples/`
* `keywords.txt`
* `library.json`
* `library.properties`
* `src/`

## Running the tests

There are tests included in this project that rely on a project called
[ArduinoUnit](https://github.com/mmurdoch/arduinounit).

Note that the code for ArduinoUnit is not included in this library and needs
to be downloaded separately.

## Code style

Code style for this project mostly follows the
[Google C++ Style Guide](https://google.github.io/styleguide/cppguide.html).

## References

1. Concise Binary Object Representation (CBOR):
   [RFC 7049](https://tools.ietf.org/html/rfc7049)
2. [RFC 7049 Errata](https://www.rfc-editor.org/errata_search.php?rfc=7049)
3. Specification including the errata:
   [spec-with-errata-fixed](https://github.com/cbor/spec-with-errata-fixed/)

---

Copyright (c) 2017-2019 Shawn Silverman
