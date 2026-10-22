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

#अन्तर्भाव "llvm/Support/Path.h"
#अन्तर्भाव "llvm/ADT/ArrayRef.h"
#अन्तर्भाव "llvm/ADT/ScopeExit.h"
#अन्तर्भाव "llvm/ADT/StringExtras.h"
#अन्तर्भाव "llvm/Config/config.h"
#अन्तर्भाव "llvm/Config/llvm-config.h"
#अन्तर्भाव "llvm/Support/Errc.h"
#अन्तर्भाव "llvm/Support/ErrorHandling.h"
#अन्तर्भाव "llvm/Support/FileSystem.h"
#अन्तर्भाव "llvm/Support/IOSandbox.h"
#अन्तर्भाव "llvm/Support/Process.h"
#अन्तर्भाव "llvm/Support/Signals.h"
#अन्तर्भाव <cctype>

#यदि !defined(_MSC_VER) && !defined(__MINGW32__)
#अन्तर्भाव <unistd.h>
#अन्यथा
#अन्तर्भाव <io.h>
#यदि_अन्तः

उपयुज्य नामाकाश llvm;
उपयुज्य नामाकाश llvm::support::endian;

नामाकाश {
  उपयुज्य llvm::StringRef;
  उपयुज्य llvm::sys::मार्गः_2::is_separator;
  उपयुज्य llvm::sys::मार्गः_2::Style;

  अन्तःस्थ तार्किक prefer_forward_slash() {
    स्थिर तार्किक prefer = []() {
      यदि (std::optional<std::string> Env =
              sys::Process::GetEnv("LLVM_WINDOWS_PREFER_FORWARD_SLASH"))
        प्रत्यागम *Env == "1";
      प्रत्यागम static_cast<तार्किक>(LLVM_WINDOWS_PREFER_FORWARD_SLASH);
    }();
    प्रत्यागम prefer;
  }

  अन्तःस्थ Style real_style(Style style) {
    यदि (style != Style::native)
      प्रत्यागम style;
    यदि (is_style_posix(style))
      प्रत्यागम Style::posix;
    प्रत्यागम prefer_forward_slash() ? Style::windows_slash
                                  : Style::windows_backslash;
  }

  अन्तःस्थ ध्रुव वर्ण *separators(Style style) {
    यदि (is_style_windows(style))
      प्रत्यागम "\\/";
    प्रत्यागम "/";
  }

  अन्तःस्थ वर्ण preferred_separator(Style style) {
    यदि (real_style(style) == Style::windows)
      प्रत्यागम '\\';
    प्रत्यागम '/';
  }

  StringRef find_first_component(StringRef मार्गः_2, Style style) {
    // Look for this first component in the following order.
    // * empty (in this case we return an empty string)
    // * either C: or {//,\\}net.
    // * {/,\}
    // * {file,directory}name

    यदि (मार्गः_2.रिक्तानि())
      प्रत्यागम मार्गः_2;

    यदि (is_style_windows(style)) {
      // C:
      यदि (मार्गः_2.आकार_2() >= 2 &&
          std::अक्षरम्_वा(static_cast<अचिह्नित वर्ण>(मार्गः_2[0])) && मार्गः_2[1] == ':')
        प्रत्यागम मार्गः_2.substr(0, 2);
    }

    // //net
    यदि ((मार्गः_2.आकार_2() > 2) && is_separator(मार्गः_2[0], style) &&
        मार्गः_2[0] == मार्गः_2[1] && !is_separator(मार्गः_2[2], style)) {
      // Find the next directory separator.
      size_t अन्तः = मार्गः_2.find_first_सम्बन्धि(separators(style), 2);
      प्रत्यागम मार्गः_2.substr(0, अन्तः);
    }

    // {/,\}
    यदि (is_separator(मार्गः_2[0], style))
      प्रत्यागम मार्गः_2.substr(0, 1);

    // * {file,directory}name
    size_t अन्तः = मार्गः_2.find_first_सम्बन्धि(separators(style));
    प्रत्यागम मार्गः_2.substr(0, अन्तः);
  }

  // Returns the first character of the filename in str. For paths ending in
  // '/', it returns the position of the '/'.
  size_t filename_pos(StringRef str, Style style) {
    यदि (str.आकार_2() > 0 && is_separator(str[str.आकार_2() - 1], style))
      प्रत्यागम str.आकार_2() - 1;

    size_t pos = str.find_चरमः_सम्बन्धि(separators(style), str.आकार_2() - 1);

    यदि (is_style_windows(style)) {
      यदि (pos == StringRef::npos)
        pos = str.find_चरमः_सम्बन्धि(':', str.आकार_2() - 1);
    }

    यदि (pos == StringRef::npos || (pos == 1 && is_separator(str[0], style)))
      प्रत्यागम 0;

    प्रत्यागम pos + 1;
  }

  // Returns the position of the root directory in str. If there is no root
  // directory in str, it returns StringRef::npos.
  size_t मूलम्_dir_start(StringRef str, Style style) {
    // case "c:/"
    यदि (is_style_windows(style)) {
      यदि (str.आकार_2() > 2 && str[1] == ':' && is_separator(str[2], style))
        प्रत्यागम 2;
    }

    // case "//net"
    यदि (str.आकार_2() > 3 && is_separator(str[0], style) && str[0] == str[1] &&
        !is_separator(str[2], style)) {
      प्रत्यागम str.find_first_सम्बन्धि(separators(style), 2);
    }

    // case "/"
    यदि (str.आकार_2() > 0 && is_separator(str[0], style))
      प्रत्यागम 0;

    प्रत्यागम StringRef::npos;
  }

  // Returns the position past the end of the "parent path" of path. The parent
  // path will not end in '/', unless the parent is the root directory. If the
  // path has no parent, 0 is returned.
  size_t parent_मार्गः_अन्तः(StringRef मार्गः_2, Style style) {
    size_t अन्तः_pos = filename_pos(मार्गः_2, style);

    तार्किक filename_was_sep =
        मार्गः_2.आकार_2() > 0 && is_separator(मार्गः_2[अन्तः_pos], style);

    // Skip separators until we reach root dir (or the start of the string).
    size_t मूलम्_dir_pos = मूलम्_dir_start(मार्गः_2, style);
    यावत् (अन्तः_pos > 0 &&
           (मूलम्_dir_pos == StringRef::npos || अन्तः_pos > मूलम्_dir_pos) &&
           is_separator(मार्गः_2[अन्तः_pos - 1], style))
      --अन्तः_pos;

    यदि (अन्तः_pos == मूलम्_dir_pos && !filename_was_sep) {
      // We've reached the root dir and the input path was *not* ending in a
      // sequence of slashes. Include the root dir in the parent path.
      प्रत्यागम मूलम्_dir_pos + 1;
    }

    // Otherwise, just include before the last slash.
    प्रत्यागम अन्तः_pos;
  }
} // end unnamed namespace

गणना FSEntity {
  FS_Dir,
  FS_File,
  FS_Name
};

