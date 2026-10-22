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

استعمال نطاق llvm;

ساكن ثابت size_t TabStop = 8;

// Out of line to avoid needing definition of vfs::FileSystem in header.
ذريعو_Mgr::ذريعو_Mgr() = افتراضي;
ذريعو_Mgr::ذريعو_Mgr(IntrusiveRefCntPtr<vfs::FileSystem> FS)
    : FS(std::move(FS)) {}
ذريعو_Mgr::ذريعو_Mgr(ذريعو_Mgr &&) = افتراضي;
ذريعو_Mgr &ذريعو_Mgr::عامل=(ذريعو_Mgr &&) = افتراضي;
ذريعو_Mgr::~ذريعو_Mgr() = افتراضي;

IntrusiveRefCntPtr<vfs::FileSystem> ذريعو_Mgr::getVirtualFileSystem() ثابت {
  أرجع FS;
}

فراغ ذريعو_Mgr::سيٽ_Virtual_File_System(IntrusiveRefCntPtr<vfs::FileSystem> FS) {
  هذا->FS = std::move(FS);
}

بنانشان ذريعو_Mgr::جوڙيو_Include_File(ثابت std::string &Filename,
                                   SMLoc IncludeLoc,
                                   std::string &IncludedFile) {
  ErrorOr<std::unique_ptr<يادگيري_بفر>> NewBufOrErr =
      OpenIncludeFile(Filename, IncludedFile);
  إذا (!NewBufOrErr)
    أرجع 0;

  أرجع جوڙيو_New_ذريعو_بفر(std::move(*NewBufOrErr), IncludeLoc);
}

ErrorOr<std::unique_ptr<يادگيري_بفر>>
ذريعو_Mgr::OpenIncludeFile(ثابت std::string &Filename,
                           std::string &IncludedFile,
                           منطقي RequiresNullTerminator) {
  خودڪار GetFile = [هذا, RequiresNullTerminator](StringRef رستو) {
    أرجع FS ? FS->get_بفر_For_File(رستو, /*FileSize=*/-1,
                                     RequiresNullTerminator)
              : يادگيري_بفر::getFile(رستو, /*IsText=*/خطأ,
                                      RequiresNullTerminator);
  };

  ErrorOr<std::unique_ptr<يادگيري_بفر>> NewBufOrErr = GetFile(Filename);

  SmallString<64> بفر(Filename);
  // If the file didn't exist directly, see if it's in an include path.
  لكل (بنانشان i = 0, e = IncludeDirectories.سائيز_2(); i != e && !NewBufOrErr;
       ++i) {
    بفر = IncludeDirectories[i];
    sys::رستو_2::append(بفر, Filename);
    NewBufOrErr = GetFile(بفر);
  }

  إذا (NewBufOrErr)
    IncludedFile = static_cast<std::string>(بفر);

  أرجع NewBufOrErr;
}

بنانشان ذريعو_Mgr::ڳوليو_بفر_Containing_Loc(SMLoc Loc) ثابت {
  لكل (بنانشان i = 0, e = Buffers.سائيز_2(); i != e; ++i)
    إذا (Loc.get_اشارو() >= Buffers[i].بفر->get_بفر_شروعات() &&
        // Use <= here so that a pointer to the null at the end of the buffer
        // is included as part of the buffer.
        Loc.get_اشارو() <= Buffers[i].بفر->get_بفر_پڇاڙي())
      أرجع i + 1;
  أرجع 0;
}

