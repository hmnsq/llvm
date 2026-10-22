//===- WithColor.cpp ------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ଅନ୍ତର୍ଭାଵ "llvm/Support/WithColor.h"

#ଅନ୍ତର୍ଭାଵ "DebugOptions.h"

#ଅନ୍ତର୍ଭାଵ "llvm/Support/CommandLine.h"
#ଅନ୍ତର୍ଭାଵ "llvm/Support/Error.h"
#ଅନ୍ତର୍ଭାଵ "llvm/Support/ManagedStatic.h"

using ନାମାକାଶ llvm;

cl::OptionCategory &llvm::getColorCategory() {
  ସ୍ଥିର cl::OptionCategory ColorCategory("Color Options");
  ଫେରନ୍ତୁ ColorCategory;
}
ନାମାକାଶ {
ସଂରଚନା CreateUseColor {
  ସ୍ଥିର ଶୂନ୍ୟ *call() {
    ଫେରନ୍ତୁ ନୂତନ cl::opt<cl::boolOrDefault>(
        "color", cl::cat(getColorCategory()),
        cl::desc("Use colors in output (default=autodetect)"),
        cl::init(cl::boolOrDefault::BOU_UNSET));
  }
};
} // namespace
ସ୍ଥିର ManagedStatic<cl::opt<cl::boolOrDefault>, CreateUseColor> UseColor;
ଶୂନ୍ୟ llvm::initWithColorOptions() { *UseColor; }

ସ୍ଥିର ତାର୍କିକ DefaultAutoDetectFunction(ଅଚଳ raw_ostream &OS) {
  ଫେରନ୍ତୁ *UseColor == cl::boolOrDefault::BOU_UNSET
             ? OS.has_colors()
             : *UseColor == cl::boolOrDefault::BOU_TRUE;
}

WithColor::AutoDetectFunctionType WithColor::AutoDetectFunction =
    DefaultAutoDetectFunction;

WithColor::WithColor(raw_ostream &OS, HighlightColor Color, ColorMode Mode)
    : OS(OS), Mode(Mode) {
  // Detect color from terminal type unless the user passed the --color option.
  ଯଦି (colorsEnabled()) {
    ବାଛ (Color) {
    ସ୍ଥିତି HighlightColor::Address:
      OS.changeColor(raw_ostream::YELLOW);
      ଭାଙ୍ଗ;
    ସ୍ଥିତି HighlightColor::String:
      OS.changeColor(raw_ostream::GREEN);
      ଭାଙ୍ଗ;
    ସ୍ଥିତି HighlightColor::Tag:
      OS.changeColor(raw_ostream::BLUE);
      ଭାଙ୍ଗ;
    ସ୍ଥିତି HighlightColor::Attribute:
      OS.changeColor(raw_ostream::CYAN);
      ଭାଙ୍ଗ;
    ସ୍ଥିତି HighlightColor::Enumerator:
      OS.changeColor(raw_ostream::MAGENTA);
      ଭାଙ୍ଗ;
    ସ୍ଥିତି HighlightColor::Macro:
      OS.changeColor(raw_ostream::RED);
      ଭାଙ୍ଗ;
    ସ୍ଥିତି HighlightColor::Error:
      OS.changeColor(raw_ostream::RED, ସତ୍ୟ);
      ଭାଙ୍ଗ;
    ସ୍ଥିତି HighlightColor::Warning:
      OS.changeColor(raw_ostream::MAGENTA, ସତ୍ୟ);
      ଭାଙ୍ଗ;
    ସ୍ଥିତି HighlightColor::Note:
      OS.changeColor(raw_ostream::BLACK, ସତ୍ୟ);
      ଭାଙ୍ଗ;
    ସ୍ଥିତି HighlightColor::Remark:
      OS.changeColor(raw_ostream::BLUE, ସତ୍ୟ);
      ଭାଙ୍ଗ;
    }
  }
}

raw_ostream &WithColor::error() { ଫେରନ୍ତୁ error(errs()); }

