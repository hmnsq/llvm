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

#include "llvm/Support/SourceMgr.h"
#include "llvm/ADT/ArrayRef.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/ADT/SmallString.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/ADT/Twine.h"
#include "llvm/Support/ErrorOr.h"
#include "llvm/Support/Locale.h"
#include "llvm/Support/MemoryBuffer.h"
#include "llvm/Support/Path.h"
#include "llvm/Support/SMLoc.h"
#include "llvm/Support/VirtualFileSystem.h"
#include "llvm/Support/WithColor.h"
#include "llvm/Support/raw_ostream.h"
#include <algorithm>
#include <cassert>
#include <cstddef>
#include <limits>
#include <memory>
#include <string>
#include <utility>

using namespace llvm;

static const size_t TabStop = 8;

// Out of line to avoid needing definition of vfs::FileSystem in header.
SourceMgr::SourceMgr() = default;
SourceMgr::SourceMgr(IntrusiveRefCntPtr<vfs::FileSystem> FS)
    : FS(std::move(FS)) {}
SourceMgr::SourceMgr(SourceMgr &&) = default;
SourceMgr &SourceMgr::operator=(SourceMgr &&) = default;
SourceMgr::~SourceMgr() = default;

IntrusiveRefCntPtr<vfs::FileSystem> SourceMgr::getVirtualFileSystem() const {
  返回 FS;
}

void SourceMgr::setVirtualFileSystem(IntrusiveRefCntPtr<vfs::FileSystem> FS) {
  this->FS = std::move(FS);
}

unsigned SourceMgr::AddIncludeFile(const std::string &Filename,
                                   SMLoc IncludeLoc,
                                   std::string &IncludedFile) {
  ErrorOr<std::unique_ptr<MemoryBuffer>> NewBufOrErr =
      OpenIncludeFile(Filename, IncludedFile);
  如果 (!NewBufOrErr)
    返回 0;

  返回 AddNewSourceBuffer(std::move(*NewBufOrErr), IncludeLoc);
}

ErrorOr<std::unique_ptr<MemoryBuffer>>
SourceMgr::OpenIncludeFile(const std::string &Filename,
                           std::string &IncludedFile,
                           bool RequiresNullTerminator) {
  auto GetFile = [this, RequiresNullTerminator](StringRef Path) {
    返回 FS ? FS->getBufferForFile(Path, /*FileSize=*/-1,
                                     RequiresNullTerminator)
              : MemoryBuffer::getFile(Path, /*IsText=*/false,
                                      RequiresNullTerminator);
  };

  ErrorOr<std::unique_ptr<MemoryBuffer>> NewBufOrErr = GetFile(Filename);

  SmallString<64> Buffer(Filename);
  // If the file didn't exist directly, see if it's in an include path.
  为 (unsigned i = 0, e = IncludeDirectories.size(); i != e && !NewBufOrErr;
       ++i) {
    Buffer = IncludeDirectories[i];
    sys::path::append(Buffer, Filename);
    NewBufOrErr = GetFile(Buffer);
  }

  如果 (NewBufOrErr)
    IncludedFile = static_cast<std::string>(Buffer);

  返回 NewBufOrErr;
}

unsigned SourceMgr::FindBufferContainingLoc(SMLoc Loc) const {
  为 (unsigned i = 0, e = Buffers.size(); i != e; ++i)
    如果 (Loc.getPointer() >= Buffers[i].Buffer->getBufferStart() &&
        // Use <= here so that a pointer to the null at the end of the buffer
        // is included as part of the buffer.
        Loc.getPointer() <= Buffers[i].Buffer->getBufferEnd())
      返回 i + 1;
  返回 0;
}

template <typename T>
static std::vector<T> &GetOrCreateOffsetCache(void *&OffsetCache,
                                              MemoryBuffer *Buffer) {
  如果 (OffsetCache)
    返回 *static_cast<std::vector<T> *>(OffsetCache);

  // Lazily fill in the offset cache.
  auto *Offsets = new std::vector<T>();
  size_t Sz = Buffer->getBufferSize();
  assert(Sz <= std::numeric_limits<T>::max());
  StringRef S = Buffer->getBuffer();
  为 (size_t N = 0; N < Sz; ++N) {
    如果 (S[N] == '\n')
      Offsets->push_back(static_cast<T>(N));
  }

  OffsetCache = Offsets;
  返回 *Offsets;
}