سانچو <قسم_نالو T>
ساكن std::ويڪٽر<T> &Get_Or_Create_وٿي_Cache(فراغ *&وٿي_Cache,
                                              يادگيري_بفر *بفر) {
  إذا (وٿي_Cache)
    أرجع *static_cast<std::ويڪٽر<T> *>(وٿي_Cache);

  // Lazily fill in the offset cache.
  خودڪار *Offsets = جديد std::ويڪٽر<T>();
  size_t Sz = بفر->get_بفر_سائيز();
  أكد(Sz <= std::numeric_limits<T>::max());
  StringRef S = بفر->get_بفر();
  لكل (size_t N = 0; N < Sz; ++N) {
    إذا (S[N] == '\n')
      Offsets->ڌڪو_پٺ(static_cast<T>(N));
  }

  وٿي_Cache = Offsets;
  أرجع *Offsets;
}

سانچو <قسم_نالو T>
std::pair<بنانشان, بنانشان>
ذريعو_Mgr::Src_بفر::get_سٽ_And_ڪالم_Specialized(ثابت حرف *Ptr) ثابت {
  std::ويڪٽر<T> &Offsets =
      Get_Or_Create_وٿي_Cache<T>(وٿي_Cache, بفر.get());

  ثابت حرف *Buf_شروعات = بفر->get_بفر_شروعات();
  أكد(Ptr >= Buf_شروعات && Ptr <= بفر->get_بفر_پڇاڙي());
  ptrdiff_t PtrDiff = Ptr - Buf_شروعات;
  أكد(PtrDiff >= 0 &&
         static_cast<size_t>(PtrDiff) <= std::numeric_limits<T>::max());
  T Ptr_وٿي = static_cast<T>(PtrDiff);

  // llvm::lower_bound gives the number of EOL before PtrOffset. Add 1 to get
  // the line number.
  خودڪار I = llvm::هيٺاهون_bound(Offsets, Ptr_وٿي);
  بنانشان سٽ_No = I - Offsets.begin() + 1;

  // The column number is the distance from the previous EOL (or the start of
  // the buffer) to the pointer.
  T سٽ_شروعات_Offs = (I == Offsets.begin()) ? 0 : (I[-1] + 1);
  بنانشان کانو_No = Ptr_وٿي - سٽ_شروعات_Offs + 1;
  أرجع {سٽ_No, کانو_No};
}

/// Look up a given \p Ptr in the buffer, determining which line and column
/// it came from.
LLVM_ABI std::pair<بنانشان, بنانشان>
ذريعو_Mgr::Src_بفر::get_سٽ_And_ڪالم(ثابت حرف *Ptr) ثابت {
  size_t Sz = بفر->get_بفر_سائيز();
  إذا (Sz <= std::numeric_limits<uint8_t>::max())
    أرجع get_سٽ_And_ڪالم_Specialized<uint8_t>(Ptr);
  إذا (Sz <= std::numeric_limits<uint16_t>::max())
    أرجع get_سٽ_And_ڪالم_Specialized<uint16_t>(Ptr);
  إذا (Sz <= std::numeric_limits<uint32_t>::max())
    أرجع get_سٽ_And_ڪالم_Specialized<uint32_t>(Ptr);
  أرجع get_سٽ_And_ڪالم_Specialized<uint64_t>(Ptr);
}

سانچو <قسم_نالو T>
ثابت حرف *ذريعو_Mgr::Src_بفر::get_اشارو_For_سٽ_عدد_Specialized(
    بنانشان سٽ_No) ثابت {
  std::ويڪٽر<T> &Offsets =
      Get_Or_Create_وٿي_Cache<T>(وٿي_Cache, بفر.get());

  // We start counting line and column numbers from 1.
  إذا (سٽ_No != 0)
    --سٽ_No;

  ثابت حرف *Buf_شروعات = بفر->get_بفر_شروعات();

  // The offset cache contains the location of the \n for the specified line,
  // we want the start of the line.  As such, we look for the previous entry.
  إذا (سٽ_No == 0)
    أرجع Buf_شروعات;
  إذا (سٽ_No > Offsets.سائيز_2())
    أرجع خالي_اشارو;
  أرجع Buf_شروعات + Offsets[سٽ_No - 1] + 1;
}

