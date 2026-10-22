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

MemoryBuffer::~MemoryBuffer() = স্বভাব;

/// init - Initialize this MemoryBuffer as a reference to externally allocated
/// memory, memory that we know is already null terminated.
শূন্য MemoryBuffer::init(ধ্রুব বর্ণ *BufStart, ধ্রুব বর্ণ *BufEnd,
                        তার্কিক RequiresNullTerminator) {
  প্রতিজ্ঞা((!RequiresNullTerminator || BufEnd[0] == 0) &&
         "Buffer is not null terminated!");
  BufferStart = BufStart;
  BufferEnd = BufEnd;
}

//===----------------------------------------------------------------------===//
// MemoryBufferMem implementation.
//===----------------------------------------------------------------------===//

/// CopyStringRef - Copies contents of a StringRef into a block of memory and
/// null-terminates it.
স্থির শূন্য CopyStringRef(বর্ণ *Memory, StringRef Data) {
  যদি (!Data.empty())
    memcpy(Memory, Data.data(), Data.size());
  Memory[Data.size()] = 0; // Null terminate string.
}

নামাকাশ {
সংরচনা NamedBufferAlloc {
  ধ্রুব Twine &Name;
  NamedBufferAlloc(ধ্রুব Twine &Name) : Name(Name) {}
};
} // namespace

শূন্য *operator নূতন(size_t N, ধ্রুব NamedBufferAlloc &Alloc) {
  SmallString<256> NameBuf;
  StringRef NameRef = Alloc.Name.toStringRef(NameBuf);

  // We use malloc() and manually handle it returning null instead of calling
  // operator new because we need all uses of NamedBufferAlloc to be
  // deallocated with a call to free() due to needing to use malloc() in
  // WritableMemoryBuffer::getNewUninitMemBuffer() to work around the out-of-
  // memory handler installed by default in LLVM. See operator delete() member
  // functions within this file for the paired call to free().
  বর্ণ *Mem =
      static_cast<বর্ণ *>(std::malloc(N + পরিমাণ(size_t) + NameRef.size() + 1));
  যদি (!Mem)
    llvm::report_bad_alloc_error("Allocation failed");
  *reinterpret_cast<size_t *>(Mem + N) = NameRef.size();
  CopyStringRef(Mem + N + পরিমাণ(size_t), NameRef);
  প্রত্যাগম Mem;
}

নামাকাশ {
/// MemoryBufferMem - Named MemoryBuffer pointing to a block of memory.
template<typename MB>
বর্গ MemoryBufferMem : সার্বজনিক MB {
সার্বজনিক:
  MemoryBufferMem(StringRef প্রবেশ_Data, তার্কিক RequiresNullTerminator) {
    MemoryBuffer::init(প্রবেশ_Data.begin(), প্রবেশ_Data.end(),
                       RequiresNullTerminator);
  }

  /// Disable sized deallocation for MemoryBufferMem, because it has
  /// tail-allocated data.
  শূন্য operator delete(শূন্য *p) { std::মুক্ত(p); }

  StringRef getBufferIdentifier() ধ্রুব override {
    // The name is stored after the class itself.
    প্রত্যাগম StringRef(reinterpret_cast<ধ্রুব বর্ণ *>(এতত্ + 1) + পরিমাণ(size_t),
                     *reinterpret_cast<ধ্রুব size_t *>(এতত্ + 1));
  }

  MemoryBuffer::BufferKind getBufferKind() ধ্রুব override {
    প্রত্যাগম MemoryBuffer::MemoryBuffer_Malloc;
  }
};
} // namespace

template <typename MB>
স্থির ErrorOr<std::unique_ptr<MB>>
getFileAux(ধ্রুব Twine &Filename, uint64_t MapSize, uint64_t Offset,
           তার্কিক IsText, তার্কিক RequiresNullTerminator, তার্কিক IsVolatile,
           std::optional<Align> Alignment);

