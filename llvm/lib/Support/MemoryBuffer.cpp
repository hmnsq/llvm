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

#अन्तर्भाव "llvm/Support/MemoryBuffer.h"
#अन्तर्भाव "llvm/ADT/STLExtras.h"
#अन्तर्भाव "llvm/ADT/SmallString.h"
#अन्तर्भाव "llvm/Config/config.h"
#अन्तर्भाव "llvm/Support/Alignment.h"
#अन्तर्भाव "llvm/Support/AutoConvert.h"
#अन्तर्भाव "llvm/Support/Errc.h"
#अन्तर्भाव "llvm/Support/Error.h"
#अन्तर्भाव "llvm/Support/ErrorHandling.h"
#अन्तर्भाव "llvm/Support/FileSystem.h"
#अन्तर्भाव "llvm/Support/IOSandbox.h"
#अन्तर्भाव "llvm/Support/Process.h"
#अन्तर्भाव "llvm/Support/Program.h"
#अन्तर्भाव "llvm/Support/SmallVectorMemoryBuffer.h"
#अन्तर्भाव <algorithm>
#अन्तर्भाव <cassert>
#अन्तर्भाव <cstring>
#अन्तर्भाव <new>
#अन्तर्भाव <sys/types.h>
#अन्तर्भाव <system_error>
#सचे !defined(_MSC_VER) && !defined(__MINGW32__)
#अन्तर्भाव <unistd.h>
#अञ्ञथा
#अन्तर्भाव <io.h>
#endif

using नामाकाश llvm;

//===----------------------------------------------------------------------===//
// MemoryBuffer implementation itself.
//===----------------------------------------------------------------------===//

MemoryBuffer::~MemoryBuffer() = स्वभाव;

/// init - Initialize this MemoryBuffer as a reference to externally allocated
/// memory, memory that we know is already null terminated.
शून्य MemoryBuffer::init(ध्रुव वर्ण *BufStart, ध्रुव वर्ण *BufEnd,
                        तार्किक RequiresNullTerminator) {
  प्रतिज्ञा((!RequiresNullTerminator || BufEnd[0] == 0) &&
         "Buffer is not null terminated!");
  BufferStart = BufStart;
  BufferEnd = BufEnd;
}

//===----------------------------------------------------------------------===//
// MemoryBufferMem implementation.
//===----------------------------------------------------------------------===//

/// CopyStringRef - Copies contents of a StringRef into a block of memory and
/// null-terminates it.
स्थिर शून्य CopyStringRef(वर्ण *Memory, StringRef Data) {
  सचे (!Data.empty())
    memcpy(Memory, Data.data(), Data.size());
  Memory[Data.size()] = 0; // Null terminate string.
}

नामाकाश {
संरचना NamedBufferAlloc {
  ध्रुव Twine &Name;
  NamedBufferAlloc(ध्रुव Twine &Name) : Name(Name) {}
};
} // namespace

शून्य *operator नूतन(size_t N, ध्रुव NamedBufferAlloc &Alloc) {
  SmallString<256> NameBuf;
  StringRef NameRef = Alloc.Name.toStringRef(NameBuf);

  // We use malloc() and manually handle it returning null instead of calling
  // operator new because we need all uses of NamedBufferAlloc to be
  // deallocated with a call to free() due to needing to use malloc() in
  // WritableMemoryBuffer::getNewUninitMemBuffer() to work around the out-of-
  // memory handler installed by default in LLVM. See operator delete() member
  // functions within this file for the paired call to free().
  वर्ण *Mem =
      static_cast<वर्ण *>(std::malloc(N + परिमाण(size_t) + NameRef.size() + 1));
  सचे (!Mem)
    llvm::report_bad_alloc_error("Allocation failed");
  *reinterpret_cast<size_t *>(Mem + N) = NameRef.size();
  CopyStringRef(Mem + N + परिमाण(size_t), NameRef);
  पटिनिवत्त Mem;
}

