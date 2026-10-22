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

#شامل "llvm/Support/FileUtilities.h"
#شامل "llvm/ADT/SmallString.h"
#شامل "llvm/ADT/StringExtras.h"
#شامل "llvm/Support/Error.h"
#شامل "llvm/Support/ErrorOr.h"
#شامل "llvm/Support/MemoryBuffer.h"
#شامل "llvm/Support/Process.h"
#شامل "llvm/Support/raw_ostream.h"
#شامل <cmath>
#شامل <cstdint>
#شامل <cstdlib>
#شامل <cstring>
#شامل <memory>
#شامل <system_error>

استعمال نامخلا llvm;

ساکن منطقی isSignedChar(حرف C) {
  واپس (C == '+' || C == '-');
}

ساکن منطقی isExponentChar(حرف C) {
  انتخاب (C) {
  حالت 'D':  // Strange exponential notation.
  حالت 'd':  // Strange exponential notation.
  حالت 'e':
  حالت 'E': واپس سچ;
  طےشدہ: واپس جھوٹ;
  }
}

ساکن منطقی is_نمبر_Char(حرف C) {
  انتخاب (C) {
  حالت '0': حالت '1': حالت '2': حالت '3': حالت '4':
  حالت '5': حالت '6': حالت '7': حالت '8': حالت '9':
  حالت '.': واپس سچ;
  طےشدہ: واپس isSignedChar(C) || isExponentChar(C);
  }
}

ساکن ثابت حرف *Backup_نمبر(ثابت حرف *Pos, ثابت حرف *پہلا_Char) {
  // If we didn't stop in the middle of a number, don't backup.
  اگر (!is_نمبر_Char(*Pos)) واپس Pos;

  // Otherwise, return to the start of the number.
  منطقی HasPeriod = جھوٹ;
  جب (Pos > پہلا_Char && is_نمبر_Char(Pos[-1])) {
    // Backup over at most one period.
    اگر (Pos[-1] == '.') {
      اگر (HasPeriod)
        توڑو;
      HasPeriod = سچ;
    }

    --Pos;
    اگر (Pos > پہلا_Char && isSignedChar(Pos[0]) && !isExponentChar(Pos[-1]))
      توڑو;
  }
  واپس Pos;
}

/// EndOfNumber - Return the first character that is not part of the specified
/// number.  This assumes that the buffer is null terminated, so it won't fall
/// off the end.
ساکن ثابت حرف *انتہا_کا_نمبر(ثابت حرف *Pos) {
  جب (is_نمبر_Char(*Pos))
    ++Pos;
  واپس Pos;
}

/// CompareNumbers - compare two numbers, returning true if they are different.
ساکن منطقی CompareNumbers(ثابت حرف *&F1P, ثابت حرف *&F2P,
                           ثابت حرف *F1_انتہا, ثابت حرف *F2_انتہا,
                           دوہرا AbsTolerance, دوہرا RelTolerance,
                           std::string *Error_پیغام) {
  ثابت حرف *F1_عدد_انتہا, *F2_عدد_انتہا;
  دوہرا V1 = 0.0, V2 = 0.0;

  // If one of the positions is at a space and the other isn't, chomp up 'til
  // the end of the space.
  جب (isSpace(static_cast<غیرعلامتی حرف>(*F1P)) && F1P != F1_انتہا)
    ++F1P;
  جب (isSpace(static_cast<غیرعلامتی حرف>(*F2P)) && F2P != F2_انتہا)
    ++F2P;

  // If we stop on numbers, compare their difference.
  اگر (!is_نمبر_Char(*F1P) || !is_نمبر_Char(*F2P)) {
    // The diff failed.
    F1_عدد_انتہا = F1P;
    F2_عدد_انتہا = F2P;
  } ورنہ {
    // Note that some ugliness is built into this to permit support for numbers
    // that use "D" or "d" as their exponential marker, e.g. "1.234D45".  This
    // occurs in 200.sixtrack in spec2k.
    V1 = strtod(F1P, const_cast<حرف**>(&F1_عدد_انتہا));
    V2 = strtod(F2P, const_cast<حرف**>(&F2_عدد_انتہا));

    اگر (*F1_عدد_انتہا == 'D' || *F1_عدد_انتہا == 'd') {
      // Copy string into tmp buffer to replace the 'D' with an 'e'.
      SmallString<200> StrTmp(F1P, انتہا_کا_نمبر(F1_عدد_انتہا)+1);
      // Strange exponential notation!
      StrTmp[static_cast<غیرعلامتی>(F1_عدد_انتہا-F1P)] = 'e';

      V1 = strtod(&StrTmp[0], const_cast<حرف**>(&F1_عدد_انتہا));
      F1_عدد_انتہا = F1P + (F1_عدد_انتہا-&StrTmp[0]);
    }

    اگر (*F2_عدد_انتہا == 'D' || *F2_عدد_انتہا == 'd') {
      // Copy string into tmp buffer to replace the 'D' with an 'e'.
      SmallString<200> StrTmp(F2P, انتہا_کا_نمبر(F2_عدد_انتہا)+1);
      // Strange exponential notation!
      StrTmp[static_cast<غیرعلامتی>(F2_عدد_انتہا-F2P)] = 'e';

      V2 = strtod(&StrTmp[0], const_cast<حرف**>(&F2_عدد_انتہا));
      F2_عدد_انتہا = F2P + (F2_عدد_انتہا-&StrTmp[0]);
    }
  }

  اگر (F1_عدد_انتہا == F1P || F2_عدد_انتہا == F2P) {
    اگر (Error_پیغام) {
      *Error_پیغام = "FP Comparison failed, not a numeric difference between '";
      *Error_پیغام += F1P[0];
      *Error_پیغام += "' and '";
      *Error_پیغام += F2P[0];
      *Error_پیغام += "'";
    }
    واپس سچ;
  }

  // Check to see if these are inside the absolute tolerance
  اگر (AbsTolerance < std::مطلق_قدر(V1-V2)) {
    // Nope, check the relative tolerance...
    دوہرا Diff;
    اگر (V2)
      Diff = std::مطلق_قدر(V1/V2 - 1.0);
    ورنہ اگر (V1)
      Diff = std::مطلق_قدر(V2/V1 - 1.0);
    ورنہ
      Diff = 0;  // Both zero.
    اگر (Diff > RelTolerance) {
      اگر (Error_پیغام) {
        raw_string_ostream(*Error_پیغام)
          << "Compared: " << V1 << " and " << V2 << '\n'
          << "abs. diff = " << std::مطلق_قدر(V1-V2) << " rel.diff = " << Diff << '\n'
          << "Out of tolerance: rel/abs: " << RelTolerance << '/'
          << AbsTolerance;
      }
      واپس سچ;
    }
  }

  // Otherwise, advance our read pointers to the end of the numbers.
  F1P = F1_عدد_انتہا;  F2P = F2_عدد_انتہا;
  واپس جھوٹ;
}

