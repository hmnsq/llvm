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

#അന്തര്ഭാവ "llvm/Support/SourceMgr.h"
#അന്തര്ഭാവ "llvm/ADT/ArrayRef.h"
#അന്തര്ഭാവ "llvm/ADT/STLExtras.h"
#അന്തര്ഭാവ "llvm/ADT/SmallString.h"
#അന്തര്ഭാവ "llvm/ADT/SmallVector.h"
#അന്തര്ഭാവ "llvm/ADT/StringRef.h"
#അന്തര്ഭാവ "llvm/ADT/Twine.h"
#അന്തര്ഭാവ "llvm/Support/ErrorOr.h"
#അന്തര്ഭാവ "llvm/Support/Locale.h"
#അന്തര്ഭാവ "llvm/Support/MemoryBuffer.h"
#അന്തര്ഭാവ "llvm/Support/Path.h"
#അന്തര്ഭാവ "llvm/Support/SMLoc.h"
#അന്തര്ഭാവ "llvm/Support/VirtualFileSystem.h"
#അന്തര്ഭാവ "llvm/Support/WithColor.h"
#അന്തര്ഭാവ "llvm/Support/raw_ostream.h"
#അന്തര്ഭാവ <algorithm>
#അന്തര്ഭാവ <cassert>
#അന്തര്ഭാവ <cstddef>
#അന്തര്ഭാവ <limits>
#അന്തര്ഭാവ <memory>
#അന്തര്ഭാവ <string>
#അന്തര്ഭാവ <utility>

using നാമാകാശ llvm;

സ്ഥിതം സ്ഥിരം size_t TabStop = 8;

// Out of line to avoid needing definition of vfs::FileSystem in header.
SourceMgr::SourceMgr() = സ്ഥിരസ്ഥിതി;
SourceMgr::SourceMgr(IntrusiveRefCntPtr<vfs::FileSystem> FS)
    : FS(std::move(FS)) {}
SourceMgr::SourceMgr(SourceMgr &&) = സ്ഥിരസ്ഥിതി;
SourceMgr &SourceMgr::operator=(SourceMgr &&) = സ്ഥിരസ്ഥിതി;
SourceMgr::~SourceMgr() = സ്ഥിരസ്ഥിതി;

IntrusiveRefCntPtr<vfs::FileSystem> SourceMgr::getVirtualFileSystem() സ്ഥിരം {
  തിരികെ FS;
}

ശൂന്യം SourceMgr::setVirtualFileSystem(IntrusiveRefCntPtr<vfs::FileSystem> FS) {
  ഇത്->FS = std::move(FS);
}

unsigned SourceMgr::AddIncludeFile(സ്ഥിരം std::string &Filename,
                                   SMLoc IncludeLoc,
                                   std::string &IncludedFile) {
  ErrorOr<std::unique_ptr<MemoryBuffer>> NewBufOrErr =
      OpenIncludeFile(Filename, IncludedFile);
  എങ്കിൽ (!NewBufOrErr)
    തിരികെ 0;

  തിരികെ AddNewSourceBuffer(std::move(*NewBufOrErr), IncludeLoc);
}

ErrorOr<std::unique_ptr<MemoryBuffer>>
SourceMgr::OpenIncludeFile(സ്ഥിരം std::string &Filename,
                           std::string &IncludedFile,
                           തർക്കം RequiresNullTerminator) {
  auto GetFile = [ഇത്, RequiresNullTerminator](StringRef Path) {
    തിരികെ FS ? FS->getBufferForFile(Path, /*FileSize=*/-1,
                                     RequiresNullTerminator)
              : MemoryBuffer::getFile(Path, /*IsText=*/അസത്യം,
                                      RequiresNullTerminator);
  };

  ErrorOr<std::unique_ptr<MemoryBuffer>> NewBufOrErr = GetFile(Filename);

  SmallString<64> Buffer(Filename);
  // If the file didn't exist directly, see if it's in an include path.
  വേണ്ടി (unsigned i = 0, e = IncludeDirectories.size(); i != e && !NewBufOrErr;
       ++i) {
    Buffer = IncludeDirectories[i];
    sys::path::append(Buffer, Filename);
    NewBufOrErr = GetFile(Buffer);
  }

  എങ്കിൽ (NewBufOrErr)
    IncludedFile = static_cast<std::string>(Buffer);

  തിരികെ NewBufOrErr;
}

