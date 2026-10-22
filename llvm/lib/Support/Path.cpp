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

#അന്തര്ഭാവ "llvm/Support/Path.h"
#അന്തര്ഭാവ "llvm/ADT/ArrayRef.h"
#അന്തര്ഭാവ "llvm/ADT/ScopeExit.h"
#അന്തര്ഭാവ "llvm/ADT/StringExtras.h"
#അന്തര്ഭാവ "llvm/Config/config.h"
#അന്തര്ഭാവ "llvm/Config/llvm-config.h"
#അന്തര്ഭാവ "llvm/Support/Errc.h"
#അന്തര്ഭാവ "llvm/Support/ErrorHandling.h"
#അന്തര്ഭാവ "llvm/Support/FileSystem.h"
#അന്തര്ഭാവ "llvm/Support/IOSandbox.h"
#അന്തര്ഭാവ "llvm/Support/Process.h"
#അന്തര്ഭാവ "llvm/Support/Signals.h"
#അന്തര്ഭാവ <cctype>

#എങ്കിൽ !defined(_MSC_VER) && !defined(__MINGW32__)
#അന്തര്ഭാവ <unistd.h>
#അല്ലെങ്കിൽ
#അന്തര്ഭാവ <io.h>
#endif

using നാമാകാശ llvm;
using നാമാകാശ llvm::support::endian;

നാമാകാശ {
  using llvm::StringRef;
  using llvm::sys::path::is_separator;
  using llvm::sys::path::Style;

  വരിയിൽ തർക്കം prefer_forward_slash() {
    സ്ഥിതം തർക്കം prefer = []() {
      എങ്കിൽ (std::optional<std::string> Env =
              sys::Process::GetEnv("LLVM_WINDOWS_PREFER_FORWARD_SLASH"))
        തിരികെ *Env == "1";
      തിരികെ static_cast<തർക്കം>(LLVM_WINDOWS_PREFER_FORWARD_SLASH);
    }();
    തിരികെ prefer;
  }

  വരിയിൽ Style real_style(Style style) {
    എങ്കിൽ (style != Style::native)
      തിരികെ style;
    എങ്കിൽ (is_style_posix(style))
      തിരികെ Style::posix;
    തിരികെ prefer_forward_slash() ? Style::windows_slash
                                  : Style::windows_backslash;
  }

  വരിയിൽ സ്ഥിരം അക്ഷരം *separators(Style style) {
    എങ്കിൽ (is_style_windows(style))
      തിരികെ "\\/";
    തിരികെ "/";
  }

  വരിയിൽ അക്ഷരം preferred_separator(Style style) {
    എങ്കിൽ (real_style(style) == Style::windows)
      തിരികെ '\\';
    തിരികെ '/';
  }

  StringRef find_first_component(StringRef path, Style style) {
    // Look for this first component in the following order.
    // * empty (in this case we return an empty string)
    // * either C: or {//,\\}net.
    // * {/,\}
    // * {file,directory}name

    എങ്കിൽ (path.empty())
      തിരികെ path;

    എങ്കിൽ (is_style_windows(style)) {
      // C:
      എങ്കിൽ (path.size() >= 2 &&
          std::isalpha(static_cast<unsigned അക്ഷരം>(path[0])) && path[1] == ':')
        തിരികെ path.substr(0, 2);
    }

    // //net
    എങ്കിൽ ((path.size() > 2) && is_separator(path[0], style) &&
        path[0] == path[1] && !is_separator(path[2], style)) {
      // Find the next directory separator.
      size_t end = path.find_first_of(separators(style), 2);
      തിരികെ path.substr(0, end);
    }

    // {/,\}
    എങ്കിൽ (is_separator(path[0], style))
      തിരികെ path.substr(0, 1);

    // * {file,directory}name
    size_t end = path.find_first_of(separators(style));
    തിരികെ path.substr(0, end);
  }

  // Returns the first character of the filename in str. For paths ending in
  // '/', it returns the position of the '/'.
  size_t filename_pos(StringRef str, Style style) {
    എങ്കിൽ (str.size() > 0 && is_separator(str[str.size() - 1], style))
      തിരികെ str.size() - 1;

    size_t pos = str.find_last_of(separators(style), str.size() - 1);

    എങ്കിൽ (is_style_windows(style)) {
      എങ്കിൽ (pos == StringRef::npos)
        pos = str.find_last_of(':', str.size() - 1);
    }

    എങ്കിൽ (pos == StringRef::npos || (pos == 1 && is_separator(str[0], style)))
      തിരികെ 0;

    തിരികെ pos + 1;
  }

  // Returns the position of the root directory in str. If there is no root
  // directory in str, it returns StringRef::npos.
  size_t വേര്_dir_start(StringRef str, Style style) {
    // case "c:/"
    എങ്കിൽ (is_style_windows(style)) {
      എങ്കിൽ (str.size() > 2 && str[1] == ':' && is_separator(str[2], style))
        തിരികെ 2;
    }

    // case "//net"
    എങ്കിൽ (str.size() > 3 && is_separator(str[0], style) && str[0] == str[1] &&
        !is_separator(str[2], style)) {
      തിരികെ str.find_first_of(separators(style), 2);
    }

    // case "/"
    എങ്കിൽ (str.size() > 0 && is_separator(str[0], style))
      തിരികെ 0;

    തിരികെ StringRef::npos;
  }

  // Returns the position past the end of the "parent path" of path. The parent
  // path will not end in '/', unless the parent is the root directory. If the
  // path has no parent, 0 is returned.
  size_t parent_path_end(StringRef path, Style style) {
    size_t end_pos = filename_pos(path, style);

    തർക്കം filename_was_sep =
        path.size() > 0 && is_separator(path[end_pos], style);

    // Skip separators until we reach root dir (or the start of the string).
    size_t വേര്_dir_pos = വേര്_dir_start(path, style);
    ആകുമ്പോൾ (end_pos > 0 &&
           (വേര്_dir_pos == StringRef::npos || end_pos > വേര്_dir_pos) &&
           is_separator(path[end_pos - 1], style))
      --end_pos;

    എങ്കിൽ (end_pos == വേര്_dir_pos && !filename_was_sep) {
      // We've reached the root dir and the input path was *not* ending in a
      // sequence of slashes. Include the root dir in the parent path.
      തിരികെ വേര്_dir_pos + 1;
    }

    // Otherwise, just include before the last slash.
    തിരികെ end_pos;
  }
} // end unnamed namespace

എണ്ണൽ FSEntity {
  FS_Dir,
  FS_File,
  FS_Name
};

