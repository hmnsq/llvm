//===- llvm/Support/Parallel.cpp - Parallel algorithms --------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ଅନ୍ତର୍ଭାଵ "llvm/Support/Parallel.h"
#ଅନ୍ତର୍ଭାଵ "llvm/ADT/ScopeExit.h"
#ଅନ୍ତର୍ଭାଵ "llvm/Config/llvm-config.h"
#ଅନ୍ତର୍ଭାଵ "llvm/Support/ExponentialBackoff.h"
#ଅନ୍ତର୍ଭାଵ "llvm/Support/Jobserver.h"
#ଅନ୍ତର୍ଭାଵ "llvm/Support/ManagedStatic.h"
#ଅନ୍ତର୍ଭାଵ "llvm/Support/Threading.h"

#ଅନ୍ତର୍ଭାଵ <atomic>
#ଅନ୍ତର୍ଭାଵ <future>
#ଅନ୍ତର୍ଭାଵ <memory>
#ଅନ୍ତର୍ଭାଵ <mutex>
#ଅନ୍ତର୍ଭାଵ <thread>
#ଅନ୍ତର୍ଭାଵ <vector>

using ନାମାକାଶ llvm;
using ନାମାକାଶ llvm::parallel;

llvm::ThreadPoolStrategy parallel::strategy;

#ଯଦି LLVM_ENABLE_THREADS

ସ୍ଥିର thread_local unsigned threadIndex = UINT_MAX;