unsigned SourceMgr::FindBufferContainingLoc(SMLoc Loc) സ്ഥിരം {
  വേണ്ടി (unsigned i = 0, e = Buffers.size(); i != e; ++i)
    എങ്കിൽ (Loc.getPointer() >= Buffers[i].Buffer->getBufferStart() &&
        // Use <= here so that a pointer to the null at the end of the buffer
        // is included as part of the buffer.
        Loc.getPointer() <= Buffers[i].Buffer->getBufferEnd())
      തിരികെ i + 1;
  തിരികെ 0;
}

template <typename T>
സ്ഥിതം std::vector<T> &GetOrCreateOffsetCache(ശൂന്യം *&OffsetCache,
                                              MemoryBuffer *Buffer) {
  എങ്കിൽ (OffsetCache)
    തിരികെ *static_cast<std::vector<T> *>(OffsetCache);

  // Lazily fill in the offset cache.
  auto *Offsets = പുതിയ std::vector<T>();
  size_t Sz = Buffer->getBufferSize();
  പ്രതിജ്ഞാ(Sz <= std::numeric_limits<T>::max());
  StringRef S = Buffer->getBuffer();
  വേണ്ടി (size_t N = 0; N < Sz; ++N) {
    എങ്കിൽ (S[N] == '\n')
      Offsets->തള്ളുക_back(static_cast<T>(N));
  }

  OffsetCache = Offsets;
  തിരികെ *Offsets;
}

template <typename T>
std::pair<unsigned, unsigned>
SourceMgr::SrcBuffer::get_വരി_And_Column_Specialized(സ്ഥിരം അക്ഷരം *Ptr) സ്ഥിരം {
  std::vector<T> &Offsets =
      GetOrCreateOffsetCache<T>(OffsetCache, Buffer.get());

  സ്ഥിരം അക്ഷരം *BufStart = Buffer->getBufferStart();
  പ്രതിജ്ഞാ(Ptr >= BufStart && Ptr <= Buffer->getBufferEnd());
  ptrdiff_t PtrDiff = Ptr - BufStart;
  പ്രതിജ്ഞാ(PtrDiff >= 0 &&
         static_cast<size_t>(PtrDiff) <= std::numeric_limits<T>::max());
  T PtrOffset = static_cast<T>(PtrDiff);

  // llvm::lower_bound gives the number of EOL before PtrOffset. Add 1 to get
  // the line number.
  auto I = llvm::lower_bound(Offsets, PtrOffset);
  unsigned വരി_No = I - Offsets.begin() + 1;

  // The column number is the distance from the previous EOL (or the start of
  // the buffer) to the pointer.
  T വരി_Start_Offs = (I == Offsets.begin()) ? 0 : (I[-1] + 1);
  unsigned ColNo = PtrOffset - വരി_Start_Offs + 1;
  തിരികെ {വരി_No, ColNo};
}

/// Look up a given \p Ptr in the buffer, determining which line and column
/// it came from.
LLVM_ABI std::pair<unsigned, unsigned>
SourceMgr::SrcBuffer::get_വരി_And_Column(സ്ഥിരം അക്ഷരം *Ptr) സ്ഥിരം {
  size_t Sz = Buffer->getBufferSize();
  എങ്കിൽ (Sz <= std::numeric_limits<uint8_t>::max())
    തിരികെ get_വരി_And_Column_Specialized<uint8_t>(Ptr);
  എങ്കിൽ (Sz <= std::numeric_limits<uint16_t>::max())
    തിരികെ get_വരി_And_Column_Specialized<uint16_t>(Ptr);
  എങ്കിൽ (Sz <= std::numeric_limits<uint32_t>::max())
    തിരികെ get_വരി_And_Column_Specialized<uint32_t>(Ptr);
  തിരികെ get_വരി_And_Column_Specialized<uint64_t>(Ptr);
}

