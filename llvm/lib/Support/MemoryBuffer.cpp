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
#यदि_अन्तः

उपयुज्य नामाकाश llvm;

//===----------------------------------------------------------------------===//
// MemoryBuffer implementation itself.
//===----------------------------------------------------------------------===//

स्मृतिः_बफर::~स्मृतिः_बफर() = स्वभाव;

/// init - Initialize this MemoryBuffer as a reference to externally allocated
/// memory, memory that we know is already null terminated.
शून्य स्मृतिः_बफर::init(ध्रुव वर्ण *BufStart, ध्रुव वर्ण *Buf_अन्तः,
                        तार्किक RequiresNullTerminator) {
  प्रतिज्ञा((!RequiresNullTerminator || Buf_अन्तः[0] == 0) &&
         "Buffer is not null terminated!");
  बफर_Start = BufStart;
  बफर_अन्तः = Buf_अन्तः;
}

//===----------------------------------------------------------------------===//
// MemoryBufferMem implementation.
//===----------------------------------------------------------------------===//

/// CopyStringRef - Copies contents of a StringRef into a block of memory and
/// null-terminates it.
स्थिर शून्य CopyStringRef(वर्ण *स्मृतिः_2, StringRef दत्त_2) {
  यदि (!दत्त_2.रिक्तानि())
    स्मृतिं_प्रतिलिख(स्मृतिः_2, दत्त_2.दत्त(), दत्त_2.आकार_2());
  स्मृतिः_2[दत्त_2.आकार_2()] = 0; // Null terminate string.
}

नामाकाश {
संरचना Named_बफर_Alloc {
  ध्रुव Twine &Name;
  Named_बफर_Alloc(ध्रुव Twine &Name) : Name(Name) {}
};
} // namespace

शून्य *संक्रियकः नूतन(size_t N, ध्रुव Named_बफर_Alloc &Alloc) {
  SmallString<256> NameBuf;
  StringRef NameRef = Alloc.Name.गन्तव्यम्_String_Ref(NameBuf);

  // We use malloc() and manually handle it returning null instead of calling
  // operator new because we need all uses of NamedBufferAlloc to be
  // deallocated with a call to free() due to needing to use malloc() in
  // WritableMemoryBuffer::getNewUninitMemBuffer() to work around the out-of-
  // memory handler installed by default in LLVM. See operator delete() member
  // functions within this file for the paired call to free().
  वर्ण *Mem =
      static_cast<वर्ण *>(std::स्मृतिं_देहि(N + परिमाण(size_t) + NameRef.आकार_2() + 1));
  यदि (!Mem)
    llvm::report_bad_alloc_error("Allocation failed");
  *reinterpret_cast<size_t *>(Mem + N) = NameRef.आकार_2();
  CopyStringRef(Mem + N + परिमाण(size_t), NameRef);
  प्रत्यागम Mem;
}

नामाकाश {
/// MemoryBufferMem - Named MemoryBuffer pointing to a block of memory.
प्रतिरूपम्<प्रकारनाम MB>
वर्ग स्मृतिः_बफर_Mem : सार्वजनिक MB {
सार्वजनिक:
  स्मृतिः_बफर_Mem(StringRef निवेशः_दत्त, तार्किक RequiresNullTerminator) {
    स्मृतिः_बफर::init(निवेशः_दत्त.begin(), निवेशः_दत्त.अन्तः(),
                       RequiresNullTerminator);
  }

  /// Disable sized deallocation for MemoryBufferMem, because it has
  /// tail-allocated data.
  शून्य संक्रियकः विलोपय(शून्य *p) { std::मुक्त(p); }

  StringRef get_बफर_Identifier() ध्रुव override {
    // The name is stored after the class itself.
    प्रत्यागम StringRef(reinterpret_cast<ध्रुव वर्ण *>(एतत् + 1) + परिमाण(size_t),
                     *reinterpret_cast<ध्रुव size_t *>(एतत् + 1));
  }

  स्मृतिः_बफर::बफर_विधा get_बफर_विधा() ध्रुव override {
    प्रत्यागम स्मृतिः_बफर::स्मृतिः_बफर_Malloc;
  }
};
} // namespace

