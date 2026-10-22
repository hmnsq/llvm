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

#ଅନ୍ତର୍ଭାଵ "llvm/Support/MemoryBuffer.h"
#ଅନ୍ତର୍ଭାଵ "llvm/ADT/STLExtras.h"
#ଅନ୍ତର୍ଭାଵ "llvm/ADT/SmallString.h"
#ଅନ୍ତର୍ଭାଵ "llvm/Config/config.h"
#ଅନ୍ତର୍ଭାଵ "llvm/Support/Alignment.h"
#ଅନ୍ତର୍ଭାଵ "llvm/Support/AutoConvert.h"
#ଅନ୍ତର୍ଭାଵ "llvm/Support/Errc.h"
#ଅନ୍ତର୍ଭାଵ "llvm/Support/Error.h"
#ଅନ୍ତର୍ଭାଵ "llvm/Support/ErrorHandling.h"
#ଅନ୍ତର୍ଭାଵ "llvm/Support/FileSystem.h"
#ଅନ୍ତର୍ଭାଵ "llvm/Support/IOSandbox.h"
#ଅନ୍ତର୍ଭାଵ "llvm/Support/Process.h"
#ଅନ୍ତର୍ଭାଵ "llvm/Support/Program.h"
#ଅନ୍ତର୍ଭାଵ "llvm/Support/SmallVectorMemoryBuffer.h"
#ଅନ୍ତର୍ଭାଵ <algorithm>
#ଅନ୍ତର୍ଭାଵ <cassert>
#ଅନ୍ତର୍ଭାଵ <cstring>
#ଅନ୍ତର୍ଭାଵ <new>
#ଅନ୍ତର୍ଭାଵ <sys/types.h>
#ଅନ୍ତର୍ଭାଵ <system_error>
#ଯଦି !defined(_MSC_VER) && !defined(__MINGW32__)
#ଅନ୍ତର୍ଭାଵ <unistd.h>
#ଅନ୍ୟଥା
#ଅନ୍ତର୍ଭାଵ <io.h>
#endif

using ନାମାକାଶ llvm;

//===----------------------------------------------------------------------===//
// MemoryBuffer implementation itself.
//===----------------------------------------------------------------------===//

MemoryBuffer::~MemoryBuffer() = ପୂର୍ବନିର୍ଦ୍ଧାରିତ;

/// init - Initialize this MemoryBuffer as a reference to externally allocated
/// memory, memory that we know is already null terminated.
ଶୂନ୍ୟ MemoryBuffer::init(ଅଚଳ ଅକ୍ଷର *BufStart, ଅଚଳ ଅକ୍ଷର *BufEnd,
                        ତାର୍କିକ RequiresNullTerminator) {
  ପ୍ରତିଜ୍ଞା((!RequiresNullTerminator || BufEnd[0] == 0) &&
         "Buffer is not null terminated!");
  BufferStart = BufStart;
  BufferEnd = BufEnd;
}

//===----------------------------------------------------------------------===//
// MemoryBufferMem implementation.
//===----------------------------------------------------------------------===//

/// CopyStringRef - Copies contents of a StringRef into a block of memory and
/// null-terminates it.
ସ୍ଥିର ଶୂନ୍ୟ CopyStringRef(ଅକ୍ଷର *Memory, StringRef Data) {
  ଯଦି (!Data.empty())
    memcpy(Memory, Data.data(), Data.size());
  Memory[Data.size()] = 0; // Null terminate string.
}

ନାମାକାଶ {
ସଂରଚନା NamedBufferAlloc {
  ଅଚଳ Twine &Name;
  NamedBufferAlloc(ଅଚଳ Twine &Name) : Name(Name) {}
};
} // namespace

