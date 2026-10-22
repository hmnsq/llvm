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
        फर्काऊ *Env == "1";
      फर्काऊ static_cast<तार्किक>(LLVM_WINDOWS_PREFER_FORWARD_SLASH);
    }();
    फर्काऊ prefer;
  }

  अन्तःस्थ Style real_style(Style style) {
    यदि (style != Style::native)
      फर्काऊ style;
    यदि (is_style_posix(style))
      फर्काऊ Style::posix;
    फर्काऊ prefer_forward_slash() ? Style::windows_slash
                                  : Style::windows_backslash;
  }

  अन्तःस्थ अचर वर्ण *separators(Style style) {
    यदि (is_style_windows(style))
      फर्काऊ "\\/";
    फर्काऊ "/";
  }

  अन्तःस्थ वर्ण preferred_separator(Style style) {
    यदि (real_style(style) == Style::windows)
      फर्काऊ '\\';
    फर्काऊ '/';
  }

  StringRef find_first_component(StringRef path, Style style) {
    // Look for this first component in the following order.
    // * empty (in this case we return an empty string)
    // * either C: or {//,\\}net.
    // * {/,\}
    // * {file,directory}name

    यदि (path.empty())
      फर्काऊ path;

    यदि (is_style_windows(style)) {
      // C:
      यदि (path.size() >= 2 &&
          std::है_अक्षर(static_cast<अचिन्हित वर्ण>(path[0])) && path[1] == ':')
        फर्काऊ path.substr(0, 2);
    }

    // //net
    यदि ((path.size() > 2) && is_separator(path[0], style) &&
        path[0] == path[1] && !is_separator(path[2], style)) {
      // Find the next directory separator.
      माप_प्रकार end = path.find_first_of(separators(style), 2);
      फर्काऊ path.substr(0, end);
    }

    // {/,\}
    यदि (is_separator(path[0], style))
      फर्काऊ path.substr(0, 1);

    // * {file,directory}name
    माप_प्रकार end = path.find_first_of(separators(style));
    फर्काऊ path.substr(0, end);
  }

  // Returns the first character of the filename in str. For paths ending in
  // '/', it returns the position of the '/'.
  माप_प्रकार filename_pos(StringRef str, Style style) {
    यदि (str.size() > 0 && is_separator(str[str.size() - 1], style))
      फर्काऊ str.size() - 1;

    माप_प्रकार pos = str.find_last_of(separators(style), str.size() - 1);

    यदि (is_style_windows(style)) {
      यदि (pos == StringRef::npos)
        pos = str.find_last_of(':', str.size() - 1);
    }

    यदि (pos == StringRef::npos || (pos == 1 && is_separator(str[0], style)))
      फर्काऊ 0;

    फर्काऊ pos + 1;
  }

  // Returns the position of the root directory in str. If there is no root
  // directory in str, it returns StringRef::npos.
  माप_प्रकार जरा_dir_start(StringRef str, Style style) {
    // case "c:/"
    यदि (is_style_windows(style)) {
      यदि (str.size() > 2 && str[1] == ':' && is_separator(str[2], style))
        फर्काऊ 2;
    }

    // case "//net"
    यदि (str.size() > 3 && is_separator(str[0], style) && str[0] == str[1] &&
        !is_separator(str[2], style)) {
      फर्काऊ str.find_first_of(separators(style), 2);
    }

    // case "/"
    यदि (str.size() > 0 && is_separator(str[0], style))
      फर्काऊ 0;

    फर्काऊ StringRef::npos;
  }

  // Returns the position past the end of the "parent path" of path. The parent
  // path will not end in '/', unless the parent is the root directory. If the
  // path has no parent, 0 is returned.
  माप_प्रकार parent_path_end(StringRef path, Style style) {
    माप_प्रकार end_pos = filename_pos(path, style);

    तार्किक filename_was_sep =
        path.size() > 0 && is_separator(path[end_pos], style);

    // Skip separators until we reach root dir (or the start of the string).
    माप_प्रकार जरा_dir_pos = जरा_dir_start(path, style);
    जबसम्म (end_pos > 0 &&
           (जरा_dir_pos == StringRef::npos || end_pos > जरा_dir_pos) &&
           is_separator(path[end_pos - 1], style))
      --end_pos;

    यदि (end_pos == जरा_dir_pos && !filename_was_sep) {
      // We've reached the root dir and the input path was *not* ending in a
      // sequence of slashes. Include the root dir in the parent path.
      फर्काऊ जरा_dir_pos + 1;
    }

    // Otherwise, just include before the last slash.
    फर्काऊ end_pos;
  }
} // end unnamed namespace

गणना FSEntity {
  FS_Dir,
  FS_File,
  FS_Name
};

