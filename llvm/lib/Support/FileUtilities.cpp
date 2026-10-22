//===- Support/FileUtilities.cpp - File System Utilities ------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file implements a family of utility functions which are useful for doing
// various things with files.
//
//===----------------------------------------------------------------------===//

#ضمن "llvm/Support/FileUtilities.h"
#ضمن "llvm/ADT/SmallString.h"
#ضمن "llvm/ADT/StringExtras.h"
#ضمن "llvm/Support/Error.h"
#ضمن "llvm/Support/ErrorOr.h"
#ضمن "llvm/Support/MemoryBuffer.h"
#ضمن "llvm/Support/Process.h"
#ضمن "llvm/Support/raw_ostream.h"
#ضمن <cmath>
#ضمن <cstdint>
#ضمن <cstdlib>
#ضمن <cstring>
#ضمن <memory>
#ضمن <system_error>

استفاده نطاق llvm;

ساكن منطقي isSignedChar(حرف C) {
  أرجع (C == '+' || C == '-');
}

ساكن منطقي isExponentChar(حرف C) {
  بدل (C) {
  حالة 'D':  // Strange exponential notation.
  حالة 'd':  // Strange exponential notation.
  حالة 'e':
  حالة 'E': أرجع درست;
  افتراضي: أرجع خطأ;
  }
}

ساكن منطقي is_عدد_Char(حرف C) {
  بدل (C) {
  حالة '0': حالة '1': حالة '2': حالة '3': حالة '4':
  حالة '5': حالة '6': حالة '7': حالة '8': حالة '9':
  حالة '.': أرجع درست;
  افتراضي: أرجع isSignedChar(C) || isExponentChar(C);
  }
}

ساكن ثابت حرف *Backup_عدد(ثابت حرف *Pos, ثابت حرف *اولی_Char) {
  // If we didn't stop in the middle of a number, don't backup.
  إذا (!is_عدد_Char(*Pos)) أرجع Pos;

  // Otherwise, return to the start of the number.
  منطقي HasPeriod = خطأ;
  طالما (Pos > اولی_Char && is_عدد_Char(Pos[-1])) {
    // Backup over at most one period.
    إذا (Pos[-1] == '.') {
      إذا (HasPeriod)
        اكسر;
      HasPeriod = درست;
    }

    --Pos;
    إذا (Pos > اولی_Char && isSignedChar(Pos[0]) && !isExponentChar(Pos[-1]))
      اكسر;
  }
  أرجع Pos;
}

/// EndOfNumber - Return the first character that is not part of the specified
/// number.  This assumes that the buffer is null terminated, so it won't fall
/// off the end.
ساكن ثابت حرف *انتها_از_عدد(ثابت حرف *Pos) {
  طالما (is_عدد_Char(*Pos))
    ++Pos;
  أرجع Pos;
}

