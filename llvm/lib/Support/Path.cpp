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

#અન્તર્ભાવ "llvm/Support/Path.h"
#અન્તર્ભાવ "llvm/ADT/ArrayRef.h"
#અન્તર્ભાવ "llvm/ADT/ScopeExit.h"
#અન્તર્ભાવ "llvm/ADT/StringExtras.h"
#અન્તર્ભાવ "llvm/Config/config.h"
#અન્તર્ભાવ "llvm/Config/llvm-config.h"
#અન્તર્ભાવ "llvm/Support/Errc.h"
#અન્તર્ભાવ "llvm/Support/ErrorHandling.h"
#અન્તર્ભાવ "llvm/Support/FileSystem.h"
#અન્તર્ભાવ "llvm/Support/IOSandbox.h"
#અન્તર્ભાવ "llvm/Support/Process.h"
#અન્તર્ભાવ "llvm/Support/Signals.h"
#અન્તર્ભાવ <cctype>

#જો !defined(_MSC_VER) && !defined(__MINGW32__)
#અન્તર્ભાવ <unistd.h>
#નહીંતર
#અન્તર્ભાવ <io.h>
#endif

using નામાકાશ llvm;
using નામાકાશ llvm::support::endian;

નામાકાશ {
  using llvm::StringRef;
  using llvm::sys::path::is_separator;
  using llvm::sys::path::Style;

  અંતઃરેખ તાર્કિક prefer_forward_slash() {
    સ્થિર તાર્કિક prefer = []() {
      જો (std::optional<std::string> Env =
              sys::Process::GetEnv("LLVM_WINDOWS_PREFER_FORWARD_SLASH"))
        પાછા *Env == "1";
      પાછા static_cast<તાર્કિક>(LLVM_WINDOWS_PREFER_FORWARD_SLASH);
    }();
    પાછા prefer;
  }

  અંતઃરેખ Style real_style(Style style) {
    જો (style != Style::native)
      પાછા style;
    જો (is_style_posix(style))
      પાછા Style::posix;
    પાછા prefer_forward_slash() ? Style::windows_slash
                                  : Style::windows_backslash;
  }

  અંતઃરેખ અચલ અક્ષર *separators(Style style) {
    જો (is_style_windows(style))
      પાછા "\\/";
    પાછા "/";
  }

  અંતઃરેખ અક્ષર preferred_separator(Style style) {
    જો (real_style(style) == Style::windows)
      પાછા '\\';
    પાછા '/';
  }

  StringRef find_first_component(StringRef path, Style style) {
    // Look for this first component in the following order.
    // * empty (in this case we return an empty string)
    // * either C: or {//,\\}net.
    // * {/,\}
    // * {file,directory}name

    જો (path.empty())
      પાછા path;

    જો (is_style_windows(style)) {
      // C:
      જો (path.size() >= 2 &&
          std::isalpha(static_cast<unsigned અક્ષર>(path[0])) && path[1] == ':')
        પાછા path.substr(0, 2);
    }

    // //net
    જો ((path.size() > 2) && is_separator(path[0], style) &&
        path[0] == path[1] && !is_separator(path[2], style)) {
      // Find the next directory separator.
      size_t end = path.find_first_of(separators(style), 2);
      પાછા path.substr(0, end);
    }

    // {/,\}
    જો (is_separator(path[0], style))
      પાછા path.substr(0, 1);

    // * {file,directory}name
    size_t end = path.find_first_of(separators(style));
    પાછા path.substr(0, end);
  }

  // Returns the first character of the filename in str. For paths ending in
  // '/', it returns the position of the '/'.
  size_t filename_pos(StringRef str, Style style) {
    જો (str.size() > 0 && is_separator(str[str.size() - 1], style))
      પાછા str.size() - 1;

    size_t pos = str.find_last_of(separators(style), str.size() - 1);

    જો (is_style_windows(style)) {
      જો (pos == StringRef::npos)
        pos = str.find_last_of(':', str.size() - 1);
    }

    જો (pos == StringRef::npos || (pos == 1 && is_separator(str[0], style)))
      પાછા 0;

    પાછા pos + 1;
  }

  // Returns the position of the root directory in str. If there is no root
  // directory in str, it returns StringRef::npos.
  size_t મૂળ_dir_start(StringRef str, Style style) {
    // case "c:/"
    જો (is_style_windows(style)) {
      જો (str.size() > 2 && str[1] == ':' && is_separator(str[2], style))
        પાછા 2;
    }

    // case "//net"
    જો (str.size() > 3 && is_separator(str[0], style) && str[0] == str[1] &&
        !is_separator(str[2], style)) {
      પાછા str.find_first_of(separators(style), 2);
    }

    // case "/"
    જો (str.size() > 0 && is_separator(str[0], style))
      પાછા 0;

    પાછા StringRef::npos;
  }

  // Returns the position past the end of the "parent path" of path. The parent
  // path will not end in '/', unless the parent is the root directory. If the
  // path has no parent, 0 is returned.
  size_t parent_path_end(StringRef path, Style style) {
    size_t end_pos = filename_pos(path, style);

    તાર્કિક filename_was_sep =
        path.size() > 0 && is_separator(path[end_pos], style);

    // Skip separators until we reach root dir (or the start of the string).
    size_t મૂળ_dir_pos = મૂળ_dir_start(path, style);
    જ્યારે (end_pos > 0 &&
           (મૂળ_dir_pos == StringRef::npos || end_pos > મૂળ_dir_pos) &&
           is_separator(path[end_pos - 1], style))
      --end_pos;

    જો (end_pos == મૂળ_dir_pos && !filename_was_sep) {
      // We've reached the root dir and the input path was *not* ending in a
      // sequence of slashes. Include the root dir in the parent path.
      પાછા મૂળ_dir_pos + 1;
    }

    // Otherwise, just include before the last slash.
    પાછા end_pos;
  }
} // end unnamed namespace

ગણના FSEntity {
  FS_Dir,
  FS_File,
  FS_Name
};

