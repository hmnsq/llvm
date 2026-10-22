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

#شامل "llvm/Support/MemoryBuffer.h"
#شامل "llvm/ADT/STLExtras.h"
#شامل "llvm/ADT/SmallString.h"
#شامل "llvm/Config/config.h"
#شامل "llvm/Support/Alignment.h"
#شامل "llvm/Support/AutoConvert.h"
#شامل "llvm/Support/Errc.h"
#شامل "llvm/Support/Error.h"
#شامل "llvm/Support/ErrorHandling.h"
#شامل "llvm/Support/FileSystem.h"
#شامل "llvm/Support/IOSandbox.h"
#شامل "llvm/Support/Process.h"
#شامل "llvm/Support/Program.h"
#شامل "llvm/Support/SmallVectorMemoryBuffer.h"
#شامل <algorithm>
#شامل <cassert>
#شامل <cstring>
#شامل <new>
#شامل <sys/types.h>
#شامل <system_error>
#اگر !defined(_MSC_VER) && !defined(__MINGW32__)
#شامل <unistd.h>
#ورنہ
#شامل <io.h>
#اگر_ختم

استعمال نامخلا llvm;

//===----------------------------------------------------------------------===//
// MemoryBuffer implementation itself.
//===----------------------------------------------------------------------===//

حافظہ_بفر::~حافظہ_بفر() = طےشدہ;

/// init - Initialize this MemoryBuffer as a reference to externally allocated
/// memory, memory that we know is already null terminated.
خالی حافظہ_بفر::init(ثابت حرف *Buf_آغاز, ثابت حرف *Buf_انتہا,
                        منطقی RequiresNullTerminator) {
  أكد((!RequiresNullTerminator || Buf_انتہا[0] == 0) &&
         "Buffer is not null terminated!");
  بفر_آغاز = Buf_آغاز;
  بفر_انتہا = Buf_انتہا;
}

//===----------------------------------------------------------------------===//
// MemoryBufferMem implementation.
//===----------------------------------------------------------------------===//

/// CopyStringRef - Copies contents of a StringRef into a block of memory and
/// null-terminates it.
ساکن خالی CopyStringRef(حرف *حافظہ_2, StringRef ڈیٹا_2) {
  اگر (!ڈیٹا_2.خالی_خانے())
    حافظہ_نقل(حافظہ_2, ڈیٹا_2.ڈیٹا(), ڈیٹا_2.سائز_2());
  حافظہ_2[ڈیٹا_2.سائز_2()] = 0; // Null terminate string.
}

نامخلا {
ساخت Named_بفر_Alloc {
  ثابت Twine &Name;
  Named_بفر_Alloc(ثابت Twine &Name) : Name(Name) {}
};
} // namespace

خالی *عامل نیا(size_t N, ثابت Named_بفر_Alloc &Alloc) {
  SmallString<256> NameBuf;
  StringRef NameRef = Alloc.Name.مقصد_String_Ref(NameBuf);

  // We use malloc() and manually handle it returning null instead of calling
  // operator new because we need all uses of NamedBufferAlloc to be
  // deallocated with a call to free() due to needing to use malloc() in
  // WritableMemoryBuffer::getNewUninitMemBuffer() to work around the out-of-
  // memory handler installed by default in LLVM. See operator delete() member
  // functions within this file for the paired call to free().
  حرف *Mem =
      static_cast<حرف *>(std::حافظہ_دو(N + حجم(size_t) + NameRef.سائز_2() + 1));
  اگر (!Mem)
    llvm::report_bad_alloc_error("Allocation failed");
  *reinterpret_cast<size_t *>(Mem + N) = NameRef.سائز_2();
  CopyStringRef(Mem + N + حجم(size_t), NameRef);
  واپس Mem;
}

