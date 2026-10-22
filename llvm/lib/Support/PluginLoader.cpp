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

#నిర్వచించు DONT_GET_PLUGIN_LOADER_OPTION
#అన్తర్భావ "llvm/Support/PluginLoader.h"
#అన్తర్భావ "llvm/Support/DynamicLibrary.h"
#అన్తర్భావ "llvm/Support/Mutex.h"
#అన్తర్భావ "llvm/Support/raw_ostream.h"
#అన్తర్భావ <vector>
ఉపయోగించి నామాకాశ llvm;

నామాకాశ {

నిర్మాణం Plugins {
  sys::SmartMutex<నిజం> తాళం_3;
  std::సదిశ<std::string> List;
};

Plugins &getPlugins() {
  స్థిర Plugins P;
  తిరిగి P;
}

} // anonymous namespace

శూన్య PluginLoader::క్రియాకారకం=(స్థిరరాశి std::string &Filename) {
  స్వయంచాలక &P = getPlugins();
  sys::Smart_Scoped_తాళం<నిజం> తాళం_3(P.తాళం_3);
  std::string Error;
  ఒకవేళ (sys::DynamicLibrary::ఎక్కించు_Library_Permanently(Filename.c_str(), &Error)) {
    errs() << "Error opening '" << Filename << "': " << Error
           << "\n  -load request ignored.\n";
  } లేకపోతే {
    P.List.నెట్టు_వెనుక(Filename);
  }
}

గుర్తులేని PluginLoader::get_అంకె_Plugins() {
  స్వయంచాలక &P = getPlugins();
  sys::Smart_Scoped_తాళం<నిజం> తాళం_3(P.తాళం_3);
  తిరిగి P.List.కొలత_2();
}

std::string &PluginLoader::getPlugin(గుర్తులేని అంకె_2) {
  స్వయంచాలక &P = getPlugins();
  sys::Smart_Scoped_తాళం<నిజం> తాళం_3(P.తాళం_3);
  ప్రతిజ్ఞా(అంకె_2 < P.List.కొలత_2() && "Asking for an out of bounds plugin");
  తిరిగి P.List[అంకె_2];
}
