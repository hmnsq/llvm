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

#ضمن "llvm/Support/Path.h"
#ضمن "llvm/ADT/ArrayRef.h"
#ضمن "llvm/ADT/ScopeExit.h"
#ضمن "llvm/ADT/StringExtras.h"
#ضمن "llvm/Config/config.h"
#ضمن "llvm/Config/llvm-config.h"
#ضمن "llvm/Support/Errc.h"
#ضمن "llvm/Support/ErrorHandling.h"
#ضمن "llvm/Support/FileSystem.h"
#ضمن "llvm/Support/IOSandbox.h"
#ضمن "llvm/Support/Process.h"
#ضمن "llvm/Support/Signals.h"
#ضمن <cctype>

#אם !defined(_MSC_VER) && !defined(__MINGW32__)
#ضمن <unistd.h>
#אחרת
#ضمن <io.h>
#endif

using نطاق llvm;
using نطاق llvm::support::endian;

نطاق {
  using llvm::StringRef;
  using llvm::sys::path::is_separator;
  using llvm::sys::path::Style;

  مضمن منطقي prefer_forward_slash() {
    ساكن منطقي prefer = []() {
      אם (std::optional<std::string> Env =
              sys::Process::GetEnv("LLVM_WINDOWS_PREFER_FORWARD_SLASH"))
        החזר *Env == "1";
      החזר static_cast<منطقي>(LLVM_WINDOWS_PREFER_FORWARD_SLASH);
    }();
    החזר prefer;
  }

  مضمن Style real_style(Style style) {
    אם (style != Style::native)
      החזר style;
    אם (is_style_posix(style))
      החזר Style::posix;
    החזר prefer_forward_slash() ? Style::windows_slash
                                  : Style::windows_backslash;
  }

  مضمن ثابت حرف *separators(Style style) {
    אם (is_style_windows(style))
      החזר "\\/";
    החזר "/";
  }

  مضمن حرف preferred_separator(Style style) {
    אם (real_style(style) == Style::windows)
      החזר '\\';
    החזר '/';
  }

  StringRef find_first_component(StringRef path, Style style) {
    // Look for this first component in the following order.
    // * empty (in this case we return an empty string)
    // * either C: or {//,\\}net.
    // * {/,\}
    // * {file,directory}name

    אם (path.empty())
      החזר path;

    אם (is_style_windows(style)) {
      // C:
      אם (path.size() >= 2 &&
          std::isalpha(static_cast<unsigned حرف>(path[0])) && path[1] == ':')
        החזר path.substr(0, 2);
    }

    // //net
    אם ((path.size() > 2) && is_separator(path[0], style) &&
        path[0] == path[1] && !is_separator(path[2], style)) {
      // Find the next directory separator.
      size_t end = path.find_first_of(separators(style), 2);
      החזר path.substr(0, end);
    }

    // {/,\}
    אם (is_separator(path[0], style))
      החזר path.substr(0, 1);

    // * {file,directory}name
    size_t end = path.find_first_of(separators(style));
    החזר path.substr(0, end);
  }

  // Returns the first character of the filename in str. For paths ending in
  // '/', it returns the position of the '/'.
  size_t filename_pos(StringRef str, Style style) {
    אם (str.size() > 0 && is_separator(str[str.size() - 1], style))
      החזר str.size() - 1;

    size_t pos = str.find_last_of(separators(style), str.size() - 1);

    אם (is_style_windows(style)) {
      אם (pos == StringRef::npos)
        pos = str.find_last_of(':', str.size() - 1);
    }

    אם (pos == StringRef::npos || (pos == 1 && is_separator(str[0], style)))
      החזר 0;

    החזר pos + 1;
  }

  // Returns the position of the root directory in str. If there is no root
  // directory in str, it returns StringRef::npos.
  size_t root_dir_start(StringRef str, Style style) {
    // case "c:/"
    אם (is_style_windows(style)) {
      אם (str.size() > 2 && str[1] == ':' && is_separator(str[2], style))
        החזר 2;
    }

    // case "//net"
    אם (str.size() > 3 && is_separator(str[0], style) && str[0] == str[1] &&
        !is_separator(str[2], style)) {
      החזר str.find_first_of(separators(style), 2);
    }

    // case "/"
    אם (str.size() > 0 && is_separator(str[0], style))
      החזר 0;

    החזר StringRef::npos;
  }

  // Returns the position past the end of the "parent path" of path. The parent
  // path will not end in '/', unless the parent is the root directory. If the
  // path has no parent, 0 is returned.
  size_t parent_path_end(StringRef path, Style style) {
    size_t end_pos = filename_pos(path, style);

    منطقي filename_was_sep =
        path.size() > 0 && is_separator(path[end_pos], style);

    // Skip separators until we reach root dir (or the start of the string).
    size_t root_dir_pos = root_dir_start(path, style);
    כלעוד (end_pos > 0 &&
           (root_dir_pos == StringRef::npos || end_pos > root_dir_pos) &&
           is_separator(path[end_pos - 1], style))
      --end_pos;

    אם (end_pos == root_dir_pos && !filename_was_sep) {
      // We've reached the root dir and the input path was *not* ending in a
      // sequence of slashes. Include the root dir in the parent path.
      החזר root_dir_pos + 1;
    }

    // Otherwise, just include before the last slash.
    החזר end_pos;
  }
} // end unnamed namespace

تعداد FSEntity {
  FS_Dir,
  FS_File,
  FS_Name
};