સ્થિર std::error_code
createUniqueEntity(અચલ Twine &Model, પૂર્ણાંક &ResultFD,
                   SmallVectorImpl<અક્ષર> &ResultPath, તાર્કિક MakeAbsolute,
                   FSEntity Type, sys::fs::OpenFlags Flags = sys::fs::OF_None,
                   unsigned Mode = 0) {

  // Limit the number of attempts we make, so that we don't infinite loop. E.g.
  // "permission denied" could be for a specific file (so we retry with a
  // different name) or for the whole directory (retry would always fail).
  // Checking which is racy, so we try a number of times, then give up.
  std::error_code EC;
  માટે (પૂર્ણાંક Retries = 128; Retries > 0; --Retries) {
    sys::fs::createUniquePath(Model, ResultPath, MakeAbsolute);
    // Try to open + create the file.
    પસંદગી (Type) {
    સ્થિતિ FS_File: {
      EC = sys::fs::openFileForReadWrite(Twine(ResultPath.begin()), ResultFD,
                                         sys::fs::CD_CreateNew, Flags, Mode);
      જો (EC) {
        // errc::permission_denied happens on Windows when we try to open a file
        // that has been marked for deletion.
        જો (EC == errc::file_exists || EC == errc::permission_denied)
          ચાલુ;
        પાછા EC;
      }

      પાછા std::error_code();
    }

    સ્થિતિ FS_Name: {
      EC = sys::fs::access(ResultPath.begin(), sys::fs::AccessMode::Exist);
      જો (EC == errc::no_such_file_or_directory)
        પાછા std::error_code();
      જો (EC)
        પાછા EC;
      ચાલુ;
    }

    સ્થિતિ FS_Dir: {
      EC = sys::fs::create_directory(ResultPath.begin(), ખોટું);
      જો (EC) {
        જો (EC == errc::file_exists)
          ચાલુ;
        પાછા EC;
      }
      પાછા std::error_code();
    }
    }
    llvm_unreachable("Invalid Type");
  }
  પાછા EC;
}

