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

स्थिर ध्रुव माप_प्रकार TabStop = 8;

// Out of line to avoid needing definition of vfs::FileSystem in header.
SourceMgr::SourceMgr() = स्वभाव;
SourceMgr::SourceMgr(IntrusiveRefCntPtr<vfs::FileSystem> FS)
    : FS(std::move(FS)) {}
SourceMgr::SourceMgr(SourceMgr &&) = स्वभाव;
SourceMgr &SourceMgr::चालक=(SourceMgr &&) = स्वभाव;
SourceMgr::~SourceMgr() = स्वभाव;

IntrusiveRefCntPtr<vfs::FileSystem> SourceMgr::getVirtualFileSystem() ध्रुव {
  प्रत्यागम FS;
}

शून्य SourceMgr::setVirtualFileSystem(IntrusiveRefCntPtr<vfs::FileSystem> FS) {
  एतत्->FS = std::move(FS);
}

अचिन्हित SourceMgr::AddIncludeFile(ध्रुव std::string &Filename,
                                   SMLoc IncludeLoc,
                                   std::string &IncludedFile) {
  ErrorOr<std::unique_ptr<MemoryBuffer>> NewBufOrErr =
      OpenIncludeFile(Filename, IncludedFile);
  यदि (!NewBufOrErr)
    प्रत्यागम 0;

  प्रत्यागम AddNewSourceBuffer(std::move(*NewBufOrErr), IncludeLoc);
}

ErrorOr<std::unique_ptr<MemoryBuffer>>
SourceMgr::OpenIncludeFile(ध्रुव std::string &Filename,
                           std::string &IncludedFile,
                           तार्किक RequiresNullTerminator) {
  स्वतः GetFile = [एतत्, RequiresNullTerminator](StringRef Path) {
    प्रत्यागम FS ? FS->getBufferForFile(Path, /*FileSize=*/-1,
                                     RequiresNullTerminator)
              : MemoryBuffer::getFile(Path, /*IsText=*/असत्य,
                                      RequiresNullTerminator);
  };

  ErrorOr<std::unique_ptr<MemoryBuffer>> NewBufOrErr = GetFile(Filename);

  SmallString<64> Buffer(Filename);
  // If the file didn't exist directly, see if it's in an include path.
  प्रति (अचिन्हित i = 0, e = IncludeDirectories.size(); i != e && !NewBufOrErr;
       ++i) {
    Buffer = IncludeDirectories[i];
    sys::path::append(Buffer, Filename);
    NewBufOrErr = GetFile(Buffer);
  }

  यदि (NewBufOrErr)
    IncludedFile = static_cast<std::string>(Buffer);

  प्रत्यागम NewBufOrErr;
}

अचिन्हित SourceMgr::FindBufferContainingLoc(SMLoc Loc) ध्रुव {
  प्रति (अचिन्हित i = 0, e = Buffers.size(); i != e; ++i)
    यदि (Loc.getPointer() >= Buffers[i].Buffer->getBufferStart() &&
        // Use <= here so that a pointer to the null at the end of the buffer
        // is included as part of the buffer.
        Loc.getPointer() <= Buffers[i].Buffer->getBufferEnd())
      प्रत्यागम i + 1;
  प्रत्यागम 0;
}

ढाँचा <typename T>
स्थिर std::vector<T> &GetOrCreateOffsetCache(शून्य *&OffsetCache,
                                              MemoryBuffer *Buffer) {
  यदि (OffsetCache)
    प्रत्यागम *static_cast<std::vector<T> *>(OffsetCache);

  // Lazily fill in the offset cache.
  स्वतः *Offsets = नूतन std::vector<T>();
  माप_प्रकार Sz = Buffer->getBufferSize();
  प्रतिज्ञा(Sz <= std::numeric_limits<T>::max());
  StringRef S = Buffer->getBuffer();
  प्रति (माप_प्रकार N = 0; N < Sz; ++N) {
    यदि (S[N] == '\n')
      Offsets->धकेलो_back(static_cast<T>(N));
  }

  OffsetCache = Offsets;
  प्रत्यागम *Offsets;
}

