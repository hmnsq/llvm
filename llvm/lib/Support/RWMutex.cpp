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

#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/RWMutex.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Config/config.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Config/llvm-config.h" // for LLVM_ENABLE_THREADS
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/Allocator.h"

#ਜੇ defined(LLVM_USE_RW_MUTEX_IMPL)
ਵਰਤੋਂ ਨਾਮਾਕਾਸ਼ llvm;
ਵਰਤੋਂ ਨਾਮਾਕਾਸ਼ sys;

#ਜੇ !defined(LLVM_ENABLE_ਧਾਗੇ) || LLVM_ENABLE_ਧਾਗੇ == 0
// Define all methods as no-ops if threading is explicitly disabled

RWMutexImpl::RWMutexImpl() = ਮੂਲ;
RWMutexImpl::~RWMutexImpl() = ਮੂਲ;

ਤਾਰਕਿਕ RWMutexImpl::ਤਾਲਾ_shared() { ਵਾਪਸ ਸੱਚ; }
ਤਾਰਕਿਕ RWMutexImpl::unlock_shared() { ਵਾਪਸ ਸੱਚ; }
ਤਾਰਕਿਕ RWMutexImpl::try_ਤਾਲਾ_shared() { ਵਾਪਸ ਸੱਚ; }
ਤਾਰਕਿਕ RWMutexImpl::ਤਾਲਾ() { ਵਾਪਸ ਸੱਚ; }
ਤਾਰਕਿਕ RWMutexImpl::unlock() { ਵਾਪਸ ਸੱਚ; }
ਤਾਰਕਿਕ RWMutexImpl::try_ਤਾਲਾ_2() { ਵਾਪਸ ਸੱਚ; }

#ਨਹੀਂਤਾਂ

#ਜੇ defined(HAVE_PTHREAD_H) && defined(HAVE_PTHREAD_RWLOCK_INIT)

#ਅਨ੍ਤਰ੍ਭਾਵ <cassert>
#ਅਨ੍ਤਰ੍ਭਾਵ <cstdlib>
#ਅਨ੍ਤਰ੍ਭਾਵ <pthread.h>

// Construct a RWMutex using pthread calls
RWMutexImpl::RWMutexImpl()
{
  // Declare the pthread_rwlock data structures
  pthread_rwlock_t* rwlock =
    static_cast<pthread_rwlock_t*>(ਸੁਰੱਖਿਅਤ_malloc(ਆਕਾਰ(pthread_rwlock_t)));

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ __APPLE__
  // Workaround a bug/mis-feature in Darwin's pthread_rwlock_init.
  bzero(rwlock, ਆਕਾਰ(pthread_rwlock_t));
#ਜੇ_ਅੰਤ

  // Initialize the rwlock
  ਪੂਰਨ errorcode = pthread_rwlock_init(rwlock, ਖਾਲੀ_ਸੂਚਕ);
  (ਖਾਲੀ)errorcode;
  ਪ੍ਰਤਿਜ੍ਞਾ(errorcode == 0);

  // Assign the data member
  ਅੰਕੜੇ_3 = rwlock;
}

// Destruct a RWMutex
RWMutexImpl::~RWMutexImpl()
{
  pthread_rwlock_t* rwlock = static_cast<pthread_rwlock_t*>(ਅੰਕੜੇ_3);
  ਪ੍ਰਤਿਜ੍ਞਾ(rwlock != ਖਾਲੀ_ਸੂਚਕ);
  pthread_rwlock_destroy(rwlock);
  ਮੁਕਤ(rwlock);
}

ਤਾਰਕਿਕ
RWMutexImpl::ਤਾਲਾ_shared()
{
  pthread_rwlock_t* rwlock = static_cast<pthread_rwlock_t*>(ਅੰਕੜੇ_3);
  ਪ੍ਰਤਿਜ੍ਞਾ(rwlock != ਖਾਲੀ_ਸੂਚਕ);

  ਪੂਰਨ errorcode = pthread_rwlock_rdlock(rwlock);
  ਵਾਪਸ errorcode == 0;
}

