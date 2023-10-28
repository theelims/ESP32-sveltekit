// CBOR_parsing.cpp is part of libCBOR.
// (c) 2017 Shawn Silverman

#include "CBOR_parsing.h"

// C++ includes
#ifdef __has_include
#if __has_include(<climits>)
#include <climits>
#else
#include <limits.h>
#endif
#if __has_include(<cmath>)
#include <cmath>
#else
#include <math.h>
#endif
#else
#include <climits>
#include <cmath>
#endif

// Other includes
#include <Arduino.h>

namespace qindesign
{
    namespace cbor
    {

        // ***************************************************************************
        //  Expectation functions.
        // ***************************************************************************

        bool expectValue(Reader &r, DataType dt, uint64_t val)
        {
            if (r.readDataType() != dt)
            {
                return false;
            }
            switch (dt)
            {
            case DataType::kUnsignedInt:
                return r.getUnsignedInt() == val;
            case DataType::kNegativeInt:
                return r.getInt() == static_cast<int64_t>(val);
            case DataType::kSimpleValue:
                return r.getSimpleValue() == val;
            case DataType::kTag:
                return r.getTag() == val;
            case DataType::kBytes:
            case DataType::kText:
            case DataType::kArray:
            case DataType::kMap:
                return r.getLength() == val;
            default:
                return false;
            }
        }

        bool expectUnsignedIntValue(Reader &r, uint64_t u)
        {
            return (r.readDataType() == DataType::kUnsignedInt) &&
                   r.getUnsignedInt() == u;
        }

        bool expectIntValue(Reader &r, int64_t i)
        {
            switch (r.readDataType())
            {
            case DataType::kNegativeInt:
            case DataType::kUnsignedInt:
                break;
            default:
                return false;
            }
            return r.getInt() == i &&
                   !(i >= 0 && r.isNegativeOverflow()) &&
                   !(i < 0 && r.isUnsigned());
        }

        bool expectBytesLength(Reader &r, uint64_t len)
        {
            return (r.readDataType() == DataType::kBytes) && r.getLength() == len;
        }

        bool expectTextLength(Reader &r, uint64_t len)
        {
            return (r.readDataType() == DataType::kText) && r.getLength() == len;
        }

        bool expectArrayLength(Reader &r, uint64_t len)
        {
            return (r.readDataType() == DataType::kArray) && r.getLength() == len;
        }

        bool expectMapLength(Reader &r, uint64_t len)
        {
            return (r.readDataType() == DataType::kMap) && r.getLength() == len;
        }

        bool expectFloatValue(Reader &r, float f)
        {
            if (r.readDataType() != DataType::kFloat)
            {
                return false;
            }
            float v = r.getFloat();
            if (std::isnan(f))
            {
                return std::isnan(v);
            }
            // If we wanted zero and negative zero to not match, use this:
            // if (f == 0.0f) {
            //   return (v == 0) && (std::signbit(v) == std::signbit(f));
            // }
            return v == f;
        }

        bool expectDoubleValue(Reader &r, double d)
        {
            if (r.readDataType() != DataType::kDouble)
            {
                return false;
            }
            double v = r.getDouble();
            if (std::isnan(d))
            {
                return std::isnan(v);
            }
            // If we wanted zero and negative zero to not match, use this:
            // if (d == 0.0) {
            //   return (v == 0) && (std::signbit(v) == std::signbit(d));
            // }
            return v == d;
        }

        bool expectBooleanValue(Reader &r, bool b)
        {
            return (r.readDataType() == DataType::kBoolean) && r.getBoolean() == b;
        }

        bool expectTrue(Reader &r)
        {
            return expectBooleanValue(r, true);
        }

        bool expectFalse(Reader &r)
        {
            return expectBooleanValue(r, false);
        }

        bool expectTagValue(Reader &r, uint64_t val)
        {
            return (r.readDataType() == DataType::kTag) && r.getTag() == val;
        }

        bool expectUnsignedInt(Reader &r, uint64_t *u)
        {
            if (r.readDataType() != DataType::kUnsignedInt)
            {
                return false;
            }
            *u = r.getUnsignedInt();
            return true;
        }

        bool expectInt(Reader &r, int64_t *i)
        {
            switch (r.readDataType())
            {
            case DataType::kNegativeInt:
            case DataType::kUnsignedInt:
                break;
            default:
                return false;
            }
            *i = r.getInt();
            return true;
        }

        bool expectBytes(Reader &r, uint64_t *length, bool *isIndefinite)
        {
            if (r.readDataType() != DataType::kBytes)
            {
                return false;
            }
            if (r.isIndefiniteLength())
            {
                *isIndefinite = true;
                *length = 0;
            }
            else
            {
                *isIndefinite = false;
                *length = r.getLength();
            }
            return true;
        }

