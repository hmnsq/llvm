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
#جيڪڏهن_پورو

استعمال نطاق llvm;

//===----------------------------------------------------------------------===//
// MemoryBuffer implementation itself.
//===----------------------------------------------------------------------===//

يادگيري_بفر::~يادگيري_بفر() = افتراضي;

/// init - Initialize this MemoryBuffer as a reference to externally allocated
/// memory, memory that we know is already null terminated.
فراغ يادگيري_بفر::init(ثابت حرف *Buf_شروعات, ثابت حرف *Buf_پڇاڙي,
                        منطقي RequiresNullTerminator) {
  أكد((!RequiresNullTerminator || Buf_پڇاڙي[0] == 0) &&
         "Buffer is not null terminated!");
  بفر_شروعات = Buf_شروعات;
  بفر_پڇاڙي = Buf_پڇاڙي;
}

//===----------------------------------------------------------------------===//
// MemoryBufferMem implementation.
//===----------------------------------------------------------------------===//

/// CopyStringRef - Copies contents of a StringRef into a block of memory and
/// null-terminates it.
ساكن فراغ CopyStringRef(حرف *يادگيري_2, StringRef معلومات_2) {
  إذا (!معلومات_2.خالي_جايون())
    يادگيري_نقل(يادگيري_2, معلومات_2.معلومات(), معلومات_2.سائيز_2());
  يادگيري_2[معلومات_2.سائيز_2()] = 0; // Null terminate string.
}

نطاق {
بنية Named_بفر_Alloc {
  ثابت Twine &Name;
  Named_بفر_Alloc(ثابت Twine &Name) : Name(Name) {}
};
} // namespace

فراغ *عامل جديد(size_t N, ثابت Named_بفر_Alloc &Alloc) {
  SmallString<256> NameBuf;
  StringRef NameRef = Alloc.Name.منزل_String_Ref(NameBuf);

  // We use malloc() and manually handle it returning null instead of calling
  // operator new because we need all uses of NamedBufferAlloc to be
  // deallocated with a call to free() due to needing to use malloc() in
  // WritableMemoryBuffer::getNewUninitMemBuffer() to work around the out-of-
  // memory handler installed by default in LLVM. See operator delete() member
  // functions within this file for the paired call to free().
  حرف *Mem =
      static_cast<حرف *>(std::يادگيري_ڏيو(N + حجم(size_t) + NameRef.سائيز_2() + 1));
  إذا (!Mem)
    llvm::report_bad_alloc_error("Allocation failed");
  *reinterpret_cast<size_t *>(Mem + N) = NameRef.سائيز_2();
  CopyStringRef(Mem + N + حجم(size_t), NameRef);
  أرجع Mem;
}

نطاق {
/// MemoryBufferMem - Named MemoryBuffer pointing to a block of memory.
سانچو<قسم_نالو MB>
صنف يادگيري_بفر_Mem : عام MB {
عام:
  يادگيري_بفر_Mem(StringRef داخلا_معلومات, منطقي RequiresNullTerminator) {
    يادگيري_بفر::init(داخلا_معلومات.begin(), داخلا_معلومات.پڇاڙي(),
                       RequiresNullTerminator);
  }

  /// Disable sized deallocation for MemoryBufferMem, because it has
  /// tail-allocated data.
  فراغ عامل ميٽيو(فراغ *p) { std::حرر(p); }

  StringRef get_بفر_Identifier() ثابت override {
    // The name is stored after the class itself.
    أرجع StringRef(reinterpret_cast<ثابت حرف *>(هذا + 1) + حجم(size_t),
                     *reinterpret_cast<ثابت size_t *>(هذا + 1));
  }

  يادگيري_بفر::بفر_قسم get_بفر_قسم() ثابت override {
    أرجع يادگيري_بفر::يادگيري_بفر_Malloc;
  }
};
} // namespace

سانچو <قسم_نالو MB>
ساكن ErrorOr<std::unique_ptr<MB>>
getFileAux(ثابت Twine &Filename, uint64_t Map_سائيز, uint64_t وٿي,
           منطقي Is_متن, منطقي RequiresNullTerminator, منطقي IsVolatile,
           std::optional<Align> Alignment);