स्थिर std::error_code
createUniqueEntity(अचर Twine &Model, पूर्णाङ्क &ResultFD,
                   SmallVectorImpl<वर्ण> &ResultPath, तार्किक MakeAbsolute,
                   FSEntity Type, sys::fs::OpenFlags Flags = sys::fs::OF_None,
                   अचिन्हित Mode = 0) {

  // Limit the number of attempts we make, so that we don't infinite loop. E.g.
  // "permission denied" could be for a specific file (so we retry with a
  // different name) or for the whole directory (retry would always fail).
  // Checking which is racy, so we try a number of times, then give up.
  std::error_code EC;
  लागि (पूर्णाङ्क Retries = 128; Retries > 0; --Retries) {
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
          जारी;
        फर्काऊ EC;
      }

      फर्काऊ std::error_code();
    }

    पक्ष FS_Name: {
      EC = sys::fs::access(ResultPath.begin(), sys::fs::AccessMode::Exist);
      यदि (EC == errc::no_such_file_or_directory)
        फर्काऊ std::error_code();
      यदि (EC)
        फर्काऊ EC;
      जारी;
    }

    पक्ष FS_Dir: {
      EC = sys::fs::create_directory(ResultPath.begin(), असत्य);
      यदि (EC) {
        यदि (EC == errc::file_exists)
          जारी;
        फर्काऊ EC;
      }
      फर्काऊ std::error_code();
    }
    }
    llvm_unreachable("Invalid Type");
  }
  फर्काऊ EC;
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
  फर्काऊ i;
}

const_iterator end(StringRef path) {
  const_iterator i;
  i.Path      = path;
  i.Position  = path.size();
  फर्काऊ i;
}

const_iterator &const_iterator::चालक++() {
  प्रतिज्ञा(Position < Path.size() && "Tried to increment past end!");

  // Increment Position to past the current component
  Position += Component.size();

  // Check for end.
  यदि (Position == Path.size()) {
    Component = StringRef();
    फर्काऊ *एतत्;
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
      फर्काऊ *एतत्;
    }

    // Skip extra separators.
    जबसम्म (Position != Path.size() && is_separator(Path[Position], S)) {
      ++Position;
    }

    // Treat trailing '/' as a '.', unless it is the root dir.
    यदि (Position == Path.size() && Component != "/") {
      --Position;
      Component = ".";
      फर्काऊ *एतत्;
    }
  }

  // Find next component.
  माप_प्रकार end_pos = Path.find_first_of(separators(S), Position);
  Component = Path.slice(Position, end_pos);

  फर्काऊ *एतत्;
}

तार्किक const_iterator::चालक==(अचर const_iterator &RHS) अचर {
  फर्काऊ Path.begin() == RHS.Path.begin() && Position == RHS.Position;
}

सूचक_भेद_प्रकार const_iterator::चालक-(अचर const_iterator &RHS) अचर {
  फर्काऊ Position - RHS.Position;
}

reverse_iterator rbegin(StringRef Path, Style style) {
  reverse_iterator I;
  I.Path = Path;
  I.Position = Path.size();
  I.S = style;
  ++I;
  फर्काऊ I;
}

reverse_iterator rend(StringRef Path) {
  reverse_iterator I;
  I.Path = Path;
  I.Component = Path.substr(0, 0);
  I.Position = 0;
  फर्काऊ I;
}

reverse_iterator &reverse_iterator::चालक++() {
  माप_प्रकार जरा_dir_pos = जरा_dir_start(Path, S);

  // Skip separators unless it's the root directory.
  माप_प्रकार end_pos = Position;
  जबसम्म (end_pos > 0 && (end_pos - 1) != जरा_dir_pos &&
         is_separator(Path[end_pos - 1], S))
    --end_pos;

  // Treat trailing '/' as a '.', unless it is the root dir.
  यदि (Position == Path.size() && !Path.empty() &&
      is_separator(Path.back(), S) &&
      (जरा_dir_pos == StringRef::npos || end_pos - 1 > जरा_dir_pos)) {
    --Position;
    Component = ".";
    फर्काऊ *एतत्;
  }

  // Find next separator.
  माप_प्रकार start_pos = filename_pos(Path.substr(0, end_pos), S);
  Component = Path.slice(start_pos, end_pos);
  Position = start_pos;
  फर्काऊ *एतत्;
}

तार्किक reverse_iterator::चालक==(अचर reverse_iterator &RHS) अचर {
  फर्काऊ Path.begin() == RHS.Path.begin() && Component == RHS.Component &&
         Position == RHS.Position;
}

सूचक_भेद_प्रकार reverse_iterator::चालक-(अचर reverse_iterator &RHS) अचर {
  फर्काऊ Position - RHS.Position;
}

StringRef जरा_path(StringRef path, Style style) {
  const_iterator b = begin(path, style), pos = b, e = end(path);
  यदि (b != e) {
    तार्किक has_net =
        b->size() > 2 && is_separator((*b)[0], style) && (*b)[1] == (*b)[0];
    तार्किक has_drive = is_style_windows(style) && b->ends_with(":");

    यदि (has_net || has_drive) {
      यदि ((++pos != e) && is_separator((*pos)[0], style)) {
        // {C:/,//net/}, so get the first two components.
        फर्काऊ path.substr(0, b->size() + pos->size());
      }
      // just {C:,//net}, return the first component.
      फर्काऊ *b;
    }

    // POSIX style root directory.
    यदि (is_separator((*b)[0], style)) {
      फर्काऊ *b;
    }
  }

  फर्काऊ StringRef();
}

StringRef जरा_name(StringRef path, Style style) {
  const_iterator b = begin(path, style), e = end(path);
  यदि (b != e) {
    तार्किक has_net =
        b->size() > 2 && is_separator((*b)[0], style) && (*b)[1] == (*b)[0];
    तार्किक has_drive = is_style_windows(style) && b->ends_with(":");

    यदि (has_net || has_drive) {
      // just {C:,//net}, return the first component.
      फर्काऊ *b;
    }
  }

  // No path or no name.
  फर्काऊ StringRef();
}

