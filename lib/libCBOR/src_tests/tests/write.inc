// write.inc is part of libCBOR.
// (c) 2017 Shawn Silverman

// ***************************************************************************
//  Write tests
// ***************************************************************************

test(write_boolean_false) {
  uint8_t b[1];
  cbor::BytesPrint bp{b, sizeof(b)};
  cbor::Writer w{bp};
  w.writeBoolean(false);
  assertEqual(w.getWriteSize(), size_t{1});

  cbor::BytesStream bs{b, sizeof(b)};
  cbor::Reader r{bs};
  assertTrue(r.isWellFormed());
  assertEqual(r.getReadSize(), size_t{1});

  uint8_t b2[] = { (7 << 5) + 20 };
  assertEqual(sizeof(b), sizeof(b2));
  for (size_t i = 0; i < sizeof(b2); i++) {
    assertEqual(b[i], b2[i]);
  }
  assertEqual(w.getWriteError(), 0);
}

test(write_boolean_true) {
  uint8_t b[1];
  cbor::BytesPrint bp{b, sizeof(b)};
  cbor::Writer w{bp};
  w.writeBoolean(true);
  assertEqual(w.getWriteSize(), size_t{1});

  cbor::BytesStream bs{b, sizeof(b)};
  cbor::Reader r{bs};
  assertTrue(r.isWellFormed());
  assertEqual(r.getReadSize(), size_t{1});

  uint8_t b2[] = { (7 << 5) + 21 };
  assertEqual(sizeof(b), sizeof(b2));
  for (size_t i = 0; i < sizeof(b2); i++) {
    assertEqual(b[i], b2[i]);
  }
  assertEqual(w.getWriteError(), 0);
}

test(write_float_negative_4p1) {
  uint8_t b[5]{0};
  cbor::BytesPrint bp{b, sizeof(b)};
  cbor::Writer w{bp};
  w.writeFloat(-4.1f);
  assertEqual(w.getWriteSize(), size_t{5});

  cbor::BytesStream bs{b, sizeof(b)};
  cbor::Reader r{bs};
  assertTrue(r.isWellFormed());
  assertEqual(r.getReadSize(), size_t{5});

  uint8_t b2[] = { (7 << 5) + 26, 0xc0, 0x83, 0x33, 0x33 };
  assertEqual(sizeof(b), sizeof(b2));
  for (size_t i = 0; i < sizeof(b2); i++) {
    assertEqual(b[i], b2[i]);
  }
  assertEqual(w.getWriteError(), 0);
}

test(write_float_100000) {
  uint8_t b[5]{0};
  cbor::BytesPrint bp{b, sizeof(b)};
  cbor::Writer w{bp};
  w.writeFloat(100000.0f);
  assertEqual(w.getWriteSize(), size_t{5});

  cbor::BytesStream bs{b, sizeof(b)};
  cbor::Reader r{bs};
  assertTrue(r.isWellFormed());
  assertEqual(r.getReadSize(), size_t{5});

  uint8_t b2[] = { (7 << 5) + 26, 0x47, 0xc3, 0x50, 0x00 };
  assertEqual(sizeof(b), sizeof(b2));
  for (size_t i = 0; i < sizeof(b2); i++) {
    assertEqual(b[i], b2[i]);
  }
  assertEqual(w.getWriteError(), 0);
}

test(write_float_large) {
  uint8_t b[5]{0};
  cbor::BytesPrint bp{b, sizeof(b)};
  cbor::Writer w{bp};
  w.writeFloat(3.4028234663852886e+38f);
  assertEqual(w.getWriteSize(), size_t{5});

  cbor::BytesStream bs{b, sizeof(b)};
  cbor::Reader r{bs};
  assertTrue(r.isWellFormed());
  assertEqual(r.getReadSize(), size_t{5});

  uint8_t b2[] = { (7 << 5) + 26, 0x7f, 0x7f, 0xff, 0xff };
  assertEqual(sizeof(b), sizeof(b2));
  for (size_t i = 0; i < sizeof(b2); i++) {
    assertEqual(b[i], b2[i]);
  }
  assertEqual(w.getWriteError(), 0);
}

test(write_float_infinity) {
  uint8_t b[5]{0};
  cbor::BytesPrint bp{b, sizeof(b)};
  cbor::Writer w{bp};
  w.writeFloat(INFINITY);
  assertEqual(w.getWriteSize(), size_t{5});

  cbor::BytesStream bs{b, sizeof(b)};
  cbor::Reader r{bs};
  assertTrue(r.isWellFormed());
  assertEqual(r.getReadSize(), size_t{5});

  uint8_t b2[] = { (7 << 5) + 26, 0x7f, 0x80, 0x00, 0x00 };
  assertEqual(sizeof(b), sizeof(b2));
  for (size_t i = 0; i < sizeof(b2); i++) {
    assertEqual(b[i], b2[i]);
  }
  assertEqual(w.getWriteError(), 0);
}

