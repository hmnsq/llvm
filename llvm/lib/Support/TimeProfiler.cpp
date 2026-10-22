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

#ضمن "llvm/Support/TimeProfiler.h"
#ضمن "llvm/ADT/STLExtras.h"
#ضمن "llvm/ADT/STLFunctionalExtras.h"
#ضمن "llvm/ADT/SmallVector.h"
#ضمن "llvm/ADT/StringMap.h"
#ضمن "llvm/Support/Compression.h"
#ضمن "llvm/Support/JSON.h"
#ضمن "llvm/Support/Path.h"
#ضمن "llvm/Support/Process.h"
#ضمن "llvm/Support/Threading.h"
#ضمن <algorithm>
#ضمن <cassert>
#ضمن <chrono>
#ضمن <memory>
#ضمن <mutex>
#ضمن <string>
#ضمن <vector>

استعمال نطاق llvm;

نطاق {

استعمال std::chrono::duration;
استعمال std::chrono::duration_cast;
استعمال std::chrono::microseconds;
استعمال std::chrono::steady_clock;
استعمال std::chrono::system_clock;
استعمال std::chrono::وقت_نقطو_2;
استعمال std::chrono::وقت_نقطو_cast;

بنية وقت_Trace_Profiler_Instances {
  std::mutex تالو_2;
  std::ويڪٽر<وقت_Trace_Profiler *> List;
};

وقت_Trace_Profiler_Instances &get_وقت_Trace_Profiler_Instances() {
  ساكن وقت_Trace_Profiler_Instances Instances;
  أرجع Instances;
}

ساكن ثابت حرف وقت_Trace_File_Extension[] = ".time-trace.json";

} // anonymous namespace

// Per Thread instance
ساكن LLVM_THREAD_LOCAL وقت_Trace_Profiler *وقت_Trace_Profiler_Instance = خالي_اشارو;

وقت_Trace_Profiler *llvm::get_وقت_Trace_Profiler_Instance() {
  أرجع وقت_Trace_Profiler_Instance;
}

نطاق {

استعمال ClockType = steady_clock;
استعمال وقت_نقطو_Type = وقت_نقطو_2<ClockType>;
استعمال DurationType = duration<ClockType::rep, ClockType::period>;
استعمال ڳڻپ_And_Duration_Type = std::pair<size_t, DurationType>;
استعمال Name_And_ڳڻپ_And_Duration_Type =
    std::pair<std::string, ڳڻپ_And_Duration_Type>;

} // anonymous namespace

/// Represents an open or completed time section entry to be captured.
بنية llvm::وقت_Trace_Profiler_داخلا {
  ثابت وقت_نقطو_Type شروعات;
  وقت_نقطو_Type پڇاڙي_2;
  ثابت std::string Name;
  وقت_Trace_Metadata Metadata;

  ثابت وقت_Trace_Event_Type EventType = وقت_Trace_Event_Type::CompleteEvent;
  وقت_Trace_Profiler_داخلا(وقت_نقطو_Type &&S, وقت_نقطو_Type &&E, std::string &&N,
                         std::string &&Dt, وقت_Trace_Event_Type Et)
      : شروعات(std::move(S)), پڇاڙي_2(std::move(E)), Name(std::move(N)), Metadata(),
        EventType(Et) {
    Metadata.Detail = std::move(Dt);
  }

  وقت_Trace_Profiler_داخلا(وقت_نقطو_Type &&S, وقت_نقطو_Type &&E, std::string &&N,
                         وقت_Trace_Metadata &&Mt, وقت_Trace_Event_Type Et)
      : شروعات(std::move(S)), پڇاڙي_2(std::move(E)), Name(std::move(N)),
        Metadata(std::move(Mt)), EventType(Et) {}

  // Calculate timings for FlameGraph. Cast time points to microsecond precision
  // rather than casting duration. This avoids truncation issues causing inner
  // scopes overruning outer scopes.
  ClockType::rep get_Flame_گراف_شروعات_Us(وقت_نقطو_Type شروعات_وقت) ثابت {
    أرجع (وقت_نقطو_cast<microseconds>(شروعات) -
            وقت_نقطو_cast<microseconds>(شروعات_وقت))
        .ڳڻپ();
  }

  ClockType::rep get_Flame_گراف_Dur_Us() ثابت {
    أرجع (وقت_نقطو_cast<microseconds>(پڇاڙي_2) -
            وقت_نقطو_cast<microseconds>(شروعات))
        .ڳڻپ();
  }
};

