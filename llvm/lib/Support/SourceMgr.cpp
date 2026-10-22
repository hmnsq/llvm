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

#شامل "llvm/Support/SourceMgr.h"
#شامل "llvm/ADT/ArrayRef.h"
#شامل "llvm/ADT/STLExtras.h"
#شامل "llvm/ADT/SmallString.h"
#شامل "llvm/ADT/SmallVector.h"
#شامل "llvm/ADT/StringRef.h"
#شامل "llvm/ADT/Twine.h"
#شامل "llvm/Support/ErrorOr.h"
#شامل "llvm/Support/Locale.h"
#شامل "llvm/Support/MemoryBuffer.h"
#شامل "llvm/Support/Path.h"
#شامل "llvm/Support/SMLoc.h"
#شامل "llvm/Support/VirtualFileSystem.h"
#شامل "llvm/Support/WithColor.h"
#شامل "llvm/Support/raw_ostream.h"
#شامل <algorithm>
#شامل <cassert>
#شامل <cstddef>
#شامل <limits>
#شامل <memory>
#شامل <string>
#شامل <utility>

using فضاینام llvm;

ایستا ثابت size_t TabStop = 8;

// Out of line to avoid needing definition of vfs::FileSystem in header.
SourceMgr::SourceMgr() = پیشفرض;
SourceMgr::SourceMgr(IntrusiveRefCntPtr<vfs::FileSystem> FS)
    : FS(std::move(FS)) {}
SourceMgr::SourceMgr(SourceMgr &&) = پیشفرض;
SourceMgr &SourceMgr::operator=(SourceMgr &&) = پیشفرض;
SourceMgr::~SourceMgr() = پیشفرض;

IntrusiveRefCntPtr<vfs::FileSystem> SourceMgr::getVirtualFileSystem() ثابت {
  بازگشت FS;
}

تهی SourceMgr::setVirtualFileSystem(IntrusiveRefCntPtr<vfs::FileSystem> FS) {
  این->FS = std::move(FS);
}

unsigned SourceMgr::AddIncludeFile(ثابت std::string &Filename,
                                   SMLoc IncludeLoc,
                                   std::string &IncludedFile) {
  ErrorOr<std::unique_ptr<MemoryBuffer>> NewBufOrErr =
      OpenIncludeFile(Filename, IncludedFile);
  اگر (!NewBufOrErr)
    بازگشت 0;

  بازگشت AddNewSourceBuffer(std::move(*NewBufOrErr), IncludeLoc);
}

ErrorOr<std::unique_ptr<MemoryBuffer>>
SourceMgr::OpenIncludeFile(ثابت std::string &Filename,
                           std::string &IncludedFile,
                           منطقی RequiresNullTerminator) {
  auto GetFile = [این, RequiresNullTerminator](StringRef Path) {
    بازگشت FS ? FS->getBufferForFile(Path, /*FileSize=*/-1,
                                     RequiresNullTerminator)
              : MemoryBuffer::getFile(Path, /*IsText=*/نادرست,
                                      RequiresNullTerminator);
  };

  ErrorOr<std::unique_ptr<MemoryBuffer>> NewBufOrErr = GetFile(Filename);

  SmallString<64> Buffer(Filename);
  // If the file didn't exist directly, see if it's in an include path.
  برای (unsigned i = 0, e = IncludeDirectories.size(); i != e && !NewBufOrErr;
       ++i) {
    Buffer = IncludeDirectories[i];
    sys::path::append(Buffer, Filename);
    NewBufOrErr = GetFile(Buffer);
  }

  اگر (NewBufOrErr)
    IncludedFile = static_cast<std::string>(Buffer);

  بازگشت NewBufOrErr;
}

unsigned SourceMgr::FindBufferContainingLoc(SMLoc Loc) ثابت {
  برای (unsigned i = 0, e = Buffers.size(); i != e; ++i)
    اگر (Loc.getPointer() >= Buffers[i].Buffer->getBufferStart() &&
        // Use <= here so that a pointer to the null at the end of the buffer
        // is included as part of the buffer.
        Loc.getPointer() <= Buffers[i].Buffer->getBufferEnd())
      بازگشت i + 1;
  بازگشت 0;
}

