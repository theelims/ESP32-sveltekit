// CBOR.h defines CBOR functions.
// See [Concise Binary Object Representation (CBOR)](https://tools.ietf.org/html/rfc7049)
// This is part of libCBOR.
// (c) 2017 Shawn Silverman

#ifndef QINDESIGN_CBOR_H_
#define QINDESIGN_CBOR_H_

#include <Print.h>
#include <Stream.h>

namespace qindesign
{
    namespace cbor
    {

        // Tag that can be used to self-describe a CBOR item.
        constexpr uint16_t kSelfDescribeTag = 55799;

        enum class DataType
        {
            kUnsignedInt,
            kNegativeInt,
            kBytes,
            kText,
            kArray,
            kMap,
            kTag,
            kFloat,
            kDouble,
            kBoolean,
            kNull,
            kUndefined,
            kBreak,
            kSimpleValue,
            kEOS,
            kSyntaxError,
        };

        enum class SyntaxError
        {
            kNoError,
            kUnknownAdditionalInfo,
            kNotAnIndefiniteType,
            kBadSimpleValue,
        };

        // Reader provides a way to parse bytes in a CBOR-encoded stream. This
        // class only provides rudimentary parsing for data items; callers will
        // have to manage nested depths themselves.
        //
        // The readXXX() functions advance the stream, while the getXXX() functions
        // do not.
        //
        // If any of the getXXX() functions are called and the type does not match,
        // then a default value of zero or false is returned.
        class Reader : public Stream
        {
        public:
            Reader(Stream &in)
                : state_(State::kStart),
                  in_(in),
                  initialByte_(0),
                  majorType_(0),
                  addlInfo_(0),
                  waitAvailable_(0),
                  value_(0),
                  syntaxError_(SyntaxError::kNoError),
                  bytesAvailable_(0),
                  readSize_(0) {}
            ~Reader() = default;

            // Returns any read error in the underlying Stream object. This will return
            // zero if there is no error or if the platform does not support this. An
            // error may have occurred if readBytes returns zero.
            int getReadError()
            {
#if defined(TEENSYDUINO)
                return in_.getReadError();
#else
                return 0;
#endif
            }

            // Reads the data type of the next data item. This returns DataType::kEOS
            // if the end of stream has been reached. Otherwise, this guarantees that
            // all the bytes necessary for any attached value are available from the
            // stream.
            //
            // Once the type is known, one of the getXXX() functions can be called
            // to retrieve any immediate values. If this is called and returns
            // DataType::kEOS, then the data from any of the getXXX() functions will
            // be invalid.
            //
            // For bytes, text, arrays, and maps, only the length will be retrieved;
            // further readBytes or readByte calls may be necessary.
            //
            // This advances the stream, unlike getDataType().
            DataType readDataType();

            // Gets the current data type. This does not advance the stream, unlike
            // readDataType(). In order to return something valid, note that
            // readDataType() must be called prior. This is not the same as a peek
            // function.
            DataType getDataType() const;

            // Reads data for bytes or text. It is up to the caller to read the correct
            // number of bytes, and also to concatenate any definite-length portions
            // of an indefinite-length byte or text stream.
            //
            // This will only read bytes and advance the stream if there are bytes
            // available for the current Bytes or Text data item. In other words, this
            // does not let the caller read past the number of bytes available in this
            // data item. This will return 0 if there are no more bytes in the current
            // data item, or if the underlying stream has reached end-of-stream.
            //
            // Note that it is possible for the underlying stream to reach
            // end-of-stream with bytes still available for the current data item.
            //
            // This follows the same contract as Stream::readBytes. The read error will
            // probably have been set if end-of-stream for the underlying stream was
            // reached.
            //
            // Use bytesAvailable() to determine how many bytes are actually available
            // for this data item.
            size_t readBytes(uint8_t *buffer, size_t length);

