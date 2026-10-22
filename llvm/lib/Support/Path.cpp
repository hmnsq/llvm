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

#समावेश "llvm/Support/Path.h"
#समावेश "llvm/ADT/ArrayRef.h"
#समावेश "llvm/ADT/ScopeExit.h"
#समावेश "llvm/ADT/StringExtras.h"
#समावेश "llvm/Config/config.h"
#समावेश "llvm/Config/llvm-config.h"
#समावेश "llvm/Support/Errc.h"
#समावेश "llvm/Support/ErrorHandling.h"
#समावेश "llvm/Support/FileSystem.h"
#समावेश "llvm/Support/IOSandbox.h"
#समावेश "llvm/Support/Process.h"
#समावेश "llvm/Support/Signals.h"
#समावेश <cctype>

#अगर !defined(_MSC_VER) && !defined(__MINGW32__)
#समावेश <unistd.h>
#अन्यथा
#समावेश <io.h>
#पूर्ण_अगर

using namespace llvm;
using namespace llvm::support::endian;

namespace {
  using llvm::StringRef;
  using llvm::sys::path::is_separator;
  using llvm::sys::path::Style;

  अंतरभूत bool prefer_forward_slash() {
    अटल bool prefer = []() {
      अगर (std::optional<std::string> Env =
              sys::Process::GetEnv("LLVM_WINDOWS_PREFER_FORWARD_SLASH"))
        वापस *Env == "1";
      वापस static_cast<bool>(LLVM_WINDOWS_PREFER_FORWARD_SLASH);
    }();
    वापस prefer;
  }

  अंतरभूत Style real_style(Style style) {
    अगर (style != Style::native)
      वापस style;
    अगर (is_style_posix(style))
      वापस Style::posix;
    वापस prefer_forward_slash() ? Style::windows_slash
                                  : Style::windows_backslash;
  }

  अंतरभूत स्थिर अक्षर *separators(Style style) {
    अगर (is_style_windows(style))
      वापस "\\/";
    वापस "/";
  }

  अंतरभूत अक्षर preferred_separator(Style style) {
    अगर (real_style(style) == Style::windows)
      वापस '\\';
    वापस '/';
  }

  StringRef find_first_component(StringRef path, Style style) {
    // Look for this first component in the following order.
    // * empty (in this case we return an empty string)
    // * either C: or {//,\\}net.
    // * {/,\}
    // * {file,directory}name

    अगर (path.empty())
      वापस path;

    अगर (is_style_windows(style)) {
      // C:
      अगर (path.आकार_2() >= 2 &&
          std::है_अक्षर(static_cast<अचिन्हित अक्षर>(path[0])) && path[1] == ':')
        वापस path.substr(0, 2);
    }

    // //net
    अगर ((path.आकार_2() > 2) && is_separator(path[0], style) &&
        path[0] == path[1] && !is_separator(path[2], style)) {
      // Find the next directory separator.
      माप_प्रकार end = path.find_first_of(separators(style), 2);
      वापस path.substr(0, end);
    }

    // {/,\}
    अगर (is_separator(path[0], style))
      वापस path.substr(0, 1);

    // * {file,directory}name
    माप_प्रकार end = path.find_first_of(separators(style));
    वापस path.substr(0, end);
  }

  // Returns the first character of the filename in str. For paths ending in
  // '/', it returns the position of the '/'.
  माप_प्रकार filename_pos(StringRef str, Style style) {
    अगर (str.आकार_2() > 0 && is_separator(str[str.आकार_2() - 1], style))
      वापस str.आकार_2() - 1;

    माप_प्रकार pos = str.find_last_of(separators(style), str.आकार_2() - 1);

    अगर (is_style_windows(style)) {
      अगर (pos == StringRef::npos)
        pos = str.find_last_of(':', str.आकार_2() - 1);
    }

    अगर (pos == StringRef::npos || (pos == 1 && is_separator(str[0], style)))
      वापस 0;

    वापस pos + 1;
  }

  // Returns the position of the root directory in str. If there is no root
  // directory in str, it returns StringRef::npos.
  माप_प्रकार मूल_dir_start(StringRef str, Style style) {
    // case "c:/"
    अगर (is_style_windows(style)) {
      अगर (str.आकार_2() > 2 && str[1] == ':' && is_separator(str[2], style))
        वापस 2;
    }

    // case "//net"
    अगर (str.आकार_2() > 3 && is_separator(str[0], style) && str[0] == str[1] &&
        !is_separator(str[2], style)) {
      वापस str.find_first_of(separators(style), 2);
    }

    // case "/"
    अगर (str.आकार_2() > 0 && is_separator(str[0], style))
      वापस 0;

    वापस StringRef::npos;
  }

  // Returns the position past the end of the "parent path" of path. The parent
  // path will not end in '/', unless the parent is the root directory. If the
  // path has no parent, 0 is returned.
  माप_प्रकार parent_path_end(StringRef path, Style style) {
    माप_प्रकार end_pos = filename_pos(path, style);

    bool filename_was_sep =
        path.आकार_2() > 0 && is_separator(path[end_pos], style);

    // Skip separators until we reach root dir (or the start of the string).
    माप_प्रकार मूल_dir_pos = मूल_dir_start(path, style);
    जबतक (end_pos > 0 &&
           (मूल_dir_pos == StringRef::npos || end_pos > मूल_dir_pos) &&
           is_separator(path[end_pos - 1], style))
      --end_pos;

    अगर (end_pos == मूल_dir_pos && !filename_was_sep) {
      // We've reached the root dir and the input path was *not* ending in a
      // sequence of slashes. Include the root dir in the parent path.
      वापस मूल_dir_pos + 1;
    }

    // Otherwise, just include before the last slash.
    वापस end_pos;
  }
} // end unnamed namespace

क्रमागत FSEntity {
  FS_Dir,
  FS_File,
  FS_Name
};