نامخلا {
/// MemoryBufferMem - Named MemoryBuffer pointing to a block of memory.
سانچہ<قسم_نام MB>
جماعت حافظہ_بفر_Mem : عوامی MB {
عوامی:
  حافظہ_بفر_Mem(StringRef داخلہ_ڈیٹا, منطقی RequiresNullTerminator) {
    حافظہ_بفر::init(داخلہ_ڈیٹا.begin(), داخلہ_ڈیٹا.انتہا(),
                       RequiresNullTerminator);
  }

  /// Disable sized deallocation for MemoryBufferMem, because it has
  /// tail-allocated data.
  خالی عامل حذف(خالی *p) { std::آزاد(p); }

  StringRef get_بفر_Identifier() ثابت override {
    // The name is stored after the class itself.
    واپس StringRef(reinterpret_cast<ثابت حرف *>(یہ + 1) + حجم(size_t),
                     *reinterpret_cast<ثابت size_t *>(یہ + 1));
  }

  حافظہ_بفر::بفر_قسم get_بفر_قسم() ثابت override {
    واپس حافظہ_بفر::حافظہ_بفر_Malloc;
  }
};
} // namespace

سانچہ <قسم_نام MB>
ساکن ErrorOr<std::unique_ptr<MB>>
getFileAux(ثابت Twine &Filename, uint64_t Map_سائز, uint64_t انحراف,
           منطقی Is_متن, منطقی RequiresNullTerminator, منطقی IsVolatile,
           std::optional<Align> Alignment);

std::unique_ptr<حافظہ_بفر>
حافظہ_بفر::get_Mem_بفر(StringRef داخلہ_ڈیٹا, StringRef بفر_Name,
                           منطقی RequiresNullTerminator) {
  خودکار *Ret = نیا (Named_بفر_Alloc(بفر_Name))
      حافظہ_بفر_Mem<حافظہ_بفر>(داخلہ_ڈیٹا, RequiresNullTerminator);
  واپس std::unique_ptr<حافظہ_بفر>(Ret);
}

std::unique_ptr<حافظہ_بفر>
حافظہ_بفر::get_Mem_بفر(حافظہ_بفر_Ref Ref, منطقی RequiresNullTerminator) {
  واپس std::unique_ptr<حافظہ_بفر>(get_Mem_بفر(
      Ref.get_بفر(), Ref.get_بفر_Identifier(), RequiresNullTerminator));
}

ساکن ErrorOr<std::unique_ptr<Writable_حافظہ_بفر>>
get_Mem_بفر_Copy_Impl(StringRef داخلہ_ڈیٹا, ثابت Twine &بفر_Name) {
  خودکار Buf =
      Writable_حافظہ_بفر::get_New_Uninit_Mem_بفر(داخلہ_ڈیٹا.سائز_2(), بفر_Name);
  اگر (!Buf)
    واپس make_error_رمز(errc::not_enough_حافظہ);
  // Calling memcpy with null src/dst is UB, and an empty StringRef is
  // represented with {nullptr, 0}.
  llvm::copy(داخلہ_ڈیٹا, Buf->get_بفر_آغاز());
  واپس std::move(Buf);
}

std::unique_ptr<حافظہ_بفر>
حافظہ_بفر::get_Mem_بفر_Copy(StringRef داخلہ_ڈیٹا, ثابت Twine &بفر_Name) {
  خودکار Buf = get_Mem_بفر_Copy_Impl(داخلہ_ڈیٹا, بفر_Name);
  اگر (Buf)
    واپس std::move(*Buf);
  واپس خالی_اشاریہ;
}

ErrorOr<std::unique_ptr<حافظہ_بفر>>
حافظہ_بفر::getFileOrSTDIN(ثابت Twine &Filename, منطقی Is_متن,
                             منطقی RequiresNullTerminator,
                             std::optional<Align> Alignment) {
  sys::sandbox::violationIfEnabled();

  SmallString<256> NameBuf;
  StringRef NameRef = Filename.مقصد_String_Ref(NameBuf);

  اگر (NameRef == "-")
    واپس getSTDIN();
  واپس getFile(Filename, Is_متن, RequiresNullTerminator,
                 /*IsVolatile=*/جھوٹ, Alignment);
}

ErrorOr<std::unique_ptr<حافظہ_بفر>>
حافظہ_بفر::get_File_قاش(ثابت Twine &File_راستہ, uint64_t Map_سائز,
                           uint64_t انحراف, منطقی IsVolatile,
                           std::optional<Align> Alignment) {
  sys::sandbox::violationIfEnabled();

  واپس getFileAux<حافظہ_بفر>(File_راستہ, Map_سائز, انحراف, /*IsText=*/جھوٹ,
                                  /*RequiresNullTerminator=*/جھوٹ, IsVolatile,
                                  Alignment);
}

