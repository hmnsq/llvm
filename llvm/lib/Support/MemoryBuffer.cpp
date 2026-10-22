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

#அந்தர்பாவ "llvm/Support/MemoryBuffer.h"
#அந்தர்பாவ "llvm/ADT/STLExtras.h"
#அந்தர்பாவ "llvm/ADT/SmallString.h"
#அந்தர்பாவ "llvm/Config/config.h"
#அந்தர்பாவ "llvm/Support/Alignment.h"
#அந்தர்பாவ "llvm/Support/AutoConvert.h"
#அந்தர்பாவ "llvm/Support/Errc.h"
#அந்தர்பாவ "llvm/Support/Error.h"
#அந்தர்பாவ "llvm/Support/ErrorHandling.h"
#அந்தர்பாவ "llvm/Support/FileSystem.h"
#அந்தர்பாவ "llvm/Support/IOSandbox.h"
#அந்தர்பாவ "llvm/Support/Process.h"
#அந்தர்பாவ "llvm/Support/Program.h"
#அந்தர்பாவ "llvm/Support/SmallVectorMemoryBuffer.h"
#அந்தர்பாவ <algorithm>
#அந்தர்பாவ <cassert>
#அந்தர்பாவ <cstring>
#அந்தர்பாவ <new>
#அந்தர்பாவ <sys/types.h>
#அந்தர்பாவ <system_error>
#எனில் !defined(_MSC_VER) && !defined(__MINGW32__)
#அந்தர்பாவ <unistd.h>
#இல்லையெனில்
#அந்தர்பாவ <io.h>
#endif

using நாமாகாஶ llvm;

//===----------------------------------------------------------------------===//
// MemoryBuffer implementation itself.
//===----------------------------------------------------------------------===//

MemoryBuffer::~MemoryBuffer() = இயல்பு;

/// init - Initialize this MemoryBuffer as a reference to externally allocated
/// memory, memory that we know is already null terminated.
வெறுமை MemoryBuffer::init(மாறிலி எழுத்து *BufStart, மாறிலி எழுத்து *BufEnd,
                        தருக்க RequiresNullTerminator) {
  ப்ரதிஜ்ஞா((!RequiresNullTerminator || BufEnd[0] == 0) &&
         "Buffer is not null terminated!");
  BufferStart = BufStart;
  BufferEnd = BufEnd;
}

//===----------------------------------------------------------------------===//
// MemoryBufferMem implementation.
//===----------------------------------------------------------------------===//

/// CopyStringRef - Copies contents of a StringRef into a block of memory and
/// null-terminates it.
நிலையான வெறுமை CopyStringRef(எழுத்து *Memory, StringRef Data) {
  எனில் (!Data.empty())
    memcpy(Memory, Data.data(), Data.size());
  Memory[Data.size()] = 0; // Null terminate string.
}

நாமாகாஶ {
கட்டமைப்பு NamedBufferAlloc {
  மாறிலி Twine &Name;
  NamedBufferAlloc(மாறிலி Twine &Name) : Name(Name) {}
};
} // namespace

வெறுமை *operator புதிய(size_t N, மாறிலி NamedBufferAlloc &Alloc) {
  SmallString<256> NameBuf;
  StringRef NameRef = Alloc.Name.toStringRef(NameBuf);

  // We use malloc() and manually handle it returning null instead of calling
  // operator new because we need all uses of NamedBufferAlloc to be
  // deallocated with a call to free() due to needing to use malloc() in
  // WritableMemoryBuffer::getNewUninitMemBuffer() to work around the out-of-
  // memory handler installed by default in LLVM. See operator delete() member
  // functions within this file for the paired call to free().
  எழுத்து *Mem =
      static_cast<எழுத்து *>(std::malloc(N + அளவு(size_t) + NameRef.size() + 1));
  எனில் (!Mem)
    llvm::report_bad_alloc_error("Allocation failed");
  *reinterpret_cast<size_t *>(Mem + N) = NameRef.size();
  CopyStringRef(Mem + N + அளவு(size_t), NameRef);
  திரும்பு Mem;
}

