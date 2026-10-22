//==-- llvm/Support/ThreadPool.cpp - A ThreadPool implementation -*- C++ -*-==//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
//
// This file implements a crude C++11 based thread pool.
//
//===----------------------------------------------------------------------===//

#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/ThreadPool.h"

#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Config/llvm-config.h"

#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/ADT/ScopeExit.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/ExponentialBackoff.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/FormatVariadic.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/Threading.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/raw_ostream.h"

ਵਰਤੋਂ ਨਾਮਾਕਾਸ਼ llvm;

ThreadPoolInterface::~ThreadPoolInterface() = ਮੂਲ;

// A note on thread groups: Tasks are by default in no group (represented
// by nullptr ThreadPoolTaskGroup pointer in the Tasks queue) and functionality
// here normally works on all tasks regardless of their group (functions
// in that case receive nullptr ThreadPoolTaskGroup pointer as argument).
// A task in a group has a pointer to that ThreadPoolTaskGroup in the Tasks
// queue, and functions called to work only on tasks from one group take that
// pointer.

#ਜੇ LLVM_ENABLE_ਧਾਗੇ

StdThreadPool::StdThreadPool(ThreadPoolStrategy S)
    : Strategy(S), Max_Thread_ਗਿਣਤੀ(S.compute_thread_ਗਿਣਤੀ()) {
  ਜੇ (Strategy.UseJobserver)
    TheJobserver = JobserverClient::getInstance();
}

ਖਾਲੀ StdThreadPool::grow(ਪੂਰਨ requested) {
  llvm::sys::ScopedWriter ਤਾਲਾ_Guard(ਧਾਗੇ_ਤਾਲਾ);
  ਜੇ (ਧਾਗੇ.ਮਾਪ_2() >= Max_Thread_ਗਿਣਤੀ)
    ਵਾਪਸ; // Already hit the max thread pool size.
  ਪੂਰਨ new_Thread_ਗਿਣਤੀ = std::min<ਪੂਰਨ>(requested, Max_Thread_ਗਿਣਤੀ);
  ਜਦੋਂ (static_cast<ਪੂਰਨ>(ਧਾਗੇ.ਮਾਪ_2()) < new_Thread_ਗਿਣਤੀ) {
    ਪੂਰਨ ThreadID = ਧਾਗੇ.ਮਾਪ_2();
    ਧਾਗੇ.emplace_ਪਿੱਠ([ਇਹ, ThreadID] {
      ਸੈੱਟ_thread_name(formatv("llvm-worker-{0}", ThreadID));
      Strategy.apply_thread_strategy(ThreadID);
      // Note on jobserver deadlock avoidance:
      // GNU Make grants each invoked process one implicit job slot.
      // JobserverClient::tryAcquire() returns that implicit slot on the first
      // successful call in a process, ensuring forward progress without a
      // dedicated "always-on" thread.
      ਜੇ (TheJobserver)
        processTasksWithJobserver();
      ਨਹੀਂਤਾਂ
        processTasks(ਖਾਲੀ_ਸੂਚਕ);
    });
  }
}

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ NDEBUG
// The group of the tasks run by the current thread.
ਸਥਿਰ LLVM_THREAD_LOCAL std::ਸਦਿਸ਼<Thread_Pool_Task_ਸਮੂਹ *>
    *CurrentThreadTaskGroups = ਖਾਲੀ_ਸੂਚਕ;
#ਜੇ_ਅੰਤ

