//===--- MemoryBuffer.cpp - Memory Buffer implementation ------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
//  This file implements the MemoryBuffer interface.
//
//===----------------------------------------------------------------------===//

#ضمن "llvm/Support/MemoryBuffer.h"
#ضمن "llvm/ADT/STLExtras.h"
#ضمن "llvm/ADT/SmallString.h"
#ضمن "llvm/Config/config.h"
#ضمن "llvm/Support/Alignment.h"
#ضمن "llvm/Support/AutoConvert.h"
#ضمن "llvm/Support/Errc.h"
#ضمن "llvm/Support/Error.h"
#ضمن "llvm/Support/ErrorHandling.h"
#ضمن "llvm/Support/FileSystem.h"
#ضمن "llvm/Support/IOSandbox.h"
#ضمن "llvm/Support/Process.h"
#ضمن "llvm/Support/Program.h"
#ضمن "llvm/Support/SmallVectorMemoryBuffer.h"
#ضمن <algorithm>
#ضمن <cassert>
#ضمن <cstring>
#ضمن <new>
#ضمن <sys/types.h>
#ضمن <system_error>
#إذا !defined(_MSC_VER) && !defined(__MINGW32__)
#ضمن <unistd.h>
#وإلا
#ضمن <io.h>
#پایان_اگر

استفاده نطاق llvm;

//===----------------------------------------------------------------------===//
// MemoryBuffer implementation itself.
//===----------------------------------------------------------------------===//

حافظه_میانگیر::~حافظه_میانگیر() = افتراضي;

/// init - Initialize this MemoryBuffer as a reference to externally allocated
/// memory, memory that we know is already null terminated.
فراغ حافظه_میانگیر::init(ثابت حرف *Buf_شروع, ثابت حرف *Buf_انتها,
                        منطقي RequiresNullTerminator) {
  أكد((!RequiresNullTerminator || Buf_انتها[0] == 0) &&
         "Buffer is not null terminated!");
  میانگیر_شروع = Buf_شروع;
  میانگیر_انتها = Buf_انتها;
}

//===----------------------------------------------------------------------===//
// MemoryBufferMem implementation.
//===----------------------------------------------------------------------===//

/// CopyStringRef - Copies contents of a StringRef into a block of memory and
/// null-terminates it.
ساكن فراغ CopyStringRef(حرف *حافظه_2, StringRef داده_2) {
  إذا (!داده_2.خالی())
    کپی_حافظه(حافظه_2, داده_2.داده(), داده_2.اندازه_2());
  حافظه_2[داده_2.اندازه_2()] = 0; // Null terminate string.
}

نطاق {
بنية Named_میانگیر_Alloc {
  ثابت Twine &Name;
  Named_میانگیر_Alloc(ثابت Twine &Name) : Name(Name) {}
};
} // namespace

فراغ *عملگر جديد(size_t N, ثابت Named_میانگیر_Alloc &Alloc) {
  SmallString<256> NameBuf;
  StringRef NameRef = Alloc.Name.مقصد_String_Ref(NameBuf);

  // We use malloc() and manually handle it returning null instead of calling
  // operator new because we need all uses of NamedBufferAlloc to be
  // deallocated with a call to free() due to needing to use malloc() in
  // WritableMemoryBuffer::getNewUninitMemBuffer() to work around the out-of-
  // memory handler installed by default in LLVM. See operator delete() member
  // functions within this file for the paired call to free().
  حرف *Mem =
      static_cast<حرف *>(std::حافظه_بده(N + حجم(size_t) + NameRef.اندازه_2() + 1));
  إذا (!Mem)
    llvm::report_bad_alloc_error("Allocation failed");
  *reinterpret_cast<size_t *>(Mem + N) = NameRef.اندازه_2();
  CopyStringRef(Mem + N + حجم(size_t), NameRef);
  أرجع Mem;
}

