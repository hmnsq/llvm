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

#அந்தர்பாவ "llvm/Support/Path.h"
#அந்தர்பாவ "llvm/ADT/ArrayRef.h"
#அந்தர்பாவ "llvm/ADT/ScopeExit.h"
#அந்தர்பாவ "llvm/ADT/StringExtras.h"
#அந்தர்பாவ "llvm/Config/config.h"
#அந்தர்பாவ "llvm/Config/llvm-config.h"
#அந்தர்பாவ "llvm/Support/Errc.h"
#அந்தர்பாவ "llvm/Support/ErrorHandling.h"
#அந்தர்பாவ "llvm/Support/FileSystem.h"
#அந்தர்பாவ "llvm/Support/IOSandbox.h"
#அந்தர்பாவ "llvm/Support/Process.h"
#அந்தர்பாவ "llvm/Support/Signals.h"
#அந்தர்பாவ <cctype>

#எனில் !defined(_MSC_VER) && !defined(__MINGW32__)
#அந்தர்பாவ <unistd.h>
#இல்லையெனில்
#அந்தர்பாவ <io.h>
#endif

using நாமாகாஶ llvm;
using நாமாகாஶ llvm::support::endian;

நாமாகாஶ {
  using llvm::StringRef;
  using llvm::sys::path::is_separator;
  using llvm::sys::path::Style;

  உள்வரி தருக்க prefer_forward_slash() {
    நிலையான தருக்க prefer = []() {
      எனில் (std::optional<std::string> Env =
              sys::Process::GetEnv("LLVM_WINDOWS_PREFER_FORWARD_SLASH"))
        திரும்பு *Env == "1";
      திரும்பு static_cast<தருக்க>(LLVM_WINDOWS_PREFER_FORWARD_SLASH);
    }();
    திரும்பு prefer;
  }

  உள்வரி Style real_style(Style style) {
    எனில் (style != Style::native)
      திரும்பு style;
    எனில் (is_style_posix(style))
      திரும்பு Style::posix;
    திரும்பு prefer_forward_slash() ? Style::windows_slash
                                  : Style::windows_backslash;
  }

  உள்வரி மாறிலி எழுத்து *separators(Style style) {
    எனில் (is_style_windows(style))
      திரும்பு "\\/";
    திரும்பு "/";
  }

  உள்வரி எழுத்து preferred_separator(Style style) {
    எனில் (real_style(style) == Style::windows)
      திரும்பு '\\';
    திரும்பு '/';
  }

  StringRef find_first_component(StringRef path, Style style) {
    // Look for this first component in the following order.
    // * empty (in this case we return an empty string)
    // * either C: or {//,\\}net.
    // * {/,\}
    // * {file,directory}name

    எனில் (path.empty())
      திரும்பு path;

    எனில் (is_style_windows(style)) {
      // C:
      எனில் (path.size() >= 2 &&
          std::isalpha(static_cast<unsigned எழுத்து>(path[0])) && path[1] == ':')
        திரும்பு path.substr(0, 2);
    }

    // //net
    எனில் ((path.size() > 2) && is_separator(path[0], style) &&
        path[0] == path[1] && !is_separator(path[2], style)) {
      // Find the next directory separator.
      size_t end = path.find_first_of(separators(style), 2);
      திரும்பு path.substr(0, end);
    }

    // {/,\}
    எனில் (is_separator(path[0], style))
      திரும்பு path.substr(0, 1);

    // * {file,directory}name
    size_t end = path.find_first_of(separators(style));
    திரும்பு path.substr(0, end);
  }

  // Returns the first character of the filename in str. For paths ending in
  // '/', it returns the position of the '/'.
  size_t filename_pos(StringRef str, Style style) {
    எனில் (str.size() > 0 && is_separator(str[str.size() - 1], style))
      திரும்பு str.size() - 1;

    size_t pos = str.find_last_of(separators(style), str.size() - 1);

    எனில் (is_style_windows(style)) {
      எனில் (pos == StringRef::npos)
        pos = str.find_last_of(':', str.size() - 1);
    }

    எனில் (pos == StringRef::npos || (pos == 1 && is_separator(str[0], style)))
      திரும்பு 0;

    திரும்பு pos + 1;
  }

  // Returns the position of the root directory in str. If there is no root
  // directory in str, it returns StringRef::npos.
  size_t வேர்_dir_start(StringRef str, Style style) {
    // case "c:/"
    எனில் (is_style_windows(style)) {
      எனில் (str.size() > 2 && str[1] == ':' && is_separator(str[2], style))
        திரும்பு 2;
    }

    // case "//net"
    எனில் (str.size() > 3 && is_separator(str[0], style) && str[0] == str[1] &&
        !is_separator(str[2], style)) {
      திரும்பு str.find_first_of(separators(style), 2);
    }

    // case "/"
    எனில் (str.size() > 0 && is_separator(str[0], style))
      திரும்பு 0;

    திரும்பு StringRef::npos;
  }

  // Returns the position past the end of the "parent path" of path. The parent
  // path will not end in '/', unless the parent is the root directory. If the
  // path has no parent, 0 is returned.
  size_t parent_path_end(StringRef path, Style style) {
    size_t end_pos = filename_pos(path, style);

    தருக்க filename_was_sep =
        path.size() > 0 && is_separator(path[end_pos], style);

    // Skip separators until we reach root dir (or the start of the string).
    size_t வேர்_dir_pos = வேர்_dir_start(path, style);
    போது (end_pos > 0 &&
           (வேர்_dir_pos == StringRef::npos || end_pos > வேர்_dir_pos) &&
           is_separator(path[end_pos - 1], style))
      --end_pos;

    எனில் (end_pos == வேர்_dir_pos && !filename_was_sep) {
      // We've reached the root dir and the input path was *not* ending in a
      // sequence of slashes. Include the root dir in the parent path.
      திரும்பு வேர்_dir_pos + 1;
    }

    // Otherwise, just include before the last slash.
    திரும்பு end_pos;
  }
} // end unnamed namespace

எண்ணிக்கை FSEntity {
  FS_Dir,
  FS_File,
  FS_Name
};

