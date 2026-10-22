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
استعمال نامخلا llvm;
استعمال نامخلا sys;

#اگر !defined(LLVM_ENABLE_دھاگے) || LLVM_ENABLE_دھاگے == 0
// Define all methods as no-ops if threading is explicitly disabled

RWMutexImpl::RWMutexImpl() = طےشدہ;
RWMutexImpl::~RWMutexImpl() = طےشدہ;

منطقی RWMutexImpl::تالا_shared() { واپس سچ; }
منطقی RWMutexImpl::unlock_shared() { واپس سچ; }
منطقی RWMutexImpl::try_تالا_shared() { واپس سچ; }
منطقی RWMutexImpl::تالا_2() { واپس سچ; }
منطقی RWMutexImpl::unlock() { واپس سچ; }
منطقی RWMutexImpl::try_تالا_2() { واپس سچ; }

#ورنہ

#اگر defined(HAVE_PTHREAD_H) && defined(HAVE_PTHREAD_RWLOCK_INIT)

#شامل <cassert>
#شامل <cstdlib>
#شامل <pthread.h>

// Construct a RWMutex using pthread calls
RWMutexImpl::RWMutexImpl()
{
  // Declare the pthread_rwlock data structures
  pthread_rwlock_t* rwlock =
    static_cast<pthread_rwlock_t*>(محفوظ_malloc(حجم(pthread_rwlock_t)));

#اگر_تعریف_شدہ __APPLE__
  // Workaround a bug/mis-feature in Darwin's pthread_rwlock_init.
  bzero(rwlock, حجم(pthread_rwlock_t));
#اگر_ختم

  // Initialize the rwlock
  صحیح errorcode = pthread_rwlock_init(rwlock, خالی_اشاریہ);
  (خالی)errorcode;
  أكد(errorcode == 0);

  // Assign the data member
  ڈیٹا_3 = rwlock;
}

// Destruct a RWMutex
RWMutexImpl::~RWMutexImpl()
{
  pthread_rwlock_t* rwlock = static_cast<pthread_rwlock_t*>(ڈیٹا_3);
  أكد(rwlock != خالی_اشاریہ);
  pthread_rwlock_destroy(rwlock);
  آزاد(rwlock);
}

منطقی
RWMutexImpl::تالا_shared()
{
  pthread_rwlock_t* rwlock = static_cast<pthread_rwlock_t*>(ڈیٹا_3);
  أكد(rwlock != خالی_اشاریہ);

  صحیح errorcode = pthread_rwlock_rdlock(rwlock);
  واپس errorcode == 0;
}

منطقی
RWMutexImpl::unlock_shared()
{
  pthread_rwlock_t* rwlock = static_cast<pthread_rwlock_t*>(ڈیٹا_3);
  أكد(rwlock != خالی_اشاریہ);

  صحیح errorcode = pthread_rwlock_unlock(rwlock);
  واپس errorcode == 0;
}

منطقی RWMutexImpl::try_تالا_shared() {
  pthread_rwlock_t *rwlock = static_cast<pthread_rwlock_t *>(ڈیٹا_3);
  أكد(rwlock != خالی_اشاریہ);

  صحیح errorcode = pthread_rwlock_tryrdlock(rwlock);
  واپس errorcode == 0;
}

منطقی
RWMutexImpl::تالا_2()
{
  pthread_rwlock_t* rwlock = static_cast<pthread_rwlock_t*>(ڈیٹا_3);
  أكد(rwlock != خالی_اشاریہ);

  صحیح errorcode = pthread_rwlock_wrlock(rwlock);
  واپس errorcode == 0;
}

منطقی
RWMutexImpl::unlock()
{
  pthread_rwlock_t* rwlock = static_cast<pthread_rwlock_t*>(ڈیٹا_3);
  أكد(rwlock != خالی_اشاریہ);

  صحیح errorcode = pthread_rwlock_unlock(rwlock);
  واپس errorcode == 0;
}

منطقی RWMutexImpl::try_تالا_2() {
  pthread_rwlock_t *rwlock = static_cast<pthread_rwlock_t *>(ڈیٹا_3);
  أكد(rwlock != خالی_اشاریہ);

  صحیح errorcode = pthread_rwlock_trywrlock(rwlock);
  واپس errorcode == 0;
}

#ورنہ

RWMutexImpl::RWMutexImpl() : ڈیٹا_3(نیا MutexImpl(جھوٹ)) { }

RWMutexImpl::~RWMutexImpl() {
  حذف static_cast<MutexImpl *>(ڈیٹا_3);
}

منطقی RWMutexImpl::تالا_shared() {
  واپس static_cast<MutexImpl *>(ڈیٹا_3)->acquire();
}

منطقی RWMutexImpl::unlock_shared() {
  واپس static_cast<MutexImpl *>(ڈیٹا_3)->release();
}

منطقی RWMutexImpl::try_تالا_shared() {
  واپس static_cast<MutexImpl *>(ڈیٹا_3)->tryacquire();
}

منطقی RWMutexImpl::تالا_2() {
  واپس static_cast<MutexImpl *>(ڈیٹا_3)->acquire();
}

منطقی RWMutexImpl::unlock() {
  واپس static_cast<MutexImpl *>(ڈیٹا_3)->release();
}

منطقی RWMutexImpl::try_تالا_2() {
  واپس static_cast<MutexImpl *>(ڈیٹا_3)->tryacquire();
}

#اگر_ختم
#اگر_ختم
#اگر_ختم
