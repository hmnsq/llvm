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

#అన్తర్భావ "llvm/Support/MemoryBuffer.h"
#అన్తర్భావ "llvm/ADT/STLExtras.h"
#అన్తర్భావ "llvm/ADT/SmallString.h"
#అన్తర్భావ "llvm/Config/config.h"
#అన్తర్భావ "llvm/Support/Alignment.h"
#అన్తర్భావ "llvm/Support/AutoConvert.h"
#అన్తర్భావ "llvm/Support/Errc.h"
#అన్తర్భావ "llvm/Support/Error.h"
#అన్తర్భావ "llvm/Support/ErrorHandling.h"
#అన్తర్భావ "llvm/Support/FileSystem.h"
#అన్తర్భావ "llvm/Support/IOSandbox.h"
#అన్తర్భావ "llvm/Support/Process.h"
#అన్తర్భావ "llvm/Support/Program.h"
#అన్తర్భావ "llvm/Support/SmallVectorMemoryBuffer.h"
#అన్తర్భావ <algorithm>
#అన్తర్భావ <cassert>
#అన్తర్భావ <cstring>
#అన్తర్భావ <new>
#అన్తర్భావ <sys/types.h>
#అన్తర్భావ <system_error>
#ఒకవేళ !defined(_MSC_VER) && !defined(__MINGW32__)
#అన్తర్భావ <unistd.h>
#లేకపోతే
#అన్తర్భావ <io.h>
#అయితే_ముగింపు

ఉపయోగించి నామాకాశ llvm;

//===----------------------------------------------------------------------===//
// MemoryBuffer implementation itself.
//===----------------------------------------------------------------------===//

జ్ఞాపకం_బఫరు::~జ్ఞాపకం_బఫరు() = అప్రమేయ;

/// init - Initialize this MemoryBuffer as a reference to externally allocated
/// memory, memory that we know is already null terminated.
శూన్య జ్ఞాపకం_బఫరు::init(స్థిరరాశి అక్షరం *Buf_ప్రారంభం, స్థిరరాశి అక్షరం *Buf_చివర,
                        తార్కిక RequiresNullTerminator) {
  ప్రతిజ్ఞా((!RequiresNullTerminator || Buf_చివర[0] == 0) &&
         "Buffer is not null terminated!");
  బఫరు_ప్రారంభం = Buf_ప్రారంభం;
  బఫరు_చివర = Buf_చివర;
}

//===----------------------------------------------------------------------===//
// MemoryBufferMem implementation.
//===----------------------------------------------------------------------===//

/// CopyStringRef - Copies contents of a StringRef into a block of memory and
/// null-terminates it.
స్థిర శూన్య CopyStringRef(అక్షరం *జ్ఞాపకం_2, StringRef దత్తాంశం_2) {
  ఒకవేళ (!దత్తాంశం_2.ఖాళీలు())
    జ్ఞాపకం_నకలు(జ్ఞాపకం_2, దత్తాంశం_2.దత్తాంశం(), దత్తాంశం_2.కొలత_2());
  జ్ఞాపకం_2[దత్తాంశం_2.కొలత_2()] = 0; // Null terminate string.
}

నామాకాశ {
నిర్మాణం Named_బఫరు_Alloc {
  స్థిరరాశి Twine &Name;
  Named_బఫరు_Alloc(స్థిరరాశి Twine &Name) : Name(Name) {}
};
} // namespace

శూన్య *క్రియాకారకం కొత్త(size_t N, స్థిరరాశి Named_బఫరు_Alloc &Alloc) {
  SmallString<256> NameBuf;
  StringRef NameRef = Alloc.Name.గమ్యం_String_Ref(NameBuf);

  // We use malloc() and manually handle it returning null instead of calling
  // operator new because we need all uses of NamedBufferAlloc to be
  // deallocated with a call to free() due to needing to use malloc() in
  // WritableMemoryBuffer::getNewUninitMemBuffer() to work around the out-of-
  // memory handler installed by default in LLVM. See operator delete() member
  // functions within this file for the paired call to free().
  అక్షరం *Mem =
      static_cast<అక్షరం *>(std::జ్ఞాపకం_ఇవ్వు(N + పరిమాణం(size_t) + NameRef.కొలత_2() + 1));
  ఒకవేళ (!Mem)
    llvm::report_bad_alloc_error("Allocation failed");
  *reinterpret_cast<size_t *>(Mem + N) = NameRef.కొలత_2();
  CopyStringRef(Mem + N + పరిమాణం(size_t), NameRef);
  తిరిగి Mem;
}

