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

#ಅನ್ತರ್ಭಾವ "llvm/Support/SourceMgr.h"
#ಅನ್ತರ್ಭಾವ "llvm/ADT/ArrayRef.h"
#ಅನ್ತರ್ಭಾವ "llvm/ADT/STLExtras.h"
#ಅನ್ತರ್ಭಾವ "llvm/ADT/SmallString.h"
#ಅನ್ತರ್ಭಾವ "llvm/ADT/SmallVector.h"
#ಅನ್ತರ್ಭಾವ "llvm/ADT/StringRef.h"
#ಅನ್ತರ್ಭಾವ "llvm/ADT/Twine.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/ErrorOr.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/Locale.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/MemoryBuffer.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/Path.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/SMLoc.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/VirtualFileSystem.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/WithColor.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/raw_ostream.h"
#ಅನ್ತರ್ಭಾವ <algorithm>
#ಅನ್ತರ್ಭಾವ <cassert>
#ಅನ್ತರ್ಭಾವ <cstddef>
#ಅನ್ತರ್ಭಾವ <limits>
#ಅನ್ತರ್ಭಾವ <memory>
#ಅನ್ತರ್ಭಾವ <string>
#ಅನ್ತರ್ಭಾವ <utility>

using ನಾಮಾಕಾಶ llvm;

ಸ್ಥಿರ ಧ್ರುವ size_t TabStop = 8;

// Out of line to avoid needing definition of vfs::FileSystem in header.
SourceMgr::SourceMgr() = ಸ್ವಭಾವ;
SourceMgr::SourceMgr(IntrusiveRefCntPtr<vfs::FileSystem> FS)
    : FS(std::move(FS)) {}
SourceMgr::SourceMgr(SourceMgr &&) = ಸ್ವಭಾವ;
SourceMgr &SourceMgr::operator=(SourceMgr &&) = ಸ್ವಭಾವ;
SourceMgr::~SourceMgr() = ಸ್ವಭಾವ;

IntrusiveRefCntPtr<vfs::FileSystem> SourceMgr::getVirtualFileSystem() ಧ್ರುವ {
  ಪ್ರತ್ಯಾಗಮ FS;
}

ಶೂನ್ಯ SourceMgr::setVirtualFileSystem(IntrusiveRefCntPtr<vfs::FileSystem> FS) {
  ಏತತ್->FS = std::move(FS);
}

unsigned SourceMgr::AddIncludeFile(ಧ್ರುವ std::string &Filename,
                                   SMLoc IncludeLoc,
                                   std::string &IncludedFile) {
  ErrorOr<std::unique_ptr<MemoryBuffer>> NewBufOrErr =
      OpenIncludeFile(Filename, IncludedFile);
  ಯದಿ (!NewBufOrErr)
    ಪ್ರತ್ಯಾಗಮ 0;

  ಪ್ರತ್ಯಾಗಮ AddNewSourceBuffer(std::move(*NewBufOrErr), IncludeLoc);
}

ErrorOr<std::unique_ptr<MemoryBuffer>>
SourceMgr::OpenIncludeFile(ಧ್ರುವ std::string &Filename,
                           std::string &IncludedFile,
                           ತಾರ್ಕಿಕ RequiresNullTerminator) {
  auto GetFile = [ಏತತ್, RequiresNullTerminator](StringRef Path) {
    ಪ್ರತ್ಯಾಗಮ FS ? FS->getBufferForFile(Path, /*FileSize=*/-1,
                                     RequiresNullTerminator)
              : MemoryBuffer::getFile(Path, /*IsText=*/ಅಸತ್ಯ,
                                      RequiresNullTerminator);
  };

  ErrorOr<std::unique_ptr<MemoryBuffer>> NewBufOrErr = GetFile(Filename);

  SmallString<64> Buffer(Filename);
  // If the file didn't exist directly, see if it's in an include path.
  ಪ್ರತಿ (unsigned i = 0, e = IncludeDirectories.size(); i != e && !NewBufOrErr;
       ++i) {
    Buffer = IncludeDirectories[i];
    sys::path::append(Buffer, Filename);
    NewBufOrErr = GetFile(Buffer);
  }

  ಯದಿ (NewBufOrErr)
    IncludedFile = static_cast<std::string>(Buffer);

  ಪ್ರತ್ಯಾಗಮ NewBufOrErr;
}