template <typename T>
സ്ഥിരം അക്ഷരം *SourceMgr::SrcBuffer::get_Pointer_For_വരി_സംഖ്യ_Specialized(
    unsigned വരി_No) സ്ഥിരം {
  std::vector<T> &Offsets =
      GetOrCreateOffsetCache<T>(OffsetCache, Buffer.get());

  // We start counting line and column numbers from 1.
  എങ്കിൽ (വരി_No != 0)
    --വരി_No;

  സ്ഥിരം അക്ഷരം *BufStart = Buffer->getBufferStart();

  // The offset cache contains the location of the \n for the specified line,
  // we want the start of the line.  As such, we look for the previous entry.
  എങ്കിൽ (വരി_No == 0)
    തിരികെ BufStart;
  എങ്കിൽ (വരി_No > Offsets.size())
    തിരികെ nullptr;
  തിരികെ BufStart + Offsets[വരി_No - 1] + 1;
}

/// Return a pointer to the first character of the specified line number or
/// null if the line number is invalid.
സ്ഥിരം അക്ഷരം *
SourceMgr::SrcBuffer::get_Pointer_For_വരി_സംഖ്യ(unsigned വരി_No) സ്ഥിരം {
  size_t Sz = Buffer->getBufferSize();
  എങ്കിൽ (Sz <= std::numeric_limits<uint8_t>::max())
    തിരികെ get_Pointer_For_വരി_സംഖ്യ_Specialized<uint8_t>(വരി_No);
  അല്ലെങ്കിൽ എങ്കിൽ (Sz <= std::numeric_limits<uint16_t>::max())
    തിരികെ get_Pointer_For_വരി_സംഖ്യ_Specialized<uint16_t>(വരി_No);
  അല്ലെങ്കിൽ എങ്കിൽ (Sz <= std::numeric_limits<uint32_t>::max())
    തിരികെ get_Pointer_For_വരി_സംഖ്യ_Specialized<uint32_t>(വരി_No);
  അല്ലെങ്കിൽ
    തിരികെ get_Pointer_For_വരി_സംഖ്യ_Specialized<uint64_t>(വരി_No);
}

SourceMgr::SrcBuffer::SrcBuffer(SourceMgr::SrcBuffer &&Other)
    : Buffer(std::move(Other.Buffer)), OffsetCache(Other.OffsetCache),
      IncludeLoc(Other.IncludeLoc) {
  Other.OffsetCache = nullptr;
}

SourceMgr::SrcBuffer::~SrcBuffer() {
  എങ്കിൽ (OffsetCache) {
    size_t Sz = Buffer->getBufferSize();
    എങ്കിൽ (Sz <= std::numeric_limits<uint8_t>::max())
      delete static_cast<std::vector<uint8_t> *>(OffsetCache);
    അല്ലെങ്കിൽ എങ്കിൽ (Sz <= std::numeric_limits<uint16_t>::max())
      delete static_cast<std::vector<uint16_t> *>(OffsetCache);
    അല്ലെങ്കിൽ എങ്കിൽ (Sz <= std::numeric_limits<uint32_t>::max())
      delete static_cast<std::vector<uint32_t> *>(OffsetCache);
    അല്ലെങ്കിൽ
      delete static_cast<std::vector<uint64_t> *>(OffsetCache);
    OffsetCache = nullptr;
  }
}

std::pair<unsigned, unsigned>
SourceMgr::get_വരി_And_Column(SMLoc Loc, unsigned BufferID) സ്ഥിരം {
  എങ്കിൽ (!BufferID)
    BufferID = FindBufferContainingLoc(Loc);
  പ്രതിജ്ഞാ(BufferID && "Invalid location!");

  auto &SB = getBufferInfo(BufferID);
  സ്ഥിരം അക്ഷരം *Ptr = Loc.getPointer();

  തിരികെ SB.get_വരി_And_Column(Ptr);
}

