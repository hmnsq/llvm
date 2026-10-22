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

#અન્તર્ભાવ "llvm/Support/SourceMgr.h"
#અન્તર્ભાવ "llvm/ADT/ArrayRef.h"
#અન્તર્ભાવ "llvm/ADT/STLExtras.h"
#અન્તર્ભાવ "llvm/ADT/SmallString.h"
#અન્તર્ભાવ "llvm/ADT/SmallVector.h"
#અન્તર્ભાવ "llvm/ADT/StringRef.h"
#અન્તર્ભાવ "llvm/ADT/Twine.h"
#અન્તર્ભાવ "llvm/Support/ErrorOr.h"
#અન્તર્ભાવ "llvm/Support/Locale.h"
#અન્તર્ભાવ "llvm/Support/MemoryBuffer.h"
#અન્તર્ભાવ "llvm/Support/Path.h"
#અન્તર્ભાવ "llvm/Support/SMLoc.h"
#અન્તર્ભાવ "llvm/Support/VirtualFileSystem.h"
#અન્તર્ભાવ "llvm/Support/WithColor.h"
#અન્તર્ભાવ "llvm/Support/raw_ostream.h"
#અન્તર્ભાવ <algorithm>
#અન્તર્ભાવ <cassert>
#અન્તર્ભાવ <cstddef>
#અન્તર્ભાવ <limits>
#અન્તર્ભાવ <memory>
#અન્તર્ભાવ <string>
#અન્તર્ભાવ <utility>

using નામાકાશ llvm;

સ્થિર અચલ size_t TabStop = 8;

// Out of line to avoid needing definition of vfs::FileSystem in header.
SourceMgr::SourceMgr() = મૂળભૂત;
SourceMgr::SourceMgr(IntrusiveRefCntPtr<vfs::FileSystem> FS)
    : FS(std::move(FS)) {}
SourceMgr::SourceMgr(SourceMgr &&) = મૂળભૂત;
SourceMgr &SourceMgr::operator=(SourceMgr &&) = મૂળભૂત;
SourceMgr::~SourceMgr() = મૂળભૂત;

IntrusiveRefCntPtr<vfs::FileSystem> SourceMgr::getVirtualFileSystem() અચલ {
  પાછા FS;
}

શૂન્ય SourceMgr::setVirtualFileSystem(IntrusiveRefCntPtr<vfs::FileSystem> FS) {
  આ->FS = std::move(FS);
}

unsigned SourceMgr::AddIncludeFile(અચલ std::string &Filename,
                                   SMLoc IncludeLoc,
                                   std::string &IncludedFile) {
  ErrorOr<std::unique_ptr<MemoryBuffer>> NewBufOrErr =
      OpenIncludeFile(Filename, IncludedFile);
  જો (!NewBufOrErr)
    પાછા 0;

  પાછા AddNewSourceBuffer(std::move(*NewBufOrErr), IncludeLoc);
}

ErrorOr<std::unique_ptr<MemoryBuffer>>
SourceMgr::OpenIncludeFile(અચલ std::string &Filename,
                           std::string &IncludedFile,
                           તાર્કિક RequiresNullTerminator) {
  auto GetFile = [આ, RequiresNullTerminator](StringRef Path) {
    પાછા FS ? FS->getBufferForFile(Path, /*FileSize=*/-1,
                                     RequiresNullTerminator)
              : MemoryBuffer::getFile(Path, /*IsText=*/ખોટું,
                                      RequiresNullTerminator);
  };

  ErrorOr<std::unique_ptr<MemoryBuffer>> NewBufOrErr = GetFile(Filename);

  SmallString<64> Buffer(Filename);
  // If the file didn't exist directly, see if it's in an include path.
  માટે (unsigned i = 0, e = IncludeDirectories.size(); i != e && !NewBufOrErr;
       ++i) {
    Buffer = IncludeDirectories[i];
    sys::path::append(Buffer, Filename);
    NewBufOrErr = GetFile(Buffer);
  }

  જો (NewBufOrErr)
    IncludedFile = static_cast<std::string>(Buffer);

  પાછા NewBufOrErr;
}

