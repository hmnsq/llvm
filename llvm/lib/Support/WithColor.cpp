//===- WithColor.cpp ------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#समावेश "llvm/Support/WithColor.h"

#समावेश "DebugOptions.h"

#समावेश "llvm/Support/CommandLine.h"
#समावेश "llvm/Support/Error.h"
#समावेश "llvm/Support/ManagedStatic.h"

using namespace llvm;

cl::OptionCategory &llvm::getColorCategory() {
  अटल cl::OptionCategory ColorCategory("Color Options");
  वापस ColorCategory;
}
namespace {
काष्ठा CreateUseColor {
  अटल व्योम *call() {
    वापस नया cl::opt<cl::boolOrDefault>(
        "color", cl::cat(getColorCategory()),
        cl::desc("Use colors in output (default=autodetect)"),
        cl::init(cl::boolOrDefault::BOU_UNSET));
  }
};
} // namespace
अटल ManagedStatic<cl::opt<cl::boolOrDefault>, CreateUseColor> UseColor;
व्योम llvm::initWithColorOptions() { *UseColor; }

अटल bool DefaultAutoDetectFunction(स्थिर raw_ostream &OS) {
  वापस *UseColor == cl::boolOrDefault::BOU_UNSET
             ? OS.has_colors()
             : *UseColor == cl::boolOrDefault::BOU_TRUE;
}

WithColor::AutoDetectFunctionType WithColor::AutoDetectFunction =
    DefaultAutoDetectFunction;

WithColor::WithColor(raw_ostream &OS, HighlightColor Color, ColorMode Mode)
    : OS(OS), Mode(Mode) {
  // Detect color from terminal type unless the user passed the --color option.
  अगर (colorsEnabled()) {
    चयन (Color) {
    हाल HighlightColor::Address:
      OS.changeColor(raw_ostream::YELLOW);
      अवरोध;
    हाल HighlightColor::String:
      OS.changeColor(raw_ostream::GREEN);
      अवरोध;
    हाल HighlightColor::Tag:
      OS.changeColor(raw_ostream::BLUE);
      अवरोध;
    हाल HighlightColor::Attribute:
      OS.changeColor(raw_ostream::CYAN);
      अवरोध;
    हाल HighlightColor::Enumerator:
      OS.changeColor(raw_ostream::MAGENTA);
      अवरोध;
    हाल HighlightColor::Macro:
      OS.changeColor(raw_ostream::RED);
      अवरोध;
    हाल HighlightColor::Error:
      OS.changeColor(raw_ostream::RED, true);
      अवरोध;
    हाल HighlightColor::Warning:
      OS.changeColor(raw_ostream::MAGENTA, true);
      अवरोध;
    हाल HighlightColor::Note:
      OS.changeColor(raw_ostream::BLACK, true);
      अवरोध;
    हाल HighlightColor::Remark:
      OS.changeColor(raw_ostream::BLUE, true);
      अवरोध;
    }
  }
}

raw_ostream &WithColor::error() { वापस error(errs()); }

raw_ostream &WithColor::warning() { वापस warning(errs()); }

raw_ostream &WithColor::note() { वापस note(errs()); }

raw_ostream &WithColor::remark() { वापस remark(errs()); }

raw_ostream &WithColor::error(raw_ostream &OS, StringRef Prefix,
                              bool DisableColors) {
  अगर (!Prefix.empty())
    OS << Prefix << ": ";
  वापस WithColor(OS, HighlightColor::Error,
                   DisableColors ? ColorMode::Disable : ColorMode::Auto)
             .लो()
         << "error: ";
}

raw_ostream &WithColor::warning(raw_ostream &OS, StringRef Prefix,
                                bool DisableColors) {
  अगर (!Prefix.empty())
    OS << Prefix << ": ";
  वापस WithColor(OS, HighlightColor::Warning,
                   DisableColors ? ColorMode::Disable : ColorMode::Auto)
             .लो()
         << "warning: ";
}

raw_ostream &WithColor::note(raw_ostream &OS, StringRef Prefix,
                             bool DisableColors) {
  अगर (!Prefix.empty())
    OS << Prefix << ": ";
  वापस WithColor(OS, HighlightColor::Note,
                   DisableColors ? ColorMode::Disable : ColorMode::Auto)
             .लो()
         << "note: ";
}

raw_ostream &WithColor::remark(raw_ostream &OS, StringRef Prefix,
                               bool DisableColors) {
  अगर (!Prefix.empty())
    OS << Prefix << ": ";
  वापस WithColor(OS, HighlightColor::Remark,
                   DisableColors ? ColorMode::Disable : ColorMode::Auto)
             .लो()
         << "remark: ";
}

bool WithColor::colorsEnabled() {
  चयन (Mode) {
  हाल ColorMode::Enable:
    वापस true;
  हाल ColorMode::Disable:
    वापस false;
  हाल ColorMode::Auto:
    वापस AutoDetectFunction(OS);
  }
  llvm_unreachable("All cases handled above.");
}

WithColor &WithColor::changeColor(raw_ostream::Colors Color, bool Bold,
                                  bool BG) {
  अगर (colorsEnabled())
    OS.changeColor(Color, Bold, BG);
  वापस *यह;
}

WithColor &WithColor::resetColor() {
  अगर (colorsEnabled())
    OS.resetColor();
  वापस *यह;
}

WithColor::~WithColor() { resetColor(); }

व्योम WithColor::defaultErrorHandler(Error Err) {
  handleAllErrors(std::move(Err), [](ErrorInfoBase &Info) {
    WithColor::error() << Info.message() << '\n';
  });
}

व्योम WithColor::defaultWarningHandler(Error Warning) {
  handleAllErrors(std::move(Warning), [](ErrorInfoBase &Info) {
    WithColor::warning() << Info.message() << '\n';
  });
}

WithColor::AutoDetectFunctionType WithColor::defaultAutoDetectFunction() {
  वापस DefaultAutoDetectFunction;
}

व्योम WithColor::setAutoDetectFunction(
    AutoDetectFunctionType NewAutoDetectFunction) {
  AutoDetectFunction = NewAutoDetectFunction;
}