/// Return a pointer to the first character of the specified line number or
/// null if the line number is invalid.
ثابت حرف *
ذريعو_Mgr::Src_بفر::get_اشارو_For_سٽ_عدد(بنانشان سٽ_No) ثابت {
  size_t Sz = بفر->get_بفر_سائيز();
  إذا (Sz <= std::numeric_limits<uint8_t>::max())
    أرجع get_اشارو_For_سٽ_عدد_Specialized<uint8_t>(سٽ_No);
  وإلا إذا (Sz <= std::numeric_limits<uint16_t>::max())
    أرجع get_اشارو_For_سٽ_عدد_Specialized<uint16_t>(سٽ_No);
  وإلا إذا (Sz <= std::numeric_limits<uint32_t>::max())
    أرجع get_اشارو_For_سٽ_عدد_Specialized<uint32_t>(سٽ_No);
  وإلا
    أرجع get_اشارو_For_سٽ_عدد_Specialized<uint64_t>(سٽ_No);
}

ذريعو_Mgr::Src_بفر::Src_بفر(ذريعو_Mgr::Src_بفر &&Other)
    : بفر(std::move(Other.بفر)), وٿي_Cache(Other.وٿي_Cache),
      IncludeLoc(Other.IncludeLoc) {
  Other.وٿي_Cache = خالي_اشارو;
}

ذريعو_Mgr::Src_بفر::~Src_بفر() {
  إذا (وٿي_Cache) {
    size_t Sz = بفر->get_بفر_سائيز();
    إذا (Sz <= std::numeric_limits<uint8_t>::max())
      ميٽيو static_cast<std::ويڪٽر<uint8_t> *>(وٿي_Cache);
    وإلا إذا (Sz <= std::numeric_limits<uint16_t>::max())
      ميٽيو static_cast<std::ويڪٽر<uint16_t> *>(وٿي_Cache);
    وإلا إذا (Sz <= std::numeric_limits<uint32_t>::max())
      ميٽيو static_cast<std::ويڪٽر<uint32_t> *>(وٿي_Cache);
    وإلا
      ميٽيو static_cast<std::ويڪٽر<uint64_t> *>(وٿي_Cache);
    وٿي_Cache = خالي_اشارو;
  }
}

std::pair<بنانشان, بنانشان>
ذريعو_Mgr::get_سٽ_And_ڪالم(SMLoc Loc, بنانشان بفر_ID) ثابت {
  إذا (!بفر_ID)
    بفر_ID = ڳوليو_بفر_Containing_Loc(Loc);
  أكد(بفر_ID && "Invalid location!");

  خودڪار &SB = get_بفر_Info(بفر_ID);
  ثابت حرف *Ptr = Loc.get_اشارو();

  أرجع SB.get_سٽ_And_ڪالم(Ptr);
}

// FIXME: Note that the formatting of source locations is spread between
// multiple functions, some in SourceMgr and some in SMDiagnostic. A better
// solution would be a general-purpose source location formatter
// in one of those two classes, or possibly in SMLoc.

/// Get a string with the source location formatted in the standard
/// style, but without the line offset. If \p IncludePath is true, the path
/// is included. If false, only the file name and extension are included.
std::string ذريعو_Mgr::get_Formatted_Location_No_وٿي(SMLoc Loc,
                                                    منطقي Include_رستو) ثابت {
  خودڪار بفر_ID = ڳوليو_بفر_Containing_Loc(Loc);
  أكد(بفر_ID && "Invalid location!");
  خودڪار FileSpec = get_بفر_Info(بفر_ID).بفر->get_بفر_Identifier();

  إذا (Include_رستو) {
    أرجع FileSpec.str() + ":" + std::منزل_string(ڳوليو_سٽ_عدد(Loc, بفر_ID));
  } وإلا {
    خودڪار I = FileSpec.ڳوليو_آخري_جو("/\\");
    I = (I == FileSpec.سائيز_2()) ? 0 : (I + 1);
    أرجع FileSpec.substr(I).str() + ":" +
           std::منزل_string(ڳوليو_سٽ_عدد(Loc, بفر_ID));
  }
}