template <typename T>
ایستا std::vector<T> &GetOrCreateOffsetCache(تهی *&OffsetCache,
                                              MemoryBuffer *Buffer) {
  اگر (OffsetCache)
    بازگشت *static_cast<std::vector<T> *>(OffsetCache);

  // Lazily fill in the offset cache.
  auto *Offsets = جدید std::vector<T>();
  size_t Sz = Buffer->getBufferSize();
  أكد(Sz <= std::numeric_limits<T>::max());
  StringRef S = Buffer->getBuffer();
  برای (size_t N = 0; N < Sz; ++N) {
    اگر (S[N] == '\n')
      Offsets->افزودن_back(static_cast<T>(N));
  }

  OffsetCache = Offsets;
  بازگشت *Offsets;
}

template <typename T>
std::pair<unsigned, unsigned>
SourceMgr::SrcBuffer::get_سطر_And_Column_Specialized(ثابت نویسه *Ptr) ثابت {
  std::vector<T> &Offsets =
      GetOrCreateOffsetCache<T>(OffsetCache, Buffer.get());

  ثابت نویسه *BufStart = Buffer->getBufferStart();
  أكد(Ptr >= BufStart && Ptr <= Buffer->getBufferEnd());
  ptrdiff_t PtrDiff = Ptr - BufStart;
  أكد(PtrDiff >= 0 &&
         static_cast<size_t>(PtrDiff) <= std::numeric_limits<T>::max());
  T PtrOffset = static_cast<T>(PtrDiff);

  // llvm::lower_bound gives the number of EOL before PtrOffset. Add 1 to get
  // the line number.
  auto I = llvm::lower_bound(Offsets, PtrOffset);
  unsigned سطر_No = I - Offsets.begin() + 1;

  // The column number is the distance from the previous EOL (or the start of
  // the buffer) to the pointer.
  T سطر_Start_Offs = (I == Offsets.begin()) ? 0 : (I[-1] + 1);
  unsigned ColNo = PtrOffset - سطر_Start_Offs + 1;
  بازگشت {سطر_No, ColNo};
}

/// Look up a given \p Ptr in the buffer, determining which line and column
/// it came from.
LLVM_ABI std::pair<unsigned, unsigned>
SourceMgr::SrcBuffer::get_سطر_And_Column(ثابت نویسه *Ptr) ثابت {
  size_t Sz = Buffer->getBufferSize();
  اگر (Sz <= std::numeric_limits<uint8_t>::max())
    بازگشت get_سطر_And_Column_Specialized<uint8_t>(Ptr);
  اگر (Sz <= std::numeric_limits<uint16_t>::max())
    بازگشت get_سطر_And_Column_Specialized<uint16_t>(Ptr);
  اگر (Sz <= std::numeric_limits<uint32_t>::max())
    بازگشت get_سطر_And_Column_Specialized<uint32_t>(Ptr);
  بازگشت get_سطر_And_Column_Specialized<uint64_t>(Ptr);
}

template <typename T>
ثابت نویسه *SourceMgr::SrcBuffer::get_Pointer_For_سطر_عدد_Specialized(
    unsigned سطر_No) ثابت {
  std::vector<T> &Offsets =
      GetOrCreateOffsetCache<T>(OffsetCache, Buffer.get());

  // We start counting line and column numbers from 1.
  اگر (سطر_No != 0)
    --سطر_No;

  ثابت نویسه *BufStart = Buffer->getBufferStart();

  // The offset cache contains the location of the \n for the specified line,
  // we want the start of the line.  As such, we look for the previous entry.
  اگر (سطر_No == 0)
    بازگشت BufStart;
  اگر (سطر_No > Offsets.size())
    بازگشت nullptr;
  بازگشت BufStart + Offsets[سطر_No - 1] + 1;
}