StringRef जरा_directory(StringRef path, Style style) {
  const_iterator b = begin(path, style), pos = b, e = end(path);
  यदि (b != e) {
    तार्किक has_net =
        b->size() > 2 && is_separator((*b)[0], style) && (*b)[1] == (*b)[0];
    तार्किक has_drive = is_style_windows(style) && b->ends_with(":");

    यदि ((has_net || has_drive) &&
        // {C:,//net}, skip to the next component.
        (++pos != e) && is_separator((*pos)[0], style)) {
      फर्काऊ *pos;
    }

    // POSIX style root directory.
    यदि (!has_net && is_separator((*b)[0], style)) {
      फर्काऊ *b;
    }
  }

  // No path or no root.
  फर्काऊ StringRef();
}

StringRef relative_path(StringRef path, Style style) {
  StringRef जरा_3 = जरा_path(path, style);
  फर्काऊ path.substr(जरा_3.size());
}

शून्य append(SmallVectorImpl<वर्ण> &path, Style style, अचर Twine &a,
            अचर Twine &b, अचर Twine &c, अचर Twine &d) {
  SmallString<32> a_storage;
  SmallString<32> b_storage;
  SmallString<32> c_storage;
  SmallString<32> d_storage;

  SmallVector<StringRef, 4> components;
  यदि (!a.isTriviallyEmpty()) components.थप_back(a.toStringRef(a_storage));
  यदि (!b.isTriviallyEmpty()) components.थप_back(b.toStringRef(b_storage));
  यदि (!c.isTriviallyEmpty()) components.थप_back(c.toStringRef(c_storage));
  यदि (!d.isTriviallyEmpty()) components.थप_back(d.toStringRef(d_storage));

  लागि (स्वतः &component : components) {
    तार्किक path_has_sep =
        !path.empty() && is_separator(path[path.size() - 1], style);
    यदि (path_has_sep) {
      // Strip separators from beginning of component.
      माप_प्रकार loc = component.find_first_not_of(separators(style));
      StringRef c = component.substr(loc);

      // Append it.
      path.append(c.begin(), c.end());
      जारी;
    }

    तार्किक component_has_sep =
        !component.empty() && is_separator(component[0], style);
    यदि (!component_has_sep &&
        !(path.empty() || has_जरा_name(component, style))) {
      // Add a separator.
      path.थप_back(preferred_separator(style));
    }

    path.append(component.begin(), component.end());
  }
}

शून्य append(SmallVectorImpl<वर्ण> &path, अचर Twine &a, अचर Twine &b,
            अचर Twine &c, अचर Twine &d) {
  append(path, Style::native, a, b, c, d);
}

शून्य append(SmallVectorImpl<वर्ण> &path, const_iterator begin,
            const_iterator end, Style style) {
  लागि (; begin != end; ++begin)
    path::append(path, style, *begin);
}

StringRef parent_path(StringRef path, Style style) {
  माप_प्रकार end_pos = parent_path_end(path, style);
  यदि (end_pos == StringRef::npos)
    फर्काऊ StringRef();
  फर्काऊ path.substr(0, end_pos);
}

शून्य remove_filename(SmallVectorImpl<वर्ण> &path, Style style) {
  माप_प्रकार end_pos = parent_path_end(StringRef(path.begin(), path.size()), style);
  यदि (end_pos != StringRef::npos)
    path.truncate(end_pos);
}

शून्य replace_extension(SmallVectorImpl<वर्ण> &path, अचर Twine &extension,
                       Style style) {
  StringRef p(path.begin(), path.size());
  SmallString<32> ext_storage;
  StringRef ext = extension.toStringRef(ext_storage);

  // Erase existing extension.
  माप_प्रकार pos = p.find_last_of('.');
  यदि (pos != StringRef::npos && pos >= filename_pos(p, style))
    path.truncate(pos);

  // Append '.' if needed.
  यदि (ext.size() > 0 && ext[0] != '.')
    path.थप_back('.');

  // Append extension.
  path.append(ext.begin(), ext.end());
}

स्थिर तार्किक starts_with(StringRef Path, StringRef Prefix,
                        Style style = Style::native) {
  // Windows prefix matching : case and separator insensitive
  यदि (is_style_windows(style)) {
    यदि (Path.size() < Prefix.size())
      फर्काऊ असत्य;
    लागि (माप_प्रकार I = 0, E = Prefix.size(); I != E; ++I) {
      तार्किक SepPath = is_separator(Path[I], style);
      तार्किक SepPrefix = is_separator(Prefix[I], style);
      यदि (SepPath != SepPrefix)
        फर्काऊ असत्य;
      यदि (!SepPath && toLower(Path[I]) != toLower(Prefix[I]))
        फर्काऊ असत्य;
    }
    फर्काऊ सत्य;
  }
  फर्काऊ Path.starts_with(Prefix);
}

तार्किक replace_path_prefix(SmallVectorImpl<वर्ण> &Path, StringRef OldPrefix,
                         StringRef NewPrefix, Style style) {
  यदि (OldPrefix.empty() && NewPrefix.empty())
    फर्काऊ असत्य;

  StringRef OrigPath(Path.begin(), Path.size());
  यदि (!starts_with(OrigPath, OldPrefix, style))
    फर्काऊ असत्य;

  // If prefixes have the same size we can simply copy the new one over.
  यदि (OldPrefix.size() == NewPrefix.size()) {
    llvm::copy(NewPrefix, Path.begin());
    फर्काऊ सत्य;
  }

  StringRef RelPath = OrigPath.substr(OldPrefix.size());
  SmallString<256> NewPath;
  (Twine(NewPrefix) + RelPath).toVector(NewPath);
  Path.swap(NewPath);
  फर्काऊ सत्य;
}

