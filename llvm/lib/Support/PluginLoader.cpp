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

#ವ್ಯಾಖ್ಯಾನಿಸು DONT_GET_PLUGIN_LOADER_OPTION
#ಅನ್ತರ್ಭಾವ "llvm/Support/PluginLoader.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/DynamicLibrary.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/Mutex.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/raw_ostream.h"
#ಅನ್ತರ್ಭಾವ <vector>
ಬಳಸಿ ನಾಮಾಕಾಶ llvm;

ನಾಮಾಕಾಶ {

ರಚನೆ Plugins {
  sys::SmartMutex<ನಿಜ> ಬೀಗ_3;
  std::ಸದಿಶ<std::string> List;
};

Plugins &getPlugins() {
  ಸ್ಥಿರ Plugins P;
  ಹಿಂತಿರುಗಿ P;
}

} // anonymous namespace

ಶೂನ್ಯ PluginLoader::ಕ್ರಿಯಾಕಾರಕ=(ಸ್ಥಿರಾಂಕ std::string &Filename) {
  ಸ್ವಯಂಚಾಲಿತ &P = getPlugins();
  sys::Smart_Scoped_ಬೀಗ<ನಿಜ> ಬೀಗ_3(P.ಬೀಗ_3);
  std::string Error;
  ಆದರೆ (sys::DynamicLibrary::ತುಂಬು_Library_Permanently(Filename.c_str(), &Error)) {
    errs() << "Error opening '" << Filename << "': " << Error
           << "\n  -load request ignored.\n";
  } ಇಲ್ಲದಿದ್ದರೆ {
    P.List.ತಳ್ಳು_ಹಿಂಭಾಗ(Filename);
  }
}

ಚಿಹ್ನೆರಹಿತ PluginLoader::get_ಅಂಕೆ_Plugins() {
  ಸ್ವಯಂಚಾಲಿತ &P = getPlugins();
  sys::Smart_Scoped_ಬೀಗ<ನಿಜ> ಬೀಗ_3(P.ಬೀಗ_3);
  ಹಿಂತಿರುಗಿ P.List.ಗಾತ್ರ_3();
}

std::string &PluginLoader::getPlugin(ಚಿಹ್ನೆರಹಿತ ಅಂಕೆ_2) {
  ಸ್ವಯಂಚಾಲಿತ &P = getPlugins();
  sys::Smart_Scoped_ಬೀಗ<ನಿಜ> ಬೀಗ_3(P.ಬೀಗ_3);
  ಪ್ರತಿಜ್ಞಾ(ಅಂಕೆ_2 < P.List.ಗಾತ್ರ_3() && "Asking for an out of bounds plugin");
  ಹಿಂತಿರುಗಿ P.List[ಅಂಕೆ_2];
}