unsigned SourceMgr::FindBufferContainingLoc(SMLoc Loc) ಧ್ರುವ {
  ಪ್ರತಿ (unsigned i = 0, e = Buffers.size(); i != e; ++i)
    ಯದಿ (Loc.getPointer() >= Buffers[i].Buffer->getBufferStart() &&
        // Use <= here so that a pointer to the null at the end of the buffer
        // is included as part of the buffer.
        Loc.getPointer() <= Buffers[i].Buffer->getBufferEnd())
      ಪ್ರತ್ಯಾಗಮ i + 1;
  ಪ್ರತ್ಯಾಗಮ 0;
}

template <typename T>
ಸ್ಥಿರ std::vector<T> &GetOrCreateOffsetCache(ಶೂನ್ಯ *&OffsetCache,
                                              MemoryBuffer *Buffer) {
  ಯದಿ (OffsetCache)
    ಪ್ರತ್ಯಾಗಮ *static_cast<std::vector<T> *>(OffsetCache);

  // Lazily fill in the offset cache.
  auto *Offsets = ನೂತನ std::vector<T>();
  size_t Sz = Buffer->getBufferSize();
  ಪ್ರತಿಜ್ಞಾ(Sz <= std::numeric_limits<T>::max());
  StringRef S = Buffer->getBuffer();
  ಪ್ರತಿ (size_t N = 0; N < Sz; ++N) {
    ಯದಿ (S[N] == '\n')
      Offsets->ತಳ್ಳು_back(static_cast<T>(N));
  }

  OffsetCache = Offsets;
  ಪ್ರತ್ಯಾಗಮ *Offsets;
}

template <typename T>
std::pair<unsigned, unsigned>
SourceMgr::SrcBuffer::get_ಸಾಲು_And_Column_Specialized(ಧ್ರುವ ವರ್ಣ *Ptr) ಧ್ರುವ {
  std::vector<T> &Offsets =
      GetOrCreateOffsetCache<T>(OffsetCache, Buffer.get());

  ಧ್ರುವ ವರ್ಣ *BufStart = Buffer->getBufferStart();
  ಪ್ರತಿಜ್ಞಾ(Ptr >= BufStart && Ptr <= Buffer->getBufferEnd());
  ptrdiff_t PtrDiff = Ptr - BufStart;
  ಪ್ರತಿಜ್ಞಾ(PtrDiff >= 0 &&
         static_cast<size_t>(PtrDiff) <= std::numeric_limits<T>::max());
  T PtrOffset = static_cast<T>(PtrDiff);

  // llvm::lower_bound gives the number of EOL before PtrOffset. Add 1 to get
  // the line number.
  auto I = llvm::lower_bound(Offsets, PtrOffset);
  unsigned ಸಾಲು_No = I - Offsets.begin() + 1;

  // The column number is the distance from the previous EOL (or the start of
  // the buffer) to the pointer.
  T ಸಾಲು_Start_Offs = (I == Offsets.begin()) ? 0 : (I[-1] + 1);
  unsigned ColNo = PtrOffset - ಸಾಲು_Start_Offs + 1;
  ಪ್ರತ್ಯಾಗಮ {ಸಾಲು_No, ColNo};
}

/// Look up a given \p Ptr in the buffer, determining which line and column
/// it came from.
LLVM_ABI std::pair<unsigned, unsigned>
SourceMgr::SrcBuffer::get_ಸಾಲು_And_Column(ಧ್ರುವ ವರ್ಣ *Ptr) ಧ್ರುವ {
  size_t Sz = Buffer->getBufferSize();
  ಯದಿ (Sz <= std::numeric_limits<uint8_t>::max())
    ಪ್ರತ್ಯಾಗಮ get_ಸಾಲು_And_Column_Specialized<uint8_t>(Ptr);
  ಯದಿ (Sz <= std::numeric_limits<uint16_t>::max())
    ಪ್ರತ್ಯಾಗಮ get_ಸಾಲು_And_Column_Specialized<uint16_t>(Ptr);
  ಯದಿ (Sz <= std::numeric_limits<uint32_t>::max())
    ಪ್ರತ್ಯಾಗಮ get_ಸಾಲು_And_Column_Specialized<uint32_t>(Ptr);
  ಪ್ರತ್ಯಾಗಮ get_ಸಾಲು_And_Column_Specialized<uint64_t>(Ptr);
}

