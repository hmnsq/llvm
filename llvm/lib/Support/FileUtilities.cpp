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

using فضاینام llvm;

ایستا منطقی isSignedChar(نویسه C) {
  بازگشت (C == '+' || C == '-');
}

ایستا منطقی isExponentChar(نویسه C) {
  انتخاب (C) {
  حالت 'D':  // Strange exponential notation.
  حالت 'd':  // Strange exponential notation.
  حالت 'e':
  حالت 'E': بازگشت درست;
  پیشفرض: بازگشت نادرست;
  }
}

ایستا منطقی is_عدد_Char(نویسه C) {
  انتخاب (C) {
  حالت '0': حالت '1': حالت '2': حالت '3': حالت '4':
  حالت '5': حالت '6': حالت '7': حالت '8': حالت '9':
  حالت '.': بازگشت درست;
  پیشفرض: بازگشت isSignedChar(C) || isExponentChar(C);
  }
}

ایستا ثابت نویسه *Backup_عدد(ثابت نویسه *Pos, ثابت نویسه *FirstChar) {
  // If we didn't stop in the middle of a number, don't backup.
  اگر (!is_عدد_Char(*Pos)) بازگشت Pos;

  // Otherwise, return to the start of the number.
  منطقی HasPeriod = نادرست;
  وقتی (Pos > FirstChar && is_عدد_Char(Pos[-1])) {
    // Backup over at most one period.
    اگر (Pos[-1] == '.') {
      اگر (HasPeriod)
        بشکن;
      HasPeriod = درست;
    }

    --Pos;
    اگر (Pos > FirstChar && isSignedChar(Pos[0]) && !isExponentChar(Pos[-1]))
      بشکن;
  }
  بازگشت Pos;
}

/// EndOfNumber - Return the first character that is not part of the specified
/// number.  This assumes that the buffer is null terminated, so it won't fall
/// off the end.
ایستا ثابت نویسه *End_Of_عدد(ثابت نویسه *Pos) {
  وقتی (is_عدد_Char(*Pos))
    ++Pos;
  بازگشت Pos;
}