ढाँचा <typename T>
std::pair<अचिन्हित, अचिन्हित>
SourceMgr::SrcBuffer::get_पंक्ति_And_Column_Specialized(ध्रुव वर्ण *Ptr) ध्रुव {
  std::vector<T> &Offsets =
      GetOrCreateOffsetCache<T>(OffsetCache, Buffer.get());

  ध्रुव वर्ण *BufStart = Buffer->getBufferStart();
  प्रतिज्ञा(Ptr >= BufStart && Ptr <= Buffer->getBufferEnd());
  सूचक_भेद_प्रकार PtrDiff = Ptr - BufStart;
  प्रतिज्ञा(PtrDiff >= 0 &&
         static_cast<माप_प्रकार>(PtrDiff) <= std::numeric_limits<T>::max());
  T PtrOffset = static_cast<T>(PtrDiff);

  // llvm::lower_bound gives the number of EOL before PtrOffset. Add 1 to get
  // the line number.
  स्वतः I = llvm::lower_bound(Offsets, PtrOffset);
  अचिन्हित पंक्ति_No = I - Offsets.begin() + 1;

  // The column number is the distance from the previous EOL (or the start of
  // the buffer) to the pointer.
  T पंक्ति_Start_Offs = (I == Offsets.begin()) ? 0 : (I[-1] + 1);
  अचिन्हित ColNo = PtrOffset - पंक्ति_Start_Offs + 1;
  प्रत्यागम {पंक्ति_No, ColNo};
}

/// Look up a given \p Ptr in the buffer, determining which line and column
/// it came from.
LLVM_ABI std::pair<अचिन्हित, अचिन्हित>
SourceMgr::SrcBuffer::get_पंक्ति_And_Column(ध्रुव वर्ण *Ptr) ध्रुव {
  माप_प्रकार Sz = Buffer->getBufferSize();
  यदि (Sz <= std::numeric_limits<uint8_t>::max())
    प्रत्यागम get_पंक्ति_And_Column_Specialized<uint8_t>(Ptr);
  यदि (Sz <= std::numeric_limits<uint16_t>::max())
    प्रत्यागम get_पंक्ति_And_Column_Specialized<uint16_t>(Ptr);
  यदि (Sz <= std::numeric_limits<uint32_t>::max())
    प्रत्यागम get_पंक्ति_And_Column_Specialized<uint32_t>(Ptr);
  प्रत्यागम get_पंक्ति_And_Column_Specialized<uint64_t>(Ptr);
}

ढाँचा <typename T>
ध्रुव वर्ण *SourceMgr::SrcBuffer::get_Pointer_For_पंक्ति_संख्या_Specialized(
    अचिन्हित पंक्ति_No) ध्रुव {
  std::vector<T> &Offsets =
      GetOrCreateOffsetCache<T>(OffsetCache, Buffer.get());

  // We start counting line and column numbers from 1.
  यदि (पंक्ति_No != 0)
    --पंक्ति_No;

  ध्रुव वर्ण *BufStart = Buffer->getBufferStart();

  // The offset cache contains the location of the \n for the specified line,
  // we want the start of the line.  As such, we look for the previous entry.
  यदि (पंक्ति_No == 0)
    प्रत्यागम BufStart;
  यदि (पंक्ति_No > Offsets.size())
    प्रत्यागम nullptr;
  प्रत्यागम BufStart + Offsets[पंक्ति_No - 1] + 1;
}

/// Return a pointer to the first character of the specified line number or
/// null if the line number is invalid.
ध्रुव वर्ण *
SourceMgr::SrcBuffer::get_Pointer_For_पंक्ति_संख्या(अचिन्हित पंक्ति_No) ध्रुव {
  माप_प्रकार Sz = Buffer->getBufferSize();
  यदि (Sz <= std::numeric_limits<uint8_t>::max())
    प्रत्यागम get_Pointer_For_पंक्ति_संख्या_Specialized<uint8_t>(पंक्ति_No);
  अन्यथा यदि (Sz <= std::numeric_limits<uint16_t>::max())
    प्रत्यागम get_Pointer_For_पंक्ति_संख्या_Specialized<uint16_t>(पंक्ति_No);
  अन्यथा यदि (Sz <= std::numeric_limits<uint32_t>::max())
    प्रत्यागम get_Pointer_For_पंक्ति_संख्या_Specialized<uint32_t>(पंक्ति_No);
  अन्यथा
    प्रत्यागम get_Pointer_For_पंक्ति_संख्या_Specialized<uint64_t>(पंक्ति_No);
}