/// CompareNumbers - compare two numbers, returning true if they are different.
ساكن منطقي CompareNumbers(ثابت حرف *&F1P, ثابت حرف *&F2P,
                           ثابت حرف *F1_انتها, ثابت حرف *F2_انتها,
                           مزدوج AbsTolerance, مزدوج RelTolerance,
                           std::string *Error_پیام) {
  ثابت حرف *F1_رقم_انتها, *F2_رقم_انتها;
  مزدوج V1 = 0.0, V2 = 0.0;

  // If one of the positions is at a space and the other isn't, chomp up 'til
  // the end of the space.
  طالما (isSpace(static_cast<بیعلامت حرف>(*F1P)) && F1P != F1_انتها)
    ++F1P;
  طالما (isSpace(static_cast<بیعلامت حرف>(*F2P)) && F2P != F2_انتها)
    ++F2P;

  // If we stop on numbers, compare their difference.
  إذا (!is_عدد_Char(*F1P) || !is_عدد_Char(*F2P)) {
    // The diff failed.
    F1_رقم_انتها = F1P;
    F2_رقم_انتها = F2P;
  } وإلا {
    // Note that some ugliness is built into this to permit support for numbers
    // that use "D" or "d" as their exponential marker, e.g. "1.234D45".  This
    // occurs in 200.sixtrack in spec2k.
    V1 = strtod(F1P, const_cast<حرف**>(&F1_رقم_انتها));
    V2 = strtod(F2P, const_cast<حرف**>(&F2_رقم_انتها));

    إذا (*F1_رقم_انتها == 'D' || *F1_رقم_انتها == 'd') {
      // Copy string into tmp buffer to replace the 'D' with an 'e'.
      SmallString<200> StrTmp(F1P, انتها_از_عدد(F1_رقم_انتها)+1);
      // Strange exponential notation!
      StrTmp[static_cast<بیعلامت>(F1_رقم_انتها-F1P)] = 'e';

      V1 = strtod(&StrTmp[0], const_cast<حرف**>(&F1_رقم_انتها));
      F1_رقم_انتها = F1P + (F1_رقم_انتها-&StrTmp[0]);
    }

    إذا (*F2_رقم_انتها == 'D' || *F2_رقم_انتها == 'd') {
      // Copy string into tmp buffer to replace the 'D' with an 'e'.
      SmallString<200> StrTmp(F2P, انتها_از_عدد(F2_رقم_انتها)+1);
      // Strange exponential notation!
      StrTmp[static_cast<بیعلامت>(F2_رقم_انتها-F2P)] = 'e';

      V2 = strtod(&StrTmp[0], const_cast<حرف**>(&F2_رقم_انتها));
      F2_رقم_انتها = F2P + (F2_رقم_انتها-&StrTmp[0]);
    }
  }

  إذا (F1_رقم_انتها == F1P || F2_رقم_انتها == F2P) {
    إذا (Error_پیام) {
      *Error_پیام = "FP Comparison failed, not a numeric difference between '";
      *Error_پیام += F1P[0];
      *Error_پیام += "' and '";
      *Error_پیام += F2P[0];
      *Error_پیام += "'";
    }
    أرجع درست;
  }

  // Check to see if these are inside the absolute tolerance
  إذا (AbsTolerance < std::قدر_مطلق(V1-V2)) {
    // Nope, check the relative tolerance...
    مزدوج Diff;
    إذا (V2)
      Diff = std::قدر_مطلق(V1/V2 - 1.0);
    وإلا إذا (V1)
      Diff = std::قدر_مطلق(V2/V1 - 1.0);
    وإلا
      Diff = 0;  // Both zero.
    إذا (Diff > RelTolerance) {
      إذا (Error_پیام) {
        raw_string_ostream(*Error_پیام)
          << "Compared: " << V1 << " and " << V2 << '\n'
          << "abs. diff = " << std::قدر_مطلق(V1-V2) << " rel.diff = " << Diff << '\n'
          << "Out of tolerance: rel/abs: " << RelTolerance << '/'
          << AbsTolerance;
      }
      أرجع درست;
    }
  }

  // Otherwise, advance our read pointers to the end of the numbers.
  F1P = F1_رقم_انتها;  F2P = F2_رقم_انتها;
  أرجع خطأ;
}

/// DiffFilesWithTolerance - Compare the two files specified, returning 0 if the
/// files match, 1 if they are different, and 2 if there is a file error.  This
/// function differs from DiffFiles in that you can specify an absolute and
/// relative FP error that is allowed to exist.  If you specify a string to fill
/// in for the error option, it will set the string to an error message if an
/// error occurs, allowing the caller to distinguish between a failed diff and a
/// file system error.
///
صحيح llvm::DiffFilesWithTolerance(StringRef NameA,
                                 StringRef NameB,
                                 مزدوج AbsTol, مزدوج RelTol,
                                 std::string *Error) {
  // Now its safe to mmap the files into memory because both files
  // have a non-zero size.
  ErrorOr<std::unique_ptr<حافظه_میانگیر>> F1OrErr = حافظه_میانگیر::getFile(NameA);
  إذا (std::error_کد EC = F1OrErr.getError()) {
    إذا (Error)
      *Error = EC.پیام_2();
    أرجع 2;
  }
  حافظه_میانگیر &F1 = *F1OrErr.get();

  ErrorOr<std::unique_ptr<حافظه_میانگیر>> F2OrErr = حافظه_میانگیر::getFile(NameB);
  إذا (std::error_کد EC = F2OrErr.getError()) {
    إذا (Error)
      *Error = EC.پیام_2();
    أرجع 2;
  }
  حافظه_میانگیر &F2 = *F2OrErr.get();

  // Okay, now that we opened the files, scan them for the first difference.
  ثابت حرف *File1_شروع = F1.get_میانگیر_شروع();
  ثابت حرف *File2_شروع = F2.get_میانگیر_شروع();
  ثابت حرف *File1_انتها = F1.get_میانگیر_انتها();
  ثابت حرف *File2_انتها = F2.get_میانگیر_انتها();
  ثابت حرف *F1P = File1_شروع;
  ثابت حرف *F2P = File2_شروع;
  uint64_t A_اندازه = F1.get_میانگیر_اندازه();
  uint64_t B_اندازه = F2.get_میانگیر_اندازه();

  // Are the buffers identical?  Common case: Handle this efficiently.
  إذا (A_اندازه == B_اندازه &&
      std::memcmp(File1_شروع, File2_شروع, A_اندازه) == 0)
    أرجع 0;

  // Otherwise, we are done a tolerances are set.
  إذا (AbsTol == 0 && RelTol == 0) {
    إذا (Error)
      *Error = "Files differ without tolerance allowance";
    أرجع 1;   // Files different!
  }

  منطقي CompareFailed = خطأ;
  طالما (درست) {
    // Scan for the end of file or next difference.
    طالما (F1P < File1_انتها && F2P < File2_انتها && *F1P == *F2P) {
      ++F1P;
      ++F2P;
    }

    إذا (F1P >= File1_انتها || F2P >= File2_انتها) اكسر;

    // Okay, we must have found a difference.  Backup to the start of the
    // current number each stream is at so that we can compare from the
    // beginning.
    F1P = Backup_عدد(F1P, File1_شروع);
    F2P = Backup_عدد(F2P, File2_شروع);

    // Now that we are at the start of the numbers, compare them, exiting if
    // they don't match.
    إذا (CompareNumbers(F1P, F2P, File1_انتها, File2_انتها, AbsTol, RelTol, Error)) {
      CompareFailed = درست;
      اكسر;
    }
  }

  // Okay, we reached the end of file.  If both files are at the end, we
  // succeeded.
  منطقي F1_At_انتها = F1P >= File1_انتها;
  منطقي F2_At_انتها = F2P >= File2_انتها;
  إذا (!CompareFailed && (!F1_At_انتها || !F2_At_انتها)) {
    // Else, we might have run off the end due to a number: backup and retry.
    إذا (F1_At_انتها && is_عدد_Char(F1P[-1])) --F1P;
    إذا (F2_At_انتها && is_عدد_Char(F2P[-1])) --F2P;
    F1P = Backup_عدد(F1P, File1_شروع);
    F2P = Backup_عدد(F2P, File2_شروع);

    // Now that we are at the start of the numbers, compare them, exiting if
    // they don't match.
    إذا (CompareNumbers(F1P, F2P, File1_انتها, File2_انتها, AbsTol, RelTol, Error))
      CompareFailed = درست;

    // If we found the end, we succeeded.
    إذا (F1P < File1_انتها || F2P < File2_انتها)
      CompareFailed = درست;
  }

  أرجع CompareFailed;
}

