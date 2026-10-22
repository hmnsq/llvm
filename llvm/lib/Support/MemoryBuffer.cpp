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

#അന്തര്ഭാവ "llvm/Support/MemoryBuffer.h"
#അന്തര്ഭാവ "llvm/ADT/STLExtras.h"
#അന്തര്ഭാവ "llvm/ADT/SmallString.h"
#അന്തര്ഭാവ "llvm/Config/config.h"
#അന്തര്ഭാവ "llvm/Support/Alignment.h"
#അന്തര്ഭാവ "llvm/Support/AutoConvert.h"
#അന്തര്ഭാവ "llvm/Support/Errc.h"
#അന്തര്ഭാവ "llvm/Support/Error.h"
#അന്തര്ഭാവ "llvm/Support/ErrorHandling.h"
#അന്തര്ഭാവ "llvm/Support/FileSystem.h"
#അന്തര്ഭാവ "llvm/Support/IOSandbox.h"
#അന്തര്ഭാവ "llvm/Support/Process.h"
#അന്തര്ഭാവ "llvm/Support/Program.h"
#അന്തര്ഭാവ "llvm/Support/SmallVectorMemoryBuffer.h"
#അന്തര്ഭാവ <algorithm>
#അന്തര്ഭാവ <cassert>
#അന്തര്ഭാവ <cstring>
#അന്തര്ഭാവ <new>
#അന്തര്ഭാവ <sys/types.h>
#അന്തര്ഭാവ <system_error>
#എങ്കിൽ !defined(_MSC_VER) && !defined(__MINGW32__)
#അന്തര്ഭാവ <unistd.h>
#അല്ലെങ്കിൽ
#അന്തര്ഭാവ <io.h>
#endif

using നാമാകാശ llvm;

//===----------------------------------------------------------------------===//
// MemoryBuffer implementation itself.
//===----------------------------------------------------------------------===//

MemoryBuffer::~MemoryBuffer() = സ്ഥിരസ്ഥിതി;

/// init - Initialize this MemoryBuffer as a reference to externally allocated
/// memory, memory that we know is already null terminated.
ശൂന്യം MemoryBuffer::init(സ്ഥിരം അക്ഷരം *BufStart, സ്ഥിരം അക്ഷരം *BufEnd,
                        തർക്കം RequiresNullTerminator) {
  പ്രതിജ്ഞാ((!RequiresNullTerminator || BufEnd[0] == 0) &&
         "Buffer is not null terminated!");
  BufferStart = BufStart;
  BufferEnd = BufEnd;
}

//===----------------------------------------------------------------------===//
// MemoryBufferMem implementation.
//===----------------------------------------------------------------------===//

/// CopyStringRef - Copies contents of a StringRef into a block of memory and
/// null-terminates it.
സ്ഥിതം ശൂന്യം CopyStringRef(അക്ഷരം *Memory, StringRef Data) {
  എങ്കിൽ (!Data.empty())
    memcpy(Memory, Data.data(), Data.size());
  Memory[Data.size()] = 0; // Null terminate string.
}

നാമാകാശ {
ഘടന NamedBufferAlloc {
  സ്ഥിരം Twine &Name;
  NamedBufferAlloc(സ്ഥിരം Twine &Name) : Name(Name) {}
};
} // namespace

ശൂന്യം *operator പുതിയ(size_t N, സ്ഥിരം NamedBufferAlloc &Alloc) {
  SmallString<256> NameBuf;
  StringRef NameRef = Alloc.Name.toStringRef(NameBuf);

  // We use malloc() and manually handle it returning null instead of calling
  // operator new because we need all uses of NamedBufferAlloc to be
  // deallocated with a call to free() due to needing to use malloc() in
  // WritableMemoryBuffer::getNewUninitMemBuffer() to work around the out-of-
  // memory handler installed by default in LLVM. See operator delete() member
  // functions within this file for the paired call to free().
  അക്ഷരം *Mem =
      static_cast<അക്ഷരം *>(std::malloc(N + വലുപ്പം(size_t) + NameRef.size() + 1));
  എങ്കിൽ (!Mem)
    llvm::report_bad_alloc_error("Allocation failed");
  *reinterpret_cast<size_t *>(Mem + N) = NameRef.size();
  CopyStringRef(Mem + N + വലുപ്പം(size_t), NameRef);
  തിരികെ Mem;
}

