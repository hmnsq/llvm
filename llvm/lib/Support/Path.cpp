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

#include "llvm/Support/Path.h"
#include "llvm/ADT/ArrayRef.h"
#include "llvm/ADT/ScopeExit.h"
#include "llvm/ADT/StringExtras.h"
#include "llvm/Config/config.h"
#include "llvm/Config/llvm-config.h"
#include "llvm/Support/Errc.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/IOSandbox.h"
#include "llvm/Support/Process.h"
#include "llvm/Support/Signals.h"
#include <cctype>

#ပေႃး !defined(_MSC_VER) && !defined(__MINGW32__)
#include <unistd.h>
#ပေႃးဢမ်ႇ
#include <io.h>
#endif

using namespace llvm;
using namespace llvm::support::endian;

namespace {
  using llvm::StringRef;
  using llvm::sys::path::is_separator;
  using llvm::sys::path::Style;

  anta_hstha bool prefer_forward_slash() {
    static bool prefer = []() {
      ပေႃး (std::optional<std::string> Env =
              sys::Process::GetEnv("LLVM_WINDOWS_PREFER_FORWARD_SLASH"))
        ပွၵ်ႈ *Env == "1";
      ပွၵ်ႈ static_cast<bool>(LLVM_WINDOWS_PREFER_FORWARD_SLASH);
    }();
    ပွၵ်ႈ prefer;
  }

  anta_hstha Style real_style(Style style) {
    ပေႃး (style != Style::native)
      ပွၵ်ႈ style;
    ပေႃး (is_style_posix(style))
      ပွၵ်ႈ Style::posix;
    ပွၵ်ႈ prefer_forward_slash() ? Style::windows_slash
                                  : Style::windows_backslash;
  }

  anta_hstha const var_na *separators(Style style) {
    ပေႃး (is_style_windows(style))
      ပွၵ်ႈ "\\/";
    ပွၵ်ႈ "/";
  }

  anta_hstha var_na preferred_separator(Style style) {
    ပေႃး (real_style(style) == Style::windows)
      ပွၵ်ႈ '\\';
    ပွၵ်ႈ '/';
  }

  StringRef find_first_component(StringRef path, Style style) {
    // Look for this first component in the following order.
    // * empty (in this case we return an empty string)
    // * either C: or {//,\\}net.
    // * {/,\}
    // * {file,directory}name

    ပေႃး (path.empty())
      ပွၵ်ႈ path;

    ပေႃး (is_style_windows(style)) {
      // C:
      ပေႃး (path.size() >= 2 &&
          std::isalpha(static_cast<unsigned var_na>(path[0])) && path[1] == ':')
        ပွၵ်ႈ path.substr(0, 2);
    }

    // //net
    ပေႃး ((path.size() > 2) && is_separator(path[0], style) &&
        path[0] == path[1] && !is_separator(path[2], style)) {
      // Find the next directory separator.
      size_t end = path.find_first_of(separators(style), 2);
      ပွၵ်ႈ path.substr(0, end);
    }

    // {/,\}
    ပေႃး (is_separator(path[0], style))
      ပွၵ်ႈ path.substr(0, 1);

    // * {file,directory}name
    size_t end = path.find_first_of(separators(style));
    ပွၵ်ႈ path.substr(0, end);
  }

  // Returns the first character of the filename in str. For paths ending in
  // '/', it returns the position of the '/'.
  size_t filename_pos(StringRef str, Style style) {
    ပေႃး (str.size() > 0 && is_separator(str[str.size() - 1], style))
      ပွၵ်ႈ str.size() - 1;

    size_t pos = str.find_last_of(separators(style), str.size() - 1);

    ပေႃး (is_style_windows(style)) {
      ပေႃး (pos == StringRef::npos)
        pos = str.find_last_of(':', str.size() - 1);
    }

    ပေႃး (pos == StringRef::npos || (pos == 1 && is_separator(str[0], style)))
      ပွၵ်ႈ 0;

    ပွၵ်ႈ pos + 1;
  }

  // Returns the position of the root directory in str. If there is no root
  // directory in str, it returns StringRef::npos.
  size_t root_dir_start(StringRef str, Style style) {
    // case "c:/"
    ပေႃး (is_style_windows(style)) {
      ပေႃး (str.size() > 2 && str[1] == ':' && is_separator(str[2], style))
        ပွၵ်ႈ 2;
    }

    // case "//net"
    ပေႃး (str.size() > 3 && is_separator(str[0], style) && str[0] == str[1] &&
        !is_separator(str[2], style)) {
      ပွၵ်ႈ str.find_first_of(separators(style), 2);
    }

    // case "/"
    ပေႃး (str.size() > 0 && is_separator(str[0], style))
      ပွၵ်ႈ 0;

    ပွၵ်ႈ StringRef::npos;
  }

  // Returns the position past the end of the "parent path" of path. The parent
  // path will not end in '/', unless the parent is the root directory. If the
  // path has no parent, 0 is returned.
  size_t parent_path_end(StringRef path, Style style) {
    size_t end_pos = filename_pos(path, style);

    bool filename_was_sep =
        path.size() > 0 && is_separator(path[end_pos], style);

    // Skip separators until we reach root dir (or the start of the string).
    size_t root_dir_pos = root_dir_start(path, style);
    ၶၢဝ်း (end_pos > 0 &&
           (root_dir_pos == StringRef::npos || end_pos > root_dir_pos) &&
           is_separator(path[end_pos - 1], style))
      --end_pos;

    ပေႃး (end_pos == root_dir_pos && !filename_was_sep) {
      // We've reached the root dir and the input path was *not* ending in a
      // sequence of slashes. Include the root dir in the parent path.
      ပွၵ်ႈ root_dir_pos + 1;
    }

    // Otherwise, just include before the last slash.
    ပွၵ်ႈ end_pos;
  }
} // end unnamed namespace

ga_nanaa FSEntity {
  FS_Dir,
  FS_File,
  FS_Name
};

