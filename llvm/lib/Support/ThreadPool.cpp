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

#ಅನ್ತರ್ಭಾವ "llvm/Support/ThreadPool.h"

#ಅನ್ತರ್ಭಾವ "llvm/Config/llvm-config.h"

#ಅನ್ತರ್ಭಾವ "llvm/ADT/ScopeExit.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/ExponentialBackoff.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/FormatVariadic.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/Threading.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/raw_ostream.h"

ಬಳಸಿ ನಾಮಾಕಾಶ llvm;

ThreadPoolInterface::~ThreadPoolInterface() = ಪೂರ್ವನಿಯೋಜಿತ;

// A note on thread groups: Tasks are by default in no group (represented
// by nullptr ThreadPoolTaskGroup pointer in the Tasks queue) and functionality
// here normally works on all tasks regardless of their group (functions
// in that case receive nullptr ThreadPoolTaskGroup pointer as argument).
// A task in a group has a pointer to that ThreadPoolTaskGroup in the Tasks
// queue, and functions called to work only on tasks from one group take that
// pointer.

#ಆದರೆ LLVM_ENABLE_ಎಳೆಗಳು

StdThreadPool::StdThreadPool(ThreadPoolStrategy S)
    : Strategy(S), Max_Thread_ಎಣಿಕೆ(S.compute_thread_ಎಣಿಕೆ()) {
  ಆದರೆ (Strategy.UseJobserver)
    TheJobserver = JobserverClient::getInstance();
}

ಶೂನ್ಯ StdThreadPool::grow(ಪೂರ್ಣಾಂಕ requested) {
  llvm::sys::ScopedWriter ಬೀಗ_Guard(ಎಳೆಗಳು_ಬೀಗ);
  ಆದರೆ (ಎಳೆಗಳು.ಗಾತ್ರ_3() >= Max_Thread_ಎಣಿಕೆ)
    ಹಿಂತಿರುಗಿ; // Already hit the max thread pool size.
  ಪೂರ್ಣಾಂಕ new_Thread_ಎಣಿಕೆ = std::min<ಪೂರ್ಣಾಂಕ>(requested, Max_Thread_ಎಣಿಕೆ);
  ಆಗ (static_cast<ಪೂರ್ಣಾಂಕ>(ಎಳೆಗಳು.ಗಾತ್ರ_3()) < new_Thread_ಎಣಿಕೆ) {
    ಪೂರ್ಣಾಂಕ ThreadID = ಎಳೆಗಳು.ಗಾತ್ರ_3();
    ಎಳೆಗಳು.emplace_ಹಿಂಭಾಗ([ಇದು, ThreadID] {
      ಗಣ_thread_name(formatv("llvm-worker-{0}", ThreadID));
      Strategy.apply_thread_strategy(ThreadID);
      // Note on jobserver deadlock avoidance:
      // GNU Make grants each invoked process one implicit job slot.
      // JobserverClient::tryAcquire() returns that implicit slot on the first
      // successful call in a process, ensuring forward progress without a
      // dedicated "always-on" thread.
      ಆದರೆ (TheJobserver)
        processTasksWithJobserver();
      ಇಲ್ಲದಿದ್ದರೆ
        processTasks(ಶೂನ್ಯಸೂಚಕ);
    });
  }
}

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ NDEBUG
// The group of the tasks run by the current thread.
ಸ್ಥಿರ LLVM_THREAD_LOCAL std::ಸದಿಶ<Thread_Pool_Task_ಗುಂಪು *>
    *CurrentThreadTaskGroups = ಶೂನ್ಯಸೂಚಕ;
#ಆದರೆ_ಅಂತ್ಯ

