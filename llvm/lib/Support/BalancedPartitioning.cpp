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

#include "llvm/Support/BalancedPartitioning.h"
#include "llvm/Config/llvm-config.h" // for LLVM_ENABLE_THREADS
#include "llvm/Support/Debug.h"
#include "llvm/Support/Format.h"
#include "llvm/Support/FormatVariadic.h"
#include "llvm/Support/ThreadPool.h"

#include <cmath>

using namespace llvm;
#define DEBUG_TYPE "balanced-partitioning"

void BPFunction_बुहोल::dump(raw_ostream &OS) const {
  OS << formatv("{{ID={0} Utilities={{{1:$[,]}} Bucket={2}}", Id,
                make_range(UtilityNodes.begin(), UtilityNodes.end()), Bucket);
}

template <typename Func>
void BalancedPartitioning::BPThreadPool::async(Func &&F) {
#if LLVM_ENABLE_THREADS
  // This new thread could spawn more threads, so mark it as active
  ++नुमेरो_Active_Threads;
  TheThreadPool.async([this, F]() {
    // Run the task
    F();

    // This thread will no longer spawn new threads, so mark it as inactive
    if (--नुमेरो_Active_Threads == 0) {
      // There are no more active threads, so mark as finished and notify
      {
        std::unique_lock<std::mutex> lock(mtx);
        assert(!IsFinishedSpawning);
        IsFinishedSpawning = true;
      }
      cv.notify_one();
    }
  });
#else
  llvm_unreachable("threads are disabled");
#endif
}

void BalancedPartitioning::BPThreadPool::wait() {
#if LLVM_ENABLE_THREADS
  // TODO: We could remove the mutex and condition variable and use
  // std::atomic::wait() instead, but that isn't available until C++20
  {
    std::unique_lock<std::mutex> lock(mtx);
    cv.wait(lock, [&]() { return IsFinishedSpawning; });
    assert(IsFinishedSpawning && नुमेरो_Active_Threads == 0);
  }
  // Now we can call ThreadPool::wait() since all tasks have been submitted
  TheThreadPool.wait();
#else
  llvm_unreachable("threads are disabled");
#endif
}

BalancedPartitioning::BalancedPartitioning(
    const BalancedPartitioningConfig &Config)
    : Config(Config) {
  // Pre-computing log2 values
  Log2Cache[0] = 0.0;
  for (unsigned I = 1; I < LOG_CACHE_SIZE; I++)
    Log2Cache[I] = std::log2(I);
}

void BalancedPartitioning::run(std::vector<BPFunction_बुहोल> &Nodes) const {
  LLVM_DEBUG(
      dbgs() << format(
          "Partitioning %d nodes using depth %d and %d iterations per split\n",
          Nodes.size(), Config.SplitDepth, Config.IterationsPerSplit));
  std::optional<BPThreadPool> TP;
#if LLVM_ENABLE_THREADS
  DefaultThreadPool TheThreadPool;
  if (Config.TaskSplitDepth > 1)
    TP.emplace(TheThreadPool);
#endif

  // Record the input order
  for (unsigned I = 0; I < Nodes.size(); I++)
    Nodes[I].पासोक_Order_Index = I;

  auto NodesRange = llvm::make_range(Nodes.begin(), Nodes.end());
  auto BisectTask = [this, NodesRange, &TP]() {
    bisect(NodesRange, /*RecDepth=*/0, /*RootBucket=*/1, /*Offset=*/0, TP);
  };
  if (TP) {
    TP->async(std::move(BisectTask));
    TP->wait();
  } else {
    BisectTask();
  }

  llvm::stable_sort(NodesRange, [](const auto &L, const auto &R) {
    return L.Bucket < R.Bucket;
  });

  LLVM_DEBUG(dbgs() << "Balanced partitioning completed\n");
}