നാമാകാശ {
/// MemoryBufferMem - Named MemoryBuffer pointing to a block of memory.
template<typename MB>
വർഗ്ഗം MemoryBufferMem : പൊതു MB {
പൊതു:
  MemoryBufferMem(StringRef നിവേശം_Data, തർക്കം RequiresNullTerminator) {
    MemoryBuffer::init(നിവേശം_Data.begin(), നിവേശം_Data.end(),
                       RequiresNullTerminator);
  }

  /// Disable sized deallocation for MemoryBufferMem, because it has
  /// tail-allocated data.
  ശൂന്യം operator delete(ശൂന്യം *p) { std::സ്വതന്ത്രം(p); }

  StringRef getBufferIdentifier() സ്ഥിരം override {
    // The name is stored after the class itself.
    തിരികെ StringRef(reinterpret_cast<സ്ഥിരം അക്ഷരം *>(ഇത് + 1) + വലുപ്പം(size_t),
                     *reinterpret_cast<സ്ഥിരം size_t *>(ഇത് + 1));
  }

  MemoryBuffer::BufferKind getBufferKind() സ്ഥിരം override {
    തിരികെ MemoryBuffer::MemoryBuffer_Malloc;
  }
};
} // namespace

template <typename MB>
സ്ഥിതം ErrorOr<std::unique_ptr<MB>>
getFileAux(സ്ഥിരം Twine &Filename, uint64_t MapSize, uint64_t Offset,
           തർക്കം IsText, തർക്കം RequiresNullTerminator, തർക്കം IsVolatile,
           std::optional<Align> Alignment);

std::unique_ptr<MemoryBuffer>
MemoryBuffer::getMemBuffer(StringRef നിവേശം_Data, StringRef BufferName,
                           തർക്കം RequiresNullTerminator) {
  auto *Ret = പുതിയ (NamedBufferAlloc(BufferName))
      MemoryBufferMem<MemoryBuffer>(നിവേശം_Data, RequiresNullTerminator);
  തിരികെ std::unique_ptr<MemoryBuffer>(Ret);
}

std::unique_ptr<MemoryBuffer>
MemoryBuffer::getMemBuffer(MemoryBufferRef Ref, തർക്കം RequiresNullTerminator) {
  തിരികെ std::unique_ptr<MemoryBuffer>(getMemBuffer(
      Ref.getBuffer(), Ref.getBufferIdentifier(), RequiresNullTerminator));
}

സ്ഥിതം ErrorOr<std::unique_ptr<WritableMemoryBuffer>>
getMemBufferCopyImpl(StringRef നിവേശം_Data, സ്ഥിരം Twine &BufferName) {
  auto Buf =
      WritableMemoryBuffer::getNewUninitMemBuffer(നിവേശം_Data.size(), BufferName);
  എങ്കിൽ (!Buf)
    തിരികെ make_error_code(errc::not_enough_memory);
  // Calling memcpy with null src/dst is UB, and an empty StringRef is
  // represented with {nullptr, 0}.
  llvm::copy(നിവേശം_Data, Buf->getBufferStart());
  തിരികെ std::move(Buf);
}

std::unique_ptr<MemoryBuffer>
MemoryBuffer::getMemBufferCopy(StringRef നിവേശം_Data, സ്ഥിരം Twine &BufferName) {
  auto Buf = getMemBufferCopyImpl(നിവേശം_Data, BufferName);
  എങ്കിൽ (Buf)
    തിരികെ std::move(*Buf);
  തിരികെ nullptr;
}

ErrorOr<std::unique_ptr<MemoryBuffer>>
MemoryBuffer::getFileOrSTDIN(സ്ഥിരം Twine &Filename, തർക്കം IsText,
                             തർക്കം RequiresNullTerminator,
                             std::optional<Align> Alignment) {
  sys::sandbox::violationIfEnabled();

  SmallString<256> NameBuf;
  StringRef NameRef = Filename.toStringRef(NameBuf);

  എങ്കിൽ (NameRef == "-")
    തിരികെ getSTDIN();
  തിരികെ getFile(Filename, IsText, RequiresNullTerminator,
                 /*IsVolatile=*/അസത്യം, Alignment);
}