نطاق {
/// MemoryBufferMem - Named MemoryBuffer pointing to a block of memory.
قالب<نام_نوع MB>
صنف حافظه_میانگیر_Mem : عام MB {
عام:
  حافظه_میانگیر_Mem(StringRef درون_داد_داده, منطقي RequiresNullTerminator) {
    حافظه_میانگیر::init(درون_داد_داده.begin(), درون_داد_داده.انتها(),
                       RequiresNullTerminator);
  }

  /// Disable sized deallocation for MemoryBufferMem, because it has
  /// tail-allocated data.
  فراغ عملگر حذف(فراغ *p) { std::حرر(p); }

  StringRef get_میانگیر_Identifier() ثابت override {
    // The name is stored after the class itself.
    أرجع StringRef(reinterpret_cast<ثابت حرف *>(هذا + 1) + حجم(size_t),
                     *reinterpret_cast<ثابت size_t *>(هذا + 1));
  }

  حافظه_میانگیر::میانگیر_نوع get_میانگیر_نوع() ثابت override {
    أرجع حافظه_میانگیر::حافظه_میانگیر_Malloc;
  }
};
} // namespace

قالب <نام_نوع MB>
ساكن ErrorOr<std::unique_ptr<MB>>
getFileAux(ثابت Twine &Filename, uint64_t Map_اندازه, uint64_t جابه_جایی,
           منطقي Is_متن, منطقي RequiresNullTerminator, منطقي IsVolatile,
           std::optional<Align> Alignment);

std::unique_ptr<حافظه_میانگیر>
حافظه_میانگیر::get_Mem_میانگیر(StringRef درون_داد_داده, StringRef میانگیر_Name,
                           منطقي RequiresNullTerminator) {
  خودکار *Ret = جديد (Named_میانگیر_Alloc(میانگیر_Name))
      حافظه_میانگیر_Mem<حافظه_میانگیر>(درون_داد_داده, RequiresNullTerminator);
  أرجع std::unique_ptr<حافظه_میانگیر>(Ret);
}

std::unique_ptr<حافظه_میانگیر>
حافظه_میانگیر::get_Mem_میانگیر(حافظه_میانگیر_Ref Ref, منطقي RequiresNullTerminator) {
  أرجع std::unique_ptr<حافظه_میانگیر>(get_Mem_میانگیر(
      Ref.get_میانگیر(), Ref.get_میانگیر_Identifier(), RequiresNullTerminator));
}

ساكن ErrorOr<std::unique_ptr<Writable_حافظه_میانگیر>>
get_Mem_میانگیر_Copy_Impl(StringRef درون_داد_داده, ثابت Twine &میانگیر_Name) {
  خودکار Buf =
      Writable_حافظه_میانگیر::get_New_Uninit_Mem_میانگیر(درون_داد_داده.اندازه_2(), میانگیر_Name);
  إذا (!Buf)
    أرجع make_error_کد(errc::not_enough_حافظه);
  // Calling memcpy with null src/dst is UB, and an empty StringRef is
  // represented with {nullptr, 0}.
  llvm::copy(درون_داد_داده, Buf->get_میانگیر_شروع());
  أرجع std::move(Buf);
}

std::unique_ptr<حافظه_میانگیر>
حافظه_میانگیر::get_Mem_میانگیر_Copy(StringRef درون_داد_داده, ثابت Twine &میانگیر_Name) {
  خودکار Buf = get_Mem_میانگیر_Copy_Impl(درون_داد_داده, میانگیر_Name);
  إذا (Buf)
    أرجع std::move(*Buf);
  أرجع اشاره_تهی;
}

ErrorOr<std::unique_ptr<حافظه_میانگیر>>
حافظه_میانگیر::getFileOrSTDIN(ثابت Twine &Filename, منطقي Is_متن,
                             منطقي RequiresNullTerminator,
                             std::optional<Align> Alignment) {
  sys::sandbox::violationIfEnabled();

  SmallString<256> NameBuf;
  StringRef NameRef = Filename.مقصد_String_Ref(NameBuf);

  إذا (NameRef == "-")
    أرجع getSTDIN();
  أرجع getFile(Filename, Is_متن, RequiresNullTerminator,
                 /*IsVolatile=*/خطأ, Alignment);
}

ErrorOr<std::unique_ptr<حافظه_میانگیر>>
حافظه_میانگیر::get_File_برش(ثابت Twine &File_مسیر, uint64_t Map_اندازه,
                           uint64_t جابه_جایی, منطقي IsVolatile,
                           std::optional<Align> Alignment) {
  sys::sandbox::violationIfEnabled();

  أرجع getFileAux<حافظه_میانگیر>(File_مسیر, Map_اندازه, جابه_جایی, /*IsText=*/خطأ,
                                  /*RequiresNullTerminator=*/خطأ, IsVolatile,
                                  Alignment);
}

//===----------------------------------------------------------------------===//
// MemoryBuffer::getFile implementation.
//===----------------------------------------------------------------------===//

