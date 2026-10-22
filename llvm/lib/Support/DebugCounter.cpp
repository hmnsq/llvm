#ضمن "llvm/Support/DebugCounter.h"

#ضمن "DebugOptions.h"

#ضمن "llvm/Support/CommandLine.h"
#ضمن "llvm/Support/Format.h"
#ضمن "llvm/Support/ManagedStatic.h"

using نطاق llvm;

نطاق llvm {

فراغ DebugCounter::printChunks(raw_ostream &OS,
                               ArrayRef<IntegerInclusiveInterval> Chunks) {
  IntegerInclusiveIntervalUtils::printIntervals(OS, Chunks, ':');
}

} // namespace llvm

نطاق {
// This class overrides the default list implementation of printing so we
// can pretty print the list of debug counter options.  This type of
// dynamic option is pretty rare (basically this and pass lists).
מחלקה DebugCounterList : عام cl::list<std::string, DebugCounter> {
خاص:
  using Base = cl::list<std::string, DebugCounter>;

عام:
  template <מחלקה... Mods>
  explicit DebugCounterList(Mods &&... Ms) : Base(std::forward<Mods>(Ms)...) {}

خاص:
  فراغ printOptionInfo(size_t GlobalWidth) ثابت override {
    // This is a variant of from generic_parser_base::printOptionInfo.  Sadly,
    // it's not easy to make it more usable.  We could get it to print these as
    // options if we were a cl::opt and registered them, but lists don't have
    // options, nor does the parser for std::string.  The other mechanisms for
    // options are global and would pollute the global namespace with our
    // counters.  Rather than go that route, we have just overridden the
    // printing, which only a few things call anyway.
    outs() << "  -" << ArgStr;
    // All of the other options in CommandLine.cpp use ArgStr.size() + 6 for
    // width, so we do the same.
    Option::printHelpStr(HelpStr, GlobalWidth, ArgStr.size() + 6);
    ثابت auto &CounterInstance = DebugCounter::instance();
    עבור (ثابت auto &Entry : CounterInstance) {
      ثابت auto &[Name, Desc] = CounterInstance.getCounterDesc(Entry.second);
      size_t NumSpaces = GlobalWidth - Name.size() - 8;
      outs() << "    =" << Name;
      outs().indent(NumSpaces) << " -   " << Desc << '\n';
    }
  }
};

// All global objects associated to the DebugCounter, including the DebugCounter
// itself, are owned by a single global instance of the DebugCounterOwner
// struct. This makes it easier to control the order in which constructors and
// destructors are run.
بنية DebugCounterOwner : DebugCounter {
  DebugCounterList DebugCounterOption{
      "debug-counter", cl::Hidden,
      cl::desc("Comma separated list of debug counter skip and count"),
      cl::CommaSeparated, cl::location<DebugCounter>(*هذا)};
  cl::opt<منطقي, אמת> PrintDebugCounter{
      "print-debug-counter",
      cl::Hidden,
      cl::location(هذا->ShouldPrintCounter),
      cl::init(שקר),
      cl::desc("Print out debug counter info after all counters accumulated"),
      cl::callback([&](ثابت منطقي &Value) {
        אם (Value)
          activateAllCounters();
      })};
  cl::opt<منطقي, אמת> PrintDebugCounterQueries{
      "print-debug-counter-queries",
      cl::Hidden,
      cl::location(هذا->ShouldPrintCounterQueries),
      cl::init(שקר),
      cl::desc("Print out each query of an enabled debug counter")};
  cl::opt<منطقي, אמת> BreakOnLastCount{
      "debug-counter-break-on-last",
      cl::Hidden,
      cl::location(هذا->BreakOnLast),
      cl::init(שקר),
      cl::desc("Insert a break point on the last enabled count of a "
               "chunks list")};

  DebugCounterOwner() {
    // Our destructor uses the debug stream. By referencing it here, we
    // ensure that its destructor runs after our destructor.
    (فراغ)dbgs();
  }

  // Print information when destroyed, iff command line option is specified.
  ~DebugCounterOwner() {
    אם (ShouldPrintCounter)
      print(dbgs());
  }
};

} // anonymous namespace

