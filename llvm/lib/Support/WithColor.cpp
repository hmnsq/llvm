//===- WithColor.cpp ------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/WithColor.h"

#ਅਨ੍ਤਰ੍ਭਾਵ "DebugOptions.h"

#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/CommandLine.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/Error.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/ManagedStatic.h"

ਵਰਤੋਂ ਨਾਮਾਕਾਸ਼ llvm;

cl::OptionCategory &llvm::getColorCategory() {
  ਸਥਿਰ cl::OptionCategory ColorCategory("Color Options");
  ਵਾਪਸ ColorCategory;
}
ਨਾਮਾਕਾਸ਼ {
ਬਣਤਰ CreateUseColor {
  ਸਥਿਰ ਖਾਲੀ *call() {
    ਵਾਪਸ ਨਵਾਂ cl::opt<cl::boolOrDefault>(
        "color", cl::cat(getColorCategory()),
        cl::desc("Use colors in output (default=autodetect)"),
        cl::init(cl::boolOrDefault::BOU_UNSET));
  }
};
} // namespace
ਸਥਿਰ ManagedStatic<cl::opt<cl::boolOrDefault>, CreateUseColor> UseColor;
ਖਾਲੀ llvm::initWithColorOptions() { *UseColor; }

ਸਥਿਰ ਤਾਰਕਿਕ DefaultAutoDetectFunction(ਅਚਲ raw_ostream &OS) {
  ਵਾਪਸ *UseColor == cl::boolOrDefault::BOU_UNSET
             ? OS.has_colors()
             : *UseColor == cl::boolOrDefault::BOU_TRUE;
}

WithColor::AutoDetectFunctionType WithColor::AutoDetectFunction =
    DefaultAutoDetectFunction;

WithColor::WithColor(raw_ostream &OS, HighlightColor Color, Color_ਬਹੁਲਕ ਬਹੁਲਕ)
    : OS(OS), ਬਹੁਲਕ(ਬਹੁਲਕ) {
  // Detect color from terminal type unless the user passed the --color option.
  ਜੇ (colorsEnabled()) {
    ਚੋਣ (Color) {
    ਹਾਲਤ HighlightColor::ਪਤਾ:
      OS.changeColor(raw_ostream::YELLOW);
      ਰੋਕੋ;
    ਹਾਲਤ HighlightColor::String:
      OS.changeColor(raw_ostream::GREEN);
      ਰੋਕੋ;
    ਹਾਲਤ HighlightColor::ਟੈਗ:
      OS.changeColor(raw_ostream::BLUE);
      ਰੋਕੋ;
    ਹਾਲਤ HighlightColor::Attribute:
      OS.changeColor(raw_ostream::CYAN);
      ਰੋਕੋ;
    ਹਾਲਤ HighlightColor::Enumerator:
      OS.changeColor(raw_ostream::MAGENTA);
      ਰੋਕੋ;
    ਹਾਲਤ HighlightColor::Macro:
      OS.changeColor(raw_ostream::RED);
      ਰੋਕੋ;
    ਹਾਲਤ HighlightColor::Error:
      OS.changeColor(raw_ostream::RED, ਸੱਚ);
      ਰੋਕੋ;
    ਹਾਲਤ HighlightColor::Warning:
      OS.changeColor(raw_ostream::MAGENTA, ਸੱਚ);
      ਰੋਕੋ;
    ਹਾਲਤ HighlightColor::Note:
      OS.changeColor(raw_ostream::BLACK, ਸੱਚ);
      ਰੋਕੋ;
    ਹਾਲਤ HighlightColor::Remark:
      OS.changeColor(raw_ostream::BLUE, ਸੱਚ);
      ਰੋਕੋ;
    }
  }
}

raw_ostream &WithColor::error() { ਵਾਪਸ error(errs()); }

raw_ostream &WithColor::warning() { ਵਾਪਸ warning(errs()); }

raw_ostream &WithColor::note() { ਵਾਪਸ note(errs()); }

raw_ostream &WithColor::remark() { ਵਾਪਸ remark(errs()); }

