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

#അന്തര്ഭാവ "llvm/Support/VirtualOutputFile.h"
#അന്തര്ഭാവ "llvm/Support/VirtualOutputError.h"
#അന്തര്ഭാവ "llvm/Support/raw_ostream.h"
#അന്തര്ഭാവ "llvm/Support/raw_ostream_proxy.h"

using നാമാകാശ llvm;
using നാമാകാശ llvm::vfs;

അക്ഷരം OutputFileImpl::ID = 0;
അക്ഷരം NullOutputFileImpl::ID = 0;

ശൂന്യം OutputFileImpl::anchor() {}
ശൂന്യം NullOutputFileImpl::anchor() {}

വർഗ്ഗം OutputFile::TrackedProxy : പൊതു raw_pwrite_stream_proxy {
പൊതു:
  ശൂന്യം resetProxy() {
    TrackingPointer = nullptr;
    resetProxiedOS();
  }

  explicit TrackedProxy(TrackedProxy *&TrackingPointer, raw_pwrite_stream &OS)
      : raw_pwrite_stream_proxy(OS), TrackingPointer(TrackingPointer) {
    പ്രതിജ്ഞാ(!TrackingPointer && "Expected to add a proxy");
    TrackingPointer = ഇത്;
  }

  ~TrackedProxy() override { resetProxy(); }

  TrackedProxy *&TrackingPointer;
};

Expected<std::unique_ptr<raw_pwrite_stream>> OutputFile::createProxy() {
  എങ്കിൽ (OpenProxy)
    തിരികെ make_error<OutputError>(getPath(), OutputErrorCode::has_open_proxy);

  തിരികെ std::make_unique<TrackedProxy>(OpenProxy, getOS());
}

Error OutputFile::keep() {
  // Catch double-closing logic bugs.
  എങ്കിൽ (LLVM_UNLIKELY(!Impl))
    report_fatal_error(
        make_error<OutputError>(getPath(), OutputErrorCode::already_closed));

  // Report a fatal error if there's an open proxy and the file is being kept.
  // This is safer than relying on clients to remember to flush(). Also call
  // OutputFile::discard() to give the backend a chance to clean up any
  // side effects (such as temporaries).
  എങ്കിൽ (LLVM_UNLIKELY(OpenProxy))
    report_fatal_error(joinErrors(
        make_error<OutputError>(getPath(), OutputErrorCode::has_open_proxy),
        discard()));

  Error E = Impl->keep();
  Impl = nullptr;
  DiscardOnDestroyHandler = nullptr;
  തിരികെ E;
}

Error OutputFile::discard() {
  // Catch double-closing logic bugs.
  എങ്കിൽ (LLVM_UNLIKELY(!Impl))
    report_fatal_error(
        make_error<OutputError>(getPath(), OutputErrorCode::already_closed));

  // Be lenient about open proxies since client teardown paths won't
  // necessarily clean up in the right order. Reset the proxy to flush any
  // current content; if there is another write, there should be quick crash on
  // null dereference.
  എങ്കിൽ (OpenProxy)
    OpenProxy->resetProxy();

  Error E = Impl->discard();
  Impl = nullptr;
  DiscardOnDestroyHandler = nullptr;
  തിരികെ E;
}

ശൂന്യം OutputFile::destroy() {
  എങ്കിൽ (!Impl)
    തിരികെ;

  // Clean up the file. Move the discard handler into a local since discard
  // will reset it.
  auto DiscardHandler = std::move(DiscardOnDestroyHandler);
  Error E = discard();
  പ്രതിജ്ഞാ(!Impl && "Expected discard to destroy Impl");

  // If there's no handler, report a fatal error.
  എങ്കിൽ (LLVM_UNLIKELY(!DiscardHandler))
    llvm::report_fatal_error(joinErrors(
        make_error<OutputError>(getPath(), OutputErrorCode::not_closed),
        std::move(E)));
  അല്ലെങ്കിൽ എങ്കിൽ (E)
    DiscardHandler(std::move(E));
}
