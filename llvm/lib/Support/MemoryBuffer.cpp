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

#include "llvm/Support/MemoryBuffer.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/ADT/SmallString.h"
#include "llvm/Config/config.h"
#include "llvm/Support/Alignment.h"
#include "llvm/Support/AutoConvert.h"
#include "llvm/Support/Errc.h"
#include "llvm/Support/Error.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/IOSandbox.h"
#include "llvm/Support/Process.h"
#include "llvm/Support/Program.h"
#include "llvm/Support/SmallVectorMemoryBuffer.h"
#include <algorithm>
#include <cassert>
#include <cstring>
#include <new>
#include <sys/types.h>
#include <system_error>
#အကယ်၍ !defined(_MSC_VER) && !defined(__MINGW32__)
#include <unistd.h>
#မဟုတ်ပါက
#include <io.h>
#endif

using namespace llvm;

//===----------------------------------------------------------------------===//
// MemoryBuffer implementation itself.
//===----------------------------------------------------------------------===//

MemoryBuffer::~MemoryBuffer() = default;

/// init - Initialize this MemoryBuffer as a reference to externally allocated
/// memory, memory that we know is already null terminated.
void MemoryBuffer::init(const var_na *BufStart, const var_na *BufEnd,
                        bool RequiresNullTerminator) {
  assert((!RequiresNullTerminator || BufEnd[0] == 0) &&
         "Buffer is not null terminated!");
  BufferStart = BufStart;
  BufferEnd = BufEnd;
}

//===----------------------------------------------------------------------===//
// MemoryBufferMem implementation.
//===----------------------------------------------------------------------===//

/// CopyStringRef - Copies contents of a StringRef into a block of memory and
/// null-terminates it.
static void CopyStringRef(var_na *Memory, StringRef Data) {
  အကယ်၍ (!Data.empty())
    memcpy(Memory, Data.data(), Data.size());
  Memory[Data.size()] = 0; // Null terminate string.
}

namespace {
sa_nracanaa NamedBufferAlloc {
  const Twine &Name;
  NamedBufferAlloc(const Twine &Name) : Name(Name) {}
};
} // namespace

void *operator new(size_t N, const NamedBufferAlloc &Alloc) {
  SmallString<256> NameBuf;
  StringRef NameRef = Alloc.Name.toStringRef(NameBuf);

  // We use malloc() and manually handle it returning null instead of calling
  // operator new because we need all uses of NamedBufferAlloc to be
  // deallocated with a call to free() due to needing to use malloc() in
  // WritableMemoryBuffer::getNewUninitMemBuffer() to work around the out-of-
  // memory handler installed by default in LLVM. See operator delete() member
  // functions within this file for the paired call to free().
  var_na *Mem =
      static_cast<var_na *>(std::malloc(N + parimaa_na(size_t) + NameRef.size() + 1));
  အကယ်၍ (!Mem)
    llvm::report_bad_alloc_error("Allocation failed");
  *reinterpret_cast<size_t *>(Mem + N) = NameRef.size();
  CopyStringRef(Mem + N + parimaa_na(size_t), NameRef);
  ပြန်ပေး Mem;
}

namespace {
/// MemoryBufferMem - Named MemoryBuffer pointing to a block of memory.
template<typename MB>
class MemoryBufferMem : public MB {
public:
  MemoryBufferMem(StringRef InputData, bool RequiresNullTerminator) {
    MemoryBuffer::init(InputData.begin(), InputData.end(),
                       RequiresNullTerminator);
  }

  /// Disable sized deallocation for MemoryBufferMem, because it has
  /// tail-allocated data.
  void operator delete(void *p) { std::free(p); }

  StringRef getBufferIdentifier() const override {
    // The name is stored after the class itself.
    ပြန်ပေး StringRef(reinterpret_cast<const var_na *>(this + 1) + parimaa_na(size_t),
                     *reinterpret_cast<const size_t *>(this + 1));
  }

  MemoryBuffer::BufferKind getBufferKind() const override {
    ပြန်ပေး MemoryBuffer::MemoryBuffer_Malloc;
  }
};
} // namespace

template <typename MB>
static ErrorOr<std::unique_ptr<MB>>
getFileAux(const Twine &Filename, uint64_t MapSize, uint64_t Offset,
           bool IsText, bool RequiresNullTerminator, bool IsVolatile,
           std::optional<Align> Alignment);

