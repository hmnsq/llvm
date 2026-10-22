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

#وصف_ڪريو DONT_GET_PLUGIN_LOADER_OPTION
#ضمن "llvm/Support/PluginLoader.h"
#ضمن "llvm/Support/DynamicLibrary.h"
#ضمن "llvm/Support/Mutex.h"
#ضمن "llvm/Support/raw_ostream.h"
#ضمن <vector>
استعمال نطاق llvm;

نطاق {

بنية Plugins {
  sys::SmartMutex<سچ> تالو_2;
  std::ويڪٽر<std::string> List;
};

Plugins &getPlugins() {
  ساكن Plugins P;
  أرجع P;
}

} // anonymous namespace

فراغ PluginLoader::عامل=(ثابت std::string &Filename) {
  خودڪار &P = getPlugins();
  sys::Smart_Scoped_تالو<سچ> تالو_2(P.تالو_2);
  std::string Error;
  إذا (sys::DynamicLibrary::ڀريو_Library_Permanently(Filename.c_str(), &Error)) {
    errs() << "Error opening '" << Filename << "': " << Error
           << "\n  -load request ignored.\n";
  } وإلا {
    P.List.ڌڪو_پٺ(Filename);
  }
}

بنانشان PluginLoader::get_انگ_Plugins() {
  خودڪار &P = getPlugins();
  sys::Smart_Scoped_تالو<سچ> تالو_2(P.تالو_2);
  أرجع P.List.سائيز_2();
}

std::string &PluginLoader::getPlugin(بنانشان انگ_2) {
  خودڪار &P = getPlugins();
  sys::Smart_Scoped_تالو<سچ> تالو_2(P.تالو_2);
  أكد(انگ_2 < P.List.سائيز_2() && "Asking for an out of bounds plugin");
  أرجع P.List[انگ_2];
}
