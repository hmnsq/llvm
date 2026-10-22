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

#ଅନ୍ତର୍ଭାଵ "llvm/Support/ThreadPool.h"

#ଅନ୍ତର୍ଭାଵ "llvm/Config/llvm-config.h"

#ଅନ୍ତର୍ଭାଵ "llvm/ADT/ScopeExit.h"
#ଅନ୍ତର୍ଭାଵ "llvm/Support/ExponentialBackoff.h"
#ଅନ୍ତର୍ଭାଵ "llvm/Support/FormatVariadic.h"
#ଅନ୍ତର୍ଭାଵ "llvm/Support/Threading.h"
#ଅନ୍ତର୍ଭାଵ "llvm/Support/raw_ostream.h"

using ନାମାକାଶ llvm;

ThreadPoolInterface::~ThreadPoolInterface() = ପୂର୍ବନିର୍ଦ୍ଧାରିତ;

// A note on thread groups: Tasks are by default in no group (represented
// by nullptr ThreadPoolTaskGroup pointer in the Tasks queue) and functionality
// here normally works on all tasks regardless of their group (functions
// in that case receive nullptr ThreadPoolTaskGroup pointer as argument).
// A task in a group has a pointer to that ThreadPoolTaskGroup in the Tasks
// queue, and functions called to work only on tasks from one group take that
// pointer.

#ଯଦି LLVM_ENABLE_THREADS

StdThreadPool::StdThreadPool(ThreadPoolStrategy S)
    : Strategy(S), Max_Thread_ଗଣନା(S.compute_thread_ଗଣନା()) {
  ଯଦି (Strategy.UseJobserver)
    TheJobserver = JobserverClient::getInstance();
}

ଶୂନ୍ୟ StdThreadPool::grow(ପୂର୍ଣ୍ଣସଂଖ୍ୟା requested) {
  llvm::sys::ScopedWriter LockGuard(ThreadsLock);
  ଯଦି (Threads.size() >= Max_Thread_ଗଣନା)
    ଫେରନ୍ତୁ; // Already hit the max thread pool size.
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା new_Thread_ଗଣନା = std::min<ପୂର୍ଣ୍ଣସଂଖ୍ୟା>(requested, Max_Thread_ଗଣନା);
  ଯେତେବେଳେ (static_cast<ପୂର୍ଣ୍ଣସଂଖ୍ୟା>(Threads.size()) < new_Thread_ଗଣନା) {
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା ThreadID = Threads.size();
    Threads.emplace_back([ଏହି, ThreadID] {
      set_thread_name(formatv("llvm-worker-{0}", ThreadID));
      Strategy.apply_thread_strategy(ThreadID);
      // Note on jobserver deadlock avoidance:
      // GNU Make grants each invoked process one implicit job slot.
      // JobserverClient::tryAcquire() returns that implicit slot on the first
      // successful call in a process, ensuring forward progress without a
      // dedicated "always-on" thread.
      ଯଦି (TheJobserver)
        processTasksWithJobserver();
      ଅନ୍ୟଥା
        processTasks(nullptr);
    });
  }
}

#ifndef NDEBUG
// The group of the tasks run by the current thread.
ସ୍ଥିର LLVM_THREAD_LOCAL std::vector<ThreadPoolTaskGroup *>
    *CurrentThreadTaskGroups = nullptr;
#endif

