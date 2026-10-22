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
#অন্তর্ভাৱ "llvm/Support/PluginLoader.h"
#অন্তর্ভাৱ "llvm/Support/DynamicLibrary.h"
#অন্তর্ভাৱ "llvm/Support/Mutex.h"
#অন্তর্ভাৱ "llvm/Support/raw_ostream.h"
#অন্তর্ভাৱ <vector>
using নামাকাশ llvm;

নামাকাশ {

সংরচনা Plugins {
  sys::SmartMutex<সঁচা> Lock;
  std::vector<std::string> List;
};

Plugins &getPlugins() {
  স্থির Plugins P;
  ঘূৰাই P;
}

} // anonymous namespace

শূন্য PluginLoader::operator=(ধ্রুৱ std::string &Filename) {
  auto &P = getPlugins();
  sys::SmartScopedLock<সঁচা> Lock(P.Lock);
  std::string Error;
  যদি (sys::DynamicLibrary::LoadLibraryPermanently(Filename.c_str(), &Error)) {
    errs() << "Error opening '" << Filename << "': " << Error
           << "\n  -load request ignored.\n";
  } অন্যথা {
    P.List.ঠেলো_back(Filename);
  }
}

unsigned PluginLoader::get_অঙ্ক_Plugins() {
  auto &P = getPlugins();
  sys::SmartScopedLock<সঁচা> Lock(P.Lock);
  ঘূৰাই P.List.size();
}

std::string &PluginLoader::getPlugin(unsigned অঙ্ক_2) {
  auto &P = getPlugins();
  sys::SmartScopedLock<সঁচা> Lock(P.Lock);
  প্রতিজ্ঞা(অঙ্ক_2 < P.List.size() && "Asking for an out of bounds plugin");
  ঘূৰাই P.List[অঙ্ক_2];
}