نطاق {

قالب <نام_نوع MB>
constexpr sys::fs::mapped_file_region::mapmode Mapmode =
    sys::fs::mapped_file_region::readonly;
قالب <>
constexpr sys::fs::mapped_file_region::mapmode Mapmode<حافظه_میانگیر> =
    sys::fs::mapped_file_region::readonly;
قالب <>
constexpr sys::fs::mapped_file_region::mapmode Mapmode<Writable_حافظه_میانگیر> =
    sys::fs::mapped_file_region::priv;
قالب <>
constexpr sys::fs::mapped_file_region::mapmode
    Mapmode<Write_Through_حافظه_میانگیر> = sys::fs::mapped_file_region::readwrite;

/// Memory maps a file descriptor using sys::fs::mapped_file_region.
///
/// This handles converting the offset into a legal offset on the platform.
قالب<نام_نوع MB>
صنف حافظه_میانگیر_MMap_File : عام MB {
  sys::fs::mapped_file_region MFR;

  ساكن uint64_t get_Legal_Map_جابه_جایی(uint64_t جابه_جایی) {
    أرجع جابه_جایی & ~(sys::fs::mapped_file_region::alignment() - 1);
  }

  ساكن uint64_t get_Legal_Map_اندازه(uint64_t Len, uint64_t جابه_جایی) {
    أرجع Len + (جابه_جایی - get_Legal_Map_جابه_جایی(جابه_جایی));
  }

  ثابت حرف *get_شروع(uint64_t Len, uint64_t جابه_جایی) {
    أرجع MFR.const_داده() + (جابه_جایی - get_Legal_Map_جابه_جایی(جابه_جایی));
  }

عام:
  حافظه_میانگیر_MMap_File(منطقي RequiresNullTerminator, sys::fs::file_t FD, uint64_t Len,
                       uint64_t جابه_جایی, std::error_کد &EC)
      : MFR(FD, Mapmode<MB>, get_Legal_Map_اندازه(Len, جابه_جایی),
            get_Legal_Map_جابه_جایی(جابه_جایی), EC) {
    إذا (!EC) {
      ثابت حرف *شروع = get_شروع(Len, جابه_جایی);
      حافظه_میانگیر::init(شروع, شروع + Len, RequiresNullTerminator);
    }
  }

  /// Disable sized deallocation for MemoryBufferMMapFile, because it has
  /// tail-allocated data.
  فراغ عملگر حذف(فراغ *p) { std::حرر(p); }

  StringRef get_میانگیر_Identifier() ثابت override {
    // The name is stored after the class itself.
    أرجع StringRef(reinterpret_cast<ثابت حرف *>(هذا + 1) + حجم(size_t),
                     *reinterpret_cast<ثابت size_t *>(هذا + 1));
  }

  حافظه_میانگیر::میانگیر_نوع get_میانگیر_نوع() ثابت override {
    أرجع حافظه_میانگیر::حافظه_میانگیر_MMap;
  }

  فراغ dontNeedIfMmap() override { MFR.dontNeed(); }
  فراغ willNeedIfMmap() override { MFR.willNeed(); }
  فراغ randomAccessIfMmap() override { MFR.randomAccess(); }
};
} // namespace

ساكن ErrorOr<std::unique_ptr<Writable_حافظه_میانگیر>>
get_حافظه_میانگیر_For_جریان(sys::fs::file_t FD, ثابت Twine &میانگیر_Name) {
  SmallString<sys::fs::Default_Read_Chunk_اندازه> میانگیر;
  إذا (Error E = sys::fs::read_Native_File_مقصد_EOF(FD, میانگیر))
    أرجع error_مقصد_Error_کد(std::move(E));
  أرجع get_Mem_میانگیر_Copy_Impl(میانگیر, میانگیر_Name);
}

ErrorOr<std::unique_ptr<حافظه_میانگیر>>
حافظه_میانگیر::getFile(ثابت Twine &Filename, منطقي Is_متن,
                      منطقي RequiresNullTerminator, منطقي IsVolatile,
                      std::optional<Align> Alignment) {
  sys::sandbox::violationIfEnabled();

  أرجع getFileAux<حافظه_میانگیر>(Filename, /*MapSize=*/-1, /*Offset=*/0,
                                  Is_متن, RequiresNullTerminator, IsVolatile,
                                  Alignment);
}

