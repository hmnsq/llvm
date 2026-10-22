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
#وگرنه
#شامل <io.h>
#endif

using فضاینام llvm;

//===----------------------------------------------------------------------===//
// MemoryBuffer implementation itself.
//===----------------------------------------------------------------------===//

MemoryBuffer::~MemoryBuffer() = پیشفرض;

/// init - Initialize this MemoryBuffer as a reference to externally allocated
/// memory, memory that we know is already null terminated.
تهی MemoryBuffer::init(ثابت نویسه *BufStart, ثابت نویسه *BufEnd,
                        منطقی RequiresNullTerminator) {
  أكد((!RequiresNullTerminator || BufEnd[0] == 0) &&
         "Buffer is not null terminated!");
  BufferStart = BufStart;
  BufferEnd = BufEnd;
}

//===----------------------------------------------------------------------===//
// MemoryBufferMem implementation.
//===----------------------------------------------------------------------===//

/// CopyStringRef - Copies contents of a StringRef into a block of memory and
/// null-terminates it.
ایستا تهی CopyStringRef(نویسه *Memory, StringRef Data) {
  اگر (!Data.empty())
    memcpy(Memory, Data.data(), Data.size());
  Memory[Data.size()] = 0; // Null terminate string.
}

فضاینام {
ساختار NamedBufferAlloc {
  ثابت Twine &Name;
  NamedBufferAlloc(ثابت Twine &Name) : Name(Name) {}
};
} // namespace

تهی *operator جدید(size_t N, ثابت NamedBufferAlloc &Alloc) {
  SmallString<256> NameBuf;
  StringRef NameRef = Alloc.Name.toStringRef(NameBuf);

  // We use malloc() and manually handle it returning null instead of calling
  // operator new because we need all uses of NamedBufferAlloc to be
  // deallocated with a call to free() due to needing to use malloc() in
  // WritableMemoryBuffer::getNewUninitMemBuffer() to work around the out-of-
  // memory handler installed by default in LLVM. See operator delete() member
  // functions within this file for the paired call to free().
  نویسه *Mem =
      static_cast<نویسه *>(std::malloc(N + اندازه(size_t) + NameRef.size() + 1));
  اگر (!Mem)
    llvm::report_bad_alloc_error("Allocation failed");
  *reinterpret_cast<size_t *>(Mem + N) = NameRef.size();
  CopyStringRef(Mem + N + اندازه(size_t), NameRef);
  بازگشت Mem;
}

فضاینام {
/// MemoryBufferMem - Named MemoryBuffer pointing to a block of memory.
template<typename MB>
رده MemoryBufferMem : عمومی MB {
عمومی:
  MemoryBufferMem(StringRef درون_داد_Data, منطقی RequiresNullTerminator) {
    MemoryBuffer::init(درون_داد_Data.begin(), درون_داد_Data.end(),
                       RequiresNullTerminator);
  }

  /// Disable sized deallocation for MemoryBufferMem, because it has
  /// tail-allocated data.
  تهی operator delete(تهی *p) { std::آزاد(p); }

  StringRef getBufferIdentifier() ثابت override {
    // The name is stored after the class itself.
    بازگشت StringRef(reinterpret_cast<ثابت نویسه *>(این + 1) + اندازه(size_t),
                     *reinterpret_cast<ثابت size_t *>(این + 1));
  }

  MemoryBuffer::BufferKind getBufferKind() ثابت override {
    بازگشت MemoryBuffer::MemoryBuffer_Malloc;
  }
};
} // namespace

template <typename MB>
ایستا ErrorOr<std::unique_ptr<MB>>
getFileAux(ثابت Twine &Filename, uint64_t MapSize, uint64_t Offset,
           منطقی IsText, منطقی RequiresNullTerminator, منطقی IsVolatile,
           std::optional<Align> Alignment);

std::unique_ptr<MemoryBuffer>
MemoryBuffer::getMemBuffer(StringRef درون_داد_Data, StringRef BufferName,
                           منطقی RequiresNullTerminator) {
  auto *Ret = جدید (NamedBufferAlloc(BufferName))
      MemoryBufferMem<MemoryBuffer>(درون_داد_Data, RequiresNullTerminator);
  بازگشت std::unique_ptr<MemoryBuffer>(Ret);
}

