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

#ଅନ୍ତର୍ଭାଵ "llvm/Support/Path.h"
#ଅନ୍ତର୍ଭାଵ "llvm/ADT/ArrayRef.h"
#ଅନ୍ତର୍ଭାଵ "llvm/ADT/ScopeExit.h"
#ଅନ୍ତର୍ଭାଵ "llvm/ADT/StringExtras.h"
#ଅନ୍ତର୍ଭାଵ "llvm/Config/config.h"
#ଅନ୍ତର୍ଭାଵ "llvm/Config/llvm-config.h"
#ଅନ୍ତର୍ଭାଵ "llvm/Support/Errc.h"
#ଅନ୍ତର୍ଭାଵ "llvm/Support/ErrorHandling.h"
#ଅନ୍ତର୍ଭାଵ "llvm/Support/FileSystem.h"
#ଅନ୍ତର୍ଭାଵ "llvm/Support/IOSandbox.h"
#ଅନ୍ତର୍ଭାଵ "llvm/Support/Process.h"
#ଅନ୍ତର୍ଭାଵ "llvm/Support/Signals.h"
#ଅନ୍ତର୍ଭାଵ <cctype>

#ଯଦି !defined(_MSC_VER) && !defined(__MINGW32__)
#ଅନ୍ତର୍ଭାଵ <unistd.h>
#ଅନ୍ୟଥା
#ଅନ୍ତର୍ଭାଵ <io.h>
#endif

using ନାମାକାଶ llvm;
using ନାମାକାଶ llvm::support::endian;

ନାମାକାଶ {
  using llvm::StringRef;
  using llvm::sys::path::is_separator;
  using llvm::sys::path::Style;

  ଧାଡ଼ିଭିତରେ ତାର୍କିକ prefer_forward_slash() {
    ସ୍ଥିର ତାର୍କିକ prefer = []() {
      ଯଦି (std::optional<std::string> Env =
              sys::Process::GetEnv("LLVM_WINDOWS_PREFER_FORWARD_SLASH"))
        ଫେରନ୍ତୁ *Env == "1";
      ଫେରନ୍ତୁ static_cast<ତାର୍କିକ>(LLVM_WINDOWS_PREFER_FORWARD_SLASH);
    }();
    ଫେରନ୍ତୁ prefer;
  }

  ଧାଡ଼ିଭିତରେ Style real_style(Style style) {
    ଯଦି (style != Style::native)
      ଫେରନ୍ତୁ style;
    ଯଦି (is_style_posix(style))
      ଫେରନ୍ତୁ Style::posix;
    ଫେରନ୍ତୁ prefer_forward_slash() ? Style::windows_slash
                                  : Style::windows_backslash;
  }

  ଧାଡ଼ିଭିତରେ ଅଚଳ ଅକ୍ଷର *separators(Style style) {
    ଯଦି (is_style_windows(style))
      ଫେରନ୍ତୁ "\\/";
    ଫେରନ୍ତୁ "/";
  }

  ଧାଡ଼ିଭିତରେ ଅକ୍ଷର preferred_separator(Style style) {
    ଯଦି (real_style(style) == Style::windows)
      ଫେରନ୍ତୁ '\\';
    ଫେରନ୍ତୁ '/';
  }

  StringRef find_first_component(StringRef path, Style style) {
    // Look for this first component in the following order.
    // * empty (in this case we return an empty string)
    // * either C: or {//,\\}net.
    // * {/,\}
    // * {file,directory}name

    ଯଦି (path.empty())
      ଫେରନ୍ତୁ path;

    ଯଦି (is_style_windows(style)) {
      // C:
      ଯଦି (path.size() >= 2 &&
          std::isalpha(static_cast<unsigned ଅକ୍ଷର>(path[0])) && path[1] == ':')
        ଫେରନ୍ତୁ path.substr(0, 2);
    }

    // //net
    ଯଦି ((path.size() > 2) && is_separator(path[0], style) &&
        path[0] == path[1] && !is_separator(path[2], style)) {
      // Find the next directory separator.
      size_t end = path.find_first_of(separators(style), 2);
      ଫେରନ୍ତୁ path.substr(0, end);
    }

    // {/,\}
    ଯଦି (is_separator(path[0], style))
      ଫେରନ୍ତୁ path.substr(0, 1);

    // * {file,directory}name
    size_t end = path.find_first_of(separators(style));
    ଫେରନ୍ତୁ path.substr(0, end);
  }

  // Returns the first character of the filename in str. For paths ending in
  // '/', it returns the position of the '/'.
  size_t filename_pos(StringRef str, Style style) {
    ଯଦି (str.size() > 0 && is_separator(str[str.size() - 1], style))
      ଫେରନ୍ତୁ str.size() - 1;

    size_t pos = str.find_last_of(separators(style), str.size() - 1);

    ଯଦି (is_style_windows(style)) {
      ଯଦି (pos == StringRef::npos)
        pos = str.find_last_of(':', str.size() - 1);
    }

    ଯଦି (pos == StringRef::npos || (pos == 1 && is_separator(str[0], style)))
      ଫେରନ୍ତୁ 0;

    ଫେରନ୍ତୁ pos + 1;
  }

  // Returns the position of the root directory in str. If there is no root
  // directory in str, it returns StringRef::npos.
  size_t ମୂଳ_dir_start(StringRef str, Style style) {
    // case "c:/"
    ଯଦି (is_style_windows(style)) {
      ଯଦି (str.size() > 2 && str[1] == ':' && is_separator(str[2], style))
        ଫେରନ୍ତୁ 2;
    }

    // case "//net"
    ଯଦି (str.size() > 3 && is_separator(str[0], style) && str[0] == str[1] &&
        !is_separator(str[2], style)) {
      ଫେରନ୍ତୁ str.find_first_of(separators(style), 2);
    }

    // case "/"
    ଯଦି (str.size() > 0 && is_separator(str[0], style))
      ଫେରନ୍ତୁ 0;

    ଫେରନ୍ତୁ StringRef::npos;
  }

  // Returns the position past the end of the "parent path" of path. The parent
  // path will not end in '/', unless the parent is the root directory. If the
  // path has no parent, 0 is returned.
  size_t parent_path_end(StringRef path, Style style) {
    size_t end_pos = filename_pos(path, style);

    ତାର୍କିକ filename_was_sep =
        path.size() > 0 && is_separator(path[end_pos], style);

    // Skip separators until we reach root dir (or the start of the string).
    size_t ମୂଳ_dir_pos = ମୂଳ_dir_start(path, style);
    ଯେତେବେଳେ (end_pos > 0 &&
           (ମୂଳ_dir_pos == StringRef::npos || end_pos > ମୂଳ_dir_pos) &&
           is_separator(path[end_pos - 1], style))
      --end_pos;

    ଯଦି (end_pos == ମୂଳ_dir_pos && !filename_was_sep) {
      // We've reached the root dir and the input path was *not* ending in a
      // sequence of slashes. Include the root dir in the parent path.
      ଫେରନ୍ତୁ ମୂଳ_dir_pos + 1;
    }

    // Otherwise, just include before the last slash.
    ଫେରନ୍ତୁ end_pos;
  }
} // end unnamed namespace

ଗଣନା FSEntity {
  FS_Dir,
  FS_File,
  FS_Name
};

