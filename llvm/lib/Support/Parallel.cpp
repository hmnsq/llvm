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

کارول نطاق llvm;
کارول نطاق llvm::parallel;

llvm::ThreadPoolStrategy parallel::strategy;

#إذا LLVM_ENABLE_تارونه

ساكن thread_local بینښه thread_شاخص = UINT_MAX;

نطاق {

/// Runs closures on a thread pool in filo order.
صنف ThreadPoolExecutor {
عام:
  څرګند ThreadPoolExecutor(ThreadPoolStrategy S) {
    إذا (S.UseJobserver)
      TheJobserver = JobserverClient::getInstance();

    Thread_شمېر = S.compute_thread_شمېر();
    // Spawn all but one of the threads in another thread as spawning threads
    // can take a while.
    تارونه.reserve(Thread_شمېر);
    تارونه.resize(1);
    std::قلف_guard<std::mutex> قلف_2(Mutex);
    // Use operator[] before creating the thread to avoid data race in .size()
    // in 'safe libc++' mode.
    خپلکاری &Thread0 = تارونه[0];
    Thread0 = std::thread([هذا, S] {
      لكل (بینښه I = 1; I < Thread_شمېر; ++I) {
        تارونه.emplace_شا([هذا, S, I] { work(S, I); });
        إذا (Stop)
          اكسر;
      }
      تارونه_Created.سېټ_ارزښت();
      work(S, 0);
    });
  }

  // To make sure the thread pool executor can only be created with a parallel
  // strategy.
  ThreadPoolExecutor() = ړنګول;

  فراغ stop() {
    {
      std::قلف_guard<std::mutex> قلف_2(Mutex);
      إذا (Stop)
        أرجع;
      Stop = ریښتیا;
    }
    Cond.notify_all();
    تارونه_Created.get_future().wait();

    std::thread::id CurrentThreadId = std::this_thread::get_id();
    لكل (std::thread &T : تارونه)
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

  بنية Work_توکی {
    std::function<فراغ()> F;
    std::reference_wrapper<parallel::detail::Latch> L;
    فراغ چلوونکی()() {
      F();
      L.get().dec();
    }
  };

  فراغ جمع_کړه(std::function<فراغ()> F, parallel::detail::Latch &L) {
    {
      std::قلف_guard<std::mutex> قلف_2(Mutex);
      Work_ډېری.ټېل_شا({std::move(F), std::ref(L)});
    }
    Cond.notify_one();
  }

  // Execute tasks from the work queue until the latch reaches zero.
  // Used by nested TaskGroups (on worker threads) to prevent deadlock:
  // instead of blocking in sync(), actively help drain the queue.
  فراغ helpSync(ثابت parallel::detail::Latch &L) {
    طالما (L.get_شمېر() != 0) {
      std::unique_قلف<std::mutex> قلف_2(Mutex);
      إذا (Stop || Work_ډېری.تشه())
        أرجع;
      راوباسه_And_پرلپسې(قلف_2);
    }
  }

  size_t get_Thread_شمېر() ثابت { أرجع Thread_شمېر; }

خاص:
  // Pop one task from the queue and run it. Must be called with Lock held;
  // releases Lock before executing the task.
  فراغ راوباسه_And_پرلپسې(std::unique_قلف<std::mutex> &قلف_2) {
    خپلکاری توکی = std::move(Work_ډېری.شا());
    Work_ډېری.راوباسه_شا();
    قلف_2.unlock();
    توکی();
  }

  فراغ work(ThreadPoolStrategy S, بینښه ThreadID) {
    thread_شاخص = ThreadID;
    S.apply_thread_strategy(ThreadID);
    // Note on jobserver deadlock avoidance:
    // GNU Make grants each invoked process one implicit job slot. Our
    // JobserverClient models this by returning an implicit JobSlot on the
    // first successful tryAcquire() in a process. This guarantees forward
    // progress without requiring a dedicated "always-on" thread here.

    طالما (ریښتیا) {
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
        توانیز_Backoff Backoff(std::chrono::hours(24));
        دنده_خونه خونه;
        افعل {
          إذا (Stop)
            أرجع;
          خونه = TheJobserver->tryAcquire();
          إذا (خونه.isValid())
            اكسر;
        } طالما (Backoff.wait_For_بل_Attempt());

        llvm::scope_exit خونه_Releaser(
            [&] { TheJobserver->release(std::move(خونه)); });

        طالما (ریښتیا) {
          std::unique_قلف<std::mutex> قلف_2(Mutex);
          Cond.wait(قلف_2, [&] { أرجع Stop || !Work_ډېری.تشه(); });
          إذا (Stop && Work_ډېری.تشه())
            أرجع;
          إذا (Work_ډېری.تشه())
            اكسر;
          راوباسه_And_پرلپسې(قلف_2);
        }
      } وإلا {
        std::unique_قلف<std::mutex> قلف_2(Mutex);
        Cond.wait(قلف_2, [&] { أرجع Stop || !Work_ډېری.تشه(); });
        إذا (Stop)
          اكسر;
        راوباسه_And_پرلپسې(قلف_2);
      }
    }
  }

  std::atomic<منطقي> Stop{خطأ};
  std::ویکټور<Work_توکی> Work_ډېری;
  std::mutex Mutex;
  std::condition_variable Cond;
  std::promise<فراغ> تارونه_Created;
  std::ویکټور<std::thread> تارونه;
  بینښه Thread_شمېر;

  JobserverClient *TheJobserver = تش_اشاره;
};
} // namespace

