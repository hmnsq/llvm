//===-- PluginLoader.cpp - Implement -load command line option ------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file implements the -load <plugin> command line option handler.
//
//===----------------------------------------------------------------------===//

#परिभाषय DONT_GET_PLUGIN_LOADER_OPTION
#अन्तर्भाव "llvm/Support/PluginLoader.h"
#अन्तर्भाव "llvm/Support/DynamicLibrary.h"
#अन्तर्भाव "llvm/Support/Mutex.h"
#अन्तर्भाव "llvm/Support/raw_ostream.h"
#अन्तर्भाव <vector>
उपयुज्य नामाकाश llvm;

नामाकाश {

संरचना Plugins {
  sys::SmartMutex<सत्य> तालकम्_2;
  std::सदिशम्<std::string> List;
};

Plugins &getPlugins() {
  स्थिर Plugins P;
  प्रत्यागम P;
}

} // anonymous namespace

शून्य PluginLoader::संक्रियकः=(ध्रुव std::string &Filename) {
  स्वचालित &P = getPlugins();
  sys::Smart_Scoped_तालकम्<सत्य> तालकम्_2(P.तालकम्_2);
  std::string Error;
  यदि (sys::DynamicLibrary::आरोपय_Library_Permanently(Filename.c_str(), &Error)) {
    errs() << "Error opening '" << Filename << "': " << Error
           << "\n  -load request ignored.\n";
  } अन्यथा {
    P.List.प्रक्षिप_पृष्ठभागः(Filename);
  }
}

अचिह्नित PluginLoader::get_अङ्कः_Plugins() {
  स्वचालित &P = getPlugins();
  sys::Smart_Scoped_तालकम्<सत्य> तालकम्_2(P.तालकम्_2);
  प्रत्यागम P.List.आकार_2();
}

std::string &PluginLoader::getPlugin(अचिह्नित अङ्कः_2) {
  स्वचालित &P = getPlugins();
  sys::Smart_Scoped_तालकम्<सत्य> तालकम्_2(P.तालकम्_2);
  प्रतिज्ञा(अङ्कः_2 < P.List.आकार_2() && "Asking for an out of bounds plugin");
  प्रत्यागम P.List[अङ्कः_2];
}
