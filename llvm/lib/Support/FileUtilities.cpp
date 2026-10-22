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

#அந்தர்பாவ "llvm/Support/FileUtilities.h"
#அந்தர்பாவ "llvm/ADT/SmallString.h"
#அந்தர்பாவ "llvm/ADT/StringExtras.h"
#அந்தர்பாவ "llvm/Support/Error.h"
#அந்தர்பாவ "llvm/Support/ErrorOr.h"
#அந்தர்பாவ "llvm/Support/MemoryBuffer.h"
#அந்தர்பாவ "llvm/Support/Process.h"
#அந்தர்பாவ "llvm/Support/raw_ostream.h"
#அந்தர்பாவ <cmath>
#அந்தர்பாவ <cstdint>
#அந்தர்பாவ <cstdlib>
#அந்தர்பாவ <cstring>
#அந்தர்பாவ <memory>
#அந்தர்பாவ <system_error>

using நாமாகாஶ llvm;

நிலையான தருக்க isSignedChar(எழுத்து C) {
  திரும்பு (C == '+' || C == '-');
}

நிலையான தருக்க isExponentChar(எழுத்து C) {
  தேர்வு (C) {
  நிலை 'D':  // Strange exponential notation.
  நிலை 'd':  // Strange exponential notation.
  நிலை 'e':
  நிலை 'E': திரும்பு உண்மை;
  இயல்பு: திரும்பு பொய்;
  }
}

நிலையான தருக்க is_எண்_Char(எழுத்து C) {
  தேர்வு (C) {
  நிலை '0': நிலை '1': நிலை '2': நிலை '3': நிலை '4':
  நிலை '5': நிலை '6': நிலை '7': நிலை '8': நிலை '9':
  நிலை '.': திரும்பு உண்மை;
  இயல்பு: திரும்பு isSignedChar(C) || isExponentChar(C);
  }
}

நிலையான மாறிலி எழுத்து *Backup_எண்(மாறிலி எழுத்து *Pos, மாறிலி எழுத்து *FirstChar) {
  // If we didn't stop in the middle of a number, don't backup.
  எனில் (!is_எண்_Char(*Pos)) திரும்பு Pos;

  // Otherwise, return to the start of the number.
  தருக்க HasPeriod = பொய்;
  போது (Pos > FirstChar && is_எண்_Char(Pos[-1])) {
    // Backup over at most one period.
    எனில் (Pos[-1] == '.') {
      எனில் (HasPeriod)
        நிறுத்து;
      HasPeriod = உண்மை;
    }

    --Pos;
    எனில் (Pos > FirstChar && isSignedChar(Pos[0]) && !isExponentChar(Pos[-1]))
      நிறுத்து;
  }
  திரும்பு Pos;
}

/// EndOfNumber - Return the first character that is not part of the specified
/// number.  This assumes that the buffer is null terminated, so it won't fall
/// off the end.
நிலையான மாறிலி எழுத்து *End_Of_எண்(மாறிலி எழுத்து *Pos) {
  போது (is_எண்_Char(*Pos))
    ++Pos;
  திரும்பு Pos;
}

