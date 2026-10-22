//===- SourceMgr.cpp - Manager for Simple Source Buffers & Diagnostics ----===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file implements the SourceMgr class.  This class is used as a simple
// substrate for diagnostics, #include handling, and other low level things for
// simple parsers.
//
//===----------------------------------------------------------------------===//

#ଅନ୍ତର୍ଭାଵ "llvm/Support/SourceMgr.h"
#ଅନ୍ତର୍ଭାଵ "llvm/ADT/ArrayRef.h"
#ଅନ୍ତର୍ଭାଵ "llvm/ADT/STLExtras.h"
#ଅନ୍ତର୍ଭାଵ "llvm/ADT/SmallString.h"
#ଅନ୍ତର୍ଭାଵ "llvm/ADT/SmallVector.h"
#ଅନ୍ତର୍ଭାଵ "llvm/ADT/StringRef.h"
#ଅନ୍ତର୍ଭାଵ "llvm/ADT/Twine.h"
#ଅନ୍ତର୍ଭାଵ "llvm/Support/ErrorOr.h"
#ଅନ୍ତର୍ଭାଵ "llvm/Support/Locale.h"
#ଅନ୍ତର୍ଭାଵ "llvm/Support/MemoryBuffer.h"
#ଅନ୍ତର୍ଭାଵ "llvm/Support/Path.h"
#ଅନ୍ତର୍ଭାଵ "llvm/Support/SMLoc.h"
#ଅନ୍ତର୍ଭାଵ "llvm/Support/VirtualFileSystem.h"
#ଅନ୍ତର୍ଭାଵ "llvm/Support/WithColor.h"
#ଅନ୍ତର୍ଭାଵ "llvm/Support/raw_ostream.h"
#ଅନ୍ତର୍ଭାଵ <algorithm>
#ଅନ୍ତର୍ଭାଵ <cassert>
#ଅନ୍ତର୍ଭାଵ <cstddef>
#ଅନ୍ତର୍ଭାଵ <limits>
#ଅନ୍ତର୍ଭାଵ <memory>
#ଅନ୍ତର୍ଭାଵ <string>
#ଅନ୍ତର୍ଭାଵ <utility>

using ନାମାକାଶ llvm;

ସ୍ଥିର ଅଚଳ size_t TabStop = 8;

// Out of line to avoid needing definition of vfs::FileSystem in header.
SourceMgr::SourceMgr() = ପୂର୍ବନିର୍ଦ୍ଧାରିତ;
SourceMgr::SourceMgr(IntrusiveRefCntPtr<vfs::FileSystem> FS)
    : FS(std::move(FS)) {}
SourceMgr::SourceMgr(SourceMgr &&) = ପୂର୍ବନିର୍ଦ୍ଧାରିତ;
SourceMgr &SourceMgr::operator=(SourceMgr &&) = ପୂର୍ବନିର୍ଦ୍ଧାରିତ;
SourceMgr::~SourceMgr() = ପୂର୍ବନିର୍ଦ୍ଧାରିତ;

IntrusiveRefCntPtr<vfs::FileSystem> SourceMgr::getVirtualFileSystem() ଅଚଳ {
  ଫେରନ୍ତୁ FS;
}

ଶୂନ୍ୟ SourceMgr::setVirtualFileSystem(IntrusiveRefCntPtr<vfs::FileSystem> FS) {
  ଏହି->FS = std::move(FS);
}

unsigned SourceMgr::AddIncludeFile(ଅଚଳ std::string &Filename,
                                   SMLoc IncludeLoc,
                                   std::string &IncludedFile) {
  ErrorOr<std::unique_ptr<MemoryBuffer>> NewBufOrErr =
      OpenIncludeFile(Filename, IncludedFile);
  ଯଦି (!NewBufOrErr)
    ଫେରନ୍ତୁ 0;

  ଫେରନ୍ତୁ AddNewSourceBuffer(std::move(*NewBufOrErr), IncludeLoc);
}

ErrorOr<std::unique_ptr<MemoryBuffer>>
SourceMgr::OpenIncludeFile(ଅଚଳ std::string &Filename,
                           std::string &IncludedFile,
                           ତାର୍କିକ RequiresNullTerminator) {
  auto GetFile = [ଏହି, RequiresNullTerminator](StringRef Path) {
    ଫେରନ୍ତୁ FS ? FS->getBufferForFile(Path, /*FileSize=*/-1,
                                     RequiresNullTerminator)
              : MemoryBuffer::getFile(Path, /*IsText=*/ମିଥ୍ୟା,
                                      RequiresNullTerminator);
  };

  ErrorOr<std::unique_ptr<MemoryBuffer>> NewBufOrErr = GetFile(Filename);

  SmallString<64> Buffer(Filename);
  // If the file didn't exist directly, see if it's in an include path.
  ପାଇଁ (unsigned i = 0, e = IncludeDirectories.size(); i != e && !NewBufOrErr;
       ++i) {
    Buffer = IncludeDirectories[i];
    sys::path::append(Buffer, Filename);
    NewBufOrErr = GetFile(Buffer);
  }

  ଯଦି (NewBufOrErr)
    IncludedFile = static_cast<std::string>(Buffer);

  ଫେରନ୍ତୁ NewBufOrErr;
}