नामाकाश {
/// MemoryBufferMem - Named MemoryBuffer pointing to a block of memory.
template<typename MB>
वग्ग MemoryBufferMem : सार्वजनिक MB {
सार्वजनिक:
  MemoryBufferMem(StringRef निवेशः_Data, तार्किक RequiresNullTerminator) {
    MemoryBuffer::init(निवेशः_Data.begin(), निवेशः_Data.end(),
                       RequiresNullTerminator);
  }

  /// Disable sized deallocation for MemoryBufferMem, because it has
  /// tail-allocated data.
  शून्य operator delete(शून्य *p) { std::मुक्त(p); }

  StringRef getBufferIdentifier() ध्रुव override {
    // The name is stored after the class itself.
    पटिनिवत्त StringRef(reinterpret_cast<ध्रुव वर्ण *>(एतत् + 1) + परिमाण(size_t),
                     *reinterpret_cast<ध्रुव size_t *>(एतत् + 1));
  }

  MemoryBuffer::BufferKind getBufferKind() ध्रुव override {
    पटिनिवत्त MemoryBuffer::MemoryBuffer_Malloc;
  }
};
} // namespace

template <typename MB>
स्थिर ErrorOr<std::unique_ptr<MB>>
getFileAux(ध्रुव Twine &Filename, uint64_t MapSize, uint64_t Offset,
           तार्किक IsText, तार्किक RequiresNullTerminator, तार्किक IsVolatile,
           std::optional<Align> Alignment);

std::unique_ptr<MemoryBuffer>
MemoryBuffer::getMemBuffer(StringRef निवेशः_Data, StringRef BufferName,
                           तार्किक RequiresNullTerminator) {
  auto *Ret = नूतन (NamedBufferAlloc(BufferName))
      MemoryBufferMem<MemoryBuffer>(निवेशः_Data, RequiresNullTerminator);
  पटिनिवत्त std::unique_ptr<MemoryBuffer>(Ret);
}

std::unique_ptr<MemoryBuffer>
MemoryBuffer::getMemBuffer(MemoryBufferRef Ref, तार्किक RequiresNullTerminator) {
  पटिनिवत्त std::unique_ptr<MemoryBuffer>(getMemBuffer(
      Ref.getBuffer(), Ref.getBufferIdentifier(), RequiresNullTerminator));
}

स्थिर ErrorOr<std::unique_ptr<WritableMemoryBuffer>>
getMemBufferCopyImpl(StringRef निवेशः_Data, ध्रुव Twine &BufferName) {
  auto Buf =
      WritableMemoryBuffer::getNewUninitMemBuffer(निवेशः_Data.size(), BufferName);
  सचे (!Buf)
    पटिनिवत्त make_error_code(errc::not_enough_memory);
  // Calling memcpy with null src/dst is UB, and an empty StringRef is
  // represented with {nullptr, 0}.
  llvm::copy(निवेशः_Data, Buf->getBufferStart());
  पटिनिवत्त std::move(Buf);
}

std::unique_ptr<MemoryBuffer>
MemoryBuffer::getMemBufferCopy(StringRef निवेशः_Data, ध्रुव Twine &BufferName) {
  auto Buf = getMemBufferCopyImpl(निवेशः_Data, BufferName);
  सचे (Buf)
    पटिनिवत्त std::move(*Buf);
  पटिनिवत्त nullptr;
}

ErrorOr<std::unique_ptr<MemoryBuffer>>
MemoryBuffer::getFileOrSTDIN(ध्रुव Twine &Filename, तार्किक IsText,
                             तार्किक RequiresNullTerminator,
                             std::optional<Align> Alignment) {
  sys::sandbox::violationIfEnabled();

  SmallString<256> NameBuf;
  StringRef NameRef = Filename.toStringRef(NameBuf);

  सचे (NameRef == "-")
    पटिनिवत्त getSTDIN();
  पटिनिवत्त getFile(Filename, IsText, RequiresNullTerminator,
                 /*IsVolatile=*/मिच्छा, Alignment);
}