शून्य native(अचर Twine &path, SmallVectorImpl<वर्ण> &result, Style style) {
  प्रतिज्ञा((!path.isSingleStringRef() ||
          path.getSingleStringRef().data() != result.data()) &&
         "path and result are not allowed to overlap!");
  // Clear result.
  result.clear();
  path.toVector(result);
  native(result, style);
}

std::string native(अचर Twine &path, Style style) {
  SmallString<128> Result;
  native(path, Result, style);
  फर्काऊ std::string(Result);
}

शून्य native(SmallVectorImpl<वर्ण> &Path, Style style) {
  यदि (Path.empty())
    फर्काऊ;
  यदि (is_style_windows(style)) {
    लागि (वर्ण &Ch : Path)
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
    फर्काऊ std::string(path);

  std::string s = path.str();
  llvm::replace(s, '\\', '/');
  फर्काऊ s;
}

StringRef filename(StringRef path, Style style) { फर्काऊ *rbegin(path, style); }

StringRef stem(StringRef path, Style style) {
  StringRef fname = filename(path, style);
  माप_प्रकार pos = fname.find_last_of('.');
  यदि (pos == StringRef::npos)
    फर्काऊ fname;
  यदि ((fname.size() == 1 && fname == ".") ||
      (fname.size() == 2 && fname == ".."))
    फर्काऊ fname;
  फर्काऊ fname.substr(0, pos);
}

StringRef extension(StringRef path, Style style) {
  StringRef fname = filename(path, style);
  माप_प्रकार pos = fname.find_last_of('.');
  यदि (pos == StringRef::npos)
    फर्काऊ StringRef();
  यदि ((fname.size() == 1 && fname == ".") ||
      (fname.size() == 2 && fname == ".."))
    फर्काऊ StringRef();
  फर्काऊ fname.substr(pos);
}

तार्किक is_separator(वर्ण मान_2, Style style) {
  यदि (मान_2 == '/')
    फर्काऊ सत्य;
  यदि (is_style_windows(style))
    फर्काऊ मान_2 == '\\';
  फर्काऊ असत्य;
}

StringRef get_separator(Style style) {
  यदि (real_style(style) == Style::windows)
    फर्काऊ "\\";
  फर्काऊ "/";
}

तार्किक has_जरा_name(अचर Twine &path, Style style) {
  SmallString<128> path_storage;
  StringRef p = path.toStringRef(path_storage);

  फर्काऊ !जरा_name(p, style).empty();
}

तार्किक has_जरा_directory(अचर Twine &path, Style style) {
  SmallString<128> path_storage;
  StringRef p = path.toStringRef(path_storage);

  फर्काऊ !जरा_directory(p, style).empty();
}

तार्किक has_जरा_path(अचर Twine &path, Style style) {
  SmallString<128> path_storage;
  StringRef p = path.toStringRef(path_storage);

  फर्काऊ !जरा_path(p, style).empty();
}

तार्किक has_relative_path(अचर Twine &path, Style style) {
  SmallString<128> path_storage;
  StringRef p = path.toStringRef(path_storage);

  फर्काऊ !relative_path(p, style).empty();
}

तार्किक has_filename(अचर Twine &path, Style style) {
  SmallString<128> path_storage;
  StringRef p = path.toStringRef(path_storage);

  फर्काऊ !filename(p, style).empty();
}

तार्किक has_parent_path(अचर Twine &path, Style style) {
  SmallString<128> path_storage;
  StringRef p = path.toStringRef(path_storage);

  फर्काऊ !parent_path(p, style).empty();
}

तार्किक has_stem(अचर Twine &path, Style style) {
  SmallString<128> path_storage;
  StringRef p = path.toStringRef(path_storage);

  फर्काऊ !stem(p, style).empty();
}

तार्किक has_extension(अचर Twine &path, Style style) {
  SmallString<128> path_storage;
  StringRef p = path.toStringRef(path_storage);

  फर्काऊ !extension(p, style).empty();
}

तार्किक is_absolute(अचर Twine &path, Style style) {
  SmallString<128> path_storage;
  StringRef p = path.toStringRef(path_storage);

  तार्किक जरा_Dir = has_जरा_directory(p, style);
  तार्किक जरा_Name = is_style_posix(style) || has_जरा_name(p, style);

  फर्काऊ जरा_Dir && जरा_Name;
}

तार्किक is_absolute_gnu(अचर Twine &path, Style style) {
  SmallString<128> path_storage;
  StringRef p = path.toStringRef(path_storage);

  // Handle '/' which is absolute for both Windows and POSIX systems.
  // Handle '\\' on Windows.
  यदि (!p.empty() && is_separator(p.front(), style))
    फर्काऊ सत्य;

  यदि (is_style_windows(style)) {
    // Handle drive letter pattern (a character followed by ':') on Windows.
    यदि (p.size() >= 2 && (p[0] && p[1] == ':'))
      फर्काऊ सत्य;
  }

  फर्काऊ असत्य;
}

तार्किक is_relative(अचर Twine &path, Style style) {
  फर्काऊ !is_absolute(path, style);
}

शून्य make_absolute(अचर Twine &current_directory,
                   SmallVectorImpl<वर्ण> &path) {
  StringRef p(path.data(), path.size());

  तार्किक जरा_Directory = has_जरा_directory(p);
  तार्किक जरा_Name = has_जरा_name(p);

  // Already absolute.
  यदि ((जरा_Name || is_style_posix(Style::native)) && जरा_Directory)
    फर्काऊ;

  // All the following conditions will need the current directory.
  SmallString<128> current_dir;
  current_directory.toVector(current_dir);

  // Relative path. Prepend the current directory.
  यदि (!जरा_Name && !जरा_Directory) {
    // Append path to the current directory.
    append(current_dir, p);
    // Set path to the result.
    path.swap(current_dir);
    फर्काऊ;
  }

  यदि (!जरा_Name && जरा_Directory) {
    StringRef cdrn = जरा_name(current_dir);
    SmallString<128> cur_Dir_जरा_Name(cdrn.begin(), cdrn.end());
    append(cur_Dir_जरा_Name, p);
    // Set path to the result.
    path.swap(cur_Dir_जरा_Name);
    फर्काऊ;
  }

  यदि (जरा_Name && !जरा_Directory) {
    StringRef p_जरा_Name = जरा_name(p);
    StringRef b_जरा_Directory = जरा_directory(current_dir);
    StringRef bRelativePath = relative_path(current_dir);
    StringRef pRelativePath = relative_path(p);

    SmallString<128> res;
    append(res, p_जरा_Name, b_जरा_Directory, bRelativePath, pRelativePath);
    path.swap(res);
    फर्काऊ;
  }

  llvm_unreachable("All rootName and rootDirectory combinations should have "
                   "occurred above!");
}

StringRef remove_leading_dotslash(StringRef Path, Style style) {
  // Remove leading "./" (or ".//" or "././" etc.)
  जबसम्म (Path.size() > 2 && Path[0] == '.' && is_separator(Path[1], style)) {
    Path = Path.substr(2);
    जबसम्म (Path.size() > 0 && is_separator(Path[0], style))
      Path = Path.substr(1);
  }
  फर्काऊ Path;
}

तार्किक remove_dots(SmallVectorImpl<वर्ण> &the_path, तार्किक remove_dot_dot,
                 Style style) {
  style = real_style(style);
  StringRef remaining(the_path.data(), the_path.size());
  तार्किक needs_change = असत्य;
  SmallVector<StringRef, 16> components;

  // Consume the root path, if present.
  StringRef जरा_3 = path::जरा_path(remaining, style);
  तार्किक absolute = !जरा_3.empty();
  यदि (absolute)
    remaining = remaining.drop_front(जरा_3.size());

  // Loop over path components manually. This makes it easier to detect
  // non-preferred slashes and double separators that must be canonicalized.
  जबसम्म (!remaining.empty()) {
    माप_प्रकार next_slash = remaining.find_first_of(separators(style));
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
        components.झिक_back();
      } अन्यथा यदि (!absolute) {
        components.थप_back(component);
      }
    } अन्यथा {
      components.थप_back(component);
    }
  }

  SmallString<256> buffer = जरा_3;
  // "root" could be "/", which may need to be translated into "\".
  make_preferred(buffer, style);
  needs_change |= जरा_3 != buffer;

  // Avoid rewriting the path unless we have to.
  यदि (!needs_change)
    फर्काऊ असत्य;

  यदि (!components.empty()) {
    buffer += components[0];
    लागि (StringRef C : ArrayRef(components).drop_front()) {
      buffer += preferred_separator(style);
      buffer += C;
    }
  }
  the_path.swap(buffer);
  फर्काऊ सत्य;
}

} // end namespace path

