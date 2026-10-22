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

#অন্তর্ভাৱ "llvm/Support/SourceMgr.h"
#অন্তর্ভাৱ "llvm/ADT/ArrayRef.h"
#অন্তর্ভাৱ "llvm/ADT/STLExtras.h"
#অন্তর্ভাৱ "llvm/ADT/SmallString.h"
#অন্তর্ভাৱ "llvm/ADT/SmallVector.h"
#অন্তর্ভাৱ "llvm/ADT/StringRef.h"
#অন্তর্ভাৱ "llvm/ADT/Twine.h"
#অন্তর্ভাৱ "llvm/Support/ErrorOr.h"
#অন্তর্ভাৱ "llvm/Support/Locale.h"
#অন্তর্ভাৱ "llvm/Support/MemoryBuffer.h"
#অন্তর্ভাৱ "llvm/Support/Path.h"
#অন্তর্ভাৱ "llvm/Support/SMLoc.h"
#অন্তর্ভাৱ "llvm/Support/VirtualFileSystem.h"
#অন্তর্ভাৱ "llvm/Support/WithColor.h"
#অন্তর্ভাৱ "llvm/Support/raw_ostream.h"
#অন্তর্ভাৱ <algorithm>
#অন্তর্ভাৱ <cassert>
#অন্তর্ভাৱ <cstddef>
#অন্তর্ভাৱ <limits>
#অন্তর্ভাৱ <memory>
#অন্তর্ভাৱ <string>
#অন্তর্ভাৱ <utility>

using নামাকাশ llvm;

স্থির ধ্রুৱ size_t TabStop = 8;

// Out of line to avoid needing definition of vfs::FileSystem in header.
SourceMgr::SourceMgr() = স্ৱভাৱ;
SourceMgr::SourceMgr(IntrusiveRefCntPtr<vfs::FileSystem> FS)
    : FS(std::move(FS)) {}
SourceMgr::SourceMgr(SourceMgr &&) = স্ৱভাৱ;
SourceMgr &SourceMgr::operator=(SourceMgr &&) = স্ৱভাৱ;
SourceMgr::~SourceMgr() = স্ৱভাৱ;

IntrusiveRefCntPtr<vfs::FileSystem> SourceMgr::getVirtualFileSystem() ধ্রুৱ {
  ঘূৰাই FS;
}

শূন্য SourceMgr::setVirtualFileSystem(IntrusiveRefCntPtr<vfs::FileSystem> FS) {
  এতত্->FS = std::move(FS);
}

unsigned SourceMgr::AddIncludeFile(ধ্রুৱ std::string &Filename,
                                   SMLoc IncludeLoc,
                                   std::string &IncludedFile) {
  ErrorOr<std::unique_ptr<MemoryBuffer>> NewBufOrErr =
      OpenIncludeFile(Filename, IncludedFile);
  যদি (!NewBufOrErr)
    ঘূৰাই 0;

  ঘূৰাই AddNewSourceBuffer(std::move(*NewBufOrErr), IncludeLoc);
}

ErrorOr<std::unique_ptr<MemoryBuffer>>
SourceMgr::OpenIncludeFile(ধ্রুৱ std::string &Filename,
                           std::string &IncludedFile,
                           তার্কিক RequiresNullTerminator) {
  auto GetFile = [এতত্, RequiresNullTerminator](StringRef Path) {
    ঘূৰাই FS ? FS->getBufferForFile(Path, /*FileSize=*/-1,
                                     RequiresNullTerminator)
              : MemoryBuffer::getFile(Path, /*IsText=*/মিছা,
                                      RequiresNullTerminator);
  };

  ErrorOr<std::unique_ptr<MemoryBuffer>> NewBufOrErr = GetFile(Filename);

  SmallString<64> Buffer(Filename);
  // If the file didn't exist directly, see if it's in an include path.
  বাবে (unsigned i = 0, e = IncludeDirectories.size(); i != e && !NewBufOrErr;
       ++i) {
    Buffer = IncludeDirectories[i];
    sys::path::append(Buffer, Filename);
    NewBufOrErr = GetFile(Buffer);
  }

  যদি (NewBufOrErr)
    IncludedFile = static_cast<std::string>(Buffer);

  ঘূৰাই NewBufOrErr;
}

