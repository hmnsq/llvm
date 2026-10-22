//===- WithColor.cpp ------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#অন্তর্ভাৱ "llvm/Support/WithColor.h"

#অন্তর্ভাৱ "DebugOptions.h"

#অন্তর্ভাৱ "llvm/Support/CommandLine.h"
#অন্তর্ভাৱ "llvm/Support/Error.h"
#অন্তর্ভাৱ "llvm/Support/ManagedStatic.h"

using নামাকাশ llvm;

cl::OptionCategory &llvm::getColorCategory() {
  স্থির cl::OptionCategory ColorCategory("Color Options");
  ঘূৰাই ColorCategory;
}
নামাকাশ {
সংরচনা CreateUseColor {
  স্থির শূন্য *call() {
    ঘূৰাই নূতন cl::opt<cl::boolOrDefault>(
        "color", cl::cat(getColorCategory()),
        cl::desc("Use colors in output (default=autodetect)"),
        cl::init(cl::boolOrDefault::BOU_UNSET));
  }
};
} // namespace
স্থির ManagedStatic<cl::opt<cl::boolOrDefault>, CreateUseColor> UseColor;
শূন্য llvm::initWithColorOptions() { *UseColor; }

স্থির তার্কিক DefaultAutoDetectFunction(ধ্রুৱ raw_ostream &OS) {
  ঘূৰাই *UseColor == cl::boolOrDefault::BOU_UNSET
             ? OS.has_colors()
             : *UseColor == cl::boolOrDefault::BOU_TRUE;
}

WithColor::AutoDetectFunctionType WithColor::AutoDetectFunction =
    DefaultAutoDetectFunction;

WithColor::WithColor(raw_ostream &OS, HighlightColor Color, ColorMode Mode)
    : OS(OS), Mode(Mode) {
  // Detect color from terminal type unless the user passed the --color option.
  যদি (colorsEnabled()) {
    ৱিকল্প (Color) {
    পক্ষ HighlightColor::Address:
      OS.changeColor(raw_ostream::YELLOW);
      ভঙ্গ;
    পক্ষ HighlightColor::String:
      OS.changeColor(raw_ostream::GREEN);
      ভঙ্গ;
    পক্ষ HighlightColor::Tag:
      OS.changeColor(raw_ostream::BLUE);
      ভঙ্গ;
    পক্ষ HighlightColor::Attribute:
      OS.changeColor(raw_ostream::CYAN);
      ভঙ্গ;
    পক্ষ HighlightColor::Enumerator:
      OS.changeColor(raw_ostream::MAGENTA);
      ভঙ্গ;
    পক্ষ HighlightColor::Macro:
      OS.changeColor(raw_ostream::RED);
      ভঙ্গ;
    পক্ষ HighlightColor::Error:
      OS.changeColor(raw_ostream::RED, সঁচা);
      ভঙ্গ;
    পক্ষ HighlightColor::Warning:
      OS.changeColor(raw_ostream::MAGENTA, সঁচা);
      ভঙ্গ;
    পক্ষ HighlightColor::Note:
      OS.changeColor(raw_ostream::BLACK, সঁচা);
      ভঙ্গ;
    পক্ষ HighlightColor::Remark:
      OS.changeColor(raw_ostream::BLUE, সঁচা);
      ভঙ্গ;
    }
  }
}

raw_ostream &WithColor::error() { ঘূৰাই error(errs()); }

raw_ostream &WithColor::warning() { ঘূৰাই warning(errs()); }

raw_ostream &WithColor::note() { ঘূৰাই note(errs()); }

raw_ostream &WithColor::remark() { ঘূৰাই remark(errs()); }

raw_ostream &WithColor::error(raw_ostream &OS, StringRef Prefix,
                              তার্কিক DisableColors) {
  যদি (!Prefix.empty())
    OS << Prefix << ": ";
  ঘূৰাই WithColor(OS, HighlightColor::Error,
                   DisableColors ? ColorMode::Disable : ColorMode::Auto)
             .get()
         << "error: ";
}

raw_ostream &WithColor::warning(raw_ostream &OS, StringRef Prefix,
                                তার্কিক DisableColors) {
  যদি (!Prefix.empty())
    OS << Prefix << ": ";
  ঘূৰাই WithColor(OS, HighlightColor::Warning,
                   DisableColors ? ColorMode::Disable : ColorMode::Auto)
             .get()
         << "warning: ";
}

raw_ostream &WithColor::note(raw_ostream &OS, StringRef Prefix,
                             তার্কিক DisableColors) {
  যদি (!Prefix.empty())
    OS << Prefix << ": ";
  ঘূৰাই WithColor(OS, HighlightColor::Note,
                   DisableColors ? ColorMode::Disable : ColorMode::Auto)
             .get()
         << "note: ";
}

raw_ostream &WithColor::remark(raw_ostream &OS, StringRef Prefix,
                               তার্কিক DisableColors) {
  যদি (!Prefix.empty())
    OS << Prefix << ": ";
  ঘূৰাই WithColor(OS, HighlightColor::Remark,
                   DisableColors ? ColorMode::Disable : ColorMode::Auto)
             .get()
         << "remark: ";
}

তার্কিক WithColor::colorsEnabled() {
  ৱিকল্প (Mode) {
  পক্ষ ColorMode::Enable:
    ঘূৰাই সঁচা;
  পক্ষ ColorMode::Disable:
    ঘূৰাই মিছা;
  পক্ষ ColorMode::Auto:
    ঘূৰাই AutoDetectFunction(OS);
  }
  llvm_unreachable("All cases handled above.");
}

WithColor &WithColor::changeColor(raw_ostream::Colors Color, তার্কিক Bold,
                                  তার্কিক BG) {
  যদি (colorsEnabled())
    OS.changeColor(Color, Bold, BG);
  ঘূৰাই *এতত্;
}

WithColor &WithColor::resetColor() {
  যদি (colorsEnabled())
    OS.resetColor();
  ঘূৰাই *এতত্;
}

WithColor::~WithColor() { resetColor(); }

শূন্য WithColor::defaultErrorHandler(Error Err) {
  handleAllErrors(std::move(Err), [](ErrorInfoBase &Info) {
    WithColor::error() << Info.message() << '\n';
  });
}

শূন্য WithColor::defaultWarningHandler(Error Warning) {
  handleAllErrors(std::move(Warning), [](ErrorInfoBase &Info) {
    WithColor::warning() << Info.message() << '\n';
  });
}

WithColor::AutoDetectFunctionType WithColor::defaultAutoDetectFunction() {
  ঘূৰাই DefaultAutoDetectFunction;
}

শূন্য WithColor::setAutoDetectFunction(
    AutoDetectFunctionType NewAutoDetectFunction) {
  AutoDetectFunction = NewAutoDetectFunction;
}
