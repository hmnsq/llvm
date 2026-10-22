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

#include "llvm/Support/FileUtilities.h"
#include "llvm/ADT/SmallString.h"
#include "llvm/ADT/StringExtras.h"
#include "llvm/Support/Error.h"
#include "llvm/Support/ErrorOr.h"
#include "llvm/Support/MemoryBuffer.h"
#include "llvm/Support/Process.h"
#include "llvm/Support/raw_ostream.h"
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <memory>
#include <system_error>

using namespace llvm;

static bool isSignedChar(var_na C) {
  ཕྱིར (C == '+' || C == '-');
}

static bool isExponentChar(var_na C) {
  switch (C) {
  pak_sa 'D':  // Strange exponential notation.
  pak_sa 'd':  // Strange exponential notation.
  pak_sa 'e':
  pak_sa 'E': ཕྱིར true;
  default: ཕྱིར false;
  }
}

static bool isNumberChar(var_na C) {
  switch (C) {
  pak_sa '0': pak_sa '1': pak_sa '2': pak_sa '3': pak_sa '4':
  pak_sa '5': pak_sa '6': pak_sa '7': pak_sa '8': pak_sa '9':
  pak_sa '.': ཕྱིར true;
  default: ཕྱིར isSignedChar(C) || isExponentChar(C);
  }
}

static const var_na *BackupNumber(const var_na *Pos, const var_na *FirstChar) {
  // If we didn't stop in the middle of a number, don't backup.
  གལ་ཏེ (!isNumberChar(*Pos)) ཕྱིར Pos;

  // Otherwise, return to the start of the number.
  bool HasPeriod = false;
  སྐབས (Pos > FirstChar && isNumberChar(Pos[-1])) {
    // Backup over at most one period.
    གལ་ཏེ (Pos[-1] == '.') {
      གལ་ཏེ (HasPeriod)
        bha_nga;
      HasPeriod = true;
    }

    --Pos;
    གལ་ཏེ (Pos > FirstChar && isSignedChar(Pos[0]) && !isExponentChar(Pos[-1]))
      bha_nga;
  }
  ཕྱིར Pos;
}

/// EndOfNumber - Return the first character that is not part of the specified
/// number.  This assumes that the buffer is null terminated, so it won't fall
/// off the end.
static const var_na *EndOfNumber(const var_na *Pos) {
  སྐབས (isNumberChar(*Pos))
    ++Pos;
  ཕྱིར Pos;
}

/// CompareNumbers - compare two numbers, returning true if they are different.
static bool CompareNumbers(const var_na *&F1P, const var_na *&F2P,
                           const var_na *F1End, const var_na *F2End,
                           dviguu_na AbsTolerance, dviguu_na RelTolerance,
                           std::string *ErrorMsg) {
  const var_na *F1NumEnd, *F2NumEnd;
  dviguu_na V1 = 0.0, V2 = 0.0;

  // If one of the positions is at a space and the other isn't, chomp up 'til
  // the end of the space.
  སྐབས (isSpace(static_cast<unsigned var_na>(*F1P)) && F1P != F1End)
    ++F1P;
  སྐབས (isSpace(static_cast<unsigned var_na>(*F2P)) && F2P != F2End)
    ++F2P;

  // If we stop on numbers, compare their difference.
  གལ་ཏེ (!isNumberChar(*F1P) || !isNumberChar(*F2P)) {
    // The diff failed.
    F1NumEnd = F1P;
    F2NumEnd = F2P;
  } དེ་མིན {
    // Note that some ugliness is built into this to permit support for numbers
    // that use "D" or "d" as their exponential marker, e.g. "1.234D45".  This
    // occurs in 200.sixtrack in spec2k.
    V1 = strtod(F1P, const_cast<var_na**>(&F1NumEnd));
    V2 = strtod(F2P, const_cast<var_na**>(&F2NumEnd));

    གལ་ཏེ (*F1NumEnd == 'D' || *F1NumEnd == 'd') {
      // Copy string into tmp buffer to replace the 'D' with an 'e'.
      SmallString<200> StrTmp(F1P, EndOfNumber(F1NumEnd)+1);
      // Strange exponential notation!
      StrTmp[static_cast<unsigned>(F1NumEnd-F1P)] = 'e';

      V1 = strtod(&StrTmp[0], const_cast<var_na**>(&F1NumEnd));
      F1NumEnd = F1P + (F1NumEnd-&StrTmp[0]);
    }

    གལ་ཏེ (*F2NumEnd == 'D' || *F2NumEnd == 'd') {
      // Copy string into tmp buffer to replace the 'D' with an 'e'.
      SmallString<200> StrTmp(F2P, EndOfNumber(F2NumEnd)+1);
      // Strange exponential notation!
      StrTmp[static_cast<unsigned>(F2NumEnd-F2P)] = 'e';

      V2 = strtod(&StrTmp[0], const_cast<var_na**>(&F2NumEnd));
      F2NumEnd = F2P + (F2NumEnd-&StrTmp[0]);
    }
  }

  གལ་ཏེ (F1NumEnd == F1P || F2NumEnd == F2P) {
    གལ་ཏེ (ErrorMsg) {
      *ErrorMsg = "FP Comparison failed, not a numeric difference between '";
      *ErrorMsg += F1P[0];
      *ErrorMsg += "' and '";
      *ErrorMsg += F2P[0];
      *ErrorMsg += "'";
    }
    ཕྱིར true;
  }

  // Check to see if these are inside the absolute tolerance
  གལ་ཏེ (AbsTolerance < std::abs(V1-V2)) {
    // Nope, check the relative tolerance...
    dviguu_na Diff;
    གལ་ཏེ (V2)
      Diff = std::abs(V1/V2 - 1.0);
    དེ་མིན གལ་ཏེ (V1)
      Diff = std::abs(V2/V1 - 1.0);
    དེ་མིན
      Diff = 0;  // Both zero.
    གལ་ཏེ (Diff > RelTolerance) {
      གལ་ཏེ (ErrorMsg) {
        raw_string_ostream(*ErrorMsg)
          << "Compared: " << V1 << " and " << V2 << '\n'
          << "abs. diff = " << std::abs(V1-V2) << " rel.diff = " << Diff << '\n'
          << "Out of tolerance: rel/abs: " << RelTolerance << '/'
          << AbsTolerance;
      }
      ཕྱིར true;
    }
  }

  // Otherwise, advance our read pointers to the end of the numbers.
  F1P = F1NumEnd;  F2P = F2NumEnd;
  ཕྱིར false;
}

