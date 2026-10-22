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

#அந்தர்பாவ "llvm/Support/SourceMgr.h"
#அந்தர்பாவ "llvm/ADT/ArrayRef.h"
#அந்தர்பாவ "llvm/ADT/STLExtras.h"
#அந்தர்பாவ "llvm/ADT/SmallString.h"
#அந்தர்பாவ "llvm/ADT/SmallVector.h"
#அந்தர்பாவ "llvm/ADT/StringRef.h"
#அந்தர்பாவ "llvm/ADT/Twine.h"
#அந்தர்பாவ "llvm/Support/ErrorOr.h"
#அந்தர்பாவ "llvm/Support/Locale.h"
#அந்தர்பாவ "llvm/Support/MemoryBuffer.h"
#அந்தர்பாவ "llvm/Support/Path.h"
#அந்தர்பாவ "llvm/Support/SMLoc.h"
#அந்தர்பாவ "llvm/Support/VirtualFileSystem.h"
#அந்தர்பாவ "llvm/Support/WithColor.h"
#அந்தர்பாவ "llvm/Support/raw_ostream.h"
#அந்தர்பாவ <algorithm>
#அந்தர்பாவ <cassert>
#அந்தர்பாவ <cstddef>
#அந்தர்பாவ <limits>
#அந்தர்பாவ <memory>
#அந்தர்பாவ <string>
#அந்தர்பாவ <utility>

using நாமாகாஶ llvm;

நிலையான மாறிலி size_t TabStop = 8;

// Out of line to avoid needing definition of vfs::FileSystem in header.
SourceMgr::SourceMgr() = இயல்பு;
SourceMgr::SourceMgr(IntrusiveRefCntPtr<vfs::FileSystem> FS)
    : FS(std::move(FS)) {}
SourceMgr::SourceMgr(SourceMgr &&) = இயல்பு;
SourceMgr &SourceMgr::operator=(SourceMgr &&) = இயல்பு;
SourceMgr::~SourceMgr() = இயல்பு;

IntrusiveRefCntPtr<vfs::FileSystem> SourceMgr::getVirtualFileSystem() மாறிலி {
  திரும்பு FS;
}

வெறுமை SourceMgr::setVirtualFileSystem(IntrusiveRefCntPtr<vfs::FileSystem> FS) {
  இது->FS = std::move(FS);
}

unsigned SourceMgr::AddIncludeFile(மாறிலி std::string &Filename,
                                   SMLoc IncludeLoc,
                                   std::string &IncludedFile) {
  ErrorOr<std::unique_ptr<MemoryBuffer>> NewBufOrErr =
      OpenIncludeFile(Filename, IncludedFile);
  எனில் (!NewBufOrErr)
    திரும்பு 0;

  திரும்பு AddNewSourceBuffer(std::move(*NewBufOrErr), IncludeLoc);
}

ErrorOr<std::unique_ptr<MemoryBuffer>>
SourceMgr::OpenIncludeFile(மாறிலி std::string &Filename,
                           std::string &IncludedFile,
                           தருக்க RequiresNullTerminator) {
  auto GetFile = [இது, RequiresNullTerminator](StringRef Path) {
    திரும்பு FS ? FS->getBufferForFile(Path, /*FileSize=*/-1,
                                     RequiresNullTerminator)
              : MemoryBuffer::getFile(Path, /*IsText=*/பொய்,
                                      RequiresNullTerminator);
  };

  ErrorOr<std::unique_ptr<MemoryBuffer>> NewBufOrErr = GetFile(Filename);

  SmallString<64> Buffer(Filename);
  // If the file didn't exist directly, see if it's in an include path.
  க்கு (unsigned i = 0, e = IncludeDirectories.size(); i != e && !NewBufOrErr;
       ++i) {
    Buffer = IncludeDirectories[i];
    sys::path::append(Buffer, Filename);
    NewBufOrErr = GetFile(Buffer);
  }

  எனில் (NewBufOrErr)
    IncludedFile = static_cast<std::string>(Buffer);

  திரும்பு NewBufOrErr;
}

