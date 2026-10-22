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
  إذا (!NewBufOrErr)
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

  إذا (NewBufOrErr)
    IncludedFile = static_cast<std::string>(Buffer);

  أرجع NewBufOrErr;
}

unsigned SourceMgr::FindBufferContainingLoc(SMLoc Loc) ثابت {
  لكل (unsigned i = 0, e = Buffers.size(); i != e; ++i)
    إذا (Loc.getPointer() >= Buffers[i].Buffer->getBufferStart() &&
        // Use <= here so that a pointer to the null at the end of the buffer
        // is included as part of the buffer.
        Loc.getPointer() <= Buffers[i].Buffer->getBufferEnd())
      أرجع i + 1;
  أرجع 0;
}

template <typename T>
ساكن std::vector<T> &GetOrCreateOffsetCache(فراغ *&OffsetCache,
                                              MemoryBuffer *Buffer) {
  إذا (OffsetCache)
    أرجع *static_cast<std::vector<T> *>(OffsetCache);

  // Lazily fill in the offset cache.
  auto *Offsets = جديد std::vector<T>();
  size_t Sz = Buffer->getBufferSize();
  أكد(Sz <= std::numeric_limits<T>::max());
  StringRef S = Buffer->getBuffer();
  لكل (size_t N = 0; N < Sz; ++N) {
    إذا (S[N] == '\n')
      Offsets->ئىتتىرىش_back(static_cast<T>(N));
  }

  OffsetCache = Offsets;
  أرجع *Offsets;
}

template <typename T>
std::pair<unsigned, unsigned>
SourceMgr::SrcBuffer::get_قۇر_And_Column_Specialized(ثابت حرف *Ptr) ثابت {
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
  unsigned قۇر_No = I - Offsets.begin() + 1;

  // The column number is the distance from the previous EOL (or the start of
  // the buffer) to the pointer.
  T قۇر_Start_Offs = (I == Offsets.begin()) ? 0 : (I[-1] + 1);
  unsigned ColNo = PtrOffset - قۇر_Start_Offs + 1;
  أرجع {قۇر_No, ColNo};
}

/// Look up a given \p Ptr in the buffer, determining which line and column
/// it came from.
LLVM_ABI std::pair<unsigned, unsigned>
SourceMgr::SrcBuffer::get_قۇر_And_Column(ثابت حرف *Ptr) ثابت {
  size_t Sz = Buffer->getBufferSize();
  إذا (Sz <= std::numeric_limits<uint8_t>::max())
    أرجع get_قۇر_And_Column_Specialized<uint8_t>(Ptr);
  إذا (Sz <= std::numeric_limits<uint16_t>::max())
    أرجع get_قۇر_And_Column_Specialized<uint16_t>(Ptr);
  إذا (Sz <= std::numeric_limits<uint32_t>::max())
    أرجع get_قۇر_And_Column_Specialized<uint32_t>(Ptr);
  أرجع get_قۇر_And_Column_Specialized<uint64_t>(Ptr);
}

template <typename T>
ثابت حرف *SourceMgr::SrcBuffer::get_Pointer_For_قۇر_سان_Specialized(
    unsigned قۇر_No) ثابت {
  std::vector<T> &Offsets =
      GetOrCreateOffsetCache<T>(OffsetCache, Buffer.get());

  // We start counting line and column numbers from 1.
  إذا (قۇر_No != 0)
    --قۇر_No;

  ثابت حرف *BufStart = Buffer->getBufferStart();

  // The offset cache contains the location of the \n for the specified line,
  // we want the start of the line.  As such, we look for the previous entry.
  إذا (قۇر_No == 0)
    أرجع BufStart;
  إذا (قۇر_No > Offsets.size())
    أرجع nullptr;
  أرجع BufStart + Offsets[قۇر_No - 1] + 1;
}

/// Return a pointer to the first character of the specified line number or
/// null if the line number is invalid.
ثابت حرف *
SourceMgr::SrcBuffer::get_Pointer_For_قۇر_سان(unsigned قۇر_No) ثابت {
  size_t Sz = Buffer->getBufferSize();
  إذا (Sz <= std::numeric_limits<uint8_t>::max())
    أرجع get_Pointer_For_قۇر_سان_Specialized<uint8_t>(قۇر_No);
  وإلا إذا (Sz <= std::numeric_limits<uint16_t>::max())
    أرجع get_Pointer_For_قۇر_سان_Specialized<uint16_t>(قۇر_No);
  وإلا إذا (Sz <= std::numeric_limits<uint32_t>::max())
    أرجع get_Pointer_For_قۇر_سان_Specialized<uint32_t>(قۇر_No);
  وإلا
    أرجع get_Pointer_For_قۇر_سان_Specialized<uint64_t>(قۇر_No);
}

