// CBOR_streams.h contains definitions for useful Stream and
// Print implementations.
// This is part of libCBOR.
// (c) 2017 Shawn Silverman

#ifndef CBOR_STREAMS_H_
#define CBOR_STREAMS_H_

// Other includes
#include <Print.h>
#include <Stream.h>

namespace qindesign
{
    namespace cbor
    {

        // Stream implementation for a byte buffer. This intended as an input-only
        // implementation; the required Print methods do nothing.
        class BytesStream : public Stream
        {
        public:
            // Creates a new byte stream for a buffer.
            //
            // The number of wait states can be set so that end of stream is returned
            // for a certain number of calls. The default wait state count is zero.
            // This is useful for testing code that needs to handle end-of-stream
            // conditions.
            BytesStream(const uint8_t *b, size_t length, unsigned int waitStates = 0)
                : b_(b),
                  index_(0),
                  waitStates_(waitStates),
                  waiting_(waitStates)
            {
                if (b == nullptr)
                {
                    length = 0;
                }
                length_ = length;
            }

            ~BytesStream() = default;

            int available() override;
            int read() override;
            int peek() override;

            // Does nothing and returns zero.
            size_t write(uint8_t b) final
            {
                return 0;
            }

            // Does nothing.
            void flush() final
            {
            }

            // Resets the stream back to the beginning.
            void reset()
            {
                index_ = 0;
                waiting_ = waitStates_;
            }

            // Returns the current index into the byte array. This indicates how many
            // bytes were read.
            size_t getIndex() const
            {
                return index_;
            }

        private:
            const uint8_t *b_;
            size_t length_;
            size_t index_;

            const unsigned int waitStates_;
            unsigned int waiting_;
        };

        // Print implementation for a byte buffer.
        class BytesPrint : public Print
        {
        public:
            BytesPrint(uint8_t *b, size_t size)
                : buf_(b),
                  index_(0)
            {
                if (b == nullptr)
                {
                    size = 0;
                }
                size_ = size;
            }

            ~BytesPrint() = default;

            // Writes bytes to the array. This sets a write error if the array could
            // not hold the byte.
            size_t write(uint8_t b) override;

            // Resets the stream back to the beginning.
            void reset()
            {
                index_ = 0;
            }

            // Returns the current index into the byte array. This indicates how many
            // bytes were written.
            size_t getIndex() const
            {
                return index_;
            }

        private:
            uint8_t *buf_;
            size_t size_;
            size_t index_;
        };

#ifdef __has_include
#if __has_include(<EEPROM.h>)
        // Stream implementation for the EEPROM. This intended as an input-only
        // implementation; the required Print methods do nothing.
        class EEPROMStream : public Stream
        {
        public:
            // Creates a new byte stream for the EEPROM. The EEPROM size and starting
            // address are specified. If the starting address is negative then it will
            // be changed to zero.
            EEPROMStream(size_t size, int start) : size_(size)
            {
                if (start < 0)
                {
                    start = 0;
                }
                start_ = start;
                address_ = start;
            }

            ~EEPROMStream() = default;

            int available() override;
            int read() override;
            int peek() override;

            // Does nothing and returns zero.
            size_t write(uint8_t b) final
            {
                return 0;
            }

            // Does nothing.
            void flush() final
            {
            }

            // Resets the stream back to the beginning.
            void reset()
            {
                address_ = start_;
            }

            // Returns the current address. This indicates how many bytes were read.
            int getAddress() const
            {
                return address_;
            }

        private:
            const size_t size_;
            int start_;
            int address_;
        };

        // Print implementation for the EEPROM. This does not perform any
        // initialization of the EEPROM or committing of the data. Some systems
        // may require these steps before and after use of this class.
        class EEPROMPrint : public Print
        {
        public:
            EEPROMPrint(size_t size, int start) : size_(size)
            {
                if (start < 0)
                {
                    start = 0;
                }
                start_ = start;
                address_ = start;
            }

            ~EEPROMPrint() = default;

            // Writes bytes to the array. This sets a write error if the array could
            // not hold the byte.
            size_t write(uint8_t b) override;

#if !defined(ESP8266) && !defined(ESP32) && !defined(ARDUINO_ARCH_STM32)
            void flush() override;
#else
            void flush();
#endif

            // Resets the stream back to the beginning.
            void reset()
            {
                address_ = start_;
            }

            // Returns the current address. This indicates how many bytes were written.
            int getAddress() const
            {
                return address_;
            }

        private:
            const size_t size_;
            int start_;
            int address_;
        };
#endif
#endif

    } // namespace cbor
} // namespace qindesign

#endif // CBOR_STREAMS_H_