قالب <نام_نوع MB>
ساكن ErrorOr<std::unique_ptr<MB>>
getOpenFileImpl(sys::fs::file_t FD, ثابت Twine &Filename, uint64_t File_اندازه,
                uint64_t Map_اندازه, int64_t جابه_جایی, منطقي RequiresNullTerminator,
                منطقي IsVolatile, std::optional<Align> Alignment);

قالب <نام_نوع MB>
ساكن ErrorOr<std::unique_ptr<MB>>
getFileAux(ثابت Twine &Filename, uint64_t Map_اندازه, uint64_t جابه_جایی,
           منطقي Is_متن, منطقي RequiresNullTerminator, منطقي IsVolatile,
           std::optional<Align> Alignment) {
  Expected<sys::fs::file_t> FDOrErr = sys::fs::openNativeFileForRead(
      Filename, Is_متن ? sys::fs::از_متن_With_CRLF : sys::fs::از_None);
  إذا (!FDOrErr)
    أرجع error_مقصد_Error_کد(FDOrErr.takeError());
  sys::fs::file_t FD = *FDOrErr;
  خودکار Ret = getOpenFileImpl<MB>(FD, Filename, /*FileSize=*/-1, Map_اندازه, جابه_جایی,
                                 RequiresNullTerminator, IsVolatile, Alignment);
  sys::fs::closeFile(FD);
  أرجع Ret;
}

ErrorOr<std::unique_ptr<Writable_حافظه_میانگیر>>
Writable_حافظه_میانگیر::getFile(ثابت Twine &Filename, منطقي IsVolatile,
                              std::optional<Align> Alignment) {
  sys::sandbox::violationIfEnabled();

  أرجع getFileAux<Writable_حافظه_میانگیر>(
      Filename, /*MapSize=*/-1, /*Offset=*/0, /*IsText=*/خطأ,
      /*RequiresNullTerminator=*/خطأ, IsVolatile, Alignment);
}

ErrorOr<std::unique_ptr<Writable_حافظه_میانگیر>>
Writable_حافظه_میانگیر::get_File_برش(ثابت Twine &Filename, uint64_t Map_اندازه,
                                   uint64_t جابه_جایی, منطقي IsVolatile,
                                   std::optional<Align> Alignment) {
  sys::sandbox::violationIfEnabled();

  أرجع getFileAux<Writable_حافظه_میانگیر>(
      Filename, Map_اندازه, جابه_جایی, /*IsText=*/خطأ,
      /*RequiresNullTerminator=*/خطأ, IsVolatile, Alignment);
}

std::unique_ptr<Writable_حافظه_میانگیر>
Writable_حافظه_میانگیر::get_New_Uninit_Mem_میانگیر(size_t اندازه,
                                            ثابت Twine &میانگیر_Name,
                                            std::optional<Align> Alignment) {
  استفاده Mem_میانگیر = حافظه_میانگیر_Mem<Writable_حافظه_میانگیر>;

  // Use 16-byte alignment if no alignment is specified.
  Align BufAlign = Alignment.مقدار_or(Align(16));

  // Allocate space for the MemoryBuffer, the data and the name. It is important
  // that MemoryBuffer and data are aligned so PointerIntPair works with them.
  SmallString<256> NameBuf;
  StringRef NameRef = میانگیر_Name.مقصد_String_Ref(NameBuf);

  size_t StringLen = حجم(Mem_میانگیر) + حجم(size_t) + NameRef.اندازه_2() + 1;
  size_t RealLen = StringLen + اندازه + 1 + BufAlign.مقدار_2();
  إذا (RealLen <= اندازه) // Check for rollover.
    أرجع اشاره_تهی;
  // We use a call to malloc() rather than a call to a non-throwing operator
  // new() because LLVM unconditionally installs an out of memory new handler
  // when exceptions are disabled. This new handler intentionally crashes to
  // aid with debugging, but that makes non-throwing new calls unhelpful.
  // See MemoryBufferMem::operator delete() for the paired call to free(), and
  // llvm::install_out_of_memory_new_handler() for the installation of the
  // custom new handler.
  حرف *Mem = static_cast<حرف *>(std::حافظه_بده(RealLen));
  إذا (!Mem)
    أرجع اشاره_تهی;

  // The name is stored after the class itself.
  *reinterpret_cast<size_t *>(Mem + حجم(Mem_میانگیر)) = NameRef.اندازه_2();
  CopyStringRef(Mem + حجم(Mem_میانگیر) + حجم(size_t), NameRef);

  // The buffer begins after the name and must be aligned.
  حرف *Buf = (حرف *)alignAddr(Mem + StringLen, BufAlign);
  Buf[اندازه] = 0; // Null terminate buffer.

  خودکار *Ret = جديد (Mem) Mem_میانگیر(StringRef(Buf, اندازه), درست);
  أرجع std::unique_ptr<Writable_حافظه_میانگیر>(Ret);
}

