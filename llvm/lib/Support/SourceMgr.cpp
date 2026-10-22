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

ورتوں نطاق llvm;

ساكن ثابت size_t TabStop = 8;

// Out of line to avoid needing definition of vfs::FileSystem in header.
سروت_Mgr::سروت_Mgr() = افتراضي;
سروت_Mgr::سروت_Mgr(IntrusiveRefCntPtr<vfs::FileSystem> FS)
    : FS(std::move(FS)) {}
سروت_Mgr::سروت_Mgr(سروت_Mgr &&) = افتراضي;
سروت_Mgr &سروت_Mgr::عامل=(سروت_Mgr &&) = افتراضي;
سروت_Mgr::~سروت_Mgr() = افتراضي;

IntrusiveRefCntPtr<vfs::FileSystem> سروت_Mgr::getVirtualFileSystem() ثابت {
  أرجع FS;
}

فراغ سروت_Mgr::سیٹ_Virtual_File_System(IntrusiveRefCntPtr<vfs::FileSystem> FS) {
  هذا->FS = std::move(FS);
}

بنانشانی سروت_Mgr::جوڑو_Include_File(ثابت std::string &Filename,
                                   SMLoc IncludeLoc,
                                   std::string &IncludedFile) {
  ErrorOr<std::unique_ptr<حافظہ_بفر>> NewBufOrErr =
      OpenIncludeFile(Filename, IncludedFile);
  إذا (!NewBufOrErr)
    أرجع 0;

  أرجع جوڑو_New_سروت_بفر(std::move(*NewBufOrErr), IncludeLoc);
}

ErrorOr<std::unique_ptr<حافظہ_بفر>>
سروت_Mgr::OpenIncludeFile(ثابت std::string &Filename,
                           std::string &IncludedFile,
                           منطقي RequiresNullTerminator) {
  خودکار GetFile = [هذا, RequiresNullTerminator](StringRef رستہ) {
    أرجع FS ? FS->get_بفر_For_File(رستہ, /*FileSize=*/-1,
                                     RequiresNullTerminator)
              : حافظہ_بفر::getFile(رستہ, /*IsText=*/خطأ,
                                      RequiresNullTerminator);
  };

  ErrorOr<std::unique_ptr<حافظہ_بفر>> NewBufOrErr = GetFile(Filename);

  SmallString<64> بفر(Filename);
  // If the file didn't exist directly, see if it's in an include path.
  لكل (بنانشانی i = 0, e = IncludeDirectories.ناپ_2(); i != e && !NewBufOrErr;
       ++i) {
    بفر = IncludeDirectories[i];
    sys::رستہ_2::append(بفر, Filename);
    NewBufOrErr = GetFile(بفر);
  }

  إذا (NewBufOrErr)
    IncludedFile = static_cast<std::string>(بفر);

  أرجع NewBufOrErr;
}

بنانشانی سروت_Mgr::لبھو_بفر_Containing_Loc(SMLoc Loc) ثابت {
  لكل (بنانشانی i = 0, e = Buffers.ناپ_2(); i != e; ++i)
    إذا (Loc.get_اشارہ_گر() >= Buffers[i].بفر->get_بفر_شروع() &&
        // Use <= here so that a pointer to the null at the end of the buffer
        // is included as part of the buffer.
        Loc.get_اشارہ_گر() <= Buffers[i].بفر->get_بفر_سرا())
      أرجع i + 1;
  أرجع 0;
}

سانچا <قسم_ناں T>
ساكن std::سمتیہ<T> &Get_Or_Create_وتھ_Cache(فراغ *&وتھ_Cache,
                                              حافظہ_بفر *بفر) {
  إذا (وتھ_Cache)
    أرجع *static_cast<std::سمتیہ<T> *>(وتھ_Cache);

  // Lazily fill in the offset cache.
  خودکار *Offsets = جديد std::سمتیہ<T>();
  size_t Sz = بفر->get_بفر_ناپ();
  أكد(Sz <= std::numeric_limits<T>::max());
  StringRef S = بفر->get_بفر();
  لكل (size_t N = 0; N < Sz; ++N) {
    إذا (S[N] == '\n')
      Offsets->ڈالو_پٹھ(static_cast<T>(N));
  }

  وتھ_Cache = Offsets;
  أرجع *Offsets;
}

