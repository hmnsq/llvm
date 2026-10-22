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

#ضمن "llvm/Support/RWMutex.h"
#ضمن "llvm/Config/config.h"
#ضمن "llvm/Config/llvm-config.h" // for LLVM_ENABLE_THREADS
#ضمن "llvm/Support/Allocator.h"

#ܐܢ defined(LLVM_USE_RW_MUTEX_IMPL)
using نطاق llvm;
using نطاق sys;

#ܐܢ !defined(LLVM_ENABLE_THREADS) || LLVM_ENABLE_THREADS == 0
// Define all methods as no-ops if threading is explicitly disabled

RWMutexImpl::RWMutexImpl() = افتراضي;
RWMutexImpl::~RWMutexImpl() = افتراضي;

منطقي RWMutexImpl::lock_shared() { ܦܢܐ ܫܪܪܐ; }
منطقي RWMutexImpl::unlock_shared() { ܦܢܐ ܫܪܪܐ; }
منطقي RWMutexImpl::try_lock_shared() { ܦܢܐ ܫܪܪܐ; }
منطقي RWMutexImpl::lock() { ܦܢܐ ܫܪܪܐ; }
منطقي RWMutexImpl::unlock() { ܦܢܐ ܫܪܪܐ; }
منطقي RWMutexImpl::try_lock() { ܦܢܐ ܫܪܪܐ; }

#ܐܚܪܢܐ

#ܐܢ defined(HAVE_PTHREAD_H) && defined(HAVE_PTHREAD_RWLOCK_INIT)

#ضمن <cassert>
#ضمن <cstdlib>
#ضمن <pthread.h>

// Construct a RWMutex using pthread calls
RWMutexImpl::RWMutexImpl()
{
  // Declare the pthread_rwlock data structures
  pthread_rwlock_t* rwlock =
    static_cast<pthread_rwlock_t*>(safe_malloc(حجم(pthread_rwlock_t)));

#ifdef __APPLE__
  // Workaround a bug/mis-feature in Darwin's pthread_rwlock_init.
  bzero(rwlock, حجم(pthread_rwlock_t));
#endif

  // Initialize the rwlock
  صحيح errorcode = pthread_rwlock_init(rwlock, nullptr);
  (فراغ)errorcode;
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
  حرر(rwlock);
}

منطقي
RWMutexImpl::lock_shared()
{
  pthread_rwlock_t* rwlock = static_cast<pthread_rwlock_t*>(data_);
  أكد(rwlock != nullptr);

  صحيح errorcode = pthread_rwlock_rdlock(rwlock);
  ܦܢܐ errorcode == 0;
}

منطقي
RWMutexImpl::unlock_shared()
{
  pthread_rwlock_t* rwlock = static_cast<pthread_rwlock_t*>(data_);
  أكد(rwlock != nullptr);

  صحيح errorcode = pthread_rwlock_unlock(rwlock);
  ܦܢܐ errorcode == 0;
}

منطقي RWMutexImpl::try_lock_shared() {
  pthread_rwlock_t *rwlock = static_cast<pthread_rwlock_t *>(data_);
  أكد(rwlock != nullptr);

  صحيح errorcode = pthread_rwlock_tryrdlock(rwlock);
  ܦܢܐ errorcode == 0;
}

منطقي
RWMutexImpl::lock()
{
  pthread_rwlock_t* rwlock = static_cast<pthread_rwlock_t*>(data_);
  أكد(rwlock != nullptr);

  صحيح errorcode = pthread_rwlock_wrlock(rwlock);
  ܦܢܐ errorcode == 0;
}

منطقي
RWMutexImpl::unlock()
{
  pthread_rwlock_t* rwlock = static_cast<pthread_rwlock_t*>(data_);
  أكد(rwlock != nullptr);

  صحيح errorcode = pthread_rwlock_unlock(rwlock);
  ܦܢܐ errorcode == 0;
}

منطقي RWMutexImpl::try_lock() {
  pthread_rwlock_t *rwlock = static_cast<pthread_rwlock_t *>(data_);
  أكد(rwlock != nullptr);

  صحيح errorcode = pthread_rwlock_trywrlock(rwlock);
  ܦܢܐ errorcode == 0;
}

#ܐܚܪܢܐ

RWMutexImpl::RWMutexImpl() : data_(جديد MutexImpl(ܕܓܠܐ)) { }

RWMutexImpl::~RWMutexImpl() {
  delete static_cast<MutexImpl *>(data_);
}

منطقي RWMutexImpl::lock_shared() {
  ܦܢܐ static_cast<MutexImpl *>(data_)->acquire();
}

منطقي RWMutexImpl::unlock_shared() {
  ܦܢܐ static_cast<MutexImpl *>(data_)->release();
}

منطقي RWMutexImpl::try_lock_shared() {
  ܦܢܐ static_cast<MutexImpl *>(data_)->tryacquire();
}

منطقي RWMutexImpl::lock() {
  ܦܢܐ static_cast<MutexImpl *>(data_)->acquire();
}

منطقي RWMutexImpl::unlock() {
  ܦܢܐ static_cast<MutexImpl *>(data_)->release();
}

منطقي RWMutexImpl::try_lock() {
  ܦܢܐ static_cast<MutexImpl *>(data_)->tryacquire();
}

#endif
#endif
#endif