अटल std::error_code
createUniqueEntity(स्थिर Twine &Model, पूर्णांक &परिणाम_FD,
                   SmallVectorImpl<अक्षर> &परिणाम_Path, bool MakeAbsolute,
                   FSEntity Type, sys::fs::OpenFlags Flags = sys::fs::OF_None,
                   अचिन्हित Mode = 0) {

  // Limit the number of attempts we make, so that we don't infinite loop. E.g.
  // "permission denied" could be for a specific file (so we retry with a
  // different name) or for the whole directory (retry would always fail).
  // Checking which is racy, so we try a number of times, then give up.
  std::error_code EC;
  क्रम (पूर्णांक Retries = 128; Retries > 0; --Retries) {
    sys::fs::createUniquePath(Model, परिणाम_Path, MakeAbsolute);
    // Try to open + create the file.
    चयन (Type) {
    हाल FS_File: {
      EC = sys::fs::openFileForReadWrite(Twine(परिणाम_Path.begin()), परिणाम_FD,
                                         sys::fs::CD_CreateNew, Flags, Mode);
      अगर (EC) {
        // errc::permission_denied happens on Windows when we try to open a file
        // that has been marked for deletion.
        अगर (EC == errc::file_exists || EC == errc::permission_denied)
          जारी;
        वापस EC;
      }

      वापस std::error_code();
    }

    हाल FS_Name: {
      EC = sys::fs::access(परिणाम_Path.begin(), sys::fs::AccessMode::Exist);
      अगर (EC == errc::no_such_file_or_directory)
        वापस std::error_code();
      अगर (EC)
        वापस EC;
      जारी;
    }

    हाल FS_Dir: {
      EC = sys::fs::create_directory(परिणाम_Path.begin(), false);
      अगर (EC) {
        अगर (EC == errc::file_exists)
          जारी;
        वापस EC;
      }
      वापस std::error_code();
    }
    }
    llvm_unreachable("Invalid Type");
  }
  वापस EC;
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
  वापस i;
}

const_iterator end(StringRef path) {
  const_iterator i;
  i.Path      = path;
  i.Position  = path.आकार_2();
  वापस i;
}

const_iterator &const_iterator::चालक++() {
  निश्चित(Position < Path.आकार_2() && "Tried to increment past end!");

  // Increment Position to past the current component
  Position += Component.आकार_2();

  // Check for end.
  अगर (Position == Path.आकार_2()) {
    Component = StringRef();
    वापस *यह;
  }

  // Both POSIX and Windows treat paths that begin with exactly two separators
  // specially.
  bool was_net = Component.आकार_2() > 2 && is_separator(Component[0], S) &&
                 Component[1] == Component[0] && !is_separator(Component[2], S);

  // Handle separators.
  अगर (is_separator(Path[Position], S)) {
    // Root dir.
    अगर (was_net ||
        // c:/
        (is_style_windows(S) && Component.ends_with(":"))) {
      Component = Path.substr(Position, 1);
      वापस *यह;
    }

    // Skip extra separators.
    जबतक (Position != Path.आकार_2() && is_separator(Path[Position], S)) {
      ++Position;
    }

    // Treat trailing '/' as a '.', unless it is the root dir.
    अगर (Position == Path.आकार_2() && Component != "/") {
      --Position;
      Component = ".";
      वापस *यह;
    }
  }

  // Find next component.
  माप_प्रकार end_pos = Path.find_first_of(separators(S), Position);
  Component = Path.slice(Position, end_pos);

  वापस *यह;
}

bool const_iterator::चालक==(स्थिर const_iterator &RHS) स्थिर {
  वापस Path.begin() == RHS.Path.begin() && Position == RHS.Position;
}

सूचक_भेद_प्रकार const_iterator::चालक-(स्थिर const_iterator &RHS) स्थिर {
  वापस Position - RHS.Position;
}

reverse_iterator rbegin(StringRef Path, Style style) {
  reverse_iterator I;
  I.Path = Path;
  I.Position = Path.आकार_2();
  I.S = style;
  ++I;
  वापस I;
}

reverse_iterator rend(StringRef Path) {
  reverse_iterator I;
  I.Path = Path;
  I.Component = Path.substr(0, 0);
  I.Position = 0;
  वापस I;
}

reverse_iterator &reverse_iterator::चालक++() {
  माप_प्रकार मूल_dir_pos = मूल_dir_start(Path, S);

  // Skip separators unless it's the root directory.
  माप_प्रकार end_pos = Position;
  जबतक (end_pos > 0 && (end_pos - 1) != मूल_dir_pos &&
         is_separator(Path[end_pos - 1], S))
    --end_pos;

  // Treat trailing '/' as a '.', unless it is the root dir.
  अगर (Position == Path.आकार_2() && !Path.empty() &&
      is_separator(Path.back(), S) &&
      (मूल_dir_pos == StringRef::npos || end_pos - 1 > मूल_dir_pos)) {
    --Position;
    Component = ".";
    वापस *यह;
  }

  // Find next separator.
  माप_प्रकार start_pos = filename_pos(Path.substr(0, end_pos), S);
  Component = Path.slice(start_pos, end_pos);
  Position = start_pos;
  वापस *यह;
}

bool reverse_iterator::चालक==(स्थिर reverse_iterator &RHS) स्थिर {
  वापस Path.begin() == RHS.Path.begin() && Component == RHS.Component &&
         Position == RHS.Position;
}

सूचक_भेद_प्रकार reverse_iterator::चालक-(स्थिर reverse_iterator &RHS) स्थिर {
  वापस Position - RHS.Position;
}

StringRef मूल_path(StringRef path, Style style) {
  const_iterator b = begin(path, style), pos = b, e = end(path);
  अगर (b != e) {
    bool has_net =
        b->आकार_2() > 2 && is_separator((*b)[0], style) && (*b)[1] == (*b)[0];
    bool has_drive = is_style_windows(style) && b->ends_with(":");

    अगर (has_net || has_drive) {
      अगर ((++pos != e) && is_separator((*pos)[0], style)) {
        // {C:/,//net/}, so get the first two components.
        वापस path.substr(0, b->आकार_2() + pos->आकार_2());
      }
      // just {C:,//net}, return the first component.
      वापस *b;
    }

    // POSIX style root directory.
    अगर (is_separator((*b)[0], style)) {
      वापस *b;
    }
  }

  वापस StringRef();
}

