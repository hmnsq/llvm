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

#अन्तर्भाव "llvm/Support/ThreadPool.h"

#अन्तर्भाव "llvm/Config/llvm-config.h"

#अन्तर्भाव "llvm/ADT/ScopeExit.h"
#अन्तर्भाव "llvm/Support/ExponentialBackoff.h"
#अन्तर्भाव "llvm/Support/FormatVariadic.h"
#अन्तर्भाव "llvm/Support/Threading.h"
#अन्तर्भाव "llvm/Support/raw_ostream.h"

using नामाकाश llvm;

ThreadPoolInterface::~ThreadPoolInterface() = पूर्वनिर्धारित;

// A note on thread groups: Tasks are by default in no group (represented
// by nullptr ThreadPoolTaskGroup pointer in the Tasks queue) and functionality
// here normally works on all tasks regardless of their group (functions
// in that case receive nullptr ThreadPoolTaskGroup pointer as argument).
// A task in a group has a pointer to that ThreadPoolTaskGroup in the Tasks
// queue, and functions called to work only on tasks from one group take that
// pointer.

#जर LLVM_ENABLE_THREADS

StdThreadPool::StdThreadPool(ThreadPoolStrategy S)
    : Strategy(S), Max_Thread_मोजणी(S.compute_thread_मोजणी()) {
  जर (Strategy.UseJobserver)
    TheJobserver = JobserverClient::getInstance();
}

शून्य StdThreadPool::grow(पूर्णांक requested) {
  llvm::sys::ScopedWriter LockGuard(ThreadsLock);
  जर (Threads.size() >= Max_Thread_मोजणी)
    परत; // Already hit the max thread pool size.
  पूर्णांक new_Thread_मोजणी = std::min<पूर्णांक>(requested, Max_Thread_मोजणी);
  पर्यंत (static_cast<पूर्णांक>(Threads.size()) < new_Thread_मोजणी) {
    पूर्णांक ThreadID = Threads.size();
    Threads.emplace_back([हे, ThreadID] {
      set_thread_name(formatv("llvm-worker-{0}", ThreadID));
      Strategy.apply_thread_strategy(ThreadID);
      // Note on jobserver deadlock avoidance:
      // GNU Make grants each invoked process one implicit job slot.
      // JobserverClient::tryAcquire() returns that implicit slot on the first
      // successful call in a process, ensuring forward progress without a
      // dedicated "always-on" thread.
      जर (TheJobserver)
        processTasksWithJobserver();
      अन्यथा
        processTasks(nullptr);
    });
  }
}

#ifndef न_संशोधन
// The group of the tasks run by the current thread.
स्थिर LLVM_THREAD_LOCAL std::vector<ThreadPoolTaskGroup *>
    *CurrentThreadTaskGroups = nullptr;
#endif