ErrorOr<std::unique_ptr<MemoryBuffer>>
MemoryBuffer::getFileSlice(ध्रुव Twine &FilePath, uint64_t MapSize,
                           uint64_t Offset, तार्किक IsVolatile,
                           std::optional<Align> Alignment) {
  sys::sandbox::violationIfEnabled();

  पटिनिवत्त getFileAux<MemoryBuffer>(FilePath, MapSize, Offset, /*IsText=*/मिच्छा,
                                  /*RequiresNullTerminator=*/मिच्छा, IsVolatile,
                                  Alignment);
}

//===----------------------------------------------------------------------===//
// MemoryBuffer::getFile implementation.
//===----------------------------------------------------------------------===//

नामाकाश {

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
वग्ग MemoryBufferMMapFile : सार्वजनिक MB {
  sys::fs::mapped_file_region MFR;

  स्थिर uint64_t getLegalMapOffset(uint64_t Offset) {
    पटिनिवत्त Offset & ~(sys::fs::mapped_file_region::alignment() - 1);
  }

  स्थिर uint64_t getLegalMapSize(uint64_t Len, uint64_t Offset) {
    पटिनिवत्त Len + (Offset - getLegalMapOffset(Offset));
  }

  ध्रुव वर्ण *getStart(uint64_t Len, uint64_t Offset) {
    पटिनिवत्त MFR.const_data() + (Offset - getLegalMapOffset(Offset));
  }

सार्वजनिक:
  MemoryBufferMMapFile(तार्किक RequiresNullTerminator, sys::fs::file_t FD, uint64_t Len,
                       uint64_t Offset, std::error_code &EC)
      : MFR(FD, Mapmode<MB>, getLegalMapSize(Len, Offset),
            getLegalMapOffset(Offset), EC) {
    सचे (!EC) {
      ध्रुव वर्ण *Start = getStart(Len, Offset);
      MemoryBuffer::init(Start, Start + Len, RequiresNullTerminator);
    }
  }

  /// Disable sized deallocation for MemoryBufferMMapFile, because it has
  /// tail-allocated data.
  शून्य operator delete(शून्य *p) { std::मुक्त(p); }

  StringRef getBufferIdentifier() ध्रुव override {
    // The name is stored after the class itself.
    पटिनिवत्त StringRef(reinterpret_cast<ध्रुव वर्ण *>(एतत् + 1) + परिमाण(size_t),
                     *reinterpret_cast<ध्रुव size_t *>(एतत् + 1));
  }

  MemoryBuffer::BufferKind getBufferKind() ध्रुव override {
    पटिनिवत्त MemoryBuffer::MemoryBuffer_MMap;
  }

  शून्य dontNeedIfMmap() override { MFR.dontNeed(); }
  शून्य willNeedIfMmap() override { MFR.willNeed(); }
  शून्य randomAccessIfMmap() override { MFR.randomAccess(); }
};
} // namespace

स्थिर ErrorOr<std::unique_ptr<WritableMemoryBuffer>>
getMemoryBufferForStream(sys::fs::file_t FD, ध्रुव Twine &BufferName) {
  SmallString<sys::fs::DefaultReadChunkSize> Buffer;
  सचे (Error E = sys::fs::readNativeFileToEOF(FD, Buffer))
    पटिनिवत्त errorToErrorCode(std::move(E));
  पटिनिवत्त getMemBufferCopyImpl(Buffer, BufferName);
}

ErrorOr<std::unique_ptr<MemoryBuffer>>
MemoryBuffer::getFile(ध्रुव Twine &Filename, तार्किक IsText,
                      तार्किक RequiresNullTerminator, तार्किक IsVolatile,
                      std::optional<Align> Alignment) {
  sys::sandbox::violationIfEnabled();

  पटिनिवत्त getFileAux<MemoryBuffer>(Filename, /*MapSize=*/-1, /*Offset=*/0,
                                  IsText, RequiresNullTerminator, IsVolatile,
                                  Alignment);
}

template <typename MB>
स्थिर ErrorOr<std::unique_ptr<MB>>
getOpenFileImpl(sys::fs::file_t FD, ध्रुव Twine &Filename, uint64_t FileSize,
                uint64_t MapSize, int64_t Offset, तार्किक RequiresNullTerminator,
                तार्किक IsVolatile, std::optional<Align> Alignment);