std::unique_ptr<يادگيري_بفر>
يادگيري_بفر::get_Mem_بفر(StringRef داخلا_معلومات, StringRef بفر_Name,
                           منطقي RequiresNullTerminator) {
  خودڪار *Ret = جديد (Named_بفر_Alloc(بفر_Name))
      يادگيري_بفر_Mem<يادگيري_بفر>(داخلا_معلومات, RequiresNullTerminator);
  أرجع std::unique_ptr<يادگيري_بفر>(Ret);
}

std::unique_ptr<يادگيري_بفر>
يادگيري_بفر::get_Mem_بفر(يادگيري_بفر_Ref Ref, منطقي RequiresNullTerminator) {
  أرجع std::unique_ptr<يادگيري_بفر>(get_Mem_بفر(
      Ref.get_بفر(), Ref.get_بفر_Identifier(), RequiresNullTerminator));
}

ساكن ErrorOr<std::unique_ptr<Writable_يادگيري_بفر>>
get_Mem_بفر_Copy_Impl(StringRef داخلا_معلومات, ثابت Twine &بفر_Name) {
  خودڪار Buf =
      Writable_يادگيري_بفر::get_New_Uninit_Mem_بفر(داخلا_معلومات.سائيز_2(), بفر_Name);
  إذا (!Buf)
    أرجع make_error_ڪوڊ(errc::not_enough_يادگيري);
  // Calling memcpy with null src/dst is UB, and an empty StringRef is
  // represented with {nullptr, 0}.
  llvm::copy(داخلا_معلومات, Buf->get_بفر_شروعات());
  أرجع std::move(Buf);
}

std::unique_ptr<يادگيري_بفر>
يادگيري_بفر::get_Mem_بفر_Copy(StringRef داخلا_معلومات, ثابت Twine &بفر_Name) {
  خودڪار Buf = get_Mem_بفر_Copy_Impl(داخلا_معلومات, بفر_Name);
  إذا (Buf)
    أرجع std::move(*Buf);
  أرجع خالي_اشارو;
}

ErrorOr<std::unique_ptr<يادگيري_بفر>>
يادگيري_بفر::getFileOrSTDIN(ثابت Twine &Filename, منطقي Is_متن,
                             منطقي RequiresNullTerminator,
                             std::optional<Align> Alignment) {
  sys::sandbox::violationIfEnabled();

  SmallString<256> NameBuf;
  StringRef NameRef = Filename.منزل_String_Ref(NameBuf);

  إذا (NameRef == "-")
    أرجع getSTDIN();
  أرجع getFile(Filename, Is_متن, RequiresNullTerminator,
                 /*IsVolatile=*/خطأ, Alignment);
}

ErrorOr<std::unique_ptr<يادگيري_بفر>>
يادگيري_بفر::get_File_ٽڪرو(ثابت Twine &File_رستو, uint64_t Map_سائيز,
                           uint64_t وٿي, منطقي IsVolatile,
                           std::optional<Align> Alignment) {
  sys::sandbox::violationIfEnabled();

  أرجع getFileAux<يادگيري_بفر>(File_رستو, Map_سائيز, وٿي, /*IsText=*/خطأ,
                                  /*RequiresNullTerminator=*/خطأ, IsVolatile,
                                  Alignment);
}

//===----------------------------------------------------------------------===//
// MemoryBuffer::getFile implementation.
//===----------------------------------------------------------------------===//

