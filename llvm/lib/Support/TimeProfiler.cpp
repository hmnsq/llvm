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

#అన్తర్భావ "llvm/Support/TimeProfiler.h"
#అన్తర్భావ "llvm/ADT/STLExtras.h"
#అన్తర్భావ "llvm/ADT/STLFunctionalExtras.h"
#అన్తర్భావ "llvm/ADT/SmallVector.h"
#అన్తర్భావ "llvm/ADT/StringMap.h"
#అన్తర్భావ "llvm/Support/Compression.h"
#అన్తర్భావ "llvm/Support/JSON.h"
#అన్తర్భావ "llvm/Support/Path.h"
#అన్తర్భావ "llvm/Support/Process.h"
#అన్తర్భావ "llvm/Support/Threading.h"
#అన్తర్భావ <algorithm>
#అన్తర్భావ <cassert>
#అన్తర్భావ <chrono>
#అన్తర్భావ <memory>
#అన్తర్భావ <mutex>
#అన్తర్భావ <string>
#అన్తర్భావ <vector>

ఉపయోగించి నామాకాశ llvm;

నామాకాశ {

ఉపయోగించి std::chrono::duration;
ఉపయోగించి std::chrono::duration_cast;
ఉపయోగించి std::chrono::microseconds;
ఉపయోగించి std::chrono::steady_clock;
ఉపయోగించి std::chrono::system_clock;
ఉపయోగించి std::chrono::సమయం_బిందువు_2;
ఉపయోగించి std::chrono::సమయం_బిందువు_cast;

నిర్మాణం సమయం_Trace_Profiler_Instances {
  std::mutex తాళం_3;
  std::సదిశ<సమయం_Trace_Profiler *> List;
};

సమయం_Trace_Profiler_Instances &get_సమయం_Trace_Profiler_Instances() {
  స్థిర సమయం_Trace_Profiler_Instances Instances;
  తిరిగి Instances;
}

స్థిర స్థిరరాశి అక్షరం సమయం_Trace_File_Extension[] = ".time-trace.json";

} // anonymous namespace

// Per Thread instance
స్థిర LLVM_THREAD_LOCAL సమయం_Trace_Profiler *సమయం_Trace_Profiler_Instance = శూన్యసూచిక;

సమయం_Trace_Profiler *llvm::get_సమయం_Trace_Profiler_Instance() {
  తిరిగి సమయం_Trace_Profiler_Instance;
}

నామాకాశ {

ఉపయోగించి ClockType = steady_clock;
ఉపయోగించి సమయం_బిందువు_Type = సమయం_బిందువు_2<ClockType>;
ఉపయోగించి DurationType = duration<ClockType::rep, ClockType::period>;
ఉపయోగించి లెక్క_And_Duration_Type = std::pair<size_t, DurationType>;
ఉపయోగించి Name_And_లెక్క_And_Duration_Type =
    std::pair<std::string, లెక్క_And_Duration_Type>;

} // anonymous namespace

/// Represents an open or completed time section entry to be captured.
నిర్మాణం llvm::సమయం_Trace_Profiler_నమోదు {
  స్థిరరాశి సమయం_బిందువు_Type ప్రారంభం;
  సమయం_బిందువు_Type చివర_2;
  స్థిరరాశి std::string Name;
  సమయం_Trace_Metadata Metadata;

  స్థిరరాశి సమయం_Trace_Event_Type EventType = సమయం_Trace_Event_Type::CompleteEvent;
  సమయం_Trace_Profiler_నమోదు(సమయం_బిందువు_Type &&S, సమయం_బిందువు_Type &&E, std::string &&N,
                         std::string &&Dt, సమయం_Trace_Event_Type Et)
      : ప్రారంభం(std::move(S)), చివర_2(std::move(E)), Name(std::move(N)), Metadata(),
        EventType(Et) {
    Metadata.Detail = std::move(Dt);
  }

  సమయం_Trace_Profiler_నమోదు(సమయం_బిందువు_Type &&S, సమయం_బిందువు_Type &&E, std::string &&N,
                         సమయం_Trace_Metadata &&Mt, సమయం_Trace_Event_Type Et)
      : ప్రారంభం(std::move(S)), చివర_2(std::move(E)), Name(std::move(N)),
        Metadata(std::move(Mt)), EventType(Et) {}

  // Calculate timings for FlameGraph. Cast time points to microsecond precision
  // rather than casting duration. This avoids truncation issues causing inner
  // scopes overruning outer scopes.
  ClockType::rep get_Flame_లేఖాచిత్రం_ప్రారంభం_Us(సమయం_బిందువు_Type ప్రారంభం_సమయం) స్థిరరాశి {
    తిరిగి (సమయం_బిందువు_cast<microseconds>(ప్రారంభం) -
            సమయం_బిందువు_cast<microseconds>(ప్రారంభం_సమయం))
        .లెక్క();
  }

  ClockType::rep get_Flame_లేఖాచిత్రం_Dur_Us() స్థిరరాశి {
    తిరిగి (సమయం_బిందువు_cast<microseconds>(చివర_2) -
            సమయం_బిందువు_cast<microseconds>(ప్రారంభం))
        .లెక్క();
  }
};

