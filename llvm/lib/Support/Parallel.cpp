//===- llvm/Support/Parallel.cpp - Parallel algorithms --------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#અન્તર્ભાવ "llvm/Support/Parallel.h"
#અન્તર્ભાવ "llvm/ADT/ScopeExit.h"
#અન્તર્ભાવ "llvm/Config/llvm-config.h"
#અન્તર્ભાવ "llvm/Support/ExponentialBackoff.h"
#અન્તર્ભાવ "llvm/Support/Jobserver.h"
#અન્તર્ભાવ "llvm/Support/ManagedStatic.h"
#અન્તર્ભાવ "llvm/Support/Threading.h"

#અન્તર્ભાવ <atomic>
#અન્તર્ભાવ <future>
#અન્તર્ભાવ <memory>
#અન્તર્ભાવ <mutex>
#અન્તર્ભાવ <thread>
#અન્તર્ભાવ <vector>

using નામાકાશ llvm;
using નામાકાશ llvm::parallel;

llvm::ThreadPoolStrategy parallel::strategy;

#જો LLVM_ENABLE_THREADS

સ્થિર thread_local unsigned threadIndex = UINT_MAX;

નામાકાશ {

/// Runs closures on a thread pool in filo order.
વર્ગ ThreadPoolExecutor {
જાહેર:
  explicit ThreadPoolExecutor(ThreadPoolStrategy S) {
    જો (S.UseJobserver)
      TheJobserver = JobserverClient::getInstance();

    Thread_ગણતરી = S.compute_thread_ગણતરી();
    // Spawn all but one of the threads in another thread as spawning threads
    // can take a while.
    Threads.reserve(Thread_ગણતરી);
    Threads.resize(1);
    std::lock_guard<std::mutex> Lock(Mutex);
    // Use operator[] before creating the thread to avoid data race in .size()
    // in 'safe libc++' mode.
    auto &Thread0 = Threads[0];
    Thread0 = std::thread([આ, S] {
      માટે (unsigned I = 1; I < Thread_ગણતરી; ++I) {
        Threads.emplace_back([આ, S, I] { work(S, I); });
        જો (Stop)
          તોડો;
      }
      ThreadsCreated.set_કિંમત();
      work(S, 0);
    });
  }

  // To make sure the thread pool executor can only be created with a parallel
  // strategy.
  ThreadPoolExecutor() = delete;

  શૂન્ય stop() {
    {
      std::lock_guard<std::mutex> Lock(Mutex);
      જો (Stop)
        પાછા;
      Stop = સાચું;
    }
    Cond.notify_all();
    ThreadsCreated.get_future().wait();

    std::thread::id CurrentThreadId = std::this_thread::get_id();
    માટે (std::thread &T : Threads)
      જો (T.get_id() == CurrentThreadId)
        T.detach();
      નહીંતર
        T.join();
  }

  ~ThreadPoolExecutor() { stop(); }

  સંરચના Creator {
    સ્થિર શૂન્ય *call() { પાછા નવું ThreadPoolExecutor(strategy); }
  };
  સંરચના Deleter {
    સ્થિર શૂન્ય call(શૂન્ય *Ptr) { ((ThreadPoolExecutor *)Ptr)->stop(); }
  };

  સંરચના WorkItem {
    std::function<શૂન્ય()> F;
    std::reference_wrapper<parallel::detail::Latch> L;
    શૂન્ય operator()() {
      F();
      L.get().dec();
    }
  };

  શૂન્ય add(std::function<શૂન્ય()> F, parallel::detail::Latch &L) {
    {
      std::lock_guard<std::mutex> Lock(Mutex);
      Work_ઢગલો.ધકેલો_back({std::move(F), std::ref(L)});
    }
    Cond.notify_one();
  }

  // Execute tasks from the work queue until the latch reaches zero.
  // Used by nested TaskGroups (on worker threads) to prevent deadlock:
  // instead of blocking in sync(), actively help drain the queue.
  શૂન્ય helpSync(અચલ parallel::detail::Latch &L) {
    જ્યારે (L.get_ગણતરી() != 0) {
      std::unique_lock<std::mutex> Lock(Mutex);
      જો (Stop || Work_ઢગલો.empty())
        પાછા;
      કાઢો_And_Run(Lock);
    }
  }

  size_t get_Thread_ગણતરી() અચલ { પાછા Thread_ગણતરી; }

ખાનગી:
  // Pop one task from the queue and run it. Must be called with Lock held;
  // releases Lock before executing the task.
  શૂન્ય કાઢો_And_Run(std::unique_lock<std::mutex> &Lock) {
    auto Item = std::move(Work_ઢગલો.back());
    Work_ઢગલો.કાઢો_back();
    Lock.unlock();
    Item();
  }

  શૂન્ય work(ThreadPoolStrategy S, unsigned ThreadID) {
    threadIndex = ThreadID;
    S.apply_thread_strategy(ThreadID);
    // Note on jobserver deadlock avoidance:
    // GNU Make grants each invoked process one implicit job slot. Our
    // JobserverClient models this by returning an implicit JobSlot on the
    // first successful tryAcquire() in a process. This guarantees forward
    // progress without requiring a dedicated "always-on" thread here.

    જ્યારે (સાચું) {
      જો (TheJobserver) {
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
        કરો {
          જો (Stop)
            પાછા;
          Slot = TheJobserver->tryAcquire();
          જો (Slot.isValid())
            તોડો;
        } જ્યારે (Backoff.waitForNextAttempt());

        llvm::scope_exit SlotReleaser(
            [&] { TheJobserver->release(std::move(Slot)); });

        જ્યારે (સાચું) {
          std::unique_lock<std::mutex> Lock(Mutex);
          Cond.wait(Lock, [&] { પાછા Stop || !Work_ઢગલો.empty(); });
          જો (Stop && Work_ઢગલો.empty())
            પાછા;
          જો (Work_ઢગલો.empty())
            તોડો;
          કાઢો_And_Run(Lock);
        }
      } નહીંતર {
        std::unique_lock<std::mutex> Lock(Mutex);
        Cond.wait(Lock, [&] { પાછા Stop || !Work_ઢગલો.empty(); });
        જો (Stop)
          તોડો;
        કાઢો_And_Run(Lock);
      }
    }
  }

  std::atomic<તાર્કિક> Stop{ખોટું};
  std::vector<WorkItem> Work_ઢગલો;
  std::mutex Mutex;
  std::condition_variable Cond;
  std::promise<શૂન્ય> ThreadsCreated;
  std::vector<std::thread> Threads;
  unsigned Thread_ગણતરી;

  JobserverClient *TheJobserver = nullptr;
};
} // namespace

