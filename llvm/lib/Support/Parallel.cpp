//===- llvm/Support/Parallel.cpp - Parallel algorithms --------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#অন্তর্ভাব "llvm/Support/Parallel.h"
#অন্তর্ভাব "llvm/ADT/ScopeExit.h"
#অন্তর্ভাব "llvm/Config/llvm-config.h"
#অন্তর্ভাব "llvm/Support/ExponentialBackoff.h"
#অন্তর্ভাব "llvm/Support/Jobserver.h"
#অন্তর্ভাব "llvm/Support/ManagedStatic.h"
#অন্তর্ভাব "llvm/Support/Threading.h"

#অন্তর্ভাব <atomic>
#অন্তর্ভাব <future>
#অন্তর্ভাব <memory>
#অন্তর্ভাব <mutex>
#অন্তর্ভাব <thread>
#অন্তর্ভাব <vector>

using নামাকাশ llvm;
using নামাকাশ llvm::parallel;

llvm::ThreadPoolStrategy parallel::strategy;

#যদি LLVM_ENABLE_THREADS

স্থির thread_local unsigned thread_সূচক = UINT_MAX;

নামাকাশ {

/// Runs closures on a thread pool in filo order.
শ্রেণী ThreadPoolExecutor {
সর্বজনীন:
  explicit ThreadPoolExecutor(ThreadPoolStrategy S) {
    যদি (S.UseJobserver)
      TheJobserver = JobserverClient::getInstance();

    Thread_গুনতি = S.compute_thread_গুনতি();
    // Spawn all but one of the threads in another thread as spawning threads
    // can take a while.
    Threads.reserve(Thread_গুনতি);
    Threads.resize(1);
    std::lock_guard<std::mutex> Lock(Mutex);
    // Use operator[] before creating the thread to avoid data race in .size()
    // in 'safe libc++' mode.
    auto &Thread0 = Threads[0];
    Thread0 = std::thread([এই, S] {
      জন্য (unsigned I = 1; I < Thread_গুনতি; ++I) {
        Threads.emplace_back([এই, S, I] { work(S, I); });
        যদি (Stop)
          বিরতি;
      }
      ThreadsCreated.set_মান();
      work(S, 0);
    });
  }

  // To make sure the thread pool executor can only be created with a parallel
  // strategy.
  ThreadPoolExecutor() = delete;

  শূন্য stop() {
    {
      std::lock_guard<std::mutex> Lock(Mutex);
      যদি (Stop)
        ফেরত;
      Stop = সত্য;
    }
    Cond.notify_all();
    ThreadsCreated.get_future().wait();

    std::thread::id CurrentThreadId = std::this_thread::get_id();
    জন্য (std::thread &T : Threads)
      যদি (T.get_id() == CurrentThreadId)
        T.detach();
      অন্যথা
        T.join();
  }

  ~ThreadPoolExecutor() { stop(); }

  কাঠামো Creator {
    স্থির শূন্য *call() { ফেরত নতুন ThreadPoolExecutor(strategy); }
  };
  কাঠামো Deleter {
    স্থির শূন্য call(শূন্য *Ptr) { ((ThreadPoolExecutor *)Ptr)->stop(); }
  };

  কাঠামো WorkItem {
    std::function<শূন্য()> F;
    std::reference_wrapper<parallel::detail::Latch> L;
    শূন্য operator()() {
      F();
      L.get().dec();
    }
  };

  শূন্য add(std::function<শূন্য()> F, parallel::detail::Latch &L) {
    {
      std::lock_guard<std::mutex> Lock(Mutex);
      Work_স্তূপ.ঠেলো_back({std::move(F), std::ref(L)});
    }
    Cond.notify_one();
  }

  // Execute tasks from the work queue until the latch reaches zero.
  // Used by nested TaskGroups (on worker threads) to prevent deadlock:
  // instead of blocking in sync(), actively help drain the queue.
  শূন্য helpSync(ধ্রুবক parallel::detail::Latch &L) {
    যখন (L.get_গুনতি() != 0) {
      std::unique_lock<std::mutex> Lock(Mutex);
      যদি (Stop || Work_স্তূপ.empty())
        ফেরত;
      বের_করো_And_Run(Lock);
    }
  }

  size_t get_Thread_গুনতি() ধ্রুবক { ফেরত Thread_গুনতি; }

ব্যক্তিগত:
  // Pop one task from the queue and run it. Must be called with Lock held;
  // releases Lock before executing the task.
  শূন্য বের_করো_And_Run(std::unique_lock<std::mutex> &Lock) {
    auto Item = std::move(Work_স্তূপ.back());
    Work_স্তূপ.বের_করো_back();
    Lock.unlock();
    Item();
  }

  শূন্য work(ThreadPoolStrategy S, unsigned ThreadID) {
    thread_সূচক = ThreadID;
    S.apply_thread_strategy(ThreadID);
    // Note on jobserver deadlock avoidance:
    // GNU Make grants each invoked process one implicit job slot. Our
    // JobserverClient models this by returning an implicit JobSlot on the
    // first successful tryAcquire() in a process. This guarantees forward
    // progress without requiring a dedicated "always-on" thread here.

    যখন (সত্য) {
      যদি (TheJobserver) {
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
        করো {
          যদি (Stop)
            ফেরত;
          Slot = TheJobserver->tryAcquire();
          যদি (Slot.isValid())
            বিরতি;
        } যখন (Backoff.wait_For_পরবর্তী_Attempt());

        llvm::scope_exit SlotReleaser(
            [&] { TheJobserver->release(std::move(Slot)); });

        যখন (সত্য) {
          std::unique_lock<std::mutex> Lock(Mutex);
          Cond.wait(Lock, [&] { ফেরত Stop || !Work_স্তূপ.empty(); });
          যদি (Stop && Work_স্তূপ.empty())
            ফেরত;
          যদি (Work_স্তূপ.empty())
            বিরতি;
          বের_করো_And_Run(Lock);
        }
      } অন্যথা {
        std::unique_lock<std::mutex> Lock(Mutex);
        Cond.wait(Lock, [&] { ফেরত Stop || !Work_স্তূপ.empty(); });
        যদি (Stop)
          বিরতি;
        বের_করো_And_Run(Lock);
      }
    }
  }

  std::atomic<যৌক্তিক> Stop{মিথ্যা};
  std::vector<WorkItem> Work_স্তূপ;
  std::mutex Mutex;
  std::condition_variable Cond;
  std::promise<শূন্য> ThreadsCreated;
  std::vector<std::thread> Threads;
  unsigned Thread_গুনতি;

  JobserverClient *TheJobserver = nullptr;
};
} // namespace