unsigned SourceMgr::FindBufferContainingLoc(SMLoc Loc) અચલ {
  માટે (unsigned i = 0, e = Buffers.size(); i != e; ++i)
    જો (Loc.getPointer() >= Buffers[i].Buffer->getBufferStart() &&
        // Use <= here so that a pointer to the null at the end of the buffer
        // is included as part of the buffer.
        Loc.getPointer() <= Buffers[i].Buffer->getBufferEnd())
      પાછા i + 1;
  પાછા 0;
}

template <typename T>
સ્થિર std::vector<T> &GetOrCreateOffsetCache(શૂન્ય *&OffsetCache,
                                              MemoryBuffer *Buffer) {
  જો (OffsetCache)
    પાછા *static_cast<std::vector<T> *>(OffsetCache);

  // Lazily fill in the offset cache.
  auto *Offsets = નવું std::vector<T>();
  size_t Sz = Buffer->getBufferSize();
  પ્રતિજ્ઞા(Sz <= std::numeric_limits<T>::max());
  StringRef S = Buffer->getBuffer();
  માટે (size_t N = 0; N < Sz; ++N) {
    જો (S[N] == '\n')
      Offsets->ધકેલો_back(static_cast<T>(N));
  }

  OffsetCache = Offsets;
  પાછા *Offsets;
}

template <typename T>
std::pair<unsigned, unsigned>
SourceMgr::SrcBuffer::get_લીટી_And_Column_Specialized(અચલ અક્ષર *Ptr) અચલ {
  std::vector<T> &Offsets =
      GetOrCreateOffsetCache<T>(OffsetCache, Buffer.get());

  અચલ અક્ષર *BufStart = Buffer->getBufferStart();
  પ્રતિજ્ઞા(Ptr >= BufStart && Ptr <= Buffer->getBufferEnd());
  ptrdiff_t PtrDiff = Ptr - BufStart;
  પ્રતિજ્ઞા(PtrDiff >= 0 &&
         static_cast<size_t>(PtrDiff) <= std::numeric_limits<T>::max());
  T PtrOffset = static_cast<T>(PtrDiff);

  // llvm::lower_bound gives the number of EOL before PtrOffset. Add 1 to get
  // the line number.
  auto I = llvm::lower_bound(Offsets, PtrOffset);
  unsigned લીટી_No = I - Offsets.begin() + 1;

  // The column number is the distance from the previous EOL (or the start of
  // the buffer) to the pointer.
  T લીટી_Start_Offs = (I == Offsets.begin()) ? 0 : (I[-1] + 1);
  unsigned ColNo = PtrOffset - લીટી_Start_Offs + 1;
  પાછા {લીટી_No, ColNo};
}

/// Look up a given \p Ptr in the buffer, determining which line and column
/// it came from.
LLVM_ABI std::pair<unsigned, unsigned>
SourceMgr::SrcBuffer::get_લીટી_And_Column(અચલ અક્ષર *Ptr) અચલ {
  size_t Sz = Buffer->getBufferSize();
  જો (Sz <= std::numeric_limits<uint8_t>::max())
    પાછા get_લીટી_And_Column_Specialized<uint8_t>(Ptr);
  જો (Sz <= std::numeric_limits<uint16_t>::max())
    પાછા get_લીટી_And_Column_Specialized<uint16_t>(Ptr);
  જો (Sz <= std::numeric_limits<uint32_t>::max())
    પાછા get_લીટી_And_Column_Specialized<uint32_t>(Ptr);
  પાછા get_લીટી_And_Column_Specialized<uint64_t>(Ptr);
}

template <typename T>
અચલ અક્ષર *SourceMgr::SrcBuffer::get_Pointer_For_લીટી_સંખ્યા_Specialized(
    unsigned લીટી_No) અચલ {
  std::vector<T> &Offsets =
      GetOrCreateOffsetCache<T>(OffsetCache, Buffer.get());

  // We start counting line and column numbers from 1.
  જો (લીટી_No != 0)
    --લીટી_No;

  અચલ અક્ષર *BufStart = Buffer->getBufferStart();

  // The offset cache contains the location of the \n for the specified line,
  // we want the start of the line.  As such, we look for the previous entry.
  જો (લીટી_No == 0)
    પાછા BufStart;
  જો (લીટી_No > Offsets.size())
    પાછા nullptr;
  પાછા BufStart + Offsets[લીટી_No - 1] + 1;
}

