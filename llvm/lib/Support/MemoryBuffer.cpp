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

#অন্তর্ভাব "llvm/Support/MemoryBuffer.h"
#অন্তর্ভাব "llvm/ADT/STLExtras.h"
#অন্তর্ভাব "llvm/ADT/SmallString.h"
#অন্তর্ভাব "llvm/Config/config.h"
#অন্তর্ভাব "llvm/Support/Alignment.h"
#অন্তর্ভাব "llvm/Support/AutoConvert.h"
#অন্তর্ভাব "llvm/Support/Errc.h"
#অন্তর্ভাব "llvm/Support/Error.h"
#অন্তর্ভাব "llvm/Support/ErrorHandling.h"
#অন্তর্ভাব "llvm/Support/FileSystem.h"
#অন্তর্ভাব "llvm/Support/IOSandbox.h"
#অন্তর্ভাব "llvm/Support/Process.h"
#অন্তর্ভাব "llvm/Support/Program.h"
#অন্তর্ভাব "llvm/Support/SmallVectorMemoryBuffer.h"
#অন্তর্ভাব <algorithm>
#অন্তর্ভাব <cassert>
#অন্তর্ভাব <cstring>
#অন্তর্ভাব <new>
#অন্তর্ভাব <sys/types.h>
#অন্তর্ভাব <system_error>
#যদি !defined(_MSC_VER) && !defined(__MINGW32__)
#অন্তর্ভাব <unistd.h>
#অন্যথা
#অন্তর্ভাব <io.h>
#endif

using নামাকাশ llvm;

//===----------------------------------------------------------------------===//
// MemoryBuffer implementation itself.
//===----------------------------------------------------------------------===//

Memory_বাফার::~Memory_বাফার() = পূর্বনির্ধারিত;

/// init - Initialize this MemoryBuffer as a reference to externally allocated
/// memory, memory that we know is already null terminated.
শূন্য Memory_বাফার::init(ধ্রুবক অক্ষর *BufStart, ধ্রুবক অক্ষর *BufEnd,
                        যৌক্তিক RequiresNullTerminator) {
  প্রতিজ্ঞা((!RequiresNullTerminator || BufEnd[0] == 0) &&
         "Buffer is not null terminated!");
  বাফার_Start = BufStart;
  বাফার_End = BufEnd;
}

//===----------------------------------------------------------------------===//
// MemoryBufferMem implementation.
//===----------------------------------------------------------------------===//

/// CopyStringRef - Copies contents of a StringRef into a block of memory and
/// null-terminates it.
স্থির শূন্য CopyStringRef(অক্ষর *Memory, StringRef তথ্য_2) {
  যদি (!তথ্য_2.empty())
    memcpy(Memory, তথ্য_2.তথ্য(), তথ্য_2.আকার_3());
  Memory[তথ্য_2.আকার_3()] = 0; // Null terminate string.
}

নামাকাশ {
কাঠামো Named_বাফার_Alloc {
  ধ্রুবক Twine &Name;
  Named_বাফার_Alloc(ধ্রুবক Twine &Name) : Name(Name) {}
};
} // namespace

শূন্য *operator নতুন(size_t N, ধ্রুবক Named_বাফার_Alloc &Alloc) {
  SmallString<256> NameBuf;
  StringRef NameRef = Alloc.Name.toStringRef(NameBuf);

  // We use malloc() and manually handle it returning null instead of calling
  // operator new because we need all uses of NamedBufferAlloc to be
  // deallocated with a call to free() due to needing to use malloc() in
  // WritableMemoryBuffer::getNewUninitMemBuffer() to work around the out-of-
  // memory handler installed by default in LLVM. See operator delete() member
  // functions within this file for the paired call to free().
  অক্ষর *Mem =
      static_cast<অক্ষর *>(std::malloc(N + আকার(size_t) + NameRef.আকার_3() + 1));
  যদি (!Mem)
    llvm::report_bad_alloc_error("Allocation failed");
  *reinterpret_cast<size_t *>(Mem + N) = NameRef.আকার_3();
  CopyStringRef(Mem + N + আকার(size_t), NameRef);
  ফেরত Mem;
}