प्रतिरूपम् <प्रकारनाम MB>
स्थिर ErrorOr<std::unique_ptr<MB>>
getFileAux(ध्रुव Twine &Filename, uint64_t Map_आकार, uint64_t विस्थापनम्,
           तार्किक IsText, तार्किक RequiresNullTerminator, तार्किक IsVolatile,
           std::optional<Align> Alignment);

std::unique_ptr<स्मृतिः_बफर>
स्मृतिः_बफर::get_Mem_बफर(StringRef निवेशः_दत्त, StringRef बफर_Name,
                           तार्किक RequiresNullTerminator) {
  स्वचालित *Ret = नूतन (Named_बफर_Alloc(बफर_Name))
      स्मृतिः_बफर_Mem<स्मृतिः_बफर>(निवेशः_दत्त, RequiresNullTerminator);
  प्रत्यागम std::unique_ptr<स्मृतिः_बफर>(Ret);
}

std::unique_ptr<स्मृतिः_बफर>
स्मृतिः_बफर::get_Mem_बफर(स्मृतिः_बफर_Ref Ref, तार्किक RequiresNullTerminator) {
  प्रत्यागम std::unique_ptr<स्मृतिः_बफर>(get_Mem_बफर(
      Ref.get_बफर(), Ref.get_बफर_Identifier(), RequiresNullTerminator));
}

स्थिर ErrorOr<std::unique_ptr<Writable_स्मृतिः_बफर>>
get_Mem_बफर_Copy_Impl(StringRef निवेशः_दत्त, ध्रुव Twine &बफर_Name) {
  स्वचालित Buf =
      Writable_स्मृतिः_बफर::get_New_Uninit_Mem_बफर(निवेशः_दत्त.आकार_2(), बफर_Name);
  यदि (!Buf)
    प्रत्यागम make_error_संहिता(errc::not_enough_स्मृतिः);
  // Calling memcpy with null src/dst is UB, and an empty StringRef is
  // represented with {nullptr, 0}.
  llvm::copy(निवेशः_दत्त, Buf->get_बफर_Start());
  प्रत्यागम std::move(Buf);
}

std::unique_ptr<स्मृतिः_बफर>
स्मृतिः_बफर::get_Mem_बफर_Copy(StringRef निवेशः_दत्त, ध्रुव Twine &बफर_Name) {
  स्वचालित Buf = get_Mem_बफर_Copy_Impl(निवेशः_दत्त, बफर_Name);
  यदि (Buf)
    प्रत्यागम std::move(*Buf);
  प्रत्यागम शून्यसूचकः;
}

ErrorOr<std::unique_ptr<स्मृतिः_बफर>>
स्मृतिः_बफर::getFileOrSTDIN(ध्रुव Twine &Filename, तार्किक IsText,
                             तार्किक RequiresNullTerminator,
                             std::optional<Align> Alignment) {
  sys::sandbox::violationIfEnabled();

  SmallString<256> NameBuf;
  StringRef NameRef = Filename.गन्तव्यम्_String_Ref(NameBuf);

  यदि (NameRef == "-")
    प्रत्यागम getSTDIN();
  प्रत्यागम getFile(Filename, IsText, RequiresNullTerminator,
                 /*IsVolatile=*/असत्य, Alignment);
}

ErrorOr<std::unique_ptr<स्मृतिः_बफर>>
स्मृतिः_बफर::get_File_खण्डिका(ध्रुव Twine &File_मार्गः, uint64_t Map_आकार,
                           uint64_t विस्थापनम्, तार्किक IsVolatile,
                           std::optional<Align> Alignment) {
  sys::sandbox::violationIfEnabled();

  प्रत्यागम getFileAux<स्मृतिः_बफर>(File_मार्गः, Map_आकार, विस्थापनम्, /*IsText=*/असत्य,
                                  /*RequiresNullTerminator=*/असत्य, IsVolatile,
                                  Alignment);
}

