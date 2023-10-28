// CBOR_parsing.h contains functions that help with parsing CBOR data.
// This is part of libCBOR.
// (c) 2017 Shawn Silverman

#ifndef CBOR_PARSING_H_
#define CBOR_PARSING_H_

// C++ includes
#ifdef __has_include
#if __has_include(<cstdint>)
#include <cstdint>
#else
#include <stdint.h>
#endif
#else
#include <cstdint>
#endif

// Project includes
#include "CBOR.h"

namespace qindesign
{
    namespace cbor
    {

        // ***************************************************************************
        //  Expectation functions. All of these functions treat end-of-stream as
        //  an unexpected value.
        // ***************************************************************************

        // Expects a specific type and value. This will always return false for
        // some data types having no value or a non-integral value: boolean, null,
        // undefined, break, floating-point.
        bool expectValue(Reader &r, DataType dt, uint64_t val);

        // Expects a specific unsigned int value.
        bool expectUnsignedIntValue(Reader &r, uint64_t u);

        // Expects a specific integer value. This takes into account values that
        // have overflowed in some way; specifically, values that appear negative
        // but are supposed to be positive, and values that appear positive but
        // are supposed to be negative. For these special cases, this will return
        // false.
        //
        // See these Reader functions: getInt(), isNegativeOverflow(),
        // and isUnsigned().
        bool expectIntValue(Reader &r, int64_t i);

        // Expects bytes having the given length. For indefinite-length bytes,
        // use the Reader::isIndefiniteLength() function.
        bool expectBytesLength(Reader &r, uint64_t len);

        // Expects text having the given length. For indefinite-length text,
        // use the Reader::isIndefiniteLength() function.
        bool expectTextLength(Reader &r, uint64_t len);

        // Expects an array having the given length. For an indefinite-length array,
        // use the Reader::isIndefiniteLength() function.
        bool expectArrayLength(Reader &r, uint64_t len);

        // Expects a map having the given length. For an indefinite-length map,
        // use the Reader::isIndefiniteLength() function.
        bool expectMapLength(Reader &r, uint64_t len);

        // Expects a specific half- or single-precision floating-point value. Zero
        // and negative zero are considered equal values.
        bool expectFloatValue(Reader &r, float f);

        // Expects a specific double-precision floating-point value. Zero and
        // negative zero are considered equal values.
        bool expectDoubleValue(Reader &r, double d);

        // Expects a specific boolean value.
        bool expectBooleanValue(Reader &r, bool b);

        // Expects that the next data item is a boolean and has the value 'true'.
        bool expectTrue(Reader &r);

        // Expects that the next data item is a boolean and has the value 'false'.
        bool expectFalse(Reader &r);

        // Expects a specific tag value.
        bool expectTagValue(Reader &r, uint64_t val);

        // Expects and fills in an unsigned value. This returns true if the data item
        // is an unsigned value and false otherwise.
        bool expectUnsignedInt(Reader &r, uint64_t *u);

        // Expects and fills in an integer value. This returns true if the data item
        // is an integer (signed or unsigned) and false otherwise.
        //
        // Please see the isUnsigned() and isNegativeOverflow() functions to
        // determine properties about the actual value.
        bool expectInt(Reader &r, int64_t *i);

        // Expects bytes and fills in the length and a flag indicating whether it's
        // an indefinite-length data item. This returns true if the data item is the
        // bytes type and false otherwise. For bytes having an indefinite length,
        // what follows will be a series of definite-length bytes items, terminated
        // by a Break, assuming well-formed data.
        //
        // Use expectBytesOrBreak to read the definite-length bytes chunks.
        //
        // For indefinite-length bytes, length will be set to zero.
        bool expectBytes(Reader &r, uint64_t *length, bool *isIndefinite);

        // Expects non-indefinite-length bytes having the given length. It is assumed
        // that a null byte array matches anything.
        bool expectDefiniteBytes(Reader &r, const uint8_t *b, uint64_t len);

        // Expects bytes and fills in the length and a flag indicating whether a
        // Break was found. This is used to detect the chunks following an
        // indefinite-length bytes data item.
        //
        // For a break, length will be set to zero.
        bool expectBytesOrBreak(Reader &r, uint64_t *length, bool *isBreak);

        // Expects text and fills in the length and a flag indicating whether it's
        // an indefinite-length data item. This returns true if the data item is the
        // text type and false otherwise. For text having an indefinite length,
        // what follows will be a series of definite-length text items, terminated
        // by a Break, assuming well-formed data.
        //
        // Use expectTextOrBreak to read the definite-length bytes chunks.
        //
        // For indefinite-length text, length will be set to zero.
        bool expectText(Reader &r, uint64_t *length, bool *isIndefinite);

        // Expects non-indefinite-length text having the given length. It is assumed
        // that a null byte array matches anything.
        bool expectDefiniteText(Reader &r, const uint8_t *b, uint64_t len);

        // Expects text and fills in the length and a flag indicating whether a
        // Break was found. This is used to detect the chunks following an
        // indefinite-length text data item.
        //
        // For a break, length will be set to zero.
        bool expectTextOrBreak(Reader &r, uint64_t *length, bool *isBreak);

        // Expects an array and fills in the length and a flag indicating whether it's
        // an indefinite-length array. This returns true if the data item is the array
        // type and false otherwise. Indefinite arrays are terminated with a Break.
        //
        // For an indefinite-length array, length will be set to zero.
        bool expectArray(Reader &r, uint64_t *length, bool *isIndefinite);

        // Expects a map and fills in the length and a flag indicating whether it's
        // an indefinite-length array. This returns true if the data item is the map
        // type and false otherwise. Indefinite maps are terminated with a Break.
        //
        // For an indefinite-length array, length will be set to zero.
        bool expectMap(Reader &r, uint64_t *length, bool *isIndefinite);

        // Expects and fills in a boolean value. This returns true if the data item
        // is a boolean and false otherwise.
        bool expectBoolean(Reader &r, bool *b);

        // Expects and fills in a single-precision floating-point value. This returns
        // true if the data item is one of these and false otherwise.
        bool expectFloat(Reader &r, float *f);

        // Expects and fills in a double-precision floating-point value. This returns
        // true if the data item is one of these and false otherwise.
        bool expectDouble(Reader &r, double *d);

        // Expects a null value. This returns true if the data item is a null value
        // and false otherwise.
        bool expectNull(Reader &r);

        // Expects an undefined value. This returns true if the data item is an
        // undefined value and false otherwise.
        bool expectUndefined(Reader &r);

        // Expects and fills in a simple 1-byte value. This returns true if the data
        // item is one of these and false otherwise.
        bool expectSimpleValue(Reader &r, uint8_t *v);

        // // Expects a non-numerical and non-boolean simple value. The set of allowed
        // // values includes null, undefined, and break.
        // // This returns true if the data item is the expected type and false otherwise.
        // bool expectSimpleValue(Reader &r, DataType dt);

        // Expects and fills in a tag. This returns true if the data item is one
        // of these and false otherwise.
        bool expectTag(Reader &r, uint64_t *v);

        // ***************************************************************************
        //  Read functions.
        // ***************************************************************************

        // Attempts to read exactly len bytes into b. This will return the actual
        // number of bytes read, a smaller number than len only if the stream
        // encountered end-of-stream.
        size_t readFully(Reader &r, uint8_t *b, size_t len);

        // Reads from the reader until there's some data available. If end-of-stream
        // is encountered then this yields and keeps looking. Forever.
        DataType readUntilData(Reader &r);

    } // namespace cbor
} // namespace qindesign

#endif // CBOR_PARSING_H_
