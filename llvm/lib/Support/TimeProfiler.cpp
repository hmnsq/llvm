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

#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/TimeProfiler.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/ADT/STLExtras.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/ADT/STLFunctionalExtras.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/ADT/SmallVector.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/ADT/StringMap.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/Compression.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/JSON.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/Path.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/Process.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/Threading.h"
#ਅਨ੍ਤਰ੍ਭਾਵ <algorithm>
#ਅਨ੍ਤਰ੍ਭਾਵ <cassert>
#ਅਨ੍ਤਰ੍ਭਾਵ <chrono>
#ਅਨ੍ਤਰ੍ਭਾਵ <memory>
#ਅਨ੍ਤਰ੍ਭਾਵ <mutex>
#ਅਨ੍ਤਰ੍ਭਾਵ <string>
#ਅਨ੍ਤਰ੍ਭਾਵ <vector>

ਵਰਤੋਂ ਨਾਮਾਕਾਸ਼ llvm;

ਨਾਮਾਕਾਸ਼ {

ਵਰਤੋਂ std::chrono::duration;
ਵਰਤੋਂ std::chrono::duration_cast;
ਵਰਤੋਂ std::chrono::microseconds;
ਵਰਤੋਂ std::chrono::steady_clock;
ਵਰਤੋਂ std::chrono::system_clock;
ਵਰਤੋਂ std::chrono::ਸਮਾਂ_ਬਿੰਦੂ_2;
ਵਰਤੋਂ std::chrono::ਸਮਾਂ_ਬਿੰਦੂ_cast;

ਬਣਤਰ ਸਮਾਂ_Trace_Profiler_Instances {
  std::mutex ਤਾਲਾ_2;
  std::ਸਦਿਸ਼<ਸਮਾਂ_Trace_Profiler *> List;
};

ਸਮਾਂ_Trace_Profiler_Instances &get_ਸਮਾਂ_Trace_Profiler_Instances() {
  ਸਥਿਰ ਸਮਾਂ_Trace_Profiler_Instances Instances;
  ਵਾਪਸ Instances;
}

ਸਥਿਰ ਅਚਲ ਅੱਖਰ ਸਮਾਂ_Trace_File_Extension[] = ".time-trace.json";

} // anonymous namespace

// Per Thread instance
ਸਥਿਰ LLVM_THREAD_LOCAL ਸਮਾਂ_Trace_Profiler *ਸਮਾਂ_Trace_Profiler_Instance = ਖਾਲੀ_ਸੂਚਕ;

ਸਮਾਂ_Trace_Profiler *llvm::get_ਸਮਾਂ_Trace_Profiler_Instance() {
  ਵਾਪਸ ਸਮਾਂ_Trace_Profiler_Instance;
}

ਨਾਮਾਕਾਸ਼ {

ਵਰਤੋਂ ClockType = steady_clock;
ਵਰਤੋਂ ਸਮਾਂ_ਬਿੰਦੂ_Type = ਸਮਾਂ_ਬਿੰਦੂ_2<ClockType>;
ਵਰਤੋਂ DurationType = duration<ClockType::rep, ClockType::period>;
ਵਰਤੋਂ ਗਿਣਤੀ_And_Duration_Type = std::pair<size_t, DurationType>;
ਵਰਤੋਂ Name_And_ਗਿਣਤੀ_And_Duration_Type =
    std::pair<std::string, ਗਿਣਤੀ_And_Duration_Type>;

} // anonymous namespace

