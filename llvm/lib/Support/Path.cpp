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
#پایان_اگر

استفاده نطاق llvm;
استفاده نطاق llvm::support::endian;

نطاق {
  استفاده llvm::StringRef;
  استفاده llvm::sys::مسیر_2::is_separator;
  استفاده llvm::sys::مسیر_2::Style;

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

  StringRef پیدا_کن_اولی_component(StringRef مسیر_2, Style style) {
    // Look for this first component in the following order.
    // * empty (in this case we return an empty string)
    // * either C: or {//,\\}net.
    // * {/,\}
    // * {file,directory}name

    إذا (مسیر_2.خالی())
      أرجع مسیر_2;

    إذا (is_style_windows(style)) {
      // C:
      إذا (مسیر_2.اندازه_2() >= 2 &&
          std::آیا_حرف(static_cast<بیعلامت حرف>(مسیر_2[0])) && مسیر_2[1] == ':')
        أرجع مسیر_2.substr(0, 2);
    }

    // //net
    إذا ((مسیر_2.اندازه_2() > 2) && is_separator(مسیر_2[0], style) &&
        مسیر_2[0] == مسیر_2[1] && !is_separator(مسیر_2[2], style)) {
      // Find the next directory separator.
      size_t انتها = مسیر_2.پیدا_کن_اولی_از(separators(style), 2);
      أرجع مسیر_2.substr(0, انتها);
    }

    // {/,\}
    إذا (is_separator(مسیر_2[0], style))
      أرجع مسیر_2.substr(0, 1);

    // * {file,directory}name
    size_t انتها = مسیر_2.پیدا_کن_اولی_از(separators(style));
    أرجع مسیر_2.substr(0, انتها);
  }

  // Returns the first character of the filename in str. For paths ending in
  // '/', it returns the position of the '/'.
  size_t filename_pos(StringRef str, Style style) {
    إذا (str.اندازه_2() > 0 && is_separator(str[str.اندازه_2() - 1], style))
      أرجع str.اندازه_2() - 1;

    size_t pos = str.پیدا_کن_آخرین_از(separators(style), str.اندازه_2() - 1);

    إذا (is_style_windows(style)) {
      إذا (pos == StringRef::npos)
        pos = str.پیدا_کن_آخرین_از(':', str.اندازه_2() - 1);
    }

    إذا (pos == StringRef::npos || (pos == 1 && is_separator(str[0], style)))
      أرجع 0;

    أرجع pos + 1;
  }

  // Returns the position of the root directory in str. If there is no root
  // directory in str, it returns StringRef::npos.
  size_t ریشه_dir_شروع(StringRef str, Style style) {
    // case "c:/"
    إذا (is_style_windows(style)) {
      إذا (str.اندازه_2() > 2 && str[1] == ':' && is_separator(str[2], style))
        أرجع 2;
    }

    // case "//net"
    إذا (str.اندازه_2() > 3 && is_separator(str[0], style) && str[0] == str[1] &&
        !is_separator(str[2], style)) {
      أرجع str.پیدا_کن_اولی_از(separators(style), 2);
    }

    // case "/"
    إذا (str.اندازه_2() > 0 && is_separator(str[0], style))
      أرجع 0;

    أرجع StringRef::npos;
  }

  // Returns the position past the end of the "parent path" of path. The parent
  // path will not end in '/', unless the parent is the root directory. If the
  // path has no parent, 0 is returned.
  size_t والد_مسیر_انتها(StringRef مسیر_2, Style style) {
    size_t انتها_pos = filename_pos(مسیر_2, style);

    منطقي filename_was_sep =
        مسیر_2.اندازه_2() > 0 && is_separator(مسیر_2[انتها_pos], style);

    // Skip separators until we reach root dir (or the start of the string).
    size_t ریشه_dir_pos = ریشه_dir_شروع(مسیر_2, style);
    طالما (انتها_pos > 0 &&
           (ریشه_dir_pos == StringRef::npos || انتها_pos > ریشه_dir_pos) &&
           is_separator(مسیر_2[انتها_pos - 1], style))
      --انتها_pos;

    إذا (انتها_pos == ریشه_dir_pos && !filename_was_sep) {
      // We've reached the root dir and the input path was *not* ending in a
      // sequence of slashes. Include the root dir in the parent path.
      أرجع ریشه_dir_pos + 1;
    }

    // Otherwise, just include before the last slash.
    أرجع انتها_pos;
  }
} // end unnamed namespace

تعداد FSEntity {
  FS_Dir,
  FS_File,
  FS_Name
};

ساكن std::error_کد
createUniqueEntity(ثابت Twine &Model, صحيح &نتیجه_FD,
                   Small_وکتور_Impl<حرف> &نتیجه_مسیر, منطقي MakeAbsolute,
                   FSEntity Type, sys::fs::OpenFlags Flags = sys::fs::از_None,
                   بیعلامت نما = 0) {

  // Limit the number of attempts we make, so that we don't infinite loop. E.g.
  // "permission denied" could be for a specific file (so we retry with a
  // different name) or for the whole directory (retry would always fail).
  // Checking which is racy, so we try a number of times, then give up.
  std::error_کد EC;
  لكل (صحيح Retries = 128; Retries > 0; --Retries) {
    sys::fs::create_Unique_مسیر(Model, نتیجه_مسیر, MakeAbsolute);
    // Try to open + create the file.
    بدل (Type) {
    حالة FS_File: {
      EC = sys::fs::openFileForReadWrite(Twine(نتیجه_مسیر.begin()), نتیجه_FD,
                                         sys::fs::CD_CreateNew, Flags, نما);
      إذا (EC) {
        // errc::permission_denied happens on Windows when we try to open a file
        // that has been marked for deletion.
        إذا (EC == errc::file_exists || EC == errc::permission_denied)
          استمر;
        أرجع EC;
      }

      أرجع std::error_کد();
    }

    حالة FS_Name: {
      EC = sys::fs::access(نتیجه_مسیر.begin(), sys::fs::Access_نما::Exist);
      إذا (EC == errc::no_such_file_or_directory)
        أرجع std::error_کد();
      إذا (EC)
        أرجع EC;
      استمر;
    }

    حالة FS_Dir: {
      EC = sys::fs::create_directory(نتیجه_مسیر.begin(), خطأ);
      إذا (EC) {
        إذا (EC == errc::file_exists)
          استمر;
        أرجع EC;
      }
      أرجع std::error_کد();
    }
    }
    llvm_unreachable("Invalid Type");
  }
  أرجع EC;
}

