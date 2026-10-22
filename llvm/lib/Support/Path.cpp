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
#د_که_پای

کارول نطاق llvm;
کارول نطاق llvm::support::endian;

نطاق {
  کارول llvm::StringRef;
  کارول llvm::sys::لار_2::is_separator;
  کارول llvm::sys::لار_2::Style;

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

  StringRef پیدا_کړه_لومړی_component(StringRef لار_2, Style style) {
    // Look for this first component in the following order.
    // * empty (in this case we return an empty string)
    // * either C: or {//,\\}net.
    // * {/,\}
    // * {file,directory}name

    إذا (لار_2.تشه())
      أرجع لار_2;

    إذا (is_style_windows(style)) {
      // C:
      إذا (لار_2.اندازه_2() >= 2 &&
          std::که_توری(static_cast<بینښه حرف>(لار_2[0])) && لار_2[1] == ':')
        أرجع لار_2.substr(0, 2);
    }

    // //net
    إذا ((لار_2.اندازه_2() > 2) && is_separator(لار_2[0], style) &&
        لار_2[0] == لار_2[1] && !is_separator(لار_2[2], style)) {
      // Find the next directory separator.
      size_t پای_ټکی = لار_2.پیدا_کړه_لومړی_د(separators(style), 2);
      أرجع لار_2.substr(0, پای_ټکی);
    }

    // {/,\}
    إذا (is_separator(لار_2[0], style))
      أرجع لار_2.substr(0, 1);

    // * {file,directory}name
    size_t پای_ټکی = لار_2.پیدا_کړه_لومړی_د(separators(style));
    أرجع لار_2.substr(0, پای_ټکی);
  }

  // Returns the first character of the filename in str. For paths ending in
  // '/', it returns the position of the '/'.
  size_t filename_pos(StringRef str, Style style) {
    إذا (str.اندازه_2() > 0 && is_separator(str[str.اندازه_2() - 1], style))
      أرجع str.اندازه_2() - 1;

    size_t pos = str.پیدا_کړه_وروستی_د(separators(style), str.اندازه_2() - 1);

    إذا (is_style_windows(style)) {
      إذا (pos == StringRef::npos)
        pos = str.پیدا_کړه_وروستی_د(':', str.اندازه_2() - 1);
    }

    إذا (pos == StringRef::npos || (pos == 1 && is_separator(str[0], style)))
      أرجع 0;

    أرجع pos + 1;
  }

  // Returns the position of the root directory in str. If there is no root
  // directory in str, it returns StringRef::npos.
  size_t ریښه_dir_پیل(StringRef str, Style style) {
    // case "c:/"
    إذا (is_style_windows(style)) {
      إذا (str.اندازه_2() > 2 && str[1] == ':' && is_separator(str[2], style))
        أرجع 2;
    }

    // case "//net"
    إذا (str.اندازه_2() > 3 && is_separator(str[0], style) && str[0] == str[1] &&
        !is_separator(str[2], style)) {
      أرجع str.پیدا_کړه_لومړی_د(separators(style), 2);
    }

    // case "/"
    إذا (str.اندازه_2() > 0 && is_separator(str[0], style))
      أرجع 0;

    أرجع StringRef::npos;
  }

  // Returns the position past the end of the "parent path" of path. The parent
  // path will not end in '/', unless the parent is the root directory. If the
  // path has no parent, 0 is returned.
  size_t پلار_لار_پای_ټکی(StringRef لار_2, Style style) {
    size_t پای_ټکی_pos = filename_pos(لار_2, style);

    منطقي filename_was_sep =
        لار_2.اندازه_2() > 0 && is_separator(لار_2[پای_ټکی_pos], style);

    // Skip separators until we reach root dir (or the start of the string).
    size_t ریښه_dir_pos = ریښه_dir_پیل(لار_2, style);
    طالما (پای_ټکی_pos > 0 &&
           (ریښه_dir_pos == StringRef::npos || پای_ټکی_pos > ریښه_dir_pos) &&
           is_separator(لار_2[پای_ټکی_pos - 1], style))
      --پای_ټکی_pos;

    إذا (پای_ټکی_pos == ریښه_dir_pos && !filename_was_sep) {
      // We've reached the root dir and the input path was *not* ending in a
      // sequence of slashes. Include the root dir in the parent path.
      أرجع ریښه_dir_pos + 1;
    }

    // Otherwise, just include before the last slash.
    أرجع پای_ټکی_pos;
  }
} // end unnamed namespace

تعداد FSEntity {
  FS_Dir,
  FS_File,
  FS_Name
};

ساكن std::error_کوډ
createUniqueEntity(ثابت Twine &Model, صحيح &پایله_FD,
                   Small_ویکټور_Impl<حرف> &پایله_لار, منطقي MakeAbsolute,
                   FSEntity Type, sys::fs::OpenFlags Flags = sys::fs::د_None,
                   بینښه ډېرپېښ = 0) {

  // Limit the number of attempts we make, so that we don't infinite loop. E.g.
  // "permission denied" could be for a specific file (so we retry with a
  // different name) or for the whole directory (retry would always fail).
  // Checking which is racy, so we try a number of times, then give up.
  std::error_کوډ EC;
  لكل (صحيح Retries = 128; Retries > 0; --Retries) {
    sys::fs::create_Unique_لار(Model, پایله_لار, MakeAbsolute);
    // Try to open + create the file.
    بدل (Type) {
    حالة FS_File: {
      EC = sys::fs::openFileForReadWrite(Twine(پایله_لار.begin()), پایله_FD,
                                         sys::fs::CD_CreateNew, Flags, ډېرپېښ);
      إذا (EC) {
        // errc::permission_denied happens on Windows when we try to open a file
        // that has been marked for deletion.
        إذا (EC == errc::file_exists || EC == errc::permission_denied)
          استمر;
        أرجع EC;
      }

      أرجع std::error_کوډ();
    }

    حالة FS_Name: {
      EC = sys::fs::access(پایله_لار.begin(), sys::fs::Access_ډېرپېښ::Exist);
      إذا (EC == errc::no_such_file_or_directory)
        أرجع std::error_کوډ();
      إذا (EC)
        أرجع EC;
      استمر;
    }

    حالة FS_Dir: {
      EC = sys::fs::create_directory(پایله_لار.begin(), خطأ);
      إذا (EC) {
        إذا (EC == errc::file_exists)
          استمر;
        أرجع EC;
      }
      أرجع std::error_کوډ();
    }
    }
    llvm_unreachable("Invalid Type");
  }
  أرجع EC;
}

