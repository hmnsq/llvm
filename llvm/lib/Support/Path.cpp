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

#అన్తర్భావ "llvm/Support/Path.h"
#అన్తర్భావ "llvm/ADT/ArrayRef.h"
#అన్తర్భావ "llvm/ADT/ScopeExit.h"
#అన్తర్భావ "llvm/ADT/StringExtras.h"
#అన్తర్భావ "llvm/Config/config.h"
#అన్తర్భావ "llvm/Config/llvm-config.h"
#అన్తర్భావ "llvm/Support/Errc.h"
#అన్తర్భావ "llvm/Support/ErrorHandling.h"
#అన్తర్భావ "llvm/Support/FileSystem.h"
#అన్తర్భావ "llvm/Support/IOSandbox.h"
#అన్తర్భావ "llvm/Support/Process.h"
#అన్తర్భావ "llvm/Support/Signals.h"
#అన్తర్భావ <cctype>

#ఒకవేళ !defined(_MSC_VER) && !defined(__MINGW32__)
#అన్తర్భావ <unistd.h>
#లేకపోతే
#అన్తర్భావ <io.h>
#అయితే_ముగింపు

ఉపయోగించి నామాకాశ llvm;
ఉపయోగించి నామాకాశ llvm::support::endian;

నామాకాశ {
  ఉపయోగించి llvm::StringRef;
  ఉపయోగించి llvm::sys::మార్గం_2::is_separator;
  ఉపయోగించి llvm::sys::మార్గం_2::Style;

  అంతఃవరుస తార్కిక prefer_forward_slash() {
    స్థిర తార్కిక prefer = []() {
      ఒకవేళ (std::optional<std::string> Env =
              sys::Process::GetEnv("LLVM_WINDOWS_PREFER_FORWARD_SLASH"))
        తిరిగి *Env == "1";
      తిరిగి static_cast<తార్కిక>(LLVM_WINDOWS_PREFER_FORWARD_SLASH);
    }();
    తిరిగి prefer;
  }

  అంతఃవరుస Style real_style(Style style) {
    ఒకవేళ (style != Style::native)
      తిరిగి style;
    ఒకవేళ (is_style_posix(style))
      తిరిగి Style::posix;
    తిరిగి prefer_forward_slash() ? Style::windows_slash
                                  : Style::windows_backslash;
  }

  అంతఃవరుస స్థిరరాశి అక్షరం *separators(Style style) {
    ఒకవేళ (is_style_windows(style))
      తిరిగి "\\/";
    తిరిగి "/";
  }

  అంతఃవరుస అక్షరం preferred_separator(Style style) {
    ఒకవేళ (real_style(style) == Style::windows)
      తిరిగి '\\';
    తిరిగి '/';
  }

  StringRef కనుగొను_మొదటి_component(StringRef మార్గం_2, Style style) {
    // Look for this first component in the following order.
    // * empty (in this case we return an empty string)
    // * either C: or {//,\\}net.
    // * {/,\}
    // * {file,directory}name

    ఒకవేళ (మార్గం_2.ఖాళీలు())
      తిరిగి మార్గం_2;

    ఒకవేళ (is_style_windows(style)) {
      // C:
      ఒకవేళ (మార్గం_2.కొలత_2() >= 2 &&
          std::అక్షరమా(static_cast<గుర్తులేని అక్షరం>(మార్గం_2[0])) && మార్గం_2[1] == ':')
        తిరిగి మార్గం_2.substr(0, 2);
    }

    // //net
    ఒకవేళ ((మార్గం_2.కొలత_2() > 2) && is_separator(మార్గం_2[0], style) &&
        మార్గం_2[0] == మార్గం_2[1] && !is_separator(మార్గం_2[2], style)) {
      // Find the next directory separator.
      size_t చివర = మార్గం_2.కనుగొను_మొదటి_యొక్క(separators(style), 2);
      తిరిగి మార్గం_2.substr(0, చివర);
    }

    // {/,\}
    ఒకవేళ (is_separator(మార్గం_2[0], style))
      తిరిగి మార్గం_2.substr(0, 1);

    // * {file,directory}name
    size_t చివర = మార్గం_2.కనుగొను_మొదటి_యొక్క(separators(style));
    తిరిగి మార్గం_2.substr(0, చివర);
  }

  // Returns the first character of the filename in str. For paths ending in
  // '/', it returns the position of the '/'.
  size_t filename_pos(StringRef str, Style style) {
    ఒకవేళ (str.కొలత_2() > 0 && is_separator(str[str.కొలత_2() - 1], style))
      తిరిగి str.కొలత_2() - 1;

    size_t pos = str.కనుగొను_చివరి_యొక్క(separators(style), str.కొలత_2() - 1);

    ఒకవేళ (is_style_windows(style)) {
      ఒకవేళ (pos == StringRef::npos)
        pos = str.కనుగొను_చివరి_యొక్క(':', str.కొలత_2() - 1);
    }

    ఒకవేళ (pos == StringRef::npos || (pos == 1 && is_separator(str[0], style)))
      తిరిగి 0;

    తిరిగి pos + 1;
  }

  // Returns the position of the root directory in str. If there is no root
  // directory in str, it returns StringRef::npos.
  size_t మూలం_dir_ప్రారంభం(StringRef str, Style style) {
    // case "c:/"
    ఒకవేళ (is_style_windows(style)) {
      ఒకవేళ (str.కొలత_2() > 2 && str[1] == ':' && is_separator(str[2], style))
        తిరిగి 2;
    }

    // case "//net"
    ఒకవేళ (str.కొలత_2() > 3 && is_separator(str[0], style) && str[0] == str[1] &&
        !is_separator(str[2], style)) {
      తిరిగి str.కనుగొను_మొదటి_యొక్క(separators(style), 2);
    }

    // case "/"
    ఒకవేళ (str.కొలత_2() > 0 && is_separator(str[0], style))
      తిరిగి 0;

    తిరిగి StringRef::npos;
  }

  // Returns the position past the end of the "parent path" of path. The parent
  // path will not end in '/', unless the parent is the root directory. If the
  // path has no parent, 0 is returned.
  size_t మాతృ_మార్గం_చివర(StringRef మార్గం_2, Style style) {
    size_t చివర_pos = filename_pos(మార్గం_2, style);

    తార్కిక filename_was_sep =
        మార్గం_2.కొలత_2() > 0 && is_separator(మార్గం_2[చివర_pos], style);

    // Skip separators until we reach root dir (or the start of the string).
    size_t మూలం_dir_pos = మూలం_dir_ప్రారంభం(మార్గం_2, style);
    అయితే (చివర_pos > 0 &&
           (మూలం_dir_pos == StringRef::npos || చివర_pos > మూలం_dir_pos) &&
           is_separator(మార్గం_2[చివర_pos - 1], style))
      --చివర_pos;

    ఒకవేళ (చివర_pos == మూలం_dir_pos && !filename_was_sep) {
      // We've reached the root dir and the input path was *not* ending in a
      // sequence of slashes. Include the root dir in the parent path.
      తిరిగి మూలం_dir_pos + 1;
    }

    // Otherwise, just include before the last slash.
    తిరిగి చివర_pos;
  }
} // end unnamed namespace

గణన FSEntity {
  FS_Dir,
  FS_File,
  FS_Name
};

