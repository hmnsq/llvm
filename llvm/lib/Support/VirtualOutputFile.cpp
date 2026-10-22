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

#ଅନ୍ତର୍ଭାଵ "llvm/Support/VirtualOutputFile.h"
#ଅନ୍ତର୍ଭାଵ "llvm/Support/VirtualOutputError.h"
#ଅନ୍ତର୍ଭାଵ "llvm/Support/raw_ostream.h"
#ଅନ୍ତର୍ଭାଵ "llvm/Support/raw_ostream_proxy.h"

using ନାମାକାଶ llvm;
using ନାମାକାଶ llvm::vfs;

ଅକ୍ଷର OutputFileImpl::ID = 0;
ଅକ୍ଷର NullOutputFileImpl::ID = 0;

ଶୂନ୍ୟ OutputFileImpl::anchor() {}
ଶୂନ୍ୟ NullOutputFileImpl::anchor() {}

ଶ୍ରେଣୀ OutputFile::TrackedProxy : ସର୍ବସାଧାରଣ raw_pwrite_stream_proxy {
ସର୍ବସାଧାରଣ:
  ଶୂନ୍ୟ resetProxy() {
    TrackingPointer = nullptr;
    resetProxiedOS();
  }

  explicit TrackedProxy(TrackedProxy *&TrackingPointer, raw_pwrite_stream &OS)
      : raw_pwrite_stream_proxy(OS), TrackingPointer(TrackingPointer) {
    ପ୍ରତିଜ୍ଞା(!TrackingPointer && "Expected to add a proxy");
    TrackingPointer = ଏହି;
  }

  ~TrackedProxy() override { resetProxy(); }

  TrackedProxy *&TrackingPointer;
};

Expected<std::unique_ptr<raw_pwrite_stream>> OutputFile::createProxy() {
  ଯଦି (OpenProxy)
    ଫେରନ୍ତୁ make_error<OutputError>(getPath(), OutputErrorCode::has_open_proxy);

  ଫେରନ୍ତୁ std::make_unique<TrackedProxy>(OpenProxy, getOS());
}

Error OutputFile::keep() {
  // Catch double-closing logic bugs.
  ଯଦି (LLVM_UNLIKELY(!Impl))
    report_fatal_error(
        make_error<OutputError>(getPath(), OutputErrorCode::already_closed));

  // Report a fatal error if there's an open proxy and the file is being kept.
  // This is safer than relying on clients to remember to flush(). Also call
  // OutputFile::discard() to give the backend a chance to clean up any
  // side effects (such as temporaries).
  ଯଦି (LLVM_UNLIKELY(OpenProxy))
    report_fatal_error(joinErrors(
        make_error<OutputError>(getPath(), OutputErrorCode::has_open_proxy),
        discard()));

  Error E = Impl->keep();
  Impl = nullptr;
  DiscardOnDestroyHandler = nullptr;
  ଫେରନ୍ତୁ E;
}

Error OutputFile::discard() {
  // Catch double-closing logic bugs.
  ଯଦି (LLVM_UNLIKELY(!Impl))
    report_fatal_error(
        make_error<OutputError>(getPath(), OutputErrorCode::already_closed));

  // Be lenient about open proxies since client teardown paths won't
  // necessarily clean up in the right order. Reset the proxy to flush any
  // current content; if there is another write, there should be quick crash on
  // null dereference.
  ଯଦି (OpenProxy)
    OpenProxy->resetProxy();

  Error E = Impl->discard();
  Impl = nullptr;
  DiscardOnDestroyHandler = nullptr;
  ଫେରନ୍ତୁ E;
}

ଶୂନ୍ୟ OutputFile::destroy() {
  ଯଦି (!Impl)
    ଫେରନ୍ତୁ;

  // Clean up the file. Move the discard handler into a local since discard
  // will reset it.
  auto DiscardHandler = std::move(DiscardOnDestroyHandler);
  Error E = discard();
  ପ୍ରତିଜ୍ଞା(!Impl && "Expected discard to destroy Impl");

  // If there's no handler, report a fatal error.
  ଯଦି (LLVM_UNLIKELY(!DiscardHandler))
    llvm::report_fatal_error(joinErrors(
        make_error<OutputError>(getPath(), OutputErrorCode::not_closed),
        std::move(E)));
  ଅନ୍ୟଥା ଯଦି (E)
    DiscardHandler(std::move(E));
}