        bool expectDefiniteBytes(Reader &r, const uint8_t *b, uint64_t len)
        {
            if (r.readDataType() != DataType::kBytes ||
                r.isIndefiniteLength() ||
                r.getLength() != len)
            {
                return false;
            }

            if (b == nullptr)
            {
                return true;
            }

            if (len <= INT_MAX)
            {
                for (int i = len; --i >= 0;)
                {
                    if (r.readByte() != *(b++))
                    {
                        return false;
                    }
                }
            }
            else
            {
                for (uint64_t i = len; --i >= 0;)
                {
                    if (r.readByte() != *(b++))
                    {
                        return false;
                    }
                }
            }
            return true;
        }

        bool expectBytesOrBreak(Reader &r, uint64_t *length, bool *isBreak)
        {
            switch (r.readDataType())
            {
            case DataType::kBytes:
                *isBreak = false;
                *length = r.getLength();
                break;
            case DataType::kBreak:
                *isBreak = true;
                *length = 0;
                break;
            default:
                return false;
            }
            return true;
        }

        bool expectText(Reader &r, uint64_t *length, bool *isIndefinite)
        {
            if (r.readDataType() != DataType::kText)
            {
                return false;
            }
            if (r.isIndefiniteLength())
            {
                *isIndefinite = true;
                *length = 0;
            }
            else
            {
                *isIndefinite = false;
                *length = r.getLength();
            }
            return true;
        }

        bool expectDefiniteText(Reader &r, const uint8_t *b, uint64_t len)
        {
            if (r.readDataType() != DataType::kText ||
                r.isIndefiniteLength() ||
                r.getLength() != len)
            {
                return false;
            }

            if (b == nullptr)
            {
                return true;
            }

            if (len <= INT_MAX)
            {
                for (int i = len; --i >= 0;)
                {
                    if (r.readByte() != *(b++))
                    {
                        return false;
                    }
                }
            }
            else
            {
                for (uint64_t i = len; --i >= 0;)
                {
                    if (r.readByte() != *(b++))
                    {
                        return false;
                    }
                }
            }
            return true;
        }

        bool expectTextOrBreak(Reader &r, uint64_t *length, bool *isBreak)
        {
            switch (r.readDataType())
            {
            case DataType::kText:
                *isBreak = false;
                *length = r.getLength();
                break;
            case DataType::kBreak:
                *isBreak = true;
                *length = 0;
                break;
            default:
                return false;
            }
            return true;
        }

        bool expectArray(Reader &r, uint64_t *length, bool *isIndefinite)
        {
            if (r.readDataType() != DataType::kArray)
            {
                return false;
            }
            if (r.isIndefiniteLength())
            {
                *isIndefinite = true;
                *length = 0;
            }
            else
            {
                *isIndefinite = false;
                *length = r.getLength();
            }
            return true;
        }

        bool expectMap(Reader &r, uint64_t *length, bool *isIndefinite)
        {
            if (r.readDataType() != DataType::kMap)
            {
                return false;
            }
            if (r.isIndefiniteLength())
            {
                *isIndefinite = true;
                *length = 0;
            }
            else
            {
                *isIndefinite = false;
                *length = r.getLength();
            }
            return true;
        }

        bool expectBoolean(Reader &r, bool *b)
        {
            if (r.readDataType() != DataType::kBoolean)
            {
                return false;
            }
            *b = r.getBoolean();
            return true;
        }

        bool expectFloat(Reader &r, float *f)
        {
            if (r.readDataType() != DataType::kFloat)
            {
                return false;
            }
            *f = r.getFloat();
            return true;
        }

        bool expectDouble(Reader &r, double *d)
        {
            if (r.readDataType() != DataType::kDouble)
            {
                return false;
            }
            *d = r.getDouble();
            return true;
        }

        bool expectNull(Reader &r)
        {
            return (r.readDataType() == DataType::kNull);
        }

        bool expectUndefined(Reader &r)
        {
            return (r.readDataType() == DataType::kUndefined);
        }

        bool expectSimpleValue(Reader &r, uint8_t *v)
        {
            if (r.readDataType() != DataType::kSimpleValue)
            {
                return false;
            }
            *v = r.getSimpleValue();
            return true;
        }

        bool expectTag(Reader &r, uint64_t *v)
        {
            if (r.readDataType() != DataType::kTag)
            {
                return false;
            }
            *v = r.getTag();
            return true;
        }

        // ***************************************************************************
        //  Read functions.
        // ***************************************************************************

        size_t readFully(Reader &r, uint8_t *b, size_t len)
        {
            size_t count = 0;
            while (len > 0)
            {
                int read = r.readBytes(b, len);
                if (read < 0)
                {
                    return count;
                }
                count += static_cast<size_t>(read);
                len -= static_cast<size_t>(read);
            }
            return count;
        }

        DataType readUntilData(Reader &r)
        {
            while (true)
            {
                DataType dt = r.readDataType();
                if (dt == DataType::kEOS)
                {
                    yield();
                    continue;
                }
                return dt;
            }
        }

    } // namespace cbor
} // namespace qindesign
