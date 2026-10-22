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

کارول نطاق llvm;

ساكن ثابت size_t TabStop = 8;

// Out of line to avoid needing definition of vfs::FileSystem in header.
سرچینه_Mgr::سرچینه_Mgr() = افتراضي;
سرچینه_Mgr::سرچینه_Mgr(IntrusiveRefCntPtr<vfs::FileSystem> FS)
    : FS(std::move(FS)) {}
سرچینه_Mgr::سرچینه_Mgr(سرچینه_Mgr &&) = افتراضي;
سرچینه_Mgr &سرچینه_Mgr::چلوونکی=(سرچینه_Mgr &&) = افتراضي;
سرچینه_Mgr::~سرچینه_Mgr() = افتراضي;

IntrusiveRefCntPtr<vfs::FileSystem> سرچینه_Mgr::getVirtualFileSystem() ثابت {
  أرجع FS;
}

فراغ سرچینه_Mgr::سېټ_Virtual_File_System(IntrusiveRefCntPtr<vfs::FileSystem> FS) {
  هذا->FS = std::move(FS);
}

بینښه سرچینه_Mgr::جمع_کړه_Include_File(ثابت std::string &Filename,
                                   SMLoc IncludeLoc,
                                   std::string &IncludedFile) {
  ErrorOr<std::unique_ptr<حافظه_بفر>> NewBufOrErr =
      OpenIncludeFile(Filename, IncludedFile);
  إذا (!NewBufOrErr)
    أرجع 0;

  أرجع جمع_کړه_New_سرچینه_بفر(std::move(*NewBufOrErr), IncludeLoc);
}

ErrorOr<std::unique_ptr<حافظه_بفر>>
سرچینه_Mgr::OpenIncludeFile(ثابت std::string &Filename,
                           std::string &IncludedFile,
                           منطقي RequiresNullTerminator) {
  خپلکاری GetFile = [هذا, RequiresNullTerminator](StringRef لار) {
    أرجع FS ? FS->get_بفر_For_File(لار, /*FileSize=*/-1,
                                     RequiresNullTerminator)
              : حافظه_بفر::getFile(لار, /*IsText=*/خطأ,
                                      RequiresNullTerminator);
  };

  ErrorOr<std::unique_ptr<حافظه_بفر>> NewBufOrErr = GetFile(Filename);

  SmallString<64> بفر(Filename);
  // If the file didn't exist directly, see if it's in an include path.
  لكل (بینښه i = 0, e = IncludeDirectories.اندازه_2(); i != e && !NewBufOrErr;
       ++i) {
    بفر = IncludeDirectories[i];
    sys::لار_2::append(بفر, Filename);
    NewBufOrErr = GetFile(بفر);
  }

  إذا (NewBufOrErr)
    IncludedFile = static_cast<std::string>(بفر);

  أرجع NewBufOrErr;
}

بینښه سرچینه_Mgr::پیدا_کړه_بفر_Containing_Loc(SMLoc Loc) ثابت {
  لكل (بینښه i = 0, e = Buffers.اندازه_2(); i != e; ++i)
    إذا (Loc.get_اشاره_ګر() >= Buffers[i].بفر->get_بفر_پیل() &&
        // Use <= here so that a pointer to the null at the end of the buffer
        // is included as part of the buffer.
        Loc.get_اشاره_ګر() <= Buffers[i].بفر->get_بفر_پای_ټکی())
      أرجع i + 1;
  أرجع 0;
}

کينډۍ <ډول_نوم T>
ساكن std::ویکټور<T> &Get_Or_Create_دننه_واټن_Cache(فراغ *&دننه_واټن_Cache,
                                              حافظه_بفر *بفر) {
  إذا (دننه_واټن_Cache)
    أرجع *static_cast<std::ویکټور<T> *>(دننه_واټن_Cache);

  // Lazily fill in the offset cache.
  خپلکاری *Offsets = جديد std::ویکټور<T>();
  size_t Sz = بفر->get_بفر_اندازه();
  أكد(Sz <= std::numeric_limits<T>::max());
  StringRef S = بفر->get_بفر();
  لكل (size_t N = 0; N < Sz; ++N) {
    إذا (S[N] == '\n')
      Offsets->ټېل_شا(static_cast<T>(N));
  }

  دننه_واټن_Cache = Offsets;
  أرجع *Offsets;
}

