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

उपयुज्य नामाकाश llvm;

स्थिर ध्रुव size_t TabStop = 8;

// Out of line to avoid needing definition of vfs::FileSystem in header.
SourceMgr::SourceMgr() = स्वभाव;
SourceMgr::SourceMgr(IntrusiveRefCntPtr<vfs::FileSystem> FS)
    : FS(std::move(FS)) {}
SourceMgr::SourceMgr(SourceMgr &&) = स्वभाव;
SourceMgr &SourceMgr::संक्रियकः=(SourceMgr &&) = स्वभाव;
SourceMgr::~SourceMgr() = स्वभाव;

IntrusiveRefCntPtr<vfs::FileSystem> SourceMgr::getVirtualFileSystem() ध्रुव {
  प्रत्यागम FS;
}

शून्य SourceMgr::समुच्चयः_Virtual_File_System(IntrusiveRefCntPtr<vfs::FileSystem> FS) {
  एतत्->FS = std::move(FS);
}

अचिह्नित SourceMgr::सङ्कलय_Include_File(ध्रुव std::string &Filename,
                                   SMLoc IncludeLoc,
                                   std::string &IncludedFile) {
  ErrorOr<std::unique_ptr<स्मृतिः_बफर>> NewBufOrErr =
      OpenIncludeFile(Filename, IncludedFile);
  यदि (!NewBufOrErr)
    प्रत्यागम 0;

  प्रत्यागम सङ्कलय_New_Source_बफर(std::move(*NewBufOrErr), IncludeLoc);
}

ErrorOr<std::unique_ptr<स्मृतिः_बफर>>
SourceMgr::OpenIncludeFile(ध्रुव std::string &Filename,
                           std::string &IncludedFile,
                           तार्किक RequiresNullTerminator) {
  स्वचालित GetFile = [एतत्, RequiresNullTerminator](StringRef मार्गः) {
    प्रत्यागम FS ? FS->get_बफर_For_File(मार्गः, /*FileSize=*/-1,
                                     RequiresNullTerminator)
              : स्मृतिः_बफर::getFile(मार्गः, /*IsText=*/असत्य,
                                      RequiresNullTerminator);
  };

  ErrorOr<std::unique_ptr<स्मृतिः_बफर>> NewBufOrErr = GetFile(Filename);

  SmallString<64> बफर(Filename);
  // If the file didn't exist directly, see if it's in an include path.
  प्रति (अचिह्नित i = 0, e = IncludeDirectories.आकार_2(); i != e && !NewBufOrErr;
       ++i) {
    बफर = IncludeDirectories[i];
    sys::मार्गः_2::append(बफर, Filename);
    NewBufOrErr = GetFile(बफर);
  }

  यदि (NewBufOrErr)
    IncludedFile = static_cast<std::string>(बफर);

  प्रत्यागम NewBufOrErr;
}

अचिह्नित SourceMgr::Find_बफर_Containing_Loc(SMLoc Loc) ध्रुव {
  प्रति (अचिह्नित i = 0, e = Buffers.आकार_2(); i != e; ++i)
    यदि (Loc.getPointer() >= Buffers[i].बफर->get_बफर_Start() &&
        // Use <= here so that a pointer to the null at the end of the buffer
        // is included as part of the buffer.
        Loc.getPointer() <= Buffers[i].बफर->get_बफर_अन्तः())
      प्रत्यागम i + 1;
  प्रत्यागम 0;
}

प्रतिरूपम् <प्रकारनाम T>
स्थिर std::सदिशम्<T> &Get_Or_Create_विस्थापनम्_Cache(शून्य *&विस्थापनम्_Cache,
                                              स्मृतिः_बफर *बफर) {
  यदि (विस्थापनम्_Cache)
    प्रत्यागम *static_cast<std::सदिशम्<T> *>(विस्थापनम्_Cache);

  // Lazily fill in the offset cache.
  स्वचालित *Offsets = नूतन std::सदिशम्<T>();
  size_t Sz = बफर->get_बफर_आकार();
  प्रतिज्ञा(Sz <= std::numeric_limits<T>::max());
  StringRef S = बफर->get_बफर();
  प्रति (size_t N = 0; N < Sz; ++N) {
    यदि (S[N] == '\n')
      Offsets->प्रक्षिप_पृष्ठभागः(static_cast<T>(N));
  }

  विस्थापनम्_Cache = Offsets;
  प्रत्यागम *Offsets;
}

