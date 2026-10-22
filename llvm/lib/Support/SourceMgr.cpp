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

#अन्तर्भाव "llvm/Support/SourceMgr.h"
#अन्तर्भाव "llvm/ADT/ArrayRef.h"
#अन्तर्भाव "llvm/ADT/STLExtras.h"
#अन्तर्भाव "llvm/ADT/SmallString.h"
#अन्तर्भाव "llvm/ADT/SmallVector.h"
#अन्तर्भाव "llvm/ADT/StringRef.h"
#अन्तर्भाव "llvm/ADT/Twine.h"
#अन्तर्भाव "llvm/Support/ErrorOr.h"
#अन्तर्भाव "llvm/Support/Locale.h"
#अन्तर्भाव "llvm/Support/MemoryBuffer.h"
#अन्तर्भाव "llvm/Support/Path.h"
#अन्तर्भाव "llvm/Support/SMLoc.h"
#अन्तर्भाव "llvm/Support/VirtualFileSystem.h"
#अन्तर्भाव "llvm/Support/WithColor.h"
#अन्तर्भाव "llvm/Support/raw_ostream.h"
#अन्तर्भाव <algorithm>
#अन्तर्भाव <cassert>
#अन्तर्भाव <cstddef>
#अन्तर्भाव <limits>
#अन्तर्भाव <memory>
#अन्तर्भाव <string>
#अन्तर्भाव <utility>

using नामाकाश llvm;

स्थिर ध्रुव size_t TabStop = 8;

// Out of line to avoid needing definition of vfs::FileSystem in header.
SourceMgr::SourceMgr() = स्वभाव;
SourceMgr::SourceMgr(IntrusiveRefCntPtr<vfs::FileSystem> FS)
    : FS(std::move(FS)) {}
SourceMgr::SourceMgr(SourceMgr &&) = स्वभाव;
SourceMgr &SourceMgr::operator=(SourceMgr &&) = स्वभाव;
SourceMgr::~SourceMgr() = स्वभाव;

IntrusiveRefCntPtr<vfs::FileSystem> SourceMgr::getVirtualFileSystem() ध्रुव {
  पटिनिवत्त FS;
}

शून्य SourceMgr::setVirtualFileSystem(IntrusiveRefCntPtr<vfs::FileSystem> FS) {
  एतत्->FS = std::move(FS);
}

unsigned SourceMgr::AddIncludeFile(ध्रुव std::string &Filename,
                                   SMLoc IncludeLoc,
                                   std::string &IncludedFile) {
  ErrorOr<std::unique_ptr<MemoryBuffer>> NewBufOrErr =
      OpenIncludeFile(Filename, IncludedFile);
  सचे (!NewBufOrErr)
    पटिनिवत्त 0;

  पटिनिवत्त AddNewSourceBuffer(std::move(*NewBufOrErr), IncludeLoc);
}

ErrorOr<std::unique_ptr<MemoryBuffer>>
SourceMgr::OpenIncludeFile(ध्रुव std::string &Filename,
                           std::string &IncludedFile,
                           तार्किक RequiresNullTerminator) {
  auto GetFile = [एतत्, RequiresNullTerminator](StringRef Path) {
    पटिनिवत्त FS ? FS->getBufferForFile(Path, /*FileSize=*/-1,
                                     RequiresNullTerminator)
              : MemoryBuffer::getFile(Path, /*IsText=*/मिच्छा,
                                      RequiresNullTerminator);
  };

  ErrorOr<std::unique_ptr<MemoryBuffer>> NewBufOrErr = GetFile(Filename);

  SmallString<64> Buffer(Filename);
  // If the file didn't exist directly, see if it's in an include path.
  अत्थाय (unsigned i = 0, e = IncludeDirectories.size(); i != e && !NewBufOrErr;
       ++i) {
    Buffer = IncludeDirectories[i];
    sys::path::append(Buffer, Filename);
    NewBufOrErr = GetFile(Buffer);
  }

  सचे (NewBufOrErr)
    IncludedFile = static_cast<std::string>(Buffer);

  पटिनिवत्त NewBufOrErr;
}

