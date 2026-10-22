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

#அந்தர்பாவ "llvm/Support/VirtualOutputFile.h"
#அந்தர்பாவ "llvm/Support/VirtualOutputError.h"
#அந்தர்பாவ "llvm/Support/raw_ostream.h"
#அந்தர்பாவ "llvm/Support/raw_ostream_proxy.h"

using நாமாகாஶ llvm;
using நாமாகாஶ llvm::vfs;

எழுத்து OutputFileImpl::ID = 0;
எழுத்து NullOutputFileImpl::ID = 0;

வெறுமை OutputFileImpl::anchor() {}
வெறுமை NullOutputFileImpl::anchor() {}

வகுப்பு OutputFile::TrackedProxy : பொது raw_pwrite_stream_proxy {
பொது:
  வெறுமை resetProxy() {
    TrackingPointer = nullptr;
    resetProxiedOS();
  }

  explicit TrackedProxy(TrackedProxy *&TrackingPointer, raw_pwrite_stream &OS)
      : raw_pwrite_stream_proxy(OS), TrackingPointer(TrackingPointer) {
    ப்ரதிஜ்ஞா(!TrackingPointer && "Expected to add a proxy");
    TrackingPointer = இது;
  }

  ~TrackedProxy() override { resetProxy(); }

  TrackedProxy *&TrackingPointer;
};

Expected<std::unique_ptr<raw_pwrite_stream>> OutputFile::createProxy() {
  எனில் (OpenProxy)
    திரும்பு make_error<OutputError>(getPath(), OutputErrorCode::has_open_proxy);

  திரும்பு std::make_unique<TrackedProxy>(OpenProxy, getOS());
}

Error OutputFile::keep() {
  // Catch double-closing logic bugs.
  எனில் (LLVM_UNLIKELY(!Impl))
    report_fatal_error(
        make_error<OutputError>(getPath(), OutputErrorCode::already_closed));

  // Report a fatal error if there's an open proxy and the file is being kept.
  // This is safer than relying on clients to remember to flush(). Also call
  // OutputFile::discard() to give the backend a chance to clean up any
  // side effects (such as temporaries).
  எனில் (LLVM_UNLIKELY(OpenProxy))
    report_fatal_error(joinErrors(
        make_error<OutputError>(getPath(), OutputErrorCode::has_open_proxy),
        discard()));

  Error E = Impl->keep();
  Impl = nullptr;
  DiscardOnDestroyHandler = nullptr;
  திரும்பு E;
}

Error OutputFile::discard() {
  // Catch double-closing logic bugs.
  எனில் (LLVM_UNLIKELY(!Impl))
    report_fatal_error(
        make_error<OutputError>(getPath(), OutputErrorCode::already_closed));

  // Be lenient about open proxies since client teardown paths won't
  // necessarily clean up in the right order. Reset the proxy to flush any
  // current content; if there is another write, there should be quick crash on
  // null dereference.
  எனில் (OpenProxy)
    OpenProxy->resetProxy();

  Error E = Impl->discard();
  Impl = nullptr;
  DiscardOnDestroyHandler = nullptr;
  திரும்பு E;
}

வெறுமை OutputFile::destroy() {
  எனில் (!Impl)
    திரும்பு;

  // Clean up the file. Move the discard handler into a local since discard
  // will reset it.
  auto DiscardHandler = std::move(DiscardOnDestroyHandler);
  Error E = discard();
  ப்ரதிஜ்ஞா(!Impl && "Expected discard to destroy Impl");

  // If there's no handler, report a fatal error.
  எனில் (LLVM_UNLIKELY(!DiscardHandler))
    llvm::report_fatal_error(joinErrors(
        make_error<OutputError>(getPath(), OutputErrorCode::not_closed),
        std::move(E)));
  இல்லையெனில் எனில் (E)
    DiscardHandler(std::move(E));
}