ଶୂନ୍ୟ *operator ନୂତନ(size_t N, ଅଚଳ NamedBufferAlloc &Alloc) {
  SmallString<256> NameBuf;
  StringRef NameRef = Alloc.Name.toStringRef(NameBuf);

  // We use malloc() and manually handle it returning null instead of calling
  // operator new because we need all uses of NamedBufferAlloc to be
  // deallocated with a call to free() due to needing to use malloc() in
  // WritableMemoryBuffer::getNewUninitMemBuffer() to work around the out-of-
  // memory handler installed by default in LLVM. See operator delete() member
  // functions within this file for the paired call to free().
  ଅକ୍ଷର *Mem =
      static_cast<ଅକ୍ଷର *>(std::malloc(N + ଆକାର(size_t) + NameRef.size() + 1));
  ଯଦି (!Mem)
    llvm::report_bad_alloc_error("Allocation failed");
  *reinterpret_cast<size_t *>(Mem + N) = NameRef.size();
  CopyStringRef(Mem + N + ଆକାର(size_t), NameRef);
  ଫେରନ୍ତୁ Mem;
}

ନାମାକାଶ {
/// MemoryBufferMem - Named MemoryBuffer pointing to a block of memory.
template<typename MB>
ଶ୍ରେଣୀ MemoryBufferMem : ସର୍ବସାଧାରଣ MB {
ସର୍ବସାଧାରଣ:
  MemoryBufferMem(StringRef ନିବେଶ_Data, ତାର୍କିକ RequiresNullTerminator) {
    MemoryBuffer::init(ନିବେଶ_Data.begin(), ନିବେଶ_Data.end(),
                       RequiresNullTerminator);
  }

  /// Disable sized deallocation for MemoryBufferMem, because it has
  /// tail-allocated data.
  ଶୂନ୍ୟ operator delete(ଶୂନ୍ୟ *p) { std::ମୁକ୍ତ(p); }

  StringRef getBufferIdentifier() ଅଚଳ override {
    // The name is stored after the class itself.
    ଫେରନ୍ତୁ StringRef(reinterpret_cast<ଅଚଳ ଅକ୍ଷର *>(ଏହି + 1) + ଆକାର(size_t),
                     *reinterpret_cast<ଅଚଳ size_t *>(ଏହି + 1));
  }

  MemoryBuffer::BufferKind getBufferKind() ଅଚଳ override {
    ଫେରନ୍ତୁ MemoryBuffer::MemoryBuffer_Malloc;
  }
};
} // namespace

template <typename MB>
ସ୍ଥିର ErrorOr<std::unique_ptr<MB>>
getFileAux(ଅଚଳ Twine &Filename, uint64_t MapSize, uint64_t Offset,
           ତାର୍କିକ IsText, ତାର୍କିକ RequiresNullTerminator, ତାର୍କିକ IsVolatile,
           std::optional<Align> Alignment);

std::unique_ptr<MemoryBuffer>
MemoryBuffer::getMemBuffer(StringRef ନିବେଶ_Data, StringRef BufferName,
                           ତାର୍କିକ RequiresNullTerminator) {
  auto *Ret = ନୂତନ (NamedBufferAlloc(BufferName))
      MemoryBufferMem<MemoryBuffer>(ନିବେଶ_Data, RequiresNullTerminator);
  ଫେରନ୍ତୁ std::unique_ptr<MemoryBuffer>(Ret);
}

std::unique_ptr<MemoryBuffer>
MemoryBuffer::getMemBuffer(MemoryBufferRef Ref, ତାର୍କିକ RequiresNullTerminator) {
  ଫେରନ୍ତୁ std::unique_ptr<MemoryBuffer>(getMemBuffer(
      Ref.getBuffer(), Ref.getBufferIdentifier(), RequiresNullTerminator));
}

ସ୍ଥିର ErrorOr<std::unique_ptr<WritableMemoryBuffer>>
getMemBufferCopyImpl(StringRef ନିବେଶ_Data, ଅଚଳ Twine &BufferName) {
  auto Buf =
      WritableMemoryBuffer::getNewUninitMemBuffer(ନିବେଶ_Data.size(), BufferName);
  ଯଦି (!Buf)
    ଫେରନ୍ତୁ make_error_code(errc::not_enough_memory);
  // Calling memcpy with null src/dst is UB, and an empty StringRef is
  // represented with {nullptr, 0}.
  llvm::copy(ନିବେଶ_Data, Buf->getBufferStart());
  ଫେରନ୍ତୁ std::move(Buf);
}