unsigned SourceMgr::FindBufferContainingLoc(SMLoc Loc) மாறிலி {
  க்கு (unsigned i = 0, e = Buffers.size(); i != e; ++i)
    எனில் (Loc.getPointer() >= Buffers[i].Buffer->getBufferStart() &&
        // Use <= here so that a pointer to the null at the end of the buffer
        // is included as part of the buffer.
        Loc.getPointer() <= Buffers[i].Buffer->getBufferEnd())
      திரும்பு i + 1;
  திரும்பு 0;
}

template <typename T>
நிலையான std::vector<T> &GetOrCreateOffsetCache(வெறுமை *&OffsetCache,
                                              MemoryBuffer *Buffer) {
  எனில் (OffsetCache)
    திரும்பு *static_cast<std::vector<T> *>(OffsetCache);

  // Lazily fill in the offset cache.
  auto *Offsets = புதிய std::vector<T>();
  size_t Sz = Buffer->getBufferSize();
  ப்ரதிஜ்ஞா(Sz <= std::numeric_limits<T>::max());
  StringRef S = Buffer->getBuffer();
  க்கு (size_t N = 0; N < Sz; ++N) {
    எனில் (S[N] == '\n')
      Offsets->தள்ளு_back(static_cast<T>(N));
  }

  OffsetCache = Offsets;
  திரும்பு *Offsets;
}

template <typename T>
std::pair<unsigned, unsigned>
SourceMgr::SrcBuffer::get_வரி_And_Column_Specialized(மாறிலி எழுத்து *Ptr) மாறிலி {
  std::vector<T> &Offsets =
      GetOrCreateOffsetCache<T>(OffsetCache, Buffer.get());

  மாறிலி எழுத்து *BufStart = Buffer->getBufferStart();
  ப்ரதிஜ்ஞா(Ptr >= BufStart && Ptr <= Buffer->getBufferEnd());
  ptrdiff_t PtrDiff = Ptr - BufStart;
  ப்ரதிஜ்ஞா(PtrDiff >= 0 &&
         static_cast<size_t>(PtrDiff) <= std::numeric_limits<T>::max());
  T PtrOffset = static_cast<T>(PtrDiff);

  // llvm::lower_bound gives the number of EOL before PtrOffset. Add 1 to get
  // the line number.
  auto I = llvm::lower_bound(Offsets, PtrOffset);
  unsigned வரி_No = I - Offsets.begin() + 1;

  // The column number is the distance from the previous EOL (or the start of
  // the buffer) to the pointer.
  T வரி_Start_Offs = (I == Offsets.begin()) ? 0 : (I[-1] + 1);
  unsigned ColNo = PtrOffset - வரி_Start_Offs + 1;
  திரும்பு {வரி_No, ColNo};
}

/// Look up a given \p Ptr in the buffer, determining which line and column
/// it came from.
LLVM_ABI std::pair<unsigned, unsigned>
SourceMgr::SrcBuffer::get_வரி_And_Column(மாறிலி எழுத்து *Ptr) மாறிலி {
  size_t Sz = Buffer->getBufferSize();
  எனில் (Sz <= std::numeric_limits<uint8_t>::max())
    திரும்பு get_வரி_And_Column_Specialized<uint8_t>(Ptr);
  எனில் (Sz <= std::numeric_limits<uint16_t>::max())
    திரும்பு get_வரி_And_Column_Specialized<uint16_t>(Ptr);
  எனில் (Sz <= std::numeric_limits<uint32_t>::max())
    திரும்பு get_வரி_And_Column_Specialized<uint32_t>(Ptr);
  திரும்பு get_வரி_And_Column_Specialized<uint64_t>(Ptr);
}

template <typename T>
மாறிலி எழுத்து *SourceMgr::SrcBuffer::get_Pointer_For_வரி_எண்_Specialized(
    unsigned வரி_No) மாறிலி {
  std::vector<T> &Offsets =
      GetOrCreateOffsetCache<T>(OffsetCache, Buffer.get());

  // We start counting line and column numbers from 1.
  எனில் (வரி_No != 0)
    --வரி_No;

  மாறிலி எழுத்து *BufStart = Buffer->getBufferStart();

  // The offset cache contains the location of the \n for the specified line,
  // we want the start of the line.  As such, we look for the previous entry.
  எனில் (வரி_No == 0)
    திரும்பு BufStart;
  எனில் (வரி_No > Offsets.size())
    திரும்பு nullptr;
  திரும்பு BufStart + Offsets[வரி_No - 1] + 1;
}

