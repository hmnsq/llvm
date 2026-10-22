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

#অন্তর্ভাব "llvm/Support/Path.h"
#অন্তর্ভাব "llvm/ADT/ArrayRef.h"
#অন্তর্ভাব "llvm/ADT/ScopeExit.h"
#অন্তর্ভাব "llvm/ADT/StringExtras.h"
#অন্তর্ভাব "llvm/Config/config.h"
#অন্তর্ভাব "llvm/Config/llvm-config.h"
#অন্তর্ভাব "llvm/Support/Errc.h"
#অন্তর্ভাব "llvm/Support/ErrorHandling.h"
#অন্তর্ভাব "llvm/Support/FileSystem.h"
#অন্তর্ভাব "llvm/Support/IOSandbox.h"
#অন্তর্ভাব "llvm/Support/Process.h"
#অন্তর্ভাব "llvm/Support/Signals.h"
#অন্তর্ভাব <cctype>

#যদি !defined(_MSC_VER) && !defined(__MINGW32__)
#অন্তর্ভাব <unistd.h>
#অন্যথা
#অন্তর্ভাব <io.h>
#endif

using নামাকাশ llvm;
using নামাকাশ llvm::support::endian;

নামাকাশ {
  using llvm::StringRef;
  using llvm::sys::path::is_separator;
  using llvm::sys::path::Style;

  অন্তঃস্থ তার্কিক prefer_forward_slash() {
    স্থির তার্কিক prefer = []() {
      যদি (std::optional<std::string> Env =
              sys::Process::GetEnv("LLVM_WINDOWS_PREFER_FORWARD_SLASH"))
        প্রত্যাগম *Env == "1";
      প্রত্যাগম static_cast<তার্কিক>(LLVM_WINDOWS_PREFER_FORWARD_SLASH);
    }();
    প্রত্যাগম prefer;
  }

  অন্তঃস্থ Style real_style(Style style) {
    যদি (style != Style::native)
      প্রত্যাগম style;
    যদি (is_style_posix(style))
      প্রত্যাগম Style::posix;
    প্রত্যাগম prefer_forward_slash() ? Style::windows_slash
                                  : Style::windows_backslash;
  }

  অন্তঃস্থ ধ্রুব বর্ণ *separators(Style style) {
    যদি (is_style_windows(style))
      প্রত্যাগম "\\/";
    প্রত্যাগম "/";
  }

  অন্তঃস্থ বর্ণ preferred_separator(Style style) {
    যদি (real_style(style) == Style::windows)
      প্রত্যাগম '\\';
    প্রত্যাগম '/';
  }

  StringRef find_first_component(StringRef path, Style style) {
    // Look for this first component in the following order.
    // * empty (in this case we return an empty string)
    // * either C: or {//,\\}net.
    // * {/,\}
    // * {file,directory}name

    যদি (path.empty())
      প্রত্যাগম path;

    যদি (is_style_windows(style)) {
      // C:
      যদি (path.size() >= 2 &&
          std::isalpha(static_cast<unsigned বর্ণ>(path[0])) && path[1] == ':')
        প্রত্যাগম path.substr(0, 2);
    }

    // //net
    যদি ((path.size() > 2) && is_separator(path[0], style) &&
        path[0] == path[1] && !is_separator(path[2], style)) {
      // Find the next directory separator.
      size_t end = path.find_first_of(separators(style), 2);
      প্রত্যাগম path.substr(0, end);
    }

    // {/,\}
    যদি (is_separator(path[0], style))
      প্রত্যাগম path.substr(0, 1);

    // * {file,directory}name
    size_t end = path.find_first_of(separators(style));
    প্রত্যাগম path.substr(0, end);
  }

  // Returns the first character of the filename in str. For paths ending in
  // '/', it returns the position of the '/'.
  size_t filename_pos(StringRef str, Style style) {
    যদি (str.size() > 0 && is_separator(str[str.size() - 1], style))
      প্রত্যাগম str.size() - 1;

    size_t pos = str.find_last_of(separators(style), str.size() - 1);

    যদি (is_style_windows(style)) {
      যদি (pos == StringRef::npos)
        pos = str.find_last_of(':', str.size() - 1);
    }

    যদি (pos == StringRef::npos || (pos == 1 && is_separator(str[0], style)))
      প্রত্যাগম 0;

    প্রত্যাগম pos + 1;
  }

  // Returns the position of the root directory in str. If there is no root
  // directory in str, it returns StringRef::npos.
  size_t মূল_dir_start(StringRef str, Style style) {
    // case "c:/"
    যদি (is_style_windows(style)) {
      যদি (str.size() > 2 && str[1] == ':' && is_separator(str[2], style))
        প্রত্যাগম 2;
    }

    // case "//net"
    যদি (str.size() > 3 && is_separator(str[0], style) && str[0] == str[1] &&
        !is_separator(str[2], style)) {
      প্রত্যাগম str.find_first_of(separators(style), 2);
    }

    // case "/"
    যদি (str.size() > 0 && is_separator(str[0], style))
      প্রত্যাগম 0;

    প্রত্যাগম StringRef::npos;
  }

  // Returns the position past the end of the "parent path" of path. The parent
  // path will not end in '/', unless the parent is the root directory. If the
  // path has no parent, 0 is returned.
  size_t parent_path_end(StringRef path, Style style) {
    size_t end_pos = filename_pos(path, style);

    তার্কিক filename_was_sep =
        path.size() > 0 && is_separator(path[end_pos], style);

    // Skip separators until we reach root dir (or the start of the string).
    size_t মূল_dir_pos = মূল_dir_start(path, style);
    যাবত্ (end_pos > 0 &&
           (মূল_dir_pos == StringRef::npos || end_pos > মূল_dir_pos) &&
           is_separator(path[end_pos - 1], style))
      --end_pos;

    যদি (end_pos == মূল_dir_pos && !filename_was_sep) {
      // We've reached the root dir and the input path was *not* ending in a
      // sequence of slashes. Include the root dir in the parent path.
      প্রত্যাগম মূল_dir_pos + 1;
    }

    // Otherwise, just include before the last slash.
    প্রত্যাগম end_pos;
  }
} // end unnamed namespace

গণনা FSEntity {
  FS_Dir,
  FS_File,
  FS_Name
};

