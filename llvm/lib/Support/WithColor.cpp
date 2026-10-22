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

ಬಳಸಿ ನಾಮಾಕಾಶ llvm;

cl::OptionCategory &llvm::getColorCategory() {
  ಸ್ಥಿರ cl::OptionCategory ColorCategory("Color Options");
  ಹಿಂತಿರುಗಿ ColorCategory;
}
ನಾಮಾಕಾಶ {
ರಚನೆ CreateUseColor {
  ಸ್ಥಿರ ಶೂನ್ಯ *call() {
    ಹಿಂತಿರುಗಿ ಹೊಸ cl::opt<cl::boolOrDefault>(
        "color", cl::cat(getColorCategory()),
        cl::desc("Use colors in output (default=autodetect)"),
        cl::init(cl::boolOrDefault::BOU_UNSET));
  }
};
} // namespace
ಸ್ಥಿರ ManagedStatic<cl::opt<cl::boolOrDefault>, CreateUseColor> UseColor;
ಶೂನ್ಯ llvm::initWithColorOptions() { *UseColor; }

ಸ್ಥಿರ ತಾರ್ಕಿಕ DefaultAutoDetectFunction(ಸ್ಥಿರಾಂಕ raw_ostream &OS) {
  ಹಿಂತಿರುಗಿ *UseColor == cl::boolOrDefault::BOU_UNSET
             ? OS.has_colors()
             : *UseColor == cl::boolOrDefault::BOU_TRUE;
}

WithColor::AutoDetectFunctionType WithColor::AutoDetectFunction =
    DefaultAutoDetectFunction;

WithColor::WithColor(raw_ostream &OS, HighlightColor Color, Color_ಬಹುಲಕ ಬಹುಲಕ)
    : OS(OS), ಬಹುಲಕ(ಬಹುಲಕ) {
  // Detect color from terminal type unless the user passed the --color option.
  ಆದರೆ (colorsEnabled()) {
    ಆಯ್ಕೆ (Color) {
    ಸ್ಥಿತಿ HighlightColor::ವಿಳಾಸ:
      OS.changeColor(raw_ostream::YELLOW);
      ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ HighlightColor::String:
      OS.changeColor(raw_ostream::GREEN);
      ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ HighlightColor::ಟ್ಯಾಗ್:
      OS.changeColor(raw_ostream::BLUE);
      ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ HighlightColor::Attribute:
      OS.changeColor(raw_ostream::CYAN);
      ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ HighlightColor::Enumerator:
      OS.changeColor(raw_ostream::MAGENTA);
      ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ HighlightColor::Macro:
      OS.changeColor(raw_ostream::RED);
      ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ HighlightColor::Error:
      OS.changeColor(raw_ostream::RED, ನಿಜ);
      ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ HighlightColor::Warning:
      OS.changeColor(raw_ostream::MAGENTA, ನಿಜ);
      ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ HighlightColor::Note:
      OS.changeColor(raw_ostream::BLACK, ನಿಜ);
      ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ HighlightColor::Remark:
      OS.changeColor(raw_ostream::BLUE, ನಿಜ);
      ನಿಲ್ಲಿಸು;
    }
  }
}

raw_ostream &WithColor::error() { ಹಿಂತಿರುಗಿ error(errs()); }

raw_ostream &WithColor::warning() { ಹಿಂತಿರುಗಿ warning(errs()); }

raw_ostream &WithColor::note() { ಹಿಂತಿರುಗಿ note(errs()); }

raw_ostream &WithColor::remark() { ಹಿಂತಿರುಗಿ remark(errs()); }

raw_ostream &WithColor::error(raw_ostream &OS, StringRef ಪೂರ್ವಪ್ರತ್ಯಯ,
                              ತಾರ್ಕಿಕ DisableColors) {
  ಆದರೆ (!ಪೂರ್ವಪ್ರತ್ಯಯ.ಖಾಲಿಗಳು())
    OS << ಪೂರ್ವಪ್ರತ್ಯಯ << ": ";
  ಹಿಂತಿರುಗಿ WithColor(OS, HighlightColor::Error,
                   DisableColors ? Color_ಬಹುಲಕ::Disable : Color_ಬಹುಲಕ::Auto)
             .get()
         << "error: ";
}

