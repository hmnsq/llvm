//===- llvm/Support/Parallel.cpp - Parallel algorithms --------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ಅನ್ತರ್ಭಾವ "llvm/Support/Parallel.h"
#ಅನ್ತರ್ಭಾವ "llvm/ADT/ScopeExit.h"
#ಅನ್ತರ್ಭಾವ "llvm/Config/llvm-config.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/ExponentialBackoff.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/Jobserver.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/ManagedStatic.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/Threading.h"

#ಅನ್ತರ್ಭಾವ <atomic>
#ಅನ್ತರ್ಭಾವ <future>
#ಅನ್ತರ್ಭಾವ <memory>
#ಅನ್ತರ್ಭಾವ <mutex>
#ಅನ್ತರ್ಭಾವ <thread>
#ಅನ್ತರ್ಭಾವ <vector>

using ನಾಮಾಕಾಶ llvm;
using ನಾಮಾಕಾಶ llvm::parallel;

llvm::ThreadPoolStrategy parallel::strategy;

#ಯದಿ LLVM_ENABLE_THREADS

ಸ್ಥಿರ thread_local unsigned threadIndex = UINT_MAX;

ನಾಮಾಕಾಶ {

/// Runs closures on a thread pool in filo order.
ವರ್ಗ ThreadPoolExecutor {
ಸಾರ್ವಜನಿಕ:
  explicit ThreadPoolExecutor(ThreadPoolStrategy S) {
    ಯದಿ (S.UseJobserver)
      TheJobserver = JobserverClient::getInstance();

    Thread_ಎಣಿಕೆ = S.compute_thread_ಎಣಿಕೆ();
    // Spawn all but one of the threads in another thread as spawning threads
    // can take a while.
    Threads.reserve(Thread_ಎಣಿಕೆ);
    Threads.resize(1);
    std::lock_guard<std::mutex> Lock(Mutex);
    // Use operator[] before creating the thread to avoid data race in .size()
    // in 'safe libc++' mode.
    auto &Thread0 = Threads[0];
    Thread0 = std::thread([ಏತತ್, S] {
      ಪ್ರತಿ (unsigned I = 1; I < Thread_ಎಣಿಕೆ; ++I) {
        Threads.emplace_back([ಏತತ್, S, I] { work(S, I); });
        ಯದಿ (Stop)
          ಭಙ್ಗ;
      }
      ThreadsCreated.set_ಮೌಲ್ಯ();
      work(S, 0);
    });
  }

  // To make sure the thread pool executor can only be created with a parallel
  // strategy.
  ThreadPoolExecutor() = delete;

  ಶೂನ್ಯ stop() {
    {
      std::lock_guard<std::mutex> Lock(Mutex);
      ಯದಿ (Stop)
        ಪ್ರತ್ಯಾಗಮ;
      Stop = ಸತ್ಯ;
    }
    Cond.notify_all();
    ThreadsCreated.get_future().wait();

    std::thread::id CurrentThreadId = std::this_thread::get_id();
    ಪ್ರತಿ (std::thread &T : Threads)
      ಯದಿ (T.get_id() == CurrentThreadId)
        T.detach();
      ಅನ್ಯಥಾ
        T.join();
  }

  ~ThreadPoolExecutor() { stop(); }

  ಸಂರಚನಾ Creator {
    ಸ್ಥಿರ ಶೂನ್ಯ *call() { ಪ್ರತ್ಯಾಗಮ ನೂತನ ThreadPoolExecutor(strategy); }
  };
  ಸಂರಚನಾ Deleter {
    ಸ್ಥಿರ ಶೂನ್ಯ call(ಶೂನ್ಯ *Ptr) { ((ThreadPoolExecutor *)Ptr)->stop(); }
  };

  ಸಂರಚನಾ WorkItem {
    std::function<ಶೂನ್ಯ()> F;
    std::reference_wrapper<parallel::detail::Latch> L;
    ಶೂನ್ಯ operator()() {
      F();
      L.get().dec();
    }
  };

  ಶೂನ್ಯ add(std::function<ಶೂನ್ಯ()> F, parallel::detail::Latch &L) {
    {
      std::lock_guard<std::mutex> Lock(Mutex);
      Work_ರಾಶಿ.ತಳ್ಳು_back({std::move(F), std::ref(L)});
    }
    Cond.notify_one();
  }

  // Execute tasks from the work queue until the latch reaches zero.
  // Used by nested TaskGroups (on worker threads) to prevent deadlock:
  // instead of blocking in sync(), actively help drain the queue.
  ಶೂನ್ಯ helpSync(ಧ್ರುವ parallel::detail::Latch &L) {
    ಯಾವತ್ (L.get_ಎಣಿಕೆ() != 0) {
      std::unique_lock<std::mutex> Lock(Mutex);
      ಯದಿ (Stop || Work_ರಾಶಿ.empty())
        ಪ್ರತ್ಯಾಗಮ;
      ತೆಗೆ_And_Run(Lock);
    }
  }

  size_t get_Thread_ಎಣಿಕೆ() ಧ್ರುವ { ಪ್ರತ್ಯಾಗಮ Thread_ಎಣಿಕೆ; }

ಗುಪ್ತ:
  // Pop one task from the queue and run it. Must be called with Lock held;
  // releases Lock before executing the task.
  ಶೂನ್ಯ ತೆಗೆ_And_Run(std::unique_lock<std::mutex> &Lock) {
    auto Item = std::move(Work_ರಾಶಿ.back());
    Work_ರಾಶಿ.ತೆಗೆ_back();
    Lock.unlock();
    Item();
  }

  ಶೂನ್ಯ work(ThreadPoolStrategy S, unsigned ThreadID) {
    threadIndex = ThreadID;
    S.apply_thread_strategy(ThreadID);
    // Note on jobserver deadlock avoidance:
    // GNU Make grants each invoked process one implicit job slot. Our
    // JobserverClient models this by returning an implicit JobSlot on the
    // first successful tryAcquire() in a process. This guarantees forward
    // progress without requiring a dedicated "always-on" thread here.

    ಯಾವತ್ (ಸತ್ಯ) {
      ಯದಿ (TheJobserver) {
        // Jobserver-mode scheduling:
        // - Acquire one job slot (with exponential backoff to avoid busy-wait).
        // - While holding the slot, drain and run tasks from the local queue.
        // - Release the slot when the queue is empty or when shutting down.
        // Rationale: Holding a slot amortizes acquire/release overhead over
        // multiple tasks and avoids requeue/yield churn, while still enforcing
        // the jobserver’s global concurrency limit. With K available slots,
        // up to K workers run tasks in parallel; within each worker tasks run
        // sequentially until the local queue is empty.
        ExponentialBackoff Backoff(std::chrono::hours(24));
        JobSlot Slot;
        ಕುರು {
          ಯದಿ (Stop)
            ಪ್ರತ್ಯಾಗಮ;
          Slot = TheJobserver->tryAcquire();
          ಯದಿ (Slot.isValid())
            ಭಙ್ಗ;
        } ಯಾವತ್ (Backoff.waitForNextAttempt());

        llvm::scope_exit SlotReleaser(
            [&] { TheJobserver->release(std::move(Slot)); });

        ಯಾವತ್ (ಸತ್ಯ) {
          std::unique_lock<std::mutex> Lock(Mutex);
          Cond.wait(Lock, [&] { ಪ್ರತ್ಯಾಗಮ Stop || !Work_ರಾಶಿ.empty(); });
          ಯದಿ (Stop && Work_ರಾಶಿ.empty())
            ಪ್ರತ್ಯಾಗಮ;
          ಯದಿ (Work_ರಾಶಿ.empty())
            ಭಙ್ಗ;
          ತೆಗೆ_And_Run(Lock);
        }
      } ಅನ್ಯಥಾ {
        std::unique_lock<std::mutex> Lock(Mutex);
        Cond.wait(Lock, [&] { ಪ್ರತ್ಯಾಗಮ Stop || !Work_ರಾಶಿ.empty(); });
        ಯದಿ (Stop)
          ಭಙ್ಗ;
        ತೆಗೆ_And_Run(Lock);
      }
    }
  }

  std::atomic<ತಾರ್ಕಿಕ> Stop{ಅಸತ್ಯ};
  std::vector<WorkItem> Work_ರಾಶಿ;
  std::mutex Mutex;
  std::condition_variable Cond;
  std::promise<ಶೂನ್ಯ> ThreadsCreated;
  std::vector<std::thread> Threads;
  unsigned Thread_ಎಣಿಕೆ;

  JobserverClient *TheJobserver = nullptr;
};
} // namespace