کينډۍ <ډول_نوم T>
std::pair<بینښه, بینښه>
سرچینه_Mgr::Src_بفر::get_کرښه_And_ستنه_Specialized(ثابت حرف *Ptr) ثابت {
  std::ویکټور<T> &Offsets =
      Get_Or_Create_دننه_واټن_Cache<T>(دننه_واټن_Cache, بفر.get());

  ثابت حرف *Buf_پیل = بفر->get_بفر_پیل();
  أكد(Ptr >= Buf_پیل && Ptr <= بفر->get_بفر_پای_ټکی());
  ptrdiff_t PtrDiff = Ptr - Buf_پیل;
  أكد(PtrDiff >= 0 &&
         static_cast<size_t>(PtrDiff) <= std::numeric_limits<T>::max());
  T Ptr_دننه_واټن = static_cast<T>(PtrDiff);

  // llvm::lower_bound gives the number of EOL before PtrOffset. Add 1 to get
  // the line number.
  خپلکاری I = llvm::ښکتنی_bound(Offsets, Ptr_دننه_واټن);
  بینښه کرښه_No = I - Offsets.begin() + 1;

  // The column number is the distance from the previous EOL (or the start of
  // the buffer) to the pointer.
  T کرښه_پیل_Offs = (I == Offsets.begin()) ? 0 : (I[-1] + 1);
  بینښه ستون_No = Ptr_دننه_واټن - کرښه_پیل_Offs + 1;
  أرجع {کرښه_No, ستون_No};
}

/// Look up a given \p Ptr in the buffer, determining which line and column
/// it came from.
LLVM_ABI std::pair<بینښه, بینښه>
سرچینه_Mgr::Src_بفر::get_کرښه_And_ستنه(ثابت حرف *Ptr) ثابت {
  size_t Sz = بفر->get_بفر_اندازه();
  إذا (Sz <= std::numeric_limits<uint8_t>::max())
    أرجع get_کرښه_And_ستنه_Specialized<uint8_t>(Ptr);
  إذا (Sz <= std::numeric_limits<uint16_t>::max())
    أرجع get_کرښه_And_ستنه_Specialized<uint16_t>(Ptr);
  إذا (Sz <= std::numeric_limits<uint32_t>::max())
    أرجع get_کرښه_And_ستنه_Specialized<uint32_t>(Ptr);
  أرجع get_کرښه_And_ستنه_Specialized<uint64_t>(Ptr);
}

کينډۍ <ډول_نوم T>
ثابت حرف *سرچینه_Mgr::Src_بفر::get_اشاره_ګر_For_کرښه_شمېره_Specialized(
    بینښه کرښه_No) ثابت {
  std::ویکټور<T> &Offsets =
      Get_Or_Create_دننه_واټن_Cache<T>(دننه_واټن_Cache, بفر.get());

  // We start counting line and column numbers from 1.
  إذا (کرښه_No != 0)
    --کرښه_No;

  ثابت حرف *Buf_پیل = بفر->get_بفر_پیل();

  // The offset cache contains the location of the \n for the specified line,
  // we want the start of the line.  As such, we look for the previous entry.
  إذا (کرښه_No == 0)
    أرجع Buf_پیل;
  إذا (کرښه_No > Offsets.اندازه_2())
    أرجع تش_اشاره;
  أرجع Buf_پیل + Offsets[کرښه_No - 1] + 1;
}