unsigned SourceMgr::FindBufferContainingLoc(SMLoc Loc) ধ্রুৱ {
  বাবে (unsigned i = 0, e = Buffers.size(); i != e; ++i)
    যদি (Loc.getPointer() >= Buffers[i].Buffer->getBufferStart() &&
        // Use <= here so that a pointer to the null at the end of the buffer
        // is included as part of the buffer.
        Loc.getPointer() <= Buffers[i].Buffer->getBufferEnd())
      ঘূৰাই i + 1;
  ঘূৰাই 0;
}

template <typename T>
স্থির std::vector<T> &GetOrCreateOffsetCache(শূন্য *&OffsetCache,
                                              MemoryBuffer *Buffer) {
  যদি (OffsetCache)
    ঘূৰাই *static_cast<std::vector<T> *>(OffsetCache);

  // Lazily fill in the offset cache.
  auto *Offsets = নূতন std::vector<T>();
  size_t Sz = Buffer->getBufferSize();
  প্রতিজ্ঞা(Sz <= std::numeric_limits<T>::max());
  StringRef S = Buffer->getBuffer();
  বাবে (size_t N = 0; N < Sz; ++N) {
    যদি (S[N] == '\n')
      Offsets->ঠেলো_back(static_cast<T>(N));
  }

  OffsetCache = Offsets;
  ঘূৰাই *Offsets;
}

template <typename T>
std::pair<unsigned, unsigned>
SourceMgr::SrcBuffer::get_সাৰি_And_Column_Specialized(ধ্রুৱ ৱর্ণ *Ptr) ধ্রুৱ {
  std::vector<T> &Offsets =
      GetOrCreateOffsetCache<T>(OffsetCache, Buffer.get());

  ধ্রুৱ ৱর্ণ *BufStart = Buffer->getBufferStart();
  প্রতিজ্ঞা(Ptr >= BufStart && Ptr <= Buffer->getBufferEnd());
  ptrdiff_t PtrDiff = Ptr - BufStart;
  প্রতিজ্ঞা(PtrDiff >= 0 &&
         static_cast<size_t>(PtrDiff) <= std::numeric_limits<T>::max());
  T PtrOffset = static_cast<T>(PtrDiff);

  // llvm::lower_bound gives the number of EOL before PtrOffset. Add 1 to get
  // the line number.
  auto I = llvm::lower_bound(Offsets, PtrOffset);
  unsigned সাৰি_No = I - Offsets.begin() + 1;

  // The column number is the distance from the previous EOL (or the start of
  // the buffer) to the pointer.
  T সাৰি_Start_Offs = (I == Offsets.begin()) ? 0 : (I[-1] + 1);
  unsigned ColNo = PtrOffset - সাৰি_Start_Offs + 1;
  ঘূৰাই {সাৰি_No, ColNo};
}

/// Look up a given \p Ptr in the buffer, determining which line and column
/// it came from.
LLVM_ABI std::pair<unsigned, unsigned>
SourceMgr::SrcBuffer::get_সাৰি_And_Column(ধ্রুৱ ৱর্ণ *Ptr) ধ্রুৱ {
  size_t Sz = Buffer->getBufferSize();
  যদি (Sz <= std::numeric_limits<uint8_t>::max())
    ঘূৰাই get_সাৰি_And_Column_Specialized<uint8_t>(Ptr);
  যদি (Sz <= std::numeric_limits<uint16_t>::max())
    ঘূৰাই get_সাৰি_And_Column_Specialized<uint16_t>(Ptr);
  যদি (Sz <= std::numeric_limits<uint32_t>::max())
    ঘূৰাই get_সাৰি_And_Column_Specialized<uint32_t>(Ptr);
  ঘূৰাই get_সাৰি_And_Column_Specialized<uint64_t>(Ptr);
}