ಸ್ಥಿರ ThreadPoolExecutor *getDefaultExecutor() {
#ifdef _WIN32
  // The ManagedStatic enables the ThreadPoolExecutor to be stopped via
  // llvm_shutdown() on Windows. This is important to avoid various race
  // conditions at process exit that can cause crashes or deadlocks.

  ಸ್ಥಿರ ManagedStatic<ThreadPoolExecutor, ThreadPoolExecutor::Creator,
                       ThreadPoolExecutor::Deleter>
      ManagedExec;
  ಸ್ಥಿರ std::unique_ptr<ThreadPoolExecutor> Exec(&(*ManagedExec));
  ಪ್ರತ್ಯಾಗಮ Exec.get();
#ಅನ್ಯಥಾ
  // ManagedStatic is not desired on other platforms. When `Exec` is destroyed
  // by llvm_shutdown(), worker threads will clean up and invoke TLS
  // destructors. This can lead to race conditions if other threads attempt to
  // access TLS objects that have already been destroyed.
  ಸ್ಥಿರ ThreadPoolExecutor Exec(strategy);
  ಪ್ರತ್ಯಾಗಮ &Exec;
#endif
}

size_t parallel::get_Thread_ಎಣಿಕೆ() {
  ಪ್ರತ್ಯಾಗಮ getDefaultExecutor()->get_Thread_ಎಣಿಕೆ();
}
#endif