నామాకాశ {
/// MemoryBufferMem - Named MemoryBuffer pointing to a block of memory.
మూస<రకం_పేరు MB>
తరగతి జ్ఞాపకం_బఫరు_Mem : బహిరంగ MB {
బహిరంగ:
  జ్ఞాపకం_బఫరు_Mem(StringRef ఆదానం_దత్తాంశం, తార్కిక RequiresNullTerminator) {
    జ్ఞాపకం_బఫరు::init(ఆదానం_దత్తాంశం.begin(), ఆదానం_దత్తాంశం.చివర(),
                       RequiresNullTerminator);
  }

  /// Disable sized deallocation for MemoryBufferMem, because it has
  /// tail-allocated data.
  శూన్య క్రియాకారకం తొలగించు(శూన్య *p) { std::విడుదల(p); }

  StringRef get_బఫరు_Identifier() స్థిరరాశి override {
    // The name is stored after the class itself.
    తిరిగి StringRef(reinterpret_cast<స్థిరరాశి అక్షరం *>(ఇది + 1) + పరిమాణం(size_t),
                     *reinterpret_cast<స్థిరరాశి size_t *>(ఇది + 1));
  }

  జ్ఞాపకం_బఫరు::బఫరు_రకం get_బఫరు_రకం() స్థిరరాశి override {
    తిరిగి జ్ఞాపకం_బఫరు::జ్ఞాపకం_బఫరు_Malloc;
  }
};
} // namespace

మూస <రకం_పేరు MB>
స్థిర ErrorOr<std::unique_ptr<MB>>
getFileAux(స్థిరరాశి Twine &Filename, uint64_t Map_కొలత, uint64_t స్థానభ్రంశం,
           తార్కిక Is_పాఠ్యం, తార్కిక RequiresNullTerminator, తార్కిక IsVolatile,
           std::optional<Align> Alignment);

std::unique_ptr<జ్ఞాపకం_బఫరు>
జ్ఞాపకం_బఫరు::get_Mem_బఫరు(StringRef ఆదానం_దత్తాంశం, StringRef బఫరు_Name,
                           తార్కిక RequiresNullTerminator) {
  స్వయంచాలక *Ret = కొత్త (Named_బఫరు_Alloc(బఫరు_Name))
      జ్ఞాపకం_బఫరు_Mem<జ్ఞాపకం_బఫరు>(ఆదానం_దత్తాంశం, RequiresNullTerminator);
  తిరిగి std::unique_ptr<జ్ఞాపకం_బఫరు>(Ret);
}

std::unique_ptr<జ్ఞాపకం_బఫరు>
జ్ఞాపకం_బఫరు::get_Mem_బఫరు(జ్ఞాపకం_బఫరు_Ref Ref, తార్కిక RequiresNullTerminator) {
  తిరిగి std::unique_ptr<జ్ఞాపకం_బఫరు>(get_Mem_బఫరు(
      Ref.get_బఫరు(), Ref.get_బఫరు_Identifier(), RequiresNullTerminator));
}

స్థిర ErrorOr<std::unique_ptr<Writable_జ్ఞాపకం_బఫరు>>
get_Mem_బఫరు_Copy_Impl(StringRef ఆదానం_దత్తాంశం, స్థిరరాశి Twine &బఫరు_Name) {
  స్వయంచాలక Buf =
      Writable_జ్ఞాపకం_బఫరు::get_New_Uninit_Mem_బఫరు(ఆదానం_దత్తాంశం.కొలత_2(), బఫరు_Name);
  ఒకవేళ (!Buf)
    తిరిగి make_error_సంకేతం(errc::not_enough_జ్ఞాపకం);
  // Calling memcpy with null src/dst is UB, and an empty StringRef is
  // represented with {nullptr, 0}.
  llvm::copy(ఆదానం_దత్తాంశం, Buf->get_బఫరు_ప్రారంభం());
  తిరిగి std::move(Buf);
}

std::unique_ptr<జ్ఞాపకం_బఫరు>
జ్ఞాపకం_బఫరు::get_Mem_బఫరు_Copy(StringRef ఆదానం_దత్తాంశం, స్థిరరాశి Twine &బఫరు_Name) {
  స్వయంచాలక Buf = get_Mem_బఫరు_Copy_Impl(ఆదానం_దత్తాంశం, బఫరు_Name);
  ఒకవేళ (Buf)
    తిరిగి std::move(*Buf);
  తిరిగి శూన్యసూచిక;
}

