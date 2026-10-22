//===-- Path.cpp - Implement OS Path Concept ------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
//  This file implements the operating system Path API.
//
//===----------------------------------------------------------------------===//

#ಅನ್ತರ್ಭಾವ "llvm/Support/Path.h"
#ಅನ್ತರ್ಭಾವ "llvm/ADT/ArrayRef.h"
#ಅನ್ತರ್ಭಾವ "llvm/ADT/ScopeExit.h"
#ಅನ್ತರ್ಭಾವ "llvm/ADT/StringExtras.h"
#ಅನ್ತರ್ಭಾವ "llvm/Config/config.h"
#ಅನ್ತರ್ಭಾವ "llvm/Config/llvm-config.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/Errc.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/ErrorHandling.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/FileSystem.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/IOSandbox.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/Process.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/Signals.h"
#ಅನ್ತರ್ಭಾವ <cctype>

#ಆದರೆ !defined(_MSC_VER) && !defined(__MINGW32__)
#ಅನ್ತರ್ಭಾವ <unistd.h>
#ಇಲ್ಲದಿದ್ದರೆ
#ಅನ್ತರ್ಭಾವ <io.h>
#ಆದರೆ_ಅಂತ್ಯ

ಬಳಸಿ ನಾಮಾಕಾಶ llvm;
ಬಳಸಿ ನಾಮಾಕಾಶ llvm::support::endian;

ನಾಮಾಕಾಶ {
  ಬಳಸಿ llvm::StringRef;
  ಬಳಸಿ llvm::sys::ಮಾರ್ಗ_2::is_separator;
  ಬಳಸಿ llvm::sys::ಮಾರ್ಗ_2::Style;

  ಸಾಲೊಳಗೆ ತಾರ್ಕಿಕ prefer_forward_slash() {
    ಸ್ಥಿರ ತಾರ್ಕಿಕ prefer = []() {
      ಆದರೆ (std::optional<std::string> Env =
              sys::Process::GetEnv("LLVM_WINDOWS_PREFER_FORWARD_SLASH"))
        ಹಿಂತಿರುಗಿ *Env == "1";
      ಹಿಂತಿರುಗಿ static_cast<ತಾರ್ಕಿಕ>(LLVM_WINDOWS_PREFER_FORWARD_SLASH);
    }();
    ಹಿಂತಿರುಗಿ prefer;
  }

  ಸಾಲೊಳಗೆ Style real_style(Style style) {
    ಆದರೆ (style != Style::native)
      ಹಿಂತಿರುಗಿ style;
    ಆದರೆ (is_style_posix(style))
      ಹಿಂತಿರುಗಿ Style::posix;
    ಹಿಂತಿರುಗಿ prefer_forward_slash() ? Style::windows_slash
                                  : Style::windows_backslash;
  }

  ಸಾಲೊಳಗೆ ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *separators(Style style) {
    ಆದರೆ (is_style_windows(style))
      ಹಿಂತಿರುಗಿ "\\/";
    ಹಿಂತಿರುಗಿ "/";
  }

  ಸಾಲೊಳಗೆ ಅಕ್ಷರ preferred_separator(Style style) {
    ಆದರೆ (real_style(style) == Style::windows)
      ಹಿಂತಿರುಗಿ '\\';
    ಹಿಂತಿರುಗಿ '/';
  }

  StringRef ಕಂಡುಹಿಡಿ_ಮೊದಲ_component(StringRef ಮಾರ್ಗ_2, Style style) {
    // Look for this first component in the following order.
    // * empty (in this case we return an empty string)
    // * either C: or {//,\\}net.
    // * {/,\}
    // * {file,directory}name

    ಆದರೆ (ಮಾರ್ಗ_2.ಖಾಲಿಗಳು())
      ಹಿಂತಿರುಗಿ ಮಾರ್ಗ_2;

    ಆದರೆ (is_style_windows(style)) {
      // C:
      ಆದರೆ (ಮಾರ್ಗ_2.ಗಾತ್ರ_3() >= 2 &&
          std::ಅಕ್ಷರವೇ(static_cast<ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ>(ಮಾರ್ಗ_2[0])) && ಮಾರ್ಗ_2[1] == ':')
        ಹಿಂತಿರುಗಿ ಮಾರ್ಗ_2.substr(0, 2);
    }

    // //net
    ಆದರೆ ((ಮಾರ್ಗ_2.ಗಾತ್ರ_3() > 2) && is_separator(ಮಾರ್ಗ_2[0], style) &&
        ಮಾರ್ಗ_2[0] == ಮಾರ್ಗ_2[1] && !is_separator(ಮಾರ್ಗ_2[2], style)) {
      // Find the next directory separator.
      size_t ಕೊನೆ = ಮಾರ್ಗ_2.ಕಂಡುಹಿಡಿ_ಮೊದಲ_ದ(separators(style), 2);
      ಹಿಂತಿರುಗಿ ಮಾರ್ಗ_2.substr(0, ಕೊನೆ);
    }

    // {/,\}
    ಆದರೆ (is_separator(ಮಾರ್ಗ_2[0], style))
      ಹಿಂತಿರುಗಿ ಮಾರ್ಗ_2.substr(0, 1);

    // * {file,directory}name
    size_t ಕೊನೆ = ಮಾರ್ಗ_2.ಕಂಡುಹಿಡಿ_ಮೊದಲ_ದ(separators(style));
    ಹಿಂತಿರುಗಿ ಮಾರ್ಗ_2.substr(0, ಕೊನೆ);
  }

  // Returns the first character of the filename in str. For paths ending in
  // '/', it returns the position of the '/'.
  size_t filename_pos(StringRef str, Style style) {
    ಆದರೆ (str.ಗಾತ್ರ_3() > 0 && is_separator(str[str.ಗಾತ್ರ_3() - 1], style))
      ಹಿಂತಿರುಗಿ str.ಗಾತ್ರ_3() - 1;

    size_t pos = str.ಕಂಡುಹಿಡಿ_ಕೊನೆಯ_ದ(separators(style), str.ಗಾತ್ರ_3() - 1);

    ಆದರೆ (is_style_windows(style)) {
      ಆದರೆ (pos == StringRef::npos)
        pos = str.ಕಂಡುಹಿಡಿ_ಕೊನೆಯ_ದ(':', str.ಗಾತ್ರ_3() - 1);
    }

    ಆದರೆ (pos == StringRef::npos || (pos == 1 && is_separator(str[0], style)))
      ಹಿಂತಿರುಗಿ 0;

    ಹಿಂತಿರುಗಿ pos + 1;
  }

  // Returns the position of the root directory in str. If there is no root
  // directory in str, it returns StringRef::npos.
  size_t ಬೇರು_dir_ಆರಂಭ(StringRef str, Style style) {
    // case "c:/"
    ಆದರೆ (is_style_windows(style)) {
      ಆದರೆ (str.ಗಾತ್ರ_3() > 2 && str[1] == ':' && is_separator(str[2], style))
        ಹಿಂತಿರುಗಿ 2;
    }

    // case "//net"
    ಆದರೆ (str.ಗಾತ್ರ_3() > 3 && is_separator(str[0], style) && str[0] == str[1] &&
        !is_separator(str[2], style)) {
      ಹಿಂತಿರುಗಿ str.ಕಂಡುಹಿಡಿ_ಮೊದಲ_ದ(separators(style), 2);
    }

    // case "/"
    ಆದರೆ (str.ಗಾತ್ರ_3() > 0 && is_separator(str[0], style))
      ಹಿಂತಿರುಗಿ 0;

    ಹಿಂತಿರುಗಿ StringRef::npos;
  }

  // Returns the position past the end of the "parent path" of path. The parent
  // path will not end in '/', unless the parent is the root directory. If the
  // path has no parent, 0 is returned.
  size_t ಪೋಷಕ_ಮಾರ್ಗ_ಕೊನೆ(StringRef ಮಾರ್ಗ_2, Style style) {
    size_t ಕೊನೆ_pos = filename_pos(ಮಾರ್ಗ_2, style);

    ತಾರ್ಕಿಕ filename_was_sep =
        ಮಾರ್ಗ_2.ಗಾತ್ರ_3() > 0 && is_separator(ಮಾರ್ಗ_2[ಕೊನೆ_pos], style);

    // Skip separators until we reach root dir (or the start of the string).
    size_t ಬೇರು_dir_pos = ಬೇರು_dir_ಆರಂಭ(ಮಾರ್ಗ_2, style);
    ಆಗ (ಕೊನೆ_pos > 0 &&
           (ಬೇರು_dir_pos == StringRef::npos || ಕೊನೆ_pos > ಬೇರು_dir_pos) &&
           is_separator(ಮಾರ್ಗ_2[ಕೊನೆ_pos - 1], style))
      --ಕೊನೆ_pos;

    ಆದರೆ (ಕೊನೆ_pos == ಬೇರು_dir_pos && !filename_was_sep) {
      // We've reached the root dir and the input path was *not* ending in a
      // sequence of slashes. Include the root dir in the parent path.
      ಹಿಂತಿರುಗಿ ಬೇರು_dir_pos + 1;
    }

    // Otherwise, just include before the last slash.
    ಹಿಂತಿರುಗಿ ಕೊನೆ_pos;
  }
} // end unnamed namespace

ಎಣಿಕೆ FSEntity {
  FS_Dir,
  FS_File,
  FS_Name
};

ಸ್ಥಿರ std::error_ಸಂಕೇತ
createUniqueEntity(ಸ್ಥಿರಾಂಕ Twine &Model, ಪೂರ್ಣಾಂಕ &ಫಲಿತಾಂಶ_FD,
                   Small_ಸದಿಶ_Impl<ಅಕ್ಷರ> &ಫಲಿತಾಂಶ_ಮಾರ್ಗ, ತಾರ್ಕಿಕ MakeAbsolute,
                   FSEntity Type, sys::fs::OpenFlags Flags = sys::fs::ದ_None,
                   ಚಿಹ್ನೆರಹಿತ ಬಹುಲಕ = 0) {

  // Limit the number of attempts we make, so that we don't infinite loop. E.g.
  // "permission denied" could be for a specific file (so we retry with a
  // different name) or for the whole directory (retry would always fail).
  // Checking which is racy, so we try a number of times, then give up.
  std::error_ಸಂಕೇತ EC;
  ಗಾಗಿ (ಪೂರ್ಣಾಂಕ Retries = 128; Retries > 0; --Retries) {
    sys::fs::create_Unique_ಮಾರ್ಗ(Model, ಫಲಿತಾಂಶ_ಮಾರ್ಗ, MakeAbsolute);
    // Try to open + create the file.
    ಆಯ್ಕೆ (Type) {
    ಸ್ಥಿತಿ FS_File: {
      EC = sys::fs::openFileForReadWrite(Twine(ಫಲಿತಾಂಶ_ಮಾರ್ಗ.begin()), ಫಲಿತಾಂಶ_FD,
                                         sys::fs::CD_CreateNew, Flags, ಬಹುಲಕ);
      ಆದರೆ (EC) {
        // errc::permission_denied happens on Windows when we try to open a file
        // that has been marked for deletion.
        ಆದರೆ (EC == errc::file_exists || EC == errc::permission_denied)
          ಮುಂದುವರಿ;
        ಹಿಂತಿರುಗಿ EC;
      }

      ಹಿಂತಿರುಗಿ std::error_ಸಂಕೇತ();
    }

    ಸ್ಥಿತಿ FS_Name: {
      EC = sys::fs::access(ಫಲಿತಾಂಶ_ಮಾರ್ಗ.begin(), sys::fs::Access_ಬಹುಲಕ::Exist);
      ಆದರೆ (EC == errc::no_such_file_or_directory)
        ಹಿಂತಿರುಗಿ std::error_ಸಂಕೇತ();
      ಆದರೆ (EC)
        ಹಿಂತಿರುಗಿ EC;
      ಮುಂದುವರಿ;
    }

    ಸ್ಥಿತಿ FS_Dir: {
      EC = sys::fs::create_directory(ಫಲಿತಾಂಶ_ಮಾರ್ಗ.begin(), ಸುಳ್ಳು);
      ಆದರೆ (EC) {
        ಆದರೆ (EC == errc::file_exists)
          ಮುಂದುವರಿ;
        ಹಿಂತಿರುಗಿ EC;
      }
      ಹಿಂತಿರುಗಿ std::error_ಸಂಕೇತ();
    }
    }
    llvm_unreachable("Invalid Type");
  }
  ಹಿಂತಿರುಗಿ EC;
}

