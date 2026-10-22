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

#ضمن "llvm/Support/SourceMgr.h"
#ضمن "llvm/ADT/ArrayRef.h"
#ضمن "llvm/ADT/STLExtras.h"
#ضمن "llvm/ADT/SmallString.h"
#ضمن "llvm/ADT/SmallVector.h"
#ضمن "llvm/ADT/StringRef.h"
#ضمن "llvm/ADT/Twine.h"
#ضمن "llvm/Support/ErrorOr.h"
#ضمن "llvm/Support/Locale.h"
#ضمن "llvm/Support/MemoryBuffer.h"
#ضمن "llvm/Support/Path.h"
#ضمن "llvm/Support/SMLoc.h"
#ضمن "llvm/Support/VirtualFileSystem.h"
#ضمن "llvm/Support/WithColor.h"
#ضمن "llvm/Support/raw_ostream.h"
#ضمن <algorithm>
#ضمن <cassert>
#ضمن <cstddef>
#ضمن <limits>
#ضمن <memory>
#ضمن <string>
#ضمن <utility>

using نطاق llvm;

ساكن ثابت size_t TabStop = 8;

// Out of line to avoid needing definition of vfs::FileSystem in header.
SourceMgr::SourceMgr() = افتراضي;
SourceMgr::SourceMgr(IntrusiveRefCntPtr<vfs::FileSystem> FS)
    : FS(std::move(FS)) {}
SourceMgr::SourceMgr(SourceMgr &&) = افتراضي;
SourceMgr &SourceMgr::operator=(SourceMgr &&) = افتراضي;
SourceMgr::~SourceMgr() = افتراضي;

IntrusiveRefCntPtr<vfs::FileSystem> SourceMgr::getVirtualFileSystem() ثابت {
  ܦܢܐ FS;
}

فراغ SourceMgr::setVirtualFileSystem(IntrusiveRefCntPtr<vfs::FileSystem> FS) {
  هذا->FS = std::move(FS);
}

unsigned SourceMgr::AddIncludeFile(ثابت std::string &Filename,
                                   SMLoc IncludeLoc,
                                   std::string &IncludedFile) {
  ErrorOr<std::unique_ptr<MemoryBuffer>> NewBufOrErr =
      OpenIncludeFile(Filename, IncludedFile);
  ܐܢ (!NewBufOrErr)
    ܦܢܐ 0;

  ܦܢܐ AddNewSourceBuffer(std::move(*NewBufOrErr), IncludeLoc);
}

ErrorOr<std::unique_ptr<MemoryBuffer>>
SourceMgr::OpenIncludeFile(ثابت std::string &Filename,
                           std::string &IncludedFile,
                           منطقي RequiresNullTerminator) {
  auto GetFile = [هذا, RequiresNullTerminator](StringRef Path) {
    ܦܢܐ FS ? FS->getBufferForFile(Path, /*FileSize=*/-1,
                                     RequiresNullTerminator)
              : MemoryBuffer::getFile(Path, /*IsText=*/ܕܓܠܐ,
                                      RequiresNullTerminator);
  };

  ErrorOr<std::unique_ptr<MemoryBuffer>> NewBufOrErr = GetFile(Filename);

  SmallString<64> Buffer(Filename);
  // If the file didn't exist directly, see if it's in an include path.
  لكل (unsigned i = 0, e = IncludeDirectories.size(); i != e && !NewBufOrErr;
       ++i) {
    Buffer = IncludeDirectories[i];
    sys::path::append(Buffer, Filename);
    NewBufOrErr = GetFile(Buffer);
  }

  ܐܢ (NewBufOrErr)
    IncludedFile = static_cast<std::string>(Buffer);

  ܦܢܐ NewBufOrErr;
}

unsigned SourceMgr::FindBufferContainingLoc(SMLoc Loc) ثابت {
  لكل (unsigned i = 0, e = Buffers.size(); i != e; ++i)
    ܐܢ (Loc.getPointer() >= Buffers[i].Buffer->getBufferStart() &&
        // Use <= here so that a pointer to the null at the end of the buffer
        // is included as part of the buffer.
        Loc.getPointer() <= Buffers[i].Buffer->getBufferEnd())
      ܦܢܐ i + 1;
  ܦܢܐ 0;
}