ସ୍ଥିର std::error_code
createUniqueEntity(ଅଚଳ Twine &Model, ପୂର୍ଣ୍ଣସଂଖ୍ୟା &ResultFD,
                   SmallVectorImpl<ଅକ୍ଷର> &ResultPath, ତାର୍କିକ MakeAbsolute,
                   FSEntity Type, sys::fs::OpenFlags Flags = sys::fs::OF_None,
                   unsigned Mode = 0) {

  // Limit the number of attempts we make, so that we don't infinite loop. E.g.
  // "permission denied" could be for a specific file (so we retry with a
  // different name) or for the whole directory (retry would always fail).
  // Checking which is racy, so we try a number of times, then give up.
  std::error_code EC;
  ପାଇଁ (ପୂର୍ଣ୍ଣସଂଖ୍ୟା Retries = 128; Retries > 0; --Retries) {
    sys::fs::createUniquePath(Model, ResultPath, MakeAbsolute);
    // Try to open + create the file.
    ବାଛ (Type) {
    ସ୍ଥିତି FS_File: {
      EC = sys::fs::openFileForReadWrite(Twine(ResultPath.begin()), ResultFD,
                                         sys::fs::CD_CreateNew, Flags, Mode);
      ଯଦି (EC) {
        // errc::permission_denied happens on Windows when we try to open a file
        // that has been marked for deletion.
        ଯଦି (EC == errc::file_exists || EC == errc::permission_denied)
          ଜାରି;
        ଫେରନ୍ତୁ EC;
      }

      ଫେରନ୍ତୁ std::error_code();
    }

    ସ୍ଥିତି FS_Name: {
      EC = sys::fs::access(ResultPath.begin(), sys::fs::AccessMode::Exist);
      ଯଦି (EC == errc::no_such_file_or_directory)
        ଫେରନ୍ତୁ std::error_code();
      ଯଦି (EC)
        ଫେରନ୍ତୁ EC;
      ଜାରି;
    }

    ସ୍ଥିତି FS_Dir: {
      EC = sys::fs::create_directory(ResultPath.begin(), ମିଥ୍ୟା);
      ଯଦି (EC) {
        ଯଦି (EC == errc::file_exists)
          ଜାରି;
        ଫେରନ୍ତୁ EC;
      }
      ଫେରନ୍ତୁ std::error_code();
    }
    }
    llvm_unreachable("Invalid Type");
  }
  ଫେରନ୍ତୁ EC;
}