நாமாகாஶ {
/// MemoryBufferMem - Named MemoryBuffer pointing to a block of memory.
template<typename MB>
வகுப்பு MemoryBufferMem : பொது MB {
பொது:
  MemoryBufferMem(StringRef உள்ளீடு_Data, தருக்க RequiresNullTerminator) {
    MemoryBuffer::init(உள்ளீடு_Data.begin(), உள்ளீடு_Data.end(),
                       RequiresNullTerminator);
  }

  /// Disable sized deallocation for MemoryBufferMem, because it has
  /// tail-allocated data.
  வெறுமை operator delete(வெறுமை *p) { std::விடுவி(p); }

  StringRef getBufferIdentifier() மாறிலி override {
    // The name is stored after the class itself.
    திரும்பு StringRef(reinterpret_cast<மாறிலி எழுத்து *>(இது + 1) + அளவு(size_t),
                     *reinterpret_cast<மாறிலி size_t *>(இது + 1));
  }

  MemoryBuffer::BufferKind getBufferKind() மாறிலி override {
    திரும்பு MemoryBuffer::MemoryBuffer_Malloc;
  }
};
} // namespace

template <typename MB>
நிலையான ErrorOr<std::unique_ptr<MB>>
getFileAux(மாறிலி Twine &Filename, uint64_t MapSize, uint64_t Offset,
           தருக்க IsText, தருக்க RequiresNullTerminator, தருக்க IsVolatile,
           std::optional<Align> Alignment);

std::unique_ptr<MemoryBuffer>
MemoryBuffer::getMemBuffer(StringRef உள்ளீடு_Data, StringRef BufferName,
                           தருக்க RequiresNullTerminator) {
  auto *Ret = புதிய (NamedBufferAlloc(BufferName))
      MemoryBufferMem<MemoryBuffer>(உள்ளீடு_Data, RequiresNullTerminator);
  திரும்பு std::unique_ptr<MemoryBuffer>(Ret);
}

std::unique_ptr<MemoryBuffer>
MemoryBuffer::getMemBuffer(MemoryBufferRef Ref, தருக்க RequiresNullTerminator) {
  திரும்பு std::unique_ptr<MemoryBuffer>(getMemBuffer(
      Ref.getBuffer(), Ref.getBufferIdentifier(), RequiresNullTerminator));
}

நிலையான ErrorOr<std::unique_ptr<WritableMemoryBuffer>>
getMemBufferCopyImpl(StringRef உள்ளீடு_Data, மாறிலி Twine &BufferName) {
  auto Buf =
      WritableMemoryBuffer::getNewUninitMemBuffer(உள்ளீடு_Data.size(), BufferName);
  எனில் (!Buf)
    திரும்பு make_error_code(errc::not_enough_memory);
  // Calling memcpy with null src/dst is UB, and an empty StringRef is
  // represented with {nullptr, 0}.
  llvm::copy(உள்ளீடு_Data, Buf->getBufferStart());
  திரும்பு std::move(Buf);
}

std::unique_ptr<MemoryBuffer>
MemoryBuffer::getMemBufferCopy(StringRef உள்ளீடு_Data, மாறிலி Twine &BufferName) {
  auto Buf = getMemBufferCopyImpl(உள்ளீடு_Data, BufferName);
  எனில் (Buf)
    திரும்பு std::move(*Buf);
  திரும்பு nullptr;
}

ErrorOr<std::unique_ptr<MemoryBuffer>>
MemoryBuffer::getFileOrSTDIN(மாறிலி Twine &Filename, தருக்க IsText,
                             தருக்க RequiresNullTerminator,
                             std::optional<Align> Alignment) {
  sys::sandbox::violationIfEnabled();

  SmallString<256> NameBuf;
  StringRef NameRef = Filename.toStringRef(NameBuf);

  எனில் (NameRef == "-")
    திரும்பு getSTDIN();
  திரும்பு getFile(Filename, IsText, RequiresNullTerminator,
                 /*IsVolatile=*/பொய், Alignment);
}