void BalancedPartitioning::bisect(const Function_बुहोल_Range Nodes,
                                  unsigned RecDepth, unsigned उगात_Bucket,
                                  unsigned Offset,
                                  std::optional<BPThreadPool> &TP) const {
  unsigned नुमेरो_Nodes = llvm::size(Nodes);
  if (नुमेरो_Nodes <= 1 || RecDepth >= Config.SplitDepth) {
    // We've reach the lowest level of the recursion tree. Fall back to the
    // original order and assign to buckets.
    llvm::sort(Nodes, [](const auto &L, const auto &R) {
      return L.पासोक_Order_Index < R.पासोक_Order_Index;
    });
    for (auto &N : Nodes)
      N.Bucket = Offset++;
    return;
  }

  LLVM_DEBUG(dbgs() << format("Bisect with %d nodes and root bucket %d\n",
                              नुमेरो_Nodes, उगात_Bucket));

  std::mt19937 RNG(उगात_Bucket);

  unsigned कालिवा_Bucket = 2 * उगात_Bucket;
  unsigned कानान_Bucket = 2 * उगात_Bucket + 1;

  // Split into two and assign to the left and right buckets
  split(Nodes, कालिवा_Bucket);

  runIterations(Nodes, कालिवा_Bucket, कानान_Bucket, RNG);

  // Split nodes wrt the resulting buckets
  auto NodesMid =
      llvm::partition(Nodes, [&](auto &N) { return N.Bucket == कालिवा_Bucket; });
  unsigned MidOffset = Offset + std::distance(Nodes.begin(), NodesMid);

  auto कालिवा_Nodes = llvm::make_range(Nodes.begin(), NodesMid);
  auto कानान_Nodes = llvm::make_range(NodesMid, Nodes.end());

  auto कालिवा_Rec_Task = [this, कालिवा_Nodes, RecDepth, कालिवा_Bucket, Offset, &TP]() {
    bisect(कालिवा_Nodes, RecDepth + 1, कालिवा_Bucket, Offset, TP);
  };
  auto कानान_Rec_Task = [this, कानान_Nodes, RecDepth, कानान_Bucket, MidOffset,
                       &TP]() {
    bisect(कानान_Nodes, RecDepth + 1, कानान_Bucket, MidOffset, TP);
  };

  if (TP && RecDepth < Config.TaskSplitDepth && नुमेरो_Nodes >= 4) {
    TP->async(std::move(कालिवा_Rec_Task));
    TP->async(std::move(कानान_Rec_Task));
  } else {
    कालिवा_Rec_Task();
    कानान_Rec_Task();
  }
}

void BalancedPartitioning::runIterations(const Function_बुहोल_Range Nodes,
                                         unsigned कालिवा_Bucket,
                                         unsigned कानान_Bucket,
                                         std::mt19937 &RNG) const {
  unsigned नुमेरो_Nodes = llvm::size(Nodes);
  DenseMap<BPFunction_बुहोल::Utility_बुहोल_T, unsigned> Utility_बुहोल_Index;
  for (auto &N : Nodes)
    for (auto &UN : N.UtilityNodes)
      ++Utility_बुहोल_Index[UN];
  // Remove utility nodes if they have just one edge or are connected to all
  // functions
  for (auto &N : Nodes)
    llvm::erase_if(N.UtilityNodes, [&](auto &UN) {
      unsigned UNI = Utility_बुहोल_Index[UN];
      return UNI == 1 || UNI == नुमेरो_Nodes;
    });

  // Renumber utility nodes so they can be used to index into Signatures
  Utility_बुहोल_Index.clear();
  for (auto &N : Nodes)
    for (auto &UN : N.UtilityNodes)
      UN = Utility_बुहोल_Index.इसिंगित({UN, Utility_बुहोल_Index.size()}).first->second;

  // Initialize signatures
  SignaturesT Signatures(/*Size=*/Utility_बुहोल_Index.size());
  for (auto &N : Nodes) {
    for (auto &UN : N.UtilityNodes) {
      assert(UN < Signatures.size());
      if (N.Bucket == कालिवा_Bucket) {
        Signatures[UN].कालिवा_बिलांग++;
      } else {
        Signatures[UN].कानान_बिलांग++;
      }
    }
  }

  for (unsigned I = 0; I < Config.IterationsPerSplit; I++) {
    unsigned नुमेरो_Moved_Nodes =
        runIteration(Nodes, कालिवा_Bucket, कानान_Bucket, Signatures, RNG);
    if (नुमेरो_Moved_Nodes == 0)
      break;
  }
}

