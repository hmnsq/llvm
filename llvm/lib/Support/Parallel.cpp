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

ಬಳಸಿ ನಾಮಾಕಾಶ llvm;
ಬಳಸಿ ನಾಮಾಕಾಶ llvm::parallel;

llvm::ThreadPoolStrategy parallel::strategy;

#ಆದರೆ LLVM_ENABLE_ಎಳೆಗಳು

ಸ್ಥಿರ thread_local ಚಿಹ್ನೆರಹಿತ thread_ಸೂಚ್ಯಂಕ = UINT_MAX;

ನಾಮಾಕಾಶ {

/// Runs closures on a thread pool in filo order.
ವರ್ಗ ThreadPoolExecutor {
ಸಾರ್ವಜನಿಕ:
  ಸ್ಪಷ್ಟ ThreadPoolExecutor(ThreadPoolStrategy S) {
    ಆದರೆ (S.UseJobserver)
      TheJobserver = JobserverClient::getInstance();

    Thread_ಎಣಿಕೆ = S.compute_thread_ಎಣಿಕೆ();
    // Spawn all but one of the threads in another thread as spawning threads
    // can take a while.
    ಎಳೆಗಳು.reserve(Thread_ಎಣಿಕೆ);
    ಎಳೆಗಳು.resize(1);
    std::ಬೀಗ_guard<std::mutex> ಬೀಗ_3(Mutex);
    // Use operator[] before creating the thread to avoid data race in .size()
    // in 'safe libc++' mode.
    ಸ್ವಯಂಚಾಲಿತ &Thread0 = ಎಳೆಗಳು[0];
    Thread0 = std::thread([ಇದು, S] {
      ಗಾಗಿ (ಚಿಹ್ನೆರಹಿತ I = 1; I < Thread_ಎಣಿಕೆ; ++I) {
        ಎಳೆಗಳು.emplace_ಹಿಂಭಾಗ([ಇದು, S, I] { work(S, I); });
        ಆದರೆ (Stop)
          ನಿಲ್ಲಿಸು;
      }
      ಎಳೆಗಳು_Created.ಗಣ_ಮೌಲ್ಯ();
      work(S, 0);
    });
  }

  // To make sure the thread pool executor can only be created with a parallel
  // strategy.
  ThreadPoolExecutor() = ಅಳಿಸು;

  ಶೂನ್ಯ stop() {
    {
      std::ಬೀಗ_guard<std::mutex> ಬೀಗ_3(Mutex);
      ಆದರೆ (Stop)
        ಹಿಂತಿರುಗಿ;
      Stop = ನಿಜ;
    }
    Cond.notify_all();
    ಎಳೆಗಳು_Created.get_future().wait();

    std::thread::id CurrentThreadId = std::this_thread::get_id();
    ಗಾಗಿ (std::thread &T : ಎಳೆಗಳು)
      ಆದರೆ (T.get_id() == CurrentThreadId)
        T.detach();
      ಇಲ್ಲದಿದ್ದರೆ
        T.join();
  }

  ~ThreadPoolExecutor() { stop(); }

  ರಚನೆ Creator {
    ಸ್ಥಿರ ಶೂನ್ಯ *call() { ಹಿಂತಿರುಗಿ ಹೊಸ ThreadPoolExecutor(strategy); }
  };
  ರಚನೆ Deleter {
    ಸ್ಥಿರ ಶೂನ್ಯ call(ಶೂನ್ಯ *Ptr) { ((ThreadPoolExecutor *)Ptr)->stop(); }
  };

  ರಚನೆ Work_ಅಂಶ {
    std::function<ಶೂನ್ಯ()> F;
    std::reference_wrapper<parallel::detail::Latch> L;
    ಶೂನ್ಯ ಕ್ರಿಯಾಕಾರಕ()() {
      F();
      L.get().dec();
    }
  };

  ಶೂನ್ಯ ಕೂಡಿಸು(std::function<ಶೂನ್ಯ()> F, parallel::detail::Latch &L) {
    {
      std::ಬೀಗ_guard<std::mutex> ಬೀಗ_3(Mutex);
      Work_ರಾಶಿ.ತಳ್ಳು_ಹಿಂಭಾಗ({std::move(F), std::ref(L)});
    }
    Cond.notify_one();
  }

  // Execute tasks from the work queue until the latch reaches zero.
  // Used by nested TaskGroups (on worker threads) to prevent deadlock:
  // instead of blocking in sync(), actively help drain the queue.
  ಶೂನ್ಯ helpSync(ಸ್ಥಿರಾಂಕ parallel::detail::Latch &L) {
    ಆಗ (L.get_ಎಣಿಕೆ() != 0) {
      std::unique_ಬೀಗ<std::mutex> ಬೀಗ_3(Mutex);
      ಆದರೆ (Stop || Work_ರಾಶಿ.ಖಾಲಿಗಳು())
        ಹಿಂತಿರುಗಿ;
      ತೆಗೆ_And_ಸತತ(ಬೀಗ_3);
    }
  }

  size_t get_Thread_ಎಣಿಕೆ() ಸ್ಥಿರಾಂಕ { ಹಿಂತಿರುಗಿ Thread_ಎಣಿಕೆ; }

ಖಾಸಗಿ:
  // Pop one task from the queue and run it. Must be called with Lock held;
  // releases Lock before executing the task.
  ಶೂನ್ಯ ತೆಗೆ_And_ಸತತ(std::unique_ಬೀಗ<std::mutex> &ಬೀಗ_3) {
    ಸ್ವಯಂಚಾಲಿತ ಅಂಶ = std::move(Work_ರಾಶಿ.ಹಿಂಭಾಗ());
    Work_ರಾಶಿ.ತೆಗೆ_ಹಿಂಭಾಗ();
    ಬೀಗ_3.unlock();
    ಅಂಶ();
  }

  ಶೂನ್ಯ work(ThreadPoolStrategy S, ಚಿಹ್ನೆರಹಿತ ThreadID) {
    thread_ಸೂಚ್ಯಂಕ = ThreadID;
    S.apply_thread_strategy(ThreadID);
    // Note on jobserver deadlock avoidance:
    // GNU Make grants each invoked process one implicit job slot. Our
    // JobserverClient models this by returning an implicit JobSlot on the
    // first successful tryAcquire() in a process. This guarantees forward
    // progress without requiring a dedicated "always-on" thread here.

    ಆಗ (ನಿಜ) {
      ಆದರೆ (TheJobserver) {
        // Jobserver-mode scheduling:
        // - Acquire one job slot (with exponential backoff to avoid busy-wait).
        // - While holding the slot, drain and run tasks from the local queue.
        // - Release the slot when the queue is empty or when shutting down.
        // Rationale: Holding a slot amortizes acquire/release overhead over
        // multiple tasks and avoids requeue/yield churn, while still enforcing
        // the jobserver’s global concurrency limit. With K available slots,
        // up to K workers run tasks in parallel; within each worker tasks run
        // sequentially until the local queue is empty.
        ಘಾತೀಯ_Backoff Backoff(std::chrono::hours(24));
        ಕೆಲಸ_ಸ್ಥಾನ ಸ್ಥಾನ;
        ಮಾಡು {
          ಆದರೆ (Stop)
            ಹಿಂತಿರುಗಿ;
          ಸ್ಥಾನ = TheJobserver->tryAcquire();
          ಆದರೆ (ಸ್ಥಾನ.isValid())
            ನಿಲ್ಲಿಸು;
        } ಆಗ (Backoff.wait_For_ಮುಂದಿನ_Attempt());

        llvm::scope_exit ಸ್ಥಾನ_Releaser(
            [&] { TheJobserver->release(std::move(ಸ್ಥಾನ)); });

        ಆಗ (ನಿಜ) {
          std::unique_ಬೀಗ<std::mutex> ಬೀಗ_3(Mutex);
          Cond.wait(ಬೀಗ_3, [&] { ಹಿಂತಿರುಗಿ Stop || !Work_ರಾಶಿ.ಖಾಲಿಗಳು(); });
          ಆದರೆ (Stop && Work_ರಾಶಿ.ಖಾಲಿಗಳು())
            ಹಿಂತಿರುಗಿ;
          ಆದರೆ (Work_ರಾಶಿ.ಖಾಲಿಗಳು())
            ನಿಲ್ಲಿಸು;
          ತೆಗೆ_And_ಸತತ(ಬೀಗ_3);
        }
      } ಇಲ್ಲದಿದ್ದರೆ {
        std::unique_ಬೀಗ<std::mutex> ಬೀಗ_3(Mutex);
        Cond.wait(ಬೀಗ_3, [&] { ಹಿಂತಿರುಗಿ Stop || !Work_ರಾಶಿ.ಖಾಲಿಗಳು(); });
        ಆದರೆ (Stop)
          ನಿಲ್ಲಿಸು;
        ತೆಗೆ_And_ಸತತ(ಬೀಗ_3);
      }
    }
  }

  std::atomic<ತಾರ್ಕಿಕ> Stop{ಸುಳ್ಳು};
  std::ಸದಿಶ<Work_ಅಂಶ> Work_ರಾಶಿ;
  std::mutex Mutex;
  std::condition_variable Cond;
  std::promise<ಶೂನ್ಯ> ಎಳೆಗಳು_Created;
  std::ಸದಿಶ<std::thread> ಎಳೆಗಳು;
  ಚಿಹ್ನೆರಹಿತ Thread_ಎಣಿಕೆ;

  JobserverClient *TheJobserver = ಶೂನ್ಯಸೂಚಕ;
};
} // namespace