நிலையான std::error_code
createUniqueEntity(மாறிலி Twine &Model, முழு &ResultFD,
                   SmallVectorImpl<எழுத்து> &ResultPath, தருக்க MakeAbsolute,
                   FSEntity Type, sys::fs::OpenFlags Flags = sys::fs::OF_None,
                   unsigned Mode = 0) {

  // Limit the number of attempts we make, so that we don't infinite loop. E.g.
  // "permission denied" could be for a specific file (so we retry with a
  // different name) or for the whole directory (retry would always fail).
  // Checking which is racy, so we try a number of times, then give up.
  std::error_code EC;
  க்கு (முழு Retries = 128; Retries > 0; --Retries) {
    sys::fs::createUniquePath(Model, ResultPath, MakeAbsolute);
    // Try to open + create the file.
    தேர்வு (Type) {
    நிலை FS_File: {
      EC = sys::fs::openFileForReadWrite(Twine(ResultPath.begin()), ResultFD,
                                         sys::fs::CD_CreateNew, Flags, Mode);
      எனில் (EC) {
        // errc::permission_denied happens on Windows when we try to open a file
        // that has been marked for deletion.
        எனில் (EC == errc::file_exists || EC == errc::permission_denied)
          தொடர்;
        திரும்பு EC;
      }

      திரும்பு std::error_code();
    }

    நிலை FS_Name: {
      EC = sys::fs::access(ResultPath.begin(), sys::fs::AccessMode::Exist);
      எனில் (EC == errc::no_such_file_or_directory)
        திரும்பு std::error_code();
      எனில் (EC)
        திரும்பு EC;
      தொடர்;
    }

    நிலை FS_Dir: {
      EC = sys::fs::create_directory(ResultPath.begin(), பொய்);
      எனில் (EC) {
        எனில் (EC == errc::file_exists)
          தொடர்;
        திரும்பு EC;
      }
      திரும்பு std::error_code();
    }
    }
    llvm_unreachable("Invalid Type");
  }
  திரும்பு EC;
}