template <typename T>
ಧ್ರುವ ವರ್ಣ *SourceMgr::SrcBuffer::get_Pointer_For_ಸಾಲು_ಸಂಖ್ಯೆ_Specialized(
    unsigned ಸಾಲು_No) ಧ್ರುವ {
  std::vector<T> &Offsets =
      GetOrCreateOffsetCache<T>(OffsetCache, Buffer.get());

  // We start counting line and column numbers from 1.
  ಯದಿ (ಸಾಲು_No != 0)
    --ಸಾಲು_No;

  ಧ್ರುವ ವರ್ಣ *BufStart = Buffer->getBufferStart();

  // The offset cache contains the location of the \n for the specified line,
  // we want the start of the line.  As such, we look for the previous entry.
  ಯದಿ (ಸಾಲು_No == 0)
    ಪ್ರತ್ಯಾಗಮ BufStart;
  ಯದಿ (ಸಾಲು_No > Offsets.size())
    ಪ್ರತ್ಯಾಗಮ nullptr;
  ಪ್ರತ್ಯಾಗಮ BufStart + Offsets[ಸಾಲು_No - 1] + 1;
}

/// Return a pointer to the first character of the specified line number or
/// null if the line number is invalid.
ಧ್ರುವ ವರ್ಣ *
SourceMgr::SrcBuffer::get_Pointer_For_ಸಾಲು_ಸಂಖ್ಯೆ(unsigned ಸಾಲು_No) ಧ್ರುವ {
  size_t Sz = Buffer->getBufferSize();
  ಯದಿ (Sz <= std::numeric_limits<uint8_t>::max())
    ಪ್ರತ್ಯಾಗಮ get_Pointer_For_ಸಾಲು_ಸಂಖ್ಯೆ_Specialized<uint8_t>(ಸಾಲು_No);
  ಅನ್ಯಥಾ ಯದಿ (Sz <= std::numeric_limits<uint16_t>::max())
    ಪ್ರತ್ಯಾಗಮ get_Pointer_For_ಸಾಲು_ಸಂಖ್ಯೆ_Specialized<uint16_t>(ಸಾಲು_No);
  ಅನ್ಯಥಾ ಯದಿ (Sz <= std::numeric_limits<uint32_t>::max())
    ಪ್ರತ್ಯಾಗಮ get_Pointer_For_ಸಾಲು_ಸಂಖ್ಯೆ_Specialized<uint32_t>(ಸಾಲು_No);
  ಅನ್ಯಥಾ
    ಪ್ರತ್ಯಾಗಮ get_Pointer_For_ಸಾಲು_ಸಂಖ್ಯೆ_Specialized<uint64_t>(ಸಾಲು_No);
}

SourceMgr::SrcBuffer::SrcBuffer(SourceMgr::SrcBuffer &&Other)
    : Buffer(std::move(Other.Buffer)), OffsetCache(Other.OffsetCache),
      IncludeLoc(Other.IncludeLoc) {
  Other.OffsetCache = nullptr;
}

SourceMgr::SrcBuffer::~SrcBuffer() {
  ಯದಿ (OffsetCache) {
    size_t Sz = Buffer->getBufferSize();
    ಯದಿ (Sz <= std::numeric_limits<uint8_t>::max())
      delete static_cast<std::vector<uint8_t> *>(OffsetCache);
    ಅನ್ಯಥಾ ಯದಿ (Sz <= std::numeric_limits<uint16_t>::max())
      delete static_cast<std::vector<uint16_t> *>(OffsetCache);
    ಅನ್ಯಥಾ ಯದಿ (Sz <= std::numeric_limits<uint32_t>::max())
      delete static_cast<std::vector<uint32_t> *>(OffsetCache);
    ಅನ್ಯಥಾ
      delete static_cast<std::vector<uint64_t> *>(OffsetCache);
    OffsetCache = nullptr;
  }
}

std::pair<unsigned, unsigned>
SourceMgr::get_ಸಾಲು_And_Column(SMLoc Loc, unsigned BufferID) ಧ್ರುವ {
  ಯದಿ (!BufferID)
    BufferID = FindBufferContainingLoc(Loc);
  ಪ್ರತಿಜ್ಞಾ(BufferID && "Invalid location!");

  auto &SB = getBufferInfo(BufferID);
  ಧ್ರುವ ವರ್ಣ *Ptr = Loc.getPointer();

  ಪ್ರತ್ಯಾಗಮ SB.get_ಸಾಲು_And_Column(Ptr);
}

// FIXME: Note that the formatting of source locations is spread between
// multiple functions, some in SourceMgr and some in SMDiagnostic. A better
// solution would be a general-purpose source location formatter
// in one of those two classes, or possibly in SMLoc.

