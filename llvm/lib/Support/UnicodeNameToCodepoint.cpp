//===- llvm/Support/UnicodeNameToCodepoint.cpp - Unicode character properties
//-*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file implements functions to map the name or alias of a unicode
// character to its codepoint.
//
//===----------------------------------------------------------------------===//

#ಅನ್ತರ್ಭಾವ "llvm/ADT/STLExtras.h"
#ಅನ್ತರ್ಭಾವ "llvm/ADT/StringExtras.h"
#ಅನ್ತರ್ಭಾವ "llvm/ADT/StringRef.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/Unicode.h"

ನಾಮಾಕಾಶ llvm {
ನಾಮಾಕಾಶ sys {
ನಾಮಾಕಾಶ unicode {

ಬಾಹ್ಯ ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಸ್ಥಿರಾಂಕ Unicode_Name_ಗಮ್ಯ_Codepoint_Dict;
ಬಾಹ್ಯ ಸ್ಥಿರಾಂಕ uint8_t *ಸ್ಥಿರಾಂಕ Unicode_Name_ಗಮ್ಯ_Codepoint_ಸೂಚ್ಯಂಕ;
ಬಾಹ್ಯ ಸ್ಥಿರಾಂಕ std::size_t Unicode_Name_ಗಮ್ಯ_Codepoint_ಸೂಚ್ಯಂಕ_ಗಾತ್ರ;
ಬಾಹ್ಯ ಸ್ಥಿರಾಂಕ std::size_t Unicode_Name_ಗಮ್ಯ_Codepoint_ಅತಿದೊಡ್ಡ_Name_ಗಾತ್ರ;

ಬಳಸಿ ಬಫರ್_Type = SmallString<64>;

ರಚನೆ ಗಂಟು {
  ತಾರ್ಕಿಕ Is_ಬೇರು = ಸುಳ್ಳು;
  char32_t ಮೌಲ್ಯ = 0xFFFFFFFF;
  uint32_t Children_ಸ್ಥಾನಾಂತರ = 0;
  ತಾರ್ಕಿಕ HasSibling = ಸುಳ್ಳು;
  uint32_t ಗಾತ್ರ_2 = 0;
  StringRef Name;
  ಸ್ಥಿರಾಂಕ ಗಂಟು *ಪೋಷಕ_2 = ಶೂನ್ಯಸೂಚಕ;

  constexpr ತಾರ್ಕಿಕ isValid() ಸ್ಥಿರಾಂಕ {
    ಹಿಂತಿರುಗಿ !Name.ಖಾಲಿಗಳು() || ಮೌಲ್ಯ == 0xFFFFFFFF;
  }
  constexpr ತಾರ್ಕಿಕ hasChildren() ಸ್ಥಿರಾಂಕ { ಹಿಂತಿರುಗಿ Children_ಸ್ಥಾನಾಂತರ != 0 || Is_ಬೇರು; }

  std::string ತುಂಬಿದ_Name() ಸ್ಥಿರಾಂಕ {
    std::string S;
    // Reserve enough space for most unicode code points.
    // The chosen value represent the 99th percentile of name size as of
    // Unicode 15.0.
    S.reserve(46);
    ಸ್ಥಿರಾಂಕ ಗಂಟು *N = ಇದು;
    ಆಗ (N) {
      std::ಹಿಮ್ಮುಖ_copy(N->Name.begin(), N->Name.ಕೊನೆ(), std::ಹಿಂಭಾಗ_inserter(S));
      N = N->ಪೋಷಕ_2;
    }
    std::ಹಿಮ್ಮುಖ(S.begin(), S.ಕೊನೆ());
    ಹಿಂತಿರುಗಿ S;
  }
};

ಸ್ಥಿರ ಗಂಟು create_ಬೇರು() {
  ಗಂಟು N;
  N.Is_ಬೇರು = ನಿಜ;
  N.Children_ಸ್ಥಾನಾಂತರ = 1;
  N.ಗಾತ್ರ_2 = 1;
  ಹಿಂತಿರುಗಿ N;
}

ಸ್ಥಿರ ಗಂಟು read_ಗಂಟು(uint32_t ಸ್ಥಾನಾಂತರ, ಸ್ಥಿರಾಂಕ ಗಂಟು *ಪೋಷಕ_2 = ಶೂನ್ಯಸೂಚಕ) {
  ಆದರೆ (ಸ್ಥಾನಾಂತರ == 0)
    ಹಿಂತಿರುಗಿ create_ಬೇರು();

  uint32_t Origin = ಸ್ಥಾನಾಂತರ;
  ಗಂಟು N;
  N.ಪೋಷಕ_2 = ಪೋಷಕ_2;
  uint8_t NameInfo = Unicode_Name_ಗಮ್ಯ_Codepoint_ಸೂಚ್ಯಂಕ[ಸ್ಥಾನಾಂತರ++];
  ಆದರೆ (ಸ್ಥಾನಾಂತರ + 6 >= Unicode_Name_ಗಮ್ಯ_Codepoint_ಸೂಚ್ಯಂಕ_ಗಾತ್ರ)
    ಹಿಂತಿರುಗಿ N;

  ತಾರ್ಕಿಕ LongName = NameInfo & 0x40;
  ತಾರ್ಕಿಕ Has_ಮೌಲ್ಯ = NameInfo & 0x80;
  std::size_t ಗಾತ್ರ_2 = NameInfo & ~0xC0;
  ಆದರೆ (LongName) {
    uint32_t Name_ಸ್ಥಾನಾಂತರ = (Unicode_Name_ಗಮ್ಯ_Codepoint_ಸೂಚ್ಯಂಕ[ಸ್ಥಾನಾಂತರ++] << 8);
    Name_ಸ್ಥಾನಾಂತರ |= Unicode_Name_ಗಮ್ಯ_Codepoint_ಸೂಚ್ಯಂಕ[ಸ್ಥಾನಾಂತರ++];
    N.Name = StringRef(Unicode_Name_ಗಮ್ಯ_Codepoint_Dict + Name_ಸ್ಥಾನಾಂತರ, ಗಾತ್ರ_2);
  } ಇಲ್ಲದಿದ್ದರೆ {
    N.Name = StringRef(Unicode_Name_ಗಮ್ಯ_Codepoint_Dict + ಗಾತ್ರ_2, 1);
  }
  ಆದರೆ (Has_ಮೌಲ್ಯ) {
    uint8_t H = Unicode_Name_ಗಮ್ಯ_Codepoint_ಸೂಚ್ಯಂಕ[ಸ್ಥಾನಾಂತರ++];
    uint8_t M = Unicode_Name_ಗಮ್ಯ_Codepoint_ಸೂಚ್ಯಂಕ[ಸ್ಥಾನಾಂತರ++];
    uint8_t L = Unicode_Name_ಗಮ್ಯ_Codepoint_ಸೂಚ್ಯಂಕ[ಸ್ಥಾನಾಂತರ++];
    N.ಮೌಲ್ಯ = ((H << 16) | (M << 8) | L) >> 3;

    ತಾರ್ಕಿಕ HasChildren = L & 0x02;
    N.HasSibling = L & 0x01;

    ಆದರೆ (HasChildren) {
      N.Children_ಸ್ಥಾನಾಂತರ = Unicode_Name_ಗಮ್ಯ_Codepoint_ಸೂಚ್ಯಂಕ[ಸ್ಥಾನಾಂತರ++] << 16;
      N.Children_ಸ್ಥಾನಾಂತರ |= Unicode_Name_ಗಮ್ಯ_Codepoint_ಸೂಚ್ಯಂಕ[ಸ್ಥಾನಾಂತರ++] << 8;
      N.Children_ಸ್ಥಾನಾಂತರ |= Unicode_Name_ಗಮ್ಯ_Codepoint_ಸೂಚ್ಯಂಕ[ಸ್ಥಾನಾಂತರ++];
    }
  } ಇಲ್ಲದಿದ್ದರೆ {
    uint8_t H = Unicode_Name_ಗಮ್ಯ_Codepoint_ಸೂಚ್ಯಂಕ[ಸ್ಥಾನಾಂತರ++];
    N.HasSibling = H & 0x80;
    ತಾರ್ಕಿಕ HasChildren = H & 0x40;
    H &= uint8_t(~0xC0);
    ಆದರೆ (HasChildren) {
      N.Children_ಸ್ಥಾನಾಂತರ = (H << 16);
      N.Children_ಸ್ಥಾನಾಂತರ |=
          (uint32_t(Unicode_Name_ಗಮ್ಯ_Codepoint_ಸೂಚ್ಯಂಕ[ಸ್ಥಾನಾಂತರ++]) << 8);
      N.Children_ಸ್ಥಾನಾಂತರ |= Unicode_Name_ಗಮ್ಯ_Codepoint_ಸೂಚ್ಯಂಕ[ಸ್ಥಾನಾಂತರ++];
    }
  }
  N.ಗಾತ್ರ_2 = ಸ್ಥಾನಾಂತರ - Origin;
  ಹಿಂತಿರುಗಿ N;
}

ಸ್ಥಿರ ತಾರ್ಕಿಕ startsWith(StringRef Name, StringRef Needle, ತಾರ್ಕಿಕ Strict,
                       std::size_t &Consummed, ಅಕ್ಷರ &ಹಿಂದಿನ_Char_ಒಳ_Name,
                       ತಾರ್ಕಿಕ Is_ಪೂರ್ವಪ್ರತ್ಯಯ = ಸುಳ್ಳು) {

  Consummed = 0;
  ಆದರೆ (Strict) {
    ಆದರೆ (!Name.starts_with(Needle))
      ಹಿಂತಿರುಗಿ ಸುಳ್ಳು;
    Consummed = Needle.ಗಾತ್ರ_3();
    ಹಿಂತಿರುಗಿ ನಿಜ;
  }
  ಆದರೆ (Needle.ಖಾಲಿಗಳು())
    ಹಿಂತಿರುಗಿ ನಿಜ;

  ಸ್ವಯಂಚಾಲಿತ NamePos = Name.begin();
  ಸ್ವಯಂಚಾಲಿತ NeedlePos = Needle.begin();

  ಅಕ್ಷರ ಹಿಂದಿನ_Char_ಒಳ_Name_Origin = ಹಿಂದಿನ_Char_ಒಳ_Name;
  ಅಕ್ಷರ ಹಿಂದಿನ_Char_ಒಳ_Needle = *Needle.begin();
  ಸ್ವಯಂಚಾಲಿತ IgnoreSpaces = [](ಸ್ವಯಂಚಾಲಿತ It, ಸ್ವಯಂಚಾಲಿತ ಕೊನೆ_2, ಅಕ್ಷರ &ಹಿಂದಿನ_Char,
                         ತಾರ್ಕಿಕ Is_ಪೂರ್ವಪ್ರತ್ಯಯ = ಸುಳ್ಳು) {
    ಆಗ (It != ಕೊನೆ_2) {
      ಸ್ಥಿರಾಂಕ ಸ್ವಯಂಚಾಲಿತ ಮುಂದಿನ_2 = std::ಮುಂದಿನ(It);
      // Ignore spaces, underscore, medial hyphens
      // The generator ensures a needle never ends (or starts) by a medial
      // hyphen https://unicode.org/reports/tr44/#UAX44-LM2.
      ತಾರ್ಕಿಕ Ignore =
          *It == ' ' || *It == '_' ||
          (*It == '-' && isAlnum(ಹಿಂದಿನ_Char) &&
           ((ಮುಂದಿನ_2 != ಕೊನೆ_2 && isAlnum(*ಮುಂದಿನ_2)) || (ಮುಂದಿನ_2 == ಕೊನೆ_2 && Is_ಪೂರ್ವಪ್ರತ್ಯಯ)));
      ಹಿಂದಿನ_Char = *It;
      ಆದರೆ (!Ignore)
        ನಿಲ್ಲಿಸು;
      ++It;
    }
    ಹಿಂತಿರುಗಿ It;
  };

  ಆಗ (ನಿಜ) {
    NamePos = IgnoreSpaces(NamePos, Name.ಕೊನೆ(), ಹಿಂದಿನ_Char_ಒಳ_Name);
    NeedlePos =
        IgnoreSpaces(NeedlePos, Needle.ಕೊನೆ(), ಹಿಂದಿನ_Char_ಒಳ_Needle, Is_ಪೂರ್ವಪ್ರತ್ಯಯ);
    ಆದರೆ (NeedlePos == Needle.ಕೊನೆ())
      ನಿಲ್ಲಿಸು;
    ಆದರೆ (NamePos == Name.ಕೊನೆ())
      ನಿಲ್ಲಿಸು;
    ಆದರೆ (ಗಮ್ಯ_ಊರ್ಧ್ವ(*NeedlePos) != ಗಮ್ಯ_ಊರ್ಧ್ವ(*NamePos))
      ನಿಲ್ಲಿಸು;
    NeedlePos++;
    NamePos++;
  }
  Consummed = std::ದೂರ(Name.begin(), NamePos);
  ಆದರೆ (NeedlePos != Needle.ಕೊನೆ()) {
    ಹಿಂದಿನ_Char_ಒಳ_Name = ಹಿಂದಿನ_Char_ಒಳ_Name_Origin;
  }
  ಹಿಂತಿರುಗಿ NeedlePos == Needle.ಕೊನೆ();
}

ಸ್ಥಿರ std::tuple<ಗಂಟು, ತಾರ್ಕಿಕ, uint32_t>
compare_ಗಂಟು(uint32_t ಸ್ಥಾನಾಂತರ, StringRef Name, ತಾರ್ಕಿಕ Strict,
            ಅಕ್ಷರ ಹಿಂದಿನ_Char_ಒಳ_Name, ಬಫರ್_Type &ಬಫರ್,
            ಸ್ಥಿರಾಂಕ ಗಂಟು *ಪೋಷಕ_2 = ಶೂನ್ಯಸೂಚಕ) {
  ಗಂಟು N = read_ಗಂಟು(ಸ್ಥಾನಾಂತರ, ಪೋಷಕ_2);
  std::size_t Consummed = 0;
  ತಾರ್ಕಿಕ Does_ಆರಂಭ_With = N.Is_ಬೇರು || startsWith(Name, N.Name, Strict, Consummed,
                                              ಹಿಂದಿನ_Char_ಒಳ_Name);
  ಆದರೆ (!Does_ಆರಂಭ_With)
    ಹಿಂತಿರುಗಿ std::make_tuple(N, ಸುಳ್ಳು, 0);

  ಆದರೆ (Name.ಗಾತ್ರ_3() - Consummed == 0 && N.ಮೌಲ್ಯ != 0xFFFFFFFF)
    ಹಿಂತಿರುಗಿ std::make_tuple(N, ನಿಜ, N.ಮೌಲ್ಯ);

  ಆದರೆ (N.hasChildren()) {
    uint32_t ಮಗು_ಸ್ಥಾನಾಂತರ = N.Children_ಸ್ಥಾನಾಂತರ;
    ಗಾಗಿ (;;) {
      ಗಂಟು C;
      ತಾರ್ಕಿಕ Matches;
      uint32_t ಮೌಲ್ಯ;
      std::tie(C, Matches, ಮೌಲ್ಯ) =
          compare_ಗಂಟು(ಮಗು_ಸ್ಥಾನಾಂತರ, Name.substr(Consummed), Strict,
                      ಹಿಂದಿನ_Char_ಒಳ_Name, ಬಫರ್, &N);
      ಆದರೆ (Matches) {
        std::ಹಿಮ್ಮುಖ_copy(C.Name.begin(), C.Name.ಕೊನೆ(),
                          std::ಹಿಂಭಾಗ_inserter(ಬಫರ್));
        ಹಿಂತಿರುಗಿ std::make_tuple(N, ನಿಜ, ಮೌಲ್ಯ);
      }
      ಮಗು_ಸ್ಥಾನಾಂತರ += C.ಗಾತ್ರ_2;
      ಆದರೆ (!C.HasSibling)
        ನಿಲ್ಲಿಸು;
    }
  }
  ಹಿಂತಿರುಗಿ std::make_tuple(N, ಸುಳ್ಳು, 0);
}

ಸ್ಥಿರ std::tuple<ಗಂಟು, ತಾರ್ಕಿಕ, uint32_t>
compare_ಗಂಟು(uint32_t ಸ್ಥಾನಾಂತರ, StringRef Name, ತಾರ್ಕಿಕ Strict, ಬಫರ್_Type &ಬಫರ್) {
  ಹಿಂತಿರುಗಿ compare_ಗಂಟು(ಸ್ಥಾನಾಂತರ, Name, Strict, 0, ಬಫರ್);
}

// clang-format off
constexpr ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಸ್ಥಿರಾಂಕ HangulSyllables[][3] = {
    { "G",  "A",   ""   },
    { "GG", "AE",  "G"  },
    { "N",  "YA",  "GG" },
    { "D",  "YAE", "GS" },
    { "DD", "EO",  "N", },
    { "R",  "E",   "NJ" },
    { "M",  "YEO", "NH" },
    { "B",  "YE",  "D"  },
    { "BB", "O",   "L"  },
    { "S",  "WA",  "LG" },
    { "SS", "WAE", "LM" },
    { "",   "OE",  "LB" },
    { "J",  "YO",  "LS" },
    { "JJ", "U",   "LT" },
    { "C",  "WEO", "LP" },
    { "K",  "WE",  "LH" },
    { "T",  "WI",  "M"  },
    { "P",  "YU",  "B"  },
    { "H",  "EU",  "BS" },
    { 0,    "YI",  "S"  },
    { 0,    "I",   "SS" },
    { 0,    0,     "NG" },
    { 0,    0,     "J"  },
    { 0,    0,     "C"  },
    { 0,    0,     "K"  },
    { 0,    0,     "T"  },
    { 0,    0,     "P"  },
    { 0,    0,     "H"  }
    };
// clang-format on

// Unicode 18.0
// 3.12 Conjoining Jamo Behavior Common constants
constexpr char32_t SBase = 0xAC00;
constexpr uint32_t LCount = 19;
constexpr uint32_t VCount = 21;
constexpr uint32_t TCount = 28;

ಸ್ಥಿರ std::size_t ಕಂಡುಹಿಡಿ_Syllable(StringRef Name, ತಾರ್ಕಿಕ Strict,
                                ಅಕ್ಷರ &ಹಿಂದಿನ_ಒಳ_Name, ಪೂರ್ಣಾಂಕ &Pos, ಪೂರ್ಣಾಂಕ ಸ್ತಂಭ) {
  ಪ್ರತಿಜ್ಞಾ(ಸ್ತಂಭ == 0 || ಸ್ತಂಭ == 1 || ಸ್ತಂಭ == 2);
  ಸ್ಥಿರ std::size_t ಎಣಿಕೆ_ಪ್ರತಿ_ಸ್ತಂಭ[] = {LCount, VCount, TCount};
  ಪೂರ್ಣಾಂಕ Len = -1;
  ಪೂರ್ಣಾಂಕ Prev = ಹಿಂದಿನ_ಒಳ_Name;
  ಗಾಗಿ (std::size_t I = 0; I < ಎಣಿಕೆ_ಪ್ರತಿ_ಸ್ತಂಭ[ಸ್ತಂಭ]; I++) {
    StringRef Syllable(HangulSyllables[I][ಸ್ತಂಭ]);
    ಆದರೆ (ಪೂರ್ಣಾಂಕ(Syllable.ಗಾತ್ರ_3()) <= Len)
      ಮುಂದುವರಿ;
    std::size_t Consummed = 0;
    ಅಕ್ಷರ ಹಿಂದಿನ_ಒಳ_Name_Copy = ಹಿಂದಿನ_ಒಳ_Name;
    ತಾರ್ಕಿಕ Does_ಆರಂಭ_With =
        startsWith(Name, Syllable, Strict, Consummed, ಹಿಂದಿನ_ಒಳ_Name_Copy);
    ಆದರೆ (!Does_ಆರಂಭ_With)
      ಮುಂದುವರಿ;
    Len = Consummed;
    Pos = I;
    Prev = ಹಿಂದಿನ_ಒಳ_Name_Copy;
  }
  ಆದರೆ (Len == -1)
    ಹಿಂತಿರುಗಿ 0;
  ಹಿಂದಿನ_ಒಳ_Name = Prev;
  ಹಿಂತಿರುಗಿ size_t(Len);
}

ಸ್ಥಿರ std::optional<char32_t>
name_ಗಮ್ಯ_Hangul_ಸಂಕೇತ_ಬಿಂದು(StringRef Name, ತಾರ್ಕಿಕ Strict, ಬಫರ್_Type &ಬಫರ್) {
  ಬಫರ್.clear();
  // Hangul Syllable Decomposition
  std::size_t Consummed = 0;
  ಅಕ್ಷರ Name_ಆರಂಭ = 0;
  ತಾರ್ಕಿಕ Does_ಆರಂಭ_With =
      startsWith(Name, "HANGUL SYLLABLE ", Strict, Consummed, Name_ಆರಂಭ);
  ಆದರೆ (!Does_ಆರಂಭ_With)
    ಹಿಂತಿರುಗಿ std::nullopt;
  Name = Name.substr(Consummed);
  ಪೂರ್ಣಾಂಕ L = -1, V = -1, T = -1;
  Name = Name.substr(ಕಂಡುಹಿಡಿ_Syllable(Name, Strict, Name_ಆರಂಭ, L, 0));
  Name = Name.substr(ಕಂಡುಹಿಡಿ_Syllable(Name, Strict, Name_ಆರಂಭ, V, 1));
  Name = Name.substr(ಕಂಡುಹಿಡಿ_Syllable(Name, Strict, Name_ಆರಂಭ, T, 2));
  ಆದರೆ (L != -1 && V != -1 && T != -1 && Name.ಖಾಲಿಗಳು()) {
    ಆದರೆ (!Strict) {
      ಬಫರ್.append("HANGUL SYLLABLE ");
      ಆದರೆ (L != -1)
        ಬಫರ್.append(HangulSyllables[L][0]);
      ಆದರೆ (V != -1)
        ಬಫರ್.append(HangulSyllables[V][1]);
      ಆದರೆ (T != -1)
        ಬಫರ್.append(HangulSyllables[T][2]);
    }
    ಹಿಂತಿರುಗಿ SBase + (std::uint32_t(L) * VCount + std::uint32_t(V)) * TCount +
           std::uint32_t(T);
  }
  // Otherwise, it's an illegal syllable name.
  ಹಿಂತಿರುಗಿ std::nullopt;
}

ರಚನೆ Generated_ಹೆಸರುಗಳು_ದತ್ತಾಂಶ {
  StringRef ಪೂರ್ವಪ್ರತ್ಯಯ;
  uint32_t ಆರಂಭ;
  uint32_t ಕೊನೆ_2;
};

// Unicode 18.0 Name Derivation Rule Prefix Strings (extracted/DerivedName.txt)
ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ Generated_ಹೆಸರುಗಳು_ದತ್ತಾಂಶ Generated_ಹೆಸರುಗಳು_ದತ್ತಾಂಶ_ಕೋಷ್ಟಕ[] = {
    {"CJK UNIFIED IDEOGRAPH-", 0x3400, 0x4DBF},
    {"CJK UNIFIED IDEOGRAPH-", 0x4E00, 0x9FFF},
    {"CJK UNIFIED IDEOGRAPH-", 0x20000, 0x2A6DF},
    {"CJK UNIFIED IDEOGRAPH-", 0x2A700, 0x2B73F},
    {"CJK UNIFIED IDEOGRAPH-", 0x2B740, 0x2B81E},
    {"CJK UNIFIED IDEOGRAPH-", 0x2B820, 0x2CEAD},
    {"CJK UNIFIED IDEOGRAPH-", 0x2CEB0, 0x2EBE0},
    {"CJK UNIFIED IDEOGRAPH-", 0x2EBF0, 0x2EE5D},
    {"CJK UNIFIED IDEOGRAPH-", 0x30000, 0x3134A},
    {"CJK UNIFIED IDEOGRAPH-", 0x31350, 0x323AF},
    {"CJK UNIFIED IDEOGRAPH-", 0x323B0, 0x33479},
    {"EGYPTIAN HIEROGLYPH-", 0x13460, 0x143FA},
    {"TANGUT IDEOGRAPH-", 0x17000, 0x187FF},
    {"TANGUT IDEOGRAPH-", 0x18D00, 0x18D20},
    {"KHITAN SMALL SCRIPT CHARACTER-", 0x18B00, 0x18CDA},
    {"KHITAN SMALL SCRIPT CHARACTER-", 0x18CFF, 0x18CFF},
    {"JURCHEN CHARACTER-", 0x18E00, 0x19191},
    {"NUSHU CHARACTER-", 0x1B170, 0x1B2FB},
    {"SMALL SEAL CHARACTER-", 0x3D000, 0x3FC3F},
    {"CJK COMPATIBILITY IDEOGRAPH-", 0xF900, 0xFA6D},
    {"CJK COMPATIBILITY IDEOGRAPH-", 0xFA70, 0xFAD9},
    {"CJK COMPATIBILITY IDEOGRAPH-", 0x2F800, 0x2FA1D},
};

ಸ್ಥಿರ std::optional<char32_t>
name_ಗಮ್ಯ_Generated_ಸಂಕೇತ_ಬಿಂದು(StringRef Name, ತಾರ್ಕಿಕ Strict, ಬಫರ್_Type &ಬಫರ್) {
  ಗಾಗಿ (ಸ್ವಯಂಚಾಲಿತ &&ಅಂಶ : Generated_ಹೆಸರುಗಳು_ದತ್ತಾಂಶ_ಕೋಷ್ಟಕ) {
    ಬಫರ್.clear();
    std::size_t Consummed = 0;
    ಅಕ್ಷರ Name_ಆರಂಭ = 0;
    ತಾರ್ಕಿಕ Does_ಆರಂಭ_With = startsWith(Name, ಅಂಶ.ಪೂರ್ವಪ್ರತ್ಯಯ, Strict, Consummed,
                                    Name_ಆರಂಭ, /*IsPrefix=*/ನಿಜ);
    ಆದರೆ (!Does_ಆರಂಭ_With)
      ಮುಂದುವರಿ;
    ಸ್ವಯಂಚಾಲಿತ ಸಂಖ್ಯೆ = Name.substr(Consummed);
    ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ ದೀರ್ಘ V = 0;
    // Be consistent about mandating upper casing.
    ಆದರೆ (Strict &&
        llvm::any_ದ(ಸಂಖ್ಯೆ, [](ಅಕ್ಷರ C) { ಹಿಂತಿರುಗಿ C >= 'a' && C <= 'f'; }))
      ಹಿಂತಿರುಗಿ {};
    ಆದರೆ (getAsUnsignedInteger(ಸಂಖ್ಯೆ, 16, V) || V < ಅಂಶ.ಆರಂಭ || V > ಅಂಶ.ಕೊನೆ_2)
      ಮುಂದುವರಿ;
    ಆದರೆ (!Strict) {
      ಬಫರ್.append(ಅಂಶ.ಪೂರ್ವಪ್ರತ್ಯಯ);
      ಬಫರ್.append(utohexstr(V, ನಿಜ));
    }
    ಹಿಂತಿರುಗಿ V;
  }
  ಹಿಂತಿರುಗಿ std::nullopt;
}

ಸ್ಥಿರ std::optional<char32_t> name_ಗಮ್ಯ_Codepoint(StringRef Name, ತಾರ್ಕಿಕ Strict,
                                               ಬಫರ್_Type &ಬಫರ್) {
  ಆದರೆ (Name.ಖಾಲಿಗಳು())
    ಹಿಂತಿರುಗಿ std::nullopt;

  std::optional<char32_t> Res = name_ಗಮ್ಯ_Hangul_ಸಂಕೇತ_ಬಿಂದು(Name, Strict, ಬಫರ್);
  ಆದರೆ (!Res)
    Res = name_ಗಮ್ಯ_Generated_ಸಂಕೇತ_ಬಿಂದು(Name, Strict, ಬಫರ್);
  ಆದರೆ (Res)
    ಹಿಂತಿರುಗಿ *Res;

  ಬಫರ್.clear();
  ಗಂಟು ಗಂಟು;
  ತಾರ್ಕಿಕ Matches;
  uint32_t ಮೌಲ್ಯ;
  std::tie(ಗಂಟು, Matches, ಮೌಲ್ಯ) = compare_ಗಂಟು(0, Name, Strict, ಬಫರ್);
  ಆದರೆ (Matches) {
    std::ಹಿಮ್ಮುಖ(ಬಫರ್.begin(), ಬಫರ್.ಕೊನೆ());
    // UAX44-LM2. Ignore case, whitespace, underscore ('_'), and all medial
    // hyphens except the hyphen in U+1180 HANGUL JUNGSEONG O-E.
    ಆದರೆ (!Strict && ಮೌಲ್ಯ == 0x116c && Name.ಒಳಗೊಂಡಿದೆ_insensitive("O-E")) {
      ಬಫರ್ = "HANGUL JUNGSEONG O-E";
      ಮೌಲ್ಯ = 0x1180;
    }
    ಹಿಂತಿರುಗಿ ಮೌಲ್ಯ;
  }
  ಹಿಂತಿರುಗಿ std::nullopt;
}

std::optional<char32_t> name_ಗಮ್ಯ_Codepoint_Strict(StringRef Name) {

  ಬಫರ್_Type ಬಫರ್;
  ಸ್ವಯಂಚಾಲಿತ Opt = name_ಗಮ್ಯ_Codepoint(Name, ನಿಜ, ಬಫರ್);
  ಹಿಂತಿರುಗಿ Opt;
}

std::optional<Loose_Matching_ಫಲಿತಾಂಶ>
name_ಗಮ್ಯ_Codepoint_Loose_Matching(StringRef Name) {
  ಬಫರ್_Type ಬಫರ್;
  ಸ್ವಯಂಚಾಲಿತ Opt = name_ಗಮ್ಯ_Codepoint(Name, ಸುಳ್ಳು, ಬಫರ್);
  ಆದರೆ (!Opt)
    ಹಿಂತಿರುಗಿ std::nullopt;
  ಹಿಂತಿರುಗಿ Loose_Matching_ಫಲಿತಾಂಶ{*Opt, ಬಫರ್};
}

// Find the unicode character whose editing distance to Pattern
// is shortest, using the Wagner–Fischer algorithm.
llvm::Small_ಸದಿಶ<ಹೊಂದಿಕೆ_For_Codepoint_Name>
ಹತ್ತಿರದ_Matches_For_Codepoint_Name(StringRef ನಮೂನೆ, std::size_t Max_Matches_ಎಣಿಕೆ) {
  // We maintain a fixed size vector of matches,
  // sorted by distance
  // The worst match (with the biggest distance) are discarded when new elements
  // are added.
  std::size_t ಅತಿದೊಡ್ಡ_Edit_ದೂರ = 0;
  llvm::Small_ಸದಿಶ<ಹೊಂದಿಕೆ_For_Codepoint_Name> Matches;
  Matches.reserve(Max_Matches_ಎಣಿಕೆ + 1);

  ಸ್ವಯಂಚಾಲಿತ ಸೇರಿಸು_2 = [&](ಸ್ಥಿರಾಂಕ ಗಂಟು &ಗಂಟು, uint32_t ದೂರ_2,
                    char32_t ಮೌಲ್ಯ) -> ತಾರ್ಕಿಕ {
    ಆದರೆ (ದೂರ_2 > ಅತಿದೊಡ್ಡ_Edit_ದೂರ) {
      ಆದರೆ (Matches.ಗಾತ್ರ_3() == Max_Matches_ಎಣಿಕೆ)
        ಹಿಂತಿರುಗಿ ಸುಳ್ಳು;
      ಅತಿದೊಡ್ಡ_Edit_ದೂರ = ದೂರ_2;
    }
    // To avoid allocations, the creation of the name is delayed
    // as much as possible.
    std::string Name;
    ಸ್ವಯಂಚಾಲಿತ GetName = [&] {
      ಆದರೆ (Name.ಖಾಲಿಗಳು())
        Name = ಗಂಟು.ತುಂಬಿದ_Name();
      ಹಿಂತಿರುಗಿ Name;
    };

    ಸ್ವಯಂಚಾಲಿತ It = llvm::ಅಧೋ_bound(
        Matches, ದೂರ_2,
        [&](ಸ್ಥಿರಾಂಕ ಹೊಂದಿಕೆ_For_Codepoint_Name &a, std::size_t ದೂರ_2) {
          ಆದರೆ (ದೂರ_2 == a.ದೂರ_2)
            ಹಿಂತಿರುಗಿ a.Name < GetName();
          ಹಿಂತಿರುಗಿ a.ದೂರ_2 < ದೂರ_2;
        });
    ಆದರೆ (It == Matches.ಕೊನೆ() && Matches.ಗಾತ್ರ_3() == Max_Matches_ಎಣಿಕೆ)
      ಹಿಂತಿರುಗಿ ಸುಳ್ಳು;

    ಹೊಂದಿಕೆ_For_Codepoint_Name M{GetName(), ದೂರ_2, ಮೌಲ್ಯ};
    Matches.ಸೇರಿಸು(It, std::move(M));
    ಆದರೆ (Matches.ಗಾತ್ರ_3() > Max_Matches_ಎಣಿಕೆ)
      Matches.ತೆಗೆ_ಹಿಂಭಾಗ();
    ಹಿಂತಿರುಗಿ ನಿಜ;
  };

  // We ignore case, space, hyphens, etc,
  // in both the search pattern and the prospective names.
  ಸ್ವಯಂಚಾಲಿತ Normalize = [](StringRef Name) {
    std::string ಹೊರಹೋಗು;
    ಹೊರಹೋಗು.reserve(Name.ಗಾತ್ರ_3());
    ಗಾಗಿ (ಅಕ್ಷರ C : Name) {
      ಆದರೆ (isAlnum(C))
        ಹೊರಹೋಗು.ತಳ್ಳು_ಹಿಂಭಾಗ(ಗಮ್ಯ_ಊರ್ಧ್ವ(C));
    }
    ಹಿಂತಿರುಗಿ ಹೊರಹೋಗು;
  };
  std::string NormalizedName = Normalize(ನಮೂನೆ);

  // Allocate a matrix big enough for longest names.
  ಸ್ಥಿರಾಂಕ std::size_t Columns =
      std::min(NormalizedName.ಗಾತ್ರ_3(), Unicode_Name_ಗಮ್ಯ_Codepoint_ಅತಿದೊಡ್ಡ_Name_ಗಾತ್ರ) +
      1;

  [[maybe_ಬಳಸದ]] ಸ್ಥಿರ std::size_t ಅಡ್ಡಸಾಲುಗಳು_2 =
      Unicode_Name_ಗಮ್ಯ_Codepoint_ಅತಿದೊಡ್ಡ_Name_ಗಾತ್ರ + 1;

  std::ಸದಿಶ<ಅಕ್ಷರ> Distances(
      Columns * (Unicode_Name_ಗಮ್ಯ_Codepoint_ಅತಿದೊಡ್ಡ_Name_ಗಾತ್ರ + 1), 0);

  ಸ್ವಯಂಚಾಲಿತ Get = [&Distances, Columns](size_t ಸ್ತಂಭ, std::size_t ಅಡ್ಡಸಾಲು) -> ಅಕ್ಷರ & {
    ಪ್ರತಿಜ್ಞಾ(ಸ್ತಂಭ < Columns);
    ಪ್ರತಿಜ್ಞಾ(ಅಡ್ಡಸಾಲು < ಅಡ್ಡಸಾಲುಗಳು_2);
    ಹಿಂತಿರುಗಿ Distances[ಅಡ್ಡಸಾಲು * Columns + ಸ್ತಂಭ];
  };

  ಗಾಗಿ (std::size_t I = 0; I < Columns; I++)
    Get(I, 0) = I;

  // Visit the childrens,
  // Filling (and overriding) the matrix for the name fragment of each node
  // iteratively. CompleteName is used to collect the actual name of potential
  // match, respecting case and spacing.
  ಸ್ವಯಂಚಾಲಿತ ಭೇಟಿ_ಗಂಟು = [&](ಸ್ಥಿರಾಂಕ ಗಂಟು &N, std::size_t ಅಡ್ಡಸಾಲು,
                       ಸ್ವಯಂಚಾಲಿತ &ಭೇಟಿ_ಗಂಟು) -> ಶೂನ್ಯ {
    std::size_t J = 0;
    ಗಾಗಿ (; J < N.Name.ಗಾತ್ರ_3(); J++) {
      ಆದರೆ (!isAlnum(N.Name[J]))
        ಮುಂದುವರಿ;

      Get(0, ಅಡ್ಡಸಾಲು) = ಅಡ್ಡಸಾಲು;

      ಗಾಗಿ (std::size_t I = 1; I < Columns; I++) {
        ಸ್ಥಿರಾಂಕ ಪೂರ್ಣಾಂಕ Delete = Get(I - 1, ಅಡ್ಡಸಾಲು) + 1;
        ಸ್ಥಿರಾಂಕ ಪೂರ್ಣಾಂಕ ಸೇರಿಸು_2 = Get(I, ಅಡ್ಡಸಾಲು - 1) + 1;

        ಸ್ಥಿರಾಂಕ ಪೂರ್ಣಾಂಕ Replace =
            Get(I - 1, ಅಡ್ಡಸಾಲು - 1) + (NormalizedName[I - 1] != N.Name[J] ? 1 : 0);

        Get(I, ಅಡ್ಡಸಾಲು) = std::min({ಸೇರಿಸು_2, Delete, Replace});
      }

      ಅಡ್ಡಸಾಲು++;
    }

    ಚಿಹ್ನೆರಹಿತ ವೆಚ್ಚ = Get(Columns - 1, ಅಡ್ಡಸಾಲು - 1);
    ಆದರೆ (N.ಮೌಲ್ಯ != 0xFFFFFFFF) {
      ಸೇರಿಸು_2(N, ವೆಚ್ಚ, N.ಮೌಲ್ಯ);
    }

    ಆದರೆ (N.hasChildren()) {
      ಸ್ವಯಂಚಾಲಿತ ಮಗು_ಸ್ಥಾನಾಂತರ = N.Children_ಸ್ಥಾನಾಂತರ;
      ಗಾಗಿ (;;) {
        ಗಂಟು C = read_ಗಂಟು(ಮಗು_ಸ್ಥಾನಾಂತರ, &N);
        ಮಗು_ಸ್ಥಾನಾಂತರ += C.ಗಾತ್ರ_2;
        ಆದರೆ (!C.isValid())
          ನಿಲ್ಲಿಸು;
        ಭೇಟಿ_ಗಂಟು(C, ಅಡ್ಡಸಾಲು, ಭೇಟಿ_ಗಂಟು);
        ಆದರೆ (!C.HasSibling)
          ನಿಲ್ಲಿಸು;
      }
    }
  };

  ಗಂಟು ಬೇರು_2 = create_ಬೇರು();
  ಭೇಟಿ_ಗಂಟು(ಬೇರು_2, 1, ಭೇಟಿ_ಗಂಟು);
  ಹಿಂತಿರುಗಿ Matches;
}

} // namespace unicode

} // namespace sys
} // namespace llvm