//===----------------------------------------------------------------------===//
// MemoryBuffer::getFile implementation.
//===----------------------------------------------------------------------===//

नामाकाश {

प्रतिरूपम् <प्रकारनाम MB>
constexpr sys::fs::mapped_file_region::mapmode Mapmode =
    sys::fs::mapped_file_region::readonly;
प्रतिरूपम् <>
constexpr sys::fs::mapped_file_region::mapmode Mapmode<स्मृतिः_बफर> =
    sys::fs::mapped_file_region::readonly;
प्रतिरूपम् <>
constexpr sys::fs::mapped_file_region::mapmode Mapmode<Writable_स्मृतिः_बफर> =
    sys::fs::mapped_file_region::priv;
प्रतिरूपम् <>
constexpr sys::fs::mapped_file_region::mapmode
    Mapmode<Write_Through_स्मृतिः_बफर> = sys::fs::mapped_file_region::readwrite;

/// Memory maps a file descriptor using sys::fs::mapped_file_region.
///
/// This handles converting the offset into a legal offset on the platform.
प्रतिरूपम्<प्रकारनाम MB>
वर्ग स्मृतिः_बफर_MMap_File : सार्वजनिक MB {
  sys::fs::mapped_file_region MFR;

  स्थिर uint64_t get_Legal_Map_विस्थापनम्(uint64_t विस्थापनम्) {
    प्रत्यागम विस्थापनम् & ~(sys::fs::mapped_file_region::alignment() - 1);
  }

  स्थिर uint64_t get_Legal_Map_आकार(uint64_t Len, uint64_t विस्थापनम्) {
    प्रत्यागम Len + (विस्थापनम् - get_Legal_Map_विस्थापनम्(विस्थापनम्));
  }

  ध्रुव वर्ण *getStart(uint64_t Len, uint64_t विस्थापनम्) {
    प्रत्यागम MFR.const_दत्त() + (विस्थापनम् - get_Legal_Map_विस्थापनम्(विस्थापनम्));
  }

सार्वजनिक:
  स्मृतिः_बफर_MMap_File(तार्किक RequiresNullTerminator, sys::fs::file_t FD, uint64_t Len,
                       uint64_t विस्थापनम्, std::error_संहिता &EC)
      : MFR(FD, Mapmode<MB>, get_Legal_Map_आकार(Len, विस्थापनम्),
            get_Legal_Map_विस्थापनम्(विस्थापनम्), EC) {
    यदि (!EC) {
      ध्रुव वर्ण *Start = getStart(Len, विस्थापनम्);
      स्मृतिः_बफर::init(Start, Start + Len, RequiresNullTerminator);
    }
  }

  /// Disable sized deallocation for MemoryBufferMMapFile, because it has
  /// tail-allocated data.
  शून्य संक्रियकः विलोपय(शून्य *p) { std::मुक्त(p); }

  StringRef get_बफर_Identifier() ध्रुव override {
    // The name is stored after the class itself.
    प्रत्यागम StringRef(reinterpret_cast<ध्रुव वर्ण *>(एतत् + 1) + परिमाण(size_t),
                     *reinterpret_cast<ध्रुव size_t *>(एतत् + 1));
  }

  स्मृतिः_बफर::बफर_विधा get_बफर_विधा() ध्रुव override {
    प्रत्यागम स्मृतिः_बफर::स्मृतिः_बफर_MMap;
  }

  शून्य dontNeedIfMmap() override { MFR.dontNeed(); }
  शून्य willNeedIfMmap() override { MFR.willNeed(); }
  शून्य randomAccessIfMmap() override { MFR.randomAccess(); }
};
} // namespace