std::unique_ptr<Writable_حافظه_میانگیر>
Writable_حافظه_میانگیر::get_New_Mem_میانگیر(size_t اندازه, ثابت Twine &میانگیر_Name) {
  خودکار SB = Writable_حافظه_میانگیر::get_New_Uninit_Mem_میانگیر(اندازه, میانگیر_Name);
  إذا (!SB)
    أرجع اشاره_تهی;
  memset(SB->get_میانگیر_شروع(), 0, اندازه);
  أرجع SB;
}

ساكن منطقي shouldUseMmap(sys::fs::file_t FD,
                          size_t File_اندازه,
                          size_t Map_اندازه,
                          off_t جابه_جایی,
                          منطقي RequiresNullTerminator,
                          صحيح صفحه_اندازه,
                          منطقي IsVolatile) {
#إذا defined(__MVS__)
  // zOS Enhanced ASCII auto convert does not support mmap.
  أرجع خطأ;
#پایان_اگر

  // mmap may leave the buffer without null terminator if the file size changed
  // by the time the last page is mapped in, so avoid it if the file size is
  // likely to change.
  إذا (IsVolatile && RequiresNullTerminator)
    أرجع خطأ;

  // We don't use mmap for small files because this can severely fragment our
  // address space.
  إذا (Map_اندازه < 4 * 4096 || Map_اندازه < (بیعلامت)صفحه_اندازه)
    أرجع خطأ;

  إذا (!RequiresNullTerminator)
    أرجع درست;

  // If we don't know the file size, use fstat to find out.  fstat on an open
  // file descriptor is cheaper than stat on a random path.
  // FIXME: this chunk of code is duplicated, but it avoids a fstat when
  // RequiresNullTerminator = false and MapSize != -1.
  إذا (File_اندازه == size_t(-1)) {
    sys::fs::file_وضعیت وضعیت;
    إذا (sys::fs::وضعیت_2(FD, وضعیت))
      أرجع خطأ;
    File_اندازه = وضعیت.get_اندازه();
  }

  // If we need a null terminator and the end of the map is inside the file,
  // we cannot use mmap.
  size_t انتها_2 = جابه_جایی + Map_اندازه;
  أكد(انتها_2 <= File_اندازه);
  إذا (انتها_2 != File_اندازه)
    أرجع خطأ;

  // Don't try to map files that are exactly a multiple of the system page size
  // if we need a null terminator.
  إذا ((File_اندازه & (صفحه_اندازه -1)) == 0)
    أرجع خطأ;

#إذا defined(__CYGWIN__)
  // Don't try to map files that are exactly a multiple of the physical page size
  // if we need a null terminator.
  // FIXME: We should reorganize again getPageSize() on Win32.
  إذا ((File_اندازه & (4096 - 1)) == 0)
    أرجع خطأ;
#پایان_اگر

  أرجع درست;
}

ساكن ErrorOr<std::unique_ptr<Write_Through_حافظه_میانگیر>>
getReadWriteFile(ثابت Twine &Filename, uint64_t File_اندازه, uint64_t Map_اندازه,
                 uint64_t جابه_جایی) {
  Expected<sys::fs::file_t> FDOrErr = sys::fs::openNativeFileForReadWrite(
      Filename, sys::fs::CD_OpenExisting, sys::fs::از_None);
  إذا (!FDOrErr)
    أرجع error_مقصد_Error_کد(FDOrErr.takeError());
  sys::fs::file_t FD = *FDOrErr;

  // Default is to map the full file.
  إذا (Map_اندازه == uint64_t(-1)) {
    // If we don't know the file size, use fstat to find out.  fstat on an open
    // file descriptor is cheaper than stat on a random path.
    إذا (File_اندازه == uint64_t(-1)) {
      sys::fs::file_وضعیت وضعیت;
      std::error_کد EC = sys::fs::وضعیت_2(FD, وضعیت);
      إذا (EC)
        أرجع EC;

      // If this not a file or a block device (e.g. it's a named pipe
      // or character device), we can't mmap it, so error out.
      sys::fs::file_type Type = وضعیت.type();
      إذا (Type != sys::fs::file_type::regular_file &&
          Type != sys::fs::file_type::بلوک_file)
        أرجع make_error_کد(errc::invalid_آرگومان);

      File_اندازه = وضعیت.get_اندازه();
    }
    Map_اندازه = File_اندازه;
  }

  std::error_کد EC;
  std::unique_ptr<Write_Through_حافظه_میانگیر> نتیجه(
      جديد (Named_میانگیر_Alloc(Filename))
          حافظه_میانگیر_MMap_File<Write_Through_حافظه_میانگیر>(خطأ, FD, Map_اندازه,
                                                         جابه_جایی, EC));
  إذا (EC)
    أرجع EC;
  أرجع std::move(نتیجه);
}

