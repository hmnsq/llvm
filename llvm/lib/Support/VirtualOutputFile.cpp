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

#અન્તર્ભાવ "llvm/Support/VirtualOutputFile.h"
#અન્તર્ભાવ "llvm/Support/VirtualOutputError.h"
#અન્તર્ભાવ "llvm/Support/raw_ostream.h"
#અન્તર્ભાવ "llvm/Support/raw_ostream_proxy.h"

using નામાકાશ llvm;
using નામાકાશ llvm::vfs;

અક્ષર OutputFileImpl::ID = 0;
અક્ષર NullOutputFileImpl::ID = 0;

શૂન્ય OutputFileImpl::anchor() {}
શૂન્ય NullOutputFileImpl::anchor() {}

વર્ગ OutputFile::TrackedProxy : જાહેર raw_pwrite_stream_proxy {
જાહેર:
  શૂન્ય resetProxy() {
    TrackingPointer = nullptr;
    resetProxiedOS();
  }

  explicit TrackedProxy(TrackedProxy *&TrackingPointer, raw_pwrite_stream &OS)
      : raw_pwrite_stream_proxy(OS), TrackingPointer(TrackingPointer) {
    પ્રતિજ્ઞા(!TrackingPointer && "Expected to add a proxy");
    TrackingPointer = આ;
  }

  ~TrackedProxy() override { resetProxy(); }

  TrackedProxy *&TrackingPointer;
};

Expected<std::unique_ptr<raw_pwrite_stream>> OutputFile::createProxy() {
  જો (OpenProxy)
    પાછા make_error<OutputError>(getPath(), OutputErrorCode::has_open_proxy);

  પાછા std::make_unique<TrackedProxy>(OpenProxy, getOS());
}

Error OutputFile::keep() {
  // Catch double-closing logic bugs.
  જો (LLVM_UNLIKELY(!Impl))
    report_fatal_error(
        make_error<OutputError>(getPath(), OutputErrorCode::already_closed));

  // Report a fatal error if there's an open proxy and the file is being kept.
  // This is safer than relying on clients to remember to flush(). Also call
  // OutputFile::discard() to give the backend a chance to clean up any
  // side effects (such as temporaries).
  જો (LLVM_UNLIKELY(OpenProxy))
    report_fatal_error(joinErrors(
        make_error<OutputError>(getPath(), OutputErrorCode::has_open_proxy),
        discard()));

  Error E = Impl->keep();
  Impl = nullptr;
  DiscardOnDestroyHandler = nullptr;
  પાછા E;
}

Error OutputFile::discard() {
  // Catch double-closing logic bugs.
  જો (LLVM_UNLIKELY(!Impl))
    report_fatal_error(
        make_error<OutputError>(getPath(), OutputErrorCode::already_closed));

  // Be lenient about open proxies since client teardown paths won't
  // necessarily clean up in the right order. Reset the proxy to flush any
  // current content; if there is another write, there should be quick crash on
  // null dereference.
  જો (OpenProxy)
    OpenProxy->resetProxy();

  Error E = Impl->discard();
  Impl = nullptr;
  DiscardOnDestroyHandler = nullptr;
  પાછા E;
}

શૂન્ય OutputFile::destroy() {
  જો (!Impl)
    પાછા;

  // Clean up the file. Move the discard handler into a local since discard
  // will reset it.
  auto DiscardHandler = std::move(DiscardOnDestroyHandler);
  Error E = discard();
  પ્રતિજ્ઞા(!Impl && "Expected discard to destroy Impl");

  // If there's no handler, report a fatal error.
  જો (LLVM_UNLIKELY(!DiscardHandler))
    llvm::report_fatal_error(joinErrors(
        make_error<OutputError>(getPath(), OutputErrorCode::not_closed),
        std::move(E)));
  નહીંતર જો (E)
    DiscardHandler(std::move(E));
}