/// Return a pointer to the first character of the specified line number or
/// null if the line number is invalid.
અચલ અક્ષર *
SourceMgr::SrcBuffer::get_Pointer_For_લીટી_સંખ્યા(unsigned લીટી_No) અચલ {
  size_t Sz = Buffer->getBufferSize();
  જો (Sz <= std::numeric_limits<uint8_t>::max())
    પાછા get_Pointer_For_લીટી_સંખ્યા_Specialized<uint8_t>(લીટી_No);
  નહીંતર જો (Sz <= std::numeric_limits<uint16_t>::max())
    પાછા get_Pointer_For_લીટી_સંખ્યા_Specialized<uint16_t>(લીટી_No);
  નહીંતર જો (Sz <= std::numeric_limits<uint32_t>::max())
    પાછા get_Pointer_For_લીટી_સંખ્યા_Specialized<uint32_t>(લીટી_No);
  નહીંતર
    પાછા get_Pointer_For_લીટી_સંખ્યા_Specialized<uint64_t>(લીટી_No);
}

SourceMgr::SrcBuffer::SrcBuffer(SourceMgr::SrcBuffer &&Other)
    : Buffer(std::move(Other.Buffer)), OffsetCache(Other.OffsetCache),
      IncludeLoc(Other.IncludeLoc) {
  Other.OffsetCache = nullptr;
}

SourceMgr::SrcBuffer::~SrcBuffer() {
  જો (OffsetCache) {
    size_t Sz = Buffer->getBufferSize();
    જો (Sz <= std::numeric_limits<uint8_t>::max())
      delete static_cast<std::vector<uint8_t> *>(OffsetCache);
    નહીંતર જો (Sz <= std::numeric_limits<uint16_t>::max())
      delete static_cast<std::vector<uint16_t> *>(OffsetCache);
    નહીંતર જો (Sz <= std::numeric_limits<uint32_t>::max())
      delete static_cast<std::vector<uint32_t> *>(OffsetCache);
    નહીંતર
      delete static_cast<std::vector<uint64_t> *>(OffsetCache);
    OffsetCache = nullptr;
  }
}

std::pair<unsigned, unsigned>
SourceMgr::get_લીટી_And_Column(SMLoc Loc, unsigned BufferID) અચલ {
  જો (!BufferID)
    BufferID = FindBufferContainingLoc(Loc);
  પ્રતિજ્ઞા(BufferID && "Invalid location!");

  auto &SB = getBufferInfo(BufferID);
  અચલ અક્ષર *Ptr = Loc.getPointer();

  પાછા SB.get_લીટી_And_Column(Ptr);
}

// FIXME: Note that the formatting of source locations is spread between
// multiple functions, some in SourceMgr and some in SMDiagnostic. A better
// solution would be a general-purpose source location formatter
// in one of those two classes, or possibly in SMLoc.

/// Get a string with the source location formatted in the standard
/// style, but without the line offset. If \p IncludePath is true, the path
/// is included. If false, only the file name and extension are included.
std::string SourceMgr::getFormattedLocationNoOffset(SMLoc Loc,
                                                    તાર્કિક IncludePath) અચલ {
  auto BufferID = FindBufferContainingLoc(Loc);
  પ્રતિજ્ઞા(BufferID && "Invalid location!");
  auto FileSpec = getBufferInfo(BufferID).Buffer->getBufferIdentifier();

  જો (IncludePath) {
    પાછા FileSpec.str() + ":" + std::to_string(Find_લીટી_સંખ્યા(Loc, BufferID));
  } નહીંતર {
    auto I = FileSpec.find_last_of("/\\");
    I = (I == FileSpec.size()) ? 0 : (I + 1);
    પાછા FileSpec.substr(I).str() + ":" +
           std::to_string(Find_લીટી_સંખ્યા(Loc, BufferID));
  }
}