template <typename T>
ধ্রুৱ ৱর্ণ *SourceMgr::SrcBuffer::get_Pointer_For_সাৰি_সংখ্যা_Specialized(
    unsigned সাৰি_No) ধ্রুৱ {
  std::vector<T> &Offsets =
      GetOrCreateOffsetCache<T>(OffsetCache, Buffer.get());

  // We start counting line and column numbers from 1.
  যদি (সাৰি_No != 0)
    --সাৰি_No;

  ধ্রুৱ ৱর্ণ *BufStart = Buffer->getBufferStart();

  // The offset cache contains the location of the \n for the specified line,
  // we want the start of the line.  As such, we look for the previous entry.
  যদি (সাৰি_No == 0)
    ঘূৰাই BufStart;
  যদি (সাৰি_No > Offsets.size())
    ঘূৰাই nullptr;
  ঘূৰাই BufStart + Offsets[সাৰি_No - 1] + 1;
}

/// Return a pointer to the first character of the specified line number or
/// null if the line number is invalid.
ধ্রুৱ ৱর্ণ *
SourceMgr::SrcBuffer::get_Pointer_For_সাৰি_সংখ্যা(unsigned সাৰি_No) ধ্রুৱ {
  size_t Sz = Buffer->getBufferSize();
  যদি (Sz <= std::numeric_limits<uint8_t>::max())
    ঘূৰাই get_Pointer_For_সাৰি_সংখ্যা_Specialized<uint8_t>(সাৰি_No);
  অন্যথা যদি (Sz <= std::numeric_limits<uint16_t>::max())
    ঘূৰাই get_Pointer_For_সাৰি_সংখ্যা_Specialized<uint16_t>(সাৰি_No);
  অন্যথা যদি (Sz <= std::numeric_limits<uint32_t>::max())
    ঘূৰাই get_Pointer_For_সাৰি_সংখ্যা_Specialized<uint32_t>(সাৰি_No);
  অন্যথা
    ঘূৰাই get_Pointer_For_সাৰি_সংখ্যা_Specialized<uint64_t>(সাৰি_No);
}

SourceMgr::SrcBuffer::SrcBuffer(SourceMgr::SrcBuffer &&Other)
    : Buffer(std::move(Other.Buffer)), OffsetCache(Other.OffsetCache),
      IncludeLoc(Other.IncludeLoc) {
  Other.OffsetCache = nullptr;
}

SourceMgr::SrcBuffer::~SrcBuffer() {
  যদি (OffsetCache) {
    size_t Sz = Buffer->getBufferSize();
    যদি (Sz <= std::numeric_limits<uint8_t>::max())
      delete static_cast<std::vector<uint8_t> *>(OffsetCache);
    অন্যথা যদি (Sz <= std::numeric_limits<uint16_t>::max())
      delete static_cast<std::vector<uint16_t> *>(OffsetCache);
    অন্যথা যদি (Sz <= std::numeric_limits<uint32_t>::max())
      delete static_cast<std::vector<uint32_t> *>(OffsetCache);
    অন্যথা
      delete static_cast<std::vector<uint64_t> *>(OffsetCache);
    OffsetCache = nullptr;
  }
}

std::pair<unsigned, unsigned>
SourceMgr::get_সাৰি_And_Column(SMLoc Loc, unsigned BufferID) ধ্রুৱ {
  যদি (!BufferID)
    BufferID = FindBufferContainingLoc(Loc);
  প্রতিজ্ঞা(BufferID && "Invalid location!");

  auto &SB = getBufferInfo(BufferID);
  ধ্রুৱ ৱর্ণ *Ptr = Loc.getPointer();

  ঘূৰাই SB.get_সাৰি_And_Column(Ptr);
}

// FIXME: Note that the formatting of source locations is spread between
// multiple functions, some in SourceMgr and some in SMDiagnostic. A better
// solution would be a general-purpose source location formatter
// in one of those two classes, or possibly in SMLoc.