/// Return a pointer to the first character of the specified line number or
/// null if the line number is invalid.
மாறிலி எழுத்து *
SourceMgr::SrcBuffer::get_Pointer_For_வரி_எண்(unsigned வரி_No) மாறிலி {
  size_t Sz = Buffer->getBufferSize();
  எனில் (Sz <= std::numeric_limits<uint8_t>::max())
    திரும்பு get_Pointer_For_வரி_எண்_Specialized<uint8_t>(வரி_No);
  இல்லையெனில் எனில் (Sz <= std::numeric_limits<uint16_t>::max())
    திரும்பு get_Pointer_For_வரி_எண்_Specialized<uint16_t>(வரி_No);
  இல்லையெனில் எனில் (Sz <= std::numeric_limits<uint32_t>::max())
    திரும்பு get_Pointer_For_வரி_எண்_Specialized<uint32_t>(வரி_No);
  இல்லையெனில்
    திரும்பு get_Pointer_For_வரி_எண்_Specialized<uint64_t>(வரி_No);
}

SourceMgr::SrcBuffer::SrcBuffer(SourceMgr::SrcBuffer &&Other)
    : Buffer(std::move(Other.Buffer)), OffsetCache(Other.OffsetCache),
      IncludeLoc(Other.IncludeLoc) {
  Other.OffsetCache = nullptr;
}

SourceMgr::SrcBuffer::~SrcBuffer() {
  எனில் (OffsetCache) {
    size_t Sz = Buffer->getBufferSize();
    எனில் (Sz <= std::numeric_limits<uint8_t>::max())
      delete static_cast<std::vector<uint8_t> *>(OffsetCache);
    இல்லையெனில் எனில் (Sz <= std::numeric_limits<uint16_t>::max())
      delete static_cast<std::vector<uint16_t> *>(OffsetCache);
    இல்லையெனில் எனில் (Sz <= std::numeric_limits<uint32_t>::max())
      delete static_cast<std::vector<uint32_t> *>(OffsetCache);
    இல்லையெனில்
      delete static_cast<std::vector<uint64_t> *>(OffsetCache);
    OffsetCache = nullptr;
  }
}

std::pair<unsigned, unsigned>
SourceMgr::get_வரி_And_Column(SMLoc Loc, unsigned BufferID) மாறிலி {
  எனில் (!BufferID)
    BufferID = FindBufferContainingLoc(Loc);
  ப்ரதிஜ்ஞா(BufferID && "Invalid location!");

  auto &SB = getBufferInfo(BufferID);
  மாறிலி எழுத்து *Ptr = Loc.getPointer();

  திரும்பு SB.get_வரி_And_Column(Ptr);
}

// FIXME: Note that the formatting of source locations is spread between
// multiple functions, some in SourceMgr and some in SMDiagnostic. A better
// solution would be a general-purpose source location formatter
// in one of those two classes, or possibly in SMLoc.

/// Get a string with the source location formatted in the standard
/// style, but without the line offset. If \p IncludePath is true, the path
/// is included. If false, only the file name and extension are included.
std::string SourceMgr::getFormattedLocationNoOffset(SMLoc Loc,
                                                    தருக்க IncludePath) மாறிலி {
  auto BufferID = FindBufferContainingLoc(Loc);
  ப்ரதிஜ்ஞா(BufferID && "Invalid location!");
  auto FileSpec = getBufferInfo(BufferID).Buffer->getBufferIdentifier();

  எனில் (IncludePath) {
    திரும்பு FileSpec.str() + ":" + std::to_string(Find_வரி_எண்(Loc, BufferID));
  } இல்லையெனில் {
    auto I = FileSpec.find_last_of("/\\");
    I = (I == FileSpec.size()) ? 0 : (I + 1);
    திரும்பு FileSpec.substr(I).str() + ":" +
           std::to_string(Find_வரி_எண்(Loc, BufferID));
  }
}

