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

#અન્તર્ભાવ "llvm/Support/MemoryBuffer.h"
#અન્તર્ભાવ "llvm/ADT/STLExtras.h"
#અન્તર્ભાવ "llvm/ADT/SmallString.h"
#અન્તર્ભાવ "llvm/Config/config.h"
#અન્તર્ભાવ "llvm/Support/Alignment.h"
#અન્તર્ભાવ "llvm/Support/AutoConvert.h"
#અન્તર્ભાવ "llvm/Support/Errc.h"
#અન્તર્ભાવ "llvm/Support/Error.h"
#અન્તર્ભાવ "llvm/Support/ErrorHandling.h"
#અન્તર્ભાવ "llvm/Support/FileSystem.h"
#અન્તર્ભાવ "llvm/Support/IOSandbox.h"
#અન્તર્ભાવ "llvm/Support/Process.h"
#અન્તર્ભાવ "llvm/Support/Program.h"
#અન્તર્ભાવ "llvm/Support/SmallVectorMemoryBuffer.h"
#અન્તર્ભાવ <algorithm>
#અન્તર્ભાવ <cassert>
#અન્તર્ભાવ <cstring>
#અન્તર્ભાવ <new>
#અન્તર્ભાવ <sys/types.h>
#અન્તર્ભાવ <system_error>
#જો !defined(_MSC_VER) && !defined(__MINGW32__)
#અન્તર્ભાવ <unistd.h>
#નહીંતર
#અન્તર્ભાવ <io.h>
#endif

using નામાકાશ llvm;

//===----------------------------------------------------------------------===//
// MemoryBuffer implementation itself.
//===----------------------------------------------------------------------===//

MemoryBuffer::~MemoryBuffer() = મૂળભૂત;

/// init - Initialize this MemoryBuffer as a reference to externally allocated
/// memory, memory that we know is already null terminated.
શૂન્ય MemoryBuffer::init(અચલ અક્ષર *BufStart, અચલ અક્ષર *BufEnd,
                        તાર્કિક RequiresNullTerminator) {
  પ્રતિજ્ઞા((!RequiresNullTerminator || BufEnd[0] == 0) &&
         "Buffer is not null terminated!");
  BufferStart = BufStart;
  BufferEnd = BufEnd;
}

//===----------------------------------------------------------------------===//
// MemoryBufferMem implementation.
//===----------------------------------------------------------------------===//

/// CopyStringRef - Copies contents of a StringRef into a block of memory and
/// null-terminates it.
સ્થિર શૂન્ય CopyStringRef(અક્ષર *Memory, StringRef Data) {
  જો (!Data.empty())
    memcpy(Memory, Data.data(), Data.size());
  Memory[Data.size()] = 0; // Null terminate string.
}

નામાકાશ {
સંરચના NamedBufferAlloc {
  અચલ Twine &Name;
  NamedBufferAlloc(અચલ Twine &Name) : Name(Name) {}
};
} // namespace

શૂન્ય *operator નવું(size_t N, અચલ NamedBufferAlloc &Alloc) {
  SmallString<256> NameBuf;
  StringRef NameRef = Alloc.Name.toStringRef(NameBuf);

  // We use malloc() and manually handle it returning null instead of calling
  // operator new because we need all uses of NamedBufferAlloc to be
  // deallocated with a call to free() due to needing to use malloc() in
  // WritableMemoryBuffer::getNewUninitMemBuffer() to work around the out-of-
  // memory handler installed by default in LLVM. See operator delete() member
  // functions within this file for the paired call to free().
  અક્ષર *Mem =
      static_cast<અક્ષર *>(std::malloc(N + કદ(size_t) + NameRef.size() + 1));
  જો (!Mem)
    llvm::report_bad_alloc_error("Allocation failed");
  *reinterpret_cast<size_t *>(Mem + N) = NameRef.size();
  CopyStringRef(Mem + N + કદ(size_t), NameRef);
  પાછા Mem;
}