unsigned SourceMgr::FindBufferContainingLoc(SMLoc Loc) ଅଚଳ {
  ପାଇଁ (unsigned i = 0, e = Buffers.size(); i != e; ++i)
    ଯଦି (Loc.getPointer() >= Buffers[i].Buffer->getBufferStart() &&
        // Use <= here so that a pointer to the null at the end of the buffer
        // is included as part of the buffer.
        Loc.getPointer() <= Buffers[i].Buffer->getBufferEnd())
      ଫେରନ୍ତୁ i + 1;
  ଫେରନ୍ତୁ 0;
}

template <typename T>
ସ୍ଥିର std::vector<T> &GetOrCreateOffsetCache(ଶୂନ୍ୟ *&OffsetCache,
                                              MemoryBuffer *Buffer) {
  ଯଦି (OffsetCache)
    ଫେରନ୍ତୁ *static_cast<std::vector<T> *>(OffsetCache);

  // Lazily fill in the offset cache.
  auto *Offsets = ନୂତନ std::vector<T>();
  size_t Sz = Buffer->getBufferSize();
  ପ୍ରତିଜ୍ଞା(Sz <= std::numeric_limits<T>::max());
  StringRef S = Buffer->getBuffer();
  ପାଇଁ (size_t N = 0; N < Sz; ++N) {
    ଯଦି (S[N] == '\n')
      Offsets->ଠେଲ_back(static_cast<T>(N));
  }

  OffsetCache = Offsets;
  ଫେରନ୍ତୁ *Offsets;
}

template <typename T>
std::pair<unsigned, unsigned>
SourceMgr::SrcBuffer::get_ଧାଡ଼ି_And_Column_Specialized(ଅଚଳ ଅକ୍ଷର *Ptr) ଅଚଳ {
  std::vector<T> &Offsets =
      GetOrCreateOffsetCache<T>(OffsetCache, Buffer.get());

  ଅଚଳ ଅକ୍ଷର *BufStart = Buffer->getBufferStart();
  ପ୍ରତିଜ୍ଞା(Ptr >= BufStart && Ptr <= Buffer->getBufferEnd());
  ptrdiff_t PtrDiff = Ptr - BufStart;
  ପ୍ରତିଜ୍ଞା(PtrDiff >= 0 &&
         static_cast<size_t>(PtrDiff) <= std::numeric_limits<T>::max());
  T PtrOffset = static_cast<T>(PtrDiff);

  // llvm::lower_bound gives the number of EOL before PtrOffset. Add 1 to get
  // the line number.
  auto I = llvm::lower_bound(Offsets, PtrOffset);
  unsigned ଧାଡ଼ି_No = I - Offsets.begin() + 1;

  // The column number is the distance from the previous EOL (or the start of
  // the buffer) to the pointer.
  T ଧାଡ଼ି_Start_Offs = (I == Offsets.begin()) ? 0 : (I[-1] + 1);
  unsigned ColNo = PtrOffset - ଧାଡ଼ି_Start_Offs + 1;
  ଫେରନ୍ତୁ {ଧାଡ଼ି_No, ColNo};
}

/// Look up a given \p Ptr in the buffer, determining which line and column
/// it came from.
LLVM_ABI std::pair<unsigned, unsigned>
SourceMgr::SrcBuffer::get_ଧାଡ଼ି_And_Column(ଅଚଳ ଅକ୍ଷର *Ptr) ଅଚଳ {
  size_t Sz = Buffer->getBufferSize();
  ଯଦି (Sz <= std::numeric_limits<uint8_t>::max())
    ଫେରନ୍ତୁ get_ଧାଡ଼ି_And_Column_Specialized<uint8_t>(Ptr);
  ଯଦି (Sz <= std::numeric_limits<uint16_t>::max())
    ଫେରନ୍ତୁ get_ଧାଡ଼ି_And_Column_Specialized<uint16_t>(Ptr);
  ଯଦି (Sz <= std::numeric_limits<uint32_t>::max())
    ଫେରନ୍ତୁ get_ଧାଡ଼ି_And_Column_Specialized<uint32_t>(Ptr);
  ଫେରନ୍ତୁ get_ଧାଡ଼ି_And_Column_Specialized<uint64_t>(Ptr);
}

