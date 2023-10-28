// syntax.inc is part of libCBOR.
// (c) 2017 Shawn Silverman

// ***************************************************************************
//  Syntax errors
// ***************************************************************************

test(disallow_bad_ai) {
  uint8_t b[1];
  for (int i = 0; i < 8; i++) {
    for (int j = 28; j <= 30; j++) {
      b[0] = (i << 5) + j;
      cbor::BytesStream bs{b, sizeof(b)};
      cbor::Reader r{bs};
      assertFalse(r.isWellFormed());
      assertEqual(r.getReadSize(), size_t{1});
      bs.reset();
      assertEqual(static_cast<int>(r.readDataType()), static_cast<int>(cbor::DataType::kSyntaxError));
      assertEqual(static_cast<int>(r.getSyntaxError()), static_cast<int>(cbor::SyntaxError::kUnknownAdditionalInfo));
      assertEqual(r.getReadSize(), size_t{2});
    }
  }
}

test(disallow_indefinite_unsigned) {
  uint8_t b[] = { (0 << 5) + 31 };
  cbor::BytesStream bs{b, sizeof(b)};
  cbor::Reader r{bs};
  assertFalse(r.isWellFormed());
  assertEqual(r.getReadSize(), size_t{1});
  bs.reset();
  assertEqual(static_cast<int>(r.readDataType()), static_cast<int>(cbor::DataType::kSyntaxError));
  assertEqual(static_cast<int>(r.getSyntaxError()), static_cast<int>(cbor::SyntaxError::kNotAnIndefiniteType));
  assertEqual(r.getReadSize(), size_t{2});
}

test(disallow_indefinite_negative) {
  uint8_t b[] = { (1 << 5) + 31 };
  cbor::BytesStream bs{b, sizeof(b)};
  cbor::Reader r{bs};
  assertFalse(r.isWellFormed());
  assertEqual(r.getReadSize(), size_t{1});
  bs.reset();
  assertEqual(static_cast<int>(r.readDataType()), static_cast<int>(cbor::DataType::kSyntaxError));
  assertEqual(static_cast<int>(r.getSyntaxError()), static_cast<int>(cbor::SyntaxError::kNotAnIndefiniteType));
  assertEqual(r.getReadSize(), size_t{2});
}

test(disallow_indefinite_tag) {
  uint8_t b[] = { (6 << 5) + 31 };
  cbor::BytesStream bs{b, sizeof(b)};
  cbor::Reader r{bs};
  assertFalse(r.isWellFormed());
  assertEqual(r.getReadSize(), size_t{1});
  bs.reset();
  assertEqual(static_cast<int>(r.readDataType()), static_cast<int>(cbor::DataType::kSyntaxError));
  assertEqual(static_cast<int>(r.getSyntaxError()), static_cast<int>(cbor::SyntaxError::kNotAnIndefiniteType));
  assertEqual(r.getReadSize(), size_t{2});
}

// Technically, this test should disallow simple values < 32. They are
// technically well-formed but invalid.
test(disallow_but_allow_some_simple_values) {
  uint8_t b[] = { (7 << 5) + 24, 0 };
  for (int i = 0; i < 32; i++) {
    b[1] = i;
    cbor::BytesStream bs{b, sizeof(b)};
    cbor::Reader r{bs};
    assertTrue(r.isWellFormed());
    assertEqual(r.getReadSize(), size_t{2});
    bs.reset();
    // assertEqual(static_cast<int>(r.readDataType()), static_cast<int>(cbor::DataType::kSyntaxError));
    // assertEqual(static_cast<int>(r.getSyntaxError()), static_cast<int>(cbor::SyntaxError::kBadSimpleValue));
    assertEqual(static_cast<int>(r.readDataType()), static_cast<int>(cbor::DataType::kSimpleValue));
    assertEqual(r.getSimpleValue(), i);
    assertEqual(r.getReadSize(), size_t{4});
  }
}