SourceMgr::SrcBuffer::SrcBuffer(SourceMgr::SrcBuffer &&Other)
    : Buffer(std::move(Other.Buffer)), OffsetCache(Other.OffsetCache),
      IncludeLoc(Other.IncludeLoc) {
  Other.OffsetCache = nullptr;
}

SourceMgr::SrcBuffer::~SrcBuffer() {
  यदि (OffsetCache) {
    माप_प्रकार Sz = Buffer->getBufferSize();
    यदि (Sz <= std::numeric_limits<uint8_t>::max())
      delete static_cast<std::vector<uint8_t> *>(OffsetCache);
    अन्यथा यदि (Sz <= std::numeric_limits<uint16_t>::max())
      delete static_cast<std::vector<uint16_t> *>(OffsetCache);
    अन्यथा यदि (Sz <= std::numeric_limits<uint32_t>::max())
      delete static_cast<std::vector<uint32_t> *>(OffsetCache);
    अन्यथा
      delete static_cast<std::vector<uint64_t> *>(OffsetCache);
    OffsetCache = nullptr;
  }
}

std::pair<अचिन्हित, अचिन्हित>
SourceMgr::get_पंक्ति_And_Column(SMLoc Loc, अचिन्हित BufferID) ध्रुव {
  यदि (!BufferID)
    BufferID = FindBufferContainingLoc(Loc);
  प्रतिज्ञा(BufferID && "Invalid location!");

  स्वतः &SB = getBufferInfo(BufferID);
  ध्रुव वर्ण *Ptr = Loc.getPointer();

  प्रत्यागम SB.get_पंक्ति_And_Column(Ptr);
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
  स्वतः BufferID = FindBufferContainingLoc(Loc);
  प्रतिज्ञा(BufferID && "Invalid location!");
  स्वतः FileSpec = getBufferInfo(BufferID).Buffer->getBufferIdentifier();

  यदि (IncludePath) {
    प्रत्यागम FileSpec.str() + ":" + std::to_string(Find_पंक्ति_संख्या(Loc, BufferID));
  } अन्यथा {
    स्वतः I = FileSpec.find_last_of("/\\");
    I = (I == FileSpec.size()) ? 0 : (I + 1);
    प्रत्यागम FileSpec.substr(I).str() + ":" +
           std::to_string(Find_पंक्ति_संख्या(Loc, BufferID));
  }
}

/// Given a line and column number in a mapped buffer, turn it into an SMLoc.
/// This will return a null SMLoc if the line/column location is invalid.
SMLoc SourceMgr::Find_Loc_For_पंक्ति_And_Column(अचिन्हित BufferID, अचिन्हित पंक्ति_No,
                                         अचिन्हित ColNo) {
  स्वतः &SB = getBufferInfo(BufferID);
  ध्रुव वर्ण *Ptr = SB.get_Pointer_For_पंक्ति_संख्या(पंक्ति_No);
  यदि (!Ptr)
    प्रत्यागम SMLoc();

  // We start counting line and column numbers from 1.
  यदि (ColNo != 0)
    --ColNo;

  // If we have a column number, validate it.
  यदि (ColNo) {
    // Make sure the location is within the current line.
    यदि (Ptr + ColNo > SB.Buffer->getBufferEnd())
      प्रत्यागम SMLoc();

    // Make sure there is no newline in the way.
    यदि (StringRef(Ptr, ColNo).find_first_of("\n\r") != StringRef::npos)
      प्रत्यागम SMLoc();

    Ptr += ColNo;
  }

  प्रत्यागम SMLoc::getFromPointer(Ptr);
}