नामाकाश fs {

std::error_code getUniqueID(अचर Twine Path, UniqueID &Result) {
  sandbox::violationIfEnabled();

  file_status Status;
  std::error_code EC = status(Path, Status);
  यदि (EC)
    फर्काऊ EC;
  Result = Status.getUniqueID();
  फर्काऊ std::error_code();
}

शून्य createUniquePath(अचर Twine &Model, SmallVectorImpl<वर्ण> &ResultPath,
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
  ResultPath.थप_back(0);
  ResultPath.झिक_back();

  // Replace '%' with random chars.
  लागि (अचिन्हित i = 0, e = ModelStorage.size(); i != e; ++i) {
    यदि (ModelStorage[i] == '%')
      ResultPath[i] = "0123456789abcdef"[sys::Process::Get_Random_सङ्ख्या() & 15];
  }
}

std::error_code createUniqueFile(अचर Twine &Model, पूर्णाङ्क &ResultFd,
                                 SmallVectorImpl<वर्ण> &ResultPath,
                                 OpenFlags Flags, अचिन्हित Mode) {
  फर्काऊ createUniqueEntity(Model, ResultFd, ResultPath, असत्य, FS_File, Flags,
                            Mode);
}

std::error_code createUniqueFile(अचर Twine &Model,
                                 SmallVectorImpl<वर्ण> &ResultPath,
                                 अचिन्हित Mode) {
  पूर्णाङ्क FD;
  स्वतः EC = createUniqueFile(Model, FD, ResultPath, OF_None, Mode);
  यदि (EC)
    फर्काऊ EC;
  // FD is only needed to avoid race conditions. Close it right away.
  बंद(FD);
  फर्काऊ EC;
}

स्थिर std::error_code
createTemporaryFile(अचर Twine &Model, पूर्णाङ्क &ResultFD,
                    llvm::SmallVectorImpl<वर्ण> &ResultPath, FSEntity Type,
                    sys::fs::OpenFlags Flags = sys::fs::OF_None) {
  // Any *temporary* file is assumed to be a compiler-internal output, not
  // a formal one.
  स्वतः BypassSandbox = sys::sandbox::scopedDisable();

  SmallString<128> Storage;
  StringRef P = Model.toNullTerminatedStringRef(Storage);
  प्रतिज्ञा(P.find_first_of(separators(Style::native)) == StringRef::npos &&
         "Model must be a simple filename.");
  // Use P.begin() so that createUniqueEntity doesn't need to recreate Storage.
  फर्काऊ createUniqueEntity(P.begin(), ResultFD, ResultPath, सत्य, Type, Flags,
                            all_read | all_write);
}

स्थिर std::error_code
createTemporaryFile(अचर Twine &Prefix, StringRef Suffix, पूर्णाङ्क &ResultFD,
                    llvm::SmallVectorImpl<वर्ण> &ResultPath, FSEntity Type,
                    sys::fs::OpenFlags Flags = sys::fs::OF_None) {
  अचर वर्ण *बीच = Suffix.empty() ? "-%%%%%%" : "-%%%%%%.";
  फर्काऊ createTemporaryFile(Prefix + बीच + Suffix, ResultFD, ResultPath,
                             Type, Flags);
}

std::error_code createTemporaryFile(अचर Twine &Prefix, StringRef Suffix,
                                    पूर्णाङ्क &ResultFD,
                                    SmallVectorImpl<वर्ण> &ResultPath,
                                    sys::fs::OpenFlags Flags) {
  फर्काऊ createTemporaryFile(Prefix, Suffix, ResultFD, ResultPath, FS_File,
                             Flags);
}

std::error_code createTemporaryFile(अचर Twine &Prefix, StringRef Suffix,
                                    SmallVectorImpl<वर्ण> &ResultPath,
                                    sys::fs::OpenFlags Flags) {
  पूर्णाङ्क FD;
  स्वतः EC = createTemporaryFile(Prefix, Suffix, FD, ResultPath, Flags);
  यदि (EC)
    फर्काऊ EC;
  // FD is only needed to avoid race conditions. Close it right away.
  बंद(FD);
  फर्काऊ EC;
}

// This is a mkdtemp with a different pattern. We use createUniqueEntity mostly
// for consistency. We should try using mkdtemp.
std::error_code createUniqueDirectory(अचर Twine &Prefix,
                                      SmallVectorImpl<वर्ण> &ResultPath) {
  पूर्णाङ्क Dummy;
  फर्काऊ createUniqueEntity(Prefix + "-%%%%%%", Dummy, ResultPath, सत्य,
                            FS_Dir);
}

std::error_code
getPotentiallyUniqueFileName(अचर Twine &Model,
                             SmallVectorImpl<वर्ण> &ResultPath) {
  पूर्णाङ्क Dummy;
  फर्काऊ createUniqueEntity(Model, Dummy, ResultPath, असत्य, FS_Name);
}

std::error_code
getPotentiallyUniqueTempFileName(अचर Twine &Prefix, StringRef Suffix,
                                 SmallVectorImpl<वर्ण> &ResultPath) {
  पूर्णाङ्क Dummy;
  फर्काऊ createTemporaryFile(Prefix, Suffix, Dummy, ResultPath, FS_Name);
}

std::error_code make_absolute(SmallVectorImpl<वर्ण> &path) {
  sandbox::violationIfEnabled();

  यदि (path::is_absolute(path))
    फर्काऊ {};

  SmallString<128> current_dir;
  यदि (std::error_code ec = current_path(current_dir))
    फर्काऊ ec;

  path::make_absolute(current_dir, path);
  फर्काऊ {};
}

std::error_code create_directories(अचर Twine &Path, तार्किक IgnoreExisting,
                                   perms Perms) {
  SmallString<128> PathStorage;
  StringRef P = Path.toStringRef(PathStorage);

  // Be optimistic and try to create the directory
  std::error_code EC = create_directory(P, IgnoreExisting, Perms);
  // If we succeeded, or had any error other than the parent not existing, just
  // return it.
  यदि (EC != errc::no_such_file_or_directory)
    फर्काऊ EC;

  // We failed because of a no_such_file_or_directory, try to create the
  // parent.
  StringRef Parent = path::parent_path(P);
  यदि (Parent.empty())
    फर्काऊ EC;

  यदि ((EC = create_directories(Parent, IgnoreExisting, Perms)))
      फर्काऊ EC;

  फर्काऊ create_directory(P, IgnoreExisting, Perms);
}

स्थिर std::error_code copy_file_internal(पूर्णाङ्क ReadFD, पूर्णाङ्क WriteFD) {
  अचर माप_प्रकार BufSize = 4096;
  वर्ण *Buf = नूतन वर्ण[BufSize];
  पूर्णाङ्क BytesRead = 0, BytesWritten = 0;
  लागि (;;) {
    BytesRead = पठ(ReadFD, Buf, BufSize);
    यदि (BytesRead <= 0)
      रोक;
    जबसम्म (BytesRead) {
      BytesWritten = लिख(WriteFD, Buf, BytesRead);
      यदि (BytesWritten < 0)
        रोक;
      BytesRead -= BytesWritten;
    }
    यदि (BytesWritten < 0)
      रोक;
  }
  delete[] Buf;

  यदि (BytesRead < 0 || BytesWritten < 0)
    फर्काऊ errnoAsErrorCode();
  फर्काऊ std::error_code();
}

#ifndef __APPLE__
std::error_code copy_file(अचर Twine &From, अचर Twine &To) {
  पूर्णाङ्क ReadFD, WriteFD;
  यदि (std::error_code EC = openFileForRead(From, ReadFD, OF_None))
    फर्काऊ EC;
  यदि (std::error_code EC =
          openFileForWrite(To, WriteFD, CD_CreateAlways, OF_None)) {
    बंद(ReadFD);
    फर्काऊ EC;
  }

  std::error_code EC = copy_file_internal(ReadFD, WriteFD);

  बंद(ReadFD);
  बंद(WriteFD);

  फर्काऊ EC;
}
#endif

std::error_code copy_file(अचर Twine &From, पूर्णाङ्क ToFD) {
  पूर्णाङ्क ReadFD;
  यदि (std::error_code EC = openFileForRead(From, ReadFD, OF_None))
    फर्काऊ EC;

  std::error_code EC = copy_file_internal(ReadFD, ToFD);

  बंद(ReadFD);

  फर्काऊ EC;
}

ErrorOr<MD5::MD5Result> md5_contents(पूर्णाङ्क FD) {
  sandbox::violationIfEnabled();

  MD5 Hash;

  constexpr माप_प्रकार BufSize = 4096;
  std::vector<uint8_t> Buf(BufSize);
  पूर्णाङ्क BytesRead = 0;
  लागि (;;) {
    BytesRead = पठ(FD, Buf.data(), BufSize);
    यदि (BytesRead <= 0)
      रोक;
    Hash.update(ArrayRef(Buf.data(), BytesRead));
  }

  यदि (BytesRead < 0)
    फर्काऊ errnoAsErrorCode();
  MD5::MD5Result Result;
  Hash.final(Result);
  फर्काऊ Result;
}

ErrorOr<MD5::MD5Result> md5_contents(अचर Twine &Path) {
  sandbox::violationIfEnabled();

  पूर्णाङ्क FD;
  यदि (स्वतः EC = openFileForRead(Path, FD, OF_None))
    फर्काऊ EC;

  स्वतः Result = md5_contents(FD);
  बंद(FD);
  फर्काऊ Result;
}

तार्किक exists(अचर basic_file_status &status) {
  फर्काऊ status_known(status) && status.type() != file_type::file_not_found;
}

तार्किक status_known(अचर basic_file_status &s) {
  फर्काऊ s.type() != file_type::status_error;
}

file_type get_file_type(अचर Twine &Path, तार्किक Follow) {
  file_status st;
  यदि (status(Path, st, Follow))
    फर्काऊ file_type::status_error;
  फर्काऊ st.type();
}

तार्किक is_directory(अचर basic_file_status &status) {
  फर्काऊ status.type() == file_type::directory_file;
}

std::error_code is_directory(अचर Twine &path, तार्किक &result) {
  sandbox::violationIfEnabled();

  file_status st;
  यदि (std::error_code ec = status(path, st))
    फर्काऊ ec;
  result = is_directory(st);
  फर्काऊ std::error_code();
}

तार्किक is_regular_file(अचर basic_file_status &status) {
  फर्काऊ status.type() == file_type::regular_file;
}

std::error_code is_regular_file(अचर Twine &path, तार्किक &result) {
  sandbox::violationIfEnabled();

  file_status st;
  यदि (std::error_code ec = status(path, st))
    फर्काऊ ec;
  result = is_regular_file(st);
  फर्काऊ std::error_code();
}

तार्किक is_symlink_file(अचर basic_file_status &status) {
  फर्काऊ status.type() == file_type::symlink_file;
}

std::error_code is_symlink_file(अचर Twine &path, तार्किक &result) {
  sandbox::violationIfEnabled();

  file_status st;
  यदि (std::error_code ec = status(path, st, असत्य))
    फर्काऊ ec;
  result = is_symlink_file(st);
  फर्काऊ std::error_code();
}

तार्किक is_other(अचर basic_file_status &status) {
  फर्काऊ exists(status) &&
         !is_regular_file(status) &&
         !is_directory(status);
}

std::error_code is_other(अचर Twine &Path, तार्किक &Result) {
  sandbox::violationIfEnabled();

  file_status FileStatus;
  यदि (std::error_code EC = status(Path, FileStatus))
    फर्काऊ EC;
  Result = is_other(FileStatus);
  फर्काऊ std::error_code();
}

शून्य directory_entry::replace_filename(अचर Twine &Filename, file_type Type,
                                       basic_file_status Status) {
  SmallString<128> PathStr = path::parent_path(Path);
  path::append(PathStr, Filename);
  एतत्->Path = std::string(PathStr);
  एतत्->Type = Type;
  एतत्->Status = Status;
}

ErrorOr<perms> getPermissions(अचर Twine &Path) {
  sandbox::violationIfEnabled();

  file_status Status;
  यदि (std::error_code EC = status(Path, Status))
    फर्काऊ EC;

  फर्काऊ Status.permissions();
}

std::error_code setLastAccessAndModificationTime(अचर Twine &Path,
                                                 TimePoint<> AccessTime,
                                                 TimePoint<> ModificationTime) {
  पूर्णाङ्क FD;
  यदि (std::error_code EC = openFile(Path, FD, CD_OpenExisting, FA_Read,
                                    OF_UpdateAttributes | OF_OpenDirectory))
    फर्काऊ EC;
  std::error_code EC =
      setLastAccessAndModificationTime(FD, AccessTime, ModificationTime);
  Process::SafelyCloseFileDescriptor(FD);
  फर्काऊ EC;
}

माप_प्रकार mapped_file_region::size() अचर {
  प्रतिज्ञा(Mapping && "Mapping failed but used anyway!");
  फर्काऊ Size;
}

वर्ण *mapped_file_region::data() अचर {
  प्रतिज्ञा(Mapping && "Mapping failed but used anyway!");
  फर्काऊ reinterpret_cast<वर्ण *>(Mapping);
}

अचर वर्ण *mapped_file_region::const_data() अचर {
  प्रतिज्ञा(Mapping && "Mapping failed but used anyway!");
  फर्काऊ reinterpret_cast<अचर वर्ण *>(Mapping);
}

Error readNativeFileToEOF(file_t FileHandle, SmallVectorImpl<वर्ण> &Buffer,
                          ssize_t ChunkSize) {
  sandbox::violationIfEnabled();

  // Install a handler to truncate the buffer to the correct size on exit.
  माप_प्रकार Size = Buffer.size();
  llvm::scope_exit TruncateOnExit([&]() { Buffer.truncate(Size); });

  // Read into Buffer until we hit EOF.
  लागि (;;) {
    Buffer.resize_for_overwrite(Size + ChunkSize);
    Expected<माप_प्रकार> ReadBytes = readNativeFile(
        FileHandle, MutableArrayRef(Buffer.begin() + Size, ChunkSize));
    यदि (!ReadBytes)
      फर्काऊ ReadBytes.takeError();
    यदि (*ReadBytes == 0)
      फर्काऊ Error::success();
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
TempFile &TempFile::चालक=(TempFile &&Other) {
  TmpName = std::move(Other.TmpName);
  FD = Other.FD;
  Other.Done = सत्य;
  Other.FD = -1;
#ifdef _WIN32
  RemoveOnClose = Other.RemoveOnClose;
  Other.RemoveOnClose = असत्य;
#endif
  फर्काऊ *एतत्;
}

TempFile::~TempFile() { प्रतिज्ञा(Done); }

Error TempFile::discard() {
  Done = सत्य;
  यदि (FD != -1 && बंद(FD) == -1) {
    std::error_code EC = errnoAsErrorCode();
    फर्काऊ errorCodeToError(EC);
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
    RemoveEC = fs::हटाओ(TmpName);
    sys::DontRemoveFileOnSignal(TmpName);
    यदि (!RemoveEC)
      TmpName = "";
  } अन्यथा {
    TmpName = "";
  }
  फर्काऊ errorCodeToError(RemoveEC);
}

Error TempFile::keep(अचर Twine &Name) {
  प्रतिज्ञा(!Done);
  Done = सत्य;
  // Always try to close and rename.
#ifdef _WIN32
  // If we can't cancel the delete don't rename.
  स्वतः H = reinterpret_cast<HANDLE>(_get_osfhandle(FD));
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
      हटाओ(TmpName);
  }
#अन्यथा
  std::error_code RenameEC = fs::नाम(TmpName, Name);
  यदि (RenameEC) {
    // If we can't rename, try to copy to work around cross-device link issues.
    RenameEC = sys::fs::copy_file(TmpName, Name);
    // If we can't rename or copy, discard the temporary file.
    यदि (RenameEC)
      हटाओ(TmpName);
  }
#endif
  sys::DontRemoveFileOnSignal(TmpName);

  यदि (!RenameEC)
    TmpName = "";

  यदि (बंद(FD) == -1)
    फर्काऊ errorCodeToError(errnoAsErrorCode());
  FD = -1;

  फर्काऊ errorCodeToError(RenameEC);
}

Error TempFile::keep() {
  प्रतिज्ञा(!Done);
  Done = सत्य;

#ifdef _WIN32
  स्वतः H = reinterpret_cast<HANDLE>(_get_osfhandle(FD));
  यदि (std::error_code EC = setDeleteDisposition(H, असत्य))
    फर्काऊ errorCodeToError(EC);
#endif
  sys::DontRemoveFileOnSignal(TmpName);

  TmpName = "";

  यदि (बंद(FD) == -1)
    फर्काऊ errorCodeToError(errnoAsErrorCode());
  FD = -1;

  फर्काऊ Error::success();
}

Expected<TempFile> TempFile::create(अचर Twine &Model, अचिन्हित Mode,
                                    OpenFlags ExtraFlags) {
  पूर्णाङ्क FD;
  SmallString<128> ResultPath;
  यदि (std::error_code EC =
          createUniqueFile(Model, FD, ResultPath, OF_Delete | ExtraFlags, Mode))
    फर्काऊ errorCodeToError(EC);

  TempFile Ret(ResultPath, FD);
#ifdef _WIN32
  स्वतः H = reinterpret_cast<HANDLE>(_get_osfhandle(FD));
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
    फर्काऊ errorCodeToError(EC);
  }
  फर्काऊ std::move(Ret);
}
} // namespace fs

} // namespace sys
} // namespace llvm