raw_ostream &WithColor::warning() { ଫେରନ୍ତୁ warning(errs()); }

raw_ostream &WithColor::note() { ଫେରନ୍ତୁ note(errs()); }

raw_ostream &WithColor::remark() { ଫେରନ୍ତୁ remark(errs()); }

raw_ostream &WithColor::error(raw_ostream &OS, StringRef Prefix,
                              ତାର୍କିକ DisableColors) {
  ଯଦି (!Prefix.empty())
    OS << Prefix << ": ";
  ଫେରନ୍ତୁ WithColor(OS, HighlightColor::Error,
                   DisableColors ? ColorMode::Disable : ColorMode::Auto)
             .get()
         << "error: ";
}

raw_ostream &WithColor::warning(raw_ostream &OS, StringRef Prefix,
                                ତାର୍କିକ DisableColors) {
  ଯଦି (!Prefix.empty())
    OS << Prefix << ": ";
  ଫେରନ୍ତୁ WithColor(OS, HighlightColor::Warning,
                   DisableColors ? ColorMode::Disable : ColorMode::Auto)
             .get()
         << "warning: ";
}

raw_ostream &WithColor::note(raw_ostream &OS, StringRef Prefix,
                             ତାର୍କିକ DisableColors) {
  ଯଦି (!Prefix.empty())
    OS << Prefix << ": ";
  ଫେରନ୍ତୁ WithColor(OS, HighlightColor::Note,
                   DisableColors ? ColorMode::Disable : ColorMode::Auto)
             .get()
         << "note: ";
}

raw_ostream &WithColor::remark(raw_ostream &OS, StringRef Prefix,
                               ତାର୍କିକ DisableColors) {
  ଯଦି (!Prefix.empty())
    OS << Prefix << ": ";
  ଫେରନ୍ତୁ WithColor(OS, HighlightColor::Remark,
                   DisableColors ? ColorMode::Disable : ColorMode::Auto)
             .get()
         << "remark: ";
}

ତାର୍କିକ WithColor::colorsEnabled() {
  ବାଛ (Mode) {
  ସ୍ଥିତି ColorMode::Enable:
    ଫେରନ୍ତୁ ସତ୍ୟ;
  ସ୍ଥିତି ColorMode::Disable:
    ଫେରନ୍ତୁ ମିଥ୍ୟା;
  ସ୍ଥିତି ColorMode::Auto:
    ଫେରନ୍ତୁ AutoDetectFunction(OS);
  }
  llvm_unreachable("All cases handled above.");
}

WithColor &WithColor::changeColor(raw_ostream::Colors Color, ତାର୍କିକ Bold,
                                  ତାର୍କିକ BG) {
  ଯଦି (colorsEnabled())
    OS.changeColor(Color, Bold, BG);
  ଫେରନ୍ତୁ *ଏହି;
}

WithColor &WithColor::resetColor() {
  ଯଦି (colorsEnabled())
    OS.resetColor();
  ଫେରନ୍ତୁ *ଏହି;
}

WithColor::~WithColor() { resetColor(); }

ଶୂନ୍ୟ WithColor::defaultErrorHandler(Error Err) {
  handleAllErrors(std::move(Err), [](ErrorInfoBase &Info) {
    WithColor::error() << Info.message() << '\n';
  });
}

ଶୂନ୍ୟ WithColor::defaultWarningHandler(Error Warning) {
  handleAllErrors(std::move(Warning), [](ErrorInfoBase &Info) {
    WithColor::warning() << Info.message() << '\n';
  });
}

WithColor::AutoDetectFunctionType WithColor::defaultAutoDetectFunction() {
  ଫେରନ୍ତୁ DefaultAutoDetectFunction;
}

ଶୂନ୍ୟ WithColor::setAutoDetectFunction(
    AutoDetectFunctionType NewAutoDetectFunction) {
  AutoDetectFunction = NewAutoDetectFunction;
}
