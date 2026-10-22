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

#जर !defined(_MSC_VER) && !defined(__MINGW32__)
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

  ओळीत तार्किक prefer_forward_slash() {
    स्थिर तार्किक prefer = []() {
      जर (std::optional<std::string> Env =
              sys::Process::GetEnv("LLVM_WINDOWS_PREFER_FORWARD_SLASH"))
        परत *Env == "1";
      परत static_cast<तार्किक>(LLVM_WINDOWS_PREFER_FORWARD_SLASH);
    }();
    परत prefer;
  }

  ओळीत Style real_style(Style style) {
    जर (style != Style::native)
      परत style;
    जर (is_style_posix(style))
      परत Style::posix;
    परत prefer_forward_slash() ? Style::windows_slash
                                  : Style::windows_backslash;
  }

  ओळीत स्थिरांक अक्षर *separators(Style style) {
    जर (is_style_windows(style))
      परत "\\/";
    परत "/";
  }

  ओळीत अक्षर preferred_separator(Style style) {
    जर (real_style(style) == Style::windows)
      परत '\\';
    परत '/';
  }

  StringRef find_first_component(StringRef path, Style style) {
    // Look for this first component in the following order.
    // * empty (in this case we return an empty string)
    // * either C: or {//,\\}net.
    // * {/,\}
    // * {file,directory}name

    जर (path.empty())
      परत path;

    जर (is_style_windows(style)) {
      // C:
      जर (path.size() >= 2 &&
          std::है_अक्षर(static_cast<अचिन्हित अक्षर>(path[0])) && path[1] == ':')
        परत path.substr(0, 2);
    }

    // //net
    जर ((path.size() > 2) && is_separator(path[0], style) &&
        path[0] == path[1] && !is_separator(path[2], style)) {
      // Find the next directory separator.
      माप_प्रकार end = path.find_first_of(separators(style), 2);
      परत path.substr(0, end);
    }

    // {/,\}
    जर (is_separator(path[0], style))
      परत path.substr(0, 1);

    // * {file,directory}name
    माप_प्रकार end = path.find_first_of(separators(style));
    परत path.substr(0, end);
  }

  // Returns the first character of the filename in str. For paths ending in
  // '/', it returns the position of the '/'.
  माप_प्रकार filename_pos(StringRef str, Style style) {
    जर (str.size() > 0 && is_separator(str[str.size() - 1], style))
      परत str.size() - 1;

    माप_प्रकार pos = str.find_last_of(separators(style), str.size() - 1);

    जर (is_style_windows(style)) {
      जर (pos == StringRef::npos)
        pos = str.find_last_of(':', str.size() - 1);
    }

    जर (pos == StringRef::npos || (pos == 1 && is_separator(str[0], style)))
      परत 0;

    परत pos + 1;
  }

  // Returns the position of the root directory in str. If there is no root
  // directory in str, it returns StringRef::npos.
  माप_प्रकार मूळ_dir_start(StringRef str, Style style) {
    // case "c:/"
    जर (is_style_windows(style)) {
      जर (str.size() > 2 && str[1] == ':' && is_separator(str[2], style))
        परत 2;
    }

    // case "//net"
    जर (str.size() > 3 && is_separator(str[0], style) && str[0] == str[1] &&
        !is_separator(str[2], style)) {
      परत str.find_first_of(separators(style), 2);
    }

    // case "/"
    जर (str.size() > 0 && is_separator(str[0], style))
      परत 0;

    परत StringRef::npos;
  }

  // Returns the position past the end of the "parent path" of path. The parent
  // path will not end in '/', unless the parent is the root directory. If the
  // path has no parent, 0 is returned.
  माप_प्रकार parent_path_end(StringRef path, Style style) {
    माप_प्रकार end_pos = filename_pos(path, style);

    तार्किक filename_was_sep =
        path.size() > 0 && is_separator(path[end_pos], style);

    // Skip separators until we reach root dir (or the start of the string).
    माप_प्रकार मूळ_dir_pos = मूळ_dir_start(path, style);
    पर्यंत (end_pos > 0 &&
           (मूळ_dir_pos == StringRef::npos || end_pos > मूळ_dir_pos) &&
           is_separator(path[end_pos - 1], style))
      --end_pos;

    जर (end_pos == मूळ_dir_pos && !filename_was_sep) {
      // We've reached the root dir and the input path was *not* ending in a
      // sequence of slashes. Include the root dir in the parent path.
      परत मूळ_dir_pos + 1;
    }

    // Otherwise, just include before the last slash.
    परत end_pos;
  }
} // end unnamed namespace

गणना FSEntity {
  FS_Dir,
  FS_File,
  FS_Name
};

स्थिर std::error_code
createUniqueEntity(स्थिरांक Twine &Model, पूर्णांक &ResultFD,
                   SmallVectorImpl<अक्षर> &ResultPath, तार्किक MakeAbsolute,
                   FSEntity Type, sys::fs::OpenFlags Flags = sys::fs::OF_None,
                   अचिन्हित Mode = 0) {

  // Limit the number of attempts we make, so that we don't infinite loop. E.g.
  // "permission denied" could be for a specific file (so we retry with a
  // different name) or for the whole directory (retry would always fail).
  // Checking which is racy, so we try a number of times, then give up.
  std::error_code EC;
  साठी (पूर्णांक Retries = 128; Retries > 0; --Retries) {
    sys::fs::createUniquePath(Model, ResultPath, MakeAbsolute);
    // Try to open + create the file.
    निवड (Type) {
    स्थिती FS_File: {
      EC = sys::fs::openFileForReadWrite(Twine(ResultPath.begin()), ResultFD,
                                         sys::fs::CD_CreateNew, Flags, Mode);
      जर (EC) {
        // errc::permission_denied happens on Windows when we try to open a file
        // that has been marked for deletion.
        जर (EC == errc::file_exists || EC == errc::permission_denied)
          सुरूठेवा;
        परत EC;
      }

      परत std::error_code();
    }

    स्थिती FS_Name: {
      EC = sys::fs::access(ResultPath.begin(), sys::fs::AccessMode::Exist);
      जर (EC == errc::no_such_file_or_directory)
        परत std::error_code();
      जर (EC)
        परत EC;
      सुरूठेवा;
    }

    स्थिती FS_Dir: {
      EC = sys::fs::create_directory(ResultPath.begin(), असत्य);
      जर (EC) {
        जर (EC == errc::file_exists)
          सुरूठेवा;
        परत EC;
      }
      परत std::error_code();
    }
    }
    llvm_unreachable("Invalid Type");
  }
  परत EC;
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
  परत i;
}