/// Get a string with the source location formatted in the standard
/// style, but without the line offset. If \p IncludePath is true, the path
/// is included. If false, only the file name and extension are included.
std::string SourceMgr::getFormattedLocationNoOffset(SMLoc Loc,
                                                    ತಾರ್ಕಿಕ IncludePath) ಧ್ರುವ {
  auto BufferID = FindBufferContainingLoc(Loc);
  ಪ್ರತಿಜ್ಞಾ(BufferID && "Invalid location!");
  auto FileSpec = getBufferInfo(BufferID).Buffer->getBufferIdentifier();

  ಯದಿ (IncludePath) {
    ಪ್ರತ್ಯಾಗಮ FileSpec.str() + ":" + std::to_string(Find_ಸಾಲು_ಸಂಖ್ಯೆ(Loc, BufferID));
  } ಅನ್ಯಥಾ {
    auto I = FileSpec.find_last_of("/\\");
    I = (I == FileSpec.size()) ? 0 : (I + 1);
    ಪ್ರತ್ಯಾಗಮ FileSpec.substr(I).str() + ":" +
           std::to_string(Find_ಸಾಲು_ಸಂಖ್ಯೆ(Loc, BufferID));
  }
}

/// Given a line and column number in a mapped buffer, turn it into an SMLoc.
/// This will return a null SMLoc if the line/column location is invalid.
SMLoc SourceMgr::Find_Loc_For_ಸಾಲು_And_Column(unsigned BufferID, unsigned ಸಾಲು_No,
                                         unsigned ColNo) {
  auto &SB = getBufferInfo(BufferID);
  ಧ್ರುವ ವರ್ಣ *Ptr = SB.get_Pointer_For_ಸಾಲು_ಸಂಖ್ಯೆ(ಸಾಲು_No);
  ಯದಿ (!Ptr)
    ಪ್ರತ್ಯಾಗಮ SMLoc();

  // We start counting line and column numbers from 1.
  ಯದಿ (ColNo != 0)
    --ColNo;

  // If we have a column number, validate it.
  ಯದಿ (ColNo) {
    // Make sure the location is within the current line.
    ಯದಿ (Ptr + ColNo > SB.Buffer->getBufferEnd())
      ಪ್ರತ್ಯಾಗಮ SMLoc();

    // Make sure there is no newline in the way.
    ಯದಿ (StringRef(Ptr, ColNo).find_first_of("\n\r") != StringRef::npos)
      ಪ್ರತ್ಯಾಗಮ SMLoc();

    Ptr += ColNo;
  }

  ಪ್ರತ್ಯಾಗಮ SMLoc::getFromPointer(Ptr);
}

ಶೂನ್ಯ SourceMgr::Print_Include_ರಾಶಿ(SMLoc IncludeLoc, raw_ostream &OS) ಧ್ರುವ {
  ಯದಿ (IncludeLoc == SMLoc())
    ಪ್ರತ್ಯಾಗಮ; // Top of stack.

  unsigned CurBuf = FindBufferContainingLoc(IncludeLoc);
  ಪ್ರತಿಜ್ಞಾ(CurBuf && "Invalid or unspecified location!");

  Print_Include_ರಾಶಿ(getBufferInfo(CurBuf).IncludeLoc, OS);

  OS << "Included from " << getBufferInfo(CurBuf).Buffer->getBufferIdentifier()
     << ":" << Find_ಸಾಲು_ಸಂಖ್ಯೆ(IncludeLoc, CurBuf) << ":\n";
}

ಶೂನ್ಯ SourceMgr::print_Include_ರಾಶಿ_For_Diagnostic(SMLoc Loc,
                                               raw_ostream &OS) ಧ್ರುವ {
  ಯದಿ (!Loc.isValid())
    ಪ್ರತ್ಯಾಗಮ;
  unsigned DiagCurBuffer = FindBufferContainingLoc(Loc);
  ಯದಿ (DiagCurBuffer && DiagCurBuffer != getMainFileID()) {
    SMLoc ParentIncludeLoc = getParentIncludeLoc(DiagCurBuffer);
    // Ignore macro instantiation buffers to avoid redundant include stacks.
    Print_Include_ರಾಶಿ(ParentIncludeLoc, OS);
  }
}