template <typename T>
ساكن std::vector<T> &GetOrCreateOffsetCache(فراغ *&OffsetCache,
                                              MemoryBuffer *Buffer) {
  ܐܢ (OffsetCache)
    ܦܢܐ *static_cast<std::vector<T> *>(OffsetCache);

  // Lazily fill in the offset cache.
  auto *Offsets = جديد std::vector<T>();
  size_t Sz = Buffer->getBufferSize();
  أكد(Sz <= std::numeric_limits<T>::max());
  StringRef S = Buffer->getBuffer();
  لكل (size_t N = 0; N < Sz; ++N) {
    ܐܢ (S[N] == '\n')
      Offsets->push_back(static_cast<T>(N));
  }

  OffsetCache = Offsets;
  ܦܢܐ *Offsets;
}

template <typename T>
std::pair<unsigned, unsigned>
SourceMgr::SrcBuffer::getLineAndColumnSpecialized(ثابت حرف *Ptr) ثابت {
  std::vector<T> &Offsets =
      GetOrCreateOffsetCache<T>(OffsetCache, Buffer.get());

  ثابت حرف *BufStart = Buffer->getBufferStart();
  أكد(Ptr >= BufStart && Ptr <= Buffer->getBufferEnd());
  ptrdiff_t PtrDiff = Ptr - BufStart;
  أكد(PtrDiff >= 0 &&
         static_cast<size_t>(PtrDiff) <= std::numeric_limits<T>::max());
  T PtrOffset = static_cast<T>(PtrDiff);

  // llvm::lower_bound gives the number of EOL before PtrOffset. Add 1 to get
  // the line number.
  auto I = llvm::lower_bound(Offsets, PtrOffset);
  unsigned LineNo = I - Offsets.begin() + 1;

  // The column number is the distance from the previous EOL (or the start of
  // the buffer) to the pointer.
  T LineStartOffs = (I == Offsets.begin()) ? 0 : (I[-1] + 1);
  unsigned ColNo = PtrOffset - LineStartOffs + 1;
  ܦܢܐ {LineNo, ColNo};
}

/// Look up a given \p Ptr in the buffer, determining which line and column
/// it came from.
LLVM_ABI std::pair<unsigned, unsigned>
SourceMgr::SrcBuffer::getLineAndColumn(ثابت حرف *Ptr) ثابت {
  size_t Sz = Buffer->getBufferSize();
  ܐܢ (Sz <= std::numeric_limits<uint8_t>::max())
    ܦܢܐ getLineAndColumnSpecialized<uint8_t>(Ptr);
  ܐܢ (Sz <= std::numeric_limits<uint16_t>::max())
    ܦܢܐ getLineAndColumnSpecialized<uint16_t>(Ptr);
  ܐܢ (Sz <= std::numeric_limits<uint32_t>::max())
    ܦܢܐ getLineAndColumnSpecialized<uint32_t>(Ptr);
  ܦܢܐ getLineAndColumnSpecialized<uint64_t>(Ptr);
}

template <typename T>
ثابت حرف *SourceMgr::SrcBuffer::getPointerForLineNumberSpecialized(
    unsigned LineNo) ثابت {
  std::vector<T> &Offsets =
      GetOrCreateOffsetCache<T>(OffsetCache, Buffer.get());

  // We start counting line and column numbers from 1.
  ܐܢ (LineNo != 0)
    --LineNo;

  ثابت حرف *BufStart = Buffer->getBufferStart();

  // The offset cache contains the location of the \n for the specified line,
  // we want the start of the line.  As such, we look for the previous entry.
  ܐܢ (LineNo == 0)
    ܦܢܐ BufStart;
  ܐܢ (LineNo > Offsets.size())
    ܦܢܐ nullptr;
  ܦܢܐ BufStart + Offsets[LineNo - 1] + 1;
}

