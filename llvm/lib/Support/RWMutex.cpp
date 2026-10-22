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

#અન્તર્ભાવ "llvm/Support/RWMutex.h"
#અન્તર્ભાવ "llvm/Config/config.h"
#અન્તર્ભાવ "llvm/Config/llvm-config.h" // for LLVM_ENABLE_THREADS
#અન્તર્ભાવ "llvm/Support/Allocator.h"

#જો defined(LLVM_USE_RW_MUTEX_IMPL)
using નામાકાશ llvm;
using નામાકાશ sys;

#જો !defined(LLVM_ENABLE_THREADS) || LLVM_ENABLE_THREADS == 0
// Define all methods as no-ops if threading is explicitly disabled

RWMutexImpl::RWMutexImpl() = મૂળભૂત;
RWMutexImpl::~RWMutexImpl() = મૂળભૂત;

તાર્કિક RWMutexImpl::lock_shared() { પાછા સાચું; }
તાર્કિક RWMutexImpl::unlock_shared() { પાછા સાચું; }
તાર્કિક RWMutexImpl::try_lock_shared() { પાછા સાચું; }
તાર્કિક RWMutexImpl::lock() { પાછા સાચું; }
તાર્કિક RWMutexImpl::unlock() { પાછા સાચું; }
તાર્કિક RWMutexImpl::try_lock() { પાછા સાચું; }

#નહીંતર

#જો defined(HAVE_PTHREAD_H) && defined(HAVE_PTHREAD_RWLOCK_INIT)

#અન્તર્ભાવ <cassert>
#અન્તર્ભાવ <cstdlib>
#અન્તર્ભાવ <pthread.h>

// Construct a RWMutex using pthread calls
RWMutexImpl::RWMutexImpl()
{
  // Declare the pthread_rwlock data structures
  pthread_rwlock_t* rwlock =
    static_cast<pthread_rwlock_t*>(safe_malloc(કદ(pthread_rwlock_t)));

#ifdef __APPLE__
  // Workaround a bug/mis-feature in Darwin's pthread_rwlock_init.
  bzero(rwlock, કદ(pthread_rwlock_t));
#endif

  // Initialize the rwlock
  પૂર્ણાંક errorcode = pthread_rwlock_init(rwlock, nullptr);
  (શૂન્ય)errorcode;
  પ્રતિજ્ઞા(errorcode == 0);

  // Assign the data member
  data_ = rwlock;
}

// Destruct a RWMutex
RWMutexImpl::~RWMutexImpl()
{
  pthread_rwlock_t* rwlock = static_cast<pthread_rwlock_t*>(data_);
  પ્રતિજ્ઞા(rwlock != nullptr);
  pthread_rwlock_destroy(rwlock);
  મુક્ત(rwlock);
}

તાર્કિક
RWMutexImpl::lock_shared()
{
  pthread_rwlock_t* rwlock = static_cast<pthread_rwlock_t*>(data_);
  પ્રતિજ્ઞા(rwlock != nullptr);

  પૂર્ણાંક errorcode = pthread_rwlock_rdlock(rwlock);
  પાછા errorcode == 0;
}

તાર્કિક
RWMutexImpl::unlock_shared()
{
  pthread_rwlock_t* rwlock = static_cast<pthread_rwlock_t*>(data_);
  પ્રતિજ્ઞા(rwlock != nullptr);

  પૂર્ણાંક errorcode = pthread_rwlock_unlock(rwlock);
  પાછા errorcode == 0;
}

તાર્કિક RWMutexImpl::try_lock_shared() {
  pthread_rwlock_t *rwlock = static_cast<pthread_rwlock_t *>(data_);
  પ્રતિજ્ઞા(rwlock != nullptr);

  પૂર્ણાંક errorcode = pthread_rwlock_tryrdlock(rwlock);
  પાછા errorcode == 0;
}

તાર્કિક
RWMutexImpl::lock()
{
  pthread_rwlock_t* rwlock = static_cast<pthread_rwlock_t*>(data_);
  પ્રતિજ્ઞા(rwlock != nullptr);

  પૂર્ણાંક errorcode = pthread_rwlock_wrlock(rwlock);
  પાછા errorcode == 0;
}

તાર્કિક
RWMutexImpl::unlock()
{
  pthread_rwlock_t* rwlock = static_cast<pthread_rwlock_t*>(data_);
  પ્રતિજ્ઞા(rwlock != nullptr);

  પૂર્ણાંક errorcode = pthread_rwlock_unlock(rwlock);
  પાછા errorcode == 0;
}

તાર્કિક RWMutexImpl::try_lock() {
  pthread_rwlock_t *rwlock = static_cast<pthread_rwlock_t *>(data_);
  પ્રતિજ્ઞા(rwlock != nullptr);

  પૂર્ણાંક errorcode = pthread_rwlock_trywrlock(rwlock);
  પાછા errorcode == 0;
}

#નહીંતર

RWMutexImpl::RWMutexImpl() : data_(નવું MutexImpl(ખોટું)) { }

RWMutexImpl::~RWMutexImpl() {
  delete static_cast<MutexImpl *>(data_);
}

તાર્કિક RWMutexImpl::lock_shared() {
  પાછા static_cast<MutexImpl *>(data_)->acquire();
}

તાર્કિક RWMutexImpl::unlock_shared() {
  પાછા static_cast<MutexImpl *>(data_)->release();
}

તાર્કિક RWMutexImpl::try_lock_shared() {
  પાછા static_cast<MutexImpl *>(data_)->tryacquire();
}

તાર્કિક RWMutexImpl::lock() {
  પાછા static_cast<MutexImpl *>(data_)->acquire();
}

તાર્કિક RWMutexImpl::unlock() {
  પાછા static_cast<MutexImpl *>(data_)->release();
}

તાર્કિક RWMutexImpl::try_lock() {
  પાછા static_cast<MutexImpl *>(data_)->tryacquire();
}

#endif
#endif
#endif