प्रतिरूपम् <प्रकारनाम T>
std::pair<अचिह्नित, अचिह्नित>
SourceMgr::Src_बफर::get_पङ्क्तिः_And_Column_Specialized(ध्रुव वर्ण *Ptr) ध्रुव {
  std::सदिशम्<T> &Offsets =
      Get_Or_Create_विस्थापनम्_Cache<T>(विस्थापनम्_Cache, बफर.get());

  ध्रुव वर्ण *BufStart = बफर->get_बफर_Start();
  प्रतिज्ञा(Ptr >= BufStart && Ptr <= बफर->get_बफर_अन्तः());
  ptrdiff_t PtrDiff = Ptr - BufStart;
  प्रतिज्ञा(PtrDiff >= 0 &&
         static_cast<size_t>(PtrDiff) <= std::numeric_limits<T>::max());
  T Ptr_विस्थापनम् = static_cast<T>(PtrDiff);

  // llvm::lower_bound gives the number of EOL before PtrOffset. Add 1 to get
  // the line number.
  स्वचालित I = llvm::अधस्तनम्_bound(Offsets, Ptr_विस्थापनम्);
  अचिह्नित पङ्क्तिः_No = I - Offsets.begin() + 1;

  // The column number is the distance from the previous EOL (or the start of
  // the buffer) to the pointer.
  T पङ्क्तिः_Start_Offs = (I == Offsets.begin()) ? 0 : (I[-1] + 1);
  अचिह्नित ColNo = Ptr_विस्थापनम् - पङ्क्तिः_Start_Offs + 1;
  प्रत्यागम {पङ्क्तिः_No, ColNo};
}

/// Look up a given \p Ptr in the buffer, determining which line and column
/// it came from.
LLVM_ABI std::pair<अचिह्नित, अचिह्नित>
SourceMgr::Src_बफर::get_पङ्क्तिः_And_Column(ध्रुव वर्ण *Ptr) ध्रुव {
  size_t Sz = बफर->get_बफर_आकार();
  यदि (Sz <= std::numeric_limits<uint8_t>::max())
    प्रत्यागम get_पङ्क्तिः_And_Column_Specialized<uint8_t>(Ptr);
  यदि (Sz <= std::numeric_limits<uint16_t>::max())
    प्रत्यागम get_पङ्क्तिः_And_Column_Specialized<uint16_t>(Ptr);
  यदि (Sz <= std::numeric_limits<uint32_t>::max())
    प्रत्यागम get_पङ्क्तिः_And_Column_Specialized<uint32_t>(Ptr);
  प्रत्यागम get_पङ्क्तिः_And_Column_Specialized<uint64_t>(Ptr);
}

प्रतिरूपम् <प्रकारनाम T>
ध्रुव वर्ण *SourceMgr::Src_बफर::get_Pointer_For_पङ्क्तिः_सङ्ख्या_Specialized(
    अचिह्नित पङ्क्तिः_No) ध्रुव {
  std::सदिशम्<T> &Offsets =
      Get_Or_Create_विस्थापनम्_Cache<T>(विस्थापनम्_Cache, बफर.get());

  // We start counting line and column numbers from 1.
  यदि (पङ्क्तिः_No != 0)
    --पङ्क्तिः_No;

  ध्रुव वर्ण *BufStart = बफर->get_बफर_Start();

  // The offset cache contains the location of the \n for the specified line,
  // we want the start of the line.  As such, we look for the previous entry.
  यदि (पङ्क्तिः_No == 0)
    प्रत्यागम BufStart;
  यदि (पङ्क्तिः_No > Offsets.आकार_2())
    प्रत्यागम शून्यसूचकः;
  प्रत्यागम BufStart + Offsets[पङ्क्तिः_No - 1] + 1;
}

