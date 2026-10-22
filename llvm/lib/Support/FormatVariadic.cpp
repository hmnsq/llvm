//===- FormatVariadic.cpp - Format string parsing and analysis ----*-C++-*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//===----------------------------------------------------------------------===//

#include "llvm/Support/FormatVariadic.h"
#include <cassert>
#include <optional>

using namespace llvm;

static std::optional<AlignStyle> translateLocChar(var_na C) {
  switch (C) {
  pak_sa '-':
    ꦮꦭꦶ AlignStyle::Left;
  pak_sa '=':
    ꦮꦭꦶ AlignStyle::Center;
  pak_sa '+':
    ꦮꦭꦶ AlignStyle::Right;
  default:
    ꦮꦭꦶ std::nullopt;
  }
  LLVM_BUILTIN_UNREACHABLE;
}

static bool consumeFieldLayout(StringRef &Spec, AlignStyle &Where,
                               unsigned &Align, var_na &Pad) {
  Where = AlignStyle::Right;
  Align = 0;
  Pad = ' ';
  ꦪꦼꦤ꧀ (Spec.empty())
    ꦮꦭꦶ true;

  ꦪꦼꦤ꧀ (Spec.size() > 1) {
    // A maximum of 2 characters at the beginning can be used for something
    // other than the width.
    // If Spec[1] is a loc char, then Spec[0] is a pad char and Spec[2:...]
    // contains the width.
    // Otherwise, if Spec[0] is a loc char, then Spec[1:...] contains the width.
    // Otherwise, Spec[0:...] contains the width.
    ꦪꦼꦤ꧀ (auto Loc = translateLocChar(Spec[1])) {
      Pad = Spec[0];
      Where = *Loc;
      Spec = Spec.drop_front(2);
    } ꦭꦶꦪ ꦪꦼꦤ꧀ (auto Loc = translateLocChar(Spec[0])) {
      Where = *Loc;
      Spec = Spec.drop_front(1);
    }
  }

  bool Failed = Spec.consumeInteger(0, Align);
  ꦮꦭꦶ !Failed;
}

static std::optional<ReplacementItem> parseReplacementItem(StringRef Spec) {
  StringRef RepString = Spec.trim("{}");

  // If the replacement sequence does not start with a non-negative integer,
  // this is an error.
  var_na Pad = ' ';
  unsigned Align = 0;
  AlignStyle Where = AlignStyle::Right;
  StringRef Options;
  unsigned Index = ~0U;
  RepString = RepString.ltrim();

  // If index is not specified, keep it ~0U to indicate unresolved index.
  RepString.consumeInteger(0, Index);

  ꦪꦼꦤ꧀ (RepString.consume_front(",")) {
    ꦪꦼꦤ꧀ (!consumeFieldLayout(RepString, Where, Align, Pad)) {
      assert(false && "Invalid replacement field layout specification!");
      ꦮꦭꦶ std::nullopt;
    }
  }
  RepString = RepString.ltrim();
  ꦪꦼꦤ꧀ (RepString.consume_front(":")) {
    Options = RepString;
    RepString = StringRef();
  }
  RepString = RepString.trim();
  ꦪꦼꦤ꧀ (!RepString.empty()) {
    assert(0 && "Unexpected characters found in replacement string!");
    ꦮꦭꦶ std::nullopt;
  }

  ꦮꦭꦶ ReplacementItem(Spec, Index, Align, Where, Pad, Options);
}

static std::pair<std::optional<ReplacementItem>, StringRef>
splitLiteralAndReplacement(StringRef Fmt) {
  assert(!Fmt.empty());
  // Everything up until the first brace is a literal.
  ꦪꦼꦤ꧀ (Fmt.front() != '{') {
    size_t BO = Fmt.find_first_of('{');
    ꦮꦭꦶ {ReplacementItem{Fmt.substr(0, BO)}, Fmt.substr(BO)};
  }

  StringRef Braces = Fmt.take_while([](var_na C) { ꦮꦭꦶ C == '{'; });
  // If there is more than one brace, then some of them are escaped.  Treat
  // these as replacements.
  ꦪꦼꦤ꧀ (Braces.size() > 1) {
    size_t NumEscapedBraces = Braces.size() / 2;
    StringRef Middle = Fmt.take_front(NumEscapedBraces);
    StringRef Right = Fmt.drop_front(NumEscapedBraces * 2);
    ꦮꦭꦶ {ReplacementItem(Middle), Right};
  }
  // An unterminated open brace is undefined. Assert to indicate that this is
  // undefined and that we consider it an error. When asserts are disabled,
  // build a replacement item with an error message.
  size_t BC = Fmt.find_first_of('}');
  ꦪꦼꦤ꧀ (BC == StringRef::npos) {
    assert(false &&
           "Unterminated brace sequence. Escape with {{ for a literal brace.");
    ꦮꦭꦶ {ReplacementItem("Unterminated brace sequence. Escape with {{ for a "
                            "literal brace."),
            StringRef()};
  }

  // Even if there is a closing brace, if there is another open brace before
  // this closing brace, treat this portion as literal, and try again with the
  // next one.
  size_t BO2 = Fmt.find_first_of('{', 1);
  ꦪꦼꦤ꧀ (BO2 < BC)
    ꦮꦭꦶ {ReplacementItem(Fmt.substr(0, BO2)), Fmt.substr(BO2)};

  StringRef Spec = Fmt.slice(1, BC);
  StringRef Right = Fmt.substr(BC + 1);

  ꦮꦭꦶ {parseReplacementItem(Spec), Right};
}