ନାମାକାଶ {

/// Runs closures on a thread pool in filo order.
ଶ୍ରେଣୀ ThreadPoolExecutor {
ସର୍ବସାଧାରଣ:
  explicit ThreadPoolExecutor(ThreadPoolStrategy S) {
    ଯଦି (S.UseJobserver)
      TheJobserver = JobserverClient::getInstance();

    Thread_ଗଣନା = S.compute_thread_ଗଣନା();
    // Spawn all but one of the threads in another thread as spawning threads
    // can take a while.
    Threads.reserve(Thread_ଗଣନା);
    Threads.resize(1);
    std::lock_guard<std::mutex> Lock(Mutex);
    // Use operator[] before creating the thread to avoid data race in .size()
    // in 'safe libc++' mode.
    auto &Thread0 = Threads[0];
    Thread0 = std::thread([ଏହି, S] {
      ପାଇଁ (unsigned I = 1; I < Thread_ଗଣନା; ++I) {
        Threads.emplace_back([ଏହି, S, I] { work(S, I); });
        ଯଦି (Stop)
          ଭାଙ୍ଗ;
      }
      ThreadsCreated.set_ମୂଲ୍ୟ();
      work(S, 0);
    });
  }

  // To make sure the thread pool executor can only be created with a parallel
  // strategy.
  ThreadPoolExecutor() = delete;

  ଶୂନ୍ୟ stop() {
    {
      std::lock_guard<std::mutex> Lock(Mutex);
      ଯଦି (Stop)
        ଫେରନ୍ତୁ;
      Stop = ସତ୍ୟ;
    }
    Cond.notify_all();
    ThreadsCreated.get_future().wait();

    std::thread::id CurrentThreadId = std::this_thread::get_id();
    ପାଇଁ (std::thread &T : Threads)
      ଯଦି (T.get_id() == CurrentThreadId)
        T.detach();
      ଅନ୍ୟଥା
        T.join();
  }

  ~ThreadPoolExecutor() { stop(); }

  ସଂରଚନା Creator {
    ସ୍ଥିର ଶୂନ୍ୟ *call() { ଫେରନ୍ତୁ ନୂତନ ThreadPoolExecutor(strategy); }
  };
  ସଂରଚନା Deleter {
    ସ୍ଥିର ଶୂନ୍ୟ call(ଶୂନ୍ୟ *Ptr) { ((ThreadPoolExecutor *)Ptr)->stop(); }
  };

  ସଂରଚନା WorkItem {
    std::function<ଶୂନ୍ୟ()> F;
    std::reference_wrapper<parallel::detail::Latch> L;
    ଶୂନ୍ୟ operator()() {
      F();
      L.get().dec();
    }
  };

  ଶୂନ୍ୟ add(std::function<ଶୂନ୍ୟ()> F, parallel::detail::Latch &L) {
    {
      std::lock_guard<std::mutex> Lock(Mutex);
      Work_ଗଦା.ଠେଲ_back({std::move(F), std::ref(L)});
    }
    Cond.notify_one();
  }

  // Execute tasks from the work queue until the latch reaches zero.
  // Used by nested TaskGroups (on worker threads) to prevent deadlock:
  // instead of blocking in sync(), actively help drain the queue.
  ଶୂନ୍ୟ helpSync(ଅଚଳ parallel::detail::Latch &L) {
    ଯେତେବେଳେ (L.get_ଗଣନା() != 0) {
      std::unique_lock<std::mutex> Lock(Mutex);
      ଯଦି (Stop || Work_ଗଦା.empty())
        ଫେରନ୍ତୁ;
      କାଢ଼_And_Run(Lock);
    }
  }

  size_t get_Thread_ଗଣନା() ଅଚଳ { ଫେରନ୍ତୁ Thread_ଗଣନା; }

ବ୍ୟକ୍ତିଗତ:
  // Pop one task from the queue and run it. Must be called with Lock held;
  // releases Lock before executing the task.
  ଶୂନ୍ୟ କାଢ଼_And_Run(std::unique_lock<std::mutex> &Lock) {
    auto Item = std::move(Work_ଗଦା.back());
    Work_ଗଦା.କାଢ଼_back();
    Lock.unlock();
    Item();
  }

  ଶୂନ୍ୟ work(ThreadPoolStrategy S, unsigned ThreadID) {
    threadIndex = ThreadID;
    S.apply_thread_strategy(ThreadID);
    // Note on jobserver deadlock avoidance:
    // GNU Make grants each invoked process one implicit job slot. Our
    // JobserverClient models this by returning an implicit JobSlot on the
    // first successful tryAcquire() in a process. This guarantees forward
    // progress without requiring a dedicated "always-on" thread here.

    ଯେତେବେଳେ (ସତ୍ୟ) {
      ଯଦି (TheJobserver) {
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
        କର {
          ଯଦି (Stop)
            ଫେରନ୍ତୁ;
          Slot = TheJobserver->tryAcquire();
          ଯଦି (Slot.isValid())
            ଭାଙ୍ଗ;
        } ଯେତେବେଳେ (Backoff.waitForNextAttempt());

        llvm::scope_exit SlotReleaser(
            [&] { TheJobserver->release(std::move(Slot)); });

        ଯେତେବେଳେ (ସତ୍ୟ) {
          std::unique_lock<std::mutex> Lock(Mutex);
          Cond.wait(Lock, [&] { ଫେରନ୍ତୁ Stop || !Work_ଗଦା.empty(); });
          ଯଦି (Stop && Work_ଗଦା.empty())
            ଫେରନ୍ତୁ;
          ଯଦି (Work_ଗଦା.empty())
            ଭାଙ୍ଗ;
          କାଢ଼_And_Run(Lock);
        }
      } ଅନ୍ୟଥା {
        std::unique_lock<std::mutex> Lock(Mutex);
        Cond.wait(Lock, [&] { ଫେରନ୍ତୁ Stop || !Work_ଗଦା.empty(); });
        ଯଦି (Stop)
          ଭାଙ୍ଗ;
        କାଢ଼_And_Run(Lock);
      }
    }
  }

  std::atomic<ତାର୍କିକ> Stop{ମିଥ୍ୟା};
  std::vector<WorkItem> Work_ଗଦା;
  std::mutex Mutex;
  std::condition_variable Cond;
  std::promise<ଶୂନ୍ୟ> ThreadsCreated;
  std::vector<std::thread> Threads;
  unsigned Thread_ଗଣନା;

  JobserverClient *TheJobserver = nullptr;
};
} // namespace