स्थिर ErrorOr<std::unique_ptr<Writable_स्मृतिः_बफर>>
get_स्मृतिः_बफर_For_प्रवाहः(sys::fs::file_t FD, ध्रुव Twine &बफर_Name) {
  SmallString<sys::fs::Default_Read_Chunk_आकार> बफर;
  यदि (Error E = sys::fs::read_Native_File_गन्तव्यम्_EOF(FD, बफर))
    प्रत्यागम error_गन्तव्यम्_Error_संहिता(std::move(E));
  प्रत्यागम get_Mem_बफर_Copy_Impl(बफर, बफर_Name);
}

ErrorOr<std::unique_ptr<स्मृतिः_बफर>>
स्मृतिः_बफर::getFile(ध्रुव Twine &Filename, तार्किक IsText,
                      तार्किक RequiresNullTerminator, तार्किक IsVolatile,
                      std::optional<Align> Alignment) {
  sys::sandbox::violationIfEnabled();

  प्रत्यागम getFileAux<स्मृतिः_बफर>(Filename, /*MapSize=*/-1, /*Offset=*/0,
                                  IsText, RequiresNullTerminator, IsVolatile,
                                  Alignment);
}

प्रतिरूपम् <प्रकारनाम MB>
स्थिर ErrorOr<std::unique_ptr<MB>>
getOpenFileImpl(sys::fs::file_t FD, ध्रुव Twine &Filename, uint64_t File_आकार,
                uint64_t Map_आकार, int64_t विस्थापनम्, तार्किक RequiresNullTerminator,
                तार्किक IsVolatile, std::optional<Align> Alignment);

प्रतिरूपम् <प्रकारनाम MB>
स्थिर ErrorOr<std::unique_ptr<MB>>
getFileAux(ध्रुव Twine &Filename, uint64_t Map_आकार, uint64_t विस्थापनम्,
           तार्किक IsText, तार्किक RequiresNullTerminator, तार्किक IsVolatile,
           std::optional<Align> Alignment) {
  Expected<sys::fs::file_t> FDOrErr = sys::fs::openNativeFileForRead(
      Filename, IsText ? sys::fs::सम्बन्धि_Text_With_CRLF : sys::fs::सम्बन्धि_None);
  यदि (!FDOrErr)
    प्रत्यागम error_गन्तव्यम्_Error_संहिता(FDOrErr.takeError());
  sys::fs::file_t FD = *FDOrErr;
  स्वचालित Ret = getOpenFileImpl<MB>(FD, Filename, /*FileSize=*/-1, Map_आकार, विस्थापनम्,
                                 RequiresNullTerminator, IsVolatile, Alignment);
  sys::fs::closeFile(FD);
  प्रत्यागम Ret;
}

ErrorOr<std::unique_ptr<Writable_स्मृतिः_बफर>>
Writable_स्मृतिः_बफर::getFile(ध्रुव Twine &Filename, तार्किक IsVolatile,
                              std::optional<Align> Alignment) {
  sys::sandbox::violationIfEnabled();

  प्रत्यागम getFileAux<Writable_स्मृतिः_बफर>(
      Filename, /*MapSize=*/-1, /*Offset=*/0, /*IsText=*/असत्य,
      /*RequiresNullTerminator=*/असत्य, IsVolatile, Alignment);
}

ErrorOr<std::unique_ptr<Writable_स्मृतिः_बफर>>
Writable_स्मृतिः_बफर::get_File_खण्डिका(ध्रुव Twine &Filename, uint64_t Map_आकार,
                                   uint64_t विस्थापनम्, तार्किक IsVolatile,
                                   std::optional<Align> Alignment) {
  sys::sandbox::violationIfEnabled();

  प्रत्यागम getFileAux<Writable_स्मृतिः_बफर>(
      Filename, Map_आकार, विस्थापनम्, /*IsText=*/असत्य,
      /*RequiresNullTerminator=*/असत्य, IsVolatile, Alignment);
}