ErrorOr<std::unique_ptr<MemoryBuffer>>
MemoryBuffer::getFileSlice(സ്ഥിരം Twine &FilePath, uint64_t MapSize,
                           uint64_t Offset, തർക്കം IsVolatile,
                           std::optional<Align> Alignment) {
  sys::sandbox::violationIfEnabled();

  തിരികെ getFileAux<MemoryBuffer>(FilePath, MapSize, Offset, /*IsText=*/അസത്യം,
                                  /*RequiresNullTerminator=*/അസത്യം, IsVolatile,
                                  Alignment);
}

//===----------------------------------------------------------------------===//
// MemoryBuffer::getFile implementation.
//===----------------------------------------------------------------------===//

നാമാകാശ {

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
വർഗ്ഗം MemoryBufferMMapFile : പൊതു MB {
  sys::fs::mapped_file_region MFR;

  സ്ഥിതം uint64_t getLegalMapOffset(uint64_t Offset) {
    തിരികെ Offset & ~(sys::fs::mapped_file_region::alignment() - 1);
  }

  സ്ഥിതം uint64_t getLegalMapSize(uint64_t Len, uint64_t Offset) {
    തിരികെ Len + (Offset - getLegalMapOffset(Offset));
  }

  സ്ഥിരം അക്ഷരം *getStart(uint64_t Len, uint64_t Offset) {
    തിരികെ MFR.const_data() + (Offset - getLegalMapOffset(Offset));
  }

പൊതു:
  MemoryBufferMMapFile(തർക്കം RequiresNullTerminator, sys::fs::file_t FD, uint64_t Len,
                       uint64_t Offset, std::error_code &EC)
      : MFR(FD, Mapmode<MB>, getLegalMapSize(Len, Offset),
            getLegalMapOffset(Offset), EC) {
    എങ്കിൽ (!EC) {
      സ്ഥിരം അക്ഷരം *Start = getStart(Len, Offset);
      MemoryBuffer::init(Start, Start + Len, RequiresNullTerminator);
    }
  }

  /// Disable sized deallocation for MemoryBufferMMapFile, because it has
  /// tail-allocated data.
  ശൂന്യം operator delete(ശൂന്യം *p) { std::സ്വതന്ത്രം(p); }

  StringRef getBufferIdentifier() സ്ഥിരം override {
    // The name is stored after the class itself.
    തിരികെ StringRef(reinterpret_cast<സ്ഥിരം അക്ഷരം *>(ഇത് + 1) + വലുപ്പം(size_t),
                     *reinterpret_cast<സ്ഥിരം size_t *>(ഇത് + 1));
  }

  MemoryBuffer::BufferKind getBufferKind() സ്ഥിരം override {
    തിരികെ MemoryBuffer::MemoryBuffer_MMap;
  }

  ശൂന്യം dontNeedIfMmap() override { MFR.dontNeed(); }
  ശൂന്യം willNeedIfMmap() override { MFR.willNeed(); }
  ശൂന്യം randomAccessIfMmap() override { MFR.randomAccess(); }
};
} // namespace

സ്ഥിതം ErrorOr<std::unique_ptr<WritableMemoryBuffer>>
getMemoryBufferForStream(sys::fs::file_t FD, സ്ഥിരം Twine &BufferName) {
  SmallString<sys::fs::DefaultReadChunkSize> Buffer;
  എങ്കിൽ (Error E = sys::fs::readNativeFileToEOF(FD, Buffer))
    തിരികെ errorToErrorCode(std::move(E));
  തിരികെ getMemBufferCopyImpl(Buffer, BufferName);
}

ErrorOr<std::unique_ptr<MemoryBuffer>>
MemoryBuffer::getFile(സ്ഥിരം Twine &Filename, തർക്കം IsText,
                      തർക്കം RequiresNullTerminator, തർക്കം IsVolatile,
                      std::optional<Align> Alignment) {
  sys::sandbox::violationIfEnabled();

  തിരികെ getFileAux<MemoryBuffer>(Filename, /*MapSize=*/-1, /*Offset=*/0,
                                  IsText, RequiresNullTerminator, IsVolatile,
                                  Alignment);
}

