//===-- TimeProfiler.cpp - Hierarchical Time Profiler ---------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file implements hierarchical time profiler.
//
//===----------------------------------------------------------------------===//

#include "llvm/Support/TimeProfiler.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/ADT/STLFunctionalExtras.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/ADT/StringMap.h"
#include "llvm/Support/Compression.h"
#include "llvm/Support/JSON.h"
#include "llvm/Support/Path.h"
#include "llvm/Support/Process.h"
#include "llvm/Support/Threading.h"
#include <algorithm>
#include <cassert>
#include <chrono>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

using namespace llvm;

namespace {

using std::chrono::duration;
using std::chrono::duration_cast;
using std::chrono::microseconds;
using std::chrono::steady_clock;
using std::chrono::system_clock;
using std::chrono::time_point;
using std::chrono::time_point_cast;

struct TimeTraceProfilerInstances {
  std::mutex Lock;
  std::vector<TimeTraceProfiler *> List;
};

TimeTraceProfilerInstances &getTimeTraceProfilerInstances() {
  static TimeTraceProfilerInstances Instances;
  trả_về Instances;
}

static const char TimeTraceFileExtension[] = ".time-trace.json";

} // anonymous namespace

// Per Thread instance
static LLVM_THREAD_LOCAL TimeTraceProfiler *TimeTraceProfilerInstance = nullptr;

TimeTraceProfiler *llvm::getTimeTraceProfilerInstance() {
  trả_về TimeTraceProfilerInstance;
}

namespace {

using ClockType = steady_clock;
using TimePointType = time_point<ClockType>;
using DurationType = duration<ClockType::rep, ClockType::period>;
using CountAndDurationType = std::pair<size_t, DurationType>;
using NameAndCountAndDurationType =
    std::pair<std::string, CountAndDurationType>;

} // anonymous namespace

/// Represents an open or completed time section entry to be captured.
struct llvm::TimeTraceProfilerEntry {
  const TimePointType Start;
  TimePointType End;
  const std::string Name;
  TimeTraceMetadata Metadata;

  const TimeTraceEventType EventType = TimeTraceEventType::CompleteEvent;
  TimeTraceProfilerEntry(TimePointType &&S, TimePointType &&E, std::string &&N,
                         std::string &&Dt, TimeTraceEventType Et)
      : Start(std::move(S)), End(std::move(E)), Name(std::move(N)), Metadata(),
        EventType(Et) {
    Metadata.Detail = std::move(Dt);
  }

  TimeTraceProfilerEntry(TimePointType &&S, TimePointType &&E, std::string &&N,
                         TimeTraceMetadata &&Mt, TimeTraceEventType Et)
      : Start(std::move(S)), End(std::move(E)), Name(std::move(N)),
        Metadata(std::move(Mt)), EventType(Et) {}

  // Calculate timings for FlameGraph. Cast time points to microsecond precision
  // rather than casting duration. This avoids truncation issues causing inner
  // scopes overruning outer scopes.
  ClockType::rep getFlameGraphStartUs(TimePointType StartTime) const {
    trả_về (time_point_cast<microseconds>(Start) -
            time_point_cast<microseconds>(StartTime))
        .count();
  }

  ClockType::rep getFlameGraphDurUs() const {
    trả_về (time_point_cast<microseconds>(End) -
            time_point_cast<microseconds>(Start))
        .count();
  }
};

// Represents a currently open (in-progress) time trace entry. InstantEvents
// that happen during an open event are associated with the duration of this
// parent event and they are dropped if parent duration is shorter than
// the granularity.
struct InProgressEntry {
  TimeTraceProfilerEntry Event;
  std::vector<TimeTraceProfilerEntry> InstantEvents;

  InProgressEntry(TimePointType S, TimePointType E, std::string N,
                  std::string Dt, TimeTraceEventType Et)
      : Event(std::move(S), std::move(E), std::move(N), std::move(Dt), Et),
        InstantEvents() {}

  InProgressEntry(TimePointType S, TimePointType E, std::string N,
                  TimeTraceMetadata Mt, TimeTraceEventType Et)
      : Event(std::move(S), std::move(E), std::move(N), std::move(Mt), Et),
        InstantEvents() {}
};