નામાકાશ {
/// MemoryBufferMem - Named MemoryBuffer pointing to a block of memory.
template<typename MB>
વર્ગ MemoryBufferMem : જાહેર MB {
જાહેર:
  MemoryBufferMem(StringRef આગત_Data, તાર્કિક RequiresNullTerminator) {
    MemoryBuffer::init(આગત_Data.begin(), આગત_Data.end(),
                       RequiresNullTerminator);
  }

  /// Disable sized deallocation for MemoryBufferMem, because it has
  /// tail-allocated data.
  શૂન્ય operator delete(શૂન્ય *p) { std::મુક્ત(p); }

  StringRef getBufferIdentifier() અચલ override {
    // The name is stored after the class itself.
    પાછા StringRef(reinterpret_cast<અચલ અક્ષર *>(આ + 1) + કદ(size_t),
                     *reinterpret_cast<અચલ size_t *>(આ + 1));
  }

  MemoryBuffer::BufferKind getBufferKind() અચલ override {
    પાછા MemoryBuffer::MemoryBuffer_Malloc;
  }
};
} // namespace

template <typename MB>
સ્થિર ErrorOr<std::unique_ptr<MB>>
getFileAux(અચલ Twine &Filename, uint64_t MapSize, uint64_t Offset,
           તાર્કિક IsText, તાર્કિક RequiresNullTerminator, તાર્કિક IsVolatile,
           std::optional<Align> Alignment);

std::unique_ptr<MemoryBuffer>
MemoryBuffer::getMemBuffer(StringRef આગત_Data, StringRef BufferName,
                           તાર્કિક RequiresNullTerminator) {
  auto *Ret = નવું (NamedBufferAlloc(BufferName))
      MemoryBufferMem<MemoryBuffer>(આગત_Data, RequiresNullTerminator);
  પાછા std::unique_ptr<MemoryBuffer>(Ret);
}

std::unique_ptr<MemoryBuffer>
MemoryBuffer::getMemBuffer(MemoryBufferRef Ref, તાર્કિક RequiresNullTerminator) {
  પાછા std::unique_ptr<MemoryBuffer>(getMemBuffer(
      Ref.getBuffer(), Ref.getBufferIdentifier(), RequiresNullTerminator));
}

સ્થિર ErrorOr<std::unique_ptr<WritableMemoryBuffer>>
getMemBufferCopyImpl(StringRef આગત_Data, અચલ Twine &BufferName) {
  auto Buf =
      WritableMemoryBuffer::getNewUninitMemBuffer(આગત_Data.size(), BufferName);
  જો (!Buf)
    પાછા make_error_code(errc::not_enough_memory);
  // Calling memcpy with null src/dst is UB, and an empty StringRef is
  // represented with {nullptr, 0}.
  llvm::copy(આગત_Data, Buf->getBufferStart());
  પાછા std::move(Buf);
}

std::unique_ptr<MemoryBuffer>
MemoryBuffer::getMemBufferCopy(StringRef આગત_Data, અચલ Twine &BufferName) {
  auto Buf = getMemBufferCopyImpl(આગત_Data, BufferName);
  જો (Buf)
    પાછા std::move(*Buf);
  પાછા nullptr;
}

ErrorOr<std::unique_ptr<MemoryBuffer>>
MemoryBuffer::getFileOrSTDIN(અચલ Twine &Filename, તાર્કિક IsText,
                             તાર્કિક RequiresNullTerminator,
                             std::optional<Align> Alignment) {
  sys::sandbox::violationIfEnabled();

  SmallString<256> NameBuf;
  StringRef NameRef = Filename.toStringRef(NameBuf);

  જો (NameRef == "-")
    પાછા getSTDIN();
  પાછા getFile(Filename, IsText, RequiresNullTerminator,
                 /*IsVolatile=*/ખોટું, Alignment);
}

