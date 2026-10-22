//===- WithColor.cpp ------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ضمن "llvm/Support/WithColor.h"

#ضمن "DebugOptions.h"

#ضمن "llvm/Support/CommandLine.h"
#ضمن "llvm/Support/Error.h"
#ضمن "llvm/Support/ManagedStatic.h"

using نطاق llvm;

cl::OptionCategory &llvm::getColorCategory() {
  ساكن cl::OptionCategory ColorCategory("Color Options");
  ܦܢܐ ColorCategory;
}
نطاق {
بنية CreateUseColor {
  ساكن فراغ *call() {
    ܦܢܐ جديد cl::opt<cl::boolOrDefault>(
        "color", cl::cat(getColorCategory()),
        cl::desc("Use colors in output (default=autodetect)"),
        cl::init(cl::boolOrDefault::BOU_UNSET));
  }
};
} // namespace
ساكن ManagedStatic<cl::opt<cl::boolOrDefault>, CreateUseColor> UseColor;
فراغ llvm::initWithColorOptions() { *UseColor; }

ساكن منطقي DefaultAutoDetectFunction(ثابت raw_ostream &OS) {
  ܦܢܐ *UseColor == cl::boolOrDefault::BOU_UNSET
             ? OS.has_colors()
             : *UseColor == cl::boolOrDefault::BOU_TRUE;
}

WithColor::AutoDetectFunctionType WithColor::AutoDetectFunction =
    DefaultAutoDetectFunction;

WithColor::WithColor(raw_ostream &OS, HighlightColor Color, ColorMode Mode)
    : OS(OS), Mode(Mode) {
  // Detect color from terminal type unless the user passed the --color option.
  ܐܢ (colorsEnabled()) {
    بدل (Color) {
    حالة HighlightColor::Address:
      OS.changeColor(raw_ostream::YELLOW);
      اكسر;
    حالة HighlightColor::String:
      OS.changeColor(raw_ostream::GREEN);
      اكسر;
    حالة HighlightColor::Tag:
      OS.changeColor(raw_ostream::BLUE);
      اكسر;
    حالة HighlightColor::Attribute:
      OS.changeColor(raw_ostream::CYAN);
      اكسر;
    حالة HighlightColor::Enumerator:
      OS.changeColor(raw_ostream::MAGENTA);
      اكسر;
    حالة HighlightColor::Macro:
      OS.changeColor(raw_ostream::RED);
      اكسر;
    حالة HighlightColor::Error:
      OS.changeColor(raw_ostream::RED, ܫܪܪܐ);
      اكسر;
    حالة HighlightColor::Warning:
      OS.changeColor(raw_ostream::MAGENTA, ܫܪܪܐ);
      اكسر;
    حالة HighlightColor::Note:
      OS.changeColor(raw_ostream::BLACK, ܫܪܪܐ);
      اكسر;
    حالة HighlightColor::Remark:
      OS.changeColor(raw_ostream::BLUE, ܫܪܪܐ);
      اكسر;
    }
  }
}

raw_ostream &WithColor::error() { ܦܢܐ error(errs()); }

raw_ostream &WithColor::warning() { ܦܢܐ warning(errs()); }

raw_ostream &WithColor::note() { ܦܢܐ note(errs()); }

raw_ostream &WithColor::remark() { ܦܢܐ remark(errs()); }

raw_ostream &WithColor::error(raw_ostream &OS, StringRef Prefix,
                              منطقي DisableColors) {
  ܐܢ (!Prefix.empty())
    OS << Prefix << ": ";
  ܦܢܐ WithColor(OS, HighlightColor::Error,
                   DisableColors ? ColorMode::Disable : ColorMode::Auto)
             .get()
         << "error: ";
}

raw_ostream &WithColor::warning(raw_ostream &OS, StringRef Prefix,
                                منطقي DisableColors) {
  ܐܢ (!Prefix.empty())
    OS << Prefix << ": ";
  ܦܢܐ WithColor(OS, HighlightColor::Warning,
                   DisableColors ? ColorMode::Disable : ColorMode::Auto)
             .get()
         << "warning: ";
}

raw_ostream &WithColor::note(raw_ostream &OS, StringRef Prefix,
                             منطقي DisableColors) {
  ܐܢ (!Prefix.empty())
    OS << Prefix << ": ";
  ܦܢܐ WithColor(OS, HighlightColor::Note,
                   DisableColors ? ColorMode::Disable : ColorMode::Auto)
             .get()
         << "note: ";
}

raw_ostream &WithColor::remark(raw_ostream &OS, StringRef Prefix,
                               منطقي DisableColors) {
  ܐܢ (!Prefix.empty())
    OS << Prefix << ": ";
  ܦܢܐ WithColor(OS, HighlightColor::Remark,
                   DisableColors ? ColorMode::Disable : ColorMode::Auto)
             .get()
         << "remark: ";
}

منطقي WithColor::colorsEnabled() {
  بدل (Mode) {
  حالة ColorMode::Enable:
    ܦܢܐ ܫܪܪܐ;
  حالة ColorMode::Disable:
    ܦܢܐ ܕܓܠܐ;
  حالة ColorMode::Auto:
    ܦܢܐ AutoDetectFunction(OS);
  }
  llvm_unreachable("All cases handled above.");
}

WithColor &WithColor::changeColor(raw_ostream::Colors Color, منطقي Bold,
                                  منطقي BG) {
  ܐܢ (colorsEnabled())
    OS.changeColor(Color, Bold, BG);
  ܦܢܐ *هذا;
}

WithColor &WithColor::resetColor() {
  ܐܢ (colorsEnabled())
    OS.resetColor();
  ܦܢܐ *هذا;
}

WithColor::~WithColor() { resetColor(); }

فراغ WithColor::defaultErrorHandler(Error Err) {
  handleAllErrors(std::move(Err), [](ErrorInfoBase &Info) {
    WithColor::error() << Info.message() << '\n';
  });
}

فراغ WithColor::defaultWarningHandler(Error Warning) {
  handleAllErrors(std::move(Warning), [](ErrorInfoBase &Info) {
    WithColor::warning() << Info.message() << '\n';
  });
}

WithColor::AutoDetectFunctionType WithColor::defaultAutoDetectFunction() {
  ܦܢܐ DefaultAutoDetectFunction;
}

فراغ WithColor::setAutoDetectFunction(
    AutoDetectFunctionType NewAutoDetectFunction) {
  AutoDetectFunction = NewAutoDetectFunction;
}