নামাকাশ {
/// MemoryBufferMem - Named MemoryBuffer pointing to a block of memory.
template<typename MB>
শ্রেণী Memory_বাফার_Mem : সর্বজনীন MB {
সর্বজনীন:
  Memory_বাফার_Mem(StringRef প্রবেশ_তথ্য, যৌক্তিক RequiresNullTerminator) {
    Memory_বাফার::init(প্রবেশ_তথ্য.begin(), প্রবেশ_তথ্য.end(),
                       RequiresNullTerminator);
  }

  /// Disable sized deallocation for MemoryBufferMem, because it has
  /// tail-allocated data.
  শূন্য operator delete(শূন্য *p) { std::মুক্ত(p); }

  StringRef get_বাফার_Identifier() ধ্রুবক override {
    // The name is stored after the class itself.
    ফেরত StringRef(reinterpret_cast<ধ্রুবক অক্ষর *>(এই + 1) + আকার(size_t),
                     *reinterpret_cast<ধ্রুবক size_t *>(এই + 1));
  }

  Memory_বাফার::বাফার_Kind get_বাফার_Kind() ধ্রুবক override {
    ফেরত Memory_বাফার::Memory_বাফার_Malloc;
  }
};
} // namespace

template <typename MB>
স্থির ErrorOr<std::unique_ptr<MB>>
getFileAux(ধ্রুবক Twine &Filename, uint64_t Map_আকার, uint64_t Offset,
           যৌক্তিক IsText, যৌক্তিক RequiresNullTerminator, যৌক্তিক IsVolatile,
           std::optional<Align> Alignment);

std::unique_ptr<Memory_বাফার>
Memory_বাফার::get_Mem_বাফার(StringRef প্রবেশ_তথ্য, StringRef বাফার_Name,
                           যৌক্তিক RequiresNullTerminator) {
  auto *Ret = নতুন (Named_বাফার_Alloc(বাফার_Name))
      Memory_বাফার_Mem<Memory_বাফার>(প্রবেশ_তথ্য, RequiresNullTerminator);
  ফেরত std::unique_ptr<Memory_বাফার>(Ret);
}

std::unique_ptr<Memory_বাফার>
Memory_বাফার::get_Mem_বাফার(Memory_বাফার_Ref Ref, যৌক্তিক RequiresNullTerminator) {
  ফেরত std::unique_ptr<Memory_বাফার>(get_Mem_বাফার(
      Ref.get_বাফার(), Ref.get_বাফার_Identifier(), RequiresNullTerminator));
}

স্থির ErrorOr<std::unique_ptr<Writable_Memory_বাফার>>
get_Mem_বাফার_Copy_Impl(StringRef প্রবেশ_তথ্য, ধ্রুবক Twine &বাফার_Name) {
  auto Buf =
      Writable_Memory_বাফার::get_New_Uninit_Mem_বাফার(প্রবেশ_তথ্য.আকার_3(), বাফার_Name);
  যদি (!Buf)
    ফেরত make_error_code(errc::not_enough_memory);
  // Calling memcpy with null src/dst is UB, and an empty StringRef is
  // represented with {nullptr, 0}.
  llvm::copy(প্রবেশ_তথ্য, Buf->get_বাফার_Start());
  ফেরত std::move(Buf);
}

std::unique_ptr<Memory_বাফার>
Memory_বাফার::get_Mem_বাফার_Copy(StringRef প্রবেশ_তথ্য, ধ্রুবক Twine &বাফার_Name) {
  auto Buf = get_Mem_বাফার_Copy_Impl(প্রবেশ_তথ্য, বাফার_Name);
  যদি (Buf)
    ফেরত std::move(*Buf);
  ফেরত nullptr;
}

ErrorOr<std::unique_ptr<Memory_বাফার>>
Memory_বাফার::getFileOrSTDIN(ধ্রুবক Twine &Filename, যৌক্তিক IsText,
                             যৌক্তিক RequiresNullTerminator,
                             std::optional<Align> Alignment) {
  sys::sandbox::violationIfEnabled();

  SmallString<256> NameBuf;
  StringRef NameRef = Filename.toStringRef(NameBuf);

  যদি (NameRef == "-")
    ফেরত getSTDIN();
  ফেরত getFile(Filename, IsText, RequiresNullTerminator,
                 /*IsVolatile=*/মিথ্যা, Alignment);
}