ErrorOr<std::unique_ptr<MemoryBuffer>>
MemoryBuffer::getFileSlice(மாறிலி Twine &FilePath, uint64_t MapSize,
                           uint64_t Offset, தருக்க IsVolatile,
                           std::optional<Align> Alignment) {
  sys::sandbox::violationIfEnabled();

  திரும்பு getFileAux<MemoryBuffer>(FilePath, MapSize, Offset, /*IsText=*/பொய்,
                                  /*RequiresNullTerminator=*/பொய், IsVolatile,
                                  Alignment);
}

//===----------------------------------------------------------------------===//
// MemoryBuffer::getFile implementation.
//===----------------------------------------------------------------------===//

நாமாகாஶ {

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
வகுப்பு MemoryBufferMMapFile : பொது MB {
  sys::fs::mapped_file_region MFR;

  நிலையான uint64_t getLegalMapOffset(uint64_t Offset) {
    திரும்பு Offset & ~(sys::fs::mapped_file_region::alignment() - 1);
  }

  நிலையான uint64_t getLegalMapSize(uint64_t Len, uint64_t Offset) {
    திரும்பு Len + (Offset - getLegalMapOffset(Offset));
  }

  மாறிலி எழுத்து *getStart(uint64_t Len, uint64_t Offset) {
    திரும்பு MFR.const_data() + (Offset - getLegalMapOffset(Offset));
  }

பொது:
  MemoryBufferMMapFile(தருக்க RequiresNullTerminator, sys::fs::file_t FD, uint64_t Len,
                       uint64_t Offset, std::error_code &EC)
      : MFR(FD, Mapmode<MB>, getLegalMapSize(Len, Offset),
            getLegalMapOffset(Offset), EC) {
    எனில் (!EC) {
      மாறிலி எழுத்து *Start = getStart(Len, Offset);
      MemoryBuffer::init(Start, Start + Len, RequiresNullTerminator);
    }
  }

  /// Disable sized deallocation for MemoryBufferMMapFile, because it has
  /// tail-allocated data.
  வெறுமை operator delete(வெறுமை *p) { std::விடுவி(p); }

  StringRef getBufferIdentifier() மாறிலி override {
    // The name is stored after the class itself.
    திரும்பு StringRef(reinterpret_cast<மாறிலி எழுத்து *>(இது + 1) + அளவு(size_t),
                     *reinterpret_cast<மாறிலி size_t *>(இது + 1));
  }

  MemoryBuffer::BufferKind getBufferKind() மாறிலி override {
    திரும்பு MemoryBuffer::MemoryBuffer_MMap;
  }

  வெறுமை dontNeedIfMmap() override { MFR.dontNeed(); }
  வெறுமை willNeedIfMmap() override { MFR.willNeed(); }
  வெறுமை randomAccessIfMmap() override { MFR.randomAccess(); }
};
} // namespace

நிலையான ErrorOr<std::unique_ptr<WritableMemoryBuffer>>
getMemoryBufferForStream(sys::fs::file_t FD, மாறிலி Twine &BufferName) {
  SmallString<sys::fs::DefaultReadChunkSize> Buffer;
  எனில் (Error E = sys::fs::readNativeFileToEOF(FD, Buffer))
    திரும்பு errorToErrorCode(std::move(E));
  திரும்பு getMemBufferCopyImpl(Buffer, BufferName);
}

ErrorOr<std::unique_ptr<MemoryBuffer>>
MemoryBuffer::getFile(மாறிலி Twine &Filename, தருக்க IsText,
                      தருக்க RequiresNullTerminator, தருக்க IsVolatile,
                      std::optional<Align> Alignment) {
  sys::sandbox::violationIfEnabled();

  திரும்பு getFileAux<MemoryBuffer>(Filename, /*MapSize=*/-1, /*Offset=*/0,
                                  IsText, RequiresNullTerminator, IsVolatile,
                                  Alignment);
}

template <typename MB>
நிலையான ErrorOr<std::unique_ptr<MB>>
getOpenFileImpl(sys::fs::file_t FD, மாறிலி Twine &Filename, uint64_t FileSize,
                uint64_t MapSize, int64_t Offset, தருக்க RequiresNullTerminator,
                தருக்க IsVolatile, std::optional<Align> Alignment);