/// Return a pointer to the first character of the specified line number or
/// null if the line number is invalid.
ثابت حرف *
سرچینه_Mgr::Src_بفر::get_اشاره_ګر_For_کرښه_شمېره(بینښه کرښه_No) ثابت {
  size_t Sz = بفر->get_بفر_اندازه();
  إذا (Sz <= std::numeric_limits<uint8_t>::max())
    أرجع get_اشاره_ګر_For_کرښه_شمېره_Specialized<uint8_t>(کرښه_No);
  وإلا إذا (Sz <= std::numeric_limits<uint16_t>::max())
    أرجع get_اشاره_ګر_For_کرښه_شمېره_Specialized<uint16_t>(کرښه_No);
  وإلا إذا (Sz <= std::numeric_limits<uint32_t>::max())
    أرجع get_اشاره_ګر_For_کرښه_شمېره_Specialized<uint32_t>(کرښه_No);
  وإلا
    أرجع get_اشاره_ګر_For_کرښه_شمېره_Specialized<uint64_t>(کرښه_No);
}

سرچینه_Mgr::Src_بفر::Src_بفر(سرچینه_Mgr::Src_بفر &&Other)
    : بفر(std::move(Other.بفر)), دننه_واټن_Cache(Other.دننه_واټن_Cache),
      IncludeLoc(Other.IncludeLoc) {
  Other.دننه_واټن_Cache = تش_اشاره;
}

سرچینه_Mgr::Src_بفر::~Src_بفر() {
  إذا (دننه_واټن_Cache) {
    size_t Sz = بفر->get_بفر_اندازه();
    إذا (Sz <= std::numeric_limits<uint8_t>::max())
      ړنګول static_cast<std::ویکټور<uint8_t> *>(دننه_واټن_Cache);
    وإلا إذا (Sz <= std::numeric_limits<uint16_t>::max())
      ړنګول static_cast<std::ویکټور<uint16_t> *>(دننه_واټن_Cache);
    وإلا إذا (Sz <= std::numeric_limits<uint32_t>::max())
      ړنګول static_cast<std::ویکټور<uint32_t> *>(دننه_واټن_Cache);
    وإلا
      ړنګول static_cast<std::ویکټور<uint64_t> *>(دننه_واټن_Cache);
    دننه_واټن_Cache = تش_اشاره;
  }
}

std::pair<بینښه, بینښه>
سرچینه_Mgr::get_کرښه_And_ستنه(SMLoc Loc, بینښه بفر_ID) ثابت {
  إذا (!بفر_ID)
    بفر_ID = پیدا_کړه_بفر_Containing_Loc(Loc);
  أكد(بفر_ID && "Invalid location!");

  خپلکاری &SB = get_بفر_Info(بفر_ID);
  ثابت حرف *Ptr = Loc.get_اشاره_ګر();

  أرجع SB.get_کرښه_And_ستنه(Ptr);
}

// FIXME: Note that the formatting of source locations is spread between
// multiple functions, some in SourceMgr and some in SMDiagnostic. A better
// solution would be a general-purpose source location formatter
// in one of those two classes, or possibly in SMLoc.

/// Get a string with the source location formatted in the standard
/// style, but without the line offset. If \p IncludePath is true, the path
/// is included. If false, only the file name and extension are included.
std::string سرچینه_Mgr::get_Formatted_Location_No_دننه_واټن(SMLoc Loc,
                                                    منطقي Include_لار) ثابت {
  خپلکاری بفر_ID = پیدا_کړه_بفر_Containing_Loc(Loc);
  أكد(بفر_ID && "Invalid location!");
  خپلکاری FileSpec = get_بفر_Info(بفر_ID).بفر->get_بفر_Identifier();

  إذا (Include_لار) {
    أرجع FileSpec.str() + ":" + std::منزل_string(پیدا_کړه_کرښه_شمېره(Loc, بفر_ID));
  } وإلا {
    خپلکاری I = FileSpec.پیدا_کړه_وروستی_د("/\\");
    I = (I == FileSpec.اندازه_2()) ? 0 : (I + 1);
    أرجع FileSpec.substr(I).str() + ":" +
           std::منزل_string(پیدا_کړه_کرښه_شمېره(Loc, بفر_ID));
  }
}