SMDiagnostic SourceMgr::GetMessage(SMLoc Loc, SourceMgr::DiagKind Kind,
                                   ಧ್ರುವ Twine &ಸಂದೇಶ_2, ArrayRef<SMRange> Ranges,
                                   ArrayRef<SMFixIt> FixIts) ಧ್ರುವ {
  // First thing to do: find the current buffer containing the specified
  // location to pull out the source line.
  SmallVector<std::pair<unsigned, unsigned>, 4> ColRanges;
  std::pair<unsigned, unsigned> ಸಾಲು_And_Col;
  StringRef BufferID = "<unknown>";
  StringRef ಸಾಲು_Str;

  ಯದಿ (Loc.isValid()) {
    unsigned CurBuf = FindBufferContainingLoc(Loc);
    ಪ್ರತಿಜ್ಞಾ(CurBuf && "Invalid or unspecified location!");

    ಧ್ರುವ MemoryBuffer *CurMB = getMemoryBuffer(CurBuf);
    BufferID = CurMB->getBufferIdentifier();

    // Scan backward to find the start of the line.
    ಧ್ರುವ ವರ್ಣ *ಸಾಲು_Start = Loc.getPointer();
    ಧ್ರುವ ವರ್ಣ *BufStart = CurMB->getBufferStart();
    ಯಾವತ್ (ಸಾಲು_Start != BufStart && ಸಾಲು_Start[-1] != '\n' &&
           ಸಾಲು_Start[-1] != '\r')
      --ಸಾಲು_Start;

    // Get the end of the line.
    ಧ್ರುವ ವರ್ಣ *ಸಾಲು_End = Loc.getPointer();
    ಧ್ರುವ ವರ್ಣ *BufEnd = CurMB->getBufferEnd();
    ಯಾವತ್ (ಸಾಲು_End != BufEnd && ಸಾಲು_End[0] != '\n' && ಸಾಲು_End[0] != '\r')
      ++ಸಾಲು_End;
    ಸಾಲು_Str = StringRef(ಸಾಲು_Start, ಸಾಲು_End - ಸಾಲು_Start);

    // Convert any ranges to column ranges that only intersect the line of the
    // location.
    ಪ್ರತಿ (SMRange R : Ranges) {
      ಯದಿ (!R.isValid())
        ಅನುವರ್ತ;

      // If the line doesn't contain any part of the range, then ignore it.
      ಯದಿ (R.Start.getPointer() > ಸಾಲು_End || R.End.getPointer() < ಸಾಲು_Start)
        ಅನುವರ್ತ;

      // Ignore pieces of the range that go onto other lines.
      ಯದಿ (R.Start.getPointer() < ಸಾಲು_Start)
        R.Start = SMLoc::getFromPointer(ಸಾಲು_Start);
      ಯದಿ (R.End.getPointer() > ಸಾಲು_End)
        R.End = SMLoc::getFromPointer(ಸಾಲು_End);

      // Translate from SMLoc ranges to column ranges.
      // FIXME: Handle multibyte characters.
      ColRanges.ತಳ್ಳು_back(std::make_pair(R.Start.getPointer() - ಸಾಲು_Start,
                                         R.End.getPointer() - ಸಾಲು_Start));
    }

    ಸಾಲು_And_Col = get_ಸಾಲು_And_Column(Loc, CurBuf);
  }

  ಪ್ರತ್ಯಾಗಮ SMDiagnostic(*ಏತತ್, Loc, BufferID, ಸಾಲು_And_Col.first,
                      ಸಾಲು_And_Col.second - 1, Kind, ಸಂದೇಶ_2.str(), ಸಾಲು_Str,
                      ColRanges, FixIts);
}

ಶೂನ್ಯ SourceMgr::PrintMessage(raw_ostream &OS, ಧ್ರುವ SMDiagnostic &Diagnostic,
                             ತಾರ್ಕಿಕ ShowColors) ಧ್ರುವ {
  // Report the message with the diagnostic handler if present.
  ಯದಿ (DiagHandler) {
    DiagHandler(Diagnostic, DiagContext);
    ಪ್ರತ್ಯಾಗಮ;
  }

  ಯದಿ (Diagnostic.getLoc().isValid()) {
    unsigned CurBuf = FindBufferContainingLoc(Diagnostic.getLoc());
    ಪ್ರತಿಜ್ಞಾ(CurBuf && "Invalid or unspecified location!");
    Print_Include_ರಾಶಿ(getBufferInfo(CurBuf).IncludeLoc, OS);
  }

  Diagnostic.print(nullptr, OS, ShowColors);
}

ಶೂನ್ಯ SourceMgr::PrintMessage(raw_ostream &OS, SMLoc Loc,
                             SourceMgr::DiagKind Kind, ಧ್ರುವ Twine &ಸಂದೇಶ_2,
                             ArrayRef<SMRange> Ranges, ArrayRef<SMFixIt> FixIts,
                             ತಾರ್ಕಿಕ ShowColors) ಧ್ರುವ {
  PrintMessage(OS, GetMessage(Loc, Kind, ಸಂದೇಶ_2, Ranges, FixIts), ShowColors);
}

