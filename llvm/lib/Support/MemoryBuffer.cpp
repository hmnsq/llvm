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
#جے_انت

ورتوں نطاق llvm;

//===----------------------------------------------------------------------===//
// MemoryBuffer implementation itself.
//===----------------------------------------------------------------------===//

حافظہ_بفر::~حافظہ_بفر() = افتراضي;

/// init - Initialize this MemoryBuffer as a reference to externally allocated
/// memory, memory that we know is already null terminated.
فراغ حافظہ_بفر::init(ثابت حرف *Buf_شروع, ثابت حرف *Buf_سرا,
                        منطقي RequiresNullTerminator) {
  أكد((!RequiresNullTerminator || Buf_سرا[0] == 0) &&
         "Buffer is not null terminated!");
  بفر_شروع = Buf_شروع;
  بفر_سرا = Buf_سرا;
}

//===----------------------------------------------------------------------===//
// MemoryBufferMem implementation.
//===----------------------------------------------------------------------===//

/// CopyStringRef - Copies contents of a StringRef into a block of memory and
/// null-terminates it.
ساكن فراغ CopyStringRef(حرف *حافظہ_2, StringRef انکڑے_2) {
  إذا (!انکڑے_2.خالی_تھاواں())
    حافظہ_نقل(حافظہ_2, انکڑے_2.انکڑے(), انکڑے_2.ناپ_2());
  حافظہ_2[انکڑے_2.ناپ_2()] = 0; // Null terminate string.
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
      static_cast<حرف *>(std::حافظہ_دیو(N + حجم(size_t) + NameRef.ناپ_2() + 1));
  إذا (!Mem)
    llvm::report_bad_alloc_error("Allocation failed");
  *reinterpret_cast<size_t *>(Mem + N) = NameRef.ناپ_2();
  CopyStringRef(Mem + N + حجم(size_t), NameRef);
  أرجع Mem;
}

نطاق {
/// MemoryBufferMem - Named MemoryBuffer pointing to a block of memory.
سانچا<قسم_ناں MB>
صنف حافظہ_بفر_Mem : عام MB {
عام:
  حافظہ_بفر_Mem(StringRef داخلہ_انکڑے, منطقي RequiresNullTerminator) {
    حافظہ_بفر::init(داخلہ_انکڑے.begin(), داخلہ_انکڑے.سرا(),
                       RequiresNullTerminator);
  }

  /// Disable sized deallocation for MemoryBufferMem, because it has
  /// tail-allocated data.
  فراغ عامل مٹاؤ(فراغ *p) { std::حرر(p); }

  StringRef get_بفر_Identifier() ثابت override {
    // The name is stored after the class itself.
    أرجع StringRef(reinterpret_cast<ثابت حرف *>(هذا + 1) + حجم(size_t),
                     *reinterpret_cast<ثابت size_t *>(هذا + 1));
  }

  حافظہ_بفر::بفر_قسم get_بفر_قسم() ثابت override {
    أرجع حافظہ_بفر::حافظہ_بفر_Malloc;
  }
};
} // namespace

سانچا <قسم_ناں MB>
ساكن ErrorOr<std::unique_ptr<MB>>
getFileAux(ثابت Twine &Filename, uint64_t Map_ناپ, uint64_t وتھ,
           منطقي Is_متن, منطقي RequiresNullTerminator, منطقي IsVolatile,
           std::optional<Align> Alignment);

std::unique_ptr<حافظہ_بفر>
حافظہ_بفر::get_Mem_بفر(StringRef داخلہ_انکڑے, StringRef بفر_Name,
                           منطقي RequiresNullTerminator) {
  خودکار *Ret = جديد (Named_بفر_Alloc(بفر_Name))
      حافظہ_بفر_Mem<حافظہ_بفر>(داخلہ_انکڑے, RequiresNullTerminator);
  أرجع std::unique_ptr<حافظہ_بفر>(Ret);
}