/// Get a string with the source location formatted in the standard
/// style, but without the line offset. If \p IncludePath is true, the path
/// is included. If false, only the file name and extension are included.
std::string SourceMgr::getFormattedLocationNoOffset(SMLoc Loc,
                                                    তার্কিক IncludePath) ধ্রুৱ {
  auto BufferID = FindBufferContainingLoc(Loc);
  প্রতিজ্ঞা(BufferID && "Invalid location!");
  auto FileSpec = getBufferInfo(BufferID).Buffer->getBufferIdentifier();

  যদি (IncludePath) {
    ঘূৰাই FileSpec.str() + ":" + std::to_string(Find_সাৰি_সংখ্যা(Loc, BufferID));
  } অন্যথা {
    auto I = FileSpec.find_last_of("/\\");
    I = (I == FileSpec.size()) ? 0 : (I + 1);
    ঘূৰাই FileSpec.substr(I).str() + ":" +
           std::to_string(Find_সাৰি_সংখ্যা(Loc, BufferID));
  }
}

/// Given a line and column number in a mapped buffer, turn it into an SMLoc.
/// This will return a null SMLoc if the line/column location is invalid.
SMLoc SourceMgr::Find_Loc_For_সাৰি_And_Column(unsigned BufferID, unsigned সাৰি_No,
                                         unsigned ColNo) {
  auto &SB = getBufferInfo(BufferID);
  ধ্রুৱ ৱর্ণ *Ptr = SB.get_Pointer_For_সাৰি_সংখ্যা(সাৰি_No);
  যদি (!Ptr)
    ঘূৰাই SMLoc();

  // We start counting line and column numbers from 1.
  যদি (ColNo != 0)
    --ColNo;

  // If we have a column number, validate it.
  যদি (ColNo) {
    // Make sure the location is within the current line.
    যদি (Ptr + ColNo > SB.Buffer->getBufferEnd())
      ঘূৰাই SMLoc();

    // Make sure there is no newline in the way.
    যদি (StringRef(Ptr, ColNo).find_first_of("\n\r") != StringRef::npos)
      ঘূৰাই SMLoc();

    Ptr += ColNo;
  }

  ঘূৰাই SMLoc::getFromPointer(Ptr);
}

শূন্য SourceMgr::Print_Include_স্তূপ(SMLoc IncludeLoc, raw_ostream &OS) ধ্রুৱ {
  যদি (IncludeLoc == SMLoc())
    ঘূৰাই; // Top of stack.

  unsigned CurBuf = FindBufferContainingLoc(IncludeLoc);
  প্রতিজ্ঞা(CurBuf && "Invalid or unspecified location!");

  Print_Include_স্তূপ(getBufferInfo(CurBuf).IncludeLoc, OS);

  OS << "Included from " << getBufferInfo(CurBuf).Buffer->getBufferIdentifier()
     << ":" << Find_সাৰি_সংখ্যা(IncludeLoc, CurBuf) << ":\n";
}

শূন্য SourceMgr::print_Include_স্তূপ_For_Diagnostic(SMLoc Loc,
                                               raw_ostream &OS) ধ্রুৱ {
  যদি (!Loc.isValid())
    ঘূৰাই;
  unsigned DiagCurBuffer = FindBufferContainingLoc(Loc);
  যদি (DiagCurBuffer && DiagCurBuffer != getMainFileID()) {
    SMLoc ParentIncludeLoc = getParentIncludeLoc(DiagCurBuffer);
    // Ignore macro instantiation buffers to avoid redundant include stacks.
    Print_Include_স্তূপ(ParentIncludeLoc, OS);
  }
}