/// Given a line and column number in a mapped buffer, turn it into an SMLoc.
/// This will return a null SMLoc if the line/column location is invalid.
SMLoc ذريعو_Mgr::ڳوليو_Loc_For_سٽ_And_ڪالم(بنانشان بفر_ID, بنانشان سٽ_No,
                                         بنانشان کانو_No) {
  خودڪار &SB = get_بفر_Info(بفر_ID);
  ثابت حرف *Ptr = SB.get_اشارو_For_سٽ_عدد(سٽ_No);
  إذا (!Ptr)
    أرجع SMLoc();

  // We start counting line and column numbers from 1.
  إذا (کانو_No != 0)
    --کانو_No;

  // If we have a column number, validate it.
  إذا (کانو_No) {
    // Make sure the location is within the current line.
    إذا (Ptr + کانو_No > SB.بفر->get_بفر_پڇاڙي())
      أرجع SMLoc();

    // Make sure there is no newline in the way.
    إذا (StringRef(Ptr, کانو_No).ڳوليو_پهريون_جو("\n\r") != StringRef::npos)
      أرجع SMLoc();

    Ptr += کانو_No;
  }

  أرجع SMLoc::get_From_اشارو(Ptr);
}

فراغ ذريعو_Mgr::Print_Include_ڍير(SMLoc IncludeLoc, raw_ostream &OS) ثابت {
  إذا (IncludeLoc == SMLoc())
    أرجع; // Top of stack.

  بنانشان CurBuf = ڳوليو_بفر_Containing_Loc(IncludeLoc);
  أكد(CurBuf && "Invalid or unspecified location!");

  Print_Include_ڍير(get_بفر_Info(CurBuf).IncludeLoc, OS);

  OS << "Included from " << get_بفر_Info(CurBuf).بفر->get_بفر_Identifier()
     << ":" << ڳوليو_سٽ_عدد(IncludeLoc, CurBuf) << ":\n";
}

فراغ ذريعو_Mgr::print_Include_ڍير_For_Diagnostic(SMLoc Loc,
                                               raw_ostream &OS) ثابت {
  إذا (!Loc.isValid())
    أرجع;
  بنانشان Diag_Cur_بفر = ڳوليو_بفر_Containing_Loc(Loc);
  إذا (Diag_Cur_بفر && Diag_Cur_بفر != getMainFileID()) {
    SMLoc والد_Include_Loc = get_والد_Include_Loc(Diag_Cur_بفر);
    // Ignore macro instantiation buffers to avoid redundant include stacks.
    Print_Include_ڍير(والد_Include_Loc, OS);
  }
}