// Use ManagedStatic instead of function-local static variable to ensure
// the destructor (which accesses counters and streams) runs during
// llvm_shutdown() rather than at some unspecified point.
ساكن ManagedStatic<DebugCounterOwner> Owner;

فراغ llvm::initDebugCounterOptions() { (فراغ)DebugCounter::instance(); }

DebugCounter &DebugCounter::instance() { החזר *Owner; }

// This is called by the command line parser when it sees a value for the
// debug-counter option defined above.
فراغ DebugCounter::push_back(ثابت std::string &Val) {
  אם (Val.empty())
    החזר;

  // The strings should come in as counter=chunk_list
  auto CounterPair = StringRef(Val).split('=');
  אם (CounterPair.second.empty()) {
    errs() << "DebugCounter Error: " << Val << " does not have an = in it\n";
    exit(1);
  }
  StringRef CounterName = CounterPair.first;

  CounterInfo *Counter = getCounterInfo(CounterName);
  אם (!Counter) {
    errs() << "DebugCounter Error: " << CounterName
           << " is not a registered counter\n";
    החזר;
  }

  auto ExpectedChunks =
      IntegerInclusiveIntervalUtils::parseIntervals(CounterPair.second, ':');
  אם (!ExpectedChunks) {
    handleAllErrors(ExpectedChunks.takeError(), [&](ثابت StringError &E) {
      errs() << "DebugCounter Error: " << E.getMessage() << "\n";
    });
    exit(1);
  }
  Counter->Chunks = std::move(*ExpectedChunks);
  Counter->Active = Counter->IsSet = אמת;
}

فراغ DebugCounter::print(raw_ostream &OS) ثابت {
  SmallVector<StringRef, 16> CounterNames(Counters.keys());
  sort(CounterNames);

  OS << "Counters and values:\n";
  עבור (StringRef CounterName : CounterNames) {
    ثابت CounterInfo *C = getCounterInfo(CounterName);
    OS << left_justify(C->Name, 32) << ": {" << C->Count << ",";
    printChunks(OS, C->Chunks);
    OS << "}\n";
  }
}

منطقي DebugCounter::handleCounterIncrement(CounterInfo &Info) {
  int64_t CurrCount = Info.Count++;
  uint64_t CurrIdx = Info.CurrChunkIdx;

  אם (Info.Chunks.empty())
    החזר אמת;
  אם (CurrIdx >= Info.Chunks.size())
    החזר שקר;

  منطقي Res = Info.Chunks[CurrIdx].contains(CurrCount);
  אם (BreakOnLast && CurrIdx == (Info.Chunks.size() - 1) &&
      CurrCount == Info.Chunks[CurrIdx].getEnd()) {
    LLVM_BUILTIN_DEBUGTRAP;
  }
  אם (CurrCount > Info.Chunks[CurrIdx].getEnd()) {
    Info.CurrChunkIdx++;

    /// Handle consecutive blocks.
    אם (Info.CurrChunkIdx < Info.Chunks.size() &&
        CurrCount == Info.Chunks[Info.CurrChunkIdx].getBegin())
      החזר אמת;
  }
  החזר Res;
}

منطقي DebugCounter::shouldExecuteImpl(CounterInfo &Counter) {
  auto &Us = instance();
  منطقي Res = Us.handleCounterIncrement(Counter);
  אם (Us.ShouldPrintCounterQueries && Counter.IsSet) {
    dbgs() << "DebugCounter " << Counter.Name << "=" << (Counter.Count - 1)
           << (Res ? " execute" : " skip") << "\n";
  }
  החזר Res;
}

#אם !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
LLVM_DUMP_METHOD فراغ DebugCounter::dump() ثابت {
  print(dbgs());
}
#endif