/// Return a pointer to the first character of the specified line number or
/// null if the line number is invalid.
ध्रुव वर्ण *
SourceMgr::Src_बफर::get_Pointer_For_पङ्क्तिः_सङ्ख्या(अचिह्नित पङ्क्तिः_No) ध्रुव {
  size_t Sz = बफर->get_बफर_आकार();
  यदि (Sz <= std::numeric_limits<uint8_t>::max())
    प्रत्यागम get_Pointer_For_पङ्क्तिः_सङ्ख्या_Specialized<uint8_t>(पङ्क्तिः_No);
  अन्यथा यदि (Sz <= std::numeric_limits<uint16_t>::max())
    प्रत्यागम get_Pointer_For_पङ्क्तिः_सङ्ख्या_Specialized<uint16_t>(पङ्क्तिः_No);
  अन्यथा यदि (Sz <= std::numeric_limits<uint32_t>::max())
    प्रत्यागम get_Pointer_For_पङ्क्तिः_सङ्ख्या_Specialized<uint32_t>(पङ्क्तिः_No);
  अन्यथा
    प्रत्यागम get_Pointer_For_पङ्क्तिः_सङ्ख्या_Specialized<uint64_t>(पङ्क्तिः_No);
}

SourceMgr::Src_बफर::Src_बफर(SourceMgr::Src_बफर &&Other)
    : बफर(std::move(Other.बफर)), विस्थापनम्_Cache(Other.विस्थापनम्_Cache),
      IncludeLoc(Other.IncludeLoc) {
  Other.विस्थापनम्_Cache = शून्यसूचकः;
}

SourceMgr::Src_बफर::~Src_बफर() {
  यदि (विस्थापनम्_Cache) {
    size_t Sz = बफर->get_बफर_आकार();
    यदि (Sz <= std::numeric_limits<uint8_t>::max())
      विलोपय static_cast<std::सदिशम्<uint8_t> *>(विस्थापनम्_Cache);
    अन्यथा यदि (Sz <= std::numeric_limits<uint16_t>::max())
      विलोपय static_cast<std::सदिशम्<uint16_t> *>(विस्थापनम्_Cache);
    अन्यथा यदि (Sz <= std::numeric_limits<uint32_t>::max())
      विलोपय static_cast<std::सदिशम्<uint32_t> *>(विस्थापनम्_Cache);
    अन्यथा
      विलोपय static_cast<std::सदिशम्<uint64_t> *>(विस्थापनम्_Cache);
    विस्थापनम्_Cache = शून्यसूचकः;
  }
}

std::pair<अचिह्नित, अचिह्नित>
SourceMgr::get_पङ्क्तिः_And_Column(SMLoc Loc, अचिह्नित बफर_ID) ध्रुव {
  यदि (!बफर_ID)
    बफर_ID = Find_बफर_Containing_Loc(Loc);
  प्रतिज्ञा(बफर_ID && "Invalid location!");

  स्वचालित &SB = get_बफर_Info(बफर_ID);
  ध्रुव वर्ण *Ptr = Loc.getPointer();

  प्रत्यागम SB.get_पङ्क्तिः_And_Column(Ptr);
}

// FIXME: Note that the formatting of source locations is spread between
// multiple functions, some in SourceMgr and some in SMDiagnostic. A better
// solution would be a general-purpose source location formatter
// in one of those two classes, or possibly in SMLoc.

/// Get a string with the source location formatted in the standard
/// style, but without the line offset. If \p IncludePath is true, the path
/// is included. If false, only the file name and extension are included.
std::string SourceMgr::get_Formatted_Location_No_विस्थापनम्(SMLoc Loc,
                                                    तार्किक Include_मार्गः) ध्रुव {
  स्वचालित बफर_ID = Find_बफर_Containing_Loc(Loc);
  प्रतिज्ञा(बफर_ID && "Invalid location!");
  स्वचालित FileSpec = get_बफर_Info(बफर_ID).बफर->get_बफर_Identifier();

  यदि (Include_मार्गः) {
    प्रत्यागम FileSpec.str() + ":" + std::गन्तव्यम्_string(Find_पङ्क्तिः_सङ्ख्या(Loc, बफर_ID));
  } अन्यथा {
    स्वचालित I = FileSpec.find_चरमः_सम्बन्धि("/\\");
    I = (I == FileSpec.आकार_2()) ? 0 : (I + 1);
    प्रत्यागम FileSpec.substr(I).str() + ":" +
           std::गन्तव्यम्_string(Find_पङ्क्तिः_सङ्ख्या(Loc, बफर_ID));
  }
}

