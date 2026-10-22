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

#अन्तर्भाव "llvm/Support/VirtualOutputFile.h"
#अन्तर्भाव "llvm/Support/VirtualOutputError.h"
#अन्तर्भाव "llvm/Support/raw_ostream.h"
#अन्तर्भाव "llvm/Support/raw_ostream_proxy.h"

using नामाकाश llvm;
using नामाकाश llvm::vfs;

वर्ण OutputFileImpl::ID = 0;
वर्ण NullOutputFileImpl::ID = 0;

शून्य OutputFileImpl::anchor() {}
शून्य NullOutputFileImpl::anchor() {}

वग्ग OutputFile::TrackedProxy : सार्वजनिक raw_pwrite_stream_proxy {
सार्वजनिक:
  शून्य resetProxy() {
    TrackingPointer = nullptr;
    resetProxiedOS();
  }

  explicit TrackedProxy(TrackedProxy *&TrackingPointer, raw_pwrite_stream &OS)
      : raw_pwrite_stream_proxy(OS), TrackingPointer(TrackingPointer) {
    प्रतिज्ञा(!TrackingPointer && "Expected to add a proxy");
    TrackingPointer = एतत्;
  }

  ~TrackedProxy() override { resetProxy(); }

  TrackedProxy *&TrackingPointer;
};

Expected<std::unique_ptr<raw_pwrite_stream>> OutputFile::createProxy() {
  सचे (OpenProxy)
    पटिनिवत्त make_error<OutputError>(getPath(), OutputErrorCode::has_open_proxy);

  पटिनिवत्त std::make_unique<TrackedProxy>(OpenProxy, getOS());
}

Error OutputFile::keep() {
  // Catch double-closing logic bugs.
  सचे (LLVM_UNLIKELY(!Impl))
    report_fatal_error(
        make_error<OutputError>(getPath(), OutputErrorCode::already_closed));

  // Report a fatal error if there's an open proxy and the file is being kept.
  // This is safer than relying on clients to remember to flush(). Also call
  // OutputFile::discard() to give the backend a chance to clean up any
  // side effects (such as temporaries).
  सचे (LLVM_UNLIKELY(OpenProxy))
    report_fatal_error(joinErrors(
        make_error<OutputError>(getPath(), OutputErrorCode::has_open_proxy),
        discard()));

  Error E = Impl->keep();
  Impl = nullptr;
  DiscardOnDestroyHandler = nullptr;
  पटिनिवत्त E;
}

Error OutputFile::discard() {
  // Catch double-closing logic bugs.
  सचे (LLVM_UNLIKELY(!Impl))
    report_fatal_error(
        make_error<OutputError>(getPath(), OutputErrorCode::already_closed));

  // Be lenient about open proxies since client teardown paths won't
  // necessarily clean up in the right order. Reset the proxy to flush any
  // current content; if there is another write, there should be quick crash on
  // null dereference.
  सचे (OpenProxy)
    OpenProxy->resetProxy();

  Error E = Impl->discard();
  Impl = nullptr;
  DiscardOnDestroyHandler = nullptr;
  पटिनिवत्त E;
}

शून्य OutputFile::destroy() {
  सचे (!Impl)
    पटिनिवत्त;

  // Clean up the file. Move the discard handler into a local since discard
  // will reset it.
  auto DiscardHandler = std::move(DiscardOnDestroyHandler);
  Error E = discard();
  प्रतिज्ञा(!Impl && "Expected discard to destroy Impl");

  // If there's no handler, report a fatal error.
  सचे (LLVM_UNLIKELY(!DiscardHandler))
    llvm::report_fatal_error(joinErrors(
        make_error<OutputError>(getPath(), OutputErrorCode::not_closed),
        std::move(E)));
  अञ्ञथा सचे (E)
    DiscardHandler(std::move(E));
}
