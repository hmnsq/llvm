//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
///
/// \file
/// This file implements \c OutputFile class methods.
///
//===----------------------------------------------------------------------===//

#అన్తర్భావ "llvm/Support/VirtualOutputFile.h"
#అన్తర్భావ "llvm/Support/VirtualOutputError.h"
#అన్తర్భావ "llvm/Support/raw_ostream.h"
#అన్తర్భావ "llvm/Support/raw_ostream_proxy.h"

ఉపయోగించి నామాకాశ llvm;
ఉపయోగించి నామాకాశ llvm::vfs;

అక్షరం OutputFileImpl::ID = 0;
అక్షరం NullOutputFileImpl::ID = 0;

శూన్య OutputFileImpl::anchor() {}
శూన్య NullOutputFileImpl::anchor() {}

తరగతి OutputFile::TrackedProxy : బహిరంగ raw_pwrite_ప్రవాహం_proxy {
బహిరంగ:
  శూన్య resetProxy() {
    Tracking_సూచిక = శూన్యసూచిక;
    resetProxiedOS();
  }

  స్పష్టమైన TrackedProxy(TrackedProxy *&Tracking_సూచిక, raw_pwrite_ప్రవాహం &OS)
      : raw_pwrite_ప్రవాహం_proxy(OS), Tracking_సూచిక(Tracking_సూచిక) {
    ప్రతిజ్ఞా(!Tracking_సూచిక && "Expected to add a proxy");
    Tracking_సూచిక = ఇది;
  }

  ~TrackedProxy() override { resetProxy(); }

  TrackedProxy *&Tracking_సూచిక;
};

Expected<std::unique_ptr<raw_pwrite_ప్రవాహం>> OutputFile::createProxy() {
  ఒకవేళ (OpenProxy)
    తిరిగి make_error<OutputError>(get_మార్గం(), Output_Error_సంకేతం::has_open_proxy);

  తిరిగి std::make_unique<TrackedProxy>(OpenProxy, getOS());
}

Error OutputFile::keep() {
  // Catch double-closing logic bugs.
  ఒకవేళ (LLVM_UNLIKELY(!Impl))
    report_fatal_error(
        make_error<OutputError>(get_మార్గం(), Output_Error_సంకేతం::already_closed));

  // Report a fatal error if there's an open proxy and the file is being kept.
  // This is safer than relying on clients to remember to flush(). Also call
  // OutputFile::discard() to give the backend a chance to clean up any
  // side effects (such as temporaries).
  ఒకవేళ (LLVM_UNLIKELY(OpenProxy))
    report_fatal_error(joinErrors(
        make_error<OutputError>(get_మార్గం(), Output_Error_సంకేతం::has_open_proxy),
        discard()));

  Error E = Impl->keep();
  Impl = శూన్యసూచిక;
  DiscardOnDestroyHandler = శూన్యసూచిక;
  తిరిగి E;
}

Error OutputFile::discard() {
  // Catch double-closing logic bugs.
  ఒకవేళ (LLVM_UNLIKELY(!Impl))
    report_fatal_error(
        make_error<OutputError>(get_మార్గం(), Output_Error_సంకేతం::already_closed));

  // Be lenient about open proxies since client teardown paths won't
  // necessarily clean up in the right order. Reset the proxy to flush any
  // current content; if there is another write, there should be quick crash on
  // null dereference.
  ఒకవేళ (OpenProxy)
    OpenProxy->resetProxy();

  Error E = Impl->discard();
  Impl = శూన్యసూచిక;
  DiscardOnDestroyHandler = శూన్యసూచిక;
  తిరిగి E;
}

శూన్య OutputFile::destroy() {
  ఒకవేళ (!Impl)
    తిరిగి;

  // Clean up the file. Move the discard handler into a local since discard
  // will reset it.
  స్వయంచాలక DiscardHandler = std::move(DiscardOnDestroyHandler);
  Error E = discard();
  ప్రతిజ్ఞా(!Impl && "Expected discard to destroy Impl");

  // If there's no handler, report a fatal error.
  ఒకవేళ (LLVM_UNLIKELY(!DiscardHandler))
    llvm::report_fatal_error(joinErrors(
        make_error<OutputError>(get_మార్గం(), Output_Error_సంకేతం::not_closed),
        std::move(E)));
  లేకపోతే ఒకవేళ (E)
    DiscardHandler(std::move(E));
}