/// Return a pointer to the first character of the specified line number or
/// null if the line number is invalid.
ثابت حرف *
SourceMgr::SrcBuffer::getPointerForLineNumber(unsigned LineNo) ثابت {
  size_t Sz = Buffer->getBufferSize();
  ܐܢ (Sz <= std::numeric_limits<uint8_t>::max())
    ܦܢܐ getPointerForLineNumberSpecialized<uint8_t>(LineNo);
  ܐܚܪܢܐ ܐܢ (Sz <= std::numeric_limits<uint16_t>::max())
    ܦܢܐ getPointerForLineNumberSpecialized<uint16_t>(LineNo);
  ܐܚܪܢܐ ܐܢ (Sz <= std::numeric_limits<uint32_t>::max())
    ܦܢܐ getPointerForLineNumberSpecialized<uint32_t>(LineNo);
  ܐܚܪܢܐ
    ܦܢܐ getPointerForLineNumberSpecialized<uint64_t>(LineNo);
}

SourceMgr::SrcBuffer::SrcBuffer(SourceMgr::SrcBuffer &&Other)
    : Buffer(std::move(Other.Buffer)), OffsetCache(Other.OffsetCache),
      IncludeLoc(Other.IncludeLoc) {
  Other.OffsetCache = nullptr;
}

SourceMgr::SrcBuffer::~SrcBuffer() {
  ܐܢ (OffsetCache) {
    size_t Sz = Buffer->getBufferSize();
    ܐܢ (Sz <= std::numeric_limits<uint8_t>::max())
      delete static_cast<std::vector<uint8_t> *>(OffsetCache);
    ܐܚܪܢܐ ܐܢ (Sz <= std::numeric_limits<uint16_t>::max())
      delete static_cast<std::vector<uint16_t> *>(OffsetCache);
    ܐܚܪܢܐ ܐܢ (Sz <= std::numeric_limits<uint32_t>::max())
      delete static_cast<std::vector<uint32_t> *>(OffsetCache);
    ܐܚܪܢܐ
      delete static_cast<std::vector<uint64_t> *>(OffsetCache);
    OffsetCache = nullptr;
  }
}

std::pair<unsigned, unsigned>
SourceMgr::getLineAndColumn(SMLoc Loc, unsigned BufferID) ثابت {
  ܐܢ (!BufferID)
    BufferID = FindBufferContainingLoc(Loc);
  أكد(BufferID && "Invalid location!");

  auto &SB = getBufferInfo(BufferID);
  ثابت حرف *Ptr = Loc.getPointer();

  ܦܢܐ SB.getLineAndColumn(Ptr);
}

// FIXME: Note that the formatting of source locations is spread between
// multiple functions, some in SourceMgr and some in SMDiagnostic. A better
// solution would be a general-purpose source location formatter
// in one of those two classes, or possibly in SMLoc.

/// Get a string with the source location formatted in the standard
/// style, but without the line offset. If \p IncludePath is true, the path
/// is included. If false, only the file name and extension are included.
std::string SourceMgr::getFormattedLocationNoOffset(SMLoc Loc,
                                                    منطقي IncludePath) ثابت {
  auto BufferID = FindBufferContainingLoc(Loc);
  أكد(BufferID && "Invalid location!");
  auto FileSpec = getBufferInfo(BufferID).Buffer->getBufferIdentifier();

  ܐܢ (IncludePath) {
    ܦܢܐ FileSpec.str() + ":" + std::to_string(FindLineNumber(Loc, BufferID));
  } ܐܚܪܢܐ {
    auto I = FileSpec.find_last_of("/\\");
    I = (I == FileSpec.size()) ? 0 : (I + 1);
    ܦܢܐ FileSpec.substr(I).str() + ":" +
           std::to_string(FindLineNumber(Loc, BufferID));
  }
}

