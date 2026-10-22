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

#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/FileUtilities.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/ADT/SmallString.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/ADT/StringExtras.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/Error.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/ErrorOr.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/MemoryBuffer.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/Process.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/raw_ostream.h"
#ਅਨ੍ਤਰ੍ਭਾਵ <cmath>
#ਅਨ੍ਤਰ੍ਭਾਵ <cstdint>
#ਅਨ੍ਤਰ੍ਭਾਵ <cstdlib>
#ਅਨ੍ਤਰ੍ਭਾਵ <cstring>
#ਅਨ੍ਤਰ੍ਭਾਵ <memory>
#ਅਨ੍ਤਰ੍ਭਾਵ <system_error>

ਵਰਤੋਂ ਨਾਮਾਕਾਸ਼ llvm;

ਸਥਿਰ ਤਾਰਕਿਕ isSignedChar(ਅੱਖਰ C) {
  ਵਾਪਸ (C == '+' || C == '-');
}

ਸਥਿਰ ਤਾਰਕਿਕ isExponentChar(ਅੱਖਰ C) {
  ਚੋਣ (C) {
  ਹਾਲਤ 'D':  // Strange exponential notation.
  ਹਾਲਤ 'd':  // Strange exponential notation.
  ਹਾਲਤ 'e':
  ਹਾਲਤ 'E': ਵਾਪਸ ਸੱਚ;
  ਮੂਲ: ਵਾਪਸ ਝੂਠ;
  }
}

ਸਥਿਰ ਤਾਰਕਿਕ is_ਸੰਖਿਆ_Char(ਅੱਖਰ C) {
  ਚੋਣ (C) {
  ਹਾਲਤ '0': ਹਾਲਤ '1': ਹਾਲਤ '2': ਹਾਲਤ '3': ਹਾਲਤ '4':
  ਹਾਲਤ '5': ਹਾਲਤ '6': ਹਾਲਤ '7': ਹਾਲਤ '8': ਹਾਲਤ '9':
  ਹਾਲਤ '.': ਵਾਪਸ ਸੱਚ;
  ਮੂਲ: ਵਾਪਸ isSignedChar(C) || isExponentChar(C);
  }
}

ਸਥਿਰ ਅਚਲ ਅੱਖਰ *Backup_ਸੰਖਿਆ(ਅਚਲ ਅੱਖਰ *Pos, ਅਚਲ ਅੱਖਰ *ਪਹਿਲਾ_Char) {
  // If we didn't stop in the middle of a number, don't backup.
  ਜੇ (!is_ਸੰਖਿਆ_Char(*Pos)) ਵਾਪਸ Pos;

  // Otherwise, return to the start of the number.
  ਤਾਰਕਿਕ HasPeriod = ਝੂਠ;
  ਜਦੋਂ (Pos > ਪਹਿਲਾ_Char && is_ਸੰਖਿਆ_Char(Pos[-1])) {
    // Backup over at most one period.
    ਜੇ (Pos[-1] == '.') {
      ਜੇ (HasPeriod)
        ਰੋਕੋ;
      HasPeriod = ਸੱਚ;
    }

    --Pos;
    ਜੇ (Pos > ਪਹਿਲਾ_Char && isSignedChar(Pos[0]) && !isExponentChar(Pos[-1]))
      ਰੋਕੋ;
  }
  ਵਾਪਸ Pos;
}

/// EndOfNumber - Return the first character that is not part of the specified
/// number.  This assumes that the buffer is null terminated, so it won't fall
/// off the end.
ਸਥਿਰ ਅਚਲ ਅੱਖਰ *ਸਿਰਾ_ਦਾ_ਸੰਖਿਆ(ਅਚਲ ਅੱਖਰ *Pos) {
  ਜਦੋਂ (is_ਸੰਖਿਆ_Char(*Pos))
    ++Pos;
  ਵਾਪਸ Pos;
}