static std::error_code
createUniqueEntity(const Twine &Model, puur_naa_nka &ResultFD,
                   SmallVectorImpl<var_na> &ResultPath, bool MakeAbsolute,
                   FSEntity Type, sys::fs::OpenFlags Flags = sys::fs::OF_None,
                   unsigned Mode = 0) {

  // Limit the number of attempts we make, so that we don't infinite loop. E.g.
  // "permission denied" could be for a specific file (so we retry with a
  // different name) or for the whole directory (retry would always fail).
  // Checking which is racy, so we try a number of times, then give up.
  std::error_code EC;
  for (puur_naa_nka Retries = 128; Retries > 0; --Retries) {
    sys::fs::createUniquePath(Model, ResultPath, MakeAbsolute);
    // Try to open + create the file.
    switch (Type) {
    pak_sa FS_File: {
      EC = sys::fs::openFileForReadWrite(Twine(ResultPath.begin()), ResultFD,
                                         sys::fs::CD_CreateNew, Flags, Mode);
      ပေႃး (EC) {
        // errc::permission_denied happens on Windows when we try to open a file
        // that has been marked for deletion.
        ပေႃး (EC == errc::file_exists || EC == errc::permission_denied)
          continue;
        ပွၵ်ႈ EC;
      }

      ပွၵ်ႈ std::error_code();
    }

    pak_sa FS_Name: {
      EC = sys::fs::access(ResultPath.begin(), sys::fs::AccessMode::Exist);
      ပေႃး (EC == errc::no_such_file_or_directory)
        ပွၵ်ႈ std::error_code();
      ပေႃး (EC)
        ပွၵ်ႈ EC;
      continue;
    }

    pak_sa FS_Dir: {
      EC = sys::fs::create_directory(ResultPath.begin(), false);
      ပေႃး (EC) {
        ပေႃး (EC == errc::file_exists)
          continue;
        ပွၵ်ႈ EC;
      }
      ပွၵ်ႈ std::error_code();
    }
    }
    llvm_unreachable("Invalid Type");
  }
  ပွၵ်ႈ EC;
}