স্থির ThreadPoolExecutor *getDefaultExecutor() {
#ifdef _WIN32
  // The ManagedStatic enables the ThreadPoolExecutor to be stopped via
  // llvm_shutdown() on Windows. This is important to avoid various race
  // conditions at process exit that can cause crashes or deadlocks.

  স্থির ManagedStatic<ThreadPoolExecutor, ThreadPoolExecutor::Creator,
                       ThreadPoolExecutor::Deleter>
      ManagedExec;
  স্থির std::unique_ptr<ThreadPoolExecutor> Exec(&(*ManagedExec));
  ফেরত Exec.get();
#অন্যথা
  // ManagedStatic is not desired on other platforms. When `Exec` is destroyed
  // by llvm_shutdown(), worker threads will clean up and invoke TLS
  // destructors. This can lead to race conditions if other threads attempt to
  // access TLS objects that have already been destroyed.
  স্থির ThreadPoolExecutor Exec(strategy);
  ফেরত &Exec;
#endif
}

size_t parallel::get_Thread_গুনতি() {
  ফেরত getDefaultExecutor()->get_Thread_গুনতি();
}
#endif

TaskGroup::TaskGroup()
    : Parallel(
#যদি LLVM_ENABLE_THREADS
          strategy.ThreadsRequested != 1
#অন্যথা
          মিথ্যা
#endif
      ) {
}

TaskGroup::~TaskGroup() {
#যদি LLVM_ENABLE_THREADS
  // In a nested TaskGroup (threadIndex != -1u), actively help drain the queue.
  যৌক্তিক IsNested = thread_সূচক != UINT_MAX;
  যদি (Parallel && IsNested)
    getDefaultExecutor()->helpSync(L);
#endif
  L.সমলয়();
}

শূন্য TaskGroup::spawn(std::function<শূন্য()> F) {
#যদি LLVM_ENABLE_THREADS
  যদি (Parallel) {
    L.inc();
    getDefaultExecutor()->add(std::move(F), L);
    ফেরত;
  }
#endif
  F();
}

শূন্য llvm::parallelFor(size_t Begin, size_t End,
                       function_ref<শূন্য(size_t)> Fn) {
#যদি LLVM_ENABLE_THREADS
  যদি (strategy.ThreadsRequested != 1) {
    size_t অঙ্ক_বস্তুসমূহ = End - Begin;
    যদি (অঙ্ক_বস্তুসমূহ == 0)
      ফেরত;
    // Distribute work via an atomic counter shared by NumWorkers threads,
    // keeping the task count (and thus Linux futex calls) at O(ThreadCount)
    // For lld, per-file work is somewhat uneven, so a multipler > 1 is safer.
    // While 2 vs 4 vs 8 makes no measurable difference, 4 is used as a
    // reasonable default.
    size_t অঙ্ক_Workers = std::min<size_t>(অঙ্ক_বস্তুসমূহ, get_Thread_গুনতি());
    size_t Chunk_আকার = std::max(size_t(1), অঙ্ক_বস্তুসমূহ / (অঙ্ক_Workers * 4));
    std::atomic<size_t> Idx{Begin};
    auto Worker = [&] {
      যখন (সত্য) {
        size_t I = Idx.fetch_add(Chunk_আকার, std::memory_order_relaxed);
        যদি (I >= End)
          বিরতি;
        size_t IEnd = std::min(I + Chunk_আকার, End);
        জন্য (; I < IEnd; ++I)
          Fn(I);
      }
    };

    // Run one worker on the calling thread: starts working immediately and
    // avoids an idle thread.
    TaskGroup TG;
    যখন (--অঙ্ক_Workers)
      TG.spawn(Worker);
    Worker();
    ফেরত;
  }
#endif

  জন্য (; Begin != End; ++Begin)
    Fn(Begin);
}