/// CompareNumbers - compare two numbers, returning true if they are different.
ਸਥਿਰ ਤਾਰਕਿਕ CompareNumbers(ਅਚਲ ਅੱਖਰ *&F1P, ਅਚਲ ਅੱਖਰ *&F2P,
                           ਅਚਲ ਅੱਖਰ *F1_ਸਿਰਾ, ਅਚਲ ਅੱਖਰ *F2_ਸਿਰਾ,
                           ਦੁੱਗਣਾ AbsTolerance, ਦੁੱਗਣਾ RelTolerance,
                           std::string *Error_ਸੁਨੇਹਾ) {
  ਅਚਲ ਅੱਖਰ *F1_ਅੰਕ_ਸਿਰਾ, *F2_ਅੰਕ_ਸਿਰਾ;
  ਦੁੱਗਣਾ V1 = 0.0, V2 = 0.0;

  // If one of the positions is at a space and the other isn't, chomp up 'til
  // the end of the space.
  ਜਦੋਂ (isSpace(static_cast<ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ>(*F1P)) && F1P != F1_ਸਿਰਾ)
    ++F1P;
  ਜਦੋਂ (isSpace(static_cast<ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ>(*F2P)) && F2P != F2_ਸਿਰਾ)
    ++F2P;

  // If we stop on numbers, compare their difference.
  ਜੇ (!is_ਸੰਖਿਆ_Char(*F1P) || !is_ਸੰਖਿਆ_Char(*F2P)) {
    // The diff failed.
    F1_ਅੰਕ_ਸਿਰਾ = F1P;
    F2_ਅੰਕ_ਸਿਰਾ = F2P;
  } ਨਹੀਂਤਾਂ {
    // Note that some ugliness is built into this to permit support for numbers
    // that use "D" or "d" as their exponential marker, e.g. "1.234D45".  This
    // occurs in 200.sixtrack in spec2k.
    V1 = strtod(F1P, const_cast<ਅੱਖਰ**>(&F1_ਅੰਕ_ਸਿਰਾ));
    V2 = strtod(F2P, const_cast<ਅੱਖਰ**>(&F2_ਅੰਕ_ਸਿਰਾ));

    ਜੇ (*F1_ਅੰਕ_ਸਿਰਾ == 'D' || *F1_ਅੰਕ_ਸਿਰਾ == 'd') {
      // Copy string into tmp buffer to replace the 'D' with an 'e'.
      SmallString<200> StrTmp(F1P, ਸਿਰਾ_ਦਾ_ਸੰਖਿਆ(F1_ਅੰਕ_ਸਿਰਾ)+1);
      // Strange exponential notation!
      StrTmp[static_cast<ਬਿਨਾਨਿਸ਼ਾਨੀ>(F1_ਅੰਕ_ਸਿਰਾ-F1P)] = 'e';

      V1 = strtod(&StrTmp[0], const_cast<ਅੱਖਰ**>(&F1_ਅੰਕ_ਸਿਰਾ));
      F1_ਅੰਕ_ਸਿਰਾ = F1P + (F1_ਅੰਕ_ਸਿਰਾ-&StrTmp[0]);
    }

    ਜੇ (*F2_ਅੰਕ_ਸਿਰਾ == 'D' || *F2_ਅੰਕ_ਸਿਰਾ == 'd') {
      // Copy string into tmp buffer to replace the 'D' with an 'e'.
      SmallString<200> StrTmp(F2P, ਸਿਰਾ_ਦਾ_ਸੰਖਿਆ(F2_ਅੰਕ_ਸਿਰਾ)+1);
      // Strange exponential notation!
      StrTmp[static_cast<ਬਿਨਾਨਿਸ਼ਾਨੀ>(F2_ਅੰਕ_ਸਿਰਾ-F2P)] = 'e';

      V2 = strtod(&StrTmp[0], const_cast<ਅੱਖਰ**>(&F2_ਅੰਕ_ਸਿਰਾ));
      F2_ਅੰਕ_ਸਿਰਾ = F2P + (F2_ਅੰਕ_ਸਿਰਾ-&StrTmp[0]);
    }
  }

  ਜੇ (F1_ਅੰਕ_ਸਿਰਾ == F1P || F2_ਅੰਕ_ਸਿਰਾ == F2P) {
    ਜੇ (Error_ਸੁਨੇਹਾ) {
      *Error_ਸੁਨੇਹਾ = "FP Comparison failed, not a numeric difference between '";
      *Error_ਸੁਨੇਹਾ += F1P[0];
      *Error_ਸੁਨੇਹਾ += "' and '";
      *Error_ਸੁਨੇਹਾ += F2P[0];
      *Error_ਸੁਨੇਹਾ += "'";
    }
    ਵਾਪਸ ਸੱਚ;
  }

  // Check to see if these are inside the absolute tolerance
  ਜੇ (AbsTolerance < std::ਨਿਰਪੇਖ(V1-V2)) {
    // Nope, check the relative tolerance...
    ਦੁੱਗਣਾ Diff;
    ਜੇ (V2)
      Diff = std::ਨਿਰਪੇਖ(V1/V2 - 1.0);
    ਨਹੀਂਤਾਂ ਜੇ (V1)
      Diff = std::ਨਿਰਪੇਖ(V2/V1 - 1.0);
    ਨਹੀਂਤਾਂ
      Diff = 0;  // Both zero.
    ਜੇ (Diff > RelTolerance) {
      ਜੇ (Error_ਸੁਨੇਹਾ) {
        raw_string_ostream(*Error_ਸੁਨੇਹਾ)
          << "Compared: " << V1 << " and " << V2 << '\n'
          << "abs. diff = " << std::ਨਿਰਪੇਖ(V1-V2) << " rel.diff = " << Diff << '\n'
          << "Out of tolerance: rel/abs: " << RelTolerance << '/'
          << AbsTolerance;
      }
      ਵਾਪਸ ਸੱਚ;
    }
  }

  // Otherwise, advance our read pointers to the end of the numbers.
  F1P = F1_ਅੰਕ_ਸਿਰਾ;  F2P = F2_ਅੰਕ_ਸਿਰਾ;
  ਵਾਪਸ ਝੂਠ;
}

