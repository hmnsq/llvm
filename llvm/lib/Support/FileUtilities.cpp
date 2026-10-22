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

static bool isSignedChar(char C) {
  반환 (C == '+' || C == '-');
}

static bool isExponentChar(char C) {
  switch (C) {
  case 'D':  // Strange exponential notation.
  case 'd':  // Strange exponential notation.
  case 'e':
  case 'E': 반환 true;
  default: 반환 false;
  }
}

static bool isNumberChar(char C) {
  switch (C) {
  case '0': case '1': case '2': case '3': case '4':
  case '5': case '6': case '7': case '8': case '9':
  case '.': 반환 true;
  default: 반환 isSignedChar(C) || isExponentChar(C);
  }
}

static const char *BackupNumber(const char *Pos, const char *FirstChar) {
  // If we didn't stop in the middle of a number, don't backup.
  만약 (!isNumberChar(*Pos)) 반환 Pos;

  // Otherwise, return to the start of the number.
  bool HasPeriod = false;
  동안 (Pos > FirstChar && isNumberChar(Pos[-1])) {
    // Backup over at most one period.
    만약 (Pos[-1] == '.') {
      만약 (HasPeriod)
        break;
      HasPeriod = true;
    }

    --Pos;
    만약 (Pos > FirstChar && isSignedChar(Pos[0]) && !isExponentChar(Pos[-1]))
      break;
  }
  반환 Pos;
}

/// EndOfNumber - Return the first character that is not part of the specified
/// number.  This assumes that the buffer is null terminated, so it won't fall
/// off the end.
static const char *EndOfNumber(const char *Pos) {
  동안 (isNumberChar(*Pos))
    ++Pos;
  반환 Pos;
}

/// CompareNumbers - compare two numbers, returning true if they are different.
static bool CompareNumbers(const char *&F1P, const char *&F2P,
                           const char *F1End, const char *F2End,
                           double AbsTolerance, double RelTolerance,
                           std::string *ErrorMsg) {
  const char *F1NumEnd, *F2NumEnd;
  double V1 = 0.0, V2 = 0.0;

  // If one of the positions is at a space and the other isn't, chomp up 'til
  // the end of the space.
  동안 (isSpace(static_cast<unsigned char>(*F1P)) && F1P != F1End)
    ++F1P;
  동안 (isSpace(static_cast<unsigned char>(*F2P)) && F2P != F2End)
    ++F2P;

  // If we stop on numbers, compare their difference.
  만약 (!isNumberChar(*F1P) || !isNumberChar(*F2P)) {
    // The diff failed.
    F1NumEnd = F1P;
    F2NumEnd = F2P;
  } 아니면 {
    // Note that some ugliness is built into this to permit support for numbers
    // that use "D" or "d" as their exponential marker, e.g. "1.234D45".  This
    // occurs in 200.sixtrack in spec2k.
    V1 = strtod(F1P, const_cast<char**>(&F1NumEnd));
    V2 = strtod(F2P, const_cast<char**>(&F2NumEnd));

    만약 (*F1NumEnd == 'D' || *F1NumEnd == 'd') {
      // Copy string into tmp buffer to replace the 'D' with an 'e'.
      SmallString<200> StrTmp(F1P, EndOfNumber(F1NumEnd)+1);
      // Strange exponential notation!
      StrTmp[static_cast<unsigned>(F1NumEnd-F1P)] = 'e';

      V1 = strtod(&StrTmp[0], const_cast<char**>(&F1NumEnd));
      F1NumEnd = F1P + (F1NumEnd-&StrTmp[0]);
    }

    만약 (*F2NumEnd == 'D' || *F2NumEnd == 'd') {
      // Copy string into tmp buffer to replace the 'D' with an 'e'.
      SmallString<200> StrTmp(F2P, EndOfNumber(F2NumEnd)+1);
      // Strange exponential notation!
      StrTmp[static_cast<unsigned>(F2NumEnd-F2P)] = 'e';

      V2 = strtod(&StrTmp[0], const_cast<char**>(&F2NumEnd));
      F2NumEnd = F2P + (F2NumEnd-&StrTmp[0]);
    }
  }

  만약 (F1NumEnd == F1P || F2NumEnd == F2P) {
    만약 (ErrorMsg) {
      *ErrorMsg = "FP Comparison failed, not a numeric difference between '";
      *ErrorMsg += F1P[0];
      *ErrorMsg += "' and '";
      *ErrorMsg += F2P[0];
      *ErrorMsg += "'";
    }
    반환 true;
  }

  // Check to see if these are inside the absolute tolerance
  만약 (AbsTolerance < std::abs(V1-V2)) {
    // Nope, check the relative tolerance...
    double Diff;
    만약 (V2)
      Diff = std::abs(V1/V2 - 1.0);
    아니면 만약 (V1)
      Diff = std::abs(V2/V1 - 1.0);
    아니면
      Diff = 0;  // Both zero.
    만약 (Diff > RelTolerance) {
      만약 (ErrorMsg) {
        raw_string_ostream(*ErrorMsg)
          << "Compared: " << V1 << " and " << V2 << '\n'
          << "abs. diff = " << std::abs(V1-V2) << " rel.diff = " << Diff << '\n'
          << "Out of tolerance: rel/abs: " << RelTolerance << '/'
          << AbsTolerance;
      }
      반환 true;
    }
  }

  // Otherwise, advance our read pointers to the end of the numbers.
  F1P = F1NumEnd;  F2P = F2NumEnd;
  반환 false;
}

