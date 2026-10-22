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
#यदि !defined(_MSC_VER) && !defined(__MINGW32__)
#अन्तर्भाव <unistd.h>
#अन्यथा
#अन्तर्भाव <io.h>
#endif

using नामाकाश llvm;

//===----------------------------------------------------------------------===//
// MemoryBuffer implementation itself.
//===----------------------------------------------------------------------===//

MemoryBuffer::~MemoryBuffer() = स्वभाव;

/// init - Initialize this MemoryBuffer as a reference to externally allocated
/// memory, memory that we know is already null terminated.
शून्य MemoryBuffer::init(अचर वर्ण *BufStart, अचर वर्ण *BufEnd,
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
  यदि (!Data.empty())
    स_नकल(Memory, Data.data(), Data.size());
  Memory[Data.size()] = 0; // Null terminate string.
}

नामाकाश {
संरचना NamedBufferAlloc {
  अचर Twine &Name;
  NamedBufferAlloc(अचर Twine &Name) : Name(Name) {}
};
} // namespace

शून्य *चालक नूतन(माप_प्रकार N, अचर NamedBufferAlloc &Alloc) {
  SmallString<256> NameBuf;
  StringRef NameRef = Alloc.Name.toStringRef(NameBuf);

  // We use malloc() and manually handle it returning null instead of calling
  // operator new because we need all uses of NamedBufferAlloc to be
  // deallocated with a call to free() due to needing to use malloc() in
  // WritableMemoryBuffer::getNewUninitMemBuffer() to work around the out-of-
  // memory handler installed by default in LLVM. See operator delete() member
  // functions within this file for the paired call to free().
  वर्ण *Mem =
      static_cast<वर्ण *>(std::दो_स्मृति(N + परिमाण(माप_प्रकार) + NameRef.size() + 1));
  यदि (!Mem)
    llvm::report_bad_alloc_error("Allocation failed");
  *reinterpret_cast<माप_प्रकार *>(Mem + N) = NameRef.size();
  CopyStringRef(Mem + N + परिमाण(माप_प्रकार), NameRef);
  फर्काऊ Mem;
}

नामाकाश {
/// MemoryBufferMem - Named MemoryBuffer pointing to a block of memory.
ढाँचा<typename MB>
वर्ग MemoryBufferMem : सार्वजनिक MB {
सार्वजनिक:
  MemoryBufferMem(StringRef आगत_Data, तार्किक RequiresNullTerminator) {
    MemoryBuffer::init(आगत_Data.begin(), आगत_Data.end(),
                       RequiresNullTerminator);
  }

  /// Disable sized deallocation for MemoryBufferMem, because it has
  /// tail-allocated data.
  शून्य चालक delete(शून्य *p) { std::मुक्त(p); }

  StringRef getBufferIdentifier() अचर override {
    // The name is stored after the class itself.
    फर्काऊ StringRef(reinterpret_cast<अचर वर्ण *>(एतत् + 1) + परिमाण(माप_प्रकार),
                     *reinterpret_cast<अचर माप_प्रकार *>(एतत् + 1));
  }

  MemoryBuffer::BufferKind getBufferKind() अचर override {
    फर्काऊ MemoryBuffer::MemoryBuffer_Malloc;
  }
};
} // namespace

ढाँचा <typename MB>
स्थिर ErrorOr<std::unique_ptr<MB>>
getFileAux(अचर Twine &Filename, uint64_t MapSize, uint64_t Offset,
           तार्किक IsText, तार्किक RequiresNullTerminator, तार्किक IsVolatile,
           std::optional<Align> Alignment);

std::unique_ptr<MemoryBuffer>
MemoryBuffer::getMemBuffer(StringRef आगत_Data, StringRef BufferName,
                           तार्किक RequiresNullTerminator) {
  स्वतः *Ret = नूतन (NamedBufferAlloc(BufferName))
      MemoryBufferMem<MemoryBuffer>(आगत_Data, RequiresNullTerminator);
  फर्काऊ std::unique_ptr<MemoryBuffer>(Ret);
}