//===----------------------------------------------------------------------===//
// MemoryBuffer::getFile implementation.
//===----------------------------------------------------------------------===//

نامخلا {

سانچہ <قسم_نام MB>
constexpr sys::fs::mapped_file_region::mapmode Mapmode =
    sys::fs::mapped_file_region::readonly;
سانچہ <>
constexpr sys::fs::mapped_file_region::mapmode Mapmode<حافظہ_بفر> =
    sys::fs::mapped_file_region::readonly;
سانچہ <>
constexpr sys::fs::mapped_file_region::mapmode Mapmode<Writable_حافظہ_بفر> =
    sys::fs::mapped_file_region::priv;
سانچہ <>
constexpr sys::fs::mapped_file_region::mapmode
    Mapmode<Write_Through_حافظہ_بفر> = sys::fs::mapped_file_region::readwrite;

/// Memory maps a file descriptor using sys::fs::mapped_file_region.
///
/// This handles converting the offset into a legal offset on the platform.
سانچہ<قسم_نام MB>
جماعت حافظہ_بفر_MMap_File : عوامی MB {
  sys::fs::mapped_file_region MFR;

  ساکن uint64_t get_Legal_Map_انحراف(uint64_t انحراف) {
    واپس انحراف & ~(sys::fs::mapped_file_region::alignment() - 1);
  }

  ساکن uint64_t get_Legal_Map_سائز(uint64_t Len, uint64_t انحراف) {
    واپس Len + (انحراف - get_Legal_Map_انحراف(انحراف));
  }

  ثابت حرف *get_آغاز(uint64_t Len, uint64_t انحراف) {
    واپس MFR.const_ڈیٹا() + (انحراف - get_Legal_Map_انحراف(انحراف));
  }

عوامی:
  حافظہ_بفر_MMap_File(منطقی RequiresNullTerminator, sys::fs::file_t FD, uint64_t Len,
                       uint64_t انحراف, std::error_رمز &EC)
      : MFR(FD, Mapmode<MB>, get_Legal_Map_سائز(Len, انحراف),
            get_Legal_Map_انحراف(انحراف), EC) {
    اگر (!EC) {
      ثابت حرف *آغاز = get_آغاز(Len, انحراف);
      حافظہ_بفر::init(آغاز, آغاز + Len, RequiresNullTerminator);
    }
  }

  /// Disable sized deallocation for MemoryBufferMMapFile, because it has
  /// tail-allocated data.
  خالی عامل حذف(خالی *p) { std::آزاد(p); }

  StringRef get_بفر_Identifier() ثابت override {
    // The name is stored after the class itself.
    واپس StringRef(reinterpret_cast<ثابت حرف *>(یہ + 1) + حجم(size_t),
                     *reinterpret_cast<ثابت size_t *>(یہ + 1));
  }

  حافظہ_بفر::بفر_قسم get_بفر_قسم() ثابت override {
    واپس حافظہ_بفر::حافظہ_بفر_MMap;
  }

  خالی dontNeedIfMmap() override { MFR.dontNeed(); }
  خالی willNeedIfMmap() override { MFR.willNeed(); }
  خالی randomAccessIfMmap() override { MFR.randomAccess(); }
};
} // namespace

ساکن ErrorOr<std::unique_ptr<Writable_حافظہ_بفر>>
get_حافظہ_بفر_For_دھارا(sys::fs::file_t FD, ثابت Twine &بفر_Name) {
  SmallString<sys::fs::Default_Read_Chunk_سائز> بفر;
  اگر (Error E = sys::fs::read_Native_File_مقصد_EOF(FD, بفر))
    واپس error_مقصد_Error_رمز(std::move(E));
  واپس get_Mem_بفر_Copy_Impl(بفر, بفر_Name);
}

ErrorOr<std::unique_ptr<حافظہ_بفر>>
حافظہ_بفر::getFile(ثابت Twine &Filename, منطقی Is_متن,
                      منطقی RequiresNullTerminator, منطقی IsVolatile,
                      std::optional<Align> Alignment) {
  sys::sandbox::violationIfEnabled();

  واپس getFileAux<حافظہ_بفر>(Filename, /*MapSize=*/-1, /*Offset=*/0,
                                  Is_متن, RequiresNullTerminator, IsVolatile,
                                  Alignment);
}

