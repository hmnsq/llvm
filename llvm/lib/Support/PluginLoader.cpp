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
  परत P;
}

} // anonymous namespace

शून्य PluginLoader::चालक=(स्थिरांक std::string &Filename) {
  स्वतः &P = getPlugins();
  sys::SmartScopedLock<सत्य> Lock(P.Lock);
  std::string Error;
  जर (sys::DynamicLibrary::LoadLibraryPermanently(Filename.c_str(), &Error)) {
    errs() << "Error opening '" << Filename << "': " << Error
           << "\n  -load request ignored.\n";
  } अन्यथा {
    P.List.ढकला_back(Filename);
  }
}

अचिन्हित PluginLoader::get_अंक_Plugins() {
  स्वतः &P = getPlugins();
  sys::SmartScopedLock<सत्य> Lock(P.Lock);
  परत P.List.size();
}

std::string &PluginLoader::getPlugin(अचिन्हित अंक_2) {
  स्वतः &P = getPlugins();
  sys::SmartScopedLock<सत्य> Lock(P.Lock);
  प्रतिज्ञा(अंक_2 < P.List.size() && "Asking for an out of bounds plugin");
  परत P.List[अंक_2];
}