// WaitingForGroup == nullptr means all tasks regardless of their group.
शून्य StdThreadPool::processTasks(ThreadPoolTaskGroup *WaitingForGroup) {
  पर्यंत (सत्य) {
    llvm::unique_function<शून्य()> Task;
    ThreadPoolTaskGroup *GroupOfTask;
    {
      std::unique_lock<std::mutex> LockGuard(QueueLock);
      तार्किक workCompletedForGroup = असत्य; // Result of workCompletedUnlocked()
      // Wait for tasks to be pushed in the queue
      QueueCondition.रुको(LockGuard, [&] {
        परत !EnableFlag || !Tasks.empty() ||
               (WaitingForGroup != nullptr &&
                (workCompletedForGroup =
                     workCompletedUnlocked(WaitingForGroup)));
      });
      // Exit condition
      जर (!EnableFlag && Tasks.empty())
        परत;
      जर (WaitingForGroup != nullptr && workCompletedForGroup)
        परत;
      // Yeah, we have a task, grab it and release the lock on the queue

      // We first need to signal that we are active before popping the queue
      // in order for wait() to properly detect that even if the queue is
      // empty, there is still a task in flight.
      ++ActiveThreads;
      Task = std::move(Tasks.front().first);
      GroupOfTask = Tasks.front().second;
      // Need to count active threads in each group separately, ActiveThreads
      // would never be 0 if waiting for another group inside a wait.
      जर (GroupOfTask != nullptr)
        ++ActiveGroups[GroupOfTask]; // Increment or set to 1 if new item
      Tasks.काढा_front();
    }
#ifndef न_संशोधन
    जर (CurrentThreadTaskGroups == nullptr)
      CurrentThreadTaskGroups = नवीन std::vector<ThreadPoolTaskGroup *>;
    CurrentThreadTaskGroups->ढकला_back(GroupOfTask);
#endif

    // Run the task we just grabbed. This also destroys the task once run to
    // release any resources held by it through RAII captured objects.
    //
    // It is particularly important to do this here so that we're not holding
    // any lock and any further operations on the thread or `ThreadPool` take
    // place here, at the same point as the task itself is executed.
    std::exchange(Task, {})();

#ifndef न_संशोधन
    CurrentThreadTaskGroups->काढा_back();
    जर (CurrentThreadTaskGroups->empty()) {
      delete CurrentThreadTaskGroups;
      CurrentThreadTaskGroups = nullptr;
    }
#endif

    तार्किक Notify;
    तार्किक NotifyGroup;
    {
      // Adjust `ActiveThreads`, in case someone waits on StdThreadPool::wait()
      std::lock_guard<std::mutex> LockGuard(QueueLock);
      --ActiveThreads;
      जर (GroupOfTask != nullptr) {
        स्वतः A = ActiveGroups.find(GroupOfTask);
        जर (--(A->second) == 0)
          ActiveGroups.erase(A);
      }
      Notify = workCompletedUnlocked(GroupOfTask);
      NotifyGroup = GroupOfTask != nullptr && Notify;
    }
    // Notify task completion if this is the last active thread, in case
    // someone waits on StdThreadPool::wait().
    जर (Notify)
      CompletionCondition.notify_all();
    // If this was a task in a group, notify also threads waiting for tasks
    // in this function on QueueCondition, to make a recursive wait() return
    // after the group it's been waiting for has finished.
    जर (NotifyGroup)
      QueueCondition.notify_all();
  }
}

/// Main loop for worker threads when using a jobserver.
/// This function uses a two-level queue; it first acquires a job slot from the
/// external jobserver, then retrieves a task from the internal queue.
/// This allows the thread pool to cooperate with build systems like `make -j`.
शून्य StdThreadPool::processTasksWithJobserver() {
  पर्यंत (सत्य) {
    // Acquire a job slot from the external jobserver.
    // This polls for a slot and yields the thread to avoid a high-CPU wait.
    JobSlot Slot;
    // The timeout for the backoff can be very long, as the shutdown
    // is checked on each iteration. The sleep duration is capped by MaxWait
    // in ExponentialBackoff, so shutdown latency is not a problem.
    ExponentialBackoff Backoff(std::chrono::hours(24));
    तार्किक AcquiredToken = असत्य;
    करा {
      // Return if the thread pool is shutting down.
      {
        std::unique_lock<std::mutex> LockGuard(QueueLock);
        जर (!EnableFlag)
          परत;
      }

      Slot = TheJobserver->tryAcquire();
      जर (Slot.isValid()) {
        AcquiredToken = सत्य;
        थांबा;
      }
    } पर्यंत (Backoff.waitForNextAttempt());

    जर (!AcquiredToken) {
      // This is practically unreachable with a 24h timeout and indicates a
      // deeper problem if hit.
      report_fatal_error("Timed out waiting for jobserver token.");
    }

    // `llvm::scope_exit` guarantees the job slot is released, even if the
    // task throws or we exit early. This prevents deadlocking the build.
    llvm::scope_exit SlotReleaser(
        [&] { TheJobserver->release(std::move(Slot)); });

    // While we hold a job slot, process tasks from the internal queue.
    पर्यंत (सत्य) {
      llvm::unique_function<शून्य()> Task;
      ThreadPoolTaskGroup *GroupOfTask = nullptr;

      {
        std::unique_lock<std::mutex> LockGuard(QueueLock);

        // Wait until a task is available or the pool is shutting down.
        QueueCondition.रुको(LockGuard,
                            [&] { परत !EnableFlag || !Tasks.empty(); });

        // If shutting down and the queue is empty, the thread can terminate.
        जर (!EnableFlag && Tasks.empty())
          परत;

        // If the queue is empty, we're done processing tasks for now.
        // Break the inner loop to release the job slot.
        जर (Tasks.empty())
          थांबा;

        // A task is available. Mark it as active before releasing the lock
        // to prevent race conditions with `wait()`.
        ++ActiveThreads;
        Task = std::move(Tasks.front().first);
        GroupOfTask = Tasks.front().second;
        जर (GroupOfTask != nullptr)
          ++ActiveGroups[GroupOfTask];
        Tasks.काढा_front();
      } // The queue lock is released.

      // Run the task. The job slot remains acquired during execution.
      Task();

      // The task has finished. Update the active count and notify any waiters.
      {
        std::lock_guard<std::mutex> LockGuard(QueueLock);
        --ActiveThreads;
        जर (GroupOfTask != nullptr) {
          स्वतः A = ActiveGroups.find(GroupOfTask);
          जर (--(A->second) == 0)
            ActiveGroups.erase(A);
        }
        // If all tasks are complete, notify any waiting threads.
        जर (workCompletedUnlocked(nullptr))
          CompletionCondition.notify_all();
      }
    }
  }
}
तार्किक StdThreadPool::workCompletedUnlocked(ThreadPoolTaskGroup *Group) स्थिरांक {
  जर (Group == nullptr)
    परत !ActiveThreads && Tasks.empty();
  परत ActiveGroups.मोजणी(Group) == 0 &&
         !llvm::is_contained(llvm::make_second_range(Tasks), Group);
}