template <typename T>
ଅଚଳ ଅକ୍ଷର *SourceMgr::SrcBuffer::get_Pointer_For_ଧାଡ଼ି_ସଂଖ୍ୟା_Specialized(
    unsigned ଧାଡ଼ି_No) ଅଚଳ {
  std::vector<T> &Offsets =
      GetOrCreateOffsetCache<T>(OffsetCache, Buffer.get());

  // We start counting line and column numbers from 1.
  ଯଦି (ଧାଡ଼ି_No != 0)
    --ଧାଡ଼ି_No;

  ଅଚଳ ଅକ୍ଷର *BufStart = Buffer->getBufferStart();

  // The offset cache contains the location of the \n for the specified line,
  // we want the start of the line.  As such, we look for the previous entry.
  ଯଦି (ଧାଡ଼ି_No == 0)
    ଫେରନ୍ତୁ BufStart;
  ଯଦି (ଧାଡ଼ି_No > Offsets.size())
    ଫେରନ୍ତୁ nullptr;
  ଫେରନ୍ତୁ BufStart + Offsets[ଧାଡ଼ି_No - 1] + 1;
}

/// Return a pointer to the first character of the specified line number or
/// null if the line number is invalid.
ଅଚଳ ଅକ୍ଷର *
SourceMgr::SrcBuffer::get_Pointer_For_ଧାଡ଼ି_ସଂଖ୍ୟା(unsigned ଧାଡ଼ି_No) ଅଚଳ {
  size_t Sz = Buffer->getBufferSize();
  ଯଦି (Sz <= std::numeric_limits<uint8_t>::max())
    ଫେରନ୍ତୁ get_Pointer_For_ଧାଡ଼ି_ସଂଖ୍ୟା_Specialized<uint8_t>(ଧାଡ଼ି_No);
  ଅନ୍ୟଥା ଯଦି (Sz <= std::numeric_limits<uint16_t>::max())
    ଫେରନ୍ତୁ get_Pointer_For_ଧାଡ଼ି_ସଂଖ୍ୟା_Specialized<uint16_t>(ଧାଡ଼ି_No);
  ଅନ୍ୟଥା ଯଦି (Sz <= std::numeric_limits<uint32_t>::max())
    ଫେରନ୍ତୁ get_Pointer_For_ଧାଡ଼ି_ସଂଖ୍ୟା_Specialized<uint32_t>(ଧାଡ଼ି_No);
  ଅନ୍ୟଥା
    ଫେରନ୍ତୁ get_Pointer_For_ଧାଡ଼ି_ସଂଖ୍ୟା_Specialized<uint64_t>(ଧାଡ଼ି_No);
}

SourceMgr::SrcBuffer::SrcBuffer(SourceMgr::SrcBuffer &&Other)
    : Buffer(std::move(Other.Buffer)), OffsetCache(Other.OffsetCache),
      IncludeLoc(Other.IncludeLoc) {
  Other.OffsetCache = nullptr;
}

SourceMgr::SrcBuffer::~SrcBuffer() {
  ଯଦି (OffsetCache) {
    size_t Sz = Buffer->getBufferSize();
    ଯଦି (Sz <= std::numeric_limits<uint8_t>::max())
      delete static_cast<std::vector<uint8_t> *>(OffsetCache);
    ଅନ୍ୟଥା ଯଦି (Sz <= std::numeric_limits<uint16_t>::max())
      delete static_cast<std::vector<uint16_t> *>(OffsetCache);
    ଅନ୍ୟଥା ଯଦି (Sz <= std::numeric_limits<uint32_t>::max())
      delete static_cast<std::vector<uint32_t> *>(OffsetCache);
    ଅନ୍ୟଥା
      delete static_cast<std::vector<uint64_t> *>(OffsetCache);
    OffsetCache = nullptr;
  }
}

std::pair<unsigned, unsigned>
SourceMgr::get_ଧାଡ଼ି_And_Column(SMLoc Loc, unsigned BufferID) ଅଚଳ {
  ଯଦି (!BufferID)
    BufferID = FindBufferContainingLoc(Loc);
  ପ୍ରତିଜ୍ଞା(BufferID && "Invalid location!");

  auto &SB = getBufferInfo(BufferID);
  ଅଚଳ ଅକ୍ଷର *Ptr = Loc.getPointer();

  ଫେରନ୍ତୁ SB.get_ଧାଡ଼ି_And_Column(Ptr);
}

