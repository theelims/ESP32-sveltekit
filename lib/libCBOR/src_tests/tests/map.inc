// map.inc is part of libCBOR.
// (c) 2017 Shawn Silverman

// ***************************************************************************
//  Map tests
// ***************************************************************************

test(map_empty) {
  uint8_t b[] = { (5 << 5) + 0 };
  cbor::BytesStream bs{b, sizeof(b)};
  cbor::Reader r{bs};
  assertTrue(r.isWellFormed());
  assertEqual(r.getReadSize(), size_t{1});
  bs.reset();
  assertEqual(static_cast<int>(r.readDataType()), static_cast<int>(cbor::DataType::kMap));
  assertTrue(r.getLength() == 0);
  assertEqual(r.getReadSize(), size_t{2});
}

test(map_indefinite_empty) {
  uint8_t b[] = { (5 << 5) + 31, (7 << 5) + 31 };
  cbor::BytesStream bs{b, sizeof(b)};
  cbor::Reader r{bs};
  assertTrue(r.isWellFormed());
  assertEqual(r.getReadSize(), size_t{2});
  bs.reset();
  assertEqual(static_cast<int>(r.readDataType()), static_cast<int>(cbor::DataType::kMap));
  assertTrue(r.isIndefiniteLength());
  assertEqual(static_cast<int>(r.readDataType()), static_cast<int>(cbor::DataType::kBreak));
  assertEqual(r.getReadSize(), size_t{4});
}

test(map_something) {
  uint8_t b[] = { (5 << 5) + 2, 0x01, 0x02, (1 << 5) | 0x03, (0 << 5) + 24, 24 };
  cbor::BytesStream bs{b, sizeof(b)};
  cbor::Reader r{bs};
  assertTrue(r.isWellFormed());
  assertEqual(r.getReadSize(), size_t{6});
  bs.reset();
  assertEqual(static_cast<int>(r.readDataType()), static_cast<int>(cbor::DataType::kMap));
  assertTrue(r.getLength() == 2);

  assertEqual(static_cast<int>(r.readDataType()), static_cast<int>(cbor::DataType::kUnsignedInt));
  assertTrue(r.getUnsignedInt() == 1);
  assertEqual(static_cast<int>(r.readDataType()), static_cast<int>(cbor::DataType::kUnsignedInt));
  assertTrue(r.getUnsignedInt() == 2);
  assertEqual(static_cast<int>(r.readDataType()), static_cast<int>(cbor::DataType::kNegativeInt));
  assertTrue(r.getInt() == -4);
  assertEqual(static_cast<int>(r.readDataType()), static_cast<int>(cbor::DataType::kUnsignedInt));
  assertTrue(r.getUnsignedInt() == 24);
  assertEqual(r.getReadSize(), size_t{12});
}

test(map_indefinite) {
  uint8_t b[] = { (5 << 5) + 31, 0x01, 0x02, (1 << 5) | 0x03, (0 << 5) + 24, 24, (7 << 5) + 31 };
  cbor::BytesStream bs{b, sizeof(b)};
  cbor::Reader r{bs};
  assertTrue(r.isWellFormed());
  assertEqual(r.getReadSize(), size_t{7});
  bs.reset();
  assertEqual(static_cast<int>(r.readDataType()), static_cast<int>(cbor::DataType::kMap));
  assertTrue(r.isIndefiniteLength());

  assertEqual(static_cast<int>(r.readDataType()), static_cast<int>(cbor::DataType::kUnsignedInt));
  assertTrue(r.getUnsignedInt() == 1);
  assertEqual(static_cast<int>(r.readDataType()), static_cast<int>(cbor::DataType::kUnsignedInt));
  assertTrue(r.getUnsignedInt() == 2);
  assertEqual(static_cast<int>(r.readDataType()), static_cast<int>(cbor::DataType::kNegativeInt));
  assertTrue(r.getInt() == -4);
  assertEqual(static_cast<int>(r.readDataType()), static_cast<int>(cbor::DataType::kUnsignedInt));
  assertTrue(r.getUnsignedInt() == 24);

  assertEqual(static_cast<int>(r.readDataType()), static_cast<int>(cbor::DataType::kBreak));
  assertEqual(r.getReadSize(), size_t{14});
}