/// Given a line and column number in a mapped buffer, turn it into an SMLoc.
/// This will return a null SMLoc if the line/column location is invalid.
SMLoc SourceMgr::Find_Loc_For_લીટી_And_Column(unsigned BufferID, unsigned લીટી_No,
                                         unsigned ColNo) {
  auto &SB = getBufferInfo(BufferID);
  અચલ અક્ષર *Ptr = SB.get_Pointer_For_લીટી_સંખ્યા(લીટી_No);
  જો (!Ptr)
    પાછા SMLoc();

  // We start counting line and column numbers from 1.
  જો (ColNo != 0)
    --ColNo;

  // If we have a column number, validate it.
  જો (ColNo) {
    // Make sure the location is within the current line.
    જો (Ptr + ColNo > SB.Buffer->getBufferEnd())
      પાછા SMLoc();

    // Make sure there is no newline in the way.
    જો (StringRef(Ptr, ColNo).find_first_of("\n\r") != StringRef::npos)
      પાછા SMLoc();

    Ptr += ColNo;
  }

  પાછા SMLoc::getFromPointer(Ptr);
}

શૂન્ય SourceMgr::Print_Include_ઢગલો(SMLoc IncludeLoc, raw_ostream &OS) અચલ {
  જો (IncludeLoc == SMLoc())
    પાછા; // Top of stack.

  unsigned CurBuf = FindBufferContainingLoc(IncludeLoc);
  પ્રતિજ્ઞા(CurBuf && "Invalid or unspecified location!");

  Print_Include_ઢગલો(getBufferInfo(CurBuf).IncludeLoc, OS);

  OS << "Included from " << getBufferInfo(CurBuf).Buffer->getBufferIdentifier()
     << ":" << Find_લીટી_સંખ્યા(IncludeLoc, CurBuf) << ":\n";
}

શૂન્ય SourceMgr::print_Include_ઢગલો_For_Diagnostic(SMLoc Loc,
                                               raw_ostream &OS) અચલ {
  જો (!Loc.isValid())
    પાછા;
  unsigned DiagCurBuffer = FindBufferContainingLoc(Loc);
  જો (DiagCurBuffer && DiagCurBuffer != getMainFileID()) {
    SMLoc ParentIncludeLoc = getParentIncludeLoc(DiagCurBuffer);
    // Ignore macro instantiation buffers to avoid redundant include stacks.
    Print_Include_ઢગલો(ParentIncludeLoc, OS);
  }
}

SMDiagnostic SourceMgr::GetMessage(SMLoc Loc, SourceMgr::DiagKind Kind,
                                   અચલ Twine &સંદેશ_2, ArrayRef<SMRange> Ranges,
                                   ArrayRef<SMFixIt> FixIts) અચલ {
  // First thing to do: find the current buffer containing the specified
  // location to pull out the source line.
  SmallVector<std::pair<unsigned, unsigned>, 4> ColRanges;
  std::pair<unsigned, unsigned> લીટી_And_Col;
  StringRef BufferID = "<unknown>";
  StringRef લીટી_Str;

  જો (Loc.isValid()) {
    unsigned CurBuf = FindBufferContainingLoc(Loc);
    પ્રતિજ્ઞા(CurBuf && "Invalid or unspecified location!");

    અચલ MemoryBuffer *CurMB = getMemoryBuffer(CurBuf);
    BufferID = CurMB->getBufferIdentifier();

    // Scan backward to find the start of the line.
    અચલ અક્ષર *લીટી_Start = Loc.getPointer();
    અચલ અક્ષર *BufStart = CurMB->getBufferStart();
    જ્યારે (લીટી_Start != BufStart && લીટી_Start[-1] != '\n' &&
           લીટી_Start[-1] != '\r')
      --લીટી_Start;

    // Get the end of the line.
    અચલ અક્ષર *લીટી_End = Loc.getPointer();
    અચલ અક્ષર *BufEnd = CurMB->getBufferEnd();
    જ્યારે (લીટી_End != BufEnd && લીટી_End[0] != '\n' && લીટી_End[0] != '\r')
      ++લીટી_End;
    લીટી_Str = StringRef(લીટી_Start, લીટી_End - લીટી_Start);

    // Convert any ranges to column ranges that only intersect the line of the
    // location.
    માટે (SMRange R : Ranges) {
      જો (!R.isValid())
        ચાલુ;

      // If the line doesn't contain any part of the range, then ignore it.
      જો (R.Start.getPointer() > લીટી_End || R.End.getPointer() < લીટી_Start)
        ચાલુ;

      // Ignore pieces of the range that go onto other lines.
      જો (R.Start.getPointer() < લીટી_Start)
        R.Start = SMLoc::getFromPointer(લીટી_Start);
      જો (R.End.getPointer() > લીટી_End)
        R.End = SMLoc::getFromPointer(લીટી_End);

      // Translate from SMLoc ranges to column ranges.
      // FIXME: Handle multibyte characters.
      ColRanges.ધકેલો_back(std::make_pair(R.Start.getPointer() - લીટી_Start,
                                         R.End.getPointer() - લીટી_Start));
    }

    લીટી_And_Col = get_લીટી_And_Column(Loc, CurBuf);
  }

  પાછા SMDiagnostic(*આ, Loc, BufferID, લીટી_And_Col.first,
                      લીટી_And_Col.second - 1, Kind, સંદેશ_2.str(), લીટી_Str,
                      ColRanges, FixIts);
}