// FIXME: Note that the formatting of source locations is spread between
// multiple functions, some in SourceMgr and some in SMDiagnostic. A better
// solution would be a general-purpose source location formatter
// in one of those two classes, or possibly in SMLoc.

/// Get a string with the source location formatted in the standard
/// style, but without the line offset. If \p IncludePath is true, the path
/// is included. If false, only the file name and extension are included.
std::string SourceMgr::getFormattedLocationNoOffset(SMLoc Loc,
                                                    ତାର୍କିକ IncludePath) ଅଚଳ {
  auto BufferID = FindBufferContainingLoc(Loc);
  ପ୍ରତିଜ୍ଞା(BufferID && "Invalid location!");
  auto FileSpec = getBufferInfo(BufferID).Buffer->getBufferIdentifier();

  ଯଦି (IncludePath) {
    ଫେରନ୍ତୁ FileSpec.str() + ":" + std::to_string(Find_ଧାଡ଼ି_ସଂଖ୍ୟା(Loc, BufferID));
  } ଅନ୍ୟଥା {
    auto I = FileSpec.find_last_of("/\\");
    I = (I == FileSpec.size()) ? 0 : (I + 1);
    ଫେରନ୍ତୁ FileSpec.substr(I).str() + ":" +
           std::to_string(Find_ଧାଡ଼ି_ସଂଖ୍ୟା(Loc, BufferID));
  }
}

/// Given a line and column number in a mapped buffer, turn it into an SMLoc.
/// This will return a null SMLoc if the line/column location is invalid.
SMLoc SourceMgr::Find_Loc_For_ଧାଡ଼ି_And_Column(unsigned BufferID, unsigned ଧାଡ଼ି_No,
                                         unsigned ColNo) {
  auto &SB = getBufferInfo(BufferID);
  ଅଚଳ ଅକ୍ଷର *Ptr = SB.get_Pointer_For_ଧାଡ଼ି_ସଂଖ୍ୟା(ଧାଡ଼ି_No);
  ଯଦି (!Ptr)
    ଫେରନ୍ତୁ SMLoc();

  // We start counting line and column numbers from 1.
  ଯଦି (ColNo != 0)
    --ColNo;

  // If we have a column number, validate it.
  ଯଦି (ColNo) {
    // Make sure the location is within the current line.
    ଯଦି (Ptr + ColNo > SB.Buffer->getBufferEnd())
      ଫେରନ୍ତୁ SMLoc();

    // Make sure there is no newline in the way.
    ଯଦି (StringRef(Ptr, ColNo).find_first_of("\n\r") != StringRef::npos)
      ଫେରନ୍ତୁ SMLoc();

    Ptr += ColNo;
  }

  ଫେରନ୍ତୁ SMLoc::getFromPointer(Ptr);
}

ଶୂନ୍ୟ SourceMgr::Print_Include_ଗଦା(SMLoc IncludeLoc, raw_ostream &OS) ଅଚଳ {
  ଯଦି (IncludeLoc == SMLoc())
    ଫେରନ୍ତୁ; // Top of stack.

  unsigned CurBuf = FindBufferContainingLoc(IncludeLoc);
  ପ୍ରତିଜ୍ଞା(CurBuf && "Invalid or unspecified location!");

  Print_Include_ଗଦା(getBufferInfo(CurBuf).IncludeLoc, OS);

  OS << "Included from " << getBufferInfo(CurBuf).Buffer->getBufferIdentifier()
     << ":" << Find_ଧାଡ଼ି_ସଂଖ୍ୟା(IncludeLoc, CurBuf) << ":\n";
}

ଶୂନ୍ୟ SourceMgr::print_Include_ଗଦା_For_Diagnostic(SMLoc Loc,
                                               raw_ostream &OS) ଅଚଳ {
  ଯଦି (!Loc.isValid())
    ଫେରନ୍ତୁ;
  unsigned DiagCurBuffer = FindBufferContainingLoc(Loc);
  ଯଦି (DiagCurBuffer && DiagCurBuffer != getMainFileID()) {
    SMLoc ParentIncludeLoc = getParentIncludeLoc(DiagCurBuffer);
    // Ignore macro instantiation buffers to avoid redundant include stacks.
    Print_Include_ଗଦା(ParentIncludeLoc, OS);
  }
}

