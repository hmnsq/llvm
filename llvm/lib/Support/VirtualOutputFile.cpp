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

#समावेश "llvm/Support/VirtualOutputFile.h"
#समावेश "llvm/Support/VirtualOutputError.h"
#समावेश "llvm/Support/raw_ostream.h"
#समावेश "llvm/Support/raw_ostream_proxy.h"

using namespace llvm;
using namespace llvm::vfs;

अक्षर OutputFileImpl::ID = 0;
अक्षर NullOutputFileImpl::ID = 0;

व्योम OutputFileImpl::anchor() {}
व्योम NullOutputFileImpl::anchor() {}

श्रेणी OutputFile::TrackedProxy : खुला raw_pwrite_stream_proxy {
खुला:
  व्योम resetProxy() {
    TrackingPointer = nullptr;
    resetProxiedOS();
  }

  explicit TrackedProxy(TrackedProxy *&TrackingPointer, raw_pwrite_stream &OS)
      : raw_pwrite_stream_proxy(OS), TrackingPointer(TrackingPointer) {
    निश्चित(!TrackingPointer && "Expected to add a proxy");
    TrackingPointer = यह;
  }

  ~TrackedProxy() override { resetProxy(); }

  TrackedProxy *&TrackingPointer;
};

Expected<std::unique_ptr<raw_pwrite_stream>> OutputFile::createProxy() {
  अगर (OpenProxy)
    वापस make_error<OutputError>(getPath(), OutputErrorCode::has_open_proxy);

  वापस std::make_unique<TrackedProxy>(OpenProxy, getOS());
}

Error OutputFile::keep() {
  // Catch double-closing logic bugs.
  अगर (LLVM_UNLIKELY(!Impl))
    report_fatal_error(
        make_error<OutputError>(getPath(), OutputErrorCode::already_closed));

  // Report a fatal error if there's an open proxy and the file is being kept.
  // This is safer than relying on clients to remember to flush(). Also call
  // OutputFile::discard() to give the backend a chance to clean up any
  // side effects (such as temporaries).
  अगर (LLVM_UNLIKELY(OpenProxy))
    report_fatal_error(joinErrors(
        make_error<OutputError>(getPath(), OutputErrorCode::has_open_proxy),
        discard()));

  Error E = Impl->keep();
  Impl = nullptr;
  DiscardOnDestroyHandler = nullptr;
  वापस E;
}

Error OutputFile::discard() {
  // Catch double-closing logic bugs.
  अगर (LLVM_UNLIKELY(!Impl))
    report_fatal_error(
        make_error<OutputError>(getPath(), OutputErrorCode::already_closed));

  // Be lenient about open proxies since client teardown paths won't
  // necessarily clean up in the right order. Reset the proxy to flush any
  // current content; if there is another write, there should be quick crash on
  // null dereference.
  अगर (OpenProxy)
    OpenProxy->resetProxy();

  Error E = Impl->discard();
  Impl = nullptr;
  DiscardOnDestroyHandler = nullptr;
  वापस E;
}

व्योम OutputFile::destroy() {
  अगर (!Impl)
    वापस;

  // Clean up the file. Move the discard handler into a local since discard
  // will reset it.
  स्वतः DiscardHandler = std::move(DiscardOnDestroyHandler);
  Error E = discard();
  निश्चित(!Impl && "Expected discard to destroy Impl");

  // If there's no handler, report a fatal error.
  अगर (LLVM_UNLIKELY(!DiscardHandler))
    llvm::report_fatal_error(joinErrors(
        make_error<OutputError>(getPath(), OutputErrorCode::not_closed),
        std::move(E)));
  अन्यथा अगर (E)
    DiscardHandler(std::move(E));
}