template <typename MB>
സ്ഥിതം ErrorOr<std::unique_ptr<MB>>
getOpenFileImpl(sys::fs::file_t FD, സ്ഥിരം Twine &Filename, uint64_t FileSize,
                uint64_t MapSize, int64_t Offset, തർക്കം RequiresNullTerminator,
                തർക്കം IsVolatile, std::optional<Align> Alignment);

template <typename MB>
സ്ഥിതം ErrorOr<std::unique_ptr<MB>>
getFileAux(സ്ഥിരം Twine &Filename, uint64_t MapSize, uint64_t Offset,
           തർക്കം IsText, തർക്കം RequiresNullTerminator, തർക്കം IsVolatile,
           std::optional<Align> Alignment) {
  Expected<sys::fs::file_t> FDOrErr = sys::fs::openNativeFileForRead(
      Filename, IsText ? sys::fs::OF_TextWithCRLF : sys::fs::OF_None);
  എങ്കിൽ (!FDOrErr)
    തിരികെ errorToErrorCode(FDOrErr.takeError());
  sys::fs::file_t FD = *FDOrErr;
  auto Ret = getOpenFileImpl<MB>(FD, Filename, /*FileSize=*/-1, MapSize, Offset,
                                 RequiresNullTerminator, IsVolatile, Alignment);
  sys::fs::closeFile(FD);
  തിരികെ Ret;
}

ErrorOr<std::unique_ptr<WritableMemoryBuffer>>
WritableMemoryBuffer::getFile(സ്ഥിരം Twine &Filename, തർക്കം IsVolatile,
                              std::optional<Align> Alignment) {
  sys::sandbox::violationIfEnabled();

  തിരികെ getFileAux<WritableMemoryBuffer>(
      Filename, /*MapSize=*/-1, /*Offset=*/0, /*IsText=*/അസത്യം,
      /*RequiresNullTerminator=*/അസത്യം, IsVolatile, Alignment);
}

ErrorOr<std::unique_ptr<WritableMemoryBuffer>>
WritableMemoryBuffer::getFileSlice(സ്ഥിരം Twine &Filename, uint64_t MapSize,
                                   uint64_t Offset, തർക്കം IsVolatile,
                                   std::optional<Align> Alignment) {
  sys::sandbox::violationIfEnabled();

  തിരികെ getFileAux<WritableMemoryBuffer>(
      Filename, MapSize, Offset, /*IsText=*/അസത്യം,
      /*RequiresNullTerminator=*/അസത്യം, IsVolatile, Alignment);
}

std::unique_ptr<WritableMemoryBuffer>
WritableMemoryBuffer::getNewUninitMemBuffer(size_t Size,
                                            സ്ഥിരം Twine &BufferName,
                                            std::optional<Align> Alignment) {
  using MemBuffer = MemoryBufferMem<WritableMemoryBuffer>;

  // Use 16-byte alignment if no alignment is specified.
  Align BufAlign = Alignment.മൂല്യം_or(Align(16));

  // Allocate space for the MemoryBuffer, the data and the name. It is important
  // that MemoryBuffer and data are aligned so PointerIntPair works with them.
  SmallString<256> NameBuf;
  StringRef NameRef = BufferName.toStringRef(NameBuf);

  size_t StringLen = വലുപ്പം(MemBuffer) + വലുപ്പം(size_t) + NameRef.size() + 1;
  size_t RealLen = StringLen + Size + 1 + BufAlign.മൂല്യം_2();
  എങ്കിൽ (RealLen <= Size) // Check for rollover.
    തിരികെ nullptr;
  // We use a call to malloc() rather than a call to a non-throwing operator
  // new() because LLVM unconditionally installs an out of memory new handler
  // when exceptions are disabled. This new handler intentionally crashes to
  // aid with debugging, but that makes non-throwing new calls unhelpful.
  // See MemoryBufferMem::operator delete() for the paired call to free(), and
  // llvm::install_out_of_memory_new_handler() for the installation of the
  // custom new handler.
  അക്ഷരം *Mem = static_cast<അക്ഷരം *>(std::malloc(RealLen));
  എങ്കിൽ (!Mem)
    തിരികെ nullptr;

  // The name is stored after the class itself.
  *reinterpret_cast<size_t *>(Mem + വലുപ്പം(MemBuffer)) = NameRef.size();
  CopyStringRef(Mem + വലുപ്പം(MemBuffer) + വലുപ്പം(size_t), NameRef);

  // The buffer begins after the name and must be aligned.
  അക്ഷരം *Buf = (അക്ഷരം *)alignAddr(Mem + StringLen, BufAlign);
  Buf[Size] = 0; // Null terminate buffer.

  auto *Ret = പുതിയ (Mem) MemBuffer(StringRef(Buf, Size), സത്യം);
  തിരികെ std::unique_ptr<WritableMemoryBuffer>(Ret);
}