ErrorOr<std::unique_ptr<Memory_বাফার>>
Memory_বাফার::getFileSlice(ধ্রুবক Twine &FilePath, uint64_t Map_আকার,
                           uint64_t Offset, যৌক্তিক IsVolatile,
                           std::optional<Align> Alignment) {
  sys::sandbox::violationIfEnabled();

  ফেরত getFileAux<Memory_বাফার>(FilePath, Map_আকার, Offset, /*IsText=*/মিথ্যা,
                                  /*RequiresNullTerminator=*/মিথ্যা, IsVolatile,
                                  Alignment);
}

//===----------------------------------------------------------------------===//
// MemoryBuffer::getFile implementation.
//===----------------------------------------------------------------------===//

নামাকাশ {

template <typename MB>
constexpr sys::fs::mapped_file_region::mapmode Mapmode =
    sys::fs::mapped_file_region::readonly;
template <>
constexpr sys::fs::mapped_file_region::mapmode Mapmode<Memory_বাফার> =
    sys::fs::mapped_file_region::readonly;
template <>
constexpr sys::fs::mapped_file_region::mapmode Mapmode<Writable_Memory_বাফার> =
    sys::fs::mapped_file_region::priv;
template <>
constexpr sys::fs::mapped_file_region::mapmode
    Mapmode<Write_Through_Memory_বাফার> = sys::fs::mapped_file_region::readwrite;

/// Memory maps a file descriptor using sys::fs::mapped_file_region.
///
/// This handles converting the offset into a legal offset on the platform.
template<typename MB>
শ্রেণী Memory_বাফার_MMap_File : সর্বজনীন MB {
  sys::fs::mapped_file_region MFR;

  স্থির uint64_t getLegalMapOffset(uint64_t Offset) {
    ফেরত Offset & ~(sys::fs::mapped_file_region::alignment() - 1);
  }

  স্থির uint64_t get_Legal_Map_আকার(uint64_t Len, uint64_t Offset) {
    ফেরত Len + (Offset - getLegalMapOffset(Offset));
  }

  ধ্রুবক অক্ষর *getStart(uint64_t Len, uint64_t Offset) {
    ফেরত MFR.const_তথ্য() + (Offset - getLegalMapOffset(Offset));
  }

সর্বজনীন:
  Memory_বাফার_MMap_File(যৌক্তিক RequiresNullTerminator, sys::fs::file_t FD, uint64_t Len,
                       uint64_t Offset, std::error_code &EC)
      : MFR(FD, Mapmode<MB>, get_Legal_Map_আকার(Len, Offset),
            getLegalMapOffset(Offset), EC) {
    যদি (!EC) {
      ধ্রুবক অক্ষর *Start = getStart(Len, Offset);
      Memory_বাফার::init(Start, Start + Len, RequiresNullTerminator);
    }
  }

  /// Disable sized deallocation for MemoryBufferMMapFile, because it has
  /// tail-allocated data.
  শূন্য operator delete(শূন্য *p) { std::মুক্ত(p); }

  StringRef get_বাফার_Identifier() ধ্রুবক override {
    // The name is stored after the class itself.
    ফেরত StringRef(reinterpret_cast<ধ্রুবক অক্ষর *>(এই + 1) + আকার(size_t),
                     *reinterpret_cast<ধ্রুবক size_t *>(এই + 1));
  }

  Memory_বাফার::বাফার_Kind get_বাফার_Kind() ধ্রুবক override {
    ফেরত Memory_বাফার::Memory_বাফার_MMap;
  }

  শূন্য dontNeedIfMmap() override { MFR.dontNeed(); }
  শূন্য willNeedIfMmap() override { MFR.willNeed(); }
  শূন্য randomAccessIfMmap() override { MFR.randomAccess(); }
};
} // namespace

স্থির ErrorOr<std::unique_ptr<Writable_Memory_বাফার>>
get_Memory_বাফার_For_Stream(sys::fs::file_t FD, ধ্রুবক Twine &বাফার_Name) {
  SmallString<sys::fs::Default_Read_Chunk_আকার> বাফার;
  যদি (Error E = sys::fs::readNativeFileToEOF(FD, বাফার))
    ফেরত errorToErrorCode(std::move(E));
  ফেরত get_Mem_বাফার_Copy_Impl(বাফার, বাফার_Name);
}

ErrorOr<std::unique_ptr<Memory_বাফার>>
Memory_বাফার::getFile(ধ্রুবক Twine &Filename, যৌক্তিক IsText,
                      যৌক্তিক RequiresNullTerminator, যৌক্তিক IsVolatile,
                      std::optional<Align> Alignment) {
  sys::sandbox::violationIfEnabled();

  ফেরত getFileAux<Memory_বাফার>(Filename, /*MapSize=*/-1, /*Offset=*/0,
                                  IsText, RequiresNullTerminator, IsVolatile,
                                  Alignment);
}