// FIXME: Note that the formatting of source locations is spread between
// multiple functions, some in SourceMgr and some in SMDiagnostic. A better
// solution would be a general-purpose source location formatter
// in one of those two classes, or possibly in SMLoc.

/// Get a string with the source location formatted in the standard
/// style, but without the line offset. If \p IncludePath is true, the path
/// is included. If false, only the file name and extension are included.
std::string SourceMgr::getFormattedLocationNoOffset(SMLoc Loc,
                                                    തർക്കം IncludePath) സ്ഥിരം {
  auto BufferID = FindBufferContainingLoc(Loc);
  പ്രതിജ്ഞാ(BufferID && "Invalid location!");
  auto FileSpec = getBufferInfo(BufferID).Buffer->getBufferIdentifier();

  എങ്കിൽ (IncludePath) {
    തിരികെ FileSpec.str() + ":" + std::to_string(Find_വരി_സംഖ്യ(Loc, BufferID));
  } അല്ലെങ്കിൽ {
    auto I = FileSpec.find_last_of("/\\");
    I = (I == FileSpec.size()) ? 0 : (I + 1);
    തിരികെ FileSpec.substr(I).str() + ":" +
           std::to_string(Find_വരി_സംഖ്യ(Loc, BufferID));
  }
}

/// Given a line and column number in a mapped buffer, turn it into an SMLoc.
/// This will return a null SMLoc if the line/column location is invalid.
SMLoc SourceMgr::Find_Loc_For_വരി_And_Column(unsigned BufferID, unsigned വരി_No,
                                         unsigned ColNo) {
  auto &SB = getBufferInfo(BufferID);
  സ്ഥിരം അക്ഷരം *Ptr = SB.get_Pointer_For_വരി_സംഖ്യ(വരി_No);
  എങ്കിൽ (!Ptr)
    തിരികെ SMLoc();

  // We start counting line and column numbers from 1.
  എങ്കിൽ (ColNo != 0)
    --ColNo;

  // If we have a column number, validate it.
  എങ്കിൽ (ColNo) {
    // Make sure the location is within the current line.
    എങ്കിൽ (Ptr + ColNo > SB.Buffer->getBufferEnd())
      തിരികെ SMLoc();

    // Make sure there is no newline in the way.
    എങ്കിൽ (StringRef(Ptr, ColNo).find_first_of("\n\r") != StringRef::npos)
      തിരികെ SMLoc();

    Ptr += ColNo;
  }

  തിരികെ SMLoc::getFromPointer(Ptr);
}

ശൂന്യം SourceMgr::Print_Include_അട്ടി(SMLoc IncludeLoc, raw_ostream &OS) സ്ഥിരം {
  എങ്കിൽ (IncludeLoc == SMLoc())
    തിരികെ; // Top of stack.

  unsigned CurBuf = FindBufferContainingLoc(IncludeLoc);
  പ്രതിജ്ഞാ(CurBuf && "Invalid or unspecified location!");

  Print_Include_അട്ടി(getBufferInfo(CurBuf).IncludeLoc, OS);

  OS << "Included from " << getBufferInfo(CurBuf).Buffer->getBufferIdentifier()
     << ":" << Find_വരി_സംഖ്യ(IncludeLoc, CurBuf) << ":\n";
}

ശൂന്യം SourceMgr::print_Include_അട്ടി_For_Diagnostic(SMLoc Loc,
                                               raw_ostream &OS) സ്ഥിരം {
  എങ്കിൽ (!Loc.isValid())
    തിരികെ;
  unsigned DiagCurBuffer = FindBufferContainingLoc(Loc);
  എങ്കിൽ (DiagCurBuffer && DiagCurBuffer != getMainFileID()) {
    SMLoc ParentIncludeLoc = getParentIncludeLoc(DiagCurBuffer);
    // Ignore macro instantiation buffers to avoid redundant include stacks.
    Print_Include_അട്ടി(ParentIncludeLoc, OS);
  }
}