શૂન્ય SourceMgr::PrintMessage(raw_ostream &OS, અચલ SMDiagnostic &Diagnostic,
                             તાર્કિક ShowColors) અચલ {
  // Report the message with the diagnostic handler if present.
  જો (DiagHandler) {
    DiagHandler(Diagnostic, DiagContext);
    પાછા;
  }

  જો (Diagnostic.getLoc().isValid()) {
    unsigned CurBuf = FindBufferContainingLoc(Diagnostic.getLoc());
    પ્રતિજ્ઞા(CurBuf && "Invalid or unspecified location!");
    Print_Include_ઢગલો(getBufferInfo(CurBuf).IncludeLoc, OS);
  }

  Diagnostic.print(nullptr, OS, ShowColors);
}

શૂન્ય SourceMgr::PrintMessage(raw_ostream &OS, SMLoc Loc,
                             SourceMgr::DiagKind Kind, અચલ Twine &સંદેશ_2,
                             ArrayRef<SMRange> Ranges, ArrayRef<SMFixIt> FixIts,
                             તાર્કિક ShowColors) અચલ {
  PrintMessage(OS, GetMessage(Loc, Kind, સંદેશ_2, Ranges, FixIts), ShowColors);
}

શૂન્ય SourceMgr::PrintMessage(SMLoc Loc, SourceMgr::DiagKind Kind,
                             અચલ Twine &સંદેશ_2, ArrayRef<SMRange> Ranges,
                             ArrayRef<SMFixIt> FixIts, તાર્કિક ShowColors) અચલ {
  PrintMessage(errs(), Loc, Kind, સંદેશ_2, Ranges, FixIts, ShowColors);
}

//===----------------------------------------------------------------------===//
// SMFixIt Implementation
//===----------------------------------------------------------------------===//

SMFixIt::SMFixIt(SMRange R, અચલ Twine &Replacement)
    : Range(R), Text(Replacement.str()) {
  પ્રતિજ્ઞા(R.isValid());
}

//===----------------------------------------------------------------------===//
// SMDiagnostic Implementation
//===----------------------------------------------------------------------===//

SMDiagnostic::SMDiagnostic(અચલ SourceMgr &sm, SMLoc L, StringRef FN, પૂર્ણાંક લીટી,
                           પૂર્ણાંક Col, SourceMgr::DiagKind Kind, StringRef સંદેશ_2,
                           StringRef લીટી_Str,
                           ArrayRef<std::pair<unsigned, unsigned>> Ranges,
                           ArrayRef<SMFixIt> Hints)
    : SM(&sm), Loc(L), Filename(std::string(FN)), લીટી_No(લીટી), ColumnNo(Col),
      Kind(Kind), Message(સંદેશ_2), લીટી_Contents(લીટી_Str), Ranges(Ranges.vec()),
      FixIts(Hints) {
  llvm::sort(FixIts);
}

