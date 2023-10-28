// CBOR.cpp is part of libCBOR.
// (c) 2017 Shawn Silverman

#include "CBOR.h"

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

namespace qindesign
{
    namespace cbor
    {

        // Major types
        constexpr int kUnsignedInt = 0;
        constexpr int kNegativeInt = 1;
        constexpr int kBytes = 2;
        constexpr int kText = 3;
        constexpr int kArray = 4;
        constexpr int kMap = 5;
        constexpr int kTag = 6;
        constexpr int kSimpleOrFloat = 7;

        // ***************************************************************************
        //  Reader
        // ***************************************************************************

        DataType Reader::readDataType()
        {
            // Read the initial byte
            if (state_ == State::kStart)
            {
                // Initialize everything to a default
                value_ = 0;
                bytesAvailable_ = 0;
                syntaxError_ = SyntaxError::kNoError;
                initialByte_ = read();
                if (initialByte_ < 0)
                {
                    majorType_ = 0;
                    addlInfo_ = 0;
                    waitAvailable_ = 0;
                    return DataType::kEOS;
                }
                majorType_ = static_cast<uint8_t>(initialByte_) >> 5;
                addlInfo_ = initialByte_ & 0x1f;
                state_ = State::kAdditionalInfo;
            }

            // Process the additional info by noting how many bytes we need
            if (state_ == State::kAdditionalInfo)
            {
                waitAvailable_ = 0;
                switch (addlInfo_)
                {
                case 24:
                    waitAvailable_ = 1;
                    state_ = State::kWaitAvailable;
                    break;
                case 25:
                    waitAvailable_ = 2;
                    state_ = State::kWaitAvailable;
                    break;
                case 26:
                    waitAvailable_ = 4;
                    state_ = State::kWaitAvailable;
                    break;
                case 27:
                    waitAvailable_ = 8;
                    state_ = State::kWaitAvailable;
                    break;
                case 28:
                case 29:
                case 30:
                    syntaxError_ = SyntaxError::kUnknownAdditionalInfo;
                    return DataType::kSyntaxError;
                case 31:
                    switch (majorType_)
                    {
                    case kUnsignedInt:
                    case kNegativeInt:
                    case kTag:
                        syntaxError_ = SyntaxError::kNotAnIndefiniteType;
                        return DataType::kSyntaxError;
                    case kSimpleOrFloat: // Floating-point numbers and simple data types
                        // Always allow breaks
                        break;
                    }
                    state_ = State::kReadValue;
                    break;
                default:
                    state_ = State::kReadValue;
                }
            }

            // If we need to, wait for any available bytes
            if (state_ == State::kWaitAvailable)
            {
                if (available() < waitAvailable_)
                {
                    return DataType::kEOS;
                }
                state_ = State::kReadValue;
            }

            // Read the value from the stream
            if (state_ == State::kReadValue)
            {
                switch (addlInfo_)
                {
                case 24:
                    value_ = read();
                    break;
                case 25:
                    value_ =
                        (static_cast<uint16_t>(read()) << 8) |
                        (static_cast<uint16_t>(read()));
                    break;
                case 26:
                    value_ =
                        (static_cast<uint32_t>(read()) << 24) |
                        (static_cast<uint32_t>(read()) << 16) |
                        (static_cast<uint32_t>(read()) << 8) |
                        (static_cast<uint32_t>(read()));
                    break;
                case 27:
                    value_ =
                        (static_cast<uint64_t>(read()) << 56) |
                        (static_cast<uint64_t>(read()) << 48) |
                        (static_cast<uint64_t>(read()) << 40) |
                        (static_cast<uint64_t>(read()) << 32) |
                        (static_cast<uint64_t>(read()) << 24) |
                        (static_cast<uint64_t>(read()) << 16) |
                        (static_cast<uint64_t>(read()) << 8) |
                        (static_cast<uint64_t>(read()));
                    break;
                case 28:
                case 29:
                case 30:
                    // Shouldn't happen, caught before
                    value_ = addlInfo_;
                    break;
                case 31: // Indefinite length or break
                    value_ = 0;
                    break;
                default:
                    value_ = addlInfo_;
                }
                state_ = State::kDetermineType;
            }

            if (state_ == State::kDetermineType)
            {
                state_ = State::kStart;
                switch (majorType_)
                {
                case kBytes:
                case kText:
                    bytesAvailable_ = value_;
                    break;
                case kSimpleOrFloat:
                    switch (addlInfo_)
                    {
                    case 20: // False
                    case 21: // True
                    case 22: // Null
                    case 23: // Undefined
                    case 31:
                        value_ = 0;
                        break;
                    }
                    break;
                }
                return getDataType();
            }

            return DataType::kEOS;
        }

