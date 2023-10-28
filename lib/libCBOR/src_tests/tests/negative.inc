// negative.inc is part of libCBOR.
// (c) 2017 Shawn Silverman

// ***************************************************************************
//  Negative tests
// ***************************************************************************

test(negative_zero) {
  uint8_t b[] = { (1 << 5) + 0 };
  cbor::BytesStream bs{b, sizeof(b)};
  cbor::Reader r{bs};
  assertTrue(r.isWellFormed());
  assertEqual(r.getReadSize(), size_t{1});
  bs.reset();
  assertEqual(static_cast<int>(r.readDataType()), static_cast<int>(cbor::DataType::kNegativeInt));
  assertTrue(r.getInt() == -1);
  assertEqual(r.getReadSize(), size_t{2});
  assertEqual(r.isNegativeOverflow(), false);
  assertEqual(r.isUnsigned(), false);
}

test(negative_19) {
  uint8_t b[] = { (1 << 5) + 19 };
  cbor::BytesStream bs{b, sizeof(b)};
  cbor::Reader r{bs};
  assertTrue(r.isWellFormed());
  assertEqual(r.getReadSize(), size_t{1});
  bs.reset();
  assertEqual(static_cast<int>(r.readDataType()), static_cast<int>(cbor::DataType::kNegativeInt));
  assertTrue(r.getInt() == -20);
  assertEqual(r.getReadSize(), size_t{2});
  assertEqual(r.isNegativeOverflow(), false);
  assertEqual(r.isUnsigned(), false);
}

test(negative_23) {
  uint8_t b[] = { (1 << 5) + 23 };
  cbor::BytesStream bs{b, sizeof(b)};
  cbor::Reader r{bs};
  assertTrue(r.isWellFormed());
  assertEqual(r.getReadSize(), size_t{1});
  bs.reset();
  assertEqual(static_cast<int>(r.readDataType()), static_cast<int>(cbor::DataType::kNegativeInt));
  assertTrue(r.getInt() == -24);
  assertEqual(r.getReadSize(), size_t{2});
  assertEqual(r.isNegativeOverflow(), false);
  assertEqual(r.isUnsigned(), false);
}

test(negative_1_byte) {
  uint8_t b[] = { (1 << 5) + 24, 0x12 };
  cbor::BytesStream bs{b, sizeof(b)};
  cbor::Reader r{bs};
  assertTrue(r.isWellFormed());
  assertEqual(r.getReadSize(), size_t{2});
  bs.reset();
  assertEqual(static_cast<int>(r.readDataType()), static_cast<int>(cbor::DataType::kNegativeInt));
  assertTrue(r.getInt() == -0x13);
  assertEqual(r.getReadSize(), size_t{4});
  assertEqual(r.isNegativeOverflow(), false);
  assertEqual(r.isUnsigned(), false);
}

test(negative_2_bytes) {
  uint8_t b[] = { (1 << 5) + 25, 0x12, 0x34 };
  cbor::BytesStream bs{b, sizeof(b)};
  cbor::Reader r{bs};
  assertTrue(r.isWellFormed());
  assertEqual(r.getReadSize(), size_t{3});
  bs.reset();
  assertEqual(static_cast<int>(r.readDataType()), static_cast<int>(cbor::DataType::kNegativeInt));
  assertTrue(r.getInt() == -0x1235);
  assertEqual(r.getReadSize(), size_t{6});
  assertEqual(r.isNegativeOverflow(), false);
  assertEqual(r.isUnsigned(), false);
}

test(negative_4_bytes) {
  uint8_t b[] = { (1 << 5) + 26, 0x12, 0x34, 0x56, 0x78 };
  cbor::BytesStream bs{b, sizeof(b)};
  cbor::Reader r{bs};
  assertTrue(r.isWellFormed());
  assertEqual(r.getReadSize(), size_t{5});
  bs.reset();
  assertEqual(static_cast<int>(r.readDataType()), static_cast<int>(cbor::DataType::kNegativeInt));
  assertTrue(r.getInt() == -0x12345679L);
  assertEqual(r.getReadSize(), size_t{10});
  assertEqual(r.isNegativeOverflow(), false);
  assertEqual(r.isUnsigned(), false);
}

test(negative_8_bytes) {
  uint8_t b[] = { (1 << 5) + 27, 0x12, 0x34, 0x56, 0x78, 0x9a, 0xbc, 0xde, 0xf0 };
  cbor::BytesStream bs{b, sizeof(b)};
  cbor::Reader r{bs};
  assertTrue(r.isWellFormed());
  assertEqual(r.getReadSize(), size_t{9});
  bs.reset();
  assertEqual(static_cast<int>(r.readDataType()), static_cast<int>(cbor::DataType::kNegativeInt));
  assertTrue(r.getInt() == -0x123456789abcdef1LL);
  assertEqual(r.getReadSize(), size_t{18});
  assertEqual(r.isNegativeOverflow(), false);
  assertEqual(r.isUnsigned(), false);
}

test(negative_min) {
  uint8_t b[] = { (1 << 5) + 27, 0x7f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff };
  cbor::BytesStream bs{b, sizeof(b)};
  cbor::Reader r{bs};
  assertTrue(r.isWellFormed());
  assertEqual(r.getReadSize(), size_t{9});
  bs.reset();
  assertEqual(static_cast<int>(r.readDataType()), static_cast<int>(cbor::DataType::kNegativeInt));
  assertTrue(r.getInt() == (-0x7fffffffffffffffLL - 1LL));
  assertEqual(r.getReadSize(), size_t{18});
  assertEqual(r.isNegativeOverflow(), false);
  assertEqual(r.isUnsigned(), false);
}

test(negative_overflow_1) {
  uint8_t b[] = { (1 << 5) + 27, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
  cbor::BytesStream bs{b, sizeof(b)};
  cbor::Reader r{bs};
  assertTrue(r.isWellFormed());
  assertEqual(r.getReadSize(), size_t{9});
  bs.reset();
  assertEqual(static_cast<int>(r.readDataType()), static_cast<int>(cbor::DataType::kNegativeInt));
  assertTrue(r.getInt() == 0x7fffffffffffffffLL);
  assertEqual(r.getReadSize(), size_t{18});
  assertEqual(r.isNegativeOverflow(), true);
  assertEqual(r.isUnsigned(), false);
}

test(negative_overflow_max) {
  uint8_t b[] = { (1 << 5) + 27, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff };
  cbor::BytesStream bs{b, sizeof(b)};
  cbor::Reader r{bs};
  assertTrue(r.isWellFormed());
  assertEqual(r.getReadSize(), size_t{9});
  bs.reset();
  assertEqual(static_cast<int>(r.readDataType()), static_cast<int>(cbor::DataType::kNegativeInt));
  assertTrue(r.getInt() == 0LL);
  assertEqual(r.getReadSize(), size_t{18});
  assertEqual(r.isNegativeOverflow(), true);
  assertEqual(r.isUnsigned(), false);
}
