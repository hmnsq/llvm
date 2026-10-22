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

#അന്തര്ഭാവ "llvm/Support/RWMutex.h"
#അന്തര്ഭാവ "llvm/Config/config.h"
#അന്തര്ഭാവ "llvm/Config/llvm-config.h" // for LLVM_ENABLE_THREADS
#അന്തര്ഭാവ "llvm/Support/Allocator.h"

#എങ്കിൽ defined(LLVM_USE_RW_MUTEX_IMPL)
using നാമാകാശ llvm;
using നാമാകാശ sys;

#എങ്കിൽ !defined(LLVM_ENABLE_THREADS) || LLVM_ENABLE_THREADS == 0
// Define all methods as no-ops if threading is explicitly disabled

RWMutexImpl::RWMutexImpl() = സ്ഥിരസ്ഥിതി;
RWMutexImpl::~RWMutexImpl() = സ്ഥിരസ്ഥിതി;

തർക്കം RWMutexImpl::lock_shared() { തിരികെ സത്യം; }
തർക്കം RWMutexImpl::unlock_shared() { തിരികെ സത്യം; }
തർക്കം RWMutexImpl::try_lock_shared() { തിരികെ സത്യം; }
തർക്കം RWMutexImpl::lock() { തിരികെ സത്യം; }
തർക്കം RWMutexImpl::unlock() { തിരികെ സത്യം; }
തർക്കം RWMutexImpl::try_lock() { തിരികെ സത്യം; }

#അല്ലെങ്കിൽ

#എങ്കിൽ defined(HAVE_PTHREAD_H) && defined(HAVE_PTHREAD_RWLOCK_INIT)

#അന്തര്ഭാവ <cassert>
#അന്തര്ഭാവ <cstdlib>
#അന്തര്ഭാവ <pthread.h>

// Construct a RWMutex using pthread calls
RWMutexImpl::RWMutexImpl()
{
  // Declare the pthread_rwlock data structures
  pthread_rwlock_t* rwlock =
    static_cast<pthread_rwlock_t*>(safe_malloc(വലുപ്പം(pthread_rwlock_t)));

#ifdef __APPLE__
  // Workaround a bug/mis-feature in Darwin's pthread_rwlock_init.
  bzero(rwlock, വലുപ്പം(pthread_rwlock_t));
#endif

  // Initialize the rwlock
  പൂർണ്ണസംഖ്യ errorcode = pthread_rwlock_init(rwlock, nullptr);
  (ശൂന്യം)errorcode;
  പ്രതിജ്ഞാ(errorcode == 0);

  // Assign the data member
  data_ = rwlock;
}

// Destruct a RWMutex
RWMutexImpl::~RWMutexImpl()
{
  pthread_rwlock_t* rwlock = static_cast<pthread_rwlock_t*>(data_);
  പ്രതിജ്ഞാ(rwlock != nullptr);
  pthread_rwlock_destroy(rwlock);
  സ്വതന്ത്രം(rwlock);
}

തർക്കം
RWMutexImpl::lock_shared()
{
  pthread_rwlock_t* rwlock = static_cast<pthread_rwlock_t*>(data_);
  പ്രതിജ്ഞാ(rwlock != nullptr);

  പൂർണ്ണസംഖ്യ errorcode = pthread_rwlock_rdlock(rwlock);
  തിരികെ errorcode == 0;
}

തർക്കം
RWMutexImpl::unlock_shared()
{
  pthread_rwlock_t* rwlock = static_cast<pthread_rwlock_t*>(data_);
  പ്രതിജ്ഞാ(rwlock != nullptr);

  പൂർണ്ണസംഖ്യ errorcode = pthread_rwlock_unlock(rwlock);
  തിരികെ errorcode == 0;
}

തർക്കം RWMutexImpl::try_lock_shared() {
  pthread_rwlock_t *rwlock = static_cast<pthread_rwlock_t *>(data_);
  പ്രതിജ്ഞാ(rwlock != nullptr);

  പൂർണ്ണസംഖ്യ errorcode = pthread_rwlock_tryrdlock(rwlock);
  തിരികെ errorcode == 0;
}

തർക്കം
RWMutexImpl::lock()
{
  pthread_rwlock_t* rwlock = static_cast<pthread_rwlock_t*>(data_);
  പ്രതിജ്ഞാ(rwlock != nullptr);

  പൂർണ്ണസംഖ്യ errorcode = pthread_rwlock_wrlock(rwlock);
  തിരികെ errorcode == 0;
}

തർക്കം
RWMutexImpl::unlock()
{
  pthread_rwlock_t* rwlock = static_cast<pthread_rwlock_t*>(data_);
  പ്രതിജ്ഞാ(rwlock != nullptr);

  പൂർണ്ണസംഖ്യ errorcode = pthread_rwlock_unlock(rwlock);
  തിരികെ errorcode == 0;
}

തർക്കം RWMutexImpl::try_lock() {
  pthread_rwlock_t *rwlock = static_cast<pthread_rwlock_t *>(data_);
  പ്രതിജ്ഞാ(rwlock != nullptr);

  പൂർണ്ണസംഖ്യ errorcode = pthread_rwlock_trywrlock(rwlock);
  തിരികെ errorcode == 0;
}

#അല്ലെങ്കിൽ

RWMutexImpl::RWMutexImpl() : data_(പുതിയ MutexImpl(അസത്യം)) { }

RWMutexImpl::~RWMutexImpl() {
  delete static_cast<MutexImpl *>(data_);
}

തർക്കം RWMutexImpl::lock_shared() {
  തിരികെ static_cast<MutexImpl *>(data_)->acquire();
}

തർക്കം RWMutexImpl::unlock_shared() {
  തിരികെ static_cast<MutexImpl *>(data_)->release();
}

തർക്കം RWMutexImpl::try_lock_shared() {
  തിരികെ static_cast<MutexImpl *>(data_)->tryacquire();
}

തർക്കം RWMutexImpl::lock() {
  തിരികെ static_cast<MutexImpl *>(data_)->acquire();
}

തർക്കം RWMutexImpl::unlock() {
  തിരികെ static_cast<MutexImpl *>(data_)->release();
}

തർക്കം RWMutexImpl::try_lock() {
  തിരികെ static_cast<MutexImpl *>(data_)->tryacquire();
}

#endif
#endif
#endif
