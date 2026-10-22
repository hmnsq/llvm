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
#د_که_پای

کارول نطاق llvm;

//===----------------------------------------------------------------------===//
// MemoryBuffer implementation itself.
//===----------------------------------------------------------------------===//

حافظه_بفر::~حافظه_بفر() = افتراضي;

/// init - Initialize this MemoryBuffer as a reference to externally allocated
/// memory, memory that we know is already null terminated.
فراغ حافظه_بفر::init(ثابت حرف *Buf_پیل, ثابت حرف *Buf_پای_ټکی,
                        منطقي RequiresNullTerminator) {
  أكد((!RequiresNullTerminator || Buf_پای_ټکی[0] == 0) &&
         "Buffer is not null terminated!");
  بفر_پیل = Buf_پیل;
  بفر_پای_ټکی = Buf_پای_ټکی;
}

//===----------------------------------------------------------------------===//
// MemoryBufferMem implementation.
//===----------------------------------------------------------------------===//

/// CopyStringRef - Copies contents of a StringRef into a block of memory and
/// null-terminates it.
ساكن فراغ CopyStringRef(حرف *حافظه_2, StringRef معلومات_2) {
  إذا (!معلومات_2.تشه())
    حافظه_نقل(حافظه_2, معلومات_2.معلومات(), معلومات_2.اندازه_2());
  حافظه_2[معلومات_2.اندازه_2()] = 0; // Null terminate string.
}

نطاق {
بنية Named_بفر_Alloc {
  ثابت Twine &Name;
  Named_بفر_Alloc(ثابت Twine &Name) : Name(Name) {}
};
} // namespace

فراغ *چلوونکی جديد(size_t N, ثابت Named_بفر_Alloc &Alloc) {
  SmallString<256> NameBuf;
  StringRef NameRef = Alloc.Name.منزل_String_Ref(NameBuf);

  // We use malloc() and manually handle it returning null instead of calling
  // operator new because we need all uses of NamedBufferAlloc to be
  // deallocated with a call to free() due to needing to use malloc() in
  // WritableMemoryBuffer::getNewUninitMemBuffer() to work around the out-of-
  // memory handler installed by default in LLVM. See operator delete() member
  // functions within this file for the paired call to free().
  حرف *Mem =
      static_cast<حرف *>(std::حافظه_ورکړه(N + حجم(size_t) + NameRef.اندازه_2() + 1));
  إذا (!Mem)
    llvm::report_bad_alloc_error("Allocation failed");
  *reinterpret_cast<size_t *>(Mem + N) = NameRef.اندازه_2();
  CopyStringRef(Mem + N + حجم(size_t), NameRef);
  أرجع Mem;
}

نطاق {
/// MemoryBufferMem - Named MemoryBuffer pointing to a block of memory.
کينډۍ<ډول_نوم MB>
صنف حافظه_بفر_Mem : عام MB {
عام:
  حافظه_بفر_Mem(StringRef ننوت_معلومات, منطقي RequiresNullTerminator) {
    حافظه_بفر::init(ننوت_معلومات.begin(), ننوت_معلومات.پای_ټکی(),
                       RequiresNullTerminator);
  }

  /// Disable sized deallocation for MemoryBufferMem, because it has
  /// tail-allocated data.
  فراغ چلوونکی ړنګول(فراغ *p) { std::حرر(p); }

  StringRef get_بفر_Identifier() ثابت override {
    // The name is stored after the class itself.
    أرجع StringRef(reinterpret_cast<ثابت حرف *>(هذا + 1) + حجم(size_t),
                     *reinterpret_cast<ثابت size_t *>(هذا + 1));
  }

  حافظه_بفر::بفر_ډول get_بفر_ډول() ثابت override {
    أرجع حافظه_بفر::حافظه_بفر_Malloc;
  }
};
} // namespace

کينډۍ <ډول_نوم MB>
ساكن ErrorOr<std::unique_ptr<MB>>
getFileAux(ثابت Twine &Filename, uint64_t Map_اندازه, uint64_t دننه_واټن,
           منطقي Is_متن, منطقي RequiresNullTerminator, منطقي IsVolatile,
           std::optional<Align> Alignment);

