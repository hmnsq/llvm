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

#ಅನ್ತರ್ಭಾವ "llvm/Support/MemoryBuffer.h"
#ಅನ್ತರ್ಭಾವ "llvm/ADT/STLExtras.h"
#ಅನ್ತರ್ಭಾವ "llvm/ADT/SmallString.h"
#ಅನ್ತರ್ಭಾವ "llvm/Config/config.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/Alignment.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/AutoConvert.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/Errc.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/Error.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/ErrorHandling.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/FileSystem.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/IOSandbox.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/Process.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/Program.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/SmallVectorMemoryBuffer.h"
#ಅನ್ತರ್ಭಾವ <algorithm>
#ಅನ್ತರ್ಭಾವ <cassert>
#ಅನ್ತರ್ಭಾವ <cstring>
#ಅನ್ತರ್ಭಾವ <new>
#ಅನ್ತರ್ಭಾವ <sys/types.h>
#ಅನ್ತರ್ಭಾವ <system_error>
#ಯದಿ !defined(_MSC_VER) && !defined(__MINGW32__)
#ಅನ್ತರ್ಭಾವ <unistd.h>
#ಅನ್ಯಥಾ
#ಅನ್ತರ್ಭಾವ <io.h>
#endif

using ನಾಮಾಕಾಶ llvm;

//===----------------------------------------------------------------------===//
// MemoryBuffer implementation itself.
//===----------------------------------------------------------------------===//

MemoryBuffer::~MemoryBuffer() = ಸ್ವಭಾವ;

/// init - Initialize this MemoryBuffer as a reference to externally allocated
/// memory, memory that we know is already null terminated.
ಶೂನ್ಯ MemoryBuffer::init(ಧ್ರುವ ವರ್ಣ *BufStart, ಧ್ರುವ ವರ್ಣ *BufEnd,
                        ತಾರ್ಕಿಕ RequiresNullTerminator) {
  ಪ್ರತಿಜ್ಞಾ((!RequiresNullTerminator || BufEnd[0] == 0) &&
         "Buffer is not null terminated!");
  BufferStart = BufStart;
  BufferEnd = BufEnd;
}

//===----------------------------------------------------------------------===//
// MemoryBufferMem implementation.
//===----------------------------------------------------------------------===//

/// CopyStringRef - Copies contents of a StringRef into a block of memory and
/// null-terminates it.
ಸ್ಥಿರ ಶೂನ್ಯ CopyStringRef(ವರ್ಣ *Memory, StringRef Data) {
  ಯದಿ (!Data.empty())
    memcpy(Memory, Data.data(), Data.size());
  Memory[Data.size()] = 0; // Null terminate string.
}

ನಾಮಾಕಾಶ {
ಸಂರಚನಾ NamedBufferAlloc {
  ಧ್ರುವ Twine &Name;
  NamedBufferAlloc(ಧ್ರುವ Twine &Name) : Name(Name) {}
};
} // namespace

ಶೂನ್ಯ *operator ನೂತನ(size_t N, ಧ್ರುವ NamedBufferAlloc &Alloc) {
  SmallString<256> NameBuf;
  StringRef NameRef = Alloc.Name.toStringRef(NameBuf);

  // We use malloc() and manually handle it returning null instead of calling
  // operator new because we need all uses of NamedBufferAlloc to be
  // deallocated with a call to free() due to needing to use malloc() in
  // WritableMemoryBuffer::getNewUninitMemBuffer() to work around the out-of-
  // memory handler installed by default in LLVM. See operator delete() member
  // functions within this file for the paired call to free().
  ವರ್ಣ *Mem =
      static_cast<ವರ್ಣ *>(std::malloc(N + ಪರಿಮಾಣ(size_t) + NameRef.size() + 1));
  ಯದಿ (!Mem)
    llvm::report_bad_alloc_error("Allocation failed");
  *reinterpret_cast<size_t *>(Mem + N) = NameRef.size();
  CopyStringRef(Mem + N + ಪರಿಮಾಣ(size_t), NameRef);
  ಪ್ರತ್ಯಾಗಮ Mem;
}