نطاق {

سانچو <قسم_نالو MB>
constexpr sys::fs::mapped_file_region::mapmode Mapmode =
    sys::fs::mapped_file_region::readonly;
سانچو <>
constexpr sys::fs::mapped_file_region::mapmode Mapmode<يادگيري_بفر> =
    sys::fs::mapped_file_region::readonly;
سانچو <>
constexpr sys::fs::mapped_file_region::mapmode Mapmode<Writable_يادگيري_بفر> =
    sys::fs::mapped_file_region::priv;
سانچو <>
constexpr sys::fs::mapped_file_region::mapmode
    Mapmode<Write_Through_يادگيري_بفر> = sys::fs::mapped_file_region::readwrite;

/// Memory maps a file descriptor using sys::fs::mapped_file_region.
///
/// This handles converting the offset into a legal offset on the platform.
سانچو<قسم_نالو MB>
صنف يادگيري_بفر_MMap_File : عام MB {
  sys::fs::mapped_file_region MFR;

  ساكن uint64_t get_Legal_Map_وٿي(uint64_t وٿي) {
    أرجع وٿي & ~(sys::fs::mapped_file_region::alignment() - 1);
  }

  ساكن uint64_t get_Legal_Map_سائيز(uint64_t Len, uint64_t وٿي) {
    أرجع Len + (وٿي - get_Legal_Map_وٿي(وٿي));
  }

  ثابت حرف *get_شروعات(uint64_t Len, uint64_t وٿي) {
    أرجع MFR.const_معلومات() + (وٿي - get_Legal_Map_وٿي(وٿي));
  }

عام:
  يادگيري_بفر_MMap_File(منطقي RequiresNullTerminator, sys::fs::file_t FD, uint64_t Len,
                       uint64_t وٿي, std::error_ڪوڊ &EC)
      : MFR(FD, Mapmode<MB>, get_Legal_Map_سائيز(Len, وٿي),
            get_Legal_Map_وٿي(وٿي), EC) {
    إذا (!EC) {
      ثابت حرف *شروعات = get_شروعات(Len, وٿي);
      يادگيري_بفر::init(شروعات, شروعات + Len, RequiresNullTerminator);
    }
  }

  /// Disable sized deallocation for MemoryBufferMMapFile, because it has
  /// tail-allocated data.
  فراغ عامل ميٽيو(فراغ *p) { std::حرر(p); }

  StringRef get_بفر_Identifier() ثابت override {
    // The name is stored after the class itself.
    أرجع StringRef(reinterpret_cast<ثابت حرف *>(هذا + 1) + حجم(size_t),
                     *reinterpret_cast<ثابت size_t *>(هذا + 1));
  }

  يادگيري_بفر::بفر_قسم get_بفر_قسم() ثابت override {
    أرجع يادگيري_بفر::يادگيري_بفر_MMap;
  }

  فراغ dontNeedIfMmap() override { MFR.dontNeed(); }
  فراغ willNeedIfMmap() override { MFR.willNeed(); }
  فراغ randomAccessIfMmap() override { MFR.randomAccess(); }
};
} // namespace

ساكن ErrorOr<std::unique_ptr<Writable_يادگيري_بفر>>
get_يادگيري_بفر_For_وهڪرو(sys::fs::file_t FD, ثابت Twine &بفر_Name) {
  SmallString<sys::fs::Default_Read_Chunk_سائيز> بفر;
  إذا (Error E = sys::fs::read_Native_File_منزل_EOF(FD, بفر))
    أرجع error_منزل_Error_ڪوڊ(std::move(E));
  أرجع get_Mem_بفر_Copy_Impl(بفر, بفر_Name);
}

ErrorOr<std::unique_ptr<يادگيري_بفر>>
يادگيري_بفر::getFile(ثابت Twine &Filename, منطقي Is_متن,
                      منطقي RequiresNullTerminator, منطقي IsVolatile,
                      std::optional<Align> Alignment) {
  sys::sandbox::violationIfEnabled();

  أرجع getFileAux<يادگيري_بفر>(Filename, /*MapSize=*/-1, /*Offset=*/0,
                                  Is_متن, RequiresNullTerminator, IsVolatile,
                                  Alignment);
}

سانچو <قسم_نالو MB>
ساكن ErrorOr<std::unique_ptr<MB>>
getOpenFileImpl(sys::fs::file_t FD, ثابت Twine &Filename, uint64_t File_سائيز,
                uint64_t Map_سائيز, int64_t وٿي, منطقي RequiresNullTerminator,
                منطقي IsVolatile, std::optional<Align> Alignment);