// WaitingForGroup == nullptr means all tasks regardless of their group.
ಶೂನ್ಯ StdThreadPool::processTasks(Thread_Pool_Task_ಗುಂಪು *ಕಾಯುವಿಕೆ_For_ಗುಂಪು) {
  ಆಗ (ನಿಜ) {
    llvm::unique_function<ಶೂನ್ಯ()> Task;
    Thread_Pool_Task_ಗುಂಪು *ಗುಂಪು_ದ_Task;
    {
      std::unique_ಬೀಗ<std::mutex> ಬೀಗ_Guard(ಸರತಿ_ಬೀಗ);
      ತಾರ್ಕಿಕ work_Completed_For_ಗುಂಪು = ಸುಳ್ಳು; // Result of workCompletedUnlocked()
      // Wait for tasks to be pushed in the queue
      ಸರತಿ_Condition.wait(ಬೀಗ_Guard, [&] {
        ಹಿಂತಿರುಗಿ !EnableFlag || !Tasks.ಖಾಲಿಗಳು() ||
               (ಕಾಯುವಿಕೆ_For_ಗುಂಪು != ಶೂನ್ಯಸೂಚಕ &&
                (work_Completed_For_ಗುಂಪು =
                     workCompletedUnlocked(ಕಾಯುವಿಕೆ_For_ಗುಂಪು)));
      });
      // Exit condition
      ಆದರೆ (!EnableFlag && Tasks.ಖಾಲಿಗಳು())
        ಹಿಂತಿರುಗಿ;
      ಆದರೆ (ಕಾಯುವಿಕೆ_For_ಗುಂಪು != ಶೂನ್ಯಸೂಚಕ && work_Completed_For_ಗುಂಪು)
        ಹಿಂತಿರುಗಿ;
      // Yeah, we have a task, grab it and release the lock on the queue

      // We first need to signal that we are active before popping the queue
      // in order for wait() to properly detect that even if the queue is
      // empty, there is still a task in flight.
      ++ಸಕ್ರಿಯ_ಎಳೆಗಳು;
      Task = std::move(Tasks.front().ಮೊದಲ_2);
      ಗುಂಪು_ದ_Task = Tasks.front().ಎರಡನೇ_2;
      // Need to count active threads in each group separately, ActiveThreads
      // would never be 0 if waiting for another group inside a wait.
      ಆದರೆ (ಗುಂಪು_ದ_Task != ಶೂನ್ಯಸೂಚಕ)
        ++ಸಕ್ರಿಯ_Groups[ಗುಂಪು_ದ_Task]; // Increment or set to 1 if new item
      Tasks.ತೆಗೆ_front();
    }
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ NDEBUG
    ಆದರೆ (CurrentThreadTaskGroups == ಶೂನ್ಯಸೂಚಕ)
      CurrentThreadTaskGroups = ಹೊಸ std::ಸದಿಶ<Thread_Pool_Task_ಗುಂಪು *>;
    CurrentThreadTaskGroups->ತಳ್ಳು_ಹಿಂಭಾಗ(ಗುಂಪು_ದ_Task);
#ಆದರೆ_ಅಂತ್ಯ

    // Run the task we just grabbed. This also destroys the task once run to
    // release any resources held by it through RAII captured objects.
    //
    // It is particularly important to do this here so that we're not holding
    // any lock and any further operations on the thread or `ThreadPool` take
    // place here, at the same point as the task itself is executed.
    std::exchange(Task, {})();

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ NDEBUG
    CurrentThreadTaskGroups->ತೆಗೆ_ಹಿಂಭಾಗ();
    ಆದರೆ (CurrentThreadTaskGroups->ಖಾಲಿಗಳು()) {
      ಅಳಿಸು CurrentThreadTaskGroups;
      CurrentThreadTaskGroups = ಶೂನ್ಯಸೂಚಕ;
    }
#ಆದರೆ_ಅಂತ್ಯ

    ತಾರ್ಕಿಕ Notify;
    ತಾರ್ಕಿಕ Notify_ಗುಂಪು;
    {
      // Adjust `ActiveThreads`, in case someone waits on StdThreadPool::wait()
      std::ಬೀಗ_guard<std::mutex> ಬೀಗ_Guard(ಸರತಿ_ಬೀಗ);
      --ಸಕ್ರಿಯ_ಎಳೆಗಳು;
      ಆದರೆ (ಗುಂಪು_ದ_Task != ಶೂನ್ಯಸೂಚಕ) {
        ಸ್ವಯಂಚಾಲಿತ A = ಸಕ್ರಿಯ_Groups.ಕಂಡುಹಿಡಿ(ಗುಂಪು_ದ_Task);
        ಆದರೆ (--(A->ಎರಡನೇ_2) == 0)
          ಸಕ್ರಿಯ_Groups.erase(A);
      }
      Notify = workCompletedUnlocked(ಗುಂಪು_ದ_Task);
      Notify_ಗುಂಪು = ಗುಂಪು_ದ_Task != ಶೂನ್ಯಸೂಚಕ && Notify;
    }
    // Notify task completion if this is the last active thread, in case
    // someone waits on StdThreadPool::wait().
    ಆದರೆ (Notify)
      CompletionCondition.notify_all();
    // If this was a task in a group, notify also threads waiting for tasks
    // in this function on QueueCondition, to make a recursive wait() return
    // after the group it's been waiting for has finished.
    ಆದರೆ (Notify_ಗುಂಪು)
      ಸರತಿ_Condition.notify_all();
  }
}