SourceMgr::SrcBuffer::SrcBuffer(SourceMgr::SrcBuffer &&Other)
    : Buffer(std::move(Other.Buffer)), OffsetCache(Other.OffsetCache),
      IncludeLoc(Other.IncludeLoc) {
  Other.OffsetCache = nullptr;
}

SourceMgr::SrcBuffer::~SrcBuffer() {
  إذا (OffsetCache) {
    size_t Sz = Buffer->getBufferSize();
    إذا (Sz <= std::numeric_limits<uint8_t>::max())
      delete static_cast<std::vector<uint8_t> *>(OffsetCache);
    وإلا إذا (Sz <= std::numeric_limits<uint16_t>::max())
      delete static_cast<std::vector<uint16_t> *>(OffsetCache);
    وإلا إذا (Sz <= std::numeric_limits<uint32_t>::max())
      delete static_cast<std::vector<uint32_t> *>(OffsetCache);
    وإلا
      delete static_cast<std::vector<uint64_t> *>(OffsetCache);
    OffsetCache = nullptr;
  }
}

std::pair<unsigned, unsigned>
SourceMgr::get_قۇر_And_Column(SMLoc Loc, unsigned BufferID) ثابت {
  إذا (!BufferID)
    BufferID = FindBufferContainingLoc(Loc);
  أكد(BufferID && "Invalid location!");

  auto &SB = getBufferInfo(BufferID);
  ثابت حرف *Ptr = Loc.getPointer();

  أرجع SB.get_قۇر_And_Column(Ptr);
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

  إذا (IncludePath) {
    أرجع FileSpec.str() + ":" + std::to_string(Find_قۇر_سان(Loc, BufferID));
  } وإلا {
    auto I = FileSpec.find_last_of("/\\");
    I = (I == FileSpec.size()) ? 0 : (I + 1);
    أرجع FileSpec.substr(I).str() + ":" +
           std::to_string(Find_قۇر_سان(Loc, BufferID));
  }
}

/// Given a line and column number in a mapped buffer, turn it into an SMLoc.
/// This will return a null SMLoc if the line/column location is invalid.
SMLoc SourceMgr::Find_Loc_For_قۇر_And_Column(unsigned BufferID, unsigned قۇر_No,
                                         unsigned ColNo) {
  auto &SB = getBufferInfo(BufferID);
  ثابت حرف *Ptr = SB.get_Pointer_For_قۇر_سان(قۇر_No);
  إذا (!Ptr)
    أرجع SMLoc();

  // We start counting line and column numbers from 1.
  إذا (ColNo != 0)
    --ColNo;

  // If we have a column number, validate it.
  إذا (ColNo) {
    // Make sure the location is within the current line.
    إذا (Ptr + ColNo > SB.Buffer->getBufferEnd())
      أرجع SMLoc();

    // Make sure there is no newline in the way.
    إذا (StringRef(Ptr, ColNo).find_first_of("\n\r") != StringRef::npos)
      أرجع SMLoc();

    Ptr += ColNo;
  }

  أرجع SMLoc::getFromPointer(Ptr);
}

فراغ SourceMgr::Print_Include_دۆۋە(SMLoc IncludeLoc, raw_ostream &OS) ثابت {
  إذا (IncludeLoc == SMLoc())
    أرجع; // Top of stack.

  unsigned CurBuf = FindBufferContainingLoc(IncludeLoc);
  أكد(CurBuf && "Invalid or unspecified location!");

  Print_Include_دۆۋە(getBufferInfo(CurBuf).IncludeLoc, OS);

  OS << "Included from " << getBufferInfo(CurBuf).Buffer->getBufferIdentifier()
     << ":" << Find_قۇر_سان(IncludeLoc, CurBuf) << ":\n";
}

فراغ SourceMgr::print_Include_دۆۋە_For_Diagnostic(SMLoc Loc,
                                               raw_ostream &OS) ثابت {
  إذا (!Loc.isValid())
    أرجع;
  unsigned DiagCurBuffer = FindBufferContainingLoc(Loc);
  إذا (DiagCurBuffer && DiagCurBuffer != getMainFileID()) {
    SMLoc ParentIncludeLoc = getParentIncludeLoc(DiagCurBuffer);
    // Ignore macro instantiation buffers to avoid redundant include stacks.
    Print_Include_دۆۋە(ParentIncludeLoc, OS);
  }
}

