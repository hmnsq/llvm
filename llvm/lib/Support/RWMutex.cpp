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
استفاده نطاق llvm;
استفاده نطاق sys;

#إذا !defined(LLVM_ENABLE_نخها) || LLVM_ENABLE_نخها == 0
// Define all methods as no-ops if threading is explicitly disabled

RWMutexImpl::RWMutexImpl() = افتراضي;
RWMutexImpl::~RWMutexImpl() = افتراضي;

منطقي RWMutexImpl::قفل_shared() { أرجع درست; }
منطقي RWMutexImpl::unlock_shared() { أرجع درست; }
منطقي RWMutexImpl::try_قفل_shared() { أرجع درست; }
منطقي RWMutexImpl::قفل_2() { أرجع درست; }
منطقي RWMutexImpl::unlock() { أرجع درست; }
منطقي RWMutexImpl::try_قفل_2() { أرجع درست; }

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
    static_cast<pthread_rwlock_t*>(امن_malloc(حجم(pthread_rwlock_t)));

#اگر_تعریف_شده __APPLE__
  // Workaround a bug/mis-feature in Darwin's pthread_rwlock_init.
  bzero(rwlock, حجم(pthread_rwlock_t));
#پایان_اگر

  // Initialize the rwlock
  صحيح errorcode = pthread_rwlock_init(rwlock, اشاره_تهی);
  (فراغ)errorcode;
  أكد(errorcode == 0);

  // Assign the data member
  داده_3 = rwlock;
}

// Destruct a RWMutex
RWMutexImpl::~RWMutexImpl()
{
  pthread_rwlock_t* rwlock = static_cast<pthread_rwlock_t*>(داده_3);
  أكد(rwlock != اشاره_تهی);
  pthread_rwlock_destroy(rwlock);
  حرر(rwlock);
}

منطقي
RWMutexImpl::قفل_shared()
{
  pthread_rwlock_t* rwlock = static_cast<pthread_rwlock_t*>(داده_3);
  أكد(rwlock != اشاره_تهی);

  صحيح errorcode = pthread_rwlock_rdlock(rwlock);
  أرجع errorcode == 0;
}

منطقي
RWMutexImpl::unlock_shared()
{
  pthread_rwlock_t* rwlock = static_cast<pthread_rwlock_t*>(داده_3);
  أكد(rwlock != اشاره_تهی);

  صحيح errorcode = pthread_rwlock_unlock(rwlock);
  أرجع errorcode == 0;
}

منطقي RWMutexImpl::try_قفل_shared() {
  pthread_rwlock_t *rwlock = static_cast<pthread_rwlock_t *>(داده_3);
  أكد(rwlock != اشاره_تهی);

  صحيح errorcode = pthread_rwlock_tryrdlock(rwlock);
  أرجع errorcode == 0;
}

منطقي
RWMutexImpl::قفل_2()
{
  pthread_rwlock_t* rwlock = static_cast<pthread_rwlock_t*>(داده_3);
  أكد(rwlock != اشاره_تهی);

  صحيح errorcode = pthread_rwlock_wrlock(rwlock);
  أرجع errorcode == 0;
}

منطقي
RWMutexImpl::unlock()
{
  pthread_rwlock_t* rwlock = static_cast<pthread_rwlock_t*>(داده_3);
  أكد(rwlock != اشاره_تهی);

  صحيح errorcode = pthread_rwlock_unlock(rwlock);
  أرجع errorcode == 0;
}

منطقي RWMutexImpl::try_قفل_2() {
  pthread_rwlock_t *rwlock = static_cast<pthread_rwlock_t *>(داده_3);
  أكد(rwlock != اشاره_تهی);

  صحيح errorcode = pthread_rwlock_trywrlock(rwlock);
  أرجع errorcode == 0;
}

#وإلا

RWMutexImpl::RWMutexImpl() : داده_3(جديد MutexImpl(خطأ)) { }

RWMutexImpl::~RWMutexImpl() {
  حذف static_cast<MutexImpl *>(داده_3);
}

منطقي RWMutexImpl::قفل_shared() {
  أرجع static_cast<MutexImpl *>(داده_3)->acquire();
}

منطقي RWMutexImpl::unlock_shared() {
  أرجع static_cast<MutexImpl *>(داده_3)->release();
}

منطقي RWMutexImpl::try_قفل_shared() {
  أرجع static_cast<MutexImpl *>(داده_3)->tryacquire();
}

منطقي RWMutexImpl::قفل_2() {
  أرجع static_cast<MutexImpl *>(داده_3)->acquire();
}

منطقي RWMutexImpl::unlock() {
  أرجع static_cast<MutexImpl *>(داده_3)->release();
}

منطقي RWMutexImpl::try_قفل_2() {
  أرجع static_cast<MutexImpl *>(داده_3)->tryacquire();
}

#پایان_اگر
#پایان_اگر
#پایان_اگر
