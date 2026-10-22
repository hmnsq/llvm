//===- llvm/Support/Parallel.cpp - Parallel algorithms --------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#समावेश "llvm/Support/Parallel.h"
#समावेश "llvm/ADT/ScopeExit.h"
#समावेश "llvm/Config/llvm-config.h"
#समावेश "llvm/Support/ExponentialBackoff.h"
#समावेश "llvm/Support/Jobserver.h"
#समावेश "llvm/Support/ManagedStatic.h"
#समावेश "llvm/Support/Threading.h"

#समावेश <atomic>
#समावेश <future>
#समावेश <memory>
#समावेश <mutex>
#समावेश <thread>
#समावेश <vector>

using namespace llvm;
using namespace llvm::parallel;

llvm::ThreadPoolStrategy parallel::strategy;

#अगर LLVM_ENABLE_THREADS

अटल thread_local अचिन्हित thread_सूचकांक = अच_पूर्णांक_उच्च;

namespace {

/// Runs closures on a thread pool in filo order.
श्रेणी ThreadPoolExecutor {
खुला:
  explicit ThreadPoolExecutor(ThreadPoolStrategy S) {
    अगर (S.UseJobserver)
      TheJobserver = JobserverClient::getInstance();

    Thread_गिनती = S.compute_thread_गिनती();
    // Spawn all but one of the threads in another thread as spawning threads
    // can take a while.
    Threads.reserve(Thread_गिनती);
    Threads.resize(1);
    std::lock_guard<std::mutex> Lock(Mutex);
    // Use operator[] before creating the thread to avoid data race in .size()
    // in 'safe libc++' mode.
    स्वतः &Thread0 = Threads[0];
    Thread0 = std::thread([यह, S] {
      क्रम (अचिन्हित I = 1; I < Thread_गिनती; ++I) {
        Threads.emplace_back([यह, S, I] { work(S, I); });
        अगर (Stop)
          अवरोध;
      }
      ThreadsCreated.set_मूल्य();
      work(S, 0);
    });
  }

  // To make sure the thread pool executor can only be created with a parallel
  // strategy.
  ThreadPoolExecutor() = मिटाओ;

  व्योम stop() {
    {
      std::lock_guard<std::mutex> Lock(Mutex);
      अगर (Stop)
        वापस;
      Stop = true;
    }
    Cond.notify_all();
    ThreadsCreated.get_future().रुको();

    std::thread::id CurrentThreadId = std::this_thread::get_id();
    क्रम (std::thread &T : Threads)
      अगर (T.get_id() == CurrentThreadId)
        T.detach();
      अन्यथा
        T.join();
  }

  ~ThreadPoolExecutor() { stop(); }

  काष्ठा Creator {
    अटल व्योम *call() { वापस नया ThreadPoolExecutor(strategy); }
  };
  काष्ठा Deleter {
    अटल व्योम call(व्योम *Ptr) { ((ThreadPoolExecutor *)Ptr)->stop(); }
  };

  काष्ठा WorkItem {
    std::function<व्योम()> F;
    std::reference_wrapper<parallel::detail::Latch> L;
    व्योम चालक()() {
      F();
      L.लो().डेसी();
    }
  };

  व्योम add(std::function<व्योम()> F, parallel::detail::Latch &L) {
    {
      std::lock_guard<std::mutex> Lock(Mutex);
      Work_ढेर.धकेलो_back({std::move(F), std::ref(L)});
    }
    Cond.notify_one();
  }

  // Execute tasks from the work queue until the latch reaches zero.
  // Used by nested TaskGroups (on worker threads) to prevent deadlock:
  // instead of blocking in sync(), actively help drain the queue.
  व्योम helpSync(स्थिर parallel::detail::Latch &L) {
    जबतक (L.get_गिनती() != 0) {
      std::unique_lock<std::mutex> Lock(Mutex);
      अगर (Stop || Work_ढेर.empty())
        वापस;
      निकालो_And_Run(Lock);
    }
  }

  माप_प्रकार get_Thread_गिनती() स्थिर { वापस Thread_गिनती; }

निजी:
  // Pop one task from the queue and run it. Must be called with Lock held;
  // releases Lock before executing the task.
  व्योम निकालो_And_Run(std::unique_lock<std::mutex> &Lock) {
    स्वतः Item = std::move(Work_ढेर.back());
    Work_ढेर.निकालो_back();
    Lock.unlock();
    Item();
  }

  व्योम work(ThreadPoolStrategy S, अचिन्हित ThreadID) {
    thread_सूचकांक = ThreadID;
    S.apply_thread_strategy(ThreadID);
    // Note on jobserver deadlock avoidance:
    // GNU Make grants each invoked process one implicit job slot. Our
    // JobserverClient models this by returning an implicit JobSlot on the
    // first successful tryAcquire() in a process. This guarantees forward
    // progress without requiring a dedicated "always-on" thread here.

    जबतक (true) {
      अगर (TheJobserver) {
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
        करो {
          अगर (Stop)
            वापस;
          Slot = TheJobserver->tryAcquire();
          अगर (Slot.isValid())
            अवरोध;
        } जबतक (Backoff.wait_For_अग्र_Attempt());

        llvm::scope_exit SlotReleaser(
            [&] { TheJobserver->release(std::move(Slot)); });

        जबतक (true) {
          std::unique_lock<std::mutex> Lock(Mutex);
          Cond.रुको(Lock, [&] { वापस Stop || !Work_ढेर.empty(); });
          अगर (Stop && Work_ढेर.empty())
            वापस;
          अगर (Work_ढेर.empty())
            अवरोध;
          निकालो_And_Run(Lock);
        }
      } अन्यथा {
        std::unique_lock<std::mutex> Lock(Mutex);
        Cond.रुको(Lock, [&] { वापस Stop || !Work_ढेर.empty(); });
        अगर (Stop)
          अवरोध;
        निकालो_And_Run(Lock);
      }
    }
  }

  std::atomic<bool> Stop{false};
  std::vector<WorkItem> Work_ढेर;
  std::mutex Mutex;
  std::condition_variable Cond;
  std::promise<व्योम> ThreadsCreated;
  std::vector<std::thread> Threads;
  अचिन्हित Thread_गिनती;

  JobserverClient *TheJobserver = nullptr;
};
} // namespace