// Represents a currently open (in-progress) time trace entry. InstantEvents
// that happen during an open event are associated with the duration of this
// parent event and they are dropped if parent duration is shorter than
// the granularity.
నిర్మాణం లోపల_పురోగతి_నమోదు {
  సమయం_Trace_Profiler_నమోదు Event;
  std::సదిశ<సమయం_Trace_Profiler_నమోదు> InstantEvents;

  లోపల_పురోగతి_నమోదు(సమయం_బిందువు_Type S, సమయం_బిందువు_Type E, std::string N,
                  std::string Dt, సమయం_Trace_Event_Type Et)
      : Event(std::move(S), std::move(E), std::move(N), std::move(Dt), Et),
        InstantEvents() {}

  లోపల_పురోగతి_నమోదు(సమయం_బిందువు_Type S, సమయం_బిందువు_Type E, std::string N,
                  సమయం_Trace_Metadata Mt, సమయం_Trace_Event_Type Et)
      : Event(std::move(S), std::move(E), std::move(N), std::move(Mt), Et),
        InstantEvents() {}
};

నిర్మాణం llvm::సమయం_Trace_Profiler {
  సమయం_Trace_Profiler(
      గుర్తులేని సమయం_Trace_Granularity = 0, StringRef ProcName = "",
      తార్కిక సమయం_Trace_Verbose = అబద్ధం,
      సమయం_Trace_Compression సమయం_Trace_Compress = సమయం_Trace_Compression::Infer)
      : Beginning_యొక్క_సమయం(system_clock::now()), ప్రారంభం_సమయం(ClockType::now()),
        ProcName(ProcName), Pid(sys::Process::getProcessId()),
        Tid(llvm::get_threadid()), సమయం_Trace_Granularity(సమయం_Trace_Granularity),
        సమయం_Trace_Verbose(సమయం_Trace_Verbose),
        సమయం_Trace_Compress(సమయం_Trace_Compress) {
    llvm::get_thread_name(ThreadName);
  }

  సమయం_Trace_Profiler_నమోదు *
  begin(std::string Name, llvm::function_ref<std::string()> Detail,
        సమయం_Trace_Event_Type EventType = సమయం_Trace_Event_Type::CompleteEvent) {
    ప్రతిజ్ఞా(EventType != సమయం_Trace_Event_Type::InstantEvent &&
           "Instant Events don't have begin and end.");
    దొంతర.emplace_వెనుక(std::make_unique<లోపల_పురోగతి_నమోదు>(
        ClockType::now(), సమయం_బిందువు_Type(), std::move(Name), Detail(),
        EventType));
    తిరిగి &దొంతర.వెనుక()->Event;
  }

  సమయం_Trace_Profiler_నమోదు *
  begin(std::string Name, llvm::function_ref<సమయం_Trace_Metadata()> Metadata,
        సమయం_Trace_Event_Type EventType = సమయం_Trace_Event_Type::CompleteEvent) {
    ప్రతిజ్ఞా(EventType != సమయం_Trace_Event_Type::InstantEvent &&
           "Instant Events don't have begin and end.");
    దొంతర.emplace_వెనుక(std::make_unique<లోపల_పురోగతి_నమోదు>(
        ClockType::now(), సమయం_బిందువు_Type(), std::move(Name), Metadata(),
        EventType));
    తిరిగి &దొంతర.వెనుక()->Event;
  }

  శూన్య చొప్పించు(std::string Name, llvm::function_ref<std::string()> Detail) {
    ఒకవేళ (దొంతర.ఖాళీలు())
      తిరిగి;

    దొంతర.వెనుక()->InstantEvents.emplace_వెనుక(సమయం_Trace_Profiler_నమోదు(
        ClockType::now(), సమయం_బిందువు_Type(), std::move(Name), Detail(),
        సమయం_Trace_Event_Type::InstantEvent));
  }

  శూన్య చివర() {
    ప్రతిజ్ఞా(!దొంతర.ఖాళీలు() && "Must call begin() first");
    చివర(దొంతర.వెనుక()->Event);
  }

  శూన్య చివర(సమయం_Trace_Profiler_నమోదు &E) {
    ప్రతిజ్ఞా(!దొంతర.ఖాళీలు() && "Must call begin() first");
    E.చివర_2 = ClockType::now();

    // Calculate duration at full precision for overall counts.
    DurationType Duration = E.చివర_2 - E.ప్రారంభం;

    స్థిరరాశి స్వయంచాలక *Iter =
        llvm::కనుగొను_if(దొంతర, [&](స్థిరరాశి std::unique_ptr<లోపల_పురోగతి_నమోదు> &Val) {
          తిరిగి &Val->Event == &E;
        });
    ప్రతిజ్ఞా(Iter != దొంతర.చివర() && "Event not in the Stack");

    // Only include sections longer or equal to TimeTraceGranularity msec.
    ఒకవేళ (duration_cast<microseconds>(Duration).లెక్క() >= సమయం_Trace_Granularity) {
      Entries.emplace_వెనుక(E);
      కొరకు (స్వయంచాలక &IE : Iter->get()->InstantEvents) {
        Entries.emplace_వెనుక(IE);
      }
    }

    // Track total time taken by each "name", but only the topmost levels of
    // them; e.g. if there's a template instantiation that instantiates other
    // templates from within, we only want to add the topmost one. "topmost"
    // happens to be the ones that don't have any currently open entries above
    // itself.
    ఒకవేళ (llvm::none_యొక్క(llvm::drop_begin(llvm::తిరగేయి(దొంతర)),
                      [&](స్థిరరాశి std::unique_ptr<లోపల_పురోగతి_నమోదు> &Val) {
                        తిరిగి Val->Event.Name == E.Name;
                      })) {
      స్వయంచాలక &లెక్క_And_మొత్తం = లెక్క_And_మొత్తం_చొప్పున_Name[E.Name];
      లెక్క_And_మొత్తం.మొదటి_2++;
      లెక్క_And_మొత్తం.రెండవ_2 += Duration;
    };

    దొంతర.erase(Iter);
  }

  // Write events from this TimeTraceProfilerInstance and
  // ThreadTimeTraceProfilerInstances.
  శూన్య లిఖ(raw_ostream &OS) {
    // Acquire Mutex as reading ThreadTimeTraceProfilerInstances.
    స్వయంచాలక &Instances = get_సమయం_Trace_Profiler_Instances();
    std::తాళం_guard<std::mutex> తాళం_3(Instances.తాళం_3);
    ప్రతిజ్ఞా(దొంతర.ఖాళీలు() &&
           "All profiler sections should be ended when calling write");
    ప్రతిజ్ఞా(llvm::all_యొక్క(Instances.List,
                        [](స్థిరరాశి స్వయంచాలక &TTP) { తిరిగి TTP->దొంతర.ఖాళీలు(); }) &&
           "All profiler sections should be ended when calling write");

    json::OStream J(OS);
    J.objectBegin();
    J.attributeBegin("traceEvents");
    J.arrayBegin();

    // Emit all events for the main flame graph.
    స్వయంచాలక writeEvent = [&](స్థిరరాశి స్వయంచాలక &E, uint64_t Tid) {
      స్వయంచాలక ప్రారంభం_Us = E.get_Flame_లేఖాచిత్రం_ప్రారంభం_Us(ప్రారంభం_సమయం);
      స్వయంచాలక DurUs = E.get_Flame_లేఖాచిత్రం_Dur_Us();

      J.object([&] {
        J.attribute("pid", Pid);
        J.attribute("tid", int64_t(Tid));
        J.attribute("ts", ప్రారంభం_Us);
        ఒకవేళ (E.EventType == సమయం_Trace_Event_Type::AsyncEvent) {
          J.attribute("cat", E.Name);
          J.attribute("ph", "b");
          J.attribute("id", 0);
        } లేకపోతే ఒకవేళ (E.EventType == సమయం_Trace_Event_Type::CompleteEvent) {
          J.attribute("ph", "X");
          J.attribute("dur", DurUs);
        } లేకపోతే { // instant event
          ప్రతిజ్ఞా(E.EventType == సమయం_Trace_Event_Type::InstantEvent &&
                 "InstantEvent expected");
          J.attribute("ph", "i");
        }
        J.attribute("name", E.Name);
        ఒకవేళ (!E.Metadata.is_ఖాళీలు()) {
          J.attributeObject("args", [&] {
            ఒకవేళ (!E.Metadata.Detail.ఖాళీలు())
              J.attribute("detail", E.Metadata.Detail);
            ఒకవేళ (!E.Metadata.File.ఖాళీలు())
              J.attribute("file", E.Metadata.File);
            ఒకవేళ (E.Metadata.పంక్తి > 0)
              J.attribute("line", E.Metadata.పంక్తి);
          });
        }
      });

      ఒకవేళ (E.EventType == సమయం_Trace_Event_Type::AsyncEvent) {
        J.object([&] {
          J.attribute("pid", Pid);
          J.attribute("tid", int64_t(Tid));
          J.attribute("ts", ప్రారంభం_Us + DurUs);
          J.attribute("cat", E.Name);
          J.attribute("ph", "e");
          J.attribute("id", 0);
          J.attribute("name", E.Name);
        });
      }
    };
    కొరకు (స్థిరరాశి సమయం_Trace_Profiler_నమోదు &E : Entries)
      writeEvent(E, ఇది->Tid);
    కొరకు (స్థిరరాశి సమయం_Trace_Profiler *TTP : Instances.List)
      కొరకు (స్థిరరాశి సమయం_Trace_Profiler_నమోదు &E : TTP->Entries)
        writeEvent(E, TTP->Tid);

    // Emit totals by section name as additional "thread" events, sorted from
    // longest one.
    // Find highest used thread id.
    uint64_t MaxTid = ఇది->Tid;
    కొరకు (స్థిరరాశి సమయం_Trace_Profiler *TTP : Instances.List)
      MaxTid = std::max(MaxTid, TTP->Tid);

    // Combine all CountAndTotalPerName from threads into one.
    StringMap<లెక్క_And_Duration_Type> All_లెక్క_And_మొత్తం_చొప్పున_Name;
    స్వయంచాలక combineStat = [&](స్థిరరాశి స్వయంచాలక &Stat) {
      StringRef కీలకం = Stat.get_కీలకం();
      స్వయంచాలక విలువ = Stat.get_విలువ();
      స్వయంచాలక &లెక్క_And_మొత్తం = All_లెక్క_And_మొత్తం_చొప్పున_Name[కీలకం];
      లెక్క_And_మొత్తం.మొదటి_2 += విలువ.మొదటి_2;
      లెక్క_And_మొత్తం.రెండవ_2 += విలువ.రెండవ_2;
    };
    కొరకు (స్థిరరాశి స్వయంచాలక &Stat : లెక్క_And_మొత్తం_చొప్పున_Name)
      combineStat(Stat);
    కొరకు (స్థిరరాశి సమయం_Trace_Profiler *TTP : Instances.List)
      కొరకు (స్థిరరాశి స్వయంచాలక &Stat : TTP->లెక్క_And_మొత్తం_చొప్పున_Name)
        combineStat(Stat);

    std::సదిశ<Name_And_లెక్క_And_Duration_Type> SortedTotals;
    SortedTotals.reserve(All_లెక్క_And_మొత్తం_చొప్పున_Name.కొలత_2());
    కొరకు (స్థిరరాశి స్వయంచాలక &మొత్తం : All_లెక్క_And_మొత్తం_చొప్పున_Name)
      SortedTotals.emplace_వెనుక(std::string(మొత్తం.get_కీలకం()), మొత్తం.get_విలువ());

    llvm::క్రమీకరించు(SortedTotals, [](స్థిరరాశి Name_And_లెక్క_And_Duration_Type &A,
                                స్థిరరాశి Name_And_లెక్క_And_Duration_Type &B) {
      తిరిగి A.రెండవ_2.రెండవ_2 > B.రెండవ_2.రెండవ_2;
    });

    // Report totals on separate threads of tracing file.
    uint64_t మొత్తం_Tid = MaxTid + 1;
    కొరకు (స్థిరరాశి Name_And_లెక్క_And_Duration_Type &మొత్తం : SortedTotals) {
      స్వయంచాలక DurUs = duration_cast<microseconds>(మొత్తం.రెండవ_2.రెండవ_2).లెక్క();
      స్వయంచాలక లెక్క_2 = All_లెక్క_And_మొత్తం_చొప్పున_Name[మొత్తం.మొదటి_2].మొదటి_2;

      J.object([&] {
        J.attribute("pid", Pid);
        J.attribute("tid", int64_t(మొత్తం_Tid));
        J.attribute("ph", "X");
        J.attribute("ts", 0);
        J.attribute("dur", DurUs);
        J.attribute("name", "Total " + మొత్తం.మొదటి_2);
        J.attributeObject("args", [&] {
          J.attribute("count", int64_t(లెక్క_2));
          J.attribute("avg ms", int64_t(DurUs / లెక్క_2 / 1000));
        });
      });

      ++మొత్తం_Tid;
    }

    స్వయంచాలక writeMetadataEvent = [&](స్థిరరాశి అక్షరం *Name, uint64_t Tid,
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
    కొరకు (స్థిరరాశి సమయం_Trace_Profiler *TTP : Instances.List)
      writeMetadataEvent("thread_name", TTP->Tid, TTP->ThreadName);

    J.array_చివర();
    J.attribute_చివర();

    // Emit the absolute time when this TimeProfiler started.
    // This can be used to combine the profiling data from
    // multiple processes and preserve actual time intervals.
    J.attribute("beginningOfTime",
                సమయం_బిందువు_cast<microseconds>(Beginning_యొక్క_సమయం)
                    .సమయం_since_epoch()
                    .లెక్క());

    J.object_చివర();
  }

  Small_సదిశ<std::unique_ptr<లోపల_పురోగతి_నమోదు>, 16> దొంతర;
  Small_సదిశ<సమయం_Trace_Profiler_నమోదు, 128> Entries;
  StringMap<లెక్క_And_Duration_Type> లెక్క_And_మొత్తం_చొప్పున_Name;
  // System clock time when the session was begun.
  స్థిరరాశి సమయం_బిందువు_2<system_clock> Beginning_యొక్క_సమయం;
  // Profiling clock time when the session was begun.
  స్థిరరాశి సమయం_బిందువు_Type ప్రారంభం_సమయం;
  స్థిరరాశి std::string ProcName;
  స్థిరరాశి sys::Process::Pid Pid;
  SmallString<0> ThreadName;
  స్థిరరాశి uint64_t Tid;

  // Minimum time granularity (in microseconds)
  స్థిరరాశి గుర్తులేని సమయం_Trace_Granularity;

  // Make time trace capture verbose event details (e.g. source filenames). This
  // can increase the size of the output by 2-3 times.
  స్థిరరాశి తార్కిక సమయం_Trace_Verbose;

  // Compression mode for output.
  సమయం_Trace_Compression సమయం_Trace_Compress;
};

తార్కిక llvm::is_సమయం_Trace_Verbose() {
  తిరిగి get_సమయం_Trace_Profiler_Instance() &&
         get_సమయం_Trace_Profiler_Instance()->సమయం_Trace_Verbose;
}

సమయం_Trace_Compression llvm::infer_సమయం_Trace_Compression_From_మార్గం(StringRef మార్గం) {
  StringRef Ext = llvm::sys::మార్గం_2::extension(మార్గం);
  ఒకవేళ (Ext.equals_insensitive(".zst") || Ext.equals_insensitive(".zstd"))
    తిరిగి సమయం_Trace_Compression::Zstd;
  తిరిగి సమయం_Trace_Compression::None;
}

శూన్య llvm::సమయం_Trace_Profiler_Initialize(గుర్తులేని సమయం_Trace_Granularity,
                                       StringRef ProcName,
                                       తార్కిక సమయం_Trace_Verbose,
                                       సమయం_Trace_Compression సమయం_Trace_Compress) {
  ప్రతిజ్ఞా(సమయం_Trace_Profiler_Instance == శూన్యసూచిక &&
         "Profiler should not be initialized");
  సమయం_Trace_Profiler_Instance = కొత్త సమయం_Trace_Profiler(
      సమయం_Trace_Granularity, llvm::sys::మార్గం_2::filename(ProcName),
      సమయం_Trace_Verbose, సమయం_Trace_Compress);
}

// Removes all TimeTraceProfilerInstances.
// Called from main thread.
శూన్య llvm::సమయం_Trace_Profiler_Cleanup() {
  తొలగించు సమయం_Trace_Profiler_Instance;
  సమయం_Trace_Profiler_Instance = శూన్యసూచిక;

  స్వయంచాలక &Instances = get_సమయం_Trace_Profiler_Instances();
  std::తాళం_guard<std::mutex> తాళం_3(Instances.తాళం_3);
  కొరకు (స్వయంచాలక *TTP : Instances.List)
    తొలగించు TTP;
  Instances.List.clear();
}

// Finish TimeTraceProfilerInstance on a worker thread.
// This doesn't remove the instance, just moves the pointer to global vector.
శూన్య llvm::సమయం_Trace_Profiler_ముగింపు_Thread() {
  స్వయంచాలక &Instances = get_సమయం_Trace_Profiler_Instances();
  std::తాళం_guard<std::mutex> తాళం_3(Instances.తాళం_3);
  Instances.List.నెట్టు_వెనుక(సమయం_Trace_Profiler_Instance);
  సమయం_Trace_Profiler_Instance = శూన్యసూచిక;
}

శూన్య llvm::సమయం_Trace_Profiler_Write(raw_pwrite_ప్రవాహం &OS) {
  ప్రతిజ్ఞా(సమయం_Trace_Profiler_Instance != శూన్యసూచిక &&
         "Profiler object can't be null");
  సమయం_Trace_Compression CompressType =
      సమయం_Trace_Profiler_Instance->సమయం_Trace_Compress;
  ఒకవేళ (CompressType != సమయం_Trace_Compression::Zstd) {
    సమయం_Trace_Profiler_Instance->లిఖ(OS);
    తిరిగి;
  }

  compression::Format F = compression::Format::Zstd;
  ఒకవేళ (స్థిరరాశి అక్షరం *Reason = compression::getReasonIfUnsupported(F))
    report_fatal_error(Reason);

  Small_సదిశ<అక్షరం, 0> Uncompressed;
  {
    raw_svector_ostream UncompressedOS(Uncompressed);
    సమయం_Trace_Profiler_Instance->లిఖ(UncompressedOS);
  }
  Small_సదిశ<uint8_t, 0> Compressed;
  compression::compress(
      compression::Params(F),
      ArrayRef(reinterpret_cast<స్థిరరాశి uint8_t *>(Uncompressed.దత్తాంశం()),
               Uncompressed.కొలత_2()),
      Compressed);
  OS.లిఖ(reinterpret_cast<స్థిరరాశి అక్షరం *>(Compressed.దత్తాంశం()),
           Compressed.కొలత_2());
}

Error llvm::సమయం_Trace_Profiler_Write(StringRef PreferredFileName,
                                   StringRef FallbackFileName) {
  ప్రతిజ్ఞా(సమయం_Trace_Profiler_Instance != శూన్యసూచిక &&
         "Profiler object can't be null");

  సమయం_Trace_Compression CompressType =
      సమయం_Trace_Profiler_Instance->సమయం_Trace_Compress;
  std::string మార్గం = PreferredFileName.str();
  ఒకవేళ (మార్గం.ఖాళీలు()) {
    మార్గం = FallbackFileName == "-" ? "out" : FallbackFileName.str();
    మార్గం += సమయం_Trace_File_Extension;
    ఒకవేళ (CompressType == సమయం_Trace_Compression::Zstd)
      మార్గం += ".zst";
  } లేకపోతే ఒకవేళ (CompressType == సమయం_Trace_Compression::Infer) {
    CompressType = infer_సమయం_Trace_Compression_From_మార్గం(మార్గం);
  }

  ఒకవేళ (CompressType == సమయం_Trace_Compression::Zstd) {
    ఒకవేళ (స్థిరరాశి అక్షరం *Reason =
            compression::getReasonIfUnsupported(compression::Format::Zstd))
      తిరిగి createStringError(inconvertible_Error_సంకేతం(), Reason);
  }

  std::error_సంకేతం EC;
  sys::fs::OpenFlags Flags = CompressType == సమయం_Trace_Compression::Zstd
                                 ? sys::fs::యొక్క_None
                                 : sys::fs::యొక్క_పాఠ్యం_With_CRLF;
  raw_fd_ostream OS(మార్గం, EC, Flags);
  ఒకవేళ (EC)
    తిరిగి createStringError(EC, "Could not open " + మార్గం);

  సమయం_Trace_Compression SavedCompress =
      సమయం_Trace_Profiler_Instance->సమయం_Trace_Compress;
  సమయం_Trace_Profiler_Instance->సమయం_Trace_Compress = CompressType;
  సమయం_Trace_Profiler_Write(OS);
  సమయం_Trace_Profiler_Instance->సమయం_Trace_Compress = SavedCompress;
  తిరిగి Error::success();
}

సమయం_Trace_Profiler_నమోదు *llvm::సమయం_Trace_Profiler_Begin(StringRef Name,
                                                     StringRef Detail) {
  ఒకవేళ (సమయం_Trace_Profiler_Instance != శూన్యసూచిక)
    తిరిగి సమయం_Trace_Profiler_Instance->begin(
        std::string(Name), [&]() { తిరిగి std::string(Detail); },
        సమయం_Trace_Event_Type::CompleteEvent);
  తిరిగి శూన్యసూచిక;
}

సమయం_Trace_Profiler_నమోదు *
llvm::సమయం_Trace_Profiler_Begin(StringRef Name,
                             llvm::function_ref<std::string()> Detail) {
  ఒకవేళ (సమయం_Trace_Profiler_Instance != శూన్యసూచిక)
    తిరిగి సమయం_Trace_Profiler_Instance->begin(std::string(Name), Detail,
                                            సమయం_Trace_Event_Type::CompleteEvent);
  తిరిగి శూన్యసూచిక;
}

సమయం_Trace_Profiler_నమోదు *
llvm::సమయం_Trace_Profiler_Begin(StringRef Name,
                             llvm::function_ref<సమయం_Trace_Metadata()> Metadata) {
  ఒకవేళ (సమయం_Trace_Profiler_Instance != శూన్యసూచిక)
    తిరిగి సమయం_Trace_Profiler_Instance->begin(std::string(Name), Metadata,
                                            సమయం_Trace_Event_Type::CompleteEvent);
  తిరిగి శూన్యసూచిక;
}

సమయం_Trace_Profiler_నమోదు *llvm::సమయం_Trace_Async_Profiler_Begin(StringRef Name,
                                                          StringRef Detail) {
  ఒకవేళ (సమయం_Trace_Profiler_Instance != శూన్యసూచిక)
    తిరిగి సమయం_Trace_Profiler_Instance->begin(
        std::string(Name), [&]() { తిరిగి std::string(Detail); },
        సమయం_Trace_Event_Type::AsyncEvent);
  తిరిగి శూన్యసూచిక;
}

శూన్య llvm::సమయం_Trace_కూడు_Instant_Event(StringRef Name,
                                    llvm::function_ref<std::string()> Detail) {
  ఒకవేళ (సమయం_Trace_Profiler_Instance != శూన్యసూచిక)
    సమయం_Trace_Profiler_Instance->చొప్పించు(std::string(Name), Detail);
}

శూన్య llvm::సమయం_Trace_Profiler_చివర() {
  ఒకవేళ (సమయం_Trace_Profiler_Instance != శూన్యసూచిక)
    సమయం_Trace_Profiler_Instance->చివర();
}

శూన్య llvm::సమయం_Trace_Profiler_చివర(సమయం_Trace_Profiler_నమోదు *E) {
  ఒకవేళ (సమయం_Trace_Profiler_Instance != శూన్యసూచిక)
    సమయం_Trace_Profiler_Instance->చివర(*E);
}
