#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/DebugCounter.h"

#ਅਨ੍ਤਰ੍ਭਾਵ "DebugOptions.h"

#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/CommandLine.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/Format.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/ManagedStatic.h"

ਵਰਤੋਂ ਨਾਮਾਕਾਸ਼ llvm;

ਨਾਮਾਕਾਸ਼ llvm {

ਖਾਲੀ DebugCounter::printChunks(raw_ostream &OS,
                               ArrayRef<IntegerInclusiveInterval> Chunks) {
  IntegerInclusiveIntervalUtils::printIntervals(OS, Chunks, ':');
}

} // namespace llvm

ਨਾਮਾਕਾਸ਼ {
// This class overrides the default list implementation of printing so we
// can pretty print the list of debug counter options.  This type of
// dynamic option is pretty rare (basically this and pass lists).
ਵਰਗ DebugCounterList : ਜਨਤਕ cl::list<std::string, DebugCounter> {
ਨਿਜੀ:
  ਵਰਤੋਂ ਅਧਾਰ = cl::list<std::string, DebugCounter>;

ਜਨਤਕ:
  ਸਾਂਚਾ <ਵਰਗ... Mods>
  ਸਪਸ਼ਟ DebugCounterList(Mods &&... Ms) : ਅਧਾਰ(std::forward<Mods>(Ms)...) {}

ਨਿਜੀ:
  ਖਾਲੀ printOptionInfo(size_t Global_ਚੌੜਾਈ) ਅਚਲ override {
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
    Option::printHelpStr(HelpStr, Global_ਚੌੜਾਈ, ArgStr.ਮਾਪ_2() + 6);
    ਅਚਲ ਸਵੈਚਾਲਿਤ &CounterInstance = DebugCounter::instance();
    ਲਈ (ਅਚਲ ਸਵੈਚਾਲਿਤ &ਇੰਦਰਾਜ : CounterInstance) {
      ਅਚਲ ਸਵੈਚਾਲਿਤ &[Name, Desc] = CounterInstance.getCounterDesc(ਇੰਦਰਾਜ.ਦੂਜਾ_2);
      size_t ਅੰਕ_Spaces = Global_ਚੌੜਾਈ - Name.ਮਾਪ_2() - 8;
      outs() << "    =" << Name;
      outs().indent(ਅੰਕ_Spaces) << " -   " << Desc << '\n';
    }
  }
};

// All global objects associated to the DebugCounter, including the DebugCounter
// itself, are owned by a single global instance of the DebugCounterOwner
// struct. This makes it easier to control the order in which constructors and
// destructors are run.
ਬਣਤਰ DebugCounterOwner : DebugCounter {
  DebugCounterList DebugCounterOption{
      "debug-counter", cl::Hidden,
      cl::desc("Comma separated list of debug counter skip and count"),
      cl::CommaSeparated, cl::location<DebugCounter>(*ਇਹ)};
  cl::opt<ਤਾਰਕਿਕ, ਸੱਚ> PrintDebugCounter{
      "print-debug-counter",
      cl::Hidden,
      cl::location(ਇਹ->ShouldPrintCounter),
      cl::init(ਝੂਠ),
      cl::desc("Print out debug counter info after all counters accumulated"),
      cl::callback([&](ਅਚਲ ਤਾਰਕਿਕ &ਮੁੱਲ) {
        ਜੇ (ਮੁੱਲ)
          activateAllCounters();
      })};
  cl::opt<ਤਾਰਕਿਕ, ਸੱਚ> PrintDebugCounterQueries{
      "print-debug-counter-queries",
      cl::Hidden,
      cl::location(ਇਹ->ShouldPrintCounterQueries),
      cl::init(ਝੂਠ),
      cl::desc("Print out each query of an enabled debug counter")};
  cl::opt<ਤਾਰਕਿਕ, ਸੱਚ> Break_On_ਆਖਰੀ_ਗਿਣਤੀ{
      "debug-counter-break-on-last",
      cl::Hidden,
      cl::location(ਇਹ->Break_On_ਆਖਰੀ),
      cl::init(ਝੂਠ),
      cl::desc("Insert a break point on the last enabled count of a "
               "chunks list")};

  DebugCounterOwner() {
    // Our destructor uses the debug stream. By referencing it here, we
    // ensure that its destructor runs after our destructor.
    (ਖਾਲੀ)dbgs();
  }

  // Print information when destroyed, iff command line option is specified.
  ~DebugCounterOwner() {
    ਜੇ (ShouldPrintCounter)
      print(dbgs());
  }
};

} // anonymous namespace

// Use ManagedStatic instead of function-local static variable to ensure
// the destructor (which accesses counters and streams) runs during
// llvm_shutdown() rather than at some unspecified point.
ਸਥਿਰ ManagedStatic<DebugCounterOwner> Owner;

ਖਾਲੀ llvm::initDebugCounterOptions() { (ਖਾਲੀ)DebugCounter::instance(); }

DebugCounter &DebugCounter::instance() { ਵਾਪਸ *Owner; }

