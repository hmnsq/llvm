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

استفاده نطاق llvm;

ساكن ثابت size_t TabStop = 8;

// Out of line to avoid needing definition of vfs::FileSystem in header.
منبع_Mgr::منبع_Mgr() = افتراضي;
منبع_Mgr::منبع_Mgr(IntrusiveRefCntPtr<vfs::FileSystem> FS)
    : FS(std::move(FS)) {}
منبع_Mgr::منبع_Mgr(منبع_Mgr &&) = افتراضي;
منبع_Mgr &منبع_Mgr::عملگر=(منبع_Mgr &&) = افتراضي;
منبع_Mgr::~منبع_Mgr() = افتراضي;

IntrusiveRefCntPtr<vfs::FileSystem> منبع_Mgr::getVirtualFileSystem() ثابت {
  أرجع FS;
}

فراغ منبع_Mgr::مجموعه_Virtual_File_System(IntrusiveRefCntPtr<vfs::FileSystem> FS) {
  هذا->FS = std::move(FS);
}

بیعلامت منبع_Mgr::جمع_کن_Include_File(ثابت std::string &Filename,
                                   SMLoc IncludeLoc,
                                   std::string &IncludedFile) {
  ErrorOr<std::unique_ptr<حافظه_میانگیر>> NewBufOrErr =
      OpenIncludeFile(Filename, IncludedFile);
  إذا (!NewBufOrErr)
    أرجع 0;

  أرجع جمع_کن_New_منبع_میانگیر(std::move(*NewBufOrErr), IncludeLoc);
}

ErrorOr<std::unique_ptr<حافظه_میانگیر>>
منبع_Mgr::OpenIncludeFile(ثابت std::string &Filename,
                           std::string &IncludedFile,
                           منطقي RequiresNullTerminator) {
  خودکار GetFile = [هذا, RequiresNullTerminator](StringRef مسیر) {
    أرجع FS ? FS->get_میانگیر_For_File(مسیر, /*FileSize=*/-1,
                                     RequiresNullTerminator)
              : حافظه_میانگیر::getFile(مسیر, /*IsText=*/خطأ,
                                      RequiresNullTerminator);
  };

  ErrorOr<std::unique_ptr<حافظه_میانگیر>> NewBufOrErr = GetFile(Filename);

  SmallString<64> میانگیر(Filename);
  // If the file didn't exist directly, see if it's in an include path.
  لكل (بیعلامت i = 0, e = IncludeDirectories.اندازه_2(); i != e && !NewBufOrErr;
       ++i) {
    میانگیر = IncludeDirectories[i];
    sys::مسیر_2::append(میانگیر, Filename);
    NewBufOrErr = GetFile(میانگیر);
  }

  إذا (NewBufOrErr)
    IncludedFile = static_cast<std::string>(میانگیر);

  أرجع NewBufOrErr;
}

بیعلامت منبع_Mgr::پیدا_کن_میانگیر_Containing_Loc(SMLoc Loc) ثابت {
  لكل (بیعلامت i = 0, e = Buffers.اندازه_2(); i != e; ++i)
    إذا (Loc.get_اشاره_گر() >= Buffers[i].میانگیر->get_میانگیر_شروع() &&
        // Use <= here so that a pointer to the null at the end of the buffer
        // is included as part of the buffer.
        Loc.get_اشاره_گر() <= Buffers[i].میانگیر->get_میانگیر_انتها())
      أرجع i + 1;
  أرجع 0;
}

قالب <نام_نوع T>
ساكن std::وکتور<T> &Get_Or_Create_جابه_جایی_Cache(فراغ *&جابه_جایی_Cache,
                                              حافظه_میانگیر *میانگیر) {
  إذا (جابه_جایی_Cache)
    أرجع *static_cast<std::وکتور<T> *>(جابه_جایی_Cache);

  // Lazily fill in the offset cache.
  خودکار *Offsets = جديد std::وکتور<T>();
  size_t Sz = میانگیر->get_میانگیر_اندازه();
  أكد(Sz <= std::numeric_limits<T>::max());
  StringRef S = میانگیر->get_میانگیر();
  لكل (size_t N = 0; N < Sz; ++N) {
    إذا (S[N] == '\n')
      Offsets->افزودن_پشت(static_cast<T>(N));
  }

  جابه_جایی_Cache = Offsets;
  أرجع *Offsets;
}