સ્થિર ThreadPoolExecutor *getDefaultExecutor() {
#ifdef _WIN32
  // The ManagedStatic enables the ThreadPoolExecutor to be stopped via
  // llvm_shutdown() on Windows. This is important to avoid various race
  // conditions at process exit that can cause crashes or deadlocks.

  સ્થિર ManagedStatic<ThreadPoolExecutor, ThreadPoolExecutor::Creator,
                       ThreadPoolExecutor::Deleter>
      ManagedExec;
  સ્થિર std::unique_ptr<ThreadPoolExecutor> Exec(&(*ManagedExec));
  પાછા Exec.get();
#નહીંતર
  // ManagedStatic is not desired on other platforms. When `Exec` is destroyed
  // by llvm_shutdown(), worker threads will clean up and invoke TLS
  // destructors. This can lead to race conditions if other threads attempt to
  // access TLS objects that have already been destroyed.
  સ્થિર ThreadPoolExecutor Exec(strategy);
  પાછા &Exec;
#endif
}

size_t parallel::get_Thread_ગણતરી() {
  પાછા getDefaultExecutor()->get_Thread_ગણતરી();
}
#endif

TaskGroup::TaskGroup()
    : Parallel(
#જો LLVM_ENABLE_THREADS
          strategy.ThreadsRequested != 1
#નહીંતર
          ખોટું
#endif
      ) {
}

TaskGroup::~TaskGroup() {
#જો LLVM_ENABLE_THREADS
  // In a nested TaskGroup (threadIndex != -1u), actively help drain the queue.
  તાર્કિક IsNested = threadIndex != UINT_MAX;
  જો (Parallel && IsNested)
    getDefaultExecutor()->helpSync(L);
#endif
  L.સમકાલિક();
}

શૂન્ય TaskGroup::spawn(std::function<શૂન્ય()> F) {
#જો LLVM_ENABLE_THREADS
  જો (Parallel) {
    L.inc();
    getDefaultExecutor()->add(std::move(F), L);
    પાછા;
  }
#endif
  F();
}

શૂન્ય llvm::parallelFor(size_t Begin, size_t End,
                       function_ref<શૂન્ય(size_t)> Fn) {
#જો LLVM_ENABLE_THREADS
  જો (strategy.ThreadsRequested != 1) {
    size_t અંક_વસ્તુઓ = End - Begin;
    જો (અંક_વસ્તુઓ == 0)
      પાછા;
    // Distribute work via an atomic counter shared by NumWorkers threads,
    // keeping the task count (and thus Linux futex calls) at O(ThreadCount)
    // For lld, per-file work is somewhat uneven, so a multipler > 1 is safer.
    // While 2 vs 4 vs 8 makes no measurable difference, 4 is used as a
    // reasonable default.
    size_t અંક_Workers = std::min<size_t>(અંક_વસ્તુઓ, get_Thread_ગણતરી());
    size_t ChunkSize = std::max(size_t(1), અંક_વસ્તુઓ / (અંક_Workers * 4));
    std::atomic<size_t> Idx{Begin};
    auto Worker = [&] {
      જ્યારે (સાચું) {
        size_t I = Idx.fetch_add(ChunkSize, std::memory_order_relaxed);
        જો (I >= End)
          તોડો;
        size_t IEnd = std::min(I + ChunkSize, End);
        માટે (; I < IEnd; ++I)
          Fn(I);
      }
    };

    // Run one worker on the calling thread: starts working immediately and
    // avoids an idle thread.
    TaskGroup TG;
    જ્યારે (--અંક_Workers)
      TG.spawn(Worker);
    Worker();
    પાછા;
  }
#endif

  માટે (; Begin != End; ++Begin)
    Fn(Begin);
}
