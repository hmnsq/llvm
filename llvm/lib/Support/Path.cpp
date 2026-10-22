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

#إذا !defined(_MSC_VER) && !defined(__MINGW32__)
#ضمن <unistd.h>
#وإلا
#ضمن <io.h>
#جيڪڏهن_پورو

استعمال نطاق llvm;
استعمال نطاق llvm::support::endian;

نطاق {
  استعمال llvm::StringRef;
  استعمال llvm::sys::رستو_2::is_separator;
  استعمال llvm::sys::رستو_2::Style;

  مضمن منطقي prefer_forward_slash() {
    ساكن منطقي prefer = []() {
      إذا (std::optional<std::string> Env =
              sys::Process::GetEnv("LLVM_WINDOWS_PREFER_FORWARD_SLASH"))
        أرجع *Env == "1";
      أرجع static_cast<منطقي>(LLVM_WINDOWS_PREFER_FORWARD_SLASH);
    }();
    أرجع prefer;
  }

  مضمن Style real_style(Style style) {
    إذا (style != Style::native)
      أرجع style;
    إذا (is_style_posix(style))
      أرجع Style::posix;
    أرجع prefer_forward_slash() ? Style::windows_slash
                                  : Style::windows_backslash;
  }

  مضمن ثابت حرف *separators(Style style) {
    إذا (is_style_windows(style))
      أرجع "\\/";
    أرجع "/";
  }

  مضمن حرف preferred_separator(Style style) {
    إذا (real_style(style) == Style::windows)
      أرجع '\\';
    أرجع '/';
  }

  StringRef ڳوليو_پهريون_component(StringRef رستو_2, Style style) {
    // Look for this first component in the following order.
    // * empty (in this case we return an empty string)
    // * either C: or {//,\\}net.
    // * {/,\}
    // * {file,directory}name

    إذا (رستو_2.خالي_جايون())
      أرجع رستو_2;

    إذا (is_style_windows(style)) {
      // C:
      إذا (رستو_2.سائيز_2() >= 2 &&
          std::اکر_آهي(static_cast<بنانشان حرف>(رستو_2[0])) && رستو_2[1] == ':')
        أرجع رستو_2.substr(0, 2);
    }

    // //net
    إذا ((رستو_2.سائيز_2() > 2) && is_separator(رستو_2[0], style) &&
        رستو_2[0] == رستو_2[1] && !is_separator(رستو_2[2], style)) {
      // Find the next directory separator.
      size_t پڇاڙي = رستو_2.ڳوليو_پهريون_جو(separators(style), 2);
      أرجع رستو_2.substr(0, پڇاڙي);
    }

    // {/,\}
    إذا (is_separator(رستو_2[0], style))
      أرجع رستو_2.substr(0, 1);

    // * {file,directory}name
    size_t پڇاڙي = رستو_2.ڳوليو_پهريون_جو(separators(style));
    أرجع رستو_2.substr(0, پڇاڙي);
  }

  // Returns the first character of the filename in str. For paths ending in
  // '/', it returns the position of the '/'.
  size_t filename_pos(StringRef str, Style style) {
    إذا (str.سائيز_2() > 0 && is_separator(str[str.سائيز_2() - 1], style))
      أرجع str.سائيز_2() - 1;

    size_t pos = str.ڳوليو_آخري_جو(separators(style), str.سائيز_2() - 1);

    إذا (is_style_windows(style)) {
      إذا (pos == StringRef::npos)
        pos = str.ڳوليو_آخري_جو(':', str.سائيز_2() - 1);
    }

    إذا (pos == StringRef::npos || (pos == 1 && is_separator(str[0], style)))
      أرجع 0;

    أرجع pos + 1;
  }

  // Returns the position of the root directory in str. If there is no root
  // directory in str, it returns StringRef::npos.
  size_t پاڙ_dir_شروعات(StringRef str, Style style) {
    // case "c:/"
    إذا (is_style_windows(style)) {
      إذا (str.سائيز_2() > 2 && str[1] == ':' && is_separator(str[2], style))
        أرجع 2;
    }

    // case "//net"
    إذا (str.سائيز_2() > 3 && is_separator(str[0], style) && str[0] == str[1] &&
        !is_separator(str[2], style)) {
      أرجع str.ڳوليو_پهريون_جو(separators(style), 2);
    }

    // case "/"
    إذا (str.سائيز_2() > 0 && is_separator(str[0], style))
      أرجع 0;

    أرجع StringRef::npos;
  }

  // Returns the position past the end of the "parent path" of path. The parent
  // path will not end in '/', unless the parent is the root directory. If the
  // path has no parent, 0 is returned.
  size_t والد_رستو_پڇاڙي(StringRef رستو_2, Style style) {
    size_t پڇاڙي_pos = filename_pos(رستو_2, style);

    منطقي filename_was_sep =
        رستو_2.سائيز_2() > 0 && is_separator(رستو_2[پڇاڙي_pos], style);

    // Skip separators until we reach root dir (or the start of the string).
    size_t پاڙ_dir_pos = پاڙ_dir_شروعات(رستو_2, style);
    طالما (پڇاڙي_pos > 0 &&
           (پاڙ_dir_pos == StringRef::npos || پڇاڙي_pos > پاڙ_dir_pos) &&
           is_separator(رستو_2[پڇاڙي_pos - 1], style))
      --پڇاڙي_pos;

    إذا (پڇاڙي_pos == پاڙ_dir_pos && !filename_was_sep) {
      // We've reached the root dir and the input path was *not* ending in a
      // sequence of slashes. Include the root dir in the parent path.
      أرجع پاڙ_dir_pos + 1;
    }

    // Otherwise, just include before the last slash.
    أرجع پڇاڙي_pos;
  }
} // end unnamed namespace

تعداد FSEntity {
  FS_Dir,
  FS_File,
  FS_Name
};

ساكن std::error_ڪوڊ
createUniqueEntity(ثابت Twine &Model, صحيح &نتيجو_FD,
                   Small_ويڪٽر_Impl<حرف> &نتيجو_رستو, منطقي MakeAbsolute,
                   FSEntity Type, sys::fs::OpenFlags Flags = sys::fs::جو_None,
                   بنانشان گهڻائي = 0) {

  // Limit the number of attempts we make, so that we don't infinite loop. E.g.
  // "permission denied" could be for a specific file (so we retry with a
  // different name) or for the whole directory (retry would always fail).
  // Checking which is racy, so we try a number of times, then give up.
  std::error_ڪوڊ EC;
  لكل (صحيح Retries = 128; Retries > 0; --Retries) {
    sys::fs::create_Unique_رستو(Model, نتيجو_رستو, MakeAbsolute);
    // Try to open + create the file.
    بدل (Type) {
    حالة FS_File: {
      EC = sys::fs::openFileForReadWrite(Twine(نتيجو_رستو.begin()), نتيجو_FD,
                                         sys::fs::CD_CreateNew, Flags, گهڻائي);
      إذا (EC) {
        // errc::permission_denied happens on Windows when we try to open a file
        // that has been marked for deletion.
        إذا (EC == errc::file_exists || EC == errc::permission_denied)
          استمر;
        أرجع EC;
      }

      أرجع std::error_ڪوڊ();
    }

    حالة FS_Name: {
      EC = sys::fs::access(نتيجو_رستو.begin(), sys::fs::Access_گهڻائي::Exist);
      إذا (EC == errc::no_such_file_or_directory)
        أرجع std::error_ڪوڊ();
      إذا (EC)
        أرجع EC;
      استمر;
    }

    حالة FS_Dir: {
      EC = sys::fs::create_directory(نتيجو_رستو.begin(), خطأ);
      إذا (EC) {
        إذا (EC == errc::file_exists)
          استمر;
        أرجع EC;
      }
      أرجع std::error_ڪوڊ();
    }
    }
    llvm_unreachable("Invalid Type");
  }
  أرجع EC;
}

