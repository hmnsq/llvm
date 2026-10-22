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

استعمال نامخلا llvm;

ساکن ثابت size_t TabStop = 8;

// Out of line to avoid needing definition of vfs::FileSystem in header.
ماخذ_Mgr::ماخذ_Mgr() = طےشدہ;
ماخذ_Mgr::ماخذ_Mgr(IntrusiveRefCntPtr<vfs::FileSystem> FS)
    : FS(std::move(FS)) {}
ماخذ_Mgr::ماخذ_Mgr(ماخذ_Mgr &&) = طےشدہ;
ماخذ_Mgr &ماخذ_Mgr::عامل=(ماخذ_Mgr &&) = طےشدہ;
ماخذ_Mgr::~ماخذ_Mgr() = طےشدہ;

IntrusiveRefCntPtr<vfs::FileSystem> ماخذ_Mgr::getVirtualFileSystem() ثابت {
  واپس FS;
}

خالی ماخذ_Mgr::سیٹ_Virtual_File_System(IntrusiveRefCntPtr<vfs::FileSystem> FS) {
  یہ->FS = std::move(FS);
}

غیرعلامتی ماخذ_Mgr::جوڑو_Include_File(ثابت std::string &Filename,
                                   SMLoc IncludeLoc,
                                   std::string &IncludedFile) {
  ErrorOr<std::unique_ptr<حافظہ_بفر>> NewBufOrErr =
      OpenIncludeFile(Filename, IncludedFile);
  اگر (!NewBufOrErr)
    واپس 0;

  واپس جوڑو_New_ماخذ_بفر(std::move(*NewBufOrErr), IncludeLoc);
}

ErrorOr<std::unique_ptr<حافظہ_بفر>>
ماخذ_Mgr::OpenIncludeFile(ثابت std::string &Filename,
                           std::string &IncludedFile,
                           منطقی RequiresNullTerminator) {
  خودکار GetFile = [یہ, RequiresNullTerminator](StringRef راستہ) {
    واپس FS ? FS->get_بفر_For_File(راستہ, /*FileSize=*/-1,
                                     RequiresNullTerminator)
              : حافظہ_بفر::getFile(راستہ, /*IsText=*/جھوٹ,
                                      RequiresNullTerminator);
  };

  ErrorOr<std::unique_ptr<حافظہ_بفر>> NewBufOrErr = GetFile(Filename);

  SmallString<64> بفر(Filename);
  // If the file didn't exist directly, see if it's in an include path.
  برائے (غیرعلامتی i = 0, e = IncludeDirectories.سائز_2(); i != e && !NewBufOrErr;
       ++i) {
    بفر = IncludeDirectories[i];
    sys::راستہ_2::append(بفر, Filename);
    NewBufOrErr = GetFile(بفر);
  }

  اگر (NewBufOrErr)
    IncludedFile = static_cast<std::string>(بفر);

  واپس NewBufOrErr;
}

غیرعلامتی ماخذ_Mgr::ڈھونڈو_بفر_Containing_Loc(SMLoc Loc) ثابت {
  برائے (غیرعلامتی i = 0, e = Buffers.سائز_2(); i != e; ++i)
    اگر (Loc.get_اشارہ_گر() >= Buffers[i].بفر->get_بفر_آغاز() &&
        // Use <= here so that a pointer to the null at the end of the buffer
        // is included as part of the buffer.
        Loc.get_اشارہ_گر() <= Buffers[i].بفر->get_بفر_انتہا())
      واپس i + 1;
  واپس 0;
}

سانچہ <قسم_نام T>
ساکن std::سمتیہ<T> &Get_Or_Create_انحراف_Cache(خالی *&انحراف_Cache,
                                              حافظہ_بفر *بفر) {
  اگر (انحراف_Cache)
    واپس *static_cast<std::سمتیہ<T> *>(انحراف_Cache);

  // Lazily fill in the offset cache.
  خودکار *Offsets = نیا std::سمتیہ<T>();
  size_t Sz = بفر->get_بفر_سائز();
  أكد(Sz <= std::numeric_limits<T>::max());
  StringRef S = بفر->get_بفر();
  برائے (size_t N = 0; N < Sz; ++N) {
    اگر (S[N] == '\n')
      Offsets->ڈالو_عقب(static_cast<T>(N));
  }

  انحراف_Cache = Offsets;
  واپس *Offsets;
}