/// Given a line and column number in a mapped buffer, turn it into an SMLoc.
/// This will return a null SMLoc if the line/column location is invalid.
SMLoc SourceMgr::FindLocForLineAndColumn(unsigned BufferID, unsigned LineNo,
                                         unsigned ColNo) {
  auto &SB = getBufferInfo(BufferID);
  ثابت حرف *Ptr = SB.getPointerForLineNumber(LineNo);
  ܐܢ (!Ptr)
    ܦܢܐ SMLoc();

  // We start counting line and column numbers from 1.
  ܐܢ (ColNo != 0)
    --ColNo;

  // If we have a column number, validate it.
  ܐܢ (ColNo) {
    // Make sure the location is within the current line.
    ܐܢ (Ptr + ColNo > SB.Buffer->getBufferEnd())
      ܦܢܐ SMLoc();

    // Make sure there is no newline in the way.
    ܐܢ (StringRef(Ptr, ColNo).find_first_of("\n\r") != StringRef::npos)
      ܦܢܐ SMLoc();

    Ptr += ColNo;
  }

  ܦܢܐ SMLoc::getFromPointer(Ptr);
}

فراغ SourceMgr::PrintIncludeStack(SMLoc IncludeLoc, raw_ostream &OS) ثابت {
  ܐܢ (IncludeLoc == SMLoc())
    ܦܢܐ; // Top of stack.

  unsigned CurBuf = FindBufferContainingLoc(IncludeLoc);
  أكد(CurBuf && "Invalid or unspecified location!");

  PrintIncludeStack(getBufferInfo(CurBuf).IncludeLoc, OS);

  OS << "Included from " << getBufferInfo(CurBuf).Buffer->getBufferIdentifier()
     << ":" << FindLineNumber(IncludeLoc, CurBuf) << ":\n";
}

فراغ SourceMgr::printIncludeStackForDiagnostic(SMLoc Loc,
                                               raw_ostream &OS) ثابت {
  ܐܢ (!Loc.isValid())
    ܦܢܐ;
  unsigned DiagCurBuffer = FindBufferContainingLoc(Loc);
  ܐܢ (DiagCurBuffer && DiagCurBuffer != getMainFileID()) {
    SMLoc ParentIncludeLoc = getParentIncludeLoc(DiagCurBuffer);
    // Ignore macro instantiation buffers to avoid redundant include stacks.
    PrintIncludeStack(ParentIncludeLoc, OS);
  }
}

SMDiagnostic SourceMgr::GetMessage(SMLoc Loc, SourceMgr::DiagKind Kind,
                                   ثابت Twine &Msg, ArrayRef<SMRange> Ranges,
                                   ArrayRef<SMFixIt> FixIts) ثابت {
  // First thing to do: find the current buffer containing the specified
  // location to pull out the source line.
  SmallVector<std::pair<unsigned, unsigned>, 4> ColRanges;
  std::pair<unsigned, unsigned> LineAndCol;
  StringRef BufferID = "<unknown>";
  StringRef LineStr;

  ܐܢ (Loc.isValid()) {
    unsigned CurBuf = FindBufferContainingLoc(Loc);
    أكد(CurBuf && "Invalid or unspecified location!");

    ثابت MemoryBuffer *CurMB = getMemoryBuffer(CurBuf);
    BufferID = CurMB->getBufferIdentifier();

    // Scan backward to find the start of the line.
    ثابت حرف *LineStart = Loc.getPointer();
    ثابت حرف *BufStart = CurMB->getBufferStart();
    ܟܡܐ (LineStart != BufStart && LineStart[-1] != '\n' &&
           LineStart[-1] != '\r')
      --LineStart;

    // Get the end of the line.
    ثابت حرف *LineEnd = Loc.getPointer();
    ثابت حرف *BufEnd = CurMB->getBufferEnd();
    ܟܡܐ (LineEnd != BufEnd && LineEnd[0] != '\n' && LineEnd[0] != '\r')
      ++LineEnd;
    LineStr = StringRef(LineStart, LineEnd - LineStart);

    // Convert any ranges to column ranges that only intersect the line of the
    // location.
    لكل (SMRange R : Ranges) {
      ܐܢ (!R.isValid())
        استمر;

      // If the line doesn't contain any part of the range, then ignore it.
      ܐܢ (R.Start.getPointer() > LineEnd || R.End.getPointer() < LineStart)
        استمر;

      // Ignore pieces of the range that go onto other lines.
      ܐܢ (R.Start.getPointer() < LineStart)
        R.Start = SMLoc::getFromPointer(LineStart);
      ܐܢ (R.End.getPointer() > LineEnd)
        R.End = SMLoc::getFromPointer(LineEnd);

      // Translate from SMLoc ranges to column ranges.
      // FIXME: Handle multibyte characters.
      ColRanges.push_back(std::make_pair(R.Start.getPointer() - LineStart,
                                         R.End.getPointer() - LineStart));
    }

    LineAndCol = getLineAndColumn(Loc, CurBuf);
  }

  ܦܢܐ SMDiagnostic(*هذا, Loc, BufferID, LineAndCol.first,
                      LineAndCol.second - 1, Kind, Msg.str(), LineStr,
                      ColRanges, FixIts);
}