template <typename MB>
स्थिर ErrorOr<std::unique_ptr<MB>>
getFileAux(ध्रुव Twine &Filename, uint64_t MapSize, uint64_t Offset,
           तार्किक IsText, तार्किक RequiresNullTerminator, तार्किक IsVolatile,
           std::optional<Align> Alignment) {
  Expected<sys::fs::file_t> FDOrErr = sys::fs::openNativeFileForRead(
      Filename, IsText ? sys::fs::OF_TextWithCRLF : sys::fs::OF_None);
  सचे (!FDOrErr)
    पटिनिवत्त errorToErrorCode(FDOrErr.takeError());
  sys::fs::file_t FD = *FDOrErr;
  auto Ret = getOpenFileImpl<MB>(FD, Filename, /*FileSize=*/-1, MapSize, Offset,
                                 RequiresNullTerminator, IsVolatile, Alignment);
  sys::fs::closeFile(FD);
  पटिनिवत्त Ret;
}

ErrorOr<std::unique_ptr<WritableMemoryBuffer>>
WritableMemoryBuffer::getFile(ध्रुव Twine &Filename, तार्किक IsVolatile,
                              std::optional<Align> Alignment) {
  sys::sandbox::violationIfEnabled();

  पटिनिवत्त getFileAux<WritableMemoryBuffer>(
      Filename, /*MapSize=*/-1, /*Offset=*/0, /*IsText=*/मिच्छा,
      /*RequiresNullTerminator=*/मिच्छा, IsVolatile, Alignment);
}

ErrorOr<std::unique_ptr<WritableMemoryBuffer>>
WritableMemoryBuffer::getFileSlice(ध्रुव Twine &Filename, uint64_t MapSize,
                                   uint64_t Offset, तार्किक IsVolatile,
                                   std::optional<Align> Alignment) {
  sys::sandbox::violationIfEnabled();

  पटिनिवत्त getFileAux<WritableMemoryBuffer>(
      Filename, MapSize, Offset, /*IsText=*/मिच्छा,
      /*RequiresNullTerminator=*/मिच्छा, IsVolatile, Alignment);
}

std::unique_ptr<WritableMemoryBuffer>
WritableMemoryBuffer::getNewUninitMemBuffer(size_t Size,
                                            ध्रुव Twine &BufferName,
                                            std::optional<Align> Alignment) {
  using MemBuffer = MemoryBufferMem<WritableMemoryBuffer>;

  // Use 16-byte alignment if no alignment is specified.
  Align BufAlign = Alignment.मानम्_or(Align(16));

  // Allocate space for the MemoryBuffer, the data and the name. It is important
  // that MemoryBuffer and data are aligned so PointerIntPair works with them.
  SmallString<256> NameBuf;
  StringRef NameRef = BufferName.toStringRef(NameBuf);

  size_t StringLen = परिमाण(MemBuffer) + परिमाण(size_t) + NameRef.size() + 1;
  size_t RealLen = StringLen + Size + 1 + BufAlign.मानम्_2();
  सचे (RealLen <= Size) // Check for rollover.
    पटिनिवत्त nullptr;
  // We use a call to malloc() rather than a call to a non-throwing operator
  // new() because LLVM unconditionally installs an out of memory new handler
  // when exceptions are disabled. This new handler intentionally crashes to
  // aid with debugging, but that makes non-throwing new calls unhelpful.
  // See MemoryBufferMem::operator delete() for the paired call to free(), and
  // llvm::install_out_of_memory_new_handler() for the installation of the
  // custom new handler.
  वर्ण *Mem = static_cast<वर्ण *>(std::malloc(RealLen));
  सचे (!Mem)
    पटिनिवत्त nullptr;

  // The name is stored after the class itself.
  *reinterpret_cast<size_t *>(Mem + परिमाण(MemBuffer)) = NameRef.size();
  CopyStringRef(Mem + परिमाण(MemBuffer) + परिमाण(size_t), NameRef);

  // The buffer begins after the name and must be aligned.
  वर्ण *Buf = (वर्ण *)alignAddr(Mem + StringLen, BufAlign);
  Buf[Size] = 0; // Null terminate buffer.

  auto *Ret = नूतन (Mem) MemBuffer(StringRef(Buf, Size), सच्च);
  पटिनिवत्त std::unique_ptr<WritableMemoryBuffer>(Ret);
}