शून्य SourceMgr::Print_Include_ढेर(SMLoc IncludeLoc, raw_ostream &OS) ध्रुव {
  यदि (IncludeLoc == SMLoc())
    प्रत्यागम; // Top of stack.

  अचिन्हित CurBuf = FindBufferContainingLoc(IncludeLoc);
  प्रतिज्ञा(CurBuf && "Invalid or unspecified location!");

  Print_Include_ढेर(getBufferInfo(CurBuf).IncludeLoc, OS);

  OS << "Included from " << getBufferInfo(CurBuf).Buffer->getBufferIdentifier()
     << ":" << Find_पंक्ति_संख्या(IncludeLoc, CurBuf) << ":\n";
}

शून्य SourceMgr::print_Include_ढेर_For_Diagnostic(SMLoc Loc,
                                               raw_ostream &OS) ध्रुव {
  यदि (!Loc.isValid())
    प्रत्यागम;
  अचिन्हित DiagCurBuffer = FindBufferContainingLoc(Loc);
  यदि (DiagCurBuffer && DiagCurBuffer != getMainFileID()) {
    SMLoc ParentIncludeLoc = getParentIncludeLoc(DiagCurBuffer);
    // Ignore macro instantiation buffers to avoid redundant include stacks.
    Print_Include_ढेर(ParentIncludeLoc, OS);
  }
}

SMDiagnostic SourceMgr::GetMessage(SMLoc Loc, SourceMgr::DiagKind Kind,
                                   ध्रुव Twine &संदेश_2, ArrayRef<SMRange> Ranges,
                                   ArrayRef<SMFixIt> FixIts) ध्रुव {
  // First thing to do: find the current buffer containing the specified
  // location to pull out the source line.
  SmallVector<std::pair<अचिन्हित, अचिन्हित>, 4> ColRanges;
  std::pair<अचिन्हित, अचिन्हित> पंक्ति_And_Col;
  StringRef BufferID = "<unknown>";
  StringRef पंक्ति_Str;

  यदि (Loc.isValid()) {
    अचिन्हित CurBuf = FindBufferContainingLoc(Loc);
    प्रतिज्ञा(CurBuf && "Invalid or unspecified location!");

    ध्रुव MemoryBuffer *CurMB = getMemoryBuffer(CurBuf);
    BufferID = CurMB->getBufferIdentifier();

    // Scan backward to find the start of the line.
    ध्रुव वर्ण *पंक्ति_Start = Loc.getPointer();
    ध्रुव वर्ण *BufStart = CurMB->getBufferStart();
    यावत् (पंक्ति_Start != BufStart && पंक्ति_Start[-1] != '\n' &&
           पंक्ति_Start[-1] != '\r')
      --पंक्ति_Start;

    // Get the end of the line.
    ध्रुव वर्ण *पंक्ति_End = Loc.getPointer();
    ध्रुव वर्ण *BufEnd = CurMB->getBufferEnd();
    यावत् (पंक्ति_End != BufEnd && पंक्ति_End[0] != '\n' && पंक्ति_End[0] != '\r')
      ++पंक्ति_End;
    पंक्ति_Str = StringRef(पंक्ति_Start, पंक्ति_End - पंक्ति_Start);

    // Convert any ranges to column ranges that only intersect the line of the
    // location.
    प्रति (SMRange R : Ranges) {
      यदि (!R.isValid())
        अनुवर्त;

      // If the line doesn't contain any part of the range, then ignore it.
      यदि (R.Start.getPointer() > पंक्ति_End || R.End.getPointer() < पंक्ति_Start)
        अनुवर्त;

      // Ignore pieces of the range that go onto other lines.
      यदि (R.Start.getPointer() < पंक्ति_Start)
        R.Start = SMLoc::getFromPointer(पंक्ति_Start);
      यदि (R.End.getPointer() > पंक्ति_End)
        R.End = SMLoc::getFromPointer(पंक्ति_End);

      // Translate from SMLoc ranges to column ranges.
      // FIXME: Handle multibyte characters.
      ColRanges.धकेलो_back(std::make_pair(R.Start.getPointer() - पंक्ति_Start,
                                         R.End.getPointer() - पंक्ति_Start));
    }

    पंक्ति_And_Col = get_पंक्ति_And_Column(Loc, CurBuf);
  }

  प्रत्यागम SMDiagnostic(*एतत्, Loc, BufferID, पंक्ति_And_Col.first,
                      पंक्ति_And_Col.second - 1, Kind, संदेश_2.str(), पंक्ति_Str,
                      ColRanges, FixIts);
}

