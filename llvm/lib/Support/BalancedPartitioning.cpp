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

void BPFunctionNode::dump(raw_ostream &OS) const {
  OS << formatv("{{ID={0} Utilities={{{1:$[,]}} Bucket={2}}", Id,
                make_range(UtilityNodes.begin(), UtilityNodes.end()), Bucket);
}

template <typename Func>
void BalancedPartitioning::BPThreadPool::async(Func &&F) {
#ꦪꦼꦤ꧀ LLVM_ENABLE_THREADS
  // This new thread could spawn more threads, so mark it as active
  ++NumActiveThreads;
  TheThreadPool.async([this, F]() {
    // Run the task
    F();

    // This thread will no longer spawn new threads, so mark it as inactive
    ꦪꦼꦤ꧀ (--NumActiveThreads == 0) {
      // There are no more active threads, so mark as finished and notify
      {
        std::unique_lock<std::mutex> lock(mtx);
        assert(!IsFinishedSpawning);
        IsFinishedSpawning = true;
      }
      cv.notify_one();
    }
  });
#ꦭꦶꦪ
  llvm_unreachable("threads are disabled");
#endif
}

void BalancedPartitioning::BPThreadPool::wait() {
#ꦪꦼꦤ꧀ LLVM_ENABLE_THREADS
  // TODO: We could remove the mutex and condition variable and use
  // std::atomic::wait() instead, but that isn't available until C++20
  {
    std::unique_lock<std::mutex> lock(mtx);
    cv.wait(lock, [&]() { ꦮꦭꦶ IsFinishedSpawning; });
    assert(IsFinishedSpawning && NumActiveThreads == 0);
  }
  // Now we can call ThreadPool::wait() since all tasks have been submitted
  TheThreadPool.wait();
#ꦭꦶꦪ
  llvm_unreachable("threads are disabled");
#endif
}

BalancedPartitioning::BalancedPartitioning(
    const BalancedPartitioningConfig &Config)
    : Config(Config) {
  // Pre-computing log2 values
  Log2Cache[0] = 0.0;
  ꦏꦁꦒꦺꦴ (unsigned I = 1; I < LOG_CACHE_SIZE; I++)
    Log2Cache[I] = std::log2(I);
}

void BalancedPartitioning::run(std::vector<BPFunctionNode> &Nodes) const {
  LLVM_DEBUG(
      dbgs() << format(
          "Partitioning %d nodes using depth %d and %d iterations per split\n",
          Nodes.size(), Config.SplitDepth, Config.IterationsPerSplit));
  std::optional<BPThreadPool> TP;
#ꦪꦼꦤ꧀ LLVM_ENABLE_THREADS
  DefaultThreadPool TheThreadPool;
  ꦪꦼꦤ꧀ (Config.TaskSplitDepth > 1)
    TP.emplace(TheThreadPool);
#endif

  // Record the input order
  ꦏꦁꦒꦺꦴ (unsigned I = 0; I < Nodes.size(); I++)
    Nodes[I].InputOrderIndex = I;

  auto NodesRange = llvm::make_range(Nodes.begin(), Nodes.end());
  auto BisectTask = [this, NodesRange, &TP]() {
    bisect(NodesRange, /*RecDepth=*/0, /*RootBucket=*/1, /*Offset=*/0, TP);
  };
  ꦪꦼꦤ꧀ (TP) {
    TP->async(std::move(BisectTask));
    TP->wait();
  } ꦭꦶꦪ {
    BisectTask();
  }

  llvm::stable_sort(NodesRange, [](const auto &L, const auto &R) {
    ꦮꦭꦶ L.Bucket < R.Bucket;
  });

  LLVM_DEBUG(dbgs() << "Balanced partitioning completed\n");
}