TaskGroup::TaskGroup()
    : Parallel(
#ಯದಿ LLVM_ENABLE_THREADS
          strategy.ThreadsRequested != 1
#ಅನ್ಯಥಾ
          ಅಸತ್ಯ
#endif
      ) {
}

TaskGroup::~TaskGroup() {
#ಯದಿ LLVM_ENABLE_THREADS
  // In a nested TaskGroup (threadIndex != -1u), actively help drain the queue.
  ತಾರ್ಕಿಕ IsNested = threadIndex != UINT_MAX;
  ಯದಿ (Parallel && IsNested)
    getDefaultExecutor()->helpSync(L);
#endif
  L.ಸಮಕಾಲ();
}

ಶೂನ್ಯ TaskGroup::spawn(std::function<ಶೂನ್ಯ()> F) {
#ಯದಿ LLVM_ENABLE_THREADS
  ಯದಿ (Parallel) {
    L.inc();
    getDefaultExecutor()->add(std::move(F), L);
    ಪ್ರತ್ಯಾಗಮ;
  }
#endif
  F();
}

ಶೂನ್ಯ llvm::parallelFor(size_t Begin, size_t End,
                       function_ref<ಶೂನ್ಯ(size_t)> Fn) {
#ಯದಿ LLVM_ENABLE_THREADS
  ಯದಿ (strategy.ThreadsRequested != 1) {
    size_t ಅಂಕೆ_ವಸ್ತುಗಳು = End - Begin;
    ಯದಿ (ಅಂಕೆ_ವಸ್ತುಗಳು == 0)
      ಪ್ರತ್ಯಾಗಮ;
    // Distribute work via an atomic counter shared by NumWorkers threads,
    // keeping the task count (and thus Linux futex calls) at O(ThreadCount)
    // For lld, per-file work is somewhat uneven, so a multipler > 1 is safer.
    // While 2 vs 4 vs 8 makes no measurable difference, 4 is used as a
    // reasonable default.
    size_t ಅಂಕೆ_Workers = std::min<size_t>(ಅಂಕೆ_ವಸ್ತುಗಳು, get_Thread_ಎಣಿಕೆ());
    size_t ChunkSize = std::max(size_t(1), ಅಂಕೆ_ವಸ್ತುಗಳು / (ಅಂಕೆ_Workers * 4));
    std::atomic<size_t> Idx{Begin};
    auto Worker = [&] {
      ಯಾವತ್ (ಸತ್ಯ) {
        size_t I = Idx.fetch_add(ChunkSize, std::memory_order_relaxed);
        ಯದಿ (I >= End)
          ಭಙ್ಗ;
        size_t IEnd = std::min(I + ChunkSize, End);
        ಪ್ರತಿ (; I < IEnd; ++I)
          Fn(I);
      }
    };

    // Run one worker on the calling thread: starts working immediately and
    // avoids an idle thread.
    TaskGroup TG;
    ಯಾವತ್ (--ಅಂಕೆ_Workers)
      TG.spawn(Worker);
    Worker();
    ಪ್ರತ್ಯಾಗಮ;
  }
#endif

  ಪ್ರತಿ (; Begin != End; ++Begin)
    Fn(Begin);
}