std::unique_ptr<Writable_स्मृतिः_बफर>
Writable_स्मृतिः_बफर::get_New_Uninit_Mem_बफर(size_t आकार,
                                            ध्रुव Twine &बफर_Name,
                                            std::optional<Align> Alignment) {
  उपयुज्य Mem_बफर = स्मृतिः_बफर_Mem<Writable_स्मृतिः_बफर>;

  // Use 16-byte alignment if no alignment is specified.
  Align BufAlign = Alignment.मानम्_or(Align(16));

  // Allocate space for the MemoryBuffer, the data and the name. It is important
  // that MemoryBuffer and data are aligned so PointerIntPair works with them.
  SmallString<256> NameBuf;
  StringRef NameRef = बफर_Name.गन्तव्यम्_String_Ref(NameBuf);

  size_t StringLen = परिमाण(Mem_बफर) + परिमाण(size_t) + NameRef.आकार_2() + 1;
  size_t RealLen = StringLen + आकार + 1 + BufAlign.मानम्_2();
  यदि (RealLen <= आकार) // Check for rollover.
    प्रत्यागम शून्यसूचकः;
  // We use a call to malloc() rather than a call to a non-throwing operator
  // new() because LLVM unconditionally installs an out of memory new handler
  // when exceptions are disabled. This new handler intentionally crashes to
  // aid with debugging, but that makes non-throwing new calls unhelpful.
  // See MemoryBufferMem::operator delete() for the paired call to free(), and
  // llvm::install_out_of_memory_new_handler() for the installation of the
  // custom new handler.
  वर्ण *Mem = static_cast<वर्ण *>(std::स्मृतिं_देहि(RealLen));
  यदि (!Mem)
    प्रत्यागम शून्यसूचकः;

  // The name is stored after the class itself.
  *reinterpret_cast<size_t *>(Mem + परिमाण(Mem_बफर)) = NameRef.आकार_2();
  CopyStringRef(Mem + परिमाण(Mem_बफर) + परिमाण(size_t), NameRef);

  // The buffer begins after the name and must be aligned.
  वर्ण *Buf = (वर्ण *)alignAddr(Mem + StringLen, BufAlign);
  Buf[आकार] = 0; // Null terminate buffer.

  स्वचालित *Ret = नूतन (Mem) Mem_बफर(StringRef(Buf, आकार), सत्य);
  प्रत्यागम std::unique_ptr<Writable_स्मृतिः_बफर>(Ret);
}

std::unique_ptr<Writable_स्मृतिः_बफर>
Writable_स्मृतिः_बफर::get_New_Mem_बफर(size_t आकार, ध्रुव Twine &बफर_Name) {
  स्वचालित SB = Writable_स्मृतिः_बफर::get_New_Uninit_Mem_बफर(आकार, बफर_Name);
  यदि (!SB)
    प्रत्यागम शून्यसूचकः;
  स्मृतिं_पूरय(SB->get_बफर_Start(), 0, आकार);
  प्रत्यागम SB;
}

स्थिर तार्किक shouldUseMmap(sys::fs::file_t FD,
                          size_t File_आकार,
                          size_t Map_आकार,
                          off_t विस्थापनम्,
                          तार्किक RequiresNullTerminator,
                          पूर्णाङ्क Page_आकार,
                          तार्किक IsVolatile) {
#यदि defined(__MVS__)
  // zOS Enhanced ASCII auto convert does not support mmap.
  प्रत्यागम असत्य;
#यदि_अन्तः

  // mmap may leave the buffer without null terminator if the file size changed
  // by the time the last page is mapped in, so avoid it if the file size is
  // likely to change.
  यदि (IsVolatile && RequiresNullTerminator)
    प्रत्यागम असत्य;

  // We don't use mmap for small files because this can severely fragment our
  // address space.
  यदि (Map_आकार < 4 * 4096 || Map_आकार < (अचिह्नित)Page_आकार)
    प्रत्यागम असत्य;

  यदि (!RequiresNullTerminator)
    प्रत्यागम सत्य;

  // If we don't know the file size, use fstat to find out.  fstat on an open
  // file descriptor is cheaper than stat on a random path.
  // FIXME: this chunk of code is duplicated, but it avoids a fstat when
  // RequiresNullTerminator = false and MapSize != -1.
  यदि (File_आकार == size_t(-1)) {
    sys::fs::file_स्थितिः स्थितिः;
    यदि (sys::fs::स्थितिः_2(FD, स्थितिः))
      प्रत्यागम असत्य;
    File_आकार = स्थितिः.get_आकार();
  }

  // If we need a null terminator and the end of the map is inside the file,
  // we cannot use mmap.
  size_t अन्तः_2 = विस्थापनम् + Map_आकार;
  प्रतिज्ञा(अन्तः_2 <= File_आकार);
  यदि (अन्तः_2 != File_आकार)
    प्रत्यागम असत्य;

  // Don't try to map files that are exactly a multiple of the system page size
  // if we need a null terminator.
  यदि ((File_आकार & (Page_आकार -1)) == 0)
    प्रत्यागम असत्य;

#यदि defined(__CYGWIN__)
  // Don't try to map files that are exactly a multiple of the physical page size
  // if we need a null terminator.
  // FIXME: We should reorganize again getPageSize() on Win32.
  यदि ((File_आकार & (4096 - 1)) == 0)
    प्रत्यागम असत्य;
#यदि_अन्तः

  प्रत्यागम सत्य;
}