        DataType Reader::getDataType() const
        {
            switch (majorType_)
            {
            case kUnsignedInt:
                return DataType::kUnsignedInt;
            case kNegativeInt:
                return DataType::kNegativeInt;
            case kBytes:
                return DataType::kBytes;
            case kText:
                return DataType::kText;
            case kArray:
                return DataType::kArray;
            case kMap:
                return DataType::kMap;
            case kTag:
                return DataType::kTag;
            case kSimpleOrFloat: // Floating-point numbers and simple data types
                switch (addlInfo_)
                {
                case 20: // False
                case 21: // True
                    return DataType::kBoolean;
                case 22:
                    return DataType::kNull;
                case 23:
                    return DataType::kUndefined;
                case 24:
                    // Values < 32 are invalid but technically well-formed, so
                    // don't do the following check:
                    // if (value_ < 32) {
                    //   syntaxError_ = SyntaxError::kBadSimpleValue;
                    //   return DataType::kSyntaxError;
                    // }
                    return DataType::kSimpleValue;
                    break;
                case 25:
                case 26:
                    return DataType::kFloat;
                case 27:
                    return DataType::kDouble;
                case 28:
                case 29:
                case 30:
                    // Shouldn't happen, caught before
                    return DataType::kSyntaxError;
                case 31:
                    return DataType::kBreak;
                default:
                    return DataType::kSimpleValue;
                }
                break;
            default:
                // Shouldn't happen
                return DataType::kUnsignedInt;
            }
        }

        size_t Reader::readBytes(uint8_t *buffer, size_t length)
        {
            if (bytesAvailable_ == 0)
            {
                return 0;
            }
            if (length > bytesAvailable_)
            {
                length = bytesAvailable_;
            }
            size_t read = in_.readBytes(buffer, length);
            readSize_ += read;
            bytesAvailable_ -= read;
            return read;
        }

        int Reader::readByte()
        {
            if (bytesAvailable_ == 0)
            {
                return -1;
            }
            int b = read();
            if (b >= 0)
            {
                bytesAvailable_--;
            }
            return b;
        }

        SyntaxError Reader::getSyntaxError() const
        {
            return syntaxError_;
        }

        uint64_t Reader::getRawValue() const
        {
            return value_;
        }

        bool Reader::isIndefiniteLength() const
        {
            switch (majorType_)
            {
            case kBytes: // Bytes
            case kText:  // Text
            case kArray: // Array
            case kMap:   // Map
                return addlInfo_ == 31;
            }
            return false;
        }

        uint64_t Reader::getLength() const
        {
            return value_;
        }

        bool Reader::getBoolean() const
        {
            if (majorType_ == kSimpleOrFloat)
            {
                if (addlInfo_ == 21)
                {
                    return true;
                }
                // Technically well-formed but invalid, so don't check for this:
                // if (addlInfo_ == 24 && value_ == 21) {
                //   return true;
                // }
            }
            return false;
        }

        float Reader::getFloat() const
        {
            return static_cast<float>(getDouble());
        }