فراغ SourceMgr::PrintMessage(raw_ostream &OS, ثابت SMDiagnostic &Diagnostic,
                             منطقي ShowColors) ثابت {
  // Report the message with the diagnostic handler if present.
  ܐܢ (DiagHandler) {
    DiagHandler(Diagnostic, DiagContext);
    ܦܢܐ;
  }

  ܐܢ (Diagnostic.getLoc().isValid()) {
    unsigned CurBuf = FindBufferContainingLoc(Diagnostic.getLoc());
    أكد(CurBuf && "Invalid or unspecified location!");
    PrintIncludeStack(getBufferInfo(CurBuf).IncludeLoc, OS);
  }

  Diagnostic.print(nullptr, OS, ShowColors);
}

فراغ SourceMgr::PrintMessage(raw_ostream &OS, SMLoc Loc,
                             SourceMgr::DiagKind Kind, ثابت Twine &Msg,
                             ArrayRef<SMRange> Ranges, ArrayRef<SMFixIt> FixIts,
                             منطقي ShowColors) ثابت {
  PrintMessage(OS, GetMessage(Loc, Kind, Msg, Ranges, FixIts), ShowColors);
}

فراغ SourceMgr::PrintMessage(SMLoc Loc, SourceMgr::DiagKind Kind,
                             ثابت Twine &Msg, ArrayRef<SMRange> Ranges,
                             ArrayRef<SMFixIt> FixIts, منطقي ShowColors) ثابت {
  PrintMessage(errs(), Loc, Kind, Msg, Ranges, FixIts, ShowColors);
}

//===----------------------------------------------------------------------===//
// SMFixIt Implementation
//===----------------------------------------------------------------------===//

SMFixIt::SMFixIt(SMRange R, ثابت Twine &Replacement)
    : Range(R), Text(Replacement.str()) {
  أكد(R.isValid());
}

//===----------------------------------------------------------------------===//
// SMDiagnostic Implementation
//===----------------------------------------------------------------------===//

SMDiagnostic::SMDiagnostic(ثابت SourceMgr &sm, SMLoc L, StringRef FN, صحيح Line,
                           صحيح Col, SourceMgr::DiagKind Kind, StringRef Msg,
                           StringRef LineStr,
                           ArrayRef<std::pair<unsigned, unsigned>> Ranges,
                           ArrayRef<SMFixIt> Hints)
    : SM(&sm), Loc(L), Filename(std::string(FN)), LineNo(Line), ColumnNo(Col),
      Kind(Kind), Message(Msg), LineContents(LineStr), Ranges(Ranges.vec()),
      FixIts(Hints) {
  llvm::sort(FixIts);
}

