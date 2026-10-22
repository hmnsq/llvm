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
  प्रत्यागम ColorCategory;
}
नामाकाश {
संरचना CreateUseColor {
  स्थिर शून्य *call() {
    प्रत्यागम नूतन cl::opt<cl::boolOrDefault>(
        "color", cl::cat(getColorCategory()),
        cl::desc("Use colors in output (default=autodetect)"),
        cl::init(cl::boolOrDefault::BOU_UNSET));
  }
};
} // namespace
स्थिर ManagedStatic<cl::opt<cl::boolOrDefault>, CreateUseColor> UseColor;
शून्य llvm::initWithColorOptions() { *UseColor; }

स्थिर तार्किक DefaultAutoDetectFunction(ध्रुव raw_ostream &OS) {
  प्रत्यागम *UseColor == cl::boolOrDefault::BOU_UNSET
             ? OS.has_colors()
             : *UseColor == cl::boolOrDefault::BOU_TRUE;
}

WithColor::AutoDetectFunctionType WithColor::AutoDetectFunction =
    DefaultAutoDetectFunction;

WithColor::WithColor(raw_ostream &OS, HighlightColor Color, ColorMode Mode)
    : OS(OS), Mode(Mode) {
  // Detect color from terminal type unless the user passed the --color option.
  यदि (colorsEnabled()) {
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
      OS.changeColor(raw_ostream::RED, सत्य);
      भङ्ग;
    पक्ष HighlightColor::Warning:
      OS.changeColor(raw_ostream::MAGENTA, सत्य);
      भङ्ग;
    पक्ष HighlightColor::Note:
      OS.changeColor(raw_ostream::BLACK, सत्य);
      भङ्ग;
    पक्ष HighlightColor::Remark:
      OS.changeColor(raw_ostream::BLUE, सत्य);
      भङ्ग;
    }
  }
}

raw_ostream &WithColor::error() { प्रत्यागम error(errs()); }

raw_ostream &WithColor::warning() { प्रत्यागम warning(errs()); }

raw_ostream &WithColor::note() { प्रत्यागम note(errs()); }

raw_ostream &WithColor::remark() { प्रत्यागम remark(errs()); }

raw_ostream &WithColor::error(raw_ostream &OS, StringRef Prefix,
                              तार्किक DisableColors) {
  यदि (!Prefix.empty())
    OS << Prefix << ": ";
  प्रत्यागम WithColor(OS, HighlightColor::Error,
                   DisableColors ? ColorMode::Disable : ColorMode::Auto)
             .get()
         << "error: ";
}

raw_ostream &WithColor::warning(raw_ostream &OS, StringRef Prefix,
                                तार्किक DisableColors) {
  यदि (!Prefix.empty())
    OS << Prefix << ": ";
  प्रत्यागम WithColor(OS, HighlightColor::Warning,
                   DisableColors ? ColorMode::Disable : ColorMode::Auto)
             .get()
         << "warning: ";
}

raw_ostream &WithColor::note(raw_ostream &OS, StringRef Prefix,
                             तार्किक DisableColors) {
  यदि (!Prefix.empty())
    OS << Prefix << ": ";
  प्रत्यागम WithColor(OS, HighlightColor::Note,
                   DisableColors ? ColorMode::Disable : ColorMode::Auto)
             .get()
         << "note: ";
}

raw_ostream &WithColor::remark(raw_ostream &OS, StringRef Prefix,
                               तार्किक DisableColors) {
  यदि (!Prefix.empty())
    OS << Prefix << ": ";
  प्रत्यागम WithColor(OS, HighlightColor::Remark,
                   DisableColors ? ColorMode::Disable : ColorMode::Auto)
             .get()
         << "remark: ";
}

तार्किक WithColor::colorsEnabled() {
  विकल्प (Mode) {
  पक्ष ColorMode::Enable:
    प्रत्यागम सत्य;
  पक्ष ColorMode::Disable:
    प्रत्यागम असत्य;
  पक्ष ColorMode::Auto:
    प्रत्यागम AutoDetectFunction(OS);
  }
  llvm_unreachable("All cases handled above.");
}

WithColor &WithColor::changeColor(raw_ostream::Colors Color, तार्किक Bold,
                                  तार्किक BG) {
  यदि (colorsEnabled())
    OS.changeColor(Color, Bold, BG);
  प्रत्यागम *एतत्;
}

WithColor &WithColor::resetColor() {
  यदि (colorsEnabled())
    OS.resetColor();
  प्रत्यागम *एतत्;
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
  प्रत्यागम DefaultAutoDetectFunction;
}

शून्य WithColor::setAutoDetectFunction(
    AutoDetectFunctionType NewAutoDetectFunction) {
  AutoDetectFunction = NewAutoDetectFunction;
}