SMDiagnostic SourceMgr::GetMessage(SMLoc Loc, SourceMgr::DiagKind Kind,
                                   ଅଚଳ Twine &ସନ୍ଦେଶ_2, ArrayRef<SMRange> Ranges,
                                   ArrayRef<SMFixIt> FixIts) ଅଚଳ {
  // First thing to do: find the current buffer containing the specified
  // location to pull out the source line.
  SmallVector<std::pair<unsigned, unsigned>, 4> ColRanges;
  std::pair<unsigned, unsigned> ଧାଡ଼ି_And_Col;
  StringRef BufferID = "<unknown>";
  StringRef ଧାଡ଼ି_Str;

  ଯଦି (Loc.isValid()) {
    unsigned CurBuf = FindBufferContainingLoc(Loc);
    ପ୍ରତିଜ୍ଞା(CurBuf && "Invalid or unspecified location!");

    ଅଚଳ MemoryBuffer *CurMB = getMemoryBuffer(CurBuf);
    BufferID = CurMB->getBufferIdentifier();

    // Scan backward to find the start of the line.
    ଅଚଳ ଅକ୍ଷର *ଧାଡ଼ି_Start = Loc.getPointer();
    ଅଚଳ ଅକ୍ଷର *BufStart = CurMB->getBufferStart();
    ଯେତେବେଳେ (ଧାଡ଼ି_Start != BufStart && ଧାଡ଼ି_Start[-1] != '\n' &&
           ଧାଡ଼ି_Start[-1] != '\r')
      --ଧାଡ଼ି_Start;

    // Get the end of the line.
    ଅଚଳ ଅକ୍ଷର *ଧାଡ଼ି_End = Loc.getPointer();
    ଅଚଳ ଅକ୍ଷର *BufEnd = CurMB->getBufferEnd();
    ଯେତେବେଳେ (ଧାଡ଼ି_End != BufEnd && ଧାଡ଼ି_End[0] != '\n' && ଧାଡ଼ି_End[0] != '\r')
      ++ଧାଡ଼ି_End;
    ଧାଡ଼ି_Str = StringRef(ଧାଡ଼ି_Start, ଧାଡ଼ି_End - ଧାଡ଼ି_Start);

    // Convert any ranges to column ranges that only intersect the line of the
    // location.
    ପାଇଁ (SMRange R : Ranges) {
      ଯଦି (!R.isValid())
        ଜାରି;

      // If the line doesn't contain any part of the range, then ignore it.
      ଯଦି (R.Start.getPointer() > ଧାଡ଼ି_End || R.End.getPointer() < ଧାଡ଼ି_Start)
        ଜାରି;

      // Ignore pieces of the range that go onto other lines.
      ଯଦି (R.Start.getPointer() < ଧାଡ଼ି_Start)
        R.Start = SMLoc::getFromPointer(ଧାଡ଼ି_Start);
      ଯଦି (R.End.getPointer() > ଧାଡ଼ି_End)
        R.End = SMLoc::getFromPointer(ଧାଡ଼ି_End);

      // Translate from SMLoc ranges to column ranges.
      // FIXME: Handle multibyte characters.
      ColRanges.ଠେଲ_back(std::make_pair(R.Start.getPointer() - ଧାଡ଼ି_Start,
                                         R.End.getPointer() - ଧାଡ଼ି_Start));
    }

    ଧାଡ଼ି_And_Col = get_ଧାଡ଼ି_And_Column(Loc, CurBuf);
  }

  ଫେରନ୍ତୁ SMDiagnostic(*ଏହି, Loc, BufferID, ଧାଡ଼ି_And_Col.first,
                      ଧାଡ଼ି_And_Col.second - 1, Kind, ସନ୍ଦେଶ_2.str(), ଧାଡ଼ି_Str,
                      ColRanges, FixIts);
}

ଶୂନ୍ୟ SourceMgr::PrintMessage(raw_ostream &OS, ଅଚଳ SMDiagnostic &Diagnostic,
                             ତାର୍କିକ ShowColors) ଅଚଳ {
  // Report the message with the diagnostic handler if present.
  ଯଦି (DiagHandler) {
    DiagHandler(Diagnostic, DiagContext);
    ଫେରନ୍ତୁ;
  }

  ଯଦି (Diagnostic.getLoc().isValid()) {
    unsigned CurBuf = FindBufferContainingLoc(Diagnostic.getLoc());
    ପ୍ରତିଜ୍ଞା(CurBuf && "Invalid or unspecified location!");
    Print_Include_ଗଦା(getBufferInfo(CurBuf).IncludeLoc, OS);
  }

  Diagnostic.print(nullptr, OS, ShowColors);
}