ساكن فراغ buildFixItLine(std::string &CaretLine, std::string &FixItLine,
                           ArrayRef<SMFixIt> FixIts,
                           ArrayRef<حرف> SourceLine) {
  ܐܢ (FixIts.empty())
    ܦܢܐ;

  ثابت حرف *LineStart = SourceLine.begin();
  ثابت حرف *LineEnd = SourceLine.end();

  size_t PrevHintEndCol = 0;

  لكل (ثابت llvm::SMFixIt &Fixit : FixIts) {
    // If the fixit contains a newline or tab, ignore it.
    ܐܢ (Fixit.getText().find_first_of("\n\r\t") != StringRef::npos)
      استمر;

    SMRange R = Fixit.getRange();

    // If the line doesn't contain any part of the range, then ignore it.
    ܐܢ (R.Start.getPointer() > LineEnd || R.End.getPointer() < LineStart)
      استمر;

    // Translate from SMLoc to column.
    // Ignore pieces of the range that go onto other lines.
    // FIXME: Handle multibyte characters in the source line.
    unsigned FirstCol;
    ܐܢ (R.Start.getPointer() < LineStart)
      FirstCol = 0;
    ܐܚܪܢܐ
      FirstCol = R.Start.getPointer() - LineStart;

    // If we inserted a long previous hint, push this one forwards, and add
    // an extra space to show that this is not part of the previous
    // completion. This is sort of the best we can do when two hints appear
    // to overlap.
    //
    // Note that if this hint is located immediately after the previous
    // hint, no space will be added, since the location is more important.
    unsigned HintCol = FirstCol;
    ܐܢ (HintCol < PrevHintEndCol)
      HintCol = PrevHintEndCol + 1;

    // FIXME: This assertion is intended to catch unintended use of multibyte
    // characters in fixits. If we decide to do this, we'll have to track
    // separate byte widths for the source and fixit lines.
    أكد((size_t)sys::locale::columnWidth(Fixit.getText()) ==
           Fixit.getText().size());

    // This relies on one byte per column in our fixit hints.
    unsigned LastColumnModified = HintCol + Fixit.getText().size();
    ܐܢ (LastColumnModified > FixItLine.size())
      FixItLine.resize(LastColumnModified, ' ');

    llvm::copy(Fixit.getText(), FixItLine.begin() + HintCol);

    PrevHintEndCol = LastColumnModified;

    // For replacements, mark the removal range with '~'.
    // FIXME: Handle multibyte characters in the source line.
    unsigned LastCol;
    ܐܢ (R.End.getPointer() >= LineEnd)
      LastCol = LineEnd - LineStart;
    ܐܚܪܢܐ
      LastCol = R.End.getPointer() - LineStart;

    std::fill(&CaretLine[FirstCol], &CaretLine[LastCol], '~');
  }
}

ساكن فراغ printSourceLine(raw_ostream &S, StringRef LineContents) {
  // Print out the source line one character at a time, so we can expand tabs.
  لكل (unsigned i = 0, e = LineContents.size(), OutCol = 0; i != e; ++i) {
    size_t NextTab = LineContents.find('\t', i);
    // If there were no tabs left, print the rest, we are done.
    ܐܢ (NextTab == StringRef::npos) {
      S << LineContents.drop_front(i);
      اكسر;
    }

    // Otherwise, print from i to NextTab.
    S << LineContents.slice(i, NextTab);
    OutCol += NextTab - i;
    i = NextTab;

    // If we have a tab, emit at least one space, then round up to 8 columns.
    ܥܒܕ {
      S << ' ';
      ++OutCol;
    } ܟܡܐ ((OutCol % TabStop) != 0);
  }
  S << '\n';
}

ساكن منطقي isNonASCII(حرف c) { ܦܢܐ c & 0x80; }