/// Main loop for worker threads when using a jobserver.
/// This function uses a two-level queue; it first acquires a job slot from the
/// external jobserver, then retrieves a task from the internal queue.
/// This allows the thread pool to cooperate with build systems like `make -j`.
ಶೂನ್ಯ StdThreadPool::processTasksWithJobserver() {
  ಆಗ (ನಿಜ) {
    // Acquire a job slot from the external jobserver.
    // This polls for a slot and yields the thread to avoid a high-CPU wait.
    ಕೆಲಸ_ಸ್ಥಾನ ಸ್ಥಾನ;
    // The timeout for the backoff can be very long, as the shutdown
    // is checked on each iteration. The sleep duration is capped by MaxWait
    // in ExponentialBackoff, so shutdown latency is not a problem.
    ಘಾತೀಯ_Backoff Backoff(std::chrono::hours(24));
    ತಾರ್ಕಿಕ AcquiredToken = ಸುಳ್ಳು;
    ಮಾಡು {
      // Return if the thread pool is shutting down.
      {
        std::unique_ಬೀಗ<std::mutex> ಬೀಗ_Guard(ಸರತಿ_ಬೀಗ);
        ಆದರೆ (!EnableFlag)
          ಹಿಂತಿರುಗಿ;
      }

      ಸ್ಥಾನ = TheJobserver->tryAcquire();
      ಆದರೆ (ಸ್ಥಾನ.isValid()) {
        AcquiredToken = ನಿಜ;
        ನಿಲ್ಲಿಸು;
      }
    } ಆಗ (Backoff.wait_For_ಮುಂದಿನ_Attempt());

    ಆದರೆ (!AcquiredToken) {
      // This is practically unreachable with a 24h timeout and indicates a
      // deeper problem if hit.
      report_fatal_error("Timed out waiting for jobserver token.");
    }

    // `llvm::scope_exit` guarantees the job slot is released, even if the
    // task throws or we exit early. This prevents deadlocking the build.
    llvm::scope_exit ಸ್ಥಾನ_Releaser(
        [&] { TheJobserver->release(std::move(ಸ್ಥಾನ)); });

    // While we hold a job slot, process tasks from the internal queue.
    ಆಗ (ನಿಜ) {
      llvm::unique_function<ಶೂನ್ಯ()> Task;
      Thread_Pool_Task_ಗುಂಪು *ಗುಂಪು_ದ_Task = ಶೂನ್ಯಸೂಚಕ;

      {
        std::unique_ಬೀಗ<std::mutex> ಬೀಗ_Guard(ಸರತಿ_ಬೀಗ);

        // Wait until a task is available or the pool is shutting down.
        ಸರತಿ_Condition.wait(ಬೀಗ_Guard,
                            [&] { ಹಿಂತಿರುಗಿ !EnableFlag || !Tasks.ಖಾಲಿಗಳು(); });

        // If shutting down and the queue is empty, the thread can terminate.
        ಆದರೆ (!EnableFlag && Tasks.ಖಾಲಿಗಳು())
          ಹಿಂತಿರುಗಿ;

        // If the queue is empty, we're done processing tasks for now.
        // Break the inner loop to release the job slot.
        ಆದರೆ (Tasks.ಖಾಲಿಗಳು())
          ನಿಲ್ಲಿಸು;

        // A task is available. Mark it as active before releasing the lock
        // to prevent race conditions with `wait()`.
        ++ಸಕ್ರಿಯ_ಎಳೆಗಳು;
        Task = std::move(Tasks.front().ಮೊದಲ_2);
        ಗುಂಪು_ದ_Task = Tasks.front().ಎರಡನೇ_2;
        ಆದರೆ (ಗುಂಪು_ದ_Task != ಶೂನ್ಯಸೂಚಕ)
          ++ಸಕ್ರಿಯ_Groups[ಗುಂಪು_ದ_Task];
        Tasks.ತೆಗೆ_front();
      } // The queue lock is released.

      // Run the task. The job slot remains acquired during execution.
      Task();

      // The task has finished. Update the active count and notify any waiters.
      {
        std::ಬೀಗ_guard<std::mutex> ಬೀಗ_Guard(ಸರತಿ_ಬೀಗ);
        --ಸಕ್ರಿಯ_ಎಳೆಗಳು;
        ಆದರೆ (ಗುಂಪು_ದ_Task != ಶೂನ್ಯಸೂಚಕ) {
          ಸ್ವಯಂಚಾಲಿತ A = ಸಕ್ರಿಯ_Groups.ಕಂಡುಹಿಡಿ(ಗುಂಪು_ದ_Task);
          ಆದರೆ (--(A->ಎರಡನೇ_2) == 0)
            ಸಕ್ರಿಯ_Groups.erase(A);
        }
        // If all tasks are complete, notify any waiting threads.
        ಆದರೆ (workCompletedUnlocked(ಶೂನ್ಯಸೂಚಕ))
          CompletionCondition.notify_all();
      }
    }
  }
}
ತಾರ್ಕಿಕ StdThreadPool::workCompletedUnlocked(Thread_Pool_Task_ಗುಂಪು *ಗುಂಪು) ಸ್ಥಿರಾಂಕ {
  ಆದರೆ (ಗುಂಪು == ಶೂನ್ಯಸೂಚಕ)
    ಹಿಂತಿರುಗಿ !ಸಕ್ರಿಯ_ಎಳೆಗಳು && Tasks.ಖಾಲಿಗಳು();
  ಹಿಂತಿರುಗಿ ಸಕ್ರಿಯ_Groups.ಎಣಿಕೆ_2(ಗುಂಪು) == 0 &&
         !llvm::is_contained(llvm::make_ಎರಡನೇ_range(Tasks), ಗುಂಪು);
}