ನಾಮಾಕಾಶ llvm {
ನಾಮಾಕಾಶ sys  {
ನಾಮಾಕಾಶ ಮಾರ್ಗ_2 {

const_iterator begin(StringRef ಮಾರ್ಗ_2, Style style) {
  const_iterator i;
  i.ಮಾರ್ಗ      = ಮಾರ್ಗ_2;
  i.Component = ಕಂಡುಹಿಡಿ_ಮೊದಲ_component(ಮಾರ್ಗ_2, style);
  i.Position  = 0;
  i.S = style;
  ಹಿಂತಿರುಗಿ i;
}

const_iterator ಕೊನೆ(StringRef ಮಾರ್ಗ_2) {
  const_iterator i;
  i.ಮಾರ್ಗ      = ಮಾರ್ಗ_2;
  i.Position  = ಮಾರ್ಗ_2.ಗಾತ್ರ_3();
  ಹಿಂತಿರುಗಿ i;
}

const_iterator &const_iterator::ಕ್ರಿಯಾಕಾರಕ++() {
  ಪ್ರತಿಜ್ಞಾ(Position < ಮಾರ್ಗ.ಗಾತ್ರ_3() && "Tried to increment past end!");

  // Increment Position to past the current component
  Position += Component.ಗಾತ್ರ_3();

  // Check for end.
  ಆದರೆ (Position == ಮಾರ್ಗ.ಗಾತ್ರ_3()) {
    Component = StringRef();
    ಹಿಂತಿರುಗಿ *ಇದು;
  }

  // Both POSIX and Windows treat paths that begin with exactly two separators
  // specially.
  ತಾರ್ಕಿಕ was_net = Component.ಗಾತ್ರ_3() > 2 && is_separator(Component[0], S) &&
                 Component[1] == Component[0] && !is_separator(Component[2], S);

  // Handle separators.
  ಆದರೆ (is_separator(ಮಾರ್ಗ[Position], S)) {
    // Root dir.
    ಆದರೆ (was_net ||
        // c:/
        (is_style_windows(S) && Component.ತುದಿಗಳು_with(":"))) {
      Component = ಮಾರ್ಗ.substr(Position, 1);
      ಹಿಂತಿರುಗಿ *ಇದು;
    }

    // Skip extra separators.
    ಆಗ (Position != ಮಾರ್ಗ.ಗಾತ್ರ_3() && is_separator(ಮಾರ್ಗ[Position], S)) {
      ++Position;
    }

    // Treat trailing '/' as a '.', unless it is the root dir.
    ಆದರೆ (Position == ಮಾರ್ಗ.ಗಾತ್ರ_3() && Component != "/") {
      --Position;
      Component = ".";
      ಹಿಂತಿರುಗಿ *ಇದು;
    }
  }

  // Find next component.
  size_t ಕೊನೆ_pos = ಮಾರ್ಗ.ಕಂಡುಹಿಡಿ_ಮೊದಲ_ದ(separators(S), Position);
  Component = ಮಾರ್ಗ.ತುಂಡು(Position, ಕೊನೆ_pos);

  ಹಿಂತಿರುಗಿ *ಇದು;
}

ತಾರ್ಕಿಕ const_iterator::ಕ್ರಿಯಾಕಾರಕ==(ಸ್ಥಿರಾಂಕ const_iterator &RHS) ಸ್ಥಿರಾಂಕ {
  ಹಿಂತಿರುಗಿ ಮಾರ್ಗ.begin() == RHS.ಮಾರ್ಗ.begin() && Position == RHS.Position;
}

ptrdiff_t const_iterator::ಕ್ರಿಯಾಕಾರಕ-(ಸ್ಥಿರಾಂಕ const_iterator &RHS) ಸ್ಥಿರಾಂಕ {
  ಹಿಂತಿರುಗಿ Position - RHS.Position;
}

ಹಿಮ್ಮುಖ_iterator rbegin(StringRef ಮಾರ್ಗ, Style style) {
  ಹಿಮ್ಮುಖ_iterator I;
  I.ಮಾರ್ಗ = ಮಾರ್ಗ;
  I.Position = ಮಾರ್ಗ.ಗಾತ್ರ_3();
  I.S = style;
  ++I;
  ಹಿಂತಿರುಗಿ I;
}

ಹಿಮ್ಮುಖ_iterator rend(StringRef ಮಾರ್ಗ) {
  ಹಿಮ್ಮುಖ_iterator I;
  I.ಮಾರ್ಗ = ಮಾರ್ಗ;
  I.Component = ಮಾರ್ಗ.substr(0, 0);
  I.Position = 0;
  ಹಿಂತಿರುಗಿ I;
}

ಹಿಮ್ಮುಖ_iterator &ಹಿಮ್ಮುಖ_iterator::ಕ್ರಿಯಾಕಾರಕ++() {
  size_t ಬೇರು_dir_pos = ಬೇರು_dir_ಆರಂಭ(ಮಾರ್ಗ, S);

  // Skip separators unless it's the root directory.
  size_t ಕೊನೆ_pos = Position;
  ಆಗ (ಕೊನೆ_pos > 0 && (ಕೊನೆ_pos - 1) != ಬೇರು_dir_pos &&
         is_separator(ಮಾರ್ಗ[ಕೊನೆ_pos - 1], S))
    --ಕೊನೆ_pos;

  // Treat trailing '/' as a '.', unless it is the root dir.
  ಆದರೆ (Position == ಮಾರ್ಗ.ಗಾತ್ರ_3() && !ಮಾರ್ಗ.ಖಾಲಿಗಳು() &&
      is_separator(ಮಾರ್ಗ.ಹಿಂಭಾಗ(), S) &&
      (ಬೇರು_dir_pos == StringRef::npos || ಕೊನೆ_pos - 1 > ಬೇರು_dir_pos)) {
    --Position;
    Component = ".";
    ಹಿಂತಿರುಗಿ *ಇದು;
  }

  // Find next separator.
  size_t ಆರಂಭ_pos = filename_pos(ಮಾರ್ಗ.substr(0, ಕೊನೆ_pos), S);
  Component = ಮಾರ್ಗ.ತುಂಡು(ಆರಂಭ_pos, ಕೊನೆ_pos);
  Position = ಆರಂಭ_pos;
  ಹಿಂತಿರುಗಿ *ಇದು;
}

ತಾರ್ಕಿಕ ಹಿಮ್ಮುಖ_iterator::ಕ್ರಿಯಾಕಾರಕ==(ಸ್ಥಿರಾಂಕ ಹಿಮ್ಮುಖ_iterator &RHS) ಸ್ಥಿರಾಂಕ {
  ಹಿಂತಿರುಗಿ ಮಾರ್ಗ.begin() == RHS.ಮಾರ್ಗ.begin() && Component == RHS.Component &&
         Position == RHS.Position;
}

ptrdiff_t ಹಿಮ್ಮುಖ_iterator::ಕ್ರಿಯಾಕಾರಕ-(ಸ್ಥಿರಾಂಕ ಹಿಮ್ಮುಖ_iterator &RHS) ಸ್ಥಿರಾಂಕ {
  ಹಿಂತಿರುಗಿ Position - RHS.Position;
}

StringRef ಬೇರು_ಮಾರ್ಗ_2(StringRef ಮಾರ್ಗ_2, Style style) {
  const_iterator b = begin(ಮಾರ್ಗ_2, style), pos = b, e = ಕೊನೆ(ಮಾರ್ಗ_2);
  ಆದರೆ (b != e) {
    ತಾರ್ಕಿಕ has_net =
        b->ಗಾತ್ರ_3() > 2 && is_separator((*b)[0], style) && (*b)[1] == (*b)[0];
    ತಾರ್ಕಿಕ has_drive = is_style_windows(style) && b->ತುದಿಗಳು_with(":");

    ಆದರೆ (has_net || has_drive) {
      ಆದರೆ ((++pos != e) && is_separator((*pos)[0], style)) {
        // {C:/,//net/}, so get the first two components.
        ಹಿಂತಿರುಗಿ ಮಾರ್ಗ_2.substr(0, b->ಗಾತ್ರ_3() + pos->ಗಾತ್ರ_3());
      }
      // just {C:,//net}, return the first component.
      ಹಿಂತಿರುಗಿ *b;
    }

    // POSIX style root directory.
    ಆದರೆ (is_separator((*b)[0], style)) {
      ಹಿಂತಿರುಗಿ *b;
    }
  }

  ಹಿಂತಿರುಗಿ StringRef();
}

StringRef ಬೇರು_name(StringRef ಮಾರ್ಗ_2, Style style) {
  const_iterator b = begin(ಮಾರ್ಗ_2, style), e = ಕೊನೆ(ಮಾರ್ಗ_2);
  ಆದರೆ (b != e) {
    ತಾರ್ಕಿಕ has_net =
        b->ಗಾತ್ರ_3() > 2 && is_separator((*b)[0], style) && (*b)[1] == (*b)[0];
    ತಾರ್ಕಿಕ has_drive = is_style_windows(style) && b->ತುದಿಗಳು_with(":");

    ಆದರೆ (has_net || has_drive) {
      // just {C:,//net}, return the first component.
      ಹಿಂತಿರುಗಿ *b;
    }
  }

  // No path or no name.
  ಹಿಂತಿರುಗಿ StringRef();
}

StringRef ಬೇರು_directory(StringRef ಮಾರ್ಗ_2, Style style) {
  const_iterator b = begin(ಮಾರ್ಗ_2, style), pos = b, e = ಕೊನೆ(ಮಾರ್ಗ_2);
  ಆದರೆ (b != e) {
    ತಾರ್ಕಿಕ has_net =
        b->ಗಾತ್ರ_3() > 2 && is_separator((*b)[0], style) && (*b)[1] == (*b)[0];
    ತಾರ್ಕಿಕ has_drive = is_style_windows(style) && b->ತುದಿಗಳು_with(":");

    ಆದರೆ ((has_net || has_drive) &&
        // {C:,//net}, skip to the next component.
        (++pos != e) && is_separator((*pos)[0], style)) {
      ಹಿಂತಿರುಗಿ *pos;
    }

    // POSIX style root directory.
    ಆದರೆ (!has_net && is_separator((*b)[0], style)) {
      ಹಿಂತಿರುಗಿ *b;
    }
  }

  // No path or no root.
  ಹಿಂತಿರುಗಿ StringRef();
}

StringRef relative_ಮಾರ್ಗ(StringRef ಮಾರ್ಗ_2, Style style) {
  StringRef ಬೇರು_3 = ಬೇರು_ಮಾರ್ಗ_2(ಮಾರ್ಗ_2, style);
  ಹಿಂತಿರುಗಿ ಮಾರ್ಗ_2.substr(ಬೇರು_3.ಗಾತ್ರ_3());
}

ಶೂನ್ಯ append(Small_ಸದಿಶ_Impl<ಅಕ್ಷರ> &ಮಾರ್ಗ_2, Style style, ಸ್ಥಿರಾಂಕ Twine &a,
            ಸ್ಥಿರಾಂಕ Twine &b, ಸ್ಥಿರಾಂಕ Twine &c, ಸ್ಥಿರಾಂಕ Twine &d) {
  SmallString<32> a_storage;
  SmallString<32> b_storage;
  SmallString<32> c_storage;
  SmallString<32> d_storage;

  Small_ಸದಿಶ<StringRef, 4> components;
  ಆದರೆ (!a.is_Trivially_ಖಾಲಿಗಳು()) components.ತಳ್ಳು_ಹಿಂಭಾಗ(a.ಗಮ್ಯ_String_Ref(a_storage));
  ಆದರೆ (!b.is_Trivially_ಖಾಲಿಗಳು()) components.ತಳ್ಳು_ಹಿಂಭಾಗ(b.ಗಮ್ಯ_String_Ref(b_storage));
  ಆದರೆ (!c.is_Trivially_ಖಾಲಿಗಳು()) components.ತಳ್ಳು_ಹಿಂಭಾಗ(c.ಗಮ್ಯ_String_Ref(c_storage));
  ಆದರೆ (!d.is_Trivially_ಖಾಲಿಗಳು()) components.ತಳ್ಳು_ಹಿಂಭಾಗ(d.ಗಮ್ಯ_String_Ref(d_storage));

  ಗಾಗಿ (ಸ್ವಯಂಚಾಲಿತ &component : components) {
    ತಾರ್ಕಿಕ ಮಾರ್ಗ_has_sep =
        !ಮಾರ್ಗ_2.ಖಾಲಿಗಳು() && is_separator(ಮಾರ್ಗ_2[ಮಾರ್ಗ_2.ಗಾತ್ರ_3() - 1], style);
    ಆದರೆ (ಮಾರ್ಗ_has_sep) {
      // Strip separators from beginning of component.
      size_t loc = component.ಕಂಡುಹಿಡಿ_ಮೊದಲ_not_ದ(separators(style));
      StringRef c = component.substr(loc);

      // Append it.
      ಮಾರ್ಗ_2.append(c.begin(), c.ಕೊನೆ());
      ಮುಂದುವರಿ;
    }

    ತಾರ್ಕಿಕ component_has_sep =
        !component.ಖಾಲಿಗಳು() && is_separator(component[0], style);
    ಆದರೆ (!component_has_sep &&
        !(ಮಾರ್ಗ_2.ಖಾಲಿಗಳು() || has_ಬೇರು_name(component, style))) {
      // Add a separator.
      ಮಾರ್ಗ_2.ತಳ್ಳು_ಹಿಂಭಾಗ(preferred_separator(style));
    }

    ಮಾರ್ಗ_2.append(component.begin(), component.ಕೊನೆ());
  }
}

ಶೂನ್ಯ append(Small_ಸದಿಶ_Impl<ಅಕ್ಷರ> &ಮಾರ್ಗ_2, ಸ್ಥಿರಾಂಕ Twine &a, ಸ್ಥಿರಾಂಕ Twine &b,
            ಸ್ಥಿರಾಂಕ Twine &c, ಸ್ಥಿರಾಂಕ Twine &d) {
  append(ಮಾರ್ಗ_2, Style::native, a, b, c, d);
}

ಶೂನ್ಯ append(Small_ಸದಿಶ_Impl<ಅಕ್ಷರ> &ಮಾರ್ಗ_2, const_iterator begin,
            const_iterator ಕೊನೆ, Style style) {
  ಗಾಗಿ (; begin != ಕೊನೆ; ++begin)
    ಮಾರ್ಗ_2::append(ಮಾರ್ಗ_2, style, *begin);
}

StringRef ಪೋಷಕ_ಮಾರ್ಗ(StringRef ಮಾರ್ಗ_2, Style style) {
  size_t ಕೊನೆ_pos = ಪೋಷಕ_ಮಾರ್ಗ_ಕೊನೆ(ಮಾರ್ಗ_2, style);
  ಆದರೆ (ಕೊನೆ_pos == StringRef::npos)
    ಹಿಂತಿರುಗಿ StringRef();
  ಹಿಂತಿರುಗಿ ಮಾರ್ಗ_2.substr(0, ಕೊನೆ_pos);
}

ಶೂನ್ಯ remove_filename(Small_ಸದಿಶ_Impl<ಅಕ್ಷರ> &ಮಾರ್ಗ_2, Style style) {
  size_t ಕೊನೆ_pos = ಪೋಷಕ_ಮಾರ್ಗ_ಕೊನೆ(StringRef(ಮಾರ್ಗ_2.begin(), ಮಾರ್ಗ_2.ಗಾತ್ರ_3()), style);
  ಆದರೆ (ಕೊನೆ_pos != StringRef::npos)
    ಮಾರ್ಗ_2.truncate(ಕೊನೆ_pos);
}

ಶೂನ್ಯ replace_extension(Small_ಸದಿಶ_Impl<ಅಕ್ಷರ> &ಮಾರ್ಗ_2, ಸ್ಥಿರಾಂಕ Twine &extension,
                       Style style) {
  StringRef p(ಮಾರ್ಗ_2.begin(), ಮಾರ್ಗ_2.ಗಾತ್ರ_3());
  SmallString<32> ext_storage;
  StringRef ext = extension.ಗಮ್ಯ_String_Ref(ext_storage);

  // Erase existing extension.
  size_t pos = p.ಕಂಡುಹಿಡಿ_ಕೊನೆಯ_ದ('.');
  ಆದರೆ (pos != StringRef::npos && pos >= filename_pos(p, style))
    ಮಾರ್ಗ_2.truncate(pos);

  // Append '.' if needed.
  ಆದರೆ (ext.ಗಾತ್ರ_3() > 0 && ext[0] != '.')
    ಮಾರ್ಗ_2.ತಳ್ಳು_ಹಿಂಭಾಗ('.');

  // Append extension.
  ಮಾರ್ಗ_2.append(ext.begin(), ext.ಕೊನೆ());
}

ಸ್ಥಿರ ತಾರ್ಕಿಕ starts_with(StringRef ಮಾರ್ಗ, StringRef ಪೂರ್ವಪ್ರತ್ಯಯ,
                        Style style = Style::native) {
  // Windows prefix matching : case and separator insensitive
  ಆದರೆ (is_style_windows(style)) {
    ಆದರೆ (ಮಾರ್ಗ.ಗಾತ್ರ_3() < ಪೂರ್ವಪ್ರತ್ಯಯ.ಗಾತ್ರ_3())
      ಹಿಂತಿರುಗಿ ಸುಳ್ಳು;
    ಗಾಗಿ (size_t I = 0, E = ಪೂರ್ವಪ್ರತ್ಯಯ.ಗಾತ್ರ_3(); I != E; ++I) {
      ತಾರ್ಕಿಕ Sep_ಮಾರ್ಗ = is_separator(ಮಾರ್ಗ[I], style);
      ತಾರ್ಕಿಕ Sep_ಪೂರ್ವಪ್ರತ್ಯಯ = is_separator(ಪೂರ್ವಪ್ರತ್ಯಯ[I], style);
      ಆದರೆ (Sep_ಮಾರ್ಗ != Sep_ಪೂರ್ವಪ್ರತ್ಯಯ)
        ಹಿಂತಿರುಗಿ ಸುಳ್ಳು;
      ಆದರೆ (!Sep_ಮಾರ್ಗ && ಗಮ್ಯ_ಅಧೋ(ಮಾರ್ಗ[I]) != ಗಮ್ಯ_ಅಧೋ(ಪೂರ್ವಪ್ರತ್ಯಯ[I]))
        ಹಿಂತಿರುಗಿ ಸುಳ್ಳು;
    }
    ಹಿಂತಿರುಗಿ ನಿಜ;
  }
  ಹಿಂತಿರುಗಿ ಮಾರ್ಗ.starts_with(ಪೂರ್ವಪ್ರತ್ಯಯ);
}

ತಾರ್ಕಿಕ replace_ಮಾರ್ಗ_ಪೂರ್ವಪ್ರತ್ಯಯ(Small_ಸದಿಶ_Impl<ಅಕ್ಷರ> &ಮಾರ್ಗ, StringRef Old_ಪೂರ್ವಪ್ರತ್ಯಯ,
                         StringRef New_ಪೂರ್ವಪ್ರತ್ಯಯ, Style style) {
  ಆದರೆ (Old_ಪೂರ್ವಪ್ರತ್ಯಯ.ಖಾಲಿಗಳು() && New_ಪೂರ್ವಪ್ರತ್ಯಯ.ಖಾಲಿಗಳು())
    ಹಿಂತಿರುಗಿ ಸುಳ್ಳು;

  StringRef Orig_ಮಾರ್ಗ(ಮಾರ್ಗ.begin(), ಮಾರ್ಗ.ಗಾತ್ರ_3());
  ಆದರೆ (!starts_with(Orig_ಮಾರ್ಗ, Old_ಪೂರ್ವಪ್ರತ್ಯಯ, style))
    ಹಿಂತಿರುಗಿ ಸುಳ್ಳು;

  // If prefixes have the same size we can simply copy the new one over.
  ಆದರೆ (Old_ಪೂರ್ವಪ್ರತ್ಯಯ.ಗಾತ್ರ_3() == New_ಪೂರ್ವಪ್ರತ್ಯಯ.ಗಾತ್ರ_3()) {
    llvm::copy(New_ಪೂರ್ವಪ್ರತ್ಯಯ, ಮಾರ್ಗ.begin());
    ಹಿಂತಿರುಗಿ ನಿಜ;
  }

  StringRef Rel_ಮಾರ್ಗ = Orig_ಮಾರ್ಗ.substr(Old_ಪೂರ್ವಪ್ರತ್ಯಯ.ಗಾತ್ರ_3());
  SmallString<256> New_ಮಾರ್ಗ;
  (Twine(New_ಪೂರ್ವಪ್ರತ್ಯಯ) + Rel_ಮಾರ್ಗ).ಗಮ್ಯ_ಸದಿಶ(New_ಮಾರ್ಗ);
  ಮಾರ್ಗ.ಅದಲುಬದಲು(New_ಮಾರ್ಗ);
  ಹಿಂತಿರುಗಿ ನಿಜ;
}

ಶೂನ್ಯ native(ಸ್ಥಿರಾಂಕ Twine &ಮಾರ್ಗ_2, Small_ಸದಿಶ_Impl<ಅಕ್ಷರ> &ಫಲಿತಾಂಶ_2, Style style) {
  ಪ್ರತಿಜ್ಞಾ((!ಮಾರ್ಗ_2.isSingleStringRef() ||
          ಮಾರ್ಗ_2.getSingleStringRef().ದತ್ತಾಂಶ() != ಫಲಿತಾಂಶ_2.ದತ್ತಾಂಶ()) &&
         "path and result are not allowed to overlap!");
  // Clear result.
  ಫಲಿತಾಂಶ_2.clear();
  ಮಾರ್ಗ_2.ಗಮ್ಯ_ಸದಿಶ(ಫಲಿತಾಂಶ_2);
  native(ಫಲಿತಾಂಶ_2, style);
}

std::string native(ಸ್ಥಿರಾಂಕ Twine &ಮಾರ್ಗ_2, Style style) {
  SmallString<128> ಫಲಿತಾಂಶ;
  native(ಮಾರ್ಗ_2, ಫಲಿತಾಂಶ, style);
  ಹಿಂತಿರುಗಿ std::string(ಫಲಿತಾಂಶ);
}

ಶೂನ್ಯ native(Small_ಸದಿಶ_Impl<ಅಕ್ಷರ> &ಮಾರ್ಗ, Style style) {
  ಆದರೆ (ಮಾರ್ಗ.ಖಾಲಿಗಳು())
    ಹಿಂತಿರುಗಿ;
  ಆದರೆ (is_style_windows(style)) {
    ಗಾಗಿ (ಅಕ್ಷರ &Ch : ಮಾರ್ಗ)
      ಆದರೆ (is_separator(Ch, style))
        Ch = preferred_separator(style);
    ಆದರೆ (ಮಾರ್ಗ[0] == '~' && (ಮಾರ್ಗ.ಗಾತ್ರ_3() == 1 || is_separator(ಮಾರ್ಗ[1], style))) {
      SmallString<128> ಮಾರ್ಗ_Home;
      home_directory(ಮಾರ್ಗ_Home);
      ಮಾರ್ಗ_Home.append(ಮಾರ್ಗ.begin() + 1, ಮಾರ್ಗ.ಕೊನೆ());
      ಮಾರ್ಗ = std::move(ಮಾರ್ಗ_Home);
    }
  } ಇಲ್ಲದಿದ್ದರೆ {
    llvm::replace(ಮಾರ್ಗ, '\\', '/');
  }
}

std::string convert_ಗಮ್ಯ_slash(StringRef ಮಾರ್ಗ_2, Style style) {
  ಆದರೆ (is_style_posix(style))
    ಹಿಂತಿರುಗಿ std::string(ಮಾರ್ಗ_2);

  std::string s = ಮಾರ್ಗ_2.str();
  llvm::replace(s, '\\', '/');
  ಹಿಂತಿರುಗಿ s;
}

StringRef filename(StringRef ಮಾರ್ಗ_2, Style style) { ಹಿಂತಿರುಗಿ *rbegin(ಮಾರ್ಗ_2, style); }

StringRef stem(StringRef ಮಾರ್ಗ_2, Style style) {
  StringRef fname = filename(ಮಾರ್ಗ_2, style);
  size_t pos = fname.ಕಂಡುಹಿಡಿ_ಕೊನೆಯ_ದ('.');
  ಆದರೆ (pos == StringRef::npos)
    ಹಿಂತಿರುಗಿ fname;
  ಆದರೆ ((fname.ಗಾತ್ರ_3() == 1 && fname == ".") ||
      (fname.ಗಾತ್ರ_3() == 2 && fname == ".."))
    ಹಿಂತಿರುಗಿ fname;
  ಹಿಂತಿರುಗಿ fname.substr(0, pos);
}

StringRef extension(StringRef ಮಾರ್ಗ_2, Style style) {
  StringRef fname = filename(ಮಾರ್ಗ_2, style);
  size_t pos = fname.ಕಂಡುಹಿಡಿ_ಕೊನೆಯ_ದ('.');
  ಆದರೆ (pos == StringRef::npos)
    ಹಿಂತಿರುಗಿ StringRef();
  ಆದರೆ ((fname.ಗಾತ್ರ_3() == 1 && fname == ".") ||
      (fname.ಗಾತ್ರ_3() == 2 && fname == ".."))
    ಹಿಂತಿರುಗಿ StringRef();
  ಹಿಂತಿರುಗಿ fname.substr(pos);
}

ತಾರ್ಕಿಕ is_separator(ಅಕ್ಷರ ಮೌಲ್ಯ_2, Style style) {
  ಆದರೆ (ಮೌಲ್ಯ_2 == '/')
    ಹಿಂತಿರುಗಿ ನಿಜ;
  ಆದರೆ (is_style_windows(style))
    ಹಿಂತಿರುಗಿ ಮೌಲ್ಯ_2 == '\\';
  ಹಿಂತಿರುಗಿ ಸುಳ್ಳು;
}

StringRef get_separator(Style style) {
  ಆದರೆ (real_style(style) == Style::windows)
    ಹಿಂತಿರುಗಿ "\\";
  ಹಿಂತಿರುಗಿ "/";
}

ತಾರ್ಕಿಕ has_ಬೇರು_name(ಸ್ಥಿರಾಂಕ Twine &ಮಾರ್ಗ_2, Style style) {
  SmallString<128> ಮಾರ್ಗ_storage;
  StringRef p = ಮಾರ್ಗ_2.ಗಮ್ಯ_String_Ref(ಮಾರ್ಗ_storage);

  ಹಿಂತಿರುಗಿ !ಬೇರು_name(p, style).ಖಾಲಿಗಳು();
}

ತಾರ್ಕಿಕ has_ಬೇರು_directory(ಸ್ಥಿರಾಂಕ Twine &ಮಾರ್ಗ_2, Style style) {
  SmallString<128> ಮಾರ್ಗ_storage;
  StringRef p = ಮಾರ್ಗ_2.ಗಮ್ಯ_String_Ref(ಮಾರ್ಗ_storage);

  ಹಿಂತಿರುಗಿ !ಬೇರು_directory(p, style).ಖಾಲಿಗಳು();
}

ತಾರ್ಕಿಕ has_ಬೇರು_ಮಾರ್ಗ(ಸ್ಥಿರಾಂಕ Twine &ಮಾರ್ಗ_2, Style style) {
  SmallString<128> ಮಾರ್ಗ_storage;
  StringRef p = ಮಾರ್ಗ_2.ಗಮ್ಯ_String_Ref(ಮಾರ್ಗ_storage);

  ಹಿಂತಿರುಗಿ !ಬೇರು_ಮಾರ್ಗ_2(p, style).ಖಾಲಿಗಳು();
}

ತಾರ್ಕಿಕ has_relative_ಮಾರ್ಗ(ಸ್ಥಿರಾಂಕ Twine &ಮಾರ್ಗ_2, Style style) {
  SmallString<128> ಮಾರ್ಗ_storage;
  StringRef p = ಮಾರ್ಗ_2.ಗಮ್ಯ_String_Ref(ಮಾರ್ಗ_storage);

  ಹಿಂತಿರುಗಿ !relative_ಮಾರ್ಗ(p, style).ಖಾಲಿಗಳು();
}

ತಾರ್ಕಿಕ has_filename(ಸ್ಥಿರಾಂಕ Twine &ಮಾರ್ಗ_2, Style style) {
  SmallString<128> ಮಾರ್ಗ_storage;
  StringRef p = ಮಾರ್ಗ_2.ಗಮ್ಯ_String_Ref(ಮಾರ್ಗ_storage);

  ಹಿಂತಿರುಗಿ !filename(p, style).ಖಾಲಿಗಳು();
}

ತಾರ್ಕಿಕ has_ಪೋಷಕ_ಮಾರ್ಗ(ಸ್ಥಿರಾಂಕ Twine &ಮಾರ್ಗ_2, Style style) {
  SmallString<128> ಮಾರ್ಗ_storage;
  StringRef p = ಮಾರ್ಗ_2.ಗಮ್ಯ_String_Ref(ಮಾರ್ಗ_storage);

  ಹಿಂತಿರುಗಿ !ಪೋಷಕ_ಮಾರ್ಗ(p, style).ಖಾಲಿಗಳು();
}

ತಾರ್ಕಿಕ has_stem(ಸ್ಥಿರಾಂಕ Twine &ಮಾರ್ಗ_2, Style style) {
  SmallString<128> ಮಾರ್ಗ_storage;
  StringRef p = ಮಾರ್ಗ_2.ಗಮ್ಯ_String_Ref(ಮಾರ್ಗ_storage);

  ಹಿಂತಿರುಗಿ !stem(p, style).ಖಾಲಿಗಳು();
}

ತಾರ್ಕಿಕ has_extension(ಸ್ಥಿರಾಂಕ Twine &ಮಾರ್ಗ_2, Style style) {
  SmallString<128> ಮಾರ್ಗ_storage;
  StringRef p = ಮಾರ್ಗ_2.ಗಮ್ಯ_String_Ref(ಮಾರ್ಗ_storage);

  ಹಿಂತಿರುಗಿ !extension(p, style).ಖಾಲಿಗಳು();
}

ತಾರ್ಕಿಕ is_absolute(ಸ್ಥಿರಾಂಕ Twine &ಮಾರ್ಗ_2, Style style) {
  SmallString<128> ಮಾರ್ಗ_storage;
  StringRef p = ಮಾರ್ಗ_2.ಗಮ್ಯ_String_Ref(ಮಾರ್ಗ_storage);

  ತಾರ್ಕಿಕ ಬೇರು_Dir = has_ಬೇರು_directory(p, style);
  ತಾರ್ಕಿಕ ಬೇರು_Name = is_style_posix(style) || has_ಬೇರು_name(p, style);

  ಹಿಂತಿರುಗಿ ಬೇರು_Dir && ಬೇರು_Name;
}

ತಾರ್ಕಿಕ is_absolute_gnu(ಸ್ಥಿರಾಂಕ Twine &ಮಾರ್ಗ_2, Style style) {
  SmallString<128> ಮಾರ್ಗ_storage;
  StringRef p = ಮಾರ್ಗ_2.ಗಮ್ಯ_String_Ref(ಮಾರ್ಗ_storage);

  // Handle '/' which is absolute for both Windows and POSIX systems.
  // Handle '\\' on Windows.
  ಆದರೆ (!p.ಖಾಲಿಗಳು() && is_separator(p.front(), style))
    ಹಿಂತಿರುಗಿ ನಿಜ;

  ಆದರೆ (is_style_windows(style)) {
    // Handle drive letter pattern (a character followed by ':') on Windows.
    ಆದರೆ (p.ಗಾತ್ರ_3() >= 2 && (p[0] && p[1] == ':'))
      ಹಿಂತಿರುಗಿ ನಿಜ;
  }

  ಹಿಂತಿರುಗಿ ಸುಳ್ಳು;
}

ತಾರ್ಕಿಕ is_relative(ಸ್ಥಿರಾಂಕ Twine &ಮಾರ್ಗ_2, Style style) {
  ಹಿಂತಿರುಗಿ !is_absolute(ಮಾರ್ಗ_2, style);
}

ಶೂನ್ಯ make_absolute(ಸ್ಥಿರಾಂಕ Twine &current_directory,
                   Small_ಸದಿಶ_Impl<ಅಕ್ಷರ> &ಮಾರ್ಗ_2) {
  StringRef p(ಮಾರ್ಗ_2.ದತ್ತಾಂಶ(), ಮಾರ್ಗ_2.ಗಾತ್ರ_3());

  ತಾರ್ಕಿಕ ಬೇರು_Directory = has_ಬೇರು_directory(p);
  ತಾರ್ಕಿಕ ಬೇರು_Name = has_ಬೇರು_name(p);

  // Already absolute.
  ಆದರೆ ((ಬೇರು_Name || is_style_posix(Style::native)) && ಬೇರು_Directory)
    ಹಿಂತಿರುಗಿ;

  // All the following conditions will need the current directory.
  SmallString<128> current_dir;
  current_directory.ಗಮ್ಯ_ಸದಿಶ(current_dir);

  // Relative path. Prepend the current directory.
  ಆದರೆ (!ಬೇರು_Name && !ಬೇರು_Directory) {
    // Append path to the current directory.
    append(current_dir, p);
    // Set path to the result.
    ಮಾರ್ಗ_2.ಅದಲುಬದಲು(current_dir);
    ಹಿಂತಿರುಗಿ;
  }

  ಆದರೆ (!ಬೇರು_Name && ಬೇರು_Directory) {
    StringRef cdrn = ಬೇರು_name(current_dir);
    SmallString<128> cur_Dir_ಬೇರು_Name(cdrn.begin(), cdrn.ಕೊನೆ());
    append(cur_Dir_ಬೇರು_Name, p);
    // Set path to the result.
    ಮಾರ್ಗ_2.ಅದಲುಬದಲು(cur_Dir_ಬೇರು_Name);
    ಹಿಂತಿರುಗಿ;
  }

  ಆದರೆ (ಬೇರು_Name && !ಬೇರು_Directory) {
    StringRef p_ಬೇರು_Name = ಬೇರು_name(p);
    StringRef b_ಬೇರು_Directory = ಬೇರು_directory(current_dir);
    StringRef b_Relative_ಮಾರ್ಗ = relative_ಮಾರ್ಗ(current_dir);
    StringRef p_Relative_ಮಾರ್ಗ = relative_ಮಾರ್ಗ(p);

    SmallString<128> res;
    append(res, p_ಬೇರು_Name, b_ಬೇರು_Directory, b_Relative_ಮಾರ್ಗ, p_Relative_ಮಾರ್ಗ);
    ಮಾರ್ಗ_2.ಅದಲುಬದಲು(res);
    ಹಿಂತಿರುಗಿ;
  }

  llvm_unreachable("All rootName and rootDirectory combinations should have "
                   "occurred above!");
}

StringRef remove_leading_dotslash(StringRef ಮಾರ್ಗ, Style style) {
  // Remove leading "./" (or ".//" or "././" etc.)
  ಆಗ (ಮಾರ್ಗ.ಗಾತ್ರ_3() > 2 && ಮಾರ್ಗ[0] == '.' && is_separator(ಮಾರ್ಗ[1], style)) {
    ಮಾರ್ಗ = ಮಾರ್ಗ.substr(2);
    ಆಗ (ಮಾರ್ಗ.ಗಾತ್ರ_3() > 0 && is_separator(ಮಾರ್ಗ[0], style))
      ಮಾರ್ಗ = ಮಾರ್ಗ.substr(1);
  }
  ಹಿಂತಿರುಗಿ ಮಾರ್ಗ;
}

ತಾರ್ಕಿಕ remove_dots(Small_ಸದಿಶ_Impl<ಅಕ್ಷರ> &the_ಮಾರ್ಗ, ತಾರ್ಕಿಕ remove_dot_dot,
                 Style style) {
  style = real_style(style);
  StringRef ಉಳಿದ_2(the_ಮಾರ್ಗ.ದತ್ತಾಂಶ(), the_ಮಾರ್ಗ.ಗಾತ್ರ_3());
  ತಾರ್ಕಿಕ needs_change = ಸುಳ್ಳು;
  Small_ಸದಿಶ<StringRef, 16> components;

  // Consume the root path, if present.
  StringRef ಬೇರು_3 = ಮಾರ್ಗ_2::ಬೇರು_ಮಾರ್ಗ_2(ಉಳಿದ_2, style);
  ತಾರ್ಕಿಕ absolute = !ಬೇರು_3.ಖಾಲಿಗಳು();
  ಆದರೆ (absolute)
    ಉಳಿದ_2 = ಉಳಿದ_2.drop_front(ಬೇರು_3.ಗಾತ್ರ_3());

  // Loop over path components manually. This makes it easier to detect
  // non-preferred slashes and double separators that must be canonicalized.
  ಆಗ (!ಉಳಿದ_2.ಖಾಲಿಗಳು()) {
    size_t ಮುಂದಿನ_slash = ಉಳಿದ_2.ಕಂಡುಹಿಡಿ_ಮೊದಲ_ದ(separators(style));
    ಆದರೆ (ಮುಂದಿನ_slash == StringRef::npos)
      ಮುಂದಿನ_slash = ಉಳಿದ_2.ಗಾತ್ರ_3();
    StringRef component = ಉಳಿದ_2.take_front(ಮುಂದಿನ_slash);
    ಉಳಿದ_2 = ಉಳಿದ_2.drop_front(ಮುಂದಿನ_slash);

    // Eat the slash, and check if it is the preferred separator.
    ಆದರೆ (!ಉಳಿದ_2.ಖಾಲಿಗಳು()) {
      needs_change |= ಉಳಿದ_2.front() != preferred_separator(style);
      ಉಳಿದ_2 = ಉಳಿದ_2.drop_front();
      // The path needs to be rewritten if it has a trailing slash.
      // FIXME: This is emergent behavior that could be removed.
      needs_change |= ಉಳಿದ_2.ಖಾಲಿಗಳು();
    }

    // Check for path traversal components or double separators.
    ಆದರೆ (component.ಖಾಲಿಗಳು() || component == ".") {
      needs_change = ನಿಜ;
    } ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (remove_dot_dot && component == "..") {
      needs_change = ನಿಜ;
      // Do not allow ".." to remove the root component. If this is the
      // beginning of a relative path, keep the ".." component.
      ಆದರೆ (!components.ಖಾಲಿಗಳು() && components.ಹಿಂಭಾಗ() != "..") {
        components.ತೆಗೆ_ಹಿಂಭಾಗ();
      } ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (!absolute) {
        components.ತಳ್ಳು_ಹಿಂಭಾಗ(component);
      }
    } ಇಲ್ಲದಿದ್ದರೆ {
      components.ತಳ್ಳು_ಹಿಂಭಾಗ(component);
    }
  }

  SmallString<256> ಬಫರ್_2 = ಬೇರು_3;
  // "root" could be "/", which may need to be translated into "\".
  make_preferred(ಬಫರ್_2, style);
  needs_change |= ಬೇರು_3 != ಬಫರ್_2;

  // Avoid rewriting the path unless we have to.
  ಆದರೆ (!needs_change)
    ಹಿಂತಿರುಗಿ ಸುಳ್ಳು;

  ಆದರೆ (!components.ಖಾಲಿಗಳು()) {
    ಬಫರ್_2 += components[0];
    ಗಾಗಿ (StringRef C : ArrayRef(components).drop_front()) {
      ಬಫರ್_2 += preferred_separator(style);
      ಬಫರ್_2 += C;
    }
  }
  the_ಮಾರ್ಗ.ಅದಲುಬದಲು(ಬಫರ್_2);
  ಹಿಂತಿರುಗಿ ನಿಜ;
}

} // end namespace path

ನಾಮಾಕಾಶ fs {

std::error_ಸಂಕೇತ getUniqueID(ಸ್ಥಿರಾಂಕ Twine ಮಾರ್ಗ, UniqueID &ಫಲಿತಾಂಶ) {
  sandbox::violationIfEnabled();

  file_ಸ್ಥಿತಿ ಸ್ಥಿತಿ_2;
  std::error_ಸಂಕೇತ EC = ಸ್ಥಿತಿ_3(ಮಾರ್ಗ, ಸ್ಥಿತಿ_2);
  ಆದರೆ (EC)
    ಹಿಂತಿರುಗಿ EC;
  ಫಲಿತಾಂಶ = ಸ್ಥಿತಿ_2.getUniqueID();
  ಹಿಂತಿರುಗಿ std::error_ಸಂಕೇತ();
}

ಶೂನ್ಯ create_Unique_ಮಾರ್ಗ(ಸ್ಥಿರಾಂಕ Twine &Model, Small_ಸದಿಶ_Impl<ಅಕ್ಷರ> &ಫಲಿತಾಂಶ_ಮಾರ್ಗ,
                      ತಾರ್ಕಿಕ MakeAbsolute) {
  SmallString<128> ModelStorage;
  Model.ಗಮ್ಯ_ಸದಿಶ(ModelStorage);

  ಪ್ರತಿಜ್ಞಾ(llvm::is_contained(ModelStorage, '%') &&
         "createUniquePath: Model must contain at least one '%'");

  ಆದರೆ (MakeAbsolute) {
    // Make model absolute by prepending a temp directory if it's not already.
    ಆದರೆ (!sys::ಮಾರ್ಗ_2::is_absolute(Twine(ModelStorage))) {
      SmallString<128> TDir;
      sys::ಮಾರ್ಗ_2::system_ತಾತ್ಕಾಲಿಕ_directory(ನಿಜ, TDir);
      sys::ಮಾರ್ಗ_2::append(TDir, Twine(ModelStorage));
      ModelStorage.ಅದಲುಬದಲು(TDir);
    }
  }

  ಫಲಿತಾಂಶ_ಮಾರ್ಗ = ModelStorage;
  ಫಲಿತಾಂಶ_ಮಾರ್ಗ.ತಳ್ಳು_ಹಿಂಭಾಗ(0);
  ಫಲಿತಾಂಶ_ಮಾರ್ಗ.ತೆಗೆ_ಹಿಂಭಾಗ();

  // Replace '%' with random chars.
  ಗಾಗಿ (ಚಿಹ್ನೆರಹಿತ i = 0, e = ModelStorage.ಗಾತ್ರ_3(); i != e; ++i) {
    ಆದರೆ (ModelStorage[i] == '%')
      ಫಲಿತಾಂಶ_ಮಾರ್ಗ[i] = "0123456789abcdef"[sys::Process::Get_Random_ಸಂಖ್ಯೆ() & 15];
  }
}

std::error_ಸಂಕೇತ createUniqueFile(ಸ್ಥಿರಾಂಕ Twine &Model, ಪೂರ್ಣಾಂಕ &ಫಲಿತಾಂಶ_Fd,
                                 Small_ಸದಿಶ_Impl<ಅಕ್ಷರ> &ಫಲಿತಾಂಶ_ಮಾರ್ಗ,
                                 OpenFlags Flags, ಚಿಹ್ನೆರಹಿತ ಬಹುಲಕ) {
  ಹಿಂತಿರುಗಿ createUniqueEntity(Model, ಫಲಿತಾಂಶ_Fd, ಫಲಿತಾಂಶ_ಮಾರ್ಗ, ಸುಳ್ಳು, FS_File, Flags,
                            ಬಹುಲಕ);
}

std::error_ಸಂಕೇತ createUniqueFile(ಸ್ಥಿರಾಂಕ Twine &Model,
                                 Small_ಸದಿಶ_Impl<ಅಕ್ಷರ> &ಫಲಿತಾಂಶ_ಮಾರ್ಗ,
                                 ಚಿಹ್ನೆರಹಿತ ಬಹುಲಕ) {
  ಪೂರ್ಣಾಂಕ FD;
  ಸ್ವಯಂಚಾಲಿತ EC = createUniqueFile(Model, FD, ಫಲಿತಾಂಶ_ಮಾರ್ಗ, ದ_None, ಬಹುಲಕ);
  ಆದರೆ (EC)
    ಹಿಂತಿರುಗಿ EC;
  // FD is only needed to avoid race conditions. Close it right away.
  close(FD);
  ಹಿಂತಿರುಗಿ EC;
}

ಸ್ಥಿರ std::error_ಸಂಕೇತ
createTemporaryFile(ಸ್ಥಿರಾಂಕ Twine &Model, ಪೂರ್ಣಾಂಕ &ಫಲಿತಾಂಶ_FD,
                    llvm::Small_ಸದಿಶ_Impl<ಅಕ್ಷರ> &ಫಲಿತಾಂಶ_ಮಾರ್ಗ, FSEntity Type,
                    sys::fs::OpenFlags Flags = sys::fs::ದ_None) {
  // Any *temporary* file is assumed to be a compiler-internal output, not
  // a formal one.
  ಸ್ವಯಂಚಾಲಿತ BypassSandbox = sys::sandbox::scopedDisable();

  SmallString<128> Storage;
  StringRef P = Model.ಗಮ್ಯ_Null_Terminated_String_Ref(Storage);
  ಪ್ರತಿಜ್ಞಾ(P.ಕಂಡುಹಿಡಿ_ಮೊದಲ_ದ(separators(Style::native)) == StringRef::npos &&
         "Model must be a simple filename.");
  // Use P.begin() so that createUniqueEntity doesn't need to recreate Storage.
  ಹಿಂತಿರುಗಿ createUniqueEntity(P.begin(), ಫಲಿತಾಂಶ_FD, ಫಲಿತಾಂಶ_ಮಾರ್ಗ, ನಿಜ, Type, Flags,
                            all_read | all_write);
}

ಸ್ಥಿರ std::error_ಸಂಕೇತ
createTemporaryFile(ಸ್ಥಿರಾಂಕ Twine &ಪೂರ್ವಪ್ರತ್ಯಯ, StringRef Suffix, ಪೂರ್ಣಾಂಕ &ಫಲಿತಾಂಶ_FD,
                    llvm::Small_ಸದಿಶ_Impl<ಅಕ್ಷರ> &ಫಲಿತಾಂಶ_ಮಾರ್ಗ, FSEntity Type,
                    sys::fs::OpenFlags Flags = sys::fs::ದ_None) {
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಮಧ್ಯ = Suffix.ಖಾಲಿಗಳು() ? "-%%%%%%" : "-%%%%%%.";
  ಹಿಂತಿರುಗಿ createTemporaryFile(ಪೂರ್ವಪ್ರತ್ಯಯ + ಮಧ್ಯ + Suffix, ಫಲಿತಾಂಶ_FD, ಫಲಿತಾಂಶ_ಮಾರ್ಗ,
                             Type, Flags);
}

std::error_ಸಂಕೇತ createTemporaryFile(ಸ್ಥಿರಾಂಕ Twine &ಪೂರ್ವಪ್ರತ್ಯಯ, StringRef Suffix,
                                    ಪೂರ್ಣಾಂಕ &ಫಲಿತಾಂಶ_FD,
                                    Small_ಸದಿಶ_Impl<ಅಕ್ಷರ> &ಫಲಿತಾಂಶ_ಮಾರ್ಗ,
                                    sys::fs::OpenFlags Flags) {
  ಹಿಂತಿರುಗಿ createTemporaryFile(ಪೂರ್ವಪ್ರತ್ಯಯ, Suffix, ಫಲಿತಾಂಶ_FD, ಫಲಿತಾಂಶ_ಮಾರ್ಗ, FS_File,
                             Flags);
}

std::error_ಸಂಕೇತ createTemporaryFile(ಸ್ಥಿರಾಂಕ Twine &ಪೂರ್ವಪ್ರತ್ಯಯ, StringRef Suffix,
                                    Small_ಸದಿಶ_Impl<ಅಕ್ಷರ> &ಫಲಿತಾಂಶ_ಮಾರ್ಗ,
                                    sys::fs::OpenFlags Flags) {
  ಪೂರ್ಣಾಂಕ FD;
  ಸ್ವಯಂಚಾಲಿತ EC = createTemporaryFile(ಪೂರ್ವಪ್ರತ್ಯಯ, Suffix, FD, ಫಲಿತಾಂಶ_ಮಾರ್ಗ, Flags);
  ಆದರೆ (EC)
    ಹಿಂತಿರುಗಿ EC;
  // FD is only needed to avoid race conditions. Close it right away.
  close(FD);
  ಹಿಂತಿರುಗಿ EC;
}

// This is a mkdtemp with a different pattern. We use createUniqueEntity mostly
// for consistency. We should try using mkdtemp.
std::error_ಸಂಕೇತ createUniqueDirectory(ಸ್ಥಿರಾಂಕ Twine &ಪೂರ್ವಪ್ರತ್ಯಯ,
                                      Small_ಸದಿಶ_Impl<ಅಕ್ಷರ> &ಫಲಿತಾಂಶ_ಮಾರ್ಗ) {
  ಪೂರ್ಣಾಂಕ Dummy;
  ಹಿಂತಿರುಗಿ createUniqueEntity(ಪೂರ್ವಪ್ರತ್ಯಯ + "-%%%%%%", Dummy, ಫಲಿತಾಂಶ_ಮಾರ್ಗ, ನಿಜ,
                            FS_Dir);
}

std::error_ಸಂಕೇತ
getPotentiallyUniqueFileName(ಸ್ಥಿರಾಂಕ Twine &Model,
                             Small_ಸದಿಶ_Impl<ಅಕ್ಷರ> &ಫಲಿತಾಂಶ_ಮಾರ್ಗ) {
  ಪೂರ್ಣಾಂಕ Dummy;
  ಹಿಂತಿರುಗಿ createUniqueEntity(Model, Dummy, ಫಲಿತಾಂಶ_ಮಾರ್ಗ, ಸುಳ್ಳು, FS_Name);
}

std::error_ಸಂಕೇತ
get_Potentially_Unique_ತಾತ್ಕಾಲಿಕ_File_Name(ಸ್ಥಿರಾಂಕ Twine &ಪೂರ್ವಪ್ರತ್ಯಯ, StringRef Suffix,
                                 Small_ಸದಿಶ_Impl<ಅಕ್ಷರ> &ಫಲಿತಾಂಶ_ಮಾರ್ಗ) {
  ಪೂರ್ಣಾಂಕ Dummy;
  ಹಿಂತಿರುಗಿ createTemporaryFile(ಪೂರ್ವಪ್ರತ್ಯಯ, Suffix, Dummy, ಫಲಿತಾಂಶ_ಮಾರ್ಗ, FS_Name);
}

std::error_ಸಂಕೇತ make_absolute(Small_ಸದಿಶ_Impl<ಅಕ್ಷರ> &ಮಾರ್ಗ_2) {
  sandbox::violationIfEnabled();

  ಆದರೆ (ಮಾರ್ಗ_2::is_absolute(ಮಾರ್ಗ_2))
    ಹಿಂತಿರುಗಿ {};

  SmallString<128> current_dir;
  ಆದರೆ (std::error_ಸಂಕೇತ ec = current_ಮಾರ್ಗ(current_dir))
    ಹಿಂತಿರುಗಿ ec;

  ಮಾರ್ಗ_2::make_absolute(current_dir, ಮಾರ್ಗ_2);
  ಹಿಂತಿರುಗಿ {};
}

std::error_ಸಂಕೇತ create_directories(ಸ್ಥಿರಾಂಕ Twine &ಮಾರ್ಗ, ತಾರ್ಕಿಕ IgnoreExisting,
                                   perms Perms) {
  SmallString<128> ಮಾರ್ಗ_Storage;
  StringRef P = ಮಾರ್ಗ.ಗಮ್ಯ_String_Ref(ಮಾರ್ಗ_Storage);

  // Be optimistic and try to create the directory
  std::error_ಸಂಕೇತ EC = create_directory(P, IgnoreExisting, Perms);
  // If we succeeded, or had any error other than the parent not existing, just
  // return it.
  ಆದರೆ (EC != errc::no_such_file_or_directory)
    ಹಿಂತಿರುಗಿ EC;

  // We failed because of a no_such_file_or_directory, try to create the
  // parent.
  StringRef ಪೋಷಕ_2 = ಮಾರ್ಗ_2::ಪೋಷಕ_ಮಾರ್ಗ(P);
  ಆದರೆ (ಪೋಷಕ_2.ಖಾಲಿಗಳು())
    ಹಿಂತಿರುಗಿ EC;

  ಆದರೆ ((EC = create_directories(ಪೋಷಕ_2, IgnoreExisting, Perms)))
      ಹಿಂತಿರುಗಿ EC;

  ಹಿಂತಿರುಗಿ create_directory(P, IgnoreExisting, Perms);
}

ಸ್ಥಿರ std::error_ಸಂಕೇತ copy_file_ಆಂತರಿಕ(ಪೂರ್ಣಾಂಕ ReadFD, ಪೂರ್ಣಾಂಕ WriteFD) {
  ಸ್ಥಿರಾಂಕ size_t Buf_ಗಾತ್ರ = 4096;
  ಅಕ್ಷರ *Buf = ಹೊಸ ಅಕ್ಷರ[Buf_ಗಾತ್ರ];
  ಪೂರ್ಣಾಂಕ ಬೈಟುಗಳು_Read = 0, ಬೈಟುಗಳು_Written = 0;
  ಗಾಗಿ (;;) {
    ಬೈಟುಗಳು_Read = ಪಠ(ReadFD, Buf, Buf_ಗಾತ್ರ);
    ಆದರೆ (ಬೈಟುಗಳು_Read <= 0)
      ನಿಲ್ಲಿಸು;
    ಆಗ (ಬೈಟುಗಳು_Read) {
      ಬೈಟುಗಳು_Written = ಲಿಖ(WriteFD, Buf, ಬೈಟುಗಳು_Read);
      ಆದರೆ (ಬೈಟುಗಳು_Written < 0)
        ನಿಲ್ಲಿಸು;
      ಬೈಟುಗಳು_Read -= ಬೈಟುಗಳು_Written;
    }
    ಆದರೆ (ಬೈಟುಗಳು_Written < 0)
      ನಿಲ್ಲಿಸು;
  }
  ಅಳಿಸು[] Buf;

  ಆದರೆ (ಬೈಟುಗಳು_Read < 0 || ಬೈಟುಗಳು_Written < 0)
    ಹಿಂತಿರುಗಿ errno_As_Error_ಸಂಕೇತ();
  ಹಿಂತಿರುಗಿ std::error_ಸಂಕೇತ();
}

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ __APPLE__
std::error_ಸಂಕೇತ copy_file(ಸ್ಥಿರಾಂಕ Twine &From, ಸ್ಥಿರಾಂಕ Twine &ಗಮ್ಯ) {
  ಪೂರ್ಣಾಂಕ ReadFD, WriteFD;
  ಆದರೆ (std::error_ಸಂಕೇತ EC = openFileForRead(From, ReadFD, ದ_None))
    ಹಿಂತಿರುಗಿ EC;
  ಆದರೆ (std::error_ಸಂಕೇತ EC =
          openFileForWrite(ಗಮ್ಯ, WriteFD, CD_CreateAlways, ದ_None)) {
    close(ReadFD);
    ಹಿಂತಿರುಗಿ EC;
  }

  std::error_ಸಂಕೇತ EC = copy_file_ಆಂತರಿಕ(ReadFD, WriteFD);

  close(ReadFD);
  close(WriteFD);

  ಹಿಂತಿರುಗಿ EC;
}
#ಆದರೆ_ಅಂತ್ಯ

std::error_ಸಂಕೇತ copy_file(ಸ್ಥಿರಾಂಕ Twine &From, ಪೂರ್ಣಾಂಕ ಗಮ್ಯ_FD) {
  ಪೂರ್ಣಾಂಕ ReadFD;
  ಆದರೆ (std::error_ಸಂಕೇತ EC = openFileForRead(From, ReadFD, ದ_None))
    ಹಿಂತಿರುಗಿ EC;

  std::error_ಸಂಕೇತ EC = copy_file_ಆಂತರಿಕ(ReadFD, ಗಮ್ಯ_FD);

  close(ReadFD);

  ಹಿಂತಿರುಗಿ EC;
}

ErrorOr<MD5::MD5_ಫಲಿತಾಂಶ> md5_contents(ಪೂರ್ಣಾಂಕ FD) {
  sandbox::violationIfEnabled();

  MD5 Hash;

  constexpr size_t Buf_ಗಾತ್ರ = 4096;
  std::ಸದಿಶ<uint8_t> Buf(Buf_ಗಾತ್ರ);
  ಪೂರ್ಣಾಂಕ ಬೈಟುಗಳು_Read = 0;
  ಗಾಗಿ (;;) {
    ಬೈಟುಗಳು_Read = ಪಠ(FD, Buf.ದತ್ತಾಂಶ(), Buf_ಗಾತ್ರ);
    ಆದರೆ (ಬೈಟುಗಳು_Read <= 0)
      ನಿಲ್ಲಿಸು;
    Hash.update(ArrayRef(Buf.ದತ್ತಾಂಶ(), ಬೈಟುಗಳು_Read));
  }

  ಆದರೆ (ಬೈಟುಗಳು_Read < 0)
    ಹಿಂತಿರುಗಿ errno_As_Error_ಸಂಕೇತ();
  MD5::MD5_ಫಲಿತಾಂಶ ಫಲಿತಾಂಶ;
  Hash.final(ಫಲಿತಾಂಶ);
  ಹಿಂತಿರುಗಿ ಫಲಿತಾಂಶ;
}

ErrorOr<MD5::MD5_ಫಲಿತಾಂಶ> md5_contents(ಸ್ಥಿರಾಂಕ Twine &ಮಾರ್ಗ) {
  sandbox::violationIfEnabled();

  ಪೂರ್ಣಾಂಕ FD;
  ಆದರೆ (ಸ್ವಯಂಚಾಲಿತ EC = openFileForRead(ಮಾರ್ಗ, FD, ದ_None))
    ಹಿಂತಿರುಗಿ EC;

  ಸ್ವಯಂಚಾಲಿತ ಫಲಿತಾಂಶ = md5_contents(FD);
  close(FD);
  ಹಿಂತಿರುಗಿ ಫಲಿತಾಂಶ;
}

ತಾರ್ಕಿಕ exists(ಸ್ಥಿರಾಂಕ basic_file_ಸ್ಥಿತಿ &ಸ್ಥಿತಿ_3) {
  ಹಿಂತಿರುಗಿ ಸ್ಥಿತಿ_ತಿಳಿದ(ಸ್ಥಿತಿ_3) && ಸ್ಥಿತಿ_3.type() != file_type::file_not_ಸಿಕ್ಕಿತು;
}

ತಾರ್ಕಿಕ ಸ್ಥಿತಿ_ತಿಳಿದ(ಸ್ಥಿರಾಂಕ basic_file_ಸ್ಥಿತಿ &s) {
  ಹಿಂತಿರುಗಿ s.type() != file_type::ಸ್ಥಿತಿ_error;
}

file_type get_file_type(ಸ್ಥಿರಾಂಕ Twine &ಮಾರ್ಗ, ತಾರ್ಕಿಕ Follow) {
  file_ಸ್ಥಿತಿ st;
  ಆದರೆ (ಸ್ಥಿತಿ_3(ಮಾರ್ಗ, st, Follow))
    ಹಿಂತಿರುಗಿ file_type::ಸ್ಥಿತಿ_error;
  ಹಿಂತಿರುಗಿ st.type();
}

ತಾರ್ಕಿಕ is_directory(ಸ್ಥಿರಾಂಕ basic_file_ಸ್ಥಿತಿ &ಸ್ಥಿತಿ_3) {
  ಹಿಂತಿರುಗಿ ಸ್ಥಿತಿ_3.type() == file_type::directory_file;
}

std::error_ಸಂಕೇತ is_directory(ಸ್ಥಿರಾಂಕ Twine &ಮಾರ್ಗ_2, ತಾರ್ಕಿಕ &ಫಲಿತಾಂಶ_2) {
  sandbox::violationIfEnabled();

  file_ಸ್ಥಿತಿ st;
  ಆದರೆ (std::error_ಸಂಕೇತ ec = ಸ್ಥಿತಿ_3(ಮಾರ್ಗ_2, st))
    ಹಿಂತಿರುಗಿ ec;
  ಫಲಿತಾಂಶ_2 = is_directory(st);
  ಹಿಂತಿರುಗಿ std::error_ಸಂಕೇತ();
}

ತಾರ್ಕಿಕ is_regular_file(ಸ್ಥಿರಾಂಕ basic_file_ಸ್ಥಿತಿ &ಸ್ಥಿತಿ_3) {
  ಹಿಂತಿರುಗಿ ಸ್ಥಿತಿ_3.type() == file_type::regular_file;
}

std::error_ಸಂಕೇತ is_regular_file(ಸ್ಥಿರಾಂಕ Twine &ಮಾರ್ಗ_2, ತಾರ್ಕಿಕ &ಫಲಿತಾಂಶ_2) {
  sandbox::violationIfEnabled();

  file_ಸ್ಥಿತಿ st;
  ಆದರೆ (std::error_ಸಂಕೇತ ec = ಸ್ಥಿತಿ_3(ಮಾರ್ಗ_2, st))
    ಹಿಂತಿರುಗಿ ec;
  ಫಲಿತಾಂಶ_2 = is_regular_file(st);
  ಹಿಂತಿರುಗಿ std::error_ಸಂಕೇತ();
}

ತಾರ್ಕಿಕ is_symlink_file(ಸ್ಥಿರಾಂಕ basic_file_ಸ್ಥಿತಿ &ಸ್ಥಿತಿ_3) {
  ಹಿಂತಿರುಗಿ ಸ್ಥಿತಿ_3.type() == file_type::symlink_file;
}

std::error_ಸಂಕೇತ is_symlink_file(ಸ್ಥಿರಾಂಕ Twine &ಮಾರ್ಗ_2, ತಾರ್ಕಿಕ &ಫಲಿತಾಂಶ_2) {
  sandbox::violationIfEnabled();

  file_ಸ್ಥಿತಿ st;
  ಆದರೆ (std::error_ಸಂಕೇತ ec = ಸ್ಥಿತಿ_3(ಮಾರ್ಗ_2, st, ಸುಳ್ಳು))
    ಹಿಂತಿರುಗಿ ec;
  ಫಲಿತಾಂಶ_2 = is_symlink_file(st);
  ಹಿಂತಿರುಗಿ std::error_ಸಂಕೇತ();
}

ತಾರ್ಕಿಕ is_other(ಸ್ಥಿರಾಂಕ basic_file_ಸ್ಥಿತಿ &ಸ್ಥಿತಿ_3) {
  ಹಿಂತಿರುಗಿ exists(ಸ್ಥಿತಿ_3) &&
         !is_regular_file(ಸ್ಥಿತಿ_3) &&
         !is_directory(ಸ್ಥಿತಿ_3);
}

std::error_ಸಂಕೇತ is_other(ಸ್ಥಿರಾಂಕ Twine &ಮಾರ್ಗ, ತಾರ್ಕಿಕ &ಫಲಿತಾಂಶ) {
  sandbox::violationIfEnabled();

  file_ಸ್ಥಿತಿ File_ಸ್ಥಿತಿ;
  ಆದರೆ (std::error_ಸಂಕೇತ EC = ಸ್ಥಿತಿ_3(ಮಾರ್ಗ, File_ಸ್ಥಿತಿ))
    ಹಿಂತಿರುಗಿ EC;
  ಫಲಿತಾಂಶ = is_other(File_ಸ್ಥಿತಿ);
  ಹಿಂತಿರುಗಿ std::error_ಸಂಕೇತ();
}

ಶೂನ್ಯ directory_ನಮೂದು::replace_filename(ಸ್ಥಿರಾಂಕ Twine &Filename, file_type Type,
                                       basic_file_ಸ್ಥಿತಿ ಸ್ಥಿತಿ_2) {
  SmallString<128> ಮಾರ್ಗ_Str = ಮಾರ್ಗ_2::ಪೋಷಕ_ಮಾರ್ಗ(ಮಾರ್ಗ);
  ಮಾರ್ಗ_2::append(ಮಾರ್ಗ_Str, Filename);
  ಇದು->ಮಾರ್ಗ = std::string(ಮಾರ್ಗ_Str);
  ಇದು->Type = Type;
  ಇದು->ಸ್ಥಿತಿ_2 = ಸ್ಥಿತಿ_2;
}

ErrorOr<perms> getPermissions(ಸ್ಥಿರಾಂಕ Twine &ಮಾರ್ಗ) {
  sandbox::violationIfEnabled();

  file_ಸ್ಥಿತಿ ಸ್ಥಿತಿ_2;
  ಆದರೆ (std::error_ಸಂಕೇತ EC = ಸ್ಥಿತಿ_3(ಮಾರ್ಗ, ಸ್ಥಿತಿ_2))
    ಹಿಂತಿರುಗಿ EC;

  ಹಿಂತಿರುಗಿ ಸ್ಥಿತಿ_2.permissions();
}

std::error_ಸಂಕೇತ ಗಣ_ಕೊನೆಯ_Access_And_Modification_ಸಮಯ(ಸ್ಥಿರಾಂಕ Twine &ಮಾರ್ಗ,
                                                 ಸಮಯ_ಬಿಂದು<> Access_ಸಮಯ,
                                                 ಸಮಯ_ಬಿಂದು<> Modification_ಸಮಯ) {
  ಪೂರ್ಣಾಂಕ FD;
  ಆದರೆ (std::error_ಸಂಕೇತ EC = openFile(ಮಾರ್ಗ, FD, CD_OpenExisting, FA_Read,
                                    ದ_Update_ಗುಣಲಕ್ಷಣಗಳು | ದ_Open_Directory))
    ಹಿಂತಿರುಗಿ EC;
  std::error_ಸಂಕೇತ EC =
      ಗಣ_ಕೊನೆಯ_Access_And_Modification_ಸಮಯ(FD, Access_ಸಮಯ, Modification_ಸಮಯ);
  Process::SafelyCloseFileDescriptor(FD);
  ಹಿಂತಿರುಗಿ EC;
}

size_t mapped_file_region::ಗಾತ್ರ_3() ಸ್ಥಿರಾಂಕ {
  ಪ್ರತಿಜ್ಞಾ(Mapping && "Mapping failed but used anyway!");
  ಹಿಂತಿರುಗಿ ಗಾತ್ರ_2;
}

ಅಕ್ಷರ *mapped_file_region::ದತ್ತಾಂಶ() ಸ್ಥಿರಾಂಕ {
  ಪ್ರತಿಜ್ಞಾ(Mapping && "Mapping failed but used anyway!");
  ಹಿಂತಿರುಗಿ reinterpret_cast<ಅಕ್ಷರ *>(Mapping);
}

ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *mapped_file_region::const_ದತ್ತಾಂಶ() ಸ್ಥಿರಾಂಕ {
  ಪ್ರತಿಜ್ಞಾ(Mapping && "Mapping failed but used anyway!");
  ಹಿಂತಿರುಗಿ reinterpret_cast<ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *>(Mapping);
}

Error read_Native_File_ಗಮ್ಯ_EOF(file_t FileHandle, Small_ಸದಿಶ_Impl<ಅಕ್ಷರ> &ಬಫರ್,
                          ssize_t Chunk_ಗಾತ್ರ) {
  sandbox::violationIfEnabled();

  // Install a handler to truncate the buffer to the correct size on exit.
  size_t ಗಾತ್ರ_2 = ಬಫರ್.ಗಾತ್ರ_3();
  llvm::scope_exit TruncateOnExit([&]() { ಬಫರ್.truncate(ಗಾತ್ರ_2); });

  // Read into Buffer until we hit EOF.
  ಗಾಗಿ (;;) {
    ಬಫರ್.resize_for_overwrite(ಗಾತ್ರ_2 + Chunk_ಗಾತ್ರ);
    Expected<size_t> Read_ಬೈಟುಗಳು = readNativeFile(
        FileHandle, MutableArrayRef(ಬಫರ್.begin() + ಗಾತ್ರ_2, Chunk_ಗಾತ್ರ));
    ಆದರೆ (!Read_ಬೈಟುಗಳು)
      ಹಿಂತಿರುಗಿ Read_ಬೈಟುಗಳು.takeError();
    ಆದರೆ (*Read_ಬೈಟುಗಳು == 0)
      ಹಿಂತಿರುಗಿ Error::success();
    ಗಾತ್ರ_2 += *Read_ಬೈಟುಗಳು;
  }
}

} // end namespace fs
} // end namespace sys
} // end namespace llvm