ErrorOr<std::unique_ptr<జ్ఞాపకం_బఫరు>>
జ్ఞాపకం_బఫరు::getFileOrSTDIN(స్థిరరాశి Twine &Filename, తార్కిక Is_పాఠ్యం,
                             తార్కిక RequiresNullTerminator,
                             std::optional<Align> Alignment) {
  sys::sandbox::violationIfEnabled();

  SmallString<256> NameBuf;
  StringRef NameRef = Filename.గమ్యం_String_Ref(NameBuf);

  ఒకవేళ (NameRef == "-")
    తిరిగి getSTDIN();
  తిరిగి getFile(Filename, Is_పాఠ్యం, RequiresNullTerminator,
                 /*IsVolatile=*/అబద్ధం, Alignment);
}

ErrorOr<std::unique_ptr<జ్ఞాపకం_బఫరు>>
జ్ఞాపకం_బఫరు::get_File_ముక్క(స్థిరరాశి Twine &File_మార్గం, uint64_t Map_కొలత,
                           uint64_t స్థానభ్రంశం, తార్కిక IsVolatile,
                           std::optional<Align> Alignment) {
  sys::sandbox::violationIfEnabled();

  తిరిగి getFileAux<జ్ఞాపకం_బఫరు>(File_మార్గం, Map_కొలత, స్థానభ్రంశం, /*IsText=*/అబద్ధం,
                                  /*RequiresNullTerminator=*/అబద్ధం, IsVolatile,
                                  Alignment);
}

//===----------------------------------------------------------------------===//
// MemoryBuffer::getFile implementation.
//===----------------------------------------------------------------------===//

నామాకాశ {

మూస <రకం_పేరు MB>
constexpr sys::fs::mapped_file_region::mapmode Mapmode =
    sys::fs::mapped_file_region::readonly;
మూస <>
constexpr sys::fs::mapped_file_region::mapmode Mapmode<జ్ఞాపకం_బఫరు> =
    sys::fs::mapped_file_region::readonly;
మూస <>
constexpr sys::fs::mapped_file_region::mapmode Mapmode<Writable_జ్ఞాపకం_బఫరు> =
    sys::fs::mapped_file_region::priv;
మూస <>
constexpr sys::fs::mapped_file_region::mapmode
    Mapmode<Write_Through_జ్ఞాపకం_బఫరు> = sys::fs::mapped_file_region::readwrite;

/// Memory maps a file descriptor using sys::fs::mapped_file_region.
///
/// This handles converting the offset into a legal offset on the platform.
మూస<రకం_పేరు MB>
తరగతి జ్ఞాపకం_బఫరు_MMap_File : బహిరంగ MB {
  sys::fs::mapped_file_region MFR;

  స్థిర uint64_t get_Legal_Map_స్థానభ్రంశం(uint64_t స్థానభ్రంశం) {
    తిరిగి స్థానభ్రంశం & ~(sys::fs::mapped_file_region::alignment() - 1);
  }

  స్థిర uint64_t get_Legal_Map_కొలత(uint64_t Len, uint64_t స్థానభ్రంశం) {
    తిరిగి Len + (స్థానభ్రంశం - get_Legal_Map_స్థానభ్రంశం(స్థానభ్రంశం));
  }

  స్థిరరాశి అక్షరం *get_ప్రారంభం(uint64_t Len, uint64_t స్థానభ్రంశం) {
    తిరిగి MFR.const_దత్తాంశం() + (స్థానభ్రంశం - get_Legal_Map_స్థానభ్రంశం(స్థానభ్రంశం));
  }

బహిరంగ:
  జ్ఞాపకం_బఫరు_MMap_File(తార్కిక RequiresNullTerminator, sys::fs::file_t FD, uint64_t Len,
                       uint64_t స్థానభ్రంశం, std::error_సంకేతం &EC)
      : MFR(FD, Mapmode<MB>, get_Legal_Map_కొలత(Len, స్థానభ్రంశం),
            get_Legal_Map_స్థానభ్రంశం(స్థానభ్రంశం), EC) {
    ఒకవేళ (!EC) {
      స్థిరరాశి అక్షరం *ప్రారంభం = get_ప్రారంభం(Len, స్థానభ్రంశం);
      జ్ఞాపకం_బఫరు::init(ప్రారంభం, ప్రారంభం + Len, RequiresNullTerminator);
    }
  }

  /// Disable sized deallocation for MemoryBufferMMapFile, because it has
  /// tail-allocated data.
  శూన్య క్రియాకారకం తొలగించు(శూన్య *p) { std::విడుదల(p); }

  StringRef get_బఫరు_Identifier() స్థిరరాశి override {
    // The name is stored after the class itself.
    తిరిగి StringRef(reinterpret_cast<స్థిరరాశి అక్షరం *>(ఇది + 1) + పరిమాణం(size_t),
                     *reinterpret_cast<స్థిరరాశి size_t *>(ఇది + 1));
  }

  జ్ఞాపకం_బఫరు::బఫరు_రకం get_బఫరు_రకం() స్థిరరాశి override {
    తిరిగి జ్ఞాపకం_బఫరు::జ్ఞాపకం_బఫరు_MMap;
  }

  శూన్య dontNeedIfMmap() override { MFR.dontNeed(); }
  శూన్య willNeedIfMmap() override { MFR.willNeed(); }
  శూన్య randomAccessIfMmap() override { MFR.randomAccess(); }
};
} // namespace