/// Given a line and column number in a mapped buffer, turn it into an SMLoc.
/// This will return a null SMLoc if the line/column location is invalid.
SMLoc سرچینه_Mgr::پیدا_کړه_Loc_For_کرښه_And_ستنه(بینښه بفر_ID, بینښه کرښه_No,
                                         بینښه ستون_No) {
  خپلکاری &SB = get_بفر_Info(بفر_ID);
  ثابت حرف *Ptr = SB.get_اشاره_ګر_For_کرښه_شمېره(کرښه_No);
  إذا (!Ptr)
    أرجع SMLoc();

  // We start counting line and column numbers from 1.
  إذا (ستون_No != 0)
    --ستون_No;

  // If we have a column number, validate it.
  إذا (ستون_No) {
    // Make sure the location is within the current line.
    إذا (Ptr + ستون_No > SB.بفر->get_بفر_پای_ټکی())
      أرجع SMLoc();

    // Make sure there is no newline in the way.
    إذا (StringRef(Ptr, ستون_No).پیدا_کړه_لومړی_د("\n\r") != StringRef::npos)
      أرجع SMLoc();

    Ptr += ستون_No;
  }

  أرجع SMLoc::get_From_اشاره_ګر(Ptr);
}

فراغ سرچینه_Mgr::Print_Include_ډېری(SMLoc IncludeLoc, raw_ostream &OS) ثابت {
  إذا (IncludeLoc == SMLoc())
    أرجع; // Top of stack.

  بینښه CurBuf = پیدا_کړه_بفر_Containing_Loc(IncludeLoc);
  أكد(CurBuf && "Invalid or unspecified location!");

  Print_Include_ډېری(get_بفر_Info(CurBuf).IncludeLoc, OS);

  OS << "Included from " << get_بفر_Info(CurBuf).بفر->get_بفر_Identifier()
     << ":" << پیدا_کړه_کرښه_شمېره(IncludeLoc, CurBuf) << ":\n";
}

فراغ سرچینه_Mgr::print_Include_ډېری_For_Diagnostic(SMLoc Loc,
                                               raw_ostream &OS) ثابت {
  إذا (!Loc.isValid())
    أرجع;
  بینښه Diag_Cur_بفر = پیدا_کړه_بفر_Containing_Loc(Loc);
  إذا (Diag_Cur_بفر && Diag_Cur_بفر != getMainFileID()) {
    SMLoc پلار_Include_Loc = get_پلار_Include_Loc(Diag_Cur_بفر);
    // Ignore macro instantiation buffers to avoid redundant include stacks.
    Print_Include_ډېری(پلار_Include_Loc, OS);
  }
}