// Include the truly platform-specific parts.
#ಆದರೆ defined(LLVM_ON_UNIX)
#ಅನ್ತರ್ಭಾವ "Unix/Path.inc"
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ defined(_WIN32)
#ಅನ್ತರ್ಭಾವ "Windows/Path.inc"
#ಆದರೆ_ಅಂತ್ಯ

ನಾಮಾಕಾಶ llvm {
ನಾಮಾಕಾಶ sys {
ನಾಮಾಕಾಶ fs {

ತಾತ್ಕಾಲಿಕ_File::ತಾತ್ಕಾಲಿಕ_File(StringRef Name, ಪೂರ್ಣಾಂಕ FD)
    : TmpName(std::string(Name)), FD(FD) {}
ತಾತ್ಕಾಲಿಕ_File::ತಾತ್ಕಾಲಿಕ_File(ತಾತ್ಕಾಲಿಕ_File &&Other) { *ಇದು = std::move(Other); }
ತಾತ್ಕಾಲಿಕ_File &ತಾತ್ಕಾಲಿಕ_File::ಕ್ರಿಯಾಕಾರಕ=(ತಾತ್ಕಾಲಿಕ_File &&Other) {
  TmpName = std::move(Other.TmpName);
  FD = Other.FD;
  Other.ಮುಗಿದ = ನಿಜ;
  Other.FD = -1;
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ _WIN32
  RemoveOnClose = Other.RemoveOnClose;
  Other.RemoveOnClose = ಸುಳ್ಳು;
#ಆದರೆ_ಅಂತ್ಯ
  ಹಿಂತಿರುಗಿ *ಇದು;
}

ತಾತ್ಕಾಲಿಕ_File::~ತಾತ್ಕಾಲಿಕ_File() { ಪ್ರತಿಜ್ಞಾ(ಮುಗಿದ); }

Error ತಾತ್ಕಾಲಿಕ_File::discard() {
  ಮುಗಿದ = ನಿಜ;
  ಆದರೆ (FD != -1 && close(FD) == -1) {
    std::error_ಸಂಕೇತ EC = errno_As_Error_ಸಂಕೇತ();
    ಹಿಂತಿರುಗಿ error_ಸಂಕೇತ_ಗಮ್ಯ_Error(EC);
  }
  FD = -1;

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ _WIN32
  // On Windows, closing will remove the file, if we set the delete
  // disposition. If not, remove it manually.
  ತಾರ್ಕಿಕ Remove = RemoveOnClose;
#ಇಲ್ಲದಿದ್ದರೆ
  // Always try to remove the file.
  ತಾರ್ಕಿಕ Remove = ನಿಜ;
#ಆದರೆ_ಅಂತ್ಯ
  std::error_ಸಂಕೇತ RemoveEC;
  ಆದರೆ (Remove && !TmpName.ಖಾಲಿಗಳು()) {
    RemoveEC = fs::remove(TmpName);
    sys::DontRemoveFileOnSignal(TmpName);
    ಆದರೆ (!RemoveEC)
      TmpName = "";
  } ಇಲ್ಲದಿದ್ದರೆ {
    TmpName = "";
  }
  ಹಿಂತಿರುಗಿ error_ಸಂಕೇತ_ಗಮ್ಯ_Error(RemoveEC);
}

Error ತಾತ್ಕಾಲಿಕ_File::keep(ಸ್ಥಿರಾಂಕ Twine &Name) {
  ಪ್ರತಿಜ್ಞಾ(!ಮುಗಿದ);
  ಮುಗಿದ = ನಿಜ;
  // Always try to close and rename.
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ _WIN32
  // If we can't cancel the delete don't rename.
  ಸ್ವಯಂಚಾಲಿತ H = reinterpret_cast<HANDLE>(_get_osfhandle(FD));
  std::error_ಸಂಕೇತ RenameEC =
      RemoveOnClose ? std::error_ಸಂಕೇತ() : ಗಣ_Delete_Disposition(H, ಸುಳ್ಳು);
  ತಾರ್ಕಿಕ ShouldDelete = ಸುಳ್ಳು;
  ಆದರೆ (!RenameEC) {
    RenameEC = rename_handle(H, Name);
    // If rename failed because it's cross-device, copy instead
    ಆದರೆ (RenameEC ==
      std::error_ಸಂಕೇತ(ERROR_NOT_SAME_DEVICE, std::system_category())) {
      RenameEC = copy_file(TmpName, Name);
      ShouldDelete = ನಿಜ;
    }
  }

  // If we can't rename or copy, discard the temporary file.
  ಆದರೆ (RenameEC)
    ShouldDelete = ನಿಜ;
  ಆದರೆ (ShouldDelete) {
    ಆದರೆ (!RemoveOnClose)
      ಗಣ_Delete_Disposition(H, ನಿಜ);
    ಇಲ್ಲದಿದ್ದರೆ
      remove(TmpName);
  }
#ಇಲ್ಲದಿದ್ದರೆ
  std::error_ಸಂಕೇತ RenameEC = fs::rename(TmpName, Name);
  ಆದರೆ (RenameEC) {
    // If we can't rename, try to copy to work around cross-device link issues.
    RenameEC = sys::fs::copy_file(TmpName, Name);
    // If we can't rename or copy, discard the temporary file.
    ಆದರೆ (RenameEC)
      remove(TmpName);
  }
#ಆದರೆ_ಅಂತ್ಯ
  sys::DontRemoveFileOnSignal(TmpName);

  ಆದರೆ (!RenameEC)
    TmpName = "";

  ಆದರೆ (close(FD) == -1)
    ಹಿಂತಿರುಗಿ error_ಸಂಕೇತ_ಗಮ್ಯ_Error(errno_As_Error_ಸಂಕೇತ());
  FD = -1;

  ಹಿಂತಿರುಗಿ error_ಸಂಕೇತ_ಗಮ್ಯ_Error(RenameEC);
}

Error ತಾತ್ಕಾಲಿಕ_File::keep() {
  ಪ್ರತಿಜ್ಞಾ(!ಮುಗಿದ);
  ಮುಗಿದ = ನಿಜ;

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ _WIN32
  ಸ್ವಯಂಚಾಲಿತ H = reinterpret_cast<HANDLE>(_get_osfhandle(FD));
  ಆದರೆ (std::error_ಸಂಕೇತ EC = ಗಣ_Delete_Disposition(H, ಸುಳ್ಳು))
    ಹಿಂತಿರುಗಿ error_ಸಂಕೇತ_ಗಮ್ಯ_Error(EC);
#ಆದರೆ_ಅಂತ್ಯ
  sys::DontRemoveFileOnSignal(TmpName);

  TmpName = "";

  ಆದರೆ (close(FD) == -1)
    ಹಿಂತಿರುಗಿ error_ಸಂಕೇತ_ಗಮ್ಯ_Error(errno_As_Error_ಸಂಕೇತ());
  FD = -1;

  ಹಿಂತಿರುಗಿ Error::success();
}

Expected<ತಾತ್ಕಾಲಿಕ_File> ತಾತ್ಕಾಲಿಕ_File::create(ಸ್ಥಿರಾಂಕ Twine &Model, ಚಿಹ್ನೆರಹಿತ ಬಹುಲಕ,
                                    OpenFlags ExtraFlags) {
  ಪೂರ್ಣಾಂಕ FD;
  SmallString<128> ಫಲಿತಾಂಶ_ಮಾರ್ಗ;
  ಆದರೆ (std::error_ಸಂಕೇತ EC =
          createUniqueFile(Model, FD, ಫಲಿತಾಂಶ_ಮಾರ್ಗ, ದ_Delete | ExtraFlags, ಬಹುಲಕ))
    ಹಿಂತಿರುಗಿ error_ಸಂಕೇತ_ಗಮ್ಯ_Error(EC);

  ತಾತ್ಕಾಲಿಕ_File Ret(ಫಲಿತಾಂಶ_ಮಾರ್ಗ, FD);
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ _WIN32
  ಸ್ವಯಂಚಾಲಿತ H = reinterpret_cast<HANDLE>(_get_osfhandle(FD));
  ತಾರ್ಕಿಕ ಗಣ_Signal_Handler = ಸುಳ್ಳು;
  ಆದರೆ (std::error_ಸಂಕೇತ EC = ಗಣ_Delete_Disposition(H, ನಿಜ)) {
    Ret.RemoveOnClose = ನಿಜ;
    ಗಣ_Signal_Handler = ನಿಜ;
  }
#ಇಲ್ಲದಿದ್ದರೆ
  ತಾರ್ಕಿಕ ಗಣ_Signal_Handler = ನಿಜ;
#ಆದರೆ_ಅಂತ್ಯ
  ಆದರೆ (ಗಣ_Signal_Handler && sys::RemoveFileOnSignal(ಫಲಿತಾಂಶ_ಮಾರ್ಗ)) {
    // Make sure we delete the file when RemoveFileOnSignal fails.
    consumeError(Ret.discard());
    std::error_ಸಂಕೇತ EC(errc::operation_not_permitted);
    ಹಿಂತಿರುಗಿ error_ಸಂಕೇತ_ಗಮ್ಯ_Error(EC);
  }
  ಹಿಂತಿರುಗಿ std::move(Ret);
}
} // namespace fs

} // namespace sys
} // namespace llvm