std::unique_ptr<MemoryBuffer>
MemoryBuffer::getMemBuffer(MemoryBufferRef Ref, तार्किक RequiresNullTerminator) {
  फर्काऊ std::unique_ptr<MemoryBuffer>(getMemBuffer(
      Ref.getBuffer(), Ref.getBufferIdentifier(), RequiresNullTerminator));
}

स्थिर ErrorOr<std::unique_ptr<WritableMemoryBuffer>>
getMemBufferCopyImpl(StringRef आगत_Data, अचर Twine &BufferName) {
  स्वतः Buf =
      WritableMemoryBuffer::getNewUninitMemBuffer(आगत_Data.size(), BufferName);
  यदि (!Buf)
    फर्काऊ make_error_code(errc::not_enough_memory);
  // Calling memcpy with null src/dst is UB, and an empty StringRef is
  // represented with {nullptr, 0}.
  llvm::copy(आगत_Data, Buf->getBufferStart());
  फर्काऊ std::move(Buf);
}

std::unique_ptr<MemoryBuffer>
MemoryBuffer::getMemBufferCopy(StringRef आगत_Data, अचर Twine &BufferName) {
  स्वतः Buf = getMemBufferCopyImpl(आगत_Data, BufferName);
  यदि (Buf)
    फर्काऊ std::move(*Buf);
  फर्काऊ nullptr;
}

ErrorOr<std::unique_ptr<MemoryBuffer>>
MemoryBuffer::getFileOrSTDIN(अचर Twine &Filename, तार्किक IsText,
                             तार्किक RequiresNullTerminator,
                             std::optional<Align> Alignment) {
  sys::sandbox::violationIfEnabled();

  SmallString<256> NameBuf;
  StringRef NameRef = Filename.toStringRef(NameBuf);

  यदि (NameRef == "-")
    फर्काऊ getSTDIN();
  फर्काऊ getFile(Filename, IsText, RequiresNullTerminator,
                 /*IsVolatile=*/असत्य, Alignment);
}

ErrorOr<std::unique_ptr<MemoryBuffer>>
MemoryBuffer::getFileSlice(अचर Twine &FilePath, uint64_t MapSize,
                           uint64_t Offset, तार्किक IsVolatile,
                           std::optional<Align> Alignment) {
  sys::sandbox::violationIfEnabled();

  फर्काऊ getFileAux<MemoryBuffer>(FilePath, MapSize, Offset, /*IsText=*/असत्य,
                                  /*RequiresNullTerminator=*/असत्य, IsVolatile,
                                  Alignment);
}

//===----------------------------------------------------------------------===//
// MemoryBuffer::getFile implementation.
//===----------------------------------------------------------------------===//