ಶೂನ್ಯ StdThreadPool::wait() {
  ಪ್ರತಿಜ್ಞಾ(!is_ಕೆಲಸಗಾರ_Thread()); // Would deadlock waiting for itself.
  // Wait for all threads to complete and the queue to be empty
  std::unique_ಬೀಗ<std::mutex> ಬೀಗ_Guard(ಸರತಿ_ಬೀಗ);
  CompletionCondition.wait(ಬೀಗ_Guard,
                           [&] { ಹಿಂತಿರುಗಿ workCompletedUnlocked(ಶೂನ್ಯಸೂಚಕ); });
}

ಶೂನ್ಯ StdThreadPool::wait(Thread_Pool_Task_ಗುಂಪು &ಗುಂಪು) {
  // Wait for all threads in the group to complete.
  ಆದರೆ (!is_ಕೆಲಸಗಾರ_Thread()) {
    std::unique_ಬೀಗ<std::mutex> ಬೀಗ_Guard(ಸರತಿ_ಬೀಗ);
    CompletionCondition.wait(ಬೀಗ_Guard,
                             [&] { ಹಿಂತಿರುಗಿ workCompletedUnlocked(&ಗುಂಪು); });
    ಹಿಂತಿರುಗಿ;
  }
  // Make sure to not deadlock waiting for oneself.
  ಪ್ರತಿಜ್ಞಾ(CurrentThreadTaskGroups == ಶೂನ್ಯಸೂಚಕ ||
         !llvm::is_contained(*CurrentThreadTaskGroups, &ಗುಂಪು));
  // Handle the case of recursive call from another task in a different group,
  // in which case process tasks while waiting to keep the thread busy and avoid
  // possible deadlock.
  processTasks(&ಗುಂಪು);
}

