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

#تعریف_کړه DONT_GET_PLUGIN_LOADER_OPTION
#ضمن "llvm/Support/PluginLoader.h"
#ضمن "llvm/Support/DynamicLibrary.h"
#ضمن "llvm/Support/Mutex.h"
#ضمن "llvm/Support/raw_ostream.h"
#ضمن <vector>
کارول نطاق llvm;

نطاق {

بنية Plugins {
  sys::SmartMutex<ریښتیا> قلف_2;
  std::ویکټور<std::string> List;
};

Plugins &getPlugins() {
  ساكن Plugins P;
  أرجع P;
}

} // anonymous namespace

فراغ PluginLoader::چلوونکی=(ثابت std::string &Filename) {
  خپلکاری &P = getPlugins();
  sys::Smart_Scoped_قلف<ریښتیا> قلف_2(P.قلف_2);
  std::string Error;
  إذا (sys::DynamicLibrary::بارول_Library_Permanently(Filename.c_str(), &Error)) {
    errs() << "Error opening '" << Filename << "': " << Error
           << "\n  -load request ignored.\n";
  } وإلا {
    P.List.ټېل_شا(Filename);
  }
}

بینښه PluginLoader::get_عدد_Plugins() {
  خپلکاری &P = getPlugins();
  sys::Smart_Scoped_قلف<ریښتیا> قلف_2(P.قلف_2);
  أرجع P.List.اندازه_2();
}

std::string &PluginLoader::getPlugin(بینښه عدد_2) {
  خپلکاری &P = getPlugins();
  sys::Smart_Scoped_قلف<ریښتیا> قلف_2(P.قلف_2);
  أكد(عدد_2 < P.List.اندازه_2() && "Asking for an out of bounds plugin");
  أرجع P.List[عدد_2];
}