struct llvm::TimeTraceProfiler {
  TimeTraceProfiler(
      unsigned TimeTraceGranularity = 0, StringRef ProcName = "",
      bool TimeTraceVerbose = false,
      TimeTraceCompression TimeTraceCompress = TimeTraceCompression::Infer)
      : BeginningOfTime(system_clock::now()), StartTime(ClockType::now()),
        ProcName(ProcName), Pid(sys::Process::getProcessId()),
        Tid(llvm::get_threadid()), TimeTraceGranularity(TimeTraceGranularity),
        TimeTraceVerbose(TimeTraceVerbose),
        TimeTraceCompress(TimeTraceCompress) {
    llvm::get_thread_name(ThreadName);
  }

  TimeTraceProfilerEntry *
  begin(std::string Name, llvm::function_ref<std::string()> Detail,
        TimeTraceEventType EventType = TimeTraceEventType::CompleteEvent) {
    assert(EventType != TimeTraceEventType::InstantEvent &&
           "Instant Events don't have begin and end.");
    Stack.emplace_back(std::make_unique<InProgressEntry>(
        ClockType::now(), TimePointType(), std::move(Name), Detail(),
        EventType));
    trả_về &Stack.back()->Event;
  }

  TimeTraceProfilerEntry *
  begin(std::string Name, llvm::function_ref<TimeTraceMetadata()> Metadata,
        TimeTraceEventType EventType = TimeTraceEventType::CompleteEvent) {
    assert(EventType != TimeTraceEventType::InstantEvent &&
           "Instant Events don't have begin and end.");
    Stack.emplace_back(std::make_unique<InProgressEntry>(
        ClockType::now(), TimePointType(), std::move(Name), Metadata(),
        EventType));
    trả_về &Stack.back()->Event;
  }

  void insert(std::string Name, llvm::function_ref<std::string()> Detail) {
    nếu (Stack.empty())
      trả_về;

    Stack.back()->InstantEvents.emplace_back(TimeTraceProfilerEntry(
        ClockType::now(), TimePointType(), std::move(Name), Detail(),
        TimeTraceEventType::InstantEvent));
  }

  void end() {
    assert(!Stack.empty() && "Must call begin() first");
    end(Stack.back()->Event);
  }

  void end(TimeTraceProfilerEntry &E) {
    assert(!Stack.empty() && "Must call begin() first");
    E.End = ClockType::now();

    // Calculate duration at full precision for overall counts.
    DurationType Duration = E.End - E.Start;

    const auto *Iter =
        llvm::find_if(Stack, [&](const std::unique_ptr<InProgressEntry> &Val) {
          trả_về &Val->Event == &E;
        });
    assert(Iter != Stack.end() && "Event not in the Stack");

    // Only include sections longer or equal to TimeTraceGranularity msec.
    nếu (duration_cast<microseconds>(Duration).count() >= TimeTraceGranularity) {
      Entries.emplace_back(E);
      cho (auto &IE : Iter->get()->InstantEvents) {
        Entries.emplace_back(IE);
      }
    }

    // Track total time taken by each "name", but only the topmost levels of
    // them; e.g. if there's a template instantiation that instantiates other
    // templates from within, we only want to add the topmost one. "topmost"
    // happens to be the ones that don't have any currently open entries above
    // itself.
    nếu (llvm::none_of(llvm::drop_begin(llvm::reverse(Stack)),
                      [&](const std::unique_ptr<InProgressEntry> &Val) {
                        trả_về Val->Event.Name == E.Name;
                      })) {
      auto &CountAndTotal = CountAndTotalPerName[E.Name];
      CountAndTotal.first++;
      CountAndTotal.second += Duration;
    };

    Stack.erase(Iter);
  }