ਤਾਰਕਿਕ
RWMutexImpl::unlock_shared()
{
  pthread_rwlock_t* rwlock = static_cast<pthread_rwlock_t*>(ਅੰਕੜੇ_3);
  ਪ੍ਰਤਿਜ੍ਞਾ(rwlock != ਖਾਲੀ_ਸੂਚਕ);

  ਪੂਰਨ errorcode = pthread_rwlock_unlock(rwlock);
  ਵਾਪਸ errorcode == 0;
}

ਤਾਰਕਿਕ RWMutexImpl::try_ਤਾਲਾ_shared() {
  pthread_rwlock_t *rwlock = static_cast<pthread_rwlock_t *>(ਅੰਕੜੇ_3);
  ਪ੍ਰਤਿਜ੍ਞਾ(rwlock != ਖਾਲੀ_ਸੂਚਕ);

  ਪੂਰਨ errorcode = pthread_rwlock_tryrdlock(rwlock);
  ਵਾਪਸ errorcode == 0;
}

ਤਾਰਕਿਕ
RWMutexImpl::ਤਾਲਾ()
{
  pthread_rwlock_t* rwlock = static_cast<pthread_rwlock_t*>(ਅੰਕੜੇ_3);
  ਪ੍ਰਤਿਜ੍ਞਾ(rwlock != ਖਾਲੀ_ਸੂਚਕ);

  ਪੂਰਨ errorcode = pthread_rwlock_wrlock(rwlock);
  ਵਾਪਸ errorcode == 0;
}

ਤਾਰਕਿਕ
RWMutexImpl::unlock()
{
  pthread_rwlock_t* rwlock = static_cast<pthread_rwlock_t*>(ਅੰਕੜੇ_3);
  ਪ੍ਰਤਿਜ੍ਞਾ(rwlock != ਖਾਲੀ_ਸੂਚਕ);

  ਪੂਰਨ errorcode = pthread_rwlock_unlock(rwlock);
  ਵਾਪਸ errorcode == 0;
}

ਤਾਰਕਿਕ RWMutexImpl::try_ਤਾਲਾ_2() {
  pthread_rwlock_t *rwlock = static_cast<pthread_rwlock_t *>(ਅੰਕੜੇ_3);
  ਪ੍ਰਤਿਜ੍ਞਾ(rwlock != ਖਾਲੀ_ਸੂਚਕ);

  ਪੂਰਨ errorcode = pthread_rwlock_trywrlock(rwlock);
  ਵਾਪਸ errorcode == 0;
}

#ਨਹੀਂਤਾਂ

RWMutexImpl::RWMutexImpl() : ਅੰਕੜੇ_3(ਨਵਾਂ MutexImpl(ਝੂਠ)) { }

RWMutexImpl::~RWMutexImpl() {
  ਮਿਟਾਓ static_cast<MutexImpl *>(ਅੰਕੜੇ_3);
}

ਤਾਰਕਿਕ RWMutexImpl::ਤਾਲਾ_shared() {
  ਵਾਪਸ static_cast<MutexImpl *>(ਅੰਕੜੇ_3)->acquire();
}

ਤਾਰਕਿਕ RWMutexImpl::unlock_shared() {
  ਵਾਪਸ static_cast<MutexImpl *>(ਅੰਕੜੇ_3)->release();
}

ਤਾਰਕਿਕ RWMutexImpl::try_ਤਾਲਾ_shared() {
  ਵਾਪਸ static_cast<MutexImpl *>(ਅੰਕੜੇ_3)->tryacquire();
}

ਤਾਰਕਿਕ RWMutexImpl::ਤਾਲਾ() {
  ਵਾਪਸ static_cast<MutexImpl *>(ਅੰਕੜੇ_3)->acquire();
}

ਤਾਰਕਿਕ RWMutexImpl::unlock() {
  ਵਾਪਸ static_cast<MutexImpl *>(ਅੰਕੜੇ_3)->release();
}

ਤਾਰਕਿਕ RWMutexImpl::try_ਤਾਲਾ_2() {
  ਵਾਪਸ static_cast<MutexImpl *>(ਅੰਕੜੇ_3)->tryacquire();
}

#ਜੇ_ਅੰਤ
#ਜੇ_ਅੰਤ
#ਜੇ_ਅੰਤ