namespace llvm {
namespace sys  {
namespace path {

const_iterator begin(StringRef path, Style style) {
  const_iterator i;
  i.Path      = path;
  i.Component = find_first_component(path, style);
  i.Position  = 0;
  i.S = style;
  ပွၵ်ႈ i;
}

const_iterator end(StringRef path) {
  const_iterator i;
  i.Path      = path;
  i.Position  = path.size();
  ပွၵ်ႈ i;
}

const_iterator &const_iterator::operator++() {
  assert(Position < Path.size() && "Tried to increment past end!");

  // Increment Position to past the current component
  Position += Component.size();

  // Check for end.
  ပေႃး (Position == Path.size()) {
    Component = StringRef();
    ပွၵ်ႈ *this;
  }

  // Both POSIX and Windows treat paths that begin with exactly two separators
  // specially.
  bool was_net = Component.size() > 2 && is_separator(Component[0], S) &&
                 Component[1] == Component[0] && !is_separator(Component[2], S);

  // Handle separators.
  ပေႃး (is_separator(Path[Position], S)) {
    // Root dir.
    ပေႃး (was_net ||
        // c:/
        (is_style_windows(S) && Component.ends_with(":"))) {
      Component = Path.substr(Position, 1);
      ပွၵ်ႈ *this;
    }

    // Skip extra separators.
    ၶၢဝ်း (Position != Path.size() && is_separator(Path[Position], S)) {
      ++Position;
    }

    // Treat trailing '/' as a '.', unless it is the root dir.
    ပေႃး (Position == Path.size() && Component != "/") {
      --Position;
      Component = ".";
      ပွၵ်ႈ *this;
    }
  }

  // Find next component.
  size_t end_pos = Path.find_first_of(separators(S), Position);
  Component = Path.slice(Position, end_pos);

  ပွၵ်ႈ *this;
}

bool const_iterator::operator==(const const_iterator &RHS) const {
  ပွၵ်ႈ Path.begin() == RHS.Path.begin() && Position == RHS.Position;
}

ptrdiff_t const_iterator::operator-(const const_iterator &RHS) const {
  ပွၵ်ႈ Position - RHS.Position;
}

reverse_iterator rbegin(StringRef Path, Style style) {
  reverse_iterator I;
  I.Path = Path;
  I.Position = Path.size();
  I.S = style;
  ++I;
  ပွၵ်ႈ I;
}

reverse_iterator rend(StringRef Path) {
  reverse_iterator I;
  I.Path = Path;
  I.Component = Path.substr(0, 0);
  I.Position = 0;
  ပွၵ်ႈ I;
}

reverse_iterator &reverse_iterator::operator++() {
  size_t root_dir_pos = root_dir_start(Path, S);

  // Skip separators unless it's the root directory.
  size_t end_pos = Position;
  ၶၢဝ်း (end_pos > 0 && (end_pos - 1) != root_dir_pos &&
         is_separator(Path[end_pos - 1], S))
    --end_pos;

  // Treat trailing '/' as a '.', unless it is the root dir.
  ပေႃး (Position == Path.size() && !Path.empty() &&
      is_separator(Path.back(), S) &&
      (root_dir_pos == StringRef::npos || end_pos - 1 > root_dir_pos)) {
    --Position;
    Component = ".";
    ပွၵ်ႈ *this;
  }

  // Find next separator.
  size_t start_pos = filename_pos(Path.substr(0, end_pos), S);
  Component = Path.slice(start_pos, end_pos);
  Position = start_pos;
  ပွၵ်ႈ *this;
}

bool reverse_iterator::operator==(const reverse_iterator &RHS) const {
  ပွၵ်ႈ Path.begin() == RHS.Path.begin() && Component == RHS.Component &&
         Position == RHS.Position;
}

ptrdiff_t reverse_iterator::operator-(const reverse_iterator &RHS) const {
  ပွၵ်ႈ Position - RHS.Position;
}

StringRef root_path(StringRef path, Style style) {
  const_iterator b = begin(path, style), pos = b, e = end(path);
  ပေႃး (b != e) {
    bool has_net =
        b->size() > 2 && is_separator((*b)[0], style) && (*b)[1] == (*b)[0];
    bool has_drive = is_style_windows(style) && b->ends_with(":");

    ပေႃး (has_net || has_drive) {
      ပေႃး ((++pos != e) && is_separator((*pos)[0], style)) {
        // {C:/,//net/}, so get the first two components.
        ပွၵ်ႈ path.substr(0, b->size() + pos->size());
      }
      // just {C:,//net}, return the first component.
      ပွၵ်ႈ *b;
    }

    // POSIX style root directory.
    ပေႃး (is_separator((*b)[0], style)) {
      ပွၵ်ႈ *b;
    }
  }

  ပွၵ်ႈ StringRef();
}

StringRef root_name(StringRef path, Style style) {
  const_iterator b = begin(path, style), e = end(path);
  ပေႃး (b != e) {
    bool has_net =
        b->size() > 2 && is_separator((*b)[0], style) && (*b)[1] == (*b)[0];
    bool has_drive = is_style_windows(style) && b->ends_with(":");

    ပေႃး (has_net || has_drive) {
      // just {C:,//net}, return the first component.
      ပွၵ်ႈ *b;
    }
  }

  // No path or no name.
  ပွၵ်ႈ StringRef();
}

StringRef root_directory(StringRef path, Style style) {
  const_iterator b = begin(path, style), pos = b, e = end(path);
  ပေႃး (b != e) {
    bool has_net =
        b->size() > 2 && is_separator((*b)[0], style) && (*b)[1] == (*b)[0];
    bool has_drive = is_style_windows(style) && b->ends_with(":");

    ပေႃး ((has_net || has_drive) &&
        // {C:,//net}, skip to the next component.
        (++pos != e) && is_separator((*pos)[0], style)) {
      ပွၵ်ႈ *pos;
    }

    // POSIX style root directory.
    ပေႃး (!has_net && is_separator((*b)[0], style)) {
      ပွၵ်ႈ *b;
    }
  }

  // No path or no root.
  ပွၵ်ႈ StringRef();
}

StringRef relative_path(StringRef path, Style style) {
  StringRef root = root_path(path, style);
  ပွၵ်ႈ path.substr(root.size());
}

void append(SmallVectorImpl<var_na> &path, Style style, const Twine &a,
            const Twine &b, const Twine &c, const Twine &d) {
  SmallString<32> a_storage;
  SmallString<32> b_storage;
  SmallString<32> c_storage;
  SmallString<32> d_storage;

  SmallVector<StringRef, 4> components;
  ပေႃး (!a.isTriviallyEmpty()) components.push_back(a.toStringRef(a_storage));
  ပေႃး (!b.isTriviallyEmpty()) components.push_back(b.toStringRef(b_storage));
  ပေႃး (!c.isTriviallyEmpty()) components.push_back(c.toStringRef(c_storage));
  ပေႃး (!d.isTriviallyEmpty()) components.push_back(d.toStringRef(d_storage));

  for (auto &component : components) {
    bool path_has_sep =
        !path.empty() && is_separator(path[path.size() - 1], style);
    ပေႃး (path_has_sep) {
      // Strip separators from beginning of component.
      size_t loc = component.find_first_not_of(separators(style));
      StringRef c = component.substr(loc);

      // Append it.
      path.append(c.begin(), c.end());
      continue;
    }

    bool component_has_sep =
        !component.empty() && is_separator(component[0], style);
    ပေႃး (!component_has_sep &&
        !(path.empty() || has_root_name(component, style))) {
      // Add a separator.
      path.push_back(preferred_separator(style));
    }

    path.append(component.begin(), component.end());
  }
}

void append(SmallVectorImpl<var_na> &path, const Twine &a, const Twine &b,
            const Twine &c, const Twine &d) {
  append(path, Style::native, a, b, c, d);
}

void append(SmallVectorImpl<var_na> &path, const_iterator begin,
            const_iterator end, Style style) {
  for (; begin != end; ++begin)
    path::append(path, style, *begin);
}

StringRef parent_path(StringRef path, Style style) {
  size_t end_pos = parent_path_end(path, style);
  ပေႃး (end_pos == StringRef::npos)
    ပွၵ်ႈ StringRef();
  ပွၵ်ႈ path.substr(0, end_pos);
}

void remove_filename(SmallVectorImpl<var_na> &path, Style style) {
  size_t end_pos = parent_path_end(StringRef(path.begin(), path.size()), style);
  ပေႃး (end_pos != StringRef::npos)
    path.truncate(end_pos);
}

void replace_extension(SmallVectorImpl<var_na> &path, const Twine &extension,
                       Style style) {
  StringRef p(path.begin(), path.size());
  SmallString<32> ext_storage;
  StringRef ext = extension.toStringRef(ext_storage);

  // Erase existing extension.
  size_t pos = p.find_last_of('.');
  ပေႃး (pos != StringRef::npos && pos >= filename_pos(p, style))
    path.truncate(pos);

  // Append '.' if needed.
  ပေႃး (ext.size() > 0 && ext[0] != '.')
    path.push_back('.');

  // Append extension.
  path.append(ext.begin(), ext.end());
}

static bool starts_with(StringRef Path, StringRef Prefix,
                        Style style = Style::native) {
  // Windows prefix matching : case and separator insensitive
  ပေႃး (is_style_windows(style)) {
    ပေႃး (Path.size() < Prefix.size())
      ပွၵ်ႈ false;
    for (size_t I = 0, E = Prefix.size(); I != E; ++I) {
      bool SepPath = is_separator(Path[I], style);
      bool SepPrefix = is_separator(Prefix[I], style);
      ပေႃး (SepPath != SepPrefix)
        ပွၵ်ႈ false;
      ပေႃး (!SepPath && toLower(Path[I]) != toLower(Prefix[I]))
        ပွၵ်ႈ false;
    }
    ပွၵ်ႈ true;
  }
  ပွၵ်ႈ Path.starts_with(Prefix);
}

bool replace_path_prefix(SmallVectorImpl<var_na> &Path, StringRef OldPrefix,
                         StringRef NewPrefix, Style style) {
  ပေႃး (OldPrefix.empty() && NewPrefix.empty())
    ပွၵ်ႈ false;

  StringRef OrigPath(Path.begin(), Path.size());
  ပေႃး (!starts_with(OrigPath, OldPrefix, style))
    ပွၵ်ႈ false;

  // If prefixes have the same size we can simply copy the new one over.
  ပေႃး (OldPrefix.size() == NewPrefix.size()) {
    llvm::copy(NewPrefix, Path.begin());
    ပွၵ်ႈ true;
  }

  StringRef RelPath = OrigPath.substr(OldPrefix.size());
  SmallString<256> NewPath;
  (Twine(NewPrefix) + RelPath).toVector(NewPath);
  Path.swap(NewPath);
  ပွၵ်ႈ true;
}

void native(const Twine &path, SmallVectorImpl<var_na> &result, Style style) {
  assert((!path.isSingleStringRef() ||
          path.getSingleStringRef().data() != result.data()) &&
         "path and result are not allowed to overlap!");
  // Clear result.
  result.clear();
  path.toVector(result);
  native(result, style);
}

std::string native(const Twine &path, Style style) {
  SmallString<128> Result;
  native(path, Result, style);
  ပွၵ်ႈ std::string(Result);
}

void native(SmallVectorImpl<var_na> &Path, Style style) {
  ပေႃး (Path.empty())
    ပွၵ်ႈ;
  ပေႃး (is_style_windows(style)) {
    for (var_na &Ch : Path)
      ပေႃး (is_separator(Ch, style))
        Ch = preferred_separator(style);
    ပေႃး (Path[0] == '~' && (Path.size() == 1 || is_separator(Path[1], style))) {
      SmallString<128> PathHome;
      home_directory(PathHome);
      PathHome.append(Path.begin() + 1, Path.end());
      Path = std::move(PathHome);
    }
  } ပေႃးဢမ်ႇ {
    llvm::replace(Path, '\\', '/');
  }
}

std::string convert_to_slash(StringRef path, Style style) {
  ပေႃး (is_style_posix(style))
    ပွၵ်ႈ std::string(path);

  std::string s = path.str();
  llvm::replace(s, '\\', '/');
  ပွၵ်ႈ s;
}

StringRef filename(StringRef path, Style style) { ပွၵ်ႈ *rbegin(path, style); }

StringRef stem(StringRef path, Style style) {
  StringRef fname = filename(path, style);
  size_t pos = fname.find_last_of('.');
  ပေႃး (pos == StringRef::npos)
    ပွၵ်ႈ fname;
  ပေႃး ((fname.size() == 1 && fname == ".") ||
      (fname.size() == 2 && fname == ".."))
    ပွၵ်ႈ fname;
  ပွၵ်ႈ fname.substr(0, pos);
}

StringRef extension(StringRef path, Style style) {
  StringRef fname = filename(path, style);
  size_t pos = fname.find_last_of('.');
  ပေႃး (pos == StringRef::npos)
    ပွၵ်ႈ StringRef();
  ပေႃး ((fname.size() == 1 && fname == ".") ||
      (fname.size() == 2 && fname == ".."))
    ပွၵ်ႈ StringRef();
  ပွၵ်ႈ fname.substr(pos);
}

bool is_separator(var_na value, Style style) {
  ပေႃး (value == '/')
    ပွၵ်ႈ true;
  ပေႃး (is_style_windows(style))
    ပွၵ်ႈ value == '\\';
  ပွၵ်ႈ false;
}

StringRef get_separator(Style style) {
  ပေႃး (real_style(style) == Style::windows)
    ပွၵ်ႈ "\\";
  ပွၵ်ႈ "/";
}

bool has_root_name(const Twine &path, Style style) {
  SmallString<128> path_storage;
  StringRef p = path.toStringRef(path_storage);

  ပွၵ်ႈ !root_name(p, style).empty();
}

bool has_root_directory(const Twine &path, Style style) {
  SmallString<128> path_storage;
  StringRef p = path.toStringRef(path_storage);

  ပွၵ်ႈ !root_directory(p, style).empty();
}

bool has_root_path(const Twine &path, Style style) {
  SmallString<128> path_storage;
  StringRef p = path.toStringRef(path_storage);

  ပွၵ်ႈ !root_path(p, style).empty();
}

bool has_relative_path(const Twine &path, Style style) {
  SmallString<128> path_storage;
  StringRef p = path.toStringRef(path_storage);

  ပွၵ်ႈ !relative_path(p, style).empty();
}

bool has_filename(const Twine &path, Style style) {
  SmallString<128> path_storage;
  StringRef p = path.toStringRef(path_storage);

  ပွၵ်ႈ !filename(p, style).empty();
}

bool has_parent_path(const Twine &path, Style style) {
  SmallString<128> path_storage;
  StringRef p = path.toStringRef(path_storage);

  ပွၵ်ႈ !parent_path(p, style).empty();
}

bool has_stem(const Twine &path, Style style) {
  SmallString<128> path_storage;
  StringRef p = path.toStringRef(path_storage);

  ပွၵ်ႈ !stem(p, style).empty();
}

bool has_extension(const Twine &path, Style style) {
  SmallString<128> path_storage;
  StringRef p = path.toStringRef(path_storage);

  ပွၵ်ႈ !extension(p, style).empty();
}

bool is_absolute(const Twine &path, Style style) {
  SmallString<128> path_storage;
  StringRef p = path.toStringRef(path_storage);

  bool rootDir = has_root_directory(p, style);
  bool rootName = is_style_posix(style) || has_root_name(p, style);

  ပွၵ်ႈ rootDir && rootName;
}

bool is_absolute_gnu(const Twine &path, Style style) {
  SmallString<128> path_storage;
  StringRef p = path.toStringRef(path_storage);

  // Handle '/' which is absolute for both Windows and POSIX systems.
  // Handle '\\' on Windows.
  ပေႃး (!p.empty() && is_separator(p.front(), style))
    ပွၵ်ႈ true;

  ပေႃး (is_style_windows(style)) {
    // Handle drive letter pattern (a character followed by ':') on Windows.
    ပေႃး (p.size() >= 2 && (p[0] && p[1] == ':'))
      ပွၵ်ႈ true;
  }

  ပွၵ်ႈ false;
}

bool is_relative(const Twine &path, Style style) {
  ပွၵ်ႈ !is_absolute(path, style);
}

void make_absolute(const Twine &current_directory,
                   SmallVectorImpl<var_na> &path) {
  StringRef p(path.data(), path.size());

  bool rootDirectory = has_root_directory(p);
  bool rootName = has_root_name(p);

  // Already absolute.
  ပေႃး ((rootName || is_style_posix(Style::native)) && rootDirectory)
    ပွၵ်ႈ;

  // All the following conditions will need the current directory.
  SmallString<128> current_dir;
  current_directory.toVector(current_dir);

  // Relative path. Prepend the current directory.
  ပေႃး (!rootName && !rootDirectory) {
    // Append path to the current directory.
    append(current_dir, p);
    // Set path to the result.
    path.swap(current_dir);
    ပွၵ်ႈ;
  }

  ပေႃး (!rootName && rootDirectory) {
    StringRef cdrn = root_name(current_dir);
    SmallString<128> curDirRootName(cdrn.begin(), cdrn.end());
    append(curDirRootName, p);
    // Set path to the result.
    path.swap(curDirRootName);
    ပွၵ်ႈ;
  }

  ပေႃး (rootName && !rootDirectory) {
    StringRef pRootName = root_name(p);
    StringRef bRootDirectory = root_directory(current_dir);
    StringRef bRelativePath = relative_path(current_dir);
    StringRef pRelativePath = relative_path(p);

    SmallString<128> res;
    append(res, pRootName, bRootDirectory, bRelativePath, pRelativePath);
    path.swap(res);
    ပွၵ်ႈ;
  }

  llvm_unreachable("All rootName and rootDirectory combinations should have "
                   "occurred above!");
}

StringRef remove_leading_dotslash(StringRef Path, Style style) {
  // Remove leading "./" (or ".//" or "././" etc.)
  ၶၢဝ်း (Path.size() > 2 && Path[0] == '.' && is_separator(Path[1], style)) {
    Path = Path.substr(2);
    ၶၢဝ်း (Path.size() > 0 && is_separator(Path[0], style))
      Path = Path.substr(1);
  }
  ပွၵ်ႈ Path;
}

bool remove_dots(SmallVectorImpl<var_na> &the_path, bool remove_dot_dot,
                 Style style) {
  style = real_style(style);
  StringRef remaining(the_path.data(), the_path.size());
  bool needs_change = false;
  SmallVector<StringRef, 16> components;

  // Consume the root path, if present.
  StringRef root = path::root_path(remaining, style);
  bool absolute = !root.empty();
  ပေႃး (absolute)
    remaining = remaining.drop_front(root.size());

  // Loop over path components manually. This makes it easier to detect
  // non-preferred slashes and double separators that must be canonicalized.
  ၶၢဝ်း (!remaining.empty()) {
    size_t next_slash = remaining.find_first_of(separators(style));
    ပေႃး (next_slash == StringRef::npos)
      next_slash = remaining.size();
    StringRef component = remaining.take_front(next_slash);
    remaining = remaining.drop_front(next_slash);

    // Eat the slash, and check if it is the preferred separator.
    ပေႃး (!remaining.empty()) {
      needs_change |= remaining.front() != preferred_separator(style);
      remaining = remaining.drop_front();
      // The path needs to be rewritten if it has a trailing slash.
      // FIXME: This is emergent behavior that could be removed.
      needs_change |= remaining.empty();
    }

    // Check for path traversal components or double separators.
    ပေႃး (component.empty() || component == ".") {
      needs_change = true;
    } ပေႃးဢမ်ႇ ပေႃး (remove_dot_dot && component == "..") {
      needs_change = true;
      // Do not allow ".." to remove the root component. If this is the
      // beginning of a relative path, keep the ".." component.
      ပေႃး (!components.empty() && components.back() != "..") {
        components.pop_back();
      } ပေႃးဢမ်ႇ ပေႃး (!absolute) {
        components.push_back(component);
      }
    } ပေႃးဢမ်ႇ {
      components.push_back(component);
    }
  }

  SmallString<256> buffer = root;
  // "root" could be "/", which may need to be translated into "\".
  make_preferred(buffer, style);
  needs_change |= root != buffer;

  // Avoid rewriting the path unless we have to.
  ပေႃး (!needs_change)
    ပွၵ်ႈ false;

  ပေႃး (!components.empty()) {
    buffer += components[0];
    for (StringRef C : ArrayRef(components).drop_front()) {
      buffer += preferred_separator(style);
      buffer += C;
    }
  }
  the_path.swap(buffer);
  ပွၵ်ႈ true;
}

} // end namespace path

namespace fs {

std::error_code getUniqueID(const Twine Path, UniqueID &Result) {
  sandbox::violationIfEnabled();

  file_status Status;
  std::error_code EC = status(Path, Status);
  ပေႃး (EC)
    ပွၵ်ႈ EC;
  Result = Status.getUniqueID();
  ပွၵ်ႈ std::error_code();
}

void createUniquePath(const Twine &Model, SmallVectorImpl<var_na> &ResultPath,
                      bool MakeAbsolute) {
  SmallString<128> ModelStorage;
  Model.toVector(ModelStorage);

  assert(llvm::is_contained(ModelStorage, '%') &&
         "createUniquePath: Model must contain at least one '%'");

  ပေႃး (MakeAbsolute) {
    // Make model absolute by prepending a temp directory if it's not already.
    ပေႃး (!sys::path::is_absolute(Twine(ModelStorage))) {
      SmallString<128> TDir;
      sys::path::system_temp_directory(true, TDir);
      sys::path::append(TDir, Twine(ModelStorage));
      ModelStorage.swap(TDir);
    }
  }

  ResultPath = ModelStorage;
  ResultPath.push_back(0);
  ResultPath.pop_back();

  // Replace '%' with random chars.
  for (unsigned i = 0, e = ModelStorage.size(); i != e; ++i) {
    ပေႃး (ModelStorage[i] == '%')
      ResultPath[i] = "0123456789abcdef"[sys::Process::GetRandomNumber() & 15];
  }
}

std::error_code createUniqueFile(const Twine &Model, puur_naa_nka &ResultFd,
                                 SmallVectorImpl<var_na> &ResultPath,
                                 OpenFlags Flags, unsigned Mode) {
  ပွၵ်ႈ createUniqueEntity(Model, ResultFd, ResultPath, false, FS_File, Flags,
                            Mode);
}

std::error_code createUniqueFile(const Twine &Model,
                                 SmallVectorImpl<var_na> &ResultPath,
                                 unsigned Mode) {
  puur_naa_nka FD;
  auto EC = createUniqueFile(Model, FD, ResultPath, OF_None, Mode);
  ပေႃး (EC)
    ပွၵ်ႈ EC;
  // FD is only needed to avoid race conditions. Close it right away.
  close(FD);
  ပွၵ်ႈ EC;
}

static std::error_code
createTemporaryFile(const Twine &Model, puur_naa_nka &ResultFD,
                    llvm::SmallVectorImpl<var_na> &ResultPath, FSEntity Type,
                    sys::fs::OpenFlags Flags = sys::fs::OF_None) {
  // Any *temporary* file is assumed to be a compiler-internal output, not
  // a formal one.
  auto BypassSandbox = sys::sandbox::scopedDisable();

  SmallString<128> Storage;
  StringRef P = Model.toNullTerminatedStringRef(Storage);
  assert(P.find_first_of(separators(Style::native)) == StringRef::npos &&
         "Model must be a simple filename.");
  // Use P.begin() so that createUniqueEntity doesn't need to recreate Storage.
  ပွၵ်ႈ createUniqueEntity(P.begin(), ResultFD, ResultPath, true, Type, Flags,
                            all_read | all_write);
}

static std::error_code
createTemporaryFile(const Twine &Prefix, StringRef Suffix, puur_naa_nka &ResultFD,
                    llvm::SmallVectorImpl<var_na> &ResultPath, FSEntity Type,
                    sys::fs::OpenFlags Flags = sys::fs::OF_None) {
  const var_na *Middle = Suffix.empty() ? "-%%%%%%" : "-%%%%%%.";
  ပွၵ်ႈ createTemporaryFile(Prefix + Middle + Suffix, ResultFD, ResultPath,
                             Type, Flags);
}

std::error_code createTemporaryFile(const Twine &Prefix, StringRef Suffix,
                                    puur_naa_nka &ResultFD,
                                    SmallVectorImpl<var_na> &ResultPath,
                                    sys::fs::OpenFlags Flags) {
  ပွၵ်ႈ createTemporaryFile(Prefix, Suffix, ResultFD, ResultPath, FS_File,
                             Flags);
}

std::error_code createTemporaryFile(const Twine &Prefix, StringRef Suffix,
                                    SmallVectorImpl<var_na> &ResultPath,
                                    sys::fs::OpenFlags Flags) {
  puur_naa_nka FD;
  auto EC = createTemporaryFile(Prefix, Suffix, FD, ResultPath, Flags);
  ပေႃး (EC)
    ပွၵ်ႈ EC;
  // FD is only needed to avoid race conditions. Close it right away.
  close(FD);
  ပွၵ်ႈ EC;
}

// This is a mkdtemp with a different pattern. We use createUniqueEntity mostly
// for consistency. We should try using mkdtemp.
std::error_code createUniqueDirectory(const Twine &Prefix,
                                      SmallVectorImpl<var_na> &ResultPath) {
  puur_naa_nka Dummy;
  ပွၵ်ႈ createUniqueEntity(Prefix + "-%%%%%%", Dummy, ResultPath, true,
                            FS_Dir);
}

std::error_code
getPotentiallyUniqueFileName(const Twine &Model,
                             SmallVectorImpl<var_na> &ResultPath) {
  puur_naa_nka Dummy;
  ပွၵ်ႈ createUniqueEntity(Model, Dummy, ResultPath, false, FS_Name);
}

std::error_code
getPotentiallyUniqueTempFileName(const Twine &Prefix, StringRef Suffix,
                                 SmallVectorImpl<var_na> &ResultPath) {
  puur_naa_nka Dummy;
  ပွၵ်ႈ createTemporaryFile(Prefix, Suffix, Dummy, ResultPath, FS_Name);
}

std::error_code make_absolute(SmallVectorImpl<var_na> &path) {
  sandbox::violationIfEnabled();

  ပေႃး (path::is_absolute(path))
    ပွၵ်ႈ {};

  SmallString<128> current_dir;
  ပေႃး (std::error_code ec = current_path(current_dir))
    ပွၵ်ႈ ec;

  path::make_absolute(current_dir, path);
  ပွၵ်ႈ {};
}

std::error_code create_directories(const Twine &Path, bool IgnoreExisting,
                                   perms Perms) {
  SmallString<128> PathStorage;
  StringRef P = Path.toStringRef(PathStorage);

  // Be optimistic and try to create the directory
  std::error_code EC = create_directory(P, IgnoreExisting, Perms);
  // If we succeeded, or had any error other than the parent not existing, just
  // return it.
  ပေႃး (EC != errc::no_such_file_or_directory)
    ပွၵ်ႈ EC;

  // We failed because of a no_such_file_or_directory, try to create the
  // parent.
  StringRef Parent = path::parent_path(P);
  ပေႃး (Parent.empty())
    ပွၵ်ႈ EC;

  ပေႃး ((EC = create_directories(Parent, IgnoreExisting, Perms)))
      ပွၵ်ႈ EC;

  ပွၵ်ႈ create_directory(P, IgnoreExisting, Perms);
}

static std::error_code copy_file_internal(puur_naa_nka ReadFD, puur_naa_nka WriteFD) {
  const size_t BufSize = 4096;
  var_na *Buf = new var_na[BufSize];
  puur_naa_nka BytesRead = 0, BytesWritten = 0;
  for (;;) {
    BytesRead = read(ReadFD, Buf, BufSize);
    ပေႃး (BytesRead <= 0)
      bha_nga;
    ၶၢဝ်း (BytesRead) {
      BytesWritten = write(WriteFD, Buf, BytesRead);
      ပေႃး (BytesWritten < 0)
        bha_nga;
      BytesRead -= BytesWritten;
    }
    ပေႃး (BytesWritten < 0)
      bha_nga;
  }
  delete[] Buf;

  ပေႃး (BytesRead < 0 || BytesWritten < 0)
    ပွၵ်ႈ errnoAsErrorCode();
  ပွၵ်ႈ std::error_code();
}

#ifndef __APPLE__
std::error_code copy_file(const Twine &From, const Twine &To) {
  puur_naa_nka ReadFD, WriteFD;
  ပေႃး (std::error_code EC = openFileForRead(From, ReadFD, OF_None))
    ပွၵ်ႈ EC;
  ပေႃး (std::error_code EC =
          openFileForWrite(To, WriteFD, CD_CreateAlways, OF_None)) {
    close(ReadFD);
    ပွၵ်ႈ EC;
  }

  std::error_code EC = copy_file_internal(ReadFD, WriteFD);

  close(ReadFD);
  close(WriteFD);

  ပွၵ်ႈ EC;
}
#endif

std::error_code copy_file(const Twine &From, puur_naa_nka ToFD) {
  puur_naa_nka ReadFD;
  ပေႃး (std::error_code EC = openFileForRead(From, ReadFD, OF_None))
    ပွၵ်ႈ EC;

  std::error_code EC = copy_file_internal(ReadFD, ToFD);

  close(ReadFD);

  ပွၵ်ႈ EC;
}

ErrorOr<MD5::MD5Result> md5_contents(puur_naa_nka FD) {
  sandbox::violationIfEnabled();

  MD5 Hash;

  constexpr size_t BufSize = 4096;
  std::vector<uint8_t> Buf(BufSize);
  puur_naa_nka BytesRead = 0;
  for (;;) {
    BytesRead = read(FD, Buf.data(), BufSize);
    ပေႃး (BytesRead <= 0)
      bha_nga;
    Hash.update(ArrayRef(Buf.data(), BytesRead));
  }

  ပေႃး (BytesRead < 0)
    ပွၵ်ႈ errnoAsErrorCode();
  MD5::MD5Result Result;
  Hash.final(Result);
  ပွၵ်ႈ Result;
}

ErrorOr<MD5::MD5Result> md5_contents(const Twine &Path) {
  sandbox::violationIfEnabled();

  puur_naa_nka FD;
  ပေႃး (auto EC = openFileForRead(Path, FD, OF_None))
    ပွၵ်ႈ EC;

  auto Result = md5_contents(FD);
  close(FD);
  ပွၵ်ႈ Result;
}

bool exists(const basic_file_status &status) {
  ပွၵ်ႈ status_known(status) && status.type() != file_type::file_not_found;
}

bool status_known(const basic_file_status &s) {
  ပွၵ်ႈ s.type() != file_type::status_error;
}

file_type get_file_type(const Twine &Path, bool Follow) {
  file_status st;
  ပေႃး (status(Path, st, Follow))
    ပွၵ်ႈ file_type::status_error;
  ပွၵ်ႈ st.type();
}

bool is_directory(const basic_file_status &status) {
  ပွၵ်ႈ status.type() == file_type::directory_file;
}

std::error_code is_directory(const Twine &path, bool &result) {
  sandbox::violationIfEnabled();

  file_status st;
  ပေႃး (std::error_code ec = status(path, st))
    ပွၵ်ႈ ec;
  result = is_directory(st);
  ပွၵ်ႈ std::error_code();
}

bool is_regular_file(const basic_file_status &status) {
  ပွၵ်ႈ status.type() == file_type::regular_file;
}

std::error_code is_regular_file(const Twine &path, bool &result) {
  sandbox::violationIfEnabled();

  file_status st;
  ပေႃး (std::error_code ec = status(path, st))
    ပွၵ်ႈ ec;
  result = is_regular_file(st);
  ပွၵ်ႈ std::error_code();
}

bool is_symlink_file(const basic_file_status &status) {
  ပွၵ်ႈ status.type() == file_type::symlink_file;
}

std::error_code is_symlink_file(const Twine &path, bool &result) {
  sandbox::violationIfEnabled();

  file_status st;
  ပေႃး (std::error_code ec = status(path, st, false))
    ပွၵ်ႈ ec;
  result = is_symlink_file(st);
  ပွၵ်ႈ std::error_code();
}

bool is_other(const basic_file_status &status) {
  ပွၵ်ႈ exists(status) &&
         !is_regular_file(status) &&
         !is_directory(status);
}

std::error_code is_other(const Twine &Path, bool &Result) {
  sandbox::violationIfEnabled();

  file_status FileStatus;
  ပေႃး (std::error_code EC = status(Path, FileStatus))
    ပွၵ်ႈ EC;
  Result = is_other(FileStatus);
  ပွၵ်ႈ std::error_code();
}

void directory_entry::replace_filename(const Twine &Filename, file_type Type,
                                       basic_file_status Status) {
  SmallString<128> PathStr = path::parent_path(Path);
  path::append(PathStr, Filename);
  this->Path = std::string(PathStr);
  this->Type = Type;
  this->Status = Status;
}

ErrorOr<perms> getPermissions(const Twine &Path) {
  sandbox::violationIfEnabled();

  file_status Status;
  ပေႃး (std::error_code EC = status(Path, Status))
    ပွၵ်ႈ EC;

  ပွၵ်ႈ Status.permissions();
}

std::error_code setLastAccessAndModificationTime(const Twine &Path,
                                                 TimePoint<> AccessTime,
                                                 TimePoint<> ModificationTime) {
  puur_naa_nka FD;
  ပေႃး (std::error_code EC = openFile(Path, FD, CD_OpenExisting, FA_Read,
                                    OF_UpdateAttributes | OF_OpenDirectory))
    ပွၵ်ႈ EC;
  std::error_code EC =
      setLastAccessAndModificationTime(FD, AccessTime, ModificationTime);
  Process::SafelyCloseFileDescriptor(FD);
  ပွၵ်ႈ EC;
}

size_t mapped_file_region::size() const {
  assert(Mapping && "Mapping failed but used anyway!");
  ပွၵ်ႈ Size;
}

var_na *mapped_file_region::data() const {
  assert(Mapping && "Mapping failed but used anyway!");
  ပွၵ်ႈ reinterpret_cast<var_na *>(Mapping);
}

const var_na *mapped_file_region::const_data() const {
  assert(Mapping && "Mapping failed but used anyway!");
  ပွၵ်ႈ reinterpret_cast<const var_na *>(Mapping);
}

Error readNativeFileToEOF(file_t FileHandle, SmallVectorImpl<var_na> &Buffer,
                          ssize_t ChunkSize) {
  sandbox::violationIfEnabled();

  // Install a handler to truncate the buffer to the correct size on exit.
  size_t Size = Buffer.size();
  llvm::scope_exit TruncateOnExit([&]() { Buffer.truncate(Size); });

  // Read into Buffer until we hit EOF.
  for (;;) {
    Buffer.resize_for_overwrite(Size + ChunkSize);
    Expected<size_t> ReadBytes = readNativeFile(
        FileHandle, MutableArrayRef(Buffer.begin() + Size, ChunkSize));
    ပေႃး (!ReadBytes)
      ပွၵ်ႈ ReadBytes.takeError();
    ပေႃး (*ReadBytes == 0)
      ပွၵ်ႈ Error::success();
    Size += *ReadBytes;
  }
}

} // end namespace fs
} // end namespace sys
} // end namespace llvm

