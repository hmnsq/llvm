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

using نطاق llvm;

ساكن منطقي isSignedChar(حرف C) {
  أرجع (C == '+' || C == '-');
}

ساكن منطقي isExponentChar(حرف C) {
  بدل (C) {
  حالة 'D':  // Strange exponential notation.
  حالة 'd':  // Strange exponential notation.
  حالة 'e':
  حالة 'E': أرجع true;
  افتراضي: أرجع خطأ;
  }
}

ساكن منطقي is_رقم_Char(حرف C) {
  بدل (C) {
  حالة '0': حالة '1': حالة '2': حالة '3': حالة '4':
  حالة '5': حالة '6': حالة '7': حالة '8': حالة '9':
  حالة '.': أرجع true;
  افتراضي: أرجع isSignedChar(C) || isExponentChar(C);
  }
}

ساكن ثابت حرف *Backup_رقم(ثابت حرف *Pos, ثابت حرف *FirstChar) {
  // If we didn't stop in the middle of a number, don't backup.
  إذا (!is_رقم_Char(*Pos)) أرجع Pos;

  // Otherwise, return to the start of the number.
  منطقي HasPeriod = خطأ;
  طالما (Pos > FirstChar && is_رقم_Char(Pos[-1])) {
    // Backup over at most one period.
    إذا (Pos[-1] == '.') {
      إذا (HasPeriod)
        اكسر;
      HasPeriod = true;
    }

    --Pos;
    إذا (Pos > FirstChar && isSignedChar(Pos[0]) && !isExponentChar(Pos[-1]))
      اكسر;
  }
  أرجع Pos;
}

/// EndOfNumber - Return the first character that is not part of the specified
/// number.  This assumes that the buffer is null terminated, so it won't fall
/// off the end.
ساكن ثابت حرف *End_Of_رقم(ثابت حرف *Pos) {
  طالما (is_رقم_Char(*Pos))
    ++Pos;
  أرجع Pos;
}