नामाकाश {

ढाँचा <typename MB>
constexpr sys::fs::mapped_file_region::mapmode Mapmode =
    sys::fs::mapped_file_region::readonly;
ढाँचा <>
constexpr sys::fs::mapped_file_region::mapmode Mapmode<MemoryBuffer> =
    sys::fs::mapped_file_region::readonly;
ढाँचा <>
constexpr sys::fs::mapped_file_region::mapmode Mapmode<WritableMemoryBuffer> =
    sys::fs::mapped_file_region::priv;
ढाँचा <>
constexpr sys::fs::mapped_file_region::mapmode
    Mapmode<WriteThroughMemoryBuffer> = sys::fs::mapped_file_region::readwrite;

/// Memory maps a file descriptor using sys::fs::mapped_file_region.
///
/// This handles converting the offset into a legal offset on the platform.
ढाँचा<typename MB>
वर्ग MemoryBufferMMapFile : सार्वजनिक MB {
  sys::fs::mapped_file_region MFR;

  स्थिर uint64_t getLegalMapOffset(uint64_t Offset) {
    फर्काऊ Offset & ~(sys::fs::mapped_file_region::alignment() - 1);
  }

  स्थिर uint64_t getLegalMapSize(uint64_t Len, uint64_t Offset) {
    फर्काऊ Len + (Offset - getLegalMapOffset(Offset));
  }

  अचर वर्ण *getStart(uint64_t Len, uint64_t Offset) {
    फर्काऊ MFR.const_data() + (Offset - getLegalMapOffset(Offset));
  }

सार्वजनिक:
  MemoryBufferMMapFile(तार्किक RequiresNullTerminator, sys::fs::file_t FD, uint64_t Len,
                       uint64_t Offset, std::error_code &EC)
      : MFR(FD, Mapmode<MB>, getLegalMapSize(Len, Offset),
            getLegalMapOffset(Offset), EC) {
    यदि (!EC) {
      अचर वर्ण *Start = getStart(Len, Offset);
      MemoryBuffer::init(Start, Start + Len, RequiresNullTerminator);
    }
  }

  /// Disable sized deallocation for MemoryBufferMMapFile, because it has
  /// tail-allocated data.
  शून्य चालक delete(शून्य *p) { std::मुक्त(p); }

  StringRef getBufferIdentifier() अचर override {
    // The name is stored after the class itself.
    फर्काऊ StringRef(reinterpret_cast<अचर वर्ण *>(एतत् + 1) + परिमाण(माप_प्रकार),
                     *reinterpret_cast<अचर माप_प्रकार *>(एतत् + 1));
  }

  MemoryBuffer::BufferKind getBufferKind() अचर override {
    फर्काऊ MemoryBuffer::MemoryBuffer_MMap;
  }

  शून्य dontNeedIfMmap() override { MFR.dontNeed(); }
  शून्य willNeedIfMmap() override { MFR.willNeed(); }
  शून्य randomAccessIfMmap() override { MFR.randomAccess(); }
};
} // namespace

स्थिर ErrorOr<std::unique_ptr<WritableMemoryBuffer>>
getMemoryBufferForStream(sys::fs::file_t FD, अचर Twine &BufferName) {
  SmallString<sys::fs::DefaultReadChunkSize> Buffer;
  यदि (Error E = sys::fs::readNativeFileToEOF(FD, Buffer))
    फर्काऊ errorToErrorCode(std::move(E));
  फर्काऊ getMemBufferCopyImpl(Buffer, BufferName);
}

ErrorOr<std::unique_ptr<MemoryBuffer>>
MemoryBuffer::getFile(अचर Twine &Filename, तार्किक IsText,
                      तार्किक RequiresNullTerminator, तार्किक IsVolatile,
                      std::optional<Align> Alignment) {
  sys::sandbox::violationIfEnabled();

  फर्काऊ getFileAux<MemoryBuffer>(Filename, /*MapSize=*/-1, /*Offset=*/0,
                                  IsText, RequiresNullTerminator, IsVolatile,
                                  Alignment);
}

ढाँचा <typename MB>
स्थिर ErrorOr<std::unique_ptr<MB>>
getOpenFileImpl(sys::fs::file_t FD, अचर Twine &Filename, uint64_t FileSize,
                uint64_t MapSize, int64_t Offset, तार्किक RequiresNullTerminator,
                तार्किक IsVolatile, std::optional<Align> Alignment);

ढाँचा <typename MB>
स्थिर ErrorOr<std::unique_ptr<MB>>
getFileAux(अचर Twine &Filename, uint64_t MapSize, uint64_t Offset,
           तार्किक IsText, तार्किक RequiresNullTerminator, तार्किक IsVolatile,
           std::optional<Align> Alignment) {
  Expected<sys::fs::file_t> FDOrErr = sys::fs::openNativeFileForRead(
      Filename, IsText ? sys::fs::OF_TextWithCRLF : sys::fs::OF_None);
  यदि (!FDOrErr)
    फर्काऊ errorToErrorCode(FDOrErr.takeError());
  sys::fs::file_t FD = *FDOrErr;
  स्वतः Ret = getOpenFileImpl<MB>(FD, Filename, /*FileSize=*/-1, MapSize, Offset,
                                 RequiresNullTerminator, IsVolatile, Alignment);
  sys::fs::closeFile(FD);
  फर्काऊ Ret;
}