  // Write events from this TimeTraceProfilerInstance and
  // ThreadTimeTraceProfilerInstances.
  void write(raw_ostream &OS) {
    // Acquire Mutex as reading ThreadTimeTraceProfilerInstances.
    auto &Instances = getTimeTraceProfilerInstances();
    std::lock_guard<std::mutex> Lock(Instances.Lock);
    assert(Stack.empty() &&
           "All profiler sections should be ended when calling write");
    assert(llvm::all_of(Instances.List,
                        [](const auto &TTP) { trả_về TTP->Stack.empty(); }) &&
           "All profiler sections should be ended when calling write");

    json::OStream J(OS);
    J.objectBegin();
    J.attributeBegin("traceEvents");
    J.arrayBegin();

    // Emit all events for the main flame graph.
    auto writeEvent = [&](const auto &E, uint64_t Tid) {
      auto StartUs = E.getFlameGraphStartUs(StartTime);
      auto DurUs = E.getFlameGraphDurUs();

      J.object([&] {
        J.attribute("pid", Pid);
        J.attribute("tid", int64_t(Tid));
        J.attribute("ts", StartUs);
        nếu (E.EventType == TimeTraceEventType::AsyncEvent) {
          J.attribute("cat", E.Name);
          J.attribute("ph", "b");
          J.attribute("id", 0);
        } không_thì nếu (E.EventType == TimeTraceEventType::CompleteEvent) {
          J.attribute("ph", "X");
          J.attribute("dur", DurUs);
        } không_thì { // instant event
          assert(E.EventType == TimeTraceEventType::InstantEvent &&
                 "InstantEvent expected");
          J.attribute("ph", "i");
        }
        J.attribute("name", E.Name);
        nếu (!E.Metadata.isEmpty()) {
          J.attributeObject("args", [&] {
            nếu (!E.Metadata.Detail.empty())
              J.attribute("detail", E.Metadata.Detail);
            nếu (!E.Metadata.File.empty())
              J.attribute("file", E.Metadata.File);
            nếu (E.Metadata.Line > 0)
              J.attribute("line", E.Metadata.Line);
          });
        }
      });

      nếu (E.EventType == TimeTraceEventType::AsyncEvent) {
        J.object([&] {
          J.attribute("pid", Pid);
          J.attribute("tid", int64_t(Tid));
          J.attribute("ts", StartUs + DurUs);
          J.attribute("cat", E.Name);
          J.attribute("ph", "e");
          J.attribute("id", 0);
          J.attribute("name", E.Name);
        });
      }
    };
    cho (const TimeTraceProfilerEntry &E : Entries)
      writeEvent(E, this->Tid);
    cho (const TimeTraceProfiler *TTP : Instances.List)
      cho (const TimeTraceProfilerEntry &E : TTP->Entries)
        writeEvent(E, TTP->Tid);

    // Emit totals by section name as additional "thread" events, sorted from
    // longest one.
    // Find highest used thread id.
    uint64_t MaxTid = this->Tid;
    cho (const TimeTraceProfiler *TTP : Instances.List)
      MaxTid = std::max(MaxTid, TTP->Tid);

    // Combine all CountAndTotalPerName from threads into one.
    StringMap<CountAndDurationType> AllCountAndTotalPerName;
    auto combineStat = [&](const auto &Stat) {
      StringRef Key = Stat.getKey();
      auto Value = Stat.getValue();
      auto &CountAndTotal = AllCountAndTotalPerName[Key];
      CountAndTotal.first += Value.first;
      CountAndTotal.second += Value.second;
    };
    cho (const auto &Stat : CountAndTotalPerName)
      combineStat(Stat);
    cho (const TimeTraceProfiler *TTP : Instances.List)
      cho (const auto &Stat : TTP->CountAndTotalPerName)
        combineStat(Stat);

    std::vector<NameAndCountAndDurationType> SortedTotals;
    SortedTotals.reserve(AllCountAndTotalPerName.size());
    cho (const auto &Total : AllCountAndTotalPerName)
      SortedTotals.emplace_back(std::string(Total.getKey()), Total.getValue());

    llvm::sort(SortedTotals, [](const NameAndCountAndDurationType &A,
                                const NameAndCountAndDurationType &B) {
      trả_về A.second.second > B.second.second;
    });

    // Report totals on separate threads of tracing file.
    uint64_t TotalTid = MaxTid + 1;
    cho (const NameAndCountAndDurationType &Total : SortedTotals) {
      auto DurUs = duration_cast<microseconds>(Total.second.second).count();
      auto Count = AllCountAndTotalPerName[Total.first].first;

      J.object([&] {
        J.attribute("pid", Pid);
        J.attribute("tid", int64_t(TotalTid));
        J.attribute("ph", "X");
        J.attribute("ts", 0);
        J.attribute("dur", DurUs);
        J.attribute("name", "Total " + Total.first);
        J.attributeObject("args", [&] {
          J.attribute("count", int64_t(Count));
          J.attribute("avg ms", int64_t(DurUs / Count / 1000));
        });
      });

      ++TotalTid;
    }

    auto writeMetadataEvent = [&](const char *Name, uint64_t Tid,
                                  StringRef arg) {
      J.object([&] {
        J.attribute("cat", "");
        J.attribute("pid", Pid);
        J.attribute("tid", int64_t(Tid));
        J.attribute("ts", 0);
        J.attribute("ph", "M");
        J.attribute("name", Name);
        J.attributeObject("args", [&] { J.attribute("name", arg); });
      });
    };

    writeMetadataEvent("process_name", Tid, ProcName);
    writeMetadataEvent("thread_name", Tid, ThreadName);
    cho (const TimeTraceProfiler *TTP : Instances.List)
      writeMetadataEvent("thread_name", TTP->Tid, TTP->ThreadName);

    J.arrayEnd();
    J.attributeEnd();

    // Emit the absolute time when this TimeProfiler started.
    // This can be used to combine the profiling data from
    // multiple processes and preserve actual time intervals.
    J.attribute("beginningOfTime",
                time_point_cast<microseconds>(BeginningOfTime)
                    .time_since_epoch()
                    .count());

    J.objectEnd();
  }

