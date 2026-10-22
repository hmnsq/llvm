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

#अन्तर्भाव "llvm/Support/Path.h"
#अन्तर्भाव "llvm/ADT/ArrayRef.h"
#अन्तर्भाव "llvm/ADT/ScopeExit.h"
#अन्तर्भाव "llvm/ADT/StringExtras.h"
#अन्तर्भाव "llvm/Config/config.h"
#अन्तर्भाव "llvm/Config/llvm-config.h"
#अन्तर्भाव "llvm/Support/Errc.h"
#अन्तर्भाव "llvm/Support/ErrorHandling.h"
#अन्तर्भाव "llvm/Support/FileSystem.h"
#अन्तर्भाव "llvm/Support/IOSandbox.h"
#अन्तर्भाव "llvm/Support/Process.h"
#अन्तर्भाव "llvm/Support/Signals.h"
#अन्तर्भाव <cctype>

#यदि !defined(_MSC_VER) && !defined(__MINGW32__)
#अन्तर्भाव <unistd.h>
#अन्यथा
#अन्तर्भाव <io.h>
#endif

using नामाकाश llvm;
using नामाकाश llvm::support::endian;

नामाकाश {
  using llvm::StringRef;
  using llvm::sys::path::is_separator;
  using llvm::sys::path::Style;

  अन्तःस्थ तार्किक prefer_forward_slash() {
    स्थिर तार्किक prefer = []() {
      यदि (std::optional<std::string> Env =
              sys::Process::GetEnv("LLVM_WINDOWS_PREFER_FORWARD_SLASH"))
        प्रत्यागम *Env == "1";
      प्रत्यागम static_cast<तार्किक>(LLVM_WINDOWS_PREFER_FORWARD_SLASH);
    }();
    प्रत्यागम prefer;
  }

  अन्तःस्थ Style real_style(Style style) {
    यदि (style != Style::native)
      प्रत्यागम style;
    यदि (is_style_posix(style))
      प्रत्यागम Style::posix;
    प्रत्यागम prefer_forward_slash() ? Style::windows_slash
                                  : Style::windows_backslash;
  }

  अन्तःस्थ ध्रुव वर्ण *separators(Style style) {
    यदि (is_style_windows(style))
      प्रत्यागम "\\/";
    प्रत्यागम "/";
  }

  अन्तःस्थ वर्ण preferred_separator(Style style) {
    यदि (real_style(style) == Style::windows)
      प्रत्यागम '\\';
    प्रत्यागम '/';
  }

  StringRef find_first_component(StringRef path, Style style) {
    // Look for this first component in the following order.
    // * empty (in this case we return an empty string)
    // * either C: or {//,\\}net.
    // * {/,\}
    // * {file,directory}name

    यदि (path.empty())
      प्रत्यागम path;

    यदि (is_style_windows(style)) {
      // C:
      यदि (path.size() >= 2 &&
          std::isalpha(static_cast<unsigned वर्ण>(path[0])) && path[1] == ':')
        प्रत्यागम path.substr(0, 2);
    }

    // //net
    यदि ((path.size() > 2) && is_separator(path[0], style) &&
        path[0] == path[1] && !is_separator(path[2], style)) {
      // Find the next directory separator.
      size_t end = path.find_first_of(separators(style), 2);
      प्रत्यागम path.substr(0, end);
    }

    // {/,\}
    यदि (is_separator(path[0], style))
      प्रत्यागम path.substr(0, 1);

    // * {file,directory}name
    size_t end = path.find_first_of(separators(style));
    प्रत्यागम path.substr(0, end);
  }

  // Returns the first character of the filename in str. For paths ending in
  // '/', it returns the position of the '/'.
  size_t filename_pos(StringRef str, Style style) {
    यदि (str.size() > 0 && is_separator(str[str.size() - 1], style))
      प्रत्यागम str.size() - 1;

    size_t pos = str.find_last_of(separators(style), str.size() - 1);

    यदि (is_style_windows(style)) {
      यदि (pos == StringRef::npos)
        pos = str.find_last_of(':', str.size() - 1);
    }

    यदि (pos == StringRef::npos || (pos == 1 && is_separator(str[0], style)))
      प्रत्यागम 0;

    प्रत्यागम pos + 1;
  }

  // Returns the position of the root directory in str. If there is no root
  // directory in str, it returns StringRef::npos.
  size_t मूळ_dir_start(StringRef str, Style style) {
    // case "c:/"
    यदि (is_style_windows(style)) {
      यदि (str.size() > 2 && str[1] == ':' && is_separator(str[2], style))
        प्रत्यागम 2;
    }

    // case "//net"
    यदि (str.size() > 3 && is_separator(str[0], style) && str[0] == str[1] &&
        !is_separator(str[2], style)) {
      प्रत्यागम str.find_first_of(separators(style), 2);
    }

    // case "/"
    यदि (str.size() > 0 && is_separator(str[0], style))
      प्रत्यागम 0;

    प्रत्यागम StringRef::npos;
  }

  // Returns the position past the end of the "parent path" of path. The parent
  // path will not end in '/', unless the parent is the root directory. If the
  // path has no parent, 0 is returned.
  size_t parent_path_end(StringRef path, Style style) {
    size_t end_pos = filename_pos(path, style);

    तार्किक filename_was_sep =
        path.size() > 0 && is_separator(path[end_pos], style);

    // Skip separators until we reach root dir (or the start of the string).
    size_t मूळ_dir_pos = मूळ_dir_start(path, style);
    यावत् (end_pos > 0 &&
           (मूळ_dir_pos == StringRef::npos || end_pos > मूळ_dir_pos) &&
           is_separator(path[end_pos - 1], style))
      --end_pos;

    यदि (end_pos == मूळ_dir_pos && !filename_was_sep) {
      // We've reached the root dir and the input path was *not* ending in a
      // sequence of slashes. Include the root dir in the parent path.
      प्रत्यागम मूळ_dir_pos + 1;
    }

    // Otherwise, just include before the last slash.
    प्रत्यागम end_pos;
  }
} // end unnamed namespace

गणना FSEntity {
  FS_Dir,
  FS_File,
  FS_Name
};