سانچہ <قسم_نام T>
std::pair<غیرعلامتی, غیرعلامتی>
ماخذ_Mgr::Src_بفر::get_سطر_And_عمود_Specialized(ثابت حرف *Ptr) ثابت {
  std::سمتیہ<T> &Offsets =
      Get_Or_Create_انحراف_Cache<T>(انحراف_Cache, بفر.get());

  ثابت حرف *Buf_آغاز = بفر->get_بفر_آغاز();
  أكد(Ptr >= Buf_آغاز && Ptr <= بفر->get_بفر_انتہا());
  ptrdiff_t PtrDiff = Ptr - Buf_آغاز;
  أكد(PtrDiff >= 0 &&
         static_cast<size_t>(PtrDiff) <= std::numeric_limits<T>::max());
  T Ptr_انحراف = static_cast<T>(PtrDiff);

  // llvm::lower_bound gives the number of EOL before PtrOffset. Add 1 to get
  // the line number.
  خودکار I = llvm::زیریں_bound(Offsets, Ptr_انحراف);
  غیرعلامتی سطر_No = I - Offsets.begin() + 1;

  // The column number is the distance from the previous EOL (or the start of
  // the buffer) to the pointer.
  T سطر_آغاز_Offs = (I == Offsets.begin()) ? 0 : (I[-1] + 1);
  غیرعلامتی ستون_No = Ptr_انحراف - سطر_آغاز_Offs + 1;
  واپس {سطر_No, ستون_No};
}

/// Look up a given \p Ptr in the buffer, determining which line and column
/// it came from.
LLVM_ABI std::pair<غیرعلامتی, غیرعلامتی>
ماخذ_Mgr::Src_بفر::get_سطر_And_عمود(ثابت حرف *Ptr) ثابت {
  size_t Sz = بفر->get_بفر_سائز();
  اگر (Sz <= std::numeric_limits<uint8_t>::max())
    واپس get_سطر_And_عمود_Specialized<uint8_t>(Ptr);
  اگر (Sz <= std::numeric_limits<uint16_t>::max())
    واپس get_سطر_And_عمود_Specialized<uint16_t>(Ptr);
  اگر (Sz <= std::numeric_limits<uint32_t>::max())
    واپس get_سطر_And_عمود_Specialized<uint32_t>(Ptr);
  واپس get_سطر_And_عمود_Specialized<uint64_t>(Ptr);
}

سانچہ <قسم_نام T>
ثابت حرف *ماخذ_Mgr::Src_بفر::get_اشارہ_گر_For_سطر_نمبر_Specialized(
    غیرعلامتی سطر_No) ثابت {
  std::سمتیہ<T> &Offsets =
      Get_Or_Create_انحراف_Cache<T>(انحراف_Cache, بفر.get());

  // We start counting line and column numbers from 1.
  اگر (سطر_No != 0)
    --سطر_No;

  ثابت حرف *Buf_آغاز = بفر->get_بفر_آغاز();

  // The offset cache contains the location of the \n for the specified line,
  // we want the start of the line.  As such, we look for the previous entry.
  اگر (سطر_No == 0)
    واپس Buf_آغاز;
  اگر (سطر_No > Offsets.سائز_2())
    واپس خالی_اشاریہ;
  واپس Buf_آغاز + Offsets[سطر_No - 1] + 1;
}

/// Return a pointer to the first character of the specified line number or
/// null if the line number is invalid.
ثابت حرف *
ماخذ_Mgr::Src_بفر::get_اشارہ_گر_For_سطر_نمبر(غیرعلامتی سطر_No) ثابت {
  size_t Sz = بفر->get_بفر_سائز();
  اگر (Sz <= std::numeric_limits<uint8_t>::max())
    واپس get_اشارہ_گر_For_سطر_نمبر_Specialized<uint8_t>(سطر_No);
  ورنہ اگر (Sz <= std::numeric_limits<uint16_t>::max())
    واپس get_اشارہ_گر_For_سطر_نمبر_Specialized<uint16_t>(سطر_No);
  ورنہ اگر (Sz <= std::numeric_limits<uint32_t>::max())
    واپس get_اشارہ_گر_For_سطر_نمبر_Specialized<uint32_t>(سطر_No);
  ورنہ
    واپس get_اشارہ_گر_For_سطر_نمبر_Specialized<uint64_t>(سطر_No);
}

