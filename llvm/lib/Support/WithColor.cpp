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
  החזר ColorCategory;
}
نطاق {
بنية CreateUseColor {
  ساكن فراغ *call() {
    החזר جديد cl::opt<cl::boolOrDefault>(
        "color", cl::cat(getColorCategory()),
        cl::desc("Use colors in output (default=autodetect)"),
        cl::init(cl::boolOrDefault::BOU_UNSET));
  }
};
} // namespace
ساكن ManagedStatic<cl::opt<cl::boolOrDefault>, CreateUseColor> UseColor;
فراغ llvm::initWithColorOptions() { *UseColor; }

ساكن منطقي DefaultAutoDetectFunction(ثابت raw_ostream &OS) {
  החזר *UseColor == cl::boolOrDefault::BOU_UNSET
             ? OS.has_colors()
             : *UseColor == cl::boolOrDefault::BOU_TRUE;
}

WithColor::AutoDetectFunctionType WithColor::AutoDetectFunction =
    DefaultAutoDetectFunction;

WithColor::WithColor(raw_ostream &OS, HighlightColor Color, ColorMode Mode)
    : OS(OS), Mode(Mode) {
  // Detect color from terminal type unless the user passed the --color option.
  אם (colorsEnabled()) {
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
      OS.changeColor(raw_ostream::RED, אמת);
      اكسر;
    حالة HighlightColor::Warning:
      OS.changeColor(raw_ostream::MAGENTA, אמת);
      اكسر;
    حالة HighlightColor::Note:
      OS.changeColor(raw_ostream::BLACK, אמת);
      اكسر;
    حالة HighlightColor::Remark:
      OS.changeColor(raw_ostream::BLUE, אמת);
      اكسر;
    }
  }
}

raw_ostream &WithColor::error() { החזר error(errs()); }

raw_ostream &WithColor::warning() { החזר warning(errs()); }

raw_ostream &WithColor::note() { החזר note(errs()); }

raw_ostream &WithColor::remark() { החזר remark(errs()); }

raw_ostream &WithColor::error(raw_ostream &OS, StringRef Prefix,
                              منطقي DisableColors) {
  אם (!Prefix.empty())
    OS << Prefix << ": ";
  החזר WithColor(OS, HighlightColor::Error,
                   DisableColors ? ColorMode::Disable : ColorMode::Auto)
             .get()
         << "error: ";
}

raw_ostream &WithColor::warning(raw_ostream &OS, StringRef Prefix,
                                منطقي DisableColors) {
  אם (!Prefix.empty())
    OS << Prefix << ": ";
  החזר WithColor(OS, HighlightColor::Warning,
                   DisableColors ? ColorMode::Disable : ColorMode::Auto)
             .get()
         << "warning: ";
}

raw_ostream &WithColor::note(raw_ostream &OS, StringRef Prefix,
                             منطقي DisableColors) {
  אם (!Prefix.empty())
    OS << Prefix << ": ";
  החזר WithColor(OS, HighlightColor::Note,
                   DisableColors ? ColorMode::Disable : ColorMode::Auto)
             .get()
         << "note: ";
}

raw_ostream &WithColor::remark(raw_ostream &OS, StringRef Prefix,
                               منطقي DisableColors) {
  אם (!Prefix.empty())
    OS << Prefix << ": ";
  החזר WithColor(OS, HighlightColor::Remark,
                   DisableColors ? ColorMode::Disable : ColorMode::Auto)
             .get()
         << "remark: ";
}

منطقي WithColor::colorsEnabled() {
  بدل (Mode) {
  حالة ColorMode::Enable:
    החזר אמת;
  حالة ColorMode::Disable:
    החזר שקר;
  حالة ColorMode::Auto:
    החזר AutoDetectFunction(OS);
  }
  llvm_unreachable("All cases handled above.");
}

WithColor &WithColor::changeColor(raw_ostream::Colors Color, منطقي Bold,
                                  منطقي BG) {
  אם (colorsEnabled())
    OS.changeColor(Color, Bold, BG);
  החזר *هذا;
}

WithColor &WithColor::resetColor() {
  אם (colorsEnabled())
    OS.resetColor();
  החזר *هذا;
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
  החזר DefaultAutoDetectFunction;
}

فراغ WithColor::setAutoDetectFunction(
    AutoDetectFunctionType NewAutoDetectFunction) {
  AutoDetectFunction = NewAutoDetectFunction;
}