نطاق llvm {
نطاق sys  {
نطاق رستو_2 {

const_iterator begin(StringRef رستو_2, Style style) {
  const_iterator i;
  i.رستو      = رستو_2;
  i.Component = ڳوليو_پهريون_component(رستو_2, style);
  i.Position  = 0;
  i.S = style;
  أرجع i;
}

const_iterator پڇاڙي(StringRef رستو_2) {
  const_iterator i;
  i.رستو      = رستو_2;
  i.Position  = رستو_2.سائيز_2();
  أرجع i;
}

const_iterator &const_iterator::عامل++() {
  أكد(Position < رستو.سائيز_2() && "Tried to increment past end!");

  // Increment Position to past the current component
  Position += Component.سائيز_2();

  // Check for end.
  إذا (Position == رستو.سائيز_2()) {
    Component = StringRef();
    أرجع *هذا;
  }

  // Both POSIX and Windows treat paths that begin with exactly two separators
  // specially.
  منطقي was_net = Component.سائيز_2() > 2 && is_separator(Component[0], S) &&
                 Component[1] == Component[0] && !is_separator(Component[2], S);

  // Handle separators.
  إذا (is_separator(رستو[Position], S)) {
    // Root dir.
    إذا (was_net ||
        // c:/
        (is_style_windows(S) && Component.پڇاڙيون_with(":"))) {
      Component = رستو.substr(Position, 1);
      أرجع *هذا;
    }

    // Skip extra separators.
    طالما (Position != رستو.سائيز_2() && is_separator(رستو[Position], S)) {
      ++Position;
    }

    // Treat trailing '/' as a '.', unless it is the root dir.
    إذا (Position == رستو.سائيز_2() && Component != "/") {
      --Position;
      Component = ".";
      أرجع *هذا;
    }
  }

  // Find next component.
  size_t پڇاڙي_pos = رستو.ڳوليو_پهريون_جو(separators(S), Position);
  Component = رستو.ٽڪرو(Position, پڇاڙي_pos);

  أرجع *هذا;
}

منطقي const_iterator::عامل==(ثابت const_iterator &RHS) ثابت {
  أرجع رستو.begin() == RHS.رستو.begin() && Position == RHS.Position;
}

ptrdiff_t const_iterator::عامل-(ثابت const_iterator &RHS) ثابت {
  أرجع Position - RHS.Position;
}

ابتو_iterator rbegin(StringRef رستو, Style style) {
  ابتو_iterator I;
  I.رستو = رستو;
  I.Position = رستو.سائيز_2();
  I.S = style;
  ++I;
  أرجع I;
}

ابتو_iterator rend(StringRef رستو) {
  ابتو_iterator I;
  I.رستو = رستو;
  I.Component = رستو.substr(0, 0);
  I.Position = 0;
  أرجع I;
}

ابتو_iterator &ابتو_iterator::عامل++() {
  size_t پاڙ_dir_pos = پاڙ_dir_شروعات(رستو, S);

  // Skip separators unless it's the root directory.
  size_t پڇاڙي_pos = Position;
  طالما (پڇاڙي_pos > 0 && (پڇاڙي_pos - 1) != پاڙ_dir_pos &&
         is_separator(رستو[پڇاڙي_pos - 1], S))
    --پڇاڙي_pos;

  // Treat trailing '/' as a '.', unless it is the root dir.
  إذا (Position == رستو.سائيز_2() && !رستو.خالي_جايون() &&
      is_separator(رستو.پٺ(), S) &&
      (پاڙ_dir_pos == StringRef::npos || پڇاڙي_pos - 1 > پاڙ_dir_pos)) {
    --Position;
    Component = ".";
    أرجع *هذا;
  }

  // Find next separator.
  size_t شروعات_pos = filename_pos(رستو.substr(0, پڇاڙي_pos), S);
  Component = رستو.ٽڪرو(شروعات_pos, پڇاڙي_pos);
  Position = شروعات_pos;
  أرجع *هذا;
}

منطقي ابتو_iterator::عامل==(ثابت ابتو_iterator &RHS) ثابت {
  أرجع رستو.begin() == RHS.رستو.begin() && Component == RHS.Component &&
         Position == RHS.Position;
}

ptrdiff_t ابتو_iterator::عامل-(ثابت ابتو_iterator &RHS) ثابت {
  أرجع Position - RHS.Position;
}

StringRef پاڙ_رستو_2(StringRef رستو_2, Style style) {
  const_iterator b = begin(رستو_2, style), pos = b, e = پڇاڙي(رستو_2);
  إذا (b != e) {
    منطقي has_net =
        b->سائيز_2() > 2 && is_separator((*b)[0], style) && (*b)[1] == (*b)[0];
    منطقي has_drive = is_style_windows(style) && b->پڇاڙيون_with(":");

    إذا (has_net || has_drive) {
      إذا ((++pos != e) && is_separator((*pos)[0], style)) {
        // {C:/,//net/}, so get the first two components.
        أرجع رستو_2.substr(0, b->سائيز_2() + pos->سائيز_2());
      }
      // just {C:,//net}, return the first component.
      أرجع *b;
    }

    // POSIX style root directory.
    إذا (is_separator((*b)[0], style)) {
      أرجع *b;
    }
  }

  أرجع StringRef();
}

StringRef پاڙ_name(StringRef رستو_2, Style style) {
  const_iterator b = begin(رستو_2, style), e = پڇاڙي(رستو_2);
  إذا (b != e) {
    منطقي has_net =
        b->سائيز_2() > 2 && is_separator((*b)[0], style) && (*b)[1] == (*b)[0];
    منطقي has_drive = is_style_windows(style) && b->پڇاڙيون_with(":");

    إذا (has_net || has_drive) {
      // just {C:,//net}, return the first component.
      أرجع *b;
    }
  }

  // No path or no name.
  أرجع StringRef();
}

StringRef پاڙ_directory(StringRef رستو_2, Style style) {
  const_iterator b = begin(رستو_2, style), pos = b, e = پڇاڙي(رستو_2);
  إذا (b != e) {
    منطقي has_net =
        b->سائيز_2() > 2 && is_separator((*b)[0], style) && (*b)[1] == (*b)[0];
    منطقي has_drive = is_style_windows(style) && b->پڇاڙيون_with(":");

    إذا ((has_net || has_drive) &&
        // {C:,//net}, skip to the next component.
        (++pos != e) && is_separator((*pos)[0], style)) {
      أرجع *pos;
    }

    // POSIX style root directory.
    إذا (!has_net && is_separator((*b)[0], style)) {
      أرجع *b;
    }
  }

  // No path or no root.
  أرجع StringRef();
}

StringRef relative_رستو(StringRef رستو_2, Style style) {
  StringRef پاڙ_3 = پاڙ_رستو_2(رستو_2, style);
  أرجع رستو_2.substr(پاڙ_3.سائيز_2());
}

فراغ append(Small_ويڪٽر_Impl<حرف> &رستو_2, Style style, ثابت Twine &a,
            ثابت Twine &b, ثابت Twine &c, ثابت Twine &d) {
  SmallString<32> a_storage;
  SmallString<32> b_storage;
  SmallString<32> c_storage;
  SmallString<32> d_storage;

  Small_ويڪٽر<StringRef, 4> components;
  إذا (!a.is_Trivially_خالي_جايون()) components.ڌڪو_پٺ(a.منزل_String_Ref(a_storage));
  إذا (!b.is_Trivially_خالي_جايون()) components.ڌڪو_پٺ(b.منزل_String_Ref(b_storage));
  إذا (!c.is_Trivially_خالي_جايون()) components.ڌڪو_پٺ(c.منزل_String_Ref(c_storage));
  إذا (!d.is_Trivially_خالي_جايون()) components.ڌڪو_پٺ(d.منزل_String_Ref(d_storage));

  لكل (خودڪار &component : components) {
    منطقي رستو_has_sep =
        !رستو_2.خالي_جايون() && is_separator(رستو_2[رستو_2.سائيز_2() - 1], style);
    إذا (رستو_has_sep) {
      // Strip separators from beginning of component.
      size_t loc = component.ڳوليو_پهريون_not_جو(separators(style));
      StringRef c = component.substr(loc);

      // Append it.
      رستو_2.append(c.begin(), c.پڇاڙي());
      استمر;
    }

    منطقي component_has_sep =
        !component.خالي_جايون() && is_separator(component[0], style);
    إذا (!component_has_sep &&
        !(رستو_2.خالي_جايون() || has_پاڙ_name(component, style))) {
      // Add a separator.
      رستو_2.ڌڪو_پٺ(preferred_separator(style));
    }

    رستو_2.append(component.begin(), component.پڇاڙي());
  }
}

فراغ append(Small_ويڪٽر_Impl<حرف> &رستو_2, ثابت Twine &a, ثابت Twine &b,
            ثابت Twine &c, ثابت Twine &d) {
  append(رستو_2, Style::native, a, b, c, d);
}

فراغ append(Small_ويڪٽر_Impl<حرف> &رستو_2, const_iterator begin,
            const_iterator پڇاڙي, Style style) {
  لكل (; begin != پڇاڙي; ++begin)
    رستو_2::append(رستو_2, style, *begin);
}

StringRef والد_رستو(StringRef رستو_2, Style style) {
  size_t پڇاڙي_pos = والد_رستو_پڇاڙي(رستو_2, style);
  إذا (پڇاڙي_pos == StringRef::npos)
    أرجع StringRef();
  أرجع رستو_2.substr(0, پڇاڙي_pos);
}

فراغ remove_filename(Small_ويڪٽر_Impl<حرف> &رستو_2, Style style) {
  size_t پڇاڙي_pos = والد_رستو_پڇاڙي(StringRef(رستو_2.begin(), رستو_2.سائيز_2()), style);
  إذا (پڇاڙي_pos != StringRef::npos)
    رستو_2.truncate(پڇاڙي_pos);
}

فراغ replace_extension(Small_ويڪٽر_Impl<حرف> &رستو_2, ثابت Twine &extension,
                       Style style) {
  StringRef p(رستو_2.begin(), رستو_2.سائيز_2());
  SmallString<32> ext_storage;
  StringRef ext = extension.منزل_String_Ref(ext_storage);

  // Erase existing extension.
  size_t pos = p.ڳوليو_آخري_جو('.');
  إذا (pos != StringRef::npos && pos >= filename_pos(p, style))
    رستو_2.truncate(pos);

  // Append '.' if needed.
  إذا (ext.سائيز_2() > 0 && ext[0] != '.')
    رستو_2.ڌڪو_پٺ('.');

  // Append extension.
  رستو_2.append(ext.begin(), ext.پڇاڙي());
}

ساكن منطقي starts_with(StringRef رستو, StringRef اڳياڙي,
                        Style style = Style::native) {
  // Windows prefix matching : case and separator insensitive
  إذا (is_style_windows(style)) {
    إذا (رستو.سائيز_2() < اڳياڙي.سائيز_2())
      أرجع خطأ;
    لكل (size_t I = 0, E = اڳياڙي.سائيز_2(); I != E; ++I) {
      منطقي Sep_رستو = is_separator(رستو[I], style);
      منطقي Sep_اڳياڙي = is_separator(اڳياڙي[I], style);
      إذا (Sep_رستو != Sep_اڳياڙي)
        أرجع خطأ;
      إذا (!Sep_رستو && منزل_هيٺاهون(رستو[I]) != منزل_هيٺاهون(اڳياڙي[I]))
        أرجع خطأ;
    }
    أرجع سچ;
  }
  أرجع رستو.starts_with(اڳياڙي);
}

منطقي replace_رستو_اڳياڙي(Small_ويڪٽر_Impl<حرف> &رستو, StringRef Old_اڳياڙي,
                         StringRef New_اڳياڙي, Style style) {
  إذا (Old_اڳياڙي.خالي_جايون() && New_اڳياڙي.خالي_جايون())
    أرجع خطأ;

  StringRef Orig_رستو(رستو.begin(), رستو.سائيز_2());
  إذا (!starts_with(Orig_رستو, Old_اڳياڙي, style))
    أرجع خطأ;

  // If prefixes have the same size we can simply copy the new one over.
  إذا (Old_اڳياڙي.سائيز_2() == New_اڳياڙي.سائيز_2()) {
    llvm::copy(New_اڳياڙي, رستو.begin());
    أرجع سچ;
  }

  StringRef Rel_رستو = Orig_رستو.substr(Old_اڳياڙي.سائيز_2());
  SmallString<256> New_رستو;
  (Twine(New_اڳياڙي) + Rel_رستو).منزل_ويڪٽر(New_رستو);
  رستو.مٽا_سٽا(New_رستو);
  أرجع سچ;
}

فراغ native(ثابت Twine &رستو_2, Small_ويڪٽر_Impl<حرف> &نتيجو_2, Style style) {
  أكد((!رستو_2.isSingleStringRef() ||
          رستو_2.getSingleStringRef().معلومات() != نتيجو_2.معلومات()) &&
         "path and result are not allowed to overlap!");
  // Clear result.
  نتيجو_2.clear();
  رستو_2.منزل_ويڪٽر(نتيجو_2);
  native(نتيجو_2, style);
}

std::string native(ثابت Twine &رستو_2, Style style) {
  SmallString<128> نتيجو;
  native(رستو_2, نتيجو, style);
  أرجع std::string(نتيجو);
}

فراغ native(Small_ويڪٽر_Impl<حرف> &رستو, Style style) {
  إذا (رستو.خالي_جايون())
    أرجع;
  إذا (is_style_windows(style)) {
    لكل (حرف &Ch : رستو)
      إذا (is_separator(Ch, style))
        Ch = preferred_separator(style);
    إذا (رستو[0] == '~' && (رستو.سائيز_2() == 1 || is_separator(رستو[1], style))) {
      SmallString<128> رستو_Home;
      home_directory(رستو_Home);
      رستو_Home.append(رستو.begin() + 1, رستو.پڇاڙي());
      رستو = std::move(رستو_Home);
    }
  } وإلا {
    llvm::replace(رستو, '\\', '/');
  }
}

std::string convert_منزل_slash(StringRef رستو_2, Style style) {
  إذا (is_style_posix(style))
    أرجع std::string(رستو_2);

  std::string s = رستو_2.str();
  llvm::replace(s, '\\', '/');
  أرجع s;
}

StringRef filename(StringRef رستو_2, Style style) { أرجع *rbegin(رستو_2, style); }

StringRef stem(StringRef رستو_2, Style style) {
  StringRef fname = filename(رستو_2, style);
  size_t pos = fname.ڳوليو_آخري_جو('.');
  إذا (pos == StringRef::npos)
    أرجع fname;
  إذا ((fname.سائيز_2() == 1 && fname == ".") ||
      (fname.سائيز_2() == 2 && fname == ".."))
    أرجع fname;
  أرجع fname.substr(0, pos);
}

StringRef extension(StringRef رستو_2, Style style) {
  StringRef fname = filename(رستو_2, style);
  size_t pos = fname.ڳوليو_آخري_جو('.');
  إذا (pos == StringRef::npos)
    أرجع StringRef();
  إذا ((fname.سائيز_2() == 1 && fname == ".") ||
      (fname.سائيز_2() == 2 && fname == ".."))
    أرجع StringRef();
  أرجع fname.substr(pos);
}

منطقي is_separator(حرف قدر_2, Style style) {
  إذا (قدر_2 == '/')
    أرجع سچ;
  إذا (is_style_windows(style))
    أرجع قدر_2 == '\\';
  أرجع خطأ;
}

StringRef get_separator(Style style) {
  إذا (real_style(style) == Style::windows)
    أرجع "\\";
  أرجع "/";
}

منطقي has_پاڙ_name(ثابت Twine &رستو_2, Style style) {
  SmallString<128> رستو_storage;
  StringRef p = رستو_2.منزل_String_Ref(رستو_storage);

  أرجع !پاڙ_name(p, style).خالي_جايون();
}

منطقي has_پاڙ_directory(ثابت Twine &رستو_2, Style style) {
  SmallString<128> رستو_storage;
  StringRef p = رستو_2.منزل_String_Ref(رستو_storage);

  أرجع !پاڙ_directory(p, style).خالي_جايون();
}

منطقي has_پاڙ_رستو(ثابت Twine &رستو_2, Style style) {
  SmallString<128> رستو_storage;
  StringRef p = رستو_2.منزل_String_Ref(رستو_storage);

  أرجع !پاڙ_رستو_2(p, style).خالي_جايون();
}

منطقي has_relative_رستو(ثابت Twine &رستو_2, Style style) {
  SmallString<128> رستو_storage;
  StringRef p = رستو_2.منزل_String_Ref(رستو_storage);

  أرجع !relative_رستو(p, style).خالي_جايون();
}

منطقي has_filename(ثابت Twine &رستو_2, Style style) {
  SmallString<128> رستو_storage;
  StringRef p = رستو_2.منزل_String_Ref(رستو_storage);

  أرجع !filename(p, style).خالي_جايون();
}

منطقي has_والد_رستو(ثابت Twine &رستو_2, Style style) {
  SmallString<128> رستو_storage;
  StringRef p = رستو_2.منزل_String_Ref(رستو_storage);

  أرجع !والد_رستو(p, style).خالي_جايون();
}

منطقي has_stem(ثابت Twine &رستو_2, Style style) {
  SmallString<128> رستو_storage;
  StringRef p = رستو_2.منزل_String_Ref(رستو_storage);

  أرجع !stem(p, style).خالي_جايون();
}

منطقي has_extension(ثابت Twine &رستو_2, Style style) {
  SmallString<128> رستو_storage;
  StringRef p = رستو_2.منزل_String_Ref(رستو_storage);

  أرجع !extension(p, style).خالي_جايون();
}

منطقي is_absolute(ثابت Twine &رستو_2, Style style) {
  SmallString<128> رستو_storage;
  StringRef p = رستو_2.منزل_String_Ref(رستو_storage);

  منطقي پاڙ_Dir = has_پاڙ_directory(p, style);
  منطقي پاڙ_Name = is_style_posix(style) || has_پاڙ_name(p, style);

  أرجع پاڙ_Dir && پاڙ_Name;
}

منطقي is_absolute_gnu(ثابت Twine &رستو_2, Style style) {
  SmallString<128> رستو_storage;
  StringRef p = رستو_2.منزل_String_Ref(رستو_storage);

  // Handle '/' which is absolute for both Windows and POSIX systems.
  // Handle '\\' on Windows.
  إذا (!p.خالي_جايون() && is_separator(p.front(), style))
    أرجع سچ;

  إذا (is_style_windows(style)) {
    // Handle drive letter pattern (a character followed by ':') on Windows.
    إذا (p.سائيز_2() >= 2 && (p[0] && p[1] == ':'))
      أرجع سچ;
  }

  أرجع خطأ;
}

منطقي is_relative(ثابت Twine &رستو_2, Style style) {
  أرجع !is_absolute(رستو_2, style);
}

فراغ make_absolute(ثابت Twine &current_directory,
                   Small_ويڪٽر_Impl<حرف> &رستو_2) {
  StringRef p(رستو_2.معلومات(), رستو_2.سائيز_2());

  منطقي پاڙ_Directory = has_پاڙ_directory(p);
  منطقي پاڙ_Name = has_پاڙ_name(p);

  // Already absolute.
  إذا ((پاڙ_Name || is_style_posix(Style::native)) && پاڙ_Directory)
    أرجع;

  // All the following conditions will need the current directory.
  SmallString<128> current_dir;
  current_directory.منزل_ويڪٽر(current_dir);

  // Relative path. Prepend the current directory.
  إذا (!پاڙ_Name && !پاڙ_Directory) {
    // Append path to the current directory.
    append(current_dir, p);
    // Set path to the result.
    رستو_2.مٽا_سٽا(current_dir);
    أرجع;
  }

  إذا (!پاڙ_Name && پاڙ_Directory) {
    StringRef cdrn = پاڙ_name(current_dir);
    SmallString<128> cur_Dir_پاڙ_Name(cdrn.begin(), cdrn.پڇاڙي());
    append(cur_Dir_پاڙ_Name, p);
    // Set path to the result.
    رستو_2.مٽا_سٽا(cur_Dir_پاڙ_Name);
    أرجع;
  }

  إذا (پاڙ_Name && !پاڙ_Directory) {
    StringRef p_پاڙ_Name = پاڙ_name(p);
    StringRef b_پاڙ_Directory = پاڙ_directory(current_dir);
    StringRef b_Relative_رستو = relative_رستو(current_dir);
    StringRef p_Relative_رستو = relative_رستو(p);

    SmallString<128> res;
    append(res, p_پاڙ_Name, b_پاڙ_Directory, b_Relative_رستو, p_Relative_رستو);
    رستو_2.مٽا_سٽا(res);
    أرجع;
  }

  llvm_unreachable("All rootName and rootDirectory combinations should have "
                   "occurred above!");
}

StringRef remove_leading_dotslash(StringRef رستو, Style style) {
  // Remove leading "./" (or ".//" or "././" etc.)
  طالما (رستو.سائيز_2() > 2 && رستو[0] == '.' && is_separator(رستو[1], style)) {
    رستو = رستو.substr(2);
    طالما (رستو.سائيز_2() > 0 && is_separator(رستو[0], style))
      رستو = رستو.substr(1);
  }
  أرجع رستو;
}

منطقي remove_dots(Small_ويڪٽر_Impl<حرف> &the_رستو, منطقي remove_dot_dot,
                 Style style) {
  style = real_style(style);
  StringRef بچيل_2(the_رستو.معلومات(), the_رستو.سائيز_2());
  منطقي needs_change = خطأ;
  Small_ويڪٽر<StringRef, 16> components;

  // Consume the root path, if present.
  StringRef پاڙ_3 = رستو_2::پاڙ_رستو_2(بچيل_2, style);
  منطقي absolute = !پاڙ_3.خالي_جايون();
  إذا (absolute)
    بچيل_2 = بچيل_2.drop_front(پاڙ_3.سائيز_2());

  // Loop over path components manually. This makes it easier to detect
  // non-preferred slashes and double separators that must be canonicalized.
  طالما (!بچيل_2.خالي_جايون()) {
    size_t ايندڙ_slash = بچيل_2.ڳوليو_پهريون_جو(separators(style));
    إذا (ايندڙ_slash == StringRef::npos)
      ايندڙ_slash = بچيل_2.سائيز_2();
    StringRef component = بچيل_2.take_front(ايندڙ_slash);
    بچيل_2 = بچيل_2.drop_front(ايندڙ_slash);

    // Eat the slash, and check if it is the preferred separator.
    إذا (!بچيل_2.خالي_جايون()) {
      needs_change |= بچيل_2.front() != preferred_separator(style);
      بچيل_2 = بچيل_2.drop_front();
      // The path needs to be rewritten if it has a trailing slash.
      // FIXME: This is emergent behavior that could be removed.
      needs_change |= بچيل_2.خالي_جايون();
    }

    // Check for path traversal components or double separators.
    إذا (component.خالي_جايون() || component == ".") {
      needs_change = سچ;
    } وإلا إذا (remove_dot_dot && component == "..") {
      needs_change = سچ;
      // Do not allow ".." to remove the root component. If this is the
      // beginning of a relative path, keep the ".." component.
      إذا (!components.خالي_جايون() && components.پٺ() != "..") {
        components.ڪڍو_پٺ();
      } وإلا إذا (!absolute) {
        components.ڌڪو_پٺ(component);
      }
    } وإلا {
      components.ڌڪو_پٺ(component);
    }
  }

  SmallString<256> بفر_2 = پاڙ_3;
  // "root" could be "/", which may need to be translated into "\".
  make_preferred(بفر_2, style);
  needs_change |= پاڙ_3 != بفر_2;

  // Avoid rewriting the path unless we have to.
  إذا (!needs_change)
    أرجع خطأ;

  إذا (!components.خالي_جايون()) {
    بفر_2 += components[0];
    لكل (StringRef C : ArrayRef(components).drop_front()) {
      بفر_2 += preferred_separator(style);
      بفر_2 += C;
    }
  }
  the_رستو.مٽا_سٽا(بفر_2);
  أرجع سچ;
}

} // end namespace path

نطاق fs {

std::error_ڪوڊ getUniqueID(ثابت Twine رستو, UniqueID &نتيجو) {
  sandbox::violationIfEnabled();

  file_حالت حالت;
  std::error_ڪوڊ EC = حالت_2(رستو, حالت);
  إذا (EC)
    أرجع EC;
  نتيجو = حالت.getUniqueID();
  أرجع std::error_ڪوڊ();
}

فراغ create_Unique_رستو(ثابت Twine &Model, Small_ويڪٽر_Impl<حرف> &نتيجو_رستو,
                      منطقي MakeAbsolute) {
  SmallString<128> ModelStorage;
  Model.منزل_ويڪٽر(ModelStorage);

  أكد(llvm::is_contained(ModelStorage, '%') &&
         "createUniquePath: Model must contain at least one '%'");

  إذا (MakeAbsolute) {
    // Make model absolute by prepending a temp directory if it's not already.
    إذا (!sys::رستو_2::is_absolute(Twine(ModelStorage))) {
      SmallString<128> TDir;
      sys::رستو_2::system_عارضي_directory(سچ, TDir);
      sys::رستو_2::append(TDir, Twine(ModelStorage));
      ModelStorage.مٽا_سٽا(TDir);
    }
  }

  نتيجو_رستو = ModelStorage;
  نتيجو_رستو.ڌڪو_پٺ(0);
  نتيجو_رستو.ڪڍو_پٺ();

  // Replace '%' with random chars.
  لكل (بنانشان i = 0, e = ModelStorage.سائيز_2(); i != e; ++i) {
    إذا (ModelStorage[i] == '%')
      نتيجو_رستو[i] = "0123456789abcdef"[sys::Process::Get_Random_عدد() & 15];
  }
}

std::error_ڪوڊ createUniqueFile(ثابت Twine &Model, صحيح &نتيجو_Fd,
                                 Small_ويڪٽر_Impl<حرف> &نتيجو_رستو,
                                 OpenFlags Flags, بنانشان گهڻائي) {
  أرجع createUniqueEntity(Model, نتيجو_Fd, نتيجو_رستو, خطأ, FS_File, Flags,
                            گهڻائي);
}

std::error_ڪوڊ createUniqueFile(ثابت Twine &Model,
                                 Small_ويڪٽر_Impl<حرف> &نتيجو_رستو,
                                 بنانشان گهڻائي) {
  صحيح FD;
  خودڪار EC = createUniqueFile(Model, FD, نتيجو_رستو, جو_None, گهڻائي);
  إذا (EC)
    أرجع EC;
  // FD is only needed to avoid race conditions. Close it right away.
  close(FD);
  أرجع EC;
}

ساكن std::error_ڪوڊ
createTemporaryFile(ثابت Twine &Model, صحيح &نتيجو_FD,
                    llvm::Small_ويڪٽر_Impl<حرف> &نتيجو_رستو, FSEntity Type,
                    sys::fs::OpenFlags Flags = sys::fs::جو_None) {
  // Any *temporary* file is assumed to be a compiler-internal output, not
  // a formal one.
  خودڪار BypassSandbox = sys::sandbox::scopedDisable();

  SmallString<128> Storage;
  StringRef P = Model.منزل_Null_Terminated_String_Ref(Storage);
  أكد(P.ڳوليو_پهريون_جو(separators(Style::native)) == StringRef::npos &&
         "Model must be a simple filename.");
  // Use P.begin() so that createUniqueEntity doesn't need to recreate Storage.
  أرجع createUniqueEntity(P.begin(), نتيجو_FD, نتيجو_رستو, سچ, Type, Flags,
                            all_read | all_write);
}

ساكن std::error_ڪوڊ
createTemporaryFile(ثابت Twine &اڳياڙي, StringRef Suffix, صحيح &نتيجو_FD,
                    llvm::Small_ويڪٽر_Impl<حرف> &نتيجو_رستو, FSEntity Type,
                    sys::fs::OpenFlags Flags = sys::fs::جو_None) {
  ثابت حرف *وچ = Suffix.خالي_جايون() ? "-%%%%%%" : "-%%%%%%.";
  أرجع createTemporaryFile(اڳياڙي + وچ + Suffix, نتيجو_FD, نتيجو_رستو,
                             Type, Flags);
}

std::error_ڪوڊ createTemporaryFile(ثابت Twine &اڳياڙي, StringRef Suffix,
                                    صحيح &نتيجو_FD,
                                    Small_ويڪٽر_Impl<حرف> &نتيجو_رستو,
                                    sys::fs::OpenFlags Flags) {
  أرجع createTemporaryFile(اڳياڙي, Suffix, نتيجو_FD, نتيجو_رستو, FS_File,
                             Flags);
}

std::error_ڪوڊ createTemporaryFile(ثابت Twine &اڳياڙي, StringRef Suffix,
                                    Small_ويڪٽر_Impl<حرف> &نتيجو_رستو,
                                    sys::fs::OpenFlags Flags) {
  صحيح FD;
  خودڪار EC = createTemporaryFile(اڳياڙي, Suffix, FD, نتيجو_رستو, Flags);
  إذا (EC)
    أرجع EC;
  // FD is only needed to avoid race conditions. Close it right away.
  close(FD);
  أرجع EC;
}

// This is a mkdtemp with a different pattern. We use createUniqueEntity mostly
// for consistency. We should try using mkdtemp.
std::error_ڪوڊ createUniqueDirectory(ثابت Twine &اڳياڙي,
                                      Small_ويڪٽر_Impl<حرف> &نتيجو_رستو) {
  صحيح Dummy;
  أرجع createUniqueEntity(اڳياڙي + "-%%%%%%", Dummy, نتيجو_رستو, سچ,
                            FS_Dir);
}

std::error_ڪوڊ
getPotentiallyUniqueFileName(ثابت Twine &Model,
                             Small_ويڪٽر_Impl<حرف> &نتيجو_رستو) {
  صحيح Dummy;
  أرجع createUniqueEntity(Model, Dummy, نتيجو_رستو, خطأ, FS_Name);
}

std::error_ڪوڊ
get_Potentially_Unique_عارضي_File_Name(ثابت Twine &اڳياڙي, StringRef Suffix,
                                 Small_ويڪٽر_Impl<حرف> &نتيجو_رستو) {
  صحيح Dummy;
  أرجع createTemporaryFile(اڳياڙي, Suffix, Dummy, نتيجو_رستو, FS_Name);
}

std::error_ڪوڊ make_absolute(Small_ويڪٽر_Impl<حرف> &رستو_2) {
  sandbox::violationIfEnabled();

  إذا (رستو_2::is_absolute(رستو_2))
    أرجع {};

  SmallString<128> current_dir;
  إذا (std::error_ڪوڊ ec = current_رستو(current_dir))
    أرجع ec;

  رستو_2::make_absolute(current_dir, رستو_2);
  أرجع {};
}

std::error_ڪوڊ create_directories(ثابت Twine &رستو, منطقي IgnoreExisting,
                                   perms Perms) {
  SmallString<128> رستو_Storage;
  StringRef P = رستو.منزل_String_Ref(رستو_Storage);

  // Be optimistic and try to create the directory
  std::error_ڪوڊ EC = create_directory(P, IgnoreExisting, Perms);
  // If we succeeded, or had any error other than the parent not existing, just
  // return it.
  إذا (EC != errc::no_such_file_or_directory)
    أرجع EC;

  // We failed because of a no_such_file_or_directory, try to create the
  // parent.
  StringRef والد_2 = رستو_2::والد_رستو(P);
  إذا (والد_2.خالي_جايون())
    أرجع EC;

  إذا ((EC = create_directories(والد_2, IgnoreExisting, Perms)))
      أرجع EC;

  أرجع create_directory(P, IgnoreExisting, Perms);
}

ساكن std::error_ڪوڊ copy_file_اندروني(صحيح ReadFD, صحيح WriteFD) {
  ثابت size_t Buf_سائيز = 4096;
  حرف *Buf = جديد حرف[Buf_سائيز];
  صحيح بائيٽون_Read = 0, بائيٽون_Written = 0;
  لكل (;;) {
    بائيٽون_Read = اقرأ(ReadFD, Buf, Buf_سائيز);
    إذا (بائيٽون_Read <= 0)
      اكسر;
    طالما (بائيٽون_Read) {
      بائيٽون_Written = اكتب(WriteFD, Buf, بائيٽون_Read);
      إذا (بائيٽون_Written < 0)
        اكسر;
      بائيٽون_Read -= بائيٽون_Written;
    }
    إذا (بائيٽون_Written < 0)
      اكسر;
  }
  ميٽيو[] Buf;

  إذا (بائيٽون_Read < 0 || بائيٽون_Written < 0)
    أرجع errno_As_Error_ڪوڊ();
  أرجع std::error_ڪوڊ();
}

#جيڪڏهن_وصف_نه_ٿيل __APPLE__
std::error_ڪوڊ copy_file(ثابت Twine &From, ثابت Twine &منزل) {
  صحيح ReadFD, WriteFD;
  إذا (std::error_ڪوڊ EC = openFileForRead(From, ReadFD, جو_None))
    أرجع EC;
  إذا (std::error_ڪوڊ EC =
          openFileForWrite(منزل, WriteFD, CD_CreateAlways, جو_None)) {
    close(ReadFD);
    أرجع EC;
  }

  std::error_ڪوڊ EC = copy_file_اندروني(ReadFD, WriteFD);

  close(ReadFD);
  close(WriteFD);

  أرجع EC;
}
#جيڪڏهن_پورو

std::error_ڪوڊ copy_file(ثابت Twine &From, صحيح منزل_FD) {
  صحيح ReadFD;
  إذا (std::error_ڪوڊ EC = openFileForRead(From, ReadFD, جو_None))
    أرجع EC;

  std::error_ڪوڊ EC = copy_file_اندروني(ReadFD, منزل_FD);

  close(ReadFD);

  أرجع EC;
}

ErrorOr<MD5::MD5_نتيجو> md5_contents(صحيح FD) {
  sandbox::violationIfEnabled();

  MD5 Hash;

  constexpr size_t Buf_سائيز = 4096;
  std::ويڪٽر<uint8_t> Buf(Buf_سائيز);
  صحيح بائيٽون_Read = 0;
  لكل (;;) {
    بائيٽون_Read = اقرأ(FD, Buf.معلومات(), Buf_سائيز);
    إذا (بائيٽون_Read <= 0)
      اكسر;
    Hash.update(ArrayRef(Buf.معلومات(), بائيٽون_Read));
  }

  إذا (بائيٽون_Read < 0)
    أرجع errno_As_Error_ڪوڊ();
  MD5::MD5_نتيجو نتيجو;
  Hash.final(نتيجو);
  أرجع نتيجو;
}

ErrorOr<MD5::MD5_نتيجو> md5_contents(ثابت Twine &رستو) {
  sandbox::violationIfEnabled();

  صحيح FD;
  إذا (خودڪار EC = openFileForRead(رستو, FD, جو_None))
    أرجع EC;

  خودڪار نتيجو = md5_contents(FD);
  close(FD);
  أرجع نتيجو;
}

منطقي exists(ثابت basic_file_حالت &حالت_2) {
  أرجع حالت_معلوم(حالت_2) && حالت_2.type() != file_type::file_not_مليو;
}

منطقي حالت_معلوم(ثابت basic_file_حالت &s) {
  أرجع s.type() != file_type::حالت_error;
}

file_type get_file_type(ثابت Twine &رستو, منطقي Follow) {
  file_حالت st;
  إذا (حالت_2(رستو, st, Follow))
    أرجع file_type::حالت_error;
  أرجع st.type();
}

منطقي is_directory(ثابت basic_file_حالت &حالت_2) {
  أرجع حالت_2.type() == file_type::directory_file;
}

std::error_ڪوڊ is_directory(ثابت Twine &رستو_2, منطقي &نتيجو_2) {
  sandbox::violationIfEnabled();

  file_حالت st;
  إذا (std::error_ڪوڊ ec = حالت_2(رستو_2, st))
    أرجع ec;
  نتيجو_2 = is_directory(st);
  أرجع std::error_ڪوڊ();
}

منطقي is_regular_file(ثابت basic_file_حالت &حالت_2) {
  أرجع حالت_2.type() == file_type::regular_file;
}

std::error_ڪوڊ is_regular_file(ثابت Twine &رستو_2, منطقي &نتيجو_2) {
  sandbox::violationIfEnabled();

  file_حالت st;
  إذا (std::error_ڪوڊ ec = حالت_2(رستو_2, st))
    أرجع ec;
  نتيجو_2 = is_regular_file(st);
  أرجع std::error_ڪوڊ();
}

منطقي is_symlink_file(ثابت basic_file_حالت &حالت_2) {
  أرجع حالت_2.type() == file_type::symlink_file;
}

std::error_ڪوڊ is_symlink_file(ثابت Twine &رستو_2, منطقي &نتيجو_2) {
  sandbox::violationIfEnabled();

  file_حالت st;
  إذا (std::error_ڪوڊ ec = حالت_2(رستو_2, st, خطأ))
    أرجع ec;
  نتيجو_2 = is_symlink_file(st);
  أرجع std::error_ڪوڊ();
}

منطقي is_other(ثابت basic_file_حالت &حالت_2) {
  أرجع exists(حالت_2) &&
         !is_regular_file(حالت_2) &&
         !is_directory(حالت_2);
}

std::error_ڪوڊ is_other(ثابت Twine &رستو, منطقي &نتيجو) {
  sandbox::violationIfEnabled();

  file_حالت File_حالت;
  إذا (std::error_ڪوڊ EC = حالت_2(رستو, File_حالت))
    أرجع EC;
  نتيجو = is_other(File_حالت);
  أرجع std::error_ڪوڊ();
}

فراغ directory_داخلا::replace_filename(ثابت Twine &Filename, file_type Type,
                                       basic_file_حالت حالت) {
  SmallString<128> رستو_Str = رستو_2::والد_رستو(رستو);
  رستو_2::append(رستو_Str, Filename);
  هذا->رستو = std::string(رستو_Str);
  هذا->Type = Type;
  هذا->حالت = حالت;
}

ErrorOr<perms> getPermissions(ثابت Twine &رستو) {
  sandbox::violationIfEnabled();

  file_حالت حالت;
  إذا (std::error_ڪوڊ EC = حالت_2(رستو, حالت))
    أرجع EC;

  أرجع حالت.permissions();
}

std::error_ڪوڊ سيٽ_آخري_Access_And_Modification_وقت(ثابت Twine &رستو,
                                                 وقت_نقطو<> Access_وقت,
                                                 وقت_نقطو<> Modification_وقت) {
  صحيح FD;
  إذا (std::error_ڪوڊ EC = openFile(رستو, FD, CD_OpenExisting, FA_Read,
                                    جو_Update_خاصيتون | جو_Open_Directory))
    أرجع EC;
  std::error_ڪوڊ EC =
      سيٽ_آخري_Access_And_Modification_وقت(FD, Access_وقت, Modification_وقت);
  Process::SafelyCloseFileDescriptor(FD);
  أرجع EC;
}

size_t mapped_file_region::سائيز_2() ثابت {
  أكد(Mapping && "Mapping failed but used anyway!");
  أرجع سائيز;
}

حرف *mapped_file_region::معلومات() ثابت {
  أكد(Mapping && "Mapping failed but used anyway!");
  أرجع reinterpret_cast<حرف *>(Mapping);
}

ثابت حرف *mapped_file_region::const_معلومات() ثابت {
  أكد(Mapping && "Mapping failed but used anyway!");
  أرجع reinterpret_cast<ثابت حرف *>(Mapping);
}

Error read_Native_File_منزل_EOF(file_t FileHandle, Small_ويڪٽر_Impl<حرف> &بفر,
                          ssize_t Chunk_سائيز) {
  sandbox::violationIfEnabled();

  // Install a handler to truncate the buffer to the correct size on exit.
  size_t سائيز = بفر.سائيز_2();
  llvm::scope_exit TruncateOnExit([&]() { بفر.truncate(سائيز); });

  // Read into Buffer until we hit EOF.
  لكل (;;) {
    بفر.resize_for_overwrite(سائيز + Chunk_سائيز);
    Expected<size_t> Read_بائيٽون = readNativeFile(
        FileHandle, MutableArrayRef(بفر.begin() + سائيز, Chunk_سائيز));
    إذا (!Read_بائيٽون)
      أرجع Read_بائيٽون.takeError();
    إذا (*Read_بائيٽون == 0)
      أرجع Error::success();
    سائيز += *Read_بائيٽون;
  }
}

} // end namespace fs
} // end namespace sys
} // end namespace llvm

