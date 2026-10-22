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

#شامل "llvm/Support/Path.h"
#شامل "llvm/ADT/ArrayRef.h"
#شامل "llvm/ADT/ScopeExit.h"
#شامل "llvm/ADT/StringExtras.h"
#شامل "llvm/Config/config.h"
#شامل "llvm/Config/llvm-config.h"
#شامل "llvm/Support/Errc.h"
#شامل "llvm/Support/ErrorHandling.h"
#شامل "llvm/Support/FileSystem.h"
#شامل "llvm/Support/IOSandbox.h"
#شامل "llvm/Support/Process.h"
#شامل "llvm/Support/Signals.h"
#شامل <cctype>

#اگر !defined(_MSC_VER) && !defined(__MINGW32__)
#شامل <unistd.h>
#ورنہ
#شامل <io.h>
#اگر_ختم

استعمال نامخلا llvm;
استعمال نامخلا llvm::support::endian;

نامخلا {
  استعمال llvm::StringRef;
  استعمال llvm::sys::راستہ_2::is_separator;
  استعمال llvm::sys::راستہ_2::Style;

  اندرونی منطقی prefer_forward_slash() {
    ساکن منطقی prefer = []() {
      اگر (std::optional<std::string> Env =
              sys::Process::GetEnv("LLVM_WINDOWS_PREFER_FORWARD_SLASH"))
        واپس *Env == "1";
      واپس static_cast<منطقی>(LLVM_WINDOWS_PREFER_FORWARD_SLASH);
    }();
    واپس prefer;
  }

  اندرونی Style real_style(Style style) {
    اگر (style != Style::native)
      واپس style;
    اگر (is_style_posix(style))
      واپس Style::posix;
    واپس prefer_forward_slash() ? Style::windows_slash
                                  : Style::windows_backslash;
  }

  اندرونی ثابت حرف *separators(Style style) {
    اگر (is_style_windows(style))
      واپس "\\/";
    واپس "/";
  }

  اندرونی حرف preferred_separator(Style style) {
    اگر (real_style(style) == Style::windows)
      واپس '\\';
    واپس '/';
  }

  StringRef ڈھونڈو_پہلا_component(StringRef راستہ_2, Style style) {
    // Look for this first component in the following order.
    // * empty (in this case we return an empty string)
    // * either C: or {//,\\}net.
    // * {/,\}
    // * {file,directory}name

    اگر (راستہ_2.خالی_خانے())
      واپس راستہ_2;

    اگر (is_style_windows(style)) {
      // C:
      اگر (راستہ_2.سائز_2() >= 2 &&
          std::حرف_ہے(static_cast<غیرعلامتی حرف>(راستہ_2[0])) && راستہ_2[1] == ':')
        واپس راستہ_2.substr(0, 2);
    }

    // //net
    اگر ((راستہ_2.سائز_2() > 2) && is_separator(راستہ_2[0], style) &&
        راستہ_2[0] == راستہ_2[1] && !is_separator(راستہ_2[2], style)) {
      // Find the next directory separator.
      size_t انتہا = راستہ_2.ڈھونڈو_پہلا_کا(separators(style), 2);
      واپس راستہ_2.substr(0, انتہا);
    }

    // {/,\}
    اگر (is_separator(راستہ_2[0], style))
      واپس راستہ_2.substr(0, 1);

    // * {file,directory}name
    size_t انتہا = راستہ_2.ڈھونڈو_پہلا_کا(separators(style));
    واپس راستہ_2.substr(0, انتہا);
  }

  // Returns the first character of the filename in str. For paths ending in
  // '/', it returns the position of the '/'.
  size_t filename_pos(StringRef str, Style style) {
    اگر (str.سائز_2() > 0 && is_separator(str[str.سائز_2() - 1], style))
      واپس str.سائز_2() - 1;

    size_t pos = str.ڈھونڈو_آخری_کا(separators(style), str.سائز_2() - 1);

    اگر (is_style_windows(style)) {
      اگر (pos == StringRef::npos)
        pos = str.ڈھونڈو_آخری_کا(':', str.سائز_2() - 1);
    }

    اگر (pos == StringRef::npos || (pos == 1 && is_separator(str[0], style)))
      واپس 0;

    واپس pos + 1;
  }

  // Returns the position of the root directory in str. If there is no root
  // directory in str, it returns StringRef::npos.
  size_t جڑ_dir_آغاز(StringRef str, Style style) {
    // case "c:/"
    اگر (is_style_windows(style)) {
      اگر (str.سائز_2() > 2 && str[1] == ':' && is_separator(str[2], style))
        واپس 2;
    }

    // case "//net"
    اگر (str.سائز_2() > 3 && is_separator(str[0], style) && str[0] == str[1] &&
        !is_separator(str[2], style)) {
      واپس str.ڈھونڈو_پہلا_کا(separators(style), 2);
    }

    // case "/"
    اگر (str.سائز_2() > 0 && is_separator(str[0], style))
      واپس 0;

    واپس StringRef::npos;
  }

  // Returns the position past the end of the "parent path" of path. The parent
  // path will not end in '/', unless the parent is the root directory. If the
  // path has no parent, 0 is returned.
  size_t والد_راستہ_انتہا(StringRef راستہ_2, Style style) {
    size_t انتہا_pos = filename_pos(راستہ_2, style);

    منطقی filename_was_sep =
        راستہ_2.سائز_2() > 0 && is_separator(راستہ_2[انتہا_pos], style);

    // Skip separators until we reach root dir (or the start of the string).
    size_t جڑ_dir_pos = جڑ_dir_آغاز(راستہ_2, style);
    جب (انتہا_pos > 0 &&
           (جڑ_dir_pos == StringRef::npos || انتہا_pos > جڑ_dir_pos) &&
           is_separator(راستہ_2[انتہا_pos - 1], style))
      --انتہا_pos;

    اگر (انتہا_pos == جڑ_dir_pos && !filename_was_sep) {
      // We've reached the root dir and the input path was *not* ending in a
      // sequence of slashes. Include the root dir in the parent path.
      واپس جڑ_dir_pos + 1;
    }

    // Otherwise, just include before the last slash.
    واپس انتہا_pos;
  }
} // end unnamed namespace

شمار FSEntity {
  FS_Dir,
  FS_File,
  FS_Name
};