            // Reads data for bytes or text. It is up to the caller to read the correct
            // number of bytes, and also to concatenate any definite-length portions
            // of an indefinite-length byte or text stream.
            //
            // This will only read a byte and advance the stream if there are bytes
            // available for the current Bytes or Text data item. In other words, this
            // does not let the caller read past the number of bytes available in this
            // data item. This will return -1 if there are no more bytes in the current
            // data item, or if the underlying stream has reached end-of-stream.
            //
            // Note that it is possible for the underlying stream to reach
            // end-of-stream and for this to return -1, and still have bytes available
            // for the current data item.
            //
            // Use bytesAvailable() to determine how many bytes are actually available
            // for this data item.
            int readByte();

            // Returns the number of bytes available for the current Bytes or Text
            // data item.
            uint64_t bytesAvailable() const
            {
                return bytesAvailable_;
            }

            // Returns the syntax error value if readDataType() returned
            // DataType::kSyntaxError.
            SyntaxError getSyntaxError() const;

            // Gets the raw value attached to the current data item. This will return
            // a length for bytes, text, arrays, and maps. For indefinite length data
            // items, this will return zero. For boolean, null, undefined, break, and
            // other simple values less than 32, this will return zero.
            uint64_t getRawValue() const;

            // Determines if the current bytes, text, array, or map has an indefinite
            // length. The end will be determined by a data item of type
            // DataType::kBreak.
            //
            // If this returns true, then the caller should keep track of nesting, and
            // true should be passed as the value for the allowBreak parameter in
            // readDataType the next time it is called.
            bool isIndefiniteLength() const;

            // Returns the length of bytes, text, arrays, or maps. For indefinite length
            // data items, this will return 0. For these types, please use the
            // isIndefiniteLength() function.
            uint64_t getLength() const;

            // Returns the current boolean value, or false if the value isn't a boolean.
            bool getBoolean() const;

            // Returns the current single-precision float value, or 0.0f if the value
            // isn't a single-precision float.
            float getFloat() const;

            // Returns the current double-precision float value, or 0.0 if the value
            // isn't a double-precision float.
            double getDouble() const;

            // Returns the current unsigned int value, or 0 if the value isn't an
            // unsigned int.
            uint64_t getUnsignedInt() const;

            // Returns the current value as a 64-bit signed integer.
            //
            // For negative integers, this returns a positive value if the value
            // cannot fit into a 64-bit signed integer. For these cases, it is as if
            // the 65th bit is set to a 1 and the value is a 65-bit negative number.
            // The isNegativeOverflow() function can detect this case.
            //
            // For unsigned integers, this will return a negative value if the
            // value doesn't fit into 63 bits. The isUnsigned() function can detect
            // this case.
            //
            // The functions, isNegativeOverflow() and isUnsigned(), can detect these
            // two cases. This way, getInt() can be used to read the majority of
            // integer values.
            int64_t getInt() const;

            // Detects the case where the value is a negative integer and it is less
            // than the smallest 64-bit negative number, or, in other words, a 65-bit
            // negative integer with the 65th bit set to 1.
            //
            // This can never be true at the same time that isUnsigned() is true.
            bool isNegativeOverflow() const;

            // Detects when an integer is supposed to be positive. This is useful for
            // when getInt() returns a negative number, but the value is actually
            // positive and fits inside an unsigned integer.
            //
            // Another way of phrasing it: This is useful for when getInt() is used
            // to retrieve signed values and the returned value is negative but the
            // actual value is positive.
            //
            // This can never be true at the same time that isNegativeOverflow()
            // is true.
            bool isUnsigned() const;

            // Gets the simple value. Note that values < 32 are technically invalid,
            // even though they are well-formed. Values: 20=False, 21=True, 22=Null,
            // 23=Undefined, 24-31=Reserved.
            uint8_t getSimpleValue() const;

            // Returns the current tag value, or 0 if the value isn't a tag.
            uint64_t getTag() const;

