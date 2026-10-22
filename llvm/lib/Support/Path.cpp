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

#ಯದಿ !defined(_MSC_VER) && !defined(__MINGW32__)
#ಅನ್ತರ್ಭಾವ <unistd.h>
#ಅನ್ಯಥಾ
#ಅನ್ತರ್ಭಾವ <io.h>
#endif

using ನಾಮಾಕಾಶ llvm;
using ನಾಮಾಕಾಶ llvm::support::endian;

ನಾಮಾಕಾಶ {
  using llvm::StringRef;
  using llvm::sys::path::is_separator;
  using llvm::sys::path::Style;

  ಅನ್ತಃಸ್ಥ ತಾರ್ಕಿಕ prefer_forward_slash() {
    ಸ್ಥಿರ ತಾರ್ಕಿಕ prefer = []() {
      ಯದಿ (std::optional<std::string> Env =
              sys::Process::GetEnv("LLVM_WINDOWS_PREFER_FORWARD_SLASH"))
        ಪ್ರತ್ಯಾಗಮ *Env == "1";
      ಪ್ರತ್ಯಾಗಮ static_cast<ತಾರ್ಕಿಕ>(LLVM_WINDOWS_PREFER_FORWARD_SLASH);
    }();
    ಪ್ರತ್ಯಾಗಮ prefer;
  }

  ಅನ್ತಃಸ್ಥ Style real_style(Style style) {
    ಯದಿ (style != Style::native)
      ಪ್ರತ್ಯಾಗಮ style;
    ಯದಿ (is_style_posix(style))
      ಪ್ರತ್ಯಾಗಮ Style::posix;
    ಪ್ರತ್ಯಾಗಮ prefer_forward_slash() ? Style::windows_slash
                                  : Style::windows_backslash;
  }

  ಅನ್ತಃಸ್ಥ ಧ್ರುವ ವರ್ಣ *separators(Style style) {
    ಯದಿ (is_style_windows(style))
      ಪ್ರತ್ಯಾಗಮ "\\/";
    ಪ್ರತ್ಯಾಗಮ "/";
  }

  ಅನ್ತಃಸ್ಥ ವರ್ಣ preferred_separator(Style style) {
    ಯದಿ (real_style(style) == Style::windows)
      ಪ್ರತ್ಯಾಗಮ '\\';
    ಪ್ರತ್ಯಾಗಮ '/';
  }

  StringRef find_first_component(StringRef path, Style style) {
    // Look for this first component in the following order.
    // * empty (in this case we return an empty string)
    // * either C: or {//,\\}net.
    // * {/,\}
    // * {file,directory}name

    ಯದಿ (path.empty())
      ಪ್ರತ್ಯಾಗಮ path;

    ಯದಿ (is_style_windows(style)) {
      // C:
      ಯದಿ (path.size() >= 2 &&
          std::isalpha(static_cast<unsigned ವರ್ಣ>(path[0])) && path[1] == ':')
        ಪ್ರತ್ಯಾಗಮ path.substr(0, 2);
    }

    // //net
    ಯದಿ ((path.size() > 2) && is_separator(path[0], style) &&
        path[0] == path[1] && !is_separator(path[2], style)) {
      // Find the next directory separator.
      size_t end = path.find_first_of(separators(style), 2);
      ಪ್ರತ್ಯಾಗಮ path.substr(0, end);
    }

    // {/,\}
    ಯದಿ (is_separator(path[0], style))
      ಪ್ರತ್ಯಾಗಮ path.substr(0, 1);

    // * {file,directory}name
    size_t end = path.find_first_of(separators(style));
    ಪ್ರತ್ಯಾಗಮ path.substr(0, end);
  }

  // Returns the first character of the filename in str. For paths ending in
  // '/', it returns the position of the '/'.
  size_t filename_pos(StringRef str, Style style) {
    ಯದಿ (str.size() > 0 && is_separator(str[str.size() - 1], style))
      ಪ್ರತ್ಯಾಗಮ str.size() - 1;

    size_t pos = str.find_last_of(separators(style), str.size() - 1);

    ಯದಿ (is_style_windows(style)) {
      ಯದಿ (pos == StringRef::npos)
        pos = str.find_last_of(':', str.size() - 1);
    }

    ಯದಿ (pos == StringRef::npos || (pos == 1 && is_separator(str[0], style)))
      ಪ್ರತ್ಯಾಗಮ 0;

    ಪ್ರತ್ಯಾಗಮ pos + 1;
  }

  // Returns the position of the root directory in str. If there is no root
  // directory in str, it returns StringRef::npos.
  size_t ಬೇರು_dir_start(StringRef str, Style style) {
    // case "c:/"
    ಯದಿ (is_style_windows(style)) {
      ಯದಿ (str.size() > 2 && str[1] == ':' && is_separator(str[2], style))
        ಪ್ರತ್ಯಾಗಮ 2;
    }

    // case "//net"
    ಯದಿ (str.size() > 3 && is_separator(str[0], style) && str[0] == str[1] &&
        !is_separator(str[2], style)) {
      ಪ್ರತ್ಯಾಗಮ str.find_first_of(separators(style), 2);
    }

    // case "/"
    ಯದಿ (str.size() > 0 && is_separator(str[0], style))
      ಪ್ರತ್ಯಾಗಮ 0;

    ಪ್ರತ್ಯಾಗಮ StringRef::npos;
  }

  // Returns the position past the end of the "parent path" of path. The parent
  // path will not end in '/', unless the parent is the root directory. If the
  // path has no parent, 0 is returned.
  size_t parent_path_end(StringRef path, Style style) {
    size_t end_pos = filename_pos(path, style);

    ತಾರ್ಕಿಕ filename_was_sep =
        path.size() > 0 && is_separator(path[end_pos], style);

    // Skip separators until we reach root dir (or the start of the string).
    size_t ಬೇರು_dir_pos = ಬೇರು_dir_start(path, style);
    ಯಾವತ್ (end_pos > 0 &&
           (ಬೇರು_dir_pos == StringRef::npos || end_pos > ಬೇರು_dir_pos) &&
           is_separator(path[end_pos - 1], style))
      --end_pos;

    ಯದಿ (end_pos == ಬೇರು_dir_pos && !filename_was_sep) {
      // We've reached the root dir and the input path was *not* ending in a
      // sequence of slashes. Include the root dir in the parent path.
      ಪ್ರತ್ಯಾಗಮ ಬೇರು_dir_pos + 1;
    }

    // Otherwise, just include before the last slash.
    ಪ್ರತ್ಯಾಗಮ end_pos;
  }
} // end unnamed namespace

ಗಣನಾ FSEntity {
  FS_Dir,
  FS_File,
  FS_Name
};

