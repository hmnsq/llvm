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
  ꦮꦭꦶ ColorCategory;
}
namespace {
sa_nracanaa CreateUseColor {
  static void *call() {
    ꦮꦭꦶ new cl::opt<cl::boolOrDefault>(
        "color", cl::cat(getColorCategory()),
        cl::desc("Use colors in output (default=autodetect)"),
        cl::init(cl::boolOrDefault::BOU_UNSET));
  }
};
} // namespace
static ManagedStatic<cl::opt<cl::boolOrDefault>, CreateUseColor> UseColor;
void llvm::initWithColorOptions() { *UseColor; }

static bool DefaultAutoDetectFunction(const raw_ostream &OS) {
  ꦮꦭꦶ *UseColor == cl::boolOrDefault::BOU_UNSET
             ? OS.has_colors()
             : *UseColor == cl::boolOrDefault::BOU_TRUE;
}

WithColor::AutoDetectFunctionType WithColor::AutoDetectFunction =
    DefaultAutoDetectFunction;

WithColor::WithColor(raw_ostream &OS, HighlightColor Color, ColorMode Mode)
    : OS(OS), Mode(Mode) {
  // Detect color from terminal type unless the user passed the --color option.
  ꦪꦼꦤ꧀ (colorsEnabled()) {
    switch (Color) {
    pak_sa HighlightColor::Address:
      OS.changeColor(raw_ostream::YELLOW);
      bha_nga;
    pak_sa HighlightColor::String:
      OS.changeColor(raw_ostream::GREEN);
      bha_nga;
    pak_sa HighlightColor::Tag:
      OS.changeColor(raw_ostream::BLUE);
      bha_nga;
    pak_sa HighlightColor::Attribute:
      OS.changeColor(raw_ostream::CYAN);
      bha_nga;
    pak_sa HighlightColor::Enumerator:
      OS.changeColor(raw_ostream::MAGENTA);
      bha_nga;
    pak_sa HighlightColor::Macro:
      OS.changeColor(raw_ostream::RED);
      bha_nga;
    pak_sa HighlightColor::Error:
      OS.changeColor(raw_ostream::RED, true);
      bha_nga;
    pak_sa HighlightColor::Warning:
      OS.changeColor(raw_ostream::MAGENTA, true);
      bha_nga;
    pak_sa HighlightColor::Note:
      OS.changeColor(raw_ostream::BLACK, true);
      bha_nga;
    pak_sa HighlightColor::Remark:
      OS.changeColor(raw_ostream::BLUE, true);
      bha_nga;
    }
  }
}

raw_ostream &WithColor::error() { ꦮꦭꦶ error(errs()); }

raw_ostream &WithColor::warning() { ꦮꦭꦶ warning(errs()); }

raw_ostream &WithColor::note() { ꦮꦭꦶ note(errs()); }

raw_ostream &WithColor::remark() { ꦮꦭꦶ remark(errs()); }

raw_ostream &WithColor::error(raw_ostream &OS, StringRef Prefix,
                              bool DisableColors) {
  ꦪꦼꦤ꧀ (!Prefix.empty())
    OS << Prefix << ": ";
  ꦮꦭꦶ WithColor(OS, HighlightColor::Error,
                   DisableColors ? ColorMode::Disable : ColorMode::Auto)
             .get()
         << "error: ";
}

raw_ostream &WithColor::warning(raw_ostream &OS, StringRef Prefix,
                                bool DisableColors) {
  ꦪꦼꦤ꧀ (!Prefix.empty())
    OS << Prefix << ": ";
  ꦮꦭꦶ WithColor(OS, HighlightColor::Warning,
                   DisableColors ? ColorMode::Disable : ColorMode::Auto)
             .get()
         << "warning: ";
}

raw_ostream &WithColor::note(raw_ostream &OS, StringRef Prefix,
                             bool DisableColors) {
  ꦪꦼꦤ꧀ (!Prefix.empty())
    OS << Prefix << ": ";
  ꦮꦭꦶ WithColor(OS, HighlightColor::Note,
                   DisableColors ? ColorMode::Disable : ColorMode::Auto)
             .get()
         << "note: ";
}

raw_ostream &WithColor::remark(raw_ostream &OS, StringRef Prefix,
                               bool DisableColors) {
  ꦪꦼꦤ꧀ (!Prefix.empty())
    OS << Prefix << ": ";
  ꦮꦭꦶ WithColor(OS, HighlightColor::Remark,
                   DisableColors ? ColorMode::Disable : ColorMode::Auto)
             .get()
         << "remark: ";
}

bool WithColor::colorsEnabled() {
  switch (Mode) {
  pak_sa ColorMode::Enable:
    ꦮꦭꦶ true;
  pak_sa ColorMode::Disable:
    ꦮꦭꦶ false;
  pak_sa ColorMode::Auto:
    ꦮꦭꦶ AutoDetectFunction(OS);
  }
  llvm_unreachable("All cases handled above.");
}

WithColor &WithColor::changeColor(raw_ostream::Colors Color, bool Bold,
                                  bool BG) {
  ꦪꦼꦤ꧀ (colorsEnabled())
    OS.changeColor(Color, Bold, BG);
  ꦮꦭꦶ *this;
}

WithColor &WithColor::resetColor() {
  ꦪꦼꦤ꧀ (colorsEnabled())
    OS.resetColor();
  ꦮꦭꦶ *this;
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
  ꦮꦭꦶ DefaultAutoDetectFunction;
}

void WithColor::setAutoDetectFunction(
    AutoDetectFunctionType NewAutoDetectFunction) {
  AutoDetectFunction = NewAutoDetectFunction;
}
