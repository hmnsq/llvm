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
#شامل "llvm/Support/PluginLoader.h"
#شامل "llvm/Support/DynamicLibrary.h"
#شامل "llvm/Support/Mutex.h"
#شامل "llvm/Support/raw_ostream.h"
#شامل <vector>
استعمال نامخلا llvm;

نامخلا {

ساخت Plugins {
  sys::SmartMutex<سچ> تالا_3;
  std::سمتیہ<std::string> List;
};

Plugins &getPlugins() {
  ساکن Plugins P;
  واپس P;
}

} // anonymous namespace

خالی PluginLoader::عامل=(ثابت std::string &Filename) {
  خودکار &P = getPlugins();
  sys::Smart_Scoped_تالا<سچ> تالا_3(P.تالا_3);
  std::string Error;
  اگر (sys::DynamicLibrary::بھرو_Library_Permanently(Filename.c_str(), &Error)) {
    errs() << "Error opening '" << Filename << "': " << Error
           << "\n  -load request ignored.\n";
  } ورنہ {
    P.List.ڈالو_عقب(Filename);
  }
}

غیرعلامتی PluginLoader::get_عدد_Plugins() {
  خودکار &P = getPlugins();
  sys::Smart_Scoped_تالا<سچ> تالا_3(P.تالا_3);
  واپس P.List.سائز_2();
}

std::string &PluginLoader::getPlugin(غیرعلامتی عدد_2) {
  خودکار &P = getPlugins();
  sys::Smart_Scoped_تالا<سچ> تالا_3(P.تالا_3);
  أكد(عدد_2 < P.List.سائز_2() && "Asking for an out of bounds plugin");
  واپس P.List[عدد_2];
}