ErrorOr<std::unique_ptr<MemoryBuffer>>
MemoryBuffer::getFileSlice(અચલ Twine &FilePath, uint64_t MapSize,
                           uint64_t Offset, તાર્કિક IsVolatile,
                           std::optional<Align> Alignment) {
  sys::sandbox::violationIfEnabled();

  પાછા getFileAux<MemoryBuffer>(FilePath, MapSize, Offset, /*IsText=*/ખોટું,
                                  /*RequiresNullTerminator=*/ખોટું, IsVolatile,
                                  Alignment);
}

//===----------------------------------------------------------------------===//
// MemoryBuffer::getFile implementation.
//===----------------------------------------------------------------------===//

નામાકાશ {

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
વર્ગ MemoryBufferMMapFile : જાહેર MB {
  sys::fs::mapped_file_region MFR;

  સ્થિર uint64_t getLegalMapOffset(uint64_t Offset) {
    પાછા Offset & ~(sys::fs::mapped_file_region::alignment() - 1);
  }

  સ્થિર uint64_t getLegalMapSize(uint64_t Len, uint64_t Offset) {
    પાછા Len + (Offset - getLegalMapOffset(Offset));
  }

  અચલ અક્ષર *getStart(uint64_t Len, uint64_t Offset) {
    પાછા MFR.const_data() + (Offset - getLegalMapOffset(Offset));
  }

જાહેર:
  MemoryBufferMMapFile(તાર્કિક RequiresNullTerminator, sys::fs::file_t FD, uint64_t Len,
                       uint64_t Offset, std::error_code &EC)
      : MFR(FD, Mapmode<MB>, getLegalMapSize(Len, Offset),
            getLegalMapOffset(Offset), EC) {
    જો (!EC) {
      અચલ અક્ષર *Start = getStart(Len, Offset);
      MemoryBuffer::init(Start, Start + Len, RequiresNullTerminator);
    }
  }

  /// Disable sized deallocation for MemoryBufferMMapFile, because it has
  /// tail-allocated data.
  શૂન્ય operator delete(શૂન્ય *p) { std::મુક્ત(p); }

  StringRef getBufferIdentifier() અચલ override {
    // The name is stored after the class itself.
    પાછા StringRef(reinterpret_cast<અચલ અક્ષર *>(આ + 1) + કદ(size_t),
                     *reinterpret_cast<અચલ size_t *>(આ + 1));
  }

  MemoryBuffer::BufferKind getBufferKind() અચલ override {
    પાછા MemoryBuffer::MemoryBuffer_MMap;
  }

  શૂન્ય dontNeedIfMmap() override { MFR.dontNeed(); }
  શૂન્ય willNeedIfMmap() override { MFR.willNeed(); }
  શૂન્ય randomAccessIfMmap() override { MFR.randomAccess(); }
};
} // namespace

સ્થિર ErrorOr<std::unique_ptr<WritableMemoryBuffer>>
getMemoryBufferForStream(sys::fs::file_t FD, અચલ Twine &BufferName) {
  SmallString<sys::fs::DefaultReadChunkSize> Buffer;
  જો (Error E = sys::fs::readNativeFileToEOF(FD, Buffer))
    પાછા errorToErrorCode(std::move(E));
  પાછા getMemBufferCopyImpl(Buffer, BufferName);
}

ErrorOr<std::unique_ptr<MemoryBuffer>>
MemoryBuffer::getFile(અચલ Twine &Filename, તાર્કિક IsText,
                      તાર્કિક RequiresNullTerminator, તાર્કિક IsVolatile,
                      std::optional<Align> Alignment) {
  sys::sandbox::violationIfEnabled();

  પાછા getFileAux<MemoryBuffer>(Filename, /*MapSize=*/-1, /*Offset=*/0,
                                  IsText, RequiresNullTerminator, IsVolatile,
                                  Alignment);
}

template <typename MB>
સ્થિર ErrorOr<std::unique_ptr<MB>>
getOpenFileImpl(sys::fs::file_t FD, અચલ Twine &Filename, uint64_t FileSize,
                uint64_t MapSize, int64_t Offset, તાર્કિક RequiresNullTerminator,
                તાર્કિક IsVolatile, std::optional<Align> Alignment);

