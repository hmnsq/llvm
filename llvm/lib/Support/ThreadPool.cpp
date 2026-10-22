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

#समावेश "llvm/Support/ThreadPool.h"

#समावेश "llvm/Config/llvm-config.h"

#समावेश "llvm/ADT/ScopeExit.h"
#समावेश "llvm/Support/ExponentialBackoff.h"
#समावेश "llvm/Support/FormatVariadic.h"
#समावेश "llvm/Support/Threading.h"
#समावेश "llvm/Support/raw_ostream.h"

using namespace llvm;

ThreadPoolInterface::~ThreadPoolInterface() = शेष;

// A note on thread groups: Tasks are by default in no group (represented
// by nullptr ThreadPoolTaskGroup pointer in the Tasks queue) and functionality
// here normally works on all tasks regardless of their group (functions
// in that case receive nullptr ThreadPoolTaskGroup pointer as argument).
// A task in a group has a pointer to that ThreadPoolTaskGroup in the Tasks
// queue, and functions called to work only on tasks from one group take that
// pointer.

#अगर LLVM_ENABLE_THREADS

StdThreadPool::StdThreadPool(ThreadPoolStrategy S)
    : Strategy(S), Max_Thread_गिनती(S.compute_thread_गिनती()) {
  अगर (Strategy.UseJobserver)
    TheJobserver = JobserverClient::getInstance();
}

व्योम StdThreadPool::grow(पूर्णांक requested) {
  llvm::sys::ScopedWriter LockGuard(ThreadsLock);
  अगर (Threads.आकार_2() >= Max_Thread_गिनती)
    वापस; // Already hit the max thread pool size.
  पूर्णांक new_Thread_गिनती = std::min<पूर्णांक>(requested, Max_Thread_गिनती);
  जबतक (static_cast<पूर्णांक>(Threads.आकार_2()) < new_Thread_गिनती) {
    पूर्णांक ThreadID = Threads.आकार_2();
    Threads.emplace_back([यह, ThreadID] {
      set_thread_name(formatv("llvm-worker-{0}", ThreadID));
      Strategy.apply_thread_strategy(ThreadID);
      // Note on jobserver deadlock avoidance:
      // GNU Make grants each invoked process one implicit job slot.
      // JobserverClient::tryAcquire() returns that implicit slot on the first
      // successful call in a process, ensuring forward progress without a
      // dedicated "always-on" thread.
      अगर (TheJobserver)
        processTasksWithJobserver();
      अन्यथा
        processTasks(nullptr);
    });
  }
}

#अगर_अघोषित न_संशोधन
// The group of the tasks run by the current thread.
अटल LLVM_THREAD_LOCAL std::vector<ThreadPoolTaskGroup *>
    *CurrentThreadTaskGroups = nullptr;
#पूर्ण_अगर