            // Returns whether the last read data value was a boolean False.
            bool isFalse() const;

            // Returns whether the last read data value was a boolean True.
            bool isTrue() const;

            // Returns whether the last read data value was a Null.
            bool isNull() const;

            // Returns whether the last read data value was an Undefined.
            bool isUndefined() const;

            // Returns whether the last read data value was a Break.
            bool isBreak() const;

            // Checks if the next data item is well-formed. This includes any nested
            // items and advances the stream. A data item is considered not well-formed
            // if there are syntax errors or end-of-stream has been reached before
            // processing all the data. Since this advances the stream, this works best
            // with a stream that can be reset.
            //
            // This calls yield() at the end of the function if the processor is
            // an ESP8266 or ESP32.
            //
            // This advances the read size. See getReadSize().
            bool isWellFormed();

            // Gets the number of bytes read so far.
            size_t getReadSize() const
            {
                return readSize_;
            }

            // Returns the number of bytes available in the underlying stream. This
            // follows the same contract as Stream::available().
            int available() override
            {
                return in_.available();
            }

            // Returns a byte and increments the read size if end-of-stream was
            // not reached. This follows the same contract as Stream::read().
            int read() override
            {
                int b = in_.read();
                if (b >= 0)
                {
                    readSize_++;
                }
                return b;
            }

            // Peeks at the next byte. This follows the same contract as Stream::peek().
            int peek() override
            {
                return in_.peek();
            }

            // Does nothing and returns zero. This is only here to satisfy the
            // Stream interface.
            size_t write(uint8_t b) final
            {
                return 0;
            }

            // Does nothing. This is only here to satisfy the Stream interface.
            void flush() final
            {
            }

        private:
            enum class State
            {
                kStart,
                kAdditionalInfo,
                kWaitAvailable,
                kReadValue,
                kDetermineType,
            };

            // Checks if a data item is well-formed. This returns the major type
            // on success, -1 on error, and -2 on encountering a break value. The
            // breakable parameter indicates whether we should allow the break value.
            // If it's disallowed then this will return -1 if a break value is
            // encountered. When first called, breakable should be set to false.
            //
            // The initialByte argument is the value of the next read.
            int isWellFormed(int initialByte, bool breakable);

            // Checks if an indefinite-length item is well-formed. This returns the
            // major type on success, -1 on error, and -2 on encountering a break
            // value. The breakable parameter indicates whether we should allow the
            // break value.
            int isIndefiniteWellFormed(uint8_t majorType, bool breakable);

            State state_;
            Stream &in_;

            int initialByte_;
            uint8_t majorType_;
            uint8_t addlInfo_;
            int waitAvailable_;
            uint64_t value_; // The simple or non-simple value after the major type
            SyntaxError syntaxError_;

            // Bytes remaining for readByte() or readBytes.
            uint64_t bytesAvailable_;

            size_t readSize_;
        };

        // Writer provides a way to encode data to a CBOR-encoded stream. Callers
        // need to manage proper structure themselves. If there was an error writing
        // anything to the Print stream then the write error might be set
        // (depending on the Print implementation).
        //
        // This sets the write error on any short writes.
        class Writer : public Print
        {
        public:
            Writer(Print &out)
                : out_(out),
                  writeSize_(0) {}

            ~Writer() = default;

            // Returns any write error in the underlying Print object. This will
            // return zero if there is no error.
            int getWriteError()
            {
                return out_.getWriteError();
            }

            void writeBoolean(bool b);
            void writeFloat(float f);
            void writeDouble(double d);
            void writeUnsignedInt(uint64_t u);
            void writeInt(int64_t i);
            void writeNull();
            void writeUndefined();
            void writeSimpleValue(uint8_t v);
            void writeTag(uint64_t v);