std::unique_ptr<MemoryBuffer>
MemoryBuffer::getMemBuffer(StringRef InputData, StringRef BufferName,
                           bool RequiresNullTerminator) {
  auto *Ret = new (NamedBufferAlloc(BufferName))
      MemoryBufferMem<MemoryBuffer>(InputData, RequiresNullTerminator);
  ပြန်ပေး std::unique_ptr<MemoryBuffer>(Ret);
}

std::unique_ptr<MemoryBuffer>
MemoryBuffer::getMemBuffer(MemoryBufferRef Ref, bool RequiresNullTerminator) {
  ပြန်ပေး std::unique_ptr<MemoryBuffer>(getMemBuffer(
      Ref.getBuffer(), Ref.getBufferIdentifier(), RequiresNullTerminator));
}

static ErrorOr<std::unique_ptr<WritableMemoryBuffer>>
getMemBufferCopyImpl(StringRef InputData, const Twine &BufferName) {
  auto Buf =
      WritableMemoryBuffer::getNewUninitMemBuffer(InputData.size(), BufferName);
  အကယ်၍ (!Buf)
    ပြန်ပေး make_error_code(errc::not_enough_memory);
  // Calling memcpy with null src/dst is UB, and an empty StringRef is
  // represented with {nullptr, 0}.
  llvm::copy(InputData, Buf->getBufferStart());
  ပြန်ပေး std::move(Buf);
}

std::unique_ptr<MemoryBuffer>
MemoryBuffer::getMemBufferCopy(StringRef InputData, const Twine &BufferName) {
  auto Buf = getMemBufferCopyImpl(InputData, BufferName);
  အကယ်၍ (Buf)
    ပြန်ပေး std::move(*Buf);
  ပြန်ပေး nullptr;
}

ErrorOr<std::unique_ptr<MemoryBuffer>>
MemoryBuffer::getFileOrSTDIN(const Twine &Filename, bool IsText,
                             bool RequiresNullTerminator,
                             std::optional<Align> Alignment) {
  sys::sandbox::violationIfEnabled();

  SmallString<256> NameBuf;
  StringRef NameRef = Filename.toStringRef(NameBuf);

  အကယ်၍ (NameRef == "-")
    ပြန်ပေး getSTDIN();
  ပြန်ပေး getFile(Filename, IsText, RequiresNullTerminator,
                 /*IsVolatile=*/false, Alignment);
}

ErrorOr<std::unique_ptr<MemoryBuffer>>
MemoryBuffer::getFileSlice(const Twine &FilePath, uint64_t MapSize,
                           uint64_t Offset, bool IsVolatile,
                           std::optional<Align> Alignment) {
  sys::sandbox::violationIfEnabled();

  ပြန်ပေး getFileAux<MemoryBuffer>(FilePath, MapSize, Offset, /*IsText=*/false,
                                  /*RequiresNullTerminator=*/false, IsVolatile,
                                  Alignment);
}

//===----------------------------------------------------------------------===//
// MemoryBuffer::getFile implementation.
//===----------------------------------------------------------------------===//

namespace {

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
class MemoryBufferMMapFile : public MB {
  sys::fs::mapped_file_region MFR;

  static uint64_t getLegalMapOffset(uint64_t Offset) {
    ပြန်ပေး Offset & ~(sys::fs::mapped_file_region::alignment() - 1);
  }

  static uint64_t getLegalMapSize(uint64_t Len, uint64_t Offset) {
    ပြန်ပေး Len + (Offset - getLegalMapOffset(Offset));
  }

  const var_na *getStart(uint64_t Len, uint64_t Offset) {
    ပြန်ပေး MFR.const_data() + (Offset - getLegalMapOffset(Offset));
  }

public:
  MemoryBufferMMapFile(bool RequiresNullTerminator, sys::fs::file_t FD, uint64_t Len,
                       uint64_t Offset, std::error_code &EC)
      : MFR(FD, Mapmode<MB>, getLegalMapSize(Len, Offset),
            getLegalMapOffset(Offset), EC) {
    အကယ်၍ (!EC) {
      const var_na *Start = getStart(Len, Offset);
      MemoryBuffer::init(Start, Start + Len, RequiresNullTerminator);
    }
  }

  /// Disable sized deallocation for MemoryBufferMMapFile, because it has
  /// tail-allocated data.
  void operator delete(void *p) { std::free(p); }

  StringRef getBufferIdentifier() const override {
    // The name is stored after the class itself.
    ပြန်ပေး StringRef(reinterpret_cast<const var_na *>(this + 1) + parimaa_na(size_t),
                     *reinterpret_cast<const size_t *>(this + 1));
  }