نطاق llvm {
نطاق sys  {
نطاق مسیر_2 {

const_iterator begin(StringRef مسیر_2, Style style) {
  const_iterator i;
  i.مسیر      = مسیر_2;
  i.Component = پیدا_کن_اولی_component(مسیر_2, style);
  i.Position  = 0;
  i.S = style;
  أرجع i;
}

const_iterator انتها(StringRef مسیر_2) {
  const_iterator i;
  i.مسیر      = مسیر_2;
  i.Position  = مسیر_2.اندازه_2();
  أرجع i;
}

const_iterator &const_iterator::عملگر++() {
  أكد(Position < مسیر.اندازه_2() && "Tried to increment past end!");

  // Increment Position to past the current component
  Position += Component.اندازه_2();

  // Check for end.
  إذا (Position == مسیر.اندازه_2()) {
    Component = StringRef();
    أرجع *هذا;
  }

  // Both POSIX and Windows treat paths that begin with exactly two separators
  // specially.
  منطقي was_net = Component.اندازه_2() > 2 && is_separator(Component[0], S) &&
                 Component[1] == Component[0] && !is_separator(Component[2], S);

  // Handle separators.
  إذا (is_separator(مسیر[Position], S)) {
    // Root dir.
    إذا (was_net ||
        // c:/
        (is_style_windows(S) && Component.دوسر_with(":"))) {
      Component = مسیر.substr(Position, 1);
      أرجع *هذا;
    }

    // Skip extra separators.
    طالما (Position != مسیر.اندازه_2() && is_separator(مسیر[Position], S)) {
      ++Position;
    }

    // Treat trailing '/' as a '.', unless it is the root dir.
    إذا (Position == مسیر.اندازه_2() && Component != "/") {
      --Position;
      Component = ".";
      أرجع *هذا;
    }
  }

  // Find next component.
  size_t انتها_pos = مسیر.پیدا_کن_اولی_از(separators(S), Position);
  Component = مسیر.برش(Position, انتها_pos);

  أرجع *هذا;
}

منطقي const_iterator::عملگر==(ثابت const_iterator &RHS) ثابت {
  أرجع مسیر.begin() == RHS.مسیر.begin() && Position == RHS.Position;
}

ptrdiff_t const_iterator::عملگر-(ثابت const_iterator &RHS) ثابت {
  أرجع Position - RHS.Position;
}

معکوس_iterator rbegin(StringRef مسیر, Style style) {
  معکوس_iterator I;
  I.مسیر = مسیر;
  I.Position = مسیر.اندازه_2();
  I.S = style;
  ++I;
  أرجع I;
}

معکوس_iterator rend(StringRef مسیر) {
  معکوس_iterator I;
  I.مسیر = مسیر;
  I.Component = مسیر.substr(0, 0);
  I.Position = 0;
  أرجع I;
}

معکوس_iterator &معکوس_iterator::عملگر++() {
  size_t ریشه_dir_pos = ریشه_dir_شروع(مسیر, S);

  // Skip separators unless it's the root directory.
  size_t انتها_pos = Position;
  طالما (انتها_pos > 0 && (انتها_pos - 1) != ریشه_dir_pos &&
         is_separator(مسیر[انتها_pos - 1], S))
    --انتها_pos;

  // Treat trailing '/' as a '.', unless it is the root dir.
  إذا (Position == مسیر.اندازه_2() && !مسیر.خالی() &&
      is_separator(مسیر.پشت(), S) &&
      (ریشه_dir_pos == StringRef::npos || انتها_pos - 1 > ریشه_dir_pos)) {
    --Position;
    Component = ".";
    أرجع *هذا;
  }

  // Find next separator.
  size_t شروع_pos = filename_pos(مسیر.substr(0, انتها_pos), S);
  Component = مسیر.برش(شروع_pos, انتها_pos);
  Position = شروع_pos;
  أرجع *هذا;
}

منطقي معکوس_iterator::عملگر==(ثابت معکوس_iterator &RHS) ثابت {
  أرجع مسیر.begin() == RHS.مسیر.begin() && Component == RHS.Component &&
         Position == RHS.Position;
}

ptrdiff_t معکوس_iterator::عملگر-(ثابت معکوس_iterator &RHS) ثابت {
  أرجع Position - RHS.Position;
}

StringRef ریشه_مسیر_2(StringRef مسیر_2, Style style) {
  const_iterator b = begin(مسیر_2, style), pos = b, e = انتها(مسیر_2);
  إذا (b != e) {
    منطقي has_net =
        b->اندازه_2() > 2 && is_separator((*b)[0], style) && (*b)[1] == (*b)[0];
    منطقي has_drive = is_style_windows(style) && b->دوسر_with(":");

    إذا (has_net || has_drive) {
      إذا ((++pos != e) && is_separator((*pos)[0], style)) {
        // {C:/,//net/}, so get the first two components.
        أرجع مسیر_2.substr(0, b->اندازه_2() + pos->اندازه_2());
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

StringRef ریشه_name(StringRef مسیر_2, Style style) {
  const_iterator b = begin(مسیر_2, style), e = انتها(مسیر_2);
  إذا (b != e) {
    منطقي has_net =
        b->اندازه_2() > 2 && is_separator((*b)[0], style) && (*b)[1] == (*b)[0];
    منطقي has_drive = is_style_windows(style) && b->دوسر_with(":");

    إذا (has_net || has_drive) {
      // just {C:,//net}, return the first component.
      أرجع *b;
    }
  }

  // No path or no name.
  أرجع StringRef();
}

StringRef ریشه_directory(StringRef مسیر_2, Style style) {
  const_iterator b = begin(مسیر_2, style), pos = b, e = انتها(مسیر_2);
  إذا (b != e) {
    منطقي has_net =
        b->اندازه_2() > 2 && is_separator((*b)[0], style) && (*b)[1] == (*b)[0];
    منطقي has_drive = is_style_windows(style) && b->دوسر_with(":");

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

StringRef relative_مسیر(StringRef مسیر_2, Style style) {
  StringRef ریشه_3 = ریشه_مسیر_2(مسیر_2, style);
  أرجع مسیر_2.substr(ریشه_3.اندازه_2());
}

فراغ append(Small_وکتور_Impl<حرف> &مسیر_2, Style style, ثابت Twine &a,
            ثابت Twine &b, ثابت Twine &c, ثابت Twine &d) {
  SmallString<32> a_storage;
  SmallString<32> b_storage;
  SmallString<32> c_storage;
  SmallString<32> d_storage;

  Small_وکتور<StringRef, 4> components;
  إذا (!a.is_Trivially_خالی()) components.افزودن_پشت(a.مقصد_String_Ref(a_storage));
  إذا (!b.is_Trivially_خالی()) components.افزودن_پشت(b.مقصد_String_Ref(b_storage));
  إذا (!c.is_Trivially_خالی()) components.افزودن_پشت(c.مقصد_String_Ref(c_storage));
  إذا (!d.is_Trivially_خالی()) components.افزودن_پشت(d.مقصد_String_Ref(d_storage));

  لكل (خودکار &component : components) {
    منطقي مسیر_has_sep =
        !مسیر_2.خالی() && is_separator(مسیر_2[مسیر_2.اندازه_2() - 1], style);
    إذا (مسیر_has_sep) {
      // Strip separators from beginning of component.
      size_t loc = component.پیدا_کن_اولی_not_از(separators(style));
      StringRef c = component.substr(loc);

      // Append it.
      مسیر_2.append(c.begin(), c.انتها());
      استمر;
    }

    منطقي component_has_sep =
        !component.خالی() && is_separator(component[0], style);
    إذا (!component_has_sep &&
        !(مسیر_2.خالی() || has_ریشه_name(component, style))) {
      // Add a separator.
      مسیر_2.افزودن_پشت(preferred_separator(style));
    }

    مسیر_2.append(component.begin(), component.انتها());
  }
}

فراغ append(Small_وکتور_Impl<حرف> &مسیر_2, ثابت Twine &a, ثابت Twine &b,
            ثابت Twine &c, ثابت Twine &d) {
  append(مسیر_2, Style::native, a, b, c, d);
}

فراغ append(Small_وکتور_Impl<حرف> &مسیر_2, const_iterator begin,
            const_iterator انتها, Style style) {
  لكل (; begin != انتها; ++begin)
    مسیر_2::append(مسیر_2, style, *begin);
}

StringRef والد_مسیر(StringRef مسیر_2, Style style) {
  size_t انتها_pos = والد_مسیر_انتها(مسیر_2, style);
  إذا (انتها_pos == StringRef::npos)
    أرجع StringRef();
  أرجع مسیر_2.substr(0, انتها_pos);
}

فراغ remove_filename(Small_وکتور_Impl<حرف> &مسیر_2, Style style) {
  size_t انتها_pos = والد_مسیر_انتها(StringRef(مسیر_2.begin(), مسیر_2.اندازه_2()), style);
  إذا (انتها_pos != StringRef::npos)
    مسیر_2.truncate(انتها_pos);
}

فراغ replace_extension(Small_وکتور_Impl<حرف> &مسیر_2, ثابت Twine &extension,
                       Style style) {
  StringRef p(مسیر_2.begin(), مسیر_2.اندازه_2());
  SmallString<32> ext_storage;
  StringRef ext = extension.مقصد_String_Ref(ext_storage);

  // Erase existing extension.
  size_t pos = p.پیدا_کن_آخرین_از('.');
  إذا (pos != StringRef::npos && pos >= filename_pos(p, style))
    مسیر_2.truncate(pos);

  // Append '.' if needed.
  إذا (ext.اندازه_2() > 0 && ext[0] != '.')
    مسیر_2.افزودن_پشت('.');

  // Append extension.
  مسیر_2.append(ext.begin(), ext.انتها());
}

ساكن منطقي starts_with(StringRef مسیر, StringRef پیشوند,
                        Style style = Style::native) {
  // Windows prefix matching : case and separator insensitive
  إذا (is_style_windows(style)) {
    إذا (مسیر.اندازه_2() < پیشوند.اندازه_2())
      أرجع خطأ;
    لكل (size_t I = 0, E = پیشوند.اندازه_2(); I != E; ++I) {
      منطقي Sep_مسیر = is_separator(مسیر[I], style);
      منطقي Sep_پیشوند = is_separator(پیشوند[I], style);
      إذا (Sep_مسیر != Sep_پیشوند)
        أرجع خطأ;
      إذا (!Sep_مسیر && مقصد_پایینی(مسیر[I]) != مقصد_پایینی(پیشوند[I]))
        أرجع خطأ;
    }
    أرجع درست;
  }
  أرجع مسیر.starts_with(پیشوند);
}

منطقي replace_مسیر_پیشوند(Small_وکتور_Impl<حرف> &مسیر, StringRef Old_پیشوند,
                         StringRef New_پیشوند, Style style) {
  إذا (Old_پیشوند.خالی() && New_پیشوند.خالی())
    أرجع خطأ;

  StringRef Orig_مسیر(مسیر.begin(), مسیر.اندازه_2());
  إذا (!starts_with(Orig_مسیر, Old_پیشوند, style))
    أرجع خطأ;

  // If prefixes have the same size we can simply copy the new one over.
  إذا (Old_پیشوند.اندازه_2() == New_پیشوند.اندازه_2()) {
    llvm::copy(New_پیشوند, مسیر.begin());
    أرجع درست;
  }

  StringRef Rel_مسیر = Orig_مسیر.substr(Old_پیشوند.اندازه_2());
  SmallString<256> New_مسیر;
  (Twine(New_پیشوند) + Rel_مسیر).مقصد_وکتور(New_مسیر);
  مسیر.جابجا(New_مسیر);
  أرجع درست;
}

فراغ native(ثابت Twine &مسیر_2, Small_وکتور_Impl<حرف> &نتیجه_2, Style style) {
  أكد((!مسیر_2.isSingleStringRef() ||
          مسیر_2.getSingleStringRef().داده() != نتیجه_2.داده()) &&
         "path and result are not allowed to overlap!");
  // Clear result.
  نتیجه_2.clear();
  مسیر_2.مقصد_وکتور(نتیجه_2);
  native(نتیجه_2, style);
}

std::string native(ثابت Twine &مسیر_2, Style style) {
  SmallString<128> نتیجه;
  native(مسیر_2, نتیجه, style);
  أرجع std::string(نتیجه);
}

فراغ native(Small_وکتور_Impl<حرف> &مسیر, Style style) {
  إذا (مسیر.خالی())
    أرجع;
  إذا (is_style_windows(style)) {
    لكل (حرف &Ch : مسیر)
      إذا (is_separator(Ch, style))
        Ch = preferred_separator(style);
    إذا (مسیر[0] == '~' && (مسیر.اندازه_2() == 1 || is_separator(مسیر[1], style))) {
      SmallString<128> مسیر_Home;
      home_directory(مسیر_Home);
      مسیر_Home.append(مسیر.begin() + 1, مسیر.انتها());
      مسیر = std::move(مسیر_Home);
    }
  } وإلا {
    llvm::replace(مسیر, '\\', '/');
  }
}

std::string convert_مقصد_slash(StringRef مسیر_2, Style style) {
  إذا (is_style_posix(style))
    أرجع std::string(مسیر_2);

  std::string s = مسیر_2.str();
  llvm::replace(s, '\\', '/');
  أرجع s;
}

StringRef filename(StringRef مسیر_2, Style style) { أرجع *rbegin(مسیر_2, style); }

StringRef stem(StringRef مسیر_2, Style style) {
  StringRef fname = filename(مسیر_2, style);
  size_t pos = fname.پیدا_کن_آخرین_از('.');
  إذا (pos == StringRef::npos)
    أرجع fname;
  إذا ((fname.اندازه_2() == 1 && fname == ".") ||
      (fname.اندازه_2() == 2 && fname == ".."))
    أرجع fname;
  أرجع fname.substr(0, pos);
}

StringRef extension(StringRef مسیر_2, Style style) {
  StringRef fname = filename(مسیر_2, style);
  size_t pos = fname.پیدا_کن_آخرین_از('.');
  إذا (pos == StringRef::npos)
    أرجع StringRef();
  إذا ((fname.اندازه_2() == 1 && fname == ".") ||
      (fname.اندازه_2() == 2 && fname == ".."))
    أرجع StringRef();
  أرجع fname.substr(pos);
}

منطقي is_separator(حرف مقدار_2, Style style) {
  إذا (مقدار_2 == '/')
    أرجع درست;
  إذا (is_style_windows(style))
    أرجع مقدار_2 == '\\';
  أرجع خطأ;
}

StringRef get_separator(Style style) {
  إذا (real_style(style) == Style::windows)
    أرجع "\\";
  أرجع "/";
}

منطقي has_ریشه_name(ثابت Twine &مسیر_2, Style style) {
  SmallString<128> مسیر_storage;
  StringRef p = مسیر_2.مقصد_String_Ref(مسیر_storage);

  أرجع !ریشه_name(p, style).خالی();
}

منطقي has_ریشه_directory(ثابت Twine &مسیر_2, Style style) {
  SmallString<128> مسیر_storage;
  StringRef p = مسیر_2.مقصد_String_Ref(مسیر_storage);

  أرجع !ریشه_directory(p, style).خالی();
}

منطقي has_ریشه_مسیر(ثابت Twine &مسیر_2, Style style) {
  SmallString<128> مسیر_storage;
  StringRef p = مسیر_2.مقصد_String_Ref(مسیر_storage);

  أرجع !ریشه_مسیر_2(p, style).خالی();
}

منطقي has_relative_مسیر(ثابت Twine &مسیر_2, Style style) {
  SmallString<128> مسیر_storage;
  StringRef p = مسیر_2.مقصد_String_Ref(مسیر_storage);

  أرجع !relative_مسیر(p, style).خالی();
}

منطقي has_filename(ثابت Twine &مسیر_2, Style style) {
  SmallString<128> مسیر_storage;
  StringRef p = مسیر_2.مقصد_String_Ref(مسیر_storage);

  أرجع !filename(p, style).خالی();
}

منطقي has_والد_مسیر(ثابت Twine &مسیر_2, Style style) {
  SmallString<128> مسیر_storage;
  StringRef p = مسیر_2.مقصد_String_Ref(مسیر_storage);

  أرجع !والد_مسیر(p, style).خالی();
}

منطقي has_stem(ثابت Twine &مسیر_2, Style style) {
  SmallString<128> مسیر_storage;
  StringRef p = مسیر_2.مقصد_String_Ref(مسیر_storage);

  أرجع !stem(p, style).خالی();
}

منطقي has_extension(ثابت Twine &مسیر_2, Style style) {
  SmallString<128> مسیر_storage;
  StringRef p = مسیر_2.مقصد_String_Ref(مسیر_storage);

  أرجع !extension(p, style).خالی();
}

منطقي is_absolute(ثابت Twine &مسیر_2, Style style) {
  SmallString<128> مسیر_storage;
  StringRef p = مسیر_2.مقصد_String_Ref(مسیر_storage);

  منطقي ریشه_Dir = has_ریشه_directory(p, style);
  منطقي ریشه_Name = is_style_posix(style) || has_ریشه_name(p, style);

  أرجع ریشه_Dir && ریشه_Name;
}

منطقي is_absolute_gnu(ثابت Twine &مسیر_2, Style style) {
  SmallString<128> مسیر_storage;
  StringRef p = مسیر_2.مقصد_String_Ref(مسیر_storage);

  // Handle '/' which is absolute for both Windows and POSIX systems.
  // Handle '\\' on Windows.
  إذا (!p.خالی() && is_separator(p.front(), style))
    أرجع درست;

  إذا (is_style_windows(style)) {
    // Handle drive letter pattern (a character followed by ':') on Windows.
    إذا (p.اندازه_2() >= 2 && (p[0] && p[1] == ':'))
      أرجع درست;
  }

  أرجع خطأ;
}

منطقي is_relative(ثابت Twine &مسیر_2, Style style) {
  أرجع !is_absolute(مسیر_2, style);
}

فراغ make_absolute(ثابت Twine &current_directory,
                   Small_وکتور_Impl<حرف> &مسیر_2) {
  StringRef p(مسیر_2.داده(), مسیر_2.اندازه_2());

  منطقي ریشه_Directory = has_ریشه_directory(p);
  منطقي ریشه_Name = has_ریشه_name(p);

  // Already absolute.
  إذا ((ریشه_Name || is_style_posix(Style::native)) && ریشه_Directory)
    أرجع;

  // All the following conditions will need the current directory.
  SmallString<128> current_dir;
  current_directory.مقصد_وکتور(current_dir);

  // Relative path. Prepend the current directory.
  إذا (!ریشه_Name && !ریشه_Directory) {
    // Append path to the current directory.
    append(current_dir, p);
    // Set path to the result.
    مسیر_2.جابجا(current_dir);
    أرجع;
  }

  إذا (!ریشه_Name && ریشه_Directory) {
    StringRef cdrn = ریشه_name(current_dir);
    SmallString<128> cur_Dir_ریشه_Name(cdrn.begin(), cdrn.انتها());
    append(cur_Dir_ریشه_Name, p);
    // Set path to the result.
    مسیر_2.جابجا(cur_Dir_ریشه_Name);
    أرجع;
  }

  إذا (ریشه_Name && !ریشه_Directory) {
    StringRef p_ریشه_Name = ریشه_name(p);
    StringRef b_ریشه_Directory = ریشه_directory(current_dir);
    StringRef b_Relative_مسیر = relative_مسیر(current_dir);
    StringRef p_Relative_مسیر = relative_مسیر(p);

    SmallString<128> res;
    append(res, p_ریشه_Name, b_ریشه_Directory, b_Relative_مسیر, p_Relative_مسیر);
    مسیر_2.جابجا(res);
    أرجع;
  }

  llvm_unreachable("All rootName and rootDirectory combinations should have "
                   "occurred above!");
}

StringRef remove_leading_dotslash(StringRef مسیر, Style style) {
  // Remove leading "./" (or ".//" or "././" etc.)
  طالما (مسیر.اندازه_2() > 2 && مسیر[0] == '.' && is_separator(مسیر[1], style)) {
    مسیر = مسیر.substr(2);
    طالما (مسیر.اندازه_2() > 0 && is_separator(مسیر[0], style))
      مسیر = مسیر.substr(1);
  }
  أرجع مسیر;
}

منطقي remove_dots(Small_وکتور_Impl<حرف> &the_مسیر, منطقي remove_dot_dot,
                 Style style) {
  style = real_style(style);
  StringRef مانده_2(the_مسیر.داده(), the_مسیر.اندازه_2());
  منطقي needs_change = خطأ;
  Small_وکتور<StringRef, 16> components;

  // Consume the root path, if present.
  StringRef ریشه_3 = مسیر_2::ریشه_مسیر_2(مانده_2, style);
  منطقي absolute = !ریشه_3.خالی();
  إذا (absolute)
    مانده_2 = مانده_2.drop_front(ریشه_3.اندازه_2());

  // Loop over path components manually. This makes it easier to detect
  // non-preferred slashes and double separators that must be canonicalized.
  طالما (!مانده_2.خالی()) {
    size_t بعدی_slash = مانده_2.پیدا_کن_اولی_از(separators(style));
    إذا (بعدی_slash == StringRef::npos)
      بعدی_slash = مانده_2.اندازه_2();
    StringRef component = مانده_2.take_front(بعدی_slash);
    مانده_2 = مانده_2.drop_front(بعدی_slash);

    // Eat the slash, and check if it is the preferred separator.
    إذا (!مانده_2.خالی()) {
      needs_change |= مانده_2.front() != preferred_separator(style);
      مانده_2 = مانده_2.drop_front();
      // The path needs to be rewritten if it has a trailing slash.
      // FIXME: This is emergent behavior that could be removed.
      needs_change |= مانده_2.خالی();
    }

    // Check for path traversal components or double separators.
    إذا (component.خالی() || component == ".") {
      needs_change = درست;
    } وإلا إذا (remove_dot_dot && component == "..") {
      needs_change = درست;
      // Do not allow ".." to remove the root component. If this is the
      // beginning of a relative path, keep the ".." component.
      إذا (!components.خالی() && components.پشت() != "..") {
        components.برداشت_پشت();
      } وإلا إذا (!absolute) {
        components.افزودن_پشت(component);
      }
    } وإلا {
      components.افزودن_پشت(component);
    }
  }

  SmallString<256> میانگیر_2 = ریشه_3;
  // "root" could be "/", which may need to be translated into "\".
  make_preferred(میانگیر_2, style);
  needs_change |= ریشه_3 != میانگیر_2;

  // Avoid rewriting the path unless we have to.
  إذا (!needs_change)
    أرجع خطأ;

  إذا (!components.خالی()) {
    میانگیر_2 += components[0];
    لكل (StringRef C : ArrayRef(components).drop_front()) {
      میانگیر_2 += preferred_separator(style);
      میانگیر_2 += C;
    }
  }
  the_مسیر.جابجا(میانگیر_2);
  أرجع درست;
}

} // end namespace path

نطاق fs {

std::error_کد getUniqueID(ثابت Twine مسیر, UniqueID &نتیجه) {
  sandbox::violationIfEnabled();

  file_وضعیت وضعیت;
  std::error_کد EC = وضعیت_2(مسیر, وضعیت);
  إذا (EC)
    أرجع EC;
  نتیجه = وضعیت.getUniqueID();
  أرجع std::error_کد();
}

فراغ create_Unique_مسیر(ثابت Twine &Model, Small_وکتور_Impl<حرف> &نتیجه_مسیر,
                      منطقي MakeAbsolute) {
  SmallString<128> ModelStorage;
  Model.مقصد_وکتور(ModelStorage);

  أكد(llvm::is_contained(ModelStorage, '%') &&
         "createUniquePath: Model must contain at least one '%'");

  إذا (MakeAbsolute) {
    // Make model absolute by prepending a temp directory if it's not already.
    إذا (!sys::مسیر_2::is_absolute(Twine(ModelStorage))) {
      SmallString<128> TDir;
      sys::مسیر_2::system_موقت_directory(درست, TDir);
      sys::مسیر_2::append(TDir, Twine(ModelStorage));
      ModelStorage.جابجا(TDir);
    }
  }

  نتیجه_مسیر = ModelStorage;
  نتیجه_مسیر.افزودن_پشت(0);
  نتیجه_مسیر.برداشت_پشت();

  // Replace '%' with random chars.
  لكل (بیعلامت i = 0, e = ModelStorage.اندازه_2(); i != e; ++i) {
    إذا (ModelStorage[i] == '%')
      نتیجه_مسیر[i] = "0123456789abcdef"[sys::Process::Get_Random_عدد() & 15];
  }
}

std::error_کد createUniqueFile(ثابت Twine &Model, صحيح &نتیجه_Fd,
                                 Small_وکتور_Impl<حرف> &نتیجه_مسیر,
                                 OpenFlags Flags, بیعلامت نما) {
  أرجع createUniqueEntity(Model, نتیجه_Fd, نتیجه_مسیر, خطأ, FS_File, Flags,
                            نما);
}

std::error_کد createUniqueFile(ثابت Twine &Model,
                                 Small_وکتور_Impl<حرف> &نتیجه_مسیر,
                                 بیعلامت نما) {
  صحيح FD;
  خودکار EC = createUniqueFile(Model, FD, نتیجه_مسیر, از_None, نما);
  إذا (EC)
    أرجع EC;
  // FD is only needed to avoid race conditions. Close it right away.
  close(FD);
  أرجع EC;
}

ساكن std::error_کد
createTemporaryFile(ثابت Twine &Model, صحيح &نتیجه_FD,
                    llvm::Small_وکتور_Impl<حرف> &نتیجه_مسیر, FSEntity Type,
                    sys::fs::OpenFlags Flags = sys::fs::از_None) {
  // Any *temporary* file is assumed to be a compiler-internal output, not
  // a formal one.
  خودکار BypassSandbox = sys::sandbox::scopedDisable();

  SmallString<128> Storage;
  StringRef P = Model.مقصد_Null_Terminated_String_Ref(Storage);
  أكد(P.پیدا_کن_اولی_از(separators(Style::native)) == StringRef::npos &&
         "Model must be a simple filename.");
  // Use P.begin() so that createUniqueEntity doesn't need to recreate Storage.
  أرجع createUniqueEntity(P.begin(), نتیجه_FD, نتیجه_مسیر, درست, Type, Flags,
                            all_read | all_write);
}

ساكن std::error_کد
createTemporaryFile(ثابت Twine &پیشوند, StringRef Suffix, صحيح &نتیجه_FD,
                    llvm::Small_وکتور_Impl<حرف> &نتیجه_مسیر, FSEntity Type,
                    sys::fs::OpenFlags Flags = sys::fs::از_None) {
  ثابت حرف *میانه = Suffix.خالی() ? "-%%%%%%" : "-%%%%%%.";
  أرجع createTemporaryFile(پیشوند + میانه + Suffix, نتیجه_FD, نتیجه_مسیر,
                             Type, Flags);
}

std::error_کد createTemporaryFile(ثابت Twine &پیشوند, StringRef Suffix,
                                    صحيح &نتیجه_FD,
                                    Small_وکتور_Impl<حرف> &نتیجه_مسیر,
                                    sys::fs::OpenFlags Flags) {
  أرجع createTemporaryFile(پیشوند, Suffix, نتیجه_FD, نتیجه_مسیر, FS_File,
                             Flags);
}

std::error_کد createTemporaryFile(ثابت Twine &پیشوند, StringRef Suffix,
                                    Small_وکتور_Impl<حرف> &نتیجه_مسیر,
                                    sys::fs::OpenFlags Flags) {
  صحيح FD;
  خودکار EC = createTemporaryFile(پیشوند, Suffix, FD, نتیجه_مسیر, Flags);
  إذا (EC)
    أرجع EC;
  // FD is only needed to avoid race conditions. Close it right away.
  close(FD);
  أرجع EC;
}

// This is a mkdtemp with a different pattern. We use createUniqueEntity mostly
// for consistency. We should try using mkdtemp.
std::error_کد createUniqueDirectory(ثابت Twine &پیشوند,
                                      Small_وکتور_Impl<حرف> &نتیجه_مسیر) {
  صحيح Dummy;
  أرجع createUniqueEntity(پیشوند + "-%%%%%%", Dummy, نتیجه_مسیر, درست,
                            FS_Dir);
}

std::error_کد
getPotentiallyUniqueFileName(ثابت Twine &Model,
                             Small_وکتور_Impl<حرف> &نتیجه_مسیر) {
  صحيح Dummy;
  أرجع createUniqueEntity(Model, Dummy, نتیجه_مسیر, خطأ, FS_Name);
}

std::error_کد
get_Potentially_Unique_موقت_File_Name(ثابت Twine &پیشوند, StringRef Suffix,
                                 Small_وکتور_Impl<حرف> &نتیجه_مسیر) {
  صحيح Dummy;
  أرجع createTemporaryFile(پیشوند, Suffix, Dummy, نتیجه_مسیر, FS_Name);
}

std::error_کد make_absolute(Small_وکتور_Impl<حرف> &مسیر_2) {
  sandbox::violationIfEnabled();

  إذا (مسیر_2::is_absolute(مسیر_2))
    أرجع {};

  SmallString<128> current_dir;
  إذا (std::error_کد ec = current_مسیر(current_dir))
    أرجع ec;

  مسیر_2::make_absolute(current_dir, مسیر_2);
  أرجع {};
}

std::error_کد create_directories(ثابت Twine &مسیر, منطقي IgnoreExisting,
                                   perms Perms) {
  SmallString<128> مسیر_Storage;
  StringRef P = مسیر.مقصد_String_Ref(مسیر_Storage);

  // Be optimistic and try to create the directory
  std::error_کد EC = create_directory(P, IgnoreExisting, Perms);
  // If we succeeded, or had any error other than the parent not existing, just
  // return it.
  إذا (EC != errc::no_such_file_or_directory)
    أرجع EC;

  // We failed because of a no_such_file_or_directory, try to create the
  // parent.
  StringRef والد_2 = مسیر_2::والد_مسیر(P);
  إذا (والد_2.خالی())
    أرجع EC;

  إذا ((EC = create_directories(والد_2, IgnoreExisting, Perms)))
      أرجع EC;

  أرجع create_directory(P, IgnoreExisting, Perms);
}

ساكن std::error_کد copy_file_داخلی(صحيح ReadFD, صحيح WriteFD) {
  ثابت size_t Buf_اندازه = 4096;
  حرف *Buf = جديد حرف[Buf_اندازه];
  صحيح بایتها_Read = 0, بایتها_Written = 0;
  لكل (;;) {
    بایتها_Read = اقرأ(ReadFD, Buf, Buf_اندازه);
    إذا (بایتها_Read <= 0)
      اكسر;
    طالما (بایتها_Read) {
      بایتها_Written = اكتب(WriteFD, Buf, بایتها_Read);
      إذا (بایتها_Written < 0)
        اكسر;
      بایتها_Read -= بایتها_Written;
    }
    إذا (بایتها_Written < 0)
      اكسر;
  }
  حذف[] Buf;

  إذا (بایتها_Read < 0 || بایتها_Written < 0)
    أرجع errno_As_Error_کد();
  أرجع std::error_کد();
}

#اگر_تعریف_نشده __APPLE__
std::error_کد copy_file(ثابت Twine &From, ثابت Twine &مقصد) {
  صحيح ReadFD, WriteFD;
  إذا (std::error_کد EC = openFileForRead(From, ReadFD, از_None))
    أرجع EC;
  إذا (std::error_کد EC =
          openFileForWrite(مقصد, WriteFD, CD_CreateAlways, از_None)) {
    close(ReadFD);
    أرجع EC;
  }

  std::error_کد EC = copy_file_داخلی(ReadFD, WriteFD);

  close(ReadFD);
  close(WriteFD);

  أرجع EC;
}
#پایان_اگر

std::error_کد copy_file(ثابت Twine &From, صحيح مقصد_FD) {
  صحيح ReadFD;
  إذا (std::error_کد EC = openFileForRead(From, ReadFD, از_None))
    أرجع EC;

  std::error_کد EC = copy_file_داخلی(ReadFD, مقصد_FD);

  close(ReadFD);

  أرجع EC;
}

ErrorOr<MD5::MD5_نتیجه> md5_contents(صحيح FD) {
  sandbox::violationIfEnabled();

  MD5 Hash;

  constexpr size_t Buf_اندازه = 4096;
  std::وکتور<uint8_t> Buf(Buf_اندازه);
  صحيح بایتها_Read = 0;
  لكل (;;) {
    بایتها_Read = اقرأ(FD, Buf.داده(), Buf_اندازه);
    إذا (بایتها_Read <= 0)
      اكسر;
    Hash.update(ArrayRef(Buf.داده(), بایتها_Read));
  }

  إذا (بایتها_Read < 0)
    أرجع errno_As_Error_کد();
  MD5::MD5_نتیجه نتیجه;
  Hash.final(نتیجه);
  أرجع نتیجه;
}

ErrorOr<MD5::MD5_نتیجه> md5_contents(ثابت Twine &مسیر) {
  sandbox::violationIfEnabled();

  صحيح FD;
  إذا (خودکار EC = openFileForRead(مسیر, FD, از_None))
    أرجع EC;

  خودکار نتیجه = md5_contents(FD);
  close(FD);
  أرجع نتیجه;
}

منطقي exists(ثابت basic_file_وضعیت &وضعیت_2) {
  أرجع وضعیت_معلوم(وضعیت_2) && وضعیت_2.type() != file_type::file_not_یافت;
}

منطقي وضعیت_معلوم(ثابت basic_file_وضعیت &s) {
  أرجع s.type() != file_type::وضعیت_error;
}

file_type get_file_type(ثابت Twine &مسیر, منطقي Follow) {
  file_وضعیت st;
  إذا (وضعیت_2(مسیر, st, Follow))
    أرجع file_type::وضعیت_error;
  أرجع st.type();
}

منطقي is_directory(ثابت basic_file_وضعیت &وضعیت_2) {
  أرجع وضعیت_2.type() == file_type::directory_file;
}

std::error_کد is_directory(ثابت Twine &مسیر_2, منطقي &نتیجه_2) {
  sandbox::violationIfEnabled();

  file_وضعیت st;
  إذا (std::error_کد ec = وضعیت_2(مسیر_2, st))
    أرجع ec;
  نتیجه_2 = is_directory(st);
  أرجع std::error_کد();
}

منطقي is_regular_file(ثابت basic_file_وضعیت &وضعیت_2) {
  أرجع وضعیت_2.type() == file_type::regular_file;
}

std::error_کد is_regular_file(ثابت Twine &مسیر_2, منطقي &نتیجه_2) {
  sandbox::violationIfEnabled();

  file_وضعیت st;
  إذا (std::error_کد ec = وضعیت_2(مسیر_2, st))
    أرجع ec;
  نتیجه_2 = is_regular_file(st);
  أرجع std::error_کد();
}

منطقي is_symlink_file(ثابت basic_file_وضعیت &وضعیت_2) {
  أرجع وضعیت_2.type() == file_type::symlink_file;
}

std::error_کد is_symlink_file(ثابت Twine &مسیر_2, منطقي &نتیجه_2) {
  sandbox::violationIfEnabled();

  file_وضعیت st;
  إذا (std::error_کد ec = وضعیت_2(مسیر_2, st, خطأ))
    أرجع ec;
  نتیجه_2 = is_symlink_file(st);
  أرجع std::error_کد();
}

منطقي is_other(ثابت basic_file_وضعیت &وضعیت_2) {
  أرجع exists(وضعیت_2) &&
         !is_regular_file(وضعیت_2) &&
         !is_directory(وضعیت_2);
}

std::error_کد is_other(ثابت Twine &مسیر, منطقي &نتیجه) {
  sandbox::violationIfEnabled();

  file_وضعیت File_وضعیت;
  إذا (std::error_کد EC = وضعیت_2(مسیر, File_وضعیت))
    أرجع EC;
  نتیجه = is_other(File_وضعیت);
  أرجع std::error_کد();
}

فراغ directory_مدخل::replace_filename(ثابت Twine &Filename, file_type Type,
                                       basic_file_وضعیت وضعیت) {
  SmallString<128> مسیر_Str = مسیر_2::والد_مسیر(مسیر);
  مسیر_2::append(مسیر_Str, Filename);
  هذا->مسیر = std::string(مسیر_Str);
  هذا->Type = Type;
  هذا->وضعیت = وضعیت;
}

ErrorOr<perms> getPermissions(ثابت Twine &مسیر) {
  sandbox::violationIfEnabled();

  file_وضعیت وضعیت;
  إذا (std::error_کد EC = وضعیت_2(مسیر, وضعیت))
    أرجع EC;

  أرجع وضعیت.permissions();
}

std::error_کد مجموعه_آخرین_Access_And_Modification_زمان(ثابت Twine &مسیر,
                                                 زمان_نقطه<> Access_زمان,
                                                 زمان_نقطه<> Modification_زمان) {
  صحيح FD;
  إذا (std::error_کد EC = openFile(مسیر, FD, CD_OpenExisting, FA_Read,
                                    از_Update_صفات | از_Open_Directory))
    أرجع EC;
  std::error_کد EC =
      مجموعه_آخرین_Access_And_Modification_زمان(FD, Access_زمان, Modification_زمان);
  Process::SafelyCloseFileDescriptor(FD);
  أرجع EC;
}

size_t mapped_file_region::اندازه_2() ثابت {
  أكد(Mapping && "Mapping failed but used anyway!");
  أرجع اندازه;
}

حرف *mapped_file_region::داده() ثابت {
  أكد(Mapping && "Mapping failed but used anyway!");
  أرجع reinterpret_cast<حرف *>(Mapping);
}

ثابت حرف *mapped_file_region::const_داده() ثابت {
  أكد(Mapping && "Mapping failed but used anyway!");
  أرجع reinterpret_cast<ثابت حرف *>(Mapping);
}

Error read_Native_File_مقصد_EOF(file_t FileHandle, Small_وکتور_Impl<حرف> &میانگیر,
                          ssize_t Chunk_اندازه) {
  sandbox::violationIfEnabled();

  // Install a handler to truncate the buffer to the correct size on exit.
  size_t اندازه = میانگیر.اندازه_2();
  llvm::scope_exit TruncateOnExit([&]() { میانگیر.truncate(اندازه); });

  // Read into Buffer until we hit EOF.
  لكل (;;) {
    میانگیر.resize_for_overwrite(اندازه + Chunk_اندازه);
    Expected<size_t> Read_بایتها = readNativeFile(
        FileHandle, MutableArrayRef(میانگیر.begin() + اندازه, Chunk_اندازه));
    إذا (!Read_بایتها)
      أرجع Read_بایتها.takeError();
    إذا (*Read_بایتها == 0)
      أرجع Error::success();
    اندازه += *Read_بایتها;
  }
}

} // end namespace fs
} // end namespace sys
} // end namespace llvm

// Include the truly platform-specific parts.
#إذا defined(LLVM_ON_UNIX)
#ضمن "Unix/Path.inc"
#پایان_اگر
#إذا defined(_WIN32)
#ضمن "Windows/Path.inc"
#پایان_اگر

نطاق llvm {
نطاق sys {
نطاق fs {

موقت_File::موقت_File(StringRef Name, صحيح FD)
    : TmpName(std::string(Name)), FD(FD) {}
موقت_File::موقت_File(موقت_File &&Other) { *هذا = std::move(Other); }
موقت_File &موقت_File::عملگر=(موقت_File &&Other) {
  TmpName = std::move(Other.TmpName);
  FD = Other.FD;
  Other.انجامشده = درست;
  Other.FD = -1;
#اگر_تعریف_شده _WIN32
  RemoveOnClose = Other.RemoveOnClose;
  Other.RemoveOnClose = خطأ;
#پایان_اگر
  أرجع *هذا;
}

موقت_File::~موقت_File() { أكد(انجامشده); }

Error موقت_File::discard() {
  انجامشده = درست;
  إذا (FD != -1 && close(FD) == -1) {
    std::error_کد EC = errno_As_Error_کد();
    أرجع error_کد_مقصد_Error(EC);
  }
  FD = -1;

#اگر_تعریف_شده _WIN32
  // On Windows, closing will remove the file, if we set the delete
  // disposition. If not, remove it manually.
  منطقي Remove = RemoveOnClose;
#وإلا
  // Always try to remove the file.
  منطقي Remove = درست;
#پایان_اگر
  std::error_کد RemoveEC;
  إذا (Remove && !TmpName.خالی()) {
    RemoveEC = fs::remove(TmpName);
    sys::DontRemoveFileOnSignal(TmpName);
    إذا (!RemoveEC)
      TmpName = "";
  } وإلا {
    TmpName = "";
  }
  أرجع error_کد_مقصد_Error(RemoveEC);
}

Error موقت_File::keep(ثابت Twine &Name) {
  أكد(!انجامشده);
  انجامشده = درست;
  // Always try to close and rename.
#اگر_تعریف_شده _WIN32
  // If we can't cancel the delete don't rename.
  خودکار H = reinterpret_cast<HANDLE>(_get_osfhandle(FD));
  std::error_کد RenameEC =
      RemoveOnClose ? std::error_کد() : مجموعه_Delete_Disposition(H, خطأ);
  منطقي ShouldDelete = خطأ;
  إذا (!RenameEC) {
    RenameEC = rename_handle(H, Name);
    // If rename failed because it's cross-device, copy instead
    إذا (RenameEC ==
      std::error_کد(ERROR_NOT_SAME_DEVICE, std::system_category())) {
      RenameEC = copy_file(TmpName, Name);
      ShouldDelete = درست;
    }
  }

  // If we can't rename or copy, discard the temporary file.
  إذا (RenameEC)
    ShouldDelete = درست;
  إذا (ShouldDelete) {
    إذا (!RemoveOnClose)
      مجموعه_Delete_Disposition(H, درست);
    وإلا
      remove(TmpName);
  }
#وإلا
  std::error_کد RenameEC = fs::rename(TmpName, Name);
  إذا (RenameEC) {
    // If we can't rename, try to copy to work around cross-device link issues.
    RenameEC = sys::fs::copy_file(TmpName, Name);
    // If we can't rename or copy, discard the temporary file.
    إذا (RenameEC)
      remove(TmpName);
  }
#پایان_اگر
  sys::DontRemoveFileOnSignal(TmpName);

  إذا (!RenameEC)
    TmpName = "";

  إذا (close(FD) == -1)
    أرجع error_کد_مقصد_Error(errno_As_Error_کد());
  FD = -1;

  أرجع error_کد_مقصد_Error(RenameEC);
}

Error موقت_File::keep() {
  أكد(!انجامشده);
  انجامشده = درست;

#اگر_تعریف_شده _WIN32
  خودکار H = reinterpret_cast<HANDLE>(_get_osfhandle(FD));
  إذا (std::error_کد EC = مجموعه_Delete_Disposition(H, خطأ))
    أرجع error_کد_مقصد_Error(EC);
#پایان_اگر
  sys::DontRemoveFileOnSignal(TmpName);

  TmpName = "";

  إذا (close(FD) == -1)
    أرجع error_کد_مقصد_Error(errno_As_Error_کد());
  FD = -1;

  أرجع Error::success();
}

Expected<موقت_File> موقت_File::create(ثابت Twine &Model, بیعلامت نما,
                                    OpenFlags ExtraFlags) {
  صحيح FD;
  SmallString<128> نتیجه_مسیر;
  إذا (std::error_کد EC =
          createUniqueFile(Model, FD, نتیجه_مسیر, از_Delete | ExtraFlags, نما))
    أرجع error_کد_مقصد_Error(EC);

  موقت_File Ret(نتیجه_مسیر, FD);
#اگر_تعریف_شده _WIN32
  خودکار H = reinterpret_cast<HANDLE>(_get_osfhandle(FD));
  منطقي مجموعه_Signal_Handler = خطأ;
  إذا (std::error_کد EC = مجموعه_Delete_Disposition(H, درست)) {
    Ret.RemoveOnClose = درست;
    مجموعه_Signal_Handler = درست;
  }
#وإلا
  منطقي مجموعه_Signal_Handler = درست;
#پایان_اگر
  إذا (مجموعه_Signal_Handler && sys::RemoveFileOnSignal(نتیجه_مسیر)) {
    // Make sure we delete the file when RemoveFileOnSignal fails.
    consumeError(Ret.discard());
    std::error_کد EC(errc::operation_not_permitted);
    أرجع error_کد_مقصد_Error(EC);
  }
  أرجع std::move(Ret);
}
} // namespace fs

} // namespace sys
} // namespace llvm