/// Given a line and column number in a mapped buffer, turn it into an SMLoc.
/// This will return a null SMLoc if the line/column location is invalid.
SMLoc SourceMgr::Find_Loc_For_வரி_And_Column(unsigned BufferID, unsigned வரி_No,
                                         unsigned ColNo) {
  auto &SB = getBufferInfo(BufferID);
  மாறிலி எழுத்து *Ptr = SB.get_Pointer_For_வரி_எண்(வரி_No);
  எனில் (!Ptr)
    திரும்பு SMLoc();

  // We start counting line and column numbers from 1.
  எனில் (ColNo != 0)
    --ColNo;

  // If we have a column number, validate it.
  எனில் (ColNo) {
    // Make sure the location is within the current line.
    எனில் (Ptr + ColNo > SB.Buffer->getBufferEnd())
      திரும்பு SMLoc();

    // Make sure there is no newline in the way.
    எனில் (StringRef(Ptr, ColNo).find_first_of("\n\r") != StringRef::npos)
      திரும்பு SMLoc();

    Ptr += ColNo;
  }

  திரும்பு SMLoc::getFromPointer(Ptr);
}

வெறுமை SourceMgr::Print_Include_அடுக்கு(SMLoc IncludeLoc, raw_ostream &OS) மாறிலி {
  எனில் (IncludeLoc == SMLoc())
    திரும்பு; // Top of stack.

  unsigned CurBuf = FindBufferContainingLoc(IncludeLoc);
  ப்ரதிஜ்ஞா(CurBuf && "Invalid or unspecified location!");

  Print_Include_அடுக்கு(getBufferInfo(CurBuf).IncludeLoc, OS);

  OS << "Included from " << getBufferInfo(CurBuf).Buffer->getBufferIdentifier()
     << ":" << Find_வரி_எண்(IncludeLoc, CurBuf) << ":\n";
}

வெறுமை SourceMgr::print_Include_அடுக்கு_For_Diagnostic(SMLoc Loc,
                                               raw_ostream &OS) மாறிலி {
  எனில் (!Loc.isValid())
    திரும்பு;
  unsigned DiagCurBuffer = FindBufferContainingLoc(Loc);
  எனில் (DiagCurBuffer && DiagCurBuffer != getMainFileID()) {
    SMLoc ParentIncludeLoc = getParentIncludeLoc(DiagCurBuffer);
    // Ignore macro instantiation buffers to avoid redundant include stacks.
    Print_Include_அடுக்கு(ParentIncludeLoc, OS);
  }
}

SMDiagnostic SourceMgr::GetMessage(SMLoc Loc, SourceMgr::DiagKind Kind,
                                   மாறிலி Twine &செய்தி_2, ArrayRef<SMRange> Ranges,
                                   ArrayRef<SMFixIt> FixIts) மாறிலி {
  // First thing to do: find the current buffer containing the specified
  // location to pull out the source line.
  SmallVector<std::pair<unsigned, unsigned>, 4> ColRanges;
  std::pair<unsigned, unsigned> வரி_And_Col;
  StringRef BufferID = "<unknown>";
  StringRef வரி_Str;

  எனில் (Loc.isValid()) {
    unsigned CurBuf = FindBufferContainingLoc(Loc);
    ப்ரதிஜ்ஞா(CurBuf && "Invalid or unspecified location!");

    மாறிலி MemoryBuffer *CurMB = getMemoryBuffer(CurBuf);
    BufferID = CurMB->getBufferIdentifier();

    // Scan backward to find the start of the line.
    மாறிலி எழுத்து *வரி_Start = Loc.getPointer();
    மாறிலி எழுத்து *BufStart = CurMB->getBufferStart();
    போது (வரி_Start != BufStart && வரி_Start[-1] != '\n' &&
           வரி_Start[-1] != '\r')
      --வரி_Start;

    // Get the end of the line.
    மாறிலி எழுத்து *வரி_End = Loc.getPointer();
    மாறிலி எழுத்து *BufEnd = CurMB->getBufferEnd();
    போது (வரி_End != BufEnd && வரி_End[0] != '\n' && வரி_End[0] != '\r')
      ++வரி_End;
    வரி_Str = StringRef(வரி_Start, வரி_End - வரி_Start);

    // Convert any ranges to column ranges that only intersect the line of the
    // location.
    க்கு (SMRange R : Ranges) {
      எனில் (!R.isValid())
        தொடர்;

      // If the line doesn't contain any part of the range, then ignore it.
      எனில் (R.Start.getPointer() > வரி_End || R.End.getPointer() < வரி_Start)
        தொடர்;

      // Ignore pieces of the range that go onto other lines.
      எனில் (R.Start.getPointer() < வரி_Start)
        R.Start = SMLoc::getFromPointer(வரி_Start);
      எனில் (R.End.getPointer() > வரி_End)
        R.End = SMLoc::getFromPointer(வரி_End);

      // Translate from SMLoc ranges to column ranges.
      // FIXME: Handle multibyte characters.
      ColRanges.தள்ளு_back(std::make_pair(R.Start.getPointer() - வரி_Start,
                                         R.End.getPointer() - வரி_Start));
    }

    வரி_And_Col = get_வரி_And_Column(Loc, CurBuf);
  }

  திரும்பு SMDiagnostic(*இது, Loc, BufferID, வரி_And_Col.first,
                      வரி_And_Col.second - 1, Kind, செய்தி_2.str(), வரி_Str,
                      ColRanges, FixIts);
}

