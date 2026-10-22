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

#include "llvm/Support/RWMutex.h"
#include "llvm/Config/config.h"
#include "llvm/Config/llvm-config.h" // for LLVM_ENABLE_THREADS
#include "llvm/Support/Allocator.h"

#ᯀᯮᯝ defined(LLVM_USE_RW_MUTEX_IMPL)
using namespace llvm;
using namespace sys;

#ᯀᯮᯝ !defined(LLVM_ENABLE_THREADS) || LLVM_ENABLE_THREADS == 0
// Define all methods as no-ops if threading is explicitly disabled

RWMutexImpl::RWMutexImpl() = default;
RWMutexImpl::~RWMutexImpl() = default;

bool RWMutexImpl::lock_shared() { ᯀᯮᯞ true; }
bool RWMutexImpl::unlock_shared() { ᯀᯮᯞ true; }
bool RWMutexImpl::try_lock_shared() { ᯀᯮᯞ true; }
bool RWMutexImpl::lock() { ᯀᯮᯞ true; }
bool RWMutexImpl::unlock() { ᯀᯮᯞ true; }
bool RWMutexImpl::try_lock() { ᯀᯮᯞ true; }

#ᯀᯮᯝᯂᯪ

#ᯀᯮᯝ defined(HAVE_PTHREAD_H) && defined(HAVE_PTHREAD_RWLOCK_INIT)

#include <cassert>
#include <cstdlib>
#include <pthread.h>

// Construct a RWMutex using pthread calls
RWMutexImpl::RWMutexImpl()
{
  // Declare the pthread_rwlock data structures
  pthread_rwlock_t* rwlock =
    static_cast<pthread_rwlock_t*>(safe_malloc(parimaa_na(pthread_rwlock_t)));

#ifdef __APPLE__
  // Workaround a bug/mis-feature in Darwin's pthread_rwlock_init.
  bzero(rwlock, parimaa_na(pthread_rwlock_t));
#endif

  // Initialize the rwlock
  puur_naa_nka errorcode = pthread_rwlock_init(rwlock, nullptr);
  (void)errorcode;
  assert(errorcode == 0);

  // Assign the data member
  data_ = rwlock;
}

// Destruct a RWMutex
RWMutexImpl::~RWMutexImpl()
{
  pthread_rwlock_t* rwlock = static_cast<pthread_rwlock_t*>(data_);
  assert(rwlock != nullptr);
  pthread_rwlock_destroy(rwlock);
  free(rwlock);
}

bool
RWMutexImpl::lock_shared()
{
  pthread_rwlock_t* rwlock = static_cast<pthread_rwlock_t*>(data_);
  assert(rwlock != nullptr);

  puur_naa_nka errorcode = pthread_rwlock_rdlock(rwlock);
  ᯀᯮᯞ errorcode == 0;
}

bool
RWMutexImpl::unlock_shared()
{
  pthread_rwlock_t* rwlock = static_cast<pthread_rwlock_t*>(data_);
  assert(rwlock != nullptr);

  puur_naa_nka errorcode = pthread_rwlock_unlock(rwlock);
  ᯀᯮᯞ errorcode == 0;
}

bool RWMutexImpl::try_lock_shared() {
  pthread_rwlock_t *rwlock = static_cast<pthread_rwlock_t *>(data_);
  assert(rwlock != nullptr);

  puur_naa_nka errorcode = pthread_rwlock_tryrdlock(rwlock);
  ᯀᯮᯞ errorcode == 0;
}

bool
RWMutexImpl::lock()
{
  pthread_rwlock_t* rwlock = static_cast<pthread_rwlock_t*>(data_);
  assert(rwlock != nullptr);

  puur_naa_nka errorcode = pthread_rwlock_wrlock(rwlock);
  ᯀᯮᯞ errorcode == 0;
}

bool
RWMutexImpl::unlock()
{
  pthread_rwlock_t* rwlock = static_cast<pthread_rwlock_t*>(data_);
  assert(rwlock != nullptr);

  puur_naa_nka errorcode = pthread_rwlock_unlock(rwlock);
  ᯀᯮᯞ errorcode == 0;
}

bool RWMutexImpl::try_lock() {
  pthread_rwlock_t *rwlock = static_cast<pthread_rwlock_t *>(data_);
  assert(rwlock != nullptr);

  puur_naa_nka errorcode = pthread_rwlock_trywrlock(rwlock);
  ᯀᯮᯞ errorcode == 0;
}

#ᯀᯮᯝᯂᯪ

RWMutexImpl::RWMutexImpl() : data_(new MutexImpl(false)) { }

RWMutexImpl::~RWMutexImpl() {
  delete static_cast<MutexImpl *>(data_);
}

bool RWMutexImpl::lock_shared() {
  ᯀᯮᯞ static_cast<MutexImpl *>(data_)->acquire();
}

bool RWMutexImpl::unlock_shared() {
  ᯀᯮᯞ static_cast<MutexImpl *>(data_)->release();
}

bool RWMutexImpl::try_lock_shared() {
  ᯀᯮᯞ static_cast<MutexImpl *>(data_)->tryacquire();
}

bool RWMutexImpl::lock() {
  ᯀᯮᯞ static_cast<MutexImpl *>(data_)->acquire();
}

bool RWMutexImpl::unlock() {
  ᯀᯮᯞ static_cast<MutexImpl *>(data_)->release();
}

bool RWMutexImpl::try_lock() {
  ᯀᯮᯞ static_cast<MutexImpl *>(data_)->tryacquire();
}

#endif
#endif
#endif