സ്ഥിതം std::error_code
createUniqueEntity(സ്ഥിരം Twine &Model, പൂർണ്ണസംഖ്യ &ResultFD,
                   SmallVectorImpl<അക്ഷരം> &ResultPath, തർക്കം MakeAbsolute,
                   FSEntity Type, sys::fs::OpenFlags Flags = sys::fs::OF_None,
                   unsigned Mode = 0) {

  // Limit the number of attempts we make, so that we don't infinite loop. E.g.
  // "permission denied" could be for a specific file (so we retry with a
  // different name) or for the whole directory (retry would always fail).
  // Checking which is racy, so we try a number of times, then give up.
  std::error_code EC;
  വേണ്ടി (പൂർണ്ണസംഖ്യ Retries = 128; Retries > 0; --Retries) {
    sys::fs::createUniquePath(Model, ResultPath, MakeAbsolute);
    // Try to open + create the file.
    തിരഞ്ഞെടുപ്പ് (Type) {
    അവസ്ഥ FS_File: {
      EC = sys::fs::openFileForReadWrite(Twine(ResultPath.begin()), ResultFD,
                                         sys::fs::CD_CreateNew, Flags, Mode);
      എങ്കിൽ (EC) {
        // errc::permission_denied happens on Windows when we try to open a file
        // that has been marked for deletion.
        എങ്കിൽ (EC == errc::file_exists || EC == errc::permission_denied)
          തുടരുക;
        തിരികെ EC;
      }

      തിരികെ std::error_code();
    }

    അവസ്ഥ FS_Name: {
      EC = sys::fs::access(ResultPath.begin(), sys::fs::AccessMode::Exist);
      എങ്കിൽ (EC == errc::no_such_file_or_directory)
        തിരികെ std::error_code();
      എങ്കിൽ (EC)
        തിരികെ EC;
      തുടരുക;
    }

    അവസ്ഥ FS_Dir: {
      EC = sys::fs::create_directory(ResultPath.begin(), അസത്യം);
      എങ്കിൽ (EC) {
        എങ്കിൽ (EC == errc::file_exists)
          തുടരുക;
        തിരികെ EC;
      }
      തിരികെ std::error_code();
    }
    }
    llvm_unreachable("Invalid Type");
  }
  തിരികെ EC;
}

