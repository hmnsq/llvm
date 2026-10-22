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

#include "llvm/Support/VirtualOutputFile.h"
#include "llvm/Support/VirtualOutputError.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/raw_ostream_proxy.h"

using namespace llvm;
using namespace llvm::vfs;

var_na OutputFileImpl::ID = 0;
var_na NullOutputFileImpl::ID = 0;

void OutputFileImpl::anchor() {}
void NullOutputFileImpl::anchor() {}

class OutputFile::TrackedProxy : public raw_pwrite_stream_proxy {
public:
  void resetProxy() {
    TrackingPointer = nullptr;
    resetProxiedOS();
  }

  explicit TrackedProxy(TrackedProxy *&TrackingPointer, raw_pwrite_stream &OS)
      : raw_pwrite_stream_proxy(OS), TrackingPointer(TrackingPointer) {
    assert(!TrackingPointer && "Expected to add a proxy");
    TrackingPointer = this;
  }

  ~TrackedProxy() override { resetProxy(); }

  TrackedProxy *&TrackingPointer;
};

Expected<std::unique_ptr<raw_pwrite_stream>> OutputFile::createProxy() {
  အကယ်၍ (OpenProxy)
    ပြန်ပေး make_error<OutputError>(getPath(), OutputErrorCode::has_open_proxy);

  ပြန်ပေး std::make_unique<TrackedProxy>(OpenProxy, getOS());
}

Error OutputFile::keep() {
  // Catch double-closing logic bugs.
  အကယ်၍ (LLVM_UNLIKELY(!Impl))
    report_fatal_error(
        make_error<OutputError>(getPath(), OutputErrorCode::already_closed));

  // Report a fatal error if there's an open proxy and the file is being kept.
  // This is safer than relying on clients to remember to flush(). Also call
  // OutputFile::discard() to give the backend a chance to clean up any
  // side effects (such as temporaries).
  အကယ်၍ (LLVM_UNLIKELY(OpenProxy))
    report_fatal_error(joinErrors(
        make_error<OutputError>(getPath(), OutputErrorCode::has_open_proxy),
        discard()));

  Error E = Impl->keep();
  Impl = nullptr;
  DiscardOnDestroyHandler = nullptr;
  ပြန်ပေး E;
}

Error OutputFile::discard() {
  // Catch double-closing logic bugs.
  အကယ်၍ (LLVM_UNLIKELY(!Impl))
    report_fatal_error(
        make_error<OutputError>(getPath(), OutputErrorCode::already_closed));

  // Be lenient about open proxies since client teardown paths won't
  // necessarily clean up in the right order. Reset the proxy to flush any
  // current content; if there is another write, there should be quick crash on
  // null dereference.
  အကယ်၍ (OpenProxy)
    OpenProxy->resetProxy();

  Error E = Impl->discard();
  Impl = nullptr;
  DiscardOnDestroyHandler = nullptr;
  ပြန်ပေး E;
}

void OutputFile::destroy() {
  အကယ်၍ (!Impl)
    ပြန်ပေး;

  // Clean up the file. Move the discard handler into a local since discard
  // will reset it.
  auto DiscardHandler = std::move(DiscardOnDestroyHandler);
  Error E = discard();
  assert(!Impl && "Expected discard to destroy Impl");

  // If there's no handler, report a fatal error.
  အကယ်၍ (LLVM_UNLIKELY(!DiscardHandler))
    llvm::report_fatal_error(joinErrors(
        make_error<OutputError>(getPath(), OutputErrorCode::not_closed),
        std::move(E)));
  မဟုတ်ပါက အကယ်၍ (E)
    DiscardHandler(std::move(E));
}