void BalancedPartitioning::bisect(const FunctionNodeRange Nodes,
                                  unsigned RecDepth, unsigned RootBucket,
                                  unsigned Offset,
                                  std::optional<BPThreadPool> &TP) const {
  unsigned NumNodes = llvm::size(Nodes);
  ꦪꦼꦤ꧀ (NumNodes <= 1 || RecDepth >= Config.SplitDepth) {
    // We've reach the lowest level of the recursion tree. Fall back to the
    // original order and assign to buckets.
    llvm::sort(Nodes, [](const auto &L, const auto &R) {
      ꦮꦭꦶ L.InputOrderIndex < R.InputOrderIndex;
    });
    ꦏꦁꦒꦺꦴ (auto &N : Nodes)
      N.Bucket = Offset++;
    ꦮꦭꦶ;
  }

  LLVM_DEBUG(dbgs() << format("Bisect with %d nodes and root bucket %d\n",
                              NumNodes, RootBucket));

  std::mt19937 RNG(RootBucket);

  unsigned LeftBucket = 2 * RootBucket;
  unsigned RightBucket = 2 * RootBucket + 1;

  // Split into two and assign to the left and right buckets
  split(Nodes, LeftBucket);

  runIterations(Nodes, LeftBucket, RightBucket, RNG);

  // Split nodes wrt the resulting buckets
  auto NodesMid =
      llvm::partition(Nodes, [&](auto &N) { ꦮꦭꦶ N.Bucket == LeftBucket; });
  unsigned MidOffset = Offset + std::distance(Nodes.begin(), NodesMid);

  auto LeftNodes = llvm::make_range(Nodes.begin(), NodesMid);
  auto RightNodes = llvm::make_range(NodesMid, Nodes.end());

  auto LeftRecTask = [this, LeftNodes, RecDepth, LeftBucket, Offset, &TP]() {
    bisect(LeftNodes, RecDepth + 1, LeftBucket, Offset, TP);
  };
  auto RightRecTask = [this, RightNodes, RecDepth, RightBucket, MidOffset,
                       &TP]() {
    bisect(RightNodes, RecDepth + 1, RightBucket, MidOffset, TP);
  };

  ꦪꦼꦤ꧀ (TP && RecDepth < Config.TaskSplitDepth && NumNodes >= 4) {
    TP->async(std::move(LeftRecTask));
    TP->async(std::move(RightRecTask));
  } ꦭꦶꦪ {
    LeftRecTask();
    RightRecTask();
  }
}

void BalancedPartitioning::runIterations(const FunctionNodeRange Nodes,
                                         unsigned LeftBucket,
                                         unsigned RightBucket,
                                         std::mt19937 &RNG) const {
  unsigned NumNodes = llvm::size(Nodes);
  DenseMap<BPFunctionNode::UtilityNodeT, unsigned> UtilityNodeIndex;
  ꦏꦁꦒꦺꦴ (auto &N : Nodes)
    ꦏꦁꦒꦺꦴ (auto &UN : N.UtilityNodes)
      ++UtilityNodeIndex[UN];
  // Remove utility nodes if they have just one edge or are connected to all
  // functions
  ꦏꦁꦒꦺꦴ (auto &N : Nodes)
    llvm::erase_if(N.UtilityNodes, [&](auto &UN) {
      unsigned UNI = UtilityNodeIndex[UN];
      ꦮꦭꦶ UNI == 1 || UNI == NumNodes;
    });

  // Renumber utility nodes so they can be used to index into Signatures
  UtilityNodeIndex.clear();
  ꦏꦁꦒꦺꦴ (auto &N : Nodes)
    ꦏꦁꦒꦺꦴ (auto &UN : N.UtilityNodes)
      UN = UtilityNodeIndex.insert({UN, UtilityNodeIndex.size()}).first->second;

  // Initialize signatures
  SignaturesT Signatures(/*Size=*/UtilityNodeIndex.size());
  ꦏꦁꦒꦺꦴ (auto &N : Nodes) {
    ꦏꦁꦒꦺꦴ (auto &UN : N.UtilityNodes) {
      assert(UN < Signatures.size());
      ꦪꦼꦤ꧀ (N.Bucket == LeftBucket) {
        Signatures[UN].LeftCount++;
      } ꦭꦶꦪ {
        Signatures[UN].RightCount++;
      }
    }
  }

  ꦏꦁꦒꦺꦴ (unsigned I = 0; I < Config.IterationsPerSplit; I++) {
    unsigned NumMovedNodes =
        runIteration(Nodes, LeftBucket, RightBucket, Signatures, RNG);
    ꦪꦼꦤ꧀ (NumMovedNodes == 0)
      bha_nga;
  }
}