template <typename MB>
நிலையான ErrorOr<std::unique_ptr<MB>>
getFileAux(மாறிலி Twine &Filename, uint64_t MapSize, uint64_t Offset,
           தருக்க IsText, தருக்க RequiresNullTerminator, தருக்க IsVolatile,
           std::optional<Align> Alignment) {
  Expected<sys::fs::file_t> FDOrErr = sys::fs::openNativeFileForRead(
      Filename, IsText ? sys::fs::OF_TextWithCRLF : sys::fs::OF_None);
  எனில் (!FDOrErr)
    திரும்பு errorToErrorCode(FDOrErr.takeError());
  sys::fs::file_t FD = *FDOrErr;
  auto Ret = getOpenFileImpl<MB>(FD, Filename, /*FileSize=*/-1, MapSize, Offset,
                                 RequiresNullTerminator, IsVolatile, Alignment);
  sys::fs::closeFile(FD);
  திரும்பு Ret;
}

ErrorOr<std::unique_ptr<WritableMemoryBuffer>>
WritableMemoryBuffer::getFile(மாறிலி Twine &Filename, தருக்க IsVolatile,
                              std::optional<Align> Alignment) {
  sys::sandbox::violationIfEnabled();

  திரும்பு getFileAux<WritableMemoryBuffer>(
      Filename, /*MapSize=*/-1, /*Offset=*/0, /*IsText=*/பொய்,
      /*RequiresNullTerminator=*/பொய், IsVolatile, Alignment);
}

ErrorOr<std::unique_ptr<WritableMemoryBuffer>>
WritableMemoryBuffer::getFileSlice(மாறிலி Twine &Filename, uint64_t MapSize,
                                   uint64_t Offset, தருக்க IsVolatile,
                                   std::optional<Align> Alignment) {
  sys::sandbox::violationIfEnabled();

  திரும்பு getFileAux<WritableMemoryBuffer>(
      Filename, MapSize, Offset, /*IsText=*/பொய்,
      /*RequiresNullTerminator=*/பொய், IsVolatile, Alignment);
}

std::unique_ptr<WritableMemoryBuffer>
WritableMemoryBuffer::getNewUninitMemBuffer(size_t Size,
                                            மாறிலி Twine &BufferName,
                                            std::optional<Align> Alignment) {
  using MemBuffer = MemoryBufferMem<WritableMemoryBuffer>;

  // Use 16-byte alignment if no alignment is specified.
  Align BufAlign = Alignment.மதிப்பு_or(Align(16));

  // Allocate space for the MemoryBuffer, the data and the name. It is important
  // that MemoryBuffer and data are aligned so PointerIntPair works with them.
  SmallString<256> NameBuf;
  StringRef NameRef = BufferName.toStringRef(NameBuf);

  size_t StringLen = அளவு(MemBuffer) + அளவு(size_t) + NameRef.size() + 1;
  size_t RealLen = StringLen + Size + 1 + BufAlign.மதிப்பு_2();
  எனில் (RealLen <= Size) // Check for rollover.
    திரும்பு nullptr;
  // We use a call to malloc() rather than a call to a non-throwing operator
  // new() because LLVM unconditionally installs an out of memory new handler
  // when exceptions are disabled. This new handler intentionally crashes to
  // aid with debugging, but that makes non-throwing new calls unhelpful.
  // See MemoryBufferMem::operator delete() for the paired call to free(), and
  // llvm::install_out_of_memory_new_handler() for the installation of the
  // custom new handler.
  எழுத்து *Mem = static_cast<எழுத்து *>(std::malloc(RealLen));
  எனில் (!Mem)
    திரும்பு nullptr;

  // The name is stored after the class itself.
  *reinterpret_cast<size_t *>(Mem + அளவு(MemBuffer)) = NameRef.size();
  CopyStringRef(Mem + அளவு(MemBuffer) + அளவு(size_t), NameRef);

  // The buffer begins after the name and must be aligned.
  எழுத்து *Buf = (எழுத்து *)alignAddr(Mem + StringLen, BufAlign);
  Buf[Size] = 0; // Null terminate buffer.

  auto *Ret = புதிய (Mem) MemBuffer(StringRef(Buf, Size), உண்மை);
  திரும்பு std::unique_ptr<WritableMemoryBuffer>(Ret);
}