ساکن std::error_رمز
createUniqueEntity(ثابت Twine &Model, صحیح &نتیجہ_FD,
                   Small_سمتیہ_Impl<حرف> &نتیجہ_راستہ, منطقی MakeAbsolute,
                   FSEntity Type, sys::fs::OpenFlags Flags = sys::fs::کا_None,
                   غیرعلامتی منوال = 0) {

  // Limit the number of attempts we make, so that we don't infinite loop. E.g.
  // "permission denied" could be for a specific file (so we retry with a
  // different name) or for the whole directory (retry would always fail).
  // Checking which is racy, so we try a number of times, then give up.
  std::error_رمز EC;
  برائے (صحیح Retries = 128; Retries > 0; --Retries) {
    sys::fs::create_Unique_راستہ(Model, نتیجہ_راستہ, MakeAbsolute);
    // Try to open + create the file.
    انتخاب (Type) {
    حالت FS_File: {
      EC = sys::fs::openFileForReadWrite(Twine(نتیجہ_راستہ.begin()), نتیجہ_FD,
                                         sys::fs::CD_CreateNew, Flags, منوال);
      اگر (EC) {
        // errc::permission_denied happens on Windows when we try to open a file
        // that has been marked for deletion.
        اگر (EC == errc::file_exists || EC == errc::permission_denied)
          جاری;
        واپس EC;
      }

      واپس std::error_رمز();
    }

    حالت FS_Name: {
      EC = sys::fs::access(نتیجہ_راستہ.begin(), sys::fs::Access_منوال::Exist);
      اگر (EC == errc::no_such_file_or_directory)
        واپس std::error_رمز();
      اگر (EC)
        واپس EC;
      جاری;
    }

    حالت FS_Dir: {
      EC = sys::fs::create_directory(نتیجہ_راستہ.begin(), جھوٹ);
      اگر (EC) {
        اگر (EC == errc::file_exists)
          جاری;
        واپس EC;
      }
      واپس std::error_رمز();
    }
    }
    llvm_unreachable("Invalid Type");
  }
  واپس EC;
}