std::unique_ptr<WritableMemoryBuffer>
WritableMemoryBuffer::getNewMemBuffer(size_t Size, സ്ഥിരം Twine &BufferName) {
  auto SB = WritableMemoryBuffer::getNewUninitMemBuffer(Size, BufferName);
  എങ്കിൽ (!SB)
    തിരികെ nullptr;
  memset(SB->getBufferStart(), 0, Size);
  തിരികെ SB;
}

സ്ഥിതം തർക്കം shouldUseMmap(sys::fs::file_t FD,
                          size_t FileSize,
                          size_t MapSize,
                          off_t Offset,
                          തർക്കം RequiresNullTerminator,
                          പൂർണ്ണസംഖ്യ PageSize,
                          തർക്കം IsVolatile) {
#എങ്കിൽ defined(__MVS__)
  // zOS Enhanced ASCII auto convert does not support mmap.
  തിരികെ അസത്യം;
#endif

  // mmap may leave the buffer without null terminator if the file size changed
  // by the time the last page is mapped in, so avoid it if the file size is
  // likely to change.
  എങ്കിൽ (IsVolatile && RequiresNullTerminator)
    തിരികെ അസത്യം;

  // We don't use mmap for small files because this can severely fragment our
  // address space.
  എങ്കിൽ (MapSize < 4 * 4096 || MapSize < (unsigned)PageSize)
    തിരികെ അസത്യം;

  എങ്കിൽ (!RequiresNullTerminator)
    തിരികെ സത്യം;

  // If we don't know the file size, use fstat to find out.  fstat on an open
  // file descriptor is cheaper than stat on a random path.
  // FIXME: this chunk of code is duplicated, but it avoids a fstat when
  // RequiresNullTerminator = false and MapSize != -1.
  എങ്കിൽ (FileSize == size_t(-1)) {
    sys::fs::file_status Status;
    എങ്കിൽ (sys::fs::status(FD, Status))
      തിരികെ അസത്യം;
    FileSize = Status.getSize();
  }

  // If we need a null terminator and the end of the map is inside the file,
  // we cannot use mmap.
  size_t End = Offset + MapSize;
  പ്രതിജ്ഞാ(End <= FileSize);
  എങ്കിൽ (End != FileSize)
    തിരികെ അസത്യം;

  // Don't try to map files that are exactly a multiple of the system page size
  // if we need a null terminator.
  എങ്കിൽ ((FileSize & (PageSize -1)) == 0)
    തിരികെ അസത്യം;

#എങ്കിൽ defined(__CYGWIN__)
  // Don't try to map files that are exactly a multiple of the physical page size
  // if we need a null terminator.
  // FIXME: We should reorganize again getPageSize() on Win32.
  എങ്കിൽ ((FileSize & (4096 - 1)) == 0)
    തിരികെ അസത്യം;
#endif

  തിരികെ സത്യം;
}

സ്ഥിതം ErrorOr<std::unique_ptr<WriteThroughMemoryBuffer>>
getReadWriteFile(സ്ഥിരം Twine &Filename, uint64_t FileSize, uint64_t MapSize,
                 uint64_t Offset) {
  Expected<sys::fs::file_t> FDOrErr = sys::fs::openNativeFileForReadWrite(
      Filename, sys::fs::CD_OpenExisting, sys::fs::OF_None);
  എങ്കിൽ (!FDOrErr)
    തിരികെ errorToErrorCode(FDOrErr.takeError());
  sys::fs::file_t FD = *FDOrErr;

  // Default is to map the full file.
  എങ്കിൽ (MapSize == uint64_t(-1)) {
    // If we don't know the file size, use fstat to find out.  fstat on an open
    // file descriptor is cheaper than stat on a random path.
    എങ്കിൽ (FileSize == uint64_t(-1)) {
      sys::fs::file_status Status;
      std::error_code EC = sys::fs::status(FD, Status);
      എങ്കിൽ (EC)
        തിരികെ EC;

      // If this not a file or a block device (e.g. it's a named pipe
      // or character device), we can't mmap it, so error out.
      sys::fs::file_type Type = Status.type();
      എങ്കിൽ (Type != sys::fs::file_type::regular_file &&
          Type != sys::fs::file_type::block_file)
        തിരികെ make_error_code(errc::invalid_argument);

      FileSize = Status.getSize();
    }
    MapSize = FileSize;
  }

  std::error_code EC;
  std::unique_ptr<WriteThroughMemoryBuffer> Result(
      പുതിയ (NamedBufferAlloc(Filename))
          MemoryBufferMMapFile<WriteThroughMemoryBuffer>(അസത്യം, FD, MapSize,
                                                         Offset, EC));
  എങ്കിൽ (EC)
    തിരികെ EC;
  തിരികെ std::move(Result);
}

