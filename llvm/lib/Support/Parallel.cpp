//===- llvm/Support/Parallel.cpp - Parallel algorithms --------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#شامل "llvm/Support/Parallel.h"
#شامل "llvm/ADT/ScopeExit.h"
#شامل "llvm/Config/llvm-config.h"
#شامل "llvm/Support/ExponentialBackoff.h"
#شامل "llvm/Support/Jobserver.h"
#شامل "llvm/Support/ManagedStatic.h"
#شامل "llvm/Support/Threading.h"

#شامل <atomic>
#شامل <future>
#شامل <memory>
#شامل <mutex>
#شامل <thread>
#شامل <vector>

using فضاینام llvm;
using فضاینام llvm::parallel;

llvm::ThreadPoolStrategy parallel::strategy;

#اگر LLVM_ENABLE_THREADS

ایستا thread_local unsigned threadIndex = UINT_MAX;

فضاینام {

/// Runs closures on a thread pool in filo order.
رده ThreadPoolExecutor {
عمومی:
  explicit ThreadPoolExecutor(ThreadPoolStrategy S) {
    اگر (S.UseJobserver)
      TheJobserver = JobserverClient::getInstance();

    Thread_شمار = S.compute_thread_شمار();
    // Spawn all but one of the threads in another thread as spawning threads
    // can take a while.
    Threads.reserve(Thread_شمار);
    Threads.resize(1);
    std::lock_guard<std::mutex> Lock(Mutex);
    // Use operator[] before creating the thread to avoid data race in .size()
    // in 'safe libc++' mode.
    auto &Thread0 = Threads[0];
    Thread0 = std::thread([این, S] {
      برای (unsigned I = 1; I < Thread_شمار; ++I) {
        Threads.emplace_back([این, S, I] { work(S, I); });
        اگر (Stop)
          بشکن;
      }
      ThreadsCreated.set_مقدار();
      work(S, 0);
    });
  }

  // To make sure the thread pool executor can only be created with a parallel
  // strategy.
  ThreadPoolExecutor() = delete;

  تهی stop() {
    {
      std::lock_guard<std::mutex> Lock(Mutex);
      اگر (Stop)
        بازگشت;
      Stop = درست;
    }
    Cond.notify_all();
    ThreadsCreated.get_future().wait();

    std::thread::id CurrentThreadId = std::this_thread::get_id();
    برای (std::thread &T : Threads)
      اگر (T.get_id() == CurrentThreadId)
        T.detach();
      وگرنه
        T.join();
  }

  ~ThreadPoolExecutor() { stop(); }

  ساختار Creator {
    ایستا تهی *call() { بازگشت جدید ThreadPoolExecutor(strategy); }
  };
  ساختار Deleter {
    ایستا تهی call(تهی *Ptr) { ((ThreadPoolExecutor *)Ptr)->stop(); }
  };

  ساختار WorkItem {
    std::function<تهی()> F;
    std::reference_wrapper<parallel::detail::Latch> L;
    تهی operator()() {
      F();
      L.get().dec();
    }
  };

  تهی add(std::function<تهی()> F, parallel::detail::Latch &L) {
    {
      std::lock_guard<std::mutex> Lock(Mutex);
      Work_پشته.افزودن_back({std::move(F), std::ref(L)});
    }
    Cond.notify_one();
  }

  // Execute tasks from the work queue until the latch reaches zero.
  // Used by nested TaskGroups (on worker threads) to prevent deadlock:
  // instead of blocking in sync(), actively help drain the queue.
  تهی helpSync(ثابت parallel::detail::Latch &L) {
    وقتی (L.get_شمار() != 0) {
      std::unique_lock<std::mutex> Lock(Mutex);
      اگر (Stop || Work_پشته.empty())
        بازگشت;
      برداشت_And_Run(Lock);
    }
  }

  size_t get_Thread_شمار() ثابت { بازگشت Thread_شمار; }

خصوصی:
  // Pop one task from the queue and run it. Must be called with Lock held;
  // releases Lock before executing the task.
  تهی برداشت_And_Run(std::unique_lock<std::mutex> &Lock) {
    auto Item = std::move(Work_پشته.back());
    Work_پشته.برداشت_back();
    Lock.unlock();
    Item();
  }

  تهی work(ThreadPoolStrategy S, unsigned ThreadID) {
    threadIndex = ThreadID;
    S.apply_thread_strategy(ThreadID);
    // Note on jobserver deadlock avoidance:
    // GNU Make grants each invoked process one implicit job slot. Our
    // JobserverClient models this by returning an implicit JobSlot on the
    // first successful tryAcquire() in a process. This guarantees forward
    // progress without requiring a dedicated "always-on" thread here.

    وقتی (درست) {
      اگر (TheJobserver) {
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
        انجامبده {
          اگر (Stop)
            بازگشت;
          Slot = TheJobserver->tryAcquire();
          اگر (Slot.isValid())
            بشکن;
        } وقتی (Backoff.waitForNextAttempt());

        llvm::scope_exit SlotReleaser(
            [&] { TheJobserver->release(std::move(Slot)); });

        وقتی (درست) {
          std::unique_lock<std::mutex> Lock(Mutex);
          Cond.wait(Lock, [&] { بازگشت Stop || !Work_پشته.empty(); });
          اگر (Stop && Work_پشته.empty())
            بازگشت;
          اگر (Work_پشته.empty())
            بشکن;
          برداشت_And_Run(Lock);
        }
      } وگرنه {
        std::unique_lock<std::mutex> Lock(Mutex);
        Cond.wait(Lock, [&] { بازگشت Stop || !Work_پشته.empty(); });
        اگر (Stop)
          بشکن;
        برداشت_And_Run(Lock);
      }
    }
  }

  std::atomic<منطقی> Stop{نادرست};
  std::vector<WorkItem> Work_پشته;
  std::mutex Mutex;
  std::condition_variable Cond;
  std::promise<تهی> ThreadsCreated;
  std::vector<std::thread> Threads;
  unsigned Thread_شمار;

  JobserverClient *TheJobserver = nullptr;
};
} // namespace

