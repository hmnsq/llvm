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

#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/Path.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/ADT/ArrayRef.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/ADT/ScopeExit.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/ADT/StringExtras.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Config/config.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Config/llvm-config.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/Errc.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/ErrorHandling.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/FileSystem.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/IOSandbox.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/Process.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/Signals.h"
#ਅਨ੍ਤਰ੍ਭਾਵ <cctype>

#ਜੇ !defined(_MSC_VER) && !defined(__MINGW32__)
#ਅਨ੍ਤਰ੍ਭਾਵ <unistd.h>
#ਨਹੀਂਤਾਂ
#ਅਨ੍ਤਰ੍ਭਾਵ <io.h>
#ਜੇ_ਅੰਤ

ਵਰਤੋਂ ਨਾਮਾਕਾਸ਼ llvm;
ਵਰਤੋਂ ਨਾਮਾਕਾਸ਼ llvm::support::endian;

ਨਾਮਾਕਾਸ਼ {
  ਵਰਤੋਂ llvm::StringRef;
  ਵਰਤੋਂ llvm::sys::ਰਸਤਾ_2::is_separator;
  ਵਰਤੋਂ llvm::sys::ਰਸਤਾ_2::Style;

  ਸਤਰਅੰਦਰ ਤਾਰਕਿਕ prefer_forward_slash() {
    ਸਥਿਰ ਤਾਰਕਿਕ prefer = []() {
      ਜੇ (std::optional<std::string> Env =
              sys::Process::GetEnv("LLVM_WINDOWS_PREFER_FORWARD_SLASH"))
        ਵਾਪਸ *Env == "1";
      ਵਾਪਸ static_cast<ਤਾਰਕਿਕ>(LLVM_WINDOWS_PREFER_FORWARD_SLASH);
    }();
    ਵਾਪਸ prefer;
  }

  ਸਤਰਅੰਦਰ Style real_style(Style style) {
    ਜੇ (style != Style::native)
      ਵਾਪਸ style;
    ਜੇ (is_style_posix(style))
      ਵਾਪਸ Style::posix;
    ਵਾਪਸ prefer_forward_slash() ? Style::windows_slash
                                  : Style::windows_backslash;
  }

  ਸਤਰਅੰਦਰ ਅਚਲ ਅੱਖਰ *separators(Style style) {
    ਜੇ (is_style_windows(style))
      ਵਾਪਸ "\\/";
    ਵਾਪਸ "/";
  }

  ਸਤਰਅੰਦਰ ਅੱਖਰ preferred_separator(Style style) {
    ਜੇ (real_style(style) == Style::windows)
      ਵਾਪਸ '\\';
    ਵਾਪਸ '/';
  }

  StringRef ਲੱਭੋ_ਪਹਿਲਾ_component(StringRef ਰਸਤਾ_2, Style style) {
    // Look for this first component in the following order.
    // * empty (in this case we return an empty string)
    // * either C: or {//,\\}net.
    // * {/,\}
    // * {file,directory}name

    ਜੇ (ਰਸਤਾ_2.ਖਾਲੀ_ਥਾਵਾਂ())
      ਵਾਪਸ ਰਸਤਾ_2;

    ਜੇ (is_style_windows(style)) {
      // C:
      ਜੇ (ਰਸਤਾ_2.ਮਾਪ_2() >= 2 &&
          std::ਅੱਖਰ_ਹੈ(static_cast<ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ>(ਰਸਤਾ_2[0])) && ਰਸਤਾ_2[1] == ':')
        ਵਾਪਸ ਰਸਤਾ_2.substr(0, 2);
    }

    // //net
    ਜੇ ((ਰਸਤਾ_2.ਮਾਪ_2() > 2) && is_separator(ਰਸਤਾ_2[0], style) &&
        ਰਸਤਾ_2[0] == ਰਸਤਾ_2[1] && !is_separator(ਰਸਤਾ_2[2], style)) {
      // Find the next directory separator.
      size_t ਸਿਰਾ = ਰਸਤਾ_2.ਲੱਭੋ_ਪਹਿਲਾ_ਦਾ(separators(style), 2);
      ਵਾਪਸ ਰਸਤਾ_2.substr(0, ਸਿਰਾ);
    }

    // {/,\}
    ਜੇ (is_separator(ਰਸਤਾ_2[0], style))
      ਵਾਪਸ ਰਸਤਾ_2.substr(0, 1);

    // * {file,directory}name
    size_t ਸਿਰਾ = ਰਸਤਾ_2.ਲੱਭੋ_ਪਹਿਲਾ_ਦਾ(separators(style));
    ਵਾਪਸ ਰਸਤਾ_2.substr(0, ਸਿਰਾ);
  }

  // Returns the first character of the filename in str. For paths ending in
  // '/', it returns the position of the '/'.
  size_t filename_pos(StringRef str, Style style) {
    ਜੇ (str.ਮਾਪ_2() > 0 && is_separator(str[str.ਮਾਪ_2() - 1], style))
      ਵਾਪਸ str.ਮਾਪ_2() - 1;

    size_t pos = str.ਲੱਭੋ_ਆਖਰੀ_ਦਾ(separators(style), str.ਮਾਪ_2() - 1);

    ਜੇ (is_style_windows(style)) {
      ਜੇ (pos == StringRef::npos)
        pos = str.ਲੱਭੋ_ਆਖਰੀ_ਦਾ(':', str.ਮਾਪ_2() - 1);
    }

    ਜੇ (pos == StringRef::npos || (pos == 1 && is_separator(str[0], style)))
      ਵਾਪਸ 0;

    ਵਾਪਸ pos + 1;
  }

  // Returns the position of the root directory in str. If there is no root
  // directory in str, it returns StringRef::npos.
  size_t ਜੜ੍ਹ_dir_ਸ਼ੁਰੂ(StringRef str, Style style) {
    // case "c:/"
    ਜੇ (is_style_windows(style)) {
      ਜੇ (str.ਮਾਪ_2() > 2 && str[1] == ':' && is_separator(str[2], style))
        ਵਾਪਸ 2;
    }

    // case "//net"
    ਜੇ (str.ਮਾਪ_2() > 3 && is_separator(str[0], style) && str[0] == str[1] &&
        !is_separator(str[2], style)) {
      ਵਾਪਸ str.ਲੱਭੋ_ਪਹਿਲਾ_ਦਾ(separators(style), 2);
    }

    // case "/"
    ਜੇ (str.ਮਾਪ_2() > 0 && is_separator(str[0], style))
      ਵਾਪਸ 0;

    ਵਾਪਸ StringRef::npos;
  }

  // Returns the position past the end of the "parent path" of path. The parent
  // path will not end in '/', unless the parent is the root directory. If the
  // path has no parent, 0 is returned.
  size_t ਮਾਪਾ_ਰਸਤਾ_ਸਿਰਾ(StringRef ਰਸਤਾ_2, Style style) {
    size_t ਸਿਰਾ_pos = filename_pos(ਰਸਤਾ_2, style);

    ਤਾਰਕਿਕ filename_was_sep =
        ਰਸਤਾ_2.ਮਾਪ_2() > 0 && is_separator(ਰਸਤਾ_2[ਸਿਰਾ_pos], style);

    // Skip separators until we reach root dir (or the start of the string).
    size_t ਜੜ੍ਹ_dir_pos = ਜੜ੍ਹ_dir_ਸ਼ੁਰੂ(ਰਸਤਾ_2, style);
    ਜਦੋਂ (ਸਿਰਾ_pos > 0 &&
           (ਜੜ੍ਹ_dir_pos == StringRef::npos || ਸਿਰਾ_pos > ਜੜ੍ਹ_dir_pos) &&
           is_separator(ਰਸਤਾ_2[ਸਿਰਾ_pos - 1], style))
      --ਸਿਰਾ_pos;

    ਜੇ (ਸਿਰਾ_pos == ਜੜ੍ਹ_dir_pos && !filename_was_sep) {
      // We've reached the root dir and the input path was *not* ending in a
      // sequence of slashes. Include the root dir in the parent path.
      ਵਾਪਸ ਜੜ੍ਹ_dir_pos + 1;
    }

    // Otherwise, just include before the last slash.
    ਵਾਪਸ ਸਿਰਾ_pos;
  }
} // end unnamed namespace

ਗਿਣਤੀ FSEntity {
  FS_Dir,
  FS_File,
  FS_Name
};

ਸਥਿਰ std::error_ਕੋਡ
createUniqueEntity(ਅਚਲ Twine &Model, ਪੂਰਨ &ਨਤੀਜਾ_FD,
                   Small_ਸਦਿਸ਼_Impl<ਅੱਖਰ> &ਨਤੀਜਾ_ਰਸਤਾ, ਤਾਰਕਿਕ MakeAbsolute,
                   FSEntity Type, sys::fs::OpenFlags Flags = sys::fs::ਦਾ_None,
                   ਬਿਨਾਨਿਸ਼ਾਨੀ ਬਹੁਲਕ = 0) {

  // Limit the number of attempts we make, so that we don't infinite loop. E.g.
  // "permission denied" could be for a specific file (so we retry with a
  // different name) or for the whole directory (retry would always fail).
  // Checking which is racy, so we try a number of times, then give up.
  std::error_ਕੋਡ EC;
  ਲਈ (ਪੂਰਨ Retries = 128; Retries > 0; --Retries) {
    sys::fs::create_Unique_ਰਸਤਾ(Model, ਨਤੀਜਾ_ਰਸਤਾ, MakeAbsolute);
    // Try to open + create the file.
    ਚੋਣ (Type) {
    ਹਾਲਤ FS_File: {
      EC = sys::fs::openFileForReadWrite(Twine(ਨਤੀਜਾ_ਰਸਤਾ.begin()), ਨਤੀਜਾ_FD,
                                         sys::fs::CD_CreateNew, Flags, ਬਹੁਲਕ);
      ਜੇ (EC) {
        // errc::permission_denied happens on Windows when we try to open a file
        // that has been marked for deletion.
        ਜੇ (EC == errc::file_exists || EC == errc::permission_denied)
          ਜਾਰੀ;
        ਵਾਪਸ EC;
      }

      ਵਾਪਸ std::error_ਕੋਡ();
    }

    ਹਾਲਤ FS_Name: {
      EC = sys::fs::access(ਨਤੀਜਾ_ਰਸਤਾ.begin(), sys::fs::Access_ਬਹੁਲਕ::Exist);
      ਜੇ (EC == errc::no_such_file_or_directory)
        ਵਾਪਸ std::error_ਕੋਡ();
      ਜੇ (EC)
        ਵਾਪਸ EC;
      ਜਾਰੀ;
    }

    ਹਾਲਤ FS_Dir: {
      EC = sys::fs::create_directory(ਨਤੀਜਾ_ਰਸਤਾ.begin(), ਝੂਠ);
      ਜੇ (EC) {
        ਜੇ (EC == errc::file_exists)
          ਜਾਰੀ;
        ਵਾਪਸ EC;
      }
      ਵਾਪਸ std::error_ਕੋਡ();
    }
    }
    llvm_unreachable("Invalid Type");
  }
  ਵਾਪਸ EC;
}