نطاق llvm {
نطاق sys  {
نطاق لار_2 {

const_iterator begin(StringRef لار_2, Style style) {
  const_iterator i;
  i.لار      = لار_2;
  i.Component = پیدا_کړه_لومړی_component(لار_2, style);
  i.Position  = 0;
  i.S = style;
  أرجع i;
}

const_iterator پای_ټکی(StringRef لار_2) {
  const_iterator i;
  i.لار      = لار_2;
  i.Position  = لار_2.اندازه_2();
  أرجع i;
}

const_iterator &const_iterator::چلوونکی++() {
  أكد(Position < لار.اندازه_2() && "Tried to increment past end!");

  // Increment Position to past the current component
  Position += Component.اندازه_2();

  // Check for end.
  إذا (Position == لار.اندازه_2()) {
    Component = StringRef();
    أرجع *هذا;
  }

  // Both POSIX and Windows treat paths that begin with exactly two separators
  // specially.
  منطقي was_net = Component.اندازه_2() > 2 && is_separator(Component[0], S) &&
                 Component[1] == Component[0] && !is_separator(Component[2], S);

  // Handle separators.
  إذا (is_separator(لار[Position], S)) {
    // Root dir.
    إذا (was_net ||
        // c:/
        (is_style_windows(S) && Component.پایونه_with(":"))) {
      Component = لار.substr(Position, 1);
      أرجع *هذا;
    }

    // Skip extra separators.
    طالما (Position != لار.اندازه_2() && is_separator(لار[Position], S)) {
      ++Position;
    }

    // Treat trailing '/' as a '.', unless it is the root dir.
    إذا (Position == لار.اندازه_2() && Component != "/") {
      --Position;
      Component = ".";
      أرجع *هذا;
    }
  }

  // Find next component.
  size_t پای_ټکی_pos = لار.پیدا_کړه_لومړی_د(separators(S), Position);
  Component = لار.ټوټه(Position, پای_ټکی_pos);

  أرجع *هذا;
}

منطقي const_iterator::چلوونکی==(ثابت const_iterator &RHS) ثابت {
  أرجع لار.begin() == RHS.لار.begin() && Position == RHS.Position;
}

ptrdiff_t const_iterator::چلوونکی-(ثابت const_iterator &RHS) ثابت {
  أرجع Position - RHS.Position;
}

سرچپه_iterator rbegin(StringRef لار, Style style) {
  سرچپه_iterator I;
  I.لار = لار;
  I.Position = لار.اندازه_2();
  I.S = style;
  ++I;
  أرجع I;
}

سرچپه_iterator rend(StringRef لار) {
  سرچپه_iterator I;
  I.لار = لار;
  I.Component = لار.substr(0, 0);
  I.Position = 0;
  أرجع I;
}

سرچپه_iterator &سرچپه_iterator::چلوونکی++() {
  size_t ریښه_dir_pos = ریښه_dir_پیل(لار, S);

  // Skip separators unless it's the root directory.
  size_t پای_ټکی_pos = Position;
  طالما (پای_ټکی_pos > 0 && (پای_ټکی_pos - 1) != ریښه_dir_pos &&
         is_separator(لار[پای_ټکی_pos - 1], S))
    --پای_ټکی_pos;

  // Treat trailing '/' as a '.', unless it is the root dir.
  إذا (Position == لار.اندازه_2() && !لار.تشه() &&
      is_separator(لار.شا(), S) &&
      (ریښه_dir_pos == StringRef::npos || پای_ټکی_pos - 1 > ریښه_dir_pos)) {
    --Position;
    Component = ".";
    أرجع *هذا;
  }

  // Find next separator.
  size_t پیل_pos = filename_pos(لار.substr(0, پای_ټکی_pos), S);
  Component = لار.ټوټه(پیل_pos, پای_ټکی_pos);
  Position = پیل_pos;
  أرجع *هذا;
}

منطقي سرچپه_iterator::چلوونکی==(ثابت سرچپه_iterator &RHS) ثابت {
  أرجع لار.begin() == RHS.لار.begin() && Component == RHS.Component &&
         Position == RHS.Position;
}

ptrdiff_t سرچپه_iterator::چلوونکی-(ثابت سرچپه_iterator &RHS) ثابت {
  أرجع Position - RHS.Position;
}

StringRef ریښه_لار_2(StringRef لار_2, Style style) {
  const_iterator b = begin(لار_2, style), pos = b, e = پای_ټکی(لار_2);
  إذا (b != e) {
    منطقي has_net =
        b->اندازه_2() > 2 && is_separator((*b)[0], style) && (*b)[1] == (*b)[0];
    منطقي has_drive = is_style_windows(style) && b->پایونه_with(":");

    إذا (has_net || has_drive) {
      إذا ((++pos != e) && is_separator((*pos)[0], style)) {
        // {C:/,//net/}, so get the first two components.
        أرجع لار_2.substr(0, b->اندازه_2() + pos->اندازه_2());
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

StringRef ریښه_name(StringRef لار_2, Style style) {
  const_iterator b = begin(لار_2, style), e = پای_ټکی(لار_2);
  إذا (b != e) {
    منطقي has_net =
        b->اندازه_2() > 2 && is_separator((*b)[0], style) && (*b)[1] == (*b)[0];
    منطقي has_drive = is_style_windows(style) && b->پایونه_with(":");

    إذا (has_net || has_drive) {
      // just {C:,//net}, return the first component.
      أرجع *b;
    }
  }

  // No path or no name.
  أرجع StringRef();
}

StringRef ریښه_directory(StringRef لار_2, Style style) {
  const_iterator b = begin(لار_2, style), pos = b, e = پای_ټکی(لار_2);
  إذا (b != e) {
    منطقي has_net =
        b->اندازه_2() > 2 && is_separator((*b)[0], style) && (*b)[1] == (*b)[0];
    منطقي has_drive = is_style_windows(style) && b->پایونه_with(":");

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

StringRef relative_لار(StringRef لار_2, Style style) {
  StringRef ریښه_3 = ریښه_لار_2(لار_2, style);
  أرجع لار_2.substr(ریښه_3.اندازه_2());
}

فراغ append(Small_ویکټور_Impl<حرف> &لار_2, Style style, ثابت Twine &a,
            ثابت Twine &b, ثابت Twine &c, ثابت Twine &d) {
  SmallString<32> a_storage;
  SmallString<32> b_storage;
  SmallString<32> c_storage;
  SmallString<32> d_storage;

  Small_ویکټور<StringRef, 4> components;
  إذا (!a.is_Trivially_تشه()) components.ټېل_شا(a.منزل_String_Ref(a_storage));
  إذا (!b.is_Trivially_تشه()) components.ټېل_شا(b.منزل_String_Ref(b_storage));
  إذا (!c.is_Trivially_تشه()) components.ټېل_شا(c.منزل_String_Ref(c_storage));
  إذا (!d.is_Trivially_تشه()) components.ټېل_شا(d.منزل_String_Ref(d_storage));

  لكل (خپلکاری &component : components) {
    منطقي لار_has_sep =
        !لار_2.تشه() && is_separator(لار_2[لار_2.اندازه_2() - 1], style);
    إذا (لار_has_sep) {
      // Strip separators from beginning of component.
      size_t loc = component.پیدا_کړه_لومړی_not_د(separators(style));
      StringRef c = component.substr(loc);

      // Append it.
      لار_2.append(c.begin(), c.پای_ټکی());
      استمر;
    }

    منطقي component_has_sep =
        !component.تشه() && is_separator(component[0], style);
    إذا (!component_has_sep &&
        !(لار_2.تشه() || has_ریښه_name(component, style))) {
      // Add a separator.
      لار_2.ټېل_شا(preferred_separator(style));
    }

    لار_2.append(component.begin(), component.پای_ټکی());
  }
}

فراغ append(Small_ویکټور_Impl<حرف> &لار_2, ثابت Twine &a, ثابت Twine &b,
            ثابت Twine &c, ثابت Twine &d) {
  append(لار_2, Style::native, a, b, c, d);
}

فراغ append(Small_ویکټور_Impl<حرف> &لار_2, const_iterator begin,
            const_iterator پای_ټکی, Style style) {
  لكل (; begin != پای_ټکی; ++begin)
    لار_2::append(لار_2, style, *begin);
}

StringRef پلار_لار(StringRef لار_2, Style style) {
  size_t پای_ټکی_pos = پلار_لار_پای_ټکی(لار_2, style);
  إذا (پای_ټکی_pos == StringRef::npos)
    أرجع StringRef();
  أرجع لار_2.substr(0, پای_ټکی_pos);
}

فراغ remove_filename(Small_ویکټور_Impl<حرف> &لار_2, Style style) {
  size_t پای_ټکی_pos = پلار_لار_پای_ټکی(StringRef(لار_2.begin(), لار_2.اندازه_2()), style);
  إذا (پای_ټکی_pos != StringRef::npos)
    لار_2.truncate(پای_ټکی_pos);
}

فراغ replace_extension(Small_ویکټور_Impl<حرف> &لار_2, ثابت Twine &extension,
                       Style style) {
  StringRef p(لار_2.begin(), لار_2.اندازه_2());
  SmallString<32> ext_storage;
  StringRef ext = extension.منزل_String_Ref(ext_storage);

  // Erase existing extension.
  size_t pos = p.پیدا_کړه_وروستی_د('.');
  إذا (pos != StringRef::npos && pos >= filename_pos(p, style))
    لار_2.truncate(pos);

  // Append '.' if needed.
  إذا (ext.اندازه_2() > 0 && ext[0] != '.')
    لار_2.ټېل_شا('.');

  // Append extension.
  لار_2.append(ext.begin(), ext.پای_ټکی());
}

ساكن منطقي starts_with(StringRef لار, StringRef مختاړی,
                        Style style = Style::native) {
  // Windows prefix matching : case and separator insensitive
  إذا (is_style_windows(style)) {
    إذا (لار.اندازه_2() < مختاړی.اندازه_2())
      أرجع خطأ;
    لكل (size_t I = 0, E = مختاړی.اندازه_2(); I != E; ++I) {
      منطقي Sep_لار = is_separator(لار[I], style);
      منطقي Sep_مختاړی = is_separator(مختاړی[I], style);
      إذا (Sep_لار != Sep_مختاړی)
        أرجع خطأ;
      إذا (!Sep_لار && منزل_ښکتنی(لار[I]) != منزل_ښکتنی(مختاړی[I]))
        أرجع خطأ;
    }
    أرجع ریښتیا;
  }
  أرجع لار.starts_with(مختاړی);
}

منطقي replace_لار_مختاړی(Small_ویکټور_Impl<حرف> &لار, StringRef Old_مختاړی,
                         StringRef New_مختاړی, Style style) {
  إذا (Old_مختاړی.تشه() && New_مختاړی.تشه())
    أرجع خطأ;

  StringRef Orig_لار(لار.begin(), لار.اندازه_2());
  إذا (!starts_with(Orig_لار, Old_مختاړی, style))
    أرجع خطأ;

  // If prefixes have the same size we can simply copy the new one over.
  إذا (Old_مختاړی.اندازه_2() == New_مختاړی.اندازه_2()) {
    llvm::copy(New_مختاړی, لار.begin());
    أرجع ریښتیا;
  }

  StringRef Rel_لار = Orig_لار.substr(Old_مختاړی.اندازه_2());
  SmallString<256> New_لار;
  (Twine(New_مختاړی) + Rel_لار).منزل_ویکټور(New_لار);
  لار.بدلون(New_لار);
  أرجع ریښتیا;
}

فراغ native(ثابت Twine &لار_2, Small_ویکټور_Impl<حرف> &پایله_2, Style style) {
  أكد((!لار_2.isSingleStringRef() ||
          لار_2.getSingleStringRef().معلومات() != پایله_2.معلومات()) &&
         "path and result are not allowed to overlap!");
  // Clear result.
  پایله_2.clear();
  لار_2.منزل_ویکټور(پایله_2);
  native(پایله_2, style);
}

std::string native(ثابت Twine &لار_2, Style style) {
  SmallString<128> پایله;
  native(لار_2, پایله, style);
  أرجع std::string(پایله);
}

فراغ native(Small_ویکټور_Impl<حرف> &لار, Style style) {
  إذا (لار.تشه())
    أرجع;
  إذا (is_style_windows(style)) {
    لكل (حرف &Ch : لار)
      إذا (is_separator(Ch, style))
        Ch = preferred_separator(style);
    إذا (لار[0] == '~' && (لار.اندازه_2() == 1 || is_separator(لار[1], style))) {
      SmallString<128> لار_Home;
      home_directory(لار_Home);
      لار_Home.append(لار.begin() + 1, لار.پای_ټکی());
      لار = std::move(لار_Home);
    }
  } وإلا {
    llvm::replace(لار, '\\', '/');
  }
}

std::string convert_منزل_slash(StringRef لار_2, Style style) {
  إذا (is_style_posix(style))
    أرجع std::string(لار_2);

  std::string s = لار_2.str();
  llvm::replace(s, '\\', '/');
  أرجع s;
}

StringRef filename(StringRef لار_2, Style style) { أرجع *rbegin(لار_2, style); }

StringRef stem(StringRef لار_2, Style style) {
  StringRef fname = filename(لار_2, style);
  size_t pos = fname.پیدا_کړه_وروستی_د('.');
  إذا (pos == StringRef::npos)
    أرجع fname;
  إذا ((fname.اندازه_2() == 1 && fname == ".") ||
      (fname.اندازه_2() == 2 && fname == ".."))
    أرجع fname;
  أرجع fname.substr(0, pos);
}

StringRef extension(StringRef لار_2, Style style) {
  StringRef fname = filename(لار_2, style);
  size_t pos = fname.پیدا_کړه_وروستی_د('.');
  إذا (pos == StringRef::npos)
    أرجع StringRef();
  إذا ((fname.اندازه_2() == 1 && fname == ".") ||
      (fname.اندازه_2() == 2 && fname == ".."))
    أرجع StringRef();
  أرجع fname.substr(pos);
}

منطقي is_separator(حرف ارزښت_2, Style style) {
  إذا (ارزښت_2 == '/')
    أرجع ریښتیا;
  إذا (is_style_windows(style))
    أرجع ارزښت_2 == '\\';
  أرجع خطأ;
}

StringRef get_separator(Style style) {
  إذا (real_style(style) == Style::windows)
    أرجع "\\";
  أرجع "/";
}

منطقي has_ریښه_name(ثابت Twine &لار_2, Style style) {
  SmallString<128> لار_storage;
  StringRef p = لار_2.منزل_String_Ref(لار_storage);

  أرجع !ریښه_name(p, style).تشه();
}

منطقي has_ریښه_directory(ثابت Twine &لار_2, Style style) {
  SmallString<128> لار_storage;
  StringRef p = لار_2.منزل_String_Ref(لار_storage);

  أرجع !ریښه_directory(p, style).تشه();
}

منطقي has_ریښه_لار(ثابت Twine &لار_2, Style style) {
  SmallString<128> لار_storage;
  StringRef p = لار_2.منزل_String_Ref(لار_storage);

  أرجع !ریښه_لار_2(p, style).تشه();
}

منطقي has_relative_لار(ثابت Twine &لار_2, Style style) {
  SmallString<128> لار_storage;
  StringRef p = لار_2.منزل_String_Ref(لار_storage);

  أرجع !relative_لار(p, style).تشه();
}

منطقي has_filename(ثابت Twine &لار_2, Style style) {
  SmallString<128> لار_storage;
  StringRef p = لار_2.منزل_String_Ref(لار_storage);

  أرجع !filename(p, style).تشه();
}

منطقي has_پلار_لار(ثابت Twine &لار_2, Style style) {
  SmallString<128> لار_storage;
  StringRef p = لار_2.منزل_String_Ref(لار_storage);

  أرجع !پلار_لار(p, style).تشه();
}

منطقي has_stem(ثابت Twine &لار_2, Style style) {
  SmallString<128> لار_storage;
  StringRef p = لار_2.منزل_String_Ref(لار_storage);

  أرجع !stem(p, style).تشه();
}

منطقي has_extension(ثابت Twine &لار_2, Style style) {
  SmallString<128> لار_storage;
  StringRef p = لار_2.منزل_String_Ref(لار_storage);

  أرجع !extension(p, style).تشه();
}

منطقي is_absolute(ثابت Twine &لار_2, Style style) {
  SmallString<128> لار_storage;
  StringRef p = لار_2.منزل_String_Ref(لار_storage);

  منطقي ریښه_Dir = has_ریښه_directory(p, style);
  منطقي ریښه_Name = is_style_posix(style) || has_ریښه_name(p, style);

  أرجع ریښه_Dir && ریښه_Name;
}

منطقي is_absolute_gnu(ثابت Twine &لار_2, Style style) {
  SmallString<128> لار_storage;
  StringRef p = لار_2.منزل_String_Ref(لار_storage);

  // Handle '/' which is absolute for both Windows and POSIX systems.
  // Handle '\\' on Windows.
  إذا (!p.تشه() && is_separator(p.front(), style))
    أرجع ریښتیا;

  إذا (is_style_windows(style)) {
    // Handle drive letter pattern (a character followed by ':') on Windows.
    إذا (p.اندازه_2() >= 2 && (p[0] && p[1] == ':'))
      أرجع ریښتیا;
  }

  أرجع خطأ;
}

منطقي is_relative(ثابت Twine &لار_2, Style style) {
  أرجع !is_absolute(لار_2, style);
}

فراغ make_absolute(ثابت Twine &current_directory,
                   Small_ویکټور_Impl<حرف> &لار_2) {
  StringRef p(لار_2.معلومات(), لار_2.اندازه_2());

  منطقي ریښه_Directory = has_ریښه_directory(p);
  منطقي ریښه_Name = has_ریښه_name(p);

  // Already absolute.
  إذا ((ریښه_Name || is_style_posix(Style::native)) && ریښه_Directory)
    أرجع;

  // All the following conditions will need the current directory.
  SmallString<128> current_dir;
  current_directory.منزل_ویکټور(current_dir);

  // Relative path. Prepend the current directory.
  إذا (!ریښه_Name && !ریښه_Directory) {
    // Append path to the current directory.
    append(current_dir, p);
    // Set path to the result.
    لار_2.بدلون(current_dir);
    أرجع;
  }

  إذا (!ریښه_Name && ریښه_Directory) {
    StringRef cdrn = ریښه_name(current_dir);
    SmallString<128> cur_Dir_ریښه_Name(cdrn.begin(), cdrn.پای_ټکی());
    append(cur_Dir_ریښه_Name, p);
    // Set path to the result.
    لار_2.بدلون(cur_Dir_ریښه_Name);
    أرجع;
  }

  إذا (ریښه_Name && !ریښه_Directory) {
    StringRef p_ریښه_Name = ریښه_name(p);
    StringRef b_ریښه_Directory = ریښه_directory(current_dir);
    StringRef b_Relative_لار = relative_لار(current_dir);
    StringRef p_Relative_لار = relative_لار(p);

    SmallString<128> res;
    append(res, p_ریښه_Name, b_ریښه_Directory, b_Relative_لار, p_Relative_لار);
    لار_2.بدلون(res);
    أرجع;
  }

  llvm_unreachable("All rootName and rootDirectory combinations should have "
                   "occurred above!");
}

StringRef remove_leading_dotslash(StringRef لار, Style style) {
  // Remove leading "./" (or ".//" or "././" etc.)
  طالما (لار.اندازه_2() > 2 && لار[0] == '.' && is_separator(لار[1], style)) {
    لار = لار.substr(2);
    طالما (لار.اندازه_2() > 0 && is_separator(لار[0], style))
      لار = لار.substr(1);
  }
  أرجع لار;
}

منطقي remove_dots(Small_ویکټور_Impl<حرف> &the_لار, منطقي remove_dot_dot,
                 Style style) {
  style = real_style(style);
  StringRef پاتې_شوی_2(the_لار.معلومات(), the_لار.اندازه_2());
  منطقي needs_change = خطأ;
  Small_ویکټور<StringRef, 16> components;

  // Consume the root path, if present.
  StringRef ریښه_3 = لار_2::ریښه_لار_2(پاتې_شوی_2, style);
  منطقي absolute = !ریښه_3.تشه();
  إذا (absolute)
    پاتې_شوی_2 = پاتې_شوی_2.drop_front(ریښه_3.اندازه_2());

  // Loop over path components manually. This makes it easier to detect
  // non-preferred slashes and double separators that must be canonicalized.
  طالما (!پاتې_شوی_2.تشه()) {
    size_t بل_slash = پاتې_شوی_2.پیدا_کړه_لومړی_د(separators(style));
    إذا (بل_slash == StringRef::npos)
      بل_slash = پاتې_شوی_2.اندازه_2();
    StringRef component = پاتې_شوی_2.take_front(بل_slash);
    پاتې_شوی_2 = پاتې_شوی_2.drop_front(بل_slash);

    // Eat the slash, and check if it is the preferred separator.
    إذا (!پاتې_شوی_2.تشه()) {
      needs_change |= پاتې_شوی_2.front() != preferred_separator(style);
      پاتې_شوی_2 = پاتې_شوی_2.drop_front();
      // The path needs to be rewritten if it has a trailing slash.
      // FIXME: This is emergent behavior that could be removed.
      needs_change |= پاتې_شوی_2.تشه();
    }

    // Check for path traversal components or double separators.
    إذا (component.تشه() || component == ".") {
      needs_change = ریښتیا;
    } وإلا إذا (remove_dot_dot && component == "..") {
      needs_change = ریښتیا;
      // Do not allow ".." to remove the root component. If this is the
      // beginning of a relative path, keep the ".." component.
      إذا (!components.تشه() && components.شا() != "..") {
        components.راوباسه_شا();
      } وإلا إذا (!absolute) {
        components.ټېل_شا(component);
      }
    } وإلا {
      components.ټېل_شا(component);
    }
  }

  SmallString<256> بفر_2 = ریښه_3;
  // "root" could be "/", which may need to be translated into "\".
  make_preferred(بفر_2, style);
  needs_change |= ریښه_3 != بفر_2;

  // Avoid rewriting the path unless we have to.
  إذا (!needs_change)
    أرجع خطأ;

  إذا (!components.تشه()) {
    بفر_2 += components[0];
    لكل (StringRef C : ArrayRef(components).drop_front()) {
      بفر_2 += preferred_separator(style);
      بفر_2 += C;
    }
  }
  the_لار.بدلون(بفر_2);
  أرجع ریښتیا;
}

} // end namespace path

نطاق fs {

std::error_کوډ getUniqueID(ثابت Twine لار, UniqueID &پایله) {
  sandbox::violationIfEnabled();

  file_حالت حالت;
  std::error_کوډ EC = حالت_2(لار, حالت);
  إذا (EC)
    أرجع EC;
  پایله = حالت.getUniqueID();
  أرجع std::error_کوډ();
}

فراغ create_Unique_لار(ثابت Twine &Model, Small_ویکټور_Impl<حرف> &پایله_لار,
                      منطقي MakeAbsolute) {
  SmallString<128> ModelStorage;
  Model.منزل_ویکټور(ModelStorage);

  أكد(llvm::is_contained(ModelStorage, '%') &&
         "createUniquePath: Model must contain at least one '%'");

  إذا (MakeAbsolute) {
    // Make model absolute by prepending a temp directory if it's not already.
    إذا (!sys::لار_2::is_absolute(Twine(ModelStorage))) {
      SmallString<128> TDir;
      sys::لار_2::system_لنډمهاله_directory(ریښتیا, TDir);
      sys::لار_2::append(TDir, Twine(ModelStorage));
      ModelStorage.بدلون(TDir);
    }
  }

  پایله_لار = ModelStorage;
  پایله_لار.ټېل_شا(0);
  پایله_لار.راوباسه_شا();

  // Replace '%' with random chars.
  لكل (بینښه i = 0, e = ModelStorage.اندازه_2(); i != e; ++i) {
    إذا (ModelStorage[i] == '%')
      پایله_لار[i] = "0123456789abcdef"[sys::Process::Get_Random_شمېره() & 15];
  }
}

std::error_کوډ createUniqueFile(ثابت Twine &Model, صحيح &پایله_Fd,
                                 Small_ویکټور_Impl<حرف> &پایله_لار,
                                 OpenFlags Flags, بینښه ډېرپېښ) {
  أرجع createUniqueEntity(Model, پایله_Fd, پایله_لار, خطأ, FS_File, Flags,
                            ډېرپېښ);
}

std::error_کوډ createUniqueFile(ثابت Twine &Model,
                                 Small_ویکټور_Impl<حرف> &پایله_لار,
                                 بینښه ډېرپېښ) {
  صحيح FD;
  خپلکاری EC = createUniqueFile(Model, FD, پایله_لار, د_None, ډېرپېښ);
  إذا (EC)
    أرجع EC;
  // FD is only needed to avoid race conditions. Close it right away.
  close(FD);
  أرجع EC;
}

ساكن std::error_کوډ
createTemporaryFile(ثابت Twine &Model, صحيح &پایله_FD,
                    llvm::Small_ویکټور_Impl<حرف> &پایله_لار, FSEntity Type,
                    sys::fs::OpenFlags Flags = sys::fs::د_None) {
  // Any *temporary* file is assumed to be a compiler-internal output, not
  // a formal one.
  خپلکاری BypassSandbox = sys::sandbox::scopedDisable();

  SmallString<128> Storage;
  StringRef P = Model.منزل_Null_Terminated_String_Ref(Storage);
  أكد(P.پیدا_کړه_لومړی_د(separators(Style::native)) == StringRef::npos &&
         "Model must be a simple filename.");
  // Use P.begin() so that createUniqueEntity doesn't need to recreate Storage.
  أرجع createUniqueEntity(P.begin(), پایله_FD, پایله_لار, ریښتیا, Type, Flags,
                            all_read | all_write);
}

ساكن std::error_کوډ
createTemporaryFile(ثابت Twine &مختاړی, StringRef Suffix, صحيح &پایله_FD,
                    llvm::Small_ویکټور_Impl<حرف> &پایله_لار, FSEntity Type,
                    sys::fs::OpenFlags Flags = sys::fs::د_None) {
  ثابت حرف *منځ = Suffix.تشه() ? "-%%%%%%" : "-%%%%%%.";
  أرجع createTemporaryFile(مختاړی + منځ + Suffix, پایله_FD, پایله_لار,
                             Type, Flags);
}

std::error_کوډ createTemporaryFile(ثابت Twine &مختاړی, StringRef Suffix,
                                    صحيح &پایله_FD,
                                    Small_ویکټور_Impl<حرف> &پایله_لار,
                                    sys::fs::OpenFlags Flags) {
  أرجع createTemporaryFile(مختاړی, Suffix, پایله_FD, پایله_لار, FS_File,
                             Flags);
}

std::error_کوډ createTemporaryFile(ثابت Twine &مختاړی, StringRef Suffix,
                                    Small_ویکټور_Impl<حرف> &پایله_لار,
                                    sys::fs::OpenFlags Flags) {
  صحيح FD;
  خپلکاری EC = createTemporaryFile(مختاړی, Suffix, FD, پایله_لار, Flags);
  إذا (EC)
    أرجع EC;
  // FD is only needed to avoid race conditions. Close it right away.
  close(FD);
  أرجع EC;
}

// This is a mkdtemp with a different pattern. We use createUniqueEntity mostly
// for consistency. We should try using mkdtemp.
std::error_کوډ createUniqueDirectory(ثابت Twine &مختاړی,
                                      Small_ویکټور_Impl<حرف> &پایله_لار) {
  صحيح Dummy;
  أرجع createUniqueEntity(مختاړی + "-%%%%%%", Dummy, پایله_لار, ریښتیا,
                            FS_Dir);
}

std::error_کوډ
getPotentiallyUniqueFileName(ثابت Twine &Model,
                             Small_ویکټور_Impl<حرف> &پایله_لار) {
  صحيح Dummy;
  أرجع createUniqueEntity(Model, Dummy, پایله_لار, خطأ, FS_Name);
}

std::error_کوډ
get_Potentially_Unique_لنډمهاله_File_Name(ثابت Twine &مختاړی, StringRef Suffix,
                                 Small_ویکټور_Impl<حرف> &پایله_لار) {
  صحيح Dummy;
  أرجع createTemporaryFile(مختاړی, Suffix, Dummy, پایله_لار, FS_Name);
}

std::error_کوډ make_absolute(Small_ویکټور_Impl<حرف> &لار_2) {
  sandbox::violationIfEnabled();

  إذا (لار_2::is_absolute(لار_2))
    أرجع {};

  SmallString<128> current_dir;
  إذا (std::error_کوډ ec = current_لار(current_dir))
    أرجع ec;

  لار_2::make_absolute(current_dir, لار_2);
  أرجع {};
}

std::error_کوډ create_directories(ثابت Twine &لار, منطقي IgnoreExisting,
                                   perms Perms) {
  SmallString<128> لار_Storage;
  StringRef P = لار.منزل_String_Ref(لار_Storage);

  // Be optimistic and try to create the directory
  std::error_کوډ EC = create_directory(P, IgnoreExisting, Perms);
  // If we succeeded, or had any error other than the parent not existing, just
  // return it.
  إذا (EC != errc::no_such_file_or_directory)
    أرجع EC;

  // We failed because of a no_such_file_or_directory, try to create the
  // parent.
  StringRef پلار_2 = لار_2::پلار_لار(P);
  إذا (پلار_2.تشه())
    أرجع EC;

  إذا ((EC = create_directories(پلار_2, IgnoreExisting, Perms)))
      أرجع EC;

  أرجع create_directory(P, IgnoreExisting, Perms);
}

ساكن std::error_کوډ copy_file_دننی(صحيح ReadFD, صحيح WriteFD) {
  ثابت size_t Buf_اندازه = 4096;
  حرف *Buf = جديد حرف[Buf_اندازه];
  صحيح بایټونه_Read = 0, بایټونه_Written = 0;
  لكل (;;) {
    بایټونه_Read = اقرأ(ReadFD, Buf, Buf_اندازه);
    إذا (بایټونه_Read <= 0)
      اكسر;
    طالما (بایټونه_Read) {
      بایټونه_Written = اكتب(WriteFD, Buf, بایټونه_Read);
      إذا (بایټونه_Written < 0)
        اكسر;
      بایټونه_Read -= بایټونه_Written;
    }
    إذا (بایټونه_Written < 0)
      اكسر;
  }
  ړنګول[] Buf;

  إذا (بایټونه_Read < 0 || بایټونه_Written < 0)
    أرجع errno_As_Error_کوډ();
  أرجع std::error_کوډ();
}

#که_نه_تعریف_شوی __APPLE__
std::error_کوډ copy_file(ثابت Twine &From, ثابت Twine &منزل) {
  صحيح ReadFD, WriteFD;
  إذا (std::error_کوډ EC = openFileForRead(From, ReadFD, د_None))
    أرجع EC;
  إذا (std::error_کوډ EC =
          openFileForWrite(منزل, WriteFD, CD_CreateAlways, د_None)) {
    close(ReadFD);
    أرجع EC;
  }

  std::error_کوډ EC = copy_file_دننی(ReadFD, WriteFD);

  close(ReadFD);
  close(WriteFD);

  أرجع EC;
}
#د_که_پای

std::error_کوډ copy_file(ثابت Twine &From, صحيح منزل_FD) {
  صحيح ReadFD;
  إذا (std::error_کوډ EC = openFileForRead(From, ReadFD, د_None))
    أرجع EC;

  std::error_کوډ EC = copy_file_دننی(ReadFD, منزل_FD);

  close(ReadFD);

  أرجع EC;
}

ErrorOr<MD5::MD5_پایله> md5_contents(صحيح FD) {
  sandbox::violationIfEnabled();

  MD5 Hash;

  constexpr size_t Buf_اندازه = 4096;
  std::ویکټور<uint8_t> Buf(Buf_اندازه);
  صحيح بایټونه_Read = 0;
  لكل (;;) {
    بایټونه_Read = اقرأ(FD, Buf.معلومات(), Buf_اندازه);
    إذا (بایټونه_Read <= 0)
      اكسر;
    Hash.update(ArrayRef(Buf.معلومات(), بایټونه_Read));
  }

  إذا (بایټونه_Read < 0)
    أرجع errno_As_Error_کوډ();
  MD5::MD5_پایله پایله;
  Hash.final(پایله);
  أرجع پایله;
}

ErrorOr<MD5::MD5_پایله> md5_contents(ثابت Twine &لار) {
  sandbox::violationIfEnabled();

  صحيح FD;
  إذا (خپلکاری EC = openFileForRead(لار, FD, د_None))
    أرجع EC;

  خپلکاری پایله = md5_contents(FD);
  close(FD);
  أرجع پایله;
}

منطقي exists(ثابت basic_file_حالت &حالت_2) {
  أرجع حالت_معلوم(حالت_2) && حالت_2.type() != file_type::file_not_وموندل;
}

منطقي حالت_معلوم(ثابت basic_file_حالت &s) {
  أرجع s.type() != file_type::حالت_error;
}

file_type get_file_type(ثابت Twine &لار, منطقي Follow) {
  file_حالت st;
  إذا (حالت_2(لار, st, Follow))
    أرجع file_type::حالت_error;
  أرجع st.type();
}

منطقي is_directory(ثابت basic_file_حالت &حالت_2) {
  أرجع حالت_2.type() == file_type::directory_file;
}

std::error_کوډ is_directory(ثابت Twine &لار_2, منطقي &پایله_2) {
  sandbox::violationIfEnabled();

  file_حالت st;
  إذا (std::error_کوډ ec = حالت_2(لار_2, st))
    أرجع ec;
  پایله_2 = is_directory(st);
  أرجع std::error_کوډ();
}

منطقي is_regular_file(ثابت basic_file_حالت &حالت_2) {
  أرجع حالت_2.type() == file_type::regular_file;
}

std::error_کوډ is_regular_file(ثابت Twine &لار_2, منطقي &پایله_2) {
  sandbox::violationIfEnabled();

  file_حالت st;
  إذا (std::error_کوډ ec = حالت_2(لار_2, st))
    أرجع ec;
  پایله_2 = is_regular_file(st);
  أرجع std::error_کوډ();
}

منطقي is_symlink_file(ثابت basic_file_حالت &حالت_2) {
  أرجع حالت_2.type() == file_type::symlink_file;
}

std::error_کوډ is_symlink_file(ثابت Twine &لار_2, منطقي &پایله_2) {
  sandbox::violationIfEnabled();

  file_حالت st;
  إذا (std::error_کوډ ec = حالت_2(لار_2, st, خطأ))
    أرجع ec;
  پایله_2 = is_symlink_file(st);
  أرجع std::error_کوډ();
}

منطقي is_other(ثابت basic_file_حالت &حالت_2) {
  أرجع exists(حالت_2) &&
         !is_regular_file(حالت_2) &&
         !is_directory(حالت_2);
}

std::error_کوډ is_other(ثابت Twine &لار, منطقي &پایله) {
  sandbox::violationIfEnabled();

  file_حالت File_حالت;
  إذا (std::error_کوډ EC = حالت_2(لار, File_حالت))
    أرجع EC;
  پایله = is_other(File_حالت);
  أرجع std::error_کوډ();
}

فراغ directory_ثبت::replace_filename(ثابت Twine &Filename, file_type Type,
                                       basic_file_حالت حالت) {
  SmallString<128> لار_Str = لار_2::پلار_لار(لار);
  لار_2::append(لار_Str, Filename);
  هذا->لار = std::string(لار_Str);
  هذا->Type = Type;
  هذا->حالت = حالت;
}

ErrorOr<perms> getPermissions(ثابت Twine &لار) {
  sandbox::violationIfEnabled();

  file_حالت حالت;
  إذا (std::error_کوډ EC = حالت_2(لار, حالت))
    أرجع EC;

  أرجع حالت.permissions();
}

std::error_کوډ سېټ_وروستی_Access_And_Modification_وخت(ثابت Twine &لار,
                                                 وخت_ټکی<> Access_وخت,
                                                 وخت_ټکی<> Modification_وخت) {
  صحيح FD;
  إذا (std::error_کوډ EC = openFile(لار, FD, CD_OpenExisting, FA_Read,
                                    د_Update_ځانګړتیاوې | د_Open_Directory))
    أرجع EC;
  std::error_کوډ EC =
      سېټ_وروستی_Access_And_Modification_وخت(FD, Access_وخت, Modification_وخت);
  Process::SafelyCloseFileDescriptor(FD);
  أرجع EC;
}

size_t mapped_file_region::اندازه_2() ثابت {
  أكد(Mapping && "Mapping failed but used anyway!");
  أرجع اندازه;
}

حرف *mapped_file_region::معلومات() ثابت {
  أكد(Mapping && "Mapping failed but used anyway!");
  أرجع reinterpret_cast<حرف *>(Mapping);
}

ثابت حرف *mapped_file_region::const_معلومات() ثابت {
  أكد(Mapping && "Mapping failed but used anyway!");
  أرجع reinterpret_cast<ثابت حرف *>(Mapping);
}

Error read_Native_File_منزل_EOF(file_t FileHandle, Small_ویکټور_Impl<حرف> &بفر,
                          ssize_t Chunk_اندازه) {
  sandbox::violationIfEnabled();

  // Install a handler to truncate the buffer to the correct size on exit.
  size_t اندازه = بفر.اندازه_2();
  llvm::scope_exit TruncateOnExit([&]() { بفر.truncate(اندازه); });

  // Read into Buffer until we hit EOF.
  لكل (;;) {
    بفر.resize_for_overwrite(اندازه + Chunk_اندازه);
    Expected<size_t> Read_بایټونه = readNativeFile(
        FileHandle, MutableArrayRef(بفر.begin() + اندازه, Chunk_اندازه));
    إذا (!Read_بایټونه)
      أرجع Read_بایټونه.takeError();
    إذا (*Read_بایټونه == 0)
      أرجع Error::success();
    اندازه += *Read_بایټونه;
  }
}

} // end namespace fs
} // end namespace sys
} // end namespace llvm

// Include the truly platform-specific parts.
#إذا defined(LLVM_ON_UNIX)
#ضمن "Unix/Path.inc"
#د_که_پای
#إذا defined(_WIN32)
#ضمن "Windows/Path.inc"
#د_که_پای

نطاق llvm {
نطاق sys {
نطاق fs {

لنډمهاله_File::لنډمهاله_File(StringRef Name, صحيح FD)
    : TmpName(std::string(Name)), FD(FD) {}
لنډمهاله_File::لنډمهاله_File(لنډمهاله_File &&Other) { *هذا = std::move(Other); }
لنډمهاله_File &لنډمهاله_File::چلوونکی=(لنډمهاله_File &&Other) {
  TmpName = std::move(Other.TmpName);
  FD = Other.FD;
  Other.بشپړ = ریښتیا;
  Other.FD = -1;
#که_تعریف_شوی _WIN32
  RemoveOnClose = Other.RemoveOnClose;
  Other.RemoveOnClose = خطأ;
#د_که_پای
  أرجع *هذا;
}

لنډمهاله_File::~لنډمهاله_File() { أكد(بشپړ); }

Error لنډمهاله_File::discard() {
  بشپړ = ریښتیا;
  إذا (FD != -1 && close(FD) == -1) {
    std::error_کوډ EC = errno_As_Error_کوډ();
    أرجع error_کوډ_منزل_Error(EC);
  }
  FD = -1;

#که_تعریف_شوی _WIN32
  // On Windows, closing will remove the file, if we set the delete
  // disposition. If not, remove it manually.
  منطقي Remove = RemoveOnClose;
#وإلا
  // Always try to remove the file.
  منطقي Remove = ریښتیا;
#د_که_پای
  std::error_کوډ RemoveEC;
  إذا (Remove && !TmpName.تشه()) {
    RemoveEC = fs::remove(TmpName);
    sys::DontRemoveFileOnSignal(TmpName);
    إذا (!RemoveEC)
      TmpName = "";
  } وإلا {
    TmpName = "";
  }
  أرجع error_کوډ_منزل_Error(RemoveEC);
}

Error لنډمهاله_File::keep(ثابت Twine &Name) {
  أكد(!بشپړ);
  بشپړ = ریښتیا;
  // Always try to close and rename.
#که_تعریف_شوی _WIN32
  // If we can't cancel the delete don't rename.
  خپلکاری H = reinterpret_cast<HANDLE>(_get_osfhandle(FD));
  std::error_کوډ RenameEC =
      RemoveOnClose ? std::error_کوډ() : سېټ_Delete_Disposition(H, خطأ);
  منطقي ShouldDelete = خطأ;
  إذا (!RenameEC) {
    RenameEC = rename_handle(H, Name);
    // If rename failed because it's cross-device, copy instead
    إذا (RenameEC ==
      std::error_کوډ(ERROR_NOT_SAME_DEVICE, std::system_category())) {
      RenameEC = copy_file(TmpName, Name);
      ShouldDelete = ریښتیا;
    }
  }

  // If we can't rename or copy, discard the temporary file.
  إذا (RenameEC)
    ShouldDelete = ریښتیا;
  إذا (ShouldDelete) {
    إذا (!RemoveOnClose)
      سېټ_Delete_Disposition(H, ریښتیا);
    وإلا
      remove(TmpName);
  }
#وإلا
  std::error_کوډ RenameEC = fs::rename(TmpName, Name);
  إذا (RenameEC) {
    // If we can't rename, try to copy to work around cross-device link issues.
    RenameEC = sys::fs::copy_file(TmpName, Name);
    // If we can't rename or copy, discard the temporary file.
    إذا (RenameEC)
      remove(TmpName);
  }
#د_که_پای
  sys::DontRemoveFileOnSignal(TmpName);

  إذا (!RenameEC)
    TmpName = "";

  إذا (close(FD) == -1)
    أرجع error_کوډ_منزل_Error(errno_As_Error_کوډ());
  FD = -1;

  أرجع error_کوډ_منزل_Error(RenameEC);
}

Error لنډمهاله_File::keep() {
  أكد(!بشپړ);
  بشپړ = ریښتیا;

#که_تعریف_شوی _WIN32
  خپلکاری H = reinterpret_cast<HANDLE>(_get_osfhandle(FD));
  إذا (std::error_کوډ EC = سېټ_Delete_Disposition(H, خطأ))
    أرجع error_کوډ_منزل_Error(EC);
#د_که_پای
  sys::DontRemoveFileOnSignal(TmpName);

  TmpName = "";

  إذا (close(FD) == -1)
    أرجع error_کوډ_منزل_Error(errno_As_Error_کوډ());
  FD = -1;

  أرجع Error::success();
}

Expected<لنډمهاله_File> لنډمهاله_File::create(ثابت Twine &Model, بینښه ډېرپېښ,
                                    OpenFlags ExtraFlags) {
  صحيح FD;
  SmallString<128> پایله_لار;
  إذا (std::error_کوډ EC =
          createUniqueFile(Model, FD, پایله_لار, د_Delete | ExtraFlags, ډېرپېښ))
    أرجع error_کوډ_منزل_Error(EC);

  لنډمهاله_File Ret(پایله_لار, FD);
#که_تعریف_شوی _WIN32
  خپلکاری H = reinterpret_cast<HANDLE>(_get_osfhandle(FD));
  منطقي سېټ_Signal_Handler = خطأ;
  إذا (std::error_کوډ EC = سېټ_Delete_Disposition(H, ریښتیا)) {
    Ret.RemoveOnClose = ریښتیا;
    سېټ_Signal_Handler = ریښتیا;
  }
#وإلا
  منطقي سېټ_Signal_Handler = ریښتیا;
#د_که_پای
  إذا (سېټ_Signal_Handler && sys::RemoveFileOnSignal(پایله_لار)) {
    // Make sure we delete the file when RemoveFileOnSignal fails.
    consumeError(Ret.discard());
    std::error_کوډ EC(errc::operation_not_permitted);
    أرجع error_کوډ_منزل_Error(EC);
  }
  أرجع std::move(Ret);
}
} // namespace fs

} // namespace sys
} // namespace llvm