ನಾಮಾಕಾಶ {
/// MemoryBufferMem - Named MemoryBuffer pointing to a block of memory.
template<typename MB>
ವರ್ಗ MemoryBufferMem : ಸಾರ್ವಜನಿಕ MB {
ಸಾರ್ವಜನಿಕ:
  MemoryBufferMem(StringRef ಒಳಹರಿವು_Data, ತಾರ್ಕಿಕ RequiresNullTerminator) {
    MemoryBuffer::init(ಒಳಹರಿವು_Data.begin(), ಒಳಹರಿವು_Data.end(),
                       RequiresNullTerminator);
  }

  /// Disable sized deallocation for MemoryBufferMem, because it has
  /// tail-allocated data.
  ಶೂನ್ಯ operator delete(ಶೂನ್ಯ *p) { std::ಮುಕ್ತ(p); }

  StringRef getBufferIdentifier() ಧ್ರುವ override {
    // The name is stored after the class itself.
    ಪ್ರತ್ಯಾಗಮ StringRef(reinterpret_cast<ಧ್ರುವ ವರ್ಣ *>(ಏತತ್ + 1) + ಪರಿಮಾಣ(size_t),
                     *reinterpret_cast<ಧ್ರುವ size_t *>(ಏತತ್ + 1));
  }

  MemoryBuffer::BufferKind getBufferKind() ಧ್ರುವ override {
    ಪ್ರತ್ಯಾಗಮ MemoryBuffer::MemoryBuffer_Malloc;
  }
};
} // namespace

template <typename MB>
ಸ್ಥಿರ ErrorOr<std::unique_ptr<MB>>
getFileAux(ಧ್ರುವ Twine &Filename, uint64_t MapSize, uint64_t Offset,
           ತಾರ್ಕಿಕ IsText, ತಾರ್ಕಿಕ RequiresNullTerminator, ತಾರ್ಕಿಕ IsVolatile,
           std::optional<Align> Alignment);

std::unique_ptr<MemoryBuffer>
MemoryBuffer::getMemBuffer(StringRef ಒಳಹರಿವು_Data, StringRef BufferName,
                           ತಾರ್ಕಿಕ RequiresNullTerminator) {
  auto *Ret = ನೂತನ (NamedBufferAlloc(BufferName))
      MemoryBufferMem<MemoryBuffer>(ಒಳಹರಿವು_Data, RequiresNullTerminator);
  ಪ್ರತ್ಯಾಗಮ std::unique_ptr<MemoryBuffer>(Ret);
}

std::unique_ptr<MemoryBuffer>
MemoryBuffer::getMemBuffer(MemoryBufferRef Ref, ತಾರ್ಕಿಕ RequiresNullTerminator) {
  ಪ್ರತ್ಯಾಗಮ std::unique_ptr<MemoryBuffer>(getMemBuffer(
      Ref.getBuffer(), Ref.getBufferIdentifier(), RequiresNullTerminator));
}

ಸ್ಥಿರ ErrorOr<std::unique_ptr<WritableMemoryBuffer>>
getMemBufferCopyImpl(StringRef ಒಳಹರಿವು_Data, ಧ್ರುವ Twine &BufferName) {
  auto Buf =
      WritableMemoryBuffer::getNewUninitMemBuffer(ಒಳಹರಿವು_Data.size(), BufferName);
  ಯದಿ (!Buf)
    ಪ್ರತ್ಯಾಗಮ make_error_code(errc::not_enough_memory);
  // Calling memcpy with null src/dst is UB, and an empty StringRef is
  // represented with {nullptr, 0}.
  llvm::copy(ಒಳಹರಿವು_Data, Buf->getBufferStart());
  ಪ್ರತ್ಯಾಗಮ std::move(Buf);
}

std::unique_ptr<MemoryBuffer>
MemoryBuffer::getMemBufferCopy(StringRef ಒಳಹರಿವು_Data, ಧ್ರುವ Twine &BufferName) {
  auto Buf = getMemBufferCopyImpl(ಒಳಹರಿವು_Data, BufferName);
  ಯದಿ (Buf)
    ಪ್ರತ್ಯಾಗಮ std::move(*Buf);
  ಪ್ರತ್ಯಾಗಮ nullptr;
}

