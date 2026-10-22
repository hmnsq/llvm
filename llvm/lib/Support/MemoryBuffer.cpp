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

#समावेश "llvm/Support/MemoryBuffer.h"
#समावेश "llvm/ADT/STLExtras.h"
#समावेश "llvm/ADT/SmallString.h"
#समावेश "llvm/Config/config.h"
#समावेश "llvm/Support/Alignment.h"
#समावेश "llvm/Support/AutoConvert.h"
#समावेश "llvm/Support/Errc.h"
#समावेश "llvm/Support/Error.h"
#समावेश "llvm/Support/ErrorHandling.h"
#समावेश "llvm/Support/FileSystem.h"
#समावेश "llvm/Support/IOSandbox.h"
#समावेश "llvm/Support/Process.h"
#समावेश "llvm/Support/Program.h"
#समावेश "llvm/Support/SmallVectorMemoryBuffer.h"
#समावेश <algorithm>
#समावेश <cassert>
#समावेश <cstring>
#समावेश <new>
#समावेश <sys/types.h>
#समावेश <system_error>
#अगर !defined(_MSC_VER) && !defined(__MINGW32__)
#समावेश <unistd.h>
#अन्यथा
#समावेश <io.h>
#पूर्ण_अगर

using namespace llvm;

//===----------------------------------------------------------------------===//
// MemoryBuffer implementation itself.
//===----------------------------------------------------------------------===//

Memory_बफर::~Memory_बफर() = शेष;

/// init - Initialize this MemoryBuffer as a reference to externally allocated
/// memory, memory that we know is already null terminated.
व्योम Memory_बफर::init(स्थिर अक्षर *BufStart, स्थिर अक्षर *BufEnd,
                        bool RequiresNullTerminator) {
  निश्चित((!RequiresNullTerminator || BufEnd[0] == 0) &&
         "Buffer is not null terminated!");
  बफर_Start = BufStart;
  बफर_End = BufEnd;
}

//===----------------------------------------------------------------------===//
// MemoryBufferMem implementation.
//===----------------------------------------------------------------------===//

/// CopyStringRef - Copies contents of a StringRef into a block of memory and
/// null-terminates it.
अटल व्योम CopyStringRef(अक्षर *Memory, StringRef तथ्य_2) {
  अगर (!तथ्य_2.empty())
    स_नकल(Memory, तथ्य_2.तथ्य(), तथ्य_2.आकार_2());
  Memory[तथ्य_2.आकार_2()] = 0; // Null terminate string.
}

namespace {
काष्ठा Named_बफर_Alloc {
  स्थिर Twine &Name;
  Named_बफर_Alloc(स्थिर Twine &Name) : Name(Name) {}
};
} // namespace

व्योम *चालक नया(माप_प्रकार N, स्थिर Named_बफर_Alloc &Alloc) {
  SmallString<256> NameBuf;
  StringRef NameRef = Alloc.Name.toStringRef(NameBuf);

  // We use malloc() and manually handle it returning null instead of calling
  // operator new because we need all uses of NamedBufferAlloc to be
  // deallocated with a call to free() due to needing to use malloc() in
  // WritableMemoryBuffer::getNewUninitMemBuffer() to work around the out-of-
  // memory handler installed by default in LLVM. See operator delete() member
  // functions within this file for the paired call to free().
  अक्षर *Mem =
      static_cast<अक्षर *>(std::दो_स्मृति(N + माप(माप_प्रकार) + NameRef.आकार_2() + 1));
  अगर (!Mem)
    llvm::report_bad_alloc_error("Allocation failed");
  *reinterpret_cast<माप_प्रकार *>(Mem + N) = NameRef.आकार_2();
  CopyStringRef(Mem + N + माप(माप_प्रकार), NameRef);
  वापस Mem;
}