// WaitingForGroup == nullptr means all tasks regardless of their group.
ਖਾਲੀ StdThreadPool::processTasks(Thread_Pool_Task_ਸਮੂਹ *ਉਡੀਕ_For_ਸਮੂਹ) {
  ਜਦੋਂ (ਸੱਚ) {
    llvm::unique_function<ਖਾਲੀ()> Task;
    Thread_Pool_Task_ਸਮੂਹ *ਸਮੂਹ_ਦਾ_Task;
    {
      std::unique_ਤਾਲਾ<std::mutex> ਤਾਲਾ_Guard(ਕਤਾਰ_ਤਾਲਾ);
      ਤਾਰਕਿਕ work_Completed_For_ਸਮੂਹ = ਝੂਠ; // Result of workCompletedUnlocked()
      // Wait for tasks to be pushed in the queue
      ਕਤਾਰ_Condition.wait(ਤਾਲਾ_Guard, [&] {
        ਵਾਪਸ !EnableFlag || !Tasks.ਖਾਲੀ_ਥਾਵਾਂ() ||
               (ਉਡੀਕ_For_ਸਮੂਹ != ਖਾਲੀ_ਸੂਚਕ &&
                (work_Completed_For_ਸਮੂਹ =
                     workCompletedUnlocked(ਉਡੀਕ_For_ਸਮੂਹ)));
      });
      // Exit condition
      ਜੇ (!EnableFlag && Tasks.ਖਾਲੀ_ਥਾਵਾਂ())
        ਵਾਪਸ;
      ਜੇ (ਉਡੀਕ_For_ਸਮੂਹ != ਖਾਲੀ_ਸੂਚਕ && work_Completed_For_ਸਮੂਹ)
        ਵਾਪਸ;
      // Yeah, we have a task, grab it and release the lock on the queue

      // We first need to signal that we are active before popping the queue
      // in order for wait() to properly detect that even if the queue is
      // empty, there is still a task in flight.
      ++ਸਰਗਰਮ_ਧਾਗੇ;
      Task = std::move(Tasks.front().ਪਹਿਲਾ_2);
      ਸਮੂਹ_ਦਾ_Task = Tasks.front().ਦੂਜਾ_2;
      // Need to count active threads in each group separately, ActiveThreads
      // would never be 0 if waiting for another group inside a wait.
      ਜੇ (ਸਮੂਹ_ਦਾ_Task != ਖਾਲੀ_ਸੂਚਕ)
        ++ਸਰਗਰਮ_Groups[ਸਮੂਹ_ਦਾ_Task]; // Increment or set to 1 if new item
      Tasks.ਕੱਢੋ_front();
    }
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ NDEBUG
    ਜੇ (CurrentThreadTaskGroups == ਖਾਲੀ_ਸੂਚਕ)
      CurrentThreadTaskGroups = ਨਵਾਂ std::ਸਦਿਸ਼<Thread_Pool_Task_ਸਮੂਹ *>;
    CurrentThreadTaskGroups->ਧੱਕੋ_ਪਿੱਠ(ਸਮੂਹ_ਦਾ_Task);
#ਜੇ_ਅੰਤ

    // Run the task we just grabbed. This also destroys the task once run to
    // release any resources held by it through RAII captured objects.
    //
    // It is particularly important to do this here so that we're not holding
    // any lock and any further operations on the thread or `ThreadPool` take
    // place here, at the same point as the task itself is executed.
    std::exchange(Task, {})();

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ NDEBUG
    CurrentThreadTaskGroups->ਕੱਢੋ_ਪਿੱਠ();
    ਜੇ (CurrentThreadTaskGroups->ਖਾਲੀ_ਥਾਵਾਂ()) {
      ਮਿਟਾਓ CurrentThreadTaskGroups;
      CurrentThreadTaskGroups = ਖਾਲੀ_ਸੂਚਕ;
    }
#ਜੇ_ਅੰਤ

    ਤਾਰਕਿਕ Notify;
    ਤਾਰਕਿਕ Notify_ਸਮੂਹ;
    {
      // Adjust `ActiveThreads`, in case someone waits on StdThreadPool::wait()
      std::ਤਾਲਾ_guard<std::mutex> ਤਾਲਾ_Guard(ਕਤਾਰ_ਤਾਲਾ);
      --ਸਰਗਰਮ_ਧਾਗੇ;
      ਜੇ (ਸਮੂਹ_ਦਾ_Task != ਖਾਲੀ_ਸੂਚਕ) {
        ਸਵੈਚਾਲਿਤ A = ਸਰਗਰਮ_Groups.ਲੱਭੋ(ਸਮੂਹ_ਦਾ_Task);
        ਜੇ (--(A->ਦੂਜਾ_2) == 0)
          ਸਰਗਰਮ_Groups.erase(A);
      }
      Notify = workCompletedUnlocked(ਸਮੂਹ_ਦਾ_Task);
      Notify_ਸਮੂਹ = ਸਮੂਹ_ਦਾ_Task != ਖਾਲੀ_ਸੂਚਕ && Notify;
    }
    // Notify task completion if this is the last active thread, in case
    // someone waits on StdThreadPool::wait().
    ਜੇ (Notify)
      CompletionCondition.notify_all();
    // If this was a task in a group, notify also threads waiting for tasks
    // in this function on QueueCondition, to make a recursive wait() return
    // after the group it's been waiting for has finished.
    ਜੇ (Notify_ਸਮੂਹ)
      ਕਤਾਰ_Condition.notify_all();
  }
}