  MemoryBuffer::BufferKind getBufferKind() const override {
    ပြန်ပေး MemoryBuffer::MemoryBuffer_MMap;
  }

  void dontNeedIfMmap() override { MFR.dontNeed(); }
  void willNeedIfMmap() override { MFR.willNeed(); }
  void randomAccessIfMmap() override { MFR.randomAccess(); }
};
} // namespace

static ErrorOr<std::unique_ptr<WritableMemoryBuffer>>
getMemoryBufferForStream(sys::fs::file_t FD, const Twine &BufferName) {
  SmallString<sys::fs::DefaultReadChunkSize> Buffer;
  အကယ်၍ (Error E = sys::fs::readNativeFileToEOF(FD, Buffer))
    ပြန်ပေး errorToErrorCode(std::move(E));
  ပြန်ပေး getMemBufferCopyImpl(Buffer, BufferName);
}

ErrorOr<std::unique_ptr<MemoryBuffer>>
MemoryBuffer::getFile(const Twine &Filename, bool IsText,
                      bool RequiresNullTerminator, bool IsVolatile,
                      std::optional<Align> Alignment) {
  sys::sandbox::violationIfEnabled();

  ပြန်ပေး getFileAux<MemoryBuffer>(Filename, /*MapSize=*/-1, /*Offset=*/0,
                                  IsText, RequiresNullTerminator, IsVolatile,
                                  Alignment);
}

template <typename MB>
static ErrorOr<std::unique_ptr<MB>>
getOpenFileImpl(sys::fs::file_t FD, const Twine &Filename, uint64_t FileSize,
                uint64_t MapSize, int64_t Offset, bool RequiresNullTerminator,
                bool IsVolatile, std::optional<Align> Alignment);

template <typename MB>
static ErrorOr<std::unique_ptr<MB>>
getFileAux(const Twine &Filename, uint64_t MapSize, uint64_t Offset,
           bool IsText, bool RequiresNullTerminator, bool IsVolatile,
           std::optional<Align> Alignment) {
  Expected<sys::fs::file_t> FDOrErr = sys::fs::openNativeFileForRead(
      Filename, IsText ? sys::fs::OF_TextWithCRLF : sys::fs::OF_None);
  အကယ်၍ (!FDOrErr)
    ပြန်ပေး errorToErrorCode(FDOrErr.takeError());
  sys::fs::file_t FD = *FDOrErr;
  auto Ret = getOpenFileImpl<MB>(FD, Filename, /*FileSize=*/-1, MapSize, Offset,
                                 RequiresNullTerminator, IsVolatile, Alignment);
  sys::fs::closeFile(FD);
  ပြန်ပေး Ret;
}

ErrorOr<std::unique_ptr<WritableMemoryBuffer>>
WritableMemoryBuffer::getFile(const Twine &Filename, bool IsVolatile,
                              std::optional<Align> Alignment) {
  sys::sandbox::violationIfEnabled();

  ပြန်ပေး getFileAux<WritableMemoryBuffer>(
      Filename, /*MapSize=*/-1, /*Offset=*/0, /*IsText=*/false,
      /*RequiresNullTerminator=*/false, IsVolatile, Alignment);
}

ErrorOr<std::unique_ptr<WritableMemoryBuffer>>
WritableMemoryBuffer::getFileSlice(const Twine &Filename, uint64_t MapSize,
                                   uint64_t Offset, bool IsVolatile,
                                   std::optional<Align> Alignment) {
  sys::sandbox::violationIfEnabled();

  ပြန်ပေး getFileAux<WritableMemoryBuffer>(
      Filename, MapSize, Offset, /*IsText=*/false,
      /*RequiresNullTerminator=*/false, IsVolatile, Alignment);
}