سانچا <قسم_ناں T>
std::pair<بنانشانی, بنانشانی>
سروت_Mgr::Src_بفر::get_سطر_And_کالم_Specialized(ثابت حرف *Ptr) ثابت {
  std::سمتیہ<T> &Offsets =
      Get_Or_Create_وتھ_Cache<T>(وتھ_Cache, بفر.get());

  ثابت حرف *Buf_شروع = بفر->get_بفر_شروع();
  أكد(Ptr >= Buf_شروع && Ptr <= بفر->get_بفر_سرا());
  ptrdiff_t PtrDiff = Ptr - Buf_شروع;
  أكد(PtrDiff >= 0 &&
         static_cast<size_t>(PtrDiff) <= std::numeric_limits<T>::max());
  T Ptr_وتھ = static_cast<T>(PtrDiff);

  // llvm::lower_bound gives the number of EOL before PtrOffset. Add 1 to get
  // the line number.
  خودکار I = llvm::ہیٹھلا_bound(Offsets, Ptr_وتھ);
  بنانشانی سطر_No = I - Offsets.begin() + 1;

  // The column number is the distance from the previous EOL (or the start of
  // the buffer) to the pointer.
  T سطر_شروع_Offs = (I == Offsets.begin()) ? 0 : (I[-1] + 1);
  بنانشانی خانہ_No = Ptr_وتھ - سطر_شروع_Offs + 1;
  أرجع {سطر_No, خانہ_No};
}

/// Look up a given \p Ptr in the buffer, determining which line and column
/// it came from.
LLVM_ABI std::pair<بنانشانی, بنانشانی>
سروت_Mgr::Src_بفر::get_سطر_And_کالم(ثابت حرف *Ptr) ثابت {
  size_t Sz = بفر->get_بفر_ناپ();
  إذا (Sz <= std::numeric_limits<uint8_t>::max())
    أرجع get_سطر_And_کالم_Specialized<uint8_t>(Ptr);
  إذا (Sz <= std::numeric_limits<uint16_t>::max())
    أرجع get_سطر_And_کالم_Specialized<uint16_t>(Ptr);
  إذا (Sz <= std::numeric_limits<uint32_t>::max())
    أرجع get_سطر_And_کالم_Specialized<uint32_t>(Ptr);
  أرجع get_سطر_And_کالم_Specialized<uint64_t>(Ptr);
}

سانچا <قسم_ناں T>
ثابت حرف *سروت_Mgr::Src_بفر::get_اشارہ_گر_For_سطر_نمبر_Specialized(
    بنانشانی سطر_No) ثابت {
  std::سمتیہ<T> &Offsets =
      Get_Or_Create_وتھ_Cache<T>(وتھ_Cache, بفر.get());

  // We start counting line and column numbers from 1.
  إذا (سطر_No != 0)
    --سطر_No;

  ثابت حرف *Buf_شروع = بفر->get_بفر_شروع();

  // The offset cache contains the location of the \n for the specified line,
  // we want the start of the line.  As such, we look for the previous entry.
  إذا (سطر_No == 0)
    أرجع Buf_شروع;
  إذا (سطر_No > Offsets.ناپ_2())
    أرجع خالی_سوچک;
  أرجع Buf_شروع + Offsets[سطر_No - 1] + 1;
}

/// Return a pointer to the first character of the specified line number or
/// null if the line number is invalid.
ثابت حرف *
سروت_Mgr::Src_بفر::get_اشارہ_گر_For_سطر_نمبر(بنانشانی سطر_No) ثابت {
  size_t Sz = بفر->get_بفر_ناپ();
  إذا (Sz <= std::numeric_limits<uint8_t>::max())
    أرجع get_اشارہ_گر_For_سطر_نمبر_Specialized<uint8_t>(سطر_No);
  وإلا إذا (Sz <= std::numeric_limits<uint16_t>::max())
    أرجع get_اشارہ_گر_For_سطر_نمبر_Specialized<uint16_t>(سطر_No);
  وإلا إذا (Sz <= std::numeric_limits<uint32_t>::max())
    أرجع get_اشارہ_گر_For_سطر_نمبر_Specialized<uint32_t>(سطر_No);
  وإلا
    أرجع get_اشارہ_گر_For_سطر_نمبر_Specialized<uint64_t>(سطر_No);
}