SMDiagnostic SourceMgr::GetMessage(SMLoc Loc, SourceMgr::DiagKind Kind,
                                   ধ্রুৱ Twine &বাৰ্তা_2, ArrayRef<SMRange> Ranges,
                                   ArrayRef<SMFixIt> FixIts) ধ্রুৱ {
  // First thing to do: find the current buffer containing the specified
  // location to pull out the source line.
  SmallVector<std::pair<unsigned, unsigned>, 4> ColRanges;
  std::pair<unsigned, unsigned> সাৰি_And_Col;
  StringRef BufferID = "<unknown>";
  StringRef সাৰি_Str;

  যদি (Loc.isValid()) {
    unsigned CurBuf = FindBufferContainingLoc(Loc);
    প্রতিজ্ঞা(CurBuf && "Invalid or unspecified location!");

    ধ্রুৱ MemoryBuffer *CurMB = getMemoryBuffer(CurBuf);
    BufferID = CurMB->getBufferIdentifier();

    // Scan backward to find the start of the line.
    ধ্রুৱ ৱর্ণ *সাৰি_Start = Loc.getPointer();
    ধ্রুৱ ৱর্ণ *BufStart = CurMB->getBufferStart();
    যেতিয়ালৈ (সাৰি_Start != BufStart && সাৰি_Start[-1] != '\n' &&
           সাৰি_Start[-1] != '\r')
      --সাৰি_Start;

    // Get the end of the line.
    ধ্রুৱ ৱর্ণ *সাৰি_End = Loc.getPointer();
    ধ্রুৱ ৱর্ণ *BufEnd = CurMB->getBufferEnd();
    যেতিয়ালৈ (সাৰি_End != BufEnd && সাৰি_End[0] != '\n' && সাৰি_End[0] != '\r')
      ++সাৰি_End;
    সাৰি_Str = StringRef(সাৰি_Start, সাৰি_End - সাৰি_Start);

    // Convert any ranges to column ranges that only intersect the line of the
    // location.
    বাবে (SMRange R : Ranges) {
      যদি (!R.isValid())
        অনুৱর্ত;

      // If the line doesn't contain any part of the range, then ignore it.
      যদি (R.Start.getPointer() > সাৰি_End || R.End.getPointer() < সাৰি_Start)
        অনুৱর্ত;

      // Ignore pieces of the range that go onto other lines.
      যদি (R.Start.getPointer() < সাৰি_Start)
        R.Start = SMLoc::getFromPointer(সাৰি_Start);
      যদি (R.End.getPointer() > সাৰি_End)
        R.End = SMLoc::getFromPointer(সাৰি_End);

      // Translate from SMLoc ranges to column ranges.
      // FIXME: Handle multibyte characters.
      ColRanges.ঠেলো_back(std::make_pair(R.Start.getPointer() - সাৰি_Start,
                                         R.End.getPointer() - সাৰি_Start));
    }

    সাৰি_And_Col = get_সাৰি_And_Column(Loc, CurBuf);
  }

  ঘূৰাই SMDiagnostic(*এতত্, Loc, BufferID, সাৰি_And_Col.first,
                      সাৰি_And_Col.second - 1, Kind, বাৰ্তা_2.str(), সাৰি_Str,
                      ColRanges, FixIts);
}

শূন্য SourceMgr::PrintMessage(raw_ostream &OS, ধ্রুৱ SMDiagnostic &Diagnostic,
                             তার্কিক ShowColors) ধ্রুৱ {
  // Report the message with the diagnostic handler if present.
  যদি (DiagHandler) {
    DiagHandler(Diagnostic, DiagContext);
    ঘূৰাই;
  }

  যদি (Diagnostic.getLoc().isValid()) {
    unsigned CurBuf = FindBufferContainingLoc(Diagnostic.getLoc());
    প্রতিজ্ঞা(CurBuf && "Invalid or unspecified location!");
    Print_Include_স্তূপ(getBufferInfo(CurBuf).IncludeLoc, OS);
  }

  Diagnostic.print(nullptr, OS, ShowColors);
}