/// Return a pointer to the first character of the specified line number or
/// null if the line number is invalid.
ثابت نویسه *
SourceMgr::SrcBuffer::get_Pointer_For_سطر_عدد(unsigned سطر_No) ثابت {
  size_t Sz = Buffer->getBufferSize();
  اگر (Sz <= std::numeric_limits<uint8_t>::max())
    بازگشت get_Pointer_For_سطر_عدد_Specialized<uint8_t>(سطر_No);
  وگرنه اگر (Sz <= std::numeric_limits<uint16_t>::max())
    بازگشت get_Pointer_For_سطر_عدد_Specialized<uint16_t>(سطر_No);
  وگرنه اگر (Sz <= std::numeric_limits<uint32_t>::max())
    بازگشت get_Pointer_For_سطر_عدد_Specialized<uint32_t>(سطر_No);
  وگرنه
    بازگشت get_Pointer_For_سطر_عدد_Specialized<uint64_t>(سطر_No);
}

SourceMgr::SrcBuffer::SrcBuffer(SourceMgr::SrcBuffer &&Other)
    : Buffer(std::move(Other.Buffer)), OffsetCache(Other.OffsetCache),
      IncludeLoc(Other.IncludeLoc) {
  Other.OffsetCache = nullptr;
}

SourceMgr::SrcBuffer::~SrcBuffer() {
  اگر (OffsetCache) {
    size_t Sz = Buffer->getBufferSize();
    اگر (Sz <= std::numeric_limits<uint8_t>::max())
      delete static_cast<std::vector<uint8_t> *>(OffsetCache);
    وگرنه اگر (Sz <= std::numeric_limits<uint16_t>::max())
      delete static_cast<std::vector<uint16_t> *>(OffsetCache);
    وگرنه اگر (Sz <= std::numeric_limits<uint32_t>::max())
      delete static_cast<std::vector<uint32_t> *>(OffsetCache);
    وگرنه
      delete static_cast<std::vector<uint64_t> *>(OffsetCache);
    OffsetCache = nullptr;
  }
}

std::pair<unsigned, unsigned>
SourceMgr::get_سطر_And_Column(SMLoc Loc, unsigned BufferID) ثابت {
  اگر (!BufferID)
    BufferID = FindBufferContainingLoc(Loc);
  أكد(BufferID && "Invalid location!");

  auto &SB = getBufferInfo(BufferID);
  ثابت نویسه *Ptr = Loc.getPointer();

  بازگشت SB.get_سطر_And_Column(Ptr);
}

// FIXME: Note that the formatting of source locations is spread between
// multiple functions, some in SourceMgr and some in SMDiagnostic. A better
// solution would be a general-purpose source location formatter
// in one of those two classes, or possibly in SMLoc.

/// Get a string with the source location formatted in the standard
/// style, but without the line offset. If \p IncludePath is true, the path
/// is included. If false, only the file name and extension are included.
std::string SourceMgr::getFormattedLocationNoOffset(SMLoc Loc,
                                                    منطقی IncludePath) ثابت {
  auto BufferID = FindBufferContainingLoc(Loc);
  أكد(BufferID && "Invalid location!");
  auto FileSpec = getBufferInfo(BufferID).Buffer->getBufferIdentifier();

  اگر (IncludePath) {
    بازگشت FileSpec.str() + ":" + std::to_string(Find_سطر_عدد(Loc, BufferID));
  } وگرنه {
    auto I = FileSpec.find_last_of("/\\");
    I = (I == FileSpec.size()) ? 0 : (I + 1);
    بازگشت FileSpec.substr(I).str() + ":" +
           std::to_string(Find_سطر_عدد(Loc, BufferID));
  }
}

