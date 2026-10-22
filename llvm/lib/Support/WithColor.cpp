//===- WithColor.cpp ------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ಅನ್ತರ್ಭಾವ "llvm/Support/WithColor.h"

#ಅನ್ತರ್ಭಾವ "DebugOptions.h"

#ಅನ್ತರ್ಭಾವ "llvm/Support/CommandLine.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/Error.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/ManagedStatic.h"

using ನಾಮಾಕಾಶ llvm;

cl::OptionCategory &llvm::getColorCategory() {
  ಸ್ಥಿರ cl::OptionCategory ColorCategory("Color Options");
  ಪ್ರತ್ಯಾಗಮ ColorCategory;
}
ನಾಮಾಕಾಶ {
ಸಂರಚನಾ CreateUseColor {
  ಸ್ಥಿರ ಶೂನ್ಯ *call() {
    ಪ್ರತ್ಯಾಗಮ ನೂತನ cl::opt<cl::boolOrDefault>(
        "color", cl::cat(getColorCategory()),
        cl::desc("Use colors in output (default=autodetect)"),
        cl::init(cl::boolOrDefault::BOU_UNSET));
  }
};
} // namespace
ಸ್ಥಿರ ManagedStatic<cl::opt<cl::boolOrDefault>, CreateUseColor> UseColor;
ಶೂನ್ಯ llvm::initWithColorOptions() { *UseColor; }

ಸ್ಥಿರ ತಾರ್ಕಿಕ DefaultAutoDetectFunction(ಧ್ರುವ raw_ostream &OS) {
  ಪ್ರತ್ಯಾಗಮ *UseColor == cl::boolOrDefault::BOU_UNSET
             ? OS.has_colors()
             : *UseColor == cl::boolOrDefault::BOU_TRUE;
}

WithColor::AutoDetectFunctionType WithColor::AutoDetectFunction =
    DefaultAutoDetectFunction;

WithColor::WithColor(raw_ostream &OS, HighlightColor Color, ColorMode Mode)
    : OS(OS), Mode(Mode) {
  // Detect color from terminal type unless the user passed the --color option.
  ಯದಿ (colorsEnabled()) {
    ವಿಕಲ್ಪ (Color) {
    ಪಕ್ಷ HighlightColor::Address:
      OS.changeColor(raw_ostream::YELLOW);
      ಭಙ್ಗ;
    ಪಕ್ಷ HighlightColor::String:
      OS.changeColor(raw_ostream::GREEN);
      ಭಙ್ಗ;
    ಪಕ್ಷ HighlightColor::Tag:
      OS.changeColor(raw_ostream::BLUE);
      ಭಙ್ಗ;
    ಪಕ್ಷ HighlightColor::Attribute:
      OS.changeColor(raw_ostream::CYAN);
      ಭಙ್ಗ;
    ಪಕ್ಷ HighlightColor::Enumerator:
      OS.changeColor(raw_ostream::MAGENTA);
      ಭಙ್ಗ;
    ಪಕ್ಷ HighlightColor::Macro:
      OS.changeColor(raw_ostream::RED);
      ಭಙ್ಗ;
    ಪಕ್ಷ HighlightColor::Error:
      OS.changeColor(raw_ostream::RED, ಸತ್ಯ);
      ಭಙ್ಗ;
    ಪಕ್ಷ HighlightColor::Warning:
      OS.changeColor(raw_ostream::MAGENTA, ಸತ್ಯ);
      ಭಙ್ಗ;
    ಪಕ್ಷ HighlightColor::Note:
      OS.changeColor(raw_ostream::BLACK, ಸತ್ಯ);
      ಭಙ್ಗ;
    ಪಕ್ಷ HighlightColor::Remark:
      OS.changeColor(raw_ostream::BLUE, ಸತ್ಯ);
      ಭಙ್ಗ;
    }
  }
}

raw_ostream &WithColor::error() { ಪ್ರತ್ಯಾಗಮ error(errs()); }

raw_ostream &WithColor::warning() { ಪ್ರತ್ಯಾಗಮ warning(errs()); }