ErrorOr<std::unique_ptr<MemoryBuffer>>
MemoryBuffer::getFileOrSTDIN(ಧ್ರುವ Twine &Filename, ತಾರ್ಕಿಕ IsText,
                             ತಾರ್ಕಿಕ RequiresNullTerminator,
                             std::optional<Align> Alignment) {
  sys::sandbox::violationIfEnabled();

  SmallString<256> NameBuf;
  StringRef NameRef = Filename.toStringRef(NameBuf);

  ಯದಿ (NameRef == "-")
    ಪ್ರತ್ಯಾಗಮ getSTDIN();
  ಪ್ರತ್ಯಾಗಮ getFile(Filename, IsText, RequiresNullTerminator,
                 /*IsVolatile=*/ಅಸತ್ಯ, Alignment);
}

ErrorOr<std::unique_ptr<MemoryBuffer>>
MemoryBuffer::getFileSlice(ಧ್ರುವ Twine &FilePath, uint64_t MapSize,
                           uint64_t Offset, ತಾರ್ಕಿಕ IsVolatile,
                           std::optional<Align> Alignment) {
  sys::sandbox::violationIfEnabled();

  ಪ್ರತ್ಯಾಗಮ getFileAux<MemoryBuffer>(FilePath, MapSize, Offset, /*IsText=*/ಅಸತ್ಯ,
                                  /*RequiresNullTerminator=*/ಅಸತ್ಯ, IsVolatile,
                                  Alignment);
}

//===----------------------------------------------------------------------===//
// MemoryBuffer::getFile implementation.
//===----------------------------------------------------------------------===//

ನಾಮಾಕಾಶ {

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
ವರ್ಗ MemoryBufferMMapFile : ಸಾರ್ವಜನಿಕ MB {
  sys::fs::mapped_file_region MFR;

  ಸ್ಥಿರ uint64_t getLegalMapOffset(uint64_t Offset) {
    ಪ್ರತ್ಯಾಗಮ Offset & ~(sys::fs::mapped_file_region::alignment() - 1);
  }

  ಸ್ಥಿರ uint64_t getLegalMapSize(uint64_t Len, uint64_t Offset) {
    ಪ್ರತ್ಯಾಗಮ Len + (Offset - getLegalMapOffset(Offset));
  }

  ಧ್ರುವ ವರ್ಣ *getStart(uint64_t Len, uint64_t Offset) {
    ಪ್ರತ್ಯಾಗಮ MFR.const_data() + (Offset - getLegalMapOffset(Offset));
  }

ಸಾರ್ವಜನಿಕ:
  MemoryBufferMMapFile(ತಾರ್ಕಿಕ RequiresNullTerminator, sys::fs::file_t FD, uint64_t Len,
                       uint64_t Offset, std::error_code &EC)
      : MFR(FD, Mapmode<MB>, getLegalMapSize(Len, Offset),
            getLegalMapOffset(Offset), EC) {
    ಯದಿ (!EC) {
      ಧ್ರುವ ವರ್ಣ *Start = getStart(Len, Offset);
      MemoryBuffer::init(Start, Start + Len, RequiresNullTerminator);
    }
  }

  /// Disable sized deallocation for MemoryBufferMMapFile, because it has
  /// tail-allocated data.
  ಶೂನ್ಯ operator delete(ಶೂನ್ಯ *p) { std::ಮುಕ್ತ(p); }

  StringRef getBufferIdentifier() ಧ್ರುವ override {
    // The name is stored after the class itself.
    ಪ್ರತ್ಯಾಗಮ StringRef(reinterpret_cast<ಧ್ರುವ ವರ್ಣ *>(ಏತತ್ + 1) + ಪರಿಮಾಣ(size_t),
                     *reinterpret_cast<ಧ್ರುವ size_t *>(ಏತತ್ + 1));
  }

  MemoryBuffer::BufferKind getBufferKind() ಧ್ರುವ override {
    ಪ್ರತ್ಯಾಗಮ MemoryBuffer::MemoryBuffer_MMap;
  }

  ಶೂನ್ಯ dontNeedIfMmap() override { MFR.dontNeed(); }
  ಶೂನ್ಯ willNeedIfMmap() override { MFR.willNeed(); }
  ಶೂನ್ಯ randomAccessIfMmap() override { MFR.randomAccess(); }
};
} // namespace