namespace {
/// MemoryBufferMem - Named MemoryBuffer pointing to a block of memory.
ढाँचा<typename MB>
श्रेणी Memory_बफर_Mem : खुला MB {
खुला:
  Memory_बफर_Mem(StringRef निवेश_तथ्य, bool RequiresNullTerminator) {
    Memory_बफर::init(निवेश_तथ्य.begin(), निवेश_तथ्य.end(),
                       RequiresNullTerminator);
  }

  /// Disable sized deallocation for MemoryBufferMem, because it has
  /// tail-allocated data.
  व्योम चालक मिटाओ(व्योम *p) { std::मुक्त(p); }

  StringRef get_बफर_Identifier() स्थिर override {
    // The name is stored after the class itself.
    वापस StringRef(reinterpret_cast<स्थिर अक्षर *>(यह + 1) + माप(माप_प्रकार),
                     *reinterpret_cast<स्थिर माप_प्रकार *>(यह + 1));
  }

  Memory_बफर::बफर_Kind get_बफर_Kind() स्थिर override {
    वापस Memory_बफर::Memory_बफर_Malloc;
  }
};
} // namespace

ढाँचा <typename MB>
अटल ErrorOr<std::unique_ptr<MB>>
getFileAux(स्थिर Twine &Filename, uint64_t Map_आकार, uint64_t Offset,
           bool IsText, bool RequiresNullTerminator, bool IsVolatile,
           std::optional<Align> Alignment);

std::unique_ptr<Memory_बफर>
Memory_बफर::get_Mem_बफर(StringRef निवेश_तथ्य, StringRef बफर_Name,
                           bool RequiresNullTerminator) {
  स्वतः *Ret = नया (Named_बफर_Alloc(बफर_Name))
      Memory_बफर_Mem<Memory_बफर>(निवेश_तथ्य, RequiresNullTerminator);
  वापस std::unique_ptr<Memory_बफर>(Ret);
}

std::unique_ptr<Memory_बफर>
Memory_बफर::get_Mem_बफर(Memory_बफर_Ref Ref, bool RequiresNullTerminator) {
  वापस std::unique_ptr<Memory_बफर>(get_Mem_बफर(
      Ref.get_बफर(), Ref.get_बफर_Identifier(), RequiresNullTerminator));
}

अटल ErrorOr<std::unique_ptr<Writable_Memory_बफर>>
get_Mem_बफर_Copy_Impl(StringRef निवेश_तथ्य, स्थिर Twine &बफर_Name) {
  स्वतः Buf =
      Writable_Memory_बफर::get_New_Uninit_Mem_बफर(निवेश_तथ्य.आकार_2(), बफर_Name);
  अगर (!Buf)
    वापस make_error_code(errc::not_enough_memory);
  // Calling memcpy with null src/dst is UB, and an empty StringRef is
  // represented with {nullptr, 0}.
  llvm::copy(निवेश_तथ्य, Buf->get_बफर_Start());
  वापस std::move(Buf);
}

std::unique_ptr<Memory_बफर>
Memory_बफर::get_Mem_बफर_Copy(StringRef निवेश_तथ्य, स्थिर Twine &बफर_Name) {
  स्वतः Buf = get_Mem_बफर_Copy_Impl(निवेश_तथ्य, बफर_Name);
  अगर (Buf)
    वापस std::move(*Buf);
  वापस nullptr;
}

ErrorOr<std::unique_ptr<Memory_बफर>>
Memory_बफर::getFileOrSTDIN(स्थिर Twine &Filename, bool IsText,
                             bool RequiresNullTerminator,
                             std::optional<Align> Alignment) {
  sys::sandbox::violationIfEnabled();

  SmallString<256> NameBuf;
  StringRef NameRef = Filename.toStringRef(NameBuf);

  अगर (NameRef == "-")
    वापस getSTDIN();
  वापस getFile(Filename, IsText, RequiresNullTerminator,
                 /*IsVolatile=*/false, Alignment);
}

ErrorOr<std::unique_ptr<Memory_बफर>>
Memory_बफर::getFileSlice(स्थिर Twine &FilePath, uint64_t Map_आकार,
                           uint64_t Offset, bool IsVolatile,
                           std::optional<Align> Alignment) {
  sys::sandbox::violationIfEnabled();

  वापस getFileAux<Memory_बफर>(FilePath, Map_आकार, Offset, /*IsText=*/false,
                                  /*RequiresNullTerminator=*/false, IsVolatile,
                                  Alignment);
}

