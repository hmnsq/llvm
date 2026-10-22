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

  অন্তঃরেখ যৌক্তিক prefer_forward_slash() {
    স্থির যৌক্তিক prefer = []() {
      যদি (std::optional<std::string> Env =
              sys::Process::GetEnv("LLVM_WINDOWS_PREFER_FORWARD_SLASH"))
        ফেরত *Env == "1";
      ফেরত static_cast<যৌক্তিক>(LLVM_WINDOWS_PREFER_FORWARD_SLASH);
    }();
    ফেরত prefer;
  }

  অন্তঃরেখ Style real_style(Style style) {
    যদি (style != Style::native)
      ফেরত style;
    যদি (is_style_posix(style))
      ফেরত Style::posix;
    ফেরত prefer_forward_slash() ? Style::windows_slash
                                  : Style::windows_backslash;
  }

  অন্তঃরেখ ধ্রুবক অক্ষর *separators(Style style) {
    যদি (is_style_windows(style))
      ফেরত "\\/";
    ফেরত "/";
  }

  অন্তঃরেখ অক্ষর preferred_separator(Style style) {
    যদি (real_style(style) == Style::windows)
      ফেরত '\\';
    ফেরত '/';
  }

  StringRef find_first_component(StringRef path, Style style) {
    // Look for this first component in the following order.
    // * empty (in this case we return an empty string)
    // * either C: or {//,\\}net.
    // * {/,\}
    // * {file,directory}name

    যদি (path.empty())
      ফেরত path;

    যদি (is_style_windows(style)) {
      // C:
      যদি (path.আকার_3() >= 2 &&
          std::isalpha(static_cast<unsigned অক্ষর>(path[0])) && path[1] == ':')
        ফেরত path.substr(0, 2);
    }

    // //net
    যদি ((path.আকার_3() > 2) && is_separator(path[0], style) &&
        path[0] == path[1] && !is_separator(path[2], style)) {
      // Find the next directory separator.
      size_t end = path.find_first_of(separators(style), 2);
      ফেরত path.substr(0, end);
    }

    // {/,\}
    যদি (is_separator(path[0], style))
      ফেরত path.substr(0, 1);

    // * {file,directory}name
    size_t end = path.find_first_of(separators(style));
    ফেরত path.substr(0, end);
  }

  // Returns the first character of the filename in str. For paths ending in
  // '/', it returns the position of the '/'.
  size_t filename_pos(StringRef str, Style style) {
    যদি (str.আকার_3() > 0 && is_separator(str[str.আকার_3() - 1], style))
      ফেরত str.আকার_3() - 1;

    size_t pos = str.find_last_of(separators(style), str.আকার_3() - 1);

    যদি (is_style_windows(style)) {
      যদি (pos == StringRef::npos)
        pos = str.find_last_of(':', str.আকার_3() - 1);
    }

    যদি (pos == StringRef::npos || (pos == 1 && is_separator(str[0], style)))
      ফেরত 0;

    ফেরত pos + 1;
  }

  // Returns the position of the root directory in str. If there is no root
  // directory in str, it returns StringRef::npos.
  size_t মূল_dir_start(StringRef str, Style style) {
    // case "c:/"
    যদি (is_style_windows(style)) {
      যদি (str.আকার_3() > 2 && str[1] == ':' && is_separator(str[2], style))
        ফেরত 2;
    }

    // case "//net"
    যদি (str.আকার_3() > 3 && is_separator(str[0], style) && str[0] == str[1] &&
        !is_separator(str[2], style)) {
      ফেরত str.find_first_of(separators(style), 2);
    }

    // case "/"
    যদি (str.আকার_3() > 0 && is_separator(str[0], style))
      ফেরত 0;

    ফেরত StringRef::npos;
  }

  // Returns the position past the end of the "parent path" of path. The parent
  // path will not end in '/', unless the parent is the root directory. If the
  // path has no parent, 0 is returned.
  size_t parent_path_end(StringRef path, Style style) {
    size_t end_pos = filename_pos(path, style);

    যৌক্তিক filename_was_sep =
        path.আকার_3() > 0 && is_separator(path[end_pos], style);

    // Skip separators until we reach root dir (or the start of the string).
    size_t মূল_dir_pos = মূল_dir_start(path, style);
    যখন (end_pos > 0 &&
           (মূল_dir_pos == StringRef::npos || end_pos > মূল_dir_pos) &&
           is_separator(path[end_pos - 1], style))
      --end_pos;

    যদি (end_pos == মূল_dir_pos && !filename_was_sep) {
      // We've reached the root dir and the input path was *not* ending in a
      // sequence of slashes. Include the root dir in the parent path.
      ফেরত মূল_dir_pos + 1;
    }

    // Otherwise, just include before the last slash.
    ফেরত end_pos;
  }
} // end unnamed namespace

গণনা FSEntity {
  FS_Dir,
  FS_File,
  FS_Name
};

স্থির std::error_code
createUniqueEntity(ধ্রুবক Twine &Model, পূর্ণসংখ্যা &ফলাফল_FD,
                   SmallVectorImpl<অক্ষর> &ফলাফল_Path, যৌক্তিক MakeAbsolute,
                   FSEntity Type, sys::fs::OpenFlags Flags = sys::fs::OF_None,
                   unsigned Mode = 0) {

  // Limit the number of attempts we make, so that we don't infinite loop. E.g.
  // "permission denied" could be for a specific file (so we retry with a
  // different name) or for the whole directory (retry would always fail).
  // Checking which is racy, so we try a number of times, then give up.
  std::error_code EC;
  জন্য (পূর্ণসংখ্যা Retries = 128; Retries > 0; --Retries) {
    sys::fs::createUniquePath(Model, ফলাফল_Path, MakeAbsolute);
    // Try to open + create the file.
    নির্বাচন (Type) {
    ক্ষেত্র FS_File: {
      EC = sys::fs::openFileForReadWrite(Twine(ফলাফল_Path.begin()), ফলাফল_FD,
                                         sys::fs::CD_CreateNew, Flags, Mode);
      যদি (EC) {
        // errc::permission_denied happens on Windows when we try to open a file
        // that has been marked for deletion.
        যদি (EC == errc::file_exists || EC == errc::permission_denied)
          চালু;
        ফেরত EC;
      }

      ফেরত std::error_code();
    }

    ক্ষেত্র FS_Name: {
      EC = sys::fs::access(ফলাফল_Path.begin(), sys::fs::AccessMode::Exist);
      যদি (EC == errc::no_such_file_or_directory)
        ফেরত std::error_code();
      যদি (EC)
        ফেরত EC;
      চালু;
    }

    ক্ষেত্র FS_Dir: {
      EC = sys::fs::create_directory(ফলাফল_Path.begin(), মিথ্যা);
      যদি (EC) {
        যদি (EC == errc::file_exists)
          চালু;
        ফেরত EC;
      }
      ফেরত std::error_code();
    }
    }
    llvm_unreachable("Invalid Type");
  }
  ফেরত EC;
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
  ফেরত i;
}