std::unique_ptr<MemoryBuffer>
MemoryBuffer::getMemBuffer(MemoryBufferRef Ref, منطقی RequiresNullTerminator) {
  بازگشت std::unique_ptr<MemoryBuffer>(getMemBuffer(
      Ref.getBuffer(), Ref.getBufferIdentifier(), RequiresNullTerminator));
}

ایستا ErrorOr<std::unique_ptr<WritableMemoryBuffer>>
getMemBufferCopyImpl(StringRef درون_داد_Data, ثابت Twine &BufferName) {
  auto Buf =
      WritableMemoryBuffer::getNewUninitMemBuffer(درون_داد_Data.size(), BufferName);
  اگر (!Buf)
    بازگشت make_error_code(errc::not_enough_memory);
  // Calling memcpy with null src/dst is UB, and an empty StringRef is
  // represented with {nullptr, 0}.
  llvm::copy(درون_داد_Data, Buf->getBufferStart());
  بازگشت std::move(Buf);
}

std::unique_ptr<MemoryBuffer>
MemoryBuffer::getMemBufferCopy(StringRef درون_داد_Data, ثابت Twine &BufferName) {
  auto Buf = getMemBufferCopyImpl(درون_داد_Data, BufferName);
  اگر (Buf)
    بازگشت std::move(*Buf);
  بازگشت nullptr;
}

ErrorOr<std::unique_ptr<MemoryBuffer>>
MemoryBuffer::getFileOrSTDIN(ثابت Twine &Filename, منطقی IsText,
                             منطقی RequiresNullTerminator,
                             std::optional<Align> Alignment) {
  sys::sandbox::violationIfEnabled();

  SmallString<256> NameBuf;
  StringRef NameRef = Filename.toStringRef(NameBuf);

  اگر (NameRef == "-")
    بازگشت getSTDIN();
  بازگشت getFile(Filename, IsText, RequiresNullTerminator,
                 /*IsVolatile=*/نادرست, Alignment);
}

ErrorOr<std::unique_ptr<MemoryBuffer>>
MemoryBuffer::getFileSlice(ثابت Twine &FilePath, uint64_t MapSize,
                           uint64_t Offset, منطقی IsVolatile,
                           std::optional<Align> Alignment) {
  sys::sandbox::violationIfEnabled();

  بازگشت getFileAux<MemoryBuffer>(FilePath, MapSize, Offset, /*IsText=*/نادرست,
                                  /*RequiresNullTerminator=*/نادرست, IsVolatile,
                                  Alignment);
}

//===----------------------------------------------------------------------===//
// MemoryBuffer::getFile implementation.
//===----------------------------------------------------------------------===//

فضاینام {

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
رده MemoryBufferMMapFile : عمومی MB {
  sys::fs::mapped_file_region MFR;

  ایستا uint64_t getLegalMapOffset(uint64_t Offset) {
    بازگشت Offset & ~(sys::fs::mapped_file_region::alignment() - 1);
  }

  ایستا uint64_t getLegalMapSize(uint64_t Len, uint64_t Offset) {
    بازگشت Len + (Offset - getLegalMapOffset(Offset));
  }

  ثابت نویسه *getStart(uint64_t Len, uint64_t Offset) {
    بازگشت MFR.const_data() + (Offset - getLegalMapOffset(Offset));
  }

عمومی:
  MemoryBufferMMapFile(منطقی RequiresNullTerminator, sys::fs::file_t FD, uint64_t Len,
                       uint64_t Offset, std::error_code &EC)
      : MFR(FD, Mapmode<MB>, getLegalMapSize(Len, Offset),
            getLegalMapOffset(Offset), EC) {
    اگر (!EC) {
      ثابت نویسه *Start = getStart(Len, Offset);
      MemoryBuffer::init(Start, Start + Len, RequiresNullTerminator);
    }
  }

  /// Disable sized deallocation for MemoryBufferMMapFile, because it has
  /// tail-allocated data.
  تهی operator delete(تهی *p) { std::آزاد(p); }

  StringRef getBufferIdentifier() ثابت override {
    // The name is stored after the class itself.
    بازگشت StringRef(reinterpret_cast<ثابت نویسه *>(این + 1) + اندازه(size_t),
                     *reinterpret_cast<ثابت size_t *>(این + 1));
  }

  MemoryBuffer::BufferKind getBufferKind() ثابت override {
    بازگشت MemoryBuffer::MemoryBuffer_MMap;
  }

  تهی dontNeedIfMmap() override { MFR.dontNeed(); }
  تهی willNeedIfMmap() override { MFR.willNeed(); }
  تهی randomAccessIfMmap() override { MFR.randomAccess(); }
};
} // namespace