قالب <نام_نوع T>
std::pair<بیعلامت, بیعلامت>
منبع_Mgr::Src_میانگیر::get_سطر_And_ستونی_Specialized(ثابت حرف *Ptr) ثابت {
  std::وکتور<T> &Offsets =
      Get_Or_Create_جابه_جایی_Cache<T>(جابه_جایی_Cache, میانگیر.get());

  ثابت حرف *Buf_شروع = میانگیر->get_میانگیر_شروع();
  أكد(Ptr >= Buf_شروع && Ptr <= میانگیر->get_میانگیر_انتها());
  ptrdiff_t PtrDiff = Ptr - Buf_شروع;
  أكد(PtrDiff >= 0 &&
         static_cast<size_t>(PtrDiff) <= std::numeric_limits<T>::max());
  T Ptr_جابه_جایی = static_cast<T>(PtrDiff);

  // llvm::lower_bound gives the number of EOL before PtrOffset. Add 1 to get
  // the line number.
  خودکار I = llvm::پایینی_bound(Offsets, Ptr_جابه_جایی);
  بیعلامت سطر_No = I - Offsets.begin() + 1;

  // The column number is the distance from the previous EOL (or the start of
  // the buffer) to the pointer.
  T سطر_شروع_Offs = (I == Offsets.begin()) ? 0 : (I[-1] + 1);
  بیعلامت ستون_No = Ptr_جابه_جایی - سطر_شروع_Offs + 1;
  أرجع {سطر_No, ستون_No};
}

/// Look up a given \p Ptr in the buffer, determining which line and column
/// it came from.
LLVM_ABI std::pair<بیعلامت, بیعلامت>
منبع_Mgr::Src_میانگیر::get_سطر_And_ستونی(ثابت حرف *Ptr) ثابت {
  size_t Sz = میانگیر->get_میانگیر_اندازه();
  إذا (Sz <= std::numeric_limits<uint8_t>::max())
    أرجع get_سطر_And_ستونی_Specialized<uint8_t>(Ptr);
  إذا (Sz <= std::numeric_limits<uint16_t>::max())
    أرجع get_سطر_And_ستونی_Specialized<uint16_t>(Ptr);
  إذا (Sz <= std::numeric_limits<uint32_t>::max())
    أرجع get_سطر_And_ستونی_Specialized<uint32_t>(Ptr);
  أرجع get_سطر_And_ستونی_Specialized<uint64_t>(Ptr);
}

قالب <نام_نوع T>
ثابت حرف *منبع_Mgr::Src_میانگیر::get_اشاره_گر_For_سطر_عدد_Specialized(
    بیعلامت سطر_No) ثابت {
  std::وکتور<T> &Offsets =
      Get_Or_Create_جابه_جایی_Cache<T>(جابه_جایی_Cache, میانگیر.get());

  // We start counting line and column numbers from 1.
  إذا (سطر_No != 0)
    --سطر_No;

  ثابت حرف *Buf_شروع = میانگیر->get_میانگیر_شروع();

  // The offset cache contains the location of the \n for the specified line,
  // we want the start of the line.  As such, we look for the previous entry.
  إذا (سطر_No == 0)
    أرجع Buf_شروع;
  إذا (سطر_No > Offsets.اندازه_2())
    أرجع اشاره_تهی;
  أرجع Buf_شروع + Offsets[سطر_No - 1] + 1;
}

/// Return a pointer to the first character of the specified line number or
/// null if the line number is invalid.
ثابت حرف *
منبع_Mgr::Src_میانگیر::get_اشاره_گر_For_سطر_عدد(بیعلامت سطر_No) ثابت {
  size_t Sz = میانگیر->get_میانگیر_اندازه();
  إذا (Sz <= std::numeric_limits<uint8_t>::max())
    أرجع get_اشاره_گر_For_سطر_عدد_Specialized<uint8_t>(سطر_No);
  وإلا إذا (Sz <= std::numeric_limits<uint16_t>::max())
    أرجع get_اشاره_گر_For_سطر_عدد_Specialized<uint16_t>(سطر_No);
  وإلا إذا (Sz <= std::numeric_limits<uint32_t>::max())
    أرجع get_اشاره_گر_For_سطر_عدد_Specialized<uint32_t>(سطر_No);
  وإلا
    أرجع get_اشاره_گر_For_سطر_عدد_Specialized<uint64_t>(سطر_No);
}

