// bytes.inc is part of libCBOR.
// (c) 2017 Shawn Silverman

// ***************************************************************************
//  Bytes tests
// ***************************************************************************

test(bytes_empty) {
  uint8_t b[] = { (2 << 5) + 0 };
  cbor::BytesStream bs{b, sizeof(b)};
  cbor::Reader r{bs};
  assertTrue(r.isWellFormed());
  assertEqual(r.getReadSize(), size_t{1});
  bs.reset();
  assertEqual(static_cast<int>(r.readDataType()), static_cast<int>(cbor::DataType::kBytes));
  assertTrue(r.bytesAvailable() == 0);
  assertTrue(r.getLength() == 0);
  assertEqual(r.readByte(), -1);
  assertEqual(r.getReadSize(), size_t{2});
}

test(bytes_something) {
  uint8_t b[] = { (2 << 5) + 4, 0x01, 0x02, 0x03, 0x04, 0 };
  cbor::BytesStream bs{b, sizeof(b)};
  cbor::Reader r{bs};
  assertTrue(r.isWellFormed());
  assertEqual(r.getReadSize(), size_t{5});
  bs.reset();
  assertEqual(static_cast<int>(r.readDataType()), static_cast<int>(cbor::DataType::kBytes));
  assertTrue(r.bytesAvailable() == 4);
  assertTrue(r.getLength() == 4);
  uint8_t b2[5]{0};
  assertEqual(r.readBytes(b2, 4), size_t{4});
  assertEqual(reinterpret_cast<char*>(&b2[0]), reinterpret_cast<char*>(&b[1]));
  assertEqual(r.readByte(), -1);
  assertEqual(r.getReadSize(), size_t{10});
}

test(bytes_indefinite) {
  uint8_t b[] = { (2 << 5) + 31, (2 << 5) + 2, 0x01, 0x02, (2 << 5) + 3, 0x03, 0x04, 0x05, (7 << 5) + 31 };
  cbor::BytesStream bs{b, sizeof(b)};
  cbor::Reader r{bs};
  assertTrue(r.isWellFormed());
  assertEqual(r.getReadSize(), size_t{9});
  bs.reset();
  assertEqual(static_cast<int>(r.readDataType()), static_cast<int>(cbor::DataType::kBytes));
  assertTrue(r.bytesAvailable() == 0);
  assertTrue(r.isIndefiniteLength());

  uint8_t b2[6]{0};
  assertEqual(static_cast<int>(r.readDataType()), static_cast<int>(cbor::DataType::kBytes));
  assertTrue(r.bytesAvailable() == 2);
  assertTrue(r.getLength() == 2);
  assertEqual(r.readBytes(&b2[0], 2), size_t{2});
  assertEqual(r.readByte(), -1);
  assertEqual(static_cast<int>(r.readDataType()), static_cast<int>(cbor::DataType::kBytes));
  assertTrue(r.bytesAvailable() == 3);
  assertTrue(r.getLength() == 3);
  assertEqual(r.readBytes(&b2[2], 3), size_t{3});
  assertEqual(r.readByte(), -1);

  assertEqual(static_cast<int>(r.readDataType()), static_cast<int>(cbor::DataType::kBreak));
  assertTrue(r.bytesAvailable() == 0);

  uint8_t b3[] = { 0x01, 0x02, 0x03, 0x04, 0x05, 0 };
  assertEqual(reinterpret_cast<char*>(b2), reinterpret_cast<char*>(b3));
  assertEqual(r.getReadSize(), size_t{18});
}

test(bytes_indefinite_with_empty) {
  uint8_t b[] = { (2 << 5) + 31, (2 << 5) + 0, (7 << 5) + 31 };
  cbor::BytesStream bs{b, sizeof(b)};
  cbor::Reader r{bs};
  assertTrue(r.isWellFormed());
  assertEqual(r.getReadSize(), size_t{3});
  bs.reset();
  assertEqual(static_cast<int>(r.readDataType()), static_cast<int>(cbor::DataType::kBytes));
  assertTrue(r.bytesAvailable() == 0);
  assertTrue(r.isIndefiniteLength());

  assertEqual(static_cast<int>(r.readDataType()), static_cast<int>(cbor::DataType::kBytes));
  assertTrue(r.bytesAvailable() == 0);
  assertTrue(r.getLength() == 0);
  assertEqual(r.readByte(), -1);
  assertEqual(static_cast<int>(r.readDataType()), static_cast<int>(cbor::DataType::kBreak));
  assertTrue(r.bytesAvailable() == 0);
}

