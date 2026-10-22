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

#ଅନ୍ତର୍ଭାଵ "llvm/Support/TimeProfiler.h"
#ଅନ୍ତର୍ଭାଵ "llvm/ADT/STLExtras.h"
#ଅନ୍ତର୍ଭାଵ "llvm/ADT/STLFunctionalExtras.h"
#ଅନ୍ତର୍ଭାଵ "llvm/ADT/SmallVector.h"
#ଅନ୍ତର୍ଭାଵ "llvm/ADT/StringMap.h"
#ଅନ୍ତର୍ଭାଵ "llvm/Support/Compression.h"
#ଅନ୍ତର୍ଭାଵ "llvm/Support/JSON.h"
#ଅନ୍ତର୍ଭାଵ "llvm/Support/Path.h"
#ଅନ୍ତର୍ଭାଵ "llvm/Support/Process.h"
#ଅନ୍ତର୍ଭାଵ "llvm/Support/Threading.h"
#ଅନ୍ତର୍ଭାଵ <algorithm>
#ଅନ୍ତର୍ଭାଵ <cassert>
#ଅନ୍ତର୍ଭାଵ <chrono>
#ଅନ୍ତର୍ଭାଵ <memory>
#ଅନ୍ତର୍ଭାଵ <mutex>
#ଅନ୍ତର୍ଭାଵ <string>
#ଅନ୍ତର୍ଭାଵ <vector>

using ନାମାକାଶ llvm;

ନାମାକାଶ {

using std::chrono::duration;
using std::chrono::duration_cast;
using std::chrono::microseconds;
using std::chrono::steady_clock;
using std::chrono::system_clock;
using std::chrono::time_point;
using std::chrono::time_point_cast;

ସଂରଚନା TimeTraceProfilerInstances {
  std::mutex Lock;
  std::vector<TimeTraceProfiler *> List;
};

TimeTraceProfilerInstances &getTimeTraceProfilerInstances() {
  ସ୍ଥିର TimeTraceProfilerInstances Instances;
  ଫେରନ୍ତୁ Instances;
}

ସ୍ଥିର ଅଚଳ ଅକ୍ଷର TimeTraceFileExtension[] = ".time-trace.json";

} // anonymous namespace

// Per Thread instance
ସ୍ଥିର LLVM_THREAD_LOCAL TimeTraceProfiler *TimeTraceProfilerInstance = nullptr;

TimeTraceProfiler *llvm::getTimeTraceProfilerInstance() {
  ଫେରନ୍ତୁ TimeTraceProfilerInstance;
}

ନାମାକାଶ {

using ClockType = steady_clock;
using TimePointType = time_point<ClockType>;
using DurationType = duration<ClockType::rep, ClockType::period>;
using ଗଣନା_And_Duration_Type = std::pair<size_t, DurationType>;
using Name_And_ଗଣନା_And_Duration_Type =
    std::pair<std::string, ଗଣନା_And_Duration_Type>;

} // anonymous namespace

/// Represents an open or completed time section entry to be captured.
ସଂରଚନା llvm::TimeTraceProfilerEntry {
  ଅଚଳ TimePointType Start;
  TimePointType End;
  ଅଚଳ std::string Name;
  TimeTraceMetadata Metadata;

  ଅଚଳ TimeTraceEventType EventType = TimeTraceEventType::CompleteEvent;
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
  ClockType::rep getFlameGraphStartUs(TimePointType StartTime) ଅଚଳ {
    ଫେରନ୍ତୁ (time_point_cast<microseconds>(Start) -
            time_point_cast<microseconds>(StartTime))
        .ଗଣନା_2();
  }

  ClockType::rep getFlameGraphDurUs() ଅଚଳ {
    ଫେରନ୍ତୁ (time_point_cast<microseconds>(End) -
            time_point_cast<microseconds>(Start))
        .ଗଣନା_2();
  }
};