        double Reader::getDouble() const
        {
            // NOTE: Doing the conversion this way avoids endian and size differences

            if (majorType_ != kSimpleOrFloat)
            {
                return 0.0;
            }

            if (addlInfo_ == 25)
            { // Half-precision
                constexpr int kBitsM = 10;
                constexpr int kBitsE = 5;
                constexpr int kExpBias = (1 << (kBitsE - 1)) - 1; // 15
                uint16_t half = static_cast<uint16_t>(value_);
                int e = (half >> kBitsM) & ((1 << kBitsE) - 1);
                int m = half & ((1 << kBitsM) - 1);
                double val;
                if (e == 0)
                {
                    val = ldexp(m, 1 - kExpBias - kBitsM);
                }
                else if (e != (1 << kBitsE) - 1)
                {
                    val = ldexp(m + (1 << kBitsM), e - kExpBias - kBitsM);
                }
                else
                {
                    val = (m == 0) ? INFINITY : NAN;
                }
                double sign = (half & (1 << (kBitsM + kBitsE))) ? -1 : 1;
                return copysign(val, sign);
            }

            if (addlInfo_ == 26)
            { // Single-precision
                float f;
                uint32_t bits = static_cast<uint32_t>(value_);
                memcpy(&f, &bits, 4); // TODO: Is the size always 4?
                return f;
                // constexpr int kBitsM = 23;
                // constexpr int kBitsE = 8;
                // constexpr int kExpBias = (1 << (kBitsE - 1)) - 1;  // 127
                // uint32_t single = static_cast<uint32_t>(value_);
                // int e = (single >> kBitsM) & ((1 << kBitsE) - 1);
                // unsigned long m = single & ((1UL << kBitsM) - 1);
                // double val;
                // if (e == 0) {
                //   val = ldexp(m, 1 - kExpBias - kBitsM);
                // } else if (e != (1 << kBitsE) - 1) {
                //   val = ldexp(m + (1UL << kBitsM), e - kExpBias - kBitsM);
                // } else if (m == 0) {
                //   val = INFINITY;
                // } else {  // NaN
                //   float f;
                //   uint32_t bits = static_cast<uint32_t>(value_);
                //   memcpy(&f, &bits, 4);  // TODO: Is the size always 4?
                //   return f;
                // }
                // double sign = (single & (1UL << (kBitsM + kBitsE))) ? -1 : 1;
                // return copysign(val, sign);
            }

            if (addlInfo_ == 27)
            { // Double-precision
                double val;
                memcpy(&val, &value_, 8); // TODO: Is the size always 8?
                return val;
                // constexpr int kBitsM = 52;
                // constexpr int kBitsE = 11;
                // constexpr int kExpBias = (1 << (kBitsE - 1)) - 1;  // 1023
                // int e = (value_ >> kBitsM) & ((1 << kBitsE) - 1);
                // unsigned long long m = value_ & ((1ULL << kBitsM) - 1);
                // double val;
                // if (e == 0) {
                //   val = ldexp(m, 1 - kExpBias - kBitsM);
                // } else if (e != (1 << kBitsE) - 1) {
                //   val = ldexp(m + (1ULL << kBitsM), e - kExpBias - kBitsM);
                // } else if (m == 0) {
                //   val = INFINITY;
                // } else {  // NaN
                //   memcpy(&val, &value_, 8);  // TODO: Is the size always 8?
                //   return val;
                // }
                // double sign = (value_ & (1ULL << (kBitsM + kBitsE))) ? -1 : 1;
                // return copysign(val, sign);
            }

            return 0.0;
        }

        uint64_t Reader::getUnsignedInt() const
        {
            if (majorType_ == kUnsignedInt)
            {
                return value_;
            }
            return 0ULL;
        }

        int64_t Reader::getInt() const
        {
            if (majorType_ == kNegativeInt)
            {
                return static_cast<int64_t>(~value_);
            }
            if (majorType_ == kUnsignedInt)
            {
                return static_cast<int64_t>(value_);
            }
            return 0LL;
        }

        bool Reader::isNegativeOverflow() const
        {
            return (majorType_ == kNegativeInt) &&
                   ((value_ & 0x8000000000000000ULL) != 0);
        }

        bool Reader::isUnsigned() const
        {
            return (majorType_ == kUnsignedInt);
        }

        uint8_t Reader::getSimpleValue() const
        {
            if (majorType_ == kSimpleOrFloat)
            {
                return static_cast<uint8_t>(value_);
            }
            return 0;
        }

        uint64_t Reader::getTag() const
        {
            if (majorType_ == kTag)
            {
                return value_;
            }
            return 0ULL;
        }

        bool Reader::isFalse() const
        {
            return majorType_ == kSimpleOrFloat && addlInfo_ == 20;
        }

        bool Reader::isTrue() const
        {
            return majorType_ == kSimpleOrFloat && addlInfo_ == 21;
        }

        bool Reader::isNull() const
        {
            return majorType_ == kSimpleOrFloat && addlInfo_ == 22;
        }

        bool Reader::isUndefined() const
        {
            return majorType_ == kSimpleOrFloat && addlInfo_ == 23;
        }

        bool Reader::isBreak() const
        {
            return majorType_ == kSimpleOrFloat && addlInfo_ == 31;
        }

