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

#ಅನ್ತರ್ಭಾವ "llvm/Support/RWMutex.h"
#ಅನ್ತರ್ಭಾವ "llvm/Config/config.h"
#ಅನ್ತರ್ಭಾವ "llvm/Config/llvm-config.h" // for LLVM_ENABLE_THREADS
#ಅನ್ತರ್ಭಾವ "llvm/Support/Allocator.h"

#ಆದರೆ defined(LLVM_USE_RW_MUTEX_IMPL)
ಬಳಸಿ ನಾಮಾಕಾಶ llvm;
ಬಳಸಿ ನಾಮಾಕಾಶ sys;

#ಆದರೆ !defined(LLVM_ENABLE_ಎಳೆಗಳು) || LLVM_ENABLE_ಎಳೆಗಳು == 0
// Define all methods as no-ops if threading is explicitly disabled

RWMutexImpl::RWMutexImpl() = ಪೂರ್ವನಿಯೋಜಿತ;
RWMutexImpl::~RWMutexImpl() = ಪೂರ್ವನಿಯೋಜಿತ;

ತಾರ್ಕಿಕ RWMutexImpl::ಬೀಗ_shared() { ಹಿಂತಿರುಗಿ ನಿಜ; }
ತಾರ್ಕಿಕ RWMutexImpl::unlock_shared() { ಹಿಂತಿರುಗಿ ನಿಜ; }
ತಾರ್ಕಿಕ RWMutexImpl::try_ಬೀಗ_shared() { ಹಿಂತಿರುಗಿ ನಿಜ; }
ತಾರ್ಕಿಕ RWMutexImpl::ಬೀಗ_2() { ಹಿಂತಿರುಗಿ ನಿಜ; }
ತಾರ್ಕಿಕ RWMutexImpl::unlock() { ಹಿಂತಿರುಗಿ ನಿಜ; }
ತಾರ್ಕಿಕ RWMutexImpl::try_ಬೀಗ_2() { ಹಿಂತಿರುಗಿ ನಿಜ; }

#ಇಲ್ಲದಿದ್ದರೆ

#ಆದರೆ defined(HAVE_PTHREAD_H) && defined(HAVE_PTHREAD_RWLOCK_INIT)

#ಅನ್ತರ್ಭಾವ <cassert>
#ಅನ್ತರ್ಭಾವ <cstdlib>
#ಅನ್ತರ್ಭಾವ <pthread.h>

// Construct a RWMutex using pthread calls
RWMutexImpl::RWMutexImpl()
{
  // Declare the pthread_rwlock data structures
  pthread_rwlock_t* rwlock =
    static_cast<pthread_rwlock_t*>(ಸುರಕ್ಷಿತ_malloc(ಗಾತ್ರ(pthread_rwlock_t)));

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ __APPLE__
  // Workaround a bug/mis-feature in Darwin's pthread_rwlock_init.
  bzero(rwlock, ಗಾತ್ರ(pthread_rwlock_t));
#ಆದರೆ_ಅಂತ್ಯ

  // Initialize the rwlock
  ಪೂರ್ಣಾಂಕ errorcode = pthread_rwlock_init(rwlock, ಶೂನ್ಯಸೂಚಕ);
  (ಶೂನ್ಯ)errorcode;
  ಪ್ರತಿಜ್ಞಾ(errorcode == 0);

  // Assign the data member
  ದತ್ತಾಂಶ_3 = rwlock;
}

// Destruct a RWMutex
RWMutexImpl::~RWMutexImpl()
{
  pthread_rwlock_t* rwlock = static_cast<pthread_rwlock_t*>(ದತ್ತಾಂಶ_3);
  ಪ್ರತಿಜ್ಞಾ(rwlock != ಶೂನ್ಯಸೂಚಕ);
  pthread_rwlock_destroy(rwlock);
  ಬಿಡುಗಡೆ(rwlock);
}

ತಾರ್ಕಿಕ
RWMutexImpl::ಬೀಗ_shared()
{
  pthread_rwlock_t* rwlock = static_cast<pthread_rwlock_t*>(ದತ್ತಾಂಶ_3);
  ಪ್ರತಿಜ್ಞಾ(rwlock != ಶೂನ್ಯಸೂಚಕ);

  ಪೂರ್ಣಾಂಕ errorcode = pthread_rwlock_rdlock(rwlock);
  ಹಿಂತಿರುಗಿ errorcode == 0;
}

