//===- WithColor.cpp ------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#شامل "llvm/Support/WithColor.h"

#شامل "DebugOptions.h"

#شامل "llvm/Support/CommandLine.h"
#شامل "llvm/Support/Error.h"
#شامل "llvm/Support/ManagedStatic.h"

using فضاینام llvm;

cl::OptionCategory &llvm::getColorCategory() {
  ایستا cl::OptionCategory ColorCategory("Color Options");
  بازگشت ColorCategory;
}
فضاینام {
ساختار CreateUseColor {
  ایستا تهی *call() {
    بازگشت جدید cl::opt<cl::boolOrDefault>(
        "color", cl::cat(getColorCategory()),
        cl::desc("Use colors in output (default=autodetect)"),
        cl::init(cl::boolOrDefault::BOU_UNSET));
  }
};
} // namespace
ایستا ManagedStatic<cl::opt<cl::boolOrDefault>, CreateUseColor> UseColor;
تهی llvm::initWithColorOptions() { *UseColor; }

ایستا منطقی DefaultAutoDetectFunction(ثابت raw_ostream &OS) {
  بازگشت *UseColor == cl::boolOrDefault::BOU_UNSET
             ? OS.has_colors()
             : *UseColor == cl::boolOrDefault::BOU_TRUE;
}

WithColor::AutoDetectFunctionType WithColor::AutoDetectFunction =
    DefaultAutoDetectFunction;

WithColor::WithColor(raw_ostream &OS, HighlightColor Color, ColorMode Mode)
    : OS(OS), Mode(Mode) {
  // Detect color from terminal type unless the user passed the --color option.
  اگر (colorsEnabled()) {
    انتخاب (Color) {
    حالت HighlightColor::Address:
      OS.changeColor(raw_ostream::YELLOW);
      بشکن;
    حالت HighlightColor::String:
      OS.changeColor(raw_ostream::GREEN);
      بشکن;
    حالت HighlightColor::Tag:
      OS.changeColor(raw_ostream::BLUE);
      بشکن;
    حالت HighlightColor::Attribute:
      OS.changeColor(raw_ostream::CYAN);
      بشکن;
    حالت HighlightColor::Enumerator:
      OS.changeColor(raw_ostream::MAGENTA);
      بشکن;
    حالت HighlightColor::Macro:
      OS.changeColor(raw_ostream::RED);
      بشکن;
    حالت HighlightColor::Error:
      OS.changeColor(raw_ostream::RED, درست);
      بشکن;
    حالت HighlightColor::Warning:
      OS.changeColor(raw_ostream::MAGENTA, درست);
      بشکن;
    حالت HighlightColor::Note:
      OS.changeColor(raw_ostream::BLACK, درست);
      بشکن;
    حالت HighlightColor::Remark:
      OS.changeColor(raw_ostream::BLUE, درست);
      بشکن;
    }
  }
}

raw_ostream &WithColor::error() { بازگشت error(errs()); }

raw_ostream &WithColor::warning() { بازگشت warning(errs()); }

raw_ostream &WithColor::note() { بازگشت note(errs()); }

raw_ostream &WithColor::remark() { بازگشت remark(errs()); }

raw_ostream &WithColor::error(raw_ostream &OS, StringRef Prefix,
                              منطقی DisableColors) {
  اگر (!Prefix.empty())
    OS << Prefix << ": ";
  بازگشت WithColor(OS, HighlightColor::Error,
                   DisableColors ? ColorMode::Disable : ColorMode::Auto)
             .get()
         << "error: ";
}

raw_ostream &WithColor::warning(raw_ostream &OS, StringRef Prefix,
                                منطقی DisableColors) {
  اگر (!Prefix.empty())
    OS << Prefix << ": ";
  بازگشت WithColor(OS, HighlightColor::Warning,
                   DisableColors ? ColorMode::Disable : ColorMode::Auto)
             .get()
         << "warning: ";
}

raw_ostream &WithColor::note(raw_ostream &OS, StringRef Prefix,
                             منطقی DisableColors) {
  اگر (!Prefix.empty())
    OS << Prefix << ": ";
  بازگشت WithColor(OS, HighlightColor::Note,
                   DisableColors ? ColorMode::Disable : ColorMode::Auto)
             .get()
         << "note: ";
}

raw_ostream &WithColor::remark(raw_ostream &OS, StringRef Prefix,
                               منطقی DisableColors) {
  اگر (!Prefix.empty())
    OS << Prefix << ": ";
  بازگشت WithColor(OS, HighlightColor::Remark,
                   DisableColors ? ColorMode::Disable : ColorMode::Auto)
             .get()
         << "remark: ";
}

منطقی WithColor::colorsEnabled() {
  انتخاب (Mode) {
  حالت ColorMode::Enable:
    بازگشت درست;
  حالت ColorMode::Disable:
    بازگشت نادرست;
  حالت ColorMode::Auto:
    بازگشت AutoDetectFunction(OS);
  }
  llvm_unreachable("All cases handled above.");
}

WithColor &WithColor::changeColor(raw_ostream::Colors Color, منطقی Bold,
                                  منطقی BG) {
  اگر (colorsEnabled())
    OS.changeColor(Color, Bold, BG);
  بازگشت *این;
}

WithColor &WithColor::resetColor() {
  اگر (colorsEnabled())
    OS.resetColor();
  بازگشت *این;
}

WithColor::~WithColor() { resetColor(); }

تهی WithColor::defaultErrorHandler(Error Err) {
  handleAllErrors(std::move(Err), [](ErrorInfoBase &Info) {
    WithColor::error() << Info.message() << '\n';
  });
}

تهی WithColor::defaultWarningHandler(Error Warning) {
  handleAllErrors(std::move(Warning), [](ErrorInfoBase &Info) {
    WithColor::warning() << Info.message() << '\n';
  });
}

WithColor::AutoDetectFunctionType WithColor::defaultAutoDetectFunction() {
  بازگشت DefaultAutoDetectFunction;
}

تهی WithColor::setAutoDetectFunction(
    AutoDetectFunctionType NewAutoDetectFunction) {
  AutoDetectFunction = NewAutoDetectFunction;
}