स्थिर std::error_संहिता
createUniqueEntity(ध्रुव Twine &Model, पूर्णाङ्क &परिणाम_FD,
                   Small_सदिशम्_Impl<वर्ण> &परिणाम_मार्गः, तार्किक MakeAbsolute,
                   FSEntity Type, sys::fs::OpenFlags Flags = sys::fs::सम्बन्धि_None,
                   अचिह्नित बहुलकम् = 0) {

  // Limit the number of attempts we make, so that we don't infinite loop. E.g.
  // "permission denied" could be for a specific file (so we retry with a
  // different name) or for the whole directory (retry would always fail).
  // Checking which is racy, so we try a number of times, then give up.
  std::error_संहिता EC;
  प्रति (पूर्णाङ्क Retries = 128; Retries > 0; --Retries) {
    sys::fs::create_Unique_मार्गः(Model, परिणाम_मार्गः, MakeAbsolute);
    // Try to open + create the file.
    विकल्प (Type) {
    पक्ष FS_File: {
      EC = sys::fs::openFileForReadWrite(Twine(परिणाम_मार्गः.begin()), परिणाम_FD,
                                         sys::fs::CD_CreateNew, Flags, बहुलकम्);
      यदि (EC) {
        // errc::permission_denied happens on Windows when we try to open a file
        // that has been marked for deletion.
        यदि (EC == errc::file_exists || EC == errc::permission_denied)
          अनुवर्त;
        प्रत्यागम EC;
      }

      प्रत्यागम std::error_संहिता();
    }

    पक्ष FS_Name: {
      EC = sys::fs::access(परिणाम_मार्गः.begin(), sys::fs::Access_बहुलकम्::Exist);
      यदि (EC == errc::no_such_file_or_directory)
        प्रत्यागम std::error_संहिता();
      यदि (EC)
        प्रत्यागम EC;
      अनुवर्त;
    }

    पक्ष FS_Dir: {
      EC = sys::fs::create_directory(परिणाम_मार्गः.begin(), असत्य);
      यदि (EC) {
        यदि (EC == errc::file_exists)
          अनुवर्त;
        प्रत्यागम EC;
      }
      प्रत्यागम std::error_संहिता();
    }
    }
    llvm_unreachable("Invalid Type");
  }
  प्रत्यागम EC;
}