test(bytes_indefinite_with_empty_and_nested_indefinite) {
  uint8_t b[] = { (2 << 5) + 31, (2 << 5) + 0, (2 << 5) + 31, (7 << 5) + 31, (7 << 5) + 31 };
  cbor::BytesStream bs{b, sizeof(b)};
  cbor::Reader r{bs};
  assertFalse(r.isWellFormed());
  assertEqual(r.getReadSize(), size_t{3});
}

test(bytes_empty_indefinite) {
  uint8_t b[] = { (2 << 5) + 31, (7 << 5) + 31 };
  cbor::BytesStream bs{b, sizeof(b)};
  cbor::Reader r{bs};
  assertTrue(r.isWellFormed());
  assertEqual(r.getReadSize(), size_t{2});
  bs.reset();
  assertEqual(static_cast<int>(r.readDataType()), static_cast<int>(cbor::DataType::kBytes));
  assertTrue(r.bytesAvailable() == 0);
  assertTrue(r.isIndefiniteLength());
  assertEqual(static_cast<int>(r.readDataType()), static_cast<int>(cbor::DataType::kBreak));
  assertTrue(r.bytesAvailable() == 0);
}

test(bytes_nested_indefinite) {
  uint8_t b[] = { (2 << 5) + 31, (2 << 5) + 31, (2 << 5) + 0, (7 << 5) + 31, (7 << 5) + 31 };
  cbor::BytesStream bs{b, sizeof(b)};
  cbor::Reader r{bs};
  assertFalse(r.isWellFormed());
  assertEqual(r.getReadSize(), size_t{2});
}

test(text_empty) {
  uint8_t b[] = { (3 << 5) + 0 };
  cbor::BytesStream bs{b, sizeof(b)};
  cbor::Reader r{bs};
  assertTrue(r.isWellFormed());
  assertEqual(r.getReadSize(), size_t{1});
  bs.reset();
  assertEqual(static_cast<int>(r.readDataType()), static_cast<int>(cbor::DataType::kText));
  assertTrue(r.bytesAvailable() == 0);
  assertTrue(r.getLength() == 0);
  assertEqual(r.readByte(), -1);
  assertEqual(r.getReadSize(), size_t{2});
}

test(text_something) {
  uint8_t b[] = { (3 << 5) + 4, 0x49, 0x45, 0x54, 0x46, 0 };
  cbor::BytesStream bs{b, sizeof(b)};
  cbor::Reader r{bs};
  assertTrue(r.isWellFormed());
  assertEqual(r.getReadSize(), size_t{5});
  bs.reset();
  assertEqual(static_cast<int>(r.readDataType()), static_cast<int>(cbor::DataType::kText));
  assertTrue(r.bytesAvailable() == 4);
  assertTrue(r.getLength() == 4);
  uint8_t b2[5]{0};
  assertEqual(r.readBytes(b2, 4), size_t{4});
  assertEqual(reinterpret_cast<char*>(&b2[0]), reinterpret_cast<char*>(&b[1]));
  assertEqual(r.readByte(), -1);
  assertEqual(r.getReadSize(), size_t{10});
}

