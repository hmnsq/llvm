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
#അന്തര്ഭാവ "llvm/Support/PluginLoader.h"
#അന്തര്ഭാവ "llvm/Support/DynamicLibrary.h"
#അന്തര്ഭാവ "llvm/Support/Mutex.h"
#അന്തര്ഭാവ "llvm/Support/raw_ostream.h"
#അന്തര്ഭാവ <vector>
using നാമാകാശ llvm;

നാമാകാശ {

ഘടന Plugins {
  sys::SmartMutex<സത്യം> Lock;
  std::vector<std::string> List;
};

Plugins &getPlugins() {
  സ്ഥിതം Plugins P;
  തിരികെ P;
}

} // anonymous namespace

ശൂന്യം PluginLoader::operator=(സ്ഥിരം std::string &Filename) {
  auto &P = getPlugins();
  sys::SmartScopedLock<സത്യം> Lock(P.Lock);
  std::string Error;
  എങ്കിൽ (sys::DynamicLibrary::LoadLibraryPermanently(Filename.c_str(), &Error)) {
    errs() << "Error opening '" << Filename << "': " << Error
           << "\n  -load request ignored.\n";
  } അല്ലെങ്കിൽ {
    P.List.തള്ളുക_back(Filename);
  }
}

unsigned PluginLoader::get_അക്കം_Plugins() {
  auto &P = getPlugins();
  sys::SmartScopedLock<സത്യം> Lock(P.Lock);
  തിരികെ P.List.size();
}

std::string &PluginLoader::getPlugin(unsigned അക്കം_2) {
  auto &P = getPlugins();
  sys::SmartScopedLock<സത്യം> Lock(P.Lock);
  പ്രതിജ്ഞാ(അക്കം_2 < P.List.size() && "Asking for an out of bounds plugin");
  തിരികെ P.List[അക്കം_2];
}
