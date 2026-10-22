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
  फर्काऊ ColorCategory;
}
नामाकाश {
संरचना CreateUseColor {
  स्थिर शून्य *call() {
    फर्काऊ नूतन cl::opt<cl::boolOrDefault>(
        "color", cl::cat(getColorCategory()),
        cl::desc("Use colors in output (default=autodetect)"),
        cl::init(cl::boolOrDefault::BOU_UNSET));
  }
};
} // namespace
स्थिर ManagedStatic<cl::opt<cl::boolOrDefault>, CreateUseColor> UseColor;
शून्य llvm::initWithColorOptions() { *UseColor; }

स्थिर तार्किक DefaultAutoDetectFunction(अचर raw_ostream &OS) {
  फर्काऊ *UseColor == cl::boolOrDefault::BOU_UNSET
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
      रोक;
    पक्ष HighlightColor::String:
      OS.changeColor(raw_ostream::GREEN);
      रोक;
    पक्ष HighlightColor::Tag:
      OS.changeColor(raw_ostream::BLUE);
      रोक;
    पक्ष HighlightColor::Attribute:
      OS.changeColor(raw_ostream::CYAN);
      रोक;
    पक्ष HighlightColor::Enumerator:
      OS.changeColor(raw_ostream::MAGENTA);
      रोक;
    पक्ष HighlightColor::Macro:
      OS.changeColor(raw_ostream::RED);
      रोक;
    पक्ष HighlightColor::Error:
      OS.changeColor(raw_ostream::RED, सत्य);
      रोक;
    पक्ष HighlightColor::Warning:
      OS.changeColor(raw_ostream::MAGENTA, सत्य);
      रोक;
    पक्ष HighlightColor::Note:
      OS.changeColor(raw_ostream::BLACK, सत्य);
      रोक;
    पक्ष HighlightColor::Remark:
      OS.changeColor(raw_ostream::BLUE, सत्य);
      रोक;
    }
  }
}

raw_ostream &WithColor::error() { फर्काऊ error(errs()); }

raw_ostream &WithColor::warning() { फर्काऊ warning(errs()); }

raw_ostream &WithColor::note() { फर्काऊ note(errs()); }

raw_ostream &WithColor::remark() { फर्काऊ remark(errs()); }

raw_ostream &WithColor::error(raw_ostream &OS, StringRef Prefix,
                              तार्किक DisableColors) {
  यदि (!Prefix.empty())
    OS << Prefix << ": ";
  फर्काऊ WithColor(OS, HighlightColor::Error,
                   DisableColors ? ColorMode::Disable : ColorMode::Auto)
             .get()
         << "error: ";
}

raw_ostream &WithColor::warning(raw_ostream &OS, StringRef Prefix,
                                तार्किक DisableColors) {
  यदि (!Prefix.empty())
    OS << Prefix << ": ";
  फर्काऊ WithColor(OS, HighlightColor::Warning,
                   DisableColors ? ColorMode::Disable : ColorMode::Auto)
             .get()
         << "warning: ";
}

raw_ostream &WithColor::note(raw_ostream &OS, StringRef Prefix,
                             तार्किक DisableColors) {
  यदि (!Prefix.empty())
    OS << Prefix << ": ";
  फर्काऊ WithColor(OS, HighlightColor::Note,
                   DisableColors ? ColorMode::Disable : ColorMode::Auto)
             .get()
         << "note: ";
}

raw_ostream &WithColor::remark(raw_ostream &OS, StringRef Prefix,
                               तार्किक DisableColors) {
  यदि (!Prefix.empty())
    OS << Prefix << ": ";
  फर्काऊ WithColor(OS, HighlightColor::Remark,
                   DisableColors ? ColorMode::Disable : ColorMode::Auto)
             .get()
         << "remark: ";
}

तार्किक WithColor::colorsEnabled() {
  विकल्प (Mode) {
  पक्ष ColorMode::Enable:
    फर्काऊ सत्य;
  पक्ष ColorMode::Disable:
    फर्काऊ असत्य;
  पक्ष ColorMode::Auto:
    फर्काऊ AutoDetectFunction(OS);
  }
  llvm_unreachable("All cases handled above.");
}

WithColor &WithColor::changeColor(raw_ostream::Colors Color, तार्किक Bold,
                                  तार्किक BG) {
  यदि (colorsEnabled())
    OS.changeColor(Color, Bold, BG);
  फर्काऊ *एतत्;
}

WithColor &WithColor::resetColor() {
  यदि (colorsEnabled())
    OS.resetColor();
  फर्काऊ *एतत्;
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
  फर्काऊ DefaultAutoDetectFunction;
}

शून्य WithColor::setAutoDetectFunction(
    AutoDetectFunctionType NewAutoDetectFunction) {
  AutoDetectFunction = NewAutoDetectFunction;
}