/// Represents an open or completed time section entry to be captured.
ਬਣਤਰ llvm::ਸਮਾਂ_Trace_Profiler_ਇੰਦਰਾਜ {
  ਅਚਲ ਸਮਾਂ_ਬਿੰਦੂ_Type ਸ਼ੁਰੂ;
  ਸਮਾਂ_ਬਿੰਦੂ_Type ਸਿਰਾ_2;
  ਅਚਲ std::string Name;
  ਸਮਾਂ_Trace_Metadata Metadata;

  ਅਚਲ ਸਮਾਂ_Trace_Event_Type EventType = ਸਮਾਂ_Trace_Event_Type::CompleteEvent;
  ਸਮਾਂ_Trace_Profiler_ਇੰਦਰਾਜ(ਸਮਾਂ_ਬਿੰਦੂ_Type &&S, ਸਮਾਂ_ਬਿੰਦੂ_Type &&E, std::string &&N,
                         std::string &&Dt, ਸਮਾਂ_Trace_Event_Type Et)
      : ਸ਼ੁਰੂ(std::move(S)), ਸਿਰਾ_2(std::move(E)), Name(std::move(N)), Metadata(),
        EventType(Et) {
    Metadata.Detail = std::move(Dt);
  }

  ਸਮਾਂ_Trace_Profiler_ਇੰਦਰਾਜ(ਸਮਾਂ_ਬਿੰਦੂ_Type &&S, ਸਮਾਂ_ਬਿੰਦੂ_Type &&E, std::string &&N,
                         ਸਮਾਂ_Trace_Metadata &&Mt, ਸਮਾਂ_Trace_Event_Type Et)
      : ਸ਼ੁਰੂ(std::move(S)), ਸਿਰਾ_2(std::move(E)), Name(std::move(N)),
        Metadata(std::move(Mt)), EventType(Et) {}

  // Calculate timings for FlameGraph. Cast time points to microsecond precision
  // rather than casting duration. This avoids truncation issues causing inner
  // scopes overruning outer scopes.
  ClockType::rep get_Flame_ਗ੍ਰਾਫ_ਸ਼ੁਰੂ_Us(ਸਮਾਂ_ਬਿੰਦੂ_Type ਸ਼ੁਰੂ_ਸਮਾਂ) ਅਚਲ {
    ਵਾਪਸ (ਸਮਾਂ_ਬਿੰਦੂ_cast<microseconds>(ਸ਼ੁਰੂ) -
            ਸਮਾਂ_ਬਿੰਦੂ_cast<microseconds>(ਸ਼ੁਰੂ_ਸਮਾਂ))
        .ਗਿਣਤੀ_2();
  }

  ClockType::rep get_Flame_ਗ੍ਰਾਫ_Dur_Us() ਅਚਲ {
    ਵਾਪਸ (ਸਮਾਂ_ਬਿੰਦੂ_cast<microseconds>(ਸਿਰਾ_2) -
            ਸਮਾਂ_ਬਿੰਦੂ_cast<microseconds>(ਸ਼ੁਰੂ))
        .ਗਿਣਤੀ_2();
  }
};

// Represents a currently open (in-progress) time trace entry. InstantEvents
// that happen during an open event are associated with the duration of this
// parent event and they are dropped if parent duration is shorter than
// the granularity.
ਬਣਤਰ ਵਿੱਚ_ਤਰੱਕੀ_ਇੰਦਰਾਜ {
  ਸਮਾਂ_Trace_Profiler_ਇੰਦਰਾਜ Event;
  std::ਸਦਿਸ਼<ਸਮਾਂ_Trace_Profiler_ਇੰਦਰਾਜ> InstantEvents;

  ਵਿੱਚ_ਤਰੱਕੀ_ਇੰਦਰਾਜ(ਸਮਾਂ_ਬਿੰਦੂ_Type S, ਸਮਾਂ_ਬਿੰਦੂ_Type E, std::string N,
                  std::string Dt, ਸਮਾਂ_Trace_Event_Type Et)
      : Event(std::move(S), std::move(E), std::move(N), std::move(Dt), Et),
        InstantEvents() {}

  ਵਿੱਚ_ਤਰੱਕੀ_ਇੰਦਰਾਜ(ਸਮਾਂ_ਬਿੰਦੂ_Type S, ਸਮਾਂ_ਬਿੰਦੂ_Type E, std::string N,
                  ਸਮਾਂ_Trace_Metadata Mt, ਸਮਾਂ_Trace_Event_Type Et)
      : Event(std::move(S), std::move(E), std::move(N), std::move(Mt), Et),
        InstantEvents() {}
};