நாமாகாஶ llvm {
நாமாகாஶ sys  {
நாமாகாஶ path {

const_iterator begin(StringRef path, Style style) {
  const_iterator i;
  i.Path      = path;
  i.Component = find_first_component(path, style);
  i.Position  = 0;
  i.S = style;
  திரும்பு i;
}

const_iterator end(StringRef path) {
  const_iterator i;
  i.Path      = path;
  i.Position  = path.size();
  திரும்பு i;
}

const_iterator &const_iterator::operator++() {
  ப்ரதிஜ்ஞா(Position < Path.size() && "Tried to increment past end!");

  // Increment Position to past the current component
  Position += Component.size();

  // Check for end.
  எனில் (Position == Path.size()) {
    Component = StringRef();
    திரும்பு *இது;
  }

  // Both POSIX and Windows treat paths that begin with exactly two separators
  // specially.
  தருக்க was_net = Component.size() > 2 && is_separator(Component[0], S) &&
                 Component[1] == Component[0] && !is_separator(Component[2], S);

  // Handle separators.
  எனில் (is_separator(Path[Position], S)) {
    // Root dir.
    எனில் (was_net ||
        // c:/
        (is_style_windows(S) && Component.ends_with(":"))) {
      Component = Path.substr(Position, 1);
      திரும்பு *இது;
    }

    // Skip extra separators.
    போது (Position != Path.size() && is_separator(Path[Position], S)) {
      ++Position;
    }

    // Treat trailing '/' as a '.', unless it is the root dir.
    எனில் (Position == Path.size() && Component != "/") {
      --Position;
      Component = ".";
      திரும்பு *இது;
    }
  }

  // Find next component.
  size_t end_pos = Path.find_first_of(separators(S), Position);
  Component = Path.slice(Position, end_pos);

  திரும்பு *இது;
}

தருக்க const_iterator::operator==(மாறிலி const_iterator &RHS) மாறிலி {
  திரும்பு Path.begin() == RHS.Path.begin() && Position == RHS.Position;
}

ptrdiff_t const_iterator::operator-(மாறிலி const_iterator &RHS) மாறிலி {
  திரும்பு Position - RHS.Position;
}

reverse_iterator rbegin(StringRef Path, Style style) {
  reverse_iterator I;
  I.Path = Path;
  I.Position = Path.size();
  I.S = style;
  ++I;
  திரும்பு I;
}

reverse_iterator rend(StringRef Path) {
  reverse_iterator I;
  I.Path = Path;
  I.Component = Path.substr(0, 0);
  I.Position = 0;
  திரும்பு I;
}

reverse_iterator &reverse_iterator::operator++() {
  size_t வேர்_dir_pos = வேர்_dir_start(Path, S);

  // Skip separators unless it's the root directory.
  size_t end_pos = Position;
  போது (end_pos > 0 && (end_pos - 1) != வேர்_dir_pos &&
         is_separator(Path[end_pos - 1], S))
    --end_pos;

  // Treat trailing '/' as a '.', unless it is the root dir.
  எனில் (Position == Path.size() && !Path.empty() &&
      is_separator(Path.back(), S) &&
      (வேர்_dir_pos == StringRef::npos || end_pos - 1 > வேர்_dir_pos)) {
    --Position;
    Component = ".";
    திரும்பு *இது;
  }

  // Find next separator.
  size_t start_pos = filename_pos(Path.substr(0, end_pos), S);
  Component = Path.slice(start_pos, end_pos);
  Position = start_pos;
  திரும்பு *இது;
}

தருக்க reverse_iterator::operator==(மாறிலி reverse_iterator &RHS) மாறிலி {
  திரும்பு Path.begin() == RHS.Path.begin() && Component == RHS.Component &&
         Position == RHS.Position;
}

ptrdiff_t reverse_iterator::operator-(மாறிலி reverse_iterator &RHS) மாறிலி {
  திரும்பு Position - RHS.Position;
}

StringRef வேர்_path(StringRef path, Style style) {
  const_iterator b = begin(path, style), pos = b, e = end(path);
  எனில் (b != e) {
    தருக்க has_net =
        b->size() > 2 && is_separator((*b)[0], style) && (*b)[1] == (*b)[0];
    தருக்க has_drive = is_style_windows(style) && b->ends_with(":");

    எனில் (has_net || has_drive) {
      எனில் ((++pos != e) && is_separator((*pos)[0], style)) {
        // {C:/,//net/}, so get the first two components.
        திரும்பு path.substr(0, b->size() + pos->size());
      }
      // just {C:,//net}, return the first component.
      திரும்பு *b;
    }

    // POSIX style root directory.
    எனில் (is_separator((*b)[0], style)) {
      திரும்பு *b;
    }
  }

  திரும்பு StringRef();
}

StringRef வேர்_name(StringRef path, Style style) {
  const_iterator b = begin(path, style), e = end(path);
  எனில் (b != e) {
    தருக்க has_net =
        b->size() > 2 && is_separator((*b)[0], style) && (*b)[1] == (*b)[0];
    தருக்க has_drive = is_style_windows(style) && b->ends_with(":");

    எனில் (has_net || has_drive) {
      // just {C:,//net}, return the first component.
      திரும்பு *b;
    }
  }

  // No path or no name.
  திரும்பு StringRef();
}

StringRef வேர்_directory(StringRef path, Style style) {
  const_iterator b = begin(path, style), pos = b, e = end(path);
  எனில் (b != e) {
    தருக்க has_net =
        b->size() > 2 && is_separator((*b)[0], style) && (*b)[1] == (*b)[0];
    தருக்க has_drive = is_style_windows(style) && b->ends_with(":");

    எனில் ((has_net || has_drive) &&
        // {C:,//net}, skip to the next component.
        (++pos != e) && is_separator((*pos)[0], style)) {
      திரும்பு *pos;
    }

    // POSIX style root directory.
    எனில் (!has_net && is_separator((*b)[0], style)) {
      திரும்பு *b;
    }
  }

  // No path or no root.
  திரும்பு StringRef();
}

StringRef relative_path(StringRef path, Style style) {
  StringRef வேர்_3 = வேர்_path(path, style);
  திரும்பு path.substr(வேர்_3.size());
}

வெறுமை append(SmallVectorImpl<எழுத்து> &path, Style style, மாறிலி Twine &a,
            மாறிலி Twine &b, மாறிலி Twine &c, மாறிலி Twine &d) {
  SmallString<32> a_storage;
  SmallString<32> b_storage;
  SmallString<32> c_storage;
  SmallString<32> d_storage;

  SmallVector<StringRef, 4> components;
  எனில் (!a.isTriviallyEmpty()) components.தள்ளு_back(a.toStringRef(a_storage));
  எனில் (!b.isTriviallyEmpty()) components.தள்ளு_back(b.toStringRef(b_storage));
  எனில் (!c.isTriviallyEmpty()) components.தள்ளு_back(c.toStringRef(c_storage));
  எனில் (!d.isTriviallyEmpty()) components.தள்ளு_back(d.toStringRef(d_storage));

  க்கு (auto &component : components) {
    தருக்க path_has_sep =
        !path.empty() && is_separator(path[path.size() - 1], style);
    எனில் (path_has_sep) {
      // Strip separators from beginning of component.
      size_t loc = component.find_first_not_of(separators(style));
      StringRef c = component.substr(loc);

      // Append it.
      path.append(c.begin(), c.end());
      தொடர்;
    }

    தருக்க component_has_sep =
        !component.empty() && is_separator(component[0], style);
    எனில் (!component_has_sep &&
        !(path.empty() || has_வேர்_name(component, style))) {
      // Add a separator.
      path.தள்ளு_back(preferred_separator(style));
    }

    path.append(component.begin(), component.end());
  }
}

வெறுமை append(SmallVectorImpl<எழுத்து> &path, மாறிலி Twine &a, மாறிலி Twine &b,
            மாறிலி Twine &c, மாறிலி Twine &d) {
  append(path, Style::native, a, b, c, d);
}

வெறுமை append(SmallVectorImpl<எழுத்து> &path, const_iterator begin,
            const_iterator end, Style style) {
  க்கு (; begin != end; ++begin)
    path::append(path, style, *begin);
}

StringRef parent_path(StringRef path, Style style) {
  size_t end_pos = parent_path_end(path, style);
  எனில் (end_pos == StringRef::npos)
    திரும்பு StringRef();
  திரும்பு path.substr(0, end_pos);
}

வெறுமை remove_filename(SmallVectorImpl<எழுத்து> &path, Style style) {
  size_t end_pos = parent_path_end(StringRef(path.begin(), path.size()), style);
  எனில் (end_pos != StringRef::npos)
    path.truncate(end_pos);
}

வெறுமை replace_extension(SmallVectorImpl<எழுத்து> &path, மாறிலி Twine &extension,
                       Style style) {
  StringRef p(path.begin(), path.size());
  SmallString<32> ext_storage;
  StringRef ext = extension.toStringRef(ext_storage);

  // Erase existing extension.
  size_t pos = p.find_last_of('.');
  எனில் (pos != StringRef::npos && pos >= filename_pos(p, style))
    path.truncate(pos);

  // Append '.' if needed.
  எனில் (ext.size() > 0 && ext[0] != '.')
    path.தள்ளு_back('.');

  // Append extension.
  path.append(ext.begin(), ext.end());
}

நிலையான தருக்க starts_with(StringRef Path, StringRef Prefix,
                        Style style = Style::native) {
  // Windows prefix matching : case and separator insensitive
  எனில் (is_style_windows(style)) {
    எனில் (Path.size() < Prefix.size())
      திரும்பு பொய்;
    க்கு (size_t I = 0, E = Prefix.size(); I != E; ++I) {
      தருக்க SepPath = is_separator(Path[I], style);
      தருக்க SepPrefix = is_separator(Prefix[I], style);
      எனில் (SepPath != SepPrefix)
        திரும்பு பொய்;
      எனில் (!SepPath && toLower(Path[I]) != toLower(Prefix[I]))
        திரும்பு பொய்;
    }
    திரும்பு உண்மை;
  }
  திரும்பு Path.starts_with(Prefix);
}

தருக்க replace_path_prefix(SmallVectorImpl<எழுத்து> &Path, StringRef OldPrefix,
                         StringRef NewPrefix, Style style) {
  எனில் (OldPrefix.empty() && NewPrefix.empty())
    திரும்பு பொய்;

  StringRef OrigPath(Path.begin(), Path.size());
  எனில் (!starts_with(OrigPath, OldPrefix, style))
    திரும்பு பொய்;

  // If prefixes have the same size we can simply copy the new one over.
  எனில் (OldPrefix.size() == NewPrefix.size()) {
    llvm::copy(NewPrefix, Path.begin());
    திரும்பு உண்மை;
  }

  StringRef RelPath = OrigPath.substr(OldPrefix.size());
  SmallString<256> NewPath;
  (Twine(NewPrefix) + RelPath).toVector(NewPath);
  Path.swap(NewPath);
  திரும்பு உண்மை;
}

வெறுமை native(மாறிலி Twine &path, SmallVectorImpl<எழுத்து> &result, Style style) {
  ப்ரதிஜ்ஞா((!path.isSingleStringRef() ||
          path.getSingleStringRef().data() != result.data()) &&
         "path and result are not allowed to overlap!");
  // Clear result.
  result.clear();
  path.toVector(result);
  native(result, style);
}

std::string native(மாறிலி Twine &path, Style style) {
  SmallString<128> Result;
  native(path, Result, style);
  திரும்பு std::string(Result);
}

வெறுமை native(SmallVectorImpl<எழுத்து> &Path, Style style) {
  எனில் (Path.empty())
    திரும்பு;
  எனில் (is_style_windows(style)) {
    க்கு (எழுத்து &Ch : Path)
      எனில் (is_separator(Ch, style))
        Ch = preferred_separator(style);
    எனில் (Path[0] == '~' && (Path.size() == 1 || is_separator(Path[1], style))) {
      SmallString<128> PathHome;
      home_directory(PathHome);
      PathHome.append(Path.begin() + 1, Path.end());
      Path = std::move(PathHome);
    }
  } இல்லையெனில் {
    llvm::replace(Path, '\\', '/');
  }
}

std::string convert_to_slash(StringRef path, Style style) {
  எனில் (is_style_posix(style))
    திரும்பு std::string(path);

  std::string s = path.str();
  llvm::replace(s, '\\', '/');
  திரும்பு s;
}

StringRef filename(StringRef path, Style style) { திரும்பு *rbegin(path, style); }

StringRef stem(StringRef path, Style style) {
  StringRef fname = filename(path, style);
  size_t pos = fname.find_last_of('.');
  எனில் (pos == StringRef::npos)
    திரும்பு fname;
  எனில் ((fname.size() == 1 && fname == ".") ||
      (fname.size() == 2 && fname == ".."))
    திரும்பு fname;
  திரும்பு fname.substr(0, pos);
}

StringRef extension(StringRef path, Style style) {
  StringRef fname = filename(path, style);
  size_t pos = fname.find_last_of('.');
  எனில் (pos == StringRef::npos)
    திரும்பு StringRef();
  எனில் ((fname.size() == 1 && fname == ".") ||
      (fname.size() == 2 && fname == ".."))
    திரும்பு StringRef();
  திரும்பு fname.substr(pos);
}

தருக்க is_separator(எழுத்து மதிப்பு_2, Style style) {
  எனில் (மதிப்பு_2 == '/')
    திரும்பு உண்மை;
  எனில் (is_style_windows(style))
    திரும்பு மதிப்பு_2 == '\\';
  திரும்பு பொய்;
}

StringRef get_separator(Style style) {
  எனில் (real_style(style) == Style::windows)
    திரும்பு "\\";
  திரும்பு "/";
}

தருக்க has_வேர்_name(மாறிலி Twine &path, Style style) {
  SmallString<128> path_storage;
  StringRef p = path.toStringRef(path_storage);

  திரும்பு !வேர்_name(p, style).empty();
}

தருக்க has_வேர்_directory(மாறிலி Twine &path, Style style) {
  SmallString<128> path_storage;
  StringRef p = path.toStringRef(path_storage);

  திரும்பு !வேர்_directory(p, style).empty();
}

தருக்க has_வேர்_path(மாறிலி Twine &path, Style style) {
  SmallString<128> path_storage;
  StringRef p = path.toStringRef(path_storage);

  திரும்பு !வேர்_path(p, style).empty();
}

தருக்க has_relative_path(மாறிலி Twine &path, Style style) {
  SmallString<128> path_storage;
  StringRef p = path.toStringRef(path_storage);

  திரும்பு !relative_path(p, style).empty();
}

தருக்க has_filename(மாறிலி Twine &path, Style style) {
  SmallString<128> path_storage;
  StringRef p = path.toStringRef(path_storage);

  திரும்பு !filename(p, style).empty();
}

தருக்க has_parent_path(மாறிலி Twine &path, Style style) {
  SmallString<128> path_storage;
  StringRef p = path.toStringRef(path_storage);

  திரும்பு !parent_path(p, style).empty();
}

தருக்க has_stem(மாறிலி Twine &path, Style style) {
  SmallString<128> path_storage;
  StringRef p = path.toStringRef(path_storage);

  திரும்பு !stem(p, style).empty();
}

தருக்க has_extension(மாறிலி Twine &path, Style style) {
  SmallString<128> path_storage;
  StringRef p = path.toStringRef(path_storage);

  திரும்பு !extension(p, style).empty();
}

தருக்க is_absolute(மாறிலி Twine &path, Style style) {
  SmallString<128> path_storage;
  StringRef p = path.toStringRef(path_storage);

  தருக்க வேர்_Dir = has_வேர்_directory(p, style);
  தருக்க வேர்_Name = is_style_posix(style) || has_வேர்_name(p, style);

  திரும்பு வேர்_Dir && வேர்_Name;
}

தருக்க is_absolute_gnu(மாறிலி Twine &path, Style style) {
  SmallString<128> path_storage;
  StringRef p = path.toStringRef(path_storage);

  // Handle '/' which is absolute for both Windows and POSIX systems.
  // Handle '\\' on Windows.
  எனில் (!p.empty() && is_separator(p.front(), style))
    திரும்பு உண்மை;

  எனில் (is_style_windows(style)) {
    // Handle drive letter pattern (a character followed by ':') on Windows.
    எனில் (p.size() >= 2 && (p[0] && p[1] == ':'))
      திரும்பு உண்மை;
  }

  திரும்பு பொய்;
}

தருக்க is_relative(மாறிலி Twine &path, Style style) {
  திரும்பு !is_absolute(path, style);
}

வெறுமை make_absolute(மாறிலி Twine &current_directory,
                   SmallVectorImpl<எழுத்து> &path) {
  StringRef p(path.data(), path.size());

  தருக்க வேர்_Directory = has_வேர்_directory(p);
  தருக்க வேர்_Name = has_வேர்_name(p);

  // Already absolute.
  எனில் ((வேர்_Name || is_style_posix(Style::native)) && வேர்_Directory)
    திரும்பு;

  // All the following conditions will need the current directory.
  SmallString<128> current_dir;
  current_directory.toVector(current_dir);

  // Relative path. Prepend the current directory.
  எனில் (!வேர்_Name && !வேர்_Directory) {
    // Append path to the current directory.
    append(current_dir, p);
    // Set path to the result.
    path.swap(current_dir);
    திரும்பு;
  }

  எனில் (!வேர்_Name && வேர்_Directory) {
    StringRef cdrn = வேர்_name(current_dir);
    SmallString<128> cur_Dir_வேர்_Name(cdrn.begin(), cdrn.end());
    append(cur_Dir_வேர்_Name, p);
    // Set path to the result.
    path.swap(cur_Dir_வேர்_Name);
    திரும்பு;
  }

  எனில் (வேர்_Name && !வேர்_Directory) {
    StringRef p_வேர்_Name = வேர்_name(p);
    StringRef b_வேர்_Directory = வேர்_directory(current_dir);
    StringRef bRelativePath = relative_path(current_dir);
    StringRef pRelativePath = relative_path(p);

    SmallString<128> res;
    append(res, p_வேர்_Name, b_வேர்_Directory, bRelativePath, pRelativePath);
    path.swap(res);
    திரும்பு;
  }

  llvm_unreachable("All rootName and rootDirectory combinations should have "
                   "occurred above!");
}

StringRef remove_leading_dotslash(StringRef Path, Style style) {
  // Remove leading "./" (or ".//" or "././" etc.)
  போது (Path.size() > 2 && Path[0] == '.' && is_separator(Path[1], style)) {
    Path = Path.substr(2);
    போது (Path.size() > 0 && is_separator(Path[0], style))
      Path = Path.substr(1);
  }
  திரும்பு Path;
}

தருக்க remove_dots(SmallVectorImpl<எழுத்து> &the_path, தருக்க remove_dot_dot,
                 Style style) {
  style = real_style(style);
  StringRef remaining(the_path.data(), the_path.size());
  தருக்க needs_change = பொய்;
  SmallVector<StringRef, 16> components;

  // Consume the root path, if present.
  StringRef வேர்_3 = path::வேர்_path(remaining, style);
  தருக்க absolute = !வேர்_3.empty();
  எனில் (absolute)
    remaining = remaining.drop_front(வேர்_3.size());

  // Loop over path components manually. This makes it easier to detect
  // non-preferred slashes and double separators that must be canonicalized.
  போது (!remaining.empty()) {
    size_t next_slash = remaining.find_first_of(separators(style));
    எனில் (next_slash == StringRef::npos)
      next_slash = remaining.size();
    StringRef component = remaining.take_front(next_slash);
    remaining = remaining.drop_front(next_slash);

    // Eat the slash, and check if it is the preferred separator.
    எனில் (!remaining.empty()) {
      needs_change |= remaining.front() != preferred_separator(style);
      remaining = remaining.drop_front();
      // The path needs to be rewritten if it has a trailing slash.
      // FIXME: This is emergent behavior that could be removed.
      needs_change |= remaining.empty();
    }

    // Check for path traversal components or double separators.
    எனில் (component.empty() || component == ".") {
      needs_change = உண்மை;
    } இல்லையெனில் எனில் (remove_dot_dot && component == "..") {
      needs_change = உண்மை;
      // Do not allow ".." to remove the root component. If this is the
      // beginning of a relative path, keep the ".." component.
      எனில் (!components.empty() && components.back() != "..") {
        components.எடு_back();
      } இல்லையெனில் எனில் (!absolute) {
        components.தள்ளு_back(component);
      }
    } இல்லையெனில் {
      components.தள்ளு_back(component);
    }
  }

  SmallString<256> buffer = வேர்_3;
  // "root" could be "/", which may need to be translated into "\".
  make_preferred(buffer, style);
  needs_change |= வேர்_3 != buffer;

  // Avoid rewriting the path unless we have to.
  எனில் (!needs_change)
    திரும்பு பொய்;

  எனில் (!components.empty()) {
    buffer += components[0];
    க்கு (StringRef C : ArrayRef(components).drop_front()) {
      buffer += preferred_separator(style);
      buffer += C;
    }
  }
  the_path.swap(buffer);
  திரும்பு உண்மை;
}

} // end namespace path

நாமாகாஶ fs {

std::error_code getUniqueID(மாறிலி Twine Path, UniqueID &Result) {
  sandbox::violationIfEnabled();

  file_status Status;
  std::error_code EC = status(Path, Status);
  எனில் (EC)
    திரும்பு EC;
  Result = Status.getUniqueID();
  திரும்பு std::error_code();
}

வெறுமை createUniquePath(மாறிலி Twine &Model, SmallVectorImpl<எழுத்து> &ResultPath,
                      தருக்க MakeAbsolute) {
  SmallString<128> ModelStorage;
  Model.toVector(ModelStorage);

  ப்ரதிஜ்ஞா(llvm::is_contained(ModelStorage, '%') &&
         "createUniquePath: Model must contain at least one '%'");

  எனில் (MakeAbsolute) {
    // Make model absolute by prepending a temp directory if it's not already.
    எனில் (!sys::path::is_absolute(Twine(ModelStorage))) {
      SmallString<128> TDir;
      sys::path::system_temp_directory(உண்மை, TDir);
      sys::path::append(TDir, Twine(ModelStorage));
      ModelStorage.swap(TDir);
    }
  }

  ResultPath = ModelStorage;
  ResultPath.தள்ளு_back(0);
  ResultPath.எடு_back();

  // Replace '%' with random chars.
  க்கு (unsigned i = 0, e = ModelStorage.size(); i != e; ++i) {
    எனில் (ModelStorage[i] == '%')
      ResultPath[i] = "0123456789abcdef"[sys::Process::Get_Random_எண்() & 15];
  }
}

std::error_code createUniqueFile(மாறிலி Twine &Model, முழு &ResultFd,
                                 SmallVectorImpl<எழுத்து> &ResultPath,
                                 OpenFlags Flags, unsigned Mode) {
  திரும்பு createUniqueEntity(Model, ResultFd, ResultPath, பொய், FS_File, Flags,
                            Mode);
}

std::error_code createUniqueFile(மாறிலி Twine &Model,
                                 SmallVectorImpl<எழுத்து> &ResultPath,
                                 unsigned Mode) {
  முழு FD;
  auto EC = createUniqueFile(Model, FD, ResultPath, OF_None, Mode);
  எனில் (EC)
    திரும்பு EC;
  // FD is only needed to avoid race conditions. Close it right away.
  close(FD);
  திரும்பு EC;
}

நிலையான std::error_code
createTemporaryFile(மாறிலி Twine &Model, முழு &ResultFD,
                    llvm::SmallVectorImpl<எழுத்து> &ResultPath, FSEntity Type,
                    sys::fs::OpenFlags Flags = sys::fs::OF_None) {
  // Any *temporary* file is assumed to be a compiler-internal output, not
  // a formal one.
  auto BypassSandbox = sys::sandbox::scopedDisable();

  SmallString<128> Storage;
  StringRef P = Model.toNullTerminatedStringRef(Storage);
  ப்ரதிஜ்ஞா(P.find_first_of(separators(Style::native)) == StringRef::npos &&
         "Model must be a simple filename.");
  // Use P.begin() so that createUniqueEntity doesn't need to recreate Storage.
  திரும்பு createUniqueEntity(P.begin(), ResultFD, ResultPath, உண்மை, Type, Flags,
                            all_read | all_write);
}

நிலையான std::error_code
createTemporaryFile(மாறிலி Twine &Prefix, StringRef Suffix, முழு &ResultFD,
                    llvm::SmallVectorImpl<எழுத்து> &ResultPath, FSEntity Type,
                    sys::fs::OpenFlags Flags = sys::fs::OF_None) {
  மாறிலி எழுத்து *நடு = Suffix.empty() ? "-%%%%%%" : "-%%%%%%.";
  திரும்பு createTemporaryFile(Prefix + நடு + Suffix, ResultFD, ResultPath,
                             Type, Flags);
}

std::error_code createTemporaryFile(மாறிலி Twine &Prefix, StringRef Suffix,
                                    முழு &ResultFD,
                                    SmallVectorImpl<எழுத்து> &ResultPath,
                                    sys::fs::OpenFlags Flags) {
  திரும்பு createTemporaryFile(Prefix, Suffix, ResultFD, ResultPath, FS_File,
                             Flags);
}

std::error_code createTemporaryFile(மாறிலி Twine &Prefix, StringRef Suffix,
                                    SmallVectorImpl<எழுத்து> &ResultPath,
                                    sys::fs::OpenFlags Flags) {
  முழு FD;
  auto EC = createTemporaryFile(Prefix, Suffix, FD, ResultPath, Flags);
  எனில் (EC)
    திரும்பு EC;
  // FD is only needed to avoid race conditions. Close it right away.
  close(FD);
  திரும்பு EC;
}

// This is a mkdtemp with a different pattern. We use createUniqueEntity mostly
// for consistency. We should try using mkdtemp.
std::error_code createUniqueDirectory(மாறிலி Twine &Prefix,
                                      SmallVectorImpl<எழுத்து> &ResultPath) {
  முழு Dummy;
  திரும்பு createUniqueEntity(Prefix + "-%%%%%%", Dummy, ResultPath, உண்மை,
                            FS_Dir);
}

std::error_code
getPotentiallyUniqueFileName(மாறிலி Twine &Model,
                             SmallVectorImpl<எழுத்து> &ResultPath) {
  முழு Dummy;
  திரும்பு createUniqueEntity(Model, Dummy, ResultPath, பொய், FS_Name);
}

std::error_code
getPotentiallyUniqueTempFileName(மாறிலி Twine &Prefix, StringRef Suffix,
                                 SmallVectorImpl<எழுத்து> &ResultPath) {
  முழு Dummy;
  திரும்பு createTemporaryFile(Prefix, Suffix, Dummy, ResultPath, FS_Name);
}

std::error_code make_absolute(SmallVectorImpl<எழுத்து> &path) {
  sandbox::violationIfEnabled();

  எனில் (path::is_absolute(path))
    திரும்பு {};

  SmallString<128> current_dir;
  எனில் (std::error_code ec = current_path(current_dir))
    திரும்பு ec;

  path::make_absolute(current_dir, path);
  திரும்பு {};
}

std::error_code create_directories(மாறிலி Twine &Path, தருக்க IgnoreExisting,
                                   perms Perms) {
  SmallString<128> PathStorage;
  StringRef P = Path.toStringRef(PathStorage);

  // Be optimistic and try to create the directory
  std::error_code EC = create_directory(P, IgnoreExisting, Perms);
  // If we succeeded, or had any error other than the parent not existing, just
  // return it.
  எனில் (EC != errc::no_such_file_or_directory)
    திரும்பு EC;

  // We failed because of a no_such_file_or_directory, try to create the
  // parent.
  StringRef Parent = path::parent_path(P);
  எனில் (Parent.empty())
    திரும்பு EC;

  எனில் ((EC = create_directories(Parent, IgnoreExisting, Perms)))
      திரும்பு EC;

  திரும்பு create_directory(P, IgnoreExisting, Perms);
}

நிலையான std::error_code copy_file_internal(முழு ReadFD, முழு WriteFD) {
  மாறிலி size_t BufSize = 4096;
  எழுத்து *Buf = புதிய எழுத்து[BufSize];
  முழு BytesRead = 0, BytesWritten = 0;
  க்கு (;;) {
    BytesRead = பட(ReadFD, Buf, BufSize);
    எனில் (BytesRead <= 0)
      நிறுத்து;
    போது (BytesRead) {
      BytesWritten = லிக(WriteFD, Buf, BytesRead);
      எனில் (BytesWritten < 0)
        நிறுத்து;
      BytesRead -= BytesWritten;
    }
    எனில் (BytesWritten < 0)
      நிறுத்து;
  }
  delete[] Buf;

  எனில் (BytesRead < 0 || BytesWritten < 0)
    திரும்பு errnoAsErrorCode();
  திரும்பு std::error_code();
}

#ifndef __APPLE__
std::error_code copy_file(மாறிலி Twine &From, மாறிலி Twine &To) {
  முழு ReadFD, WriteFD;
  எனில் (std::error_code EC = openFileForRead(From, ReadFD, OF_None))
    திரும்பு EC;
  எனில் (std::error_code EC =
          openFileForWrite(To, WriteFD, CD_CreateAlways, OF_None)) {
    close(ReadFD);
    திரும்பு EC;
  }

  std::error_code EC = copy_file_internal(ReadFD, WriteFD);

  close(ReadFD);
  close(WriteFD);

  திரும்பு EC;
}
#endif

std::error_code copy_file(மாறிலி Twine &From, முழு ToFD) {
  முழு ReadFD;
  எனில் (std::error_code EC = openFileForRead(From, ReadFD, OF_None))
    திரும்பு EC;

  std::error_code EC = copy_file_internal(ReadFD, ToFD);

  close(ReadFD);

  திரும்பு EC;
}

ErrorOr<MD5::MD5Result> md5_contents(முழு FD) {
  sandbox::violationIfEnabled();

  MD5 Hash;

  constexpr size_t BufSize = 4096;
  std::vector<uint8_t> Buf(BufSize);
  முழு BytesRead = 0;
  க்கு (;;) {
    BytesRead = பட(FD, Buf.data(), BufSize);
    எனில் (BytesRead <= 0)
      நிறுத்து;
    Hash.update(ArrayRef(Buf.data(), BytesRead));
  }

  எனில் (BytesRead < 0)
    திரும்பு errnoAsErrorCode();
  MD5::MD5Result Result;
  Hash.final(Result);
  திரும்பு Result;
}

ErrorOr<MD5::MD5Result> md5_contents(மாறிலி Twine &Path) {
  sandbox::violationIfEnabled();

  முழு FD;
  எனில் (auto EC = openFileForRead(Path, FD, OF_None))
    திரும்பு EC;

  auto Result = md5_contents(FD);
  close(FD);
  திரும்பு Result;
}

தருக்க exists(மாறிலி basic_file_status &status) {
  திரும்பு status_known(status) && status.type() != file_type::file_not_found;
}

தருக்க status_known(மாறிலி basic_file_status &s) {
  திரும்பு s.type() != file_type::status_error;
}

file_type get_file_type(மாறிலி Twine &Path, தருக்க Follow) {
  file_status st;
  எனில் (status(Path, st, Follow))
    திரும்பு file_type::status_error;
  திரும்பு st.type();
}

தருக்க is_directory(மாறிலி basic_file_status &status) {
  திரும்பு status.type() == file_type::directory_file;
}

std::error_code is_directory(மாறிலி Twine &path, தருக்க &result) {
  sandbox::violationIfEnabled();

  file_status st;
  எனில் (std::error_code ec = status(path, st))
    திரும்பு ec;
  result = is_directory(st);
  திரும்பு std::error_code();
}

தருக்க is_regular_file(மாறிலி basic_file_status &status) {
  திரும்பு status.type() == file_type::regular_file;
}

std::error_code is_regular_file(மாறிலி Twine &path, தருக்க &result) {
  sandbox::violationIfEnabled();

  file_status st;
  எனில் (std::error_code ec = status(path, st))
    திரும்பு ec;
  result = is_regular_file(st);
  திரும்பு std::error_code();
}

தருக்க is_symlink_file(மாறிலி basic_file_status &status) {
  திரும்பு status.type() == file_type::symlink_file;
}

std::error_code is_symlink_file(மாறிலி Twine &path, தருக்க &result) {
  sandbox::violationIfEnabled();

  file_status st;
  எனில் (std::error_code ec = status(path, st, பொய்))
    திரும்பு ec;
  result = is_symlink_file(st);
  திரும்பு std::error_code();
}

தருக்க is_other(மாறிலி basic_file_status &status) {
  திரும்பு exists(status) &&
         !is_regular_file(status) &&
         !is_directory(status);
}

std::error_code is_other(மாறிலி Twine &Path, தருக்க &Result) {
  sandbox::violationIfEnabled();

  file_status FileStatus;
  எனில் (std::error_code EC = status(Path, FileStatus))
    திரும்பு EC;
  Result = is_other(FileStatus);
  திரும்பு std::error_code();
}

வெறுமை directory_entry::replace_filename(மாறிலி Twine &Filename, file_type Type,
                                       basic_file_status Status) {
  SmallString<128> PathStr = path::parent_path(Path);
  path::append(PathStr, Filename);
  இது->Path = std::string(PathStr);
  இது->Type = Type;
  இது->Status = Status;
}

ErrorOr<perms> getPermissions(மாறிலி Twine &Path) {
  sandbox::violationIfEnabled();

  file_status Status;
  எனில் (std::error_code EC = status(Path, Status))
    திரும்பு EC;

  திரும்பு Status.permissions();
}

std::error_code setLastAccessAndModificationTime(மாறிலி Twine &Path,
                                                 TimePoint<> AccessTime,
                                                 TimePoint<> ModificationTime) {
  முழு FD;
  எனில் (std::error_code EC = openFile(Path, FD, CD_OpenExisting, FA_Read,
                                    OF_UpdateAttributes | OF_OpenDirectory))
    திரும்பு EC;
  std::error_code EC =
      setLastAccessAndModificationTime(FD, AccessTime, ModificationTime);
  Process::SafelyCloseFileDescriptor(FD);
  திரும்பு EC;
}

size_t mapped_file_region::size() மாறிலி {
  ப்ரதிஜ்ஞா(Mapping && "Mapping failed but used anyway!");
  திரும்பு Size;
}

எழுத்து *mapped_file_region::data() மாறிலி {
  ப்ரதிஜ்ஞா(Mapping && "Mapping failed but used anyway!");
  திரும்பு reinterpret_cast<எழுத்து *>(Mapping);
}

மாறிலி எழுத்து *mapped_file_region::const_data() மாறிலி {
  ப்ரதிஜ்ஞா(Mapping && "Mapping failed but used anyway!");
  திரும்பு reinterpret_cast<மாறிலி எழுத்து *>(Mapping);
}

Error readNativeFileToEOF(file_t FileHandle, SmallVectorImpl<எழுத்து> &Buffer,
                          ssize_t ChunkSize) {
  sandbox::violationIfEnabled();

  // Install a handler to truncate the buffer to the correct size on exit.
  size_t Size = Buffer.size();
  llvm::scope_exit TruncateOnExit([&]() { Buffer.truncate(Size); });

  // Read into Buffer until we hit EOF.
  க்கு (;;) {
    Buffer.resize_for_overwrite(Size + ChunkSize);
    Expected<size_t> ReadBytes = readNativeFile(
        FileHandle, MutableArrayRef(Buffer.begin() + Size, ChunkSize));
    எனில் (!ReadBytes)
      திரும்பு ReadBytes.takeError();
    எனில் (*ReadBytes == 0)
      திரும்பு Error::success();
    Size += *ReadBytes;
  }
}

} // end namespace fs
} // end namespace sys
} // end namespace llvm