std::unique_ptr<MemoryBuffer>
MemoryBuffer::getMemBufferCopy(StringRef ନିବେଶ_Data, ଅଚଳ Twine &BufferName) {
  auto Buf = getMemBufferCopyImpl(ନିବେଶ_Data, BufferName);
  ଯଦି (Buf)
    ଫେରନ୍ତୁ std::move(*Buf);
  ଫେରନ୍ତୁ nullptr;
}

ErrorOr<std::unique_ptr<MemoryBuffer>>
MemoryBuffer::getFileOrSTDIN(ଅଚଳ Twine &Filename, ତାର୍କିକ IsText,
                             ତାର୍କିକ RequiresNullTerminator,
                             std::optional<Align> Alignment) {
  sys::sandbox::violationIfEnabled();

  SmallString<256> NameBuf;
  StringRef NameRef = Filename.toStringRef(NameBuf);

  ଯଦି (NameRef == "-")
    ଫେରନ୍ତୁ getSTDIN();
  ଫେରନ୍ତୁ getFile(Filename, IsText, RequiresNullTerminator,
                 /*IsVolatile=*/ମିଥ୍ୟା, Alignment);
}

ErrorOr<std::unique_ptr<MemoryBuffer>>
MemoryBuffer::getFileSlice(ଅଚଳ Twine &FilePath, uint64_t MapSize,
                           uint64_t Offset, ତାର୍କିକ IsVolatile,
                           std::optional<Align> Alignment) {
  sys::sandbox::violationIfEnabled();

  ଫେରନ୍ତୁ getFileAux<MemoryBuffer>(FilePath, MapSize, Offset, /*IsText=*/ମିଥ୍ୟା,
                                  /*RequiresNullTerminator=*/ମିଥ୍ୟା, IsVolatile,
                                  Alignment);
}

//===----------------------------------------------------------------------===//
// MemoryBuffer::getFile implementation.
//===----------------------------------------------------------------------===//

ନାମାକାଶ {

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
ଶ୍ରେଣୀ MemoryBufferMMapFile : ସର୍ବସାଧାରଣ MB {
  sys::fs::mapped_file_region MFR;

  ସ୍ଥିର uint64_t getLegalMapOffset(uint64_t Offset) {
    ଫେରନ୍ତୁ Offset & ~(sys::fs::mapped_file_region::alignment() - 1);
  }

  ସ୍ଥିର uint64_t getLegalMapSize(uint64_t Len, uint64_t Offset) {
    ଫେରନ୍ତୁ Len + (Offset - getLegalMapOffset(Offset));
  }

  ଅଚଳ ଅକ୍ଷର *getStart(uint64_t Len, uint64_t Offset) {
    ଫେରନ୍ତୁ MFR.const_data() + (Offset - getLegalMapOffset(Offset));
  }

ସର୍ବସାଧାରଣ:
  MemoryBufferMMapFile(ତାର୍କିକ RequiresNullTerminator, sys::fs::file_t FD, uint64_t Len,
                       uint64_t Offset, std::error_code &EC)
      : MFR(FD, Mapmode<MB>, getLegalMapSize(Len, Offset),
            getLegalMapOffset(Offset), EC) {
    ଯଦି (!EC) {
      ଅଚଳ ଅକ୍ଷର *Start = getStart(Len, Offset);
      MemoryBuffer::init(Start, Start + Len, RequiresNullTerminator);
    }
  }

  /// Disable sized deallocation for MemoryBufferMMapFile, because it has
  /// tail-allocated data.
  ଶୂନ୍ୟ operator delete(ଶୂନ୍ୟ *p) { std::ମୁକ୍ତ(p); }

  StringRef getBufferIdentifier() ଅଚଳ override {
    // The name is stored after the class itself.
    ଫେରନ୍ତୁ StringRef(reinterpret_cast<ଅଚଳ ଅକ୍ଷର *>(ଏହି + 1) + ଆକାର(size_t),
                     *reinterpret_cast<ଅଚଳ size_t *>(ଏହି + 1));
  }

  MemoryBuffer::BufferKind getBufferKind() ଅଚଳ override {
    ଫେରନ୍ତୁ MemoryBuffer::MemoryBuffer_MMap;
  }

  ଶୂନ୍ୟ dontNeedIfMmap() override { MFR.dontNeed(); }
  ଶୂନ୍ୟ willNeedIfMmap() override { MFR.willNeed(); }
  ଶୂନ୍ୟ randomAccessIfMmap() override { MFR.randomAccess(); }
};
} // namespace