ایستا ErrorOr<std::unique_ptr<WritableMemoryBuffer>>
getMemoryBufferForStream(sys::fs::file_t FD, ثابت Twine &BufferName) {
  SmallString<sys::fs::DefaultReadChunkSize> Buffer;
  اگر (Error E = sys::fs::readNativeFileToEOF(FD, Buffer))
    بازگشت errorToErrorCode(std::move(E));
  بازگشت getMemBufferCopyImpl(Buffer, BufferName);
}

ErrorOr<std::unique_ptr<MemoryBuffer>>
MemoryBuffer::getFile(ثابت Twine &Filename, منطقی IsText,
                      منطقی RequiresNullTerminator, منطقی IsVolatile,
                      std::optional<Align> Alignment) {
  sys::sandbox::violationIfEnabled();

  بازگشت getFileAux<MemoryBuffer>(Filename, /*MapSize=*/-1, /*Offset=*/0,
                                  IsText, RequiresNullTerminator, IsVolatile,
                                  Alignment);
}

template <typename MB>
ایستا ErrorOr<std::unique_ptr<MB>>
getOpenFileImpl(sys::fs::file_t FD, ثابت Twine &Filename, uint64_t FileSize,
                uint64_t MapSize, int64_t Offset, منطقی RequiresNullTerminator,
                منطقی IsVolatile, std::optional<Align> Alignment);

template <typename MB>
ایستا ErrorOr<std::unique_ptr<MB>>
getFileAux(ثابت Twine &Filename, uint64_t MapSize, uint64_t Offset,
           منطقی IsText, منطقی RequiresNullTerminator, منطقی IsVolatile,
           std::optional<Align> Alignment) {
  Expected<sys::fs::file_t> FDOrErr = sys::fs::openNativeFileForRead(
      Filename, IsText ? sys::fs::OF_TextWithCRLF : sys::fs::OF_None);
  اگر (!FDOrErr)
    بازگشت errorToErrorCode(FDOrErr.takeError());
  sys::fs::file_t FD = *FDOrErr;
  auto Ret = getOpenFileImpl<MB>(FD, Filename, /*FileSize=*/-1, MapSize, Offset,
                                 RequiresNullTerminator, IsVolatile, Alignment);
  sys::fs::closeFile(FD);
  بازگشت Ret;
}

ErrorOr<std::unique_ptr<WritableMemoryBuffer>>
WritableMemoryBuffer::getFile(ثابت Twine &Filename, منطقی IsVolatile,
                              std::optional<Align> Alignment) {
  sys::sandbox::violationIfEnabled();

  بازگشت getFileAux<WritableMemoryBuffer>(
      Filename, /*MapSize=*/-1, /*Offset=*/0, /*IsText=*/نادرست,
      /*RequiresNullTerminator=*/نادرست, IsVolatile, Alignment);
}

ErrorOr<std::unique_ptr<WritableMemoryBuffer>>
WritableMemoryBuffer::getFileSlice(ثابت Twine &Filename, uint64_t MapSize,
                                   uint64_t Offset, منطقی IsVolatile,
                                   std::optional<Align> Alignment) {
  sys::sandbox::violationIfEnabled();

  بازگشت getFileAux<WritableMemoryBuffer>(
      Filename, MapSize, Offset, /*IsText=*/نادرست,
      /*RequiresNullTerminator=*/نادرست, IsVolatile, Alignment);
}