ଶୂନ୍ୟ SourceMgr::PrintMessage(raw_ostream &OS, SMLoc Loc,
                             SourceMgr::DiagKind Kind, ଅଚଳ Twine &ସନ୍ଦେଶ_2,
                             ArrayRef<SMRange> Ranges, ArrayRef<SMFixIt> FixIts,
                             ତାର୍କିକ ShowColors) ଅଚଳ {
  PrintMessage(OS, GetMessage(Loc, Kind, ସନ୍ଦେଶ_2, Ranges, FixIts), ShowColors);
}

ଶୂନ୍ୟ SourceMgr::PrintMessage(SMLoc Loc, SourceMgr::DiagKind Kind,
                             ଅଚଳ Twine &ସନ୍ଦେଶ_2, ArrayRef<SMRange> Ranges,
                             ArrayRef<SMFixIt> FixIts, ତାର୍କିକ ShowColors) ଅଚଳ {
  PrintMessage(errs(), Loc, Kind, ସନ୍ଦେଶ_2, Ranges, FixIts, ShowColors);
}

//===----------------------------------------------------------------------===//
// SMFixIt Implementation
//===----------------------------------------------------------------------===//

SMFixIt::SMFixIt(SMRange R, ଅଚଳ Twine &Replacement)
    : Range(R), Text(Replacement.str()) {
  ପ୍ରତିଜ୍ଞା(R.isValid());
}

//===----------------------------------------------------------------------===//
// SMDiagnostic Implementation
//===----------------------------------------------------------------------===//

SMDiagnostic::SMDiagnostic(ଅଚଳ SourceMgr &sm, SMLoc L, StringRef FN, ପୂର୍ଣ୍ଣସଂଖ୍ୟା ଧାଡ଼ି,
                           ପୂର୍ଣ୍ଣସଂଖ୍ୟା Col, SourceMgr::DiagKind Kind, StringRef ସନ୍ଦେଶ_2,
                           StringRef ଧାଡ଼ି_Str,
                           ArrayRef<std::pair<unsigned, unsigned>> Ranges,
                           ArrayRef<SMFixIt> Hints)
    : SM(&sm), Loc(L), Filename(std::string(FN)), ଧାଡ଼ି_No(ଧାଡ଼ି), ColumnNo(Col),
      Kind(Kind), Message(ସନ୍ଦେଶ_2), ଧାଡ଼ି_Contents(ଧାଡ଼ି_Str), Ranges(Ranges.vec()),
      FixIts(Hints) {
  llvm::sort(FixIts);
}

ସ୍ଥିର ଶୂନ୍ୟ build_Fix_It_ଧାଡ଼ି(std::string &Caret_ଧାଡ଼ି, std::string &Fix_It_ଧାଡ଼ି,
                           ArrayRef<SMFixIt> FixIts,
                           ArrayRef<ଅକ୍ଷର> Source_ଧାଡ଼ି) {
  ଯଦି (FixIts.empty())
    ଫେରନ୍ତୁ;

  ଅଚଳ ଅକ୍ଷର *ଧାଡ଼ି_Start = Source_ଧାଡ଼ି.begin();
  ଅଚଳ ଅକ୍ଷର *ଧାଡ଼ି_End = Source_ଧାଡ଼ି.end();

  size_t PrevHintEndCol = 0;

  ପାଇଁ (ଅଚଳ llvm::SMFixIt &Fixit : FixIts) {
    // If the fixit contains a newline or tab, ignore it.
    ଯଦି (Fixit.getText().find_first_of("\n\r\t") != StringRef::npos)
      ଜାରି;

    SMRange R = Fixit.getRange();

    // If the line doesn't contain any part of the range, then ignore it.
    ଯଦି (R.Start.getPointer() > ଧାଡ଼ି_End || R.End.getPointer() < ଧାଡ଼ି_Start)
      ଜାରି;

    // Translate from SMLoc to column.
    // Ignore pieces of the range that go onto other lines.
    // FIXME: Handle multibyte characters in the source line.
    unsigned FirstCol;
    ଯଦି (R.Start.getPointer() < ଧାଡ଼ି_Start)
      FirstCol = 0;
    ଅନ୍ୟଥା
      FirstCol = R.Start.getPointer() - ଧାଡ଼ି_Start;

    // If we inserted a long previous hint, push this one forwards, and add
    // an extra space to show that this is not part of the previous
    // completion. This is sort of the best we can do when two hints appear
    // to overlap.
    //
    // Note that if this hint is located immediately after the previous
    // hint, no space will be added, since the location is more important.
    unsigned HintCol = FirstCol;
    ଯଦି (HintCol < PrevHintEndCol)
      HintCol = PrevHintEndCol + 1;

    // FIXME: This assertion is intended to catch unintended use of multibyte
    // characters in fixits. If we decide to do this, we'll have to track
    // separate byte widths for the source and fixit lines.
    ପ୍ରତିଜ୍ଞା((size_t)sys::locale::columnWidth(Fixit.getText()) ==
           Fixit.getText().size());

    // This relies on one byte per column in our fixit hints.
    unsigned LastColumnModified = HintCol + Fixit.getText().size();
    ଯଦି (LastColumnModified > Fix_It_ଧାଡ଼ି.size())
      Fix_It_ଧାଡ଼ି.resize(LastColumnModified, ' ');

    llvm::copy(Fixit.getText(), Fix_It_ଧାଡ଼ି.begin() + HintCol);

    PrevHintEndCol = LastColumnModified;

    // For replacements, mark the removal range with '~'.
    // FIXME: Handle multibyte characters in the source line.
    unsigned LastCol;
    ଯଦି (R.End.getPointer() >= ଧାଡ଼ି_End)
      LastCol = ଧାଡ଼ି_End - ଧାଡ଼ି_Start;
    ଅନ୍ୟଥା
      LastCol = R.End.getPointer() - ଧାଡ଼ି_Start;

    std::fill(&Caret_ଧାଡ଼ି[FirstCol], &Caret_ଧାଡ଼ି[LastCol], '~');
  }
}