            // Writes bytes to the output. This must be preceded by a call to
            // beginBytes, beginIndefiniteBytes, beginText, or beginIndefiniteText.
            // endIndefinite must be called when finished writing bytes preceded by
            // a beginIndefiniteBytes or beginIndefiniteText call. Finally, if
            // beginBytes or beginText was called, then it is up to the caller to
            // write enough bytes so that the total size matches the length given
            // in beginBytes or beginText.
            //
            // This returns the number of bytes actually written. For short writes,
            // the write error will be set.
            void writeBytes(const uint8_t *buffer, size_t length);

            // Writes a single byte to the output. See the docs for writeBytes for
            // more information.
            //
            // The write error will be set for short writes.
            void writeByte(uint8_t b);

            // Starts a byte string having a specific length. It is up to the caller
            // to write the correct total number of bytes.
            void beginBytes(unsigned int length);

            // Starts a text (UTF-8) string having a specific length. It is up to the
            // caller to write the correct total number of bytes.
            void beginText(unsigned int length);

            // Starts a byte string having no specific length. It is up to the caller
            // to write the correct total number of bytes by calling beginBytes followed
            // by an appropriate number of writeBytes calls.
            //
            // When done, endIndefinite() needs to be called to end the sequence
            // of bytes items.
            void beginIndefiniteBytes();

            // Starts a text string having no specific length. It is up to the caller
            // to write the correct total number of bytes by calling beginText followed
            // by an appropriate number of writeBytes calls.
            //
            // When done, endIndefinite() needs to be called to end the sequence
            // of text items.
            void beginIndefiniteText();

            // Starts an array having a specific length. It is up to the caller
            // to write the correct number of data items.
            void beginArray(unsigned int length);

            // Starts a map having a specific length. It is up to the caller
            // to write the correct number of key/value pairs.
            void beginMap(unsigned int length);

            // Starts an array having no specific length. It is up to the caller to
            // call endIndefinite after the correct number of elements has been written.
            //
            // When done, endIndefinite() needs to be called to end the sequence
            // of items.
            void beginIndefiniteArray();

            // Starts a map having no specific length. It is up to the caller to
            // call endIndefinite after the correct number of key/value pairs has
            // been written.
            //
            // When done, endIndefinite() needs to be called to end the sequence
            // of items.
            void beginIndefiniteMap();

            // Ends an indefinite stream of bytes, text, array elements, or map pairs.
            void endIndefinite();

            // Gets the number of bytes written so far.
            size_t getWriteSize() const
            {
                return writeSize_;
            }

            // Reset writeSize to 0 so that a buffer can be reused
            void resetWriteSize()
            {
                writeSize_ = 0;
            }

            // Writes a byte and returns 1 if the write was successful, and zero
            // otherwise. This follows the same contract as Print::write(uint8_t).
            //
            // If the write is unsuccessful then this will set the write error.
            size_t write(uint8_t b) override
            {
                if (out_.write(b) == 0)
                {
                    setWriteError();
                    return 0;
                }
                writeSize_++;
                return 1;
            }

            // Writes bytes and returns the number of bytes successfully written.
            // This follows the same contract as Print::write(const uint8_t*, size_t).
            //
            // If the number of successful bytes written is less than the given size
            // then this will set the write error.
            size_t write(const uint8_t *buffer, size_t size) override
            {
                size_t written = out_.write(buffer, size);
                if (written < size)
                {
                    setWriteError();
                }
                writeSize_ += written;
                return written;
            }

#if !defined(ESP8266) && !defined(ESP32) && !defined(ARDUINO_ARCH_STM32)
            void flush() override
            {
                out_.flush();
            }
#else
            void flush()
            {
                // Print does not have flush() in ESP8266, ESP32, and STM32
            }
#endif

        private:
            // Writes an int having the given major type, either 0x20 (signed)
            // or 0x00 (unsigned).
            void writeTypedInt(uint8_t mt, uint64_t u);

            Print &out_;

            size_t writeSize_;
        };

    } // namespace cbor
} // namespace qindesign

#endif // QINDESIGN_CBOR_H_
