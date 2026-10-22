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
#অন্তর্ভাব "llvm/Support/PluginLoader.h"
#অন্তর্ভাব "llvm/Support/DynamicLibrary.h"
#অন্তর্ভাব "llvm/Support/Mutex.h"
#অন্তর্ভাব "llvm/Support/raw_ostream.h"
#অন্তর্ভাব <vector>
using নামাকাশ llvm;

নামাকাশ {

কাঠামো Plugins {
  sys::SmartMutex<সত্য> Lock;
  std::vector<std::string> List;
};

Plugins &getPlugins() {
  স্থির Plugins P;
  ফেরত P;
}

} // anonymous namespace

শূন্য PluginLoader::operator=(ধ্রুবক std::string &Filename) {
  auto &P = getPlugins();
  sys::SmartScopedLock<সত্য> Lock(P.Lock);
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
  sys::SmartScopedLock<সত্য> Lock(P.Lock);
  ফেরত P.List.আকার_3();
}

std::string &PluginLoader::getPlugin(unsigned অঙ্ক_2) {
  auto &P = getPlugins();
  sys::SmartScopedLock<সত্য> Lock(P.Lock);
  প্রতিজ্ঞা(অঙ্ক_2 < P.List.আকার_3() && "Asking for an out of bounds plugin");
  ফেরত P.List[অঙ্ক_2];
}