منبع_Mgr::Src_میانگیر::Src_میانگیر(منبع_Mgr::Src_میانگیر &&Other)
    : میانگیر(std::move(Other.میانگیر)), جابه_جایی_Cache(Other.جابه_جایی_Cache),
      IncludeLoc(Other.IncludeLoc) {
  Other.جابه_جایی_Cache = اشاره_تهی;
}

منبع_Mgr::Src_میانگیر::~Src_میانگیر() {
  إذا (جابه_جایی_Cache) {
    size_t Sz = میانگیر->get_میانگیر_اندازه();
    إذا (Sz <= std::numeric_limits<uint8_t>::max())
      حذف static_cast<std::وکتور<uint8_t> *>(جابه_جایی_Cache);
    وإلا إذا (Sz <= std::numeric_limits<uint16_t>::max())
      حذف static_cast<std::وکتور<uint16_t> *>(جابه_جایی_Cache);
    وإلا إذا (Sz <= std::numeric_limits<uint32_t>::max())
      حذف static_cast<std::وکتور<uint32_t> *>(جابه_جایی_Cache);
    وإلا
      حذف static_cast<std::وکتور<uint64_t> *>(جابه_جایی_Cache);
    جابه_جایی_Cache = اشاره_تهی;
  }
}

std::pair<بیعلامت, بیعلامت>
منبع_Mgr::get_سطر_And_ستونی(SMLoc Loc, بیعلامت میانگیر_ID) ثابت {
  إذا (!میانگیر_ID)
    میانگیر_ID = پیدا_کن_میانگیر_Containing_Loc(Loc);
  أكد(میانگیر_ID && "Invalid location!");

  خودکار &SB = get_میانگیر_Info(میانگیر_ID);
  ثابت حرف *Ptr = Loc.get_اشاره_گر();

  أرجع SB.get_سطر_And_ستونی(Ptr);
}

// FIXME: Note that the formatting of source locations is spread between
// multiple functions, some in SourceMgr and some in SMDiagnostic. A better
// solution would be a general-purpose source location formatter
// in one of those two classes, or possibly in SMLoc.

/// Get a string with the source location formatted in the standard
/// style, but without the line offset. If \p IncludePath is true, the path
/// is included. If false, only the file name and extension are included.
std::string منبع_Mgr::get_Formatted_Location_No_جابه_جایی(SMLoc Loc,
                                                    منطقي Include_مسیر) ثابت {
  خودکار میانگیر_ID = پیدا_کن_میانگیر_Containing_Loc(Loc);
  أكد(میانگیر_ID && "Invalid location!");
  خودکار FileSpec = get_میانگیر_Info(میانگیر_ID).میانگیر->get_میانگیر_Identifier();

  إذا (Include_مسیر) {
    أرجع FileSpec.str() + ":" + std::مقصد_string(پیدا_کن_سطر_عدد(Loc, میانگیر_ID));
  } وإلا {
    خودکار I = FileSpec.پیدا_کن_آخرین_از("/\\");
    I = (I == FileSpec.اندازه_2()) ? 0 : (I + 1);
    أرجع FileSpec.substr(I).str() + ":" +
           std::مقصد_string(پیدا_کن_سطر_عدد(Loc, میانگیر_ID));
  }
}

/// Given a line and column number in a mapped buffer, turn it into an SMLoc.
/// This will return a null SMLoc if the line/column location is invalid.
SMLoc منبع_Mgr::پیدا_کن_Loc_For_سطر_And_ستونی(بیعلامت میانگیر_ID, بیعلامت سطر_No,
                                         بیعلامت ستون_No) {
  خودکار &SB = get_میانگیر_Info(میانگیر_ID);
  ثابت حرف *Ptr = SB.get_اشاره_گر_For_سطر_عدد(سطر_No);
  إذا (!Ptr)
    أرجع SMLoc();

  // We start counting line and column numbers from 1.
  إذا (ستون_No != 0)
    --ستون_No;

  // If we have a column number, validate it.
  إذا (ستون_No) {
    // Make sure the location is within the current line.
    إذا (Ptr + ستون_No > SB.میانگیر->get_میانگیر_انتها())
      أرجع SMLoc();

    // Make sure there is no newline in the way.
    إذا (StringRef(Ptr, ستون_No).پیدا_کن_اولی_از("\n\r") != StringRef::npos)
      أرجع SMLoc();

    Ptr += ستون_No;
  }

  أرجع SMLoc::get_From_اشاره_گر(Ptr);
}