ErrorOr<std::unique_ptr<WritableMemoryBuffer>>
WritableMemoryBuffer::getFile(अचर Twine &Filename, तार्किक IsVolatile,
                              std::optional<Align> Alignment) {
  sys::sandbox::violationIfEnabled();

  फर्काऊ getFileAux<WritableMemoryBuffer>(
      Filename, /*MapSize=*/-1, /*Offset=*/0, /*IsText=*/असत्य,
      /*RequiresNullTerminator=*/असत्य, IsVolatile, Alignment);
}

ErrorOr<std::unique_ptr<WritableMemoryBuffer>>
WritableMemoryBuffer::getFileSlice(अचर Twine &Filename, uint64_t MapSize,
                                   uint64_t Offset, तार्किक IsVolatile,
                                   std::optional<Align> Alignment) {
  sys::sandbox::violationIfEnabled();

  फर्काऊ getFileAux<WritableMemoryBuffer>(
      Filename, MapSize, Offset, /*IsText=*/असत्य,
      /*RequiresNullTerminator=*/असत्य, IsVolatile, Alignment);
}

std::unique_ptr<WritableMemoryBuffer>
WritableMemoryBuffer::getNewUninitMemBuffer(माप_प्रकार Size,
                                            अचर Twine &BufferName,
                                            std::optional<Align> Alignment) {
  using MemBuffer = MemoryBufferMem<WritableMemoryBuffer>;

  // Use 16-byte alignment if no alignment is specified.
  Align BufAlign = Alignment.मान_or(Align(16));

  // Allocate space for the MemoryBuffer, the data and the name. It is important
  // that MemoryBuffer and data are aligned so PointerIntPair works with them.
  SmallString<256> NameBuf;
  StringRef NameRef = BufferName.toStringRef(NameBuf);

  माप_प्रकार StringLen = परिमाण(MemBuffer) + परिमाण(माप_प्रकार) + NameRef.size() + 1;
  माप_प्रकार RealLen = StringLen + Size + 1 + BufAlign.मान_2();
  यदि (RealLen <= Size) // Check for rollover.
    फर्काऊ nullptr;
  // We use a call to malloc() rather than a call to a non-throwing operator
  // new() because LLVM unconditionally installs an out of memory new handler
  // when exceptions are disabled. This new handler intentionally crashes to
  // aid with debugging, but that makes non-throwing new calls unhelpful.
  // See MemoryBufferMem::operator delete() for the paired call to free(), and
  // llvm::install_out_of_memory_new_handler() for the installation of the
  // custom new handler.
  वर्ण *Mem = static_cast<वर्ण *>(std::दो_स्मृति(RealLen));
  यदि (!Mem)
    फर्काऊ nullptr;

  // The name is stored after the class itself.
  *reinterpret_cast<माप_प्रकार *>(Mem + परिमाण(MemBuffer)) = NameRef.size();
  CopyStringRef(Mem + परिमाण(MemBuffer) + परिमाण(माप_प्रकार), NameRef);

  // The buffer begins after the name and must be aligned.
  वर्ण *Buf = (वर्ण *)alignAddr(Mem + StringLen, BufAlign);
  Buf[Size] = 0; // Null terminate buffer.

  स्वतः *Ret = नूतन (Mem) MemBuffer(StringRef(Buf, Size), सत्य);
  फर्काऊ std::unique_ptr<WritableMemoryBuffer>(Ret);
}