/// DiffFilesWithTolerance - Compare the two files specified, returning 0 if the
/// files match, 1 if they are different, and 2 if there is a file error.  This
/// function differs from DiffFiles in that you can specify an absolute and
/// relative FP error that is allowed to exist.  If you specify a string to fill
/// in for the error option, it will set the string to an error message if an
/// error occurs, allowing the caller to distinguish between a failed diff and a
/// file system error.
///
صحیح llvm::DiffFilesWithTolerance(StringRef NameA,
                                 StringRef NameB,
                                 دوہرا AbsTol, دوہرا RelTol,
                                 std::string *Error) {
  // Now its safe to mmap the files into memory because both files
  // have a non-zero size.
  ErrorOr<std::unique_ptr<حافظہ_بفر>> F1OrErr = حافظہ_بفر::getFile(NameA);
  اگر (std::error_رمز EC = F1OrErr.getError()) {
    اگر (Error)
      *Error = EC.پیغام_2();
    واپس 2;
  }
  حافظہ_بفر &F1 = *F1OrErr.get();

  ErrorOr<std::unique_ptr<حافظہ_بفر>> F2OrErr = حافظہ_بفر::getFile(NameB);
  اگر (std::error_رمز EC = F2OrErr.getError()) {
    اگر (Error)
      *Error = EC.پیغام_2();
    واپس 2;
  }
  حافظہ_بفر &F2 = *F2OrErr.get();

  // Okay, now that we opened the files, scan them for the first difference.
  ثابت حرف *File1_آغاز = F1.get_بفر_آغاز();
  ثابت حرف *File2_آغاز = F2.get_بفر_آغاز();
  ثابت حرف *File1_انتہا = F1.get_بفر_انتہا();
  ثابت حرف *File2_انتہا = F2.get_بفر_انتہا();
  ثابت حرف *F1P = File1_آغاز;
  ثابت حرف *F2P = File2_آغاز;
  uint64_t A_سائز = F1.get_بفر_سائز();
  uint64_t B_سائز = F2.get_بفر_سائز();

  // Are the buffers identical?  Common case: Handle this efficiently.
  اگر (A_سائز == B_سائز &&
      std::memcmp(File1_آغاز, File2_آغاز, A_سائز) == 0)
    واپس 0;

  // Otherwise, we are done a tolerances are set.
  اگر (AbsTol == 0 && RelTol == 0) {
    اگر (Error)
      *Error = "Files differ without tolerance allowance";
    واپس 1;   // Files different!
  }

  منطقی CompareFailed = جھوٹ;
  جب (سچ) {
    // Scan for the end of file or next difference.
    جب (F1P < File1_انتہا && F2P < File2_انتہا && *F1P == *F2P) {
      ++F1P;
      ++F2P;
    }

    اگر (F1P >= File1_انتہا || F2P >= File2_انتہا) توڑو;

    // Okay, we must have found a difference.  Backup to the start of the
    // current number each stream is at so that we can compare from the
    // beginning.
    F1P = Backup_نمبر(F1P, File1_آغاز);
    F2P = Backup_نمبر(F2P, File2_آغاز);

    // Now that we are at the start of the numbers, compare them, exiting if
    // they don't match.
    اگر (CompareNumbers(F1P, F2P, File1_انتہا, File2_انتہا, AbsTol, RelTol, Error)) {
      CompareFailed = سچ;
      توڑو;
    }
  }

  // Okay, we reached the end of file.  If both files are at the end, we
  // succeeded.
  منطقی F1_At_انتہا = F1P >= File1_انتہا;
  منطقی F2_At_انتہا = F2P >= File2_انتہا;
  اگر (!CompareFailed && (!F1_At_انتہا || !F2_At_انتہا)) {
    // Else, we might have run off the end due to a number: backup and retry.
    اگر (F1_At_انتہا && is_نمبر_Char(F1P[-1])) --F1P;
    اگر (F2_At_انتہا && is_نمبر_Char(F2P[-1])) --F2P;
    F1P = Backup_نمبر(F1P, File1_آغاز);
    F2P = Backup_نمبر(F2P, File2_آغاز);

    // Now that we are at the start of the numbers, compare them, exiting if
    // they don't match.
    اگر (CompareNumbers(F1P, F2P, File1_انتہا, File2_انتہا, AbsTol, RelTol, Error))
      CompareFailed = سچ;

    // If we found the end, we succeeded.
    اگر (F1P < File1_انتہا || F2P < File2_انتہا)
      CompareFailed = سچ;
  }

  واپس CompareFailed;
}