Expected<FilePermissionsApplier>
FilePermissionsApplier::create(StringRef درون_داد_Filename) {
  sys::fs::file_وضعیت وضعیت;

  إذا (درون_داد_Filename != "-") {
    إذا (خودکار EC = sys::fs::وضعیت_2(درون_داد_Filename, وضعیت))
      أرجع createFileError(درون_داد_Filename, EC);
  } وإلا {
    وضعیت.permissions(static_cast<sys::fs::perms>(0777));
  }

  أرجع FilePermissionsApplier(درون_داد_Filename, وضعیت);
}

Error FilePermissionsApplier::apply(
    StringRef OutputFilename, منطقي CopyDates,
    std::optional<sys::fs::perms> OverwritePermissions) {
  sys::fs::file_وضعیت وضعیت = درون_داد_وضعیت;

  إذا (OverwritePermissions)
    وضعیت.permissions(*OverwritePermissions);

  صحيح FD = 0;

  // Writing to stdout should not be treated as an error here, just
  // do not set access/modification times or permissions.
  إذا (OutputFilename == "-")
    أرجع Error::success();

  إذا (std::error_کد EC = sys::fs::openFileForWrite(OutputFilename, FD,
                                                     sys::fs::CD_OpenExisting))
    أرجع createFileError(OutputFilename, EC);

  إذا (CopyDates)
    إذا (std::error_کد EC = sys::fs::مجموعه_آخرین_Access_And_Modification_زمان(
            FD, وضعیت.get_آخرین_Accessed_زمان(), وضعیت.get_آخرین_Modification_زمان()))
      أرجع createFileError(OutputFilename, EC);

  sys::fs::file_وضعیت OStat;
  إذا (std::error_کد EC =
          sys::fs::وضعیت_2(sys::fs::convertFDToNativeFile(FD), OStat))
    أرجع createFileError(OutputFilename, EC);
  إذا (OStat.type() == sys::fs::file_type::regular_file) {
#اگر_تعریف_نشده _WIN32
    // Keep ownership if llvm-objcopy is called under root.
    إذا (OutputFilename == درون_داد_Filename && OStat.getUser() == 0)
      sys::fs::changeFileOwnership(FD, وضعیت.getUser(), وضعیت.get_گروه());
#پایان_اگر

    sys::fs::perms Perm = وضعیت.permissions();
    إذا (OutputFilename != درون_داد_Filename)
      Perm = static_cast<sys::fs::perms>(Perm & ~sys::fs::getUmask() & ~06000);
#اگر_تعریف_شده _WIN32
    إذا (std::error_کد EC = sys::fs::مجموعه_Permissions(OutputFilename, Perm))
#وإلا
    إذا (std::error_کد EC = sys::fs::مجموعه_Permissions(FD, Perm))
#پایان_اگر
      أرجع createFileError(OutputFilename, EC);
  }

  إذا (std::error_کد EC = sys::Process::SafelyCloseFileDescriptor(FD))
    أرجع createFileError(OutputFilename, EC);

  أرجع Error::success();
}