നാമാകാശ llvm {
നാമാകാശ sys  {
നാമാകാശ path {

const_iterator begin(StringRef path, Style style) {
  const_iterator i;
  i.Path      = path;
  i.Component = find_first_component(path, style);
  i.Position  = 0;
  i.S = style;
  തിരികെ i;
}

const_iterator end(StringRef path) {
  const_iterator i;
  i.Path      = path;
  i.Position  = path.size();
  തിരികെ i;
}

const_iterator &const_iterator::operator++() {
  പ്രതിജ്ഞാ(Position < Path.size() && "Tried to increment past end!");

  // Increment Position to past the current component
  Position += Component.size();

  // Check for end.
  എങ്കിൽ (Position == Path.size()) {
    Component = StringRef();
    തിരികെ *ഇത്;
  }

  // Both POSIX and Windows treat paths that begin with exactly two separators
  // specially.
  തർക്കം was_net = Component.size() > 2 && is_separator(Component[0], S) &&
                 Component[1] == Component[0] && !is_separator(Component[2], S);

  // Handle separators.
  എങ്കിൽ (is_separator(Path[Position], S)) {
    // Root dir.
    എങ്കിൽ (was_net ||
        // c:/
        (is_style_windows(S) && Component.ends_with(":"))) {
      Component = Path.substr(Position, 1);
      തിരികെ *ഇത്;
    }

    // Skip extra separators.
    ആകുമ്പോൾ (Position != Path.size() && is_separator(Path[Position], S)) {
      ++Position;
    }

    // Treat trailing '/' as a '.', unless it is the root dir.
    എങ്കിൽ (Position == Path.size() && Component != "/") {
      --Position;
      Component = ".";
      തിരികെ *ഇത്;
    }
  }

  // Find next component.
  size_t end_pos = Path.find_first_of(separators(S), Position);
  Component = Path.slice(Position, end_pos);

  തിരികെ *ഇത്;
}

തർക്കം const_iterator::operator==(സ്ഥിരം const_iterator &RHS) സ്ഥിരം {
  തിരികെ Path.begin() == RHS.Path.begin() && Position == RHS.Position;
}

ptrdiff_t const_iterator::operator-(സ്ഥിരം const_iterator &RHS) സ്ഥിരം {
  തിരികെ Position - RHS.Position;
}

reverse_iterator rbegin(StringRef Path, Style style) {
  reverse_iterator I;
  I.Path = Path;
  I.Position = Path.size();
  I.S = style;
  ++I;
  തിരികെ I;
}

reverse_iterator rend(StringRef Path) {
  reverse_iterator I;
  I.Path = Path;
  I.Component = Path.substr(0, 0);
  I.Position = 0;
  തിരികെ I;
}

reverse_iterator &reverse_iterator::operator++() {
  size_t വേര്_dir_pos = വേര്_dir_start(Path, S);

  // Skip separators unless it's the root directory.
  size_t end_pos = Position;
  ആകുമ്പോൾ (end_pos > 0 && (end_pos - 1) != വേര്_dir_pos &&
         is_separator(Path[end_pos - 1], S))
    --end_pos;

  // Treat trailing '/' as a '.', unless it is the root dir.
  എങ്കിൽ (Position == Path.size() && !Path.empty() &&
      is_separator(Path.back(), S) &&
      (വേര്_dir_pos == StringRef::npos || end_pos - 1 > വേര്_dir_pos)) {
    --Position;
    Component = ".";
    തിരികെ *ഇത്;
  }

  // Find next separator.
  size_t start_pos = filename_pos(Path.substr(0, end_pos), S);
  Component = Path.slice(start_pos, end_pos);
  Position = start_pos;
  തിരികെ *ഇത്;
}

തർക്കം reverse_iterator::operator==(സ്ഥിരം reverse_iterator &RHS) സ്ഥിരം {
  തിരികെ Path.begin() == RHS.Path.begin() && Component == RHS.Component &&
         Position == RHS.Position;
}

ptrdiff_t reverse_iterator::operator-(സ്ഥിരം reverse_iterator &RHS) സ്ഥിരം {
  തിരികെ Position - RHS.Position;
}

StringRef വേര്_path(StringRef path, Style style) {
  const_iterator b = begin(path, style), pos = b, e = end(path);
  എങ്കിൽ (b != e) {
    തർക്കം has_net =
        b->size() > 2 && is_separator((*b)[0], style) && (*b)[1] == (*b)[0];
    തർക്കം has_drive = is_style_windows(style) && b->ends_with(":");

    എങ്കിൽ (has_net || has_drive) {
      എങ്കിൽ ((++pos != e) && is_separator((*pos)[0], style)) {
        // {C:/,//net/}, so get the first two components.
        തിരികെ path.substr(0, b->size() + pos->size());
      }
      // just {C:,//net}, return the first component.
      തിരികെ *b;
    }

    // POSIX style root directory.
    എങ്കിൽ (is_separator((*b)[0], style)) {
      തിരികെ *b;
    }
  }

  തിരികെ StringRef();
}

StringRef വേര്_name(StringRef path, Style style) {
  const_iterator b = begin(path, style), e = end(path);
  എങ്കിൽ (b != e) {
    തർക്കം has_net =
        b->size() > 2 && is_separator((*b)[0], style) && (*b)[1] == (*b)[0];
    തർക്കം has_drive = is_style_windows(style) && b->ends_with(":");

    എങ്കിൽ (has_net || has_drive) {
      // just {C:,//net}, return the first component.
      തിരികെ *b;
    }
  }

  // No path or no name.
  തിരികെ StringRef();
}

StringRef വേര്_directory(StringRef path, Style style) {
  const_iterator b = begin(path, style), pos = b, e = end(path);
  എങ്കിൽ (b != e) {
    തർക്കം has_net =
        b->size() > 2 && is_separator((*b)[0], style) && (*b)[1] == (*b)[0];
    തർക്കം has_drive = is_style_windows(style) && b->ends_with(":");

    എങ്കിൽ ((has_net || has_drive) &&
        // {C:,//net}, skip to the next component.
        (++pos != e) && is_separator((*pos)[0], style)) {
      തിരികെ *pos;
    }

    // POSIX style root directory.
    എങ്കിൽ (!has_net && is_separator((*b)[0], style)) {
      തിരികെ *b;
    }
  }

  // No path or no root.
  തിരികെ StringRef();
}

StringRef relative_path(StringRef path, Style style) {
  StringRef വേര്_3 = വേര്_path(path, style);
  തിരികെ path.substr(വേര്_3.size());
}

ശൂന്യം append(SmallVectorImpl<അക്ഷരം> &path, Style style, സ്ഥിരം Twine &a,
            സ്ഥിരം Twine &b, സ്ഥിരം Twine &c, സ്ഥിരം Twine &d) {
  SmallString<32> a_storage;
  SmallString<32> b_storage;
  SmallString<32> c_storage;
  SmallString<32> d_storage;

  SmallVector<StringRef, 4> components;
  എങ്കിൽ (!a.isTriviallyEmpty()) components.തള്ളുക_back(a.toStringRef(a_storage));
  എങ്കിൽ (!b.isTriviallyEmpty()) components.തള്ളുക_back(b.toStringRef(b_storage));
  എങ്കിൽ (!c.isTriviallyEmpty()) components.തള്ളുക_back(c.toStringRef(c_storage));
  എങ്കിൽ (!d.isTriviallyEmpty()) components.തള്ളുക_back(d.toStringRef(d_storage));

  വേണ്ടി (auto &component : components) {
    തർക്കം path_has_sep =
        !path.empty() && is_separator(path[path.size() - 1], style);
    എങ്കിൽ (path_has_sep) {
      // Strip separators from beginning of component.
      size_t loc = component.find_first_not_of(separators(style));
      StringRef c = component.substr(loc);

      // Append it.
      path.append(c.begin(), c.end());
      തുടരുക;
    }

    തർക്കം component_has_sep =
        !component.empty() && is_separator(component[0], style);
    എങ്കിൽ (!component_has_sep &&
        !(path.empty() || has_വേര്_name(component, style))) {
      // Add a separator.
      path.തള്ളുക_back(preferred_separator(style));
    }

    path.append(component.begin(), component.end());
  }
}

ശൂന്യം append(SmallVectorImpl<അക്ഷരം> &path, സ്ഥിരം Twine &a, സ്ഥിരം Twine &b,
            സ്ഥിരം Twine &c, സ്ഥിരം Twine &d) {
  append(path, Style::native, a, b, c, d);
}

ശൂന്യം append(SmallVectorImpl<അക്ഷരം> &path, const_iterator begin,
            const_iterator end, Style style) {
  വേണ്ടി (; begin != end; ++begin)
    path::append(path, style, *begin);
}

StringRef parent_path(StringRef path, Style style) {
  size_t end_pos = parent_path_end(path, style);
  എങ്കിൽ (end_pos == StringRef::npos)
    തിരികെ StringRef();
  തിരികെ path.substr(0, end_pos);
}

ശൂന്യം remove_filename(SmallVectorImpl<അക്ഷരം> &path, Style style) {
  size_t end_pos = parent_path_end(StringRef(path.begin(), path.size()), style);
  എങ്കിൽ (end_pos != StringRef::npos)
    path.truncate(end_pos);
}

ശൂന്യം replace_extension(SmallVectorImpl<അക്ഷരം> &path, സ്ഥിരം Twine &extension,
                       Style style) {
  StringRef p(path.begin(), path.size());
  SmallString<32> ext_storage;
  StringRef ext = extension.toStringRef(ext_storage);

  // Erase existing extension.
  size_t pos = p.find_last_of('.');
  എങ്കിൽ (pos != StringRef::npos && pos >= filename_pos(p, style))
    path.truncate(pos);

  // Append '.' if needed.
  എങ്കിൽ (ext.size() > 0 && ext[0] != '.')
    path.തള്ളുക_back('.');

  // Append extension.
  path.append(ext.begin(), ext.end());
}

സ്ഥിതം തർക്കം starts_with(StringRef Path, StringRef Prefix,
                        Style style = Style::native) {
  // Windows prefix matching : case and separator insensitive
  എങ്കിൽ (is_style_windows(style)) {
    എങ്കിൽ (Path.size() < Prefix.size())
      തിരികെ അസത്യം;
    വേണ്ടി (size_t I = 0, E = Prefix.size(); I != E; ++I) {
      തർക്കം SepPath = is_separator(Path[I], style);
      തർക്കം SepPrefix = is_separator(Prefix[I], style);
      എങ്കിൽ (SepPath != SepPrefix)
        തിരികെ അസത്യം;
      എങ്കിൽ (!SepPath && toLower(Path[I]) != toLower(Prefix[I]))
        തിരികെ അസത്യം;
    }
    തിരികെ സത്യം;
  }
  തിരികെ Path.starts_with(Prefix);
}

തർക്കം replace_path_prefix(SmallVectorImpl<അക്ഷരം> &Path, StringRef OldPrefix,
                         StringRef NewPrefix, Style style) {
  എങ്കിൽ (OldPrefix.empty() && NewPrefix.empty())
    തിരികെ അസത്യം;

  StringRef OrigPath(Path.begin(), Path.size());
  എങ്കിൽ (!starts_with(OrigPath, OldPrefix, style))
    തിരികെ അസത്യം;

  // If prefixes have the same size we can simply copy the new one over.
  എങ്കിൽ (OldPrefix.size() == NewPrefix.size()) {
    llvm::copy(NewPrefix, Path.begin());
    തിരികെ സത്യം;
  }

  StringRef RelPath = OrigPath.substr(OldPrefix.size());
  SmallString<256> NewPath;
  (Twine(NewPrefix) + RelPath).toVector(NewPath);
  Path.swap(NewPath);
  തിരികെ സത്യം;
}

ശൂന്യം native(സ്ഥിരം Twine &path, SmallVectorImpl<അക്ഷരം> &result, Style style) {
  പ്രതിജ്ഞാ((!path.isSingleStringRef() ||
          path.getSingleStringRef().data() != result.data()) &&
         "path and result are not allowed to overlap!");
  // Clear result.
  result.clear();
  path.toVector(result);
  native(result, style);
}

std::string native(സ്ഥിരം Twine &path, Style style) {
  SmallString<128> Result;
  native(path, Result, style);
  തിരികെ std::string(Result);
}

ശൂന്യം native(SmallVectorImpl<അക്ഷരം> &Path, Style style) {
  എങ്കിൽ (Path.empty())
    തിരികെ;
  എങ്കിൽ (is_style_windows(style)) {
    വേണ്ടി (അക്ഷരം &Ch : Path)
      എങ്കിൽ (is_separator(Ch, style))
        Ch = preferred_separator(style);
    എങ്കിൽ (Path[0] == '~' && (Path.size() == 1 || is_separator(Path[1], style))) {
      SmallString<128> PathHome;
      home_directory(PathHome);
      PathHome.append(Path.begin() + 1, Path.end());
      Path = std::move(PathHome);
    }
  } അല്ലെങ്കിൽ {
    llvm::replace(Path, '\\', '/');
  }
}

std::string convert_to_slash(StringRef path, Style style) {
  എങ്കിൽ (is_style_posix(style))
    തിരികെ std::string(path);

  std::string s = path.str();
  llvm::replace(s, '\\', '/');
  തിരികെ s;
}

StringRef filename(StringRef path, Style style) { തിരികെ *rbegin(path, style); }

StringRef stem(StringRef path, Style style) {
  StringRef fname = filename(path, style);
  size_t pos = fname.find_last_of('.');
  എങ്കിൽ (pos == StringRef::npos)
    തിരികെ fname;
  എങ്കിൽ ((fname.size() == 1 && fname == ".") ||
      (fname.size() == 2 && fname == ".."))
    തിരികെ fname;
  തിരികെ fname.substr(0, pos);
}

StringRef extension(StringRef path, Style style) {
  StringRef fname = filename(path, style);
  size_t pos = fname.find_last_of('.');
  എങ്കിൽ (pos == StringRef::npos)
    തിരികെ StringRef();
  എങ്കിൽ ((fname.size() == 1 && fname == ".") ||
      (fname.size() == 2 && fname == ".."))
    തിരികെ StringRef();
  തിരികെ fname.substr(pos);
}

തർക്കം is_separator(അക്ഷരം മൂല്യം_2, Style style) {
  എങ്കിൽ (മൂല്യം_2 == '/')
    തിരികെ സത്യം;
  എങ്കിൽ (is_style_windows(style))
    തിരികെ മൂല്യം_2 == '\\';
  തിരികെ അസത്യം;
}

StringRef get_separator(Style style) {
  എങ്കിൽ (real_style(style) == Style::windows)
    തിരികെ "\\";
  തിരികെ "/";
}

തർക്കം has_വേര്_name(സ്ഥിരം Twine &path, Style style) {
  SmallString<128> path_storage;
  StringRef p = path.toStringRef(path_storage);

  തിരികെ !വേര്_name(p, style).empty();
}

തർക്കം has_വേര്_directory(സ്ഥിരം Twine &path, Style style) {
  SmallString<128> path_storage;
  StringRef p = path.toStringRef(path_storage);

  തിരികെ !വേര്_directory(p, style).empty();
}

തർക്കം has_വേര്_path(സ്ഥിരം Twine &path, Style style) {
  SmallString<128> path_storage;
  StringRef p = path.toStringRef(path_storage);

  തിരികെ !വേര്_path(p, style).empty();
}

തർക്കം has_relative_path(സ്ഥിരം Twine &path, Style style) {
  SmallString<128> path_storage;
  StringRef p = path.toStringRef(path_storage);

  തിരികെ !relative_path(p, style).empty();
}

തർക്കം has_filename(സ്ഥിരം Twine &path, Style style) {
  SmallString<128> path_storage;
  StringRef p = path.toStringRef(path_storage);

  തിരികെ !filename(p, style).empty();
}

തർക്കം has_parent_path(സ്ഥിരം Twine &path, Style style) {
  SmallString<128> path_storage;
  StringRef p = path.toStringRef(path_storage);

  തിരികെ !parent_path(p, style).empty();
}

തർക്കം has_stem(സ്ഥിരം Twine &path, Style style) {
  SmallString<128> path_storage;
  StringRef p = path.toStringRef(path_storage);

  തിരികെ !stem(p, style).empty();
}

തർക്കം has_extension(സ്ഥിരം Twine &path, Style style) {
  SmallString<128> path_storage;
  StringRef p = path.toStringRef(path_storage);

  തിരികെ !extension(p, style).empty();
}

തർക്കം is_absolute(സ്ഥിരം Twine &path, Style style) {
  SmallString<128> path_storage;
  StringRef p = path.toStringRef(path_storage);

  തർക്കം വേര്_Dir = has_വേര്_directory(p, style);
  തർക്കം വേര്_Name = is_style_posix(style) || has_വേര്_name(p, style);

  തിരികെ വേര്_Dir && വേര്_Name;
}

തർക്കം is_absolute_gnu(സ്ഥിരം Twine &path, Style style) {
  SmallString<128> path_storage;
  StringRef p = path.toStringRef(path_storage);

  // Handle '/' which is absolute for both Windows and POSIX systems.
  // Handle '\\' on Windows.
  എങ്കിൽ (!p.empty() && is_separator(p.front(), style))
    തിരികെ സത്യം;

  എങ്കിൽ (is_style_windows(style)) {
    // Handle drive letter pattern (a character followed by ':') on Windows.
    എങ്കിൽ (p.size() >= 2 && (p[0] && p[1] == ':'))
      തിരികെ സത്യം;
  }

  തിരികെ അസത്യം;
}

തർക്കം is_relative(സ്ഥിരം Twine &path, Style style) {
  തിരികെ !is_absolute(path, style);
}

ശൂന്യം make_absolute(സ്ഥിരം Twine &current_directory,
                   SmallVectorImpl<അക്ഷരം> &path) {
  StringRef p(path.data(), path.size());

  തർക്കം വേര്_Directory = has_വേര്_directory(p);
  തർക്കം വേര്_Name = has_വേര്_name(p);

  // Already absolute.
  എങ്കിൽ ((വേര്_Name || is_style_posix(Style::native)) && വേര്_Directory)
    തിരികെ;

  // All the following conditions will need the current directory.
  SmallString<128> current_dir;
  current_directory.toVector(current_dir);

  // Relative path. Prepend the current directory.
  എങ്കിൽ (!വേര്_Name && !വേര്_Directory) {
    // Append path to the current directory.
    append(current_dir, p);
    // Set path to the result.
    path.swap(current_dir);
    തിരികെ;
  }

  എങ്കിൽ (!വേര്_Name && വേര്_Directory) {
    StringRef cdrn = വേര്_name(current_dir);
    SmallString<128> cur_Dir_വേര്_Name(cdrn.begin(), cdrn.end());
    append(cur_Dir_വേര്_Name, p);
    // Set path to the result.
    path.swap(cur_Dir_വേര്_Name);
    തിരികെ;
  }

  എങ്കിൽ (വേര്_Name && !വേര്_Directory) {
    StringRef p_വേര്_Name = വേര്_name(p);
    StringRef b_വേര്_Directory = വേര്_directory(current_dir);
    StringRef bRelativePath = relative_path(current_dir);
    StringRef pRelativePath = relative_path(p);

    SmallString<128> res;
    append(res, p_വേര്_Name, b_വേര്_Directory, bRelativePath, pRelativePath);
    path.swap(res);
    തിരികെ;
  }

  llvm_unreachable("All rootName and rootDirectory combinations should have "
                   "occurred above!");
}

StringRef remove_leading_dotslash(StringRef Path, Style style) {
  // Remove leading "./" (or ".//" or "././" etc.)
  ആകുമ്പോൾ (Path.size() > 2 && Path[0] == '.' && is_separator(Path[1], style)) {
    Path = Path.substr(2);
    ആകുമ്പോൾ (Path.size() > 0 && is_separator(Path[0], style))
      Path = Path.substr(1);
  }
  തിരികെ Path;
}

തർക്കം remove_dots(SmallVectorImpl<അക്ഷരം> &the_path, തർക്കം remove_dot_dot,
                 Style style) {
  style = real_style(style);
  StringRef remaining(the_path.data(), the_path.size());
  തർക്കം needs_change = അസത്യം;
  SmallVector<StringRef, 16> components;

  // Consume the root path, if present.
  StringRef വേര്_3 = path::വേര്_path(remaining, style);
  തർക്കം absolute = !വേര്_3.empty();
  എങ്കിൽ (absolute)
    remaining = remaining.drop_front(വേര്_3.size());

  // Loop over path components manually. This makes it easier to detect
  // non-preferred slashes and double separators that must be canonicalized.
  ആകുമ്പോൾ (!remaining.empty()) {
    size_t next_slash = remaining.find_first_of(separators(style));
    എങ്കിൽ (next_slash == StringRef::npos)
      next_slash = remaining.size();
    StringRef component = remaining.take_front(next_slash);
    remaining = remaining.drop_front(next_slash);

    // Eat the slash, and check if it is the preferred separator.
    എങ്കിൽ (!remaining.empty()) {
      needs_change |= remaining.front() != preferred_separator(style);
      remaining = remaining.drop_front();
      // The path needs to be rewritten if it has a trailing slash.
      // FIXME: This is emergent behavior that could be removed.
      needs_change |= remaining.empty();
    }

    // Check for path traversal components or double separators.
    എങ്കിൽ (component.empty() || component == ".") {
      needs_change = സത്യം;
    } അല്ലെങ്കിൽ എങ്കിൽ (remove_dot_dot && component == "..") {
      needs_change = സത്യം;
      // Do not allow ".." to remove the root component. If this is the
      // beginning of a relative path, keep the ".." component.
      എങ്കിൽ (!components.empty() && components.back() != "..") {
        components.എടുക്കുക_back();
      } അല്ലെങ്കിൽ എങ്കിൽ (!absolute) {
        components.തള്ളുക_back(component);
      }
    } അല്ലെങ്കിൽ {
      components.തള്ളുക_back(component);
    }
  }

  SmallString<256> buffer = വേര്_3;
  // "root" could be "/", which may need to be translated into "\".
  make_preferred(buffer, style);
  needs_change |= വേര്_3 != buffer;

  // Avoid rewriting the path unless we have to.
  എങ്കിൽ (!needs_change)
    തിരികെ അസത്യം;

  എങ്കിൽ (!components.empty()) {
    buffer += components[0];
    വേണ്ടി (StringRef C : ArrayRef(components).drop_front()) {
      buffer += preferred_separator(style);
      buffer += C;
    }
  }
  the_path.swap(buffer);
  തിരികെ സത്യം;
}

} // end namespace path

നാമാകാശ fs {

std::error_code getUniqueID(സ്ഥിരം Twine Path, UniqueID &Result) {
  sandbox::violationIfEnabled();

  file_status Status;
  std::error_code EC = status(Path, Status);
  എങ്കിൽ (EC)
    തിരികെ EC;
  Result = Status.getUniqueID();
  തിരികെ std::error_code();
}

ശൂന്യം createUniquePath(സ്ഥിരം Twine &Model, SmallVectorImpl<അക്ഷരം> &ResultPath,
                      തർക്കം MakeAbsolute) {
  SmallString<128> ModelStorage;
  Model.toVector(ModelStorage);

  പ്രതിജ്ഞാ(llvm::is_contained(ModelStorage, '%') &&
         "createUniquePath: Model must contain at least one '%'");

  എങ്കിൽ (MakeAbsolute) {
    // Make model absolute by prepending a temp directory if it's not already.
    എങ്കിൽ (!sys::path::is_absolute(Twine(ModelStorage))) {
      SmallString<128> TDir;
      sys::path::system_temp_directory(സത്യം, TDir);
      sys::path::append(TDir, Twine(ModelStorage));
      ModelStorage.swap(TDir);
    }
  }

  ResultPath = ModelStorage;
  ResultPath.തള്ളുക_back(0);
  ResultPath.എടുക്കുക_back();

  // Replace '%' with random chars.
  വേണ്ടി (unsigned i = 0, e = ModelStorage.size(); i != e; ++i) {
    എങ്കിൽ (ModelStorage[i] == '%')
      ResultPath[i] = "0123456789abcdef"[sys::Process::Get_Random_സംഖ്യ() & 15];
  }
}

std::error_code createUniqueFile(സ്ഥിരം Twine &Model, പൂർണ്ണസംഖ്യ &ResultFd,
                                 SmallVectorImpl<അക്ഷരം> &ResultPath,
                                 OpenFlags Flags, unsigned Mode) {
  തിരികെ createUniqueEntity(Model, ResultFd, ResultPath, അസത്യം, FS_File, Flags,
                            Mode);
}

std::error_code createUniqueFile(സ്ഥിരം Twine &Model,
                                 SmallVectorImpl<അക്ഷരം> &ResultPath,
                                 unsigned Mode) {
  പൂർണ്ണസംഖ്യ FD;
  auto EC = createUniqueFile(Model, FD, ResultPath, OF_None, Mode);
  എങ്കിൽ (EC)
    തിരികെ EC;
  // FD is only needed to avoid race conditions. Close it right away.
  close(FD);
  തിരികെ EC;
}

സ്ഥിതം std::error_code
createTemporaryFile(സ്ഥിരം Twine &Model, പൂർണ്ണസംഖ്യ &ResultFD,
                    llvm::SmallVectorImpl<അക്ഷരം> &ResultPath, FSEntity Type,
                    sys::fs::OpenFlags Flags = sys::fs::OF_None) {
  // Any *temporary* file is assumed to be a compiler-internal output, not
  // a formal one.
  auto BypassSandbox = sys::sandbox::scopedDisable();

  SmallString<128> Storage;
  StringRef P = Model.toNullTerminatedStringRef(Storage);
  പ്രതിജ്ഞാ(P.find_first_of(separators(Style::native)) == StringRef::npos &&
         "Model must be a simple filename.");
  // Use P.begin() so that createUniqueEntity doesn't need to recreate Storage.
  തിരികെ createUniqueEntity(P.begin(), ResultFD, ResultPath, സത്യം, Type, Flags,
                            all_read | all_write);
}

സ്ഥിതം std::error_code
createTemporaryFile(സ്ഥിരം Twine &Prefix, StringRef Suffix, പൂർണ്ണസംഖ്യ &ResultFD,
                    llvm::SmallVectorImpl<അക്ഷരം> &ResultPath, FSEntity Type,
                    sys::fs::OpenFlags Flags = sys::fs::OF_None) {
  സ്ഥിരം അക്ഷരം *നടുവ് = Suffix.empty() ? "-%%%%%%" : "-%%%%%%.";
  തിരികെ createTemporaryFile(Prefix + നടുവ് + Suffix, ResultFD, ResultPath,
                             Type, Flags);
}

std::error_code createTemporaryFile(സ്ഥിരം Twine &Prefix, StringRef Suffix,
                                    പൂർണ്ണസംഖ്യ &ResultFD,
                                    SmallVectorImpl<അക്ഷരം> &ResultPath,
                                    sys::fs::OpenFlags Flags) {
  തിരികെ createTemporaryFile(Prefix, Suffix, ResultFD, ResultPath, FS_File,
                             Flags);
}

std::error_code createTemporaryFile(സ്ഥിരം Twine &Prefix, StringRef Suffix,
                                    SmallVectorImpl<അക്ഷരം> &ResultPath,
                                    sys::fs::OpenFlags Flags) {
  പൂർണ്ണസംഖ്യ FD;
  auto EC = createTemporaryFile(Prefix, Suffix, FD, ResultPath, Flags);
  എങ്കിൽ (EC)
    തിരികെ EC;
  // FD is only needed to avoid race conditions. Close it right away.
  close(FD);
  തിരികെ EC;
}

// This is a mkdtemp with a different pattern. We use createUniqueEntity mostly
// for consistency. We should try using mkdtemp.
std::error_code createUniqueDirectory(സ്ഥിരം Twine &Prefix,
                                      SmallVectorImpl<അക്ഷരം> &ResultPath) {
  പൂർണ്ണസംഖ്യ Dummy;
  തിരികെ createUniqueEntity(Prefix + "-%%%%%%", Dummy, ResultPath, സത്യം,
                            FS_Dir);
}

std::error_code
getPotentiallyUniqueFileName(സ്ഥിരം Twine &Model,
                             SmallVectorImpl<അക്ഷരം> &ResultPath) {
  പൂർണ്ണസംഖ്യ Dummy;
  തിരികെ createUniqueEntity(Model, Dummy, ResultPath, അസത്യം, FS_Name);
}

std::error_code
getPotentiallyUniqueTempFileName(സ്ഥിരം Twine &Prefix, StringRef Suffix,
                                 SmallVectorImpl<അക്ഷരം> &ResultPath) {
  പൂർണ്ണസംഖ്യ Dummy;
  തിരികെ createTemporaryFile(Prefix, Suffix, Dummy, ResultPath, FS_Name);
}

std::error_code make_absolute(SmallVectorImpl<അക്ഷരം> &path) {
  sandbox::violationIfEnabled();

  എങ്കിൽ (path::is_absolute(path))
    തിരികെ {};

  SmallString<128> current_dir;
  എങ്കിൽ (std::error_code ec = current_path(current_dir))
    തിരികെ ec;

  path::make_absolute(current_dir, path);
  തിരികെ {};
}

std::error_code create_directories(സ്ഥിരം Twine &Path, തർക്കം IgnoreExisting,
                                   perms Perms) {
  SmallString<128> PathStorage;
  StringRef P = Path.toStringRef(PathStorage);

  // Be optimistic and try to create the directory
  std::error_code EC = create_directory(P, IgnoreExisting, Perms);
  // If we succeeded, or had any error other than the parent not existing, just
  // return it.
  എങ്കിൽ (EC != errc::no_such_file_or_directory)
    തിരികെ EC;

  // We failed because of a no_such_file_or_directory, try to create the
  // parent.
  StringRef Parent = path::parent_path(P);
  എങ്കിൽ (Parent.empty())
    തിരികെ EC;

  എങ്കിൽ ((EC = create_directories(Parent, IgnoreExisting, Perms)))
      തിരികെ EC;

  തിരികെ create_directory(P, IgnoreExisting, Perms);
}

സ്ഥിതം std::error_code copy_file_internal(പൂർണ്ണസംഖ്യ ReadFD, പൂർണ്ണസംഖ്യ WriteFD) {
  സ്ഥിരം size_t BufSize = 4096;
  അക്ഷരം *Buf = പുതിയ അക്ഷരം[BufSize];
  പൂർണ്ണസംഖ്യ BytesRead = 0, BytesWritten = 0;
  വേണ്ടി (;;) {
    BytesRead = പഠ(ReadFD, Buf, BufSize);
    എങ്കിൽ (BytesRead <= 0)
      നിർത്തുക;
    ആകുമ്പോൾ (BytesRead) {
      BytesWritten = ലിഖ(WriteFD, Buf, BytesRead);
      എങ്കിൽ (BytesWritten < 0)
        നിർത്തുക;
      BytesRead -= BytesWritten;
    }
    എങ്കിൽ (BytesWritten < 0)
      നിർത്തുക;
  }
  delete[] Buf;

  എങ്കിൽ (BytesRead < 0 || BytesWritten < 0)
    തിരികെ errnoAsErrorCode();
  തിരികെ std::error_code();
}

#ifndef __APPLE__
std::error_code copy_file(സ്ഥിരം Twine &From, സ്ഥിരം Twine &To) {
  പൂർണ്ണസംഖ്യ ReadFD, WriteFD;
  എങ്കിൽ (std::error_code EC = openFileForRead(From, ReadFD, OF_None))
    തിരികെ EC;
  എങ്കിൽ (std::error_code EC =
          openFileForWrite(To, WriteFD, CD_CreateAlways, OF_None)) {
    close(ReadFD);
    തിരികെ EC;
  }

  std::error_code EC = copy_file_internal(ReadFD, WriteFD);

  close(ReadFD);
  close(WriteFD);

  തിരികെ EC;
}
#endif

std::error_code copy_file(സ്ഥിരം Twine &From, പൂർണ്ണസംഖ്യ ToFD) {
  പൂർണ്ണസംഖ്യ ReadFD;
  എങ്കിൽ (std::error_code EC = openFileForRead(From, ReadFD, OF_None))
    തിരികെ EC;

  std::error_code EC = copy_file_internal(ReadFD, ToFD);

  close(ReadFD);

  തിരികെ EC;
}

ErrorOr<MD5::MD5Result> md5_contents(പൂർണ്ണസംഖ്യ FD) {
  sandbox::violationIfEnabled();

  MD5 Hash;

  constexpr size_t BufSize = 4096;
  std::vector<uint8_t> Buf(BufSize);
  പൂർണ്ണസംഖ്യ BytesRead = 0;
  വേണ്ടി (;;) {
    BytesRead = പഠ(FD, Buf.data(), BufSize);
    എങ്കിൽ (BytesRead <= 0)
      നിർത്തുക;
    Hash.update(ArrayRef(Buf.data(), BytesRead));
  }

  എങ്കിൽ (BytesRead < 0)
    തിരികെ errnoAsErrorCode();
  MD5::MD5Result Result;
  Hash.final(Result);
  തിരികെ Result;
}

ErrorOr<MD5::MD5Result> md5_contents(സ്ഥിരം Twine &Path) {
  sandbox::violationIfEnabled();

  പൂർണ്ണസംഖ്യ FD;
  എങ്കിൽ (auto EC = openFileForRead(Path, FD, OF_None))
    തിരികെ EC;

  auto Result = md5_contents(FD);
  close(FD);
  തിരികെ Result;
}

തർക്കം exists(സ്ഥിരം basic_file_status &status) {
  തിരികെ status_known(status) && status.type() != file_type::file_not_found;
}

തർക്കം status_known(സ്ഥിരം basic_file_status &s) {
  തിരികെ s.type() != file_type::status_error;
}

file_type get_file_type(സ്ഥിരം Twine &Path, തർക്കം Follow) {
  file_status st;
  എങ്കിൽ (status(Path, st, Follow))
    തിരികെ file_type::status_error;
  തിരികെ st.type();
}

തർക്കം is_directory(സ്ഥിരം basic_file_status &status) {
  തിരികെ status.type() == file_type::directory_file;
}

std::error_code is_directory(സ്ഥിരം Twine &path, തർക്കം &result) {
  sandbox::violationIfEnabled();

  file_status st;
  എങ്കിൽ (std::error_code ec = status(path, st))
    തിരികെ ec;
  result = is_directory(st);
  തിരികെ std::error_code();
}

തർക്കം is_regular_file(സ്ഥിരം basic_file_status &status) {
  തിരികെ status.type() == file_type::regular_file;
}

std::error_code is_regular_file(സ്ഥിരം Twine &path, തർക്കം &result) {
  sandbox::violationIfEnabled();

  file_status st;
  എങ്കിൽ (std::error_code ec = status(path, st))
    തിരികെ ec;
  result = is_regular_file(st);
  തിരികെ std::error_code();
}

തർക്കം is_symlink_file(സ്ഥിരം basic_file_status &status) {
  തിരികെ status.type() == file_type::symlink_file;
}

std::error_code is_symlink_file(സ്ഥിരം Twine &path, തർക്കം &result) {
  sandbox::violationIfEnabled();

  file_status st;
  എങ്കിൽ (std::error_code ec = status(path, st, അസത്യം))
    തിരികെ ec;
  result = is_symlink_file(st);
  തിരികെ std::error_code();
}

തർക്കം is_other(സ്ഥിരം basic_file_status &status) {
  തിരികെ exists(status) &&
         !is_regular_file(status) &&
         !is_directory(status);
}

std::error_code is_other(സ്ഥിരം Twine &Path, തർക്കം &Result) {
  sandbox::violationIfEnabled();

  file_status FileStatus;
  എങ്കിൽ (std::error_code EC = status(Path, FileStatus))
    തിരികെ EC;
  Result = is_other(FileStatus);
  തിരികെ std::error_code();
}

ശൂന്യം directory_entry::replace_filename(സ്ഥിരം Twine &Filename, file_type Type,
                                       basic_file_status Status) {
  SmallString<128> PathStr = path::parent_path(Path);
  path::append(PathStr, Filename);
  ഇത്->Path = std::string(PathStr);
  ഇത്->Type = Type;
  ഇത്->Status = Status;
}

ErrorOr<perms> getPermissions(സ്ഥിരം Twine &Path) {
  sandbox::violationIfEnabled();

  file_status Status;
  എങ്കിൽ (std::error_code EC = status(Path, Status))
    തിരികെ EC;

  തിരികെ Status.permissions();
}

std::error_code setLastAccessAndModificationTime(സ്ഥിരം Twine &Path,
                                                 TimePoint<> AccessTime,
                                                 TimePoint<> ModificationTime) {
  പൂർണ്ണസംഖ്യ FD;
  എങ്കിൽ (std::error_code EC = openFile(Path, FD, CD_OpenExisting, FA_Read,
                                    OF_UpdateAttributes | OF_OpenDirectory))
    തിരികെ EC;
  std::error_code EC =
      setLastAccessAndModificationTime(FD, AccessTime, ModificationTime);
  Process::SafelyCloseFileDescriptor(FD);
  തിരികെ EC;
}

size_t mapped_file_region::size() സ്ഥിരം {
  പ്രതിജ്ഞാ(Mapping && "Mapping failed but used anyway!");
  തിരികെ Size;
}

അക്ഷരം *mapped_file_region::data() സ്ഥിരം {
  പ്രതിജ്ഞാ(Mapping && "Mapping failed but used anyway!");
  തിരികെ reinterpret_cast<അക്ഷരം *>(Mapping);
}

സ്ഥിരം അക്ഷരം *mapped_file_region::const_data() സ്ഥിരം {
  പ്രതിജ്ഞാ(Mapping && "Mapping failed but used anyway!");
  തിരികെ reinterpret_cast<സ്ഥിരം അക്ഷരം *>(Mapping);
}

Error readNativeFileToEOF(file_t FileHandle, SmallVectorImpl<അക്ഷരം> &Buffer,
                          ssize_t ChunkSize) {
  sandbox::violationIfEnabled();

  // Install a handler to truncate the buffer to the correct size on exit.
  size_t Size = Buffer.size();
  llvm::scope_exit TruncateOnExit([&]() { Buffer.truncate(Size); });

  // Read into Buffer until we hit EOF.
  വേണ്ടി (;;) {
    Buffer.resize_for_overwrite(Size + ChunkSize);
    Expected<size_t> ReadBytes = readNativeFile(
        FileHandle, MutableArrayRef(Buffer.begin() + Size, ChunkSize));
    എങ്കിൽ (!ReadBytes)
      തിരികെ ReadBytes.takeError();
    എങ്കിൽ (*ReadBytes == 0)
      തിരികെ Error::success();
    Size += *ReadBytes;
  }
}

} // end namespace fs
} // end namespace sys
} // end namespace llvm