வெறுமை SourceMgr::PrintMessage(raw_ostream &OS, மாறிலி SMDiagnostic &Diagnostic,
                             தருக்க ShowColors) மாறிலி {
  // Report the message with the diagnostic handler if present.
  எனில் (DiagHandler) {
    DiagHandler(Diagnostic, DiagContext);
    திரும்பு;
  }

  எனில் (Diagnostic.getLoc().isValid()) {
    unsigned CurBuf = FindBufferContainingLoc(Diagnostic.getLoc());
    ப்ரதிஜ்ஞா(CurBuf && "Invalid or unspecified location!");
    Print_Include_அடுக்கு(getBufferInfo(CurBuf).IncludeLoc, OS);
  }

  Diagnostic.print(nullptr, OS, ShowColors);
}

வெறுமை SourceMgr::PrintMessage(raw_ostream &OS, SMLoc Loc,
                             SourceMgr::DiagKind Kind, மாறிலி Twine &செய்தி_2,
                             ArrayRef<SMRange> Ranges, ArrayRef<SMFixIt> FixIts,
                             தருக்க ShowColors) மாறிலி {
  PrintMessage(OS, GetMessage(Loc, Kind, செய்தி_2, Ranges, FixIts), ShowColors);
}

வெறுமை SourceMgr::PrintMessage(SMLoc Loc, SourceMgr::DiagKind Kind,
                             மாறிலி Twine &செய்தி_2, ArrayRef<SMRange> Ranges,
                             ArrayRef<SMFixIt> FixIts, தருக்க ShowColors) மாறிலி {
  PrintMessage(errs(), Loc, Kind, செய்தி_2, Ranges, FixIts, ShowColors);
}

//===----------------------------------------------------------------------===//
// SMFixIt Implementation
//===----------------------------------------------------------------------===//

SMFixIt::SMFixIt(SMRange R, மாறிலி Twine &Replacement)
    : Range(R), Text(Replacement.str()) {
  ப்ரதிஜ்ஞா(R.isValid());
}

//===----------------------------------------------------------------------===//
// SMDiagnostic Implementation
//===----------------------------------------------------------------------===//

SMDiagnostic::SMDiagnostic(மாறிலி SourceMgr &sm, SMLoc L, StringRef FN, முழு வரி,
                           முழு Col, SourceMgr::DiagKind Kind, StringRef செய்தி_2,
                           StringRef வரி_Str,
                           ArrayRef<std::pair<unsigned, unsigned>> Ranges,
                           ArrayRef<SMFixIt> Hints)
    : SM(&sm), Loc(L), Filename(std::string(FN)), வரி_No(வரி), ColumnNo(Col),
      Kind(Kind), Message(செய்தி_2), வரி_Contents(வரி_Str), Ranges(Ranges.vec()),
      FixIts(Hints) {
  llvm::sort(FixIts);
}