स्थिर ErrorOr<std::unique_ptr<Write_Through_स्मृतिः_बफर>>
getReadWriteFile(ध्रुव Twine &Filename, uint64_t File_आकार, uint64_t Map_आकार,
                 uint64_t विस्थापनम्) {
  Expected<sys::fs::file_t> FDOrErr = sys::fs::openNativeFileForReadWrite(
      Filename, sys::fs::CD_OpenExisting, sys::fs::सम्बन्धि_None);
  यदि (!FDOrErr)
    प्रत्यागम error_गन्तव्यम्_Error_संहिता(FDOrErr.takeError());
  sys::fs::file_t FD = *FDOrErr;

  // Default is to map the full file.
  यदि (Map_आकार == uint64_t(-1)) {
    // If we don't know the file size, use fstat to find out.  fstat on an open
    // file descriptor is cheaper than stat on a random path.
    यदि (File_आकार == uint64_t(-1)) {
      sys::fs::file_स्थितिः स्थितिः;
      std::error_संहिता EC = sys::fs::स्थितिः_2(FD, स्थितिः);
      यदि (EC)
        प्रत्यागम EC;

      // If this not a file or a block device (e.g. it's a named pipe
      // or character device), we can't mmap it, so error out.
      sys::fs::file_type Type = स्थितिः.type();
      यदि (Type != sys::fs::file_type::regular_file &&
          Type != sys::fs::file_type::खण्डम्_file)
        प्रत्यागम make_error_संहिता(errc::invalid_तर्कः);

      File_आकार = स्थितिः.get_आकार();
    }
    Map_आकार = File_आकार;
  }

  std::error_संहिता EC;
  std::unique_ptr<Write_Through_स्मृतिः_बफर> परिणाम(
      नूतन (Named_बफर_Alloc(Filename))
          स्मृतिः_बफर_MMap_File<Write_Through_स्मृतिः_बफर>(असत्य, FD, Map_आकार,
                                                         विस्थापनम्, EC));
  यदि (EC)
    प्रत्यागम EC;
  प्रत्यागम std::move(परिणाम);
}

ErrorOr<std::unique_ptr<Write_Through_स्मृतिः_बफर>>
Write_Through_स्मृतिः_बफर::getFile(ध्रुव Twine &Filename, int64_t File_आकार) {
  sys::sandbox::violationIfEnabled();

  प्रत्यागम getReadWriteFile(Filename, File_आकार, File_आकार, 0);
}

/// Map a subrange of the specified file as a WritableMemoryBuffer.
ErrorOr<std::unique_ptr<Write_Through_स्मृतिः_बफर>>
Write_Through_स्मृतिः_बफर::get_File_खण्डिका(ध्रुव Twine &Filename, uint64_t Map_आकार,
                                       uint64_t विस्थापनम्) {
  sys::sandbox::violationIfEnabled();

  प्रत्यागम getReadWriteFile(Filename, -1, Map_आकार, विस्थापनम्);
}

