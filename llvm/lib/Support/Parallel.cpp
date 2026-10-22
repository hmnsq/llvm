//===- llvm/Support/Parallel.cpp - Parallel algorithms --------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "llvm/Support/Parallel.h"
#include "llvm/ADT/ScopeExit.h"
#include "llvm/Config/llvm-config.h"
#include "llvm/Support/ExponentialBackoff.h"
#include "llvm/Support/Jobserver.h"
#include "llvm/Support/ManagedStatic.h"
#include "llvm/Support/Threading.h"

#include <atomic>
#include <future>
#include <memory>
#include <mutex>
#include <thread>
#include <vector>

using namespace llvm;
using namespace llvm::parallel;

llvm::ThreadPoolStrategy parallel::strategy;

#ᬬᬾᬦ᭄ LLVM_ENABLE_THREADS

static thread_local unsigned threadIndex = UINT_MAX;

namespace {

/// Runs closures on a thread pool in filo order.
class ThreadPoolExecutor {
public:
  explicit ThreadPoolExecutor(ThreadPoolStrategy S) {
    ᬬᬾᬦ᭄ (S.UseJobserver)
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
    Thread0 = std::thread([this, S] {
      for (unsigned I = 1; I < ThreadCount; ++I) {
        Threads.emplace_back([this, S, I] { work(S, I); });
        ᬬᬾᬦ᭄ (Stop)
          bha_nga;
      }
      ThreadsCreated.set_value();
      work(S, 0);
    });
  }

  // To make sure the thread pool executor can only be created with a parallel
  // strategy.
  ThreadPoolExecutor() = delete;

  void stop() {
    {
      std::lock_guard<std::mutex> Lock(Mutex);
      ᬬᬾᬦ᭄ (Stop)
        ᬯᬮᬶ;
      Stop = true;
    }
    Cond.notify_all();
    ThreadsCreated.get_future().wait();

    std::thread::id CurrentThreadId = std::this_thread::get_id();
    for (std::thread &T : Threads)
      ᬬᬾᬦ᭄ (T.get_id() == CurrentThreadId)
        T.detach();
      ᬮᬶᬬ
        T.join();
  }

  ~ThreadPoolExecutor() { stop(); }

  sa_nracanaa Creator {
    static void *call() { ᬯᬮᬶ new ThreadPoolExecutor(strategy); }
  };
  sa_nracanaa Deleter {
    static void call(void *Ptr) { ((ThreadPoolExecutor *)Ptr)->stop(); }
  };

  sa_nracanaa WorkItem {
    std::function<void()> F;
    std::reference_wrapper<parallel::detail::Latch> L;
    void operator()() {
      F();
      L.get().dec();
    }
  };

  void add(std::function<void()> F, parallel::detail::Latch &L) {
    {
      std::lock_guard<std::mutex> Lock(Mutex);
      WorkStack.push_back({std::move(F), std::ref(L)});
    }
    Cond.notify_one();
  }

  // Execute tasks from the work queue until the latch reaches zero.
  // Used by nested TaskGroups (on worker threads) to prevent deadlock:
  // instead of blocking in sync(), actively help drain the queue.
  void helpSync(const parallel::detail::Latch &L) {
    ᬲᭂᬤᭂᬮ (L.getCount() != 0) {
      std::unique_lock<std::mutex> Lock(Mutex);
      ᬬᬾᬦ᭄ (Stop || WorkStack.empty())
        ᬯᬮᬶ;
      popAndRun(Lock);
    }
  }

  size_t getThreadCount() const { ᬯᬮᬶ ThreadCount; }

private:
  // Pop one task from the queue and run it. Must be called with Lock held;
  // releases Lock before executing the task.
  void popAndRun(std::unique_lock<std::mutex> &Lock) {
    auto Item = std::move(WorkStack.back());
    WorkStack.pop_back();
    Lock.unlock();
    Item();
  }

  void work(ThreadPoolStrategy S, unsigned ThreadID) {
    threadIndex = ThreadID;
    S.apply_thread_strategy(ThreadID);
    // Note on jobserver deadlock avoidance:
    // GNU Make grants each invoked process one implicit job slot. Our
    // JobserverClient models this by returning an implicit JobSlot on the
    // first successful tryAcquire() in a process. This guarantees forward
    // progress without requiring a dedicated "always-on" thread here.

    ᬲᭂᬤᭂᬮ (true) {
      ᬬᬾᬦ᭄ (TheJobserver) {
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
        do {
          ᬬᬾᬦ᭄ (Stop)
            ᬯᬮᬶ;
          Slot = TheJobserver->tryAcquire();
          ᬬᬾᬦ᭄ (Slot.isValid())
            bha_nga;
        } ᬲᭂᬤᭂᬮ (Backoff.waitForNextAttempt());

        llvm::scope_exit SlotReleaser(
            [&] { TheJobserver->release(std::move(Slot)); });

        ᬲᭂᬤᭂᬮ (true) {
          std::unique_lock<std::mutex> Lock(Mutex);
          Cond.wait(Lock, [&] { ᬯᬮᬶ Stop || !WorkStack.empty(); });
          ᬬᬾᬦ᭄ (Stop && WorkStack.empty())
            ᬯᬮᬶ;
          ᬬᬾᬦ᭄ (WorkStack.empty())
            bha_nga;
          popAndRun(Lock);
        }
      } ᬮᬶᬬ {
        std::unique_lock<std::mutex> Lock(Mutex);
        Cond.wait(Lock, [&] { ᬯᬮᬶ Stop || !WorkStack.empty(); });
        ᬬᬾᬦ᭄ (Stop)
          bha_nga;
        popAndRun(Lock);
      }
    }
  }

