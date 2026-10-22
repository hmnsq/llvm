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

#شامل "llvm/Support/Path.h"
#شامل "llvm/ADT/ArrayRef.h"
#شامل "llvm/ADT/ScopeExit.h"
#شامل "llvm/ADT/StringExtras.h"
#شامل "llvm/Config/config.h"
#شامل "llvm/Config/llvm-config.h"
#شامل "llvm/Support/Errc.h"
#شامل "llvm/Support/ErrorHandling.h"
#شامل "llvm/Support/FileSystem.h"
#شامل "llvm/Support/IOSandbox.h"
#شامل "llvm/Support/Process.h"
#شامل "llvm/Support/Signals.h"
#شامل <cctype>

#اگر !defined(_MSC_VER) && !defined(__MINGW32__)
#شامل <unistd.h>
#وگرنه
#شامل <io.h>
#endif

using فضاینام llvm;
using فضاینام llvm::support::endian;

فضاینام {
  using llvm::StringRef;
  using llvm::sys::path::is_separator;
  using llvm::sys::path::Style;

  درونخطی منطقی prefer_forward_slash() {
    ایستا منطقی prefer = []() {
      اگر (std::optional<std::string> Env =
              sys::Process::GetEnv("LLVM_WINDOWS_PREFER_FORWARD_SLASH"))
        بازگشت *Env == "1";
      بازگشت static_cast<منطقی>(LLVM_WINDOWS_PREFER_FORWARD_SLASH);
    }();
    بازگشت prefer;
  }

  درونخطی Style real_style(Style style) {
    اگر (style != Style::native)
      بازگشت style;
    اگر (is_style_posix(style))
      بازگشت Style::posix;
    بازگشت prefer_forward_slash() ? Style::windows_slash
                                  : Style::windows_backslash;
  }

  درونخطی ثابت نویسه *separators(Style style) {
    اگر (is_style_windows(style))
      بازگشت "\\/";
    بازگشت "/";
  }

  درونخطی نویسه preferred_separator(Style style) {
    اگر (real_style(style) == Style::windows)
      بازگشت '\\';
    بازگشت '/';
  }

  StringRef find_first_component(StringRef path, Style style) {
    // Look for this first component in the following order.
    // * empty (in this case we return an empty string)
    // * either C: or {//,\\}net.
    // * {/,\}
    // * {file,directory}name

    اگر (path.empty())
      بازگشت path;

    اگر (is_style_windows(style)) {
      // C:
      اگر (path.size() >= 2 &&
          std::isalpha(static_cast<unsigned نویسه>(path[0])) && path[1] == ':')
        بازگشت path.substr(0, 2);
    }

    // //net
    اگر ((path.size() > 2) && is_separator(path[0], style) &&
        path[0] == path[1] && !is_separator(path[2], style)) {
      // Find the next directory separator.
      size_t end = path.find_first_of(separators(style), 2);
      بازگشت path.substr(0, end);
    }

    // {/,\}
    اگر (is_separator(path[0], style))
      بازگشت path.substr(0, 1);

    // * {file,directory}name
    size_t end = path.find_first_of(separators(style));
    بازگشت path.substr(0, end);
  }

  // Returns the first character of the filename in str. For paths ending in
  // '/', it returns the position of the '/'.
  size_t filename_pos(StringRef str, Style style) {
    اگر (str.size() > 0 && is_separator(str[str.size() - 1], style))
      بازگشت str.size() - 1;

    size_t pos = str.find_last_of(separators(style), str.size() - 1);

    اگر (is_style_windows(style)) {
      اگر (pos == StringRef::npos)
        pos = str.find_last_of(':', str.size() - 1);
    }

    اگر (pos == StringRef::npos || (pos == 1 && is_separator(str[0], style)))
      بازگشت 0;

    بازگشت pos + 1;
  }

  // Returns the position of the root directory in str. If there is no root
  // directory in str, it returns StringRef::npos.
  size_t ریشه_dir_start(StringRef str, Style style) {
    // case "c:/"
    اگر (is_style_windows(style)) {
      اگر (str.size() > 2 && str[1] == ':' && is_separator(str[2], style))
        بازگشت 2;
    }

    // case "//net"
    اگر (str.size() > 3 && is_separator(str[0], style) && str[0] == str[1] &&
        !is_separator(str[2], style)) {
      بازگشت str.find_first_of(separators(style), 2);
    }

    // case "/"
    اگر (str.size() > 0 && is_separator(str[0], style))
      بازگشت 0;

    بازگشت StringRef::npos;
  }

  // Returns the position past the end of the "parent path" of path. The parent
  // path will not end in '/', unless the parent is the root directory. If the
  // path has no parent, 0 is returned.
  size_t parent_path_end(StringRef path, Style style) {
    size_t end_pos = filename_pos(path, style);

    منطقی filename_was_sep =
        path.size() > 0 && is_separator(path[end_pos], style);

    // Skip separators until we reach root dir (or the start of the string).
    size_t ریشه_dir_pos = ریشه_dir_start(path, style);
    وقتی (end_pos > 0 &&
           (ریشه_dir_pos == StringRef::npos || end_pos > ریشه_dir_pos) &&
           is_separator(path[end_pos - 1], style))
      --end_pos;

    اگر (end_pos == ریشه_dir_pos && !filename_was_sep) {
      // We've reached the root dir and the input path was *not* ending in a
      // sequence of slashes. Include the root dir in the parent path.
      بازگشت ریشه_dir_pos + 1;
    }

    // Otherwise, just include before the last slash.
    بازگشت end_pos;
  }
} // end unnamed namespace

شمارش FSEntity {
  FS_Dir,
  FS_File,
  FS_Name
};