std::unique_ptr<حافظه_بفر>
حافظه_بفر::get_Mem_بفر(StringRef ننوت_معلومات, StringRef بفر_Name,
                           منطقي RequiresNullTerminator) {
  خپلکاری *Ret = جديد (Named_بفر_Alloc(بفر_Name))
      حافظه_بفر_Mem<حافظه_بفر>(ننوت_معلومات, RequiresNullTerminator);
  أرجع std::unique_ptr<حافظه_بفر>(Ret);
}

std::unique_ptr<حافظه_بفر>
حافظه_بفر::get_Mem_بفر(حافظه_بفر_Ref Ref, منطقي RequiresNullTerminator) {
  أرجع std::unique_ptr<حافظه_بفر>(get_Mem_بفر(
      Ref.get_بفر(), Ref.get_بفر_Identifier(), RequiresNullTerminator));
}

ساكن ErrorOr<std::unique_ptr<Writable_حافظه_بفر>>
get_Mem_بفر_Copy_Impl(StringRef ننوت_معلومات, ثابت Twine &بفر_Name) {
  خپلکاری Buf =
      Writable_حافظه_بفر::get_New_Uninit_Mem_بفر(ننوت_معلومات.اندازه_2(), بفر_Name);
  إذا (!Buf)
    أرجع make_error_کوډ(errc::not_enough_حافظه);
  // Calling memcpy with null src/dst is UB, and an empty StringRef is
  // represented with {nullptr, 0}.
  llvm::copy(ننوت_معلومات, Buf->get_بفر_پیل());
  أرجع std::move(Buf);
}

std::unique_ptr<حافظه_بفر>
حافظه_بفر::get_Mem_بفر_Copy(StringRef ننوت_معلومات, ثابت Twine &بفر_Name) {
  خپلکاری Buf = get_Mem_بفر_Copy_Impl(ننوت_معلومات, بفر_Name);
  إذا (Buf)
    أرجع std::move(*Buf);
  أرجع تش_اشاره;
}