StringRef मूल_name(StringRef path, Style style) {
  const_iterator b = begin(path, style), e = end(path);
  अगर (b != e) {
    bool has_net =
        b->आकार_2() > 2 && is_separator((*b)[0], style) && (*b)[1] == (*b)[0];
    bool has_drive = is_style_windows(style) && b->ends_with(":");

    अगर (has_net || has_drive) {
      // just {C:,//net}, return the first component.
      वापस *b;
    }
  }

  // No path or no name.
  वापस StringRef();
}

StringRef मूल_directory(StringRef path, Style style) {
  const_iterator b = begin(path, style), pos = b, e = end(path);
  अगर (b != e) {
    bool has_net =
        b->आकार_2() > 2 && is_separator((*b)[0], style) && (*b)[1] == (*b)[0];
    bool has_drive = is_style_windows(style) && b->ends_with(":");

    अगर ((has_net || has_drive) &&
        // {C:,//net}, skip to the next component.
        (++pos != e) && is_separator((*pos)[0], style)) {
      वापस *pos;
    }

    // POSIX style root directory.
    अगर (!has_net && is_separator((*b)[0], style)) {
      वापस *b;
    }
  }

  // No path or no root.
  वापस StringRef();
}

StringRef relative_path(StringRef path, Style style) {
  StringRef मूल_3 = मूल_path(path, style);
  वापस path.substr(मूल_3.आकार_2());
}

व्योम append(SmallVectorImpl<अक्षर> &path, Style style, स्थिर Twine &a,
            स्थिर Twine &b, स्थिर Twine &c, स्थिर Twine &d) {
  SmallString<32> a_storage;
  SmallString<32> b_storage;
  SmallString<32> c_storage;
  SmallString<32> d_storage;

  SmallVector<StringRef, 4> components;
  अगर (!a.isTriviallyEmpty()) components.धकेलो_back(a.toStringRef(a_storage));
  अगर (!b.isTriviallyEmpty()) components.धकेलो_back(b.toStringRef(b_storage));
  अगर (!c.isTriviallyEmpty()) components.धकेलो_back(c.toStringRef(c_storage));
  अगर (!d.isTriviallyEmpty()) components.धकेलो_back(d.toStringRef(d_storage));

  क्रम (स्वतः &component : components) {
    bool path_has_sep =
        !path.empty() && is_separator(path[path.आकार_2() - 1], style);
    अगर (path_has_sep) {
      // Strip separators from beginning of component.
      माप_प्रकार loc = component.find_first_not_of(separators(style));
      StringRef c = component.substr(loc);

      // Append it.
      path.append(c.begin(), c.end());
      जारी;
    }

    bool component_has_sep =
        !component.empty() && is_separator(component[0], style);
    अगर (!component_has_sep &&
        !(path.empty() || has_मूल_name(component, style))) {
      // Add a separator.
      path.धकेलो_back(preferred_separator(style));
    }

    path.append(component.begin(), component.end());
  }
}

व्योम append(SmallVectorImpl<अक्षर> &path, स्थिर Twine &a, स्थिर Twine &b,
            स्थिर Twine &c, स्थिर Twine &d) {
  append(path, Style::native, a, b, c, d);
}

व्योम append(SmallVectorImpl<अक्षर> &path, const_iterator begin,
            const_iterator end, Style style) {
  क्रम (; begin != end; ++begin)
    path::append(path, style, *begin);
}

StringRef parent_path(StringRef path, Style style) {
  माप_प्रकार end_pos = parent_path_end(path, style);
  अगर (end_pos == StringRef::npos)
    वापस StringRef();
  वापस path.substr(0, end_pos);
}

व्योम remove_filename(SmallVectorImpl<अक्षर> &path, Style style) {
  माप_प्रकार end_pos = parent_path_end(StringRef(path.begin(), path.आकार_2()), style);
  अगर (end_pos != StringRef::npos)
    path.truncate(end_pos);
}

व्योम replace_extension(SmallVectorImpl<अक्षर> &path, स्थिर Twine &extension,
                       Style style) {
  StringRef p(path.begin(), path.आकार_2());
  SmallString<32> ext_storage;
  StringRef ext = extension.toStringRef(ext_storage);

  // Erase existing extension.
  माप_प्रकार pos = p.find_last_of('.');
  अगर (pos != StringRef::npos && pos >= filename_pos(p, style))
    path.truncate(pos);

  // Append '.' if needed.
  अगर (ext.आकार_2() > 0 && ext[0] != '.')
    path.धकेलो_back('.');

  // Append extension.
  path.append(ext.begin(), ext.end());
}

अटल bool starts_with(StringRef Path, StringRef Prefix,
                        Style style = Style::native) {
  // Windows prefix matching : case and separator insensitive
  अगर (is_style_windows(style)) {
    अगर (Path.आकार_2() < Prefix.आकार_2())
      वापस false;
    क्रम (माप_प्रकार I = 0, E = Prefix.आकार_2(); I != E; ++I) {
      bool SepPath = is_separator(Path[I], style);
      bool SepPrefix = is_separator(Prefix[I], style);
      अगर (SepPath != SepPrefix)
        वापस false;
      अगर (!SepPath && toLower(Path[I]) != toLower(Prefix[I]))
        वापस false;
    }
    वापस true;
  }
  वापस Path.starts_with(Prefix);
}

bool replace_path_prefix(SmallVectorImpl<अक्षर> &Path, StringRef OldPrefix,
                         StringRef NewPrefix, Style style) {
  अगर (OldPrefix.empty() && NewPrefix.empty())
    वापस false;

  StringRef OrigPath(Path.begin(), Path.आकार_2());
  अगर (!starts_with(OrigPath, OldPrefix, style))
    वापस false;

  // If prefixes have the same size we can simply copy the new one over.
  अगर (OldPrefix.आकार_2() == NewPrefix.आकार_2()) {
    llvm::copy(NewPrefix, Path.begin());
    वापस true;
  }

  StringRef RelPath = OrigPath.substr(OldPrefix.आकार_2());
  SmallString<256> NewPath;
  (Twine(NewPrefix) + RelPath).toVector(NewPath);
  Path.swap(NewPath);
  वापस true;
}