        // ***************************************************************************
        //  Well-formedness checks
        // ***************************************************************************

        bool Reader::isWellFormed()
        {
            bool retval = (isWellFormed(read(), false) >= 0);
#if defined(ESP8266) || defined(ESP32)
            yield();
#endif
            return retval;
        }

        int Reader::isWellFormed(int initialByte, bool breakable)
        {
            if (initialByte < 0)
            {
                return -1;
            }
            uint8_t majorType = static_cast<uint8_t>(initialByte) >> 5;
            uint8_t ai = initialByte & 0x1f; // Additional information
            uint64_t val;
            switch (ai)
            {
            case 24:
                if (available() < 1)
                {
                    return -1;
                }
                val = static_cast<uint8_t>(read());
                // Simple types having a 1-byte value < 32 are invalid but
                // technically well-formed, so don't do the following check:
                // if (majorType == kSimpleOrFloat && val < 32) {
                //   return -1;
                // }
                break;
            case 25:
                if (available() < 2)
                {
                    return -1;
                }
                val =
                    (static_cast<uint16_t>(read()) << 8) |
                    (static_cast<uint16_t>(read()));
                break;
            case 26:
                if (available() < 4)
                {
                    return -1;
                }
                val =
                    (static_cast<uint32_t>(read()) << 24) |
                    (static_cast<uint32_t>(read()) << 16) |
                    (static_cast<uint32_t>(read()) << 8) |
                    (static_cast<uint32_t>(read()));
                break;
            case 27:
                if (available() < 8)
                {
                    return -1;
                }
                val =
                    (static_cast<uint64_t>(read()) << 56) |
                    (static_cast<uint64_t>(read()) << 48) |
                    (static_cast<uint64_t>(read()) << 40) |
                    (static_cast<uint64_t>(read()) << 32) |
                    (static_cast<uint64_t>(read()) << 24) |
                    (static_cast<uint64_t>(read()) << 16) |
                    (static_cast<uint64_t>(read()) << 8) |
                    (static_cast<uint64_t>(read()));
                break;
            case 28:
            case 29:
            case 30:
                return -1;
            case 31:
                return isIndefiniteWellFormed(majorType, breakable);
            default:
                val = ai;
            }

            switch (majorType)
            {
            // No content for 0, 1, or 7
            case 2: // Byte string
            case 3: // Text string (UTF-8)
                if (val <= UINT32_MAX)
                {
                    for (uint32_t i = 0, max = static_cast<uint32_t>(val); i < max; i++)
                    {
                        if (read() < 0)
                        {
                            return -1;
                        }
                    }
                }
                else
                {
                    for (uint64_t i = 0; i < val; i++)
                    {
                        if (read() < 0)
                        {
                            return -1;
                        }
                    }
                }
                break;
            case 5: // Map
                // Check for overflow
                if (val != 0 && 2 * val <= val)
                {
                    return -1;
                }
                val <<= 1;
                // fallthrough
            case 4: // Array
                if (val <= UINT32_MAX)
                {
                    for (uint32_t i = 0, max = static_cast<uint32_t>(val); i < max; i++)
                    {
                        if (isWellFormed(read(), false) < 0)
                        {
                            return -1;
                        }
                    }
                }
                else
                {
                    for (uint64_t i = 0; i < val; i++)
                    {
                        if (isWellFormed(read(), false) < 0)
                        {
                            return -1;
                        }
                    }
                }
                break;
            case 6:
                if (isWellFormed(read(), false) < 0)
                {
                    return -1;
                }
                break;
            default:
                // Unsigned integer (0), Negative integer (1),
                // Floating-point numbers and simple data types (7)
                break;
            }
            return majorType;
        }