template <typename MB>
સ્થિર ErrorOr<std::unique_ptr<MB>>
getFileAux(અચલ Twine &Filename, uint64_t MapSize, uint64_t Offset,
           તાર્કિક IsText, તાર્કિક RequiresNullTerminator, તાર્કિક IsVolatile,
           std::optional<Align> Alignment) {
  Expected<sys::fs::file_t> FDOrErr = sys::fs::openNativeFileForRead(
      Filename, IsText ? sys::fs::OF_TextWithCRLF : sys::fs::OF_None);
  જો (!FDOrErr)
    પાછા errorToErrorCode(FDOrErr.takeError());
  sys::fs::file_t FD = *FDOrErr;
  auto Ret = getOpenFileImpl<MB>(FD, Filename, /*FileSize=*/-1, MapSize, Offset,
                                 RequiresNullTerminator, IsVolatile, Alignment);
  sys::fs::closeFile(FD);
  પાછા Ret;
}

ErrorOr<std::unique_ptr<WritableMemoryBuffer>>
WritableMemoryBuffer::getFile(અચલ Twine &Filename, તાર્કિક IsVolatile,
                              std::optional<Align> Alignment) {
  sys::sandbox::violationIfEnabled();

  પાછા getFileAux<WritableMemoryBuffer>(
      Filename, /*MapSize=*/-1, /*Offset=*/0, /*IsText=*/ખોટું,
      /*RequiresNullTerminator=*/ખોટું, IsVolatile, Alignment);
}

ErrorOr<std::unique_ptr<WritableMemoryBuffer>>
WritableMemoryBuffer::getFileSlice(અચલ Twine &Filename, uint64_t MapSize,
                                   uint64_t Offset, તાર્કિક IsVolatile,
                                   std::optional<Align> Alignment) {
  sys::sandbox::violationIfEnabled();

  પાછા getFileAux<WritableMemoryBuffer>(
      Filename, MapSize, Offset, /*IsText=*/ખોટું,
      /*RequiresNullTerminator=*/ખોટું, IsVolatile, Alignment);
}

std::unique_ptr<WritableMemoryBuffer>
WritableMemoryBuffer::getNewUninitMemBuffer(size_t Size,
                                            અચલ Twine &BufferName,
                                            std::optional<Align> Alignment) {
  using MemBuffer = MemoryBufferMem<WritableMemoryBuffer>;

  // Use 16-byte alignment if no alignment is specified.
  Align BufAlign = Alignment.કિંમત_or(Align(16));

  // Allocate space for the MemoryBuffer, the data and the name. It is important
  // that MemoryBuffer and data are aligned so PointerIntPair works with them.
  SmallString<256> NameBuf;
  StringRef NameRef = BufferName.toStringRef(NameBuf);

  size_t StringLen = કદ(MemBuffer) + કદ(size_t) + NameRef.size() + 1;
  size_t RealLen = StringLen + Size + 1 + BufAlign.કિંમત_2();
  જો (RealLen <= Size) // Check for rollover.
    પાછા nullptr;
  // We use a call to malloc() rather than a call to a non-throwing operator
  // new() because LLVM unconditionally installs an out of memory new handler
  // when exceptions are disabled. This new handler intentionally crashes to
  // aid with debugging, but that makes non-throwing new calls unhelpful.
  // See MemoryBufferMem::operator delete() for the paired call to free(), and
  // llvm::install_out_of_memory_new_handler() for the installation of the
  // custom new handler.
  અક્ષર *Mem = static_cast<અક્ષર *>(std::malloc(RealLen));
  જો (!Mem)
    પાછા nullptr;

  // The name is stored after the class itself.
  *reinterpret_cast<size_t *>(Mem + કદ(MemBuffer)) = NameRef.size();
  CopyStringRef(Mem + કદ(MemBuffer) + કદ(size_t), NameRef);

  // The buffer begins after the name and must be aligned.
  અક્ષર *Buf = (અક્ષર *)alignAddr(Mem + StringLen, BufAlign);
  Buf[Size] = 0; // Null terminate buffer.

  auto *Ret = નવું (Mem) MemBuffer(StringRef(Buf, Size), સાચું);
  પાછા std::unique_ptr<WritableMemoryBuffer>(Ret);
}