سانچہ <قسم_نام MB>
ساکن ErrorOr<std::unique_ptr<MB>>
getOpenFileImpl(sys::fs::file_t FD, ثابت Twine &Filename, uint64_t File_سائز,
                uint64_t Map_سائز, int64_t انحراف, منطقی RequiresNullTerminator,
                منطقی IsVolatile, std::optional<Align> Alignment);

سانچہ <قسم_نام MB>
ساکن ErrorOr<std::unique_ptr<MB>>
getFileAux(ثابت Twine &Filename, uint64_t Map_سائز, uint64_t انحراف,
           منطقی Is_متن, منطقی RequiresNullTerminator, منطقی IsVolatile,
           std::optional<Align> Alignment) {
  Expected<sys::fs::file_t> FDOrErr = sys::fs::openNativeFileForRead(
      Filename, Is_متن ? sys::fs::کا_متن_With_CRLF : sys::fs::کا_None);
  اگر (!FDOrErr)
    واپس error_مقصد_Error_رمز(FDOrErr.takeError());
  sys::fs::file_t FD = *FDOrErr;
  خودکار Ret = getOpenFileImpl<MB>(FD, Filename, /*FileSize=*/-1, Map_سائز, انحراف,
                                 RequiresNullTerminator, IsVolatile, Alignment);
  sys::fs::closeFile(FD);
  واپس Ret;
}

ErrorOr<std::unique_ptr<Writable_حافظہ_بفر>>
Writable_حافظہ_بفر::getFile(ثابت Twine &Filename, منطقی IsVolatile,
                              std::optional<Align> Alignment) {
  sys::sandbox::violationIfEnabled();

  واپس getFileAux<Writable_حافظہ_بفر>(
      Filename, /*MapSize=*/-1, /*Offset=*/0, /*IsText=*/جھوٹ,
      /*RequiresNullTerminator=*/جھوٹ, IsVolatile, Alignment);
}

ErrorOr<std::unique_ptr<Writable_حافظہ_بفر>>
Writable_حافظہ_بفر::get_File_قاش(ثابت Twine &Filename, uint64_t Map_سائز,
                                   uint64_t انحراف, منطقی IsVolatile,
                                   std::optional<Align> Alignment) {
  sys::sandbox::violationIfEnabled();

  واپس getFileAux<Writable_حافظہ_بفر>(
      Filename, Map_سائز, انحراف, /*IsText=*/جھوٹ,
      /*RequiresNullTerminator=*/جھوٹ, IsVolatile, Alignment);
}

std::unique_ptr<Writable_حافظہ_بفر>
Writable_حافظہ_بفر::get_New_Uninit_Mem_بفر(size_t سائز,
                                            ثابت Twine &بفر_Name,
                                            std::optional<Align> Alignment) {
  استعمال Mem_بفر = حافظہ_بفر_Mem<Writable_حافظہ_بفر>;

  // Use 16-byte alignment if no alignment is specified.
  Align BufAlign = Alignment.قدر_or(Align(16));

  // Allocate space for the MemoryBuffer, the data and the name. It is important
  // that MemoryBuffer and data are aligned so PointerIntPair works with them.
  SmallString<256> NameBuf;
  StringRef NameRef = بفر_Name.مقصد_String_Ref(NameBuf);

  size_t StringLen = حجم(Mem_بفر) + حجم(size_t) + NameRef.سائز_2() + 1;
  size_t RealLen = StringLen + سائز + 1 + BufAlign.قدر_2();
  اگر (RealLen <= سائز) // Check for rollover.
    واپس خالی_اشاریہ;
  // We use a call to malloc() rather than a call to a non-throwing operator
  // new() because LLVM unconditionally installs an out of memory new handler
  // when exceptions are disabled. This new handler intentionally crashes to
  // aid with debugging, but that makes non-throwing new calls unhelpful.
  // See MemoryBufferMem::operator delete() for the paired call to free(), and
  // llvm::install_out_of_memory_new_handler() for the installation of the
  // custom new handler.
  حرف *Mem = static_cast<حرف *>(std::حافظہ_دو(RealLen));
  اگر (!Mem)
    واپس خالی_اشاریہ;

  // The name is stored after the class itself.
  *reinterpret_cast<size_t *>(Mem + حجم(Mem_بفر)) = NameRef.سائز_2();
  CopyStringRef(Mem + حجم(Mem_بفر) + حجم(size_t), NameRef);

  // The buffer begins after the name and must be aligned.
  حرف *Buf = (حرف *)alignAddr(Mem + StringLen, BufAlign);
  Buf[سائز] = 0; // Null terminate buffer.

  خودکار *Ret = نیا (Mem) Mem_بفر(StringRef(Buf, سائز), سچ);
  واپس std::unique_ptr<Writable_حافظہ_بفر>(Ret);
}