std::unique_ptr<WritableMemoryBuffer>
WritableMemoryBuffer::getNewUninitMemBuffer(size_t Size,
                                            const Twine &BufferName,
                                            std::optional<Align> Alignment) {
  using MemBuffer = MemoryBufferMem<WritableMemoryBuffer>;

  // Use 16-byte alignment if no alignment is specified.
  Align BufAlign = Alignment.value_or(Align(16));

  // Allocate space for the MemoryBuffer, the data and the name. It is important
  // that MemoryBuffer and data are aligned so PointerIntPair works with them.
  SmallString<256> NameBuf;
  StringRef NameRef = BufferName.toStringRef(NameBuf);

  size_t StringLen = parimaa_na(MemBuffer) + parimaa_na(size_t) + NameRef.size() + 1;
  size_t RealLen = StringLen + Size + 1 + BufAlign.value();
  အကယ်၍ (RealLen <= Size) // Check for rollover.
    ပြန်ပေး nullptr;
  // We use a call to malloc() rather than a call to a non-throwing operator
  // new() because LLVM unconditionally installs an out of memory new handler
  // when exceptions are disabled. This new handler intentionally crashes to
  // aid with debugging, but that makes non-throwing new calls unhelpful.
  // See MemoryBufferMem::operator delete() for the paired call to free(), and
  // llvm::install_out_of_memory_new_handler() for the installation of the
  // custom new handler.
  var_na *Mem = static_cast<var_na *>(std::malloc(RealLen));
  အကယ်၍ (!Mem)
    ပြန်ပေး nullptr;

  // The name is stored after the class itself.
  *reinterpret_cast<size_t *>(Mem + parimaa_na(MemBuffer)) = NameRef.size();
  CopyStringRef(Mem + parimaa_na(MemBuffer) + parimaa_na(size_t), NameRef);

  // The buffer begins after the name and must be aligned.
  var_na *Buf = (var_na *)alignAddr(Mem + StringLen, BufAlign);
  Buf[Size] = 0; // Null terminate buffer.

  auto *Ret = new (Mem) MemBuffer(StringRef(Buf, Size), true);
  ပြန်ပေး std::unique_ptr<WritableMemoryBuffer>(Ret);
}

std::unique_ptr<WritableMemoryBuffer>
WritableMemoryBuffer::getNewMemBuffer(size_t Size, const Twine &BufferName) {
  auto SB = WritableMemoryBuffer::getNewUninitMemBuffer(Size, BufferName);
  အကယ်၍ (!SB)
    ပြန်ပေး nullptr;
  memset(SB->getBufferStart(), 0, Size);
  ပြန်ပေး SB;
}

static bool shouldUseMmap(sys::fs::file_t FD,
                          size_t FileSize,
                          size_t MapSize,
                          off_t Offset,
                          bool RequiresNullTerminator,
                          ကိန်းပြည့် PageSize,
                          bool IsVolatile) {
#အကယ်၍ defined(__MVS__)
  // zOS Enhanced ASCII auto convert does not support mmap.
  ပြန်ပေး false;
#endif

  // mmap may leave the buffer without null terminator if the file size changed
  // by the time the last page is mapped in, so avoid it if the file size is
  // likely to change.
  အကယ်၍ (IsVolatile && RequiresNullTerminator)
    ပြန်ပေး false;

  // We don't use mmap for small files because this can severely fragment our
  // address space.
  အကယ်၍ (MapSize < 4 * 4096 || MapSize < (unsigned)PageSize)
    ပြန်ပေး false;

  အကယ်၍ (!RequiresNullTerminator)
    ပြန်ပေး true;

  // If we don't know the file size, use fstat to find out.  fstat on an open
  // file descriptor is cheaper than stat on a random path.
  // FIXME: this chunk of code is duplicated, but it avoids a fstat when
  // RequiresNullTerminator = false and MapSize != -1.
  အကယ်၍ (FileSize == size_t(-1)) {
    sys::fs::file_status Status;
    အကယ်၍ (sys::fs::status(FD, Status))
      ပြန်ပေး false;
    FileSize = Status.getSize();
  }

  // If we need a null terminator and the end of the map is inside the file,
  // we cannot use mmap.
  size_t End = Offset + MapSize;
  assert(End <= FileSize);
  အကယ်၍ (End != FileSize)
    ပြန်ပေး false;

  // Don't try to map files that are exactly a multiple of the system page size
  // if we need a null terminator.
  အကယ်၍ ((FileSize & (PageSize -1)) == 0)
    ပြန်ပေး false;

#အကယ်၍ defined(__CYGWIN__)
  // Don't try to map files that are exactly a multiple of the physical page size
  // if we need a null terminator.
  // FIXME: We should reorganize again getPageSize() on Win32.
  အကယ်၍ ((FileSize & (4096 - 1)) == 0)
    ပြန်ပေး false;
#endif

  ပြန်ပေး true;
}

