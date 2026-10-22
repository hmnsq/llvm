//===- WithColor.cpp ------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#అన్తర్భావ "llvm/Support/WithColor.h"

#అన్తర్భావ "DebugOptions.h"

#అన్తర్భావ "llvm/Support/CommandLine.h"
#అన్తర్భావ "llvm/Support/Error.h"
#అన్తర్భావ "llvm/Support/ManagedStatic.h"

ఉపయోగించి నామాకాశ llvm;

cl::OptionCategory &llvm::getColorCategory() {
  స్థిర cl::OptionCategory ColorCategory("Color Options");
  తిరిగి ColorCategory;
}
నామాకాశ {
నిర్మాణం CreateUseColor {
  స్థిర శూన్య *call() {
    తిరిగి కొత్త cl::opt<cl::boolOrDefault>(
        "color", cl::cat(getColorCategory()),
        cl::desc("Use colors in output (default=autodetect)"),
        cl::init(cl::boolOrDefault::BOU_UNSET));
  }
};
} // namespace
స్థిర ManagedStatic<cl::opt<cl::boolOrDefault>, CreateUseColor> UseColor;
శూన్య llvm::initWithColorOptions() { *UseColor; }

స్థిర తార్కిక DefaultAutoDetectFunction(స్థిరరాశి raw_ostream &OS) {
  తిరిగి *UseColor == cl::boolOrDefault::BOU_UNSET
             ? OS.has_colors()
             : *UseColor == cl::boolOrDefault::BOU_TRUE;
}

WithColor::AutoDetectFunctionType WithColor::AutoDetectFunction =
    DefaultAutoDetectFunction;

WithColor::WithColor(raw_ostream &OS, HighlightColor Color, Color_బహుళకం బహుళకం)
    : OS(OS), బహుళకం(బహుళకం) {
  // Detect color from terminal type unless the user passed the --color option.
  ఒకవేళ (colorsEnabled()) {
    ఎంపిక (Color) {
    స్థితి HighlightColor::చిరునామా:
      OS.changeColor(raw_ostream::YELLOW);
      ఆపు;
    స్థితి HighlightColor::String:
      OS.changeColor(raw_ostream::GREEN);
      ఆపు;
    స్థితి HighlightColor::ట్యాగ్:
      OS.changeColor(raw_ostream::BLUE);
      ఆపు;
    స్థితి HighlightColor::Attribute:
      OS.changeColor(raw_ostream::CYAN);
      ఆపు;
    స్థితి HighlightColor::Enumerator:
      OS.changeColor(raw_ostream::MAGENTA);
      ఆపు;
    స్థితి HighlightColor::Macro:
      OS.changeColor(raw_ostream::RED);
      ఆపు;
    స్థితి HighlightColor::Error:
      OS.changeColor(raw_ostream::RED, నిజం);
      ఆపు;
    స్థితి HighlightColor::Warning:
      OS.changeColor(raw_ostream::MAGENTA, నిజం);
      ఆపు;
    స్థితి HighlightColor::Note:
      OS.changeColor(raw_ostream::BLACK, నిజం);
      ఆపు;
    స్థితి HighlightColor::Remark:
      OS.changeColor(raw_ostream::BLUE, నిజం);
      ఆపు;
    }
  }
}

raw_ostream &WithColor::error() { తిరిగి error(errs()); }

raw_ostream &WithColor::warning() { తిరిగి warning(errs()); }

raw_ostream &WithColor::note() { తిరిగి note(errs()); }

raw_ostream &WithColor::remark() { తిరిగి remark(errs()); }

raw_ostream &WithColor::error(raw_ostream &OS, StringRef ఉపసర్గ,
                              తార్కిక DisableColors) {
  ఒకవేళ (!ఉపసర్గ.ఖాళీలు())
    OS << ఉపసర్గ << ": ";
  తిరిగి WithColor(OS, HighlightColor::Error,
                   DisableColors ? Color_బహుళకం::Disable : Color_బహుళకం::Auto)
             .get()
         << "error: ";
}

raw_ostream &WithColor::warning(raw_ostream &OS, StringRef ఉపసర్గ,
                                తార్కిక DisableColors) {
  ఒకవేళ (!ఉపసర్గ.ఖాళీలు())
    OS << ఉపసర్గ << ": ";
  తిరిగి WithColor(OS, HighlightColor::Warning,
                   DisableColors ? Color_బహుళకం::Disable : Color_బహుళకం::Auto)
             .get()
         << "warning: ";
}

raw_ostream &WithColor::note(raw_ostream &OS, StringRef ఉపసర్గ,
                             తార్కిక DisableColors) {
  ఒకవేళ (!ఉపసర్గ.ఖాళీలు())
    OS << ఉపసర్గ << ": ";
  తిరిగి WithColor(OS, HighlightColor::Note,
                   DisableColors ? Color_బహుళకం::Disable : Color_బహుళకం::Auto)
             .get()
         << "note: ";
}

raw_ostream &WithColor::remark(raw_ostream &OS, StringRef ఉపసర్గ,
                               తార్కిక DisableColors) {
  ఒకవేళ (!ఉపసర్గ.ఖాళీలు())
    OS << ఉపసర్గ << ": ";
  తిరిగి WithColor(OS, HighlightColor::Remark,
                   DisableColors ? Color_బహుళకం::Disable : Color_బహుళకం::Auto)
             .get()
         << "remark: ";
}

తార్కిక WithColor::colorsEnabled() {
  ఎంపిక (బహుళకం) {
  స్థితి Color_బహుళకం::Enable:
    తిరిగి నిజం;
  స్థితి Color_బహుళకం::Disable:
    తిరిగి అబద్ధం;
  స్థితి Color_బహుళకం::Auto:
    తిరిగి AutoDetectFunction(OS);
  }
  llvm_unreachable("All cases handled above.");
}

WithColor &WithColor::changeColor(raw_ostream::Colors Color, తార్కిక Bold,
                                  తార్కిక BG) {
  ఒకవేళ (colorsEnabled())
    OS.changeColor(Color, Bold, BG);
  తిరిగి *ఇది;
}

WithColor &WithColor::resetColor() {
  ఒకవేళ (colorsEnabled())
    OS.resetColor();
  తిరిగి *ఇది;
}

WithColor::~WithColor() { resetColor(); }

శూన్య WithColor::defaultErrorHandler(Error Err) {
  handleAllErrors(std::move(Err), [](Error_Info_పునాది &Info) {
    WithColor::error() << Info.సందేశం_2() << '\n';
  });
}

శూన్య WithColor::defaultWarningHandler(Error Warning) {
  handleAllErrors(std::move(Warning), [](Error_Info_పునాది &Info) {
    WithColor::warning() << Info.సందేశం_2() << '\n';
  });
}

WithColor::AutoDetectFunctionType WithColor::defaultAutoDetectFunction() {
  తిరిగి DefaultAutoDetectFunction;
}

శూన్య WithColor::సమితి_Auto_Detect_Function(
    AutoDetectFunctionType NewAutoDetectFunction) {
  AutoDetectFunction = NewAutoDetectFunction;
}