unsigned SourceMgr::FindBufferContainingLoc(SMLoc Loc) ध्रुव {
  अत्थाय (unsigned i = 0, e = Buffers.size(); i != e; ++i)
    सचे (Loc.getPointer() >= Buffers[i].Buffer->getBufferStart() &&
        // Use <= here so that a pointer to the null at the end of the buffer
        // is included as part of the buffer.
        Loc.getPointer() <= Buffers[i].Buffer->getBufferEnd())
      पटिनिवत्त i + 1;
  पटिनिवत्त 0;
}

template <typename T>
स्थिर std::vector<T> &GetOrCreateOffsetCache(शून्य *&OffsetCache,
                                              MemoryBuffer *Buffer) {
  सचे (OffsetCache)
    पटिनिवत्त *static_cast<std::vector<T> *>(OffsetCache);

  // Lazily fill in the offset cache.
  auto *Offsets = नूतन std::vector<T>();
  size_t Sz = Buffer->getBufferSize();
  प्रतिज्ञा(Sz <= std::numeric_limits<T>::max());
  StringRef S = Buffer->getBuffer();
  अत्थाय (size_t N = 0; N < Sz; ++N) {
    सचे (S[N] == '\n')
      Offsets->प्रक्षिप_back(static_cast<T>(N));
  }

  OffsetCache = Offsets;
  पटिनिवत्त *Offsets;
}

template <typename T>
std::pair<unsigned, unsigned>
SourceMgr::SrcBuffer::get_पङ्क्तिः_And_Column_Specialized(ध्रुव वर्ण *Ptr) ध्रुव {
  std::vector<T> &Offsets =
      GetOrCreateOffsetCache<T>(OffsetCache, Buffer.get());

  ध्रुव वर्ण *BufStart = Buffer->getBufferStart();
  प्रतिज्ञा(Ptr >= BufStart && Ptr <= Buffer->getBufferEnd());
  ptrdiff_t PtrDiff = Ptr - BufStart;
  प्रतिज्ञा(PtrDiff >= 0 &&
         static_cast<size_t>(PtrDiff) <= std::numeric_limits<T>::max());
  T PtrOffset = static_cast<T>(PtrDiff);

  // llvm::lower_bound gives the number of EOL before PtrOffset. Add 1 to get
  // the line number.
  auto I = llvm::lower_bound(Offsets, PtrOffset);
  unsigned पङ्क्तिः_No = I - Offsets.begin() + 1;

  // The column number is the distance from the previous EOL (or the start of
  // the buffer) to the pointer.
  T पङ्क्तिः_Start_Offs = (I == Offsets.begin()) ? 0 : (I[-1] + 1);
  unsigned ColNo = PtrOffset - पङ्क्तिः_Start_Offs + 1;
  पटिनिवत्त {पङ्क्तिः_No, ColNo};
}

/// Look up a given \p Ptr in the buffer, determining which line and column
/// it came from.
LLVM_ABI std::pair<unsigned, unsigned>
SourceMgr::SrcBuffer::get_पङ्क्तिः_And_Column(ध्रुव वर्ण *Ptr) ध्रुव {
  size_t Sz = Buffer->getBufferSize();
  सचे (Sz <= std::numeric_limits<uint8_t>::max())
    पटिनिवत्त get_पङ्क्तिः_And_Column_Specialized<uint8_t>(Ptr);
  सचे (Sz <= std::numeric_limits<uint16_t>::max())
    पटिनिवत्त get_पङ्क्तिः_And_Column_Specialized<uint16_t>(Ptr);
  सचे (Sz <= std::numeric_limits<uint32_t>::max())
    पटिनिवत्त get_पङ्क्तिः_And_Column_Specialized<uint32_t>(Ptr);
  पटिनिवत्त get_पङ्क्तिः_And_Column_Specialized<uint64_t>(Ptr);
}