test(text_indefinite) {
  uint8_t b[] = { (3 << 5) + 31, (3 << 5) + 5, 0x73, 0x74, 0x72, 0x65, 0x61, (3 << 5) + 4, 0x6d, 0x69, 0x6e, 0x67, (7 << 5) + 31 };
  cbor::BytesStream bs{b, sizeof(b)};
  cbor::Reader r{bs};
  assertTrue(r.isWellFormed());
  assertEqual(r.getReadSize(), size_t{13});
  bs.reset();
  assertEqual(static_cast<int>(r.readDataType()), static_cast<int>(cbor::DataType::kText));
  assertTrue(r.bytesAvailable() == 0);
  assertTrue(r.isIndefiniteLength());

  uint8_t b2[10]{0};
  assertEqual(static_cast<int>(r.readDataType()), static_cast<int>(cbor::DataType::kText));
  assertTrue(r.bytesAvailable() == 5);
  assertTrue(r.getLength() == 5);
  assertEqual(r.readBytes(&b2[0], 5), size_t{5});
  assertEqual(r.readByte(), -1);
  assertEqual(static_cast<int>(r.readDataType()), static_cast<int>(cbor::DataType::kText));
  assertTrue(r.bytesAvailable() == 4);
  assertTrue(r.getLength() == 4);
  assertEqual(r.readBytes(&b2[5], 4), size_t{4});
  assertEqual(r.readByte(), -1);

  assertEqual(static_cast<int>(r.readDataType()), static_cast<int>(cbor::DataType::kBreak));
  assertTrue(r.bytesAvailable() == 0);

  uint8_t b3[] = { 0x73, 0x74, 0x72, 0x65, 0x61, 0x6d, 0x69, 0x6e, 0x67, 0 };
  assertEqual(reinterpret_cast<char*>(b2), reinterpret_cast<char*>(b3));
  assertEqual(r.getReadSize(), size_t{26});
}

test(text_indefinite_with_empty) {
  uint8_t b[] = { (3 << 5) + 31, (3 << 5) + 0, (7 << 5) + 31 };
  cbor::BytesStream bs{b, sizeof(b)};
  cbor::Reader r{bs};
  assertTrue(r.isWellFormed());
  assertEqual(r.getReadSize(), size_t{3});
  bs.reset();
  assertEqual(static_cast<int>(r.readDataType()), static_cast<int>(cbor::DataType::kText));
  assertTrue(r.bytesAvailable() == 0);
  assertTrue(r.isIndefiniteLength());

  assertEqual(static_cast<int>(r.readDataType()), static_cast<int>(cbor::DataType::kText));
  assertTrue(r.bytesAvailable() == 0);
  assertTrue(r.getLength() == 0);
  assertEqual(r.readByte(), -1);
  assertEqual(static_cast<int>(r.readDataType()), static_cast<int>(cbor::DataType::kBreak));
  assertTrue(r.bytesAvailable() == 0);
}

test(text_indefinite_with_empty_and_nested_indefinite) {
  uint8_t b[] = { (3 << 5) + 31, (3 << 5) + 0, (3 << 5) + 31, (7 << 5) + 31, (7 << 5) + 31 };
  cbor::BytesStream bs{b, sizeof(b)};
  cbor::Reader r{bs};
  assertFalse(r.isWellFormed());
  assertEqual(r.getReadSize(), size_t{3});
}

test(text_empty_indefinite) {
  uint8_t b[] = { (3 << 5) + 31, (7 << 5) + 31 };
  cbor::BytesStream bs{b, sizeof(b)};
  cbor::Reader r{bs};
  assertTrue(r.isWellFormed());
  assertEqual(r.getReadSize(), size_t{2});
  bs.reset();
  assertEqual(static_cast<int>(r.readDataType()), static_cast<int>(cbor::DataType::kText));
  assertTrue(r.bytesAvailable() == 0);
  assertTrue(r.isIndefiniteLength());
  assertEqual(static_cast<int>(r.readDataType()), static_cast<int>(cbor::DataType::kBreak));
  assertTrue(r.bytesAvailable() == 0);
}

test(text_nested_indefinite) {
  uint8_t b[] = { (3 << 5) + 31, (3 << 5) + 31, (3 << 5) + 0, (7 << 5) + 31, (7 << 5) + 31 };
  cbor::BytesStream bs{b, sizeof(b)};
  cbor::Reader r{bs};
  assertFalse(r.isWellFormed());
  assertEqual(r.getReadSize(), size_t{2});
}