ಸ್ಥಿರ std::error_code
createUniqueEntity(ಧ್ರುವ Twine &Model, ಪೂರ್ಣಾಙ್ಕ &ResultFD,
                   SmallVectorImpl<ವರ್ಣ> &ResultPath, ತಾರ್ಕಿಕ MakeAbsolute,
                   FSEntity Type, sys::fs::OpenFlags Flags = sys::fs::OF_None,
                   unsigned Mode = 0) {

  // Limit the number of attempts we make, so that we don't infinite loop. E.g.
  // "permission denied" could be for a specific file (so we retry with a
  // different name) or for the whole directory (retry would always fail).
  // Checking which is racy, so we try a number of times, then give up.
  std::error_code EC;
  ಪ್ರತಿ (ಪೂರ್ಣಾಙ್ಕ Retries = 128; Retries > 0; --Retries) {
    sys::fs::createUniquePath(Model, ResultPath, MakeAbsolute);
    // Try to open + create the file.
    ವಿಕಲ್ಪ (Type) {
    ಪಕ್ಷ FS_File: {
      EC = sys::fs::openFileForReadWrite(Twine(ResultPath.begin()), ResultFD,
                                         sys::fs::CD_CreateNew, Flags, Mode);
      ಯದಿ (EC) {
        // errc::permission_denied happens on Windows when we try to open a file
        // that has been marked for deletion.
        ಯದಿ (EC == errc::file_exists || EC == errc::permission_denied)
          ಅನುವರ್ತ;
        ಪ್ರತ್ಯಾಗಮ EC;
      }

      ಪ್ರತ್ಯಾಗಮ std::error_code();
    }

    ಪಕ್ಷ FS_Name: {
      EC = sys::fs::access(ResultPath.begin(), sys::fs::AccessMode::Exist);
      ಯದಿ (EC == errc::no_such_file_or_directory)
        ಪ್ರತ್ಯಾಗಮ std::error_code();
      ಯದಿ (EC)
        ಪ್ರತ್ಯಾಗಮ EC;
      ಅನುವರ್ತ;
    }

    ಪಕ್ಷ FS_Dir: {
      EC = sys::fs::create_directory(ResultPath.begin(), ಅಸತ್ಯ);
      ಯದಿ (EC) {
        ಯದಿ (EC == errc::file_exists)
          ಅನುವರ್ತ;
        ಪ್ರತ್ಯಾಗಮ EC;
      }
      ಪ್ರತ್ಯಾಗಮ std::error_code();
    }
    }
    llvm_unreachable("Invalid Type");
  }
  ಪ್ರತ್ಯಾಗಮ EC;
}