std::unique_ptr<Writable_حافظہ_بفر>
Writable_حافظہ_بفر::get_New_Mem_بفر(size_t سائز, ثابت Twine &بفر_Name) {
  خودکار SB = Writable_حافظہ_بفر::get_New_Uninit_Mem_بفر(سائز, بفر_Name);
  اگر (!SB)
    واپس خالی_اشاریہ;
  memset(SB->get_بفر_آغاز(), 0, سائز);
  واپس SB;
}

ساکن منطقی shouldUseMmap(sys::fs::file_t FD,
                          size_t File_سائز,
                          size_t Map_سائز,
                          off_t انحراف,
                          منطقی RequiresNullTerminator,
                          صحیح صفحہ_سائز,
                          منطقی IsVolatile) {
#اگر defined(__MVS__)
  // zOS Enhanced ASCII auto convert does not support mmap.
  واپس جھوٹ;
#اگر_ختم

  // mmap may leave the buffer without null terminator if the file size changed
  // by the time the last page is mapped in, so avoid it if the file size is
  // likely to change.
  اگر (IsVolatile && RequiresNullTerminator)
    واپس جھوٹ;

  // We don't use mmap for small files because this can severely fragment our
  // address space.
  اگر (Map_سائز < 4 * 4096 || Map_سائز < (غیرعلامتی)صفحہ_سائز)
    واپس جھوٹ;

  اگر (!RequiresNullTerminator)
    واپس سچ;

  // If we don't know the file size, use fstat to find out.  fstat on an open
  // file descriptor is cheaper than stat on a random path.
  // FIXME: this chunk of code is duplicated, but it avoids a fstat when
  // RequiresNullTerminator = false and MapSize != -1.
  اگر (File_سائز == size_t(-1)) {
    sys::fs::file_حالت حالت_2;
    اگر (sys::fs::حالت_3(FD, حالت_2))
      واپس جھوٹ;
    File_سائز = حالت_2.get_سائز();
  }

  // If we need a null terminator and the end of the map is inside the file,
  // we cannot use mmap.
  size_t انتہا_2 = انحراف + Map_سائز;
  أكد(انتہا_2 <= File_سائز);
  اگر (انتہا_2 != File_سائز)
    واپس جھوٹ;

  // Don't try to map files that are exactly a multiple of the system page size
  // if we need a null terminator.
  اگر ((File_سائز & (صفحہ_سائز -1)) == 0)
    واپس جھوٹ;

#اگر defined(__CYGWIN__)
  // Don't try to map files that are exactly a multiple of the physical page size
  // if we need a null terminator.
  // FIXME: We should reorganize again getPageSize() on Win32.
  اگر ((File_سائز & (4096 - 1)) == 0)
    واپس جھوٹ;
#اگر_ختم

  واپس سچ;
}

ساکن ErrorOr<std::unique_ptr<Write_Through_حافظہ_بفر>>
getReadWriteFile(ثابت Twine &Filename, uint64_t File_سائز, uint64_t Map_سائز,
                 uint64_t انحراف) {
  Expected<sys::fs::file_t> FDOrErr = sys::fs::openNativeFileForReadWrite(
      Filename, sys::fs::CD_OpenExisting, sys::fs::کا_None);
  اگر (!FDOrErr)
    واپس error_مقصد_Error_رمز(FDOrErr.takeError());
  sys::fs::file_t FD = *FDOrErr;

  // Default is to map the full file.
  اگر (Map_سائز == uint64_t(-1)) {
    // If we don't know the file size, use fstat to find out.  fstat on an open
    // file descriptor is cheaper than stat on a random path.
    اگر (File_سائز == uint64_t(-1)) {
      sys::fs::file_حالت حالت_2;
      std::error_رمز EC = sys::fs::حالت_3(FD, حالت_2);
      اگر (EC)
        واپس EC;

      // If this not a file or a block device (e.g. it's a named pipe
      // or character device), we can't mmap it, so error out.
      sys::fs::file_type Type = حالت_2.type();
      اگر (Type != sys::fs::file_type::regular_file &&
          Type != sys::fs::file_type::بلاک_file)
        واپس make_error_رمز(errc::invalid_دلیل);

      File_سائز = حالت_2.get_سائز();
    }
    Map_سائز = File_سائز;
  }

  std::error_رمز EC;
  std::unique_ptr<Write_Through_حافظہ_بفر> نتیجہ(
      نیا (Named_بفر_Alloc(Filename))
          حافظہ_بفر_MMap_File<Write_Through_حافظہ_بفر>(جھوٹ, FD, Map_سائز,
                                                         انحراف, EC));
  اگر (EC)
    واپس EC;
  واپس std::move(نتیجہ);
}