Expected<FilePermissionsApplier>
FilePermissionsApplier::create(StringRef داخلہ_Filename) {
  sys::fs::file_حالت حالت_2;

  اگر (داخلہ_Filename != "-") {
    اگر (خودکار EC = sys::fs::حالت_3(داخلہ_Filename, حالت_2))
      واپس createFileError(داخلہ_Filename, EC);
  } ورنہ {
    حالت_2.permissions(static_cast<sys::fs::perms>(0777));
  }

  واپس FilePermissionsApplier(داخلہ_Filename, حالت_2);
}

Error FilePermissionsApplier::apply(
    StringRef OutputFilename, منطقی CopyDates,
    std::optional<sys::fs::perms> OverwritePermissions) {
  sys::fs::file_حالت حالت_2 = داخلہ_حالت;

  اگر (OverwritePermissions)
    حالت_2.permissions(*OverwritePermissions);

  صحیح FD = 0;

  // Writing to stdout should not be treated as an error here, just
  // do not set access/modification times or permissions.
  اگر (OutputFilename == "-")
    واپس Error::success();

  اگر (std::error_رمز EC = sys::fs::openFileForWrite(OutputFilename, FD,
                                                     sys::fs::CD_OpenExisting))
    واپس createFileError(OutputFilename, EC);

  اگر (CopyDates)
    اگر (std::error_رمز EC = sys::fs::سیٹ_آخری_Access_And_Modification_وقت(
            FD, حالت_2.get_آخری_Accessed_وقت(), حالت_2.get_آخری_Modification_وقت()))
      واپس createFileError(OutputFilename, EC);

  sys::fs::file_حالت OStat;
  اگر (std::error_رمز EC =
          sys::fs::حالت_3(sys::fs::convertFDToNativeFile(FD), OStat))
    واپس createFileError(OutputFilename, EC);
  اگر (OStat.type() == sys::fs::file_type::regular_file) {
#اگر_غیر_تعریف_شدہ _WIN32
    // Keep ownership if llvm-objcopy is called under root.
    اگر (OutputFilename == داخلہ_Filename && OStat.getUser() == 0)
      sys::fs::changeFileOwnership(FD, حالت_2.getUser(), حالت_2.get_گروہ());
#اگر_ختم

    sys::fs::perms Perm = حالت_2.permissions();
    اگر (OutputFilename != داخلہ_Filename)
      Perm = static_cast<sys::fs::perms>(Perm & ~sys::fs::getUmask() & ~06000);
#اگر_تعریف_شدہ _WIN32
    اگر (std::error_رمز EC = sys::fs::سیٹ_Permissions(OutputFilename, Perm))
#ورنہ
    اگر (std::error_رمز EC = sys::fs::سیٹ_Permissions(FD, Perm))
#اگر_ختم
      واپس createFileError(OutputFilename, EC);
  }

  اگر (std::error_رمز EC = sys::Process::SafelyCloseFileDescriptor(FD))
    واپس createFileError(OutputFilename, EC);

  واپس Error::success();
}
