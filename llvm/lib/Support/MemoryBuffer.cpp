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

#ضمن "llvm/Support/MemoryBuffer.h"
#ضمن "llvm/ADT/STLExtras.h"
#ضمن "llvm/ADT/SmallString.h"
#ضمن "llvm/Config/config.h"
#ضمن "llvm/Support/Alignment.h"
#ضمن "llvm/Support/AutoConvert.h"
#ضمن "llvm/Support/Errc.h"
#ضمن "llvm/Support/Error.h"
#ضمن "llvm/Support/ErrorHandling.h"
#ضمن "llvm/Support/FileSystem.h"
#ضمن "llvm/Support/IOSandbox.h"
#ضمن "llvm/Support/Process.h"
#ضمن "llvm/Support/Program.h"
#ضمن "llvm/Support/SmallVectorMemoryBuffer.h"
#ضمن <algorithm>
#ضمن <cassert>
#ضمن <cstring>
#ضمن <new>
#ضمن <sys/types.h>
#ضمن <system_error>
#إذا !defined(_MSC_VER) && !defined(__MINGW32__)
#ضمن <unistd.h>
#وإلا
#ضمن <io.h>
#endif

using نطاق llvm;

//===----------------------------------------------------------------------===//
// MemoryBuffer implementation itself.
//===----------------------------------------------------------------------===//

MemoryBuffer::~MemoryBuffer() = افتراضي;