std::unique_ptr<MemoryBuffer>
MemoryBuffer::getMemBuffer(StringRef প্রবেশ_Data, StringRef BufferName,
                           তার্কিক RequiresNullTerminator) {
  auto *Ret = নূতন (NamedBufferAlloc(BufferName))
      MemoryBufferMem<MemoryBuffer>(প্রবেশ_Data, RequiresNullTerminator);
  প্রত্যাগম std::unique_ptr<MemoryBuffer>(Ret);
}

std::unique_ptr<MemoryBuffer>
MemoryBuffer::getMemBuffer(MemoryBufferRef Ref, তার্কিক RequiresNullTerminator) {
  প্রত্যাগম std::unique_ptr<MemoryBuffer>(getMemBuffer(
      Ref.getBuffer(), Ref.getBufferIdentifier(), RequiresNullTerminator));
}

স্থির ErrorOr<std::unique_ptr<WritableMemoryBuffer>>
getMemBufferCopyImpl(StringRef প্রবেশ_Data, ধ্রুব Twine &BufferName) {
  auto Buf =
      WritableMemoryBuffer::getNewUninitMemBuffer(প্রবেশ_Data.size(), BufferName);
  যদি (!Buf)
    প্রত্যাগম make_error_code(errc::not_enough_memory);
  // Calling memcpy with null src/dst is UB, and an empty StringRef is
  // represented with {nullptr, 0}.
  llvm::copy(প্রবেশ_Data, Buf->getBufferStart());
  প্রত্যাগম std::move(Buf);
}

std::unique_ptr<MemoryBuffer>
MemoryBuffer::getMemBufferCopy(StringRef প্রবেশ_Data, ধ্রুব Twine &BufferName) {
  auto Buf = getMemBufferCopyImpl(প্রবেশ_Data, BufferName);
  যদি (Buf)
    প্রত্যাগম std::move(*Buf);
  প্রত্যাগম nullptr;
}

ErrorOr<std::unique_ptr<MemoryBuffer>>
MemoryBuffer::getFileOrSTDIN(ধ্রুব Twine &Filename, তার্কিক IsText,
                             তার্কিক RequiresNullTerminator,
                             std::optional<Align> Alignment) {
  sys::sandbox::violationIfEnabled();

  SmallString<256> NameBuf;
  StringRef NameRef = Filename.toStringRef(NameBuf);

  যদি (NameRef == "-")
    প্রত্যাগম getSTDIN();
  প্রত্যাগম getFile(Filename, IsText, RequiresNullTerminator,
                 /*IsVolatile=*/অসত্য, Alignment);
}