ಶೂನ್ಯ SourceMgr::PrintMessage(SMLoc Loc, SourceMgr::DiagKind Kind,
                             ಧ್ರುವ Twine &ಸಂದೇಶ_2, ArrayRef<SMRange> Ranges,
                             ArrayRef<SMFixIt> FixIts, ತಾರ್ಕಿಕ ShowColors) ಧ್ರುವ {
  PrintMessage(errs(), Loc, Kind, ಸಂದೇಶ_2, Ranges, FixIts, ShowColors);
}

//===----------------------------------------------------------------------===//
// SMFixIt Implementation
//===----------------------------------------------------------------------===//

SMFixIt::SMFixIt(SMRange R, ಧ್ರುವ Twine &Replacement)
    : Range(R), Text(Replacement.str()) {
  ಪ್ರತಿಜ್ಞಾ(R.isValid());
}

//===----------------------------------------------------------------------===//
// SMDiagnostic Implementation
//===----------------------------------------------------------------------===//

SMDiagnostic::SMDiagnostic(ಧ್ರುವ SourceMgr &sm, SMLoc L, StringRef FN, ಪೂರ್ಣಾಙ್ಕ ಸಾಲು,
                           ಪೂರ್ಣಾಙ್ಕ Col, SourceMgr::DiagKind Kind, StringRef ಸಂದೇಶ_2,
                           StringRef ಸಾಲು_Str,
                           ArrayRef<std::pair<unsigned, unsigned>> Ranges,
                           ArrayRef<SMFixIt> Hints)
    : SM(&sm), Loc(L), Filename(std::string(FN)), ಸಾಲು_No(ಸಾಲು), ColumnNo(Col),
      Kind(Kind), Message(ಸಂದೇಶ_2), ಸಾಲು_Contents(ಸಾಲು_Str), Ranges(Ranges.vec()),
      FixIts(Hints) {
  llvm::sort(FixIts);
}

ಸ್ಥಿರ ಶೂನ್ಯ build_Fix_It_ಸಾಲು(std::string &Caret_ಸಾಲು, std::string &Fix_It_ಸಾಲು,
                           ArrayRef<SMFixIt> FixIts,
                           ArrayRef<ವರ್ಣ> Source_ಸಾಲು) {
  ಯದಿ (FixIts.empty())
    ಪ್ರತ್ಯಾಗಮ;

  ಧ್ರುವ ವರ್ಣ *ಸಾಲು_Start = Source_ಸಾಲು.begin();
  ಧ್ರುವ ವರ್ಣ *ಸಾಲು_End = Source_ಸಾಲು.end();

  size_t PrevHintEndCol = 0;

  ಪ್ರತಿ (ಧ್ರುವ llvm::SMFixIt &Fixit : FixIts) {
    // If the fixit contains a newline or tab, ignore it.
    ಯದಿ (Fixit.getText().find_first_of("\n\r\t") != StringRef::npos)
      ಅನುವರ್ತ;

    SMRange R = Fixit.getRange();

    // If the line doesn't contain any part of the range, then ignore it.
    ಯದಿ (R.Start.getPointer() > ಸಾಲು_End || R.End.getPointer() < ಸಾಲು_Start)
      ಅನುವರ್ತ;

    // Translate from SMLoc to column.
    // Ignore pieces of the range that go onto other lines.
    // FIXME: Handle multibyte characters in the source line.
    unsigned FirstCol;
    ಯದಿ (R.Start.getPointer() < ಸಾಲು_Start)
      FirstCol = 0;
    ಅನ್ಯಥಾ
      FirstCol = R.Start.getPointer() - ಸಾಲು_Start;

    // If we inserted a long previous hint, push this one forwards, and add
    // an extra space to show that this is not part of the previous
    // completion. This is sort of the best we can do when two hints appear
    // to overlap.
    //
    // Note that if this hint is located immediately after the previous
    // hint, no space will be added, since the location is more important.
    unsigned HintCol = FirstCol;
    ಯದಿ (HintCol < PrevHintEndCol)
      HintCol = PrevHintEndCol + 1;

    // FIXME: This assertion is intended to catch unintended use of multibyte
    // characters in fixits. If we decide to do this, we'll have to track
    // separate byte widths for the source and fixit lines.
    ಪ್ರತಿಜ್ಞಾ((size_t)sys::locale::columnWidth(Fixit.getText()) ==
           Fixit.getText().size());

    // This relies on one byte per column in our fixit hints.
    unsigned LastColumnModified = HintCol + Fixit.getText().size();
    ಯದಿ (LastColumnModified > Fix_It_ಸಾಲು.size())
      Fix_It_ಸಾಲು.resize(LastColumnModified, ' ');

    llvm::copy(Fixit.getText(), Fix_It_ಸಾಲು.begin() + HintCol);

    PrevHintEndCol = LastColumnModified;

    // For replacements, mark the removal range with '~'.
    // FIXME: Handle multibyte characters in the source line.
    unsigned LastCol;
    ಯದಿ (R.End.getPointer() >= ಸಾಲು_End)
      LastCol = ಸಾಲು_End - ಸಾಲು_Start;
    ಅನ್ಯಥಾ
      LastCol = R.End.getPointer() - ಸಾಲು_Start;

    std::fill(&Caret_ಸಾಲು[FirstCol], &Caret_ಸಾಲು[LastCol], '~');
  }
}

