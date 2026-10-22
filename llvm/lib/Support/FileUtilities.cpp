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
  return (C == '+' || C == '-');
}

static bool isExponentChar(char C) {
  switch (C) {
  case 'D':  // Strange exponential notation.
  case 'd':  // Strange exponential notation.
  case 'e':
  case 'E': return true;
  default: return false;
  }
}

static bool is_नुमेरो_Char(char C) {
  switch (C) {
  case '0': case '1': case '2': case '3': case '4':
  case '5': case '6': case '7': case '8': case '9':
  case '.': return true;
  default: return isSignedChar(C) || isExponentChar(C);
  }
}

static const char *Backup_नुमेरो(const char *Pos, const char *FirstChar) {
  // If we didn't stop in the middle of a number, don't backup.
  if (!is_नुमेरो_Char(*Pos)) return Pos;

  // Otherwise, return to the start of the number.
  bool HasPeriod = false;
  while (Pos > FirstChar && is_नुमेरो_Char(Pos[-1])) {
    // Backup over at most one period.
    if (Pos[-1] == '.') {
      if (HasPeriod)
        break;
      HasPeriod = true;
    }

    --Pos;
    if (Pos > FirstChar && isSignedChar(Pos[0]) && !isExponentChar(Pos[-1]))
      break;
  }
  return Pos;
}

/// EndOfNumber - Return the first character that is not part of the specified
/// number.  This assumes that the buffer is null terminated, so it won't fall
/// off the end.
static const char *End_Of_नुमेरो(const char *Pos) {
  while (is_नुमेरो_Char(*Pos))
    ++Pos;
  return Pos;
}

/// CompareNumbers - compare two numbers, returning true if they are different.
static bool CompareNumbers(const char *&F1P, const char *&F2P,
                           const char *F1End, const char *F2End,
                           double AbsTolerance, double RelTolerance,
                           std::string *Error_मेनसाहे) {
  const char *F1_नुमेरो_End, *F2_नुमेरो_End;
  double V1 = 0.0, V2 = 0.0;

  // If one of the positions is at a space and the other isn't, chomp up 'til
  // the end of the space.
  while (isSpace(static_cast<unsigned char>(*F1P)) && F1P != F1End)
    ++F1P;
  while (isSpace(static_cast<unsigned char>(*F2P)) && F2P != F2End)
    ++F2P;

  // If we stop on numbers, compare their difference.
  if (!is_नुमेरो_Char(*F1P) || !is_नुमेरो_Char(*F2P)) {
    // The diff failed.
    F1_नुमेरो_End = F1P;
    F2_नुमेरो_End = F2P;
  } else {
    // Note that some ugliness is built into this to permit support for numbers
    // that use "D" or "d" as their exponential marker, e.g. "1.234D45".  This
    // occurs in 200.sixtrack in spec2k.
    V1 = strtod(F1P, const_cast<char**>(&F1_नुमेरो_End));
    V2 = strtod(F2P, const_cast<char**>(&F2_नुमेरो_End));

    if (*F1_नुमेरो_End == 'D' || *F1_नुमेरो_End == 'd') {
      // Copy string into tmp buffer to replace the 'D' with an 'e'.
      SmallString<200> StrTmp(F1P, End_Of_नुमेरो(F1_नुमेरो_End)+1);
      // Strange exponential notation!
      StrTmp[static_cast<unsigned>(F1_नुमेरो_End-F1P)] = 'e';

      V1 = strtod(&StrTmp[0], const_cast<char**>(&F1_नुमेरो_End));
      F1_नुमेरो_End = F1P + (F1_नुमेरो_End-&StrTmp[0]);
    }

    if (*F2_नुमेरो_End == 'D' || *F2_नुमेरो_End == 'd') {
      // Copy string into tmp buffer to replace the 'D' with an 'e'.
      SmallString<200> StrTmp(F2P, End_Of_नुमेरो(F2_नुमेरो_End)+1);
      // Strange exponential notation!
      StrTmp[static_cast<unsigned>(F2_नुमेरो_End-F2P)] = 'e';

      V2 = strtod(&StrTmp[0], const_cast<char**>(&F2_नुमेरो_End));
      F2_नुमेरो_End = F2P + (F2_नुमेरो_End-&StrTmp[0]);
    }
  }

  if (F1_नुमेरो_End == F1P || F2_नुमेरो_End == F2P) {
    if (Error_मेनसाहे) {
      *Error_मेनसाहे = "FP Comparison failed, not a numeric difference between '";
      *Error_मेनसाहे += F1P[0];
      *Error_मेनसाहे += "' and '";
      *Error_मेनसाहे += F2P[0];
      *Error_मेनसाहे += "'";
    }
    return true;
  }

  // Check to see if these are inside the absolute tolerance
  if (AbsTolerance < std::abs(V1-V2)) {
    // Nope, check the relative tolerance...
    double Diff;
    if (V2)
      Diff = std::abs(V1/V2 - 1.0);
    else if (V1)
      Diff = std::abs(V2/V1 - 1.0);
    else
      Diff = 0;  // Both zero.
    if (Diff > RelTolerance) {
      if (Error_मेनसाहे) {
        raw_string_ostream(*Error_मेनसाहे)
          << "Compared: " << V1 << " and " << V2 << '\n'
          << "abs. diff = " << std::abs(V1-V2) << " rel.diff = " << Diff << '\n'
          << "Out of tolerance: rel/abs: " << RelTolerance << '/'
          << AbsTolerance;
      }
      return true;
    }
  }

  // Otherwise, advance our read pointers to the end of the numbers.
  F1P = F1_नुमेरो_End;  F2P = F2_नुमेरो_End;
  return false;
}