ସ୍ଥିର ErrorOr<std::unique_ptr<WritableMemoryBuffer>>
getMemoryBufferForStream(sys::fs::file_t FD, ଅଚଳ Twine &BufferName) {
  SmallString<sys::fs::DefaultReadChunkSize> Buffer;
  ଯଦି (Error E = sys::fs::readNativeFileToEOF(FD, Buffer))
    ଫେରନ୍ତୁ errorToErrorCode(std::move(E));
  ଫେରନ୍ତୁ getMemBufferCopyImpl(Buffer, BufferName);
}

ErrorOr<std::unique_ptr<MemoryBuffer>>
MemoryBuffer::getFile(ଅଚଳ Twine &Filename, ତାର୍କିକ IsText,
                      ତାର୍କିକ RequiresNullTerminator, ତାର୍କିକ IsVolatile,
                      std::optional<Align> Alignment) {
  sys::sandbox::violationIfEnabled();

  ଫେରନ୍ତୁ getFileAux<MemoryBuffer>(Filename, /*MapSize=*/-1, /*Offset=*/0,
                                  IsText, RequiresNullTerminator, IsVolatile,
                                  Alignment);
}

template <typename MB>
ସ୍ଥିର ErrorOr<std::unique_ptr<MB>>
getOpenFileImpl(sys::fs::file_t FD, ଅଚଳ Twine &Filename, uint64_t FileSize,
                uint64_t MapSize, int64_t Offset, ତାର୍କିକ RequiresNullTerminator,
                ତାର୍କିକ IsVolatile, std::optional<Align> Alignment);

template <typename MB>
ସ୍ଥିର ErrorOr<std::unique_ptr<MB>>
getFileAux(ଅଚଳ Twine &Filename, uint64_t MapSize, uint64_t Offset,
           ତାର୍କିକ IsText, ତାର୍କିକ RequiresNullTerminator, ତାର୍କିକ IsVolatile,
           std::optional<Align> Alignment) {
  Expected<sys::fs::file_t> FDOrErr = sys::fs::openNativeFileForRead(
      Filename, IsText ? sys::fs::OF_TextWithCRLF : sys::fs::OF_None);
  ଯଦି (!FDOrErr)
    ଫେରନ୍ତୁ errorToErrorCode(FDOrErr.takeError());
  sys::fs::file_t FD = *FDOrErr;
  auto Ret = getOpenFileImpl<MB>(FD, Filename, /*FileSize=*/-1, MapSize, Offset,
                                 RequiresNullTerminator, IsVolatile, Alignment);
  sys::fs::closeFile(FD);
  ଫେରନ୍ତୁ Ret;
}

ErrorOr<std::unique_ptr<WritableMemoryBuffer>>
WritableMemoryBuffer::getFile(ଅଚଳ Twine &Filename, ତାର୍କିକ IsVolatile,
                              std::optional<Align> Alignment) {
  sys::sandbox::violationIfEnabled();

  ଫେରନ୍ତୁ getFileAux<WritableMemoryBuffer>(
      Filename, /*MapSize=*/-1, /*Offset=*/0, /*IsText=*/ମିଥ୍ୟା,
      /*RequiresNullTerminator=*/ମିଥ୍ୟା, IsVolatile, Alignment);
}