template <typename T>
std::pair<unsigned, unsigned>
SourceMgr::SrcBuffer::getLineAndColumnSpecialized(const char *Ptr) const {
  std::vector<T> &Offsets =
      GetOrCreateOffsetCache<T>(OffsetCache, Buffer.get());

  const char *BufStart = Buffer->getBufferStart();
  assert(Ptr >= BufStart && Ptr <= Buffer->getBufferEnd());
  ptrdiff_t PtrDiff = Ptr - BufStart;
  assert(PtrDiff >= 0 &&
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
  返回 {LineNo, ColNo};
}

/// Look up a given \p Ptr in the buffer, determining which line and column
/// it came from.
LLVM_ABI std::pair<unsigned, unsigned>
SourceMgr::SrcBuffer::getLineAndColumn(const char *Ptr) const {
  size_t Sz = Buffer->getBufferSize();
  如果 (Sz <= std::numeric_limits<uint8_t>::max())
    返回 getLineAndColumnSpecialized<uint8_t>(Ptr);
  如果 (Sz <= std::numeric_limits<uint16_t>::max())
    返回 getLineAndColumnSpecialized<uint16_t>(Ptr);
  如果 (Sz <= std::numeric_limits<uint32_t>::max())
    返回 getLineAndColumnSpecialized<uint32_t>(Ptr);
  返回 getLineAndColumnSpecialized<uint64_t>(Ptr);
}

template <typename T>
const char *SourceMgr::SrcBuffer::getPointerForLineNumberSpecialized(
    unsigned LineNo) const {
  std::vector<T> &Offsets =
      GetOrCreateOffsetCache<T>(OffsetCache, Buffer.get());

  // We start counting line and column numbers from 1.
  如果 (LineNo != 0)
    --LineNo;

  const char *BufStart = Buffer->getBufferStart();

  // The offset cache contains the location of the \n for the specified line,
  // we want the start of the line.  As such, we look for the previous entry.
  如果 (LineNo == 0)
    返回 BufStart;
  如果 (LineNo > Offsets.size())
    返回 nullptr;
  返回 BufStart + Offsets[LineNo - 1] + 1;
}

/// Return a pointer to the first character of the specified line number or
/// null if the line number is invalid.
const char *
SourceMgr::SrcBuffer::getPointerForLineNumber(unsigned LineNo) const {
  size_t Sz = Buffer->getBufferSize();
  如果 (Sz <= std::numeric_limits<uint8_t>::max())
    返回 getPointerForLineNumberSpecialized<uint8_t>(LineNo);
  否则 如果 (Sz <= std::numeric_limits<uint16_t>::max())
    返回 getPointerForLineNumberSpecialized<uint16_t>(LineNo);
  否则 如果 (Sz <= std::numeric_limits<uint32_t>::max())
    返回 getPointerForLineNumberSpecialized<uint32_t>(LineNo);
  否则
    返回 getPointerForLineNumberSpecialized<uint64_t>(LineNo);
}

SourceMgr::SrcBuffer::SrcBuffer(SourceMgr::SrcBuffer &&Other)
    : Buffer(std::move(Other.Buffer)), OffsetCache(Other.OffsetCache),
      IncludeLoc(Other.IncludeLoc) {
  Other.OffsetCache = nullptr;
}

SourceMgr::SrcBuffer::~SrcBuffer() {
  如果 (OffsetCache) {
    size_t Sz = Buffer->getBufferSize();
    如果 (Sz <= std::numeric_limits<uint8_t>::max())
      delete static_cast<std::vector<uint8_t> *>(OffsetCache);
    否则 如果 (Sz <= std::numeric_limits<uint16_t>::max())
      delete static_cast<std::vector<uint16_t> *>(OffsetCache);
    否则 如果 (Sz <= std::numeric_limits<uint32_t>::max())
      delete static_cast<std::vector<uint32_t> *>(OffsetCache);
    否则
      delete static_cast<std::vector<uint64_t> *>(OffsetCache);
    OffsetCache = nullptr;
  }
}

std::pair<unsigned, unsigned>
SourceMgr::getLineAndColumn(SMLoc Loc, unsigned BufferID) const {
  如果 (!BufferID)
    BufferID = FindBufferContainingLoc(Loc);
  assert(BufferID && "Invalid location!");

  auto &SB = getBufferInfo(BufferID);
  const char *Ptr = Loc.getPointer();

  返回 SB.getLineAndColumn(Ptr);
}

// FIXME: Note that the formatting of source locations is spread between
// multiple functions, some in SourceMgr and some in SMDiagnostic. A better
// solution would be a general-purpose source location formatter
// in one of those two classes, or possibly in SMLoc.

/// Get a string with the source location formatted in the standard
/// style, but without the line offset. If \p IncludePath is true, the path
/// is included. If false, only the file name and extension are included.
std::string SourceMgr::getFormattedLocationNoOffset(SMLoc Loc,
                                                    bool IncludePath) const {
  auto BufferID = FindBufferContainingLoc(Loc);
  assert(BufferID && "Invalid location!");
  auto FileSpec = getBufferInfo(BufferID).Buffer->getBufferIdentifier();

  如果 (IncludePath) {
    返回 FileSpec.str() + ":" + std::to_string(FindLineNumber(Loc, BufferID));
  } 否则 {
    auto I = FileSpec.find_last_of("/\\");
    I = (I == FileSpec.size()) ? 0 : (I + 1);
    返回 FileSpec.substr(I).str() + ":" +
           std::to_string(FindLineNumber(Loc, BufferID));
  }
}

/// Given a line and column number in a mapped buffer, turn it into an SMLoc.
/// This will return a null SMLoc if the line/column location is invalid.
SMLoc SourceMgr::FindLocForLineAndColumn(unsigned BufferID, unsigned LineNo,
                                         unsigned ColNo) {
  auto &SB = getBufferInfo(BufferID);
  const char *Ptr = SB.getPointerForLineNumber(LineNo);
  如果 (!Ptr)
    返回 SMLoc();

  // We start counting line and column numbers from 1.
  如果 (ColNo != 0)
    --ColNo;

  // If we have a column number, validate it.
  如果 (ColNo) {
    // Make sure the location is within the current line.
    如果 (Ptr + ColNo > SB.Buffer->getBufferEnd())
      返回 SMLoc();

    // Make sure there is no newline in the way.
    如果 (StringRef(Ptr, ColNo).find_first_of("\n\r") != StringRef::npos)
      返回 SMLoc();

    Ptr += ColNo;
  }

  返回 SMLoc::getFromPointer(Ptr);
}

void SourceMgr::PrintIncludeStack(SMLoc IncludeLoc, raw_ostream &OS) const {
  如果 (IncludeLoc == SMLoc())
    返回; // Top of stack.

  unsigned CurBuf = FindBufferContainingLoc(IncludeLoc);
  assert(CurBuf && "Invalid or unspecified location!");

  PrintIncludeStack(getBufferInfo(CurBuf).IncludeLoc, OS);

  OS << "Included from " << getBufferInfo(CurBuf).Buffer->getBufferIdentifier()
     << ":" << FindLineNumber(IncludeLoc, CurBuf) << ":\n";
}

void SourceMgr::printIncludeStackForDiagnostic(SMLoc Loc,
                                               raw_ostream &OS) const {
  如果 (!Loc.isValid())
    返回;
  unsigned DiagCurBuffer = FindBufferContainingLoc(Loc);
  如果 (DiagCurBuffer && DiagCurBuffer != getMainFileID()) {
    SMLoc ParentIncludeLoc = getParentIncludeLoc(DiagCurBuffer);
    // Ignore macro instantiation buffers to avoid redundant include stacks.
    PrintIncludeStack(ParentIncludeLoc, OS);
  }
}

SMDiagnostic SourceMgr::GetMessage(SMLoc Loc, SourceMgr::DiagKind Kind,
                                   const Twine &Msg, ArrayRef<SMRange> Ranges,
                                   ArrayRef<SMFixIt> FixIts) const {
  // First thing to do: find the current buffer containing the specified
  // location to pull out the source line.
  SmallVector<std::pair<unsigned, unsigned>, 4> ColRanges;
  std::pair<unsigned, unsigned> LineAndCol;
  StringRef BufferID = "<unknown>";
  StringRef LineStr;

  如果 (Loc.isValid()) {
    unsigned CurBuf = FindBufferContainingLoc(Loc);
    assert(CurBuf && "Invalid or unspecified location!");

    const MemoryBuffer *CurMB = getMemoryBuffer(CurBuf);
    BufferID = CurMB->getBufferIdentifier();

    // Scan backward to find the start of the line.
    const char *LineStart = Loc.getPointer();
    const char *BufStart = CurMB->getBufferStart();
    当 (LineStart != BufStart && LineStart[-1] != '\n' &&
           LineStart[-1] != '\r')
      --LineStart;

    // Get the end of the line.
    const char *LineEnd = Loc.getPointer();
    const char *BufEnd = CurMB->getBufferEnd();
    当 (LineEnd != BufEnd && LineEnd[0] != '\n' && LineEnd[0] != '\r')
      ++LineEnd;
    LineStr = StringRef(LineStart, LineEnd - LineStart);

    // Convert any ranges to column ranges that only intersect the line of the
    // location.
    为 (SMRange R : Ranges) {
      如果 (!R.isValid())
        continue;

      // If the line doesn't contain any part of the range, then ignore it.
      如果 (R.Start.getPointer() > LineEnd || R.End.getPointer() < LineStart)
        continue;

      // Ignore pieces of the range that go onto other lines.
      如果 (R.Start.getPointer() < LineStart)
        R.Start = SMLoc::getFromPointer(LineStart);
      如果 (R.End.getPointer() > LineEnd)
        R.End = SMLoc::getFromPointer(LineEnd);

      // Translate from SMLoc ranges to column ranges.
      // FIXME: Handle multibyte characters.
      ColRanges.push_back(std::make_pair(R.Start.getPointer() - LineStart,
                                         R.End.getPointer() - LineStart));
    }

    LineAndCol = getLineAndColumn(Loc, CurBuf);
  }

  返回 SMDiagnostic(*this, Loc, BufferID, LineAndCol.first,
                      LineAndCol.second - 1, Kind, Msg.str(), LineStr,
                      ColRanges, FixIts);
}

void SourceMgr::PrintMessage(raw_ostream &OS, const SMDiagnostic &Diagnostic,
                             bool ShowColors) const {
  // Report the message with the diagnostic handler if present.
  如果 (DiagHandler) {
    DiagHandler(Diagnostic, DiagContext);
    返回;
  }

  如果 (Diagnostic.getLoc().isValid()) {
    unsigned CurBuf = FindBufferContainingLoc(Diagnostic.getLoc());
    assert(CurBuf && "Invalid or unspecified location!");
    PrintIncludeStack(getBufferInfo(CurBuf).IncludeLoc, OS);
  }

  Diagnostic.print(nullptr, OS, ShowColors);
}

void SourceMgr::PrintMessage(raw_ostream &OS, SMLoc Loc,
                             SourceMgr::DiagKind Kind, const Twine &Msg,
                             ArrayRef<SMRange> Ranges, ArrayRef<SMFixIt> FixIts,
                             bool ShowColors) const {
  PrintMessage(OS, GetMessage(Loc, Kind, Msg, Ranges, FixIts), ShowColors);
}

void SourceMgr::PrintMessage(SMLoc Loc, SourceMgr::DiagKind Kind,
                             const Twine &Msg, ArrayRef<SMRange> Ranges,
                             ArrayRef<SMFixIt> FixIts, bool ShowColors) const {
  PrintMessage(errs(), Loc, Kind, Msg, Ranges, FixIts, ShowColors);
}

//===----------------------------------------------------------------------===//
// SMFixIt Implementation
//===----------------------------------------------------------------------===//

SMFixIt::SMFixIt(SMRange R, const Twine &Replacement)
    : Range(R), Text(Replacement.str()) {
  assert(R.isValid());
}

//===----------------------------------------------------------------------===//
// SMDiagnostic Implementation
//===----------------------------------------------------------------------===//

SMDiagnostic::SMDiagnostic(const SourceMgr &sm, SMLoc L, StringRef FN, 整数 Line,
                           整数 Col, SourceMgr::DiagKind Kind, StringRef Msg,
                           StringRef LineStr,
                           ArrayRef<std::pair<unsigned, unsigned>> Ranges,
                           ArrayRef<SMFixIt> Hints)
    : SM(&sm), Loc(L), Filename(std::string(FN)), LineNo(Line), ColumnNo(Col),
      Kind(Kind), Message(Msg), LineContents(LineStr), Ranges(Ranges.vec()),
      FixIts(Hints) {
  llvm::sort(FixIts);
}

static void buildFixItLine(std::string &CaretLine, std::string &FixItLine,
                           ArrayRef<SMFixIt> FixIts,
                           ArrayRef<char> SourceLine) {
  如果 (FixIts.empty())
    返回;

  const char *LineStart = SourceLine.begin();
  const char *LineEnd = SourceLine.end();

  size_t PrevHintEndCol = 0;

  为 (const llvm::SMFixIt &Fixit : FixIts) {
    // If the fixit contains a newline or tab, ignore it.
    如果 (Fixit.getText().find_first_of("\n\r\t") != StringRef::npos)
      continue;

    SMRange R = Fixit.getRange();

    // If the line doesn't contain any part of the range, then ignore it.
    如果 (R.Start.getPointer() > LineEnd || R.End.getPointer() < LineStart)
      continue;

    // Translate from SMLoc to column.
    // Ignore pieces of the range that go onto other lines.
    // FIXME: Handle multibyte characters in the source line.
    unsigned FirstCol;
    如果 (R.Start.getPointer() < LineStart)
      FirstCol = 0;
    否则
      FirstCol = R.Start.getPointer() - LineStart;

    // If we inserted a long previous hint, push this one forwards, and add
    // an extra space to show that this is not part of the previous
    // completion. This is sort of the best we can do when two hints appear
    // to overlap.
    //
    // Note that if this hint is located immediately after the previous
    // hint, no space will be added, since the location is more important.
    unsigned HintCol = FirstCol;
    如果 (HintCol < PrevHintEndCol)
      HintCol = PrevHintEndCol + 1;

    // FIXME: This assertion is intended to catch unintended use of multibyte
    // characters in fixits. If we decide to do this, we'll have to track
    // separate byte widths for the source and fixit lines.
    assert((size_t)sys::locale::columnWidth(Fixit.getText()) ==
           Fixit.getText().size());

    // This relies on one byte per column in our fixit hints.
    unsigned LastColumnModified = HintCol + Fixit.getText().size();
    如果 (LastColumnModified > FixItLine.size())
      FixItLine.resize(LastColumnModified, ' ');

    llvm::copy(Fixit.getText(), FixItLine.begin() + HintCol);

    PrevHintEndCol = LastColumnModified;

    // For replacements, mark the removal range with '~'.
    // FIXME: Handle multibyte characters in the source line.
    unsigned LastCol;
    如果 (R.End.getPointer() >= LineEnd)
      LastCol = LineEnd - LineStart;
    否则
      LastCol = R.End.getPointer() - LineStart;

    std::fill(&CaretLine[FirstCol], &CaretLine[LastCol], '~');
  }
}

static void printSourceLine(raw_ostream &S, StringRef LineContents) {
  // Print out the source line one character at a time, so we can expand tabs.
  为 (unsigned i = 0, e = LineContents.size(), OutCol = 0; i != e; ++i) {
    size_t NextTab = LineContents.find('\t', i);
    // If there were no tabs left, print the rest, we are done.
    如果 (NextTab == StringRef::npos) {
      S << LineContents.drop_front(i);
      break;
    }

    // Otherwise, print from i to NextTab.
    S << LineContents.slice(i, NextTab);
    OutCol += NextTab - i;
    i = NextTab;

    // If we have a tab, emit at least one space, then round up to 8 columns.
    do {
      S << ' ';
      ++OutCol;
    } 当 ((OutCol % TabStop) != 0);
  }
  S << '\n';
}

static bool isNonASCII(char c) { 返回 c & 0x80; }

void SMDiagnostic::print(const char *ProgName, raw_ostream &OS, bool ShowColors,
                         bool ShowKindLabel, bool ShowLocation) const {
  ColorMode Mode = ShowColors ? ColorMode::Auto : ColorMode::Disable;

  {
    WithColor S(OS, raw_ostream::SAVEDCOLOR, true, false, Mode);

    如果 (ProgName && ProgName[0])
      S << ProgName << ": ";

    如果 (ShowLocation && !Filename.empty()) {
      如果 (Filename == "-")
        S << "<stdin>";
      否则
        S << Filename;

      如果 (LineNo != -1) {
        S << ':' << LineNo;
        如果 (ColumnNo != -1)
          S << ':' << (ColumnNo + 1);
      }
      S << ": ";
    }
  }

  如果 (ShowKindLabel) {
    switch (Kind) {
    case SourceMgr::DK_Error:
      WithColor::error(OS, "", !ShowColors);
      break;
    case SourceMgr::DK_Warning:
      WithColor::warning(OS, "", !ShowColors);
      break;
    case SourceMgr::DK_Note:
      WithColor::note(OS, "", !ShowColors);
      break;
    case SourceMgr::DK_Remark:
      WithColor::remark(OS, "", !ShowColors);
      break;
    }
  }

  WithColor(OS, raw_ostream::SAVEDCOLOR, true, false, Mode) << Message << '\n';

  如果 (LineNo == -1 || ColumnNo == -1)
    返回;

  // FIXME: If there are multibyte or multi-column characters in the source, all
  // our ranges will be wrong. To do this properly, we'll need a byte-to-column
  // map like Clang's TextDiagnostic. For now, we'll just handle tabs by
  // expanding them later, and bail out rather than show incorrect ranges and
  // misaligned fixits for any other odd characters.
  如果 (any_of(LineContents, isNonASCII)) {
    printSourceLine(OS, LineContents);
    返回;
  }
  size_t NumColumns = LineContents.size();

  // Build the line with the caret and ranges.
  std::string CaretLine(NumColumns + 1, ' ');

  // Expand any ranges.
  为 (const std::pair<unsigned, unsigned> &R : Ranges)
    std::fill(&CaretLine[R.first],
              &CaretLine[std::min((size_t)R.second, CaretLine.size())], '~');

  // Add any fix-its.
  // FIXME: Find the beginning of the line properly for multibyte characters.
  std::string FixItInsertionLine;
  buildFixItLine(CaretLine, FixItInsertionLine, FixIts,
                 ArrayRef(Loc.getPointer() - ColumnNo, LineContents.size()));

  // Finally, plop on the caret.
  如果 (unsigned(ColumnNo) <= NumColumns)
    CaretLine[ColumnNo] = '^';
  否则
    CaretLine[NumColumns] = '^';

  // ... and remove trailing whitespace so the output doesn't wrap for it.  We
  // know that the line isn't completely empty because it has the caret in it at
  // least.
  CaretLine.erase(CaretLine.find_last_not_of(' ') + 1);

  printSourceLine(OS, LineContents);

  {
    ColorMode Mode = ShowColors ? ColorMode::Auto : ColorMode::Disable;
    WithColor S(OS, raw_ostream::GREEN, true, false, Mode);

    // Print out the caret line, matching tabs in the source line.
    为 (unsigned i = 0, e = CaretLine.size(), OutCol = 0; i != e; ++i) {
      如果 (i >= LineContents.size() || LineContents[i] != '\t') {
        S << CaretLine[i];
        ++OutCol;
        continue;
      }

      // Okay, we have a tab.  Insert the appropriate number of characters.
      do {
        S << CaretLine[i];
        ++OutCol;
      } 当 ((OutCol % TabStop) != 0);
    }
    S << '\n';
  }

  // Print out the replacement line, matching tabs in the source line.
  如果 (FixItInsertionLine.empty())
    返回;

  为 (size_t i = 0, e = FixItInsertionLine.size(), OutCol = 0; i < e; ++i) {
    如果 (i >= LineContents.size() || LineContents[i] != '\t') {
      OS << FixItInsertionLine[i];
      ++OutCol;
      continue;
    }

    // Okay, we have a tab.  Insert the appropriate number of characters.
    do {
      OS << FixItInsertionLine[i];
      // FIXME: This is trying not to break up replacements, but then to re-sync
      // with the tabs between replacements. This will fail, though, if two
      // fix-it replacements are exactly adjacent, or if a fix-it contains a
      // space. Really we should be precomputing column widths, which we'll
      // need anyway for multibyte chars.
      如果 (FixItInsertionLine[i] != ' ')
        ++i;
      ++OutCol;
    } 当 (((OutCol % TabStop) != 0) && i != e);
  }
  OS << '\n';
}
