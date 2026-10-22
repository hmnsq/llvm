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

#ଅନ୍ତର୍ଭାଵ "llvm/Support/RWMutex.h"
#ଅନ୍ତର୍ଭାଵ "llvm/Config/config.h"
#ଅନ୍ତର୍ଭାଵ "llvm/Config/llvm-config.h" // for LLVM_ENABLE_THREADS
#ଅନ୍ତର୍ଭାଵ "llvm/Support/Allocator.h"

#ଯଦି defined(LLVM_USE_RW_MUTEX_IMPL)
using ନାମାକାଶ llvm;
using ନାମାକାଶ sys;

#ଯଦି !defined(LLVM_ENABLE_THREADS) || LLVM_ENABLE_THREADS == 0
// Define all methods as no-ops if threading is explicitly disabled

RWMutexImpl::RWMutexImpl() = ପୂର୍ବନିର୍ଦ୍ଧାରିତ;
RWMutexImpl::~RWMutexImpl() = ପୂର୍ବନିର୍ଦ୍ଧାରିତ;

ତାର୍କିକ RWMutexImpl::lock_shared() { ଫେରନ୍ତୁ ସତ୍ୟ; }
ତାର୍କିକ RWMutexImpl::unlock_shared() { ଫେରନ୍ତୁ ସତ୍ୟ; }
ତାର୍କିକ RWMutexImpl::try_lock_shared() { ଫେରନ୍ତୁ ସତ୍ୟ; }
ତାର୍କିକ RWMutexImpl::lock() { ଫେରନ୍ତୁ ସତ୍ୟ; }
ତାର୍କିକ RWMutexImpl::unlock() { ଫେରନ୍ତୁ ସତ୍ୟ; }
ତାର୍କିକ RWMutexImpl::try_lock() { ଫେରନ୍ତୁ ସତ୍ୟ; }

#ଅନ୍ୟଥା

#ଯଦି defined(HAVE_PTHREAD_H) && defined(HAVE_PTHREAD_RWLOCK_INIT)

#ଅନ୍ତର୍ଭାଵ <cassert>
#ଅନ୍ତର୍ଭାଵ <cstdlib>
#ଅନ୍ତର୍ଭାଵ <pthread.h>

// Construct a RWMutex using pthread calls
RWMutexImpl::RWMutexImpl()
{
  // Declare the pthread_rwlock data structures
  pthread_rwlock_t* rwlock =
    static_cast<pthread_rwlock_t*>(safe_malloc(ଆକାର(pthread_rwlock_t)));

#ifdef __APPLE__
  // Workaround a bug/mis-feature in Darwin's pthread_rwlock_init.
  bzero(rwlock, ଆକାର(pthread_rwlock_t));
#endif

  // Initialize the rwlock
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା errorcode = pthread_rwlock_init(rwlock, nullptr);
  (ଶୂନ୍ୟ)errorcode;
  ପ୍ରତିଜ୍ଞା(errorcode == 0);

  // Assign the data member
  data_ = rwlock;
}

// Destruct a RWMutex
RWMutexImpl::~RWMutexImpl()
{
  pthread_rwlock_t* rwlock = static_cast<pthread_rwlock_t*>(data_);
  ପ୍ରତିଜ୍ଞା(rwlock != nullptr);
  pthread_rwlock_destroy(rwlock);
  ମୁକ୍ତ(rwlock);
}

ତାର୍କିକ
RWMutexImpl::lock_shared()
{
  pthread_rwlock_t* rwlock = static_cast<pthread_rwlock_t*>(data_);
  ପ୍ରତିଜ୍ଞା(rwlock != nullptr);

  ପୂର୍ଣ୍ଣସଂଖ୍ୟା errorcode = pthread_rwlock_rdlock(rwlock);
  ଫେରନ୍ତୁ errorcode == 0;
}

ତାର୍କିକ
RWMutexImpl::unlock_shared()
{
  pthread_rwlock_t* rwlock = static_cast<pthread_rwlock_t*>(data_);
  ପ୍ରତିଜ୍ଞା(rwlock != nullptr);

  ପୂର୍ଣ୍ଣସଂଖ୍ୟା errorcode = pthread_rwlock_unlock(rwlock);
  ଫେରନ୍ତୁ errorcode == 0;
}

ତାର୍କିକ RWMutexImpl::try_lock_shared() {
  pthread_rwlock_t *rwlock = static_cast<pthread_rwlock_t *>(data_);
  ପ୍ରତିଜ୍ଞା(rwlock != nullptr);

  ପୂର୍ଣ୍ଣସଂଖ୍ୟା errorcode = pthread_rwlock_tryrdlock(rwlock);
  ଫେରନ୍ତୁ errorcode == 0;
}

ତାର୍କିକ
RWMutexImpl::lock()
{
  pthread_rwlock_t* rwlock = static_cast<pthread_rwlock_t*>(data_);
  ପ୍ରତିଜ୍ଞା(rwlock != nullptr);

  ପୂର୍ଣ୍ଣସଂଖ୍ୟା errorcode = pthread_rwlock_wrlock(rwlock);
  ଫେରନ୍ତୁ errorcode == 0;
}

ତାର୍କିକ
RWMutexImpl::unlock()
{
  pthread_rwlock_t* rwlock = static_cast<pthread_rwlock_t*>(data_);
  ପ୍ରତିଜ୍ଞା(rwlock != nullptr);

  ପୂର୍ଣ୍ଣସଂଖ୍ୟା errorcode = pthread_rwlock_unlock(rwlock);
  ଫେରନ୍ତୁ errorcode == 0;
}

ତାର୍କିକ RWMutexImpl::try_lock() {
  pthread_rwlock_t *rwlock = static_cast<pthread_rwlock_t *>(data_);
  ପ୍ରତିଜ୍ଞା(rwlock != nullptr);

  ପୂର୍ଣ୍ଣସଂଖ୍ୟା errorcode = pthread_rwlock_trywrlock(rwlock);
  ଫେରନ୍ତୁ errorcode == 0;
}

#ଅନ୍ୟଥା

RWMutexImpl::RWMutexImpl() : data_(ନୂତନ MutexImpl(ମିଥ୍ୟା)) { }

RWMutexImpl::~RWMutexImpl() {
  delete static_cast<MutexImpl *>(data_);
}

ତାର୍କିକ RWMutexImpl::lock_shared() {
  ଫେରନ୍ତୁ static_cast<MutexImpl *>(data_)->acquire();
}

ତାର୍କିକ RWMutexImpl::unlock_shared() {
  ଫେରନ୍ତୁ static_cast<MutexImpl *>(data_)->release();
}

ତାର୍କିକ RWMutexImpl::try_lock_shared() {
  ଫେରନ୍ତୁ static_cast<MutexImpl *>(data_)->tryacquire();
}

ତାର୍କିକ RWMutexImpl::lock() {
  ଫେରନ୍ତୁ static_cast<MutexImpl *>(data_)->acquire();
}

ତାର୍କିକ RWMutexImpl::unlock() {
  ଫେରନ୍ତୁ static_cast<MutexImpl *>(data_)->release();
}

ତାର୍କିକ RWMutexImpl::try_lock() {
  ଫେରନ୍ତୁ static_cast<MutexImpl *>(data_)->tryacquire();
}

#endif
#endif
#endif
