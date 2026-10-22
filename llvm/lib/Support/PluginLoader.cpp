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

#تعریف_کرو DONT_GET_PLUGIN_LOADER_OPTION
#ضمن "llvm/Support/PluginLoader.h"
#ضمن "llvm/Support/DynamicLibrary.h"
#ضمن "llvm/Support/Mutex.h"
#ضمن "llvm/Support/raw_ostream.h"
#ضمن <vector>
ورتوں نطاق llvm;

نطاق {

بنية Plugins {
  sys::SmartMutex<سچ> جندرا_2;
  std::سمتیہ<std::string> List;
};

Plugins &getPlugins() {
  ساكن Plugins P;
  أرجع P;
}

} // anonymous namespace

فراغ PluginLoader::عامل=(ثابت std::string &Filename) {
  خودکار &P = getPlugins();
  sys::Smart_Scoped_جندرا<سچ> جندرا_2(P.جندرا_2);
  std::string Error;
  إذا (sys::DynamicLibrary::لدو_Library_Permanently(Filename.c_str(), &Error)) {
    errs() << "Error opening '" << Filename << "': " << Error
           << "\n  -load request ignored.\n";
  } وإلا {
    P.List.ڈالو_پٹھ(Filename);
  }
}

بنانشانی PluginLoader::get_عدد_Plugins() {
  خودکار &P = getPlugins();
  sys::Smart_Scoped_جندرا<سچ> جندرا_2(P.جندرا_2);
  أرجع P.List.ناپ_2();
}

std::string &PluginLoader::getPlugin(بنانشانی عدد_2) {
  خودکار &P = getPlugins();
  sys::Smart_Scoped_جندرا<سچ> جندرا_2(P.جندرا_2);
  أكد(عدد_2 < P.List.ناپ_2() && "Asking for an out of bounds plugin");
  أرجع P.List[عدد_2];
}