நிலையான வெறுமை build_Fix_It_வரி(std::string &Caret_வரி, std::string &Fix_It_வரி,
                           ArrayRef<SMFixIt> FixIts,
                           ArrayRef<எழுத்து> Source_வரி) {
  எனில் (FixIts.empty())
    திரும்பு;

  மாறிலி எழுத்து *வரி_Start = Source_வரி.begin();
  மாறிலி எழுத்து *வரி_End = Source_வரி.end();

  size_t PrevHintEndCol = 0;

  க்கு (மாறிலி llvm::SMFixIt &Fixit : FixIts) {
    // If the fixit contains a newline or tab, ignore it.
    எனில் (Fixit.getText().find_first_of("\n\r\t") != StringRef::npos)
      தொடர்;

    SMRange R = Fixit.getRange();

    // If the line doesn't contain any part of the range, then ignore it.
    எனில் (R.Start.getPointer() > வரி_End || R.End.getPointer() < வரி_Start)
      தொடர்;

    // Translate from SMLoc to column.
    // Ignore pieces of the range that go onto other lines.
    // FIXME: Handle multibyte characters in the source line.
    unsigned FirstCol;
    எனில் (R.Start.getPointer() < வரி_Start)
      FirstCol = 0;
    இல்லையெனில்
      FirstCol = R.Start.getPointer() - வரி_Start;

    // If we inserted a long previous hint, push this one forwards, and add
    // an extra space to show that this is not part of the previous
    // completion. This is sort of the best we can do when two hints appear
    // to overlap.
    //
    // Note that if this hint is located immediately after the previous
    // hint, no space will be added, since the location is more important.
    unsigned HintCol = FirstCol;
    எனில் (HintCol < PrevHintEndCol)
      HintCol = PrevHintEndCol + 1;

    // FIXME: This assertion is intended to catch unintended use of multibyte
    // characters in fixits. If we decide to do this, we'll have to track
    // separate byte widths for the source and fixit lines.
    ப்ரதிஜ்ஞா((size_t)sys::locale::columnWidth(Fixit.getText()) ==
           Fixit.getText().size());

    // This relies on one byte per column in our fixit hints.
    unsigned LastColumnModified = HintCol + Fixit.getText().size();
    எனில் (LastColumnModified > Fix_It_வரி.size())
      Fix_It_வரி.resize(LastColumnModified, ' ');

    llvm::copy(Fixit.getText(), Fix_It_வரி.begin() + HintCol);

    PrevHintEndCol = LastColumnModified;

    // For replacements, mark the removal range with '~'.
    // FIXME: Handle multibyte characters in the source line.
    unsigned LastCol;
    எனில் (R.End.getPointer() >= வரி_End)
      LastCol = வரி_End - வரி_Start;
    இல்லையெனில்
      LastCol = R.End.getPointer() - வரி_Start;

    std::fill(&Caret_வரி[FirstCol], &Caret_வரி[LastCol], '~');
  }
}

நிலையான வெறுமை print_Source_வரி(raw_ostream &S, StringRef வரி_Contents) {
  // Print out the source line one character at a time, so we can expand tabs.
  க்கு (unsigned i = 0, e = வரி_Contents.size(), OutCol = 0; i != e; ++i) {
    size_t NextTab = வரி_Contents.find('\t', i);
    // If there were no tabs left, print the rest, we are done.
    எனில் (NextTab == StringRef::npos) {
      S << வரி_Contents.drop_front(i);
      நிறுத்து;
    }

    // Otherwise, print from i to NextTab.
    S << வரி_Contents.slice(i, NextTab);
    OutCol += NextTab - i;
    i = NextTab;

    // If we have a tab, emit at least one space, then round up to 8 columns.
    செய் {
      S << ' ';
      ++OutCol;
    } போது ((OutCol % TabStop) != 0);
  }
  S << '\n';
}

நிலையான தருக்க isNonASCII(எழுத்து c) { திரும்பு c & 0x80; }