સ્થિર શૂન્ય build_Fix_It_લીટી(std::string &Caret_લીટી, std::string &Fix_It_લીટી,
                           ArrayRef<SMFixIt> FixIts,
                           ArrayRef<અક્ષર> Source_લીટી) {
  જો (FixIts.empty())
    પાછા;

  અચલ અક્ષર *લીટી_Start = Source_લીટી.begin();
  અચલ અક્ષર *લીટી_End = Source_લીટી.end();

  size_t PrevHintEndCol = 0;

  માટે (અચલ llvm::SMFixIt &Fixit : FixIts) {
    // If the fixit contains a newline or tab, ignore it.
    જો (Fixit.getText().find_first_of("\n\r\t") != StringRef::npos)
      ચાલુ;

    SMRange R = Fixit.getRange();

    // If the line doesn't contain any part of the range, then ignore it.
    જો (R.Start.getPointer() > લીટી_End || R.End.getPointer() < લીટી_Start)
      ચાલુ;

    // Translate from SMLoc to column.
    // Ignore pieces of the range that go onto other lines.
    // FIXME: Handle multibyte characters in the source line.
    unsigned FirstCol;
    જો (R.Start.getPointer() < લીટી_Start)
      FirstCol = 0;
    નહીંતર
      FirstCol = R.Start.getPointer() - લીટી_Start;

    // If we inserted a long previous hint, push this one forwards, and add
    // an extra space to show that this is not part of the previous
    // completion. This is sort of the best we can do when two hints appear
    // to overlap.
    //
    // Note that if this hint is located immediately after the previous
    // hint, no space will be added, since the location is more important.
    unsigned HintCol = FirstCol;
    જો (HintCol < PrevHintEndCol)
      HintCol = PrevHintEndCol + 1;

    // FIXME: This assertion is intended to catch unintended use of multibyte
    // characters in fixits. If we decide to do this, we'll have to track
    // separate byte widths for the source and fixit lines.
    પ્રતિજ્ઞા((size_t)sys::locale::columnWidth(Fixit.getText()) ==
           Fixit.getText().size());

    // This relies on one byte per column in our fixit hints.
    unsigned LastColumnModified = HintCol + Fixit.getText().size();
    જો (LastColumnModified > Fix_It_લીટી.size())
      Fix_It_લીટી.resize(LastColumnModified, ' ');

    llvm::copy(Fixit.getText(), Fix_It_લીટી.begin() + HintCol);

    PrevHintEndCol = LastColumnModified;

    // For replacements, mark the removal range with '~'.
    // FIXME: Handle multibyte characters in the source line.
    unsigned LastCol;
    જો (R.End.getPointer() >= લીટી_End)
      LastCol = લીટી_End - લીટી_Start;
    નહીંતર
      LastCol = R.End.getPointer() - લીટી_Start;

    std::fill(&Caret_લીટી[FirstCol], &Caret_લીટી[LastCol], '~');
  }
}

સ્થિર શૂન્ય print_Source_લીટી(raw_ostream &S, StringRef લીટી_Contents) {
  // Print out the source line one character at a time, so we can expand tabs.
  માટે (unsigned i = 0, e = લીટી_Contents.size(), OutCol = 0; i != e; ++i) {
    size_t NextTab = લીટી_Contents.find('\t', i);
    // If there were no tabs left, print the rest, we are done.
    જો (NextTab == StringRef::npos) {
      S << લીટી_Contents.drop_front(i);
      તોડો;
    }

    // Otherwise, print from i to NextTab.
    S << લીટી_Contents.slice(i, NextTab);
    OutCol += NextTab - i;
    i = NextTab;

    // If we have a tab, emit at least one space, then round up to 8 columns.
    કરો {
      S << ' ';
      ++OutCol;
    } જ્યારે ((OutCol % TabStop) != 0);
  }
  S << '\n';
}

સ્થિર તાર્કિક isNonASCII(અક્ષર c) { પાછા c & 0x80; }