std::unique_ptr<WritableMemoryBuffer>
WritableMemoryBuffer::getNewMemBuffer(size_t Size, மாறிலி Twine &BufferName) {
  auto SB = WritableMemoryBuffer::getNewUninitMemBuffer(Size, BufferName);
  எனில் (!SB)
    திரும்பு nullptr;
  memset(SB->getBufferStart(), 0, Size);
  திரும்பு SB;
}

நிலையான தருக்க shouldUseMmap(sys::fs::file_t FD,
                          size_t FileSize,
                          size_t MapSize,
                          off_t Offset,
                          தருக்க RequiresNullTerminator,
                          முழு PageSize,
                          தருக்க IsVolatile) {
#எனில் defined(__MVS__)
  // zOS Enhanced ASCII auto convert does not support mmap.
  திரும்பு பொய்;
#endif

  // mmap may leave the buffer without null terminator if the file size changed
  // by the time the last page is mapped in, so avoid it if the file size is
  // likely to change.
  எனில் (IsVolatile && RequiresNullTerminator)
    திரும்பு பொய்;

  // We don't use mmap for small files because this can severely fragment our
  // address space.
  எனில் (MapSize < 4 * 4096 || MapSize < (unsigned)PageSize)
    திரும்பு பொய்;

  எனில் (!RequiresNullTerminator)
    திரும்பு உண்மை;

  // If we don't know the file size, use fstat to find out.  fstat on an open
  // file descriptor is cheaper than stat on a random path.
  // FIXME: this chunk of code is duplicated, but it avoids a fstat when
  // RequiresNullTerminator = false and MapSize != -1.
  எனில் (FileSize == size_t(-1)) {
    sys::fs::file_status Status;
    எனில் (sys::fs::status(FD, Status))
      திரும்பு பொய்;
    FileSize = Status.getSize();
  }

  // If we need a null terminator and the end of the map is inside the file,
  // we cannot use mmap.
  size_t End = Offset + MapSize;
  ப்ரதிஜ்ஞா(End <= FileSize);
  எனில் (End != FileSize)
    திரும்பு பொய்;

  // Don't try to map files that are exactly a multiple of the system page size
  // if we need a null terminator.
  எனில் ((FileSize & (PageSize -1)) == 0)
    திரும்பு பொய்;

#எனில் defined(__CYGWIN__)
  // Don't try to map files that are exactly a multiple of the physical page size
  // if we need a null terminator.
  // FIXME: We should reorganize again getPageSize() on Win32.
  எனில் ((FileSize & (4096 - 1)) == 0)
    திரும்பு பொய்;
#endif

  திரும்பு உண்மை;
}

நிலையான ErrorOr<std::unique_ptr<WriteThroughMemoryBuffer>>
getReadWriteFile(மாறிலி Twine &Filename, uint64_t FileSize, uint64_t MapSize,
                 uint64_t Offset) {
  Expected<sys::fs::file_t> FDOrErr = sys::fs::openNativeFileForReadWrite(
      Filename, sys::fs::CD_OpenExisting, sys::fs::OF_None);
  எனில் (!FDOrErr)
    திரும்பு errorToErrorCode(FDOrErr.takeError());
  sys::fs::file_t FD = *FDOrErr;

  // Default is to map the full file.
  எனில் (MapSize == uint64_t(-1)) {
    // If we don't know the file size, use fstat to find out.  fstat on an open
    // file descriptor is cheaper than stat on a random path.
    எனில் (FileSize == uint64_t(-1)) {
      sys::fs::file_status Status;
      std::error_code EC = sys::fs::status(FD, Status);
      எனில் (EC)
        திரும்பு EC;

      // If this not a file or a block device (e.g. it's a named pipe
      // or character device), we can't mmap it, so error out.
      sys::fs::file_type Type = Status.type();
      எனில் (Type != sys::fs::file_type::regular_file &&
          Type != sys::fs::file_type::block_file)
        திரும்பு make_error_code(errc::invalid_argument);

      FileSize = Status.getSize();
    }
    MapSize = FileSize;
  }

  std::error_code EC;
  std::unique_ptr<WriteThroughMemoryBuffer> Result(
      புதிய (NamedBufferAlloc(Filename))
          MemoryBufferMMapFile<WriteThroughMemoryBuffer>(பொய், FD, MapSize,
                                                         Offset, EC));
  எனில் (EC)
    திரும்பு EC;
  திரும்பு std::move(Result);
}