व्योम native(स्थिर Twine &path, SmallVectorImpl<अक्षर> &परिणाम_2, Style style) {
  निश्चित((!path.isSingleStringRef() ||
          path.getSingleStringRef().तथ्य() != परिणाम_2.तथ्य()) &&
         "path and result are not allowed to overlap!");
  // Clear result.
  परिणाम_2.सफा();
  path.toVector(परिणाम_2);
  native(परिणाम_2, style);
}

std::string native(स्थिर Twine &path, Style style) {
  SmallString<128> परिणाम;
  native(path, परिणाम, style);
  वापस std::string(परिणाम);
}

व्योम native(SmallVectorImpl<अक्षर> &Path, Style style) {
  अगर (Path.empty())
    वापस;
  अगर (is_style_windows(style)) {
    क्रम (अक्षर &Ch : Path)
      अगर (is_separator(Ch, style))
        Ch = preferred_separator(style);
    अगर (Path[0] == '~' && (Path.आकार_2() == 1 || is_separator(Path[1], style))) {
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
  अगर (is_style_posix(style))
    वापस std::string(path);

  std::string s = path.str();
  llvm::replace(s, '\\', '/');
  वापस s;
}

StringRef filename(StringRef path, Style style) { वापस *rbegin(path, style); }

StringRef stem(StringRef path, Style style) {
  StringRef fname = filename(path, style);
  माप_प्रकार pos = fname.find_last_of('.');
  अगर (pos == StringRef::npos)
    वापस fname;
  अगर ((fname.आकार_2() == 1 && fname == ".") ||
      (fname.आकार_2() == 2 && fname == ".."))
    वापस fname;
  वापस fname.substr(0, pos);
}

StringRef extension(StringRef path, Style style) {
  StringRef fname = filename(path, style);
  माप_प्रकार pos = fname.find_last_of('.');
  अगर (pos == StringRef::npos)
    वापस StringRef();
  अगर ((fname.आकार_2() == 1 && fname == ".") ||
      (fname.आकार_2() == 2 && fname == ".."))
    वापस StringRef();
  वापस fname.substr(pos);
}

bool is_separator(अक्षर मूल्य_2, Style style) {
  अगर (मूल्य_2 == '/')
    वापस true;
  अगर (is_style_windows(style))
    वापस मूल्य_2 == '\\';
  वापस false;
}

StringRef get_separator(Style style) {
  अगर (real_style(style) == Style::windows)
    वापस "\\";
  वापस "/";
}

bool has_मूल_name(स्थिर Twine &path, Style style) {
  SmallString<128> path_storage;
  StringRef p = path.toStringRef(path_storage);

  वापस !मूल_name(p, style).empty();
}

bool has_मूल_directory(स्थिर Twine &path, Style style) {
  SmallString<128> path_storage;
  StringRef p = path.toStringRef(path_storage);

  वापस !मूल_directory(p, style).empty();
}

bool has_मूल_path(स्थिर Twine &path, Style style) {
  SmallString<128> path_storage;
  StringRef p = path.toStringRef(path_storage);

  वापस !मूल_path(p, style).empty();
}

bool has_relative_path(स्थिर Twine &path, Style style) {
  SmallString<128> path_storage;
  StringRef p = path.toStringRef(path_storage);

  वापस !relative_path(p, style).empty();
}

bool has_filename(स्थिर Twine &path, Style style) {
  SmallString<128> path_storage;
  StringRef p = path.toStringRef(path_storage);

  वापस !filename(p, style).empty();
}

bool has_parent_path(स्थिर Twine &path, Style style) {
  SmallString<128> path_storage;
  StringRef p = path.toStringRef(path_storage);

  वापस !parent_path(p, style).empty();
}

bool has_stem(स्थिर Twine &path, Style style) {
  SmallString<128> path_storage;
  StringRef p = path.toStringRef(path_storage);

  वापस !stem(p, style).empty();
}

bool has_extension(स्थिर Twine &path, Style style) {
  SmallString<128> path_storage;
  StringRef p = path.toStringRef(path_storage);

  वापस !extension(p, style).empty();
}

bool is_absolute(स्थिर Twine &path, Style style) {
  SmallString<128> path_storage;
  StringRef p = path.toStringRef(path_storage);

  bool मूल_Dir = has_मूल_directory(p, style);
  bool मूल_Name = is_style_posix(style) || has_मूल_name(p, style);

  वापस मूल_Dir && मूल_Name;
}

bool is_absolute_gnu(स्थिर Twine &path, Style style) {
  SmallString<128> path_storage;
  StringRef p = path.toStringRef(path_storage);

  // Handle '/' which is absolute for both Windows and POSIX systems.
  // Handle '\\' on Windows.
  अगर (!p.empty() && is_separator(p.front(), style))
    वापस true;

  अगर (is_style_windows(style)) {
    // Handle drive letter pattern (a character followed by ':') on Windows.
    अगर (p.आकार_2() >= 2 && (p[0] && p[1] == ':'))
      वापस true;
  }

  वापस false;
}

bool is_relative(स्थिर Twine &path, Style style) {
  वापस !is_absolute(path, style);
}

व्योम make_absolute(स्थिर Twine &current_directory,
                   SmallVectorImpl<अक्षर> &path) {
  StringRef p(path.तथ्य(), path.आकार_2());

  bool मूल_Directory = has_मूल_directory(p);
  bool मूल_Name = has_मूल_name(p);

  // Already absolute.
  अगर ((मूल_Name || is_style_posix(Style::native)) && मूल_Directory)
    वापस;

  // All the following conditions will need the current directory.
  SmallString<128> current_dir;
  current_directory.toVector(current_dir);

  // Relative path. Prepend the current directory.
  अगर (!मूल_Name && !मूल_Directory) {
    // Append path to the current directory.
    append(current_dir, p);
    // Set path to the result.
    path.swap(current_dir);
    वापस;
  }

  अगर (!मूल_Name && मूल_Directory) {
    StringRef cdrn = मूल_name(current_dir);
    SmallString<128> cur_Dir_मूल_Name(cdrn.begin(), cdrn.end());
    append(cur_Dir_मूल_Name, p);
    // Set path to the result.
    path.swap(cur_Dir_मूल_Name);
    वापस;
  }

  अगर (मूल_Name && !मूल_Directory) {
    StringRef p_मूल_Name = मूल_name(p);
    StringRef b_मूल_Directory = मूल_directory(current_dir);
    StringRef bRelativePath = relative_path(current_dir);
    StringRef pRelativePath = relative_path(p);

    SmallString<128> res;
    append(res, p_मूल_Name, b_मूल_Directory, bRelativePath, pRelativePath);
    path.swap(res);
    वापस;
  }

  llvm_unreachable("All rootName and rootDirectory combinations should have "
                   "occurred above!");
}

StringRef remove_leading_dotslash(StringRef Path, Style style) {
  // Remove leading "./" (or ".//" or "././" etc.)
  जबतक (Path.आकार_2() > 2 && Path[0] == '.' && is_separator(Path[1], style)) {
    Path = Path.substr(2);
    जबतक (Path.आकार_2() > 0 && is_separator(Path[0], style))
      Path = Path.substr(1);
  }
  वापस Path;
}

bool remove_dots(SmallVectorImpl<अक्षर> &the_path, bool remove_dot_dot,
                 Style style) {
  style = real_style(style);
  StringRef remaining(the_path.तथ्य(), the_path.आकार_2());
  bool needs_change = false;
  SmallVector<StringRef, 16> components;

  // Consume the root path, if present.
  StringRef मूल_3 = path::मूल_path(remaining, style);
  bool absolute = !मूल_3.empty();
  अगर (absolute)
    remaining = remaining.drop_front(मूल_3.आकार_2());

  // Loop over path components manually. This makes it easier to detect
  // non-preferred slashes and double separators that must be canonicalized.
  जबतक (!remaining.empty()) {
    माप_प्रकार अग्र_slash = remaining.find_first_of(separators(style));
    अगर (अग्र_slash == StringRef::npos)
      अग्र_slash = remaining.आकार_2();
    StringRef component = remaining.take_front(अग्र_slash);
    remaining = remaining.drop_front(अग्र_slash);

    // Eat the slash, and check if it is the preferred separator.
    अगर (!remaining.empty()) {
      needs_change |= remaining.front() != preferred_separator(style);
      remaining = remaining.drop_front();
      // The path needs to be rewritten if it has a trailing slash.
      // FIXME: This is emergent behavior that could be removed.
      needs_change |= remaining.empty();
    }

    // Check for path traversal components or double separators.
    अगर (component.empty() || component == ".") {
      needs_change = true;
    } अन्यथा अगर (remove_dot_dot && component == "..") {
      needs_change = true;
      // Do not allow ".." to remove the root component. If this is the
      // beginning of a relative path, keep the ".." component.
      अगर (!components.empty() && components.back() != "..") {
        components.निकालो_back();
      } अन्यथा अगर (!absolute) {
        components.धकेलो_back(component);
      }
    } अन्यथा {
      components.धकेलो_back(component);
    }
  }

  SmallString<256> बफर_2 = मूल_3;
  // "root" could be "/", which may need to be translated into "\".
  make_preferred(बफर_2, style);
  needs_change |= मूल_3 != बफर_2;

  // Avoid rewriting the path unless we have to.
  अगर (!needs_change)
    वापस false;

  अगर (!components.empty()) {
    बफर_2 += components[0];
    क्रम (StringRef C : ArrayRef(components).drop_front()) {
      बफर_2 += preferred_separator(style);
      बफर_2 += C;
    }
  }
  the_path.swap(बफर_2);
  वापस true;
}

} // end namespace path

namespace fs {

std::error_code getUniqueID(स्थिर Twine Path, UniqueID &परिणाम) {
  sandbox::violationIfEnabled();

  file_status Status;
  std::error_code EC = status(Path, Status);
  अगर (EC)
    वापस EC;
  परिणाम = Status.getUniqueID();
  वापस std::error_code();
}

व्योम createUniquePath(स्थिर Twine &Model, SmallVectorImpl<अक्षर> &परिणाम_Path,
                      bool MakeAbsolute) {
  SmallString<128> ModelStorage;
  Model.toVector(ModelStorage);

  निश्चित(llvm::is_contained(ModelStorage, '%') &&
         "createUniquePath: Model must contain at least one '%'");

  अगर (MakeAbsolute) {
    // Make model absolute by prepending a temp directory if it's not already.
    अगर (!sys::path::is_absolute(Twine(ModelStorage))) {
      SmallString<128> TDir;
      sys::path::system_अस्थायी_directory(true, TDir);
      sys::path::append(TDir, Twine(ModelStorage));
      ModelStorage.swap(TDir);
    }
  }

  परिणाम_Path = ModelStorage;
  परिणाम_Path.धकेलो_back(0);
  परिणाम_Path.निकालो_back();

  // Replace '%' with random chars.
  क्रम (अचिन्हित i = 0, e = ModelStorage.आकार_2(); i != e; ++i) {
    अगर (ModelStorage[i] == '%')
      परिणाम_Path[i] = "0123456789abcdef"[sys::Process::Get_Random_संख्या() & 15];
  }
}

std::error_code createUniqueFile(स्थिर Twine &Model, पूर्णांक &परिणाम_Fd,
                                 SmallVectorImpl<अक्षर> &परिणाम_Path,
                                 OpenFlags Flags, अचिन्हित Mode) {
  वापस createUniqueEntity(Model, परिणाम_Fd, परिणाम_Path, false, FS_File, Flags,
                            Mode);
}

std::error_code createUniqueFile(स्थिर Twine &Model,
                                 SmallVectorImpl<अक्षर> &परिणाम_Path,
                                 अचिन्हित Mode) {
  पूर्णांक FD;
  स्वतः EC = createUniqueFile(Model, FD, परिणाम_Path, OF_None, Mode);
  अगर (EC)
    वापस EC;
  // FD is only needed to avoid race conditions. Close it right away.
  बंद(FD);
  वापस EC;
}

अटल std::error_code
createTemporaryFile(स्थिर Twine &Model, पूर्णांक &परिणाम_FD,
                    llvm::SmallVectorImpl<अक्षर> &परिणाम_Path, FSEntity Type,
                    sys::fs::OpenFlags Flags = sys::fs::OF_None) {
  // Any *temporary* file is assumed to be a compiler-internal output, not
  // a formal one.
  स्वतः BypassSandbox = sys::sandbox::scopedDisable();

  SmallString<128> Storage;
  StringRef P = Model.toNullTerminatedStringRef(Storage);
  निश्चित(P.find_first_of(separators(Style::native)) == StringRef::npos &&
         "Model must be a simple filename.");
  // Use P.begin() so that createUniqueEntity doesn't need to recreate Storage.
  वापस createUniqueEntity(P.begin(), परिणाम_FD, परिणाम_Path, true, Type, Flags,
                            all_read | all_write);
}

अटल std::error_code
createTemporaryFile(स्थिर Twine &Prefix, StringRef Suffix, पूर्णांक &परिणाम_FD,
                    llvm::SmallVectorImpl<अक्षर> &परिणाम_Path, FSEntity Type,
                    sys::fs::OpenFlags Flags = sys::fs::OF_None) {
  स्थिर अक्षर *मध्य = Suffix.empty() ? "-%%%%%%" : "-%%%%%%.";
  वापस createTemporaryFile(Prefix + मध्य + Suffix, परिणाम_FD, परिणाम_Path,
                             Type, Flags);
}

std::error_code createTemporaryFile(स्थिर Twine &Prefix, StringRef Suffix,
                                    पूर्णांक &परिणाम_FD,
                                    SmallVectorImpl<अक्षर> &परिणाम_Path,
                                    sys::fs::OpenFlags Flags) {
  वापस createTemporaryFile(Prefix, Suffix, परिणाम_FD, परिणाम_Path, FS_File,
                             Flags);
}

std::error_code createTemporaryFile(स्थिर Twine &Prefix, StringRef Suffix,
                                    SmallVectorImpl<अक्षर> &परिणाम_Path,
                                    sys::fs::OpenFlags Flags) {
  पूर्णांक FD;
  स्वतः EC = createTemporaryFile(Prefix, Suffix, FD, परिणाम_Path, Flags);
  अगर (EC)
    वापस EC;
  // FD is only needed to avoid race conditions. Close it right away.
  बंद(FD);
  वापस EC;
}

// This is a mkdtemp with a different pattern. We use createUniqueEntity mostly
// for consistency. We should try using mkdtemp.
std::error_code createUniqueDirectory(स्थिर Twine &Prefix,
                                      SmallVectorImpl<अक्षर> &परिणाम_Path) {
  पूर्णांक Dummy;
  वापस createUniqueEntity(Prefix + "-%%%%%%", Dummy, परिणाम_Path, true,
                            FS_Dir);
}

std::error_code
getPotentiallyUniqueFileName(स्थिर Twine &Model,
                             SmallVectorImpl<अक्षर> &परिणाम_Path) {
  पूर्णांक Dummy;
  वापस createUniqueEntity(Model, Dummy, परिणाम_Path, false, FS_Name);
}

std::error_code
get_Potentially_Unique_अस्थायी_File_Name(स्थिर Twine &Prefix, StringRef Suffix,
                                 SmallVectorImpl<अक्षर> &परिणाम_Path) {
  पूर्णांक Dummy;
  वापस createTemporaryFile(Prefix, Suffix, Dummy, परिणाम_Path, FS_Name);
}

std::error_code make_absolute(SmallVectorImpl<अक्षर> &path) {
  sandbox::violationIfEnabled();

  अगर (path::is_absolute(path))
    वापस {};

  SmallString<128> current_dir;
  अगर (std::error_code ec = current_path(current_dir))
    वापस ec;

  path::make_absolute(current_dir, path);
  वापस {};
}

std::error_code create_directories(स्थिर Twine &Path, bool IgnoreExisting,
                                   perms Perms) {
  SmallString<128> PathStorage;
  StringRef P = Path.toStringRef(PathStorage);

  // Be optimistic and try to create the directory
  std::error_code EC = create_directory(P, IgnoreExisting, Perms);
  // If we succeeded, or had any error other than the parent not existing, just
  // return it.
  अगर (EC != errc::no_such_file_or_directory)
    वापस EC;

  // We failed because of a no_such_file_or_directory, try to create the
  // parent.
  StringRef Parent = path::parent_path(P);
  अगर (Parent.empty())
    वापस EC;

  अगर ((EC = create_directories(Parent, IgnoreExisting, Perms)))
      वापस EC;

  वापस create_directory(P, IgnoreExisting, Perms);
}

अटल std::error_code copy_file_internal(पूर्णांक ReadFD, पूर्णांक WriteFD) {
  स्थिर माप_प्रकार Buf_आकार = 4096;
  अक्षर *Buf = नया अक्षर[Buf_आकार];
  पूर्णांक BytesRead = 0, BytesWritten = 0;
  क्रम (;;) {
    BytesRead = पढ़ो(ReadFD, Buf, Buf_आकार);
    अगर (BytesRead <= 0)
      अवरोध;
    जबतक (BytesRead) {
      BytesWritten = ग_लिखो(WriteFD, Buf, BytesRead);
      अगर (BytesWritten < 0)
        अवरोध;
      BytesRead -= BytesWritten;
    }
    अगर (BytesWritten < 0)
      अवरोध;
  }
  मिटाओ[] Buf;

  अगर (BytesRead < 0 || BytesWritten < 0)
    वापस errnoAsErrorCode();
  वापस std::error_code();
}

#अगर_अघोषित __APPLE__
std::error_code copy_file(स्थिर Twine &From, स्थिर Twine &To) {
  पूर्णांक ReadFD, WriteFD;
  अगर (std::error_code EC = openFileForRead(From, ReadFD, OF_None))
    वापस EC;
  अगर (std::error_code EC =
          openFileForWrite(To, WriteFD, CD_CreateAlways, OF_None)) {
    बंद(ReadFD);
    वापस EC;
  }

  std::error_code EC = copy_file_internal(ReadFD, WriteFD);

  बंद(ReadFD);
  बंद(WriteFD);

  वापस EC;
}
#पूर्ण_अगर

std::error_code copy_file(स्थिर Twine &From, पूर्णांक ToFD) {
  पूर्णांक ReadFD;
  अगर (std::error_code EC = openFileForRead(From, ReadFD, OF_None))
    वापस EC;

  std::error_code EC = copy_file_internal(ReadFD, ToFD);

  बंद(ReadFD);

  वापस EC;
}

ErrorOr<MD5::MD5_परिणाम> md5_contents(पूर्णांक FD) {
  sandbox::violationIfEnabled();

  MD5 Hash;

  constexpr माप_प्रकार Buf_आकार = 4096;
  std::vector<uint8_t> Buf(Buf_आकार);
  पूर्णांक BytesRead = 0;
  क्रम (;;) {
    BytesRead = पढ़ो(FD, Buf.तथ्य(), Buf_आकार);
    अगर (BytesRead <= 0)
      अवरोध;
    Hash.update(ArrayRef(Buf.तथ्य(), BytesRead));
  }

  अगर (BytesRead < 0)
    वापस errnoAsErrorCode();
  MD5::MD5_परिणाम परिणाम;
  Hash.final(परिणाम);
  वापस परिणाम;
}

ErrorOr<MD5::MD5_परिणाम> md5_contents(स्थिर Twine &Path) {
  sandbox::violationIfEnabled();

  पूर्णांक FD;
  अगर (स्वतः EC = openFileForRead(Path, FD, OF_None))
    वापस EC;

  स्वतः परिणाम = md5_contents(FD);
  बंद(FD);
  वापस परिणाम;
}

bool exists(स्थिर basic_file_status &status) {
  वापस status_known(status) && status.type() != file_type::file_not_found;
}

bool status_known(स्थिर basic_file_status &s) {
  वापस s.type() != file_type::status_error;
}

file_type get_file_type(स्थिर Twine &Path, bool Follow) {
  file_status st;
  अगर (status(Path, st, Follow))
    वापस file_type::status_error;
  वापस st.type();
}

bool is_directory(स्थिर basic_file_status &status) {
  वापस status.type() == file_type::directory_file;
}

std::error_code is_directory(स्थिर Twine &path, bool &परिणाम_2) {
  sandbox::violationIfEnabled();

  file_status st;
  अगर (std::error_code ec = status(path, st))
    वापस ec;
  परिणाम_2 = is_directory(st);
  वापस std::error_code();
}

bool is_regular_file(स्थिर basic_file_status &status) {
  वापस status.type() == file_type::regular_file;
}

std::error_code is_regular_file(स्थिर Twine &path, bool &परिणाम_2) {
  sandbox::violationIfEnabled();

  file_status st;
  अगर (std::error_code ec = status(path, st))
    वापस ec;
  परिणाम_2 = is_regular_file(st);
  वापस std::error_code();
}

bool is_symlink_file(स्थिर basic_file_status &status) {
  वापस status.type() == file_type::symlink_file;
}

std::error_code is_symlink_file(स्थिर Twine &path, bool &परिणाम_2) {
  sandbox::violationIfEnabled();

  file_status st;
  अगर (std::error_code ec = status(path, st, false))
    वापस ec;
  परिणाम_2 = is_symlink_file(st);
  वापस std::error_code();
}

bool is_other(स्थिर basic_file_status &status) {
  वापस exists(status) &&
         !is_regular_file(status) &&
         !is_directory(status);
}

std::error_code is_other(स्थिर Twine &Path, bool &परिणाम) {
  sandbox::violationIfEnabled();

  file_status FileStatus;
  अगर (std::error_code EC = status(Path, FileStatus))
    वापस EC;
  परिणाम = is_other(FileStatus);
  वापस std::error_code();
}

व्योम directory_entry::replace_filename(स्थिर Twine &Filename, file_type Type,
                                       basic_file_status Status) {
  SmallString<128> PathStr = path::parent_path(Path);
  path::append(PathStr, Filename);
  यह->Path = std::string(PathStr);
  यह->Type = Type;
  यह->Status = Status;
}

ErrorOr<perms> getPermissions(स्थिर Twine &Path) {
  sandbox::violationIfEnabled();

  file_status Status;
  अगर (std::error_code EC = status(Path, Status))
    वापस EC;

  वापस Status.permissions();
}

std::error_code setLastAccessAndModificationTime(स्थिर Twine &Path,
                                                 TimePoint<> AccessTime,
                                                 TimePoint<> ModificationTime) {
  पूर्णांक FD;
  अगर (std::error_code EC = openFile(Path, FD, CD_OpenExisting, FA_Read,
                                    OF_UpdateAttributes | OF_OpenDirectory))
    वापस EC;
  std::error_code EC =
      setLastAccessAndModificationTime(FD, AccessTime, ModificationTime);
  Process::SafelyCloseFileDescriptor(FD);
  वापस EC;
}

माप_प्रकार mapped_file_region::आकार_2() स्थिर {
  निश्चित(Mapping && "Mapping failed but used anyway!");
  वापस आकार;
}

अक्षर *mapped_file_region::तथ्य() स्थिर {
  निश्चित(Mapping && "Mapping failed but used anyway!");
  वापस reinterpret_cast<अक्षर *>(Mapping);
}

स्थिर अक्षर *mapped_file_region::const_तथ्य() स्थिर {
  निश्चित(Mapping && "Mapping failed but used anyway!");
  वापस reinterpret_cast<स्थिर अक्षर *>(Mapping);
}

Error readNativeFileToEOF(file_t FileHandle, SmallVectorImpl<अक्षर> &बफर,
                          ssize_t Chunk_आकार) {
  sandbox::violationIfEnabled();

  // Install a handler to truncate the buffer to the correct size on exit.
  माप_प्रकार आकार = बफर.आकार_2();
  llvm::scope_exit TruncateOnExit([&]() { बफर.truncate(आकार); });

  // Read into Buffer until we hit EOF.
  क्रम (;;) {
    बफर.resize_for_overwrite(आकार + Chunk_आकार);
    Expected<माप_प्रकार> ReadBytes = readNativeFile(
        FileHandle, MutableArrayRef(बफर.begin() + आकार, Chunk_आकार));
    अगर (!ReadBytes)
      वापस ReadBytes.takeError();
    अगर (*ReadBytes == 0)
      वापस Error::success();
    आकार += *ReadBytes;
  }
}

} // end namespace fs
} // end namespace sys
} // end namespace llvm