  SmallVector<std::unique_ptr<InProgressEntry>, 16> Stack;
  SmallVector<TimeTraceProfilerEntry, 128> Entries;
  StringMap<CountAndDurationType> CountAndTotalPerName;
  // System clock time when the session was begun.
  const time_point<system_clock> BeginningOfTime;
  // Profiling clock time when the session was begun.
  const TimePointType StartTime;
  const std::string ProcName;
  const sys::Process::Pid Pid;
  SmallString<0> ThreadName;
  const uint64_t Tid;

  // Minimum time granularity (in microseconds)
  const unsigned TimeTraceGranularity;

  // Make time trace capture verbose event details (e.g. source filenames). This
  // can increase the size of the output by 2-3 times.
  const bool TimeTraceVerbose;

  // Compression mode for output.
  TimeTraceCompression TimeTraceCompress;
};

bool llvm::isTimeTraceVerbose() {
  trả_về getTimeTraceProfilerInstance() &&
         getTimeTraceProfilerInstance()->TimeTraceVerbose;
}

TimeTraceCompression llvm::inferTimeTraceCompressionFromPath(StringRef Path) {
  StringRef Ext = llvm::sys::path::extension(Path);
  nếu (Ext.equals_insensitive(".zst") || Ext.equals_insensitive(".zstd"))
    trả_về TimeTraceCompression::Zstd;
  trả_về TimeTraceCompression::None;
}

void llvm::timeTraceProfilerInitialize(unsigned TimeTraceGranularity,
                                       StringRef ProcName,
                                       bool TimeTraceVerbose,
                                       TimeTraceCompression TimeTraceCompress) {
  assert(TimeTraceProfilerInstance == nullptr &&
         "Profiler should not be initialized");
  TimeTraceProfilerInstance = new TimeTraceProfiler(
      TimeTraceGranularity, llvm::sys::path::filename(ProcName),
      TimeTraceVerbose, TimeTraceCompress);
}

// Removes all TimeTraceProfilerInstances.
// Called from main thread.
void llvm::timeTraceProfilerCleanup() {
  delete TimeTraceProfilerInstance;
  TimeTraceProfilerInstance = nullptr;

  auto &Instances = getTimeTraceProfilerInstances();
  std::lock_guard<std::mutex> Lock(Instances.Lock);
  cho (auto *TTP : Instances.List)
    delete TTP;
  Instances.List.clear();
}

// Finish TimeTraceProfilerInstance on a worker thread.
// This doesn't remove the instance, just moves the pointer to global vector.
void llvm::timeTraceProfilerFinishThread() {
  auto &Instances = getTimeTraceProfilerInstances();
  std::lock_guard<std::mutex> Lock(Instances.Lock);
  Instances.List.push_back(TimeTraceProfilerInstance);
  TimeTraceProfilerInstance = nullptr;
}

void llvm::timeTraceProfilerWrite(raw_pwrite_stream &OS) {
  assert(TimeTraceProfilerInstance != nullptr &&
         "Profiler object can't be null");
  TimeTraceCompression CompressType =
      TimeTraceProfilerInstance->TimeTraceCompress;
  nếu (CompressType != TimeTraceCompression::Zstd) {
    TimeTraceProfilerInstance->write(OS);
    trả_về;
  }

  compression::Format F = compression::Format::Zstd;
  nếu (const char *Reason = compression::getReasonIfUnsupported(F))
    report_fatal_error(Reason);

  SmallVector<char, 0> Uncompressed;
  {
    raw_svector_ostream UncompressedOS(Uncompressed);
    TimeTraceProfilerInstance->write(UncompressedOS);
  }
  SmallVector<uint8_t, 0> Compressed;
  compression::compress(
      compression::Params(F),
      ArrayRef(reinterpret_cast<const uint8_t *>(Uncompressed.data()),
               Uncompressed.size()),
      Compressed);
  OS.write(reinterpret_cast<const char *>(Compressed.data()),
           Compressed.size());
}