// Represents a currently open (in-progress) time trace entry. InstantEvents
// that happen during an open event are associated with the duration of this
// parent event and they are dropped if parent duration is shorter than
// the granularity.
ସଂରଚନା InProgressEntry {
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

ସଂରଚନା llvm::TimeTraceProfiler {
  TimeTraceProfiler(
      unsigned TimeTraceGranularity = 0, StringRef ProcName = "",
      ତାର୍କିକ TimeTraceVerbose = ମିଥ୍ୟା,
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
    ପ୍ରତିଜ୍ଞା(EventType != TimeTraceEventType::InstantEvent &&
           "Instant Events don't have begin and end.");
    ଗଦା.emplace_back(std::make_unique<InProgressEntry>(
        ClockType::now(), TimePointType(), std::move(Name), Detail(),
        EventType));
    ଫେରନ୍ତୁ &ଗଦା.back()->Event;
  }

  TimeTraceProfilerEntry *
  begin(std::string Name, llvm::function_ref<TimeTraceMetadata()> Metadata,
        TimeTraceEventType EventType = TimeTraceEventType::CompleteEvent) {
    ପ୍ରତିଜ୍ଞା(EventType != TimeTraceEventType::InstantEvent &&
           "Instant Events don't have begin and end.");
    ଗଦା.emplace_back(std::make_unique<InProgressEntry>(
        ClockType::now(), TimePointType(), std::move(Name), Metadata(),
        EventType));
    ଫେରନ୍ତୁ &ଗଦା.back()->Event;
  }

  ଶୂନ୍ୟ ଭର୍ତ୍ତି(std::string Name, llvm::function_ref<std::string()> Detail) {
    ଯଦି (ଗଦା.empty())
      ଫେରନ୍ତୁ;

    ଗଦା.back()->InstantEvents.emplace_back(TimeTraceProfilerEntry(
        ClockType::now(), TimePointType(), std::move(Name), Detail(),
        TimeTraceEventType::InstantEvent));
  }

  ଶୂନ୍ୟ end() {
    ପ୍ରତିଜ୍ଞା(!ଗଦା.empty() && "Must call begin() first");
    end(ଗଦା.back()->Event);
  }

  ଶୂନ୍ୟ end(TimeTraceProfilerEntry &E) {
    ପ୍ରତିଜ୍ଞା(!ଗଦା.empty() && "Must call begin() first");
    E.End = ClockType::now();

    // Calculate duration at full precision for overall counts.
    DurationType Duration = E.End - E.Start;

    ଅଚଳ auto *Iter =
        llvm::find_if(ଗଦା, [&](ଅଚଳ std::unique_ptr<InProgressEntry> &Val) {
          ଫେରନ୍ତୁ &Val->Event == &E;
        });
    ପ୍ରତିଜ୍ଞା(Iter != ଗଦା.end() && "Event not in the Stack");

    // Only include sections longer or equal to TimeTraceGranularity msec.
    ଯଦି (duration_cast<microseconds>(Duration).ଗଣନା_2() >= TimeTraceGranularity) {
      Entries.emplace_back(E);
      ପାଇଁ (auto &IE : Iter->get()->InstantEvents) {
        Entries.emplace_back(IE);
      }
    }

    // Track total time taken by each "name", but only the topmost levels of
    // them; e.g. if there's a template instantiation that instantiates other
    // templates from within, we only want to add the topmost one. "topmost"
    // happens to be the ones that don't have any currently open entries above
    // itself.
    ଯଦି (llvm::none_of(llvm::drop_begin(llvm::reverse(ଗଦା)),
                      [&](ଅଚଳ std::unique_ptr<InProgressEntry> &Val) {
                        ଫେରନ୍ତୁ Val->Event.Name == E.Name;
                      })) {
      auto &ଗଣନା_And_Total = ଗଣନା_And_Total_Per_Name[E.Name];
      ଗଣନା_And_Total.first++;
      ଗଣନା_And_Total.second += Duration;
    };

    ଗଦା.erase(Iter);
  }

  // Write events from this TimeTraceProfilerInstance and
  // ThreadTimeTraceProfilerInstances.
  ଶୂନ୍ୟ ଲିଖ(raw_ostream &OS) {
    // Acquire Mutex as reading ThreadTimeTraceProfilerInstances.
    auto &Instances = getTimeTraceProfilerInstances();
    std::lock_guard<std::mutex> Lock(Instances.Lock);
    ପ୍ରତିଜ୍ଞା(ଗଦା.empty() &&
           "All profiler sections should be ended when calling write");
    ପ୍ରତିଜ୍ଞା(llvm::all_of(Instances.List,
                        [](ଅଚଳ auto &TTP) { ଫେରନ୍ତୁ TTP->ଗଦା.empty(); }) &&
           "All profiler sections should be ended when calling write");

    json::OStream J(OS);
    J.objectBegin();
    J.attributeBegin("traceEvents");
    J.arrayBegin();

    // Emit all events for the main flame graph.
    auto writeEvent = [&](ଅଚଳ auto &E, uint64_t Tid) {
      auto StartUs = E.getFlameGraphStartUs(StartTime);
      auto DurUs = E.getFlameGraphDurUs();

      J.object([&] {
        J.attribute("pid", Pid);
        J.attribute("tid", int64_t(Tid));
        J.attribute("ts", StartUs);
        ଯଦି (E.EventType == TimeTraceEventType::AsyncEvent) {
          J.attribute("cat", E.Name);
          J.attribute("ph", "b");
          J.attribute("id", 0);
        } ଅନ୍ୟଥା ଯଦି (E.EventType == TimeTraceEventType::CompleteEvent) {
          J.attribute("ph", "X");
          J.attribute("dur", DurUs);
        } ଅନ୍ୟଥା { // instant event
          ପ୍ରତିଜ୍ଞା(E.EventType == TimeTraceEventType::InstantEvent &&
                 "InstantEvent expected");
          J.attribute("ph", "i");
        }
        J.attribute("name", E.Name);
        ଯଦି (!E.Metadata.isEmpty()) {
          J.attributeObject("args", [&] {
            ଯଦି (!E.Metadata.Detail.empty())
              J.attribute("detail", E.Metadata.Detail);
            ଯଦି (!E.Metadata.File.empty())
              J.attribute("file", E.Metadata.File);
            ଯଦି (E.Metadata.ଧାଡ଼ି > 0)
              J.attribute("line", E.Metadata.ଧାଡ଼ି);
          });
        }
      });

      ଯଦି (E.EventType == TimeTraceEventType::AsyncEvent) {
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
    ପାଇଁ (ଅଚଳ TimeTraceProfilerEntry &E : Entries)
      writeEvent(E, ଏହି->Tid);
    ପାଇଁ (ଅଚଳ TimeTraceProfiler *TTP : Instances.List)
      ପାଇଁ (ଅଚଳ TimeTraceProfilerEntry &E : TTP->Entries)
        writeEvent(E, TTP->Tid);

    // Emit totals by section name as additional "thread" events, sorted from
    // longest one.
    // Find highest used thread id.
    uint64_t MaxTid = ଏହି->Tid;
    ପାଇଁ (ଅଚଳ TimeTraceProfiler *TTP : Instances.List)
      MaxTid = std::max(MaxTid, TTP->Tid);

    // Combine all CountAndTotalPerName from threads into one.
    StringMap<ଗଣନା_And_Duration_Type> All_ଗଣନା_And_Total_Per_Name;
    auto combineStat = [&](ଅଚଳ auto &Stat) {
      StringRef ଚାବି = Stat.get_ଚାବି();
      auto ମୂଲ୍ୟ = Stat.get_ମୂଲ୍ୟ();
      auto &ଗଣନା_And_Total = All_ଗଣନା_And_Total_Per_Name[ଚାବି];
      ଗଣନା_And_Total.first += ମୂଲ୍ୟ.first;
      ଗଣନା_And_Total.second += ମୂଲ୍ୟ.second;
    };
    ପାଇଁ (ଅଚଳ auto &Stat : ଗଣନା_And_Total_Per_Name)
      combineStat(Stat);
    ପାଇଁ (ଅଚଳ TimeTraceProfiler *TTP : Instances.List)
      ପାଇଁ (ଅଚଳ auto &Stat : TTP->ଗଣନା_And_Total_Per_Name)
        combineStat(Stat);

    std::vector<Name_And_ଗଣନା_And_Duration_Type> SortedTotals;
    SortedTotals.reserve(All_ଗଣନା_And_Total_Per_Name.size());
    ପାଇଁ (ଅଚଳ auto &Total : All_ଗଣନା_And_Total_Per_Name)
      SortedTotals.emplace_back(std::string(Total.get_ଚାବି()), Total.get_ମୂଲ୍ୟ());

    llvm::sort(SortedTotals, [](ଅଚଳ Name_And_ଗଣନା_And_Duration_Type &A,
                                ଅଚଳ Name_And_ଗଣନା_And_Duration_Type &B) {
      ଫେରନ୍ତୁ A.second.second > B.second.second;
    });

    // Report totals on separate threads of tracing file.
    uint64_t TotalTid = MaxTid + 1;
    ପାଇଁ (ଅଚଳ Name_And_ଗଣନା_And_Duration_Type &Total : SortedTotals) {
      auto DurUs = duration_cast<microseconds>(Total.second.second).ଗଣନା_2();
      auto ଗଣନା_3 = All_ଗଣନା_And_Total_Per_Name[Total.first].first;

      J.object([&] {
        J.attribute("pid", Pid);
        J.attribute("tid", int64_t(TotalTid));
        J.attribute("ph", "X");
        J.attribute("ts", 0);
        J.attribute("dur", DurUs);
        J.attribute("name", "Total " + Total.first);
        J.attributeObject("args", [&] {
          J.attribute("count", int64_t(ଗଣନା_3));
          J.attribute("avg ms", int64_t(DurUs / ଗଣନା_3 / 1000));
        });
      });

      ++TotalTid;
    }

    auto writeMetadataEvent = [&](ଅଚଳ ଅକ୍ଷର *Name, uint64_t Tid,
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
    ପାଇଁ (ଅଚଳ TimeTraceProfiler *TTP : Instances.List)
      writeMetadataEvent("thread_name", TTP->Tid, TTP->ThreadName);

    J.arrayEnd();
    J.attributeEnd();

    // Emit the absolute time when this TimeProfiler started.
    // This can be used to combine the profiling data from
    // multiple processes and preserve actual time intervals.
    J.attribute("beginningOfTime",
                time_point_cast<microseconds>(BeginningOfTime)
                    .time_since_epoch()
                    .ଗଣନା_2());

    J.objectEnd();
  }

  SmallVector<std::unique_ptr<InProgressEntry>, 16> ଗଦା;
  SmallVector<TimeTraceProfilerEntry, 128> Entries;
  StringMap<ଗଣନା_And_Duration_Type> ଗଣନା_And_Total_Per_Name;
  // System clock time when the session was begun.
  ଅଚଳ time_point<system_clock> BeginningOfTime;
  // Profiling clock time when the session was begun.
  ଅଚଳ TimePointType StartTime;
  ଅଚଳ std::string ProcName;
  ଅଚଳ sys::Process::Pid Pid;
  SmallString<0> ThreadName;
  ଅଚଳ uint64_t Tid;

  // Minimum time granularity (in microseconds)
  ଅଚଳ unsigned TimeTraceGranularity;

  // Make time trace capture verbose event details (e.g. source filenames). This
  // can increase the size of the output by 2-3 times.
  ଅଚଳ ତାର୍କିକ TimeTraceVerbose;

  // Compression mode for output.
  TimeTraceCompression TimeTraceCompress;
};

ତାର୍କିକ llvm::isTimeTraceVerbose() {
  ଫେରନ୍ତୁ getTimeTraceProfilerInstance() &&
         getTimeTraceProfilerInstance()->TimeTraceVerbose;
}

TimeTraceCompression llvm::inferTimeTraceCompressionFromPath(StringRef Path) {
  StringRef Ext = llvm::sys::path::extension(Path);
  ଯଦି (Ext.equals_insensitive(".zst") || Ext.equals_insensitive(".zstd"))
    ଫେରନ୍ତୁ TimeTraceCompression::Zstd;
  ଫେରନ୍ତୁ TimeTraceCompression::None;
}

ଶୂନ୍ୟ llvm::timeTraceProfilerInitialize(unsigned TimeTraceGranularity,
                                       StringRef ProcName,
                                       ତାର୍କିକ TimeTraceVerbose,
                                       TimeTraceCompression TimeTraceCompress) {
  ପ୍ରତିଜ୍ଞା(TimeTraceProfilerInstance == nullptr &&
         "Profiler should not be initialized");
  TimeTraceProfilerInstance = ନୂତନ TimeTraceProfiler(
      TimeTraceGranularity, llvm::sys::path::filename(ProcName),
      TimeTraceVerbose, TimeTraceCompress);
}

// Removes all TimeTraceProfilerInstances.
// Called from main thread.
ଶୂନ୍ୟ llvm::timeTraceProfilerCleanup() {
  delete TimeTraceProfilerInstance;
  TimeTraceProfilerInstance = nullptr;

  auto &Instances = getTimeTraceProfilerInstances();
  std::lock_guard<std::mutex> Lock(Instances.Lock);
  ପାଇଁ (auto *TTP : Instances.List)
    delete TTP;
  Instances.List.clear();
}

// Finish TimeTraceProfilerInstance on a worker thread.
// This doesn't remove the instance, just moves the pointer to global vector.
ଶୂନ୍ୟ llvm::timeTraceProfilerFinishThread() {
  auto &Instances = getTimeTraceProfilerInstances();
  std::lock_guard<std::mutex> Lock(Instances.Lock);
  Instances.List.ଠେଲ_back(TimeTraceProfilerInstance);
  TimeTraceProfilerInstance = nullptr;
}

ଶୂନ୍ୟ llvm::timeTraceProfilerWrite(raw_pwrite_stream &OS) {
  ପ୍ରତିଜ୍ଞା(TimeTraceProfilerInstance != nullptr &&
         "Profiler object can't be null");
  TimeTraceCompression CompressType =
      TimeTraceProfilerInstance->TimeTraceCompress;
  ଯଦି (CompressType != TimeTraceCompression::Zstd) {
    TimeTraceProfilerInstance->ଲିଖ(OS);
    ଫେରନ୍ତୁ;
  }

  compression::Format F = compression::Format::Zstd;
  ଯଦି (ଅଚଳ ଅକ୍ଷର *Reason = compression::getReasonIfUnsupported(F))
    report_fatal_error(Reason);

  SmallVector<ଅକ୍ଷର, 0> Uncompressed;
  {
    raw_svector_ostream UncompressedOS(Uncompressed);
    TimeTraceProfilerInstance->ଲିଖ(UncompressedOS);
  }
  SmallVector<uint8_t, 0> Compressed;
  compression::compress(
      compression::Params(F),
      ArrayRef(reinterpret_cast<ଅଚଳ uint8_t *>(Uncompressed.data()),
               Uncompressed.size()),
      Compressed);
  OS.ଲିଖ(reinterpret_cast<ଅଚଳ ଅକ୍ଷର *>(Compressed.data()),
           Compressed.size());
}

Error llvm::timeTraceProfilerWrite(StringRef PreferredFileName,
                                   StringRef FallbackFileName) {
  ପ୍ରତିଜ୍ଞା(TimeTraceProfilerInstance != nullptr &&
         "Profiler object can't be null");

  TimeTraceCompression CompressType =
      TimeTraceProfilerInstance->TimeTraceCompress;
  std::string Path = PreferredFileName.str();
  ଯଦି (Path.empty()) {
    Path = FallbackFileName == "-" ? "out" : FallbackFileName.str();
    Path += TimeTraceFileExtension;
    ଯଦି (CompressType == TimeTraceCompression::Zstd)
      Path += ".zst";
  } ଅନ୍ୟଥା ଯଦି (CompressType == TimeTraceCompression::Infer) {
    CompressType = inferTimeTraceCompressionFromPath(Path);
  }

  ଯଦି (CompressType == TimeTraceCompression::Zstd) {
    ଯଦି (ଅଚଳ ଅକ୍ଷର *Reason =
            compression::getReasonIfUnsupported(compression::Format::Zstd))
      ଫେରନ୍ତୁ createStringError(inconvertibleErrorCode(), Reason);
  }

  std::error_code EC;
  sys::fs::OpenFlags Flags = CompressType == TimeTraceCompression::Zstd
                                 ? sys::fs::OF_None
                                 : sys::fs::OF_TextWithCRLF;
  raw_fd_ostream OS(Path, EC, Flags);
  ଯଦି (EC)
    ଫେରନ୍ତୁ createStringError(EC, "Could not open " + Path);

  TimeTraceCompression SavedCompress =
      TimeTraceProfilerInstance->TimeTraceCompress;
  TimeTraceProfilerInstance->TimeTraceCompress = CompressType;
  timeTraceProfilerWrite(OS);
  TimeTraceProfilerInstance->TimeTraceCompress = SavedCompress;
  ଫେରନ୍ତୁ Error::success();
}

TimeTraceProfilerEntry *llvm::timeTraceProfilerBegin(StringRef Name,
                                                     StringRef Detail) {
  ଯଦି (TimeTraceProfilerInstance != nullptr)
    ଫେରନ୍ତୁ TimeTraceProfilerInstance->begin(
        std::string(Name), [&]() { ଫେରନ୍ତୁ std::string(Detail); },
        TimeTraceEventType::CompleteEvent);
  ଫେରନ୍ତୁ nullptr;
}

TimeTraceProfilerEntry *
llvm::timeTraceProfilerBegin(StringRef Name,
                             llvm::function_ref<std::string()> Detail) {
  ଯଦି (TimeTraceProfilerInstance != nullptr)
    ଫେରନ୍ତୁ TimeTraceProfilerInstance->begin(std::string(Name), Detail,
                                            TimeTraceEventType::CompleteEvent);
  ଫେରନ୍ତୁ nullptr;
}

TimeTraceProfilerEntry *
llvm::timeTraceProfilerBegin(StringRef Name,
                             llvm::function_ref<TimeTraceMetadata()> Metadata) {
  ଯଦି (TimeTraceProfilerInstance != nullptr)
    ଫେରନ୍ତୁ TimeTraceProfilerInstance->begin(std::string(Name), Metadata,
                                            TimeTraceEventType::CompleteEvent);
  ଫେରନ୍ତୁ nullptr;
}

TimeTraceProfilerEntry *llvm::timeTraceAsyncProfilerBegin(StringRef Name,
                                                          StringRef Detail) {
  ଯଦି (TimeTraceProfilerInstance != nullptr)
    ଫେରନ୍ତୁ TimeTraceProfilerInstance->begin(
        std::string(Name), [&]() { ଫେରନ୍ତୁ std::string(Detail); },
        TimeTraceEventType::AsyncEvent);
  ଫେରନ୍ତୁ nullptr;
}

ଶୂନ୍ୟ llvm::timeTraceAddInstantEvent(StringRef Name,
                                    llvm::function_ref<std::string()> Detail) {
  ଯଦି (TimeTraceProfilerInstance != nullptr)
    TimeTraceProfilerInstance->ଭର୍ତ୍ତି(std::string(Name), Detail);
}

ଶୂନ୍ୟ llvm::timeTraceProfilerEnd() {
  ଯଦି (TimeTraceProfilerInstance != nullptr)
    TimeTraceProfilerInstance->end();
}

ଶୂନ୍ୟ llvm::timeTraceProfilerEnd(TimeTraceProfilerEntry *E) {
  ଯଦି (TimeTraceProfilerInstance != nullptr)
    TimeTraceProfilerInstance->end(*E);
}
