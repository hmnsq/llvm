//===- BinaryStreamRef.cpp - ----------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ضمن "llvm/Support/BinaryStreamRef.h"
#ضمن "llvm/Support/BinaryByteStream.h"

ورتوں نطاق llvm;

نطاق {

صنف ArrayRefImpl : عام ثنائی_دھارا {
عام:
  ArrayRefImpl(ArrayRef<uint8_t> انکڑے_2, endianness Endian) : BBS(انکڑے_2, Endian) {}

  llvm::endianness getEndian() ثابت override { أرجع BBS.getEndian(); }
  Error read_بائٹاں(uint64_t وتھ, uint64_t ناپ,
                  ArrayRef<uint8_t> &بفر) override {
    أرجع BBS.read_بائٹاں(وتھ, ناپ, بفر);
  }
  Error readLongestContiguousChunk(uint64_t وتھ,
                                   ArrayRef<uint8_t> &بفر) override {
    أرجع BBS.readLongestContiguousChunk(وتھ, بفر);
  }
  uint64_t get_لمائی() override { أرجع BBS.get_لمائی(); }

خاص:
  ثنائی_Byte_دھارا BBS;
};

صنف MutableArrayRefImpl : عام Writable_ثنائی_دھارا {
عام:
  MutableArrayRefImpl(MutableArrayRef<uint8_t> انکڑے_2, endianness Endian)
      : BBS(انکڑے_2, Endian) {}

  // Inherited via WritableBinaryStream
  llvm::endianness getEndian() ثابت override { أرجع BBS.getEndian(); }
  Error read_بائٹاں(uint64_t وتھ, uint64_t ناپ,
                  ArrayRef<uint8_t> &بفر) override {
    أرجع BBS.read_بائٹاں(وتھ, ناپ, بفر);
  }
  Error readLongestContiguousChunk(uint64_t وتھ,
                                   ArrayRef<uint8_t> &بفر) override {
    أرجع BBS.readLongestContiguousChunk(وتھ, بفر);
  }
  uint64_t get_لمائی() override { أرجع BBS.get_لمائی(); }

  Error write_بائٹاں(uint64_t وتھ, ArrayRef<uint8_t> انکڑے_2) override {
    أرجع BBS.write_بائٹاں(وتھ, انکڑے_2);
  }
  Error commit() override { أرجع BBS.commit(); }

خاص:
  Mutable_ثنائی_Byte_دھارا BBS;
};
} // namespace

ثنائی_دھارا_Ref::ثنائی_دھارا_Ref(ثنائی_دھارا &دھارا)
    : ثنائی_دھارا_Ref_ادھار(دھارا) {}
ثنائی_دھارا_Ref::ثنائی_دھارا_Ref(ثنائی_دھارا &دھارا, uint64_t وتھ,
                                 std::optional<uint64_t> لمائی)
    : ثنائی_دھارا_Ref_ادھار(دھارا, وتھ, لمائی) {}
ثنائی_دھارا_Ref::ثنائی_دھارا_Ref(ArrayRef<uint8_t> انکڑے_2, endianness Endian)
    : ثنائی_دھارا_Ref_ادھار(std::make_shared<ArrayRefImpl>(انکڑے_2, Endian), 0,
                          انکڑے_2.ناپ_2()) {}
ثنائی_دھارا_Ref::ثنائی_دھارا_Ref(StringRef انکڑے_2, endianness Endian)
    : ثنائی_دھارا_Ref(ArrayRef(انکڑے_2.بائٹاں_begin(), انکڑے_2.بائٹاں_سرا()), Endian) {}

Error ثنائی_دھارا_Ref::read_بائٹاں(uint64_t وتھ, uint64_t ناپ,
                                 ArrayRef<uint8_t> &بفر) ثابت {
  إذا (خودکار EC = check_وتھ_For_Read(وتھ, ناپ))
    أرجع EC;
  أرجع BorrowedImpl->read_بائٹاں(View_وتھ + وتھ, ناپ, بفر);
}

Error ثنائی_دھارا_Ref::readLongestContiguousChunk(
    uint64_t وتھ, ArrayRef<uint8_t> &بفر) ثابت {
  إذا (خودکار EC = check_وتھ_For_Read(وتھ, 1))
    أرجع EC;

  إذا (خودکار EC =
          BorrowedImpl->readLongestContiguousChunk(View_وتھ + وتھ, بفر))
    أرجع EC;
  // This StreamRef might refer to a smaller window over a larger stream.  In
  // that case we will have read out more bytes than we should return, because
  // we should not read past the end of the current view.
  uint64_t Max_لمائی = get_لمائی() - وتھ;
  إذا (بفر.ناپ_2() > Max_لمائی)
    بفر = بفر.ٹکڑا(0, Max_لمائی);
  أرجع Error::success();
}

Writable_ثنائی_دھارا_Ref::Writable_ثنائی_دھارا_Ref(Writable_ثنائی_دھارا &دھارا)
    : ثنائی_دھارا_Ref_ادھار(دھارا) {}

Writable_ثنائی_دھارا_Ref::Writable_ثنائی_دھارا_Ref(Writable_ثنائی_دھارا &دھارا,
                                                 uint64_t وتھ,
                                                 std::optional<uint64_t> لمائی)
    : ثنائی_دھارا_Ref_ادھار(دھارا, وتھ, لمائی) {}

Writable_ثنائی_دھارا_Ref::Writable_ثنائی_دھارا_Ref(MutableArrayRef<uint8_t> انکڑے_2,
                                                 endianness Endian)
    : ثنائی_دھارا_Ref_ادھار(std::make_shared<MutableArrayRefImpl>(انکڑے_2, Endian),
                          0, انکڑے_2.ناپ_2()) {}

Error Writable_ثنائی_دھارا_Ref::write_بائٹاں(uint64_t وتھ,
                                          ArrayRef<uint8_t> انکڑے_2) ثابت {
  إذا (خودکار EC = check_وتھ_For_Write(وتھ, انکڑے_2.ناپ_2()))
    أرجع EC;

  أرجع BorrowedImpl->write_بائٹاں(View_وتھ + وتھ, انکڑے_2);
}

Writable_ثنائی_دھارا_Ref::عامل ثنائی_دھارا_Ref() ثابت {
  أرجع ثنائی_دھارا_Ref(*BorrowedImpl, View_وتھ, لمائی);
}

/// For buffered streams, commits changes to the backing store.
Error Writable_ثنائی_دھارا_Ref::commit() { أرجع BorrowedImpl->commit(); }