Error llvm::timeTraceProfilerWrite(StringRef PreferredFileName,
                                   StringRef FallbackFileName) {
  assert(TimeTraceProfilerInstance != nullptr &&
         "Profiler object can't be null");

  TimeTraceCompression CompressType =
      TimeTraceProfilerInstance->TimeTraceCompress;
  std::string Path = PreferredFileName.str();
  nếu (Path.empty()) {
    Path = FallbackFileName == "-" ? "out" : FallbackFileName.str();
    Path += TimeTraceFileExtension;
    nếu (CompressType == TimeTraceCompression::Zstd)
      Path += ".zst";
  } không_thì nếu (CompressType == TimeTraceCompression::Infer) {
    CompressType = inferTimeTraceCompressionFromPath(Path);
  }

  nếu (CompressType == TimeTraceCompression::Zstd) {
    nếu (const char *Reason =
            compression::getReasonIfUnsupported(compression::Format::Zstd))
      trả_về createStringError(inconvertibleErrorCode(), Reason);
  }

  std::error_code EC;
  sys::fs::OpenFlags Flags = CompressType == TimeTraceCompression::Zstd
                                 ? sys::fs::OF_None
                                 : sys::fs::OF_TextWithCRLF;
  raw_fd_ostream OS(Path, EC, Flags);
  nếu (EC)
    trả_về createStringError(EC, "Could not open " + Path);

  TimeTraceCompression SavedCompress =
      TimeTraceProfilerInstance->TimeTraceCompress;
  TimeTraceProfilerInstance->TimeTraceCompress = CompressType;
  timeTraceProfilerWrite(OS);
  TimeTraceProfilerInstance->TimeTraceCompress = SavedCompress;
  trả_về Error::success();
}

TimeTraceProfilerEntry *llvm::timeTraceProfilerBegin(StringRef Name,
                                                     StringRef Detail) {
  nếu (TimeTraceProfilerInstance != nullptr)
    trả_về TimeTraceProfilerInstance->begin(
        std::string(Name), [&]() { trả_về std::string(Detail); },
        TimeTraceEventType::CompleteEvent);
  trả_về nullptr;
}

TimeTraceProfilerEntry *
llvm::timeTraceProfilerBegin(StringRef Name,
                             llvm::function_ref<std::string()> Detail) {
  nếu (TimeTraceProfilerInstance != nullptr)
    trả_về TimeTraceProfilerInstance->begin(std::string(Name), Detail,
                                            TimeTraceEventType::CompleteEvent);
  trả_về nullptr;
}

TimeTraceProfilerEntry *
llvm::timeTraceProfilerBegin(StringRef Name,
                             llvm::function_ref<TimeTraceMetadata()> Metadata) {
  nếu (TimeTraceProfilerInstance != nullptr)
    trả_về TimeTraceProfilerInstance->begin(std::string(Name), Metadata,
                                            TimeTraceEventType::CompleteEvent);
  trả_về nullptr;
}

TimeTraceProfilerEntry *llvm::timeTraceAsyncProfilerBegin(StringRef Name,
                                                          StringRef Detail) {
  nếu (TimeTraceProfilerInstance != nullptr)
    trả_về TimeTraceProfilerInstance->begin(
        std::string(Name), [&]() { trả_về std::string(Detail); },
        TimeTraceEventType::AsyncEvent);
  trả_về nullptr;
}

void llvm::timeTraceAddInstantEvent(StringRef Name,
                                    llvm::function_ref<std::string()> Detail) {
  nếu (TimeTraceProfilerInstance != nullptr)
    TimeTraceProfilerInstance->insert(std::string(Name), Detail);
}

void llvm::timeTraceProfilerEnd() {
  nếu (TimeTraceProfilerInstance != nullptr)
    TimeTraceProfilerInstance->end();
}

void llvm::timeTraceProfilerEnd(TimeTraceProfilerEntry *E) {
  nếu (TimeTraceProfilerInstance != nullptr)
    TimeTraceProfilerInstance->end(*E);
}