// Include the truly platform-specific parts.
#إذا defined(LLVM_ON_UNIX)
#ضمن "Unix/Path.inc"
#جيڪڏهن_پورو
#إذا defined(_WIN32)
#ضمن "Windows/Path.inc"
#جيڪڏهن_پورو

نطاق llvm {
نطاق sys {
نطاق fs {

عارضي_File::عارضي_File(StringRef Name, صحيح FD)
    : TmpName(std::string(Name)), FD(FD) {}
عارضي_File::عارضي_File(عارضي_File &&Other) { *هذا = std::move(Other); }
عارضي_File &عارضي_File::عامل=(عارضي_File &&Other) {
  TmpName = std::move(Other.TmpName);
  FD = Other.FD;
  Other.پورو = سچ;
  Other.FD = -1;
#جيڪڏهن_وصف_ٿيل _WIN32
  RemoveOnClose = Other.RemoveOnClose;
  Other.RemoveOnClose = خطأ;
#جيڪڏهن_پورو
  أرجع *هذا;
}

عارضي_File::~عارضي_File() { أكد(پورو); }

Error عارضي_File::discard() {
  پورو = سچ;
  إذا (FD != -1 && close(FD) == -1) {
    std::error_ڪوڊ EC = errno_As_Error_ڪوڊ();
    أرجع error_ڪوڊ_منزل_Error(EC);
  }
  FD = -1;

#جيڪڏهن_وصف_ٿيل _WIN32
  // On Windows, closing will remove the file, if we set the delete
  // disposition. If not, remove it manually.
  منطقي Remove = RemoveOnClose;
#وإلا
  // Always try to remove the file.
  منطقي Remove = سچ;
#جيڪڏهن_پورو
  std::error_ڪوڊ RemoveEC;
  إذا (Remove && !TmpName.خالي_جايون()) {
    RemoveEC = fs::remove(TmpName);
    sys::DontRemoveFileOnSignal(TmpName);
    إذا (!RemoveEC)
      TmpName = "";
  } وإلا {
    TmpName = "";
  }
  أرجع error_ڪوڊ_منزل_Error(RemoveEC);
}

Error عارضي_File::keep(ثابت Twine &Name) {
  أكد(!پورو);
  پورو = سچ;
  // Always try to close and rename.
#جيڪڏهن_وصف_ٿيل _WIN32
  // If we can't cancel the delete don't rename.
  خودڪار H = reinterpret_cast<HANDLE>(_get_osfhandle(FD));
  std::error_ڪوڊ RenameEC =
      RemoveOnClose ? std::error_ڪوڊ() : سيٽ_Delete_Disposition(H, خطأ);
  منطقي ShouldDelete = خطأ;
  إذا (!RenameEC) {
    RenameEC = rename_handle(H, Name);
    // If rename failed because it's cross-device, copy instead
    إذا (RenameEC ==
      std::error_ڪوڊ(ERROR_NOT_SAME_DEVICE, std::system_category())) {
      RenameEC = copy_file(TmpName, Name);
      ShouldDelete = سچ;
    }
  }

  // If we can't rename or copy, discard the temporary file.
  إذا (RenameEC)
    ShouldDelete = سچ;
  إذا (ShouldDelete) {
    إذا (!RemoveOnClose)
      سيٽ_Delete_Disposition(H, سچ);
    وإلا
      remove(TmpName);
  }
#وإلا
  std::error_ڪوڊ RenameEC = fs::rename(TmpName, Name);
  إذا (RenameEC) {
    // If we can't rename, try to copy to work around cross-device link issues.
    RenameEC = sys::fs::copy_file(TmpName, Name);
    // If we can't rename or copy, discard the temporary file.
    إذا (RenameEC)
      remove(TmpName);
  }
#جيڪڏهن_پورو
  sys::DontRemoveFileOnSignal(TmpName);

  إذا (!RenameEC)
    TmpName = "";

  إذا (close(FD) == -1)
    أرجع error_ڪوڊ_منزل_Error(errno_As_Error_ڪوڊ());
  FD = -1;

  أرجع error_ڪوڊ_منزل_Error(RenameEC);
}

Error عارضي_File::keep() {
  أكد(!پورو);
  پورو = سچ;

#جيڪڏهن_وصف_ٿيل _WIN32
  خودڪار H = reinterpret_cast<HANDLE>(_get_osfhandle(FD));
  إذا (std::error_ڪوڊ EC = سيٽ_Delete_Disposition(H, خطأ))
    أرجع error_ڪوڊ_منزل_Error(EC);
#جيڪڏهن_پورو
  sys::DontRemoveFileOnSignal(TmpName);

  TmpName = "";

  إذا (close(FD) == -1)
    أرجع error_ڪوڊ_منزل_Error(errno_As_Error_ڪوڊ());
  FD = -1;

  أرجع Error::success();
}

Expected<عارضي_File> عارضي_File::create(ثابت Twine &Model, بنانشان گهڻائي,
                                    OpenFlags ExtraFlags) {
  صحيح FD;
  SmallString<128> نتيجو_رستو;
  إذا (std::error_ڪوڊ EC =
          createUniqueFile(Model, FD, نتيجو_رستو, جو_Delete | ExtraFlags, گهڻائي))
    أرجع error_ڪوڊ_منزل_Error(EC);

  عارضي_File Ret(نتيجو_رستو, FD);
#جيڪڏهن_وصف_ٿيل _WIN32
  خودڪار H = reinterpret_cast<HANDLE>(_get_osfhandle(FD));
  منطقي سيٽ_Signal_Handler = خطأ;
  إذا (std::error_ڪوڊ EC = سيٽ_Delete_Disposition(H, سچ)) {
    Ret.RemoveOnClose = سچ;
    سيٽ_Signal_Handler = سچ;
  }
#وإلا
  منطقي سيٽ_Signal_Handler = سچ;
#جيڪڏهن_پورو
  إذا (سيٽ_Signal_Handler && sys::RemoveFileOnSignal(نتيجو_رستو)) {
    // Make sure we delete the file when RemoveFileOnSignal fails.
    consumeError(Ret.discard());
    std::error_ڪوڊ EC(errc::operation_not_permitted);
    أرجع error_ڪوڊ_منزل_Error(EC);
  }
  أرجع std::move(Ret);
}
} // namespace fs

} // namespace sys
} // namespace llvm