ತಾರ್ಕಿಕ
RWMutexImpl::unlock_shared()
{
  pthread_rwlock_t* rwlock = static_cast<pthread_rwlock_t*>(ದತ್ತಾಂಶ_3);
  ಪ್ರತಿಜ್ಞಾ(rwlock != ಶೂನ್ಯಸೂಚಕ);

  ಪೂರ್ಣಾಂಕ errorcode = pthread_rwlock_unlock(rwlock);
  ಹಿಂತಿರುಗಿ errorcode == 0;
}

ತಾರ್ಕಿಕ RWMutexImpl::try_ಬೀಗ_shared() {
  pthread_rwlock_t *rwlock = static_cast<pthread_rwlock_t *>(ದತ್ತಾಂಶ_3);
  ಪ್ರತಿಜ್ಞಾ(rwlock != ಶೂನ್ಯಸೂಚಕ);

  ಪೂರ್ಣಾಂಕ errorcode = pthread_rwlock_tryrdlock(rwlock);
  ಹಿಂತಿರುಗಿ errorcode == 0;
}

ತಾರ್ಕಿಕ
RWMutexImpl::ಬೀಗ_2()
{
  pthread_rwlock_t* rwlock = static_cast<pthread_rwlock_t*>(ದತ್ತಾಂಶ_3);
  ಪ್ರತಿಜ್ಞಾ(rwlock != ಶೂನ್ಯಸೂಚಕ);

  ಪೂರ್ಣಾಂಕ errorcode = pthread_rwlock_wrlock(rwlock);
  ಹಿಂತಿರುಗಿ errorcode == 0;
}

ತಾರ್ಕಿಕ
RWMutexImpl::unlock()
{
  pthread_rwlock_t* rwlock = static_cast<pthread_rwlock_t*>(ದತ್ತಾಂಶ_3);
  ಪ್ರತಿಜ್ಞಾ(rwlock != ಶೂನ್ಯಸೂಚಕ);

  ಪೂರ್ಣಾಂಕ errorcode = pthread_rwlock_unlock(rwlock);
  ಹಿಂತಿರುಗಿ errorcode == 0;
}

ತಾರ್ಕಿಕ RWMutexImpl::try_ಬೀಗ_2() {
  pthread_rwlock_t *rwlock = static_cast<pthread_rwlock_t *>(ದತ್ತಾಂಶ_3);
  ಪ್ರತಿಜ್ಞಾ(rwlock != ಶೂನ್ಯಸೂಚಕ);

  ಪೂರ್ಣಾಂಕ errorcode = pthread_rwlock_trywrlock(rwlock);
  ಹಿಂತಿರುಗಿ errorcode == 0;
}

#ಇಲ್ಲದಿದ್ದರೆ

RWMutexImpl::RWMutexImpl() : ದತ್ತಾಂಶ_3(ಹೊಸ MutexImpl(ಸುಳ್ಳು)) { }

RWMutexImpl::~RWMutexImpl() {
  ಅಳಿಸು static_cast<MutexImpl *>(ದತ್ತಾಂಶ_3);
}

ತಾರ್ಕಿಕ RWMutexImpl::ಬೀಗ_shared() {
  ಹಿಂತಿರುಗಿ static_cast<MutexImpl *>(ದತ್ತಾಂಶ_3)->acquire();
}

ತಾರ್ಕಿಕ RWMutexImpl::unlock_shared() {
  ಹಿಂತಿರುಗಿ static_cast<MutexImpl *>(ದತ್ತಾಂಶ_3)->release();
}

ತಾರ್ಕಿಕ RWMutexImpl::try_ಬೀಗ_shared() {
  ಹಿಂತಿರುಗಿ static_cast<MutexImpl *>(ದತ್ತಾಂಶ_3)->tryacquire();
}

ತಾರ್ಕಿಕ RWMutexImpl::ಬೀಗ_2() {
  ಹಿಂತಿರುಗಿ static_cast<MutexImpl *>(ದತ್ತಾಂಶ_3)->acquire();
}

ತಾರ್ಕಿಕ RWMutexImpl::unlock() {
  ಹಿಂತಿರುಗಿ static_cast<MutexImpl *>(ದತ್ತಾಂಶ_3)->release();
}

ತಾರ್ಕಿಕ RWMutexImpl::try_ಬೀಗ_2() {
  ಹಿಂತಿರುಗಿ static_cast<MutexImpl *>(ದತ್ತಾಂಶ_3)->tryacquire();
}

#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ_ಅಂತ್ಯ