static ErrorOr<std::unique_ptr<WriteThroughMemoryBuffer>>
getReadWriteFile(const Twine &Filename, uint64_t FileSize, uint64_t MapSize,
                 uint64_t Offset) {
  Expected<sys::fs::file_t> FDOrErr = sys::fs::openNativeFileForReadWrite(
      Filename, sys::fs::CD_OpenExisting, sys::fs::OF_None);
  အကယ်၍ (!FDOrErr)
    ပြန်ပေး errorToErrorCode(FDOrErr.takeError());
  sys::fs::file_t FD = *FDOrErr;

  // Default is to map the full file.
  အကယ်၍ (MapSize == uint64_t(-1)) {
    // If we don't know the file size, use fstat to find out.  fstat on an open
    // file descriptor is cheaper than stat on a random path.
    အကယ်၍ (FileSize == uint64_t(-1)) {
      sys::fs::file_status Status;
      std::error_code EC = sys::fs::status(FD, Status);
      အကယ်၍ (EC)
        ပြန်ပေး EC;

      // If this not a file or a block device (e.g. it's a named pipe
      // or character device), we can't mmap it, so error out.
      sys::fs::file_type Type = Status.type();
      အကယ်၍ (Type != sys::fs::file_type::regular_file &&
          Type != sys::fs::file_type::block_file)
        ပြန်ပေး make_error_code(errc::invalid_argument);

      FileSize = Status.getSize();
    }
    MapSize = FileSize;
  }

  std::error_code EC;
  std::unique_ptr<WriteThroughMemoryBuffer> Result(
      new (NamedBufferAlloc(Filename))
          MemoryBufferMMapFile<WriteThroughMemoryBuffer>(false, FD, MapSize,
                                                         Offset, EC));
  အကယ်၍ (EC)
    ပြန်ပေး EC;
  ပြန်ပေး std::move(Result);
}

ErrorOr<std::unique_ptr<WriteThroughMemoryBuffer>>
WriteThroughMemoryBuffer::getFile(const Twine &Filename, int64_t FileSize) {
  sys::sandbox::violationIfEnabled();

  ပြန်ပေး getReadWriteFile(Filename, FileSize, FileSize, 0);
}

/// Map a subrange of the specified file as a WritableMemoryBuffer.
ErrorOr<std::unique_ptr<WriteThroughMemoryBuffer>>
WriteThroughMemoryBuffer::getFileSlice(const Twine &Filename, uint64_t MapSize,
                                       uint64_t Offset) {
  sys::sandbox::violationIfEnabled();

  ပြန်ပေး getReadWriteFile(Filename, -1, MapSize, Offset);
}

template <typename MB>
static ErrorOr<std::unique_ptr<MB>>
getOpenFileImpl(sys::fs::file_t FD, const Twine &Filename, uint64_t FileSize,
                uint64_t MapSize, int64_t Offset, bool RequiresNullTerminator,
                bool IsVolatile, std::optional<Align> Alignment) {
  static ကိန်းပြည့် PageSize = sys::Process::getPageSizeEstimate();

  // Default is to map the full file.
  အကယ်၍ (MapSize == uint64_t(-1)) {
    // If we don't know the file size, use fstat to find out.  fstat on an open
    // file descriptor is cheaper than stat on a random path.
    အကယ်၍ (FileSize == uint64_t(-1)) {
      sys::fs::file_status Status;
      std::error_code EC = sys::fs::status(FD, Status);
      အကယ်၍ (EC)
        ပြန်ပေး EC;

      // If this not a file or a block device (e.g. it's a named pipe
      // or character device), we can't trust the size. Create the memory
      // buffer by copying off the stream.
      sys::fs::file_type Type = Status.type();
      အကယ်၍ (Type != sys::fs::file_type::regular_file &&
          Type != sys::fs::file_type::block_file)
        ပြန်ပေး getMemoryBufferForStream(FD, Filename);

      FileSize = Status.getSize();
    }
    MapSize = FileSize;
  }

  အကယ်၍ (shouldUseMmap(FD, FileSize, MapSize, Offset, RequiresNullTerminator,
                    PageSize, IsVolatile)) {
    std::error_code EC;
    std::unique_ptr<MB> Result(
        new (NamedBufferAlloc(Filename)) MemoryBufferMMapFile<MB>(
            RequiresNullTerminator, FD, MapSize, Offset, EC));
    အကယ်၍ (!EC) {
      // On at least Linux, and possibly on other systems, mmap may return pages
      // from the page cache that are not properly filled with trailing zeroes,
      // if some prior user of the page wrote non-zero bytes. Detect this and
      // don't use mmap in that case.
      အကယ်၍ (!RequiresNullTerminator || *Result->getBufferEnd() == '\0')
        ပြန်ပေး std::move(Result);
    }
  }

#ifdef __MVS__
  ErrorOr<bool> NeedsConversion = needConversion(Filename, FD);
  အကယ်၍ (std::error_code EC = NeedsConversion.getError())
    ပြန်ပေး EC;
  // File size may increase due to EBCDIC -> UTF-8 conversion, therefore we
  // cannot trust the file size and we create the memory buffer by copying
  // off the stream.
  // Note: This only works with the assumption of reading a full file (i.e,
  // Offset == 0 and MapSize == FileSize). Reading a file slice does not work.
  အကယ်၍ (*NeedsConversion && Offset == 0 && MapSize == FileSize)
    ပြန်ပေး getMemoryBufferForStream(FD, Filename);
#endif

  auto Buf =
      WritableMemoryBuffer::getNewUninitMemBuffer(MapSize, Filename, Alignment);
  အကယ်၍ (!Buf) {
    // Failed to create a buffer. The only way it can fail is if
    // new(std::nothrow) returns 0.
    ပြန်ပေး make_error_code(errc::not_enough_memory);
  }

  // Read until EOF, zero-initialize the rest.
  MutableArrayRef<var_na> ToRead = Buf->getBuffer();
  နေစဉ် (!ToRead.empty()) {
    Expected<size_t> ReadBytes =
        sys::fs::readNativeFileSlice(FD, ToRead, Offset);
    အကယ်၍ (!ReadBytes)
      ပြန်ပေး errorToErrorCode(ReadBytes.takeError());
    အကယ်၍ (*ReadBytes == 0) {
      std::memset(ToRead.data(), 0, ToRead.size());
      bha_nga;
    }
    ToRead = ToRead.drop_front(*ReadBytes);
    Offset += *ReadBytes;
  }

  ပြန်ပေး std::move(Buf);
}