std::unique_ptr<حافظہ_بفر>
حافظہ_بفر::get_Mem_بفر(حافظہ_بفر_Ref Ref, منطقي RequiresNullTerminator) {
  أرجع std::unique_ptr<حافظہ_بفر>(get_Mem_بفر(
      Ref.get_بفر(), Ref.get_بفر_Identifier(), RequiresNullTerminator));
}

ساكن ErrorOr<std::unique_ptr<Writable_حافظہ_بفر>>
get_Mem_بفر_Copy_Impl(StringRef داخلہ_انکڑے, ثابت Twine &بفر_Name) {
  خودکار Buf =
      Writable_حافظہ_بفر::get_New_Uninit_Mem_بفر(داخلہ_انکڑے.ناپ_2(), بفر_Name);
  إذا (!Buf)
    أرجع make_error_کوڈ(errc::not_enough_حافظہ);
  // Calling memcpy with null src/dst is UB, and an empty StringRef is
  // represented with {nullptr, 0}.
  llvm::copy(داخلہ_انکڑے, Buf->get_بفر_شروع());
  أرجع std::move(Buf);
}

std::unique_ptr<حافظہ_بفر>
حافظہ_بفر::get_Mem_بفر_Copy(StringRef داخلہ_انکڑے, ثابت Twine &بفر_Name) {
  خودکار Buf = get_Mem_بفر_Copy_Impl(داخلہ_انکڑے, بفر_Name);
  إذا (Buf)
    أرجع std::move(*Buf);
  أرجع خالی_سوچک;
}