template <typename MB>
স্থির ErrorOr<std::unique_ptr<MB>>
getOpenFileImpl(sys::fs::file_t FD, ধ্রুবক Twine &Filename, uint64_t File_আকার,
                uint64_t Map_আকার, int64_t Offset, যৌক্তিক RequiresNullTerminator,
                যৌক্তিক IsVolatile, std::optional<Align> Alignment);

template <typename MB>
স্থির ErrorOr<std::unique_ptr<MB>>
getFileAux(ধ্রুবক Twine &Filename, uint64_t Map_আকার, uint64_t Offset,
           যৌক্তিক IsText, যৌক্তিক RequiresNullTerminator, যৌক্তিক IsVolatile,
           std::optional<Align> Alignment) {
  Expected<sys::fs::file_t> FDOrErr = sys::fs::openNativeFileForRead(
      Filename, IsText ? sys::fs::OF_TextWithCRLF : sys::fs::OF_None);
  যদি (!FDOrErr)
    ফেরত errorToErrorCode(FDOrErr.takeError());
  sys::fs::file_t FD = *FDOrErr;
  auto Ret = getOpenFileImpl<MB>(FD, Filename, /*FileSize=*/-1, Map_আকার, Offset,
                                 RequiresNullTerminator, IsVolatile, Alignment);
  sys::fs::closeFile(FD);
  ফেরত Ret;
}

ErrorOr<std::unique_ptr<Writable_Memory_বাফার>>
Writable_Memory_বাফার::getFile(ধ্রুবক Twine &Filename, যৌক্তিক IsVolatile,
                              std::optional<Align> Alignment) {
  sys::sandbox::violationIfEnabled();

  ফেরত getFileAux<Writable_Memory_বাফার>(
      Filename, /*MapSize=*/-1, /*Offset=*/0, /*IsText=*/মিথ্যা,
      /*RequiresNullTerminator=*/মিথ্যা, IsVolatile, Alignment);
}

ErrorOr<std::unique_ptr<Writable_Memory_বাফার>>
Writable_Memory_বাফার::getFileSlice(ধ্রুবক Twine &Filename, uint64_t Map_আকার,
                                   uint64_t Offset, যৌক্তিক IsVolatile,
                                   std::optional<Align> Alignment) {
  sys::sandbox::violationIfEnabled();

  ফেরত getFileAux<Writable_Memory_বাফার>(
      Filename, Map_আকার, Offset, /*IsText=*/মিথ্যা,
      /*RequiresNullTerminator=*/মিথ্যা, IsVolatile, Alignment);
}

std::unique_ptr<Writable_Memory_বাফার>
Writable_Memory_বাফার::get_New_Uninit_Mem_বাফার(size_t আকার_2,
                                            ধ্রুবক Twine &বাফার_Name,
                                            std::optional<Align> Alignment) {
  using Mem_বাফার = Memory_বাফার_Mem<Writable_Memory_বাফার>;

  // Use 16-byte alignment if no alignment is specified.
  Align BufAlign = Alignment.মান_or(Align(16));

  // Allocate space for the MemoryBuffer, the data and the name. It is important
  // that MemoryBuffer and data are aligned so PointerIntPair works with them.
  SmallString<256> NameBuf;
  StringRef NameRef = বাফার_Name.toStringRef(NameBuf);

  size_t StringLen = আকার(Mem_বাফার) + আকার(size_t) + NameRef.আকার_3() + 1;
  size_t RealLen = StringLen + আকার_2 + 1 + BufAlign.মান_2();
  যদি (RealLen <= আকার_2) // Check for rollover.
    ফেরত nullptr;
  // We use a call to malloc() rather than a call to a non-throwing operator
  // new() because LLVM unconditionally installs an out of memory new handler
  // when exceptions are disabled. This new handler intentionally crashes to
  // aid with debugging, but that makes non-throwing new calls unhelpful.
  // See MemoryBufferMem::operator delete() for the paired call to free(), and
  // llvm::install_out_of_memory_new_handler() for the installation of the
  // custom new handler.
  অক্ষর *Mem = static_cast<অক্ষর *>(std::malloc(RealLen));
  যদি (!Mem)
    ফেরত nullptr;

  // The name is stored after the class itself.
  *reinterpret_cast<size_t *>(Mem + আকার(Mem_বাফার)) = NameRef.আকার_3();
  CopyStringRef(Mem + আকার(Mem_বাফার) + আকার(size_t), NameRef);

  // The buffer begins after the name and must be aligned.
  অক্ষর *Buf = (অক্ষর *)alignAddr(Mem + StringLen, BufAlign);
  Buf[আকার_2] = 0; // Null terminate buffer.

  auto *Ret = নতুন (Mem) Mem_বাফার(StringRef(Buf, আকার_2), সত্য);
  ফেরত std::unique_ptr<Writable_Memory_বাফার>(Ret);
}

