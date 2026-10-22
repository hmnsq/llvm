//===- BinaryStreamRef.cpp - ----------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ضمن "llvm/Support/BinaryStreamRef.h"
#ضمن "llvm/Support/BinaryByteStream.h"

استفاده نطاق llvm;

نطاق {

صنف ArrayRefImpl : عام دودویی_جریان {
عام:
  ArrayRefImpl(ArrayRef<uint8_t> داده_2, endianness Endian) : BBS(داده_2, Endian) {}

  llvm::endianness getEndian() ثابت override { أرجع BBS.getEndian(); }
  Error read_بایتها(uint64_t جابه_جایی, uint64_t اندازه,
                  ArrayRef<uint8_t> &میانگیر) override {
    أرجع BBS.read_بایتها(جابه_جایی, اندازه, میانگیر);
  }
  Error readLongestContiguousChunk(uint64_t جابه_جایی,
                                   ArrayRef<uint8_t> &میانگیر) override {
    أرجع BBS.readLongestContiguousChunk(جابه_جایی, میانگیر);
  }
  uint64_t get_درازی() override { أرجع BBS.get_درازی(); }

خاص:
  دودویی_Byte_جریان BBS;
};

صنف MutableArrayRefImpl : عام Writable_دودویی_جریان {
عام:
  MutableArrayRefImpl(MutableArrayRef<uint8_t> داده_2, endianness Endian)
      : BBS(داده_2, Endian) {}

  // Inherited via WritableBinaryStream
  llvm::endianness getEndian() ثابت override { أرجع BBS.getEndian(); }
  Error read_بایتها(uint64_t جابه_جایی, uint64_t اندازه,
                  ArrayRef<uint8_t> &میانگیر) override {
    أرجع BBS.read_بایتها(جابه_جایی, اندازه, میانگیر);
  }
  Error readLongestContiguousChunk(uint64_t جابه_جایی,
                                   ArrayRef<uint8_t> &میانگیر) override {
    أرجع BBS.readLongestContiguousChunk(جابه_جایی, میانگیر);
  }
  uint64_t get_درازی() override { أرجع BBS.get_درازی(); }

  Error write_بایتها(uint64_t جابه_جایی, ArrayRef<uint8_t> داده_2) override {
    أرجع BBS.write_بایتها(جابه_جایی, داده_2);
  }
  Error commit() override { أرجع BBS.commit(); }

خاص:
  Mutable_دودویی_Byte_جریان BBS;
};
} // namespace

دودویی_جریان_Ref::دودویی_جریان_Ref(دودویی_جریان &جریان)
    : دودویی_جریان_Ref_پایه(جریان) {}
دودویی_جریان_Ref::دودویی_جریان_Ref(دودویی_جریان &جریان, uint64_t جابه_جایی,
                                 std::optional<uint64_t> درازی)
    : دودویی_جریان_Ref_پایه(جریان, جابه_جایی, درازی) {}
دودویی_جریان_Ref::دودویی_جریان_Ref(ArrayRef<uint8_t> داده_2, endianness Endian)
    : دودویی_جریان_Ref_پایه(std::make_shared<ArrayRefImpl>(داده_2, Endian), 0,
                          داده_2.اندازه_2()) {}
دودویی_جریان_Ref::دودویی_جریان_Ref(StringRef داده_2, endianness Endian)
    : دودویی_جریان_Ref(ArrayRef(داده_2.بایتها_begin(), داده_2.بایتها_انتها()), Endian) {}

Error دودویی_جریان_Ref::read_بایتها(uint64_t جابه_جایی, uint64_t اندازه,
                                 ArrayRef<uint8_t> &میانگیر) ثابت {
  إذا (خودکار EC = check_جابه_جایی_For_Read(جابه_جایی, اندازه))
    أرجع EC;
  أرجع BorrowedImpl->read_بایتها(View_جابه_جایی + جابه_جایی, اندازه, میانگیر);
}

Error دودویی_جریان_Ref::readLongestContiguousChunk(
    uint64_t جابه_جایی, ArrayRef<uint8_t> &میانگیر) ثابت {
  إذا (خودکار EC = check_جابه_جایی_For_Read(جابه_جایی, 1))
    أرجع EC;

  إذا (خودکار EC =
          BorrowedImpl->readLongestContiguousChunk(View_جابه_جایی + جابه_جایی, میانگیر))
    أرجع EC;
  // This StreamRef might refer to a smaller window over a larger stream.  In
  // that case we will have read out more bytes than we should return, because
  // we should not read past the end of the current view.
  uint64_t Max_درازی = get_درازی() - جابه_جایی;
  إذا (میانگیر.اندازه_2() > Max_درازی)
    میانگیر = میانگیر.برش(0, Max_درازی);
  أرجع Error::success();
}

Writable_دودویی_جریان_Ref::Writable_دودویی_جریان_Ref(Writable_دودویی_جریان &جریان)
    : دودویی_جریان_Ref_پایه(جریان) {}

Writable_دودویی_جریان_Ref::Writable_دودویی_جریان_Ref(Writable_دودویی_جریان &جریان,
                                                 uint64_t جابه_جایی,
                                                 std::optional<uint64_t> درازی)
    : دودویی_جریان_Ref_پایه(جریان, جابه_جایی, درازی) {}

Writable_دودویی_جریان_Ref::Writable_دودویی_جریان_Ref(MutableArrayRef<uint8_t> داده_2,
                                                 endianness Endian)
    : دودویی_جریان_Ref_پایه(std::make_shared<MutableArrayRefImpl>(داده_2, Endian),
                          0, داده_2.اندازه_2()) {}

Error Writable_دودویی_جریان_Ref::write_بایتها(uint64_t جابه_جایی,
                                          ArrayRef<uint8_t> داده_2) ثابت {
  إذا (خودکار EC = check_جابه_جایی_For_Write(جابه_جایی, داده_2.اندازه_2()))
    أرجع EC;

  أرجع BorrowedImpl->write_بایتها(View_جابه_جایی + جابه_جایی, داده_2);
}

Writable_دودویی_جریان_Ref::عملگر دودویی_جریان_Ref() ثابت {
  أرجع دودویی_جریان_Ref(*BorrowedImpl, View_جابه_جایی, درازی);
}

/// For buffered streams, commits changes to the backing store.
Error Writable_دودویی_جریان_Ref::commit() { أرجع BorrowedImpl->commit(); }