ErrorOr<std::unique_ptr<Write_Through_حافظہ_بفر>>
Write_Through_حافظہ_بفر::getFile(ثابت Twine &Filename, int64_t File_سائز) {
  sys::sandbox::violationIfEnabled();

  واپس getReadWriteFile(Filename, File_سائز, File_سائز, 0);
}

/// Map a subrange of the specified file as a WritableMemoryBuffer.
ErrorOr<std::unique_ptr<Write_Through_حافظہ_بفر>>
Write_Through_حافظہ_بفر::get_File_قاش(ثابت Twine &Filename, uint64_t Map_سائز,
                                       uint64_t انحراف) {
  sys::sandbox::violationIfEnabled();

  واپس getReadWriteFile(Filename, -1, Map_سائز, انحراف);
}

سانچہ <قسم_نام MB>
ساکن ErrorOr<std::unique_ptr<MB>>
getOpenFileImpl(sys::fs::file_t FD, ثابت Twine &Filename, uint64_t File_سائز,
                uint64_t Map_سائز, int64_t انحراف, منطقی RequiresNullTerminator,
                منطقی IsVolatile, std::optional<Align> Alignment) {
  ساکن صحیح صفحہ_سائز = sys::Process::get_صفحہ_سائز_Estimate();

  // Default is to map the full file.
  اگر (Map_سائز == uint64_t(-1)) {
    // If we don't know the file size, use fstat to find out.  fstat on an open
    // file descriptor is cheaper than stat on a random path.
    اگر (File_سائز == uint64_t(-1)) {
      sys::fs::file_حالت حالت_2;
      std::error_رمز EC = sys::fs::حالت_3(FD, حالت_2);
      اگر (EC)
        واپس EC;

      // If this not a file or a block device (e.g. it's a named pipe
      // or character device), we can't trust the size. Create the memory
      // buffer by copying off the stream.
      sys::fs::file_type Type = حالت_2.type();
      اگر (Type != sys::fs::file_type::regular_file &&
          Type != sys::fs::file_type::بلاک_file)
        واپس get_حافظہ_بفر_For_دھارا(FD, Filename);

      File_سائز = حالت_2.get_سائز();
    }
    Map_سائز = File_سائز;
  }

  اگر (shouldUseMmap(FD, File_سائز, Map_سائز, انحراف, RequiresNullTerminator,
                    صفحہ_سائز, IsVolatile)) {
    std::error_رمز EC;
    std::unique_ptr<MB> نتیجہ(
        نیا (Named_بفر_Alloc(Filename)) حافظہ_بفر_MMap_File<MB>(
            RequiresNullTerminator, FD, Map_سائز, انحراف, EC));
    اگر (!EC) {
      // On at least Linux, and possibly on other systems, mmap may return pages
      // from the page cache that are not properly filled with trailing zeroes,
      // if some prior user of the page wrote non-zero bytes. Detect this and
      // don't use mmap in that case.
      اگر (!RequiresNullTerminator || *نتیجہ->get_بفر_انتہا() == '\0')
        واپس std::move(نتیجہ);
    }
  }

#اگر_تعریف_شدہ __MVS__
  ErrorOr<منطقی> NeedsConversion = needConversion(Filename, FD);
  اگر (std::error_رمز EC = NeedsConversion.getError())
    واپس EC;
  // File size may increase due to EBCDIC -> UTF-8 conversion, therefore we
  // cannot trust the file size and we create the memory buffer by copying
  // off the stream.
  // Note: This only works with the assumption of reading a full file (i.e,
  // Offset == 0 and MapSize == FileSize). Reading a file slice does not work.
  اگر (*NeedsConversion && انحراف == 0 && Map_سائز == File_سائز)
    واپس get_حافظہ_بفر_For_دھارا(FD, Filename);
#اگر_ختم

  خودکار Buf =
      Writable_حافظہ_بفر::get_New_Uninit_Mem_بفر(Map_سائز, Filename, Alignment);
  اگر (!Buf) {
    // Failed to create a buffer. The only way it can fail is if
    // new(std::nothrow) returns 0.
    واپس make_error_رمز(errc::not_enough_حافظہ);
  }

  // Read until EOF, zero-initialize the rest.
  MutableArrayRef<حرف> مقصد_Read = Buf->get_بفر();
  جب (!مقصد_Read.خالی_خانے()) {
    Expected<size_t> Read_بائٹیں =
        sys::fs::read_Native_File_قاش(FD, مقصد_Read, انحراف);
    اگر (!Read_بائٹیں)
      واپس error_مقصد_Error_رمز(Read_بائٹیں.takeError());
    اگر (*Read_بائٹیں == 0) {
      std::memset(مقصد_Read.ڈیٹا(), 0, مقصد_Read.سائز_2());
      توڑو;
    }
    مقصد_Read = مقصد_Read.drop_front(*Read_بائٹیں);
    انحراف += *Read_بائٹیں;
  }

  واپس std::move(Buf);
}