std::unique_ptr<Writable_Memory_বাফার>
Writable_Memory_বাফার::get_New_Mem_বাফার(size_t আকার_2, ধ্রুবক Twine &বাফার_Name) {
  auto SB = Writable_Memory_বাফার::get_New_Uninit_Mem_বাফার(আকার_2, বাফার_Name);
  যদি (!SB)
    ফেরত nullptr;
  memset(SB->get_বাফার_Start(), 0, আকার_2);
  ফেরত SB;
}

স্থির যৌক্তিক shouldUseMmap(sys::fs::file_t FD,
                          size_t File_আকার,
                          size_t Map_আকার,
                          off_t Offset,
                          যৌক্তিক RequiresNullTerminator,
                          পূর্ণসংখ্যা Page_আকার,
                          যৌক্তিক IsVolatile) {
#যদি defined(__MVS__)
  // zOS Enhanced ASCII auto convert does not support mmap.
  ফেরত মিথ্যা;
#endif

  // mmap may leave the buffer without null terminator if the file size changed
  // by the time the last page is mapped in, so avoid it if the file size is
  // likely to change.
  যদি (IsVolatile && RequiresNullTerminator)
    ফেরত মিথ্যা;

  // We don't use mmap for small files because this can severely fragment our
  // address space.
  যদি (Map_আকার < 4 * 4096 || Map_আকার < (unsigned)Page_আকার)
    ফেরত মিথ্যা;

  যদি (!RequiresNullTerminator)
    ফেরত সত্য;

  // If we don't know the file size, use fstat to find out.  fstat on an open
  // file descriptor is cheaper than stat on a random path.
  // FIXME: this chunk of code is duplicated, but it avoids a fstat when
  // RequiresNullTerminator = false and MapSize != -1.
  যদি (File_আকার == size_t(-1)) {
    sys::fs::file_status Status;
    যদি (sys::fs::status(FD, Status))
      ফেরত মিথ্যা;
    File_আকার = Status.get_আকার();
  }

  // If we need a null terminator and the end of the map is inside the file,
  // we cannot use mmap.
  size_t End = Offset + Map_আকার;
  প্রতিজ্ঞা(End <= File_আকার);
  যদি (End != File_আকার)
    ফেরত মিথ্যা;

  // Don't try to map files that are exactly a multiple of the system page size
  // if we need a null terminator.
  যদি ((File_আকার & (Page_আকার -1)) == 0)
    ফেরত মিথ্যা;

#যদি defined(__CYGWIN__)
  // Don't try to map files that are exactly a multiple of the physical page size
  // if we need a null terminator.
  // FIXME: We should reorganize again getPageSize() on Win32.
  যদি ((File_আকার & (4096 - 1)) == 0)
    ফেরত মিথ্যা;
#endif

  ফেরত সত্য;
}