نامخلا llvm {
نامخلا sys  {
نامخلا راستہ_2 {

const_iterator begin(StringRef راستہ_2, Style style) {
  const_iterator i;
  i.راستہ      = راستہ_2;
  i.Component = ڈھونڈو_پہلا_component(راستہ_2, style);
  i.Position  = 0;
  i.S = style;
  واپس i;
}

const_iterator انتہا(StringRef راستہ_2) {
  const_iterator i;
  i.راستہ      = راستہ_2;
  i.Position  = راستہ_2.سائز_2();
  واپس i;
}

const_iterator &const_iterator::عامل++() {
  أكد(Position < راستہ.سائز_2() && "Tried to increment past end!");

  // Increment Position to past the current component
  Position += Component.سائز_2();

  // Check for end.
  اگر (Position == راستہ.سائز_2()) {
    Component = StringRef();
    واپس *یہ;
  }

  // Both POSIX and Windows treat paths that begin with exactly two separators
  // specially.
  منطقی was_net = Component.سائز_2() > 2 && is_separator(Component[0], S) &&
                 Component[1] == Component[0] && !is_separator(Component[2], S);

  // Handle separators.
  اگر (is_separator(راستہ[Position], S)) {
    // Root dir.
    اگر (was_net ||
        // c:/
        (is_style_windows(S) && Component.سرے_with(":"))) {
      Component = راستہ.substr(Position, 1);
      واپس *یہ;
    }

    // Skip extra separators.
    جب (Position != راستہ.سائز_2() && is_separator(راستہ[Position], S)) {
      ++Position;
    }

    // Treat trailing '/' as a '.', unless it is the root dir.
    اگر (Position == راستہ.سائز_2() && Component != "/") {
      --Position;
      Component = ".";
      واپس *یہ;
    }
  }

  // Find next component.
  size_t انتہا_pos = راستہ.ڈھونڈو_پہلا_کا(separators(S), Position);
  Component = راستہ.قاش(Position, انتہا_pos);

  واپس *یہ;
}

منطقی const_iterator::عامل==(ثابت const_iterator &RHS) ثابت {
  واپس راستہ.begin() == RHS.راستہ.begin() && Position == RHS.Position;
}

ptrdiff_t const_iterator::عامل-(ثابت const_iterator &RHS) ثابت {
  واپس Position - RHS.Position;
}

الٹ_iterator rbegin(StringRef راستہ, Style style) {
  الٹ_iterator I;
  I.راستہ = راستہ;
  I.Position = راستہ.سائز_2();
  I.S = style;
  ++I;
  واپس I;
}

الٹ_iterator rend(StringRef راستہ) {
  الٹ_iterator I;
  I.راستہ = راستہ;
  I.Component = راستہ.substr(0, 0);
  I.Position = 0;
  واپس I;
}

الٹ_iterator &الٹ_iterator::عامل++() {
  size_t جڑ_dir_pos = جڑ_dir_آغاز(راستہ, S);

  // Skip separators unless it's the root directory.
  size_t انتہا_pos = Position;
  جب (انتہا_pos > 0 && (انتہا_pos - 1) != جڑ_dir_pos &&
         is_separator(راستہ[انتہا_pos - 1], S))
    --انتہا_pos;

  // Treat trailing '/' as a '.', unless it is the root dir.
  اگر (Position == راستہ.سائز_2() && !راستہ.خالی_خانے() &&
      is_separator(راستہ.عقب(), S) &&
      (جڑ_dir_pos == StringRef::npos || انتہا_pos - 1 > جڑ_dir_pos)) {
    --Position;
    Component = ".";
    واپس *یہ;
  }

  // Find next separator.
  size_t آغاز_pos = filename_pos(راستہ.substr(0, انتہا_pos), S);
  Component = راستہ.قاش(آغاز_pos, انتہا_pos);
  Position = آغاز_pos;
  واپس *یہ;
}

منطقی الٹ_iterator::عامل==(ثابت الٹ_iterator &RHS) ثابت {
  واپس راستہ.begin() == RHS.راستہ.begin() && Component == RHS.Component &&
         Position == RHS.Position;
}

ptrdiff_t الٹ_iterator::عامل-(ثابت الٹ_iterator &RHS) ثابت {
  واپس Position - RHS.Position;
}

StringRef جڑ_راستہ_2(StringRef راستہ_2, Style style) {
  const_iterator b = begin(راستہ_2, style), pos = b, e = انتہا(راستہ_2);
  اگر (b != e) {
    منطقی has_net =
        b->سائز_2() > 2 && is_separator((*b)[0], style) && (*b)[1] == (*b)[0];
    منطقی has_drive = is_style_windows(style) && b->سرے_with(":");

    اگر (has_net || has_drive) {
      اگر ((++pos != e) && is_separator((*pos)[0], style)) {
        // {C:/,//net/}, so get the first two components.
        واپس راستہ_2.substr(0, b->سائز_2() + pos->سائز_2());
      }
      // just {C:,//net}, return the first component.
      واپس *b;
    }

    // POSIX style root directory.
    اگر (is_separator((*b)[0], style)) {
      واپس *b;
    }
  }

  واپس StringRef();
}

StringRef جڑ_name(StringRef راستہ_2, Style style) {
  const_iterator b = begin(راستہ_2, style), e = انتہا(راستہ_2);
  اگر (b != e) {
    منطقی has_net =
        b->سائز_2() > 2 && is_separator((*b)[0], style) && (*b)[1] == (*b)[0];
    منطقی has_drive = is_style_windows(style) && b->سرے_with(":");

    اگر (has_net || has_drive) {
      // just {C:,//net}, return the first component.
      واپس *b;
    }
  }

  // No path or no name.
  واپس StringRef();
}

StringRef جڑ_directory(StringRef راستہ_2, Style style) {
  const_iterator b = begin(راستہ_2, style), pos = b, e = انتہا(راستہ_2);
  اگر (b != e) {
    منطقی has_net =
        b->سائز_2() > 2 && is_separator((*b)[0], style) && (*b)[1] == (*b)[0];
    منطقی has_drive = is_style_windows(style) && b->سرے_with(":");

    اگر ((has_net || has_drive) &&
        // {C:,//net}, skip to the next component.
        (++pos != e) && is_separator((*pos)[0], style)) {
      واپس *pos;
    }

    // POSIX style root directory.
    اگر (!has_net && is_separator((*b)[0], style)) {
      واپس *b;
    }
  }

  // No path or no root.
  واپس StringRef();
}

StringRef relative_راستہ(StringRef راستہ_2, Style style) {
  StringRef جڑ_3 = جڑ_راستہ_2(راستہ_2, style);
  واپس راستہ_2.substr(جڑ_3.سائز_2());
}

خالی append(Small_سمتیہ_Impl<حرف> &راستہ_2, Style style, ثابت Twine &a,
            ثابت Twine &b, ثابت Twine &c, ثابت Twine &d) {
  SmallString<32> a_storage;
  SmallString<32> b_storage;
  SmallString<32> c_storage;
  SmallString<32> d_storage;

  Small_سمتیہ<StringRef, 4> components;
  اگر (!a.is_Trivially_خالی_خانے()) components.ڈالو_عقب(a.مقصد_String_Ref(a_storage));
  اگر (!b.is_Trivially_خالی_خانے()) components.ڈالو_عقب(b.مقصد_String_Ref(b_storage));
  اگر (!c.is_Trivially_خالی_خانے()) components.ڈالو_عقب(c.مقصد_String_Ref(c_storage));
  اگر (!d.is_Trivially_خالی_خانے()) components.ڈالو_عقب(d.مقصد_String_Ref(d_storage));

  برائے (خودکار &component : components) {
    منطقی راستہ_has_sep =
        !راستہ_2.خالی_خانے() && is_separator(راستہ_2[راستہ_2.سائز_2() - 1], style);
    اگر (راستہ_has_sep) {
      // Strip separators from beginning of component.
      size_t loc = component.ڈھونڈو_پہلا_not_کا(separators(style));
      StringRef c = component.substr(loc);

      // Append it.
      راستہ_2.append(c.begin(), c.انتہا());
      جاری;
    }

    منطقی component_has_sep =
        !component.خالی_خانے() && is_separator(component[0], style);
    اگر (!component_has_sep &&
        !(راستہ_2.خالی_خانے() || has_جڑ_name(component, style))) {
      // Add a separator.
      راستہ_2.ڈالو_عقب(preferred_separator(style));
    }

    راستہ_2.append(component.begin(), component.انتہا());
  }
}

خالی append(Small_سمتیہ_Impl<حرف> &راستہ_2, ثابت Twine &a, ثابت Twine &b,
            ثابت Twine &c, ثابت Twine &d) {
  append(راستہ_2, Style::native, a, b, c, d);
}

خالی append(Small_سمتیہ_Impl<حرف> &راستہ_2, const_iterator begin,
            const_iterator انتہا, Style style) {
  برائے (; begin != انتہا; ++begin)
    راستہ_2::append(راستہ_2, style, *begin);
}

StringRef والد_راستہ(StringRef راستہ_2, Style style) {
  size_t انتہا_pos = والد_راستہ_انتہا(راستہ_2, style);
  اگر (انتہا_pos == StringRef::npos)
    واپس StringRef();
  واپس راستہ_2.substr(0, انتہا_pos);
}

خالی remove_filename(Small_سمتیہ_Impl<حرف> &راستہ_2, Style style) {
  size_t انتہا_pos = والد_راستہ_انتہا(StringRef(راستہ_2.begin(), راستہ_2.سائز_2()), style);
  اگر (انتہا_pos != StringRef::npos)
    راستہ_2.truncate(انتہا_pos);
}

خالی replace_extension(Small_سمتیہ_Impl<حرف> &راستہ_2, ثابت Twine &extension,
                       Style style) {
  StringRef p(راستہ_2.begin(), راستہ_2.سائز_2());
  SmallString<32> ext_storage;
  StringRef ext = extension.مقصد_String_Ref(ext_storage);

  // Erase existing extension.
  size_t pos = p.ڈھونڈو_آخری_کا('.');
  اگر (pos != StringRef::npos && pos >= filename_pos(p, style))
    راستہ_2.truncate(pos);

  // Append '.' if needed.
  اگر (ext.سائز_2() > 0 && ext[0] != '.')
    راستہ_2.ڈالو_عقب('.');

  // Append extension.
  راستہ_2.append(ext.begin(), ext.انتہا());
}

ساکن منطقی starts_with(StringRef راستہ, StringRef سابقہ,
                        Style style = Style::native) {
  // Windows prefix matching : case and separator insensitive
  اگر (is_style_windows(style)) {
    اگر (راستہ.سائز_2() < سابقہ.سائز_2())
      واپس جھوٹ;
    برائے (size_t I = 0, E = سابقہ.سائز_2(); I != E; ++I) {
      منطقی Sep_راستہ = is_separator(راستہ[I], style);
      منطقی Sep_سابقہ = is_separator(سابقہ[I], style);
      اگر (Sep_راستہ != Sep_سابقہ)
        واپس جھوٹ;
      اگر (!Sep_راستہ && مقصد_زیریں(راستہ[I]) != مقصد_زیریں(سابقہ[I]))
        واپس جھوٹ;
    }
    واپس سچ;
  }
  واپس راستہ.starts_with(سابقہ);
}

منطقی replace_راستہ_سابقہ(Small_سمتیہ_Impl<حرف> &راستہ, StringRef Old_سابقہ,
                         StringRef New_سابقہ, Style style) {
  اگر (Old_سابقہ.خالی_خانے() && New_سابقہ.خالی_خانے())
    واپس جھوٹ;

  StringRef Orig_راستہ(راستہ.begin(), راستہ.سائز_2());
  اگر (!starts_with(Orig_راستہ, Old_سابقہ, style))
    واپس جھوٹ;

  // If prefixes have the same size we can simply copy the new one over.
  اگر (Old_سابقہ.سائز_2() == New_سابقہ.سائز_2()) {
    llvm::copy(New_سابقہ, راستہ.begin());
    واپس سچ;
  }

  StringRef Rel_راستہ = Orig_راستہ.substr(Old_سابقہ.سائز_2());
  SmallString<256> New_راستہ;
  (Twine(New_سابقہ) + Rel_راستہ).مقصد_سمتیہ(New_راستہ);
  راستہ.تبادلہ(New_راستہ);
  واپس سچ;
}

خالی native(ثابت Twine &راستہ_2, Small_سمتیہ_Impl<حرف> &نتیجہ_2, Style style) {
  أكد((!راستہ_2.isSingleStringRef() ||
          راستہ_2.getSingleStringRef().ڈیٹا() != نتیجہ_2.ڈیٹا()) &&
         "path and result are not allowed to overlap!");
  // Clear result.
  نتیجہ_2.clear();
  راستہ_2.مقصد_سمتیہ(نتیجہ_2);
  native(نتیجہ_2, style);
}

std::string native(ثابت Twine &راستہ_2, Style style) {
  SmallString<128> نتیجہ;
  native(راستہ_2, نتیجہ, style);
  واپس std::string(نتیجہ);
}

خالی native(Small_سمتیہ_Impl<حرف> &راستہ, Style style) {
  اگر (راستہ.خالی_خانے())
    واپس;
  اگر (is_style_windows(style)) {
    برائے (حرف &Ch : راستہ)
      اگر (is_separator(Ch, style))
        Ch = preferred_separator(style);
    اگر (راستہ[0] == '~' && (راستہ.سائز_2() == 1 || is_separator(راستہ[1], style))) {
      SmallString<128> راستہ_Home;
      home_directory(راستہ_Home);
      راستہ_Home.append(راستہ.begin() + 1, راستہ.انتہا());
      راستہ = std::move(راستہ_Home);
    }
  } ورنہ {
    llvm::replace(راستہ, '\\', '/');
  }
}

std::string convert_مقصد_slash(StringRef راستہ_2, Style style) {
  اگر (is_style_posix(style))
    واپس std::string(راستہ_2);

  std::string s = راستہ_2.str();
  llvm::replace(s, '\\', '/');
  واپس s;
}

StringRef filename(StringRef راستہ_2, Style style) { واپس *rbegin(راستہ_2, style); }

StringRef stem(StringRef راستہ_2, Style style) {
  StringRef fname = filename(راستہ_2, style);
  size_t pos = fname.ڈھونڈو_آخری_کا('.');
  اگر (pos == StringRef::npos)
    واپس fname;
  اگر ((fname.سائز_2() == 1 && fname == ".") ||
      (fname.سائز_2() == 2 && fname == ".."))
    واپس fname;
  واپس fname.substr(0, pos);
}

StringRef extension(StringRef راستہ_2, Style style) {
  StringRef fname = filename(راستہ_2, style);
  size_t pos = fname.ڈھونڈو_آخری_کا('.');
  اگر (pos == StringRef::npos)
    واپس StringRef();
  اگر ((fname.سائز_2() == 1 && fname == ".") ||
      (fname.سائز_2() == 2 && fname == ".."))
    واپس StringRef();
  واپس fname.substr(pos);
}

منطقی is_separator(حرف قدر_2, Style style) {
  اگر (قدر_2 == '/')
    واپس سچ;
  اگر (is_style_windows(style))
    واپس قدر_2 == '\\';
  واپس جھوٹ;
}

StringRef get_separator(Style style) {
  اگر (real_style(style) == Style::windows)
    واپس "\\";
  واپس "/";
}

منطقی has_جڑ_name(ثابت Twine &راستہ_2, Style style) {
  SmallString<128> راستہ_storage;
  StringRef p = راستہ_2.مقصد_String_Ref(راستہ_storage);

  واپس !جڑ_name(p, style).خالی_خانے();
}

منطقی has_جڑ_directory(ثابت Twine &راستہ_2, Style style) {
  SmallString<128> راستہ_storage;
  StringRef p = راستہ_2.مقصد_String_Ref(راستہ_storage);

  واپس !جڑ_directory(p, style).خالی_خانے();
}

منطقی has_جڑ_راستہ(ثابت Twine &راستہ_2, Style style) {
  SmallString<128> راستہ_storage;
  StringRef p = راستہ_2.مقصد_String_Ref(راستہ_storage);

  واپس !جڑ_راستہ_2(p, style).خالی_خانے();
}

منطقی has_relative_راستہ(ثابت Twine &راستہ_2, Style style) {
  SmallString<128> راستہ_storage;
  StringRef p = راستہ_2.مقصد_String_Ref(راستہ_storage);

  واپس !relative_راستہ(p, style).خالی_خانے();
}

منطقی has_filename(ثابت Twine &راستہ_2, Style style) {
  SmallString<128> راستہ_storage;
  StringRef p = راستہ_2.مقصد_String_Ref(راستہ_storage);

  واپس !filename(p, style).خالی_خانے();
}

منطقی has_والد_راستہ(ثابت Twine &راستہ_2, Style style) {
  SmallString<128> راستہ_storage;
  StringRef p = راستہ_2.مقصد_String_Ref(راستہ_storage);

  واپس !والد_راستہ(p, style).خالی_خانے();
}

منطقی has_stem(ثابت Twine &راستہ_2, Style style) {
  SmallString<128> راستہ_storage;
  StringRef p = راستہ_2.مقصد_String_Ref(راستہ_storage);

  واپس !stem(p, style).خالی_خانے();
}

منطقی has_extension(ثابت Twine &راستہ_2, Style style) {
  SmallString<128> راستہ_storage;
  StringRef p = راستہ_2.مقصد_String_Ref(راستہ_storage);

  واپس !extension(p, style).خالی_خانے();
}

منطقی is_absolute(ثابت Twine &راستہ_2, Style style) {
  SmallString<128> راستہ_storage;
  StringRef p = راستہ_2.مقصد_String_Ref(راستہ_storage);

  منطقی جڑ_Dir = has_جڑ_directory(p, style);
  منطقی جڑ_Name = is_style_posix(style) || has_جڑ_name(p, style);

  واپس جڑ_Dir && جڑ_Name;
}

منطقی is_absolute_gnu(ثابت Twine &راستہ_2, Style style) {
  SmallString<128> راستہ_storage;
  StringRef p = راستہ_2.مقصد_String_Ref(راستہ_storage);

  // Handle '/' which is absolute for both Windows and POSIX systems.
  // Handle '\\' on Windows.
  اگر (!p.خالی_خانے() && is_separator(p.front(), style))
    واپس سچ;

  اگر (is_style_windows(style)) {
    // Handle drive letter pattern (a character followed by ':') on Windows.
    اگر (p.سائز_2() >= 2 && (p[0] && p[1] == ':'))
      واپس سچ;
  }

  واپس جھوٹ;
}

منطقی is_relative(ثابت Twine &راستہ_2, Style style) {
  واپس !is_absolute(راستہ_2, style);
}

خالی make_absolute(ثابت Twine &current_directory,
                   Small_سمتیہ_Impl<حرف> &راستہ_2) {
  StringRef p(راستہ_2.ڈیٹا(), راستہ_2.سائز_2());

  منطقی جڑ_Directory = has_جڑ_directory(p);
  منطقی جڑ_Name = has_جڑ_name(p);

  // Already absolute.
  اگر ((جڑ_Name || is_style_posix(Style::native)) && جڑ_Directory)
    واپس;

  // All the following conditions will need the current directory.
  SmallString<128> current_dir;
  current_directory.مقصد_سمتیہ(current_dir);

  // Relative path. Prepend the current directory.
  اگر (!جڑ_Name && !جڑ_Directory) {
    // Append path to the current directory.
    append(current_dir, p);
    // Set path to the result.
    راستہ_2.تبادلہ(current_dir);
    واپس;
  }

  اگر (!جڑ_Name && جڑ_Directory) {
    StringRef cdrn = جڑ_name(current_dir);
    SmallString<128> cur_Dir_جڑ_Name(cdrn.begin(), cdrn.انتہا());
    append(cur_Dir_جڑ_Name, p);
    // Set path to the result.
    راستہ_2.تبادلہ(cur_Dir_جڑ_Name);
    واپس;
  }

  اگر (جڑ_Name && !جڑ_Directory) {
    StringRef p_جڑ_Name = جڑ_name(p);
    StringRef b_جڑ_Directory = جڑ_directory(current_dir);
    StringRef b_Relative_راستہ = relative_راستہ(current_dir);
    StringRef p_Relative_راستہ = relative_راستہ(p);

    SmallString<128> res;
    append(res, p_جڑ_Name, b_جڑ_Directory, b_Relative_راستہ, p_Relative_راستہ);
    راستہ_2.تبادلہ(res);
    واپس;
  }

  llvm_unreachable("All rootName and rootDirectory combinations should have "
                   "occurred above!");
}

StringRef remove_leading_dotslash(StringRef راستہ, Style style) {
  // Remove leading "./" (or ".//" or "././" etc.)
  جب (راستہ.سائز_2() > 2 && راستہ[0] == '.' && is_separator(راستہ[1], style)) {
    راستہ = راستہ.substr(2);
    جب (راستہ.سائز_2() > 0 && is_separator(راستہ[0], style))
      راستہ = راستہ.substr(1);
  }
  واپس راستہ;
}

منطقی remove_dots(Small_سمتیہ_Impl<حرف> &the_راستہ, منطقی remove_dot_dot,
                 Style style) {
  style = real_style(style);
  StringRef بقایا_2(the_راستہ.ڈیٹا(), the_راستہ.سائز_2());
  منطقی needs_change = جھوٹ;
  Small_سمتیہ<StringRef, 16> components;

  // Consume the root path, if present.
  StringRef جڑ_3 = راستہ_2::جڑ_راستہ_2(بقایا_2, style);
  منطقی absolute = !جڑ_3.خالی_خانے();
  اگر (absolute)
    بقایا_2 = بقایا_2.drop_front(جڑ_3.سائز_2());

  // Loop over path components manually. This makes it easier to detect
  // non-preferred slashes and double separators that must be canonicalized.
  جب (!بقایا_2.خالی_خانے()) {
    size_t اگلا_slash = بقایا_2.ڈھونڈو_پہلا_کا(separators(style));
    اگر (اگلا_slash == StringRef::npos)
      اگلا_slash = بقایا_2.سائز_2();
    StringRef component = بقایا_2.take_front(اگلا_slash);
    بقایا_2 = بقایا_2.drop_front(اگلا_slash);

    // Eat the slash, and check if it is the preferred separator.
    اگر (!بقایا_2.خالی_خانے()) {
      needs_change |= بقایا_2.front() != preferred_separator(style);
      بقایا_2 = بقایا_2.drop_front();
      // The path needs to be rewritten if it has a trailing slash.
      // FIXME: This is emergent behavior that could be removed.
      needs_change |= بقایا_2.خالی_خانے();
    }

    // Check for path traversal components or double separators.
    اگر (component.خالی_خانے() || component == ".") {
      needs_change = سچ;
    } ورنہ اگر (remove_dot_dot && component == "..") {
      needs_change = سچ;
      // Do not allow ".." to remove the root component. If this is the
      // beginning of a relative path, keep the ".." component.
      اگر (!components.خالی_خانے() && components.عقب() != "..") {
        components.نکالو_عقب();
      } ورنہ اگر (!absolute) {
        components.ڈالو_عقب(component);
      }
    } ورنہ {
      components.ڈالو_عقب(component);
    }
  }

  SmallString<256> بفر_2 = جڑ_3;
  // "root" could be "/", which may need to be translated into "\".
  make_preferred(بفر_2, style);
  needs_change |= جڑ_3 != بفر_2;

  // Avoid rewriting the path unless we have to.
  اگر (!needs_change)
    واپس جھوٹ;

  اگر (!components.خالی_خانے()) {
    بفر_2 += components[0];
    برائے (StringRef C : ArrayRef(components).drop_front()) {
      بفر_2 += preferred_separator(style);
      بفر_2 += C;
    }
  }
  the_راستہ.تبادلہ(بفر_2);
  واپس سچ;
}

} // end namespace path

نامخلا fs {

std::error_رمز getUniqueID(ثابت Twine راستہ, UniqueID &نتیجہ) {
  sandbox::violationIfEnabled();

  file_حالت حالت_2;
  std::error_رمز EC = حالت_3(راستہ, حالت_2);
  اگر (EC)
    واپس EC;
  نتیجہ = حالت_2.getUniqueID();
  واپس std::error_رمز();
}

خالی create_Unique_راستہ(ثابت Twine &Model, Small_سمتیہ_Impl<حرف> &نتیجہ_راستہ,
                      منطقی MakeAbsolute) {
  SmallString<128> ModelStorage;
  Model.مقصد_سمتیہ(ModelStorage);

  أكد(llvm::is_contained(ModelStorage, '%') &&
         "createUniquePath: Model must contain at least one '%'");

  اگر (MakeAbsolute) {
    // Make model absolute by prepending a temp directory if it's not already.
    اگر (!sys::راستہ_2::is_absolute(Twine(ModelStorage))) {
      SmallString<128> TDir;
      sys::راستہ_2::system_عارضی_directory(سچ, TDir);
      sys::راستہ_2::append(TDir, Twine(ModelStorage));
      ModelStorage.تبادلہ(TDir);
    }
  }

  نتیجہ_راستہ = ModelStorage;
  نتیجہ_راستہ.ڈالو_عقب(0);
  نتیجہ_راستہ.نکالو_عقب();

  // Replace '%' with random chars.
  برائے (غیرعلامتی i = 0, e = ModelStorage.سائز_2(); i != e; ++i) {
    اگر (ModelStorage[i] == '%')
      نتیجہ_راستہ[i] = "0123456789abcdef"[sys::Process::Get_Random_نمبر() & 15];
  }
}

std::error_رمز createUniqueFile(ثابت Twine &Model, صحیح &نتیجہ_Fd,
                                 Small_سمتیہ_Impl<حرف> &نتیجہ_راستہ,
                                 OpenFlags Flags, غیرعلامتی منوال) {
  واپس createUniqueEntity(Model, نتیجہ_Fd, نتیجہ_راستہ, جھوٹ, FS_File, Flags,
                            منوال);
}

std::error_رمز createUniqueFile(ثابت Twine &Model,
                                 Small_سمتیہ_Impl<حرف> &نتیجہ_راستہ,
                                 غیرعلامتی منوال) {
  صحیح FD;
  خودکار EC = createUniqueFile(Model, FD, نتیجہ_راستہ, کا_None, منوال);
  اگر (EC)
    واپس EC;
  // FD is only needed to avoid race conditions. Close it right away.
  close(FD);
  واپس EC;
}

ساکن std::error_رمز
createTemporaryFile(ثابت Twine &Model, صحیح &نتیجہ_FD,
                    llvm::Small_سمتیہ_Impl<حرف> &نتیجہ_راستہ, FSEntity Type,
                    sys::fs::OpenFlags Flags = sys::fs::کا_None) {
  // Any *temporary* file is assumed to be a compiler-internal output, not
  // a formal one.
  خودکار BypassSandbox = sys::sandbox::scopedDisable();

  SmallString<128> Storage;
  StringRef P = Model.مقصد_Null_Terminated_String_Ref(Storage);
  أكد(P.ڈھونڈو_پہلا_کا(separators(Style::native)) == StringRef::npos &&
         "Model must be a simple filename.");
  // Use P.begin() so that createUniqueEntity doesn't need to recreate Storage.
  واپس createUniqueEntity(P.begin(), نتیجہ_FD, نتیجہ_راستہ, سچ, Type, Flags,
                            all_read | all_write);
}

ساکن std::error_رمز
createTemporaryFile(ثابت Twine &سابقہ, StringRef Suffix, صحیح &نتیجہ_FD,
                    llvm::Small_سمتیہ_Impl<حرف> &نتیجہ_راستہ, FSEntity Type,
                    sys::fs::OpenFlags Flags = sys::fs::کا_None) {
  ثابت حرف *درمیان = Suffix.خالی_خانے() ? "-%%%%%%" : "-%%%%%%.";
  واپس createTemporaryFile(سابقہ + درمیان + Suffix, نتیجہ_FD, نتیجہ_راستہ,
                             Type, Flags);
}

std::error_رمز createTemporaryFile(ثابت Twine &سابقہ, StringRef Suffix,
                                    صحیح &نتیجہ_FD,
                                    Small_سمتیہ_Impl<حرف> &نتیجہ_راستہ,
                                    sys::fs::OpenFlags Flags) {
  واپس createTemporaryFile(سابقہ, Suffix, نتیجہ_FD, نتیجہ_راستہ, FS_File,
                             Flags);
}

std::error_رمز createTemporaryFile(ثابت Twine &سابقہ, StringRef Suffix,
                                    Small_سمتیہ_Impl<حرف> &نتیجہ_راستہ,
                                    sys::fs::OpenFlags Flags) {
  صحیح FD;
  خودکار EC = createTemporaryFile(سابقہ, Suffix, FD, نتیجہ_راستہ, Flags);
  اگر (EC)
    واپس EC;
  // FD is only needed to avoid race conditions. Close it right away.
  close(FD);
  واپس EC;
}

// This is a mkdtemp with a different pattern. We use createUniqueEntity mostly
// for consistency. We should try using mkdtemp.
std::error_رمز createUniqueDirectory(ثابت Twine &سابقہ,
                                      Small_سمتیہ_Impl<حرف> &نتیجہ_راستہ) {
  صحیح Dummy;
  واپس createUniqueEntity(سابقہ + "-%%%%%%", Dummy, نتیجہ_راستہ, سچ,
                            FS_Dir);
}

std::error_رمز
getPotentiallyUniqueFileName(ثابت Twine &Model,
                             Small_سمتیہ_Impl<حرف> &نتیجہ_راستہ) {
  صحیح Dummy;
  واپس createUniqueEntity(Model, Dummy, نتیجہ_راستہ, جھوٹ, FS_Name);
}

std::error_رمز
get_Potentially_Unique_عارضی_File_Name(ثابت Twine &سابقہ, StringRef Suffix,
                                 Small_سمتیہ_Impl<حرف> &نتیجہ_راستہ) {
  صحیح Dummy;
  واپس createTemporaryFile(سابقہ, Suffix, Dummy, نتیجہ_راستہ, FS_Name);
}

std::error_رمز make_absolute(Small_سمتیہ_Impl<حرف> &راستہ_2) {
  sandbox::violationIfEnabled();

  اگر (راستہ_2::is_absolute(راستہ_2))
    واپس {};

  SmallString<128> current_dir;
  اگر (std::error_رمز ec = current_راستہ(current_dir))
    واپس ec;

  راستہ_2::make_absolute(current_dir, راستہ_2);
  واپس {};
}

std::error_رمز create_directories(ثابت Twine &راستہ, منطقی IgnoreExisting,
                                   perms Perms) {
  SmallString<128> راستہ_Storage;
  StringRef P = راستہ.مقصد_String_Ref(راستہ_Storage);

  // Be optimistic and try to create the directory
  std::error_رمز EC = create_directory(P, IgnoreExisting, Perms);
  // If we succeeded, or had any error other than the parent not existing, just
  // return it.
  اگر (EC != errc::no_such_file_or_directory)
    واپس EC;

  // We failed because of a no_such_file_or_directory, try to create the
  // parent.
  StringRef والد_2 = راستہ_2::والد_راستہ(P);
  اگر (والد_2.خالی_خانے())
    واپس EC;

  اگر ((EC = create_directories(والد_2, IgnoreExisting, Perms)))
      واپس EC;

  واپس create_directory(P, IgnoreExisting, Perms);
}

ساکن std::error_رمز copy_file_اندرونی(صحیح ReadFD, صحیح WriteFD) {
  ثابت size_t Buf_سائز = 4096;
  حرف *Buf = نیا حرف[Buf_سائز];
  صحیح بائٹیں_Read = 0, بائٹیں_Written = 0;
  برائے (;;) {
    بائٹیں_Read = اقرأ(ReadFD, Buf, Buf_سائز);
    اگر (بائٹیں_Read <= 0)
      توڑو;
    جب (بائٹیں_Read) {
      بائٹیں_Written = اكتب(WriteFD, Buf, بائٹیں_Read);
      اگر (بائٹیں_Written < 0)
        توڑو;
      بائٹیں_Read -= بائٹیں_Written;
    }
    اگر (بائٹیں_Written < 0)
      توڑو;
  }
  حذف[] Buf;

  اگر (بائٹیں_Read < 0 || بائٹیں_Written < 0)
    واپس errno_As_Error_رمز();
  واپس std::error_رمز();
}

#اگر_غیر_تعریف_شدہ __APPLE__
std::error_رمز copy_file(ثابت Twine &From, ثابت Twine &مقصد) {
  صحیح ReadFD, WriteFD;
  اگر (std::error_رمز EC = openFileForRead(From, ReadFD, کا_None))
    واپس EC;
  اگر (std::error_رمز EC =
          openFileForWrite(مقصد, WriteFD, CD_CreateAlways, کا_None)) {
    close(ReadFD);
    واپس EC;
  }

  std::error_رمز EC = copy_file_اندرونی(ReadFD, WriteFD);

  close(ReadFD);
  close(WriteFD);

  واپس EC;
}
#اگر_ختم

std::error_رمز copy_file(ثابت Twine &From, صحیح مقصد_FD) {
  صحیح ReadFD;
  اگر (std::error_رمز EC = openFileForRead(From, ReadFD, کا_None))
    واپس EC;

  std::error_رمز EC = copy_file_اندرونی(ReadFD, مقصد_FD);

  close(ReadFD);

  واپس EC;
}

ErrorOr<MD5::MD5_نتیجہ> md5_contents(صحیح FD) {
  sandbox::violationIfEnabled();

  MD5 Hash;

  constexpr size_t Buf_سائز = 4096;
  std::سمتیہ<uint8_t> Buf(Buf_سائز);
  صحیح بائٹیں_Read = 0;
  برائے (;;) {
    بائٹیں_Read = اقرأ(FD, Buf.ڈیٹا(), Buf_سائز);
    اگر (بائٹیں_Read <= 0)
      توڑو;
    Hash.update(ArrayRef(Buf.ڈیٹا(), بائٹیں_Read));
  }

  اگر (بائٹیں_Read < 0)
    واپس errno_As_Error_رمز();
  MD5::MD5_نتیجہ نتیجہ;
  Hash.final(نتیجہ);
  واپس نتیجہ;
}

ErrorOr<MD5::MD5_نتیجہ> md5_contents(ثابت Twine &راستہ) {
  sandbox::violationIfEnabled();

  صحیح FD;
  اگر (خودکار EC = openFileForRead(راستہ, FD, کا_None))
    واپس EC;

  خودکار نتیجہ = md5_contents(FD);
  close(FD);
  واپس نتیجہ;
}

منطقی exists(ثابت basic_file_حالت &حالت_3) {
  واپس حالت_معلوم(حالت_3) && حالت_3.type() != file_type::file_not_ملا;
}

منطقی حالت_معلوم(ثابت basic_file_حالت &s) {
  واپس s.type() != file_type::حالت_error;
}

file_type get_file_type(ثابت Twine &راستہ, منطقی Follow) {
  file_حالت st;
  اگر (حالت_3(راستہ, st, Follow))
    واپس file_type::حالت_error;
  واپس st.type();
}

منطقی is_directory(ثابت basic_file_حالت &حالت_3) {
  واپس حالت_3.type() == file_type::directory_file;
}

std::error_رمز is_directory(ثابت Twine &راستہ_2, منطقی &نتیجہ_2) {
  sandbox::violationIfEnabled();

  file_حالت st;
  اگر (std::error_رمز ec = حالت_3(راستہ_2, st))
    واپس ec;
  نتیجہ_2 = is_directory(st);
  واپس std::error_رمز();
}

منطقی is_regular_file(ثابت basic_file_حالت &حالت_3) {
  واپس حالت_3.type() == file_type::regular_file;
}

std::error_رمز is_regular_file(ثابت Twine &راستہ_2, منطقی &نتیجہ_2) {
  sandbox::violationIfEnabled();

  file_حالت st;
  اگر (std::error_رمز ec = حالت_3(راستہ_2, st))
    واپس ec;
  نتیجہ_2 = is_regular_file(st);
  واپس std::error_رمز();
}

منطقی is_symlink_file(ثابت basic_file_حالت &حالت_3) {
  واپس حالت_3.type() == file_type::symlink_file;
}

std::error_رمز is_symlink_file(ثابت Twine &راستہ_2, منطقی &نتیجہ_2) {
  sandbox::violationIfEnabled();

  file_حالت st;
  اگر (std::error_رمز ec = حالت_3(راستہ_2, st, جھوٹ))
    واپس ec;
  نتیجہ_2 = is_symlink_file(st);
  واپس std::error_رمز();
}

منطقی is_other(ثابت basic_file_حالت &حالت_3) {
  واپس exists(حالت_3) &&
         !is_regular_file(حالت_3) &&
         !is_directory(حالت_3);
}

std::error_رمز is_other(ثابت Twine &راستہ, منطقی &نتیجہ) {
  sandbox::violationIfEnabled();

  file_حالت File_حالت;
  اگر (std::error_رمز EC = حالت_3(راستہ, File_حالت))
    واپس EC;
  نتیجہ = is_other(File_حالت);
  واپس std::error_رمز();
}

خالی directory_اندراج::replace_filename(ثابت Twine &Filename, file_type Type,
                                       basic_file_حالت حالت_2) {
  SmallString<128> راستہ_Str = راستہ_2::والد_راستہ(راستہ);
  راستہ_2::append(راستہ_Str, Filename);
  یہ->راستہ = std::string(راستہ_Str);
  یہ->Type = Type;
  یہ->حالت_2 = حالت_2;
}

ErrorOr<perms> getPermissions(ثابت Twine &راستہ) {
  sandbox::violationIfEnabled();

  file_حالت حالت_2;
  اگر (std::error_رمز EC = حالت_3(راستہ, حالت_2))
    واپس EC;

  واپس حالت_2.permissions();
}

std::error_رمز سیٹ_آخری_Access_And_Modification_وقت(ثابت Twine &راستہ,
                                                 وقت_نقطہ<> Access_وقت,
                                                 وقت_نقطہ<> Modification_وقت) {
  صحیح FD;
  اگر (std::error_رمز EC = openFile(راستہ, FD, CD_OpenExisting, FA_Read,
                                    کا_Update_اوصاف | کا_Open_Directory))
    واپس EC;
  std::error_رمز EC =
      سیٹ_آخری_Access_And_Modification_وقت(FD, Access_وقت, Modification_وقت);
  Process::SafelyCloseFileDescriptor(FD);
  واپس EC;
}

size_t mapped_file_region::سائز_2() ثابت {
  أكد(Mapping && "Mapping failed but used anyway!");
  واپس سائز;
}

حرف *mapped_file_region::ڈیٹا() ثابت {
  أكد(Mapping && "Mapping failed but used anyway!");
  واپس reinterpret_cast<حرف *>(Mapping);
}

ثابت حرف *mapped_file_region::const_ڈیٹا() ثابت {
  أكد(Mapping && "Mapping failed but used anyway!");
  واپس reinterpret_cast<ثابت حرف *>(Mapping);
}

Error read_Native_File_مقصد_EOF(file_t FileHandle, Small_سمتیہ_Impl<حرف> &بفر,
                          ssize_t Chunk_سائز) {
  sandbox::violationIfEnabled();

  // Install a handler to truncate the buffer to the correct size on exit.
  size_t سائز = بفر.سائز_2();
  llvm::scope_exit TruncateOnExit([&]() { بفر.truncate(سائز); });

  // Read into Buffer until we hit EOF.
  برائے (;;) {
    بفر.resize_for_overwrite(سائز + Chunk_سائز);
    Expected<size_t> Read_بائٹیں = readNativeFile(
        FileHandle, MutableArrayRef(بفر.begin() + سائز, Chunk_سائز));
    اگر (!Read_بائٹیں)
      واپس Read_بائٹیں.takeError();
    اگر (*Read_بائٹیں == 0)
      واپس Error::success();
    سائز += *Read_بائٹیں;
  }
}

} // end namespace fs
} // end namespace sys
} // end namespace llvm