std::unique_ptr<WritableMemoryBuffer>
WritableMemoryBuffer::getNewMemBuffer(size_t Size, અચલ Twine &BufferName) {
  auto SB = WritableMemoryBuffer::getNewUninitMemBuffer(Size, BufferName);
  જો (!SB)
    પાછા nullptr;
  memset(SB->getBufferStart(), 0, Size);
  પાછા SB;
}

સ્થિર તાર્કિક shouldUseMmap(sys::fs::file_t FD,
                          size_t FileSize,
                          size_t MapSize,
                          off_t Offset,
                          તાર્કિક RequiresNullTerminator,
                          પૂર્ણાંક PageSize,
                          તાર્કિક IsVolatile) {
#જો defined(__MVS__)
  // zOS Enhanced ASCII auto convert does not support mmap.
  પાછા ખોટું;
#endif

  // mmap may leave the buffer without null terminator if the file size changed
  // by the time the last page is mapped in, so avoid it if the file size is
  // likely to change.
  જો (IsVolatile && RequiresNullTerminator)
    પાછા ખોટું;

  // We don't use mmap for small files because this can severely fragment our
  // address space.
  જો (MapSize < 4 * 4096 || MapSize < (unsigned)PageSize)
    પાછા ખોટું;

  જો (!RequiresNullTerminator)
    પાછા સાચું;

  // If we don't know the file size, use fstat to find out.  fstat on an open
  // file descriptor is cheaper than stat on a random path.
  // FIXME: this chunk of code is duplicated, but it avoids a fstat when
  // RequiresNullTerminator = false and MapSize != -1.
  જો (FileSize == size_t(-1)) {
    sys::fs::file_status Status;
    જો (sys::fs::status(FD, Status))
      પાછા ખોટું;
    FileSize = Status.getSize();
  }

  // If we need a null terminator and the end of the map is inside the file,
  // we cannot use mmap.
  size_t End = Offset + MapSize;
  પ્રતિજ્ઞા(End <= FileSize);
  જો (End != FileSize)
    પાછા ખોટું;

  // Don't try to map files that are exactly a multiple of the system page size
  // if we need a null terminator.
  જો ((FileSize & (PageSize -1)) == 0)
    પાછા ખોટું;

#જો defined(__CYGWIN__)
  // Don't try to map files that are exactly a multiple of the physical page size
  // if we need a null terminator.
  // FIXME: We should reorganize again getPageSize() on Win32.
  જો ((FileSize & (4096 - 1)) == 0)
    પાછા ખોટું;
#endif

  પાછા સાચું;
}

સ્થિર ErrorOr<std::unique_ptr<WriteThroughMemoryBuffer>>
getReadWriteFile(અચલ Twine &Filename, uint64_t FileSize, uint64_t MapSize,
                 uint64_t Offset) {
  Expected<sys::fs::file_t> FDOrErr = sys::fs::openNativeFileForReadWrite(
      Filename, sys::fs::CD_OpenExisting, sys::fs::OF_None);
  જો (!FDOrErr)
    પાછા errorToErrorCode(FDOrErr.takeError());
  sys::fs::file_t FD = *FDOrErr;

  // Default is to map the full file.
  જો (MapSize == uint64_t(-1)) {
    // If we don't know the file size, use fstat to find out.  fstat on an open
    // file descriptor is cheaper than stat on a random path.
    જો (FileSize == uint64_t(-1)) {
      sys::fs::file_status Status;
      std::error_code EC = sys::fs::status(FD, Status);
      જો (EC)
        પાછા EC;

      // If this not a file or a block device (e.g. it's a named pipe
      // or character device), we can't mmap it, so error out.
      sys::fs::file_type Type = Status.type();
      જો (Type != sys::fs::file_type::regular_file &&
          Type != sys::fs::file_type::block_file)
        પાછા make_error_code(errc::invalid_argument);

      FileSize = Status.getSize();
    }
    MapSize = FileSize;
  }

  std::error_code EC;
  std::unique_ptr<WriteThroughMemoryBuffer> Result(
      નવું (NamedBufferAlloc(Filename))
          MemoryBufferMMapFile<WriteThroughMemoryBuffer>(ખોટું, FD, MapSize,
                                                         Offset, EC));
  જો (EC)
    પાછા EC;
  પાછા std::move(Result);
}