raw_ostream &WithColor::error(raw_ostream &OS, StringRef ਅਗੇਤਰ,
                              ਤਾਰਕਿਕ DisableColors) {
  ਜੇ (!ਅਗੇਤਰ.ਖਾਲੀ_ਥਾਵਾਂ())
    OS << ਅਗੇਤਰ << ": ";
  ਵਾਪਸ WithColor(OS, HighlightColor::Error,
                   DisableColors ? Color_ਬਹੁਲਕ::Disable : Color_ਬਹੁਲਕ::Auto)
             .get()
         << "error: ";
}

raw_ostream &WithColor::warning(raw_ostream &OS, StringRef ਅਗੇਤਰ,
                                ਤਾਰਕਿਕ DisableColors) {
  ਜੇ (!ਅਗੇਤਰ.ਖਾਲੀ_ਥਾਵਾਂ())
    OS << ਅਗੇਤਰ << ": ";
  ਵਾਪਸ WithColor(OS, HighlightColor::Warning,
                   DisableColors ? Color_ਬਹੁਲਕ::Disable : Color_ਬਹੁਲਕ::Auto)
             .get()
         << "warning: ";
}

raw_ostream &WithColor::note(raw_ostream &OS, StringRef ਅਗੇਤਰ,
                             ਤਾਰਕਿਕ DisableColors) {
  ਜੇ (!ਅਗੇਤਰ.ਖਾਲੀ_ਥਾਵਾਂ())
    OS << ਅਗੇਤਰ << ": ";
  ਵਾਪਸ WithColor(OS, HighlightColor::Note,
                   DisableColors ? Color_ਬਹੁਲਕ::Disable : Color_ਬਹੁਲਕ::Auto)
             .get()
         << "note: ";
}

raw_ostream &WithColor::remark(raw_ostream &OS, StringRef ਅਗੇਤਰ,
                               ਤਾਰਕਿਕ DisableColors) {
  ਜੇ (!ਅਗੇਤਰ.ਖਾਲੀ_ਥਾਵਾਂ())
    OS << ਅਗੇਤਰ << ": ";
  ਵਾਪਸ WithColor(OS, HighlightColor::Remark,
                   DisableColors ? Color_ਬਹੁਲਕ::Disable : Color_ਬਹੁਲਕ::Auto)
             .get()
         << "remark: ";
}

ਤਾਰਕਿਕ WithColor::colorsEnabled() {
  ਚੋਣ (ਬਹੁਲਕ) {
  ਹਾਲਤ Color_ਬਹੁਲਕ::Enable:
    ਵਾਪਸ ਸੱਚ;
  ਹਾਲਤ Color_ਬਹੁਲਕ::Disable:
    ਵਾਪਸ ਝੂਠ;
  ਹਾਲਤ Color_ਬਹੁਲਕ::Auto:
    ਵਾਪਸ AutoDetectFunction(OS);
  }
  llvm_unreachable("All cases handled above.");
}

WithColor &WithColor::changeColor(raw_ostream::Colors Color, ਤਾਰਕਿਕ Bold,
                                  ਤਾਰਕਿਕ BG) {
  ਜੇ (colorsEnabled())
    OS.changeColor(Color, Bold, BG);
  ਵਾਪਸ *ਇਹ;
}

WithColor &WithColor::resetColor() {
  ਜੇ (colorsEnabled())
    OS.resetColor();
  ਵਾਪਸ *ਇਹ;
}

WithColor::~WithColor() { resetColor(); }

ਖਾਲੀ WithColor::defaultErrorHandler(Error Err) {
  handleAllErrors(std::move(Err), [](Error_Info_ਅਧਾਰ &Info) {
    WithColor::error() << Info.ਸੁਨੇਹਾ_2() << '\n';
  });
}

ਖਾਲੀ WithColor::defaultWarningHandler(Error Warning) {
  handleAllErrors(std::move(Warning), [](Error_Info_ਅਧਾਰ &Info) {
    WithColor::warning() << Info.ਸੁਨੇਹਾ_2() << '\n';
  });
}

WithColor::AutoDetectFunctionType WithColor::defaultAutoDetectFunction() {
  ਵਾਪਸ DefaultAutoDetectFunction;
}

ਖਾਲੀ WithColor::ਸੈੱਟ_Auto_Detect_Function(
    AutoDetectFunctionType NewAutoDetectFunction) {
  AutoDetectFunction = NewAutoDetectFunction;
}