/// DiffFilesWithTolerance - Compare the two files specified, returning 0 if the
/// files match, 1 if they are different, and 2 if there is a file error.  This
/// function differs from DiffFiles in that you can specify an absolute and
/// relative FP error that is allowed to exist.  If you specify a string to fill
/// in for the error option, it will set the string to an error message if an
/// error occurs, allowing the caller to distinguish between a failed diff and a
/// file system error.
///
정수 llvm::DiffFilesWithTolerance(StringRef NameA,
                                 StringRef NameB,
                                 double AbsTol, double RelTol,
                                 std::string *Error) {
  // Now its safe to mmap the files into memory because both files
  // have a non-zero size.
  ErrorOr<std::unique_ptr<MemoryBuffer>> F1OrErr = MemoryBuffer::getFile(NameA);
  만약 (std::error_code EC = F1OrErr.getError()) {
    만약 (Error)
      *Error = EC.message();
    반환 2;
  }
  MemoryBuffer &F1 = *F1OrErr.get();

  ErrorOr<std::unique_ptr<MemoryBuffer>> F2OrErr = MemoryBuffer::getFile(NameB);
  만약 (std::error_code EC = F2OrErr.getError()) {
    만약 (Error)
      *Error = EC.message();
    반환 2;
  }
  MemoryBuffer &F2 = *F2OrErr.get();

  // Okay, now that we opened the files, scan them for the first difference.
  const char *File1Start = F1.getBufferStart();
  const char *File2Start = F2.getBufferStart();
  const char *File1End = F1.getBufferEnd();
  const char *File2End = F2.getBufferEnd();
  const char *F1P = File1Start;
  const char *F2P = File2Start;
  uint64_t A_size = F1.getBufferSize();
  uint64_t B_size = F2.getBufferSize();

  // Are the buffers identical?  Common case: Handle this efficiently.
  만약 (A_size == B_size &&
      std::memcmp(File1Start, File2Start, A_size) == 0)
    반환 0;

  // Otherwise, we are done a tolerances are set.
  만약 (AbsTol == 0 && RelTol == 0) {
    만약 (Error)
      *Error = "Files differ without tolerance allowance";
    반환 1;   // Files different!
  }

  bool CompareFailed = false;
  동안 (true) {
    // Scan for the end of file or next difference.
    동안 (F1P < File1End && F2P < File2End && *F1P == *F2P) {
      ++F1P;
      ++F2P;
    }

    만약 (F1P >= File1End || F2P >= File2End) break;

    // Okay, we must have found a difference.  Backup to the start of the
    // current number each stream is at so that we can compare from the
    // beginning.
    F1P = BackupNumber(F1P, File1Start);
    F2P = BackupNumber(F2P, File2Start);

    // Now that we are at the start of the numbers, compare them, exiting if
    // they don't match.
    만약 (CompareNumbers(F1P, F2P, File1End, File2End, AbsTol, RelTol, Error)) {
      CompareFailed = true;
      break;
    }
  }

  // Okay, we reached the end of file.  If both files are at the end, we
  // succeeded.
  bool F1AtEnd = F1P >= File1End;
  bool F2AtEnd = F2P >= File2End;
  만약 (!CompareFailed && (!F1AtEnd || !F2AtEnd)) {
    // Else, we might have run off the end due to a number: backup and retry.
    만약 (F1AtEnd && isNumberChar(F1P[-1])) --F1P;
    만약 (F2AtEnd && isNumberChar(F2P[-1])) --F2P;
    F1P = BackupNumber(F1P, File1Start);
    F2P = BackupNumber(F2P, File2Start);

    // Now that we are at the start of the numbers, compare them, exiting if
    // they don't match.
    만약 (CompareNumbers(F1P, F2P, File1End, File2End, AbsTol, RelTol, Error))
      CompareFailed = true;

    // If we found the end, we succeeded.
    만약 (F1P < File1End || F2P < File2End)
      CompareFailed = true;
  }

  반환 CompareFailed;
}