స్థిర ErrorOr<std::unique_ptr<Writable_జ్ఞాపకం_బఫరు>>
get_జ్ఞాపకం_బఫరు_For_ప్రవాహం(sys::fs::file_t FD, స్థిరరాశి Twine &బఫరు_Name) {
  SmallString<sys::fs::Default_Read_Chunk_కొలత> బఫరు;
  ఒకవేళ (Error E = sys::fs::read_Native_File_గమ్యం_EOF(FD, బఫరు))
    తిరిగి error_గమ్యం_Error_సంకేతం(std::move(E));
  తిరిగి get_Mem_బఫరు_Copy_Impl(బఫరు, బఫరు_Name);
}

ErrorOr<std::unique_ptr<జ్ఞాపకం_బఫరు>>
జ్ఞాపకం_బఫరు::getFile(స్థిరరాశి Twine &Filename, తార్కిక Is_పాఠ్యం,
                      తార్కిక RequiresNullTerminator, తార్కిక IsVolatile,
                      std::optional<Align> Alignment) {
  sys::sandbox::violationIfEnabled();

  తిరిగి getFileAux<జ్ఞాపకం_బఫరు>(Filename, /*MapSize=*/-1, /*Offset=*/0,
                                  Is_పాఠ్యం, RequiresNullTerminator, IsVolatile,
                                  Alignment);
}

మూస <రకం_పేరు MB>
స్థిర ErrorOr<std::unique_ptr<MB>>
getOpenFileImpl(sys::fs::file_t FD, స్థిరరాశి Twine &Filename, uint64_t File_కొలత,
                uint64_t Map_కొలత, int64_t స్థానభ్రంశం, తార్కిక RequiresNullTerminator,
                తార్కిక IsVolatile, std::optional<Align> Alignment);

మూస <రకం_పేరు MB>
స్థిర ErrorOr<std::unique_ptr<MB>>
getFileAux(స్థిరరాశి Twine &Filename, uint64_t Map_కొలత, uint64_t స్థానభ్రంశం,
           తార్కిక Is_పాఠ్యం, తార్కిక RequiresNullTerminator, తార్కిక IsVolatile,
           std::optional<Align> Alignment) {
  Expected<sys::fs::file_t> FDOrErr = sys::fs::openNativeFileForRead(
      Filename, Is_పాఠ్యం ? sys::fs::యొక్క_పాఠ్యం_With_CRLF : sys::fs::యొక్క_None);
  ఒకవేళ (!FDOrErr)
    తిరిగి error_గమ్యం_Error_సంకేతం(FDOrErr.takeError());
  sys::fs::file_t FD = *FDOrErr;
  స్వయంచాలక Ret = getOpenFileImpl<MB>(FD, Filename, /*FileSize=*/-1, Map_కొలత, స్థానభ్రంశం,
                                 RequiresNullTerminator, IsVolatile, Alignment);
  sys::fs::closeFile(FD);
  తిరిగి Ret;
}

ErrorOr<std::unique_ptr<Writable_జ్ఞాపకం_బఫరు>>
Writable_జ్ఞాపకం_బఫరు::getFile(స్థిరరాశి Twine &Filename, తార్కిక IsVolatile,
                              std::optional<Align> Alignment) {
  sys::sandbox::violationIfEnabled();

  తిరిగి getFileAux<Writable_జ్ఞాపకం_బఫరు>(
      Filename, /*MapSize=*/-1, /*Offset=*/0, /*IsText=*/అబద్ధం,
      /*RequiresNullTerminator=*/అబద్ధం, IsVolatile, Alignment);
}