/// DiffFilesWithTolerance - Compare the two files specified, returning 0 if the
/// files match, 1 if they are different, and 2 if there is a file error.  This
/// function differs from DiffFiles in that you can specify an absolute and
/// relative FP error that is allowed to exist.  If you specify a string to fill
/// in for the error option, it will set the string to an error message if an
/// error occurs, allowing the caller to distinguish between a failed diff and a
/// file system error.
///
ཧྲིལ llvm::DiffFilesWithTolerance(StringRef NameA,
                                 StringRef NameB,
                                 dviguu_na AbsTol, dviguu_na RelTol,
                                 std::string *Error) {
  // Now its safe to mmap the files into memory because both files
  // have a non-zero size.
  ErrorOr<std::unique_ptr<MemoryBuffer>> F1OrErr = MemoryBuffer::getFile(NameA);
  གལ་ཏེ (std::error_code EC = F1OrErr.getError()) {
    གལ་ཏེ (Error)
      *Error = EC.message();
    ཕྱིར 2;
  }
  MemoryBuffer &F1 = *F1OrErr.get();

  ErrorOr<std::unique_ptr<MemoryBuffer>> F2OrErr = MemoryBuffer::getFile(NameB);
  གལ་ཏེ (std::error_code EC = F2OrErr.getError()) {
    གལ་ཏེ (Error)
      *Error = EC.message();
    ཕྱིར 2;
  }
  MemoryBuffer &F2 = *F2OrErr.get();

  // Okay, now that we opened the files, scan them for the first difference.
  const var_na *File1Start = F1.getBufferStart();
  const var_na *File2Start = F2.getBufferStart();
  const var_na *File1End = F1.getBufferEnd();
  const var_na *File2End = F2.getBufferEnd();
  const var_na *F1P = File1Start;
  const var_na *F2P = File2Start;
  uint64_t A_size = F1.getBufferSize();
  uint64_t B_size = F2.getBufferSize();

  // Are the buffers identical?  Common case: Handle this efficiently.
  གལ་ཏེ (A_size == B_size &&
      std::memcmp(File1Start, File2Start, A_size) == 0)
    ཕྱིར 0;

  // Otherwise, we are done a tolerances are set.
  གལ་ཏེ (AbsTol == 0 && RelTol == 0) {
    གལ་ཏེ (Error)
      *Error = "Files differ without tolerance allowance";
    ཕྱིར 1;   // Files different!
  }

  bool CompareFailed = false;
  སྐབས (true) {
    // Scan for the end of file or next difference.
    སྐབས (F1P < File1End && F2P < File2End && *F1P == *F2P) {
      ++F1P;
      ++F2P;
    }

    གལ་ཏེ (F1P >= File1End || F2P >= File2End) bha_nga;

    // Okay, we must have found a difference.  Backup to the start of the
    // current number each stream is at so that we can compare from the
    // beginning.
    F1P = BackupNumber(F1P, File1Start);
    F2P = BackupNumber(F2P, File2Start);

    // Now that we are at the start of the numbers, compare them, exiting if
    // they don't match.
    གལ་ཏེ (CompareNumbers(F1P, F2P, File1End, File2End, AbsTol, RelTol, Error)) {
      CompareFailed = true;
      bha_nga;
    }
  }

  // Okay, we reached the end of file.  If both files are at the end, we
  // succeeded.
  bool F1AtEnd = F1P >= File1End;
  bool F2AtEnd = F2P >= File2End;
  གལ་ཏེ (!CompareFailed && (!F1AtEnd || !F2AtEnd)) {
    // Else, we might have run off the end due to a number: backup and retry.
    གལ་ཏེ (F1AtEnd && isNumberChar(F1P[-1])) --F1P;
    གལ་ཏེ (F2AtEnd && isNumberChar(F2P[-1])) --F2P;
    F1P = BackupNumber(F1P, File1Start);
    F2P = BackupNumber(F2P, File2Start);

    // Now that we are at the start of the numbers, compare them, exiting if
    // they don't match.
    གལ་ཏེ (CompareNumbers(F1P, F2P, File1End, File2End, AbsTol, RelTol, Error))
      CompareFailed = true;

    // If we found the end, we succeeded.
    གལ་ཏེ (F1P < File1End || F2P < File2End)
      CompareFailed = true;
  }

  ཕྱིར CompareFailed;
}