ಸ್ಥಿರ ErrorOr<std::unique_ptr<WritableMemoryBuffer>>
getMemoryBufferForStream(sys::fs::file_t FD, ಧ್ರುವ Twine &BufferName) {
  SmallString<sys::fs::DefaultReadChunkSize> Buffer;
  ಯದಿ (Error E = sys::fs::readNativeFileToEOF(FD, Buffer))
    ಪ್ರತ್ಯಾಗಮ errorToErrorCode(std::move(E));
  ಪ್ರತ್ಯಾಗಮ getMemBufferCopyImpl(Buffer, BufferName);
}

ErrorOr<std::unique_ptr<MemoryBuffer>>
MemoryBuffer::getFile(ಧ್ರುವ Twine &Filename, ತಾರ್ಕಿಕ IsText,
                      ತಾರ್ಕಿಕ RequiresNullTerminator, ತಾರ್ಕಿಕ IsVolatile,
                      std::optional<Align> Alignment) {
  sys::sandbox::violationIfEnabled();

  ಪ್ರತ್ಯಾಗಮ getFileAux<MemoryBuffer>(Filename, /*MapSize=*/-1, /*Offset=*/0,
                                  IsText, RequiresNullTerminator, IsVolatile,
                                  Alignment);
}

template <typename MB>
ಸ್ಥಿರ ErrorOr<std::unique_ptr<MB>>
getOpenFileImpl(sys::fs::file_t FD, ಧ್ರುವ Twine &Filename, uint64_t FileSize,
                uint64_t MapSize, int64_t Offset, ತಾರ್ಕಿಕ RequiresNullTerminator,
                ತಾರ್ಕಿಕ IsVolatile, std::optional<Align> Alignment);

template <typename MB>
ಸ್ಥಿರ ErrorOr<std::unique_ptr<MB>>
getFileAux(ಧ್ರುವ Twine &Filename, uint64_t MapSize, uint64_t Offset,
           ತಾರ್ಕಿಕ IsText, ತಾರ್ಕಿಕ RequiresNullTerminator, ತಾರ್ಕಿಕ IsVolatile,
           std::optional<Align> Alignment) {
  Expected<sys::fs::file_t> FDOrErr = sys::fs::openNativeFileForRead(
      Filename, IsText ? sys::fs::OF_TextWithCRLF : sys::fs::OF_None);
  ಯದಿ (!FDOrErr)
    ಪ್ರತ್ಯಾಗಮ errorToErrorCode(FDOrErr.takeError());
  sys::fs::file_t FD = *FDOrErr;
  auto Ret = getOpenFileImpl<MB>(FD, Filename, /*FileSize=*/-1, MapSize, Offset,
                                 RequiresNullTerminator, IsVolatile, Alignment);
  sys::fs::closeFile(FD);
  ಪ್ರತ್ಯಾಗಮ Ret;
}

ErrorOr<std::unique_ptr<WritableMemoryBuffer>>
WritableMemoryBuffer::getFile(ಧ್ರುವ Twine &Filename, ತಾರ್ಕಿಕ IsVolatile,
                              std::optional<Align> Alignment) {
  sys::sandbox::violationIfEnabled();

  ಪ್ರತ್ಯಾಗಮ getFileAux<WritableMemoryBuffer>(
      Filename, /*MapSize=*/-1, /*Offset=*/0, /*IsText=*/ಅಸತ್ಯ,
      /*RequiresNullTerminator=*/ಅಸತ್ಯ, IsVolatile, Alignment);
}

ErrorOr<std::unique_ptr<WritableMemoryBuffer>>
WritableMemoryBuffer::getFileSlice(ಧ್ರುವ Twine &Filename, uint64_t MapSize,
                                   uint64_t Offset, ತಾರ್ಕಿಕ IsVolatile,
                                   std::optional<Align> Alignment) {
  sys::sandbox::violationIfEnabled();

  ಪ್ರತ್ಯಾಗಮ getFileAux<WritableMemoryBuffer>(
      Filename, MapSize, Offset, /*IsText=*/ಅಸತ್ಯ,
      /*RequiresNullTerminator=*/ಅಸತ್ಯ, IsVolatile, Alignment);
}