// Include the truly platform-specific parts.
#اگر defined(LLVM_ON_UNIX)
#شامل "Unix/Path.inc"
#اگر_ختم
#اگر defined(_WIN32)
#شامل "Windows/Path.inc"
#اگر_ختم

نامخلا llvm {
نامخلا sys {
نامخلا fs {

عارضی_File::عارضی_File(StringRef Name, صحیح FD)
    : TmpName(std::string(Name)), FD(FD) {}
عارضی_File::عارضی_File(عارضی_File &&Other) { *یہ = std::move(Other); }
عارضی_File &عارضی_File::عامل=(عارضی_File &&Other) {
  TmpName = std::move(Other.TmpName);
  FD = Other.FD;
  Other.مکمل = سچ;
  Other.FD = -1;
#اگر_تعریف_شدہ _WIN32
  RemoveOnClose = Other.RemoveOnClose;
  Other.RemoveOnClose = جھوٹ;
#اگر_ختم
  واپس *یہ;
}

عارضی_File::~عارضی_File() { أكد(مکمل); }

Error عارضی_File::discard() {
  مکمل = سچ;
  اگر (FD != -1 && close(FD) == -1) {
    std::error_رمز EC = errno_As_Error_رمز();
    واپس error_رمز_مقصد_Error(EC);
  }
  FD = -1;

#اگر_تعریف_شدہ _WIN32
  // On Windows, closing will remove the file, if we set the delete
  // disposition. If not, remove it manually.
  منطقی Remove = RemoveOnClose;
#ورنہ
  // Always try to remove the file.
  منطقی Remove = سچ;
#اگر_ختم
  std::error_رمز RemoveEC;
  اگر (Remove && !TmpName.خالی_خانے()) {
    RemoveEC = fs::remove(TmpName);
    sys::DontRemoveFileOnSignal(TmpName);
    اگر (!RemoveEC)
      TmpName = "";
  } ورنہ {
    TmpName = "";
  }
  واپس error_رمز_مقصد_Error(RemoveEC);
}

Error عارضی_File::keep(ثابت Twine &Name) {
  أكد(!مکمل);
  مکمل = سچ;
  // Always try to close and rename.
#اگر_تعریف_شدہ _WIN32
  // If we can't cancel the delete don't rename.
  خودکار H = reinterpret_cast<HANDLE>(_get_osfhandle(FD));
  std::error_رمز RenameEC =
      RemoveOnClose ? std::error_رمز() : سیٹ_Delete_Disposition(H, جھوٹ);
  منطقی ShouldDelete = جھوٹ;
  اگر (!RenameEC) {
    RenameEC = rename_handle(H, Name);
    // If rename failed because it's cross-device, copy instead
    اگر (RenameEC ==
      std::error_رمز(ERROR_NOT_SAME_DEVICE, std::system_category())) {
      RenameEC = copy_file(TmpName, Name);
      ShouldDelete = سچ;
    }
  }

  // If we can't rename or copy, discard the temporary file.
  اگر (RenameEC)
    ShouldDelete = سچ;
  اگر (ShouldDelete) {
    اگر (!RemoveOnClose)
      سیٹ_Delete_Disposition(H, سچ);
    ورنہ
      remove(TmpName);
  }
#ورنہ
  std::error_رمز RenameEC = fs::rename(TmpName, Name);
  اگر (RenameEC) {
    // If we can't rename, try to copy to work around cross-device link issues.
    RenameEC = sys::fs::copy_file(TmpName, Name);
    // If we can't rename or copy, discard the temporary file.
    اگر (RenameEC)
      remove(TmpName);
  }
#اگر_ختم
  sys::DontRemoveFileOnSignal(TmpName);

  اگر (!RenameEC)
    TmpName = "";

  اگر (close(FD) == -1)
    واپس error_رمز_مقصد_Error(errno_As_Error_رمز());
  FD = -1;

  واپس error_رمز_مقصد_Error(RenameEC);
}

Error عارضی_File::keep() {
  أكد(!مکمل);
  مکمل = سچ;

#اگر_تعریف_شدہ _WIN32
  خودکار H = reinterpret_cast<HANDLE>(_get_osfhandle(FD));
  اگر (std::error_رمز EC = سیٹ_Delete_Disposition(H, جھوٹ))
    واپس error_رمز_مقصد_Error(EC);
#اگر_ختم
  sys::DontRemoveFileOnSignal(TmpName);

  TmpName = "";

  اگر (close(FD) == -1)
    واپس error_رمز_مقصد_Error(errno_As_Error_رمز());
  FD = -1;

  واپس Error::success();
}

Expected<عارضی_File> عارضی_File::create(ثابت Twine &Model, غیرعلامتی منوال,
                                    OpenFlags ExtraFlags) {
  صحیح FD;
  SmallString<128> نتیجہ_راستہ;
  اگر (std::error_رمز EC =
          createUniqueFile(Model, FD, نتیجہ_راستہ, کا_Delete | ExtraFlags, منوال))
    واپس error_رمز_مقصد_Error(EC);

  عارضی_File Ret(نتیجہ_راستہ, FD);
#اگر_تعریف_شدہ _WIN32
  خودکار H = reinterpret_cast<HANDLE>(_get_osfhandle(FD));
  منطقی سیٹ_Signal_Handler = جھوٹ;
  اگر (std::error_رمز EC = سیٹ_Delete_Disposition(H, سچ)) {
    Ret.RemoveOnClose = سچ;
    سیٹ_Signal_Handler = سچ;
  }
#ورنہ
  منطقی سیٹ_Signal_Handler = سچ;
#اگر_ختم
  اگر (سیٹ_Signal_Handler && sys::RemoveFileOnSignal(نتیجہ_راستہ)) {
    // Make sure we delete the file when RemoveFileOnSignal fails.
    consumeError(Ret.discard());
    std::error_رمز EC(errc::operation_not_permitted);
    واپس error_رمز_مقصد_Error(EC);
  }
  واپس std::move(Ret);
}
} // namespace fs

} // namespace sys
} // namespace llvm
