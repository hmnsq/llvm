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

#تعریف_کن DONT_GET_PLUGIN_LOADER_OPTION
#ضمن "llvm/Support/PluginLoader.h"
#ضمن "llvm/Support/DynamicLibrary.h"
#ضمن "llvm/Support/Mutex.h"
#ضمن "llvm/Support/raw_ostream.h"
#ضمن <vector>
استفاده نطاق llvm;

نطاق {

بنية Plugins {
  sys::SmartMutex<درست> قفل_3;
  std::وکتور<std::string> List;
};

Plugins &getPlugins() {
  ساكن Plugins P;
  أرجع P;
}

} // anonymous namespace

فراغ PluginLoader::عملگر=(ثابت std::string &Filename) {
  خودکار &P = getPlugins();
  sys::Smart_Scoped_قفل<درست> قفل_3(P.قفل_3);
  std::string Error;
  إذا (sys::DynamicLibrary::بارگیری_Library_Permanently(Filename.c_str(), &Error)) {
    errs() << "Error opening '" << Filename << "': " << Error
           << "\n  -load request ignored.\n";
  } وإلا {
    P.List.افزودن_پشت(Filename);
  }
}

بیعلامت PluginLoader::get_رقم_Plugins() {
  خودکار &P = getPlugins();
  sys::Smart_Scoped_قفل<درست> قفل_3(P.قفل_3);
  أرجع P.List.اندازه_2();
}

std::string &PluginLoader::getPlugin(بیعلامت رقم_2) {
  خودکار &P = getPlugins();
  sys::Smart_Scoped_قفل<درست> قفل_3(P.قفل_3);
  أكد(رقم_2 < P.List.اندازه_2() && "Asking for an out of bounds plugin");
  أرجع P.List[رقم_2];
}