SMDiagnostic سرچینه_Mgr::Get_پیغام(SMLoc Loc, سرچینه_Mgr::Diag_ډول ډول_2,
                                   ثابت Twine &پیغام_4, ArrayRef<SMRange> Ranges,
                                   ArrayRef<SMFixIt> FixIts) ثابت {
  // First thing to do: find the current buffer containing the specified
  // location to pull out the source line.
  Small_ویکټور<std::pair<بینښه, بینښه>, 4> ستون_Ranges;
  std::pair<بینښه, بینښه> کرښه_And_ستون;
  StringRef بفر_ID = "<unknown>";
  StringRef کرښه_Str;

  إذا (Loc.isValid()) {
    بینښه CurBuf = پیدا_کړه_بفر_Containing_Loc(Loc);
    أكد(CurBuf && "Invalid or unspecified location!");

    ثابت حافظه_بفر *CurMB = get_حافظه_بفر(CurBuf);
    بفر_ID = CurMB->get_بفر_Identifier();

    // Scan backward to find the start of the line.
    ثابت حرف *کرښه_پیل = Loc.get_اشاره_ګر();
    ثابت حرف *Buf_پیل = CurMB->get_بفر_پیل();
    طالما (کرښه_پیل != Buf_پیل && کرښه_پیل[-1] != '\n' &&
           کرښه_پیل[-1] != '\r')
      --کرښه_پیل;

    // Get the end of the line.
    ثابت حرف *کرښه_پای_ټکی = Loc.get_اشاره_ګر();
    ثابت حرف *Buf_پای_ټکی = CurMB->get_بفر_پای_ټکی();
    طالما (کرښه_پای_ټکی != Buf_پای_ټکی && کرښه_پای_ټکی[0] != '\n' && کرښه_پای_ټکی[0] != '\r')
      ++کرښه_پای_ټکی;
    کرښه_Str = StringRef(کرښه_پیل, کرښه_پای_ټکی - کرښه_پیل);

    // Convert any ranges to column ranges that only intersect the line of the
    // location.
    لكل (SMRange R : Ranges) {
      إذا (!R.isValid())
        استمر;

      // If the line doesn't contain any part of the range, then ignore it.
      إذا (R.پیل.get_اشاره_ګر() > کرښه_پای_ټکی || R.پای_ټکی_2.get_اشاره_ګر() < کرښه_پیل)
        استمر;

      // Ignore pieces of the range that go onto other lines.
      إذا (R.پیل.get_اشاره_ګر() < کرښه_پیل)
        R.پیل = SMLoc::get_From_اشاره_ګر(کرښه_پیل);
      إذا (R.پای_ټکی_2.get_اشاره_ګر() > کرښه_پای_ټکی)
        R.پای_ټکی_2 = SMLoc::get_From_اشاره_ګر(کرښه_پای_ټکی);

      // Translate from SMLoc ranges to column ranges.
      // FIXME: Handle multibyte characters.
      ستون_Ranges.ټېل_شا(std::make_pair(R.پیل.get_اشاره_ګر() - کرښه_پیل,
                                         R.پای_ټکی_2.get_اشاره_ګر() - کرښه_پیل));
    }

    کرښه_And_ستون = get_کرښه_And_ستنه(Loc, CurBuf);
  }

  أرجع SMDiagnostic(*هذا, Loc, بفر_ID, کرښه_And_ستون.لومړی_2,
                      کرښه_And_ستون.دوهم_2 - 1, ډول_2, پیغام_4.str(), کرښه_Str,
                      ستون_Ranges, FixIts);
}

فراغ سرچینه_Mgr::Print_پیغام(raw_ostream &OS, ثابت SMDiagnostic &Diagnostic,
                             منطقي ShowColors) ثابت {
  // Report the message with the diagnostic handler if present.
  إذا (DiagHandler) {
    DiagHandler(Diagnostic, DiagContext);
    أرجع;
  }

  إذا (Diagnostic.getLoc().isValid()) {
    بینښه CurBuf = پیدا_کړه_بفر_Containing_Loc(Diagnostic.getLoc());
    أكد(CurBuf && "Invalid or unspecified location!");
    Print_Include_ډېری(get_بفر_Info(CurBuf).IncludeLoc, OS);
  }

  Diagnostic.print(تش_اشاره, OS, ShowColors);
}

فراغ سرچینه_Mgr::Print_پیغام(raw_ostream &OS, SMLoc Loc,
                             سرچینه_Mgr::Diag_ډول ډول_2, ثابت Twine &پیغام_4,
                             ArrayRef<SMRange> Ranges, ArrayRef<SMFixIt> FixIts,
                             منطقي ShowColors) ثابت {
  Print_پیغام(OS, Get_پیغام(Loc, ډول_2, پیغام_4, Ranges, FixIts), ShowColors);
}

فراغ سرچینه_Mgr::Print_پیغام(SMLoc Loc, سرچینه_Mgr::Diag_ډول ډول_2,
                             ثابت Twine &پیغام_4, ArrayRef<SMRange> Ranges,
                             ArrayRef<SMFixIt> FixIts, منطقي ShowColors) ثابت {
  Print_پیغام(errs(), Loc, ډول_2, پیغام_4, Ranges, FixIts, ShowColors);
}

