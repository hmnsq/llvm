//===- llvm/Support/Parallel.cpp - Parallel algorithms --------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ضمن "llvm/Support/Parallel.h"
#ضمن "llvm/ADT/ScopeExit.h"
#ضمن "llvm/Config/llvm-config.h"
#ضمن "llvm/Support/ExponentialBackoff.h"
#ضمن "llvm/Support/Jobserver.h"
#ضمن "llvm/Support/ManagedStatic.h"
#ضمن "llvm/Support/Threading.h"

#ضمن <atomic>
#ضمن <future>
#ضمن <memory>
#ضمن <mutex>
#ضمن <thread>
#ضمن <vector>

using نطاق llvm;
using نطاق llvm::parallel;

llvm::ThreadPoolStrategy parallel::strategy;

#𐡄𐡍 LLVM_ENABLE_THREADS

ساكن thread_local unsigned threadIndex = UINT_MAX;

نطاق {

/// Runs closures on a thread pool in filo order.
صنف ThreadPoolExecutor {
عام:
  explicit ThreadPoolExecutor(ThreadPoolStrategy S) {
    𐡄𐡍 (S.UseJobserver)
      TheJobserver = JobserverClient::getInstance();

    ThreadCount = S.compute_thread_count();
    // Spawn all but one of the threads in another thread as spawning threads
    // can take a while.
    Threads.reserve(ThreadCount);
    Threads.resize(1);
    std::lock_guard<std::mutex> Lock(Mutex);
    // Use operator[] before creating the thread to avoid data race in .size()
    // in 'safe libc++' mode.
    auto &Thread0 = Threads[0];
    Thread0 = std::thread([هذا, S] {
      لكل (unsigned I = 1; I < ThreadCount; ++I) {
        Threads.emplace_back([هذا, S, I] { work(S, I); });
        𐡄𐡍 (Stop)
          اكسر;
      }
      ThreadsCreated.set_value();
      work(S, 0);
    });
  }

  // To make sure the thread pool executor can only be created with a parallel
  // strategy.
  ThreadPoolExecutor() = delete;

  فراغ stop() {
    {
      std::lock_guard<std::mutex> Lock(Mutex);
      𐡄𐡍 (Stop)
        أرجع;
      Stop = 𐡒𐡔𐡈;
    }
    Cond.notify_all();
    ThreadsCreated.get_future().wait();

    std::thread::id CurrentThreadId = std::this_thread::get_id();
    لكل (std::thread &T : Threads)
      𐡄𐡍 (T.get_id() == CurrentThreadId)
        T.detach();
      وإلا
        T.join();
  }

  ~ThreadPoolExecutor() { stop(); }

  بنية Creator {
    ساكن فراغ *call() { أرجع جديد ThreadPoolExecutor(strategy); }
  };
  بنية Deleter {
    ساكن فراغ call(فراغ *Ptr) { ((ThreadPoolExecutor *)Ptr)->stop(); }
  };

  بنية WorkItem {
    std::function<فراغ()> F;
    std::reference_wrapper<parallel::detail::Latch> L;
    فراغ operator()() {
      F();
      L.get().dec();
    }
  };

  فراغ add(std::function<فراغ()> F, parallel::detail::Latch &L) {
    {
      std::lock_guard<std::mutex> Lock(Mutex);
      WorkStack.push_back({std::move(F), std::ref(L)});
    }
    Cond.notify_one();
  }

  // Execute tasks from the work queue until the latch reaches zero.
  // Used by nested TaskGroups (on worker threads) to prevent deadlock:
  // instead of blocking in sync(), actively help drain the queue.
  فراغ helpSync(ثابت parallel::detail::Latch &L) {
    طالما (L.getCount() != 0) {
      std::unique_lock<std::mutex> Lock(Mutex);
      𐡄𐡍 (Stop || WorkStack.empty())
        أرجع;
      popAndRun(Lock);
    }
  }

  size_t getThreadCount() ثابت { أرجع ThreadCount; }

خاص:
  // Pop one task from the queue and run it. Must be called with Lock held;
  // releases Lock before executing the task.
  فراغ popAndRun(std::unique_lock<std::mutex> &Lock) {
    auto Item = std::move(WorkStack.back());
    WorkStack.pop_back();
    Lock.unlock();
    Item();
  }

  فراغ work(ThreadPoolStrategy S, unsigned ThreadID) {
    threadIndex = ThreadID;
    S.apply_thread_strategy(ThreadID);
    // Note on jobserver deadlock avoidance:
    // GNU Make grants each invoked process one implicit job slot. Our
    // JobserverClient models this by returning an implicit JobSlot on the
    // first successful tryAcquire() in a process. This guarantees forward
    // progress without requiring a dedicated "always-on" thread here.

    طالما (𐡒𐡔𐡈) {
      𐡄𐡍 (TheJobserver) {
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
        𐡏𐡁𐡃 {
          𐡄𐡍 (Stop)
            أرجع;
          Slot = TheJobserver->tryAcquire();
          𐡄𐡍 (Slot.isValid())
            اكسر;
        } طالما (Backoff.waitForNextAttempt());

        llvm::scope_exit SlotReleaser(
            [&] { TheJobserver->release(std::move(Slot)); });

        طالما (𐡒𐡔𐡈) {
          std::unique_lock<std::mutex> Lock(Mutex);
          Cond.wait(Lock, [&] { أرجع Stop || !WorkStack.empty(); });
          𐡄𐡍 (Stop && WorkStack.empty())
            أرجع;
          𐡄𐡍 (WorkStack.empty())
            اكسر;
          popAndRun(Lock);
        }
      } وإلا {
        std::unique_lock<std::mutex> Lock(Mutex);
        Cond.wait(Lock, [&] { أرجع Stop || !WorkStack.empty(); });
        𐡄𐡍 (Stop)
          اكسر;
        popAndRun(Lock);
      }
    }
  }

  std::atomic<منطقي> Stop{خطأ};
  std::vector<WorkItem> WorkStack;
  std::mutex Mutex;
  std::condition_variable Cond;
  std::promise<فراغ> ThreadsCreated;
  std::vector<std::thread> Threads;
  unsigned ThreadCount;

  JobserverClient *TheJobserver = nullptr;
};
} // namespace