const_iterator end(StringRef path) {
  const_iterator i;
  i.Path      = path;
  i.Position  = path.আকার_3();
  ফেরত i;
}

const_iterator &const_iterator::operator++() {
  প্রতিজ্ঞা(Position < Path.আকার_3() && "Tried to increment past end!");

  // Increment Position to past the current component
  Position += Component.আকার_3();

  // Check for end.
  যদি (Position == Path.আকার_3()) {
    Component = StringRef();
    ফেরত *এই;
  }

  // Both POSIX and Windows treat paths that begin with exactly two separators
  // specially.
  যৌক্তিক was_net = Component.আকার_3() > 2 && is_separator(Component[0], S) &&
                 Component[1] == Component[0] && !is_separator(Component[2], S);

  // Handle separators.
  যদি (is_separator(Path[Position], S)) {
    // Root dir.
    যদি (was_net ||
        // c:/
        (is_style_windows(S) && Component.ends_with(":"))) {
      Component = Path.substr(Position, 1);
      ফেরত *এই;
    }

    // Skip extra separators.
    যখন (Position != Path.আকার_3() && is_separator(Path[Position], S)) {
      ++Position;
    }

    // Treat trailing '/' as a '.', unless it is the root dir.
    যদি (Position == Path.আকার_3() && Component != "/") {
      --Position;
      Component = ".";
      ফেরত *এই;
    }
  }

  // Find next component.
  size_t end_pos = Path.find_first_of(separators(S), Position);
  Component = Path.slice(Position, end_pos);

  ফেরত *এই;
}

যৌক্তিক const_iterator::operator==(ধ্রুবক const_iterator &RHS) ধ্রুবক {
  ফেরত Path.begin() == RHS.Path.begin() && Position == RHS.Position;
}

ptrdiff_t const_iterator::operator-(ধ্রুবক const_iterator &RHS) ধ্রুবক {
  ফেরত Position - RHS.Position;
}

reverse_iterator rbegin(StringRef Path, Style style) {
  reverse_iterator I;
  I.Path = Path;
  I.Position = Path.আকার_3();
  I.S = style;
  ++I;
  ফেরত I;
}

reverse_iterator rend(StringRef Path) {
  reverse_iterator I;
  I.Path = Path;
  I.Component = Path.substr(0, 0);
  I.Position = 0;
  ফেরত I;
}

reverse_iterator &reverse_iterator::operator++() {
  size_t মূল_dir_pos = মূল_dir_start(Path, S);

  // Skip separators unless it's the root directory.
  size_t end_pos = Position;
  যখন (end_pos > 0 && (end_pos - 1) != মূল_dir_pos &&
         is_separator(Path[end_pos - 1], S))
    --end_pos;

  // Treat trailing '/' as a '.', unless it is the root dir.
  যদি (Position == Path.আকার_3() && !Path.empty() &&
      is_separator(Path.back(), S) &&
      (মূল_dir_pos == StringRef::npos || end_pos - 1 > মূল_dir_pos)) {
    --Position;
    Component = ".";
    ফেরত *এই;
  }

  // Find next separator.
  size_t start_pos = filename_pos(Path.substr(0, end_pos), S);
  Component = Path.slice(start_pos, end_pos);
  Position = start_pos;
  ফেরত *এই;
}

যৌক্তিক reverse_iterator::operator==(ধ্রুবক reverse_iterator &RHS) ধ্রুবক {
  ফেরত Path.begin() == RHS.Path.begin() && Component == RHS.Component &&
         Position == RHS.Position;
}

ptrdiff_t reverse_iterator::operator-(ধ্রুবক reverse_iterator &RHS) ধ্রুবক {
  ফেরত Position - RHS.Position;
}

StringRef মূল_path(StringRef path, Style style) {
  const_iterator b = begin(path, style), pos = b, e = end(path);
  যদি (b != e) {
    যৌক্তিক has_net =
        b->আকার_3() > 2 && is_separator((*b)[0], style) && (*b)[1] == (*b)[0];
    যৌক্তিক has_drive = is_style_windows(style) && b->ends_with(":");

    যদি (has_net || has_drive) {
      যদি ((++pos != e) && is_separator((*pos)[0], style)) {
        // {C:/,//net/}, so get the first two components.
        ফেরত path.substr(0, b->আকার_3() + pos->আকার_3());
      }
      // just {C:,//net}, return the first component.
      ফেরত *b;
    }

    // POSIX style root directory.
    যদি (is_separator((*b)[0], style)) {
      ফেরত *b;
    }
  }

  ফেরত StringRef();
}

StringRef মূল_name(StringRef path, Style style) {
  const_iterator b = begin(path, style), e = end(path);
  যদি (b != e) {
    যৌক্তিক has_net =
        b->আকার_3() > 2 && is_separator((*b)[0], style) && (*b)[1] == (*b)[0];
    যৌক্তিক has_drive = is_style_windows(style) && b->ends_with(":");

    যদি (has_net || has_drive) {
      // just {C:,//net}, return the first component.
      ফেরত *b;
    }
  }

  // No path or no name.
  ফেরত StringRef();
}