test(write_float_negative_infinity) {
  uint8_t b[5]{0};
  cbor::BytesPrint bp{b, sizeof(b)};
  cbor::Writer w{bp};
  w.writeFloat(-INFINITY);
  assertEqual(w.getWriteSize(), size_t{5});

  cbor::BytesStream bs{b, sizeof(b)};
  cbor::Reader r{bs};
  assertTrue(r.isWellFormed());
  assertEqual(r.getReadSize(), size_t{5});

  uint8_t b2[] = { (7 << 5) + 26, 0xff, 0x80, 0x00, 0x00 };
  assertEqual(sizeof(b), sizeof(b2));
  for (size_t i = 0; i < sizeof(b2); i++) {
    assertEqual(b[i], b2[i]);
  }
  assertEqual(w.getWriteError(), 0);
}

test(write_float_nan) {
  uint8_t b[5]{0};
  cbor::BytesPrint bp{b, sizeof(b)};
  cbor::Writer w{bp};
  w.writeFloat(NAN);
  assertEqual(w.getWriteSize(), size_t{5});

  cbor::BytesStream bs{b, sizeof(b)};
  cbor::Reader r{bs};
  assertTrue(r.isWellFormed());
  assertEqual(r.getReadSize(), size_t{5});

  uint8_t b2[] = { (7 << 5) + 26, 0x7f, 0xc0, 0x00, 0x00 };
  assertEqual(sizeof(b), sizeof(b2));
  for (size_t i = 0; i < sizeof(b2); i++) {
    assertEqual(b[i], b2[i]);
  }
  assertEqual(w.getWriteError(), 0);
}

test(write_double_1p1) {
  uint8_t b[9]{0};
  cbor::BytesPrint bp{b, sizeof(b)};
  cbor::Writer w{bp};
  w.writeDouble(1.1);
  assertEqual(w.getWriteSize(), size_t{9});

  cbor::BytesStream bs{b, sizeof(b)};
  cbor::Reader r{bs};
  assertTrue(r.isWellFormed());
  assertEqual(r.getReadSize(), size_t{9});

  uint8_t b2[] = { (7 << 5) + 27, 0x3f, 0xf1, 0x99, 0x99, 0x99, 0x99, 0x99, 0x9a };
  assertEqual(sizeof(b), sizeof(b2));
  for (size_t i = 0; i < sizeof(b2); i++) {
    assertEqual(b[i], b2[i]);
  }
  assertEqual(w.getWriteError(), 0);
}

test(write_double_negative_4p1) {
  uint8_t b[9]{0};
  cbor::BytesPrint bp{b, sizeof(b)};
  cbor::Writer w{bp};
  w.writeDouble(-4.1);
  assertEqual(w.getWriteSize(), size_t{9});

  cbor::BytesStream bs{b, sizeof(b)};
  cbor::Reader r{bs};
  assertTrue(r.isWellFormed());
  assertEqual(r.getReadSize(), size_t{9});

  uint8_t b2[] = { (7 << 5) + 27, 0xc0, 0x10, 0x66, 0x66, 0x66, 0x66, 0x66, 0x66 };
  assertEqual(sizeof(b), sizeof(b2));
  for (size_t i = 0; i < sizeof(b2); i++) {
    assertEqual(b[i], b2[i]);
  }
  assertEqual(w.getWriteError(), 0);
}

test(write_double_large) {
  uint8_t b[9]{0};
  cbor::BytesPrint bp{b, sizeof(b)};
  cbor::Writer w{bp};
  w.writeDouble(1.0e+300);
  assertEqual(w.getWriteSize(), size_t{9});

  cbor::BytesStream bs{b, sizeof(b)};
  cbor::Reader r{bs};
  assertTrue(r.isWellFormed());
  assertEqual(r.getReadSize(), size_t{9});

  uint8_t b2[] = { (7 << 5) + 27, 0x7e, 0x37, 0xe4, 0x3c, 0x88, 0x00, 0x75, 0x9c };
  assertEqual(sizeof(b), sizeof(b2));
  for (size_t i = 0; i < sizeof(b2); i++) {
    assertEqual(b[i], b2[i]);
  }
  assertEqual(w.getWriteError(), 0);
}

