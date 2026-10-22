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

ಬಳಸಿ ನಾಮಾಕಾಶ llvm;
ಬಳಸಿ ನಾಮಾಕಾಶ llvm::vfs;

ಅಕ್ಷರ OutputFileImpl::ID = 0;
ಅಕ್ಷರ NullOutputFileImpl::ID = 0;

ಶೂನ್ಯ OutputFileImpl::anchor() {}
ಶೂನ್ಯ NullOutputFileImpl::anchor() {}

ವರ್ಗ OutputFile::TrackedProxy : ಸಾರ್ವಜನಿಕ raw_pwrite_ಹರಿವು_proxy {
ಸಾರ್ವಜನಿಕ:
  ಶೂನ್ಯ resetProxy() {
    Tracking_ಸೂಚಕ = ಶೂನ್ಯಸೂಚಕ;
    resetProxiedOS();
  }

  ಸ್ಪಷ್ಟ TrackedProxy(TrackedProxy *&Tracking_ಸೂಚಕ, raw_pwrite_ಹರಿವು &OS)
      : raw_pwrite_ಹರಿವು_proxy(OS), Tracking_ಸೂಚಕ(Tracking_ಸೂಚಕ) {
    ಪ್ರತಿಜ್ಞಾ(!Tracking_ಸೂಚಕ && "Expected to add a proxy");
    Tracking_ಸೂಚಕ = ಇದು;
  }

  ~TrackedProxy() override { resetProxy(); }

  TrackedProxy *&Tracking_ಸೂಚಕ;
};

Expected<std::unique_ptr<raw_pwrite_ಹರಿವು>> OutputFile::createProxy() {
  ಆದರೆ (OpenProxy)
    ಹಿಂತಿರುಗಿ make_error<OutputError>(get_ಮಾರ್ಗ(), Output_Error_ಸಂಕೇತ::has_open_proxy);

  ಹಿಂತಿರುಗಿ std::make_unique<TrackedProxy>(OpenProxy, getOS());
}

Error OutputFile::keep() {
  // Catch double-closing logic bugs.
  ಆದರೆ (LLVM_UNLIKELY(!Impl))
    report_fatal_error(
        make_error<OutputError>(get_ಮಾರ್ಗ(), Output_Error_ಸಂಕೇತ::already_closed));

  // Report a fatal error if there's an open proxy and the file is being kept.
  // This is safer than relying on clients to remember to flush(). Also call
  // OutputFile::discard() to give the backend a chance to clean up any
  // side effects (such as temporaries).
  ಆದರೆ (LLVM_UNLIKELY(OpenProxy))
    report_fatal_error(joinErrors(
        make_error<OutputError>(get_ಮಾರ್ಗ(), Output_Error_ಸಂಕೇತ::has_open_proxy),
        discard()));

  Error E = Impl->keep();
  Impl = ಶೂನ್ಯಸೂಚಕ;
  DiscardOnDestroyHandler = ಶೂನ್ಯಸೂಚಕ;
  ಹಿಂತಿರುಗಿ E;
}

Error OutputFile::discard() {
  // Catch double-closing logic bugs.
  ಆದರೆ (LLVM_UNLIKELY(!Impl))
    report_fatal_error(
        make_error<OutputError>(get_ಮಾರ್ಗ(), Output_Error_ಸಂಕೇತ::already_closed));

  // Be lenient about open proxies since client teardown paths won't
  // necessarily clean up in the right order. Reset the proxy to flush any
  // current content; if there is another write, there should be quick crash on
  // null dereference.
  ಆದರೆ (OpenProxy)
    OpenProxy->resetProxy();

  Error E = Impl->discard();
  Impl = ಶೂನ್ಯಸೂಚಕ;
  DiscardOnDestroyHandler = ಶೂನ್ಯಸೂಚಕ;
  ಹಿಂತಿರುಗಿ E;
}

ಶೂನ್ಯ OutputFile::destroy() {
  ಆದರೆ (!Impl)
    ಹಿಂತಿರುಗಿ;

  // Clean up the file. Move the discard handler into a local since discard
  // will reset it.
  ಸ್ವಯಂಚಾಲಿತ DiscardHandler = std::move(DiscardOnDestroyHandler);
  Error E = discard();
  ಪ್ರತಿಜ್ಞಾ(!Impl && "Expected discard to destroy Impl");

  // If there's no handler, report a fatal error.
  ಆದರೆ (LLVM_UNLIKELY(!DiscardHandler))
    llvm::report_fatal_error(joinErrors(
        make_error<OutputError>(get_ಮಾರ್ಗ(), Output_Error_ಸಂಕೇತ::not_closed),
        std::move(E)));
  ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (E)
    DiscardHandler(std::move(E));
}