ایستا std::error_code
createUniqueEntity(ثابت Twine &Model, صحیح &ResultFD,
                   SmallVectorImpl<نویسه> &ResultPath, منطقی MakeAbsolute,
                   FSEntity Type, sys::fs::OpenFlags Flags = sys::fs::OF_None,
                   unsigned Mode = 0) {

  // Limit the number of attempts we make, so that we don't infinite loop. E.g.
  // "permission denied" could be for a specific file (so we retry with a
  // different name) or for the whole directory (retry would always fail).
  // Checking which is racy, so we try a number of times, then give up.
  std::error_code EC;
  برای (صحیح Retries = 128; Retries > 0; --Retries) {
    sys::fs::createUniquePath(Model, ResultPath, MakeAbsolute);
    // Try to open + create the file.
    انتخاب (Type) {
    حالت FS_File: {
      EC = sys::fs::openFileForReadWrite(Twine(ResultPath.begin()), ResultFD,
                                         sys::fs::CD_CreateNew, Flags, Mode);
      اگر (EC) {
        // errc::permission_denied happens on Windows when we try to open a file
        // that has been marked for deletion.
        اگر (EC == errc::file_exists || EC == errc::permission_denied)
          ادامه;
        بازگشت EC;
      }

      بازگشت std::error_code();
    }

    حالت FS_Name: {
      EC = sys::fs::access(ResultPath.begin(), sys::fs::AccessMode::Exist);
      اگر (EC == errc::no_such_file_or_directory)
        بازگشت std::error_code();
      اگر (EC)
        بازگشت EC;
      ادامه;
    }

    حالت FS_Dir: {
      EC = sys::fs::create_directory(ResultPath.begin(), نادرست);
      اگر (EC) {
        اگر (EC == errc::file_exists)
          ادامه;
        بازگشت EC;
      }
      بازگشت std::error_code();
    }
    }
    llvm_unreachable("Invalid Type");
  }
  بازگشت EC;
}