ErrorOr<std::unique_ptr<WritableMemoryBuffer>>
WritableMemoryBuffer::getFileSlice(ଅଚଳ Twine &Filename, uint64_t MapSize,
                                   uint64_t Offset, ତାର୍କିକ IsVolatile,
                                   std::optional<Align> Alignment) {
  sys::sandbox::violationIfEnabled();

  ଫେରନ୍ତୁ getFileAux<WritableMemoryBuffer>(
      Filename, MapSize, Offset, /*IsText=*/ମିଥ୍ୟା,
      /*RequiresNullTerminator=*/ମିଥ୍ୟା, IsVolatile, Alignment);
}

std::unique_ptr<WritableMemoryBuffer>
WritableMemoryBuffer::getNewUninitMemBuffer(size_t Size,
                                            ଅଚଳ Twine &BufferName,
                                            std::optional<Align> Alignment) {
  using MemBuffer = MemoryBufferMem<WritableMemoryBuffer>;

  // Use 16-byte alignment if no alignment is specified.
  Align BufAlign = Alignment.ମୂଲ୍ୟ_or(Align(16));

  // Allocate space for the MemoryBuffer, the data and the name. It is important
  // that MemoryBuffer and data are aligned so PointerIntPair works with them.
  SmallString<256> NameBuf;
  StringRef NameRef = BufferName.toStringRef(NameBuf);

  size_t StringLen = ଆକାର(MemBuffer) + ଆକାର(size_t) + NameRef.size() + 1;
  size_t RealLen = StringLen + Size + 1 + BufAlign.ମୂଲ୍ୟ_2();
  ଯଦି (RealLen <= Size) // Check for rollover.
    ଫେରନ୍ତୁ nullptr;
  // We use a call to malloc() rather than a call to a non-throwing operator
  // new() because LLVM unconditionally installs an out of memory new handler
  // when exceptions are disabled. This new handler intentionally crashes to
  // aid with debugging, but that makes non-throwing new calls unhelpful.
  // See MemoryBufferMem::operator delete() for the paired call to free(), and
  // llvm::install_out_of_memory_new_handler() for the installation of the
  // custom new handler.
  ଅକ୍ଷର *Mem = static_cast<ଅକ୍ଷର *>(std::malloc(RealLen));
  ଯଦି (!Mem)
    ଫେରନ୍ତୁ nullptr;

  // The name is stored after the class itself.
  *reinterpret_cast<size_t *>(Mem + ଆକାର(MemBuffer)) = NameRef.size();
  CopyStringRef(Mem + ଆକାର(MemBuffer) + ଆକାର(size_t), NameRef);

  // The buffer begins after the name and must be aligned.
  ଅକ୍ଷର *Buf = (ଅକ୍ଷର *)alignAddr(Mem + StringLen, BufAlign);
  Buf[Size] = 0; // Null terminate buffer.

  auto *Ret = ନୂତନ (Mem) MemBuffer(StringRef(Buf, Size), ସତ୍ୟ);
  ଫେରନ୍ତୁ std::unique_ptr<WritableMemoryBuffer>(Ret);
}

std::unique_ptr<WritableMemoryBuffer>
WritableMemoryBuffer::getNewMemBuffer(size_t Size, ଅଚଳ Twine &BufferName) {
  auto SB = WritableMemoryBuffer::getNewUninitMemBuffer(Size, BufferName);
  ଯଦି (!SB)
    ଫେରନ୍ତୁ nullptr;
  memset(SB->getBufferStart(), 0, Size);
  ଫେରନ୍ତୁ SB;
}

