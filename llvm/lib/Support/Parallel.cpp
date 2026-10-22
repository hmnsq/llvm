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

استعمال نطاق llvm;
استعمال نطاق llvm::parallel;

llvm::ThreadPoolStrategy parallel::strategy;

#إذا LLVM_ENABLE_ڌاڳا

ساكن thread_local بنانشان thread_اشاريو = UINT_MAX;

نطاق {

/// Runs closures on a thread pool in filo order.
صنف ThreadPoolExecutor {
عام:
  واضح ThreadPoolExecutor(ThreadPoolStrategy S) {
    إذا (S.UseJobserver)
      TheJobserver = JobserverClient::getInstance();

    Thread_ڳڻپ = S.compute_thread_ڳڻپ();
    // Spawn all but one of the threads in another thread as spawning threads
    // can take a while.
    ڌاڳا.reserve(Thread_ڳڻپ);
    ڌاڳا.resize(1);
    std::تالو_guard<std::mutex> تالو_2(Mutex);
    // Use operator[] before creating the thread to avoid data race in .size()
    // in 'safe libc++' mode.
    خودڪار &Thread0 = ڌاڳا[0];
    Thread0 = std::thread([هذا, S] {
      لكل (بنانشان I = 1; I < Thread_ڳڻپ; ++I) {
        ڌاڳا.emplace_پٺ([هذا, S, I] { work(S, I); });
        إذا (Stop)
          اكسر;
      }
      ڌاڳا_Created.سيٽ_قدر();
      work(S, 0);
    });
  }

  // To make sure the thread pool executor can only be created with a parallel
  // strategy.
  ThreadPoolExecutor() = ميٽيو;

  فراغ stop() {
    {
      std::تالو_guard<std::mutex> تالو_2(Mutex);
      إذا (Stop)
        أرجع;
      Stop = سچ;
    }
    Cond.notify_all();
    ڌاڳا_Created.get_future().wait();

    std::thread::id CurrentThreadId = std::this_thread::get_id();
    لكل (std::thread &T : ڌاڳا)
      إذا (T.get_id() == CurrentThreadId)
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

  بنية Work_شي {
    std::function<فراغ()> F;
    std::reference_wrapper<parallel::detail::Latch> L;
    فراغ عامل()() {
      F();
      L.get().dec();
    }
  };

  فراغ جوڙيو(std::function<فراغ()> F, parallel::detail::Latch &L) {
    {
      std::تالو_guard<std::mutex> تالو_2(Mutex);
      Work_ڍير.ڌڪو_پٺ({std::move(F), std::ref(L)});
    }
    Cond.notify_one();
  }

  // Execute tasks from the work queue until the latch reaches zero.
  // Used by nested TaskGroups (on worker threads) to prevent deadlock:
  // instead of blocking in sync(), actively help drain the queue.
  فراغ helpSync(ثابت parallel::detail::Latch &L) {
    طالما (L.get_ڳڻپ() != 0) {
      std::unique_تالو<std::mutex> تالو_2(Mutex);
      إذا (Stop || Work_ڍير.خالي_جايون())
        أرجع;
      ڪڍو_And_لاڳيتو(تالو_2);
    }
  }

  size_t get_Thread_ڳڻپ() ثابت { أرجع Thread_ڳڻپ; }

خاص:
  // Pop one task from the queue and run it. Must be called with Lock held;
  // releases Lock before executing the task.
  فراغ ڪڍو_And_لاڳيتو(std::unique_تالو<std::mutex> &تالو_2) {
    خودڪار شي = std::move(Work_ڍير.پٺ());
    Work_ڍير.ڪڍو_پٺ();
    تالو_2.unlock();
    شي();
  }

  فراغ work(ThreadPoolStrategy S, بنانشان ThreadID) {
    thread_اشاريو = ThreadID;
    S.apply_thread_strategy(ThreadID);
    // Note on jobserver deadlock avoidance:
    // GNU Make grants each invoked process one implicit job slot. Our
    // JobserverClient models this by returning an implicit JobSlot on the
    // first successful tryAcquire() in a process. This guarantees forward
    // progress without requiring a dedicated "always-on" thread here.

    طالما (سچ) {
      إذا (TheJobserver) {
        // Jobserver-mode scheduling:
        // - Acquire one job slot (with exponential backoff to avoid busy-wait).
        // - While holding the slot, drain and run tasks from the local queue.
        // - Release the slot when the queue is empty or when shutting down.
        // Rationale: Holding a slot amortizes acquire/release overhead over
        // multiple tasks and avoids requeue/yield churn, while still enforcing
        // the jobserver’s global concurrency limit. With K available slots,
        // up to K workers run tasks in parallel; within each worker tasks run
        // sequentially until the local queue is empty.
        قوت_نمائي_Backoff Backoff(std::chrono::hours(24));
        ڪم_جڳهه جڳهه;
        افعل {
          إذا (Stop)
            أرجع;
          جڳهه = TheJobserver->tryAcquire();
          إذا (جڳهه.isValid())
            اكسر;
        } طالما (Backoff.wait_For_ايندڙ_Attempt());

        llvm::scope_exit جڳهه_Releaser(
            [&] { TheJobserver->release(std::move(جڳهه)); });

        طالما (سچ) {
          std::unique_تالو<std::mutex> تالو_2(Mutex);
          Cond.wait(تالو_2, [&] { أرجع Stop || !Work_ڍير.خالي_جايون(); });
          إذا (Stop && Work_ڍير.خالي_جايون())
            أرجع;
          إذا (Work_ڍير.خالي_جايون())
            اكسر;
          ڪڍو_And_لاڳيتو(تالو_2);
        }
      } وإلا {
        std::unique_تالو<std::mutex> تالو_2(Mutex);
        Cond.wait(تالو_2, [&] { أرجع Stop || !Work_ڍير.خالي_جايون(); });
        إذا (Stop)
          اكسر;
        ڪڍو_And_لاڳيتو(تالو_2);
      }
    }
  }

  std::atomic<منطقي> Stop{خطأ};
  std::ويڪٽر<Work_شي> Work_ڍير;
  std::mutex Mutex;
  std::condition_variable Cond;
  std::promise<فراغ> ڌاڳا_Created;
  std::ويڪٽر<std::thread> ڌاڳا;
  بنانشان Thread_ڳڻپ;

  JobserverClient *TheJobserver = خالي_اشارو;
};
} // namespace

ساكن ThreadPoolExecutor *getDefaultExecutor() {
#جيڪڏهن_وصف_ٿيل _WIN32
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
#جيڪڏهن_پورو
}

size_t parallel::get_Thread_ڳڻپ() {
  أرجع getDefaultExecutor()->get_Thread_ڳڻپ();
}
#جيڪڏهن_پورو

Task_گروهه::Task_گروهه()
    : Parallel(
#إذا LLVM_ENABLE_ڌاڳا
          strategy.ڌاڳا_Requested != 1
#وإلا
          خطأ
#جيڪڏهن_پورو
      ) {
}

Task_گروهه::~Task_گروهه() {
#إذا LLVM_ENABLE_ڌاڳا
  // In a nested TaskGroup (threadIndex != -1u), actively help drain the queue.
  منطقي IsNested = thread_اشاريو != UINT_MAX;
  إذا (Parallel && IsNested)
    getDefaultExecutor()->helpSync(L);
#جيڪڏهن_پورو
  L.متزامن();
}

فراغ Task_گروهه::spawn(std::function<فراغ()> F) {
#إذا LLVM_ENABLE_ڌاڳا
  إذا (Parallel) {
    L.inc();
    getDefaultExecutor()->جوڙيو(std::move(F), L);
    أرجع;
  }
#جيڪڏهن_پورو
  F();
}

فراغ llvm::parallelFor(size_t Begin, size_t پڇاڙي_2,
                       function_ref<فراغ(size_t)> Fn) {
#إذا LLVM_ENABLE_ڌاڳا
  إذا (strategy.ڌاڳا_Requested != 1) {
    size_t انگ_شيون = پڇاڙي_2 - Begin;
    إذا (انگ_شيون == 0)
      أرجع;
    // Distribute work via an atomic counter shared by NumWorkers threads,
    // keeping the task count (and thus Linux futex calls) at O(ThreadCount)
    // For lld, per-file work is somewhat uneven, so a multipler > 1 is safer.
    // While 2 vs 4 vs 8 makes no measurable difference, 4 is used as a
    // reasonable default.
    size_t انگ_Workers = std::min<size_t>(انگ_شيون, get_Thread_ڳڻپ());
    size_t Chunk_سائيز = std::max(size_t(1), انگ_شيون / (انگ_Workers * 4));
    std::atomic<size_t> Idx{Begin};
    خودڪار ڪم_ڪندڙ = [&] {
      طالما (سچ) {
        size_t I = Idx.fetch_جوڙيو(Chunk_سائيز, std::memory_order_relaxed);
        إذا (I >= پڇاڙي_2)
          اكسر;
        size_t IEnd = std::min(I + Chunk_سائيز, پڇاڙي_2);
        لكل (; I < IEnd; ++I)
          Fn(I);
      }
    };

    // Run one worker on the calling thread: starts working immediately and
    // avoids an idle thread.
    Task_گروهه TG;
    طالما (--انگ_Workers)
      TG.spawn(ڪم_ڪندڙ);
    ڪم_ڪندڙ();
    أرجع;
  }
#جيڪڏهن_پورو

  لكل (; Begin != پڇاڙي_2; ++Begin)
    Fn(Begin);
}
