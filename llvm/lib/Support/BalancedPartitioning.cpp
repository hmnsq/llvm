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

#અન્તર્ભાવ "llvm/Support/BalancedPartitioning.h"
#અન્તર્ભાવ "llvm/Config/llvm-config.h" // for LLVM_ENABLE_THREADS
#અન્તર્ભાવ "llvm/Support/Debug.h"
#અન્તર્ભાવ "llvm/Support/Format.h"
#અન્તર્ભાવ "llvm/Support/FormatVariadic.h"
#અન્તર્ભાવ "llvm/Support/ThreadPool.h"

#અન્તર્ભાવ <cmath>

using નામાકાશ llvm;
#define DEBUG_TYPE "balanced-partitioning"

શૂન્ય BPFunction_ગાંઠ::dump(raw_ostream &OS) અચલ {
  OS << formatv("{{ID={0} Utilities={{{1:$[,]}} Bucket={2}}", Id,
                make_range(UtilityNodes.begin(), UtilityNodes.end()), Bucket);
}

template <typename Func>
શૂન્ય BalancedPartitioning::BPThreadPool::async(Func &&F) {
#જો LLVM_ENABLE_THREADS
  // This new thread could spawn more threads, so mark it as active
  ++અંક_Active_Threads;
  TheThreadPool.async([આ, F]() {
    // Run the task
    F();

    // This thread will no longer spawn new threads, so mark it as inactive
    જો (--અંક_Active_Threads == 0) {
      // There are no more active threads, so mark as finished and notify
      {
        std::unique_lock<std::mutex> lock(mtx);
        પ્રતિજ્ઞા(!IsFinishedSpawning);
        IsFinishedSpawning = સાચું;
      }
      cv.notify_one();
    }
  });
#નહીંતર
  llvm_unreachable("threads are disabled");
#endif
}

શૂન્ય BalancedPartitioning::BPThreadPool::wait() {
#જો LLVM_ENABLE_THREADS
  // TODO: We could remove the mutex and condition variable and use
  // std::atomic::wait() instead, but that isn't available until C++20
  {
    std::unique_lock<std::mutex> lock(mtx);
    cv.wait(lock, [&]() { પાછા IsFinishedSpawning; });
    પ્રતિજ્ઞા(IsFinishedSpawning && અંક_Active_Threads == 0);
  }
  // Now we can call ThreadPool::wait() since all tasks have been submitted
  TheThreadPool.wait();
#નહીંતર
  llvm_unreachable("threads are disabled");
#endif
}

BalancedPartitioning::BalancedPartitioning(
    અચલ BalancedPartitioningConfig &Config)
    : Config(Config) {
  // Pre-computing log2 values
  Log2Cache[0] = 0.0;
  માટે (unsigned I = 1; I < LOG_CACHE_SIZE; I++)
    Log2Cache[I] = std::log2(I);
}

શૂન્ય BalancedPartitioning::run(std::vector<BPFunction_ગાંઠ> &Nodes) અચલ {
  LLVM_DEBUG(
      dbgs() << format(
          "Partitioning %d nodes using depth %d and %d iterations per split\n",
          Nodes.size(), Config.SplitDepth, Config.IterationsPerSplit));
  std::optional<BPThreadPool> TP;
#જો LLVM_ENABLE_THREADS
  DefaultThreadPool TheThreadPool;
  જો (Config.TaskSplitDepth > 1)
    TP.emplace(TheThreadPool);
#endif

  // Record the input order
  માટે (unsigned I = 0; I < Nodes.size(); I++)
    Nodes[I].આગત_Order_Index = I;

  auto NodesRange = llvm::make_range(Nodes.begin(), Nodes.end());
  auto BisectTask = [આ, NodesRange, &TP]() {
    bisect(NodesRange, /*RecDepth=*/0, /*RootBucket=*/1, /*Offset=*/0, TP);
  };
  જો (TP) {
    TP->async(std::move(BisectTask));
    TP->wait();
  } નહીંતર {
    BisectTask();
  }

  llvm::stable_sort(NodesRange, [](અચલ auto &L, અચલ auto &R) {
    પાછા L.Bucket < R.Bucket;
  });

  LLVM_DEBUG(dbgs() << "Balanced partitioning completed\n");
}