/// Given a line and column number in a mapped buffer, turn it into an SMLoc.
/// This will return a null SMLoc if the line/column location is invalid.
SMLoc SourceMgr::Find_Loc_For_سطر_And_Column(unsigned BufferID, unsigned سطر_No,
                                         unsigned ColNo) {
  auto &SB = getBufferInfo(BufferID);
  ثابت نویسه *Ptr = SB.get_Pointer_For_سطر_عدد(سطر_No);
  اگر (!Ptr)
    بازگشت SMLoc();

  // We start counting line and column numbers from 1.
  اگر (ColNo != 0)
    --ColNo;

  // If we have a column number, validate it.
  اگر (ColNo) {
    // Make sure the location is within the current line.
    اگر (Ptr + ColNo > SB.Buffer->getBufferEnd())
      بازگشت SMLoc();

    // Make sure there is no newline in the way.
    اگر (StringRef(Ptr, ColNo).find_first_of("\n\r") != StringRef::npos)
      بازگشت SMLoc();

    Ptr += ColNo;
  }

  بازگشت SMLoc::getFromPointer(Ptr);
}

تهی SourceMgr::Print_Include_پشته(SMLoc IncludeLoc, raw_ostream &OS) ثابت {
  اگر (IncludeLoc == SMLoc())
    بازگشت; // Top of stack.

  unsigned CurBuf = FindBufferContainingLoc(IncludeLoc);
  أكد(CurBuf && "Invalid or unspecified location!");

  Print_Include_پشته(getBufferInfo(CurBuf).IncludeLoc, OS);

  OS << "Included from " << getBufferInfo(CurBuf).Buffer->getBufferIdentifier()
     << ":" << Find_سطر_عدد(IncludeLoc, CurBuf) << ":\n";
}

تهی SourceMgr::print_Include_پشته_For_Diagnostic(SMLoc Loc,
                                               raw_ostream &OS) ثابت {
  اگر (!Loc.isValid())
    بازگشت;
  unsigned DiagCurBuffer = FindBufferContainingLoc(Loc);
  اگر (DiagCurBuffer && DiagCurBuffer != getMainFileID()) {
    SMLoc ParentIncludeLoc = getParentIncludeLoc(DiagCurBuffer);
    // Ignore macro instantiation buffers to avoid redundant include stacks.
    Print_Include_پشته(ParentIncludeLoc, OS);
  }
}

SMDiagnostic SourceMgr::GetMessage(SMLoc Loc, SourceMgr::DiagKind Kind,
                                   ثابت Twine &پیام_2, ArrayRef<SMRange> Ranges,
                                   ArrayRef<SMFixIt> FixIts) ثابت {
  // First thing to do: find the current buffer containing the specified
  // location to pull out the source line.
  SmallVector<std::pair<unsigned, unsigned>, 4> ColRanges;
  std::pair<unsigned, unsigned> سطر_And_Col;
  StringRef BufferID = "<unknown>";
  StringRef سطر_Str;

  اگر (Loc.isValid()) {
    unsigned CurBuf = FindBufferContainingLoc(Loc);
    أكد(CurBuf && "Invalid or unspecified location!");

    ثابت MemoryBuffer *CurMB = getMemoryBuffer(CurBuf);
    BufferID = CurMB->getBufferIdentifier();

    // Scan backward to find the start of the line.
    ثابت نویسه *سطر_Start = Loc.getPointer();
    ثابت نویسه *BufStart = CurMB->getBufferStart();
    وقتی (سطر_Start != BufStart && سطر_Start[-1] != '\n' &&
           سطر_Start[-1] != '\r')
      --سطر_Start;

    // Get the end of the line.
    ثابت نویسه *سطر_End = Loc.getPointer();
    ثابت نویسه *BufEnd = CurMB->getBufferEnd();
    وقتی (سطر_End != BufEnd && سطر_End[0] != '\n' && سطر_End[0] != '\r')
      ++سطر_End;
    سطر_Str = StringRef(سطر_Start, سطر_End - سطر_Start);

    // Convert any ranges to column ranges that only intersect the line of the
    // location.
    برای (SMRange R : Ranges) {
      اگر (!R.isValid())
        ادامه;

      // If the line doesn't contain any part of the range, then ignore it.
      اگر (R.Start.getPointer() > سطر_End || R.End.getPointer() < سطر_Start)
        ادامه;

      // Ignore pieces of the range that go onto other lines.
      اگر (R.Start.getPointer() < سطر_Start)
        R.Start = SMLoc::getFromPointer(سطر_Start);
      اگر (R.End.getPointer() > سطر_End)
        R.End = SMLoc::getFromPointer(سطر_End);

      // Translate from SMLoc ranges to column ranges.
      // FIXME: Handle multibyte characters.
      ColRanges.افزودن_back(std::make_pair(R.Start.getPointer() - سطر_Start,
                                         R.End.getPointer() - سطر_Start));
    }

    سطر_And_Col = get_سطر_And_Column(Loc, CurBuf);
  }

  بازگشت SMDiagnostic(*این, Loc, BufferID, سطر_And_Col.first,
                      سطر_And_Col.second - 1, Kind, پیام_2.str(), سطر_Str,
                      ColRanges, FixIts);
}