std::unique_ptr<WritableMemoryBuffer>
WritableMemoryBuffer::getNewUninitMemBuffer(size_t Size,
                                            ಧ್ರುವ Twine &BufferName,
                                            std::optional<Align> Alignment) {
  using MemBuffer = MemoryBufferMem<WritableMemoryBuffer>;

  // Use 16-byte alignment if no alignment is specified.
  Align BufAlign = Alignment.ಮೌಲ್ಯ_or(Align(16));

  // Allocate space for the MemoryBuffer, the data and the name. It is important
  // that MemoryBuffer and data are aligned so PointerIntPair works with them.
  SmallString<256> NameBuf;
  StringRef NameRef = BufferName.toStringRef(NameBuf);

  size_t StringLen = ಪರಿಮಾಣ(MemBuffer) + ಪರಿಮಾಣ(size_t) + NameRef.size() + 1;
  size_t RealLen = StringLen + Size + 1 + BufAlign.ಮೌಲ್ಯ_2();
  ಯದಿ (RealLen <= Size) // Check for rollover.
    ಪ್ರತ್ಯಾಗಮ nullptr;
  // We use a call to malloc() rather than a call to a non-throwing operator
  // new() because LLVM unconditionally installs an out of memory new handler
  // when exceptions are disabled. This new handler intentionally crashes to
  // aid with debugging, but that makes non-throwing new calls unhelpful.
  // See MemoryBufferMem::operator delete() for the paired call to free(), and
  // llvm::install_out_of_memory_new_handler() for the installation of the
  // custom new handler.
  ವರ್ಣ *Mem = static_cast<ವರ್ಣ *>(std::malloc(RealLen));
  ಯದಿ (!Mem)
    ಪ್ರತ್ಯಾಗಮ nullptr;

  // The name is stored after the class itself.
  *reinterpret_cast<size_t *>(Mem + ಪರಿಮಾಣ(MemBuffer)) = NameRef.size();
  CopyStringRef(Mem + ಪರಿಮಾಣ(MemBuffer) + ಪರಿಮಾಣ(size_t), NameRef);

  // The buffer begins after the name and must be aligned.
  ವರ್ಣ *Buf = (ವರ್ಣ *)alignAddr(Mem + StringLen, BufAlign);
  Buf[Size] = 0; // Null terminate buffer.

  auto *Ret = ನೂತನ (Mem) MemBuffer(StringRef(Buf, Size), ಸತ್ಯ);
  ಪ್ರತ್ಯಾಗಮ std::unique_ptr<WritableMemoryBuffer>(Ret);
}

std::unique_ptr<WritableMemoryBuffer>
WritableMemoryBuffer::getNewMemBuffer(size_t Size, ಧ್ರುವ Twine &BufferName) {
  auto SB = WritableMemoryBuffer::getNewUninitMemBuffer(Size, BufferName);
  ಯದಿ (!SB)
    ಪ್ರತ್ಯಾಗಮ nullptr;
  memset(SB->getBufferStart(), 0, Size);
  ಪ್ರತ್ಯಾಗಮ SB;
}