template <typename T>
ध्रुव वर्ण *SourceMgr::SrcBuffer::get_Pointer_For_पङ्क्तिः_सङ्ख्या_Specialized(
    unsigned पङ्क्तिः_No) ध्रुव {
  std::vector<T> &Offsets =
      GetOrCreateOffsetCache<T>(OffsetCache, Buffer.get());

  // We start counting line and column numbers from 1.
  सचे (पङ्क्तिः_No != 0)
    --पङ्क्तिः_No;

  ध्रुव वर्ण *BufStart = Buffer->getBufferStart();

  // The offset cache contains the location of the \n for the specified line,
  // we want the start of the line.  As such, we look for the previous entry.
  सचे (पङ्क्तिः_No == 0)
    पटिनिवत्त BufStart;
  सचे (पङ्क्तिः_No > Offsets.size())
    पटिनिवत्त nullptr;
  पटिनिवत्त BufStart + Offsets[पङ्क्तिः_No - 1] + 1;
}

/// Return a pointer to the first character of the specified line number or
/// null if the line number is invalid.
ध्रुव वर्ण *
SourceMgr::SrcBuffer::get_Pointer_For_पङ्क्तिः_सङ्ख्या(unsigned पङ्क्तिः_No) ध्रुव {
  size_t Sz = Buffer->getBufferSize();
  सचे (Sz <= std::numeric_limits<uint8_t>::max())
    पटिनिवत्त get_Pointer_For_पङ्क्तिः_सङ्ख्या_Specialized<uint8_t>(पङ्क्तिः_No);
  अञ्ञथा सचे (Sz <= std::numeric_limits<uint16_t>::max())
    पटिनिवत्त get_Pointer_For_पङ्क्तिः_सङ्ख्या_Specialized<uint16_t>(पङ्क्तिः_No);
  अञ्ञथा सचे (Sz <= std::numeric_limits<uint32_t>::max())
    पटिनिवत्त get_Pointer_For_पङ्क्तिः_सङ्ख्या_Specialized<uint32_t>(पङ्क्तिः_No);
  अञ्ञथा
    पटिनिवत्त get_Pointer_For_पङ्क्तिः_सङ्ख्या_Specialized<uint64_t>(पङ्क्तिः_No);
}

SourceMgr::SrcBuffer::SrcBuffer(SourceMgr::SrcBuffer &&Other)
    : Buffer(std::move(Other.Buffer)), OffsetCache(Other.OffsetCache),
      IncludeLoc(Other.IncludeLoc) {
  Other.OffsetCache = nullptr;
}

SourceMgr::SrcBuffer::~SrcBuffer() {
  सचे (OffsetCache) {
    size_t Sz = Buffer->getBufferSize();
    सचे (Sz <= std::numeric_limits<uint8_t>::max())
      delete static_cast<std::vector<uint8_t> *>(OffsetCache);
    अञ्ञथा सचे (Sz <= std::numeric_limits<uint16_t>::max())
      delete static_cast<std::vector<uint16_t> *>(OffsetCache);
    अञ्ञथा सचे (Sz <= std::numeric_limits<uint32_t>::max())
      delete static_cast<std::vector<uint32_t> *>(OffsetCache);
    अञ्ञथा
      delete static_cast<std::vector<uint64_t> *>(OffsetCache);
    OffsetCache = nullptr;
  }
}

std::pair<unsigned, unsigned>
SourceMgr::get_पङ्क्तिः_And_Column(SMLoc Loc, unsigned BufferID) ध्रुव {
  सचे (!BufferID)
    BufferID = FindBufferContainingLoc(Loc);
  प्रतिज्ञा(BufferID && "Invalid location!");

  auto &SB = getBufferInfo(BufferID);
  ध्रुव वर्ण *Ptr = Loc.getPointer();

  पटिनिवत्त SB.get_पङ्क्तिः_And_Column(Ptr);
}

// FIXME: Note that the formatting of source locations is spread between
// multiple functions, some in SourceMgr and some in SMDiagnostic. A better
// solution would be a general-purpose source location formatter
// in one of those two classes, or possibly in SMLoc.