ErrorOr<std::unique_ptr<Writable_జ్ఞాపకం_బఫరు>>
Writable_జ్ఞాపకం_బఫరు::get_File_ముక్క(స్థిరరాశి Twine &Filename, uint64_t Map_కొలత,
                                   uint64_t స్థానభ్రంశం, తార్కిక IsVolatile,
                                   std::optional<Align> Alignment) {
  sys::sandbox::violationIfEnabled();

  తిరిగి getFileAux<Writable_జ్ఞాపకం_బఫరు>(
      Filename, Map_కొలత, స్థానభ్రంశం, /*IsText=*/అబద్ధం,
      /*RequiresNullTerminator=*/అబద్ధం, IsVolatile, Alignment);
}

std::unique_ptr<Writable_జ్ఞాపకం_బఫరు>
Writable_జ్ఞాపకం_బఫరు::get_New_Uninit_Mem_బఫరు(size_t కొలత,
                                            స్థిరరాశి Twine &బఫరు_Name,
                                            std::optional<Align> Alignment) {
  ఉపయోగించి Mem_బఫరు = జ్ఞాపకం_బఫరు_Mem<Writable_జ్ఞాపకం_బఫరు>;

  // Use 16-byte alignment if no alignment is specified.
  Align BufAlign = Alignment.విలువ_or(Align(16));

  // Allocate space for the MemoryBuffer, the data and the name. It is important
  // that MemoryBuffer and data are aligned so PointerIntPair works with them.
  SmallString<256> NameBuf;
  StringRef NameRef = బఫరు_Name.గమ్యం_String_Ref(NameBuf);

  size_t StringLen = పరిమాణం(Mem_బఫరు) + పరిమాణం(size_t) + NameRef.కొలత_2() + 1;
  size_t RealLen = StringLen + కొలత + 1 + BufAlign.విలువ_2();
  ఒకవేళ (RealLen <= కొలత) // Check for rollover.
    తిరిగి శూన్యసూచిక;
  // We use a call to malloc() rather than a call to a non-throwing operator
  // new() because LLVM unconditionally installs an out of memory new handler
  // when exceptions are disabled. This new handler intentionally crashes to
  // aid with debugging, but that makes non-throwing new calls unhelpful.
  // See MemoryBufferMem::operator delete() for the paired call to free(), and
  // llvm::install_out_of_memory_new_handler() for the installation of the
  // custom new handler.
  అక్షరం *Mem = static_cast<అక్షరం *>(std::జ్ఞాపకం_ఇవ్వు(RealLen));
  ఒకవేళ (!Mem)
    తిరిగి శూన్యసూచిక;

  // The name is stored after the class itself.
  *reinterpret_cast<size_t *>(Mem + పరిమాణం(Mem_బఫరు)) = NameRef.కొలత_2();
  CopyStringRef(Mem + పరిమాణం(Mem_బఫరు) + పరిమాణం(size_t), NameRef);

  // The buffer begins after the name and must be aligned.
  అక్షరం *Buf = (అక్షరం *)alignAddr(Mem + StringLen, BufAlign);
  Buf[కొలత] = 0; // Null terminate buffer.

  స్వయంచాలక *Ret = కొత్త (Mem) Mem_బఫరు(StringRef(Buf, కొలత), నిజం);
  తిరిగి std::unique_ptr<Writable_జ్ఞాపకం_బఫరు>(Ret);
}

std::unique_ptr<Writable_జ్ఞాపకం_బఫరు>
Writable_జ్ఞాపకం_బఫరు::get_New_Mem_బఫరు(size_t కొలత, స్థిరరాశి Twine &బఫరు_Name) {
  స్వయంచాలక SB = Writable_జ్ఞాపకం_బఫరు::get_New_Uninit_Mem_బఫరు(కొలత, బఫరు_Name);
  ఒకవేళ (!SB)
    తిరిగి శూన్యసూచిక;
  memset(SB->get_బఫరు_ప్రారంభం(), 0, కొలత);
  తిరిగి SB;
}

