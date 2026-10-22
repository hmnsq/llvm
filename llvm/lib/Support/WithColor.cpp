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

استعمال نامخلا llvm;

cl::OptionCategory &llvm::getColorCategory() {
  ساکن cl::OptionCategory ColorCategory("Color Options");
  واپس ColorCategory;
}
نامخلا {
ساخت CreateUseColor {
  ساکن خالی *call() {
    واپس نیا cl::opt<cl::boolOrDefault>(
        "color", cl::cat(getColorCategory()),
        cl::desc("Use colors in output (default=autodetect)"),
        cl::init(cl::boolOrDefault::BOU_UNSET));
  }
};
} // namespace
ساکن ManagedStatic<cl::opt<cl::boolOrDefault>, CreateUseColor> UseColor;
خالی llvm::initWithColorOptions() { *UseColor; }

ساکن منطقی DefaultAutoDetectFunction(ثابت raw_ostream &OS) {
  واپس *UseColor == cl::boolOrDefault::BOU_UNSET
             ? OS.has_colors()
             : *UseColor == cl::boolOrDefault::BOU_TRUE;
}

WithColor::AutoDetectFunctionType WithColor::AutoDetectFunction =
    DefaultAutoDetectFunction;

WithColor::WithColor(raw_ostream &OS, HighlightColor Color, Color_منوال منوال)
    : OS(OS), منوال(منوال) {
  // Detect color from terminal type unless the user passed the --color option.
  اگر (colorsEnabled()) {
    انتخاب (Color) {
    حالت HighlightColor::پتہ:
      OS.changeColor(raw_ostream::YELLOW);
      توڑو;
    حالت HighlightColor::String:
      OS.changeColor(raw_ostream::GREEN);
      توڑو;
    حالت HighlightColor::لیبل:
      OS.changeColor(raw_ostream::BLUE);
      توڑو;
    حالت HighlightColor::Attribute:
      OS.changeColor(raw_ostream::CYAN);
      توڑو;
    حالت HighlightColor::Enumerator:
      OS.changeColor(raw_ostream::MAGENTA);
      توڑو;
    حالت HighlightColor::Macro:
      OS.changeColor(raw_ostream::RED);
      توڑو;
    حالت HighlightColor::Error:
      OS.changeColor(raw_ostream::RED, سچ);
      توڑو;
    حالت HighlightColor::Warning:
      OS.changeColor(raw_ostream::MAGENTA, سچ);
      توڑو;
    حالت HighlightColor::Note:
      OS.changeColor(raw_ostream::BLACK, سچ);
      توڑو;
    حالت HighlightColor::Remark:
      OS.changeColor(raw_ostream::BLUE, سچ);
      توڑو;
    }
  }
}

raw_ostream &WithColor::error() { واپس error(errs()); }

raw_ostream &WithColor::warning() { واپس warning(errs()); }

raw_ostream &WithColor::note() { واپس note(errs()); }

raw_ostream &WithColor::remark() { واپس remark(errs()); }

raw_ostream &WithColor::error(raw_ostream &OS, StringRef سابقہ,
                              منطقی DisableColors) {
  اگر (!سابقہ.خالی_خانے())
    OS << سابقہ << ": ";
  واپس WithColor(OS, HighlightColor::Error,
                   DisableColors ? Color_منوال::Disable : Color_منوال::Auto)
             .get()
         << "error: ";
}

raw_ostream &WithColor::warning(raw_ostream &OS, StringRef سابقہ,
                                منطقی DisableColors) {
  اگر (!سابقہ.خالی_خانے())
    OS << سابقہ << ": ";
  واپس WithColor(OS, HighlightColor::Warning,
                   DisableColors ? Color_منوال::Disable : Color_منوال::Auto)
             .get()
         << "warning: ";
}

raw_ostream &WithColor::note(raw_ostream &OS, StringRef سابقہ,
                             منطقی DisableColors) {
  اگر (!سابقہ.خالی_خانے())
    OS << سابقہ << ": ";
  واپس WithColor(OS, HighlightColor::Note,
                   DisableColors ? Color_منوال::Disable : Color_منوال::Auto)
             .get()
         << "note: ";
}

raw_ostream &WithColor::remark(raw_ostream &OS, StringRef سابقہ,
                               منطقی DisableColors) {
  اگر (!سابقہ.خالی_خانے())
    OS << سابقہ << ": ";
  واپس WithColor(OS, HighlightColor::Remark,
                   DisableColors ? Color_منوال::Disable : Color_منوال::Auto)
             .get()
         << "remark: ";
}

منطقی WithColor::colorsEnabled() {
  انتخاب (منوال) {
  حالت Color_منوال::Enable:
    واپس سچ;
  حالت Color_منوال::Disable:
    واپس جھوٹ;
  حالت Color_منوال::Auto:
    واپس AutoDetectFunction(OS);
  }
  llvm_unreachable("All cases handled above.");
}

WithColor &WithColor::changeColor(raw_ostream::Colors Color, منطقی Bold,
                                  منطقی BG) {
  اگر (colorsEnabled())
    OS.changeColor(Color, Bold, BG);
  واپس *یہ;
}

WithColor &WithColor::resetColor() {
  اگر (colorsEnabled())
    OS.resetColor();
  واپس *یہ;
}

WithColor::~WithColor() { resetColor(); }

خالی WithColor::defaultErrorHandler(Error Err) {
  handleAllErrors(std::move(Err), [](Error_Info_بنیاد &Info) {
    WithColor::error() << Info.پیغام_2() << '\n';
  });
}

خالی WithColor::defaultWarningHandler(Error Warning) {
  handleAllErrors(std::move(Warning), [](Error_Info_بنیاد &Info) {
    WithColor::warning() << Info.پیغام_2() << '\n';
  });
}

WithColor::AutoDetectFunctionType WithColor::defaultAutoDetectFunction() {
  واپس DefaultAutoDetectFunction;
}

خالی WithColor::سیٹ_Auto_Detect_Function(
    AutoDetectFunctionType NewAutoDetectFunction) {
  AutoDetectFunction = NewAutoDetectFunction;
}
