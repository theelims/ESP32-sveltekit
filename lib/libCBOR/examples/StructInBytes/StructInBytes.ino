/*
 * Demonstrates reading and writing CBOR-formatted data inside
 * a byte array.
 *
 * It is assumed that the data starts with a tag having the
 * "Self-describe CBOR" value, 55799, and continues with an
 * array.
 *
 * Data structure:
 * 1. Tag (55799)
 * 2. Array of 2 items:
 *    2.1. Boolean
 *    2.2. Text (max. 10 bytes)
 *
 * Note that this is an example program that shows a few techniques for
 * processing CBOR data and is not a complete program. This uses bytes
 * instead of EEPROM so that it doesn't contribute to wearing down that
 * memory. See EEPROMStream and EEPROMPrint for other stream options.
 *
 * (c) 2017 Shawn Silverman
 */

#include <cstring>

#include <CBOR.h>
#include <CBOR_parsing.h>
#include <CBOR_streams.h>

namespace cbor = ::qindesign::cbor;

// Choose an array size of 256 and a starting address of zero
constexpr size_t kBytesSize = 256;
constexpr int kStartAddress = 0;

// Read from and write to a byte array. Note that both BytesStream
// and BytesPrint are resettable to the start of the array. The streams
// don't necessarily have to be pre-declared here; they can be declared
// where they're used, also.
uint8_t bytes[kBytesSize]{0};
cbor::BytesStream bs{bytes, sizeof(bytes)};
cbor::BytesPrint bp{bytes, sizeof(bytes)};

// MyData contains important data.
struct MyData {
  bool flag = false;
  size_t textLen = 0;
  uint8_t text[10]{0};  // UTF-8 encoded
} myData;

// Forward declarations
bool loadMyData(MyData *myData);
void storeMyData(const MyData &myData);

void setup() {
  // Standard serial initialization
  Serial.begin(115200);
  while (!Serial) {
    ;
  }
  delay(2000);  // Wait for the serial monitor come up

  // Load the data and if there's not already something there, store
  // a default
  if (!loadMyData(&myData)) {
    Serial.println("Malformed structure in memory. Writing default.");
    myData.flag = true;
    myData.textLen = 2;
    myData.text[0] = 'A';
    myData.text[1] = 'B';
    storeMyData(myData);
  }

  // For illustration, here's one way to load and compare
  // Could also define operator== and operator!= for the structure
  MyData d2;
  if (!loadMyData(&d2)) {
    Serial.println("Malformed new structure. Something's wrong!");
  } else {
    bool match = true;
    if (d2.flag != myData.flag) {
      match = false;
      Serial.println("Mismatch: flag");
    }
    if (d2.textLen != myData.textLen) {
      match = false;
      Serial.println("Mismatch: textLen; not testing text");
    } else {
      if (memcmp(d2.text, myData.text, d2.textLen) != 0) {
        match = false;
        Serial.println("Mismatch: text");
      }
    }
    if (match) {
      Serial.println("Everything matches!");
    }
  }
}

void loop() {
  // Do stuff here
}

// Loads data from 'bytes' into the given structure. This returns true
// if valid data was found and loaded, and false otherwise.
bool loadMyData(MyData *myData) {
  cbor::Reader cbor{bs};

  // The following reset() call is only necessary if we don't know
  // the position of the stream, and if we wanted to reset it
  // to the beginning
  bs.reset();

  // First check if things are well-formed
  if (!cbor.isWellFormed()) {
    Serial.println("Not well-formed CBOR data.");
    return false;
  }

  // Reset so that we start reading from the beginning
  bs.reset();

  // Assume the data starts with the "Self-describe CBOR" tag and continues
  // with an array consisting of one boolean item and one text item.
  if (!expectValue(cbor, cbor::DataType::kTag, cbor::kSelfDescribeTag)) {
    return false;
  }
  if (!expectValue(cbor, cbor::DataType::kArray, 2)) {
    return false;
  }
  if (!expectBoolean(cbor, &myData->flag)) {
    return false;
  }
  uint64_t length;
  bool isIndefinite;
  if (!expectText(cbor, &length, &isIndefinite) || isIndefinite) {
    return false;
  }
  if (length > 10) {
    return false;
  }
  myData->textLen = static_cast<size_t>(length);
  if (readFully(cbor, myData->text, static_cast<size_t>(length)) < length) {
    return false;
  }

  return true;
}

// Stores data into 'bytes from the given structure.
void storeMyData(const MyData &myData) {
  cbor::Writer cbor{bp};

  // The following reset() call is only necessary if we don't know
  // the position of the printer, and if we wanted to reset it
  // to the beginning
  bp.reset();

  cbor.writeTag(cbor::kSelfDescribeTag);
  cbor.beginArray(2);
  cbor.writeBoolean(myData.flag);
  cbor.beginText(myData.textLen);
  cbor.writeBytes(myData.text, myData.textLen);
}
