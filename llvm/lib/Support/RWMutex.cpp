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
استعمال نطاق llvm;
استعمال نطاق sys;

#إذا !defined(LLVM_ENABLE_ڌاڳا) || LLVM_ENABLE_ڌاڳا == 0
// Define all methods as no-ops if threading is explicitly disabled

RWMutexImpl::RWMutexImpl() = افتراضي;
RWMutexImpl::~RWMutexImpl() = افتراضي;

منطقي RWMutexImpl::تالو_shared() { أرجع سچ; }
منطقي RWMutexImpl::unlock_shared() { أرجع سچ; }
منطقي RWMutexImpl::try_تالو_shared() { أرجع سچ; }
منطقي RWMutexImpl::تالو() { أرجع سچ; }
منطقي RWMutexImpl::unlock() { أرجع سچ; }
منطقي RWMutexImpl::try_تالو_2() { أرجع سچ; }

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

#جيڪڏهن_وصف_ٿيل __APPLE__
  // Workaround a bug/mis-feature in Darwin's pthread_rwlock_init.
  bzero(rwlock, حجم(pthread_rwlock_t));
#جيڪڏهن_پورو

  // Initialize the rwlock
  صحيح errorcode = pthread_rwlock_init(rwlock, خالي_اشارو);
  (فراغ)errorcode;
  أكد(errorcode == 0);

  // Assign the data member
  معلومات_3 = rwlock;
}

// Destruct a RWMutex
RWMutexImpl::~RWMutexImpl()
{
  pthread_rwlock_t* rwlock = static_cast<pthread_rwlock_t*>(معلومات_3);
  أكد(rwlock != خالي_اشارو);
  pthread_rwlock_destroy(rwlock);
  حرر(rwlock);
}

منطقي
RWMutexImpl::تالو_shared()
{
  pthread_rwlock_t* rwlock = static_cast<pthread_rwlock_t*>(معلومات_3);
  أكد(rwlock != خالي_اشارو);

  صحيح errorcode = pthread_rwlock_rdlock(rwlock);
  أرجع errorcode == 0;
}

منطقي
RWMutexImpl::unlock_shared()
{
  pthread_rwlock_t* rwlock = static_cast<pthread_rwlock_t*>(معلومات_3);
  أكد(rwlock != خالي_اشارو);

  صحيح errorcode = pthread_rwlock_unlock(rwlock);
  أرجع errorcode == 0;
}

منطقي RWMutexImpl::try_تالو_shared() {
  pthread_rwlock_t *rwlock = static_cast<pthread_rwlock_t *>(معلومات_3);
  أكد(rwlock != خالي_اشارو);

  صحيح errorcode = pthread_rwlock_tryrdlock(rwlock);
  أرجع errorcode == 0;
}

منطقي
RWMutexImpl::تالو()
{
  pthread_rwlock_t* rwlock = static_cast<pthread_rwlock_t*>(معلومات_3);
  أكد(rwlock != خالي_اشارو);

  صحيح errorcode = pthread_rwlock_wrlock(rwlock);
  أرجع errorcode == 0;
}

منطقي
RWMutexImpl::unlock()
{
  pthread_rwlock_t* rwlock = static_cast<pthread_rwlock_t*>(معلومات_3);
  أكد(rwlock != خالي_اشارو);

  صحيح errorcode = pthread_rwlock_unlock(rwlock);
  أرجع errorcode == 0;
}

منطقي RWMutexImpl::try_تالو_2() {
  pthread_rwlock_t *rwlock = static_cast<pthread_rwlock_t *>(معلومات_3);
  أكد(rwlock != خالي_اشارو);

  صحيح errorcode = pthread_rwlock_trywrlock(rwlock);
  أرجع errorcode == 0;
}

#وإلا

RWMutexImpl::RWMutexImpl() : معلومات_3(جديد MutexImpl(خطأ)) { }

RWMutexImpl::~RWMutexImpl() {
  ميٽيو static_cast<MutexImpl *>(معلومات_3);
}

منطقي RWMutexImpl::تالو_shared() {
  أرجع static_cast<MutexImpl *>(معلومات_3)->acquire();
}

منطقي RWMutexImpl::unlock_shared() {
  أرجع static_cast<MutexImpl *>(معلومات_3)->release();
}

منطقي RWMutexImpl::try_تالو_shared() {
  أرجع static_cast<MutexImpl *>(معلومات_3)->tryacquire();
}

منطقي RWMutexImpl::تالو() {
  أرجع static_cast<MutexImpl *>(معلومات_3)->acquire();
}

منطقي RWMutexImpl::unlock() {
  أرجع static_cast<MutexImpl *>(معلومات_3)->release();
}

منطقي RWMutexImpl::try_تالو_2() {
  أرجع static_cast<MutexImpl *>(معلومات_3)->tryacquire();
}

#جيڪڏهن_پورو
#جيڪڏهن_پورو
#جيڪڏهن_پورو