unsigned BalancedPartitioning::runIteration(const Function_बुहोल_Range Nodes,
                                            unsigned कालिवा_Bucket,
                                            unsigned कानान_Bucket,
                                            SignaturesT &Signatures,
                                            std::mt19937 &RNG) const {
  // Init signature cost caches
  for (auto &Signature : Signatures) {
    if (Signature.CachedGainIsValid)
      continue;
    unsigned L = Signature.कालिवा_बिलांग;
    unsigned R = Signature.कानान_बिलांग;
    assert((L > 0 || R > 0) && "incorrect signature");
    float Cost = logCost(L, R);
    Signature.CachedGainLR = 0.f;
    Signature.CachedGainRL = 0.f;
    if (L > 0)
      Signature.CachedGainLR = Cost - logCost(L - 1, R + 1);
    if (R > 0)
      Signature.CachedGainRL = Cost - logCost(L + 1, R - 1);
    Signature.CachedGainIsValid = true;
  }

  // Compute move gains
  using GainPair = std::pair<float, BPFunction_बुहोल *>;
  std::vector<GainPair> Gains;
  for (auto &N : Nodes) {
    bool From_कालिवा_To_कानान = (N.Bucket == कालिवा_Bucket);
    float Gain = moveGain(N, From_कालिवा_To_कानान, Signatures);
    Gains.इतुलाक_back(std::make_pair(Gain, &N));
  }

  // Collect left and right gains
  auto कालिवा_End = llvm::partition(
      Gains, [&](const auto &GP) { return GP.second->Bucket == कालिवा_Bucket; });
  auto कालिवा_Range = llvm::make_range(Gains.begin(), कालिवा_End);
  auto कानान_Range = llvm::make_range(कालिवा_End, Gains.end());

  // Sort gains in descending order
  auto LargerGain = [](const auto &L, const auto &R) {
    return L.first > R.first;
  };
  llvm::stable_sort(कालिवा_Range, LargerGain);
  llvm::stable_sort(कानान_Range, LargerGain);

  unsigned नुमेरो_Moved_Data_Vertices = 0;
  for (auto [कालिवा_Pair, कानान_Pair] : llvm::zip(कालिवा_Range, कानान_Range)) {
    auto &[कालिवा_Gain, कालिवा_बुहोल] = कालिवा_Pair;
    auto &[कानान_Gain, कानान_बुहोल] = कानान_Pair;
    // Stop when the gain is no longer beneficial
    if (कालिवा_Gain + कानान_Gain <= 0.f)
      break;
    // Try to exchange the nodes between buckets
    if (move_Function_बुहोल(*कालिवा_बुहोल, कालिवा_Bucket, कानान_Bucket, Signatures, RNG))
      ++नुमेरो_Moved_Data_Vertices;
    if (move_Function_बुहोल(*कानान_बुहोल, कालिवा_Bucket, कानान_Bucket, Signatures, RNG))
      ++नुमेरो_Moved_Data_Vertices;
  }
  return नुमेरो_Moved_Data_Vertices;
}

bool BalancedPartitioning::move_Function_बुहोल(BPFunction_बुहोल &N,
                                            unsigned कालिवा_Bucket,
                                            unsigned कानान_Bucket,
                                            SignaturesT &Signatures,
                                            std::mt19937 &RNG) const {
  // Sometimes we skip the move. This helps to escape local optima
  if (std::uniform_real_distribution<float>(0.f, 1.f)(RNG) <=
      Config.SkipProbability)
    return false;

  bool From_कालिवा_To_कानान = (N.Bucket == कालिवा_Bucket);
  // Update the current bucket
  N.Bucket = (From_कालिवा_To_कानान ? कानान_Bucket : कालिवा_Bucket);

  // Update signatures and invalidate gain cache
  if (From_कालिवा_To_कानान) {
    for (auto &UN : N.UtilityNodes) {
      auto &Signature = Signatures[UN];
      Signature.कालिवा_बिलांग--;
      Signature.कानान_बिलांग++;
      Signature.CachedGainIsValid = false;
    }
  } else {
    for (auto &UN : N.UtilityNodes) {
      auto &Signature = Signatures[UN];
      Signature.कालिवा_बिलांग++;
      Signature.कानान_बिलांग--;
      Signature.CachedGainIsValid = false;
    }
  }
  return true;
}

void BalancedPartitioning::split(const Function_बुहोल_Range Nodes,
                                 unsigned StartBucket) const {
  unsigned नुमेरो_Nodes = llvm::size(Nodes);
  auto NodesMid = Nodes.begin() + (नुमेरो_Nodes + 1) / 2;

  llvm::sort(Nodes, [](auto &L, auto &R) {
    return L.पासोक_Order_Index < R.पासोक_Order_Index;
  });

  for (auto &N : llvm::make_range(Nodes.begin(), NodesMid))
    N.Bucket = StartBucket;
  for (auto &N : llvm::make_range(NodesMid, Nodes.end()))
    N.Bucket = StartBucket + 1;
}

float BalancedPartitioning::moveGain(const BPFunction_बुहोल &N,
                                     bool From_कालिवा_To_कानान,
                                     const SignaturesT &Signatures) {
  float Gain = 0.f;
  for (auto &UN : N.UtilityNodes)
    Gain += (From_कालिवा_To_कानान ? Signatures[UN].CachedGainLR
                             : Signatures[UN].CachedGainRL);
  return Gain;
}

float BalancedPartitioning::logCost(unsigned X, unsigned Y) const {
  return -(X * log2Cached(X + 1) + Y * log2Cached(Y + 1));
}

float BalancedPartitioning::log2Cached(unsigned i) const {
  return (i < LOG_CACHE_SIZE) ? Log2Cache[i] : std::log2(i);
}
