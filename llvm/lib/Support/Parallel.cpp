//===- llvm/Support/Parallel.cpp - Parallel algorithms --------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#അന്തര്ഭാവ "llvm/Support/Parallel.h"
#അന്തര്ഭാവ "llvm/ADT/ScopeExit.h"
#അന്തര്ഭാവ "llvm/Config/llvm-config.h"
#അന്തര്ഭാവ "llvm/Support/ExponentialBackoff.h"
#അന്തര്ഭാവ "llvm/Support/Jobserver.h"
#അന്തര്ഭാവ "llvm/Support/ManagedStatic.h"
#അന്തര്ഭാവ "llvm/Support/Threading.h"

#അന്തര്ഭാവ <atomic>
#അന്തര്ഭാവ <future>
#അന്തര്ഭാവ <memory>
#അന്തര്ഭാവ <mutex>
#അന്തര്ഭാവ <thread>
#അന്തര്ഭാവ <vector>

using നാമാകാശ llvm;
using നാമാകാശ llvm::parallel;

llvm::ThreadPoolStrategy parallel::strategy;

#എങ്കിൽ LLVM_ENABLE_THREADS

സ്ഥിതം thread_local unsigned threadIndex = UINT_MAX;

നാമാകാശ {

/// Runs closures on a thread pool in filo order.
വർഗ്ഗം ThreadPoolExecutor {
പൊതു:
  explicit ThreadPoolExecutor(ThreadPoolStrategy S) {
    എങ്കിൽ (S.UseJobserver)
      TheJobserver = JobserverClient::getInstance();

    Thread_എണ്ണം = S.compute_thread_എണ്ണം();
    // Spawn all but one of the threads in another thread as spawning threads
    // can take a while.
    Threads.reserve(Thread_എണ്ണം);
    Threads.resize(1);
    std::lock_guard<std::mutex> Lock(Mutex);
    // Use operator[] before creating the thread to avoid data race in .size()
    // in 'safe libc++' mode.
    auto &Thread0 = Threads[0];
    Thread0 = std::thread([ഇത്, S] {
      വേണ്ടി (unsigned I = 1; I < Thread_എണ്ണം; ++I) {
        Threads.emplace_back([ഇത്, S, I] { work(S, I); });
        എങ്കിൽ (Stop)
          നിർത്തുക;
      }
      ThreadsCreated.set_മൂല്യം();
      work(S, 0);
    });
  }

  // To make sure the thread pool executor can only be created with a parallel
  // strategy.
  ThreadPoolExecutor() = delete;

  ശൂന്യം stop() {
    {
      std::lock_guard<std::mutex> Lock(Mutex);
      എങ്കിൽ (Stop)
        തിരികെ;
      Stop = സത്യം;
    }
    Cond.notify_all();
    ThreadsCreated.get_future().wait();

    std::thread::id CurrentThreadId = std::this_thread::get_id();
    വേണ്ടി (std::thread &T : Threads)
      എങ്കിൽ (T.get_id() == CurrentThreadId)
        T.detach();
      അല്ലെങ്കിൽ
        T.join();
  }

  ~ThreadPoolExecutor() { stop(); }

  ഘടന Creator {
    സ്ഥിതം ശൂന്യം *call() { തിരികെ പുതിയ ThreadPoolExecutor(strategy); }
  };
  ഘടന Deleter {
    സ്ഥിതം ശൂന്യം call(ശൂന്യം *Ptr) { ((ThreadPoolExecutor *)Ptr)->stop(); }
  };

  ഘടന WorkItem {
    std::function<ശൂന്യം()> F;
    std::reference_wrapper<parallel::detail::Latch> L;
    ശൂന്യം operator()() {
      F();
      L.get().dec();
    }
  };

  ശൂന്യം add(std::function<ശൂന്യം()> F, parallel::detail::Latch &L) {
    {
      std::lock_guard<std::mutex> Lock(Mutex);
      Work_അട്ടി.തള്ളുക_back({std::move(F), std::ref(L)});
    }
    Cond.notify_one();
  }

  // Execute tasks from the work queue until the latch reaches zero.
  // Used by nested TaskGroups (on worker threads) to prevent deadlock:
  // instead of blocking in sync(), actively help drain the queue.
  ശൂന്യം helpSync(സ്ഥിരം parallel::detail::Latch &L) {
    ആകുമ്പോൾ (L.get_എണ്ണം() != 0) {
      std::unique_lock<std::mutex> Lock(Mutex);
      എങ്കിൽ (Stop || Work_അട്ടി.empty())
        തിരികെ;
      എടുക്കുക_And_Run(Lock);
    }
  }

  size_t get_Thread_എണ്ണം() സ്ഥിരം { തിരികെ Thread_എണ്ണം; }

സ്വകാര്യ:
  // Pop one task from the queue and run it. Must be called with Lock held;
  // releases Lock before executing the task.
  ശൂന്യം എടുക്കുക_And_Run(std::unique_lock<std::mutex> &Lock) {
    auto Item = std::move(Work_അട്ടി.back());
    Work_അട്ടി.എടുക്കുക_back();
    Lock.unlock();
    Item();
  }

  ശൂന്യം work(ThreadPoolStrategy S, unsigned ThreadID) {
    threadIndex = ThreadID;
    S.apply_thread_strategy(ThreadID);
    // Note on jobserver deadlock avoidance:
    // GNU Make grants each invoked process one implicit job slot. Our
    // JobserverClient models this by returning an implicit JobSlot on the
    // first successful tryAcquire() in a process. This guarantees forward
    // progress without requiring a dedicated "always-on" thread here.

    ആകുമ്പോൾ (സത്യം) {
      എങ്കിൽ (TheJobserver) {
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
        ചെയ്യുക {
          എങ്കിൽ (Stop)
            തിരികെ;
          Slot = TheJobserver->tryAcquire();
          എങ്കിൽ (Slot.isValid())
            നിർത്തുക;
        } ആകുമ്പോൾ (Backoff.waitForNextAttempt());

        llvm::scope_exit SlotReleaser(
            [&] { TheJobserver->release(std::move(Slot)); });

        ആകുമ്പോൾ (സത്യം) {
          std::unique_lock<std::mutex> Lock(Mutex);
          Cond.wait(Lock, [&] { തിരികെ Stop || !Work_അട്ടി.empty(); });
          എങ്കിൽ (Stop && Work_അട്ടി.empty())
            തിരികെ;
          എങ്കിൽ (Work_അട്ടി.empty())
            നിർത്തുക;
          എടുക്കുക_And_Run(Lock);
        }
      } അല്ലെങ്കിൽ {
        std::unique_lock<std::mutex> Lock(Mutex);
        Cond.wait(Lock, [&] { തിരികെ Stop || !Work_അട്ടി.empty(); });
        എങ്കിൽ (Stop)
          നിർത്തുക;
        എടുക്കുക_And_Run(Lock);
      }
    }
  }

  std::atomic<തർക്കം> Stop{അസത്യം};
  std::vector<WorkItem> Work_അട്ടി;
  std::mutex Mutex;
  std::condition_variable Cond;
  std::promise<ശൂന്യം> ThreadsCreated;
  std::vector<std::thread> Threads;
  unsigned Thread_എണ്ണം;

  JobserverClient *TheJobserver = nullptr;
};
} // namespace