স্থির std::error_code
createUniqueEntity(ধ্রুব Twine &Model, পূর্ণাঙ্ক &ResultFD,
                   SmallVectorImpl<বর্ণ> &ResultPath, তার্কিক MakeAbsolute,
                   FSEntity Type, sys::fs::OpenFlags Flags = sys::fs::OF_None,
                   unsigned Mode = 0) {

  // Limit the number of attempts we make, so that we don't infinite loop. E.g.
  // "permission denied" could be for a specific file (so we retry with a
  // different name) or for the whole directory (retry would always fail).
  // Checking which is racy, so we try a number of times, then give up.
  std::error_code EC;
  প্রতি (পূর্ণাঙ্ক Retries = 128; Retries > 0; --Retries) {
    sys::fs::createUniquePath(Model, ResultPath, MakeAbsolute);
    // Try to open + create the file.
    বিকল্প (Type) {
    পক্ষ FS_File: {
      EC = sys::fs::openFileForReadWrite(Twine(ResultPath.begin()), ResultFD,
                                         sys::fs::CD_CreateNew, Flags, Mode);
      যদি (EC) {
        // errc::permission_denied happens on Windows when we try to open a file
        // that has been marked for deletion.
        যদি (EC == errc::file_exists || EC == errc::permission_denied)
          অনুবর্ত;
        প্রত্যাগম EC;
      }

      প্রত্যাগম std::error_code();
    }

    পক্ষ FS_Name: {
      EC = sys::fs::access(ResultPath.begin(), sys::fs::AccessMode::Exist);
      যদি (EC == errc::no_such_file_or_directory)
        প্রত্যাগম std::error_code();
      যদি (EC)
        প্রত্যাগম EC;
      অনুবর্ত;
    }

    পক্ষ FS_Dir: {
      EC = sys::fs::create_directory(ResultPath.begin(), অসত্য);
      যদি (EC) {
        যদি (EC == errc::file_exists)
          অনুবর্ত;
        প্রত্যাগম EC;
      }
      প্রত্যাগম std::error_code();
    }
    }
    llvm_unreachable("Invalid Type");
  }
  প্রত্যাগম EC;
}