/// CompareNumbers - compare two numbers, returning true if they are different.
நிலையான தருக்க CompareNumbers(மாறிலி எழுத்து *&F1P, மாறிலி எழுத்து *&F2P,
                           மாறிலி எழுத்து *F1End, மாறிலி எழுத்து *F2End,
                           இரட்டை AbsTolerance, இரட்டை RelTolerance,
                           std::string *Error_செய்தி) {
  மாறிலி எழுத்து *F1_இலக்கம்_End, *F2_இலக்கம்_End;
  இரட்டை V1 = 0.0, V2 = 0.0;

  // If one of the positions is at a space and the other isn't, chomp up 'til
  // the end of the space.
  போது (isSpace(static_cast<unsigned எழுத்து>(*F1P)) && F1P != F1End)
    ++F1P;
  போது (isSpace(static_cast<unsigned எழுத்து>(*F2P)) && F2P != F2End)
    ++F2P;

  // If we stop on numbers, compare their difference.
  எனில் (!is_எண்_Char(*F1P) || !is_எண்_Char(*F2P)) {
    // The diff failed.
    F1_இலக்கம்_End = F1P;
    F2_இலக்கம்_End = F2P;
  } இல்லையெனில் {
    // Note that some ugliness is built into this to permit support for numbers
    // that use "D" or "d" as their exponential marker, e.g. "1.234D45".  This
    // occurs in 200.sixtrack in spec2k.
    V1 = strtod(F1P, const_cast<எழுத்து**>(&F1_இலக்கம்_End));
    V2 = strtod(F2P, const_cast<எழுத்து**>(&F2_இலக்கம்_End));

    எனில் (*F1_இலக்கம்_End == 'D' || *F1_இலக்கம்_End == 'd') {
      // Copy string into tmp buffer to replace the 'D' with an 'e'.
      SmallString<200> StrTmp(F1P, End_Of_எண்(F1_இலக்கம்_End)+1);
      // Strange exponential notation!
      StrTmp[static_cast<unsigned>(F1_இலக்கம்_End-F1P)] = 'e';

      V1 = strtod(&StrTmp[0], const_cast<எழுத்து**>(&F1_இலக்கம்_End));
      F1_இலக்கம்_End = F1P + (F1_இலக்கம்_End-&StrTmp[0]);
    }

    எனில் (*F2_இலக்கம்_End == 'D' || *F2_இலக்கம்_End == 'd') {
      // Copy string into tmp buffer to replace the 'D' with an 'e'.
      SmallString<200> StrTmp(F2P, End_Of_எண்(F2_இலக்கம்_End)+1);
      // Strange exponential notation!
      StrTmp[static_cast<unsigned>(F2_இலக்கம்_End-F2P)] = 'e';

      V2 = strtod(&StrTmp[0], const_cast<எழுத்து**>(&F2_இலக்கம்_End));
      F2_இலக்கம்_End = F2P + (F2_இலக்கம்_End-&StrTmp[0]);
    }
  }

  எனில் (F1_இலக்கம்_End == F1P || F2_இலக்கம்_End == F2P) {
    எனில் (Error_செய்தி) {
      *Error_செய்தி = "FP Comparison failed, not a numeric difference between '";
      *Error_செய்தி += F1P[0];
      *Error_செய்தி += "' and '";
      *Error_செய்தி += F2P[0];
      *Error_செய்தி += "'";
    }
    திரும்பு உண்மை;
  }

  // Check to see if these are inside the absolute tolerance
  எனில் (AbsTolerance < std::abs(V1-V2)) {
    // Nope, check the relative tolerance...
    இரட்டை Diff;
    எனில் (V2)
      Diff = std::abs(V1/V2 - 1.0);
    இல்லையெனில் எனில் (V1)
      Diff = std::abs(V2/V1 - 1.0);
    இல்லையெனில்
      Diff = 0;  // Both zero.
    எனில் (Diff > RelTolerance) {
      எனில் (Error_செய்தி) {
        raw_string_ostream(*Error_செய்தி)
          << "Compared: " << V1 << " and " << V2 << '\n'
          << "abs. diff = " << std::abs(V1-V2) << " rel.diff = " << Diff << '\n'
          << "Out of tolerance: rel/abs: " << RelTolerance << '/'
          << AbsTolerance;
      }
      திரும்பு உண்மை;
    }
  }

  // Otherwise, advance our read pointers to the end of the numbers.
  F1P = F1_இலக்கம்_End;  F2P = F2_இலக்கம்_End;
  திரும்பு பொய்;
}

/// DiffFilesWithTolerance - Compare the two files specified, returning 0 if the
/// files match, 1 if they are different, and 2 if there is a file error.  This
/// function differs from DiffFiles in that you can specify an absolute and
/// relative FP error that is allowed to exist.  If you specify a string to fill
/// in for the error option, it will set the string to an error message if an
/// error occurs, allowing the caller to distinguish between a failed diff and a
/// file system error.
///
முழு llvm::DiffFilesWithTolerance(StringRef NameA,
                                 StringRef NameB,
                                 இரட்டை AbsTol, இரட்டை RelTol,
                                 std::string *Error) {
  // Now its safe to mmap the files into memory because both files
  // have a non-zero size.
  ErrorOr<std::unique_ptr<MemoryBuffer>> F1OrErr = MemoryBuffer::getFile(NameA);
  எனில் (std::error_code EC = F1OrErr.getError()) {
    எனில் (Error)
      *Error = EC.message();
    திரும்பு 2;
  }
  MemoryBuffer &F1 = *F1OrErr.get();

  ErrorOr<std::unique_ptr<MemoryBuffer>> F2OrErr = MemoryBuffer::getFile(NameB);
  எனில் (std::error_code EC = F2OrErr.getError()) {
    எனில் (Error)
      *Error = EC.message();
    திரும்பு 2;
  }
  MemoryBuffer &F2 = *F2OrErr.get();

  // Okay, now that we opened the files, scan them for the first difference.
  மாறிலி எழுத்து *File1Start = F1.getBufferStart();
  மாறிலி எழுத்து *File2Start = F2.getBufferStart();
  மாறிலி எழுத்து *File1End = F1.getBufferEnd();
  மாறிலி எழுத்து *File2End = F2.getBufferEnd();
  மாறிலி எழுத்து *F1P = File1Start;
  மாறிலி எழுத்து *F2P = File2Start;
  uint64_t A_size = F1.getBufferSize();
  uint64_t B_size = F2.getBufferSize();

  // Are the buffers identical?  Common case: Handle this efficiently.
  எனில் (A_size == B_size &&
      std::memcmp(File1Start, File2Start, A_size) == 0)
    திரும்பு 0;

  // Otherwise, we are done a tolerances are set.
  எனில் (AbsTol == 0 && RelTol == 0) {
    எனில் (Error)
      *Error = "Files differ without tolerance allowance";
    திரும்பு 1;   // Files different!
  }

  தருக்க CompareFailed = பொய்;
  போது (உண்மை) {
    // Scan for the end of file or next difference.
    போது (F1P < File1End && F2P < File2End && *F1P == *F2P) {
      ++F1P;
      ++F2P;
    }

    எனில் (F1P >= File1End || F2P >= File2End) நிறுத்து;

    // Okay, we must have found a difference.  Backup to the start of the
    // current number each stream is at so that we can compare from the
    // beginning.
    F1P = Backup_எண்(F1P, File1Start);
    F2P = Backup_எண்(F2P, File2Start);

    // Now that we are at the start of the numbers, compare them, exiting if
    // they don't match.
    எனில் (CompareNumbers(F1P, F2P, File1End, File2End, AbsTol, RelTol, Error)) {
      CompareFailed = உண்மை;
      நிறுத்து;
    }
  }

  // Okay, we reached the end of file.  If both files are at the end, we
  // succeeded.
  தருக்க F1AtEnd = F1P >= File1End;
  தருக்க F2AtEnd = F2P >= File2End;
  எனில் (!CompareFailed && (!F1AtEnd || !F2AtEnd)) {
    // Else, we might have run off the end due to a number: backup and retry.
    எனில் (F1AtEnd && is_எண்_Char(F1P[-1])) --F1P;
    எனில் (F2AtEnd && is_எண்_Char(F2P[-1])) --F2P;
    F1P = Backup_எண்(F1P, File1Start);
    F2P = Backup_எண்(F2P, File2Start);

    // Now that we are at the start of the numbers, compare them, exiting if
    // they don't match.
    எனில் (CompareNumbers(F1P, F2P, File1End, File2End, AbsTol, RelTol, Error))
      CompareFailed = உண்மை;

    // If we found the end, we succeeded.
    எனில் (F1P < File1End || F2P < File2End)
      CompareFailed = உண்மை;
  }

  திரும்பு CompareFailed;
}

