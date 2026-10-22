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

#define DONT_GET_PLUGIN_LOADER_OPTION
#अन्तर्भाव "llvm/Support/PluginLoader.h"
#अन्तर्भाव "llvm/Support/DynamicLibrary.h"
#अन्तर्भाव "llvm/Support/Mutex.h"
#अन्तर्भाव "llvm/Support/raw_ostream.h"
#अन्तर्भाव <vector>
using नामाकाश llvm;

नामाकाश {

संरचना Plugins {
  sys::SmartMutex<सत्य> Lock;
  std::vector<std::string> List;
};

Plugins &getPlugins() {
  स्थिर Plugins P;
  प्रत्यागम P;
}

} // anonymous namespace

शून्य PluginLoader::operator=(ध्रुव std::string &Filename) {
  auto &P = getPlugins();
  sys::SmartScopedLock<सत्य> Lock(P.Lock);
  std::string Error;
  यदि (sys::DynamicLibrary::LoadLibraryPermanently(Filename.c_str(), &Error)) {
    errs() << "Error opening '" << Filename << "': " << Error
           << "\n  -load request ignored.\n";
  } अन्यथा {
    P.List.धकेलो_back(Filename);
  }
}

unsigned PluginLoader::get_अंक_Plugins() {
  auto &P = getPlugins();
  sys::SmartScopedLock<सत्य> Lock(P.Lock);
  प्रत्यागम P.List.size();
}

std::string &PluginLoader::getPlugin(unsigned अंक_2) {
  auto &P = getPlugins();
  sys::SmartScopedLock<सत्य> Lock(P.Lock);
  प्रतिज्ञा(अंक_2 < P.List.size() && "Asking for an out of bounds plugin");
  प्रत्यागम P.List[अंक_2];
}