/// CompareNumbers - compare two numbers, returning true if they are different.
ایستا منطقی CompareNumbers(ثابت نویسه *&F1P, ثابت نویسه *&F2P,
                           ثابت نویسه *F1End, ثابت نویسه *F2End,
                           دوگانه AbsTolerance, دوگانه RelTolerance,
                           std::string *Error_پیام) {
  ثابت نویسه *F1_رقم_End, *F2_رقم_End;
  دوگانه V1 = 0.0, V2 = 0.0;

  // If one of the positions is at a space and the other isn't, chomp up 'til
  // the end of the space.
  وقتی (isSpace(static_cast<unsigned نویسه>(*F1P)) && F1P != F1End)
    ++F1P;
  وقتی (isSpace(static_cast<unsigned نویسه>(*F2P)) && F2P != F2End)
    ++F2P;

  // If we stop on numbers, compare their difference.
  اگر (!is_عدد_Char(*F1P) || !is_عدد_Char(*F2P)) {
    // The diff failed.
    F1_رقم_End = F1P;
    F2_رقم_End = F2P;
  } وگرنه {
    // Note that some ugliness is built into this to permit support for numbers
    // that use "D" or "d" as their exponential marker, e.g. "1.234D45".  This
    // occurs in 200.sixtrack in spec2k.
    V1 = strtod(F1P, const_cast<نویسه**>(&F1_رقم_End));
    V2 = strtod(F2P, const_cast<نویسه**>(&F2_رقم_End));

    اگر (*F1_رقم_End == 'D' || *F1_رقم_End == 'd') {
      // Copy string into tmp buffer to replace the 'D' with an 'e'.
      SmallString<200> StrTmp(F1P, End_Of_عدد(F1_رقم_End)+1);
      // Strange exponential notation!
      StrTmp[static_cast<unsigned>(F1_رقم_End-F1P)] = 'e';

      V1 = strtod(&StrTmp[0], const_cast<نویسه**>(&F1_رقم_End));
      F1_رقم_End = F1P + (F1_رقم_End-&StrTmp[0]);
    }

    اگر (*F2_رقم_End == 'D' || *F2_رقم_End == 'd') {
      // Copy string into tmp buffer to replace the 'D' with an 'e'.
      SmallString<200> StrTmp(F2P, End_Of_عدد(F2_رقم_End)+1);
      // Strange exponential notation!
      StrTmp[static_cast<unsigned>(F2_رقم_End-F2P)] = 'e';

      V2 = strtod(&StrTmp[0], const_cast<نویسه**>(&F2_رقم_End));
      F2_رقم_End = F2P + (F2_رقم_End-&StrTmp[0]);
    }
  }

  اگر (F1_رقم_End == F1P || F2_رقم_End == F2P) {
    اگر (Error_پیام) {
      *Error_پیام = "FP Comparison failed, not a numeric difference between '";
      *Error_پیام += F1P[0];
      *Error_پیام += "' and '";
      *Error_پیام += F2P[0];
      *Error_پیام += "'";
    }
    بازگشت درست;
  }

  // Check to see if these are inside the absolute tolerance
  اگر (AbsTolerance < std::abs(V1-V2)) {
    // Nope, check the relative tolerance...
    دوگانه Diff;
    اگر (V2)
      Diff = std::abs(V1/V2 - 1.0);
    وگرنه اگر (V1)
      Diff = std::abs(V2/V1 - 1.0);
    وگرنه
      Diff = 0;  // Both zero.
    اگر (Diff > RelTolerance) {
      اگر (Error_پیام) {
        raw_string_ostream(*Error_پیام)
          << "Compared: " << V1 << " and " << V2 << '\n'
          << "abs. diff = " << std::abs(V1-V2) << " rel.diff = " << Diff << '\n'
          << "Out of tolerance: rel/abs: " << RelTolerance << '/'
          << AbsTolerance;
      }
      بازگشت درست;
    }
  }

  // Otherwise, advance our read pointers to the end of the numbers.
  F1P = F1_رقم_End;  F2P = F2_رقم_End;
  بازگشت نادرست;
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
                                 دوگانه AbsTol, دوگانه RelTol,
                                 std::string *Error) {
  // Now its safe to mmap the files into memory because both files
  // have a non-zero size.
  ErrorOr<std::unique_ptr<MemoryBuffer>> F1OrErr = MemoryBuffer::getFile(NameA);
  اگر (std::error_code EC = F1OrErr.getError()) {
    اگر (Error)
      *Error = EC.message();
    بازگشت 2;
  }
  MemoryBuffer &F1 = *F1OrErr.get();

  ErrorOr<std::unique_ptr<MemoryBuffer>> F2OrErr = MemoryBuffer::getFile(NameB);
  اگر (std::error_code EC = F2OrErr.getError()) {
    اگر (Error)
      *Error = EC.message();
    بازگشت 2;
  }
  MemoryBuffer &F2 = *F2OrErr.get();

  // Okay, now that we opened the files, scan them for the first difference.
  ثابت نویسه *File1Start = F1.getBufferStart();
  ثابت نویسه *File2Start = F2.getBufferStart();
  ثابت نویسه *File1End = F1.getBufferEnd();
  ثابت نویسه *File2End = F2.getBufferEnd();
  ثابت نویسه *F1P = File1Start;
  ثابت نویسه *F2P = File2Start;
  uint64_t A_size = F1.getBufferSize();
  uint64_t B_size = F2.getBufferSize();

  // Are the buffers identical?  Common case: Handle this efficiently.
  اگر (A_size == B_size &&
      std::memcmp(File1Start, File2Start, A_size) == 0)
    بازگشت 0;

  // Otherwise, we are done a tolerances are set.
  اگر (AbsTol == 0 && RelTol == 0) {
    اگر (Error)
      *Error = "Files differ without tolerance allowance";
    بازگشت 1;   // Files different!
  }

  منطقی CompareFailed = نادرست;
  وقتی (درست) {
    // Scan for the end of file or next difference.
    وقتی (F1P < File1End && F2P < File2End && *F1P == *F2P) {
      ++F1P;
      ++F2P;
    }

    اگر (F1P >= File1End || F2P >= File2End) بشکن;

    // Okay, we must have found a difference.  Backup to the start of the
    // current number each stream is at so that we can compare from the
    // beginning.
    F1P = Backup_عدد(F1P, File1Start);
    F2P = Backup_عدد(F2P, File2Start);

    // Now that we are at the start of the numbers, compare them, exiting if
    // they don't match.
    اگر (CompareNumbers(F1P, F2P, File1End, File2End, AbsTol, RelTol, Error)) {
      CompareFailed = درست;
      بشکن;
    }
  }

  // Okay, we reached the end of file.  If both files are at the end, we
  // succeeded.
  منطقی F1AtEnd = F1P >= File1End;
  منطقی F2AtEnd = F2P >= File2End;
  اگر (!CompareFailed && (!F1AtEnd || !F2AtEnd)) {
    // Else, we might have run off the end due to a number: backup and retry.
    اگر (F1AtEnd && is_عدد_Char(F1P[-1])) --F1P;
    اگر (F2AtEnd && is_عدد_Char(F2P[-1])) --F2P;
    F1P = Backup_عدد(F1P, File1Start);
    F2P = Backup_عدد(F2P, File2Start);

    // Now that we are at the start of the numbers, compare them, exiting if
    // they don't match.
    اگر (CompareNumbers(F1P, F2P, File1End, File2End, AbsTol, RelTol, Error))
      CompareFailed = درست;

    // If we found the end, we succeeded.
    اگر (F1P < File1End || F2P < File2End)
      CompareFailed = درست;
  }

  بازگشت CompareFailed;
}

