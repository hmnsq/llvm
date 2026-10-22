//===- WithColor.cpp ------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#અન્તર્ભાવ "llvm/Support/WithColor.h"

#અન્તર્ભાવ "DebugOptions.h"

#અન્તર્ભાવ "llvm/Support/CommandLine.h"
#અન્તર્ભાવ "llvm/Support/Error.h"
#અન્તર્ભાવ "llvm/Support/ManagedStatic.h"

using નામાકાશ llvm;

cl::OptionCategory &llvm::getColorCategory() {
  સ્થિર cl::OptionCategory ColorCategory("Color Options");
  પાછા ColorCategory;
}
નામાકાશ {
સંરચના CreateUseColor {
  સ્થિર શૂન્ય *call() {
    પાછા નવું cl::opt<cl::boolOrDefault>(
        "color", cl::cat(getColorCategory()),
        cl::desc("Use colors in output (default=autodetect)"),
        cl::init(cl::boolOrDefault::BOU_UNSET));
  }
};
} // namespace
સ્થિર ManagedStatic<cl::opt<cl::boolOrDefault>, CreateUseColor> UseColor;
શૂન્ય llvm::initWithColorOptions() { *UseColor; }

સ્થિર તાર્કિક DefaultAutoDetectFunction(અચલ raw_ostream &OS) {
  પાછા *UseColor == cl::boolOrDefault::BOU_UNSET
             ? OS.has_colors()
             : *UseColor == cl::boolOrDefault::BOU_TRUE;
}

WithColor::AutoDetectFunctionType WithColor::AutoDetectFunction =
    DefaultAutoDetectFunction;

WithColor::WithColor(raw_ostream &OS, HighlightColor Color, ColorMode Mode)
    : OS(OS), Mode(Mode) {
  // Detect color from terminal type unless the user passed the --color option.
  જો (colorsEnabled()) {
    પસંદગી (Color) {
    સ્થિતિ HighlightColor::Address:
      OS.changeColor(raw_ostream::YELLOW);
      તોડો;
    સ્થિતિ HighlightColor::String:
      OS.changeColor(raw_ostream::GREEN);
      તોડો;
    સ્થિતિ HighlightColor::Tag:
      OS.changeColor(raw_ostream::BLUE);
      તોડો;
    સ્થિતિ HighlightColor::Attribute:
      OS.changeColor(raw_ostream::CYAN);
      તોડો;
    સ્થિતિ HighlightColor::Enumerator:
      OS.changeColor(raw_ostream::MAGENTA);
      તોડો;
    સ્થિતિ HighlightColor::Macro:
      OS.changeColor(raw_ostream::RED);
      તોડો;
    સ્થિતિ HighlightColor::Error:
      OS.changeColor(raw_ostream::RED, સાચું);
      તોડો;
    સ્થિતિ HighlightColor::Warning:
      OS.changeColor(raw_ostream::MAGENTA, સાચું);
      તોડો;
    સ્થિતિ HighlightColor::Note:
      OS.changeColor(raw_ostream::BLACK, સાચું);
      તોડો;
    સ્થિતિ HighlightColor::Remark:
      OS.changeColor(raw_ostream::BLUE, સાચું);
      તોડો;
    }
  }
}

raw_ostream &WithColor::error() { પાછા error(errs()); }

raw_ostream &WithColor::warning() { પાછા warning(errs()); }

raw_ostream &WithColor::note() { પાછા note(errs()); }

raw_ostream &WithColor::remark() { પાછા remark(errs()); }

raw_ostream &WithColor::error(raw_ostream &OS, StringRef Prefix,
                              તાર્કિક DisableColors) {
  જો (!Prefix.empty())
    OS << Prefix << ": ";
  પાછા WithColor(OS, HighlightColor::Error,
                   DisableColors ? ColorMode::Disable : ColorMode::Auto)
             .get()
         << "error: ";
}

raw_ostream &WithColor::warning(raw_ostream &OS, StringRef Prefix,
                                તાર્કિક DisableColors) {
  જો (!Prefix.empty())
    OS << Prefix << ": ";
  પાછા WithColor(OS, HighlightColor::Warning,
                   DisableColors ? ColorMode::Disable : ColorMode::Auto)
             .get()
         << "warning: ";
}

raw_ostream &WithColor::note(raw_ostream &OS, StringRef Prefix,
                             તાર્કિક DisableColors) {
  જો (!Prefix.empty())
    OS << Prefix << ": ";
  પાછા WithColor(OS, HighlightColor::Note,
                   DisableColors ? ColorMode::Disable : ColorMode::Auto)
             .get()
         << "note: ";
}

raw_ostream &WithColor::remark(raw_ostream &OS, StringRef Prefix,
                               તાર્કિક DisableColors) {
  જો (!Prefix.empty())
    OS << Prefix << ": ";
  પાછા WithColor(OS, HighlightColor::Remark,
                   DisableColors ? ColorMode::Disable : ColorMode::Auto)
             .get()
         << "remark: ";
}

તાર્કિક WithColor::colorsEnabled() {
  પસંદગી (Mode) {
  સ્થિતિ ColorMode::Enable:
    પાછા સાચું;
  સ્થિતિ ColorMode::Disable:
    પાછા ખોટું;
  સ્થિતિ ColorMode::Auto:
    પાછા AutoDetectFunction(OS);
  }
  llvm_unreachable("All cases handled above.");
}

WithColor &WithColor::changeColor(raw_ostream::Colors Color, તાર્કિક Bold,
                                  તાર્કિક BG) {
  જો (colorsEnabled())
    OS.changeColor(Color, Bold, BG);
  પાછા *આ;
}

WithColor &WithColor::resetColor() {
  જો (colorsEnabled())
    OS.resetColor();
  પાછા *આ;
}

WithColor::~WithColor() { resetColor(); }

શૂન્ય WithColor::defaultErrorHandler(Error Err) {
  handleAllErrors(std::move(Err), [](ErrorInfoBase &Info) {
    WithColor::error() << Info.message() << '\n';
  });
}

શૂન્ય WithColor::defaultWarningHandler(Error Warning) {
  handleAllErrors(std::move(Warning), [](ErrorInfoBase &Info) {
    WithColor::warning() << Info.message() << '\n';
  });
}

WithColor::AutoDetectFunctionType WithColor::defaultAutoDetectFunction() {
  પાછા DefaultAutoDetectFunction;
}

શૂન્ય WithColor::setAutoDetectFunction(
    AutoDetectFunctionType NewAutoDetectFunction) {
  AutoDetectFunction = NewAutoDetectFunction;
}