const_iterator end(StringRef path) {
  const_iterator i;
  i.Path      = path;
  i.Position  = path.size();
  परत i;
}

const_iterator &const_iterator::चालक++() {
  प्रतिज्ञा(Position < Path.size() && "Tried to increment past end!");

  // Increment Position to past the current component
  Position += Component.size();

  // Check for end.
  जर (Position == Path.size()) {
    Component = StringRef();
    परत *हे;
  }

  // Both POSIX and Windows treat paths that begin with exactly two separators
  // specially.
  तार्किक was_net = Component.size() > 2 && is_separator(Component[0], S) &&
                 Component[1] == Component[0] && !is_separator(Component[2], S);

  // Handle separators.
  जर (is_separator(Path[Position], S)) {
    // Root dir.
    जर (was_net ||
        // c:/
        (is_style_windows(S) && Component.ends_with(":"))) {
      Component = Path.substr(Position, 1);
      परत *हे;
    }

    // Skip extra separators.
    पर्यंत (Position != Path.size() && is_separator(Path[Position], S)) {
      ++Position;
    }

    // Treat trailing '/' as a '.', unless it is the root dir.
    जर (Position == Path.size() && Component != "/") {
      --Position;
      Component = ".";
      परत *हे;
    }
  }

  // Find next component.
  माप_प्रकार end_pos = Path.find_first_of(separators(S), Position);
  Component = Path.slice(Position, end_pos);

  परत *हे;
}

तार्किक const_iterator::चालक==(स्थिरांक const_iterator &RHS) स्थिरांक {
  परत Path.begin() == RHS.Path.begin() && Position == RHS.Position;
}

सूचक_भेद_प्रकार const_iterator::चालक-(स्थिरांक const_iterator &RHS) स्थिरांक {
  परत Position - RHS.Position;
}

reverse_iterator rbegin(StringRef Path, Style style) {
  reverse_iterator I;
  I.Path = Path;
  I.Position = Path.size();
  I.S = style;
  ++I;
  परत I;
}

reverse_iterator rend(StringRef Path) {
  reverse_iterator I;
  I.Path = Path;
  I.Component = Path.substr(0, 0);
  I.Position = 0;
  परत I;
}

reverse_iterator &reverse_iterator::चालक++() {
  माप_प्रकार मूळ_dir_pos = मूळ_dir_start(Path, S);

  // Skip separators unless it's the root directory.
  माप_प्रकार end_pos = Position;
  पर्यंत (end_pos > 0 && (end_pos - 1) != मूळ_dir_pos &&
         is_separator(Path[end_pos - 1], S))
    --end_pos;

  // Treat trailing '/' as a '.', unless it is the root dir.
  जर (Position == Path.size() && !Path.empty() &&
      is_separator(Path.back(), S) &&
      (मूळ_dir_pos == StringRef::npos || end_pos - 1 > मूळ_dir_pos)) {
    --Position;
    Component = ".";
    परत *हे;
  }

  // Find next separator.
  माप_प्रकार start_pos = filename_pos(Path.substr(0, end_pos), S);
  Component = Path.slice(start_pos, end_pos);
  Position = start_pos;
  परत *हे;
}

तार्किक reverse_iterator::चालक==(स्थिरांक reverse_iterator &RHS) स्थिरांक {
  परत Path.begin() == RHS.Path.begin() && Component == RHS.Component &&
         Position == RHS.Position;
}

सूचक_भेद_प्रकार reverse_iterator::चालक-(स्थिरांक reverse_iterator &RHS) स्थिरांक {
  परत Position - RHS.Position;
}

StringRef मूळ_path(StringRef path, Style style) {
  const_iterator b = begin(path, style), pos = b, e = end(path);
  जर (b != e) {
    तार्किक has_net =
        b->size() > 2 && is_separator((*b)[0], style) && (*b)[1] == (*b)[0];
    तार्किक has_drive = is_style_windows(style) && b->ends_with(":");

    जर (has_net || has_drive) {
      जर ((++pos != e) && is_separator((*pos)[0], style)) {
        // {C:/,//net/}, so get the first two components.
        परत path.substr(0, b->size() + pos->size());
      }
      // just {C:,//net}, return the first component.
      परत *b;
    }

    // POSIX style root directory.
    जर (is_separator((*b)[0], style)) {
      परत *b;
    }
  }

  परत StringRef();
}

StringRef मूळ_name(StringRef path, Style style) {
  const_iterator b = begin(path, style), e = end(path);
  जर (b != e) {
    तार्किक has_net =
        b->size() > 2 && is_separator((*b)[0], style) && (*b)[1] == (*b)[0];
    तार्किक has_drive = is_style_windows(style) && b->ends_with(":");

    जर (has_net || has_drive) {
      // just {C:,//net}, return the first component.
      परत *b;
    }
  }

  // No path or no name.
  परत StringRef();
}

