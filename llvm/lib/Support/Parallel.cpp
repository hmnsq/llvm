//===- llvm/Support/Parallel.cpp - Parallel algorithms --------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#अन्तर्भाव "llvm/Support/Parallel.h"
#अन्तर्भाव "llvm/ADT/ScopeExit.h"
#अन्तर्भाव "llvm/Config/llvm-config.h"
#अन्तर्भाव "llvm/Support/ExponentialBackoff.h"
#अन्तर्भाव "llvm/Support/Jobserver.h"
#अन्तर्भाव "llvm/Support/ManagedStatic.h"
#अन्तर्भाव "llvm/Support/Threading.h"

#अन्तर्भाव <atomic>
#अन्तर्भाव <future>
#अन्तर्भाव <memory>
#अन्तर्भाव <mutex>
#अन्तर्भाव <thread>
#अन्तर्भाव <vector>

using नामाकाश llvm;
using नामाकाश llvm::parallel;

llvm::ThreadPoolStrategy parallel::strategy;

#जर LLVM_ENABLE_THREADS

स्थिर thread_local अचिन्हित threadIndex = अच_पूर्णांक_उच्च;

नामाकाश {

/// Runs closures on a thread pool in filo order.
वर्ग ThreadPoolExecutor {
सार्वजनिक:
  explicit ThreadPoolExecutor(ThreadPoolStrategy S) {
    जर (S.UseJobserver)
      TheJobserver = JobserverClient::getInstance();

    Thread_मोजणी = S.compute_thread_मोजणी();
    // Spawn all but one of the threads in another thread as spawning threads
    // can take a while.
    Threads.reserve(Thread_मोजणी);
    Threads.resize(1);
    std::lock_guard<std::mutex> Lock(Mutex);
    // Use operator[] before creating the thread to avoid data race in .size()
    // in 'safe libc++' mode.
    स्वतः &Thread0 = Threads[0];
    Thread0 = std::thread([हे, S] {
      साठी (अचिन्हित I = 1; I < Thread_मोजणी; ++I) {
        Threads.emplace_back([हे, S, I] { work(S, I); });
        जर (Stop)
          थांबा;
      }
      ThreadsCreated.set_मूल्य();
      work(S, 0);
    });
  }

  // To make sure the thread pool executor can only be created with a parallel
  // strategy.
  ThreadPoolExecutor() = delete;

  शून्य stop() {
    {
      std::lock_guard<std::mutex> Lock(Mutex);
      जर (Stop)
        परत;
      Stop = सत्य;
    }
    Cond.notify_all();
    ThreadsCreated.get_future().रुको();

    std::thread::id CurrentThreadId = std::this_thread::get_id();
    साठी (std::thread &T : Threads)
      जर (T.get_id() == CurrentThreadId)
        T.detach();
      अन्यथा
        T.join();
  }

  ~ThreadPoolExecutor() { stop(); }

  संरचना Creator {
    स्थिर शून्य *call() { परत नवीन ThreadPoolExecutor(strategy); }
  };
  संरचना Deleter {
    स्थिर शून्य call(शून्य *Ptr) { ((ThreadPoolExecutor *)Ptr)->stop(); }
  };

  संरचना WorkItem {
    std::function<शून्य()> F;
    std::reference_wrapper<parallel::detail::Latch> L;
    शून्य चालक()() {
      F();
      L.get().dec();
    }
  };

  शून्य add(std::function<शून्य()> F, parallel::detail::Latch &L) {
    {
      std::lock_guard<std::mutex> Lock(Mutex);
      Work_चळत.ढकला_back({std::move(F), std::ref(L)});
    }
    Cond.notify_one();
  }

  // Execute tasks from the work queue until the latch reaches zero.
  // Used by nested TaskGroups (on worker threads) to prevent deadlock:
  // instead of blocking in sync(), actively help drain the queue.
  शून्य helpSync(स्थिरांक parallel::detail::Latch &L) {
    पर्यंत (L.get_मोजणी() != 0) {
      std::unique_lock<std::mutex> Lock(Mutex);
      जर (Stop || Work_चळत.empty())
        परत;
      काढा_And_Run(Lock);
    }
  }

  माप_प्रकार get_Thread_मोजणी() स्थिरांक { परत Thread_मोजणी; }

खाजगी:
  // Pop one task from the queue and run it. Must be called with Lock held;
  // releases Lock before executing the task.
  शून्य काढा_And_Run(std::unique_lock<std::mutex> &Lock) {
    स्वतः Item = std::move(Work_चळत.back());
    Work_चळत.काढा_back();
    Lock.unlock();
    Item();
  }

  शून्य work(ThreadPoolStrategy S, अचिन्हित ThreadID) {
    threadIndex = ThreadID;
    S.apply_thread_strategy(ThreadID);
    // Note on jobserver deadlock avoidance:
    // GNU Make grants each invoked process one implicit job slot. Our
    // JobserverClient models this by returning an implicit JobSlot on the
    // first successful tryAcquire() in a process. This guarantees forward
    // progress without requiring a dedicated "always-on" thread here.

    पर्यंत (सत्य) {
      जर (TheJobserver) {
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
        करा {
          जर (Stop)
            परत;
          Slot = TheJobserver->tryAcquire();
          जर (Slot.isValid())
            थांबा;
        } पर्यंत (Backoff.waitForNextAttempt());

        llvm::scope_exit SlotReleaser(
            [&] { TheJobserver->release(std::move(Slot)); });

        पर्यंत (सत्य) {
          std::unique_lock<std::mutex> Lock(Mutex);
          Cond.रुको(Lock, [&] { परत Stop || !Work_चळत.empty(); });
          जर (Stop && Work_चळत.empty())
            परत;
          जर (Work_चळत.empty())
            थांबा;
          काढा_And_Run(Lock);
        }
      } अन्यथा {
        std::unique_lock<std::mutex> Lock(Mutex);
        Cond.रुको(Lock, [&] { परत Stop || !Work_चळत.empty(); });
        जर (Stop)
          थांबा;
        काढा_And_Run(Lock);
      }
    }
  }

  std::atomic<तार्किक> Stop{असत्य};
  std::vector<WorkItem> Work_चळत;
  std::mutex Mutex;
  std::condition_variable Cond;
  std::promise<शून्य> ThreadsCreated;
  std::vector<std::thread> Threads;
  अचिन्हित Thread_मोजणी;

  JobserverClient *TheJobserver = nullptr;
};
} // namespace

