//===- BinaryStreamRef.cpp - ----------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ضمن "llvm/Support/BinaryStreamRef.h"
#ضمن "llvm/Support/BinaryByteStream.h"

کارول نطاق llvm;

نطاق {

صنف ArrayRefImpl : عام دوګونی_بهیر {
عام:
  ArrayRefImpl(ArrayRef<uint8_t> معلومات_2, endianness Endian) : BBS(معلومات_2, Endian) {}

  llvm::endianness getEndian() ثابت override { أرجع BBS.getEndian(); }
  Error read_بایټونه(uint64_t دننه_واټن, uint64_t اندازه,
                  ArrayRef<uint8_t> &بفر) override {
    أرجع BBS.read_بایټونه(دننه_واټن, اندازه, بفر);
  }
  Error readLongestContiguousChunk(uint64_t دننه_واټن,
                                   ArrayRef<uint8_t> &بفر) override {
    أرجع BBS.readLongestContiguousChunk(دننه_واټن, بفر);
  }
  uint64_t get_اوږدتیا() override { أرجع BBS.get_اوږدتیا(); }

خاص:
  دوګونی_Byte_بهیر BBS;
};

صنف MutableArrayRefImpl : عام Writable_دوګونی_بهیر {
عام:
  MutableArrayRefImpl(MutableArrayRef<uint8_t> معلومات_2, endianness Endian)
      : BBS(معلومات_2, Endian) {}

  // Inherited via WritableBinaryStream
  llvm::endianness getEndian() ثابت override { أرجع BBS.getEndian(); }
  Error read_بایټونه(uint64_t دننه_واټن, uint64_t اندازه,
                  ArrayRef<uint8_t> &بفر) override {
    أرجع BBS.read_بایټونه(دننه_واټن, اندازه, بفر);
  }
  Error readLongestContiguousChunk(uint64_t دننه_واټن,
                                   ArrayRef<uint8_t> &بفر) override {
    أرجع BBS.readLongestContiguousChunk(دننه_واټن, بفر);
  }
  uint64_t get_اوږدتیا() override { أرجع BBS.get_اوږدتیا(); }

  Error write_بایټونه(uint64_t دننه_واټن, ArrayRef<uint8_t> معلومات_2) override {
    أرجع BBS.write_بایټونه(دننه_واټن, معلومات_2);
  }
  Error commit() override { أرجع BBS.commit(); }

خاص:
  Mutable_دوګونی_Byte_بهیر BBS;
};
} // namespace

دوګونی_بهیر_Ref::دوګونی_بهیر_Ref(دوګونی_بهیر &بهیر)
    : دوګونی_بهیر_Ref_بنسټ(بهیر) {}
دوګونی_بهیر_Ref::دوګونی_بهیر_Ref(دوګونی_بهیر &بهیر, uint64_t دننه_واټن,
                                 std::optional<uint64_t> اوږدتیا)
    : دوګونی_بهیر_Ref_بنسټ(بهیر, دننه_واټن, اوږدتیا) {}
دوګونی_بهیر_Ref::دوګونی_بهیر_Ref(ArrayRef<uint8_t> معلومات_2, endianness Endian)
    : دوګونی_بهیر_Ref_بنسټ(std::make_shared<ArrayRefImpl>(معلومات_2, Endian), 0,
                          معلومات_2.اندازه_2()) {}
دوګونی_بهیر_Ref::دوګونی_بهیر_Ref(StringRef معلومات_2, endianness Endian)
    : دوګونی_بهیر_Ref(ArrayRef(معلومات_2.بایټونه_begin(), معلومات_2.بایټونه_پای_ټکی()), Endian) {}

Error دوګونی_بهیر_Ref::read_بایټونه(uint64_t دننه_واټن, uint64_t اندازه,
                                 ArrayRef<uint8_t> &بفر) ثابت {
  إذا (خپلکاری EC = check_دننه_واټن_For_Read(دننه_واټن, اندازه))
    أرجع EC;
  أرجع BorrowedImpl->read_بایټونه(View_دننه_واټن + دننه_واټن, اندازه, بفر);
}

Error دوګونی_بهیر_Ref::readLongestContiguousChunk(
    uint64_t دننه_واټن, ArrayRef<uint8_t> &بفر) ثابت {
  إذا (خپلکاری EC = check_دننه_واټن_For_Read(دننه_واټن, 1))
    أرجع EC;

  إذا (خپلکاری EC =
          BorrowedImpl->readLongestContiguousChunk(View_دننه_واټن + دننه_واټن, بفر))
    أرجع EC;
  // This StreamRef might refer to a smaller window over a larger stream.  In
  // that case we will have read out more bytes than we should return, because
  // we should not read past the end of the current view.
  uint64_t Max_اوږدتیا = get_اوږدتیا() - دننه_واټن;
  إذا (بفر.اندازه_2() > Max_اوږدتیا)
    بفر = بفر.ټوټه(0, Max_اوږدتیا);
  أرجع Error::success();
}

Writable_دوګونی_بهیر_Ref::Writable_دوګونی_بهیر_Ref(Writable_دوګونی_بهیر &بهیر)
    : دوګونی_بهیر_Ref_بنسټ(بهیر) {}

Writable_دوګونی_بهیر_Ref::Writable_دوګونی_بهیر_Ref(Writable_دوګونی_بهیر &بهیر,
                                                 uint64_t دننه_واټن,
                                                 std::optional<uint64_t> اوږدتیا)
    : دوګونی_بهیر_Ref_بنسټ(بهیر, دننه_واټن, اوږدتیا) {}

Writable_دوګونی_بهیر_Ref::Writable_دوګونی_بهیر_Ref(MutableArrayRef<uint8_t> معلومات_2,
                                                 endianness Endian)
    : دوګونی_بهیر_Ref_بنسټ(std::make_shared<MutableArrayRefImpl>(معلومات_2, Endian),
                          0, معلومات_2.اندازه_2()) {}

Error Writable_دوګونی_بهیر_Ref::write_بایټونه(uint64_t دننه_واټن,
                                          ArrayRef<uint8_t> معلومات_2) ثابت {
  إذا (خپلکاری EC = check_دننه_واټن_For_Write(دننه_واټن, معلومات_2.اندازه_2()))
    أرجع EC;

  أرجع BorrowedImpl->write_بایټونه(View_دننه_واټن + دننه_واټن, معلومات_2);
}

Writable_دوګونی_بهیر_Ref::چلوونکی دوګونی_بهیر_Ref() ثابت {
  أرجع دوګونی_بهیر_Ref(*BorrowedImpl, View_دننه_واټن, اوږدتیا);
}

/// For buffered streams, commits changes to the backing store.
Error Writable_دوګونی_بهیر_Ref::commit() { أرجع BorrowedImpl->commit(); }
