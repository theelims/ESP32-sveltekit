// CBOR_utils.cpp is part of libCBOR.
// (c) 2017 Shawn Silverman

#include "CBOR_utils.h"

// Project includes
#include "CBOR.h"
#include "CBOR_streams.h"

namespace qindesign
{
    namespace cbor
    {

#ifdef __has_include
#if __has_include(<EEPROM.h>)
        bool isEEPROMWellFormed(size_t eepromSize, int address)
        {
            if (address < 0 || eepromSize == 0)
            {
                return false;
            }
            EEPROMStream es{eepromSize, address};
            Reader cbor{es};
            return cbor.isWellFormed();
        }
#endif
#endif

    } // namespace cbor
} // namespace qindesign