/// Get a string with the source location formatted in the standard
/// style, but without the line offset. If \p IncludePath is true, the path
/// is included. If false, only the file name and extension are included.
std::string SourceMgr::getFormattedLocationNoOffset(SMLoc Loc,
                                                    तार्किक IncludePath) ध्रुव {
  auto BufferID = FindBufferContainingLoc(Loc);
  प्रतिज्ञा(BufferID && "Invalid location!");
  auto FileSpec = getBufferInfo(BufferID).Buffer->getBufferIdentifier();

  सचे (IncludePath) {
    पटिनिवत्त FileSpec.str() + ":" + std::to_string(Find_पङ्क्तिः_सङ्ख्या(Loc, BufferID));
  } अञ्ञथा {
    auto I = FileSpec.find_last_of("/\\");
    I = (I == FileSpec.size()) ? 0 : (I + 1);
    पटिनिवत्त FileSpec.substr(I).str() + ":" +
           std::to_string(Find_पङ्क्तिः_सङ्ख्या(Loc, BufferID));
  }
}

/// Given a line and column number in a mapped buffer, turn it into an SMLoc.
/// This will return a null SMLoc if the line/column location is invalid.
SMLoc SourceMgr::Find_Loc_For_पङ्क्तिः_And_Column(unsigned BufferID, unsigned पङ्क्तिः_No,
                                         unsigned ColNo) {
  auto &SB = getBufferInfo(BufferID);
  ध्रुव वर्ण *Ptr = SB.get_Pointer_For_पङ्क्तिः_सङ्ख्या(पङ्क्तिः_No);
  सचे (!Ptr)
    पटिनिवत्त SMLoc();

  // We start counting line and column numbers from 1.
  सचे (ColNo != 0)
    --ColNo;

  // If we have a column number, validate it.
  सचे (ColNo) {
    // Make sure the location is within the current line.
    सचे (Ptr + ColNo > SB.Buffer->getBufferEnd())
      पटिनिवत्त SMLoc();

    // Make sure there is no newline in the way.
    सचे (StringRef(Ptr, ColNo).find_first_of("\n\r") != StringRef::npos)
      पटिनिवत्त SMLoc();

    Ptr += ColNo;
  }

  पटिनिवत्त SMLoc::getFromPointer(Ptr);
}

शून्य SourceMgr::Print_Include_राशिः(SMLoc IncludeLoc, raw_ostream &OS) ध्रुव {
  सचे (IncludeLoc == SMLoc())
    पटिनिवत्त; // Top of stack.

  unsigned CurBuf = FindBufferContainingLoc(IncludeLoc);
  प्रतिज्ञा(CurBuf && "Invalid or unspecified location!");

  Print_Include_राशिः(getBufferInfo(CurBuf).IncludeLoc, OS);

  OS << "Included from " << getBufferInfo(CurBuf).Buffer->getBufferIdentifier()
     << ":" << Find_पङ्क्तिः_सङ्ख्या(IncludeLoc, CurBuf) << ":\n";
}

शून्य SourceMgr::print_Include_राशिः_For_Diagnostic(SMLoc Loc,
                                               raw_ostream &OS) ध्रुव {
  सचे (!Loc.isValid())
    पटिनिवत्त;
  unsigned DiagCurBuffer = FindBufferContainingLoc(Loc);
  सचे (DiagCurBuffer && DiagCurBuffer != getMainFileID()) {
    SMLoc ParentIncludeLoc = getParentIncludeLoc(DiagCurBuffer);
    // Ignore macro instantiation buffers to avoid redundant include stacks.
    Print_Include_राशिः(ParentIncludeLoc, OS);
  }
}