SMDiagnostic SourceMgr::GetMessage(SMLoc Loc, SourceMgr::DiagKind Kind,
                                   ثابت Twine &ئۇچۇر_2, ArrayRef<SMRange> Ranges,
                                   ArrayRef<SMFixIt> FixIts) ثابت {
  // First thing to do: find the current buffer containing the specified
  // location to pull out the source line.
  SmallVector<std::pair<unsigned, unsigned>, 4> ColRanges;
  std::pair<unsigned, unsigned> قۇر_And_Col;
  StringRef BufferID = "<unknown>";
  StringRef قۇر_Str;

  إذا (Loc.isValid()) {
    unsigned CurBuf = FindBufferContainingLoc(Loc);
    أكد(CurBuf && "Invalid or unspecified location!");

    ثابت MemoryBuffer *CurMB = getMemoryBuffer(CurBuf);
    BufferID = CurMB->getBufferIdentifier();

    // Scan backward to find the start of the line.
    ثابت حرف *قۇر_Start = Loc.getPointer();
    ثابت حرف *BufStart = CurMB->getBufferStart();
    طالما (قۇر_Start != BufStart && قۇر_Start[-1] != '\n' &&
           قۇر_Start[-1] != '\r')
      --قۇر_Start;

    // Get the end of the line.
    ثابت حرف *قۇر_End = Loc.getPointer();
    ثابت حرف *BufEnd = CurMB->getBufferEnd();
    طالما (قۇر_End != BufEnd && قۇر_End[0] != '\n' && قۇر_End[0] != '\r')
      ++قۇر_End;
    قۇر_Str = StringRef(قۇر_Start, قۇر_End - قۇر_Start);

    // Convert any ranges to column ranges that only intersect the line of the
    // location.
    لكل (SMRange R : Ranges) {
      إذا (!R.isValid())
        استمر;

      // If the line doesn't contain any part of the range, then ignore it.
      إذا (R.Start.getPointer() > قۇر_End || R.End.getPointer() < قۇر_Start)
        استمر;

      // Ignore pieces of the range that go onto other lines.
      إذا (R.Start.getPointer() < قۇر_Start)
        R.Start = SMLoc::getFromPointer(قۇر_Start);
      إذا (R.End.getPointer() > قۇر_End)
        R.End = SMLoc::getFromPointer(قۇر_End);

      // Translate from SMLoc ranges to column ranges.
      // FIXME: Handle multibyte characters.
      ColRanges.ئىتتىرىش_back(std::make_pair(R.Start.getPointer() - قۇر_Start,
                                         R.End.getPointer() - قۇر_Start));
    }

    قۇر_And_Col = get_قۇر_And_Column(Loc, CurBuf);
  }

  أرجع SMDiagnostic(*هذا, Loc, BufferID, قۇر_And_Col.first,
                      قۇر_And_Col.second - 1, Kind, ئۇچۇر_2.str(), قۇر_Str,
                      ColRanges, FixIts);
}

فراغ SourceMgr::PrintMessage(raw_ostream &OS, ثابت SMDiagnostic &Diagnostic,
                             منطقي ShowColors) ثابت {
  // Report the message with the diagnostic handler if present.
  إذا (DiagHandler) {
    DiagHandler(Diagnostic, DiagContext);
    أرجع;
  }

  إذا (Diagnostic.getLoc().isValid()) {
    unsigned CurBuf = FindBufferContainingLoc(Diagnostic.getLoc());
    أكد(CurBuf && "Invalid or unspecified location!");
    Print_Include_دۆۋە(getBufferInfo(CurBuf).IncludeLoc, OS);
  }

  Diagnostic.print(nullptr, OS, ShowColors);
}

فراغ SourceMgr::PrintMessage(raw_ostream &OS, SMLoc Loc,
                             SourceMgr::DiagKind Kind, ثابت Twine &ئۇچۇر_2,
                             ArrayRef<SMRange> Ranges, ArrayRef<SMFixIt> FixIts,
                             منطقي ShowColors) ثابت {
  PrintMessage(OS, GetMessage(Loc, Kind, ئۇچۇر_2, Ranges, FixIts), ShowColors);
}