ErrorOr<std::unique_ptr<Write_Through_حافظه_میانگیر>>
Write_Through_حافظه_میانگیر::getFile(ثابت Twine &Filename, int64_t File_اندازه) {
  sys::sandbox::violationIfEnabled();

  أرجع getReadWriteFile(Filename, File_اندازه, File_اندازه, 0);
}

/// Map a subrange of the specified file as a WritableMemoryBuffer.
ErrorOr<std::unique_ptr<Write_Through_حافظه_میانگیر>>
Write_Through_حافظه_میانگیر::get_File_برش(ثابت Twine &Filename, uint64_t Map_اندازه,
                                       uint64_t جابه_جایی) {
  sys::sandbox::violationIfEnabled();

  أرجع getReadWriteFile(Filename, -1, Map_اندازه, جابه_جایی);
}

قالب <نام_نوع MB>
ساكن ErrorOr<std::unique_ptr<MB>>
getOpenFileImpl(sys::fs::file_t FD, ثابت Twine &Filename, uint64_t File_اندازه,
                uint64_t Map_اندازه, int64_t جابه_جایی, منطقي RequiresNullTerminator,
                منطقي IsVolatile, std::optional<Align> Alignment) {
  ساكن صحيح صفحه_اندازه = sys::Process::get_صفحه_اندازه_Estimate();

  // Default is to map the full file.
  إذا (Map_اندازه == uint64_t(-1)) {
    // If we don't know the file size, use fstat to find out.  fstat on an open
    // file descriptor is cheaper than stat on a random path.
    إذا (File_اندازه == uint64_t(-1)) {
      sys::fs::file_وضعیت وضعیت;
      std::error_کد EC = sys::fs::وضعیت_2(FD, وضعیت);
      إذا (EC)
        أرجع EC;

      // If this not a file or a block device (e.g. it's a named pipe
      // or character device), we can't trust the size. Create the memory
      // buffer by copying off the stream.
      sys::fs::file_type Type = وضعیت.type();
      إذا (Type != sys::fs::file_type::regular_file &&
          Type != sys::fs::file_type::بلوک_file)
        أرجع get_حافظه_میانگیر_For_جریان(FD, Filename);

      File_اندازه = وضعیت.get_اندازه();
    }
    Map_اندازه = File_اندازه;
  }

  إذا (shouldUseMmap(FD, File_اندازه, Map_اندازه, جابه_جایی, RequiresNullTerminator,
                    صفحه_اندازه, IsVolatile)) {
    std::error_کد EC;
    std::unique_ptr<MB> نتیجه(
        جديد (Named_میانگیر_Alloc(Filename)) حافظه_میانگیر_MMap_File<MB>(
            RequiresNullTerminator, FD, Map_اندازه, جابه_جایی, EC));
    إذا (!EC) {
      // On at least Linux, and possibly on other systems, mmap may return pages
      // from the page cache that are not properly filled with trailing zeroes,
      // if some prior user of the page wrote non-zero bytes. Detect this and
      // don't use mmap in that case.
      إذا (!RequiresNullTerminator || *نتیجه->get_میانگیر_انتها() == '\0')
        أرجع std::move(نتیجه);
    }
  }

#اگر_تعریف_شده __MVS__
  ErrorOr<منطقي> NeedsConversion = needConversion(Filename, FD);
  إذا (std::error_کد EC = NeedsConversion.getError())
    أرجع EC;
  // File size may increase due to EBCDIC -> UTF-8 conversion, therefore we
  // cannot trust the file size and we create the memory buffer by copying
  // off the stream.
  // Note: This only works with the assumption of reading a full file (i.e,
  // Offset == 0 and MapSize == FileSize). Reading a file slice does not work.
  إذا (*NeedsConversion && جابه_جایی == 0 && Map_اندازه == File_اندازه)
    أرجع get_حافظه_میانگیر_For_جریان(FD, Filename);
#پایان_اگر

  خودکار Buf =
      Writable_حافظه_میانگیر::get_New_Uninit_Mem_میانگیر(Map_اندازه, Filename, Alignment);
  إذا (!Buf) {
    // Failed to create a buffer. The only way it can fail is if
    // new(std::nothrow) returns 0.
    أرجع make_error_کد(errc::not_enough_حافظه);
  }

  // Read until EOF, zero-initialize the rest.
  MutableArrayRef<حرف> مقصد_Read = Buf->get_میانگیر();
  طالما (!مقصد_Read.خالی()) {
    Expected<size_t> Read_بایتها =
        sys::fs::read_Native_File_برش(FD, مقصد_Read, جابه_جایی);
    إذا (!Read_بایتها)
      أرجع error_مقصد_Error_کد(Read_بایتها.takeError());
    إذا (*Read_بایتها == 0) {
      std::memset(مقصد_Read.داده(), 0, مقصد_Read.اندازه_2());
      اكسر;
    }
    مقصد_Read = مقصد_Read.drop_front(*Read_بایتها);
    جابه_جایی += *Read_بایتها;
  }

  أرجع std::move(Buf);
}