/// CompareNumbers - compare two numbers, returning true if they are different.
ساكن منطقي CompareNumbers(ثابت حرف *&F1P, ثابت حرف *&F2P,
                           ثابت حرف *F1End, ثابت حرف *F2End,
                           مزدوج AbsTolerance, مزدوج RelTolerance,
                           std::string *Error_رسالة) {
  ثابت حرف *F1_رقم_End, *F2_رقم_End;
  مزدوج V1 = 0.0, V2 = 0.0;

  // If one of the positions is at a space and the other isn't, chomp up 'til
  // the end of the space.
  طالما (isSpace(static_cast<unsigned حرف>(*F1P)) && F1P != F1End)
    ++F1P;
  طالما (isSpace(static_cast<unsigned حرف>(*F2P)) && F2P != F2End)
    ++F2P;

  // If we stop on numbers, compare their difference.
  إذا (!is_رقم_Char(*F1P) || !is_رقم_Char(*F2P)) {
    // The diff failed.
    F1_رقم_End = F1P;
    F2_رقم_End = F2P;
  } وإلا {
    // Note that some ugliness is built into this to permit support for numbers
    // that use "D" or "d" as their exponential marker, e.g. "1.234D45".  This
    // occurs in 200.sixtrack in spec2k.
    V1 = strtod(F1P, const_cast<حرف**>(&F1_رقم_End));
    V2 = strtod(F2P, const_cast<حرف**>(&F2_رقم_End));

    إذا (*F1_رقم_End == 'D' || *F1_رقم_End == 'd') {
      // Copy string into tmp buffer to replace the 'D' with an 'e'.
      SmallString<200> StrTmp(F1P, End_Of_رقم(F1_رقم_End)+1);
      // Strange exponential notation!
      StrTmp[static_cast<unsigned>(F1_رقم_End-F1P)] = 'e';

      V1 = strtod(&StrTmp[0], const_cast<حرف**>(&F1_رقم_End));
      F1_رقم_End = F1P + (F1_رقم_End-&StrTmp[0]);
    }

    إذا (*F2_رقم_End == 'D' || *F2_رقم_End == 'd') {
      // Copy string into tmp buffer to replace the 'D' with an 'e'.
      SmallString<200> StrTmp(F2P, End_Of_رقم(F2_رقم_End)+1);
      // Strange exponential notation!
      StrTmp[static_cast<unsigned>(F2_رقم_End-F2P)] = 'e';

      V2 = strtod(&StrTmp[0], const_cast<حرف**>(&F2_رقم_End));
      F2_رقم_End = F2P + (F2_رقم_End-&StrTmp[0]);
    }
  }

  إذا (F1_رقم_End == F1P || F2_رقم_End == F2P) {
    إذا (Error_رسالة) {
      *Error_رسالة = "FP Comparison failed, not a numeric difference between '";
      *Error_رسالة += F1P[0];
      *Error_رسالة += "' and '";
      *Error_رسالة += F2P[0];
      *Error_رسالة += "'";
    }
    أرجع true;
  }

  // Check to see if these are inside the absolute tolerance
  إذا (AbsTolerance < std::abs(V1-V2)) {
    // Nope, check the relative tolerance...
    مزدوج Diff;
    إذا (V2)
      Diff = std::abs(V1/V2 - 1.0);
    وإلا إذا (V1)
      Diff = std::abs(V2/V1 - 1.0);
    وإلا
      Diff = 0;  // Both zero.
    إذا (Diff > RelTolerance) {
      إذا (Error_رسالة) {
        raw_string_ostream(*Error_رسالة)
          << "Compared: " << V1 << " and " << V2 << '\n'
          << "abs. diff = " << std::abs(V1-V2) << " rel.diff = " << Diff << '\n'
          << "Out of tolerance: rel/abs: " << RelTolerance << '/'
          << AbsTolerance;
      }
      أرجع true;
    }
  }

  // Otherwise, advance our read pointers to the end of the numbers.
  F1P = F1_رقم_End;  F2P = F2_رقم_End;
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
  ErrorOr<std::unique_ptr<MemoryBuffer>> F1OrErr = MemoryBuffer::getFile(NameA);
  إذا (std::error_code EC = F1OrErr.getError()) {
    إذا (Error)
      *Error = EC.message();
    أرجع 2;
  }
  MemoryBuffer &F1 = *F1OrErr.get();

  ErrorOr<std::unique_ptr<MemoryBuffer>> F2OrErr = MemoryBuffer::getFile(NameB);
  إذا (std::error_code EC = F2OrErr.getError()) {
    إذا (Error)
      *Error = EC.message();
    أرجع 2;
  }
  MemoryBuffer &F2 = *F2OrErr.get();

  // Okay, now that we opened the files, scan them for the first difference.
  ثابت حرف *File1Start = F1.getBufferStart();
  ثابت حرف *File2Start = F2.getBufferStart();
  ثابت حرف *File1End = F1.getBufferEnd();
  ثابت حرف *File2End = F2.getBufferEnd();
  ثابت حرف *F1P = File1Start;
  ثابت حرف *F2P = File2Start;
  uint64_t A_size = F1.getBufferSize();
  uint64_t B_size = F2.getBufferSize();

  // Are the buffers identical?  Common case: Handle this efficiently.
  إذا (A_size == B_size &&
      std::memcmp(File1Start, File2Start, A_size) == 0)
    أرجع 0;

  // Otherwise, we are done a tolerances are set.
  إذا (AbsTol == 0 && RelTol == 0) {
    إذا (Error)
      *Error = "Files differ without tolerance allowance";
    أرجع 1;   // Files different!
  }

  منطقي CompareFailed = خطأ;
  طالما (true) {
    // Scan for the end of file or next difference.
    طالما (F1P < File1End && F2P < File2End && *F1P == *F2P) {
      ++F1P;
      ++F2P;
    }

    إذا (F1P >= File1End || F2P >= File2End) اكسر;

    // Okay, we must have found a difference.  Backup to the start of the
    // current number each stream is at so that we can compare from the
    // beginning.
    F1P = Backup_رقم(F1P, File1Start);
    F2P = Backup_رقم(F2P, File2Start);

    // Now that we are at the start of the numbers, compare them, exiting if
    // they don't match.
    إذا (CompareNumbers(F1P, F2P, File1End, File2End, AbsTol, RelTol, Error)) {
      CompareFailed = true;
      اكسر;
    }
  }

  // Okay, we reached the end of file.  If both files are at the end, we
  // succeeded.
  منطقي F1AtEnd = F1P >= File1End;
  منطقي F2AtEnd = F2P >= File2End;
  إذا (!CompareFailed && (!F1AtEnd || !F2AtEnd)) {
    // Else, we might have run off the end due to a number: backup and retry.
    إذا (F1AtEnd && is_رقم_Char(F1P[-1])) --F1P;
    إذا (F2AtEnd && is_رقم_Char(F2P[-1])) --F2P;
    F1P = Backup_رقم(F1P, File1Start);
    F2P = Backup_رقم(F2P, File2Start);

    // Now that we are at the start of the numbers, compare them, exiting if
    // they don't match.
    إذا (CompareNumbers(F1P, F2P, File1End, File2End, AbsTol, RelTol, Error))
      CompareFailed = true;

    // If we found the end, we succeeded.
    إذا (F1P < File1End || F2P < File2End)
      CompareFailed = true;
  }

  أرجع CompareFailed;
}