// Represents a currently open (in-progress) time trace entry. InstantEvents
// that happen during an open event are associated with the duration of this
// parent event and they are dropped if parent duration is shorter than
// the granularity.
بنية اندريون_ترقي_داخلا {
  وقت_Trace_Profiler_داخلا Event;
  std::ويڪٽر<وقت_Trace_Profiler_داخلا> InstantEvents;

  اندريون_ترقي_داخلا(وقت_نقطو_Type S, وقت_نقطو_Type E, std::string N,
                  std::string Dt, وقت_Trace_Event_Type Et)
      : Event(std::move(S), std::move(E), std::move(N), std::move(Dt), Et),
        InstantEvents() {}

  اندريون_ترقي_داخلا(وقت_نقطو_Type S, وقت_نقطو_Type E, std::string N,
                  وقت_Trace_Metadata Mt, وقت_Trace_Event_Type Et)
      : Event(std::move(S), std::move(E), std::move(N), std::move(Mt), Et),
        InstantEvents() {}
};

بنية llvm::وقت_Trace_Profiler {
  وقت_Trace_Profiler(
      بنانشان وقت_Trace_Granularity = 0, StringRef ProcName = "",
      منطقي وقت_Trace_Verbose = خطأ,
      وقت_Trace_Compression وقت_Trace_Compress = وقت_Trace_Compression::Infer)
      : Beginning_جو_وقت(system_clock::now()), شروعات_وقت(ClockType::now()),
        ProcName(ProcName), Pid(sys::Process::getProcessId()),
        Tid(llvm::get_threadid()), وقت_Trace_Granularity(وقت_Trace_Granularity),
        وقت_Trace_Verbose(وقت_Trace_Verbose),
        وقت_Trace_Compress(وقت_Trace_Compress) {
    llvm::get_thread_name(ThreadName);
  }

  وقت_Trace_Profiler_داخلا *
  begin(std::string Name, llvm::function_ref<std::string()> Detail,
        وقت_Trace_Event_Type EventType = وقت_Trace_Event_Type::CompleteEvent) {
    أكد(EventType != وقت_Trace_Event_Type::InstantEvent &&
           "Instant Events don't have begin and end.");
    ڍير.emplace_پٺ(std::make_unique<اندريون_ترقي_داخلا>(
        ClockType::now(), وقت_نقطو_Type(), std::move(Name), Detail(),
        EventType));
    أرجع &ڍير.پٺ()->Event;
  }

  وقت_Trace_Profiler_داخلا *
  begin(std::string Name, llvm::function_ref<وقت_Trace_Metadata()> Metadata,
        وقت_Trace_Event_Type EventType = وقت_Trace_Event_Type::CompleteEvent) {
    أكد(EventType != وقت_Trace_Event_Type::InstantEvent &&
           "Instant Events don't have begin and end.");
    ڍير.emplace_پٺ(std::make_unique<اندريون_ترقي_داخلا>(
        ClockType::now(), وقت_نقطو_Type(), std::move(Name), Metadata(),
        EventType));
    أرجع &ڍير.پٺ()->Event;
  }

  فراغ وجهو(std::string Name, llvm::function_ref<std::string()> Detail) {
    إذا (ڍير.خالي_جايون())
      أرجع;

    ڍير.پٺ()->InstantEvents.emplace_پٺ(وقت_Trace_Profiler_داخلا(
        ClockType::now(), وقت_نقطو_Type(), std::move(Name), Detail(),
        وقت_Trace_Event_Type::InstantEvent));
  }

  فراغ پڇاڙي() {
    أكد(!ڍير.خالي_جايون() && "Must call begin() first");
    پڇاڙي(ڍير.پٺ()->Event);
  }

  فراغ پڇاڙي(وقت_Trace_Profiler_داخلا &E) {
    أكد(!ڍير.خالي_جايون() && "Must call begin() first");
    E.پڇاڙي_2 = ClockType::now();

    // Calculate duration at full precision for overall counts.
    DurationType Duration = E.پڇاڙي_2 - E.شروعات;

    ثابت خودڪار *Iter =
        llvm::ڳوليو_if(ڍير, [&](ثابت std::unique_ptr<اندريون_ترقي_داخلا> &Val) {
          أرجع &Val->Event == &E;
        });
    أكد(Iter != ڍير.پڇاڙي() && "Event not in the Stack");

    // Only include sections longer or equal to TimeTraceGranularity msec.
    إذا (duration_cast<microseconds>(Duration).ڳڻپ() >= وقت_Trace_Granularity) {
      Entries.emplace_پٺ(E);
      لكل (خودڪار &IE : Iter->get()->InstantEvents) {
        Entries.emplace_پٺ(IE);
      }
    }

    // Track total time taken by each "name", but only the topmost levels of
    // them; e.g. if there's a template instantiation that instantiates other
    // templates from within, we only want to add the topmost one. "topmost"
    // happens to be the ones that don't have any currently open entries above
    // itself.
    إذا (llvm::none_جو(llvm::drop_begin(llvm::ابتو(ڍير)),
                      [&](ثابت std::unique_ptr<اندريون_ترقي_داخلا> &Val) {
                        أرجع Val->Event.Name == E.Name;
                      })) {
      خودڪار &ڳڻپ_And_ڪل = ڳڻپ_And_ڪل_في_Name[E.Name];
      ڳڻپ_And_ڪل.پهريون_2++;
      ڳڻپ_And_ڪل.ٻيو_2 += Duration;
    };

    ڍير.erase(Iter);
  }

  // Write events from this TimeTraceProfilerInstance and
  // ThreadTimeTraceProfilerInstances.
  فراغ اكتب(raw_ostream &OS) {
    // Acquire Mutex as reading ThreadTimeTraceProfilerInstances.
    خودڪار &Instances = get_وقت_Trace_Profiler_Instances();
    std::تالو_guard<std::mutex> تالو_2(Instances.تالو_2);
    أكد(ڍير.خالي_جايون() &&
           "All profiler sections should be ended when calling write");
    أكد(llvm::all_جو(Instances.List,
                        [](ثابت خودڪار &TTP) { أرجع TTP->ڍير.خالي_جايون(); }) &&
           "All profiler sections should be ended when calling write");

    json::OStream J(OS);
    J.objectBegin();
    J.attributeBegin("traceEvents");
    J.arrayBegin();

    // Emit all events for the main flame graph.
    خودڪار writeEvent = [&](ثابت خودڪار &E, uint64_t Tid) {
      خودڪار شروعات_Us = E.get_Flame_گراف_شروعات_Us(شروعات_وقت);
      خودڪار DurUs = E.get_Flame_گراف_Dur_Us();

      J.object([&] {
        J.attribute("pid", Pid);
        J.attribute("tid", int64_t(Tid));
        J.attribute("ts", شروعات_Us);
        إذا (E.EventType == وقت_Trace_Event_Type::AsyncEvent) {
          J.attribute("cat", E.Name);
          J.attribute("ph", "b");
          J.attribute("id", 0);
        } وإلا إذا (E.EventType == وقت_Trace_Event_Type::CompleteEvent) {
          J.attribute("ph", "X");
          J.attribute("dur", DurUs);
        } وإلا { // instant event
          أكد(E.EventType == وقت_Trace_Event_Type::InstantEvent &&
                 "InstantEvent expected");
          J.attribute("ph", "i");
        }
        J.attribute("name", E.Name);
        إذا (!E.Metadata.is_خالي_جايون()) {
          J.attributeObject("args", [&] {
            إذا (!E.Metadata.Detail.خالي_جايون())
              J.attribute("detail", E.Metadata.Detail);
            إذا (!E.Metadata.File.خالي_جايون())
              J.attribute("file", E.Metadata.File);
            إذا (E.Metadata.سٽ > 0)
              J.attribute("line", E.Metadata.سٽ);
          });
        }
      });

      إذا (E.EventType == وقت_Trace_Event_Type::AsyncEvent) {
        J.object([&] {
          J.attribute("pid", Pid);
          J.attribute("tid", int64_t(Tid));
          J.attribute("ts", شروعات_Us + DurUs);
          J.attribute("cat", E.Name);
          J.attribute("ph", "e");
          J.attribute("id", 0);
          J.attribute("name", E.Name);
        });
      }
    };
    لكل (ثابت وقت_Trace_Profiler_داخلا &E : Entries)
      writeEvent(E, هذا->Tid);
    لكل (ثابت وقت_Trace_Profiler *TTP : Instances.List)
      لكل (ثابت وقت_Trace_Profiler_داخلا &E : TTP->Entries)
        writeEvent(E, TTP->Tid);

    // Emit totals by section name as additional "thread" events, sorted from
    // longest one.
    // Find highest used thread id.
    uint64_t MaxTid = هذا->Tid;
    لكل (ثابت وقت_Trace_Profiler *TTP : Instances.List)
      MaxTid = std::max(MaxTid, TTP->Tid);

    // Combine all CountAndTotalPerName from threads into one.
    StringMap<ڳڻپ_And_Duration_Type> All_ڳڻپ_And_ڪل_في_Name;
    خودڪار combineStat = [&](ثابت خودڪار &Stat) {
      StringRef ڪنجي = Stat.get_ڪنجي();
      خودڪار قدر = Stat.get_قدر();
      خودڪار &ڳڻپ_And_ڪل = All_ڳڻپ_And_ڪل_في_Name[ڪنجي];
      ڳڻپ_And_ڪل.پهريون_2 += قدر.پهريون_2;
      ڳڻپ_And_ڪل.ٻيو_2 += قدر.ٻيو_2;
    };
    لكل (ثابت خودڪار &Stat : ڳڻپ_And_ڪل_في_Name)
      combineStat(Stat);
    لكل (ثابت وقت_Trace_Profiler *TTP : Instances.List)
      لكل (ثابت خودڪار &Stat : TTP->ڳڻپ_And_ڪل_في_Name)
        combineStat(Stat);

    std::ويڪٽر<Name_And_ڳڻپ_And_Duration_Type> SortedTotals;
    SortedTotals.reserve(All_ڳڻپ_And_ڪل_في_Name.سائيز_2());
    لكل (ثابت خودڪار &ڪل : All_ڳڻپ_And_ڪل_في_Name)
      SortedTotals.emplace_پٺ(std::string(ڪل.get_ڪنجي()), ڪل.get_قدر());

    llvm::ترتيب_ڏيو(SortedTotals, [](ثابت Name_And_ڳڻپ_And_Duration_Type &A,
                                ثابت Name_And_ڳڻپ_And_Duration_Type &B) {
      أرجع A.ٻيو_2.ٻيو_2 > B.ٻيو_2.ٻيو_2;
    });

    // Report totals on separate threads of tracing file.
    uint64_t ڪل_Tid = MaxTid + 1;
    لكل (ثابت Name_And_ڳڻپ_And_Duration_Type &ڪل : SortedTotals) {
      خودڪار DurUs = duration_cast<microseconds>(ڪل.ٻيو_2.ٻيو_2).ڳڻپ();
      خودڪار ڳڻپ_2 = All_ڳڻپ_And_ڪل_في_Name[ڪل.پهريون_2].پهريون_2;

      J.object([&] {
        J.attribute("pid", Pid);
        J.attribute("tid", int64_t(ڪل_Tid));
        J.attribute("ph", "X");
        J.attribute("ts", 0);
        J.attribute("dur", DurUs);
        J.attribute("name", "Total " + ڪل.پهريون_2);
        J.attributeObject("args", [&] {
          J.attribute("count", int64_t(ڳڻپ_2));
          J.attribute("avg ms", int64_t(DurUs / ڳڻپ_2 / 1000));
        });
      });

      ++ڪل_Tid;
    }

    خودڪار writeMetadataEvent = [&](ثابت حرف *Name, uint64_t Tid,
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
    لكل (ثابت وقت_Trace_Profiler *TTP : Instances.List)
      writeMetadataEvent("thread_name", TTP->Tid, TTP->ThreadName);

    J.array_پڇاڙي();
    J.attribute_پڇاڙي();

    // Emit the absolute time when this TimeProfiler started.
    // This can be used to combine the profiling data from
    // multiple processes and preserve actual time intervals.
    J.attribute("beginningOfTime",
                وقت_نقطو_cast<microseconds>(Beginning_جو_وقت)
                    .وقت_since_epoch()
                    .ڳڻپ());

    J.object_پڇاڙي();
  }

  Small_ويڪٽر<std::unique_ptr<اندريون_ترقي_داخلا>, 16> ڍير;
  Small_ويڪٽر<وقت_Trace_Profiler_داخلا, 128> Entries;
  StringMap<ڳڻپ_And_Duration_Type> ڳڻپ_And_ڪل_في_Name;
  // System clock time when the session was begun.
  ثابت وقت_نقطو_2<system_clock> Beginning_جو_وقت;
  // Profiling clock time when the session was begun.
  ثابت وقت_نقطو_Type شروعات_وقت;
  ثابت std::string ProcName;
  ثابت sys::Process::Pid Pid;
  SmallString<0> ThreadName;
  ثابت uint64_t Tid;

  // Minimum time granularity (in microseconds)
  ثابت بنانشان وقت_Trace_Granularity;

  // Make time trace capture verbose event details (e.g. source filenames). This
  // can increase the size of the output by 2-3 times.
  ثابت منطقي وقت_Trace_Verbose;

  // Compression mode for output.
  وقت_Trace_Compression وقت_Trace_Compress;
};

منطقي llvm::is_وقت_Trace_Verbose() {
  أرجع get_وقت_Trace_Profiler_Instance() &&
         get_وقت_Trace_Profiler_Instance()->وقت_Trace_Verbose;
}

وقت_Trace_Compression llvm::infer_وقت_Trace_Compression_From_رستو(StringRef رستو) {
  StringRef Ext = llvm::sys::رستو_2::extension(رستو);
  إذا (Ext.equals_insensitive(".zst") || Ext.equals_insensitive(".zstd"))
    أرجع وقت_Trace_Compression::Zstd;
  أرجع وقت_Trace_Compression::None;
}

فراغ llvm::وقت_Trace_Profiler_Initialize(بنانشان وقت_Trace_Granularity,
                                       StringRef ProcName,
                                       منطقي وقت_Trace_Verbose,
                                       وقت_Trace_Compression وقت_Trace_Compress) {
  أكد(وقت_Trace_Profiler_Instance == خالي_اشارو &&
         "Profiler should not be initialized");
  وقت_Trace_Profiler_Instance = جديد وقت_Trace_Profiler(
      وقت_Trace_Granularity, llvm::sys::رستو_2::filename(ProcName),
      وقت_Trace_Verbose, وقت_Trace_Compress);
}

// Removes all TimeTraceProfilerInstances.
// Called from main thread.
فراغ llvm::وقت_Trace_Profiler_Cleanup() {
  ميٽيو وقت_Trace_Profiler_Instance;
  وقت_Trace_Profiler_Instance = خالي_اشارو;

  خودڪار &Instances = get_وقت_Trace_Profiler_Instances();
  std::تالو_guard<std::mutex> تالو_2(Instances.تالو_2);
  لكل (خودڪار *TTP : Instances.List)
    ميٽيو TTP;
  Instances.List.clear();
}

// Finish TimeTraceProfilerInstance on a worker thread.
// This doesn't remove the instance, just moves the pointer to global vector.
فراغ llvm::وقت_Trace_Profiler_پڄاڻي_Thread() {
  خودڪار &Instances = get_وقت_Trace_Profiler_Instances();
  std::تالو_guard<std::mutex> تالو_2(Instances.تالو_2);
  Instances.List.ڌڪو_پٺ(وقت_Trace_Profiler_Instance);
  وقت_Trace_Profiler_Instance = خالي_اشارو;
}

فراغ llvm::وقت_Trace_Profiler_Write(raw_pwrite_وهڪرو &OS) {
  أكد(وقت_Trace_Profiler_Instance != خالي_اشارو &&
         "Profiler object can't be null");
  وقت_Trace_Compression CompressType =
      وقت_Trace_Profiler_Instance->وقت_Trace_Compress;
  إذا (CompressType != وقت_Trace_Compression::Zstd) {
    وقت_Trace_Profiler_Instance->اكتب(OS);
    أرجع;
  }

  compression::Format F = compression::Format::Zstd;
  إذا (ثابت حرف *Reason = compression::getReasonIfUnsupported(F))
    report_fatal_error(Reason);

  Small_ويڪٽر<حرف, 0> Uncompressed;
  {
    raw_svector_ostream UncompressedOS(Uncompressed);
    وقت_Trace_Profiler_Instance->اكتب(UncompressedOS);
  }
  Small_ويڪٽر<uint8_t, 0> Compressed;
  compression::compress(
      compression::Params(F),
      ArrayRef(reinterpret_cast<ثابت uint8_t *>(Uncompressed.معلومات()),
               Uncompressed.سائيز_2()),
      Compressed);
  OS.اكتب(reinterpret_cast<ثابت حرف *>(Compressed.معلومات()),
           Compressed.سائيز_2());
}

Error llvm::وقت_Trace_Profiler_Write(StringRef PreferredFileName,
                                   StringRef FallbackFileName) {
  أكد(وقت_Trace_Profiler_Instance != خالي_اشارو &&
         "Profiler object can't be null");

  وقت_Trace_Compression CompressType =
      وقت_Trace_Profiler_Instance->وقت_Trace_Compress;
  std::string رستو = PreferredFileName.str();
  إذا (رستو.خالي_جايون()) {
    رستو = FallbackFileName == "-" ? "out" : FallbackFileName.str();
    رستو += وقت_Trace_File_Extension;
    إذا (CompressType == وقت_Trace_Compression::Zstd)
      رستو += ".zst";
  } وإلا إذا (CompressType == وقت_Trace_Compression::Infer) {
    CompressType = infer_وقت_Trace_Compression_From_رستو(رستو);
  }

  إذا (CompressType == وقت_Trace_Compression::Zstd) {
    إذا (ثابت حرف *Reason =
            compression::getReasonIfUnsupported(compression::Format::Zstd))
      أرجع createStringError(inconvertible_Error_ڪوڊ(), Reason);
  }

  std::error_ڪوڊ EC;
  sys::fs::OpenFlags Flags = CompressType == وقت_Trace_Compression::Zstd
                                 ? sys::fs::جو_None
                                 : sys::fs::جو_متن_With_CRLF;
  raw_fd_ostream OS(رستو, EC, Flags);
  إذا (EC)
    أرجع createStringError(EC, "Could not open " + رستو);

  وقت_Trace_Compression SavedCompress =
      وقت_Trace_Profiler_Instance->وقت_Trace_Compress;
  وقت_Trace_Profiler_Instance->وقت_Trace_Compress = CompressType;
  وقت_Trace_Profiler_Write(OS);
  وقت_Trace_Profiler_Instance->وقت_Trace_Compress = SavedCompress;
  أرجع Error::success();
}

وقت_Trace_Profiler_داخلا *llvm::وقت_Trace_Profiler_Begin(StringRef Name,
                                                     StringRef Detail) {
  إذا (وقت_Trace_Profiler_Instance != خالي_اشارو)
    أرجع وقت_Trace_Profiler_Instance->begin(
        std::string(Name), [&]() { أرجع std::string(Detail); },
        وقت_Trace_Event_Type::CompleteEvent);
  أرجع خالي_اشارو;
}

وقت_Trace_Profiler_داخلا *
llvm::وقت_Trace_Profiler_Begin(StringRef Name,
                             llvm::function_ref<std::string()> Detail) {
  إذا (وقت_Trace_Profiler_Instance != خالي_اشارو)
    أرجع وقت_Trace_Profiler_Instance->begin(std::string(Name), Detail,
                                            وقت_Trace_Event_Type::CompleteEvent);
  أرجع خالي_اشارو;
}

وقت_Trace_Profiler_داخلا *
llvm::وقت_Trace_Profiler_Begin(StringRef Name,
                             llvm::function_ref<وقت_Trace_Metadata()> Metadata) {
  إذا (وقت_Trace_Profiler_Instance != خالي_اشارو)
    أرجع وقت_Trace_Profiler_Instance->begin(std::string(Name), Metadata,
                                            وقت_Trace_Event_Type::CompleteEvent);
  أرجع خالي_اشارو;
}

وقت_Trace_Profiler_داخلا *llvm::وقت_Trace_Async_Profiler_Begin(StringRef Name,
                                                          StringRef Detail) {
  إذا (وقت_Trace_Profiler_Instance != خالي_اشارو)
    أرجع وقت_Trace_Profiler_Instance->begin(
        std::string(Name), [&]() { أرجع std::string(Detail); },
        وقت_Trace_Event_Type::AsyncEvent);
  أرجع خالي_اشارو;
}

فراغ llvm::وقت_Trace_جوڙيو_Instant_Event(StringRef Name,
                                    llvm::function_ref<std::string()> Detail) {
  إذا (وقت_Trace_Profiler_Instance != خالي_اشارو)
    وقت_Trace_Profiler_Instance->وجهو(std::string(Name), Detail);
}

فراغ llvm::وقت_Trace_Profiler_پڇاڙي() {
  إذا (وقت_Trace_Profiler_Instance != خالي_اشارو)
    وقت_Trace_Profiler_Instance->پڇاڙي();
}

فراغ llvm::وقت_Trace_Profiler_پڇاڙي(وقت_Trace_Profiler_داخلا *E) {
  إذا (وقت_Trace_Profiler_Instance != خالي_اشارو)
    وقت_Trace_Profiler_Instance->پڇاڙي(*E);
}