ماخذ_Mgr::Src_بفر::Src_بفر(ماخذ_Mgr::Src_بفر &&Other)
    : بفر(std::move(Other.بفر)), انحراف_Cache(Other.انحراف_Cache),
      IncludeLoc(Other.IncludeLoc) {
  Other.انحراف_Cache = خالی_اشاریہ;
}

ماخذ_Mgr::Src_بفر::~Src_بفر() {
  اگر (انحراف_Cache) {
    size_t Sz = بفر->get_بفر_سائز();
    اگر (Sz <= std::numeric_limits<uint8_t>::max())
      حذف static_cast<std::سمتیہ<uint8_t> *>(انحراف_Cache);
    ورنہ اگر (Sz <= std::numeric_limits<uint16_t>::max())
      حذف static_cast<std::سمتیہ<uint16_t> *>(انحراف_Cache);
    ورنہ اگر (Sz <= std::numeric_limits<uint32_t>::max())
      حذف static_cast<std::سمتیہ<uint32_t> *>(انحراف_Cache);
    ورنہ
      حذف static_cast<std::سمتیہ<uint64_t> *>(انحراف_Cache);
    انحراف_Cache = خالی_اشاریہ;
  }
}

std::pair<غیرعلامتی, غیرعلامتی>
ماخذ_Mgr::get_سطر_And_عمود(SMLoc Loc, غیرعلامتی بفر_ID) ثابت {
  اگر (!بفر_ID)
    بفر_ID = ڈھونڈو_بفر_Containing_Loc(Loc);
  أكد(بفر_ID && "Invalid location!");

  خودکار &SB = get_بفر_Info(بفر_ID);
  ثابت حرف *Ptr = Loc.get_اشارہ_گر();

  واپس SB.get_سطر_And_عمود(Ptr);
}

// FIXME: Note that the formatting of source locations is spread between
// multiple functions, some in SourceMgr and some in SMDiagnostic. A better
// solution would be a general-purpose source location formatter
// in one of those two classes, or possibly in SMLoc.

/// Get a string with the source location formatted in the standard
/// style, but without the line offset. If \p IncludePath is true, the path
/// is included. If false, only the file name and extension are included.
std::string ماخذ_Mgr::get_Formatted_Location_No_انحراف(SMLoc Loc,
                                                    منطقی Include_راستہ) ثابت {
  خودکار بفر_ID = ڈھونڈو_بفر_Containing_Loc(Loc);
  أكد(بفر_ID && "Invalid location!");
  خودکار FileSpec = get_بفر_Info(بفر_ID).بفر->get_بفر_Identifier();

  اگر (Include_راستہ) {
    واپس FileSpec.str() + ":" + std::مقصد_string(ڈھونڈو_سطر_نمبر(Loc, بفر_ID));
  } ورنہ {
    خودکار I = FileSpec.ڈھونڈو_آخری_کا("/\\");
    I = (I == FileSpec.سائز_2()) ? 0 : (I + 1);
    واپس FileSpec.substr(I).str() + ":" +
           std::مقصد_string(ڈھونڈو_سطر_نمبر(Loc, بفر_ID));
  }
}

/// Given a line and column number in a mapped buffer, turn it into an SMLoc.
/// This will return a null SMLoc if the line/column location is invalid.
SMLoc ماخذ_Mgr::ڈھونڈو_Loc_For_سطر_And_عمود(غیرعلامتی بفر_ID, غیرعلامتی سطر_No,
                                         غیرعلامتی ستون_No) {
  خودکار &SB = get_بفر_Info(بفر_ID);
  ثابت حرف *Ptr = SB.get_اشارہ_گر_For_سطر_نمبر(سطر_No);
  اگر (!Ptr)
    واپس SMLoc();

  // We start counting line and column numbers from 1.
  اگر (ستون_No != 0)
    --ستون_No;

  // If we have a column number, validate it.
  اگر (ستون_No) {
    // Make sure the location is within the current line.
    اگر (Ptr + ستون_No > SB.بفر->get_بفر_انتہا())
      واپس SMLoc();

    // Make sure there is no newline in the way.
    اگر (StringRef(Ptr, ستون_No).ڈھونڈو_پہلا_کا("\n\r") != StringRef::npos)
      واپس SMLoc();

    Ptr += ستون_No;
  }

  واپس SMLoc::get_From_اشارہ_گر(Ptr);
}

