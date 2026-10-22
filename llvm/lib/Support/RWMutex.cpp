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

#यदि defined(LLVM_USE_RW_MUTEX_IMPL)
उपयुज्य नामाकाश llvm;
उपयुज्य नामाकाश sys;

#यदि !defined(LLVM_ENABLE_तन्तवः) || LLVM_ENABLE_तन्तवः == 0
// Define all methods as no-ops if threading is explicitly disabled

RWMutexImpl::RWMutexImpl() = स्वभाव;
RWMutexImpl::~RWMutexImpl() = स्वभाव;

तार्किक RWMutexImpl::तालकम्_shared() { प्रत्यागम सत्य; }
तार्किक RWMutexImpl::unlock_shared() { प्रत्यागम सत्य; }
तार्किक RWMutexImpl::try_तालकम्_shared() { प्रत्यागम सत्य; }
तार्किक RWMutexImpl::तालकम्() { प्रत्यागम सत्य; }
तार्किक RWMutexImpl::unlock() { प्रत्यागम सत्य; }
तार्किक RWMutexImpl::try_तालकम्_2() { प्रत्यागम सत्य; }

#अन्यथा

#यदि defined(HAVE_PTHREAD_H) && defined(HAVE_PTHREAD_RWLOCK_INIT)

#अन्तर्भाव <cassert>
#अन्तर्भाव <cstdlib>
#अन्तर्भाव <pthread.h>

// Construct a RWMutex using pthread calls
RWMutexImpl::RWMutexImpl()
{
  // Declare the pthread_rwlock data structures
  pthread_rwlock_t* rwlock =
    static_cast<pthread_rwlock_t*>(safe_malloc(परिमाण(pthread_rwlock_t)));

#यदि_परिभाषितम् __APPLE__
  // Workaround a bug/mis-feature in Darwin's pthread_rwlock_init.
  bzero(rwlock, परिमाण(pthread_rwlock_t));
#यदि_अन्तः

  // Initialize the rwlock
  पूर्णाङ्क errorcode = pthread_rwlock_init(rwlock, शून्यसूचकः);
  (शून्य)errorcode;
  प्रतिज्ञा(errorcode == 0);

  // Assign the data member
  दत्त_3 = rwlock;
}

// Destruct a RWMutex
RWMutexImpl::~RWMutexImpl()
{
  pthread_rwlock_t* rwlock = static_cast<pthread_rwlock_t*>(दत्त_3);
  प्रतिज्ञा(rwlock != शून्यसूचकः);
  pthread_rwlock_destroy(rwlock);
  मुक्त(rwlock);
}

तार्किक
RWMutexImpl::तालकम्_shared()
{
  pthread_rwlock_t* rwlock = static_cast<pthread_rwlock_t*>(दत्त_3);
  प्रतिज्ञा(rwlock != शून्यसूचकः);

  पूर्णाङ्क errorcode = pthread_rwlock_rdlock(rwlock);
  प्रत्यागम errorcode == 0;
}

तार्किक
RWMutexImpl::unlock_shared()
{
  pthread_rwlock_t* rwlock = static_cast<pthread_rwlock_t*>(दत्त_3);
  प्रतिज्ञा(rwlock != शून्यसूचकः);

  पूर्णाङ्क errorcode = pthread_rwlock_unlock(rwlock);
  प्रत्यागम errorcode == 0;
}

तार्किक RWMutexImpl::try_तालकम्_shared() {
  pthread_rwlock_t *rwlock = static_cast<pthread_rwlock_t *>(दत्त_3);
  प्रतिज्ञा(rwlock != शून्यसूचकः);

  पूर्णाङ्क errorcode = pthread_rwlock_tryrdlock(rwlock);
  प्रत्यागम errorcode == 0;
}

तार्किक
RWMutexImpl::तालकम्()
{
  pthread_rwlock_t* rwlock = static_cast<pthread_rwlock_t*>(दत्त_3);
  प्रतिज्ञा(rwlock != शून्यसूचकः);

  पूर्णाङ्क errorcode = pthread_rwlock_wrlock(rwlock);
  प्रत्यागम errorcode == 0;
}

तार्किक
RWMutexImpl::unlock()
{
  pthread_rwlock_t* rwlock = static_cast<pthread_rwlock_t*>(दत्त_3);
  प्रतिज्ञा(rwlock != शून्यसूचकः);

  पूर्णाङ्क errorcode = pthread_rwlock_unlock(rwlock);
  प्रत्यागम errorcode == 0;
}

तार्किक RWMutexImpl::try_तालकम्_2() {
  pthread_rwlock_t *rwlock = static_cast<pthread_rwlock_t *>(दत्त_3);
  प्रतिज्ञा(rwlock != शून्यसूचकः);

  पूर्णाङ्क errorcode = pthread_rwlock_trywrlock(rwlock);
  प्रत्यागम errorcode == 0;
}

#अन्यथा

RWMutexImpl::RWMutexImpl() : दत्त_3(नूतन MutexImpl(असत्य)) { }

RWMutexImpl::~RWMutexImpl() {
  विलोपय static_cast<MutexImpl *>(दत्त_3);
}

तार्किक RWMutexImpl::तालकम्_shared() {
  प्रत्यागम static_cast<MutexImpl *>(दत्त_3)->acquire();
}

तार्किक RWMutexImpl::unlock_shared() {
  प्रत्यागम static_cast<MutexImpl *>(दत्त_3)->release();
}

तार्किक RWMutexImpl::try_तालकम्_shared() {
  प्रत्यागम static_cast<MutexImpl *>(दत्त_3)->tryacquire();
}

तार्किक RWMutexImpl::तालकम्() {
  प्रत्यागम static_cast<MutexImpl *>(दत्त_3)->acquire();
}

तार्किक RWMutexImpl::unlock() {
  प्रत्यागम static_cast<MutexImpl *>(दत्त_3)->release();
}

तार्किक RWMutexImpl::try_तालकम्_2() {
  प्रत्यागम static_cast<MutexImpl *>(दत्त_3)->tryacquire();
}

#यदि_अन्तः
#यदि_अन्तः
#यदि_अन्तः
