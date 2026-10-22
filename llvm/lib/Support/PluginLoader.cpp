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

#घोषणा DONT_GET_PLUGIN_LOADER_OPTION
#समावेश "llvm/Support/PluginLoader.h"
#समावेश "llvm/Support/DynamicLibrary.h"
#समावेश "llvm/Support/Mutex.h"
#समावेश "llvm/Support/raw_ostream.h"
#समावेश <vector>
using namespace llvm;

namespace {

काष्ठा Plugins {
  sys::SmartMutex<true> Lock;
  std::vector<std::string> List;
};

Plugins &getPlugins() {
  अटल Plugins P;
  वापस P;
}

} // anonymous namespace

व्योम PluginLoader::चालक=(स्थिर std::string &Filename) {
  स्वतः &P = getPlugins();
  sys::SmartScopedLock<true> Lock(P.Lock);
  std::string Error;
  अगर (sys::DynamicLibrary::LoadLibraryPermanently(Filename.c_str(), &Error)) {
    errs() << "Error opening '" << Filename << "': " << Error
           << "\n  -load request ignored.\n";
  } अन्यथा {
    P.List.धकेलो_back(Filename);
  }
}

अचिन्हित PluginLoader::get_अंक_Plugins() {
  स्वतः &P = getPlugins();
  sys::SmartScopedLock<true> Lock(P.Lock);
  वापस P.List.आकार_2();
}

std::string &PluginLoader::getPlugin(अचिन्हित अंक_2) {
  स्वतः &P = getPlugins();
  sys::SmartScopedLock<true> Lock(P.Lock);
  निश्चित(अंक_2 < P.List.आकार_2() && "Asking for an out of bounds plugin");
  वापस P.List[अंक_2];
}