std::unique_ptr<WritableMemoryBuffer>
WritableMemoryBuffer::getNewUninitMemBuffer(size_t Size,
                                            ثابت Twine &BufferName,
                                            std::optional<Align> Alignment) {
  using MemBuffer = MemoryBufferMem<WritableMemoryBuffer>;

  // Use 16-byte alignment if no alignment is specified.
  Align BufAlign = Alignment.مقدار_or(Align(16));

  // Allocate space for the MemoryBuffer, the data and the name. It is important
  // that MemoryBuffer and data are aligned so PointerIntPair works with them.
  SmallString<256> NameBuf;
  StringRef NameRef = BufferName.toStringRef(NameBuf);

  size_t StringLen = اندازه(MemBuffer) + اندازه(size_t) + NameRef.size() + 1;
  size_t RealLen = StringLen + Size + 1 + BufAlign.مقدار_2();
  اگر (RealLen <= Size) // Check for rollover.
    بازگشت nullptr;
  // We use a call to malloc() rather than a call to a non-throwing operator
  // new() because LLVM unconditionally installs an out of memory new handler
  // when exceptions are disabled. This new handler intentionally crashes to
  // aid with debugging, but that makes non-throwing new calls unhelpful.
  // See MemoryBufferMem::operator delete() for the paired call to free(), and
  // llvm::install_out_of_memory_new_handler() for the installation of the
  // custom new handler.
  نویسه *Mem = static_cast<نویسه *>(std::malloc(RealLen));
  اگر (!Mem)
    بازگشت nullptr;

  // The name is stored after the class itself.
  *reinterpret_cast<size_t *>(Mem + اندازه(MemBuffer)) = NameRef.size();
  CopyStringRef(Mem + اندازه(MemBuffer) + اندازه(size_t), NameRef);

  // The buffer begins after the name and must be aligned.
  نویسه *Buf = (نویسه *)alignAddr(Mem + StringLen, BufAlign);
  Buf[Size] = 0; // Null terminate buffer.

  auto *Ret = جدید (Mem) MemBuffer(StringRef(Buf, Size), درست);
  بازگشت std::unique_ptr<WritableMemoryBuffer>(Ret);
}

std::unique_ptr<WritableMemoryBuffer>
WritableMemoryBuffer::getNewMemBuffer(size_t Size, ثابت Twine &BufferName) {
  auto SB = WritableMemoryBuffer::getNewUninitMemBuffer(Size, BufferName);
  اگر (!SB)
    بازگشت nullptr;
  memset(SB->getBufferStart(), 0, Size);
  بازگشت SB;
}

ایستا منطقی shouldUseMmap(sys::fs::file_t FD,
                          size_t FileSize,
                          size_t MapSize,
                          off_t Offset,
                          منطقی RequiresNullTerminator,
                          صحیح PageSize,
                          منطقی IsVolatile) {
#اگر defined(__MVS__)
  // zOS Enhanced ASCII auto convert does not support mmap.
  بازگشت نادرست;
#endif

  // mmap may leave the buffer without null terminator if the file size changed
  // by the time the last page is mapped in, so avoid it if the file size is
  // likely to change.
  اگر (IsVolatile && RequiresNullTerminator)
    بازگشت نادرست;

  // We don't use mmap for small files because this can severely fragment our
  // address space.
  اگر (MapSize < 4 * 4096 || MapSize < (unsigned)PageSize)
    بازگشت نادرست;

  اگر (!RequiresNullTerminator)
    بازگشت درست;

  // If we don't know the file size, use fstat to find out.  fstat on an open
  // file descriptor is cheaper than stat on a random path.
  // FIXME: this chunk of code is duplicated, but it avoids a fstat when
  // RequiresNullTerminator = false and MapSize != -1.
  اگر (FileSize == size_t(-1)) {
    sys::fs::file_status Status;
    اگر (sys::fs::status(FD, Status))
      بازگشت نادرست;
    FileSize = Status.getSize();
  }

  // If we need a null terminator and the end of the map is inside the file,
  // we cannot use mmap.
  size_t End = Offset + MapSize;
  أكد(End <= FileSize);
  اگر (End != FileSize)
    بازگشت نادرست;

  // Don't try to map files that are exactly a multiple of the system page size
  // if we need a null terminator.
  اگر ((FileSize & (PageSize -1)) == 0)
    بازگشت نادرست;

#اگر defined(__CYGWIN__)
  // Don't try to map files that are exactly a multiple of the physical page size
  // if we need a null terminator.
  // FIXME: We should reorganize again getPageSize() on Win32.
  اگر ((FileSize & (4096 - 1)) == 0)
    بازگشت نادرست;
#endif

  بازگشت درست;
}