// Include the truly platform-specific parts.
#എങ്കിൽ defined(LLVM_ON_UNIX)
#അന്തര്ഭാവ "Unix/Path.inc"
#endif
#എങ്കിൽ defined(_WIN32)
#അന്തര്ഭാവ "Windows/Path.inc"
#endif

നാമാകാശ llvm {
നാമാകാശ sys {
നാമാകാശ fs {

TempFile::TempFile(StringRef Name, പൂർണ്ണസംഖ്യ FD)
    : TmpName(std::string(Name)), FD(FD) {}
TempFile::TempFile(TempFile &&Other) { *ഇത് = std::move(Other); }
TempFile &TempFile::operator=(TempFile &&Other) {
  TmpName = std::move(Other.TmpName);
  FD = Other.FD;
  Other.Done = സത്യം;
  Other.FD = -1;
#ifdef _WIN32
  RemoveOnClose = Other.RemoveOnClose;
  Other.RemoveOnClose = അസത്യം;
#endif
  തിരികെ *ഇത്;
}

TempFile::~TempFile() { പ്രതിജ്ഞാ(Done); }

Error TempFile::discard() {
  Done = സത്യം;
  എങ്കിൽ (FD != -1 && close(FD) == -1) {
    std::error_code EC = errnoAsErrorCode();
    തിരികെ errorCodeToError(EC);
  }
  FD = -1;

#ifdef _WIN32
  // On Windows, closing will remove the file, if we set the delete
  // disposition. If not, remove it manually.
  തർക്കം Remove = RemoveOnClose;
#അല്ലെങ്കിൽ
  // Always try to remove the file.
  തർക്കം Remove = സത്യം;
#endif
  std::error_code RemoveEC;
  എങ്കിൽ (Remove && !TmpName.empty()) {
    RemoveEC = fs::remove(TmpName);
    sys::DontRemoveFileOnSignal(TmpName);
    എങ്കിൽ (!RemoveEC)
      TmpName = "";
  } അല്ലെങ്കിൽ {
    TmpName = "";
  }
  തിരികെ errorCodeToError(RemoveEC);
}

Error TempFile::keep(സ്ഥിരം Twine &Name) {
  പ്രതിജ്ഞാ(!Done);
  Done = സത്യം;
  // Always try to close and rename.
#ifdef _WIN32
  // If we can't cancel the delete don't rename.
  auto H = reinterpret_cast<HANDLE>(_get_osfhandle(FD));
  std::error_code RenameEC =
      RemoveOnClose ? std::error_code() : setDeleteDisposition(H, അസത്യം);
  തർക്കം ShouldDelete = അസത്യം;
  എങ്കിൽ (!RenameEC) {
    RenameEC = rename_handle(H, Name);
    // If rename failed because it's cross-device, copy instead
    എങ്കിൽ (RenameEC ==
      std::error_code(ERROR_NOT_SAME_DEVICE, std::system_category())) {
      RenameEC = copy_file(TmpName, Name);
      ShouldDelete = സത്യം;
    }
  }

  // If we can't rename or copy, discard the temporary file.
  എങ്കിൽ (RenameEC)
    ShouldDelete = സത്യം;
  എങ്കിൽ (ShouldDelete) {
    എങ്കിൽ (!RemoveOnClose)
      setDeleteDisposition(H, സത്യം);
    അല്ലെങ്കിൽ
      remove(TmpName);
  }
#അല്ലെങ്കിൽ
  std::error_code RenameEC = fs::rename(TmpName, Name);
  എങ്കിൽ (RenameEC) {
    // If we can't rename, try to copy to work around cross-device link issues.
    RenameEC = sys::fs::copy_file(TmpName, Name);
    // If we can't rename or copy, discard the temporary file.
    എങ്കിൽ (RenameEC)
      remove(TmpName);
  }
#endif
  sys::DontRemoveFileOnSignal(TmpName);

  എങ്കിൽ (!RenameEC)
    TmpName = "";

  എങ്കിൽ (close(FD) == -1)
    തിരികെ errorCodeToError(errnoAsErrorCode());
  FD = -1;

  തിരികെ errorCodeToError(RenameEC);
}

Error TempFile::keep() {
  പ്രതിജ്ഞാ(!Done);
  Done = സത്യം;

#ifdef _WIN32
  auto H = reinterpret_cast<HANDLE>(_get_osfhandle(FD));
  എങ്കിൽ (std::error_code EC = setDeleteDisposition(H, അസത്യം))
    തിരികെ errorCodeToError(EC);
#endif
  sys::DontRemoveFileOnSignal(TmpName);

  TmpName = "";

  എങ്കിൽ (close(FD) == -1)
    തിരികെ errorCodeToError(errnoAsErrorCode());
  FD = -1;

  തിരികെ Error::success();
}

Expected<TempFile> TempFile::create(സ്ഥിരം Twine &Model, unsigned Mode,
                                    OpenFlags ExtraFlags) {
  പൂർണ്ണസംഖ്യ FD;
  SmallString<128> ResultPath;
  എങ്കിൽ (std::error_code EC =
          createUniqueFile(Model, FD, ResultPath, OF_Delete | ExtraFlags, Mode))
    തിരികെ errorCodeToError(EC);

  TempFile Ret(ResultPath, FD);
#ifdef _WIN32
  auto H = reinterpret_cast<HANDLE>(_get_osfhandle(FD));
  തർക്കം SetSignalHandler = അസത്യം;
  എങ്കിൽ (std::error_code EC = setDeleteDisposition(H, സത്യം)) {
    Ret.RemoveOnClose = സത്യം;
    SetSignalHandler = സത്യം;
  }
#അല്ലെങ്കിൽ
  തർക്കം SetSignalHandler = സത്യം;
#endif
  എങ്കിൽ (SetSignalHandler && sys::RemoveFileOnSignal(ResultPath)) {
    // Make sure we delete the file when RemoveFileOnSignal fails.
    consumeError(Ret.discard());
    std::error_code EC(errc::operation_not_permitted);
    തിരികെ errorCodeToError(EC);
  }
  തിരികെ std::move(Ret);
}
} // namespace fs

} // namespace sys
} // namespace llvm
