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

#إذا defined(LLVM_USE_RW_MUTEX_IMPL)
ورتوں نطاق llvm;
ورتوں نطاق sys;

#إذا !defined(LLVM_ENABLE_دھاگے) || LLVM_ENABLE_دھاگے == 0
// Define all methods as no-ops if threading is explicitly disabled

RWMutexImpl::RWMutexImpl() = افتراضي;
RWMutexImpl::~RWMutexImpl() = افتراضي;

منطقي RWMutexImpl::جندرا_shared() { أرجع سچ; }
منطقي RWMutexImpl::unlock_shared() { أرجع سچ; }
منطقي RWMutexImpl::try_جندرا_shared() { أرجع سچ; }
منطقي RWMutexImpl::جندرا() { أرجع سچ; }
منطقي RWMutexImpl::unlock() { أرجع سچ; }
منطقي RWMutexImpl::try_جندرا_2() { أرجع سچ; }

#وإلا

#إذا defined(HAVE_PTHREAD_H) && defined(HAVE_PTHREAD_RWLOCK_INIT)

#ضمن <cassert>
#ضمن <cstdlib>
#ضمن <pthread.h>

// Construct a RWMutex using pthread calls
RWMutexImpl::RWMutexImpl()
{
  // Declare the pthread_rwlock data structures
  pthread_rwlock_t* rwlock =
    static_cast<pthread_rwlock_t*>(محفوظ_malloc(حجم(pthread_rwlock_t)));

#جے_تعریف_شدہ __APPLE__
  // Workaround a bug/mis-feature in Darwin's pthread_rwlock_init.
  bzero(rwlock, حجم(pthread_rwlock_t));
#جے_انت

  // Initialize the rwlock
  صحيح errorcode = pthread_rwlock_init(rwlock, خالی_سوچک);
  (فراغ)errorcode;
  أكد(errorcode == 0);

  // Assign the data member
  انکڑے_3 = rwlock;
}

// Destruct a RWMutex
RWMutexImpl::~RWMutexImpl()
{
  pthread_rwlock_t* rwlock = static_cast<pthread_rwlock_t*>(انکڑے_3);
  أكد(rwlock != خالی_سوچک);
  pthread_rwlock_destroy(rwlock);
  حرر(rwlock);
}

منطقي
RWMutexImpl::جندرا_shared()
{
  pthread_rwlock_t* rwlock = static_cast<pthread_rwlock_t*>(انکڑے_3);
  أكد(rwlock != خالی_سوچک);

  صحيح errorcode = pthread_rwlock_rdlock(rwlock);
  أرجع errorcode == 0;
}

منطقي
RWMutexImpl::unlock_shared()
{
  pthread_rwlock_t* rwlock = static_cast<pthread_rwlock_t*>(انکڑے_3);
  أكد(rwlock != خالی_سوچک);

  صحيح errorcode = pthread_rwlock_unlock(rwlock);
  أرجع errorcode == 0;
}

منطقي RWMutexImpl::try_جندرا_shared() {
  pthread_rwlock_t *rwlock = static_cast<pthread_rwlock_t *>(انکڑے_3);
  أكد(rwlock != خالی_سوچک);

  صحيح errorcode = pthread_rwlock_tryrdlock(rwlock);
  أرجع errorcode == 0;
}

منطقي
RWMutexImpl::جندرا()
{
  pthread_rwlock_t* rwlock = static_cast<pthread_rwlock_t*>(انکڑے_3);
  أكد(rwlock != خالی_سوچک);

  صحيح errorcode = pthread_rwlock_wrlock(rwlock);
  أرجع errorcode == 0;
}

منطقي
RWMutexImpl::unlock()
{
  pthread_rwlock_t* rwlock = static_cast<pthread_rwlock_t*>(انکڑے_3);
  أكد(rwlock != خالی_سوچک);

  صحيح errorcode = pthread_rwlock_unlock(rwlock);
  أرجع errorcode == 0;
}

منطقي RWMutexImpl::try_جندرا_2() {
  pthread_rwlock_t *rwlock = static_cast<pthread_rwlock_t *>(انکڑے_3);
  أكد(rwlock != خالی_سوچک);

  صحيح errorcode = pthread_rwlock_trywrlock(rwlock);
  أرجع errorcode == 0;
}

#وإلا

RWMutexImpl::RWMutexImpl() : انکڑے_3(جديد MutexImpl(خطأ)) { }

RWMutexImpl::~RWMutexImpl() {
  مٹاؤ static_cast<MutexImpl *>(انکڑے_3);
}

منطقي RWMutexImpl::جندرا_shared() {
  أرجع static_cast<MutexImpl *>(انکڑے_3)->acquire();
}

منطقي RWMutexImpl::unlock_shared() {
  أرجع static_cast<MutexImpl *>(انکڑے_3)->release();
}

منطقي RWMutexImpl::try_جندرا_shared() {
  أرجع static_cast<MutexImpl *>(انکڑے_3)->tryacquire();
}

منطقي RWMutexImpl::جندرا() {
  أرجع static_cast<MutexImpl *>(انکڑے_3)->acquire();
}

منطقي RWMutexImpl::unlock() {
  أرجع static_cast<MutexImpl *>(انکڑے_3)->release();
}

منطقي RWMutexImpl::try_جندرا_2() {
  أرجع static_cast<MutexImpl *>(انکڑے_3)->tryacquire();
}

#جے_انت
#جے_انت
#جے_انت