ನಾಮಾಕಾಶ llvm {
ನಾಮಾಕಾಶ sys  {
ನಾಮಾಕಾಶ path {

const_iterator begin(StringRef path, Style style) {
  const_iterator i;
  i.Path      = path;
  i.Component = find_first_component(path, style);
  i.Position  = 0;
  i.S = style;
  ಪ್ರತ್ಯಾಗಮ i;
}

const_iterator end(StringRef path) {
  const_iterator i;
  i.Path      = path;
  i.Position  = path.size();
  ಪ್ರತ್ಯಾಗಮ i;
}

const_iterator &const_iterator::operator++() {
  ಪ್ರತಿಜ್ಞಾ(Position < Path.size() && "Tried to increment past end!");

  // Increment Position to past the current component
  Position += Component.size();

  // Check for end.
  ಯದಿ (Position == Path.size()) {
    Component = StringRef();
    ಪ್ರತ್ಯಾಗಮ *ಏತತ್;
  }

  // Both POSIX and Windows treat paths that begin with exactly two separators
  // specially.
  ತಾರ್ಕಿಕ was_net = Component.size() > 2 && is_separator(Component[0], S) &&
                 Component[1] == Component[0] && !is_separator(Component[2], S);

  // Handle separators.
  ಯದಿ (is_separator(Path[Position], S)) {
    // Root dir.
    ಯದಿ (was_net ||
        // c:/
        (is_style_windows(S) && Component.ends_with(":"))) {
      Component = Path.substr(Position, 1);
      ಪ್ರತ್ಯಾಗಮ *ಏತತ್;
    }

    // Skip extra separators.
    ಯಾವತ್ (Position != Path.size() && is_separator(Path[Position], S)) {
      ++Position;
    }

    // Treat trailing '/' as a '.', unless it is the root dir.
    ಯದಿ (Position == Path.size() && Component != "/") {
      --Position;
      Component = ".";
      ಪ್ರತ್ಯಾಗಮ *ಏತತ್;
    }
  }

  // Find next component.
  size_t end_pos = Path.find_first_of(separators(S), Position);
  Component = Path.slice(Position, end_pos);

  ಪ್ರತ್ಯಾಗಮ *ಏತತ್;
}

ತಾರ್ಕಿಕ const_iterator::operator==(ಧ್ರುವ const_iterator &RHS) ಧ್ರುವ {
  ಪ್ರತ್ಯಾಗಮ Path.begin() == RHS.Path.begin() && Position == RHS.Position;
}

ptrdiff_t const_iterator::operator-(ಧ್ರುವ const_iterator &RHS) ಧ್ರುವ {
  ಪ್ರತ್ಯಾಗಮ Position - RHS.Position;
}

reverse_iterator rbegin(StringRef Path, Style style) {
  reverse_iterator I;
  I.Path = Path;
  I.Position = Path.size();
  I.S = style;
  ++I;
  ಪ್ರತ್ಯಾಗಮ I;
}

reverse_iterator rend(StringRef Path) {
  reverse_iterator I;
  I.Path = Path;
  I.Component = Path.substr(0, 0);
  I.Position = 0;
  ಪ್ರತ್ಯಾಗಮ I;
}

reverse_iterator &reverse_iterator::operator++() {
  size_t ಬೇರು_dir_pos = ಬೇರು_dir_start(Path, S);

  // Skip separators unless it's the root directory.
  size_t end_pos = Position;
  ಯಾವತ್ (end_pos > 0 && (end_pos - 1) != ಬೇರು_dir_pos &&
         is_separator(Path[end_pos - 1], S))
    --end_pos;

  // Treat trailing '/' as a '.', unless it is the root dir.
  ಯದಿ (Position == Path.size() && !Path.empty() &&
      is_separator(Path.back(), S) &&
      (ಬೇರು_dir_pos == StringRef::npos || end_pos - 1 > ಬೇರು_dir_pos)) {
    --Position;
    Component = ".";
    ಪ್ರತ್ಯಾಗಮ *ಏತತ್;
  }

  // Find next separator.
  size_t start_pos = filename_pos(Path.substr(0, end_pos), S);
  Component = Path.slice(start_pos, end_pos);
  Position = start_pos;
  ಪ್ರತ್ಯಾಗಮ *ಏತತ್;
}

ತಾರ್ಕಿಕ reverse_iterator::operator==(ಧ್ರುವ reverse_iterator &RHS) ಧ್ರುವ {
  ಪ್ರತ್ಯಾಗಮ Path.begin() == RHS.Path.begin() && Component == RHS.Component &&
         Position == RHS.Position;
}

ptrdiff_t reverse_iterator::operator-(ಧ್ರುವ reverse_iterator &RHS) ಧ್ರುವ {
  ಪ್ರತ್ಯಾಗಮ Position - RHS.Position;
}

StringRef ಬೇರು_path(StringRef path, Style style) {
  const_iterator b = begin(path, style), pos = b, e = end(path);
  ಯದಿ (b != e) {
    ತಾರ್ಕಿಕ has_net =
        b->size() > 2 && is_separator((*b)[0], style) && (*b)[1] == (*b)[0];
    ತಾರ್ಕಿಕ has_drive = is_style_windows(style) && b->ends_with(":");

    ಯದಿ (has_net || has_drive) {
      ಯದಿ ((++pos != e) && is_separator((*pos)[0], style)) {
        // {C:/,//net/}, so get the first two components.
        ಪ್ರತ್ಯಾಗಮ path.substr(0, b->size() + pos->size());
      }
      // just {C:,//net}, return the first component.
      ಪ್ರತ್ಯಾಗಮ *b;
    }

    // POSIX style root directory.
    ಯದಿ (is_separator((*b)[0], style)) {
      ಪ್ರತ್ಯಾಗಮ *b;
    }
  }

  ಪ್ರತ್ಯಾಗಮ StringRef();
}

StringRef ಬೇರು_name(StringRef path, Style style) {
  const_iterator b = begin(path, style), e = end(path);
  ಯದಿ (b != e) {
    ತಾರ್ಕಿಕ has_net =
        b->size() > 2 && is_separator((*b)[0], style) && (*b)[1] == (*b)[0];
    ತಾರ್ಕಿಕ has_drive = is_style_windows(style) && b->ends_with(":");

    ಯದಿ (has_net || has_drive) {
      // just {C:,//net}, return the first component.
      ಪ್ರತ್ಯಾಗಮ *b;
    }
  }

  // No path or no name.
  ಪ್ರತ್ಯಾಗಮ StringRef();
}

StringRef ಬೇರು_directory(StringRef path, Style style) {
  const_iterator b = begin(path, style), pos = b, e = end(path);
  ಯದಿ (b != e) {
    ತಾರ್ಕಿಕ has_net =
        b->size() > 2 && is_separator((*b)[0], style) && (*b)[1] == (*b)[0];
    ತಾರ್ಕಿಕ has_drive = is_style_windows(style) && b->ends_with(":");

    ಯದಿ ((has_net || has_drive) &&
        // {C:,//net}, skip to the next component.
        (++pos != e) && is_separator((*pos)[0], style)) {
      ಪ್ರತ್ಯಾಗಮ *pos;
    }

    // POSIX style root directory.
    ಯದಿ (!has_net && is_separator((*b)[0], style)) {
      ಪ್ರತ್ಯಾಗಮ *b;
    }
  }

  // No path or no root.
  ಪ್ರತ್ಯಾಗಮ StringRef();
}

StringRef relative_path(StringRef path, Style style) {
  StringRef ಬೇರು_3 = ಬೇರು_path(path, style);
  ಪ್ರತ್ಯಾಗಮ path.substr(ಬೇರು_3.size());
}

ಶೂನ್ಯ append(SmallVectorImpl<ವರ್ಣ> &path, Style style, ಧ್ರುವ Twine &a,
            ಧ್ರುವ Twine &b, ಧ್ರುವ Twine &c, ಧ್ರುವ Twine &d) {
  SmallString<32> a_storage;
  SmallString<32> b_storage;
  SmallString<32> c_storage;
  SmallString<32> d_storage;

  SmallVector<StringRef, 4> components;
  ಯದಿ (!a.isTriviallyEmpty()) components.ತಳ್ಳು_back(a.toStringRef(a_storage));
  ಯದಿ (!b.isTriviallyEmpty()) components.ತಳ್ಳು_back(b.toStringRef(b_storage));
  ಯದಿ (!c.isTriviallyEmpty()) components.ತಳ್ಳು_back(c.toStringRef(c_storage));
  ಯದಿ (!d.isTriviallyEmpty()) components.ತಳ್ಳು_back(d.toStringRef(d_storage));

  ಪ್ರತಿ (auto &component : components) {
    ತಾರ್ಕಿಕ path_has_sep =
        !path.empty() && is_separator(path[path.size() - 1], style);
    ಯದಿ (path_has_sep) {
      // Strip separators from beginning of component.
      size_t loc = component.find_first_not_of(separators(style));
      StringRef c = component.substr(loc);

      // Append it.
      path.append(c.begin(), c.end());
      ಅನುವರ್ತ;
    }

    ತಾರ್ಕಿಕ component_has_sep =
        !component.empty() && is_separator(component[0], style);
    ಯದಿ (!component_has_sep &&
        !(path.empty() || has_ಬೇರು_name(component, style))) {
      // Add a separator.
      path.ತಳ್ಳು_back(preferred_separator(style));
    }

    path.append(component.begin(), component.end());
  }
}

ಶೂನ್ಯ append(SmallVectorImpl<ವರ್ಣ> &path, ಧ್ರುವ Twine &a, ಧ್ರುವ Twine &b,
            ಧ್ರುವ Twine &c, ಧ್ರುವ Twine &d) {
  append(path, Style::native, a, b, c, d);
}

ಶೂನ್ಯ append(SmallVectorImpl<ವರ್ಣ> &path, const_iterator begin,
            const_iterator end, Style style) {
  ಪ್ರತಿ (; begin != end; ++begin)
    path::append(path, style, *begin);
}

StringRef parent_path(StringRef path, Style style) {
  size_t end_pos = parent_path_end(path, style);
  ಯದಿ (end_pos == StringRef::npos)
    ಪ್ರತ್ಯಾಗಮ StringRef();
  ಪ್ರತ್ಯಾಗಮ path.substr(0, end_pos);
}

ಶೂನ್ಯ remove_filename(SmallVectorImpl<ವರ್ಣ> &path, Style style) {
  size_t end_pos = parent_path_end(StringRef(path.begin(), path.size()), style);
  ಯದಿ (end_pos != StringRef::npos)
    path.truncate(end_pos);
}

ಶೂನ್ಯ replace_extension(SmallVectorImpl<ವರ್ಣ> &path, ಧ್ರುವ Twine &extension,
                       Style style) {
  StringRef p(path.begin(), path.size());
  SmallString<32> ext_storage;
  StringRef ext = extension.toStringRef(ext_storage);

  // Erase existing extension.
  size_t pos = p.find_last_of('.');
  ಯದಿ (pos != StringRef::npos && pos >= filename_pos(p, style))
    path.truncate(pos);

  // Append '.' if needed.
  ಯದಿ (ext.size() > 0 && ext[0] != '.')
    path.ತಳ್ಳು_back('.');

  // Append extension.
  path.append(ext.begin(), ext.end());
}

ಸ್ಥಿರ ತಾರ್ಕಿಕ starts_with(StringRef Path, StringRef Prefix,
                        Style style = Style::native) {
  // Windows prefix matching : case and separator insensitive
  ಯದಿ (is_style_windows(style)) {
    ಯದಿ (Path.size() < Prefix.size())
      ಪ್ರತ್ಯಾಗಮ ಅಸತ್ಯ;
    ಪ್ರತಿ (size_t I = 0, E = Prefix.size(); I != E; ++I) {
      ತಾರ್ಕಿಕ SepPath = is_separator(Path[I], style);
      ತಾರ್ಕಿಕ SepPrefix = is_separator(Prefix[I], style);
      ಯದಿ (SepPath != SepPrefix)
        ಪ್ರತ್ಯಾಗಮ ಅಸತ್ಯ;
      ಯದಿ (!SepPath && toLower(Path[I]) != toLower(Prefix[I]))
        ಪ್ರತ್ಯಾಗಮ ಅಸತ್ಯ;
    }
    ಪ್ರತ್ಯಾಗಮ ಸತ್ಯ;
  }
  ಪ್ರತ್ಯಾಗಮ Path.starts_with(Prefix);
}

ತಾರ್ಕಿಕ replace_path_prefix(SmallVectorImpl<ವರ್ಣ> &Path, StringRef OldPrefix,
                         StringRef NewPrefix, Style style) {
  ಯದಿ (OldPrefix.empty() && NewPrefix.empty())
    ಪ್ರತ್ಯಾಗಮ ಅಸತ್ಯ;

  StringRef OrigPath(Path.begin(), Path.size());
  ಯದಿ (!starts_with(OrigPath, OldPrefix, style))
    ಪ್ರತ್ಯಾಗಮ ಅಸತ್ಯ;

  // If prefixes have the same size we can simply copy the new one over.
  ಯದಿ (OldPrefix.size() == NewPrefix.size()) {
    llvm::copy(NewPrefix, Path.begin());
    ಪ್ರತ್ಯಾಗಮ ಸತ್ಯ;
  }

  StringRef RelPath = OrigPath.substr(OldPrefix.size());
  SmallString<256> NewPath;
  (Twine(NewPrefix) + RelPath).toVector(NewPath);
  Path.swap(NewPath);
  ಪ್ರತ್ಯಾಗಮ ಸತ್ಯ;
}

ಶೂನ್ಯ native(ಧ್ರುವ Twine &path, SmallVectorImpl<ವರ್ಣ> &result, Style style) {
  ಪ್ರತಿಜ್ಞಾ((!path.isSingleStringRef() ||
          path.getSingleStringRef().data() != result.data()) &&
         "path and result are not allowed to overlap!");
  // Clear result.
  result.clear();
  path.toVector(result);
  native(result, style);
}

std::string native(ಧ್ರುವ Twine &path, Style style) {
  SmallString<128> Result;
  native(path, Result, style);
  ಪ್ರತ್ಯಾಗಮ std::string(Result);
}

ಶೂನ್ಯ native(SmallVectorImpl<ವರ್ಣ> &Path, Style style) {
  ಯದಿ (Path.empty())
    ಪ್ರತ್ಯಾಗಮ;
  ಯದಿ (is_style_windows(style)) {
    ಪ್ರತಿ (ವರ್ಣ &Ch : Path)
      ಯದಿ (is_separator(Ch, style))
        Ch = preferred_separator(style);
    ಯದಿ (Path[0] == '~' && (Path.size() == 1 || is_separator(Path[1], style))) {
      SmallString<128> PathHome;
      home_directory(PathHome);
      PathHome.append(Path.begin() + 1, Path.end());
      Path = std::move(PathHome);
    }
  } ಅನ್ಯಥಾ {
    llvm::replace(Path, '\\', '/');
  }
}

std::string convert_to_slash(StringRef path, Style style) {
  ಯದಿ (is_style_posix(style))
    ಪ್ರತ್ಯಾಗಮ std::string(path);

  std::string s = path.str();
  llvm::replace(s, '\\', '/');
  ಪ್ರತ್ಯಾಗಮ s;
}

StringRef filename(StringRef path, Style style) { ಪ್ರತ್ಯಾಗಮ *rbegin(path, style); }

StringRef stem(StringRef path, Style style) {
  StringRef fname = filename(path, style);
  size_t pos = fname.find_last_of('.');
  ಯದಿ (pos == StringRef::npos)
    ಪ್ರತ್ಯಾಗಮ fname;
  ಯದಿ ((fname.size() == 1 && fname == ".") ||
      (fname.size() == 2 && fname == ".."))
    ಪ್ರತ್ಯಾಗಮ fname;
  ಪ್ರತ್ಯಾಗಮ fname.substr(0, pos);
}

StringRef extension(StringRef path, Style style) {
  StringRef fname = filename(path, style);
  size_t pos = fname.find_last_of('.');
  ಯದಿ (pos == StringRef::npos)
    ಪ್ರತ್ಯಾಗಮ StringRef();
  ಯದಿ ((fname.size() == 1 && fname == ".") ||
      (fname.size() == 2 && fname == ".."))
    ಪ್ರತ್ಯಾಗಮ StringRef();
  ಪ್ರತ್ಯಾಗಮ fname.substr(pos);
}

ತಾರ್ಕಿಕ is_separator(ವರ್ಣ ಮೌಲ್ಯ_2, Style style) {
  ಯದಿ (ಮೌಲ್ಯ_2 == '/')
    ಪ್ರತ್ಯಾಗಮ ಸತ್ಯ;
  ಯದಿ (is_style_windows(style))
    ಪ್ರತ್ಯಾಗಮ ಮೌಲ್ಯ_2 == '\\';
  ಪ್ರತ್ಯಾಗಮ ಅಸತ್ಯ;
}

StringRef get_separator(Style style) {
  ಯದಿ (real_style(style) == Style::windows)
    ಪ್ರತ್ಯಾಗಮ "\\";
  ಪ್ರತ್ಯಾಗಮ "/";
}

ತಾರ್ಕಿಕ has_ಬೇರು_name(ಧ್ರುವ Twine &path, Style style) {
  SmallString<128> path_storage;
  StringRef p = path.toStringRef(path_storage);

  ಪ್ರತ್ಯಾಗಮ !ಬೇರು_name(p, style).empty();
}

ತಾರ್ಕಿಕ has_ಬೇರು_directory(ಧ್ರುವ Twine &path, Style style) {
  SmallString<128> path_storage;
  StringRef p = path.toStringRef(path_storage);

  ಪ್ರತ್ಯಾಗಮ !ಬೇರು_directory(p, style).empty();
}

ತಾರ್ಕಿಕ has_ಬೇರು_path(ಧ್ರುವ Twine &path, Style style) {
  SmallString<128> path_storage;
  StringRef p = path.toStringRef(path_storage);

  ಪ್ರತ್ಯಾಗಮ !ಬೇರು_path(p, style).empty();
}

ತಾರ್ಕಿಕ has_relative_path(ಧ್ರುವ Twine &path, Style style) {
  SmallString<128> path_storage;
  StringRef p = path.toStringRef(path_storage);

  ಪ್ರತ್ಯಾಗಮ !relative_path(p, style).empty();
}

ತಾರ್ಕಿಕ has_filename(ಧ್ರುವ Twine &path, Style style) {
  SmallString<128> path_storage;
  StringRef p = path.toStringRef(path_storage);

  ಪ್ರತ್ಯಾಗಮ !filename(p, style).empty();
}

ತಾರ್ಕಿಕ has_parent_path(ಧ್ರುವ Twine &path, Style style) {
  SmallString<128> path_storage;
  StringRef p = path.toStringRef(path_storage);

  ಪ್ರತ್ಯಾಗಮ !parent_path(p, style).empty();
}

ತಾರ್ಕಿಕ has_stem(ಧ್ರುವ Twine &path, Style style) {
  SmallString<128> path_storage;
  StringRef p = path.toStringRef(path_storage);

  ಪ್ರತ್ಯಾಗಮ !stem(p, style).empty();
}

ತಾರ್ಕಿಕ has_extension(ಧ್ರುವ Twine &path, Style style) {
  SmallString<128> path_storage;
  StringRef p = path.toStringRef(path_storage);

  ಪ್ರತ್ಯಾಗಮ !extension(p, style).empty();
}

ತಾರ್ಕಿಕ is_absolute(ಧ್ರುವ Twine &path, Style style) {
  SmallString<128> path_storage;
  StringRef p = path.toStringRef(path_storage);

  ತಾರ್ಕಿಕ ಬೇರು_Dir = has_ಬೇರು_directory(p, style);
  ತಾರ್ಕಿಕ ಬೇರು_Name = is_style_posix(style) || has_ಬೇರು_name(p, style);

  ಪ್ರತ್ಯಾಗಮ ಬೇರು_Dir && ಬೇರು_Name;
}

ತಾರ್ಕಿಕ is_absolute_gnu(ಧ್ರುವ Twine &path, Style style) {
  SmallString<128> path_storage;
  StringRef p = path.toStringRef(path_storage);

  // Handle '/' which is absolute for both Windows and POSIX systems.
  // Handle '\\' on Windows.
  ಯದಿ (!p.empty() && is_separator(p.front(), style))
    ಪ್ರತ್ಯಾಗಮ ಸತ್ಯ;

  ಯದಿ (is_style_windows(style)) {
    // Handle drive letter pattern (a character followed by ':') on Windows.
    ಯದಿ (p.size() >= 2 && (p[0] && p[1] == ':'))
      ಪ್ರತ್ಯಾಗಮ ಸತ್ಯ;
  }

  ಪ್ರತ್ಯಾಗಮ ಅಸತ್ಯ;
}

ತಾರ್ಕಿಕ is_relative(ಧ್ರುವ Twine &path, Style style) {
  ಪ್ರತ್ಯಾಗಮ !is_absolute(path, style);
}

ಶೂನ್ಯ make_absolute(ಧ್ರುವ Twine &current_directory,
                   SmallVectorImpl<ವರ್ಣ> &path) {
  StringRef p(path.data(), path.size());

  ತಾರ್ಕಿಕ ಬೇರು_Directory = has_ಬೇರು_directory(p);
  ತಾರ್ಕಿಕ ಬೇರು_Name = has_ಬೇರು_name(p);

  // Already absolute.
  ಯದಿ ((ಬೇರು_Name || is_style_posix(Style::native)) && ಬೇರು_Directory)
    ಪ್ರತ್ಯಾಗಮ;

  // All the following conditions will need the current directory.
  SmallString<128> current_dir;
  current_directory.toVector(current_dir);

  // Relative path. Prepend the current directory.
  ಯದಿ (!ಬೇರು_Name && !ಬೇರು_Directory) {
    // Append path to the current directory.
    append(current_dir, p);
    // Set path to the result.
    path.swap(current_dir);
    ಪ್ರತ್ಯಾಗಮ;
  }

  ಯದಿ (!ಬೇರು_Name && ಬೇರು_Directory) {
    StringRef cdrn = ಬೇರು_name(current_dir);
    SmallString<128> cur_Dir_ಬೇರು_Name(cdrn.begin(), cdrn.end());
    append(cur_Dir_ಬೇರು_Name, p);
    // Set path to the result.
    path.swap(cur_Dir_ಬೇರು_Name);
    ಪ್ರತ್ಯಾಗಮ;
  }

  ಯದಿ (ಬೇರು_Name && !ಬೇರು_Directory) {
    StringRef p_ಬೇರು_Name = ಬೇರು_name(p);
    StringRef b_ಬೇರು_Directory = ಬೇರು_directory(current_dir);
    StringRef bRelativePath = relative_path(current_dir);
    StringRef pRelativePath = relative_path(p);

    SmallString<128> res;
    append(res, p_ಬೇರು_Name, b_ಬೇರು_Directory, bRelativePath, pRelativePath);
    path.swap(res);
    ಪ್ರತ್ಯಾಗಮ;
  }

  llvm_unreachable("All rootName and rootDirectory combinations should have "
                   "occurred above!");
}

StringRef remove_leading_dotslash(StringRef Path, Style style) {
  // Remove leading "./" (or ".//" or "././" etc.)
  ಯಾವತ್ (Path.size() > 2 && Path[0] == '.' && is_separator(Path[1], style)) {
    Path = Path.substr(2);
    ಯಾವತ್ (Path.size() > 0 && is_separator(Path[0], style))
      Path = Path.substr(1);
  }
  ಪ್ರತ್ಯಾಗಮ Path;
}

ತಾರ್ಕಿಕ remove_dots(SmallVectorImpl<ವರ್ಣ> &the_path, ತಾರ್ಕಿಕ remove_dot_dot,
                 Style style) {
  style = real_style(style);
  StringRef remaining(the_path.data(), the_path.size());
  ತಾರ್ಕಿಕ needs_change = ಅಸತ್ಯ;
  SmallVector<StringRef, 16> components;

  // Consume the root path, if present.
  StringRef ಬೇರು_3 = path::ಬೇರು_path(remaining, style);
  ತಾರ್ಕಿಕ absolute = !ಬೇರು_3.empty();
  ಯದಿ (absolute)
    remaining = remaining.drop_front(ಬೇರು_3.size());

  // Loop over path components manually. This makes it easier to detect
  // non-preferred slashes and double separators that must be canonicalized.
  ಯಾವತ್ (!remaining.empty()) {
    size_t next_slash = remaining.find_first_of(separators(style));
    ಯದಿ (next_slash == StringRef::npos)
      next_slash = remaining.size();
    StringRef component = remaining.take_front(next_slash);
    remaining = remaining.drop_front(next_slash);

    // Eat the slash, and check if it is the preferred separator.
    ಯದಿ (!remaining.empty()) {
      needs_change |= remaining.front() != preferred_separator(style);
      remaining = remaining.drop_front();
      // The path needs to be rewritten if it has a trailing slash.
      // FIXME: This is emergent behavior that could be removed.
      needs_change |= remaining.empty();
    }

    // Check for path traversal components or double separators.
    ಯದಿ (component.empty() || component == ".") {
      needs_change = ಸತ್ಯ;
    } ಅನ್ಯಥಾ ಯದಿ (remove_dot_dot && component == "..") {
      needs_change = ಸತ್ಯ;
      // Do not allow ".." to remove the root component. If this is the
      // beginning of a relative path, keep the ".." component.
      ಯದಿ (!components.empty() && components.back() != "..") {
        components.ತೆಗೆ_back();
      } ಅನ್ಯಥಾ ಯದಿ (!absolute) {
        components.ತಳ್ಳು_back(component);
      }
    } ಅನ್ಯಥಾ {
      components.ತಳ್ಳು_back(component);
    }
  }

  SmallString<256> buffer = ಬೇರು_3;
  // "root" could be "/", which may need to be translated into "\".
  make_preferred(buffer, style);
  needs_change |= ಬೇರು_3 != buffer;

  // Avoid rewriting the path unless we have to.
  ಯದಿ (!needs_change)
    ಪ್ರತ್ಯಾಗಮ ಅಸತ್ಯ;

  ಯದಿ (!components.empty()) {
    buffer += components[0];
    ಪ್ರತಿ (StringRef C : ArrayRef(components).drop_front()) {
      buffer += preferred_separator(style);
      buffer += C;
    }
  }
  the_path.swap(buffer);
  ಪ್ರತ್ಯಾಗಮ ಸತ್ಯ;
}

} // end namespace path

ನಾಮಾಕಾಶ fs {

std::error_code getUniqueID(ಧ್ರುವ Twine Path, UniqueID &Result) {
  sandbox::violationIfEnabled();

  file_status Status;
  std::error_code EC = status(Path, Status);
  ಯದಿ (EC)
    ಪ್ರತ್ಯಾಗಮ EC;
  Result = Status.getUniqueID();
  ಪ್ರತ್ಯಾಗಮ std::error_code();
}

ಶೂನ್ಯ createUniquePath(ಧ್ರುವ Twine &Model, SmallVectorImpl<ವರ್ಣ> &ResultPath,
                      ತಾರ್ಕಿಕ MakeAbsolute) {
  SmallString<128> ModelStorage;
  Model.toVector(ModelStorage);

  ಪ್ರತಿಜ್ಞಾ(llvm::is_contained(ModelStorage, '%') &&
         "createUniquePath: Model must contain at least one '%'");

  ಯದಿ (MakeAbsolute) {
    // Make model absolute by prepending a temp directory if it's not already.
    ಯದಿ (!sys::path::is_absolute(Twine(ModelStorage))) {
      SmallString<128> TDir;
      sys::path::system_temp_directory(ಸತ್ಯ, TDir);
      sys::path::append(TDir, Twine(ModelStorage));
      ModelStorage.swap(TDir);
    }
  }

  ResultPath = ModelStorage;
  ResultPath.ತಳ್ಳು_back(0);
  ResultPath.ತೆಗೆ_back();

  // Replace '%' with random chars.
  ಪ್ರತಿ (unsigned i = 0, e = ModelStorage.size(); i != e; ++i) {
    ಯದಿ (ModelStorage[i] == '%')
      ResultPath[i] = "0123456789abcdef"[sys::Process::Get_Random_ಸಂಖ್ಯೆ() & 15];
  }
}

std::error_code createUniqueFile(ಧ್ರುವ Twine &Model, ಪೂರ್ಣಾಙ್ಕ &ResultFd,
                                 SmallVectorImpl<ವರ್ಣ> &ResultPath,
                                 OpenFlags Flags, unsigned Mode) {
  ಪ್ರತ್ಯಾಗಮ createUniqueEntity(Model, ResultFd, ResultPath, ಅಸತ್ಯ, FS_File, Flags,
                            Mode);
}

std::error_code createUniqueFile(ಧ್ರುವ Twine &Model,
                                 SmallVectorImpl<ವರ್ಣ> &ResultPath,
                                 unsigned Mode) {
  ಪೂರ್ಣಾಙ್ಕ FD;
  auto EC = createUniqueFile(Model, FD, ResultPath, OF_None, Mode);
  ಯದಿ (EC)
    ಪ್ರತ್ಯಾಗಮ EC;
  // FD is only needed to avoid race conditions. Close it right away.
  close(FD);
  ಪ್ರತ್ಯಾಗಮ EC;
}

ಸ್ಥಿರ std::error_code
createTemporaryFile(ಧ್ರುವ Twine &Model, ಪೂರ್ಣಾಙ್ಕ &ResultFD,
                    llvm::SmallVectorImpl<ವರ್ಣ> &ResultPath, FSEntity Type,
                    sys::fs::OpenFlags Flags = sys::fs::OF_None) {
  // Any *temporary* file is assumed to be a compiler-internal output, not
  // a formal one.
  auto BypassSandbox = sys::sandbox::scopedDisable();

  SmallString<128> Storage;
  StringRef P = Model.toNullTerminatedStringRef(Storage);
  ಪ್ರತಿಜ್ಞಾ(P.find_first_of(separators(Style::native)) == StringRef::npos &&
         "Model must be a simple filename.");
  // Use P.begin() so that createUniqueEntity doesn't need to recreate Storage.
  ಪ್ರತ್ಯಾಗಮ createUniqueEntity(P.begin(), ResultFD, ResultPath, ಸತ್ಯ, Type, Flags,
                            all_read | all_write);
}

ಸ್ಥಿರ std::error_code
createTemporaryFile(ಧ್ರುವ Twine &Prefix, StringRef Suffix, ಪೂರ್ಣಾಙ್ಕ &ResultFD,
                    llvm::SmallVectorImpl<ವರ್ಣ> &ResultPath, FSEntity Type,
                    sys::fs::OpenFlags Flags = sys::fs::OF_None) {
  ಧ್ರುವ ವರ್ಣ *ಮಧ್ಯ = Suffix.empty() ? "-%%%%%%" : "-%%%%%%.";
  ಪ್ರತ್ಯಾಗಮ createTemporaryFile(Prefix + ಮಧ್ಯ + Suffix, ResultFD, ResultPath,
                             Type, Flags);
}

std::error_code createTemporaryFile(ಧ್ರುವ Twine &Prefix, StringRef Suffix,
                                    ಪೂರ್ಣಾಙ್ಕ &ResultFD,
                                    SmallVectorImpl<ವರ್ಣ> &ResultPath,
                                    sys::fs::OpenFlags Flags) {
  ಪ್ರತ್ಯಾಗಮ createTemporaryFile(Prefix, Suffix, ResultFD, ResultPath, FS_File,
                             Flags);
}

std::error_code createTemporaryFile(ಧ್ರುವ Twine &Prefix, StringRef Suffix,
                                    SmallVectorImpl<ವರ್ಣ> &ResultPath,
                                    sys::fs::OpenFlags Flags) {
  ಪೂರ್ಣಾಙ್ಕ FD;
  auto EC = createTemporaryFile(Prefix, Suffix, FD, ResultPath, Flags);
  ಯದಿ (EC)
    ಪ್ರತ್ಯಾಗಮ EC;
  // FD is only needed to avoid race conditions. Close it right away.
  close(FD);
  ಪ್ರತ್ಯಾಗಮ EC;
}

// This is a mkdtemp with a different pattern. We use createUniqueEntity mostly
// for consistency. We should try using mkdtemp.
std::error_code createUniqueDirectory(ಧ್ರುವ Twine &Prefix,
                                      SmallVectorImpl<ವರ್ಣ> &ResultPath) {
  ಪೂರ್ಣಾಙ್ಕ Dummy;
  ಪ್ರತ್ಯಾಗಮ createUniqueEntity(Prefix + "-%%%%%%", Dummy, ResultPath, ಸತ್ಯ,
                            FS_Dir);
}

std::error_code
getPotentiallyUniqueFileName(ಧ್ರುವ Twine &Model,
                             SmallVectorImpl<ವರ್ಣ> &ResultPath) {
  ಪೂರ್ಣಾಙ್ಕ Dummy;
  ಪ್ರತ್ಯಾಗಮ createUniqueEntity(Model, Dummy, ResultPath, ಅಸತ್ಯ, FS_Name);
}

std::error_code
getPotentiallyUniqueTempFileName(ಧ್ರುವ Twine &Prefix, StringRef Suffix,
                                 SmallVectorImpl<ವರ್ಣ> &ResultPath) {
  ಪೂರ್ಣಾಙ್ಕ Dummy;
  ಪ್ರತ್ಯಾಗಮ createTemporaryFile(Prefix, Suffix, Dummy, ResultPath, FS_Name);
}

std::error_code make_absolute(SmallVectorImpl<ವರ್ಣ> &path) {
  sandbox::violationIfEnabled();

  ಯದಿ (path::is_absolute(path))
    ಪ್ರತ್ಯಾಗಮ {};

  SmallString<128> current_dir;
  ಯದಿ (std::error_code ec = current_path(current_dir))
    ಪ್ರತ್ಯಾಗಮ ec;

  path::make_absolute(current_dir, path);
  ಪ್ರತ್ಯಾಗಮ {};
}

std::error_code create_directories(ಧ್ರುವ Twine &Path, ತಾರ್ಕಿಕ IgnoreExisting,
                                   perms Perms) {
  SmallString<128> PathStorage;
  StringRef P = Path.toStringRef(PathStorage);

  // Be optimistic and try to create the directory
  std::error_code EC = create_directory(P, IgnoreExisting, Perms);
  // If we succeeded, or had any error other than the parent not existing, just
  // return it.
  ಯದಿ (EC != errc::no_such_file_or_directory)
    ಪ್ರತ್ಯಾಗಮ EC;

  // We failed because of a no_such_file_or_directory, try to create the
  // parent.
  StringRef Parent = path::parent_path(P);
  ಯದಿ (Parent.empty())
    ಪ್ರತ್ಯಾಗಮ EC;

  ಯದಿ ((EC = create_directories(Parent, IgnoreExisting, Perms)))
      ಪ್ರತ್ಯಾಗಮ EC;

  ಪ್ರತ್ಯಾಗಮ create_directory(P, IgnoreExisting, Perms);
}

ಸ್ಥಿರ std::error_code copy_file_internal(ಪೂರ್ಣಾಙ್ಕ ReadFD, ಪೂರ್ಣಾಙ್ಕ WriteFD) {
  ಧ್ರುವ size_t BufSize = 4096;
  ವರ್ಣ *Buf = ನೂತನ ವರ್ಣ[BufSize];
  ಪೂರ್ಣಾಙ್ಕ BytesRead = 0, BytesWritten = 0;
  ಪ್ರತಿ (;;) {
    BytesRead = ಪಠ(ReadFD, Buf, BufSize);
    ಯದಿ (BytesRead <= 0)
      ಭಙ್ಗ;
    ಯಾವತ್ (BytesRead) {
      BytesWritten = ಲಿಖ(WriteFD, Buf, BytesRead);
      ಯದಿ (BytesWritten < 0)
        ಭಙ್ಗ;
      BytesRead -= BytesWritten;
    }
    ಯದಿ (BytesWritten < 0)
      ಭಙ್ಗ;
  }
  delete[] Buf;

  ಯದಿ (BytesRead < 0 || BytesWritten < 0)
    ಪ್ರತ್ಯಾಗಮ errnoAsErrorCode();
  ಪ್ರತ್ಯಾಗಮ std::error_code();
}

#ifndef __APPLE__
std::error_code copy_file(ಧ್ರುವ Twine &From, ಧ್ರುವ Twine &To) {
  ಪೂರ್ಣಾಙ್ಕ ReadFD, WriteFD;
  ಯದಿ (std::error_code EC = openFileForRead(From, ReadFD, OF_None))
    ಪ್ರತ್ಯಾಗಮ EC;
  ಯದಿ (std::error_code EC =
          openFileForWrite(To, WriteFD, CD_CreateAlways, OF_None)) {
    close(ReadFD);
    ಪ್ರತ್ಯಾಗಮ EC;
  }

  std::error_code EC = copy_file_internal(ReadFD, WriteFD);

  close(ReadFD);
  close(WriteFD);

  ಪ್ರತ್ಯಾಗಮ EC;
}
#endif

std::error_code copy_file(ಧ್ರುವ Twine &From, ಪೂರ್ಣಾಙ್ಕ ToFD) {
  ಪೂರ್ಣಾಙ್ಕ ReadFD;
  ಯದಿ (std::error_code EC = openFileForRead(From, ReadFD, OF_None))
    ಪ್ರತ್ಯಾಗಮ EC;

  std::error_code EC = copy_file_internal(ReadFD, ToFD);

  close(ReadFD);

  ಪ್ರತ್ಯಾಗಮ EC;
}

ErrorOr<MD5::MD5Result> md5_contents(ಪೂರ್ಣಾಙ್ಕ FD) {
  sandbox::violationIfEnabled();

  MD5 Hash;

  constexpr size_t BufSize = 4096;
  std::vector<uint8_t> Buf(BufSize);
  ಪೂರ್ಣಾಙ್ಕ BytesRead = 0;
  ಪ್ರತಿ (;;) {
    BytesRead = ಪಠ(FD, Buf.data(), BufSize);
    ಯದಿ (BytesRead <= 0)
      ಭಙ್ಗ;
    Hash.update(ArrayRef(Buf.data(), BytesRead));
  }

  ಯದಿ (BytesRead < 0)
    ಪ್ರತ್ಯಾಗಮ errnoAsErrorCode();
  MD5::MD5Result Result;
  Hash.final(Result);
  ಪ್ರತ್ಯಾಗಮ Result;
}

ErrorOr<MD5::MD5Result> md5_contents(ಧ್ರುವ Twine &Path) {
  sandbox::violationIfEnabled();

  ಪೂರ್ಣಾಙ್ಕ FD;
  ಯದಿ (auto EC = openFileForRead(Path, FD, OF_None))
    ಪ್ರತ್ಯಾಗಮ EC;

  auto Result = md5_contents(FD);
  close(FD);
  ಪ್ರತ್ಯಾಗಮ Result;
}

ತಾರ್ಕಿಕ exists(ಧ್ರುವ basic_file_status &status) {
  ಪ್ರತ್ಯಾಗಮ status_known(status) && status.type() != file_type::file_not_found;
}

ತಾರ್ಕಿಕ status_known(ಧ್ರುವ basic_file_status &s) {
  ಪ್ರತ್ಯಾಗಮ s.type() != file_type::status_error;
}

file_type get_file_type(ಧ್ರುವ Twine &Path, ತಾರ್ಕಿಕ Follow) {
  file_status st;
  ಯದಿ (status(Path, st, Follow))
    ಪ್ರತ್ಯಾಗಮ file_type::status_error;
  ಪ್ರತ್ಯಾಗಮ st.type();
}

ತಾರ್ಕಿಕ is_directory(ಧ್ರುವ basic_file_status &status) {
  ಪ್ರತ್ಯಾಗಮ status.type() == file_type::directory_file;
}

std::error_code is_directory(ಧ್ರುವ Twine &path, ತಾರ್ಕಿಕ &result) {
  sandbox::violationIfEnabled();

  file_status st;
  ಯದಿ (std::error_code ec = status(path, st))
    ಪ್ರತ್ಯಾಗಮ ec;
  result = is_directory(st);
  ಪ್ರತ್ಯಾಗಮ std::error_code();
}

ತಾರ್ಕಿಕ is_regular_file(ಧ್ರುವ basic_file_status &status) {
  ಪ್ರತ್ಯಾಗಮ status.type() == file_type::regular_file;
}

std::error_code is_regular_file(ಧ್ರುವ Twine &path, ತಾರ್ಕಿಕ &result) {
  sandbox::violationIfEnabled();

  file_status st;
  ಯದಿ (std::error_code ec = status(path, st))
    ಪ್ರತ್ಯಾಗಮ ec;
  result = is_regular_file(st);
  ಪ್ರತ್ಯಾಗಮ std::error_code();
}

ತಾರ್ಕಿಕ is_symlink_file(ಧ್ರುವ basic_file_status &status) {
  ಪ್ರತ್ಯಾಗಮ status.type() == file_type::symlink_file;
}

std::error_code is_symlink_file(ಧ್ರುವ Twine &path, ತಾರ್ಕಿಕ &result) {
  sandbox::violationIfEnabled();

  file_status st;
  ಯದಿ (std::error_code ec = status(path, st, ಅಸತ್ಯ))
    ಪ್ರತ್ಯಾಗಮ ec;
  result = is_symlink_file(st);
  ಪ್ರತ್ಯಾಗಮ std::error_code();
}

ತಾರ್ಕಿಕ is_other(ಧ್ರುವ basic_file_status &status) {
  ಪ್ರತ್ಯಾಗಮ exists(status) &&
         !is_regular_file(status) &&
         !is_directory(status);
}

std::error_code is_other(ಧ್ರುವ Twine &Path, ತಾರ್ಕಿಕ &Result) {
  sandbox::violationIfEnabled();

  file_status FileStatus;
  ಯದಿ (std::error_code EC = status(Path, FileStatus))
    ಪ್ರತ್ಯಾಗಮ EC;
  Result = is_other(FileStatus);
  ಪ್ರತ್ಯಾಗಮ std::error_code();
}

ಶೂನ್ಯ directory_entry::replace_filename(ಧ್ರುವ Twine &Filename, file_type Type,
                                       basic_file_status Status) {
  SmallString<128> PathStr = path::parent_path(Path);
  path::append(PathStr, Filename);
  ಏತತ್->Path = std::string(PathStr);
  ಏತತ್->Type = Type;
  ಏತತ್->Status = Status;
}

ErrorOr<perms> getPermissions(ಧ್ರುವ Twine &Path) {
  sandbox::violationIfEnabled();

  file_status Status;
  ಯದಿ (std::error_code EC = status(Path, Status))
    ಪ್ರತ್ಯಾಗಮ EC;

  ಪ್ರತ್ಯಾಗಮ Status.permissions();
}

std::error_code setLastAccessAndModificationTime(ಧ್ರುವ Twine &Path,
                                                 TimePoint<> AccessTime,
                                                 TimePoint<> ModificationTime) {
  ಪೂರ್ಣಾಙ್ಕ FD;
  ಯದಿ (std::error_code EC = openFile(Path, FD, CD_OpenExisting, FA_Read,
                                    OF_UpdateAttributes | OF_OpenDirectory))
    ಪ್ರತ್ಯಾಗಮ EC;
  std::error_code EC =
      setLastAccessAndModificationTime(FD, AccessTime, ModificationTime);
  Process::SafelyCloseFileDescriptor(FD);
  ಪ್ರತ್ಯಾಗಮ EC;
}

size_t mapped_file_region::size() ಧ್ರುವ {
  ಪ್ರತಿಜ್ಞಾ(Mapping && "Mapping failed but used anyway!");
  ಪ್ರತ್ಯಾಗಮ Size;
}

ವರ್ಣ *mapped_file_region::data() ಧ್ರುವ {
  ಪ್ರತಿಜ್ಞಾ(Mapping && "Mapping failed but used anyway!");
  ಪ್ರತ್ಯಾಗಮ reinterpret_cast<ವರ್ಣ *>(Mapping);
}

ಧ್ರುವ ವರ್ಣ *mapped_file_region::const_data() ಧ್ರುವ {
  ಪ್ರತಿಜ್ಞಾ(Mapping && "Mapping failed but used anyway!");
  ಪ್ರತ್ಯಾಗಮ reinterpret_cast<ಧ್ರುವ ವರ್ಣ *>(Mapping);
}

Error readNativeFileToEOF(file_t FileHandle, SmallVectorImpl<ವರ್ಣ> &Buffer,
                          ssize_t ChunkSize) {
  sandbox::violationIfEnabled();

  // Install a handler to truncate the buffer to the correct size on exit.
  size_t Size = Buffer.size();
  llvm::scope_exit TruncateOnExit([&]() { Buffer.truncate(Size); });

  // Read into Buffer until we hit EOF.
  ಪ್ರತಿ (;;) {
    Buffer.resize_for_overwrite(Size + ChunkSize);
    Expected<size_t> ReadBytes = readNativeFile(
        FileHandle, MutableArrayRef(Buffer.begin() + Size, ChunkSize));
    ಯದಿ (!ReadBytes)
      ಪ್ರತ್ಯಾಗಮ ReadBytes.takeError();
    ಯದಿ (*ReadBytes == 0)
      ಪ್ರತ್ಯಾಗಮ Error::success();
    Size += *ReadBytes;
  }
}

} // end namespace fs
} // end namespace sys
} // end namespace llvm