స్థిర std::error_సంకేతం
createUniqueEntity(స్థిరరాశి Twine &Model, పూర్ణాంకం &ఫలితం_FD,
                   Small_సదిశ_Impl<అక్షరం> &ఫలితం_మార్గం, తార్కిక MakeAbsolute,
                   FSEntity Type, sys::fs::OpenFlags Flags = sys::fs::యొక్క_None,
                   గుర్తులేని బహుళకం = 0) {

  // Limit the number of attempts we make, so that we don't infinite loop. E.g.
  // "permission denied" could be for a specific file (so we retry with a
  // different name) or for the whole directory (retry would always fail).
  // Checking which is racy, so we try a number of times, then give up.
  std::error_సంకేతం EC;
  కొరకు (పూర్ణాంకం Retries = 128; Retries > 0; --Retries) {
    sys::fs::create_Unique_మార్గం(Model, ఫలితం_మార్గం, MakeAbsolute);
    // Try to open + create the file.
    ఎంపిక (Type) {
    స్థితి FS_File: {
      EC = sys::fs::openFileForReadWrite(Twine(ఫలితం_మార్గం.begin()), ఫలితం_FD,
                                         sys::fs::CD_CreateNew, Flags, బహుళకం);
      ఒకవేళ (EC) {
        // errc::permission_denied happens on Windows when we try to open a file
        // that has been marked for deletion.
        ఒకవేళ (EC == errc::file_exists || EC == errc::permission_denied)
          కొనసాగు;
        తిరిగి EC;
      }

      తిరిగి std::error_సంకేతం();
    }

    స్థితి FS_Name: {
      EC = sys::fs::access(ఫలితం_మార్గం.begin(), sys::fs::Access_బహుళకం::Exist);
      ఒకవేళ (EC == errc::no_such_file_or_directory)
        తిరిగి std::error_సంకేతం();
      ఒకవేళ (EC)
        తిరిగి EC;
      కొనసాగు;
    }

    స్థితి FS_Dir: {
      EC = sys::fs::create_directory(ఫలితం_మార్గం.begin(), అబద్ధం);
      ఒకవేళ (EC) {
        ఒకవేళ (EC == errc::file_exists)
          కొనసాగు;
        తిరిగి EC;
      }
      తిరిగి std::error_సంకేతం();
    }
    }
    llvm_unreachable("Invalid Type");
  }
  తిరిగి EC;
}

