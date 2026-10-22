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

#అన్తర్భావ "llvm/Support/RWMutex.h"
#అన్తర్భావ "llvm/Config/config.h"
#అన్తర్భావ "llvm/Config/llvm-config.h" // for LLVM_ENABLE_THREADS
#అన్తర్భావ "llvm/Support/Allocator.h"

#ఒకవేళ defined(LLVM_USE_RW_MUTEX_IMPL)
ఉపయోగించి నామాకాశ llvm;
ఉపయోగించి నామాకాశ sys;

#ఒకవేళ !defined(LLVM_ENABLE_దారాలు) || LLVM_ENABLE_దారాలు == 0
// Define all methods as no-ops if threading is explicitly disabled

RWMutexImpl::RWMutexImpl() = అప్రమేయ;
RWMutexImpl::~RWMutexImpl() = అప్రమేయ;

తార్కిక RWMutexImpl::తాళం_shared() { తిరిగి నిజం; }
తార్కిక RWMutexImpl::unlock_shared() { తిరిగి నిజం; }
తార్కిక RWMutexImpl::try_తాళం_shared() { తిరిగి నిజం; }
తార్కిక RWMutexImpl::తాళం_2() { తిరిగి నిజం; }
తార్కిక RWMutexImpl::unlock() { తిరిగి నిజం; }
తార్కిక RWMutexImpl::try_తాళం_2() { తిరిగి నిజం; }

#లేకపోతే

#ఒకవేళ defined(HAVE_PTHREAD_H) && defined(HAVE_PTHREAD_RWLOCK_INIT)

#అన్తర్భావ <cassert>
#అన్తర్భావ <cstdlib>
#అన్తర్భావ <pthread.h>

// Construct a RWMutex using pthread calls
RWMutexImpl::RWMutexImpl()
{
  // Declare the pthread_rwlock data structures
  pthread_rwlock_t* rwlock =
    static_cast<pthread_rwlock_t*>(సురక్షిత_malloc(పరిమాణం(pthread_rwlock_t)));

#నిర్వచితమైతే __APPLE__
  // Workaround a bug/mis-feature in Darwin's pthread_rwlock_init.
  bzero(rwlock, పరిమాణం(pthread_rwlock_t));
#అయితే_ముగింపు

  // Initialize the rwlock
  పూర్ణాంకం errorcode = pthread_rwlock_init(rwlock, శూన్యసూచిక);
  (శూన్య)errorcode;
  ప్రతిజ్ఞా(errorcode == 0);

  // Assign the data member
  దత్తాంశం_3 = rwlock;
}

// Destruct a RWMutex
RWMutexImpl::~RWMutexImpl()
{
  pthread_rwlock_t* rwlock = static_cast<pthread_rwlock_t*>(దత్తాంశం_3);
  ప్రతిజ్ఞా(rwlock != శూన్యసూచిక);
  pthread_rwlock_destroy(rwlock);
  విడుదల(rwlock);
}

తార్కిక
RWMutexImpl::తాళం_shared()
{
  pthread_rwlock_t* rwlock = static_cast<pthread_rwlock_t*>(దత్తాంశం_3);
  ప్రతిజ్ఞా(rwlock != శూన్యసూచిక);

  పూర్ణాంకం errorcode = pthread_rwlock_rdlock(rwlock);
  తిరిగి errorcode == 0;
}

తార్కిక
RWMutexImpl::unlock_shared()
{
  pthread_rwlock_t* rwlock = static_cast<pthread_rwlock_t*>(దత్తాంశం_3);
  ప్రతిజ్ఞా(rwlock != శూన్యసూచిక);

  పూర్ణాంకం errorcode = pthread_rwlock_unlock(rwlock);
  తిరిగి errorcode == 0;
}

తార్కిక RWMutexImpl::try_తాళం_shared() {
  pthread_rwlock_t *rwlock = static_cast<pthread_rwlock_t *>(దత్తాంశం_3);
  ప్రతిజ్ఞా(rwlock != శూన్యసూచిక);

  పూర్ణాంకం errorcode = pthread_rwlock_tryrdlock(rwlock);
  తిరిగి errorcode == 0;
}

తార్కిక
RWMutexImpl::తాళం_2()
{
  pthread_rwlock_t* rwlock = static_cast<pthread_rwlock_t*>(దత్తాంశం_3);
  ప్రతిజ్ఞా(rwlock != శూన్యసూచిక);

  పూర్ణాంకం errorcode = pthread_rwlock_wrlock(rwlock);
  తిరిగి errorcode == 0;
}

తార్కిక
RWMutexImpl::unlock()
{
  pthread_rwlock_t* rwlock = static_cast<pthread_rwlock_t*>(దత్తాంశం_3);
  ప్రతిజ్ఞా(rwlock != శూన్యసూచిక);

  పూర్ణాంకం errorcode = pthread_rwlock_unlock(rwlock);
  తిరిగి errorcode == 0;
}

తార్కిక RWMutexImpl::try_తాళం_2() {
  pthread_rwlock_t *rwlock = static_cast<pthread_rwlock_t *>(దత్తాంశం_3);
  ప్రతిజ్ఞా(rwlock != శూన్యసూచిక);

  పూర్ణాంకం errorcode = pthread_rwlock_trywrlock(rwlock);
  తిరిగి errorcode == 0;
}

#లేకపోతే

RWMutexImpl::RWMutexImpl() : దత్తాంశం_3(కొత్త MutexImpl(అబద్ధం)) { }

RWMutexImpl::~RWMutexImpl() {
  తొలగించు static_cast<MutexImpl *>(దత్తాంశం_3);
}

తార్కిక RWMutexImpl::తాళం_shared() {
  తిరిగి static_cast<MutexImpl *>(దత్తాంశం_3)->acquire();
}

తార్కిక RWMutexImpl::unlock_shared() {
  తిరిగి static_cast<MutexImpl *>(దత్తాంశం_3)->release();
}

తార్కిక RWMutexImpl::try_తాళం_shared() {
  తిరిగి static_cast<MutexImpl *>(దత్తాంశం_3)->tryacquire();
}

తార్కిక RWMutexImpl::తాళం_2() {
  తిరిగి static_cast<MutexImpl *>(దత్తాంశం_3)->acquire();
}

తార్కిక RWMutexImpl::unlock() {
  తిరిగి static_cast<MutexImpl *>(దత్తాంశం_3)->release();
}

తార్కిక RWMutexImpl::try_తాళం_2() {
  తిరిగి static_cast<MutexImpl *>(దత్తాంశం_3)->tryacquire();
}

#అయితే_ముగింపు
#అయితే_ముగింపు
#అయితే_ముగింపు