Expected<FilePermissionsApplier>
FilePermissionsApplier::create(StringRef درون_داد_Filename) {
  sys::fs::file_status Status;

  اگر (درون_داد_Filename != "-") {
    اگر (auto EC = sys::fs::status(درون_داد_Filename, Status))
      بازگشت createFileError(درون_داد_Filename, EC);
  } وگرنه {
    Status.permissions(static_cast<sys::fs::perms>(0777));
  }

  بازگشت FilePermissionsApplier(درون_داد_Filename, Status);
}

Error FilePermissionsApplier::apply(
    StringRef OutputFilename, منطقی CopyDates,
    std::optional<sys::fs::perms> OverwritePermissions) {
  sys::fs::file_status Status = درون_داد_Status;

  اگر (OverwritePermissions)
    Status.permissions(*OverwritePermissions);

  صحیح FD = 0;

  // Writing to stdout should not be treated as an error here, just
  // do not set access/modification times or permissions.
  اگر (OutputFilename == "-")
    بازگشت Error::success();

  اگر (std::error_code EC = sys::fs::openFileForWrite(OutputFilename, FD,
                                                     sys::fs::CD_OpenExisting))
    بازگشت createFileError(OutputFilename, EC);

  اگر (CopyDates)
    اگر (std::error_code EC = sys::fs::setLastAccessAndModificationTime(
            FD, Status.getLastAccessedTime(), Status.getLastModificationTime()))
      بازگشت createFileError(OutputFilename, EC);

  sys::fs::file_status OStat;
  اگر (std::error_code EC =
          sys::fs::status(sys::fs::convertFDToNativeFile(FD), OStat))
    بازگشت createFileError(OutputFilename, EC);
  اگر (OStat.type() == sys::fs::file_type::regular_file) {
#ifndef _WIN32
    // Keep ownership if llvm-objcopy is called under root.
    اگر (OutputFilename == درون_داد_Filename && OStat.getUser() == 0)
      sys::fs::changeFileOwnership(FD, Status.getUser(), Status.getGroup());
#endif

    sys::fs::perms Perm = Status.permissions();
    اگر (OutputFilename != درون_داد_Filename)
      Perm = static_cast<sys::fs::perms>(Perm & ~sys::fs::getUmask() & ~06000);
#ifdef _WIN32
    اگر (std::error_code EC = sys::fs::setPermissions(OutputFilename, Perm))
#وگرنه
    اگر (std::error_code EC = sys::fs::setPermissions(FD, Perm))
#endif
      بازگشت createFileError(OutputFilename, EC);
  }

  اگر (std::error_code EC = sys::Process::SafelyCloseFileDescriptor(FD))
    بازگشت createFileError(OutputFilename, EC);

  بازگشت Error::success();
}
