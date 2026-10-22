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

#अन्तर्भाव "llvm/Support/TimeProfiler.h"
#अन्तर्भाव "llvm/ADT/STLExtras.h"
#अन्तर्भाव "llvm/ADT/STLFunctionalExtras.h"
#अन्तर्भाव "llvm/ADT/SmallVector.h"
#अन्तर्भाव "llvm/ADT/StringMap.h"
#अन्तर्भाव "llvm/Support/Compression.h"
#अन्तर्भाव "llvm/Support/JSON.h"
#अन्तर्भाव "llvm/Support/Path.h"
#अन्तर्भाव "llvm/Support/Process.h"
#अन्तर्भाव "llvm/Support/Threading.h"
#अन्तर्भाव <algorithm>
#अन्तर्भाव <cassert>
#अन्तर्भाव <chrono>
#अन्तर्भाव <memory>
#अन्तर्भाव <mutex>
#अन्तर्भाव <string>
#अन्तर्भाव <vector>

उपयुज्य नामाकाश llvm;

नामाकाश {

उपयुज्य std::chrono::duration;
उपयुज्य std::chrono::duration_cast;
उपयुज्य std::chrono::microseconds;
उपयुज्य std::chrono::steady_clock;
उपयुज्य std::chrono::system_clock;
उपयुज्य std::chrono::time_point;
उपयुज्य std::chrono::time_point_cast;

संरचना TimeTraceProfilerInstances {
  std::mutex तालकम्_2;
  std::सदिशम्<TimeTraceProfiler *> List;
};

TimeTraceProfilerInstances &getTimeTraceProfilerInstances() {
  स्थिर TimeTraceProfilerInstances Instances;
  प्रत्यागम Instances;
}

स्थिर ध्रुव वर्ण TimeTraceFileExtension[] = ".time-trace.json";

} // anonymous namespace

// Per Thread instance
स्थिर LLVM_THREAD_LOCAL TimeTraceProfiler *TimeTraceProfilerInstance = शून्यसूचकः;

TimeTraceProfiler *llvm::getTimeTraceProfilerInstance() {
  प्रत्यागम TimeTraceProfilerInstance;
}

नामाकाश {

उपयुज्य ClockType = steady_clock;
उपयुज्य TimePointType = time_point<ClockType>;
उपयुज्य DurationType = duration<ClockType::rep, ClockType::period>;
उपयुज्य गणनम्_And_Duration_Type = std::pair<size_t, DurationType>;
उपयुज्य Name_And_गणनम्_And_Duration_Type =
    std::pair<std::string, गणनम्_And_Duration_Type>;

} // anonymous namespace

/// Represents an open or completed time section entry to be captured.
संरचना llvm::Time_Trace_Profiler_प्रविष्टिः {
  ध्रुव TimePointType Start;
  TimePointType अन्तः_2;
  ध्रुव std::string Name;
  TimeTraceMetadata Metadata;

  ध्रुव TimeTraceEventType EventType = TimeTraceEventType::CompleteEvent;
  Time_Trace_Profiler_प्रविष्टिः(TimePointType &&S, TimePointType &&E, std::string &&N,
                         std::string &&Dt, TimeTraceEventType Et)
      : Start(std::move(S)), अन्तः_2(std::move(E)), Name(std::move(N)), Metadata(),
        EventType(Et) {
    Metadata.Detail = std::move(Dt);
  }

  Time_Trace_Profiler_प्रविष्टिः(TimePointType &&S, TimePointType &&E, std::string &&N,
                         TimeTraceMetadata &&Mt, TimeTraceEventType Et)
      : Start(std::move(S)), अन्तः_2(std::move(E)), Name(std::move(N)),
        Metadata(std::move(Mt)), EventType(Et) {}

  // Calculate timings for FlameGraph. Cast time points to microsecond precision
  // rather than casting duration. This avoids truncation issues causing inner
  // scopes overruning outer scopes.
  ClockType::rep getFlameGraphStartUs(TimePointType StartTime) ध्रुव {
    प्रत्यागम (time_point_cast<microseconds>(Start) -
            time_point_cast<microseconds>(StartTime))
        .गणनम्();
  }

  ClockType::rep getFlameGraphDurUs() ध्रुव {
    प्रत्यागम (time_point_cast<microseconds>(अन्तः_2) -
            time_point_cast<microseconds>(Start))
        .गणनम्();
  }
};

