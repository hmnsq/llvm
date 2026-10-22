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

#അന്തര്ഭാവ "llvm/Support/BalancedPartitioning.h"
#അന്തര്ഭാവ "llvm/Config/llvm-config.h" // for LLVM_ENABLE_THREADS
#അന്തര്ഭാവ "llvm/Support/Debug.h"
#അന്തര്ഭാവ "llvm/Support/Format.h"
#അന്തര്ഭാവ "llvm/Support/FormatVariadic.h"
#അന്തര്ഭാവ "llvm/Support/ThreadPool.h"

#അന്തര്ഭാവ <cmath>

using നാമാകാശ llvm;
#define DEBUG_TYPE "balanced-partitioning"

ശൂന്യം BPFunction_കണ്ണി::dump(raw_ostream &OS) സ്ഥിരം {
  OS << formatv("{{ID={0} Utilities={{{1:$[,]}} Bucket={2}}", Id,
                make_range(UtilityNodes.begin(), UtilityNodes.end()), Bucket);
}

template <typename Func>
ശൂന്യം BalancedPartitioning::BPThreadPool::async(Func &&F) {
#എങ്കിൽ LLVM_ENABLE_THREADS
  // This new thread could spawn more threads, so mark it as active
  ++അക്കം_Active_Threads;
  TheThreadPool.async([ഇത്, F]() {
    // Run the task
    F();

    // This thread will no longer spawn new threads, so mark it as inactive
    എങ്കിൽ (--അക്കം_Active_Threads == 0) {
      // There are no more active threads, so mark as finished and notify
      {
        std::unique_lock<std::mutex> lock(mtx);
        പ്രതിജ്ഞാ(!IsFinishedSpawning);
        IsFinishedSpawning = സത്യം;
      }
      cv.notify_one();
    }
  });
#അല്ലെങ്കിൽ
  llvm_unreachable("threads are disabled");
#endif
}

ശൂന്യം BalancedPartitioning::BPThreadPool::wait() {
#എങ്കിൽ LLVM_ENABLE_THREADS
  // TODO: We could remove the mutex and condition variable and use
  // std::atomic::wait() instead, but that isn't available until C++20
  {
    std::unique_lock<std::mutex> lock(mtx);
    cv.wait(lock, [&]() { തിരികെ IsFinishedSpawning; });
    പ്രതിജ്ഞാ(IsFinishedSpawning && അക്കം_Active_Threads == 0);
  }
  // Now we can call ThreadPool::wait() since all tasks have been submitted
  TheThreadPool.wait();
#അല്ലെങ്കിൽ
  llvm_unreachable("threads are disabled");
#endif
}

BalancedPartitioning::BalancedPartitioning(
    സ്ഥിരം BalancedPartitioningConfig &Config)
    : Config(Config) {
  // Pre-computing log2 values
  Log2Cache[0] = 0.0;
  വേണ്ടി (unsigned I = 1; I < LOG_CACHE_SIZE; I++)
    Log2Cache[I] = std::log2(I);
}

ശൂന്യം BalancedPartitioning::run(std::vector<BPFunction_കണ്ണി> &Nodes) സ്ഥിരം {
  LLVM_DEBUG(
      dbgs() << format(
          "Partitioning %d nodes using depth %d and %d iterations per split\n",
          Nodes.size(), Config.SplitDepth, Config.IterationsPerSplit));
  std::optional<BPThreadPool> TP;
#എങ്കിൽ LLVM_ENABLE_THREADS
  DefaultThreadPool TheThreadPool;
  എങ്കിൽ (Config.TaskSplitDepth > 1)
    TP.emplace(TheThreadPool);
#endif

  // Record the input order
  വേണ്ടി (unsigned I = 0; I < Nodes.size(); I++)
    Nodes[I].നിവേശം_Order_Index = I;

  auto NodesRange = llvm::make_range(Nodes.begin(), Nodes.end());
  auto BisectTask = [ഇത്, NodesRange, &TP]() {
    bisect(NodesRange, /*RecDepth=*/0, /*RootBucket=*/1, /*Offset=*/0, TP);
  };
  എങ്കിൽ (TP) {
    TP->async(std::move(BisectTask));
    TP->wait();
  } അല്ലെങ്കിൽ {
    BisectTask();
  }

  llvm::stable_sort(NodesRange, [](സ്ഥിരം auto &L, സ്ഥിരം auto &R) {
    തിരികെ L.Bucket < R.Bucket;
  });

  LLVM_DEBUG(dbgs() << "Balanced partitioning completed\n");
}

