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
#ضمن "llvm/Support/PluginLoader.h"
#ضمن "llvm/Support/DynamicLibrary.h"
#ضمن "llvm/Support/Mutex.h"
#ضمن "llvm/Support/raw_ostream.h"
#ضمن <vector>
using نطاق llvm;

نطاق {

بنية Plugins {
  sys::SmartMutex<true> Lock;
  std::vector<std::string> List;
};

Plugins &getPlugins() {
  ساكن Plugins P;
  أرجع P;
}

} // anonymous namespace

فراغ PluginLoader::operator=(ثابت std::string &Filename) {
  auto &P = getPlugins();
  sys::SmartScopedLock<true> Lock(P.Lock);
  std::string Error;
  إذا (sys::DynamicLibrary::LoadLibraryPermanently(Filename.c_str(), &Error)) {
    errs() << "Error opening '" << Filename << "': " << Error
           << "\n  -load request ignored.\n";
  } وإلا {
    P.List.ادفع_back(Filename);
  }
}

unsigned PluginLoader::get_رقم_Plugins() {
  auto &P = getPlugins();
  sys::SmartScopedLock<true> Lock(P.Lock);
  أرجع P.List.size();
}

std::string &PluginLoader::getPlugin(unsigned رقم_3) {
  auto &P = getPlugins();
  sys::SmartScopedLock<true> Lock(P.Lock);
  أكد(رقم_3 < P.List.size() && "Asking for an out of bounds plugin");
  أرجع P.List[رقم_3];
}