নামাকাশ llvm {
নামাকাশ sys  {
নামাকাশ path {

const_iterator begin(StringRef path, Style style) {
  const_iterator i;
  i.Path      = path;
  i.Component = find_first_component(path, style);
  i.Position  = 0;
  i.S = style;
  প্রত্যাগম i;
}

const_iterator end(StringRef path) {
  const_iterator i;
  i.Path      = path;
  i.Position  = path.size();
  প্রত্যাগম i;
}

const_iterator &const_iterator::operator++() {
  প্রতিজ্ঞা(Position < Path.size() && "Tried to increment past end!");

  // Increment Position to past the current component
  Position += Component.size();

  // Check for end.
  যদি (Position == Path.size()) {
    Component = StringRef();
    প্রত্যাগম *এতত্;
  }

  // Both POSIX and Windows treat paths that begin with exactly two separators
  // specially.
  তার্কিক was_net = Component.size() > 2 && is_separator(Component[0], S) &&
                 Component[1] == Component[0] && !is_separator(Component[2], S);

  // Handle separators.
  যদি (is_separator(Path[Position], S)) {
    // Root dir.
    যদি (was_net ||
        // c:/
        (is_style_windows(S) && Component.ends_with(":"))) {
      Component = Path.substr(Position, 1);
      প্রত্যাগম *এতত্;
    }

    // Skip extra separators.
    যাবত্ (Position != Path.size() && is_separator(Path[Position], S)) {
      ++Position;
    }

    // Treat trailing '/' as a '.', unless it is the root dir.
    যদি (Position == Path.size() && Component != "/") {
      --Position;
      Component = ".";
      প্রত্যাগম *এতত্;
    }
  }

  // Find next component.
  size_t end_pos = Path.find_first_of(separators(S), Position);
  Component = Path.slice(Position, end_pos);

  প্রত্যাগম *এতত্;
}

তার্কিক const_iterator::operator==(ধ্রুব const_iterator &RHS) ধ্রুব {
  প্রত্যাগম Path.begin() == RHS.Path.begin() && Position == RHS.Position;
}

ptrdiff_t const_iterator::operator-(ধ্রুব const_iterator &RHS) ধ্রুব {
  প্রত্যাগম Position - RHS.Position;
}

reverse_iterator rbegin(StringRef Path, Style style) {
  reverse_iterator I;
  I.Path = Path;
  I.Position = Path.size();
  I.S = style;
  ++I;
  প্রত্যাগম I;
}

reverse_iterator rend(StringRef Path) {
  reverse_iterator I;
  I.Path = Path;
  I.Component = Path.substr(0, 0);
  I.Position = 0;
  প্রত্যাগম I;
}

reverse_iterator &reverse_iterator::operator++() {
  size_t মূল_dir_pos = মূল_dir_start(Path, S);

  // Skip separators unless it's the root directory.
  size_t end_pos = Position;
  যাবত্ (end_pos > 0 && (end_pos - 1) != মূল_dir_pos &&
         is_separator(Path[end_pos - 1], S))
    --end_pos;

  // Treat trailing '/' as a '.', unless it is the root dir.
  যদি (Position == Path.size() && !Path.empty() &&
      is_separator(Path.back(), S) &&
      (মূল_dir_pos == StringRef::npos || end_pos - 1 > মূল_dir_pos)) {
    --Position;
    Component = ".";
    প্রত্যাগম *এতত্;
  }

  // Find next separator.
  size_t start_pos = filename_pos(Path.substr(0, end_pos), S);
  Component = Path.slice(start_pos, end_pos);
  Position = start_pos;
  প্রত্যাগম *এতত্;
}

তার্কিক reverse_iterator::operator==(ধ্রুব reverse_iterator &RHS) ধ্রুব {
  প্রত্যাগম Path.begin() == RHS.Path.begin() && Component == RHS.Component &&
         Position == RHS.Position;
}

ptrdiff_t reverse_iterator::operator-(ধ্রুব reverse_iterator &RHS) ধ্রুব {
  প্রত্যাগম Position - RHS.Position;
}

StringRef মূল_path(StringRef path, Style style) {
  const_iterator b = begin(path, style), pos = b, e = end(path);
  যদি (b != e) {
    তার্কিক has_net =
        b->size() > 2 && is_separator((*b)[0], style) && (*b)[1] == (*b)[0];
    তার্কিক has_drive = is_style_windows(style) && b->ends_with(":");

    যদি (has_net || has_drive) {
      যদি ((++pos != e) && is_separator((*pos)[0], style)) {
        // {C:/,//net/}, so get the first two components.
        প্রত্যাগম path.substr(0, b->size() + pos->size());
      }
      // just {C:,//net}, return the first component.
      প্রত্যাগম *b;
    }

    // POSIX style root directory.
    যদি (is_separator((*b)[0], style)) {
      প্রত্যাগম *b;
    }
  }

  প্রত্যাগম StringRef();
}

StringRef মূল_name(StringRef path, Style style) {
  const_iterator b = begin(path, style), e = end(path);
  যদি (b != e) {
    তার্কিক has_net =
        b->size() > 2 && is_separator((*b)[0], style) && (*b)[1] == (*b)[0];
    তার্কিক has_drive = is_style_windows(style) && b->ends_with(":");

    যদি (has_net || has_drive) {
      // just {C:,//net}, return the first component.
      প্রত্যাগম *b;
    }
  }

  // No path or no name.
  প্রত্যাগম StringRef();
}

StringRef মূল_directory(StringRef path, Style style) {
  const_iterator b = begin(path, style), pos = b, e = end(path);
  যদি (b != e) {
    তার্কিক has_net =
        b->size() > 2 && is_separator((*b)[0], style) && (*b)[1] == (*b)[0];
    তার্কিক has_drive = is_style_windows(style) && b->ends_with(":");

    যদি ((has_net || has_drive) &&
        // {C:,//net}, skip to the next component.
        (++pos != e) && is_separator((*pos)[0], style)) {
      প্রত্যাগম *pos;
    }

    // POSIX style root directory.
    যদি (!has_net && is_separator((*b)[0], style)) {
      প্রত্যাগম *b;
    }
  }

  // No path or no root.
  প্রত্যাগম StringRef();
}

StringRef relative_path(StringRef path, Style style) {
  StringRef মূল_3 = মূল_path(path, style);
  প্রত্যাগম path.substr(মূল_3.size());
}

শূন্য append(SmallVectorImpl<বর্ণ> &path, Style style, ধ্রুব Twine &a,
            ধ্রুব Twine &b, ধ্রুব Twine &c, ধ্রুব Twine &d) {
  SmallString<32> a_storage;
  SmallString<32> b_storage;
  SmallString<32> c_storage;
  SmallString<32> d_storage;

  SmallVector<StringRef, 4> components;
  যদি (!a.isTriviallyEmpty()) components.ঠেলো_back(a.toStringRef(a_storage));
  যদি (!b.isTriviallyEmpty()) components.ঠেলো_back(b.toStringRef(b_storage));
  যদি (!c.isTriviallyEmpty()) components.ঠেলো_back(c.toStringRef(c_storage));
  যদি (!d.isTriviallyEmpty()) components.ঠেলো_back(d.toStringRef(d_storage));

  প্রতি (auto &component : components) {
    তার্কিক path_has_sep =
        !path.empty() && is_separator(path[path.size() - 1], style);
    যদি (path_has_sep) {
      // Strip separators from beginning of component.
      size_t loc = component.find_first_not_of(separators(style));
      StringRef c = component.substr(loc);

      // Append it.
      path.append(c.begin(), c.end());
      অনুবর্ত;
    }

    তার্কিক component_has_sep =
        !component.empty() && is_separator(component[0], style);
    যদি (!component_has_sep &&
        !(path.empty() || has_মূল_name(component, style))) {
      // Add a separator.
      path.ঠেলো_back(preferred_separator(style));
    }

    path.append(component.begin(), component.end());
  }
}

শূন্য append(SmallVectorImpl<বর্ণ> &path, ধ্রুব Twine &a, ধ্রুব Twine &b,
            ধ্রুব Twine &c, ধ্রুব Twine &d) {
  append(path, Style::native, a, b, c, d);
}

শূন্য append(SmallVectorImpl<বর্ণ> &path, const_iterator begin,
            const_iterator end, Style style) {
  প্রতি (; begin != end; ++begin)
    path::append(path, style, *begin);
}

StringRef parent_path(StringRef path, Style style) {
  size_t end_pos = parent_path_end(path, style);
  যদি (end_pos == StringRef::npos)
    প্রত্যাগম StringRef();
  প্রত্যাগম path.substr(0, end_pos);
}

শূন্য remove_filename(SmallVectorImpl<বর্ণ> &path, Style style) {
  size_t end_pos = parent_path_end(StringRef(path.begin(), path.size()), style);
  যদি (end_pos != StringRef::npos)
    path.truncate(end_pos);
}

শূন্য replace_extension(SmallVectorImpl<বর্ণ> &path, ধ্রুব Twine &extension,
                       Style style) {
  StringRef p(path.begin(), path.size());
  SmallString<32> ext_storage;
  StringRef ext = extension.toStringRef(ext_storage);

  // Erase existing extension.
  size_t pos = p.find_last_of('.');
  যদি (pos != StringRef::npos && pos >= filename_pos(p, style))
    path.truncate(pos);

  // Append '.' if needed.
  যদি (ext.size() > 0 && ext[0] != '.')
    path.ঠেলো_back('.');

  // Append extension.
  path.append(ext.begin(), ext.end());
}

স্থির তার্কিক starts_with(StringRef Path, StringRef Prefix,
                        Style style = Style::native) {
  // Windows prefix matching : case and separator insensitive
  যদি (is_style_windows(style)) {
    যদি (Path.size() < Prefix.size())
      প্রত্যাগম অসত্য;
    প্রতি (size_t I = 0, E = Prefix.size(); I != E; ++I) {
      তার্কিক SepPath = is_separator(Path[I], style);
      তার্কিক SepPrefix = is_separator(Prefix[I], style);
      যদি (SepPath != SepPrefix)
        প্রত্যাগম অসত্য;
      যদি (!SepPath && toLower(Path[I]) != toLower(Prefix[I]))
        প্রত্যাগম অসত্য;
    }
    প্রত্যাগম সত্য;
  }
  প্রত্যাগম Path.starts_with(Prefix);
}

তার্কিক replace_path_prefix(SmallVectorImpl<বর্ণ> &Path, StringRef OldPrefix,
                         StringRef NewPrefix, Style style) {
  যদি (OldPrefix.empty() && NewPrefix.empty())
    প্রত্যাগম অসত্য;

  StringRef OrigPath(Path.begin(), Path.size());
  যদি (!starts_with(OrigPath, OldPrefix, style))
    প্রত্যাগম অসত্য;

  // If prefixes have the same size we can simply copy the new one over.
  যদি (OldPrefix.size() == NewPrefix.size()) {
    llvm::copy(NewPrefix, Path.begin());
    প্রত্যাগম সত্য;
  }

  StringRef RelPath = OrigPath.substr(OldPrefix.size());
  SmallString<256> NewPath;
  (Twine(NewPrefix) + RelPath).toVector(NewPath);
  Path.swap(NewPath);
  প্রত্যাগম সত্য;
}

শূন্য native(ধ্রুব Twine &path, SmallVectorImpl<বর্ণ> &result, Style style) {
  প্রতিজ্ঞা((!path.isSingleStringRef() ||
          path.getSingleStringRef().data() != result.data()) &&
         "path and result are not allowed to overlap!");
  // Clear result.
  result.clear();
  path.toVector(result);
  native(result, style);
}

std::string native(ধ্রুব Twine &path, Style style) {
  SmallString<128> Result;
  native(path, Result, style);
  প্রত্যাগম std::string(Result);
}

শূন্য native(SmallVectorImpl<বর্ণ> &Path, Style style) {
  যদি (Path.empty())
    প্রত্যাগম;
  যদি (is_style_windows(style)) {
    প্রতি (বর্ণ &Ch : Path)
      যদি (is_separator(Ch, style))
        Ch = preferred_separator(style);
    যদি (Path[0] == '~' && (Path.size() == 1 || is_separator(Path[1], style))) {
      SmallString<128> PathHome;
      home_directory(PathHome);
      PathHome.append(Path.begin() + 1, Path.end());
      Path = std::move(PathHome);
    }
  } অন্যথা {
    llvm::replace(Path, '\\', '/');
  }
}

std::string convert_to_slash(StringRef path, Style style) {
  যদি (is_style_posix(style))
    প্রত্যাগম std::string(path);

  std::string s = path.str();
  llvm::replace(s, '\\', '/');
  প্রত্যাগম s;
}

StringRef filename(StringRef path, Style style) { প্রত্যাগম *rbegin(path, style); }

StringRef stem(StringRef path, Style style) {
  StringRef fname = filename(path, style);
  size_t pos = fname.find_last_of('.');
  যদি (pos == StringRef::npos)
    প্রত্যাগম fname;
  যদি ((fname.size() == 1 && fname == ".") ||
      (fname.size() == 2 && fname == ".."))
    প্রত্যাগম fname;
  প্রত্যাগম fname.substr(0, pos);
}

StringRef extension(StringRef path, Style style) {
  StringRef fname = filename(path, style);
  size_t pos = fname.find_last_of('.');
  যদি (pos == StringRef::npos)
    প্রত্যাগম StringRef();
  যদি ((fname.size() == 1 && fname == ".") ||
      (fname.size() == 2 && fname == ".."))
    প্রত্যাগম StringRef();
  প্রত্যাগম fname.substr(pos);
}

তার্কিক is_separator(বর্ণ মান_2, Style style) {
  যদি (মান_2 == '/')
    প্রত্যাগম সত্য;
  যদি (is_style_windows(style))
    প্রত্যাগম মান_2 == '\\';
  প্রত্যাগম অসত্য;
}

StringRef get_separator(Style style) {
  যদি (real_style(style) == Style::windows)
    প্রত্যাগম "\\";
  প্রত্যাগম "/";
}

তার্কিক has_মূল_name(ধ্রুব Twine &path, Style style) {
  SmallString<128> path_storage;
  StringRef p = path.toStringRef(path_storage);

  প্রত্যাগম !মূল_name(p, style).empty();
}

তার্কিক has_মূল_directory(ধ্রুব Twine &path, Style style) {
  SmallString<128> path_storage;
  StringRef p = path.toStringRef(path_storage);

  প্রত্যাগম !মূল_directory(p, style).empty();
}

তার্কিক has_মূল_path(ধ্রুব Twine &path, Style style) {
  SmallString<128> path_storage;
  StringRef p = path.toStringRef(path_storage);

  প্রত্যাগম !মূল_path(p, style).empty();
}

তার্কিক has_relative_path(ধ্রুব Twine &path, Style style) {
  SmallString<128> path_storage;
  StringRef p = path.toStringRef(path_storage);

  প্রত্যাগম !relative_path(p, style).empty();
}

তার্কিক has_filename(ধ্রুব Twine &path, Style style) {
  SmallString<128> path_storage;
  StringRef p = path.toStringRef(path_storage);

  প্রত্যাগম !filename(p, style).empty();
}

তার্কিক has_parent_path(ধ্রুব Twine &path, Style style) {
  SmallString<128> path_storage;
  StringRef p = path.toStringRef(path_storage);

  প্রত্যাগম !parent_path(p, style).empty();
}

তার্কিক has_stem(ধ্রুব Twine &path, Style style) {
  SmallString<128> path_storage;
  StringRef p = path.toStringRef(path_storage);

  প্রত্যাগম !stem(p, style).empty();
}

তার্কিক has_extension(ধ্রুব Twine &path, Style style) {
  SmallString<128> path_storage;
  StringRef p = path.toStringRef(path_storage);

  প্রত্যাগম !extension(p, style).empty();
}

তার্কিক is_absolute(ধ্রুব Twine &path, Style style) {
  SmallString<128> path_storage;
  StringRef p = path.toStringRef(path_storage);

  তার্কিক মূল_Dir = has_মূল_directory(p, style);
  তার্কিক মূল_Name = is_style_posix(style) || has_মূল_name(p, style);

  প্রত্যাগম মূল_Dir && মূল_Name;
}

তার্কিক is_absolute_gnu(ধ্রুব Twine &path, Style style) {
  SmallString<128> path_storage;
  StringRef p = path.toStringRef(path_storage);

  // Handle '/' which is absolute for both Windows and POSIX systems.
  // Handle '\\' on Windows.
  যদি (!p.empty() && is_separator(p.front(), style))
    প্রত্যাগম সত্য;

  যদি (is_style_windows(style)) {
    // Handle drive letter pattern (a character followed by ':') on Windows.
    যদি (p.size() >= 2 && (p[0] && p[1] == ':'))
      প্রত্যাগম সত্য;
  }

  প্রত্যাগম অসত্য;
}

তার্কিক is_relative(ধ্রুব Twine &path, Style style) {
  প্রত্যাগম !is_absolute(path, style);
}

শূন্য make_absolute(ধ্রুব Twine &current_directory,
                   SmallVectorImpl<বর্ণ> &path) {
  StringRef p(path.data(), path.size());

  তার্কিক মূল_Directory = has_মূল_directory(p);
  তার্কিক মূল_Name = has_মূল_name(p);

  // Already absolute.
  যদি ((মূল_Name || is_style_posix(Style::native)) && মূল_Directory)
    প্রত্যাগম;

  // All the following conditions will need the current directory.
  SmallString<128> current_dir;
  current_directory.toVector(current_dir);

  // Relative path. Prepend the current directory.
  যদি (!মূল_Name && !মূল_Directory) {
    // Append path to the current directory.
    append(current_dir, p);
    // Set path to the result.
    path.swap(current_dir);
    প্রত্যাগম;
  }

  যদি (!মূল_Name && মূল_Directory) {
    StringRef cdrn = মূল_name(current_dir);
    SmallString<128> cur_Dir_মূল_Name(cdrn.begin(), cdrn.end());
    append(cur_Dir_মূল_Name, p);
    // Set path to the result.
    path.swap(cur_Dir_মূল_Name);
    প্রত্যাগম;
  }

  যদি (মূল_Name && !মূল_Directory) {
    StringRef p_মূল_Name = মূল_name(p);
    StringRef b_মূল_Directory = মূল_directory(current_dir);
    StringRef bRelativePath = relative_path(current_dir);
    StringRef pRelativePath = relative_path(p);

    SmallString<128> res;
    append(res, p_মূল_Name, b_মূল_Directory, bRelativePath, pRelativePath);
    path.swap(res);
    প্রত্যাগম;
  }

  llvm_unreachable("All rootName and rootDirectory combinations should have "
                   "occurred above!");
}

StringRef remove_leading_dotslash(StringRef Path, Style style) {
  // Remove leading "./" (or ".//" or "././" etc.)
  যাবত্ (Path.size() > 2 && Path[0] == '.' && is_separator(Path[1], style)) {
    Path = Path.substr(2);
    যাবত্ (Path.size() > 0 && is_separator(Path[0], style))
      Path = Path.substr(1);
  }
  প্রত্যাগম Path;
}

তার্কিক remove_dots(SmallVectorImpl<বর্ণ> &the_path, তার্কিক remove_dot_dot,
                 Style style) {
  style = real_style(style);
  StringRef remaining(the_path.data(), the_path.size());
  তার্কিক needs_change = অসত্য;
  SmallVector<StringRef, 16> components;

  // Consume the root path, if present.
  StringRef মূল_3 = path::মূল_path(remaining, style);
  তার্কিক absolute = !মূল_3.empty();
  যদি (absolute)
    remaining = remaining.drop_front(মূল_3.size());

  // Loop over path components manually. This makes it easier to detect
  // non-preferred slashes and double separators that must be canonicalized.
  যাবত্ (!remaining.empty()) {
    size_t next_slash = remaining.find_first_of(separators(style));
    যদি (next_slash == StringRef::npos)
      next_slash = remaining.size();
    StringRef component = remaining.take_front(next_slash);
    remaining = remaining.drop_front(next_slash);

    // Eat the slash, and check if it is the preferred separator.
    যদি (!remaining.empty()) {
      needs_change |= remaining.front() != preferred_separator(style);
      remaining = remaining.drop_front();
      // The path needs to be rewritten if it has a trailing slash.
      // FIXME: This is emergent behavior that could be removed.
      needs_change |= remaining.empty();
    }

    // Check for path traversal components or double separators.
    যদি (component.empty() || component == ".") {
      needs_change = সত্য;
    } অন্যথা যদি (remove_dot_dot && component == "..") {
      needs_change = সত্য;
      // Do not allow ".." to remove the root component. If this is the
      // beginning of a relative path, keep the ".." component.
      যদি (!components.empty() && components.back() != "..") {
        components.বের_করো_back();
      } অন্যথা যদি (!absolute) {
        components.ঠেলো_back(component);
      }
    } অন্যথা {
      components.ঠেলো_back(component);
    }
  }

  SmallString<256> buffer = মূল_3;
  // "root" could be "/", which may need to be translated into "\".
  make_preferred(buffer, style);
  needs_change |= মূল_3 != buffer;

  // Avoid rewriting the path unless we have to.
  যদি (!needs_change)
    প্রত্যাগম অসত্য;

  যদি (!components.empty()) {
    buffer += components[0];
    প্রতি (StringRef C : ArrayRef(components).drop_front()) {
      buffer += preferred_separator(style);
      buffer += C;
    }
  }
  the_path.swap(buffer);
  প্রত্যাগম সত্য;
}

} // end namespace path

নামাকাশ fs {

std::error_code getUniqueID(ধ্রুব Twine Path, UniqueID &Result) {
  sandbox::violationIfEnabled();

  file_status Status;
  std::error_code EC = status(Path, Status);
  যদি (EC)
    প্রত্যাগম EC;
  Result = Status.getUniqueID();
  প্রত্যাগম std::error_code();
}

শূন্য createUniquePath(ধ্রুব Twine &Model, SmallVectorImpl<বর্ণ> &ResultPath,
                      তার্কিক MakeAbsolute) {
  SmallString<128> ModelStorage;
  Model.toVector(ModelStorage);

  প্রতিজ্ঞা(llvm::is_contained(ModelStorage, '%') &&
         "createUniquePath: Model must contain at least one '%'");

  যদি (MakeAbsolute) {
    // Make model absolute by prepending a temp directory if it's not already.
    যদি (!sys::path::is_absolute(Twine(ModelStorage))) {
      SmallString<128> TDir;
      sys::path::system_temp_directory(সত্য, TDir);
      sys::path::append(TDir, Twine(ModelStorage));
      ModelStorage.swap(TDir);
    }
  }

  ResultPath = ModelStorage;
  ResultPath.ঠেলো_back(0);
  ResultPath.বের_করো_back();

  // Replace '%' with random chars.
  প্রতি (unsigned i = 0, e = ModelStorage.size(); i != e; ++i) {
    যদি (ModelStorage[i] == '%')
      ResultPath[i] = "0123456789abcdef"[sys::Process::Get_Random_সংখ্যা() & 15];
  }
}

std::error_code createUniqueFile(ধ্রুব Twine &Model, পূর্ণাঙ্ক &ResultFd,
                                 SmallVectorImpl<বর্ণ> &ResultPath,
                                 OpenFlags Flags, unsigned Mode) {
  প্রত্যাগম createUniqueEntity(Model, ResultFd, ResultPath, অসত্য, FS_File, Flags,
                            Mode);
}

std::error_code createUniqueFile(ধ্রুব Twine &Model,
                                 SmallVectorImpl<বর্ণ> &ResultPath,
                                 unsigned Mode) {
  পূর্ণাঙ্ক FD;
  auto EC = createUniqueFile(Model, FD, ResultPath, OF_None, Mode);
  যদি (EC)
    প্রত্যাগম EC;
  // FD is only needed to avoid race conditions. Close it right away.
  close(FD);
  প্রত্যাগম EC;
}

স্থির std::error_code
createTemporaryFile(ধ্রুব Twine &Model, পূর্ণাঙ্ক &ResultFD,
                    llvm::SmallVectorImpl<বর্ণ> &ResultPath, FSEntity Type,
                    sys::fs::OpenFlags Flags = sys::fs::OF_None) {
  // Any *temporary* file is assumed to be a compiler-internal output, not
  // a formal one.
  auto BypassSandbox = sys::sandbox::scopedDisable();

  SmallString<128> Storage;
  StringRef P = Model.toNullTerminatedStringRef(Storage);
  প্রতিজ্ঞা(P.find_first_of(separators(Style::native)) == StringRef::npos &&
         "Model must be a simple filename.");
  // Use P.begin() so that createUniqueEntity doesn't need to recreate Storage.
  প্রত্যাগম createUniqueEntity(P.begin(), ResultFD, ResultPath, সত্য, Type, Flags,
                            all_read | all_write);
}

স্থির std::error_code
createTemporaryFile(ধ্রুব Twine &Prefix, StringRef Suffix, পূর্ণাঙ্ক &ResultFD,
                    llvm::SmallVectorImpl<বর্ণ> &ResultPath, FSEntity Type,
                    sys::fs::OpenFlags Flags = sys::fs::OF_None) {
  ধ্রুব বর্ণ *মধ্য = Suffix.empty() ? "-%%%%%%" : "-%%%%%%.";
  প্রত্যাগম createTemporaryFile(Prefix + মধ্য + Suffix, ResultFD, ResultPath,
                             Type, Flags);
}

std::error_code createTemporaryFile(ধ্রুব Twine &Prefix, StringRef Suffix,
                                    পূর্ণাঙ্ক &ResultFD,
                                    SmallVectorImpl<বর্ণ> &ResultPath,
                                    sys::fs::OpenFlags Flags) {
  প্রত্যাগম createTemporaryFile(Prefix, Suffix, ResultFD, ResultPath, FS_File,
                             Flags);
}

std::error_code createTemporaryFile(ধ্রুব Twine &Prefix, StringRef Suffix,
                                    SmallVectorImpl<বর্ণ> &ResultPath,
                                    sys::fs::OpenFlags Flags) {
  পূর্ণাঙ্ক FD;
  auto EC = createTemporaryFile(Prefix, Suffix, FD, ResultPath, Flags);
  যদি (EC)
    প্রত্যাগম EC;
  // FD is only needed to avoid race conditions. Close it right away.
  close(FD);
  প্রত্যাগম EC;
}

// This is a mkdtemp with a different pattern. We use createUniqueEntity mostly
// for consistency. We should try using mkdtemp.
std::error_code createUniqueDirectory(ধ্রুব Twine &Prefix,
                                      SmallVectorImpl<বর্ণ> &ResultPath) {
  পূর্ণাঙ্ক Dummy;
  প্রত্যাগম createUniqueEntity(Prefix + "-%%%%%%", Dummy, ResultPath, সত্য,
                            FS_Dir);
}

std::error_code
getPotentiallyUniqueFileName(ধ্রুব Twine &Model,
                             SmallVectorImpl<বর্ণ> &ResultPath) {
  পূর্ণাঙ্ক Dummy;
  প্রত্যাগম createUniqueEntity(Model, Dummy, ResultPath, অসত্য, FS_Name);
}

std::error_code
getPotentiallyUniqueTempFileName(ধ্রুব Twine &Prefix, StringRef Suffix,
                                 SmallVectorImpl<বর্ণ> &ResultPath) {
  পূর্ণাঙ্ক Dummy;
  প্রত্যাগম createTemporaryFile(Prefix, Suffix, Dummy, ResultPath, FS_Name);
}

std::error_code make_absolute(SmallVectorImpl<বর্ণ> &path) {
  sandbox::violationIfEnabled();

  যদি (path::is_absolute(path))
    প্রত্যাগম {};

  SmallString<128> current_dir;
  যদি (std::error_code ec = current_path(current_dir))
    প্রত্যাগম ec;

  path::make_absolute(current_dir, path);
  প্রত্যাগম {};
}

std::error_code create_directories(ধ্রুব Twine &Path, তার্কিক IgnoreExisting,
                                   perms Perms) {
  SmallString<128> PathStorage;
  StringRef P = Path.toStringRef(PathStorage);

  // Be optimistic and try to create the directory
  std::error_code EC = create_directory(P, IgnoreExisting, Perms);
  // If we succeeded, or had any error other than the parent not existing, just
  // return it.
  যদি (EC != errc::no_such_file_or_directory)
    প্রত্যাগম EC;

  // We failed because of a no_such_file_or_directory, try to create the
  // parent.
  StringRef Parent = path::parent_path(P);
  যদি (Parent.empty())
    প্রত্যাগম EC;

  যদি ((EC = create_directories(Parent, IgnoreExisting, Perms)))
      প্রত্যাগম EC;

  প্রত্যাগম create_directory(P, IgnoreExisting, Perms);
}

স্থির std::error_code copy_file_internal(পূর্ণাঙ্ক ReadFD, পূর্ণাঙ্ক WriteFD) {
  ধ্রুব size_t BufSize = 4096;
  বর্ণ *Buf = নূতন বর্ণ[BufSize];
  পূর্ণাঙ্ক BytesRead = 0, BytesWritten = 0;
  প্রতি (;;) {
    BytesRead = পঠ(ReadFD, Buf, BufSize);
    যদি (BytesRead <= 0)
      ভঙ্গ;
    যাবত্ (BytesRead) {
      BytesWritten = লিখ(WriteFD, Buf, BytesRead);
      যদি (BytesWritten < 0)
        ভঙ্গ;
      BytesRead -= BytesWritten;
    }
    যদি (BytesWritten < 0)
      ভঙ্গ;
  }
  delete[] Buf;

  যদি (BytesRead < 0 || BytesWritten < 0)
    প্রত্যাগম errnoAsErrorCode();
  প্রত্যাগম std::error_code();
}

#ifndef __APPLE__
std::error_code copy_file(ধ্রুব Twine &From, ধ্রুব Twine &To) {
  পূর্ণাঙ্ক ReadFD, WriteFD;
  যদি (std::error_code EC = openFileForRead(From, ReadFD, OF_None))
    প্রত্যাগম EC;
  যদি (std::error_code EC =
          openFileForWrite(To, WriteFD, CD_CreateAlways, OF_None)) {
    close(ReadFD);
    প্রত্যাগম EC;
  }

  std::error_code EC = copy_file_internal(ReadFD, WriteFD);

  close(ReadFD);
  close(WriteFD);

  প্রত্যাগম EC;
}
#endif

std::error_code copy_file(ধ্রুব Twine &From, পূর্ণাঙ্ক ToFD) {
  পূর্ণাঙ্ক ReadFD;
  যদি (std::error_code EC = openFileForRead(From, ReadFD, OF_None))
    প্রত্যাগম EC;

  std::error_code EC = copy_file_internal(ReadFD, ToFD);

  close(ReadFD);

  প্রত্যাগম EC;
}

ErrorOr<MD5::MD5Result> md5_contents(পূর্ণাঙ্ক FD) {
  sandbox::violationIfEnabled();

  MD5 Hash;

  constexpr size_t BufSize = 4096;
  std::vector<uint8_t> Buf(BufSize);
  পূর্ণাঙ্ক BytesRead = 0;
  প্রতি (;;) {
    BytesRead = পঠ(FD, Buf.data(), BufSize);
    যদি (BytesRead <= 0)
      ভঙ্গ;
    Hash.update(ArrayRef(Buf.data(), BytesRead));
  }

  যদি (BytesRead < 0)
    প্রত্যাগম errnoAsErrorCode();
  MD5::MD5Result Result;
  Hash.final(Result);
  প্রত্যাগম Result;
}

ErrorOr<MD5::MD5Result> md5_contents(ধ্রুব Twine &Path) {
  sandbox::violationIfEnabled();

  পূর্ণাঙ্ক FD;
  যদি (auto EC = openFileForRead(Path, FD, OF_None))
    প্রত্যাগম EC;

  auto Result = md5_contents(FD);
  close(FD);
  প্রত্যাগম Result;
}

তার্কিক exists(ধ্রুব basic_file_status &status) {
  প্রত্যাগম status_known(status) && status.type() != file_type::file_not_found;
}

তার্কিক status_known(ধ্রুব basic_file_status &s) {
  প্রত্যাগম s.type() != file_type::status_error;
}

file_type get_file_type(ধ্রুব Twine &Path, তার্কিক Follow) {
  file_status st;
  যদি (status(Path, st, Follow))
    প্রত্যাগম file_type::status_error;
  প্রত্যাগম st.type();
}

তার্কিক is_directory(ধ্রুব basic_file_status &status) {
  প্রত্যাগম status.type() == file_type::directory_file;
}

std::error_code is_directory(ধ্রুব Twine &path, তার্কিক &result) {
  sandbox::violationIfEnabled();

  file_status st;
  যদি (std::error_code ec = status(path, st))
    প্রত্যাগম ec;
  result = is_directory(st);
  প্রত্যাগম std::error_code();
}

তার্কিক is_regular_file(ধ্রুব basic_file_status &status) {
  প্রত্যাগম status.type() == file_type::regular_file;
}

std::error_code is_regular_file(ধ্রুব Twine &path, তার্কিক &result) {
  sandbox::violationIfEnabled();

  file_status st;
  যদি (std::error_code ec = status(path, st))
    প্রত্যাগম ec;
  result = is_regular_file(st);
  প্রত্যাগম std::error_code();
}

তার্কিক is_symlink_file(ধ্রুব basic_file_status &status) {
  প্রত্যাগম status.type() == file_type::symlink_file;
}

std::error_code is_symlink_file(ধ্রুব Twine &path, তার্কিক &result) {
  sandbox::violationIfEnabled();

  file_status st;
  যদি (std::error_code ec = status(path, st, অসত্য))
    প্রত্যাগম ec;
  result = is_symlink_file(st);
  প্রত্যাগম std::error_code();
}

তার্কিক is_other(ধ্রুব basic_file_status &status) {
  প্রত্যাগম exists(status) &&
         !is_regular_file(status) &&
         !is_directory(status);
}

std::error_code is_other(ধ্রুব Twine &Path, তার্কিক &Result) {
  sandbox::violationIfEnabled();

  file_status FileStatus;
  যদি (std::error_code EC = status(Path, FileStatus))
    প্রত্যাগম EC;
  Result = is_other(FileStatus);
  প্রত্যাগম std::error_code();
}

শূন্য directory_entry::replace_filename(ধ্রুব Twine &Filename, file_type Type,
                                       basic_file_status Status) {
  SmallString<128> PathStr = path::parent_path(Path);
  path::append(PathStr, Filename);
  এতত্->Path = std::string(PathStr);
  এতত্->Type = Type;
  এতত্->Status = Status;
}

ErrorOr<perms> getPermissions(ধ্রুব Twine &Path) {
  sandbox::violationIfEnabled();

  file_status Status;
  যদি (std::error_code EC = status(Path, Status))
    প্রত্যাগম EC;

  প্রত্যাগম Status.permissions();
}

std::error_code setLastAccessAndModificationTime(ধ্রুব Twine &Path,
                                                 TimePoint<> AccessTime,
                                                 TimePoint<> ModificationTime) {
  পূর্ণাঙ্ক FD;
  যদি (std::error_code EC = openFile(Path, FD, CD_OpenExisting, FA_Read,
                                    OF_UpdateAttributes | OF_OpenDirectory))
    প্রত্যাগম EC;
  std::error_code EC =
      setLastAccessAndModificationTime(FD, AccessTime, ModificationTime);
  Process::SafelyCloseFileDescriptor(FD);
  প্রত্যাগম EC;
}

size_t mapped_file_region::size() ধ্রুব {
  প্রতিজ্ঞা(Mapping && "Mapping failed but used anyway!");
  প্রত্যাগম Size;
}

বর্ণ *mapped_file_region::data() ধ্রুব {
  প্রতিজ্ঞা(Mapping && "Mapping failed but used anyway!");
  প্রত্যাগম reinterpret_cast<বর্ণ *>(Mapping);
}

ধ্রুব বর্ণ *mapped_file_region::const_data() ধ্রুব {
  প্রতিজ্ঞা(Mapping && "Mapping failed but used anyway!");
  প্রত্যাগম reinterpret_cast<ধ্রুব বর্ণ *>(Mapping);
}

Error readNativeFileToEOF(file_t FileHandle, SmallVectorImpl<বর্ণ> &Buffer,
                          ssize_t ChunkSize) {
  sandbox::violationIfEnabled();

  // Install a handler to truncate the buffer to the correct size on exit.
  size_t Size = Buffer.size();
  llvm::scope_exit TruncateOnExit([&]() { Buffer.truncate(Size); });

  // Read into Buffer until we hit EOF.
  প্রতি (;;) {
    Buffer.resize_for_overwrite(Size + ChunkSize);
    Expected<size_t> ReadBytes = readNativeFile(
        FileHandle, MutableArrayRef(Buffer.begin() + Size, ChunkSize));
    যদি (!ReadBytes)
      প্রত্যাগম ReadBytes.takeError();
    যদি (*ReadBytes == 0)
      প্রত্যাগম Error::success();
    Size += *ReadBytes;
  }
}

} // end namespace fs
} // end namespace sys
} // end namespace llvm

