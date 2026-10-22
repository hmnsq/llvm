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
  החזר (C == '+' || C == '-');
}

ساكن منطقي isExponentChar(حرف C) {
  بدل (C) {
  حالة 'D':  // Strange exponential notation.
  حالة 'd':  // Strange exponential notation.
  حالة 'e':
  حالة 'E': החזר אמת;
  افتراضي: החזר שקר;
  }
}

ساكن منطقي isNumberChar(حرف C) {
  بدل (C) {
  حالة '0': حالة '1': حالة '2': حالة '3': حالة '4':
  حالة '5': حالة '6': حالة '7': حالة '8': حالة '9':
  حالة '.': החזר אמת;
  افتراضي: החזר isSignedChar(C) || isExponentChar(C);
  }
}

ساكن ثابت حرف *BackupNumber(ثابت حرف *Pos, ثابت حرف *FirstChar) {
  // If we didn't stop in the middle of a number, don't backup.
  אם (!isNumberChar(*Pos)) החזר Pos;

  // Otherwise, return to the start of the number.
  منطقي HasPeriod = שקר;
  כלעוד (Pos > FirstChar && isNumberChar(Pos[-1])) {
    // Backup over at most one period.
    אם (Pos[-1] == '.') {
      אם (HasPeriod)
        اكسر;
      HasPeriod = אמת;
    }

    --Pos;
    אם (Pos > FirstChar && isSignedChar(Pos[0]) && !isExponentChar(Pos[-1]))
      اكسر;
  }
  החזר Pos;
}

/// EndOfNumber - Return the first character that is not part of the specified
/// number.  This assumes that the buffer is null terminated, so it won't fall
/// off the end.
ساكن ثابت حرف *EndOfNumber(ثابت حرف *Pos) {
  כלעוד (isNumberChar(*Pos))
    ++Pos;
  החזר Pos;
}

