// simple_value.inc is part of libCBOR.
// (c) 2017 Shawn Silverman

// ***************************************************************************
//  Simple-value tests
// ***************************************************************************

test(boolean_false) {
  uint8_t b[] = { (7 << 5) + 20 };
  cbor::BytesStream bs{b, sizeof(b)};
  cbor::Reader r{bs};
  assertTrue(r.isWellFormed());
  assertEqual(r.getReadSize(), size_t{1});
  bs.reset();
  assertEqual(static_cast<int>(r.readDataType()), static_cast<int>(cbor::DataType::kBoolean));
  assertEqual(r.getBoolean(), false);
  assertEqual(r.getReadSize(), size_t{2});
}

test(boolean_true) {
  uint8_t b[] = { (7 << 5) + 21 };
  cbor::BytesStream bs{b, sizeof(b)};
  cbor::Reader r{bs};
  assertTrue(r.isWellFormed());
  assertEqual(r.getReadSize(), size_t{1});
  bs.reset();
  assertEqual(static_cast<int>(r.readDataType()), static_cast<int>(cbor::DataType::kBoolean));
  assertEqual(r.getBoolean(), true);
  assertEqual(r.getReadSize(), size_t{2});
}

test(null) {
  uint8_t b[] = { (7 << 5) + 22 };
  cbor::BytesStream bs{b, sizeof(b)};
  cbor::Reader r{bs};
  assertTrue(r.isWellFormed());
  assertEqual(r.getReadSize(), size_t{1});
  bs.reset();
  assertEqual(static_cast<int>(r.readDataType()), static_cast<int>(cbor::DataType::kNull));
  assertEqual(r.getReadSize(), size_t{2});
}

test(undefined) {
  uint8_t b[] = { (7 << 5) + 23 };
  cbor::BytesStream bs{b, sizeof(b)};
  cbor::Reader r{bs};
  assertTrue(r.isWellFormed());
  assertEqual(r.getReadSize(), size_t{1});
  bs.reset();
  assertEqual(static_cast<int>(r.readDataType()), static_cast<int>(cbor::DataType::kUndefined));
  assertEqual(r.getReadSize(), size_t{2});
}

test(simple_value_break) {
  uint8_t b[] = { (7 << 5) + 31 };
  cbor::BytesStream bs{b, sizeof(b)};
  cbor::Reader r{bs};
  assertFalse(r.isWellFormed());  // TODO: Can't have just a break?
  assertEqual(r.getReadSize(), size_t{1});
  bs.reset();
  assertEqual(static_cast<int>(r.readDataType()), static_cast<int>(cbor::DataType::kBreak));
  assertEqual(r.getReadSize(), size_t{2});
}

test(simple_value_zero) {
  uint8_t b[] = { (7 << 5) + 0 };
  cbor::BytesStream bs{b, sizeof(b)};
  cbor::Reader r{bs};
  assertTrue(r.isWellFormed());
  assertEqual(r.getReadSize(), size_t{1});
  bs.reset();
  assertEqual(static_cast<int>(r.readDataType()), static_cast<int>(cbor::DataType::kSimpleValue));
  assertEqual(r.getSimpleValue(), 0);
  assertEqual(r.getReadSize(), size_t{2});
}

test(simple_value_19) {
  uint8_t b[] = { (7 << 5) + 19 };
  cbor::BytesStream bs{b, sizeof(b)};
  cbor::Reader r{bs};
  assertTrue(r.isWellFormed());
  assertEqual(r.getReadSize(), size_t{1});
  bs.reset();
  assertEqual(static_cast<int>(r.readDataType()), static_cast<int>(cbor::DataType::kSimpleValue));
  assertEqual(r.getSimpleValue(), 19);
  assertEqual(r.getReadSize(), size_t{2});
}