ಸ್ಥಿರ ThreadPoolExecutor *getDefaultExecutor() {
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ _WIN32
  // The ManagedStatic enables the ThreadPoolExecutor to be stopped via
  // llvm_shutdown() on Windows. This is important to avoid various race
  // conditions at process exit that can cause crashes or deadlocks.

  ಸ್ಥಿರ ManagedStatic<ThreadPoolExecutor, ThreadPoolExecutor::Creator,
                       ThreadPoolExecutor::Deleter>
      ManagedExec;
  ಸ್ಥಿರ std::unique_ptr<ThreadPoolExecutor> Exec(&(*ManagedExec));
  ಹಿಂತಿರುಗಿ Exec.get();
#ಇಲ್ಲದಿದ್ದರೆ
  // ManagedStatic is not desired on other platforms. When `Exec` is destroyed
  // by llvm_shutdown(), worker threads will clean up and invoke TLS
  // destructors. This can lead to race conditions if other threads attempt to
  // access TLS objects that have already been destroyed.
  ಸ್ಥಿರ ThreadPoolExecutor Exec(strategy);
  ಹಿಂತಿರುಗಿ &Exec;
#ಆದರೆ_ಅಂತ್ಯ
}

size_t parallel::get_Thread_ಎಣಿಕೆ() {
  ಹಿಂತಿರುಗಿ getDefaultExecutor()->get_Thread_ಎಣಿಕೆ();
}
#ಆದರೆ_ಅಂತ್ಯ