ନାମାକାଶ llvm {
ନାମାକାଶ sys  {
ନାମାକାଶ path {

const_iterator begin(StringRef path, Style style) {
  const_iterator i;
  i.Path      = path;
  i.Component = find_first_component(path, style);
  i.Position  = 0;
  i.S = style;
  ଫେରନ୍ତୁ i;
}

const_iterator end(StringRef path) {
  const_iterator i;
  i.Path      = path;
  i.Position  = path.size();
  ଫେରନ୍ତୁ i;
}

const_iterator &const_iterator::operator++() {
  ପ୍ରତିଜ୍ଞା(Position < Path.size() && "Tried to increment past end!");

  // Increment Position to past the current component
  Position += Component.size();

  // Check for end.
  ଯଦି (Position == Path.size()) {
    Component = StringRef();
    ଫେରନ୍ତୁ *ଏହି;
  }

  // Both POSIX and Windows treat paths that begin with exactly two separators
  // specially.
  ତାର୍କିକ was_net = Component.size() > 2 && is_separator(Component[0], S) &&
                 Component[1] == Component[0] && !is_separator(Component[2], S);

  // Handle separators.
  ଯଦି (is_separator(Path[Position], S)) {
    // Root dir.
    ଯଦି (was_net ||
        // c:/
        (is_style_windows(S) && Component.ends_with(":"))) {
      Component = Path.substr(Position, 1);
      ଫେରନ୍ତୁ *ଏହି;
    }

    // Skip extra separators.
    ଯେତେବେଳେ (Position != Path.size() && is_separator(Path[Position], S)) {
      ++Position;
    }

    // Treat trailing '/' as a '.', unless it is the root dir.
    ଯଦି (Position == Path.size() && Component != "/") {
      --Position;
      Component = ".";
      ଫେରନ୍ତୁ *ଏହି;
    }
  }

  // Find next component.
  size_t end_pos = Path.find_first_of(separators(S), Position);
  Component = Path.slice(Position, end_pos);

  ଫେରନ୍ତୁ *ଏହି;
}

ତାର୍କିକ const_iterator::operator==(ଅଚଳ const_iterator &RHS) ଅଚଳ {
  ଫେରନ୍ତୁ Path.begin() == RHS.Path.begin() && Position == RHS.Position;
}

ptrdiff_t const_iterator::operator-(ଅଚଳ const_iterator &RHS) ଅଚଳ {
  ଫେରନ୍ତୁ Position - RHS.Position;
}

reverse_iterator rbegin(StringRef Path, Style style) {
  reverse_iterator I;
  I.Path = Path;
  I.Position = Path.size();
  I.S = style;
  ++I;
  ଫେରନ୍ତୁ I;
}

reverse_iterator rend(StringRef Path) {
  reverse_iterator I;
  I.Path = Path;
  I.Component = Path.substr(0, 0);
  I.Position = 0;
  ଫେରନ୍ତୁ I;
}

reverse_iterator &reverse_iterator::operator++() {
  size_t ମୂଳ_dir_pos = ମୂଳ_dir_start(Path, S);

  // Skip separators unless it's the root directory.
  size_t end_pos = Position;
  ଯେତେବେଳେ (end_pos > 0 && (end_pos - 1) != ମୂଳ_dir_pos &&
         is_separator(Path[end_pos - 1], S))
    --end_pos;

  // Treat trailing '/' as a '.', unless it is the root dir.
  ଯଦି (Position == Path.size() && !Path.empty() &&
      is_separator(Path.back(), S) &&
      (ମୂଳ_dir_pos == StringRef::npos || end_pos - 1 > ମୂଳ_dir_pos)) {
    --Position;
    Component = ".";
    ଫେରନ୍ତୁ *ଏହି;
  }

  // Find next separator.
  size_t start_pos = filename_pos(Path.substr(0, end_pos), S);
  Component = Path.slice(start_pos, end_pos);
  Position = start_pos;
  ଫେରନ୍ତୁ *ଏହି;
}

ତାର୍କିକ reverse_iterator::operator==(ଅଚଳ reverse_iterator &RHS) ଅଚଳ {
  ଫେରନ୍ତୁ Path.begin() == RHS.Path.begin() && Component == RHS.Component &&
         Position == RHS.Position;
}

ptrdiff_t reverse_iterator::operator-(ଅଚଳ reverse_iterator &RHS) ଅଚଳ {
  ଫେରନ୍ତୁ Position - RHS.Position;
}

StringRef ମୂଳ_path(StringRef path, Style style) {
  const_iterator b = begin(path, style), pos = b, e = end(path);
  ଯଦି (b != e) {
    ତାର୍କିକ has_net =
        b->size() > 2 && is_separator((*b)[0], style) && (*b)[1] == (*b)[0];
    ତାର୍କିକ has_drive = is_style_windows(style) && b->ends_with(":");

    ଯଦି (has_net || has_drive) {
      ଯଦି ((++pos != e) && is_separator((*pos)[0], style)) {
        // {C:/,//net/}, so get the first two components.
        ଫେରନ୍ତୁ path.substr(0, b->size() + pos->size());
      }
      // just {C:,//net}, return the first component.
      ଫେରନ୍ତୁ *b;
    }

    // POSIX style root directory.
    ଯଦି (is_separator((*b)[0], style)) {
      ଫେରନ୍ତୁ *b;
    }
  }

  ଫେରନ୍ତୁ StringRef();
}

StringRef ମୂଳ_name(StringRef path, Style style) {
  const_iterator b = begin(path, style), e = end(path);
  ଯଦି (b != e) {
    ତାର୍କିକ has_net =
        b->size() > 2 && is_separator((*b)[0], style) && (*b)[1] == (*b)[0];
    ତାର୍କିକ has_drive = is_style_windows(style) && b->ends_with(":");

    ଯଦି (has_net || has_drive) {
      // just {C:,//net}, return the first component.
      ଫେରନ୍ତୁ *b;
    }
  }

  // No path or no name.
  ଫେରନ୍ତୁ StringRef();
}

StringRef ମୂଳ_directory(StringRef path, Style style) {
  const_iterator b = begin(path, style), pos = b, e = end(path);
  ଯଦି (b != e) {
    ତାର୍କିକ has_net =
        b->size() > 2 && is_separator((*b)[0], style) && (*b)[1] == (*b)[0];
    ତାର୍କିକ has_drive = is_style_windows(style) && b->ends_with(":");

    ଯଦି ((has_net || has_drive) &&
        // {C:,//net}, skip to the next component.
        (++pos != e) && is_separator((*pos)[0], style)) {
      ଫେରନ୍ତୁ *pos;
    }

    // POSIX style root directory.
    ଯଦି (!has_net && is_separator((*b)[0], style)) {
      ଫେରନ୍ତୁ *b;
    }
  }

  // No path or no root.
  ଫେରନ୍ତୁ StringRef();
}

StringRef relative_path(StringRef path, Style style) {
  StringRef ମୂଳ_3 = ମୂଳ_path(path, style);
  ଫେରନ୍ତୁ path.substr(ମୂଳ_3.size());
}

ଶୂନ୍ୟ append(SmallVectorImpl<ଅକ୍ଷର> &path, Style style, ଅଚଳ Twine &a,
            ଅଚଳ Twine &b, ଅଚଳ Twine &c, ଅଚଳ Twine &d) {
  SmallString<32> a_storage;
  SmallString<32> b_storage;
  SmallString<32> c_storage;
  SmallString<32> d_storage;

  SmallVector<StringRef, 4> components;
  ଯଦି (!a.isTriviallyEmpty()) components.ଠେଲ_back(a.toStringRef(a_storage));
  ଯଦି (!b.isTriviallyEmpty()) components.ଠେଲ_back(b.toStringRef(b_storage));
  ଯଦି (!c.isTriviallyEmpty()) components.ଠେଲ_back(c.toStringRef(c_storage));
  ଯଦି (!d.isTriviallyEmpty()) components.ଠେଲ_back(d.toStringRef(d_storage));

  ପାଇଁ (auto &component : components) {
    ତାର୍କିକ path_has_sep =
        !path.empty() && is_separator(path[path.size() - 1], style);
    ଯଦି (path_has_sep) {
      // Strip separators from beginning of component.
      size_t loc = component.find_first_not_of(separators(style));
      StringRef c = component.substr(loc);

      // Append it.
      path.append(c.begin(), c.end());
      ଜାରି;
    }

    ତାର୍କିକ component_has_sep =
        !component.empty() && is_separator(component[0], style);
    ଯଦି (!component_has_sep &&
        !(path.empty() || has_ମୂଳ_name(component, style))) {
      // Add a separator.
      path.ଠେଲ_back(preferred_separator(style));
    }

    path.append(component.begin(), component.end());
  }
}

ଶୂନ୍ୟ append(SmallVectorImpl<ଅକ୍ଷର> &path, ଅଚଳ Twine &a, ଅଚଳ Twine &b,
            ଅଚଳ Twine &c, ଅଚଳ Twine &d) {
  append(path, Style::native, a, b, c, d);
}

ଶୂନ୍ୟ append(SmallVectorImpl<ଅକ୍ଷର> &path, const_iterator begin,
            const_iterator end, Style style) {
  ପାଇଁ (; begin != end; ++begin)
    path::append(path, style, *begin);
}

StringRef parent_path(StringRef path, Style style) {
  size_t end_pos = parent_path_end(path, style);
  ଯଦି (end_pos == StringRef::npos)
    ଫେରନ୍ତୁ StringRef();
  ଫେରନ୍ତୁ path.substr(0, end_pos);
}

ଶୂନ୍ୟ remove_filename(SmallVectorImpl<ଅକ୍ଷର> &path, Style style) {
  size_t end_pos = parent_path_end(StringRef(path.begin(), path.size()), style);
  ଯଦି (end_pos != StringRef::npos)
    path.truncate(end_pos);
}

ଶୂନ୍ୟ replace_extension(SmallVectorImpl<ଅକ୍ଷର> &path, ଅଚଳ Twine &extension,
                       Style style) {
  StringRef p(path.begin(), path.size());
  SmallString<32> ext_storage;
  StringRef ext = extension.toStringRef(ext_storage);

  // Erase existing extension.
  size_t pos = p.find_last_of('.');
  ଯଦି (pos != StringRef::npos && pos >= filename_pos(p, style))
    path.truncate(pos);

  // Append '.' if needed.
  ଯଦି (ext.size() > 0 && ext[0] != '.')
    path.ଠେଲ_back('.');

  // Append extension.
  path.append(ext.begin(), ext.end());
}

ସ୍ଥିର ତାର୍କିକ starts_with(StringRef Path, StringRef Prefix,
                        Style style = Style::native) {
  // Windows prefix matching : case and separator insensitive
  ଯଦି (is_style_windows(style)) {
    ଯଦି (Path.size() < Prefix.size())
      ଫେରନ୍ତୁ ମିଥ୍ୟା;
    ପାଇଁ (size_t I = 0, E = Prefix.size(); I != E; ++I) {
      ତାର୍କିକ SepPath = is_separator(Path[I], style);
      ତାର୍କିକ SepPrefix = is_separator(Prefix[I], style);
      ଯଦି (SepPath != SepPrefix)
        ଫେରନ୍ତୁ ମିଥ୍ୟା;
      ଯଦି (!SepPath && toLower(Path[I]) != toLower(Prefix[I]))
        ଫେରନ୍ତୁ ମିଥ୍ୟା;
    }
    ଫେରନ୍ତୁ ସତ୍ୟ;
  }
  ଫେରନ୍ତୁ Path.starts_with(Prefix);
}

ତାର୍କିକ replace_path_prefix(SmallVectorImpl<ଅକ୍ଷର> &Path, StringRef OldPrefix,
                         StringRef NewPrefix, Style style) {
  ଯଦି (OldPrefix.empty() && NewPrefix.empty())
    ଫେରନ୍ତୁ ମିଥ୍ୟା;

  StringRef OrigPath(Path.begin(), Path.size());
  ଯଦି (!starts_with(OrigPath, OldPrefix, style))
    ଫେରନ୍ତୁ ମିଥ୍ୟା;

  // If prefixes have the same size we can simply copy the new one over.
  ଯଦି (OldPrefix.size() == NewPrefix.size()) {
    llvm::copy(NewPrefix, Path.begin());
    ଫେରନ୍ତୁ ସତ୍ୟ;
  }

  StringRef RelPath = OrigPath.substr(OldPrefix.size());
  SmallString<256> NewPath;
  (Twine(NewPrefix) + RelPath).toVector(NewPath);
  Path.swap(NewPath);
  ଫେରନ୍ତୁ ସତ୍ୟ;
}

ଶୂନ୍ୟ native(ଅଚଳ Twine &path, SmallVectorImpl<ଅକ୍ଷର> &result, Style style) {
  ପ୍ରତିଜ୍ଞା((!path.isSingleStringRef() ||
          path.getSingleStringRef().data() != result.data()) &&
         "path and result are not allowed to overlap!");
  // Clear result.
  result.clear();
  path.toVector(result);
  native(result, style);
}

std::string native(ଅଚଳ Twine &path, Style style) {
  SmallString<128> Result;
  native(path, Result, style);
  ଫେରନ୍ତୁ std::string(Result);
}

ଶୂନ୍ୟ native(SmallVectorImpl<ଅକ୍ଷର> &Path, Style style) {
  ଯଦି (Path.empty())
    ଫେରନ୍ତୁ;
  ଯଦି (is_style_windows(style)) {
    ପାଇଁ (ଅକ୍ଷର &Ch : Path)
      ଯଦି (is_separator(Ch, style))
        Ch = preferred_separator(style);
    ଯଦି (Path[0] == '~' && (Path.size() == 1 || is_separator(Path[1], style))) {
      SmallString<128> PathHome;
      home_directory(PathHome);
      PathHome.append(Path.begin() + 1, Path.end());
      Path = std::move(PathHome);
    }
  } ଅନ୍ୟଥା {
    llvm::replace(Path, '\\', '/');
  }
}

std::string convert_to_slash(StringRef path, Style style) {
  ଯଦି (is_style_posix(style))
    ଫେରନ୍ତୁ std::string(path);

  std::string s = path.str();
  llvm::replace(s, '\\', '/');
  ଫେରନ୍ତୁ s;
}

StringRef filename(StringRef path, Style style) { ଫେରନ୍ତୁ *rbegin(path, style); }

StringRef stem(StringRef path, Style style) {
  StringRef fname = filename(path, style);
  size_t pos = fname.find_last_of('.');
  ଯଦି (pos == StringRef::npos)
    ଫେରନ୍ତୁ fname;
  ଯଦି ((fname.size() == 1 && fname == ".") ||
      (fname.size() == 2 && fname == ".."))
    ଫେରନ୍ତୁ fname;
  ଫେରନ୍ତୁ fname.substr(0, pos);
}

StringRef extension(StringRef path, Style style) {
  StringRef fname = filename(path, style);
  size_t pos = fname.find_last_of('.');
  ଯଦି (pos == StringRef::npos)
    ଫେରନ୍ତୁ StringRef();
  ଯଦି ((fname.size() == 1 && fname == ".") ||
      (fname.size() == 2 && fname == ".."))
    ଫେରନ୍ତୁ StringRef();
  ଫେରନ୍ତୁ fname.substr(pos);
}

ତାର୍କିକ is_separator(ଅକ୍ଷର ମୂଲ୍ୟ_2, Style style) {
  ଯଦି (ମୂଲ୍ୟ_2 == '/')
    ଫେରନ୍ତୁ ସତ୍ୟ;
  ଯଦି (is_style_windows(style))
    ଫେରନ୍ତୁ ମୂଲ୍ୟ_2 == '\\';
  ଫେରନ୍ତୁ ମିଥ୍ୟା;
}

StringRef get_separator(Style style) {
  ଯଦି (real_style(style) == Style::windows)
    ଫେରନ୍ତୁ "\\";
  ଫେରନ୍ତୁ "/";
}

ତାର୍କିକ has_ମୂଳ_name(ଅଚଳ Twine &path, Style style) {
  SmallString<128> path_storage;
  StringRef p = path.toStringRef(path_storage);

  ଫେରନ୍ତୁ !ମୂଳ_name(p, style).empty();
}

ତାର୍କିକ has_ମୂଳ_directory(ଅଚଳ Twine &path, Style style) {
  SmallString<128> path_storage;
  StringRef p = path.toStringRef(path_storage);

  ଫେରନ୍ତୁ !ମୂଳ_directory(p, style).empty();
}

ତାର୍କିକ has_ମୂଳ_path(ଅଚଳ Twine &path, Style style) {
  SmallString<128> path_storage;
  StringRef p = path.toStringRef(path_storage);

  ଫେରନ୍ତୁ !ମୂଳ_path(p, style).empty();
}

ତାର୍କିକ has_relative_path(ଅଚଳ Twine &path, Style style) {
  SmallString<128> path_storage;
  StringRef p = path.toStringRef(path_storage);

  ଫେରନ୍ତୁ !relative_path(p, style).empty();
}

ତାର୍କିକ has_filename(ଅଚଳ Twine &path, Style style) {
  SmallString<128> path_storage;
  StringRef p = path.toStringRef(path_storage);

  ଫେରନ୍ତୁ !filename(p, style).empty();
}

ତାର୍କିକ has_parent_path(ଅଚଳ Twine &path, Style style) {
  SmallString<128> path_storage;
  StringRef p = path.toStringRef(path_storage);

  ଫେରନ୍ତୁ !parent_path(p, style).empty();
}

ତାର୍କିକ has_stem(ଅଚଳ Twine &path, Style style) {
  SmallString<128> path_storage;
  StringRef p = path.toStringRef(path_storage);

  ଫେରନ୍ତୁ !stem(p, style).empty();
}

ତାର୍କିକ has_extension(ଅଚଳ Twine &path, Style style) {
  SmallString<128> path_storage;
  StringRef p = path.toStringRef(path_storage);

  ଫେରନ୍ତୁ !extension(p, style).empty();
}

ତାର୍କିକ is_absolute(ଅଚଳ Twine &path, Style style) {
  SmallString<128> path_storage;
  StringRef p = path.toStringRef(path_storage);

  ତାର୍କିକ ମୂଳ_Dir = has_ମୂଳ_directory(p, style);
  ତାର୍କିକ ମୂଳ_Name = is_style_posix(style) || has_ମୂଳ_name(p, style);

  ଫେରନ୍ତୁ ମୂଳ_Dir && ମୂଳ_Name;
}

ତାର୍କିକ is_absolute_gnu(ଅଚଳ Twine &path, Style style) {
  SmallString<128> path_storage;
  StringRef p = path.toStringRef(path_storage);

  // Handle '/' which is absolute for both Windows and POSIX systems.
  // Handle '\\' on Windows.
  ଯଦି (!p.empty() && is_separator(p.front(), style))
    ଫେରନ୍ତୁ ସତ୍ୟ;

  ଯଦି (is_style_windows(style)) {
    // Handle drive letter pattern (a character followed by ':') on Windows.
    ଯଦି (p.size() >= 2 && (p[0] && p[1] == ':'))
      ଫେରନ୍ତୁ ସତ୍ୟ;
  }

  ଫେରନ୍ତୁ ମିଥ୍ୟା;
}

ତାର୍କିକ is_relative(ଅଚଳ Twine &path, Style style) {
  ଫେରନ୍ତୁ !is_absolute(path, style);
}

ଶୂନ୍ୟ make_absolute(ଅଚଳ Twine &current_directory,
                   SmallVectorImpl<ଅକ୍ଷର> &path) {
  StringRef p(path.data(), path.size());

  ତାର୍କିକ ମୂଳ_Directory = has_ମୂଳ_directory(p);
  ତାର୍କିକ ମୂଳ_Name = has_ମୂଳ_name(p);

  // Already absolute.
  ଯଦି ((ମୂଳ_Name || is_style_posix(Style::native)) && ମୂଳ_Directory)
    ଫେରନ୍ତୁ;

  // All the following conditions will need the current directory.
  SmallString<128> current_dir;
  current_directory.toVector(current_dir);

  // Relative path. Prepend the current directory.
  ଯଦି (!ମୂଳ_Name && !ମୂଳ_Directory) {
    // Append path to the current directory.
    append(current_dir, p);
    // Set path to the result.
    path.swap(current_dir);
    ଫେରନ୍ତୁ;
  }

  ଯଦି (!ମୂଳ_Name && ମୂଳ_Directory) {
    StringRef cdrn = ମୂଳ_name(current_dir);
    SmallString<128> cur_Dir_ମୂଳ_Name(cdrn.begin(), cdrn.end());
    append(cur_Dir_ମୂଳ_Name, p);
    // Set path to the result.
    path.swap(cur_Dir_ମୂଳ_Name);
    ଫେରନ୍ତୁ;
  }

  ଯଦି (ମୂଳ_Name && !ମୂଳ_Directory) {
    StringRef p_ମୂଳ_Name = ମୂଳ_name(p);
    StringRef b_ମୂଳ_Directory = ମୂଳ_directory(current_dir);
    StringRef bRelativePath = relative_path(current_dir);
    StringRef pRelativePath = relative_path(p);

    SmallString<128> res;
    append(res, p_ମୂଳ_Name, b_ମୂଳ_Directory, bRelativePath, pRelativePath);
    path.swap(res);
    ଫେରନ୍ତୁ;
  }

  llvm_unreachable("All rootName and rootDirectory combinations should have "
                   "occurred above!");
}

StringRef remove_leading_dotslash(StringRef Path, Style style) {
  // Remove leading "./" (or ".//" or "././" etc.)
  ଯେତେବେଳେ (Path.size() > 2 && Path[0] == '.' && is_separator(Path[1], style)) {
    Path = Path.substr(2);
    ଯେତେବେଳେ (Path.size() > 0 && is_separator(Path[0], style))
      Path = Path.substr(1);
  }
  ଫେରନ୍ତୁ Path;
}

ତାର୍କିକ remove_dots(SmallVectorImpl<ଅକ୍ଷର> &the_path, ତାର୍କିକ remove_dot_dot,
                 Style style) {
  style = real_style(style);
  StringRef remaining(the_path.data(), the_path.size());
  ତାର୍କିକ needs_change = ମିଥ୍ୟା;
  SmallVector<StringRef, 16> components;

  // Consume the root path, if present.
  StringRef ମୂଳ_3 = path::ମୂଳ_path(remaining, style);
  ତାର୍କିକ absolute = !ମୂଳ_3.empty();
  ଯଦି (absolute)
    remaining = remaining.drop_front(ମୂଳ_3.size());

  // Loop over path components manually. This makes it easier to detect
  // non-preferred slashes and double separators that must be canonicalized.
  ଯେତେବେଳେ (!remaining.empty()) {
    size_t next_slash = remaining.find_first_of(separators(style));
    ଯଦି (next_slash == StringRef::npos)
      next_slash = remaining.size();
    StringRef component = remaining.take_front(next_slash);
    remaining = remaining.drop_front(next_slash);

    // Eat the slash, and check if it is the preferred separator.
    ଯଦି (!remaining.empty()) {
      needs_change |= remaining.front() != preferred_separator(style);
      remaining = remaining.drop_front();
      // The path needs to be rewritten if it has a trailing slash.
      // FIXME: This is emergent behavior that could be removed.
      needs_change |= remaining.empty();
    }

    // Check for path traversal components or double separators.
    ଯଦି (component.empty() || component == ".") {
      needs_change = ସତ୍ୟ;
    } ଅନ୍ୟଥା ଯଦି (remove_dot_dot && component == "..") {
      needs_change = ସତ୍ୟ;
      // Do not allow ".." to remove the root component. If this is the
      // beginning of a relative path, keep the ".." component.
      ଯଦି (!components.empty() && components.back() != "..") {
        components.କାଢ଼_back();
      } ଅନ୍ୟଥା ଯଦି (!absolute) {
        components.ଠେଲ_back(component);
      }
    } ଅନ୍ୟଥା {
      components.ଠେଲ_back(component);
    }
  }

  SmallString<256> buffer = ମୂଳ_3;
  // "root" could be "/", which may need to be translated into "\".
  make_preferred(buffer, style);
  needs_change |= ମୂଳ_3 != buffer;

  // Avoid rewriting the path unless we have to.
  ଯଦି (!needs_change)
    ଫେରନ୍ତୁ ମିଥ୍ୟା;

  ଯଦି (!components.empty()) {
    buffer += components[0];
    ପାଇଁ (StringRef C : ArrayRef(components).drop_front()) {
      buffer += preferred_separator(style);
      buffer += C;
    }
  }
  the_path.swap(buffer);
  ଫେରନ୍ତୁ ସତ୍ୟ;
}

} // end namespace path

ନାମାକାଶ fs {

std::error_code getUniqueID(ଅଚଳ Twine Path, UniqueID &Result) {
  sandbox::violationIfEnabled();

  file_status Status;
  std::error_code EC = status(Path, Status);
  ଯଦି (EC)
    ଫେରନ୍ତୁ EC;
  Result = Status.getUniqueID();
  ଫେରନ୍ତୁ std::error_code();
}

ଶୂନ୍ୟ createUniquePath(ଅଚଳ Twine &Model, SmallVectorImpl<ଅକ୍ଷର> &ResultPath,
                      ତାର୍କିକ MakeAbsolute) {
  SmallString<128> ModelStorage;
  Model.toVector(ModelStorage);

  ପ୍ରତିଜ୍ଞା(llvm::is_contained(ModelStorage, '%') &&
         "createUniquePath: Model must contain at least one '%'");

  ଯଦି (MakeAbsolute) {
    // Make model absolute by prepending a temp directory if it's not already.
    ଯଦି (!sys::path::is_absolute(Twine(ModelStorage))) {
      SmallString<128> TDir;
      sys::path::system_temp_directory(ସତ୍ୟ, TDir);
      sys::path::append(TDir, Twine(ModelStorage));
      ModelStorage.swap(TDir);
    }
  }

  ResultPath = ModelStorage;
  ResultPath.ଠେଲ_back(0);
  ResultPath.କାଢ଼_back();

  // Replace '%' with random chars.
  ପାଇଁ (unsigned i = 0, e = ModelStorage.size(); i != e; ++i) {
    ଯଦି (ModelStorage[i] == '%')
      ResultPath[i] = "0123456789abcdef"[sys::Process::Get_Random_ସଂଖ୍ୟା() & 15];
  }
}

std::error_code createUniqueFile(ଅଚଳ Twine &Model, ପୂର୍ଣ୍ଣସଂଖ୍ୟା &ResultFd,
                                 SmallVectorImpl<ଅକ୍ଷର> &ResultPath,
                                 OpenFlags Flags, unsigned Mode) {
  ଫେରନ୍ତୁ createUniqueEntity(Model, ResultFd, ResultPath, ମିଥ୍ୟା, FS_File, Flags,
                            Mode);
}

std::error_code createUniqueFile(ଅଚଳ Twine &Model,
                                 SmallVectorImpl<ଅକ୍ଷର> &ResultPath,
                                 unsigned Mode) {
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା FD;
  auto EC = createUniqueFile(Model, FD, ResultPath, OF_None, Mode);
  ଯଦି (EC)
    ଫେରନ୍ତୁ EC;
  // FD is only needed to avoid race conditions. Close it right away.
  close(FD);
  ଫେରନ୍ତୁ EC;
}

ସ୍ଥିର std::error_code
createTemporaryFile(ଅଚଳ Twine &Model, ପୂର୍ଣ୍ଣସଂଖ୍ୟା &ResultFD,
                    llvm::SmallVectorImpl<ଅକ୍ଷର> &ResultPath, FSEntity Type,
                    sys::fs::OpenFlags Flags = sys::fs::OF_None) {
  // Any *temporary* file is assumed to be a compiler-internal output, not
  // a formal one.
  auto BypassSandbox = sys::sandbox::scopedDisable();

  SmallString<128> Storage;
  StringRef P = Model.toNullTerminatedStringRef(Storage);
  ପ୍ରତିଜ୍ଞା(P.find_first_of(separators(Style::native)) == StringRef::npos &&
         "Model must be a simple filename.");
  // Use P.begin() so that createUniqueEntity doesn't need to recreate Storage.
  ଫେରନ୍ତୁ createUniqueEntity(P.begin(), ResultFD, ResultPath, ସତ୍ୟ, Type, Flags,
                            all_read | all_write);
}

ସ୍ଥିର std::error_code
createTemporaryFile(ଅଚଳ Twine &Prefix, StringRef Suffix, ପୂର୍ଣ୍ଣସଂଖ୍ୟା &ResultFD,
                    llvm::SmallVectorImpl<ଅକ୍ଷର> &ResultPath, FSEntity Type,
                    sys::fs::OpenFlags Flags = sys::fs::OF_None) {
  ଅଚଳ ଅକ୍ଷର *ମଧ୍ୟ = Suffix.empty() ? "-%%%%%%" : "-%%%%%%.";
  ଫେରନ୍ତୁ createTemporaryFile(Prefix + ମଧ୍ୟ + Suffix, ResultFD, ResultPath,
                             Type, Flags);
}

std::error_code createTemporaryFile(ଅଚଳ Twine &Prefix, StringRef Suffix,
                                    ପୂର୍ଣ୍ଣସଂଖ୍ୟା &ResultFD,
                                    SmallVectorImpl<ଅକ୍ଷର> &ResultPath,
                                    sys::fs::OpenFlags Flags) {
  ଫେରନ୍ତୁ createTemporaryFile(Prefix, Suffix, ResultFD, ResultPath, FS_File,
                             Flags);
}

std::error_code createTemporaryFile(ଅଚଳ Twine &Prefix, StringRef Suffix,
                                    SmallVectorImpl<ଅକ୍ଷର> &ResultPath,
                                    sys::fs::OpenFlags Flags) {
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା FD;
  auto EC = createTemporaryFile(Prefix, Suffix, FD, ResultPath, Flags);
  ଯଦି (EC)
    ଫେରନ୍ତୁ EC;
  // FD is only needed to avoid race conditions. Close it right away.
  close(FD);
  ଫେରନ୍ତୁ EC;
}

// This is a mkdtemp with a different pattern. We use createUniqueEntity mostly
// for consistency. We should try using mkdtemp.
std::error_code createUniqueDirectory(ଅଚଳ Twine &Prefix,
                                      SmallVectorImpl<ଅକ୍ଷର> &ResultPath) {
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା Dummy;
  ଫେରନ୍ତୁ createUniqueEntity(Prefix + "-%%%%%%", Dummy, ResultPath, ସତ୍ୟ,
                            FS_Dir);
}

std::error_code
getPotentiallyUniqueFileName(ଅଚଳ Twine &Model,
                             SmallVectorImpl<ଅକ୍ଷର> &ResultPath) {
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା Dummy;
  ଫେରନ୍ତୁ createUniqueEntity(Model, Dummy, ResultPath, ମିଥ୍ୟା, FS_Name);
}

std::error_code
getPotentiallyUniqueTempFileName(ଅଚଳ Twine &Prefix, StringRef Suffix,
                                 SmallVectorImpl<ଅକ୍ଷର> &ResultPath) {
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା Dummy;
  ଫେରନ୍ତୁ createTemporaryFile(Prefix, Suffix, Dummy, ResultPath, FS_Name);
}

std::error_code make_absolute(SmallVectorImpl<ଅକ୍ଷର> &path) {
  sandbox::violationIfEnabled();

  ଯଦି (path::is_absolute(path))
    ଫେରନ୍ତୁ {};

  SmallString<128> current_dir;
  ଯଦି (std::error_code ec = current_path(current_dir))
    ଫେରନ୍ତୁ ec;

  path::make_absolute(current_dir, path);
  ଫେରନ୍ତୁ {};
}

std::error_code create_directories(ଅଚଳ Twine &Path, ତାର୍କିକ IgnoreExisting,
                                   perms Perms) {
  SmallString<128> PathStorage;
  StringRef P = Path.toStringRef(PathStorage);

  // Be optimistic and try to create the directory
  std::error_code EC = create_directory(P, IgnoreExisting, Perms);
  // If we succeeded, or had any error other than the parent not existing, just
  // return it.
  ଯଦି (EC != errc::no_such_file_or_directory)
    ଫେରନ୍ତୁ EC;

  // We failed because of a no_such_file_or_directory, try to create the
  // parent.
  StringRef Parent = path::parent_path(P);
  ଯଦି (Parent.empty())
    ଫେରନ୍ତୁ EC;

  ଯଦି ((EC = create_directories(Parent, IgnoreExisting, Perms)))
      ଫେରନ୍ତୁ EC;

  ଫେରନ୍ତୁ create_directory(P, IgnoreExisting, Perms);
}

ସ୍ଥିର std::error_code copy_file_internal(ପୂର୍ଣ୍ଣସଂଖ୍ୟା ReadFD, ପୂର୍ଣ୍ଣସଂଖ୍ୟା WriteFD) {
  ଅଚଳ size_t BufSize = 4096;
  ଅକ୍ଷର *Buf = ନୂତନ ଅକ୍ଷର[BufSize];
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା BytesRead = 0, BytesWritten = 0;
  ପାଇଁ (;;) {
    BytesRead = ପଠ(ReadFD, Buf, BufSize);
    ଯଦି (BytesRead <= 0)
      ଭାଙ୍ଗ;
    ଯେତେବେଳେ (BytesRead) {
      BytesWritten = ଲିଖ(WriteFD, Buf, BytesRead);
      ଯଦି (BytesWritten < 0)
        ଭାଙ୍ଗ;
      BytesRead -= BytesWritten;
    }
    ଯଦି (BytesWritten < 0)
      ଭାଙ୍ଗ;
  }
  delete[] Buf;

  ଯଦି (BytesRead < 0 || BytesWritten < 0)
    ଫେରନ୍ତୁ errnoAsErrorCode();
  ଫେରନ୍ତୁ std::error_code();
}

#ifndef __APPLE__
std::error_code copy_file(ଅଚଳ Twine &From, ଅଚଳ Twine &To) {
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା ReadFD, WriteFD;
  ଯଦି (std::error_code EC = openFileForRead(From, ReadFD, OF_None))
    ଫେରନ୍ତୁ EC;
  ଯଦି (std::error_code EC =
          openFileForWrite(To, WriteFD, CD_CreateAlways, OF_None)) {
    close(ReadFD);
    ଫେରନ୍ତୁ EC;
  }

  std::error_code EC = copy_file_internal(ReadFD, WriteFD);

  close(ReadFD);
  close(WriteFD);

  ଫେରନ୍ତୁ EC;
}
#endif

std::error_code copy_file(ଅଚଳ Twine &From, ପୂର୍ଣ୍ଣସଂଖ୍ୟା ToFD) {
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା ReadFD;
  ଯଦି (std::error_code EC = openFileForRead(From, ReadFD, OF_None))
    ଫେରନ୍ତୁ EC;

  std::error_code EC = copy_file_internal(ReadFD, ToFD);

  close(ReadFD);

  ଫେରନ୍ତୁ EC;
}

ErrorOr<MD5::MD5Result> md5_contents(ପୂର୍ଣ୍ଣସଂଖ୍ୟା FD) {
  sandbox::violationIfEnabled();

  MD5 Hash;

  constexpr size_t BufSize = 4096;
  std::vector<uint8_t> Buf(BufSize);
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା BytesRead = 0;
  ପାଇଁ (;;) {
    BytesRead = ପଠ(FD, Buf.data(), BufSize);
    ଯଦି (BytesRead <= 0)
      ଭାଙ୍ଗ;
    Hash.update(ArrayRef(Buf.data(), BytesRead));
  }

  ଯଦି (BytesRead < 0)
    ଫେରନ୍ତୁ errnoAsErrorCode();
  MD5::MD5Result Result;
  Hash.final(Result);
  ଫେରନ୍ତୁ Result;
}

ErrorOr<MD5::MD5Result> md5_contents(ଅଚଳ Twine &Path) {
  sandbox::violationIfEnabled();

  ପୂର୍ଣ୍ଣସଂଖ୍ୟା FD;
  ଯଦି (auto EC = openFileForRead(Path, FD, OF_None))
    ଫେରନ୍ତୁ EC;

  auto Result = md5_contents(FD);
  close(FD);
  ଫେରନ୍ତୁ Result;
}

ତାର୍କିକ exists(ଅଚଳ basic_file_status &status) {
  ଫେରନ୍ତୁ status_known(status) && status.type() != file_type::file_not_found;
}

ତାର୍କିକ status_known(ଅଚଳ basic_file_status &s) {
  ଫେରନ୍ତୁ s.type() != file_type::status_error;
}

file_type get_file_type(ଅଚଳ Twine &Path, ତାର୍କିକ Follow) {
  file_status st;
  ଯଦି (status(Path, st, Follow))
    ଫେରନ୍ତୁ file_type::status_error;
  ଫେରନ୍ତୁ st.type();
}

ତାର୍କିକ is_directory(ଅଚଳ basic_file_status &status) {
  ଫେରନ୍ତୁ status.type() == file_type::directory_file;
}

std::error_code is_directory(ଅଚଳ Twine &path, ତାର୍କିକ &result) {
  sandbox::violationIfEnabled();

  file_status st;
  ଯଦି (std::error_code ec = status(path, st))
    ଫେରନ୍ତୁ ec;
  result = is_directory(st);
  ଫେରନ୍ତୁ std::error_code();
}

ତାର୍କିକ is_regular_file(ଅଚଳ basic_file_status &status) {
  ଫେରନ୍ତୁ status.type() == file_type::regular_file;
}

std::error_code is_regular_file(ଅଚଳ Twine &path, ତାର୍କିକ &result) {
  sandbox::violationIfEnabled();

  file_status st;
  ଯଦି (std::error_code ec = status(path, st))
    ଫେରନ୍ତୁ ec;
  result = is_regular_file(st);
  ଫେରନ୍ତୁ std::error_code();
}

ତାର୍କିକ is_symlink_file(ଅଚଳ basic_file_status &status) {
  ଫେରନ୍ତୁ status.type() == file_type::symlink_file;
}

std::error_code is_symlink_file(ଅଚଳ Twine &path, ତାର୍କିକ &result) {
  sandbox::violationIfEnabled();

  file_status st;
  ଯଦି (std::error_code ec = status(path, st, ମିଥ୍ୟା))
    ଫେରନ୍ତୁ ec;
  result = is_symlink_file(st);
  ଫେରନ୍ତୁ std::error_code();
}

ତାର୍କିକ is_other(ଅଚଳ basic_file_status &status) {
  ଫେରନ୍ତୁ exists(status) &&
         !is_regular_file(status) &&
         !is_directory(status);
}

std::error_code is_other(ଅଚଳ Twine &Path, ତାର୍କିକ &Result) {
  sandbox::violationIfEnabled();

  file_status FileStatus;
  ଯଦି (std::error_code EC = status(Path, FileStatus))
    ଫେରନ୍ତୁ EC;
  Result = is_other(FileStatus);
  ଫେରନ୍ତୁ std::error_code();
}

ଶୂନ୍ୟ directory_entry::replace_filename(ଅଚଳ Twine &Filename, file_type Type,
                                       basic_file_status Status) {
  SmallString<128> PathStr = path::parent_path(Path);
  path::append(PathStr, Filename);
  ଏହି->Path = std::string(PathStr);
  ଏହି->Type = Type;
  ଏହି->Status = Status;
}

ErrorOr<perms> getPermissions(ଅଚଳ Twine &Path) {
  sandbox::violationIfEnabled();

  file_status Status;
  ଯଦି (std::error_code EC = status(Path, Status))
    ଫେରନ୍ତୁ EC;

  ଫେରନ୍ତୁ Status.permissions();
}

std::error_code setLastAccessAndModificationTime(ଅଚଳ Twine &Path,
                                                 TimePoint<> AccessTime,
                                                 TimePoint<> ModificationTime) {
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା FD;
  ଯଦି (std::error_code EC = openFile(Path, FD, CD_OpenExisting, FA_Read,
                                    OF_UpdateAttributes | OF_OpenDirectory))
    ଫେରନ୍ତୁ EC;
  std::error_code EC =
      setLastAccessAndModificationTime(FD, AccessTime, ModificationTime);
  Process::SafelyCloseFileDescriptor(FD);
  ଫେରନ୍ତୁ EC;
}

size_t mapped_file_region::size() ଅଚଳ {
  ପ୍ରତିଜ୍ଞା(Mapping && "Mapping failed but used anyway!");
  ଫେରନ୍ତୁ Size;
}

ଅକ୍ଷର *mapped_file_region::data() ଅଚଳ {
  ପ୍ରତିଜ୍ଞା(Mapping && "Mapping failed but used anyway!");
  ଫେରନ୍ତୁ reinterpret_cast<ଅକ୍ଷର *>(Mapping);
}

ଅଚଳ ଅକ୍ଷର *mapped_file_region::const_data() ଅଚଳ {
  ପ୍ରତିଜ୍ଞା(Mapping && "Mapping failed but used anyway!");
  ଫେରନ୍ତୁ reinterpret_cast<ଅଚଳ ଅକ୍ଷର *>(Mapping);
}

Error readNativeFileToEOF(file_t FileHandle, SmallVectorImpl<ଅକ୍ଷର> &Buffer,
                          ssize_t ChunkSize) {
  sandbox::violationIfEnabled();

  // Install a handler to truncate the buffer to the correct size on exit.
  size_t Size = Buffer.size();
  llvm::scope_exit TruncateOnExit([&]() { Buffer.truncate(Size); });

  // Read into Buffer until we hit EOF.
  ପାଇଁ (;;) {
    Buffer.resize_for_overwrite(Size + ChunkSize);
    Expected<size_t> ReadBytes = readNativeFile(
        FileHandle, MutableArrayRef(Buffer.begin() + Size, ChunkSize));
    ଯଦି (!ReadBytes)
      ଫେରନ୍ତୁ ReadBytes.takeError();
    ଯଦି (*ReadBytes == 0)
      ଫେରନ୍ତୁ Error::success();
    Size += *ReadBytes;
  }
}

} // end namespace fs
} // end namespace sys
} // end namespace llvm