స్థిర తార్కిక shouldUseMmap(sys::fs::file_t FD,
                          size_t File_కొలత,
                          size_t Map_కొలత,
                          off_t స్థానభ్రంశం,
                          తార్కిక RequiresNullTerminator,
                          పూర్ణాంకం పుట_కొలత,
                          తార్కిక IsVolatile) {
#ఒకవేళ defined(__MVS__)
  // zOS Enhanced ASCII auto convert does not support mmap.
  తిరిగి అబద్ధం;
#అయితే_ముగింపు

  // mmap may leave the buffer without null terminator if the file size changed
  // by the time the last page is mapped in, so avoid it if the file size is
  // likely to change.
  ఒకవేళ (IsVolatile && RequiresNullTerminator)
    తిరిగి అబద్ధం;

  // We don't use mmap for small files because this can severely fragment our
  // address space.
  ఒకవేళ (Map_కొలత < 4 * 4096 || Map_కొలత < (గుర్తులేని)పుట_కొలత)
    తిరిగి అబద్ధం;

  ఒకవేళ (!RequiresNullTerminator)
    తిరిగి నిజం;

  // If we don't know the file size, use fstat to find out.  fstat on an open
  // file descriptor is cheaper than stat on a random path.
  // FIXME: this chunk of code is duplicated, but it avoids a fstat when
  // RequiresNullTerminator = false and MapSize != -1.
  ఒకవేళ (File_కొలత == size_t(-1)) {
    sys::fs::file_స్థితి స్థితి_2;
    ఒకవేళ (sys::fs::స్థితి_3(FD, స్థితి_2))
      తిరిగి అబద్ధం;
    File_కొలత = స్థితి_2.get_కొలత();
  }

  // If we need a null terminator and the end of the map is inside the file,
  // we cannot use mmap.
  size_t చివర_2 = స్థానభ్రంశం + Map_కొలత;
  ప్రతిజ్ఞా(చివర_2 <= File_కొలత);
  ఒకవేళ (చివర_2 != File_కొలత)
    తిరిగి అబద్ధం;

  // Don't try to map files that are exactly a multiple of the system page size
  // if we need a null terminator.
  ఒకవేళ ((File_కొలత & (పుట_కొలత -1)) == 0)
    తిరిగి అబద్ధం;

#ఒకవేళ defined(__CYGWIN__)
  // Don't try to map files that are exactly a multiple of the physical page size
  // if we need a null terminator.
  // FIXME: We should reorganize again getPageSize() on Win32.
  ఒకవేళ ((File_కొలత & (4096 - 1)) == 0)
    తిరిగి అబద్ధం;
#అయితే_ముగింపు

  తిరిగి నిజం;
}

స్థిర ErrorOr<std::unique_ptr<Write_Through_జ్ఞాపకం_బఫరు>>
getReadWriteFile(స్థిరరాశి Twine &Filename, uint64_t File_కొలత, uint64_t Map_కొలత,
                 uint64_t స్థానభ్రంశం) {
  Expected<sys::fs::file_t> FDOrErr = sys::fs::openNativeFileForReadWrite(
      Filename, sys::fs::CD_OpenExisting, sys::fs::యొక్క_None);
  ఒకవేళ (!FDOrErr)
    తిరిగి error_గమ్యం_Error_సంకేతం(FDOrErr.takeError());
  sys::fs::file_t FD = *FDOrErr;

  // Default is to map the full file.
  ఒకవేళ (Map_కొలత == uint64_t(-1)) {
    // If we don't know the file size, use fstat to find out.  fstat on an open
    // file descriptor is cheaper than stat on a random path.
    ఒకవేళ (File_కొలత == uint64_t(-1)) {
      sys::fs::file_స్థితి స్థితి_2;
      std::error_సంకేతం EC = sys::fs::స్థితి_3(FD, స్థితి_2);
      ఒకవేళ (EC)
        తిరిగి EC;

      // If this not a file or a block device (e.g. it's a named pipe
      // or character device), we can't mmap it, so error out.
      sys::fs::file_type Type = స్థితి_2.type();
      ఒకవేళ (Type != sys::fs::file_type::regular_file &&
          Type != sys::fs::file_type::ఖండం_file)
        తిరిగి make_error_సంకేతం(errc::invalid_వాదం);

      File_కొలత = స్థితి_2.get_కొలత();
    }
    Map_కొలత = File_కొలత;
  }

  std::error_సంకేతం EC;
  std::unique_ptr<Write_Through_జ్ఞాపకం_బఫరు> ఫలితం(
      కొత్త (Named_బఫరు_Alloc(Filename))
          జ్ఞాపకం_బఫరు_MMap_File<Write_Through_జ్ఞాపకం_బఫరు>(అబద్ధం, FD, Map_కొలత,
                                                         స్థానభ్రంశం, EC));
  ఒకవేళ (EC)
    తిరిగి EC;
  తిరిగి std::move(ఫలితం);
}

ErrorOr<std::unique_ptr<Write_Through_జ్ఞాపకం_బఫరు>>
Write_Through_జ్ఞాపకం_బఫరు::getFile(స్థిరరాశి Twine &Filename, int64_t File_కొలత) {
  sys::sandbox::violationIfEnabled();

  తిరిగి getReadWriteFile(Filename, File_కొలత, File_కొలత, 0);
}