ساكن std::error_code
createUniqueEntity(ثابت Twine &Model, صحيح &ResultFD,
                   SmallVectorImpl<حرف> &ResultPath, منطقي MakeAbsolute,
                   FSEntity Type, sys::fs::OpenFlags Flags = sys::fs::OF_None,
                   unsigned Mode = 0) {

  // Limit the number of attempts we make, so that we don't infinite loop. E.g.
  // "permission denied" could be for a specific file (so we retry with a
  // different name) or for the whole directory (retry would always fail).
  // Checking which is racy, so we try a number of times, then give up.
  std::error_code EC;
  עבור (صحيح Retries = 128; Retries > 0; --Retries) {
    sys::fs::createUniquePath(Model, ResultPath, MakeAbsolute);
    // Try to open + create the file.
    بدل (Type) {
    حالة FS_File: {
      EC = sys::fs::openFileForReadWrite(Twine(ResultPath.begin()), ResultFD,
                                         sys::fs::CD_CreateNew, Flags, Mode);
      אם (EC) {
        // errc::permission_denied happens on Windows when we try to open a file
        // that has been marked for deletion.
        אם (EC == errc::file_exists || EC == errc::permission_denied)
          استمر;
        החזר EC;
      }

      החזר std::error_code();
    }

    حالة FS_Name: {
      EC = sys::fs::access(ResultPath.begin(), sys::fs::AccessMode::Exist);
      אם (EC == errc::no_such_file_or_directory)
        החזר std::error_code();
      אם (EC)
        החזר EC;
      استمر;
    }

    حالة FS_Dir: {
      EC = sys::fs::create_directory(ResultPath.begin(), שקר);
      אם (EC) {
        אם (EC == errc::file_exists)
          استمر;
        החזר EC;
      }
      החזר std::error_code();
    }
    }
    llvm_unreachable("Invalid Type");
  }
  החזר EC;
}

