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

#অন্তর্ভাৱ "llvm/Support/VirtualOutputFile.h"
#অন্তর্ভাৱ "llvm/Support/VirtualOutputError.h"
#অন্তর্ভাৱ "llvm/Support/raw_ostream.h"
#অন্তর্ভাৱ "llvm/Support/raw_ostream_proxy.h"

using নামাকাশ llvm;
using নামাকাশ llvm::vfs;

ৱর্ণ OutputFileImpl::ID = 0;
ৱর্ণ NullOutputFileImpl::ID = 0;

শূন্য OutputFileImpl::anchor() {}
শূন্য NullOutputFileImpl::anchor() {}

শ্ৰেণী OutputFile::TrackedProxy : সার্ৱজনিক raw_pwrite_stream_proxy {
সার্ৱজনিক:
  শূন্য resetProxy() {
    TrackingPointer = nullptr;
    resetProxiedOS();
  }

  explicit TrackedProxy(TrackedProxy *&TrackingPointer, raw_pwrite_stream &OS)
      : raw_pwrite_stream_proxy(OS), TrackingPointer(TrackingPointer) {
    প্রতিজ্ঞা(!TrackingPointer && "Expected to add a proxy");
    TrackingPointer = এতত্;
  }

  ~TrackedProxy() override { resetProxy(); }

  TrackedProxy *&TrackingPointer;
};

Expected<std::unique_ptr<raw_pwrite_stream>> OutputFile::createProxy() {
  যদি (OpenProxy)
    ঘূৰাই make_error<OutputError>(getPath(), OutputErrorCode::has_open_proxy);

  ঘূৰাই std::make_unique<TrackedProxy>(OpenProxy, getOS());
}

Error OutputFile::keep() {
  // Catch double-closing logic bugs.
  যদি (LLVM_UNLIKELY(!Impl))
    report_fatal_error(
        make_error<OutputError>(getPath(), OutputErrorCode::already_closed));

  // Report a fatal error if there's an open proxy and the file is being kept.
  // This is safer than relying on clients to remember to flush(). Also call
  // OutputFile::discard() to give the backend a chance to clean up any
  // side effects (such as temporaries).
  যদি (LLVM_UNLIKELY(OpenProxy))
    report_fatal_error(joinErrors(
        make_error<OutputError>(getPath(), OutputErrorCode::has_open_proxy),
        discard()));

  Error E = Impl->keep();
  Impl = nullptr;
  DiscardOnDestroyHandler = nullptr;
  ঘূৰাই E;
}

Error OutputFile::discard() {
  // Catch double-closing logic bugs.
  যদি (LLVM_UNLIKELY(!Impl))
    report_fatal_error(
        make_error<OutputError>(getPath(), OutputErrorCode::already_closed));

  // Be lenient about open proxies since client teardown paths won't
  // necessarily clean up in the right order. Reset the proxy to flush any
  // current content; if there is another write, there should be quick crash on
  // null dereference.
  যদি (OpenProxy)
    OpenProxy->resetProxy();

  Error E = Impl->discard();
  Impl = nullptr;
  DiscardOnDestroyHandler = nullptr;
  ঘূৰাই E;
}

শূন্য OutputFile::destroy() {
  যদি (!Impl)
    ঘূৰাই;

  // Clean up the file. Move the discard handler into a local since discard
  // will reset it.
  auto DiscardHandler = std::move(DiscardOnDestroyHandler);
  Error E = discard();
  প্রতিজ্ঞা(!Impl && "Expected discard to destroy Impl");

  // If there's no handler, report a fatal error.
  যদি (LLVM_UNLIKELY(!DiscardHandler))
    llvm::report_fatal_error(joinErrors(
        make_error<OutputError>(getPath(), OutputErrorCode::not_closed),
        std::move(E)));
  অন্যথা যদি (E)
    DiscardHandler(std::move(E));
}