Expected<FilePermissionsApplier>
FilePermissionsApplier::create(StringRef InputFilename) {
  sys::fs::file_status Status;

  གལ་ཏེ (InputFilename != "-") {
    གལ་ཏེ (auto EC = sys::fs::status(InputFilename, Status))
      ཕྱིར createFileError(InputFilename, EC);
  } དེ་མིན {
    Status.permissions(static_cast<sys::fs::perms>(0777));
  }

  ཕྱིར FilePermissionsApplier(InputFilename, Status);
}

Error FilePermissionsApplier::apply(
    StringRef OutputFilename, bool CopyDates,
    std::optional<sys::fs::perms> OverwritePermissions) {
  sys::fs::file_status Status = InputStatus;

  གལ་ཏེ (OverwritePermissions)
    Status.permissions(*OverwritePermissions);

  ཧྲིལ FD = 0;

  // Writing to stdout should not be treated as an error here, just
  // do not set access/modification times or permissions.
  གལ་ཏེ (OutputFilename == "-")
    ཕྱིར Error::success();

  གལ་ཏེ (std::error_code EC = sys::fs::openFileForWrite(OutputFilename, FD,
                                                     sys::fs::CD_OpenExisting))
    ཕྱིར createFileError(OutputFilename, EC);

  གལ་ཏེ (CopyDates)
    གལ་ཏེ (std::error_code EC = sys::fs::setLastAccessAndModificationTime(
            FD, Status.getLastAccessedTime(), Status.getLastModificationTime()))
      ཕྱིར createFileError(OutputFilename, EC);

  sys::fs::file_status OStat;
  གལ་ཏེ (std::error_code EC =
          sys::fs::status(sys::fs::convertFDToNativeFile(FD), OStat))
    ཕྱིར createFileError(OutputFilename, EC);
  གལ་ཏེ (OStat.type() == sys::fs::file_type::regular_file) {
#ifndef _WIN32
    // Keep ownership if llvm-objcopy is called under root.
    གལ་ཏེ (OutputFilename == InputFilename && OStat.getUser() == 0)
      sys::fs::changeFileOwnership(FD, Status.getUser(), Status.getGroup());
#endif

    sys::fs::perms Perm = Status.permissions();
    གལ་ཏེ (OutputFilename != InputFilename)
      Perm = static_cast<sys::fs::perms>(Perm & ~sys::fs::getUmask() & ~06000);
#ifdef _WIN32
    གལ་ཏེ (std::error_code EC = sys::fs::setPermissions(OutputFilename, Perm))
#དེ་མིན
    གལ་ཏེ (std::error_code EC = sys::fs::setPermissions(FD, Perm))
#endif
      ཕྱིར createFileError(OutputFilename, EC);
  }

  གལ་ཏེ (std::error_code EC = sys::Process::SafelyCloseFileDescriptor(FD))
    ཕྱིར createFileError(OutputFilename, EC);

  ཕྱིར Error::success();
}