ସ୍ଥିର ଶୂନ୍ୟ print_Source_ଧାଡ଼ି(raw_ostream &S, StringRef ଧାଡ଼ି_Contents) {
  // Print out the source line one character at a time, so we can expand tabs.
  ପାଇଁ (unsigned i = 0, e = ଧାଡ଼ି_Contents.size(), OutCol = 0; i != e; ++i) {
    size_t NextTab = ଧାଡ଼ି_Contents.find('\t', i);
    // If there were no tabs left, print the rest, we are done.
    ଯଦି (NextTab == StringRef::npos) {
      S << ଧାଡ଼ି_Contents.drop_front(i);
      ଭାଙ୍ଗ;
    }

    // Otherwise, print from i to NextTab.
    S << ଧାଡ଼ି_Contents.slice(i, NextTab);
    OutCol += NextTab - i;
    i = NextTab;

    // If we have a tab, emit at least one space, then round up to 8 columns.
    କର {
      S << ' ';
      ++OutCol;
    } ଯେତେବେଳେ ((OutCol % TabStop) != 0);
  }
  S << '\n';
}

ସ୍ଥିର ତାର୍କିକ isNonASCII(ଅକ୍ଷର c) { ଫେରନ୍ତୁ c & 0x80; }

ଶୂନ୍ୟ SMDiagnostic::print(ଅଚଳ ଅକ୍ଷର *ProgName, raw_ostream &OS, ତାର୍କିକ ShowColors,
                         ତାର୍କିକ ShowKindLabel, ତାର୍କିକ ShowLocation) ଅଚଳ {
  ColorMode Mode = ShowColors ? ColorMode::Auto : ColorMode::Disable;

  {
    WithColor S(OS, raw_ostream::SAVEDCOLOR, ସତ୍ୟ, ମିଥ୍ୟା, Mode);

    ଯଦି (ProgName && ProgName[0])
      S << ProgName << ": ";

    ଯଦି (ShowLocation && !Filename.empty()) {
      ଯଦି (Filename == "-")
        S << "<stdin>";
      ଅନ୍ୟଥା
        S << Filename;

      ଯଦି (ଧାଡ଼ି_No != -1) {
        S << ':' << ଧାଡ଼ି_No;
        ଯଦି (ColumnNo != -1)
          S << ':' << (ColumnNo + 1);
      }
      S << ": ";
    }
  }

  ଯଦି (ShowKindLabel) {
    ବାଛ (Kind) {
    ସ୍ଥିତି SourceMgr::DK_Error:
      WithColor::error(OS, "", !ShowColors);
      ଭାଙ୍ଗ;
    ସ୍ଥିତି SourceMgr::DK_Warning:
      WithColor::warning(OS, "", !ShowColors);
      ଭାଙ୍ଗ;
    ସ୍ଥିତି SourceMgr::DK_Note:
      WithColor::note(OS, "", !ShowColors);
      ଭାଙ୍ଗ;
    ସ୍ଥିତି SourceMgr::DK_Remark:
      WithColor::remark(OS, "", !ShowColors);
      ଭାଙ୍ଗ;
    }
  }

  WithColor(OS, raw_ostream::SAVEDCOLOR, ସତ୍ୟ, ମିଥ୍ୟା, Mode) << Message << '\n';

  ଯଦି (ଧାଡ଼ି_No == -1 || ColumnNo == -1)
    ଫେରନ୍ତୁ;

  // FIXME: If there are multibyte or multi-column characters in the source, all
  // our ranges will be wrong. To do this properly, we'll need a byte-to-column
  // map like Clang's TextDiagnostic. For now, we'll just handle tabs by
  // expanding them later, and bail out rather than show incorrect ranges and
  // misaligned fixits for any other odd characters.
  ଯଦି (any_of(ଧାଡ଼ି_Contents, isNonASCII)) {
    print_Source_ଧାଡ଼ି(OS, ଧାଡ଼ି_Contents);
    ଫେରନ୍ତୁ;
  }
  size_t ଅଙ୍କ_Columns = ଧାଡ଼ି_Contents.size();

  // Build the line with the caret and ranges.
  std::string Caret_ଧାଡ଼ି(ଅଙ୍କ_Columns + 1, ' ');

  // Expand any ranges.
  ପାଇଁ (ଅଚଳ std::pair<unsigned, unsigned> &R : Ranges)
    std::fill(&Caret_ଧାଡ଼ି[R.first],
              &Caret_ଧାଡ଼ି[std::min((size_t)R.second, Caret_ଧାଡ଼ି.size())], '~');

  // Add any fix-its.
  // FIXME: Find the beginning of the line properly for multibyte characters.
  std::string Fix_It_Insertion_ଧାଡ଼ି;
  build_Fix_It_ଧାଡ଼ି(Caret_ଧାଡ଼ି, Fix_It_Insertion_ଧାଡ଼ି, FixIts,
                 ArrayRef(Loc.getPointer() - ColumnNo, ଧାଡ଼ି_Contents.size()));

  // Finally, plop on the caret.
  ଯଦି (unsigned(ColumnNo) <= ଅଙ୍କ_Columns)
    Caret_ଧାଡ଼ି[ColumnNo] = '^';
  ଅନ୍ୟଥା
    Caret_ଧାଡ଼ି[ଅଙ୍କ_Columns] = '^';

  // ... and remove trailing whitespace so the output doesn't wrap for it.  We
  // know that the line isn't completely empty because it has the caret in it at
  // least.
  Caret_ଧାଡ଼ି.erase(Caret_ଧାଡ଼ି.find_last_not_of(' ') + 1);

  print_Source_ଧାଡ଼ି(OS, ଧାଡ଼ି_Contents);

  {
    ColorMode Mode = ShowColors ? ColorMode::Auto : ColorMode::Disable;
    WithColor S(OS, raw_ostream::GREEN, ସତ୍ୟ, ମିଥ୍ୟା, Mode);

    // Print out the caret line, matching tabs in the source line.
    ପାଇଁ (unsigned i = 0, e = Caret_ଧାଡ଼ି.size(), OutCol = 0; i != e; ++i) {
      ଯଦି (i >= ଧାଡ଼ି_Contents.size() || ଧାଡ଼ି_Contents[i] != '\t') {
        S << Caret_ଧାଡ଼ି[i];
        ++OutCol;
        ଜାରି;
      }

      // Okay, we have a tab.  Insert the appropriate number of characters.
      କର {
        S << Caret_ଧାଡ଼ି[i];
        ++OutCol;
      } ଯେତେବେଳେ ((OutCol % TabStop) != 0);
    }
    S << '\n';
  }

  // Print out the replacement line, matching tabs in the source line.
  ଯଦି (Fix_It_Insertion_ଧାଡ଼ି.empty())
    ଫେରନ୍ତୁ;

  ପାଇଁ (size_t i = 0, e = Fix_It_Insertion_ଧାଡ଼ି.size(), OutCol = 0; i < e; ++i) {
    ଯଦି (i >= ଧାଡ଼ି_Contents.size() || ଧାଡ଼ି_Contents[i] != '\t') {
      OS << Fix_It_Insertion_ଧାଡ଼ି[i];
      ++OutCol;
      ଜାରି;
    }

    // Okay, we have a tab.  Insert the appropriate number of characters.
    କର {
      OS << Fix_It_Insertion_ଧାଡ଼ି[i];
      // FIXME: This is trying not to break up replacements, but then to re-sync
      // with the tabs between replacements. This will fail, though, if two
      // fix-it replacements are exactly adjacent, or if a fix-it contains a
      // space. Really we should be precomputing column widths, which we'll
      // need anyway for multibyte chars.
      ଯଦି (Fix_It_Insertion_ଧାଡ଼ି[i] != ' ')
        ++i;
      ++OutCol;
    } ଯେତେବେଳେ (((OutCol % TabStop) != 0) && i != e);
  }
  OS << '\n';
}
