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
#شامل "llvm/Support/PluginLoader.h"
#شامل "llvm/Support/DynamicLibrary.h"
#شامل "llvm/Support/Mutex.h"
#شامل "llvm/Support/raw_ostream.h"
#شامل <vector>
using فضاینام llvm;

فضاینام {

ساختار Plugins {
  sys::SmartMutex<درست> Lock;
  std::vector<std::string> List;
};

Plugins &getPlugins() {
  ایستا Plugins P;
  بازگشت P;
}

} // anonymous namespace

تهی PluginLoader::operator=(ثابت std::string &Filename) {
  auto &P = getPlugins();
  sys::SmartScopedLock<درست> Lock(P.Lock);
  std::string Error;
  اگر (sys::DynamicLibrary::LoadLibraryPermanently(Filename.c_str(), &Error)) {
    errs() << "Error opening '" << Filename << "': " << Error
           << "\n  -load request ignored.\n";
  } وگرنه {
    P.List.افزودن_back(Filename);
  }
}

unsigned PluginLoader::get_رقم_Plugins() {
  auto &P = getPlugins();
  sys::SmartScopedLock<درست> Lock(P.Lock);
  بازگشت P.List.size();
}

std::string &PluginLoader::getPlugin(unsigned رقم_2) {
  auto &P = getPlugins();
  sys::SmartScopedLock<درست> Lock(P.Lock);
  أكد(رقم_2 < P.List.size() && "Asking for an out of bounds plugin");
  بازگشت P.List[رقم_2];
}
