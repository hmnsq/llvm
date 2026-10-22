//===- RWMutex.cpp - Reader/Writer Mutual Exclusion Lock --------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file implements the llvm::sys::RWMutex class.
//
//===----------------------------------------------------------------------===//

#ಅನ್ತರ್ಭಾವ "llvm/Support/RWMutex.h"
#ಅನ್ತರ್ಭಾವ "llvm/Config/config.h"
#ಅನ್ತರ್ಭಾವ "llvm/Config/llvm-config.h" // for LLVM_ENABLE_THREADS
#ಅನ್ತರ್ಭಾವ "llvm/Support/Allocator.h"

#ಯದಿ defined(LLVM_USE_RW_MUTEX_IMPL)
using ನಾಮಾಕಾಶ llvm;
using ನಾಮಾಕಾಶ sys;

#ಯದಿ !defined(LLVM_ENABLE_THREADS) || LLVM_ENABLE_THREADS == 0
// Define all methods as no-ops if threading is explicitly disabled

RWMutexImpl::RWMutexImpl() = ಸ್ವಭಾವ;
RWMutexImpl::~RWMutexImpl() = ಸ್ವಭಾವ;

ತಾರ್ಕಿಕ RWMutexImpl::lock_shared() { ಪ್ರತ್ಯಾಗಮ ಸತ್ಯ; }
ತಾರ್ಕಿಕ RWMutexImpl::unlock_shared() { ಪ್ರತ್ಯಾಗಮ ಸತ್ಯ; }
ತಾರ್ಕಿಕ RWMutexImpl::try_lock_shared() { ಪ್ರತ್ಯಾಗಮ ಸತ್ಯ; }
ತಾರ್ಕಿಕ RWMutexImpl::lock() { ಪ್ರತ್ಯಾಗಮ ಸತ್ಯ; }
ತಾರ್ಕಿಕ RWMutexImpl::unlock() { ಪ್ರತ್ಯಾಗಮ ಸತ್ಯ; }
ತಾರ್ಕಿಕ RWMutexImpl::try_lock() { ಪ್ರತ್ಯಾಗಮ ಸತ್ಯ; }

#ಅನ್ಯಥಾ

#ಯದಿ defined(HAVE_PTHREAD_H) && defined(HAVE_PTHREAD_RWLOCK_INIT)

#ಅನ್ತರ್ಭಾವ <cassert>
#ಅನ್ತರ್ಭಾವ <cstdlib>
#ಅನ್ತರ್ಭಾವ <pthread.h>

// Construct a RWMutex using pthread calls
RWMutexImpl::RWMutexImpl()
{
  // Declare the pthread_rwlock data structures
  pthread_rwlock_t* rwlock =
    static_cast<pthread_rwlock_t*>(safe_malloc(ಪರಿಮಾಣ(pthread_rwlock_t)));

#ifdef __APPLE__
  // Workaround a bug/mis-feature in Darwin's pthread_rwlock_init.
  bzero(rwlock, ಪರಿಮಾಣ(pthread_rwlock_t));
#endif

  // Initialize the rwlock
  ಪೂರ್ಣಾಙ್ಕ errorcode = pthread_rwlock_init(rwlock, nullptr);
  (ಶೂನ್ಯ)errorcode;
  ಪ್ರತಿಜ್ಞಾ(errorcode == 0);

  // Assign the data member
  data_ = rwlock;
}

// Destruct a RWMutex
RWMutexImpl::~RWMutexImpl()
{
  pthread_rwlock_t* rwlock = static_cast<pthread_rwlock_t*>(data_);
  ಪ್ರತಿಜ್ಞಾ(rwlock != nullptr);
  pthread_rwlock_destroy(rwlock);
  ಮುಕ್ತ(rwlock);
}

ತಾರ್ಕಿಕ
RWMutexImpl::lock_shared()
{
  pthread_rwlock_t* rwlock = static_cast<pthread_rwlock_t*>(data_);
  ಪ್ರತಿಜ್ಞಾ(rwlock != nullptr);

  ಪೂರ್ಣಾಙ್ಕ errorcode = pthread_rwlock_rdlock(rwlock);
  ಪ್ರತ್ಯಾಗಮ errorcode == 0;
}

