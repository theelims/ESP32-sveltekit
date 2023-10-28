// int.inc is part of libCBOR.
// (c) 2017 Shawn Silverman

// ***************************************************************************
//  Integer tests that don't fit into the unsigned or negative tests.
// ***************************************************************************
test(int_zero) {
  uint8_t b[] = { (0 << 5) + 0 };
  cbor::BytesStream bs{b, sizeof(b)};
  cbor::Reader r{bs};
  assertTrue(r.isWellFormed());
  assertEqual(r.getReadSize(), size_t{1});
  bs.reset();
  assertEqual(static_cast<int>(r.readDataType()), static_cast<int>(cbor::DataType::kUnsignedInt));
  assertTrue(r.getInt() == 0);
  assertEqual(r.getReadSize(), size_t{2});
  assertEqual(r.isUnsigned(), true);
  assertEqual(r.isNegativeOverflow(), false);
}

test(int_zero_neg) {
  uint8_t b[] = { (1 << 5) + 0 };
  cbor::BytesStream bs{b, sizeof(b)};
  cbor::Reader r{bs};
  assertTrue(r.isWellFormed());
  assertEqual(r.getReadSize(), size_t{1});
  bs.reset();
  assertEqual(static_cast<int>(r.readDataType()), static_cast<int>(cbor::DataType::kNegativeInt));
  assertTrue(r.getInt() == -1);
  assertEqual(r.getReadSize(), size_t{2});
  assertEqual(r.isUnsigned(), false);
  assertEqual(r.isNegativeOverflow(), false);
}

test(int_19) {
  uint8_t b[] = { (0 << 5) + 19 };
  cbor::BytesStream bs{b, sizeof(b)};
  cbor::Reader r{bs};
  assertTrue(r.isWellFormed());
  assertEqual(r.getReadSize(), size_t{1});
  bs.reset();
  assertEqual(static_cast<int>(r.readDataType()), static_cast<int>(cbor::DataType::kUnsignedInt));
  assertTrue(r.getInt() == 19);
  assertEqual(r.getReadSize(), size_t{2});
  assertEqual(r.isUnsigned(), true);
  assertEqual(r.isNegativeOverflow(), false);
}

test(int_19_neg) {
  uint8_t b[] = { (1 << 5) + 19 };
  cbor::BytesStream bs{b, sizeof(b)};
  cbor::Reader r{bs};
  assertTrue(r.isWellFormed());
  assertEqual(r.getReadSize(), size_t{1});
  bs.reset();
  assertEqual(static_cast<int>(r.readDataType()), static_cast<int>(cbor::DataType::kNegativeInt));
  assertTrue(r.getInt() == -20);
  assertEqual(r.getReadSize(), size_t{2});
  assertEqual(r.isUnsigned(), false);
  assertEqual(r.isNegativeOverflow(), false);
}

test(int_23) {
  uint8_t b[] = { (0 << 5) + 23 };
  cbor::BytesStream bs{b, sizeof(b)};
  cbor::Reader r{bs};
  assertTrue(r.isWellFormed());
  assertEqual(r.getReadSize(), size_t{1});
  bs.reset();
  assertEqual(static_cast<int>(r.readDataType()), static_cast<int>(cbor::DataType::kUnsignedInt));
  assertTrue(r.getInt() == 23);
  assertEqual(r.getReadSize(), size_t{2});
  assertEqual(r.isUnsigned(), true);
  assertEqual(r.isNegativeOverflow(), false);
}

test(int_23_neg) {
  uint8_t b[] = { (1 << 5) + 23 };
  cbor::BytesStream bs{b, sizeof(b)};
  cbor::Reader r{bs};
  assertTrue(r.isWellFormed());
  assertEqual(r.getReadSize(), size_t{1});
  bs.reset();
  assertEqual(static_cast<int>(r.readDataType()), static_cast<int>(cbor::DataType::kNegativeInt));
  assertTrue(r.getInt() == -24);
  assertEqual(r.getReadSize(), size_t{2});
  assertEqual(r.isUnsigned(), false);
  assertEqual(r.isNegativeOverflow(), false);
}

test(int_1_byte) {
  uint8_t b[] = { (0 << 5) + 24, 0x12 };
  cbor::BytesStream bs{b, sizeof(b)};
  cbor::Reader r{bs};
  assertTrue(r.isWellFormed());
  assertEqual(r.getReadSize(), size_t{2});
  bs.reset();
  assertEqual(static_cast<int>(r.readDataType()), static_cast<int>(cbor::DataType::kUnsignedInt));
  assertTrue(r.getInt() == 0x12);
  assertEqual(r.getReadSize(), size_t{4});
  assertEqual(r.isUnsigned(), true);
  assertEqual(r.isNegativeOverflow(), false);
}