نطاق llvm {
نطاق sys  {
نطاق path {

const_iterator begin(StringRef path, Style style) {
  const_iterator i;
  i.Path      = path;
  i.Component = find_first_component(path, style);
  i.Position  = 0;
  i.S = style;
  החזר i;
}

const_iterator end(StringRef path) {
  const_iterator i;
  i.Path      = path;
  i.Position  = path.size();
  החזר i;
}

const_iterator &const_iterator::operator++() {
  أكد(Position < Path.size() && "Tried to increment past end!");

  // Increment Position to past the current component
  Position += Component.size();

  // Check for end.
  אם (Position == Path.size()) {
    Component = StringRef();
    החזר *هذا;
  }

  // Both POSIX and Windows treat paths that begin with exactly two separators
  // specially.
  منطقي was_net = Component.size() > 2 && is_separator(Component[0], S) &&
                 Component[1] == Component[0] && !is_separator(Component[2], S);

  // Handle separators.
  אם (is_separator(Path[Position], S)) {
    // Root dir.
    אם (was_net ||
        // c:/
        (is_style_windows(S) && Component.ends_with(":"))) {
      Component = Path.substr(Position, 1);
      החזר *هذا;
    }

    // Skip extra separators.
    כלעוד (Position != Path.size() && is_separator(Path[Position], S)) {
      ++Position;
    }

    // Treat trailing '/' as a '.', unless it is the root dir.
    אם (Position == Path.size() && Component != "/") {
      --Position;
      Component = ".";
      החזר *هذا;
    }
  }

  // Find next component.
  size_t end_pos = Path.find_first_of(separators(S), Position);
  Component = Path.slice(Position, end_pos);

  החזר *هذا;
}

منطقي const_iterator::operator==(ثابت const_iterator &RHS) ثابت {
  החזר Path.begin() == RHS.Path.begin() && Position == RHS.Position;
}

ptrdiff_t const_iterator::operator-(ثابت const_iterator &RHS) ثابت {
  החזר Position - RHS.Position;
}

reverse_iterator rbegin(StringRef Path, Style style) {
  reverse_iterator I;
  I.Path = Path;
  I.Position = Path.size();
  I.S = style;
  ++I;
  החזר I;
}

reverse_iterator rend(StringRef Path) {
  reverse_iterator I;
  I.Path = Path;
  I.Component = Path.substr(0, 0);
  I.Position = 0;
  החזר I;
}

reverse_iterator &reverse_iterator::operator++() {
  size_t root_dir_pos = root_dir_start(Path, S);

  // Skip separators unless it's the root directory.
  size_t end_pos = Position;
  כלעוד (end_pos > 0 && (end_pos - 1) != root_dir_pos &&
         is_separator(Path[end_pos - 1], S))
    --end_pos;

  // Treat trailing '/' as a '.', unless it is the root dir.
  אם (Position == Path.size() && !Path.empty() &&
      is_separator(Path.back(), S) &&
      (root_dir_pos == StringRef::npos || end_pos - 1 > root_dir_pos)) {
    --Position;
    Component = ".";
    החזר *هذا;
  }

  // Find next separator.
  size_t start_pos = filename_pos(Path.substr(0, end_pos), S);
  Component = Path.slice(start_pos, end_pos);
  Position = start_pos;
  החזר *هذا;
}

منطقي reverse_iterator::operator==(ثابت reverse_iterator &RHS) ثابت {
  החזר Path.begin() == RHS.Path.begin() && Component == RHS.Component &&
         Position == RHS.Position;
}

ptrdiff_t reverse_iterator::operator-(ثابت reverse_iterator &RHS) ثابت {
  החזר Position - RHS.Position;
}

StringRef root_path(StringRef path, Style style) {
  const_iterator b = begin(path, style), pos = b, e = end(path);
  אם (b != e) {
    منطقي has_net =
        b->size() > 2 && is_separator((*b)[0], style) && (*b)[1] == (*b)[0];
    منطقي has_drive = is_style_windows(style) && b->ends_with(":");

    אם (has_net || has_drive) {
      אם ((++pos != e) && is_separator((*pos)[0], style)) {
        // {C:/,//net/}, so get the first two components.
        החזר path.substr(0, b->size() + pos->size());
      }
      // just {C:,//net}, return the first component.
      החזר *b;
    }

    // POSIX style root directory.
    אם (is_separator((*b)[0], style)) {
      החזר *b;
    }
  }

  החזר StringRef();
}

StringRef root_name(StringRef path, Style style) {
  const_iterator b = begin(path, style), e = end(path);
  אם (b != e) {
    منطقي has_net =
        b->size() > 2 && is_separator((*b)[0], style) && (*b)[1] == (*b)[0];
    منطقي has_drive = is_style_windows(style) && b->ends_with(":");

    אם (has_net || has_drive) {
      // just {C:,//net}, return the first component.
      החזר *b;
    }
  }

  // No path or no name.
  החזר StringRef();
}

StringRef root_directory(StringRef path, Style style) {
  const_iterator b = begin(path, style), pos = b, e = end(path);
  אם (b != e) {
    منطقي has_net =
        b->size() > 2 && is_separator((*b)[0], style) && (*b)[1] == (*b)[0];
    منطقي has_drive = is_style_windows(style) && b->ends_with(":");

    אם ((has_net || has_drive) &&
        // {C:,//net}, skip to the next component.
        (++pos != e) && is_separator((*pos)[0], style)) {
      החזר *pos;
    }

    // POSIX style root directory.
    אם (!has_net && is_separator((*b)[0], style)) {
      החזר *b;
    }
  }

  // No path or no root.
  החזר StringRef();
}

StringRef relative_path(StringRef path, Style style) {
  StringRef root = root_path(path, style);
  החזר path.substr(root.size());
}

فراغ append(SmallVectorImpl<حرف> &path, Style style, ثابت Twine &a,
            ثابت Twine &b, ثابت Twine &c, ثابت Twine &d) {
  SmallString<32> a_storage;
  SmallString<32> b_storage;
  SmallString<32> c_storage;
  SmallString<32> d_storage;

  SmallVector<StringRef, 4> components;
  אם (!a.isTriviallyEmpty()) components.push_back(a.toStringRef(a_storage));
  אם (!b.isTriviallyEmpty()) components.push_back(b.toStringRef(b_storage));
  אם (!c.isTriviallyEmpty()) components.push_back(c.toStringRef(c_storage));
  אם (!d.isTriviallyEmpty()) components.push_back(d.toStringRef(d_storage));

  עבור (auto &component : components) {
    منطقي path_has_sep =
        !path.empty() && is_separator(path[path.size() - 1], style);
    אם (path_has_sep) {
      // Strip separators from beginning of component.
      size_t loc = component.find_first_not_of(separators(style));
      StringRef c = component.substr(loc);

      // Append it.
      path.append(c.begin(), c.end());
      استمر;
    }

    منطقي component_has_sep =
        !component.empty() && is_separator(component[0], style);
    אם (!component_has_sep &&
        !(path.empty() || has_root_name(component, style))) {
      // Add a separator.
      path.push_back(preferred_separator(style));
    }

    path.append(component.begin(), component.end());
  }
}

فراغ append(SmallVectorImpl<حرف> &path, ثابت Twine &a, ثابت Twine &b,
            ثابت Twine &c, ثابت Twine &d) {
  append(path, Style::native, a, b, c, d);
}

فراغ append(SmallVectorImpl<حرف> &path, const_iterator begin,
            const_iterator end, Style style) {
  עבור (; begin != end; ++begin)
    path::append(path, style, *begin);
}

StringRef parent_path(StringRef path, Style style) {
  size_t end_pos = parent_path_end(path, style);
  אם (end_pos == StringRef::npos)
    החזר StringRef();
  החזר path.substr(0, end_pos);
}

فراغ remove_filename(SmallVectorImpl<حرف> &path, Style style) {
  size_t end_pos = parent_path_end(StringRef(path.begin(), path.size()), style);
  אם (end_pos != StringRef::npos)
    path.truncate(end_pos);
}

فراغ replace_extension(SmallVectorImpl<حرف> &path, ثابت Twine &extension,
                       Style style) {
  StringRef p(path.begin(), path.size());
  SmallString<32> ext_storage;
  StringRef ext = extension.toStringRef(ext_storage);

  // Erase existing extension.
  size_t pos = p.find_last_of('.');
  אם (pos != StringRef::npos && pos >= filename_pos(p, style))
    path.truncate(pos);

  // Append '.' if needed.
  אם (ext.size() > 0 && ext[0] != '.')
    path.push_back('.');

  // Append extension.
  path.append(ext.begin(), ext.end());
}

ساكن منطقي starts_with(StringRef Path, StringRef Prefix,
                        Style style = Style::native) {
  // Windows prefix matching : case and separator insensitive
  אם (is_style_windows(style)) {
    אם (Path.size() < Prefix.size())
      החזר שקר;
    עבור (size_t I = 0, E = Prefix.size(); I != E; ++I) {
      منطقي SepPath = is_separator(Path[I], style);
      منطقي SepPrefix = is_separator(Prefix[I], style);
      אם (SepPath != SepPrefix)
        החזר שקר;
      אם (!SepPath && toLower(Path[I]) != toLower(Prefix[I]))
        החזר שקר;
    }
    החזר אמת;
  }
  החזר Path.starts_with(Prefix);
}

منطقي replace_path_prefix(SmallVectorImpl<حرف> &Path, StringRef OldPrefix,
                         StringRef NewPrefix, Style style) {
  אם (OldPrefix.empty() && NewPrefix.empty())
    החזר שקר;

  StringRef OrigPath(Path.begin(), Path.size());
  אם (!starts_with(OrigPath, OldPrefix, style))
    החזר שקר;

  // If prefixes have the same size we can simply copy the new one over.
  אם (OldPrefix.size() == NewPrefix.size()) {
    llvm::copy(NewPrefix, Path.begin());
    החזר אמת;
  }

  StringRef RelPath = OrigPath.substr(OldPrefix.size());
  SmallString<256> NewPath;
  (Twine(NewPrefix) + RelPath).toVector(NewPath);
  Path.swap(NewPath);
  החזר אמת;
}

فراغ native(ثابت Twine &path, SmallVectorImpl<حرف> &result, Style style) {
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
  החזר std::string(Result);
}

فراغ native(SmallVectorImpl<حرف> &Path, Style style) {
  אם (Path.empty())
    החזר;
  אם (is_style_windows(style)) {
    עבור (حرف &Ch : Path)
      אם (is_separator(Ch, style))
        Ch = preferred_separator(style);
    אם (Path[0] == '~' && (Path.size() == 1 || is_separator(Path[1], style))) {
      SmallString<128> PathHome;
      home_directory(PathHome);
      PathHome.append(Path.begin() + 1, Path.end());
      Path = std::move(PathHome);
    }
  } אחרת {
    llvm::replace(Path, '\\', '/');
  }
}

std::string convert_to_slash(StringRef path, Style style) {
  אם (is_style_posix(style))
    החזר std::string(path);

  std::string s = path.str();
  llvm::replace(s, '\\', '/');
  החזר s;
}

StringRef filename(StringRef path, Style style) { החזר *rbegin(path, style); }

StringRef stem(StringRef path, Style style) {
  StringRef fname = filename(path, style);
  size_t pos = fname.find_last_of('.');
  אם (pos == StringRef::npos)
    החזר fname;
  אם ((fname.size() == 1 && fname == ".") ||
      (fname.size() == 2 && fname == ".."))
    החזר fname;
  החזר fname.substr(0, pos);
}

StringRef extension(StringRef path, Style style) {
  StringRef fname = filename(path, style);
  size_t pos = fname.find_last_of('.');
  אם (pos == StringRef::npos)
    החזר StringRef();
  אם ((fname.size() == 1 && fname == ".") ||
      (fname.size() == 2 && fname == ".."))
    החזר StringRef();
  החזר fname.substr(pos);
}

منطقي is_separator(حرف value, Style style) {
  אם (value == '/')
    החזר אמת;
  אם (is_style_windows(style))
    החזר value == '\\';
  החזר שקר;
}

StringRef get_separator(Style style) {
  אם (real_style(style) == Style::windows)
    החזר "\\";
  החזר "/";
}

منطقي has_root_name(ثابت Twine &path, Style style) {
  SmallString<128> path_storage;
  StringRef p = path.toStringRef(path_storage);

  החזר !root_name(p, style).empty();
}

منطقي has_root_directory(ثابت Twine &path, Style style) {
  SmallString<128> path_storage;
  StringRef p = path.toStringRef(path_storage);

  החזר !root_directory(p, style).empty();
}

منطقي has_root_path(ثابت Twine &path, Style style) {
  SmallString<128> path_storage;
  StringRef p = path.toStringRef(path_storage);

  החזר !root_path(p, style).empty();
}

منطقي has_relative_path(ثابت Twine &path, Style style) {
  SmallString<128> path_storage;
  StringRef p = path.toStringRef(path_storage);

  החזר !relative_path(p, style).empty();
}

منطقي has_filename(ثابت Twine &path, Style style) {
  SmallString<128> path_storage;
  StringRef p = path.toStringRef(path_storage);

  החזר !filename(p, style).empty();
}

منطقي has_parent_path(ثابت Twine &path, Style style) {
  SmallString<128> path_storage;
  StringRef p = path.toStringRef(path_storage);

  החזר !parent_path(p, style).empty();
}

منطقي has_stem(ثابت Twine &path, Style style) {
  SmallString<128> path_storage;
  StringRef p = path.toStringRef(path_storage);

  החזר !stem(p, style).empty();
}

منطقي has_extension(ثابت Twine &path, Style style) {
  SmallString<128> path_storage;
  StringRef p = path.toStringRef(path_storage);

  החזר !extension(p, style).empty();
}

منطقي is_absolute(ثابت Twine &path, Style style) {
  SmallString<128> path_storage;
  StringRef p = path.toStringRef(path_storage);

  منطقي rootDir = has_root_directory(p, style);
  منطقي rootName = is_style_posix(style) || has_root_name(p, style);

  החזר rootDir && rootName;
}

منطقي is_absolute_gnu(ثابت Twine &path, Style style) {
  SmallString<128> path_storage;
  StringRef p = path.toStringRef(path_storage);

  // Handle '/' which is absolute for both Windows and POSIX systems.
  // Handle '\\' on Windows.
  אם (!p.empty() && is_separator(p.front(), style))
    החזר אמת;

  אם (is_style_windows(style)) {
    // Handle drive letter pattern (a character followed by ':') on Windows.
    אם (p.size() >= 2 && (p[0] && p[1] == ':'))
      החזר אמת;
  }

  החזר שקר;
}

منطقي is_relative(ثابت Twine &path, Style style) {
  החזר !is_absolute(path, style);
}

فراغ make_absolute(ثابت Twine &current_directory,
                   SmallVectorImpl<حرف> &path) {
  StringRef p(path.data(), path.size());

  منطقي rootDirectory = has_root_directory(p);
  منطقي rootName = has_root_name(p);

  // Already absolute.
  אם ((rootName || is_style_posix(Style::native)) && rootDirectory)
    החזר;

  // All the following conditions will need the current directory.
  SmallString<128> current_dir;
  current_directory.toVector(current_dir);

  // Relative path. Prepend the current directory.
  אם (!rootName && !rootDirectory) {
    // Append path to the current directory.
    append(current_dir, p);
    // Set path to the result.
    path.swap(current_dir);
    החזר;
  }

  אם (!rootName && rootDirectory) {
    StringRef cdrn = root_name(current_dir);
    SmallString<128> curDirRootName(cdrn.begin(), cdrn.end());
    append(curDirRootName, p);
    // Set path to the result.
    path.swap(curDirRootName);
    החזר;
  }

  אם (rootName && !rootDirectory) {
    StringRef pRootName = root_name(p);
    StringRef bRootDirectory = root_directory(current_dir);
    StringRef bRelativePath = relative_path(current_dir);
    StringRef pRelativePath = relative_path(p);

    SmallString<128> res;
    append(res, pRootName, bRootDirectory, bRelativePath, pRelativePath);
    path.swap(res);
    החזר;
  }

  llvm_unreachable("All rootName and rootDirectory combinations should have "
                   "occurred above!");
}

StringRef remove_leading_dotslash(StringRef Path, Style style) {
  // Remove leading "./" (or ".//" or "././" etc.)
  כלעוד (Path.size() > 2 && Path[0] == '.' && is_separator(Path[1], style)) {
    Path = Path.substr(2);
    כלעוד (Path.size() > 0 && is_separator(Path[0], style))
      Path = Path.substr(1);
  }
  החזר Path;
}

منطقي remove_dots(SmallVectorImpl<حرف> &the_path, منطقي remove_dot_dot,
                 Style style) {
  style = real_style(style);
  StringRef remaining(the_path.data(), the_path.size());
  منطقي needs_change = שקר;
  SmallVector<StringRef, 16> components;

  // Consume the root path, if present.
  StringRef root = path::root_path(remaining, style);
  منطقي absolute = !root.empty();
  אם (absolute)
    remaining = remaining.drop_front(root.size());

  // Loop over path components manually. This makes it easier to detect
  // non-preferred slashes and double separators that must be canonicalized.
  כלעוד (!remaining.empty()) {
    size_t next_slash = remaining.find_first_of(separators(style));
    אם (next_slash == StringRef::npos)
      next_slash = remaining.size();
    StringRef component = remaining.take_front(next_slash);
    remaining = remaining.drop_front(next_slash);

    // Eat the slash, and check if it is the preferred separator.
    אם (!remaining.empty()) {
      needs_change |= remaining.front() != preferred_separator(style);
      remaining = remaining.drop_front();
      // The path needs to be rewritten if it has a trailing slash.
      // FIXME: This is emergent behavior that could be removed.
      needs_change |= remaining.empty();
    }

    // Check for path traversal components or double separators.
    אם (component.empty() || component == ".") {
      needs_change = אמת;
    } אחרת אם (remove_dot_dot && component == "..") {
      needs_change = אמת;
      // Do not allow ".." to remove the root component. If this is the
      // beginning of a relative path, keep the ".." component.
      אם (!components.empty() && components.back() != "..") {
        components.pop_back();
      } אחרת אם (!absolute) {
        components.push_back(component);
      }
    } אחרת {
      components.push_back(component);
    }
  }

  SmallString<256> buffer = root;
  // "root" could be "/", which may need to be translated into "\".
  make_preferred(buffer, style);
  needs_change |= root != buffer;

  // Avoid rewriting the path unless we have to.
  אם (!needs_change)
    החזר שקר;

  אם (!components.empty()) {
    buffer += components[0];
    עבור (StringRef C : ArrayRef(components).drop_front()) {
      buffer += preferred_separator(style);
      buffer += C;
    }
  }
  the_path.swap(buffer);
  החזר אמת;
}

} // end namespace path

نطاق fs {

std::error_code getUniqueID(ثابت Twine Path, UniqueID &Result) {
  sandbox::violationIfEnabled();

  file_status Status;
  std::error_code EC = status(Path, Status);
  אם (EC)
    החזר EC;
  Result = Status.getUniqueID();
  החזר std::error_code();
}

فراغ createUniquePath(ثابت Twine &Model, SmallVectorImpl<حرف> &ResultPath,
                      منطقي MakeAbsolute) {
  SmallString<128> ModelStorage;
  Model.toVector(ModelStorage);

  أكد(llvm::is_contained(ModelStorage, '%') &&
         "createUniquePath: Model must contain at least one '%'");

  אם (MakeAbsolute) {
    // Make model absolute by prepending a temp directory if it's not already.
    אם (!sys::path::is_absolute(Twine(ModelStorage))) {
      SmallString<128> TDir;
      sys::path::system_temp_directory(אמת, TDir);
      sys::path::append(TDir, Twine(ModelStorage));
      ModelStorage.swap(TDir);
    }
  }

  ResultPath = ModelStorage;
  ResultPath.push_back(0);
  ResultPath.pop_back();

  // Replace '%' with random chars.
  עבור (unsigned i = 0, e = ModelStorage.size(); i != e; ++i) {
    אם (ModelStorage[i] == '%')
      ResultPath[i] = "0123456789abcdef"[sys::Process::GetRandomNumber() & 15];
  }
}

std::error_code createUniqueFile(ثابت Twine &Model, صحيح &ResultFd,
                                 SmallVectorImpl<حرف> &ResultPath,
                                 OpenFlags Flags, unsigned Mode) {
  החזר createUniqueEntity(Model, ResultFd, ResultPath, שקר, FS_File, Flags,
                            Mode);
}

std::error_code createUniqueFile(ثابت Twine &Model,
                                 SmallVectorImpl<حرف> &ResultPath,
                                 unsigned Mode) {
  صحيح FD;
  auto EC = createUniqueFile(Model, FD, ResultPath, OF_None, Mode);
  אם (EC)
    החזר EC;
  // FD is only needed to avoid race conditions. Close it right away.
  close(FD);
  החזר EC;
}

ساكن std::error_code
createTemporaryFile(ثابت Twine &Model, صحيح &ResultFD,
                    llvm::SmallVectorImpl<حرف> &ResultPath, FSEntity Type,
                    sys::fs::OpenFlags Flags = sys::fs::OF_None) {
  // Any *temporary* file is assumed to be a compiler-internal output, not
  // a formal one.
  auto BypassSandbox = sys::sandbox::scopedDisable();

  SmallString<128> Storage;
  StringRef P = Model.toNullTerminatedStringRef(Storage);
  أكد(P.find_first_of(separators(Style::native)) == StringRef::npos &&
         "Model must be a simple filename.");
  // Use P.begin() so that createUniqueEntity doesn't need to recreate Storage.
  החזר createUniqueEntity(P.begin(), ResultFD, ResultPath, אמת, Type, Flags,
                            all_read | all_write);
}

ساكن std::error_code
createTemporaryFile(ثابت Twine &Prefix, StringRef Suffix, صحيح &ResultFD,
                    llvm::SmallVectorImpl<حرف> &ResultPath, FSEntity Type,
                    sys::fs::OpenFlags Flags = sys::fs::OF_None) {
  ثابت حرف *Middle = Suffix.empty() ? "-%%%%%%" : "-%%%%%%.";
  החזר createTemporaryFile(Prefix + Middle + Suffix, ResultFD, ResultPath,
                             Type, Flags);
}

std::error_code createTemporaryFile(ثابت Twine &Prefix, StringRef Suffix,
                                    صحيح &ResultFD,
                                    SmallVectorImpl<حرف> &ResultPath,
                                    sys::fs::OpenFlags Flags) {
  החזר createTemporaryFile(Prefix, Suffix, ResultFD, ResultPath, FS_File,
                             Flags);
}

std::error_code createTemporaryFile(ثابت Twine &Prefix, StringRef Suffix,
                                    SmallVectorImpl<حرف> &ResultPath,
                                    sys::fs::OpenFlags Flags) {
  صحيح FD;
  auto EC = createTemporaryFile(Prefix, Suffix, FD, ResultPath, Flags);
  אם (EC)
    החזר EC;
  // FD is only needed to avoid race conditions. Close it right away.
  close(FD);
  החזר EC;
}

// This is a mkdtemp with a different pattern. We use createUniqueEntity mostly
// for consistency. We should try using mkdtemp.
std::error_code createUniqueDirectory(ثابت Twine &Prefix,
                                      SmallVectorImpl<حرف> &ResultPath) {
  صحيح Dummy;
  החזר createUniqueEntity(Prefix + "-%%%%%%", Dummy, ResultPath, אמת,
                            FS_Dir);
}

std::error_code
getPotentiallyUniqueFileName(ثابت Twine &Model,
                             SmallVectorImpl<حرف> &ResultPath) {
  صحيح Dummy;
  החזר createUniqueEntity(Model, Dummy, ResultPath, שקר, FS_Name);
}

std::error_code
getPotentiallyUniqueTempFileName(ثابت Twine &Prefix, StringRef Suffix,
                                 SmallVectorImpl<حرف> &ResultPath) {
  صحيح Dummy;
  החזר createTemporaryFile(Prefix, Suffix, Dummy, ResultPath, FS_Name);
}

std::error_code make_absolute(SmallVectorImpl<حرف> &path) {
  sandbox::violationIfEnabled();

  אם (path::is_absolute(path))
    החזר {};

  SmallString<128> current_dir;
  אם (std::error_code ec = current_path(current_dir))
    החזר ec;

  path::make_absolute(current_dir, path);
  החזר {};
}

std::error_code create_directories(ثابت Twine &Path, منطقي IgnoreExisting,
                                   perms Perms) {
  SmallString<128> PathStorage;
  StringRef P = Path.toStringRef(PathStorage);

  // Be optimistic and try to create the directory
  std::error_code EC = create_directory(P, IgnoreExisting, Perms);
  // If we succeeded, or had any error other than the parent not existing, just
  // return it.
  אם (EC != errc::no_such_file_or_directory)
    החזר EC;

  // We failed because of a no_such_file_or_directory, try to create the
  // parent.
  StringRef Parent = path::parent_path(P);
  אם (Parent.empty())
    החזר EC;

  אם ((EC = create_directories(Parent, IgnoreExisting, Perms)))
      החזר EC;

  החזר create_directory(P, IgnoreExisting, Perms);
}

ساكن std::error_code copy_file_internal(صحيح ReadFD, صحيح WriteFD) {
  ثابت size_t BufSize = 4096;
  حرف *Buf = جديد حرف[BufSize];
  صحيح BytesRead = 0, BytesWritten = 0;
  עבור (;;) {
    BytesRead = اقرأ(ReadFD, Buf, BufSize);
    אם (BytesRead <= 0)
      اكسر;
    כלעוד (BytesRead) {
      BytesWritten = اكتب(WriteFD, Buf, BytesRead);
      אם (BytesWritten < 0)
        اكسر;
      BytesRead -= BytesWritten;
    }
    אם (BytesWritten < 0)
      اكسر;
  }
  delete[] Buf;

  אם (BytesRead < 0 || BytesWritten < 0)
    החזר errnoAsErrorCode();
  החזר std::error_code();
}

#ifndef __APPLE__
std::error_code copy_file(ثابت Twine &From, ثابت Twine &To) {
  صحيح ReadFD, WriteFD;
  אם (std::error_code EC = openFileForRead(From, ReadFD, OF_None))
    החזר EC;
  אם (std::error_code EC =
          openFileForWrite(To, WriteFD, CD_CreateAlways, OF_None)) {
    close(ReadFD);
    החזר EC;
  }

  std::error_code EC = copy_file_internal(ReadFD, WriteFD);

  close(ReadFD);
  close(WriteFD);

  החזר EC;
}
#endif

std::error_code copy_file(ثابت Twine &From, صحيح ToFD) {
  صحيح ReadFD;
  אם (std::error_code EC = openFileForRead(From, ReadFD, OF_None))
    החזר EC;

  std::error_code EC = copy_file_internal(ReadFD, ToFD);

  close(ReadFD);

  החזר EC;
}

ErrorOr<MD5::MD5Result> md5_contents(صحيح FD) {
  sandbox::violationIfEnabled();

  MD5 Hash;

  constexpr size_t BufSize = 4096;
  std::vector<uint8_t> Buf(BufSize);
  صحيح BytesRead = 0;
  עבור (;;) {
    BytesRead = اقرأ(FD, Buf.data(), BufSize);
    אם (BytesRead <= 0)
      اكسر;
    Hash.update(ArrayRef(Buf.data(), BytesRead));
  }

  אם (BytesRead < 0)
    החזר errnoAsErrorCode();
  MD5::MD5Result Result;
  Hash.final(Result);
  החזר Result;
}

ErrorOr<MD5::MD5Result> md5_contents(ثابت Twine &Path) {
  sandbox::violationIfEnabled();

  صحيح FD;
  אם (auto EC = openFileForRead(Path, FD, OF_None))
    החזר EC;

  auto Result = md5_contents(FD);
  close(FD);
  החזר Result;
}

منطقي exists(ثابت basic_file_status &status) {
  החזר status_known(status) && status.type() != file_type::file_not_found;
}

منطقي status_known(ثابت basic_file_status &s) {
  החזר s.type() != file_type::status_error;
}

file_type get_file_type(ثابت Twine &Path, منطقي Follow) {
  file_status st;
  אם (status(Path, st, Follow))
    החזר file_type::status_error;
  החזר st.type();
}

منطقي is_directory(ثابت basic_file_status &status) {
  החזר status.type() == file_type::directory_file;
}

std::error_code is_directory(ثابت Twine &path, منطقي &result) {
  sandbox::violationIfEnabled();

  file_status st;
  אם (std::error_code ec = status(path, st))
    החזר ec;
  result = is_directory(st);
  החזר std::error_code();
}

منطقي is_regular_file(ثابت basic_file_status &status) {
  החזר status.type() == file_type::regular_file;
}

std::error_code is_regular_file(ثابت Twine &path, منطقي &result) {
  sandbox::violationIfEnabled();

  file_status st;
  אם (std::error_code ec = status(path, st))
    החזר ec;
  result = is_regular_file(st);
  החזר std::error_code();
}

منطقي is_symlink_file(ثابت basic_file_status &status) {
  החזר status.type() == file_type::symlink_file;
}

std::error_code is_symlink_file(ثابت Twine &path, منطقي &result) {
  sandbox::violationIfEnabled();

  file_status st;
  אם (std::error_code ec = status(path, st, שקר))
    החזר ec;
  result = is_symlink_file(st);
  החזר std::error_code();
}

منطقي is_other(ثابت basic_file_status &status) {
  החזר exists(status) &&
         !is_regular_file(status) &&
         !is_directory(status);
}

std::error_code is_other(ثابت Twine &Path, منطقي &Result) {
  sandbox::violationIfEnabled();

  file_status FileStatus;
  אם (std::error_code EC = status(Path, FileStatus))
    החזר EC;
  Result = is_other(FileStatus);
  החזר std::error_code();
}

فراغ directory_entry::replace_filename(ثابت Twine &Filename, file_type Type,
                                       basic_file_status Status) {
  SmallString<128> PathStr = path::parent_path(Path);
  path::append(PathStr, Filename);
  هذا->Path = std::string(PathStr);
  هذا->Type = Type;
  هذا->Status = Status;
}

ErrorOr<perms> getPermissions(ثابت Twine &Path) {
  sandbox::violationIfEnabled();

  file_status Status;
  אם (std::error_code EC = status(Path, Status))
    החזר EC;

  החזר Status.permissions();
}

std::error_code setLastAccessAndModificationTime(ثابت Twine &Path,
                                                 TimePoint<> AccessTime,
                                                 TimePoint<> ModificationTime) {
  صحيح FD;
  אם (std::error_code EC = openFile(Path, FD, CD_OpenExisting, FA_Read,
                                    OF_UpdateAttributes | OF_OpenDirectory))
    החזר EC;
  std::error_code EC =
      setLastAccessAndModificationTime(FD, AccessTime, ModificationTime);
  Process::SafelyCloseFileDescriptor(FD);
  החזר EC;
}

size_t mapped_file_region::size() ثابت {
  أكد(Mapping && "Mapping failed but used anyway!");
  החזר Size;
}

حرف *mapped_file_region::data() ثابت {
  أكد(Mapping && "Mapping failed but used anyway!");
  החזר reinterpret_cast<حرف *>(Mapping);
}

ثابت حرف *mapped_file_region::const_data() ثابت {
  أكد(Mapping && "Mapping failed but used anyway!");
  החזר reinterpret_cast<ثابت حرف *>(Mapping);
}

Error readNativeFileToEOF(file_t FileHandle, SmallVectorImpl<حرف> &Buffer,
                          ssize_t ChunkSize) {
  sandbox::violationIfEnabled();

  // Install a handler to truncate the buffer to the correct size on exit.
  size_t Size = Buffer.size();
  llvm::scope_exit TruncateOnExit([&]() { Buffer.truncate(Size); });

  // Read into Buffer until we hit EOF.
  עבור (;;) {
    Buffer.resize_for_overwrite(Size + ChunkSize);
    Expected<size_t> ReadBytes = readNativeFile(
        FileHandle, MutableArrayRef(Buffer.begin() + Size, ChunkSize));
    אם (!ReadBytes)
      החזר ReadBytes.takeError();
    אם (*ReadBytes == 0)
      החזר Error::success();
    Size += *ReadBytes;
  }
}

} // end namespace fs
} // end namespace sys
} // end namespace llvm

