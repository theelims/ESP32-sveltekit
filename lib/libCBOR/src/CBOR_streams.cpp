// CBOR_streams.cpp is part of libCBOR.
// (c) 2017 Shawn Silverman

#include "CBOR_streams.h"

// Other includes
#ifdef __has_include
#if __has_include(<EEPROM.h>)
#include <EEPROM.h>
#endif
#endif

namespace qindesign
{
    namespace cbor
    {

        int BytesStream::available()
        {
            if (index_ >= length_)
            {
                return 0;
            }
            if (waiting_ > 0)
            {
                waiting_--;
                return 0;
            }
            return length_ - index_;
        }

        int BytesStream::read()
        {
            if (index_ >= length_)
            {
                return -1;
            }
            if (waiting_ > 0)
            {
                waiting_--;
                return -1;
            }
            waiting_ = waitStates_;
            return b_[index_++];
        }

        int BytesStream::peek()
        {
            if (index_ >= length_)
            {
                return -1;
            }
            if (waiting_ > 0)
            {
                waiting_--;
                return -1;
            }
            return b_[index_];
        }

        size_t BytesPrint::write(uint8_t b)
        {
            if (index_ < size_)
            {
                buf_[index_++] = b;
                return 1;
            }
            setWriteError();
            return 0;
        }

#ifdef __has_include
#if __has_include(<EEPROM.h>)
        int EEPROMStream::available()
        {
            if (static_cast<unsigned int>(address_) >= size_)
            {
                return 0;
            }
            return size_ - address_;
        }

        int EEPROMStream::read()
        {
            if (static_cast<unsigned int>(address_) >= size_)
            {
                return -1;
            }
            return EEPROM.read(address_++);
        }

        int EEPROMStream::peek()
        {
            if (static_cast<unsigned int>(address_) >= size_)
            {
                return -1;
            }
            return EEPROM.read(address_);
        }

        size_t EEPROMPrint::write(uint8_t b)
        {
            if (static_cast<unsigned int>(address_) < size_)
            {
#if !defined(ESP8266) && !defined(ESP32)
                EEPROM.update(address_++, b);
#else
                if (EEPROM.read(address_) != b)
                {
                    EEPROM.write(address_, b);
                }
                address_++;
#endif
                return 1;
            }
            setWriteError();
            return 0;
        }

        void EEPROMPrint::flush()
        {
        }
#endif
#endif

    } // namespace cbor
} // namespace qindesign