ErrorOr<std::unique_ptr<MemoryBuffer>>
MemoryBuffer::getFileSlice(ধ্রুব Twine &FilePath, uint64_t MapSize,
                           uint64_t Offset, তার্কিক IsVolatile,
                           std::optional<Align> Alignment) {
  sys::sandbox::violationIfEnabled();

  প্রত্যাগম getFileAux<MemoryBuffer>(FilePath, MapSize, Offset, /*IsText=*/অসত্য,
                                  /*RequiresNullTerminator=*/অসত্য, IsVolatile,
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
constexpr sys::fs::mapped_file_region::mapmode Mapmode<MemoryBuffer> =
    sys::fs::mapped_file_region::readonly;
template <>
constexpr sys::fs::mapped_file_region::mapmode Mapmode<WritableMemoryBuffer> =
    sys::fs::mapped_file_region::priv;
template <>
constexpr sys::fs::mapped_file_region::mapmode
    Mapmode<WriteThroughMemoryBuffer> = sys::fs::mapped_file_region::readwrite;

/// Memory maps a file descriptor using sys::fs::mapped_file_region.
///
/// This handles converting the offset into a legal offset on the platform.
template<typename MB>
বর্গ MemoryBufferMMapFile : সার্বজনিক MB {
  sys::fs::mapped_file_region MFR;

  স্থির uint64_t getLegalMapOffset(uint64_t Offset) {
    প্রত্যাগম Offset & ~(sys::fs::mapped_file_region::alignment() - 1);
  }

  স্থির uint64_t getLegalMapSize(uint64_t Len, uint64_t Offset) {
    প্রত্যাগম Len + (Offset - getLegalMapOffset(Offset));
  }

  ধ্রুব বর্ণ *getStart(uint64_t Len, uint64_t Offset) {
    প্রত্যাগম MFR.const_data() + (Offset - getLegalMapOffset(Offset));
  }

সার্বজনিক:
  MemoryBufferMMapFile(তার্কিক RequiresNullTerminator, sys::fs::file_t FD, uint64_t Len,
                       uint64_t Offset, std::error_code &EC)
      : MFR(FD, Mapmode<MB>, getLegalMapSize(Len, Offset),
            getLegalMapOffset(Offset), EC) {
    যদি (!EC) {
      ধ্রুব বর্ণ *Start = getStart(Len, Offset);
      MemoryBuffer::init(Start, Start + Len, RequiresNullTerminator);
    }
  }

  /// Disable sized deallocation for MemoryBufferMMapFile, because it has
  /// tail-allocated data.
  শূন্য operator delete(শূন্য *p) { std::মুক্ত(p); }

  StringRef getBufferIdentifier() ধ্রুব override {
    // The name is stored after the class itself.
    প্রত্যাগম StringRef(reinterpret_cast<ধ্রুব বর্ণ *>(এতত্ + 1) + পরিমাণ(size_t),
                     *reinterpret_cast<ধ্রুব size_t *>(এতত্ + 1));
  }

  MemoryBuffer::BufferKind getBufferKind() ধ্রুব override {
    প্রত্যাগম MemoryBuffer::MemoryBuffer_MMap;
  }

  শূন্য dontNeedIfMmap() override { MFR.dontNeed(); }
  শূন্য willNeedIfMmap() override { MFR.willNeed(); }
  শূন্য randomAccessIfMmap() override { MFR.randomAccess(); }
};
} // namespace

স্থির ErrorOr<std::unique_ptr<WritableMemoryBuffer>>
getMemoryBufferForStream(sys::fs::file_t FD, ধ্রুব Twine &BufferName) {
  SmallString<sys::fs::DefaultReadChunkSize> Buffer;
  যদি (Error E = sys::fs::readNativeFileToEOF(FD, Buffer))
    প্রত্যাগম errorToErrorCode(std::move(E));
  প্রত্যাগম getMemBufferCopyImpl(Buffer, BufferName);
}

ErrorOr<std::unique_ptr<MemoryBuffer>>
MemoryBuffer::getFile(ধ্রুব Twine &Filename, তার্কিক IsText,
                      তার্কিক RequiresNullTerminator, তার্কিক IsVolatile,
                      std::optional<Align> Alignment) {
  sys::sandbox::violationIfEnabled();

  প্রত্যাগম getFileAux<MemoryBuffer>(Filename, /*MapSize=*/-1, /*Offset=*/0,
                                  IsText, RequiresNullTerminator, IsVolatile,
                                  Alignment);
}

template <typename MB>
স্থির ErrorOr<std::unique_ptr<MB>>
getOpenFileImpl(sys::fs::file_t FD, ধ্রুব Twine &Filename, uint64_t FileSize,
                uint64_t MapSize, int64_t Offset, তার্কিক RequiresNullTerminator,
                তার্কিক IsVolatile, std::optional<Align> Alignment);

template <typename MB>
স্থির ErrorOr<std::unique_ptr<MB>>
getFileAux(ধ্রুব Twine &Filename, uint64_t MapSize, uint64_t Offset,
           তার্কিক IsText, তার্কিক RequiresNullTerminator, তার্কিক IsVolatile,
           std::optional<Align> Alignment) {
  Expected<sys::fs::file_t> FDOrErr = sys::fs::openNativeFileForRead(
      Filename, IsText ? sys::fs::OF_TextWithCRLF : sys::fs::OF_None);
  যদি (!FDOrErr)
    প্রত্যাগম errorToErrorCode(FDOrErr.takeError());
  sys::fs::file_t FD = *FDOrErr;
  auto Ret = getOpenFileImpl<MB>(FD, Filename, /*FileSize=*/-1, MapSize, Offset,
                                 RequiresNullTerminator, IsVolatile, Alignment);
  sys::fs::closeFile(FD);
  প্রত্যাগম Ret;
}

ErrorOr<std::unique_ptr<WritableMemoryBuffer>>
WritableMemoryBuffer::getFile(ধ্রুব Twine &Filename, তার্কিক IsVolatile,
                              std::optional<Align> Alignment) {
  sys::sandbox::violationIfEnabled();

  প্রত্যাগম getFileAux<WritableMemoryBuffer>(
      Filename, /*MapSize=*/-1, /*Offset=*/0, /*IsText=*/অসত্য,
      /*RequiresNullTerminator=*/অসত্য, IsVolatile, Alignment);
}

ErrorOr<std::unique_ptr<WritableMemoryBuffer>>
WritableMemoryBuffer::getFileSlice(ধ্রুব Twine &Filename, uint64_t MapSize,
                                   uint64_t Offset, তার্কিক IsVolatile,
                                   std::optional<Align> Alignment) {
  sys::sandbox::violationIfEnabled();

  প্রত্যাগম getFileAux<WritableMemoryBuffer>(
      Filename, MapSize, Offset, /*IsText=*/অসত্য,
      /*RequiresNullTerminator=*/অসত্য, IsVolatile, Alignment);
}

std::unique_ptr<WritableMemoryBuffer>
WritableMemoryBuffer::getNewUninitMemBuffer(size_t Size,
                                            ধ্রুব Twine &BufferName,
                                            std::optional<Align> Alignment) {
  using MemBuffer = MemoryBufferMem<WritableMemoryBuffer>;

  // Use 16-byte alignment if no alignment is specified.
  Align BufAlign = Alignment.মান_or(Align(16));

  // Allocate space for the MemoryBuffer, the data and the name. It is important
  // that MemoryBuffer and data are aligned so PointerIntPair works with them.
  SmallString<256> NameBuf;
  StringRef NameRef = BufferName.toStringRef(NameBuf);

  size_t StringLen = পরিমাণ(MemBuffer) + পরিমাণ(size_t) + NameRef.size() + 1;
  size_t RealLen = StringLen + Size + 1 + BufAlign.মান_2();
  যদি (RealLen <= Size) // Check for rollover.
    প্রত্যাগম nullptr;
  // We use a call to malloc() rather than a call to a non-throwing operator
  // new() because LLVM unconditionally installs an out of memory new handler
  // when exceptions are disabled. This new handler intentionally crashes to
  // aid with debugging, but that makes non-throwing new calls unhelpful.
  // See MemoryBufferMem::operator delete() for the paired call to free(), and
  // llvm::install_out_of_memory_new_handler() for the installation of the
  // custom new handler.
  বর্ণ *Mem = static_cast<বর্ণ *>(std::malloc(RealLen));
  যদি (!Mem)
    প্রত্যাগম nullptr;

  // The name is stored after the class itself.
  *reinterpret_cast<size_t *>(Mem + পরিমাণ(MemBuffer)) = NameRef.size();
  CopyStringRef(Mem + পরিমাণ(MemBuffer) + পরিমাণ(size_t), NameRef);

  // The buffer begins after the name and must be aligned.
  বর্ণ *Buf = (বর্ণ *)alignAddr(Mem + StringLen, BufAlign);
  Buf[Size] = 0; // Null terminate buffer.

  auto *Ret = নূতন (Mem) MemBuffer(StringRef(Buf, Size), সত্য);
  প্রত্যাগম std::unique_ptr<WritableMemoryBuffer>(Ret);
}

std::unique_ptr<WritableMemoryBuffer>
WritableMemoryBuffer::getNewMemBuffer(size_t Size, ধ্রুব Twine &BufferName) {
  auto SB = WritableMemoryBuffer::getNewUninitMemBuffer(Size, BufferName);
  যদি (!SB)
    প্রত্যাগম nullptr;
  memset(SB->getBufferStart(), 0, Size);
  প্রত্যাগম SB;
}

স্থির তার্কিক shouldUseMmap(sys::fs::file_t FD,
                          size_t FileSize,
                          size_t MapSize,
                          off_t Offset,
                          তার্কিক RequiresNullTerminator,
                          পূর্ণাঙ্ক PageSize,
                          তার্কিক IsVolatile) {
#যদি defined(__MVS__)
  // zOS Enhanced ASCII auto convert does not support mmap.
  প্রত্যাগম অসত্য;
#endif

  // mmap may leave the buffer without null terminator if the file size changed
  // by the time the last page is mapped in, so avoid it if the file size is
  // likely to change.
  যদি (IsVolatile && RequiresNullTerminator)
    প্রত্যাগম অসত্য;

  // We don't use mmap for small files because this can severely fragment our
  // address space.
  যদি (MapSize < 4 * 4096 || MapSize < (unsigned)PageSize)
    প্রত্যাগম অসত্য;

  যদি (!RequiresNullTerminator)
    প্রত্যাগম সত্য;

  // If we don't know the file size, use fstat to find out.  fstat on an open
  // file descriptor is cheaper than stat on a random path.
  // FIXME: this chunk of code is duplicated, but it avoids a fstat when
  // RequiresNullTerminator = false and MapSize != -1.
  যদি (FileSize == size_t(-1)) {
    sys::fs::file_status Status;
    যদি (sys::fs::status(FD, Status))
      প্রত্যাগম অসত্য;
    FileSize = Status.getSize();
  }

  // If we need a null terminator and the end of the map is inside the file,
  // we cannot use mmap.
  size_t End = Offset + MapSize;
  প্রতিজ্ঞা(End <= FileSize);
  যদি (End != FileSize)
    প্রত্যাগম অসত্য;

  // Don't try to map files that are exactly a multiple of the system page size
  // if we need a null terminator.
  যদি ((FileSize & (PageSize -1)) == 0)
    প্রত্যাগম অসত্য;

#যদি defined(__CYGWIN__)
  // Don't try to map files that are exactly a multiple of the physical page size
  // if we need a null terminator.
  // FIXME: We should reorganize again getPageSize() on Win32.
  যদি ((FileSize & (4096 - 1)) == 0)
    প্রত্যাগম অসত্য;
#endif

  প্রত্যাগম সত্য;
}

স্থির ErrorOr<std::unique_ptr<WriteThroughMemoryBuffer>>
getReadWriteFile(ধ্রুব Twine &Filename, uint64_t FileSize, uint64_t MapSize,
                 uint64_t Offset) {
  Expected<sys::fs::file_t> FDOrErr = sys::fs::openNativeFileForReadWrite(
      Filename, sys::fs::CD_OpenExisting, sys::fs::OF_None);
  যদি (!FDOrErr)
    প্রত্যাগম errorToErrorCode(FDOrErr.takeError());
  sys::fs::file_t FD = *FDOrErr;

  // Default is to map the full file.
  যদি (MapSize == uint64_t(-1)) {
    // If we don't know the file size, use fstat to find out.  fstat on an open
    // file descriptor is cheaper than stat on a random path.
    যদি (FileSize == uint64_t(-1)) {
      sys::fs::file_status Status;
      std::error_code EC = sys::fs::status(FD, Status);
      যদি (EC)
        প্রত্যাগম EC;

      // If this not a file or a block device (e.g. it's a named pipe
      // or character device), we can't mmap it, so error out.
      sys::fs::file_type Type = Status.type();
      যদি (Type != sys::fs::file_type::regular_file &&
          Type != sys::fs::file_type::block_file)
        প্রত্যাগম make_error_code(errc::invalid_argument);

      FileSize = Status.getSize();
    }
    MapSize = FileSize;
  }

  std::error_code EC;
  std::unique_ptr<WriteThroughMemoryBuffer> Result(
      নূতন (NamedBufferAlloc(Filename))
          MemoryBufferMMapFile<WriteThroughMemoryBuffer>(অসত্য, FD, MapSize,
                                                         Offset, EC));
  যদি (EC)
    প্রত্যাগম EC;
  প্রত্যাগম std::move(Result);
}

ErrorOr<std::unique_ptr<WriteThroughMemoryBuffer>>
WriteThroughMemoryBuffer::getFile(ধ্রুব Twine &Filename, int64_t FileSize) {
  sys::sandbox::violationIfEnabled();

  প্রত্যাগম getReadWriteFile(Filename, FileSize, FileSize, 0);
}

/// Map a subrange of the specified file as a WritableMemoryBuffer.
ErrorOr<std::unique_ptr<WriteThroughMemoryBuffer>>
WriteThroughMemoryBuffer::getFileSlice(ধ্রুব Twine &Filename, uint64_t MapSize,
                                       uint64_t Offset) {
  sys::sandbox::violationIfEnabled();

  প্রত্যাগম getReadWriteFile(Filename, -1, MapSize, Offset);
}

template <typename MB>
স্থির ErrorOr<std::unique_ptr<MB>>
getOpenFileImpl(sys::fs::file_t FD, ধ্রুব Twine &Filename, uint64_t FileSize,
                uint64_t MapSize, int64_t Offset, তার্কিক RequiresNullTerminator,
                তার্কিক IsVolatile, std::optional<Align> Alignment) {
  স্থির পূর্ণাঙ্ক PageSize = sys::Process::getPageSizeEstimate();

  // Default is to map the full file.
  যদি (MapSize == uint64_t(-1)) {
    // If we don't know the file size, use fstat to find out.  fstat on an open
    // file descriptor is cheaper than stat on a random path.
    যদি (FileSize == uint64_t(-1)) {
      sys::fs::file_status Status;
      std::error_code EC = sys::fs::status(FD, Status);
      যদি (EC)
        প্রত্যাগম EC;

      // If this not a file or a block device (e.g. it's a named pipe
      // or character device), we can't trust the size. Create the memory
      // buffer by copying off the stream.
      sys::fs::file_type Type = Status.type();
      যদি (Type != sys::fs::file_type::regular_file &&
          Type != sys::fs::file_type::block_file)
        প্রত্যাগম getMemoryBufferForStream(FD, Filename);

      FileSize = Status.getSize();
    }
    MapSize = FileSize;
  }

  যদি (shouldUseMmap(FD, FileSize, MapSize, Offset, RequiresNullTerminator,
                    PageSize, IsVolatile)) {
    std::error_code EC;
    std::unique_ptr<MB> Result(
        নূতন (NamedBufferAlloc(Filename)) MemoryBufferMMapFile<MB>(
            RequiresNullTerminator, FD, MapSize, Offset, EC));
    যদি (!EC) {
      // On at least Linux, and possibly on other systems, mmap may return pages
      // from the page cache that are not properly filled with trailing zeroes,
      // if some prior user of the page wrote non-zero bytes. Detect this and
      // don't use mmap in that case.
      যদি (!RequiresNullTerminator || *Result->getBufferEnd() == '\0')
        প্রত্যাগম std::move(Result);
    }
  }

#ifdef __MVS__
  ErrorOr<তার্কিক> NeedsConversion = needConversion(Filename, FD);
  যদি (std::error_code EC = NeedsConversion.getError())
    প্রত্যাগম EC;
  // File size may increase due to EBCDIC -> UTF-8 conversion, therefore we
  // cannot trust the file size and we create the memory buffer by copying
  // off the stream.
  // Note: This only works with the assumption of reading a full file (i.e,
  // Offset == 0 and MapSize == FileSize). Reading a file slice does not work.
  যদি (*NeedsConversion && Offset == 0 && MapSize == FileSize)
    প্রত্যাগম getMemoryBufferForStream(FD, Filename);
#endif

  auto Buf =
      WritableMemoryBuffer::getNewUninitMemBuffer(MapSize, Filename, Alignment);
  যদি (!Buf) {
    // Failed to create a buffer. The only way it can fail is if
    // new(std::nothrow) returns 0.
    প্রত্যাগম make_error_code(errc::not_enough_memory);
  }

  // Read until EOF, zero-initialize the rest.
  MutableArrayRef<বর্ণ> ToRead = Buf->getBuffer();
  যাবত্ (!ToRead.empty()) {
    Expected<size_t> ReadBytes =
        sys::fs::readNativeFileSlice(FD, ToRead, Offset);
    যদি (!ReadBytes)
      প্রত্যাগম errorToErrorCode(ReadBytes.takeError());
    যদি (*ReadBytes == 0) {
      std::memset(ToRead.data(), 0, ToRead.size());
      ভঙ্গ;
    }
    ToRead = ToRead.drop_front(*ReadBytes);
    Offset += *ReadBytes;
  }

  প্রত্যাগম std::move(Buf);
}

ErrorOr<std::unique_ptr<MemoryBuffer>>
MemoryBuffer::getOpenFile(sys::fs::file_t FD, ধ্রুব Twine &Filename,
                          uint64_t FileSize, তার্কিক RequiresNullTerminator,
                          তার্কিক IsVolatile, std::optional<Align> Alignment) {
  sys::sandbox::violationIfEnabled();

  প্রত্যাগম getOpenFileImpl<MemoryBuffer>(FD, Filename, FileSize, FileSize, 0,
                                       RequiresNullTerminator, IsVolatile,
                                       Alignment);
}

ErrorOr<std::unique_ptr<MemoryBuffer>> MemoryBuffer::getOpenFileSlice(
    sys::fs::file_t FD, ধ্রুব Twine &Filename, uint64_t MapSize, int64_t Offset,
    তার্কিক IsVolatile, std::optional<Align> Alignment) {
  প্রতিজ্ঞা(MapSize != uint64_t(-1));

  sys::sandbox::violationIfEnabled();

  প্রত্যাগম getOpenFileImpl<MemoryBuffer>(FD, Filename, -1, MapSize, Offset, অসত্য,
                                       IsVolatile, Alignment);
}

ErrorOr<std::unique_ptr<MemoryBuffer>> MemoryBuffer::getSTDIN() {
  sys::sandbox::violationIfEnabled();

  // Read in all of the data from stdin, we cannot mmap stdin.
  //
  // FIXME: That isn't necessarily true, we should try to mmap stdin and
  // fallback if it fails.
  sys::ChangeStdinMode(sys::fs::OF_Text);

  প্রত্যাগম getMemoryBufferForStream(sys::fs::getStdinHandle(), "<stdin>");
}

ErrorOr<std::unique_ptr<MemoryBuffer>>
MemoryBuffer::getFileAsStream(ধ্রুব Twine &Filename) {
  sys::sandbox::violationIfEnabled();

  Expected<sys::fs::file_t> FDOrErr =
      sys::fs::openNativeFileForRead(Filename, sys::fs::OF_None);
  যদি (!FDOrErr)
    প্রত্যাগম errorToErrorCode(FDOrErr.takeError());
  sys::fs::file_t FD = *FDOrErr;
  ErrorOr<std::unique_ptr<MemoryBuffer>> Ret =
      getMemoryBufferForStream(FD, Filename);
  sys::fs::closeFile(FD);
  প্রত্যাগম Ret;
}

MemoryBufferRef MemoryBuffer::getMemBufferRef() ধ্রুব {
  StringRef Data = getBuffer();
  StringRef Identifier = getBufferIdentifier();
  প্রত্যাগম MemoryBufferRef(Data, Identifier);
}

SmallVectorMemoryBuffer::~SmallVectorMemoryBuffer() = স্বভাব;