ਨਾਮਾਕਾਸ਼ llvm {
ਨਾਮਾਕਾਸ਼ sys  {
ਨਾਮਾਕਾਸ਼ ਰਸਤਾ_2 {

const_iterator begin(StringRef ਰਸਤਾ_2, Style style) {
  const_iterator i;
  i.ਰਸਤਾ      = ਰਸਤਾ_2;
  i.Component = ਲੱਭੋ_ਪਹਿਲਾ_component(ਰਸਤਾ_2, style);
  i.Position  = 0;
  i.S = style;
  ਵਾਪਸ i;
}

const_iterator ਸਿਰਾ(StringRef ਰਸਤਾ_2) {
  const_iterator i;
  i.ਰਸਤਾ      = ਰਸਤਾ_2;
  i.Position  = ਰਸਤਾ_2.ਮਾਪ_2();
  ਵਾਪਸ i;
}

const_iterator &const_iterator::ਸੰਚਾਲਕ++() {
  ਪ੍ਰਤਿਜ੍ਞਾ(Position < ਰਸਤਾ.ਮਾਪ_2() && "Tried to increment past end!");

  // Increment Position to past the current component
  Position += Component.ਮਾਪ_2();

  // Check for end.
  ਜੇ (Position == ਰਸਤਾ.ਮਾਪ_2()) {
    Component = StringRef();
    ਵਾਪਸ *ਇਹ;
  }

  // Both POSIX and Windows treat paths that begin with exactly two separators
  // specially.
  ਤਾਰਕਿਕ was_net = Component.ਮਾਪ_2() > 2 && is_separator(Component[0], S) &&
                 Component[1] == Component[0] && !is_separator(Component[2], S);

  // Handle separators.
  ਜੇ (is_separator(ਰਸਤਾ[Position], S)) {
    // Root dir.
    ਜੇ (was_net ||
        // c:/
        (is_style_windows(S) && Component.ਸਿਰੇ_with(":"))) {
      Component = ਰਸਤਾ.substr(Position, 1);
      ਵਾਪਸ *ਇਹ;
    }

    // Skip extra separators.
    ਜਦੋਂ (Position != ਰਸਤਾ.ਮਾਪ_2() && is_separator(ਰਸਤਾ[Position], S)) {
      ++Position;
    }

    // Treat trailing '/' as a '.', unless it is the root dir.
    ਜੇ (Position == ਰਸਤਾ.ਮਾਪ_2() && Component != "/") {
      --Position;
      Component = ".";
      ਵਾਪਸ *ਇਹ;
    }
  }

  // Find next component.
  size_t ਸਿਰਾ_pos = ਰਸਤਾ.ਲੱਭੋ_ਪਹਿਲਾ_ਦਾ(separators(S), Position);
  Component = ਰਸਤਾ.ਟੁਕੜਾ(Position, ਸਿਰਾ_pos);

  ਵਾਪਸ *ਇਹ;
}

ਤਾਰਕਿਕ const_iterator::ਸੰਚਾਲਕ==(ਅਚਲ const_iterator &RHS) ਅਚਲ {
  ਵਾਪਸ ਰਸਤਾ.begin() == RHS.ਰਸਤਾ.begin() && Position == RHS.Position;
}

ptrdiff_t const_iterator::ਸੰਚਾਲਕ-(ਅਚਲ const_iterator &RHS) ਅਚਲ {
  ਵਾਪਸ Position - RHS.Position;
}

ਉਲਟ_iterator rbegin(StringRef ਰਸਤਾ, Style style) {
  ਉਲਟ_iterator I;
  I.ਰਸਤਾ = ਰਸਤਾ;
  I.Position = ਰਸਤਾ.ਮਾਪ_2();
  I.S = style;
  ++I;
  ਵਾਪਸ I;
}

ਉਲਟ_iterator rend(StringRef ਰਸਤਾ) {
  ਉਲਟ_iterator I;
  I.ਰਸਤਾ = ਰਸਤਾ;
  I.Component = ਰਸਤਾ.substr(0, 0);
  I.Position = 0;
  ਵਾਪਸ I;
}

ਉਲਟ_iterator &ਉਲਟ_iterator::ਸੰਚਾਲਕ++() {
  size_t ਜੜ੍ਹ_dir_pos = ਜੜ੍ਹ_dir_ਸ਼ੁਰੂ(ਰਸਤਾ, S);

  // Skip separators unless it's the root directory.
  size_t ਸਿਰਾ_pos = Position;
  ਜਦੋਂ (ਸਿਰਾ_pos > 0 && (ਸਿਰਾ_pos - 1) != ਜੜ੍ਹ_dir_pos &&
         is_separator(ਰਸਤਾ[ਸਿਰਾ_pos - 1], S))
    --ਸਿਰਾ_pos;

  // Treat trailing '/' as a '.', unless it is the root dir.
  ਜੇ (Position == ਰਸਤਾ.ਮਾਪ_2() && !ਰਸਤਾ.ਖਾਲੀ_ਥਾਵਾਂ() &&
      is_separator(ਰਸਤਾ.ਪਿੱਠ(), S) &&
      (ਜੜ੍ਹ_dir_pos == StringRef::npos || ਸਿਰਾ_pos - 1 > ਜੜ੍ਹ_dir_pos)) {
    --Position;
    Component = ".";
    ਵਾਪਸ *ਇਹ;
  }

  // Find next separator.
  size_t ਸ਼ੁਰੂ_pos = filename_pos(ਰਸਤਾ.substr(0, ਸਿਰਾ_pos), S);
  Component = ਰਸਤਾ.ਟੁਕੜਾ(ਸ਼ੁਰੂ_pos, ਸਿਰਾ_pos);
  Position = ਸ਼ੁਰੂ_pos;
  ਵਾਪਸ *ਇਹ;
}

ਤਾਰਕਿਕ ਉਲਟ_iterator::ਸੰਚਾਲਕ==(ਅਚਲ ਉਲਟ_iterator &RHS) ਅਚਲ {
  ਵਾਪਸ ਰਸਤਾ.begin() == RHS.ਰਸਤਾ.begin() && Component == RHS.Component &&
         Position == RHS.Position;
}

ptrdiff_t ਉਲਟ_iterator::ਸੰਚਾਲਕ-(ਅਚਲ ਉਲਟ_iterator &RHS) ਅਚਲ {
  ਵਾਪਸ Position - RHS.Position;
}

StringRef ਜੜ੍ਹ_ਰਸਤਾ_2(StringRef ਰਸਤਾ_2, Style style) {
  const_iterator b = begin(ਰਸਤਾ_2, style), pos = b, e = ਸਿਰਾ(ਰਸਤਾ_2);
  ਜੇ (b != e) {
    ਤਾਰਕਿਕ has_net =
        b->ਮਾਪ_2() > 2 && is_separator((*b)[0], style) && (*b)[1] == (*b)[0];
    ਤਾਰਕਿਕ has_drive = is_style_windows(style) && b->ਸਿਰੇ_with(":");

    ਜੇ (has_net || has_drive) {
      ਜੇ ((++pos != e) && is_separator((*pos)[0], style)) {
        // {C:/,//net/}, so get the first two components.
        ਵਾਪਸ ਰਸਤਾ_2.substr(0, b->ਮਾਪ_2() + pos->ਮਾਪ_2());
      }
      // just {C:,//net}, return the first component.
      ਵਾਪਸ *b;
    }

    // POSIX style root directory.
    ਜੇ (is_separator((*b)[0], style)) {
      ਵਾਪਸ *b;
    }
  }

  ਵਾਪਸ StringRef();
}

StringRef ਜੜ੍ਹ_name(StringRef ਰਸਤਾ_2, Style style) {
  const_iterator b = begin(ਰਸਤਾ_2, style), e = ਸਿਰਾ(ਰਸਤਾ_2);
  ਜੇ (b != e) {
    ਤਾਰਕਿਕ has_net =
        b->ਮਾਪ_2() > 2 && is_separator((*b)[0], style) && (*b)[1] == (*b)[0];
    ਤਾਰਕਿਕ has_drive = is_style_windows(style) && b->ਸਿਰੇ_with(":");

    ਜੇ (has_net || has_drive) {
      // just {C:,//net}, return the first component.
      ਵਾਪਸ *b;
    }
  }

  // No path or no name.
  ਵਾਪਸ StringRef();
}

StringRef ਜੜ੍ਹ_directory(StringRef ਰਸਤਾ_2, Style style) {
  const_iterator b = begin(ਰਸਤਾ_2, style), pos = b, e = ਸਿਰਾ(ਰਸਤਾ_2);
  ਜੇ (b != e) {
    ਤਾਰਕਿਕ has_net =
        b->ਮਾਪ_2() > 2 && is_separator((*b)[0], style) && (*b)[1] == (*b)[0];
    ਤਾਰਕਿਕ has_drive = is_style_windows(style) && b->ਸਿਰੇ_with(":");

    ਜੇ ((has_net || has_drive) &&
        // {C:,//net}, skip to the next component.
        (++pos != e) && is_separator((*pos)[0], style)) {
      ਵਾਪਸ *pos;
    }

    // POSIX style root directory.
    ਜੇ (!has_net && is_separator((*b)[0], style)) {
      ਵਾਪਸ *b;
    }
  }

  // No path or no root.
  ਵਾਪਸ StringRef();
}

StringRef relative_ਰਸਤਾ(StringRef ਰਸਤਾ_2, Style style) {
  StringRef ਜੜ੍ਹ_3 = ਜੜ੍ਹ_ਰਸਤਾ_2(ਰਸਤਾ_2, style);
  ਵਾਪਸ ਰਸਤਾ_2.substr(ਜੜ੍ਹ_3.ਮਾਪ_2());
}

ਖਾਲੀ append(Small_ਸਦਿਸ਼_Impl<ਅੱਖਰ> &ਰਸਤਾ_2, Style style, ਅਚਲ Twine &a,
            ਅਚਲ Twine &b, ਅਚਲ Twine &c, ਅਚਲ Twine &d) {
  SmallString<32> a_storage;
  SmallString<32> b_storage;
  SmallString<32> c_storage;
  SmallString<32> d_storage;

  Small_ਸਦਿਸ਼<StringRef, 4> components;
  ਜੇ (!a.is_Trivially_ਖਾਲੀ_ਥਾਵਾਂ()) components.ਧੱਕੋ_ਪਿੱਠ(a.ਮੰਜ਼ਿਲ_String_Ref(a_storage));
  ਜੇ (!b.is_Trivially_ਖਾਲੀ_ਥਾਵਾਂ()) components.ਧੱਕੋ_ਪਿੱਠ(b.ਮੰਜ਼ਿਲ_String_Ref(b_storage));
  ਜੇ (!c.is_Trivially_ਖਾਲੀ_ਥਾਵਾਂ()) components.ਧੱਕੋ_ਪਿੱਠ(c.ਮੰਜ਼ਿਲ_String_Ref(c_storage));
  ਜੇ (!d.is_Trivially_ਖਾਲੀ_ਥਾਵਾਂ()) components.ਧੱਕੋ_ਪਿੱਠ(d.ਮੰਜ਼ਿਲ_String_Ref(d_storage));

  ਲਈ (ਸਵੈਚਾਲਿਤ &component : components) {
    ਤਾਰਕਿਕ ਰਸਤਾ_has_sep =
        !ਰਸਤਾ_2.ਖਾਲੀ_ਥਾਵਾਂ() && is_separator(ਰਸਤਾ_2[ਰਸਤਾ_2.ਮਾਪ_2() - 1], style);
    ਜੇ (ਰਸਤਾ_has_sep) {
      // Strip separators from beginning of component.
      size_t loc = component.ਲੱਭੋ_ਪਹਿਲਾ_not_ਦਾ(separators(style));
      StringRef c = component.substr(loc);

      // Append it.
      ਰਸਤਾ_2.append(c.begin(), c.ਸਿਰਾ());
      ਜਾਰੀ;
    }

    ਤਾਰਕਿਕ component_has_sep =
        !component.ਖਾਲੀ_ਥਾਵਾਂ() && is_separator(component[0], style);
    ਜੇ (!component_has_sep &&
        !(ਰਸਤਾ_2.ਖਾਲੀ_ਥਾਵਾਂ() || has_ਜੜ੍ਹ_name(component, style))) {
      // Add a separator.
      ਰਸਤਾ_2.ਧੱਕੋ_ਪਿੱਠ(preferred_separator(style));
    }

    ਰਸਤਾ_2.append(component.begin(), component.ਸਿਰਾ());
  }
}

ਖਾਲੀ append(Small_ਸਦਿਸ਼_Impl<ਅੱਖਰ> &ਰਸਤਾ_2, ਅਚਲ Twine &a, ਅਚਲ Twine &b,
            ਅਚਲ Twine &c, ਅਚਲ Twine &d) {
  append(ਰਸਤਾ_2, Style::native, a, b, c, d);
}

ਖਾਲੀ append(Small_ਸਦਿਸ਼_Impl<ਅੱਖਰ> &ਰਸਤਾ_2, const_iterator begin,
            const_iterator ਸਿਰਾ, Style style) {
  ਲਈ (; begin != ਸਿਰਾ; ++begin)
    ਰਸਤਾ_2::append(ਰਸਤਾ_2, style, *begin);
}

StringRef ਮਾਪਾ_ਰਸਤਾ(StringRef ਰਸਤਾ_2, Style style) {
  size_t ਸਿਰਾ_pos = ਮਾਪਾ_ਰਸਤਾ_ਸਿਰਾ(ਰਸਤਾ_2, style);
  ਜੇ (ਸਿਰਾ_pos == StringRef::npos)
    ਵਾਪਸ StringRef();
  ਵਾਪਸ ਰਸਤਾ_2.substr(0, ਸਿਰਾ_pos);
}

ਖਾਲੀ remove_filename(Small_ਸਦਿਸ਼_Impl<ਅੱਖਰ> &ਰਸਤਾ_2, Style style) {
  size_t ਸਿਰਾ_pos = ਮਾਪਾ_ਰਸਤਾ_ਸਿਰਾ(StringRef(ਰਸਤਾ_2.begin(), ਰਸਤਾ_2.ਮਾਪ_2()), style);
  ਜੇ (ਸਿਰਾ_pos != StringRef::npos)
    ਰਸਤਾ_2.truncate(ਸਿਰਾ_pos);
}

ਖਾਲੀ replace_extension(Small_ਸਦਿਸ਼_Impl<ਅੱਖਰ> &ਰਸਤਾ_2, ਅਚਲ Twine &extension,
                       Style style) {
  StringRef p(ਰਸਤਾ_2.begin(), ਰਸਤਾ_2.ਮਾਪ_2());
  SmallString<32> ext_storage;
  StringRef ext = extension.ਮੰਜ਼ਿਲ_String_Ref(ext_storage);

  // Erase existing extension.
  size_t pos = p.ਲੱਭੋ_ਆਖਰੀ_ਦਾ('.');
  ਜੇ (pos != StringRef::npos && pos >= filename_pos(p, style))
    ਰਸਤਾ_2.truncate(pos);

  // Append '.' if needed.
  ਜੇ (ext.ਮਾਪ_2() > 0 && ext[0] != '.')
    ਰਸਤਾ_2.ਧੱਕੋ_ਪਿੱਠ('.');

  // Append extension.
  ਰਸਤਾ_2.append(ext.begin(), ext.ਸਿਰਾ());
}

ਸਥਿਰ ਤਾਰਕਿਕ starts_with(StringRef ਰਸਤਾ, StringRef ਅਗੇਤਰ,
                        Style style = Style::native) {
  // Windows prefix matching : case and separator insensitive
  ਜੇ (is_style_windows(style)) {
    ਜੇ (ਰਸਤਾ.ਮਾਪ_2() < ਅਗੇਤਰ.ਮਾਪ_2())
      ਵਾਪਸ ਝੂਠ;
    ਲਈ (size_t I = 0, E = ਅਗੇਤਰ.ਮਾਪ_2(); I != E; ++I) {
      ਤਾਰਕਿਕ Sep_ਰਸਤਾ = is_separator(ਰਸਤਾ[I], style);
      ਤਾਰਕਿਕ Sep_ਅਗੇਤਰ = is_separator(ਅਗੇਤਰ[I], style);
      ਜੇ (Sep_ਰਸਤਾ != Sep_ਅਗੇਤਰ)
        ਵਾਪਸ ਝੂਠ;
      ਜੇ (!Sep_ਰਸਤਾ && ਮੰਜ਼ਿਲ_ਹੇਠਲਾ(ਰਸਤਾ[I]) != ਮੰਜ਼ਿਲ_ਹੇਠਲਾ(ਅਗੇਤਰ[I]))
        ਵਾਪਸ ਝੂਠ;
    }
    ਵਾਪਸ ਸੱਚ;
  }
  ਵਾਪਸ ਰਸਤਾ.starts_with(ਅਗੇਤਰ);
}

ਤਾਰਕਿਕ replace_ਰਸਤਾ_ਅਗੇਤਰ(Small_ਸਦਿਸ਼_Impl<ਅੱਖਰ> &ਰਸਤਾ, StringRef Old_ਅਗੇਤਰ,
                         StringRef New_ਅਗੇਤਰ, Style style) {
  ਜੇ (Old_ਅਗੇਤਰ.ਖਾਲੀ_ਥਾਵਾਂ() && New_ਅਗੇਤਰ.ਖਾਲੀ_ਥਾਵਾਂ())
    ਵਾਪਸ ਝੂਠ;

  StringRef Orig_ਰਸਤਾ(ਰਸਤਾ.begin(), ਰਸਤਾ.ਮਾਪ_2());
  ਜੇ (!starts_with(Orig_ਰਸਤਾ, Old_ਅਗੇਤਰ, style))
    ਵਾਪਸ ਝੂਠ;

  // If prefixes have the same size we can simply copy the new one over.
  ਜੇ (Old_ਅਗੇਤਰ.ਮਾਪ_2() == New_ਅਗੇਤਰ.ਮਾਪ_2()) {
    llvm::copy(New_ਅਗੇਤਰ, ਰਸਤਾ.begin());
    ਵਾਪਸ ਸੱਚ;
  }

  StringRef Rel_ਰਸਤਾ = Orig_ਰਸਤਾ.substr(Old_ਅਗੇਤਰ.ਮਾਪ_2());
  SmallString<256> New_ਰਸਤਾ;
  (Twine(New_ਅਗੇਤਰ) + Rel_ਰਸਤਾ).ਮੰਜ਼ਿਲ_ਸਦਿਸ਼(New_ਰਸਤਾ);
  ਰਸਤਾ.ਅਦਲਾ_ਬਦਲੀ(New_ਰਸਤਾ);
  ਵਾਪਸ ਸੱਚ;
}

ਖਾਲੀ native(ਅਚਲ Twine &ਰਸਤਾ_2, Small_ਸਦਿਸ਼_Impl<ਅੱਖਰ> &ਨਤੀਜਾ_2, Style style) {
  ਪ੍ਰਤਿਜ੍ਞਾ((!ਰਸਤਾ_2.isSingleStringRef() ||
          ਰਸਤਾ_2.getSingleStringRef().ਅੰਕੜੇ() != ਨਤੀਜਾ_2.ਅੰਕੜੇ()) &&
         "path and result are not allowed to overlap!");
  // Clear result.
  ਨਤੀਜਾ_2.clear();
  ਰਸਤਾ_2.ਮੰਜ਼ਿਲ_ਸਦਿਸ਼(ਨਤੀਜਾ_2);
  native(ਨਤੀਜਾ_2, style);
}

std::string native(ਅਚਲ Twine &ਰਸਤਾ_2, Style style) {
  SmallString<128> ਨਤੀਜਾ;
  native(ਰਸਤਾ_2, ਨਤੀਜਾ, style);
  ਵਾਪਸ std::string(ਨਤੀਜਾ);
}

ਖਾਲੀ native(Small_ਸਦਿਸ਼_Impl<ਅੱਖਰ> &ਰਸਤਾ, Style style) {
  ਜੇ (ਰਸਤਾ.ਖਾਲੀ_ਥਾਵਾਂ())
    ਵਾਪਸ;
  ਜੇ (is_style_windows(style)) {
    ਲਈ (ਅੱਖਰ &Ch : ਰਸਤਾ)
      ਜੇ (is_separator(Ch, style))
        Ch = preferred_separator(style);
    ਜੇ (ਰਸਤਾ[0] == '~' && (ਰਸਤਾ.ਮਾਪ_2() == 1 || is_separator(ਰਸਤਾ[1], style))) {
      SmallString<128> ਰਸਤਾ_Home;
      home_directory(ਰਸਤਾ_Home);
      ਰਸਤਾ_Home.append(ਰਸਤਾ.begin() + 1, ਰਸਤਾ.ਸਿਰਾ());
      ਰਸਤਾ = std::move(ਰਸਤਾ_Home);
    }
  } ਨਹੀਂਤਾਂ {
    llvm::replace(ਰਸਤਾ, '\\', '/');
  }
}

std::string convert_ਮੰਜ਼ਿਲ_slash(StringRef ਰਸਤਾ_2, Style style) {
  ਜੇ (is_style_posix(style))
    ਵਾਪਸ std::string(ਰਸਤਾ_2);

  std::string s = ਰਸਤਾ_2.str();
  llvm::replace(s, '\\', '/');
  ਵਾਪਸ s;
}

StringRef filename(StringRef ਰਸਤਾ_2, Style style) { ਵਾਪਸ *rbegin(ਰਸਤਾ_2, style); }

StringRef stem(StringRef ਰਸਤਾ_2, Style style) {
  StringRef fname = filename(ਰਸਤਾ_2, style);
  size_t pos = fname.ਲੱਭੋ_ਆਖਰੀ_ਦਾ('.');
  ਜੇ (pos == StringRef::npos)
    ਵਾਪਸ fname;
  ਜੇ ((fname.ਮਾਪ_2() == 1 && fname == ".") ||
      (fname.ਮਾਪ_2() == 2 && fname == ".."))
    ਵਾਪਸ fname;
  ਵਾਪਸ fname.substr(0, pos);
}

StringRef extension(StringRef ਰਸਤਾ_2, Style style) {
  StringRef fname = filename(ਰਸਤਾ_2, style);
  size_t pos = fname.ਲੱਭੋ_ਆਖਰੀ_ਦਾ('.');
  ਜੇ (pos == StringRef::npos)
    ਵਾਪਸ StringRef();
  ਜੇ ((fname.ਮਾਪ_2() == 1 && fname == ".") ||
      (fname.ਮਾਪ_2() == 2 && fname == ".."))
    ਵਾਪਸ StringRef();
  ਵਾਪਸ fname.substr(pos);
}

ਤਾਰਕਿਕ is_separator(ਅੱਖਰ ਮੁੱਲ_2, Style style) {
  ਜੇ (ਮੁੱਲ_2 == '/')
    ਵਾਪਸ ਸੱਚ;
  ਜੇ (is_style_windows(style))
    ਵਾਪਸ ਮੁੱਲ_2 == '\\';
  ਵਾਪਸ ਝੂਠ;
}

StringRef get_separator(Style style) {
  ਜੇ (real_style(style) == Style::windows)
    ਵਾਪਸ "\\";
  ਵਾਪਸ "/";
}

ਤਾਰਕਿਕ has_ਜੜ੍ਹ_name(ਅਚਲ Twine &ਰਸਤਾ_2, Style style) {
  SmallString<128> ਰਸਤਾ_storage;
  StringRef p = ਰਸਤਾ_2.ਮੰਜ਼ਿਲ_String_Ref(ਰਸਤਾ_storage);

  ਵਾਪਸ !ਜੜ੍ਹ_name(p, style).ਖਾਲੀ_ਥਾਵਾਂ();
}

ਤਾਰਕਿਕ has_ਜੜ੍ਹ_directory(ਅਚਲ Twine &ਰਸਤਾ_2, Style style) {
  SmallString<128> ਰਸਤਾ_storage;
  StringRef p = ਰਸਤਾ_2.ਮੰਜ਼ਿਲ_String_Ref(ਰਸਤਾ_storage);

  ਵਾਪਸ !ਜੜ੍ਹ_directory(p, style).ਖਾਲੀ_ਥਾਵਾਂ();
}

ਤਾਰਕਿਕ has_ਜੜ੍ਹ_ਰਸਤਾ(ਅਚਲ Twine &ਰਸਤਾ_2, Style style) {
  SmallString<128> ਰਸਤਾ_storage;
  StringRef p = ਰਸਤਾ_2.ਮੰਜ਼ਿਲ_String_Ref(ਰਸਤਾ_storage);

  ਵਾਪਸ !ਜੜ੍ਹ_ਰਸਤਾ_2(p, style).ਖਾਲੀ_ਥਾਵਾਂ();
}

ਤਾਰਕਿਕ has_relative_ਰਸਤਾ(ਅਚਲ Twine &ਰਸਤਾ_2, Style style) {
  SmallString<128> ਰਸਤਾ_storage;
  StringRef p = ਰਸਤਾ_2.ਮੰਜ਼ਿਲ_String_Ref(ਰਸਤਾ_storage);

  ਵਾਪਸ !relative_ਰਸਤਾ(p, style).ਖਾਲੀ_ਥਾਵਾਂ();
}

ਤਾਰਕਿਕ has_filename(ਅਚਲ Twine &ਰਸਤਾ_2, Style style) {
  SmallString<128> ਰਸਤਾ_storage;
  StringRef p = ਰਸਤਾ_2.ਮੰਜ਼ਿਲ_String_Ref(ਰਸਤਾ_storage);

  ਵਾਪਸ !filename(p, style).ਖਾਲੀ_ਥਾਵਾਂ();
}

ਤਾਰਕਿਕ has_ਮਾਪਾ_ਰਸਤਾ(ਅਚਲ Twine &ਰਸਤਾ_2, Style style) {
  SmallString<128> ਰਸਤਾ_storage;
  StringRef p = ਰਸਤਾ_2.ਮੰਜ਼ਿਲ_String_Ref(ਰਸਤਾ_storage);

  ਵਾਪਸ !ਮਾਪਾ_ਰਸਤਾ(p, style).ਖਾਲੀ_ਥਾਵਾਂ();
}

ਤਾਰਕਿਕ has_stem(ਅਚਲ Twine &ਰਸਤਾ_2, Style style) {
  SmallString<128> ਰਸਤਾ_storage;
  StringRef p = ਰਸਤਾ_2.ਮੰਜ਼ਿਲ_String_Ref(ਰਸਤਾ_storage);

  ਵਾਪਸ !stem(p, style).ਖਾਲੀ_ਥਾਵਾਂ();
}

ਤਾਰਕਿਕ has_extension(ਅਚਲ Twine &ਰਸਤਾ_2, Style style) {
  SmallString<128> ਰਸਤਾ_storage;
  StringRef p = ਰਸਤਾ_2.ਮੰਜ਼ਿਲ_String_Ref(ਰਸਤਾ_storage);

  ਵਾਪਸ !extension(p, style).ਖਾਲੀ_ਥਾਵਾਂ();
}

ਤਾਰਕਿਕ is_absolute(ਅਚਲ Twine &ਰਸਤਾ_2, Style style) {
  SmallString<128> ਰਸਤਾ_storage;
  StringRef p = ਰਸਤਾ_2.ਮੰਜ਼ਿਲ_String_Ref(ਰਸਤਾ_storage);

  ਤਾਰਕਿਕ ਜੜ੍ਹ_Dir = has_ਜੜ੍ਹ_directory(p, style);
  ਤਾਰਕਿਕ ਜੜ੍ਹ_Name = is_style_posix(style) || has_ਜੜ੍ਹ_name(p, style);

  ਵਾਪਸ ਜੜ੍ਹ_Dir && ਜੜ੍ਹ_Name;
}

ਤਾਰਕਿਕ is_absolute_gnu(ਅਚਲ Twine &ਰਸਤਾ_2, Style style) {
  SmallString<128> ਰਸਤਾ_storage;
  StringRef p = ਰਸਤਾ_2.ਮੰਜ਼ਿਲ_String_Ref(ਰਸਤਾ_storage);

  // Handle '/' which is absolute for both Windows and POSIX systems.
  // Handle '\\' on Windows.
  ਜੇ (!p.ਖਾਲੀ_ਥਾਵਾਂ() && is_separator(p.front(), style))
    ਵਾਪਸ ਸੱਚ;

  ਜੇ (is_style_windows(style)) {
    // Handle drive letter pattern (a character followed by ':') on Windows.
    ਜੇ (p.ਮਾਪ_2() >= 2 && (p[0] && p[1] == ':'))
      ਵਾਪਸ ਸੱਚ;
  }

  ਵਾਪਸ ਝੂਠ;
}

ਤਾਰਕਿਕ is_relative(ਅਚਲ Twine &ਰਸਤਾ_2, Style style) {
  ਵਾਪਸ !is_absolute(ਰਸਤਾ_2, style);
}

ਖਾਲੀ make_absolute(ਅਚਲ Twine &current_directory,
                   Small_ਸਦਿਸ਼_Impl<ਅੱਖਰ> &ਰਸਤਾ_2) {
  StringRef p(ਰਸਤਾ_2.ਅੰਕੜੇ(), ਰਸਤਾ_2.ਮਾਪ_2());

  ਤਾਰਕਿਕ ਜੜ੍ਹ_Directory = has_ਜੜ੍ਹ_directory(p);
  ਤਾਰਕਿਕ ਜੜ੍ਹ_Name = has_ਜੜ੍ਹ_name(p);

  // Already absolute.
  ਜੇ ((ਜੜ੍ਹ_Name || is_style_posix(Style::native)) && ਜੜ੍ਹ_Directory)
    ਵਾਪਸ;

  // All the following conditions will need the current directory.
  SmallString<128> current_dir;
  current_directory.ਮੰਜ਼ਿਲ_ਸਦਿਸ਼(current_dir);

  // Relative path. Prepend the current directory.
  ਜੇ (!ਜੜ੍ਹ_Name && !ਜੜ੍ਹ_Directory) {
    // Append path to the current directory.
    append(current_dir, p);
    // Set path to the result.
    ਰਸਤਾ_2.ਅਦਲਾ_ਬਦਲੀ(current_dir);
    ਵਾਪਸ;
  }

  ਜੇ (!ਜੜ੍ਹ_Name && ਜੜ੍ਹ_Directory) {
    StringRef cdrn = ਜੜ੍ਹ_name(current_dir);
    SmallString<128> cur_Dir_ਜੜ੍ਹ_Name(cdrn.begin(), cdrn.ਸਿਰਾ());
    append(cur_Dir_ਜੜ੍ਹ_Name, p);
    // Set path to the result.
    ਰਸਤਾ_2.ਅਦਲਾ_ਬਦਲੀ(cur_Dir_ਜੜ੍ਹ_Name);
    ਵਾਪਸ;
  }

  ਜੇ (ਜੜ੍ਹ_Name && !ਜੜ੍ਹ_Directory) {
    StringRef p_ਜੜ੍ਹ_Name = ਜੜ੍ਹ_name(p);
    StringRef b_ਜੜ੍ਹ_Directory = ਜੜ੍ਹ_directory(current_dir);
    StringRef b_Relative_ਰਸਤਾ = relative_ਰਸਤਾ(current_dir);
    StringRef p_Relative_ਰਸਤਾ = relative_ਰਸਤਾ(p);

    SmallString<128> res;
    append(res, p_ਜੜ੍ਹ_Name, b_ਜੜ੍ਹ_Directory, b_Relative_ਰਸਤਾ, p_Relative_ਰਸਤਾ);
    ਰਸਤਾ_2.ਅਦਲਾ_ਬਦਲੀ(res);
    ਵਾਪਸ;
  }

  llvm_unreachable("All rootName and rootDirectory combinations should have "
                   "occurred above!");
}

StringRef remove_leading_dotslash(StringRef ਰਸਤਾ, Style style) {
  // Remove leading "./" (or ".//" or "././" etc.)
  ਜਦੋਂ (ਰਸਤਾ.ਮਾਪ_2() > 2 && ਰਸਤਾ[0] == '.' && is_separator(ਰਸਤਾ[1], style)) {
    ਰਸਤਾ = ਰਸਤਾ.substr(2);
    ਜਦੋਂ (ਰਸਤਾ.ਮਾਪ_2() > 0 && is_separator(ਰਸਤਾ[0], style))
      ਰਸਤਾ = ਰਸਤਾ.substr(1);
  }
  ਵਾਪਸ ਰਸਤਾ;
}

ਤਾਰਕਿਕ remove_dots(Small_ਸਦਿਸ਼_Impl<ਅੱਖਰ> &the_ਰਸਤਾ, ਤਾਰਕਿਕ remove_dot_dot,
                 Style style) {
  style = real_style(style);
  StringRef ਬਚਿਆ_2(the_ਰਸਤਾ.ਅੰਕੜੇ(), the_ਰਸਤਾ.ਮਾਪ_2());
  ਤਾਰਕਿਕ needs_change = ਝੂਠ;
  Small_ਸਦਿਸ਼<StringRef, 16> components;

  // Consume the root path, if present.
  StringRef ਜੜ੍ਹ_3 = ਰਸਤਾ_2::ਜੜ੍ਹ_ਰਸਤਾ_2(ਬਚਿਆ_2, style);
  ਤਾਰਕਿਕ absolute = !ਜੜ੍ਹ_3.ਖਾਲੀ_ਥਾਵਾਂ();
  ਜੇ (absolute)
    ਬਚਿਆ_2 = ਬਚਿਆ_2.drop_front(ਜੜ੍ਹ_3.ਮਾਪ_2());

  // Loop over path components manually. This makes it easier to detect
  // non-preferred slashes and double separators that must be canonicalized.
  ਜਦੋਂ (!ਬਚਿਆ_2.ਖਾਲੀ_ਥਾਵਾਂ()) {
    size_t ਅਗਲਾ_slash = ਬਚਿਆ_2.ਲੱਭੋ_ਪਹਿਲਾ_ਦਾ(separators(style));
    ਜੇ (ਅਗਲਾ_slash == StringRef::npos)
      ਅਗਲਾ_slash = ਬਚਿਆ_2.ਮਾਪ_2();
    StringRef component = ਬਚਿਆ_2.take_front(ਅਗਲਾ_slash);
    ਬਚਿਆ_2 = ਬਚਿਆ_2.drop_front(ਅਗਲਾ_slash);

    // Eat the slash, and check if it is the preferred separator.
    ਜੇ (!ਬਚਿਆ_2.ਖਾਲੀ_ਥਾਵਾਂ()) {
      needs_change |= ਬਚਿਆ_2.front() != preferred_separator(style);
      ਬਚਿਆ_2 = ਬਚਿਆ_2.drop_front();
      // The path needs to be rewritten if it has a trailing slash.
      // FIXME: This is emergent behavior that could be removed.
      needs_change |= ਬਚਿਆ_2.ਖਾਲੀ_ਥਾਵਾਂ();
    }

    // Check for path traversal components or double separators.
    ਜੇ (component.ਖਾਲੀ_ਥਾਵਾਂ() || component == ".") {
      needs_change = ਸੱਚ;
    } ਨਹੀਂਤਾਂ ਜੇ (remove_dot_dot && component == "..") {
      needs_change = ਸੱਚ;
      // Do not allow ".." to remove the root component. If this is the
      // beginning of a relative path, keep the ".." component.
      ਜੇ (!components.ਖਾਲੀ_ਥਾਵਾਂ() && components.ਪਿੱਠ() != "..") {
        components.ਕੱਢੋ_ਪਿੱਠ();
      } ਨਹੀਂਤਾਂ ਜੇ (!absolute) {
        components.ਧੱਕੋ_ਪਿੱਠ(component);
      }
    } ਨਹੀਂਤਾਂ {
      components.ਧੱਕੋ_ਪਿੱਠ(component);
    }
  }

  SmallString<256> ਬਫਰ_2 = ਜੜ੍ਹ_3;
  // "root" could be "/", which may need to be translated into "\".
  make_preferred(ਬਫਰ_2, style);
  needs_change |= ਜੜ੍ਹ_3 != ਬਫਰ_2;

  // Avoid rewriting the path unless we have to.
  ਜੇ (!needs_change)
    ਵਾਪਸ ਝੂਠ;

  ਜੇ (!components.ਖਾਲੀ_ਥਾਵਾਂ()) {
    ਬਫਰ_2 += components[0];
    ਲਈ (StringRef C : ArrayRef(components).drop_front()) {
      ਬਫਰ_2 += preferred_separator(style);
      ਬਫਰ_2 += C;
    }
  }
  the_ਰਸਤਾ.ਅਦਲਾ_ਬਦਲੀ(ਬਫਰ_2);
  ਵਾਪਸ ਸੱਚ;
}

} // end namespace path

ਨਾਮਾਕਾਸ਼ fs {

std::error_ਕੋਡ getUniqueID(ਅਚਲ Twine ਰਸਤਾ, UniqueID &ਨਤੀਜਾ) {
  sandbox::violationIfEnabled();

  file_ਸਥਿਤੀ ਸਥਿਤੀ;
  std::error_ਕੋਡ EC = ਸਥਿਤੀ_2(ਰਸਤਾ, ਸਥਿਤੀ);
  ਜੇ (EC)
    ਵਾਪਸ EC;
  ਨਤੀਜਾ = ਸਥਿਤੀ.getUniqueID();
  ਵਾਪਸ std::error_ਕੋਡ();
}

ਖਾਲੀ create_Unique_ਰਸਤਾ(ਅਚਲ Twine &Model, Small_ਸਦਿਸ਼_Impl<ਅੱਖਰ> &ਨਤੀਜਾ_ਰਸਤਾ,
                      ਤਾਰਕਿਕ MakeAbsolute) {
  SmallString<128> ModelStorage;
  Model.ਮੰਜ਼ਿਲ_ਸਦਿਸ਼(ModelStorage);

  ਪ੍ਰਤਿਜ੍ਞਾ(llvm::is_contained(ModelStorage, '%') &&
         "createUniquePath: Model must contain at least one '%'");

  ਜੇ (MakeAbsolute) {
    // Make model absolute by prepending a temp directory if it's not already.
    ਜੇ (!sys::ਰਸਤਾ_2::is_absolute(Twine(ModelStorage))) {
      SmallString<128> TDir;
      sys::ਰਸਤਾ_2::system_ਆਰਜ਼ੀ_directory(ਸੱਚ, TDir);
      sys::ਰਸਤਾ_2::append(TDir, Twine(ModelStorage));
      ModelStorage.ਅਦਲਾ_ਬਦਲੀ(TDir);
    }
  }

  ਨਤੀਜਾ_ਰਸਤਾ = ModelStorage;
  ਨਤੀਜਾ_ਰਸਤਾ.ਧੱਕੋ_ਪਿੱਠ(0);
  ਨਤੀਜਾ_ਰਸਤਾ.ਕੱਢੋ_ਪਿੱਠ();

  // Replace '%' with random chars.
  ਲਈ (ਬਿਨਾਨਿਸ਼ਾਨੀ i = 0, e = ModelStorage.ਮਾਪ_2(); i != e; ++i) {
    ਜੇ (ModelStorage[i] == '%')
      ਨਤੀਜਾ_ਰਸਤਾ[i] = "0123456789abcdef"[sys::Process::Get_Random_ਸੰਖਿਆ() & 15];
  }
}

std::error_ਕੋਡ createUniqueFile(ਅਚਲ Twine &Model, ਪੂਰਨ &ਨਤੀਜਾ_Fd,
                                 Small_ਸਦਿਸ਼_Impl<ਅੱਖਰ> &ਨਤੀਜਾ_ਰਸਤਾ,
                                 OpenFlags Flags, ਬਿਨਾਨਿਸ਼ਾਨੀ ਬਹੁਲਕ) {
  ਵਾਪਸ createUniqueEntity(Model, ਨਤੀਜਾ_Fd, ਨਤੀਜਾ_ਰਸਤਾ, ਝੂਠ, FS_File, Flags,
                            ਬਹੁਲਕ);
}

std::error_ਕੋਡ createUniqueFile(ਅਚਲ Twine &Model,
                                 Small_ਸਦਿਸ਼_Impl<ਅੱਖਰ> &ਨਤੀਜਾ_ਰਸਤਾ,
                                 ਬਿਨਾਨਿਸ਼ਾਨੀ ਬਹੁਲਕ) {
  ਪੂਰਨ FD;
  ਸਵੈਚਾਲਿਤ EC = createUniqueFile(Model, FD, ਨਤੀਜਾ_ਰਸਤਾ, ਦਾ_None, ਬਹੁਲਕ);
  ਜੇ (EC)
    ਵਾਪਸ EC;
  // FD is only needed to avoid race conditions. Close it right away.
  close(FD);
  ਵਾਪਸ EC;
}

ਸਥਿਰ std::error_ਕੋਡ
createTemporaryFile(ਅਚਲ Twine &Model, ਪੂਰਨ &ਨਤੀਜਾ_FD,
                    llvm::Small_ਸਦਿਸ਼_Impl<ਅੱਖਰ> &ਨਤੀਜਾ_ਰਸਤਾ, FSEntity Type,
                    sys::fs::OpenFlags Flags = sys::fs::ਦਾ_None) {
  // Any *temporary* file is assumed to be a compiler-internal output, not
  // a formal one.
  ਸਵੈਚਾਲਿਤ BypassSandbox = sys::sandbox::scopedDisable();

  SmallString<128> Storage;
  StringRef P = Model.ਮੰਜ਼ਿਲ_Null_Terminated_String_Ref(Storage);
  ਪ੍ਰਤਿਜ੍ਞਾ(P.ਲੱਭੋ_ਪਹਿਲਾ_ਦਾ(separators(Style::native)) == StringRef::npos &&
         "Model must be a simple filename.");
  // Use P.begin() so that createUniqueEntity doesn't need to recreate Storage.
  ਵਾਪਸ createUniqueEntity(P.begin(), ਨਤੀਜਾ_FD, ਨਤੀਜਾ_ਰਸਤਾ, ਸੱਚ, Type, Flags,
                            all_read | all_write);
}

ਸਥਿਰ std::error_ਕੋਡ
createTemporaryFile(ਅਚਲ Twine &ਅਗੇਤਰ, StringRef Suffix, ਪੂਰਨ &ਨਤੀਜਾ_FD,
                    llvm::Small_ਸਦਿਸ਼_Impl<ਅੱਖਰ> &ਨਤੀਜਾ_ਰਸਤਾ, FSEntity Type,
                    sys::fs::OpenFlags Flags = sys::fs::ਦਾ_None) {
  ਅਚਲ ਅੱਖਰ *ਵਿਚਕਾਰ = Suffix.ਖਾਲੀ_ਥਾਵਾਂ() ? "-%%%%%%" : "-%%%%%%.";
  ਵਾਪਸ createTemporaryFile(ਅਗੇਤਰ + ਵਿਚਕਾਰ + Suffix, ਨਤੀਜਾ_FD, ਨਤੀਜਾ_ਰਸਤਾ,
                             Type, Flags);
}

std::error_ਕੋਡ createTemporaryFile(ਅਚਲ Twine &ਅਗੇਤਰ, StringRef Suffix,
                                    ਪੂਰਨ &ਨਤੀਜਾ_FD,
                                    Small_ਸਦਿਸ਼_Impl<ਅੱਖਰ> &ਨਤੀਜਾ_ਰਸਤਾ,
                                    sys::fs::OpenFlags Flags) {
  ਵਾਪਸ createTemporaryFile(ਅਗੇਤਰ, Suffix, ਨਤੀਜਾ_FD, ਨਤੀਜਾ_ਰਸਤਾ, FS_File,
                             Flags);
}

std::error_ਕੋਡ createTemporaryFile(ਅਚਲ Twine &ਅਗੇਤਰ, StringRef Suffix,
                                    Small_ਸਦਿਸ਼_Impl<ਅੱਖਰ> &ਨਤੀਜਾ_ਰਸਤਾ,
                                    sys::fs::OpenFlags Flags) {
  ਪੂਰਨ FD;
  ਸਵੈਚਾਲਿਤ EC = createTemporaryFile(ਅਗੇਤਰ, Suffix, FD, ਨਤੀਜਾ_ਰਸਤਾ, Flags);
  ਜੇ (EC)
    ਵਾਪਸ EC;
  // FD is only needed to avoid race conditions. Close it right away.
  close(FD);
  ਵਾਪਸ EC;
}

// This is a mkdtemp with a different pattern. We use createUniqueEntity mostly
// for consistency. We should try using mkdtemp.
std::error_ਕੋਡ createUniqueDirectory(ਅਚਲ Twine &ਅਗੇਤਰ,
                                      Small_ਸਦਿਸ਼_Impl<ਅੱਖਰ> &ਨਤੀਜਾ_ਰਸਤਾ) {
  ਪੂਰਨ Dummy;
  ਵਾਪਸ createUniqueEntity(ਅਗੇਤਰ + "-%%%%%%", Dummy, ਨਤੀਜਾ_ਰਸਤਾ, ਸੱਚ,
                            FS_Dir);
}

std::error_ਕੋਡ
getPotentiallyUniqueFileName(ਅਚਲ Twine &Model,
                             Small_ਸਦਿਸ਼_Impl<ਅੱਖਰ> &ਨਤੀਜਾ_ਰਸਤਾ) {
  ਪੂਰਨ Dummy;
  ਵਾਪਸ createUniqueEntity(Model, Dummy, ਨਤੀਜਾ_ਰਸਤਾ, ਝੂਠ, FS_Name);
}

std::error_ਕੋਡ
get_Potentially_Unique_ਆਰਜ਼ੀ_File_Name(ਅਚਲ Twine &ਅਗੇਤਰ, StringRef Suffix,
                                 Small_ਸਦਿਸ਼_Impl<ਅੱਖਰ> &ਨਤੀਜਾ_ਰਸਤਾ) {
  ਪੂਰਨ Dummy;
  ਵਾਪਸ createTemporaryFile(ਅਗੇਤਰ, Suffix, Dummy, ਨਤੀਜਾ_ਰਸਤਾ, FS_Name);
}

std::error_ਕੋਡ make_absolute(Small_ਸਦਿਸ਼_Impl<ਅੱਖਰ> &ਰਸਤਾ_2) {
  sandbox::violationIfEnabled();

  ਜੇ (ਰਸਤਾ_2::is_absolute(ਰਸਤਾ_2))
    ਵਾਪਸ {};

  SmallString<128> current_dir;
  ਜੇ (std::error_ਕੋਡ ec = current_ਰਸਤਾ(current_dir))
    ਵਾਪਸ ec;

  ਰਸਤਾ_2::make_absolute(current_dir, ਰਸਤਾ_2);
  ਵਾਪਸ {};
}

std::error_ਕੋਡ create_directories(ਅਚਲ Twine &ਰਸਤਾ, ਤਾਰਕਿਕ IgnoreExisting,
                                   perms Perms) {
  SmallString<128> ਰਸਤਾ_Storage;
  StringRef P = ਰਸਤਾ.ਮੰਜ਼ਿਲ_String_Ref(ਰਸਤਾ_Storage);

  // Be optimistic and try to create the directory
  std::error_ਕੋਡ EC = create_directory(P, IgnoreExisting, Perms);
  // If we succeeded, or had any error other than the parent not existing, just
  // return it.
  ਜੇ (EC != errc::no_such_file_or_directory)
    ਵਾਪਸ EC;

  // We failed because of a no_such_file_or_directory, try to create the
  // parent.
  StringRef ਮਾਪਾ_2 = ਰਸਤਾ_2::ਮਾਪਾ_ਰਸਤਾ(P);
  ਜੇ (ਮਾਪਾ_2.ਖਾਲੀ_ਥਾਵਾਂ())
    ਵਾਪਸ EC;

  ਜੇ ((EC = create_directories(ਮਾਪਾ_2, IgnoreExisting, Perms)))
      ਵਾਪਸ EC;

  ਵਾਪਸ create_directory(P, IgnoreExisting, Perms);
}

ਸਥਿਰ std::error_ਕੋਡ copy_file_ਅੰਦਰੂਨੀ(ਪੂਰਨ ReadFD, ਪੂਰਨ WriteFD) {
  ਅਚਲ size_t Buf_ਮਾਪ = 4096;
  ਅੱਖਰ *Buf = ਨਵਾਂ ਅੱਖਰ[Buf_ਮਾਪ];
  ਪੂਰਨ ਬਾਈਟਾਂ_Read = 0, ਬਾਈਟਾਂ_Written = 0;
  ਲਈ (;;) {
    ਬਾਈਟਾਂ_Read = ਪਠ(ReadFD, Buf, Buf_ਮਾਪ);
    ਜੇ (ਬਾਈਟਾਂ_Read <= 0)
      ਰੋਕੋ;
    ਜਦੋਂ (ਬਾਈਟਾਂ_Read) {
      ਬਾਈਟਾਂ_Written = ਲਿਖ(WriteFD, Buf, ਬਾਈਟਾਂ_Read);
      ਜੇ (ਬਾਈਟਾਂ_Written < 0)
        ਰੋਕੋ;
      ਬਾਈਟਾਂ_Read -= ਬਾਈਟਾਂ_Written;
    }
    ਜੇ (ਬਾਈਟਾਂ_Written < 0)
      ਰੋਕੋ;
  }
  ਮਿਟਾਓ[] Buf;

  ਜੇ (ਬਾਈਟਾਂ_Read < 0 || ਬਾਈਟਾਂ_Written < 0)
    ਵਾਪਸ errno_As_Error_ਕੋਡ();
  ਵਾਪਸ std::error_ਕੋਡ();
}

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ __APPLE__
std::error_ਕੋਡ copy_file(ਅਚਲ Twine &From, ਅਚਲ Twine &ਮੰਜ਼ਿਲ) {
  ਪੂਰਨ ReadFD, WriteFD;
  ਜੇ (std::error_ਕੋਡ EC = openFileForRead(From, ReadFD, ਦਾ_None))
    ਵਾਪਸ EC;
  ਜੇ (std::error_ਕੋਡ EC =
          openFileForWrite(ਮੰਜ਼ਿਲ, WriteFD, CD_CreateAlways, ਦਾ_None)) {
    close(ReadFD);
    ਵਾਪਸ EC;
  }

  std::error_ਕੋਡ EC = copy_file_ਅੰਦਰੂਨੀ(ReadFD, WriteFD);

  close(ReadFD);
  close(WriteFD);

  ਵਾਪਸ EC;
}
#ਜੇ_ਅੰਤ

std::error_ਕੋਡ copy_file(ਅਚਲ Twine &From, ਪੂਰਨ ਮੰਜ਼ਿਲ_FD) {
  ਪੂਰਨ ReadFD;
  ਜੇ (std::error_ਕੋਡ EC = openFileForRead(From, ReadFD, ਦਾ_None))
    ਵਾਪਸ EC;

  std::error_ਕੋਡ EC = copy_file_ਅੰਦਰੂਨੀ(ReadFD, ਮੰਜ਼ਿਲ_FD);

  close(ReadFD);

  ਵਾਪਸ EC;
}

ErrorOr<MD5::MD5_ਨਤੀਜਾ> md5_contents(ਪੂਰਨ FD) {
  sandbox::violationIfEnabled();

  MD5 Hash;

  constexpr size_t Buf_ਮਾਪ = 4096;
  std::ਸਦਿਸ਼<uint8_t> Buf(Buf_ਮਾਪ);
  ਪੂਰਨ ਬਾਈਟਾਂ_Read = 0;
  ਲਈ (;;) {
    ਬਾਈਟਾਂ_Read = ਪਠ(FD, Buf.ਅੰਕੜੇ(), Buf_ਮਾਪ);
    ਜੇ (ਬਾਈਟਾਂ_Read <= 0)
      ਰੋਕੋ;
    Hash.update(ArrayRef(Buf.ਅੰਕੜੇ(), ਬਾਈਟਾਂ_Read));
  }

  ਜੇ (ਬਾਈਟਾਂ_Read < 0)
    ਵਾਪਸ errno_As_Error_ਕੋਡ();
  MD5::MD5_ਨਤੀਜਾ ਨਤੀਜਾ;
  Hash.final(ਨਤੀਜਾ);
  ਵਾਪਸ ਨਤੀਜਾ;
}

ErrorOr<MD5::MD5_ਨਤੀਜਾ> md5_contents(ਅਚਲ Twine &ਰਸਤਾ) {
  sandbox::violationIfEnabled();

  ਪੂਰਨ FD;
  ਜੇ (ਸਵੈਚਾਲਿਤ EC = openFileForRead(ਰਸਤਾ, FD, ਦਾ_None))
    ਵਾਪਸ EC;

  ਸਵੈਚਾਲਿਤ ਨਤੀਜਾ = md5_contents(FD);
  close(FD);
  ਵਾਪਸ ਨਤੀਜਾ;
}

ਤਾਰਕਿਕ exists(ਅਚਲ basic_file_ਸਥਿਤੀ &ਸਥਿਤੀ_2) {
  ਵਾਪਸ ਸਥਿਤੀ_ਗਿਆਤ(ਸਥਿਤੀ_2) && ਸਥਿਤੀ_2.type() != file_type::file_not_ਮਿਲਿਆ;
}

ਤਾਰਕਿਕ ਸਥਿਤੀ_ਗਿਆਤ(ਅਚਲ basic_file_ਸਥਿਤੀ &s) {
  ਵਾਪਸ s.type() != file_type::ਸਥਿਤੀ_error;
}

file_type get_file_type(ਅਚਲ Twine &ਰਸਤਾ, ਤਾਰਕਿਕ Follow) {
  file_ਸਥਿਤੀ st;
  ਜੇ (ਸਥਿਤੀ_2(ਰਸਤਾ, st, Follow))
    ਵਾਪਸ file_type::ਸਥਿਤੀ_error;
  ਵਾਪਸ st.type();
}

ਤਾਰਕਿਕ is_directory(ਅਚਲ basic_file_ਸਥਿਤੀ &ਸਥਿਤੀ_2) {
  ਵਾਪਸ ਸਥਿਤੀ_2.type() == file_type::directory_file;
}

std::error_ਕੋਡ is_directory(ਅਚਲ Twine &ਰਸਤਾ_2, ਤਾਰਕਿਕ &ਨਤੀਜਾ_2) {
  sandbox::violationIfEnabled();

  file_ਸਥਿਤੀ st;
  ਜੇ (std::error_ਕੋਡ ec = ਸਥਿਤੀ_2(ਰਸਤਾ_2, st))
    ਵਾਪਸ ec;
  ਨਤੀਜਾ_2 = is_directory(st);
  ਵਾਪਸ std::error_ਕੋਡ();
}

ਤਾਰਕਿਕ is_regular_file(ਅਚਲ basic_file_ਸਥਿਤੀ &ਸਥਿਤੀ_2) {
  ਵਾਪਸ ਸਥਿਤੀ_2.type() == file_type::regular_file;
}

std::error_ਕੋਡ is_regular_file(ਅਚਲ Twine &ਰਸਤਾ_2, ਤਾਰਕਿਕ &ਨਤੀਜਾ_2) {
  sandbox::violationIfEnabled();

  file_ਸਥਿਤੀ st;
  ਜੇ (std::error_ਕੋਡ ec = ਸਥਿਤੀ_2(ਰਸਤਾ_2, st))
    ਵਾਪਸ ec;
  ਨਤੀਜਾ_2 = is_regular_file(st);
  ਵਾਪਸ std::error_ਕੋਡ();
}

ਤਾਰਕਿਕ is_symlink_file(ਅਚਲ basic_file_ਸਥਿਤੀ &ਸਥਿਤੀ_2) {
  ਵਾਪਸ ਸਥਿਤੀ_2.type() == file_type::symlink_file;
}

std::error_ਕੋਡ is_symlink_file(ਅਚਲ Twine &ਰਸਤਾ_2, ਤਾਰਕਿਕ &ਨਤੀਜਾ_2) {
  sandbox::violationIfEnabled();

  file_ਸਥਿਤੀ st;
  ਜੇ (std::error_ਕੋਡ ec = ਸਥਿਤੀ_2(ਰਸਤਾ_2, st, ਝੂਠ))
    ਵਾਪਸ ec;
  ਨਤੀਜਾ_2 = is_symlink_file(st);
  ਵਾਪਸ std::error_ਕੋਡ();
}

ਤਾਰਕਿਕ is_other(ਅਚਲ basic_file_ਸਥਿਤੀ &ਸਥਿਤੀ_2) {
  ਵਾਪਸ exists(ਸਥਿਤੀ_2) &&
         !is_regular_file(ਸਥਿਤੀ_2) &&
         !is_directory(ਸਥਿਤੀ_2);
}

std::error_ਕੋਡ is_other(ਅਚਲ Twine &ਰਸਤਾ, ਤਾਰਕਿਕ &ਨਤੀਜਾ) {
  sandbox::violationIfEnabled();

  file_ਸਥਿਤੀ File_ਸਥਿਤੀ;
  ਜੇ (std::error_ਕੋਡ EC = ਸਥਿਤੀ_2(ਰਸਤਾ, File_ਸਥਿਤੀ))
    ਵਾਪਸ EC;
  ਨਤੀਜਾ = is_other(File_ਸਥਿਤੀ);
  ਵਾਪਸ std::error_ਕੋਡ();
}

ਖਾਲੀ directory_ਇੰਦਰਾਜ::replace_filename(ਅਚਲ Twine &Filename, file_type Type,
                                       basic_file_ਸਥਿਤੀ ਸਥਿਤੀ) {
  SmallString<128> ਰਸਤਾ_Str = ਰਸਤਾ_2::ਮਾਪਾ_ਰਸਤਾ(ਰਸਤਾ);
  ਰਸਤਾ_2::append(ਰਸਤਾ_Str, Filename);
  ਇਹ->ਰਸਤਾ = std::string(ਰਸਤਾ_Str);
  ਇਹ->Type = Type;
  ਇਹ->ਸਥਿਤੀ = ਸਥਿਤੀ;
}

ErrorOr<perms> getPermissions(ਅਚਲ Twine &ਰਸਤਾ) {
  sandbox::violationIfEnabled();

  file_ਸਥਿਤੀ ਸਥਿਤੀ;
  ਜੇ (std::error_ਕੋਡ EC = ਸਥਿਤੀ_2(ਰਸਤਾ, ਸਥਿਤੀ))
    ਵਾਪਸ EC;

  ਵਾਪਸ ਸਥਿਤੀ.permissions();
}

std::error_ਕੋਡ ਸੈੱਟ_ਆਖਰੀ_Access_And_Modification_ਸਮਾਂ(ਅਚਲ Twine &ਰਸਤਾ,
                                                 ਸਮਾਂ_ਬਿੰਦੂ<> Access_ਸਮਾਂ,
                                                 ਸਮਾਂ_ਬਿੰਦੂ<> Modification_ਸਮਾਂ) {
  ਪੂਰਨ FD;
  ਜੇ (std::error_ਕੋਡ EC = openFile(ਰਸਤਾ, FD, CD_OpenExisting, FA_Read,
                                    ਦਾ_Update_ਗੁਣ | ਦਾ_Open_Directory))
    ਵਾਪਸ EC;
  std::error_ਕੋਡ EC =
      ਸੈੱਟ_ਆਖਰੀ_Access_And_Modification_ਸਮਾਂ(FD, Access_ਸਮਾਂ, Modification_ਸਮਾਂ);
  Process::SafelyCloseFileDescriptor(FD);
  ਵਾਪਸ EC;
}

size_t mapped_file_region::ਮਾਪ_2() ਅਚਲ {
  ਪ੍ਰਤਿਜ੍ਞਾ(Mapping && "Mapping failed but used anyway!");
  ਵਾਪਸ ਮਾਪ;
}

ਅੱਖਰ *mapped_file_region::ਅੰਕੜੇ() ਅਚਲ {
  ਪ੍ਰਤਿਜ੍ਞਾ(Mapping && "Mapping failed but used anyway!");
  ਵਾਪਸ reinterpret_cast<ਅੱਖਰ *>(Mapping);
}

ਅਚਲ ਅੱਖਰ *mapped_file_region::const_ਅੰਕੜੇ() ਅਚਲ {
  ਪ੍ਰਤਿਜ੍ਞਾ(Mapping && "Mapping failed but used anyway!");
  ਵਾਪਸ reinterpret_cast<ਅਚਲ ਅੱਖਰ *>(Mapping);
}

Error read_Native_File_ਮੰਜ਼ਿਲ_EOF(file_t FileHandle, Small_ਸਦਿਸ਼_Impl<ਅੱਖਰ> &ਬਫਰ,
                          ssize_t Chunk_ਮਾਪ) {
  sandbox::violationIfEnabled();

  // Install a handler to truncate the buffer to the correct size on exit.
  size_t ਮਾਪ = ਬਫਰ.ਮਾਪ_2();
  llvm::scope_exit TruncateOnExit([&]() { ਬਫਰ.truncate(ਮਾਪ); });

  // Read into Buffer until we hit EOF.
  ਲਈ (;;) {
    ਬਫਰ.resize_for_overwrite(ਮਾਪ + Chunk_ਮਾਪ);
    Expected<size_t> Read_ਬਾਈਟਾਂ = readNativeFile(
        FileHandle, MutableArrayRef(ਬਫਰ.begin() + ਮਾਪ, Chunk_ਮਾਪ));
    ਜੇ (!Read_ਬਾਈਟਾਂ)
      ਵਾਪਸ Read_ਬਾਈਟਾਂ.takeError();
    ਜੇ (*Read_ਬਾਈਟਾਂ == 0)
      ਵਾਪਸ Error::success();
    ਮਾਪ += *Read_ਬਾਈਟਾਂ;
  }
}

} // end namespace fs
} // end namespace sys
} // end namespace llvm