ತಾರ್ಕಿಕ
RWMutexImpl::unlock_shared()
{
  pthread_rwlock_t* rwlock = static_cast<pthread_rwlock_t*>(data_);
  ಪ್ರತಿಜ್ಞಾ(rwlock != nullptr);

  ಪೂರ್ಣಾಙ್ಕ errorcode = pthread_rwlock_unlock(rwlock);
  ಪ್ರತ್ಯಾಗಮ errorcode == 0;
}

ತಾರ್ಕಿಕ RWMutexImpl::try_lock_shared() {
  pthread_rwlock_t *rwlock = static_cast<pthread_rwlock_t *>(data_);
  ಪ್ರತಿಜ್ಞಾ(rwlock != nullptr);

  ಪೂರ್ಣಾಙ್ಕ errorcode = pthread_rwlock_tryrdlock(rwlock);
  ಪ್ರತ್ಯಾಗಮ errorcode == 0;
}

ತಾರ್ಕಿಕ
RWMutexImpl::lock()
{
  pthread_rwlock_t* rwlock = static_cast<pthread_rwlock_t*>(data_);
  ಪ್ರತಿಜ್ಞಾ(rwlock != nullptr);

  ಪೂರ್ಣಾಙ್ಕ errorcode = pthread_rwlock_wrlock(rwlock);
  ಪ್ರತ್ಯಾಗಮ errorcode == 0;
}

ತಾರ್ಕಿಕ
RWMutexImpl::unlock()
{
  pthread_rwlock_t* rwlock = static_cast<pthread_rwlock_t*>(data_);
  ಪ್ರತಿಜ್ಞಾ(rwlock != nullptr);

  ಪೂರ್ಣಾಙ್ಕ errorcode = pthread_rwlock_unlock(rwlock);
  ಪ್ರತ್ಯಾಗಮ errorcode == 0;
}

ತಾರ್ಕಿಕ RWMutexImpl::try_lock() {
  pthread_rwlock_t *rwlock = static_cast<pthread_rwlock_t *>(data_);
  ಪ್ರತಿಜ್ಞಾ(rwlock != nullptr);

  ಪೂರ್ಣಾಙ್ಕ errorcode = pthread_rwlock_trywrlock(rwlock);
  ಪ್ರತ್ಯಾಗಮ errorcode == 0;
}

#ಅನ್ಯಥಾ

RWMutexImpl::RWMutexImpl() : data_(ನೂತನ MutexImpl(ಅಸತ್ಯ)) { }

RWMutexImpl::~RWMutexImpl() {
  delete static_cast<MutexImpl *>(data_);
}

ತಾರ್ಕಿಕ RWMutexImpl::lock_shared() {
  ಪ್ರತ್ಯಾಗಮ static_cast<MutexImpl *>(data_)->acquire();
}

ತಾರ್ಕಿಕ RWMutexImpl::unlock_shared() {
  ಪ್ರತ್ಯಾಗಮ static_cast<MutexImpl *>(data_)->release();
}

ತಾರ್ಕಿಕ RWMutexImpl::try_lock_shared() {
  ಪ್ರತ್ಯಾಗಮ static_cast<MutexImpl *>(data_)->tryacquire();
}

ತಾರ್ಕಿಕ RWMutexImpl::lock() {
  ಪ್ರತ್ಯಾಗಮ static_cast<MutexImpl *>(data_)->acquire();
}

ತಾರ್ಕಿಕ RWMutexImpl::unlock() {
  ಪ್ರತ್ಯಾಗಮ static_cast<MutexImpl *>(data_)->release();
}

ತಾರ್ಕಿಕ RWMutexImpl::try_lock() {
  ಪ್ರತ್ಯಾಗಮ static_cast<MutexImpl *>(data_)->tryacquire();
}

#endif
#endif
#endif
