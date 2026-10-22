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

उपयुज्य नामाकाश llvm;
उपयुज्य नामाकाश llvm::vfs;

वर्ण OutputFileImpl::ID = 0;
वर्ण NullOutputFileImpl::ID = 0;

शून्य OutputFileImpl::anchor() {}
शून्य NullOutputFileImpl::anchor() {}

वर्ग OutputFile::TrackedProxy : सार्वजनिक raw_pwrite_प्रवाहः_proxy {
सार्वजनिक:
  शून्य resetProxy() {
    TrackingPointer = शून्यसूचकः;
    resetProxiedOS();
  }

  स्पष्ट TrackedProxy(TrackedProxy *&TrackingPointer, raw_pwrite_प्रवाहः &OS)
      : raw_pwrite_प्रवाहः_proxy(OS), TrackingPointer(TrackingPointer) {
    प्रतिज्ञा(!TrackingPointer && "Expected to add a proxy");
    TrackingPointer = एतत्;
  }

  ~TrackedProxy() override { resetProxy(); }

  TrackedProxy *&TrackingPointer;
};

Expected<std::unique_ptr<raw_pwrite_प्रवाहः>> OutputFile::createProxy() {
  यदि (OpenProxy)
    प्रत्यागम make_error<OutputError>(get_मार्गः(), Output_Error_संहिता::has_open_proxy);

  प्रत्यागम std::make_unique<TrackedProxy>(OpenProxy, getOS());
}

Error OutputFile::keep() {
  // Catch double-closing logic bugs.
  यदि (LLVM_UNLIKELY(!Impl))
    report_fatal_error(
        make_error<OutputError>(get_मार्गः(), Output_Error_संहिता::already_closed));

  // Report a fatal error if there's an open proxy and the file is being kept.
  // This is safer than relying on clients to remember to flush(). Also call
  // OutputFile::discard() to give the backend a chance to clean up any
  // side effects (such as temporaries).
  यदि (LLVM_UNLIKELY(OpenProxy))
    report_fatal_error(joinErrors(
        make_error<OutputError>(get_मार्गः(), Output_Error_संहिता::has_open_proxy),
        discard()));

  Error E = Impl->keep();
  Impl = शून्यसूचकः;
  DiscardOnDestroyHandler = शून्यसूचकः;
  प्रत्यागम E;
}

Error OutputFile::discard() {
  // Catch double-closing logic bugs.
  यदि (LLVM_UNLIKELY(!Impl))
    report_fatal_error(
        make_error<OutputError>(get_मार्गः(), Output_Error_संहिता::already_closed));

  // Be lenient about open proxies since client teardown paths won't
  // necessarily clean up in the right order. Reset the proxy to flush any
  // current content; if there is another write, there should be quick crash on
  // null dereference.
  यदि (OpenProxy)
    OpenProxy->resetProxy();

  Error E = Impl->discard();
  Impl = शून्यसूचकः;
  DiscardOnDestroyHandler = शून्यसूचकः;
  प्रत्यागम E;
}

शून्य OutputFile::destroy() {
  यदि (!Impl)
    प्रत्यागम;

  // Clean up the file. Move the discard handler into a local since discard
  // will reset it.
  स्वचालित DiscardHandler = std::move(DiscardOnDestroyHandler);
  Error E = discard();
  प्रतिज्ञा(!Impl && "Expected discard to destroy Impl");

  // If there's no handler, report a fatal error.
  यदि (LLVM_UNLIKELY(!DiscardHandler))
    llvm::report_fatal_error(joinErrors(
        make_error<OutputError>(get_मार्गः(), Output_Error_संहिता::not_closed),
        std::move(E)));
  अन्यथा यदि (E)
    DiscardHandler(std::move(E));
}
