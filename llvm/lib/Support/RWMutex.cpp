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

#அந்தர்பாவ "llvm/Support/RWMutex.h"
#அந்தர்பாவ "llvm/Config/config.h"
#அந்தர்பாவ "llvm/Config/llvm-config.h" // for LLVM_ENABLE_THREADS
#அந்தர்பாவ "llvm/Support/Allocator.h"

#எனில் defined(LLVM_USE_RW_MUTEX_IMPL)
using நாமாகாஶ llvm;
using நாமாகாஶ sys;

#எனில் !defined(LLVM_ENABLE_THREADS) || LLVM_ENABLE_THREADS == 0
// Define all methods as no-ops if threading is explicitly disabled

RWMutexImpl::RWMutexImpl() = இயல்பு;
RWMutexImpl::~RWMutexImpl() = இயல்பு;

தருக்க RWMutexImpl::lock_shared() { திரும்பு உண்மை; }
தருக்க RWMutexImpl::unlock_shared() { திரும்பு உண்மை; }
தருக்க RWMutexImpl::try_lock_shared() { திரும்பு உண்மை; }
தருக்க RWMutexImpl::lock() { திரும்பு உண்மை; }
தருக்க RWMutexImpl::unlock() { திரும்பு உண்மை; }
தருக்க RWMutexImpl::try_lock() { திரும்பு உண்மை; }

#இல்லையெனில்

#எனில் defined(HAVE_PTHREAD_H) && defined(HAVE_PTHREAD_RWLOCK_INIT)

#அந்தர்பாவ <cassert>
#அந்தர்பாவ <cstdlib>
#அந்தர்பாவ <pthread.h>

// Construct a RWMutex using pthread calls
RWMutexImpl::RWMutexImpl()
{
  // Declare the pthread_rwlock data structures
  pthread_rwlock_t* rwlock =
    static_cast<pthread_rwlock_t*>(safe_malloc(அளவு(pthread_rwlock_t)));

#ifdef __APPLE__
  // Workaround a bug/mis-feature in Darwin's pthread_rwlock_init.
  bzero(rwlock, அளவு(pthread_rwlock_t));
#endif

  // Initialize the rwlock
  முழு errorcode = pthread_rwlock_init(rwlock, nullptr);
  (வெறுமை)errorcode;
  ப்ரதிஜ்ஞா(errorcode == 0);

  // Assign the data member
  data_ = rwlock;
}

// Destruct a RWMutex
RWMutexImpl::~RWMutexImpl()
{
  pthread_rwlock_t* rwlock = static_cast<pthread_rwlock_t*>(data_);
  ப்ரதிஜ்ஞா(rwlock != nullptr);
  pthread_rwlock_destroy(rwlock);
  விடுவி(rwlock);
}

தருக்க
RWMutexImpl::lock_shared()
{
  pthread_rwlock_t* rwlock = static_cast<pthread_rwlock_t*>(data_);
  ப்ரதிஜ்ஞா(rwlock != nullptr);

  முழு errorcode = pthread_rwlock_rdlock(rwlock);
  திரும்பு errorcode == 0;
}

தருக்க
RWMutexImpl::unlock_shared()
{
  pthread_rwlock_t* rwlock = static_cast<pthread_rwlock_t*>(data_);
  ப்ரதிஜ்ஞா(rwlock != nullptr);

  முழு errorcode = pthread_rwlock_unlock(rwlock);
  திரும்பு errorcode == 0;
}

தருக்க RWMutexImpl::try_lock_shared() {
  pthread_rwlock_t *rwlock = static_cast<pthread_rwlock_t *>(data_);
  ப்ரதிஜ்ஞா(rwlock != nullptr);

  முழு errorcode = pthread_rwlock_tryrdlock(rwlock);
  திரும்பு errorcode == 0;
}

தருக்க
RWMutexImpl::lock()
{
  pthread_rwlock_t* rwlock = static_cast<pthread_rwlock_t*>(data_);
  ப்ரதிஜ்ஞா(rwlock != nullptr);

  முழு errorcode = pthread_rwlock_wrlock(rwlock);
  திரும்பு errorcode == 0;
}

தருக்க
RWMutexImpl::unlock()
{
  pthread_rwlock_t* rwlock = static_cast<pthread_rwlock_t*>(data_);
  ப்ரதிஜ்ஞா(rwlock != nullptr);

  முழு errorcode = pthread_rwlock_unlock(rwlock);
  திரும்பு errorcode == 0;
}

தருக்க RWMutexImpl::try_lock() {
  pthread_rwlock_t *rwlock = static_cast<pthread_rwlock_t *>(data_);
  ப்ரதிஜ்ஞா(rwlock != nullptr);

  முழு errorcode = pthread_rwlock_trywrlock(rwlock);
  திரும்பு errorcode == 0;
}

#இல்லையெனில்

RWMutexImpl::RWMutexImpl() : data_(புதிய MutexImpl(பொய்)) { }

RWMutexImpl::~RWMutexImpl() {
  delete static_cast<MutexImpl *>(data_);
}

தருக்க RWMutexImpl::lock_shared() {
  திரும்பு static_cast<MutexImpl *>(data_)->acquire();
}

தருக்க RWMutexImpl::unlock_shared() {
  திரும்பு static_cast<MutexImpl *>(data_)->release();
}

தருக்க RWMutexImpl::try_lock_shared() {
  திரும்பு static_cast<MutexImpl *>(data_)->tryacquire();
}

தருக்க RWMutexImpl::lock() {
  திரும்பு static_cast<MutexImpl *>(data_)->acquire();
}

தருக்க RWMutexImpl::unlock() {
  திரும்பு static_cast<MutexImpl *>(data_)->release();
}

தருக்க RWMutexImpl::try_lock() {
  திரும்பு static_cast<MutexImpl *>(data_)->tryacquire();
}

#endif
#endif
#endif