ସ୍ଥିର ତାର୍କିକ shouldUseMmap(sys::fs::file_t FD,
                          size_t FileSize,
                          size_t MapSize,
                          off_t Offset,
                          ତାର୍କିକ RequiresNullTerminator,
                          ପୂର୍ଣ୍ଣସଂଖ୍ୟା PageSize,
                          ତାର୍କିକ IsVolatile) {
#ଯଦି defined(__MVS__)
  // zOS Enhanced ASCII auto convert does not support mmap.
  ଫେରନ୍ତୁ ମିଥ୍ୟା;
#endif

  // mmap may leave the buffer without null terminator if the file size changed
  // by the time the last page is mapped in, so avoid it if the file size is
  // likely to change.
  ଯଦି (IsVolatile && RequiresNullTerminator)
    ଫେରନ୍ତୁ ମିଥ୍ୟା;

  // We don't use mmap for small files because this can severely fragment our
  // address space.
  ଯଦି (MapSize < 4 * 4096 || MapSize < (unsigned)PageSize)
    ଫେରନ୍ତୁ ମିଥ୍ୟା;

  ଯଦି (!RequiresNullTerminator)
    ଫେରନ୍ତୁ ସତ୍ୟ;

  // If we don't know the file size, use fstat to find out.  fstat on an open
  // file descriptor is cheaper than stat on a random path.
  // FIXME: this chunk of code is duplicated, but it avoids a fstat when
  // RequiresNullTerminator = false and MapSize != -1.
  ଯଦି (FileSize == size_t(-1)) {
    sys::fs::file_status Status;
    ଯଦି (sys::fs::status(FD, Status))
      ଫେରନ୍ତୁ ମିଥ୍ୟା;
    FileSize = Status.getSize();
  }

  // If we need a null terminator and the end of the map is inside the file,
  // we cannot use mmap.
  size_t End = Offset + MapSize;
  ପ୍ରତିଜ୍ଞା(End <= FileSize);
  ଯଦି (End != FileSize)
    ଫେରନ୍ତୁ ମିଥ୍ୟା;

  // Don't try to map files that are exactly a multiple of the system page size
  // if we need a null terminator.
  ଯଦି ((FileSize & (PageSize -1)) == 0)
    ଫେରନ୍ତୁ ମିଥ୍ୟା;

#ଯଦି defined(__CYGWIN__)
  // Don't try to map files that are exactly a multiple of the physical page size
  // if we need a null terminator.
  // FIXME: We should reorganize again getPageSize() on Win32.
  ଯଦି ((FileSize & (4096 - 1)) == 0)
    ଫେରନ୍ତୁ ମିଥ୍ୟା;
#endif

  ଫେରନ୍ତୁ ସତ୍ୟ;
}

ସ୍ଥିର ErrorOr<std::unique_ptr<WriteThroughMemoryBuffer>>
getReadWriteFile(ଅଚଳ Twine &Filename, uint64_t FileSize, uint64_t MapSize,
                 uint64_t Offset) {
  Expected<sys::fs::file_t> FDOrErr = sys::fs::openNativeFileForReadWrite(
      Filename, sys::fs::CD_OpenExisting, sys::fs::OF_None);
  ଯଦି (!FDOrErr)
    ଫେରନ୍ତୁ errorToErrorCode(FDOrErr.takeError());
  sys::fs::file_t FD = *FDOrErr;

  // Default is to map the full file.
  ଯଦି (MapSize == uint64_t(-1)) {
    // If we don't know the file size, use fstat to find out.  fstat on an open
    // file descriptor is cheaper than stat on a random path.
    ଯଦି (FileSize == uint64_t(-1)) {
      sys::fs::file_status Status;
      std::error_code EC = sys::fs::status(FD, Status);
      ଯଦି (EC)
        ଫେରନ୍ତୁ EC;

      // If this not a file or a block device (e.g. it's a named pipe
      // or character device), we can't mmap it, so error out.
      sys::fs::file_type Type = Status.type();
      ଯଦି (Type != sys::fs::file_type::regular_file &&
          Type != sys::fs::file_type::block_file)
        ଫେରନ୍ତୁ make_error_code(errc::invalid_argument);

      FileSize = Status.getSize();
    }
    MapSize = FileSize;
  }

  std::error_code EC;
  std::unique_ptr<WriteThroughMemoryBuffer> Result(
      ନୂତନ (NamedBufferAlloc(Filename))
          MemoryBufferMMapFile<WriteThroughMemoryBuffer>(ମିଥ୍ୟା, FD, MapSize,
                                                         Offset, EC));
  ଯଦି (EC)
    ଫେରନ୍ତୁ EC;
  ଫେରନ୍ତୁ std::move(Result);
}

