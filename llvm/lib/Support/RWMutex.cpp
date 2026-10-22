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

#شامل "llvm/Support/RWMutex.h"
#شامل "llvm/Config/config.h"
#شامل "llvm/Config/llvm-config.h" // for LLVM_ENABLE_THREADS
#شامل "llvm/Support/Allocator.h"

#اگر defined(LLVM_USE_RW_MUTEX_IMPL)
using فضاینام llvm;
using فضاینام sys;

#اگر !defined(LLVM_ENABLE_THREADS) || LLVM_ENABLE_THREADS == 0
// Define all methods as no-ops if threading is explicitly disabled

RWMutexImpl::RWMutexImpl() = پیشفرض;
RWMutexImpl::~RWMutexImpl() = پیشفرض;

منطقی RWMutexImpl::lock_shared() { بازگشت درست; }
منطقی RWMutexImpl::unlock_shared() { بازگشت درست; }
منطقی RWMutexImpl::try_lock_shared() { بازگشت درست; }
منطقی RWMutexImpl::lock() { بازگشت درست; }
منطقی RWMutexImpl::unlock() { بازگشت درست; }
منطقی RWMutexImpl::try_lock() { بازگشت درست; }

#وگرنه

#اگر defined(HAVE_PTHREAD_H) && defined(HAVE_PTHREAD_RWLOCK_INIT)

#شامل <cassert>
#شامل <cstdlib>
#شامل <pthread.h>

// Construct a RWMutex using pthread calls
RWMutexImpl::RWMutexImpl()
{
  // Declare the pthread_rwlock data structures
  pthread_rwlock_t* rwlock =
    static_cast<pthread_rwlock_t*>(safe_malloc(اندازه(pthread_rwlock_t)));

#ifdef __APPLE__
  // Workaround a bug/mis-feature in Darwin's pthread_rwlock_init.
  bzero(rwlock, اندازه(pthread_rwlock_t));
#endif

  // Initialize the rwlock
  صحیح errorcode = pthread_rwlock_init(rwlock, nullptr);
  (تهی)errorcode;
  أكد(errorcode == 0);

  // Assign the data member
  data_ = rwlock;
}

// Destruct a RWMutex
RWMutexImpl::~RWMutexImpl()
{
  pthread_rwlock_t* rwlock = static_cast<pthread_rwlock_t*>(data_);
  أكد(rwlock != nullptr);
  pthread_rwlock_destroy(rwlock);
  آزاد(rwlock);
}

منطقی
RWMutexImpl::lock_shared()
{
  pthread_rwlock_t* rwlock = static_cast<pthread_rwlock_t*>(data_);
  أكد(rwlock != nullptr);

  صحیح errorcode = pthread_rwlock_rdlock(rwlock);
  بازگشت errorcode == 0;
}

منطقی
RWMutexImpl::unlock_shared()
{
  pthread_rwlock_t* rwlock = static_cast<pthread_rwlock_t*>(data_);
  أكد(rwlock != nullptr);

  صحیح errorcode = pthread_rwlock_unlock(rwlock);
  بازگشت errorcode == 0;
}

منطقی RWMutexImpl::try_lock_shared() {
  pthread_rwlock_t *rwlock = static_cast<pthread_rwlock_t *>(data_);
  أكد(rwlock != nullptr);

  صحیح errorcode = pthread_rwlock_tryrdlock(rwlock);
  بازگشت errorcode == 0;
}

منطقی
RWMutexImpl::lock()
{
  pthread_rwlock_t* rwlock = static_cast<pthread_rwlock_t*>(data_);
  أكد(rwlock != nullptr);

  صحیح errorcode = pthread_rwlock_wrlock(rwlock);
  بازگشت errorcode == 0;
}

منطقی
RWMutexImpl::unlock()
{
  pthread_rwlock_t* rwlock = static_cast<pthread_rwlock_t*>(data_);
  أكد(rwlock != nullptr);

  صحیح errorcode = pthread_rwlock_unlock(rwlock);
  بازگشت errorcode == 0;
}

منطقی RWMutexImpl::try_lock() {
  pthread_rwlock_t *rwlock = static_cast<pthread_rwlock_t *>(data_);
  أكد(rwlock != nullptr);

  صحیح errorcode = pthread_rwlock_trywrlock(rwlock);
  بازگشت errorcode == 0;
}

#وگرنه

RWMutexImpl::RWMutexImpl() : data_(جدید MutexImpl(نادرست)) { }

RWMutexImpl::~RWMutexImpl() {
  delete static_cast<MutexImpl *>(data_);
}

منطقی RWMutexImpl::lock_shared() {
  بازگشت static_cast<MutexImpl *>(data_)->acquire();
}

منطقی RWMutexImpl::unlock_shared() {
  بازگشت static_cast<MutexImpl *>(data_)->release();
}

منطقی RWMutexImpl::try_lock_shared() {
  بازگشت static_cast<MutexImpl *>(data_)->tryacquire();
}

منطقی RWMutexImpl::lock() {
  بازگشت static_cast<MutexImpl *>(data_)->acquire();
}

منطقی RWMutexImpl::unlock() {
  بازگشت static_cast<MutexImpl *>(data_)->release();
}

منطقی RWMutexImpl::try_lock() {
  بازگشت static_cast<MutexImpl *>(data_)->tryacquire();
}

#endif
#endif
#endif
