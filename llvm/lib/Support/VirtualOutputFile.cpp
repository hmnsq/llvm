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

#অন্তর্ভাব "llvm/Support/VirtualOutputFile.h"
#অন্তর্ভাব "llvm/Support/VirtualOutputError.h"
#অন্তর্ভাব "llvm/Support/raw_ostream.h"
#অন্তর্ভাব "llvm/Support/raw_ostream_proxy.h"

using নামাকাশ llvm;
using নামাকাশ llvm::vfs;

অক্ষর OutputFileImpl::ID = 0;
অক্ষর NullOutputFileImpl::ID = 0;

শূন্য OutputFileImpl::anchor() {}
শূন্য NullOutputFileImpl::anchor() {}

শ্রেণী OutputFile::TrackedProxy : সর্বজনীন raw_pwrite_stream_proxy {
সর্বজনীন:
  শূন্য resetProxy() {
    TrackingPointer = nullptr;
    resetProxiedOS();
  }

  explicit TrackedProxy(TrackedProxy *&TrackingPointer, raw_pwrite_stream &OS)
      : raw_pwrite_stream_proxy(OS), TrackingPointer(TrackingPointer) {
    প্রতিজ্ঞা(!TrackingPointer && "Expected to add a proxy");
    TrackingPointer = এই;
  }

  ~TrackedProxy() override { resetProxy(); }

  TrackedProxy *&TrackingPointer;
};

Expected<std::unique_ptr<raw_pwrite_stream>> OutputFile::createProxy() {
  যদি (OpenProxy)
    ফেরত make_error<OutputError>(getPath(), OutputErrorCode::has_open_proxy);

  ফেরত std::make_unique<TrackedProxy>(OpenProxy, getOS());
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
  ফেরত E;
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
  ফেরত E;
}

শূন্য OutputFile::destroy() {
  যদি (!Impl)
    ফেরত;

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