શૂન્ય BalancedPartitioning::bisect(અચલ Function_ગાંઠ_Range Nodes,
                                  unsigned RecDepth, unsigned મૂળ_Bucket,
                                  unsigned Offset,
                                  std::optional<BPThreadPool> &TP) અચલ {
  unsigned અંક_Nodes = llvm::size(Nodes);
  જો (અંક_Nodes <= 1 || RecDepth >= Config.SplitDepth) {
    // We've reach the lowest level of the recursion tree. Fall back to the
    // original order and assign to buckets.
    llvm::sort(Nodes, [](અચલ auto &L, અચલ auto &R) {
      પાછા L.આગત_Order_Index < R.આગત_Order_Index;
    });
    માટે (auto &N : Nodes)
      N.Bucket = Offset++;
    પાછા;
  }

  LLVM_DEBUG(dbgs() << format("Bisect with %d nodes and root bucket %d\n",
                              અંક_Nodes, મૂળ_Bucket));

  std::mt19937 RNG(મૂળ_Bucket);

  unsigned ડાબું_Bucket = 2 * મૂળ_Bucket;
  unsigned જમણું_Bucket = 2 * મૂળ_Bucket + 1;

  // Split into two and assign to the left and right buckets
  split(Nodes, ડાબું_Bucket);

  runIterations(Nodes, ડાબું_Bucket, જમણું_Bucket, RNG);

  // Split nodes wrt the resulting buckets
  auto NodesMid =
      llvm::partition(Nodes, [&](auto &N) { પાછા N.Bucket == ડાબું_Bucket; });
  unsigned MidOffset = Offset + std::distance(Nodes.begin(), NodesMid);

  auto ડાબું_Nodes = llvm::make_range(Nodes.begin(), NodesMid);
  auto જમણું_Nodes = llvm::make_range(NodesMid, Nodes.end());

  auto ડાબું_Rec_Task = [આ, ડાબું_Nodes, RecDepth, ડાબું_Bucket, Offset, &TP]() {
    bisect(ડાબું_Nodes, RecDepth + 1, ડાબું_Bucket, Offset, TP);
  };
  auto જમણું_Rec_Task = [આ, જમણું_Nodes, RecDepth, જમણું_Bucket, MidOffset,
                       &TP]() {
    bisect(જમણું_Nodes, RecDepth + 1, જમણું_Bucket, MidOffset, TP);
  };

  જો (TP && RecDepth < Config.TaskSplitDepth && અંક_Nodes >= 4) {
    TP->async(std::move(ડાબું_Rec_Task));
    TP->async(std::move(જમણું_Rec_Task));
  } નહીંતર {
    ડાબું_Rec_Task();
    જમણું_Rec_Task();
  }
}

શૂન્ય BalancedPartitioning::runIterations(અચલ Function_ગાંઠ_Range Nodes,
                                         unsigned ડાબું_Bucket,
                                         unsigned જમણું_Bucket,
                                         std::mt19937 &RNG) અચલ {
  unsigned અંક_Nodes = llvm::size(Nodes);
  DenseMap<BPFunction_ગાંઠ::Utility_ગાંઠ_T, unsigned> Utility_ગાંઠ_Index;
  માટે (auto &N : Nodes)
    માટે (auto &UN : N.UtilityNodes)
      ++Utility_ગાંઠ_Index[UN];
  // Remove utility nodes if they have just one edge or are connected to all
  // functions
  માટે (auto &N : Nodes)
    llvm::erase_if(N.UtilityNodes, [&](auto &UN) {
      unsigned UNI = Utility_ગાંઠ_Index[UN];
      પાછા UNI == 1 || UNI == અંક_Nodes;
    });

  // Renumber utility nodes so they can be used to index into Signatures
  Utility_ગાંઠ_Index.clear();
  માટે (auto &N : Nodes)
    માટે (auto &UN : N.UtilityNodes)
      UN = Utility_ગાંઠ_Index.ઉમેરો({UN, Utility_ગાંઠ_Index.size()}).first->second;

  // Initialize signatures
  SignaturesT Signatures(/*Size=*/Utility_ગાંઠ_Index.size());
  માટે (auto &N : Nodes) {
    માટે (auto &UN : N.UtilityNodes) {
      પ્રતિજ્ઞા(UN < Signatures.size());
      જો (N.Bucket == ડાબું_Bucket) {
        Signatures[UN].ડાબું_ગણતરી++;
      } નહીંતર {
        Signatures[UN].જમણું_ગણતરી++;
      }
    }
  }

  માટે (unsigned I = 0; I < Config.IterationsPerSplit; I++) {
    unsigned અંક_Moved_Nodes =
        runIteration(Nodes, ડાબું_Bucket, જમણું_Bucket, Signatures, RNG);
    જો (અંક_Moved_Nodes == 0)
      તોડો;
  }
}