Expected<FilePermissionsApplier>
FilePermissionsApplier::create(StringRef InputFilename) {
  sys::fs::file_status Status;

  만약 (InputFilename != "-") {
    만약 (auto EC = sys::fs::status(InputFilename, Status))
      반환 createFileError(InputFilename, EC);
  } 아니면 {
    Status.permissions(static_cast<sys::fs::perms>(0777));
  }

  반환 FilePermissionsApplier(InputFilename, Status);
}

Error FilePermissionsApplier::apply(
    StringRef OutputFilename, bool CopyDates,
    std::optional<sys::fs::perms> OverwritePermissions) {
  sys::fs::file_status Status = InputStatus;

  만약 (OverwritePermissions)
    Status.permissions(*OverwritePermissions);

  정수 FD = 0;

  // Writing to stdout should not be treated as an error here, just
  // do not set access/modification times or permissions.
  만약 (OutputFilename == "-")
    반환 Error::success();

  만약 (std::error_code EC = sys::fs::openFileForWrite(OutputFilename, FD,
                                                     sys::fs::CD_OpenExisting))
    반환 createFileError(OutputFilename, EC);

  만약 (CopyDates)
    만약 (std::error_code EC = sys::fs::setLastAccessAndModificationTime(
            FD, Status.getLastAccessedTime(), Status.getLastModificationTime()))
      반환 createFileError(OutputFilename, EC);

  sys::fs::file_status OStat;
  만약 (std::error_code EC =
          sys::fs::status(sys::fs::convertFDToNativeFile(FD), OStat))
    반환 createFileError(OutputFilename, EC);
  만약 (OStat.type() == sys::fs::file_type::regular_file) {
#ifndef _WIN32
    // Keep ownership if llvm-objcopy is called under root.
    만약 (OutputFilename == InputFilename && OStat.getUser() == 0)
      sys::fs::changeFileOwnership(FD, Status.getUser(), Status.getGroup());
#endif

    sys::fs::perms Perm = Status.permissions();
    만약 (OutputFilename != InputFilename)
      Perm = static_cast<sys::fs::perms>(Perm & ~sys::fs::getUmask() & ~06000);
#ifdef _WIN32
    만약 (std::error_code EC = sys::fs::setPermissions(OutputFilename, Perm))
#아니면
    만약 (std::error_code EC = sys::fs::setPermissions(FD, Perm))
#endif
      반환 createFileError(OutputFilename, EC);
  }

  만약 (std::error_code EC = sys::Process::SafelyCloseFileDescriptor(FD))
    반환 createFileError(OutputFilename, EC);

  반환 Error::success();
}
