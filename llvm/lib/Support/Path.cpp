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
#جے_انت

ورتوں نطاق llvm;
ورتوں نطاق llvm::support::endian;

نطاق {
  ورتوں llvm::StringRef;
  ورتوں llvm::sys::رستہ_2::is_separator;
  ورتوں llvm::sys::رستہ_2::Style;

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

  StringRef لبھو_پہلا_component(StringRef رستہ_2, Style style) {
    // Look for this first component in the following order.
    // * empty (in this case we return an empty string)
    // * either C: or {//,\\}net.
    // * {/,\}
    // * {file,directory}name

    إذا (رستہ_2.خالی_تھاواں())
      أرجع رستہ_2;

    إذا (is_style_windows(style)) {
      // C:
      إذا (رستہ_2.ناپ_2() >= 2 &&
          std::حرف_اے(static_cast<بنانشانی حرف>(رستہ_2[0])) && رستہ_2[1] == ':')
        أرجع رستہ_2.substr(0, 2);
    }

    // //net
    إذا ((رستہ_2.ناپ_2() > 2) && is_separator(رستہ_2[0], style) &&
        رستہ_2[0] == رستہ_2[1] && !is_separator(رستہ_2[2], style)) {
      // Find the next directory separator.
      size_t سرا = رستہ_2.لبھو_پہلا_دا(separators(style), 2);
      أرجع رستہ_2.substr(0, سرا);
    }

    // {/,\}
    إذا (is_separator(رستہ_2[0], style))
      أرجع رستہ_2.substr(0, 1);

    // * {file,directory}name
    size_t سرا = رستہ_2.لبھو_پہلا_دا(separators(style));
    أرجع رستہ_2.substr(0, سرا);
  }

  // Returns the first character of the filename in str. For paths ending in
  // '/', it returns the position of the '/'.
  size_t filename_pos(StringRef str, Style style) {
    إذا (str.ناپ_2() > 0 && is_separator(str[str.ناپ_2() - 1], style))
      أرجع str.ناپ_2() - 1;

    size_t pos = str.لبھو_آخری_دا(separators(style), str.ناپ_2() - 1);

    إذا (is_style_windows(style)) {
      إذا (pos == StringRef::npos)
        pos = str.لبھو_آخری_دا(':', str.ناپ_2() - 1);
    }

    إذا (pos == StringRef::npos || (pos == 1 && is_separator(str[0], style)))
      أرجع 0;

    أرجع pos + 1;
  }

  // Returns the position of the root directory in str. If there is no root
  // directory in str, it returns StringRef::npos.
  size_t جڑ_dir_شروع(StringRef str, Style style) {
    // case "c:/"
    إذا (is_style_windows(style)) {
      إذا (str.ناپ_2() > 2 && str[1] == ':' && is_separator(str[2], style))
        أرجع 2;
    }

    // case "//net"
    إذا (str.ناپ_2() > 3 && is_separator(str[0], style) && str[0] == str[1] &&
        !is_separator(str[2], style)) {
      أرجع str.لبھو_پہلا_دا(separators(style), 2);
    }

    // case "/"
    إذا (str.ناپ_2() > 0 && is_separator(str[0], style))
      أرجع 0;

    أرجع StringRef::npos;
  }

  // Returns the position past the end of the "parent path" of path. The parent
  // path will not end in '/', unless the parent is the root directory. If the
  // path has no parent, 0 is returned.
  size_t ماپا_رستہ_سرا(StringRef رستہ_2, Style style) {
    size_t سرا_pos = filename_pos(رستہ_2, style);

    منطقي filename_was_sep =
        رستہ_2.ناپ_2() > 0 && is_separator(رستہ_2[سرا_pos], style);

    // Skip separators until we reach root dir (or the start of the string).
    size_t جڑ_dir_pos = جڑ_dir_شروع(رستہ_2, style);
    طالما (سرا_pos > 0 &&
           (جڑ_dir_pos == StringRef::npos || سرا_pos > جڑ_dir_pos) &&
           is_separator(رستہ_2[سرا_pos - 1], style))
      --سرا_pos;

    إذا (سرا_pos == جڑ_dir_pos && !filename_was_sep) {
      // We've reached the root dir and the input path was *not* ending in a
      // sequence of slashes. Include the root dir in the parent path.
      أرجع جڑ_dir_pos + 1;
    }

    // Otherwise, just include before the last slash.
    أرجع سرا_pos;
  }
} // end unnamed namespace

تعداد FSEntity {
  FS_Dir,
  FS_File,
  FS_Name
};

ساكن std::error_کوڈ
createUniqueEntity(ثابت Twine &Model, صحيح &نتیجہ_FD,
                   Small_سمتیہ_Impl<حرف> &نتیجہ_رستہ, منطقي MakeAbsolute,
                   FSEntity Type, sys::fs::OpenFlags Flags = sys::fs::دا_None,
                   بنانشانی منوال = 0) {

  // Limit the number of attempts we make, so that we don't infinite loop. E.g.
  // "permission denied" could be for a specific file (so we retry with a
  // different name) or for the whole directory (retry would always fail).
  // Checking which is racy, so we try a number of times, then give up.
  std::error_کوڈ EC;
  لكل (صحيح Retries = 128; Retries > 0; --Retries) {
    sys::fs::create_Unique_رستہ(Model, نتیجہ_رستہ, MakeAbsolute);
    // Try to open + create the file.
    بدل (Type) {
    حالة FS_File: {
      EC = sys::fs::openFileForReadWrite(Twine(نتیجہ_رستہ.begin()), نتیجہ_FD,
                                         sys::fs::CD_CreateNew, Flags, منوال);
      إذا (EC) {
        // errc::permission_denied happens on Windows when we try to open a file
        // that has been marked for deletion.
        إذا (EC == errc::file_exists || EC == errc::permission_denied)
          استمر;
        أرجع EC;
      }

      أرجع std::error_کوڈ();
    }

    حالة FS_Name: {
      EC = sys::fs::access(نتیجہ_رستہ.begin(), sys::fs::Access_منوال::Exist);
      إذا (EC == errc::no_such_file_or_directory)
        أرجع std::error_کوڈ();
      إذا (EC)
        أرجع EC;
      استمر;
    }

    حالة FS_Dir: {
      EC = sys::fs::create_directory(نتیجہ_رستہ.begin(), خطأ);
      إذا (EC) {
        إذا (EC == errc::file_exists)
          استمر;
        أرجع EC;
      }
      أرجع std::error_کوڈ();
    }
    }
    llvm_unreachable("Invalid Type");
  }
  أرجع EC;
}