StringRef মূল_directory(StringRef path, Style style) {
  const_iterator b = begin(path, style), pos = b, e = end(path);
  যদি (b != e) {
    যৌক্তিক has_net =
        b->আকার_3() > 2 && is_separator((*b)[0], style) && (*b)[1] == (*b)[0];
    যৌক্তিক has_drive = is_style_windows(style) && b->ends_with(":");

    যদি ((has_net || has_drive) &&
        // {C:,//net}, skip to the next component.
        (++pos != e) && is_separator((*pos)[0], style)) {
      ফেরত *pos;
    }

    // POSIX style root directory.
    যদি (!has_net && is_separator((*b)[0], style)) {
      ফেরত *b;
    }
  }

  // No path or no root.
  ফেরত StringRef();
}

StringRef relative_path(StringRef path, Style style) {
  StringRef মূল_3 = মূল_path(path, style);
  ফেরত path.substr(মূল_3.আকার_3());
}

শূন্য append(SmallVectorImpl<অক্ষর> &path, Style style, ধ্রুবক Twine &a,
            ধ্রুবক Twine &b, ধ্রুবক Twine &c, ধ্রুবক Twine &d) {
  SmallString<32> a_storage;
  SmallString<32> b_storage;
  SmallString<32> c_storage;
  SmallString<32> d_storage;

  SmallVector<StringRef, 4> components;
  যদি (!a.isTriviallyEmpty()) components.ঠেলো_back(a.toStringRef(a_storage));
  যদি (!b.isTriviallyEmpty()) components.ঠেলো_back(b.toStringRef(b_storage));
  যদি (!c.isTriviallyEmpty()) components.ঠেলো_back(c.toStringRef(c_storage));
  যদি (!d.isTriviallyEmpty()) components.ঠেলো_back(d.toStringRef(d_storage));

  জন্য (auto &component : components) {
    যৌক্তিক path_has_sep =
        !path.empty() && is_separator(path[path.আকার_3() - 1], style);
    যদি (path_has_sep) {
      // Strip separators from beginning of component.
      size_t loc = component.find_first_not_of(separators(style));
      StringRef c = component.substr(loc);

      // Append it.
      path.append(c.begin(), c.end());
      চালু;
    }

    যৌক্তিক component_has_sep =
        !component.empty() && is_separator(component[0], style);
    যদি (!component_has_sep &&
        !(path.empty() || has_মূল_name(component, style))) {
      // Add a separator.
      path.ঠেলো_back(preferred_separator(style));
    }

    path.append(component.begin(), component.end());
  }
}

শূন্য append(SmallVectorImpl<অক্ষর> &path, ধ্রুবক Twine &a, ধ্রুবক Twine &b,
            ধ্রুবক Twine &c, ধ্রুবক Twine &d) {
  append(path, Style::native, a, b, c, d);
}

শূন্য append(SmallVectorImpl<অক্ষর> &path, const_iterator begin,
            const_iterator end, Style style) {
  জন্য (; begin != end; ++begin)
    path::append(path, style, *begin);
}

StringRef parent_path(StringRef path, Style style) {
  size_t end_pos = parent_path_end(path, style);
  যদি (end_pos == StringRef::npos)
    ফেরত StringRef();
  ফেরত path.substr(0, end_pos);
}

শূন্য remove_filename(SmallVectorImpl<অক্ষর> &path, Style style) {
  size_t end_pos = parent_path_end(StringRef(path.begin(), path.আকার_3()), style);
  যদি (end_pos != StringRef::npos)
    path.truncate(end_pos);
}

শূন্য replace_extension(SmallVectorImpl<অক্ষর> &path, ধ্রুবক Twine &extension,
                       Style style) {
  StringRef p(path.begin(), path.আকার_3());
  SmallString<32> ext_storage;
  StringRef ext = extension.toStringRef(ext_storage);

  // Erase existing extension.
  size_t pos = p.find_last_of('.');
  যদি (pos != StringRef::npos && pos >= filename_pos(p, style))
    path.truncate(pos);

  // Append '.' if needed.
  যদি (ext.আকার_3() > 0 && ext[0] != '.')
    path.ঠেলো_back('.');

  // Append extension.
  path.append(ext.begin(), ext.end());
}

স্থির যৌক্তিক starts_with(StringRef Path, StringRef Prefix,
                        Style style = Style::native) {
  // Windows prefix matching : case and separator insensitive
  যদি (is_style_windows(style)) {
    যদি (Path.আকার_3() < Prefix.আকার_3())
      ফেরত মিথ্যা;
    জন্য (size_t I = 0, E = Prefix.আকার_3(); I != E; ++I) {
      যৌক্তিক SepPath = is_separator(Path[I], style);
      যৌক্তিক SepPrefix = is_separator(Prefix[I], style);
      যদি (SepPath != SepPrefix)
        ফেরত মিথ্যা;
      যদি (!SepPath && toLower(Path[I]) != toLower(Prefix[I]))
        ফেরত মিথ্যা;
    }
    ফেরত সত্য;
  }
  ফেরত Path.starts_with(Prefix);
}

যৌক্তিক replace_path_prefix(SmallVectorImpl<অক্ষর> &Path, StringRef OldPrefix,
                         StringRef NewPrefix, Style style) {
  যদি (OldPrefix.empty() && NewPrefix.empty())
    ফেরত মিথ্যা;

  StringRef OrigPath(Path.begin(), Path.আকার_3());
  যদি (!starts_with(OrigPath, OldPrefix, style))
    ফেরত মিথ্যা;

  // If prefixes have the same size we can simply copy the new one over.
  যদি (OldPrefix.আকার_3() == NewPrefix.আকার_3()) {
    llvm::copy(NewPrefix, Path.begin());
    ফেরত সত্য;
  }

  StringRef RelPath = OrigPath.substr(OldPrefix.আকার_3());
  SmallString<256> NewPath;
  (Twine(NewPrefix) + RelPath).toVector(NewPath);
  Path.swap(NewPath);
  ফেরত সত্য;
}