// Include the truly platform-specific parts.
#যদি defined(LLVM_ON_UNIX)
#অন্তর্ভাব "Unix/Path.inc"
#endif
#যদি defined(_WIN32)
#অন্তর্ভাব "Windows/Path.inc"
#endif

নামাকাশ llvm {
নামাকাশ sys {
নামাকাশ fs {

TempFile::TempFile(StringRef Name, পূর্ণাঙ্ক FD)
    : TmpName(std::string(Name)), FD(FD) {}
TempFile::TempFile(TempFile &&Other) { *এতত্ = std::move(Other); }
TempFile &TempFile::operator=(TempFile &&Other) {
  TmpName = std::move(Other.TmpName);
  FD = Other.FD;
  Other.Done = সত্য;
  Other.FD = -1;
#ifdef _WIN32
  RemoveOnClose = Other.RemoveOnClose;
  Other.RemoveOnClose = অসত্য;
#endif
  প্রত্যাগম *এতত্;
}

TempFile::~TempFile() { প্রতিজ্ঞা(Done); }

Error TempFile::discard() {
  Done = সত্য;
  যদি (FD != -1 && close(FD) == -1) {
    std::error_code EC = errnoAsErrorCode();
    প্রত্যাগম errorCodeToError(EC);
  }
  FD = -1;

#ifdef _WIN32
  // On Windows, closing will remove the file, if we set the delete
  // disposition. If not, remove it manually.
  তার্কিক Remove = RemoveOnClose;
#অন্যথা
  // Always try to remove the file.
  তার্কিক Remove = সত্য;
#endif
  std::error_code RemoveEC;
  যদি (Remove && !TmpName.empty()) {
    RemoveEC = fs::remove(TmpName);
    sys::DontRemoveFileOnSignal(TmpName);
    যদি (!RemoveEC)
      TmpName = "";
  } অন্যথা {
    TmpName = "";
  }
  প্রত্যাগম errorCodeToError(RemoveEC);
}

Error TempFile::keep(ধ্রুব Twine &Name) {
  প্রতিজ্ঞা(!Done);
  Done = সত্য;
  // Always try to close and rename.
#ifdef _WIN32
  // If we can't cancel the delete don't rename.
  auto H = reinterpret_cast<HANDLE>(_get_osfhandle(FD));
  std::error_code RenameEC =
      RemoveOnClose ? std::error_code() : setDeleteDisposition(H, অসত্য);
  তার্কিক ShouldDelete = অসত্য;
  যদি (!RenameEC) {
    RenameEC = rename_handle(H, Name);
    // If rename failed because it's cross-device, copy instead
    যদি (RenameEC ==
      std::error_code(ERROR_NOT_SAME_DEVICE, std::system_category())) {
      RenameEC = copy_file(TmpName, Name);
      ShouldDelete = সত্য;
    }
  }

  // If we can't rename or copy, discard the temporary file.
  যদি (RenameEC)
    ShouldDelete = সত্য;
  যদি (ShouldDelete) {
    যদি (!RemoveOnClose)
      setDeleteDisposition(H, সত্য);
    অন্যথা
      remove(TmpName);
  }
#অন্যথা
  std::error_code RenameEC = fs::rename(TmpName, Name);
  যদি (RenameEC) {
    // If we can't rename, try to copy to work around cross-device link issues.
    RenameEC = sys::fs::copy_file(TmpName, Name);
    // If we can't rename or copy, discard the temporary file.
    যদি (RenameEC)
      remove(TmpName);
  }
#endif
  sys::DontRemoveFileOnSignal(TmpName);

  যদি (!RenameEC)
    TmpName = "";

  যদি (close(FD) == -1)
    প্রত্যাগম errorCodeToError(errnoAsErrorCode());
  FD = -1;

  প্রত্যাগম errorCodeToError(RenameEC);
}

Error TempFile::keep() {
  প্রতিজ্ঞা(!Done);
  Done = সত্য;

#ifdef _WIN32
  auto H = reinterpret_cast<HANDLE>(_get_osfhandle(FD));
  যদি (std::error_code EC = setDeleteDisposition(H, অসত্য))
    প্রত্যাগম errorCodeToError(EC);
#endif
  sys::DontRemoveFileOnSignal(TmpName);

  TmpName = "";

  যদি (close(FD) == -1)
    প্রত্যাগম errorCodeToError(errnoAsErrorCode());
  FD = -1;

  প্রত্যাগম Error::success();
}

Expected<TempFile> TempFile::create(ধ্রুব Twine &Model, unsigned Mode,
                                    OpenFlags ExtraFlags) {
  পূর্ণাঙ্ক FD;
  SmallString<128> ResultPath;
  যদি (std::error_code EC =
          createUniqueFile(Model, FD, ResultPath, OF_Delete | ExtraFlags, Mode))
    প্রত্যাগম errorCodeToError(EC);

  TempFile Ret(ResultPath, FD);
#ifdef _WIN32
  auto H = reinterpret_cast<HANDLE>(_get_osfhandle(FD));
  তার্কিক SetSignalHandler = অসত্য;
  যদি (std::error_code EC = setDeleteDisposition(H, সত্য)) {
    Ret.RemoveOnClose = সত্য;
    SetSignalHandler = সত্য;
  }
#অন্যথা
  তার্কিক SetSignalHandler = সত্য;
#endif
  যদি (SetSignalHandler && sys::RemoveFileOnSignal(ResultPath)) {
    // Make sure we delete the file when RemoveFileOnSignal fails.
    consumeError(Ret.discard());
    std::error_code EC(errc::operation_not_permitted);
    প্রত্যাগম errorCodeToError(EC);
  }
  প্রত্যাগম std::move(Ret);
}
} // namespace fs

} // namespace sys
} // namespace llvm
