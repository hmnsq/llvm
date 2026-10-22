//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
///
/// \file
/// This file implements \c vfs::OutputBackend class methods.
///
//===----------------------------------------------------------------------===//

#అన్తర్భావ "llvm/Support/VirtualOutputBackend.h"
#అన్తర్భావ "llvm/ADT/SmallString.h"
#అన్తర్భావ "llvm/Support/VirtualOutputError.h"

ఉపయోగించి నామాకాశ llvm;
ఉపయోగించి నామాకాశ llvm::vfs;

శూన్య OutputBackend::anchor() {}

Expected<OutputFile>
OutputBackend::createFile(స్థిరరాశి Twine &మార్గం,
                          std::optional<OutputConfig> Config) {
  SmallString<128> మార్గం_Storage;
  మార్గం.గమ్యం_సదిశ(మార్గం_Storage);

  ఒకవేళ (Config) {
    // Check for invalid configs.
    ఒకవేళ (!Config->get_పాఠ్యం() && Config->getCRLF())
      తిరిగి make_error<OutputConfigError>(*Config, మార్గం_Storage);
  }

  std::unique_ptr<OutputFileImpl> Impl;
  ఒకవేళ (Error E = createFileImpl(మార్గం_Storage, Config).moveInto(Impl))
    తిరిగి std::move(E);
  ప్రతిజ్ఞా(Impl && "Expected valid Impl or Error");
  తిరిగి OutputFile(మార్గం_Storage, std::move(Impl));
}