Task_ಗುಂಪು::Task_ಗುಂಪು()
    : Parallel(
#ಆದರೆ LLVM_ENABLE_ಎಳೆಗಳು
          strategy.ಎಳೆಗಳು_Requested != 1
#ಇಲ್ಲದಿದ್ದರೆ
          ಸುಳ್ಳು
#ಆದರೆ_ಅಂತ್ಯ
      ) {
}

Task_ಗುಂಪು::~Task_ಗುಂಪು() {
#ಆದರೆ LLVM_ENABLE_ಎಳೆಗಳು
  // In a nested TaskGroup (threadIndex != -1u), actively help drain the queue.
  ತಾರ್ಕಿಕ IsNested = thread_ಸೂಚ್ಯಂಕ != UINT_MAX;
  ಆದರೆ (Parallel && IsNested)
    getDefaultExecutor()->helpSync(L);
#ಆದರೆ_ಅಂತ್ಯ
  L.ಸಮಕಾಲೀಕರಣ();
}

ಶೂನ್ಯ Task_ಗುಂಪು::spawn(std::function<ಶೂನ್ಯ()> F) {
#ಆದರೆ LLVM_ENABLE_ಎಳೆಗಳು
  ಆದರೆ (Parallel) {
    L.inc();
    getDefaultExecutor()->ಕೂಡಿಸು(std::move(F), L);
    ಹಿಂತಿರುಗಿ;
  }
#ಆದರೆ_ಅಂತ್ಯ
  F();
}

ಶೂನ್ಯ llvm::parallelFor(size_t Begin, size_t ಕೊನೆ_2,
                       function_ref<ಶೂನ್ಯ(size_t)> Fn) {
#ಆದರೆ LLVM_ENABLE_ಎಳೆಗಳು
  ಆದರೆ (strategy.ಎಳೆಗಳು_Requested != 1) {
    size_t ಅಂಕೆ_ವಸ್ತುಗಳು = ಕೊನೆ_2 - Begin;
    ಆದರೆ (ಅಂಕೆ_ವಸ್ತುಗಳು == 0)
      ಹಿಂತಿರುಗಿ;
    // Distribute work via an atomic counter shared by NumWorkers threads,
    // keeping the task count (and thus Linux futex calls) at O(ThreadCount)
    // For lld, per-file work is somewhat uneven, so a multipler > 1 is safer.
    // While 2 vs 4 vs 8 makes no measurable difference, 4 is used as a
    // reasonable default.
    size_t ಅಂಕೆ_Workers = std::min<size_t>(ಅಂಕೆ_ವಸ್ತುಗಳು, get_Thread_ಎಣಿಕೆ());
    size_t Chunk_ಗಾತ್ರ = std::max(size_t(1), ಅಂಕೆ_ವಸ್ತುಗಳು / (ಅಂಕೆ_Workers * 4));
    std::atomic<size_t> Idx{Begin};
    ಸ್ವಯಂಚಾಲಿತ ಕೆಲಸಗಾರ = [&] {
      ಆಗ (ನಿಜ) {
        size_t I = Idx.fetch_ಕೂಡಿಸು(Chunk_ಗಾತ್ರ, std::memory_order_relaxed);
        ಆದರೆ (I >= ಕೊನೆ_2)
          ನಿಲ್ಲಿಸು;
        size_t IEnd = std::min(I + Chunk_ಗಾತ್ರ, ಕೊನೆ_2);
        ಗಾಗಿ (; I < IEnd; ++I)
          Fn(I);
      }
    };

    // Run one worker on the calling thread: starts working immediately and
    // avoids an idle thread.
    Task_ಗುಂಪು TG;
    ಆಗ (--ಅಂಕೆ_Workers)
      TG.spawn(ಕೆಲಸಗಾರ);
    ಕೆಲಸಗಾರ();
    ಹಿಂತಿರುಗಿ;
  }
#ಆದರೆ_ಅಂತ್ಯ

  ಗಾಗಿ (; Begin != ಕೊನೆ_2; ++Begin)
    Fn(Begin);
}