SMDiagnostic ذريعو_Mgr::Get_پيغام(SMLoc Loc, ذريعو_Mgr::Diag_قسم قسم_2,
                                   ثابت Twine &نياپو_2, ArrayRef<SMRange> Ranges,
                                   ArrayRef<SMFixIt> FixIts) ثابت {
  // First thing to do: find the current buffer containing the specified
  // location to pull out the source line.
  Small_ويڪٽر<std::pair<بنانشان, بنانشان>, 4> کانو_Ranges;
  std::pair<بنانشان, بنانشان> سٽ_And_کانو;
  StringRef بفر_ID = "<unknown>";
  StringRef سٽ_Str;

  إذا (Loc.isValid()) {
    بنانشان CurBuf = ڳوليو_بفر_Containing_Loc(Loc);
    أكد(CurBuf && "Invalid or unspecified location!");

    ثابت يادگيري_بفر *CurMB = get_يادگيري_بفر(CurBuf);
    بفر_ID = CurMB->get_بفر_Identifier();

    // Scan backward to find the start of the line.
    ثابت حرف *سٽ_شروعات = Loc.get_اشارو();
    ثابت حرف *Buf_شروعات = CurMB->get_بفر_شروعات();
    طالما (سٽ_شروعات != Buf_شروعات && سٽ_شروعات[-1] != '\n' &&
           سٽ_شروعات[-1] != '\r')
      --سٽ_شروعات;

    // Get the end of the line.
    ثابت حرف *سٽ_پڇاڙي = Loc.get_اشارو();
    ثابت حرف *Buf_پڇاڙي = CurMB->get_بفر_پڇاڙي();
    طالما (سٽ_پڇاڙي != Buf_پڇاڙي && سٽ_پڇاڙي[0] != '\n' && سٽ_پڇاڙي[0] != '\r')
      ++سٽ_پڇاڙي;
    سٽ_Str = StringRef(سٽ_شروعات, سٽ_پڇاڙي - سٽ_شروعات);

    // Convert any ranges to column ranges that only intersect the line of the
    // location.
    لكل (SMRange R : Ranges) {
      إذا (!R.isValid())
        استمر;

      // If the line doesn't contain any part of the range, then ignore it.
      إذا (R.شروعات.get_اشارو() > سٽ_پڇاڙي || R.پڇاڙي_2.get_اشارو() < سٽ_شروعات)
        استمر;

      // Ignore pieces of the range that go onto other lines.
      إذا (R.شروعات.get_اشارو() < سٽ_شروعات)
        R.شروعات = SMLoc::get_From_اشارو(سٽ_شروعات);
      إذا (R.پڇاڙي_2.get_اشارو() > سٽ_پڇاڙي)
        R.پڇاڙي_2 = SMLoc::get_From_اشارو(سٽ_پڇاڙي);

      // Translate from SMLoc ranges to column ranges.
      // FIXME: Handle multibyte characters.
      کانو_Ranges.ڌڪو_پٺ(std::make_pair(R.شروعات.get_اشارو() - سٽ_شروعات,
                                         R.پڇاڙي_2.get_اشارو() - سٽ_شروعات));
    }

    سٽ_And_کانو = get_سٽ_And_ڪالم(Loc, CurBuf);
  }

  أرجع SMDiagnostic(*هذا, Loc, بفر_ID, سٽ_And_کانو.پهريون_2,
                      سٽ_And_کانو.ٻيو_2 - 1, قسم_2, نياپو_2.str(), سٽ_Str,
                      کانو_Ranges, FixIts);
}

فراغ ذريعو_Mgr::Print_پيغام(raw_ostream &OS, ثابت SMDiagnostic &Diagnostic,
                             منطقي ShowColors) ثابت {
  // Report the message with the diagnostic handler if present.
  إذا (DiagHandler) {
    DiagHandler(Diagnostic, DiagContext);
    أرجع;
  }

  إذا (Diagnostic.getLoc().isValid()) {
    بنانشان CurBuf = ڳوليو_بفر_Containing_Loc(Diagnostic.getLoc());
    أكد(CurBuf && "Invalid or unspecified location!");
    Print_Include_ڍير(get_بفر_Info(CurBuf).IncludeLoc, OS);
  }

  Diagnostic.print(خالي_اشارو, OS, ShowColors);
}

فراغ ذريعو_Mgr::Print_پيغام(raw_ostream &OS, SMLoc Loc,
                             ذريعو_Mgr::Diag_قسم قسم_2, ثابت Twine &نياپو_2,
                             ArrayRef<SMRange> Ranges, ArrayRef<SMFixIt> FixIts,
                             منطقي ShowColors) ثابت {
  Print_پيغام(OS, Get_پيغام(Loc, قسم_2, نياپو_2, Ranges, FixIts), ShowColors);
}

