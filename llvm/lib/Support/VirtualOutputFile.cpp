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

#ಅನ್ತರ್ಭಾವ "llvm/Support/VirtualOutputFile.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/VirtualOutputError.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/raw_ostream.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/raw_ostream_proxy.h"

using ನಾಮಾಕಾಶ llvm;
using ನಾಮಾಕಾಶ llvm::vfs;

ವರ್ಣ OutputFileImpl::ID = 0;
ವರ್ಣ NullOutputFileImpl::ID = 0;

ಶೂನ್ಯ OutputFileImpl::anchor() {}
ಶೂನ್ಯ NullOutputFileImpl::anchor() {}

ವರ್ಗ OutputFile::TrackedProxy : ಸಾರ್ವಜನಿಕ raw_pwrite_stream_proxy {
ಸಾರ್ವಜನಿಕ:
  ಶೂನ್ಯ resetProxy() {
    TrackingPointer = nullptr;
    resetProxiedOS();
  }

  explicit TrackedProxy(TrackedProxy *&TrackingPointer, raw_pwrite_stream &OS)
      : raw_pwrite_stream_proxy(OS), TrackingPointer(TrackingPointer) {
    ಪ್ರತಿಜ್ಞಾ(!TrackingPointer && "Expected to add a proxy");
    TrackingPointer = ಏತತ್;
  }

  ~TrackedProxy() override { resetProxy(); }

  TrackedProxy *&TrackingPointer;
};

Expected<std::unique_ptr<raw_pwrite_stream>> OutputFile::createProxy() {
  ಯದಿ (OpenProxy)
    ಪ್ರತ್ಯಾಗಮ make_error<OutputError>(getPath(), OutputErrorCode::has_open_proxy);

  ಪ್ರತ್ಯಾಗಮ std::make_unique<TrackedProxy>(OpenProxy, getOS());
}

Error OutputFile::keep() {
  // Catch double-closing logic bugs.
  ಯದಿ (LLVM_UNLIKELY(!Impl))
    report_fatal_error(
        make_error<OutputError>(getPath(), OutputErrorCode::already_closed));

  // Report a fatal error if there's an open proxy and the file is being kept.
  // This is safer than relying on clients to remember to flush(). Also call
  // OutputFile::discard() to give the backend a chance to clean up any
  // side effects (such as temporaries).
  ಯದಿ (LLVM_UNLIKELY(OpenProxy))
    report_fatal_error(joinErrors(
        make_error<OutputError>(getPath(), OutputErrorCode::has_open_proxy),
        discard()));

  Error E = Impl->keep();
  Impl = nullptr;
  DiscardOnDestroyHandler = nullptr;
  ಪ್ರತ್ಯಾಗಮ E;
}

Error OutputFile::discard() {
  // Catch double-closing logic bugs.
  ಯದಿ (LLVM_UNLIKELY(!Impl))
    report_fatal_error(
        make_error<OutputError>(getPath(), OutputErrorCode::already_closed));

  // Be lenient about open proxies since client teardown paths won't
  // necessarily clean up in the right order. Reset the proxy to flush any
  // current content; if there is another write, there should be quick crash on
  // null dereference.
  ಯದಿ (OpenProxy)
    OpenProxy->resetProxy();

  Error E = Impl->discard();
  Impl = nullptr;
  DiscardOnDestroyHandler = nullptr;
  ಪ್ರತ್ಯಾಗಮ E;
}

ಶೂನ್ಯ OutputFile::destroy() {
  ಯದಿ (!Impl)
    ಪ್ರತ್ಯಾಗಮ;

  // Clean up the file. Move the discard handler into a local since discard
  // will reset it.
  auto DiscardHandler = std::move(DiscardOnDestroyHandler);
  Error E = discard();
  ಪ್ರತಿಜ್ಞಾ(!Impl && "Expected discard to destroy Impl");

  // If there's no handler, report a fatal error.
  ಯದಿ (LLVM_UNLIKELY(!DiscardHandler))
    llvm::report_fatal_error(joinErrors(
        make_error<OutputError>(getPath(), OutputErrorCode::not_closed),
        std::move(E)));
  ಅನ್ಯಥಾ ಯದಿ (E)
    DiscardHandler(std::move(E));
}
