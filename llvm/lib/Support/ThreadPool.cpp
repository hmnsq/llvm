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

#അന്തര്ഭാവ "llvm/Support/ThreadPool.h"

#അന്തര്ഭാവ "llvm/Config/llvm-config.h"

#അന്തര്ഭാവ "llvm/ADT/ScopeExit.h"
#അന്തര്ഭാവ "llvm/Support/ExponentialBackoff.h"
#അന്തര്ഭാവ "llvm/Support/FormatVariadic.h"
#അന്തര്ഭാവ "llvm/Support/Threading.h"
#അന്തര്ഭാവ "llvm/Support/raw_ostream.h"

using നാമാകാശ llvm;

ThreadPoolInterface::~ThreadPoolInterface() = സ്ഥിരസ്ഥിതി;

// A note on thread groups: Tasks are by default in no group (represented
// by nullptr ThreadPoolTaskGroup pointer in the Tasks queue) and functionality
// here normally works on all tasks regardless of their group (functions
// in that case receive nullptr ThreadPoolTaskGroup pointer as argument).
// A task in a group has a pointer to that ThreadPoolTaskGroup in the Tasks
// queue, and functions called to work only on tasks from one group take that
// pointer.

#എങ്കിൽ LLVM_ENABLE_THREADS

StdThreadPool::StdThreadPool(ThreadPoolStrategy S)
    : Strategy(S), Max_Thread_എണ്ണം(S.compute_thread_എണ്ണം()) {
  എങ്കിൽ (Strategy.UseJobserver)
    TheJobserver = JobserverClient::getInstance();
}

ശൂന്യം StdThreadPool::grow(പൂർണ്ണസംഖ്യ requested) {
  llvm::sys::ScopedWriter LockGuard(ThreadsLock);
  എങ്കിൽ (Threads.size() >= Max_Thread_എണ്ണം)
    തിരികെ; // Already hit the max thread pool size.
  പൂർണ്ണസംഖ്യ new_Thread_എണ്ണം = std::min<പൂർണ്ണസംഖ്യ>(requested, Max_Thread_എണ്ണം);
  ആകുമ്പോൾ (static_cast<പൂർണ്ണസംഖ്യ>(Threads.size()) < new_Thread_എണ്ണം) {
    പൂർണ്ണസംഖ്യ ThreadID = Threads.size();
    Threads.emplace_back([ഇത്, ThreadID] {
      set_thread_name(formatv("llvm-worker-{0}", ThreadID));
      Strategy.apply_thread_strategy(ThreadID);
      // Note on jobserver deadlock avoidance:
      // GNU Make grants each invoked process one implicit job slot.
      // JobserverClient::tryAcquire() returns that implicit slot on the first
      // successful call in a process, ensuring forward progress without a
      // dedicated "always-on" thread.
      എങ്കിൽ (TheJobserver)
        processTasksWithJobserver();
      അല്ലെങ്കിൽ
        processTasks(nullptr);
    });
  }
}

#ifndef NDEBUG
// The group of the tasks run by the current thread.
സ്ഥിതം LLVM_THREAD_LOCAL std::vector<ThreadPoolTaskGroup *>
    *CurrentThreadTaskGroups = nullptr;
#endif