শূন্য native(ধ্রুবক Twine &path, SmallVectorImpl<অক্ষর> &ফলাফল_2, Style style) {
  প্রতিজ্ঞা((!path.isSingleStringRef() ||
          path.getSingleStringRef().তথ্য() != ফলাফল_2.তথ্য()) &&
         "path and result are not allowed to overlap!");
  // Clear result.
  ফলাফল_2.clear();
  path.toVector(ফলাফল_2);
  native(ফলাফল_2, style);
}

std::string native(ধ্রুবক Twine &path, Style style) {
  SmallString<128> ফলাফল;
  native(path, ফলাফল, style);
  ফেরত std::string(ফলাফল);
}

শূন্য native(SmallVectorImpl<অক্ষর> &Path, Style style) {
  যদি (Path.empty())
    ফেরত;
  যদি (is_style_windows(style)) {
    জন্য (অক্ষর &Ch : Path)
      যদি (is_separator(Ch, style))
        Ch = preferred_separator(style);
    যদি (Path[0] == '~' && (Path.আকার_3() == 1 || is_separator(Path[1], style))) {
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
    ফেরত std::string(path);

  std::string s = path.str();
  llvm::replace(s, '\\', '/');
  ফেরত s;
}

StringRef filename(StringRef path, Style style) { ফেরত *rbegin(path, style); }

StringRef stem(StringRef path, Style style) {
  StringRef fname = filename(path, style);
  size_t pos = fname.find_last_of('.');
  যদি (pos == StringRef::npos)
    ফেরত fname;
  যদি ((fname.আকার_3() == 1 && fname == ".") ||
      (fname.আকার_3() == 2 && fname == ".."))
    ফেরত fname;
  ফেরত fname.substr(0, pos);
}

StringRef extension(StringRef path, Style style) {
  StringRef fname = filename(path, style);
  size_t pos = fname.find_last_of('.');
  যদি (pos == StringRef::npos)
    ফেরত StringRef();
  যদি ((fname.আকার_3() == 1 && fname == ".") ||
      (fname.আকার_3() == 2 && fname == ".."))
    ফেরত StringRef();
  ফেরত fname.substr(pos);
}

যৌক্তিক is_separator(অক্ষর মান_2, Style style) {
  যদি (মান_2 == '/')
    ফেরত সত্য;
  যদি (is_style_windows(style))
    ফেরত মান_2 == '\\';
  ফেরত মিথ্যা;
}

StringRef get_separator(Style style) {
  যদি (real_style(style) == Style::windows)
    ফেরত "\\";
  ফেরত "/";
}

যৌক্তিক has_মূল_name(ধ্রুবক Twine &path, Style style) {
  SmallString<128> path_storage;
  StringRef p = path.toStringRef(path_storage);

  ফেরত !মূল_name(p, style).empty();
}

যৌক্তিক has_মূল_directory(ধ্রুবক Twine &path, Style style) {
  SmallString<128> path_storage;
  StringRef p = path.toStringRef(path_storage);

  ফেরত !মূল_directory(p, style).empty();
}

যৌক্তিক has_মূল_path(ধ্রুবক Twine &path, Style style) {
  SmallString<128> path_storage;
  StringRef p = path.toStringRef(path_storage);

  ফেরত !মূল_path(p, style).empty();
}

যৌক্তিক has_relative_path(ধ্রুবক Twine &path, Style style) {
  SmallString<128> path_storage;
  StringRef p = path.toStringRef(path_storage);

  ফেরত !relative_path(p, style).empty();
}

যৌক্তিক has_filename(ধ্রুবক Twine &path, Style style) {
  SmallString<128> path_storage;
  StringRef p = path.toStringRef(path_storage);

  ফেরত !filename(p, style).empty();
}

যৌক্তিক has_parent_path(ধ্রুবক Twine &path, Style style) {
  SmallString<128> path_storage;
  StringRef p = path.toStringRef(path_storage);

  ফেরত !parent_path(p, style).empty();
}

যৌক্তিক has_stem(ধ্রুবক Twine &path, Style style) {
  SmallString<128> path_storage;
  StringRef p = path.toStringRef(path_storage);

  ফেরত !stem(p, style).empty();
}

যৌক্তিক has_extension(ধ্রুবক Twine &path, Style style) {
  SmallString<128> path_storage;
  StringRef p = path.toStringRef(path_storage);

  ফেরত !extension(p, style).empty();
}

যৌক্তিক is_absolute(ধ্রুবক Twine &path, Style style) {
  SmallString<128> path_storage;
  StringRef p = path.toStringRef(path_storage);

  যৌক্তিক মূল_Dir = has_মূল_directory(p, style);
  যৌক্তিক মূল_Name = is_style_posix(style) || has_মূল_name(p, style);

  ফেরত মূল_Dir && মূল_Name;
}

যৌক্তিক is_absolute_gnu(ধ্রুবক Twine &path, Style style) {
  SmallString<128> path_storage;
  StringRef p = path.toStringRef(path_storage);

  // Handle '/' which is absolute for both Windows and POSIX systems.
  // Handle '\\' on Windows.
  যদি (!p.empty() && is_separator(p.front(), style))
    ফেরত সত্য;

  যদি (is_style_windows(style)) {
    // Handle drive letter pattern (a character followed by ':') on Windows.
    যদি (p.আকার_3() >= 2 && (p[0] && p[1] == ':'))
      ফেরত সত্য;
  }

  ফেরত মিথ্যা;
}

যৌক্তিক is_relative(ধ্রুবক Twine &path, Style style) {
  ফেরত !is_absolute(path, style);
}

শূন্য make_absolute(ধ্রুবক Twine &current_directory,
                   SmallVectorImpl<অক্ষর> &path) {
  StringRef p(path.তথ্য(), path.আকার_3());

  যৌক্তিক মূল_Directory = has_মূল_directory(p);
  যৌক্তিক মূল_Name = has_মূল_name(p);

  // Already absolute.
  যদি ((মূল_Name || is_style_posix(Style::native)) && মূল_Directory)
    ফেরত;

  // All the following conditions will need the current directory.
  SmallString<128> current_dir;
  current_directory.toVector(current_dir);

  // Relative path. Prepend the current directory.
  যদি (!মূল_Name && !মূল_Directory) {
    // Append path to the current directory.
    append(current_dir, p);
    // Set path to the result.
    path.swap(current_dir);
    ফেরত;
  }

  যদি (!মূল_Name && মূল_Directory) {
    StringRef cdrn = মূল_name(current_dir);
    SmallString<128> cur_Dir_মূল_Name(cdrn.begin(), cdrn.end());
    append(cur_Dir_মূল_Name, p);
    // Set path to the result.
    path.swap(cur_Dir_মূল_Name);
    ফেরত;
  }

  যদি (মূল_Name && !মূল_Directory) {
    StringRef p_মূল_Name = মূল_name(p);
    StringRef b_মূল_Directory = মূল_directory(current_dir);
    StringRef bRelativePath = relative_path(current_dir);
    StringRef pRelativePath = relative_path(p);

    SmallString<128> res;
    append(res, p_মূল_Name, b_মূল_Directory, bRelativePath, pRelativePath);
    path.swap(res);
    ফেরত;
  }

  llvm_unreachable("All rootName and rootDirectory combinations should have "
                   "occurred above!");
}

StringRef remove_leading_dotslash(StringRef Path, Style style) {
  // Remove leading "./" (or ".//" or "././" etc.)
  যখন (Path.আকার_3() > 2 && Path[0] == '.' && is_separator(Path[1], style)) {
    Path = Path.substr(2);
    যখন (Path.আকার_3() > 0 && is_separator(Path[0], style))
      Path = Path.substr(1);
  }
  ফেরত Path;
}

যৌক্তিক remove_dots(SmallVectorImpl<অক্ষর> &the_path, যৌক্তিক remove_dot_dot,
                 Style style) {
  style = real_style(style);
  StringRef remaining(the_path.তথ্য(), the_path.আকার_3());
  যৌক্তিক needs_change = মিথ্যা;
  SmallVector<StringRef, 16> components;

  // Consume the root path, if present.
  StringRef মূল_3 = path::মূল_path(remaining, style);
  যৌক্তিক absolute = !মূল_3.empty();
  যদি (absolute)
    remaining = remaining.drop_front(মূল_3.আকার_3());

  // Loop over path components manually. This makes it easier to detect
  // non-preferred slashes and double separators that must be canonicalized.
  যখন (!remaining.empty()) {
    size_t পরবর্তী_slash = remaining.find_first_of(separators(style));
    যদি (পরবর্তী_slash == StringRef::npos)
      পরবর্তী_slash = remaining.আকার_3();
    StringRef component = remaining.take_front(পরবর্তী_slash);
    remaining = remaining.drop_front(পরবর্তী_slash);

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

  SmallString<256> বাফার_2 = মূল_3;
  // "root" could be "/", which may need to be translated into "\".
  make_preferred(বাফার_2, style);
  needs_change |= মূল_3 != বাফার_2;

  // Avoid rewriting the path unless we have to.
  যদি (!needs_change)
    ফেরত মিথ্যা;

  যদি (!components.empty()) {
    বাফার_2 += components[0];
    জন্য (StringRef C : ArrayRef(components).drop_front()) {
      বাফার_2 += preferred_separator(style);
      বাফার_2 += C;
    }
  }
  the_path.swap(বাফার_2);
  ফেরত সত্য;
}

} // end namespace path

নামাকাশ fs {

std::error_code getUniqueID(ধ্রুবক Twine Path, UniqueID &ফলাফল) {
  sandbox::violationIfEnabled();

  file_status Status;
  std::error_code EC = status(Path, Status);
  যদি (EC)
    ফেরত EC;
  ফলাফল = Status.getUniqueID();
  ফেরত std::error_code();
}

শূন্য createUniquePath(ধ্রুবক Twine &Model, SmallVectorImpl<অক্ষর> &ফলাফল_Path,
                      যৌক্তিক MakeAbsolute) {
  SmallString<128> ModelStorage;
  Model.toVector(ModelStorage);

  প্রতিজ্ঞা(llvm::is_contained(ModelStorage, '%') &&
         "createUniquePath: Model must contain at least one '%'");

  যদি (MakeAbsolute) {
    // Make model absolute by prepending a temp directory if it's not already.
    যদি (!sys::path::is_absolute(Twine(ModelStorage))) {
      SmallString<128> TDir;
      sys::path::system_অস্থায়ী_directory(সত্য, TDir);
      sys::path::append(TDir, Twine(ModelStorage));
      ModelStorage.swap(TDir);
    }
  }

  ফলাফল_Path = ModelStorage;
  ফলাফল_Path.ঠেলো_back(0);
  ফলাফল_Path.বের_করো_back();

  // Replace '%' with random chars.
  জন্য (unsigned i = 0, e = ModelStorage.আকার_3(); i != e; ++i) {
    যদি (ModelStorage[i] == '%')
      ফলাফল_Path[i] = "0123456789abcdef"[sys::Process::Get_Random_সংখ্যা() & 15];
  }
}

std::error_code createUniqueFile(ধ্রুবক Twine &Model, পূর্ণসংখ্যা &ফলাফল_Fd,
                                 SmallVectorImpl<অক্ষর> &ফলাফল_Path,
                                 OpenFlags Flags, unsigned Mode) {
  ফেরত createUniqueEntity(Model, ফলাফল_Fd, ফলাফল_Path, মিথ্যা, FS_File, Flags,
                            Mode);
}

std::error_code createUniqueFile(ধ্রুবক Twine &Model,
                                 SmallVectorImpl<অক্ষর> &ফলাফল_Path,
                                 unsigned Mode) {
  পূর্ণসংখ্যা FD;
  auto EC = createUniqueFile(Model, FD, ফলাফল_Path, OF_None, Mode);
  যদি (EC)
    ফেরত EC;
  // FD is only needed to avoid race conditions. Close it right away.
  close(FD);
  ফেরত EC;
}

স্থির std::error_code
createTemporaryFile(ধ্রুবক Twine &Model, পূর্ণসংখ্যা &ফলাফল_FD,
                    llvm::SmallVectorImpl<অক্ষর> &ফলাফল_Path, FSEntity Type,
                    sys::fs::OpenFlags Flags = sys::fs::OF_None) {
  // Any *temporary* file is assumed to be a compiler-internal output, not
  // a formal one.
  auto BypassSandbox = sys::sandbox::scopedDisable();

  SmallString<128> Storage;
  StringRef P = Model.toNullTerminatedStringRef(Storage);
  প্রতিজ্ঞা(P.find_first_of(separators(Style::native)) == StringRef::npos &&
         "Model must be a simple filename.");
  // Use P.begin() so that createUniqueEntity doesn't need to recreate Storage.
  ফেরত createUniqueEntity(P.begin(), ফলাফল_FD, ফলাফল_Path, সত্য, Type, Flags,
                            all_read | all_write);
}

স্থির std::error_code
createTemporaryFile(ধ্রুবক Twine &Prefix, StringRef Suffix, পূর্ণসংখ্যা &ফলাফল_FD,
                    llvm::SmallVectorImpl<অক্ষর> &ফলাফল_Path, FSEntity Type,
                    sys::fs::OpenFlags Flags = sys::fs::OF_None) {
  ধ্রুবক অক্ষর *মধ্য = Suffix.empty() ? "-%%%%%%" : "-%%%%%%.";
  ফেরত createTemporaryFile(Prefix + মধ্য + Suffix, ফলাফল_FD, ফলাফল_Path,
                             Type, Flags);
}

std::error_code createTemporaryFile(ধ্রুবক Twine &Prefix, StringRef Suffix,
                                    পূর্ণসংখ্যা &ফলাফল_FD,
                                    SmallVectorImpl<অক্ষর> &ফলাফল_Path,
                                    sys::fs::OpenFlags Flags) {
  ফেরত createTemporaryFile(Prefix, Suffix, ফলাফল_FD, ফলাফল_Path, FS_File,
                             Flags);
}

std::error_code createTemporaryFile(ধ্রুবক Twine &Prefix, StringRef Suffix,
                                    SmallVectorImpl<অক্ষর> &ফলাফল_Path,
                                    sys::fs::OpenFlags Flags) {
  পূর্ণসংখ্যা FD;
  auto EC = createTemporaryFile(Prefix, Suffix, FD, ফলাফল_Path, Flags);
  যদি (EC)
    ফেরত EC;
  // FD is only needed to avoid race conditions. Close it right away.
  close(FD);
  ফেরত EC;
}

// This is a mkdtemp with a different pattern. We use createUniqueEntity mostly
// for consistency. We should try using mkdtemp.
std::error_code createUniqueDirectory(ধ্রুবক Twine &Prefix,
                                      SmallVectorImpl<অক্ষর> &ফলাফল_Path) {
  পূর্ণসংখ্যা Dummy;
  ফেরত createUniqueEntity(Prefix + "-%%%%%%", Dummy, ফলাফল_Path, সত্য,
                            FS_Dir);
}

std::error_code
getPotentiallyUniqueFileName(ধ্রুবক Twine &Model,
                             SmallVectorImpl<অক্ষর> &ফলাফল_Path) {
  পূর্ণসংখ্যা Dummy;
  ফেরত createUniqueEntity(Model, Dummy, ফলাফল_Path, মিথ্যা, FS_Name);
}

std::error_code
get_Potentially_Unique_অস্থায়ী_File_Name(ধ্রুবক Twine &Prefix, StringRef Suffix,
                                 SmallVectorImpl<অক্ষর> &ফলাফল_Path) {
  পূর্ণসংখ্যা Dummy;
  ফেরত createTemporaryFile(Prefix, Suffix, Dummy, ফলাফল_Path, FS_Name);
}

std::error_code make_absolute(SmallVectorImpl<অক্ষর> &path) {
  sandbox::violationIfEnabled();

  যদি (path::is_absolute(path))
    ফেরত {};

  SmallString<128> current_dir;
  যদি (std::error_code ec = current_path(current_dir))
    ফেরত ec;

  path::make_absolute(current_dir, path);
  ফেরত {};
}

std::error_code create_directories(ধ্রুবক Twine &Path, যৌক্তিক IgnoreExisting,
                                   perms Perms) {
  SmallString<128> PathStorage;
  StringRef P = Path.toStringRef(PathStorage);

  // Be optimistic and try to create the directory
  std::error_code EC = create_directory(P, IgnoreExisting, Perms);
  // If we succeeded, or had any error other than the parent not existing, just
  // return it.
  যদি (EC != errc::no_such_file_or_directory)
    ফেরত EC;

  // We failed because of a no_such_file_or_directory, try to create the
  // parent.
  StringRef Parent = path::parent_path(P);
  যদি (Parent.empty())
    ফেরত EC;

  যদি ((EC = create_directories(Parent, IgnoreExisting, Perms)))
      ফেরত EC;

  ফেরত create_directory(P, IgnoreExisting, Perms);
}

স্থির std::error_code copy_file_internal(পূর্ণসংখ্যা ReadFD, পূর্ণসংখ্যা WriteFD) {
  ধ্রুবক size_t Buf_আকার = 4096;
  অক্ষর *Buf = নতুন অক্ষর[Buf_আকার];
  পূর্ণসংখ্যা BytesRead = 0, BytesWritten = 0;
  জন্য (;;) {
    BytesRead = পঠ(ReadFD, Buf, Buf_আকার);
    যদি (BytesRead <= 0)
      বিরতি;
    যখন (BytesRead) {
      BytesWritten = লিখ(WriteFD, Buf, BytesRead);
      যদি (BytesWritten < 0)
        বিরতি;
      BytesRead -= BytesWritten;
    }
    যদি (BytesWritten < 0)
      বিরতি;
  }
  delete[] Buf;

  যদি (BytesRead < 0 || BytesWritten < 0)
    ফেরত errnoAsErrorCode();
  ফেরত std::error_code();
}

#ifndef __APPLE__
std::error_code copy_file(ধ্রুবক Twine &From, ধ্রুবক Twine &To) {
  পূর্ণসংখ্যা ReadFD, WriteFD;
  যদি (std::error_code EC = openFileForRead(From, ReadFD, OF_None))
    ফেরত EC;
  যদি (std::error_code EC =
          openFileForWrite(To, WriteFD, CD_CreateAlways, OF_None)) {
    close(ReadFD);
    ফেরত EC;
  }

  std::error_code EC = copy_file_internal(ReadFD, WriteFD);

  close(ReadFD);
  close(WriteFD);

  ফেরত EC;
}
#endif

std::error_code copy_file(ধ্রুবক Twine &From, পূর্ণসংখ্যা ToFD) {
  পূর্ণসংখ্যা ReadFD;
  যদি (std::error_code EC = openFileForRead(From, ReadFD, OF_None))
    ফেরত EC;

  std::error_code EC = copy_file_internal(ReadFD, ToFD);

  close(ReadFD);

  ফেরত EC;
}

ErrorOr<MD5::MD5_ফলাফল> md5_contents(পূর্ণসংখ্যা FD) {
  sandbox::violationIfEnabled();

  MD5 Hash;

  constexpr size_t Buf_আকার = 4096;
  std::vector<uint8_t> Buf(Buf_আকার);
  পূর্ণসংখ্যা BytesRead = 0;
  জন্য (;;) {
    BytesRead = পঠ(FD, Buf.তথ্য(), Buf_আকার);
    যদি (BytesRead <= 0)
      বিরতি;
    Hash.update(ArrayRef(Buf.তথ্য(), BytesRead));
  }

  যদি (BytesRead < 0)
    ফেরত errnoAsErrorCode();
  MD5::MD5_ফলাফল ফলাফল;
  Hash.final(ফলাফল);
  ফেরত ফলাফল;
}

ErrorOr<MD5::MD5_ফলাফল> md5_contents(ধ্রুবক Twine &Path) {
  sandbox::violationIfEnabled();

  পূর্ণসংখ্যা FD;
  যদি (auto EC = openFileForRead(Path, FD, OF_None))
    ফেরত EC;

  auto ফলাফল = md5_contents(FD);
  close(FD);
  ফেরত ফলাফল;
}

যৌক্তিক exists(ধ্রুবক basic_file_status &status) {
  ফেরত status_known(status) && status.type() != file_type::file_not_found;
}

যৌক্তিক status_known(ধ্রুবক basic_file_status &s) {
  ফেরত s.type() != file_type::status_error;
}

file_type get_file_type(ধ্রুবক Twine &Path, যৌক্তিক Follow) {
  file_status st;
  যদি (status(Path, st, Follow))
    ফেরত file_type::status_error;
  ফেরত st.type();
}

যৌক্তিক is_directory(ধ্রুবক basic_file_status &status) {
  ফেরত status.type() == file_type::directory_file;
}

std::error_code is_directory(ধ্রুবক Twine &path, যৌক্তিক &ফলাফল_2) {
  sandbox::violationIfEnabled();

  file_status st;
  যদি (std::error_code ec = status(path, st))
    ফেরত ec;
  ফলাফল_2 = is_directory(st);
  ফেরত std::error_code();
}

যৌক্তিক is_regular_file(ধ্রুবক basic_file_status &status) {
  ফেরত status.type() == file_type::regular_file;
}

std::error_code is_regular_file(ধ্রুবক Twine &path, যৌক্তিক &ফলাফল_2) {
  sandbox::violationIfEnabled();

  file_status st;
  যদি (std::error_code ec = status(path, st))
    ফেরত ec;
  ফলাফল_2 = is_regular_file(st);
  ফেরত std::error_code();
}

যৌক্তিক is_symlink_file(ধ্রুবক basic_file_status &status) {
  ফেরত status.type() == file_type::symlink_file;
}

std::error_code is_symlink_file(ধ্রুবক Twine &path, যৌক্তিক &ফলাফল_2) {
  sandbox::violationIfEnabled();

  file_status st;
  যদি (std::error_code ec = status(path, st, মিথ্যা))
    ফেরত ec;
  ফলাফল_2 = is_symlink_file(st);
  ফেরত std::error_code();
}

যৌক্তিক is_other(ধ্রুবক basic_file_status &status) {
  ফেরত exists(status) &&
         !is_regular_file(status) &&
         !is_directory(status);
}

std::error_code is_other(ধ্রুবক Twine &Path, যৌক্তিক &ফলাফল) {
  sandbox::violationIfEnabled();

  file_status FileStatus;
  যদি (std::error_code EC = status(Path, FileStatus))
    ফেরত EC;
  ফলাফল = is_other(FileStatus);
  ফেরত std::error_code();
}

শূন্য directory_entry::replace_filename(ধ্রুবক Twine &Filename, file_type Type,
                                       basic_file_status Status) {
  SmallString<128> PathStr = path::parent_path(Path);
  path::append(PathStr, Filename);
  এই->Path = std::string(PathStr);
  এই->Type = Type;
  এই->Status = Status;
}

ErrorOr<perms> getPermissions(ধ্রুবক Twine &Path) {
  sandbox::violationIfEnabled();

  file_status Status;
  যদি (std::error_code EC = status(Path, Status))
    ফেরত EC;

  ফেরত Status.permissions();
}

std::error_code setLastAccessAndModificationTime(ধ্রুবক Twine &Path,
                                                 TimePoint<> AccessTime,
                                                 TimePoint<> ModificationTime) {
  পূর্ণসংখ্যা FD;
  যদি (std::error_code EC = openFile(Path, FD, CD_OpenExisting, FA_Read,
                                    OF_UpdateAttributes | OF_OpenDirectory))
    ফেরত EC;
  std::error_code EC =
      setLastAccessAndModificationTime(FD, AccessTime, ModificationTime);
  Process::SafelyCloseFileDescriptor(FD);
  ফেরত EC;
}

size_t mapped_file_region::আকার_3() ধ্রুবক {
  প্রতিজ্ঞা(Mapping && "Mapping failed but used anyway!");
  ফেরত আকার_2;
}

অক্ষর *mapped_file_region::তথ্য() ধ্রুবক {
  প্রতিজ্ঞা(Mapping && "Mapping failed but used anyway!");
  ফেরত reinterpret_cast<অক্ষর *>(Mapping);
}

ধ্রুবক অক্ষর *mapped_file_region::const_তথ্য() ধ্রুবক {
  প্রতিজ্ঞা(Mapping && "Mapping failed but used anyway!");
  ফেরত reinterpret_cast<ধ্রুবক অক্ষর *>(Mapping);
}

Error readNativeFileToEOF(file_t FileHandle, SmallVectorImpl<অক্ষর> &বাফার,
                          ssize_t Chunk_আকার) {
  sandbox::violationIfEnabled();

  // Install a handler to truncate the buffer to the correct size on exit.
  size_t আকার_2 = বাফার.আকার_3();
  llvm::scope_exit TruncateOnExit([&]() { বাফার.truncate(আকার_2); });

  // Read into Buffer until we hit EOF.
  জন্য (;;) {
    বাফার.resize_for_overwrite(আকার_2 + Chunk_আকার);
    Expected<size_t> ReadBytes = readNativeFile(
        FileHandle, MutableArrayRef(বাফার.begin() + আকার_2, Chunk_আকার));
    যদি (!ReadBytes)
      ফেরত ReadBytes.takeError();
    যদি (*ReadBytes == 0)
      ফেরত Error::success();
    আকার_2 += *ReadBytes;
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

অস্থায়ী_File::অস্থায়ী_File(StringRef Name, পূর্ণসংখ্যা FD)
    : TmpName(std::string(Name)), FD(FD) {}
অস্থায়ী_File::অস্থায়ী_File(অস্থায়ী_File &&Other) { *এই = std::move(Other); }
অস্থায়ী_File &অস্থায়ী_File::operator=(অস্থায়ী_File &&Other) {
  TmpName = std::move(Other.TmpName);
  FD = Other.FD;
  Other.Done = সত্য;
  Other.FD = -1;
#ifdef _WIN32
  RemoveOnClose = Other.RemoveOnClose;
  Other.RemoveOnClose = মিথ্যা;
#endif
  ফেরত *এই;
}

অস্থায়ী_File::~অস্থায়ী_File() { প্রতিজ্ঞা(Done); }

Error অস্থায়ী_File::discard() {
  Done = সত্য;
  যদি (FD != -1 && close(FD) == -1) {
    std::error_code EC = errnoAsErrorCode();
    ফেরত errorCodeToError(EC);
  }
  FD = -1;

#ifdef _WIN32
  // On Windows, closing will remove the file, if we set the delete
  // disposition. If not, remove it manually.
  যৌক্তিক Remove = RemoveOnClose;
#অন্যথা
  // Always try to remove the file.
  যৌক্তিক Remove = সত্য;
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
  ফেরত errorCodeToError(RemoveEC);
}

Error অস্থায়ী_File::keep(ধ্রুবক Twine &Name) {
  প্রতিজ্ঞা(!Done);
  Done = সত্য;
  // Always try to close and rename.
#ifdef _WIN32
  // If we can't cancel the delete don't rename.
  auto H = reinterpret_cast<HANDLE>(_get_osfhandle(FD));
  std::error_code RenameEC =
      RemoveOnClose ? std::error_code() : setDeleteDisposition(H, মিথ্যা);
  যৌক্তিক ShouldDelete = মিথ্যা;
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
    ফেরত errorCodeToError(errnoAsErrorCode());
  FD = -1;

  ফেরত errorCodeToError(RenameEC);
}

Error অস্থায়ী_File::keep() {
  প্রতিজ্ঞা(!Done);
  Done = সত্য;

#ifdef _WIN32
  auto H = reinterpret_cast<HANDLE>(_get_osfhandle(FD));
  যদি (std::error_code EC = setDeleteDisposition(H, মিথ্যা))
    ফেরত errorCodeToError(EC);
#endif
  sys::DontRemoveFileOnSignal(TmpName);

  TmpName = "";

  যদি (close(FD) == -1)
    ফেরত errorCodeToError(errnoAsErrorCode());
  FD = -1;

  ফেরত Error::success();
}

Expected<অস্থায়ী_File> অস্থায়ী_File::create(ধ্রুবক Twine &Model, unsigned Mode,
                                    OpenFlags ExtraFlags) {
  পূর্ণসংখ্যা FD;
  SmallString<128> ফলাফল_Path;
  যদি (std::error_code EC =
          createUniqueFile(Model, FD, ফলাফল_Path, OF_Delete | ExtraFlags, Mode))
    ফেরত errorCodeToError(EC);

  অস্থায়ী_File Ret(ফলাফল_Path, FD);
#ifdef _WIN32
  auto H = reinterpret_cast<HANDLE>(_get_osfhandle(FD));
  যৌক্তিক SetSignalHandler = মিথ্যা;
  যদি (std::error_code EC = setDeleteDisposition(H, সত্য)) {
    Ret.RemoveOnClose = সত্য;
    SetSignalHandler = সত্য;
  }
#অন্যথা
  যৌক্তিক SetSignalHandler = সত্য;
#endif
  যদি (SetSignalHandler && sys::RemoveFileOnSignal(ফলাফল_Path)) {
    // Make sure we delete the file when RemoveFileOnSignal fails.
    consumeError(Ret.discard());
    std::error_code EC(errc::operation_not_permitted);
    ফেরত errorCodeToError(EC);
  }
  ফেরত std::move(Ret);
}
} // namespace fs

} // namespace sys
} // namespace llvm
