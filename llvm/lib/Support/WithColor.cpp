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

उपयुज्य नामाकाश llvm;

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

WithColor::WithColor(raw_ostream &OS, HighlightColor Color, Color_बहुलकम् बहुलकम्)
    : OS(OS), बहुलकम्(बहुलकम्) {
  // Detect color from terminal type unless the user passed the --color option.
  यदि (colorsEnabled()) {
    विकल्प (Color) {
    पक्ष HighlightColor::स्थानाङ्कः:
      OS.changeColor(raw_ostream::YELLOW);
      भङ्ग;
    पक्ष HighlightColor::String:
      OS.changeColor(raw_ostream::GREEN);
      भङ्ग;
    पक्ष HighlightColor::चिह्नकम्:
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

raw_ostream &WithColor::error(raw_ostream &OS, StringRef उपसर्गः,
                              तार्किक DisableColors) {
  यदि (!उपसर्गः.रिक्तानि())
    OS << उपसर्गः << ": ";
  प्रत्यागम WithColor(OS, HighlightColor::Error,
                   DisableColors ? Color_बहुलकम्::Disable : Color_बहुलकम्::Auto)
             .get()
         << "error: ";
}

raw_ostream &WithColor::warning(raw_ostream &OS, StringRef उपसर्गः,
                                तार्किक DisableColors) {
  यदि (!उपसर्गः.रिक्तानि())
    OS << उपसर्गः << ": ";
  प्रत्यागम WithColor(OS, HighlightColor::Warning,
                   DisableColors ? Color_बहुलकम्::Disable : Color_बहुलकम्::Auto)
             .get()
         << "warning: ";
}

raw_ostream &WithColor::note(raw_ostream &OS, StringRef उपसर्गः,
                             तार्किक DisableColors) {
  यदि (!उपसर्गः.रिक्तानि())
    OS << उपसर्गः << ": ";
  प्रत्यागम WithColor(OS, HighlightColor::Note,
                   DisableColors ? Color_बहुलकम्::Disable : Color_बहुलकम्::Auto)
             .get()
         << "note: ";
}

raw_ostream &WithColor::remark(raw_ostream &OS, StringRef उपसर्गः,
                               तार्किक DisableColors) {
  यदि (!उपसर्गः.रिक्तानि())
    OS << उपसर्गः << ": ";
  प्रत्यागम WithColor(OS, HighlightColor::Remark,
                   DisableColors ? Color_बहुलकम्::Disable : Color_बहुलकम्::Auto)
             .get()
         << "remark: ";
}

तार्किक WithColor::colorsEnabled() {
  विकल्प (बहुलकम्) {
  पक्ष Color_बहुलकम्::Enable:
    प्रत्यागम सत्य;
  पक्ष Color_बहुलकम्::Disable:
    प्रत्यागम असत्य;
  पक्ष Color_बहुलकम्::Auto:
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
  handleAllErrors(std::move(Err), [](Error_Info_आधारः &Info) {
    WithColor::error() << Info.सन्देशः_2() << '\n';
  });
}

शून्य WithColor::defaultWarningHandler(Error Warning) {
  handleAllErrors(std::move(Warning), [](Error_Info_आधारः &Info) {
    WithColor::warning() << Info.सन्देशः_2() << '\n';
  });
}

WithColor::AutoDetectFunctionType WithColor::defaultAutoDetectFunction() {
  प्रत्यागम DefaultAutoDetectFunction;
}

शून्य WithColor::समुच्चयः_Auto_Detect_Function(
    AutoDetectFunctionType NewAutoDetectFunction) {
  AutoDetectFunction = NewAutoDetectFunction;
}