/// DiffFilesWithTolerance - Compare the two files specified, returning 0 if the
/// files match, 1 if they are different, and 2 if there is a file error.  This
/// function differs from DiffFiles in that you can specify an absolute and
/// relative FP error that is allowed to exist.  If you specify a string to fill
/// in for the error option, it will set the string to an error message if an
/// error occurs, allowing the caller to distinguish between a failed diff and a
/// file system error.
///
ਪੂਰਨ llvm::DiffFilesWithTolerance(StringRef NameA,
                                 StringRef NameB,
                                 ਦੁੱਗਣਾ AbsTol, ਦੁੱਗਣਾ RelTol,
                                 std::string *Error) {
  // Now its safe to mmap the files into memory because both files
  // have a non-zero size.
  ErrorOr<std::unique_ptr<ਸਮ੍ਰਿਤੀ_ਬਫਰ>> F1OrErr = ਸਮ੍ਰਿਤੀ_ਬਫਰ::getFile(NameA);
  ਜੇ (std::error_ਕੋਡ EC = F1OrErr.getError()) {
    ਜੇ (Error)
      *Error = EC.ਸੁਨੇਹਾ_2();
    ਵਾਪਸ 2;
  }
  ਸਮ੍ਰਿਤੀ_ਬਫਰ &F1 = *F1OrErr.get();

  ErrorOr<std::unique_ptr<ਸਮ੍ਰਿਤੀ_ਬਫਰ>> F2OrErr = ਸਮ੍ਰਿਤੀ_ਬਫਰ::getFile(NameB);
  ਜੇ (std::error_ਕੋਡ EC = F2OrErr.getError()) {
    ਜੇ (Error)
      *Error = EC.ਸੁਨੇਹਾ_2();
    ਵਾਪਸ 2;
  }
  ਸਮ੍ਰਿਤੀ_ਬਫਰ &F2 = *F2OrErr.get();

  // Okay, now that we opened the files, scan them for the first difference.
  ਅਚਲ ਅੱਖਰ *File1_ਸ਼ੁਰੂ = F1.get_ਬਫਰ_ਸ਼ੁਰੂ();
  ਅਚਲ ਅੱਖਰ *File2_ਸ਼ੁਰੂ = F2.get_ਬਫਰ_ਸ਼ੁਰੂ();
  ਅਚਲ ਅੱਖਰ *File1_ਸਿਰਾ = F1.get_ਬਫਰ_ਸਿਰਾ();
  ਅਚਲ ਅੱਖਰ *File2_ਸਿਰਾ = F2.get_ਬਫਰ_ਸਿਰਾ();
  ਅਚਲ ਅੱਖਰ *F1P = File1_ਸ਼ੁਰੂ;
  ਅਚਲ ਅੱਖਰ *F2P = File2_ਸ਼ੁਰੂ;
  uint64_t A_ਮਾਪ = F1.get_ਬਫਰ_ਮਾਪ();
  uint64_t B_ਮਾਪ = F2.get_ਬਫਰ_ਮਾਪ();

  // Are the buffers identical?  Common case: Handle this efficiently.
  ਜੇ (A_ਮਾਪ == B_ਮਾਪ &&
      std::memcmp(File1_ਸ਼ੁਰੂ, File2_ਸ਼ੁਰੂ, A_ਮਾਪ) == 0)
    ਵਾਪਸ 0;

  // Otherwise, we are done a tolerances are set.
  ਜੇ (AbsTol == 0 && RelTol == 0) {
    ਜੇ (Error)
      *Error = "Files differ without tolerance allowance";
    ਵਾਪਸ 1;   // Files different!
  }

  ਤਾਰਕਿਕ CompareFailed = ਝੂਠ;
  ਜਦੋਂ (ਸੱਚ) {
    // Scan for the end of file or next difference.
    ਜਦੋਂ (F1P < File1_ਸਿਰਾ && F2P < File2_ਸਿਰਾ && *F1P == *F2P) {
      ++F1P;
      ++F2P;
    }

    ਜੇ (F1P >= File1_ਸਿਰਾ || F2P >= File2_ਸਿਰਾ) ਰੋਕੋ;

    // Okay, we must have found a difference.  Backup to the start of the
    // current number each stream is at so that we can compare from the
    // beginning.
    F1P = Backup_ਸੰਖਿਆ(F1P, File1_ਸ਼ੁਰੂ);
    F2P = Backup_ਸੰਖਿਆ(F2P, File2_ਸ਼ੁਰੂ);

    // Now that we are at the start of the numbers, compare them, exiting if
    // they don't match.
    ਜੇ (CompareNumbers(F1P, F2P, File1_ਸਿਰਾ, File2_ਸਿਰਾ, AbsTol, RelTol, Error)) {
      CompareFailed = ਸੱਚ;
      ਰੋਕੋ;
    }
  }

  // Okay, we reached the end of file.  If both files are at the end, we
  // succeeded.
  ਤਾਰਕਿਕ F1_At_ਸਿਰਾ = F1P >= File1_ਸਿਰਾ;
  ਤਾਰਕਿਕ F2_At_ਸਿਰਾ = F2P >= File2_ਸਿਰਾ;
  ਜੇ (!CompareFailed && (!F1_At_ਸਿਰਾ || !F2_At_ਸਿਰਾ)) {
    // Else, we might have run off the end due to a number: backup and retry.
    ਜੇ (F1_At_ਸਿਰਾ && is_ਸੰਖਿਆ_Char(F1P[-1])) --F1P;
    ਜੇ (F2_At_ਸਿਰਾ && is_ਸੰਖਿਆ_Char(F2P[-1])) --F2P;
    F1P = Backup_ਸੰਖਿਆ(F1P, File1_ਸ਼ੁਰੂ);
    F2P = Backup_ਸੰਖਿਆ(F2P, File2_ਸ਼ੁਰੂ);

    // Now that we are at the start of the numbers, compare them, exiting if
    // they don't match.
    ਜੇ (CompareNumbers(F1P, F2P, File1_ਸਿਰਾ, File2_ਸਿਰਾ, AbsTol, RelTol, Error))
      CompareFailed = ਸੱਚ;

    // If we found the end, we succeeded.
    ਜੇ (F1P < File1_ਸਿਰਾ || F2P < File2_ਸਿਰਾ)
      CompareFailed = ਸੱਚ;
  }

  ਵਾਪਸ CompareFailed;
}