فضاینام llvm {
فضاینام sys  {
فضاینام path {

const_iterator begin(StringRef path, Style style) {
  const_iterator i;
  i.Path      = path;
  i.Component = find_first_component(path, style);
  i.Position  = 0;
  i.S = style;
  بازگشت i;
}

const_iterator end(StringRef path) {
  const_iterator i;
  i.Path      = path;
  i.Position  = path.size();
  بازگشت i;
}

const_iterator &const_iterator::operator++() {
  أكد(Position < Path.size() && "Tried to increment past end!");

  // Increment Position to past the current component
  Position += Component.size();

  // Check for end.
  اگر (Position == Path.size()) {
    Component = StringRef();
    بازگشت *این;
  }

  // Both POSIX and Windows treat paths that begin with exactly two separators
  // specially.
  منطقی was_net = Component.size() > 2 && is_separator(Component[0], S) &&
                 Component[1] == Component[0] && !is_separator(Component[2], S);

  // Handle separators.
  اگر (is_separator(Path[Position], S)) {
    // Root dir.
    اگر (was_net ||
        // c:/
        (is_style_windows(S) && Component.ends_with(":"))) {
      Component = Path.substr(Position, 1);
      بازگشت *این;
    }

    // Skip extra separators.
    وقتی (Position != Path.size() && is_separator(Path[Position], S)) {
      ++Position;
    }

    // Treat trailing '/' as a '.', unless it is the root dir.
    اگر (Position == Path.size() && Component != "/") {
      --Position;
      Component = ".";
      بازگشت *این;
    }
  }

  // Find next component.
  size_t end_pos = Path.find_first_of(separators(S), Position);
  Component = Path.slice(Position, end_pos);

  بازگشت *این;
}

منطقی const_iterator::operator==(ثابت const_iterator &RHS) ثابت {
  بازگشت Path.begin() == RHS.Path.begin() && Position == RHS.Position;
}

ptrdiff_t const_iterator::operator-(ثابت const_iterator &RHS) ثابت {
  بازگشت Position - RHS.Position;
}

reverse_iterator rbegin(StringRef Path, Style style) {
  reverse_iterator I;
  I.Path = Path;
  I.Position = Path.size();
  I.S = style;
  ++I;
  بازگشت I;
}

reverse_iterator rend(StringRef Path) {
  reverse_iterator I;
  I.Path = Path;
  I.Component = Path.substr(0, 0);
  I.Position = 0;
  بازگشت I;
}

reverse_iterator &reverse_iterator::operator++() {
  size_t ریشه_dir_pos = ریشه_dir_start(Path, S);

  // Skip separators unless it's the root directory.
  size_t end_pos = Position;
  وقتی (end_pos > 0 && (end_pos - 1) != ریشه_dir_pos &&
         is_separator(Path[end_pos - 1], S))
    --end_pos;

  // Treat trailing '/' as a '.', unless it is the root dir.
  اگر (Position == Path.size() && !Path.empty() &&
      is_separator(Path.back(), S) &&
      (ریشه_dir_pos == StringRef::npos || end_pos - 1 > ریشه_dir_pos)) {
    --Position;
    Component = ".";
    بازگشت *این;
  }

  // Find next separator.
  size_t start_pos = filename_pos(Path.substr(0, end_pos), S);
  Component = Path.slice(start_pos, end_pos);
  Position = start_pos;
  بازگشت *این;
}

منطقی reverse_iterator::operator==(ثابت reverse_iterator &RHS) ثابت {
  بازگشت Path.begin() == RHS.Path.begin() && Component == RHS.Component &&
         Position == RHS.Position;
}

ptrdiff_t reverse_iterator::operator-(ثابت reverse_iterator &RHS) ثابت {
  بازگشت Position - RHS.Position;
}

StringRef ریشه_path(StringRef path, Style style) {
  const_iterator b = begin(path, style), pos = b, e = end(path);
  اگر (b != e) {
    منطقی has_net =
        b->size() > 2 && is_separator((*b)[0], style) && (*b)[1] == (*b)[0];
    منطقی has_drive = is_style_windows(style) && b->ends_with(":");

    اگر (has_net || has_drive) {
      اگر ((++pos != e) && is_separator((*pos)[0], style)) {
        // {C:/,//net/}, so get the first two components.
        بازگشت path.substr(0, b->size() + pos->size());
      }
      // just {C:,//net}, return the first component.
      بازگشت *b;
    }

    // POSIX style root directory.
    اگر (is_separator((*b)[0], style)) {
      بازگشت *b;
    }
  }

  بازگشت StringRef();
}

StringRef ریشه_name(StringRef path, Style style) {
  const_iterator b = begin(path, style), e = end(path);
  اگر (b != e) {
    منطقی has_net =
        b->size() > 2 && is_separator((*b)[0], style) && (*b)[1] == (*b)[0];
    منطقی has_drive = is_style_windows(style) && b->ends_with(":");

    اگر (has_net || has_drive) {
      // just {C:,//net}, return the first component.
      بازگشت *b;
    }
  }

  // No path or no name.
  بازگشت StringRef();
}

StringRef ریشه_directory(StringRef path, Style style) {
  const_iterator b = begin(path, style), pos = b, e = end(path);
  اگر (b != e) {
    منطقی has_net =
        b->size() > 2 && is_separator((*b)[0], style) && (*b)[1] == (*b)[0];
    منطقی has_drive = is_style_windows(style) && b->ends_with(":");

    اگر ((has_net || has_drive) &&
        // {C:,//net}, skip to the next component.
        (++pos != e) && is_separator((*pos)[0], style)) {
      بازگشت *pos;
    }

    // POSIX style root directory.
    اگر (!has_net && is_separator((*b)[0], style)) {
      بازگشت *b;
    }
  }

  // No path or no root.
  بازگشت StringRef();
}

StringRef relative_path(StringRef path, Style style) {
  StringRef ریشه_3 = ریشه_path(path, style);
  بازگشت path.substr(ریشه_3.size());
}

تهی append(SmallVectorImpl<نویسه> &path, Style style, ثابت Twine &a,
            ثابت Twine &b, ثابت Twine &c, ثابت Twine &d) {
  SmallString<32> a_storage;
  SmallString<32> b_storage;
  SmallString<32> c_storage;
  SmallString<32> d_storage;

  SmallVector<StringRef, 4> components;
  اگر (!a.isTriviallyEmpty()) components.افزودن_back(a.toStringRef(a_storage));
  اگر (!b.isTriviallyEmpty()) components.افزودن_back(b.toStringRef(b_storage));
  اگر (!c.isTriviallyEmpty()) components.افزودن_back(c.toStringRef(c_storage));
  اگر (!d.isTriviallyEmpty()) components.افزودن_back(d.toStringRef(d_storage));

  برای (auto &component : components) {
    منطقی path_has_sep =
        !path.empty() && is_separator(path[path.size() - 1], style);
    اگر (path_has_sep) {
      // Strip separators from beginning of component.
      size_t loc = component.find_first_not_of(separators(style));
      StringRef c = component.substr(loc);

      // Append it.
      path.append(c.begin(), c.end());
      ادامه;
    }

    منطقی component_has_sep =
        !component.empty() && is_separator(component[0], style);
    اگر (!component_has_sep &&
        !(path.empty() || has_ریشه_name(component, style))) {
      // Add a separator.
      path.افزودن_back(preferred_separator(style));
    }

    path.append(component.begin(), component.end());
  }
}

تهی append(SmallVectorImpl<نویسه> &path, ثابت Twine &a, ثابت Twine &b,
            ثابت Twine &c, ثابت Twine &d) {
  append(path, Style::native, a, b, c, d);
}

تهی append(SmallVectorImpl<نویسه> &path, const_iterator begin,
            const_iterator end, Style style) {
  برای (; begin != end; ++begin)
    path::append(path, style, *begin);
}

StringRef parent_path(StringRef path, Style style) {
  size_t end_pos = parent_path_end(path, style);
  اگر (end_pos == StringRef::npos)
    بازگشت StringRef();
  بازگشت path.substr(0, end_pos);
}

تهی remove_filename(SmallVectorImpl<نویسه> &path, Style style) {
  size_t end_pos = parent_path_end(StringRef(path.begin(), path.size()), style);
  اگر (end_pos != StringRef::npos)
    path.truncate(end_pos);
}

تهی replace_extension(SmallVectorImpl<نویسه> &path, ثابت Twine &extension,
                       Style style) {
  StringRef p(path.begin(), path.size());
  SmallString<32> ext_storage;
  StringRef ext = extension.toStringRef(ext_storage);

  // Erase existing extension.
  size_t pos = p.find_last_of('.');
  اگر (pos != StringRef::npos && pos >= filename_pos(p, style))
    path.truncate(pos);

  // Append '.' if needed.
  اگر (ext.size() > 0 && ext[0] != '.')
    path.افزودن_back('.');

  // Append extension.
  path.append(ext.begin(), ext.end());
}

ایستا منطقی starts_with(StringRef Path, StringRef Prefix,
                        Style style = Style::native) {
  // Windows prefix matching : case and separator insensitive
  اگر (is_style_windows(style)) {
    اگر (Path.size() < Prefix.size())
      بازگشت نادرست;
    برای (size_t I = 0, E = Prefix.size(); I != E; ++I) {
      منطقی SepPath = is_separator(Path[I], style);
      منطقی SepPrefix = is_separator(Prefix[I], style);
      اگر (SepPath != SepPrefix)
        بازگشت نادرست;
      اگر (!SepPath && toLower(Path[I]) != toLower(Prefix[I]))
        بازگشت نادرست;
    }
    بازگشت درست;
  }
  بازگشت Path.starts_with(Prefix);
}

منطقی replace_path_prefix(SmallVectorImpl<نویسه> &Path, StringRef OldPrefix,
                         StringRef NewPrefix, Style style) {
  اگر (OldPrefix.empty() && NewPrefix.empty())
    بازگشت نادرست;

  StringRef OrigPath(Path.begin(), Path.size());
  اگر (!starts_with(OrigPath, OldPrefix, style))
    بازگشت نادرست;

  // If prefixes have the same size we can simply copy the new one over.
  اگر (OldPrefix.size() == NewPrefix.size()) {
    llvm::copy(NewPrefix, Path.begin());
    بازگشت درست;
  }

  StringRef RelPath = OrigPath.substr(OldPrefix.size());
  SmallString<256> NewPath;
  (Twine(NewPrefix) + RelPath).toVector(NewPath);
  Path.swap(NewPath);
  بازگشت درست;
}

تهی native(ثابت Twine &path, SmallVectorImpl<نویسه> &result, Style style) {
  أكد((!path.isSingleStringRef() ||
          path.getSingleStringRef().data() != result.data()) &&
         "path and result are not allowed to overlap!");
  // Clear result.
  result.clear();
  path.toVector(result);
  native(result, style);
}

std::string native(ثابت Twine &path, Style style) {
  SmallString<128> Result;
  native(path, Result, style);
  بازگشت std::string(Result);
}

تهی native(SmallVectorImpl<نویسه> &Path, Style style) {
  اگر (Path.empty())
    بازگشت;
  اگر (is_style_windows(style)) {
    برای (نویسه &Ch : Path)
      اگر (is_separator(Ch, style))
        Ch = preferred_separator(style);
    اگر (Path[0] == '~' && (Path.size() == 1 || is_separator(Path[1], style))) {
      SmallString<128> PathHome;
      home_directory(PathHome);
      PathHome.append(Path.begin() + 1, Path.end());
      Path = std::move(PathHome);
    }
  } وگرنه {
    llvm::replace(Path, '\\', '/');
  }
}

std::string convert_to_slash(StringRef path, Style style) {
  اگر (is_style_posix(style))
    بازگشت std::string(path);

  std::string s = path.str();
  llvm::replace(s, '\\', '/');
  بازگشت s;
}

StringRef filename(StringRef path, Style style) { بازگشت *rbegin(path, style); }

StringRef stem(StringRef path, Style style) {
  StringRef fname = filename(path, style);
  size_t pos = fname.find_last_of('.');
  اگر (pos == StringRef::npos)
    بازگشت fname;
  اگر ((fname.size() == 1 && fname == ".") ||
      (fname.size() == 2 && fname == ".."))
    بازگشت fname;
  بازگشت fname.substr(0, pos);
}

StringRef extension(StringRef path, Style style) {
  StringRef fname = filename(path, style);
  size_t pos = fname.find_last_of('.');
  اگر (pos == StringRef::npos)
    بازگشت StringRef();
  اگر ((fname.size() == 1 && fname == ".") ||
      (fname.size() == 2 && fname == ".."))
    بازگشت StringRef();
  بازگشت fname.substr(pos);
}

منطقی is_separator(نویسه مقدار_2, Style style) {
  اگر (مقدار_2 == '/')
    بازگشت درست;
  اگر (is_style_windows(style))
    بازگشت مقدار_2 == '\\';
  بازگشت نادرست;
}

StringRef get_separator(Style style) {
  اگر (real_style(style) == Style::windows)
    بازگشت "\\";
  بازگشت "/";
}

منطقی has_ریشه_name(ثابت Twine &path, Style style) {
  SmallString<128> path_storage;
  StringRef p = path.toStringRef(path_storage);

  بازگشت !ریشه_name(p, style).empty();
}

منطقی has_ریشه_directory(ثابت Twine &path, Style style) {
  SmallString<128> path_storage;
  StringRef p = path.toStringRef(path_storage);

  بازگشت !ریشه_directory(p, style).empty();
}

منطقی has_ریشه_path(ثابت Twine &path, Style style) {
  SmallString<128> path_storage;
  StringRef p = path.toStringRef(path_storage);

  بازگشت !ریشه_path(p, style).empty();
}

منطقی has_relative_path(ثابت Twine &path, Style style) {
  SmallString<128> path_storage;
  StringRef p = path.toStringRef(path_storage);

  بازگشت !relative_path(p, style).empty();
}

منطقی has_filename(ثابت Twine &path, Style style) {
  SmallString<128> path_storage;
  StringRef p = path.toStringRef(path_storage);

  بازگشت !filename(p, style).empty();
}

منطقی has_parent_path(ثابت Twine &path, Style style) {
  SmallString<128> path_storage;
  StringRef p = path.toStringRef(path_storage);

  بازگشت !parent_path(p, style).empty();
}

منطقی has_stem(ثابت Twine &path, Style style) {
  SmallString<128> path_storage;
  StringRef p = path.toStringRef(path_storage);

  بازگشت !stem(p, style).empty();
}

منطقی has_extension(ثابت Twine &path, Style style) {
  SmallString<128> path_storage;
  StringRef p = path.toStringRef(path_storage);

  بازگشت !extension(p, style).empty();
}

منطقی is_absolute(ثابت Twine &path, Style style) {
  SmallString<128> path_storage;
  StringRef p = path.toStringRef(path_storage);

  منطقی ریشه_Dir = has_ریشه_directory(p, style);
  منطقی ریشه_Name = is_style_posix(style) || has_ریشه_name(p, style);

  بازگشت ریشه_Dir && ریشه_Name;
}

منطقی is_absolute_gnu(ثابت Twine &path, Style style) {
  SmallString<128> path_storage;
  StringRef p = path.toStringRef(path_storage);

  // Handle '/' which is absolute for both Windows and POSIX systems.
  // Handle '\\' on Windows.
  اگر (!p.empty() && is_separator(p.front(), style))
    بازگشت درست;

  اگر (is_style_windows(style)) {
    // Handle drive letter pattern (a character followed by ':') on Windows.
    اگر (p.size() >= 2 && (p[0] && p[1] == ':'))
      بازگشت درست;
  }

  بازگشت نادرست;
}

منطقی is_relative(ثابت Twine &path, Style style) {
  بازگشت !is_absolute(path, style);
}

تهی make_absolute(ثابت Twine &current_directory,
                   SmallVectorImpl<نویسه> &path) {
  StringRef p(path.data(), path.size());

  منطقی ریشه_Directory = has_ریشه_directory(p);
  منطقی ریشه_Name = has_ریشه_name(p);

  // Already absolute.
  اگر ((ریشه_Name || is_style_posix(Style::native)) && ریشه_Directory)
    بازگشت;

  // All the following conditions will need the current directory.
  SmallString<128> current_dir;
  current_directory.toVector(current_dir);

  // Relative path. Prepend the current directory.
  اگر (!ریشه_Name && !ریشه_Directory) {
    // Append path to the current directory.
    append(current_dir, p);
    // Set path to the result.
    path.swap(current_dir);
    بازگشت;
  }

  اگر (!ریشه_Name && ریشه_Directory) {
    StringRef cdrn = ریشه_name(current_dir);
    SmallString<128> cur_Dir_ریشه_Name(cdrn.begin(), cdrn.end());
    append(cur_Dir_ریشه_Name, p);
    // Set path to the result.
    path.swap(cur_Dir_ریشه_Name);
    بازگشت;
  }

  اگر (ریشه_Name && !ریشه_Directory) {
    StringRef p_ریشه_Name = ریشه_name(p);
    StringRef b_ریشه_Directory = ریشه_directory(current_dir);
    StringRef bRelativePath = relative_path(current_dir);
    StringRef pRelativePath = relative_path(p);

    SmallString<128> res;
    append(res, p_ریشه_Name, b_ریشه_Directory, bRelativePath, pRelativePath);
    path.swap(res);
    بازگشت;
  }

  llvm_unreachable("All rootName and rootDirectory combinations should have "
                   "occurred above!");
}

StringRef remove_leading_dotslash(StringRef Path, Style style) {
  // Remove leading "./" (or ".//" or "././" etc.)
  وقتی (Path.size() > 2 && Path[0] == '.' && is_separator(Path[1], style)) {
    Path = Path.substr(2);
    وقتی (Path.size() > 0 && is_separator(Path[0], style))
      Path = Path.substr(1);
  }
  بازگشت Path;
}

منطقی remove_dots(SmallVectorImpl<نویسه> &the_path, منطقی remove_dot_dot,
                 Style style) {
  style = real_style(style);
  StringRef remaining(the_path.data(), the_path.size());
  منطقی needs_change = نادرست;
  SmallVector<StringRef, 16> components;

  // Consume the root path, if present.
  StringRef ریشه_3 = path::ریشه_path(remaining, style);
  منطقی absolute = !ریشه_3.empty();
  اگر (absolute)
    remaining = remaining.drop_front(ریشه_3.size());

  // Loop over path components manually. This makes it easier to detect
  // non-preferred slashes and double separators that must be canonicalized.
  وقتی (!remaining.empty()) {
    size_t next_slash = remaining.find_first_of(separators(style));
    اگر (next_slash == StringRef::npos)
      next_slash = remaining.size();
    StringRef component = remaining.take_front(next_slash);
    remaining = remaining.drop_front(next_slash);

    // Eat the slash, and check if it is the preferred separator.
    اگر (!remaining.empty()) {
      needs_change |= remaining.front() != preferred_separator(style);
      remaining = remaining.drop_front();
      // The path needs to be rewritten if it has a trailing slash.
      // FIXME: This is emergent behavior that could be removed.
      needs_change |= remaining.empty();
    }

    // Check for path traversal components or double separators.
    اگر (component.empty() || component == ".") {
      needs_change = درست;
    } وگرنه اگر (remove_dot_dot && component == "..") {
      needs_change = درست;
      // Do not allow ".." to remove the root component. If this is the
      // beginning of a relative path, keep the ".." component.
      اگر (!components.empty() && components.back() != "..") {
        components.برداشت_back();
      } وگرنه اگر (!absolute) {
        components.افزودن_back(component);
      }
    } وگرنه {
      components.افزودن_back(component);
    }
  }

  SmallString<256> buffer = ریشه_3;
  // "root" could be "/", which may need to be translated into "\".
  make_preferred(buffer, style);
  needs_change |= ریشه_3 != buffer;

  // Avoid rewriting the path unless we have to.
  اگر (!needs_change)
    بازگشت نادرست;

  اگر (!components.empty()) {
    buffer += components[0];
    برای (StringRef C : ArrayRef(components).drop_front()) {
      buffer += preferred_separator(style);
      buffer += C;
    }
  }
  the_path.swap(buffer);
  بازگشت درست;
}

} // end namespace path

فضاینام fs {

std::error_code getUniqueID(ثابت Twine Path, UniqueID &Result) {
  sandbox::violationIfEnabled();

  file_status Status;
  std::error_code EC = status(Path, Status);
  اگر (EC)
    بازگشت EC;
  Result = Status.getUniqueID();
  بازگشت std::error_code();
}

تهی createUniquePath(ثابت Twine &Model, SmallVectorImpl<نویسه> &ResultPath,
                      منطقی MakeAbsolute) {
  SmallString<128> ModelStorage;
  Model.toVector(ModelStorage);

  أكد(llvm::is_contained(ModelStorage, '%') &&
         "createUniquePath: Model must contain at least one '%'");

  اگر (MakeAbsolute) {
    // Make model absolute by prepending a temp directory if it's not already.
    اگر (!sys::path::is_absolute(Twine(ModelStorage))) {
      SmallString<128> TDir;
      sys::path::system_temp_directory(درست, TDir);
      sys::path::append(TDir, Twine(ModelStorage));
      ModelStorage.swap(TDir);
    }
  }

  ResultPath = ModelStorage;
  ResultPath.افزودن_back(0);
  ResultPath.برداشت_back();

  // Replace '%' with random chars.
  برای (unsigned i = 0, e = ModelStorage.size(); i != e; ++i) {
    اگر (ModelStorage[i] == '%')
      ResultPath[i] = "0123456789abcdef"[sys::Process::Get_Random_عدد() & 15];
  }
}

std::error_code createUniqueFile(ثابت Twine &Model, صحیح &ResultFd,
                                 SmallVectorImpl<نویسه> &ResultPath,
                                 OpenFlags Flags, unsigned Mode) {
  بازگشت createUniqueEntity(Model, ResultFd, ResultPath, نادرست, FS_File, Flags,
                            Mode);
}

std::error_code createUniqueFile(ثابت Twine &Model,
                                 SmallVectorImpl<نویسه> &ResultPath,
                                 unsigned Mode) {
  صحیح FD;
  auto EC = createUniqueFile(Model, FD, ResultPath, OF_None, Mode);
  اگر (EC)
    بازگشت EC;
  // FD is only needed to avoid race conditions. Close it right away.
  close(FD);
  بازگشت EC;
}

ایستا std::error_code
createTemporaryFile(ثابت Twine &Model, صحیح &ResultFD,
                    llvm::SmallVectorImpl<نویسه> &ResultPath, FSEntity Type,
                    sys::fs::OpenFlags Flags = sys::fs::OF_None) {
  // Any *temporary* file is assumed to be a compiler-internal output, not
  // a formal one.
  auto BypassSandbox = sys::sandbox::scopedDisable();

  SmallString<128> Storage;
  StringRef P = Model.toNullTerminatedStringRef(Storage);
  أكد(P.find_first_of(separators(Style::native)) == StringRef::npos &&
         "Model must be a simple filename.");
  // Use P.begin() so that createUniqueEntity doesn't need to recreate Storage.
  بازگشت createUniqueEntity(P.begin(), ResultFD, ResultPath, درست, Type, Flags,
                            all_read | all_write);
}

ایستا std::error_code
createTemporaryFile(ثابت Twine &Prefix, StringRef Suffix, صحیح &ResultFD,
                    llvm::SmallVectorImpl<نویسه> &ResultPath, FSEntity Type,
                    sys::fs::OpenFlags Flags = sys::fs::OF_None) {
  ثابت نویسه *میانه = Suffix.empty() ? "-%%%%%%" : "-%%%%%%.";
  بازگشت createTemporaryFile(Prefix + میانه + Suffix, ResultFD, ResultPath,
                             Type, Flags);
}

std::error_code createTemporaryFile(ثابت Twine &Prefix, StringRef Suffix,
                                    صحیح &ResultFD,
                                    SmallVectorImpl<نویسه> &ResultPath,
                                    sys::fs::OpenFlags Flags) {
  بازگشت createTemporaryFile(Prefix, Suffix, ResultFD, ResultPath, FS_File,
                             Flags);
}

std::error_code createTemporaryFile(ثابت Twine &Prefix, StringRef Suffix,
                                    SmallVectorImpl<نویسه> &ResultPath,
                                    sys::fs::OpenFlags Flags) {
  صحیح FD;
  auto EC = createTemporaryFile(Prefix, Suffix, FD, ResultPath, Flags);
  اگر (EC)
    بازگشت EC;
  // FD is only needed to avoid race conditions. Close it right away.
  close(FD);
  بازگشت EC;
}

// This is a mkdtemp with a different pattern. We use createUniqueEntity mostly
// for consistency. We should try using mkdtemp.
std::error_code createUniqueDirectory(ثابت Twine &Prefix,
                                      SmallVectorImpl<نویسه> &ResultPath) {
  صحیح Dummy;
  بازگشت createUniqueEntity(Prefix + "-%%%%%%", Dummy, ResultPath, درست,
                            FS_Dir);
}

std::error_code
getPotentiallyUniqueFileName(ثابت Twine &Model,
                             SmallVectorImpl<نویسه> &ResultPath) {
  صحیح Dummy;
  بازگشت createUniqueEntity(Model, Dummy, ResultPath, نادرست, FS_Name);
}

std::error_code
getPotentiallyUniqueTempFileName(ثابت Twine &Prefix, StringRef Suffix,
                                 SmallVectorImpl<نویسه> &ResultPath) {
  صحیح Dummy;
  بازگشت createTemporaryFile(Prefix, Suffix, Dummy, ResultPath, FS_Name);
}

std::error_code make_absolute(SmallVectorImpl<نویسه> &path) {
  sandbox::violationIfEnabled();

  اگر (path::is_absolute(path))
    بازگشت {};

  SmallString<128> current_dir;
  اگر (std::error_code ec = current_path(current_dir))
    بازگشت ec;

  path::make_absolute(current_dir, path);
  بازگشت {};
}

std::error_code create_directories(ثابت Twine &Path, منطقی IgnoreExisting,
                                   perms Perms) {
  SmallString<128> PathStorage;
  StringRef P = Path.toStringRef(PathStorage);

  // Be optimistic and try to create the directory
  std::error_code EC = create_directory(P, IgnoreExisting, Perms);
  // If we succeeded, or had any error other than the parent not existing, just
  // return it.
  اگر (EC != errc::no_such_file_or_directory)
    بازگشت EC;

  // We failed because of a no_such_file_or_directory, try to create the
  // parent.
  StringRef Parent = path::parent_path(P);
  اگر (Parent.empty())
    بازگشت EC;

  اگر ((EC = create_directories(Parent, IgnoreExisting, Perms)))
      بازگشت EC;

  بازگشت create_directory(P, IgnoreExisting, Perms);
}

ایستا std::error_code copy_file_internal(صحیح ReadFD, صحیح WriteFD) {
  ثابت size_t BufSize = 4096;
  نویسه *Buf = جدید نویسه[BufSize];
  صحیح BytesRead = 0, BytesWritten = 0;
  برای (;;) {
    BytesRead = اقرأ(ReadFD, Buf, BufSize);
    اگر (BytesRead <= 0)
      بشکن;
    وقتی (BytesRead) {
      BytesWritten = اكتب(WriteFD, Buf, BytesRead);
      اگر (BytesWritten < 0)
        بشکن;
      BytesRead -= BytesWritten;
    }
    اگر (BytesWritten < 0)
      بشکن;
  }
  delete[] Buf;

  اگر (BytesRead < 0 || BytesWritten < 0)
    بازگشت errnoAsErrorCode();
  بازگشت std::error_code();
}

#ifndef __APPLE__
std::error_code copy_file(ثابت Twine &From, ثابت Twine &To) {
  صحیح ReadFD, WriteFD;
  اگر (std::error_code EC = openFileForRead(From, ReadFD, OF_None))
    بازگشت EC;
  اگر (std::error_code EC =
          openFileForWrite(To, WriteFD, CD_CreateAlways, OF_None)) {
    close(ReadFD);
    بازگشت EC;
  }

  std::error_code EC = copy_file_internal(ReadFD, WriteFD);

  close(ReadFD);
  close(WriteFD);

  بازگشت EC;
}
#endif

std::error_code copy_file(ثابت Twine &From, صحیح ToFD) {
  صحیح ReadFD;
  اگر (std::error_code EC = openFileForRead(From, ReadFD, OF_None))
    بازگشت EC;

  std::error_code EC = copy_file_internal(ReadFD, ToFD);

  close(ReadFD);

  بازگشت EC;
}

ErrorOr<MD5::MD5Result> md5_contents(صحیح FD) {
  sandbox::violationIfEnabled();

  MD5 Hash;

  constexpr size_t BufSize = 4096;
  std::vector<uint8_t> Buf(BufSize);
  صحیح BytesRead = 0;
  برای (;;) {
    BytesRead = اقرأ(FD, Buf.data(), BufSize);
    اگر (BytesRead <= 0)
      بشکن;
    Hash.update(ArrayRef(Buf.data(), BytesRead));
  }

  اگر (BytesRead < 0)
    بازگشت errnoAsErrorCode();
  MD5::MD5Result Result;
  Hash.final(Result);
  بازگشت Result;
}

ErrorOr<MD5::MD5Result> md5_contents(ثابت Twine &Path) {
  sandbox::violationIfEnabled();

  صحیح FD;
  اگر (auto EC = openFileForRead(Path, FD, OF_None))
    بازگشت EC;

  auto Result = md5_contents(FD);
  close(FD);
  بازگشت Result;
}

منطقی exists(ثابت basic_file_status &status) {
  بازگشت status_known(status) && status.type() != file_type::file_not_found;
}

منطقی status_known(ثابت basic_file_status &s) {
  بازگشت s.type() != file_type::status_error;
}

file_type get_file_type(ثابت Twine &Path, منطقی Follow) {
  file_status st;
  اگر (status(Path, st, Follow))
    بازگشت file_type::status_error;
  بازگشت st.type();
}

منطقی is_directory(ثابت basic_file_status &status) {
  بازگشت status.type() == file_type::directory_file;
}

std::error_code is_directory(ثابت Twine &path, منطقی &result) {
  sandbox::violationIfEnabled();

  file_status st;
  اگر (std::error_code ec = status(path, st))
    بازگشت ec;
  result = is_directory(st);
  بازگشت std::error_code();
}

منطقی is_regular_file(ثابت basic_file_status &status) {
  بازگشت status.type() == file_type::regular_file;
}

std::error_code is_regular_file(ثابت Twine &path, منطقی &result) {
  sandbox::violationIfEnabled();

  file_status st;
  اگر (std::error_code ec = status(path, st))
    بازگشت ec;
  result = is_regular_file(st);
  بازگشت std::error_code();
}

منطقی is_symlink_file(ثابت basic_file_status &status) {
  بازگشت status.type() == file_type::symlink_file;
}

std::error_code is_symlink_file(ثابت Twine &path, منطقی &result) {
  sandbox::violationIfEnabled();

  file_status st;
  اگر (std::error_code ec = status(path, st, نادرست))
    بازگشت ec;
  result = is_symlink_file(st);
  بازگشت std::error_code();
}

منطقی is_other(ثابت basic_file_status &status) {
  بازگشت exists(status) &&
         !is_regular_file(status) &&
         !is_directory(status);
}

std::error_code is_other(ثابت Twine &Path, منطقی &Result) {
  sandbox::violationIfEnabled();

  file_status FileStatus;
  اگر (std::error_code EC = status(Path, FileStatus))
    بازگشت EC;
  Result = is_other(FileStatus);
  بازگشت std::error_code();
}

تهی directory_entry::replace_filename(ثابت Twine &Filename, file_type Type,
                                       basic_file_status Status) {
  SmallString<128> PathStr = path::parent_path(Path);
  path::append(PathStr, Filename);
  این->Path = std::string(PathStr);
  این->Type = Type;
  این->Status = Status;
}

ErrorOr<perms> getPermissions(ثابت Twine &Path) {
  sandbox::violationIfEnabled();

  file_status Status;
  اگر (std::error_code EC = status(Path, Status))
    بازگشت EC;

  بازگشت Status.permissions();
}

std::error_code setLastAccessAndModificationTime(ثابت Twine &Path,
                                                 TimePoint<> AccessTime,
                                                 TimePoint<> ModificationTime) {
  صحیح FD;
  اگر (std::error_code EC = openFile(Path, FD, CD_OpenExisting, FA_Read,
                                    OF_UpdateAttributes | OF_OpenDirectory))
    بازگشت EC;
  std::error_code EC =
      setLastAccessAndModificationTime(FD, AccessTime, ModificationTime);
  Process::SafelyCloseFileDescriptor(FD);
  بازگشت EC;
}

size_t mapped_file_region::size() ثابت {
  أكد(Mapping && "Mapping failed but used anyway!");
  بازگشت Size;
}

نویسه *mapped_file_region::data() ثابت {
  أكد(Mapping && "Mapping failed but used anyway!");
  بازگشت reinterpret_cast<نویسه *>(Mapping);
}

ثابت نویسه *mapped_file_region::const_data() ثابت {
  أكد(Mapping && "Mapping failed but used anyway!");
  بازگشت reinterpret_cast<ثابت نویسه *>(Mapping);
}

Error readNativeFileToEOF(file_t FileHandle, SmallVectorImpl<نویسه> &Buffer,
                          ssize_t ChunkSize) {
  sandbox::violationIfEnabled();

  // Install a handler to truncate the buffer to the correct size on exit.
  size_t Size = Buffer.size();
  llvm::scope_exit TruncateOnExit([&]() { Buffer.truncate(Size); });

  // Read into Buffer until we hit EOF.
  برای (;;) {
    Buffer.resize_for_overwrite(Size + ChunkSize);
    Expected<size_t> ReadBytes = readNativeFile(
        FileHandle, MutableArrayRef(Buffer.begin() + Size, ChunkSize));
    اگر (!ReadBytes)
      بازگشت ReadBytes.takeError();
    اگر (*ReadBytes == 0)
      بازگشت Error::success();
    Size += *ReadBytes;
  }
}

} // end namespace fs
} // end namespace sys
} // end namespace llvm