ErrorOr<std::unique_ptr<WriteThroughMemoryBuffer>>
WriteThroughMemoryBuffer::getFile(மாறிலி Twine &Filename, int64_t FileSize) {
  sys::sandbox::violationIfEnabled();

  திரும்பு getReadWriteFile(Filename, FileSize, FileSize, 0);
}

/// Map a subrange of the specified file as a WritableMemoryBuffer.
ErrorOr<std::unique_ptr<WriteThroughMemoryBuffer>>
WriteThroughMemoryBuffer::getFileSlice(மாறிலி Twine &Filename, uint64_t MapSize,
                                       uint64_t Offset) {
  sys::sandbox::violationIfEnabled();

  திரும்பு getReadWriteFile(Filename, -1, MapSize, Offset);
}

template <typename MB>
நிலையான ErrorOr<std::unique_ptr<MB>>
getOpenFileImpl(sys::fs::file_t FD, மாறிலி Twine &Filename, uint64_t FileSize,
                uint64_t MapSize, int64_t Offset, தருக்க RequiresNullTerminator,
                தருக்க IsVolatile, std::optional<Align> Alignment) {
  நிலையான முழு PageSize = sys::Process::getPageSizeEstimate();

  // Default is to map the full file.
  எனில் (MapSize == uint64_t(-1)) {
    // If we don't know the file size, use fstat to find out.  fstat on an open
    // file descriptor is cheaper than stat on a random path.
    எனில் (FileSize == uint64_t(-1)) {
      sys::fs::file_status Status;
      std::error_code EC = sys::fs::status(FD, Status);
      எனில் (EC)
        திரும்பு EC;

      // If this not a file or a block device (e.g. it's a named pipe
      // or character device), we can't trust the size. Create the memory
      // buffer by copying off the stream.
      sys::fs::file_type Type = Status.type();
      எனில் (Type != sys::fs::file_type::regular_file &&
          Type != sys::fs::file_type::block_file)
        திரும்பு getMemoryBufferForStream(FD, Filename);

      FileSize = Status.getSize();
    }
    MapSize = FileSize;
  }

  எனில் (shouldUseMmap(FD, FileSize, MapSize, Offset, RequiresNullTerminator,
                    PageSize, IsVolatile)) {
    std::error_code EC;
    std::unique_ptr<MB> Result(
        புதிய (NamedBufferAlloc(Filename)) MemoryBufferMMapFile<MB>(
            RequiresNullTerminator, FD, MapSize, Offset, EC));
    எனில் (!EC) {
      // On at least Linux, and possibly on other systems, mmap may return pages
      // from the page cache that are not properly filled with trailing zeroes,
      // if some prior user of the page wrote non-zero bytes. Detect this and
      // don't use mmap in that case.
      எனில் (!RequiresNullTerminator || *Result->getBufferEnd() == '\0')
        திரும்பு std::move(Result);
    }
  }

#ifdef __MVS__
  ErrorOr<தருக்க> NeedsConversion = needConversion(Filename, FD);
  எனில் (std::error_code EC = NeedsConversion.getError())
    திரும்பு EC;
  // File size may increase due to EBCDIC -> UTF-8 conversion, therefore we
  // cannot trust the file size and we create the memory buffer by copying
  // off the stream.
  // Note: This only works with the assumption of reading a full file (i.e,
  // Offset == 0 and MapSize == FileSize). Reading a file slice does not work.
  எனில் (*NeedsConversion && Offset == 0 && MapSize == FileSize)
    திரும்பு getMemoryBufferForStream(FD, Filename);
#endif

  auto Buf =
      WritableMemoryBuffer::getNewUninitMemBuffer(MapSize, Filename, Alignment);
  எனில் (!Buf) {
    // Failed to create a buffer. The only way it can fail is if
    // new(std::nothrow) returns 0.
    திரும்பு make_error_code(errc::not_enough_memory);
  }

  // Read until EOF, zero-initialize the rest.
  MutableArrayRef<எழுத்து> ToRead = Buf->getBuffer();
  போது (!ToRead.empty()) {
    Expected<size_t> ReadBytes =
        sys::fs::readNativeFileSlice(FD, ToRead, Offset);
    எனில் (!ReadBytes)
      திரும்பு errorToErrorCode(ReadBytes.takeError());
    எனில் (*ReadBytes == 0) {
      std::memset(ToRead.data(), 0, ToRead.size());
      நிறுத்து;
    }
    ToRead = ToRead.drop_front(*ReadBytes);
    Offset += *ReadBytes;
  }

  திரும்பு std::move(Buf);
}

