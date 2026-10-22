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
#અન્તર્ભાવ "llvm/Support/PluginLoader.h"
#અન્તર્ભાવ "llvm/Support/DynamicLibrary.h"
#અન્તર્ભાવ "llvm/Support/Mutex.h"
#અન્તર્ભાવ "llvm/Support/raw_ostream.h"
#અન્તર્ભાવ <vector>
using નામાકાશ llvm;

નામાકાશ {

સંરચના Plugins {
  sys::SmartMutex<સાચું> Lock;
  std::vector<std::string> List;
};

Plugins &getPlugins() {
  સ્થિર Plugins P;
  પાછા P;
}

} // anonymous namespace

શૂન્ય PluginLoader::operator=(અચલ std::string &Filename) {
  auto &P = getPlugins();
  sys::SmartScopedLock<સાચું> Lock(P.Lock);
  std::string Error;
  જો (sys::DynamicLibrary::LoadLibraryPermanently(Filename.c_str(), &Error)) {
    errs() << "Error opening '" << Filename << "': " << Error
           << "\n  -load request ignored.\n";
  } નહીંતર {
    P.List.ધકેલો_back(Filename);
  }
}

unsigned PluginLoader::get_અંક_Plugins() {
  auto &P = getPlugins();
  sys::SmartScopedLock<સાચું> Lock(P.Lock);
  પાછા P.List.size();
}

std::string &PluginLoader::getPlugin(unsigned અંક_2) {
  auto &P = getPlugins();
  sys::SmartScopedLock<સાચું> Lock(P.Lock);
  પ્રતિજ્ઞા(અંક_2 < P.List.size() && "Asking for an out of bounds plugin");
  પાછા P.List[અંક_2];
}