//===----------------------------------------------------------------------===//
// MemoryBuffer::getFile implementation.
//===----------------------------------------------------------------------===//

namespace {

ढाँचा <typename MB>
constexpr sys::fs::mapped_file_region::mapmode Mapmode =
    sys::fs::mapped_file_region::readonly;
ढाँचा <>
constexpr sys::fs::mapped_file_region::mapmode Mapmode<Memory_बफर> =
    sys::fs::mapped_file_region::readonly;
ढाँचा <>
constexpr sys::fs::mapped_file_region::mapmode Mapmode<Writable_Memory_बफर> =
    sys::fs::mapped_file_region::priv;
ढाँचा <>
constexpr sys::fs::mapped_file_region::mapmode
    Mapmode<Write_Through_Memory_बफर> = sys::fs::mapped_file_region::readwrite;

/// Memory maps a file descriptor using sys::fs::mapped_file_region.
///
/// This handles converting the offset into a legal offset on the platform.
ढाँचा<typename MB>
श्रेणी Memory_बफर_MMap_File : खुला MB {
  sys::fs::mapped_file_region MFR;

  अटल uint64_t getLegalMapOffset(uint64_t Offset) {
    वापस Offset & ~(sys::fs::mapped_file_region::alignment() - 1);
  }

  अटल uint64_t get_Legal_Map_आकार(uint64_t Len, uint64_t Offset) {
    वापस Len + (Offset - getLegalMapOffset(Offset));
  }

  स्थिर अक्षर *getStart(uint64_t Len, uint64_t Offset) {
    वापस MFR.const_तथ्य() + (Offset - getLegalMapOffset(Offset));
  }

खुला:
  Memory_बफर_MMap_File(bool RequiresNullTerminator, sys::fs::file_t FD, uint64_t Len,
                       uint64_t Offset, std::error_code &EC)
      : MFR(FD, Mapmode<MB>, get_Legal_Map_आकार(Len, Offset),
            getLegalMapOffset(Offset), EC) {
    अगर (!EC) {
      स्थिर अक्षर *Start = getStart(Len, Offset);
      Memory_बफर::init(Start, Start + Len, RequiresNullTerminator);
    }
  }

  /// Disable sized deallocation for MemoryBufferMMapFile, because it has
  /// tail-allocated data.
  व्योम चालक मिटाओ(व्योम *p) { std::मुक्त(p); }

  StringRef get_बफर_Identifier() स्थिर override {
    // The name is stored after the class itself.
    वापस StringRef(reinterpret_cast<स्थिर अक्षर *>(यह + 1) + माप(माप_प्रकार),
                     *reinterpret_cast<स्थिर माप_प्रकार *>(यह + 1));
  }

  Memory_बफर::बफर_Kind get_बफर_Kind() स्थिर override {
    वापस Memory_बफर::Memory_बफर_MMap;
  }

  व्योम dontNeedIfMmap() override { MFR.dontNeed(); }
  व्योम willNeedIfMmap() override { MFR.willNeed(); }
  व्योम randomAccessIfMmap() override { MFR.randomAccess(); }
};
} // namespace

अटल ErrorOr<std::unique_ptr<Writable_Memory_बफर>>
get_Memory_बफर_For_Stream(sys::fs::file_t FD, स्थिर Twine &बफर_Name) {
  SmallString<sys::fs::Default_Read_Chunk_आकार> बफर;
  अगर (Error E = sys::fs::readNativeFileToEOF(FD, बफर))
    वापस errorToErrorCode(std::move(E));
  वापस get_Mem_बफर_Copy_Impl(बफर, बफर_Name);
}

ErrorOr<std::unique_ptr<Memory_बफर>>
Memory_बफर::getFile(स्थिर Twine &Filename, bool IsText,
                      bool RequiresNullTerminator, bool IsVolatile,
                      std::optional<Align> Alignment) {
  sys::sandbox::violationIfEnabled();

  वापस getFileAux<Memory_बफर>(Filename, /*MapSize=*/-1, /*Offset=*/0,
                                  IsText, RequiresNullTerminator, IsVolatile,
                                  Alignment);
}