خالی ماخذ_Mgr::Print_Include_ڈھیر(SMLoc IncludeLoc, raw_ostream &OS) ثابت {
  اگر (IncludeLoc == SMLoc())
    واپس; // Top of stack.

  غیرعلامتی CurBuf = ڈھونڈو_بفر_Containing_Loc(IncludeLoc);
  أكد(CurBuf && "Invalid or unspecified location!");

  Print_Include_ڈھیر(get_بفر_Info(CurBuf).IncludeLoc, OS);

  OS << "Included from " << get_بفر_Info(CurBuf).بفر->get_بفر_Identifier()
     << ":" << ڈھونڈو_سطر_نمبر(IncludeLoc, CurBuf) << ":\n";
}

خالی ماخذ_Mgr::print_Include_ڈھیر_For_Diagnostic(SMLoc Loc,
                                               raw_ostream &OS) ثابت {
  اگر (!Loc.isValid())
    واپس;
  غیرعلامتی Diag_Cur_بفر = ڈھونڈو_بفر_Containing_Loc(Loc);
  اگر (Diag_Cur_بفر && Diag_Cur_بفر != getMainFileID()) {
    SMLoc والد_Include_Loc = get_والد_Include_Loc(Diag_Cur_بفر);
    // Ignore macro instantiation buffers to avoid redundant include stacks.
    Print_Include_ڈھیر(والد_Include_Loc, OS);
  }
}

SMDiagnostic ماخذ_Mgr::Get_پیغام(SMLoc Loc, ماخذ_Mgr::Diag_قسم قسم_3,
                                   ثابت Twine &پیغام_4, ArrayRef<SMRange> Ranges,
                                   ArrayRef<SMFixIt> FixIts) ثابت {
  // First thing to do: find the current buffer containing the specified
  // location to pull out the source line.
  Small_سمتیہ<std::pair<غیرعلامتی, غیرعلامتی>, 4> ستون_Ranges;
  std::pair<غیرعلامتی, غیرعلامتی> سطر_And_ستون;
  StringRef بفر_ID = "<unknown>";
  StringRef سطر_Str;

  اگر (Loc.isValid()) {
    غیرعلامتی CurBuf = ڈھونڈو_بفر_Containing_Loc(Loc);
    أكد(CurBuf && "Invalid or unspecified location!");

    ثابت حافظہ_بفر *CurMB = get_حافظہ_بفر(CurBuf);
    بفر_ID = CurMB->get_بفر_Identifier();

    // Scan backward to find the start of the line.
    ثابت حرف *سطر_آغاز = Loc.get_اشارہ_گر();
    ثابت حرف *Buf_آغاز = CurMB->get_بفر_آغاز();
    جب (سطر_آغاز != Buf_آغاز && سطر_آغاز[-1] != '\n' &&
           سطر_آغاز[-1] != '\r')
      --سطر_آغاز;

    // Get the end of the line.
    ثابت حرف *سطر_انتہا = Loc.get_اشارہ_گر();
    ثابت حرف *Buf_انتہا = CurMB->get_بفر_انتہا();
    جب (سطر_انتہا != Buf_انتہا && سطر_انتہا[0] != '\n' && سطر_انتہا[0] != '\r')
      ++سطر_انتہا;
    سطر_Str = StringRef(سطر_آغاز, سطر_انتہا - سطر_آغاز);

    // Convert any ranges to column ranges that only intersect the line of the
    // location.
    برائے (SMRange R : Ranges) {
      اگر (!R.isValid())
        جاری;

      // If the line doesn't contain any part of the range, then ignore it.
      اگر (R.آغاز.get_اشارہ_گر() > سطر_انتہا || R.انتہا_2.get_اشارہ_گر() < سطر_آغاز)
        جاری;

      // Ignore pieces of the range that go onto other lines.
      اگر (R.آغاز.get_اشارہ_گر() < سطر_آغاز)
        R.آغاز = SMLoc::get_From_اشارہ_گر(سطر_آغاز);
      اگر (R.انتہا_2.get_اشارہ_گر() > سطر_انتہا)
        R.انتہا_2 = SMLoc::get_From_اشارہ_گر(سطر_انتہا);

      // Translate from SMLoc ranges to column ranges.
      // FIXME: Handle multibyte characters.
      ستون_Ranges.ڈالو_عقب(std::make_pair(R.آغاز.get_اشارہ_گر() - سطر_آغاز,
                                         R.انتہا_2.get_اشارہ_گر() - سطر_آغاز));
    }

    سطر_And_ستون = get_سطر_And_عمود(Loc, CurBuf);
  }

  واپس SMDiagnostic(*یہ, Loc, بفر_ID, سطر_And_ستون.پہلا_2,
                      سطر_And_ستون.دوسرا_2 - 1, قسم_3, پیغام_4.str(), سطر_Str,
                      ستون_Ranges, FixIts);
}