স্থির ErrorOr<std::unique_ptr<Write_Through_Memory_বাফার>>
getReadWriteFile(ধ্রুবক Twine &Filename, uint64_t File_আকার, uint64_t Map_আকার,
                 uint64_t Offset) {
  Expected<sys::fs::file_t> FDOrErr = sys::fs::openNativeFileForReadWrite(
      Filename, sys::fs::CD_OpenExisting, sys::fs::OF_None);
  যদি (!FDOrErr)
    ফেরত errorToErrorCode(FDOrErr.takeError());
  sys::fs::file_t FD = *FDOrErr;

  // Default is to map the full file.
  যদি (Map_আকার == uint64_t(-1)) {
    // If we don't know the file size, use fstat to find out.  fstat on an open
    // file descriptor is cheaper than stat on a random path.
    যদি (File_আকার == uint64_t(-1)) {
      sys::fs::file_status Status;
      std::error_code EC = sys::fs::status(FD, Status);
      যদি (EC)
        ফেরত EC;

      // If this not a file or a block device (e.g. it's a named pipe
      // or character device), we can't mmap it, so error out.
      sys::fs::file_type Type = Status.type();
      যদি (Type != sys::fs::file_type::regular_file &&
          Type != sys::fs::file_type::block_file)
        ফেরত make_error_code(errc::invalid_argument);

      File_আকার = Status.get_আকার();
    }
    Map_আকার = File_আকার;
  }

  std::error_code EC;
  std::unique_ptr<Write_Through_Memory_বাফার> ফলাফল(
      নতুন (Named_বাফার_Alloc(Filename))
          Memory_বাফার_MMap_File<Write_Through_Memory_বাফার>(মিথ্যা, FD, Map_আকার,
                                                         Offset, EC));
  যদি (EC)
    ফেরত EC;
  ফেরত std::move(ফলাফল);
}

ErrorOr<std::unique_ptr<Write_Through_Memory_বাফার>>
Write_Through_Memory_বাফার::getFile(ধ্রুবক Twine &Filename, int64_t File_আকার) {
  sys::sandbox::violationIfEnabled();

  ফেরত getReadWriteFile(Filename, File_আকার, File_আকার, 0);
}

/// Map a subrange of the specified file as a WritableMemoryBuffer.
ErrorOr<std::unique_ptr<Write_Through_Memory_বাফার>>
Write_Through_Memory_বাফার::getFileSlice(ধ্রুবক Twine &Filename, uint64_t Map_আকার,
                                       uint64_t Offset) {
  sys::sandbox::violationIfEnabled();

  ফেরত getReadWriteFile(Filename, -1, Map_আকার, Offset);
}

template <typename MB>
স্থির ErrorOr<std::unique_ptr<MB>>
getOpenFileImpl(sys::fs::file_t FD, ধ্রুবক Twine &Filename, uint64_t File_আকার,
                uint64_t Map_আকার, int64_t Offset, যৌক্তিক RequiresNullTerminator,
                যৌক্তিক IsVolatile, std::optional<Align> Alignment) {
  স্থির পূর্ণসংখ্যা Page_আকার = sys::Process::get_Page_আকার_Estimate();

  // Default is to map the full file.
  যদি (Map_আকার == uint64_t(-1)) {
    // If we don't know the file size, use fstat to find out.  fstat on an open
    // file descriptor is cheaper than stat on a random path.
    যদি (File_আকার == uint64_t(-1)) {
      sys::fs::file_status Status;
      std::error_code EC = sys::fs::status(FD, Status);
      যদি (EC)
        ফেরত EC;

      // If this not a file or a block device (e.g. it's a named pipe
      // or character device), we can't trust the size. Create the memory
      // buffer by copying off the stream.
      sys::fs::file_type Type = Status.type();
      যদি (Type != sys::fs::file_type::regular_file &&
          Type != sys::fs::file_type::block_file)
        ফেরত get_Memory_বাফার_For_Stream(FD, Filename);

      File_আকার = Status.get_আকার();
    }
    Map_আকার = File_আকার;
  }

  যদি (shouldUseMmap(FD, File_আকার, Map_আকার, Offset, RequiresNullTerminator,
                    Page_আকার, IsVolatile)) {
    std::error_code EC;
    std::unique_ptr<MB> ফলাফল(
        নতুন (Named_বাফার_Alloc(Filename)) Memory_বাফার_MMap_File<MB>(
            RequiresNullTerminator, FD, Map_আকার, Offset, EC));
    যদি (!EC) {
      // On at least Linux, and possibly on other systems, mmap may return pages
      // from the page cache that are not properly filled with trailing zeroes,
      // if some prior user of the page wrote non-zero bytes. Detect this and
      // don't use mmap in that case.
      যদি (!RequiresNullTerminator || *ফলাফল->get_বাফার_End() == '\0')
        ফেরত std::move(ফলাফল);
    }
  }

#ifdef __MVS__
  ErrorOr<যৌক্তিক> NeedsConversion = needConversion(Filename, FD);
  যদি (std::error_code EC = NeedsConversion.getError())
    ফেরত EC;
  // File size may increase due to EBCDIC -> UTF-8 conversion, therefore we
  // cannot trust the file size and we create the memory buffer by copying
  // off the stream.
  // Note: This only works with the assumption of reading a full file (i.e,
  // Offset == 0 and MapSize == FileSize). Reading a file slice does not work.
  যদি (*NeedsConversion && Offset == 0 && Map_আকার == File_আকার)
    ফেরত get_Memory_বাফার_For_Stream(FD, Filename);
#endif

  auto Buf =
      Writable_Memory_বাফার::get_New_Uninit_Mem_বাফার(Map_আকার, Filename, Alignment);
  যদি (!Buf) {
    // Failed to create a buffer. The only way it can fail is if
    // new(std::nothrow) returns 0.
    ফেরত make_error_code(errc::not_enough_memory);
  }

  // Read until EOF, zero-initialize the rest.
  MutableArrayRef<অক্ষর> ToRead = Buf->get_বাফার();
  যখন (!ToRead.empty()) {
    Expected<size_t> ReadBytes =
        sys::fs::readNativeFileSlice(FD, ToRead, Offset);
    যদি (!ReadBytes)
      ফেরত errorToErrorCode(ReadBytes.takeError());
    যদি (*ReadBytes == 0) {
      std::memset(ToRead.তথ্য(), 0, ToRead.আকার_3());
      বিরতি;
    }
    ToRead = ToRead.drop_front(*ReadBytes);
    Offset += *ReadBytes;
  }

  ফেরত std::move(Buf);
}