// Represents a currently open (in-progress) time trace entry. InstantEvents
// that happen during an open event are associated with the duration of this
// parent event and they are dropped if parent duration is shorter than
// the granularity.
संरचना अन्तर्_प्रगतिः_प्रविष्टिः {
  Time_Trace_Profiler_प्रविष्टिः Event;
  std::सदिशम्<Time_Trace_Profiler_प्रविष्टिः> InstantEvents;

  अन्तर्_प्रगतिः_प्रविष्टिः(TimePointType S, TimePointType E, std::string N,
                  std::string Dt, TimeTraceEventType Et)
      : Event(std::move(S), std::move(E), std::move(N), std::move(Dt), Et),
        InstantEvents() {}

  अन्तर्_प्रगतिः_प्रविष्टिः(TimePointType S, TimePointType E, std::string N,
                  TimeTraceMetadata Mt, TimeTraceEventType Et)
      : Event(std::move(S), std::move(E), std::move(N), std::move(Mt), Et),
        InstantEvents() {}
};

संरचना llvm::TimeTraceProfiler {
  TimeTraceProfiler(
      अचिह्नित TimeTraceGranularity = 0, StringRef ProcName = "",
      तार्किक TimeTraceVerbose = असत्य,
      TimeTraceCompression TimeTraceCompress = TimeTraceCompression::Infer)
      : Beginning_सम्बन्धि_Time(system_clock::now()), StartTime(ClockType::now()),
        ProcName(ProcName), Pid(sys::Process::getProcessId()),
        Tid(llvm::get_threadid()), TimeTraceGranularity(TimeTraceGranularity),
        TimeTraceVerbose(TimeTraceVerbose),
        TimeTraceCompress(TimeTraceCompress) {
    llvm::get_thread_name(ThreadName);
  }

  Time_Trace_Profiler_प्रविष्टिः *
  begin(std::string Name, llvm::function_ref<std::string()> Detail,
        TimeTraceEventType EventType = TimeTraceEventType::CompleteEvent) {
    प्रतिज्ञा(EventType != TimeTraceEventType::InstantEvent &&
           "Instant Events don't have begin and end.");
    राशिः.emplace_पृष्ठभागः(std::make_unique<अन्तर्_प्रगतिः_प्रविष्टिः>(
        ClockType::now(), TimePointType(), std::move(Name), Detail(),
        EventType));
    प्रत्यागम &राशिः.पृष्ठभागः()->Event;
  }

  Time_Trace_Profiler_प्रविष्टिः *
  begin(std::string Name, llvm::function_ref<TimeTraceMetadata()> Metadata,
        TimeTraceEventType EventType = TimeTraceEventType::CompleteEvent) {
    प्रतिज्ञा(EventType != TimeTraceEventType::InstantEvent &&
           "Instant Events don't have begin and end.");
    राशिः.emplace_पृष्ठभागः(std::make_unique<अन्तर्_प्रगतिः_प्रविष्टिः>(
        ClockType::now(), TimePointType(), std::move(Name), Metadata(),
        EventType));
    प्रत्यागम &राशिः.पृष्ठभागः()->Event;
  }

  शून्य निवेशय(std::string Name, llvm::function_ref<std::string()> Detail) {
    यदि (राशिः.रिक्तानि())
      प्रत्यागम;

    राशिः.पृष्ठभागः()->InstantEvents.emplace_पृष्ठभागः(Time_Trace_Profiler_प्रविष्टिः(
        ClockType::now(), TimePointType(), std::move(Name), Detail(),
        TimeTraceEventType::InstantEvent));
  }

  शून्य अन्तः() {
    प्रतिज्ञा(!राशिः.रिक्तानि() && "Must call begin() first");
    अन्तः(राशिः.पृष्ठभागः()->Event);
  }

  शून्य अन्तः(Time_Trace_Profiler_प्रविष्टिः &E) {
    प्रतिज्ञा(!राशिः.रिक्तानि() && "Must call begin() first");
    E.अन्तः_2 = ClockType::now();

    // Calculate duration at full precision for overall counts.
    DurationType Duration = E.अन्तः_2 - E.Start;

    ध्रुव स्वचालित *Iter =
        llvm::find_if(राशिः, [&](ध्रुव std::unique_ptr<अन्तर्_प्रगतिः_प्रविष्टिः> &Val) {
          प्रत्यागम &Val->Event == &E;
        });
    प्रतिज्ञा(Iter != राशिः.अन्तः() && "Event not in the Stack");

    // Only include sections longer or equal to TimeTraceGranularity msec.
    यदि (duration_cast<microseconds>(Duration).गणनम्() >= TimeTraceGranularity) {
      Entries.emplace_पृष्ठभागः(E);
      प्रति (स्वचालित &IE : Iter->get()->InstantEvents) {
        Entries.emplace_पृष्ठभागः(IE);
      }
    }

    // Track total time taken by each "name", but only the topmost levels of
    // them; e.g. if there's a template instantiation that instantiates other
    // templates from within, we only want to add the topmost one. "topmost"
    // happens to be the ones that don't have any currently open entries above
    // itself.
    यदि (llvm::none_सम्बन्धि(llvm::drop_begin(llvm::reverse(राशिः)),
                      [&](ध्रुव std::unique_ptr<अन्तर्_प्रगतिः_प्रविष्टिः> &Val) {
                        प्रत्यागम Val->Event.Name == E.Name;
                      })) {
      स्वचालित &गणनम्_And_योग = गणनम्_And_योग_प्रत्येकम्_Name[E.Name];
      गणनम्_And_योग.first++;
      गणनम्_And_योग.second += Duration;
    };

    राशिः.erase(Iter);
  }

  // Write events from this TimeTraceProfilerInstance and
  // ThreadTimeTraceProfilerInstances.
  शून्य लिख(raw_ostream &OS) {
    // Acquire Mutex as reading ThreadTimeTraceProfilerInstances.
    स्वचालित &Instances = getTimeTraceProfilerInstances();
    std::तालकम्_guard<std::mutex> तालकम्_2(Instances.तालकम्_2);
    प्रतिज्ञा(राशिः.रिक्तानि() &&
           "All profiler sections should be ended when calling write");
    प्रतिज्ञा(llvm::all_सम्बन्धि(Instances.List,
                        [](ध्रुव स्वचालित &TTP) { प्रत्यागम TTP->राशिः.रिक्तानि(); }) &&
           "All profiler sections should be ended when calling write");

    json::OStream J(OS);
    J.objectBegin();
    J.attributeBegin("traceEvents");
    J.arrayBegin();

    // Emit all events for the main flame graph.
    स्वचालित writeEvent = [&](ध्रुव स्वचालित &E, uint64_t Tid) {
      स्वचालित StartUs = E.getFlameGraphStartUs(StartTime);
      स्वचालित DurUs = E.getFlameGraphDurUs();

      J.object([&] {
        J.attribute("pid", Pid);
        J.attribute("tid", int64_t(Tid));
        J.attribute("ts", StartUs);
        यदि (E.EventType == TimeTraceEventType::AsyncEvent) {
          J.attribute("cat", E.Name);
          J.attribute("ph", "b");
          J.attribute("id", 0);
        } अन्यथा यदि (E.EventType == TimeTraceEventType::CompleteEvent) {
          J.attribute("ph", "X");
          J.attribute("dur", DurUs);
        } अन्यथा { // instant event
          प्रतिज्ञा(E.EventType == TimeTraceEventType::InstantEvent &&
                 "InstantEvent expected");
          J.attribute("ph", "i");
        }
        J.attribute("name", E.Name);
        यदि (!E.Metadata.is_रिक्तानि()) {
          J.attributeObject("args", [&] {
            यदि (!E.Metadata.Detail.रिक्तानि())
              J.attribute("detail", E.Metadata.Detail);
            यदि (!E.Metadata.File.रिक्तानि())
              J.attribute("file", E.Metadata.File);
            यदि (E.Metadata.पङ्क्तिः > 0)
              J.attribute("line", E.Metadata.पङ्क्तिः);
          });
        }
      });

      यदि (E.EventType == TimeTraceEventType::AsyncEvent) {
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
    प्रति (ध्रुव Time_Trace_Profiler_प्रविष्टिः &E : Entries)
      writeEvent(E, एतत्->Tid);
    प्रति (ध्रुव TimeTraceProfiler *TTP : Instances.List)
      प्रति (ध्रुव Time_Trace_Profiler_प्रविष्टिः &E : TTP->Entries)
        writeEvent(E, TTP->Tid);

    // Emit totals by section name as additional "thread" events, sorted from
    // longest one.
    // Find highest used thread id.
    uint64_t MaxTid = एतत्->Tid;
    प्रति (ध्रुव TimeTraceProfiler *TTP : Instances.List)
      MaxTid = std::max(MaxTid, TTP->Tid);

    // Combine all CountAndTotalPerName from threads into one.
    StringMap<गणनम्_And_Duration_Type> All_गणनम्_And_योग_प्रत्येकम्_Name;
    स्वचालित combineStat = [&](ध्रुव स्वचालित &Stat) {
      StringRef कुञ्चिका = Stat.get_कुञ्चिका();
      स्वचालित मानम् = Stat.get_मानम्();
      स्वचालित &गणनम्_And_योग = All_गणनम्_And_योग_प्रत्येकम्_Name[कुञ्चिका];
      गणनम्_And_योग.first += मानम्.first;
      गणनम्_And_योग.second += मानम्.second;
    };
    प्रति (ध्रुव स्वचालित &Stat : गणनम्_And_योग_प्रत्येकम्_Name)
      combineStat(Stat);
    प्रति (ध्रुव TimeTraceProfiler *TTP : Instances.List)
      प्रति (ध्रुव स्वचालित &Stat : TTP->गणनम्_And_योग_प्रत्येकम्_Name)
        combineStat(Stat);

    std::सदिशम्<Name_And_गणनम्_And_Duration_Type> SortedTotals;
    SortedTotals.reserve(All_गणनम्_And_योग_प्रत्येकम्_Name.आकार_2());
    प्रति (ध्रुव स्वचालित &योग_2 : All_गणनम्_And_योग_प्रत्येकम्_Name)
      SortedTotals.emplace_पृष्ठभागः(std::string(योग_2.get_कुञ्चिका()), योग_2.get_मानम्());

    llvm::क्रमय(SortedTotals, [](ध्रुव Name_And_गणनम्_And_Duration_Type &A,
                                ध्रुव Name_And_गणनम्_And_Duration_Type &B) {
      प्रत्यागम A.second.second > B.second.second;
    });

    // Report totals on separate threads of tracing file.
    uint64_t योग_Tid = MaxTid + 1;
    प्रति (ध्रुव Name_And_गणनम्_And_Duration_Type &योग_2 : SortedTotals) {
      स्वचालित DurUs = duration_cast<microseconds>(योग_2.second.second).गणनम्();
      स्वचालित गणनम्_2 = All_गणनम्_And_योग_प्रत्येकम्_Name[योग_2.first].first;

      J.object([&] {
        J.attribute("pid", Pid);
        J.attribute("tid", int64_t(योग_Tid));
        J.attribute("ph", "X");
        J.attribute("ts", 0);
        J.attribute("dur", DurUs);
        J.attribute("name", "Total " + योग_2.first);
        J.attributeObject("args", [&] {
          J.attribute("count", int64_t(गणनम्_2));
          J.attribute("avg ms", int64_t(DurUs / गणनम्_2 / 1000));
        });
      });

      ++योग_Tid;
    }

    स्वचालित writeMetadataEvent = [&](ध्रुव वर्ण *Name, uint64_t Tid,
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
    प्रति (ध्रुव TimeTraceProfiler *TTP : Instances.List)
      writeMetadataEvent("thread_name", TTP->Tid, TTP->ThreadName);

    J.array_अन्तः();
    J.attribute_अन्तः();

    // Emit the absolute time when this TimeProfiler started.
    // This can be used to combine the profiling data from
    // multiple processes and preserve actual time intervals.
    J.attribute("beginningOfTime",
                time_point_cast<microseconds>(Beginning_सम्बन्धि_Time)
                    .time_since_epoch()
                    .गणनम्());

    J.object_अन्तः();
  }

  Small_सदिशम्<std::unique_ptr<अन्तर्_प्रगतिः_प्रविष्टिः>, 16> राशिः;
  Small_सदिशम्<Time_Trace_Profiler_प्रविष्टिः, 128> Entries;
  StringMap<गणनम्_And_Duration_Type> गणनम्_And_योग_प्रत्येकम्_Name;
  // System clock time when the session was begun.
  ध्रुव time_point<system_clock> Beginning_सम्बन्धि_Time;
  // Profiling clock time when the session was begun.
  ध्रुव TimePointType StartTime;
  ध्रुव std::string ProcName;
  ध्रुव sys::Process::Pid Pid;
  SmallString<0> ThreadName;
  ध्रुव uint64_t Tid;

  // Minimum time granularity (in microseconds)
  ध्रुव अचिह्नित TimeTraceGranularity;

  // Make time trace capture verbose event details (e.g. source filenames). This
  // can increase the size of the output by 2-3 times.
  ध्रुव तार्किक TimeTraceVerbose;

  // Compression mode for output.
  TimeTraceCompression TimeTraceCompress;
};

तार्किक llvm::isTimeTraceVerbose() {
  प्रत्यागम getTimeTraceProfilerInstance() &&
         getTimeTraceProfilerInstance()->TimeTraceVerbose;
}

TimeTraceCompression llvm::infer_Time_Trace_Compression_From_मार्गः(StringRef मार्गः) {
  StringRef Ext = llvm::sys::मार्गः_2::extension(मार्गः);
  यदि (Ext.equals_insensitive(".zst") || Ext.equals_insensitive(".zstd"))
    प्रत्यागम TimeTraceCompression::Zstd;
  प्रत्यागम TimeTraceCompression::None;
}

शून्य llvm::timeTraceProfilerInitialize(अचिह्नित TimeTraceGranularity,
                                       StringRef ProcName,
                                       तार्किक TimeTraceVerbose,
                                       TimeTraceCompression TimeTraceCompress) {
  प्रतिज्ञा(TimeTraceProfilerInstance == शून्यसूचकः &&
         "Profiler should not be initialized");
  TimeTraceProfilerInstance = नूतन TimeTraceProfiler(
      TimeTraceGranularity, llvm::sys::मार्गः_2::filename(ProcName),
      TimeTraceVerbose, TimeTraceCompress);
}

// Removes all TimeTraceProfilerInstances.
// Called from main thread.
शून्य llvm::timeTraceProfilerCleanup() {
  विलोपय TimeTraceProfilerInstance;
  TimeTraceProfilerInstance = शून्यसूचकः;

  स्वचालित &Instances = getTimeTraceProfilerInstances();
  std::तालकम्_guard<std::mutex> तालकम्_2(Instances.तालकम्_2);
  प्रति (स्वचालित *TTP : Instances.List)
    विलोपय TTP;
  Instances.List.clear();
}

// Finish TimeTraceProfilerInstance on a worker thread.
// This doesn't remove the instance, just moves the pointer to global vector.
शून्य llvm::timeTraceProfilerFinishThread() {
  स्वचालित &Instances = getTimeTraceProfilerInstances();
  std::तालकम्_guard<std::mutex> तालकम्_2(Instances.तालकम्_2);
  Instances.List.प्रक्षिप_पृष्ठभागः(TimeTraceProfilerInstance);
  TimeTraceProfilerInstance = शून्यसूचकः;
}

शून्य llvm::timeTraceProfilerWrite(raw_pwrite_प्रवाहः &OS) {
  प्रतिज्ञा(TimeTraceProfilerInstance != शून्यसूचकः &&
         "Profiler object can't be null");
  TimeTraceCompression CompressType =
      TimeTraceProfilerInstance->TimeTraceCompress;
  यदि (CompressType != TimeTraceCompression::Zstd) {
    TimeTraceProfilerInstance->लिख(OS);
    प्रत्यागम;
  }

  compression::Format F = compression::Format::Zstd;
  यदि (ध्रुव वर्ण *Reason = compression::getReasonIfUnsupported(F))
    report_fatal_error(Reason);

  Small_सदिशम्<वर्ण, 0> Uncompressed;
  {
    raw_svector_ostream UncompressedOS(Uncompressed);
    TimeTraceProfilerInstance->लिख(UncompressedOS);
  }
  Small_सदिशम्<uint8_t, 0> Compressed;
  compression::compress(
      compression::Params(F),
      ArrayRef(reinterpret_cast<ध्रुव uint8_t *>(Uncompressed.दत्त()),
               Uncompressed.आकार_2()),
      Compressed);
  OS.लिख(reinterpret_cast<ध्रुव वर्ण *>(Compressed.दत्त()),
           Compressed.आकार_2());
}

Error llvm::timeTraceProfilerWrite(StringRef PreferredFileName,
                                   StringRef FallbackFileName) {
  प्रतिज्ञा(TimeTraceProfilerInstance != शून्यसूचकः &&
         "Profiler object can't be null");

  TimeTraceCompression CompressType =
      TimeTraceProfilerInstance->TimeTraceCompress;
  std::string मार्गः = PreferredFileName.str();
  यदि (मार्गः.रिक्तानि()) {
    मार्गः = FallbackFileName == "-" ? "out" : FallbackFileName.str();
    मार्गः += TimeTraceFileExtension;
    यदि (CompressType == TimeTraceCompression::Zstd)
      मार्गः += ".zst";
  } अन्यथा यदि (CompressType == TimeTraceCompression::Infer) {
    CompressType = infer_Time_Trace_Compression_From_मार्गः(मार्गः);
  }

  यदि (CompressType == TimeTraceCompression::Zstd) {
    यदि (ध्रुव वर्ण *Reason =
            compression::getReasonIfUnsupported(compression::Format::Zstd))
      प्रत्यागम createStringError(inconvertible_Error_संहिता(), Reason);
  }

  std::error_संहिता EC;
  sys::fs::OpenFlags Flags = CompressType == TimeTraceCompression::Zstd
                                 ? sys::fs::सम्बन्धि_None
                                 : sys::fs::सम्बन्धि_Text_With_CRLF;
  raw_fd_ostream OS(मार्गः, EC, Flags);
  यदि (EC)
    प्रत्यागम createStringError(EC, "Could not open " + मार्गः);

  TimeTraceCompression SavedCompress =
      TimeTraceProfilerInstance->TimeTraceCompress;
  TimeTraceProfilerInstance->TimeTraceCompress = CompressType;
  timeTraceProfilerWrite(OS);
  TimeTraceProfilerInstance->TimeTraceCompress = SavedCompress;
  प्रत्यागम Error::success();
}

Time_Trace_Profiler_प्रविष्टिः *llvm::timeTraceProfilerBegin(StringRef Name,
                                                     StringRef Detail) {
  यदि (TimeTraceProfilerInstance != शून्यसूचकः)
    प्रत्यागम TimeTraceProfilerInstance->begin(
        std::string(Name), [&]() { प्रत्यागम std::string(Detail); },
        TimeTraceEventType::CompleteEvent);
  प्रत्यागम शून्यसूचकः;
}

Time_Trace_Profiler_प्रविष्टिः *
llvm::timeTraceProfilerBegin(StringRef Name,
                             llvm::function_ref<std::string()> Detail) {
  यदि (TimeTraceProfilerInstance != शून्यसूचकः)
    प्रत्यागम TimeTraceProfilerInstance->begin(std::string(Name), Detail,
                                            TimeTraceEventType::CompleteEvent);
  प्रत्यागम शून्यसूचकः;
}

Time_Trace_Profiler_प्रविष्टिः *
llvm::timeTraceProfilerBegin(StringRef Name,
                             llvm::function_ref<TimeTraceMetadata()> Metadata) {
  यदि (TimeTraceProfilerInstance != शून्यसूचकः)
    प्रत्यागम TimeTraceProfilerInstance->begin(std::string(Name), Metadata,
                                            TimeTraceEventType::CompleteEvent);
  प्रत्यागम शून्यसूचकः;
}

Time_Trace_Profiler_प्रविष्टिः *llvm::timeTraceAsyncProfilerBegin(StringRef Name,
                                                          StringRef Detail) {
  यदि (TimeTraceProfilerInstance != शून्यसूचकः)
    प्रत्यागम TimeTraceProfilerInstance->begin(
        std::string(Name), [&]() { प्रत्यागम std::string(Detail); },
        TimeTraceEventType::AsyncEvent);
  प्रत्यागम शून्यसूचकः;
}

शून्य llvm::time_Trace_सङ्कलय_Instant_Event(StringRef Name,
                                    llvm::function_ref<std::string()> Detail) {
  यदि (TimeTraceProfilerInstance != शून्यसूचकः)
    TimeTraceProfilerInstance->निवेशय(std::string(Name), Detail);
}

शून्य llvm::time_Trace_Profiler_अन्तः() {
  यदि (TimeTraceProfilerInstance != शून्यसूचकः)
    TimeTraceProfilerInstance->अन्तः();
}

शून्य llvm::time_Trace_Profiler_अन्तः(Time_Trace_Profiler_प्रविष्टिः *E) {
  यदि (TimeTraceProfilerInstance != शून्यसूचकः)
    TimeTraceProfilerInstance->अन्तः(*E);
}