ErrorOr<std::unique_ptr<MemoryBuffer>>
MemoryBuffer::getOpenFile(sys::fs::file_t FD, மாறிலி Twine &Filename,
                          uint64_t FileSize, தருக்க RequiresNullTerminator,
                          தருக்க IsVolatile, std::optional<Align> Alignment) {
  sys::sandbox::violationIfEnabled();

  திரும்பு getOpenFileImpl<MemoryBuffer>(FD, Filename, FileSize, FileSize, 0,
                                       RequiresNullTerminator, IsVolatile,
                                       Alignment);
}

ErrorOr<std::unique_ptr<MemoryBuffer>> MemoryBuffer::getOpenFileSlice(
    sys::fs::file_t FD, மாறிலி Twine &Filename, uint64_t MapSize, int64_t Offset,
    தருக்க IsVolatile, std::optional<Align> Alignment) {
  ப்ரதிஜ்ஞா(MapSize != uint64_t(-1));

  sys::sandbox::violationIfEnabled();

  திரும்பு getOpenFileImpl<MemoryBuffer>(FD, Filename, -1, MapSize, Offset, பொய்,
                                       IsVolatile, Alignment);
}

ErrorOr<std::unique_ptr<MemoryBuffer>> MemoryBuffer::getSTDIN() {
  sys::sandbox::violationIfEnabled();

  // Read in all of the data from stdin, we cannot mmap stdin.
  //
  // FIXME: That isn't necessarily true, we should try to mmap stdin and
  // fallback if it fails.
  sys::ChangeStdinMode(sys::fs::OF_Text);

  திரும்பு getMemoryBufferForStream(sys::fs::getStdinHandle(), "<stdin>");
}

ErrorOr<std::unique_ptr<MemoryBuffer>>
MemoryBuffer::getFileAsStream(மாறிலி Twine &Filename) {
  sys::sandbox::violationIfEnabled();

  Expected<sys::fs::file_t> FDOrErr =
      sys::fs::openNativeFileForRead(Filename, sys::fs::OF_None);
  எனில் (!FDOrErr)
    திரும்பு errorToErrorCode(FDOrErr.takeError());
  sys::fs::file_t FD = *FDOrErr;
  ErrorOr<std::unique_ptr<MemoryBuffer>> Ret =
      getMemoryBufferForStream(FD, Filename);
  sys::fs::closeFile(FD);
  திரும்பு Ret;
}

MemoryBufferRef MemoryBuffer::getMemBufferRef() மாறிலி {
  StringRef Data = getBuffer();
  StringRef Identifier = getBufferIdentifier();
  திரும்பு MemoryBufferRef(Data, Identifier);
}

SmallVectorMemoryBuffer::~SmallVectorMemoryBuffer() = இயல்பு;