unsigned BalancedPartitioning::runIteration(અચલ Function_ગાંઠ_Range Nodes,
                                            unsigned ડાબું_Bucket,
                                            unsigned જમણું_Bucket,
                                            SignaturesT &Signatures,
                                            std::mt19937 &RNG) અચલ {
  // Init signature cost caches
  માટે (auto &Signature : Signatures) {
    જો (Signature.CachedGainIsValid)
      ચાલુ;
    unsigned L = Signature.ડાબું_ગણતરી;
    unsigned R = Signature.જમણું_ગણતરી;
    પ્રતિજ્ઞા((L > 0 || R > 0) && "incorrect signature");
    દશાંશ Cost = logCost(L, R);
    Signature.CachedGainLR = 0.f;
    Signature.CachedGainRL = 0.f;
    જો (L > 0)
      Signature.CachedGainLR = Cost - logCost(L - 1, R + 1);
    જો (R > 0)
      Signature.CachedGainRL = Cost - logCost(L + 1, R - 1);
    Signature.CachedGainIsValid = સાચું;
  }

  // Compute move gains
  using GainPair = std::pair<દશાંશ, BPFunction_ગાંઠ *>;
  std::vector<GainPair> Gains;
  માટે (auto &N : Nodes) {
    તાર્કિક From_ડાબું_To_જમણું = (N.Bucket == ડાબું_Bucket);
    દશાંશ Gain = moveGain(N, From_ડાબું_To_જમણું, Signatures);
    Gains.ધકેલો_back(std::make_pair(Gain, &N));
  }

  // Collect left and right gains
  auto ડાબું_End = llvm::partition(
      Gains, [&](અચલ auto &GP) { પાછા GP.second->Bucket == ડાબું_Bucket; });
  auto ડાબું_Range = llvm::make_range(Gains.begin(), ડાબું_End);
  auto જમણું_Range = llvm::make_range(ડાબું_End, Gains.end());

  // Sort gains in descending order
  auto LargerGain = [](અચલ auto &L, અચલ auto &R) {
    પાછા L.first > R.first;
  };
  llvm::stable_sort(ડાબું_Range, LargerGain);
  llvm::stable_sort(જમણું_Range, LargerGain);

  unsigned અંક_Moved_Data_Vertices = 0;
  માટે (auto [ડાબું_Pair, જમણું_Pair] : llvm::zip(ડાબું_Range, જમણું_Range)) {
    auto &[ડાબું_Gain, ડાબું_ગાંઠ] = ડાબું_Pair;
    auto &[જમણું_Gain, જમણું_ગાંઠ] = જમણું_Pair;
    // Stop when the gain is no longer beneficial
    જો (ડાબું_Gain + જમણું_Gain <= 0.f)
      તોડો;
    // Try to exchange the nodes between buckets
    જો (move_Function_ગાંઠ(*ડાબું_ગાંઠ, ડાબું_Bucket, જમણું_Bucket, Signatures, RNG))
      ++અંક_Moved_Data_Vertices;
    જો (move_Function_ગાંઠ(*જમણું_ગાંઠ, ડાબું_Bucket, જમણું_Bucket, Signatures, RNG))
      ++અંક_Moved_Data_Vertices;
  }
  પાછા અંક_Moved_Data_Vertices;
}

તાર્કિક BalancedPartitioning::move_Function_ગાંઠ(BPFunction_ગાંઠ &N,
                                            unsigned ડાબું_Bucket,
                                            unsigned જમણું_Bucket,
                                            SignaturesT &Signatures,
                                            std::mt19937 &RNG) અચલ {
  // Sometimes we skip the move. This helps to escape local optima
  જો (std::uniform_real_distribution<દશાંશ>(0.f, 1.f)(RNG) <=
      Config.SkipProbability)
    પાછા ખોટું;

  તાર્કિક From_ડાબું_To_જમણું = (N.Bucket == ડાબું_Bucket);
  // Update the current bucket
  N.Bucket = (From_ડાબું_To_જમણું ? જમણું_Bucket : ડાબું_Bucket);

  // Update signatures and invalidate gain cache
  જો (From_ડાબું_To_જમણું) {
    માટે (auto &UN : N.UtilityNodes) {
      auto &Signature = Signatures[UN];
      Signature.ડાબું_ગણતરી--;
      Signature.જમણું_ગણતરી++;
      Signature.CachedGainIsValid = ખોટું;
    }
  } નહીંતર {
    માટે (auto &UN : N.UtilityNodes) {
      auto &Signature = Signatures[UN];
      Signature.ડાબું_ગણતરી++;
      Signature.જમણું_ગણતરી--;
      Signature.CachedGainIsValid = ખોટું;
    }
  }
  પાછા સાચું;
}

શૂન્ય BalancedPartitioning::split(અચલ Function_ગાંઠ_Range Nodes,
                                 unsigned StartBucket) અચલ {
  unsigned અંક_Nodes = llvm::size(Nodes);
  auto NodesMid = Nodes.begin() + (અંક_Nodes + 1) / 2;

  llvm::sort(Nodes, [](auto &L, auto &R) {
    પાછા L.આગત_Order_Index < R.આગત_Order_Index;
  });

  માટે (auto &N : llvm::make_range(Nodes.begin(), NodesMid))
    N.Bucket = StartBucket;
  માટે (auto &N : llvm::make_range(NodesMid, Nodes.end()))
    N.Bucket = StartBucket + 1;
}

દશાંશ BalancedPartitioning::moveGain(અચલ BPFunction_ગાંઠ &N,
                                     તાર્કિક From_ડાબું_To_જમણું,
                                     અચલ SignaturesT &Signatures) {
  દશાંશ Gain = 0.f;
  માટે (auto &UN : N.UtilityNodes)
    Gain += (From_ડાબું_To_જમણું ? Signatures[UN].CachedGainLR
                             : Signatures[UN].CachedGainRL);
  પાછા Gain;
}

દશાંશ BalancedPartitioning::logCost(unsigned X, unsigned Y) અચલ {
  પાછા -(X * log2Cached(X + 1) + Y * log2Cached(Y + 1));
}

દશાંશ BalancedPartitioning::log2Cached(unsigned i) અચલ {
  પાછા (i < LOG_CACHE_SIZE) ? Log2Cache[i] : std::log2(i);
}