// WaitingForGroup == nullptr means all tasks regardless of their group.
व्योम StdThreadPool::processTasks(ThreadPoolTaskGroup *WaitingForGroup) {
  जबतक (true) {
    llvm::unique_function<व्योम()> Task;
    ThreadPoolTaskGroup *GroupOfTask;
    {
      std::unique_lock<std::mutex> LockGuard(QueueLock);
      bool workCompletedForGroup = false; // Result of workCompletedUnlocked()
      // Wait for tasks to be pushed in the queue
      QueueCondition.रुको(LockGuard, [&] {
        वापस !EnableFlag || !Tasks.empty() ||
               (WaitingForGroup != nullptr &&
                (workCompletedForGroup =
                     workCompletedUnlocked(WaitingForGroup)));
      });
      // Exit condition
      अगर (!EnableFlag && Tasks.empty())
        वापस;
      अगर (WaitingForGroup != nullptr && workCompletedForGroup)
        वापस;
      // Yeah, we have a task, grab it and release the lock on the queue

      // We first need to signal that we are active before popping the queue
      // in order for wait() to properly detect that even if the queue is
      // empty, there is still a task in flight.
      ++ActiveThreads;
      Task = std::move(Tasks.front().first);
      GroupOfTask = Tasks.front().second;
      // Need to count active threads in each group separately, ActiveThreads
      // would never be 0 if waiting for another group inside a wait.
      अगर (GroupOfTask != nullptr)
        ++ActiveGroups[GroupOfTask]; // Increment or set to 1 if new item
      Tasks.निकालो_front();
    }
#अगर_अघोषित न_संशोधन
    अगर (CurrentThreadTaskGroups == nullptr)
      CurrentThreadTaskGroups = नया std::vector<ThreadPoolTaskGroup *>;
    CurrentThreadTaskGroups->धकेलो_back(GroupOfTask);
#पूर्ण_अगर

    // Run the task we just grabbed. This also destroys the task once run to
    // release any resources held by it through RAII captured objects.
    //
    // It is particularly important to do this here so that we're not holding
    // any lock and any further operations on the thread or `ThreadPool` take
    // place here, at the same point as the task itself is executed.
    std::exchange(Task, {})();

#अगर_अघोषित न_संशोधन
    CurrentThreadTaskGroups->निकालो_back();
    अगर (CurrentThreadTaskGroups->empty()) {
      मिटाओ CurrentThreadTaskGroups;
      CurrentThreadTaskGroups = nullptr;
    }
#पूर्ण_अगर

    bool Notify;
    bool NotifyGroup;
    {
      // Adjust `ActiveThreads`, in case someone waits on StdThreadPool::wait()
      std::lock_guard<std::mutex> LockGuard(QueueLock);
      --ActiveThreads;
      अगर (GroupOfTask != nullptr) {
        स्वतः A = ActiveGroups.find(GroupOfTask);
        अगर (--(A->second) == 0)
          ActiveGroups.erase(A);
      }
      Notify = workCompletedUnlocked(GroupOfTask);
      NotifyGroup = GroupOfTask != nullptr && Notify;
    }
    // Notify task completion if this is the last active thread, in case
    // someone waits on StdThreadPool::wait().
    अगर (Notify)
      CompletionCondition.notify_all();
    // If this was a task in a group, notify also threads waiting for tasks
    // in this function on QueueCondition, to make a recursive wait() return
    // after the group it's been waiting for has finished.
    अगर (NotifyGroup)
      QueueCondition.notify_all();
  }
}

/// Main loop for worker threads when using a jobserver.
/// This function uses a two-level queue; it first acquires a job slot from the
/// external jobserver, then retrieves a task from the internal queue.
/// This allows the thread pool to cooperate with build systems like `make -j`.
व्योम StdThreadPool::processTasksWithJobserver() {
  जबतक (true) {
    // Acquire a job slot from the external jobserver.
    // This polls for a slot and yields the thread to avoid a high-CPU wait.
    JobSlot Slot;
    // The timeout for the backoff can be very long, as the shutdown
    // is checked on each iteration. The sleep duration is capped by MaxWait
    // in ExponentialBackoff, so shutdown latency is not a problem.
    ExponentialBackoff Backoff(std::chrono::hours(24));
    bool AcquiredToken = false;
    करो {
      // Return if the thread pool is shutting down.
      {
        std::unique_lock<std::mutex> LockGuard(QueueLock);
        अगर (!EnableFlag)
          वापस;
      }

      Slot = TheJobserver->tryAcquire();
      अगर (Slot.isValid()) {
        AcquiredToken = true;
        अवरोध;
      }
    } जबतक (Backoff.wait_For_अग्र_Attempt());

    अगर (!AcquiredToken) {
      // This is practically unreachable with a 24h timeout and indicates a
      // deeper problem if hit.
      report_fatal_error("Timed out waiting for jobserver token.");
    }

    // `llvm::scope_exit` guarantees the job slot is released, even if the
    // task throws or we exit early. This prevents deadlocking the build.
    llvm::scope_exit SlotReleaser(
        [&] { TheJobserver->release(std::move(Slot)); });

    // While we hold a job slot, process tasks from the internal queue.
    जबतक (true) {
      llvm::unique_function<व्योम()> Task;
      ThreadPoolTaskGroup *GroupOfTask = nullptr;

      {
        std::unique_lock<std::mutex> LockGuard(QueueLock);

        // Wait until a task is available or the pool is shutting down.
        QueueCondition.रुको(LockGuard,
                            [&] { वापस !EnableFlag || !Tasks.empty(); });

        // If shutting down and the queue is empty, the thread can terminate.
        अगर (!EnableFlag && Tasks.empty())
          वापस;

        // If the queue is empty, we're done processing tasks for now.
        // Break the inner loop to release the job slot.
        अगर (Tasks.empty())
          अवरोध;

        // A task is available. Mark it as active before releasing the lock
        // to prevent race conditions with `wait()`.
        ++ActiveThreads;
        Task = std::move(Tasks.front().first);
        GroupOfTask = Tasks.front().second;
        अगर (GroupOfTask != nullptr)
          ++ActiveGroups[GroupOfTask];
        Tasks.निकालो_front();
      } // The queue lock is released.

      // Run the task. The job slot remains acquired during execution.
      Task();

      // The task has finished. Update the active count and notify any waiters.
      {
        std::lock_guard<std::mutex> LockGuard(QueueLock);
        --ActiveThreads;
        अगर (GroupOfTask != nullptr) {
          स्वतः A = ActiveGroups.find(GroupOfTask);
          अगर (--(A->second) == 0)
            ActiveGroups.erase(A);
        }
        // If all tasks are complete, notify any waiting threads.
        अगर (workCompletedUnlocked(nullptr))
          CompletionCondition.notify_all();
      }
    }
  }
}
bool StdThreadPool::workCompletedUnlocked(ThreadPoolTaskGroup *Group) स्थिर {
  अगर (Group == nullptr)
    वापस !ActiveThreads && Tasks.empty();
  वापस ActiveGroups.गिनती(Group) == 0 &&
         !llvm::is_contained(llvm::make_second_range(Tasks), Group);
}