ശൂന്യം BalancedPartitioning::bisect(സ്ഥിരം Function_കണ്ണി_Range Nodes,
                                  unsigned RecDepth, unsigned വേര്_Bucket,
                                  unsigned Offset,
                                  std::optional<BPThreadPool> &TP) സ്ഥിരം {
  unsigned അക്കം_Nodes = llvm::size(Nodes);
  എങ്കിൽ (അക്കം_Nodes <= 1 || RecDepth >= Config.SplitDepth) {
    // We've reach the lowest level of the recursion tree. Fall back to the
    // original order and assign to buckets.
    llvm::sort(Nodes, [](സ്ഥിരം auto &L, സ്ഥിരം auto &R) {
      തിരികെ L.നിവേശം_Order_Index < R.നിവേശം_Order_Index;
    });
    വേണ്ടി (auto &N : Nodes)
      N.Bucket = Offset++;
    തിരികെ;
  }

  LLVM_DEBUG(dbgs() << format("Bisect with %d nodes and root bucket %d\n",
                              അക്കം_Nodes, വേര്_Bucket));

  std::mt19937 RNG(വേര്_Bucket);

  unsigned ഇടത്_Bucket = 2 * വേര്_Bucket;
  unsigned വലത്_Bucket = 2 * വേര്_Bucket + 1;

  // Split into two and assign to the left and right buckets
  split(Nodes, ഇടത്_Bucket);

  runIterations(Nodes, ഇടത്_Bucket, വലത്_Bucket, RNG);

  // Split nodes wrt the resulting buckets
  auto NodesMid =
      llvm::partition(Nodes, [&](auto &N) { തിരികെ N.Bucket == ഇടത്_Bucket; });
  unsigned MidOffset = Offset + std::distance(Nodes.begin(), NodesMid);

  auto ഇടത്_Nodes = llvm::make_range(Nodes.begin(), NodesMid);
  auto വലത്_Nodes = llvm::make_range(NodesMid, Nodes.end());

  auto ഇടത്_Rec_Task = [ഇത്, ഇടത്_Nodes, RecDepth, ഇടത്_Bucket, Offset, &TP]() {
    bisect(ഇടത്_Nodes, RecDepth + 1, ഇടത്_Bucket, Offset, TP);
  };
  auto വലത്_Rec_Task = [ഇത്, വലത്_Nodes, RecDepth, വലത്_Bucket, MidOffset,
                       &TP]() {
    bisect(വലത്_Nodes, RecDepth + 1, വലത്_Bucket, MidOffset, TP);
  };

  എങ്കിൽ (TP && RecDepth < Config.TaskSplitDepth && അക്കം_Nodes >= 4) {
    TP->async(std::move(ഇടത്_Rec_Task));
    TP->async(std::move(വലത്_Rec_Task));
  } അല്ലെങ്കിൽ {
    ഇടത്_Rec_Task();
    വലത്_Rec_Task();
  }
}