سروت_Mgr::Src_بفر::Src_بفر(سروت_Mgr::Src_بفر &&Other)
    : بفر(std::move(Other.بفر)), وتھ_Cache(Other.وتھ_Cache),
      IncludeLoc(Other.IncludeLoc) {
  Other.وتھ_Cache = خالی_سوچک;
}

سروت_Mgr::Src_بفر::~Src_بفر() {
  إذا (وتھ_Cache) {
    size_t Sz = بفر->get_بفر_ناپ();
    إذا (Sz <= std::numeric_limits<uint8_t>::max())
      مٹاؤ static_cast<std::سمتیہ<uint8_t> *>(وتھ_Cache);
    وإلا إذا (Sz <= std::numeric_limits<uint16_t>::max())
      مٹاؤ static_cast<std::سمتیہ<uint16_t> *>(وتھ_Cache);
    وإلا إذا (Sz <= std::numeric_limits<uint32_t>::max())
      مٹاؤ static_cast<std::سمتیہ<uint32_t> *>(وتھ_Cache);
    وإلا
      مٹاؤ static_cast<std::سمتیہ<uint64_t> *>(وتھ_Cache);
    وتھ_Cache = خالی_سوچک;
  }
}

std::pair<بنانشانی, بنانشانی>
سروت_Mgr::get_سطر_And_کالم(SMLoc Loc, بنانشانی بفر_ID) ثابت {
  إذا (!بفر_ID)
    بفر_ID = لبھو_بفر_Containing_Loc(Loc);
  أكد(بفر_ID && "Invalid location!");

  خودکار &SB = get_بفر_Info(بفر_ID);
  ثابت حرف *Ptr = Loc.get_اشارہ_گر();

  أرجع SB.get_سطر_And_کالم(Ptr);
}

// FIXME: Note that the formatting of source locations is spread between
// multiple functions, some in SourceMgr and some in SMDiagnostic. A better
// solution would be a general-purpose source location formatter
// in one of those two classes, or possibly in SMLoc.

/// Get a string with the source location formatted in the standard
/// style, but without the line offset. If \p IncludePath is true, the path
/// is included. If false, only the file name and extension are included.
std::string سروت_Mgr::get_Formatted_Location_No_وتھ(SMLoc Loc,
                                                    منطقي Include_رستہ) ثابت {
  خودکار بفر_ID = لبھو_بفر_Containing_Loc(Loc);
  أكد(بفر_ID && "Invalid location!");
  خودکار FileSpec = get_بفر_Info(بفر_ID).بفر->get_بفر_Identifier();

  إذا (Include_رستہ) {
    أرجع FileSpec.str() + ":" + std::منزل_string(لبھو_سطر_نمبر(Loc, بفر_ID));
  } وإلا {
    خودکار I = FileSpec.لبھو_آخری_دا("/\\");
    I = (I == FileSpec.ناپ_2()) ? 0 : (I + 1);
    أرجع FileSpec.substr(I).str() + ":" +
           std::منزل_string(لبھو_سطر_نمبر(Loc, بفر_ID));
  }
}

/// Given a line and column number in a mapped buffer, turn it into an SMLoc.
/// This will return a null SMLoc if the line/column location is invalid.
SMLoc سروت_Mgr::لبھو_Loc_For_سطر_And_کالم(بنانشانی بفر_ID, بنانشانی سطر_No,
                                         بنانشانی خانہ_No) {
  خودکار &SB = get_بفر_Info(بفر_ID);
  ثابت حرف *Ptr = SB.get_اشارہ_گر_For_سطر_نمبر(سطر_No);
  إذا (!Ptr)
    أرجع SMLoc();

  // We start counting line and column numbers from 1.
  إذا (خانہ_No != 0)
    --خانہ_No;

  // If we have a column number, validate it.
  إذا (خانہ_No) {
    // Make sure the location is within the current line.
    إذا (Ptr + خانہ_No > SB.بفر->get_بفر_سرا())
      أرجع SMLoc();

    // Make sure there is no newline in the way.
    إذا (StringRef(Ptr, خانہ_No).لبھو_پہلا_دا("\n\r") != StringRef::npos)
      أرجع SMLoc();

    Ptr += خانہ_No;
  }

  أرجع SMLoc::get_From_اشارہ_گر(Ptr);
}