/// Map a subrange of the specified file as a WritableMemoryBuffer.
ErrorOr<std::unique_ptr<Write_Through_జ్ఞాపకం_బఫరు>>
Write_Through_జ్ఞాపకం_బఫరు::get_File_ముక్క(స్థిరరాశి Twine &Filename, uint64_t Map_కొలత,
                                       uint64_t స్థానభ్రంశం) {
  sys::sandbox::violationIfEnabled();

  తిరిగి getReadWriteFile(Filename, -1, Map_కొలత, స్థానభ్రంశం);
}

మూస <రకం_పేరు MB>
స్థిర ErrorOr<std::unique_ptr<MB>>
getOpenFileImpl(sys::fs::file_t FD, స్థిరరాశి Twine &Filename, uint64_t File_కొలత,
                uint64_t Map_కొలత, int64_t స్థానభ్రంశం, తార్కిక RequiresNullTerminator,
                తార్కిక IsVolatile, std::optional<Align> Alignment) {
  స్థిర పూర్ణాంకం పుట_కొలత = sys::Process::get_పుట_కొలత_Estimate();

  // Default is to map the full file.
  ఒకవేళ (Map_కొలత == uint64_t(-1)) {
    // If we don't know the file size, use fstat to find out.  fstat on an open
    // file descriptor is cheaper than stat on a random path.
    ఒకవేళ (File_కొలత == uint64_t(-1)) {
      sys::fs::file_స్థితి స్థితి_2;
      std::error_సంకేతం EC = sys::fs::స్థితి_3(FD, స్థితి_2);
      ఒకవేళ (EC)
        తిరిగి EC;

      // If this not a file or a block device (e.g. it's a named pipe
      // or character device), we can't trust the size. Create the memory
      // buffer by copying off the stream.
      sys::fs::file_type Type = స్థితి_2.type();
      ఒకవేళ (Type != sys::fs::file_type::regular_file &&
          Type != sys::fs::file_type::ఖండం_file)
        తిరిగి get_జ్ఞాపకం_బఫరు_For_ప్రవాహం(FD, Filename);

      File_కొలత = స్థితి_2.get_కొలత();
    }
    Map_కొలత = File_కొలత;
  }

  ఒకవేళ (shouldUseMmap(FD, File_కొలత, Map_కొలత, స్థానభ్రంశం, RequiresNullTerminator,
                    పుట_కొలత, IsVolatile)) {
    std::error_సంకేతం EC;
    std::unique_ptr<MB> ఫలితం(
        కొత్త (Named_బఫరు_Alloc(Filename)) జ్ఞాపకం_బఫరు_MMap_File<MB>(
            RequiresNullTerminator, FD, Map_కొలత, స్థానభ్రంశం, EC));
    ఒకవేళ (!EC) {
      // On at least Linux, and possibly on other systems, mmap may return pages
      // from the page cache that are not properly filled with trailing zeroes,
      // if some prior user of the page wrote non-zero bytes. Detect this and
      // don't use mmap in that case.
      ఒకవేళ (!RequiresNullTerminator || *ఫలితం->get_బఫరు_చివర() == '\0')
        తిరిగి std::move(ఫలితం);
    }
  }

#నిర్వచితమైతే __MVS__
  ErrorOr<తార్కిక> NeedsConversion = needConversion(Filename, FD);
  ఒకవేళ (std::error_సంకేతం EC = NeedsConversion.getError())
    తిరిగి EC;
  // File size may increase due to EBCDIC -> UTF-8 conversion, therefore we
  // cannot trust the file size and we create the memory buffer by copying
  // off the stream.
  // Note: This only works with the assumption of reading a full file (i.e,
  // Offset == 0 and MapSize == FileSize). Reading a file slice does not work.
  ఒకవేళ (*NeedsConversion && స్థానభ్రంశం == 0 && Map_కొలత == File_కొలత)
    తిరిగి get_జ్ఞాపకం_బఫరు_For_ప్రవాహం(FD, Filename);
#అయితే_ముగింపు

  స్వయంచాలక Buf =
      Writable_జ్ఞాపకం_బఫరు::get_New_Uninit_Mem_బఫరు(Map_కొలత, Filename, Alignment);
  ఒకవేళ (!Buf) {
    // Failed to create a buffer. The only way it can fail is if
    // new(std::nothrow) returns 0.
    తిరిగి make_error_సంకేతం(errc::not_enough_జ్ఞాపకం);
  }

  // Read until EOF, zero-initialize the rest.
  MutableArrayRef<అక్షరం> గమ్యం_Read = Buf->get_బఫరు();
  అయితే (!గమ్యం_Read.ఖాళీలు()) {
    Expected<size_t> Read_బైట్లు =
        sys::fs::read_Native_File_ముక్క(FD, గమ్యం_Read, స్థానభ్రంశం);
    ఒకవేళ (!Read_బైట్లు)
      తిరిగి error_గమ్యం_Error_సంకేతం(Read_బైట్లు.takeError());
    ఒకవేళ (*Read_బైట్లు == 0) {
      std::memset(గమ్యం_Read.దత్తాంశం(), 0, గమ్యం_Read.కొలత_2());
      ఆపు;
    }
    గమ్యం_Read = గమ్యం_Read.drop_front(*Read_బైట్లు);
    స్థానభ్రంశం += *Read_బైట్లు;
  }

  తిరిగి std::move(Buf);
}