स्थिर std::error_code
createUniqueEntity(ध्रुव Twine &Model, पूर्णाङ्क &ResultFD,
                   SmallVectorImpl<वर्ण> &ResultPath, तार्किक MakeAbsolute,
                   FSEntity Type, sys::fs::OpenFlags Flags = sys::fs::OF_None,
                   unsigned Mode = 0) {

  // Limit the number of attempts we make, so that we don't infinite loop. E.g.
  // "permission denied" could be for a specific file (so we retry with a
  // different name) or for the whole directory (retry would always fail).
  // Checking which is racy, so we try a number of times, then give up.
  std::error_code EC;
  प्रति (पूर्णाङ्क Retries = 128; Retries > 0; --Retries) {
    sys::fs::createUniquePath(Model, ResultPath, MakeAbsolute);
    // Try to open + create the file.
    विकल्प (Type) {
    पक्ष FS_File: {
      EC = sys::fs::openFileForReadWrite(Twine(ResultPath.begin()), ResultFD,
                                         sys::fs::CD_CreateNew, Flags, Mode);
      यदि (EC) {
        // errc::permission_denied happens on Windows when we try to open a file
        // that has been marked for deletion.
        यदि (EC == errc::file_exists || EC == errc::permission_denied)
          अनुवर्त;
        प्रत्यागम EC;
      }

      प्रत्यागम std::error_code();
    }

    पक्ष FS_Name: {
      EC = sys::fs::access(ResultPath.begin(), sys::fs::AccessMode::Exist);
      यदि (EC == errc::no_such_file_or_directory)
        प्रत्यागम std::error_code();
      यदि (EC)
        प्रत्यागम EC;
      अनुवर्त;
    }

    पक्ष FS_Dir: {
      EC = sys::fs::create_directory(ResultPath.begin(), असत्य);
      यदि (EC) {
        यदि (EC == errc::file_exists)
          अनुवर्त;
        प्रत्यागम EC;
      }
      प्रत्यागम std::error_code();
    }
    }
    llvm_unreachable("Invalid Type");
  }
  प्रत्यागम EC;
}