ErrorOr<std::unique_ptr<حافظہ_بفر>>
حافظہ_بفر::getFileOrSTDIN(ثابت Twine &Filename, منطقي Is_متن,
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

ErrorOr<std::unique_ptr<حافظہ_بفر>>
حافظہ_بفر::get_File_ٹکڑا(ثابت Twine &File_رستہ, uint64_t Map_ناپ,
                           uint64_t وتھ, منطقي IsVolatile,
                           std::optional<Align> Alignment) {
  sys::sandbox::violationIfEnabled();

  أرجع getFileAux<حافظہ_بفر>(File_رستہ, Map_ناپ, وتھ, /*IsText=*/خطأ,
                                  /*RequiresNullTerminator=*/خطأ, IsVolatile,
                                  Alignment);
}

//===----------------------------------------------------------------------===//
// MemoryBuffer::getFile implementation.
//===----------------------------------------------------------------------===//

نطاق {

سانچا <قسم_ناں MB>
constexpr sys::fs::mapped_file_region::mapmode Mapmode =
    sys::fs::mapped_file_region::readonly;
سانچا <>
constexpr sys::fs::mapped_file_region::mapmode Mapmode<حافظہ_بفر> =
    sys::fs::mapped_file_region::readonly;
سانچا <>
constexpr sys::fs::mapped_file_region::mapmode Mapmode<Writable_حافظہ_بفر> =
    sys::fs::mapped_file_region::priv;
سانچا <>
constexpr sys::fs::mapped_file_region::mapmode
    Mapmode<Write_Through_حافظہ_بفر> = sys::fs::mapped_file_region::readwrite;

/// Memory maps a file descriptor using sys::fs::mapped_file_region.
///
/// This handles converting the offset into a legal offset on the platform.
سانچا<قسم_ناں MB>
صنف حافظہ_بفر_MMap_File : عام MB {
  sys::fs::mapped_file_region MFR;

  ساكن uint64_t get_Legal_Map_وتھ(uint64_t وتھ) {
    أرجع وتھ & ~(sys::fs::mapped_file_region::alignment() - 1);
  }

  ساكن uint64_t get_Legal_Map_ناپ(uint64_t Len, uint64_t وتھ) {
    أرجع Len + (وتھ - get_Legal_Map_وتھ(وتھ));
  }

  ثابت حرف *get_شروع(uint64_t Len, uint64_t وتھ) {
    أرجع MFR.const_انکڑے() + (وتھ - get_Legal_Map_وتھ(وتھ));
  }

عام:
  حافظہ_بفر_MMap_File(منطقي RequiresNullTerminator, sys::fs::file_t FD, uint64_t Len,
                       uint64_t وتھ, std::error_کوڈ &EC)
      : MFR(FD, Mapmode<MB>, get_Legal_Map_ناپ(Len, وتھ),
            get_Legal_Map_وتھ(وتھ), EC) {
    إذا (!EC) {
      ثابت حرف *شروع = get_شروع(Len, وتھ);
      حافظہ_بفر::init(شروع, شروع + Len, RequiresNullTerminator);
    }
  }

  /// Disable sized deallocation for MemoryBufferMMapFile, because it has
  /// tail-allocated data.
  فراغ عامل مٹاؤ(فراغ *p) { std::حرر(p); }

  StringRef get_بفر_Identifier() ثابت override {
    // The name is stored after the class itself.
    أرجع StringRef(reinterpret_cast<ثابت حرف *>(هذا + 1) + حجم(size_t),
                     *reinterpret_cast<ثابت size_t *>(هذا + 1));
  }

  حافظہ_بفر::بفر_قسم get_بفر_قسم() ثابت override {
    أرجع حافظہ_بفر::حافظہ_بفر_MMap;
  }

  فراغ dontNeedIfMmap() override { MFR.dontNeed(); }
  فراغ willNeedIfMmap() override { MFR.willNeed(); }
  فراغ randomAccessIfMmap() override { MFR.randomAccess(); }
};
} // namespace

ساكن ErrorOr<std::unique_ptr<Writable_حافظہ_بفر>>
get_حافظہ_بفر_For_دھارا(sys::fs::file_t FD, ثابت Twine &بفر_Name) {
  SmallString<sys::fs::Default_Read_Chunk_ناپ> بفر;
  إذا (Error E = sys::fs::read_Native_File_منزل_EOF(FD, بفر))
    أرجع error_منزل_Error_کوڈ(std::move(E));
  أرجع get_Mem_بفر_Copy_Impl(بفر, بفر_Name);
}

ErrorOr<std::unique_ptr<حافظہ_بفر>>
حافظہ_بفر::getFile(ثابت Twine &Filename, منطقي Is_متن,
                      منطقي RequiresNullTerminator, منطقي IsVolatile,
                      std::optional<Align> Alignment) {
  sys::sandbox::violationIfEnabled();

  أرجع getFileAux<حافظہ_بفر>(Filename, /*MapSize=*/-1, /*Offset=*/0,
                                  Is_متن, RequiresNullTerminator, IsVolatile,
                                  Alignment);
}

سانچا <قسم_ناں MB>
ساكن ErrorOr<std::unique_ptr<MB>>
getOpenFileImpl(sys::fs::file_t FD, ثابت Twine &Filename, uint64_t File_ناپ,
                uint64_t Map_ناپ, int64_t وتھ, منطقي RequiresNullTerminator,
                منطقي IsVolatile, std::optional<Align> Alignment);

سانچا <قسم_ناں MB>
ساكن ErrorOr<std::unique_ptr<MB>>
getFileAux(ثابت Twine &Filename, uint64_t Map_ناپ, uint64_t وتھ,
           منطقي Is_متن, منطقي RequiresNullTerminator, منطقي IsVolatile,
           std::optional<Align> Alignment) {
  Expected<sys::fs::file_t> FDOrErr = sys::fs::openNativeFileForRead(
      Filename, Is_متن ? sys::fs::دا_متن_With_CRLF : sys::fs::دا_None);
  إذا (!FDOrErr)
    أرجع error_منزل_Error_کوڈ(FDOrErr.takeError());
  sys::fs::file_t FD = *FDOrErr;
  خودکار Ret = getOpenFileImpl<MB>(FD, Filename, /*FileSize=*/-1, Map_ناپ, وتھ,
                                 RequiresNullTerminator, IsVolatile, Alignment);
  sys::fs::closeFile(FD);
  أرجع Ret;
}

ErrorOr<std::unique_ptr<Writable_حافظہ_بفر>>
Writable_حافظہ_بفر::getFile(ثابت Twine &Filename, منطقي IsVolatile,
                              std::optional<Align> Alignment) {
  sys::sandbox::violationIfEnabled();

  أرجع getFileAux<Writable_حافظہ_بفر>(
      Filename, /*MapSize=*/-1, /*Offset=*/0, /*IsText=*/خطأ,
      /*RequiresNullTerminator=*/خطأ, IsVolatile, Alignment);
}

ErrorOr<std::unique_ptr<Writable_حافظہ_بفر>>
Writable_حافظہ_بفر::get_File_ٹکڑا(ثابت Twine &Filename, uint64_t Map_ناپ,
                                   uint64_t وتھ, منطقي IsVolatile,
                                   std::optional<Align> Alignment) {
  sys::sandbox::violationIfEnabled();

  أرجع getFileAux<Writable_حافظہ_بفر>(
      Filename, Map_ناپ, وتھ, /*IsText=*/خطأ,
      /*RequiresNullTerminator=*/خطأ, IsVolatile, Alignment);
}

std::unique_ptr<Writable_حافظہ_بفر>
Writable_حافظہ_بفر::get_New_Uninit_Mem_بفر(size_t ناپ,
                                            ثابت Twine &بفر_Name,
                                            std::optional<Align> Alignment) {
  ورتوں Mem_بفر = حافظہ_بفر_Mem<Writable_حافظہ_بفر>;

  // Use 16-byte alignment if no alignment is specified.
  Align BufAlign = Alignment.قدر_or(Align(16));

  // Allocate space for the MemoryBuffer, the data and the name. It is important
  // that MemoryBuffer and data are aligned so PointerIntPair works with them.
  SmallString<256> NameBuf;
  StringRef NameRef = بفر_Name.منزل_String_Ref(NameBuf);

  size_t StringLen = حجم(Mem_بفر) + حجم(size_t) + NameRef.ناپ_2() + 1;
  size_t RealLen = StringLen + ناپ + 1 + BufAlign.قدر_2();
  إذا (RealLen <= ناپ) // Check for rollover.
    أرجع خالی_سوچک;
  // We use a call to malloc() rather than a call to a non-throwing operator
  // new() because LLVM unconditionally installs an out of memory new handler
  // when exceptions are disabled. This new handler intentionally crashes to
  // aid with debugging, but that makes non-throwing new calls unhelpful.
  // See MemoryBufferMem::operator delete() for the paired call to free(), and
  // llvm::install_out_of_memory_new_handler() for the installation of the
  // custom new handler.
  حرف *Mem = static_cast<حرف *>(std::حافظہ_دیو(RealLen));
  إذا (!Mem)
    أرجع خالی_سوچک;

  // The name is stored after the class itself.
  *reinterpret_cast<size_t *>(Mem + حجم(Mem_بفر)) = NameRef.ناپ_2();
  CopyStringRef(Mem + حجم(Mem_بفر) + حجم(size_t), NameRef);

  // The buffer begins after the name and must be aligned.
  حرف *Buf = (حرف *)alignAddr(Mem + StringLen, BufAlign);
  Buf[ناپ] = 0; // Null terminate buffer.

  خودکار *Ret = جديد (Mem) Mem_بفر(StringRef(Buf, ناپ), سچ);
  أرجع std::unique_ptr<Writable_حافظہ_بفر>(Ret);
}

std::unique_ptr<Writable_حافظہ_بفر>
Writable_حافظہ_بفر::get_New_Mem_بفر(size_t ناپ, ثابت Twine &بفر_Name) {
  خودکار SB = Writable_حافظہ_بفر::get_New_Uninit_Mem_بفر(ناپ, بفر_Name);
  إذا (!SB)
    أرجع خالی_سوچک;
  memset(SB->get_بفر_شروع(), 0, ناپ);
  أرجع SB;
}

ساكن منطقي shouldUseMmap(sys::fs::file_t FD,
                          size_t File_ناپ,
                          size_t Map_ناپ,
                          off_t وتھ,
                          منطقي RequiresNullTerminator,
                          صحيح ورقہ_ناپ,
                          منطقي IsVolatile) {
#إذا defined(__MVS__)
  // zOS Enhanced ASCII auto convert does not support mmap.
  أرجع خطأ;
#جے_انت

  // mmap may leave the buffer without null terminator if the file size changed
  // by the time the last page is mapped in, so avoid it if the file size is
  // likely to change.
  إذا (IsVolatile && RequiresNullTerminator)
    أرجع خطأ;

  // We don't use mmap for small files because this can severely fragment our
  // address space.
  إذا (Map_ناپ < 4 * 4096 || Map_ناپ < (بنانشانی)ورقہ_ناپ)
    أرجع خطأ;

  إذا (!RequiresNullTerminator)
    أرجع سچ;

  // If we don't know the file size, use fstat to find out.  fstat on an open
  // file descriptor is cheaper than stat on a random path.
  // FIXME: this chunk of code is duplicated, but it avoids a fstat when
  // RequiresNullTerminator = false and MapSize != -1.
  إذا (File_ناپ == size_t(-1)) {
    sys::fs::file_حالت حالت;
    إذا (sys::fs::حالت_2(FD, حالت))
      أرجع خطأ;
    File_ناپ = حالت.get_ناپ();
  }

  // If we need a null terminator and the end of the map is inside the file,
  // we cannot use mmap.
  size_t سرا_2 = وتھ + Map_ناپ;
  أكد(سرا_2 <= File_ناپ);
  إذا (سرا_2 != File_ناپ)
    أرجع خطأ;

  // Don't try to map files that are exactly a multiple of the system page size
  // if we need a null terminator.
  إذا ((File_ناپ & (ورقہ_ناپ -1)) == 0)
    أرجع خطأ;

#إذا defined(__CYGWIN__)
  // Don't try to map files that are exactly a multiple of the physical page size
  // if we need a null terminator.
  // FIXME: We should reorganize again getPageSize() on Win32.
  إذا ((File_ناپ & (4096 - 1)) == 0)
    أرجع خطأ;
#جے_انت

  أرجع سچ;
}

ساكن ErrorOr<std::unique_ptr<Write_Through_حافظہ_بفر>>
getReadWriteFile(ثابت Twine &Filename, uint64_t File_ناپ, uint64_t Map_ناپ,
                 uint64_t وتھ) {
  Expected<sys::fs::file_t> FDOrErr = sys::fs::openNativeFileForReadWrite(
      Filename, sys::fs::CD_OpenExisting, sys::fs::دا_None);
  إذا (!FDOrErr)
    أرجع error_منزل_Error_کوڈ(FDOrErr.takeError());
  sys::fs::file_t FD = *FDOrErr;

  // Default is to map the full file.
  إذا (Map_ناپ == uint64_t(-1)) {
    // If we don't know the file size, use fstat to find out.  fstat on an open
    // file descriptor is cheaper than stat on a random path.
    إذا (File_ناپ == uint64_t(-1)) {
      sys::fs::file_حالت حالت;
      std::error_کوڈ EC = sys::fs::حالت_2(FD, حالت);
      إذا (EC)
        أرجع EC;

      // If this not a file or a block device (e.g. it's a named pipe
      // or character device), we can't mmap it, so error out.
      sys::fs::file_type Type = حالت.type();
      إذا (Type != sys::fs::file_type::regular_file &&
          Type != sys::fs::file_type::بلاک_file)
        أرجع make_error_کوڈ(errc::invalid_دلیل);

      File_ناپ = حالت.get_ناپ();
    }
    Map_ناپ = File_ناپ;
  }

  std::error_کوڈ EC;
  std::unique_ptr<Write_Through_حافظہ_بفر> نتیجہ(
      جديد (Named_بفر_Alloc(Filename))
          حافظہ_بفر_MMap_File<Write_Through_حافظہ_بفر>(خطأ, FD, Map_ناپ,
                                                         وتھ, EC));
  إذا (EC)
    أرجع EC;
  أرجع std::move(نتیجہ);
}

ErrorOr<std::unique_ptr<Write_Through_حافظہ_بفر>>
Write_Through_حافظہ_بفر::getFile(ثابت Twine &Filename, int64_t File_ناپ) {
  sys::sandbox::violationIfEnabled();

  أرجع getReadWriteFile(Filename, File_ناپ, File_ناپ, 0);
}

/// Map a subrange of the specified file as a WritableMemoryBuffer.
ErrorOr<std::unique_ptr<Write_Through_حافظہ_بفر>>
Write_Through_حافظہ_بفر::get_File_ٹکڑا(ثابت Twine &Filename, uint64_t Map_ناپ,
                                       uint64_t وتھ) {
  sys::sandbox::violationIfEnabled();

  أرجع getReadWriteFile(Filename, -1, Map_ناپ, وتھ);
}

سانچا <قسم_ناں MB>
ساكن ErrorOr<std::unique_ptr<MB>>
getOpenFileImpl(sys::fs::file_t FD, ثابت Twine &Filename, uint64_t File_ناپ,
                uint64_t Map_ناپ, int64_t وتھ, منطقي RequiresNullTerminator,
                منطقي IsVolatile, std::optional<Align> Alignment) {
  ساكن صحيح ورقہ_ناپ = sys::Process::get_ورقہ_ناپ_Estimate();

  // Default is to map the full file.
  إذا (Map_ناپ == uint64_t(-1)) {
    // If we don't know the file size, use fstat to find out.  fstat on an open
    // file descriptor is cheaper than stat on a random path.
    إذا (File_ناپ == uint64_t(-1)) {
      sys::fs::file_حالت حالت;
      std::error_کوڈ EC = sys::fs::حالت_2(FD, حالت);
      إذا (EC)
        أرجع EC;

      // If this not a file or a block device (e.g. it's a named pipe
      // or character device), we can't trust the size. Create the memory
      // buffer by copying off the stream.
      sys::fs::file_type Type = حالت.type();
      إذا (Type != sys::fs::file_type::regular_file &&
          Type != sys::fs::file_type::بلاک_file)
        أرجع get_حافظہ_بفر_For_دھارا(FD, Filename);

      File_ناپ = حالت.get_ناپ();
    }
    Map_ناپ = File_ناپ;
  }

  إذا (shouldUseMmap(FD, File_ناپ, Map_ناپ, وتھ, RequiresNullTerminator,
                    ورقہ_ناپ, IsVolatile)) {
    std::error_کوڈ EC;
    std::unique_ptr<MB> نتیجہ(
        جديد (Named_بفر_Alloc(Filename)) حافظہ_بفر_MMap_File<MB>(
            RequiresNullTerminator, FD, Map_ناپ, وتھ, EC));
    إذا (!EC) {
      // On at least Linux, and possibly on other systems, mmap may return pages
      // from the page cache that are not properly filled with trailing zeroes,
      // if some prior user of the page wrote non-zero bytes. Detect this and
      // don't use mmap in that case.
      إذا (!RequiresNullTerminator || *نتیجہ->get_بفر_سرا() == '\0')
        أرجع std::move(نتیجہ);
    }
  }

#جے_تعریف_شدہ __MVS__
  ErrorOr<منطقي> NeedsConversion = needConversion(Filename, FD);
  إذا (std::error_کوڈ EC = NeedsConversion.getError())
    أرجع EC;
  // File size may increase due to EBCDIC -> UTF-8 conversion, therefore we
  // cannot trust the file size and we create the memory buffer by copying
  // off the stream.
  // Note: This only works with the assumption of reading a full file (i.e,
  // Offset == 0 and MapSize == FileSize). Reading a file slice does not work.
  إذا (*NeedsConversion && وتھ == 0 && Map_ناپ == File_ناپ)
    أرجع get_حافظہ_بفر_For_دھارا(FD, Filename);
#جے_انت

  خودکار Buf =
      Writable_حافظہ_بفر::get_New_Uninit_Mem_بفر(Map_ناپ, Filename, Alignment);
  إذا (!Buf) {
    // Failed to create a buffer. The only way it can fail is if
    // new(std::nothrow) returns 0.
    أرجع make_error_کوڈ(errc::not_enough_حافظہ);
  }

  // Read until EOF, zero-initialize the rest.
  MutableArrayRef<حرف> منزل_Read = Buf->get_بفر();
  طالما (!منزل_Read.خالی_تھاواں()) {
    Expected<size_t> Read_بائٹاں =
        sys::fs::read_Native_File_ٹکڑا(FD, منزل_Read, وتھ);
    إذا (!Read_بائٹاں)
      أرجع error_منزل_Error_کوڈ(Read_بائٹاں.takeError());
    إذا (*Read_بائٹاں == 0) {
      std::memset(منزل_Read.انکڑے(), 0, منزل_Read.ناپ_2());
      اكسر;
    }
    منزل_Read = منزل_Read.drop_front(*Read_بائٹاں);
    وتھ += *Read_بائٹاں;
  }

  أرجع std::move(Buf);
}

ErrorOr<std::unique_ptr<حافظہ_بفر>>
حافظہ_بفر::getOpenFile(sys::fs::file_t FD, ثابت Twine &Filename,
                          uint64_t File_ناپ, منطقي RequiresNullTerminator,
                          منطقي IsVolatile, std::optional<Align> Alignment) {
  sys::sandbox::violationIfEnabled();

  أرجع getOpenFileImpl<حافظہ_بفر>(FD, Filename, File_ناپ, File_ناپ, 0,
                                       RequiresNullTerminator, IsVolatile,
                                       Alignment);
}

ErrorOr<std::unique_ptr<حافظہ_بفر>> حافظہ_بفر::get_Open_File_ٹکڑا(
    sys::fs::file_t FD, ثابت Twine &Filename, uint64_t Map_ناپ, int64_t وتھ,
    منطقي IsVolatile, std::optional<Align> Alignment) {
  أكد(Map_ناپ != uint64_t(-1));

  sys::sandbox::violationIfEnabled();

  أرجع getOpenFileImpl<حافظہ_بفر>(FD, Filename, -1, Map_ناپ, وتھ, خطأ,
                                       IsVolatile, Alignment);
}

ErrorOr<std::unique_ptr<حافظہ_بفر>> حافظہ_بفر::getSTDIN() {
  sys::sandbox::violationIfEnabled();

  // Read in all of the data from stdin, we cannot mmap stdin.
  //
  // FIXME: That isn't necessarily true, we should try to mmap stdin and
  // fallback if it fails.
  sys::Change_Stdin_منوال(sys::fs::دا_متن);

  أرجع get_حافظہ_بفر_For_دھارا(sys::fs::getStdinHandle(), "<stdin>");
}

ErrorOr<std::unique_ptr<حافظہ_بفر>>
حافظہ_بفر::get_File_As_دھارا(ثابت Twine &Filename) {
  sys::sandbox::violationIfEnabled();

  Expected<sys::fs::file_t> FDOrErr =
      sys::fs::openNativeFileForRead(Filename, sys::fs::دا_None);
  إذا (!FDOrErr)
    أرجع error_منزل_Error_کوڈ(FDOrErr.takeError());
  sys::fs::file_t FD = *FDOrErr;
  ErrorOr<std::unique_ptr<حافظہ_بفر>> Ret =
      get_حافظہ_بفر_For_دھارا(FD, Filename);
  sys::fs::closeFile(FD);
  أرجع Ret;
}

حافظہ_بفر_Ref حافظہ_بفر::get_Mem_بفر_Ref() ثابت {
  StringRef انکڑے_2 = get_بفر();
  StringRef Identifier = get_بفر_Identifier();
  أرجع حافظہ_بفر_Ref(انکڑے_2, Identifier);
}

Small_سمتیہ_حافظہ_بفر::~Small_سمتیہ_حافظہ_بفر() = افتراضي;