raw_ostream &WithColor::note() { ಪ್ರತ್ಯಾಗಮ note(errs()); }

raw_ostream &WithColor::remark() { ಪ್ರತ್ಯಾಗಮ remark(errs()); }

raw_ostream &WithColor::error(raw_ostream &OS, StringRef Prefix,
                              ತಾರ್ಕಿಕ DisableColors) {
  ಯದಿ (!Prefix.empty())
    OS << Prefix << ": ";
  ಪ್ರತ್ಯಾಗಮ WithColor(OS, HighlightColor::Error,
                   DisableColors ? ColorMode::Disable : ColorMode::Auto)
             .get()
         << "error: ";
}

raw_ostream &WithColor::warning(raw_ostream &OS, StringRef Prefix,
                                ತಾರ್ಕಿಕ DisableColors) {
  ಯದಿ (!Prefix.empty())
    OS << Prefix << ": ";
  ಪ್ರತ್ಯಾಗಮ WithColor(OS, HighlightColor::Warning,
                   DisableColors ? ColorMode::Disable : ColorMode::Auto)
             .get()
         << "warning: ";
}

raw_ostream &WithColor::note(raw_ostream &OS, StringRef Prefix,
                             ತಾರ್ಕಿಕ DisableColors) {
  ಯದಿ (!Prefix.empty())
    OS << Prefix << ": ";
  ಪ್ರತ್ಯಾಗಮ WithColor(OS, HighlightColor::Note,
                   DisableColors ? ColorMode::Disable : ColorMode::Auto)
             .get()
         << "note: ";
}

raw_ostream &WithColor::remark(raw_ostream &OS, StringRef Prefix,
                               ತಾರ್ಕಿಕ DisableColors) {
  ಯದಿ (!Prefix.empty())
    OS << Prefix << ": ";
  ಪ್ರತ್ಯಾಗಮ WithColor(OS, HighlightColor::Remark,
                   DisableColors ? ColorMode::Disable : ColorMode::Auto)
             .get()
         << "remark: ";
}

ತಾರ್ಕಿಕ WithColor::colorsEnabled() {
  ವಿಕಲ್ಪ (Mode) {
  ಪಕ್ಷ ColorMode::Enable:
    ಪ್ರತ್ಯಾಗಮ ಸತ್ಯ;
  ಪಕ್ಷ ColorMode::Disable:
    ಪ್ರತ್ಯಾಗಮ ಅಸತ್ಯ;
  ಪಕ್ಷ ColorMode::Auto:
    ಪ್ರತ್ಯಾಗಮ AutoDetectFunction(OS);
  }
  llvm_unreachable("All cases handled above.");
}

WithColor &WithColor::changeColor(raw_ostream::Colors Color, ತಾರ್ಕಿಕ Bold,
                                  ತಾರ್ಕಿಕ BG) {
  ಯದಿ (colorsEnabled())
    OS.changeColor(Color, Bold, BG);
  ಪ್ರತ್ಯಾಗಮ *ಏತತ್;
}

WithColor &WithColor::resetColor() {
  ಯದಿ (colorsEnabled())
    OS.resetColor();
  ಪ್ರತ್ಯಾಗಮ *ಏತತ್;
}

WithColor::~WithColor() { resetColor(); }

ಶೂನ್ಯ WithColor::defaultErrorHandler(Error Err) {
  handleAllErrors(std::move(Err), [](ErrorInfoBase &Info) {
    WithColor::error() << Info.message() << '\n';
  });
}

ಶೂನ್ಯ WithColor::defaultWarningHandler(Error Warning) {
  handleAllErrors(std::move(Warning), [](ErrorInfoBase &Info) {
    WithColor::warning() << Info.message() << '\n';
  });
}

WithColor::AutoDetectFunctionType WithColor::defaultAutoDetectFunction() {
  ಪ್ರತ್ಯಾಗಮ DefaultAutoDetectFunction;
}

ಶೂನ್ಯ WithColor::setAutoDetectFunction(
    AutoDetectFunctionType NewAutoDetectFunction) {
  AutoDetectFunction = NewAutoDetectFunction;
}
