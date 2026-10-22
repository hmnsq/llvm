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

RWMutexImpl::RWMutexImpl() = স্বভাব;
RWMutexImpl::~RWMutexImpl() = স্বভাব;

তার্কিক RWMutexImpl::lock_shared() { প্রত্যাগম সত্য; }
তার্কিক RWMutexImpl::unlock_shared() { প্রত্যাগম সত্য; }
তার্কিক RWMutexImpl::try_lock_shared() { প্রত্যাগম সত্য; }
তার্কিক RWMutexImpl::lock() { প্রত্যাগম সত্য; }
তার্কিক RWMutexImpl::unlock() { প্রত্যাগম সত্য; }
তার্কিক RWMutexImpl::try_lock() { প্রত্যাগম সত্য; }

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
    static_cast<pthread_rwlock_t*>(safe_malloc(পরিমাণ(pthread_rwlock_t)));

#ifdef __APPLE__
  // Workaround a bug/mis-feature in Darwin's pthread_rwlock_init.
  bzero(rwlock, পরিমাণ(pthread_rwlock_t));
#endif

  // Initialize the rwlock
  পূর্ণাঙ্ক errorcode = pthread_rwlock_init(rwlock, nullptr);
  (শূন্য)errorcode;
  প্রতিজ্ঞা(errorcode == 0);

  // Assign the data member
  data_ = rwlock;
}

// Destruct a RWMutex
RWMutexImpl::~RWMutexImpl()
{
  pthread_rwlock_t* rwlock = static_cast<pthread_rwlock_t*>(data_);
  প্রতিজ্ঞা(rwlock != nullptr);
  pthread_rwlock_destroy(rwlock);
  মুক্ত(rwlock);
}

তার্কিক
RWMutexImpl::lock_shared()
{
  pthread_rwlock_t* rwlock = static_cast<pthread_rwlock_t*>(data_);
  প্রতিজ্ঞা(rwlock != nullptr);

  পূর্ণাঙ্ক errorcode = pthread_rwlock_rdlock(rwlock);
  প্রত্যাগম errorcode == 0;
}

তার্কিক
RWMutexImpl::unlock_shared()
{
  pthread_rwlock_t* rwlock = static_cast<pthread_rwlock_t*>(data_);
  প্রতিজ্ঞা(rwlock != nullptr);

  পূর্ণাঙ্ক errorcode = pthread_rwlock_unlock(rwlock);
  প্রত্যাগম errorcode == 0;
}

তার্কিক RWMutexImpl::try_lock_shared() {
  pthread_rwlock_t *rwlock = static_cast<pthread_rwlock_t *>(data_);
  প্রতিজ্ঞা(rwlock != nullptr);

  পূর্ণাঙ্ক errorcode = pthread_rwlock_tryrdlock(rwlock);
  প্রত্যাগম errorcode == 0;
}

তার্কিক
RWMutexImpl::lock()
{
  pthread_rwlock_t* rwlock = static_cast<pthread_rwlock_t*>(data_);
  প্রতিজ্ঞা(rwlock != nullptr);

  পূর্ণাঙ্ক errorcode = pthread_rwlock_wrlock(rwlock);
  প্রত্যাগম errorcode == 0;
}

তার্কিক
RWMutexImpl::unlock()
{
  pthread_rwlock_t* rwlock = static_cast<pthread_rwlock_t*>(data_);
  প্রতিজ্ঞা(rwlock != nullptr);

  পূর্ণাঙ্ক errorcode = pthread_rwlock_unlock(rwlock);
  প্রত্যাগম errorcode == 0;
}

তার্কিক RWMutexImpl::try_lock() {
  pthread_rwlock_t *rwlock = static_cast<pthread_rwlock_t *>(data_);
  প্রতিজ্ঞা(rwlock != nullptr);

  পূর্ণাঙ্ক errorcode = pthread_rwlock_trywrlock(rwlock);
  প্রত্যাগম errorcode == 0;
}

#অন্যথা

RWMutexImpl::RWMutexImpl() : data_(নূতন MutexImpl(অসত্য)) { }

RWMutexImpl::~RWMutexImpl() {
  delete static_cast<MutexImpl *>(data_);
}

তার্কিক RWMutexImpl::lock_shared() {
  প্রত্যাগম static_cast<MutexImpl *>(data_)->acquire();
}

তার্কিক RWMutexImpl::unlock_shared() {
  প্রত্যাগম static_cast<MutexImpl *>(data_)->release();
}

তার্কিক RWMutexImpl::try_lock_shared() {
  প্রত্যাগম static_cast<MutexImpl *>(data_)->tryacquire();
}

তার্কিক RWMutexImpl::lock() {
  প্রত্যাগম static_cast<MutexImpl *>(data_)->acquire();
}

তার্কিক RWMutexImpl::unlock() {
  প্রত্যাগম static_cast<MutexImpl *>(data_)->release();
}

তার্কিক RWMutexImpl::try_lock() {
  প্রত্যাগম static_cast<MutexImpl *>(data_)->tryacquire();
}

#endif
#endif
#endif