ErrorOr<std::unique_ptr<حافظه_بفر>>
حافظه_بفر::getFileOrSTDIN(ثابت Twine &Filename, منطقي Is_متن,
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

ErrorOr<std::unique_ptr<حافظه_بفر>>
حافظه_بفر::get_File_ټوټه(ثابت Twine &File_لار, uint64_t Map_اندازه,
                           uint64_t دننه_واټن, منطقي IsVolatile,
                           std::optional<Align> Alignment) {
  sys::sandbox::violationIfEnabled();

  أرجع getFileAux<حافظه_بفر>(File_لار, Map_اندازه, دننه_واټن, /*IsText=*/خطأ,
                                  /*RequiresNullTerminator=*/خطأ, IsVolatile,
                                  Alignment);
}

//===----------------------------------------------------------------------===//
// MemoryBuffer::getFile implementation.
//===----------------------------------------------------------------------===//

نطاق {

کينډۍ <ډول_نوم MB>
constexpr sys::fs::mapped_file_region::mapmode Mapmode =
    sys::fs::mapped_file_region::readonly;
کينډۍ <>
constexpr sys::fs::mapped_file_region::mapmode Mapmode<حافظه_بفر> =
    sys::fs::mapped_file_region::readonly;
کينډۍ <>
constexpr sys::fs::mapped_file_region::mapmode Mapmode<Writable_حافظه_بفر> =
    sys::fs::mapped_file_region::priv;
کينډۍ <>
constexpr sys::fs::mapped_file_region::mapmode
    Mapmode<Write_Through_حافظه_بفر> = sys::fs::mapped_file_region::readwrite;

/// Memory maps a file descriptor using sys::fs::mapped_file_region.
///
/// This handles converting the offset into a legal offset on the platform.
کينډۍ<ډول_نوم MB>
صنف حافظه_بفر_MMap_File : عام MB {
  sys::fs::mapped_file_region MFR;

  ساكن uint64_t get_Legal_Map_دننه_واټن(uint64_t دننه_واټن) {
    أرجع دننه_واټن & ~(sys::fs::mapped_file_region::alignment() - 1);
  }

  ساكن uint64_t get_Legal_Map_اندازه(uint64_t Len, uint64_t دننه_واټن) {
    أرجع Len + (دننه_واټن - get_Legal_Map_دننه_واټن(دننه_واټن));
  }

  ثابت حرف *get_پیل(uint64_t Len, uint64_t دننه_واټن) {
    أرجع MFR.const_معلومات() + (دننه_واټن - get_Legal_Map_دننه_واټن(دننه_واټن));
  }

عام:
  حافظه_بفر_MMap_File(منطقي RequiresNullTerminator, sys::fs::file_t FD, uint64_t Len,
                       uint64_t دننه_واټن, std::error_کوډ &EC)
      : MFR(FD, Mapmode<MB>, get_Legal_Map_اندازه(Len, دننه_واټن),
            get_Legal_Map_دننه_واټن(دننه_واټن), EC) {
    إذا (!EC) {
      ثابت حرف *پیل = get_پیل(Len, دننه_واټن);
      حافظه_بفر::init(پیل, پیل + Len, RequiresNullTerminator);
    }
  }

  /// Disable sized deallocation for MemoryBufferMMapFile, because it has
  /// tail-allocated data.
  فراغ چلوونکی ړنګول(فراغ *p) { std::حرر(p); }

  StringRef get_بفر_Identifier() ثابت override {
    // The name is stored after the class itself.
    أرجع StringRef(reinterpret_cast<ثابت حرف *>(هذا + 1) + حجم(size_t),
                     *reinterpret_cast<ثابت size_t *>(هذا + 1));
  }

  حافظه_بفر::بفر_ډول get_بفر_ډول() ثابت override {
    أرجع حافظه_بفر::حافظه_بفر_MMap;
  }

  فراغ dontNeedIfMmap() override { MFR.dontNeed(); }
  فراغ willNeedIfMmap() override { MFR.willNeed(); }
  فراغ randomAccessIfMmap() override { MFR.randomAccess(); }
};
} // namespace

ساكن ErrorOr<std::unique_ptr<Writable_حافظه_بفر>>
get_حافظه_بفر_For_بهیر(sys::fs::file_t FD, ثابت Twine &بفر_Name) {
  SmallString<sys::fs::Default_Read_Chunk_اندازه> بفر;
  إذا (Error E = sys::fs::read_Native_File_منزل_EOF(FD, بفر))
    أرجع error_منزل_Error_کوډ(std::move(E));
  أرجع get_Mem_بفر_Copy_Impl(بفر, بفر_Name);
}

ErrorOr<std::unique_ptr<حافظه_بفر>>
حافظه_بفر::getFile(ثابت Twine &Filename, منطقي Is_متن,
                      منطقي RequiresNullTerminator, منطقي IsVolatile,
                      std::optional<Align> Alignment) {
  sys::sandbox::violationIfEnabled();

  أرجع getFileAux<حافظه_بفر>(Filename, /*MapSize=*/-1, /*Offset=*/0,
                                  Is_متن, RequiresNullTerminator, IsVolatile,
                                  Alignment);
}

کينډۍ <ډول_نوم MB>
ساكن ErrorOr<std::unique_ptr<MB>>
getOpenFileImpl(sys::fs::file_t FD, ثابت Twine &Filename, uint64_t File_اندازه,
                uint64_t Map_اندازه, int64_t دننه_واټن, منطقي RequiresNullTerminator,
                منطقي IsVolatile, std::optional<Align> Alignment);

کينډۍ <ډول_نوم MB>
ساكن ErrorOr<std::unique_ptr<MB>>
getFileAux(ثابت Twine &Filename, uint64_t Map_اندازه, uint64_t دننه_واټن,
           منطقي Is_متن, منطقي RequiresNullTerminator, منطقي IsVolatile,
           std::optional<Align> Alignment) {
  Expected<sys::fs::file_t> FDOrErr = sys::fs::openNativeFileForRead(
      Filename, Is_متن ? sys::fs::د_متن_With_CRLF : sys::fs::د_None);
  إذا (!FDOrErr)
    أرجع error_منزل_Error_کوډ(FDOrErr.takeError());
  sys::fs::file_t FD = *FDOrErr;
  خپلکاری Ret = getOpenFileImpl<MB>(FD, Filename, /*FileSize=*/-1, Map_اندازه, دننه_واټن,
                                 RequiresNullTerminator, IsVolatile, Alignment);
  sys::fs::closeFile(FD);
  أرجع Ret;
}

ErrorOr<std::unique_ptr<Writable_حافظه_بفر>>
Writable_حافظه_بفر::getFile(ثابت Twine &Filename, منطقي IsVolatile,
                              std::optional<Align> Alignment) {
  sys::sandbox::violationIfEnabled();

  أرجع getFileAux<Writable_حافظه_بفر>(
      Filename, /*MapSize=*/-1, /*Offset=*/0, /*IsText=*/خطأ,
      /*RequiresNullTerminator=*/خطأ, IsVolatile, Alignment);
}

ErrorOr<std::unique_ptr<Writable_حافظه_بفر>>
Writable_حافظه_بفر::get_File_ټوټه(ثابت Twine &Filename, uint64_t Map_اندازه,
                                   uint64_t دننه_واټن, منطقي IsVolatile,
                                   std::optional<Align> Alignment) {
  sys::sandbox::violationIfEnabled();

  أرجع getFileAux<Writable_حافظه_بفر>(
      Filename, Map_اندازه, دننه_واټن, /*IsText=*/خطأ,
      /*RequiresNullTerminator=*/خطأ, IsVolatile, Alignment);
}

std::unique_ptr<Writable_حافظه_بفر>
Writable_حافظه_بفر::get_New_Uninit_Mem_بفر(size_t اندازه,
                                            ثابت Twine &بفر_Name,
                                            std::optional<Align> Alignment) {
  کارول Mem_بفر = حافظه_بفر_Mem<Writable_حافظه_بفر>;

  // Use 16-byte alignment if no alignment is specified.
  Align BufAlign = Alignment.ارزښت_or(Align(16));

  // Allocate space for the MemoryBuffer, the data and the name. It is important
  // that MemoryBuffer and data are aligned so PointerIntPair works with them.
  SmallString<256> NameBuf;
  StringRef NameRef = بفر_Name.منزل_String_Ref(NameBuf);

  size_t StringLen = حجم(Mem_بفر) + حجم(size_t) + NameRef.اندازه_2() + 1;
  size_t RealLen = StringLen + اندازه + 1 + BufAlign.ارزښت_2();
  إذا (RealLen <= اندازه) // Check for rollover.
    أرجع تش_اشاره;
  // We use a call to malloc() rather than a call to a non-throwing operator
  // new() because LLVM unconditionally installs an out of memory new handler
  // when exceptions are disabled. This new handler intentionally crashes to
  // aid with debugging, but that makes non-throwing new calls unhelpful.
  // See MemoryBufferMem::operator delete() for the paired call to free(), and
  // llvm::install_out_of_memory_new_handler() for the installation of the
  // custom new handler.
  حرف *Mem = static_cast<حرف *>(std::حافظه_ورکړه(RealLen));
  إذا (!Mem)
    أرجع تش_اشاره;

  // The name is stored after the class itself.
  *reinterpret_cast<size_t *>(Mem + حجم(Mem_بفر)) = NameRef.اندازه_2();
  CopyStringRef(Mem + حجم(Mem_بفر) + حجم(size_t), NameRef);

  // The buffer begins after the name and must be aligned.
  حرف *Buf = (حرف *)alignAddr(Mem + StringLen, BufAlign);
  Buf[اندازه] = 0; // Null terminate buffer.

  خپلکاری *Ret = جديد (Mem) Mem_بفر(StringRef(Buf, اندازه), ریښتیا);
  أرجع std::unique_ptr<Writable_حافظه_بفر>(Ret);
}

std::unique_ptr<Writable_حافظه_بفر>
Writable_حافظه_بفر::get_New_Mem_بفر(size_t اندازه, ثابت Twine &بفر_Name) {
  خپلکاری SB = Writable_حافظه_بفر::get_New_Uninit_Mem_بفر(اندازه, بفر_Name);
  إذا (!SB)
    أرجع تش_اشاره;
  memset(SB->get_بفر_پیل(), 0, اندازه);
  أرجع SB;
}

ساكن منطقي shouldUseMmap(sys::fs::file_t FD,
                          size_t File_اندازه,
                          size_t Map_اندازه,
                          off_t دننه_واټن,
                          منطقي RequiresNullTerminator,
                          صحيح پاڼه_اندازه,
                          منطقي IsVolatile) {
#إذا defined(__MVS__)
  // zOS Enhanced ASCII auto convert does not support mmap.
  أرجع خطأ;
#د_که_پای

  // mmap may leave the buffer without null terminator if the file size changed
  // by the time the last page is mapped in, so avoid it if the file size is
  // likely to change.
  إذا (IsVolatile && RequiresNullTerminator)
    أرجع خطأ;

  // We don't use mmap for small files because this can severely fragment our
  // address space.
  إذا (Map_اندازه < 4 * 4096 || Map_اندازه < (بینښه)پاڼه_اندازه)
    أرجع خطأ;

  إذا (!RequiresNullTerminator)
    أرجع ریښتیا;

  // If we don't know the file size, use fstat to find out.  fstat on an open
  // file descriptor is cheaper than stat on a random path.
  // FIXME: this chunk of code is duplicated, but it avoids a fstat when
  // RequiresNullTerminator = false and MapSize != -1.
  إذا (File_اندازه == size_t(-1)) {
    sys::fs::file_حالت حالت;
    إذا (sys::fs::حالت_2(FD, حالت))
      أرجع خطأ;
    File_اندازه = حالت.get_اندازه();
  }

  // If we need a null terminator and the end of the map is inside the file,
  // we cannot use mmap.
  size_t پای_ټکی_2 = دننه_واټن + Map_اندازه;
  أكد(پای_ټکی_2 <= File_اندازه);
  إذا (پای_ټکی_2 != File_اندازه)
    أرجع خطأ;

  // Don't try to map files that are exactly a multiple of the system page size
  // if we need a null terminator.
  إذا ((File_اندازه & (پاڼه_اندازه -1)) == 0)
    أرجع خطأ;

#إذا defined(__CYGWIN__)
  // Don't try to map files that are exactly a multiple of the physical page size
  // if we need a null terminator.
  // FIXME: We should reorganize again getPageSize() on Win32.
  إذا ((File_اندازه & (4096 - 1)) == 0)
    أرجع خطأ;
#د_که_پای

  أرجع ریښتیا;
}

ساكن ErrorOr<std::unique_ptr<Write_Through_حافظه_بفر>>
getReadWriteFile(ثابت Twine &Filename, uint64_t File_اندازه, uint64_t Map_اندازه,
                 uint64_t دننه_واټن) {
  Expected<sys::fs::file_t> FDOrErr = sys::fs::openNativeFileForReadWrite(
      Filename, sys::fs::CD_OpenExisting, sys::fs::د_None);
  إذا (!FDOrErr)
    أرجع error_منزل_Error_کوډ(FDOrErr.takeError());
  sys::fs::file_t FD = *FDOrErr;

  // Default is to map the full file.
  إذا (Map_اندازه == uint64_t(-1)) {
    // If we don't know the file size, use fstat to find out.  fstat on an open
    // file descriptor is cheaper than stat on a random path.
    إذا (File_اندازه == uint64_t(-1)) {
      sys::fs::file_حالت حالت;
      std::error_کوډ EC = sys::fs::حالت_2(FD, حالت);
      إذا (EC)
        أرجع EC;

      // If this not a file or a block device (e.g. it's a named pipe
      // or character device), we can't mmap it, so error out.
      sys::fs::file_type Type = حالت.type();
      إذا (Type != sys::fs::file_type::regular_file &&
          Type != sys::fs::file_type::بلاک_file)
        أرجع make_error_کوډ(errc::invalid_دلیل);

      File_اندازه = حالت.get_اندازه();
    }
    Map_اندازه = File_اندازه;
  }

  std::error_کوډ EC;
  std::unique_ptr<Write_Through_حافظه_بفر> پایله(
      جديد (Named_بفر_Alloc(Filename))
          حافظه_بفر_MMap_File<Write_Through_حافظه_بفر>(خطأ, FD, Map_اندازه,
                                                         دننه_واټن, EC));
  إذا (EC)
    أرجع EC;
  أرجع std::move(پایله);
}

ErrorOr<std::unique_ptr<Write_Through_حافظه_بفر>>
Write_Through_حافظه_بفر::getFile(ثابت Twine &Filename, int64_t File_اندازه) {
  sys::sandbox::violationIfEnabled();

  أرجع getReadWriteFile(Filename, File_اندازه, File_اندازه, 0);
}

/// Map a subrange of the specified file as a WritableMemoryBuffer.
ErrorOr<std::unique_ptr<Write_Through_حافظه_بفر>>
Write_Through_حافظه_بفر::get_File_ټوټه(ثابت Twine &Filename, uint64_t Map_اندازه,
                                       uint64_t دننه_واټن) {
  sys::sandbox::violationIfEnabled();

  أرجع getReadWriteFile(Filename, -1, Map_اندازه, دننه_واټن);
}

کينډۍ <ډول_نوم MB>
ساكن ErrorOr<std::unique_ptr<MB>>
getOpenFileImpl(sys::fs::file_t FD, ثابت Twine &Filename, uint64_t File_اندازه,
                uint64_t Map_اندازه, int64_t دننه_واټن, منطقي RequiresNullTerminator,
                منطقي IsVolatile, std::optional<Align> Alignment) {
  ساكن صحيح پاڼه_اندازه = sys::Process::get_پاڼه_اندازه_Estimate();

  // Default is to map the full file.
  إذا (Map_اندازه == uint64_t(-1)) {
    // If we don't know the file size, use fstat to find out.  fstat on an open
    // file descriptor is cheaper than stat on a random path.
    إذا (File_اندازه == uint64_t(-1)) {
      sys::fs::file_حالت حالت;
      std::error_کوډ EC = sys::fs::حالت_2(FD, حالت);
      إذا (EC)
        أرجع EC;

      // If this not a file or a block device (e.g. it's a named pipe
      // or character device), we can't trust the size. Create the memory
      // buffer by copying off the stream.
      sys::fs::file_type Type = حالت.type();
      إذا (Type != sys::fs::file_type::regular_file &&
          Type != sys::fs::file_type::بلاک_file)
        أرجع get_حافظه_بفر_For_بهیر(FD, Filename);

      File_اندازه = حالت.get_اندازه();
    }
    Map_اندازه = File_اندازه;
  }

  إذا (shouldUseMmap(FD, File_اندازه, Map_اندازه, دننه_واټن, RequiresNullTerminator,
                    پاڼه_اندازه, IsVolatile)) {
    std::error_کوډ EC;
    std::unique_ptr<MB> پایله(
        جديد (Named_بفر_Alloc(Filename)) حافظه_بفر_MMap_File<MB>(
            RequiresNullTerminator, FD, Map_اندازه, دننه_واټن, EC));
    إذا (!EC) {
      // On at least Linux, and possibly on other systems, mmap may return pages
      // from the page cache that are not properly filled with trailing zeroes,
      // if some prior user of the page wrote non-zero bytes. Detect this and
      // don't use mmap in that case.
      إذا (!RequiresNullTerminator || *پایله->get_بفر_پای_ټکی() == '\0')
        أرجع std::move(پایله);
    }
  }

#که_تعریف_شوی __MVS__
  ErrorOr<منطقي> NeedsConversion = needConversion(Filename, FD);
  إذا (std::error_کوډ EC = NeedsConversion.getError())
    أرجع EC;
  // File size may increase due to EBCDIC -> UTF-8 conversion, therefore we
  // cannot trust the file size and we create the memory buffer by copying
  // off the stream.
  // Note: This only works with the assumption of reading a full file (i.e,
  // Offset == 0 and MapSize == FileSize). Reading a file slice does not work.
  إذا (*NeedsConversion && دننه_واټن == 0 && Map_اندازه == File_اندازه)
    أرجع get_حافظه_بفر_For_بهیر(FD, Filename);
#د_که_پای

  خپلکاری Buf =
      Writable_حافظه_بفر::get_New_Uninit_Mem_بفر(Map_اندازه, Filename, Alignment);
  إذا (!Buf) {
    // Failed to create a buffer. The only way it can fail is if
    // new(std::nothrow) returns 0.
    أرجع make_error_کوډ(errc::not_enough_حافظه);
  }

  // Read until EOF, zero-initialize the rest.
  MutableArrayRef<حرف> منزل_Read = Buf->get_بفر();
  طالما (!منزل_Read.تشه()) {
    Expected<size_t> Read_بایټونه =
        sys::fs::read_Native_File_ټوټه(FD, منزل_Read, دننه_واټن);
    إذا (!Read_بایټونه)
      أرجع error_منزل_Error_کوډ(Read_بایټونه.takeError());
    إذا (*Read_بایټونه == 0) {
      std::memset(منزل_Read.معلومات(), 0, منزل_Read.اندازه_2());
      اكسر;
    }
    منزل_Read = منزل_Read.drop_front(*Read_بایټونه);
    دننه_واټن += *Read_بایټونه;
  }

  أرجع std::move(Buf);
}

ErrorOr<std::unique_ptr<حافظه_بفر>>
حافظه_بفر::getOpenFile(sys::fs::file_t FD, ثابت Twine &Filename,
                          uint64_t File_اندازه, منطقي RequiresNullTerminator,
                          منطقي IsVolatile, std::optional<Align> Alignment) {
  sys::sandbox::violationIfEnabled();

  أرجع getOpenFileImpl<حافظه_بفر>(FD, Filename, File_اندازه, File_اندازه, 0,
                                       RequiresNullTerminator, IsVolatile,
                                       Alignment);
}

ErrorOr<std::unique_ptr<حافظه_بفر>> حافظه_بفر::get_Open_File_ټوټه(
    sys::fs::file_t FD, ثابت Twine &Filename, uint64_t Map_اندازه, int64_t دننه_واټن,
    منطقي IsVolatile, std::optional<Align> Alignment) {
  أكد(Map_اندازه != uint64_t(-1));

  sys::sandbox::violationIfEnabled();

  أرجع getOpenFileImpl<حافظه_بفر>(FD, Filename, -1, Map_اندازه, دننه_واټن, خطأ,
                                       IsVolatile, Alignment);
}

ErrorOr<std::unique_ptr<حافظه_بفر>> حافظه_بفر::getSTDIN() {
  sys::sandbox::violationIfEnabled();

  // Read in all of the data from stdin, we cannot mmap stdin.
  //
  // FIXME: That isn't necessarily true, we should try to mmap stdin and
  // fallback if it fails.
  sys::Change_Stdin_ډېرپېښ(sys::fs::د_متن);

  أرجع get_حافظه_بفر_For_بهیر(sys::fs::getStdinHandle(), "<stdin>");
}

ErrorOr<std::unique_ptr<حافظه_بفر>>
حافظه_بفر::get_File_As_بهیر(ثابت Twine &Filename) {
  sys::sandbox::violationIfEnabled();

  Expected<sys::fs::file_t> FDOrErr =
      sys::fs::openNativeFileForRead(Filename, sys::fs::د_None);
  إذا (!FDOrErr)
    أرجع error_منزل_Error_کوډ(FDOrErr.takeError());
  sys::fs::file_t FD = *FDOrErr;
  ErrorOr<std::unique_ptr<حافظه_بفر>> Ret =
      get_حافظه_بفر_For_بهیر(FD, Filename);
  sys::fs::closeFile(FD);
  أرجع Ret;
}

حافظه_بفر_Ref حافظه_بفر::get_Mem_بفر_Ref() ثابت {
  StringRef معلومات_2 = get_بفر();
  StringRef Identifier = get_بفر_Identifier();
  أرجع حافظه_بفر_Ref(معلومات_2, Identifier);
}

Small_ویکټور_حافظه_بفر::~Small_ویکټور_حافظه_بفر() = افتراضي;