// This is called by the command line parser when it sees a value for the
// debug-counter option defined above.
ਖਾਲੀ DebugCounter::ਧੱਕੋ_ਪਿੱਠ(ਅਚਲ std::string &Val) {
  ਜੇ (Val.ਖਾਲੀ_ਥਾਵਾਂ())
    ਵਾਪਸ;

  // The strings should come in as counter=chunk_list
  ਸਵੈਚਾਲਿਤ CounterPair = StringRef(Val).ਵੰਡੋ('=');
  ਜੇ (CounterPair.ਦੂਜਾ_2.ਖਾਲੀ_ਥਾਵਾਂ()) {
    errs() << "DebugCounter Error: " << Val << " does not have an = in it\n";
    ਬਾਹਰ_ਨਿਕਲੋ(1);
  }
  StringRef CounterName = CounterPair.ਪਹਿਲਾ_2;

  CounterInfo *Counter = getCounterInfo(CounterName);
  ਜੇ (!Counter) {
    errs() << "DebugCounter Error: " << CounterName
           << " is not a registered counter\n";
    ਵਾਪਸ;
  }

  ਸਵੈਚਾਲਿਤ ExpectedChunks =
      IntegerInclusiveIntervalUtils::parseIntervals(CounterPair.ਦੂਜਾ_2, ':');
  ਜੇ (!ExpectedChunks) {
    handleAllErrors(ExpectedChunks.takeError(), [&](ਅਚਲ StringError &E) {
      errs() << "DebugCounter Error: " << E.get_ਸੁਨੇਹਾ() << "\n";
    });
    ਬਾਹਰ_ਨਿਕਲੋ(1);
  }
  Counter->Chunks = std::move(*ExpectedChunks);
  Counter->ਸਰਗਰਮ = Counter->Is_ਸੈੱਟ = ਸੱਚ;
}

ਖਾਲੀ DebugCounter::print(raw_ostream &OS) ਅਚਲ {
  Small_ਸਦਿਸ਼<StringRef, 16> Counter_ਨਾਂ(Counters.ਕੁੰਜੀਆਂ());
  ਛਾਂਟੋ(Counter_ਨਾਂ);

  OS << "Counters and values:\n";
  ਲਈ (StringRef CounterName : Counter_ਨਾਂ) {
    ਅਚਲ CounterInfo *C = getCounterInfo(CounterName);
    OS << ਖੱਬਾ_justify(C->Name, 32) << ": {" << C->ਗਿਣਤੀ_3 << ",";
    printChunks(OS, C->Chunks);
    OS << "}\n";
  }
}

ਤਾਰਕਿਕ DebugCounter::handleCounterIncrement(CounterInfo &Info) {
  int64_t Curr_ਗਿਣਤੀ = Info.ਗਿਣਤੀ_3++;
  uint64_t CurrIdx = Info.CurrChunkIdx;

  ਜੇ (Info.Chunks.ਖਾਲੀ_ਥਾਵਾਂ())
    ਵਾਪਸ ਸੱਚ;
  ਜੇ (CurrIdx >= Info.Chunks.ਮਾਪ_2())
    ਵਾਪਸ ਝੂਠ;

  ਤਾਰਕਿਕ Res = Info.Chunks[CurrIdx].ਸ਼ਾਮਲ_ਹੈ(Curr_ਗਿਣਤੀ);
  ਜੇ (Break_On_ਆਖਰੀ && CurrIdx == (Info.Chunks.ਮਾਪ_2() - 1) &&
      Curr_ਗਿਣਤੀ == Info.Chunks[CurrIdx].get_ਸਿਰਾ()) {
    LLVM_BUILTIN_DEBUGTRAP;
  }
  ਜੇ (Curr_ਗਿਣਤੀ > Info.Chunks[CurrIdx].get_ਸਿਰਾ()) {
    Info.CurrChunkIdx++;

    /// Handle consecutive blocks.
    ਜੇ (Info.CurrChunkIdx < Info.Chunks.ਮਾਪ_2() &&
        Curr_ਗਿਣਤੀ == Info.Chunks[Info.CurrChunkIdx].getBegin())
      ਵਾਪਸ ਸੱਚ;
  }
  ਵਾਪਸ Res;
}

ਤਾਰਕਿਕ DebugCounter::shouldExecuteImpl(CounterInfo &Counter) {
  ਸਵੈਚਾਲਿਤ &Us = instance();
  ਤਾਰਕਿਕ Res = Us.handleCounterIncrement(Counter);
  ਜੇ (Us.ShouldPrintCounterQueries && Counter.Is_ਸੈੱਟ) {
    dbgs() << "DebugCounter " << Counter.Name << "=" << (Counter.ਗਿਣਤੀ_3 - 1)
           << (Res ? " execute" : " skip") << "\n";
  }
  ਵਾਪਸ Res;
}

#ਜੇ !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
LLVM_DUMP_ਵਿਧੀ ਖਾਲੀ DebugCounter::dump() ਅਚਲ {
  print(dbgs());
}
#ਜੇ_ਅੰਤ