سانچو <قسم_نالو MB>
ساكن ErrorOr<std::unique_ptr<MB>>
getFileAux(ثابت Twine &Filename, uint64_t Map_سائيز, uint64_t وٿي,
           منطقي Is_متن, منطقي RequiresNullTerminator, منطقي IsVolatile,
           std::optional<Align> Alignment) {
  Expected<sys::fs::file_t> FDOrErr = sys::fs::openNativeFileForRead(
      Filename, Is_متن ? sys::fs::جو_متن_With_CRLF : sys::fs::جو_None);
  إذا (!FDOrErr)
    أرجع error_منزل_Error_ڪوڊ(FDOrErr.takeError());
  sys::fs::file_t FD = *FDOrErr;
  خودڪار Ret = getOpenFileImpl<MB>(FD, Filename, /*FileSize=*/-1, Map_سائيز, وٿي,
                                 RequiresNullTerminator, IsVolatile, Alignment);
  sys::fs::closeFile(FD);
  أرجع Ret;
}

ErrorOr<std::unique_ptr<Writable_يادگيري_بفر>>
Writable_يادگيري_بفر::getFile(ثابت Twine &Filename, منطقي IsVolatile,
                              std::optional<Align> Alignment) {
  sys::sandbox::violationIfEnabled();

  أرجع getFileAux<Writable_يادگيري_بفر>(
      Filename, /*MapSize=*/-1, /*Offset=*/0, /*IsText=*/خطأ,
      /*RequiresNullTerminator=*/خطأ, IsVolatile, Alignment);
}

ErrorOr<std::unique_ptr<Writable_يادگيري_بفر>>
Writable_يادگيري_بفر::get_File_ٽڪرو(ثابت Twine &Filename, uint64_t Map_سائيز,
                                   uint64_t وٿي, منطقي IsVolatile,
                                   std::optional<Align> Alignment) {
  sys::sandbox::violationIfEnabled();

  أرجع getFileAux<Writable_يادگيري_بفر>(
      Filename, Map_سائيز, وٿي, /*IsText=*/خطأ,
      /*RequiresNullTerminator=*/خطأ, IsVolatile, Alignment);
}

std::unique_ptr<Writable_يادگيري_بفر>
Writable_يادگيري_بفر::get_New_Uninit_Mem_بفر(size_t سائيز,
                                            ثابت Twine &بفر_Name,
                                            std::optional<Align> Alignment) {
  استعمال Mem_بفر = يادگيري_بفر_Mem<Writable_يادگيري_بفر>;

  // Use 16-byte alignment if no alignment is specified.
  Align BufAlign = Alignment.قدر_or(Align(16));

  // Allocate space for the MemoryBuffer, the data and the name. It is important
  // that MemoryBuffer and data are aligned so PointerIntPair works with them.
  SmallString<256> NameBuf;
  StringRef NameRef = بفر_Name.منزل_String_Ref(NameBuf);

  size_t StringLen = حجم(Mem_بفر) + حجم(size_t) + NameRef.سائيز_2() + 1;
  size_t RealLen = StringLen + سائيز + 1 + BufAlign.قدر_2();
  إذا (RealLen <= سائيز) // Check for rollover.
    أرجع خالي_اشارو;
  // We use a call to malloc() rather than a call to a non-throwing operator
  // new() because LLVM unconditionally installs an out of memory new handler
  // when exceptions are disabled. This new handler intentionally crashes to
  // aid with debugging, but that makes non-throwing new calls unhelpful.
  // See MemoryBufferMem::operator delete() for the paired call to free(), and
  // llvm::install_out_of_memory_new_handler() for the installation of the
  // custom new handler.
  حرف *Mem = static_cast<حرف *>(std::يادگيري_ڏيو(RealLen));
  إذا (!Mem)
    أرجع خالي_اشارو;

  // The name is stored after the class itself.
  *reinterpret_cast<size_t *>(Mem + حجم(Mem_بفر)) = NameRef.سائيز_2();
  CopyStringRef(Mem + حجم(Mem_بفر) + حجم(size_t), NameRef);

  // The buffer begins after the name and must be aligned.
  حرف *Buf = (حرف *)alignAddr(Mem + StringLen, BufAlign);
  Buf[سائيز] = 0; // Null terminate buffer.

  خودڪار *Ret = جديد (Mem) Mem_بفر(StringRef(Buf, سائيز), سچ);
  أرجع std::unique_ptr<Writable_يادگيري_بفر>(Ret);
}