        int Reader::isIndefiniteWellFormed(uint8_t majorType, bool breakable)
        {
            switch (majorType)
            {
            case kBytes:
            case kText:
                while (true)
                {
                    int ib = read(); // Initial byte
                    if (ib < 0)
                    {
                        return -1;
                    }

                    // The only case we allow the major type to not match is a break
                    if (ib == (kSimpleOrFloat << 5) + 31)
                    {
                        break;
                    }

                    // Now short-circuit to exit if the major type doesn't match
                    // or if there's a nested indefinite-length situation
                    uint8_t mt = static_cast<uint8_t>(ib) >> 5; // Major type
                    if (mt != majorType || (ib & 0x1f) == 31)
                    {
                        return -1;
                    }

                    int t = isWellFormed(ib, true);
                    if (t == -2)
                    { // Break
                        // NOTE: This case shouldn't happen because of the Break check above
                        break;
                    }
                    if (t == -1)
                    { // Malformed
                        return -1;
                    }
                }
                break;
            case kArray:
                while (true)
                {
                    int t = isWellFormed(read(), true);
                    if (t == -2)
                    { // Break
                        break;
                    }
                    if (t == -1)
                    { // Malformed
                        return -1;
                    }
                }
                break;
            case kMap:
                while (true)
                {
                    int t = isWellFormed(read(), true);
                    if (t == -2)
                    { // Break
                        break;
                    }
                    if (t == -1)
                    { // Malformed
                        return -1;
                    }
                    if (isWellFormed(read(), false) < 0)
                    {
                        return -1;
                    }
                }
                break;
            case kSimpleOrFloat: // Floating-point numbers and simple data types
                if (breakable)
                {
                    return -2;
                }
                return -1;
            default:
                // Unsigned integer (0), Negative integer (1), Tag (6)
                return -1;
            }

            return majorType;
        }

        // ***************************************************************************
        //  Writer
        // ***************************************************************************

        void Writer::writeBoolean(bool b)
        {
            write((kSimpleOrFloat << 5) + (b ? 21 : 20));
        }

        void Writer::writeFloat(float f)
        {
            if (write((kSimpleOrFloat << 5) + 26) == 0)
            {
                return;
            }

            // constexpr int kBitsM = 23;
            // constexpr int kBitsE = 8;

            uint32_t val;
            memcpy(&val, &f, 4); // TODO: Is the size always 4?
            // if (std::isnan(f)) {
            //   memcpy(&val, &f, 4);  // TODO: Is the size always 4?
            // } else if (std::isinf(f)) {
            //   // All 1's for the exponent
            //   val = ((1UL << kBitsE) - 1UL) << kBitsM;
            // } else if (f == 0) {
            //   val = 0;
            // } else {
            //   float f2 = (f < 0) ? -f : f;
            //   constexpr int kExpBias = (1 << (kBitsE - 1)) - 1;  // 127
            //   int e = ilogb(f2);
            //   unsigned long m;
            //   if (e <= -kExpBias) {
            //     // exp = 1 - kExpBias - kBitsM
            //     e = 0;
            //     m = static_cast<unsigned long>(scalbn(f2, -(1 - kExpBias - kBitsM)));
            //   } else {
            //     // exp = e - kExpBias - kBitsM
            //     m = static_cast<unsigned long>(scalbn(f2, -(e - kBitsM))) - (1UL << kBitsM);
            //     e += kExpBias;
            //   }
            //   val = (static_cast<unsigned long>(e) << kBitsM) | m;
            // }
            // if (!std::isnan(f) && std::signbit(f)) {
            //   val |= (1UL << 31);
            // }

            if (write(val >> 24) == 0)
            {
                return;
            }
            if (write(val >> 16) == 0)
            {
                return;
            }
            if (write(val >> 8) == 0)
            {
                return;
            }
            write(val);
        }

        void Writer::writeDouble(double d)
        {
            if (write((kSimpleOrFloat << 5) + 27) == 0)
            {
                return;
            }

            // constexpr int kBitsM = 52;
            // constexpr int kBitsE = 11;

            uint64_t val;
            memcpy(&val, &d, 8); // TODO: Is the size always 8?
            // if (std::isnan(d)) {
            //   memcpy(&val, &d, 8);  // TODO: Is the size always 8?
            // } else if (std::isinf(d)) {
            //   // All 1's for the exponent
            //   val = ((1ULL << kBitsE) - 1ULL) << kBitsM;
            // } else if (d == 0) {
            //   val = 0;
            // } else {
            //   double d2 = (d < 0) ? -d : d;
            //   constexpr int kExpBias = (1 << (kBitsE - 1)) - 1;  // 1023
            //   int e = ilogb(d2);
            //   unsigned long long m;
            //   if (e <= -kExpBias) {
            //     // exp = 1 - kExpBias - kBitsM
            //     e = 0;
            //     m = static_cast<unsigned long long>(scalbn(d2, -(1 - kExpBias - kBitsM)));
            //   } else {
            //     // exp = e - kExpBias - kBitsM
            //     m = static_cast<unsigned long long>(scalbn(d2, -(e - kBitsM))) - (1ULL << kBitsM);
            //     e += kExpBias;
            //   }
            //   val = (static_cast<unsigned long long>(e) << kBitsM) | m;
            // }
            // if (!std::isnan(d) && std::signbit(d)) {
            //   val |= (1ULL << 63);
            // }

            if (write(val >> 56) == 0)
            {
                return;
            }
            if (write(val >> 48) == 0)
            {
                return;
            }
            if (write(val >> 40) == 0)
            {
                return;
            }
            if (write(val >> 32) == 0)
            {
                return;
            }
            if (write(val >> 24) == 0)
            {
                return;
            }
            if (write(val >> 16) == 0)
            {
                return;
            }
            if (write(val >> 8) == 0)
            {
                return;
            }
            write(val);
        }

