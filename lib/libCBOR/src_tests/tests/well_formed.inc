// well_formed.inc is part of libCBOR.
// (c) 2017 Shawn Silverman

// ***************************************************************************
//  Well-formedness tests
// ***************************************************************************

test(well_formed_empty) {
  uint8_t b[0];
  cbor::BytesStream bs{b, sizeof(b)};
  cbor::Reader r{bs};
  assertFalse(r.isWellFormed());
  assertEqual(r.getReadSize(), size_t{0});
}

test(well_formed_incomplete_1_byte) {
  uint8_t b[] = { 0 };
  for (int i = 0; i < 8; i++) {
    b[0] = (i << 5) + 24;
    cbor::BytesStream bs{b, sizeof(b)};
    cbor::Reader r{bs};
    assertFalse(r.isWellFormed());
    assertEqual(r.getReadSize(), size_t{1});
  }
}

test(well_formed_incomplete_2_bytes) {
  uint8_t b[] = { 0, 0x01 };
  for (int i = 0; i < 8; i++) {
    b[0] = (i << 5) + 25;
    cbor::BytesStream bs{b, sizeof(b)};
    cbor::Reader r{bs};
    assertFalse(r.isWellFormed());
    assertEqual(r.getReadSize(), size_t{1});
  }
}

test(well_formed_incomplete_4_bytes) {
  uint8_t b[] = { 0, 0x01, 0x02, 0x03 };
  for (int i = 0; i < 8; i++) {
    b[0] = (i << 5) + 26;
    cbor::BytesStream bs{b, sizeof(b)};
    cbor::Reader r{bs};
    assertFalse(r.isWellFormed());
    assertEqual(r.getReadSize(), size_t{1});
  }
}

test(well_formed_incomplete_8_bytes) {
  uint8_t b[] = { 0, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07 };
  for (int i = 0; i < 8; i++) {
    b[0] = (i << 5) + 27;
    cbor::BytesStream bs{b, sizeof(b)};
    cbor::Reader r{bs};
    assertFalse(r.isWellFormed());
    assertEqual(r.getReadSize(), size_t{1});
  }
}

test(well_formed_bad_additional_info) {
  uint8_t b[1];
  for (int i = 0; i < 8; i++) {
    for (int j = 28; j <= 30; j++) {
      b[0] = (i << 5) + j;
      cbor::BytesStream bs{b, sizeof(b)};
      cbor::Reader r{bs};
      assertFalse(r.isWellFormed());
      assertEqual(r.getReadSize(), size_t{1});
    }
  }
}

test(well_formed_missing_tagged_item) {
  uint8_t b[] = { (6 << 5) + 25, 0xd9, 0xf7 };
  cbor::BytesStream bs{b, sizeof(b)};
  cbor::Reader r{bs};
  assertFalse(r.isWellFormed());
  assertEqual(r.getReadSize(), size_t{3});
}

test(well_formed_bytes_too_short) {
  uint8_t b[] = { (2 << 5) + 3, 0x01, 0x02 };
  cbor::BytesStream bs{b, sizeof(b)};
  cbor::Reader r{bs};
  assertFalse(r.isWellFormed());
  assertEqual(r.getReadSize(), size_t{3});
}

test(well_formed_bytes_indefinite_too_short) {
  uint8_t b[] = { (2 << 5) + 31, (2 << 5) + 2, 0x01, 0x02 };
  cbor::BytesStream bs{b, sizeof(b)};
  cbor::Reader r{bs};
  assertFalse(r.isWellFormed());
  assertEqual(r.getReadSize(), size_t{4});
}

test(well_formed_bytes_indefinite_bad_type) {
  uint8_t b[] = { (2 << 5) + 31, (3 << 5) + 2, 0x01, 0x02, (7 << 5) + 31 };
  cbor::BytesStream bs{b, sizeof(b)};
  cbor::Reader r{bs};
  assertFalse(r.isWellFormed());
  assertEqual(r.getReadSize(), size_t{2});
}

test(well_formed_text_too_short) {
  uint8_t b[] = { (3 << 5) + 3, 0x01, 0x02 };
  cbor::BytesStream bs{b, sizeof(b)};
  cbor::Reader r{bs};
  assertFalse(r.isWellFormed());
  assertEqual(r.getReadSize(), size_t{3});
}

test(well_formed_text_indefinite_too_short) {
  uint8_t b[] = { (3 << 5) + 31, (3 << 5) + 2, 0x01, 0x02 };
  cbor::BytesStream bs{b, sizeof(b)};
  cbor::Reader r{bs};
  assertFalse(r.isWellFormed());
  assertEqual(r.getReadSize(), size_t{4});
}

test(well_formed_text_indefinite_bad_type) {
  uint8_t b[] = { (3 << 5) + 31, (2 << 5) + 2, 0x01, 0x02, (7 << 5) + 31 };
  cbor::BytesStream bs{b, sizeof(b)};
  cbor::Reader r{bs};
  assertFalse(r.isWellFormed());
  assertEqual(r.getReadSize(), size_t{2});
}

test(well_formed_array_too_short) {
  uint8_t b[] = { (4 << 5) + 3, 0x01, 0x02 };
  cbor::BytesStream bs{b, sizeof(b)};
  cbor::Reader r{bs};
  assertFalse(r.isWellFormed());
  assertEqual(r.getReadSize(), size_t{3});
}

test(well_formed_map_too_short_complete_pair) {
  uint8_t b[] = { (5 << 5) + 3, 0x01, 0x02, 0x03, 0x04 };
  cbor::BytesStream bs{b, sizeof(b)};
  cbor::Reader r{bs};
  assertFalse(r.isWellFormed());
  assertEqual(r.getReadSize(), size_t{5});
}

test(well_formed_map_too_short_incomplete_pair) {
  uint8_t b[] = { (5 << 5) + 3, 0x01, 0x02, 0x03, 0x04, 0x05 };
  cbor::BytesStream bs{b, sizeof(b)};
  cbor::Reader r{bs};
  assertFalse(r.isWellFormed());
  assertEqual(r.getReadSize(), size_t{6});
}

test(well_formed_lone_break) {
  uint8_t b[] = { (7 << 5) + 31 };
  cbor::BytesStream bs{b, sizeof(b)};
  cbor::Reader r{bs};
  assertFalse(r.isWellFormed());
  assertEqual(r.getReadSize(), size_t{1});
}
