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

استعمال نطاق llvm;

cl::OptionCategory &llvm::getColorCategory() {
  ساكن cl::OptionCategory ColorCategory("Color Options");
  أرجع ColorCategory;
}
نطاق {
بنية CreateUseColor {
  ساكن فراغ *call() {
    أرجع جديد cl::opt<cl::boolOrDefault>(
        "color", cl::cat(getColorCategory()),
        cl::desc("Use colors in output (default=autodetect)"),
        cl::init(cl::boolOrDefault::BOU_UNSET));
  }
};
} // namespace
ساكن ManagedStatic<cl::opt<cl::boolOrDefault>, CreateUseColor> UseColor;
فراغ llvm::initWithColorOptions() { *UseColor; }

ساكن منطقي DefaultAutoDetectFunction(ثابت raw_ostream &OS) {
  أرجع *UseColor == cl::boolOrDefault::BOU_UNSET
             ? OS.has_colors()
             : *UseColor == cl::boolOrDefault::BOU_TRUE;
}

WithColor::AutoDetectFunctionType WithColor::AutoDetectFunction =
    DefaultAutoDetectFunction;

WithColor::WithColor(raw_ostream &OS, HighlightColor Color, Color_گهڻائي گهڻائي)
    : OS(OS), گهڻائي(گهڻائي) {
  // Detect color from terminal type unless the user passed the --color option.
  إذا (colorsEnabled()) {
    بدل (Color) {
    حالة HighlightColor::پتو:
      OS.changeColor(raw_ostream::YELLOW);
      اكسر;
    حالة HighlightColor::String:
      OS.changeColor(raw_ostream::GREEN);
      اكسر;
    حالة HighlightColor::ٽيگ:
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
      OS.changeColor(raw_ostream::RED, سچ);
      اكسر;
    حالة HighlightColor::Warning:
      OS.changeColor(raw_ostream::MAGENTA, سچ);
      اكسر;
    حالة HighlightColor::Note:
      OS.changeColor(raw_ostream::BLACK, سچ);
      اكسر;
    حالة HighlightColor::Remark:
      OS.changeColor(raw_ostream::BLUE, سچ);
      اكسر;
    }
  }
}

raw_ostream &WithColor::error() { أرجع error(errs()); }

raw_ostream &WithColor::warning() { أرجع warning(errs()); }

raw_ostream &WithColor::note() { أرجع note(errs()); }

raw_ostream &WithColor::remark() { أرجع remark(errs()); }

raw_ostream &WithColor::error(raw_ostream &OS, StringRef اڳياڙي,
                              منطقي DisableColors) {
  إذا (!اڳياڙي.خالي_جايون())
    OS << اڳياڙي << ": ";
  أرجع WithColor(OS, HighlightColor::Error,
                   DisableColors ? Color_گهڻائي::Disable : Color_گهڻائي::Auto)
             .get()
         << "error: ";
}

raw_ostream &WithColor::warning(raw_ostream &OS, StringRef اڳياڙي,
                                منطقي DisableColors) {
  إذا (!اڳياڙي.خالي_جايون())
    OS << اڳياڙي << ": ";
  أرجع WithColor(OS, HighlightColor::Warning,
                   DisableColors ? Color_گهڻائي::Disable : Color_گهڻائي::Auto)
             .get()
         << "warning: ";
}

raw_ostream &WithColor::note(raw_ostream &OS, StringRef اڳياڙي,
                             منطقي DisableColors) {
  إذا (!اڳياڙي.خالي_جايون())
    OS << اڳياڙي << ": ";
  أرجع WithColor(OS, HighlightColor::Note,
                   DisableColors ? Color_گهڻائي::Disable : Color_گهڻائي::Auto)
             .get()
         << "note: ";
}

raw_ostream &WithColor::remark(raw_ostream &OS, StringRef اڳياڙي,
                               منطقي DisableColors) {
  إذا (!اڳياڙي.خالي_جايون())
    OS << اڳياڙي << ": ";
  أرجع WithColor(OS, HighlightColor::Remark,
                   DisableColors ? Color_گهڻائي::Disable : Color_گهڻائي::Auto)
             .get()
         << "remark: ";
}

منطقي WithColor::colorsEnabled() {
  بدل (گهڻائي) {
  حالة Color_گهڻائي::Enable:
    أرجع سچ;
  حالة Color_گهڻائي::Disable:
    أرجع خطأ;
  حالة Color_گهڻائي::Auto:
    أرجع AutoDetectFunction(OS);
  }
  llvm_unreachable("All cases handled above.");
}

WithColor &WithColor::changeColor(raw_ostream::Colors Color, منطقي Bold,
                                  منطقي BG) {
  إذا (colorsEnabled())
    OS.changeColor(Color, Bold, BG);
  أرجع *هذا;
}

WithColor &WithColor::resetColor() {
  إذا (colorsEnabled())
    OS.resetColor();
  أرجع *هذا;
}

WithColor::~WithColor() { resetColor(); }

فراغ WithColor::defaultErrorHandler(Error Err) {
  handleAllErrors(std::move(Err), [](Error_Info_بنياد &Info) {
    WithColor::error() << Info.پيغام() << '\n';
  });
}

فراغ WithColor::defaultWarningHandler(Error Warning) {
  handleAllErrors(std::move(Warning), [](Error_Info_بنياد &Info) {
    WithColor::warning() << Info.پيغام() << '\n';
  });
}

WithColor::AutoDetectFunctionType WithColor::defaultAutoDetectFunction() {
  أرجع DefaultAutoDetectFunction;
}

فراغ WithColor::سيٽ_Auto_Detect_Function(
    AutoDetectFunctionType NewAutoDetectFunction) {
  AutoDetectFunction = NewAutoDetectFunction;
}