ढाँचा <typename MB>
अटल ErrorOr<std::unique_ptr<MB>>
getOpenFileImpl(sys::fs::file_t FD, स्थिर Twine &Filename, uint64_t File_आकार,
                uint64_t Map_आकार, int64_t Offset, bool RequiresNullTerminator,
                bool IsVolatile, std::optional<Align> Alignment);

ढाँचा <typename MB>
अटल ErrorOr<std::unique_ptr<MB>>
getFileAux(स्थिर Twine &Filename, uint64_t Map_आकार, uint64_t Offset,
           bool IsText, bool RequiresNullTerminator, bool IsVolatile,
           std::optional<Align> Alignment) {
  Expected<sys::fs::file_t> FDOrErr = sys::fs::openNativeFileForRead(
      Filename, IsText ? sys::fs::OF_TextWithCRLF : sys::fs::OF_None);
  अगर (!FDOrErr)
    वापस errorToErrorCode(FDOrErr.takeError());
  sys::fs::file_t FD = *FDOrErr;
  स्वतः Ret = getOpenFileImpl<MB>(FD, Filename, /*FileSize=*/-1, Map_आकार, Offset,
                                 RequiresNullTerminator, IsVolatile, Alignment);
  sys::fs::closeFile(FD);
  वापस Ret;
}

ErrorOr<std::unique_ptr<Writable_Memory_बफर>>
Writable_Memory_बफर::getFile(स्थिर Twine &Filename, bool IsVolatile,
                              std::optional<Align> Alignment) {
  sys::sandbox::violationIfEnabled();

  वापस getFileAux<Writable_Memory_बफर>(
      Filename, /*MapSize=*/-1, /*Offset=*/0, /*IsText=*/false,
      /*RequiresNullTerminator=*/false, IsVolatile, Alignment);
}

ErrorOr<std::unique_ptr<Writable_Memory_बफर>>
Writable_Memory_बफर::getFileSlice(स्थिर Twine &Filename, uint64_t Map_आकार,
                                   uint64_t Offset, bool IsVolatile,
                                   std::optional<Align> Alignment) {
  sys::sandbox::violationIfEnabled();

  वापस getFileAux<Writable_Memory_बफर>(
      Filename, Map_आकार, Offset, /*IsText=*/false,
      /*RequiresNullTerminator=*/false, IsVolatile, Alignment);
}

std::unique_ptr<Writable_Memory_बफर>
Writable_Memory_बफर::get_New_Uninit_Mem_बफर(माप_प्रकार आकार,
                                            स्थिर Twine &बफर_Name,
                                            std::optional<Align> Alignment) {
  using Mem_बफर = Memory_बफर_Mem<Writable_Memory_बफर>;

  // Use 16-byte alignment if no alignment is specified.
  Align BufAlign = Alignment.मूल्य_or(Align(16));

  // Allocate space for the MemoryBuffer, the data and the name. It is important
  // that MemoryBuffer and data are aligned so PointerIntPair works with them.
  SmallString<256> NameBuf;
  StringRef NameRef = बफर_Name.toStringRef(NameBuf);

  माप_प्रकार StringLen = माप(Mem_बफर) + माप(माप_प्रकार) + NameRef.आकार_2() + 1;
  माप_प्रकार RealLen = StringLen + आकार + 1 + BufAlign.मूल्य_2();
  अगर (RealLen <= आकार) // Check for rollover.
    वापस nullptr;
  // We use a call to malloc() rather than a call to a non-throwing operator
  // new() because LLVM unconditionally installs an out of memory new handler
  // when exceptions are disabled. This new handler intentionally crashes to
  // aid with debugging, but that makes non-throwing new calls unhelpful.
  // See MemoryBufferMem::operator delete() for the paired call to free(), and
  // llvm::install_out_of_memory_new_handler() for the installation of the
  // custom new handler.
  अक्षर *Mem = static_cast<अक्षर *>(std::दो_स्मृति(RealLen));
  अगर (!Mem)
    वापस nullptr;

  // The name is stored after the class itself.
  *reinterpret_cast<माप_प्रकार *>(Mem + माप(Mem_बफर)) = NameRef.आकार_2();
  CopyStringRef(Mem + माप(Mem_बफर) + माप(माप_प्रकार), NameRef);

  // The buffer begins after the name and must be aligned.
  अक्षर *Buf = (अक्षर *)alignAddr(Mem + StringLen, BufAlign);
  Buf[आकार] = 0; // Null terminate buffer.

  स्वतः *Ret = नया (Mem) Mem_बफर(StringRef(Buf, आकार), true);
  वापस std::unique_ptr<Writable_Memory_बफर>(Ret);
}