नामाकाश llvm {
नामाकाश sys  {
नामाकाश मार्गः_2 {

const_iterator begin(StringRef मार्गः_2, Style style) {
  const_iterator i;
  i.मार्गः      = मार्गः_2;
  i.Component = find_first_component(मार्गः_2, style);
  i.Position  = 0;
  i.S = style;
  प्रत्यागम i;
}

const_iterator अन्तः(StringRef मार्गः_2) {
  const_iterator i;
  i.मार्गः      = मार्गः_2;
  i.Position  = मार्गः_2.आकार_2();
  प्रत्यागम i;
}

const_iterator &const_iterator::संक्रियकः++() {
  प्रतिज्ञा(Position < मार्गः.आकार_2() && "Tried to increment past end!");

  // Increment Position to past the current component
  Position += Component.आकार_2();

  // Check for end.
  यदि (Position == मार्गः.आकार_2()) {
    Component = StringRef();
    प्रत्यागम *एतत्;
  }

  // Both POSIX and Windows treat paths that begin with exactly two separators
  // specially.
  तार्किक was_net = Component.आकार_2() > 2 && is_separator(Component[0], S) &&
                 Component[1] == Component[0] && !is_separator(Component[2], S);

  // Handle separators.
  यदि (is_separator(मार्गः[Position], S)) {
    // Root dir.
    यदि (was_net ||
        // c:/
        (is_style_windows(S) && Component.अन्ताः_with(":"))) {
      Component = मार्गः.substr(Position, 1);
      प्रत्यागम *एतत्;
    }

    // Skip extra separators.
    यावत् (Position != मार्गः.आकार_2() && is_separator(मार्गः[Position], S)) {
      ++Position;
    }

    // Treat trailing '/' as a '.', unless it is the root dir.
    यदि (Position == मार्गः.आकार_2() && Component != "/") {
      --Position;
      Component = ".";
      प्रत्यागम *एतत्;
    }
  }

  // Find next component.
  size_t अन्तः_pos = मार्गः.find_first_सम्बन्धि(separators(S), Position);
  Component = मार्गः.खण्डिका(Position, अन्तः_pos);

  प्रत्यागम *एतत्;
}

तार्किक const_iterator::संक्रियकः==(ध्रुव const_iterator &RHS) ध्रुव {
  प्रत्यागम मार्गः.begin() == RHS.मार्गः.begin() && Position == RHS.Position;
}

ptrdiff_t const_iterator::संक्रियकः-(ध्रुव const_iterator &RHS) ध्रुव {
  प्रत्यागम Position - RHS.Position;
}

reverse_iterator rbegin(StringRef मार्गः, Style style) {
  reverse_iterator I;
  I.मार्गः = मार्गः;
  I.Position = मार्गः.आकार_2();
  I.S = style;
  ++I;
  प्रत्यागम I;
}

reverse_iterator rend(StringRef मार्गः) {
  reverse_iterator I;
  I.मार्गः = मार्गः;
  I.Component = मार्गः.substr(0, 0);
  I.Position = 0;
  प्रत्यागम I;
}

reverse_iterator &reverse_iterator::संक्रियकः++() {
  size_t मूलम्_dir_pos = मूलम्_dir_start(मार्गः, S);

  // Skip separators unless it's the root directory.
  size_t अन्तः_pos = Position;
  यावत् (अन्तः_pos > 0 && (अन्तः_pos - 1) != मूलम्_dir_pos &&
         is_separator(मार्गः[अन्तः_pos - 1], S))
    --अन्तः_pos;

  // Treat trailing '/' as a '.', unless it is the root dir.
  यदि (Position == मार्गः.आकार_2() && !मार्गः.रिक्तानि() &&
      is_separator(मार्गः.पृष्ठभागः(), S) &&
      (मूलम्_dir_pos == StringRef::npos || अन्तः_pos - 1 > मूलम्_dir_pos)) {
    --Position;
    Component = ".";
    प्रत्यागम *एतत्;
  }

  // Find next separator.
  size_t start_pos = filename_pos(मार्गः.substr(0, अन्तः_pos), S);
  Component = मार्गः.खण्डिका(start_pos, अन्तः_pos);
  Position = start_pos;
  प्रत्यागम *एतत्;
}

तार्किक reverse_iterator::संक्रियकः==(ध्रुव reverse_iterator &RHS) ध्रुव {
  प्रत्यागम मार्गः.begin() == RHS.मार्गः.begin() && Component == RHS.Component &&
         Position == RHS.Position;
}

ptrdiff_t reverse_iterator::संक्रियकः-(ध्रुव reverse_iterator &RHS) ध्रुव {
  प्रत्यागम Position - RHS.Position;
}

StringRef मूलम्_मार्गः_2(StringRef मार्गः_2, Style style) {
  const_iterator b = begin(मार्गः_2, style), pos = b, e = अन्तः(मार्गः_2);
  यदि (b != e) {
    तार्किक has_net =
        b->आकार_2() > 2 && is_separator((*b)[0], style) && (*b)[1] == (*b)[0];
    तार्किक has_drive = is_style_windows(style) && b->अन्ताः_with(":");

    यदि (has_net || has_drive) {
      यदि ((++pos != e) && is_separator((*pos)[0], style)) {
        // {C:/,//net/}, so get the first two components.
        प्रत्यागम मार्गः_2.substr(0, b->आकार_2() + pos->आकार_2());
      }
      // just {C:,//net}, return the first component.
      प्रत्यागम *b;
    }

    // POSIX style root directory.
    यदि (is_separator((*b)[0], style)) {
      प्रत्यागम *b;
    }
  }

  प्रत्यागम StringRef();
}

StringRef मूलम्_name(StringRef मार्गः_2, Style style) {
  const_iterator b = begin(मार्गः_2, style), e = अन्तः(मार्गः_2);
  यदि (b != e) {
    तार्किक has_net =
        b->आकार_2() > 2 && is_separator((*b)[0], style) && (*b)[1] == (*b)[0];
    तार्किक has_drive = is_style_windows(style) && b->अन्ताः_with(":");

    यदि (has_net || has_drive) {
      // just {C:,//net}, return the first component.
      प्रत्यागम *b;
    }
  }

  // No path or no name.
  प्रत्यागम StringRef();
}

StringRef मूलम्_directory(StringRef मार्गः_2, Style style) {
  const_iterator b = begin(मार्गः_2, style), pos = b, e = अन्तः(मार्गः_2);
  यदि (b != e) {
    तार्किक has_net =
        b->आकार_2() > 2 && is_separator((*b)[0], style) && (*b)[1] == (*b)[0];
    तार्किक has_drive = is_style_windows(style) && b->अन्ताः_with(":");

    यदि ((has_net || has_drive) &&
        // {C:,//net}, skip to the next component.
        (++pos != e) && is_separator((*pos)[0], style)) {
      प्रत्यागम *pos;
    }

    // POSIX style root directory.
    यदि (!has_net && is_separator((*b)[0], style)) {
      प्रत्यागम *b;
    }
  }

  // No path or no root.
  प्रत्यागम StringRef();
}

StringRef relative_मार्गः(StringRef मार्गः_2, Style style) {
  StringRef मूलम्_3 = मूलम्_मार्गः_2(मार्गः_2, style);
  प्रत्यागम मार्गः_2.substr(मूलम्_3.आकार_2());
}

शून्य append(Small_सदिशम्_Impl<वर्ण> &मार्गः_2, Style style, ध्रुव Twine &a,
            ध्रुव Twine &b, ध्रुव Twine &c, ध्रुव Twine &d) {
  SmallString<32> a_storage;
  SmallString<32> b_storage;
  SmallString<32> c_storage;
  SmallString<32> d_storage;

  Small_सदिशम्<StringRef, 4> components;
  यदि (!a.is_Trivially_रिक्तानि()) components.प्रक्षिप_पृष्ठभागः(a.गन्तव्यम्_String_Ref(a_storage));
  यदि (!b.is_Trivially_रिक्तानि()) components.प्रक्षिप_पृष्ठभागः(b.गन्तव्यम्_String_Ref(b_storage));
  यदि (!c.is_Trivially_रिक्तानि()) components.प्रक्षिप_पृष्ठभागः(c.गन्तव्यम्_String_Ref(c_storage));
  यदि (!d.is_Trivially_रिक्तानि()) components.प्रक्षिप_पृष्ठभागः(d.गन्तव्यम्_String_Ref(d_storage));

  प्रति (स्वचालित &component : components) {
    तार्किक मार्गः_has_sep =
        !मार्गः_2.रिक्तानि() && is_separator(मार्गः_2[मार्गः_2.आकार_2() - 1], style);
    यदि (मार्गः_has_sep) {
      // Strip separators from beginning of component.
      size_t loc = component.find_first_not_सम्बन्धि(separators(style));
      StringRef c = component.substr(loc);

      // Append it.
      मार्गः_2.append(c.begin(), c.अन्तः());
      अनुवर्त;
    }

    तार्किक component_has_sep =
        !component.रिक्तानि() && is_separator(component[0], style);
    यदि (!component_has_sep &&
        !(मार्गः_2.रिक्तानि() || has_मूलम्_name(component, style))) {
      // Add a separator.
      मार्गः_2.प्रक्षिप_पृष्ठभागः(preferred_separator(style));
    }

    मार्गः_2.append(component.begin(), component.अन्तः());
  }
}

शून्य append(Small_सदिशम्_Impl<वर्ण> &मार्गः_2, ध्रुव Twine &a, ध्रुव Twine &b,
            ध्रुव Twine &c, ध्रुव Twine &d) {
  append(मार्गः_2, Style::native, a, b, c, d);
}

शून्य append(Small_सदिशम्_Impl<वर्ण> &मार्गः_2, const_iterator begin,
            const_iterator अन्तः, Style style) {
  प्रति (; begin != अन्तः; ++begin)
    मार्गः_2::append(मार्गः_2, style, *begin);
}

StringRef parent_मार्गः(StringRef मार्गः_2, Style style) {
  size_t अन्तः_pos = parent_मार्गः_अन्तः(मार्गः_2, style);
  यदि (अन्तः_pos == StringRef::npos)
    प्रत्यागम StringRef();
  प्रत्यागम मार्गः_2.substr(0, अन्तः_pos);
}

शून्य remove_filename(Small_सदिशम्_Impl<वर्ण> &मार्गः_2, Style style) {
  size_t अन्तः_pos = parent_मार्गः_अन्तः(StringRef(मार्गः_2.begin(), मार्गः_2.आकार_2()), style);
  यदि (अन्तः_pos != StringRef::npos)
    मार्गः_2.truncate(अन्तः_pos);
}

शून्य replace_extension(Small_सदिशम्_Impl<वर्ण> &मार्गः_2, ध्रुव Twine &extension,
                       Style style) {
  StringRef p(मार्गः_2.begin(), मार्गः_2.आकार_2());
  SmallString<32> ext_storage;
  StringRef ext = extension.गन्तव्यम्_String_Ref(ext_storage);

  // Erase existing extension.
  size_t pos = p.find_चरमः_सम्बन्धि('.');
  यदि (pos != StringRef::npos && pos >= filename_pos(p, style))
    मार्गः_2.truncate(pos);

  // Append '.' if needed.
  यदि (ext.आकार_2() > 0 && ext[0] != '.')
    मार्गः_2.प्रक्षिप_पृष्ठभागः('.');

  // Append extension.
  मार्गः_2.append(ext.begin(), ext.अन्तः());
}

स्थिर तार्किक starts_with(StringRef मार्गः, StringRef उपसर्गः,
                        Style style = Style::native) {
  // Windows prefix matching : case and separator insensitive
  यदि (is_style_windows(style)) {
    यदि (मार्गः.आकार_2() < उपसर्गः.आकार_2())
      प्रत्यागम असत्य;
    प्रति (size_t I = 0, E = उपसर्गः.आकार_2(); I != E; ++I) {
      तार्किक Sep_मार्गः = is_separator(मार्गः[I], style);
      तार्किक Sep_उपसर्गः = is_separator(उपसर्गः[I], style);
      यदि (Sep_मार्गः != Sep_उपसर्गः)
        प्रत्यागम असत्य;
      यदि (!Sep_मार्गः && गन्तव्यम्_अधस्तनम्(मार्गः[I]) != गन्तव्यम्_अधस्तनम्(उपसर्गः[I]))
        प्रत्यागम असत्य;
    }
    प्रत्यागम सत्य;
  }
  प्रत्यागम मार्गः.starts_with(उपसर्गः);
}

तार्किक replace_मार्गः_उपसर्गः(Small_सदिशम्_Impl<वर्ण> &मार्गः, StringRef Old_उपसर्गः,
                         StringRef New_उपसर्गः, Style style) {
  यदि (Old_उपसर्गः.रिक्तानि() && New_उपसर्गः.रिक्तानि())
    प्रत्यागम असत्य;

  StringRef Orig_मार्गः(मार्गः.begin(), मार्गः.आकार_2());
  यदि (!starts_with(Orig_मार्गः, Old_उपसर्गः, style))
    प्रत्यागम असत्य;

  // If prefixes have the same size we can simply copy the new one over.
  यदि (Old_उपसर्गः.आकार_2() == New_उपसर्गः.आकार_2()) {
    llvm::copy(New_उपसर्गः, मार्गः.begin());
    प्रत्यागम सत्य;
  }

  StringRef Rel_मार्गः = Orig_मार्गः.substr(Old_उपसर्गः.आकार_2());
  SmallString<256> New_मार्गः;
  (Twine(New_उपसर्गः) + Rel_मार्गः).गन्तव्यम्_सदिशम्(New_मार्गः);
  मार्गः.swap(New_मार्गः);
  प्रत्यागम सत्य;
}

शून्य native(ध्रुव Twine &मार्गः_2, Small_सदिशम्_Impl<वर्ण> &परिणाम_2, Style style) {
  प्रतिज्ञा((!मार्गः_2.isSingleStringRef() ||
          मार्गः_2.getSingleStringRef().दत्त() != परिणाम_2.दत्त()) &&
         "path and result are not allowed to overlap!");
  // Clear result.
  परिणाम_2.clear();
  मार्गः_2.गन्तव्यम्_सदिशम्(परिणाम_2);
  native(परिणाम_2, style);
}

std::string native(ध्रुव Twine &मार्गः_2, Style style) {
  SmallString<128> परिणाम;
  native(मार्गः_2, परिणाम, style);
  प्रत्यागम std::string(परिणाम);
}

शून्य native(Small_सदिशम्_Impl<वर्ण> &मार्गः, Style style) {
  यदि (मार्गः.रिक्तानि())
    प्रत्यागम;
  यदि (is_style_windows(style)) {
    प्रति (वर्ण &Ch : मार्गः)
      यदि (is_separator(Ch, style))
        Ch = preferred_separator(style);
    यदि (मार्गः[0] == '~' && (मार्गः.आकार_2() == 1 || is_separator(मार्गः[1], style))) {
      SmallString<128> मार्गः_Home;
      home_directory(मार्गः_Home);
      मार्गः_Home.append(मार्गः.begin() + 1, मार्गः.अन्तः());
      मार्गः = std::move(मार्गः_Home);
    }
  } अन्यथा {
    llvm::replace(मार्गः, '\\', '/');
  }
}

std::string convert_गन्तव्यम्_slash(StringRef मार्गः_2, Style style) {
  यदि (is_style_posix(style))
    प्रत्यागम std::string(मार्गः_2);

  std::string s = मार्गः_2.str();
  llvm::replace(s, '\\', '/');
  प्रत्यागम s;
}

StringRef filename(StringRef मार्गः_2, Style style) { प्रत्यागम *rbegin(मार्गः_2, style); }

StringRef stem(StringRef मार्गः_2, Style style) {
  StringRef fname = filename(मार्गः_2, style);
  size_t pos = fname.find_चरमः_सम्बन्धि('.');
  यदि (pos == StringRef::npos)
    प्रत्यागम fname;
  यदि ((fname.आकार_2() == 1 && fname == ".") ||
      (fname.आकार_2() == 2 && fname == ".."))
    प्रत्यागम fname;
  प्रत्यागम fname.substr(0, pos);
}

StringRef extension(StringRef मार्गः_2, Style style) {
  StringRef fname = filename(मार्गः_2, style);
  size_t pos = fname.find_चरमः_सम्बन्धि('.');
  यदि (pos == StringRef::npos)
    प्रत्यागम StringRef();
  यदि ((fname.आकार_2() == 1 && fname == ".") ||
      (fname.आकार_2() == 2 && fname == ".."))
    प्रत्यागम StringRef();
  प्रत्यागम fname.substr(pos);
}

तार्किक is_separator(वर्ण मानम्_2, Style style) {
  यदि (मानम्_2 == '/')
    प्रत्यागम सत्य;
  यदि (is_style_windows(style))
    प्रत्यागम मानम्_2 == '\\';
  प्रत्यागम असत्य;
}

StringRef get_separator(Style style) {
  यदि (real_style(style) == Style::windows)
    प्रत्यागम "\\";
  प्रत्यागम "/";
}

तार्किक has_मूलम्_name(ध्रुव Twine &मार्गः_2, Style style) {
  SmallString<128> मार्गः_storage;
  StringRef p = मार्गः_2.गन्तव्यम्_String_Ref(मार्गः_storage);

  प्रत्यागम !मूलम्_name(p, style).रिक्तानि();
}

तार्किक has_मूलम्_directory(ध्रुव Twine &मार्गः_2, Style style) {
  SmallString<128> मार्गः_storage;
  StringRef p = मार्गः_2.गन्तव्यम्_String_Ref(मार्गः_storage);

  प्रत्यागम !मूलम्_directory(p, style).रिक्तानि();
}

तार्किक has_मूलम्_मार्गः(ध्रुव Twine &मार्गः_2, Style style) {
  SmallString<128> मार्गः_storage;
  StringRef p = मार्गः_2.गन्तव्यम्_String_Ref(मार्गः_storage);

  प्रत्यागम !मूलम्_मार्गः_2(p, style).रिक्तानि();
}

तार्किक has_relative_मार्गः(ध्रुव Twine &मार्गः_2, Style style) {
  SmallString<128> मार्गः_storage;
  StringRef p = मार्गः_2.गन्तव्यम्_String_Ref(मार्गः_storage);

  प्रत्यागम !relative_मार्गः(p, style).रिक्तानि();
}

तार्किक has_filename(ध्रुव Twine &मार्गः_2, Style style) {
  SmallString<128> मार्गः_storage;
  StringRef p = मार्गः_2.गन्तव्यम्_String_Ref(मार्गः_storage);

  प्रत्यागम !filename(p, style).रिक्तानि();
}

तार्किक has_parent_मार्गः(ध्रुव Twine &मार्गः_2, Style style) {
  SmallString<128> मार्गः_storage;
  StringRef p = मार्गः_2.गन्तव्यम्_String_Ref(मार्गः_storage);

  प्रत्यागम !parent_मार्गः(p, style).रिक्तानि();
}

तार्किक has_stem(ध्रुव Twine &मार्गः_2, Style style) {
  SmallString<128> मार्गः_storage;
  StringRef p = मार्गः_2.गन्तव्यम्_String_Ref(मार्गः_storage);

  प्रत्यागम !stem(p, style).रिक्तानि();
}

तार्किक has_extension(ध्रुव Twine &मार्गः_2, Style style) {
  SmallString<128> मार्गः_storage;
  StringRef p = मार्गः_2.गन्तव्यम्_String_Ref(मार्गः_storage);

  प्रत्यागम !extension(p, style).रिक्तानि();
}

तार्किक is_absolute(ध्रुव Twine &मार्गः_2, Style style) {
  SmallString<128> मार्गः_storage;
  StringRef p = मार्गः_2.गन्तव्यम्_String_Ref(मार्गः_storage);

  तार्किक मूलम्_Dir = has_मूलम्_directory(p, style);
  तार्किक मूलम्_Name = is_style_posix(style) || has_मूलम्_name(p, style);

  प्रत्यागम मूलम्_Dir && मूलम्_Name;
}

तार्किक is_absolute_gnu(ध्रुव Twine &मार्गः_2, Style style) {
  SmallString<128> मार्गः_storage;
  StringRef p = मार्गः_2.गन्तव्यम्_String_Ref(मार्गः_storage);

  // Handle '/' which is absolute for both Windows and POSIX systems.
  // Handle '\\' on Windows.
  यदि (!p.रिक्तानि() && is_separator(p.front(), style))
    प्रत्यागम सत्य;

  यदि (is_style_windows(style)) {
    // Handle drive letter pattern (a character followed by ':') on Windows.
    यदि (p.आकार_2() >= 2 && (p[0] && p[1] == ':'))
      प्रत्यागम सत्य;
  }

  प्रत्यागम असत्य;
}

तार्किक is_relative(ध्रुव Twine &मार्गः_2, Style style) {
  प्रत्यागम !is_absolute(मार्गः_2, style);
}

शून्य make_absolute(ध्रुव Twine &current_directory,
                   Small_सदिशम्_Impl<वर्ण> &मार्गः_2) {
  StringRef p(मार्गः_2.दत्त(), मार्गः_2.आकार_2());

  तार्किक मूलम्_Directory = has_मूलम्_directory(p);
  तार्किक मूलम्_Name = has_मूलम्_name(p);

  // Already absolute.
  यदि ((मूलम्_Name || is_style_posix(Style::native)) && मूलम्_Directory)
    प्रत्यागम;

  // All the following conditions will need the current directory.
  SmallString<128> current_dir;
  current_directory.गन्तव्यम्_सदिशम्(current_dir);

  // Relative path. Prepend the current directory.
  यदि (!मूलम्_Name && !मूलम्_Directory) {
    // Append path to the current directory.
    append(current_dir, p);
    // Set path to the result.
    मार्गः_2.swap(current_dir);
    प्रत्यागम;
  }

  यदि (!मूलम्_Name && मूलम्_Directory) {
    StringRef cdrn = मूलम्_name(current_dir);
    SmallString<128> cur_Dir_मूलम्_Name(cdrn.begin(), cdrn.अन्तः());
    append(cur_Dir_मूलम्_Name, p);
    // Set path to the result.
    मार्गः_2.swap(cur_Dir_मूलम्_Name);
    प्रत्यागम;
  }

  यदि (मूलम्_Name && !मूलम्_Directory) {
    StringRef p_मूलम्_Name = मूलम्_name(p);
    StringRef b_मूलम्_Directory = मूलम्_directory(current_dir);
    StringRef b_Relative_मार्गः = relative_मार्गः(current_dir);
    StringRef p_Relative_मार्गः = relative_मार्गः(p);

    SmallString<128> res;
    append(res, p_मूलम्_Name, b_मूलम्_Directory, b_Relative_मार्गः, p_Relative_मार्गः);
    मार्गः_2.swap(res);
    प्रत्यागम;
  }

  llvm_unreachable("All rootName and rootDirectory combinations should have "
                   "occurred above!");
}

StringRef remove_leading_dotslash(StringRef मार्गः, Style style) {
  // Remove leading "./" (or ".//" or "././" etc.)
  यावत् (मार्गः.आकार_2() > 2 && मार्गः[0] == '.' && is_separator(मार्गः[1], style)) {
    मार्गः = मार्गः.substr(2);
    यावत् (मार्गः.आकार_2() > 0 && is_separator(मार्गः[0], style))
      मार्गः = मार्गः.substr(1);
  }
  प्रत्यागम मार्गः;
}

तार्किक remove_dots(Small_सदिशम्_Impl<वर्ण> &the_मार्गः, तार्किक remove_dot_dot,
                 Style style) {
  style = real_style(style);
  StringRef remaining(the_मार्गः.दत्त(), the_मार्गः.आकार_2());
  तार्किक needs_change = असत्य;
  Small_सदिशम्<StringRef, 16> components;

  // Consume the root path, if present.
  StringRef मूलम्_3 = मार्गः_2::मूलम्_मार्गः_2(remaining, style);
  तार्किक absolute = !मूलम्_3.रिक्तानि();
  यदि (absolute)
    remaining = remaining.drop_front(मूलम्_3.आकार_2());

  // Loop over path components manually. This makes it easier to detect
  // non-preferred slashes and double separators that must be canonicalized.
  यावत् (!remaining.रिक्तानि()) {
    size_t अग्र_slash = remaining.find_first_सम्बन्धि(separators(style));
    यदि (अग्र_slash == StringRef::npos)
      अग्र_slash = remaining.आकार_2();
    StringRef component = remaining.take_front(अग्र_slash);
    remaining = remaining.drop_front(अग्र_slash);

    // Eat the slash, and check if it is the preferred separator.
    यदि (!remaining.रिक्तानि()) {
      needs_change |= remaining.front() != preferred_separator(style);
      remaining = remaining.drop_front();
      // The path needs to be rewritten if it has a trailing slash.
      // FIXME: This is emergent behavior that could be removed.
      needs_change |= remaining.रिक्तानि();
    }

    // Check for path traversal components or double separators.
    यदि (component.रिक्तानि() || component == ".") {
      needs_change = सत्य;
    } अन्यथा यदि (remove_dot_dot && component == "..") {
      needs_change = सत्य;
      // Do not allow ".." to remove the root component. If this is the
      // beginning of a relative path, keep the ".." component.
      यदि (!components.रिक्तानि() && components.पृष्ठभागः() != "..") {
        components.निष्कासय_पृष्ठभागः();
      } अन्यथा यदि (!absolute) {
        components.प्रक्षिप_पृष्ठभागः(component);
      }
    } अन्यथा {
      components.प्रक्षिप_पृष्ठभागः(component);
    }
  }

  SmallString<256> बफर_2 = मूलम्_3;
  // "root" could be "/", which may need to be translated into "\".
  make_preferred(बफर_2, style);
  needs_change |= मूलम्_3 != बफर_2;

  // Avoid rewriting the path unless we have to.
  यदि (!needs_change)
    प्रत्यागम असत्य;

  यदि (!components.रिक्तानि()) {
    बफर_2 += components[0];
    प्रति (StringRef C : ArrayRef(components).drop_front()) {
      बफर_2 += preferred_separator(style);
      बफर_2 += C;
    }
  }
  the_मार्गः.swap(बफर_2);
  प्रत्यागम सत्य;
}

} // end namespace path

नामाकाश fs {

std::error_संहिता getUniqueID(ध्रुव Twine मार्गः, UniqueID &परिणाम) {
  sandbox::violationIfEnabled();

  file_स्थितिः स्थितिः;
  std::error_संहिता EC = स्थितिः_2(मार्गः, स्थितिः);
  यदि (EC)
    प्रत्यागम EC;
  परिणाम = स्थितिः.getUniqueID();
  प्रत्यागम std::error_संहिता();
}

शून्य create_Unique_मार्गः(ध्रुव Twine &Model, Small_सदिशम्_Impl<वर्ण> &परिणाम_मार्गः,
                      तार्किक MakeAbsolute) {
  SmallString<128> ModelStorage;
  Model.गन्तव्यम्_सदिशम्(ModelStorage);

  प्रतिज्ञा(llvm::is_contained(ModelStorage, '%') &&
         "createUniquePath: Model must contain at least one '%'");

  यदि (MakeAbsolute) {
    // Make model absolute by prepending a temp directory if it's not already.
    यदि (!sys::मार्गः_2::is_absolute(Twine(ModelStorage))) {
      SmallString<128> TDir;
      sys::मार्गः_2::system_अस्थायी_directory(सत्य, TDir);
      sys::मार्गः_2::append(TDir, Twine(ModelStorage));
      ModelStorage.swap(TDir);
    }
  }

  परिणाम_मार्गः = ModelStorage;
  परिणाम_मार्गः.प्रक्षिप_पृष्ठभागः(0);
  परिणाम_मार्गः.निष्कासय_पृष्ठभागः();

  // Replace '%' with random chars.
  प्रति (अचिह्नित i = 0, e = ModelStorage.आकार_2(); i != e; ++i) {
    यदि (ModelStorage[i] == '%')
      परिणाम_मार्गः[i] = "0123456789abcdef"[sys::Process::Get_Random_सङ्ख्या() & 15];
  }
}

std::error_संहिता createUniqueFile(ध्रुव Twine &Model, पूर्णाङ्क &परिणाम_Fd,
                                 Small_सदिशम्_Impl<वर्ण> &परिणाम_मार्गः,
                                 OpenFlags Flags, अचिह्नित बहुलकम्) {
  प्रत्यागम createUniqueEntity(Model, परिणाम_Fd, परिणाम_मार्गः, असत्य, FS_File, Flags,
                            बहुलकम्);
}

std::error_संहिता createUniqueFile(ध्रुव Twine &Model,
                                 Small_सदिशम्_Impl<वर्ण> &परिणाम_मार्गः,
                                 अचिह्नित बहुलकम्) {
  पूर्णाङ्क FD;
  स्वचालित EC = createUniqueFile(Model, FD, परिणाम_मार्गः, सम्बन्धि_None, बहुलकम्);
  यदि (EC)
    प्रत्यागम EC;
  // FD is only needed to avoid race conditions. Close it right away.
  close(FD);
  प्रत्यागम EC;
}

स्थिर std::error_संहिता
createTemporaryFile(ध्रुव Twine &Model, पूर्णाङ्क &परिणाम_FD,
                    llvm::Small_सदिशम्_Impl<वर्ण> &परिणाम_मार्गः, FSEntity Type,
                    sys::fs::OpenFlags Flags = sys::fs::सम्बन्धि_None) {
  // Any *temporary* file is assumed to be a compiler-internal output, not
  // a formal one.
  स्वचालित BypassSandbox = sys::sandbox::scopedDisable();

  SmallString<128> Storage;
  StringRef P = Model.गन्तव्यम्_Null_Terminated_String_Ref(Storage);
  प्रतिज्ञा(P.find_first_सम्बन्धि(separators(Style::native)) == StringRef::npos &&
         "Model must be a simple filename.");
  // Use P.begin() so that createUniqueEntity doesn't need to recreate Storage.
  प्रत्यागम createUniqueEntity(P.begin(), परिणाम_FD, परिणाम_मार्गः, सत्य, Type, Flags,
                            all_read | all_write);
}

स्थिर std::error_संहिता
createTemporaryFile(ध्रुव Twine &उपसर्गः, StringRef Suffix, पूर्णाङ्क &परिणाम_FD,
                    llvm::Small_सदिशम्_Impl<वर्ण> &परिणाम_मार्गः, FSEntity Type,
                    sys::fs::OpenFlags Flags = sys::fs::सम्बन्धि_None) {
  ध्रुव वर्ण *मध्यः = Suffix.रिक्तानि() ? "-%%%%%%" : "-%%%%%%.";
  प्रत्यागम createTemporaryFile(उपसर्गः + मध्यः + Suffix, परिणाम_FD, परिणाम_मार्गः,
                             Type, Flags);
}

std::error_संहिता createTemporaryFile(ध्रुव Twine &उपसर्गः, StringRef Suffix,
                                    पूर्णाङ्क &परिणाम_FD,
                                    Small_सदिशम्_Impl<वर्ण> &परिणाम_मार्गः,
                                    sys::fs::OpenFlags Flags) {
  प्रत्यागम createTemporaryFile(उपसर्गः, Suffix, परिणाम_FD, परिणाम_मार्गः, FS_File,
                             Flags);
}

std::error_संहिता createTemporaryFile(ध्रुव Twine &उपसर्गः, StringRef Suffix,
                                    Small_सदिशम्_Impl<वर्ण> &परिणाम_मार्गः,
                                    sys::fs::OpenFlags Flags) {
  पूर्णाङ्क FD;
  स्वचालित EC = createTemporaryFile(उपसर्गः, Suffix, FD, परिणाम_मार्गः, Flags);
  यदि (EC)
    प्रत्यागम EC;
  // FD is only needed to avoid race conditions. Close it right away.
  close(FD);
  प्रत्यागम EC;
}

// This is a mkdtemp with a different pattern. We use createUniqueEntity mostly
// for consistency. We should try using mkdtemp.
std::error_संहिता createUniqueDirectory(ध्रुव Twine &उपसर्गः,
                                      Small_सदिशम्_Impl<वर्ण> &परिणाम_मार्गः) {
  पूर्णाङ्क Dummy;
  प्रत्यागम createUniqueEntity(उपसर्गः + "-%%%%%%", Dummy, परिणाम_मार्गः, सत्य,
                            FS_Dir);
}

std::error_संहिता
getPotentiallyUniqueFileName(ध्रुव Twine &Model,
                             Small_सदिशम्_Impl<वर्ण> &परिणाम_मार्गः) {
  पूर्णाङ्क Dummy;
  प्रत्यागम createUniqueEntity(Model, Dummy, परिणाम_मार्गः, असत्य, FS_Name);
}

std::error_संहिता
get_Potentially_Unique_अस्थायी_File_Name(ध्रुव Twine &उपसर्गः, StringRef Suffix,
                                 Small_सदिशम्_Impl<वर्ण> &परिणाम_मार्गः) {
  पूर्णाङ्क Dummy;
  प्रत्यागम createTemporaryFile(उपसर्गः, Suffix, Dummy, परिणाम_मार्गः, FS_Name);
}

std::error_संहिता make_absolute(Small_सदिशम्_Impl<वर्ण> &मार्गः_2) {
  sandbox::violationIfEnabled();

  यदि (मार्गः_2::is_absolute(मार्गः_2))
    प्रत्यागम {};

  SmallString<128> current_dir;
  यदि (std::error_संहिता ec = current_मार्गः(current_dir))
    प्रत्यागम ec;

  मार्गः_2::make_absolute(current_dir, मार्गः_2);
  प्रत्यागम {};
}

std::error_संहिता create_directories(ध्रुव Twine &मार्गः, तार्किक IgnoreExisting,
                                   perms Perms) {
  SmallString<128> मार्गः_Storage;
  StringRef P = मार्गः.गन्तव्यम्_String_Ref(मार्गः_Storage);

  // Be optimistic and try to create the directory
  std::error_संहिता EC = create_directory(P, IgnoreExisting, Perms);
  // If we succeeded, or had any error other than the parent not existing, just
  // return it.
  यदि (EC != errc::no_such_file_or_directory)
    प्रत्यागम EC;

  // We failed because of a no_such_file_or_directory, try to create the
  // parent.
  StringRef Parent = मार्गः_2::parent_मार्गः(P);
  यदि (Parent.रिक्तानि())
    प्रत्यागम EC;

  यदि ((EC = create_directories(Parent, IgnoreExisting, Perms)))
      प्रत्यागम EC;

  प्रत्यागम create_directory(P, IgnoreExisting, Perms);
}

स्थिर std::error_संहिता copy_file_आन्तरिकम्(पूर्णाङ्क ReadFD, पूर्णाङ्क WriteFD) {
  ध्रुव size_t Buf_आकार = 4096;
  वर्ण *Buf = नूतन वर्ण[Buf_आकार];
  पूर्णाङ्क अष्टकानि_Read = 0, अष्टकानि_Written = 0;
  प्रति (;;) {
    अष्टकानि_Read = पठ(ReadFD, Buf, Buf_आकार);
    यदि (अष्टकानि_Read <= 0)
      भङ्ग;
    यावत् (अष्टकानि_Read) {
      अष्टकानि_Written = लिख(WriteFD, Buf, अष्टकानि_Read);
      यदि (अष्टकानि_Written < 0)
        भङ्ग;
      अष्टकानि_Read -= अष्टकानि_Written;
    }
    यदि (अष्टकानि_Written < 0)
      भङ्ग;
  }
  विलोपय[] Buf;

  यदि (अष्टकानि_Read < 0 || अष्टकानि_Written < 0)
    प्रत्यागम errno_As_Error_संहिता();
  प्रत्यागम std::error_संहिता();
}

#यदि_अपरिभाषितम् __APPLE__
std::error_संहिता copy_file(ध्रुव Twine &From, ध्रुव Twine &गन्तव्यम्) {
  पूर्णाङ्क ReadFD, WriteFD;
  यदि (std::error_संहिता EC = openFileForRead(From, ReadFD, सम्बन्धि_None))
    प्रत्यागम EC;
  यदि (std::error_संहिता EC =
          openFileForWrite(गन्तव्यम्, WriteFD, CD_CreateAlways, सम्बन्धि_None)) {
    close(ReadFD);
    प्रत्यागम EC;
  }

  std::error_संहिता EC = copy_file_आन्तरिकम्(ReadFD, WriteFD);

  close(ReadFD);
  close(WriteFD);

  प्रत्यागम EC;
}
#यदि_अन्तः

std::error_संहिता copy_file(ध्रुव Twine &From, पूर्णाङ्क गन्तव्यम्_FD) {
  पूर्णाङ्क ReadFD;
  यदि (std::error_संहिता EC = openFileForRead(From, ReadFD, सम्बन्धि_None))
    प्रत्यागम EC;

  std::error_संहिता EC = copy_file_आन्तरिकम्(ReadFD, गन्तव्यम्_FD);

  close(ReadFD);

  प्रत्यागम EC;
}

ErrorOr<MD5::MD5_परिणाम> md5_contents(पूर्णाङ्क FD) {
  sandbox::violationIfEnabled();

  MD5 Hash;

  constexpr size_t Buf_आकार = 4096;
  std::सदिशम्<uint8_t> Buf(Buf_आकार);
  पूर्णाङ्क अष्टकानि_Read = 0;
  प्रति (;;) {
    अष्टकानि_Read = पठ(FD, Buf.दत्त(), Buf_आकार);
    यदि (अष्टकानि_Read <= 0)
      भङ्ग;
    Hash.update(ArrayRef(Buf.दत्त(), अष्टकानि_Read));
  }

  यदि (अष्टकानि_Read < 0)
    प्रत्यागम errno_As_Error_संहिता();
  MD5::MD5_परिणाम परिणाम;
  Hash.final(परिणाम);
  प्रत्यागम परिणाम;
}

ErrorOr<MD5::MD5_परिणाम> md5_contents(ध्रुव Twine &मार्गः) {
  sandbox::violationIfEnabled();

  पूर्णाङ्क FD;
  यदि (स्वचालित EC = openFileForRead(मार्गः, FD, सम्बन्धि_None))
    प्रत्यागम EC;

  स्वचालित परिणाम = md5_contents(FD);
  close(FD);
  प्रत्यागम परिणाम;
}

तार्किक exists(ध्रुव basic_file_स्थितिः &स्थितिः_2) {
  प्रत्यागम स्थितिः_ज्ञातम्(स्थितिः_2) && स्थितिः_2.type() != file_type::file_not_प्राप्तम्;
}

तार्किक स्थितिः_ज्ञातम्(ध्रुव basic_file_स्थितिः &s) {
  प्रत्यागम s.type() != file_type::स्थितिः_error;
}

file_type get_file_type(ध्रुव Twine &मार्गः, तार्किक Follow) {
  file_स्थितिः st;
  यदि (स्थितिः_2(मार्गः, st, Follow))
    प्रत्यागम file_type::स्थितिः_error;
  प्रत्यागम st.type();
}

तार्किक is_directory(ध्रुव basic_file_स्थितिः &स्थितिः_2) {
  प्रत्यागम स्थितिः_2.type() == file_type::directory_file;
}

std::error_संहिता is_directory(ध्रुव Twine &मार्गः_2, तार्किक &परिणाम_2) {
  sandbox::violationIfEnabled();

  file_स्थितिः st;
  यदि (std::error_संहिता ec = स्थितिः_2(मार्गः_2, st))
    प्रत्यागम ec;
  परिणाम_2 = is_directory(st);
  प्रत्यागम std::error_संहिता();
}

तार्किक is_regular_file(ध्रुव basic_file_स्थितिः &स्थितिः_2) {
  प्रत्यागम स्थितिः_2.type() == file_type::regular_file;
}

std::error_संहिता is_regular_file(ध्रुव Twine &मार्गः_2, तार्किक &परिणाम_2) {
  sandbox::violationIfEnabled();

  file_स्थितिः st;
  यदि (std::error_संहिता ec = स्थितिः_2(मार्गः_2, st))
    प्रत्यागम ec;
  परिणाम_2 = is_regular_file(st);
  प्रत्यागम std::error_संहिता();
}

तार्किक is_symlink_file(ध्रुव basic_file_स्थितिः &स्थितिः_2) {
  प्रत्यागम स्थितिः_2.type() == file_type::symlink_file;
}

std::error_संहिता is_symlink_file(ध्रुव Twine &मार्गः_2, तार्किक &परिणाम_2) {
  sandbox::violationIfEnabled();

  file_स्थितिः st;
  यदि (std::error_संहिता ec = स्थितिः_2(मार्गः_2, st, असत्य))
    प्रत्यागम ec;
  परिणाम_2 = is_symlink_file(st);
  प्रत्यागम std::error_संहिता();
}

तार्किक is_other(ध्रुव basic_file_स्थितिः &स्थितिः_2) {
  प्रत्यागम exists(स्थितिः_2) &&
         !is_regular_file(स्थितिः_2) &&
         !is_directory(स्थितिः_2);
}

std::error_संहिता is_other(ध्रुव Twine &मार्गः, तार्किक &परिणाम) {
  sandbox::violationIfEnabled();

  file_स्थितिः File_स्थितिः;
  यदि (std::error_संहिता EC = स्थितिः_2(मार्गः, File_स्थितिः))
    प्रत्यागम EC;
  परिणाम = is_other(File_स्थितिः);
  प्रत्यागम std::error_संहिता();
}

शून्य directory_प्रविष्टिः::replace_filename(ध्रुव Twine &Filename, file_type Type,
                                       basic_file_स्थितिः स्थितिः) {
  SmallString<128> मार्गः_Str = मार्गः_2::parent_मार्गः(मार्गः);
  मार्गः_2::append(मार्गः_Str, Filename);
  एतत्->मार्गः = std::string(मार्गः_Str);
  एतत्->Type = Type;
  एतत्->स्थितिः = स्थितिः;
}

ErrorOr<perms> getPermissions(ध्रुव Twine &मार्गः) {
  sandbox::violationIfEnabled();

  file_स्थितिः स्थितिः;
  यदि (std::error_संहिता EC = स्थितिः_2(मार्गः, स्थितिः))
    प्रत्यागम EC;

  प्रत्यागम स्थितिः.permissions();
}

std::error_संहिता समुच्चयः_चरमः_Access_And_Modification_Time(ध्रुव Twine &मार्गः,
                                                 TimePoint<> AccessTime,
                                                 TimePoint<> ModificationTime) {
  पूर्णाङ्क FD;
  यदि (std::error_संहिता EC = openFile(मार्गः, FD, CD_OpenExisting, FA_Read,
                                    सम्बन्धि_Update_गुणाः | सम्बन्धि_Open_Directory))
    प्रत्यागम EC;
  std::error_संहिता EC =
      समुच्चयः_चरमः_Access_And_Modification_Time(FD, AccessTime, ModificationTime);
  Process::SafelyCloseFileDescriptor(FD);
  प्रत्यागम EC;
}

size_t mapped_file_region::आकार_2() ध्रुव {
  प्रतिज्ञा(Mapping && "Mapping failed but used anyway!");
  प्रत्यागम आकार;
}

वर्ण *mapped_file_region::दत्त() ध्रुव {
  प्रतिज्ञा(Mapping && "Mapping failed but used anyway!");
  प्रत्यागम reinterpret_cast<वर्ण *>(Mapping);
}

ध्रुव वर्ण *mapped_file_region::const_दत्त() ध्रुव {
  प्रतिज्ञा(Mapping && "Mapping failed but used anyway!");
  प्रत्यागम reinterpret_cast<ध्रुव वर्ण *>(Mapping);
}

Error read_Native_File_गन्तव्यम्_EOF(file_t FileHandle, Small_सदिशम्_Impl<वर्ण> &बफर,
                          ssize_t Chunk_आकार) {
  sandbox::violationIfEnabled();

  // Install a handler to truncate the buffer to the correct size on exit.
  size_t आकार = बफर.आकार_2();
  llvm::scope_exit TruncateOnExit([&]() { बफर.truncate(आकार); });

  // Read into Buffer until we hit EOF.
  प्रति (;;) {
    बफर.resize_for_overwrite(आकार + Chunk_आकार);
    Expected<size_t> Read_अष्टकानि = readNativeFile(
        FileHandle, MutableArrayRef(बफर.begin() + आकार, Chunk_आकार));
    यदि (!Read_अष्टकानि)
      प्रत्यागम Read_अष्टकानि.takeError();
    यदि (*Read_अष्टकानि == 0)
      प्रत्यागम Error::success();
    आकार += *Read_अष्टकानि;
  }
}

} // end namespace fs
} // end namespace sys
} // end namespace llvm