std::unique_ptr<WritableMemoryBuffer>
WritableMemoryBuffer::getNewMemBuffer(माप_प्रकार Size, अचर Twine &BufferName) {
  स्वतः SB = WritableMemoryBuffer::getNewUninitMemBuffer(Size, BufferName);
  यदि (!SB)
    फर्काऊ nullptr;
  स_रखो(SB->getBufferStart(), 0, Size);
  फर्काऊ SB;
}

स्थिर तार्किक shouldUseMmap(sys::fs::file_t FD,
                          माप_प्रकार FileSize,
                          माप_प्रकार MapSize,
                          off_t Offset,
                          तार्किक RequiresNullTerminator,
                          पूर्णाङ्क PageSize,
                          तार्किक IsVolatile) {
#यदि defined(__MVS__)
  // zOS Enhanced ASCII auto convert does not support mmap.
  फर्काऊ असत्य;
#endif

  // mmap may leave the buffer without null terminator if the file size changed
  // by the time the last page is mapped in, so avoid it if the file size is
  // likely to change.
  यदि (IsVolatile && RequiresNullTerminator)
    फर्काऊ असत्य;

  // We don't use mmap for small files because this can severely fragment our
  // address space.
  यदि (MapSize < 4 * 4096 || MapSize < (अचिन्हित)PageSize)
    फर्काऊ असत्य;

  यदि (!RequiresNullTerminator)
    फर्काऊ सत्य;

  // If we don't know the file size, use fstat to find out.  fstat on an open
  // file descriptor is cheaper than stat on a random path.
  // FIXME: this chunk of code is duplicated, but it avoids a fstat when
  // RequiresNullTerminator = false and MapSize != -1.
  यदि (FileSize == माप_प्रकार(-1)) {
    sys::fs::file_status Status;
    यदि (sys::fs::status(FD, Status))
      फर्काऊ असत्य;
    FileSize = Status.getSize();
  }

  // If we need a null terminator and the end of the map is inside the file,
  // we cannot use mmap.
  माप_प्रकार End = Offset + MapSize;
  प्रतिज्ञा(End <= FileSize);
  यदि (End != FileSize)
    फर्काऊ असत्य;

  // Don't try to map files that are exactly a multiple of the system page size
  // if we need a null terminator.
  यदि ((FileSize & (PageSize -1)) == 0)
    फर्काऊ असत्य;

#यदि defined(__CYGWIN__)
  // Don't try to map files that are exactly a multiple of the physical page size
  // if we need a null terminator.
  // FIXME: We should reorganize again getPageSize() on Win32.
  यदि ((FileSize & (4096 - 1)) == 0)
    फर्काऊ असत्य;
#endif

  फर्काऊ सत्य;
}

स्थिर ErrorOr<std::unique_ptr<WriteThroughMemoryBuffer>>
getReadWriteFile(अचर Twine &Filename, uint64_t FileSize, uint64_t MapSize,
                 uint64_t Offset) {
  Expected<sys::fs::file_t> FDOrErr = sys::fs::openNativeFileForReadWrite(
      Filename, sys::fs::CD_OpenExisting, sys::fs::OF_None);
  यदि (!FDOrErr)
    फर्काऊ errorToErrorCode(FDOrErr.takeError());
  sys::fs::file_t FD = *FDOrErr;

  // Default is to map the full file.
  यदि (MapSize == uint64_t(-1)) {
    // If we don't know the file size, use fstat to find out.  fstat on an open
    // file descriptor is cheaper than stat on a random path.
    यदि (FileSize == uint64_t(-1)) {
      sys::fs::file_status Status;
      std::error_code EC = sys::fs::status(FD, Status);
      यदि (EC)
        फर्काऊ EC;

      // If this not a file or a block device (e.g. it's a named pipe
      // or character device), we can't mmap it, so error out.
      sys::fs::file_type Type = Status.type();
      यदि (Type != sys::fs::file_type::regular_file &&
          Type != sys::fs::file_type::block_file)
        फर्काऊ make_error_code(errc::invalid_argument);

      FileSize = Status.getSize();
    }
    MapSize = FileSize;
  }

  std::error_code EC;
  std::unique_ptr<WriteThroughMemoryBuffer> Result(
      नूतन (NamedBufferAlloc(Filename))
          MemoryBufferMMapFile<WriteThroughMemoryBuffer>(असत्य, FD, MapSize,
                                                         Offset, EC));
  यदि (EC)
    फर्काऊ EC;
  फर्काऊ std::move(Result);
}