ਬਣਤਰ llvm::ਸਮਾਂ_Trace_Profiler {
  ਸਮਾਂ_Trace_Profiler(
      ਬਿਨਾਨਿਸ਼ਾਨੀ ਸਮਾਂ_Trace_Granularity = 0, StringRef ProcName = "",
      ਤਾਰਕਿਕ ਸਮਾਂ_Trace_Verbose = ਝੂਠ,
      ਸਮਾਂ_Trace_Compression ਸਮਾਂ_Trace_Compress = ਸਮਾਂ_Trace_Compression::Infer)
      : Beginning_ਦਾ_ਸਮਾਂ(system_clock::now()), ਸ਼ੁਰੂ_ਸਮਾਂ(ClockType::now()),
        ProcName(ProcName), Pid(sys::Process::getProcessId()),
        Tid(llvm::get_threadid()), ਸਮਾਂ_Trace_Granularity(ਸਮਾਂ_Trace_Granularity),
        ਸਮਾਂ_Trace_Verbose(ਸਮਾਂ_Trace_Verbose),
        ਸਮਾਂ_Trace_Compress(ਸਮਾਂ_Trace_Compress) {
    llvm::get_thread_name(ThreadName);
  }

  ਸਮਾਂ_Trace_Profiler_ਇੰਦਰਾਜ *
  begin(std::string Name, llvm::function_ref<std::string()> Detail,
        ਸਮਾਂ_Trace_Event_Type EventType = ਸਮਾਂ_Trace_Event_Type::CompleteEvent) {
    ਪ੍ਰਤਿਜ੍ਞਾ(EventType != ਸਮਾਂ_Trace_Event_Type::InstantEvent &&
           "Instant Events don't have begin and end.");
    ਢੇਰ.emplace_ਪਿੱਠ(std::make_unique<ਵਿੱਚ_ਤਰੱਕੀ_ਇੰਦਰਾਜ>(
        ClockType::now(), ਸਮਾਂ_ਬਿੰਦੂ_Type(), std::move(Name), Detail(),
        EventType));
    ਵਾਪਸ &ਢੇਰ.ਪਿੱਠ()->Event;
  }

  ਸਮਾਂ_Trace_Profiler_ਇੰਦਰਾਜ *
  begin(std::string Name, llvm::function_ref<ਸਮਾਂ_Trace_Metadata()> Metadata,
        ਸਮਾਂ_Trace_Event_Type EventType = ਸਮਾਂ_Trace_Event_Type::CompleteEvent) {
    ਪ੍ਰਤਿਜ੍ਞਾ(EventType != ਸਮਾਂ_Trace_Event_Type::InstantEvent &&
           "Instant Events don't have begin and end.");
    ਢੇਰ.emplace_ਪਿੱਠ(std::make_unique<ਵਿੱਚ_ਤਰੱਕੀ_ਇੰਦਰਾਜ>(
        ClockType::now(), ਸਮਾਂ_ਬਿੰਦੂ_Type(), std::move(Name), Metadata(),
        EventType));
    ਵਾਪਸ &ਢੇਰ.ਪਿੱਠ()->Event;
  }

  ਖਾਲੀ ਪਾਓ(std::string Name, llvm::function_ref<std::string()> Detail) {
    ਜੇ (ਢੇਰ.ਖਾਲੀ_ਥਾਵਾਂ())
      ਵਾਪਸ;

    ਢੇਰ.ਪਿੱਠ()->InstantEvents.emplace_ਪਿੱਠ(ਸਮਾਂ_Trace_Profiler_ਇੰਦਰਾਜ(
        ClockType::now(), ਸਮਾਂ_ਬਿੰਦੂ_Type(), std::move(Name), Detail(),
        ਸਮਾਂ_Trace_Event_Type::InstantEvent));
  }

  ਖਾਲੀ ਸਿਰਾ() {
    ਪ੍ਰਤਿਜ੍ਞਾ(!ਢੇਰ.ਖਾਲੀ_ਥਾਵਾਂ() && "Must call begin() first");
    ਸਿਰਾ(ਢੇਰ.ਪਿੱਠ()->Event);
  }

  ਖਾਲੀ ਸਿਰਾ(ਸਮਾਂ_Trace_Profiler_ਇੰਦਰਾਜ &E) {
    ਪ੍ਰਤਿਜ੍ਞਾ(!ਢੇਰ.ਖਾਲੀ_ਥਾਵਾਂ() && "Must call begin() first");
    E.ਸਿਰਾ_2 = ClockType::now();

    // Calculate duration at full precision for overall counts.
    DurationType Duration = E.ਸਿਰਾ_2 - E.ਸ਼ੁਰੂ;

    ਅਚਲ ਸਵੈਚਾਲਿਤ *Iter =
        llvm::ਲੱਭੋ_if(ਢੇਰ, [&](ਅਚਲ std::unique_ptr<ਵਿੱਚ_ਤਰੱਕੀ_ਇੰਦਰਾਜ> &Val) {
          ਵਾਪਸ &Val->Event == &E;
        });
    ਪ੍ਰਤਿਜ੍ਞਾ(Iter != ਢੇਰ.ਸਿਰਾ() && "Event not in the Stack");

    // Only include sections longer or equal to TimeTraceGranularity msec.
    ਜੇ (duration_cast<microseconds>(Duration).ਗਿਣਤੀ_2() >= ਸਮਾਂ_Trace_Granularity) {
      Entries.emplace_ਪਿੱਠ(E);
      ਲਈ (ਸਵੈਚਾਲਿਤ &IE : Iter->get()->InstantEvents) {
        Entries.emplace_ਪਿੱਠ(IE);
      }
    }

    // Track total time taken by each "name", but only the topmost levels of
    // them; e.g. if there's a template instantiation that instantiates other
    // templates from within, we only want to add the topmost one. "topmost"
    // happens to be the ones that don't have any currently open entries above
    // itself.
    ਜੇ (llvm::none_ਦਾ(llvm::drop_begin(llvm::ਉਲਟ(ਢੇਰ)),
                      [&](ਅਚਲ std::unique_ptr<ਵਿੱਚ_ਤਰੱਕੀ_ਇੰਦਰਾਜ> &Val) {
                        ਵਾਪਸ Val->Event.Name == E.Name;
                      })) {
      ਸਵੈਚਾਲਿਤ &ਗਿਣਤੀ_And_ਕੁੱਲ = ਗਿਣਤੀ_And_ਕੁੱਲ_ਪ੍ਰਤੀ_Name[E.Name];
      ਗਿਣਤੀ_And_ਕੁੱਲ.ਪਹਿਲਾ_2++;
      ਗਿਣਤੀ_And_ਕੁੱਲ.ਦੂਜਾ_2 += Duration;
    };

    ਢੇਰ.erase(Iter);
  }

  // Write events from this TimeTraceProfilerInstance and
  // ThreadTimeTraceProfilerInstances.
  ਖਾਲੀ ਲਿਖ(raw_ostream &OS) {
    // Acquire Mutex as reading ThreadTimeTraceProfilerInstances.
    ਸਵੈਚਾਲਿਤ &Instances = get_ਸਮਾਂ_Trace_Profiler_Instances();
    std::ਤਾਲਾ_guard<std::mutex> ਤਾਲਾ_2(Instances.ਤਾਲਾ_2);
    ਪ੍ਰਤਿਜ੍ਞਾ(ਢੇਰ.ਖਾਲੀ_ਥਾਵਾਂ() &&
           "All profiler sections should be ended when calling write");
    ਪ੍ਰਤਿਜ੍ਞਾ(llvm::all_ਦਾ(Instances.List,
                        [](ਅਚਲ ਸਵੈਚਾਲਿਤ &TTP) { ਵਾਪਸ TTP->ਢੇਰ.ਖਾਲੀ_ਥਾਵਾਂ(); }) &&
           "All profiler sections should be ended when calling write");

    json::OStream J(OS);
    J.objectBegin();
    J.attributeBegin("traceEvents");
    J.arrayBegin();

    // Emit all events for the main flame graph.
    ਸਵੈਚਾਲਿਤ writeEvent = [&](ਅਚਲ ਸਵੈਚਾਲਿਤ &E, uint64_t Tid) {
      ਸਵੈਚਾਲਿਤ ਸ਼ੁਰੂ_Us = E.get_Flame_ਗ੍ਰਾਫ_ਸ਼ੁਰੂ_Us(ਸ਼ੁਰੂ_ਸਮਾਂ);
      ਸਵੈਚਾਲਿਤ DurUs = E.get_Flame_ਗ੍ਰਾਫ_Dur_Us();

      J.object([&] {
        J.attribute("pid", Pid);
        J.attribute("tid", int64_t(Tid));
        J.attribute("ts", ਸ਼ੁਰੂ_Us);
        ਜੇ (E.EventType == ਸਮਾਂ_Trace_Event_Type::AsyncEvent) {
          J.attribute("cat", E.Name);
          J.attribute("ph", "b");
          J.attribute("id", 0);
        } ਨਹੀਂਤਾਂ ਜੇ (E.EventType == ਸਮਾਂ_Trace_Event_Type::CompleteEvent) {
          J.attribute("ph", "X");
          J.attribute("dur", DurUs);
        } ਨਹੀਂਤਾਂ { // instant event
          ਪ੍ਰਤਿਜ੍ਞਾ(E.EventType == ਸਮਾਂ_Trace_Event_Type::InstantEvent &&
                 "InstantEvent expected");
          J.attribute("ph", "i");
        }
        J.attribute("name", E.Name);
        ਜੇ (!E.Metadata.is_ਖਾਲੀ_ਥਾਵਾਂ()) {
          J.attributeObject("args", [&] {
            ਜੇ (!E.Metadata.Detail.ਖਾਲੀ_ਥਾਵਾਂ())
              J.attribute("detail", E.Metadata.Detail);
            ਜੇ (!E.Metadata.File.ਖਾਲੀ_ਥਾਵਾਂ())
              J.attribute("file", E.Metadata.File);
            ਜੇ (E.Metadata.ਸਤਰ > 0)
              J.attribute("line", E.Metadata.ਸਤਰ);
          });
        }
      });

      ਜੇ (E.EventType == ਸਮਾਂ_Trace_Event_Type::AsyncEvent) {
        J.object([&] {
          J.attribute("pid", Pid);
          J.attribute("tid", int64_t(Tid));
          J.attribute("ts", ਸ਼ੁਰੂ_Us + DurUs);
          J.attribute("cat", E.Name);
          J.attribute("ph", "e");
          J.attribute("id", 0);
          J.attribute("name", E.Name);
        });
      }
    };
    ਲਈ (ਅਚਲ ਸਮਾਂ_Trace_Profiler_ਇੰਦਰਾਜ &E : Entries)
      writeEvent(E, ਇਹ->Tid);
    ਲਈ (ਅਚਲ ਸਮਾਂ_Trace_Profiler *TTP : Instances.List)
      ਲਈ (ਅਚਲ ਸਮਾਂ_Trace_Profiler_ਇੰਦਰਾਜ &E : TTP->Entries)
        writeEvent(E, TTP->Tid);

    // Emit totals by section name as additional "thread" events, sorted from
    // longest one.
    // Find highest used thread id.
    uint64_t MaxTid = ਇਹ->Tid;
    ਲਈ (ਅਚਲ ਸਮਾਂ_Trace_Profiler *TTP : Instances.List)
      MaxTid = std::max(MaxTid, TTP->Tid);

    // Combine all CountAndTotalPerName from threads into one.
    StringMap<ਗਿਣਤੀ_And_Duration_Type> All_ਗਿਣਤੀ_And_ਕੁੱਲ_ਪ੍ਰਤੀ_Name;
    ਸਵੈਚਾਲਿਤ combineStat = [&](ਅਚਲ ਸਵੈਚਾਲਿਤ &Stat) {
      StringRef ਕੁੰਜੀ = Stat.get_ਕੁੰਜੀ();
      ਸਵੈਚਾਲਿਤ ਮੁੱਲ = Stat.get_ਮੁੱਲ();
      ਸਵੈਚਾਲਿਤ &ਗਿਣਤੀ_And_ਕੁੱਲ = All_ਗਿਣਤੀ_And_ਕੁੱਲ_ਪ੍ਰਤੀ_Name[ਕੁੰਜੀ];
      ਗਿਣਤੀ_And_ਕੁੱਲ.ਪਹਿਲਾ_2 += ਮੁੱਲ.ਪਹਿਲਾ_2;
      ਗਿਣਤੀ_And_ਕੁੱਲ.ਦੂਜਾ_2 += ਮੁੱਲ.ਦੂਜਾ_2;
    };
    ਲਈ (ਅਚਲ ਸਵੈਚਾਲਿਤ &Stat : ਗਿਣਤੀ_And_ਕੁੱਲ_ਪ੍ਰਤੀ_Name)
      combineStat(Stat);
    ਲਈ (ਅਚਲ ਸਮਾਂ_Trace_Profiler *TTP : Instances.List)
      ਲਈ (ਅਚਲ ਸਵੈਚਾਲਿਤ &Stat : TTP->ਗਿਣਤੀ_And_ਕੁੱਲ_ਪ੍ਰਤੀ_Name)
        combineStat(Stat);

    std::ਸਦਿਸ਼<Name_And_ਗਿਣਤੀ_And_Duration_Type> SortedTotals;
    SortedTotals.reserve(All_ਗਿਣਤੀ_And_ਕੁੱਲ_ਪ੍ਰਤੀ_Name.ਮਾਪ_2());
    ਲਈ (ਅਚਲ ਸਵੈਚਾਲਿਤ &ਕੁੱਲ : All_ਗਿਣਤੀ_And_ਕੁੱਲ_ਪ੍ਰਤੀ_Name)
      SortedTotals.emplace_ਪਿੱਠ(std::string(ਕੁੱਲ.get_ਕੁੰਜੀ()), ਕੁੱਲ.get_ਮੁੱਲ());

    llvm::ਛਾਂਟੋ(SortedTotals, [](ਅਚਲ Name_And_ਗਿਣਤੀ_And_Duration_Type &A,
                                ਅਚਲ Name_And_ਗਿਣਤੀ_And_Duration_Type &B) {
      ਵਾਪਸ A.ਦੂਜਾ_2.ਦੂਜਾ_2 > B.ਦੂਜਾ_2.ਦੂਜਾ_2;
    });

    // Report totals on separate threads of tracing file.
    uint64_t ਕੁੱਲ_Tid = MaxTid + 1;
    ਲਈ (ਅਚਲ Name_And_ਗਿਣਤੀ_And_Duration_Type &ਕੁੱਲ : SortedTotals) {
      ਸਵੈਚਾਲਿਤ DurUs = duration_cast<microseconds>(ਕੁੱਲ.ਦੂਜਾ_2.ਦੂਜਾ_2).ਗਿਣਤੀ_2();
      ਸਵੈਚਾਲਿਤ ਗਿਣਤੀ_3 = All_ਗਿਣਤੀ_And_ਕੁੱਲ_ਪ੍ਰਤੀ_Name[ਕੁੱਲ.ਪਹਿਲਾ_2].ਪਹਿਲਾ_2;

      J.object([&] {
        J.attribute("pid", Pid);
        J.attribute("tid", int64_t(ਕੁੱਲ_Tid));
        J.attribute("ph", "X");
        J.attribute("ts", 0);
        J.attribute("dur", DurUs);
        J.attribute("name", "Total " + ਕੁੱਲ.ਪਹਿਲਾ_2);
        J.attributeObject("args", [&] {
          J.attribute("count", int64_t(ਗਿਣਤੀ_3));
          J.attribute("avg ms", int64_t(DurUs / ਗਿਣਤੀ_3 / 1000));
        });
      });

      ++ਕੁੱਲ_Tid;
    }

    ਸਵੈਚਾਲਿਤ writeMetadataEvent = [&](ਅਚਲ ਅੱਖਰ *Name, uint64_t Tid,
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
    ਲਈ (ਅਚਲ ਸਮਾਂ_Trace_Profiler *TTP : Instances.List)
      writeMetadataEvent("thread_name", TTP->Tid, TTP->ThreadName);

    J.array_ਸਿਰਾ();
    J.attribute_ਸਿਰਾ();

    // Emit the absolute time when this TimeProfiler started.
    // This can be used to combine the profiling data from
    // multiple processes and preserve actual time intervals.
    J.attribute("beginningOfTime",
                ਸਮਾਂ_ਬਿੰਦੂ_cast<microseconds>(Beginning_ਦਾ_ਸਮਾਂ)
                    .ਸਮਾਂ_since_epoch()
                    .ਗਿਣਤੀ_2());

    J.object_ਸਿਰਾ();
  }

  Small_ਸਦਿਸ਼<std::unique_ptr<ਵਿੱਚ_ਤਰੱਕੀ_ਇੰਦਰਾਜ>, 16> ਢੇਰ;
  Small_ਸਦਿਸ਼<ਸਮਾਂ_Trace_Profiler_ਇੰਦਰਾਜ, 128> Entries;
  StringMap<ਗਿਣਤੀ_And_Duration_Type> ਗਿਣਤੀ_And_ਕੁੱਲ_ਪ੍ਰਤੀ_Name;
  // System clock time when the session was begun.
  ਅਚਲ ਸਮਾਂ_ਬਿੰਦੂ_2<system_clock> Beginning_ਦਾ_ਸਮਾਂ;
  // Profiling clock time when the session was begun.
  ਅਚਲ ਸਮਾਂ_ਬਿੰਦੂ_Type ਸ਼ੁਰੂ_ਸਮਾਂ;
  ਅਚਲ std::string ProcName;
  ਅਚਲ sys::Process::Pid Pid;
  SmallString<0> ThreadName;
  ਅਚਲ uint64_t Tid;

  // Minimum time granularity (in microseconds)
  ਅਚਲ ਬਿਨਾਨਿਸ਼ਾਨੀ ਸਮਾਂ_Trace_Granularity;

  // Make time trace capture verbose event details (e.g. source filenames). This
  // can increase the size of the output by 2-3 times.
  ਅਚਲ ਤਾਰਕਿਕ ਸਮਾਂ_Trace_Verbose;

  // Compression mode for output.
  ਸਮਾਂ_Trace_Compression ਸਮਾਂ_Trace_Compress;
};

ਤਾਰਕਿਕ llvm::is_ਸਮਾਂ_Trace_Verbose() {
  ਵਾਪਸ get_ਸਮਾਂ_Trace_Profiler_Instance() &&
         get_ਸਮਾਂ_Trace_Profiler_Instance()->ਸਮਾਂ_Trace_Verbose;
}

ਸਮਾਂ_Trace_Compression llvm::infer_ਸਮਾਂ_Trace_Compression_From_ਰਸਤਾ(StringRef ਰਸਤਾ) {
  StringRef Ext = llvm::sys::ਰਸਤਾ_2::extension(ਰਸਤਾ);
  ਜੇ (Ext.equals_insensitive(".zst") || Ext.equals_insensitive(".zstd"))
    ਵਾਪਸ ਸਮਾਂ_Trace_Compression::Zstd;
  ਵਾਪਸ ਸਮਾਂ_Trace_Compression::None;
}

ਖਾਲੀ llvm::ਸਮਾਂ_Trace_Profiler_Initialize(ਬਿਨਾਨਿਸ਼ਾਨੀ ਸਮਾਂ_Trace_Granularity,
                                       StringRef ProcName,
                                       ਤਾਰਕਿਕ ਸਮਾਂ_Trace_Verbose,
                                       ਸਮਾਂ_Trace_Compression ਸਮਾਂ_Trace_Compress) {
  ਪ੍ਰਤਿਜ੍ਞਾ(ਸਮਾਂ_Trace_Profiler_Instance == ਖਾਲੀ_ਸੂਚਕ &&
         "Profiler should not be initialized");
  ਸਮਾਂ_Trace_Profiler_Instance = ਨਵਾਂ ਸਮਾਂ_Trace_Profiler(
      ਸਮਾਂ_Trace_Granularity, llvm::sys::ਰਸਤਾ_2::filename(ProcName),
      ਸਮਾਂ_Trace_Verbose, ਸਮਾਂ_Trace_Compress);
}

// Removes all TimeTraceProfilerInstances.
// Called from main thread.
ਖਾਲੀ llvm::ਸਮਾਂ_Trace_Profiler_Cleanup() {
  ਮਿਟਾਓ ਸਮਾਂ_Trace_Profiler_Instance;
  ਸਮਾਂ_Trace_Profiler_Instance = ਖਾਲੀ_ਸੂਚਕ;

  ਸਵੈਚਾਲਿਤ &Instances = get_ਸਮਾਂ_Trace_Profiler_Instances();
  std::ਤਾਲਾ_guard<std::mutex> ਤਾਲਾ_2(Instances.ਤਾਲਾ_2);
  ਲਈ (ਸਵੈਚਾਲਿਤ *TTP : Instances.List)
    ਮਿਟਾਓ TTP;
  Instances.List.clear();
}

// Finish TimeTraceProfilerInstance on a worker thread.
// This doesn't remove the instance, just moves the pointer to global vector.
ਖਾਲੀ llvm::ਸਮਾਂ_Trace_Profiler_ਅੰਤ_Thread() {
  ਸਵੈਚਾਲਿਤ &Instances = get_ਸਮਾਂ_Trace_Profiler_Instances();
  std::ਤਾਲਾ_guard<std::mutex> ਤਾਲਾ_2(Instances.ਤਾਲਾ_2);
  Instances.List.ਧੱਕੋ_ਪਿੱਠ(ਸਮਾਂ_Trace_Profiler_Instance);
  ਸਮਾਂ_Trace_Profiler_Instance = ਖਾਲੀ_ਸੂਚਕ;
}

ਖਾਲੀ llvm::ਸਮਾਂ_Trace_Profiler_Write(raw_pwrite_ਧਾਰਾ &OS) {
  ਪ੍ਰਤਿਜ੍ਞਾ(ਸਮਾਂ_Trace_Profiler_Instance != ਖਾਲੀ_ਸੂਚਕ &&
         "Profiler object can't be null");
  ਸਮਾਂ_Trace_Compression CompressType =
      ਸਮਾਂ_Trace_Profiler_Instance->ਸਮਾਂ_Trace_Compress;
  ਜੇ (CompressType != ਸਮਾਂ_Trace_Compression::Zstd) {
    ਸਮਾਂ_Trace_Profiler_Instance->ਲਿਖ(OS);
    ਵਾਪਸ;
  }

  compression::Format F = compression::Format::Zstd;
  ਜੇ (ਅਚਲ ਅੱਖਰ *Reason = compression::getReasonIfUnsupported(F))
    report_fatal_error(Reason);

  Small_ਸਦਿਸ਼<ਅੱਖਰ, 0> Uncompressed;
  {
    raw_svector_ostream UncompressedOS(Uncompressed);
    ਸਮਾਂ_Trace_Profiler_Instance->ਲਿਖ(UncompressedOS);
  }
  Small_ਸਦਿਸ਼<uint8_t, 0> Compressed;
  compression::compress(
      compression::Params(F),
      ArrayRef(reinterpret_cast<ਅਚਲ uint8_t *>(Uncompressed.ਅੰਕੜੇ()),
               Uncompressed.ਮਾਪ_2()),
      Compressed);
  OS.ਲਿਖ(reinterpret_cast<ਅਚਲ ਅੱਖਰ *>(Compressed.ਅੰਕੜੇ()),
           Compressed.ਮਾਪ_2());
}

Error llvm::ਸਮਾਂ_Trace_Profiler_Write(StringRef PreferredFileName,
                                   StringRef FallbackFileName) {
  ਪ੍ਰਤਿਜ੍ਞਾ(ਸਮਾਂ_Trace_Profiler_Instance != ਖਾਲੀ_ਸੂਚਕ &&
         "Profiler object can't be null");

  ਸਮਾਂ_Trace_Compression CompressType =
      ਸਮਾਂ_Trace_Profiler_Instance->ਸਮਾਂ_Trace_Compress;
  std::string ਰਸਤਾ = PreferredFileName.str();
  ਜੇ (ਰਸਤਾ.ਖਾਲੀ_ਥਾਵਾਂ()) {
    ਰਸਤਾ = FallbackFileName == "-" ? "out" : FallbackFileName.str();
    ਰਸਤਾ += ਸਮਾਂ_Trace_File_Extension;
    ਜੇ (CompressType == ਸਮਾਂ_Trace_Compression::Zstd)
      ਰਸਤਾ += ".zst";
  } ਨਹੀਂਤਾਂ ਜੇ (CompressType == ਸਮਾਂ_Trace_Compression::Infer) {
    CompressType = infer_ਸਮਾਂ_Trace_Compression_From_ਰਸਤਾ(ਰਸਤਾ);
  }

  ਜੇ (CompressType == ਸਮਾਂ_Trace_Compression::Zstd) {
    ਜੇ (ਅਚਲ ਅੱਖਰ *Reason =
            compression::getReasonIfUnsupported(compression::Format::Zstd))
      ਵਾਪਸ createStringError(inconvertible_Error_ਕੋਡ(), Reason);
  }

  std::error_ਕੋਡ EC;
  sys::fs::OpenFlags Flags = CompressType == ਸਮਾਂ_Trace_Compression::Zstd
                                 ? sys::fs::ਦਾ_None
                                 : sys::fs::ਦਾ_ਪਾਠ_With_CRLF;
  raw_fd_ostream OS(ਰਸਤਾ, EC, Flags);
  ਜੇ (EC)
    ਵਾਪਸ createStringError(EC, "Could not open " + ਰਸਤਾ);

  ਸਮਾਂ_Trace_Compression SavedCompress =
      ਸਮਾਂ_Trace_Profiler_Instance->ਸਮਾਂ_Trace_Compress;
  ਸਮਾਂ_Trace_Profiler_Instance->ਸਮਾਂ_Trace_Compress = CompressType;
  ਸਮਾਂ_Trace_Profiler_Write(OS);
  ਸਮਾਂ_Trace_Profiler_Instance->ਸਮਾਂ_Trace_Compress = SavedCompress;
  ਵਾਪਸ Error::success();
}

ਸਮਾਂ_Trace_Profiler_ਇੰਦਰਾਜ *llvm::ਸਮਾਂ_Trace_Profiler_Begin(StringRef Name,
                                                     StringRef Detail) {
  ਜੇ (ਸਮਾਂ_Trace_Profiler_Instance != ਖਾਲੀ_ਸੂਚਕ)
    ਵਾਪਸ ਸਮਾਂ_Trace_Profiler_Instance->begin(
        std::string(Name), [&]() { ਵਾਪਸ std::string(Detail); },
        ਸਮਾਂ_Trace_Event_Type::CompleteEvent);
  ਵਾਪਸ ਖਾਲੀ_ਸੂਚਕ;
}

ਸਮਾਂ_Trace_Profiler_ਇੰਦਰਾਜ *
llvm::ਸਮਾਂ_Trace_Profiler_Begin(StringRef Name,
                             llvm::function_ref<std::string()> Detail) {
  ਜੇ (ਸਮਾਂ_Trace_Profiler_Instance != ਖਾਲੀ_ਸੂਚਕ)
    ਵਾਪਸ ਸਮਾਂ_Trace_Profiler_Instance->begin(std::string(Name), Detail,
                                            ਸਮਾਂ_Trace_Event_Type::CompleteEvent);
  ਵਾਪਸ ਖਾਲੀ_ਸੂਚਕ;
}

ਸਮਾਂ_Trace_Profiler_ਇੰਦਰਾਜ *
llvm::ਸਮਾਂ_Trace_Profiler_Begin(StringRef Name,
                             llvm::function_ref<ਸਮਾਂ_Trace_Metadata()> Metadata) {
  ਜੇ (ਸਮਾਂ_Trace_Profiler_Instance != ਖਾਲੀ_ਸੂਚਕ)
    ਵਾਪਸ ਸਮਾਂ_Trace_Profiler_Instance->begin(std::string(Name), Metadata,
                                            ਸਮਾਂ_Trace_Event_Type::CompleteEvent);
  ਵਾਪਸ ਖਾਲੀ_ਸੂਚਕ;
}

ਸਮਾਂ_Trace_Profiler_ਇੰਦਰਾਜ *llvm::ਸਮਾਂ_Trace_Async_Profiler_Begin(StringRef Name,
                                                          StringRef Detail) {
  ਜੇ (ਸਮਾਂ_Trace_Profiler_Instance != ਖਾਲੀ_ਸੂਚਕ)
    ਵਾਪਸ ਸਮਾਂ_Trace_Profiler_Instance->begin(
        std::string(Name), [&]() { ਵਾਪਸ std::string(Detail); },
        ਸਮਾਂ_Trace_Event_Type::AsyncEvent);
  ਵਾਪਸ ਖਾਲੀ_ਸੂਚਕ;
}

ਖਾਲੀ llvm::ਸਮਾਂ_Trace_ਜੋੜੋ_Instant_Event(StringRef Name,
                                    llvm::function_ref<std::string()> Detail) {
  ਜੇ (ਸਮਾਂ_Trace_Profiler_Instance != ਖਾਲੀ_ਸੂਚਕ)
    ਸਮਾਂ_Trace_Profiler_Instance->ਪਾਓ(std::string(Name), Detail);
}

ਖਾਲੀ llvm::ਸਮਾਂ_Trace_Profiler_ਸਿਰਾ() {
  ਜੇ (ਸਮਾਂ_Trace_Profiler_Instance != ਖਾਲੀ_ਸੂਚਕ)
    ਸਮਾਂ_Trace_Profiler_Instance->ਸਿਰਾ();
}

ਖਾਲੀ llvm::ਸਮਾਂ_Trace_Profiler_ਸਿਰਾ(ਸਮਾਂ_Trace_Profiler_ਇੰਦਰਾਜ *E) {
  ਜੇ (ਸਮਾਂ_Trace_Profiler_Instance != ਖਾਲੀ_ਸੂਚਕ)
    ਸਮਾਂ_Trace_Profiler_Instance->ਸਿਰਾ(*E);
}