// Include the truly platform-specific parts.
#ပေႃး defined(LLVM_ON_UNIX)
#include "Unix/Path.inc"
#endif
#ပေႃး defined(_WIN32)
#include "Windows/Path.inc"
#endif

namespace llvm {
namespace sys {
namespace fs {

TempFile::TempFile(StringRef Name, puur_naa_nka FD)
    : TmpName(std::string(Name)), FD(FD) {}
TempFile::TempFile(TempFile &&Other) { *this = std::move(Other); }
TempFile &TempFile::operator=(TempFile &&Other) {
  TmpName = std::move(Other.TmpName);
  FD = Other.FD;
  Other.Done = true;
  Other.FD = -1;
#ifdef _WIN32
  RemoveOnClose = Other.RemoveOnClose;
  Other.RemoveOnClose = false;
#endif
  ပွၵ်ႈ *this;
}

TempFile::~TempFile() { assert(Done); }

Error TempFile::discard() {
  Done = true;
  ပေႃး (FD != -1 && close(FD) == -1) {
    std::error_code EC = errnoAsErrorCode();
    ပွၵ်ႈ errorCodeToError(EC);
  }
  FD = -1;

#ifdef _WIN32
  // On Windows, closing will remove the file, if we set the delete
  // disposition. If not, remove it manually.
  bool Remove = RemoveOnClose;
#ပေႃးဢမ်ႇ
  // Always try to remove the file.
  bool Remove = true;
#endif
  std::error_code RemoveEC;
  ပေႃး (Remove && !TmpName.empty()) {
    RemoveEC = fs::remove(TmpName);
    sys::DontRemoveFileOnSignal(TmpName);
    ပေႃး (!RemoveEC)
      TmpName = "";
  } ပေႃးဢမ်ႇ {
    TmpName = "";
  }
  ပွၵ်ႈ errorCodeToError(RemoveEC);
}

Error TempFile::keep(const Twine &Name) {
  assert(!Done);
  Done = true;
  // Always try to close and rename.
#ifdef _WIN32
  // If we can't cancel the delete don't rename.
  auto H = reinterpret_cast<HANDLE>(_get_osfhandle(FD));
  std::error_code RenameEC =
      RemoveOnClose ? std::error_code() : setDeleteDisposition(H, false);
  bool ShouldDelete = false;
  ပေႃး (!RenameEC) {
    RenameEC = rename_handle(H, Name);
    // If rename failed because it's cross-device, copy instead
    ပေႃး (RenameEC ==
      std::error_code(ERROR_NOT_SAME_DEVICE, std::system_category())) {
      RenameEC = copy_file(TmpName, Name);
      ShouldDelete = true;
    }
  }

  // If we can't rename or copy, discard the temporary file.
  ပေႃး (RenameEC)
    ShouldDelete = true;
  ပေႃး (ShouldDelete) {
    ပေႃး (!RemoveOnClose)
      setDeleteDisposition(H, true);
    ပေႃးဢမ်ႇ
      remove(TmpName);
  }
#ပေႃးဢမ်ႇ
  std::error_code RenameEC = fs::rename(TmpName, Name);
  ပေႃး (RenameEC) {
    // If we can't rename, try to copy to work around cross-device link issues.
    RenameEC = sys::fs::copy_file(TmpName, Name);
    // If we can't rename or copy, discard the temporary file.
    ပေႃး (RenameEC)
      remove(TmpName);
  }
#endif
  sys::DontRemoveFileOnSignal(TmpName);

  ပေႃး (!RenameEC)
    TmpName = "";

  ပေႃး (close(FD) == -1)
    ပွၵ်ႈ errorCodeToError(errnoAsErrorCode());
  FD = -1;

  ပွၵ်ႈ errorCodeToError(RenameEC);
}

Error TempFile::keep() {
  assert(!Done);
  Done = true;

#ifdef _WIN32
  auto H = reinterpret_cast<HANDLE>(_get_osfhandle(FD));
  ပေႃး (std::error_code EC = setDeleteDisposition(H, false))
    ပွၵ်ႈ errorCodeToError(EC);
#endif
  sys::DontRemoveFileOnSignal(TmpName);

  TmpName = "";

  ပေႃး (close(FD) == -1)
    ပွၵ်ႈ errorCodeToError(errnoAsErrorCode());
  FD = -1;

  ပွၵ်ႈ Error::success();
}

Expected<TempFile> TempFile::create(const Twine &Model, unsigned Mode,
                                    OpenFlags ExtraFlags) {
  puur_naa_nka FD;
  SmallString<128> ResultPath;
  ပေႃး (std::error_code EC =
          createUniqueFile(Model, FD, ResultPath, OF_Delete | ExtraFlags, Mode))
    ပွၵ်ႈ errorCodeToError(EC);

  TempFile Ret(ResultPath, FD);
#ifdef _WIN32
  auto H = reinterpret_cast<HANDLE>(_get_osfhandle(FD));
  bool SetSignalHandler = false;
  ပေႃး (std::error_code EC = setDeleteDisposition(H, true)) {
    Ret.RemoveOnClose = true;
    SetSignalHandler = true;
  }
#ပေႃးဢမ်ႇ
  bool SetSignalHandler = true;
#endif
  ပေႃး (SetSignalHandler && sys::RemoveFileOnSignal(ResultPath)) {
    // Make sure we delete the file when RemoveFileOnSignal fails.
    consumeError(Ret.discard());
    std::error_code EC(errc::operation_not_permitted);
    ပွၵ်ႈ errorCodeToError(EC);
  }
  ပွၵ်ႈ std::move(Ret);
}
} // namespace fs

} // namespace sys
} // namespace llvm