शून्य StdThreadPool::रुको() {
  प्रतिज्ञा(!isWorkerThread()); // Would deadlock waiting for itself.
  // Wait for all threads to complete and the queue to be empty
  std::unique_lock<std::mutex> LockGuard(QueueLock);
  CompletionCondition.रुको(LockGuard,
                           [&] { परत workCompletedUnlocked(nullptr); });
}

शून्य StdThreadPool::रुको(ThreadPoolTaskGroup &Group) {
  // Wait for all threads in the group to complete.
  जर (!isWorkerThread()) {
    std::unique_lock<std::mutex> LockGuard(QueueLock);
    CompletionCondition.रुको(LockGuard,
                             [&] { परत workCompletedUnlocked(&Group); });
    परत;
  }
  // Make sure to not deadlock waiting for oneself.
  प्रतिज्ञा(CurrentThreadTaskGroups == nullptr ||
         !llvm::is_contained(*CurrentThreadTaskGroups, &Group));
  // Handle the case of recursive call from another task in a different group,
  // in which case process tasks while waiting to keep the thread busy and avoid
  // possible deadlock.
  processTasks(&Group);
}

तार्किक StdThreadPool::isWorkerThread() स्थिरांक {
  llvm::sys::ScopedReader LockGuard(ThreadsLock);
  llvm::thread::id CurrentThreadId = llvm::this_thread::get_id();
  साठी (स्थिरांक llvm::thread &Thread : Threads)
    जर (CurrentThreadId == Thread.get_id())
      परत सत्य;
  परत असत्य;
}

// The destructor joins all threads, waiting for completion.
StdThreadPool::~StdThreadPool() {
  {
    std::unique_lock<std::mutex> LockGuard(QueueLock);
    EnableFlag = असत्य;
  }
  QueueCondition.notify_all();
  llvm::sys::ScopedReader LockGuard(ThreadsLock);
  साठी (स्वतः &Worker : Threads)
    Worker.join();
}

#endif // LLVM_ENABLE_THREADS Disabled

// No threads are launched, issue a warning if ThreadCount is not 0
SingleThreadExecutor::SingleThreadExecutor(ThreadPoolStrategy S) {
  पूर्णांक Thread_मोजणी = S.compute_thread_मोजणी();
  जर (Thread_मोजणी != 1) {
    errs() << "Warning: request a ThreadPool with " << Thread_मोजणी
           << " threads, but LLVM_ENABLE_THREADS has been turned off\n";
  }
}

शून्य SingleThreadExecutor::रुको() {
  // Sequential implementation running the tasks
  पर्यंत (!Tasks.empty()) {
    स्वतः Task = std::move(Tasks.front().first);
    Tasks.काढा_front();
    Task();
  }
}

शून्य SingleThreadExecutor::रुको(ThreadPoolTaskGroup &) {
  // Simply wait for all, this works even if recursive (the running task
  // is already removed from the queue).
  रुको();
}

तार्किक SingleThreadExecutor::isWorkerThread() स्थिरांक {
  report_fatal_error("LLVM compiled without multithreading");
}

SingleThreadExecutor::~SingleThreadExecutor() { रुको(); }