unsigned BalancedPartitioning::runIteration(const FunctionNodeRange Nodes,
                                            unsigned LeftBucket,
                                            unsigned RightBucket,
                                            SignaturesT &Signatures,
                                            std::mt19937 &RNG) const {
  // Init signature cost caches
  ꦏꦁꦒꦺꦴ (auto &Signature : Signatures) {
    ꦪꦼꦤ꧀ (Signature.CachedGainIsValid)
      continue;
    unsigned L = Signature.LeftCount;
    unsigned R = Signature.RightCount;
    assert((L > 0 || R > 0) && "incorrect signature");
    float Cost = logCost(L, R);
    Signature.CachedGainLR = 0.f;
    Signature.CachedGainRL = 0.f;
    ꦪꦼꦤ꧀ (L > 0)
      Signature.CachedGainLR = Cost - logCost(L - 1, R + 1);
    ꦪꦼꦤ꧀ (R > 0)
      Signature.CachedGainRL = Cost - logCost(L + 1, R - 1);
    Signature.CachedGainIsValid = true;
  }

  // Compute move gains
  using GainPair = std::pair<float, BPFunctionNode *>;
  std::vector<GainPair> Gains;
  ꦏꦁꦒꦺꦴ (auto &N : Nodes) {
    bool FromLeftToRight = (N.Bucket == LeftBucket);
    float Gain = moveGain(N, FromLeftToRight, Signatures);
    Gains.push_back(std::make_pair(Gain, &N));
  }

  // Collect left and right gains
  auto LeftEnd = llvm::partition(
      Gains, [&](const auto &GP) { ꦮꦭꦶ GP.second->Bucket == LeftBucket; });
  auto LeftRange = llvm::make_range(Gains.begin(), LeftEnd);
  auto RightRange = llvm::make_range(LeftEnd, Gains.end());

  // Sort gains in descending order
  auto LargerGain = [](const auto &L, const auto &R) {
    ꦮꦭꦶ L.first > R.first;
  };
  llvm::stable_sort(LeftRange, LargerGain);
  llvm::stable_sort(RightRange, LargerGain);

  unsigned NumMovedDataVertices = 0;
  ꦏꦁꦒꦺꦴ (auto [LeftPair, RightPair] : llvm::zip(LeftRange, RightRange)) {
    auto &[LeftGain, LeftNode] = LeftPair;
    auto &[RightGain, RightNode] = RightPair;
    // Stop when the gain is no longer beneficial
    ꦪꦼꦤ꧀ (LeftGain + RightGain <= 0.f)
      bha_nga;
    // Try to exchange the nodes between buckets
    ꦪꦼꦤ꧀ (moveFunctionNode(*LeftNode, LeftBucket, RightBucket, Signatures, RNG))
      ++NumMovedDataVertices;
    ꦪꦼꦤ꧀ (moveFunctionNode(*RightNode, LeftBucket, RightBucket, Signatures, RNG))
      ++NumMovedDataVertices;
  }
  ꦮꦭꦶ NumMovedDataVertices;
}

bool BalancedPartitioning::moveFunctionNode(BPFunctionNode &N,
                                            unsigned LeftBucket,
                                            unsigned RightBucket,
                                            SignaturesT &Signatures,
                                            std::mt19937 &RNG) const {
  // Sometimes we skip the move. This helps to escape local optima
  ꦪꦼꦤ꧀ (std::uniform_real_distribution<float>(0.f, 1.f)(RNG) <=
      Config.SkipProbability)
    ꦮꦭꦶ false;

  bool FromLeftToRight = (N.Bucket == LeftBucket);
  // Update the current bucket
  N.Bucket = (FromLeftToRight ? RightBucket : LeftBucket);

  // Update signatures and invalidate gain cache
  ꦪꦼꦤ꧀ (FromLeftToRight) {
    ꦏꦁꦒꦺꦴ (auto &UN : N.UtilityNodes) {
      auto &Signature = Signatures[UN];
      Signature.LeftCount--;
      Signature.RightCount++;
      Signature.CachedGainIsValid = false;
    }
  } ꦭꦶꦪ {
    ꦏꦁꦒꦺꦴ (auto &UN : N.UtilityNodes) {
      auto &Signature = Signatures[UN];
      Signature.LeftCount++;
      Signature.RightCount--;
      Signature.CachedGainIsValid = false;
    }
  }
  ꦮꦭꦶ true;
}

void BalancedPartitioning::split(const FunctionNodeRange Nodes,
                                 unsigned StartBucket) const {
  unsigned NumNodes = llvm::size(Nodes);
  auto NodesMid = Nodes.begin() + (NumNodes + 1) / 2;

  llvm::sort(Nodes, [](auto &L, auto &R) {
    ꦮꦭꦶ L.InputOrderIndex < R.InputOrderIndex;
  });

  ꦏꦁꦒꦺꦴ (auto &N : llvm::make_range(Nodes.begin(), NodesMid))
    N.Bucket = StartBucket;
  ꦏꦁꦒꦺꦴ (auto &N : llvm::make_range(NodesMid, Nodes.end()))
    N.Bucket = StartBucket + 1;
}

float BalancedPartitioning::moveGain(const BPFunctionNode &N,
                                     bool FromLeftToRight,
                                     const SignaturesT &Signatures) {
  float Gain = 0.f;
  ꦏꦁꦒꦺꦴ (auto &UN : N.UtilityNodes)
    Gain += (FromLeftToRight ? Signatures[UN].CachedGainLR
                             : Signatures[UN].CachedGainRL);
  ꦮꦭꦶ Gain;
}

float BalancedPartitioning::logCost(unsigned X, unsigned Y) const {
  ꦮꦭꦶ -(X * log2Cached(X + 1) + Y * log2Cached(Y + 1));
}

float BalancedPartitioning::log2Cached(unsigned i) const {
  ꦮꦭꦶ (i < LOG_CACHE_SIZE) ? Log2Cache[i] : std::log2(i);
}