प्रतिरूपम् <प्रकारनाम MB>
स्थिर ErrorOr<std::unique_ptr<MB>>
getOpenFileImpl(sys::fs::file_t FD, ध्रुव Twine &Filename, uint64_t File_आकार,
                uint64_t Map_आकार, int64_t विस्थापनम्, तार्किक RequiresNullTerminator,
                तार्किक IsVolatile, std::optional<Align> Alignment) {
  स्थिर पूर्णाङ्क Page_आकार = sys::Process::get_Page_आकार_Estimate();

  // Default is to map the full file.
  यदि (Map_आकार == uint64_t(-1)) {
    // If we don't know the file size, use fstat to find out.  fstat on an open
    // file descriptor is cheaper than stat on a random path.
    यदि (File_आकार == uint64_t(-1)) {
      sys::fs::file_स्थितिः स्थितिः;
      std::error_संहिता EC = sys::fs::स्थितिः_2(FD, स्थितिः);
      यदि (EC)
        प्रत्यागम EC;

      // If this not a file or a block device (e.g. it's a named pipe
      // or character device), we can't trust the size. Create the memory
      // buffer by copying off the stream.
      sys::fs::file_type Type = स्थितिः.type();
      यदि (Type != sys::fs::file_type::regular_file &&
          Type != sys::fs::file_type::खण्डम्_file)
        प्रत्यागम get_स्मृतिः_बफर_For_प्रवाहः(FD, Filename);

      File_आकार = स्थितिः.get_आकार();
    }
    Map_आकार = File_आकार;
  }

  यदि (shouldUseMmap(FD, File_आकार, Map_आकार, विस्थापनम्, RequiresNullTerminator,
                    Page_आकार, IsVolatile)) {
    std::error_संहिता EC;
    std::unique_ptr<MB> परिणाम(
        नूतन (Named_बफर_Alloc(Filename)) स्मृतिः_बफर_MMap_File<MB>(
            RequiresNullTerminator, FD, Map_आकार, विस्थापनम्, EC));
    यदि (!EC) {
      // On at least Linux, and possibly on other systems, mmap may return pages
      // from the page cache that are not properly filled with trailing zeroes,
      // if some prior user of the page wrote non-zero bytes. Detect this and
      // don't use mmap in that case.
      यदि (!RequiresNullTerminator || *परिणाम->get_बफर_अन्तः() == '\0')
        प्रत्यागम std::move(परिणाम);
    }
  }

#यदि_परिभाषितम् __MVS__
  ErrorOr<तार्किक> NeedsConversion = needConversion(Filename, FD);
  यदि (std::error_संहिता EC = NeedsConversion.getError())
    प्रत्यागम EC;
  // File size may increase due to EBCDIC -> UTF-8 conversion, therefore we
  // cannot trust the file size and we create the memory buffer by copying
  // off the stream.
  // Note: This only works with the assumption of reading a full file (i.e,
  // Offset == 0 and MapSize == FileSize). Reading a file slice does not work.
  यदि (*NeedsConversion && विस्थापनम् == 0 && Map_आकार == File_आकार)
    प्रत्यागम get_स्मृतिः_बफर_For_प्रवाहः(FD, Filename);
#यदि_अन्तः

  स्वचालित Buf =
      Writable_स्मृतिः_बफर::get_New_Uninit_Mem_बफर(Map_आकार, Filename, Alignment);
  यदि (!Buf) {
    // Failed to create a buffer. The only way it can fail is if
    // new(std::nothrow) returns 0.
    प्रत्यागम make_error_संहिता(errc::not_enough_स्मृतिः);
  }

  // Read until EOF, zero-initialize the rest.
  MutableArrayRef<वर्ण> गन्तव्यम्_Read = Buf->get_बफर();
  यावत् (!गन्तव्यम्_Read.रिक्तानि()) {
    Expected<size_t> Read_अष्टकानि =
        sys::fs::read_Native_File_खण्डिका(FD, गन्तव्यम्_Read, विस्थापनम्);
    यदि (!Read_अष्टकानि)
      प्रत्यागम error_गन्तव्यम्_Error_संहिता(Read_अष्टकानि.takeError());
    यदि (*Read_अष्टकानि == 0) {
      std::स्मृतिं_पूरय(गन्तव्यम्_Read.दत्त(), 0, गन्तव्यम्_Read.आकार_2());
      भङ्ग;
    }
    गन्तव्यम्_Read = गन्तव्यम्_Read.drop_front(*Read_अष्टकानि);
    विस्थापनम् += *Read_अष्टकानि;
  }

  प्रत्यागम std::move(Buf);
}