शून्य SourceMgr::PrintMessage(raw_ostream &OS, ध्रुव SMDiagnostic &Diagnostic,
                             तार्किक ShowColors) ध्रुव {
  // Report the message with the diagnostic handler if present.
  यदि (DiagHandler) {
    DiagHandler(Diagnostic, DiagContext);
    प्रत्यागम;
  }

  यदि (Diagnostic.getLoc().isValid()) {
    अचिन्हित CurBuf = FindBufferContainingLoc(Diagnostic.getLoc());
    प्रतिज्ञा(CurBuf && "Invalid or unspecified location!");
    Print_Include_ढेर(getBufferInfo(CurBuf).IncludeLoc, OS);
  }

  Diagnostic.print(nullptr, OS, ShowColors);
}

शून्य SourceMgr::PrintMessage(raw_ostream &OS, SMLoc Loc,
                             SourceMgr::DiagKind Kind, ध्रुव Twine &संदेश_2,
                             ArrayRef<SMRange> Ranges, ArrayRef<SMFixIt> FixIts,
                             तार्किक ShowColors) ध्रुव {
  PrintMessage(OS, GetMessage(Loc, Kind, संदेश_2, Ranges, FixIts), ShowColors);
}

शून्य SourceMgr::PrintMessage(SMLoc Loc, SourceMgr::DiagKind Kind,
                             ध्रुव Twine &संदेश_2, ArrayRef<SMRange> Ranges,
                             ArrayRef<SMFixIt> FixIts, तार्किक ShowColors) ध्रुव {
  PrintMessage(errs(), Loc, Kind, संदेश_2, Ranges, FixIts, ShowColors);
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

SMDiagnostic::SMDiagnostic(ध्रुव SourceMgr &sm, SMLoc L, StringRef FN, पूर्णाङ्क पंक्ति,
                           पूर्णाङ्क Col, SourceMgr::DiagKind Kind, StringRef संदेश_2,
                           StringRef पंक्ति_Str,
                           ArrayRef<std::pair<अचिन्हित, अचिन्हित>> Ranges,
                           ArrayRef<SMFixIt> Hints)
    : SM(&sm), Loc(L), Filename(std::string(FN)), पंक्ति_No(पंक्ति), ColumnNo(Col),
      Kind(Kind), Message(संदेश_2), पंक्ति_Contents(पंक्ति_Str), Ranges(Ranges.vec()),
      FixIts(Hints) {
  llvm::sort(FixIts);
}

स्थिर शून्य build_Fix_It_पंक्ति(std::string &Caret_पंक्ति, std::string &Fix_It_पंक्ति,
                           ArrayRef<SMFixIt> FixIts,
                           ArrayRef<वर्ण> Source_पंक्ति) {
  यदि (FixIts.empty())
    प्रत्यागम;

  ध्रुव वर्ण *पंक्ति_Start = Source_पंक्ति.begin();
  ध्रुव वर्ण *पंक्ति_End = Source_पंक्ति.end();

  माप_प्रकार PrevHintEndCol = 0;

  प्रति (ध्रुव llvm::SMFixIt &Fixit : FixIts) {
    // If the fixit contains a newline or tab, ignore it.
    यदि (Fixit.getText().find_first_of("\n\r\t") != StringRef::npos)
      अनुवर्त;

    SMRange R = Fixit.getRange();

    // If the line doesn't contain any part of the range, then ignore it.
    यदि (R.Start.getPointer() > पंक्ति_End || R.End.getPointer() < पंक्ति_Start)
      अनुवर्त;

    // Translate from SMLoc to column.
    // Ignore pieces of the range that go onto other lines.
    // FIXME: Handle multibyte characters in the source line.
    अचिन्हित FirstCol;
    यदि (R.Start.getPointer() < पंक्ति_Start)
      FirstCol = 0;
    अन्यथा
      FirstCol = R.Start.getPointer() - पंक्ति_Start;

    // If we inserted a long previous hint, push this one forwards, and add
    // an extra space to show that this is not part of the previous
    // completion. This is sort of the best we can do when two hints appear
    // to overlap.
    //
    // Note that if this hint is located immediately after the previous
    // hint, no space will be added, since the location is more important.
    अचिन्हित HintCol = FirstCol;
    यदि (HintCol < PrevHintEndCol)
      HintCol = PrevHintEndCol + 1;

    // FIXME: This assertion is intended to catch unintended use of multibyte
    // characters in fixits. If we decide to do this, we'll have to track
    // separate byte widths for the source and fixit lines.
    प्रतिज्ञा((माप_प्रकार)sys::locale::columnWidth(Fixit.getText()) ==
           Fixit.getText().size());

    // This relies on one byte per column in our fixit hints.
    अचिन्हित LastColumnModified = HintCol + Fixit.getText().size();
    यदि (LastColumnModified > Fix_It_पंक्ति.size())
      Fix_It_पंक्ति.resize(LastColumnModified, ' ');

    llvm::copy(Fixit.getText(), Fix_It_पंक्ति.begin() + HintCol);

    PrevHintEndCol = LastColumnModified;

    // For replacements, mark the removal range with '~'.
    // FIXME: Handle multibyte characters in the source line.
    अचिन्हित LastCol;
    यदि (R.End.getPointer() >= पंक्ति_End)
      LastCol = पंक्ति_End - पंक्ति_Start;
    अन्यथा
      LastCol = R.End.getPointer() - पंक्ति_Start;

    std::fill(&Caret_पंक्ति[FirstCol], &Caret_पंक्ति[LastCol], '~');
  }
}

स्थिर शून्य print_Source_पंक्ति(raw_ostream &S, StringRef पंक्ति_Contents) {
  // Print out the source line one character at a time, so we can expand tabs.
  प्रति (अचिन्हित i = 0, e = पंक्ति_Contents.size(), OutCol = 0; i != e; ++i) {
    माप_प्रकार NextTab = पंक्ति_Contents.find('\t', i);
    // If there were no tabs left, print the rest, we are done.
    यदि (NextTab == StringRef::npos) {
      S << पंक्ति_Contents.drop_front(i);
      भङ्ग;
    }

    // Otherwise, print from i to NextTab.
    S << पंक्ति_Contents.slice(i, NextTab);
    OutCol += NextTab - i;
    i = NextTab;

    // If we have a tab, emit at least one space, then round up to 8 columns.
    कुरु {
      S << ' ';
      ++OutCol;
    } यावत् ((OutCol % TabStop) != 0);
  }
  S << '\n';
}

स्थिर तार्किक isNonASCII(वर्ण c) { प्रत्यागम c & 0x80; }

शून्य SMDiagnostic::print(ध्रुव वर्ण *ProgName, raw_ostream &OS, तार्किक ShowColors,
                         तार्किक ShowKindLabel, तार्किक ShowLocation) ध्रुव {
  ColorMode Mode = ShowColors ? ColorMode::Auto : ColorMode::Disable;

  {
    WithColor S(OS, raw_ostream::SAVEDCOLOR, सत्य, असत्य, Mode);

    यदि (ProgName && ProgName[0])
      S << ProgName << ": ";

    यदि (ShowLocation && !Filename.empty()) {
      यदि (Filename == "-")
        S << "<stdin>";
      अन्यथा
        S << Filename;

      यदि (पंक्ति_No != -1) {
        S << ':' << पंक्ति_No;
        यदि (ColumnNo != -1)
          S << ':' << (ColumnNo + 1);
      }
      S << ": ";
    }
  }

  यदि (ShowKindLabel) {
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

  WithColor(OS, raw_ostream::SAVEDCOLOR, सत्य, असत्य, Mode) << Message << '\n';

  यदि (पंक्ति_No == -1 || ColumnNo == -1)
    प्रत्यागम;

  // FIXME: If there are multibyte or multi-column characters in the source, all
  // our ranges will be wrong. To do this properly, we'll need a byte-to-column
  // map like Clang's TextDiagnostic. For now, we'll just handle tabs by
  // expanding them later, and bail out rather than show incorrect ranges and
  // misaligned fixits for any other odd characters.
  यदि (any_of(पंक्ति_Contents, isNonASCII)) {
    print_Source_पंक्ति(OS, पंक्ति_Contents);
    प्रत्यागम;
  }
  माप_प्रकार अंक_Columns = पंक्ति_Contents.size();

  // Build the line with the caret and ranges.
  std::string Caret_पंक्ति(अंक_Columns + 1, ' ');

  // Expand any ranges.
  प्रति (ध्रुव std::pair<अचिन्हित, अचिन्हित> &R : Ranges)
    std::fill(&Caret_पंक्ति[R.first],
              &Caret_पंक्ति[std::min((माप_प्रकार)R.second, Caret_पंक्ति.size())], '~');

  // Add any fix-its.
  // FIXME: Find the beginning of the line properly for multibyte characters.
  std::string Fix_It_Insertion_पंक्ति;
  build_Fix_It_पंक्ति(Caret_पंक्ति, Fix_It_Insertion_पंक्ति, FixIts,
                 ArrayRef(Loc.getPointer() - ColumnNo, पंक्ति_Contents.size()));

  // Finally, plop on the caret.
  यदि (अचिन्हित(ColumnNo) <= अंक_Columns)
    Caret_पंक्ति[ColumnNo] = '^';
  अन्यथा
    Caret_पंक्ति[अंक_Columns] = '^';

  // ... and remove trailing whitespace so the output doesn't wrap for it.  We
  // know that the line isn't completely empty because it has the caret in it at
  // least.
  Caret_पंक्ति.erase(Caret_पंक्ति.find_last_not_of(' ') + 1);

  print_Source_पंक्ति(OS, पंक्ति_Contents);

  {
    ColorMode Mode = ShowColors ? ColorMode::Auto : ColorMode::Disable;
    WithColor S(OS, raw_ostream::GREEN, सत्य, असत्य, Mode);

    // Print out the caret line, matching tabs in the source line.
    प्रति (अचिन्हित i = 0, e = Caret_पंक्ति.size(), OutCol = 0; i != e; ++i) {
      यदि (i >= पंक्ति_Contents.size() || पंक्ति_Contents[i] != '\t') {
        S << Caret_पंक्ति[i];
        ++OutCol;
        अनुवर्त;
      }

      // Okay, we have a tab.  Insert the appropriate number of characters.
      कुरु {
        S << Caret_पंक्ति[i];
        ++OutCol;
      } यावत् ((OutCol % TabStop) != 0);
    }
    S << '\n';
  }

  // Print out the replacement line, matching tabs in the source line.
  यदि (Fix_It_Insertion_पंक्ति.empty())
    प्रत्यागम;

  प्रति (माप_प्रकार i = 0, e = Fix_It_Insertion_पंक्ति.size(), OutCol = 0; i < e; ++i) {
    यदि (i >= पंक्ति_Contents.size() || पंक्ति_Contents[i] != '\t') {
      OS << Fix_It_Insertion_पंक्ति[i];
      ++OutCol;
      अनुवर्त;
    }

    // Okay, we have a tab.  Insert the appropriate number of characters.
    कुरु {
      OS << Fix_It_Insertion_पंक्ति[i];
      // FIXME: This is trying not to break up replacements, but then to re-sync
      // with the tabs between replacements. This will fail, though, if two
      // fix-it replacements are exactly adjacent, or if a fix-it contains a
      // space. Really we should be precomputing column widths, which we'll
      // need anyway for multibyte chars.
      यदि (Fix_It_Insertion_पंक्ति[i] != ' ')
        ++i;
      ++OutCol;
    } यावत् (((OutCol % TabStop) != 0) && i != e);
  }
  OS << '\n';
}