/// init - Initialize this MemoryBuffer as a reference to externally allocated
/// memory, memory that we know is already null terminated.
فراغ MemoryBuffer::init(ثابت حرف *BufStart, ثابت حرف *BufEnd,
                        منطقي RequiresNullTerminator) {
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
ساكن فراغ CopyStringRef(حرف *Memory, StringRef Data) {
  إذا (!Data.empty())
    memcpy(Memory, Data.data(), Data.size());
  Memory[Data.size()] = 0; // Null terminate string.
}

نطاق {
بنية NamedBufferAlloc {
  ثابت Twine &Name;
  NamedBufferAlloc(ثابت Twine &Name) : Name(Name) {}
};
} // namespace

فراغ *operator جديد(size_t N, ثابت NamedBufferAlloc &Alloc) {
  SmallString<256> NameBuf;
  StringRef NameRef = Alloc.Name.toStringRef(NameBuf);

  // We use malloc() and manually handle it returning null instead of calling
  // operator new because we need all uses of NamedBufferAlloc to be
  // deallocated with a call to free() due to needing to use malloc() in
  // WritableMemoryBuffer::getNewUninitMemBuffer() to work around the out-of-
  // memory handler installed by default in LLVM. See operator delete() member
  // functions within this file for the paired call to free().
  حرف *Mem =
      static_cast<حرف *>(std::malloc(N + حجم(size_t) + NameRef.size() + 1));
  إذا (!Mem)
    llvm::report_bad_alloc_error("Allocation failed");
  *reinterpret_cast<size_t *>(Mem + N) = NameRef.size();
  CopyStringRef(Mem + N + حجم(size_t), NameRef);
  أرجع Mem;
}

نطاق {
/// MemoryBufferMem - Named MemoryBuffer pointing to a block of memory.
template<typename MB>
صنف MemoryBufferMem : عام MB {
عام:
  MemoryBufferMem(StringRef مدخل_Data, منطقي RequiresNullTerminator) {
    MemoryBuffer::init(مدخل_Data.begin(), مدخل_Data.end(),
                       RequiresNullTerminator);
  }

  /// Disable sized deallocation for MemoryBufferMem, because it has
  /// tail-allocated data.
  فراغ operator delete(فراغ *p) { std::حرر(p); }

  StringRef getBufferIdentifier() ثابت override {
    // The name is stored after the class itself.
    أرجع StringRef(reinterpret_cast<ثابت حرف *>(هذا + 1) + حجم(size_t),
                     *reinterpret_cast<ثابت size_t *>(هذا + 1));
  }

  MemoryBuffer::BufferKind getBufferKind() ثابت override {
    أرجع MemoryBuffer::MemoryBuffer_Malloc;
  }
};
} // namespace

template <typename MB>
ساكن ErrorOr<std::unique_ptr<MB>>
getFileAux(ثابت Twine &Filename, uint64_t MapSize, uint64_t Offset,
           منطقي IsText, منطقي RequiresNullTerminator, منطقي IsVolatile,
           std::optional<Align> Alignment);

std::unique_ptr<MemoryBuffer>
MemoryBuffer::getMemBuffer(StringRef مدخل_Data, StringRef BufferName,
                           منطقي RequiresNullTerminator) {
  auto *Ret = جديد (NamedBufferAlloc(BufferName))
      MemoryBufferMem<MemoryBuffer>(مدخل_Data, RequiresNullTerminator);
  أرجع std::unique_ptr<MemoryBuffer>(Ret);
}

std::unique_ptr<MemoryBuffer>
MemoryBuffer::getMemBuffer(MemoryBufferRef Ref, منطقي RequiresNullTerminator) {
  أرجع std::unique_ptr<MemoryBuffer>(getMemBuffer(
      Ref.getBuffer(), Ref.getBufferIdentifier(), RequiresNullTerminator));
}

ساكن ErrorOr<std::unique_ptr<WritableMemoryBuffer>>
getMemBufferCopyImpl(StringRef مدخل_Data, ثابت Twine &BufferName) {
  auto Buf =
      WritableMemoryBuffer::getNewUninitMemBuffer(مدخل_Data.size(), BufferName);
  إذا (!Buf)
    أرجع make_error_code(errc::not_enough_memory);
  // Calling memcpy with null src/dst is UB, and an empty StringRef is
  // represented with {nullptr, 0}.
  llvm::copy(مدخل_Data, Buf->getBufferStart());
  أرجع std::move(Buf);
}

std::unique_ptr<MemoryBuffer>
MemoryBuffer::getMemBufferCopy(StringRef مدخل_Data, ثابت Twine &BufferName) {
  auto Buf = getMemBufferCopyImpl(مدخل_Data, BufferName);
  إذا (Buf)
    أرجع std::move(*Buf);
  أرجع nullptr;
}

ErrorOr<std::unique_ptr<MemoryBuffer>>
MemoryBuffer::getFileOrSTDIN(ثابت Twine &Filename, منطقي IsText,
                             منطقي RequiresNullTerminator,
                             std::optional<Align> Alignment) {
  sys::sandbox::violationIfEnabled();

  SmallString<256> NameBuf;
  StringRef NameRef = Filename.toStringRef(NameBuf);

  إذا (NameRef == "-")
    أرجع getSTDIN();
  أرجع getFile(Filename, IsText, RequiresNullTerminator,
                 /*IsVolatile=*/خطأ, Alignment);
}

ErrorOr<std::unique_ptr<MemoryBuffer>>
MemoryBuffer::getFileSlice(ثابت Twine &FilePath, uint64_t MapSize,
                           uint64_t Offset, منطقي IsVolatile,
                           std::optional<Align> Alignment) {
  sys::sandbox::violationIfEnabled();

  أرجع getFileAux<MemoryBuffer>(FilePath, MapSize, Offset, /*IsText=*/خطأ,
                                  /*RequiresNullTerminator=*/خطأ, IsVolatile,
                                  Alignment);
}

//===----------------------------------------------------------------------===//
// MemoryBuffer::getFile implementation.
//===----------------------------------------------------------------------===//

نطاق {

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
صنف MemoryBufferMMapFile : عام MB {
  sys::fs::mapped_file_region MFR;

  ساكن uint64_t getLegalMapOffset(uint64_t Offset) {
    أرجع Offset & ~(sys::fs::mapped_file_region::alignment() - 1);
  }

  ساكن uint64_t getLegalMapSize(uint64_t Len, uint64_t Offset) {
    أرجع Len + (Offset - getLegalMapOffset(Offset));
  }

  ثابت حرف *getStart(uint64_t Len, uint64_t Offset) {
    أرجع MFR.const_data() + (Offset - getLegalMapOffset(Offset));
  }

عام:
  MemoryBufferMMapFile(منطقي RequiresNullTerminator, sys::fs::file_t FD, uint64_t Len,
                       uint64_t Offset, std::error_code &EC)
      : MFR(FD, Mapmode<MB>, getLegalMapSize(Len, Offset),
            getLegalMapOffset(Offset), EC) {
    إذا (!EC) {
      ثابت حرف *Start = getStart(Len, Offset);
      MemoryBuffer::init(Start, Start + Len, RequiresNullTerminator);
    }
  }

  /// Disable sized deallocation for MemoryBufferMMapFile, because it has
  /// tail-allocated data.
  فراغ operator delete(فراغ *p) { std::حرر(p); }

  StringRef getBufferIdentifier() ثابت override {
    // The name is stored after the class itself.
    أرجع StringRef(reinterpret_cast<ثابت حرف *>(هذا + 1) + حجم(size_t),
                     *reinterpret_cast<ثابت size_t *>(هذا + 1));
  }

  MemoryBuffer::BufferKind getBufferKind() ثابت override {
    أرجع MemoryBuffer::MemoryBuffer_MMap;
  }

  فراغ dontNeedIfMmap() override { MFR.dontNeed(); }
  فراغ willNeedIfMmap() override { MFR.willNeed(); }
  فراغ randomAccessIfMmap() override { MFR.randomAccess(); }
};
} // namespace

ساكن ErrorOr<std::unique_ptr<WritableMemoryBuffer>>
getMemoryBufferForStream(sys::fs::file_t FD, ثابت Twine &BufferName) {
  SmallString<sys::fs::DefaultReadChunkSize> Buffer;
  إذا (Error E = sys::fs::readNativeFileToEOF(FD, Buffer))
    أرجع errorToErrorCode(std::move(E));
  أرجع getMemBufferCopyImpl(Buffer, BufferName);
}

ErrorOr<std::unique_ptr<MemoryBuffer>>
MemoryBuffer::getFile(ثابت Twine &Filename, منطقي IsText,
                      منطقي RequiresNullTerminator, منطقي IsVolatile,
                      std::optional<Align> Alignment) {
  sys::sandbox::violationIfEnabled();

  أرجع getFileAux<MemoryBuffer>(Filename, /*MapSize=*/-1, /*Offset=*/0,
                                  IsText, RequiresNullTerminator, IsVolatile,
                                  Alignment);
}

template <typename MB>
ساكن ErrorOr<std::unique_ptr<MB>>
getOpenFileImpl(sys::fs::file_t FD, ثابت Twine &Filename, uint64_t FileSize,
                uint64_t MapSize, int64_t Offset, منطقي RequiresNullTerminator,
                منطقي IsVolatile, std::optional<Align> Alignment);

template <typename MB>
ساكن ErrorOr<std::unique_ptr<MB>>
getFileAux(ثابت Twine &Filename, uint64_t MapSize, uint64_t Offset,
           منطقي IsText, منطقي RequiresNullTerminator, منطقي IsVolatile,
           std::optional<Align> Alignment) {
  Expected<sys::fs::file_t> FDOrErr = sys::fs::openNativeFileForRead(
      Filename, IsText ? sys::fs::OF_TextWithCRLF : sys::fs::OF_None);
  إذا (!FDOrErr)
    أرجع errorToErrorCode(FDOrErr.takeError());
  sys::fs::file_t FD = *FDOrErr;
  auto Ret = getOpenFileImpl<MB>(FD, Filename, /*FileSize=*/-1, MapSize, Offset,
                                 RequiresNullTerminator, IsVolatile, Alignment);
  sys::fs::closeFile(FD);
  أرجع Ret;
}

ErrorOr<std::unique_ptr<WritableMemoryBuffer>>
WritableMemoryBuffer::getFile(ثابت Twine &Filename, منطقي IsVolatile,
                              std::optional<Align> Alignment) {
  sys::sandbox::violationIfEnabled();

  أرجع getFileAux<WritableMemoryBuffer>(
      Filename, /*MapSize=*/-1, /*Offset=*/0, /*IsText=*/خطأ,
      /*RequiresNullTerminator=*/خطأ, IsVolatile, Alignment);
}

ErrorOr<std::unique_ptr<WritableMemoryBuffer>>
WritableMemoryBuffer::getFileSlice(ثابت Twine &Filename, uint64_t MapSize,
                                   uint64_t Offset, منطقي IsVolatile,
                                   std::optional<Align> Alignment) {
  sys::sandbox::violationIfEnabled();

  أرجع getFileAux<WritableMemoryBuffer>(
      Filename, MapSize, Offset, /*IsText=*/خطأ,
      /*RequiresNullTerminator=*/خطأ, IsVolatile, Alignment);
}

std::unique_ptr<WritableMemoryBuffer>
WritableMemoryBuffer::getNewUninitMemBuffer(size_t Size,
                                            ثابت Twine &BufferName,
                                            std::optional<Align> Alignment) {
  using MemBuffer = MemoryBufferMem<WritableMemoryBuffer>;

  // Use 16-byte alignment if no alignment is specified.
  Align BufAlign = Alignment.قيمة_or(Align(16));

  // Allocate space for the MemoryBuffer, the data and the name. It is important
  // that MemoryBuffer and data are aligned so PointerIntPair works with them.
  SmallString<256> NameBuf;
  StringRef NameRef = BufferName.toStringRef(NameBuf);

  size_t StringLen = حجم(MemBuffer) + حجم(size_t) + NameRef.size() + 1;
  size_t RealLen = StringLen + Size + 1 + BufAlign.قيمة_2();
  إذا (RealLen <= Size) // Check for rollover.
    أرجع nullptr;
  // We use a call to malloc() rather than a call to a non-throwing operator
  // new() because LLVM unconditionally installs an out of memory new handler
  // when exceptions are disabled. This new handler intentionally crashes to
  // aid with debugging, but that makes non-throwing new calls unhelpful.
  // See MemoryBufferMem::operator delete() for the paired call to free(), and
  // llvm::install_out_of_memory_new_handler() for the installation of the
  // custom new handler.
  حرف *Mem = static_cast<حرف *>(std::malloc(RealLen));
  إذا (!Mem)
    أرجع nullptr;

  // The name is stored after the class itself.
  *reinterpret_cast<size_t *>(Mem + حجم(MemBuffer)) = NameRef.size();
  CopyStringRef(Mem + حجم(MemBuffer) + حجم(size_t), NameRef);

  // The buffer begins after the name and must be aligned.
  حرف *Buf = (حرف *)alignAddr(Mem + StringLen, BufAlign);
  Buf[Size] = 0; // Null terminate buffer.

  auto *Ret = جديد (Mem) MemBuffer(StringRef(Buf, Size), true);
  أرجع std::unique_ptr<WritableMemoryBuffer>(Ret);
}

std::unique_ptr<WritableMemoryBuffer>
WritableMemoryBuffer::getNewMemBuffer(size_t Size, ثابت Twine &BufferName) {
  auto SB = WritableMemoryBuffer::getNewUninitMemBuffer(Size, BufferName);
  إذا (!SB)
    أرجع nullptr;
  memset(SB->getBufferStart(), 0, Size);
  أرجع SB;
}

ساكن منطقي shouldUseMmap(sys::fs::file_t FD,
                          size_t FileSize,
                          size_t MapSize,
                          off_t Offset,
                          منطقي RequiresNullTerminator,
                          صحيح PageSize,
                          منطقي IsVolatile) {
#إذا defined(__MVS__)
  // zOS Enhanced ASCII auto convert does not support mmap.
  أرجع خطأ;
#endif

  // mmap may leave the buffer without null terminator if the file size changed
  // by the time the last page is mapped in, so avoid it if the file size is
  // likely to change.
  إذا (IsVolatile && RequiresNullTerminator)
    أرجع خطأ;

  // We don't use mmap for small files because this can severely fragment our
  // address space.
  إذا (MapSize < 4 * 4096 || MapSize < (unsigned)PageSize)
    أرجع خطأ;

  إذا (!RequiresNullTerminator)
    أرجع true;

  // If we don't know the file size, use fstat to find out.  fstat on an open
  // file descriptor is cheaper than stat on a random path.
  // FIXME: this chunk of code is duplicated, but it avoids a fstat when
  // RequiresNullTerminator = false and MapSize != -1.
  إذا (FileSize == size_t(-1)) {
    sys::fs::file_status Status;
    إذا (sys::fs::status(FD, Status))
      أرجع خطأ;
    FileSize = Status.getSize();
  }

  // If we need a null terminator and the end of the map is inside the file,
  // we cannot use mmap.
  size_t End = Offset + MapSize;
  أكد(End <= FileSize);
  إذا (End != FileSize)
    أرجع خطأ;

  // Don't try to map files that are exactly a multiple of the system page size
  // if we need a null terminator.
  إذا ((FileSize & (PageSize -1)) == 0)
    أرجع خطأ;

#إذا defined(__CYGWIN__)
  // Don't try to map files that are exactly a multiple of the physical page size
  // if we need a null terminator.
  // FIXME: We should reorganize again getPageSize() on Win32.
  إذا ((FileSize & (4096 - 1)) == 0)
    أرجع خطأ;
#endif

  أرجع true;
}

ساكن ErrorOr<std::unique_ptr<WriteThroughMemoryBuffer>>
getReadWriteFile(ثابت Twine &Filename, uint64_t FileSize, uint64_t MapSize,
                 uint64_t Offset) {
  Expected<sys::fs::file_t> FDOrErr = sys::fs::openNativeFileForReadWrite(
      Filename, sys::fs::CD_OpenExisting, sys::fs::OF_None);
  إذا (!FDOrErr)
    أرجع errorToErrorCode(FDOrErr.takeError());
  sys::fs::file_t FD = *FDOrErr;

  // Default is to map the full file.
  إذا (MapSize == uint64_t(-1)) {
    // If we don't know the file size, use fstat to find out.  fstat on an open
    // file descriptor is cheaper than stat on a random path.
    إذا (FileSize == uint64_t(-1)) {
      sys::fs::file_status Status;
      std::error_code EC = sys::fs::status(FD, Status);
      إذا (EC)
        أرجع EC;

      // If this not a file or a block device (e.g. it's a named pipe
      // or character device), we can't mmap it, so error out.
      sys::fs::file_type Type = Status.type();
      إذا (Type != sys::fs::file_type::regular_file &&
          Type != sys::fs::file_type::block_file)
        أرجع make_error_code(errc::invalid_argument);

      FileSize = Status.getSize();
    }
    MapSize = FileSize;
  }

  std::error_code EC;
  std::unique_ptr<WriteThroughMemoryBuffer> Result(
      جديد (NamedBufferAlloc(Filename))
          MemoryBufferMMapFile<WriteThroughMemoryBuffer>(خطأ, FD, MapSize,
                                                         Offset, EC));
  إذا (EC)
    أرجع EC;
  أرجع std::move(Result);
}

ErrorOr<std::unique_ptr<WriteThroughMemoryBuffer>>
WriteThroughMemoryBuffer::getFile(ثابت Twine &Filename, int64_t FileSize) {
  sys::sandbox::violationIfEnabled();

  أرجع getReadWriteFile(Filename, FileSize, FileSize, 0);
}

/// Map a subrange of the specified file as a WritableMemoryBuffer.
ErrorOr<std::unique_ptr<WriteThroughMemoryBuffer>>
WriteThroughMemoryBuffer::getFileSlice(ثابت Twine &Filename, uint64_t MapSize,
                                       uint64_t Offset) {
  sys::sandbox::violationIfEnabled();

  أرجع getReadWriteFile(Filename, -1, MapSize, Offset);
}

template <typename MB>
ساكن ErrorOr<std::unique_ptr<MB>>
getOpenFileImpl(sys::fs::file_t FD, ثابت Twine &Filename, uint64_t FileSize,
                uint64_t MapSize, int64_t Offset, منطقي RequiresNullTerminator,
                منطقي IsVolatile, std::optional<Align> Alignment) {
  ساكن صحيح PageSize = sys::Process::getPageSizeEstimate();

  // Default is to map the full file.
  إذا (MapSize == uint64_t(-1)) {
    // If we don't know the file size, use fstat to find out.  fstat on an open
    // file descriptor is cheaper than stat on a random path.
    إذا (FileSize == uint64_t(-1)) {
      sys::fs::file_status Status;
      std::error_code EC = sys::fs::status(FD, Status);
      إذا (EC)
        أرجع EC;

      // If this not a file or a block device (e.g. it's a named pipe
      // or character device), we can't trust the size. Create the memory
      // buffer by copying off the stream.
      sys::fs::file_type Type = Status.type();
      إذا (Type != sys::fs::file_type::regular_file &&
          Type != sys::fs::file_type::block_file)
        أرجع getMemoryBufferForStream(FD, Filename);

      FileSize = Status.getSize();
    }
    MapSize = FileSize;
  }

  إذا (shouldUseMmap(FD, FileSize, MapSize, Offset, RequiresNullTerminator,
                    PageSize, IsVolatile)) {
    std::error_code EC;
    std::unique_ptr<MB> Result(
        جديد (NamedBufferAlloc(Filename)) MemoryBufferMMapFile<MB>(
            RequiresNullTerminator, FD, MapSize, Offset, EC));
    إذا (!EC) {
      // On at least Linux, and possibly on other systems, mmap may return pages
      // from the page cache that are not properly filled with trailing zeroes,
      // if some prior user of the page wrote non-zero bytes. Detect this and
      // don't use mmap in that case.
      إذا (!RequiresNullTerminator || *Result->getBufferEnd() == '\0')
        أرجع std::move(Result);
    }
  }

#ifdef __MVS__
  ErrorOr<منطقي> NeedsConversion = needConversion(Filename, FD);
  إذا (std::error_code EC = NeedsConversion.getError())
    أرجع EC;
  // File size may increase due to EBCDIC -> UTF-8 conversion, therefore we
  // cannot trust the file size and we create the memory buffer by copying
  // off the stream.
  // Note: This only works with the assumption of reading a full file (i.e,
  // Offset == 0 and MapSize == FileSize). Reading a file slice does not work.
  إذا (*NeedsConversion && Offset == 0 && MapSize == FileSize)
    أرجع getMemoryBufferForStream(FD, Filename);
#endif

  auto Buf =
      WritableMemoryBuffer::getNewUninitMemBuffer(MapSize, Filename, Alignment);
  إذا (!Buf) {
    // Failed to create a buffer. The only way it can fail is if
    // new(std::nothrow) returns 0.
    أرجع make_error_code(errc::not_enough_memory);
  }

  // Read until EOF, zero-initialize the rest.
  MutableArrayRef<حرف> ToRead = Buf->getBuffer();
  طالما (!ToRead.empty()) {
    Expected<size_t> ReadBytes =
        sys::fs::readNativeFileSlice(FD, ToRead, Offset);
    إذا (!ReadBytes)
      أرجع errorToErrorCode(ReadBytes.takeError());
    إذا (*ReadBytes == 0) {
      std::memset(ToRead.data(), 0, ToRead.size());
      اكسر;
    }
    ToRead = ToRead.drop_front(*ReadBytes);
    Offset += *ReadBytes;
  }

  أرجع std::move(Buf);
}

ErrorOr<std::unique_ptr<MemoryBuffer>>
MemoryBuffer::getOpenFile(sys::fs::file_t FD, ثابت Twine &Filename,
                          uint64_t FileSize, منطقي RequiresNullTerminator,
                          منطقي IsVolatile, std::optional<Align> Alignment) {
  sys::sandbox::violationIfEnabled();

  أرجع getOpenFileImpl<MemoryBuffer>(FD, Filename, FileSize, FileSize, 0,
                                       RequiresNullTerminator, IsVolatile,
                                       Alignment);
}

ErrorOr<std::unique_ptr<MemoryBuffer>> MemoryBuffer::getOpenFileSlice(
    sys::fs::file_t FD, ثابت Twine &Filename, uint64_t MapSize, int64_t Offset,
    منطقي IsVolatile, std::optional<Align> Alignment) {
  أكد(MapSize != uint64_t(-1));

  sys::sandbox::violationIfEnabled();

  أرجع getOpenFileImpl<MemoryBuffer>(FD, Filename, -1, MapSize, Offset, خطأ,
                                       IsVolatile, Alignment);
}

ErrorOr<std::unique_ptr<MemoryBuffer>> MemoryBuffer::getSTDIN() {
  sys::sandbox::violationIfEnabled();

  // Read in all of the data from stdin, we cannot mmap stdin.
  //
  // FIXME: That isn't necessarily true, we should try to mmap stdin and
  // fallback if it fails.
  sys::ChangeStdinMode(sys::fs::OF_Text);

  أرجع getMemoryBufferForStream(sys::fs::getStdinHandle(), "<stdin>");
}

ErrorOr<std::unique_ptr<MemoryBuffer>>
MemoryBuffer::getFileAsStream(ثابت Twine &Filename) {
  sys::sandbox::violationIfEnabled();

  Expected<sys::fs::file_t> FDOrErr =
      sys::fs::openNativeFileForRead(Filename, sys::fs::OF_None);
  إذا (!FDOrErr)
    أرجع errorToErrorCode(FDOrErr.takeError());
  sys::fs::file_t FD = *FDOrErr;
  ErrorOr<std::unique_ptr<MemoryBuffer>> Ret =
      getMemoryBufferForStream(FD, Filename);
  sys::fs::closeFile(FD);
  أرجع Ret;
}

MemoryBufferRef MemoryBuffer::getMemBufferRef() ثابت {
  StringRef Data = getBuffer();
  StringRef Identifier = getBufferIdentifier();
  أرجع MemoryBufferRef(Data, Identifier);
}

SmallVectorMemoryBuffer::~SmallVectorMemoryBuffer() = افتراضي;