// Include the truly platform-specific parts.
#ਜੇ defined(LLVM_ON_UNIX)
#ਅਨ੍ਤਰ੍ਭਾਵ "Unix/Path.inc"
#ਜੇ_ਅੰਤ
#ਜੇ defined(_WIN32)
#ਅਨ੍ਤਰ੍ਭਾਵ "Windows/Path.inc"
#ਜੇ_ਅੰਤ

ਨਾਮਾਕਾਸ਼ llvm {
ਨਾਮਾਕਾਸ਼ sys {
ਨਾਮਾਕਾਸ਼ fs {

ਆਰਜ਼ੀ_File::ਆਰਜ਼ੀ_File(StringRef Name, ਪੂਰਨ FD)
    : TmpName(std::string(Name)), FD(FD) {}
ਆਰਜ਼ੀ_File::ਆਰਜ਼ੀ_File(ਆਰਜ਼ੀ_File &&Other) { *ਇਹ = std::move(Other); }
ਆਰਜ਼ੀ_File &ਆਰਜ਼ੀ_File::ਸੰਚਾਲਕ=(ਆਰਜ਼ੀ_File &&Other) {
  TmpName = std::move(Other.TmpName);
  FD = Other.FD;
  Other.ਪੂਰਾ = ਸੱਚ;
  Other.FD = -1;
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ _WIN32
  RemoveOnClose = Other.RemoveOnClose;
  Other.RemoveOnClose = ਝੂਠ;
#ਜੇ_ਅੰਤ
  ਵਾਪਸ *ਇਹ;
}

ਆਰਜ਼ੀ_File::~ਆਰਜ਼ੀ_File() { ਪ੍ਰਤਿਜ੍ਞਾ(ਪੂਰਾ); }

Error ਆਰਜ਼ੀ_File::discard() {
  ਪੂਰਾ = ਸੱਚ;
  ਜੇ (FD != -1 && close(FD) == -1) {
    std::error_ਕੋਡ EC = errno_As_Error_ਕੋਡ();
    ਵਾਪਸ error_ਕੋਡ_ਮੰਜ਼ਿਲ_Error(EC);
  }
  FD = -1;

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ _WIN32
  // On Windows, closing will remove the file, if we set the delete
  // disposition. If not, remove it manually.
  ਤਾਰਕਿਕ Remove = RemoveOnClose;
#ਨਹੀਂਤਾਂ
  // Always try to remove the file.
  ਤਾਰਕਿਕ Remove = ਸੱਚ;
#ਜੇ_ਅੰਤ
  std::error_ਕੋਡ RemoveEC;
  ਜੇ (Remove && !TmpName.ਖਾਲੀ_ਥਾਵਾਂ()) {
    RemoveEC = fs::remove(TmpName);
    sys::DontRemoveFileOnSignal(TmpName);
    ਜੇ (!RemoveEC)
      TmpName = "";
  } ਨਹੀਂਤਾਂ {
    TmpName = "";
  }
  ਵਾਪਸ error_ਕੋਡ_ਮੰਜ਼ਿਲ_Error(RemoveEC);
}

Error ਆਰਜ਼ੀ_File::keep(ਅਚਲ Twine &Name) {
  ਪ੍ਰਤਿਜ੍ਞਾ(!ਪੂਰਾ);
  ਪੂਰਾ = ਸੱਚ;
  // Always try to close and rename.
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ _WIN32
  // If we can't cancel the delete don't rename.
  ਸਵੈਚਾਲਿਤ H = reinterpret_cast<HANDLE>(_get_osfhandle(FD));
  std::error_ਕੋਡ RenameEC =
      RemoveOnClose ? std::error_ਕੋਡ() : ਸੈੱਟ_Delete_Disposition(H, ਝੂਠ);
  ਤਾਰਕਿਕ ShouldDelete = ਝੂਠ;
  ਜੇ (!RenameEC) {
    RenameEC = rename_handle(H, Name);
    // If rename failed because it's cross-device, copy instead
    ਜੇ (RenameEC ==
      std::error_ਕੋਡ(ERROR_NOT_SAME_DEVICE, std::system_category())) {
      RenameEC = copy_file(TmpName, Name);
      ShouldDelete = ਸੱਚ;
    }
  }

  // If we can't rename or copy, discard the temporary file.
  ਜੇ (RenameEC)
    ShouldDelete = ਸੱਚ;
  ਜੇ (ShouldDelete) {
    ਜੇ (!RemoveOnClose)
      ਸੈੱਟ_Delete_Disposition(H, ਸੱਚ);
    ਨਹੀਂਤਾਂ
      remove(TmpName);
  }
#ਨਹੀਂਤਾਂ
  std::error_ਕੋਡ RenameEC = fs::rename(TmpName, Name);
  ਜੇ (RenameEC) {
    // If we can't rename, try to copy to work around cross-device link issues.
    RenameEC = sys::fs::copy_file(TmpName, Name);
    // If we can't rename or copy, discard the temporary file.
    ਜੇ (RenameEC)
      remove(TmpName);
  }
#ਜੇ_ਅੰਤ
  sys::DontRemoveFileOnSignal(TmpName);

  ਜੇ (!RenameEC)
    TmpName = "";

  ਜੇ (close(FD) == -1)
    ਵਾਪਸ error_ਕੋਡ_ਮੰਜ਼ਿਲ_Error(errno_As_Error_ਕੋਡ());
  FD = -1;

  ਵਾਪਸ error_ਕੋਡ_ਮੰਜ਼ਿਲ_Error(RenameEC);
}

Error ਆਰਜ਼ੀ_File::keep() {
  ਪ੍ਰਤਿਜ੍ਞਾ(!ਪੂਰਾ);
  ਪੂਰਾ = ਸੱਚ;

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ _WIN32
  ਸਵੈਚਾਲਿਤ H = reinterpret_cast<HANDLE>(_get_osfhandle(FD));
  ਜੇ (std::error_ਕੋਡ EC = ਸੈੱਟ_Delete_Disposition(H, ਝੂਠ))
    ਵਾਪਸ error_ਕੋਡ_ਮੰਜ਼ਿਲ_Error(EC);
#ਜੇ_ਅੰਤ
  sys::DontRemoveFileOnSignal(TmpName);

  TmpName = "";

  ਜੇ (close(FD) == -1)
    ਵਾਪਸ error_ਕੋਡ_ਮੰਜ਼ਿਲ_Error(errno_As_Error_ਕੋਡ());
  FD = -1;

  ਵਾਪਸ Error::success();
}

Expected<ਆਰਜ਼ੀ_File> ਆਰਜ਼ੀ_File::create(ਅਚਲ Twine &Model, ਬਿਨਾਨਿਸ਼ਾਨੀ ਬਹੁਲਕ,
                                    OpenFlags ExtraFlags) {
  ਪੂਰਨ FD;
  SmallString<128> ਨਤੀਜਾ_ਰਸਤਾ;
  ਜੇ (std::error_ਕੋਡ EC =
          createUniqueFile(Model, FD, ਨਤੀਜਾ_ਰਸਤਾ, ਦਾ_Delete | ExtraFlags, ਬਹੁਲਕ))
    ਵਾਪਸ error_ਕੋਡ_ਮੰਜ਼ਿਲ_Error(EC);

  ਆਰਜ਼ੀ_File Ret(ਨਤੀਜਾ_ਰਸਤਾ, FD);
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ _WIN32
  ਸਵੈਚਾਲਿਤ H = reinterpret_cast<HANDLE>(_get_osfhandle(FD));
  ਤਾਰਕਿਕ ਸੈੱਟ_Signal_Handler = ਝੂਠ;
  ਜੇ (std::error_ਕੋਡ EC = ਸੈੱਟ_Delete_Disposition(H, ਸੱਚ)) {
    Ret.RemoveOnClose = ਸੱਚ;
    ਸੈੱਟ_Signal_Handler = ਸੱਚ;
  }
#ਨਹੀਂਤਾਂ
  ਤਾਰਕਿਕ ਸੈੱਟ_Signal_Handler = ਸੱਚ;
#ਜੇ_ਅੰਤ
  ਜੇ (ਸੈੱਟ_Signal_Handler && sys::RemoveFileOnSignal(ਨਤੀਜਾ_ਰਸਤਾ)) {
    // Make sure we delete the file when RemoveFileOnSignal fails.
    consumeError(Ret.discard());
    std::error_ਕੋਡ EC(errc::operation_not_permitted);
    ਵਾਪਸ error_ਕੋਡ_ਮੰਜ਼ਿਲ_Error(EC);
  }
  ਵਾਪਸ std::move(Ret);
}
} // namespace fs

} // namespace sys
} // namespace llvm