ಸ್ಥಿರ ತಾರ್ಕಿಕ shouldUseMmap(sys::fs::file_t FD,
                          size_t FileSize,
                          size_t MapSize,
                          off_t Offset,
                          ತಾರ್ಕಿಕ RequiresNullTerminator,
                          ಪೂರ್ಣಾಙ್ಕ PageSize,
                          ತಾರ್ಕಿಕ IsVolatile) {
#ಯದಿ defined(__MVS__)
  // zOS Enhanced ASCII auto convert does not support mmap.
  ಪ್ರತ್ಯಾಗಮ ಅಸತ್ಯ;
#endif

  // mmap may leave the buffer without null terminator if the file size changed
  // by the time the last page is mapped in, so avoid it if the file size is
  // likely to change.
  ಯದಿ (IsVolatile && RequiresNullTerminator)
    ಪ್ರತ್ಯಾಗಮ ಅಸತ್ಯ;

  // We don't use mmap for small files because this can severely fragment our
  // address space.
  ಯದಿ (MapSize < 4 * 4096 || MapSize < (unsigned)PageSize)
    ಪ್ರತ್ಯಾಗಮ ಅಸತ್ಯ;

  ಯದಿ (!RequiresNullTerminator)
    ಪ್ರತ್ಯಾಗಮ ಸತ್ಯ;

  // If we don't know the file size, use fstat to find out.  fstat on an open
  // file descriptor is cheaper than stat on a random path.
  // FIXME: this chunk of code is duplicated, but it avoids a fstat when
  // RequiresNullTerminator = false and MapSize != -1.
  ಯದಿ (FileSize == size_t(-1)) {
    sys::fs::file_status Status;
    ಯದಿ (sys::fs::status(FD, Status))
      ಪ್ರತ್ಯಾಗಮ ಅಸತ್ಯ;
    FileSize = Status.getSize();
  }

  // If we need a null terminator and the end of the map is inside the file,
  // we cannot use mmap.
  size_t End = Offset + MapSize;
  ಪ್ರತಿಜ್ಞಾ(End <= FileSize);
  ಯದಿ (End != FileSize)
    ಪ್ರತ್ಯಾಗಮ ಅಸತ್ಯ;

  // Don't try to map files that are exactly a multiple of the system page size
  // if we need a null terminator.
  ಯದಿ ((FileSize & (PageSize -1)) == 0)
    ಪ್ರತ್ಯಾಗಮ ಅಸತ್ಯ;

#ಯದಿ defined(__CYGWIN__)
  // Don't try to map files that are exactly a multiple of the physical page size
  // if we need a null terminator.
  // FIXME: We should reorganize again getPageSize() on Win32.
  ಯದಿ ((FileSize & (4096 - 1)) == 0)
    ಪ್ರತ್ಯಾಗಮ ಅಸತ್ಯ;
#endif

  ಪ್ರತ್ಯಾಗಮ ಸತ್ಯ;
}

ಸ್ಥಿರ ErrorOr<std::unique_ptr<WriteThroughMemoryBuffer>>
getReadWriteFile(ಧ್ರುವ Twine &Filename, uint64_t FileSize, uint64_t MapSize,
                 uint64_t Offset) {
  Expected<sys::fs::file_t> FDOrErr = sys::fs::openNativeFileForReadWrite(
      Filename, sys::fs::CD_OpenExisting, sys::fs::OF_None);
  ಯದಿ (!FDOrErr)
    ಪ್ರತ್ಯಾಗಮ errorToErrorCode(FDOrErr.takeError());
  sys::fs::file_t FD = *FDOrErr;

  // Default is to map the full file.
  ಯದಿ (MapSize == uint64_t(-1)) {
    // If we don't know the file size, use fstat to find out.  fstat on an open
    // file descriptor is cheaper than stat on a random path.
    ಯದಿ (FileSize == uint64_t(-1)) {
      sys::fs::file_status Status;
      std::error_code EC = sys::fs::status(FD, Status);
      ಯದಿ (EC)
        ಪ್ರತ್ಯಾಗಮ EC;

      // If this not a file or a block device (e.g. it's a named pipe
      // or character device), we can't mmap it, so error out.
      sys::fs::file_type Type = Status.type();
      ಯದಿ (Type != sys::fs::file_type::regular_file &&
          Type != sys::fs::file_type::block_file)
        ಪ್ರತ್ಯಾಗಮ make_error_code(errc::invalid_argument);

      FileSize = Status.getSize();
    }
    MapSize = FileSize;
  }

  std::error_code EC;
  std::unique_ptr<WriteThroughMemoryBuffer> Result(
      ನೂತನ (NamedBufferAlloc(Filename))
          MemoryBufferMMapFile<WriteThroughMemoryBuffer>(ಅಸತ್ಯ, FD, MapSize,
                                                         Offset, EC));
  ಯದಿ (EC)
    ಪ್ರತ್ಯಾಗಮ EC;
  ಪ್ರತ್ಯಾಗಮ std::move(Result);
}