ErrorOr<std::unique_ptr<WriteThroughMemoryBuffer>>
WriteThroughMemoryBuffer::getFile(ଅଚଳ Twine &Filename, int64_t FileSize) {
  sys::sandbox::violationIfEnabled();

  ଫେରନ୍ତୁ getReadWriteFile(Filename, FileSize, FileSize, 0);
}

/// Map a subrange of the specified file as a WritableMemoryBuffer.
ErrorOr<std::unique_ptr<WriteThroughMemoryBuffer>>
WriteThroughMemoryBuffer::getFileSlice(ଅଚଳ Twine &Filename, uint64_t MapSize,
                                       uint64_t Offset) {
  sys::sandbox::violationIfEnabled();

  ଫେରନ୍ତୁ getReadWriteFile(Filename, -1, MapSize, Offset);
}

template <typename MB>
ସ୍ଥିର ErrorOr<std::unique_ptr<MB>>
getOpenFileImpl(sys::fs::file_t FD, ଅଚଳ Twine &Filename, uint64_t FileSize,
                uint64_t MapSize, int64_t Offset, ତାର୍କିକ RequiresNullTerminator,
                ତାର୍କିକ IsVolatile, std::optional<Align> Alignment) {
  ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା PageSize = sys::Process::getPageSizeEstimate();

  // Default is to map the full file.
  ଯଦି (MapSize == uint64_t(-1)) {
    // If we don't know the file size, use fstat to find out.  fstat on an open
    // file descriptor is cheaper than stat on a random path.
    ଯଦି (FileSize == uint64_t(-1)) {
      sys::fs::file_status Status;
      std::error_code EC = sys::fs::status(FD, Status);
      ଯଦି (EC)
        ଫେରନ୍ତୁ EC;

      // If this not a file or a block device (e.g. it's a named pipe
      // or character device), we can't trust the size. Create the memory
      // buffer by copying off the stream.
      sys::fs::file_type Type = Status.type();
      ଯଦି (Type != sys::fs::file_type::regular_file &&
          Type != sys::fs::file_type::block_file)
        ଫେରନ୍ତୁ getMemoryBufferForStream(FD, Filename);

      FileSize = Status.getSize();
    }
    MapSize = FileSize;
  }

  ଯଦି (shouldUseMmap(FD, FileSize, MapSize, Offset, RequiresNullTerminator,
                    PageSize, IsVolatile)) {
    std::error_code EC;
    std::unique_ptr<MB> Result(
        ନୂତନ (NamedBufferAlloc(Filename)) MemoryBufferMMapFile<MB>(
            RequiresNullTerminator, FD, MapSize, Offset, EC));
    ଯଦି (!EC) {
      // On at least Linux, and possibly on other systems, mmap may return pages
      // from the page cache that are not properly filled with trailing zeroes,
      // if some prior user of the page wrote non-zero bytes. Detect this and
      // don't use mmap in that case.
      ଯଦି (!RequiresNullTerminator || *Result->getBufferEnd() == '\0')
        ଫେରନ୍ତୁ std::move(Result);
    }
  }

#ifdef __MVS__
  ErrorOr<ତାର୍କିକ> NeedsConversion = needConversion(Filename, FD);
  ଯଦି (std::error_code EC = NeedsConversion.getError())
    ଫେରନ୍ତୁ EC;
  // File size may increase due to EBCDIC -> UTF-8 conversion, therefore we
  // cannot trust the file size and we create the memory buffer by copying
  // off the stream.
  // Note: This only works with the assumption of reading a full file (i.e,
  // Offset == 0 and MapSize == FileSize). Reading a file slice does not work.
  ଯଦି (*NeedsConversion && Offset == 0 && MapSize == FileSize)
    ଫେରନ୍ତୁ getMemoryBufferForStream(FD, Filename);
#endif

  auto Buf =
      WritableMemoryBuffer::getNewUninitMemBuffer(MapSize, Filename, Alignment);
  ଯଦି (!Buf) {
    // Failed to create a buffer. The only way it can fail is if
    // new(std::nothrow) returns 0.
    ଫେରନ୍ତୁ make_error_code(errc::not_enough_memory);
  }

  // Read until EOF, zero-initialize the rest.
  MutableArrayRef<ଅକ୍ଷର> ToRead = Buf->getBuffer();
  ଯେତେବେଳେ (!ToRead.empty()) {
    Expected<size_t> ReadBytes =
        sys::fs::readNativeFileSlice(FD, ToRead, Offset);
    ଯଦି (!ReadBytes)
      ଫେରନ୍ତୁ errorToErrorCode(ReadBytes.takeError());
    ଯଦି (*ReadBytes == 0) {
      std::memset(ToRead.data(), 0, ToRead.size());
      ଭାଙ୍ଗ;
    }
    ToRead = ToRead.drop_front(*ReadBytes);
    Offset += *ReadBytes;
  }

  ଫେରନ୍ତୁ std::move(Buf);
}