નામાકાશ llvm {
નામાકાશ sys  {
નામાકાશ path {

const_iterator begin(StringRef path, Style style) {
  const_iterator i;
  i.Path      = path;
  i.Component = find_first_component(path, style);
  i.Position  = 0;
  i.S = style;
  પાછા i;
}

const_iterator end(StringRef path) {
  const_iterator i;
  i.Path      = path;
  i.Position  = path.size();
  પાછા i;
}

const_iterator &const_iterator::operator++() {
  પ્રતિજ્ઞા(Position < Path.size() && "Tried to increment past end!");

  // Increment Position to past the current component
  Position += Component.size();

  // Check for end.
  જો (Position == Path.size()) {
    Component = StringRef();
    પાછા *આ;
  }

  // Both POSIX and Windows treat paths that begin with exactly two separators
  // specially.
  તાર્કિક was_net = Component.size() > 2 && is_separator(Component[0], S) &&
                 Component[1] == Component[0] && !is_separator(Component[2], S);

  // Handle separators.
  જો (is_separator(Path[Position], S)) {
    // Root dir.
    જો (was_net ||
        // c:/
        (is_style_windows(S) && Component.ends_with(":"))) {
      Component = Path.substr(Position, 1);
      પાછા *આ;
    }

    // Skip extra separators.
    જ્યારે (Position != Path.size() && is_separator(Path[Position], S)) {
      ++Position;
    }

    // Treat trailing '/' as a '.', unless it is the root dir.
    જો (Position == Path.size() && Component != "/") {
      --Position;
      Component = ".";
      પાછા *આ;
    }
  }

  // Find next component.
  size_t end_pos = Path.find_first_of(separators(S), Position);
  Component = Path.slice(Position, end_pos);

  પાછા *આ;
}

તાર્કિક const_iterator::operator==(અચલ const_iterator &RHS) અચલ {
  પાછા Path.begin() == RHS.Path.begin() && Position == RHS.Position;
}

ptrdiff_t const_iterator::operator-(અચલ const_iterator &RHS) અચલ {
  પાછા Position - RHS.Position;
}

reverse_iterator rbegin(StringRef Path, Style style) {
  reverse_iterator I;
  I.Path = Path;
  I.Position = Path.size();
  I.S = style;
  ++I;
  પાછા I;
}

reverse_iterator rend(StringRef Path) {
  reverse_iterator I;
  I.Path = Path;
  I.Component = Path.substr(0, 0);
  I.Position = 0;
  પાછા I;
}

reverse_iterator &reverse_iterator::operator++() {
  size_t મૂળ_dir_pos = મૂળ_dir_start(Path, S);

  // Skip separators unless it's the root directory.
  size_t end_pos = Position;
  જ્યારે (end_pos > 0 && (end_pos - 1) != મૂળ_dir_pos &&
         is_separator(Path[end_pos - 1], S))
    --end_pos;

  // Treat trailing '/' as a '.', unless it is the root dir.
  જો (Position == Path.size() && !Path.empty() &&
      is_separator(Path.back(), S) &&
      (મૂળ_dir_pos == StringRef::npos || end_pos - 1 > મૂળ_dir_pos)) {
    --Position;
    Component = ".";
    પાછા *આ;
  }

  // Find next separator.
  size_t start_pos = filename_pos(Path.substr(0, end_pos), S);
  Component = Path.slice(start_pos, end_pos);
  Position = start_pos;
  પાછા *આ;
}

તાર્કિક reverse_iterator::operator==(અચલ reverse_iterator &RHS) અચલ {
  પાછા Path.begin() == RHS.Path.begin() && Component == RHS.Component &&
         Position == RHS.Position;
}

ptrdiff_t reverse_iterator::operator-(અચલ reverse_iterator &RHS) અચલ {
  પાછા Position - RHS.Position;
}

StringRef મૂળ_path(StringRef path, Style style) {
  const_iterator b = begin(path, style), pos = b, e = end(path);
  જો (b != e) {
    તાર્કિક has_net =
        b->size() > 2 && is_separator((*b)[0], style) && (*b)[1] == (*b)[0];
    તાર્કિક has_drive = is_style_windows(style) && b->ends_with(":");

    જો (has_net || has_drive) {
      જો ((++pos != e) && is_separator((*pos)[0], style)) {
        // {C:/,//net/}, so get the first two components.
        પાછા path.substr(0, b->size() + pos->size());
      }
      // just {C:,//net}, return the first component.
      પાછા *b;
    }

    // POSIX style root directory.
    જો (is_separator((*b)[0], style)) {
      પાછા *b;
    }
  }

  પાછા StringRef();
}

StringRef મૂળ_name(StringRef path, Style style) {
  const_iterator b = begin(path, style), e = end(path);
  જો (b != e) {
    તાર્કિક has_net =
        b->size() > 2 && is_separator((*b)[0], style) && (*b)[1] == (*b)[0];
    તાર્કિક has_drive = is_style_windows(style) && b->ends_with(":");

    જો (has_net || has_drive) {
      // just {C:,//net}, return the first component.
      પાછા *b;
    }
  }

  // No path or no name.
  પાછા StringRef();
}

StringRef મૂળ_directory(StringRef path, Style style) {
  const_iterator b = begin(path, style), pos = b, e = end(path);
  જો (b != e) {
    તાર્કિક has_net =
        b->size() > 2 && is_separator((*b)[0], style) && (*b)[1] == (*b)[0];
    તાર્કિક has_drive = is_style_windows(style) && b->ends_with(":");

    જો ((has_net || has_drive) &&
        // {C:,//net}, skip to the next component.
        (++pos != e) && is_separator((*pos)[0], style)) {
      પાછા *pos;
    }

    // POSIX style root directory.
    જો (!has_net && is_separator((*b)[0], style)) {
      પાછા *b;
    }
  }

  // No path or no root.
  પાછા StringRef();
}

StringRef relative_path(StringRef path, Style style) {
  StringRef મૂળ_3 = મૂળ_path(path, style);
  પાછા path.substr(મૂળ_3.size());
}

શૂન્ય append(SmallVectorImpl<અક્ષર> &path, Style style, અચલ Twine &a,
            અચલ Twine &b, અચલ Twine &c, અચલ Twine &d) {
  SmallString<32> a_storage;
  SmallString<32> b_storage;
  SmallString<32> c_storage;
  SmallString<32> d_storage;

  SmallVector<StringRef, 4> components;
  જો (!a.isTriviallyEmpty()) components.ધકેલો_back(a.toStringRef(a_storage));
  જો (!b.isTriviallyEmpty()) components.ધકેલો_back(b.toStringRef(b_storage));
  જો (!c.isTriviallyEmpty()) components.ધકેલો_back(c.toStringRef(c_storage));
  જો (!d.isTriviallyEmpty()) components.ધકેલો_back(d.toStringRef(d_storage));

  માટે (auto &component : components) {
    તાર્કિક path_has_sep =
        !path.empty() && is_separator(path[path.size() - 1], style);
    જો (path_has_sep) {
      // Strip separators from beginning of component.
      size_t loc = component.find_first_not_of(separators(style));
      StringRef c = component.substr(loc);

      // Append it.
      path.append(c.begin(), c.end());
      ચાલુ;
    }

    તાર્કિક component_has_sep =
        !component.empty() && is_separator(component[0], style);
    જો (!component_has_sep &&
        !(path.empty() || has_મૂળ_name(component, style))) {
      // Add a separator.
      path.ધકેલો_back(preferred_separator(style));
    }

    path.append(component.begin(), component.end());
  }
}

શૂન્ય append(SmallVectorImpl<અક્ષર> &path, અચલ Twine &a, અચલ Twine &b,
            અચલ Twine &c, અચલ Twine &d) {
  append(path, Style::native, a, b, c, d);
}

શૂન્ય append(SmallVectorImpl<અક્ષર> &path, const_iterator begin,
            const_iterator end, Style style) {
  માટે (; begin != end; ++begin)
    path::append(path, style, *begin);
}

StringRef parent_path(StringRef path, Style style) {
  size_t end_pos = parent_path_end(path, style);
  જો (end_pos == StringRef::npos)
    પાછા StringRef();
  પાછા path.substr(0, end_pos);
}

શૂન્ય remove_filename(SmallVectorImpl<અક્ષર> &path, Style style) {
  size_t end_pos = parent_path_end(StringRef(path.begin(), path.size()), style);
  જો (end_pos != StringRef::npos)
    path.truncate(end_pos);
}

શૂન્ય replace_extension(SmallVectorImpl<અક્ષર> &path, અચલ Twine &extension,
                       Style style) {
  StringRef p(path.begin(), path.size());
  SmallString<32> ext_storage;
  StringRef ext = extension.toStringRef(ext_storage);

  // Erase existing extension.
  size_t pos = p.find_last_of('.');
  જો (pos != StringRef::npos && pos >= filename_pos(p, style))
    path.truncate(pos);

  // Append '.' if needed.
  જો (ext.size() > 0 && ext[0] != '.')
    path.ધકેલો_back('.');

  // Append extension.
  path.append(ext.begin(), ext.end());
}

સ્થિર તાર્કિક starts_with(StringRef Path, StringRef Prefix,
                        Style style = Style::native) {
  // Windows prefix matching : case and separator insensitive
  જો (is_style_windows(style)) {
    જો (Path.size() < Prefix.size())
      પાછા ખોટું;
    માટે (size_t I = 0, E = Prefix.size(); I != E; ++I) {
      તાર્કિક SepPath = is_separator(Path[I], style);
      તાર્કિક SepPrefix = is_separator(Prefix[I], style);
      જો (SepPath != SepPrefix)
        પાછા ખોટું;
      જો (!SepPath && toLower(Path[I]) != toLower(Prefix[I]))
        પાછા ખોટું;
    }
    પાછા સાચું;
  }
  પાછા Path.starts_with(Prefix);
}

તાર્કિક replace_path_prefix(SmallVectorImpl<અક્ષર> &Path, StringRef OldPrefix,
                         StringRef NewPrefix, Style style) {
  જો (OldPrefix.empty() && NewPrefix.empty())
    પાછા ખોટું;

  StringRef OrigPath(Path.begin(), Path.size());
  જો (!starts_with(OrigPath, OldPrefix, style))
    પાછા ખોટું;

  // If prefixes have the same size we can simply copy the new one over.
  જો (OldPrefix.size() == NewPrefix.size()) {
    llvm::copy(NewPrefix, Path.begin());
    પાછા સાચું;
  }

  StringRef RelPath = OrigPath.substr(OldPrefix.size());
  SmallString<256> NewPath;
  (Twine(NewPrefix) + RelPath).toVector(NewPath);
  Path.swap(NewPath);
  પાછા સાચું;
}

શૂન્ય native(અચલ Twine &path, SmallVectorImpl<અક્ષર> &result, Style style) {
  પ્રતિજ્ઞા((!path.isSingleStringRef() ||
          path.getSingleStringRef().data() != result.data()) &&
         "path and result are not allowed to overlap!");
  // Clear result.
  result.clear();
  path.toVector(result);
  native(result, style);
}

std::string native(અચલ Twine &path, Style style) {
  SmallString<128> Result;
  native(path, Result, style);
  પાછા std::string(Result);
}

શૂન્ય native(SmallVectorImpl<અક્ષર> &Path, Style style) {
  જો (Path.empty())
    પાછા;
  જો (is_style_windows(style)) {
    માટે (અક્ષર &Ch : Path)
      જો (is_separator(Ch, style))
        Ch = preferred_separator(style);
    જો (Path[0] == '~' && (Path.size() == 1 || is_separator(Path[1], style))) {
      SmallString<128> PathHome;
      home_directory(PathHome);
      PathHome.append(Path.begin() + 1, Path.end());
      Path = std::move(PathHome);
    }
  } નહીંતર {
    llvm::replace(Path, '\\', '/');
  }
}

std::string convert_to_slash(StringRef path, Style style) {
  જો (is_style_posix(style))
    પાછા std::string(path);

  std::string s = path.str();
  llvm::replace(s, '\\', '/');
  પાછા s;
}

StringRef filename(StringRef path, Style style) { પાછા *rbegin(path, style); }

StringRef stem(StringRef path, Style style) {
  StringRef fname = filename(path, style);
  size_t pos = fname.find_last_of('.');
  જો (pos == StringRef::npos)
    પાછા fname;
  જો ((fname.size() == 1 && fname == ".") ||
      (fname.size() == 2 && fname == ".."))
    પાછા fname;
  પાછા fname.substr(0, pos);
}

StringRef extension(StringRef path, Style style) {
  StringRef fname = filename(path, style);
  size_t pos = fname.find_last_of('.');
  જો (pos == StringRef::npos)
    પાછા StringRef();
  જો ((fname.size() == 1 && fname == ".") ||
      (fname.size() == 2 && fname == ".."))
    પાછા StringRef();
  પાછા fname.substr(pos);
}

તાર્કિક is_separator(અક્ષર કિંમત_2, Style style) {
  જો (કિંમત_2 == '/')
    પાછા સાચું;
  જો (is_style_windows(style))
    પાછા કિંમત_2 == '\\';
  પાછા ખોટું;
}

StringRef get_separator(Style style) {
  જો (real_style(style) == Style::windows)
    પાછા "\\";
  પાછા "/";
}

તાર્કિક has_મૂળ_name(અચલ Twine &path, Style style) {
  SmallString<128> path_storage;
  StringRef p = path.toStringRef(path_storage);

  પાછા !મૂળ_name(p, style).empty();
}

તાર્કિક has_મૂળ_directory(અચલ Twine &path, Style style) {
  SmallString<128> path_storage;
  StringRef p = path.toStringRef(path_storage);

  પાછા !મૂળ_directory(p, style).empty();
}

તાર્કિક has_મૂળ_path(અચલ Twine &path, Style style) {
  SmallString<128> path_storage;
  StringRef p = path.toStringRef(path_storage);

  પાછા !મૂળ_path(p, style).empty();
}

તાર્કિક has_relative_path(અચલ Twine &path, Style style) {
  SmallString<128> path_storage;
  StringRef p = path.toStringRef(path_storage);

  પાછા !relative_path(p, style).empty();
}

તાર્કિક has_filename(અચલ Twine &path, Style style) {
  SmallString<128> path_storage;
  StringRef p = path.toStringRef(path_storage);

  પાછા !filename(p, style).empty();
}

તાર્કિક has_parent_path(અચલ Twine &path, Style style) {
  SmallString<128> path_storage;
  StringRef p = path.toStringRef(path_storage);

  પાછા !parent_path(p, style).empty();
}

તાર્કિક has_stem(અચલ Twine &path, Style style) {
  SmallString<128> path_storage;
  StringRef p = path.toStringRef(path_storage);

  પાછા !stem(p, style).empty();
}

તાર્કિક has_extension(અચલ Twine &path, Style style) {
  SmallString<128> path_storage;
  StringRef p = path.toStringRef(path_storage);

  પાછા !extension(p, style).empty();
}

તાર્કિક is_absolute(અચલ Twine &path, Style style) {
  SmallString<128> path_storage;
  StringRef p = path.toStringRef(path_storage);

  તાર્કિક મૂળ_Dir = has_મૂળ_directory(p, style);
  તાર્કિક મૂળ_Name = is_style_posix(style) || has_મૂળ_name(p, style);

  પાછા મૂળ_Dir && મૂળ_Name;
}

તાર્કિક is_absolute_gnu(અચલ Twine &path, Style style) {
  SmallString<128> path_storage;
  StringRef p = path.toStringRef(path_storage);

  // Handle '/' which is absolute for both Windows and POSIX systems.
  // Handle '\\' on Windows.
  જો (!p.empty() && is_separator(p.front(), style))
    પાછા સાચું;

  જો (is_style_windows(style)) {
    // Handle drive letter pattern (a character followed by ':') on Windows.
    જો (p.size() >= 2 && (p[0] && p[1] == ':'))
      પાછા સાચું;
  }

  પાછા ખોટું;
}

તાર્કિક is_relative(અચલ Twine &path, Style style) {
  પાછા !is_absolute(path, style);
}

શૂન્ય make_absolute(અચલ Twine &current_directory,
                   SmallVectorImpl<અક્ષર> &path) {
  StringRef p(path.data(), path.size());

  તાર્કિક મૂળ_Directory = has_મૂળ_directory(p);
  તાર્કિક મૂળ_Name = has_મૂળ_name(p);

  // Already absolute.
  જો ((મૂળ_Name || is_style_posix(Style::native)) && મૂળ_Directory)
    પાછા;

  // All the following conditions will need the current directory.
  SmallString<128> current_dir;
  current_directory.toVector(current_dir);

  // Relative path. Prepend the current directory.
  જો (!મૂળ_Name && !મૂળ_Directory) {
    // Append path to the current directory.
    append(current_dir, p);
    // Set path to the result.
    path.swap(current_dir);
    પાછા;
  }

  જો (!મૂળ_Name && મૂળ_Directory) {
    StringRef cdrn = મૂળ_name(current_dir);
    SmallString<128> cur_Dir_મૂળ_Name(cdrn.begin(), cdrn.end());
    append(cur_Dir_મૂળ_Name, p);
    // Set path to the result.
    path.swap(cur_Dir_મૂળ_Name);
    પાછા;
  }

  જો (મૂળ_Name && !મૂળ_Directory) {
    StringRef p_મૂળ_Name = મૂળ_name(p);
    StringRef b_મૂળ_Directory = મૂળ_directory(current_dir);
    StringRef bRelativePath = relative_path(current_dir);
    StringRef pRelativePath = relative_path(p);

    SmallString<128> res;
    append(res, p_મૂળ_Name, b_મૂળ_Directory, bRelativePath, pRelativePath);
    path.swap(res);
    પાછા;
  }

  llvm_unreachable("All rootName and rootDirectory combinations should have "
                   "occurred above!");
}

StringRef remove_leading_dotslash(StringRef Path, Style style) {
  // Remove leading "./" (or ".//" or "././" etc.)
  જ્યારે (Path.size() > 2 && Path[0] == '.' && is_separator(Path[1], style)) {
    Path = Path.substr(2);
    જ્યારે (Path.size() > 0 && is_separator(Path[0], style))
      Path = Path.substr(1);
  }
  પાછા Path;
}

તાર્કિક remove_dots(SmallVectorImpl<અક્ષર> &the_path, તાર્કિક remove_dot_dot,
                 Style style) {
  style = real_style(style);
  StringRef remaining(the_path.data(), the_path.size());
  તાર્કિક needs_change = ખોટું;
  SmallVector<StringRef, 16> components;

  // Consume the root path, if present.
  StringRef મૂળ_3 = path::મૂળ_path(remaining, style);
  તાર્કિક absolute = !મૂળ_3.empty();
  જો (absolute)
    remaining = remaining.drop_front(મૂળ_3.size());

  // Loop over path components manually. This makes it easier to detect
  // non-preferred slashes and double separators that must be canonicalized.
  જ્યારે (!remaining.empty()) {
    size_t next_slash = remaining.find_first_of(separators(style));
    જો (next_slash == StringRef::npos)
      next_slash = remaining.size();
    StringRef component = remaining.take_front(next_slash);
    remaining = remaining.drop_front(next_slash);

    // Eat the slash, and check if it is the preferred separator.
    જો (!remaining.empty()) {
      needs_change |= remaining.front() != preferred_separator(style);
      remaining = remaining.drop_front();
      // The path needs to be rewritten if it has a trailing slash.
      // FIXME: This is emergent behavior that could be removed.
      needs_change |= remaining.empty();
    }

    // Check for path traversal components or double separators.
    જો (component.empty() || component == ".") {
      needs_change = સાચું;
    } નહીંતર જો (remove_dot_dot && component == "..") {
      needs_change = સાચું;
      // Do not allow ".." to remove the root component. If this is the
      // beginning of a relative path, keep the ".." component.
      જો (!components.empty() && components.back() != "..") {
        components.કાઢો_back();
      } નહીંતર જો (!absolute) {
        components.ધકેલો_back(component);
      }
    } નહીંતર {
      components.ધકેલો_back(component);
    }
  }

  SmallString<256> buffer = મૂળ_3;
  // "root" could be "/", which may need to be translated into "\".
  make_preferred(buffer, style);
  needs_change |= મૂળ_3 != buffer;

  // Avoid rewriting the path unless we have to.
  જો (!needs_change)
    પાછા ખોટું;

  જો (!components.empty()) {
    buffer += components[0];
    માટે (StringRef C : ArrayRef(components).drop_front()) {
      buffer += preferred_separator(style);
      buffer += C;
    }
  }
  the_path.swap(buffer);
  પાછા સાચું;
}

} // end namespace path

નામાકાશ fs {

std::error_code getUniqueID(અચલ Twine Path, UniqueID &Result) {
  sandbox::violationIfEnabled();

  file_status Status;
  std::error_code EC = status(Path, Status);
  જો (EC)
    પાછા EC;
  Result = Status.getUniqueID();
  પાછા std::error_code();
}

શૂન્ય createUniquePath(અચલ Twine &Model, SmallVectorImpl<અક્ષર> &ResultPath,
                      તાર્કિક MakeAbsolute) {
  SmallString<128> ModelStorage;
  Model.toVector(ModelStorage);

  પ્રતિજ્ઞા(llvm::is_contained(ModelStorage, '%') &&
         "createUniquePath: Model must contain at least one '%'");

  જો (MakeAbsolute) {
    // Make model absolute by prepending a temp directory if it's not already.
    જો (!sys::path::is_absolute(Twine(ModelStorage))) {
      SmallString<128> TDir;
      sys::path::system_temp_directory(સાચું, TDir);
      sys::path::append(TDir, Twine(ModelStorage));
      ModelStorage.swap(TDir);
    }
  }

  ResultPath = ModelStorage;
  ResultPath.ધકેલો_back(0);
  ResultPath.કાઢો_back();

  // Replace '%' with random chars.
  માટે (unsigned i = 0, e = ModelStorage.size(); i != e; ++i) {
    જો (ModelStorage[i] == '%')
      ResultPath[i] = "0123456789abcdef"[sys::Process::Get_Random_સંખ્યા() & 15];
  }
}

std::error_code createUniqueFile(અચલ Twine &Model, પૂર્ણાંક &ResultFd,
                                 SmallVectorImpl<અક્ષર> &ResultPath,
                                 OpenFlags Flags, unsigned Mode) {
  પાછા createUniqueEntity(Model, ResultFd, ResultPath, ખોટું, FS_File, Flags,
                            Mode);
}

std::error_code createUniqueFile(અચલ Twine &Model,
                                 SmallVectorImpl<અક્ષર> &ResultPath,
                                 unsigned Mode) {
  પૂર્ણાંક FD;
  auto EC = createUniqueFile(Model, FD, ResultPath, OF_None, Mode);
  જો (EC)
    પાછા EC;
  // FD is only needed to avoid race conditions. Close it right away.
  close(FD);
  પાછા EC;
}

સ્થિર std::error_code
createTemporaryFile(અચલ Twine &Model, પૂર્ણાંક &ResultFD,
                    llvm::SmallVectorImpl<અક્ષર> &ResultPath, FSEntity Type,
                    sys::fs::OpenFlags Flags = sys::fs::OF_None) {
  // Any *temporary* file is assumed to be a compiler-internal output, not
  // a formal one.
  auto BypassSandbox = sys::sandbox::scopedDisable();

  SmallString<128> Storage;
  StringRef P = Model.toNullTerminatedStringRef(Storage);
  પ્રતિજ્ઞા(P.find_first_of(separators(Style::native)) == StringRef::npos &&
         "Model must be a simple filename.");
  // Use P.begin() so that createUniqueEntity doesn't need to recreate Storage.
  પાછા createUniqueEntity(P.begin(), ResultFD, ResultPath, સાચું, Type, Flags,
                            all_read | all_write);
}

સ્થિર std::error_code
createTemporaryFile(અચલ Twine &Prefix, StringRef Suffix, પૂર્ણાંક &ResultFD,
                    llvm::SmallVectorImpl<અક્ષર> &ResultPath, FSEntity Type,
                    sys::fs::OpenFlags Flags = sys::fs::OF_None) {
  અચલ અક્ષર *મધ્ય = Suffix.empty() ? "-%%%%%%" : "-%%%%%%.";
  પાછા createTemporaryFile(Prefix + મધ્ય + Suffix, ResultFD, ResultPath,
                             Type, Flags);
}

std::error_code createTemporaryFile(અચલ Twine &Prefix, StringRef Suffix,
                                    પૂર્ણાંક &ResultFD,
                                    SmallVectorImpl<અક્ષર> &ResultPath,
                                    sys::fs::OpenFlags Flags) {
  પાછા createTemporaryFile(Prefix, Suffix, ResultFD, ResultPath, FS_File,
                             Flags);
}

std::error_code createTemporaryFile(અચલ Twine &Prefix, StringRef Suffix,
                                    SmallVectorImpl<અક્ષર> &ResultPath,
                                    sys::fs::OpenFlags Flags) {
  પૂર્ણાંક FD;
  auto EC = createTemporaryFile(Prefix, Suffix, FD, ResultPath, Flags);
  જો (EC)
    પાછા EC;
  // FD is only needed to avoid race conditions. Close it right away.
  close(FD);
  પાછા EC;
}

// This is a mkdtemp with a different pattern. We use createUniqueEntity mostly
// for consistency. We should try using mkdtemp.
std::error_code createUniqueDirectory(અચલ Twine &Prefix,
                                      SmallVectorImpl<અક્ષર> &ResultPath) {
  પૂર્ણાંક Dummy;
  પાછા createUniqueEntity(Prefix + "-%%%%%%", Dummy, ResultPath, સાચું,
                            FS_Dir);
}

std::error_code
getPotentiallyUniqueFileName(અચલ Twine &Model,
                             SmallVectorImpl<અક્ષર> &ResultPath) {
  પૂર્ણાંક Dummy;
  પાછા createUniqueEntity(Model, Dummy, ResultPath, ખોટું, FS_Name);
}

std::error_code
getPotentiallyUniqueTempFileName(અચલ Twine &Prefix, StringRef Suffix,
                                 SmallVectorImpl<અક્ષર> &ResultPath) {
  પૂર્ણાંક Dummy;
  પાછા createTemporaryFile(Prefix, Suffix, Dummy, ResultPath, FS_Name);
}

std::error_code make_absolute(SmallVectorImpl<અક્ષર> &path) {
  sandbox::violationIfEnabled();

  જો (path::is_absolute(path))
    પાછા {};

  SmallString<128> current_dir;
  જો (std::error_code ec = current_path(current_dir))
    પાછા ec;

  path::make_absolute(current_dir, path);
  પાછા {};
}

std::error_code create_directories(અચલ Twine &Path, તાર્કિક IgnoreExisting,
                                   perms Perms) {
  SmallString<128> PathStorage;
  StringRef P = Path.toStringRef(PathStorage);

  // Be optimistic and try to create the directory
  std::error_code EC = create_directory(P, IgnoreExisting, Perms);
  // If we succeeded, or had any error other than the parent not existing, just
  // return it.
  જો (EC != errc::no_such_file_or_directory)
    પાછા EC;

  // We failed because of a no_such_file_or_directory, try to create the
  // parent.
  StringRef Parent = path::parent_path(P);
  જો (Parent.empty())
    પાછા EC;

  જો ((EC = create_directories(Parent, IgnoreExisting, Perms)))
      પાછા EC;

  પાછા create_directory(P, IgnoreExisting, Perms);
}

સ્થિર std::error_code copy_file_internal(પૂર્ણાંક ReadFD, પૂર્ણાંક WriteFD) {
  અચલ size_t BufSize = 4096;
  અક્ષર *Buf = નવું અક્ષર[BufSize];
  પૂર્ણાંક BytesRead = 0, BytesWritten = 0;
  માટે (;;) {
    BytesRead = પઠ(ReadFD, Buf, BufSize);
    જો (BytesRead <= 0)
      તોડો;
    જ્યારે (BytesRead) {
      BytesWritten = લિખ(WriteFD, Buf, BytesRead);
      જો (BytesWritten < 0)
        તોડો;
      BytesRead -= BytesWritten;
    }
    જો (BytesWritten < 0)
      તોડો;
  }
  delete[] Buf;

  જો (BytesRead < 0 || BytesWritten < 0)
    પાછા errnoAsErrorCode();
  પાછા std::error_code();
}

#ifndef __APPLE__
std::error_code copy_file(અચલ Twine &From, અચલ Twine &To) {
  પૂર્ણાંક ReadFD, WriteFD;
  જો (std::error_code EC = openFileForRead(From, ReadFD, OF_None))
    પાછા EC;
  જો (std::error_code EC =
          openFileForWrite(To, WriteFD, CD_CreateAlways, OF_None)) {
    close(ReadFD);
    પાછા EC;
  }

  std::error_code EC = copy_file_internal(ReadFD, WriteFD);

  close(ReadFD);
  close(WriteFD);

  પાછા EC;
}
#endif

std::error_code copy_file(અચલ Twine &From, પૂર્ણાંક ToFD) {
  પૂર્ણાંક ReadFD;
  જો (std::error_code EC = openFileForRead(From, ReadFD, OF_None))
    પાછા EC;

  std::error_code EC = copy_file_internal(ReadFD, ToFD);

  close(ReadFD);

  પાછા EC;
}

ErrorOr<MD5::MD5Result> md5_contents(પૂર્ણાંક FD) {
  sandbox::violationIfEnabled();

  MD5 Hash;

  constexpr size_t BufSize = 4096;
  std::vector<uint8_t> Buf(BufSize);
  પૂર્ણાંક BytesRead = 0;
  માટે (;;) {
    BytesRead = પઠ(FD, Buf.data(), BufSize);
    જો (BytesRead <= 0)
      તોડો;
    Hash.update(ArrayRef(Buf.data(), BytesRead));
  }

  જો (BytesRead < 0)
    પાછા errnoAsErrorCode();
  MD5::MD5Result Result;
  Hash.final(Result);
  પાછા Result;
}

ErrorOr<MD5::MD5Result> md5_contents(અચલ Twine &Path) {
  sandbox::violationIfEnabled();

  પૂર્ણાંક FD;
  જો (auto EC = openFileForRead(Path, FD, OF_None))
    પાછા EC;

  auto Result = md5_contents(FD);
  close(FD);
  પાછા Result;
}

તાર્કિક exists(અચલ basic_file_status &status) {
  પાછા status_known(status) && status.type() != file_type::file_not_found;
}

તાર્કિક status_known(અચલ basic_file_status &s) {
  પાછા s.type() != file_type::status_error;
}

file_type get_file_type(અચલ Twine &Path, તાર્કિક Follow) {
  file_status st;
  જો (status(Path, st, Follow))
    પાછા file_type::status_error;
  પાછા st.type();
}

તાર્કિક is_directory(અચલ basic_file_status &status) {
  પાછા status.type() == file_type::directory_file;
}

std::error_code is_directory(અચલ Twine &path, તાર્કિક &result) {
  sandbox::violationIfEnabled();

  file_status st;
  જો (std::error_code ec = status(path, st))
    પાછા ec;
  result = is_directory(st);
  પાછા std::error_code();
}

તાર્કિક is_regular_file(અચલ basic_file_status &status) {
  પાછા status.type() == file_type::regular_file;
}

std::error_code is_regular_file(અચલ Twine &path, તાર્કિક &result) {
  sandbox::violationIfEnabled();

  file_status st;
  જો (std::error_code ec = status(path, st))
    પાછા ec;
  result = is_regular_file(st);
  પાછા std::error_code();
}

તાર્કિક is_symlink_file(અચલ basic_file_status &status) {
  પાછા status.type() == file_type::symlink_file;
}

std::error_code is_symlink_file(અચલ Twine &path, તાર્કિક &result) {
  sandbox::violationIfEnabled();

  file_status st;
  જો (std::error_code ec = status(path, st, ખોટું))
    પાછા ec;
  result = is_symlink_file(st);
  પાછા std::error_code();
}

તાર્કિક is_other(અચલ basic_file_status &status) {
  પાછા exists(status) &&
         !is_regular_file(status) &&
         !is_directory(status);
}

std::error_code is_other(અચલ Twine &Path, તાર્કિક &Result) {
  sandbox::violationIfEnabled();

  file_status FileStatus;
  જો (std::error_code EC = status(Path, FileStatus))
    પાછા EC;
  Result = is_other(FileStatus);
  પાછા std::error_code();
}

શૂન્ય directory_entry::replace_filename(અચલ Twine &Filename, file_type Type,
                                       basic_file_status Status) {
  SmallString<128> PathStr = path::parent_path(Path);
  path::append(PathStr, Filename);
  આ->Path = std::string(PathStr);
  આ->Type = Type;
  આ->Status = Status;
}

ErrorOr<perms> getPermissions(અચલ Twine &Path) {
  sandbox::violationIfEnabled();

  file_status Status;
  જો (std::error_code EC = status(Path, Status))
    પાછા EC;

  પાછા Status.permissions();
}

std::error_code setLastAccessAndModificationTime(અચલ Twine &Path,
                                                 TimePoint<> AccessTime,
                                                 TimePoint<> ModificationTime) {
  પૂર્ણાંક FD;
  જો (std::error_code EC = openFile(Path, FD, CD_OpenExisting, FA_Read,
                                    OF_UpdateAttributes | OF_OpenDirectory))
    પાછા EC;
  std::error_code EC =
      setLastAccessAndModificationTime(FD, AccessTime, ModificationTime);
  Process::SafelyCloseFileDescriptor(FD);
  પાછા EC;
}

size_t mapped_file_region::size() અચલ {
  પ્રતિજ્ઞા(Mapping && "Mapping failed but used anyway!");
  પાછા Size;
}

અક્ષર *mapped_file_region::data() અચલ {
  પ્રતિજ્ઞા(Mapping && "Mapping failed but used anyway!");
  પાછા reinterpret_cast<અક્ષર *>(Mapping);
}

અચલ અક્ષર *mapped_file_region::const_data() અચલ {
  પ્રતિજ્ઞા(Mapping && "Mapping failed but used anyway!");
  પાછા reinterpret_cast<અચલ અક્ષર *>(Mapping);
}

Error readNativeFileToEOF(file_t FileHandle, SmallVectorImpl<અક્ષર> &Buffer,
                          ssize_t ChunkSize) {
  sandbox::violationIfEnabled();

  // Install a handler to truncate the buffer to the correct size on exit.
  size_t Size = Buffer.size();
  llvm::scope_exit TruncateOnExit([&]() { Buffer.truncate(Size); });

  // Read into Buffer until we hit EOF.
  માટે (;;) {
    Buffer.resize_for_overwrite(Size + ChunkSize);
    Expected<size_t> ReadBytes = readNativeFile(
        FileHandle, MutableArrayRef(Buffer.begin() + Size, ChunkSize));
    જો (!ReadBytes)
      પાછા ReadBytes.takeError();
    જો (*ReadBytes == 0)
      પાછા Error::success();
    Size += *ReadBytes;
  }
}

} // end namespace fs
} // end namespace sys
} // end namespace llvm