test(write_double_infinity) {
  uint8_t b[9]{0};
  cbor::BytesPrint bp{b, sizeof(b)};
  cbor::Writer w{bp};
  w.writeDouble(INFINITY);
  assertEqual(w.getWriteSize(), size_t{9});

  cbor::BytesStream bs{b, sizeof(b)};
  cbor::Reader r{bs};
  assertTrue(r.isWellFormed());
  assertEqual(r.getReadSize(), size_t{9});

  uint8_t b2[] = { (7 << 5) + 27, 0x7f, 0xf0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
  assertEqual(sizeof(b), sizeof(b2));
  for (size_t i = 0; i < sizeof(b2); i++) {
    assertEqual(b[i], b2[i]);
  }
  assertEqual(w.getWriteError(), 0);
}

test(write_double_negative_infinity) {
  uint8_t b[9]{0};
  cbor::BytesPrint bp{b, sizeof(b)};
  cbor::Writer w{bp};
  w.writeDouble(-INFINITY);
  assertEqual(w.getWriteSize(), size_t{9});

  cbor::BytesStream bs{b, sizeof(b)};
  cbor::Reader r{bs};
  assertTrue(r.isWellFormed());
  assertEqual(r.getReadSize(), size_t{9});

  uint8_t b2[] = { (7 << 5) + 27, 0xff, 0xf0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
  assertEqual(sizeof(b), sizeof(b2));
  for (size_t i = 0; i < sizeof(b2); i++) {
    assertEqual(b[i], b2[i]);
  }
  assertEqual(w.getWriteError(), 0);
}

test(write_double_nan) {
  uint8_t b[9]{0};
  cbor::BytesPrint bp{b, sizeof(b)};
  cbor::Writer w{bp};
  w.writeDouble(NAN);
  assertEqual(w.getWriteSize(), size_t{9});

  cbor::BytesStream bs{b, sizeof(b)};
  cbor::Reader r{bs};
  assertTrue(r.isWellFormed());
  assertEqual(r.getReadSize(), size_t{9});

  uint8_t b2[] = { (7 << 5) + 27, 0x7f, 0xf8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
  assertEqual(sizeof(b), sizeof(b2));
  for (size_t i = 0; i < sizeof(b2); i++) {
    assertEqual(b[i], b2[i]);
  }
  assertEqual(w.getWriteError(), 0);
}

test(write_unsigned_zero) {
  uint8_t b[1]{0};
  cbor::BytesPrint bp{b, sizeof(b)};
  cbor::Writer w{bp};
  w.writeUnsignedInt(0);
  assertEqual(w.getWriteSize(), size_t{1});

  cbor::BytesStream bs{b, sizeof(b)};
  cbor::Reader r{bs};
  assertTrue(r.isWellFormed());
  assertEqual(r.getReadSize(), size_t{1});

  uint8_t b2[] = { (0 << 5) + 0 };
  assertEqual(sizeof(b), sizeof(b2));
  for (size_t i = 0; i < sizeof(b2); i++) {
    assertEqual(b[i], b2[i]);
  }
  assertEqual(w.getWriteError(), 0);
}

test(write_unsigned_23) {
  uint8_t b[1]{0};
  cbor::BytesPrint bp{b, sizeof(b)};
  cbor::Writer w{bp};
  w.writeUnsignedInt(23);
  assertEqual(w.getWriteSize(), size_t{1});

  cbor::BytesStream bs{b, sizeof(b)};
  cbor::Reader r{bs};
  assertTrue(r.isWellFormed());
  assertEqual(r.getReadSize(), size_t{1});

  uint8_t b2[] = { (0 << 5) + 23 };
  assertEqual(sizeof(b), sizeof(b2));
  for (size_t i = 0; i < sizeof(b2); i++) {
    assertEqual(b[i], b2[i]);
  }
  assertEqual(w.getWriteError(), 0);
}

test(write_unsigned_1_byte) {
  uint8_t b[2]{0};
  cbor::BytesPrint bp{b, sizeof(b)};
  cbor::Writer w{bp};
  w.writeUnsignedInt(24);
  assertEqual(w.getWriteSize(), size_t{2});

  cbor::BytesStream bs{b, sizeof(b)};
  cbor::Reader r{bs};
  assertTrue(r.isWellFormed());
  assertEqual(r.getReadSize(), size_t{2});

  uint8_t b2[] = { (0 << 5) + 24, 24 };
  assertEqual(sizeof(b), sizeof(b2));
  for (size_t i = 0; i < sizeof(b2); i++) {
    assertEqual(b[i], b2[i]);
  }
  assertEqual(w.getWriteError(), 0);
}

test(write_unsigned_2_bytes) {
  uint8_t b[3]{0};
  cbor::BytesPrint bp{b, sizeof(b)};
  cbor::Writer w{bp};
  w.writeUnsignedInt(0x0102);
  assertEqual(w.getWriteSize(), size_t{3});

  cbor::BytesStream bs{b, sizeof(b)};
  cbor::Reader r{bs};
  assertTrue(r.isWellFormed());
  assertEqual(r.getReadSize(), size_t{3});

  uint8_t b2[] = { (0 << 5) + 25, 0x01, 0x02 };
  assertEqual(sizeof(b), sizeof(b2));
  for (size_t i = 0; i < sizeof(b2); i++) {
    assertEqual(b[i], b2[i]);
  }
  assertEqual(w.getWriteError(), 0);
}

test(write_unsigned_4_bytes) {
  uint8_t b[5]{0};
  cbor::BytesPrint bp{b, sizeof(b)};
  cbor::Writer w{bp};
  w.writeUnsignedInt(0x01020304UL);
  assertEqual(w.getWriteSize(), size_t{5});

  cbor::BytesStream bs{b, sizeof(b)};
  cbor::Reader r{bs};
  assertTrue(r.isWellFormed());
  assertEqual(r.getReadSize(), size_t{5});

  uint8_t b2[] = { (0 << 5) + 26, 0x01, 0x02, 0x03, 0x04 };
  assertEqual(sizeof(b), sizeof(b2));
  for (size_t i = 0; i < sizeof(b2); i++) {
    assertEqual(b[i], b2[i]);
  }
  assertEqual(w.getWriteError(), 0);
}

test(write_unsigned_8_bytes) {
  uint8_t b[9]{0};
  cbor::BytesPrint bp{b, sizeof(b)};
  cbor::Writer w{bp};
  w.writeUnsignedInt(0x0102030405060708ULL);
  assertEqual(w.getWriteSize(), size_t{9});

  cbor::BytesStream bs{b, sizeof(b)};
  cbor::Reader r{bs};
  assertTrue(r.isWellFormed());
  assertEqual(r.getReadSize(), size_t{9});

  uint8_t b2[] = { (0 << 5) + 27, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08 };
  assertEqual(sizeof(b), sizeof(b2));
  for (size_t i = 0; i < sizeof(b2); i++) {
    assertEqual(b[i], b2[i]);
  }
  assertEqual(w.getWriteError(), 0);
}

test(write_negative_24) {
  uint8_t b[1]{0};
  cbor::BytesPrint bp{b, sizeof(b)};
  cbor::Writer w{bp};
  w.writeInt(-24);
  assertEqual(w.getWriteSize(), size_t{1});

  cbor::BytesStream bs{b, sizeof(b)};
  cbor::Reader r{bs};
  assertTrue(r.isWellFormed());
  assertEqual(r.getReadSize(), size_t{1});

  uint8_t b2[] = { (1 << 5) + 23 };
  assertEqual(sizeof(b), sizeof(b2));
  for (size_t i = 0; i < sizeof(b2); i++) {
    assertEqual(b[i], b2[i]);
  }
  assertEqual(w.getWriteError(), 0);
}

test(write_negative_1_byte) {
  uint8_t b[2]{0};
  cbor::BytesPrint bp{b, sizeof(b)};
  cbor::Writer w{bp};
  w.writeInt(-25);
  assertEqual(w.getWriteSize(), size_t{2});

  cbor::BytesStream bs{b, sizeof(b)};
  cbor::Reader r{bs};
  assertTrue(r.isWellFormed());
  assertEqual(r.getReadSize(), size_t{2});

  uint8_t b2[] = { (1 << 5) + 24, 24 };
  assertEqual(sizeof(b), sizeof(b2));
  for (size_t i = 0; i < sizeof(b2); i++) {
    assertEqual(b[i], b2[i]);
  }
  assertEqual(w.getWriteError(), 0);
}

test(write_negative_2_bytes) {
  uint8_t b[3]{0};
  cbor::BytesPrint bp{b, sizeof(b)};
  cbor::Writer w{bp};
  w.writeInt(-0x0103);
  assertEqual(w.getWriteSize(), size_t{3});

  cbor::BytesStream bs{b, sizeof(b)};
  cbor::Reader r{bs};
  assertTrue(r.isWellFormed());
  assertEqual(r.getReadSize(), size_t{3});

  uint8_t b2[] = { (1 << 5) + 25, 0x01, 0x02 };
  assertEqual(sizeof(b), sizeof(b2));
  for (size_t i = 0; i < sizeof(b2); i++) {
    assertEqual(b[i], b2[i]);
  }
  assertEqual(w.getWriteError(), 0);
}

test(write_negative_4_bytes) {
  uint8_t b[5]{0};
  cbor::BytesPrint bp{b, sizeof(b)};
  cbor::Writer w{bp};
  w.writeInt(-0x01020305L);
  assertEqual(w.getWriteSize(), size_t{5});

  cbor::BytesStream bs{b, sizeof(b)};
  cbor::Reader r{bs};
  assertTrue(r.isWellFormed());
  assertEqual(r.getReadSize(), size_t{5});

  uint8_t b2[] = { (1 << 5) + 26, 0x01, 0x02, 0x03, 0x04 };
  assertEqual(sizeof(b), sizeof(b2));
  for (size_t i = 0; i < sizeof(b2); i++) {
    assertEqual(b[i], b2[i]);
  }
  assertEqual(w.getWriteError(), 0);
}

test(write_negative_8_bytes) {
  uint8_t b[9]{0};
  cbor::BytesPrint bp{b, sizeof(b)};
  cbor::Writer w{bp};
  w.writeInt(-0x0102030405060709LL);
  assertEqual(w.getWriteSize(), size_t{9});

  cbor::BytesStream bs{b, sizeof(b)};
  cbor::Reader r{bs};
  assertTrue(r.isWellFormed());
  assertEqual(r.getReadSize(), size_t{9});

  uint8_t b2[] = { (1 << 5) + 27, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08 };
  assertEqual(sizeof(b), sizeof(b2));
  for (size_t i = 0; i < sizeof(b2); i++) {
    assertEqual(b[i], b2[i]);
  }
  assertEqual(w.getWriteError(), 0);
}

test(write_null) {
  uint8_t b[1]{0};
  cbor::BytesPrint bp{b, sizeof(b)};
  cbor::Writer w{bp};
  w.writeNull();
  assertEqual(w.getWriteSize(), size_t{1});

  cbor::BytesStream bs{b, sizeof(b)};
  cbor::Reader r{bs};
  assertTrue(r.isWellFormed());
  assertEqual(r.getReadSize(), size_t{1});

  uint8_t b2[] = { (7 << 5) + 22 };
  assertEqual(sizeof(b), sizeof(b2));
  for (size_t i = 0; i < sizeof(b2); i++) {
    assertEqual(b[i], b2[i]);
  }
  assertEqual(w.getWriteError(), 0);
}

test(write_undefined) {
  uint8_t b[1]{0};
  cbor::BytesPrint bp{b, sizeof(b)};
  cbor::Writer w{bp};
  w.writeUndefined();
  assertEqual(w.getWriteSize(), size_t{1});

  cbor::BytesStream bs{b, sizeof(b)};
  cbor::Reader r{bs};
  assertTrue(r.isWellFormed());
  assertEqual(r.getReadSize(), size_t{1});

  uint8_t b2[] = { (7 << 5) + 23 };
  assertEqual(sizeof(b), sizeof(b2));
  for (size_t i = 0; i < sizeof(b2); i++) {
    assertEqual(b[i], b2[i]);
  }
  assertEqual(w.getWriteError(), 0);
}

test(write_simple_value_19) {
  uint8_t b[1]{0};
  cbor::BytesPrint bp{b, sizeof(b)};
  cbor::Writer w{bp};
  w.writeSimpleValue(19);
  assertEqual(w.getWriteSize(), size_t{1});

  cbor::BytesStream bs{b, sizeof(b)};
  cbor::Reader r{bs};
  assertTrue(r.isWellFormed());
  assertEqual(r.getReadSize(), size_t{1});

  uint8_t b2[] = { (7 << 5) + 19 };
  assertEqual(sizeof(b), sizeof(b2));
  for (size_t i = 0; i < sizeof(b2); i++) {
    assertEqual(b[i], b2[i]);
  }
  assertEqual(w.getWriteError(), 0);
}

test(write_simple_value_1_byte) {
  uint8_t b[2]{0};
  cbor::BytesPrint bp{b, sizeof(b)};
  cbor::Writer w{bp};
  w.writeSimpleValue(32);
  assertEqual(w.getWriteSize(), size_t{2});

  cbor::BytesStream bs{b, sizeof(b)};
  cbor::Reader r{bs};
  assertTrue(r.isWellFormed());
  assertEqual(r.getReadSize(), size_t{2});

  uint8_t b2[] = { (7 << 5) + 24, 32 };
  assertEqual(sizeof(b), sizeof(b2));
  for (size_t i = 0; i < sizeof(b2); i++) {
    assertEqual(b[i], b2[i]);
  }
  assertEqual(w.getWriteError(), 0);
}

test(write_tag_zero) {
  uint8_t b[2]{0};
  cbor::BytesPrint bp{b, sizeof(b)};
  cbor::Writer w{bp};
  w.writeTag(0);
  w.writeUnsignedInt(1);  // Ensure there's something after the tag for well-formedness
  assertEqual(w.getWriteSize(), size_t{2});

  cbor::BytesStream bs{b, sizeof(b)};
  cbor::Reader r{bs};
  assertTrue(r.isWellFormed());
  assertEqual(r.getReadSize(), size_t{2});

  uint8_t b2[] = { (6 << 5) + 0, (0 << 5) + 1 };
  assertEqual(sizeof(b), sizeof(b2));
  for (size_t i = 0; i < sizeof(b2); i++) {
    assertEqual(b[i], b2[i]);
  }
  assertEqual(w.getWriteError(), 0);
}

test(write_tag_23) {
  uint8_t b[2]{0};
  cbor::BytesPrint bp{b, sizeof(b)};
  cbor::Writer w{bp};
  w.writeTag(23);
  w.writeUnsignedInt(1);  // Ensure there's something after the tag for well-formedness
  assertEqual(w.getWriteSize(), size_t{2});

  cbor::BytesStream bs{b, sizeof(b)};
  cbor::Reader r{bs};
  assertTrue(r.isWellFormed());
  assertEqual(r.getReadSize(), size_t{2});

  uint8_t b2[] = { (6 << 5) + 23, (0 << 5) + 1 };
  assertEqual(sizeof(b), sizeof(b2));
  for (size_t i = 0; i < sizeof(b2); i++) {
    assertEqual(b[i], b2[i]);
  }
  assertEqual(w.getWriteError(), 0);
}

test(write_tag_1_byte) {
  uint8_t b[3]{0};
  cbor::BytesPrint bp{b, sizeof(b)};
  cbor::Writer w{bp};
  w.writeTag(24);
  w.writeUnsignedInt(1);  // Ensure there's something after the tag for well-formedness
  assertEqual(w.getWriteSize(), size_t{3});

  cbor::BytesStream bs{b, sizeof(b)};
  cbor::Reader r{bs};
  assertTrue(r.isWellFormed());
  assertEqual(r.getReadSize(), size_t{3});

  uint8_t b2[] = { (6 << 5) + 24, 24, (0 << 5) + 1 };
  assertEqual(sizeof(b), sizeof(b2));
  for (size_t i = 0; i < sizeof(b2); i++) {
    assertEqual(b[i], b2[i]);
  }
  assertEqual(w.getWriteError(), 0);
}

test(write_tag_2_bytes) {
  uint8_t b[4]{0};
  cbor::BytesPrint bp{b, sizeof(b)};
  cbor::Writer w{bp};
  w.writeTag(0x0102);
  w.writeUnsignedInt(1);  // Ensure there's something after the tag for well-formedness
  assertEqual(w.getWriteSize(), size_t{4});

  cbor::BytesStream bs{b, sizeof(b)};
  cbor::Reader r{bs};
  assertTrue(r.isWellFormed());
  assertEqual(r.getReadSize(), size_t{4});

  uint8_t b2[] = { (6 << 5) + 25, 0x01, 0x02, (0 << 5) + 1 };
  assertEqual(sizeof(b), sizeof(b2));
  for (size_t i = 0; i < sizeof(b2); i++) {
    assertEqual(b[i], b2[i]);
  }
  assertEqual(w.getWriteError(), 0);
}

test(write_tag_4_bytes) {
  uint8_t b[6]{0};
  cbor::BytesPrint bp{b, sizeof(b)};
  cbor::Writer w{bp};
  w.writeTag(0x01020304UL);
  w.writeUnsignedInt(1);  // Ensure there's something after the tag for well-formedness
  assertEqual(w.getWriteSize(), size_t{6});

  cbor::BytesStream bs{b, sizeof(b)};
  cbor::Reader r{bs};
  assertTrue(r.isWellFormed());
  assertEqual(r.getReadSize(), size_t{6});

  uint8_t b2[] = { (6 << 5) + 26, 0x01, 0x02, 0x03, 0x04, (0 << 5) + 1 };
  assertEqual(sizeof(b), sizeof(b2));
  for (size_t i = 0; i < sizeof(b2); i++) {
    assertEqual(b[i], b2[i]);
  }
  assertEqual(w.getWriteError(), 0);
}

test(write_tag_8_bytes) {
  uint8_t b[10]{0};
  cbor::BytesPrint bp{b, sizeof(b)};
  cbor::Writer w{bp};
  w.writeTag(0x0102030405060708ULL);
  w.writeUnsignedInt(1);  // Ensure there's something after the tag for well-formedness
  assertEqual(w.getWriteSize(), size_t{10});

  cbor::BytesStream bs{b, sizeof(b)};
  cbor::Reader r{bs};
  assertTrue(r.isWellFormed());
  assertEqual(r.getReadSize(), size_t{10});

  uint8_t b2[] = { (6 << 5) + 27, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, (0 << 5) + 1 };
  assertEqual(sizeof(b), sizeof(b2));
  for (size_t i = 0; i < sizeof(b2); i++) {
    assertEqual(b[i], b2[i]);
  }
  assertEqual(w.getWriteError(), 0);
}

test(write_bytes_empty) {
  uint8_t b[1]{0};
  cbor::BytesPrint bp{b, sizeof(b)};
  cbor::Writer w{bp};
  w.beginBytes(0);
  assertEqual(w.getWriteSize(), size_t{1});

  cbor::BytesStream bs{b, sizeof(b)};
  cbor::Reader r{bs};
  assertTrue(r.isWellFormed());
  assertEqual(r.getReadSize(), size_t{1});

  uint8_t b2[] = { (2 << 5) + 0 };
  assertEqual(sizeof(b), sizeof(b2));
  for (size_t i = 0; i < sizeof(b2); i++) {
    assertEqual(b[i], b2[i]);
  }
  assertEqual(w.getWriteError(), 0);
}

test(write_bytes_indefinite_empty) {
  uint8_t b[2]{0};
  cbor::BytesPrint bp{b, sizeof(b)};
  cbor::Writer w{bp};
  w.beginIndefiniteBytes();
  w.endIndefinite();
  assertEqual(w.getWriteSize(), size_t{2});

  cbor::BytesStream bs{b, sizeof(b)};
  cbor::Reader r{bs};
  assertTrue(r.isWellFormed());
  assertEqual(r.getReadSize(), size_t{2});

  uint8_t b2[] = { (2 << 5) + 31, (7 << 5) + 31 };
  assertEqual(sizeof(b), sizeof(b2));
  for (size_t i = 0; i < sizeof(b2); i++) {
    assertEqual(b[i], b2[i]);
  }
  assertEqual(w.getWriteError(), 0);
}

test(write_bytes_2) {
  uint8_t b[3]{0};
  cbor::BytesPrint bp{b, sizeof(b)};
  cbor::Writer w{bp};
  w.beginBytes(2);
  uint8_t data[] = { 0x01, 0x02 };
  w.writeBytes(data, sizeof(data));
  assertEqual(w.getWriteSize(), size_t{3});

  cbor::BytesStream bs{b, sizeof(b)};
  cbor::Reader r{bs};
  assertTrue(r.isWellFormed());
  assertEqual(r.getReadSize(), size_t{3});

  uint8_t b2[] = { (2 << 5) + 2, 0x01, 0x02 };
  assertEqual(sizeof(b), sizeof(b2));
  for (size_t i = 0; i < sizeof(b2); i++) {
    assertEqual(b[i], b2[i]);
  }
  assertEqual(w.getWriteError(), 0);
}

test(write_bytes_indefinite) {
  uint8_t b[4]{0};
  cbor::BytesPrint bp{b, sizeof(b)};
  cbor::Writer w{bp};
  w.beginIndefiniteBytes();
  uint8_t data[] = { 0x01 };
  w.beginBytes(sizeof(data));
  w.writeBytes(data, sizeof(data));
  w.endIndefinite();
  assertEqual(w.getWriteSize(), size_t{4});

  cbor::BytesStream bs{b, sizeof(b)};
  cbor::Reader r{bs};
  assertTrue(r.isWellFormed());
  assertEqual(r.getReadSize(), size_t{4});

  uint8_t b2[] = { (2 << 5) + 31, (2 << 5) + 1, 0x01, (7 << 5) + 31 };
  assertEqual(sizeof(b), sizeof(b2));
  for (size_t i = 0; i < sizeof(b2); i++) {
    assertEqual(b[i], b2[i]);
  }
  assertEqual(w.getWriteError(), 0);
}

test(write_text_empty) {
  uint8_t b[1]{0};
  cbor::BytesPrint bp{b, sizeof(b)};
  cbor::Writer w{bp};
  w.beginText(0);
  assertEqual(w.getWriteSize(), size_t{1});

  cbor::BytesStream bs{b, sizeof(b)};
  cbor::Reader r{bs};
  assertTrue(r.isWellFormed());
  assertEqual(r.getReadSize(), size_t{1});

  uint8_t b2[] = { (3 << 5) + 0 };
  assertEqual(sizeof(b), sizeof(b2));
  for (size_t i = 0; i < sizeof(b2); i++) {
    assertEqual(b[i], b2[i]);
  }
  assertEqual(w.getWriteError(), 0);
}

test(write_text_indefinite_empty) {
  uint8_t b[2]{0};
  cbor::BytesPrint bp{b, sizeof(b)};
  cbor::Writer w{bp};
  w.beginIndefiniteText();
  w.endIndefinite();
  assertEqual(w.getWriteSize(), size_t{2});

  cbor::BytesStream bs{b, sizeof(b)};
  cbor::Reader r{bs};
  assertTrue(r.isWellFormed());
  assertEqual(r.getReadSize(), size_t{2});

  uint8_t b2[] = { (3 << 5) + 31, (7 << 5) + 31 };
  assertEqual(sizeof(b), sizeof(b2));
  for (size_t i = 0; i < sizeof(b2); i++) {
    assertEqual(b[i], b2[i]);
  }
  assertEqual(w.getWriteError(), 0);
}

test(write_text_2) {
  uint8_t b[3]{0};
  cbor::BytesPrint bp{b, sizeof(b)};
  cbor::Writer w{bp};
  w.beginText(2);
  uint8_t data[] = { 0x01, 0x02 };
  w.writeBytes(data, sizeof(data));
  assertEqual(w.getWriteSize(), size_t{3});

  cbor::BytesStream bs{b, sizeof(b)};
  cbor::Reader r{bs};
  assertTrue(r.isWellFormed());
  assertEqual(r.getReadSize(), size_t{3});

  uint8_t b2[] = { (3 << 5) + 2, 0x01, 0x02 };
  assertEqual(sizeof(b), sizeof(b2));
  for (size_t i = 0; i < sizeof(b2); i++) {
    assertEqual(b[i], b2[i]);
  }
  assertEqual(w.getWriteError(), 0);
}

test(write_text_indefinite) {
  uint8_t b[4]{0};
  cbor::BytesPrint bp{b, sizeof(b)};
  cbor::Writer w{bp};
  w.beginIndefiniteText();
  uint8_t data[] = { 0x01 };
  w.beginText(sizeof(data));
  w.writeBytes(data, sizeof(data));
  w.endIndefinite();
  assertEqual(w.getWriteSize(), size_t{4});

  cbor::BytesStream bs{b, sizeof(b)};
  cbor::Reader r{bs};
  assertTrue(r.isWellFormed());
  assertEqual(r.getReadSize(), size_t{4});

  uint8_t b2[] = { (3 << 5) + 31, (3 << 5) + 1, 0x01, (7 << 5) + 31 };
  assertEqual(sizeof(b), sizeof(b2));
  for (size_t i = 0; i < sizeof(b2); i++) {
    assertEqual(b[i], b2[i]);
  }
  assertEqual(w.getWriteError(), 0);
}

test(write_array_empty) {
  uint8_t b[1]{0};
  cbor::BytesPrint bp{b, sizeof(b)};
  cbor::Writer w{bp};
  w.beginArray(0);
  assertEqual(w.getWriteSize(), size_t{1});

  cbor::BytesStream bs{b, sizeof(b)};
  cbor::Reader r{bs};
  assertTrue(r.isWellFormed());
  assertEqual(r.getReadSize(), size_t{1});

  uint8_t b2[] = { (4 << 5) + 0 };
  assertEqual(sizeof(b), sizeof(b2));
  for (size_t i = 0; i < sizeof(b2); i++) {
    assertEqual(b[i], b2[i]);
  }
  assertEqual(w.getWriteError(), 0);
}

test(write_array_indefinite_empty) {
  uint8_t b[2]{0};
  cbor::BytesPrint bp{b, sizeof(b)};
  cbor::Writer w{bp};
  w.beginIndefiniteArray();
  w.endIndefinite();
  assertEqual(w.getWriteSize(), size_t{2});

  cbor::BytesStream bs{b, sizeof(b)};
  cbor::Reader r{bs};
  assertTrue(r.isWellFormed());
  assertEqual(r.getReadSize(), size_t{2});

  uint8_t b2[] = { (4 << 5) + 31, (7 << 5) + 31 };
  assertEqual(sizeof(b), sizeof(b2));
  for (size_t i = 0; i < sizeof(b2); i++) {
    assertEqual(b[i], b2[i]);
  }
  assertEqual(w.getWriteError(), 0);
}

test(write_array_2) {
  uint8_t b[3]{0};
  cbor::BytesPrint bp{b, sizeof(b)};
  cbor::Writer w{bp};
  w.beginArray(2);
  uint8_t data[] = { 0x01, 0x02 };
  for (size_t i = 0; i < sizeof(data); i++) {
    w.writeUnsignedInt(data[i]);
  }
  assertEqual(w.getWriteSize(), size_t{3});

  cbor::BytesStream bs{b, sizeof(b)};
  cbor::Reader r{bs};
  assertTrue(r.isWellFormed());
  assertEqual(r.getReadSize(), size_t{3});

  uint8_t b2[] = { (4 << 5) + 2, 0x01, 0x02 };
  assertEqual(sizeof(b), sizeof(b2));
  for (size_t i = 0; i < sizeof(b2); i++) {
    assertEqual(b[i], b2[i]);
  }
  assertEqual(w.getWriteError(), 0);
}

test(write_array_indefinite) {
  uint8_t b[3]{0};
  cbor::BytesPrint bp{b, sizeof(b)};
  cbor::Writer w{bp};
  w.beginIndefiniteArray();
  uint8_t data[] = { 0x01 };
  for (size_t i = 0; i < sizeof(data); i++) {
    w.writeUnsignedInt(data[i]);
  }
  w.endIndefinite();
  assertEqual(w.getWriteSize(), size_t{3});

  cbor::BytesStream bs{b, sizeof(b)};
  cbor::Reader r{bs};
  assertTrue(r.isWellFormed());
  assertEqual(r.getReadSize(), size_t{3});

  uint8_t b2[] = { (4 << 5) + 31, 0x01, (7 << 5) + 31 };
  assertEqual(sizeof(b), sizeof(b2));
  for (size_t i = 0; i < sizeof(b2); i++) {
    assertEqual(b[i], b2[i]);
  }
  assertEqual(w.getWriteError(), 0);
}

test(write_map_empty) {
  uint8_t b[1]{0};
  cbor::BytesPrint bp{b, sizeof(b)};
  cbor::Writer w{bp};
  w.beginMap(0);
  assertEqual(w.getWriteSize(), size_t{1});

  cbor::BytesStream bs{b, sizeof(b)};
  cbor::Reader r{bs};
  assertTrue(r.isWellFormed());
  assertEqual(r.getReadSize(), size_t{1});

  uint8_t b2[] = { (5 << 5) + 0 };
  assertEqual(sizeof(b), sizeof(b2));
  for (size_t i = 0; i < sizeof(b2); i++) {
    assertEqual(b[i], b2[i]);
  }
  assertEqual(w.getWriteError(), 0);
}

test(write_map_indefinite_empty) {
  uint8_t b[2]{0};
  cbor::BytesPrint bp{b, sizeof(b)};
  cbor::Writer w{bp};
  w.beginIndefiniteMap();
  w.endIndefinite();
  assertEqual(w.getWriteSize(), size_t{2});

  cbor::BytesStream bs{b, sizeof(b)};
  cbor::Reader r{bs};
  assertTrue(r.isWellFormed());
  assertEqual(r.getReadSize(), size_t{2});

  uint8_t b2[] = { (5 << 5) + 31, (7 << 5) + 31 };
  assertEqual(sizeof(b), sizeof(b2));
  for (size_t i = 0; i < sizeof(b2); i++) {
    assertEqual(b[i], b2[i]);
  }
  assertEqual(w.getWriteError(), 0);
}

test(write_map_2) {
  uint8_t b[5]{0};
  cbor::BytesPrint bp{b, sizeof(b)};
  cbor::Writer w{bp};
  w.beginMap(2);
  uint8_t data[] = { 0x01, 0x02, 0x03, 0x04 };
  for (size_t i = 0; i < sizeof(data); i++) {
    w.writeUnsignedInt(data[i]);
  }
  assertEqual(w.getWriteSize(), size_t{5});

  cbor::BytesStream bs{b, sizeof(b)};
  cbor::Reader r{bs};
  assertTrue(r.isWellFormed());
  assertEqual(r.getReadSize(), size_t{5});

  uint8_t b2[] = { (5 << 5) + 2, 0x01, 0x02, 0x03, 0x04 };
  assertEqual(sizeof(b), sizeof(b2));
  for (size_t i = 0; i < sizeof(b2); i++) {
    assertEqual(b[i], b2[i]);
  }
  assertEqual(w.getWriteError(), 0);
}

test(write_map_indefinite) {
  uint8_t b[4]{0};
  cbor::BytesPrint bp{b, sizeof(b)};
  cbor::Writer w{bp};
  w.beginIndefiniteMap();
  uint8_t data[] = { 0x01, 0x02 };
  for (size_t i = 0; i < sizeof(data); i++) {
    w.writeUnsignedInt(data[i]);
  }
  w.endIndefinite();
  assertEqual(w.getWriteSize(), size_t{4});

  cbor::BytesStream bs{b, sizeof(b)};
  cbor::Reader r{bs};
  assertTrue(r.isWellFormed());
  assertEqual(r.getReadSize(), size_t{4});

  uint8_t b2[] = { (5 << 5) + 31, 0x01, 0x02, (7 << 5) + 31 };
  assertEqual(sizeof(b), sizeof(b2));
  for (size_t i = 0; i < sizeof(b2); i++) {
    assertEqual(b[i], b2[i]);
  }
  assertEqual(w.getWriteError(), 0);
}
