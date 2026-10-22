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

#অন্তর্ভাৱ "llvm/Support/ThreadPool.h"

#অন্তর্ভাৱ "llvm/Config/llvm-config.h"

#অন্তর্ভাৱ "llvm/ADT/ScopeExit.h"
#অন্তর্ভাৱ "llvm/Support/ExponentialBackoff.h"
#অন্তর্ভাৱ "llvm/Support/FormatVariadic.h"
#অন্তর্ভাৱ "llvm/Support/Threading.h"
#অন্তর্ভাৱ "llvm/Support/raw_ostream.h"

using নামাকাশ llvm;

ThreadPoolInterface::~ThreadPoolInterface() = স্ৱভাৱ;

// A note on thread groups: Tasks are by default in no group (represented
// by nullptr ThreadPoolTaskGroup pointer in the Tasks queue) and functionality
// here normally works on all tasks regardless of their group (functions
// in that case receive nullptr ThreadPoolTaskGroup pointer as argument).
// A task in a group has a pointer to that ThreadPoolTaskGroup in the Tasks
// queue, and functions called to work only on tasks from one group take that
// pointer.

#যদি LLVM_ENABLE_THREADS

StdThreadPool::StdThreadPool(ThreadPoolStrategy S)
    : Strategy(S), Max_Thread_গুনতি(S.compute_thread_গুনতি()) {
  যদি (Strategy.UseJobserver)
    TheJobserver = JobserverClient::getInstance();
}

শূন্য StdThreadPool::grow(পূর্ণাঙ্ক requested) {
  llvm::sys::ScopedWriter LockGuard(ThreadsLock);
  যদি (Threads.size() >= Max_Thread_গুনতি)
    ঘূৰাই; // Already hit the max thread pool size.
  পূর্ণাঙ্ক new_Thread_গুনতি = std::min<পূর্ণাঙ্ক>(requested, Max_Thread_গুনতি);
  যেতিয়ালৈ (static_cast<পূর্ণাঙ্ক>(Threads.size()) < new_Thread_গুনতি) {
    পূর্ণাঙ্ক ThreadID = Threads.size();
    Threads.emplace_back([এতত্, ThreadID] {
      set_thread_name(formatv("llvm-worker-{0}", ThreadID));
      Strategy.apply_thread_strategy(ThreadID);
      // Note on jobserver deadlock avoidance:
      // GNU Make grants each invoked process one implicit job slot.
      // JobserverClient::tryAcquire() returns that implicit slot on the first
      // successful call in a process, ensuring forward progress without a
      // dedicated "always-on" thread.
      যদি (TheJobserver)
        processTasksWithJobserver();
      অন্যথা
        processTasks(nullptr);
    });
  }
}

#ifndef NDEBUG
// The group of the tasks run by the current thread.
স্থির LLVM_THREAD_LOCAL std::vector<ThreadPoolTaskGroup *>
    *CurrentThreadTaskGroups = nullptr;
#endif