സ്ഥിതം ThreadPoolExecutor *getDefaultExecutor() {
#ifdef _WIN32
  // The ManagedStatic enables the ThreadPoolExecutor to be stopped via
  // llvm_shutdown() on Windows. This is important to avoid various race
  // conditions at process exit that can cause crashes or deadlocks.

  സ്ഥിതം ManagedStatic<ThreadPoolExecutor, ThreadPoolExecutor::Creator,
                       ThreadPoolExecutor::Deleter>
      ManagedExec;
  സ്ഥിതം std::unique_ptr<ThreadPoolExecutor> Exec(&(*ManagedExec));
  തിരികെ Exec.get();
#അല്ലെങ്കിൽ
  // ManagedStatic is not desired on other platforms. When `Exec` is destroyed
  // by llvm_shutdown(), worker threads will clean up and invoke TLS
  // destructors. This can lead to race conditions if other threads attempt to
  // access TLS objects that have already been destroyed.
  സ്ഥിതം ThreadPoolExecutor Exec(strategy);
  തിരികെ &Exec;
#endif
}

size_t parallel::get_Thread_എണ്ണം() {
  തിരികെ getDefaultExecutor()->get_Thread_എണ്ണം();
}
#endif

TaskGroup::TaskGroup()
    : Parallel(
#എങ്കിൽ LLVM_ENABLE_THREADS
          strategy.ThreadsRequested != 1
#അല്ലെങ്കിൽ
          അസത്യം
#endif
      ) {
}

TaskGroup::~TaskGroup() {
#എങ്കിൽ LLVM_ENABLE_THREADS
  // In a nested TaskGroup (threadIndex != -1u), actively help drain the queue.
  തർക്കം IsNested = threadIndex != UINT_MAX;
  എങ്കിൽ (Parallel && IsNested)
    getDefaultExecutor()->helpSync(L);
#endif
  L.സമന്വയം();
}

ശൂന്യം TaskGroup::spawn(std::function<ശൂന്യം()> F) {
#എങ്കിൽ LLVM_ENABLE_THREADS
  എങ്കിൽ (Parallel) {
    L.inc();
    getDefaultExecutor()->add(std::move(F), L);
    തിരികെ;
  }
#endif
  F();
}

ശൂന്യം llvm::parallelFor(size_t Begin, size_t End,
                       function_ref<ശൂന്യം(size_t)> Fn) {
#എങ്കിൽ LLVM_ENABLE_THREADS
  എങ്കിൽ (strategy.ThreadsRequested != 1) {
    size_t അക്കം_ഇനങ്ങൾ = End - Begin;
    എങ്കിൽ (അക്കം_ഇനങ്ങൾ == 0)
      തിരികെ;
    // Distribute work via an atomic counter shared by NumWorkers threads,
    // keeping the task count (and thus Linux futex calls) at O(ThreadCount)
    // For lld, per-file work is somewhat uneven, so a multipler > 1 is safer.
    // While 2 vs 4 vs 8 makes no measurable difference, 4 is used as a
    // reasonable default.
    size_t അക്കം_Workers = std::min<size_t>(അക്കം_ഇനങ്ങൾ, get_Thread_എണ്ണം());
    size_t ChunkSize = std::max(size_t(1), അക്കം_ഇനങ്ങൾ / (അക്കം_Workers * 4));
    std::atomic<size_t> Idx{Begin};
    auto Worker = [&] {
      ആകുമ്പോൾ (സത്യം) {
        size_t I = Idx.fetch_add(ChunkSize, std::memory_order_relaxed);
        എങ്കിൽ (I >= End)
          നിർത്തുക;
        size_t IEnd = std::min(I + ChunkSize, End);
        വേണ്ടി (; I < IEnd; ++I)
          Fn(I);
      }
    };

    // Run one worker on the calling thread: starts working immediately and
    // avoids an idle thread.
    TaskGroup TG;
    ആകുമ്പോൾ (--അക്കം_Workers)
      TG.spawn(Worker);
    Worker();
    തിരികെ;
  }
#endif

  വേണ്ടി (; Begin != End; ++Begin)
    Fn(Begin);
}