ಸ್ಥಿರ ಶೂನ್ಯ print_Source_ಸಾಲು(raw_ostream &S, StringRef ಸಾಲು_Contents) {
  // Print out the source line one character at a time, so we can expand tabs.
  ಪ್ರತಿ (unsigned i = 0, e = ಸಾಲು_Contents.size(), OutCol = 0; i != e; ++i) {
    size_t NextTab = ಸಾಲು_Contents.find('\t', i);
    // If there were no tabs left, print the rest, we are done.
    ಯದಿ (NextTab == StringRef::npos) {
      S << ಸಾಲು_Contents.drop_front(i);
      ಭಙ್ಗ;
    }

    // Otherwise, print from i to NextTab.
    S << ಸಾಲು_Contents.slice(i, NextTab);
    OutCol += NextTab - i;
    i = NextTab;

    // If we have a tab, emit at least one space, then round up to 8 columns.
    ಕುರು {
      S << ' ';
      ++OutCol;
    } ಯಾವತ್ ((OutCol % TabStop) != 0);
  }
  S << '\n';
}

ಸ್ಥಿರ ತಾರ್ಕಿಕ isNonASCII(ವರ್ಣ c) { ಪ್ರತ್ಯಾಗಮ c & 0x80; }

ಶೂನ್ಯ SMDiagnostic::print(ಧ್ರುವ ವರ್ಣ *ProgName, raw_ostream &OS, ತಾರ್ಕಿಕ ShowColors,
                         ತಾರ್ಕಿಕ ShowKindLabel, ತಾರ್ಕಿಕ ShowLocation) ಧ್ರುವ {
  ColorMode Mode = ShowColors ? ColorMode::Auto : ColorMode::Disable;

  {
    WithColor S(OS, raw_ostream::SAVEDCOLOR, ಸತ್ಯ, ಅಸತ್ಯ, Mode);

    ಯದಿ (ProgName && ProgName[0])
      S << ProgName << ": ";

    ಯದಿ (ShowLocation && !Filename.empty()) {
      ಯದಿ (Filename == "-")
        S << "<stdin>";
      ಅನ್ಯಥಾ
        S << Filename;

      ಯದಿ (ಸಾಲು_No != -1) {
        S << ':' << ಸಾಲು_No;
        ಯದಿ (ColumnNo != -1)
          S << ':' << (ColumnNo + 1);
      }
      S << ": ";
    }
  }

  ಯದಿ (ShowKindLabel) {
    ವಿಕಲ್ಪ (Kind) {
    ಪಕ್ಷ SourceMgr::DK_Error:
      WithColor::error(OS, "", !ShowColors);
      ಭಙ್ಗ;
    ಪಕ್ಷ SourceMgr::DK_Warning:
      WithColor::warning(OS, "", !ShowColors);
      ಭಙ್ಗ;
    ಪಕ್ಷ SourceMgr::DK_Note:
      WithColor::note(OS, "", !ShowColors);
      ಭಙ್ಗ;
    ಪಕ್ಷ SourceMgr::DK_Remark:
      WithColor::remark(OS, "", !ShowColors);
      ಭಙ್ಗ;
    }
  }

  WithColor(OS, raw_ostream::SAVEDCOLOR, ಸತ್ಯ, ಅಸತ್ಯ, Mode) << Message << '\n';

  ಯದಿ (ಸಾಲು_No == -1 || ColumnNo == -1)
    ಪ್ರತ್ಯಾಗಮ;

  // FIXME: If there are multibyte or multi-column characters in the source, all
  // our ranges will be wrong. To do this properly, we'll need a byte-to-column
  // map like Clang's TextDiagnostic. For now, we'll just handle tabs by
  // expanding them later, and bail out rather than show incorrect ranges and
  // misaligned fixits for any other odd characters.
  ಯದಿ (any_of(ಸಾಲು_Contents, isNonASCII)) {
    print_Source_ಸಾಲು(OS, ಸಾಲು_Contents);
    ಪ್ರತ್ಯಾಗಮ;
  }
  size_t ಅಂಕೆ_Columns = ಸಾಲು_Contents.size();

  // Build the line with the caret and ranges.
  std::string Caret_ಸಾಲು(ಅಂಕೆ_Columns + 1, ' ');

  // Expand any ranges.
  ಪ್ರತಿ (ಧ್ರುವ std::pair<unsigned, unsigned> &R : Ranges)
    std::fill(&Caret_ಸಾಲು[R.first],
              &Caret_ಸಾಲು[std::min((size_t)R.second, Caret_ಸಾಲು.size())], '~');

  // Add any fix-its.
  // FIXME: Find the beginning of the line properly for multibyte characters.
  std::string Fix_It_Insertion_ಸಾಲು;
  build_Fix_It_ಸಾಲು(Caret_ಸಾಲು, Fix_It_Insertion_ಸಾಲು, FixIts,
                 ArrayRef(Loc.getPointer() - ColumnNo, ಸಾಲು_Contents.size()));

  // Finally, plop on the caret.
  ಯದಿ (unsigned(ColumnNo) <= ಅಂಕೆ_Columns)
    Caret_ಸಾಲು[ColumnNo] = '^';
  ಅನ್ಯಥಾ
    Caret_ಸಾಲು[ಅಂಕೆ_Columns] = '^';

  // ... and remove trailing whitespace so the output doesn't wrap for it.  We
  // know that the line isn't completely empty because it has the caret in it at
  // least.
  Caret_ಸಾಲು.erase(Caret_ಸಾಲು.find_last_not_of(' ') + 1);

  print_Source_ಸಾಲು(OS, ಸಾಲು_Contents);

  {
    ColorMode Mode = ShowColors ? ColorMode::Auto : ColorMode::Disable;
    WithColor S(OS, raw_ostream::GREEN, ಸತ್ಯ, ಅಸತ್ಯ, Mode);

    // Print out the caret line, matching tabs in the source line.
    ಪ್ರತಿ (unsigned i = 0, e = Caret_ಸಾಲು.size(), OutCol = 0; i != e; ++i) {
      ಯದಿ (i >= ಸಾಲು_Contents.size() || ಸಾಲು_Contents[i] != '\t') {
        S << Caret_ಸಾಲು[i];
        ++OutCol;
        ಅನುವರ್ತ;
      }

      // Okay, we have a tab.  Insert the appropriate number of characters.
      ಕುರು {
        S << Caret_ಸಾಲು[i];
        ++OutCol;
      } ಯಾವತ್ ((OutCol % TabStop) != 0);
    }
    S << '\n';
  }

  // Print out the replacement line, matching tabs in the source line.
  ಯದಿ (Fix_It_Insertion_ಸಾಲು.empty())
    ಪ್ರತ್ಯಾಗಮ;

  ಪ್ರತಿ (size_t i = 0, e = Fix_It_Insertion_ಸಾಲು.size(), OutCol = 0; i < e; ++i) {
    ಯದಿ (i >= ಸಾಲು_Contents.size() || ಸಾಲು_Contents[i] != '\t') {
      OS << Fix_It_Insertion_ಸಾಲು[i];
      ++OutCol;
      ಅನುವರ್ತ;
    }

    // Okay, we have a tab.  Insert the appropriate number of characters.
    ಕುರು {
      OS << Fix_It_Insertion_ಸಾಲು[i];
      // FIXME: This is trying not to break up replacements, but then to re-sync
      // with the tabs between replacements. This will fail, though, if two
      // fix-it replacements are exactly adjacent, or if a fix-it contains a
      // space. Really we should be precomputing column widths, which we'll
      // need anyway for multibyte chars.
      ಯದಿ (Fix_It_Insertion_ಸಾಲು[i] != ' ')
        ++i;
      ++OutCol;
    } ಯಾವತ್ (((OutCol % TabStop) != 0) && i != e);
  }
  OS << '\n';
}