ErrorOr<std::unique_ptr<स्मृतिः_बफर>>
स्मृतिः_बफर::getOpenFile(sys::fs::file_t FD, ध्रुव Twine &Filename,
                          uint64_t File_आकार, तार्किक RequiresNullTerminator,
                          तार्किक IsVolatile, std::optional<Align> Alignment) {
  sys::sandbox::violationIfEnabled();

  प्रत्यागम getOpenFileImpl<स्मृतिः_बफर>(FD, Filename, File_आकार, File_आकार, 0,
                                       RequiresNullTerminator, IsVolatile,
                                       Alignment);
}

ErrorOr<std::unique_ptr<स्मृतिः_बफर>> स्मृतिः_बफर::get_Open_File_खण्डिका(
    sys::fs::file_t FD, ध्रुव Twine &Filename, uint64_t Map_आकार, int64_t विस्थापनम्,
    तार्किक IsVolatile, std::optional<Align> Alignment) {
  प्रतिज्ञा(Map_आकार != uint64_t(-1));

  sys::sandbox::violationIfEnabled();

  प्रत्यागम getOpenFileImpl<स्मृतिः_बफर>(FD, Filename, -1, Map_आकार, विस्थापनम्, असत्य,
                                       IsVolatile, Alignment);
}

ErrorOr<std::unique_ptr<स्मृतिः_बफर>> स्मृतिः_बफर::getSTDIN() {
  sys::sandbox::violationIfEnabled();

  // Read in all of the data from stdin, we cannot mmap stdin.
  //
  // FIXME: That isn't necessarily true, we should try to mmap stdin and
  // fallback if it fails.
  sys::Change_Stdin_बहुलकम्(sys::fs::सम्बन्धि_Text);

  प्रत्यागम get_स्मृतिः_बफर_For_प्रवाहः(sys::fs::getStdinHandle(), "<stdin>");
}

ErrorOr<std::unique_ptr<स्मृतिः_बफर>>
स्मृतिः_बफर::get_File_As_प्रवाहः(ध्रुव Twine &Filename) {
  sys::sandbox::violationIfEnabled();

  Expected<sys::fs::file_t> FDOrErr =
      sys::fs::openNativeFileForRead(Filename, sys::fs::सम्बन्धि_None);
  यदि (!FDOrErr)
    प्रत्यागम error_गन्तव्यम्_Error_संहिता(FDOrErr.takeError());
  sys::fs::file_t FD = *FDOrErr;
  ErrorOr<std::unique_ptr<स्मृतिः_बफर>> Ret =
      get_स्मृतिः_बफर_For_प्रवाहः(FD, Filename);
  sys::fs::closeFile(FD);
  प्रत्यागम Ret;
}

स्मृतिः_बफर_Ref स्मृतिः_बफर::get_Mem_बफर_Ref() ध्रुव {
  StringRef दत्त_2 = get_बफर();
  StringRef Identifier = get_बफर_Identifier();
  प्रत्यागम स्मृतिः_बफर_Ref(दत्त_2, Identifier);
}

Small_सदिशम्_स्मृतिः_बफर::~Small_सदिशम्_स्मृतिः_बफर() = स्वभाव;
