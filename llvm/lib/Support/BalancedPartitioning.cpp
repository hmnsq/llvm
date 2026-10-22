//===- BalancedPartitioning.cpp -------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file implements BalancedPartitioning, a recursive balanced graph
// partitioning algorithm.
//
//===----------------------------------------------------------------------===//

#ଅନ୍ତର୍ଭାଵ "llvm/Support/BalancedPartitioning.h"
#ଅନ୍ତର୍ଭାଵ "llvm/Config/llvm-config.h" // for LLVM_ENABLE_THREADS
#ଅନ୍ତର୍ଭାଵ "llvm/Support/Debug.h"
#ଅନ୍ତର୍ଭାଵ "llvm/Support/Format.h"
#ଅନ୍ତର୍ଭାଵ "llvm/Support/FormatVariadic.h"
#ଅନ୍ତର୍ଭାଵ "llvm/Support/ThreadPool.h"

#ଅନ୍ତର୍ଭାଵ <cmath>

using ନାମାକାଶ llvm;
#define DEBUG_TYPE "balanced-partitioning"

ଶୂନ୍ୟ BPFunction_ଗଣ୍ଠି::dump(raw_ostream &OS) ଅଚଳ {
  OS << formatv("{{ID={0} Utilities={{{1:$[,]}} Bucket={2}}", Id,
                make_range(UtilityNodes.begin(), UtilityNodes.end()), Bucket);
}

template <typename Func>
ଶୂନ୍ୟ BalancedPartitioning::BPThreadPool::async(Func &&F) {
#ଯଦି LLVM_ENABLE_THREADS
  // This new thread could spawn more threads, so mark it as active
  ++ଅଙ୍କ_Active_Threads;
  TheThreadPool.async([ଏହି, F]() {
    // Run the task
    F();

    // This thread will no longer spawn new threads, so mark it as inactive
    ଯଦି (--ଅଙ୍କ_Active_Threads == 0) {
      // There are no more active threads, so mark as finished and notify
      {
        std::unique_lock<std::mutex> lock(mtx);
        ପ୍ରତିଜ୍ଞା(!IsFinishedSpawning);
        IsFinishedSpawning = ସତ୍ୟ;
      }
      cv.notify_one();
    }
  });
#ଅନ୍ୟଥା
  llvm_unreachable("threads are disabled");
#endif
}

ଶୂନ୍ୟ BalancedPartitioning::BPThreadPool::wait() {
#ଯଦି LLVM_ENABLE_THREADS
  // TODO: We could remove the mutex and condition variable and use
  // std::atomic::wait() instead, but that isn't available until C++20
  {
    std::unique_lock<std::mutex> lock(mtx);
    cv.wait(lock, [&]() { ଫେରନ୍ତୁ IsFinishedSpawning; });
    ପ୍ରତିଜ୍ଞା(IsFinishedSpawning && ଅଙ୍କ_Active_Threads == 0);
  }
  // Now we can call ThreadPool::wait() since all tasks have been submitted
  TheThreadPool.wait();
#ଅନ୍ୟଥା
  llvm_unreachable("threads are disabled");
#endif
}

BalancedPartitioning::BalancedPartitioning(
    ଅଚଳ BalancedPartitioningConfig &Config)
    : Config(Config) {
  // Pre-computing log2 values
  Log2Cache[0] = 0.0;
  ପାଇଁ (unsigned I = 1; I < LOG_CACHE_SIZE; I++)
    Log2Cache[I] = std::log2(I);
}

ଶୂନ୍ୟ BalancedPartitioning::run(std::vector<BPFunction_ଗଣ୍ଠି> &Nodes) ଅଚଳ {
  LLVM_DEBUG(
      dbgs() << format(
          "Partitioning %d nodes using depth %d and %d iterations per split\n",
          Nodes.size(), Config.SplitDepth, Config.IterationsPerSplit));
  std::optional<BPThreadPool> TP;
#ଯଦି LLVM_ENABLE_THREADS
  DefaultThreadPool TheThreadPool;
  ଯଦି (Config.TaskSplitDepth > 1)
    TP.emplace(TheThreadPool);
#endif

  // Record the input order
  ପାଇଁ (unsigned I = 0; I < Nodes.size(); I++)
    Nodes[I].ନିବେଶ_Order_Index = I;

  auto NodesRange = llvm::make_range(Nodes.begin(), Nodes.end());
  auto BisectTask = [ଏହି, NodesRange, &TP]() {
    bisect(NodesRange, /*RecDepth=*/0, /*RootBucket=*/1, /*Offset=*/0, TP);
  };
  ଯଦି (TP) {
    TP->async(std::move(BisectTask));
    TP->wait();
  } ଅନ୍ୟଥା {
    BisectTask();
  }

  llvm::stable_sort(NodesRange, [](ଅଚଳ auto &L, ଅଚଳ auto &R) {
    ଫେରନ୍ତୁ L.Bucket < R.Bucket;
  });

  LLVM_DEBUG(dbgs() << "Balanced partitioning completed\n");
}