SMDiagnostic SourceMgr::GetMessage(SMLoc Loc, SourceMgr::DiagKind Kind,
                                   സ്ഥിരം Twine &സന്ദേശം_2, ArrayRef<SMRange> Ranges,
                                   ArrayRef<SMFixIt> FixIts) സ്ഥിരം {
  // First thing to do: find the current buffer containing the specified
  // location to pull out the source line.
  SmallVector<std::pair<unsigned, unsigned>, 4> ColRanges;
  std::pair<unsigned, unsigned> വരി_And_Col;
  StringRef BufferID = "<unknown>";
  StringRef വരി_Str;

  എങ്കിൽ (Loc.isValid()) {
    unsigned CurBuf = FindBufferContainingLoc(Loc);
    പ്രതിജ്ഞാ(CurBuf && "Invalid or unspecified location!");

    സ്ഥിരം MemoryBuffer *CurMB = getMemoryBuffer(CurBuf);
    BufferID = CurMB->getBufferIdentifier();

    // Scan backward to find the start of the line.
    സ്ഥിരം അക്ഷരം *വരി_Start = Loc.getPointer();
    സ്ഥിരം അക്ഷരം *BufStart = CurMB->getBufferStart();
    ആകുമ്പോൾ (വരി_Start != BufStart && വരി_Start[-1] != '\n' &&
           വരി_Start[-1] != '\r')
      --വരി_Start;

    // Get the end of the line.
    സ്ഥിരം അക്ഷരം *വരി_End = Loc.getPointer();
    സ്ഥിരം അക്ഷരം *BufEnd = CurMB->getBufferEnd();
    ആകുമ്പോൾ (വരി_End != BufEnd && വരി_End[0] != '\n' && വരി_End[0] != '\r')
      ++വരി_End;
    വരി_Str = StringRef(വരി_Start, വരി_End - വരി_Start);

    // Convert any ranges to column ranges that only intersect the line of the
    // location.
    വേണ്ടി (SMRange R : Ranges) {
      എങ്കിൽ (!R.isValid())
        തുടരുക;

      // If the line doesn't contain any part of the range, then ignore it.
      എങ്കിൽ (R.Start.getPointer() > വരി_End || R.End.getPointer() < വരി_Start)
        തുടരുക;

      // Ignore pieces of the range that go onto other lines.
      എങ്കിൽ (R.Start.getPointer() < വരി_Start)
        R.Start = SMLoc::getFromPointer(വരി_Start);
      എങ്കിൽ (R.End.getPointer() > വരി_End)
        R.End = SMLoc::getFromPointer(വരി_End);

      // Translate from SMLoc ranges to column ranges.
      // FIXME: Handle multibyte characters.
      ColRanges.തള്ളുക_back(std::make_pair(R.Start.getPointer() - വരി_Start,
                                         R.End.getPointer() - വരി_Start));
    }

    വരി_And_Col = get_വരി_And_Column(Loc, CurBuf);
  }

  തിരികെ SMDiagnostic(*ഇത്, Loc, BufferID, വരി_And_Col.first,
                      വരി_And_Col.second - 1, Kind, സന്ദേശം_2.str(), വരി_Str,
                      ColRanges, FixIts);
}

ശൂന്യം SourceMgr::PrintMessage(raw_ostream &OS, സ്ഥിരം SMDiagnostic &Diagnostic,
                             തർക്കം ShowColors) സ്ഥിരം {
  // Report the message with the diagnostic handler if present.
  എങ്കിൽ (DiagHandler) {
    DiagHandler(Diagnostic, DiagContext);
    തിരികെ;
  }

  എങ്കിൽ (Diagnostic.getLoc().isValid()) {
    unsigned CurBuf = FindBufferContainingLoc(Diagnostic.getLoc());
    പ്രതിജ്ഞാ(CurBuf && "Invalid or unspecified location!");
    Print_Include_അട്ടി(getBufferInfo(CurBuf).IncludeLoc, OS);
  }

  Diagnostic.print(nullptr, OS, ShowColors);
}