std::unique_ptr<Writable_Memory_बफर>
Writable_Memory_बफर::get_New_Mem_बफर(माप_प्रकार आकार, स्थिर Twine &बफर_Name) {
  स्वतः SB = Writable_Memory_बफर::get_New_Uninit_Mem_बफर(आकार, बफर_Name);
  अगर (!SB)
    वापस nullptr;
  स_रखो(SB->get_बफर_Start(), 0, आकार);
  वापस SB;
}

अटल bool shouldUseMmap(sys::fs::file_t FD,
                          माप_प्रकार File_आकार,
                          माप_प्रकार Map_आकार,
                          off_t Offset,
                          bool RequiresNullTerminator,
                          पूर्णांक Page_आकार,
                          bool IsVolatile) {
#अगर defined(__MVS__)
  // zOS Enhanced ASCII auto convert does not support mmap.
  वापस false;
#पूर्ण_अगर

  // mmap may leave the buffer without null terminator if the file size changed
  // by the time the last page is mapped in, so avoid it if the file size is
  // likely to change.
  अगर (IsVolatile && RequiresNullTerminator)
    वापस false;

  // We don't use mmap for small files because this can severely fragment our
  // address space.
  अगर (Map_आकार < 4 * 4096 || Map_आकार < (अचिन्हित)Page_आकार)
    वापस false;

  अगर (!RequiresNullTerminator)
    वापस true;

  // If we don't know the file size, use fstat to find out.  fstat on an open
  // file descriptor is cheaper than stat on a random path.
  // FIXME: this chunk of code is duplicated, but it avoids a fstat when
  // RequiresNullTerminator = false and MapSize != -1.
  अगर (File_आकार == माप_प्रकार(-1)) {
    sys::fs::file_status Status;
    अगर (sys::fs::status(FD, Status))
      वापस false;
    File_आकार = Status.get_आकार();
  }

  // If we need a null terminator and the end of the map is inside the file,
  // we cannot use mmap.
  माप_प्रकार End = Offset + Map_आकार;
  निश्चित(End <= File_आकार);
  अगर (End != File_आकार)
    वापस false;

  // Don't try to map files that are exactly a multiple of the system page size
  // if we need a null terminator.
  अगर ((File_आकार & (Page_आकार -1)) == 0)
    वापस false;

#अगर defined(__CYGWIN__)
  // Don't try to map files that are exactly a multiple of the physical page size
  // if we need a null terminator.
  // FIXME: We should reorganize again getPageSize() on Win32.
  अगर ((File_आकार & (4096 - 1)) == 0)
    वापस false;
#पूर्ण_अगर

  वापस true;
}

अटल ErrorOr<std::unique_ptr<Write_Through_Memory_बफर>>
getReadWriteFile(स्थिर Twine &Filename, uint64_t File_आकार, uint64_t Map_आकार,
                 uint64_t Offset) {
  Expected<sys::fs::file_t> FDOrErr = sys::fs::openNativeFileForReadWrite(
      Filename, sys::fs::CD_OpenExisting, sys::fs::OF_None);
  अगर (!FDOrErr)
    वापस errorToErrorCode(FDOrErr.takeError());
  sys::fs::file_t FD = *FDOrErr;

  // Default is to map the full file.
  अगर (Map_आकार == uint64_t(-1)) {
    // If we don't know the file size, use fstat to find out.  fstat on an open
    // file descriptor is cheaper than stat on a random path.
    अगर (File_आकार == uint64_t(-1)) {
      sys::fs::file_status Status;
      std::error_code EC = sys::fs::status(FD, Status);
      अगर (EC)
        वापस EC;

      // If this not a file or a block device (e.g. it's a named pipe
      // or character device), we can't mmap it, so error out.
      sys::fs::file_type Type = Status.type();
      अगर (Type != sys::fs::file_type::regular_file &&
          Type != sys::fs::file_type::block_file)
        वापस make_error_code(errc::invalid_argument);

      File_आकार = Status.get_आकार();
    }
    Map_आकार = File_आकार;
  }

  std::error_code EC;
  std::unique_ptr<Write_Through_Memory_बफर> परिणाम(
      नया (Named_बफर_Alloc(Filename))
          Memory_बफर_MMap_File<Write_Through_Memory_बफर>(false, FD, Map_आकार,
                                                         Offset, EC));
  अगर (EC)
    वापस EC;
  वापस std::move(परिणाम);
}