        void Writer::writeUnsignedInt(uint64_t u)
        {
            writeTypedInt(kUnsignedInt << 5, u);
        }

        void Writer::writeInt(int64_t i)
        {
            uint64_t u = i >> 63;                 // Extend sign
            uint8_t mt = u & (kNegativeInt << 5); // Major type, 0x20 (signed) or 0x00 (unsigned)
            u ^= i;                               // Complement negatives, equivalent to -1 - i
            writeTypedInt(mt, u);
        }

        void Writer::writeTypedInt(uint8_t mt, uint64_t u)
        {
            if (u < 24)
            {
                write(mt + u);
            }
            else if (u < (1 << 8))
            {
                if (write(mt + 24) == 0)
                {
                    return;
                }
                write(u);
            }
            else if (u < (1UL << 16))
            {
                if (write(mt + 25) == 0)
                {
                    return;
                }
                if (write(u >> 8) == 0)
                {
                    return;
                }
                write(u);
            }
            else if (u < (1ULL << 32))
            {
                if (write(mt + 26) == 0)
                {
                    return;
                }
                if (write(u >> 24) == 0)
                {
                    return;
                }
                if (write(u >> 16) == 0)
                {
                    return;
                }
                if (write(u >> 8) == 0)
                {
                    return;
                }
                write(u);
            }
            else
            {
                if (write(mt + 27) == 0)
                {
                    return;
                }
                if (write(u >> 56) == 0)
                {
                    return;
                }
                if (write(u >> 48) == 0)
                {
                    return;
                }
                if (write(u >> 40) == 0)
                {
                    return;
                }
                if (write(u >> 32) == 0)
                {
                    return;
                }
                if (write(u >> 24) == 0)
                {
                    return;
                }
                if (write(u >> 16) == 0)
                {
                    return;
                }
                if (write(u >> 8) == 0)
                {
                    return;
                }
                write(u);
            }
        }

        void Writer::writeNull()
        {
            write((kSimpleOrFloat << 5) + 22);
        }

        void Writer::writeUndefined()
        {
            write((kSimpleOrFloat << 5) + 23);
        }

        void Writer::writeSimpleValue(uint8_t v)
        {
            if (v < 24)
            {
                write((kSimpleOrFloat << 5) + v);
            }
            else
            {
                if (write((kSimpleOrFloat << 5) + 24) == 0)
                {
                    return;
                }
                write(v);
            }
        }

        void Writer::writeTag(uint64_t v)
        {
            writeTypedInt(kTag << 5, v);
        }

        void Writer::writeBytes(const uint8_t *buffer, size_t length)
        {
            write(buffer, length);
        }

        void Writer::writeByte(uint8_t b)
        {
            write(b);
        }

        void Writer::beginBytes(unsigned int length)
        {
            writeTypedInt(kBytes << 5, length);
        }

        void Writer::beginText(unsigned int length)
        {
            writeTypedInt(kText << 5, length);
        }

        void Writer::beginIndefiniteBytes()
        {
            write((kBytes << 5) + 31);
        }

        void Writer::beginIndefiniteText()
        {
            write((kText << 5) + 31);
        }

        void Writer::beginArray(unsigned int length)
        {
            writeTypedInt(kArray << 5, length);
        }

        void Writer::beginMap(unsigned int length)
        {
            writeTypedInt(kMap << 5, length);
        }

        void Writer::beginIndefiniteArray()
        {
            write((kArray << 5) + 31);
        }

        void Writer::beginIndefiniteMap()
        {
            write((kMap << 5) + 31);
        }