ErrorOr<std::unique_ptr<WriteThroughMemoryBuffer>>
WriteThroughMemoryBuffer::getFile(अचर Twine &Filename, int64_t FileSize) {
  sys::sandbox::violationIfEnabled();

  फर्काऊ getReadWriteFile(Filename, FileSize, FileSize, 0);
}

/// Map a subrange of the specified file as a WritableMemoryBuffer.
ErrorOr<std::unique_ptr<WriteThroughMemoryBuffer>>
WriteThroughMemoryBuffer::getFileSlice(अचर Twine &Filename, uint64_t MapSize,
                                       uint64_t Offset) {
  sys::sandbox::violationIfEnabled();

  फर्काऊ getReadWriteFile(Filename, -1, MapSize, Offset);
}

ढाँचा <typename MB>
स्थिर ErrorOr<std::unique_ptr<MB>>
getOpenFileImpl(sys::fs::file_t FD, अचर Twine &Filename, uint64_t FileSize,
                uint64_t MapSize, int64_t Offset, तार्किक RequiresNullTerminator,
                तार्किक IsVolatile, std::optional<Align> Alignment) {
  स्थिर पूर्णाङ्क PageSize = sys::Process::getPageSizeEstimate();

  // Default is to map the full file.
  यदि (MapSize == uint64_t(-1)) {
    // If we don't know the file size, use fstat to find out.  fstat on an open
    // file descriptor is cheaper than stat on a random path.
    यदि (FileSize == uint64_t(-1)) {
      sys::fs::file_status Status;
      std::error_code EC = sys::fs::status(FD, Status);
      यदि (EC)
        फर्काऊ EC;

      // If this not a file or a block device (e.g. it's a named pipe
      // or character device), we can't trust the size. Create the memory
      // buffer by copying off the stream.
      sys::fs::file_type Type = Status.type();
      यदि (Type != sys::fs::file_type::regular_file &&
          Type != sys::fs::file_type::block_file)
        फर्काऊ getMemoryBufferForStream(FD, Filename);

      FileSize = Status.getSize();
    }
    MapSize = FileSize;
  }

  यदि (shouldUseMmap(FD, FileSize, MapSize, Offset, RequiresNullTerminator,
                    PageSize, IsVolatile)) {
    std::error_code EC;
    std::unique_ptr<MB> Result(
        नूतन (NamedBufferAlloc(Filename)) MemoryBufferMMapFile<MB>(
            RequiresNullTerminator, FD, MapSize, Offset, EC));
    यदि (!EC) {
      // On at least Linux, and possibly on other systems, mmap may return pages
      // from the page cache that are not properly filled with trailing zeroes,
      // if some prior user of the page wrote non-zero bytes. Detect this and
      // don't use mmap in that case.
      यदि (!RequiresNullTerminator || *Result->getBufferEnd() == '\0')
        फर्काऊ std::move(Result);
    }
  }

#ifdef __MVS__
  ErrorOr<तार्किक> NeedsConversion = needConversion(Filename, FD);
  यदि (std::error_code EC = NeedsConversion.getError())
    फर्काऊ EC;
  // File size may increase due to EBCDIC -> UTF-8 conversion, therefore we
  // cannot trust the file size and we create the memory buffer by copying
  // off the stream.
  // Note: This only works with the assumption of reading a full file (i.e,
  // Offset == 0 and MapSize == FileSize). Reading a file slice does not work.
  यदि (*NeedsConversion && Offset == 0 && MapSize == FileSize)
    फर्काऊ getMemoryBufferForStream(FD, Filename);
#endif

  स्वतः Buf =
      WritableMemoryBuffer::getNewUninitMemBuffer(MapSize, Filename, Alignment);
  यदि (!Buf) {
    // Failed to create a buffer. The only way it can fail is if
    // new(std::nothrow) returns 0.
    फर्काऊ make_error_code(errc::not_enough_memory);
  }

  // Read until EOF, zero-initialize the rest.
  MutableArrayRef<वर्ण> ToRead = Buf->getBuffer();
  जबसम्म (!ToRead.empty()) {
    Expected<माप_प्रकार> ReadBytes =
        sys::fs::readNativeFileSlice(FD, ToRead, Offset);
    यदि (!ReadBytes)
      फर्काऊ errorToErrorCode(ReadBytes.takeError());
    यदि (*ReadBytes == 0) {
      std::स_रखो(ToRead.data(), 0, ToRead.size());
      रोक;
    }
    ToRead = ToRead.drop_front(*ReadBytes);
    Offset += *ReadBytes;
  }

  फर्काऊ std::move(Buf);
}