শূন্য SourceMgr::PrintMessage(raw_ostream &OS, SMLoc Loc,
                             SourceMgr::DiagKind Kind, ধ্রুৱ Twine &বাৰ্তা_2,
                             ArrayRef<SMRange> Ranges, ArrayRef<SMFixIt> FixIts,
                             তার্কিক ShowColors) ধ্রুৱ {
  PrintMessage(OS, GetMessage(Loc, Kind, বাৰ্তা_2, Ranges, FixIts), ShowColors);
}

শূন্য SourceMgr::PrintMessage(SMLoc Loc, SourceMgr::DiagKind Kind,
                             ধ্রুৱ Twine &বাৰ্তা_2, ArrayRef<SMRange> Ranges,
                             ArrayRef<SMFixIt> FixIts, তার্কিক ShowColors) ধ্রুৱ {
  PrintMessage(errs(), Loc, Kind, বাৰ্তা_2, Ranges, FixIts, ShowColors);
}

//===----------------------------------------------------------------------===//
// SMFixIt Implementation
//===----------------------------------------------------------------------===//

SMFixIt::SMFixIt(SMRange R, ধ্রুৱ Twine &Replacement)
    : Range(R), Text(Replacement.str()) {
  প্রতিজ্ঞা(R.isValid());
}

//===----------------------------------------------------------------------===//
// SMDiagnostic Implementation
//===----------------------------------------------------------------------===//

SMDiagnostic::SMDiagnostic(ধ্রুৱ SourceMgr &sm, SMLoc L, StringRef FN, পূর্ণাঙ্ক সাৰি,
                           পূর্ণাঙ্ক Col, SourceMgr::DiagKind Kind, StringRef বাৰ্তা_2,
                           StringRef সাৰি_Str,
                           ArrayRef<std::pair<unsigned, unsigned>> Ranges,
                           ArrayRef<SMFixIt> Hints)
    : SM(&sm), Loc(L), Filename(std::string(FN)), সাৰি_No(সাৰি), ColumnNo(Col),
      Kind(Kind), Message(বাৰ্তা_2), সাৰি_Contents(সাৰি_Str), Ranges(Ranges.vec()),
      FixIts(Hints) {
  llvm::sort(FixIts);
}

স্থির শূন্য build_Fix_It_সাৰি(std::string &Caret_সাৰি, std::string &Fix_It_সাৰি,
                           ArrayRef<SMFixIt> FixIts,
                           ArrayRef<ৱর্ণ> Source_সাৰি) {
  যদি (FixIts.empty())
    ঘূৰাই;

  ধ্রুৱ ৱর্ণ *সাৰি_Start = Source_সাৰি.begin();
  ধ্রুৱ ৱর্ণ *সাৰি_End = Source_সাৰি.end();

  size_t PrevHintEndCol = 0;

  বাবে (ধ্রুৱ llvm::SMFixIt &Fixit : FixIts) {
    // If the fixit contains a newline or tab, ignore it.
    যদি (Fixit.getText().find_first_of("\n\r\t") != StringRef::npos)
      অনুৱর্ত;

    SMRange R = Fixit.getRange();

    // If the line doesn't contain any part of the range, then ignore it.
    যদি (R.Start.getPointer() > সাৰি_End || R.End.getPointer() < সাৰি_Start)
      অনুৱর্ত;

    // Translate from SMLoc to column.
    // Ignore pieces of the range that go onto other lines.
    // FIXME: Handle multibyte characters in the source line.
    unsigned FirstCol;
    যদি (R.Start.getPointer() < সাৰি_Start)
      FirstCol = 0;
    অন্যথা
      FirstCol = R.Start.getPointer() - সাৰি_Start;

    // If we inserted a long previous hint, push this one forwards, and add
    // an extra space to show that this is not part of the previous
    // completion. This is sort of the best we can do when two hints appear
    // to overlap.
    //
    // Note that if this hint is located immediately after the previous
    // hint, no space will be added, since the location is more important.
    unsigned HintCol = FirstCol;
    যদি (HintCol < PrevHintEndCol)
      HintCol = PrevHintEndCol + 1;

    // FIXME: This assertion is intended to catch unintended use of multibyte
    // characters in fixits. If we decide to do this, we'll have to track
    // separate byte widths for the source and fixit lines.
    প্রতিজ্ঞা((size_t)sys::locale::columnWidth(Fixit.getText()) ==
           Fixit.getText().size());

    // This relies on one byte per column in our fixit hints.
    unsigned LastColumnModified = HintCol + Fixit.getText().size();
    যদি (LastColumnModified > Fix_It_সাৰি.size())
      Fix_It_সাৰি.resize(LastColumnModified, ' ');

    llvm::copy(Fixit.getText(), Fix_It_সাৰি.begin() + HintCol);

    PrevHintEndCol = LastColumnModified;

    // For replacements, mark the removal range with '~'.
    // FIXME: Handle multibyte characters in the source line.
    unsigned LastCol;
    যদি (R.End.getPointer() >= সাৰি_End)
      LastCol = সাৰি_End - সাৰি_Start;
    অন্যথা
      LastCol = R.End.getPointer() - সাৰি_Start;

    std::fill(&Caret_সাৰি[FirstCol], &Caret_সাৰি[LastCol], '~');
  }
}