        void Writer::endIndefinite()
        {
            write((kSimpleOrFloat << 5) + 31);
        }

        // // Forward declarations
        // int isWellFormed(int *address, bool breakable);
        // int isIndefiniteWellFormed(int address, uint8_t majorType, bool breakable);
        //
        // bool isEEPROMWellFormed(int address) {
        //   return isWellFormed(false, &address) >= 0;
        // }
        //
        // // Checks if an item is well-formed and returns its major type. address
        // // is modified to be just past the item, or -1 if the item is not valid.
        // int isWellFormed(int *address, bool breakable) {
        //   uint8_t ib = EEPROM.read((*address)++);  // Initial byte
        //   uint8_t majorType = ib >> 5;  // Major type
        //   uint64_t val;
        //   switch (ib & 0x1f) {  // Additional information
        //     case 24:
        //       val = EEPROM.read((*address)++);
        //       break;
        //     case 25:
        //       val =
        //           (uint16_t{EEPROM.read(*address + 0)} << 8) |
        //           (uint16_t{EEPROM.read(*address + 1)});
        //       *address += 2;
        //       break;
        //     case 26:
        //       val =
        //           (uint32_t{EEPROM.read(*address + 0)} << 24) |
        //           (uint32_t{EEPROM.read(*address + 1)} << 16) |
        //           (uint32_t{EEPROM.read(*address + 2)} << 8) |
        //           (uint32_t{EEPROM.read(*address + 3)});
        //       *address += 4;
        //       break;
        //     case 27:
        //       val =
        //           (uint64_t{EEPROM.read(*address + 0)} << 56) |
        //           (uint64_t{EEPROM.read(*address + 1)} << 48) |
        //           (uint64_t{EEPROM.read(*address + 2)} << 40) |
        //           (uint64_t{EEPROM.read(*address + 3)} << 32) |
        //           (uint64_t{EEPROM.read(*address + 4)} << 24) |
        //           (uint64_t{EEPROM.read(*address + 5)} << 16) |
        //           (uint64_t{EEPROM.read(*address + 6)} << 8) |
        //           (uint64_t{EEPROM.read(*address + 7)});
        //       *address += 4;
        //       break;
        //     case 28: case 29: case 30:
        //       return -1;
        //     case 31:
        //       return isIndefiniteWellFormed(address, majorType, breakable);
        //     default:
        //       return -1;
        //   }
        //
        //   switch (majorType) {  // Major type
        //     // No content for 0, 1, or 7
        //     case 0:  // Unsigned integer
        //     case 1:  // Negative integer
        //     case 7:  // Floating-point numbers and simple data types
        //       break;
        //     case 2:  // Byte string
        //     case 3:  // Text string (UTF-8)
        //       *address += val;
        //       break;
        //     case 5:  // Map
        //       // Check for overflow
        //       if (2*val <= val) {
        //         return -1;
        //       }
        //       val <<= 1;
        //       // fallthrough
        //     case 4:  // Array
        //       if (val <= 0xffffffff) {
        //         for (uint32_t i = 0, max = static_cast<uint32_t>(val); i < max; i++) {
        //           if (isWellFormed(address, breakable) < 0) {
        //             return -1;
        //           }
        //         }
        //       } else {
        //         for (uint64_t i = 0; i < val; i++) {
        //           if (isWellFormed(address, breakable) < 0) {
        //             return -1;
        //           }
        //         }
        //       }
        //       break;
        //     case 6:
        //       if (isWellFormed(address, breakable) < 0) {
        //         return -1;
        //       }
        //       break;
        //     default:
        //       return -1;
        //   }
        //   return address;
        // }
        //
        // int isIndefiniteWellFormed(int *address, uint8_t majorType, bool breakable) {
        //   switch (majorType) {
        //     case 2:  // Byte string
        //     case 3:  // Text string (UTF-8)
        //       while (true) {
        //         int itemType = isWellFormed(address, true);
        //         if (itemType != majorType) {
        //           return -1;
        //         }
        //       }
        //       break;
        //     case 4:  // Array
        //
        //     case 5:  // Map
        //     case 7:  // Floating-point numbers and simple data types
        //       if (breakable) {
        //         return -2;
        //       }
        //       return -1;
        //   }
        //   return address;
        // }

    } // namespace cbor
} // namespace qindesign
