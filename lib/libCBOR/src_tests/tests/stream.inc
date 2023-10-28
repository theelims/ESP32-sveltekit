// stream.inc is part of libCBOR.
// (c) 2017 Shawn Silverman

// ***************************************************************************
//  Basic stream tests
// ***************************************************************************

test(empty_stream) {
  uint8_t b[] = { };
  cbor::BytesStream bs{b, sizeof(b)};
  cbor::Reader r{bs};
  assertFalse(r.isWellFormed());
  assertEqual(r.getReadSize(), size_t{0});
  bs.reset();
  assertEqual(static_cast<int>(r.readDataType()), static_cast<int>(cbor::DataType::kEOS));
  assertEqual(r.getReadSize(), size_t{0});
}

test(partial_1_byte) {
  uint8_t b[] = { (0 << 5) + 24 };
  cbor::BytesStream bs{b, sizeof(b)};
  cbor::Reader r{bs};
  assertFalse(r.isWellFormed());
  assertEqual(r.getReadSize(), size_t{1});
  bs.reset();
  assertEqual(static_cast<int>(r.readDataType()), static_cast<int>(cbor::DataType::kEOS));
  assertEqual(r.getReadSize(), size_t{2});
}

test(partial_2_bytes) {
  uint8_t b[] = { (0 << 5) + 25, 0x12 };
  cbor::BytesStream bs{b, sizeof(b)};
  cbor::Reader r{bs};
  assertFalse(r.isWellFormed());
  assertEqual(r.getReadSize(), size_t{1});
  bs.reset();
  assertEqual(static_cast<int>(r.readDataType()), static_cast<int>(cbor::DataType::kEOS));
  assertEqual(r.getReadSize(), size_t{2});
}

test(partial_4_bytes) {
  uint8_t b[] = { (0 << 5) + 26, 0x12, 0x34, 0x56 };
  cbor::BytesStream bs{b, sizeof(b)};
  cbor::Reader r{bs};
  assertFalse(r.isWellFormed());
  assertEqual(r.getReadSize(), size_t{1});
  bs.reset();
  assertEqual(static_cast<int>(r.readDataType()), static_cast<int>(cbor::DataType::kEOS));
  assertEqual(r.getReadSize(), size_t{2});
}

test(partial_8_bytes) {
  uint8_t b[] = { (0 << 5) + 27, 0x12, 0x34, 0x56, 0x78, 0x9a, 0xbc, 0xde };
  cbor::BytesStream bs{b, sizeof(b)};
  cbor::Reader r{bs};
  assertFalse(r.isWellFormed());
  assertEqual(r.getReadSize(), size_t{1});
  bs.reset();
  assertEqual(static_cast<int>(r.readDataType()), static_cast<int>(cbor::DataType::kEOS));
  assertEqual(r.getReadSize(), size_t{2});
}

test(unsigned_1_byte_with_wait) {
  uint8_t b[] = { (0 << 5) + 24, 0x21 };
  cbor::BytesStream bs{b, sizeof(b), 1};
  cbor::Reader r{bs};
  assertFalse(r.isWellFormed());
  assertEqual(r.getReadSize(), size_t{0});
  bs.reset();
  assertEqual(static_cast<int>(r.readDataType()), static_cast<int>(cbor::DataType::kEOS));
  assertEqual(static_cast<int>(r.readDataType()), static_cast<int>(cbor::DataType::kEOS));
  assertEqual(static_cast<int>(r.readDataType()), static_cast<int>(cbor::DataType::kUnsignedInt));
  assertTrue(r.getUnsignedInt() == 0x21);
  assertEqual(r.getReadSize(), size_t{2});
}

test(bytes_unavailable) {
  // One beyond the maximum, so we have at least one available in the stream
  uint8_t b[10];
  cbor::BytesStream bs{b, sizeof(b)};
  cbor::Reader r{bs};

  // Unsigned and negative
  bs.reset();
  b[0] = (0 << 5) + 1;
  assertTrue(r.isWellFormed());
  assertTrue(r.bytesAvailable() == 0);
  assertEqual(r.readByte(), -1);
  bs.reset();
  b[0] = (1 << 5) + 1;
  assertTrue(r.isWellFormed());
  assertTrue(r.bytesAvailable() == 0);
  assertEqual(r.readByte(), -1);

  // Array
  bs.reset();
  b[0] = (4 << 5) + 1;
  b[1] = (0 << 5) + 2;
  assertTrue(r.isWellFormed());
  assertTrue(r.bytesAvailable() == 0);
  assertEqual(r.readByte(), -1);

  // Map
  bs.reset();
  b[0] = (5 << 5) + 1;
  b[1] = (0 << 5) + 2;
  b[2] = (0 << 5) + 3;
  assertTrue(r.isWellFormed());
  assertTrue(r.bytesAvailable() == 0);
  assertEqual(r.readByte(), -1);

  // Tag
  bs.reset();
  b[0] = (6 << 5) + 1;
  assertTrue(r.isWellFormed());
  assertTrue(r.bytesAvailable() == 0);
  assertEqual(r.readByte(), -1);

  // Basic simple values
  for (int i = 0; i < 24; i++) {
    bs.reset();
    b[0] = (7 << 5) + i;
    assertTrue(r.isWellFormed());
    assertTrue(r.bytesAvailable() == 0);
    assertEqual(r.readByte(), -1);
  }

  // Simple value
  bs.reset();
  b[0] = (7 << 5) + 24;
  b[1] = 1;
  assertTrue(r.isWellFormed());
  assertTrue(r.bytesAvailable() == 0);
  assertEqual(r.readByte(), -1);

  // Half-precision float
  bs.reset();
  b[0] = (7 << 5) + 25;
  b[1] = 0;
  b[2] = 0;
  assertTrue(r.isWellFormed());
  assertTrue(r.bytesAvailable() == 0);
  assertEqual(r.readByte(), -1);

  // Single-precision float
  bs.reset();
  b[0] = (7 << 5) + 26;
  b[1] = 0;
  b[2] = 0;
  b[3] = 0;
  b[4] = 0;
  assertTrue(r.isWellFormed());
  assertTrue(r.bytesAvailable() == 0);
  assertEqual(r.readByte(), -1);

  // Double-precision float
  bs.reset();
  b[0] = (7 << 5) + 27;
  b[1] = 0;
  b[2] = 0;
  b[3] = 0;
  b[4] = 0;
  b[5] = 0;
  b[6] = 0;
  b[7] = 0;
  b[8] = 0;
  assertTrue(r.isWellFormed());
  assertTrue(r.bytesAvailable() == 0);
  assertEqual(r.readByte(), -1);

  // Bytes with a break
  bs.reset();
  b[0] = (2 << 5) + 31;
  b[1] = (7 << 5) + 31;
  assertTrue(r.isWellFormed());
  assertTrue(r.bytesAvailable() == 0);
  assertEqual(r.readByte(), -1);
}