// WaitingForGroup == nullptr means all tasks regardless of their group.
শূন্য StdThreadPool::processTasks(ThreadPoolTaskGroup *WaitingForGroup) {
  যেতিয়ালৈ (সঁচা) {
    llvm::unique_function<শূন্য()> Task;
    ThreadPoolTaskGroup *GroupOfTask;
    {
      std::unique_lock<std::mutex> LockGuard(QueueLock);
      তার্কিক workCompletedForGroup = মিছা; // Result of workCompletedUnlocked()
      // Wait for tasks to be pushed in the queue
      QueueCondition.wait(LockGuard, [&] {
        ঘূৰাই !EnableFlag || !Tasks.empty() ||
               (WaitingForGroup != nullptr &&
                (workCompletedForGroup =
                     workCompletedUnlocked(WaitingForGroup)));
      });
      // Exit condition
      যদি (!EnableFlag && Tasks.empty())
        ঘূৰাই;
      যদি (WaitingForGroup != nullptr && workCompletedForGroup)
        ঘূৰাই;
      // Yeah, we have a task, grab it and release the lock on the queue

      // We first need to signal that we are active before popping the queue
      // in order for wait() to properly detect that even if the queue is
      // empty, there is still a task in flight.
      ++ActiveThreads;
      Task = std::move(Tasks.front().first);
      GroupOfTask = Tasks.front().second;
      // Need to count active threads in each group separately, ActiveThreads
      // would never be 0 if waiting for another group inside a wait.
      যদি (GroupOfTask != nullptr)
        ++ActiveGroups[GroupOfTask]; // Increment or set to 1 if new item
      Tasks.বেৰ_কৰো_front();
    }
#ifndef NDEBUG
    যদি (CurrentThreadTaskGroups == nullptr)
      CurrentThreadTaskGroups = নূতন std::vector<ThreadPoolTaskGroup *>;
    CurrentThreadTaskGroups->ঠেলো_back(GroupOfTask);
#endif

    // Run the task we just grabbed. This also destroys the task once run to
    // release any resources held by it through RAII captured objects.
    //
    // It is particularly important to do this here so that we're not holding
    // any lock and any further operations on the thread or `ThreadPool` take
    // place here, at the same point as the task itself is executed.
    std::exchange(Task, {})();

#ifndef NDEBUG
    CurrentThreadTaskGroups->বেৰ_কৰো_back();
    যদি (CurrentThreadTaskGroups->empty()) {
      delete CurrentThreadTaskGroups;
      CurrentThreadTaskGroups = nullptr;
    }
#endif

    তার্কিক Notify;
    তার্কিক NotifyGroup;
    {
      // Adjust `ActiveThreads`, in case someone waits on StdThreadPool::wait()
      std::lock_guard<std::mutex> LockGuard(QueueLock);
      --ActiveThreads;
      যদি (GroupOfTask != nullptr) {
        auto A = ActiveGroups.find(GroupOfTask);
        যদি (--(A->second) == 0)
          ActiveGroups.erase(A);
      }
      Notify = workCompletedUnlocked(GroupOfTask);
      NotifyGroup = GroupOfTask != nullptr && Notify;
    }
    // Notify task completion if this is the last active thread, in case
    // someone waits on StdThreadPool::wait().
    যদি (Notify)
      CompletionCondition.notify_all();
    // If this was a task in a group, notify also threads waiting for tasks
    // in this function on QueueCondition, to make a recursive wait() return
    // after the group it's been waiting for has finished.
    যদি (NotifyGroup)
      QueueCondition.notify_all();
  }
}

/// Main loop for worker threads when using a jobserver.
/// This function uses a two-level queue; it first acquires a job slot from the
/// external jobserver, then retrieves a task from the internal queue.
/// This allows the thread pool to cooperate with build systems like `make -j`.
শূন্য StdThreadPool::processTasksWithJobserver() {
  যেতিয়ালৈ (সঁচা) {
    // Acquire a job slot from the external jobserver.
    // This polls for a slot and yields the thread to avoid a high-CPU wait.
    JobSlot Slot;
    // The timeout for the backoff can be very long, as the shutdown
    // is checked on each iteration. The sleep duration is capped by MaxWait
    // in ExponentialBackoff, so shutdown latency is not a problem.
    ExponentialBackoff Backoff(std::chrono::hours(24));
    তার্কিক AcquiredToken = মিছা;
    কৰা {
      // Return if the thread pool is shutting down.
      {
        std::unique_lock<std::mutex> LockGuard(QueueLock);
        যদি (!EnableFlag)
          ঘূৰাই;
      }

      Slot = TheJobserver->tryAcquire();
      যদি (Slot.isValid()) {
        AcquiredToken = সঁচা;
        ভঙ্গ;
      }
    } যেতিয়ালৈ (Backoff.waitForNextAttempt());

    যদি (!AcquiredToken) {
      // This is practically unreachable with a 24h timeout and indicates a
      // deeper problem if hit.
      report_fatal_error("Timed out waiting for jobserver token.");
    }

    // `llvm::scope_exit` guarantees the job slot is released, even if the
    // task throws or we exit early. This prevents deadlocking the build.
    llvm::scope_exit SlotReleaser(
        [&] { TheJobserver->release(std::move(Slot)); });

    // While we hold a job slot, process tasks from the internal queue.
    যেতিয়ালৈ (সঁচা) {
      llvm::unique_function<শূন্য()> Task;
      ThreadPoolTaskGroup *GroupOfTask = nullptr;

      {
        std::unique_lock<std::mutex> LockGuard(QueueLock);

        // Wait until a task is available or the pool is shutting down.
        QueueCondition.wait(LockGuard,
                            [&] { ঘূৰাই !EnableFlag || !Tasks.empty(); });

        // If shutting down and the queue is empty, the thread can terminate.
        যদি (!EnableFlag && Tasks.empty())
          ঘূৰাই;

        // If the queue is empty, we're done processing tasks for now.
        // Break the inner loop to release the job slot.
        যদি (Tasks.empty())
          ভঙ্গ;

        // A task is available. Mark it as active before releasing the lock
        // to prevent race conditions with `wait()`.
        ++ActiveThreads;
        Task = std::move(Tasks.front().first);
        GroupOfTask = Tasks.front().second;
        যদি (GroupOfTask != nullptr)
          ++ActiveGroups[GroupOfTask];
        Tasks.বেৰ_কৰো_front();
      } // The queue lock is released.

      // Run the task. The job slot remains acquired during execution.
      Task();

      // The task has finished. Update the active count and notify any waiters.
      {
        std::lock_guard<std::mutex> LockGuard(QueueLock);
        --ActiveThreads;
        যদি (GroupOfTask != nullptr) {
          auto A = ActiveGroups.find(GroupOfTask);
          যদি (--(A->second) == 0)
            ActiveGroups.erase(A);
        }
        // If all tasks are complete, notify any waiting threads.
        যদি (workCompletedUnlocked(nullptr))
          CompletionCondition.notify_all();
      }
    }
  }
}
তার্কিক StdThreadPool::workCompletedUnlocked(ThreadPoolTaskGroup *Group) ধ্রুৱ {
  যদি (Group == nullptr)
    ঘূৰাই !ActiveThreads && Tasks.empty();
  ঘূৰাই ActiveGroups.গুনতি(Group) == 0 &&
         !llvm::is_contained(llvm::make_second_range(Tasks), Group);
}