فراغ سروت_Mgr::Print_Include_ڈھیر(SMLoc IncludeLoc, raw_ostream &OS) ثابت {
  إذا (IncludeLoc == SMLoc())
    أرجع; // Top of stack.

  بنانشانی CurBuf = لبھو_بفر_Containing_Loc(IncludeLoc);
  أكد(CurBuf && "Invalid or unspecified location!");

  Print_Include_ڈھیر(get_بفر_Info(CurBuf).IncludeLoc, OS);

  OS << "Included from " << get_بفر_Info(CurBuf).بفر->get_بفر_Identifier()
     << ":" << لبھو_سطر_نمبر(IncludeLoc, CurBuf) << ":\n";
}

فراغ سروت_Mgr::print_Include_ڈھیر_For_Diagnostic(SMLoc Loc,
                                               raw_ostream &OS) ثابت {
  إذا (!Loc.isValid())
    أرجع;
  بنانشانی Diag_Cur_بفر = لبھو_بفر_Containing_Loc(Loc);
  إذا (Diag_Cur_بفر && Diag_Cur_بفر != getMainFileID()) {
    SMLoc ماپا_Include_Loc = get_ماپا_Include_Loc(Diag_Cur_بفر);
    // Ignore macro instantiation buffers to avoid redundant include stacks.
    Print_Include_ڈھیر(ماپا_Include_Loc, OS);
  }
}

SMDiagnostic سروت_Mgr::Get_سنیہا(SMLoc Loc, سروت_Mgr::Diag_قسم قسم_2,
                                   ثابت Twine &پیغام_2, ArrayRef<SMRange> Ranges,
                                   ArrayRef<SMFixIt> FixIts) ثابت {
  // First thing to do: find the current buffer containing the specified
  // location to pull out the source line.
  Small_سمتیہ<std::pair<بنانشانی, بنانشانی>, 4> خانہ_Ranges;
  std::pair<بنانشانی, بنانشانی> سطر_And_خانہ;
  StringRef بفر_ID = "<unknown>";
  StringRef سطر_Str;

  إذا (Loc.isValid()) {
    بنانشانی CurBuf = لبھو_بفر_Containing_Loc(Loc);
    أكد(CurBuf && "Invalid or unspecified location!");

    ثابت حافظہ_بفر *CurMB = get_حافظہ_بفر(CurBuf);
    بفر_ID = CurMB->get_بفر_Identifier();

    // Scan backward to find the start of the line.
    ثابت حرف *سطر_شروع = Loc.get_اشارہ_گر();
    ثابت حرف *Buf_شروع = CurMB->get_بفر_شروع();
    طالما (سطر_شروع != Buf_شروع && سطر_شروع[-1] != '\n' &&
           سطر_شروع[-1] != '\r')
      --سطر_شروع;

    // Get the end of the line.
    ثابت حرف *سطر_سرا = Loc.get_اشارہ_گر();
    ثابت حرف *Buf_سرا = CurMB->get_بفر_سرا();
    طالما (سطر_سرا != Buf_سرا && سطر_سرا[0] != '\n' && سطر_سرا[0] != '\r')
      ++سطر_سرا;
    سطر_Str = StringRef(سطر_شروع, سطر_سرا - سطر_شروع);

    // Convert any ranges to column ranges that only intersect the line of the
    // location.
    لكل (SMRange R : Ranges) {
      إذا (!R.isValid())
        استمر;

      // If the line doesn't contain any part of the range, then ignore it.
      إذا (R.شروع.get_اشارہ_گر() > سطر_سرا || R.سرا_2.get_اشارہ_گر() < سطر_شروع)
        استمر;

      // Ignore pieces of the range that go onto other lines.
      إذا (R.شروع.get_اشارہ_گر() < سطر_شروع)
        R.شروع = SMLoc::get_From_اشارہ_گر(سطر_شروع);
      إذا (R.سرا_2.get_اشارہ_گر() > سطر_سرا)
        R.سرا_2 = SMLoc::get_From_اشارہ_گر(سطر_سرا);

      // Translate from SMLoc ranges to column ranges.
      // FIXME: Handle multibyte characters.
      خانہ_Ranges.ڈالو_پٹھ(std::make_pair(R.شروع.get_اشارہ_گر() - سطر_شروع,
                                         R.سرا_2.get_اشارہ_گر() - سطر_شروع));
    }

    سطر_And_خانہ = get_سطر_And_کالم(Loc, CurBuf);
  }

  أرجع SMDiagnostic(*هذا, Loc, بفر_ID, سطر_And_خانہ.پہلا_2,
                      سطر_And_خانہ.دوجا_2 - 1, قسم_2, پیغام_2.str(), سطر_Str,
                      خانہ_Ranges, FixIts);
}

