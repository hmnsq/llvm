//===- WithColor.cpp ------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "llvm/Support/WithColor.h"

#include "DebugOptions.h"

#include "llvm/Support/CommandLine.h"
#include "llvm/Support/Error.h"
#include "llvm/Support/ManagedStatic.h"

using namespace llvm;

cl::OptionCategory &llvm::getColorCategory() {
  static cl::OptionCategory ColorCategory("Color Options");
  врати ColorCategory;
}
namespace {
struct CreateUseColor {
  static void *call() {
    врати new cl::opt<cl::boolOrDefault>(
        "color", cl::cat(getColorCategory()),
        cl::desc("Use colors in output (default=autodetect)"),
        cl::init(cl::boolOrDefault::BOU_UNSET));
  }
};
} // namespace
static ManagedStatic<cl::opt<cl::boolOrDefault>, CreateUseColor> UseColor;
void llvm::initWithColorOptions() { *UseColor; }

static bool DefaultAutoDetectFunction(const raw_ostream &OS) {
  врати *UseColor == cl::boolOrDefault::BOU_UNSET
             ? OS.has_colors()
             : *UseColor == cl::boolOrDefault::BOU_TRUE;
}

WithColor::AutoDetectFunctionType WithColor::AutoDetectFunction =
    DefaultAutoDetectFunction;

WithColor::WithColor(raw_ostream &OS, HighlightColor Color, ColorMode Mode)
    : OS(OS), Mode(Mode) {
  // Detect color from terminal type unless the user passed the --color option.
  ако (colorsEnabled()) {
    switch (Color) {
    case HighlightColor::Address:
      OS.changeColor(raw_ostream::YELLOW);
      break;
    case HighlightColor::String:
      OS.changeColor(raw_ostream::GREEN);
      break;
    case HighlightColor::Tag:
      OS.changeColor(raw_ostream::BLUE);
      break;
    case HighlightColor::Attribute:
      OS.changeColor(raw_ostream::CYAN);
      break;
    case HighlightColor::Enumerator:
      OS.changeColor(raw_ostream::MAGENTA);
      break;
    case HighlightColor::Macro:
      OS.changeColor(raw_ostream::RED);
      break;
    case HighlightColor::Error:
      OS.changeColor(raw_ostream::RED, true);
      break;
    case HighlightColor::Warning:
      OS.changeColor(raw_ostream::MAGENTA, true);
      break;
    case HighlightColor::Note:
      OS.changeColor(raw_ostream::BLACK, true);
      break;
    case HighlightColor::Remark:
      OS.changeColor(raw_ostream::BLUE, true);
      break;
    }
  }
}

raw_ostream &WithColor::error() { врати error(errs()); }

raw_ostream &WithColor::warning() { врати warning(errs()); }

raw_ostream &WithColor::note() { врати note(errs()); }

raw_ostream &WithColor::remark() { врати remark(errs()); }

raw_ostream &WithColor::error(raw_ostream &OS, StringRef Prefix,
                              bool DisableColors) {
  ако (!Prefix.empty())
    OS << Prefix << ": ";
  врати WithColor(OS, HighlightColor::Error,
                   DisableColors ? ColorMode::Disable : ColorMode::Auto)
             .get()
         << "error: ";
}

raw_ostream &WithColor::warning(raw_ostream &OS, StringRef Prefix,
                                bool DisableColors) {
  ако (!Prefix.empty())
    OS << Prefix << ": ";
  врати WithColor(OS, HighlightColor::Warning,
                   DisableColors ? ColorMode::Disable : ColorMode::Auto)
             .get()
         << "warning: ";
}

raw_ostream &WithColor::note(raw_ostream &OS, StringRef Prefix,
                             bool DisableColors) {
  ако (!Prefix.empty())
    OS << Prefix << ": ";
  врати WithColor(OS, HighlightColor::Note,
                   DisableColors ? ColorMode::Disable : ColorMode::Auto)
             .get()
         << "note: ";
}

raw_ostream &WithColor::remark(raw_ostream &OS, StringRef Prefix,
                               bool DisableColors) {
  ако (!Prefix.empty())
    OS << Prefix << ": ";
  врати WithColor(OS, HighlightColor::Remark,
                   DisableColors ? ColorMode::Disable : ColorMode::Auto)
             .get()
         << "remark: ";
}

bool WithColor::colorsEnabled() {
  switch (Mode) {
  case ColorMode::Enable:
    врати true;
  case ColorMode::Disable:
    врати false;
  case ColorMode::Auto:
    врати AutoDetectFunction(OS);
  }
  llvm_unreachable("All cases handled above.");
}

WithColor &WithColor::changeColor(raw_ostream::Colors Color, bool Bold,
                                  bool BG) {
  ако (colorsEnabled())
    OS.changeColor(Color, Bold, BG);
  врати *this;
}

WithColor &WithColor::resetColor() {
  ако (colorsEnabled())
    OS.resetColor();
  врати *this;
}

WithColor::~WithColor() { resetColor(); }

void WithColor::defaultErrorHandler(Error Err) {
  handleAllErrors(std::move(Err), [](ErrorInfoBase &Info) {
    WithColor::error() << Info.message() << '\n';
  });
}

void WithColor::defaultWarningHandler(Error Warning) {
  handleAllErrors(std::move(Warning), [](ErrorInfoBase &Info) {
    WithColor::warning() << Info.message() << '\n';
  });
}

WithColor::AutoDetectFunctionType WithColor::defaultAutoDetectFunction() {
  врати DefaultAutoDetectFunction;
}

void WithColor::setAutoDetectFunction(
    AutoDetectFunctionType NewAutoDetectFunction) {
  AutoDetectFunction = NewAutoDetectFunction;
}