શૂન્ય SMDiagnostic::print(અચલ અક્ષર *ProgName, raw_ostream &OS, તાર્કિક ShowColors,
                         તાર્કિક ShowKindLabel, તાર્કિક ShowLocation) અચલ {
  ColorMode Mode = ShowColors ? ColorMode::Auto : ColorMode::Disable;

  {
    WithColor S(OS, raw_ostream::SAVEDCOLOR, સાચું, ખોટું, Mode);

    જો (ProgName && ProgName[0])
      S << ProgName << ": ";

    જો (ShowLocation && !Filename.empty()) {
      જો (Filename == "-")
        S << "<stdin>";
      નહીંતર
        S << Filename;

      જો (લીટી_No != -1) {
        S << ':' << લીટી_No;
        જો (ColumnNo != -1)
          S << ':' << (ColumnNo + 1);
      }
      S << ": ";
    }
  }

  જો (ShowKindLabel) {
    પસંદગી (Kind) {
    સ્થિતિ SourceMgr::DK_Error:
      WithColor::error(OS, "", !ShowColors);
      તોડો;
    સ્થિતિ SourceMgr::DK_Warning:
      WithColor::warning(OS, "", !ShowColors);
      તોડો;
    સ્થિતિ SourceMgr::DK_Note:
      WithColor::note(OS, "", !ShowColors);
      તોડો;
    સ્થિતિ SourceMgr::DK_Remark:
      WithColor::remark(OS, "", !ShowColors);
      તોડો;
    }
  }

  WithColor(OS, raw_ostream::SAVEDCOLOR, સાચું, ખોટું, Mode) << Message << '\n';

  જો (લીટી_No == -1 || ColumnNo == -1)
    પાછા;

  // FIXME: If there are multibyte or multi-column characters in the source, all
  // our ranges will be wrong. To do this properly, we'll need a byte-to-column
  // map like Clang's TextDiagnostic. For now, we'll just handle tabs by
  // expanding them later, and bail out rather than show incorrect ranges and
  // misaligned fixits for any other odd characters.
  જો (any_of(લીટી_Contents, isNonASCII)) {
    print_Source_લીટી(OS, લીટી_Contents);
    પાછા;
  }
  size_t અંક_Columns = લીટી_Contents.size();

  // Build the line with the caret and ranges.
  std::string Caret_લીટી(અંક_Columns + 1, ' ');

  // Expand any ranges.
  માટે (અચલ std::pair<unsigned, unsigned> &R : Ranges)
    std::fill(&Caret_લીટી[R.first],
              &Caret_લીટી[std::min((size_t)R.second, Caret_લીટી.size())], '~');

  // Add any fix-its.
  // FIXME: Find the beginning of the line properly for multibyte characters.
  std::string Fix_It_Insertion_લીટી;
  build_Fix_It_લીટી(Caret_લીટી, Fix_It_Insertion_લીટી, FixIts,
                 ArrayRef(Loc.getPointer() - ColumnNo, લીટી_Contents.size()));

  // Finally, plop on the caret.
  જો (unsigned(ColumnNo) <= અંક_Columns)
    Caret_લીટી[ColumnNo] = '^';
  નહીંતર
    Caret_લીટી[અંક_Columns] = '^';

  // ... and remove trailing whitespace so the output doesn't wrap for it.  We
  // know that the line isn't completely empty because it has the caret in it at
  // least.
  Caret_લીટી.erase(Caret_લીટી.find_last_not_of(' ') + 1);

  print_Source_લીટી(OS, લીટી_Contents);

  {
    ColorMode Mode = ShowColors ? ColorMode::Auto : ColorMode::Disable;
    WithColor S(OS, raw_ostream::GREEN, સાચું, ખોટું, Mode);

    // Print out the caret line, matching tabs in the source line.
    માટે (unsigned i = 0, e = Caret_લીટી.size(), OutCol = 0; i != e; ++i) {
      જો (i >= લીટી_Contents.size() || લીટી_Contents[i] != '\t') {
        S << Caret_લીટી[i];
        ++OutCol;
        ચાલુ;
      }

      // Okay, we have a tab.  Insert the appropriate number of characters.
      કરો {
        S << Caret_લીટી[i];
        ++OutCol;
      } જ્યારે ((OutCol % TabStop) != 0);
    }
    S << '\n';
  }

  // Print out the replacement line, matching tabs in the source line.
  જો (Fix_It_Insertion_લીટી.empty())
    પાછા;

  માટે (size_t i = 0, e = Fix_It_Insertion_લીટી.size(), OutCol = 0; i < e; ++i) {
    જો (i >= લીટી_Contents.size() || લીટી_Contents[i] != '\t') {
      OS << Fix_It_Insertion_લીટી[i];
      ++OutCol;
      ચાલુ;
    }

    // Okay, we have a tab.  Insert the appropriate number of characters.
    કરો {
      OS << Fix_It_Insertion_લીટી[i];
      // FIXME: This is trying not to break up replacements, but then to re-sync
      // with the tabs between replacements. This will fail, though, if two
      // fix-it replacements are exactly adjacent, or if a fix-it contains a
      // space. Really we should be precomputing column widths, which we'll
      // need anyway for multibyte chars.
      જો (Fix_It_Insertion_લીટી[i] != ' ')
        ++i;
      ++OutCol;
    } જ્યારે (((OutCol % TabStop) != 0) && i != e);
  }
  OS << '\n';
}