Expected<FilePermissionsApplier>
FilePermissionsApplier::create(StringRef ਨਿਵੇਸ਼_Filename) {
  sys::fs::file_ਸਥਿਤੀ ਸਥਿਤੀ;

  ਜੇ (ਨਿਵੇਸ਼_Filename != "-") {
    ਜੇ (ਸਵੈਚਾਲਿਤ EC = sys::fs::ਸਥਿਤੀ_2(ਨਿਵੇਸ਼_Filename, ਸਥਿਤੀ))
      ਵਾਪਸ createFileError(ਨਿਵੇਸ਼_Filename, EC);
  } ਨਹੀਂਤਾਂ {
    ਸਥਿਤੀ.permissions(static_cast<sys::fs::perms>(0777));
  }

  ਵਾਪਸ FilePermissionsApplier(ਨਿਵੇਸ਼_Filename, ਸਥਿਤੀ);
}

Error FilePermissionsApplier::apply(
    StringRef OutputFilename, ਤਾਰਕਿਕ CopyDates,
    std::optional<sys::fs::perms> OverwritePermissions) {
  sys::fs::file_ਸਥਿਤੀ ਸਥਿਤੀ = ਨਿਵੇਸ਼_ਸਥਿਤੀ;

  ਜੇ (OverwritePermissions)
    ਸਥਿਤੀ.permissions(*OverwritePermissions);

  ਪੂਰਨ FD = 0;

  // Writing to stdout should not be treated as an error here, just
  // do not set access/modification times or permissions.
  ਜੇ (OutputFilename == "-")
    ਵਾਪਸ Error::success();

  ਜੇ (std::error_ਕੋਡ EC = sys::fs::openFileForWrite(OutputFilename, FD,
                                                     sys::fs::CD_OpenExisting))
    ਵਾਪਸ createFileError(OutputFilename, EC);

  ਜੇ (CopyDates)
    ਜੇ (std::error_ਕੋਡ EC = sys::fs::ਸੈੱਟ_ਆਖਰੀ_Access_And_Modification_ਸਮਾਂ(
            FD, ਸਥਿਤੀ.get_ਆਖਰੀ_Accessed_ਸਮਾਂ(), ਸਥਿਤੀ.get_ਆਖਰੀ_Modification_ਸਮਾਂ()))
      ਵਾਪਸ createFileError(OutputFilename, EC);

  sys::fs::file_ਸਥਿਤੀ OStat;
  ਜੇ (std::error_ਕੋਡ EC =
          sys::fs::ਸਥਿਤੀ_2(sys::fs::convertFDToNativeFile(FD), OStat))
    ਵਾਪਸ createFileError(OutputFilename, EC);
  ਜੇ (OStat.type() == sys::fs::file_type::regular_file) {
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ _WIN32
    // Keep ownership if llvm-objcopy is called under root.
    ਜੇ (OutputFilename == ਨਿਵੇਸ਼_Filename && OStat.getUser() == 0)
      sys::fs::changeFileOwnership(FD, ਸਥਿਤੀ.getUser(), ਸਥਿਤੀ.get_ਸਮੂਹ());
#ਜੇ_ਅੰਤ

    sys::fs::perms Perm = ਸਥਿਤੀ.permissions();
    ਜੇ (OutputFilename != ਨਿਵੇਸ਼_Filename)
      Perm = static_cast<sys::fs::perms>(Perm & ~sys::fs::getUmask() & ~06000);
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ _WIN32
    ਜੇ (std::error_ਕੋਡ EC = sys::fs::ਸੈੱਟ_Permissions(OutputFilename, Perm))
#ਨਹੀਂਤਾਂ
    ਜੇ (std::error_ਕੋਡ EC = sys::fs::ਸੈੱਟ_Permissions(FD, Perm))
#ਜੇ_ਅੰਤ
      ਵਾਪਸ createFileError(OutputFilename, EC);
  }

  ਜੇ (std::error_ਕੋਡ EC = sys::Process::SafelyCloseFileDescriptor(FD))
    ਵਾਪਸ createFileError(OutputFilename, EC);

  ਵਾਪਸ Error::success();
}