/// Main loop for worker threads when using a jobserver.
/// This function uses a two-level queue; it first acquires a job slot from the
/// external jobserver, then retrieves a task from the internal queue.
/// This allows the thread pool to cooperate with build systems like `make -j`.
ਖਾਲੀ StdThreadPool::processTasksWithJobserver() {
  ਜਦੋਂ (ਸੱਚ) {
    // Acquire a job slot from the external jobserver.
    // This polls for a slot and yields the thread to avoid a high-CPU wait.
    ਕੰਮ_ਥਾਂ ਥਾਂ;
    // The timeout for the backoff can be very long, as the shutdown
    // is checked on each iteration. The sleep duration is capped by MaxWait
    // in ExponentialBackoff, so shutdown latency is not a problem.
    ਘਾਤਾਂਕੀ_Backoff Backoff(std::chrono::hours(24));
    ਤਾਰਕਿਕ AcquiredToken = ਝੂਠ;
    ਕਰੋ {
      // Return if the thread pool is shutting down.
      {
        std::unique_ਤਾਲਾ<std::mutex> ਤਾਲਾ_Guard(ਕਤਾਰ_ਤਾਲਾ);
        ਜੇ (!EnableFlag)
          ਵਾਪਸ;
      }

      ਥਾਂ = TheJobserver->tryAcquire();
      ਜੇ (ਥਾਂ.isValid()) {
        AcquiredToken = ਸੱਚ;
        ਰੋਕੋ;
      }
    } ਜਦੋਂ (Backoff.wait_For_ਅਗਲਾ_Attempt());

    ਜੇ (!AcquiredToken) {
      // This is practically unreachable with a 24h timeout and indicates a
      // deeper problem if hit.
      report_fatal_error("Timed out waiting for jobserver token.");
    }

    // `llvm::scope_exit` guarantees the job slot is released, even if the
    // task throws or we exit early. This prevents deadlocking the build.
    llvm::scope_exit ਥਾਂ_Releaser(
        [&] { TheJobserver->release(std::move(ਥਾਂ)); });

    // While we hold a job slot, process tasks from the internal queue.
    ਜਦੋਂ (ਸੱਚ) {
      llvm::unique_function<ਖਾਲੀ()> Task;
      Thread_Pool_Task_ਸਮੂਹ *ਸਮੂਹ_ਦਾ_Task = ਖਾਲੀ_ਸੂਚਕ;

      {
        std::unique_ਤਾਲਾ<std::mutex> ਤਾਲਾ_Guard(ਕਤਾਰ_ਤਾਲਾ);

        // Wait until a task is available or the pool is shutting down.
        ਕਤਾਰ_Condition.wait(ਤਾਲਾ_Guard,
                            [&] { ਵਾਪਸ !EnableFlag || !Tasks.ਖਾਲੀ_ਥਾਵਾਂ(); });

        // If shutting down and the queue is empty, the thread can terminate.
        ਜੇ (!EnableFlag && Tasks.ਖਾਲੀ_ਥਾਵਾਂ())
          ਵਾਪਸ;

        // If the queue is empty, we're done processing tasks for now.
        // Break the inner loop to release the job slot.
        ਜੇ (Tasks.ਖਾਲੀ_ਥਾਵਾਂ())
          ਰੋਕੋ;

        // A task is available. Mark it as active before releasing the lock
        // to prevent race conditions with `wait()`.
        ++ਸਰਗਰਮ_ਧਾਗੇ;
        Task = std::move(Tasks.front().ਪਹਿਲਾ_2);
        ਸਮੂਹ_ਦਾ_Task = Tasks.front().ਦੂਜਾ_2;
        ਜੇ (ਸਮੂਹ_ਦਾ_Task != ਖਾਲੀ_ਸੂਚਕ)
          ++ਸਰਗਰਮ_Groups[ਸਮੂਹ_ਦਾ_Task];
        Tasks.ਕੱਢੋ_front();
      } // The queue lock is released.

      // Run the task. The job slot remains acquired during execution.
      Task();

      // The task has finished. Update the active count and notify any waiters.
      {
        std::ਤਾਲਾ_guard<std::mutex> ਤਾਲਾ_Guard(ਕਤਾਰ_ਤਾਲਾ);
        --ਸਰਗਰਮ_ਧਾਗੇ;
        ਜੇ (ਸਮੂਹ_ਦਾ_Task != ਖਾਲੀ_ਸੂਚਕ) {
          ਸਵੈਚਾਲਿਤ A = ਸਰਗਰਮ_Groups.ਲੱਭੋ(ਸਮੂਹ_ਦਾ_Task);
          ਜੇ (--(A->ਦੂਜਾ_2) == 0)
            ਸਰਗਰਮ_Groups.erase(A);
        }
        // If all tasks are complete, notify any waiting threads.
        ਜੇ (workCompletedUnlocked(ਖਾਲੀ_ਸੂਚਕ))
          CompletionCondition.notify_all();
      }
    }
  }
}
ਤਾਰਕਿਕ StdThreadPool::workCompletedUnlocked(Thread_Pool_Task_ਸਮੂਹ *ਸਮੂਹ) ਅਚਲ {
  ਜੇ (ਸਮੂਹ == ਖਾਲੀ_ਸੂਚਕ)
    ਵਾਪਸ !ਸਰਗਰਮ_ਧਾਗੇ && Tasks.ਖਾਲੀ_ਥਾਵਾਂ();
  ਵਾਪਸ ਸਰਗਰਮ_Groups.ਗਿਣਤੀ_2(ਸਮੂਹ) == 0 &&
         !llvm::is_contained(llvm::make_ਦੂਜਾ_range(Tasks), ਸਮੂਹ);
}