فراغ منبع_Mgr::Print_Include_پشته(SMLoc IncludeLoc, raw_ostream &OS) ثابت {
  إذا (IncludeLoc == SMLoc())
    أرجع; // Top of stack.

  بیعلامت CurBuf = پیدا_کن_میانگیر_Containing_Loc(IncludeLoc);
  أكد(CurBuf && "Invalid or unspecified location!");

  Print_Include_پشته(get_میانگیر_Info(CurBuf).IncludeLoc, OS);

  OS << "Included from " << get_میانگیر_Info(CurBuf).میانگیر->get_میانگیر_Identifier()
     << ":" << پیدا_کن_سطر_عدد(IncludeLoc, CurBuf) << ":\n";
}

فراغ منبع_Mgr::print_Include_پشته_For_Diagnostic(SMLoc Loc,
                                               raw_ostream &OS) ثابت {
  إذا (!Loc.isValid())
    أرجع;
  بیعلامت Diag_Cur_میانگیر = پیدا_کن_میانگیر_Containing_Loc(Loc);
  إذا (Diag_Cur_میانگیر && Diag_Cur_میانگیر != getMainFileID()) {
    SMLoc والد_Include_Loc = get_والد_Include_Loc(Diag_Cur_میانگیر);
    // Ignore macro instantiation buffers to avoid redundant include stacks.
    Print_Include_پشته(والد_Include_Loc, OS);
  }
}

SMDiagnostic منبع_Mgr::Get_پیام(SMLoc Loc, منبع_Mgr::Diag_نوع نوع_3,
                                   ثابت Twine &پیام_4, ArrayRef<SMRange> Ranges,
                                   ArrayRef<SMFixIt> FixIts) ثابت {
  // First thing to do: find the current buffer containing the specified
  // location to pull out the source line.
  Small_وکتور<std::pair<بیعلامت, بیعلامت>, 4> ستون_Ranges;
  std::pair<بیعلامت, بیعلامت> سطر_And_ستون;
  StringRef میانگیر_ID = "<unknown>";
  StringRef سطر_Str;

  إذا (Loc.isValid()) {
    بیعلامت CurBuf = پیدا_کن_میانگیر_Containing_Loc(Loc);
    أكد(CurBuf && "Invalid or unspecified location!");

    ثابت حافظه_میانگیر *CurMB = get_حافظه_میانگیر(CurBuf);
    میانگیر_ID = CurMB->get_میانگیر_Identifier();

    // Scan backward to find the start of the line.
    ثابت حرف *سطر_شروع = Loc.get_اشاره_گر();
    ثابت حرف *Buf_شروع = CurMB->get_میانگیر_شروع();
    طالما (سطر_شروع != Buf_شروع && سطر_شروع[-1] != '\n' &&
           سطر_شروع[-1] != '\r')
      --سطر_شروع;

    // Get the end of the line.
    ثابت حرف *سطر_انتها = Loc.get_اشاره_گر();
    ثابت حرف *Buf_انتها = CurMB->get_میانگیر_انتها();
    طالما (سطر_انتها != Buf_انتها && سطر_انتها[0] != '\n' && سطر_انتها[0] != '\r')
      ++سطر_انتها;
    سطر_Str = StringRef(سطر_شروع, سطر_انتها - سطر_شروع);

    // Convert any ranges to column ranges that only intersect the line of the
    // location.
    لكل (SMRange R : Ranges) {
      إذا (!R.isValid())
        استمر;

      // If the line doesn't contain any part of the range, then ignore it.
      إذا (R.شروع.get_اشاره_گر() > سطر_انتها || R.انتها_2.get_اشاره_گر() < سطر_شروع)
        استمر;

      // Ignore pieces of the range that go onto other lines.
      إذا (R.شروع.get_اشاره_گر() < سطر_شروع)
        R.شروع = SMLoc::get_From_اشاره_گر(سطر_شروع);
      إذا (R.انتها_2.get_اشاره_گر() > سطر_انتها)
        R.انتها_2 = SMLoc::get_From_اشاره_گر(سطر_انتها);

      // Translate from SMLoc ranges to column ranges.
      // FIXME: Handle multibyte characters.
      ستون_Ranges.افزودن_پشت(std::make_pair(R.شروع.get_اشاره_گر() - سطر_شروع,
                                         R.انتها_2.get_اشاره_گر() - سطر_شروع));
    }

    سطر_And_ستون = get_سطر_And_ستونی(Loc, CurBuf);
  }

  أرجع SMDiagnostic(*هذا, Loc, میانگیر_ID, سطر_And_ستون.اولی_2,
                      سطر_And_ستون.دومی_2 - 1, نوع_3, پیام_4.str(), سطر_Str,
                      ستون_Ranges, FixIts);
}