শূন্য StdThreadPool::wait() {
  প্রতিজ্ঞা(!isWorkerThread()); // Would deadlock waiting for itself.
  // Wait for all threads to complete and the queue to be empty
  std::unique_lock<std::mutex> LockGuard(QueueLock);
  CompletionCondition.wait(LockGuard,
                           [&] { ঘূৰাই workCompletedUnlocked(nullptr); });
}

শূন্য StdThreadPool::wait(ThreadPoolTaskGroup &Group) {
  // Wait for all threads in the group to complete.
  যদি (!isWorkerThread()) {
    std::unique_lock<std::mutex> LockGuard(QueueLock);
    CompletionCondition.wait(LockGuard,
                             [&] { ঘূৰাই workCompletedUnlocked(&Group); });
    ঘূৰাই;
  }
  // Make sure to not deadlock waiting for oneself.
  প্রতিজ্ঞা(CurrentThreadTaskGroups == nullptr ||
         !llvm::is_contained(*CurrentThreadTaskGroups, &Group));
  // Handle the case of recursive call from another task in a different group,
  // in which case process tasks while waiting to keep the thread busy and avoid
  // possible deadlock.
  processTasks(&Group);
}

তার্কিক StdThreadPool::isWorkerThread() ধ্রুৱ {
  llvm::sys::ScopedReader LockGuard(ThreadsLock);
  llvm::thread::id CurrentThreadId = llvm::this_thread::get_id();
  বাবে (ধ্রুৱ llvm::thread &Thread : Threads)
    যদি (CurrentThreadId == Thread.get_id())
      ঘূৰাই সঁচা;
  ঘূৰাই মিছা;
}

// The destructor joins all threads, waiting for completion.
StdThreadPool::~StdThreadPool() {
  {
    std::unique_lock<std::mutex> LockGuard(QueueLock);
    EnableFlag = মিছা;
  }
  QueueCondition.notify_all();
  llvm::sys::ScopedReader LockGuard(ThreadsLock);
  বাবে (auto &Worker : Threads)
    Worker.join();
}

#endif // LLVM_ENABLE_THREADS Disabled

// No threads are launched, issue a warning if ThreadCount is not 0
SingleThreadExecutor::SingleThreadExecutor(ThreadPoolStrategy S) {
  পূর্ণাঙ্ক Thread_গুনতি = S.compute_thread_গুনতি();
  যদি (Thread_গুনতি != 1) {
    errs() << "Warning: request a ThreadPool with " << Thread_গুনতি
           << " threads, but LLVM_ENABLE_THREADS has been turned off\n";
  }
}

শূন্য SingleThreadExecutor::wait() {
  // Sequential implementation running the tasks
  যেতিয়ালৈ (!Tasks.empty()) {
    auto Task = std::move(Tasks.front().first);
    Tasks.বেৰ_কৰো_front();
    Task();
  }
}

শূন্য SingleThreadExecutor::wait(ThreadPoolTaskGroup &) {
  // Simply wait for all, this works even if recursive (the running task
  // is already removed from the queue).
  wait();
}

তার্কিক SingleThreadExecutor::isWorkerThread() ধ্রুৱ {
  report_fatal_error("LLVM compiled without multithreading");
}

SingleThreadExecutor::~SingleThreadExecutor() { wait(); }