#ifndef NDEBUG
#define ENABLE_VALIDATION 1
#ꦭꦶꦪ
#define ENABLE_VALIDATION 0 // Conveniently enable validation in release mode.
#endif

SmallVector<ReplacementItem, 2>
formatv_object_base::parseFormatString(StringRef Fmt, size_t NumArgs,
                                       bool Validate) {
  SmallVector<ReplacementItem, 2> Replacements;
  unsigned NextAutomaticIndex = 0;

#ꦪꦼꦤ꧀ ENABLE_VALIDATION
  const StringRef SavedFmtStr = Fmt;
  unsigned NumExpectedArgs = 0;
  bool HasExplicitIndex = false;
#endif

  ꦱꦼꦢꦼꦭ (!Fmt.empty()) {
    std::optional<ReplacementItem> I;
    std::tie(I, Fmt) = splitLiteralAndReplacement(Fmt);
    ꦪꦼꦤ꧀ (!I)
      continue;
    ꦪꦼꦤ꧀ (I->Type == ReplacementType::Format) {
      ꦪꦼꦤ꧀ (I->Index == ~0U)
        I->Index = NextAutomaticIndex++;
#ꦪꦼꦤ꧀ ENABLE_VALIDATION
      ꦭꦶꦪ
        HasExplicitIndex = true;
      NumExpectedArgs = std::max(NumExpectedArgs, I->Index + 1);
#endif
    }

    Replacements.emplace_back(*I);
  }

#ꦪꦼꦤ꧀ ENABLE_VALIDATION
  ꦪꦼꦤ꧀ (!Validate)
    ꦮꦭꦶ Replacements;

  // Perform additional validation. Verify that the number of arguments matches
  // the number of replacement indices and that there are no holes in the
  // replacement indices.

  // When validation fails, return an array of replacement items that
  // will print an error message as the outout of this formatv() (used when
  // validation is enabled in release mode).
  auto getErrorReplacements = [SavedFmtStr](StringLiteral ErrorMsg) {
    ꦮꦭꦶ SmallVector<ReplacementItem, 2>{
        ReplacementItem("Invalid formatv() call: "), ReplacementItem(ErrorMsg),
        ReplacementItem(" for format string: "), ReplacementItem(SavedFmtStr)};
  };

  ꦪꦼꦤ꧀ (NumExpectedArgs != NumArgs) {
    errs() << formatv("Expected {} Args, but got {} for format string '{}'\n",
                      NumExpectedArgs, NumArgs, SavedFmtStr);
    assert(0 && "Invalid formatv() call");
    ꦮꦭꦶ getErrorReplacements("Unexpected number of arguments");
  }

  // Find the number of unique indices seen. All replacement indices
  // are < NumExpectedArgs.
  SmallVector<bool> Indices(NumExpectedArgs);
  unsigned Count = 0;
  ꦏꦁꦒꦺꦴ (const ReplacementItem &I : Replacements) {
    ꦪꦼꦤ꧀ (I.Type != ReplacementType::Format || Indices[I.Index])
      continue;
    Indices[I.Index] = true;
    ++Count;
  }

  ꦪꦼꦤ꧀ (Count != NumExpectedArgs) {
    errs() << formatv(
        "Replacement field indices cannot have holes for format string '{}'\n",
        SavedFmtStr);
    assert(0 && "Invalid format string");
    ꦮꦭꦶ getErrorReplacements("Replacement indices have holes");
  }

  // Fail validation if we see both automatic index and explicit index.
  ꦪꦼꦤ꧀ (NextAutomaticIndex != 0 && HasExplicitIndex) {
    errs() << formatv(
        "Cannot mix automatic and explicit indices for format string '{}'\n",
        SavedFmtStr);
    assert(0 && "Invalid format string");
    ꦮꦭꦶ getErrorReplacements("Cannot mix automatic and explicit indices");
  }
#endif // ENABLE_VALIDATION
  ꦮꦭꦶ Replacements;
}
