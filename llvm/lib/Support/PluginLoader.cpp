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
#அந்தர்பாவ "llvm/Support/PluginLoader.h"
#அந்தர்பாவ "llvm/Support/DynamicLibrary.h"
#அந்தர்பாவ "llvm/Support/Mutex.h"
#அந்தர்பாவ "llvm/Support/raw_ostream.h"
#அந்தர்பாவ <vector>
using நாமாகாஶ llvm;

நாமாகாஶ {

கட்டமைப்பு Plugins {
  sys::SmartMutex<உண்மை> Lock;
  std::vector<std::string> List;
};

Plugins &getPlugins() {
  நிலையான Plugins P;
  திரும்பு P;
}

} // anonymous namespace

வெறுமை PluginLoader::operator=(மாறிலி std::string &Filename) {
  auto &P = getPlugins();
  sys::SmartScopedLock<உண்மை> Lock(P.Lock);
  std::string Error;
  எனில் (sys::DynamicLibrary::LoadLibraryPermanently(Filename.c_str(), &Error)) {
    errs() << "Error opening '" << Filename << "': " << Error
           << "\n  -load request ignored.\n";
  } இல்லையெனில் {
    P.List.தள்ளு_back(Filename);
  }
}

unsigned PluginLoader::get_இலக்கம்_Plugins() {
  auto &P = getPlugins();
  sys::SmartScopedLock<உண்மை> Lock(P.Lock);
  திரும்பு P.List.size();
}

std::string &PluginLoader::getPlugin(unsigned இலக்கம்_2) {
  auto &P = getPlugins();
  sys::SmartScopedLock<உண்மை> Lock(P.Lock);
  ப்ரதிஜ்ஞா(இலக்கம்_2 < P.List.size() && "Asking for an out of bounds plugin");
  திரும்பு P.List[இலக்கம்_2];
}