std::unique_ptr<WritableMemoryBuffer>
WritableMemoryBuffer::getNewMemBuffer(size_t Size, ध्रुव Twine &BufferName) {
  auto SB = WritableMemoryBuffer::getNewUninitMemBuffer(Size, BufferName);
  सचे (!SB)
    पटिनिवत्त nullptr;
  memset(SB->getBufferStart(), 0, Size);
  पटिनिवत्त SB;
}

स्थिर तार्किक shouldUseMmap(sys::fs::file_t FD,
                          size_t FileSize,
                          size_t MapSize,
                          off_t Offset,
                          तार्किक RequiresNullTerminator,
                          पूर्णाङ्क PageSize,
                          तार्किक IsVolatile) {
#सचे defined(__MVS__)
  // zOS Enhanced ASCII auto convert does not support mmap.
  पटिनिवत्त मिच्छा;
#endif

  // mmap may leave the buffer without null terminator if the file size changed
  // by the time the last page is mapped in, so avoid it if the file size is
  // likely to change.
  सचे (IsVolatile && RequiresNullTerminator)
    पटिनिवत्त मिच्छा;

  // We don't use mmap for small files because this can severely fragment our
  // address space.
  सचे (MapSize < 4 * 4096 || MapSize < (unsigned)PageSize)
    पटिनिवत्त मिच्छा;

  सचे (!RequiresNullTerminator)
    पटिनिवत्त सच्च;

  // If we don't know the file size, use fstat to find out.  fstat on an open
  // file descriptor is cheaper than stat on a random path.
  // FIXME: this chunk of code is duplicated, but it avoids a fstat when
  // RequiresNullTerminator = false and MapSize != -1.
  सचे (FileSize == size_t(-1)) {
    sys::fs::file_status Status;
    सचे (sys::fs::status(FD, Status))
      पटिनिवत्त मिच्छा;
    FileSize = Status.getSize();
  }

  // If we need a null terminator and the end of the map is inside the file,
  // we cannot use mmap.
  size_t End = Offset + MapSize;
  प्रतिज्ञा(End <= FileSize);
  सचे (End != FileSize)
    पटिनिवत्त मिच्छा;

  // Don't try to map files that are exactly a multiple of the system page size
  // if we need a null terminator.
  सचे ((FileSize & (PageSize -1)) == 0)
    पटिनिवत्त मिच्छा;

#सचे defined(__CYGWIN__)
  // Don't try to map files that are exactly a multiple of the physical page size
  // if we need a null terminator.
  // FIXME: We should reorganize again getPageSize() on Win32.
  सचे ((FileSize & (4096 - 1)) == 0)
    पटिनिवत्त मिच्छा;
#endif

  पटिनिवत्त सच्च;
}

स्थिर ErrorOr<std::unique_ptr<WriteThroughMemoryBuffer>>
getReadWriteFile(ध्रुव Twine &Filename, uint64_t FileSize, uint64_t MapSize,
                 uint64_t Offset) {
  Expected<sys::fs::file_t> FDOrErr = sys::fs::openNativeFileForReadWrite(
      Filename, sys::fs::CD_OpenExisting, sys::fs::OF_None);
  सचे (!FDOrErr)
    पटिनिवत्त errorToErrorCode(FDOrErr.takeError());
  sys::fs::file_t FD = *FDOrErr;

  // Default is to map the full file.
  सचे (MapSize == uint64_t(-1)) {
    // If we don't know the file size, use fstat to find out.  fstat on an open
    // file descriptor is cheaper than stat on a random path.
    सचे (FileSize == uint64_t(-1)) {
      sys::fs::file_status Status;
      std::error_code EC = sys::fs::status(FD, Status);
      सचे (EC)
        पटिनिवत्त EC;

      // If this not a file or a block device (e.g. it's a named pipe
      // or character device), we can't mmap it, so error out.
      sys::fs::file_type Type = Status.type();
      सचे (Type != sys::fs::file_type::regular_file &&
          Type != sys::fs::file_type::block_file)
        पटिनिवत्त make_error_code(errc::invalid_argument);

      FileSize = Status.getSize();
    }
    MapSize = FileSize;
  }

  std::error_code EC;
  std::unique_ptr<WriteThroughMemoryBuffer> Result(
      नूतन (NamedBufferAlloc(Filename))
          MemoryBufferMMapFile<WriteThroughMemoryBuffer>(मिच्छा, FD, MapSize,
                                                         Offset, EC));
  सचे (EC)
    पटिनिवत्त EC;
  पटिनिवत्त std::move(Result);
}