ଶୂନ୍ୟ BalancedPartitioning::bisect(ଅଚଳ Function_ଗଣ୍ଠି_Range Nodes,
                                  unsigned RecDepth, unsigned ମୂଳ_Bucket,
                                  unsigned Offset,
                                  std::optional<BPThreadPool> &TP) ଅଚଳ {
  unsigned ଅଙ୍କ_Nodes = llvm::size(Nodes);
  ଯଦି (ଅଙ୍କ_Nodes <= 1 || RecDepth >= Config.SplitDepth) {
    // We've reach the lowest level of the recursion tree. Fall back to the
    // original order and assign to buckets.
    llvm::sort(Nodes, [](ଅଚଳ auto &L, ଅଚଳ auto &R) {
      ଫେରନ୍ତୁ L.ନିବେଶ_Order_Index < R.ନିବେଶ_Order_Index;
    });
    ପାଇଁ (auto &N : Nodes)
      N.Bucket = Offset++;
    ଫେରନ୍ତୁ;
  }

  LLVM_DEBUG(dbgs() << format("Bisect with %d nodes and root bucket %d\n",
                              ଅଙ୍କ_Nodes, ମୂଳ_Bucket));

  std::mt19937 RNG(ମୂଳ_Bucket);

  unsigned ବାମ_Bucket = 2 * ମୂଳ_Bucket;
  unsigned ଡାହାଣ_Bucket = 2 * ମୂଳ_Bucket + 1;

  // Split into two and assign to the left and right buckets
  split(Nodes, ବାମ_Bucket);

  runIterations(Nodes, ବାମ_Bucket, ଡାହାଣ_Bucket, RNG);

  // Split nodes wrt the resulting buckets
  auto NodesMid =
      llvm::partition(Nodes, [&](auto &N) { ଫେରନ୍ତୁ N.Bucket == ବାମ_Bucket; });
  unsigned MidOffset = Offset + std::distance(Nodes.begin(), NodesMid);

  auto ବାମ_Nodes = llvm::make_range(Nodes.begin(), NodesMid);
  auto ଡାହାଣ_Nodes = llvm::make_range(NodesMid, Nodes.end());

  auto ବାମ_Rec_Task = [ଏହି, ବାମ_Nodes, RecDepth, ବାମ_Bucket, Offset, &TP]() {
    bisect(ବାମ_Nodes, RecDepth + 1, ବାମ_Bucket, Offset, TP);
  };
  auto ଡାହାଣ_Rec_Task = [ଏହି, ଡାହାଣ_Nodes, RecDepth, ଡାହାଣ_Bucket, MidOffset,
                       &TP]() {
    bisect(ଡାହାଣ_Nodes, RecDepth + 1, ଡାହାଣ_Bucket, MidOffset, TP);
  };

  ଯଦି (TP && RecDepth < Config.TaskSplitDepth && ଅଙ୍କ_Nodes >= 4) {
    TP->async(std::move(ବାମ_Rec_Task));
    TP->async(std::move(ଡାହାଣ_Rec_Task));
  } ଅନ୍ୟଥା {
    ବାମ_Rec_Task();
    ଡାହାଣ_Rec_Task();
  }
}