ശൂന്യം SourceMgr::PrintMessage(raw_ostream &OS, SMLoc Loc,
                             SourceMgr::DiagKind Kind, സ്ഥിരം Twine &സന്ദേശം_2,
                             ArrayRef<SMRange> Ranges, ArrayRef<SMFixIt> FixIts,
                             തർക്കം ShowColors) സ്ഥിരം {
  PrintMessage(OS, GetMessage(Loc, Kind, സന്ദേശം_2, Ranges, FixIts), ShowColors);
}

ശൂന്യം SourceMgr::PrintMessage(SMLoc Loc, SourceMgr::DiagKind Kind,
                             സ്ഥിരം Twine &സന്ദേശം_2, ArrayRef<SMRange> Ranges,
                             ArrayRef<SMFixIt> FixIts, തർക്കം ShowColors) സ്ഥിരം {
  PrintMessage(errs(), Loc, Kind, സന്ദേശം_2, Ranges, FixIts, ShowColors);
}

//===----------------------------------------------------------------------===//
// SMFixIt Implementation
//===----------------------------------------------------------------------===//

SMFixIt::SMFixIt(SMRange R, സ്ഥിരം Twine &Replacement)
    : Range(R), Text(Replacement.str()) {
  പ്രതിജ്ഞാ(R.isValid());
}

//===----------------------------------------------------------------------===//
// SMDiagnostic Implementation
//===----------------------------------------------------------------------===//

SMDiagnostic::SMDiagnostic(സ്ഥിരം SourceMgr &sm, SMLoc L, StringRef FN, പൂർണ്ണസംഖ്യ വരി,
                           പൂർണ്ണസംഖ്യ Col, SourceMgr::DiagKind Kind, StringRef സന്ദേശം_2,
                           StringRef വരി_Str,
                           ArrayRef<std::pair<unsigned, unsigned>> Ranges,
                           ArrayRef<SMFixIt> Hints)
    : SM(&sm), Loc(L), Filename(std::string(FN)), വരി_No(വരി), ColumnNo(Col),
      Kind(Kind), Message(സന്ദേശം_2), വരി_Contents(വരി_Str), Ranges(Ranges.vec()),
      FixIts(Hints) {
  llvm::sort(FixIts);
}

സ്ഥിതം ശൂന്യം build_Fix_It_വരി(std::string &Caret_വരി, std::string &Fix_It_വരി,
                           ArrayRef<SMFixIt> FixIts,
                           ArrayRef<അക്ഷരം> Source_വരി) {
  എങ്കിൽ (FixIts.empty())
    തിരികെ;

  സ്ഥിരം അക്ഷരം *വരി_Start = Source_വരി.begin();
  സ്ഥിരം അക്ഷരം *വരി_End = Source_വരി.end();

  size_t PrevHintEndCol = 0;

  വേണ്ടി (സ്ഥിരം llvm::SMFixIt &Fixit : FixIts) {
    // If the fixit contains a newline or tab, ignore it.
    എങ്കിൽ (Fixit.getText().find_first_of("\n\r\t") != StringRef::npos)
      തുടരുക;

    SMRange R = Fixit.getRange();

    // If the line doesn't contain any part of the range, then ignore it.
    എങ്കിൽ (R.Start.getPointer() > വരി_End || R.End.getPointer() < വരി_Start)
      തുടരുക;

    // Translate from SMLoc to column.
    // Ignore pieces of the range that go onto other lines.
    // FIXME: Handle multibyte characters in the source line.
    unsigned FirstCol;
    എങ്കിൽ (R.Start.getPointer() < വരി_Start)
      FirstCol = 0;
    അല്ലെങ്കിൽ
      FirstCol = R.Start.getPointer() - വരി_Start;

    // If we inserted a long previous hint, push this one forwards, and add
    // an extra space to show that this is not part of the previous
    // completion. This is sort of the best we can do when two hints appear
    // to overlap.
    //
    // Note that if this hint is located immediately after the previous
    // hint, no space will be added, since the location is more important.
    unsigned HintCol = FirstCol;
    എങ്കിൽ (HintCol < PrevHintEndCol)
      HintCol = PrevHintEndCol + 1;

    // FIXME: This assertion is intended to catch unintended use of multibyte
    // characters in fixits. If we decide to do this, we'll have to track
    // separate byte widths for the source and fixit lines.
    പ്രതിജ്ഞാ((size_t)sys::locale::columnWidth(Fixit.getText()) ==
           Fixit.getText().size());

    // This relies on one byte per column in our fixit hints.
    unsigned LastColumnModified = HintCol + Fixit.getText().size();
    എങ്കിൽ (LastColumnModified > Fix_It_വരി.size())
      Fix_It_വരി.resize(LastColumnModified, ' ');

    llvm::copy(Fixit.getText(), Fix_It_വരി.begin() + HintCol);

    PrevHintEndCol = LastColumnModified;

    // For replacements, mark the removal range with '~'.
    // FIXME: Handle multibyte characters in the source line.
    unsigned LastCol;
    എങ്കിൽ (R.End.getPointer() >= വരി_End)
      LastCol = വരി_End - വരി_Start;
    അല്ലെങ്കിൽ
      LastCol = R.End.getPointer() - വരി_Start;

    std::fill(&Caret_വരി[FirstCol], &Caret_വരി[LastCol], '~');
  }
}