ਖਾਲੀ StdThreadPool::wait() {
  ਪ੍ਰਤਿਜ੍ਞਾ(!is_ਕਾਮਾ_Thread()); // Would deadlock waiting for itself.
  // Wait for all threads to complete and the queue to be empty
  std::unique_ਤਾਲਾ<std::mutex> ਤਾਲਾ_Guard(ਕਤਾਰ_ਤਾਲਾ);
  CompletionCondition.wait(ਤਾਲਾ_Guard,
                           [&] { ਵਾਪਸ workCompletedUnlocked(ਖਾਲੀ_ਸੂਚਕ); });
}

ਖਾਲੀ StdThreadPool::wait(Thread_Pool_Task_ਸਮੂਹ &ਸਮੂਹ) {
  // Wait for all threads in the group to complete.
  ਜੇ (!is_ਕਾਮਾ_Thread()) {
    std::unique_ਤਾਲਾ<std::mutex> ਤਾਲਾ_Guard(ਕਤਾਰ_ਤਾਲਾ);
    CompletionCondition.wait(ਤਾਲਾ_Guard,
                             [&] { ਵਾਪਸ workCompletedUnlocked(&ਸਮੂਹ); });
    ਵਾਪਸ;
  }
  // Make sure to not deadlock waiting for oneself.
  ਪ੍ਰਤਿਜ੍ਞਾ(CurrentThreadTaskGroups == ਖਾਲੀ_ਸੂਚਕ ||
         !llvm::is_contained(*CurrentThreadTaskGroups, &ਸਮੂਹ));
  // Handle the case of recursive call from another task in a different group,
  // in which case process tasks while waiting to keep the thread busy and avoid
  // possible deadlock.
  processTasks(&ਸਮੂਹ);
}

ਤਾਰਕਿਕ StdThreadPool::is_ਕਾਮਾ_Thread() ਅਚਲ {
  llvm::sys::ScopedReader ਤਾਲਾ_Guard(ਧਾਗੇ_ਤਾਲਾ);
  llvm::thread::id CurrentThreadId = llvm::this_thread::get_id();
  ਲਈ (ਅਚਲ llvm::thread &Thread : ਧਾਗੇ)
    ਜੇ (CurrentThreadId == Thread.get_id())
      ਵਾਪਸ ਸੱਚ;
  ਵਾਪਸ ਝੂਠ;
}

// The destructor joins all threads, waiting for completion.
StdThreadPool::~StdThreadPool() {
  {
    std::unique_ਤਾਲਾ<std::mutex> ਤਾਲਾ_Guard(ਕਤਾਰ_ਤਾਲਾ);
    EnableFlag = ਝੂਠ;
  }
  ਕਤਾਰ_Condition.notify_all();
  llvm::sys::ScopedReader ਤਾਲਾ_Guard(ਧਾਗੇ_ਤਾਲਾ);
  ਲਈ (ਸਵੈਚਾਲਿਤ &ਕਾਮਾ : ਧਾਗੇ)
    ਕਾਮਾ.join();
}

#ਜੇ_ਅੰਤ // LLVM_ENABLE_THREADS Disabled

// No threads are launched, issue a warning if ThreadCount is not 0
SingleThreadExecutor::SingleThreadExecutor(ThreadPoolStrategy S) {
  ਪੂਰਨ Thread_ਗਿਣਤੀ = S.compute_thread_ਗਿਣਤੀ();
  ਜੇ (Thread_ਗਿਣਤੀ != 1) {
    errs() << "Warning: request a ThreadPool with " << Thread_ਗਿਣਤੀ
           << " threads, but LLVM_ENABLE_THREADS has been turned off\n";
  }
}

ਖਾਲੀ SingleThreadExecutor::wait() {
  // Sequential implementation running the tasks
  ਜਦੋਂ (!Tasks.ਖਾਲੀ_ਥਾਵਾਂ()) {
    ਸਵੈਚਾਲਿਤ Task = std::move(Tasks.front().ਪਹਿਲਾ_2);
    Tasks.ਕੱਢੋ_front();
    Task();
  }
}

ਖਾਲੀ SingleThreadExecutor::wait(Thread_Pool_Task_ਸਮੂਹ &) {
  // Simply wait for all, this works even if recursive (the running task
  // is already removed from the queue).
  wait();
}

ਤਾਰਕਿਕ SingleThreadExecutor::is_ਕਾਮਾ_Thread() ਅਚਲ {
  report_fatal_error("LLVM compiled without multithreading");
}

SingleThreadExecutor::~SingleThreadExecutor() { wait(); }