//===----------------------------------------------------------------------===//
// SMFixIt Implementation
//===----------------------------------------------------------------------===//

SMFixIt::SMFixIt(SMRange R, ثابت Twine &Replacement)
    : Range(R), متن_2(Replacement.str()) {
  أكد(R.isValid());
}

//===----------------------------------------------------------------------===//
// SMDiagnostic Implementation
//===----------------------------------------------------------------------===//

SMDiagnostic::SMDiagnostic(ثابت سرچینه_Mgr &sm, SMLoc L, StringRef FN, صحيح کرښه,
                           صحيح ستون, سرچینه_Mgr::Diag_ډول ډول_2, StringRef پیغام_4,
                           StringRef کرښه_Str,
                           ArrayRef<std::pair<بینښه, بینښه>> Ranges,
                           ArrayRef<SMFixIt> Hints)
    : SM(&sm), Loc(L), Filename(std::string(FN)), کرښه_No(کرښه), ستنه_No(ستون),
      ډول_2(ډول_2), پیغام_3(پیغام_4), کرښه_Contents(کرښه_Str), Ranges(Ranges.vec()),
      FixIts(Hints) {
  llvm::ترتیبول(FixIts);
}

ساكن فراغ build_Fix_It_کرښه(std::string &Caret_کرښه, std::string &Fix_It_کرښه,
                           ArrayRef<SMFixIt> FixIts,
                           ArrayRef<حرف> سرچینه_کرښه) {
  إذا (FixIts.تشه())
    أرجع;

  ثابت حرف *کرښه_پیل = سرچینه_کرښه.begin();
  ثابت حرف *کرښه_پای_ټکی = سرچینه_کرښه.پای_ټکی();

  size_t Prev_Hint_پای_ټکی_ستون = 0;

  لكل (ثابت llvm::SMFixIt &Fixit : FixIts) {
    // If the fixit contains a newline or tab, ignore it.
    إذا (Fixit.get_متن().پیدا_کړه_لومړی_د("\n\r\t") != StringRef::npos)
      استمر;

    SMRange R = Fixit.getRange();

    // If the line doesn't contain any part of the range, then ignore it.
    إذا (R.پیل.get_اشاره_ګر() > کرښه_پای_ټکی || R.پای_ټکی_2.get_اشاره_ګر() < کرښه_پیل)
      استمر;

    // Translate from SMLoc to column.
    // Ignore pieces of the range that go onto other lines.
    // FIXME: Handle multibyte characters in the source line.
    بینښه لومړی_ستون;
    إذا (R.پیل.get_اشاره_ګر() < کرښه_پیل)
      لومړی_ستون = 0;
    وإلا
      لومړی_ستون = R.پیل.get_اشاره_ګر() - کرښه_پیل;

    // If we inserted a long previous hint, push this one forwards, and add
    // an extra space to show that this is not part of the previous
    // completion. This is sort of the best we can do when two hints appear
    // to overlap.
    //
    // Note that if this hint is located immediately after the previous
    // hint, no space will be added, since the location is more important.
    بینښه Hint_ستون = لومړی_ستون;
    إذا (Hint_ستون < Prev_Hint_پای_ټکی_ستون)
      Hint_ستون = Prev_Hint_پای_ټکی_ستون + 1;

    // FIXME: This assertion is intended to catch unintended use of multibyte
    // characters in fixits. If we decide to do this, we'll have to track
    // separate byte widths for the source and fixit lines.
    أكد((size_t)sys::locale::ستنه_پلنوالی(Fixit.get_متن()) ==
           Fixit.get_متن().اندازه_2());

    // This relies on one byte per column in our fixit hints.
    بینښه وروستی_ستنه_Modified = Hint_ستون + Fixit.get_متن().اندازه_2();
    إذا (وروستی_ستنه_Modified > Fix_It_کرښه.اندازه_2())
      Fix_It_کرښه.resize(وروستی_ستنه_Modified, ' ');

    llvm::copy(Fixit.get_متن(), Fix_It_کرښه.begin() + Hint_ستون);

    Prev_Hint_پای_ټکی_ستون = وروستی_ستنه_Modified;

    // For replacements, mark the removal range with '~'.
    // FIXME: Handle multibyte characters in the source line.
    بینښه وروستی_ستون;
    إذا (R.پای_ټکی_2.get_اشاره_ګر() >= کرښه_پای_ټکی)
      وروستی_ستون = کرښه_پای_ټکی - کرښه_پیل;
    وإلا
      وروستی_ستون = R.پای_ټکی_2.get_اشاره_ګر() - کرښه_پیل;

    std::fill(&Caret_کرښه[لومړی_ستون], &Caret_کرښه[وروستی_ستون], '~');
  }
}