ଶୂନ୍ୟ BalancedPartitioning::runIterations(ଅଚଳ Function_ଗଣ୍ଠି_Range Nodes,
                                         unsigned ବାମ_Bucket,
                                         unsigned ଡାହାଣ_Bucket,
                                         std::mt19937 &RNG) ଅଚଳ {
  unsigned ଅଙ୍କ_Nodes = llvm::size(Nodes);
  DenseMap<BPFunction_ଗଣ୍ଠି::Utility_ଗଣ୍ଠି_T, unsigned> Utility_ଗଣ୍ଠି_Index;
  ପାଇଁ (auto &N : Nodes)
    ପାଇଁ (auto &UN : N.UtilityNodes)
      ++Utility_ଗଣ୍ଠି_Index[UN];
  // Remove utility nodes if they have just one edge or are connected to all
  // functions
  ପାଇଁ (auto &N : Nodes)
    llvm::erase_if(N.UtilityNodes, [&](auto &UN) {
      unsigned UNI = Utility_ଗଣ୍ଠି_Index[UN];
      ଫେରନ୍ତୁ UNI == 1 || UNI == ଅଙ୍କ_Nodes;
    });

  // Renumber utility nodes so they can be used to index into Signatures
  Utility_ଗଣ୍ଠି_Index.clear();
  ପାଇଁ (auto &N : Nodes)
    ପାଇଁ (auto &UN : N.UtilityNodes)
      UN = Utility_ଗଣ୍ଠି_Index.ଭର୍ତ୍ତି({UN, Utility_ଗଣ୍ଠି_Index.size()}).first->second;

  // Initialize signatures
  SignaturesT Signatures(/*Size=*/Utility_ଗଣ୍ଠି_Index.size());
  ପାଇଁ (auto &N : Nodes) {
    ପାଇଁ (auto &UN : N.UtilityNodes) {
      ପ୍ରତିଜ୍ଞା(UN < Signatures.size());
      ଯଦି (N.Bucket == ବାମ_Bucket) {
        Signatures[UN].ବାମ_ଗଣନା++;
      } ଅନ୍ୟଥା {
        Signatures[UN].ଡାହାଣ_ଗଣନା++;
      }
    }
  }

  ପାଇଁ (unsigned I = 0; I < Config.IterationsPerSplit; I++) {
    unsigned ଅଙ୍କ_Moved_Nodes =
        runIteration(Nodes, ବାମ_Bucket, ଡାହାଣ_Bucket, Signatures, RNG);
    ଯଦି (ଅଙ୍କ_Moved_Nodes == 0)
      ଭାଙ୍ଗ;
  }
}

unsigned BalancedPartitioning::runIteration(ଅଚଳ Function_ଗଣ୍ଠି_Range Nodes,
                                            unsigned ବାମ_Bucket,
                                            unsigned ଡାହାଣ_Bucket,
                                            SignaturesT &Signatures,
                                            std::mt19937 &RNG) ଅଚଳ {
  // Init signature cost caches
  ପାଇଁ (auto &Signature : Signatures) {
    ଯଦି (Signature.CachedGainIsValid)
      ଜାରି;
    unsigned L = Signature.ବାମ_ଗଣନା;
    unsigned R = Signature.ଡାହାଣ_ଗଣନା;
    ପ୍ରତିଜ୍ଞା((L > 0 || R > 0) && "incorrect signature");
    ଦଶମିକ Cost = logCost(L, R);
    Signature.CachedGainLR = 0.f;
    Signature.CachedGainRL = 0.f;
    ଯଦି (L > 0)
      Signature.CachedGainLR = Cost - logCost(L - 1, R + 1);
    ଯଦି (R > 0)
      Signature.CachedGainRL = Cost - logCost(L + 1, R - 1);
    Signature.CachedGainIsValid = ସତ୍ୟ;
  }

  // Compute move gains
  using GainPair = std::pair<ଦଶମିକ, BPFunction_ଗଣ୍ଠି *>;
  std::vector<GainPair> Gains;
  ପାଇଁ (auto &N : Nodes) {
    ତାର୍କିକ From_ବାମ_To_ଡାହାଣ = (N.Bucket == ବାମ_Bucket);
    ଦଶମିକ Gain = moveGain(N, From_ବାମ_To_ଡାହାଣ, Signatures);
    Gains.ଠେଲ_back(std::make_pair(Gain, &N));
  }

  // Collect left and right gains
  auto ବାମ_End = llvm::partition(
      Gains, [&](ଅଚଳ auto &GP) { ଫେରନ୍ତୁ GP.second->Bucket == ବାମ_Bucket; });
  auto ବାମ_Range = llvm::make_range(Gains.begin(), ବାମ_End);
  auto ଡାହାଣ_Range = llvm::make_range(ବାମ_End, Gains.end());

  // Sort gains in descending order
  auto LargerGain = [](ଅଚଳ auto &L, ଅଚଳ auto &R) {
    ଫେରନ୍ତୁ L.first > R.first;
  };
  llvm::stable_sort(ବାମ_Range, LargerGain);
  llvm::stable_sort(ଡାହାଣ_Range, LargerGain);

  unsigned ଅଙ୍କ_Moved_Data_Vertices = 0;
  ପାଇଁ (auto [ବାମ_Pair, ଡାହାଣ_Pair] : llvm::zip(ବାମ_Range, ଡାହାଣ_Range)) {
    auto &[ବାମ_Gain, ବାମ_ଗଣ୍ଠି] = ବାମ_Pair;
    auto &[ଡାହାଣ_Gain, ଡାହାଣ_ଗଣ୍ଠି] = ଡାହାଣ_Pair;
    // Stop when the gain is no longer beneficial
    ଯଦି (ବାମ_Gain + ଡାହାଣ_Gain <= 0.f)
      ଭାଙ୍ଗ;
    // Try to exchange the nodes between buckets
    ଯଦି (move_Function_ଗଣ୍ଠି(*ବାମ_ଗଣ୍ଠି, ବାମ_Bucket, ଡାହାଣ_Bucket, Signatures, RNG))
      ++ଅଙ୍କ_Moved_Data_Vertices;
    ଯଦି (move_Function_ଗଣ୍ଠି(*ଡାହାଣ_ଗଣ୍ଠି, ବାମ_Bucket, ଡାହାଣ_Bucket, Signatures, RNG))
      ++ଅଙ୍କ_Moved_Data_Vertices;
  }
  ଫେରନ୍ତୁ ଅଙ୍କ_Moved_Data_Vertices;
}

