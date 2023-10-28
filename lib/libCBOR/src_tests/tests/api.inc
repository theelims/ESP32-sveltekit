// api.inc is part of libCBOR.
// (c) 2017 Shawn Silverman

// Tests for API calls that aren't tested elsewhere.

test(short_write_single_byte) {
  uint8_t b[1];
  cbor::BytesPrint bp{b, sizeof(b)};
  cbor::Writer w{bp};
  assertTrue(w.getWriteError() == 0);

  uint8_t b2[] = { (0 << 5) + 24, 24 };
  w.writeUnsignedInt(b2[1]);
  assertEqual(w.getWriteSize(), size_t{1});
  assertTrue(w.getWriteError() != 0);

  for (size_t i = 0; i < sizeof(b); i++) {
    assertEqual(b[i], b2[i]);
  }
}

test(short_write_2_bytes) {
  uint8_t b[2];
  cbor::BytesPrint bp{b, sizeof(b)};
  cbor::Writer w{bp};
  assertTrue(w.getWriteError() == 0);

  uint8_t b2[] = { (2 << 5) + 2, 0x01, 0x02 };
  w.beginBytes(2);
  w.writeByte(b2[1]);
  assertEqual(w.getWriteSize(), size_t{2});
  assertTrue(w.getWriteError() == 0);
  w.writeByte(b2[2]);
  assertEqual(w.getWriteSize(), size_t{2});
  assertTrue(w.getWriteError() != 0);

  for (size_t i = 0; i < sizeof(b); i++) {
    assertEqual(b[i], b2[i]);
  }
}

test(short_write_multi_bytes) {
  uint8_t b[2];
  cbor::BytesPrint bp{b, sizeof(b)};
  cbor::Writer w{bp};
  assertTrue(w.getWriteError() == 0);
  w.beginBytes(2);

  uint8_t b2[] = { (2 << 5) + 2, 0x01, 0x02 };
  w.writeBytes(&b2[1], 2);
  assertEqual(w.getWriteSize(), size_t{2});
  assertTrue(w.getWriteError() != 0);

  for (size_t i = 0; i < sizeof(b); i++) {
    assertEqual(b[i], b2[i]);
  }
}