ErrorOr<std::unique_ptr<WriteThroughMemoryBuffer>>
WriteThroughMemoryBuffer::getFile(અચલ Twine &Filename, int64_t FileSize) {
  sys::sandbox::violationIfEnabled();

  પાછા getReadWriteFile(Filename, FileSize, FileSize, 0);
}

/// Map a subrange of the specified file as a WritableMemoryBuffer.
ErrorOr<std::unique_ptr<WriteThroughMemoryBuffer>>
WriteThroughMemoryBuffer::getFileSlice(અચલ Twine &Filename, uint64_t MapSize,
                                       uint64_t Offset) {
  sys::sandbox::violationIfEnabled();

  પાછા getReadWriteFile(Filename, -1, MapSize, Offset);
}

template <typename MB>
સ્થિર ErrorOr<std::unique_ptr<MB>>
getOpenFileImpl(sys::fs::file_t FD, અચલ Twine &Filename, uint64_t FileSize,
                uint64_t MapSize, int64_t Offset, તાર્કિક RequiresNullTerminator,
                તાર્કિક IsVolatile, std::optional<Align> Alignment) {
  સ્થિર પૂર્ણાંક PageSize = sys::Process::getPageSizeEstimate();

  // Default is to map the full file.
  જો (MapSize == uint64_t(-1)) {
    // If we don't know the file size, use fstat to find out.  fstat on an open
    // file descriptor is cheaper than stat on a random path.
    જો (FileSize == uint64_t(-1)) {
      sys::fs::file_status Status;
      std::error_code EC = sys::fs::status(FD, Status);
      જો (EC)
        પાછા EC;

      // If this not a file or a block device (e.g. it's a named pipe
      // or character device), we can't trust the size. Create the memory
      // buffer by copying off the stream.
      sys::fs::file_type Type = Status.type();
      જો (Type != sys::fs::file_type::regular_file &&
          Type != sys::fs::file_type::block_file)
        પાછા getMemoryBufferForStream(FD, Filename);

      FileSize = Status.getSize();
    }
    MapSize = FileSize;
  }

  જો (shouldUseMmap(FD, FileSize, MapSize, Offset, RequiresNullTerminator,
                    PageSize, IsVolatile)) {
    std::error_code EC;
    std::unique_ptr<MB> Result(
        નવું (NamedBufferAlloc(Filename)) MemoryBufferMMapFile<MB>(
            RequiresNullTerminator, FD, MapSize, Offset, EC));
    જો (!EC) {
      // On at least Linux, and possibly on other systems, mmap may return pages
      // from the page cache that are not properly filled with trailing zeroes,
      // if some prior user of the page wrote non-zero bytes. Detect this and
      // don't use mmap in that case.
      જો (!RequiresNullTerminator || *Result->getBufferEnd() == '\0')
        પાછા std::move(Result);
    }
  }

#ifdef __MVS__
  ErrorOr<તાર્કિક> NeedsConversion = needConversion(Filename, FD);
  જો (std::error_code EC = NeedsConversion.getError())
    પાછા EC;
  // File size may increase due to EBCDIC -> UTF-8 conversion, therefore we
  // cannot trust the file size and we create the memory buffer by copying
  // off the stream.
  // Note: This only works with the assumption of reading a full file (i.e,
  // Offset == 0 and MapSize == FileSize). Reading a file slice does not work.
  જો (*NeedsConversion && Offset == 0 && MapSize == FileSize)
    પાછા getMemoryBufferForStream(FD, Filename);
#endif

  auto Buf =
      WritableMemoryBuffer::getNewUninitMemBuffer(MapSize, Filename, Alignment);
  જો (!Buf) {
    // Failed to create a buffer. The only way it can fail is if
    // new(std::nothrow) returns 0.
    પાછા make_error_code(errc::not_enough_memory);
  }

  // Read until EOF, zero-initialize the rest.
  MutableArrayRef<અક્ષર> ToRead = Buf->getBuffer();
  જ્યારે (!ToRead.empty()) {
    Expected<size_t> ReadBytes =
        sys::fs::readNativeFileSlice(FD, ToRead, Offset);
    જો (!ReadBytes)
      પાછા errorToErrorCode(ReadBytes.takeError());
    જો (*ReadBytes == 0) {
      std::memset(ToRead.data(), 0, ToRead.size());
      તોડો;
    }
    ToRead = ToRead.drop_front(*ReadBytes);
    Offset += *ReadBytes;
  }

  પાછા std::move(Buf);
}