// WaitingForGroup == nullptr means all tasks regardless of their group.
ଶୂନ୍ୟ StdThreadPool::processTasks(ThreadPoolTaskGroup *WaitingForGroup) {
  ଯେତେବେଳେ (ସତ୍ୟ) {
    llvm::unique_function<ଶୂନ୍ୟ()> Task;
    ThreadPoolTaskGroup *GroupOfTask;
    {
      std::unique_lock<std::mutex> LockGuard(QueueLock);
      ତାର୍କିକ workCompletedForGroup = ମିଥ୍ୟା; // Result of workCompletedUnlocked()
      // Wait for tasks to be pushed in the queue
      QueueCondition.wait(LockGuard, [&] {
        ଫେରନ୍ତୁ !EnableFlag || !Tasks.empty() ||
               (WaitingForGroup != nullptr &&
                (workCompletedForGroup =
                     workCompletedUnlocked(WaitingForGroup)));
      });
      // Exit condition
      ଯଦି (!EnableFlag && Tasks.empty())
        ଫେରନ୍ତୁ;
      ଯଦି (WaitingForGroup != nullptr && workCompletedForGroup)
        ଫେରନ୍ତୁ;
      // Yeah, we have a task, grab it and release the lock on the queue

      // We first need to signal that we are active before popping the queue
      // in order for wait() to properly detect that even if the queue is
      // empty, there is still a task in flight.
      ++ActiveThreads;
      Task = std::move(Tasks.front().first);
      GroupOfTask = Tasks.front().second;
      // Need to count active threads in each group separately, ActiveThreads
      // would never be 0 if waiting for another group inside a wait.
      ଯଦି (GroupOfTask != nullptr)
        ++ActiveGroups[GroupOfTask]; // Increment or set to 1 if new item
      Tasks.କାଢ଼_front();
    }
#ifndef NDEBUG
    ଯଦି (CurrentThreadTaskGroups == nullptr)
      CurrentThreadTaskGroups = ନୂତନ std::vector<ThreadPoolTaskGroup *>;
    CurrentThreadTaskGroups->ଠେଲ_back(GroupOfTask);
#endif

    // Run the task we just grabbed. This also destroys the task once run to
    // release any resources held by it through RAII captured objects.
    //
    // It is particularly important to do this here so that we're not holding
    // any lock and any further operations on the thread or `ThreadPool` take
    // place here, at the same point as the task itself is executed.
    std::exchange(Task, {})();

#ifndef NDEBUG
    CurrentThreadTaskGroups->କାଢ଼_back();
    ଯଦି (CurrentThreadTaskGroups->empty()) {
      delete CurrentThreadTaskGroups;
      CurrentThreadTaskGroups = nullptr;
    }
#endif

    ତାର୍କିକ Notify;
    ତାର୍କିକ NotifyGroup;
    {
      // Adjust `ActiveThreads`, in case someone waits on StdThreadPool::wait()
      std::lock_guard<std::mutex> LockGuard(QueueLock);
      --ActiveThreads;
      ଯଦି (GroupOfTask != nullptr) {
        auto A = ActiveGroups.find(GroupOfTask);
        ଯଦି (--(A->second) == 0)
          ActiveGroups.erase(A);
      }
      Notify = workCompletedUnlocked(GroupOfTask);
      NotifyGroup = GroupOfTask != nullptr && Notify;
    }
    // Notify task completion if this is the last active thread, in case
    // someone waits on StdThreadPool::wait().
    ଯଦି (Notify)
      CompletionCondition.notify_all();
    // If this was a task in a group, notify also threads waiting for tasks
    // in this function on QueueCondition, to make a recursive wait() return
    // after the group it's been waiting for has finished.
    ଯଦି (NotifyGroup)
      QueueCondition.notify_all();
  }
}

/// Main loop for worker threads when using a jobserver.
/// This function uses a two-level queue; it first acquires a job slot from the
/// external jobserver, then retrieves a task from the internal queue.
/// This allows the thread pool to cooperate with build systems like `make -j`.
ଶୂନ୍ୟ StdThreadPool::processTasksWithJobserver() {
  ଯେତେବେଳେ (ସତ୍ୟ) {
    // Acquire a job slot from the external jobserver.
    // This polls for a slot and yields the thread to avoid a high-CPU wait.
    JobSlot Slot;
    // The timeout for the backoff can be very long, as the shutdown
    // is checked on each iteration. The sleep duration is capped by MaxWait
    // in ExponentialBackoff, so shutdown latency is not a problem.
    ExponentialBackoff Backoff(std::chrono::hours(24));
    ତାର୍କିକ AcquiredToken = ମିଥ୍ୟା;
    କର {
      // Return if the thread pool is shutting down.
      {
        std::unique_lock<std::mutex> LockGuard(QueueLock);
        ଯଦି (!EnableFlag)
          ଫେରନ୍ତୁ;
      }

      Slot = TheJobserver->tryAcquire();
      ଯଦି (Slot.isValid()) {
        AcquiredToken = ସତ୍ୟ;
        ଭାଙ୍ଗ;
      }
    } ଯେତେବେଳେ (Backoff.waitForNextAttempt());

    ଯଦି (!AcquiredToken) {
      // This is practically unreachable with a 24h timeout and indicates a
      // deeper problem if hit.
      report_fatal_error("Timed out waiting for jobserver token.");
    }

    // `llvm::scope_exit` guarantees the job slot is released, even if the
    // task throws or we exit early. This prevents deadlocking the build.
    llvm::scope_exit SlotReleaser(
        [&] { TheJobserver->release(std::move(Slot)); });

    // While we hold a job slot, process tasks from the internal queue.
    ଯେତେବେଳେ (ସତ୍ୟ) {
      llvm::unique_function<ଶୂନ୍ୟ()> Task;
      ThreadPoolTaskGroup *GroupOfTask = nullptr;

      {
        std::unique_lock<std::mutex> LockGuard(QueueLock);

        // Wait until a task is available or the pool is shutting down.
        QueueCondition.wait(LockGuard,
                            [&] { ଫେରନ୍ତୁ !EnableFlag || !Tasks.empty(); });

        // If shutting down and the queue is empty, the thread can terminate.
        ଯଦି (!EnableFlag && Tasks.empty())
          ଫେରନ୍ତୁ;

        // If the queue is empty, we're done processing tasks for now.
        // Break the inner loop to release the job slot.
        ଯଦି (Tasks.empty())
          ଭାଙ୍ଗ;

        // A task is available. Mark it as active before releasing the lock
        // to prevent race conditions with `wait()`.
        ++ActiveThreads;
        Task = std::move(Tasks.front().first);
        GroupOfTask = Tasks.front().second;
        ଯଦି (GroupOfTask != nullptr)
          ++ActiveGroups[GroupOfTask];
        Tasks.କାଢ଼_front();
      } // The queue lock is released.

      // Run the task. The job slot remains acquired during execution.
      Task();

      // The task has finished. Update the active count and notify any waiters.
      {
        std::lock_guard<std::mutex> LockGuard(QueueLock);
        --ActiveThreads;
        ଯଦି (GroupOfTask != nullptr) {
          auto A = ActiveGroups.find(GroupOfTask);
          ଯଦି (--(A->second) == 0)
            ActiveGroups.erase(A);
        }
        // If all tasks are complete, notify any waiting threads.
        ଯଦି (workCompletedUnlocked(nullptr))
          CompletionCondition.notify_all();
      }
    }
  }
}
ତାର୍କିକ StdThreadPool::workCompletedUnlocked(ThreadPoolTaskGroup *Group) ଅଚଳ {
  ଯଦି (Group == nullptr)
    ଫେରନ୍ତୁ !ActiveThreads && Tasks.empty();
  ଫେରନ୍ତୁ ActiveGroups.ଗଣନା_2(Group) == 0 &&
         !llvm::is_contained(llvm::make_second_range(Tasks), Group);
}

