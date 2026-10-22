//===- WithColor.cpp ------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#അന്തര്ഭാവ "llvm/Support/WithColor.h"

#അന്തര്ഭാവ "DebugOptions.h"

#അന്തര്ഭാവ "llvm/Support/CommandLine.h"
#അന്തര്ഭാവ "llvm/Support/Error.h"
#അന്തര്ഭാവ "llvm/Support/ManagedStatic.h"

using നാമാകാശ llvm;

cl::OptionCategory &llvm::getColorCategory() {
  സ്ഥിതം cl::OptionCategory ColorCategory("Color Options");
  തിരികെ ColorCategory;
}
നാമാകാശ {
ഘടന CreateUseColor {
  സ്ഥിതം ശൂന്യം *call() {
    തിരികെ പുതിയ cl::opt<cl::boolOrDefault>(
        "color", cl::cat(getColorCategory()),
        cl::desc("Use colors in output (default=autodetect)"),
        cl::init(cl::boolOrDefault::BOU_UNSET));
  }
};
} // namespace
സ്ഥിതം ManagedStatic<cl::opt<cl::boolOrDefault>, CreateUseColor> UseColor;
ശൂന്യം llvm::initWithColorOptions() { *UseColor; }

സ്ഥിതം തർക്കം DefaultAutoDetectFunction(സ്ഥിരം raw_ostream &OS) {
  തിരികെ *UseColor == cl::boolOrDefault::BOU_UNSET
             ? OS.has_colors()
             : *UseColor == cl::boolOrDefault::BOU_TRUE;
}

WithColor::AutoDetectFunctionType WithColor::AutoDetectFunction =
    DefaultAutoDetectFunction;

WithColor::WithColor(raw_ostream &OS, HighlightColor Color, ColorMode Mode)
    : OS(OS), Mode(Mode) {
  // Detect color from terminal type unless the user passed the --color option.
  എങ്കിൽ (colorsEnabled()) {
    തിരഞ്ഞെടുപ്പ് (Color) {
    അവസ്ഥ HighlightColor::Address:
      OS.changeColor(raw_ostream::YELLOW);
      നിർത്തുക;
    അവസ്ഥ HighlightColor::String:
      OS.changeColor(raw_ostream::GREEN);
      നിർത്തുക;
    അവസ്ഥ HighlightColor::Tag:
      OS.changeColor(raw_ostream::BLUE);
      നിർത്തുക;
    അവസ്ഥ HighlightColor::Attribute:
      OS.changeColor(raw_ostream::CYAN);
      നിർത്തുക;
    അവസ്ഥ HighlightColor::Enumerator:
      OS.changeColor(raw_ostream::MAGENTA);
      നിർത്തുക;
    അവസ്ഥ HighlightColor::Macro:
      OS.changeColor(raw_ostream::RED);
      നിർത്തുക;
    അവസ്ഥ HighlightColor::Error:
      OS.changeColor(raw_ostream::RED, സത്യം);
      നിർത്തുക;
    അവസ്ഥ HighlightColor::Warning:
      OS.changeColor(raw_ostream::MAGENTA, സത്യം);
      നിർത്തുക;
    അവസ്ഥ HighlightColor::Note:
      OS.changeColor(raw_ostream::BLACK, സത്യം);
      നിർത്തുക;
    അവസ്ഥ HighlightColor::Remark:
      OS.changeColor(raw_ostream::BLUE, സത്യം);
      നിർത്തുക;
    }
  }
}

raw_ostream &WithColor::error() { തിരികെ error(errs()); }

raw_ostream &WithColor::warning() { തിരികെ warning(errs()); }

raw_ostream &WithColor::note() { തിരികെ note(errs()); }

raw_ostream &WithColor::remark() { തിരികെ remark(errs()); }

raw_ostream &WithColor::error(raw_ostream &OS, StringRef Prefix,
                              തർക്കം DisableColors) {
  എങ്കിൽ (!Prefix.empty())
    OS << Prefix << ": ";
  തിരികെ WithColor(OS, HighlightColor::Error,
                   DisableColors ? ColorMode::Disable : ColorMode::Auto)
             .get()
         << "error: ";
}

raw_ostream &WithColor::warning(raw_ostream &OS, StringRef Prefix,
                                തർക്കം DisableColors) {
  എങ്കിൽ (!Prefix.empty())
    OS << Prefix << ": ";
  തിരികെ WithColor(OS, HighlightColor::Warning,
                   DisableColors ? ColorMode::Disable : ColorMode::Auto)
             .get()
         << "warning: ";
}

raw_ostream &WithColor::note(raw_ostream &OS, StringRef Prefix,
                             തർക്കം DisableColors) {
  എങ്കിൽ (!Prefix.empty())
    OS << Prefix << ": ";
  തിരികെ WithColor(OS, HighlightColor::Note,
                   DisableColors ? ColorMode::Disable : ColorMode::Auto)
             .get()
         << "note: ";
}

raw_ostream &WithColor::remark(raw_ostream &OS, StringRef Prefix,
                               തർക്കം DisableColors) {
  എങ്കിൽ (!Prefix.empty())
    OS << Prefix << ": ";
  തിരികെ WithColor(OS, HighlightColor::Remark,
                   DisableColors ? ColorMode::Disable : ColorMode::Auto)
             .get()
         << "remark: ";
}

തർക്കം WithColor::colorsEnabled() {
  തിരഞ്ഞെടുപ്പ് (Mode) {
  അവസ്ഥ ColorMode::Enable:
    തിരികെ സത്യം;
  അവസ്ഥ ColorMode::Disable:
    തിരികെ അസത്യം;
  അവസ്ഥ ColorMode::Auto:
    തിരികെ AutoDetectFunction(OS);
  }
  llvm_unreachable("All cases handled above.");
}

WithColor &WithColor::changeColor(raw_ostream::Colors Color, തർക്കം Bold,
                                  തർക്കം BG) {
  എങ്കിൽ (colorsEnabled())
    OS.changeColor(Color, Bold, BG);
  തിരികെ *ഇത്;
}

WithColor &WithColor::resetColor() {
  എങ്കിൽ (colorsEnabled())
    OS.resetColor();
  തിരികെ *ഇത്;
}

WithColor::~WithColor() { resetColor(); }

ശൂന്യം WithColor::defaultErrorHandler(Error Err) {
  handleAllErrors(std::move(Err), [](ErrorInfoBase &Info) {
    WithColor::error() << Info.message() << '\n';
  });
}

ശൂന്യം WithColor::defaultWarningHandler(Error Warning) {
  handleAllErrors(std::move(Warning), [](ErrorInfoBase &Info) {
    WithColor::warning() << Info.message() << '\n';
  });
}

WithColor::AutoDetectFunctionType WithColor::defaultAutoDetectFunction() {
  തിരികെ DefaultAutoDetectFunction;
}

ശൂന്യം WithColor::setAutoDetectFunction(
    AutoDetectFunctionType NewAutoDetectFunction) {
  AutoDetectFunction = NewAutoDetectFunction;
}