ErrorOr<std::unique_ptr<MemoryBuffer>>
MemoryBuffer::getOpenFile(sys::fs::file_t FD, अचर Twine &Filename,
                          uint64_t FileSize, तार्किक RequiresNullTerminator,
                          तार्किक IsVolatile, std::optional<Align> Alignment) {
  sys::sandbox::violationIfEnabled();

  फर्काऊ getOpenFileImpl<MemoryBuffer>(FD, Filename, FileSize, FileSize, 0,
                                       RequiresNullTerminator, IsVolatile,
                                       Alignment);
}

ErrorOr<std::unique_ptr<MemoryBuffer>> MemoryBuffer::getOpenFileSlice(
    sys::fs::file_t FD, अचर Twine &Filename, uint64_t MapSize, int64_t Offset,
    तार्किक IsVolatile, std::optional<Align> Alignment) {
  प्रतिज्ञा(MapSize != uint64_t(-1));

  sys::sandbox::violationIfEnabled();

  फर्काऊ getOpenFileImpl<MemoryBuffer>(FD, Filename, -1, MapSize, Offset, असत्य,
                                       IsVolatile, Alignment);
}

ErrorOr<std::unique_ptr<MemoryBuffer>> MemoryBuffer::getSTDIN() {
  sys::sandbox::violationIfEnabled();

  // Read in all of the data from stdin, we cannot mmap stdin.
  //
  // FIXME: That isn't necessarily true, we should try to mmap stdin and
  // fallback if it fails.
  sys::ChangeStdinMode(sys::fs::OF_Text);

  फर्काऊ getMemoryBufferForStream(sys::fs::getStdinHandle(), "<stdin>");
}

ErrorOr<std::unique_ptr<MemoryBuffer>>
MemoryBuffer::getFileAsStream(अचर Twine &Filename) {
  sys::sandbox::violationIfEnabled();

  Expected<sys::fs::file_t> FDOrErr =
      sys::fs::openNativeFileForRead(Filename, sys::fs::OF_None);
  यदि (!FDOrErr)
    फर्काऊ errorToErrorCode(FDOrErr.takeError());
  sys::fs::file_t FD = *FDOrErr;
  ErrorOr<std::unique_ptr<MemoryBuffer>> Ret =
      getMemoryBufferForStream(FD, Filename);
  sys::fs::closeFile(FD);
  फर्काऊ Ret;
}

MemoryBufferRef MemoryBuffer::getMemBufferRef() अचर {
  StringRef Data = getBuffer();
  StringRef Identifier = getBufferIdentifier();
  फर्काऊ MemoryBufferRef(Data, Identifier);
}

SmallVectorMemoryBuffer::~SmallVectorMemoryBuffer() = स्वभाव;