ErrorOr<std::unique_ptr<حافظہ_بفر>>
حافظہ_بفر::getOpenFile(sys::fs::file_t FD, ثابت Twine &Filename,
                          uint64_t File_سائز, منطقی RequiresNullTerminator,
                          منطقی IsVolatile, std::optional<Align> Alignment) {
  sys::sandbox::violationIfEnabled();

  واپس getOpenFileImpl<حافظہ_بفر>(FD, Filename, File_سائز, File_سائز, 0,
                                       RequiresNullTerminator, IsVolatile,
                                       Alignment);
}

ErrorOr<std::unique_ptr<حافظہ_بفر>> حافظہ_بفر::get_Open_File_قاش(
    sys::fs::file_t FD, ثابت Twine &Filename, uint64_t Map_سائز, int64_t انحراف,
    منطقی IsVolatile, std::optional<Align> Alignment) {
  أكد(Map_سائز != uint64_t(-1));

  sys::sandbox::violationIfEnabled();

  واپس getOpenFileImpl<حافظہ_بفر>(FD, Filename, -1, Map_سائز, انحراف, جھوٹ,
                                       IsVolatile, Alignment);
}

ErrorOr<std::unique_ptr<حافظہ_بفر>> حافظہ_بفر::getSTDIN() {
  sys::sandbox::violationIfEnabled();

  // Read in all of the data from stdin, we cannot mmap stdin.
  //
  // FIXME: That isn't necessarily true, we should try to mmap stdin and
  // fallback if it fails.
  sys::Change_Stdin_منوال(sys::fs::کا_متن);

  واپس get_حافظہ_بفر_For_دھارا(sys::fs::getStdinHandle(), "<stdin>");
}

ErrorOr<std::unique_ptr<حافظہ_بفر>>
حافظہ_بفر::get_File_As_دھارا(ثابت Twine &Filename) {
  sys::sandbox::violationIfEnabled();

  Expected<sys::fs::file_t> FDOrErr =
      sys::fs::openNativeFileForRead(Filename, sys::fs::کا_None);
  اگر (!FDOrErr)
    واپس error_مقصد_Error_رمز(FDOrErr.takeError());
  sys::fs::file_t FD = *FDOrErr;
  ErrorOr<std::unique_ptr<حافظہ_بفر>> Ret =
      get_حافظہ_بفر_For_دھارا(FD, Filename);
  sys::fs::closeFile(FD);
  واپس Ret;
}

حافظہ_بفر_Ref حافظہ_بفر::get_Mem_بفر_Ref() ثابت {
  StringRef ڈیٹا_2 = get_بفر();
  StringRef Identifier = get_بفر_Identifier();
  واپس حافظہ_بفر_Ref(ڈیٹا_2, Identifier);
}

Small_سمتیہ_حافظہ_بفر::~Small_سمتیہ_حافظہ_بفر() = طےشدہ;