فراغ ذريعو_Mgr::Print_پيغام(SMLoc Loc, ذريعو_Mgr::Diag_قسم قسم_2,
                             ثابت Twine &نياپو_2, ArrayRef<SMRange> Ranges,
                             ArrayRef<SMFixIt> FixIts, منطقي ShowColors) ثابت {
  Print_پيغام(errs(), Loc, قسم_2, نياپو_2, Ranges, FixIts, ShowColors);
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

SMDiagnostic::SMDiagnostic(ثابت ذريعو_Mgr &sm, SMLoc L, StringRef FN, صحيح سٽ,
                           صحيح کانو, ذريعو_Mgr::Diag_قسم قسم_2, StringRef نياپو_2,
                           StringRef سٽ_Str,
                           ArrayRef<std::pair<بنانشان, بنانشان>> Ranges,
                           ArrayRef<SMFixIt> Hints)
    : SM(&sm), Loc(L), Filename(std::string(FN)), سٽ_No(سٽ), ڪالم_No(کانو),
      قسم_2(قسم_2), پيغام_2(نياپو_2), سٽ_Contents(سٽ_Str), Ranges(Ranges.vec()),
      FixIts(Hints) {
  llvm::ترتيب_ڏيو(FixIts);
}

ساكن فراغ build_Fix_It_سٽ(std::string &Caret_سٽ, std::string &Fix_It_سٽ,
                           ArrayRef<SMFixIt> FixIts,
                           ArrayRef<حرف> ذريعو_سٽ) {
  إذا (FixIts.خالي_جايون())
    أرجع;

  ثابت حرف *سٽ_شروعات = ذريعو_سٽ.begin();
  ثابت حرف *سٽ_پڇاڙي = ذريعو_سٽ.پڇاڙي();

  size_t Prev_Hint_پڇاڙي_کانو = 0;

  لكل (ثابت llvm::SMFixIt &Fixit : FixIts) {
    // If the fixit contains a newline or tab, ignore it.
    إذا (Fixit.get_متن().ڳوليو_پهريون_جو("\n\r\t") != StringRef::npos)
      استمر;

    SMRange R = Fixit.getRange();

    // If the line doesn't contain any part of the range, then ignore it.
    إذا (R.شروعات.get_اشارو() > سٽ_پڇاڙي || R.پڇاڙي_2.get_اشارو() < سٽ_شروعات)
      استمر;

    // Translate from SMLoc to column.
    // Ignore pieces of the range that go onto other lines.
    // FIXME: Handle multibyte characters in the source line.
    بنانشان پهريون_کانو;
    إذا (R.شروعات.get_اشارو() < سٽ_شروعات)
      پهريون_کانو = 0;
    وإلا
      پهريون_کانو = R.شروعات.get_اشارو() - سٽ_شروعات;

    // If we inserted a long previous hint, push this one forwards, and add
    // an extra space to show that this is not part of the previous
    // completion. This is sort of the best we can do when two hints appear
    // to overlap.
    //
    // Note that if this hint is located immediately after the previous
    // hint, no space will be added, since the location is more important.
    بنانشان Hint_کانو = پهريون_کانو;
    إذا (Hint_کانو < Prev_Hint_پڇاڙي_کانو)
      Hint_کانو = Prev_Hint_پڇاڙي_کانو + 1;

    // FIXME: This assertion is intended to catch unintended use of multibyte
    // characters in fixits. If we decide to do this, we'll have to track
    // separate byte widths for the source and fixit lines.
    أكد((size_t)sys::locale::ڪالم_ويڪر(Fixit.get_متن()) ==
           Fixit.get_متن().سائيز_2());

    // This relies on one byte per column in our fixit hints.
    بنانشان آخري_ڪالم_Modified = Hint_کانو + Fixit.get_متن().سائيز_2();
    إذا (آخري_ڪالم_Modified > Fix_It_سٽ.سائيز_2())
      Fix_It_سٽ.resize(آخري_ڪالم_Modified, ' ');

    llvm::copy(Fixit.get_متن(), Fix_It_سٽ.begin() + Hint_کانو);

    Prev_Hint_پڇاڙي_کانو = آخري_ڪالم_Modified;

    // For replacements, mark the removal range with '~'.
    // FIXME: Handle multibyte characters in the source line.
    بنانشان آخري_کانو;
    إذا (R.پڇاڙي_2.get_اشارو() >= سٽ_پڇاڙي)
      آخري_کانو = سٽ_پڇاڙي - سٽ_شروعات;
    وإلا
      آخري_کانو = R.پڇاڙي_2.get_اشارو() - سٽ_شروعات;

    std::fill(&Caret_سٽ[پهريون_کانو], &Caret_سٽ[آخري_کانو], '~');
  }
}

ساكن فراغ print_ذريعو_سٽ(raw_ostream &S, StringRef سٽ_Contents) {
  // Print out the source line one character at a time, so we can expand tabs.
  لكل (بنانشان i = 0, e = سٽ_Contents.سائيز_2(), ٻاهريون_کانو = 0; i != e; ++i) {
    size_t ايندڙ_Tab = سٽ_Contents.ڳوليو('\t', i);
    // If there were no tabs left, print the rest, we are done.
    إذا (ايندڙ_Tab == StringRef::npos) {
      S << سٽ_Contents.drop_front(i);
      اكسر;
    }

    // Otherwise, print from i to NextTab.
    S << سٽ_Contents.ٽڪرو(i, ايندڙ_Tab);
    ٻاهريون_کانو += ايندڙ_Tab - i;
    i = ايندڙ_Tab;

    // If we have a tab, emit at least one space, then round up to 8 columns.
    افعل {
      S << ' ';
      ++ٻاهريون_کانو;
    } طالما ((ٻاهريون_کانو % TabStop) != 0);
  }
  S << '\n';
}

ساكن منطقي isNonASCII(حرف c) { أرجع c & 0x80; }

فراغ SMDiagnostic::print(ثابت حرف *ProgName, raw_ostream &OS, منطقي ShowColors,
                         منطقي Show_قسم_Label, منطقي ShowLocation) ثابت {
  Color_گهڻائي گهڻائي = ShowColors ? Color_گهڻائي::Auto : Color_گهڻائي::Disable;

  {
    WithColor S(OS, raw_ostream::SAVEDCOLOR, سچ, خطأ, گهڻائي);

    إذا (ProgName && ProgName[0])
      S << ProgName << ": ";

    إذا (ShowLocation && !Filename.خالي_جايون()) {
      إذا (Filename == "-")
        S << "<stdin>";
      وإلا
        S << Filename;

      إذا (سٽ_No != -1) {
        S << ':' << سٽ_No;
        إذا (ڪالم_No != -1)
          S << ':' << (ڪالم_No + 1);
      }
      S << ": ";
    }
  }

  إذا (Show_قسم_Label) {
    بدل (قسم_2) {
    حالة ذريعو_Mgr::DK_Error:
      WithColor::error(OS, "", !ShowColors);
      اكسر;
    حالة ذريعو_Mgr::DK_Warning:
      WithColor::warning(OS, "", !ShowColors);
      اكسر;
    حالة ذريعو_Mgr::DK_Note:
      WithColor::note(OS, "", !ShowColors);
      اكسر;
    حالة ذريعو_Mgr::DK_Remark:
      WithColor::remark(OS, "", !ShowColors);
      اكسر;
    }
  }

  WithColor(OS, raw_ostream::SAVEDCOLOR, سچ, خطأ, گهڻائي) << پيغام_2 << '\n';

  إذا (سٽ_No == -1 || ڪالم_No == -1)
    أرجع;

  // FIXME: If there are multibyte or multi-column characters in the source, all
  // our ranges will be wrong. To do this properly, we'll need a byte-to-column
  // map like Clang's TextDiagnostic. For now, we'll just handle tabs by
  // expanding them later, and bail out rather than show incorrect ranges and
  // misaligned fixits for any other odd characters.
  إذا (any_جو(سٽ_Contents, isNonASCII)) {
    print_ذريعو_سٽ(OS, سٽ_Contents);
    أرجع;
  }
  size_t انگ_Columns = سٽ_Contents.سائيز_2();

  // Build the line with the caret and ranges.
  std::string Caret_سٽ(انگ_Columns + 1, ' ');

  // Expand any ranges.
  لكل (ثابت std::pair<بنانشان, بنانشان> &R : Ranges)
    std::fill(&Caret_سٽ[R.پهريون_2],
              &Caret_سٽ[std::min((size_t)R.ٻيو_2, Caret_سٽ.سائيز_2())], '~');

  // Add any fix-its.
  // FIXME: Find the beginning of the line properly for multibyte characters.
  std::string Fix_It_Insertion_سٽ;
  build_Fix_It_سٽ(Caret_سٽ, Fix_It_Insertion_سٽ, FixIts,
                 ArrayRef(Loc.get_اشارو() - ڪالم_No, سٽ_Contents.سائيز_2()));

  // Finally, plop on the caret.
  إذا (بنانشان(ڪالم_No) <= انگ_Columns)
    Caret_سٽ[ڪالم_No] = '^';
  وإلا
    Caret_سٽ[انگ_Columns] = '^';

  // ... and remove trailing whitespace so the output doesn't wrap for it.  We
  // know that the line isn't completely empty because it has the caret in it at
  // least.
  Caret_سٽ.erase(Caret_سٽ.ڳوليو_آخري_not_جو(' ') + 1);

  print_ذريعو_سٽ(OS, سٽ_Contents);

  {
    Color_گهڻائي گهڻائي = ShowColors ? Color_گهڻائي::Auto : Color_گهڻائي::Disable;
    WithColor S(OS, raw_ostream::GREEN, سچ, خطأ, گهڻائي);

    // Print out the caret line, matching tabs in the source line.
    لكل (بنانشان i = 0, e = Caret_سٽ.سائيز_2(), ٻاهريون_کانو = 0; i != e; ++i) {
      إذا (i >= سٽ_Contents.سائيز_2() || سٽ_Contents[i] != '\t') {
        S << Caret_سٽ[i];
        ++ٻاهريون_کانو;
        استمر;
      }

      // Okay, we have a tab.  Insert the appropriate number of characters.
      افعل {
        S << Caret_سٽ[i];
        ++ٻاهريون_کانو;
      } طالما ((ٻاهريون_کانو % TabStop) != 0);
    }
    S << '\n';
  }

  // Print out the replacement line, matching tabs in the source line.
  إذا (Fix_It_Insertion_سٽ.خالي_جايون())
    أرجع;

  لكل (size_t i = 0, e = Fix_It_Insertion_سٽ.سائيز_2(), ٻاهريون_کانو = 0; i < e; ++i) {
    إذا (i >= سٽ_Contents.سائيز_2() || سٽ_Contents[i] != '\t') {
      OS << Fix_It_Insertion_سٽ[i];
      ++ٻاهريون_کانو;
      استمر;
    }

    // Okay, we have a tab.  Insert the appropriate number of characters.
    افعل {
      OS << Fix_It_Insertion_سٽ[i];
      // FIXME: This is trying not to break up replacements, but then to re-sync
      // with the tabs between replacements. This will fail, though, if two
      // fix-it replacements are exactly adjacent, or if a fix-it contains a
      // space. Really we should be precomputing column widths, which we'll
      // need anyway for multibyte chars.
      إذا (Fix_It_Insertion_سٽ[i] != ' ')
        ++i;
      ++ٻاهريون_کانو;
    } طالما (((ٻاهريون_کانو % TabStop) != 0) && i != e);
  }
  OS << '\n';
}