ساكن ThreadPoolExecutor *getDefaultExecutor() {
#که_تعریف_شوی _WIN32
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
#د_که_پای
}

size_t parallel::get_Thread_شمېر() {
  أرجع getDefaultExecutor()->get_Thread_شمېر();
}
#د_که_پای

Task_ډله::Task_ډله()
    : Parallel(
#إذا LLVM_ENABLE_تارونه
          strategy.تارونه_Requested != 1
#وإلا
          خطأ
#د_که_پای
      ) {
}

Task_ډله::~Task_ډله() {
#إذا LLVM_ENABLE_تارونه
  // In a nested TaskGroup (threadIndex != -1u), actively help drain the queue.
  منطقي IsNested = thread_شاخص != UINT_MAX;
  إذا (Parallel && IsNested)
    getDefaultExecutor()->helpSync(L);
#د_که_پای
  L.متزامن();
}

فراغ Task_ډله::spawn(std::function<فراغ()> F) {
#إذا LLVM_ENABLE_تارونه
  إذا (Parallel) {
    L.inc();
    getDefaultExecutor()->جمع_کړه(std::move(F), L);
    أرجع;
  }
#د_که_پای
  F();
}

فراغ llvm::parallelFor(size_t Begin, size_t پای_ټکی_2,
                       function_ref<فراغ(size_t)> Fn) {
#إذا LLVM_ENABLE_تارونه
  إذا (strategy.تارونه_Requested != 1) {
    size_t عدد_توکي = پای_ټکی_2 - Begin;
    إذا (عدد_توکي == 0)
      أرجع;
    // Distribute work via an atomic counter shared by NumWorkers threads,
    // keeping the task count (and thus Linux futex calls) at O(ThreadCount)
    // For lld, per-file work is somewhat uneven, so a multipler > 1 is safer.
    // While 2 vs 4 vs 8 makes no measurable difference, 4 is used as a
    // reasonable default.
    size_t عدد_Workers = std::min<size_t>(عدد_توکي, get_Thread_شمېر());
    size_t Chunk_اندازه = std::max(size_t(1), عدد_توکي / (عدد_Workers * 4));
    std::atomic<size_t> Idx{Begin};
    خپلکاری کارګر = [&] {
      طالما (ریښتیا) {
        size_t I = Idx.fetch_جمع_کړه(Chunk_اندازه, std::memory_order_relaxed);
        إذا (I >= پای_ټکی_2)
          اكسر;
        size_t IEnd = std::min(I + Chunk_اندازه, پای_ټکی_2);
        لكل (; I < IEnd; ++I)
          Fn(I);
      }
    };

    // Run one worker on the calling thread: starts working immediately and
    // avoids an idle thread.
    Task_ډله TG;
    طالما (--عدد_Workers)
      TG.spawn(کارګر);
    کارګر();
    أرجع;
  }
#د_که_پای

  لكل (; Begin != پای_ټکی_2; ++Begin)
    Fn(Begin);
}