std::unique_ptr<Writable_يادگيري_بفر>
Writable_يادگيري_بفر::get_New_Mem_بفر(size_t سائيز, ثابت Twine &بفر_Name) {
  خودڪار SB = Writable_يادگيري_بفر::get_New_Uninit_Mem_بفر(سائيز, بفر_Name);
  إذا (!SB)
    أرجع خالي_اشارو;
  memset(SB->get_بفر_شروعات(), 0, سائيز);
  أرجع SB;
}

ساكن منطقي shouldUseMmap(sys::fs::file_t FD,
                          size_t File_سائيز,
                          size_t Map_سائيز,
                          off_t وٿي,
                          منطقي RequiresNullTerminator,
                          صحيح صفحو_سائيز,
                          منطقي IsVolatile) {
#إذا defined(__MVS__)
  // zOS Enhanced ASCII auto convert does not support mmap.
  أرجع خطأ;
#جيڪڏهن_پورو

  // mmap may leave the buffer without null terminator if the file size changed
  // by the time the last page is mapped in, so avoid it if the file size is
  // likely to change.
  إذا (IsVolatile && RequiresNullTerminator)
    أرجع خطأ;

  // We don't use mmap for small files because this can severely fragment our
  // address space.
  إذا (Map_سائيز < 4 * 4096 || Map_سائيز < (بنانشان)صفحو_سائيز)
    أرجع خطأ;

  إذا (!RequiresNullTerminator)
    أرجع سچ;

  // If we don't know the file size, use fstat to find out.  fstat on an open
  // file descriptor is cheaper than stat on a random path.
  // FIXME: this chunk of code is duplicated, but it avoids a fstat when
  // RequiresNullTerminator = false and MapSize != -1.
  إذا (File_سائيز == size_t(-1)) {
    sys::fs::file_حالت حالت;
    إذا (sys::fs::حالت_2(FD, حالت))
      أرجع خطأ;
    File_سائيز = حالت.get_سائيز();
  }

  // If we need a null terminator and the end of the map is inside the file,
  // we cannot use mmap.
  size_t پڇاڙي_2 = وٿي + Map_سائيز;
  أكد(پڇاڙي_2 <= File_سائيز);
  إذا (پڇاڙي_2 != File_سائيز)
    أرجع خطأ;

  // Don't try to map files that are exactly a multiple of the system page size
  // if we need a null terminator.
  إذا ((File_سائيز & (صفحو_سائيز -1)) == 0)
    أرجع خطأ;

#إذا defined(__CYGWIN__)
  // Don't try to map files that are exactly a multiple of the physical page size
  // if we need a null terminator.
  // FIXME: We should reorganize again getPageSize() on Win32.
  إذا ((File_سائيز & (4096 - 1)) == 0)
    أرجع خطأ;
#جيڪڏهن_پورو

  أرجع سچ;
}

ساكن ErrorOr<std::unique_ptr<Write_Through_يادگيري_بفر>>
getReadWriteFile(ثابت Twine &Filename, uint64_t File_سائيز, uint64_t Map_سائيز,
                 uint64_t وٿي) {
  Expected<sys::fs::file_t> FDOrErr = sys::fs::openNativeFileForReadWrite(
      Filename, sys::fs::CD_OpenExisting, sys::fs::جو_None);
  إذا (!FDOrErr)
    أرجع error_منزل_Error_ڪوڊ(FDOrErr.takeError());
  sys::fs::file_t FD = *FDOrErr;

  // Default is to map the full file.
  إذا (Map_سائيز == uint64_t(-1)) {
    // If we don't know the file size, use fstat to find out.  fstat on an open
    // file descriptor is cheaper than stat on a random path.
    إذا (File_سائيز == uint64_t(-1)) {
      sys::fs::file_حالت حالت;
      std::error_ڪوڊ EC = sys::fs::حالت_2(FD, حالت);
      إذا (EC)
        أرجع EC;

      // If this not a file or a block device (e.g. it's a named pipe
      // or character device), we can't mmap it, so error out.
      sys::fs::file_type Type = حالت.type();
      إذا (Type != sys::fs::file_type::regular_file &&
          Type != sys::fs::file_type::بلاڪ_file)
        أرجع make_error_ڪوڊ(errc::invalid_دليل);

      File_سائيز = حالت.get_سائيز();
    }
    Map_سائيز = File_سائيز;
  }

  std::error_ڪوڊ EC;
  std::unique_ptr<Write_Through_يادگيري_بفر> نتيجو(
      جديد (Named_بفر_Alloc(Filename))
          يادگيري_بفر_MMap_File<Write_Through_يادگيري_بفر>(خطأ, FD, Map_سائيز,
                                                         وٿي, EC));
  إذا (EC)
    أرجع EC;
  أرجع std::move(نتيجو);
}