व्योम StdThreadPool::रुको() {
  निश्चित(!isWorkerThread()); // Would deadlock waiting for itself.
  // Wait for all threads to complete and the queue to be empty
  std::unique_lock<std::mutex> LockGuard(QueueLock);
  CompletionCondition.रुको(LockGuard,
                           [&] { वापस workCompletedUnlocked(nullptr); });
}

व्योम StdThreadPool::रुको(ThreadPoolTaskGroup &Group) {
  // Wait for all threads in the group to complete.
  अगर (!isWorkerThread()) {
    std::unique_lock<std::mutex> LockGuard(QueueLock);
    CompletionCondition.रुको(LockGuard,
                             [&] { वापस workCompletedUnlocked(&Group); });
    वापस;
  }
  // Make sure to not deadlock waiting for oneself.
  निश्चित(CurrentThreadTaskGroups == nullptr ||
         !llvm::is_contained(*CurrentThreadTaskGroups, &Group));
  // Handle the case of recursive call from another task in a different group,
  // in which case process tasks while waiting to keep the thread busy and avoid
  // possible deadlock.
  processTasks(&Group);
}

bool StdThreadPool::isWorkerThread() स्थिर {
  llvm::sys::ScopedReader LockGuard(ThreadsLock);
  llvm::thread::id CurrentThreadId = llvm::this_thread::get_id();
  क्रम (स्थिर llvm::thread &Thread : Threads)
    अगर (CurrentThreadId == Thread.get_id())
      वापस true;
  वापस false;
}

// The destructor joins all threads, waiting for completion.
StdThreadPool::~StdThreadPool() {
  {
    std::unique_lock<std::mutex> LockGuard(QueueLock);
    EnableFlag = false;
  }
  QueueCondition.notify_all();
  llvm::sys::ScopedReader LockGuard(ThreadsLock);
  क्रम (स्वतः &Worker : Threads)
    Worker.join();
}

#पूर्ण_अगर // LLVM_ENABLE_THREADS Disabled

// No threads are launched, issue a warning if ThreadCount is not 0
SingleThreadExecutor::SingleThreadExecutor(ThreadPoolStrategy S) {
  पूर्णांक Thread_गिनती = S.compute_thread_गिनती();
  अगर (Thread_गिनती != 1) {
    errs() << "Warning: request a ThreadPool with " << Thread_गिनती
           << " threads, but LLVM_ENABLE_THREADS has been turned off\n";
  }
}

व्योम SingleThreadExecutor::रुको() {
  // Sequential implementation running the tasks
  जबतक (!Tasks.empty()) {
    स्वतः Task = std::move(Tasks.front().first);
    Tasks.निकालो_front();
    Task();
  }
}

व्योम SingleThreadExecutor::रुको(ThreadPoolTaskGroup &) {
  // Simply wait for all, this works even if recursive (the running task
  // is already removed from the queue).
  रुको();
}

bool SingleThreadExecutor::isWorkerThread() स्थिर {
  report_fatal_error("LLVM compiled without multithreading");
}

SingleThreadExecutor::~SingleThreadExecutor() { रुको(); }