ساكن فراغ print_سرچینه_کرښه(raw_ostream &S, StringRef کرښه_Contents) {
  // Print out the source line one character at a time, so we can expand tabs.
  لكل (بینښه i = 0, e = کرښه_Contents.اندازه_2(), بهر_ستون = 0; i != e; ++i) {
    size_t بل_Tab = کرښه_Contents.پیدا_کړه('\t', i);
    // If there were no tabs left, print the rest, we are done.
    إذا (بل_Tab == StringRef::npos) {
      S << کرښه_Contents.drop_front(i);
      اكسر;
    }

    // Otherwise, print from i to NextTab.
    S << کرښه_Contents.ټوټه(i, بل_Tab);
    بهر_ستون += بل_Tab - i;
    i = بل_Tab;

    // If we have a tab, emit at least one space, then round up to 8 columns.
    افعل {
      S << ' ';
      ++بهر_ستون;
    } طالما ((بهر_ستون % TabStop) != 0);
  }
  S << '\n';
}

ساكن منطقي isNonASCII(حرف c) { أرجع c & 0x80; }

فراغ SMDiagnostic::print(ثابت حرف *ProgName, raw_ostream &OS, منطقي ShowColors,
                         منطقي Show_ډول_Label, منطقي ShowLocation) ثابت {
  Color_ډېرپېښ ډېرپېښ = ShowColors ? Color_ډېرپېښ::Auto : Color_ډېرپېښ::Disable;

  {
    WithColor S(OS, raw_ostream::SAVEDCOLOR, ریښتیا, خطأ, ډېرپېښ);

    إذا (ProgName && ProgName[0])
      S << ProgName << ": ";

    إذا (ShowLocation && !Filename.تشه()) {
      إذا (Filename == "-")
        S << "<stdin>";
      وإلا
        S << Filename;

      إذا (کرښه_No != -1) {
        S << ':' << کرښه_No;
        إذا (ستنه_No != -1)
          S << ':' << (ستنه_No + 1);
      }
      S << ": ";
    }
  }

  إذا (Show_ډول_Label) {
    بدل (ډول_2) {
    حالة سرچینه_Mgr::DK_Error:
      WithColor::error(OS, "", !ShowColors);
      اكسر;
    حالة سرچینه_Mgr::DK_Warning:
      WithColor::warning(OS, "", !ShowColors);
      اكسر;
    حالة سرچینه_Mgr::DK_Note:
      WithColor::note(OS, "", !ShowColors);
      اكسر;
    حالة سرچینه_Mgr::DK_Remark:
      WithColor::remark(OS, "", !ShowColors);
      اكسر;
    }
  }

  WithColor(OS, raw_ostream::SAVEDCOLOR, ریښتیا, خطأ, ډېرپېښ) << پیغام_3 << '\n';

  إذا (کرښه_No == -1 || ستنه_No == -1)
    أرجع;

  // FIXME: If there are multibyte or multi-column characters in the source, all
  // our ranges will be wrong. To do this properly, we'll need a byte-to-column
  // map like Clang's TextDiagnostic. For now, we'll just handle tabs by
  // expanding them later, and bail out rather than show incorrect ranges and
  // misaligned fixits for any other odd characters.
  إذا (any_د(کرښه_Contents, isNonASCII)) {
    print_سرچینه_کرښه(OS, کرښه_Contents);
    أرجع;
  }
  size_t عدد_Columns = کرښه_Contents.اندازه_2();

  // Build the line with the caret and ranges.
  std::string Caret_کرښه(عدد_Columns + 1, ' ');

  // Expand any ranges.
  لكل (ثابت std::pair<بینښه, بینښه> &R : Ranges)
    std::fill(&Caret_کرښه[R.لومړی_2],
              &Caret_کرښه[std::min((size_t)R.دوهم_2, Caret_کرښه.اندازه_2())], '~');

  // Add any fix-its.
  // FIXME: Find the beginning of the line properly for multibyte characters.
  std::string Fix_It_Insertion_کرښه;
  build_Fix_It_کرښه(Caret_کرښه, Fix_It_Insertion_کرښه, FixIts,
                 ArrayRef(Loc.get_اشاره_ګر() - ستنه_No, کرښه_Contents.اندازه_2()));

  // Finally, plop on the caret.
  إذا (بینښه(ستنه_No) <= عدد_Columns)
    Caret_کرښه[ستنه_No] = '^';
  وإلا
    Caret_کرښه[عدد_Columns] = '^';

  // ... and remove trailing whitespace so the output doesn't wrap for it.  We
  // know that the line isn't completely empty because it has the caret in it at
  // least.
  Caret_کرښه.erase(Caret_کرښه.پیدا_کړه_وروستی_not_د(' ') + 1);

  print_سرچینه_کرښه(OS, کرښه_Contents);

  {
    Color_ډېرپېښ ډېرپېښ = ShowColors ? Color_ډېرپېښ::Auto : Color_ډېرپېښ::Disable;
    WithColor S(OS, raw_ostream::GREEN, ریښتیا, خطأ, ډېرپېښ);

    // Print out the caret line, matching tabs in the source line.
    لكل (بینښه i = 0, e = Caret_کرښه.اندازه_2(), بهر_ستون = 0; i != e; ++i) {
      إذا (i >= کرښه_Contents.اندازه_2() || کرښه_Contents[i] != '\t') {
        S << Caret_کرښه[i];
        ++بهر_ستون;
        استمر;
      }

      // Okay, we have a tab.  Insert the appropriate number of characters.
      افعل {
        S << Caret_کرښه[i];
        ++بهر_ستون;
      } طالما ((بهر_ستون % TabStop) != 0);
    }
    S << '\n';
  }

  // Print out the replacement line, matching tabs in the source line.
  إذا (Fix_It_Insertion_کرښه.تشه())
    أرجع;

  لكل (size_t i = 0, e = Fix_It_Insertion_کرښه.اندازه_2(), بهر_ستون = 0; i < e; ++i) {
    إذا (i >= کرښه_Contents.اندازه_2() || کرښه_Contents[i] != '\t') {
      OS << Fix_It_Insertion_کرښه[i];
      ++بهر_ستون;
      استمر;
    }

    // Okay, we have a tab.  Insert the appropriate number of characters.
    افعل {
      OS << Fix_It_Insertion_کرښه[i];
      // FIXME: This is trying not to break up replacements, but then to re-sync
      // with the tabs between replacements. This will fail, though, if two
      // fix-it replacements are exactly adjacent, or if a fix-it contains a
      // space. Really we should be precomputing column widths, which we'll
      // need anyway for multibyte chars.
      إذا (Fix_It_Insertion_کرښه[i] != ' ')
        ++i;
      ++بهر_ستون;
    } طالما (((بهر_ستون % TabStop) != 0) && i != e);
  }
  OS << '\n';
}