Expected<FilePermissionsApplier>
FilePermissionsApplier::create(StringRef உள்ளீடு_Filename) {
  sys::fs::file_status Status;

  எனில் (உள்ளீடு_Filename != "-") {
    எனில் (auto EC = sys::fs::status(உள்ளீடு_Filename, Status))
      திரும்பு createFileError(உள்ளீடு_Filename, EC);
  } இல்லையெனில் {
    Status.permissions(static_cast<sys::fs::perms>(0777));
  }

  திரும்பு FilePermissionsApplier(உள்ளீடு_Filename, Status);
}

Error FilePermissionsApplier::apply(
    StringRef OutputFilename, தருக்க CopyDates,
    std::optional<sys::fs::perms> OverwritePermissions) {
  sys::fs::file_status Status = உள்ளீடு_Status;

  எனில் (OverwritePermissions)
    Status.permissions(*OverwritePermissions);

  முழு FD = 0;

  // Writing to stdout should not be treated as an error here, just
  // do not set access/modification times or permissions.
  எனில் (OutputFilename == "-")
    திரும்பு Error::success();

  எனில் (std::error_code EC = sys::fs::openFileForWrite(OutputFilename, FD,
                                                     sys::fs::CD_OpenExisting))
    திரும்பு createFileError(OutputFilename, EC);

  எனில் (CopyDates)
    எனில் (std::error_code EC = sys::fs::setLastAccessAndModificationTime(
            FD, Status.getLastAccessedTime(), Status.getLastModificationTime()))
      திரும்பு createFileError(OutputFilename, EC);

  sys::fs::file_status OStat;
  எனில் (std::error_code EC =
          sys::fs::status(sys::fs::convertFDToNativeFile(FD), OStat))
    திரும்பு createFileError(OutputFilename, EC);
  எனில் (OStat.type() == sys::fs::file_type::regular_file) {
#ifndef _WIN32
    // Keep ownership if llvm-objcopy is called under root.
    எனில் (OutputFilename == உள்ளீடு_Filename && OStat.getUser() == 0)
      sys::fs::changeFileOwnership(FD, Status.getUser(), Status.getGroup());
#endif

    sys::fs::perms Perm = Status.permissions();
    எனில் (OutputFilename != உள்ளீடு_Filename)
      Perm = static_cast<sys::fs::perms>(Perm & ~sys::fs::getUmask() & ~06000);
#ifdef _WIN32
    எனில் (std::error_code EC = sys::fs::setPermissions(OutputFilename, Perm))
#இல்லையெனில்
    எனில் (std::error_code EC = sys::fs::setPermissions(FD, Perm))
#endif
      திரும்பு createFileError(OutputFilename, EC);
  }

  எனில் (std::error_code EC = sys::Process::SafelyCloseFileDescriptor(FD))
    திரும்பு createFileError(OutputFilename, EC);

  திரும்பு Error::success();
}