test (simple_value_1_byte_false) {
  uint8_t b[] = { (7 << 5) + 24, 20 };
  cbor::BytesStream bs{b, sizeof(b)};
  cbor::Reader r{bs};
  assertTrue(r.isWellFormed());
  assertEqual(r.getReadSize(), size_t{2});
  bs.reset();
  assertEqual(static_cast<int>(r.readDataType()), static_cast<int>(cbor::DataType::kSimpleValue));
  assertEqual(r.getSimpleValue(), 20);
  assertEqual(r.getReadSize(), size_t{4});
  assertFalse(r.isFalse());
  assertFalse(r.isTrue());
  assertEqual(r.getBoolean(), false);
}

test (simple_value_1_byte_true) {
  uint8_t b[] = { (7 << 5) + 24, 21 };
  cbor::BytesStream bs{b, sizeof(b)};
  cbor::Reader r{bs};
  assertTrue(r.isWellFormed());
  assertEqual(r.getReadSize(), size_t{2});
  bs.reset();
  assertEqual(static_cast<int>(r.readDataType()), static_cast<int>(cbor::DataType::kSimpleValue));
  assertEqual(r.getSimpleValue(), 21);
  assertEqual(r.getReadSize(), size_t{4});
  assertFalse(r.isTrue());
  assertFalse(r.isFalse());
  assertEqual(r.getBoolean(), false);
}

test (simple_value_1_byte_null) {
  uint8_t b[] = { (7 << 5) + 24, 22 };
  cbor::BytesStream bs{b, sizeof(b)};
  cbor::Reader r{bs};
  assertTrue(r.isWellFormed());
  assertEqual(r.getReadSize(), size_t{2});
  bs.reset();
  assertEqual(static_cast<int>(r.readDataType()), static_cast<int>(cbor::DataType::kSimpleValue));
  assertEqual(r.getSimpleValue(), 22);
  assertEqual(r.getReadSize(), size_t{4});
  assertFalse(r.isNull());
}

test (simple_value_1_byte_undefined) {
  uint8_t b[] = { (7 << 5) + 24, 23 };
  cbor::BytesStream bs{b, sizeof(b)};
  cbor::Reader r{bs};
  assertTrue(r.isWellFormed());
  assertEqual(r.getReadSize(), size_t{2});
  bs.reset();
  assertEqual(static_cast<int>(r.readDataType()), static_cast<int>(cbor::DataType::kSimpleValue));
  assertEqual(r.getSimpleValue(), 23);
  assertEqual(r.getReadSize(), size_t{4});
  assertFalse(r.isUndefined());
}

test (simple_value_1_byte_break) {
  uint8_t b[] = { (7 << 5) + 24, 31 };
  cbor::BytesStream bs{b, sizeof(b)};
  cbor::Reader r{bs};
  assertTrue(r.isWellFormed());
  assertEqual(r.getReadSize(), size_t{2});
  bs.reset();
  assertEqual(static_cast<int>(r.readDataType()), static_cast<int>(cbor::DataType::kSimpleValue));
  assertEqual(r.getSimpleValue(), 31);
  assertEqual(r.getReadSize(), size_t{4});
  assertFalse(r.isBreak());
}

test(simple_value_32_1_byte) {
  uint8_t b[] = { (7 << 5) + 24, 32 };
  cbor::BytesStream bs{b, sizeof(b)};
  cbor::Reader r{bs};
  assertTrue(r.isWellFormed());
  assertEqual(r.getReadSize(), size_t{2});
  bs.reset();
  assertEqual(static_cast<int>(r.readDataType()), static_cast<int>(cbor::DataType::kSimpleValue));
  assertEqual(r.getSimpleValue(), 32);
  assertEqual(r.getReadSize(), size_t{4});
}

test(simple_value_255_1_byte) {
  uint8_t b[] = { (7 << 5) + 24, 255 };
  cbor::BytesStream bs{b, sizeof(b)};
  cbor::Reader r{bs};
  assertTrue(r.isWellFormed());
  assertEqual(r.getReadSize(), size_t{2});
  bs.reset();
  assertEqual(static_cast<int>(r.readDataType()), static_cast<int>(cbor::DataType::kSimpleValue));
  assertEqual(r.getSimpleValue(), 255);
  assertEqual(r.getReadSize(), size_t{4});
}