স্থির শূন্য print_Source_সাৰি(raw_ostream &S, StringRef সাৰি_Contents) {
  // Print out the source line one character at a time, so we can expand tabs.
  বাবে (unsigned i = 0, e = সাৰি_Contents.size(), OutCol = 0; i != e; ++i) {
    size_t NextTab = সাৰি_Contents.find('\t', i);
    // If there were no tabs left, print the rest, we are done.
    যদি (NextTab == StringRef::npos) {
      S << সাৰি_Contents.drop_front(i);
      ভঙ্গ;
    }

    // Otherwise, print from i to NextTab.
    S << সাৰি_Contents.slice(i, NextTab);
    OutCol += NextTab - i;
    i = NextTab;

    // If we have a tab, emit at least one space, then round up to 8 columns.
    কৰা {
      S << ' ';
      ++OutCol;
    } যেতিয়ালৈ ((OutCol % TabStop) != 0);
  }
  S << '\n';
}

স্থির তার্কিক isNonASCII(ৱর্ণ c) { ঘূৰাই c & 0x80; }

শূন্য SMDiagnostic::print(ধ্রুৱ ৱর্ণ *ProgName, raw_ostream &OS, তার্কিক ShowColors,
                         তার্কিক ShowKindLabel, তার্কিক ShowLocation) ধ্রুৱ {
  ColorMode Mode = ShowColors ? ColorMode::Auto : ColorMode::Disable;

  {
    WithColor S(OS, raw_ostream::SAVEDCOLOR, সঁচা, মিছা, Mode);

    যদি (ProgName && ProgName[0])
      S << ProgName << ": ";

    যদি (ShowLocation && !Filename.empty()) {
      যদি (Filename == "-")
        S << "<stdin>";
      অন্যথা
        S << Filename;

      যদি (সাৰি_No != -1) {
        S << ':' << সাৰি_No;
        যদি (ColumnNo != -1)
          S << ':' << (ColumnNo + 1);
      }
      S << ": ";
    }
  }

  যদি (ShowKindLabel) {
    ৱিকল্প (Kind) {
    পক্ষ SourceMgr::DK_Error:
      WithColor::error(OS, "", !ShowColors);
      ভঙ্গ;
    পক্ষ SourceMgr::DK_Warning:
      WithColor::warning(OS, "", !ShowColors);
      ভঙ্গ;
    পক্ষ SourceMgr::DK_Note:
      WithColor::note(OS, "", !ShowColors);
      ভঙ্গ;
    পক্ষ SourceMgr::DK_Remark:
      WithColor::remark(OS, "", !ShowColors);
      ভঙ্গ;
    }
  }

  WithColor(OS, raw_ostream::SAVEDCOLOR, সঁচা, মিছা, Mode) << Message << '\n';

  যদি (সাৰি_No == -1 || ColumnNo == -1)
    ঘূৰাই;

  // FIXME: If there are multibyte or multi-column characters in the source, all
  // our ranges will be wrong. To do this properly, we'll need a byte-to-column
  // map like Clang's TextDiagnostic. For now, we'll just handle tabs by
  // expanding them later, and bail out rather than show incorrect ranges and
  // misaligned fixits for any other odd characters.
  যদি (any_of(সাৰি_Contents, isNonASCII)) {
    print_Source_সাৰি(OS, সাৰি_Contents);
    ঘূৰাই;
  }
  size_t অঙ্ক_Columns = সাৰি_Contents.size();

  // Build the line with the caret and ranges.
  std::string Caret_সাৰি(অঙ্ক_Columns + 1, ' ');

  // Expand any ranges.
  বাবে (ধ্রুৱ std::pair<unsigned, unsigned> &R : Ranges)
    std::fill(&Caret_সাৰি[R.first],
              &Caret_সাৰি[std::min((size_t)R.second, Caret_সাৰি.size())], '~');

  // Add any fix-its.
  // FIXME: Find the beginning of the line properly for multibyte characters.
  std::string Fix_It_Insertion_সাৰি;
  build_Fix_It_সাৰি(Caret_সাৰি, Fix_It_Insertion_সাৰি, FixIts,
                 ArrayRef(Loc.getPointer() - ColumnNo, সাৰি_Contents.size()));

  // Finally, plop on the caret.
  যদি (unsigned(ColumnNo) <= অঙ্ক_Columns)
    Caret_সাৰি[ColumnNo] = '^';
  অন্যথা
    Caret_সাৰি[অঙ্ক_Columns] = '^';

  // ... and remove trailing whitespace so the output doesn't wrap for it.  We
  // know that the line isn't completely empty because it has the caret in it at
  // least.
  Caret_সাৰি.erase(Caret_সাৰি.find_last_not_of(' ') + 1);

  print_Source_সাৰি(OS, সাৰি_Contents);

  {
    ColorMode Mode = ShowColors ? ColorMode::Auto : ColorMode::Disable;
    WithColor S(OS, raw_ostream::GREEN, সঁচা, মিছা, Mode);

    // Print out the caret line, matching tabs in the source line.
    বাবে (unsigned i = 0, e = Caret_সাৰি.size(), OutCol = 0; i != e; ++i) {
      যদি (i >= সাৰি_Contents.size() || সাৰি_Contents[i] != '\t') {
        S << Caret_সাৰি[i];
        ++OutCol;
        অনুৱর্ত;
      }

      // Okay, we have a tab.  Insert the appropriate number of characters.
      কৰা {
        S << Caret_সাৰি[i];
        ++OutCol;
      } যেতিয়ালৈ ((OutCol % TabStop) != 0);
    }
    S << '\n';
  }

  // Print out the replacement line, matching tabs in the source line.
  যদি (Fix_It_Insertion_সাৰি.empty())
    ঘূৰাই;

  বাবে (size_t i = 0, e = Fix_It_Insertion_সাৰি.size(), OutCol = 0; i < e; ++i) {
    যদি (i >= সাৰি_Contents.size() || সাৰি_Contents[i] != '\t') {
      OS << Fix_It_Insertion_সাৰি[i];
      ++OutCol;
      অনুৱর্ত;
    }

    // Okay, we have a tab.  Insert the appropriate number of characters.
    কৰা {
      OS << Fix_It_Insertion_সাৰি[i];
      // FIXME: This is trying not to break up replacements, but then to re-sync
      // with the tabs between replacements. This will fail, though, if two
      // fix-it replacements are exactly adjacent, or if a fix-it contains a
      // space. Really we should be precomputing column widths, which we'll
      // need anyway for multibyte chars.
      যদি (Fix_It_Insertion_সাৰি[i] != ' ')
        ++i;
      ++OutCol;
    } যেতিয়ালৈ (((OutCol % TabStop) != 0) && i != e);
  }
  OS << '\n';
}
