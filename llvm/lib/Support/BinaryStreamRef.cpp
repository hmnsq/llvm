//===- BinaryStreamRef.cpp - ----------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#شامل "llvm/Support/BinaryStreamRef.h"
#شامل "llvm/Support/BinaryByteStream.h"

استعمال نامخلا llvm;

نامخلا {

جماعت ArrayRefImpl : عوامی ثنائی_دھارا {
عوامی:
  ArrayRefImpl(ArrayRef<uint8_t> ڈیٹا_2, endianness Endian) : BBS(ڈیٹا_2, Endian) {}

  llvm::endianness getEndian() ثابت override { واپس BBS.getEndian(); }
  Error read_بائٹیں(uint64_t انحراف, uint64_t سائز,
                  ArrayRef<uint8_t> &بفر) override {
    واپس BBS.read_بائٹیں(انحراف, سائز, بفر);
  }
  Error readLongestContiguousChunk(uint64_t انحراف,
                                   ArrayRef<uint8_t> &بفر) override {
    واپس BBS.readLongestContiguousChunk(انحراف, بفر);
  }
  uint64_t get_لمبائی() override { واپس BBS.get_لمبائی(); }

نجی:
  ثنائی_Byte_دھارا BBS;
};

جماعت MutableArrayRefImpl : عوامی Writable_ثنائی_دھارا {
عوامی:
  MutableArrayRefImpl(MutableArrayRef<uint8_t> ڈیٹا_2, endianness Endian)
      : BBS(ڈیٹا_2, Endian) {}

  // Inherited via WritableBinaryStream
  llvm::endianness getEndian() ثابت override { واپس BBS.getEndian(); }
  Error read_بائٹیں(uint64_t انحراف, uint64_t سائز,
                  ArrayRef<uint8_t> &بفر) override {
    واپس BBS.read_بائٹیں(انحراف, سائز, بفر);
  }
  Error readLongestContiguousChunk(uint64_t انحراف,
                                   ArrayRef<uint8_t> &بفر) override {
    واپس BBS.readLongestContiguousChunk(انحراف, بفر);
  }
  uint64_t get_لمبائی() override { واپس BBS.get_لمبائی(); }

  Error write_بائٹیں(uint64_t انحراف, ArrayRef<uint8_t> ڈیٹا_2) override {
    واپس BBS.write_بائٹیں(انحراف, ڈیٹا_2);
  }
  Error commit() override { واپس BBS.commit(); }

نجی:
  Mutable_ثنائی_Byte_دھارا BBS;
};
} // namespace

ثنائی_دھارا_Ref::ثنائی_دھارا_Ref(ثنائی_دھارا &دھارا)
    : ثنائی_دھارا_Ref_بنیاد(دھارا) {}
ثنائی_دھارا_Ref::ثنائی_دھارا_Ref(ثنائی_دھارا &دھارا, uint64_t انحراف,
                                 std::optional<uint64_t> لمبائی)
    : ثنائی_دھارا_Ref_بنیاد(دھارا, انحراف, لمبائی) {}
ثنائی_دھارا_Ref::ثنائی_دھارا_Ref(ArrayRef<uint8_t> ڈیٹا_2, endianness Endian)
    : ثنائی_دھارا_Ref_بنیاد(std::make_shared<ArrayRefImpl>(ڈیٹا_2, Endian), 0,
                          ڈیٹا_2.سائز_2()) {}
ثنائی_دھارا_Ref::ثنائی_دھارا_Ref(StringRef ڈیٹا_2, endianness Endian)
    : ثنائی_دھارا_Ref(ArrayRef(ڈیٹا_2.بائٹیں_begin(), ڈیٹا_2.بائٹیں_انتہا()), Endian) {}

Error ثنائی_دھارا_Ref::read_بائٹیں(uint64_t انحراف, uint64_t سائز,
                                 ArrayRef<uint8_t> &بفر) ثابت {
  اگر (خودکار EC = check_انحراف_For_Read(انحراف, سائز))
    واپس EC;
  واپس BorrowedImpl->read_بائٹیں(View_انحراف + انحراف, سائز, بفر);
}

Error ثنائی_دھارا_Ref::readLongestContiguousChunk(
    uint64_t انحراف, ArrayRef<uint8_t> &بفر) ثابت {
  اگر (خودکار EC = check_انحراف_For_Read(انحراف, 1))
    واپس EC;

  اگر (خودکار EC =
          BorrowedImpl->readLongestContiguousChunk(View_انحراف + انحراف, بفر))
    واپس EC;
  // This StreamRef might refer to a smaller window over a larger stream.  In
  // that case we will have read out more bytes than we should return, because
  // we should not read past the end of the current view.
  uint64_t Max_لمبائی = get_لمبائی() - انحراف;
  اگر (بفر.سائز_2() > Max_لمبائی)
    بفر = بفر.قاش(0, Max_لمبائی);
  واپس Error::success();
}

Writable_ثنائی_دھارا_Ref::Writable_ثنائی_دھارا_Ref(Writable_ثنائی_دھارا &دھارا)
    : ثنائی_دھارا_Ref_بنیاد(دھارا) {}

Writable_ثنائی_دھارا_Ref::Writable_ثنائی_دھارا_Ref(Writable_ثنائی_دھارا &دھارا,
                                                 uint64_t انحراف,
                                                 std::optional<uint64_t> لمبائی)
    : ثنائی_دھارا_Ref_بنیاد(دھارا, انحراف, لمبائی) {}

Writable_ثنائی_دھارا_Ref::Writable_ثنائی_دھارا_Ref(MutableArrayRef<uint8_t> ڈیٹا_2,
                                                 endianness Endian)
    : ثنائی_دھارا_Ref_بنیاد(std::make_shared<MutableArrayRefImpl>(ڈیٹا_2, Endian),
                          0, ڈیٹا_2.سائز_2()) {}

Error Writable_ثنائی_دھارا_Ref::write_بائٹیں(uint64_t انحراف,
                                          ArrayRef<uint8_t> ڈیٹا_2) ثابت {
  اگر (خودکار EC = check_انحراف_For_Write(انحراف, ڈیٹا_2.سائز_2()))
    واپس EC;

  واپس BorrowedImpl->write_بائٹیں(View_انحراف + انحراف, ڈیٹا_2);
}

Writable_ثنائی_دھارا_Ref::عامل ثنائی_دھارا_Ref() ثابت {
  واپس ثنائی_دھارا_Ref(*BorrowedImpl, View_انحراف, لمبائی);
}

/// For buffered streams, commits changes to the backing store.
Error Writable_ثنائی_دھارا_Ref::commit() { واپس BorrowedImpl->commit(); }