فراغ منبع_Mgr::Print_پیام(raw_ostream &OS, ثابت SMDiagnostic &Diagnostic,
                             منطقي ShowColors) ثابت {
  // Report the message with the diagnostic handler if present.
  إذا (DiagHandler) {
    DiagHandler(Diagnostic, DiagContext);
    أرجع;
  }

  إذا (Diagnostic.getLoc().isValid()) {
    بیعلامت CurBuf = پیدا_کن_میانگیر_Containing_Loc(Diagnostic.getLoc());
    أكد(CurBuf && "Invalid or unspecified location!");
    Print_Include_پشته(get_میانگیر_Info(CurBuf).IncludeLoc, OS);
  }

  Diagnostic.print(اشاره_تهی, OS, ShowColors);
}

فراغ منبع_Mgr::Print_پیام(raw_ostream &OS, SMLoc Loc,
                             منبع_Mgr::Diag_نوع نوع_3, ثابت Twine &پیام_4,
                             ArrayRef<SMRange> Ranges, ArrayRef<SMFixIt> FixIts,
                             منطقي ShowColors) ثابت {
  Print_پیام(OS, Get_پیام(Loc, نوع_3, پیام_4, Ranges, FixIts), ShowColors);
}

فراغ منبع_Mgr::Print_پیام(SMLoc Loc, منبع_Mgr::Diag_نوع نوع_3,
                             ثابت Twine &پیام_4, ArrayRef<SMRange> Ranges,
                             ArrayRef<SMFixIt> FixIts, منطقي ShowColors) ثابت {
  Print_پیام(errs(), Loc, نوع_3, پیام_4, Ranges, FixIts, ShowColors);
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

SMDiagnostic::SMDiagnostic(ثابت منبع_Mgr &sm, SMLoc L, StringRef FN, صحيح سطر,
                           صحيح ستون, منبع_Mgr::Diag_نوع نوع_3, StringRef پیام_4,
                           StringRef سطر_Str,
                           ArrayRef<std::pair<بیعلامت, بیعلامت>> Ranges,
                           ArrayRef<SMFixIt> Hints)
    : SM(&sm), Loc(L), Filename(std::string(FN)), سطر_No(سطر), ستونی_No(ستون),
      نوع_3(نوع_3), پیام_3(پیام_4), سطر_Contents(سطر_Str), Ranges(Ranges.vec()),
      FixIts(Hints) {
  llvm::مرتب_سازی(FixIts);
}

ساكن فراغ build_Fix_It_سطر(std::string &Caret_سطر, std::string &Fix_It_سطر,
                           ArrayRef<SMFixIt> FixIts,
                           ArrayRef<حرف> منبع_سطر) {
  إذا (FixIts.خالی())
    أرجع;

  ثابت حرف *سطر_شروع = منبع_سطر.begin();
  ثابت حرف *سطر_انتها = منبع_سطر.انتها();

  size_t Prev_Hint_انتها_ستون = 0;

  لكل (ثابت llvm::SMFixIt &Fixit : FixIts) {
    // If the fixit contains a newline or tab, ignore it.
    إذا (Fixit.get_متن().پیدا_کن_اولی_از("\n\r\t") != StringRef::npos)
      استمر;

    SMRange R = Fixit.getRange();

    // If the line doesn't contain any part of the range, then ignore it.
    إذا (R.شروع.get_اشاره_گر() > سطر_انتها || R.انتها_2.get_اشاره_گر() < سطر_شروع)
      استمر;

    // Translate from SMLoc to column.
    // Ignore pieces of the range that go onto other lines.
    // FIXME: Handle multibyte characters in the source line.
    بیعلامت اولی_ستون;
    إذا (R.شروع.get_اشاره_گر() < سطر_شروع)
      اولی_ستون = 0;
    وإلا
      اولی_ستون = R.شروع.get_اشاره_گر() - سطر_شروع;

    // If we inserted a long previous hint, push this one forwards, and add
    // an extra space to show that this is not part of the previous
    // completion. This is sort of the best we can do when two hints appear
    // to overlap.
    //
    // Note that if this hint is located immediately after the previous
    // hint, no space will be added, since the location is more important.
    بیعلامت Hint_ستون = اولی_ستون;
    إذا (Hint_ستون < Prev_Hint_انتها_ستون)
      Hint_ستون = Prev_Hint_انتها_ستون + 1;

    // FIXME: This assertion is intended to catch unintended use of multibyte
    // characters in fixits. If we decide to do this, we'll have to track
    // separate byte widths for the source and fixit lines.
    أكد((size_t)sys::locale::ستونی_عرض(Fixit.get_متن()) ==
           Fixit.get_متن().اندازه_2());

    // This relies on one byte per column in our fixit hints.
    بیعلامت آخرین_ستونی_Modified = Hint_ستون + Fixit.get_متن().اندازه_2();
    إذا (آخرین_ستونی_Modified > Fix_It_سطر.اندازه_2())
      Fix_It_سطر.resize(آخرین_ستونی_Modified, ' ');

    llvm::copy(Fixit.get_متن(), Fix_It_سطر.begin() + Hint_ستون);

    Prev_Hint_انتها_ستون = آخرین_ستونی_Modified;

    // For replacements, mark the removal range with '~'.
    // FIXME: Handle multibyte characters in the source line.
    بیعلامت آخرین_ستون;
    إذا (R.انتها_2.get_اشاره_گر() >= سطر_انتها)
      آخرین_ستون = سطر_انتها - سطر_شروع;
    وإلا
      آخرین_ستون = R.انتها_2.get_اشاره_گر() - سطر_شروع;

    std::fill(&Caret_سطر[اولی_ستون], &Caret_سطر[آخرین_ستون], '~');
  }
}

ساكن فراغ print_منبع_سطر(raw_ostream &S, StringRef سطر_Contents) {
  // Print out the source line one character at a time, so we can expand tabs.
  لكل (بیعلامت i = 0, e = سطر_Contents.اندازه_2(), بیرون_ستون = 0; i != e; ++i) {
    size_t بعدی_Tab = سطر_Contents.پیدا_کن('\t', i);
    // If there were no tabs left, print the rest, we are done.
    إذا (بعدی_Tab == StringRef::npos) {
      S << سطر_Contents.drop_front(i);
      اكسر;
    }

    // Otherwise, print from i to NextTab.
    S << سطر_Contents.برش(i, بعدی_Tab);
    بیرون_ستون += بعدی_Tab - i;
    i = بعدی_Tab;

    // If we have a tab, emit at least one space, then round up to 8 columns.
    افعل {
      S << ' ';
      ++بیرون_ستون;
    } طالما ((بیرون_ستون % TabStop) != 0);
  }
  S << '\n';
}

ساكن منطقي isNonASCII(حرف c) { أرجع c & 0x80; }

فراغ SMDiagnostic::print(ثابت حرف *ProgName, raw_ostream &OS, منطقي ShowColors,
                         منطقي Show_نوع_Label, منطقي ShowLocation) ثابت {
  Color_نما نما = ShowColors ? Color_نما::Auto : Color_نما::Disable;

  {
    WithColor S(OS, raw_ostream::SAVEDCOLOR, درست, خطأ, نما);

    إذا (ProgName && ProgName[0])
      S << ProgName << ": ";

    إذا (ShowLocation && !Filename.خالی()) {
      إذا (Filename == "-")
        S << "<stdin>";
      وإلا
        S << Filename;

      إذا (سطر_No != -1) {
        S << ':' << سطر_No;
        إذا (ستونی_No != -1)
          S << ':' << (ستونی_No + 1);
      }
      S << ": ";
    }
  }

  إذا (Show_نوع_Label) {
    بدل (نوع_3) {
    حالة منبع_Mgr::DK_Error:
      WithColor::error(OS, "", !ShowColors);
      اكسر;
    حالة منبع_Mgr::DK_Warning:
      WithColor::warning(OS, "", !ShowColors);
      اكسر;
    حالة منبع_Mgr::DK_Note:
      WithColor::note(OS, "", !ShowColors);
      اكسر;
    حالة منبع_Mgr::DK_Remark:
      WithColor::remark(OS, "", !ShowColors);
      اكسر;
    }
  }

  WithColor(OS, raw_ostream::SAVEDCOLOR, درست, خطأ, نما) << پیام_3 << '\n';

  إذا (سطر_No == -1 || ستونی_No == -1)
    أرجع;

  // FIXME: If there are multibyte or multi-column characters in the source, all
  // our ranges will be wrong. To do this properly, we'll need a byte-to-column
  // map like Clang's TextDiagnostic. For now, we'll just handle tabs by
  // expanding them later, and bail out rather than show incorrect ranges and
  // misaligned fixits for any other odd characters.
  إذا (any_از(سطر_Contents, isNonASCII)) {
    print_منبع_سطر(OS, سطر_Contents);
    أرجع;
  }
  size_t رقم_Columns = سطر_Contents.اندازه_2();

  // Build the line with the caret and ranges.
  std::string Caret_سطر(رقم_Columns + 1, ' ');

  // Expand any ranges.
  لكل (ثابت std::pair<بیعلامت, بیعلامت> &R : Ranges)
    std::fill(&Caret_سطر[R.اولی_2],
              &Caret_سطر[std::min((size_t)R.دومی_2, Caret_سطر.اندازه_2())], '~');

  // Add any fix-its.
  // FIXME: Find the beginning of the line properly for multibyte characters.
  std::string Fix_It_Insertion_سطر;
  build_Fix_It_سطر(Caret_سطر, Fix_It_Insertion_سطر, FixIts,
                 ArrayRef(Loc.get_اشاره_گر() - ستونی_No, سطر_Contents.اندازه_2()));

  // Finally, plop on the caret.
  إذا (بیعلامت(ستونی_No) <= رقم_Columns)
    Caret_سطر[ستونی_No] = '^';
  وإلا
    Caret_سطر[رقم_Columns] = '^';

  // ... and remove trailing whitespace so the output doesn't wrap for it.  We
  // know that the line isn't completely empty because it has the caret in it at
  // least.
  Caret_سطر.erase(Caret_سطر.پیدا_کن_آخرین_not_از(' ') + 1);

  print_منبع_سطر(OS, سطر_Contents);

  {
    Color_نما نما = ShowColors ? Color_نما::Auto : Color_نما::Disable;
    WithColor S(OS, raw_ostream::GREEN, درست, خطأ, نما);

    // Print out the caret line, matching tabs in the source line.
    لكل (بیعلامت i = 0, e = Caret_سطر.اندازه_2(), بیرون_ستون = 0; i != e; ++i) {
      إذا (i >= سطر_Contents.اندازه_2() || سطر_Contents[i] != '\t') {
        S << Caret_سطر[i];
        ++بیرون_ستون;
        استمر;
      }

      // Okay, we have a tab.  Insert the appropriate number of characters.
      افعل {
        S << Caret_سطر[i];
        ++بیرون_ستون;
      } طالما ((بیرون_ستون % TabStop) != 0);
    }
    S << '\n';
  }

  // Print out the replacement line, matching tabs in the source line.
  إذا (Fix_It_Insertion_سطر.خالی())
    أرجع;

  لكل (size_t i = 0, e = Fix_It_Insertion_سطر.اندازه_2(), بیرون_ستون = 0; i < e; ++i) {
    إذا (i >= سطر_Contents.اندازه_2() || سطر_Contents[i] != '\t') {
      OS << Fix_It_Insertion_سطر[i];
      ++بیرون_ستون;
      استمر;
    }

    // Okay, we have a tab.  Insert the appropriate number of characters.
    افعل {
      OS << Fix_It_Insertion_سطر[i];
      // FIXME: This is trying not to break up replacements, but then to re-sync
      // with the tabs between replacements. This will fail, though, if two
      // fix-it replacements are exactly adjacent, or if a fix-it contains a
      // space. Really we should be precomputing column widths, which we'll
      // need anyway for multibyte chars.
      إذا (Fix_It_Insertion_سطر[i] != ' ')
        ++i;
      ++بیرون_ستون;
    } طالما (((بیرون_ستون % TabStop) != 0) && i != e);
  }
  OS << '\n';
}