ସ୍ଥିର ThreadPoolExecutor *getDefaultExecutor() {
#ifdef _WIN32
  // The ManagedStatic enables the ThreadPoolExecutor to be stopped via
  // llvm_shutdown() on Windows. This is important to avoid various race
  // conditions at process exit that can cause crashes or deadlocks.

  ସ୍ଥିର ManagedStatic<ThreadPoolExecutor, ThreadPoolExecutor::Creator,
                       ThreadPoolExecutor::Deleter>
      ManagedExec;
  ସ୍ଥିର std::unique_ptr<ThreadPoolExecutor> Exec(&(*ManagedExec));
  ଫେରନ୍ତୁ Exec.get();
#ଅନ୍ୟଥା
  // ManagedStatic is not desired on other platforms. When `Exec` is destroyed
  // by llvm_shutdown(), worker threads will clean up and invoke TLS
  // destructors. This can lead to race conditions if other threads attempt to
  // access TLS objects that have already been destroyed.
  ସ୍ଥିର ThreadPoolExecutor Exec(strategy);
  ଫେରନ୍ତୁ &Exec;
#endif
}

size_t parallel::get_Thread_ଗଣନା() {
  ଫେରନ୍ତୁ getDefaultExecutor()->get_Thread_ଗଣନା();
}
#endif

TaskGroup::TaskGroup()
    : Parallel(
#ଯଦି LLVM_ENABLE_THREADS
          strategy.ThreadsRequested != 1
#ଅନ୍ୟଥା
          ମିଥ୍ୟା
#endif
      ) {
}

TaskGroup::~TaskGroup() {
#ଯଦି LLVM_ENABLE_THREADS
  // In a nested TaskGroup (threadIndex != -1u), actively help drain the queue.
  ତାର୍କିକ IsNested = threadIndex != UINT_MAX;
  ଯଦି (Parallel && IsNested)
    getDefaultExecutor()->helpSync(L);
#endif
  L.ସମକାଳ();
}

ଶୂନ୍ୟ TaskGroup::spawn(std::function<ଶୂନ୍ୟ()> F) {
#ଯଦି LLVM_ENABLE_THREADS
  ଯଦି (Parallel) {
    L.inc();
    getDefaultExecutor()->add(std::move(F), L);
    ଫେରନ୍ତୁ;
  }
#endif
  F();
}

ଶୂନ୍ୟ llvm::parallelFor(size_t Begin, size_t End,
                       function_ref<ଶୂନ୍ୟ(size_t)> Fn) {
#ଯଦି LLVM_ENABLE_THREADS
  ଯଦି (strategy.ThreadsRequested != 1) {
    size_t ଅଙ୍କ_ବସ୍ତୁଗୁଡ଼ିକ = End - Begin;
    ଯଦି (ଅଙ୍କ_ବସ୍ତୁଗୁଡ଼ିକ == 0)
      ଫେରନ୍ତୁ;
    // Distribute work via an atomic counter shared by NumWorkers threads,
    // keeping the task count (and thus Linux futex calls) at O(ThreadCount)
    // For lld, per-file work is somewhat uneven, so a multipler > 1 is safer.
    // While 2 vs 4 vs 8 makes no measurable difference, 4 is used as a
    // reasonable default.
    size_t ଅଙ୍କ_Workers = std::min<size_t>(ଅଙ୍କ_ବସ୍ତୁଗୁଡ଼ିକ, get_Thread_ଗଣନା());
    size_t ChunkSize = std::max(size_t(1), ଅଙ୍କ_ବସ୍ତୁଗୁଡ଼ିକ / (ଅଙ୍କ_Workers * 4));
    std::atomic<size_t> Idx{Begin};
    auto Worker = [&] {
      ଯେତେବେଳେ (ସତ୍ୟ) {
        size_t I = Idx.fetch_add(ChunkSize, std::memory_order_relaxed);
        ଯଦି (I >= End)
          ଭାଙ୍ଗ;
        size_t IEnd = std::min(I + ChunkSize, End);
        ପାଇଁ (; I < IEnd; ++I)
          Fn(I);
      }
    };

    // Run one worker on the calling thread: starts working immediately and
    // avoids an idle thread.
    TaskGroup TG;
    ଯେତେବେଳେ (--ଅଙ୍କ_Workers)
      TG.spawn(Worker);
    Worker();
    ଫେରନ୍ତୁ;
  }
#endif

  ପାଇଁ (; Begin != End; ++Begin)
    Fn(Begin);
}
