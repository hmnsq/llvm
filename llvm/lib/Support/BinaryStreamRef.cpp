//===- BinaryStreamRef.cpp - ----------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ضمن "llvm/Support/BinaryStreamRef.h"
#ضمن "llvm/Support/BinaryByteStream.h"

استعمال نطاق llvm;

نطاق {

صنف ArrayRefImpl : عام ٻٽو_وهڪرو {
عام:
  ArrayRefImpl(ArrayRef<uint8_t> معلومات_2, endianness Endian) : BBS(معلومات_2, Endian) {}

  llvm::endianness getEndian() ثابت override { أرجع BBS.getEndian(); }
  Error read_بائيٽون(uint64_t وٿي, uint64_t سائيز,
                  ArrayRef<uint8_t> &بفر) override {
    أرجع BBS.read_بائيٽون(وٿي, سائيز, بفر);
  }
  Error readLongestContiguousChunk(uint64_t وٿي,
                                   ArrayRef<uint8_t> &بفر) override {
    أرجع BBS.readLongestContiguousChunk(وٿي, بفر);
  }
  uint64_t get_ڊيگهائي() override { أرجع BBS.get_ڊيگهائي(); }

خاص:
  ٻٽو_Byte_وهڪرو BBS;
};

صنف MutableArrayRefImpl : عام Writable_ٻٽو_وهڪرو {
عام:
  MutableArrayRefImpl(MutableArrayRef<uint8_t> معلومات_2, endianness Endian)
      : BBS(معلومات_2, Endian) {}

  // Inherited via WritableBinaryStream
  llvm::endianness getEndian() ثابت override { أرجع BBS.getEndian(); }
  Error read_بائيٽون(uint64_t وٿي, uint64_t سائيز,
                  ArrayRef<uint8_t> &بفر) override {
    أرجع BBS.read_بائيٽون(وٿي, سائيز, بفر);
  }
  Error readLongestContiguousChunk(uint64_t وٿي,
                                   ArrayRef<uint8_t> &بفر) override {
    أرجع BBS.readLongestContiguousChunk(وٿي, بفر);
  }
  uint64_t get_ڊيگهائي() override { أرجع BBS.get_ڊيگهائي(); }

  Error write_بائيٽون(uint64_t وٿي, ArrayRef<uint8_t> معلومات_2) override {
    أرجع BBS.write_بائيٽون(وٿي, معلومات_2);
  }
  Error commit() override { أرجع BBS.commit(); }

خاص:
  Mutable_ٻٽو_Byte_وهڪرو BBS;
};
} // namespace

ٻٽو_وهڪرو_Ref::ٻٽو_وهڪرو_Ref(ٻٽو_وهڪرو &وهڪرو)
    : ٻٽو_وهڪرو_Ref_بنياد(وهڪرو) {}
ٻٽو_وهڪرو_Ref::ٻٽو_وهڪرو_Ref(ٻٽو_وهڪرو &وهڪرو, uint64_t وٿي,
                                 std::optional<uint64_t> ڊيگهائي)
    : ٻٽو_وهڪرو_Ref_بنياد(وهڪرو, وٿي, ڊيگهائي) {}
ٻٽو_وهڪرو_Ref::ٻٽو_وهڪرو_Ref(ArrayRef<uint8_t> معلومات_2, endianness Endian)
    : ٻٽو_وهڪرو_Ref_بنياد(std::make_shared<ArrayRefImpl>(معلومات_2, Endian), 0,
                          معلومات_2.سائيز_2()) {}
ٻٽو_وهڪرو_Ref::ٻٽو_وهڪرو_Ref(StringRef معلومات_2, endianness Endian)
    : ٻٽو_وهڪرو_Ref(ArrayRef(معلومات_2.بائيٽون_begin(), معلومات_2.بائيٽون_پڇاڙي()), Endian) {}

Error ٻٽو_وهڪرو_Ref::read_بائيٽون(uint64_t وٿي, uint64_t سائيز,
                                 ArrayRef<uint8_t> &بفر) ثابت {
  إذا (خودڪار EC = check_وٿي_For_Read(وٿي, سائيز))
    أرجع EC;
  أرجع BorrowedImpl->read_بائيٽون(View_وٿي + وٿي, سائيز, بفر);
}

Error ٻٽو_وهڪرو_Ref::readLongestContiguousChunk(
    uint64_t وٿي, ArrayRef<uint8_t> &بفر) ثابت {
  إذا (خودڪار EC = check_وٿي_For_Read(وٿي, 1))
    أرجع EC;

  إذا (خودڪار EC =
          BorrowedImpl->readLongestContiguousChunk(View_وٿي + وٿي, بفر))
    أرجع EC;
  // This StreamRef might refer to a smaller window over a larger stream.  In
  // that case we will have read out more bytes than we should return, because
  // we should not read past the end of the current view.
  uint64_t Max_ڊيگهائي = get_ڊيگهائي() - وٿي;
  إذا (بفر.سائيز_2() > Max_ڊيگهائي)
    بفر = بفر.ٽڪرو(0, Max_ڊيگهائي);
  أرجع Error::success();
}

Writable_ٻٽو_وهڪرو_Ref::Writable_ٻٽو_وهڪرو_Ref(Writable_ٻٽو_وهڪرو &وهڪرو)
    : ٻٽو_وهڪرو_Ref_بنياد(وهڪرو) {}

Writable_ٻٽو_وهڪرو_Ref::Writable_ٻٽو_وهڪرو_Ref(Writable_ٻٽو_وهڪرو &وهڪرو,
                                                 uint64_t وٿي,
                                                 std::optional<uint64_t> ڊيگهائي)
    : ٻٽو_وهڪرو_Ref_بنياد(وهڪرو, وٿي, ڊيگهائي) {}

Writable_ٻٽو_وهڪرو_Ref::Writable_ٻٽو_وهڪرو_Ref(MutableArrayRef<uint8_t> معلومات_2,
                                                 endianness Endian)
    : ٻٽو_وهڪرو_Ref_بنياد(std::make_shared<MutableArrayRefImpl>(معلومات_2, Endian),
                          0, معلومات_2.سائيز_2()) {}

Error Writable_ٻٽو_وهڪرو_Ref::write_بائيٽون(uint64_t وٿي,
                                          ArrayRef<uint8_t> معلومات_2) ثابت {
  إذا (خودڪار EC = check_وٿي_For_Write(وٿي, معلومات_2.سائيز_2()))
    أرجع EC;

  أرجع BorrowedImpl->write_بائيٽون(View_وٿي + وٿي, معلومات_2);
}

Writable_ٻٽو_وهڪرو_Ref::عامل ٻٽو_وهڪرو_Ref() ثابت {
  أرجع ٻٽو_وهڪرو_Ref(*BorrowedImpl, View_وٿي, ڊيگهائي);
}

/// For buffered streams, commits changes to the backing store.
Error Writable_ٻٽو_وهڪرو_Ref::commit() { أرجع BorrowedImpl->commit(); }