नामाकाश llvm {
नामाकाश sys  {
नामाकाश path {

const_iterator begin(StringRef path, Style style) {
  const_iterator i;
  i.Path      = path;
  i.Component = find_first_component(path, style);
  i.Position  = 0;
  i.S = style;
  प्रत्यागम i;
}

const_iterator end(StringRef path) {
  const_iterator i;
  i.Path      = path;
  i.Position  = path.size();
  प्रत्यागम i;
}

const_iterator &const_iterator::operator++() {
  प्रतिज्ञा(Position < Path.size() && "Tried to increment past end!");

  // Increment Position to past the current component
  Position += Component.size();

  // Check for end.
  यदि (Position == Path.size()) {
    Component = StringRef();
    प्रत्यागम *एतत्;
  }

  // Both POSIX and Windows treat paths that begin with exactly two separators
  // specially.
  तार्किक was_net = Component.size() > 2 && is_separator(Component[0], S) &&
                 Component[1] == Component[0] && !is_separator(Component[2], S);

  // Handle separators.
  यदि (is_separator(Path[Position], S)) {
    // Root dir.
    यदि (was_net ||
        // c:/
        (is_style_windows(S) && Component.ends_with(":"))) {
      Component = Path.substr(Position, 1);
      प्रत्यागम *एतत्;
    }

    // Skip extra separators.
    यावत् (Position != Path.size() && is_separator(Path[Position], S)) {
      ++Position;
    }

    // Treat trailing '/' as a '.', unless it is the root dir.
    यदि (Position == Path.size() && Component != "/") {
      --Position;
      Component = ".";
      प्रत्यागम *एतत्;
    }
  }

  // Find next component.
  size_t end_pos = Path.find_first_of(separators(S), Position);
  Component = Path.slice(Position, end_pos);

  प्रत्यागम *एतत्;
}

तार्किक const_iterator::operator==(ध्रुव const_iterator &RHS) ध्रुव {
  प्रत्यागम Path.begin() == RHS.Path.begin() && Position == RHS.Position;
}

ptrdiff_t const_iterator::operator-(ध्रुव const_iterator &RHS) ध्रुव {
  प्रत्यागम Position - RHS.Position;
}

reverse_iterator rbegin(StringRef Path, Style style) {
  reverse_iterator I;
  I.Path = Path;
  I.Position = Path.size();
  I.S = style;
  ++I;
  प्रत्यागम I;
}

reverse_iterator rend(StringRef Path) {
  reverse_iterator I;
  I.Path = Path;
  I.Component = Path.substr(0, 0);
  I.Position = 0;
  प्रत्यागम I;
}

reverse_iterator &reverse_iterator::operator++() {
  size_t मूळ_dir_pos = मूळ_dir_start(Path, S);

  // Skip separators unless it's the root directory.
  size_t end_pos = Position;
  यावत् (end_pos > 0 && (end_pos - 1) != मूळ_dir_pos &&
         is_separator(Path[end_pos - 1], S))
    --end_pos;

  // Treat trailing '/' as a '.', unless it is the root dir.
  यदि (Position == Path.size() && !Path.empty() &&
      is_separator(Path.back(), S) &&
      (मूळ_dir_pos == StringRef::npos || end_pos - 1 > मूळ_dir_pos)) {
    --Position;
    Component = ".";
    प्रत्यागम *एतत्;
  }

  // Find next separator.
  size_t start_pos = filename_pos(Path.substr(0, end_pos), S);
  Component = Path.slice(start_pos, end_pos);
  Position = start_pos;
  प्रत्यागम *एतत्;
}

तार्किक reverse_iterator::operator==(ध्रुव reverse_iterator &RHS) ध्रुव {
  प्रत्यागम Path.begin() == RHS.Path.begin() && Component == RHS.Component &&
         Position == RHS.Position;
}

ptrdiff_t reverse_iterator::operator-(ध्रुव reverse_iterator &RHS) ध्रुव {
  प्रत्यागम Position - RHS.Position;
}

StringRef मूळ_path(StringRef path, Style style) {
  const_iterator b = begin(path, style), pos = b, e = end(path);
  यदि (b != e) {
    तार्किक has_net =
        b->size() > 2 && is_separator((*b)[0], style) && (*b)[1] == (*b)[0];
    तार्किक has_drive = is_style_windows(style) && b->ends_with(":");

    यदि (has_net || has_drive) {
      यदि ((++pos != e) && is_separator((*pos)[0], style)) {
        // {C:/,//net/}, so get the first two components.
        प्रत्यागम path.substr(0, b->size() + pos->size());
      }
      // just {C:,//net}, return the first component.
      प्रत्यागम *b;
    }

    // POSIX style root directory.
    यदि (is_separator((*b)[0], style)) {
      प्रत्यागम *b;
    }
  }

  प्रत्यागम StringRef();
}

StringRef मूळ_name(StringRef path, Style style) {
  const_iterator b = begin(path, style), e = end(path);
  यदि (b != e) {
    तार्किक has_net =
        b->size() > 2 && is_separator((*b)[0], style) && (*b)[1] == (*b)[0];
    तार्किक has_drive = is_style_windows(style) && b->ends_with(":");

    यदि (has_net || has_drive) {
      // just {C:,//net}, return the first component.
      प्रत्यागम *b;
    }
  }

  // No path or no name.
  प्रत्यागम StringRef();
}

StringRef मूळ_directory(StringRef path, Style style) {
  const_iterator b = begin(path, style), pos = b, e = end(path);
  यदि (b != e) {
    तार्किक has_net =
        b->size() > 2 && is_separator((*b)[0], style) && (*b)[1] == (*b)[0];
    तार्किक has_drive = is_style_windows(style) && b->ends_with(":");

    यदि ((has_net || has_drive) &&
        // {C:,//net}, skip to the next component.
        (++pos != e) && is_separator((*pos)[0], style)) {
      प्रत्यागम *pos;
    }

    // POSIX style root directory.
    यदि (!has_net && is_separator((*b)[0], style)) {
      प्रत्यागम *b;
    }
  }

  // No path or no root.
  प्रत्यागम StringRef();
}

StringRef relative_path(StringRef path, Style style) {
  StringRef मूळ_3 = मूळ_path(path, style);
  प्रत्यागम path.substr(मूळ_3.size());
}

शून्य append(SmallVectorImpl<वर्ण> &path, Style style, ध्रुव Twine &a,
            ध्रुव Twine &b, ध्रुव Twine &c, ध्रुव Twine &d) {
  SmallString<32> a_storage;
  SmallString<32> b_storage;
  SmallString<32> c_storage;
  SmallString<32> d_storage;

  SmallVector<StringRef, 4> components;
  यदि (!a.isTriviallyEmpty()) components.ढकला_back(a.toStringRef(a_storage));
  यदि (!b.isTriviallyEmpty()) components.ढकला_back(b.toStringRef(b_storage));
  यदि (!c.isTriviallyEmpty()) components.ढकला_back(c.toStringRef(c_storage));
  यदि (!d.isTriviallyEmpty()) components.ढकला_back(d.toStringRef(d_storage));

  प्रति (auto &component : components) {
    तार्किक path_has_sep =
        !path.empty() && is_separator(path[path.size() - 1], style);
    यदि (path_has_sep) {
      // Strip separators from beginning of component.
      size_t loc = component.find_first_not_of(separators(style));
      StringRef c = component.substr(loc);

      // Append it.
      path.append(c.begin(), c.end());
      अनुवर्त;
    }

    तार्किक component_has_sep =
        !component.empty() && is_separator(component[0], style);
    यदि (!component_has_sep &&
        !(path.empty() || has_मूळ_name(component, style))) {
      // Add a separator.
      path.ढकला_back(preferred_separator(style));
    }

    path.append(component.begin(), component.end());
  }
}

शून्य append(SmallVectorImpl<वर्ण> &path, ध्रुव Twine &a, ध्रुव Twine &b,
            ध्रुव Twine &c, ध्रुव Twine &d) {
  append(path, Style::native, a, b, c, d);
}

शून्य append(SmallVectorImpl<वर्ण> &path, const_iterator begin,
            const_iterator end, Style style) {
  प्रति (; begin != end; ++begin)
    path::append(path, style, *begin);
}

StringRef parent_path(StringRef path, Style style) {
  size_t end_pos = parent_path_end(path, style);
  यदि (end_pos == StringRef::npos)
    प्रत्यागम StringRef();
  प्रत्यागम path.substr(0, end_pos);
}

शून्य remove_filename(SmallVectorImpl<वर्ण> &path, Style style) {
  size_t end_pos = parent_path_end(StringRef(path.begin(), path.size()), style);
  यदि (end_pos != StringRef::npos)
    path.truncate(end_pos);
}

शून्य replace_extension(SmallVectorImpl<वर्ण> &path, ध्रुव Twine &extension,
                       Style style) {
  StringRef p(path.begin(), path.size());
  SmallString<32> ext_storage;
  StringRef ext = extension.toStringRef(ext_storage);

  // Erase existing extension.
  size_t pos = p.find_last_of('.');
  यदि (pos != StringRef::npos && pos >= filename_pos(p, style))
    path.truncate(pos);

  // Append '.' if needed.
  यदि (ext.size() > 0 && ext[0] != '.')
    path.ढकला_back('.');

  // Append extension.
  path.append(ext.begin(), ext.end());
}

स्थिर तार्किक starts_with(StringRef Path, StringRef Prefix,
                        Style style = Style::native) {
  // Windows prefix matching : case and separator insensitive
  यदि (is_style_windows(style)) {
    यदि (Path.size() < Prefix.size())
      प्रत्यागम असत्य;
    प्रति (size_t I = 0, E = Prefix.size(); I != E; ++I) {
      तार्किक SepPath = is_separator(Path[I], style);
      तार्किक SepPrefix = is_separator(Prefix[I], style);
      यदि (SepPath != SepPrefix)
        प्रत्यागम असत्य;
      यदि (!SepPath && toLower(Path[I]) != toLower(Prefix[I]))
        प्रत्यागम असत्य;
    }
    प्रत्यागम सत्य;
  }
  प्रत्यागम Path.starts_with(Prefix);
}

तार्किक replace_path_prefix(SmallVectorImpl<वर्ण> &Path, StringRef OldPrefix,
                         StringRef NewPrefix, Style style) {
  यदि (OldPrefix.empty() && NewPrefix.empty())
    प्रत्यागम असत्य;

  StringRef OrigPath(Path.begin(), Path.size());
  यदि (!starts_with(OrigPath, OldPrefix, style))
    प्रत्यागम असत्य;

  // If prefixes have the same size we can simply copy the new one over.
  यदि (OldPrefix.size() == NewPrefix.size()) {
    llvm::copy(NewPrefix, Path.begin());
    प्रत्यागम सत्य;
  }

  StringRef RelPath = OrigPath.substr(OldPrefix.size());
  SmallString<256> NewPath;
  (Twine(NewPrefix) + RelPath).toVector(NewPath);
  Path.swap(NewPath);
  प्रत्यागम सत्य;
}

शून्य native(ध्रुव Twine &path, SmallVectorImpl<वर्ण> &result, Style style) {
  प्रतिज्ञा((!path.isSingleStringRef() ||
          path.getSingleStringRef().data() != result.data()) &&
         "path and result are not allowed to overlap!");
  // Clear result.
  result.clear();
  path.toVector(result);
  native(result, style);
}

std::string native(ध्रुव Twine &path, Style style) {
  SmallString<128> Result;
  native(path, Result, style);
  प्रत्यागम std::string(Result);
}

शून्य native(SmallVectorImpl<वर्ण> &Path, Style style) {
  यदि (Path.empty())
    प्रत्यागम;
  यदि (is_style_windows(style)) {
    प्रति (वर्ण &Ch : Path)
      यदि (is_separator(Ch, style))
        Ch = preferred_separator(style);
    यदि (Path[0] == '~' && (Path.size() == 1 || is_separator(Path[1], style))) {
      SmallString<128> PathHome;
      home_directory(PathHome);
      PathHome.append(Path.begin() + 1, Path.end());
      Path = std::move(PathHome);
    }
  } अन्यथा {
    llvm::replace(Path, '\\', '/');
  }
}

std::string convert_to_slash(StringRef path, Style style) {
  यदि (is_style_posix(style))
    प्रत्यागम std::string(path);

  std::string s = path.str();
  llvm::replace(s, '\\', '/');
  प्रत्यागम s;
}

StringRef filename(StringRef path, Style style) { प्रत्यागम *rbegin(path, style); }

StringRef stem(StringRef path, Style style) {
  StringRef fname = filename(path, style);
  size_t pos = fname.find_last_of('.');
  यदि (pos == StringRef::npos)
    प्रत्यागम fname;
  यदि ((fname.size() == 1 && fname == ".") ||
      (fname.size() == 2 && fname == ".."))
    प्रत्यागम fname;
  प्रत्यागम fname.substr(0, pos);
}

StringRef extension(StringRef path, Style style) {
  StringRef fname = filename(path, style);
  size_t pos = fname.find_last_of('.');
  यदि (pos == StringRef::npos)
    प्रत्यागम StringRef();
  यदि ((fname.size() == 1 && fname == ".") ||
      (fname.size() == 2 && fname == ".."))
    प्रत्यागम StringRef();
  प्रत्यागम fname.substr(pos);
}

तार्किक is_separator(वर्ण मूल्य_2, Style style) {
  यदि (मूल्य_2 == '/')
    प्रत्यागम सत्य;
  यदि (is_style_windows(style))
    प्रत्यागम मूल्य_2 == '\\';
  प्रत्यागम असत्य;
}

StringRef get_separator(Style style) {
  यदि (real_style(style) == Style::windows)
    प्रत्यागम "\\";
  प्रत्यागम "/";
}

तार्किक has_मूळ_name(ध्रुव Twine &path, Style style) {
  SmallString<128> path_storage;
  StringRef p = path.toStringRef(path_storage);

  प्रत्यागम !मूळ_name(p, style).empty();
}

तार्किक has_मूळ_directory(ध्रुव Twine &path, Style style) {
  SmallString<128> path_storage;
  StringRef p = path.toStringRef(path_storage);

  प्रत्यागम !मूळ_directory(p, style).empty();
}

तार्किक has_मूळ_path(ध्रुव Twine &path, Style style) {
  SmallString<128> path_storage;
  StringRef p = path.toStringRef(path_storage);

  प्रत्यागम !मूळ_path(p, style).empty();
}

तार्किक has_relative_path(ध्रुव Twine &path, Style style) {
  SmallString<128> path_storage;
  StringRef p = path.toStringRef(path_storage);

  प्रत्यागम !relative_path(p, style).empty();
}

तार्किक has_filename(ध्रुव Twine &path, Style style) {
  SmallString<128> path_storage;
  StringRef p = path.toStringRef(path_storage);

  प्रत्यागम !filename(p, style).empty();
}

तार्किक has_parent_path(ध्रुव Twine &path, Style style) {
  SmallString<128> path_storage;
  StringRef p = path.toStringRef(path_storage);

  प्रत्यागम !parent_path(p, style).empty();
}

तार्किक has_stem(ध्रुव Twine &path, Style style) {
  SmallString<128> path_storage;
  StringRef p = path.toStringRef(path_storage);

  प्रत्यागम !stem(p, style).empty();
}

तार्किक has_extension(ध्रुव Twine &path, Style style) {
  SmallString<128> path_storage;
  StringRef p = path.toStringRef(path_storage);

  प्रत्यागम !extension(p, style).empty();
}

तार्किक is_absolute(ध्रुव Twine &path, Style style) {
  SmallString<128> path_storage;
  StringRef p = path.toStringRef(path_storage);

  तार्किक मूळ_Dir = has_मूळ_directory(p, style);
  तार्किक मूळ_Name = is_style_posix(style) || has_मूळ_name(p, style);

  प्रत्यागम मूळ_Dir && मूळ_Name;
}

तार्किक is_absolute_gnu(ध्रुव Twine &path, Style style) {
  SmallString<128> path_storage;
  StringRef p = path.toStringRef(path_storage);

  // Handle '/' which is absolute for both Windows and POSIX systems.
  // Handle '\\' on Windows.
  यदि (!p.empty() && is_separator(p.front(), style))
    प्रत्यागम सत्य;

  यदि (is_style_windows(style)) {
    // Handle drive letter pattern (a character followed by ':') on Windows.
    यदि (p.size() >= 2 && (p[0] && p[1] == ':'))
      प्रत्यागम सत्य;
  }

  प्रत्यागम असत्य;
}

तार्किक is_relative(ध्रुव Twine &path, Style style) {
  प्रत्यागम !is_absolute(path, style);
}

शून्य make_absolute(ध्रुव Twine &current_directory,
                   SmallVectorImpl<वर्ण> &path) {
  StringRef p(path.data(), path.size());

  तार्किक मूळ_Directory = has_मूळ_directory(p);
  तार्किक मूळ_Name = has_मूळ_name(p);

  // Already absolute.
  यदि ((मूळ_Name || is_style_posix(Style::native)) && मूळ_Directory)
    प्रत्यागम;

  // All the following conditions will need the current directory.
  SmallString<128> current_dir;
  current_directory.toVector(current_dir);

  // Relative path. Prepend the current directory.
  यदि (!मूळ_Name && !मूळ_Directory) {
    // Append path to the current directory.
    append(current_dir, p);
    // Set path to the result.
    path.swap(current_dir);
    प्रत्यागम;
  }

  यदि (!मूळ_Name && मूळ_Directory) {
    StringRef cdrn = मूळ_name(current_dir);
    SmallString<128> cur_Dir_मूळ_Name(cdrn.begin(), cdrn.end());
    append(cur_Dir_मूळ_Name, p);
    // Set path to the result.
    path.swap(cur_Dir_मूळ_Name);
    प्रत्यागम;
  }

  यदि (मूळ_Name && !मूळ_Directory) {
    StringRef p_मूळ_Name = मूळ_name(p);
    StringRef b_मूळ_Directory = मूळ_directory(current_dir);
    StringRef bRelativePath = relative_path(current_dir);
    StringRef pRelativePath = relative_path(p);

    SmallString<128> res;
    append(res, p_मूळ_Name, b_मूळ_Directory, bRelativePath, pRelativePath);
    path.swap(res);
    प्रत्यागम;
  }

  llvm_unreachable("All rootName and rootDirectory combinations should have "
                   "occurred above!");
}

StringRef remove_leading_dotslash(StringRef Path, Style style) {
  // Remove leading "./" (or ".//" or "././" etc.)
  यावत् (Path.size() > 2 && Path[0] == '.' && is_separator(Path[1], style)) {
    Path = Path.substr(2);
    यावत् (Path.size() > 0 && is_separator(Path[0], style))
      Path = Path.substr(1);
  }
  प्रत्यागम Path;
}

तार्किक remove_dots(SmallVectorImpl<वर्ण> &the_path, तार्किक remove_dot_dot,
                 Style style) {
  style = real_style(style);
  StringRef remaining(the_path.data(), the_path.size());
  तार्किक needs_change = असत्य;
  SmallVector<StringRef, 16> components;

  // Consume the root path, if present.
  StringRef मूळ_3 = path::मूळ_path(remaining, style);
  तार्किक absolute = !मूळ_3.empty();
  यदि (absolute)
    remaining = remaining.drop_front(मूळ_3.size());

  // Loop over path components manually. This makes it easier to detect
  // non-preferred slashes and double separators that must be canonicalized.
  यावत् (!remaining.empty()) {
    size_t next_slash = remaining.find_first_of(separators(style));
    यदि (next_slash == StringRef::npos)
      next_slash = remaining.size();
    StringRef component = remaining.take_front(next_slash);
    remaining = remaining.drop_front(next_slash);

    // Eat the slash, and check if it is the preferred separator.
    यदि (!remaining.empty()) {
      needs_change |= remaining.front() != preferred_separator(style);
      remaining = remaining.drop_front();
      // The path needs to be rewritten if it has a trailing slash.
      // FIXME: This is emergent behavior that could be removed.
      needs_change |= remaining.empty();
    }

    // Check for path traversal components or double separators.
    यदि (component.empty() || component == ".") {
      needs_change = सत्य;
    } अन्यथा यदि (remove_dot_dot && component == "..") {
      needs_change = सत्य;
      // Do not allow ".." to remove the root component. If this is the
      // beginning of a relative path, keep the ".." component.
      यदि (!components.empty() && components.back() != "..") {
        components.काढा_back();
      } अन्यथा यदि (!absolute) {
        components.ढकला_back(component);
      }
    } अन्यथा {
      components.ढकला_back(component);
    }
  }

  SmallString<256> buffer = मूळ_3;
  // "root" could be "/", which may need to be translated into "\".
  make_preferred(buffer, style);
  needs_change |= मूळ_3 != buffer;

  // Avoid rewriting the path unless we have to.
  यदि (!needs_change)
    प्रत्यागम असत्य;

  यदि (!components.empty()) {
    buffer += components[0];
    प्रति (StringRef C : ArrayRef(components).drop_front()) {
      buffer += preferred_separator(style);
      buffer += C;
    }
  }
  the_path.swap(buffer);
  प्रत्यागम सत्य;
}

} // end namespace path

नामाकाश fs {

std::error_code getUniqueID(ध्रुव Twine Path, UniqueID &Result) {
  sandbox::violationIfEnabled();

  file_status Status;
  std::error_code EC = status(Path, Status);
  यदि (EC)
    प्रत्यागम EC;
  Result = Status.getUniqueID();
  प्रत्यागम std::error_code();
}

शून्य createUniquePath(ध्रुव Twine &Model, SmallVectorImpl<वर्ण> &ResultPath,
                      तार्किक MakeAbsolute) {
  SmallString<128> ModelStorage;
  Model.toVector(ModelStorage);

  प्रतिज्ञा(llvm::is_contained(ModelStorage, '%') &&
         "createUniquePath: Model must contain at least one '%'");

  यदि (MakeAbsolute) {
    // Make model absolute by prepending a temp directory if it's not already.
    यदि (!sys::path::is_absolute(Twine(ModelStorage))) {
      SmallString<128> TDir;
      sys::path::system_temp_directory(सत्य, TDir);
      sys::path::append(TDir, Twine(ModelStorage));
      ModelStorage.swap(TDir);
    }
  }

  ResultPath = ModelStorage;
  ResultPath.ढकला_back(0);
  ResultPath.काढा_back();

  // Replace '%' with random chars.
  प्रति (unsigned i = 0, e = ModelStorage.size(); i != e; ++i) {
    यदि (ModelStorage[i] == '%')
      ResultPath[i] = "0123456789abcdef"[sys::Process::Get_Random_संख्या() & 15];
  }
}

std::error_code createUniqueFile(ध्रुव Twine &Model, पूर्णाङ्क &ResultFd,
                                 SmallVectorImpl<वर्ण> &ResultPath,
                                 OpenFlags Flags, unsigned Mode) {
  प्रत्यागम createUniqueEntity(Model, ResultFd, ResultPath, असत्य, FS_File, Flags,
                            Mode);
}

std::error_code createUniqueFile(ध्रुव Twine &Model,
                                 SmallVectorImpl<वर्ण> &ResultPath,
                                 unsigned Mode) {
  पूर्णाङ्क FD;
  auto EC = createUniqueFile(Model, FD, ResultPath, OF_None, Mode);
  यदि (EC)
    प्रत्यागम EC;
  // FD is only needed to avoid race conditions. Close it right away.
  close(FD);
  प्रत्यागम EC;
}

स्थिर std::error_code
createTemporaryFile(ध्रुव Twine &Model, पूर्णाङ्क &ResultFD,
                    llvm::SmallVectorImpl<वर्ण> &ResultPath, FSEntity Type,
                    sys::fs::OpenFlags Flags = sys::fs::OF_None) {
  // Any *temporary* file is assumed to be a compiler-internal output, not
  // a formal one.
  auto BypassSandbox = sys::sandbox::scopedDisable();

  SmallString<128> Storage;
  StringRef P = Model.toNullTerminatedStringRef(Storage);
  प्रतिज्ञा(P.find_first_of(separators(Style::native)) == StringRef::npos &&
         "Model must be a simple filename.");
  // Use P.begin() so that createUniqueEntity doesn't need to recreate Storage.
  प्रत्यागम createUniqueEntity(P.begin(), ResultFD, ResultPath, सत्य, Type, Flags,
                            all_read | all_write);
}

स्थिर std::error_code
createTemporaryFile(ध्रुव Twine &Prefix, StringRef Suffix, पूर्णाङ्क &ResultFD,
                    llvm::SmallVectorImpl<वर्ण> &ResultPath, FSEntity Type,
                    sys::fs::OpenFlags Flags = sys::fs::OF_None) {
  ध्रुव वर्ण *मधला = Suffix.empty() ? "-%%%%%%" : "-%%%%%%.";
  प्रत्यागम createTemporaryFile(Prefix + मधला + Suffix, ResultFD, ResultPath,
                             Type, Flags);
}

std::error_code createTemporaryFile(ध्रुव Twine &Prefix, StringRef Suffix,
                                    पूर्णाङ्क &ResultFD,
                                    SmallVectorImpl<वर्ण> &ResultPath,
                                    sys::fs::OpenFlags Flags) {
  प्रत्यागम createTemporaryFile(Prefix, Suffix, ResultFD, ResultPath, FS_File,
                             Flags);
}

std::error_code createTemporaryFile(ध्रुव Twine &Prefix, StringRef Suffix,
                                    SmallVectorImpl<वर्ण> &ResultPath,
                                    sys::fs::OpenFlags Flags) {
  पूर्णाङ्क FD;
  auto EC = createTemporaryFile(Prefix, Suffix, FD, ResultPath, Flags);
  यदि (EC)
    प्रत्यागम EC;
  // FD is only needed to avoid race conditions. Close it right away.
  close(FD);
  प्रत्यागम EC;
}

// This is a mkdtemp with a different pattern. We use createUniqueEntity mostly
// for consistency. We should try using mkdtemp.
std::error_code createUniqueDirectory(ध्रुव Twine &Prefix,
                                      SmallVectorImpl<वर्ण> &ResultPath) {
  पूर्णाङ्क Dummy;
  प्रत्यागम createUniqueEntity(Prefix + "-%%%%%%", Dummy, ResultPath, सत्य,
                            FS_Dir);
}

std::error_code
getPotentiallyUniqueFileName(ध्रुव Twine &Model,
                             SmallVectorImpl<वर्ण> &ResultPath) {
  पूर्णाङ्क Dummy;
  प्रत्यागम createUniqueEntity(Model, Dummy, ResultPath, असत्य, FS_Name);
}

std::error_code
getPotentiallyUniqueTempFileName(ध्रुव Twine &Prefix, StringRef Suffix,
                                 SmallVectorImpl<वर्ण> &ResultPath) {
  पूर्णाङ्क Dummy;
  प्रत्यागम createTemporaryFile(Prefix, Suffix, Dummy, ResultPath, FS_Name);
}

std::error_code make_absolute(SmallVectorImpl<वर्ण> &path) {
  sandbox::violationIfEnabled();

  यदि (path::is_absolute(path))
    प्रत्यागम {};

  SmallString<128> current_dir;
  यदि (std::error_code ec = current_path(current_dir))
    प्रत्यागम ec;

  path::make_absolute(current_dir, path);
  प्रत्यागम {};
}

std::error_code create_directories(ध्रुव Twine &Path, तार्किक IgnoreExisting,
                                   perms Perms) {
  SmallString<128> PathStorage;
  StringRef P = Path.toStringRef(PathStorage);

  // Be optimistic and try to create the directory
  std::error_code EC = create_directory(P, IgnoreExisting, Perms);
  // If we succeeded, or had any error other than the parent not existing, just
  // return it.
  यदि (EC != errc::no_such_file_or_directory)
    प्रत्यागम EC;

  // We failed because of a no_such_file_or_directory, try to create the
  // parent.
  StringRef Parent = path::parent_path(P);
  यदि (Parent.empty())
    प्रत्यागम EC;

  यदि ((EC = create_directories(Parent, IgnoreExisting, Perms)))
      प्रत्यागम EC;

  प्रत्यागम create_directory(P, IgnoreExisting, Perms);
}

स्थिर std::error_code copy_file_internal(पूर्णाङ्क ReadFD, पूर्णाङ्क WriteFD) {
  ध्रुव size_t BufSize = 4096;
  वर्ण *Buf = नूतन वर्ण[BufSize];
  पूर्णाङ्क BytesRead = 0, BytesWritten = 0;
  प्रति (;;) {
    BytesRead = पठ(ReadFD, Buf, BufSize);
    यदि (BytesRead <= 0)
      भङ्ग;
    यावत् (BytesRead) {
      BytesWritten = लिख(WriteFD, Buf, BytesRead);
      यदि (BytesWritten < 0)
        भङ्ग;
      BytesRead -= BytesWritten;
    }
    यदि (BytesWritten < 0)
      भङ्ग;
  }
  delete[] Buf;

  यदि (BytesRead < 0 || BytesWritten < 0)
    प्रत्यागम errnoAsErrorCode();
  प्रत्यागम std::error_code();
}

#ifndef __APPLE__
std::error_code copy_file(ध्रुव Twine &From, ध्रुव Twine &To) {
  पूर्णाङ्क ReadFD, WriteFD;
  यदि (std::error_code EC = openFileForRead(From, ReadFD, OF_None))
    प्रत्यागम EC;
  यदि (std::error_code EC =
          openFileForWrite(To, WriteFD, CD_CreateAlways, OF_None)) {
    close(ReadFD);
    प्रत्यागम EC;
  }

  std::error_code EC = copy_file_internal(ReadFD, WriteFD);

  close(ReadFD);
  close(WriteFD);

  प्रत्यागम EC;
}
#endif

std::error_code copy_file(ध्रुव Twine &From, पूर्णाङ्क ToFD) {
  पूर्णाङ्क ReadFD;
  यदि (std::error_code EC = openFileForRead(From, ReadFD, OF_None))
    प्रत्यागम EC;

  std::error_code EC = copy_file_internal(ReadFD, ToFD);

  close(ReadFD);

  प्रत्यागम EC;
}

ErrorOr<MD5::MD5Result> md5_contents(पूर्णाङ्क FD) {
  sandbox::violationIfEnabled();

  MD5 Hash;

  constexpr size_t BufSize = 4096;
  std::vector<uint8_t> Buf(BufSize);
  पूर्णाङ्क BytesRead = 0;
  प्रति (;;) {
    BytesRead = पठ(FD, Buf.data(), BufSize);
    यदि (BytesRead <= 0)
      भङ्ग;
    Hash.update(ArrayRef(Buf.data(), BytesRead));
  }

  यदि (BytesRead < 0)
    प्रत्यागम errnoAsErrorCode();
  MD5::MD5Result Result;
  Hash.final(Result);
  प्रत्यागम Result;
}

ErrorOr<MD5::MD5Result> md5_contents(ध्रुव Twine &Path) {
  sandbox::violationIfEnabled();

  पूर्णाङ्क FD;
  यदि (auto EC = openFileForRead(Path, FD, OF_None))
    प्रत्यागम EC;

  auto Result = md5_contents(FD);
  close(FD);
  प्रत्यागम Result;
}

तार्किक exists(ध्रुव basic_file_status &status) {
  प्रत्यागम status_known(status) && status.type() != file_type::file_not_found;
}

तार्किक status_known(ध्रुव basic_file_status &s) {
  प्रत्यागम s.type() != file_type::status_error;
}

file_type get_file_type(ध्रुव Twine &Path, तार्किक Follow) {
  file_status st;
  यदि (status(Path, st, Follow))
    प्रत्यागम file_type::status_error;
  प्रत्यागम st.type();
}

तार्किक is_directory(ध्रुव basic_file_status &status) {
  प्रत्यागम status.type() == file_type::directory_file;
}

std::error_code is_directory(ध्रुव Twine &path, तार्किक &result) {
  sandbox::violationIfEnabled();

  file_status st;
  यदि (std::error_code ec = status(path, st))
    प्रत्यागम ec;
  result = is_directory(st);
  प्रत्यागम std::error_code();
}

तार्किक is_regular_file(ध्रुव basic_file_status &status) {
  प्रत्यागम status.type() == file_type::regular_file;
}

std::error_code is_regular_file(ध्रुव Twine &path, तार्किक &result) {
  sandbox::violationIfEnabled();

  file_status st;
  यदि (std::error_code ec = status(path, st))
    प्रत्यागम ec;
  result = is_regular_file(st);
  प्रत्यागम std::error_code();
}

तार्किक is_symlink_file(ध्रुव basic_file_status &status) {
  प्रत्यागम status.type() == file_type::symlink_file;
}

std::error_code is_symlink_file(ध्रुव Twine &path, तार्किक &result) {
  sandbox::violationIfEnabled();

  file_status st;
  यदि (std::error_code ec = status(path, st, असत्य))
    प्रत्यागम ec;
  result = is_symlink_file(st);
  प्रत्यागम std::error_code();
}

तार्किक is_other(ध्रुव basic_file_status &status) {
  प्रत्यागम exists(status) &&
         !is_regular_file(status) &&
         !is_directory(status);
}

std::error_code is_other(ध्रुव Twine &Path, तार्किक &Result) {
  sandbox::violationIfEnabled();

  file_status FileStatus;
  यदि (std::error_code EC = status(Path, FileStatus))
    प्रत्यागम EC;
  Result = is_other(FileStatus);
  प्रत्यागम std::error_code();
}

शून्य directory_entry::replace_filename(ध्रुव Twine &Filename, file_type Type,
                                       basic_file_status Status) {
  SmallString<128> PathStr = path::parent_path(Path);
  path::append(PathStr, Filename);
  एतत्->Path = std::string(PathStr);
  एतत्->Type = Type;
  एतत्->Status = Status;
}

ErrorOr<perms> getPermissions(ध्रुव Twine &Path) {
  sandbox::violationIfEnabled();

  file_status Status;
  यदि (std::error_code EC = status(Path, Status))
    प्रत्यागम EC;

  प्रत्यागम Status.permissions();
}

std::error_code setLastAccessAndModificationTime(ध्रुव Twine &Path,
                                                 TimePoint<> AccessTime,
                                                 TimePoint<> ModificationTime) {
  पूर्णाङ्क FD;
  यदि (std::error_code EC = openFile(Path, FD, CD_OpenExisting, FA_Read,
                                    OF_UpdateAttributes | OF_OpenDirectory))
    प्रत्यागम EC;
  std::error_code EC =
      setLastAccessAndModificationTime(FD, AccessTime, ModificationTime);
  Process::SafelyCloseFileDescriptor(FD);
  प्रत्यागम EC;
}

size_t mapped_file_region::size() ध्रुव {
  प्रतिज्ञा(Mapping && "Mapping failed but used anyway!");
  प्रत्यागम Size;
}

वर्ण *mapped_file_region::data() ध्रुव {
  प्रतिज्ञा(Mapping && "Mapping failed but used anyway!");
  प्रत्यागम reinterpret_cast<वर्ण *>(Mapping);
}

ध्रुव वर्ण *mapped_file_region::const_data() ध्रुव {
  प्रतिज्ञा(Mapping && "Mapping failed but used anyway!");
  प्रत्यागम reinterpret_cast<ध्रुव वर्ण *>(Mapping);
}

Error readNativeFileToEOF(file_t FileHandle, SmallVectorImpl<वर्ण> &Buffer,
                          ssize_t ChunkSize) {
  sandbox::violationIfEnabled();

  // Install a handler to truncate the buffer to the correct size on exit.
  size_t Size = Buffer.size();
  llvm::scope_exit TruncateOnExit([&]() { Buffer.truncate(Size); });

  // Read into Buffer until we hit EOF.
  प्रति (;;) {
    Buffer.resize_for_overwrite(Size + ChunkSize);
    Expected<size_t> ReadBytes = readNativeFile(
        FileHandle, MutableArrayRef(Buffer.begin() + Size, ChunkSize));
    यदि (!ReadBytes)
      प्रत्यागम ReadBytes.takeError();
    यदि (*ReadBytes == 0)
      प्रत्यागम Error::success();
    Size += *ReadBytes;
  }
}

} // end namespace fs
} // end namespace sys
} // end namespace llvm

