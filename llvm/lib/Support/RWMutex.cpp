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

#समावेश "llvm/Support/RWMutex.h"
#समावेश "llvm/Config/config.h"
#समावेश "llvm/Config/llvm-config.h" // for LLVM_ENABLE_THREADS
#समावेश "llvm/Support/Allocator.h"

#अगर defined(LLVM_USE_RW_MUTEX_IMPL)
using namespace llvm;
using namespace sys;

#अगर !defined(LLVM_ENABLE_THREADS) || LLVM_ENABLE_THREADS == 0
// Define all methods as no-ops if threading is explicitly disabled

RWMutexImpl::RWMutexImpl() = शेष;
RWMutexImpl::~RWMutexImpl() = शेष;

bool RWMutexImpl::lock_shared() { वापस true; }
bool RWMutexImpl::unlock_shared() { वापस true; }
bool RWMutexImpl::try_lock_shared() { वापस true; }
bool RWMutexImpl::lock() { वापस true; }
bool RWMutexImpl::unlock() { वापस true; }
bool RWMutexImpl::try_lock() { वापस true; }

#अन्यथा

#अगर defined(HAVE_PTHREAD_H) && defined(HAVE_PTHREAD_RWLOCK_INIT)

#समावेश <cassert>
#समावेश <cstdlib>
#समावेश <pthread.h>

// Construct a RWMutex using pthread calls
RWMutexImpl::RWMutexImpl()
{
  // Declare the pthread_rwlock data structures
  pthread_rwlock_t* rwlock =
    static_cast<pthread_rwlock_t*>(safe_malloc(माप(pthread_rwlock_t)));

#अगर_घोषित __APPLE__
  // Workaround a bug/mis-feature in Darwin's pthread_rwlock_init.
  bzero(rwlock, माप(pthread_rwlock_t));
#पूर्ण_अगर

  // Initialize the rwlock
  पूर्णांक errorcode = pthread_rwlock_init(rwlock, nullptr);
  (व्योम)errorcode;
  निश्चित(errorcode == 0);

  // Assign the data member
  तथ्य_3 = rwlock;
}

// Destruct a RWMutex
RWMutexImpl::~RWMutexImpl()
{
  pthread_rwlock_t* rwlock = static_cast<pthread_rwlock_t*>(तथ्य_3);
  निश्चित(rwlock != nullptr);
  pthread_rwlock_destroy(rwlock);
  मुक्त(rwlock);
}

bool
RWMutexImpl::lock_shared()
{
  pthread_rwlock_t* rwlock = static_cast<pthread_rwlock_t*>(तथ्य_3);
  निश्चित(rwlock != nullptr);

  पूर्णांक errorcode = pthread_rwlock_rdlock(rwlock);
  वापस errorcode == 0;
}

bool
RWMutexImpl::unlock_shared()
{
  pthread_rwlock_t* rwlock = static_cast<pthread_rwlock_t*>(तथ्य_3);
  निश्चित(rwlock != nullptr);

  पूर्णांक errorcode = pthread_rwlock_unlock(rwlock);
  वापस errorcode == 0;
}

bool RWMutexImpl::try_lock_shared() {
  pthread_rwlock_t *rwlock = static_cast<pthread_rwlock_t *>(तथ्य_3);
  निश्चित(rwlock != nullptr);

  पूर्णांक errorcode = pthread_rwlock_tryrdlock(rwlock);
  वापस errorcode == 0;
}

bool
RWMutexImpl::lock()
{
  pthread_rwlock_t* rwlock = static_cast<pthread_rwlock_t*>(तथ्य_3);
  निश्चित(rwlock != nullptr);

  पूर्णांक errorcode = pthread_rwlock_wrlock(rwlock);
  वापस errorcode == 0;
}

bool
RWMutexImpl::unlock()
{
  pthread_rwlock_t* rwlock = static_cast<pthread_rwlock_t*>(तथ्य_3);
  निश्चित(rwlock != nullptr);

  पूर्णांक errorcode = pthread_rwlock_unlock(rwlock);
  वापस errorcode == 0;
}

bool RWMutexImpl::try_lock() {
  pthread_rwlock_t *rwlock = static_cast<pthread_rwlock_t *>(तथ्य_3);
  निश्चित(rwlock != nullptr);

  पूर्णांक errorcode = pthread_rwlock_trywrlock(rwlock);
  वापस errorcode == 0;
}

#अन्यथा

RWMutexImpl::RWMutexImpl() : तथ्य_3(नया MutexImpl(false)) { }

RWMutexImpl::~RWMutexImpl() {
  मिटाओ static_cast<MutexImpl *>(तथ्य_3);
}

bool RWMutexImpl::lock_shared() {
  वापस static_cast<MutexImpl *>(तथ्य_3)->acquire();
}

bool RWMutexImpl::unlock_shared() {
  वापस static_cast<MutexImpl *>(तथ्य_3)->release();
}

bool RWMutexImpl::try_lock_shared() {
  वापस static_cast<MutexImpl *>(तथ्य_3)->tryacquire();
}

bool RWMutexImpl::lock() {
  वापस static_cast<MutexImpl *>(तथ्य_3)->acquire();
}

bool RWMutexImpl::unlock() {
  वापस static_cast<MutexImpl *>(तथ्य_3)->release();
}

bool RWMutexImpl::try_lock() {
  वापस static_cast<MutexImpl *>(तथ्य_3)->tryacquire();
}

#पूर्ण_अगर
#पूर्ण_अगर
#पूर्ण_अगर