نطاق llvm {
نطاق sys  {
نطاق رستہ_2 {

const_iterator begin(StringRef رستہ_2, Style style) {
  const_iterator i;
  i.رستہ      = رستہ_2;
  i.Component = لبھو_پہلا_component(رستہ_2, style);
  i.Position  = 0;
  i.S = style;
  أرجع i;
}

const_iterator سرا(StringRef رستہ_2) {
  const_iterator i;
  i.رستہ      = رستہ_2;
  i.Position  = رستہ_2.ناپ_2();
  أرجع i;
}

const_iterator &const_iterator::عامل++() {
  أكد(Position < رستہ.ناپ_2() && "Tried to increment past end!");

  // Increment Position to past the current component
  Position += Component.ناپ_2();

  // Check for end.
  إذا (Position == رستہ.ناپ_2()) {
    Component = StringRef();
    أرجع *هذا;
  }

  // Both POSIX and Windows treat paths that begin with exactly two separators
  // specially.
  منطقي was_net = Component.ناپ_2() > 2 && is_separator(Component[0], S) &&
                 Component[1] == Component[0] && !is_separator(Component[2], S);

  // Handle separators.
  إذا (is_separator(رستہ[Position], S)) {
    // Root dir.
    إذا (was_net ||
        // c:/
        (is_style_windows(S) && Component.سرے_with(":"))) {
      Component = رستہ.substr(Position, 1);
      أرجع *هذا;
    }

    // Skip extra separators.
    طالما (Position != رستہ.ناپ_2() && is_separator(رستہ[Position], S)) {
      ++Position;
    }

    // Treat trailing '/' as a '.', unless it is the root dir.
    إذا (Position == رستہ.ناپ_2() && Component != "/") {
      --Position;
      Component = ".";
      أرجع *هذا;
    }
  }

  // Find next component.
  size_t سرا_pos = رستہ.لبھو_پہلا_دا(separators(S), Position);
  Component = رستہ.ٹکڑا(Position, سرا_pos);

  أرجع *هذا;
}

منطقي const_iterator::عامل==(ثابت const_iterator &RHS) ثابت {
  أرجع رستہ.begin() == RHS.رستہ.begin() && Position == RHS.Position;
}

ptrdiff_t const_iterator::عامل-(ثابت const_iterator &RHS) ثابت {
  أرجع Position - RHS.Position;
}

الٹ_iterator rbegin(StringRef رستہ, Style style) {
  الٹ_iterator I;
  I.رستہ = رستہ;
  I.Position = رستہ.ناپ_2();
  I.S = style;
  ++I;
  أرجع I;
}

الٹ_iterator rend(StringRef رستہ) {
  الٹ_iterator I;
  I.رستہ = رستہ;
  I.Component = رستہ.substr(0, 0);
  I.Position = 0;
  أرجع I;
}

الٹ_iterator &الٹ_iterator::عامل++() {
  size_t جڑ_dir_pos = جڑ_dir_شروع(رستہ, S);

  // Skip separators unless it's the root directory.
  size_t سرا_pos = Position;
  طالما (سرا_pos > 0 && (سرا_pos - 1) != جڑ_dir_pos &&
         is_separator(رستہ[سرا_pos - 1], S))
    --سرا_pos;

  // Treat trailing '/' as a '.', unless it is the root dir.
  إذا (Position == رستہ.ناپ_2() && !رستہ.خالی_تھاواں() &&
      is_separator(رستہ.پٹھ(), S) &&
      (جڑ_dir_pos == StringRef::npos || سرا_pos - 1 > جڑ_dir_pos)) {
    --Position;
    Component = ".";
    أرجع *هذا;
  }

  // Find next separator.
  size_t شروع_pos = filename_pos(رستہ.substr(0, سرا_pos), S);
  Component = رستہ.ٹکڑا(شروع_pos, سرا_pos);
  Position = شروع_pos;
  أرجع *هذا;
}

منطقي الٹ_iterator::عامل==(ثابت الٹ_iterator &RHS) ثابت {
  أرجع رستہ.begin() == RHS.رستہ.begin() && Component == RHS.Component &&
         Position == RHS.Position;
}

ptrdiff_t الٹ_iterator::عامل-(ثابت الٹ_iterator &RHS) ثابت {
  أرجع Position - RHS.Position;
}

StringRef جڑ_رستہ_2(StringRef رستہ_2, Style style) {
  const_iterator b = begin(رستہ_2, style), pos = b, e = سرا(رستہ_2);
  إذا (b != e) {
    منطقي has_net =
        b->ناپ_2() > 2 && is_separator((*b)[0], style) && (*b)[1] == (*b)[0];
    منطقي has_drive = is_style_windows(style) && b->سرے_with(":");

    إذا (has_net || has_drive) {
      إذا ((++pos != e) && is_separator((*pos)[0], style)) {
        // {C:/,//net/}, so get the first two components.
        أرجع رستہ_2.substr(0, b->ناپ_2() + pos->ناپ_2());
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

StringRef جڑ_name(StringRef رستہ_2, Style style) {
  const_iterator b = begin(رستہ_2, style), e = سرا(رستہ_2);
  إذا (b != e) {
    منطقي has_net =
        b->ناپ_2() > 2 && is_separator((*b)[0], style) && (*b)[1] == (*b)[0];
    منطقي has_drive = is_style_windows(style) && b->سرے_with(":");

    إذا (has_net || has_drive) {
      // just {C:,//net}, return the first component.
      أرجع *b;
    }
  }

  // No path or no name.
  أرجع StringRef();
}

StringRef جڑ_directory(StringRef رستہ_2, Style style) {
  const_iterator b = begin(رستہ_2, style), pos = b, e = سرا(رستہ_2);
  إذا (b != e) {
    منطقي has_net =
        b->ناپ_2() > 2 && is_separator((*b)[0], style) && (*b)[1] == (*b)[0];
    منطقي has_drive = is_style_windows(style) && b->سرے_with(":");

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

StringRef relative_رستہ(StringRef رستہ_2, Style style) {
  StringRef جڑ_3 = جڑ_رستہ_2(رستہ_2, style);
  أرجع رستہ_2.substr(جڑ_3.ناپ_2());
}

فراغ append(Small_سمتیہ_Impl<حرف> &رستہ_2, Style style, ثابت Twine &a,
            ثابت Twine &b, ثابت Twine &c, ثابت Twine &d) {
  SmallString<32> a_storage;
  SmallString<32> b_storage;
  SmallString<32> c_storage;
  SmallString<32> d_storage;

  Small_سمتیہ<StringRef, 4> components;
  إذا (!a.is_Trivially_خالی_تھاواں()) components.ڈالو_پٹھ(a.منزل_String_Ref(a_storage));
  إذا (!b.is_Trivially_خالی_تھاواں()) components.ڈالو_پٹھ(b.منزل_String_Ref(b_storage));
  إذا (!c.is_Trivially_خالی_تھاواں()) components.ڈالو_پٹھ(c.منزل_String_Ref(c_storage));
  إذا (!d.is_Trivially_خالی_تھاواں()) components.ڈالو_پٹھ(d.منزل_String_Ref(d_storage));

  لكل (خودکار &component : components) {
    منطقي رستہ_has_sep =
        !رستہ_2.خالی_تھاواں() && is_separator(رستہ_2[رستہ_2.ناپ_2() - 1], style);
    إذا (رستہ_has_sep) {
      // Strip separators from beginning of component.
      size_t loc = component.لبھو_پہلا_not_دا(separators(style));
      StringRef c = component.substr(loc);

      // Append it.
      رستہ_2.append(c.begin(), c.سرا());
      استمر;
    }

    منطقي component_has_sep =
        !component.خالی_تھاواں() && is_separator(component[0], style);
    إذا (!component_has_sep &&
        !(رستہ_2.خالی_تھاواں() || has_جڑ_name(component, style))) {
      // Add a separator.
      رستہ_2.ڈالو_پٹھ(preferred_separator(style));
    }

    رستہ_2.append(component.begin(), component.سرا());
  }
}

فراغ append(Small_سمتیہ_Impl<حرف> &رستہ_2, ثابت Twine &a, ثابت Twine &b,
            ثابت Twine &c, ثابت Twine &d) {
  append(رستہ_2, Style::native, a, b, c, d);
}

فراغ append(Small_سمتیہ_Impl<حرف> &رستہ_2, const_iterator begin,
            const_iterator سرا, Style style) {
  لكل (; begin != سرا; ++begin)
    رستہ_2::append(رستہ_2, style, *begin);
}

StringRef ماپا_رستہ(StringRef رستہ_2, Style style) {
  size_t سرا_pos = ماپا_رستہ_سرا(رستہ_2, style);
  إذا (سرا_pos == StringRef::npos)
    أرجع StringRef();
  أرجع رستہ_2.substr(0, سرا_pos);
}

فراغ remove_filename(Small_سمتیہ_Impl<حرف> &رستہ_2, Style style) {
  size_t سرا_pos = ماپا_رستہ_سرا(StringRef(رستہ_2.begin(), رستہ_2.ناپ_2()), style);
  إذا (سرا_pos != StringRef::npos)
    رستہ_2.truncate(سرا_pos);
}

فراغ replace_extension(Small_سمتیہ_Impl<حرف> &رستہ_2, ثابت Twine &extension,
                       Style style) {
  StringRef p(رستہ_2.begin(), رستہ_2.ناپ_2());
  SmallString<32> ext_storage;
  StringRef ext = extension.منزل_String_Ref(ext_storage);

  // Erase existing extension.
  size_t pos = p.لبھو_آخری_دا('.');
  إذا (pos != StringRef::npos && pos >= filename_pos(p, style))
    رستہ_2.truncate(pos);

  // Append '.' if needed.
  إذا (ext.ناپ_2() > 0 && ext[0] != '.')
    رستہ_2.ڈالو_پٹھ('.');

  // Append extension.
  رستہ_2.append(ext.begin(), ext.سرا());
}

ساكن منطقي starts_with(StringRef رستہ, StringRef اگیتر,
                        Style style = Style::native) {
  // Windows prefix matching : case and separator insensitive
  إذا (is_style_windows(style)) {
    إذا (رستہ.ناپ_2() < اگیتر.ناپ_2())
      أرجع خطأ;
    لكل (size_t I = 0, E = اگیتر.ناپ_2(); I != E; ++I) {
      منطقي Sep_رستہ = is_separator(رستہ[I], style);
      منطقي Sep_اگیتر = is_separator(اگیتر[I], style);
      إذا (Sep_رستہ != Sep_اگیتر)
        أرجع خطأ;
      إذا (!Sep_رستہ && منزل_ہیٹھلا(رستہ[I]) != منزل_ہیٹھلا(اگیتر[I]))
        أرجع خطأ;
    }
    أرجع سچ;
  }
  أرجع رستہ.starts_with(اگیتر);
}

منطقي replace_رستہ_اگیتر(Small_سمتیہ_Impl<حرف> &رستہ, StringRef Old_اگیتر,
                         StringRef New_اگیتر, Style style) {
  إذا (Old_اگیتر.خالی_تھاواں() && New_اگیتر.خالی_تھاواں())
    أرجع خطأ;

  StringRef Orig_رستہ(رستہ.begin(), رستہ.ناپ_2());
  إذا (!starts_with(Orig_رستہ, Old_اگیتر, style))
    أرجع خطأ;

  // If prefixes have the same size we can simply copy the new one over.
  إذا (Old_اگیتر.ناپ_2() == New_اگیتر.ناپ_2()) {
    llvm::copy(New_اگیتر, رستہ.begin());
    أرجع سچ;
  }

  StringRef Rel_رستہ = Orig_رستہ.substr(Old_اگیتر.ناپ_2());
  SmallString<256> New_رستہ;
  (Twine(New_اگیتر) + Rel_رستہ).منزل_سمتیہ(New_رستہ);
  رستہ.وٹاندرا(New_رستہ);
  أرجع سچ;
}

فراغ native(ثابت Twine &رستہ_2, Small_سمتیہ_Impl<حرف> &نتیجہ_2, Style style) {
  أكد((!رستہ_2.isSingleStringRef() ||
          رستہ_2.getSingleStringRef().انکڑے() != نتیجہ_2.انکڑے()) &&
         "path and result are not allowed to overlap!");
  // Clear result.
  نتیجہ_2.clear();
  رستہ_2.منزل_سمتیہ(نتیجہ_2);
  native(نتیجہ_2, style);
}

std::string native(ثابت Twine &رستہ_2, Style style) {
  SmallString<128> نتیجہ;
  native(رستہ_2, نتیجہ, style);
  أرجع std::string(نتیجہ);
}

فراغ native(Small_سمتیہ_Impl<حرف> &رستہ, Style style) {
  إذا (رستہ.خالی_تھاواں())
    أرجع;
  إذا (is_style_windows(style)) {
    لكل (حرف &Ch : رستہ)
      إذا (is_separator(Ch, style))
        Ch = preferred_separator(style);
    إذا (رستہ[0] == '~' && (رستہ.ناپ_2() == 1 || is_separator(رستہ[1], style))) {
      SmallString<128> رستہ_Home;
      home_directory(رستہ_Home);
      رستہ_Home.append(رستہ.begin() + 1, رستہ.سرا());
      رستہ = std::move(رستہ_Home);
    }
  } وإلا {
    llvm::replace(رستہ, '\\', '/');
  }
}

std::string convert_منزل_slash(StringRef رستہ_2, Style style) {
  إذا (is_style_posix(style))
    أرجع std::string(رستہ_2);

  std::string s = رستہ_2.str();
  llvm::replace(s, '\\', '/');
  أرجع s;
}

StringRef filename(StringRef رستہ_2, Style style) { أرجع *rbegin(رستہ_2, style); }

StringRef stem(StringRef رستہ_2, Style style) {
  StringRef fname = filename(رستہ_2, style);
  size_t pos = fname.لبھو_آخری_دا('.');
  إذا (pos == StringRef::npos)
    أرجع fname;
  إذا ((fname.ناپ_2() == 1 && fname == ".") ||
      (fname.ناپ_2() == 2 && fname == ".."))
    أرجع fname;
  أرجع fname.substr(0, pos);
}

StringRef extension(StringRef رستہ_2, Style style) {
  StringRef fname = filename(رستہ_2, style);
  size_t pos = fname.لبھو_آخری_دا('.');
  إذا (pos == StringRef::npos)
    أرجع StringRef();
  إذا ((fname.ناپ_2() == 1 && fname == ".") ||
      (fname.ناپ_2() == 2 && fname == ".."))
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

منطقي has_جڑ_name(ثابت Twine &رستہ_2, Style style) {
  SmallString<128> رستہ_storage;
  StringRef p = رستہ_2.منزل_String_Ref(رستہ_storage);

  أرجع !جڑ_name(p, style).خالی_تھاواں();
}

منطقي has_جڑ_directory(ثابت Twine &رستہ_2, Style style) {
  SmallString<128> رستہ_storage;
  StringRef p = رستہ_2.منزل_String_Ref(رستہ_storage);

  أرجع !جڑ_directory(p, style).خالی_تھاواں();
}

منطقي has_جڑ_رستہ(ثابت Twine &رستہ_2, Style style) {
  SmallString<128> رستہ_storage;
  StringRef p = رستہ_2.منزل_String_Ref(رستہ_storage);

  أرجع !جڑ_رستہ_2(p, style).خالی_تھاواں();
}

منطقي has_relative_رستہ(ثابت Twine &رستہ_2, Style style) {
  SmallString<128> رستہ_storage;
  StringRef p = رستہ_2.منزل_String_Ref(رستہ_storage);

  أرجع !relative_رستہ(p, style).خالی_تھاواں();
}

منطقي has_filename(ثابت Twine &رستہ_2, Style style) {
  SmallString<128> رستہ_storage;
  StringRef p = رستہ_2.منزل_String_Ref(رستہ_storage);

  أرجع !filename(p, style).خالی_تھاواں();
}

منطقي has_ماپا_رستہ(ثابت Twine &رستہ_2, Style style) {
  SmallString<128> رستہ_storage;
  StringRef p = رستہ_2.منزل_String_Ref(رستہ_storage);

  أرجع !ماپا_رستہ(p, style).خالی_تھاواں();
}

منطقي has_stem(ثابت Twine &رستہ_2, Style style) {
  SmallString<128> رستہ_storage;
  StringRef p = رستہ_2.منزل_String_Ref(رستہ_storage);

  أرجع !stem(p, style).خالی_تھاواں();
}

منطقي has_extension(ثابت Twine &رستہ_2, Style style) {
  SmallString<128> رستہ_storage;
  StringRef p = رستہ_2.منزل_String_Ref(رستہ_storage);

  أرجع !extension(p, style).خالی_تھاواں();
}

منطقي is_absolute(ثابت Twine &رستہ_2, Style style) {
  SmallString<128> رستہ_storage;
  StringRef p = رستہ_2.منزل_String_Ref(رستہ_storage);

  منطقي جڑ_Dir = has_جڑ_directory(p, style);
  منطقي جڑ_Name = is_style_posix(style) || has_جڑ_name(p, style);

  أرجع جڑ_Dir && جڑ_Name;
}

منطقي is_absolute_gnu(ثابت Twine &رستہ_2, Style style) {
  SmallString<128> رستہ_storage;
  StringRef p = رستہ_2.منزل_String_Ref(رستہ_storage);

  // Handle '/' which is absolute for both Windows and POSIX systems.
  // Handle '\\' on Windows.
  إذا (!p.خالی_تھاواں() && is_separator(p.front(), style))
    أرجع سچ;

  إذا (is_style_windows(style)) {
    // Handle drive letter pattern (a character followed by ':') on Windows.
    إذا (p.ناپ_2() >= 2 && (p[0] && p[1] == ':'))
      أرجع سچ;
  }

  أرجع خطأ;
}

منطقي is_relative(ثابت Twine &رستہ_2, Style style) {
  أرجع !is_absolute(رستہ_2, style);
}

فراغ make_absolute(ثابت Twine &current_directory,
                   Small_سمتیہ_Impl<حرف> &رستہ_2) {
  StringRef p(رستہ_2.انکڑے(), رستہ_2.ناپ_2());

  منطقي جڑ_Directory = has_جڑ_directory(p);
  منطقي جڑ_Name = has_جڑ_name(p);

  // Already absolute.
  إذا ((جڑ_Name || is_style_posix(Style::native)) && جڑ_Directory)
    أرجع;

  // All the following conditions will need the current directory.
  SmallString<128> current_dir;
  current_directory.منزل_سمتیہ(current_dir);

  // Relative path. Prepend the current directory.
  إذا (!جڑ_Name && !جڑ_Directory) {
    // Append path to the current directory.
    append(current_dir, p);
    // Set path to the result.
    رستہ_2.وٹاندرا(current_dir);
    أرجع;
  }

  إذا (!جڑ_Name && جڑ_Directory) {
    StringRef cdrn = جڑ_name(current_dir);
    SmallString<128> cur_Dir_جڑ_Name(cdrn.begin(), cdrn.سرا());
    append(cur_Dir_جڑ_Name, p);
    // Set path to the result.
    رستہ_2.وٹاندرا(cur_Dir_جڑ_Name);
    أرجع;
  }

  إذا (جڑ_Name && !جڑ_Directory) {
    StringRef p_جڑ_Name = جڑ_name(p);
    StringRef b_جڑ_Directory = جڑ_directory(current_dir);
    StringRef b_Relative_رستہ = relative_رستہ(current_dir);
    StringRef p_Relative_رستہ = relative_رستہ(p);

    SmallString<128> res;
    append(res, p_جڑ_Name, b_جڑ_Directory, b_Relative_رستہ, p_Relative_رستہ);
    رستہ_2.وٹاندرا(res);
    أرجع;
  }

  llvm_unreachable("All rootName and rootDirectory combinations should have "
                   "occurred above!");
}

StringRef remove_leading_dotslash(StringRef رستہ, Style style) {
  // Remove leading "./" (or ".//" or "././" etc.)
  طالما (رستہ.ناپ_2() > 2 && رستہ[0] == '.' && is_separator(رستہ[1], style)) {
    رستہ = رستہ.substr(2);
    طالما (رستہ.ناپ_2() > 0 && is_separator(رستہ[0], style))
      رستہ = رستہ.substr(1);
  }
  أرجع رستہ;
}

منطقي remove_dots(Small_سمتیہ_Impl<حرف> &the_رستہ, منطقي remove_dot_dot,
                 Style style) {
  style = real_style(style);
  StringRef بچیا_2(the_رستہ.انکڑے(), the_رستہ.ناپ_2());
  منطقي needs_change = خطأ;
  Small_سمتیہ<StringRef, 16> components;

  // Consume the root path, if present.
  StringRef جڑ_3 = رستہ_2::جڑ_رستہ_2(بچیا_2, style);
  منطقي absolute = !جڑ_3.خالی_تھاواں();
  إذا (absolute)
    بچیا_2 = بچیا_2.drop_front(جڑ_3.ناپ_2());

  // Loop over path components manually. This makes it easier to detect
  // non-preferred slashes and double separators that must be canonicalized.
  طالما (!بچیا_2.خالی_تھاواں()) {
    size_t اگلا_slash = بچیا_2.لبھو_پہلا_دا(separators(style));
    إذا (اگلا_slash == StringRef::npos)
      اگلا_slash = بچیا_2.ناپ_2();
    StringRef component = بچیا_2.take_front(اگلا_slash);
    بچیا_2 = بچیا_2.drop_front(اگلا_slash);

    // Eat the slash, and check if it is the preferred separator.
    إذا (!بچیا_2.خالی_تھاواں()) {
      needs_change |= بچیا_2.front() != preferred_separator(style);
      بچیا_2 = بچیا_2.drop_front();
      // The path needs to be rewritten if it has a trailing slash.
      // FIXME: This is emergent behavior that could be removed.
      needs_change |= بچیا_2.خالی_تھاواں();
    }

    // Check for path traversal components or double separators.
    إذا (component.خالی_تھاواں() || component == ".") {
      needs_change = سچ;
    } وإلا إذا (remove_dot_dot && component == "..") {
      needs_change = سچ;
      // Do not allow ".." to remove the root component. If this is the
      // beginning of a relative path, keep the ".." component.
      إذا (!components.خالی_تھاواں() && components.پٹھ() != "..") {
        components.نکالو_پٹھ();
      } وإلا إذا (!absolute) {
        components.ڈالو_پٹھ(component);
      }
    } وإلا {
      components.ڈالو_پٹھ(component);
    }
  }

  SmallString<256> بفر_2 = جڑ_3;
  // "root" could be "/", which may need to be translated into "\".
  make_preferred(بفر_2, style);
  needs_change |= جڑ_3 != بفر_2;

  // Avoid rewriting the path unless we have to.
  إذا (!needs_change)
    أرجع خطأ;

  إذا (!components.خالی_تھاواں()) {
    بفر_2 += components[0];
    لكل (StringRef C : ArrayRef(components).drop_front()) {
      بفر_2 += preferred_separator(style);
      بفر_2 += C;
    }
  }
  the_رستہ.وٹاندرا(بفر_2);
  أرجع سچ;
}

} // end namespace path

نطاق fs {

std::error_کوڈ getUniqueID(ثابت Twine رستہ, UniqueID &نتیجہ) {
  sandbox::violationIfEnabled();

  file_حالت حالت;
  std::error_کوڈ EC = حالت_2(رستہ, حالت);
  إذا (EC)
    أرجع EC;
  نتیجہ = حالت.getUniqueID();
  أرجع std::error_کوڈ();
}

فراغ create_Unique_رستہ(ثابت Twine &Model, Small_سمتیہ_Impl<حرف> &نتیجہ_رستہ,
                      منطقي MakeAbsolute) {
  SmallString<128> ModelStorage;
  Model.منزل_سمتیہ(ModelStorage);

  أكد(llvm::is_contained(ModelStorage, '%') &&
         "createUniquePath: Model must contain at least one '%'");

  إذا (MakeAbsolute) {
    // Make model absolute by prepending a temp directory if it's not already.
    إذا (!sys::رستہ_2::is_absolute(Twine(ModelStorage))) {
      SmallString<128> TDir;
      sys::رستہ_2::system_عارضی_directory(سچ, TDir);
      sys::رستہ_2::append(TDir, Twine(ModelStorage));
      ModelStorage.وٹاندرا(TDir);
    }
  }

  نتیجہ_رستہ = ModelStorage;
  نتیجہ_رستہ.ڈالو_پٹھ(0);
  نتیجہ_رستہ.نکالو_پٹھ();

  // Replace '%' with random chars.
  لكل (بنانشانی i = 0, e = ModelStorage.ناپ_2(); i != e; ++i) {
    إذا (ModelStorage[i] == '%')
      نتیجہ_رستہ[i] = "0123456789abcdef"[sys::Process::Get_Random_نمبر() & 15];
  }
}

std::error_کوڈ createUniqueFile(ثابت Twine &Model, صحيح &نتیجہ_Fd,
                                 Small_سمتیہ_Impl<حرف> &نتیجہ_رستہ,
                                 OpenFlags Flags, بنانشانی منوال) {
  أرجع createUniqueEntity(Model, نتیجہ_Fd, نتیجہ_رستہ, خطأ, FS_File, Flags,
                            منوال);
}

std::error_کوڈ createUniqueFile(ثابت Twine &Model,
                                 Small_سمتیہ_Impl<حرف> &نتیجہ_رستہ,
                                 بنانشانی منوال) {
  صحيح FD;
  خودکار EC = createUniqueFile(Model, FD, نتیجہ_رستہ, دا_None, منوال);
  إذا (EC)
    أرجع EC;
  // FD is only needed to avoid race conditions. Close it right away.
  close(FD);
  أرجع EC;
}

ساكن std::error_کوڈ
createTemporaryFile(ثابت Twine &Model, صحيح &نتیجہ_FD,
                    llvm::Small_سمتیہ_Impl<حرف> &نتیجہ_رستہ, FSEntity Type,
                    sys::fs::OpenFlags Flags = sys::fs::دا_None) {
  // Any *temporary* file is assumed to be a compiler-internal output, not
  // a formal one.
  خودکار BypassSandbox = sys::sandbox::scopedDisable();

  SmallString<128> Storage;
  StringRef P = Model.منزل_Null_Terminated_String_Ref(Storage);
  أكد(P.لبھو_پہلا_دا(separators(Style::native)) == StringRef::npos &&
         "Model must be a simple filename.");
  // Use P.begin() so that createUniqueEntity doesn't need to recreate Storage.
  أرجع createUniqueEntity(P.begin(), نتیجہ_FD, نتیجہ_رستہ, سچ, Type, Flags,
                            all_read | all_write);
}

ساكن std::error_کوڈ
createTemporaryFile(ثابت Twine &اگیتر, StringRef Suffix, صحيح &نتیجہ_FD,
                    llvm::Small_سمتیہ_Impl<حرف> &نتیجہ_رستہ, FSEntity Type,
                    sys::fs::OpenFlags Flags = sys::fs::دا_None) {
  ثابت حرف *درمیان = Suffix.خالی_تھاواں() ? "-%%%%%%" : "-%%%%%%.";
  أرجع createTemporaryFile(اگیتر + درمیان + Suffix, نتیجہ_FD, نتیجہ_رستہ,
                             Type, Flags);
}

std::error_کوڈ createTemporaryFile(ثابت Twine &اگیتر, StringRef Suffix,
                                    صحيح &نتیجہ_FD,
                                    Small_سمتیہ_Impl<حرف> &نتیجہ_رستہ,
                                    sys::fs::OpenFlags Flags) {
  أرجع createTemporaryFile(اگیتر, Suffix, نتیجہ_FD, نتیجہ_رستہ, FS_File,
                             Flags);
}

std::error_کوڈ createTemporaryFile(ثابت Twine &اگیتر, StringRef Suffix,
                                    Small_سمتیہ_Impl<حرف> &نتیجہ_رستہ,
                                    sys::fs::OpenFlags Flags) {
  صحيح FD;
  خودکار EC = createTemporaryFile(اگیتر, Suffix, FD, نتیجہ_رستہ, Flags);
  إذا (EC)
    أرجع EC;
  // FD is only needed to avoid race conditions. Close it right away.
  close(FD);
  أرجع EC;
}

// This is a mkdtemp with a different pattern. We use createUniqueEntity mostly
// for consistency. We should try using mkdtemp.
std::error_کوڈ createUniqueDirectory(ثابت Twine &اگیتر,
                                      Small_سمتیہ_Impl<حرف> &نتیجہ_رستہ) {
  صحيح Dummy;
  أرجع createUniqueEntity(اگیتر + "-%%%%%%", Dummy, نتیجہ_رستہ, سچ,
                            FS_Dir);
}

std::error_کوڈ
getPotentiallyUniqueFileName(ثابت Twine &Model,
                             Small_سمتیہ_Impl<حرف> &نتیجہ_رستہ) {
  صحيح Dummy;
  أرجع createUniqueEntity(Model, Dummy, نتیجہ_رستہ, خطأ, FS_Name);
}

std::error_کوڈ
get_Potentially_Unique_عارضی_File_Name(ثابت Twine &اگیتر, StringRef Suffix,
                                 Small_سمتیہ_Impl<حرف> &نتیجہ_رستہ) {
  صحيح Dummy;
  أرجع createTemporaryFile(اگیتر, Suffix, Dummy, نتیجہ_رستہ, FS_Name);
}

std::error_کوڈ make_absolute(Small_سمتیہ_Impl<حرف> &رستہ_2) {
  sandbox::violationIfEnabled();

  إذا (رستہ_2::is_absolute(رستہ_2))
    أرجع {};

  SmallString<128> current_dir;
  إذا (std::error_کوڈ ec = current_رستہ(current_dir))
    أرجع ec;

  رستہ_2::make_absolute(current_dir, رستہ_2);
  أرجع {};
}

std::error_کوڈ create_directories(ثابت Twine &رستہ, منطقي IgnoreExisting,
                                   perms Perms) {
  SmallString<128> رستہ_Storage;
  StringRef P = رستہ.منزل_String_Ref(رستہ_Storage);

  // Be optimistic and try to create the directory
  std::error_کوڈ EC = create_directory(P, IgnoreExisting, Perms);
  // If we succeeded, or had any error other than the parent not existing, just
  // return it.
  إذا (EC != errc::no_such_file_or_directory)
    أرجع EC;

  // We failed because of a no_such_file_or_directory, try to create the
  // parent.
  StringRef ماپا_2 = رستہ_2::ماپا_رستہ(P);
  إذا (ماپا_2.خالی_تھاواں())
    أرجع EC;

  إذا ((EC = create_directories(ماپا_2, IgnoreExisting, Perms)))
      أرجع EC;

  أرجع create_directory(P, IgnoreExisting, Perms);
}

ساكن std::error_کوڈ copy_file_اندرونی(صحيح ReadFD, صحيح WriteFD) {
  ثابت size_t Buf_ناپ = 4096;
  حرف *Buf = جديد حرف[Buf_ناپ];
  صحيح بائٹاں_Read = 0, بائٹاں_Written = 0;
  لكل (;;) {
    بائٹاں_Read = اقرأ(ReadFD, Buf, Buf_ناپ);
    إذا (بائٹاں_Read <= 0)
      اكسر;
    طالما (بائٹاں_Read) {
      بائٹاں_Written = اكتب(WriteFD, Buf, بائٹاں_Read);
      إذا (بائٹاں_Written < 0)
        اكسر;
      بائٹاں_Read -= بائٹاں_Written;
    }
    إذا (بائٹاں_Written < 0)
      اكسر;
  }
  مٹاؤ[] Buf;

  إذا (بائٹاں_Read < 0 || بائٹاں_Written < 0)
    أرجع errno_As_Error_کوڈ();
  أرجع std::error_کوڈ();
}

#جے_بنا_تعریف __APPLE__
std::error_کوڈ copy_file(ثابت Twine &From, ثابت Twine &منزل) {
  صحيح ReadFD, WriteFD;
  إذا (std::error_کوڈ EC = openFileForRead(From, ReadFD, دا_None))
    أرجع EC;
  إذا (std::error_کوڈ EC =
          openFileForWrite(منزل, WriteFD, CD_CreateAlways, دا_None)) {
    close(ReadFD);
    أرجع EC;
  }

  std::error_کوڈ EC = copy_file_اندرونی(ReadFD, WriteFD);

  close(ReadFD);
  close(WriteFD);

  أرجع EC;
}
#جے_انت

std::error_کوڈ copy_file(ثابت Twine &From, صحيح منزل_FD) {
  صحيح ReadFD;
  إذا (std::error_کوڈ EC = openFileForRead(From, ReadFD, دا_None))
    أرجع EC;

  std::error_کوڈ EC = copy_file_اندرونی(ReadFD, منزل_FD);

  close(ReadFD);

  أرجع EC;
}

ErrorOr<MD5::MD5_نتیجہ> md5_contents(صحيح FD) {
  sandbox::violationIfEnabled();

  MD5 Hash;

  constexpr size_t Buf_ناپ = 4096;
  std::سمتیہ<uint8_t> Buf(Buf_ناپ);
  صحيح بائٹاں_Read = 0;
  لكل (;;) {
    بائٹاں_Read = اقرأ(FD, Buf.انکڑے(), Buf_ناپ);
    إذا (بائٹاں_Read <= 0)
      اكسر;
    Hash.update(ArrayRef(Buf.انکڑے(), بائٹاں_Read));
  }

  إذا (بائٹاں_Read < 0)
    أرجع errno_As_Error_کوڈ();
  MD5::MD5_نتیجہ نتیجہ;
  Hash.final(نتیجہ);
  أرجع نتیجہ;
}

ErrorOr<MD5::MD5_نتیجہ> md5_contents(ثابت Twine &رستہ) {
  sandbox::violationIfEnabled();

  صحيح FD;
  إذا (خودکار EC = openFileForRead(رستہ, FD, دا_None))
    أرجع EC;

  خودکار نتیجہ = md5_contents(FD);
  close(FD);
  أرجع نتیجہ;
}

منطقي exists(ثابت basic_file_حالت &حالت_2) {
  أرجع حالت_معلوم(حالت_2) && حالت_2.type() != file_type::file_not_لبھیا;
}

منطقي حالت_معلوم(ثابت basic_file_حالت &s) {
  أرجع s.type() != file_type::حالت_error;
}

file_type get_file_type(ثابت Twine &رستہ, منطقي Follow) {
  file_حالت st;
  إذا (حالت_2(رستہ, st, Follow))
    أرجع file_type::حالت_error;
  أرجع st.type();
}

منطقي is_directory(ثابت basic_file_حالت &حالت_2) {
  أرجع حالت_2.type() == file_type::directory_file;
}

std::error_کوڈ is_directory(ثابت Twine &رستہ_2, منطقي &نتیجہ_2) {
  sandbox::violationIfEnabled();

  file_حالت st;
  إذا (std::error_کوڈ ec = حالت_2(رستہ_2, st))
    أرجع ec;
  نتیجہ_2 = is_directory(st);
  أرجع std::error_کوڈ();
}

منطقي is_regular_file(ثابت basic_file_حالت &حالت_2) {
  أرجع حالت_2.type() == file_type::regular_file;
}

std::error_کوڈ is_regular_file(ثابت Twine &رستہ_2, منطقي &نتیجہ_2) {
  sandbox::violationIfEnabled();

  file_حالت st;
  إذا (std::error_کوڈ ec = حالت_2(رستہ_2, st))
    أرجع ec;
  نتیجہ_2 = is_regular_file(st);
  أرجع std::error_کوڈ();
}

منطقي is_symlink_file(ثابت basic_file_حالت &حالت_2) {
  أرجع حالت_2.type() == file_type::symlink_file;
}

std::error_کوڈ is_symlink_file(ثابت Twine &رستہ_2, منطقي &نتیجہ_2) {
  sandbox::violationIfEnabled();

  file_حالت st;
  إذا (std::error_کوڈ ec = حالت_2(رستہ_2, st, خطأ))
    أرجع ec;
  نتیجہ_2 = is_symlink_file(st);
  أرجع std::error_کوڈ();
}

منطقي is_other(ثابت basic_file_حالت &حالت_2) {
  أرجع exists(حالت_2) &&
         !is_regular_file(حالت_2) &&
         !is_directory(حالت_2);
}

std::error_کوڈ is_other(ثابت Twine &رستہ, منطقي &نتیجہ) {
  sandbox::violationIfEnabled();

  file_حالت File_حالت;
  إذا (std::error_کوڈ EC = حالت_2(رستہ, File_حالت))
    أرجع EC;
  نتیجہ = is_other(File_حالت);
  أرجع std::error_کوڈ();
}

فراغ directory_اندراج::replace_filename(ثابت Twine &Filename, file_type Type,
                                       basic_file_حالت حالت) {
  SmallString<128> رستہ_Str = رستہ_2::ماپا_رستہ(رستہ);
  رستہ_2::append(رستہ_Str, Filename);
  هذا->رستہ = std::string(رستہ_Str);
  هذا->Type = Type;
  هذا->حالت = حالت;
}

ErrorOr<perms> getPermissions(ثابت Twine &رستہ) {
  sandbox::violationIfEnabled();

  file_حالت حالت;
  إذا (std::error_کوڈ EC = حالت_2(رستہ, حالت))
    أرجع EC;

  أرجع حالت.permissions();
}

std::error_کوڈ سیٹ_آخری_Access_And_Modification_ویلا(ثابت Twine &رستہ,
                                                 ویلا_نقطہ<> Access_ویلا,
                                                 ویلا_نقطہ<> Modification_ویلا) {
  صحيح FD;
  إذا (std::error_کوڈ EC = openFile(رستہ, FD, CD_OpenExisting, FA_Read,
                                    دا_Update_گن | دا_Open_Directory))
    أرجع EC;
  std::error_کوڈ EC =
      سیٹ_آخری_Access_And_Modification_ویلا(FD, Access_ویلا, Modification_ویلا);
  Process::SafelyCloseFileDescriptor(FD);
  أرجع EC;
}

size_t mapped_file_region::ناپ_2() ثابت {
  أكد(Mapping && "Mapping failed but used anyway!");
  أرجع ناپ;
}

حرف *mapped_file_region::انکڑے() ثابت {
  أكد(Mapping && "Mapping failed but used anyway!");
  أرجع reinterpret_cast<حرف *>(Mapping);
}

ثابت حرف *mapped_file_region::const_انکڑے() ثابت {
  أكد(Mapping && "Mapping failed but used anyway!");
  أرجع reinterpret_cast<ثابت حرف *>(Mapping);
}

Error read_Native_File_منزل_EOF(file_t FileHandle, Small_سمتیہ_Impl<حرف> &بفر,
                          ssize_t Chunk_ناپ) {
  sandbox::violationIfEnabled();

  // Install a handler to truncate the buffer to the correct size on exit.
  size_t ناپ = بفر.ناپ_2();
  llvm::scope_exit TruncateOnExit([&]() { بفر.truncate(ناپ); });

  // Read into Buffer until we hit EOF.
  لكل (;;) {
    بفر.resize_for_overwrite(ناپ + Chunk_ناپ);
    Expected<size_t> Read_بائٹاں = readNativeFile(
        FileHandle, MutableArrayRef(بفر.begin() + ناپ, Chunk_ناپ));
    إذا (!Read_بائٹاں)
      أرجع Read_بائٹاں.takeError();
    إذا (*Read_بائٹاں == 0)
      أرجع Error::success();
    ناپ += *Read_بائٹاں;
  }
}

} // end namespace fs
} // end namespace sys
} // end namespace llvm

// Include the truly platform-specific parts.
#إذا defined(LLVM_ON_UNIX)
#ضمن "Unix/Path.inc"
#جے_انت
#إذا defined(_WIN32)
#ضمن "Windows/Path.inc"
#جے_انت

نطاق llvm {
نطاق sys {
نطاق fs {

عارضی_File::عارضی_File(StringRef Name, صحيح FD)
    : TmpName(std::string(Name)), FD(FD) {}
عارضی_File::عارضی_File(عارضی_File &&Other) { *هذا = std::move(Other); }
عارضی_File &عارضی_File::عامل=(عارضی_File &&Other) {
  TmpName = std::move(Other.TmpName);
  FD = Other.FD;
  Other.پورا = سچ;
  Other.FD = -1;
#جے_تعریف_شدہ _WIN32
  RemoveOnClose = Other.RemoveOnClose;
  Other.RemoveOnClose = خطأ;
#جے_انت
  أرجع *هذا;
}

عارضی_File::~عارضی_File() { أكد(پورا); }

Error عارضی_File::discard() {
  پورا = سچ;
  إذا (FD != -1 && close(FD) == -1) {
    std::error_کوڈ EC = errno_As_Error_کوڈ();
    أرجع error_کوڈ_منزل_Error(EC);
  }
  FD = -1;

#جے_تعریف_شدہ _WIN32
  // On Windows, closing will remove the file, if we set the delete
  // disposition. If not, remove it manually.
  منطقي Remove = RemoveOnClose;
#وإلا
  // Always try to remove the file.
  منطقي Remove = سچ;
#جے_انت
  std::error_کوڈ RemoveEC;
  إذا (Remove && !TmpName.خالی_تھاواں()) {
    RemoveEC = fs::remove(TmpName);
    sys::DontRemoveFileOnSignal(TmpName);
    إذا (!RemoveEC)
      TmpName = "";
  } وإلا {
    TmpName = "";
  }
  أرجع error_کوڈ_منزل_Error(RemoveEC);
}

Error عارضی_File::keep(ثابت Twine &Name) {
  أكد(!پورا);
  پورا = سچ;
  // Always try to close and rename.
#جے_تعریف_شدہ _WIN32
  // If we can't cancel the delete don't rename.
  خودکار H = reinterpret_cast<HANDLE>(_get_osfhandle(FD));
  std::error_کوڈ RenameEC =
      RemoveOnClose ? std::error_کوڈ() : سیٹ_Delete_Disposition(H, خطأ);
  منطقي ShouldDelete = خطأ;
  إذا (!RenameEC) {
    RenameEC = rename_handle(H, Name);
    // If rename failed because it's cross-device, copy instead
    إذا (RenameEC ==
      std::error_کوڈ(ERROR_NOT_SAME_DEVICE, std::system_category())) {
      RenameEC = copy_file(TmpName, Name);
      ShouldDelete = سچ;
    }
  }

  // If we can't rename or copy, discard the temporary file.
  إذا (RenameEC)
    ShouldDelete = سچ;
  إذا (ShouldDelete) {
    إذا (!RemoveOnClose)
      سیٹ_Delete_Disposition(H, سچ);
    وإلا
      remove(TmpName);
  }
#وإلا
  std::error_کوڈ RenameEC = fs::rename(TmpName, Name);
  إذا (RenameEC) {
    // If we can't rename, try to copy to work around cross-device link issues.
    RenameEC = sys::fs::copy_file(TmpName, Name);
    // If we can't rename or copy, discard the temporary file.
    إذا (RenameEC)
      remove(TmpName);
  }
#جے_انت
  sys::DontRemoveFileOnSignal(TmpName);

  إذا (!RenameEC)
    TmpName = "";

  إذا (close(FD) == -1)
    أرجع error_کوڈ_منزل_Error(errno_As_Error_کوڈ());
  FD = -1;

  أرجع error_کوڈ_منزل_Error(RenameEC);
}

Error عارضی_File::keep() {
  أكد(!پورا);
  پورا = سچ;

#جے_تعریف_شدہ _WIN32
  خودکار H = reinterpret_cast<HANDLE>(_get_osfhandle(FD));
  إذا (std::error_کوڈ EC = سیٹ_Delete_Disposition(H, خطأ))
    أرجع error_کوڈ_منزل_Error(EC);
#جے_انت
  sys::DontRemoveFileOnSignal(TmpName);

  TmpName = "";

  إذا (close(FD) == -1)
    أرجع error_کوڈ_منزل_Error(errno_As_Error_کوڈ());
  FD = -1;

  أرجع Error::success();
}

Expected<عارضی_File> عارضی_File::create(ثابت Twine &Model, بنانشانی منوال,
                                    OpenFlags ExtraFlags) {
  صحيح FD;
  SmallString<128> نتیجہ_رستہ;
  إذا (std::error_کوڈ EC =
          createUniqueFile(Model, FD, نتیجہ_رستہ, دا_Delete | ExtraFlags, منوال))
    أرجع error_کوڈ_منزل_Error(EC);

  عارضی_File Ret(نتیجہ_رستہ, FD);
#جے_تعریف_شدہ _WIN32
  خودکار H = reinterpret_cast<HANDLE>(_get_osfhandle(FD));
  منطقي سیٹ_Signal_Handler = خطأ;
  إذا (std::error_کوڈ EC = سیٹ_Delete_Disposition(H, سچ)) {
    Ret.RemoveOnClose = سچ;
    سیٹ_Signal_Handler = سچ;
  }
#وإلا
  منطقي سیٹ_Signal_Handler = سچ;
#جے_انت
  إذا (سیٹ_Signal_Handler && sys::RemoveFileOnSignal(نتیجہ_رستہ)) {
    // Make sure we delete the file when RemoveFileOnSignal fails.
    consumeError(Ret.discard());
    std::error_کوڈ EC(errc::operation_not_permitted);
    أرجع error_کوڈ_منزل_Error(EC);
  }
  أرجع std::move(Ret);
}
} // namespace fs

} // namespace sys
} // namespace llvm