ErrorOr<std::unique_ptr<WriteThroughMemoryBuffer>>
WriteThroughMemoryBuffer::getFile(ಧ್ರುವ Twine &Filename, int64_t FileSize) {
  sys::sandbox::violationIfEnabled();

  ಪ್ರತ್ಯಾಗಮ getReadWriteFile(Filename, FileSize, FileSize, 0);
}

/// Map a subrange of the specified file as a WritableMemoryBuffer.
ErrorOr<std::unique_ptr<WriteThroughMemoryBuffer>>
WriteThroughMemoryBuffer::getFileSlice(ಧ್ರುವ Twine &Filename, uint64_t MapSize,
                                       uint64_t Offset) {
  sys::sandbox::violationIfEnabled();

  ಪ್ರತ್ಯಾಗಮ getReadWriteFile(Filename, -1, MapSize, Offset);
}

template <typename MB>
ಸ್ಥಿರ ErrorOr<std::unique_ptr<MB>>
getOpenFileImpl(sys::fs::file_t FD, ಧ್ರುವ Twine &Filename, uint64_t FileSize,
                uint64_t MapSize, int64_t Offset, ತಾರ್ಕಿಕ RequiresNullTerminator,
                ತಾರ್ಕಿಕ IsVolatile, std::optional<Align> Alignment) {
  ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ PageSize = sys::Process::getPageSizeEstimate();

  // Default is to map the full file.
  ಯದಿ (MapSize == uint64_t(-1)) {
    // If we don't know the file size, use fstat to find out.  fstat on an open
    // file descriptor is cheaper than stat on a random path.
    ಯದಿ (FileSize == uint64_t(-1)) {
      sys::fs::file_status Status;
      std::error_code EC = sys::fs::status(FD, Status);
      ಯದಿ (EC)
        ಪ್ರತ್ಯಾಗಮ EC;

      // If this not a file or a block device (e.g. it's a named pipe
      // or character device), we can't trust the size. Create the memory
      // buffer by copying off the stream.
      sys::fs::file_type Type = Status.type();
      ಯದಿ (Type != sys::fs::file_type::regular_file &&
          Type != sys::fs::file_type::block_file)
        ಪ್ರತ್ಯಾಗಮ getMemoryBufferForStream(FD, Filename);

      FileSize = Status.getSize();
    }
    MapSize = FileSize;
  }

  ಯದಿ (shouldUseMmap(FD, FileSize, MapSize, Offset, RequiresNullTerminator,
                    PageSize, IsVolatile)) {
    std::error_code EC;
    std::unique_ptr<MB> Result(
        ನೂತನ (NamedBufferAlloc(Filename)) MemoryBufferMMapFile<MB>(
            RequiresNullTerminator, FD, MapSize, Offset, EC));
    ಯದಿ (!EC) {
      // On at least Linux, and possibly on other systems, mmap may return pages
      // from the page cache that are not properly filled with trailing zeroes,
      // if some prior user of the page wrote non-zero bytes. Detect this and
      // don't use mmap in that case.
      ಯದಿ (!RequiresNullTerminator || *Result->getBufferEnd() == '\0')
        ಪ್ರತ್ಯಾಗಮ std::move(Result);
    }
  }

#ifdef __MVS__
  ErrorOr<ತಾರ್ಕಿಕ> NeedsConversion = needConversion(Filename, FD);
  ಯದಿ (std::error_code EC = NeedsConversion.getError())
    ಪ್ರತ್ಯಾಗಮ EC;
  // File size may increase due to EBCDIC -> UTF-8 conversion, therefore we
  // cannot trust the file size and we create the memory buffer by copying
  // off the stream.
  // Note: This only works with the assumption of reading a full file (i.e,
  // Offset == 0 and MapSize == FileSize). Reading a file slice does not work.
  ಯದಿ (*NeedsConversion && Offset == 0 && MapSize == FileSize)
    ಪ್ರತ್ಯಾಗಮ getMemoryBufferForStream(FD, Filename);
#endif

  auto Buf =
      WritableMemoryBuffer::getNewUninitMemBuffer(MapSize, Filename, Alignment);
  ಯದಿ (!Buf) {
    // Failed to create a buffer. The only way it can fail is if
    // new(std::nothrow) returns 0.
    ಪ್ರತ್ಯಾಗಮ make_error_code(errc::not_enough_memory);
  }

  // Read until EOF, zero-initialize the rest.
  MutableArrayRef<ವರ್ಣ> ToRead = Buf->getBuffer();
  ಯಾವತ್ (!ToRead.empty()) {
    Expected<size_t> ReadBytes =
        sys::fs::readNativeFileSlice(FD, ToRead, Offset);
    ಯದಿ (!ReadBytes)
      ಪ್ರತ್ಯಾಗಮ errorToErrorCode(ReadBytes.takeError());
    ಯದಿ (*ReadBytes == 0) {
      std::memset(ToRead.data(), 0, ToRead.size());
      ಭಙ್ಗ;
    }
    ToRead = ToRead.drop_front(*ReadBytes);
    Offset += *ReadBytes;
  }

  ಪ್ರತ್ಯಾಗಮ std::move(Buf);
}