ErrorOr<std::unique_ptr<Write_Through_Memory_बफर>>
Write_Through_Memory_बफर::getFile(स्थिर Twine &Filename, int64_t File_आकार) {
  sys::sandbox::violationIfEnabled();

  वापस getReadWriteFile(Filename, File_आकार, File_आकार, 0);
}

/// Map a subrange of the specified file as a WritableMemoryBuffer.
ErrorOr<std::unique_ptr<Write_Through_Memory_बफर>>
Write_Through_Memory_बफर::getFileSlice(स्थिर Twine &Filename, uint64_t Map_आकार,
                                       uint64_t Offset) {
  sys::sandbox::violationIfEnabled();

  वापस getReadWriteFile(Filename, -1, Map_आकार, Offset);
}

ढाँचा <typename MB>
अटल ErrorOr<std::unique_ptr<MB>>
getOpenFileImpl(sys::fs::file_t FD, स्थिर Twine &Filename, uint64_t File_आकार,
                uint64_t Map_आकार, int64_t Offset, bool RequiresNullTerminator,
                bool IsVolatile, std::optional<Align> Alignment) {
  अटल पूर्णांक Page_आकार = sys::Process::get_Page_आकार_Estimate();

  // Default is to map the full file.
  अगर (Map_आकार == uint64_t(-1)) {
    // If we don't know the file size, use fstat to find out.  fstat on an open
    // file descriptor is cheaper than stat on a random path.
    अगर (File_आकार == uint64_t(-1)) {
      sys::fs::file_status Status;
      std::error_code EC = sys::fs::status(FD, Status);
      अगर (EC)
        वापस EC;

      // If this not a file or a block device (e.g. it's a named pipe
      // or character device), we can't trust the size. Create the memory
      // buffer by copying off the stream.
      sys::fs::file_type Type = Status.type();
      अगर (Type != sys::fs::file_type::regular_file &&
          Type != sys::fs::file_type::block_file)
        वापस get_Memory_बफर_For_Stream(FD, Filename);

      File_आकार = Status.get_आकार();
    }
    Map_आकार = File_आकार;
  }

  अगर (shouldUseMmap(FD, File_आकार, Map_आकार, Offset, RequiresNullTerminator,
                    Page_आकार, IsVolatile)) {
    std::error_code EC;
    std::unique_ptr<MB> परिणाम(
        नया (Named_बफर_Alloc(Filename)) Memory_बफर_MMap_File<MB>(
            RequiresNullTerminator, FD, Map_आकार, Offset, EC));
    अगर (!EC) {
      // On at least Linux, and possibly on other systems, mmap may return pages
      // from the page cache that are not properly filled with trailing zeroes,
      // if some prior user of the page wrote non-zero bytes. Detect this and
      // don't use mmap in that case.
      अगर (!RequiresNullTerminator || *परिणाम->get_बफर_End() == '\0')
        वापस std::move(परिणाम);
    }
  }

#अगर_घोषित __MVS__
  ErrorOr<bool> NeedsConversion = needConversion(Filename, FD);
  अगर (std::error_code EC = NeedsConversion.getError())
    वापस EC;
  // File size may increase due to EBCDIC -> UTF-8 conversion, therefore we
  // cannot trust the file size and we create the memory buffer by copying
  // off the stream.
  // Note: This only works with the assumption of reading a full file (i.e,
  // Offset == 0 and MapSize == FileSize). Reading a file slice does not work.
  अगर (*NeedsConversion && Offset == 0 && Map_आकार == File_आकार)
    वापस get_Memory_बफर_For_Stream(FD, Filename);
#पूर्ण_अगर

  स्वतः Buf =
      Writable_Memory_बफर::get_New_Uninit_Mem_बफर(Map_आकार, Filename, Alignment);
  अगर (!Buf) {
    // Failed to create a buffer. The only way it can fail is if
    // new(std::nothrow) returns 0.
    वापस make_error_code(errc::not_enough_memory);
  }

  // Read until EOF, zero-initialize the rest.
  MutableArrayRef<अक्षर> ToRead = Buf->get_बफर();
  जबतक (!ToRead.empty()) {
    Expected<माप_प्रकार> ReadBytes =
        sys::fs::readNativeFileSlice(FD, ToRead, Offset);
    अगर (!ReadBytes)
      वापस errorToErrorCode(ReadBytes.takeError());
    अगर (*ReadBytes == 0) {
      std::स_रखो(ToRead.तथ्य(), 0, ToRead.आकार_2());
      अवरोध;
    }
    ToRead = ToRead.drop_front(*ReadBytes);
    Offset += *ReadBytes;
  }

  वापस std::move(Buf);
}