SMDiagnostic SourceMgr::GetMessage(SMLoc Loc, SourceMgr::DiagKind Kind,
                                   ध्रुव Twine &सन्देशः_2, ArrayRef<SMRange> Ranges,
                                   ArrayRef<SMFixIt> FixIts) ध्रुव {
  // First thing to do: find the current buffer containing the specified
  // location to pull out the source line.
  SmallVector<std::pair<unsigned, unsigned>, 4> ColRanges;
  std::pair<unsigned, unsigned> पङ्क्तिः_And_Col;
  StringRef BufferID = "<unknown>";
  StringRef पङ्क्तिः_Str;

  सचे (Loc.isValid()) {
    unsigned CurBuf = FindBufferContainingLoc(Loc);
    प्रतिज्ञा(CurBuf && "Invalid or unspecified location!");

    ध्रुव MemoryBuffer *CurMB = getMemoryBuffer(CurBuf);
    BufferID = CurMB->getBufferIdentifier();

    // Scan backward to find the start of the line.
    ध्रुव वर्ण *पङ्क्तिः_Start = Loc.getPointer();
    ध्रुव वर्ण *BufStart = CurMB->getBufferStart();
    याव (पङ्क्तिः_Start != BufStart && पङ्क्तिः_Start[-1] != '\n' &&
           पङ्क्तिः_Start[-1] != '\r')
      --पङ्क्तिः_Start;

    // Get the end of the line.
    ध्रुव वर्ण *पङ्क्तिः_End = Loc.getPointer();
    ध्रुव वर्ण *BufEnd = CurMB->getBufferEnd();
    याव (पङ्क्तिः_End != BufEnd && पङ्क्तिः_End[0] != '\n' && पङ्क्तिः_End[0] != '\r')
      ++पङ्क्तिः_End;
    पङ्क्तिः_Str = StringRef(पङ्क्तिः_Start, पङ्क्तिः_End - पङ्क्तिः_Start);

    // Convert any ranges to column ranges that only intersect the line of the
    // location.
    अत्थाय (SMRange R : Ranges) {
      सचे (!R.isValid())
        अनुवर्त;

      // If the line doesn't contain any part of the range, then ignore it.
      सचे (R.Start.getPointer() > पङ्क्तिः_End || R.End.getPointer() < पङ्क्तिः_Start)
        अनुवर्त;

      // Ignore pieces of the range that go onto other lines.
      सचे (R.Start.getPointer() < पङ्क्तिः_Start)
        R.Start = SMLoc::getFromPointer(पङ्क्तिः_Start);
      सचे (R.End.getPointer() > पङ्क्तिः_End)
        R.End = SMLoc::getFromPointer(पङ्क्तिः_End);

      // Translate from SMLoc ranges to column ranges.
      // FIXME: Handle multibyte characters.
      ColRanges.प्रक्षिप_back(std::make_pair(R.Start.getPointer() - पङ्क्तिः_Start,
                                         R.End.getPointer() - पङ्क्तिः_Start));
    }

    पङ्क्तिः_And_Col = get_पङ्क्तिः_And_Column(Loc, CurBuf);
  }

  पटिनिवत्त SMDiagnostic(*एतत्, Loc, BufferID, पङ्क्तिः_And_Col.first,
                      पङ्क्तिः_And_Col.second - 1, Kind, सन्देशः_2.str(), पङ्क्तिः_Str,
                      ColRanges, FixIts);
}

शून्य SourceMgr::PrintMessage(raw_ostream &OS, ध्रुव SMDiagnostic &Diagnostic,
                             तार्किक ShowColors) ध्रुव {
  // Report the message with the diagnostic handler if present.
  सचे (DiagHandler) {
    DiagHandler(Diagnostic, DiagContext);
    पटिनिवत्त;
  }

  सचे (Diagnostic.getLoc().isValid()) {
    unsigned CurBuf = FindBufferContainingLoc(Diagnostic.getLoc());
    प्रतिज्ञा(CurBuf && "Invalid or unspecified location!");
    Print_Include_राशिः(getBufferInfo(CurBuf).IncludeLoc, OS);
  }

  Diagnostic.print(nullptr, OS, ShowColors);
}

शून्य SourceMgr::PrintMessage(raw_ostream &OS, SMLoc Loc,
                             SourceMgr::DiagKind Kind, ध्रुव Twine &सन्देशः_2,
                             ArrayRef<SMRange> Ranges, ArrayRef<SMFixIt> FixIts,
                             तार्किक ShowColors) ध्रुव {
  PrintMessage(OS, GetMessage(Loc, Kind, सन्देशः_2, Ranges, FixIts), ShowColors);
}

