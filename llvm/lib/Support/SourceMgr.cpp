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
  أرجع FS;
}

فراغ SourceMgr::setVirtualFileSystem(IntrusiveRefCntPtr<vfs::FileSystem> FS) {
  هذا->FS = std::move(FS);
}

unsigned SourceMgr::AddIncludeFile(ثابت std::string &Filename,
                                   SMLoc IncludeLoc,
                                   std::string &IncludedFile) {
  ErrorOr<std::unique_ptr<MemoryBuffer>> NewBufOrErr =
      OpenIncludeFile(Filename, IncludedFile);
  𐤀𐤌 (!NewBufOrErr)
    أرجع 0;

  أرجع AddNewSourceBuffer(std::move(*NewBufOrErr), IncludeLoc);
}

ErrorOr<std::unique_ptr<MemoryBuffer>>
SourceMgr::OpenIncludeFile(ثابت std::string &Filename,
                           std::string &IncludedFile,
                           منطقي RequiresNullTerminator) {
  auto GetFile = [هذا, RequiresNullTerminator](StringRef Path) {
    أرجع FS ? FS->getBufferForFile(Path, /*FileSize=*/-1,
                                     RequiresNullTerminator)
              : MemoryBuffer::getFile(Path, /*IsText=*/خطأ,
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

  𐤀𐤌 (NewBufOrErr)
    IncludedFile = static_cast<std::string>(Buffer);

  أرجع NewBufOrErr;
}

unsigned SourceMgr::FindBufferContainingLoc(SMLoc Loc) ثابت {
  لكل (unsigned i = 0, e = Buffers.size(); i != e; ++i)
    𐤀𐤌 (Loc.getPointer() >= Buffers[i].Buffer->getBufferStart() &&
        // Use <= here so that a pointer to the null at the end of the buffer
        // is included as part of the buffer.
        Loc.getPointer() <= Buffers[i].Buffer->getBufferEnd())
      أرجع i + 1;
  أرجع 0;
}

template <typename T>
ساكن std::vector<T> &GetOrCreateOffsetCache(فراغ *&OffsetCache,
                                              MemoryBuffer *Buffer) {
  𐤀𐤌 (OffsetCache)
    أرجع *static_cast<std::vector<T> *>(OffsetCache);

  // Lazily fill in the offset cache.
  auto *Offsets = جديد std::vector<T>();
  size_t Sz = Buffer->getBufferSize();
  أكد(Sz <= std::numeric_limits<T>::max());
  StringRef S = Buffer->getBuffer();
  لكل (size_t N = 0; N < Sz; ++N) {
    𐤀𐤌 (S[N] == '\n')
      Offsets->push_back(static_cast<T>(N));
  }

  OffsetCache = Offsets;
  أرجع *Offsets;
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
  أرجع {LineNo, ColNo};
}

/// Look up a given \p Ptr in the buffer, determining which line and column
/// it came from.
LLVM_ABI std::pair<unsigned, unsigned>
SourceMgr::SrcBuffer::getLineAndColumn(ثابت حرف *Ptr) ثابت {
  size_t Sz = Buffer->getBufferSize();
  𐤀𐤌 (Sz <= std::numeric_limits<uint8_t>::max())
    أرجع getLineAndColumnSpecialized<uint8_t>(Ptr);
  𐤀𐤌 (Sz <= std::numeric_limits<uint16_t>::max())
    أرجع getLineAndColumnSpecialized<uint16_t>(Ptr);
  𐤀𐤌 (Sz <= std::numeric_limits<uint32_t>::max())
    أرجع getLineAndColumnSpecialized<uint32_t>(Ptr);
  أرجع getLineAndColumnSpecialized<uint64_t>(Ptr);
}

template <typename T>
ثابت حرف *SourceMgr::SrcBuffer::getPointerForLineNumberSpecialized(
    unsigned LineNo) ثابت {
  std::vector<T> &Offsets =
      GetOrCreateOffsetCache<T>(OffsetCache, Buffer.get());

  // We start counting line and column numbers from 1.
  𐤀𐤌 (LineNo != 0)
    --LineNo;

  ثابت حرف *BufStart = Buffer->getBufferStart();

  // The offset cache contains the location of the \n for the specified line,
  // we want the start of the line.  As such, we look for the previous entry.
  𐤀𐤌 (LineNo == 0)
    أرجع BufStart;
  𐤀𐤌 (LineNo > Offsets.size())
    أرجع nullptr;
  أرجع BufStart + Offsets[LineNo - 1] + 1;
}

/// Return a pointer to the first character of the specified line number or
/// null if the line number is invalid.
ثابت حرف *
SourceMgr::SrcBuffer::getPointerForLineNumber(unsigned LineNo) ثابت {
  size_t Sz = Buffer->getBufferSize();
  𐤀𐤌 (Sz <= std::numeric_limits<uint8_t>::max())
    أرجع getPointerForLineNumberSpecialized<uint8_t>(LineNo);
  وإلا 𐤀𐤌 (Sz <= std::numeric_limits<uint16_t>::max())
    أرجع getPointerForLineNumberSpecialized<uint16_t>(LineNo);
  وإلا 𐤀𐤌 (Sz <= std::numeric_limits<uint32_t>::max())
    أرجع getPointerForLineNumberSpecialized<uint32_t>(LineNo);
  وإلا
    أرجع getPointerForLineNumberSpecialized<uint64_t>(LineNo);
}

SourceMgr::SrcBuffer::SrcBuffer(SourceMgr::SrcBuffer &&Other)
    : Buffer(std::move(Other.Buffer)), OffsetCache(Other.OffsetCache),
      IncludeLoc(Other.IncludeLoc) {
  Other.OffsetCache = nullptr;
}

SourceMgr::SrcBuffer::~SrcBuffer() {
  𐤀𐤌 (OffsetCache) {
    size_t Sz = Buffer->getBufferSize();
    𐤀𐤌 (Sz <= std::numeric_limits<uint8_t>::max())
      delete static_cast<std::vector<uint8_t> *>(OffsetCache);
    وإلا 𐤀𐤌 (Sz <= std::numeric_limits<uint16_t>::max())
      delete static_cast<std::vector<uint16_t> *>(OffsetCache);
    وإلا 𐤀𐤌 (Sz <= std::numeric_limits<uint32_t>::max())
      delete static_cast<std::vector<uint32_t> *>(OffsetCache);
    وإلا
      delete static_cast<std::vector<uint64_t> *>(OffsetCache);
    OffsetCache = nullptr;
  }
}

std::pair<unsigned, unsigned>
SourceMgr::getLineAndColumn(SMLoc Loc, unsigned BufferID) ثابت {
  𐤀𐤌 (!BufferID)
    BufferID = FindBufferContainingLoc(Loc);
  أكد(BufferID && "Invalid location!");

  auto &SB = getBufferInfo(BufferID);
  ثابت حرف *Ptr = Loc.getPointer();

  أرجع SB.getLineAndColumn(Ptr);
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

  𐤀𐤌 (IncludePath) {
    أرجع FileSpec.str() + ":" + std::to_string(FindLineNumber(Loc, BufferID));
  } وإلا {
    auto I = FileSpec.find_last_of("/\\");
    I = (I == FileSpec.size()) ? 0 : (I + 1);
    أرجع FileSpec.substr(I).str() + ":" +
           std::to_string(FindLineNumber(Loc, BufferID));
  }
}

/// Given a line and column number in a mapped buffer, turn it into an SMLoc.
/// This will return a null SMLoc if the line/column location is invalid.
SMLoc SourceMgr::FindLocForLineAndColumn(unsigned BufferID, unsigned LineNo,
                                         unsigned ColNo) {
  auto &SB = getBufferInfo(BufferID);
  ثابت حرف *Ptr = SB.getPointerForLineNumber(LineNo);
  𐤀𐤌 (!Ptr)
    أرجع SMLoc();

  // We start counting line and column numbers from 1.
  𐤀𐤌 (ColNo != 0)
    --ColNo;

  // If we have a column number, validate it.
  𐤀𐤌 (ColNo) {
    // Make sure the location is within the current line.
    𐤀𐤌 (Ptr + ColNo > SB.Buffer->getBufferEnd())
      أرجع SMLoc();

    // Make sure there is no newline in the way.
    𐤀𐤌 (StringRef(Ptr, ColNo).find_first_of("\n\r") != StringRef::npos)
      أرجع SMLoc();

    Ptr += ColNo;
  }

  أرجع SMLoc::getFromPointer(Ptr);
}

فراغ SourceMgr::PrintIncludeStack(SMLoc IncludeLoc, raw_ostream &OS) ثابت {
  𐤀𐤌 (IncludeLoc == SMLoc())
    أرجع; // Top of stack.

  unsigned CurBuf = FindBufferContainingLoc(IncludeLoc);
  أكد(CurBuf && "Invalid or unspecified location!");

  PrintIncludeStack(getBufferInfo(CurBuf).IncludeLoc, OS);

  OS << "Included from " << getBufferInfo(CurBuf).Buffer->getBufferIdentifier()
     << ":" << FindLineNumber(IncludeLoc, CurBuf) << ":\n";
}

فراغ SourceMgr::printIncludeStackForDiagnostic(SMLoc Loc,
                                               raw_ostream &OS) ثابت {
  𐤀𐤌 (!Loc.isValid())
    أرجع;
  unsigned DiagCurBuffer = FindBufferContainingLoc(Loc);
  𐤀𐤌 (DiagCurBuffer && DiagCurBuffer != getMainFileID()) {
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

  𐤀𐤌 (Loc.isValid()) {
    unsigned CurBuf = FindBufferContainingLoc(Loc);
    أكد(CurBuf && "Invalid or unspecified location!");

    ثابت MemoryBuffer *CurMB = getMemoryBuffer(CurBuf);
    BufferID = CurMB->getBufferIdentifier();

    // Scan backward to find the start of the line.
    ثابت حرف *LineStart = Loc.getPointer();
    ثابت حرف *BufStart = CurMB->getBufferStart();
    طالما (LineStart != BufStart && LineStart[-1] != '\n' &&
           LineStart[-1] != '\r')
      --LineStart;

    // Get the end of the line.
    ثابت حرف *LineEnd = Loc.getPointer();
    ثابت حرف *BufEnd = CurMB->getBufferEnd();
    طالما (LineEnd != BufEnd && LineEnd[0] != '\n' && LineEnd[0] != '\r')
      ++LineEnd;
    LineStr = StringRef(LineStart, LineEnd - LineStart);

    // Convert any ranges to column ranges that only intersect the line of the
    // location.
    لكل (SMRange R : Ranges) {
      𐤀𐤌 (!R.isValid())
        استمر;

      // If the line doesn't contain any part of the range, then ignore it.
      𐤀𐤌 (R.Start.getPointer() > LineEnd || R.End.getPointer() < LineStart)
        استمر;

      // Ignore pieces of the range that go onto other lines.
      𐤀𐤌 (R.Start.getPointer() < LineStart)
        R.Start = SMLoc::getFromPointer(LineStart);
      𐤀𐤌 (R.End.getPointer() > LineEnd)
        R.End = SMLoc::getFromPointer(LineEnd);

      // Translate from SMLoc ranges to column ranges.
      // FIXME: Handle multibyte characters.
      ColRanges.push_back(std::make_pair(R.Start.getPointer() - LineStart,
                                         R.End.getPointer() - LineStart));
    }

    LineAndCol = getLineAndColumn(Loc, CurBuf);
  }

  أرجع SMDiagnostic(*هذا, Loc, BufferID, LineAndCol.first,
                      LineAndCol.second - 1, Kind, Msg.str(), LineStr,
                      ColRanges, FixIts);
}

فراغ SourceMgr::PrintMessage(raw_ostream &OS, ثابت SMDiagnostic &Diagnostic,
                             منطقي ShowColors) ثابت {
  // Report the message with the diagnostic handler if present.
  𐤀𐤌 (DiagHandler) {
    DiagHandler(Diagnostic, DiagContext);
    أرجع;
  }

  𐤀𐤌 (Diagnostic.getLoc().isValid()) {
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
  𐤀𐤌 (FixIts.empty())
    أرجع;

  ثابت حرف *LineStart = SourceLine.begin();
  ثابت حرف *LineEnd = SourceLine.end();

  size_t PrevHintEndCol = 0;

  لكل (ثابت llvm::SMFixIt &Fixit : FixIts) {
    // If the fixit contains a newline or tab, ignore it.
    𐤀𐤌 (Fixit.getText().find_first_of("\n\r\t") != StringRef::npos)
      استمر;

    SMRange R = Fixit.getRange();

    // If the line doesn't contain any part of the range, then ignore it.
    𐤀𐤌 (R.Start.getPointer() > LineEnd || R.End.getPointer() < LineStart)
      استمر;

    // Translate from SMLoc to column.
    // Ignore pieces of the range that go onto other lines.
    // FIXME: Handle multibyte characters in the source line.
    unsigned FirstCol;
    𐤀𐤌 (R.Start.getPointer() < LineStart)
      FirstCol = 0;
    وإلا
      FirstCol = R.Start.getPointer() - LineStart;

    // If we inserted a long previous hint, push this one forwards, and add
    // an extra space to show that this is not part of the previous
    // completion. This is sort of the best we can do when two hints appear
    // to overlap.
    //
    // Note that if this hint is located immediately after the previous
    // hint, no space will be added, since the location is more important.
    unsigned HintCol = FirstCol;
    𐤀𐤌 (HintCol < PrevHintEndCol)
      HintCol = PrevHintEndCol + 1;

    // FIXME: This assertion is intended to catch unintended use of multibyte
    // characters in fixits. If we decide to do this, we'll have to track
    // separate byte widths for the source and fixit lines.
    أكد((size_t)sys::locale::columnWidth(Fixit.getText()) ==
           Fixit.getText().size());

    // This relies on one byte per column in our fixit hints.
    unsigned LastColumnModified = HintCol + Fixit.getText().size();
    𐤀𐤌 (LastColumnModified > FixItLine.size())
      FixItLine.resize(LastColumnModified, ' ');

    llvm::copy(Fixit.getText(), FixItLine.begin() + HintCol);

    PrevHintEndCol = LastColumnModified;

    // For replacements, mark the removal range with '~'.
    // FIXME: Handle multibyte characters in the source line.
    unsigned LastCol;
    𐤀𐤌 (R.End.getPointer() >= LineEnd)
      LastCol = LineEnd - LineStart;
    وإلا
      LastCol = R.End.getPointer() - LineStart;

    std::fill(&CaretLine[FirstCol], &CaretLine[LastCol], '~');
  }
}

ساكن فراغ printSourceLine(raw_ostream &S, StringRef LineContents) {
  // Print out the source line one character at a time, so we can expand tabs.
  لكل (unsigned i = 0, e = LineContents.size(), OutCol = 0; i != e; ++i) {
    size_t NextTab = LineContents.find('\t', i);
    // If there were no tabs left, print the rest, we are done.
    𐤀𐤌 (NextTab == StringRef::npos) {
      S << LineContents.drop_front(i);
      اكسر;
    }

    // Otherwise, print from i to NextTab.
    S << LineContents.slice(i, NextTab);
    OutCol += NextTab - i;
    i = NextTab;

    // If we have a tab, emit at least one space, then round up to 8 columns.
    𐤐𐤏𐤋 {
      S << ' ';
      ++OutCol;
    } طالما ((OutCol % TabStop) != 0);
  }
  S << '\n';
}

ساكن منطقي isNonASCII(حرف c) { أرجع c & 0x80; }

فراغ SMDiagnostic::print(ثابت حرف *ProgName, raw_ostream &OS, منطقي ShowColors,
                         منطقي ShowKindLabel, منطقي ShowLocation) ثابت {
  ColorMode Mode = ShowColors ? ColorMode::Auto : ColorMode::Disable;

  {
    WithColor S(OS, raw_ostream::SAVEDCOLOR, true, خطأ, Mode);

    𐤀𐤌 (ProgName && ProgName[0])
      S << ProgName << ": ";

    𐤀𐤌 (ShowLocation && !Filename.empty()) {
      𐤀𐤌 (Filename == "-")
        S << "<stdin>";
      وإلا
        S << Filename;

      𐤀𐤌 (LineNo != -1) {
        S << ':' << LineNo;
        𐤀𐤌 (ColumnNo != -1)
          S << ':' << (ColumnNo + 1);
      }
      S << ": ";
    }
  }

  𐤀𐤌 (ShowKindLabel) {
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

  WithColor(OS, raw_ostream::SAVEDCOLOR, true, خطأ, Mode) << Message << '\n';

  𐤀𐤌 (LineNo == -1 || ColumnNo == -1)
    أرجع;

  // FIXME: If there are multibyte or multi-column characters in the source, all
  // our ranges will be wrong. To do this properly, we'll need a byte-to-column
  // map like Clang's TextDiagnostic. For now, we'll just handle tabs by
  // expanding them later, and bail out rather than show incorrect ranges and
  // misaligned fixits for any other odd characters.
  𐤀𐤌 (any_of(LineContents, isNonASCII)) {
    printSourceLine(OS, LineContents);
    أرجع;
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
  𐤀𐤌 (unsigned(ColumnNo) <= NumColumns)
    CaretLine[ColumnNo] = '^';
  وإلا
    CaretLine[NumColumns] = '^';

  // ... and remove trailing whitespace so the output doesn't wrap for it.  We
  // know that the line isn't completely empty because it has the caret in it at
  // least.
  CaretLine.erase(CaretLine.find_last_not_of(' ') + 1);

  printSourceLine(OS, LineContents);

  {
    ColorMode Mode = ShowColors ? ColorMode::Auto : ColorMode::Disable;
    WithColor S(OS, raw_ostream::GREEN, true, خطأ, Mode);

    // Print out the caret line, matching tabs in the source line.
    لكل (unsigned i = 0, e = CaretLine.size(), OutCol = 0; i != e; ++i) {
      𐤀𐤌 (i >= LineContents.size() || LineContents[i] != '\t') {
        S << CaretLine[i];
        ++OutCol;
        استمر;
      }

      // Okay, we have a tab.  Insert the appropriate number of characters.
      𐤐𐤏𐤋 {
        S << CaretLine[i];
        ++OutCol;
      } طالما ((OutCol % TabStop) != 0);
    }
    S << '\n';
  }

  // Print out the replacement line, matching tabs in the source line.
  𐤀𐤌 (FixItInsertionLine.empty())
    أرجع;

  لكل (size_t i = 0, e = FixItInsertionLine.size(), OutCol = 0; i < e; ++i) {
    𐤀𐤌 (i >= LineContents.size() || LineContents[i] != '\t') {
      OS << FixItInsertionLine[i];
      ++OutCol;
      استمر;
    }

    // Okay, we have a tab.  Insert the appropriate number of characters.
    𐤐𐤏𐤋 {
      OS << FixItInsertionLine[i];
      // FIXME: This is trying not to break up replacements, but then to re-sync
      // with the tabs between replacements. This will fail, though, if two
      // fix-it replacements are exactly adjacent, or if a fix-it contains a
      // space. Really we should be precomputing column widths, which we'll
      // need anyway for multibyte chars.
      𐤀𐤌 (FixItInsertionLine[i] != ' ')
        ++i;
      ++OutCol;
    } طالما (((OutCol % TabStop) != 0) && i != e);
  }
  OS << '\n';
}