// Include the truly platform-specific parts.
#اگر defined(LLVM_ON_UNIX)
#شامل "Unix/Path.inc"
#endif
#اگر defined(_WIN32)
#شامل "Windows/Path.inc"
#endif

فضاینام llvm {
فضاینام sys {
فضاینام fs {

TempFile::TempFile(StringRef Name, صحیح FD)
    : TmpName(std::string(Name)), FD(FD) {}
TempFile::TempFile(TempFile &&Other) { *این = std::move(Other); }
TempFile &TempFile::operator=(TempFile &&Other) {
  TmpName = std::move(Other.TmpName);
  FD = Other.FD;
  Other.Done = درست;
  Other.FD = -1;
#ifdef _WIN32
  RemoveOnClose = Other.RemoveOnClose;
  Other.RemoveOnClose = نادرست;
#endif
  بازگشت *این;
}

TempFile::~TempFile() { أكد(Done); }

Error TempFile::discard() {
  Done = درست;
  اگر (FD != -1 && close(FD) == -1) {
    std::error_code EC = errnoAsErrorCode();
    بازگشت errorCodeToError(EC);
  }
  FD = -1;

#ifdef _WIN32
  // On Windows, closing will remove the file, if we set the delete
  // disposition. If not, remove it manually.
  منطقی Remove = RemoveOnClose;
#وگرنه
  // Always try to remove the file.
  منطقی Remove = درست;
#endif
  std::error_code RemoveEC;
  اگر (Remove && !TmpName.empty()) {
    RemoveEC = fs::remove(TmpName);
    sys::DontRemoveFileOnSignal(TmpName);
    اگر (!RemoveEC)
      TmpName = "";
  } وگرنه {
    TmpName = "";
  }
  بازگشت errorCodeToError(RemoveEC);
}

Error TempFile::keep(ثابت Twine &Name) {
  أكد(!Done);
  Done = درست;
  // Always try to close and rename.
#ifdef _WIN32
  // If we can't cancel the delete don't rename.
  auto H = reinterpret_cast<HANDLE>(_get_osfhandle(FD));
  std::error_code RenameEC =
      RemoveOnClose ? std::error_code() : setDeleteDisposition(H, نادرست);
  منطقی ShouldDelete = نادرست;
  اگر (!RenameEC) {
    RenameEC = rename_handle(H, Name);
    // If rename failed because it's cross-device, copy instead
    اگر (RenameEC ==
      std::error_code(ERROR_NOT_SAME_DEVICE, std::system_category())) {
      RenameEC = copy_file(TmpName, Name);
      ShouldDelete = درست;
    }
  }

  // If we can't rename or copy, discard the temporary file.
  اگر (RenameEC)
    ShouldDelete = درست;
  اگر (ShouldDelete) {
    اگر (!RemoveOnClose)
      setDeleteDisposition(H, درست);
    وگرنه
      remove(TmpName);
  }
#وگرنه
  std::error_code RenameEC = fs::rename(TmpName, Name);
  اگر (RenameEC) {
    // If we can't rename, try to copy to work around cross-device link issues.
    RenameEC = sys::fs::copy_file(TmpName, Name);
    // If we can't rename or copy, discard the temporary file.
    اگر (RenameEC)
      remove(TmpName);
  }
#endif
  sys::DontRemoveFileOnSignal(TmpName);

  اگر (!RenameEC)
    TmpName = "";

  اگر (close(FD) == -1)
    بازگشت errorCodeToError(errnoAsErrorCode());
  FD = -1;

  بازگشت errorCodeToError(RenameEC);
}

Error TempFile::keep() {
  أكد(!Done);
  Done = درست;

#ifdef _WIN32
  auto H = reinterpret_cast<HANDLE>(_get_osfhandle(FD));
  اگر (std::error_code EC = setDeleteDisposition(H, نادرست))
    بازگشت errorCodeToError(EC);
#endif
  sys::DontRemoveFileOnSignal(TmpName);

  TmpName = "";

  اگر (close(FD) == -1)
    بازگشت errorCodeToError(errnoAsErrorCode());
  FD = -1;

  بازگشت Error::success();
}

Expected<TempFile> TempFile::create(ثابت Twine &Model, unsigned Mode,
                                    OpenFlags ExtraFlags) {
  صحیح FD;
  SmallString<128> ResultPath;
  اگر (std::error_code EC =
          createUniqueFile(Model, FD, ResultPath, OF_Delete | ExtraFlags, Mode))
    بازگشت errorCodeToError(EC);

  TempFile Ret(ResultPath, FD);
#ifdef _WIN32
  auto H = reinterpret_cast<HANDLE>(_get_osfhandle(FD));
  منطقی SetSignalHandler = نادرست;
  اگر (std::error_code EC = setDeleteDisposition(H, درست)) {
    Ret.RemoveOnClose = درست;
    SetSignalHandler = درست;
  }
#وگرنه
  منطقی SetSignalHandler = درست;
#endif
  اگر (SetSignalHandler && sys::RemoveFileOnSignal(ResultPath)) {
    // Make sure we delete the file when RemoveFileOnSignal fails.
    consumeError(Ret.discard());
    std::error_code EC(errc::operation_not_permitted);
    بازگشت errorCodeToError(EC);
  }
  بازگشت std::move(Ret);
}
} // namespace fs

} // namespace sys
} // namespace llvm