ErrorOr<std::unique_ptr<జ్ఞాపకం_బఫరు>>
జ్ఞాపకం_బఫరు::getOpenFile(sys::fs::file_t FD, స్థిరరాశి Twine &Filename,
                          uint64_t File_కొలత, తార్కిక RequiresNullTerminator,
                          తార్కిక IsVolatile, std::optional<Align> Alignment) {
  sys::sandbox::violationIfEnabled();

  తిరిగి getOpenFileImpl<జ్ఞాపకం_బఫరు>(FD, Filename, File_కొలత, File_కొలత, 0,
                                       RequiresNullTerminator, IsVolatile,
                                       Alignment);
}

ErrorOr<std::unique_ptr<జ్ఞాపకం_బఫరు>> జ్ఞాపకం_బఫరు::get_Open_File_ముక్క(
    sys::fs::file_t FD, స్థిరరాశి Twine &Filename, uint64_t Map_కొలత, int64_t స్థానభ్రంశం,
    తార్కిక IsVolatile, std::optional<Align> Alignment) {
  ప్రతిజ్ఞా(Map_కొలత != uint64_t(-1));

  sys::sandbox::violationIfEnabled();

  తిరిగి getOpenFileImpl<జ్ఞాపకం_బఫరు>(FD, Filename, -1, Map_కొలత, స్థానభ్రంశం, అబద్ధం,
                                       IsVolatile, Alignment);
}

ErrorOr<std::unique_ptr<జ్ఞాపకం_బఫరు>> జ్ఞాపకం_బఫరు::getSTDIN() {
  sys::sandbox::violationIfEnabled();

  // Read in all of the data from stdin, we cannot mmap stdin.
  //
  // FIXME: That isn't necessarily true, we should try to mmap stdin and
  // fallback if it fails.
  sys::Change_Stdin_బహుళకం(sys::fs::యొక్క_పాఠ్యం);

  తిరిగి get_జ్ఞాపకం_బఫరు_For_ప్రవాహం(sys::fs::getStdinHandle(), "<stdin>");
}

ErrorOr<std::unique_ptr<జ్ఞాపకం_బఫరు>>
జ్ఞాపకం_బఫరు::get_File_As_ప్రవాహం(స్థిరరాశి Twine &Filename) {
  sys::sandbox::violationIfEnabled();

  Expected<sys::fs::file_t> FDOrErr =
      sys::fs::openNativeFileForRead(Filename, sys::fs::యొక్క_None);
  ఒకవేళ (!FDOrErr)
    తిరిగి error_గమ్యం_Error_సంకేతం(FDOrErr.takeError());
  sys::fs::file_t FD = *FDOrErr;
  ErrorOr<std::unique_ptr<జ్ఞాపకం_బఫరు>> Ret =
      get_జ్ఞాపకం_బఫరు_For_ప్రవాహం(FD, Filename);
  sys::fs::closeFile(FD);
  తిరిగి Ret;
}

జ్ఞాపకం_బఫరు_Ref జ్ఞాపకం_బఫరు::get_Mem_బఫరు_Ref() స్థిరరాశి {
  StringRef దత్తాంశం_2 = get_బఫరు();
  StringRef Identifier = get_బఫరు_Identifier();
  తిరిగి జ్ఞాపకం_బఫరు_Ref(దత్తాంశం_2, Identifier);
}

Small_సదిశ_జ్ఞాపకం_బఫరు::~Small_సదిశ_జ్ఞాపకం_బఫరు() = అప్రమేయ;