// Include the truly platform-specific parts.
#यदि defined(LLVM_ON_UNIX)
#अन्तर्भाव "Unix/Path.inc"
#यदि_अन्तः
#यदि defined(_WIN32)
#अन्तर्भाव "Windows/Path.inc"
#यदि_अन्तः

नामाकाश llvm {
नामाकाश sys {
नामाकाश fs {

अस्थायी_File::अस्थायी_File(StringRef Name, पूर्णाङ्क FD)
    : TmpName(std::string(Name)), FD(FD) {}
अस्थायी_File::अस्थायी_File(अस्थायी_File &&Other) { *एतत् = std::move(Other); }
अस्थायी_File &अस्थायी_File::संक्रियकः=(अस्थायी_File &&Other) {
  TmpName = std::move(Other.TmpName);
  FD = Other.FD;
  Other.समाप्तम् = सत्य;
  Other.FD = -1;
#यदि_परिभाषितम् _WIN32
  RemoveOnClose = Other.RemoveOnClose;
  Other.RemoveOnClose = असत्य;
#यदि_अन्तः
  प्रत्यागम *एतत्;
}

अस्थायी_File::~अस्थायी_File() { प्रतिज्ञा(समाप्तम्); }

Error अस्थायी_File::discard() {
  समाप्तम् = सत्य;
  यदि (FD != -1 && close(FD) == -1) {
    std::error_संहिता EC = errno_As_Error_संहिता();
    प्रत्यागम error_संहिता_गन्तव्यम्_Error(EC);
  }
  FD = -1;

#यदि_परिभाषितम् _WIN32
  // On Windows, closing will remove the file, if we set the delete
  // disposition. If not, remove it manually.
  तार्किक Remove = RemoveOnClose;
#अन्यथा
  // Always try to remove the file.
  तार्किक Remove = सत्य;
#यदि_अन्तः
  std::error_संहिता RemoveEC;
  यदि (Remove && !TmpName.रिक्तानि()) {
    RemoveEC = fs::remove(TmpName);
    sys::DontRemoveFileOnSignal(TmpName);
    यदि (!RemoveEC)
      TmpName = "";
  } अन्यथा {
    TmpName = "";
  }
  प्रत्यागम error_संहिता_गन्तव्यम्_Error(RemoveEC);
}

Error अस्थायी_File::keep(ध्रुव Twine &Name) {
  प्रतिज्ञा(!समाप्तम्);
  समाप्तम् = सत्य;
  // Always try to close and rename.
#यदि_परिभाषितम् _WIN32
  // If we can't cancel the delete don't rename.
  स्वचालित H = reinterpret_cast<HANDLE>(_get_osfhandle(FD));
  std::error_संहिता RenameEC =
      RemoveOnClose ? std::error_संहिता() : समुच्चयः_Delete_Disposition(H, असत्य);
  तार्किक ShouldDelete = असत्य;
  यदि (!RenameEC) {
    RenameEC = rename_handle(H, Name);
    // If rename failed because it's cross-device, copy instead
    यदि (RenameEC ==
      std::error_संहिता(ERROR_NOT_SAME_DEVICE, std::system_category())) {
      RenameEC = copy_file(TmpName, Name);
      ShouldDelete = सत्य;
    }
  }

  // If we can't rename or copy, discard the temporary file.
  यदि (RenameEC)
    ShouldDelete = सत्य;
  यदि (ShouldDelete) {
    यदि (!RemoveOnClose)
      समुच्चयः_Delete_Disposition(H, सत्य);
    अन्यथा
      remove(TmpName);
  }
#अन्यथा
  std::error_संहिता RenameEC = fs::rename(TmpName, Name);
  यदि (RenameEC) {
    // If we can't rename, try to copy to work around cross-device link issues.
    RenameEC = sys::fs::copy_file(TmpName, Name);
    // If we can't rename or copy, discard the temporary file.
    यदि (RenameEC)
      remove(TmpName);
  }
#यदि_अन्तः
  sys::DontRemoveFileOnSignal(TmpName);

  यदि (!RenameEC)
    TmpName = "";

  यदि (close(FD) == -1)
    प्रत्यागम error_संहिता_गन्तव्यम्_Error(errno_As_Error_संहिता());
  FD = -1;

  प्रत्यागम error_संहिता_गन्तव्यम्_Error(RenameEC);
}

Error अस्थायी_File::keep() {
  प्रतिज्ञा(!समाप्तम्);
  समाप्तम् = सत्य;

#यदि_परिभाषितम् _WIN32
  स्वचालित H = reinterpret_cast<HANDLE>(_get_osfhandle(FD));
  यदि (std::error_संहिता EC = समुच्चयः_Delete_Disposition(H, असत्य))
    प्रत्यागम error_संहिता_गन्तव्यम्_Error(EC);
#यदि_अन्तः
  sys::DontRemoveFileOnSignal(TmpName);

  TmpName = "";

  यदि (close(FD) == -1)
    प्रत्यागम error_संहिता_गन्तव्यम्_Error(errno_As_Error_संहिता());
  FD = -1;

  प्रत्यागम Error::success();
}

Expected<अस्थायी_File> अस्थायी_File::create(ध्रुव Twine &Model, अचिह्नित बहुलकम्,
                                    OpenFlags ExtraFlags) {
  पूर्णाङ्क FD;
  SmallString<128> परिणाम_मार्गः;
  यदि (std::error_संहिता EC =
          createUniqueFile(Model, FD, परिणाम_मार्गः, सम्बन्धि_Delete | ExtraFlags, बहुलकम्))
    प्रत्यागम error_संहिता_गन्तव्यम्_Error(EC);

  अस्थायी_File Ret(परिणाम_मार्गः, FD);
#यदि_परिभाषितम् _WIN32
  स्वचालित H = reinterpret_cast<HANDLE>(_get_osfhandle(FD));
  तार्किक समुच्चयः_Signal_Handler = असत्य;
  यदि (std::error_संहिता EC = समुच्चयः_Delete_Disposition(H, सत्य)) {
    Ret.RemoveOnClose = सत्य;
    समुच्चयः_Signal_Handler = सत्य;
  }
#अन्यथा
  तार्किक समुच्चयः_Signal_Handler = सत्य;
#यदि_अन्तः
  यदि (समुच्चयः_Signal_Handler && sys::RemoveFileOnSignal(परिणाम_मार्गः)) {
    // Make sure we delete the file when RemoveFileOnSignal fails.
    consumeError(Ret.discard());
    std::error_संहिता EC(errc::operation_not_permitted);
    प्रत्यागम error_संहिता_गन्तव्यम्_Error(EC);
  }
  प्रत्यागम std::move(Ret);
}
} // namespace fs

} // namespace sys
} // namespace llvm
