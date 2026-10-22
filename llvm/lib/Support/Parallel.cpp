//===- llvm/Support/Parallel.cpp - Parallel algorithms --------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#அந்தர்பாவ "llvm/Support/Parallel.h"
#அந்தர்பாவ "llvm/ADT/ScopeExit.h"
#அந்தர்பாவ "llvm/Config/llvm-config.h"
#அந்தர்பாவ "llvm/Support/ExponentialBackoff.h"
#அந்தர்பாவ "llvm/Support/Jobserver.h"
#அந்தர்பாவ "llvm/Support/ManagedStatic.h"
#அந்தர்பாவ "llvm/Support/Threading.h"

#அந்தர்பாவ <atomic>
#அந்தர்பாவ <future>
#அந்தர்பாவ <memory>
#அந்தர்பாவ <mutex>
#அந்தர்பாவ <thread>
#அந்தர்பாவ <vector>

using நாமாகாஶ llvm;
using நாமாகாஶ llvm::parallel;

llvm::ThreadPoolStrategy parallel::strategy;

#எனில் LLVM_ENABLE_THREADS

நிலையான thread_local unsigned threadIndex = UINT_MAX;

நாமாகாஶ {

/// Runs closures on a thread pool in filo order.
வகுப்பு ThreadPoolExecutor {
பொது:
  explicit ThreadPoolExecutor(ThreadPoolStrategy S) {
    எனில் (S.UseJobserver)
      TheJobserver = JobserverClient::getInstance();

    Thread_மொத்தம் = S.compute_thread_மொத்தம்();
    // Spawn all but one of the threads in another thread as spawning threads
    // can take a while.
    Threads.reserve(Thread_மொத்தம்);
    Threads.resize(1);
    std::lock_guard<std::mutex> Lock(Mutex);
    // Use operator[] before creating the thread to avoid data race in .size()
    // in 'safe libc++' mode.
    auto &Thread0 = Threads[0];
    Thread0 = std::thread([இது, S] {
      க்கு (unsigned I = 1; I < Thread_மொத்தம்; ++I) {
        Threads.emplace_back([இது, S, I] { work(S, I); });
        எனில் (Stop)
          நிறுத்து;
      }
      ThreadsCreated.set_மதிப்பு();
      work(S, 0);
    });
  }

  // To make sure the thread pool executor can only be created with a parallel
  // strategy.
  ThreadPoolExecutor() = delete;

  வெறுமை stop() {
    {
      std::lock_guard<std::mutex> Lock(Mutex);
      எனில் (Stop)
        திரும்பு;
      Stop = உண்மை;
    }
    Cond.notify_all();
    ThreadsCreated.get_future().wait();

    std::thread::id CurrentThreadId = std::this_thread::get_id();
    க்கு (std::thread &T : Threads)
      எனில் (T.get_id() == CurrentThreadId)
        T.detach();
      இல்லையெனில்
        T.join();
  }

  ~ThreadPoolExecutor() { stop(); }

  கட்டமைப்பு Creator {
    நிலையான வெறுமை *call() { திரும்பு புதிய ThreadPoolExecutor(strategy); }
  };
  கட்டமைப்பு Deleter {
    நிலையான வெறுமை call(வெறுமை *Ptr) { ((ThreadPoolExecutor *)Ptr)->stop(); }
  };

  கட்டமைப்பு WorkItem {
    std::function<வெறுமை()> F;
    std::reference_wrapper<parallel::detail::Latch> L;
    வெறுமை operator()() {
      F();
      L.get().dec();
    }
  };

  வெறுமை add(std::function<வெறுமை()> F, parallel::detail::Latch &L) {
    {
      std::lock_guard<std::mutex> Lock(Mutex);
      Work_அடுக்கு.தள்ளு_back({std::move(F), std::ref(L)});
    }
    Cond.notify_one();
  }

  // Execute tasks from the work queue until the latch reaches zero.
  // Used by nested TaskGroups (on worker threads) to prevent deadlock:
  // instead of blocking in sync(), actively help drain the queue.
  வெறுமை helpSync(மாறிலி parallel::detail::Latch &L) {
    போது (L.get_மொத்தம்() != 0) {
      std::unique_lock<std::mutex> Lock(Mutex);
      எனில் (Stop || Work_அடுக்கு.empty())
        திரும்பு;
      எடு_And_Run(Lock);
    }
  }

  size_t get_Thread_மொத்தம்() மாறிலி { திரும்பு Thread_மொத்தம்; }

தனிப்பட்ட:
  // Pop one task from the queue and run it. Must be called with Lock held;
  // releases Lock before executing the task.
  வெறுமை எடு_And_Run(std::unique_lock<std::mutex> &Lock) {
    auto Item = std::move(Work_அடுக்கு.back());
    Work_அடுக்கு.எடு_back();
    Lock.unlock();
    Item();
  }

  வெறுமை work(ThreadPoolStrategy S, unsigned ThreadID) {
    threadIndex = ThreadID;
    S.apply_thread_strategy(ThreadID);
    // Note on jobserver deadlock avoidance:
    // GNU Make grants each invoked process one implicit job slot. Our
    // JobserverClient models this by returning an implicit JobSlot on the
    // first successful tryAcquire() in a process. This guarantees forward
    // progress without requiring a dedicated "always-on" thread here.

    போது (உண்மை) {
      எனில் (TheJobserver) {
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
        செய் {
          எனில் (Stop)
            திரும்பு;
          Slot = TheJobserver->tryAcquire();
          எனில் (Slot.isValid())
            நிறுத்து;
        } போது (Backoff.waitForNextAttempt());

        llvm::scope_exit SlotReleaser(
            [&] { TheJobserver->release(std::move(Slot)); });

        போது (உண்மை) {
          std::unique_lock<std::mutex> Lock(Mutex);
          Cond.wait(Lock, [&] { திரும்பு Stop || !Work_அடுக்கு.empty(); });
          எனில் (Stop && Work_அடுக்கு.empty())
            திரும்பு;
          எனில் (Work_அடுக்கு.empty())
            நிறுத்து;
          எடு_And_Run(Lock);
        }
      } இல்லையெனில் {
        std::unique_lock<std::mutex> Lock(Mutex);
        Cond.wait(Lock, [&] { திரும்பு Stop || !Work_அடுக்கு.empty(); });
        எனில் (Stop)
          நிறுத்து;
        எடு_And_Run(Lock);
      }
    }
  }

  std::atomic<தருக்க> Stop{பொய்};
  std::vector<WorkItem> Work_அடுக்கு;
  std::mutex Mutex;
  std::condition_variable Cond;
  std::promise<வெறுமை> ThreadsCreated;
  std::vector<std::thread> Threads;
  unsigned Thread_மொத்தம்;

  JobserverClient *TheJobserver = nullptr;
};
} // namespace