فراغ SourceMgr::PrintMessage(SMLoc Loc, SourceMgr::DiagKind Kind,
                             ثابت Twine &ئۇچۇر_2, ArrayRef<SMRange> Ranges,
                             ArrayRef<SMFixIt> FixIts, منطقي ShowColors) ثابت {
  PrintMessage(errs(), Loc, Kind, ئۇچۇر_2, Ranges, FixIts, ShowColors);
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

SMDiagnostic::SMDiagnostic(ثابت SourceMgr &sm, SMLoc L, StringRef FN, صحيح قۇر,
                           صحيح Col, SourceMgr::DiagKind Kind, StringRef ئۇچۇر_2,
                           StringRef قۇر_Str,
                           ArrayRef<std::pair<unsigned, unsigned>> Ranges,
                           ArrayRef<SMFixIt> Hints)
    : SM(&sm), Loc(L), Filename(std::string(FN)), قۇر_No(قۇر), ColumnNo(Col),
      Kind(Kind), Message(ئۇچۇر_2), قۇر_Contents(قۇر_Str), Ranges(Ranges.vec()),
      FixIts(Hints) {
  llvm::sort(FixIts);
}

ساكن فراغ build_Fix_It_قۇر(std::string &Caret_قۇر, std::string &Fix_It_قۇر,
                           ArrayRef<SMFixIt> FixIts,
                           ArrayRef<حرف> Source_قۇر) {
  إذا (FixIts.empty())
    أرجع;

  ثابت حرف *قۇر_Start = Source_قۇر.begin();
  ثابت حرف *قۇر_End = Source_قۇر.end();

  size_t PrevHintEndCol = 0;

  لكل (ثابت llvm::SMFixIt &Fixit : FixIts) {
    // If the fixit contains a newline or tab, ignore it.
    إذا (Fixit.getText().find_first_of("\n\r\t") != StringRef::npos)
      استمر;

    SMRange R = Fixit.getRange();

    // If the line doesn't contain any part of the range, then ignore it.
    إذا (R.Start.getPointer() > قۇر_End || R.End.getPointer() < قۇر_Start)
      استمر;

    // Translate from SMLoc to column.
    // Ignore pieces of the range that go onto other lines.
    // FIXME: Handle multibyte characters in the source line.
    unsigned FirstCol;
    إذا (R.Start.getPointer() < قۇر_Start)
      FirstCol = 0;
    وإلا
      FirstCol = R.Start.getPointer() - قۇر_Start;

    // If we inserted a long previous hint, push this one forwards, and add
    // an extra space to show that this is not part of the previous
    // completion. This is sort of the best we can do when two hints appear
    // to overlap.
    //
    // Note that if this hint is located immediately after the previous
    // hint, no space will be added, since the location is more important.
    unsigned HintCol = FirstCol;
    إذا (HintCol < PrevHintEndCol)
      HintCol = PrevHintEndCol + 1;

    // FIXME: This assertion is intended to catch unintended use of multibyte
    // characters in fixits. If we decide to do this, we'll have to track
    // separate byte widths for the source and fixit lines.
    أكد((size_t)sys::locale::columnWidth(Fixit.getText()) ==
           Fixit.getText().size());

    // This relies on one byte per column in our fixit hints.
    unsigned LastColumnModified = HintCol + Fixit.getText().size();
    إذا (LastColumnModified > Fix_It_قۇر.size())
      Fix_It_قۇر.resize(LastColumnModified, ' ');

    llvm::copy(Fixit.getText(), Fix_It_قۇر.begin() + HintCol);

    PrevHintEndCol = LastColumnModified;

    // For replacements, mark the removal range with '~'.
    // FIXME: Handle multibyte characters in the source line.
    unsigned LastCol;
    إذا (R.End.getPointer() >= قۇر_End)
      LastCol = قۇر_End - قۇر_Start;
    وإلا
      LastCol = R.End.getPointer() - قۇر_Start;

    std::fill(&Caret_قۇر[FirstCol], &Caret_قۇر[LastCol], '~');
  }
}

ساكن فراغ print_Source_قۇر(raw_ostream &S, StringRef قۇر_Contents) {
  // Print out the source line one character at a time, so we can expand tabs.
  لكل (unsigned i = 0, e = قۇر_Contents.size(), OutCol = 0; i != e; ++i) {
    size_t NextTab = قۇر_Contents.find('\t', i);
    // If there were no tabs left, print the rest, we are done.
    إذا (NextTab == StringRef::npos) {
      S << قۇر_Contents.drop_front(i);
      اكسر;
    }

    // Otherwise, print from i to NextTab.
    S << قۇر_Contents.slice(i, NextTab);
    OutCol += NextTab - i;
    i = NextTab;

    // If we have a tab, emit at least one space, then round up to 8 columns.
    افعل {
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

    إذا (ProgName && ProgName[0])
      S << ProgName << ": ";

    إذا (ShowLocation && !Filename.empty()) {
      إذا (Filename == "-")
        S << "<stdin>";
      وإلا
        S << Filename;

      إذا (قۇر_No != -1) {
        S << ':' << قۇر_No;
        إذا (ColumnNo != -1)
          S << ':' << (ColumnNo + 1);
      }
      S << ": ";
    }
  }

  إذا (ShowKindLabel) {
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

  إذا (قۇر_No == -1 || ColumnNo == -1)
    أرجع;

  // FIXME: If there are multibyte or multi-column characters in the source, all
  // our ranges will be wrong. To do this properly, we'll need a byte-to-column
  // map like Clang's TextDiagnostic. For now, we'll just handle tabs by
  // expanding them later, and bail out rather than show incorrect ranges and
  // misaligned fixits for any other odd characters.
  إذا (any_of(قۇر_Contents, isNonASCII)) {
    print_Source_قۇر(OS, قۇر_Contents);
    أرجع;
  }
  size_t رەقەم_Columns = قۇر_Contents.size();

  // Build the line with the caret and ranges.
  std::string Caret_قۇر(رەقەم_Columns + 1, ' ');

  // Expand any ranges.
  لكل (ثابت std::pair<unsigned, unsigned> &R : Ranges)
    std::fill(&Caret_قۇر[R.first],
              &Caret_قۇر[std::min((size_t)R.second, Caret_قۇر.size())], '~');

  // Add any fix-its.
  // FIXME: Find the beginning of the line properly for multibyte characters.
  std::string Fix_It_Insertion_قۇر;
  build_Fix_It_قۇر(Caret_قۇر, Fix_It_Insertion_قۇر, FixIts,
                 ArrayRef(Loc.getPointer() - ColumnNo, قۇر_Contents.size()));

  // Finally, plop on the caret.
  إذا (unsigned(ColumnNo) <= رەقەم_Columns)
    Caret_قۇر[ColumnNo] = '^';
  وإلا
    Caret_قۇر[رەقەم_Columns] = '^';

  // ... and remove trailing whitespace so the output doesn't wrap for it.  We
  // know that the line isn't completely empty because it has the caret in it at
  // least.
  Caret_قۇر.erase(Caret_قۇر.find_last_not_of(' ') + 1);

  print_Source_قۇر(OS, قۇر_Contents);

  {
    ColorMode Mode = ShowColors ? ColorMode::Auto : ColorMode::Disable;
    WithColor S(OS, raw_ostream::GREEN, true, خطأ, Mode);

    // Print out the caret line, matching tabs in the source line.
    لكل (unsigned i = 0, e = Caret_قۇر.size(), OutCol = 0; i != e; ++i) {
      إذا (i >= قۇر_Contents.size() || قۇر_Contents[i] != '\t') {
        S << Caret_قۇر[i];
        ++OutCol;
        استمر;
      }

      // Okay, we have a tab.  Insert the appropriate number of characters.
      افعل {
        S << Caret_قۇر[i];
        ++OutCol;
      } طالما ((OutCol % TabStop) != 0);
    }
    S << '\n';
  }

  // Print out the replacement line, matching tabs in the source line.
  إذا (Fix_It_Insertion_قۇر.empty())
    أرجع;

  لكل (size_t i = 0, e = Fix_It_Insertion_قۇر.size(), OutCol = 0; i < e; ++i) {
    إذا (i >= قۇر_Contents.size() || قۇر_Contents[i] != '\t') {
      OS << Fix_It_Insertion_قۇر[i];
      ++OutCol;
      استمر;
    }

    // Okay, we have a tab.  Insert the appropriate number of characters.
    افعل {
      OS << Fix_It_Insertion_قۇر[i];
      // FIXME: This is trying not to break up replacements, but then to re-sync
      // with the tabs between replacements. This will fail, though, if two
      // fix-it replacements are exactly adjacent, or if a fix-it contains a
      // space. Really we should be precomputing column widths, which we'll
      // need anyway for multibyte chars.
      إذا (Fix_It_Insertion_قۇر[i] != ' ')
        ++i;
      ++OutCol;
    } طالما (((OutCol % TabStop) != 0) && i != e);
  }
  OS << '\n';
}