ശൂന്യം BalancedPartitioning::runIterations(സ്ഥിരം Function_കണ്ണി_Range Nodes,
                                         unsigned ഇടത്_Bucket,
                                         unsigned വലത്_Bucket,
                                         std::mt19937 &RNG) സ്ഥിരം {
  unsigned അക്കം_Nodes = llvm::size(Nodes);
  DenseMap<BPFunction_കണ്ണി::Utility_കണ്ണി_T, unsigned> Utility_കണ്ണി_Index;
  വേണ്ടി (auto &N : Nodes)
    വേണ്ടി (auto &UN : N.UtilityNodes)
      ++Utility_കണ്ണി_Index[UN];
  // Remove utility nodes if they have just one edge or are connected to all
  // functions
  വേണ്ടി (auto &N : Nodes)
    llvm::erase_if(N.UtilityNodes, [&](auto &UN) {
      unsigned UNI = Utility_കണ്ണി_Index[UN];
      തിരികെ UNI == 1 || UNI == അക്കം_Nodes;
    });

  // Renumber utility nodes so they can be used to index into Signatures
  Utility_കണ്ണി_Index.clear();
  വേണ്ടി (auto &N : Nodes)
    വേണ്ടി (auto &UN : N.UtilityNodes)
      UN = Utility_കണ്ണി_Index.തിരുകുക({UN, Utility_കണ്ണി_Index.size()}).first->second;

  // Initialize signatures
  SignaturesT Signatures(/*Size=*/Utility_കണ്ണി_Index.size());
  വേണ്ടി (auto &N : Nodes) {
    വേണ്ടി (auto &UN : N.UtilityNodes) {
      പ്രതിജ്ഞാ(UN < Signatures.size());
      എങ്കിൽ (N.Bucket == ഇടത്_Bucket) {
        Signatures[UN].ഇടത്_എണ്ണം++;
      } അല്ലെങ്കിൽ {
        Signatures[UN].വലത്_എണ്ണം++;
      }
    }
  }

  വേണ്ടി (unsigned I = 0; I < Config.IterationsPerSplit; I++) {
    unsigned അക്കം_Moved_Nodes =
        runIteration(Nodes, ഇടത്_Bucket, വലത്_Bucket, Signatures, RNG);
    എങ്കിൽ (അക്കം_Moved_Nodes == 0)
      നിർത്തുക;
  }
}

unsigned BalancedPartitioning::runIteration(സ്ഥിരം Function_കണ്ണി_Range Nodes,
                                            unsigned ഇടത്_Bucket,
                                            unsigned വലത്_Bucket,
                                            SignaturesT &Signatures,
                                            std::mt19937 &RNG) സ്ഥിരം {
  // Init signature cost caches
  വേണ്ടി (auto &Signature : Signatures) {
    എങ്കിൽ (Signature.CachedGainIsValid)
      തുടരുക;
    unsigned L = Signature.ഇടത്_എണ്ണം;
    unsigned R = Signature.വലത്_എണ്ണം;
    പ്രതിജ്ഞാ((L > 0 || R > 0) && "incorrect signature");
    ദശാംശം Cost = logCost(L, R);
    Signature.CachedGainLR = 0.f;
    Signature.CachedGainRL = 0.f;
    എങ്കിൽ (L > 0)
      Signature.CachedGainLR = Cost - logCost(L - 1, R + 1);
    എങ്കിൽ (R > 0)
      Signature.CachedGainRL = Cost - logCost(L + 1, R - 1);
    Signature.CachedGainIsValid = സത്യം;
  }

  // Compute move gains
  using GainPair = std::pair<ദശാംശം, BPFunction_കണ്ണി *>;
  std::vector<GainPair> Gains;
  വേണ്ടി (auto &N : Nodes) {
    തർക്കം From_ഇടത്_To_വലത് = (N.Bucket == ഇടത്_Bucket);
    ദശാംശം Gain = moveGain(N, From_ഇടത്_To_വലത്, Signatures);
    Gains.തള്ളുക_back(std::make_pair(Gain, &N));
  }

  // Collect left and right gains
  auto ഇടത്_End = llvm::partition(
      Gains, [&](സ്ഥിരം auto &GP) { തിരികെ GP.second->Bucket == ഇടത്_Bucket; });
  auto ഇടത്_Range = llvm::make_range(Gains.begin(), ഇടത്_End);
  auto വലത്_Range = llvm::make_range(ഇടത്_End, Gains.end());

  // Sort gains in descending order
  auto LargerGain = [](സ്ഥിരം auto &L, സ്ഥിരം auto &R) {
    തിരികെ L.first > R.first;
  };
  llvm::stable_sort(ഇടത്_Range, LargerGain);
  llvm::stable_sort(വലത്_Range, LargerGain);

  unsigned അക്കം_Moved_Data_Vertices = 0;
  വേണ്ടി (auto [ഇടത്_Pair, വലത്_Pair] : llvm::zip(ഇടത്_Range, വലത്_Range)) {
    auto &[ഇടത്_Gain, ഇടത്_കണ്ണി] = ഇടത്_Pair;
    auto &[വലത്_Gain, വലത്_കണ്ണി] = വലത്_Pair;
    // Stop when the gain is no longer beneficial
    എങ്കിൽ (ഇടത്_Gain + വലത്_Gain <= 0.f)
      നിർത്തുക;
    // Try to exchange the nodes between buckets
    എങ്കിൽ (move_Function_കണ്ണി(*ഇടത്_കണ്ണി, ഇടത്_Bucket, വലത്_Bucket, Signatures, RNG))
      ++അക്കം_Moved_Data_Vertices;
    എങ്കിൽ (move_Function_കണ്ണി(*വലത്_കണ്ണി, ഇടത്_Bucket, വലത്_Bucket, Signatures, RNG))
      ++അക്കം_Moved_Data_Vertices;
  }
  തിരികെ അക്കം_Moved_Data_Vertices;
}