فراغ سروت_Mgr::Print_سنیہا(raw_ostream &OS, ثابت SMDiagnostic &Diagnostic,
                             منطقي ShowColors) ثابت {
  // Report the message with the diagnostic handler if present.
  إذا (DiagHandler) {
    DiagHandler(Diagnostic, DiagContext);
    أرجع;
  }

  إذا (Diagnostic.getLoc().isValid()) {
    بنانشانی CurBuf = لبھو_بفر_Containing_Loc(Diagnostic.getLoc());
    أكد(CurBuf && "Invalid or unspecified location!");
    Print_Include_ڈھیر(get_بفر_Info(CurBuf).IncludeLoc, OS);
  }

  Diagnostic.print(خالی_سوچک, OS, ShowColors);
}

فراغ سروت_Mgr::Print_سنیہا(raw_ostream &OS, SMLoc Loc,
                             سروت_Mgr::Diag_قسم قسم_2, ثابت Twine &پیغام_2,
                             ArrayRef<SMRange> Ranges, ArrayRef<SMFixIt> FixIts,
                             منطقي ShowColors) ثابت {
  Print_سنیہا(OS, Get_سنیہا(Loc, قسم_2, پیغام_2, Ranges, FixIts), ShowColors);
}

فراغ سروت_Mgr::Print_سنیہا(SMLoc Loc, سروت_Mgr::Diag_قسم قسم_2,
                             ثابت Twine &پیغام_2, ArrayRef<SMRange> Ranges,
                             ArrayRef<SMFixIt> FixIts, منطقي ShowColors) ثابت {
  Print_سنیہا(errs(), Loc, قسم_2, پیغام_2, Ranges, FixIts, ShowColors);
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

SMDiagnostic::SMDiagnostic(ثابت سروت_Mgr &sm, SMLoc L, StringRef FN, صحيح سطر,
                           صحيح خانہ, سروت_Mgr::Diag_قسم قسم_2, StringRef پیغام_2,
                           StringRef سطر_Str,
                           ArrayRef<std::pair<بنانشانی, بنانشانی>> Ranges,
                           ArrayRef<SMFixIt> Hints)
    : SM(&sm), Loc(L), Filename(std::string(FN)), سطر_No(سطر), کالم_No(خانہ),
      قسم_2(قسم_2), سنیہا_2(پیغام_2), سطر_Contents(سطر_Str), Ranges(Ranges.vec()),
      FixIts(Hints) {
  llvm::ترتیب_دیو(FixIts);
}

ساكن فراغ build_Fix_It_سطر(std::string &Caret_سطر, std::string &Fix_It_سطر,
                           ArrayRef<SMFixIt> FixIts,
                           ArrayRef<حرف> سروت_سطر) {
  إذا (FixIts.خالی_تھاواں())
    أرجع;

  ثابت حرف *سطر_شروع = سروت_سطر.begin();
  ثابت حرف *سطر_سرا = سروت_سطر.سرا();

  size_t Prev_Hint_سرا_خانہ = 0;

  لكل (ثابت llvm::SMFixIt &Fixit : FixIts) {
    // If the fixit contains a newline or tab, ignore it.
    إذا (Fixit.get_متن().لبھو_پہلا_دا("\n\r\t") != StringRef::npos)
      استمر;

    SMRange R = Fixit.getRange();

    // If the line doesn't contain any part of the range, then ignore it.
    إذا (R.شروع.get_اشارہ_گر() > سطر_سرا || R.سرا_2.get_اشارہ_گر() < سطر_شروع)
      استمر;

    // Translate from SMLoc to column.
    // Ignore pieces of the range that go onto other lines.
    // FIXME: Handle multibyte characters in the source line.
    بنانشانی پہلا_خانہ;
    إذا (R.شروع.get_اشارہ_گر() < سطر_شروع)
      پہلا_خانہ = 0;
    وإلا
      پہلا_خانہ = R.شروع.get_اشارہ_گر() - سطر_شروع;

    // If we inserted a long previous hint, push this one forwards, and add
    // an extra space to show that this is not part of the previous
    // completion. This is sort of the best we can do when two hints appear
    // to overlap.
    //
    // Note that if this hint is located immediately after the previous
    // hint, no space will be added, since the location is more important.
    بنانشانی Hint_خانہ = پہلا_خانہ;
    إذا (Hint_خانہ < Prev_Hint_سرا_خانہ)
      Hint_خانہ = Prev_Hint_سرا_خانہ + 1;

    // FIXME: This assertion is intended to catch unintended use of multibyte
    // characters in fixits. If we decide to do this, we'll have to track
    // separate byte widths for the source and fixit lines.
    أكد((size_t)sys::locale::کالم_چوڑائی(Fixit.get_متن()) ==
           Fixit.get_متن().ناپ_2());

    // This relies on one byte per column in our fixit hints.
    بنانشانی آخری_کالم_Modified = Hint_خانہ + Fixit.get_متن().ناپ_2();
    إذا (آخری_کالم_Modified > Fix_It_سطر.ناپ_2())
      Fix_It_سطر.resize(آخری_کالم_Modified, ' ');

    llvm::copy(Fixit.get_متن(), Fix_It_سطر.begin() + Hint_خانہ);

    Prev_Hint_سرا_خانہ = آخری_کالم_Modified;

    // For replacements, mark the removal range with '~'.
    // FIXME: Handle multibyte characters in the source line.
    بنانشانی آخری_خانہ;
    إذا (R.سرا_2.get_اشارہ_گر() >= سطر_سرا)
      آخری_خانہ = سطر_سرا - سطر_شروع;
    وإلا
      آخری_خانہ = R.سرا_2.get_اشارہ_گر() - سطر_شروع;

    std::fill(&Caret_سطر[پہلا_خانہ], &Caret_سطر[آخری_خانہ], '~');
  }
}

ساكن فراغ print_سروت_سطر(raw_ostream &S, StringRef سطر_Contents) {
  // Print out the source line one character at a time, so we can expand tabs.
  لكل (بنانشانی i = 0, e = سطر_Contents.ناپ_2(), باہرلا_خانہ = 0; i != e; ++i) {
    size_t اگلا_Tab = سطر_Contents.لبھو('\t', i);
    // If there were no tabs left, print the rest, we are done.
    إذا (اگلا_Tab == StringRef::npos) {
      S << سطر_Contents.drop_front(i);
      اكسر;
    }

    // Otherwise, print from i to NextTab.
    S << سطر_Contents.ٹکڑا(i, اگلا_Tab);
    باہرلا_خانہ += اگلا_Tab - i;
    i = اگلا_Tab;

    // If we have a tab, emit at least one space, then round up to 8 columns.
    افعل {
      S << ' ';
      ++باہرلا_خانہ;
    } طالما ((باہرلا_خانہ % TabStop) != 0);
  }
  S << '\n';
}

ساكن منطقي isNonASCII(حرف c) { أرجع c & 0x80; }

فراغ SMDiagnostic::print(ثابت حرف *ProgName, raw_ostream &OS, منطقي ShowColors,
                         منطقي Show_قسم_Label, منطقي ShowLocation) ثابت {
  Color_منوال منوال = ShowColors ? Color_منوال::Auto : Color_منوال::Disable;

  {
    WithColor S(OS, raw_ostream::SAVEDCOLOR, سچ, خطأ, منوال);

    إذا (ProgName && ProgName[0])
      S << ProgName << ": ";

    إذا (ShowLocation && !Filename.خالی_تھاواں()) {
      إذا (Filename == "-")
        S << "<stdin>";
      وإلا
        S << Filename;

      إذا (سطر_No != -1) {
        S << ':' << سطر_No;
        إذا (کالم_No != -1)
          S << ':' << (کالم_No + 1);
      }
      S << ": ";
    }
  }

  إذا (Show_قسم_Label) {
    بدل (قسم_2) {
    حالة سروت_Mgr::DK_Error:
      WithColor::error(OS, "", !ShowColors);
      اكسر;
    حالة سروت_Mgr::DK_Warning:
      WithColor::warning(OS, "", !ShowColors);
      اكسر;
    حالة سروت_Mgr::DK_Note:
      WithColor::note(OS, "", !ShowColors);
      اكسر;
    حالة سروت_Mgr::DK_Remark:
      WithColor::remark(OS, "", !ShowColors);
      اكسر;
    }
  }

  WithColor(OS, raw_ostream::SAVEDCOLOR, سچ, خطأ, منوال) << سنیہا_2 << '\n';

  إذا (سطر_No == -1 || کالم_No == -1)
    أرجع;

  // FIXME: If there are multibyte or multi-column characters in the source, all
  // our ranges will be wrong. To do this properly, we'll need a byte-to-column
  // map like Clang's TextDiagnostic. For now, we'll just handle tabs by
  // expanding them later, and bail out rather than show incorrect ranges and
  // misaligned fixits for any other odd characters.
  إذا (any_دا(سطر_Contents, isNonASCII)) {
    print_سروت_سطر(OS, سطر_Contents);
    أرجع;
  }
  size_t عدد_Columns = سطر_Contents.ناپ_2();

  // Build the line with the caret and ranges.
  std::string Caret_سطر(عدد_Columns + 1, ' ');

  // Expand any ranges.
  لكل (ثابت std::pair<بنانشانی, بنانشانی> &R : Ranges)
    std::fill(&Caret_سطر[R.پہلا_2],
              &Caret_سطر[std::min((size_t)R.دوجا_2, Caret_سطر.ناپ_2())], '~');

  // Add any fix-its.
  // FIXME: Find the beginning of the line properly for multibyte characters.
  std::string Fix_It_Insertion_سطر;
  build_Fix_It_سطر(Caret_سطر, Fix_It_Insertion_سطر, FixIts,
                 ArrayRef(Loc.get_اشارہ_گر() - کالم_No, سطر_Contents.ناپ_2()));

  // Finally, plop on the caret.
  إذا (بنانشانی(کالم_No) <= عدد_Columns)
    Caret_سطر[کالم_No] = '^';
  وإلا
    Caret_سطر[عدد_Columns] = '^';

  // ... and remove trailing whitespace so the output doesn't wrap for it.  We
  // know that the line isn't completely empty because it has the caret in it at
  // least.
  Caret_سطر.erase(Caret_سطر.لبھو_آخری_not_دا(' ') + 1);

  print_سروت_سطر(OS, سطر_Contents);

  {
    Color_منوال منوال = ShowColors ? Color_منوال::Auto : Color_منوال::Disable;
    WithColor S(OS, raw_ostream::GREEN, سچ, خطأ, منوال);

    // Print out the caret line, matching tabs in the source line.
    لكل (بنانشانی i = 0, e = Caret_سطر.ناپ_2(), باہرلا_خانہ = 0; i != e; ++i) {
      إذا (i >= سطر_Contents.ناپ_2() || سطر_Contents[i] != '\t') {
        S << Caret_سطر[i];
        ++باہرلا_خانہ;
        استمر;
      }

      // Okay, we have a tab.  Insert the appropriate number of characters.
      افعل {
        S << Caret_سطر[i];
        ++باہرلا_خانہ;
      } طالما ((باہرلا_خانہ % TabStop) != 0);
    }
    S << '\n';
  }

  // Print out the replacement line, matching tabs in the source line.
  إذا (Fix_It_Insertion_سطر.خالی_تھاواں())
    أرجع;

  لكل (size_t i = 0, e = Fix_It_Insertion_سطر.ناپ_2(), باہرلا_خانہ = 0; i < e; ++i) {
    إذا (i >= سطر_Contents.ناپ_2() || سطر_Contents[i] != '\t') {
      OS << Fix_It_Insertion_سطر[i];
      ++باہرلا_خانہ;
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
      ++باہرلا_خانہ;
    } طالما (((باہرلا_خانہ % TabStop) != 0) && i != e);
  }
  OS << '\n';
}