raw_ostream &WithColor::warning(raw_ostream &OS, StringRef ಪೂರ್ವಪ್ರತ್ಯಯ,
                                ತಾರ್ಕಿಕ DisableColors) {
  ಆದರೆ (!ಪೂರ್ವಪ್ರತ್ಯಯ.ಖಾಲಿಗಳು())
    OS << ಪೂರ್ವಪ್ರತ್ಯಯ << ": ";
  ಹಿಂತಿರುಗಿ WithColor(OS, HighlightColor::Warning,
                   DisableColors ? Color_ಬಹುಲಕ::Disable : Color_ಬಹುಲಕ::Auto)
             .get()
         << "warning: ";
}

raw_ostream &WithColor::note(raw_ostream &OS, StringRef ಪೂರ್ವಪ್ರತ್ಯಯ,
                             ತಾರ್ಕಿಕ DisableColors) {
  ಆದರೆ (!ಪೂರ್ವಪ್ರತ್ಯಯ.ಖಾಲಿಗಳು())
    OS << ಪೂರ್ವಪ್ರತ್ಯಯ << ": ";
  ಹಿಂತಿರುಗಿ WithColor(OS, HighlightColor::Note,
                   DisableColors ? Color_ಬಹುಲಕ::Disable : Color_ಬಹುಲಕ::Auto)
             .get()
         << "note: ";
}

raw_ostream &WithColor::remark(raw_ostream &OS, StringRef ಪೂರ್ವಪ್ರತ್ಯಯ,
                               ತಾರ್ಕಿಕ DisableColors) {
  ಆದರೆ (!ಪೂರ್ವಪ್ರತ್ಯಯ.ಖಾಲಿಗಳು())
    OS << ಪೂರ್ವಪ್ರತ್ಯಯ << ": ";
  ಹಿಂತಿರುಗಿ WithColor(OS, HighlightColor::Remark,
                   DisableColors ? Color_ಬಹುಲಕ::Disable : Color_ಬಹುಲಕ::Auto)
             .get()
         << "remark: ";
}

ತಾರ್ಕಿಕ WithColor::colorsEnabled() {
  ಆಯ್ಕೆ (ಬಹುಲಕ) {
  ಸ್ಥಿತಿ Color_ಬಹುಲಕ::Enable:
    ಹಿಂತಿರುಗಿ ನಿಜ;
  ಸ್ಥಿತಿ Color_ಬಹುಲಕ::Disable:
    ಹಿಂತಿರುಗಿ ಸುಳ್ಳು;
  ಸ್ಥಿತಿ Color_ಬಹುಲಕ::Auto:
    ಹಿಂತಿರುಗಿ AutoDetectFunction(OS);
  }
  llvm_unreachable("All cases handled above.");
}

WithColor &WithColor::changeColor(raw_ostream::Colors Color, ತಾರ್ಕಿಕ Bold,
                                  ತಾರ್ಕಿಕ BG) {
  ಆದರೆ (colorsEnabled())
    OS.changeColor(Color, Bold, BG);
  ಹಿಂತಿರುಗಿ *ಇದು;
}

WithColor &WithColor::resetColor() {
  ಆದರೆ (colorsEnabled())
    OS.resetColor();
  ಹಿಂತಿರುಗಿ *ಇದು;
}

WithColor::~WithColor() { resetColor(); }

ಶೂನ್ಯ WithColor::defaultErrorHandler(Error Err) {
  handleAllErrors(std::move(Err), [](Error_Info_ತಳ &Info) {
    WithColor::error() << Info.ಸಂದೇಶ_2() << '\n';
  });
}

ಶೂನ್ಯ WithColor::defaultWarningHandler(Error Warning) {
  handleAllErrors(std::move(Warning), [](Error_Info_ತಳ &Info) {
    WithColor::warning() << Info.ಸಂದೇಶ_2() << '\n';
  });
}

WithColor::AutoDetectFunctionType WithColor::defaultAutoDetectFunction() {
  ಹಿಂತಿರುಗಿ DefaultAutoDetectFunction;
}

ಶೂನ್ಯ WithColor::ಗಣ_Auto_Detect_Function(
    AutoDetectFunctionType NewAutoDetectFunction) {
  AutoDetectFunction = NewAutoDetectFunction;
}