/// CompareNumbers - compare two numbers, returning true if they are different.
ساكن منطقي CompareNumbers(ثابت حرف *&F1P, ثابت حرف *&F2P,
                           ثابت حرف *F1End, ثابت حرف *F2End,
                           مزدوج AbsTolerance, مزدوج RelTolerance,
                           std::string *ErrorMsg) {
  ثابت حرف *F1NumEnd, *F2NumEnd;
  مزدوج V1 = 0.0, V2 = 0.0;

  // If one of the positions is at a space and the other isn't, chomp up 'til
  // the end of the space.
  כלעוד (isSpace(static_cast<unsigned حرف>(*F1P)) && F1P != F1End)
    ++F1P;
  כלעוד (isSpace(static_cast<unsigned حرف>(*F2P)) && F2P != F2End)
    ++F2P;

  // If we stop on numbers, compare their difference.
  אם (!isNumberChar(*F1P) || !isNumberChar(*F2P)) {
    // The diff failed.
    F1NumEnd = F1P;
    F2NumEnd = F2P;
  } אחרת {
    // Note that some ugliness is built into this to permit support for numbers
    // that use "D" or "d" as their exponential marker, e.g. "1.234D45".  This
    // occurs in 200.sixtrack in spec2k.
    V1 = strtod(F1P, const_cast<حرف**>(&F1NumEnd));
    V2 = strtod(F2P, const_cast<حرف**>(&F2NumEnd));

    אם (*F1NumEnd == 'D' || *F1NumEnd == 'd') {
      // Copy string into tmp buffer to replace the 'D' with an 'e'.
      SmallString<200> StrTmp(F1P, EndOfNumber(F1NumEnd)+1);
      // Strange exponential notation!
      StrTmp[static_cast<unsigned>(F1NumEnd-F1P)] = 'e';

      V1 = strtod(&StrTmp[0], const_cast<حرف**>(&F1NumEnd));
      F1NumEnd = F1P + (F1NumEnd-&StrTmp[0]);
    }

    אם (*F2NumEnd == 'D' || *F2NumEnd == 'd') {
      // Copy string into tmp buffer to replace the 'D' with an 'e'.
      SmallString<200> StrTmp(F2P, EndOfNumber(F2NumEnd)+1);
      // Strange exponential notation!
      StrTmp[static_cast<unsigned>(F2NumEnd-F2P)] = 'e';

      V2 = strtod(&StrTmp[0], const_cast<حرف**>(&F2NumEnd));
      F2NumEnd = F2P + (F2NumEnd-&StrTmp[0]);
    }
  }

  אם (F1NumEnd == F1P || F2NumEnd == F2P) {
    אם (ErrorMsg) {
      *ErrorMsg = "FP Comparison failed, not a numeric difference between '";
      *ErrorMsg += F1P[0];
      *ErrorMsg += "' and '";
      *ErrorMsg += F2P[0];
      *ErrorMsg += "'";
    }
    החזר אמת;
  }

  // Check to see if these are inside the absolute tolerance
  אם (AbsTolerance < std::abs(V1-V2)) {
    // Nope, check the relative tolerance...
    مزدوج Diff;
    אם (V2)
      Diff = std::abs(V1/V2 - 1.0);
    אחרת אם (V1)
      Diff = std::abs(V2/V1 - 1.0);
    אחרת
      Diff = 0;  // Both zero.
    אם (Diff > RelTolerance) {
      אם (ErrorMsg) {
        raw_string_ostream(*ErrorMsg)
          << "Compared: " << V1 << " and " << V2 << '\n'
          << "abs. diff = " << std::abs(V1-V2) << " rel.diff = " << Diff << '\n'
          << "Out of tolerance: rel/abs: " << RelTolerance << '/'
          << AbsTolerance;
      }
      החזר אמת;
    }
  }

  // Otherwise, advance our read pointers to the end of the numbers.
  F1P = F1NumEnd;  F2P = F2NumEnd;
  החזר שקר;
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
  אם (std::error_code EC = F1OrErr.getError()) {
    אם (Error)
      *Error = EC.message();
    החזר 2;
  }
  MemoryBuffer &F1 = *F1OrErr.get();

  ErrorOr<std::unique_ptr<MemoryBuffer>> F2OrErr = MemoryBuffer::getFile(NameB);
  אם (std::error_code EC = F2OrErr.getError()) {
    אם (Error)
      *Error = EC.message();
    החזר 2;
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
  אם (A_size == B_size &&
      std::memcmp(File1Start, File2Start, A_size) == 0)
    החזר 0;

  // Otherwise, we are done a tolerances are set.
  אם (AbsTol == 0 && RelTol == 0) {
    אם (Error)
      *Error = "Files differ without tolerance allowance";
    החזר 1;   // Files different!
  }

  منطقي CompareFailed = שקר;
  כלעוד (אמת) {
    // Scan for the end of file or next difference.
    כלעוד (F1P < File1End && F2P < File2End && *F1P == *F2P) {
      ++F1P;
      ++F2P;
    }

    אם (F1P >= File1End || F2P >= File2End) اكسر;

    // Okay, we must have found a difference.  Backup to the start of the
    // current number each stream is at so that we can compare from the
    // beginning.
    F1P = BackupNumber(F1P, File1Start);
    F2P = BackupNumber(F2P, File2Start);

    // Now that we are at the start of the numbers, compare them, exiting if
    // they don't match.
    אם (CompareNumbers(F1P, F2P, File1End, File2End, AbsTol, RelTol, Error)) {
      CompareFailed = אמת;
      اكسر;
    }
  }

  // Okay, we reached the end of file.  If both files are at the end, we
  // succeeded.
  منطقي F1AtEnd = F1P >= File1End;
  منطقي F2AtEnd = F2P >= File2End;
  אם (!CompareFailed && (!F1AtEnd || !F2AtEnd)) {
    // Else, we might have run off the end due to a number: backup and retry.
    אם (F1AtEnd && isNumberChar(F1P[-1])) --F1P;
    אם (F2AtEnd && isNumberChar(F2P[-1])) --F2P;
    F1P = BackupNumber(F1P, File1Start);
    F2P = BackupNumber(F2P, File2Start);

    // Now that we are at the start of the numbers, compare them, exiting if
    // they don't match.
    אם (CompareNumbers(F1P, F2P, File1End, File2End, AbsTol, RelTol, Error))
      CompareFailed = אמת;

    // If we found the end, we succeeded.
    אם (F1P < File1End || F2P < File2End)
      CompareFailed = אמת;
  }

  החזר CompareFailed;
}

Expected<FilePermissionsApplier>
FilePermissionsApplier::create(StringRef InputFilename) {
  sys::fs::file_status Status;

  אם (InputFilename != "-") {
    אם (auto EC = sys::fs::status(InputFilename, Status))
      החזר createFileError(InputFilename, EC);
  } אחרת {
    Status.permissions(static_cast<sys::fs::perms>(0777));
  }

  החזר FilePermissionsApplier(InputFilename, Status);
}

Error FilePermissionsApplier::apply(
    StringRef OutputFilename, منطقي CopyDates,
    std::optional<sys::fs::perms> OverwritePermissions) {
  sys::fs::file_status Status = InputStatus;

  אם (OverwritePermissions)
    Status.permissions(*OverwritePermissions);

  صحيح FD = 0;

  // Writing to stdout should not be treated as an error here, just
  // do not set access/modification times or permissions.
  אם (OutputFilename == "-")
    החזר Error::success();

  אם (std::error_code EC = sys::fs::openFileForWrite(OutputFilename, FD,
                                                     sys::fs::CD_OpenExisting))
    החזר createFileError(OutputFilename, EC);

  אם (CopyDates)
    אם (std::error_code EC = sys::fs::setLastAccessAndModificationTime(
            FD, Status.getLastAccessedTime(), Status.getLastModificationTime()))
      החזר createFileError(OutputFilename, EC);

  sys::fs::file_status OStat;
  אם (std::error_code EC =
          sys::fs::status(sys::fs::convertFDToNativeFile(FD), OStat))
    החזר createFileError(OutputFilename, EC);
  אם (OStat.type() == sys::fs::file_type::regular_file) {
#ifndef _WIN32
    // Keep ownership if llvm-objcopy is called under root.
    אם (OutputFilename == InputFilename && OStat.getUser() == 0)
      sys::fs::changeFileOwnership(FD, Status.getUser(), Status.getGroup());
#endif

    sys::fs::perms Perm = Status.permissions();
    אם (OutputFilename != InputFilename)
      Perm = static_cast<sys::fs::perms>(Perm & ~sys::fs::getUmask() & ~06000);
#ifdef _WIN32
    אם (std::error_code EC = sys::fs::setPermissions(OutputFilename, Perm))
#אחרת
    אם (std::error_code EC = sys::fs::setPermissions(FD, Perm))
#endif
      החזר createFileError(OutputFilename, EC);
  }

  אם (std::error_code EC = sys::Process::SafelyCloseFileDescriptor(FD))
    החזר createFileError(OutputFilename, EC);

  החזר Error::success();
}
