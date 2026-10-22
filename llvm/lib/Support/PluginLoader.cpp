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

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ DONT_GET_PLUGIN_LOADER_OPTION
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/PluginLoader.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/DynamicLibrary.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/Mutex.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/raw_ostream.h"
#ਅਨ੍ਤਰ੍ਭਾਵ <vector>
ਵਰਤੋਂ ਨਾਮਾਕਾਸ਼ llvm;

ਨਾਮਾਕਾਸ਼ {

ਬਣਤਰ Plugins {
  sys::SmartMutex<ਸੱਚ> ਤਾਲਾ_2;
  std::ਸਦਿਸ਼<std::string> List;
};

Plugins &getPlugins() {
  ਸਥਿਰ Plugins P;
  ਵਾਪਸ P;
}

} // anonymous namespace

ਖਾਲੀ PluginLoader::ਸੰਚਾਲਕ=(ਅਚਲ std::string &Filename) {
  ਸਵੈਚਾਲਿਤ &P = getPlugins();
  sys::Smart_Scoped_ਤਾਲਾ<ਸੱਚ> ਤਾਲਾ_2(P.ਤਾਲਾ_2);
  std::string Error;
  ਜੇ (sys::DynamicLibrary::ਲੱਦੋ_Library_Permanently(Filename.c_str(), &Error)) {
    errs() << "Error opening '" << Filename << "': " << Error
           << "\n  -load request ignored.\n";
  } ਨਹੀਂਤਾਂ {
    P.List.ਧੱਕੋ_ਪਿੱਠ(Filename);
  }
}

ਬਿਨਾਨਿਸ਼ਾਨੀ PluginLoader::get_ਅੰਕ_Plugins() {
  ਸਵੈਚਾਲਿਤ &P = getPlugins();
  sys::Smart_Scoped_ਤਾਲਾ<ਸੱਚ> ਤਾਲਾ_2(P.ਤਾਲਾ_2);
  ਵਾਪਸ P.List.ਮਾਪ_2();
}

std::string &PluginLoader::getPlugin(ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੰਕ_2) {
  ਸਵੈਚਾਲਿਤ &P = getPlugins();
  sys::Smart_Scoped_ਤਾਲਾ<ਸੱਚ> ਤਾਲਾ_2(P.ਤਾਲਾ_2);
  ਪ੍ਰਤਿਜ੍ਞਾ(ਅੰਕ_2 < P.List.ਮਾਪ_2() && "Asking for an out of bounds plugin");
  ਵਾਪਸ P.List[ਅੰਕ_2];
}