शून्य SourceMgr::PrintMessage(SMLoc Loc, SourceMgr::DiagKind Kind,
                             ध्रुव Twine &सन्देशः_2, ArrayRef<SMRange> Ranges,
                             ArrayRef<SMFixIt> FixIts, तार्किक ShowColors) ध्रुव {
  PrintMessage(errs(), Loc, Kind, सन्देशः_2, Ranges, FixIts, ShowColors);
}

//===----------------------------------------------------------------------===//
// SMFixIt Implementation
//===----------------------------------------------------------------------===//

SMFixIt::SMFixIt(SMRange R, ध्रुव Twine &Replacement)
    : Range(R), Text(Replacement.str()) {
  प्रतिज्ञा(R.isValid());
}

//===----------------------------------------------------------------------===//
// SMDiagnostic Implementation
//===----------------------------------------------------------------------===//

SMDiagnostic::SMDiagnostic(ध्रुव SourceMgr &sm, SMLoc L, StringRef FN, पूर्णाङ्क पङ्क्तिः,
                           पूर्णाङ्क Col, SourceMgr::DiagKind Kind, StringRef सन्देशः_2,
                           StringRef पङ्क्तिः_Str,
                           ArrayRef<std::pair<unsigned, unsigned>> Ranges,
                           ArrayRef<SMFixIt> Hints)
    : SM(&sm), Loc(L), Filename(std::string(FN)), पङ्क्तिः_No(पङ्क्तिः), ColumnNo(Col),
      Kind(Kind), Message(सन्देशः_2), पङ्क्तिः_Contents(पङ्क्तिः_Str), Ranges(Ranges.vec()),
      FixIts(Hints) {
  llvm::sort(FixIts);
}

स्थिर शून्य build_Fix_It_पङ्क्तिः(std::string &Caret_पङ्क्तिः, std::string &Fix_It_पङ्क्तिः,
                           ArrayRef<SMFixIt> FixIts,
                           ArrayRef<वर्ण> Source_पङ्क्तिः) {
  सचे (FixIts.empty())
    पटिनिवत्त;

  ध्रुव वर्ण *पङ्क्तिः_Start = Source_पङ्क्तिः.begin();
  ध्रुव वर्ण *पङ्क्तिः_End = Source_पङ्क्तिः.end();

  size_t PrevHintEndCol = 0;

  अत्थाय (ध्रुव llvm::SMFixIt &Fixit : FixIts) {
    // If the fixit contains a newline or tab, ignore it.
    सचे (Fixit.getText().find_first_of("\n\r\t") != StringRef::npos)
      अनुवर्त;

    SMRange R = Fixit.getRange();

    // If the line doesn't contain any part of the range, then ignore it.
    सचे (R.Start.getPointer() > पङ्क्तिः_End || R.End.getPointer() < पङ्क्तिः_Start)
      अनुवर्त;

    // Translate from SMLoc to column.
    // Ignore pieces of the range that go onto other lines.
    // FIXME: Handle multibyte characters in the source line.
    unsigned FirstCol;
    सचे (R.Start.getPointer() < पङ्क्तिः_Start)
      FirstCol = 0;
    अञ्ञथा
      FirstCol = R.Start.getPointer() - पङ्क्तिः_Start;

    // If we inserted a long previous hint, push this one forwards, and add
    // an extra space to show that this is not part of the previous
    // completion. This is sort of the best we can do when two hints appear
    // to overlap.
    //
    // Note that if this hint is located immediately after the previous
    // hint, no space will be added, since the location is more important.
    unsigned HintCol = FirstCol;
    सचे (HintCol < PrevHintEndCol)
      HintCol = PrevHintEndCol + 1;

    // FIXME: This assertion is intended to catch unintended use of multibyte
    // characters in fixits. If we decide to do this, we'll have to track
    // separate byte widths for the source and fixit lines.
    प्रतिज्ञा((size_t)sys::locale::columnWidth(Fixit.getText()) ==
           Fixit.getText().size());

    // This relies on one byte per column in our fixit hints.
    unsigned LastColumnModified = HintCol + Fixit.getText().size();
    सचे (LastColumnModified > Fix_It_पङ्क्तिः.size())
      Fix_It_पङ्क्तिः.resize(LastColumnModified, ' ');

    llvm::copy(Fixit.getText(), Fix_It_पङ्क्तिः.begin() + HintCol);

    PrevHintEndCol = LastColumnModified;

    // For replacements, mark the removal range with '~'.
    // FIXME: Handle multibyte characters in the source line.
    unsigned LastCol;
    सचे (R.End.getPointer() >= पङ्क्तिः_End)
      LastCol = पङ्क्तिः_End - पङ्क्तिः_Start;
    अञ्ञथा
      LastCol = R.End.getPointer() - पङ्क्तिः_Start;

    std::fill(&Caret_पङ्क्तिः[FirstCol], &Caret_पङ्क्तिः[LastCol], '~');
  }
}