StringRef मूळ_directory(StringRef path, Style style) {
  const_iterator b = begin(path, style), pos = b, e = end(path);
  जर (b != e) {
    तार्किक has_net =
        b->size() > 2 && is_separator((*b)[0], style) && (*b)[1] == (*b)[0];
    तार्किक has_drive = is_style_windows(style) && b->ends_with(":");

    जर ((has_net || has_drive) &&
        // {C:,//net}, skip to the next component.
        (++pos != e) && is_separator((*pos)[0], style)) {
      परत *pos;
    }

    // POSIX style root directory.
    जर (!has_net && is_separator((*b)[0], style)) {
      परत *b;
    }
  }

  // No path or no root.
  परत StringRef();
}

StringRef relative_path(StringRef path, Style style) {
  StringRef मूळ_3 = मूळ_path(path, style);
  परत path.substr(मूळ_3.size());
}

शून्य append(SmallVectorImpl<अक्षर> &path, Style style, स्थिरांक Twine &a,
            स्थिरांक Twine &b, स्थिरांक Twine &c, स्थिरांक Twine &d) {
  SmallString<32> a_storage;
  SmallString<32> b_storage;
  SmallString<32> c_storage;
  SmallString<32> d_storage;

  SmallVector<StringRef, 4> components;
  जर (!a.isTriviallyEmpty()) components.ढकला_back(a.toStringRef(a_storage));
  जर (!b.isTriviallyEmpty()) components.ढकला_back(b.toStringRef(b_storage));
  जर (!c.isTriviallyEmpty()) components.ढकला_back(c.toStringRef(c_storage));
  जर (!d.isTriviallyEmpty()) components.ढकला_back(d.toStringRef(d_storage));

  साठी (स्वतः &component : components) {
    तार्किक path_has_sep =
        !path.empty() && is_separator(path[path.size() - 1], style);
    जर (path_has_sep) {
      // Strip separators from beginning of component.
      माप_प्रकार loc = component.find_first_not_of(separators(style));
      StringRef c = component.substr(loc);

      // Append it.
      path.append(c.begin(), c.end());
      सुरूठेवा;
    }

    तार्किक component_has_sep =
        !component.empty() && is_separator(component[0], style);
    जर (!component_has_sep &&
        !(path.empty() || has_मूळ_name(component, style))) {
      // Add a separator.
      path.ढकला_back(preferred_separator(style));
    }

    path.append(component.begin(), component.end());
  }
}

शून्य append(SmallVectorImpl<अक्षर> &path, स्थिरांक Twine &a, स्थिरांक Twine &b,
            स्थिरांक Twine &c, स्थिरांक Twine &d) {
  append(path, Style::native, a, b, c, d);
}

शून्य append(SmallVectorImpl<अक्षर> &path, const_iterator begin,
            const_iterator end, Style style) {
  साठी (; begin != end; ++begin)
    path::append(path, style, *begin);
}

StringRef parent_path(StringRef path, Style style) {
  माप_प्रकार end_pos = parent_path_end(path, style);
  जर (end_pos == StringRef::npos)
    परत StringRef();
  परत path.substr(0, end_pos);
}

शून्य remove_filename(SmallVectorImpl<अक्षर> &path, Style style) {
  माप_प्रकार end_pos = parent_path_end(StringRef(path.begin(), path.size()), style);
  जर (end_pos != StringRef::npos)
    path.truncate(end_pos);
}

शून्य replace_extension(SmallVectorImpl<अक्षर> &path, स्थिरांक Twine &extension,
                       Style style) {
  StringRef p(path.begin(), path.size());
  SmallString<32> ext_storage;
  StringRef ext = extension.toStringRef(ext_storage);

  // Erase existing extension.
  माप_प्रकार pos = p.find_last_of('.');
  जर (pos != StringRef::npos && pos >= filename_pos(p, style))
    path.truncate(pos);

  // Append '.' if needed.
  जर (ext.size() > 0 && ext[0] != '.')
    path.ढकला_back('.');

  // Append extension.
  path.append(ext.begin(), ext.end());
}

स्थिर तार्किक starts_with(StringRef Path, StringRef Prefix,
                        Style style = Style::native) {
  // Windows prefix matching : case and separator insensitive
  जर (is_style_windows(style)) {
    जर (Path.size() < Prefix.size())
      परत असत्य;
    साठी (माप_प्रकार I = 0, E = Prefix.size(); I != E; ++I) {
      तार्किक SepPath = is_separator(Path[I], style);
      तार्किक SepPrefix = is_separator(Prefix[I], style);
      जर (SepPath != SepPrefix)
        परत असत्य;
      जर (!SepPath && toLower(Path[I]) != toLower(Prefix[I]))
        परत असत्य;
    }
    परत सत्य;
  }
  परत Path.starts_with(Prefix);
}

तार्किक replace_path_prefix(SmallVectorImpl<अक्षर> &Path, StringRef OldPrefix,
                         StringRef NewPrefix, Style style) {
  जर (OldPrefix.empty() && NewPrefix.empty())
    परत असत्य;

  StringRef OrigPath(Path.begin(), Path.size());
  जर (!starts_with(OrigPath, OldPrefix, style))
    परत असत्य;

  // If prefixes have the same size we can simply copy the new one over.
  जर (OldPrefix.size() == NewPrefix.size()) {
    llvm::copy(NewPrefix, Path.begin());
    परत सत्य;
  }

  StringRef RelPath = OrigPath.substr(OldPrefix.size());
  SmallString<256> NewPath;
  (Twine(NewPrefix) + RelPath).toVector(NewPath);
  Path.swap(NewPath);
  परत सत्य;
}

