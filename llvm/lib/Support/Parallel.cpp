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

उपयुज्य नामाकाश llvm;
उपयुज्य नामाकाश llvm::parallel;

llvm::ThreadPoolStrategy parallel::strategy;

#यदि LLVM_ENABLE_तन्तवः

स्थिर thread_local अचिह्नित thread_सूचकांक = UINT_MAX;

नामाकाश {

/// Runs closures on a thread pool in filo order.
वर्ग ThreadPoolExecutor {
सार्वजनिक:
  स्पष्ट ThreadPoolExecutor(ThreadPoolStrategy S) {
    यदि (S.UseJobserver)
      TheJobserver = JobserverClient::getInstance();

    Thread_गणनम् = S.compute_thread_गणनम्();
    // Spawn all but one of the threads in another thread as spawning threads
    // can take a while.
    तन्तवः.reserve(Thread_गणनम्);
    तन्तवः.resize(1);
    std::तालकम्_guard<std::mutex> तालकम्_2(Mutex);
    // Use operator[] before creating the thread to avoid data race in .size()
    // in 'safe libc++' mode.
    स्वचालित &Thread0 = तन्तवः[0];
    Thread0 = std::thread([एतत्, S] {
      प्रति (अचिह्नित I = 1; I < Thread_गणनम्; ++I) {
        तन्तवः.emplace_पृष्ठभागः([एतत्, S, I] { work(S, I); });
        यदि (Stop)
          भङ्ग;
      }
      तन्तवः_Created.समुच्चयः_मानम्();
      work(S, 0);
    });
  }

  // To make sure the thread pool executor can only be created with a parallel
  // strategy.
  ThreadPoolExecutor() = विलोपय;

  शून्य stop() {
    {
      std::तालकम्_guard<std::mutex> तालकम्_2(Mutex);
      यदि (Stop)
        प्रत्यागम;
      Stop = सत्य;
    }
    Cond.notify_all();
    तन्तवः_Created.get_future().प्रतीक्षस्व();

    std::thread::id CurrentThreadId = std::this_thread::get_id();
    प्रति (std::thread &T : तन्तवः)
      यदि (T.get_id() == CurrentThreadId)
        T.detach();
      अन्यथा
        T.join();
  }

  ~ThreadPoolExecutor() { stop(); }

  संरचना Creator {
    स्थिर शून्य *call() { प्रत्यागम नूतन ThreadPoolExecutor(strategy); }
  };
  संरचना Deleter {
    स्थिर शून्य call(शून्य *Ptr) { ((ThreadPoolExecutor *)Ptr)->stop(); }
  };

  संरचना WorkItem {
    std::function<शून्य()> F;
    std::reference_wrapper<parallel::detail::Latch> L;
    शून्य संक्रियकः()() {
      F();
      L.get().dec();
    }
  };

  शून्य सङ्कलय(std::function<शून्य()> F, parallel::detail::Latch &L) {
    {
      std::तालकम्_guard<std::mutex> तालकम्_2(Mutex);
      Work_राशिः.प्रक्षिप_पृष्ठभागः({std::move(F), std::ref(L)});
    }
    Cond.notify_one();
  }

  // Execute tasks from the work queue until the latch reaches zero.
  // Used by nested TaskGroups (on worker threads) to prevent deadlock:
  // instead of blocking in sync(), actively help drain the queue.
  शून्य helpSync(ध्रुव parallel::detail::Latch &L) {
    यावत् (L.get_गणनम्() != 0) {
      std::unique_तालकम्<std::mutex> तालकम्_2(Mutex);
      यदि (Stop || Work_राशिः.रिक्तानि())
        प्रत्यागम;
      निष्कासय_And_सततम्(तालकम्_2);
    }
  }

  size_t get_Thread_गणनम्() ध्रुव { प्रत्यागम Thread_गणनम्; }

गुप्त:
  // Pop one task from the queue and run it. Must be called with Lock held;
  // releases Lock before executing the task.
  शून्य निष्कासय_And_सततम्(std::unique_तालकम्<std::mutex> &तालकम्_2) {
    स्वचालित Item = std::move(Work_राशिः.पृष्ठभागः());
    Work_राशिः.निष्कासय_पृष्ठभागः();
    तालकम्_2.unlock();
    Item();
  }

  शून्य work(ThreadPoolStrategy S, अचिह्नित ThreadID) {
    thread_सूचकांक = ThreadID;
    S.apply_thread_strategy(ThreadID);
    // Note on jobserver deadlock avoidance:
    // GNU Make grants each invoked process one implicit job slot. Our
    // JobserverClient models this by returning an implicit JobSlot on the
    // first successful tryAcquire() in a process. This guarantees forward
    // progress without requiring a dedicated "always-on" thread here.

    यावत् (सत्य) {
      यदि (TheJobserver) {
        // Jobserver-mode scheduling:
        // - Acquire one job slot (with exponential backoff to avoid busy-wait).
        // - While holding the slot, drain and run tasks from the local queue.
        // - Release the slot when the queue is empty or when shutting down.
        // Rationale: Holding a slot amortizes acquire/release overhead over
        // multiple tasks and avoids requeue/yield churn, while still enforcing
        // the jobserver’s global concurrency limit. With K available slots,
        // up to K workers run tasks in parallel; within each worker tasks run
        // sequentially until the local queue is empty.
        घातीयम्_Backoff Backoff(std::chrono::hours(24));
        JobSlot Slot;
        कुरु {
          यदि (Stop)
            प्रत्यागम;
          Slot = TheJobserver->tryAcquire();
          यदि (Slot.isValid())
            भङ्ग;
        } यावत् (Backoff.wait_For_अग्र_Attempt());

        llvm::scope_exit SlotReleaser(
            [&] { TheJobserver->release(std::move(Slot)); });

        यावत् (सत्य) {
          std::unique_तालकम्<std::mutex> तालकम्_2(Mutex);
          Cond.प्रतीक्षस्व(तालकम्_2, [&] { प्रत्यागम Stop || !Work_राशिः.रिक्तानि(); });
          यदि (Stop && Work_राशिः.रिक्तानि())
            प्रत्यागम;
          यदि (Work_राशिः.रिक्तानि())
            भङ्ग;
          निष्कासय_And_सततम्(तालकम्_2);
        }
      } अन्यथा {
        std::unique_तालकम्<std::mutex> तालकम्_2(Mutex);
        Cond.प्रतीक्षस्व(तालकम्_2, [&] { प्रत्यागम Stop || !Work_राशिः.रिक्तानि(); });
        यदि (Stop)
          भङ्ग;
        निष्कासय_And_सततम्(तालकम्_2);
      }
    }
  }

  std::atomic<तार्किक> Stop{असत्य};
  std::सदिशम्<WorkItem> Work_राशिः;
  std::mutex Mutex;
  std::condition_variable Cond;
  std::promise<शून्य> तन्तवः_Created;
  std::सदिशम्<std::thread> तन्तवः;
  अचिह्नित Thread_गणनम्;

  JobserverClient *TheJobserver = शून्यसूचकः;
};
} // namespace