ErrorOr<std::unique_ptr<WriteThroughMemoryBuffer>>
WriteThroughMemoryBuffer::getFile(ध्रुव Twine &Filename, int64_t FileSize) {
  sys::sandbox::violationIfEnabled();

  पटिनिवत्त getReadWriteFile(Filename, FileSize, FileSize, 0);
}

/// Map a subrange of the specified file as a WritableMemoryBuffer.
ErrorOr<std::unique_ptr<WriteThroughMemoryBuffer>>
WriteThroughMemoryBuffer::getFileSlice(ध्रुव Twine &Filename, uint64_t MapSize,
                                       uint64_t Offset) {
  sys::sandbox::violationIfEnabled();

  पटिनिवत्त getReadWriteFile(Filename, -1, MapSize, Offset);
}

template <typename MB>
स्थिर ErrorOr<std::unique_ptr<MB>>
getOpenFileImpl(sys::fs::file_t FD, ध्रुव Twine &Filename, uint64_t FileSize,
                uint64_t MapSize, int64_t Offset, तार्किक RequiresNullTerminator,
                तार्किक IsVolatile, std::optional<Align> Alignment) {
  स्थिर पूर्णाङ्क PageSize = sys::Process::getPageSizeEstimate();

  // Default is to map the full file.
  सचे (MapSize == uint64_t(-1)) {
    // If we don't know the file size, use fstat to find out.  fstat on an open
    // file descriptor is cheaper than stat on a random path.
    सचे (FileSize == uint64_t(-1)) {
      sys::fs::file_status Status;
      std::error_code EC = sys::fs::status(FD, Status);
      सचे (EC)
        पटिनिवत्त EC;

      // If this not a file or a block device (e.g. it's a named pipe
      // or character device), we can't trust the size. Create the memory
      // buffer by copying off the stream.
      sys::fs::file_type Type = Status.type();
      सचे (Type != sys::fs::file_type::regular_file &&
          Type != sys::fs::file_type::block_file)
        पटिनिवत्त getMemoryBufferForStream(FD, Filename);

      FileSize = Status.getSize();
    }
    MapSize = FileSize;
  }

  सचे (shouldUseMmap(FD, FileSize, MapSize, Offset, RequiresNullTerminator,
                    PageSize, IsVolatile)) {
    std::error_code EC;
    std::unique_ptr<MB> Result(
        नूतन (NamedBufferAlloc(Filename)) MemoryBufferMMapFile<MB>(
            RequiresNullTerminator, FD, MapSize, Offset, EC));
    सचे (!EC) {
      // On at least Linux, and possibly on other systems, mmap may return pages
      // from the page cache that are not properly filled with trailing zeroes,
      // if some prior user of the page wrote non-zero bytes. Detect this and
      // don't use mmap in that case.
      सचे (!RequiresNullTerminator || *Result->getBufferEnd() == '\0')
        पटिनिवत्त std::move(Result);
    }
  }

#ifdef __MVS__
  ErrorOr<तार्किक> NeedsConversion = needConversion(Filename, FD);
  सचे (std::error_code EC = NeedsConversion.getError())
    पटिनिवत्त EC;
  // File size may increase due to EBCDIC -> UTF-8 conversion, therefore we
  // cannot trust the file size and we create the memory buffer by copying
  // off the stream.
  // Note: This only works with the assumption of reading a full file (i.e,
  // Offset == 0 and MapSize == FileSize). Reading a file slice does not work.
  सचे (*NeedsConversion && Offset == 0 && MapSize == FileSize)
    पटिनिवत्त getMemoryBufferForStream(FD, Filename);
#endif

  auto Buf =
      WritableMemoryBuffer::getNewUninitMemBuffer(MapSize, Filename, Alignment);
  सचे (!Buf) {
    // Failed to create a buffer. The only way it can fail is if
    // new(std::nothrow) returns 0.
    पटिनिवत्त make_error_code(errc::not_enough_memory);
  }

  // Read until EOF, zero-initialize the rest.
  MutableArrayRef<वर्ण> ToRead = Buf->getBuffer();
  याव (!ToRead.empty()) {
    Expected<size_t> ReadBytes =
        sys::fs::readNativeFileSlice(FD, ToRead, Offset);
    सचे (!ReadBytes)
      पटिनिवत्त errorToErrorCode(ReadBytes.takeError());
    सचे (*ReadBytes == 0) {
      std::memset(ToRead.data(), 0, ToRead.size());
      भङ्ग;
    }
    ToRead = ToRead.drop_front(*ReadBytes);
    Offset += *ReadBytes;
  }

  पटिनिवत्त std::move(Buf);
}