ତାର୍କିକ BalancedPartitioning::move_Function_ଗଣ୍ଠି(BPFunction_ଗଣ୍ଠି &N,
                                            unsigned ବାମ_Bucket,
                                            unsigned ଡାହାଣ_Bucket,
                                            SignaturesT &Signatures,
                                            std::mt19937 &RNG) ଅଚଳ {
  // Sometimes we skip the move. This helps to escape local optima
  ଯଦି (std::uniform_real_distribution<ଦଶମିକ>(0.f, 1.f)(RNG) <=
      Config.SkipProbability)
    ଫେରନ୍ତୁ ମିଥ୍ୟା;

  ତାର୍କିକ From_ବାମ_To_ଡାହାଣ = (N.Bucket == ବାମ_Bucket);
  // Update the current bucket
  N.Bucket = (From_ବାମ_To_ଡାହାଣ ? ଡାହାଣ_Bucket : ବାମ_Bucket);

  // Update signatures and invalidate gain cache
  ଯଦି (From_ବାମ_To_ଡାହାଣ) {
    ପାଇଁ (auto &UN : N.UtilityNodes) {
      auto &Signature = Signatures[UN];
      Signature.ବାମ_ଗଣନା--;
      Signature.ଡାହାଣ_ଗଣନା++;
      Signature.CachedGainIsValid = ମିଥ୍ୟା;
    }
  } ଅନ୍ୟଥା {
    ପାଇଁ (auto &UN : N.UtilityNodes) {
      auto &Signature = Signatures[UN];
      Signature.ବାମ_ଗଣନା++;
      Signature.ଡାହାଣ_ଗଣନା--;
      Signature.CachedGainIsValid = ମିଥ୍ୟା;
    }
  }
  ଫେରନ୍ତୁ ସତ୍ୟ;
}

ଶୂନ୍ୟ BalancedPartitioning::split(ଅଚଳ Function_ଗଣ୍ଠି_Range Nodes,
                                 unsigned StartBucket) ଅଚଳ {
  unsigned ଅଙ୍କ_Nodes = llvm::size(Nodes);
  auto NodesMid = Nodes.begin() + (ଅଙ୍କ_Nodes + 1) / 2;

  llvm::sort(Nodes, [](auto &L, auto &R) {
    ଫେରନ୍ତୁ L.ନିବେଶ_Order_Index < R.ନିବେଶ_Order_Index;
  });

  ପାଇଁ (auto &N : llvm::make_range(Nodes.begin(), NodesMid))
    N.Bucket = StartBucket;
  ପାଇଁ (auto &N : llvm::make_range(NodesMid, Nodes.end()))
    N.Bucket = StartBucket + 1;
}

ଦଶମିକ BalancedPartitioning::moveGain(ଅଚଳ BPFunction_ଗଣ୍ଠି &N,
                                     ତାର୍କିକ From_ବାମ_To_ଡାହାଣ,
                                     ଅଚଳ SignaturesT &Signatures) {
  ଦଶମିକ Gain = 0.f;
  ପାଇଁ (auto &UN : N.UtilityNodes)
    Gain += (From_ବାମ_To_ଡାହାଣ ? Signatures[UN].CachedGainLR
                             : Signatures[UN].CachedGainRL);
  ଫେରନ୍ତୁ Gain;
}

ଦଶମିକ BalancedPartitioning::logCost(unsigned X, unsigned Y) ଅଚଳ {
  ଫେରନ୍ତୁ -(X * log2Cached(X + 1) + Y * log2Cached(Y + 1));
}

ଦଶମିକ BalancedPartitioning::log2Cached(unsigned i) ଅଚଳ {
  ଫେରନ୍ତୁ (i < LOG_CACHE_SIZE) ? Log2Cache[i] : std::log2(i);
}