/// Given a line and column number in a mapped buffer, turn it into an SMLoc.
/// This will return a null SMLoc if the line/column location is invalid.
SMLoc SourceMgr::Find_Loc_For_पङ्क्तिः_And_Column(अचिह्नित बफर_ID, अचिह्नित पङ्क्तिः_No,
                                         अचिह्नित ColNo) {
  स्वचालित &SB = get_बफर_Info(बफर_ID);
  ध्रुव वर्ण *Ptr = SB.get_Pointer_For_पङ्क्तिः_सङ्ख्या(पङ्क्तिः_No);
  यदि (!Ptr)
    प्रत्यागम SMLoc();

  // We start counting line and column numbers from 1.
  यदि (ColNo != 0)
    --ColNo;

  // If we have a column number, validate it.
  यदि (ColNo) {
    // Make sure the location is within the current line.
    यदि (Ptr + ColNo > SB.बफर->get_बफर_अन्तः())
      प्रत्यागम SMLoc();

    // Make sure there is no newline in the way.
    यदि (StringRef(Ptr, ColNo).find_first_सम्बन्धि("\n\r") != StringRef::npos)
      प्रत्यागम SMLoc();

    Ptr += ColNo;
  }

  प्रत्यागम SMLoc::getFromPointer(Ptr);
}

शून्य SourceMgr::Print_Include_राशिः(SMLoc IncludeLoc, raw_ostream &OS) ध्रुव {
  यदि (IncludeLoc == SMLoc())
    प्रत्यागम; // Top of stack.

  अचिह्नित CurBuf = Find_बफर_Containing_Loc(IncludeLoc);
  प्रतिज्ञा(CurBuf && "Invalid or unspecified location!");

  Print_Include_राशिः(get_बफर_Info(CurBuf).IncludeLoc, OS);

  OS << "Included from " << get_बफर_Info(CurBuf).बफर->get_बफर_Identifier()
     << ":" << Find_पङ्क्तिः_सङ्ख्या(IncludeLoc, CurBuf) << ":\n";
}

शून्य SourceMgr::print_Include_राशिः_For_Diagnostic(SMLoc Loc,
                                               raw_ostream &OS) ध्रुव {
  यदि (!Loc.isValid())
    प्रत्यागम;
  अचिह्नित Diag_Cur_बफर = Find_बफर_Containing_Loc(Loc);
  यदि (Diag_Cur_बफर && Diag_Cur_बफर != getMainFileID()) {
    SMLoc ParentIncludeLoc = getParentIncludeLoc(Diag_Cur_बफर);
    // Ignore macro instantiation buffers to avoid redundant include stacks.
    Print_Include_राशिः(ParentIncludeLoc, OS);
  }
}