स्थिर ThreadPoolExecutor *getDefaultExecutor() {
#यदि_परिभाषितम् _WIN32
  // The ManagedStatic enables the ThreadPoolExecutor to be stopped via
  // llvm_shutdown() on Windows. This is important to avoid various race
  // conditions at process exit that can cause crashes or deadlocks.

  स्थिर ManagedStatic<ThreadPoolExecutor, ThreadPoolExecutor::Creator,
                       ThreadPoolExecutor::Deleter>
      ManagedExec;
  स्थिर std::unique_ptr<ThreadPoolExecutor> Exec(&(*ManagedExec));
  प्रत्यागम Exec.get();
#अन्यथा
  // ManagedStatic is not desired on other platforms. When `Exec` is destroyed
  // by llvm_shutdown(), worker threads will clean up and invoke TLS
  // destructors. This can lead to race conditions if other threads attempt to
  // access TLS objects that have already been destroyed.
  स्थिर ThreadPoolExecutor Exec(strategy);
  प्रत्यागम &Exec;
#यदि_अन्तः
}

size_t parallel::get_Thread_गणनम्() {
  प्रत्यागम getDefaultExecutor()->get_Thread_गणनम्();
}
#यदि_अन्तः

Task_गणः::Task_गणः()
    : Parallel(
#यदि LLVM_ENABLE_तन्तवः
          strategy.तन्तवः_Requested != 1
#अन्यथा
          असत्य
#यदि_अन्तः
      ) {
}

Task_गणः::~Task_गणः() {
#यदि LLVM_ENABLE_तन्तवः
  // In a nested TaskGroup (threadIndex != -1u), actively help drain the queue.
  तार्किक IsNested = thread_सूचकांक != UINT_MAX;
  यदि (Parallel && IsNested)
    getDefaultExecutor()->helpSync(L);
#यदि_अन्तः
  L.समकाल();
}

शून्य Task_गणः::spawn(std::function<शून्य()> F) {
#यदि LLVM_ENABLE_तन्तवः
  यदि (Parallel) {
    L.inc();
    getDefaultExecutor()->सङ्कलय(std::move(F), L);
    प्रत्यागम;
  }
#यदि_अन्तः
  F();
}

शून्य llvm::parallelFor(size_t Begin, size_t अन्तः_2,
                       function_ref<शून्य(size_t)> Fn) {
#यदि LLVM_ENABLE_तन्तवः
  यदि (strategy.तन्तवः_Requested != 1) {
    size_t अङ्कः_वस्तूनि = अन्तः_2 - Begin;
    यदि (अङ्कः_वस्तूनि == 0)
      प्रत्यागम;
    // Distribute work via an atomic counter shared by NumWorkers threads,
    // keeping the task count (and thus Linux futex calls) at O(ThreadCount)
    // For lld, per-file work is somewhat uneven, so a multipler > 1 is safer.
    // While 2 vs 4 vs 8 makes no measurable difference, 4 is used as a
    // reasonable default.
    size_t अङ्कः_Workers = std::min<size_t>(अङ्कः_वस्तूनि, get_Thread_गणनम्());
    size_t Chunk_आकार = std::max(size_t(1), अङ्कः_वस्तूनि / (अङ्कः_Workers * 4));
    std::atomic<size_t> Idx{Begin};
    स्वचालित कार्यकर्ता = [&] {
      यावत् (सत्य) {
        size_t I = Idx.fetch_सङ्कलय(Chunk_आकार, std::memory_order_relaxed);
        यदि (I >= अन्तः_2)
          भङ्ग;
        size_t IEnd = std::min(I + Chunk_आकार, अन्तः_2);
        प्रति (; I < IEnd; ++I)
          Fn(I);
      }
    };

    // Run one worker on the calling thread: starts working immediately and
    // avoids an idle thread.
    Task_गणः TG;
    यावत् (--अङ्कः_Workers)
      TG.spawn(कार्यकर्ता);
    कार्यकर्ता();
    प्रत्यागम;
  }
#यदि_अन्तः

  प्रति (; Begin != अन्तः_2; ++Begin)
    Fn(Begin);
}