ErrorOr<std::unique_ptr<حافظه_میانگیر>>
حافظه_میانگیر::getOpenFile(sys::fs::file_t FD, ثابت Twine &Filename,
                          uint64_t File_اندازه, منطقي RequiresNullTerminator,
                          منطقي IsVolatile, std::optional<Align> Alignment) {
  sys::sandbox::violationIfEnabled();

  أرجع getOpenFileImpl<حافظه_میانگیر>(FD, Filename, File_اندازه, File_اندازه, 0,
                                       RequiresNullTerminator, IsVolatile,
                                       Alignment);
}

ErrorOr<std::unique_ptr<حافظه_میانگیر>> حافظه_میانگیر::get_Open_File_برش(
    sys::fs::file_t FD, ثابت Twine &Filename, uint64_t Map_اندازه, int64_t جابه_جایی,
    منطقي IsVolatile, std::optional<Align> Alignment) {
  أكد(Map_اندازه != uint64_t(-1));

  sys::sandbox::violationIfEnabled();

  أرجع getOpenFileImpl<حافظه_میانگیر>(FD, Filename, -1, Map_اندازه, جابه_جایی, خطأ,
                                       IsVolatile, Alignment);
}

ErrorOr<std::unique_ptr<حافظه_میانگیر>> حافظه_میانگیر::getSTDIN() {
  sys::sandbox::violationIfEnabled();

  // Read in all of the data from stdin, we cannot mmap stdin.
  //
  // FIXME: That isn't necessarily true, we should try to mmap stdin and
  // fallback if it fails.
  sys::Change_Stdin_نما(sys::fs::از_متن);

  أرجع get_حافظه_میانگیر_For_جریان(sys::fs::getStdinHandle(), "<stdin>");
}

ErrorOr<std::unique_ptr<حافظه_میانگیر>>
حافظه_میانگیر::get_File_As_جریان(ثابت Twine &Filename) {
  sys::sandbox::violationIfEnabled();

  Expected<sys::fs::file_t> FDOrErr =
      sys::fs::openNativeFileForRead(Filename, sys::fs::از_None);
  إذا (!FDOrErr)
    أرجع error_مقصد_Error_کد(FDOrErr.takeError());
  sys::fs::file_t FD = *FDOrErr;
  ErrorOr<std::unique_ptr<حافظه_میانگیر>> Ret =
      get_حافظه_میانگیر_For_جریان(FD, Filename);
  sys::fs::closeFile(FD);
  أرجع Ret;
}

حافظه_میانگیر_Ref حافظه_میانگیر::get_Mem_میانگیر_Ref() ثابت {
  StringRef داده_2 = get_میانگیر();
  StringRef Identifier = get_میانگیر_Identifier();
  أرجع حافظه_میانگیر_Ref(داده_2, Identifier);
}

Small_وکتور_حافظه_میانگیر::~Small_وکتور_حافظه_میانگیر() = افتراضي;