నామాకాశ llvm {
నామాకాశ sys  {
నామాకాశ మార్గం_2 {

const_iterator begin(StringRef మార్గం_2, Style style) {
  const_iterator i;
  i.మార్గం      = మార్గం_2;
  i.Component = కనుగొను_మొదటి_component(మార్గం_2, style);
  i.Position  = 0;
  i.S = style;
  తిరిగి i;
}

const_iterator చివర(StringRef మార్గం_2) {
  const_iterator i;
  i.మార్గం      = మార్గం_2;
  i.Position  = మార్గం_2.కొలత_2();
  తిరిగి i;
}

const_iterator &const_iterator::క్రియాకారకం++() {
  ప్రతిజ్ఞా(Position < మార్గం.కొలత_2() && "Tried to increment past end!");

  // Increment Position to past the current component
  Position += Component.కొలత_2();

  // Check for end.
  ఒకవేళ (Position == మార్గం.కొలత_2()) {
    Component = StringRef();
    తిరిగి *ఇది;
  }

  // Both POSIX and Windows treat paths that begin with exactly two separators
  // specially.
  తార్కిక was_net = Component.కొలత_2() > 2 && is_separator(Component[0], S) &&
                 Component[1] == Component[0] && !is_separator(Component[2], S);

  // Handle separators.
  ఒకవేళ (is_separator(మార్గం[Position], S)) {
    // Root dir.
    ఒకవేళ (was_net ||
        // c:/
        (is_style_windows(S) && Component.చివర్లు_with(":"))) {
      Component = మార్గం.substr(Position, 1);
      తిరిగి *ఇది;
    }

    // Skip extra separators.
    అయితే (Position != మార్గం.కొలత_2() && is_separator(మార్గం[Position], S)) {
      ++Position;
    }

    // Treat trailing '/' as a '.', unless it is the root dir.
    ఒకవేళ (Position == మార్గం.కొలత_2() && Component != "/") {
      --Position;
      Component = ".";
      తిరిగి *ఇది;
    }
  }

  // Find next component.
  size_t చివర_pos = మార్గం.కనుగొను_మొదటి_యొక్క(separators(S), Position);
  Component = మార్గం.ముక్క(Position, చివర_pos);

  తిరిగి *ఇది;
}

తార్కిక const_iterator::క్రియాకారకం==(స్థిరరాశి const_iterator &RHS) స్థిరరాశి {
  తిరిగి మార్గం.begin() == RHS.మార్గం.begin() && Position == RHS.Position;
}

ptrdiff_t const_iterator::క్రియాకారకం-(స్థిరరాశి const_iterator &RHS) స్థిరరాశి {
  తిరిగి Position - RHS.Position;
}

తిరగేయి_iterator rbegin(StringRef మార్గం, Style style) {
  తిరగేయి_iterator I;
  I.మార్గం = మార్గం;
  I.Position = మార్గం.కొలత_2();
  I.S = style;
  ++I;
  తిరిగి I;
}

తిరగేయి_iterator rend(StringRef మార్గం) {
  తిరగేయి_iterator I;
  I.మార్గం = మార్గం;
  I.Component = మార్గం.substr(0, 0);
  I.Position = 0;
  తిరిగి I;
}

తిరగేయి_iterator &తిరగేయి_iterator::క్రియాకారకం++() {
  size_t మూలం_dir_pos = మూలం_dir_ప్రారంభం(మార్గం, S);

  // Skip separators unless it's the root directory.
  size_t చివర_pos = Position;
  అయితే (చివర_pos > 0 && (చివర_pos - 1) != మూలం_dir_pos &&
         is_separator(మార్గం[చివర_pos - 1], S))
    --చివర_pos;

  // Treat trailing '/' as a '.', unless it is the root dir.
  ఒకవేళ (Position == మార్గం.కొలత_2() && !మార్గం.ఖాళీలు() &&
      is_separator(మార్గం.వెనుక(), S) &&
      (మూలం_dir_pos == StringRef::npos || చివర_pos - 1 > మూలం_dir_pos)) {
    --Position;
    Component = ".";
    తిరిగి *ఇది;
  }

  // Find next separator.
  size_t ప్రారంభం_pos = filename_pos(మార్గం.substr(0, చివర_pos), S);
  Component = మార్గం.ముక్క(ప్రారంభం_pos, చివర_pos);
  Position = ప్రారంభం_pos;
  తిరిగి *ఇది;
}

తార్కిక తిరగేయి_iterator::క్రియాకారకం==(స్థిరరాశి తిరగేయి_iterator &RHS) స్థిరరాశి {
  తిరిగి మార్గం.begin() == RHS.మార్గం.begin() && Component == RHS.Component &&
         Position == RHS.Position;
}

ptrdiff_t తిరగేయి_iterator::క్రియాకారకం-(స్థిరరాశి తిరగేయి_iterator &RHS) స్థిరరాశి {
  తిరిగి Position - RHS.Position;
}

StringRef మూలం_మార్గం_2(StringRef మార్గం_2, Style style) {
  const_iterator b = begin(మార్గం_2, style), pos = b, e = చివర(మార్గం_2);
  ఒకవేళ (b != e) {
    తార్కిక has_net =
        b->కొలత_2() > 2 && is_separator((*b)[0], style) && (*b)[1] == (*b)[0];
    తార్కిక has_drive = is_style_windows(style) && b->చివర్లు_with(":");

    ఒకవేళ (has_net || has_drive) {
      ఒకవేళ ((++pos != e) && is_separator((*pos)[0], style)) {
        // {C:/,//net/}, so get the first two components.
        తిరిగి మార్గం_2.substr(0, b->కొలత_2() + pos->కొలత_2());
      }
      // just {C:,//net}, return the first component.
      తిరిగి *b;
    }

    // POSIX style root directory.
    ఒకవేళ (is_separator((*b)[0], style)) {
      తిరిగి *b;
    }
  }

  తిరిగి StringRef();
}

StringRef మూలం_name(StringRef మార్గం_2, Style style) {
  const_iterator b = begin(మార్గం_2, style), e = చివర(మార్గం_2);
  ఒకవేళ (b != e) {
    తార్కిక has_net =
        b->కొలత_2() > 2 && is_separator((*b)[0], style) && (*b)[1] == (*b)[0];
    తార్కిక has_drive = is_style_windows(style) && b->చివర్లు_with(":");

    ఒకవేళ (has_net || has_drive) {
      // just {C:,//net}, return the first component.
      తిరిగి *b;
    }
  }

  // No path or no name.
  తిరిగి StringRef();
}

StringRef మూలం_directory(StringRef మార్గం_2, Style style) {
  const_iterator b = begin(మార్గం_2, style), pos = b, e = చివర(మార్గం_2);
  ఒకవేళ (b != e) {
    తార్కిక has_net =
        b->కొలత_2() > 2 && is_separator((*b)[0], style) && (*b)[1] == (*b)[0];
    తార్కిక has_drive = is_style_windows(style) && b->చివర్లు_with(":");

    ఒకవేళ ((has_net || has_drive) &&
        // {C:,//net}, skip to the next component.
        (++pos != e) && is_separator((*pos)[0], style)) {
      తిరిగి *pos;
    }

    // POSIX style root directory.
    ఒకవేళ (!has_net && is_separator((*b)[0], style)) {
      తిరిగి *b;
    }
  }

  // No path or no root.
  తిరిగి StringRef();
}

StringRef relative_మార్గం(StringRef మార్గం_2, Style style) {
  StringRef మూలం_3 = మూలం_మార్గం_2(మార్గం_2, style);
  తిరిగి మార్గం_2.substr(మూలం_3.కొలత_2());
}

శూన్య append(Small_సదిశ_Impl<అక్షరం> &మార్గం_2, Style style, స్థిరరాశి Twine &a,
            స్థిరరాశి Twine &b, స్థిరరాశి Twine &c, స్థిరరాశి Twine &d) {
  SmallString<32> a_storage;
  SmallString<32> b_storage;
  SmallString<32> c_storage;
  SmallString<32> d_storage;

  Small_సదిశ<StringRef, 4> components;
  ఒకవేళ (!a.is_Trivially_ఖాళీలు()) components.నెట్టు_వెనుక(a.గమ్యం_String_Ref(a_storage));
  ఒకవేళ (!b.is_Trivially_ఖాళీలు()) components.నెట్టు_వెనుక(b.గమ్యం_String_Ref(b_storage));
  ఒకవేళ (!c.is_Trivially_ఖాళీలు()) components.నెట్టు_వెనుక(c.గమ్యం_String_Ref(c_storage));
  ఒకవేళ (!d.is_Trivially_ఖాళీలు()) components.నెట్టు_వెనుక(d.గమ్యం_String_Ref(d_storage));

  కొరకు (స్వయంచాలక &component : components) {
    తార్కిక మార్గం_has_sep =
        !మార్గం_2.ఖాళీలు() && is_separator(మార్గం_2[మార్గం_2.కొలత_2() - 1], style);
    ఒకవేళ (మార్గం_has_sep) {
      // Strip separators from beginning of component.
      size_t loc = component.కనుగొను_మొదటి_not_యొక్క(separators(style));
      StringRef c = component.substr(loc);

      // Append it.
      మార్గం_2.append(c.begin(), c.చివర());
      కొనసాగు;
    }

    తార్కిక component_has_sep =
        !component.ఖాళీలు() && is_separator(component[0], style);
    ఒకవేళ (!component_has_sep &&
        !(మార్గం_2.ఖాళీలు() || has_మూలం_name(component, style))) {
      // Add a separator.
      మార్గం_2.నెట్టు_వెనుక(preferred_separator(style));
    }

    మార్గం_2.append(component.begin(), component.చివర());
  }
}

శూన్య append(Small_సదిశ_Impl<అక్షరం> &మార్గం_2, స్థిరరాశి Twine &a, స్థిరరాశి Twine &b,
            స్థిరరాశి Twine &c, స్థిరరాశి Twine &d) {
  append(మార్గం_2, Style::native, a, b, c, d);
}

శూన్య append(Small_సదిశ_Impl<అక్షరం> &మార్గం_2, const_iterator begin,
            const_iterator చివర, Style style) {
  కొరకు (; begin != చివర; ++begin)
    మార్గం_2::append(మార్గం_2, style, *begin);
}

StringRef మాతృ_మార్గం(StringRef మార్గం_2, Style style) {
  size_t చివర_pos = మాతృ_మార్గం_చివర(మార్గం_2, style);
  ఒకవేళ (చివర_pos == StringRef::npos)
    తిరిగి StringRef();
  తిరిగి మార్గం_2.substr(0, చివర_pos);
}

శూన్య remove_filename(Small_సదిశ_Impl<అక్షరం> &మార్గం_2, Style style) {
  size_t చివర_pos = మాతృ_మార్గం_చివర(StringRef(మార్గం_2.begin(), మార్గం_2.కొలత_2()), style);
  ఒకవేళ (చివర_pos != StringRef::npos)
    మార్గం_2.truncate(చివర_pos);
}

శూన్య replace_extension(Small_సదిశ_Impl<అక్షరం> &మార్గం_2, స్థిరరాశి Twine &extension,
                       Style style) {
  StringRef p(మార్గం_2.begin(), మార్గం_2.కొలత_2());
  SmallString<32> ext_storage;
  StringRef ext = extension.గమ్యం_String_Ref(ext_storage);

  // Erase existing extension.
  size_t pos = p.కనుగొను_చివరి_యొక్క('.');
  ఒకవేళ (pos != StringRef::npos && pos >= filename_pos(p, style))
    మార్గం_2.truncate(pos);

  // Append '.' if needed.
  ఒకవేళ (ext.కొలత_2() > 0 && ext[0] != '.')
    మార్గం_2.నెట్టు_వెనుక('.');

  // Append extension.
  మార్గం_2.append(ext.begin(), ext.చివర());
}

స్థిర తార్కిక starts_with(StringRef మార్గం, StringRef ఉపసర్గ,
                        Style style = Style::native) {
  // Windows prefix matching : case and separator insensitive
  ఒకవేళ (is_style_windows(style)) {
    ఒకవేళ (మార్గం.కొలత_2() < ఉపసర్గ.కొలత_2())
      తిరిగి అబద్ధం;
    కొరకు (size_t I = 0, E = ఉపసర్గ.కొలత_2(); I != E; ++I) {
      తార్కిక Sep_మార్గం = is_separator(మార్గం[I], style);
      తార్కిక Sep_ఉపసర్గ = is_separator(ఉపసర్గ[I], style);
      ఒకవేళ (Sep_మార్గం != Sep_ఉపసర్గ)
        తిరిగి అబద్ధం;
      ఒకవేళ (!Sep_మార్గం && గమ్యం_క్రింది(మార్గం[I]) != గమ్యం_క్రింది(ఉపసర్గ[I]))
        తిరిగి అబద్ధం;
    }
    తిరిగి నిజం;
  }
  తిరిగి మార్గం.starts_with(ఉపసర్గ);
}

తార్కిక replace_మార్గం_ఉపసర్గ(Small_సదిశ_Impl<అక్షరం> &మార్గం, StringRef Old_ఉపసర్గ,
                         StringRef New_ఉపసర్గ, Style style) {
  ఒకవేళ (Old_ఉపసర్గ.ఖాళీలు() && New_ఉపసర్గ.ఖాళీలు())
    తిరిగి అబద్ధం;

  StringRef Orig_మార్గం(మార్గం.begin(), మార్గం.కొలత_2());
  ఒకవేళ (!starts_with(Orig_మార్గం, Old_ఉపసర్గ, style))
    తిరిగి అబద్ధం;

  // If prefixes have the same size we can simply copy the new one over.
  ఒకవేళ (Old_ఉపసర్గ.కొలత_2() == New_ఉపసర్గ.కొలత_2()) {
    llvm::copy(New_ఉపసర్గ, మార్గం.begin());
    తిరిగి నిజం;
  }

  StringRef Rel_మార్గం = Orig_మార్గం.substr(Old_ఉపసర్గ.కొలత_2());
  SmallString<256> New_మార్గం;
  (Twine(New_ఉపసర్గ) + Rel_మార్గం).గమ్యం_సదిశ(New_మార్గం);
  మార్గం.మార్పిడి(New_మార్గం);
  తిరిగి నిజం;
}

శూన్య native(స్థిరరాశి Twine &మార్గం_2, Small_సదిశ_Impl<అక్షరం> &ఫలితం_2, Style style) {
  ప్రతిజ్ఞా((!మార్గం_2.isSingleStringRef() ||
          మార్గం_2.getSingleStringRef().దత్తాంశం() != ఫలితం_2.దత్తాంశం()) &&
         "path and result are not allowed to overlap!");
  // Clear result.
  ఫలితం_2.clear();
  మార్గం_2.గమ్యం_సదిశ(ఫలితం_2);
  native(ఫలితం_2, style);
}

std::string native(స్థిరరాశి Twine &మార్గం_2, Style style) {
  SmallString<128> ఫలితం;
  native(మార్గం_2, ఫలితం, style);
  తిరిగి std::string(ఫలితం);
}

శూన్య native(Small_సదిశ_Impl<అక్షరం> &మార్గం, Style style) {
  ఒకవేళ (మార్గం.ఖాళీలు())
    తిరిగి;
  ఒకవేళ (is_style_windows(style)) {
    కొరకు (అక్షరం &Ch : మార్గం)
      ఒకవేళ (is_separator(Ch, style))
        Ch = preferred_separator(style);
    ఒకవేళ (మార్గం[0] == '~' && (మార్గం.కొలత_2() == 1 || is_separator(మార్గం[1], style))) {
      SmallString<128> మార్గం_Home;
      home_directory(మార్గం_Home);
      మార్గం_Home.append(మార్గం.begin() + 1, మార్గం.చివర());
      మార్గం = std::move(మార్గం_Home);
    }
  } లేకపోతే {
    llvm::replace(మార్గం, '\\', '/');
  }
}

std::string convert_గమ్యం_slash(StringRef మార్గం_2, Style style) {
  ఒకవేళ (is_style_posix(style))
    తిరిగి std::string(మార్గం_2);

  std::string s = మార్గం_2.str();
  llvm::replace(s, '\\', '/');
  తిరిగి s;
}

StringRef filename(StringRef మార్గం_2, Style style) { తిరిగి *rbegin(మార్గం_2, style); }

StringRef stem(StringRef మార్గం_2, Style style) {
  StringRef fname = filename(మార్గం_2, style);
  size_t pos = fname.కనుగొను_చివరి_యొక్క('.');
  ఒకవేళ (pos == StringRef::npos)
    తిరిగి fname;
  ఒకవేళ ((fname.కొలత_2() == 1 && fname == ".") ||
      (fname.కొలత_2() == 2 && fname == ".."))
    తిరిగి fname;
  తిరిగి fname.substr(0, pos);
}

StringRef extension(StringRef మార్గం_2, Style style) {
  StringRef fname = filename(మార్గం_2, style);
  size_t pos = fname.కనుగొను_చివరి_యొక్క('.');
  ఒకవేళ (pos == StringRef::npos)
    తిరిగి StringRef();
  ఒకవేళ ((fname.కొలత_2() == 1 && fname == ".") ||
      (fname.కొలత_2() == 2 && fname == ".."))
    తిరిగి StringRef();
  తిరిగి fname.substr(pos);
}

తార్కిక is_separator(అక్షరం విలువ_2, Style style) {
  ఒకవేళ (విలువ_2 == '/')
    తిరిగి నిజం;
  ఒకవేళ (is_style_windows(style))
    తిరిగి విలువ_2 == '\\';
  తిరిగి అబద్ధం;
}

StringRef get_separator(Style style) {
  ఒకవేళ (real_style(style) == Style::windows)
    తిరిగి "\\";
  తిరిగి "/";
}

తార్కిక has_మూలం_name(స్థిరరాశి Twine &మార్గం_2, Style style) {
  SmallString<128> మార్గం_storage;
  StringRef p = మార్గం_2.గమ్యం_String_Ref(మార్గం_storage);

  తిరిగి !మూలం_name(p, style).ఖాళీలు();
}

తార్కిక has_మూలం_directory(స్థిరరాశి Twine &మార్గం_2, Style style) {
  SmallString<128> మార్గం_storage;
  StringRef p = మార్గం_2.గమ్యం_String_Ref(మార్గం_storage);

  తిరిగి !మూలం_directory(p, style).ఖాళీలు();
}

తార్కిక has_మూలం_మార్గం(స్థిరరాశి Twine &మార్గం_2, Style style) {
  SmallString<128> మార్గం_storage;
  StringRef p = మార్గం_2.గమ్యం_String_Ref(మార్గం_storage);

  తిరిగి !మూలం_మార్గం_2(p, style).ఖాళీలు();
}

తార్కిక has_relative_మార్గం(స్థిరరాశి Twine &మార్గం_2, Style style) {
  SmallString<128> మార్గం_storage;
  StringRef p = మార్గం_2.గమ్యం_String_Ref(మార్గం_storage);

  తిరిగి !relative_మార్గం(p, style).ఖాళీలు();
}

తార్కిక has_filename(స్థిరరాశి Twine &మార్గం_2, Style style) {
  SmallString<128> మార్గం_storage;
  StringRef p = మార్గం_2.గమ్యం_String_Ref(మార్గం_storage);

  తిరిగి !filename(p, style).ఖాళీలు();
}

తార్కిక has_మాతృ_మార్గం(స్థిరరాశి Twine &మార్గం_2, Style style) {
  SmallString<128> మార్గం_storage;
  StringRef p = మార్గం_2.గమ్యం_String_Ref(మార్గం_storage);

  తిరిగి !మాతృ_మార్గం(p, style).ఖాళీలు();
}

తార్కిక has_stem(స్థిరరాశి Twine &మార్గం_2, Style style) {
  SmallString<128> మార్గం_storage;
  StringRef p = మార్గం_2.గమ్యం_String_Ref(మార్గం_storage);

  తిరిగి !stem(p, style).ఖాళీలు();
}

తార్కిక has_extension(స్థిరరాశి Twine &మార్గం_2, Style style) {
  SmallString<128> మార్గం_storage;
  StringRef p = మార్గం_2.గమ్యం_String_Ref(మార్గం_storage);

  తిరిగి !extension(p, style).ఖాళీలు();
}

తార్కిక is_absolute(స్థిరరాశి Twine &మార్గం_2, Style style) {
  SmallString<128> మార్గం_storage;
  StringRef p = మార్గం_2.గమ్యం_String_Ref(మార్గం_storage);

  తార్కిక మూలం_Dir = has_మూలం_directory(p, style);
  తార్కిక మూలం_Name = is_style_posix(style) || has_మూలం_name(p, style);

  తిరిగి మూలం_Dir && మూలం_Name;
}

తార్కిక is_absolute_gnu(స్థిరరాశి Twine &మార్గం_2, Style style) {
  SmallString<128> మార్గం_storage;
  StringRef p = మార్గం_2.గమ్యం_String_Ref(మార్గం_storage);

  // Handle '/' which is absolute for both Windows and POSIX systems.
  // Handle '\\' on Windows.
  ఒకవేళ (!p.ఖాళీలు() && is_separator(p.front(), style))
    తిరిగి నిజం;

  ఒకవేళ (is_style_windows(style)) {
    // Handle drive letter pattern (a character followed by ':') on Windows.
    ఒకవేళ (p.కొలత_2() >= 2 && (p[0] && p[1] == ':'))
      తిరిగి నిజం;
  }

  తిరిగి అబద్ధం;
}

తార్కిక is_relative(స్థిరరాశి Twine &మార్గం_2, Style style) {
  తిరిగి !is_absolute(మార్గం_2, style);
}

శూన్య make_absolute(స్థిరరాశి Twine &current_directory,
                   Small_సదిశ_Impl<అక్షరం> &మార్గం_2) {
  StringRef p(మార్గం_2.దత్తాంశం(), మార్గం_2.కొలత_2());

  తార్కిక మూలం_Directory = has_మూలం_directory(p);
  తార్కిక మూలం_Name = has_మూలం_name(p);

  // Already absolute.
  ఒకవేళ ((మూలం_Name || is_style_posix(Style::native)) && మూలం_Directory)
    తిరిగి;

  // All the following conditions will need the current directory.
  SmallString<128> current_dir;
  current_directory.గమ్యం_సదిశ(current_dir);

  // Relative path. Prepend the current directory.
  ఒకవేళ (!మూలం_Name && !మూలం_Directory) {
    // Append path to the current directory.
    append(current_dir, p);
    // Set path to the result.
    మార్గం_2.మార్పిడి(current_dir);
    తిరిగి;
  }

  ఒకవేళ (!మూలం_Name && మూలం_Directory) {
    StringRef cdrn = మూలం_name(current_dir);
    SmallString<128> cur_Dir_మూలం_Name(cdrn.begin(), cdrn.చివర());
    append(cur_Dir_మూలం_Name, p);
    // Set path to the result.
    మార్గం_2.మార్పిడి(cur_Dir_మూలం_Name);
    తిరిగి;
  }

  ఒకవేళ (మూలం_Name && !మూలం_Directory) {
    StringRef p_మూలం_Name = మూలం_name(p);
    StringRef b_మూలం_Directory = మూలం_directory(current_dir);
    StringRef b_Relative_మార్గం = relative_మార్గం(current_dir);
    StringRef p_Relative_మార్గం = relative_మార్గం(p);

    SmallString<128> res;
    append(res, p_మూలం_Name, b_మూలం_Directory, b_Relative_మార్గం, p_Relative_మార్గం);
    మార్గం_2.మార్పిడి(res);
    తిరిగి;
  }

  llvm_unreachable("All rootName and rootDirectory combinations should have "
                   "occurred above!");
}

StringRef remove_leading_dotslash(StringRef మార్గం, Style style) {
  // Remove leading "./" (or ".//" or "././" etc.)
  అయితే (మార్గం.కొలత_2() > 2 && మార్గం[0] == '.' && is_separator(మార్గం[1], style)) {
    మార్గం = మార్గం.substr(2);
    అయితే (మార్గం.కొలత_2() > 0 && is_separator(మార్గం[0], style))
      మార్గం = మార్గం.substr(1);
  }
  తిరిగి మార్గం;
}

తార్కిక remove_dots(Small_సదిశ_Impl<అక్షరం> &the_మార్గం, తార్కిక remove_dot_dot,
                 Style style) {
  style = real_style(style);
  StringRef మిగిలిన_2(the_మార్గం.దత్తాంశం(), the_మార్గం.కొలత_2());
  తార్కిక needs_change = అబద్ధం;
  Small_సదిశ<StringRef, 16> components;

  // Consume the root path, if present.
  StringRef మూలం_3 = మార్గం_2::మూలం_మార్గం_2(మిగిలిన_2, style);
  తార్కిక absolute = !మూలం_3.ఖాళీలు();
  ఒకవేళ (absolute)
    మిగిలిన_2 = మిగిలిన_2.drop_front(మూలం_3.కొలత_2());

  // Loop over path components manually. This makes it easier to detect
  // non-preferred slashes and double separators that must be canonicalized.
  అయితే (!మిగిలిన_2.ఖాళీలు()) {
    size_t తదుపరి_slash = మిగిలిన_2.కనుగొను_మొదటి_యొక్క(separators(style));
    ఒకవేళ (తదుపరి_slash == StringRef::npos)
      తదుపరి_slash = మిగిలిన_2.కొలత_2();
    StringRef component = మిగిలిన_2.take_front(తదుపరి_slash);
    మిగిలిన_2 = మిగిలిన_2.drop_front(తదుపరి_slash);

    // Eat the slash, and check if it is the preferred separator.
    ఒకవేళ (!మిగిలిన_2.ఖాళీలు()) {
      needs_change |= మిగిలిన_2.front() != preferred_separator(style);
      మిగిలిన_2 = మిగిలిన_2.drop_front();
      // The path needs to be rewritten if it has a trailing slash.
      // FIXME: This is emergent behavior that could be removed.
      needs_change |= మిగిలిన_2.ఖాళీలు();
    }

    // Check for path traversal components or double separators.
    ఒకవేళ (component.ఖాళీలు() || component == ".") {
      needs_change = నిజం;
    } లేకపోతే ఒకవేళ (remove_dot_dot && component == "..") {
      needs_change = నిజం;
      // Do not allow ".." to remove the root component. If this is the
      // beginning of a relative path, keep the ".." component.
      ఒకవేళ (!components.ఖాళీలు() && components.వెనుక() != "..") {
        components.తీయి_వెనుక();
      } లేకపోతే ఒకవేళ (!absolute) {
        components.నెట్టు_వెనుక(component);
      }
    } లేకపోతే {
      components.నెట్టు_వెనుక(component);
    }
  }

  SmallString<256> బఫరు_2 = మూలం_3;
  // "root" could be "/", which may need to be translated into "\".
  make_preferred(బఫరు_2, style);
  needs_change |= మూలం_3 != బఫరు_2;

  // Avoid rewriting the path unless we have to.
  ఒకవేళ (!needs_change)
    తిరిగి అబద్ధం;

  ఒకవేళ (!components.ఖాళీలు()) {
    బఫరు_2 += components[0];
    కొరకు (StringRef C : ArrayRef(components).drop_front()) {
      బఫరు_2 += preferred_separator(style);
      బఫరు_2 += C;
    }
  }
  the_మార్గం.మార్పిడి(బఫరు_2);
  తిరిగి నిజం;
}

} // end namespace path

నామాకాశ fs {

std::error_సంకేతం getUniqueID(స్థిరరాశి Twine మార్గం, UniqueID &ఫలితం) {
  sandbox::violationIfEnabled();

  file_స్థితి స్థితి_2;
  std::error_సంకేతం EC = స్థితి_3(మార్గం, స్థితి_2);
  ఒకవేళ (EC)
    తిరిగి EC;
  ఫలితం = స్థితి_2.getUniqueID();
  తిరిగి std::error_సంకేతం();
}

శూన్య create_Unique_మార్గం(స్థిరరాశి Twine &Model, Small_సదిశ_Impl<అక్షరం> &ఫలితం_మార్గం,
                      తార్కిక MakeAbsolute) {
  SmallString<128> ModelStorage;
  Model.గమ్యం_సదిశ(ModelStorage);

  ప్రతిజ్ఞా(llvm::is_contained(ModelStorage, '%') &&
         "createUniquePath: Model must contain at least one '%'");

  ఒకవేళ (MakeAbsolute) {
    // Make model absolute by prepending a temp directory if it's not already.
    ఒకవేళ (!sys::మార్గం_2::is_absolute(Twine(ModelStorage))) {
      SmallString<128> TDir;
      sys::మార్గం_2::system_తాత్కాలిక_directory(నిజం, TDir);
      sys::మార్గం_2::append(TDir, Twine(ModelStorage));
      ModelStorage.మార్పిడి(TDir);
    }
  }

  ఫలితం_మార్గం = ModelStorage;
  ఫలితం_మార్గం.నెట్టు_వెనుక(0);
  ఫలితం_మార్గం.తీయి_వెనుక();

  // Replace '%' with random chars.
  కొరకు (గుర్తులేని i = 0, e = ModelStorage.కొలత_2(); i != e; ++i) {
    ఒకవేళ (ModelStorage[i] == '%')
      ఫలితం_మార్గం[i] = "0123456789abcdef"[sys::Process::Get_Random_సంఖ్య() & 15];
  }
}

std::error_సంకేతం createUniqueFile(స్థిరరాశి Twine &Model, పూర్ణాంకం &ఫలితం_Fd,
                                 Small_సదిశ_Impl<అక్షరం> &ఫలితం_మార్గం,
                                 OpenFlags Flags, గుర్తులేని బహుళకం) {
  తిరిగి createUniqueEntity(Model, ఫలితం_Fd, ఫలితం_మార్గం, అబద్ధం, FS_File, Flags,
                            బహుళకం);
}

std::error_సంకేతం createUniqueFile(స్థిరరాశి Twine &Model,
                                 Small_సదిశ_Impl<అక్షరం> &ఫలితం_మార్గం,
                                 గుర్తులేని బహుళకం) {
  పూర్ణాంకం FD;
  స్వయంచాలక EC = createUniqueFile(Model, FD, ఫలితం_మార్గం, యొక్క_None, బహుళకం);
  ఒకవేళ (EC)
    తిరిగి EC;
  // FD is only needed to avoid race conditions. Close it right away.
  close(FD);
  తిరిగి EC;
}

స్థిర std::error_సంకేతం
createTemporaryFile(స్థిరరాశి Twine &Model, పూర్ణాంకం &ఫలితం_FD,
                    llvm::Small_సదిశ_Impl<అక్షరం> &ఫలితం_మార్గం, FSEntity Type,
                    sys::fs::OpenFlags Flags = sys::fs::యొక్క_None) {
  // Any *temporary* file is assumed to be a compiler-internal output, not
  // a formal one.
  స్వయంచాలక BypassSandbox = sys::sandbox::scopedDisable();

  SmallString<128> Storage;
  StringRef P = Model.గమ్యం_Null_Terminated_String_Ref(Storage);
  ప్రతిజ్ఞా(P.కనుగొను_మొదటి_యొక్క(separators(Style::native)) == StringRef::npos &&
         "Model must be a simple filename.");
  // Use P.begin() so that createUniqueEntity doesn't need to recreate Storage.
  తిరిగి createUniqueEntity(P.begin(), ఫలితం_FD, ఫలితం_మార్గం, నిజం, Type, Flags,
                            all_read | all_write);
}

స్థిర std::error_సంకేతం
createTemporaryFile(స్థిరరాశి Twine &ఉపసర్గ, StringRef Suffix, పూర్ణాంకం &ఫలితం_FD,
                    llvm::Small_సదిశ_Impl<అక్షరం> &ఫలితం_మార్గం, FSEntity Type,
                    sys::fs::OpenFlags Flags = sys::fs::యొక్క_None) {
  స్థిరరాశి అక్షరం *మధ్య = Suffix.ఖాళీలు() ? "-%%%%%%" : "-%%%%%%.";
  తిరిగి createTemporaryFile(ఉపసర్గ + మధ్య + Suffix, ఫలితం_FD, ఫలితం_మార్గం,
                             Type, Flags);
}

std::error_సంకేతం createTemporaryFile(స్థిరరాశి Twine &ఉపసర్గ, StringRef Suffix,
                                    పూర్ణాంకం &ఫలితం_FD,
                                    Small_సదిశ_Impl<అక్షరం> &ఫలితం_మార్గం,
                                    sys::fs::OpenFlags Flags) {
  తిరిగి createTemporaryFile(ఉపసర్గ, Suffix, ఫలితం_FD, ఫలితం_మార్గం, FS_File,
                             Flags);
}

std::error_సంకేతం createTemporaryFile(స్థిరరాశి Twine &ఉపసర్గ, StringRef Suffix,
                                    Small_సదిశ_Impl<అక్షరం> &ఫలితం_మార్గం,
                                    sys::fs::OpenFlags Flags) {
  పూర్ణాంకం FD;
  స్వయంచాలక EC = createTemporaryFile(ఉపసర్గ, Suffix, FD, ఫలితం_మార్గం, Flags);
  ఒకవేళ (EC)
    తిరిగి EC;
  // FD is only needed to avoid race conditions. Close it right away.
  close(FD);
  తిరిగి EC;
}

// This is a mkdtemp with a different pattern. We use createUniqueEntity mostly
// for consistency. We should try using mkdtemp.
std::error_సంకేతం createUniqueDirectory(స్థిరరాశి Twine &ఉపసర్గ,
                                      Small_సదిశ_Impl<అక్షరం> &ఫలితం_మార్గం) {
  పూర్ణాంకం Dummy;
  తిరిగి createUniqueEntity(ఉపసర్గ + "-%%%%%%", Dummy, ఫలితం_మార్గం, నిజం,
                            FS_Dir);
}

std::error_సంకేతం
getPotentiallyUniqueFileName(స్థిరరాశి Twine &Model,
                             Small_సదిశ_Impl<అక్షరం> &ఫలితం_మార్గం) {
  పూర్ణాంకం Dummy;
  తిరిగి createUniqueEntity(Model, Dummy, ఫలితం_మార్గం, అబద్ధం, FS_Name);
}

std::error_సంకేతం
get_Potentially_Unique_తాత్కాలిక_File_Name(స్థిరరాశి Twine &ఉపసర్గ, StringRef Suffix,
                                 Small_సదిశ_Impl<అక్షరం> &ఫలితం_మార్గం) {
  పూర్ణాంకం Dummy;
  తిరిగి createTemporaryFile(ఉపసర్గ, Suffix, Dummy, ఫలితం_మార్గం, FS_Name);
}

std::error_సంకేతం make_absolute(Small_సదిశ_Impl<అక్షరం> &మార్గం_2) {
  sandbox::violationIfEnabled();

  ఒకవేళ (మార్గం_2::is_absolute(మార్గం_2))
    తిరిగి {};

  SmallString<128> current_dir;
  ఒకవేళ (std::error_సంకేతం ec = current_మార్గం(current_dir))
    తిరిగి ec;

  మార్గం_2::make_absolute(current_dir, మార్గం_2);
  తిరిగి {};
}

std::error_సంకేతం create_directories(స్థిరరాశి Twine &మార్గం, తార్కిక IgnoreExisting,
                                   perms Perms) {
  SmallString<128> మార్గం_Storage;
  StringRef P = మార్గం.గమ్యం_String_Ref(మార్గం_Storage);

  // Be optimistic and try to create the directory
  std::error_సంకేతం EC = create_directory(P, IgnoreExisting, Perms);
  // If we succeeded, or had any error other than the parent not existing, just
  // return it.
  ఒకవేళ (EC != errc::no_such_file_or_directory)
    తిరిగి EC;

  // We failed because of a no_such_file_or_directory, try to create the
  // parent.
  StringRef మాతృ_2 = మార్గం_2::మాతృ_మార్గం(P);
  ఒకవేళ (మాతృ_2.ఖాళీలు())
    తిరిగి EC;

  ఒకవేళ ((EC = create_directories(మాతృ_2, IgnoreExisting, Perms)))
      తిరిగి EC;

  తిరిగి create_directory(P, IgnoreExisting, Perms);
}

స్థిర std::error_సంకేతం copy_file_అంతర్గత(పూర్ణాంకం ReadFD, పూర్ణాంకం WriteFD) {
  స్థిరరాశి size_t Buf_కొలత = 4096;
  అక్షరం *Buf = కొత్త అక్షరం[Buf_కొలత];
  పూర్ణాంకం బైట్లు_Read = 0, బైట్లు_Written = 0;
  కొరకు (;;) {
    బైట్లు_Read = పఠ(ReadFD, Buf, Buf_కొలత);
    ఒకవేళ (బైట్లు_Read <= 0)
      ఆపు;
    అయితే (బైట్లు_Read) {
      బైట్లు_Written = లిఖ(WriteFD, Buf, బైట్లు_Read);
      ఒకవేళ (బైట్లు_Written < 0)
        ఆపు;
      బైట్లు_Read -= బైట్లు_Written;
    }
    ఒకవేళ (బైట్లు_Written < 0)
      ఆపు;
  }
  తొలగించు[] Buf;

  ఒకవేళ (బైట్లు_Read < 0 || బైట్లు_Written < 0)
    తిరిగి errno_As_Error_సంకేతం();
  తిరిగి std::error_సంకేతం();
}

#నిర్వచితం_కాకపోతే __APPLE__
std::error_సంకేతం copy_file(స్థిరరాశి Twine &From, స్థిరరాశి Twine &గమ్యం) {
  పూర్ణాంకం ReadFD, WriteFD;
  ఒకవేళ (std::error_సంకేతం EC = openFileForRead(From, ReadFD, యొక్క_None))
    తిరిగి EC;
  ఒకవేళ (std::error_సంకేతం EC =
          openFileForWrite(గమ్యం, WriteFD, CD_CreateAlways, యొక్క_None)) {
    close(ReadFD);
    తిరిగి EC;
  }

  std::error_సంకేతం EC = copy_file_అంతర్గత(ReadFD, WriteFD);

  close(ReadFD);
  close(WriteFD);

  తిరిగి EC;
}
#అయితే_ముగింపు

std::error_సంకేతం copy_file(స్థిరరాశి Twine &From, పూర్ణాంకం గమ్యం_FD) {
  పూర్ణాంకం ReadFD;
  ఒకవేళ (std::error_సంకేతం EC = openFileForRead(From, ReadFD, యొక్క_None))
    తిరిగి EC;

  std::error_సంకేతం EC = copy_file_అంతర్గత(ReadFD, గమ్యం_FD);

  close(ReadFD);

  తిరిగి EC;
}

ErrorOr<MD5::MD5_ఫలితం> md5_contents(పూర్ణాంకం FD) {
  sandbox::violationIfEnabled();

  MD5 Hash;

  constexpr size_t Buf_కొలత = 4096;
  std::సదిశ<uint8_t> Buf(Buf_కొలత);
  పూర్ణాంకం బైట్లు_Read = 0;
  కొరకు (;;) {
    బైట్లు_Read = పఠ(FD, Buf.దత్తాంశం(), Buf_కొలత);
    ఒకవేళ (బైట్లు_Read <= 0)
      ఆపు;
    Hash.update(ArrayRef(Buf.దత్తాంశం(), బైట్లు_Read));
  }

  ఒకవేళ (బైట్లు_Read < 0)
    తిరిగి errno_As_Error_సంకేతం();
  MD5::MD5_ఫలితం ఫలితం;
  Hash.final(ఫలితం);
  తిరిగి ఫలితం;
}

ErrorOr<MD5::MD5_ఫలితం> md5_contents(స్థిరరాశి Twine &మార్గం) {
  sandbox::violationIfEnabled();

  పూర్ణాంకం FD;
  ఒకవేళ (స్వయంచాలక EC = openFileForRead(మార్గం, FD, యొక్క_None))
    తిరిగి EC;

  స్వయంచాలక ఫలితం = md5_contents(FD);
  close(FD);
  తిరిగి ఫలితం;
}

తార్కిక exists(స్థిరరాశి basic_file_స్థితి &స్థితి_3) {
  తిరిగి స్థితి_తెలిసిన(స్థితి_3) && స్థితి_3.type() != file_type::file_not_దొరికింది;
}

తార్కిక స్థితి_తెలిసిన(స్థిరరాశి basic_file_స్థితి &s) {
  తిరిగి s.type() != file_type::స్థితి_error;
}

file_type get_file_type(స్థిరరాశి Twine &మార్గం, తార్కిక Follow) {
  file_స్థితి st;
  ఒకవేళ (స్థితి_3(మార్గం, st, Follow))
    తిరిగి file_type::స్థితి_error;
  తిరిగి st.type();
}

తార్కిక is_directory(స్థిరరాశి basic_file_స్థితి &స్థితి_3) {
  తిరిగి స్థితి_3.type() == file_type::directory_file;
}

std::error_సంకేతం is_directory(స్థిరరాశి Twine &మార్గం_2, తార్కిక &ఫలితం_2) {
  sandbox::violationIfEnabled();

  file_స్థితి st;
  ఒకవేళ (std::error_సంకేతం ec = స్థితి_3(మార్గం_2, st))
    తిరిగి ec;
  ఫలితం_2 = is_directory(st);
  తిరిగి std::error_సంకేతం();
}

తార్కిక is_regular_file(స్థిరరాశి basic_file_స్థితి &స్థితి_3) {
  తిరిగి స్థితి_3.type() == file_type::regular_file;
}

std::error_సంకేతం is_regular_file(స్థిరరాశి Twine &మార్గం_2, తార్కిక &ఫలితం_2) {
  sandbox::violationIfEnabled();

  file_స్థితి st;
  ఒకవేళ (std::error_సంకేతం ec = స్థితి_3(మార్గం_2, st))
    తిరిగి ec;
  ఫలితం_2 = is_regular_file(st);
  తిరిగి std::error_సంకేతం();
}

తార్కిక is_symlink_file(స్థిరరాశి basic_file_స్థితి &స్థితి_3) {
  తిరిగి స్థితి_3.type() == file_type::symlink_file;
}

std::error_సంకేతం is_symlink_file(స్థిరరాశి Twine &మార్గం_2, తార్కిక &ఫలితం_2) {
  sandbox::violationIfEnabled();

  file_స్థితి st;
  ఒకవేళ (std::error_సంకేతం ec = స్థితి_3(మార్గం_2, st, అబద్ధం))
    తిరిగి ec;
  ఫలితం_2 = is_symlink_file(st);
  తిరిగి std::error_సంకేతం();
}

తార్కిక is_other(స్థిరరాశి basic_file_స్థితి &స్థితి_3) {
  తిరిగి exists(స్థితి_3) &&
         !is_regular_file(స్థితి_3) &&
         !is_directory(స్థితి_3);
}

std::error_సంకేతం is_other(స్థిరరాశి Twine &మార్గం, తార్కిక &ఫలితం) {
  sandbox::violationIfEnabled();

  file_స్థితి File_స్థితి;
  ఒకవేళ (std::error_సంకేతం EC = స్థితి_3(మార్గం, File_స్థితి))
    తిరిగి EC;
  ఫలితం = is_other(File_స్థితి);
  తిరిగి std::error_సంకేతం();
}

శూన్య directory_నమోదు::replace_filename(స్థిరరాశి Twine &Filename, file_type Type,
                                       basic_file_స్థితి స్థితి_2) {
  SmallString<128> మార్గం_Str = మార్గం_2::మాతృ_మార్గం(మార్గం);
  మార్గం_2::append(మార్గం_Str, Filename);
  ఇది->మార్గం = std::string(మార్గం_Str);
  ఇది->Type = Type;
  ఇది->స్థితి_2 = స్థితి_2;
}

ErrorOr<perms> getPermissions(స్థిరరాశి Twine &మార్గం) {
  sandbox::violationIfEnabled();

  file_స్థితి స్థితి_2;
  ఒకవేళ (std::error_సంకేతం EC = స్థితి_3(మార్గం, స్థితి_2))
    తిరిగి EC;

  తిరిగి స్థితి_2.permissions();
}

std::error_సంకేతం సమితి_చివరి_Access_And_Modification_సమయం(స్థిరరాశి Twine &మార్గం,
                                                 సమయం_బిందువు<> Access_సమయం,
                                                 సమయం_బిందువు<> Modification_సమయం) {
  పూర్ణాంకం FD;
  ఒకవేళ (std::error_సంకేతం EC = openFile(మార్గం, FD, CD_OpenExisting, FA_Read,
                                    యొక్క_Update_లక్షణాలు | యొక్క_Open_Directory))
    తిరిగి EC;
  std::error_సంకేతం EC =
      సమితి_చివరి_Access_And_Modification_సమయం(FD, Access_సమయం, Modification_సమయం);
  Process::SafelyCloseFileDescriptor(FD);
  తిరిగి EC;
}

size_t mapped_file_region::కొలత_2() స్థిరరాశి {
  ప్రతిజ్ఞా(Mapping && "Mapping failed but used anyway!");
  తిరిగి కొలత;
}

అక్షరం *mapped_file_region::దత్తాంశం() స్థిరరాశి {
  ప్రతిజ్ఞా(Mapping && "Mapping failed but used anyway!");
  తిరిగి reinterpret_cast<అక్షరం *>(Mapping);
}

స్థిరరాశి అక్షరం *mapped_file_region::const_దత్తాంశం() స్థిరరాశి {
  ప్రతిజ్ఞా(Mapping && "Mapping failed but used anyway!");
  తిరిగి reinterpret_cast<స్థిరరాశి అక్షరం *>(Mapping);
}

Error read_Native_File_గమ్యం_EOF(file_t FileHandle, Small_సదిశ_Impl<అక్షరం> &బఫరు,
                          ssize_t Chunk_కొలత) {
  sandbox::violationIfEnabled();

  // Install a handler to truncate the buffer to the correct size on exit.
  size_t కొలత = బఫరు.కొలత_2();
  llvm::scope_exit TruncateOnExit([&]() { బఫరు.truncate(కొలత); });

  // Read into Buffer until we hit EOF.
  కొరకు (;;) {
    బఫరు.resize_for_overwrite(కొలత + Chunk_కొలత);
    Expected<size_t> Read_బైట్లు = readNativeFile(
        FileHandle, MutableArrayRef(బఫరు.begin() + కొలత, Chunk_కొలత));
    ఒకవేళ (!Read_బైట్లు)
      తిరిగి Read_బైట్లు.takeError();
    ఒకవేళ (*Read_బైట్లు == 0)
      తిరిగి Error::success();
    కొలత += *Read_బైట్లు;
  }
}

} // end namespace fs
} // end namespace sys
} // end namespace llvm