സ്ഥിതം ശൂന്യം print_Source_വരി(raw_ostream &S, StringRef വരി_Contents) {
  // Print out the source line one character at a time, so we can expand tabs.
  വേണ്ടി (unsigned i = 0, e = വരി_Contents.size(), OutCol = 0; i != e; ++i) {
    size_t NextTab = വരി_Contents.find('\t', i);
    // If there were no tabs left, print the rest, we are done.
    എങ്കിൽ (NextTab == StringRef::npos) {
      S << വരി_Contents.drop_front(i);
      നിർത്തുക;
    }

    // Otherwise, print from i to NextTab.
    S << വരി_Contents.slice(i, NextTab);
    OutCol += NextTab - i;
    i = NextTab;

    // If we have a tab, emit at least one space, then round up to 8 columns.
    ചെയ്യുക {
      S << ' ';
      ++OutCol;
    } ആകുമ്പോൾ ((OutCol % TabStop) != 0);
  }
  S << '\n';
}

സ്ഥിതം തർക്കം isNonASCII(അക്ഷരം c) { തിരികെ c & 0x80; }

ശൂന്യം SMDiagnostic::print(സ്ഥിരം അക്ഷരം *ProgName, raw_ostream &OS, തർക്കം ShowColors,
                         തർക്കം ShowKindLabel, തർക്കം ShowLocation) സ്ഥിരം {
  ColorMode Mode = ShowColors ? ColorMode::Auto : ColorMode::Disable;

  {
    WithColor S(OS, raw_ostream::SAVEDCOLOR, സത്യം, അസത്യം, Mode);

    എങ്കിൽ (ProgName && ProgName[0])
      S << ProgName << ": ";

    എങ്കിൽ (ShowLocation && !Filename.empty()) {
      എങ്കിൽ (Filename == "-")
        S << "<stdin>";
      അല്ലെങ്കിൽ
        S << Filename;

      എങ്കിൽ (വരി_No != -1) {
        S << ':' << വരി_No;
        എങ്കിൽ (ColumnNo != -1)
          S << ':' << (ColumnNo + 1);
      }
      S << ": ";
    }
  }

  എങ്കിൽ (ShowKindLabel) {
    തിരഞ്ഞെടുപ്പ് (Kind) {
    അവസ്ഥ SourceMgr::DK_Error:
      WithColor::error(OS, "", !ShowColors);
      നിർത്തുക;
    അവസ്ഥ SourceMgr::DK_Warning:
      WithColor::warning(OS, "", !ShowColors);
      നിർത്തുക;
    അവസ്ഥ SourceMgr::DK_Note:
      WithColor::note(OS, "", !ShowColors);
      നിർത്തുക;
    അവസ്ഥ SourceMgr::DK_Remark:
      WithColor::remark(OS, "", !ShowColors);
      നിർത്തുക;
    }
  }

  WithColor(OS, raw_ostream::SAVEDCOLOR, സത്യം, അസത്യം, Mode) << Message << '\n';

  എങ്കിൽ (വരി_No == -1 || ColumnNo == -1)
    തിരികെ;

  // FIXME: If there are multibyte or multi-column characters in the source, all
  // our ranges will be wrong. To do this properly, we'll need a byte-to-column
  // map like Clang's TextDiagnostic. For now, we'll just handle tabs by
  // expanding them later, and bail out rather than show incorrect ranges and
  // misaligned fixits for any other odd characters.
  എങ്കിൽ (any_of(വരി_Contents, isNonASCII)) {
    print_Source_വരി(OS, വരി_Contents);
    തിരികെ;
  }
  size_t അക്കം_Columns = വരി_Contents.size();

  // Build the line with the caret and ranges.
  std::string Caret_വരി(അക്കം_Columns + 1, ' ');

  // Expand any ranges.
  വേണ്ടി (സ്ഥിരം std::pair<unsigned, unsigned> &R : Ranges)
    std::fill(&Caret_വരി[R.first],
              &Caret_വരി[std::min((size_t)R.second, Caret_വരി.size())], '~');

  // Add any fix-its.
  // FIXME: Find the beginning of the line properly for multibyte characters.
  std::string Fix_It_Insertion_വരി;
  build_Fix_It_വരി(Caret_വരി, Fix_It_Insertion_വരി, FixIts,
                 ArrayRef(Loc.getPointer() - ColumnNo, വരി_Contents.size()));

  // Finally, plop on the caret.
  എങ്കിൽ (unsigned(ColumnNo) <= അക്കം_Columns)
    Caret_വരി[ColumnNo] = '^';
  അല്ലെങ്കിൽ
    Caret_വരി[അക്കം_Columns] = '^';

  // ... and remove trailing whitespace so the output doesn't wrap for it.  We
  // know that the line isn't completely empty because it has the caret in it at
  // least.
  Caret_വരി.erase(Caret_വരി.find_last_not_of(' ') + 1);

  print_Source_വരി(OS, വരി_Contents);

  {
    ColorMode Mode = ShowColors ? ColorMode::Auto : ColorMode::Disable;
    WithColor S(OS, raw_ostream::GREEN, സത്യം, അസത്യം, Mode);

    // Print out the caret line, matching tabs in the source line.
    വേണ്ടി (unsigned i = 0, e = Caret_വരി.size(), OutCol = 0; i != e; ++i) {
      എങ്കിൽ (i >= വരി_Contents.size() || വരി_Contents[i] != '\t') {
        S << Caret_വരി[i];
        ++OutCol;
        തുടരുക;
      }

      // Okay, we have a tab.  Insert the appropriate number of characters.
      ചെയ്യുക {
        S << Caret_വരി[i];
        ++OutCol;
      } ആകുമ്പോൾ ((OutCol % TabStop) != 0);
    }
    S << '\n';
  }

  // Print out the replacement line, matching tabs in the source line.
  എങ്കിൽ (Fix_It_Insertion_വരി.empty())
    തിരികെ;

  വേണ്ടി (size_t i = 0, e = Fix_It_Insertion_വരി.size(), OutCol = 0; i < e; ++i) {
    എങ്കിൽ (i >= വരി_Contents.size() || വരി_Contents[i] != '\t') {
      OS << Fix_It_Insertion_വരി[i];
      ++OutCol;
      തുടരുക;
    }

    // Okay, we have a tab.  Insert the appropriate number of characters.
    ചെയ്യുക {
      OS << Fix_It_Insertion_വരി[i];
      // FIXME: This is trying not to break up replacements, but then to re-sync
      // with the tabs between replacements. This will fail, though, if two
      // fix-it replacements are exactly adjacent, or if a fix-it contains a
      // space. Really we should be precomputing column widths, which we'll
      // need anyway for multibyte chars.
      എങ്കിൽ (Fix_It_Insertion_വരി[i] != ' ')
        ++i;
      ++OutCol;
    } ആകുമ്പോൾ (((OutCol % TabStop) != 0) && i != e);
  }
  OS << '\n';
}