// Include the truly platform-specific parts.
#જો defined(LLVM_ON_UNIX)
#અન્તર્ભાવ "Unix/Path.inc"
#endif
#જો defined(_WIN32)
#અન્તર્ભાવ "Windows/Path.inc"
#endif

નામાકાશ llvm {
નામાકાશ sys {
નામાકાશ fs {

TempFile::TempFile(StringRef Name, પૂર્ણાંક FD)
    : TmpName(std::string(Name)), FD(FD) {}
TempFile::TempFile(TempFile &&Other) { *આ = std::move(Other); }
TempFile &TempFile::operator=(TempFile &&Other) {
  TmpName = std::move(Other.TmpName);
  FD = Other.FD;
  Other.Done = સાચું;
  Other.FD = -1;
#ifdef _WIN32
  RemoveOnClose = Other.RemoveOnClose;
  Other.RemoveOnClose = ખોટું;
#endif
  પાછા *આ;
}

TempFile::~TempFile() { પ્રતિજ્ઞા(Done); }

Error TempFile::discard() {
  Done = સાચું;
  જો (FD != -1 && close(FD) == -1) {
    std::error_code EC = errnoAsErrorCode();
    પાછા errorCodeToError(EC);
  }
  FD = -1;

#ifdef _WIN32
  // On Windows, closing will remove the file, if we set the delete
  // disposition. If not, remove it manually.
  તાર્કિક Remove = RemoveOnClose;
#નહીંતર
  // Always try to remove the file.
  તાર્કિક Remove = સાચું;
#endif
  std::error_code RemoveEC;
  જો (Remove && !TmpName.empty()) {
    RemoveEC = fs::remove(TmpName);
    sys::DontRemoveFileOnSignal(TmpName);
    જો (!RemoveEC)
      TmpName = "";
  } નહીંતર {
    TmpName = "";
  }
  પાછા errorCodeToError(RemoveEC);
}

Error TempFile::keep(અચલ Twine &Name) {
  પ્રતિજ્ઞા(!Done);
  Done = સાચું;
  // Always try to close and rename.
#ifdef _WIN32
  // If we can't cancel the delete don't rename.
  auto H = reinterpret_cast<HANDLE>(_get_osfhandle(FD));
  std::error_code RenameEC =
      RemoveOnClose ? std::error_code() : setDeleteDisposition(H, ખોટું);
  તાર્કિક ShouldDelete = ખોટું;
  જો (!RenameEC) {
    RenameEC = rename_handle(H, Name);
    // If rename failed because it's cross-device, copy instead
    જો (RenameEC ==
      std::error_code(ERROR_NOT_SAME_DEVICE, std::system_category())) {
      RenameEC = copy_file(TmpName, Name);
      ShouldDelete = સાચું;
    }
  }

  // If we can't rename or copy, discard the temporary file.
  જો (RenameEC)
    ShouldDelete = સાચું;
  જો (ShouldDelete) {
    જો (!RemoveOnClose)
      setDeleteDisposition(H, સાચું);
    નહીંતર
      remove(TmpName);
  }
#નહીંતર
  std::error_code RenameEC = fs::rename(TmpName, Name);
  જો (RenameEC) {
    // If we can't rename, try to copy to work around cross-device link issues.
    RenameEC = sys::fs::copy_file(TmpName, Name);
    // If we can't rename or copy, discard the temporary file.
    જો (RenameEC)
      remove(TmpName);
  }
#endif
  sys::DontRemoveFileOnSignal(TmpName);

  જો (!RenameEC)
    TmpName = "";

  જો (close(FD) == -1)
    પાછા errorCodeToError(errnoAsErrorCode());
  FD = -1;

  પાછા errorCodeToError(RenameEC);
}

Error TempFile::keep() {
  પ્રતિજ્ઞા(!Done);
  Done = સાચું;

#ifdef _WIN32
  auto H = reinterpret_cast<HANDLE>(_get_osfhandle(FD));
  જો (std::error_code EC = setDeleteDisposition(H, ખોટું))
    પાછા errorCodeToError(EC);
#endif
  sys::DontRemoveFileOnSignal(TmpName);

  TmpName = "";

  જો (close(FD) == -1)
    પાછા errorCodeToError(errnoAsErrorCode());
  FD = -1;

  પાછા Error::success();
}

Expected<TempFile> TempFile::create(અચલ Twine &Model, unsigned Mode,
                                    OpenFlags ExtraFlags) {
  પૂર્ણાંક FD;
  SmallString<128> ResultPath;
  જો (std::error_code EC =
          createUniqueFile(Model, FD, ResultPath, OF_Delete | ExtraFlags, Mode))
    પાછા errorCodeToError(EC);

  TempFile Ret(ResultPath, FD);
#ifdef _WIN32
  auto H = reinterpret_cast<HANDLE>(_get_osfhandle(FD));
  તાર્કિક SetSignalHandler = ખોટું;
  જો (std::error_code EC = setDeleteDisposition(H, સાચું)) {
    Ret.RemoveOnClose = સાચું;
    SetSignalHandler = સાચું;
  }
#નહીંતર
  તાર્કિક SetSignalHandler = સાચું;
#endif
  જો (SetSignalHandler && sys::RemoveFileOnSignal(ResultPath)) {
    // Make sure we delete the file when RemoveFileOnSignal fails.
    consumeError(Ret.discard());
    std::error_code EC(errc::operation_not_permitted);
    પાછા errorCodeToError(EC);
  }
  પાછા std::move(Ret);
}
} // namespace fs

} // namespace sys
} // namespace llvm