ایستا ErrorOr<std::unique_ptr<WriteThroughMemoryBuffer>>
getReadWriteFile(ثابت Twine &Filename, uint64_t FileSize, uint64_t MapSize,
                 uint64_t Offset) {
  Expected<sys::fs::file_t> FDOrErr = sys::fs::openNativeFileForReadWrite(
      Filename, sys::fs::CD_OpenExisting, sys::fs::OF_None);
  اگر (!FDOrErr)
    بازگشت errorToErrorCode(FDOrErr.takeError());
  sys::fs::file_t FD = *FDOrErr;

  // Default is to map the full file.
  اگر (MapSize == uint64_t(-1)) {
    // If we don't know the file size, use fstat to find out.  fstat on an open
    // file descriptor is cheaper than stat on a random path.
    اگر (FileSize == uint64_t(-1)) {
      sys::fs::file_status Status;
      std::error_code EC = sys::fs::status(FD, Status);
      اگر (EC)
        بازگشت EC;

      // If this not a file or a block device (e.g. it's a named pipe
      // or character device), we can't mmap it, so error out.
      sys::fs::file_type Type = Status.type();
      اگر (Type != sys::fs::file_type::regular_file &&
          Type != sys::fs::file_type::block_file)
        بازگشت make_error_code(errc::invalid_argument);

      FileSize = Status.getSize();
    }
    MapSize = FileSize;
  }

  std::error_code EC;
  std::unique_ptr<WriteThroughMemoryBuffer> Result(
      جدید (NamedBufferAlloc(Filename))
          MemoryBufferMMapFile<WriteThroughMemoryBuffer>(نادرست, FD, MapSize,
                                                         Offset, EC));
  اگر (EC)
    بازگشت EC;
  بازگشت std::move(Result);
}

ErrorOr<std::unique_ptr<WriteThroughMemoryBuffer>>
WriteThroughMemoryBuffer::getFile(ثابت Twine &Filename, int64_t FileSize) {
  sys::sandbox::violationIfEnabled();

  بازگشت getReadWriteFile(Filename, FileSize, FileSize, 0);
}

/// Map a subrange of the specified file as a WritableMemoryBuffer.
ErrorOr<std::unique_ptr<WriteThroughMemoryBuffer>>
WriteThroughMemoryBuffer::getFileSlice(ثابت Twine &Filename, uint64_t MapSize,
                                       uint64_t Offset) {
  sys::sandbox::violationIfEnabled();

  بازگشت getReadWriteFile(Filename, -1, MapSize, Offset);
}

template <typename MB>
ایستا ErrorOr<std::unique_ptr<MB>>
getOpenFileImpl(sys::fs::file_t FD, ثابت Twine &Filename, uint64_t FileSize,
                uint64_t MapSize, int64_t Offset, منطقی RequiresNullTerminator,
                منطقی IsVolatile, std::optional<Align> Alignment) {
  ایستا صحیح PageSize = sys::Process::getPageSizeEstimate();

  // Default is to map the full file.
  اگر (MapSize == uint64_t(-1)) {
    // If we don't know the file size, use fstat to find out.  fstat on an open
    // file descriptor is cheaper than stat on a random path.
    اگر (FileSize == uint64_t(-1)) {
      sys::fs::file_status Status;
      std::error_code EC = sys::fs::status(FD, Status);
      اگر (EC)
        بازگشت EC;

      // If this not a file or a block device (e.g. it's a named pipe
      // or character device), we can't trust the size. Create the memory
      // buffer by copying off the stream.
      sys::fs::file_type Type = Status.type();
      اگر (Type != sys::fs::file_type::regular_file &&
          Type != sys::fs::file_type::block_file)
        بازگشت getMemoryBufferForStream(FD, Filename);

      FileSize = Status.getSize();
    }
    MapSize = FileSize;
  }

  اگر (shouldUseMmap(FD, FileSize, MapSize, Offset, RequiresNullTerminator,
                    PageSize, IsVolatile)) {
    std::error_code EC;
    std::unique_ptr<MB> Result(
        جدید (NamedBufferAlloc(Filename)) MemoryBufferMMapFile<MB>(
            RequiresNullTerminator, FD, MapSize, Offset, EC));
    اگر (!EC) {
      // On at least Linux, and possibly on other systems, mmap may return pages
      // from the page cache that are not properly filled with trailing zeroes,
      // if some prior user of the page wrote non-zero bytes. Detect this and
      // don't use mmap in that case.
      اگر (!RequiresNullTerminator || *Result->getBufferEnd() == '\0')
        بازگشت std::move(Result);
    }
  }

#ifdef __MVS__
  ErrorOr<منطقی> NeedsConversion = needConversion(Filename, FD);
  اگر (std::error_code EC = NeedsConversion.getError())
    بازگشت EC;
  // File size may increase due to EBCDIC -> UTF-8 conversion, therefore we
  // cannot trust the file size and we create the memory buffer by copying
  // off the stream.
  // Note: This only works with the assumption of reading a full file (i.e,
  // Offset == 0 and MapSize == FileSize). Reading a file slice does not work.
  اگر (*NeedsConversion && Offset == 0 && MapSize == FileSize)
    بازگشت getMemoryBufferForStream(FD, Filename);
#endif

  auto Buf =
      WritableMemoryBuffer::getNewUninitMemBuffer(MapSize, Filename, Alignment);
  اگر (!Buf) {
    // Failed to create a buffer. The only way it can fail is if
    // new(std::nothrow) returns 0.
    بازگشت make_error_code(errc::not_enough_memory);
  }

  // Read until EOF, zero-initialize the rest.
  MutableArrayRef<نویسه> ToRead = Buf->getBuffer();
  وقتی (!ToRead.empty()) {
    Expected<size_t> ReadBytes =
        sys::fs::readNativeFileSlice(FD, ToRead, Offset);
    اگر (!ReadBytes)
      بازگشت errorToErrorCode(ReadBytes.takeError());
    اگر (*ReadBytes == 0) {
      std::memset(ToRead.data(), 0, ToRead.size());
      بشکن;
    }
    ToRead = ToRead.drop_front(*ReadBytes);
    Offset += *ReadBytes;
  }

  بازگشت std::move(Buf);
}

