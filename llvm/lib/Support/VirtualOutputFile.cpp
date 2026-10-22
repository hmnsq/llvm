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

#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/VirtualOutputFile.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/VirtualOutputError.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/raw_ostream.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/raw_ostream_proxy.h"

ਵਰਤੋਂ ਨਾਮਾਕਾਸ਼ llvm;
ਵਰਤੋਂ ਨਾਮਾਕਾਸ਼ llvm::vfs;

ਅੱਖਰ OutputFileImpl::ID = 0;
ਅੱਖਰ NullOutputFileImpl::ID = 0;

ਖਾਲੀ OutputFileImpl::anchor() {}
ਖਾਲੀ NullOutputFileImpl::anchor() {}

ਵਰਗ OutputFile::TrackedProxy : ਜਨਤਕ raw_pwrite_ਧਾਰਾ_proxy {
ਜਨਤਕ:
  ਖਾਲੀ resetProxy() {
    Tracking_ਸੂਚਕ = ਖਾਲੀ_ਸੂਚਕ;
    resetProxiedOS();
  }

  ਸਪਸ਼ਟ TrackedProxy(TrackedProxy *&Tracking_ਸੂਚਕ, raw_pwrite_ਧਾਰਾ &OS)
      : raw_pwrite_ਧਾਰਾ_proxy(OS), Tracking_ਸੂਚਕ(Tracking_ਸੂਚਕ) {
    ਪ੍ਰਤਿਜ੍ਞਾ(!Tracking_ਸੂਚਕ && "Expected to add a proxy");
    Tracking_ਸੂਚਕ = ਇਹ;
  }

  ~TrackedProxy() override { resetProxy(); }

  TrackedProxy *&Tracking_ਸੂਚਕ;
};

Expected<std::unique_ptr<raw_pwrite_ਧਾਰਾ>> OutputFile::createProxy() {
  ਜੇ (OpenProxy)
    ਵਾਪਸ make_error<OutputError>(get_ਰਸਤਾ(), Output_Error_ਕੋਡ::has_open_proxy);

  ਵਾਪਸ std::make_unique<TrackedProxy>(OpenProxy, getOS());
}

Error OutputFile::keep() {
  // Catch double-closing logic bugs.
  ਜੇ (LLVM_UNLIKELY(!Impl))
    report_fatal_error(
        make_error<OutputError>(get_ਰਸਤਾ(), Output_Error_ਕੋਡ::already_closed));

  // Report a fatal error if there's an open proxy and the file is being kept.
  // This is safer than relying on clients to remember to flush(). Also call
  // OutputFile::discard() to give the backend a chance to clean up any
  // side effects (such as temporaries).
  ਜੇ (LLVM_UNLIKELY(OpenProxy))
    report_fatal_error(joinErrors(
        make_error<OutputError>(get_ਰਸਤਾ(), Output_Error_ਕੋਡ::has_open_proxy),
        discard()));

  Error E = Impl->keep();
  Impl = ਖਾਲੀ_ਸੂਚਕ;
  DiscardOnDestroyHandler = ਖਾਲੀ_ਸੂਚਕ;
  ਵਾਪਸ E;
}

Error OutputFile::discard() {
  // Catch double-closing logic bugs.
  ਜੇ (LLVM_UNLIKELY(!Impl))
    report_fatal_error(
        make_error<OutputError>(get_ਰਸਤਾ(), Output_Error_ਕੋਡ::already_closed));

  // Be lenient about open proxies since client teardown paths won't
  // necessarily clean up in the right order. Reset the proxy to flush any
  // current content; if there is another write, there should be quick crash on
  // null dereference.
  ਜੇ (OpenProxy)
    OpenProxy->resetProxy();

  Error E = Impl->discard();
  Impl = ਖਾਲੀ_ਸੂਚਕ;
  DiscardOnDestroyHandler = ਖਾਲੀ_ਸੂਚਕ;
  ਵਾਪਸ E;
}

ਖਾਲੀ OutputFile::destroy() {
  ਜੇ (!Impl)
    ਵਾਪਸ;

  // Clean up the file. Move the discard handler into a local since discard
  // will reset it.
  ਸਵੈਚਾਲਿਤ DiscardHandler = std::move(DiscardOnDestroyHandler);
  Error E = discard();
  ਪ੍ਰਤਿਜ੍ਞਾ(!Impl && "Expected discard to destroy Impl");

  // If there's no handler, report a fatal error.
  ਜੇ (LLVM_UNLIKELY(!DiscardHandler))
    llvm::report_fatal_error(joinErrors(
        make_error<OutputError>(get_ਰਸਤਾ(), Output_Error_ਕੋਡ::not_closed),
        std::move(E)));
  ਨਹੀਂਤਾਂ ਜੇ (E)
    DiscardHandler(std::move(E));
}