ErrorOr<std::unique_ptr<MemoryBuffer>>
MemoryBuffer::getOpenFile(sys::fs::file_t FD, અચલ Twine &Filename,
                          uint64_t FileSize, તાર્કિક RequiresNullTerminator,
                          તાર્કિક IsVolatile, std::optional<Align> Alignment) {
  sys::sandbox::violationIfEnabled();

  પાછા getOpenFileImpl<MemoryBuffer>(FD, Filename, FileSize, FileSize, 0,
                                       RequiresNullTerminator, IsVolatile,
                                       Alignment);
}

ErrorOr<std::unique_ptr<MemoryBuffer>> MemoryBuffer::getOpenFileSlice(
    sys::fs::file_t FD, અચલ Twine &Filename, uint64_t MapSize, int64_t Offset,
    તાર્કિક IsVolatile, std::optional<Align> Alignment) {
  પ્રતિજ્ઞા(MapSize != uint64_t(-1));

  sys::sandbox::violationIfEnabled();

  પાછા getOpenFileImpl<MemoryBuffer>(FD, Filename, -1, MapSize, Offset, ખોટું,
                                       IsVolatile, Alignment);
}

ErrorOr<std::unique_ptr<MemoryBuffer>> MemoryBuffer::getSTDIN() {
  sys::sandbox::violationIfEnabled();

  // Read in all of the data from stdin, we cannot mmap stdin.
  //
  // FIXME: That isn't necessarily true, we should try to mmap stdin and
  // fallback if it fails.
  sys::ChangeStdinMode(sys::fs::OF_Text);

  પાછા getMemoryBufferForStream(sys::fs::getStdinHandle(), "<stdin>");
}

ErrorOr<std::unique_ptr<MemoryBuffer>>
MemoryBuffer::getFileAsStream(અચલ Twine &Filename) {
  sys::sandbox::violationIfEnabled();

  Expected<sys::fs::file_t> FDOrErr =
      sys::fs::openNativeFileForRead(Filename, sys::fs::OF_None);
  જો (!FDOrErr)
    પાછા errorToErrorCode(FDOrErr.takeError());
  sys::fs::file_t FD = *FDOrErr;
  ErrorOr<std::unique_ptr<MemoryBuffer>> Ret =
      getMemoryBufferForStream(FD, Filename);
  sys::fs::closeFile(FD);
  પાછા Ret;
}

MemoryBufferRef MemoryBuffer::getMemBufferRef() અચલ {
  StringRef Data = getBuffer();
  StringRef Identifier = getBufferIdentifier();
  પાછા MemoryBufferRef(Data, Identifier);
}

SmallVectorMemoryBuffer::~SmallVectorMemoryBuffer() = મૂળભૂત;