ErrorOr<std::unique_ptr<Memory_বাফার>>
Memory_বাফার::getOpenFile(sys::fs::file_t FD, ধ্রুবক Twine &Filename,
                          uint64_t File_আকার, যৌক্তিক RequiresNullTerminator,
                          যৌক্তিক IsVolatile, std::optional<Align> Alignment) {
  sys::sandbox::violationIfEnabled();

  ফেরত getOpenFileImpl<Memory_বাফার>(FD, Filename, File_আকার, File_আকার, 0,
                                       RequiresNullTerminator, IsVolatile,
                                       Alignment);
}

ErrorOr<std::unique_ptr<Memory_বাফার>> Memory_বাফার::getOpenFileSlice(
    sys::fs::file_t FD, ধ্রুবক Twine &Filename, uint64_t Map_আকার, int64_t Offset,
    যৌক্তিক IsVolatile, std::optional<Align> Alignment) {
  প্রতিজ্ঞা(Map_আকার != uint64_t(-1));

  sys::sandbox::violationIfEnabled();

  ফেরত getOpenFileImpl<Memory_বাফার>(FD, Filename, -1, Map_আকার, Offset, মিথ্যা,
                                       IsVolatile, Alignment);
}

ErrorOr<std::unique_ptr<Memory_বাফার>> Memory_বাফার::getSTDIN() {
  sys::sandbox::violationIfEnabled();

  // Read in all of the data from stdin, we cannot mmap stdin.
  //
  // FIXME: That isn't necessarily true, we should try to mmap stdin and
  // fallback if it fails.
  sys::ChangeStdinMode(sys::fs::OF_Text);

  ফেরত get_Memory_বাফার_For_Stream(sys::fs::getStdinHandle(), "<stdin>");
}

ErrorOr<std::unique_ptr<Memory_বাফার>>
Memory_বাফার::getFileAsStream(ধ্রুবক Twine &Filename) {
  sys::sandbox::violationIfEnabled();

  Expected<sys::fs::file_t> FDOrErr =
      sys::fs::openNativeFileForRead(Filename, sys::fs::OF_None);
  যদি (!FDOrErr)
    ফেরত errorToErrorCode(FDOrErr.takeError());
  sys::fs::file_t FD = *FDOrErr;
  ErrorOr<std::unique_ptr<Memory_বাফার>> Ret =
      get_Memory_বাফার_For_Stream(FD, Filename);
  sys::fs::closeFile(FD);
  ফেরত Ret;
}

Memory_বাফার_Ref Memory_বাফার::get_Mem_বাফার_Ref() ধ্রুবক {
  StringRef তথ্য_2 = get_বাফার();
  StringRef Identifier = get_বাফার_Identifier();
  ফেরত Memory_বাফার_Ref(তথ্য_2, Identifier);
}

Small_Vector_Memory_বাফার::~Small_Vector_Memory_বাফার() = পূর্বনির্ধারিত;
