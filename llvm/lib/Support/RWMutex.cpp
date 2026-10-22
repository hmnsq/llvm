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

#অন্তর্ভাব "llvm/Support/RWMutex.h"
#অন্তর্ভাব "llvm/Config/config.h"
#অন্তর্ভাব "llvm/Config/llvm-config.h" // for LLVM_ENABLE_THREADS
#অন্তর্ভাব "llvm/Support/Allocator.h"

#যদি defined(LLVM_USE_RW_MUTEX_IMPL)
using নামাকাশ llvm;
using নামাকাশ sys;

#যদি !defined(LLVM_ENABLE_THREADS) || LLVM_ENABLE_THREADS == 0
// Define all methods as no-ops if threading is explicitly disabled

RWMutexImpl::RWMutexImpl() = পূর্বনির্ধারিত;
RWMutexImpl::~RWMutexImpl() = পূর্বনির্ধারিত;

যৌক্তিক RWMutexImpl::lock_shared() { ফেরত সত্য; }
যৌক্তিক RWMutexImpl::unlock_shared() { ফেরত সত্য; }
যৌক্তিক RWMutexImpl::try_lock_shared() { ফেরত সত্য; }
যৌক্তিক RWMutexImpl::lock() { ফেরত সত্য; }
যৌক্তিক RWMutexImpl::unlock() { ফেরত সত্য; }
যৌক্তিক RWMutexImpl::try_lock() { ফেরত সত্য; }

#অন্যথা

#যদি defined(HAVE_PTHREAD_H) && defined(HAVE_PTHREAD_RWLOCK_INIT)

#অন্তর্ভাব <cassert>
#অন্তর্ভাব <cstdlib>
#অন্তর্ভাব <pthread.h>

// Construct a RWMutex using pthread calls
RWMutexImpl::RWMutexImpl()
{
  // Declare the pthread_rwlock data structures
  pthread_rwlock_t* rwlock =
    static_cast<pthread_rwlock_t*>(safe_malloc(আকার(pthread_rwlock_t)));

#ifdef __APPLE__
  // Workaround a bug/mis-feature in Darwin's pthread_rwlock_init.
  bzero(rwlock, আকার(pthread_rwlock_t));
#endif

  // Initialize the rwlock
  পূর্ণসংখ্যা errorcode = pthread_rwlock_init(rwlock, nullptr);
  (শূন্য)errorcode;
  প্রতিজ্ঞা(errorcode == 0);

  // Assign the data member
  তথ্য_3 = rwlock;
}

// Destruct a RWMutex
RWMutexImpl::~RWMutexImpl()
{
  pthread_rwlock_t* rwlock = static_cast<pthread_rwlock_t*>(তথ্য_3);
  প্রতিজ্ঞা(rwlock != nullptr);
  pthread_rwlock_destroy(rwlock);
  মুক্ত(rwlock);
}

যৌক্তিক
RWMutexImpl::lock_shared()
{
  pthread_rwlock_t* rwlock = static_cast<pthread_rwlock_t*>(তথ্য_3);
  প্রতিজ্ঞা(rwlock != nullptr);

  পূর্ণসংখ্যা errorcode = pthread_rwlock_rdlock(rwlock);
  ফেরত errorcode == 0;
}

যৌক্তিক
RWMutexImpl::unlock_shared()
{
  pthread_rwlock_t* rwlock = static_cast<pthread_rwlock_t*>(তথ্য_3);
  প্রতিজ্ঞা(rwlock != nullptr);

  পূর্ণসংখ্যা errorcode = pthread_rwlock_unlock(rwlock);
  ফেরত errorcode == 0;
}

যৌক্তিক RWMutexImpl::try_lock_shared() {
  pthread_rwlock_t *rwlock = static_cast<pthread_rwlock_t *>(তথ্য_3);
  প্রতিজ্ঞা(rwlock != nullptr);

  পূর্ণসংখ্যা errorcode = pthread_rwlock_tryrdlock(rwlock);
  ফেরত errorcode == 0;
}

যৌক্তিক
RWMutexImpl::lock()
{
  pthread_rwlock_t* rwlock = static_cast<pthread_rwlock_t*>(তথ্য_3);
  প্রতিজ্ঞা(rwlock != nullptr);

  পূর্ণসংখ্যা errorcode = pthread_rwlock_wrlock(rwlock);
  ফেরত errorcode == 0;
}

যৌক্তিক
RWMutexImpl::unlock()
{
  pthread_rwlock_t* rwlock = static_cast<pthread_rwlock_t*>(তথ্য_3);
  প্রতিজ্ঞা(rwlock != nullptr);

  পূর্ণসংখ্যা errorcode = pthread_rwlock_unlock(rwlock);
  ফেরত errorcode == 0;
}

যৌক্তিক RWMutexImpl::try_lock() {
  pthread_rwlock_t *rwlock = static_cast<pthread_rwlock_t *>(তথ্য_3);
  প্রতিজ্ঞা(rwlock != nullptr);

  পূর্ণসংখ্যা errorcode = pthread_rwlock_trywrlock(rwlock);
  ফেরত errorcode == 0;
}

#অন্যথা

RWMutexImpl::RWMutexImpl() : তথ্য_3(নতুন MutexImpl(মিথ্যা)) { }

RWMutexImpl::~RWMutexImpl() {
  delete static_cast<MutexImpl *>(তথ্য_3);
}

যৌক্তিক RWMutexImpl::lock_shared() {
  ফেরত static_cast<MutexImpl *>(তথ্য_3)->acquire();
}

যৌক্তিক RWMutexImpl::unlock_shared() {
  ফেরত static_cast<MutexImpl *>(তথ্য_3)->release();
}

যৌক্তিক RWMutexImpl::try_lock_shared() {
  ফেরত static_cast<MutexImpl *>(তথ্য_3)->tryacquire();
}

যৌক্তিক RWMutexImpl::lock() {
  ফেরত static_cast<MutexImpl *>(তথ্য_3)->acquire();
}

যৌক্তিক RWMutexImpl::unlock() {
  ফেরত static_cast<MutexImpl *>(তথ্য_3)->release();
}

যৌক্তিক RWMutexImpl::try_lock() {
  ফেরত static_cast<MutexImpl *>(তথ্য_3)->tryacquire();
}

#endif
#endif
#endif