تهی SourceMgr::PrintMessage(raw_ostream &OS, ثابت SMDiagnostic &Diagnostic,
                             منطقی ShowColors) ثابت {
  // Report the message with the diagnostic handler if present.
  اگر (DiagHandler) {
    DiagHandler(Diagnostic, DiagContext);
    بازگشت;
  }

  اگر (Diagnostic.getLoc().isValid()) {
    unsigned CurBuf = FindBufferContainingLoc(Diagnostic.getLoc());
    أكد(CurBuf && "Invalid or unspecified location!");
    Print_Include_پشته(getBufferInfo(CurBuf).IncludeLoc, OS);
  }

  Diagnostic.print(nullptr, OS, ShowColors);
}

تهی SourceMgr::PrintMessage(raw_ostream &OS, SMLoc Loc,
                             SourceMgr::DiagKind Kind, ثابت Twine &پیام_2,
                             ArrayRef<SMRange> Ranges, ArrayRef<SMFixIt> FixIts,
                             منطقی ShowColors) ثابت {
  PrintMessage(OS, GetMessage(Loc, Kind, پیام_2, Ranges, FixIts), ShowColors);
}

تهی SourceMgr::PrintMessage(SMLoc Loc, SourceMgr::DiagKind Kind,
                             ثابت Twine &پیام_2, ArrayRef<SMRange> Ranges,
                             ArrayRef<SMFixIt> FixIts, منطقی ShowColors) ثابت {
  PrintMessage(errs(), Loc, Kind, پیام_2, Ranges, FixIts, ShowColors);
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

SMDiagnostic::SMDiagnostic(ثابت SourceMgr &sm, SMLoc L, StringRef FN, صحیح سطر,
                           صحیح Col, SourceMgr::DiagKind Kind, StringRef پیام_2,
                           StringRef سطر_Str,
                           ArrayRef<std::pair<unsigned, unsigned>> Ranges,
                           ArrayRef<SMFixIt> Hints)
    : SM(&sm), Loc(L), Filename(std::string(FN)), سطر_No(سطر), ColumnNo(Col),
      Kind(Kind), Message(پیام_2), سطر_Contents(سطر_Str), Ranges(Ranges.vec()),
      FixIts(Hints) {
  llvm::sort(FixIts);
}

ایستا تهی build_Fix_It_سطر(std::string &Caret_سطر, std::string &Fix_It_سطر,
                           ArrayRef<SMFixIt> FixIts,
                           ArrayRef<نویسه> Source_سطر) {
  اگر (FixIts.empty())
    بازگشت;

  ثابت نویسه *سطر_Start = Source_سطر.begin();
  ثابت نویسه *سطر_End = Source_سطر.end();

  size_t PrevHintEndCol = 0;

  برای (ثابت llvm::SMFixIt &Fixit : FixIts) {
    // If the fixit contains a newline or tab, ignore it.
    اگر (Fixit.getText().find_first_of("\n\r\t") != StringRef::npos)
      ادامه;

    SMRange R = Fixit.getRange();

    // If the line doesn't contain any part of the range, then ignore it.
    اگر (R.Start.getPointer() > سطر_End || R.End.getPointer() < سطر_Start)
      ادامه;

    // Translate from SMLoc to column.
    // Ignore pieces of the range that go onto other lines.
    // FIXME: Handle multibyte characters in the source line.
    unsigned FirstCol;
    اگر (R.Start.getPointer() < سطر_Start)
      FirstCol = 0;
    وگرنه
      FirstCol = R.Start.getPointer() - سطر_Start;

    // If we inserted a long previous hint, push this one forwards, and add
    // an extra space to show that this is not part of the previous
    // completion. This is sort of the best we can do when two hints appear
    // to overlap.
    //
    // Note that if this hint is located immediately after the previous
    // hint, no space will be added, since the location is more important.
    unsigned HintCol = FirstCol;
    اگر (HintCol < PrevHintEndCol)
      HintCol = PrevHintEndCol + 1;

    // FIXME: This assertion is intended to catch unintended use of multibyte
    // characters in fixits. If we decide to do this, we'll have to track
    // separate byte widths for the source and fixit lines.
    أكد((size_t)sys::locale::columnWidth(Fixit.getText()) ==
           Fixit.getText().size());

    // This relies on one byte per column in our fixit hints.
    unsigned LastColumnModified = HintCol + Fixit.getText().size();
    اگر (LastColumnModified > Fix_It_سطر.size())
      Fix_It_سطر.resize(LastColumnModified, ' ');

    llvm::copy(Fixit.getText(), Fix_It_سطر.begin() + HintCol);

    PrevHintEndCol = LastColumnModified;

    // For replacements, mark the removal range with '~'.
    // FIXME: Handle multibyte characters in the source line.
    unsigned LastCol;
    اگر (R.End.getPointer() >= سطر_End)
      LastCol = سطر_End - سطر_Start;
    وگرنه
      LastCol = R.End.getPointer() - سطر_Start;

    std::fill(&Caret_سطر[FirstCol], &Caret_سطر[LastCol], '~');
  }
}

ایستا تهی print_Source_سطر(raw_ostream &S, StringRef سطر_Contents) {
  // Print out the source line one character at a time, so we can expand tabs.
  برای (unsigned i = 0, e = سطر_Contents.size(), OutCol = 0; i != e; ++i) {
    size_t NextTab = سطر_Contents.find('\t', i);
    // If there were no tabs left, print the rest, we are done.
    اگر (NextTab == StringRef::npos) {
      S << سطر_Contents.drop_front(i);
      بشکن;
    }

    // Otherwise, print from i to NextTab.
    S << سطر_Contents.slice(i, NextTab);
    OutCol += NextTab - i;
    i = NextTab;

    // If we have a tab, emit at least one space, then round up to 8 columns.
    انجامبده {
      S << ' ';
      ++OutCol;
    } وقتی ((OutCol % TabStop) != 0);
  }
  S << '\n';
}

ایستا منطقی isNonASCII(نویسه c) { بازگشت c & 0x80; }

تهی SMDiagnostic::print(ثابت نویسه *ProgName, raw_ostream &OS, منطقی ShowColors,
                         منطقی ShowKindLabel, منطقی ShowLocation) ثابت {
  ColorMode Mode = ShowColors ? ColorMode::Auto : ColorMode::Disable;

  {
    WithColor S(OS, raw_ostream::SAVEDCOLOR, درست, نادرست, Mode);

    اگر (ProgName && ProgName[0])
      S << ProgName << ": ";

    اگر (ShowLocation && !Filename.empty()) {
      اگر (Filename == "-")
        S << "<stdin>";
      وگرنه
        S << Filename;

      اگر (سطر_No != -1) {
        S << ':' << سطر_No;
        اگر (ColumnNo != -1)
          S << ':' << (ColumnNo + 1);
      }
      S << ": ";
    }
  }

  اگر (ShowKindLabel) {
    انتخاب (Kind) {
    حالت SourceMgr::DK_Error:
      WithColor::error(OS, "", !ShowColors);
      بشکن;
    حالت SourceMgr::DK_Warning:
      WithColor::warning(OS, "", !ShowColors);
      بشکن;
    حالت SourceMgr::DK_Note:
      WithColor::note(OS, "", !ShowColors);
      بشکن;
    حالت SourceMgr::DK_Remark:
      WithColor::remark(OS, "", !ShowColors);
      بشکن;
    }
  }

  WithColor(OS, raw_ostream::SAVEDCOLOR, درست, نادرست, Mode) << Message << '\n';

  اگر (سطر_No == -1 || ColumnNo == -1)
    بازگشت;

  // FIXME: If there are multibyte or multi-column characters in the source, all
  // our ranges will be wrong. To do this properly, we'll need a byte-to-column
  // map like Clang's TextDiagnostic. For now, we'll just handle tabs by
  // expanding them later, and bail out rather than show incorrect ranges and
  // misaligned fixits for any other odd characters.
  اگر (any_of(سطر_Contents, isNonASCII)) {
    print_Source_سطر(OS, سطر_Contents);
    بازگشت;
  }
  size_t رقم_Columns = سطر_Contents.size();

  // Build the line with the caret and ranges.
  std::string Caret_سطر(رقم_Columns + 1, ' ');

  // Expand any ranges.
  برای (ثابت std::pair<unsigned, unsigned> &R : Ranges)
    std::fill(&Caret_سطر[R.first],
              &Caret_سطر[std::min((size_t)R.second, Caret_سطر.size())], '~');

  // Add any fix-its.
  // FIXME: Find the beginning of the line properly for multibyte characters.
  std::string Fix_It_Insertion_سطر;
  build_Fix_It_سطر(Caret_سطر, Fix_It_Insertion_سطر, FixIts,
                 ArrayRef(Loc.getPointer() - ColumnNo, سطر_Contents.size()));

  // Finally, plop on the caret.
  اگر (unsigned(ColumnNo) <= رقم_Columns)
    Caret_سطر[ColumnNo] = '^';
  وگرنه
    Caret_سطر[رقم_Columns] = '^';

  // ... and remove trailing whitespace so the output doesn't wrap for it.  We
  // know that the line isn't completely empty because it has the caret in it at
  // least.
  Caret_سطر.erase(Caret_سطر.find_last_not_of(' ') + 1);

  print_Source_سطر(OS, سطر_Contents);

  {
    ColorMode Mode = ShowColors ? ColorMode::Auto : ColorMode::Disable;
    WithColor S(OS, raw_ostream::GREEN, درست, نادرست, Mode);

    // Print out the caret line, matching tabs in the source line.
    برای (unsigned i = 0, e = Caret_سطر.size(), OutCol = 0; i != e; ++i) {
      اگر (i >= سطر_Contents.size() || سطر_Contents[i] != '\t') {
        S << Caret_سطر[i];
        ++OutCol;
        ادامه;
      }

      // Okay, we have a tab.  Insert the appropriate number of characters.
      انجامبده {
        S << Caret_سطر[i];
        ++OutCol;
      } وقتی ((OutCol % TabStop) != 0);
    }
    S << '\n';
  }

  // Print out the replacement line, matching tabs in the source line.
  اگر (Fix_It_Insertion_سطر.empty())
    بازگشت;

  برای (size_t i = 0, e = Fix_It_Insertion_سطر.size(), OutCol = 0; i < e; ++i) {
    اگر (i >= سطر_Contents.size() || سطر_Contents[i] != '\t') {
      OS << Fix_It_Insertion_سطر[i];
      ++OutCol;
      ادامه;
    }

    // Okay, we have a tab.  Insert the appropriate number of characters.
    انجامبده {
      OS << Fix_It_Insertion_سطر[i];
      // FIXME: This is trying not to break up replacements, but then to re-sync
      // with the tabs between replacements. This will fail, though, if two
      // fix-it replacements are exactly adjacent, or if a fix-it contains a
      // space. Really we should be precomputing column widths, which we'll
      // need anyway for multibyte chars.
      اگر (Fix_It_Insertion_سطر[i] != ' ')
        ++i;
      ++OutCol;
    } وقتی (((OutCol % TabStop) != 0) && i != e);
  }
  OS << '\n';
}