فراغ SMDiagnostic::print(ثابت حرف *ProgName, raw_ostream &OS, منطقي ShowColors,
                         منطقي ShowKindLabel, منطقي ShowLocation) ثابت {
  ColorMode Mode = ShowColors ? ColorMode::Auto : ColorMode::Disable;

  {
    WithColor S(OS, raw_ostream::SAVEDCOLOR, ܫܪܪܐ, ܕܓܠܐ, Mode);

    ܐܢ (ProgName && ProgName[0])
      S << ProgName << ": ";

    ܐܢ (ShowLocation && !Filename.empty()) {
      ܐܢ (Filename == "-")
        S << "<stdin>";
      ܐܚܪܢܐ
        S << Filename;

      ܐܢ (LineNo != -1) {
        S << ':' << LineNo;
        ܐܢ (ColumnNo != -1)
          S << ':' << (ColumnNo + 1);
      }
      S << ": ";
    }
  }

  ܐܢ (ShowKindLabel) {
    بدل (Kind) {
    حالة SourceMgr::DK_Error:
      WithColor::error(OS, "", !ShowColors);
      اكسر;
    حالة SourceMgr::DK_Warning:
      WithColor::warning(OS, "", !ShowColors);
      اكسر;
    حالة SourceMgr::DK_Note:
      WithColor::note(OS, "", !ShowColors);
      اكسر;
    حالة SourceMgr::DK_Remark:
      WithColor::remark(OS, "", !ShowColors);
      اكسر;
    }
  }

  WithColor(OS, raw_ostream::SAVEDCOLOR, ܫܪܪܐ, ܕܓܠܐ, Mode) << Message << '\n';

  ܐܢ (LineNo == -1 || ColumnNo == -1)
    ܦܢܐ;

  // FIXME: If there are multibyte or multi-column characters in the source, all
  // our ranges will be wrong. To do this properly, we'll need a byte-to-column
  // map like Clang's TextDiagnostic. For now, we'll just handle tabs by
  // expanding them later, and bail out rather than show incorrect ranges and
  // misaligned fixits for any other odd characters.
  ܐܢ (any_of(LineContents, isNonASCII)) {
    printSourceLine(OS, LineContents);
    ܦܢܐ;
  }
  size_t NumColumns = LineContents.size();

  // Build the line with the caret and ranges.
  std::string CaretLine(NumColumns + 1, ' ');

  // Expand any ranges.
  لكل (ثابت std::pair<unsigned, unsigned> &R : Ranges)
    std::fill(&CaretLine[R.first],
              &CaretLine[std::min((size_t)R.second, CaretLine.size())], '~');

  // Add any fix-its.
  // FIXME: Find the beginning of the line properly for multibyte characters.
  std::string FixItInsertionLine;
  buildFixItLine(CaretLine, FixItInsertionLine, FixIts,
                 ArrayRef(Loc.getPointer() - ColumnNo, LineContents.size()));

  // Finally, plop on the caret.
  ܐܢ (unsigned(ColumnNo) <= NumColumns)
    CaretLine[ColumnNo] = '^';
  ܐܚܪܢܐ
    CaretLine[NumColumns] = '^';

  // ... and remove trailing whitespace so the output doesn't wrap for it.  We
  // know that the line isn't completely empty because it has the caret in it at
  // least.
  CaretLine.erase(CaretLine.find_last_not_of(' ') + 1);

  printSourceLine(OS, LineContents);

  {
    ColorMode Mode = ShowColors ? ColorMode::Auto : ColorMode::Disable;
    WithColor S(OS, raw_ostream::GREEN, ܫܪܪܐ, ܕܓܠܐ, Mode);

    // Print out the caret line, matching tabs in the source line.
    لكل (unsigned i = 0, e = CaretLine.size(), OutCol = 0; i != e; ++i) {
      ܐܢ (i >= LineContents.size() || LineContents[i] != '\t') {
        S << CaretLine[i];
        ++OutCol;
        استمر;
      }

      // Okay, we have a tab.  Insert the appropriate number of characters.
      ܥܒܕ {
        S << CaretLine[i];
        ++OutCol;
      } ܟܡܐ ((OutCol % TabStop) != 0);
    }
    S << '\n';
  }

  // Print out the replacement line, matching tabs in the source line.
  ܐܢ (FixItInsertionLine.empty())
    ܦܢܐ;

  لكل (size_t i = 0, e = FixItInsertionLine.size(), OutCol = 0; i < e; ++i) {
    ܐܢ (i >= LineContents.size() || LineContents[i] != '\t') {
      OS << FixItInsertionLine[i];
      ++OutCol;
      استمر;
    }

    // Okay, we have a tab.  Insert the appropriate number of characters.
    ܥܒܕ {
      OS << FixItInsertionLine[i];
      // FIXME: This is trying not to break up replacements, but then to re-sync
      // with the tabs between replacements. This will fail, though, if two
      // fix-it replacements are exactly adjacent, or if a fix-it contains a
      // space. Really we should be precomputing column widths, which we'll
      // need anyway for multibyte chars.
      ܐܢ (FixItInsertionLine[i] != ' ')
        ++i;
      ++OutCol;
    } ܟܡܐ (((OutCol % TabStop) != 0) && i != e);
  }
  OS << '\n';
}
