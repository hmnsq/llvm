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
#ଅନ୍ତର୍ଭାଵ "llvm/Support/PluginLoader.h"
#ଅନ୍ତର୍ଭାଵ "llvm/Support/DynamicLibrary.h"
#ଅନ୍ତର୍ଭାଵ "llvm/Support/Mutex.h"
#ଅନ୍ତର୍ଭାଵ "llvm/Support/raw_ostream.h"
#ଅନ୍ତର୍ଭାଵ <vector>
using ନାମାକାଶ llvm;

ନାମାକାଶ {

ସଂରଚନା Plugins {
  sys::SmartMutex<ସତ୍ୟ> Lock;
  std::vector<std::string> List;
};

Plugins &getPlugins() {
  ସ୍ଥିର Plugins P;
  ଫେରନ୍ତୁ P;
}

} // anonymous namespace

ଶୂନ୍ୟ PluginLoader::operator=(ଅଚଳ std::string &Filename) {
  auto &P = getPlugins();
  sys::SmartScopedLock<ସତ୍ୟ> Lock(P.Lock);
  std::string Error;
  ଯଦି (sys::DynamicLibrary::LoadLibraryPermanently(Filename.c_str(), &Error)) {
    errs() << "Error opening '" << Filename << "': " << Error
           << "\n  -load request ignored.\n";
  } ଅନ୍ୟଥା {
    P.List.ଠେଲ_back(Filename);
  }
}

unsigned PluginLoader::get_ଅଙ୍କ_Plugins() {
  auto &P = getPlugins();
  sys::SmartScopedLock<ସତ୍ୟ> Lock(P.Lock);
  ଫେରନ୍ତୁ P.List.size();
}

std::string &PluginLoader::getPlugin(unsigned ଅଙ୍କ_2) {
  auto &P = getPlugins();
  sys::SmartScopedLock<ସତ୍ୟ> Lock(P.Lock);
  ପ୍ରତିଜ୍ଞା(ଅଙ୍କ_2 < P.List.size() && "Asking for an out of bounds plugin");
  ଫେରନ୍ତୁ P.List[ଅଙ୍କ_2];
}