ErrorOr<std::unique_ptr<MemoryBuffer>>
MemoryBuffer::getOpenFile(sys::fs::file_t FD, ଅଚଳ Twine &Filename,
                          uint64_t FileSize, ତାର୍କିକ RequiresNullTerminator,
                          ତାର୍କିକ IsVolatile, std::optional<Align> Alignment) {
  sys::sandbox::violationIfEnabled();

  ଫେରନ୍ତୁ getOpenFileImpl<MemoryBuffer>(FD, Filename, FileSize, FileSize, 0,
                                       RequiresNullTerminator, IsVolatile,
                                       Alignment);
}

ErrorOr<std::unique_ptr<MemoryBuffer>> MemoryBuffer::getOpenFileSlice(
    sys::fs::file_t FD, ଅଚଳ Twine &Filename, uint64_t MapSize, int64_t Offset,
    ତାର୍କିକ IsVolatile, std::optional<Align> Alignment) {
  ପ୍ରତିଜ୍ଞା(MapSize != uint64_t(-1));

  sys::sandbox::violationIfEnabled();

  ଫେରନ୍ତୁ getOpenFileImpl<MemoryBuffer>(FD, Filename, -1, MapSize, Offset, ମିଥ୍ୟା,
                                       IsVolatile, Alignment);
}

ErrorOr<std::unique_ptr<MemoryBuffer>> MemoryBuffer::getSTDIN() {
  sys::sandbox::violationIfEnabled();

  // Read in all of the data from stdin, we cannot mmap stdin.
  //
  // FIXME: That isn't necessarily true, we should try to mmap stdin and
  // fallback if it fails.
  sys::ChangeStdinMode(sys::fs::OF_Text);

  ଫେରନ୍ତୁ getMemoryBufferForStream(sys::fs::getStdinHandle(), "<stdin>");
}

ErrorOr<std::unique_ptr<MemoryBuffer>>
MemoryBuffer::getFileAsStream(ଅଚଳ Twine &Filename) {
  sys::sandbox::violationIfEnabled();

  Expected<sys::fs::file_t> FDOrErr =
      sys::fs::openNativeFileForRead(Filename, sys::fs::OF_None);
  ଯଦି (!FDOrErr)
    ଫେରନ୍ତୁ errorToErrorCode(FDOrErr.takeError());
  sys::fs::file_t FD = *FDOrErr;
  ErrorOr<std::unique_ptr<MemoryBuffer>> Ret =
      getMemoryBufferForStream(FD, Filename);
  sys::fs::closeFile(FD);
  ଫେରନ୍ତୁ Ret;
}

MemoryBufferRef MemoryBuffer::getMemBufferRef() ଅଚଳ {
  StringRef Data = getBuffer();
  StringRef Identifier = getBufferIdentifier();
  ଫେରନ୍ତୁ MemoryBufferRef(Data, Identifier);
}

SmallVectorMemoryBuffer::~SmallVectorMemoryBuffer() = ପୂର୍ବନିର୍ଦ୍ଧାରିତ;
