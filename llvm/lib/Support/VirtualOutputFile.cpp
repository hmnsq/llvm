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

#ضمن "llvm/Support/VirtualOutputFile.h"
#ضمن "llvm/Support/VirtualOutputError.h"
#ضمن "llvm/Support/raw_ostream.h"
#ضمن "llvm/Support/raw_ostream_proxy.h"

استفاده نطاق llvm;
استفاده نطاق llvm::vfs;

حرف OutputFileImpl::ID = 0;
حرف NullOutputFileImpl::ID = 0;

فراغ OutputFileImpl::anchor() {}
فراغ NullOutputFileImpl::anchor() {}

صنف OutputFile::TrackedProxy : عام raw_pwrite_جریان_proxy {
عام:
  فراغ resetProxy() {
    Tracking_اشاره_گر = اشاره_تهی;
    resetProxiedOS();
  }

  صریح TrackedProxy(TrackedProxy *&Tracking_اشاره_گر, raw_pwrite_جریان &OS)
      : raw_pwrite_جریان_proxy(OS), Tracking_اشاره_گر(Tracking_اشاره_گر) {
    أكد(!Tracking_اشاره_گر && "Expected to add a proxy");
    Tracking_اشاره_گر = هذا;
  }

  ~TrackedProxy() override { resetProxy(); }

  TrackedProxy *&Tracking_اشاره_گر;
};

Expected<std::unique_ptr<raw_pwrite_جریان>> OutputFile::createProxy() {
  إذا (OpenProxy)
    أرجع make_error<OutputError>(get_مسیر(), Output_Error_کد::has_open_proxy);

  أرجع std::make_unique<TrackedProxy>(OpenProxy, getOS());
}

Error OutputFile::keep() {
  // Catch double-closing logic bugs.
  إذا (LLVM_UNLIKELY(!Impl))
    report_fatal_error(
        make_error<OutputError>(get_مسیر(), Output_Error_کد::already_closed));

  // Report a fatal error if there's an open proxy and the file is being kept.
  // This is safer than relying on clients to remember to flush(). Also call
  // OutputFile::discard() to give the backend a chance to clean up any
  // side effects (such as temporaries).
  إذا (LLVM_UNLIKELY(OpenProxy))
    report_fatal_error(joinErrors(
        make_error<OutputError>(get_مسیر(), Output_Error_کد::has_open_proxy),
        discard()));

  Error E = Impl->keep();
  Impl = اشاره_تهی;
  DiscardOnDestroyHandler = اشاره_تهی;
  أرجع E;
}

Error OutputFile::discard() {
  // Catch double-closing logic bugs.
  إذا (LLVM_UNLIKELY(!Impl))
    report_fatal_error(
        make_error<OutputError>(get_مسیر(), Output_Error_کد::already_closed));

  // Be lenient about open proxies since client teardown paths won't
  // necessarily clean up in the right order. Reset the proxy to flush any
  // current content; if there is another write, there should be quick crash on
  // null dereference.
  إذا (OpenProxy)
    OpenProxy->resetProxy();

  Error E = Impl->discard();
  Impl = اشاره_تهی;
  DiscardOnDestroyHandler = اشاره_تهی;
  أرجع E;
}

فراغ OutputFile::destroy() {
  إذا (!Impl)
    أرجع;

  // Clean up the file. Move the discard handler into a local since discard
  // will reset it.
  خودکار DiscardHandler = std::move(DiscardOnDestroyHandler);
  Error E = discard();
  أكد(!Impl && "Expected discard to destroy Impl");

  // If there's no handler, report a fatal error.
  إذا (LLVM_UNLIKELY(!DiscardHandler))
    llvm::report_fatal_error(joinErrors(
        make_error<OutputError>(get_مسیر(), Output_Error_کد::not_closed),
        std::move(E)));
  وإلا إذا (E)
    DiscardHandler(std::move(E));
}