// Include the truly platform-specific parts.
#अगर defined(LLVM_ON_UNIX)
#समावेश "Unix/Path.inc"
#पूर्ण_अगर
#अगर defined(_WIN32)
#समावेश "Windows/Path.inc"
#पूर्ण_अगर

namespace llvm {
namespace sys {
namespace fs {

अस्थायी_File::अस्थायी_File(StringRef Name, पूर्णांक FD)
    : TmpName(std::string(Name)), FD(FD) {}
अस्थायी_File::अस्थायी_File(अस्थायी_File &&Other) { *यह = std::move(Other); }
अस्थायी_File &अस्थायी_File::चालक=(अस्थायी_File &&Other) {
  TmpName = std::move(Other.TmpName);
  FD = Other.FD;
  Other.Done = true;
  Other.FD = -1;
#अगर_घोषित _WIN32
  RemoveOnClose = Other.RemoveOnClose;
  Other.RemoveOnClose = false;
#पूर्ण_अगर
  वापस *यह;
}

अस्थायी_File::~अस्थायी_File() { निश्चित(Done); }

Error अस्थायी_File::discard() {
  Done = true;
  अगर (FD != -1 && बंद(FD) == -1) {
    std::error_code EC = errnoAsErrorCode();
    वापस errorCodeToError(EC);
  }
  FD = -1;

#अगर_घोषित _WIN32
  // On Windows, closing will remove the file, if we set the delete
  // disposition. If not, remove it manually.
  bool Remove = RemoveOnClose;
#अन्यथा
  // Always try to remove the file.
  bool Remove = true;
#पूर्ण_अगर
  std::error_code RemoveEC;
  अगर (Remove && !TmpName.empty()) {
    RemoveEC = fs::हटाओ(TmpName);
    sys::DontRemoveFileOnSignal(TmpName);
    अगर (!RemoveEC)
      TmpName = "";
  } अन्यथा {
    TmpName = "";
  }
  वापस errorCodeToError(RemoveEC);
}

Error अस्थायी_File::keep(स्थिर Twine &Name) {
  निश्चित(!Done);
  Done = true;
  // Always try to close and rename.
#अगर_घोषित _WIN32
  // If we can't cancel the delete don't rename.
  स्वतः H = reinterpret_cast<HANDLE>(_get_osfhandle(FD));
  std::error_code RenameEC =
      RemoveOnClose ? std::error_code() : setDeleteDisposition(H, false);
  bool ShouldDelete = false;
  अगर (!RenameEC) {
    RenameEC = rename_handle(H, Name);
    // If rename failed because it's cross-device, copy instead
    अगर (RenameEC ==
      std::error_code(ERROR_NOT_SAME_DEVICE, std::system_category())) {
      RenameEC = copy_file(TmpName, Name);
      ShouldDelete = true;
    }
  }

  // If we can't rename or copy, discard the temporary file.
  अगर (RenameEC)
    ShouldDelete = true;
  अगर (ShouldDelete) {
    अगर (!RemoveOnClose)
      setDeleteDisposition(H, true);
    अन्यथा
      हटाओ(TmpName);
  }
#अन्यथा
  std::error_code RenameEC = fs::नाम(TmpName, Name);
  अगर (RenameEC) {
    // If we can't rename, try to copy to work around cross-device link issues.
    RenameEC = sys::fs::copy_file(TmpName, Name);
    // If we can't rename or copy, discard the temporary file.
    अगर (RenameEC)
      हटाओ(TmpName);
  }
#पूर्ण_अगर
  sys::DontRemoveFileOnSignal(TmpName);

  अगर (!RenameEC)
    TmpName = "";

  अगर (बंद(FD) == -1)
    वापस errorCodeToError(errnoAsErrorCode());
  FD = -1;

  वापस errorCodeToError(RenameEC);
}

Error अस्थायी_File::keep() {
  निश्चित(!Done);
  Done = true;

#अगर_घोषित _WIN32
  स्वतः H = reinterpret_cast<HANDLE>(_get_osfhandle(FD));
  अगर (std::error_code EC = setDeleteDisposition(H, false))
    वापस errorCodeToError(EC);
#पूर्ण_अगर
  sys::DontRemoveFileOnSignal(TmpName);

  TmpName = "";

  अगर (बंद(FD) == -1)
    वापस errorCodeToError(errnoAsErrorCode());
  FD = -1;

  वापस Error::success();
}

Expected<अस्थायी_File> अस्थायी_File::create(स्थिर Twine &Model, अचिन्हित Mode,
                                    OpenFlags ExtraFlags) {
  पूर्णांक FD;
  SmallString<128> परिणाम_Path;
  अगर (std::error_code EC =
          createUniqueFile(Model, FD, परिणाम_Path, OF_Delete | ExtraFlags, Mode))
    वापस errorCodeToError(EC);

  अस्थायी_File Ret(परिणाम_Path, FD);
#अगर_घोषित _WIN32
  स्वतः H = reinterpret_cast<HANDLE>(_get_osfhandle(FD));
  bool SetSignalHandler = false;
  अगर (std::error_code EC = setDeleteDisposition(H, true)) {
    Ret.RemoveOnClose = true;
    SetSignalHandler = true;
  }
#अन्यथा
  bool SetSignalHandler = true;
#पूर्ण_अगर
  अगर (SetSignalHandler && sys::RemoveFileOnSignal(परिणाम_Path)) {
    // Make sure we delete the file when RemoveFileOnSignal fails.
    consumeError(Ret.discard());
    std::error_code EC(errc::operation_not_permitted);
    वापस errorCodeToError(EC);
  }
  वापस std::move(Ret);
}
} // namespace fs

} // namespace sys
} // namespace llvm