शून्य native(स्थिरांक Twine &path, SmallVectorImpl<अक्षर> &result, Style style) {
  प्रतिज्ञा((!path.isSingleStringRef() ||
          path.getSingleStringRef().data() != result.data()) &&
         "path and result are not allowed to overlap!");
  // Clear result.
  result.clear();
  path.toVector(result);
  native(result, style);
}

std::string native(स्थिरांक Twine &path, Style style) {
  SmallString<128> Result;
  native(path, Result, style);
  परत std::string(Result);
}

शून्य native(SmallVectorImpl<अक्षर> &Path, Style style) {
  जर (Path.empty())
    परत;
  जर (is_style_windows(style)) {
    साठी (अक्षर &Ch : Path)
      जर (is_separator(Ch, style))
        Ch = preferred_separator(style);
    जर (Path[0] == '~' && (Path.size() == 1 || is_separator(Path[1], style))) {
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
  जर (is_style_posix(style))
    परत std::string(path);

  std::string s = path.str();
  llvm::replace(s, '\\', '/');
  परत s;
}

StringRef filename(StringRef path, Style style) { परत *rbegin(path, style); }

StringRef stem(StringRef path, Style style) {
  StringRef fname = filename(path, style);
  माप_प्रकार pos = fname.find_last_of('.');
  जर (pos == StringRef::npos)
    परत fname;
  जर ((fname.size() == 1 && fname == ".") ||
      (fname.size() == 2 && fname == ".."))
    परत fname;
  परत fname.substr(0, pos);
}

StringRef extension(StringRef path, Style style) {
  StringRef fname = filename(path, style);
  माप_प्रकार pos = fname.find_last_of('.');
  जर (pos == StringRef::npos)
    परत StringRef();
  जर ((fname.size() == 1 && fname == ".") ||
      (fname.size() == 2 && fname == ".."))
    परत StringRef();
  परत fname.substr(pos);
}

तार्किक is_separator(अक्षर मूल्य_2, Style style) {
  जर (मूल्य_2 == '/')
    परत सत्य;
  जर (is_style_windows(style))
    परत मूल्य_2 == '\\';
  परत असत्य;
}

StringRef get_separator(Style style) {
  जर (real_style(style) == Style::windows)
    परत "\\";
  परत "/";
}

तार्किक has_मूळ_name(स्थिरांक Twine &path, Style style) {
  SmallString<128> path_storage;
  StringRef p = path.toStringRef(path_storage);

  परत !मूळ_name(p, style).empty();
}

तार्किक has_मूळ_directory(स्थिरांक Twine &path, Style style) {
  SmallString<128> path_storage;
  StringRef p = path.toStringRef(path_storage);

  परत !मूळ_directory(p, style).empty();
}

तार्किक has_मूळ_path(स्थिरांक Twine &path, Style style) {
  SmallString<128> path_storage;
  StringRef p = path.toStringRef(path_storage);

  परत !मूळ_path(p, style).empty();
}

तार्किक has_relative_path(स्थिरांक Twine &path, Style style) {
  SmallString<128> path_storage;
  StringRef p = path.toStringRef(path_storage);

  परत !relative_path(p, style).empty();
}

तार्किक has_filename(स्थिरांक Twine &path, Style style) {
  SmallString<128> path_storage;
  StringRef p = path.toStringRef(path_storage);

  परत !filename(p, style).empty();
}

तार्किक has_parent_path(स्थिरांक Twine &path, Style style) {
  SmallString<128> path_storage;
  StringRef p = path.toStringRef(path_storage);

  परत !parent_path(p, style).empty();
}

तार्किक has_stem(स्थिरांक Twine &path, Style style) {
  SmallString<128> path_storage;
  StringRef p = path.toStringRef(path_storage);

  परत !stem(p, style).empty();
}

तार्किक has_extension(स्थिरांक Twine &path, Style style) {
  SmallString<128> path_storage;
  StringRef p = path.toStringRef(path_storage);

  परत !extension(p, style).empty();
}

तार्किक is_absolute(स्थिरांक Twine &path, Style style) {
  SmallString<128> path_storage;
  StringRef p = path.toStringRef(path_storage);

  तार्किक मूळ_Dir = has_मूळ_directory(p, style);
  तार्किक मूळ_Name = is_style_posix(style) || has_मूळ_name(p, style);

  परत मूळ_Dir && मूळ_Name;
}

तार्किक is_absolute_gnu(स्थिरांक Twine &path, Style style) {
  SmallString<128> path_storage;
  StringRef p = path.toStringRef(path_storage);

  // Handle '/' which is absolute for both Windows and POSIX systems.
  // Handle '\\' on Windows.
  जर (!p.empty() && is_separator(p.front(), style))
    परत सत्य;

  जर (is_style_windows(style)) {
    // Handle drive letter pattern (a character followed by ':') on Windows.
    जर (p.size() >= 2 && (p[0] && p[1] == ':'))
      परत सत्य;
  }

  परत असत्य;
}

तार्किक is_relative(स्थिरांक Twine &path, Style style) {
  परत !is_absolute(path, style);
}

शून्य make_absolute(स्थिरांक Twine &current_directory,
                   SmallVectorImpl<अक्षर> &path) {
  StringRef p(path.data(), path.size());

  तार्किक मूळ_Directory = has_मूळ_directory(p);
  तार्किक मूळ_Name = has_मूळ_name(p);

  // Already absolute.
  जर ((मूळ_Name || is_style_posix(Style::native)) && मूळ_Directory)
    परत;

  // All the following conditions will need the current directory.
  SmallString<128> current_dir;
  current_directory.toVector(current_dir);

  // Relative path. Prepend the current directory.
  जर (!मूळ_Name && !मूळ_Directory) {
    // Append path to the current directory.
    append(current_dir, p);
    // Set path to the result.
    path.swap(current_dir);
    परत;
  }

  जर (!मूळ_Name && मूळ_Directory) {
    StringRef cdrn = मूळ_name(current_dir);
    SmallString<128> cur_Dir_मूळ_Name(cdrn.begin(), cdrn.end());
    append(cur_Dir_मूळ_Name, p);
    // Set path to the result.
    path.swap(cur_Dir_मूळ_Name);
    परत;
  }

  जर (मूळ_Name && !मूळ_Directory) {
    StringRef p_मूळ_Name = मूळ_name(p);
    StringRef b_मूळ_Directory = मूळ_directory(current_dir);
    StringRef bRelativePath = relative_path(current_dir);
    StringRef pRelativePath = relative_path(p);

    SmallString<128> res;
    append(res, p_मूळ_Name, b_मूळ_Directory, bRelativePath, pRelativePath);
    path.swap(res);
    परत;
  }

  llvm_unreachable("All rootName and rootDirectory combinations should have "
                   "occurred above!");
}

StringRef remove_leading_dotslash(StringRef Path, Style style) {
  // Remove leading "./" (or ".//" or "././" etc.)
  पर्यंत (Path.size() > 2 && Path[0] == '.' && is_separator(Path[1], style)) {
    Path = Path.substr(2);
    पर्यंत (Path.size() > 0 && is_separator(Path[0], style))
      Path = Path.substr(1);
  }
  परत Path;
}

तार्किक remove_dots(SmallVectorImpl<अक्षर> &the_path, तार्किक remove_dot_dot,
                 Style style) {
  style = real_style(style);
  StringRef remaining(the_path.data(), the_path.size());
  तार्किक needs_change = असत्य;
  SmallVector<StringRef, 16> components;

  // Consume the root path, if present.
  StringRef मूळ_3 = path::मूळ_path(remaining, style);
  तार्किक absolute = !मूळ_3.empty();
  जर (absolute)
    remaining = remaining.drop_front(मूळ_3.size());

  // Loop over path components manually. This makes it easier to detect
  // non-preferred slashes and double separators that must be canonicalized.
  पर्यंत (!remaining.empty()) {
    माप_प्रकार next_slash = remaining.find_first_of(separators(style));
    जर (next_slash == StringRef::npos)
      next_slash = remaining.size();
    StringRef component = remaining.take_front(next_slash);
    remaining = remaining.drop_front(next_slash);

    // Eat the slash, and check if it is the preferred separator.
    जर (!remaining.empty()) {
      needs_change |= remaining.front() != preferred_separator(style);
      remaining = remaining.drop_front();
      // The path needs to be rewritten if it has a trailing slash.
      // FIXME: This is emergent behavior that could be removed.
      needs_change |= remaining.empty();
    }

    // Check for path traversal components or double separators.
    जर (component.empty() || component == ".") {
      needs_change = सत्य;
    } अन्यथा जर (remove_dot_dot && component == "..") {
      needs_change = सत्य;
      // Do not allow ".." to remove the root component. If this is the
      // beginning of a relative path, keep the ".." component.
      जर (!components.empty() && components.back() != "..") {
        components.काढा_back();
      } अन्यथा जर (!absolute) {
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
  जर (!needs_change)
    परत असत्य;

  जर (!components.empty()) {
    buffer += components[0];
    साठी (StringRef C : ArrayRef(components).drop_front()) {
      buffer += preferred_separator(style);
      buffer += C;
    }
  }
  the_path.swap(buffer);
  परत सत्य;
}

} // end namespace path

नामाकाश fs {

std::error_code getUniqueID(स्थिरांक Twine Path, UniqueID &Result) {
  sandbox::violationIfEnabled();

  file_status Status;
  std::error_code EC = status(Path, Status);
  जर (EC)
    परत EC;
  Result = Status.getUniqueID();
  परत std::error_code();
}

शून्य createUniquePath(स्थिरांक Twine &Model, SmallVectorImpl<अक्षर> &ResultPath,
                      तार्किक MakeAbsolute) {
  SmallString<128> ModelStorage;
  Model.toVector(ModelStorage);

  प्रतिज्ञा(llvm::is_contained(ModelStorage, '%') &&
         "createUniquePath: Model must contain at least one '%'");

  जर (MakeAbsolute) {
    // Make model absolute by prepending a temp directory if it's not already.
    जर (!sys::path::is_absolute(Twine(ModelStorage))) {
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
  साठी (अचिन्हित i = 0, e = ModelStorage.size(); i != e; ++i) {
    जर (ModelStorage[i] == '%')
      ResultPath[i] = "0123456789abcdef"[sys::Process::Get_Random_संख्या() & 15];
  }
}

std::error_code createUniqueFile(स्थिरांक Twine &Model, पूर्णांक &ResultFd,
                                 SmallVectorImpl<अक्षर> &ResultPath,
                                 OpenFlags Flags, अचिन्हित Mode) {
  परत createUniqueEntity(Model, ResultFd, ResultPath, असत्य, FS_File, Flags,
                            Mode);
}

std::error_code createUniqueFile(स्थिरांक Twine &Model,
                                 SmallVectorImpl<अक्षर> &ResultPath,
                                 अचिन्हित Mode) {
  पूर्णांक FD;
  स्वतः EC = createUniqueFile(Model, FD, ResultPath, OF_None, Mode);
  जर (EC)
    परत EC;
  // FD is only needed to avoid race conditions. Close it right away.
  बंद(FD);
  परत EC;
}

स्थिर std::error_code
createTemporaryFile(स्थिरांक Twine &Model, पूर्णांक &ResultFD,
                    llvm::SmallVectorImpl<अक्षर> &ResultPath, FSEntity Type,
                    sys::fs::OpenFlags Flags = sys::fs::OF_None) {
  // Any *temporary* file is assumed to be a compiler-internal output, not
  // a formal one.
  स्वतः BypassSandbox = sys::sandbox::scopedDisable();

  SmallString<128> Storage;
  StringRef P = Model.toNullTerminatedStringRef(Storage);
  प्रतिज्ञा(P.find_first_of(separators(Style::native)) == StringRef::npos &&
         "Model must be a simple filename.");
  // Use P.begin() so that createUniqueEntity doesn't need to recreate Storage.
  परत createUniqueEntity(P.begin(), ResultFD, ResultPath, सत्य, Type, Flags,
                            all_read | all_write);
}

स्थिर std::error_code
createTemporaryFile(स्थिरांक Twine &Prefix, StringRef Suffix, पूर्णांक &ResultFD,
                    llvm::SmallVectorImpl<अक्षर> &ResultPath, FSEntity Type,
                    sys::fs::OpenFlags Flags = sys::fs::OF_None) {
  स्थिरांक अक्षर *मधला = Suffix.empty() ? "-%%%%%%" : "-%%%%%%.";
  परत createTemporaryFile(Prefix + मधला + Suffix, ResultFD, ResultPath,
                             Type, Flags);
}

std::error_code createTemporaryFile(स्थिरांक Twine &Prefix, StringRef Suffix,
                                    पूर्णांक &ResultFD,
                                    SmallVectorImpl<अक्षर> &ResultPath,
                                    sys::fs::OpenFlags Flags) {
  परत createTemporaryFile(Prefix, Suffix, ResultFD, ResultPath, FS_File,
                             Flags);
}

std::error_code createTemporaryFile(स्थिरांक Twine &Prefix, StringRef Suffix,
                                    SmallVectorImpl<अक्षर> &ResultPath,
                                    sys::fs::OpenFlags Flags) {
  पूर्णांक FD;
  स्वतः EC = createTemporaryFile(Prefix, Suffix, FD, ResultPath, Flags);
  जर (EC)
    परत EC;
  // FD is only needed to avoid race conditions. Close it right away.
  बंद(FD);
  परत EC;
}

// This is a mkdtemp with a different pattern. We use createUniqueEntity mostly
// for consistency. We should try using mkdtemp.
std::error_code createUniqueDirectory(स्थिरांक Twine &Prefix,
                                      SmallVectorImpl<अक्षर> &ResultPath) {
  पूर्णांक Dummy;
  परत createUniqueEntity(Prefix + "-%%%%%%", Dummy, ResultPath, सत्य,
                            FS_Dir);
}

std::error_code
getPotentiallyUniqueFileName(स्थिरांक Twine &Model,
                             SmallVectorImpl<अक्षर> &ResultPath) {
  पूर्णांक Dummy;
  परत createUniqueEntity(Model, Dummy, ResultPath, असत्य, FS_Name);
}

std::error_code
getPotentiallyUniqueTempFileName(स्थिरांक Twine &Prefix, StringRef Suffix,
                                 SmallVectorImpl<अक्षर> &ResultPath) {
  पूर्णांक Dummy;
  परत createTemporaryFile(Prefix, Suffix, Dummy, ResultPath, FS_Name);
}

std::error_code make_absolute(SmallVectorImpl<अक्षर> &path) {
  sandbox::violationIfEnabled();

  जर (path::is_absolute(path))
    परत {};

  SmallString<128> current_dir;
  जर (std::error_code ec = current_path(current_dir))
    परत ec;

  path::make_absolute(current_dir, path);
  परत {};
}

std::error_code create_directories(स्थिरांक Twine &Path, तार्किक IgnoreExisting,
                                   perms Perms) {
  SmallString<128> PathStorage;
  StringRef P = Path.toStringRef(PathStorage);

  // Be optimistic and try to create the directory
  std::error_code EC = create_directory(P, IgnoreExisting, Perms);
  // If we succeeded, or had any error other than the parent not existing, just
  // return it.
  जर (EC != errc::no_such_file_or_directory)
    परत EC;

  // We failed because of a no_such_file_or_directory, try to create the
  // parent.
  StringRef Parent = path::parent_path(P);
  जर (Parent.empty())
    परत EC;

  जर ((EC = create_directories(Parent, IgnoreExisting, Perms)))
      परत EC;

  परत create_directory(P, IgnoreExisting, Perms);
}

स्थिर std::error_code copy_file_internal(पूर्णांक ReadFD, पूर्णांक WriteFD) {
  स्थिरांक माप_प्रकार BufSize = 4096;
  अक्षर *Buf = नवीन अक्षर[BufSize];
  पूर्णांक BytesRead = 0, BytesWritten = 0;
  साठी (;;) {
    BytesRead = पठ(ReadFD, Buf, BufSize);
    जर (BytesRead <= 0)
      थांबा;
    पर्यंत (BytesRead) {
      BytesWritten = लिख(WriteFD, Buf, BytesRead);
      जर (BytesWritten < 0)
        थांबा;
      BytesRead -= BytesWritten;
    }
    जर (BytesWritten < 0)
      थांबा;
  }
  delete[] Buf;

  जर (BytesRead < 0 || BytesWritten < 0)
    परत errnoAsErrorCode();
  परत std::error_code();
}

#ifndef __APPLE__
std::error_code copy_file(स्थिरांक Twine &From, स्थिरांक Twine &To) {
  पूर्णांक ReadFD, WriteFD;
  जर (std::error_code EC = openFileForRead(From, ReadFD, OF_None))
    परत EC;
  जर (std::error_code EC =
          openFileForWrite(To, WriteFD, CD_CreateAlways, OF_None)) {
    बंद(ReadFD);
    परत EC;
  }

  std::error_code EC = copy_file_internal(ReadFD, WriteFD);

  बंद(ReadFD);
  बंद(WriteFD);

  परत EC;
}
#endif

std::error_code copy_file(स्थिरांक Twine &From, पूर्णांक ToFD) {
  पूर्णांक ReadFD;
  जर (std::error_code EC = openFileForRead(From, ReadFD, OF_None))
    परत EC;

  std::error_code EC = copy_file_internal(ReadFD, ToFD);

  बंद(ReadFD);

  परत EC;
}

ErrorOr<MD5::MD5Result> md5_contents(पूर्णांक FD) {
  sandbox::violationIfEnabled();

  MD5 Hash;

  constexpr माप_प्रकार BufSize = 4096;
  std::vector<uint8_t> Buf(BufSize);
  पूर्णांक BytesRead = 0;
  साठी (;;) {
    BytesRead = पठ(FD, Buf.data(), BufSize);
    जर (BytesRead <= 0)
      थांबा;
    Hash.update(ArrayRef(Buf.data(), BytesRead));
  }

  जर (BytesRead < 0)
    परत errnoAsErrorCode();
  MD5::MD5Result Result;
  Hash.final(Result);
  परत Result;
}

ErrorOr<MD5::MD5Result> md5_contents(स्थिरांक Twine &Path) {
  sandbox::violationIfEnabled();

  पूर्णांक FD;
  जर (स्वतः EC = openFileForRead(Path, FD, OF_None))
    परत EC;

  स्वतः Result = md5_contents(FD);
  बंद(FD);
  परत Result;
}

तार्किक exists(स्थिरांक basic_file_status &status) {
  परत status_known(status) && status.type() != file_type::file_not_found;
}

तार्किक status_known(स्थिरांक basic_file_status &s) {
  परत s.type() != file_type::status_error;
}

file_type get_file_type(स्थिरांक Twine &Path, तार्किक Follow) {
  file_status st;
  जर (status(Path, st, Follow))
    परत file_type::status_error;
  परत st.type();
}

तार्किक is_directory(स्थिरांक basic_file_status &status) {
  परत status.type() == file_type::directory_file;
}

std::error_code is_directory(स्थिरांक Twine &path, तार्किक &result) {
  sandbox::violationIfEnabled();

  file_status st;
  जर (std::error_code ec = status(path, st))
    परत ec;
  result = is_directory(st);
  परत std::error_code();
}

तार्किक is_regular_file(स्थिरांक basic_file_status &status) {
  परत status.type() == file_type::regular_file;
}

std::error_code is_regular_file(स्थिरांक Twine &path, तार्किक &result) {
  sandbox::violationIfEnabled();

  file_status st;
  जर (std::error_code ec = status(path, st))
    परत ec;
  result = is_regular_file(st);
  परत std::error_code();
}

तार्किक is_symlink_file(स्थिरांक basic_file_status &status) {
  परत status.type() == file_type::symlink_file;
}

std::error_code is_symlink_file(स्थिरांक Twine &path, तार्किक &result) {
  sandbox::violationIfEnabled();

  file_status st;
  जर (std::error_code ec = status(path, st, असत्य))
    परत ec;
  result = is_symlink_file(st);
  परत std::error_code();
}

तार्किक is_other(स्थिरांक basic_file_status &status) {
  परत exists(status) &&
         !is_regular_file(status) &&
         !is_directory(status);
}

std::error_code is_other(स्थिरांक Twine &Path, तार्किक &Result) {
  sandbox::violationIfEnabled();

  file_status FileStatus;
  जर (std::error_code EC = status(Path, FileStatus))
    परत EC;
  Result = is_other(FileStatus);
  परत std::error_code();
}

शून्य directory_entry::replace_filename(स्थिरांक Twine &Filename, file_type Type,
                                       basic_file_status Status) {
  SmallString<128> PathStr = path::parent_path(Path);
  path::append(PathStr, Filename);
  हे->Path = std::string(PathStr);
  हे->Type = Type;
  हे->Status = Status;
}

ErrorOr<perms> getPermissions(स्थिरांक Twine &Path) {
  sandbox::violationIfEnabled();

  file_status Status;
  जर (std::error_code EC = status(Path, Status))
    परत EC;

  परत Status.permissions();
}

std::error_code setLastAccessAndModificationTime(स्थिरांक Twine &Path,
                                                 TimePoint<> AccessTime,
                                                 TimePoint<> ModificationTime) {
  पूर्णांक FD;
  जर (std::error_code EC = openFile(Path, FD, CD_OpenExisting, FA_Read,
                                    OF_UpdateAttributes | OF_OpenDirectory))
    परत EC;
  std::error_code EC =
      setLastAccessAndModificationTime(FD, AccessTime, ModificationTime);
  Process::SafelyCloseFileDescriptor(FD);
  परत EC;
}

माप_प्रकार mapped_file_region::size() स्थिरांक {
  प्रतिज्ञा(Mapping && "Mapping failed but used anyway!");
  परत Size;
}

अक्षर *mapped_file_region::data() स्थिरांक {
  प्रतिज्ञा(Mapping && "Mapping failed but used anyway!");
  परत reinterpret_cast<अक्षर *>(Mapping);
}

स्थिरांक अक्षर *mapped_file_region::const_data() स्थिरांक {
  प्रतिज्ञा(Mapping && "Mapping failed but used anyway!");
  परत reinterpret_cast<स्थिरांक अक्षर *>(Mapping);
}

Error readNativeFileToEOF(file_t FileHandle, SmallVectorImpl<अक्षर> &Buffer,
                          ssize_t ChunkSize) {
  sandbox::violationIfEnabled();

  // Install a handler to truncate the buffer to the correct size on exit.
  माप_प्रकार Size = Buffer.size();
  llvm::scope_exit TruncateOnExit([&]() { Buffer.truncate(Size); });

  // Read into Buffer until we hit EOF.
  साठी (;;) {
    Buffer.resize_for_overwrite(Size + ChunkSize);
    Expected<माप_प्रकार> ReadBytes = readNativeFile(
        FileHandle, MutableArrayRef(Buffer.begin() + Size, ChunkSize));
    जर (!ReadBytes)
      परत ReadBytes.takeError();
    जर (*ReadBytes == 0)
      परत Error::success();
    Size += *ReadBytes;
  }
}

} // end namespace fs
} // end namespace sys
} // end namespace llvm

// Include the truly platform-specific parts.
#जर defined(LLVM_ON_UNIX)
#अन्तर्भाव "Unix/Path.inc"
#endif
#जर defined(_WIN32)
#अन्तर्भाव "Windows/Path.inc"
#endif

नामाकाश llvm {
नामाकाश sys {
नामाकाश fs {

TempFile::TempFile(StringRef Name, पूर्णांक FD)
    : TmpName(std::string(Name)), FD(FD) {}
TempFile::TempFile(TempFile &&Other) { *हे = std::move(Other); }
TempFile &TempFile::चालक=(TempFile &&Other) {
  TmpName = std::move(Other.TmpName);
  FD = Other.FD;
  Other.Done = सत्य;
  Other.FD = -1;
#ifdef _WIN32
  RemoveOnClose = Other.RemoveOnClose;
  Other.RemoveOnClose = असत्य;
#endif
  परत *हे;
}

TempFile::~TempFile() { प्रतिज्ञा(Done); }

Error TempFile::discard() {
  Done = सत्य;
  जर (FD != -1 && बंद(FD) == -1) {
    std::error_code EC = errnoAsErrorCode();
    परत errorCodeToError(EC);
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
  जर (Remove && !TmpName.empty()) {
    RemoveEC = fs::हटाओ(TmpName);
    sys::DontRemoveFileOnSignal(TmpName);
    जर (!RemoveEC)
      TmpName = "";
  } अन्यथा {
    TmpName = "";
  }
  परत errorCodeToError(RemoveEC);
}

Error TempFile::keep(स्थिरांक Twine &Name) {
  प्रतिज्ञा(!Done);
  Done = सत्य;
  // Always try to close and rename.
#ifdef _WIN32
  // If we can't cancel the delete don't rename.
  स्वतः H = reinterpret_cast<HANDLE>(_get_osfhandle(FD));
  std::error_code RenameEC =
      RemoveOnClose ? std::error_code() : setDeleteDisposition(H, असत्य);
  तार्किक ShouldDelete = असत्य;
  जर (!RenameEC) {
    RenameEC = rename_handle(H, Name);
    // If rename failed because it's cross-device, copy instead
    जर (RenameEC ==
      std::error_code(ERROR_NOT_SAME_DEVICE, std::system_category())) {
      RenameEC = copy_file(TmpName, Name);
      ShouldDelete = सत्य;
    }
  }

  // If we can't rename or copy, discard the temporary file.
  जर (RenameEC)
    ShouldDelete = सत्य;
  जर (ShouldDelete) {
    जर (!RemoveOnClose)
      setDeleteDisposition(H, सत्य);
    अन्यथा
      हटाओ(TmpName);
  }
#अन्यथा
  std::error_code RenameEC = fs::नाम(TmpName, Name);
  जर (RenameEC) {
    // If we can't rename, try to copy to work around cross-device link issues.
    RenameEC = sys::fs::copy_file(TmpName, Name);
    // If we can't rename or copy, discard the temporary file.
    जर (RenameEC)
      हटाओ(TmpName);
  }
#endif
  sys::DontRemoveFileOnSignal(TmpName);

  जर (!RenameEC)
    TmpName = "";

  जर (बंद(FD) == -1)
    परत errorCodeToError(errnoAsErrorCode());
  FD = -1;

  परत errorCodeToError(RenameEC);
}

Error TempFile::keep() {
  प्रतिज्ञा(!Done);
  Done = सत्य;

#ifdef _WIN32
  स्वतः H = reinterpret_cast<HANDLE>(_get_osfhandle(FD));
  जर (std::error_code EC = setDeleteDisposition(H, असत्य))
    परत errorCodeToError(EC);
#endif
  sys::DontRemoveFileOnSignal(TmpName);

  TmpName = "";

  जर (बंद(FD) == -1)
    परत errorCodeToError(errnoAsErrorCode());
  FD = -1;

  परत Error::success();
}

Expected<TempFile> TempFile::create(स्थिरांक Twine &Model, अचिन्हित Mode,
                                    OpenFlags ExtraFlags) {
  पूर्णांक FD;
  SmallString<128> ResultPath;
  जर (std::error_code EC =
          createUniqueFile(Model, FD, ResultPath, OF_Delete | ExtraFlags, Mode))
    परत errorCodeToError(EC);

  TempFile Ret(ResultPath, FD);
#ifdef _WIN32
  स्वतः H = reinterpret_cast<HANDLE>(_get_osfhandle(FD));
  तार्किक SetSignalHandler = असत्य;
  जर (std::error_code EC = setDeleteDisposition(H, सत्य)) {
    Ret.RemoveOnClose = सत्य;
    SetSignalHandler = सत्य;
  }
#अन्यथा
  तार्किक SetSignalHandler = सत्य;
#endif
  जर (SetSignalHandler && sys::RemoveFileOnSignal(ResultPath)) {
    // Make sure we delete the file when RemoveFileOnSignal fails.
    consumeError(Ret.discard());
    std::error_code EC(errc::operation_not_permitted);
    परत errorCodeToError(EC);
  }
  परत std::move(Ret);
}
} // namespace fs

} // namespace sys
} // namespace llvm