// Include the truly platform-specific parts.
#எனில் defined(LLVM_ON_UNIX)
#அந்தர்பாவ "Unix/Path.inc"
#endif
#எனில் defined(_WIN32)
#அந்தர்பாவ "Windows/Path.inc"
#endif

நாமாகாஶ llvm {
நாமாகாஶ sys {
நாமாகாஶ fs {

TempFile::TempFile(StringRef Name, முழு FD)
    : TmpName(std::string(Name)), FD(FD) {}
TempFile::TempFile(TempFile &&Other) { *இது = std::move(Other); }
TempFile &TempFile::operator=(TempFile &&Other) {
  TmpName = std::move(Other.TmpName);
  FD = Other.FD;
  Other.Done = உண்மை;
  Other.FD = -1;
#ifdef _WIN32
  RemoveOnClose = Other.RemoveOnClose;
  Other.RemoveOnClose = பொய்;
#endif
  திரும்பு *இது;
}

TempFile::~TempFile() { ப்ரதிஜ்ஞா(Done); }

Error TempFile::discard() {
  Done = உண்மை;
  எனில் (FD != -1 && close(FD) == -1) {
    std::error_code EC = errnoAsErrorCode();
    திரும்பு errorCodeToError(EC);
  }
  FD = -1;

#ifdef _WIN32
  // On Windows, closing will remove the file, if we set the delete
  // disposition. If not, remove it manually.
  தருக்க Remove = RemoveOnClose;
#இல்லையெனில்
  // Always try to remove the file.
  தருக்க Remove = உண்மை;
#endif
  std::error_code RemoveEC;
  எனில் (Remove && !TmpName.empty()) {
    RemoveEC = fs::remove(TmpName);
    sys::DontRemoveFileOnSignal(TmpName);
    எனில் (!RemoveEC)
      TmpName = "";
  } இல்லையெனில் {
    TmpName = "";
  }
  திரும்பு errorCodeToError(RemoveEC);
}

Error TempFile::keep(மாறிலி Twine &Name) {
  ப்ரதிஜ்ஞா(!Done);
  Done = உண்மை;
  // Always try to close and rename.
#ifdef _WIN32
  // If we can't cancel the delete don't rename.
  auto H = reinterpret_cast<HANDLE>(_get_osfhandle(FD));
  std::error_code RenameEC =
      RemoveOnClose ? std::error_code() : setDeleteDisposition(H, பொய்);
  தருக்க ShouldDelete = பொய்;
  எனில் (!RenameEC) {
    RenameEC = rename_handle(H, Name);
    // If rename failed because it's cross-device, copy instead
    எனில் (RenameEC ==
      std::error_code(ERROR_NOT_SAME_DEVICE, std::system_category())) {
      RenameEC = copy_file(TmpName, Name);
      ShouldDelete = உண்மை;
    }
  }

  // If we can't rename or copy, discard the temporary file.
  எனில் (RenameEC)
    ShouldDelete = உண்மை;
  எனில் (ShouldDelete) {
    எனில் (!RemoveOnClose)
      setDeleteDisposition(H, உண்மை);
    இல்லையெனில்
      remove(TmpName);
  }
#இல்லையெனில்
  std::error_code RenameEC = fs::rename(TmpName, Name);
  எனில் (RenameEC) {
    // If we can't rename, try to copy to work around cross-device link issues.
    RenameEC = sys::fs::copy_file(TmpName, Name);
    // If we can't rename or copy, discard the temporary file.
    எனில் (RenameEC)
      remove(TmpName);
  }
#endif
  sys::DontRemoveFileOnSignal(TmpName);

  எனில் (!RenameEC)
    TmpName = "";

  எனில் (close(FD) == -1)
    திரும்பு errorCodeToError(errnoAsErrorCode());
  FD = -1;

  திரும்பு errorCodeToError(RenameEC);
}

Error TempFile::keep() {
  ப்ரதிஜ்ஞா(!Done);
  Done = உண்மை;

#ifdef _WIN32
  auto H = reinterpret_cast<HANDLE>(_get_osfhandle(FD));
  எனில் (std::error_code EC = setDeleteDisposition(H, பொய்))
    திரும்பு errorCodeToError(EC);
#endif
  sys::DontRemoveFileOnSignal(TmpName);

  TmpName = "";

  எனில் (close(FD) == -1)
    திரும்பு errorCodeToError(errnoAsErrorCode());
  FD = -1;

  திரும்பு Error::success();
}

Expected<TempFile> TempFile::create(மாறிலி Twine &Model, unsigned Mode,
                                    OpenFlags ExtraFlags) {
  முழு FD;
  SmallString<128> ResultPath;
  எனில் (std::error_code EC =
          createUniqueFile(Model, FD, ResultPath, OF_Delete | ExtraFlags, Mode))
    திரும்பு errorCodeToError(EC);

  TempFile Ret(ResultPath, FD);
#ifdef _WIN32
  auto H = reinterpret_cast<HANDLE>(_get_osfhandle(FD));
  தருக்க SetSignalHandler = பொய்;
  எனில் (std::error_code EC = setDeleteDisposition(H, உண்மை)) {
    Ret.RemoveOnClose = உண்மை;
    SetSignalHandler = உண்மை;
  }
#இல்லையெனில்
  தருக்க SetSignalHandler = உண்மை;
#endif
  எனில் (SetSignalHandler && sys::RemoveFileOnSignal(ResultPath)) {
    // Make sure we delete the file when RemoveFileOnSignal fails.
    consumeError(Ret.discard());
    std::error_code EC(errc::operation_not_permitted);
    திரும்பு errorCodeToError(EC);
  }
  திரும்பு std::move(Ret);
}
} // namespace fs

} // namespace sys
} // namespace llvm