स्थिर शून्य print_Source_पङ्क्तिः(raw_ostream &S, StringRef पङ्क्तिः_Contents) {
  // Print out the source line one character at a time, so we can expand tabs.
  अत्थाय (unsigned i = 0, e = पङ्क्तिः_Contents.size(), OutCol = 0; i != e; ++i) {
    size_t NextTab = पङ्क्तिः_Contents.find('\t', i);
    // If there were no tabs left, print the rest, we are done.
    सचे (NextTab == StringRef::npos) {
      S << पङ्क्तिः_Contents.drop_front(i);
      भङ्ग;
    }

    // Otherwise, print from i to NextTab.
    S << पङ्क्तिः_Contents.slice(i, NextTab);
    OutCol += NextTab - i;
    i = NextTab;

    // If we have a tab, emit at least one space, then round up to 8 columns.
    करोहि {
      S << ' ';
      ++OutCol;
    } याव ((OutCol % TabStop) != 0);
  }
  S << '\n';
}

स्थिर तार्किक isNonASCII(वर्ण c) { पटिनिवत्त c & 0x80; }

शून्य SMDiagnostic::print(ध्रुव वर्ण *ProgName, raw_ostream &OS, तार्किक ShowColors,
                         तार्किक ShowKindLabel, तार्किक ShowLocation) ध्रुव {
  ColorMode Mode = ShowColors ? ColorMode::Auto : ColorMode::Disable;

  {
    WithColor S(OS, raw_ostream::SAVEDCOLOR, सच्च, मिच्छा, Mode);

    सचे (ProgName && ProgName[0])
      S << ProgName << ": ";

    सचे (ShowLocation && !Filename.empty()) {
      सचे (Filename == "-")
        S << "<stdin>";
      अञ्ञथा
        S << Filename;

      सचे (पङ्क्तिः_No != -1) {
        S << ':' << पङ्क्तिः_No;
        सचे (ColumnNo != -1)
          S << ':' << (ColumnNo + 1);
      }
      S << ": ";
    }
  }

  सचे (ShowKindLabel) {
    विकल्प (Kind) {
    पक्ष SourceMgr::DK_Error:
      WithColor::error(OS, "", !ShowColors);
      भङ्ग;
    पक्ष SourceMgr::DK_Warning:
      WithColor::warning(OS, "", !ShowColors);
      भङ्ग;
    पक्ष SourceMgr::DK_Note:
      WithColor::note(OS, "", !ShowColors);
      भङ्ग;
    पक्ष SourceMgr::DK_Remark:
      WithColor::remark(OS, "", !ShowColors);
      भङ्ग;
    }
  }

  WithColor(OS, raw_ostream::SAVEDCOLOR, सच्च, मिच्छा, Mode) << Message << '\n';

  सचे (पङ्क्तिः_No == -1 || ColumnNo == -1)
    पटिनिवत्त;

  // FIXME: If there are multibyte or multi-column characters in the source, all
  // our ranges will be wrong. To do this properly, we'll need a byte-to-column
  // map like Clang's TextDiagnostic. For now, we'll just handle tabs by
  // expanding them later, and bail out rather than show incorrect ranges and
  // misaligned fixits for any other odd characters.
  सचे (any_of(पङ्क्तिः_Contents, isNonASCII)) {
    print_Source_पङ्क्तिः(OS, पङ्क्तिः_Contents);
    पटिनिवत्त;
  }
  size_t अङ्कः_Columns = पङ्क्तिः_Contents.size();

  // Build the line with the caret and ranges.
  std::string Caret_पङ्क्तिः(अङ्कः_Columns + 1, ' ');

  // Expand any ranges.
  अत्थाय (ध्रुव std::pair<unsigned, unsigned> &R : Ranges)
    std::fill(&Caret_पङ्क्तिः[R.first],
              &Caret_पङ्क्तिः[std::min((size_t)R.second, Caret_पङ्क्तिः.size())], '~');

  // Add any fix-its.
  // FIXME: Find the beginning of the line properly for multibyte characters.
  std::string Fix_It_Insertion_पङ्क्तिः;
  build_Fix_It_पङ्क्तिः(Caret_पङ्क्तिः, Fix_It_Insertion_पङ्क्तिः, FixIts,
                 ArrayRef(Loc.getPointer() - ColumnNo, पङ्क्तिः_Contents.size()));

  // Finally, plop on the caret.
  सचे (unsigned(ColumnNo) <= अङ्कः_Columns)
    Caret_पङ्क्तिः[ColumnNo] = '^';
  अञ्ञथा
    Caret_पङ्क्तिः[अङ्कः_Columns] = '^';

  // ... and remove trailing whitespace so the output doesn't wrap for it.  We
  // know that the line isn't completely empty because it has the caret in it at
  // least.
  Caret_पङ्क्तिः.erase(Caret_पङ्क्तिः.find_last_not_of(' ') + 1);

  print_Source_पङ्क्तिः(OS, पङ्क्तिः_Contents);

  {
    ColorMode Mode = ShowColors ? ColorMode::Auto : ColorMode::Disable;
    WithColor S(OS, raw_ostream::GREEN, सच्च, मिच्छा, Mode);

    // Print out the caret line, matching tabs in the source line.
    अत्थाय (unsigned i = 0, e = Caret_पङ्क्तिः.size(), OutCol = 0; i != e; ++i) {
      सचे (i >= पङ्क्तिः_Contents.size() || पङ्क्तिः_Contents[i] != '\t') {
        S << Caret_पङ्क्तिः[i];
        ++OutCol;
        अनुवर्त;
      }

      // Okay, we have a tab.  Insert the appropriate number of characters.
      करोहि {
        S << Caret_पङ्क्तिः[i];
        ++OutCol;
      } याव ((OutCol % TabStop) != 0);
    }
    S << '\n';
  }

  // Print out the replacement line, matching tabs in the source line.
  सचे (Fix_It_Insertion_पङ्क्तिः.empty())
    पटिनिवत्त;

  अत्थाय (size_t i = 0, e = Fix_It_Insertion_पङ्क्तिः.size(), OutCol = 0; i < e; ++i) {
    सचे (i >= पङ्क्तिः_Contents.size() || पङ्क्तिः_Contents[i] != '\t') {
      OS << Fix_It_Insertion_पङ्क्तिः[i];
      ++OutCol;
      अनुवर्त;
    }

    // Okay, we have a tab.  Insert the appropriate number of characters.
    करोहि {
      OS << Fix_It_Insertion_पङ्क्तिः[i];
      // FIXME: This is trying not to break up replacements, but then to re-sync
      // with the tabs between replacements. This will fail, though, if two
      // fix-it replacements are exactly adjacent, or if a fix-it contains a
      // space. Really we should be precomputing column widths, which we'll
      // need anyway for multibyte chars.
      सचे (Fix_It_Insertion_पङ्क्तिः[i] != ' ')
        ++i;
      ++OutCol;
    } याव (((OutCol % TabStop) != 0) && i != e);
  }
  OS << '\n';
}