ErrorOr<std::unique_ptr<Write_Through_يادگيري_بفر>>
Write_Through_يادگيري_بفر::getFile(ثابت Twine &Filename, int64_t File_سائيز) {
  sys::sandbox::violationIfEnabled();

  أرجع getReadWriteFile(Filename, File_سائيز, File_سائيز, 0);
}

/// Map a subrange of the specified file as a WritableMemoryBuffer.
ErrorOr<std::unique_ptr<Write_Through_يادگيري_بفر>>
Write_Through_يادگيري_بفر::get_File_ٽڪرو(ثابت Twine &Filename, uint64_t Map_سائيز,
                                       uint64_t وٿي) {
  sys::sandbox::violationIfEnabled();

  أرجع getReadWriteFile(Filename, -1, Map_سائيز, وٿي);
}

سانچو <قسم_نالو MB>
ساكن ErrorOr<std::unique_ptr<MB>>
getOpenFileImpl(sys::fs::file_t FD, ثابت Twine &Filename, uint64_t File_سائيز,
                uint64_t Map_سائيز, int64_t وٿي, منطقي RequiresNullTerminator,
                منطقي IsVolatile, std::optional<Align> Alignment) {
  ساكن صحيح صفحو_سائيز = sys::Process::get_صفحو_سائيز_Estimate();

  // Default is to map the full file.
  إذا (Map_سائيز == uint64_t(-1)) {
    // If we don't know the file size, use fstat to find out.  fstat on an open
    // file descriptor is cheaper than stat on a random path.
    إذا (File_سائيز == uint64_t(-1)) {
      sys::fs::file_حالت حالت;
      std::error_ڪوڊ EC = sys::fs::حالت_2(FD, حالت);
      إذا (EC)
        أرجع EC;

      // If this not a file or a block device (e.g. it's a named pipe
      // or character device), we can't trust the size. Create the memory
      // buffer by copying off the stream.
      sys::fs::file_type Type = حالت.type();
      إذا (Type != sys::fs::file_type::regular_file &&
          Type != sys::fs::file_type::بلاڪ_file)
        أرجع get_يادگيري_بفر_For_وهڪرو(FD, Filename);

      File_سائيز = حالت.get_سائيز();
    }
    Map_سائيز = File_سائيز;
  }

  إذا (shouldUseMmap(FD, File_سائيز, Map_سائيز, وٿي, RequiresNullTerminator,
                    صفحو_سائيز, IsVolatile)) {
    std::error_ڪوڊ EC;
    std::unique_ptr<MB> نتيجو(
        جديد (Named_بفر_Alloc(Filename)) يادگيري_بفر_MMap_File<MB>(
            RequiresNullTerminator, FD, Map_سائيز, وٿي, EC));
    إذا (!EC) {
      // On at least Linux, and possibly on other systems, mmap may return pages
      // from the page cache that are not properly filled with trailing zeroes,
      // if some prior user of the page wrote non-zero bytes. Detect this and
      // don't use mmap in that case.
      إذا (!RequiresNullTerminator || *نتيجو->get_بفر_پڇاڙي() == '\0')
        أرجع std::move(نتيجو);
    }
  }

#جيڪڏهن_وصف_ٿيل __MVS__
  ErrorOr<منطقي> NeedsConversion = needConversion(Filename, FD);
  إذا (std::error_ڪوڊ EC = NeedsConversion.getError())
    أرجع EC;
  // File size may increase due to EBCDIC -> UTF-8 conversion, therefore we
  // cannot trust the file size and we create the memory buffer by copying
  // off the stream.
  // Note: This only works with the assumption of reading a full file (i.e,
  // Offset == 0 and MapSize == FileSize). Reading a file slice does not work.
  إذا (*NeedsConversion && وٿي == 0 && Map_سائيز == File_سائيز)
    أرجع get_يادگيري_بفر_For_وهڪرو(FD, Filename);
#جيڪڏهن_پورو

  خودڪار Buf =
      Writable_يادگيري_بفر::get_New_Uninit_Mem_بفر(Map_سائيز, Filename, Alignment);
  إذا (!Buf) {
    // Failed to create a buffer. The only way it can fail is if
    // new(std::nothrow) returns 0.
    أرجع make_error_ڪوڊ(errc::not_enough_يادگيري);
  }

  // Read until EOF, zero-initialize the rest.
  MutableArrayRef<حرف> منزل_Read = Buf->get_بفر();
  طالما (!منزل_Read.خالي_جايون()) {
    Expected<size_t> Read_بائيٽون =
        sys::fs::read_Native_File_ٽڪرو(FD, منزل_Read, وٿي);
    إذا (!Read_بائيٽون)
      أرجع error_منزل_Error_ڪوڊ(Read_بائيٽون.takeError());
    إذا (*Read_بائيٽون == 0) {
      std::memset(منزل_Read.معلومات(), 0, منزل_Read.سائيز_2());
      اكسر;
    }
    منزل_Read = منزل_Read.drop_front(*Read_بائيٽون);
    وٿي += *Read_بائيٽون;
  }

  أرجع std::move(Buf);
}