// Include the truly platform-specific parts.
#ಯದಿ defined(LLVM_ON_UNIX)
#ಅನ್ತರ್ಭಾವ "Unix/Path.inc"
#endif
#ಯದಿ defined(_WIN32)
#ಅನ್ತರ್ಭಾವ "Windows/Path.inc"
#endif

ನಾಮಾಕಾಶ llvm {
ನಾಮಾಕಾಶ sys {
ನಾಮಾಕಾಶ fs {

TempFile::TempFile(StringRef Name, ಪೂರ್ಣಾಙ್ಕ FD)
    : TmpName(std::string(Name)), FD(FD) {}
TempFile::TempFile(TempFile &&Other) { *ಏತತ್ = std::move(Other); }
TempFile &TempFile::operator=(TempFile &&Other) {
  TmpName = std::move(Other.TmpName);
  FD = Other.FD;
  Other.Done = ಸತ್ಯ;
  Other.FD = -1;
#ifdef _WIN32
  RemoveOnClose = Other.RemoveOnClose;
  Other.RemoveOnClose = ಅಸತ್ಯ;
#endif
  ಪ್ರತ್ಯಾಗಮ *ಏತತ್;
}

TempFile::~TempFile() { ಪ್ರತಿಜ್ಞಾ(Done); }

Error TempFile::discard() {
  Done = ಸತ್ಯ;
  ಯದಿ (FD != -1 && close(FD) == -1) {
    std::error_code EC = errnoAsErrorCode();
    ಪ್ರತ್ಯಾಗಮ errorCodeToError(EC);
  }
  FD = -1;

#ifdef _WIN32
  // On Windows, closing will remove the file, if we set the delete
  // disposition. If not, remove it manually.
  ತಾರ್ಕಿಕ Remove = RemoveOnClose;
#ಅನ್ಯಥಾ
  // Always try to remove the file.
  ತಾರ್ಕಿಕ Remove = ಸತ್ಯ;
#endif
  std::error_code RemoveEC;
  ಯದಿ (Remove && !TmpName.empty()) {
    RemoveEC = fs::remove(TmpName);
    sys::DontRemoveFileOnSignal(TmpName);
    ಯದಿ (!RemoveEC)
      TmpName = "";
  } ಅನ್ಯಥಾ {
    TmpName = "";
  }
  ಪ್ರತ್ಯಾಗಮ errorCodeToError(RemoveEC);
}

Error TempFile::keep(ಧ್ರುವ Twine &Name) {
  ಪ್ರತಿಜ್ಞಾ(!Done);
  Done = ಸತ್ಯ;
  // Always try to close and rename.
#ifdef _WIN32
  // If we can't cancel the delete don't rename.
  auto H = reinterpret_cast<HANDLE>(_get_osfhandle(FD));
  std::error_code RenameEC =
      RemoveOnClose ? std::error_code() : setDeleteDisposition(H, ಅಸತ್ಯ);
  ತಾರ್ಕಿಕ ShouldDelete = ಅಸತ್ಯ;
  ಯದಿ (!RenameEC) {
    RenameEC = rename_handle(H, Name);
    // If rename failed because it's cross-device, copy instead
    ಯದಿ (RenameEC ==
      std::error_code(ERROR_NOT_SAME_DEVICE, std::system_category())) {
      RenameEC = copy_file(TmpName, Name);
      ShouldDelete = ಸತ್ಯ;
    }
  }

  // If we can't rename or copy, discard the temporary file.
  ಯದಿ (RenameEC)
    ShouldDelete = ಸತ್ಯ;
  ಯದಿ (ShouldDelete) {
    ಯದಿ (!RemoveOnClose)
      setDeleteDisposition(H, ಸತ್ಯ);
    ಅನ್ಯಥಾ
      remove(TmpName);
  }
#ಅನ್ಯಥಾ
  std::error_code RenameEC = fs::rename(TmpName, Name);
  ಯದಿ (RenameEC) {
    // If we can't rename, try to copy to work around cross-device link issues.
    RenameEC = sys::fs::copy_file(TmpName, Name);
    // If we can't rename or copy, discard the temporary file.
    ಯದಿ (RenameEC)
      remove(TmpName);
  }
#endif
  sys::DontRemoveFileOnSignal(TmpName);

  ಯದಿ (!RenameEC)
    TmpName = "";

  ಯದಿ (close(FD) == -1)
    ಪ್ರತ್ಯಾಗಮ errorCodeToError(errnoAsErrorCode());
  FD = -1;

  ಪ್ರತ್ಯಾಗಮ errorCodeToError(RenameEC);
}

Error TempFile::keep() {
  ಪ್ರತಿಜ್ಞಾ(!Done);
  Done = ಸತ್ಯ;

#ifdef _WIN32
  auto H = reinterpret_cast<HANDLE>(_get_osfhandle(FD));
  ಯದಿ (std::error_code EC = setDeleteDisposition(H, ಅಸತ್ಯ))
    ಪ್ರತ್ಯಾಗಮ errorCodeToError(EC);
#endif
  sys::DontRemoveFileOnSignal(TmpName);

  TmpName = "";

  ಯದಿ (close(FD) == -1)
    ಪ್ರತ್ಯಾಗಮ errorCodeToError(errnoAsErrorCode());
  FD = -1;

  ಪ್ರತ್ಯಾಗಮ Error::success();
}

Expected<TempFile> TempFile::create(ಧ್ರುವ Twine &Model, unsigned Mode,
                                    OpenFlags ExtraFlags) {
  ಪೂರ್ಣಾಙ್ಕ FD;
  SmallString<128> ResultPath;
  ಯದಿ (std::error_code EC =
          createUniqueFile(Model, FD, ResultPath, OF_Delete | ExtraFlags, Mode))
    ಪ್ರತ್ಯಾಗಮ errorCodeToError(EC);

  TempFile Ret(ResultPath, FD);
#ifdef _WIN32
  auto H = reinterpret_cast<HANDLE>(_get_osfhandle(FD));
  ತಾರ್ಕಿಕ SetSignalHandler = ಅಸತ್ಯ;
  ಯದಿ (std::error_code EC = setDeleteDisposition(H, ಸತ್ಯ)) {
    Ret.RemoveOnClose = ಸತ್ಯ;
    SetSignalHandler = ಸತ್ಯ;
  }
#ಅನ್ಯಥಾ
  ತಾರ್ಕಿಕ SetSignalHandler = ಸತ್ಯ;
#endif
  ಯದಿ (SetSignalHandler && sys::RemoveFileOnSignal(ResultPath)) {
    // Make sure we delete the file when RemoveFileOnSignal fails.
    consumeError(Ret.discard());
    std::error_code EC(errc::operation_not_permitted);
    ಪ್ರತ್ಯಾಗಮ errorCodeToError(EC);
  }
  ಪ್ರತ್ಯಾಗಮ std::move(Ret);
}
} // namespace fs

} // namespace sys
} // namespace llvm