ತಾರ್ಕಿಕ StdThreadPool::is_ಕೆಲಸಗಾರ_Thread() ಸ್ಥಿರಾಂಕ {
  llvm::sys::ScopedReader ಬೀಗ_Guard(ಎಳೆಗಳು_ಬೀಗ);
  llvm::thread::id CurrentThreadId = llvm::this_thread::get_id();
  ಗಾಗಿ (ಸ್ಥಿರಾಂಕ llvm::thread &Thread : ಎಳೆಗಳು)
    ಆದರೆ (CurrentThreadId == Thread.get_id())
      ಹಿಂತಿರುಗಿ ನಿಜ;
  ಹಿಂತಿರುಗಿ ಸುಳ್ಳು;
}

// The destructor joins all threads, waiting for completion.
StdThreadPool::~StdThreadPool() {
  {
    std::unique_ಬೀಗ<std::mutex> ಬೀಗ_Guard(ಸರತಿ_ಬೀಗ);
    EnableFlag = ಸುಳ್ಳು;
  }
  ಸರತಿ_Condition.notify_all();
  llvm::sys::ScopedReader ಬೀಗ_Guard(ಎಳೆಗಳು_ಬೀಗ);
  ಗಾಗಿ (ಸ್ವಯಂಚಾಲಿತ &ಕೆಲಸಗಾರ : ಎಳೆಗಳು)
    ಕೆಲಸಗಾರ.join();
}

#ಆದರೆ_ಅಂತ್ಯ // LLVM_ENABLE_THREADS Disabled

// No threads are launched, issue a warning if ThreadCount is not 0
SingleThreadExecutor::SingleThreadExecutor(ThreadPoolStrategy S) {
  ಪೂರ್ಣಾಂಕ Thread_ಎಣಿಕೆ = S.compute_thread_ಎಣಿಕೆ();
  ಆದರೆ (Thread_ಎಣಿಕೆ != 1) {
    errs() << "Warning: request a ThreadPool with " << Thread_ಎಣಿಕೆ
           << " threads, but LLVM_ENABLE_THREADS has been turned off\n";
  }
}

ಶೂನ್ಯ SingleThreadExecutor::wait() {
  // Sequential implementation running the tasks
  ಆಗ (!Tasks.ಖಾಲಿಗಳು()) {
    ಸ್ವಯಂಚಾಲಿತ Task = std::move(Tasks.front().ಮೊದಲ_2);
    Tasks.ತೆಗೆ_front();
    Task();
  }
}

ಶೂನ್ಯ SingleThreadExecutor::wait(Thread_Pool_Task_ಗುಂಪು &) {
  // Simply wait for all, this works even if recursive (the running task
  // is already removed from the queue).
  wait();
}

ತಾರ್ಕಿಕ SingleThreadExecutor::is_ಕೆಲಸಗಾರ_Thread() ಸ್ಥಿರಾಂಕ {
  report_fatal_error("LLVM compiled without multithreading");
}

SingleThreadExecutor::~SingleThreadExecutor() { wait(); }