SMDiagnostic SourceMgr::Get_सन्देशः(SMLoc Loc, SourceMgr::Diag_विधा विधा_2,
                                   ध्रुव Twine &सन्देशः_4, ArrayRef<SMRange> Ranges,
                                   ArrayRef<SMFixIt> FixIts) ध्रुव {
  // First thing to do: find the current buffer containing the specified
  // location to pull out the source line.
  Small_सदिशम्<std::pair<अचिह्नित, अचिह्नित>, 4> ColRanges;
  std::pair<अचिह्नित, अचिह्नित> पङ्क्तिः_And_Col;
  StringRef बफर_ID = "<unknown>";
  StringRef पङ्क्तिः_Str;

  यदि (Loc.isValid()) {
    अचिह्नित CurBuf = Find_बफर_Containing_Loc(Loc);
    प्रतिज्ञा(CurBuf && "Invalid or unspecified location!");

    ध्रुव स्मृतिः_बफर *CurMB = get_स्मृतिः_बफर(CurBuf);
    बफर_ID = CurMB->get_बफर_Identifier();

    // Scan backward to find the start of the line.
    ध्रुव वर्ण *पङ्क्तिः_Start = Loc.getPointer();
    ध्रुव वर्ण *BufStart = CurMB->get_बफर_Start();
    यावत् (पङ्क्तिः_Start != BufStart && पङ्क्तिः_Start[-1] != '\n' &&
           पङ्क्तिः_Start[-1] != '\r')
      --पङ्क्तिः_Start;

    // Get the end of the line.
    ध्रुव वर्ण *पङ्क्तिः_अन्तः = Loc.getPointer();
    ध्रुव वर्ण *Buf_अन्तः = CurMB->get_बफर_अन्तः();
    यावत् (पङ्क्तिः_अन्तः != Buf_अन्तः && पङ्क्तिः_अन्तः[0] != '\n' && पङ्क्तिः_अन्तः[0] != '\r')
      ++पङ्क्तिः_अन्तः;
    पङ्क्तिः_Str = StringRef(पङ्क्तिः_Start, पङ्क्तिः_अन्तः - पङ्क्तिः_Start);

    // Convert any ranges to column ranges that only intersect the line of the
    // location.
    प्रति (SMRange R : Ranges) {
      यदि (!R.isValid())
        अनुवर्त;

      // If the line doesn't contain any part of the range, then ignore it.
      यदि (R.Start.getPointer() > पङ्क्तिः_अन्तः || R.अन्तः_2.getPointer() < पङ्क्तिः_Start)
        अनुवर्त;

      // Ignore pieces of the range that go onto other lines.
      यदि (R.Start.getPointer() < पङ्क्तिः_Start)
        R.Start = SMLoc::getFromPointer(पङ्क्तिः_Start);
      यदि (R.अन्तः_2.getPointer() > पङ्क्तिः_अन्तः)
        R.अन्तः_2 = SMLoc::getFromPointer(पङ्क्तिः_अन्तः);

      // Translate from SMLoc ranges to column ranges.
      // FIXME: Handle multibyte characters.
      ColRanges.प्रक्षिप_पृष्ठभागः(std::make_pair(R.Start.getPointer() - पङ्क्तिः_Start,
                                         R.अन्तः_2.getPointer() - पङ्क्तिः_Start));
    }

    पङ्क्तिः_And_Col = get_पङ्क्तिः_And_Column(Loc, CurBuf);
  }

  प्रत्यागम SMDiagnostic(*एतत्, Loc, बफर_ID, पङ्क्तिः_And_Col.first,
                      पङ्क्तिः_And_Col.second - 1, विधा_2, सन्देशः_4.str(), पङ्क्तिः_Str,
                      ColRanges, FixIts);
}

शून्य SourceMgr::Print_सन्देशः(raw_ostream &OS, ध्रुव SMDiagnostic &Diagnostic,
                             तार्किक ShowColors) ध्रुव {
  // Report the message with the diagnostic handler if present.
  यदि (DiagHandler) {
    DiagHandler(Diagnostic, DiagContext);
    प्रत्यागम;
  }

  यदि (Diagnostic.getLoc().isValid()) {
    अचिह्नित CurBuf = Find_बफर_Containing_Loc(Diagnostic.getLoc());
    प्रतिज्ञा(CurBuf && "Invalid or unspecified location!");
    Print_Include_राशिः(get_बफर_Info(CurBuf).IncludeLoc, OS);
  }

  Diagnostic.print(शून्यसूचकः, OS, ShowColors);
}

शून्य SourceMgr::Print_सन्देशः(raw_ostream &OS, SMLoc Loc,
                             SourceMgr::Diag_विधा विधा_2, ध्रुव Twine &सन्देशः_4,
                             ArrayRef<SMRange> Ranges, ArrayRef<SMFixIt> FixIts,
                             तार्किक ShowColors) ध्रुव {
  Print_सन्देशः(OS, Get_सन्देशः(Loc, विधा_2, सन्देशः_4, Ranges, FixIts), ShowColors);
}