अटल ThreadPoolExecutor *getDefaultExecutor() {
#अगर_घोषित _WIN32
  // The ManagedStatic enables the ThreadPoolExecutor to be stopped via
  // llvm_shutdown() on Windows. This is important to avoid various race
  // conditions at process exit that can cause crashes or deadlocks.

  अटल ManagedStatic<ThreadPoolExecutor, ThreadPoolExecutor::Creator,
                       ThreadPoolExecutor::Deleter>
      ManagedExec;
  अटल std::unique_ptr<ThreadPoolExecutor> Exec(&(*ManagedExec));
  वापस Exec.लो();
#अन्यथा
  // ManagedStatic is not desired on other platforms. When `Exec` is destroyed
  // by llvm_shutdown(), worker threads will clean up and invoke TLS
  // destructors. This can lead to race conditions if other threads attempt to
  // access TLS objects that have already been destroyed.
  अटल ThreadPoolExecutor Exec(strategy);
  वापस &Exec;
#पूर्ण_अगर
}

माप_प्रकार parallel::get_Thread_गिनती() {
  वापस getDefaultExecutor()->get_Thread_गिनती();
}
#पूर्ण_अगर

TaskGroup::TaskGroup()
    : Parallel(
#अगर LLVM_ENABLE_THREADS
          strategy.ThreadsRequested != 1
#अन्यथा
          false
#पूर्ण_अगर
      ) {
}

TaskGroup::~TaskGroup() {
#अगर LLVM_ENABLE_THREADS
  // In a nested TaskGroup (threadIndex != -1u), actively help drain the queue.
  bool IsNested = thread_सूचकांक != अच_पूर्णांक_उच्च;
  अगर (Parallel && IsNested)
    getDefaultExecutor()->helpSync(L);
#पूर्ण_अगर
  L.sync();
}

व्योम TaskGroup::spawn(std::function<व्योम()> F) {
#अगर LLVM_ENABLE_THREADS
  अगर (Parallel) {
    L.inc();
    getDefaultExecutor()->add(std::move(F), L);
    वापस;
  }
#पूर्ण_अगर
  F();
}

व्योम llvm::parallelFor(माप_प्रकार Begin, माप_प्रकार End,
                       function_ref<व्योम(माप_प्रकार)> Fn) {
#अगर LLVM_ENABLE_THREADS
  अगर (strategy.ThreadsRequested != 1) {
    माप_प्रकार अंक_वस्तुएँ = End - Begin;
    अगर (अंक_वस्तुएँ == 0)
      वापस;
    // Distribute work via an atomic counter shared by NumWorkers threads,
    // keeping the task count (and thus Linux futex calls) at O(ThreadCount)
    // For lld, per-file work is somewhat uneven, so a multipler > 1 is safer.
    // While 2 vs 4 vs 8 makes no measurable difference, 4 is used as a
    // reasonable default.
    माप_प्रकार अंक_Workers = std::min<माप_प्रकार>(अंक_वस्तुएँ, get_Thread_गिनती());
    माप_प्रकार Chunk_आकार = std::max(माप_प्रकार(1), अंक_वस्तुएँ / (अंक_Workers * 4));
    std::atomic<माप_प्रकार> Idx{Begin};
    स्वतः Worker = [&] {
      जबतक (true) {
        माप_प्रकार I = Idx.fetch_add(Chunk_आकार, std::memory_order_relaxed);
        अगर (I >= End)
          अवरोध;
        माप_प्रकार IEnd = std::min(I + Chunk_आकार, End);
        क्रम (; I < IEnd; ++I)
          Fn(I);
      }
    };

    // Run one worker on the calling thread: starts working immediately and
    // avoids an idle thread.
    TaskGroup TG;
    जबतक (--अंक_Workers)
      TG.spawn(Worker);
    Worker();
    वापस;
  }
#पूर्ण_अगर

  क्रम (; Begin != End; ++Begin)
    Fn(Begin);
}