स्थिर ThreadPoolExecutor *getDefaultExecutor() {
#ifdef _WIN32
  // The ManagedStatic enables the ThreadPoolExecutor to be stopped via
  // llvm_shutdown() on Windows. This is important to avoid various race
  // conditions at process exit that can cause crashes or deadlocks.

  स्थिर ManagedStatic<ThreadPoolExecutor, ThreadPoolExecutor::Creator,
                       ThreadPoolExecutor::Deleter>
      ManagedExec;
  स्थिर std::unique_ptr<ThreadPoolExecutor> Exec(&(*ManagedExec));
  परत Exec.get();
#अन्यथा
  // ManagedStatic is not desired on other platforms. When `Exec` is destroyed
  // by llvm_shutdown(), worker threads will clean up and invoke TLS
  // destructors. This can lead to race conditions if other threads attempt to
  // access TLS objects that have already been destroyed.
  स्थिर ThreadPoolExecutor Exec(strategy);
  परत &Exec;
#endif
}

माप_प्रकार parallel::get_Thread_मोजणी() {
  परत getDefaultExecutor()->get_Thread_मोजणी();
}
#endif

TaskGroup::TaskGroup()
    : Parallel(
#जर LLVM_ENABLE_THREADS
          strategy.ThreadsRequested != 1
#अन्यथा
          असत्य
#endif
      ) {
}

TaskGroup::~TaskGroup() {
#जर LLVM_ENABLE_THREADS
  // In a nested TaskGroup (threadIndex != -1u), actively help drain the queue.
  तार्किक IsNested = threadIndex != अच_पूर्णांक_उच्च;
  जर (Parallel && IsNested)
    getDefaultExecutor()->helpSync(L);
#endif
  L.समकालिक();
}

शून्य TaskGroup::spawn(std::function<शून्य()> F) {
#जर LLVM_ENABLE_THREADS
  जर (Parallel) {
    L.inc();
    getDefaultExecutor()->add(std::move(F), L);
    परत;
  }
#endif
  F();
}

शून्य llvm::parallelFor(माप_प्रकार Begin, माप_प्रकार End,
                       function_ref<शून्य(माप_प्रकार)> Fn) {
#जर LLVM_ENABLE_THREADS
  जर (strategy.ThreadsRequested != 1) {
    माप_प्रकार अंक_वस्तुसंच = End - Begin;
    जर (अंक_वस्तुसंच == 0)
      परत;
    // Distribute work via an atomic counter shared by NumWorkers threads,
    // keeping the task count (and thus Linux futex calls) at O(ThreadCount)
    // For lld, per-file work is somewhat uneven, so a multipler > 1 is safer.
    // While 2 vs 4 vs 8 makes no measurable difference, 4 is used as a
    // reasonable default.
    माप_प्रकार अंक_Workers = std::min<माप_प्रकार>(अंक_वस्तुसंच, get_Thread_मोजणी());
    माप_प्रकार ChunkSize = std::max(माप_प्रकार(1), अंक_वस्तुसंच / (अंक_Workers * 4));
    std::atomic<माप_प्रकार> Idx{Begin};
    स्वतः Worker = [&] {
      पर्यंत (सत्य) {
        माप_प्रकार I = Idx.fetch_add(ChunkSize, std::memory_order_relaxed);
        जर (I >= End)
          थांबा;
        माप_प्रकार IEnd = std::min(I + ChunkSize, End);
        साठी (; I < IEnd; ++I)
          Fn(I);
      }
    };

    // Run one worker on the calling thread: starts working immediately and
    // avoids an idle thread.
    TaskGroup TG;
    पर्यंत (--अंक_Workers)
      TG.spawn(Worker);
    Worker();
    परत;
  }
#endif

  साठी (; Begin != End; ++Begin)
    Fn(Begin);
}