ErrorOr<std::unique_ptr<يادگيري_بفر>>
يادگيري_بفر::getOpenFile(sys::fs::file_t FD, ثابت Twine &Filename,
                          uint64_t File_سائيز, منطقي RequiresNullTerminator,
                          منطقي IsVolatile, std::optional<Align> Alignment) {
  sys::sandbox::violationIfEnabled();

  أرجع getOpenFileImpl<يادگيري_بفر>(FD, Filename, File_سائيز, File_سائيز, 0,
                                       RequiresNullTerminator, IsVolatile,
                                       Alignment);
}

ErrorOr<std::unique_ptr<يادگيري_بفر>> يادگيري_بفر::get_Open_File_ٽڪرو(
    sys::fs::file_t FD, ثابت Twine &Filename, uint64_t Map_سائيز, int64_t وٿي,
    منطقي IsVolatile, std::optional<Align> Alignment) {
  أكد(Map_سائيز != uint64_t(-1));

  sys::sandbox::violationIfEnabled();

  أرجع getOpenFileImpl<يادگيري_بفر>(FD, Filename, -1, Map_سائيز, وٿي, خطأ,
                                       IsVolatile, Alignment);
}

ErrorOr<std::unique_ptr<يادگيري_بفر>> يادگيري_بفر::getSTDIN() {
  sys::sandbox::violationIfEnabled();

  // Read in all of the data from stdin, we cannot mmap stdin.
  //
  // FIXME: That isn't necessarily true, we should try to mmap stdin and
  // fallback if it fails.
  sys::Change_Stdin_گهڻائي(sys::fs::جو_متن);

  أرجع get_يادگيري_بفر_For_وهڪرو(sys::fs::getStdinHandle(), "<stdin>");
}

ErrorOr<std::unique_ptr<يادگيري_بفر>>
يادگيري_بفر::get_File_As_وهڪرو(ثابت Twine &Filename) {
  sys::sandbox::violationIfEnabled();

  Expected<sys::fs::file_t> FDOrErr =
      sys::fs::openNativeFileForRead(Filename, sys::fs::جو_None);
  إذا (!FDOrErr)
    أرجع error_منزل_Error_ڪوڊ(FDOrErr.takeError());
  sys::fs::file_t FD = *FDOrErr;
  ErrorOr<std::unique_ptr<يادگيري_بفر>> Ret =
      get_يادگيري_بفر_For_وهڪرو(FD, Filename);
  sys::fs::closeFile(FD);
  أرجع Ret;
}

يادگيري_بفر_Ref يادگيري_بفر::get_Mem_بفر_Ref() ثابت {
  StringRef معلومات_2 = get_بفر();
  StringRef Identifier = get_بفر_Identifier();
  أرجع يادگيري_بفر_Ref(معلومات_2, Identifier);
}

Small_ويڪٽر_يادگيري_بفر::~Small_ويڪٽر_يادگيري_بفر() = افتراضي;