// WaitingForGroup == nullptr means all tasks regardless of their group.
ശൂന്യം StdThreadPool::processTasks(ThreadPoolTaskGroup *WaitingForGroup) {
  ആകുമ്പോൾ (സത്യം) {
    llvm::unique_function<ശൂന്യം()> Task;
    ThreadPoolTaskGroup *GroupOfTask;
    {
      std::unique_lock<std::mutex> LockGuard(QueueLock);
      തർക്കം workCompletedForGroup = അസത്യം; // Result of workCompletedUnlocked()
      // Wait for tasks to be pushed in the queue
      QueueCondition.wait(LockGuard, [&] {
        തിരികെ !EnableFlag || !Tasks.empty() ||
               (WaitingForGroup != nullptr &&
                (workCompletedForGroup =
                     workCompletedUnlocked(WaitingForGroup)));
      });
      // Exit condition
      എങ്കിൽ (!EnableFlag && Tasks.empty())
        തിരികെ;
      എങ്കിൽ (WaitingForGroup != nullptr && workCompletedForGroup)
        തിരികെ;
      // Yeah, we have a task, grab it and release the lock on the queue

      // We first need to signal that we are active before popping the queue
      // in order for wait() to properly detect that even if the queue is
      // empty, there is still a task in flight.
      ++ActiveThreads;
      Task = std::move(Tasks.front().first);
      GroupOfTask = Tasks.front().second;
      // Need to count active threads in each group separately, ActiveThreads
      // would never be 0 if waiting for another group inside a wait.
      എങ്കിൽ (GroupOfTask != nullptr)
        ++ActiveGroups[GroupOfTask]; // Increment or set to 1 if new item
      Tasks.എടുക്കുക_front();
    }
#ifndef NDEBUG
    എങ്കിൽ (CurrentThreadTaskGroups == nullptr)
      CurrentThreadTaskGroups = പുതിയ std::vector<ThreadPoolTaskGroup *>;
    CurrentThreadTaskGroups->തള്ളുക_back(GroupOfTask);
#endif

    // Run the task we just grabbed. This also destroys the task once run to
    // release any resources held by it through RAII captured objects.
    //
    // It is particularly important to do this here so that we're not holding
    // any lock and any further operations on the thread or `ThreadPool` take
    // place here, at the same point as the task itself is executed.
    std::exchange(Task, {})();

#ifndef NDEBUG
    CurrentThreadTaskGroups->എടുക്കുക_back();
    എങ്കിൽ (CurrentThreadTaskGroups->empty()) {
      delete CurrentThreadTaskGroups;
      CurrentThreadTaskGroups = nullptr;
    }
#endif

    തർക്കം Notify;
    തർക്കം NotifyGroup;
    {
      // Adjust `ActiveThreads`, in case someone waits on StdThreadPool::wait()
      std::lock_guard<std::mutex> LockGuard(QueueLock);
      --ActiveThreads;
      എങ്കിൽ (GroupOfTask != nullptr) {
        auto A = ActiveGroups.find(GroupOfTask);
        എങ്കിൽ (--(A->second) == 0)
          ActiveGroups.erase(A);
      }
      Notify = workCompletedUnlocked(GroupOfTask);
      NotifyGroup = GroupOfTask != nullptr && Notify;
    }
    // Notify task completion if this is the last active thread, in case
    // someone waits on StdThreadPool::wait().
    എങ്കിൽ (Notify)
      CompletionCondition.notify_all();
    // If this was a task in a group, notify also threads waiting for tasks
    // in this function on QueueCondition, to make a recursive wait() return
    // after the group it's been waiting for has finished.
    എങ്കിൽ (NotifyGroup)
      QueueCondition.notify_all();
  }
}

/// Main loop for worker threads when using a jobserver.
/// This function uses a two-level queue; it first acquires a job slot from the
/// external jobserver, then retrieves a task from the internal queue.
/// This allows the thread pool to cooperate with build systems like `make -j`.
ശൂന്യം StdThreadPool::processTasksWithJobserver() {
  ആകുമ്പോൾ (സത്യം) {
    // Acquire a job slot from the external jobserver.
    // This polls for a slot and yields the thread to avoid a high-CPU wait.
    JobSlot Slot;
    // The timeout for the backoff can be very long, as the shutdown
    // is checked on each iteration. The sleep duration is capped by MaxWait
    // in ExponentialBackoff, so shutdown latency is not a problem.
    ExponentialBackoff Backoff(std::chrono::hours(24));
    തർക്കം AcquiredToken = അസത്യം;
    ചെയ്യുക {
      // Return if the thread pool is shutting down.
      {
        std::unique_lock<std::mutex> LockGuard(QueueLock);
        എങ്കിൽ (!EnableFlag)
          തിരികെ;
      }

      Slot = TheJobserver->tryAcquire();
      എങ്കിൽ (Slot.isValid()) {
        AcquiredToken = സത്യം;
        നിർത്തുക;
      }
    } ആകുമ്പോൾ (Backoff.waitForNextAttempt());

    എങ്കിൽ (!AcquiredToken) {
      // This is practically unreachable with a 24h timeout and indicates a
      // deeper problem if hit.
      report_fatal_error("Timed out waiting for jobserver token.");
    }

    // `llvm::scope_exit` guarantees the job slot is released, even if the
    // task throws or we exit early. This prevents deadlocking the build.
    llvm::scope_exit SlotReleaser(
        [&] { TheJobserver->release(std::move(Slot)); });

    // While we hold a job slot, process tasks from the internal queue.
    ആകുമ്പോൾ (സത്യം) {
      llvm::unique_function<ശൂന്യം()> Task;
      ThreadPoolTaskGroup *GroupOfTask = nullptr;

      {
        std::unique_lock<std::mutex> LockGuard(QueueLock);

        // Wait until a task is available or the pool is shutting down.
        QueueCondition.wait(LockGuard,
                            [&] { തിരികെ !EnableFlag || !Tasks.empty(); });

        // If shutting down and the queue is empty, the thread can terminate.
        എങ്കിൽ (!EnableFlag && Tasks.empty())
          തിരികെ;

        // If the queue is empty, we're done processing tasks for now.
        // Break the inner loop to release the job slot.
        എങ്കിൽ (Tasks.empty())
          നിർത്തുക;

        // A task is available. Mark it as active before releasing the lock
        // to prevent race conditions with `wait()`.
        ++ActiveThreads;
        Task = std::move(Tasks.front().first);
        GroupOfTask = Tasks.front().second;
        എങ്കിൽ (GroupOfTask != nullptr)
          ++ActiveGroups[GroupOfTask];
        Tasks.എടുക്കുക_front();
      } // The queue lock is released.

      // Run the task. The job slot remains acquired during execution.
      Task();

      // The task has finished. Update the active count and notify any waiters.
      {
        std::lock_guard<std::mutex> LockGuard(QueueLock);
        --ActiveThreads;
        എങ്കിൽ (GroupOfTask != nullptr) {
          auto A = ActiveGroups.find(GroupOfTask);
          എങ്കിൽ (--(A->second) == 0)
            ActiveGroups.erase(A);
        }
        // If all tasks are complete, notify any waiting threads.
        എങ്കിൽ (workCompletedUnlocked(nullptr))
          CompletionCondition.notify_all();
      }
    }
  }
}
തർക്കം StdThreadPool::workCompletedUnlocked(ThreadPoolTaskGroup *Group) സ്ഥിരം {
  എങ്കിൽ (Group == nullptr)
    തിരികെ !ActiveThreads && Tasks.empty();
  തിരികെ ActiveGroups.എണ്ണം(Group) == 0 &&
         !llvm::is_contained(llvm::make_second_range(Tasks), Group);
}