நிலையான ThreadPoolExecutor *getDefaultExecutor() {
#ifdef _WIN32
  // The ManagedStatic enables the ThreadPoolExecutor to be stopped via
  // llvm_shutdown() on Windows. This is important to avoid various race
  // conditions at process exit that can cause crashes or deadlocks.

  நிலையான ManagedStatic<ThreadPoolExecutor, ThreadPoolExecutor::Creator,
                       ThreadPoolExecutor::Deleter>
      ManagedExec;
  நிலையான std::unique_ptr<ThreadPoolExecutor> Exec(&(*ManagedExec));
  திரும்பு Exec.get();
#இல்லையெனில்
  // ManagedStatic is not desired on other platforms. When `Exec` is destroyed
  // by llvm_shutdown(), worker threads will clean up and invoke TLS
  // destructors. This can lead to race conditions if other threads attempt to
  // access TLS objects that have already been destroyed.
  நிலையான ThreadPoolExecutor Exec(strategy);
  திரும்பு &Exec;
#endif
}

size_t parallel::get_Thread_மொத்தம்() {
  திரும்பு getDefaultExecutor()->get_Thread_மொத்தம்();
}
#endif

TaskGroup::TaskGroup()
    : Parallel(
#எனில் LLVM_ENABLE_THREADS
          strategy.ThreadsRequested != 1
#இல்லையெனில்
          பொய்
#endif
      ) {
}

TaskGroup::~TaskGroup() {
#எனில் LLVM_ENABLE_THREADS
  // In a nested TaskGroup (threadIndex != -1u), actively help drain the queue.
  தருக்க IsNested = threadIndex != UINT_MAX;
  எனில் (Parallel && IsNested)
    getDefaultExecutor()->helpSync(L);
#endif
  L.ஒத்திசை();
}

வெறுமை TaskGroup::spawn(std::function<வெறுமை()> F) {
#எனில் LLVM_ENABLE_THREADS
  எனில் (Parallel) {
    L.inc();
    getDefaultExecutor()->add(std::move(F), L);
    திரும்பு;
  }
#endif
  F();
}

வெறுமை llvm::parallelFor(size_t Begin, size_t End,
                       function_ref<வெறுமை(size_t)> Fn) {
#எனில் LLVM_ENABLE_THREADS
  எனில் (strategy.ThreadsRequested != 1) {
    size_t இலக்கம்_பொருட்கள் = End - Begin;
    எனில் (இலக்கம்_பொருட்கள் == 0)
      திரும்பு;
    // Distribute work via an atomic counter shared by NumWorkers threads,
    // keeping the task count (and thus Linux futex calls) at O(ThreadCount)
    // For lld, per-file work is somewhat uneven, so a multipler > 1 is safer.
    // While 2 vs 4 vs 8 makes no measurable difference, 4 is used as a
    // reasonable default.
    size_t இலக்கம்_Workers = std::min<size_t>(இலக்கம்_பொருட்கள், get_Thread_மொத்தம்());
    size_t ChunkSize = std::max(size_t(1), இலக்கம்_பொருட்கள் / (இலக்கம்_Workers * 4));
    std::atomic<size_t> Idx{Begin};
    auto Worker = [&] {
      போது (உண்மை) {
        size_t I = Idx.fetch_add(ChunkSize, std::memory_order_relaxed);
        எனில் (I >= End)
          நிறுத்து;
        size_t IEnd = std::min(I + ChunkSize, End);
        க்கு (; I < IEnd; ++I)
          Fn(I);
      }
    };

    // Run one worker on the calling thread: starts working immediately and
    // avoids an idle thread.
    TaskGroup TG;
    போது (--இலக்கம்_Workers)
      TG.spawn(Worker);
    Worker();
    திரும்பு;
  }
#endif

  க்கு (; Begin != End; ++Begin)
    Fn(Begin);
}