തർക്കം BalancedPartitioning::move_Function_കണ്ണി(BPFunction_കണ്ണി &N,
                                            unsigned ഇടത്_Bucket,
                                            unsigned വലത്_Bucket,
                                            SignaturesT &Signatures,
                                            std::mt19937 &RNG) സ്ഥിരം {
  // Sometimes we skip the move. This helps to escape local optima
  എങ്കിൽ (std::uniform_real_distribution<ദശാംശം>(0.f, 1.f)(RNG) <=
      Config.SkipProbability)
    തിരികെ അസത്യം;

  തർക്കം From_ഇടത്_To_വലത് = (N.Bucket == ഇടത്_Bucket);
  // Update the current bucket
  N.Bucket = (From_ഇടത്_To_വലത് ? വലത്_Bucket : ഇടത്_Bucket);

  // Update signatures and invalidate gain cache
  എങ്കിൽ (From_ഇടത്_To_വലത്) {
    വേണ്ടി (auto &UN : N.UtilityNodes) {
      auto &Signature = Signatures[UN];
      Signature.ഇടത്_എണ്ണം--;
      Signature.വലത്_എണ്ണം++;
      Signature.CachedGainIsValid = അസത്യം;
    }
  } അല്ലെങ്കിൽ {
    വേണ്ടി (auto &UN : N.UtilityNodes) {
      auto &Signature = Signatures[UN];
      Signature.ഇടത്_എണ്ണം++;
      Signature.വലത്_എണ്ണം--;
      Signature.CachedGainIsValid = അസത്യം;
    }
  }
  തിരികെ സത്യം;
}

ശൂന്യം BalancedPartitioning::split(സ്ഥിരം Function_കണ്ണി_Range Nodes,
                                 unsigned StartBucket) സ്ഥിരം {
  unsigned അക്കം_Nodes = llvm::size(Nodes);
  auto NodesMid = Nodes.begin() + (അക്കം_Nodes + 1) / 2;

  llvm::sort(Nodes, [](auto &L, auto &R) {
    തിരികെ L.നിവേശം_Order_Index < R.നിവേശം_Order_Index;
  });

  വേണ്ടി (auto &N : llvm::make_range(Nodes.begin(), NodesMid))
    N.Bucket = StartBucket;
  വേണ്ടി (auto &N : llvm::make_range(NodesMid, Nodes.end()))
    N.Bucket = StartBucket + 1;
}

ദശാംശം BalancedPartitioning::moveGain(സ്ഥിരം BPFunction_കണ്ണി &N,
                                     തർക്കം From_ഇടത്_To_വലത്,
                                     സ്ഥിരം SignaturesT &Signatures) {
  ദശാംശം Gain = 0.f;
  വേണ്ടി (auto &UN : N.UtilityNodes)
    Gain += (From_ഇടത്_To_വലത് ? Signatures[UN].CachedGainLR
                             : Signatures[UN].CachedGainRL);
  തിരികെ Gain;
}

ദശാംശം BalancedPartitioning::logCost(unsigned X, unsigned Y) സ്ഥിരം {
  തിരികെ -(X * log2Cached(X + 1) + Y * log2Cached(Y + 1));
}

ദശാംശം BalancedPartitioning::log2Cached(unsigned i) സ്ഥിരം {
  തിരികെ (i < LOG_CACHE_SIZE) ? Log2Cache[i] : std::log2(i);
}