ErrorOr<std::unique_ptr<MemoryBuffer>>
MemoryBuffer::getOpenFile(sys::fs::file_t FD, ಧ್ರುವ Twine &Filename,
                          uint64_t FileSize, ತಾರ್ಕಿಕ RequiresNullTerminator,
                          ತಾರ್ಕಿಕ IsVolatile, std::optional<Align> Alignment) {
  sys::sandbox::violationIfEnabled();

  ಪ್ರತ್ಯಾಗಮ getOpenFileImpl<MemoryBuffer>(FD, Filename, FileSize, FileSize, 0,
                                       RequiresNullTerminator, IsVolatile,
                                       Alignment);
}

ErrorOr<std::unique_ptr<MemoryBuffer>> MemoryBuffer::getOpenFileSlice(
    sys::fs::file_t FD, ಧ್ರುವ Twine &Filename, uint64_t MapSize, int64_t Offset,
    ತಾರ್ಕಿಕ IsVolatile, std::optional<Align> Alignment) {
  ಪ್ರತಿಜ್ಞಾ(MapSize != uint64_t(-1));

  sys::sandbox::violationIfEnabled();

  ಪ್ರತ್ಯಾಗಮ getOpenFileImpl<MemoryBuffer>(FD, Filename, -1, MapSize, Offset, ಅಸತ್ಯ,
                                       IsVolatile, Alignment);
}

ErrorOr<std::unique_ptr<MemoryBuffer>> MemoryBuffer::getSTDIN() {
  sys::sandbox::violationIfEnabled();

  // Read in all of the data from stdin, we cannot mmap stdin.
  //
  // FIXME: That isn't necessarily true, we should try to mmap stdin and
  // fallback if it fails.
  sys::ChangeStdinMode(sys::fs::OF_Text);

  ಪ್ರತ್ಯಾಗಮ getMemoryBufferForStream(sys::fs::getStdinHandle(), "<stdin>");
}

ErrorOr<std::unique_ptr<MemoryBuffer>>
MemoryBuffer::getFileAsStream(ಧ್ರುವ Twine &Filename) {
  sys::sandbox::violationIfEnabled();

  Expected<sys::fs::file_t> FDOrErr =
      sys::fs::openNativeFileForRead(Filename, sys::fs::OF_None);
  ಯದಿ (!FDOrErr)
    ಪ್ರತ್ಯಾಗಮ errorToErrorCode(FDOrErr.takeError());
  sys::fs::file_t FD = *FDOrErr;
  ErrorOr<std::unique_ptr<MemoryBuffer>> Ret =
      getMemoryBufferForStream(FD, Filename);
  sys::fs::closeFile(FD);
  ಪ್ರತ್ಯಾಗಮ Ret;
}

MemoryBufferRef MemoryBuffer::getMemBufferRef() ಧ್ರುವ {
  StringRef Data = getBuffer();
  StringRef Identifier = getBufferIdentifier();
  ಪ್ರತ್ಯಾಗಮ MemoryBufferRef(Data, Identifier);
}

SmallVectorMemoryBuffer::~SmallVectorMemoryBuffer() = ಸ್ವಭಾವ;