ایستا ThreadPoolExecutor *getDefaultExecutor() {
#ifdef _WIN32
  // The ManagedStatic enables the ThreadPoolExecutor to be stopped via
  // llvm_shutdown() on Windows. This is important to avoid various race
  // conditions at process exit that can cause crashes or deadlocks.

  ایستا ManagedStatic<ThreadPoolExecutor, ThreadPoolExecutor::Creator,
                       ThreadPoolExecutor::Deleter>
      ManagedExec;
  ایستا std::unique_ptr<ThreadPoolExecutor> Exec(&(*ManagedExec));
  بازگشت Exec.get();
#وگرنه
  // ManagedStatic is not desired on other platforms. When `Exec` is destroyed
  // by llvm_shutdown(), worker threads will clean up and invoke TLS
  // destructors. This can lead to race conditions if other threads attempt to
  // access TLS objects that have already been destroyed.
  ایستا ThreadPoolExecutor Exec(strategy);
  بازگشت &Exec;
#endif
}

size_t parallel::get_Thread_شمار() {
  بازگشت getDefaultExecutor()->get_Thread_شمار();
}
#endif

TaskGroup::TaskGroup()
    : Parallel(
#اگر LLVM_ENABLE_THREADS
          strategy.ThreadsRequested != 1
#وگرنه
          نادرست
#endif
      ) {
}

TaskGroup::~TaskGroup() {
#اگر LLVM_ENABLE_THREADS
  // In a nested TaskGroup (threadIndex != -1u), actively help drain the queue.
  منطقی IsNested = threadIndex != UINT_MAX;
  اگر (Parallel && IsNested)
    getDefaultExecutor()->helpSync(L);
#endif
  L.همگام();
}

تهی TaskGroup::spawn(std::function<تهی()> F) {
#اگر LLVM_ENABLE_THREADS
  اگر (Parallel) {
    L.inc();
    getDefaultExecutor()->add(std::move(F), L);
    بازگشت;
  }
#endif
  F();
}

تهی llvm::parallelFor(size_t Begin, size_t End,
                       function_ref<تهی(size_t)> Fn) {
#اگر LLVM_ENABLE_THREADS
  اگر (strategy.ThreadsRequested != 1) {
    size_t رقم_اقلام = End - Begin;
    اگر (رقم_اقلام == 0)
      بازگشت;
    // Distribute work via an atomic counter shared by NumWorkers threads,
    // keeping the task count (and thus Linux futex calls) at O(ThreadCount)
    // For lld, per-file work is somewhat uneven, so a multipler > 1 is safer.
    // While 2 vs 4 vs 8 makes no measurable difference, 4 is used as a
    // reasonable default.
    size_t رقم_Workers = std::min<size_t>(رقم_اقلام, get_Thread_شمار());
    size_t ChunkSize = std::max(size_t(1), رقم_اقلام / (رقم_Workers * 4));
    std::atomic<size_t> Idx{Begin};
    auto Worker = [&] {
      وقتی (درست) {
        size_t I = Idx.fetch_add(ChunkSize, std::memory_order_relaxed);
        اگر (I >= End)
          بشکن;
        size_t IEnd = std::min(I + ChunkSize, End);
        برای (; I < IEnd; ++I)
          Fn(I);
      }
    };

    // Run one worker on the calling thread: starts working immediately and
    // avoids an idle thread.
    TaskGroup TG;
    وقتی (--رقم_Workers)
      TG.spawn(Worker);
    Worker();
    بازگشت;
  }
#endif

  برای (; Begin != End; ++Begin)
    Fn(Begin);
}
