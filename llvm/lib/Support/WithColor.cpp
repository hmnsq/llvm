//===- WithColor.cpp ------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#अन्तर्भाव "llvm/Support/WithColor.h"

#अन्तर्भाव "DebugOptions.h"

#अन्तर्भाव "llvm/Support/CommandLine.h"
#अन्तर्भाव "llvm/Support/Error.h"
#अन्तर्भाव "llvm/Support/ManagedStatic.h"

using नामाकाश llvm;

cl::OptionCategory &llvm::getColorCategory() {
  स्थिर cl::OptionCategory ColorCategory("Color Options");
  पटिनिवत्त ColorCategory;
}
नामाकाश {
संरचना CreateUseColor {
  स्थिर शून्य *call() {
    पटिनिवत्त नूतन cl::opt<cl::boolOrDefault>(
        "color", cl::cat(getColorCategory()),
        cl::desc("Use colors in output (default=autodetect)"),
        cl::init(cl::boolOrDefault::BOU_UNSET));
  }
};
} // namespace
स्थिर ManagedStatic<cl::opt<cl::boolOrDefault>, CreateUseColor> UseColor;
शून्य llvm::initWithColorOptions() { *UseColor; }

स्थिर तार्किक DefaultAutoDetectFunction(ध्रुव raw_ostream &OS) {
  पटिनिवत्त *UseColor == cl::boolOrDefault::BOU_UNSET
             ? OS.has_colors()
             : *UseColor == cl::boolOrDefault::BOU_TRUE;
}

WithColor::AutoDetectFunctionType WithColor::AutoDetectFunction =
    DefaultAutoDetectFunction;

WithColor::WithColor(raw_ostream &OS, HighlightColor Color, ColorMode Mode)
    : OS(OS), Mode(Mode) {
  // Detect color from terminal type unless the user passed the --color option.
  सचे (colorsEnabled()) {
    विकल्प (Color) {
    पक्ष HighlightColor::Address:
      OS.changeColor(raw_ostream::YELLOW);
      भङ्ग;
    पक्ष HighlightColor::String:
      OS.changeColor(raw_ostream::GREEN);
      भङ्ग;
    पक्ष HighlightColor::Tag:
      OS.changeColor(raw_ostream::BLUE);
      भङ्ग;
    पक्ष HighlightColor::Attribute:
      OS.changeColor(raw_ostream::CYAN);
      भङ्ग;
    पक्ष HighlightColor::Enumerator:
      OS.changeColor(raw_ostream::MAGENTA);
      भङ्ग;
    पक्ष HighlightColor::Macro:
      OS.changeColor(raw_ostream::RED);
      भङ्ग;
    पक्ष HighlightColor::Error:
      OS.changeColor(raw_ostream::RED, सच्च);
      भङ्ग;
    पक्ष HighlightColor::Warning:
      OS.changeColor(raw_ostream::MAGENTA, सच्च);
      भङ्ग;
    पक्ष HighlightColor::Note:
      OS.changeColor(raw_ostream::BLACK, सच्च);
      भङ्ग;
    पक्ष HighlightColor::Remark:
      OS.changeColor(raw_ostream::BLUE, सच्च);
      भङ्ग;
    }
  }
}

raw_ostream &WithColor::error() { पटिनिवत्त error(errs()); }

raw_ostream &WithColor::warning() { पटिनिवत्त warning(errs()); }

raw_ostream &WithColor::note() { पटिनिवत्त note(errs()); }

raw_ostream &WithColor::remark() { पटिनिवत्त remark(errs()); }

raw_ostream &WithColor::error(raw_ostream &OS, StringRef Prefix,
                              तार्किक DisableColors) {
  सचे (!Prefix.empty())
    OS << Prefix << ": ";
  पटिनिवत्त WithColor(OS, HighlightColor::Error,
                   DisableColors ? ColorMode::Disable : ColorMode::Auto)
             .get()
         << "error: ";
}

raw_ostream &WithColor::warning(raw_ostream &OS, StringRef Prefix,
                                तार्किक DisableColors) {
  सचे (!Prefix.empty())
    OS << Prefix << ": ";
  पटिनिवत्त WithColor(OS, HighlightColor::Warning,
                   DisableColors ? ColorMode::Disable : ColorMode::Auto)
             .get()
         << "warning: ";
}

raw_ostream &WithColor::note(raw_ostream &OS, StringRef Prefix,
                             तार्किक DisableColors) {
  सचे (!Prefix.empty())
    OS << Prefix << ": ";
  पटिनिवत्त WithColor(OS, HighlightColor::Note,
                   DisableColors ? ColorMode::Disable : ColorMode::Auto)
             .get()
         << "note: ";
}

raw_ostream &WithColor::remark(raw_ostream &OS, StringRef Prefix,
                               तार्किक DisableColors) {
  सचे (!Prefix.empty())
    OS << Prefix << ": ";
  पटिनिवत्त WithColor(OS, HighlightColor::Remark,
                   DisableColors ? ColorMode::Disable : ColorMode::Auto)
             .get()
         << "remark: ";
}

तार्किक WithColor::colorsEnabled() {
  विकल्प (Mode) {
  पक्ष ColorMode::Enable:
    पटिनिवत्त सच्च;
  पक्ष ColorMode::Disable:
    पटिनिवत्त मिच्छा;
  पक्ष ColorMode::Auto:
    पटिनिवत्त AutoDetectFunction(OS);
  }
  llvm_unreachable("All cases handled above.");
}

WithColor &WithColor::changeColor(raw_ostream::Colors Color, तार्किक Bold,
                                  तार्किक BG) {
  सचे (colorsEnabled())
    OS.changeColor(Color, Bold, BG);
  पटिनिवत्त *एतत्;
}

WithColor &WithColor::resetColor() {
  सचे (colorsEnabled())
    OS.resetColor();
  पटिनिवत्त *एतत्;
}

WithColor::~WithColor() { resetColor(); }

शून्य WithColor::defaultErrorHandler(Error Err) {
  handleAllErrors(std::move(Err), [](ErrorInfoBase &Info) {
    WithColor::error() << Info.message() << '\n';
  });
}

शून्य WithColor::defaultWarningHandler(Error Warning) {
  handleAllErrors(std::move(Warning), [](ErrorInfoBase &Info) {
    WithColor::warning() << Info.message() << '\n';
  });
}

WithColor::AutoDetectFunctionType WithColor::defaultAutoDetectFunction() {
  पटिनिवत्त DefaultAutoDetectFunction;
}

शून्य WithColor::setAutoDetectFunction(
    AutoDetectFunctionType NewAutoDetectFunction) {
  AutoDetectFunction = NewAutoDetectFunction;
}