ErrorOr<std::unique_ptr<MemoryBuffer>>
MemoryBuffer::getOpenFile(sys::fs::file_t FD, ध्रुव Twine &Filename,
                          uint64_t FileSize, तार्किक RequiresNullTerminator,
                          तार्किक IsVolatile, std::optional<Align> Alignment) {
  sys::sandbox::violationIfEnabled();

  पटिनिवत्त getOpenFileImpl<MemoryBuffer>(FD, Filename, FileSize, FileSize, 0,
                                       RequiresNullTerminator, IsVolatile,
                                       Alignment);
}

ErrorOr<std::unique_ptr<MemoryBuffer>> MemoryBuffer::getOpenFileSlice(
    sys::fs::file_t FD, ध्रुव Twine &Filename, uint64_t MapSize, int64_t Offset,
    तार्किक IsVolatile, std::optional<Align> Alignment) {
  प्रतिज्ञा(MapSize != uint64_t(-1));

  sys::sandbox::violationIfEnabled();

  पटिनिवत्त getOpenFileImpl<MemoryBuffer>(FD, Filename, -1, MapSize, Offset, मिच्छा,
                                       IsVolatile, Alignment);
}

ErrorOr<std::unique_ptr<MemoryBuffer>> MemoryBuffer::getSTDIN() {
  sys::sandbox::violationIfEnabled();

  // Read in all of the data from stdin, we cannot mmap stdin.
  //
  // FIXME: That isn't necessarily true, we should try to mmap stdin and
  // fallback if it fails.
  sys::ChangeStdinMode(sys::fs::OF_Text);

  पटिनिवत्त getMemoryBufferForStream(sys::fs::getStdinHandle(), "<stdin>");
}

ErrorOr<std::unique_ptr<MemoryBuffer>>
MemoryBuffer::getFileAsStream(ध्रुव Twine &Filename) {
  sys::sandbox::violationIfEnabled();

  Expected<sys::fs::file_t> FDOrErr =
      sys::fs::openNativeFileForRead(Filename, sys::fs::OF_None);
  सचे (!FDOrErr)
    पटिनिवत्त errorToErrorCode(FDOrErr.takeError());
  sys::fs::file_t FD = *FDOrErr;
  ErrorOr<std::unique_ptr<MemoryBuffer>> Ret =
      getMemoryBufferForStream(FD, Filename);
  sys::fs::closeFile(FD);
  पटिनिवत्त Ret;
}

MemoryBufferRef MemoryBuffer::getMemBufferRef() ध्रुव {
  StringRef Data = getBuffer();
  StringRef Identifier = getBufferIdentifier();
  पटिनिवत्त MemoryBufferRef(Data, Identifier);
}

SmallVectorMemoryBuffer::~SmallVectorMemoryBuffer() = स्वभाव;