// Include the truly platform-specific parts.
#ఒకవేళ defined(LLVM_ON_UNIX)
#అన్తర్భావ "Unix/Path.inc"
#అయితే_ముగింపు
#ఒకవేళ defined(_WIN32)
#అన్తర్భావ "Windows/Path.inc"
#అయితే_ముగింపు

నామాకాశ llvm {
నామాకాశ sys {
నామాకాశ fs {

తాత్కాలిక_File::తాత్కాలిక_File(StringRef Name, పూర్ణాంకం FD)
    : TmpName(std::string(Name)), FD(FD) {}
తాత్కాలిక_File::తాత్కాలిక_File(తాత్కాలిక_File &&Other) { *ఇది = std::move(Other); }
తాత్కాలిక_File &తాత్కాలిక_File::క్రియాకారకం=(తాత్కాలిక_File &&Other) {
  TmpName = std::move(Other.TmpName);
  FD = Other.FD;
  Other.పూర్తి = నిజం;
  Other.FD = -1;
#నిర్వచితమైతే _WIN32
  RemoveOnClose = Other.RemoveOnClose;
  Other.RemoveOnClose = అబద్ధం;
#అయితే_ముగింపు
  తిరిగి *ఇది;
}

తాత్కాలిక_File::~తాత్కాలిక_File() { ప్రతిజ్ఞా(పూర్తి); }

Error తాత్కాలిక_File::discard() {
  పూర్తి = నిజం;
  ఒకవేళ (FD != -1 && close(FD) == -1) {
    std::error_సంకేతం EC = errno_As_Error_సంకేతం();
    తిరిగి error_సంకేతం_గమ్యం_Error(EC);
  }
  FD = -1;

#నిర్వచితమైతే _WIN32
  // On Windows, closing will remove the file, if we set the delete
  // disposition. If not, remove it manually.
  తార్కిక Remove = RemoveOnClose;
#లేకపోతే
  // Always try to remove the file.
  తార్కిక Remove = నిజం;
#అయితే_ముగింపు
  std::error_సంకేతం RemoveEC;
  ఒకవేళ (Remove && !TmpName.ఖాళీలు()) {
    RemoveEC = fs::remove(TmpName);
    sys::DontRemoveFileOnSignal(TmpName);
    ఒకవేళ (!RemoveEC)
      TmpName = "";
  } లేకపోతే {
    TmpName = "";
  }
  తిరిగి error_సంకేతం_గమ్యం_Error(RemoveEC);
}

Error తాత్కాలిక_File::keep(స్థిరరాశి Twine &Name) {
  ప్రతిజ్ఞా(!పూర్తి);
  పూర్తి = నిజం;
  // Always try to close and rename.
#నిర్వచితమైతే _WIN32
  // If we can't cancel the delete don't rename.
  స్వయంచాలక H = reinterpret_cast<HANDLE>(_get_osfhandle(FD));
  std::error_సంకేతం RenameEC =
      RemoveOnClose ? std::error_సంకేతం() : సమితి_Delete_Disposition(H, అబద్ధం);
  తార్కిక ShouldDelete = అబద్ధం;
  ఒకవేళ (!RenameEC) {
    RenameEC = rename_handle(H, Name);
    // If rename failed because it's cross-device, copy instead
    ఒకవేళ (RenameEC ==
      std::error_సంకేతం(ERROR_NOT_SAME_DEVICE, std::system_category())) {
      RenameEC = copy_file(TmpName, Name);
      ShouldDelete = నిజం;
    }
  }

  // If we can't rename or copy, discard the temporary file.
  ఒకవేళ (RenameEC)
    ShouldDelete = నిజం;
  ఒకవేళ (ShouldDelete) {
    ఒకవేళ (!RemoveOnClose)
      సమితి_Delete_Disposition(H, నిజం);
    లేకపోతే
      remove(TmpName);
  }
#లేకపోతే
  std::error_సంకేతం RenameEC = fs::rename(TmpName, Name);
  ఒకవేళ (RenameEC) {
    // If we can't rename, try to copy to work around cross-device link issues.
    RenameEC = sys::fs::copy_file(TmpName, Name);
    // If we can't rename or copy, discard the temporary file.
    ఒకవేళ (RenameEC)
      remove(TmpName);
  }
#అయితే_ముగింపు
  sys::DontRemoveFileOnSignal(TmpName);

  ఒకవేళ (!RenameEC)
    TmpName = "";

  ఒకవేళ (close(FD) == -1)
    తిరిగి error_సంకేతం_గమ్యం_Error(errno_As_Error_సంకేతం());
  FD = -1;

  తిరిగి error_సంకేతం_గమ్యం_Error(RenameEC);
}

Error తాత్కాలిక_File::keep() {
  ప్రతిజ్ఞా(!పూర్తి);
  పూర్తి = నిజం;

#నిర్వచితమైతే _WIN32
  స్వయంచాలక H = reinterpret_cast<HANDLE>(_get_osfhandle(FD));
  ఒకవేళ (std::error_సంకేతం EC = సమితి_Delete_Disposition(H, అబద్ధం))
    తిరిగి error_సంకేతం_గమ్యం_Error(EC);
#అయితే_ముగింపు
  sys::DontRemoveFileOnSignal(TmpName);

  TmpName = "";

  ఒకవేళ (close(FD) == -1)
    తిరిగి error_సంకేతం_గమ్యం_Error(errno_As_Error_సంకేతం());
  FD = -1;

  తిరిగి Error::success();
}

Expected<తాత్కాలిక_File> తాత్కాలిక_File::create(స్థిరరాశి Twine &Model, గుర్తులేని బహుళకం,
                                    OpenFlags ExtraFlags) {
  పూర్ణాంకం FD;
  SmallString<128> ఫలితం_మార్గం;
  ఒకవేళ (std::error_సంకేతం EC =
          createUniqueFile(Model, FD, ఫలితం_మార్గం, యొక్క_Delete | ExtraFlags, బహుళకం))
    తిరిగి error_సంకేతం_గమ్యం_Error(EC);

  తాత్కాలిక_File Ret(ఫలితం_మార్గం, FD);
#నిర్వచితమైతే _WIN32
  స్వయంచాలక H = reinterpret_cast<HANDLE>(_get_osfhandle(FD));
  తార్కిక సమితి_Signal_Handler = అబద్ధం;
  ఒకవేళ (std::error_సంకేతం EC = సమితి_Delete_Disposition(H, నిజం)) {
    Ret.RemoveOnClose = నిజం;
    సమితి_Signal_Handler = నిజం;
  }
#లేకపోతే
  తార్కిక సమితి_Signal_Handler = నిజం;
#అయితే_ముగింపు
  ఒకవేళ (సమితి_Signal_Handler && sys::RemoveFileOnSignal(ఫలితం_మార్గం)) {
    // Make sure we delete the file when RemoveFileOnSignal fails.
    consumeError(Ret.discard());
    std::error_సంకేతం EC(errc::operation_not_permitted);
    తిరిగి error_సంకేతం_గమ్యం_Error(EC);
  }
  తిరిగి std::move(Ret);
}
} // namespace fs

} // namespace sys
} // namespace llvm
