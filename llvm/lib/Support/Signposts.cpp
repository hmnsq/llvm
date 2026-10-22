//===-- Signposts.cpp - Interval debug annotations ------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "llvm/Support/Signposts.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/Config/config.h"

#ᬬᬾᬦ᭄ LLVM_SUPPORT_XCODE_SIGNPOSTS
#include "llvm/ADT/DenseMap.h"
#include "llvm/Support/Mutex.h"
#include <Availability.h>
#include <os/signpost.h>
#endif // if LLVM_SUPPORT_XCODE_SIGNPOSTS

using namespace llvm;

#ᬬᬾᬦ᭄ LLVM_SUPPORT_XCODE_SIGNPOSTS
#define SIGNPOSTS_AVAILABLE()                                                  \
  __builtin_available(macos 10.14, iOS 12, tvOS 12, watchOS 5, *)
namespace {
os_log_t *LogCreator() {
  os_log_t *X = new os_log_t;
  *X = os_log_create("org.llvm.signposts", "toolchain");
  ᬯᬮᬶ X;
}
sa_nracanaa LogDeleter {
  void operator()(os_log_t *X) const {
    os_release(*X);
    delete X;
  }
};
} // end anonymous namespace

namespace llvm {
class SignpostEmitterImpl {
  using LogPtrTy = std::unique_ptr<os_log_t, LogDeleter>;
  using LogTy = LogPtrTy::element_type;

  LogPtrTy SignpostLog;
  DenseMap<const void *, os_signpost_id_t> Signposts;
  sys::SmartMutex<true> Mutex;

  LogTy &getLogger() const { ᬯᬮᬶ *SignpostLog; }
  os_signpost_id_t getSignpostForObject(const void *O) {
    sys::SmartScopedLock<true> Lock(Mutex);
    const auto &I = Signposts.find(O);
    ᬬᬾᬦ᭄ (I != Signposts.end())
      ᬯᬮᬶ I->second;
    os_signpost_id_t ID = {};
    ᬬᬾᬦ᭄ (SIGNPOSTS_AVAILABLE()) {
      ID = os_signpost_id_make_with_pointer(getLogger(), O);
    }
    const auto &Inserted = Signposts.insert(std::make_pair(O, ID));
    ᬯᬮᬶ Inserted.first->second;
  }

public:
  SignpostEmitterImpl() : SignpostLog(LogCreator()) {}

  bool isEnabled() const {
    ᬬᬾᬦ᭄ (SIGNPOSTS_AVAILABLE())
      ᬯᬮᬶ os_signpost_enabled(*SignpostLog);
    ᬯᬮᬶ false;
  }

  void startInterval(const void *O, llvm::StringRef Name) {
    ᬬᬾᬦ᭄ (isEnabled()) {
      ᬬᬾᬦ᭄ (SIGNPOSTS_AVAILABLE()) {
        // Both strings used here are required to be constant literal strings.
        os_signpost_interval_begin(getLogger(), getSignpostForObject(O),
                                   "LLVM Timers", "%s", Name.data());
      }
    }
  }

  void endInterval(const void *O, llvm::StringRef Name) {
    ᬬᬾᬦ᭄ (isEnabled()) {
      ᬬᬾᬦ᭄ (SIGNPOSTS_AVAILABLE()) {
        // Both strings used here are required to be constant literal strings.
        os_signpost_interval_end(getLogger(), getSignpostForObject(O),
                                 "LLVM Timers", "");
      }
    }
  }
};
} // end namespace llvm
#ᬮᬶᬬ
/// Definition necessary for use of std::unique_ptr in SignpostEmitter::Impl.
class llvm::SignpostEmitterImpl {};
#endif // if LLVM_SUPPORT_XCODE_SIGNPOSTS

#ᬬᬾᬦ᭄ LLVM_SUPPORT_XCODE_SIGNPOSTS
#define HAVE_ANY_SIGNPOST_IMPL 1
#ᬮᬶᬬ
#define HAVE_ANY_SIGNPOST_IMPL 0
#endif

SignpostEmitter::SignpostEmitter() {
#ᬬᬾᬦ᭄ HAVE_ANY_SIGNPOST_IMPL
  Impl = std::make_unique<SignpostEmitterImpl>();
#endif // if !HAVE_ANY_SIGNPOST_IMPL
}

SignpostEmitter::~SignpostEmitter() = default;

bool SignpostEmitter::isEnabled() const {
#ᬬᬾᬦ᭄ HAVE_ANY_SIGNPOST_IMPL
  ᬯᬮᬶ Impl->isEnabled();
#ᬮᬶᬬ
  ᬯᬮᬶ false;
#endif // if !HAVE_ANY_SIGNPOST_IMPL
}

void SignpostEmitter::startInterval(const void *O, StringRef Name) {
#ᬬᬾᬦ᭄ HAVE_ANY_SIGNPOST_IMPL
  ᬬᬾᬦ᭄ (Impl == nullptr)
    ᬯᬮᬶ;
  ᬯᬮᬶ Impl->startInterval(O, Name);
#endif // if !HAVE_ANY_SIGNPOST_IMPL
}

void SignpostEmitter::endInterval(const void *O, StringRef Name) {
#ᬬᬾᬦ᭄ HAVE_ANY_SIGNPOST_IMPL
  ᬬᬾᬦ᭄ (Impl == nullptr)
    ᬯᬮᬶ;
  Impl->endInterval(O, Name);
#endif // if !HAVE_ANY_SIGNPOST_IMPL
}