خالی ماخذ_Mgr::Print_پیغام(raw_ostream &OS, ثابت SMDiagnostic &Diagnostic,
                             منطقی ShowColors) ثابت {
  // Report the message with the diagnostic handler if present.
  اگر (DiagHandler) {
    DiagHandler(Diagnostic, DiagContext);
    واپس;
  }

  اگر (Diagnostic.getLoc().isValid()) {
    غیرعلامتی CurBuf = ڈھونڈو_بفر_Containing_Loc(Diagnostic.getLoc());
    أكد(CurBuf && "Invalid or unspecified location!");
    Print_Include_ڈھیر(get_بفر_Info(CurBuf).IncludeLoc, OS);
  }

  Diagnostic.print(خالی_اشاریہ, OS, ShowColors);
}

خالی ماخذ_Mgr::Print_پیغام(raw_ostream &OS, SMLoc Loc,
                             ماخذ_Mgr::Diag_قسم قسم_3, ثابت Twine &پیغام_4,
                             ArrayRef<SMRange> Ranges, ArrayRef<SMFixIt> FixIts,
                             منطقی ShowColors) ثابت {
  Print_پیغام(OS, Get_پیغام(Loc, قسم_3, پیغام_4, Ranges, FixIts), ShowColors);
}

خالی ماخذ_Mgr::Print_پیغام(SMLoc Loc, ماخذ_Mgr::Diag_قسم قسم_3,
                             ثابت Twine &پیغام_4, ArrayRef<SMRange> Ranges,
                             ArrayRef<SMFixIt> FixIts, منطقی ShowColors) ثابت {
  Print_پیغام(errs(), Loc, قسم_3, پیغام_4, Ranges, FixIts, ShowColors);
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

SMDiagnostic::SMDiagnostic(ثابت ماخذ_Mgr &sm, SMLoc L, StringRef FN, صحیح سطر,
                           صحیح ستون, ماخذ_Mgr::Diag_قسم قسم_3, StringRef پیغام_4,
                           StringRef سطر_Str,
                           ArrayRef<std::pair<غیرعلامتی, غیرعلامتی>> Ranges,
                           ArrayRef<SMFixIt> Hints)
    : SM(&sm), Loc(L), Filename(std::string(FN)), سطر_No(سطر), عمود_No(ستون),
      قسم_3(قسم_3), پیغام_3(پیغام_4), سطر_Contents(سطر_Str), Ranges(Ranges.vec()),
      FixIts(Hints) {
  llvm::ترتیب_دو(FixIts);
}

ساکن خالی build_Fix_It_سطر(std::string &Caret_سطر, std::string &Fix_It_سطر,
                           ArrayRef<SMFixIt> FixIts,
                           ArrayRef<حرف> ماخذ_سطر) {
  اگر (FixIts.خالی_خانے())
    واپس;

  ثابت حرف *سطر_آغاز = ماخذ_سطر.begin();
  ثابت حرف *سطر_انتہا = ماخذ_سطر.انتہا();

  size_t پچھلا_Hint_انتہا_ستون = 0;

  برائے (ثابت llvm::SMFixIt &Fixit : FixIts) {
    // If the fixit contains a newline or tab, ignore it.
    اگر (Fixit.get_متن().ڈھونڈو_پہلا_کا("\n\r\t") != StringRef::npos)
      جاری;

    SMRange R = Fixit.getRange();

    // If the line doesn't contain any part of the range, then ignore it.
    اگر (R.آغاز.get_اشارہ_گر() > سطر_انتہا || R.انتہا_2.get_اشارہ_گر() < سطر_آغاز)
      جاری;

    // Translate from SMLoc to column.
    // Ignore pieces of the range that go onto other lines.
    // FIXME: Handle multibyte characters in the source line.
    غیرعلامتی پہلا_ستون;
    اگر (R.آغاز.get_اشارہ_گر() < سطر_آغاز)
      پہلا_ستون = 0;
    ورنہ
      پہلا_ستون = R.آغاز.get_اشارہ_گر() - سطر_آغاز;

    // If we inserted a long previous hint, push this one forwards, and add
    // an extra space to show that this is not part of the previous
    // completion. This is sort of the best we can do when two hints appear
    // to overlap.
    //
    // Note that if this hint is located immediately after the previous
    // hint, no space will be added, since the location is more important.
    غیرعلامتی Hint_ستون = پہلا_ستون;
    اگر (Hint_ستون < پچھلا_Hint_انتہا_ستون)
      Hint_ستون = پچھلا_Hint_انتہا_ستون + 1;

    // FIXME: This assertion is intended to catch unintended use of multibyte
    // characters in fixits. If we decide to do this, we'll have to track
    // separate byte widths for the source and fixit lines.
    أكد((size_t)sys::locale::عمود_چوڑائی(Fixit.get_متن()) ==
           Fixit.get_متن().سائز_2());

    // This relies on one byte per column in our fixit hints.
    غیرعلامتی آخری_عمود_Modified = Hint_ستون + Fixit.get_متن().سائز_2();
    اگر (آخری_عمود_Modified > Fix_It_سطر.سائز_2())
      Fix_It_سطر.resize(آخری_عمود_Modified, ' ');

    llvm::copy(Fixit.get_متن(), Fix_It_سطر.begin() + Hint_ستون);

    پچھلا_Hint_انتہا_ستون = آخری_عمود_Modified;

    // For replacements, mark the removal range with '~'.
    // FIXME: Handle multibyte characters in the source line.
    غیرعلامتی آخری_ستون;
    اگر (R.انتہا_2.get_اشارہ_گر() >= سطر_انتہا)
      آخری_ستون = سطر_انتہا - سطر_آغاز;
    ورنہ
      آخری_ستون = R.انتہا_2.get_اشارہ_گر() - سطر_آغاز;

    std::fill(&Caret_سطر[پہلا_ستون], &Caret_سطر[آخری_ستون], '~');
  }
}

ساکن خالی print_ماخذ_سطر(raw_ostream &S, StringRef سطر_Contents) {
  // Print out the source line one character at a time, so we can expand tabs.
  برائے (غیرعلامتی i = 0, e = سطر_Contents.سائز_2(), برآمد_ستون = 0; i != e; ++i) {
    size_t اگلا_Tab = سطر_Contents.ڈھونڈو('\t', i);
    // If there were no tabs left, print the rest, we are done.
    اگر (اگلا_Tab == StringRef::npos) {
      S << سطر_Contents.drop_front(i);
      توڑو;
    }

    // Otherwise, print from i to NextTab.
    S << سطر_Contents.قاش(i, اگلا_Tab);
    برآمد_ستون += اگلا_Tab - i;
    i = اگلا_Tab;

    // If we have a tab, emit at least one space, then round up to 8 columns.
    کرو {
      S << ' ';
      ++برآمد_ستون;
    } جب ((برآمد_ستون % TabStop) != 0);
  }
  S << '\n';
}

ساکن منطقی isNonASCII(حرف c) { واپس c & 0x80; }

خالی SMDiagnostic::print(ثابت حرف *ProgName, raw_ostream &OS, منطقی ShowColors,
                         منطقی Show_قسم_Label, منطقی ShowLocation) ثابت {
  Color_منوال منوال = ShowColors ? Color_منوال::Auto : Color_منوال::Disable;

  {
    WithColor S(OS, raw_ostream::SAVEDCOLOR, سچ, جھوٹ, منوال);

    اگر (ProgName && ProgName[0])
      S << ProgName << ": ";

    اگر (ShowLocation && !Filename.خالی_خانے()) {
      اگر (Filename == "-")
        S << "<stdin>";
      ورنہ
        S << Filename;

      اگر (سطر_No != -1) {
        S << ':' << سطر_No;
        اگر (عمود_No != -1)
          S << ':' << (عمود_No + 1);
      }
      S << ": ";
    }
  }

  اگر (Show_قسم_Label) {
    انتخاب (قسم_3) {
    حالت ماخذ_Mgr::DK_Error:
      WithColor::error(OS, "", !ShowColors);
      توڑو;
    حالت ماخذ_Mgr::DK_Warning:
      WithColor::warning(OS, "", !ShowColors);
      توڑو;
    حالت ماخذ_Mgr::DK_Note:
      WithColor::note(OS, "", !ShowColors);
      توڑو;
    حالت ماخذ_Mgr::DK_Remark:
      WithColor::remark(OS, "", !ShowColors);
      توڑو;
    }
  }

  WithColor(OS, raw_ostream::SAVEDCOLOR, سچ, جھوٹ, منوال) << پیغام_3 << '\n';

  اگر (سطر_No == -1 || عمود_No == -1)
    واپس;

  // FIXME: If there are multibyte or multi-column characters in the source, all
  // our ranges will be wrong. To do this properly, we'll need a byte-to-column
  // map like Clang's TextDiagnostic. For now, we'll just handle tabs by
  // expanding them later, and bail out rather than show incorrect ranges and
  // misaligned fixits for any other odd characters.
  اگر (any_کا(سطر_Contents, isNonASCII)) {
    print_ماخذ_سطر(OS, سطر_Contents);
    واپس;
  }
  size_t عدد_Columns = سطر_Contents.سائز_2();

  // Build the line with the caret and ranges.
  std::string Caret_سطر(عدد_Columns + 1, ' ');

  // Expand any ranges.
  برائے (ثابت std::pair<غیرعلامتی, غیرعلامتی> &R : Ranges)
    std::fill(&Caret_سطر[R.پہلا_2],
              &Caret_سطر[std::min((size_t)R.دوسرا_2, Caret_سطر.سائز_2())], '~');

  // Add any fix-its.
  // FIXME: Find the beginning of the line properly for multibyte characters.
  std::string Fix_It_Insertion_سطر;
  build_Fix_It_سطر(Caret_سطر, Fix_It_Insertion_سطر, FixIts,
                 ArrayRef(Loc.get_اشارہ_گر() - عمود_No, سطر_Contents.سائز_2()));

  // Finally, plop on the caret.
  اگر (غیرعلامتی(عمود_No) <= عدد_Columns)
    Caret_سطر[عمود_No] = '^';
  ورنہ
    Caret_سطر[عدد_Columns] = '^';

  // ... and remove trailing whitespace so the output doesn't wrap for it.  We
  // know that the line isn't completely empty because it has the caret in it at
  // least.
  Caret_سطر.erase(Caret_سطر.ڈھونڈو_آخری_not_کا(' ') + 1);

  print_ماخذ_سطر(OS, سطر_Contents);

  {
    Color_منوال منوال = ShowColors ? Color_منوال::Auto : Color_منوال::Disable;
    WithColor S(OS, raw_ostream::GREEN, سچ, جھوٹ, منوال);

    // Print out the caret line, matching tabs in the source line.
    برائے (غیرعلامتی i = 0, e = Caret_سطر.سائز_2(), برآمد_ستون = 0; i != e; ++i) {
      اگر (i >= سطر_Contents.سائز_2() || سطر_Contents[i] != '\t') {
        S << Caret_سطر[i];
        ++برآمد_ستون;
        جاری;
      }

      // Okay, we have a tab.  Insert the appropriate number of characters.
      کرو {
        S << Caret_سطر[i];
        ++برآمد_ستون;
      } جب ((برآمد_ستون % TabStop) != 0);
    }
    S << '\n';
  }

  // Print out the replacement line, matching tabs in the source line.
  اگر (Fix_It_Insertion_سطر.خالی_خانے())
    واپس;

  برائے (size_t i = 0, e = Fix_It_Insertion_سطر.سائز_2(), برآمد_ستون = 0; i < e; ++i) {
    اگر (i >= سطر_Contents.سائز_2() || سطر_Contents[i] != '\t') {
      OS << Fix_It_Insertion_سطر[i];
      ++برآمد_ستون;
      جاری;
    }

    // Okay, we have a tab.  Insert the appropriate number of characters.
    کرو {
      OS << Fix_It_Insertion_سطر[i];
      // FIXME: This is trying not to break up replacements, but then to re-sync
      // with the tabs between replacements. This will fail, though, if two
      // fix-it replacements are exactly adjacent, or if a fix-it contains a
      // space. Really we should be precomputing column widths, which we'll
      // need anyway for multibyte chars.
      اگر (Fix_It_Insertion_سطر[i] != ' ')
        ++i;
      ++برآمد_ستون;
    } جب (((برآمد_ستون % TabStop) != 0) && i != e);
  }
  OS << '\n';
}