ErrorOr<std::unique_ptr<WriteThroughMemoryBuffer>>
WriteThroughMemoryBuffer::getFile(സ്ഥിരം Twine &Filename, int64_t FileSize) {
  sys::sandbox::violationIfEnabled();

  തിരികെ getReadWriteFile(Filename, FileSize, FileSize, 0);
}

/// Map a subrange of the specified file as a WritableMemoryBuffer.
ErrorOr<std::unique_ptr<WriteThroughMemoryBuffer>>
WriteThroughMemoryBuffer::getFileSlice(സ്ഥിരം Twine &Filename, uint64_t MapSize,
                                       uint64_t Offset) {
  sys::sandbox::violationIfEnabled();

  തിരികെ getReadWriteFile(Filename, -1, MapSize, Offset);
}

template <typename MB>
സ്ഥിതം ErrorOr<std::unique_ptr<MB>>
getOpenFileImpl(sys::fs::file_t FD, സ്ഥിരം Twine &Filename, uint64_t FileSize,
                uint64_t MapSize, int64_t Offset, തർക്കം RequiresNullTerminator,
                തർക്കം IsVolatile, std::optional<Align> Alignment) {
  സ്ഥിതം പൂർണ്ണസംഖ്യ PageSize = sys::Process::getPageSizeEstimate();

  // Default is to map the full file.
  എങ്കിൽ (MapSize == uint64_t(-1)) {
    // If we don't know the file size, use fstat to find out.  fstat on an open
    // file descriptor is cheaper than stat on a random path.
    എങ്കിൽ (FileSize == uint64_t(-1)) {
      sys::fs::file_status Status;
      std::error_code EC = sys::fs::status(FD, Status);
      എങ്കിൽ (EC)
        തിരികെ EC;

      // If this not a file or a block device (e.g. it's a named pipe
      // or character device), we can't trust the size. Create the memory
      // buffer by copying off the stream.
      sys::fs::file_type Type = Status.type();
      എങ്കിൽ (Type != sys::fs::file_type::regular_file &&
          Type != sys::fs::file_type::block_file)
        തിരികെ getMemoryBufferForStream(FD, Filename);

      FileSize = Status.getSize();
    }
    MapSize = FileSize;
  }

  എങ്കിൽ (shouldUseMmap(FD, FileSize, MapSize, Offset, RequiresNullTerminator,
                    PageSize, IsVolatile)) {
    std::error_code EC;
    std::unique_ptr<MB> Result(
        പുതിയ (NamedBufferAlloc(Filename)) MemoryBufferMMapFile<MB>(
            RequiresNullTerminator, FD, MapSize, Offset, EC));
    എങ്കിൽ (!EC) {
      // On at least Linux, and possibly on other systems, mmap may return pages
      // from the page cache that are not properly filled with trailing zeroes,
      // if some prior user of the page wrote non-zero bytes. Detect this and
      // don't use mmap in that case.
      എങ്കിൽ (!RequiresNullTerminator || *Result->getBufferEnd() == '\0')
        തിരികെ std::move(Result);
    }
  }

#ifdef __MVS__
  ErrorOr<തർക്കം> NeedsConversion = needConversion(Filename, FD);
  എങ്കിൽ (std::error_code EC = NeedsConversion.getError())
    തിരികെ EC;
  // File size may increase due to EBCDIC -> UTF-8 conversion, therefore we
  // cannot trust the file size and we create the memory buffer by copying
  // off the stream.
  // Note: This only works with the assumption of reading a full file (i.e,
  // Offset == 0 and MapSize == FileSize). Reading a file slice does not work.
  എങ്കിൽ (*NeedsConversion && Offset == 0 && MapSize == FileSize)
    തിരികെ getMemoryBufferForStream(FD, Filename);
#endif

  auto Buf =
      WritableMemoryBuffer::getNewUninitMemBuffer(MapSize, Filename, Alignment);
  എങ്കിൽ (!Buf) {
    // Failed to create a buffer. The only way it can fail is if
    // new(std::nothrow) returns 0.
    തിരികെ make_error_code(errc::not_enough_memory);
  }

  // Read until EOF, zero-initialize the rest.
  MutableArrayRef<അക്ഷരം> ToRead = Buf->getBuffer();
  ആകുമ്പോൾ (!ToRead.empty()) {
    Expected<size_t> ReadBytes =
        sys::fs::readNativeFileSlice(FD, ToRead, Offset);
    എങ്കിൽ (!ReadBytes)
      തിരികെ errorToErrorCode(ReadBytes.takeError());
    എങ്കിൽ (*ReadBytes == 0) {
      std::memset(ToRead.data(), 0, ToRead.size());
      നിർത്തുക;
    }
    ToRead = ToRead.drop_front(*ReadBytes);
    Offset += *ReadBytes;
  }

  തിരികെ std::move(Buf);
}