// Include the truly platform-specific parts.
#אם defined(LLVM_ON_UNIX)
#ضمن "Unix/Path.inc"
#endif
#אם defined(_WIN32)
#ضمن "Windows/Path.inc"
#endif

نطاق llvm {
نطاق sys {
نطاق fs {

TempFile::TempFile(StringRef Name, صحيح FD)
    : TmpName(std::string(Name)), FD(FD) {}
TempFile::TempFile(TempFile &&Other) { *هذا = std::move(Other); }
TempFile &TempFile::operator=(TempFile &&Other) {
  TmpName = std::move(Other.TmpName);
  FD = Other.FD;
  Other.Done = אמת;
  Other.FD = -1;
#ifdef _WIN32
  RemoveOnClose = Other.RemoveOnClose;
  Other.RemoveOnClose = שקר;
#endif
  החזר *هذا;
}

TempFile::~TempFile() { أكد(Done); }

Error TempFile::discard() {
  Done = אמת;
  אם (FD != -1 && close(FD) == -1) {
    std::error_code EC = errnoAsErrorCode();
    החזר errorCodeToError(EC);
  }
  FD = -1;

#ifdef _WIN32
  // On Windows, closing will remove the file, if we set the delete
  // disposition. If not, remove it manually.
  منطقي Remove = RemoveOnClose;
#אחרת
  // Always try to remove the file.
  منطقي Remove = אמת;
#endif
  std::error_code RemoveEC;
  אם (Remove && !TmpName.empty()) {
    RemoveEC = fs::remove(TmpName);
    sys::DontRemoveFileOnSignal(TmpName);
    אם (!RemoveEC)
      TmpName = "";
  } אחרת {
    TmpName = "";
  }
  החזר errorCodeToError(RemoveEC);
}

Error TempFile::keep(ثابت Twine &Name) {
  أكد(!Done);
  Done = אמת;
  // Always try to close and rename.
#ifdef _WIN32
  // If we can't cancel the delete don't rename.
  auto H = reinterpret_cast<HANDLE>(_get_osfhandle(FD));
  std::error_code RenameEC =
      RemoveOnClose ? std::error_code() : setDeleteDisposition(H, שקר);
  منطقي ShouldDelete = שקר;
  אם (!RenameEC) {
    RenameEC = rename_handle(H, Name);
    // If rename failed because it's cross-device, copy instead
    אם (RenameEC ==
      std::error_code(ERROR_NOT_SAME_DEVICE, std::system_category())) {
      RenameEC = copy_file(TmpName, Name);
      ShouldDelete = אמת;
    }
  }

  // If we can't rename or copy, discard the temporary file.
  אם (RenameEC)
    ShouldDelete = אמת;
  אם (ShouldDelete) {
    אם (!RemoveOnClose)
      setDeleteDisposition(H, אמת);
    אחרת
      remove(TmpName);
  }
#אחרת
  std::error_code RenameEC = fs::rename(TmpName, Name);
  אם (RenameEC) {
    // If we can't rename, try to copy to work around cross-device link issues.
    RenameEC = sys::fs::copy_file(TmpName, Name);
    // If we can't rename or copy, discard the temporary file.
    אם (RenameEC)
      remove(TmpName);
  }
#endif
  sys::DontRemoveFileOnSignal(TmpName);

  אם (!RenameEC)
    TmpName = "";

  אם (close(FD) == -1)
    החזר errorCodeToError(errnoAsErrorCode());
  FD = -1;

  החזר errorCodeToError(RenameEC);
}

Error TempFile::keep() {
  أكد(!Done);
  Done = אמת;

#ifdef _WIN32
  auto H = reinterpret_cast<HANDLE>(_get_osfhandle(FD));
  אם (std::error_code EC = setDeleteDisposition(H, שקר))
    החזר errorCodeToError(EC);
#endif
  sys::DontRemoveFileOnSignal(TmpName);

  TmpName = "";

  אם (close(FD) == -1)
    החזר errorCodeToError(errnoAsErrorCode());
  FD = -1;

  החזר Error::success();
}

Expected<TempFile> TempFile::create(ثابت Twine &Model, unsigned Mode,
                                    OpenFlags ExtraFlags) {
  صحيح FD;
  SmallString<128> ResultPath;
  אם (std::error_code EC =
          createUniqueFile(Model, FD, ResultPath, OF_Delete | ExtraFlags, Mode))
    החזר errorCodeToError(EC);

  TempFile Ret(ResultPath, FD);
#ifdef _WIN32
  auto H = reinterpret_cast<HANDLE>(_get_osfhandle(FD));
  منطقي SetSignalHandler = שקר;
  אם (std::error_code EC = setDeleteDisposition(H, אמת)) {
    Ret.RemoveOnClose = אמת;
    SetSignalHandler = אמת;
  }
#אחרת
  منطقي SetSignalHandler = אמת;
#endif
  אם (SetSignalHandler && sys::RemoveFileOnSignal(ResultPath)) {
    // Make sure we delete the file when RemoveFileOnSignal fails.
    consumeError(Ret.discard());
    std::error_code EC(errc::operation_not_permitted);
    החזר errorCodeToError(EC);
  }
  החזר std::move(Ret);
}
} // namespace fs

} // namespace sys
} // namespace llvm