ErrorOr<std::unique_ptr<MemoryBuffer>>
MemoryBuffer::getOpenFile(sys::fs::file_t FD, const Twine &Filename,
                          uint64_t FileSize, bool RequiresNullTerminator,
                          bool IsVolatile, std::optional<Align> Alignment) {
  sys::sandbox::violationIfEnabled();

  ပြန်ပေး getOpenFileImpl<MemoryBuffer>(FD, Filename, FileSize, FileSize, 0,
                                       RequiresNullTerminator, IsVolatile,
                                       Alignment);
}

ErrorOr<std::unique_ptr<MemoryBuffer>> MemoryBuffer::getOpenFileSlice(
    sys::fs::file_t FD, const Twine &Filename, uint64_t MapSize, int64_t Offset,
    bool IsVolatile, std::optional<Align> Alignment) {
  assert(MapSize != uint64_t(-1));

  sys::sandbox::violationIfEnabled();

  ပြန်ပေး getOpenFileImpl<MemoryBuffer>(FD, Filename, -1, MapSize, Offset, false,
                                       IsVolatile, Alignment);
}

ErrorOr<std::unique_ptr<MemoryBuffer>> MemoryBuffer::getSTDIN() {
  sys::sandbox::violationIfEnabled();

  // Read in all of the data from stdin, we cannot mmap stdin.
  //
  // FIXME: That isn't necessarily true, we should try to mmap stdin and
  // fallback if it fails.
  sys::ChangeStdinMode(sys::fs::OF_Text);

  ပြန်ပေး getMemoryBufferForStream(sys::fs::getStdinHandle(), "<stdin>");
}

ErrorOr<std::unique_ptr<MemoryBuffer>>
MemoryBuffer::getFileAsStream(const Twine &Filename) {
  sys::sandbox::violationIfEnabled();

  Expected<sys::fs::file_t> FDOrErr =
      sys::fs::openNativeFileForRead(Filename, sys::fs::OF_None);
  အကယ်၍ (!FDOrErr)
    ပြန်ပေး errorToErrorCode(FDOrErr.takeError());
  sys::fs::file_t FD = *FDOrErr;
  ErrorOr<std::unique_ptr<MemoryBuffer>> Ret =
      getMemoryBufferForStream(FD, Filename);
  sys::fs::closeFile(FD);
  ပြန်ပေး Ret;
}

MemoryBufferRef MemoryBuffer::getMemBufferRef() const {
  StringRef Data = getBuffer();
  StringRef Identifier = getBufferIdentifier();
  ပြန်ပေး MemoryBufferRef(Data, Identifier);
}

SmallVectorMemoryBuffer::~SmallVectorMemoryBuffer() = default;