ErrorOr<std::unique_ptr<MemoryBuffer>>
MemoryBuffer::getOpenFile(sys::fs::file_t FD, സ്ഥിരം Twine &Filename,
                          uint64_t FileSize, തർക്കം RequiresNullTerminator,
                          തർക്കം IsVolatile, std::optional<Align> Alignment) {
  sys::sandbox::violationIfEnabled();

  തിരികെ getOpenFileImpl<MemoryBuffer>(FD, Filename, FileSize, FileSize, 0,
                                       RequiresNullTerminator, IsVolatile,
                                       Alignment);
}

ErrorOr<std::unique_ptr<MemoryBuffer>> MemoryBuffer::getOpenFileSlice(
    sys::fs::file_t FD, സ്ഥിരം Twine &Filename, uint64_t MapSize, int64_t Offset,
    തർക്കം IsVolatile, std::optional<Align> Alignment) {
  പ്രതിജ്ഞാ(MapSize != uint64_t(-1));

  sys::sandbox::violationIfEnabled();

  തിരികെ getOpenFileImpl<MemoryBuffer>(FD, Filename, -1, MapSize, Offset, അസത്യം,
                                       IsVolatile, Alignment);
}

ErrorOr<std::unique_ptr<MemoryBuffer>> MemoryBuffer::getSTDIN() {
  sys::sandbox::violationIfEnabled();

  // Read in all of the data from stdin, we cannot mmap stdin.
  //
  // FIXME: That isn't necessarily true, we should try to mmap stdin and
  // fallback if it fails.
  sys::ChangeStdinMode(sys::fs::OF_Text);

  തിരികെ getMemoryBufferForStream(sys::fs::getStdinHandle(), "<stdin>");
}

ErrorOr<std::unique_ptr<MemoryBuffer>>
MemoryBuffer::getFileAsStream(സ്ഥിരം Twine &Filename) {
  sys::sandbox::violationIfEnabled();

  Expected<sys::fs::file_t> FDOrErr =
      sys::fs::openNativeFileForRead(Filename, sys::fs::OF_None);
  എങ്കിൽ (!FDOrErr)
    തിരികെ errorToErrorCode(FDOrErr.takeError());
  sys::fs::file_t FD = *FDOrErr;
  ErrorOr<std::unique_ptr<MemoryBuffer>> Ret =
      getMemoryBufferForStream(FD, Filename);
  sys::fs::closeFile(FD);
  തിരികെ Ret;
}

MemoryBufferRef MemoryBuffer::getMemBufferRef() സ്ഥിരം {
  StringRef Data = getBuffer();
  StringRef Identifier = getBufferIdentifier();
  തിരികെ MemoryBufferRef(Data, Identifier);
}

SmallVectorMemoryBuffer::~SmallVectorMemoryBuffer() = സ്ഥിരസ്ഥിതി;
