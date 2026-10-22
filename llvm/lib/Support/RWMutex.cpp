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

#अन्तर्भाव "llvm/Support/RWMutex.h"
#अन्तर्भाव "llvm/Config/config.h"
#अन्तर्भाव "llvm/Config/llvm-config.h" // for LLVM_ENABLE_THREADS
#अन्तर्भाव "llvm/Support/Allocator.h"

#सचे defined(LLVM_USE_RW_MUTEX_IMPL)
using नामाकाश llvm;
using नामाकाश sys;

#सचे !defined(LLVM_ENABLE_THREADS) || LLVM_ENABLE_THREADS == 0
// Define all methods as no-ops if threading is explicitly disabled

RWMutexImpl::RWMutexImpl() = स्वभाव;
RWMutexImpl::~RWMutexImpl() = स्वभाव;

तार्किक RWMutexImpl::lock_shared() { पटिनिवत्त सच्च; }
तार्किक RWMutexImpl::unlock_shared() { पटिनिवत्त सच्च; }
तार्किक RWMutexImpl::try_lock_shared() { पटिनिवत्त सच्च; }
तार्किक RWMutexImpl::lock() { पटिनिवत्त सच्च; }
तार्किक RWMutexImpl::unlock() { पटिनिवत्त सच्च; }
तार्किक RWMutexImpl::try_lock() { पटिनिवत्त सच्च; }

#अञ्ञथा

#सचे defined(HAVE_PTHREAD_H) && defined(HAVE_PTHREAD_RWLOCK_INIT)

#अन्तर्भाव <cassert>
#अन्तर्भाव <cstdlib>
#अन्तर्भाव <pthread.h>

// Construct a RWMutex using pthread calls
RWMutexImpl::RWMutexImpl()
{
  // Declare the pthread_rwlock data structures
  pthread_rwlock_t* rwlock =
    static_cast<pthread_rwlock_t*>(safe_malloc(परिमाण(pthread_rwlock_t)));

#ifdef __APPLE__
  // Workaround a bug/mis-feature in Darwin's pthread_rwlock_init.
  bzero(rwlock, परिमाण(pthread_rwlock_t));
#endif

  // Initialize the rwlock
  पूर्णाङ्क errorcode = pthread_rwlock_init(rwlock, nullptr);
  (शून्य)errorcode;
  प्रतिज्ञा(errorcode == 0);

  // Assign the data member
  data_ = rwlock;
}

// Destruct a RWMutex
RWMutexImpl::~RWMutexImpl()
{
  pthread_rwlock_t* rwlock = static_cast<pthread_rwlock_t*>(data_);
  प्रतिज्ञा(rwlock != nullptr);
  pthread_rwlock_destroy(rwlock);
  मुक्त(rwlock);
}

तार्किक
RWMutexImpl::lock_shared()
{
  pthread_rwlock_t* rwlock = static_cast<pthread_rwlock_t*>(data_);
  प्रतिज्ञा(rwlock != nullptr);

  पूर्णाङ्क errorcode = pthread_rwlock_rdlock(rwlock);
  पटिनिवत्त errorcode == 0;
}

तार्किक
RWMutexImpl::unlock_shared()
{
  pthread_rwlock_t* rwlock = static_cast<pthread_rwlock_t*>(data_);
  प्रतिज्ञा(rwlock != nullptr);

  पूर्णाङ्क errorcode = pthread_rwlock_unlock(rwlock);
  पटिनिवत्त errorcode == 0;
}

तार्किक RWMutexImpl::try_lock_shared() {
  pthread_rwlock_t *rwlock = static_cast<pthread_rwlock_t *>(data_);
  प्रतिज्ञा(rwlock != nullptr);

  पूर्णाङ्क errorcode = pthread_rwlock_tryrdlock(rwlock);
  पटिनिवत्त errorcode == 0;
}

तार्किक
RWMutexImpl::lock()
{
  pthread_rwlock_t* rwlock = static_cast<pthread_rwlock_t*>(data_);
  प्रतिज्ञा(rwlock != nullptr);

  पूर्णाङ्क errorcode = pthread_rwlock_wrlock(rwlock);
  पटिनिवत्त errorcode == 0;
}

तार्किक
RWMutexImpl::unlock()
{
  pthread_rwlock_t* rwlock = static_cast<pthread_rwlock_t*>(data_);
  प्रतिज्ञा(rwlock != nullptr);

  पूर्णाङ्क errorcode = pthread_rwlock_unlock(rwlock);
  पटिनिवत्त errorcode == 0;
}

तार्किक RWMutexImpl::try_lock() {
  pthread_rwlock_t *rwlock = static_cast<pthread_rwlock_t *>(data_);
  प्रतिज्ञा(rwlock != nullptr);

  पूर्णाङ्क errorcode = pthread_rwlock_trywrlock(rwlock);
  पटिनिवत्त errorcode == 0;
}

#अञ्ञथा

RWMutexImpl::RWMutexImpl() : data_(नूतन MutexImpl(मिच्छा)) { }

RWMutexImpl::~RWMutexImpl() {
  delete static_cast<MutexImpl *>(data_);
}

तार्किक RWMutexImpl::lock_shared() {
  पटिनिवत्त static_cast<MutexImpl *>(data_)->acquire();
}

तार्किक RWMutexImpl::unlock_shared() {
  पटिनिवत्त static_cast<MutexImpl *>(data_)->release();
}

तार्किक RWMutexImpl::try_lock_shared() {
  पटिनिवत्त static_cast<MutexImpl *>(data_)->tryacquire();
}

तार्किक RWMutexImpl::lock() {
  पटिनिवत्त static_cast<MutexImpl *>(data_)->acquire();
}

तार्किक RWMutexImpl::unlock() {
  पटिनिवत्त static_cast<MutexImpl *>(data_)->release();
}

तार्किक RWMutexImpl::try_lock() {
  पटिनिवत्त static_cast<MutexImpl *>(data_)->tryacquire();
}

#endif
#endif
#endif
