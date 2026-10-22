//===- WithColor.cpp ------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#அந்தர்பாவ "llvm/Support/WithColor.h"

#அந்தர்பாவ "DebugOptions.h"

#அந்தர்பாவ "llvm/Support/CommandLine.h"
#அந்தர்பாவ "llvm/Support/Error.h"
#அந்தர்பாவ "llvm/Support/ManagedStatic.h"

using நாமாகாஶ llvm;

cl::OptionCategory &llvm::getColorCategory() {
  நிலையான cl::OptionCategory ColorCategory("Color Options");
  திரும்பு ColorCategory;
}
நாமாகாஶ {
கட்டமைப்பு CreateUseColor {
  நிலையான வெறுமை *call() {
    திரும்பு புதிய cl::opt<cl::boolOrDefault>(
        "color", cl::cat(getColorCategory()),
        cl::desc("Use colors in output (default=autodetect)"),
        cl::init(cl::boolOrDefault::BOU_UNSET));
  }
};
} // namespace
நிலையான ManagedStatic<cl::opt<cl::boolOrDefault>, CreateUseColor> UseColor;
வெறுமை llvm::initWithColorOptions() { *UseColor; }

நிலையான தருக்க DefaultAutoDetectFunction(மாறிலி raw_ostream &OS) {
  திரும்பு *UseColor == cl::boolOrDefault::BOU_UNSET
             ? OS.has_colors()
             : *UseColor == cl::boolOrDefault::BOU_TRUE;
}

WithColor::AutoDetectFunctionType WithColor::AutoDetectFunction =
    DefaultAutoDetectFunction;

WithColor::WithColor(raw_ostream &OS, HighlightColor Color, ColorMode Mode)
    : OS(OS), Mode(Mode) {
  // Detect color from terminal type unless the user passed the --color option.
  எனில் (colorsEnabled()) {
    தேர்வு (Color) {
    நிலை HighlightColor::Address:
      OS.changeColor(raw_ostream::YELLOW);
      நிறுத்து;
    நிலை HighlightColor::String:
      OS.changeColor(raw_ostream::GREEN);
      நிறுத்து;
    நிலை HighlightColor::Tag:
      OS.changeColor(raw_ostream::BLUE);
      நிறுத்து;
    நிலை HighlightColor::Attribute:
      OS.changeColor(raw_ostream::CYAN);
      நிறுத்து;
    நிலை HighlightColor::Enumerator:
      OS.changeColor(raw_ostream::MAGENTA);
      நிறுத்து;
    நிலை HighlightColor::Macro:
      OS.changeColor(raw_ostream::RED);
      நிறுத்து;
    நிலை HighlightColor::Error:
      OS.changeColor(raw_ostream::RED, உண்மை);
      நிறுத்து;
    நிலை HighlightColor::Warning:
      OS.changeColor(raw_ostream::MAGENTA, உண்மை);
      நிறுத்து;
    நிலை HighlightColor::Note:
      OS.changeColor(raw_ostream::BLACK, உண்மை);
      நிறுத்து;
    நிலை HighlightColor::Remark:
      OS.changeColor(raw_ostream::BLUE, உண்மை);
      நிறுத்து;
    }
  }
}

raw_ostream &WithColor::error() { திரும்பு error(errs()); }

raw_ostream &WithColor::warning() { திரும்பு warning(errs()); }

raw_ostream &WithColor::note() { திரும்பு note(errs()); }

raw_ostream &WithColor::remark() { திரும்பு remark(errs()); }

raw_ostream &WithColor::error(raw_ostream &OS, StringRef Prefix,
                              தருக்க DisableColors) {
  எனில் (!Prefix.empty())
    OS << Prefix << ": ";
  திரும்பு WithColor(OS, HighlightColor::Error,
                   DisableColors ? ColorMode::Disable : ColorMode::Auto)
             .get()
         << "error: ";
}

raw_ostream &WithColor::warning(raw_ostream &OS, StringRef Prefix,
                                தருக்க DisableColors) {
  எனில் (!Prefix.empty())
    OS << Prefix << ": ";
  திரும்பு WithColor(OS, HighlightColor::Warning,
                   DisableColors ? ColorMode::Disable : ColorMode::Auto)
             .get()
         << "warning: ";
}

raw_ostream &WithColor::note(raw_ostream &OS, StringRef Prefix,
                             தருக்க DisableColors) {
  எனில் (!Prefix.empty())
    OS << Prefix << ": ";
  திரும்பு WithColor(OS, HighlightColor::Note,
                   DisableColors ? ColorMode::Disable : ColorMode::Auto)
             .get()
         << "note: ";
}

raw_ostream &WithColor::remark(raw_ostream &OS, StringRef Prefix,
                               தருக்க DisableColors) {
  எனில் (!Prefix.empty())
    OS << Prefix << ": ";
  திரும்பு WithColor(OS, HighlightColor::Remark,
                   DisableColors ? ColorMode::Disable : ColorMode::Auto)
             .get()
         << "remark: ";
}

தருக்க WithColor::colorsEnabled() {
  தேர்வு (Mode) {
  நிலை ColorMode::Enable:
    திரும்பு உண்மை;
  நிலை ColorMode::Disable:
    திரும்பு பொய்;
  நிலை ColorMode::Auto:
    திரும்பு AutoDetectFunction(OS);
  }
  llvm_unreachable("All cases handled above.");
}

WithColor &WithColor::changeColor(raw_ostream::Colors Color, தருக்க Bold,
                                  தருக்க BG) {
  எனில் (colorsEnabled())
    OS.changeColor(Color, Bold, BG);
  திரும்பு *இது;
}

WithColor &WithColor::resetColor() {
  எனில் (colorsEnabled())
    OS.resetColor();
  திரும்பு *இது;
}

WithColor::~WithColor() { resetColor(); }

வெறுமை WithColor::defaultErrorHandler(Error Err) {
  handleAllErrors(std::move(Err), [](ErrorInfoBase &Info) {
    WithColor::error() << Info.message() << '\n';
  });
}

வெறுமை WithColor::defaultWarningHandler(Error Warning) {
  handleAllErrors(std::move(Warning), [](ErrorInfoBase &Info) {
    WithColor::warning() << Info.message() << '\n';
  });
}

WithColor::AutoDetectFunctionType WithColor::defaultAutoDetectFunction() {
  திரும்பு DefaultAutoDetectFunction;
}

வெறுமை WithColor::setAutoDetectFunction(
    AutoDetectFunctionType NewAutoDetectFunction) {
  AutoDetectFunction = NewAutoDetectFunction;
}
