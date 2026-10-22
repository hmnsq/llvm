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

#ᮚᮦᮔ᮪ LLVM_SUPPORT_XCODE_SIGNPOSTS
#include "llvm/ADT/DenseMap.h"
#include "llvm/Support/Mutex.h"
#include <Availability.h>
#include <os/signpost.h>
#endif // if LLVM_SUPPORT_XCODE_SIGNPOSTS

using namespace llvm;

#ᮚᮦᮔ᮪ LLVM_SUPPORT_XCODE_SIGNPOSTS
#define SIGNPOSTS_AVAILABLE()                                                  \
  __builtin_available(macos 10.14, iOS 12, tvOS 12, watchOS 5, *)
namespace {
os_log_t *LogCreator() {
  os_log_t *X = new os_log_t;
  *X = os_log_create("org.llvm.signposts", "toolchain");
  ᮝᮜᮤ X;
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

  LogTy &getLogger() const { ᮝᮜᮤ *SignpostLog; }
  os_signpost_id_t getSignpostForObject(const void *O) {
    sys::SmartScopedLock<true> Lock(Mutex);
    const auto &I = Signposts.find(O);
    ᮚᮦᮔ᮪ (I != Signposts.end())
      ᮝᮜᮤ I->second;
    os_signpost_id_t ID = {};
    ᮚᮦᮔ᮪ (SIGNPOSTS_AVAILABLE()) {
      ID = os_signpost_id_make_with_pointer(getLogger(), O);
    }
    const auto &Inserted = Signposts.insert(std::make_pair(O, ID));
    ᮝᮜᮤ Inserted.first->second;
  }

public:
  SignpostEmitterImpl() : SignpostLog(LogCreator()) {}

  bool isEnabled() const {
    ᮚᮦᮔ᮪ (SIGNPOSTS_AVAILABLE())
      ᮝᮜᮤ os_signpost_enabled(*SignpostLog);
    ᮝᮜᮤ false;
  }

  void startInterval(const void *O, llvm::StringRef Name) {
    ᮚᮦᮔ᮪ (isEnabled()) {
      ᮚᮦᮔ᮪ (SIGNPOSTS_AVAILABLE()) {
        // Both strings used here are required to be constant literal strings.
        os_signpost_interval_begin(getLogger(), getSignpostForObject(O),
                                   "LLVM Timers", "%s", Name.data());
      }
    }
  }

  void endInterval(const void *O, llvm::StringRef Name) {
    ᮚᮦᮔ᮪ (isEnabled()) {
      ᮚᮦᮔ᮪ (SIGNPOSTS_AVAILABLE()) {
        // Both strings used here are required to be constant literal strings.
        os_signpost_interval_end(getLogger(), getSignpostForObject(O),
                                 "LLVM Timers", "");
      }
    }
  }
};
} // end namespace llvm
#ᮜᮤᮚ
/// Definition necessary for use of std::unique_ptr in SignpostEmitter::Impl.
class llvm::SignpostEmitterImpl {};
#endif // if LLVM_SUPPORT_XCODE_SIGNPOSTS

#ᮚᮦᮔ᮪ LLVM_SUPPORT_XCODE_SIGNPOSTS
#define HAVE_ANY_SIGNPOST_IMPL 1
#ᮜᮤᮚ
#define HAVE_ANY_SIGNPOST_IMPL 0
#endif

SignpostEmitter::SignpostEmitter() {
#ᮚᮦᮔ᮪ HAVE_ANY_SIGNPOST_IMPL
  Impl = std::make_unique<SignpostEmitterImpl>();
#endif // if !HAVE_ANY_SIGNPOST_IMPL
}

SignpostEmitter::~SignpostEmitter() = default;

bool SignpostEmitter::isEnabled() const {
#ᮚᮦᮔ᮪ HAVE_ANY_SIGNPOST_IMPL
  ᮝᮜᮤ Impl->isEnabled();
#ᮜᮤᮚ
  ᮝᮜᮤ false;
#endif // if !HAVE_ANY_SIGNPOST_IMPL
}

void SignpostEmitter::startInterval(const void *O, StringRef Name) {
#ᮚᮦᮔ᮪ HAVE_ANY_SIGNPOST_IMPL
  ᮚᮦᮔ᮪ (Impl == nullptr)
    ᮝᮜᮤ;
  ᮝᮜᮤ Impl->startInterval(O, Name);
#endif // if !HAVE_ANY_SIGNPOST_IMPL
}

void SignpostEmitter::endInterval(const void *O, StringRef Name) {
#ᮚᮦᮔ᮪ HAVE_ANY_SIGNPOST_IMPL
  ᮚᮦᮔ᮪ (Impl == nullptr)
    ᮝᮜᮤ;
  Impl->endInterval(O, Name);
#endif // if !HAVE_ANY_SIGNPOST_IMPL
}