ଶୂନ୍ୟ StdThreadPool::wait() {
  ପ୍ରତିଜ୍ଞା(!isWorkerThread()); // Would deadlock waiting for itself.
  // Wait for all threads to complete and the queue to be empty
  std::unique_lock<std::mutex> LockGuard(QueueLock);
  CompletionCondition.wait(LockGuard,
                           [&] { ଫେରନ୍ତୁ workCompletedUnlocked(nullptr); });
}

ଶୂନ୍ୟ StdThreadPool::wait(ThreadPoolTaskGroup &Group) {
  // Wait for all threads in the group to complete.
  ଯଦି (!isWorkerThread()) {
    std::unique_lock<std::mutex> LockGuard(QueueLock);
    CompletionCondition.wait(LockGuard,
                             [&] { ଫେରନ୍ତୁ workCompletedUnlocked(&Group); });
    ଫେରନ୍ତୁ;
  }
  // Make sure to not deadlock waiting for oneself.
  ପ୍ରତିଜ୍ଞା(CurrentThreadTaskGroups == nullptr ||
         !llvm::is_contained(*CurrentThreadTaskGroups, &Group));
  // Handle the case of recursive call from another task in a different group,
  // in which case process tasks while waiting to keep the thread busy and avoid
  // possible deadlock.
  processTasks(&Group);
}

ତାର୍କିକ StdThreadPool::isWorkerThread() ଅଚଳ {
  llvm::sys::ScopedReader LockGuard(ThreadsLock);
  llvm::thread::id CurrentThreadId = llvm::this_thread::get_id();
  ପାଇଁ (ଅଚଳ llvm::thread &Thread : Threads)
    ଯଦି (CurrentThreadId == Thread.get_id())
      ଫେରନ୍ତୁ ସତ୍ୟ;
  ଫେରନ୍ତୁ ମିଥ୍ୟା;
}

// The destructor joins all threads, waiting for completion.
StdThreadPool::~StdThreadPool() {
  {
    std::unique_lock<std::mutex> LockGuard(QueueLock);
    EnableFlag = ମିଥ୍ୟା;
  }
  QueueCondition.notify_all();
  llvm::sys::ScopedReader LockGuard(ThreadsLock);
  ପାଇଁ (auto &Worker : Threads)
    Worker.join();
}

#endif // LLVM_ENABLE_THREADS Disabled

// No threads are launched, issue a warning if ThreadCount is not 0
SingleThreadExecutor::SingleThreadExecutor(ThreadPoolStrategy S) {
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା Thread_ଗଣନା = S.compute_thread_ଗଣନା();
  ଯଦି (Thread_ଗଣନା != 1) {
    errs() << "Warning: request a ThreadPool with " << Thread_ଗଣନା
           << " threads, but LLVM_ENABLE_THREADS has been turned off\n";
  }
}

ଶୂନ୍ୟ SingleThreadExecutor::wait() {
  // Sequential implementation running the tasks
  ଯେତେବେଳେ (!Tasks.empty()) {
    auto Task = std::move(Tasks.front().first);
    Tasks.କାଢ଼_front();
    Task();
  }
}

ଶୂନ୍ୟ SingleThreadExecutor::wait(ThreadPoolTaskGroup &) {
  // Simply wait for all, this works even if recursive (the running task
  // is already removed from the queue).
  wait();
}

ତାର୍କିକ SingleThreadExecutor::isWorkerThread() ଅଚଳ {
  report_fatal_error("LLVM compiled without multithreading");
}

SingleThreadExecutor::~SingleThreadExecutor() { wait(); }