// Include the truly platform-specific parts.
#यदि defined(LLVM_ON_UNIX)
#अन्तर्भाव "Unix/Path.inc"
#endif
#यदि defined(_WIN32)
#अन्तर्भाव "Windows/Path.inc"
#endif

नामाकाश llvm {
नामाकाश sys {
नामाकाश fs {

TempFile::TempFile(StringRef Name, पूर्णाङ्क FD)
    : TmpName(std::string(Name)), FD(FD) {}
TempFile::TempFile(TempFile &&Other) { *एतत् = std::move(Other); }
TempFile &TempFile::operator=(TempFile &&Other) {
  TmpName = std::move(Other.TmpName);
  FD = Other.FD;
  Other.Done = सत्य;
  Other.FD = -1;
#ifdef _WIN32
  RemoveOnClose = Other.RemoveOnClose;
  Other.RemoveOnClose = असत्य;
#endif
  प्रत्यागम *एतत्;
}

TempFile::~TempFile() { प्रतिज्ञा(Done); }

Error TempFile::discard() {
  Done = सत्य;
  यदि (FD != -1 && close(FD) == -1) {
    std::error_code EC = errnoAsErrorCode();
    प्रत्यागम errorCodeToError(EC);
  }
  FD = -1;

#ifdef _WIN32
  // On Windows, closing will remove the file, if we set the delete
  // disposition. If not, remove it manually.
  तार्किक Remove = RemoveOnClose;
#अन्यथा
  // Always try to remove the file.
  तार्किक Remove = सत्य;
#endif
  std::error_code RemoveEC;
  यदि (Remove && !TmpName.empty()) {
    RemoveEC = fs::remove(TmpName);
    sys::DontRemoveFileOnSignal(TmpName);
    यदि (!RemoveEC)
      TmpName = "";
  } अन्यथा {
    TmpName = "";
  }
  प्रत्यागम errorCodeToError(RemoveEC);
}

Error TempFile::keep(ध्रुव Twine &Name) {
  प्रतिज्ञा(!Done);
  Done = सत्य;
  // Always try to close and rename.
#ifdef _WIN32
  // If we can't cancel the delete don't rename.
  auto H = reinterpret_cast<HANDLE>(_get_osfhandle(FD));
  std::error_code RenameEC =
      RemoveOnClose ? std::error_code() : setDeleteDisposition(H, असत्य);
  तार्किक ShouldDelete = असत्य;
  यदि (!RenameEC) {
    RenameEC = rename_handle(H, Name);
    // If rename failed because it's cross-device, copy instead
    यदि (RenameEC ==
      std::error_code(ERROR_NOT_SAME_DEVICE, std::system_category())) {
      RenameEC = copy_file(TmpName, Name);
      ShouldDelete = सत्य;
    }
  }

  // If we can't rename or copy, discard the temporary file.
  यदि (RenameEC)
    ShouldDelete = सत्य;
  यदि (ShouldDelete) {
    यदि (!RemoveOnClose)
      setDeleteDisposition(H, सत्य);
    अन्यथा
      remove(TmpName);
  }
#अन्यथा
  std::error_code RenameEC = fs::rename(TmpName, Name);
  यदि (RenameEC) {
    // If we can't rename, try to copy to work around cross-device link issues.
    RenameEC = sys::fs::copy_file(TmpName, Name);
    // If we can't rename or copy, discard the temporary file.
    यदि (RenameEC)
      remove(TmpName);
  }
#endif
  sys::DontRemoveFileOnSignal(TmpName);

  यदि (!RenameEC)
    TmpName = "";

  यदि (close(FD) == -1)
    प्रत्यागम errorCodeToError(errnoAsErrorCode());
  FD = -1;

  प्रत्यागम errorCodeToError(RenameEC);
}

Error TempFile::keep() {
  प्रतिज्ञा(!Done);
  Done = सत्य;

#ifdef _WIN32
  auto H = reinterpret_cast<HANDLE>(_get_osfhandle(FD));
  यदि (std::error_code EC = setDeleteDisposition(H, असत्य))
    प्रत्यागम errorCodeToError(EC);
#endif
  sys::DontRemoveFileOnSignal(TmpName);

  TmpName = "";

  यदि (close(FD) == -1)
    प्रत्यागम errorCodeToError(errnoAsErrorCode());
  FD = -1;

  प्रत्यागम Error::success();
}

Expected<TempFile> TempFile::create(ध्रुव Twine &Model, unsigned Mode,
                                    OpenFlags ExtraFlags) {
  पूर्णाङ्क FD;
  SmallString<128> ResultPath;
  यदि (std::error_code EC =
          createUniqueFile(Model, FD, ResultPath, OF_Delete | ExtraFlags, Mode))
    प्रत्यागम errorCodeToError(EC);

  TempFile Ret(ResultPath, FD);
#ifdef _WIN32
  auto H = reinterpret_cast<HANDLE>(_get_osfhandle(FD));
  तार्किक SetSignalHandler = असत्य;
  यदि (std::error_code EC = setDeleteDisposition(H, सत्य)) {
    Ret.RemoveOnClose = सत्य;
    SetSignalHandler = सत्य;
  }
#अन्यथा
  तार्किक SetSignalHandler = सत्य;
#endif
  यदि (SetSignalHandler && sys::RemoveFileOnSignal(ResultPath)) {
    // Make sure we delete the file when RemoveFileOnSignal fails.
    consumeError(Ret.discard());
    std::error_code EC(errc::operation_not_permitted);
    प्रत्यागम errorCodeToError(EC);
  }
  प्रत्यागम std::move(Ret);
}
} // namespace fs

} // namespace sys
} // namespace llvm