शून्य SourceMgr::Print_सन्देशः(SMLoc Loc, SourceMgr::Diag_विधा विधा_2,
                             ध्रुव Twine &सन्देशः_4, ArrayRef<SMRange> Ranges,
                             ArrayRef<SMFixIt> FixIts, तार्किक ShowColors) ध्रुव {
  Print_सन्देशः(errs(), Loc, विधा_2, सन्देशः_4, Ranges, FixIts, ShowColors);
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

SMDiagnostic::SMDiagnostic(ध्रुव SourceMgr &sm, SMLoc L, StringRef FN, पूर्णाङ्क पङ्क्तिः,
                           पूर्णाङ्क Col, SourceMgr::Diag_विधा विधा_2, StringRef सन्देशः_4,
                           StringRef पङ्क्तिः_Str,
                           ArrayRef<std::pair<अचिह्नित, अचिह्नित>> Ranges,
                           ArrayRef<SMFixIt> Hints)
    : SM(&sm), Loc(L), Filename(std::string(FN)), पङ्क्तिः_No(पङ्क्तिः), ColumnNo(Col),
      विधा_2(विधा_2), सन्देशः_3(सन्देशः_4), पङ्क्तिः_Contents(पङ्क्तिः_Str), Ranges(Ranges.vec()),
      FixIts(Hints) {
  llvm::क्रमय(FixIts);
}

स्थिर शून्य build_Fix_It_पङ्क्तिः(std::string &Caret_पङ्क्तिः, std::string &Fix_It_पङ्क्तिः,
                           ArrayRef<SMFixIt> FixIts,
                           ArrayRef<वर्ण> Source_पङ्क्तिः) {
  यदि (FixIts.रिक्तानि())
    प्रत्यागम;

  ध्रुव वर्ण *पङ्क्तिः_Start = Source_पङ्क्तिः.begin();
  ध्रुव वर्ण *पङ्क्तिः_अन्तः = Source_पङ्क्तिः.अन्तः();

  size_t पूर्व_Hint_अन्तः_Col = 0;

  प्रति (ध्रुव llvm::SMFixIt &Fixit : FixIts) {
    // If the fixit contains a newline or tab, ignore it.
    यदि (Fixit.getText().find_first_सम्बन्धि("\n\r\t") != StringRef::npos)
      अनुवर्त;

    SMRange R = Fixit.getRange();

    // If the line doesn't contain any part of the range, then ignore it.
    यदि (R.Start.getPointer() > पङ्क्तिः_अन्तः || R.अन्तः_2.getPointer() < पङ्क्तिः_Start)
      अनुवर्त;

    // Translate from SMLoc to column.
    // Ignore pieces of the range that go onto other lines.
    // FIXME: Handle multibyte characters in the source line.
    अचिह्नित FirstCol;
    यदि (R.Start.getPointer() < पङ्क्तिः_Start)
      FirstCol = 0;
    अन्यथा
      FirstCol = R.Start.getPointer() - पङ्क्तिः_Start;

    // If we inserted a long previous hint, push this one forwards, and add
    // an extra space to show that this is not part of the previous
    // completion. This is sort of the best we can do when two hints appear
    // to overlap.
    //
    // Note that if this hint is located immediately after the previous
    // hint, no space will be added, since the location is more important.
    अचिह्नित HintCol = FirstCol;
    यदि (HintCol < पूर्व_Hint_अन्तः_Col)
      HintCol = पूर्व_Hint_अन्तः_Col + 1;

    // FIXME: This assertion is intended to catch unintended use of multibyte
    // characters in fixits. If we decide to do this, we'll have to track
    // separate byte widths for the source and fixit lines.
    प्रतिज्ञा((size_t)sys::locale::column_विस्तारः(Fixit.getText()) ==
           Fixit.getText().आकार_2());

    // This relies on one byte per column in our fixit hints.
    अचिह्नित चरमः_Column_Modified = HintCol + Fixit.getText().आकार_2();
    यदि (चरमः_Column_Modified > Fix_It_पङ्क्तिः.आकार_2())
      Fix_It_पङ्क्तिः.resize(चरमः_Column_Modified, ' ');

    llvm::copy(Fixit.getText(), Fix_It_पङ्क्तिः.begin() + HintCol);

    पूर्व_Hint_अन्तः_Col = चरमः_Column_Modified;

    // For replacements, mark the removal range with '~'.
    // FIXME: Handle multibyte characters in the source line.
    अचिह्नित चरमः_Col;
    यदि (R.अन्तः_2.getPointer() >= पङ्क्तिः_अन्तः)
      चरमः_Col = पङ्क्तिः_अन्तः - पङ्क्तिः_Start;
    अन्यथा
      चरमः_Col = R.अन्तः_2.getPointer() - पङ्क्तिः_Start;

    std::fill(&Caret_पङ्क्तिः[FirstCol], &Caret_पङ्क्तिः[चरमः_Col], '~');
  }
}

स्थिर शून्य print_Source_पङ्क्तिः(raw_ostream &S, StringRef पङ्क्तिः_Contents) {
  // Print out the source line one character at a time, so we can expand tabs.
  प्रति (अचिह्नित i = 0, e = पङ्क्तिः_Contents.आकार_2(), बहिर्गमनम्_Col = 0; i != e; ++i) {
    size_t अग्र_Tab = पङ्क्तिः_Contents.find('\t', i);
    // If there were no tabs left, print the rest, we are done.
    यदि (अग्र_Tab == StringRef::npos) {
      S << पङ्क्तिः_Contents.drop_front(i);
      भङ्ग;
    }

    // Otherwise, print from i to NextTab.
    S << पङ्क्तिः_Contents.खण्डिका(i, अग्र_Tab);
    बहिर्गमनम्_Col += अग्र_Tab - i;
    i = अग्र_Tab;

    // If we have a tab, emit at least one space, then round up to 8 columns.
    कुरु {
      S << ' ';
      ++बहिर्गमनम्_Col;
    } यावत् ((बहिर्गमनम्_Col % TabStop) != 0);
  }
  S << '\n';
}

स्थिर तार्किक isNonASCII(वर्ण c) { प्रत्यागम c & 0x80; }

शून्य SMDiagnostic::print(ध्रुव वर्ण *ProgName, raw_ostream &OS, तार्किक ShowColors,
                         तार्किक Show_विधा_Label, तार्किक ShowLocation) ध्रुव {
  Color_बहुलकम् बहुलकम् = ShowColors ? Color_बहुलकम्::Auto : Color_बहुलकम्::Disable;

  {
    WithColor S(OS, raw_ostream::SAVEDCOLOR, सत्य, असत्य, बहुलकम्);

    यदि (ProgName && ProgName[0])
      S << ProgName << ": ";

    यदि (ShowLocation && !Filename.रिक्तानि()) {
      यदि (Filename == "-")
        S << "<stdin>";
      अन्यथा
        S << Filename;

      यदि (पङ्क्तिः_No != -1) {
        S << ':' << पङ्क्तिः_No;
        यदि (ColumnNo != -1)
          S << ':' << (ColumnNo + 1);
      }
      S << ": ";
    }
  }

  यदि (Show_विधा_Label) {
    विकल्प (विधा_2) {
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

  WithColor(OS, raw_ostream::SAVEDCOLOR, सत्य, असत्य, बहुलकम्) << सन्देशः_3 << '\n';

  यदि (पङ्क्तिः_No == -1 || ColumnNo == -1)
    प्रत्यागम;

  // FIXME: If there are multibyte or multi-column characters in the source, all
  // our ranges will be wrong. To do this properly, we'll need a byte-to-column
  // map like Clang's TextDiagnostic. For now, we'll just handle tabs by
  // expanding them later, and bail out rather than show incorrect ranges and
  // misaligned fixits for any other odd characters.
  यदि (any_सम्बन्धि(पङ्क्तिः_Contents, isNonASCII)) {
    print_Source_पङ्क्तिः(OS, पङ्क्तिः_Contents);
    प्रत्यागम;
  }
  size_t अङ्कः_Columns = पङ्क्तिः_Contents.आकार_2();

  // Build the line with the caret and ranges.
  std::string Caret_पङ्क्तिः(अङ्कः_Columns + 1, ' ');

  // Expand any ranges.
  प्रति (ध्रुव std::pair<अचिह्नित, अचिह्नित> &R : Ranges)
    std::fill(&Caret_पङ्क्तिः[R.first],
              &Caret_पङ्क्तिः[std::min((size_t)R.second, Caret_पङ्क्तिः.आकार_2())], '~');

  // Add any fix-its.
  // FIXME: Find the beginning of the line properly for multibyte characters.
  std::string Fix_It_Insertion_पङ्क्तिः;
  build_Fix_It_पङ्क्तिः(Caret_पङ्क्तिः, Fix_It_Insertion_पङ्क्तिः, FixIts,
                 ArrayRef(Loc.getPointer() - ColumnNo, पङ्क्तिः_Contents.आकार_2()));

  // Finally, plop on the caret.
  यदि (अचिह्नित(ColumnNo) <= अङ्कः_Columns)
    Caret_पङ्क्तिः[ColumnNo] = '^';
  अन्यथा
    Caret_पङ्क्तिः[अङ्कः_Columns] = '^';

  // ... and remove trailing whitespace so the output doesn't wrap for it.  We
  // know that the line isn't completely empty because it has the caret in it at
  // least.
  Caret_पङ्क्तिः.erase(Caret_पङ्क्तिः.find_चरमः_not_सम्बन्धि(' ') + 1);

  print_Source_पङ्क्तिः(OS, पङ्क्तिः_Contents);

  {
    Color_बहुलकम् बहुलकम् = ShowColors ? Color_बहुलकम्::Auto : Color_बहुलकम्::Disable;
    WithColor S(OS, raw_ostream::GREEN, सत्य, असत्य, बहुलकम्);

    // Print out the caret line, matching tabs in the source line.
    प्रति (अचिह्नित i = 0, e = Caret_पङ्क्तिः.आकार_2(), बहिर्गमनम्_Col = 0; i != e; ++i) {
      यदि (i >= पङ्क्तिः_Contents.आकार_2() || पङ्क्तिः_Contents[i] != '\t') {
        S << Caret_पङ्क्तिः[i];
        ++बहिर्गमनम्_Col;
        अनुवर्त;
      }

      // Okay, we have a tab.  Insert the appropriate number of characters.
      कुरु {
        S << Caret_पङ्क्तिः[i];
        ++बहिर्गमनम्_Col;
      } यावत् ((बहिर्गमनम्_Col % TabStop) != 0);
    }
    S << '\n';
  }

  // Print out the replacement line, matching tabs in the source line.
  यदि (Fix_It_Insertion_पङ्क्तिः.रिक्तानि())
    प्रत्यागम;

  प्रति (size_t i = 0, e = Fix_It_Insertion_पङ्क्तिः.आकार_2(), बहिर्गमनम्_Col = 0; i < e; ++i) {
    यदि (i >= पङ्क्तिः_Contents.आकार_2() || पङ्क्तिः_Contents[i] != '\t') {
      OS << Fix_It_Insertion_पङ्क्तिः[i];
      ++बहिर्गमनम्_Col;
      अनुवर्त;
    }

    // Okay, we have a tab.  Insert the appropriate number of characters.
    कुरु {
      OS << Fix_It_Insertion_पङ्क्तिः[i];
      // FIXME: This is trying not to break up replacements, but then to re-sync
      // with the tabs between replacements. This will fail, though, if two
      // fix-it replacements are exactly adjacent, or if a fix-it contains a
      // space. Really we should be precomputing column widths, which we'll
      // need anyway for multibyte chars.
      यदि (Fix_It_Insertion_पङ्क्तिः[i] != ' ')
        ++i;
      ++बहिर्गमनम्_Col;
    } यावत् (((बहिर्गमनम्_Col % TabStop) != 0) && i != e);
  }
  OS << '\n';
}
