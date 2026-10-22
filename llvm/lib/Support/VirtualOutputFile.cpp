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

#شامل "llvm/Support/VirtualOutputFile.h"
#شامل "llvm/Support/VirtualOutputError.h"
#شامل "llvm/Support/raw_ostream.h"
#شامل "llvm/Support/raw_ostream_proxy.h"

استعمال نامخلا llvm;
استعمال نامخلا llvm::vfs;

حرف OutputFileImpl::ID = 0;
حرف NullOutputFileImpl::ID = 0;

خالی OutputFileImpl::anchor() {}
خالی NullOutputFileImpl::anchor() {}

جماعت OutputFile::TrackedProxy : عوامی raw_pwrite_دھارا_proxy {
عوامی:
  خالی resetProxy() {
    Tracking_اشارہ_گر = خالی_اشاریہ;
    resetProxiedOS();
  }

  صریح TrackedProxy(TrackedProxy *&Tracking_اشارہ_گر, raw_pwrite_دھارا &OS)
      : raw_pwrite_دھارا_proxy(OS), Tracking_اشارہ_گر(Tracking_اشارہ_گر) {
    أكد(!Tracking_اشارہ_گر && "Expected to add a proxy");
    Tracking_اشارہ_گر = یہ;
  }

  ~TrackedProxy() override { resetProxy(); }

  TrackedProxy *&Tracking_اشارہ_گر;
};

Expected<std::unique_ptr<raw_pwrite_دھارا>> OutputFile::createProxy() {
  اگر (OpenProxy)
    واپس make_error<OutputError>(get_راستہ(), Output_Error_رمز::has_open_proxy);

  واپس std::make_unique<TrackedProxy>(OpenProxy, getOS());
}

Error OutputFile::keep() {
  // Catch double-closing logic bugs.
  اگر (LLVM_UNLIKELY(!Impl))
    report_fatal_error(
        make_error<OutputError>(get_راستہ(), Output_Error_رمز::already_closed));

  // Report a fatal error if there's an open proxy and the file is being kept.
  // This is safer than relying on clients to remember to flush(). Also call
  // OutputFile::discard() to give the backend a chance to clean up any
  // side effects (such as temporaries).
  اگر (LLVM_UNLIKELY(OpenProxy))
    report_fatal_error(joinErrors(
        make_error<OutputError>(get_راستہ(), Output_Error_رمز::has_open_proxy),
        discard()));

  Error E = Impl->keep();
  Impl = خالی_اشاریہ;
  DiscardOnDestroyHandler = خالی_اشاریہ;
  واپس E;
}

Error OutputFile::discard() {
  // Catch double-closing logic bugs.
  اگر (LLVM_UNLIKELY(!Impl))
    report_fatal_error(
        make_error<OutputError>(get_راستہ(), Output_Error_رمز::already_closed));

  // Be lenient about open proxies since client teardown paths won't
  // necessarily clean up in the right order. Reset the proxy to flush any
  // current content; if there is another write, there should be quick crash on
  // null dereference.
  اگر (OpenProxy)
    OpenProxy->resetProxy();

  Error E = Impl->discard();
  Impl = خالی_اشاریہ;
  DiscardOnDestroyHandler = خالی_اشاریہ;
  واپس E;
}

خالی OutputFile::destroy() {
  اگر (!Impl)
    واپس;

  // Clean up the file. Move the discard handler into a local since discard
  // will reset it.
  خودکار DiscardHandler = std::move(DiscardOnDestroyHandler);
  Error E = discard();
  أكد(!Impl && "Expected discard to destroy Impl");

  // If there's no handler, report a fatal error.
  اگر (LLVM_UNLIKELY(!DiscardHandler))
    llvm::report_fatal_error(joinErrors(
        make_error<OutputError>(get_راستہ(), Output_Error_رمز::not_closed),
        std::move(E)));
  ورنہ اگر (E)
    DiscardHandler(std::move(E));
}