// Include the truly platform-specific parts.
#ଯଦି defined(LLVM_ON_UNIX)
#ଅନ୍ତର୍ଭାଵ "Unix/Path.inc"
#endif
#ଯଦି defined(_WIN32)
#ଅନ୍ତର୍ଭାଵ "Windows/Path.inc"
#endif

ନାମାକାଶ llvm {
ନାମାକାଶ sys {
ନାମାକାଶ fs {

TempFile::TempFile(StringRef Name, ପୂର୍ଣ୍ଣସଂଖ୍ୟା FD)
    : TmpName(std::string(Name)), FD(FD) {}
TempFile::TempFile(TempFile &&Other) { *ଏହି = std::move(Other); }
TempFile &TempFile::operator=(TempFile &&Other) {
  TmpName = std::move(Other.TmpName);
  FD = Other.FD;
  Other.Done = ସତ୍ୟ;
  Other.FD = -1;
#ifdef _WIN32
  RemoveOnClose = Other.RemoveOnClose;
  Other.RemoveOnClose = ମିଥ୍ୟା;
#endif
  ଫେରନ୍ତୁ *ଏହି;
}

TempFile::~TempFile() { ପ୍ରତିଜ୍ଞା(Done); }

Error TempFile::discard() {
  Done = ସତ୍ୟ;
  ଯଦି (FD != -1 && close(FD) == -1) {
    std::error_code EC = errnoAsErrorCode();
    ଫେରନ୍ତୁ errorCodeToError(EC);
  }
  FD = -1;

#ifdef _WIN32
  // On Windows, closing will remove the file, if we set the delete
  // disposition. If not, remove it manually.
  ତାର୍କିକ Remove = RemoveOnClose;
#ଅନ୍ୟଥା
  // Always try to remove the file.
  ତାର୍କିକ Remove = ସତ୍ୟ;
#endif
  std::error_code RemoveEC;
  ଯଦି (Remove && !TmpName.empty()) {
    RemoveEC = fs::remove(TmpName);
    sys::DontRemoveFileOnSignal(TmpName);
    ଯଦି (!RemoveEC)
      TmpName = "";
  } ଅନ୍ୟଥା {
    TmpName = "";
  }
  ଫେରନ୍ତୁ errorCodeToError(RemoveEC);
}

Error TempFile::keep(ଅଚଳ Twine &Name) {
  ପ୍ରତିଜ୍ଞା(!Done);
  Done = ସତ୍ୟ;
  // Always try to close and rename.
#ifdef _WIN32
  // If we can't cancel the delete don't rename.
  auto H = reinterpret_cast<HANDLE>(_get_osfhandle(FD));
  std::error_code RenameEC =
      RemoveOnClose ? std::error_code() : setDeleteDisposition(H, ମିଥ୍ୟା);
  ତାର୍କିକ ShouldDelete = ମିଥ୍ୟା;
  ଯଦି (!RenameEC) {
    RenameEC = rename_handle(H, Name);
    // If rename failed because it's cross-device, copy instead
    ଯଦି (RenameEC ==
      std::error_code(ERROR_NOT_SAME_DEVICE, std::system_category())) {
      RenameEC = copy_file(TmpName, Name);
      ShouldDelete = ସତ୍ୟ;
    }
  }

  // If we can't rename or copy, discard the temporary file.
  ଯଦି (RenameEC)
    ShouldDelete = ସତ୍ୟ;
  ଯଦି (ShouldDelete) {
    ଯଦି (!RemoveOnClose)
      setDeleteDisposition(H, ସତ୍ୟ);
    ଅନ୍ୟଥା
      remove(TmpName);
  }
#ଅନ୍ୟଥା
  std::error_code RenameEC = fs::rename(TmpName, Name);
  ଯଦି (RenameEC) {
    // If we can't rename, try to copy to work around cross-device link issues.
    RenameEC = sys::fs::copy_file(TmpName, Name);
    // If we can't rename or copy, discard the temporary file.
    ଯଦି (RenameEC)
      remove(TmpName);
  }
#endif
  sys::DontRemoveFileOnSignal(TmpName);

  ଯଦି (!RenameEC)
    TmpName = "";

  ଯଦି (close(FD) == -1)
    ଫେରନ୍ତୁ errorCodeToError(errnoAsErrorCode());
  FD = -1;

  ଫେରନ୍ତୁ errorCodeToError(RenameEC);
}

Error TempFile::keep() {
  ପ୍ରତିଜ୍ଞା(!Done);
  Done = ସତ୍ୟ;

#ifdef _WIN32
  auto H = reinterpret_cast<HANDLE>(_get_osfhandle(FD));
  ଯଦି (std::error_code EC = setDeleteDisposition(H, ମିଥ୍ୟା))
    ଫେରନ୍ତୁ errorCodeToError(EC);
#endif
  sys::DontRemoveFileOnSignal(TmpName);

  TmpName = "";

  ଯଦି (close(FD) == -1)
    ଫେରନ୍ତୁ errorCodeToError(errnoAsErrorCode());
  FD = -1;

  ଫେରନ୍ତୁ Error::success();
}

Expected<TempFile> TempFile::create(ଅଚଳ Twine &Model, unsigned Mode,
                                    OpenFlags ExtraFlags) {
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା FD;
  SmallString<128> ResultPath;
  ଯଦି (std::error_code EC =
          createUniqueFile(Model, FD, ResultPath, OF_Delete | ExtraFlags, Mode))
    ଫେରନ୍ତୁ errorCodeToError(EC);

  TempFile Ret(ResultPath, FD);
#ifdef _WIN32
  auto H = reinterpret_cast<HANDLE>(_get_osfhandle(FD));
  ତାର୍କିକ SetSignalHandler = ମିଥ୍ୟା;
  ଯଦି (std::error_code EC = setDeleteDisposition(H, ସତ୍ୟ)) {
    Ret.RemoveOnClose = ସତ୍ୟ;
    SetSignalHandler = ସତ୍ୟ;
  }
#ଅନ୍ୟଥା
  ତାର୍କିକ SetSignalHandler = ସତ୍ୟ;
#endif
  ଯଦି (SetSignalHandler && sys::RemoveFileOnSignal(ResultPath)) {
    // Make sure we delete the file when RemoveFileOnSignal fails.
    consumeError(Ret.discard());
    std::error_code EC(errc::operation_not_permitted);
    ଫେରନ୍ତୁ errorCodeToError(EC);
  }
  ଫେରନ୍ତୁ std::move(Ret);
}
} // namespace fs

} // namespace sys
} // namespace llvm