Expected<FilePermissionsApplier>
FilePermissionsApplier::create(StringRef مدخل_Filename) {
  sys::fs::file_status Status;

  إذا (مدخل_Filename != "-") {
    إذا (auto EC = sys::fs::status(مدخل_Filename, Status))
      أرجع createFileError(مدخل_Filename, EC);
  } وإلا {
    Status.permissions(static_cast<sys::fs::perms>(0777));
  }

  أرجع FilePermissionsApplier(مدخل_Filename, Status);
}

Error FilePermissionsApplier::apply(
    StringRef OutputFilename, منطقي CopyDates,
    std::optional<sys::fs::perms> OverwritePermissions) {
  sys::fs::file_status Status = مدخل_Status;

  إذا (OverwritePermissions)
    Status.permissions(*OverwritePermissions);

  صحيح FD = 0;

  // Writing to stdout should not be treated as an error here, just
  // do not set access/modification times or permissions.
  إذا (OutputFilename == "-")
    أرجع Error::success();

  إذا (std::error_code EC = sys::fs::openFileForWrite(OutputFilename, FD,
                                                     sys::fs::CD_OpenExisting))
    أرجع createFileError(OutputFilename, EC);

  إذا (CopyDates)
    إذا (std::error_code EC = sys::fs::setLastAccessAndModificationTime(
            FD, Status.getLastAccessedTime(), Status.getLastModificationTime()))
      أرجع createFileError(OutputFilename, EC);

  sys::fs::file_status OStat;
  إذا (std::error_code EC =
          sys::fs::status(sys::fs::convertFDToNativeFile(FD), OStat))
    أرجع createFileError(OutputFilename, EC);
  إذا (OStat.type() == sys::fs::file_type::regular_file) {
#ifndef _WIN32
    // Keep ownership if llvm-objcopy is called under root.
    إذا (OutputFilename == مدخل_Filename && OStat.getUser() == 0)
      sys::fs::changeFileOwnership(FD, Status.getUser(), Status.getGroup());
#endif

    sys::fs::perms Perm = Status.permissions();
    إذا (OutputFilename != مدخل_Filename)
      Perm = static_cast<sys::fs::perms>(Perm & ~sys::fs::getUmask() & ~06000);
#ifdef _WIN32
    إذا (std::error_code EC = sys::fs::setPermissions(OutputFilename, Perm))
#وإلا
    إذا (std::error_code EC = sys::fs::setPermissions(FD, Perm))
#endif
      أرجع createFileError(OutputFilename, EC);
  }

  إذا (std::error_code EC = sys::Process::SafelyCloseFileDescriptor(FD))
    أرجع createFileError(OutputFilename, EC);

  أرجع Error::success();
}