வெறுமை SMDiagnostic::print(மாறிலி எழுத்து *ProgName, raw_ostream &OS, தருக்க ShowColors,
                         தருக்க ShowKindLabel, தருக்க ShowLocation) மாறிலி {
  ColorMode Mode = ShowColors ? ColorMode::Auto : ColorMode::Disable;

  {
    WithColor S(OS, raw_ostream::SAVEDCOLOR, உண்மை, பொய், Mode);

    எனில் (ProgName && ProgName[0])
      S << ProgName << ": ";

    எனில் (ShowLocation && !Filename.empty()) {
      எனில் (Filename == "-")
        S << "<stdin>";
      இல்லையெனில்
        S << Filename;

      எனில் (வரி_No != -1) {
        S << ':' << வரி_No;
        எனில் (ColumnNo != -1)
          S << ':' << (ColumnNo + 1);
      }
      S << ": ";
    }
  }

  எனில் (ShowKindLabel) {
    தேர்வு (Kind) {
    நிலை SourceMgr::DK_Error:
      WithColor::error(OS, "", !ShowColors);
      நிறுத்து;
    நிலை SourceMgr::DK_Warning:
      WithColor::warning(OS, "", !ShowColors);
      நிறுத்து;
    நிலை SourceMgr::DK_Note:
      WithColor::note(OS, "", !ShowColors);
      நிறுத்து;
    நிலை SourceMgr::DK_Remark:
      WithColor::remark(OS, "", !ShowColors);
      நிறுத்து;
    }
  }

  WithColor(OS, raw_ostream::SAVEDCOLOR, உண்மை, பொய், Mode) << Message << '\n';

  எனில் (வரி_No == -1 || ColumnNo == -1)
    திரும்பு;

  // FIXME: If there are multibyte or multi-column characters in the source, all
  // our ranges will be wrong. To do this properly, we'll need a byte-to-column
  // map like Clang's TextDiagnostic. For now, we'll just handle tabs by
  // expanding them later, and bail out rather than show incorrect ranges and
  // misaligned fixits for any other odd characters.
  எனில் (any_of(வரி_Contents, isNonASCII)) {
    print_Source_வரி(OS, வரி_Contents);
    திரும்பு;
  }
  size_t இலக்கம்_Columns = வரி_Contents.size();

  // Build the line with the caret and ranges.
  std::string Caret_வரி(இலக்கம்_Columns + 1, ' ');

  // Expand any ranges.
  க்கு (மாறிலி std::pair<unsigned, unsigned> &R : Ranges)
    std::fill(&Caret_வரி[R.first],
              &Caret_வரி[std::min((size_t)R.second, Caret_வரி.size())], '~');

  // Add any fix-its.
  // FIXME: Find the beginning of the line properly for multibyte characters.
  std::string Fix_It_Insertion_வரி;
  build_Fix_It_வரி(Caret_வரி, Fix_It_Insertion_வரி, FixIts,
                 ArrayRef(Loc.getPointer() - ColumnNo, வரி_Contents.size()));

  // Finally, plop on the caret.
  எனில் (unsigned(ColumnNo) <= இலக்கம்_Columns)
    Caret_வரி[ColumnNo] = '^';
  இல்லையெனில்
    Caret_வரி[இலக்கம்_Columns] = '^';

  // ... and remove trailing whitespace so the output doesn't wrap for it.  We
  // know that the line isn't completely empty because it has the caret in it at
  // least.
  Caret_வரி.erase(Caret_வரி.find_last_not_of(' ') + 1);

  print_Source_வரி(OS, வரி_Contents);

  {
    ColorMode Mode = ShowColors ? ColorMode::Auto : ColorMode::Disable;
    WithColor S(OS, raw_ostream::GREEN, உண்மை, பொய், Mode);

    // Print out the caret line, matching tabs in the source line.
    க்கு (unsigned i = 0, e = Caret_வரி.size(), OutCol = 0; i != e; ++i) {
      எனில் (i >= வரி_Contents.size() || வரி_Contents[i] != '\t') {
        S << Caret_வரி[i];
        ++OutCol;
        தொடர்;
      }

      // Okay, we have a tab.  Insert the appropriate number of characters.
      செய் {
        S << Caret_வரி[i];
        ++OutCol;
      } போது ((OutCol % TabStop) != 0);
    }
    S << '\n';
  }

  // Print out the replacement line, matching tabs in the source line.
  எனில் (Fix_It_Insertion_வரி.empty())
    திரும்பு;

  க்கு (size_t i = 0, e = Fix_It_Insertion_வரி.size(), OutCol = 0; i < e; ++i) {
    எனில் (i >= வரி_Contents.size() || வரி_Contents[i] != '\t') {
      OS << Fix_It_Insertion_வரி[i];
      ++OutCol;
      தொடர்;
    }

    // Okay, we have a tab.  Insert the appropriate number of characters.
    செய் {
      OS << Fix_It_Insertion_வரி[i];
      // FIXME: This is trying not to break up replacements, but then to re-sync
      // with the tabs between replacements. This will fail, though, if two
      // fix-it replacements are exactly adjacent, or if a fix-it contains a
      // space. Really we should be precomputing column widths, which we'll
      // need anyway for multibyte chars.
      எனில் (Fix_It_Insertion_வரி[i] != ' ')
        ++i;
      ++OutCol;
    } போது (((OutCol % TabStop) != 0) && i != e);
  }
  OS << '\n';
}