ErrorOr<std::unique_ptr<MemoryBuffer>>
MemoryBuffer::getOpenFile(sys::fs::file_t FD, ثابت Twine &Filename,
                          uint64_t FileSize, منطقی RequiresNullTerminator,
                          منطقی IsVolatile, std::optional<Align> Alignment) {
  sys::sandbox::violationIfEnabled();

  بازگشت getOpenFileImpl<MemoryBuffer>(FD, Filename, FileSize, FileSize, 0,
                                       RequiresNullTerminator, IsVolatile,
                                       Alignment);
}

ErrorOr<std::unique_ptr<MemoryBuffer>> MemoryBuffer::getOpenFileSlice(
    sys::fs::file_t FD, ثابت Twine &Filename, uint64_t MapSize, int64_t Offset,
    منطقی IsVolatile, std::optional<Align> Alignment) {
  أكد(MapSize != uint64_t(-1));

  sys::sandbox::violationIfEnabled();

  بازگشت getOpenFileImpl<MemoryBuffer>(FD, Filename, -1, MapSize, Offset, نادرست,
                                       IsVolatile, Alignment);
}

ErrorOr<std::unique_ptr<MemoryBuffer>> MemoryBuffer::getSTDIN() {
  sys::sandbox::violationIfEnabled();

  // Read in all of the data from stdin, we cannot mmap stdin.
  //
  // FIXME: That isn't necessarily true, we should try to mmap stdin and
  // fallback if it fails.
  sys::ChangeStdinMode(sys::fs::OF_Text);

  بازگشت getMemoryBufferForStream(sys::fs::getStdinHandle(), "<stdin>");
}

ErrorOr<std::unique_ptr<MemoryBuffer>>
MemoryBuffer::getFileAsStream(ثابت Twine &Filename) {
  sys::sandbox::violationIfEnabled();

  Expected<sys::fs::file_t> FDOrErr =
      sys::fs::openNativeFileForRead(Filename, sys::fs::OF_None);
  اگر (!FDOrErr)
    بازگشت errorToErrorCode(FDOrErr.takeError());
  sys::fs::file_t FD = *FDOrErr;
  ErrorOr<std::unique_ptr<MemoryBuffer>> Ret =
      getMemoryBufferForStream(FD, Filename);
  sys::fs::closeFile(FD);
  بازگشت Ret;
}

MemoryBufferRef MemoryBuffer::getMemBufferRef() ثابت {
  StringRef Data = getBuffer();
  StringRef Identifier = getBufferIdentifier();
  بازگشت MemoryBufferRef(Data, Identifier);
}

SmallVectorMemoryBuffer::~SmallVectorMemoryBuffer() = پیشفرض;