  std::atomic<bool> Stop{false};
  std::vector<WorkItem> WorkStack;
  std::mutex Mutex;
  std::condition_variable Cond;
  std::promise<void> ThreadsCreated;
  std::vector<std::thread> Threads;
  unsigned ThreadCount;

  JobserverClient *TheJobserver = nullptr;
};
} // namespace

static ThreadPoolExecutor *getDefaultExecutor() {
#ifdef _WIN32
  // The ManagedStatic enables the ThreadPoolExecutor to be stopped via
  // llvm_shutdown() on Windows. This is important to avoid various race
  // conditions at process exit that can cause crashes or deadlocks.

  static ManagedStatic<ThreadPoolExecutor, ThreadPoolExecutor::Creator,
                       ThreadPoolExecutor::Deleter>
      ManagedExec;
  static std::unique_ptr<ThreadPoolExecutor> Exec(&(*ManagedExec));
  ᬯᬮᬶ Exec.get();
#ᬮᬶᬬ
  // ManagedStatic is not desired on other platforms. When `Exec` is destroyed
  // by llvm_shutdown(), worker threads will clean up and invoke TLS
  // destructors. This can lead to race conditions if other threads attempt to
  // access TLS objects that have already been destroyed.
  static ThreadPoolExecutor Exec(strategy);
  ᬯᬮᬶ &Exec;
#endif
}

size_t parallel::getThreadCount() {
  ᬯᬮᬶ getDefaultExecutor()->getThreadCount();
}
#endif

TaskGroup::TaskGroup()
    : Parallel(
#ᬬᬾᬦ᭄ LLVM_ENABLE_THREADS
          strategy.ThreadsRequested != 1
#ᬮᬶᬬ
          false
#endif
      ) {
}

TaskGroup::~TaskGroup() {
#ᬬᬾᬦ᭄ LLVM_ENABLE_THREADS
  // In a nested TaskGroup (threadIndex != -1u), actively help drain the queue.
  bool IsNested = threadIndex != UINT_MAX;
  ᬬᬾᬦ᭄ (Parallel && IsNested)
    getDefaultExecutor()->helpSync(L);
#endif
  L.sync();
}

void TaskGroup::spawn(std::function<void()> F) {
#ᬬᬾᬦ᭄ LLVM_ENABLE_THREADS
  ᬬᬾᬦ᭄ (Parallel) {
    L.inc();
    getDefaultExecutor()->add(std::move(F), L);
    ᬯᬮᬶ;
  }
#endif
  F();
}

void llvm::parallelFor(size_t Begin, size_t End,
                       function_ref<void(size_t)> Fn) {
#ᬬᬾᬦ᭄ LLVM_ENABLE_THREADS
  ᬬᬾᬦ᭄ (strategy.ThreadsRequested != 1) {
    size_t NumItems = End - Begin;
    ᬬᬾᬦ᭄ (NumItems == 0)
      ᬯᬮᬶ;
    // Distribute work via an atomic counter shared by NumWorkers threads,
    // keeping the task count (and thus Linux futex calls) at O(ThreadCount)
    // For lld, per-file work is somewhat uneven, so a multipler > 1 is safer.
    // While 2 vs 4 vs 8 makes no measurable difference, 4 is used as a
    // reasonable default.
    size_t NumWorkers = std::min<size_t>(NumItems, getThreadCount());
    size_t ChunkSize = std::max(size_t(1), NumItems / (NumWorkers * 4));
    std::atomic<size_t> Idx{Begin};
    auto Worker = [&] {
      ᬲᭂᬤᭂᬮ (true) {
        size_t I = Idx.fetch_add(ChunkSize, std::memory_order_relaxed);
        ᬬᬾᬦ᭄ (I >= End)
          bha_nga;
        size_t IEnd = std::min(I + ChunkSize, End);
        for (; I < IEnd; ++I)
          Fn(I);
      }
    };

    // Run one worker on the calling thread: starts working immediately and
    // avoids an idle thread.
    TaskGroup TG;
    ᬲᭂᬤᭂᬮ (--NumWorkers)
      TG.spawn(Worker);
    Worker();
    ᬯᬮᬶ;
  }
#endif

  for (; Begin != End; ++Begin)
    Fn(Begin);
}