ശൂന്യം StdThreadPool::wait() {
  പ്രതിജ്ഞാ(!isWorkerThread()); // Would deadlock waiting for itself.
  // Wait for all threads to complete and the queue to be empty
  std::unique_lock<std::mutex> LockGuard(QueueLock);
  CompletionCondition.wait(LockGuard,
                           [&] { തിരികെ workCompletedUnlocked(nullptr); });
}

ശൂന്യം StdThreadPool::wait(ThreadPoolTaskGroup &Group) {
  // Wait for all threads in the group to complete.
  എങ്കിൽ (!isWorkerThread()) {
    std::unique_lock<std::mutex> LockGuard(QueueLock);
    CompletionCondition.wait(LockGuard,
                             [&] { തിരികെ workCompletedUnlocked(&Group); });
    തിരികെ;
  }
  // Make sure to not deadlock waiting for oneself.
  പ്രതിജ്ഞാ(CurrentThreadTaskGroups == nullptr ||
         !llvm::is_contained(*CurrentThreadTaskGroups, &Group));
  // Handle the case of recursive call from another task in a different group,
  // in which case process tasks while waiting to keep the thread busy and avoid
  // possible deadlock.
  processTasks(&Group);
}

തർക്കം StdThreadPool::isWorkerThread() സ്ഥിരം {
  llvm::sys::ScopedReader LockGuard(ThreadsLock);
  llvm::thread::id CurrentThreadId = llvm::this_thread::get_id();
  വേണ്ടി (സ്ഥിരം llvm::thread &Thread : Threads)
    എങ്കിൽ (CurrentThreadId == Thread.get_id())
      തിരികെ സത്യം;
  തിരികെ അസത്യം;
}

// The destructor joins all threads, waiting for completion.
StdThreadPool::~StdThreadPool() {
  {
    std::unique_lock<std::mutex> LockGuard(QueueLock);
    EnableFlag = അസത്യം;
  }
  QueueCondition.notify_all();
  llvm::sys::ScopedReader LockGuard(ThreadsLock);
  വേണ്ടി (auto &Worker : Threads)
    Worker.join();
}

#endif // LLVM_ENABLE_THREADS Disabled

// No threads are launched, issue a warning if ThreadCount is not 0
SingleThreadExecutor::SingleThreadExecutor(ThreadPoolStrategy S) {
  പൂർണ്ണസംഖ്യ Thread_എണ്ണം = S.compute_thread_എണ്ണം();
  എങ്കിൽ (Thread_എണ്ണം != 1) {
    errs() << "Warning: request a ThreadPool with " << Thread_എണ്ണം
           << " threads, but LLVM_ENABLE_THREADS has been turned off\n";
  }
}

ശൂന്യം SingleThreadExecutor::wait() {
  // Sequential implementation running the tasks
  ആകുമ്പോൾ (!Tasks.empty()) {
    auto Task = std::move(Tasks.front().first);
    Tasks.എടുക്കുക_front();
    Task();
  }
}

ശൂന്യം SingleThreadExecutor::wait(ThreadPoolTaskGroup &) {
  // Simply wait for all, this works even if recursive (the running task
  // is already removed from the queue).
  wait();
}

തർക്കം SingleThreadExecutor::isWorkerThread() സ്ഥിരം {
  report_fatal_error("LLVM compiled without multithreading");
}

SingleThreadExecutor::~SingleThreadExecutor() { wait(); }