ساكن ThreadPoolExecutor *getDefaultExecutor() {
#ifdef _WIN32
  // The ManagedStatic enables the ThreadPoolExecutor to be stopped via
  // llvm_shutdown() on Windows. This is important to avoid various race
  // conditions at process exit that can cause crashes or deadlocks.

  ساكن ManagedStatic<ThreadPoolExecutor, ThreadPoolExecutor::Creator,
                       ThreadPoolExecutor::Deleter>
      ManagedExec;
  ساكن std::unique_ptr<ThreadPoolExecutor> Exec(&(*ManagedExec));
  أرجع Exec.get();
#وإلا
  // ManagedStatic is not desired on other platforms. When `Exec` is destroyed
  // by llvm_shutdown(), worker threads will clean up and invoke TLS
  // destructors. This can lead to race conditions if other threads attempt to
  // access TLS objects that have already been destroyed.
  ساكن ThreadPoolExecutor Exec(strategy);
  أرجع &Exec;
#endif
}

size_t parallel::getThreadCount() {
  أرجع getDefaultExecutor()->getThreadCount();
}
#endif

TaskGroup::TaskGroup()
    : Parallel(
#𐡄𐡍 LLVM_ENABLE_THREADS
          strategy.ThreadsRequested != 1
#وإلا
          خطأ
#endif
      ) {
}

TaskGroup::~TaskGroup() {
#𐡄𐡍 LLVM_ENABLE_THREADS
  // In a nested TaskGroup (threadIndex != -1u), actively help drain the queue.
  منطقي IsNested = threadIndex != UINT_MAX;
  𐡄𐡍 (Parallel && IsNested)
    getDefaultExecutor()->helpSync(L);
#endif
  L.متزامن();
}

فراغ TaskGroup::spawn(std::function<فراغ()> F) {
#𐡄𐡍 LLVM_ENABLE_THREADS
  𐡄𐡍 (Parallel) {
    L.inc();
    getDefaultExecutor()->add(std::move(F), L);
    أرجع;
  }
#endif
  F();
}

فراغ llvm::parallelFor(size_t Begin, size_t End,
                       function_ref<فراغ(size_t)> Fn) {
#𐡄𐡍 LLVM_ENABLE_THREADS
  𐡄𐡍 (strategy.ThreadsRequested != 1) {
    size_t NumItems = End - Begin;
    𐡄𐡍 (NumItems == 0)
      أرجع;
    // Distribute work via an atomic counter shared by NumWorkers threads,
    // keeping the task count (and thus Linux futex calls) at O(ThreadCount)
    // For lld, per-file work is somewhat uneven, so a multipler > 1 is safer.
    // While 2 vs 4 vs 8 makes no measurable difference, 4 is used as a
    // reasonable default.
    size_t NumWorkers = std::min<size_t>(NumItems, getThreadCount());
    size_t ChunkSize = std::max(size_t(1), NumItems / (NumWorkers * 4));
    std::atomic<size_t> Idx{Begin};
    auto Worker = [&] {
      طالما (𐡒𐡔𐡈) {
        size_t I = Idx.fetch_add(ChunkSize, std::memory_order_relaxed);
        𐡄𐡍 (I >= End)
          اكسر;
        size_t IEnd = std::min(I + ChunkSize, End);
        لكل (; I < IEnd; ++I)
          Fn(I);
      }
    };

    // Run one worker on the calling thread: starts working immediately and
    // avoids an idle thread.
    TaskGroup TG;
    طالما (--NumWorkers)
      TG.spawn(Worker);
    Worker();
    أرجع;
  }
#endif

  لكل (; Begin != End; ++Begin)
    Fn(Begin);
}