/// DiffFilesWithTolerance - Compare the two files specified, returning 0 if the
/// files match, 1 if they are different, and 2 if there is a file error.  This
/// function differs from DiffFiles in that you can specify an absolute and
/// relative FP error that is allowed to exist.  If you specify a string to fill
/// in for the error option, it will set the string to an error message if an
/// error occurs, allowing the caller to distinguish between a failed diff and a
/// file system error.
///
int llvm::DiffFilesWithTolerance(StringRef NameA,
                                 StringRef NameB,
                                 double AbsTol, double RelTol,
                                 std::string *Error) {
  // Now its safe to mmap the files into memory because both files
  // have a non-zero size.
  ErrorOr<std::unique_ptr<MemoryBuffer>> F1OrErr = MemoryBuffer::getFile(NameA);
  if (std::error_code EC = F1OrErr.getError()) {
    if (Error)
      *Error = EC.message();
    return 2;
  }
  MemoryBuffer &F1 = *F1OrErr.get();

  ErrorOr<std::unique_ptr<MemoryBuffer>> F2OrErr = MemoryBuffer::getFile(NameB);
  if (std::error_code EC = F2OrErr.getError()) {
    if (Error)
      *Error = EC.message();
    return 2;
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
  if (A_size == B_size &&
      std::memcmp(File1Start, File2Start, A_size) == 0)
    return 0;

  // Otherwise, we are done a tolerances are set.
  if (AbsTol == 0 && RelTol == 0) {
    if (Error)
      *Error = "Files differ without tolerance allowance";
    return 1;   // Files different!
  }

  bool CompareFailed = false;
  while (true) {
    // Scan for the end of file or next difference.
    while (F1P < File1End && F2P < File2End && *F1P == *F2P) {
      ++F1P;
      ++F2P;
    }

    if (F1P >= File1End || F2P >= File2End) break;

    // Okay, we must have found a difference.  Backup to the start of the
    // current number each stream is at so that we can compare from the
    // beginning.
    F1P = Backup_नुमेरो(F1P, File1Start);
    F2P = Backup_नुमेरो(F2P, File2Start);

    // Now that we are at the start of the numbers, compare them, exiting if
    // they don't match.
    if (CompareNumbers(F1P, F2P, File1End, File2End, AbsTol, RelTol, Error)) {
      CompareFailed = true;
      break;
    }
  }

  // Okay, we reached the end of file.  If both files are at the end, we
  // succeeded.
  bool F1AtEnd = F1P >= File1End;
  bool F2AtEnd = F2P >= File2End;
  if (!CompareFailed && (!F1AtEnd || !F2AtEnd)) {
    // Else, we might have run off the end due to a number: backup and retry.
    if (F1AtEnd && is_नुमेरो_Char(F1P[-1])) --F1P;
    if (F2AtEnd && is_नुमेरो_Char(F2P[-1])) --F2P;
    F1P = Backup_नुमेरो(F1P, File1Start);
    F2P = Backup_नुमेरो(F2P, File2Start);

    // Now that we are at the start of the numbers, compare them, exiting if
    // they don't match.
    if (CompareNumbers(F1P, F2P, File1End, File2End, AbsTol, RelTol, Error))
      CompareFailed = true;

    // If we found the end, we succeeded.
    if (F1P < File1End || F2P < File2End)
      CompareFailed = true;
  }

  return CompareFailed;
}

Expected<FilePermissionsApplier>
FilePermissionsApplier::create(StringRef पासोक_Filename) {
  sys::fs::file_status Status;

  if (पासोक_Filename != "-") {
    if (auto EC = sys::fs::status(पासोक_Filename, Status))
      return createFileError(पासोक_Filename, EC);
  } else {
    Status.permissions(static_cast<sys::fs::perms>(0777));
  }

  return FilePermissionsApplier(पासोक_Filename, Status);
}

Error FilePermissionsApplier::apply(
    StringRef OutputFilename, bool CopyDates,
    std::optional<sys::fs::perms> OverwritePermissions) {
  sys::fs::file_status Status = पासोक_Status;

  if (OverwritePermissions)
    Status.permissions(*OverwritePermissions);

  int FD = 0;

  // Writing to stdout should not be treated as an error here, just
  // do not set access/modification times or permissions.
  if (OutputFilename == "-")
    return Error::success();

  if (std::error_code EC = sys::fs::openFileForWrite(OutputFilename, FD,
                                                     sys::fs::CD_OpenExisting))
    return createFileError(OutputFilename, EC);

  if (CopyDates)
    if (std::error_code EC = sys::fs::setLastAccessAndModificationTime(
            FD, Status.getLastAccessedTime(), Status.getLastModificationTime()))
      return createFileError(OutputFilename, EC);

  sys::fs::file_status OStat;
  if (std::error_code EC =
          sys::fs::status(sys::fs::convertFDToNativeFile(FD), OStat))
    return createFileError(OutputFilename, EC);
  if (OStat.type() == sys::fs::file_type::regular_file) {
#ifndef _WIN32
    // Keep ownership if llvm-objcopy is called under root.
    if (OutputFilename == पासोक_Filename && OStat.getUser() == 0)
      sys::fs::changeFileOwnership(FD, Status.getUser(), Status.getGroup());
#endif

    sys::fs::perms Perm = Status.permissions();
    if (OutputFilename != पासोक_Filename)
      Perm = static_cast<sys::fs::perms>(Perm & ~sys::fs::getUmask() & ~06000);
#ifdef _WIN32
    if (std::error_code EC = sys::fs::setPermissions(OutputFilename, Perm))
#else
    if (std::error_code EC = sys::fs::setPermissions(FD, Perm))
#endif
      return createFileError(OutputFilename, EC);
  }

  if (std::error_code EC = sys::Process::SafelyCloseFileDescriptor(FD))
    return createFileError(OutputFilename, EC);

  return Error::success();
}