ErrorOr<std::unique_ptr<Memory_बफर>>
Memory_बफर::getOpenFile(sys::fs::file_t FD, स्थिर Twine &Filename,
                          uint64_t File_आकार, bool RequiresNullTerminator,
                          bool IsVolatile, std::optional<Align> Alignment) {
  sys::sandbox::violationIfEnabled();

  वापस getOpenFileImpl<Memory_बफर>(FD, Filename, File_आकार, File_आकार, 0,
                                       RequiresNullTerminator, IsVolatile,
                                       Alignment);
}

ErrorOr<std::unique_ptr<Memory_बफर>> Memory_बफर::getOpenFileSlice(
    sys::fs::file_t FD, स्थिर Twine &Filename, uint64_t Map_आकार, int64_t Offset,
    bool IsVolatile, std::optional<Align> Alignment) {
  निश्चित(Map_आकार != uint64_t(-1));

  sys::sandbox::violationIfEnabled();

  वापस getOpenFileImpl<Memory_बफर>(FD, Filename, -1, Map_आकार, Offset, false,
                                       IsVolatile, Alignment);
}

ErrorOr<std::unique_ptr<Memory_बफर>> Memory_बफर::getSTDIN() {
  sys::sandbox::violationIfEnabled();

  // Read in all of the data from stdin, we cannot mmap stdin.
  //
  // FIXME: That isn't necessarily true, we should try to mmap stdin and
  // fallback if it fails.
  sys::ChangeStdinMode(sys::fs::OF_Text);

  वापस get_Memory_बफर_For_Stream(sys::fs::getStdinHandle(), "<stdin>");
}

ErrorOr<std::unique_ptr<Memory_बफर>>
Memory_बफर::getFileAsStream(स्थिर Twine &Filename) {
  sys::sandbox::violationIfEnabled();

  Expected<sys::fs::file_t> FDOrErr =
      sys::fs::openNativeFileForRead(Filename, sys::fs::OF_None);
  अगर (!FDOrErr)
    वापस errorToErrorCode(FDOrErr.takeError());
  sys::fs::file_t FD = *FDOrErr;
  ErrorOr<std::unique_ptr<Memory_बफर>> Ret =
      get_Memory_बफर_For_Stream(FD, Filename);
  sys::fs::closeFile(FD);
  वापस Ret;
}

Memory_बफर_Ref Memory_बफर::get_Mem_बफर_Ref() स्थिर {
  StringRef तथ्य_2 = get_बफर();
  StringRef Identifier = get_बफर_Identifier();
  वापस Memory_बफर_Ref(तथ्य_2, Identifier);
}

Small_Vector_Memory_बफर::~Small_Vector_Memory_बफर() = शेष;
