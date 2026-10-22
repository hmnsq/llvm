//===- WithColor.cpp ------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#অন্তর্ভাব "llvm/Support/WithColor.h"

#অন্তর্ভাব "DebugOptions.h"

#অন্তর্ভাব "llvm/Support/CommandLine.h"
#অন্তর্ভাব "llvm/Support/Error.h"
#অন্তর্ভাব "llvm/Support/ManagedStatic.h"

using নামাকাশ llvm;

cl::OptionCategory &llvm::getColorCategory() {
  স্থির cl::OptionCategory ColorCategory("Color Options");
  ফেরত ColorCategory;
}
নামাকাশ {
কাঠামো CreateUseColor {
  স্থির শূন্য *call() {
    ফেরত নতুন cl::opt<cl::boolOrDefault>(
        "color", cl::cat(getColorCategory()),
        cl::desc("Use colors in output (default=autodetect)"),
        cl::init(cl::boolOrDefault::BOU_UNSET));
  }
};
} // namespace
স্থির ManagedStatic<cl::opt<cl::boolOrDefault>, CreateUseColor> UseColor;
শূন্য llvm::initWithColorOptions() { *UseColor; }

স্থির যৌক্তিক DefaultAutoDetectFunction(ধ্রুবক raw_ostream &OS) {
  ফেরত *UseColor == cl::boolOrDefault::BOU_UNSET
             ? OS.has_colors()
             : *UseColor == cl::boolOrDefault::BOU_TRUE;
}

WithColor::AutoDetectFunctionType WithColor::AutoDetectFunction =
    DefaultAutoDetectFunction;

WithColor::WithColor(raw_ostream &OS, HighlightColor Color, ColorMode Mode)
    : OS(OS), Mode(Mode) {
  // Detect color from terminal type unless the user passed the --color option.
  যদি (colorsEnabled()) {
    নির্বাচন (Color) {
    ক্ষেত্র HighlightColor::Address:
      OS.changeColor(raw_ostream::YELLOW);
      বিরতি;
    ক্ষেত্র HighlightColor::String:
      OS.changeColor(raw_ostream::GREEN);
      বিরতি;
    ক্ষেত্র HighlightColor::Tag:
      OS.changeColor(raw_ostream::BLUE);
      বিরতি;
    ক্ষেত্র HighlightColor::Attribute:
      OS.changeColor(raw_ostream::CYAN);
      বিরতি;
    ক্ষেত্র HighlightColor::Enumerator:
      OS.changeColor(raw_ostream::MAGENTA);
      বিরতি;
    ক্ষেত্র HighlightColor::Macro:
      OS.changeColor(raw_ostream::RED);
      বিরতি;
    ক্ষেত্র HighlightColor::Error:
      OS.changeColor(raw_ostream::RED, সত্য);
      বিরতি;
    ক্ষেত্র HighlightColor::Warning:
      OS.changeColor(raw_ostream::MAGENTA, সত্য);
      বিরতি;
    ক্ষেত্র HighlightColor::Note:
      OS.changeColor(raw_ostream::BLACK, সত্য);
      বিরতি;
    ক্ষেত্র HighlightColor::Remark:
      OS.changeColor(raw_ostream::BLUE, সত্য);
      বিরতি;
    }
  }
}

raw_ostream &WithColor::error() { ফেরত error(errs()); }

raw_ostream &WithColor::warning() { ফেরত warning(errs()); }

raw_ostream &WithColor::note() { ফেরত note(errs()); }

raw_ostream &WithColor::remark() { ফেরত remark(errs()); }

raw_ostream &WithColor::error(raw_ostream &OS, StringRef Prefix,
                              যৌক্তিক DisableColors) {
  যদি (!Prefix.empty())
    OS << Prefix << ": ";
  ফেরত WithColor(OS, HighlightColor::Error,
                   DisableColors ? ColorMode::Disable : ColorMode::Auto)
             .get()
         << "error: ";
}

raw_ostream &WithColor::warning(raw_ostream &OS, StringRef Prefix,
                                যৌক্তিক DisableColors) {
  যদি (!Prefix.empty())
    OS << Prefix << ": ";
  ফেরত WithColor(OS, HighlightColor::Warning,
                   DisableColors ? ColorMode::Disable : ColorMode::Auto)
             .get()
         << "warning: ";
}

raw_ostream &WithColor::note(raw_ostream &OS, StringRef Prefix,
                             যৌক্তিক DisableColors) {
  যদি (!Prefix.empty())
    OS << Prefix << ": ";
  ফেরত WithColor(OS, HighlightColor::Note,
                   DisableColors ? ColorMode::Disable : ColorMode::Auto)
             .get()
         << "note: ";
}

raw_ostream &WithColor::remark(raw_ostream &OS, StringRef Prefix,
                               যৌক্তিক DisableColors) {
  যদি (!Prefix.empty())
    OS << Prefix << ": ";
  ফেরত WithColor(OS, HighlightColor::Remark,
                   DisableColors ? ColorMode::Disable : ColorMode::Auto)
             .get()
         << "remark: ";
}

যৌক্তিক WithColor::colorsEnabled() {
  নির্বাচন (Mode) {
  ক্ষেত্র ColorMode::Enable:
    ফেরত সত্য;
  ক্ষেত্র ColorMode::Disable:
    ফেরত মিথ্যা;
  ক্ষেত্র ColorMode::Auto:
    ফেরত AutoDetectFunction(OS);
  }
  llvm_unreachable("All cases handled above.");
}

WithColor &WithColor::changeColor(raw_ostream::Colors Color, যৌক্তিক Bold,
                                  যৌক্তিক BG) {
  যদি (colorsEnabled())
    OS.changeColor(Color, Bold, BG);
  ফেরত *এই;
}

WithColor &WithColor::resetColor() {
  যদি (colorsEnabled())
    OS.resetColor();
  ফেরত *এই;
}

WithColor::~WithColor() { resetColor(); }

শূন্য WithColor::defaultErrorHandler(Error Err) {
  handleAllErrors(std::move(Err), [](ErrorInfoBase &Info) {
    WithColor::error() << Info.message() << '\n';
  });
}

শূন্য WithColor::defaultWarningHandler(Error Warning) {
  handleAllErrors(std::move(Warning), [](ErrorInfoBase &Info) {
    WithColor::warning() << Info.message() << '\n';
  });
}

WithColor::AutoDetectFunctionType WithColor::defaultAutoDetectFunction() {
  ফেরত DefaultAutoDetectFunction;
}

শূন্য WithColor::setAutoDetectFunction(
    AutoDetectFunctionType NewAutoDetectFunction) {
  AutoDetectFunction = NewAutoDetectFunction;
}