test(int_1_byte_neg) {
  uint8_t b[] = { (1 << 5) + 24, 0x12 };
  cbor::BytesStream bs{b, sizeof(b)};
  cbor::Reader r{bs};
  assertTrue(r.isWellFormed());
  assertEqual(r.getReadSize(), size_t{2});
  bs.reset();
  assertEqual(static_cast<int>(r.readDataType()), static_cast<int>(cbor::DataType::kNegativeInt));
  assertTrue(r.getInt() == -0x13);
  assertEqual(r.getReadSize(), size_t{4});
  assertEqual(r.isUnsigned(), false);
  assertEqual(r.isNegativeOverflow(), false);
}

test(int_2_bytes) {
  uint8_t b[] = { (0 << 5) + 25, 0x12, 0x34 };
  cbor::BytesStream bs{b, sizeof(b)};
  cbor::Reader r{bs};
  assertTrue(r.isWellFormed());
  assertEqual(r.getReadSize(), size_t{3});
  bs.reset();
  assertEqual(static_cast<int>(r.readDataType()), static_cast<int>(cbor::DataType::kUnsignedInt));
  assertTrue(r.getInt() == 0x1234);
  assertEqual(r.getReadSize(), size_t{6});
  assertEqual(r.isUnsigned(), true);
  assertEqual(r.isNegativeOverflow(), false);
}

test(int_2_bytes_neg) {
  uint8_t b[] = { (1 << 5) + 25, 0x12, 0x34 };
  cbor::BytesStream bs{b, sizeof(b)};
  cbor::Reader r{bs};
  assertTrue(r.isWellFormed());
  assertEqual(r.getReadSize(), size_t{3});
  bs.reset();
  assertEqual(static_cast<int>(r.readDataType()), static_cast<int>(cbor::DataType::kNegativeInt));
  assertTrue(r.getInt() == -0x1235);
  assertEqual(r.getReadSize(), size_t{6});
  assertEqual(r.isUnsigned(), false);
  assertEqual(r.isNegativeOverflow(), false);
}

test(int_4_bytes) {
  uint8_t b[] = { (0 << 5) + 26, 0x12, 0x34, 0x56, 0x78 };
  cbor::BytesStream bs{b, sizeof(b)};
  cbor::Reader r{bs};
  assertTrue(r.isWellFormed());
  assertEqual(r.getReadSize(), size_t{5});
  bs.reset();
  assertEqual(static_cast<int>(r.readDataType()), static_cast<int>(cbor::DataType::kUnsignedInt));
  assertTrue(r.getInt() == 0x12345678L);
  assertEqual(r.getReadSize(), size_t{10});
  assertEqual(r.isUnsigned(), true);
  assertEqual(r.isNegativeOverflow(), false);
}

test(int_4_bytes_neg) {
  uint8_t b[] = { (1 << 5) + 26, 0x12, 0x34, 0x56, 0x78 };
  cbor::BytesStream bs{b, sizeof(b)};
  cbor::Reader r{bs};
  assertTrue(r.isWellFormed());
  assertEqual(r.getReadSize(), size_t{5});
  bs.reset();
  assertEqual(static_cast<int>(r.readDataType()), static_cast<int>(cbor::DataType::kNegativeInt));
  assertTrue(r.getInt() == -0x12345679L);
  assertEqual(r.getReadSize(), size_t{10});
  assertEqual(r.isUnsigned(), false);
  assertEqual(r.isNegativeOverflow(), false);
}

test(int_8_bytes) {
  uint8_t b[] = { (0 << 5) + 27, 0x12, 0x34, 0x56, 0x78, 0x9a, 0xbc, 0xde, 0xf0 };
  cbor::BytesStream bs{b, sizeof(b)};
  cbor::Reader r{bs};
  assertTrue(r.isWellFormed());
  assertEqual(r.getReadSize(), size_t{9});
  bs.reset();
  assertEqual(static_cast<int>(r.readDataType()), static_cast<int>(cbor::DataType::kUnsignedInt));
  assertTrue(r.getInt() == 0x123456789abcdef0LL);
  assertEqual(r.getReadSize(), size_t{18});
  assertEqual(r.isUnsigned(), true);
  assertEqual(r.isNegativeOverflow(), false);
}

test(int_8_bytes_neg) {
  uint8_t b[] = { (1 << 5) + 27, 0x12, 0x34, 0x56, 0x78, 0x9a, 0xbc, 0xde, 0xf0 };
  cbor::BytesStream bs{b, sizeof(b)};
  cbor::Reader r{bs};
  assertTrue(r.isWellFormed());
  assertEqual(r.getReadSize(), size_t{9});
  bs.reset();
  assertEqual(static_cast<int>(r.readDataType()), static_cast<int>(cbor::DataType::kNegativeInt));
  assertTrue(r.getInt() == -0x123456789abcdef1LL);
  assertEqual(r.getReadSize(), size_t{18});
  assertEqual(r.isUnsigned(), false);
  assertEqual(r.isNegativeOverflow(), false);
}
