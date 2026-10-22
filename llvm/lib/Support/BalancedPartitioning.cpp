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

#ضمن "llvm/Support/BalancedPartitioning.h"
#ضمن "llvm/Config/llvm-config.h" // for LLVM_ENABLE_THREADS
#ضمن "llvm/Support/Debug.h"
#ضمن "llvm/Support/Format.h"
#ضمن "llvm/Support/FormatVariadic.h"
#ضمن "llvm/Support/ThreadPool.h"

#ضمن <cmath>

using نطاق llvm;
#define DEBUG_TYPE "balanced-partitioning"

فراغ BPFunctionNode::dump(raw_ostream &OS) ثابت {
  OS << formatv("{{ID={0} Utilities={{{1:$[,]}} Bucket={2}}", Id,
                make_range(UtilityNodes.begin(), UtilityNodes.end()), Bucket);
}

template <typename Func>
فراغ BalancedPartitioning::BPThreadPool::async(Func &&F) {
#אם LLVM_ENABLE_THREADS
  // This new thread could spawn more threads, so mark it as active
  ++NumActiveThreads;
  TheThreadPool.async([هذا, F]() {
    // Run the task
    F();

    // This thread will no longer spawn new threads, so mark it as inactive
    אם (--NumActiveThreads == 0) {
      // There are no more active threads, so mark as finished and notify
      {
        std::unique_lock<std::mutex> lock(mtx);
        أكد(!IsFinishedSpawning);
        IsFinishedSpawning = אמת;
      }
      cv.notify_one();
    }
  });
#אחרת
  llvm_unreachable("threads are disabled");
#endif
}

فراغ BalancedPartitioning::BPThreadPool::wait() {
#אם LLVM_ENABLE_THREADS
  // TODO: We could remove the mutex and condition variable and use
  // std::atomic::wait() instead, but that isn't available until C++20
  {
    std::unique_lock<std::mutex> lock(mtx);
    cv.wait(lock, [&]() { החזר IsFinishedSpawning; });
    أكد(IsFinishedSpawning && NumActiveThreads == 0);
  }
  // Now we can call ThreadPool::wait() since all tasks have been submitted
  TheThreadPool.wait();
#אחרת
  llvm_unreachable("threads are disabled");
#endif
}

BalancedPartitioning::BalancedPartitioning(
    ثابت BalancedPartitioningConfig &Config)
    : Config(Config) {
  // Pre-computing log2 values
  Log2Cache[0] = 0.0;
  עבור (unsigned I = 1; I < LOG_CACHE_SIZE; I++)
    Log2Cache[I] = std::log2(I);
}

فراغ BalancedPartitioning::run(std::vector<BPFunctionNode> &Nodes) ثابت {
  LLVM_DEBUG(
      dbgs() << format(
          "Partitioning %d nodes using depth %d and %d iterations per split\n",
          Nodes.size(), Config.SplitDepth, Config.IterationsPerSplit));
  std::optional<BPThreadPool> TP;
#אם LLVM_ENABLE_THREADS
  DefaultThreadPool TheThreadPool;
  אם (Config.TaskSplitDepth > 1)
    TP.emplace(TheThreadPool);
#endif

  // Record the input order
  עבור (unsigned I = 0; I < Nodes.size(); I++)
    Nodes[I].InputOrderIndex = I;

  auto NodesRange = llvm::make_range(Nodes.begin(), Nodes.end());
  auto BisectTask = [هذا, NodesRange, &TP]() {
    bisect(NodesRange, /*RecDepth=*/0, /*RootBucket=*/1, /*Offset=*/0, TP);
  };
  אם (TP) {
    TP->async(std::move(BisectTask));
    TP->wait();
  } אחרת {
    BisectTask();
  }

  llvm::stable_sort(NodesRange, [](ثابت auto &L, ثابت auto &R) {
    החזר L.Bucket < R.Bucket;
  });

  LLVM_DEBUG(dbgs() << "Balanced partitioning completed\n");
}

فراغ BalancedPartitioning::bisect(ثابت FunctionNodeRange Nodes,
                                  unsigned RecDepth, unsigned RootBucket,
                                  unsigned Offset,
                                  std::optional<BPThreadPool> &TP) ثابت {
  unsigned NumNodes = llvm::size(Nodes);
  אם (NumNodes <= 1 || RecDepth >= Config.SplitDepth) {
    // We've reach the lowest level of the recursion tree. Fall back to the
    // original order and assign to buckets.
    llvm::sort(Nodes, [](ثابت auto &L, ثابت auto &R) {
      החזר L.InputOrderIndex < R.InputOrderIndex;
    });
    עבור (auto &N : Nodes)
      N.Bucket = Offset++;
    החזר;
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
      llvm::partition(Nodes, [&](auto &N) { החזר N.Bucket == LeftBucket; });
  unsigned MidOffset = Offset + std::distance(Nodes.begin(), NodesMid);

  auto LeftNodes = llvm::make_range(Nodes.begin(), NodesMid);
  auto RightNodes = llvm::make_range(NodesMid, Nodes.end());

  auto LeftRecTask = [هذا, LeftNodes, RecDepth, LeftBucket, Offset, &TP]() {
    bisect(LeftNodes, RecDepth + 1, LeftBucket, Offset, TP);
  };
  auto RightRecTask = [هذا, RightNodes, RecDepth, RightBucket, MidOffset,
                       &TP]() {
    bisect(RightNodes, RecDepth + 1, RightBucket, MidOffset, TP);
  };

  אם (TP && RecDepth < Config.TaskSplitDepth && NumNodes >= 4) {
    TP->async(std::move(LeftRecTask));
    TP->async(std::move(RightRecTask));
  } אחרת {
    LeftRecTask();
    RightRecTask();
  }
}

فراغ BalancedPartitioning::runIterations(ثابت FunctionNodeRange Nodes,
                                         unsigned LeftBucket,
                                         unsigned RightBucket,
                                         std::mt19937 &RNG) ثابت {
  unsigned NumNodes = llvm::size(Nodes);
  DenseMap<BPFunctionNode::UtilityNodeT, unsigned> UtilityNodeIndex;
  עבור (auto &N : Nodes)
    עבור (auto &UN : N.UtilityNodes)
      ++UtilityNodeIndex[UN];
  // Remove utility nodes if they have just one edge or are connected to all
  // functions
  עבור (auto &N : Nodes)
    llvm::erase_if(N.UtilityNodes, [&](auto &UN) {
      unsigned UNI = UtilityNodeIndex[UN];
      החזר UNI == 1 || UNI == NumNodes;
    });

  // Renumber utility nodes so they can be used to index into Signatures
  UtilityNodeIndex.clear();
  עבור (auto &N : Nodes)
    עבור (auto &UN : N.UtilityNodes)
      UN = UtilityNodeIndex.insert({UN, UtilityNodeIndex.size()}).first->second;

  // Initialize signatures
  SignaturesT Signatures(/*Size=*/UtilityNodeIndex.size());
  עבור (auto &N : Nodes) {
    עבור (auto &UN : N.UtilityNodes) {
      أكد(UN < Signatures.size());
      אם (N.Bucket == LeftBucket) {
        Signatures[UN].LeftCount++;
      } אחרת {
        Signatures[UN].RightCount++;
      }
    }
  }

  עבור (unsigned I = 0; I < Config.IterationsPerSplit; I++) {
    unsigned NumMovedNodes =
        runIteration(Nodes, LeftBucket, RightBucket, Signatures, RNG);
    אם (NumMovedNodes == 0)
      اكسر;
  }
}

unsigned BalancedPartitioning::runIteration(ثابت FunctionNodeRange Nodes,
                                            unsigned LeftBucket,
                                            unsigned RightBucket,
                                            SignaturesT &Signatures,
                                            std::mt19937 &RNG) ثابت {
  // Init signature cost caches
  עבור (auto &Signature : Signatures) {
    אם (Signature.CachedGainIsValid)
      استمر;
    unsigned L = Signature.LeftCount;
    unsigned R = Signature.RightCount;
    أكد((L > 0 || R > 0) && "incorrect signature");
    عائم Cost = logCost(L, R);
    Signature.CachedGainLR = 0.f;
    Signature.CachedGainRL = 0.f;
    אם (L > 0)
      Signature.CachedGainLR = Cost - logCost(L - 1, R + 1);
    אם (R > 0)
      Signature.CachedGainRL = Cost - logCost(L + 1, R - 1);
    Signature.CachedGainIsValid = אמת;
  }

  // Compute move gains
  using GainPair = std::pair<عائم, BPFunctionNode *>;
  std::vector<GainPair> Gains;
  עבור (auto &N : Nodes) {
    منطقي FromLeftToRight = (N.Bucket == LeftBucket);
    عائم Gain = moveGain(N, FromLeftToRight, Signatures);
    Gains.push_back(std::make_pair(Gain, &N));
  }

  // Collect left and right gains
  auto LeftEnd = llvm::partition(
      Gains, [&](ثابت auto &GP) { החזר GP.second->Bucket == LeftBucket; });
  auto LeftRange = llvm::make_range(Gains.begin(), LeftEnd);
  auto RightRange = llvm::make_range(LeftEnd, Gains.end());

  // Sort gains in descending order
  auto LargerGain = [](ثابت auto &L, ثابت auto &R) {
    החזר L.first > R.first;
  };
  llvm::stable_sort(LeftRange, LargerGain);
  llvm::stable_sort(RightRange, LargerGain);

  unsigned NumMovedDataVertices = 0;
  עבור (auto [LeftPair, RightPair] : llvm::zip(LeftRange, RightRange)) {
    auto &[LeftGain, LeftNode] = LeftPair;
    auto &[RightGain, RightNode] = RightPair;
    // Stop when the gain is no longer beneficial
    אם (LeftGain + RightGain <= 0.f)
      اكسر;
    // Try to exchange the nodes between buckets
    אם (moveFunctionNode(*LeftNode, LeftBucket, RightBucket, Signatures, RNG))
      ++NumMovedDataVertices;
    אם (moveFunctionNode(*RightNode, LeftBucket, RightBucket, Signatures, RNG))
      ++NumMovedDataVertices;
  }
  החזר NumMovedDataVertices;
}

منطقي BalancedPartitioning::moveFunctionNode(BPFunctionNode &N,
                                            unsigned LeftBucket,
                                            unsigned RightBucket,
                                            SignaturesT &Signatures,
                                            std::mt19937 &RNG) ثابت {
  // Sometimes we skip the move. This helps to escape local optima
  אם (std::uniform_real_distribution<عائم>(0.f, 1.f)(RNG) <=
      Config.SkipProbability)
    החזר שקר;

  منطقي FromLeftToRight = (N.Bucket == LeftBucket);
  // Update the current bucket
  N.Bucket = (FromLeftToRight ? RightBucket : LeftBucket);

  // Update signatures and invalidate gain cache
  אם (FromLeftToRight) {
    עבור (auto &UN : N.UtilityNodes) {
      auto &Signature = Signatures[UN];
      Signature.LeftCount--;
      Signature.RightCount++;
      Signature.CachedGainIsValid = שקר;
    }
  } אחרת {
    עבור (auto &UN : N.UtilityNodes) {
      auto &Signature = Signatures[UN];
      Signature.LeftCount++;
      Signature.RightCount--;
      Signature.CachedGainIsValid = שקר;
    }
  }
  החזר אמת;
}

فراغ BalancedPartitioning::split(ثابت FunctionNodeRange Nodes,
                                 unsigned StartBucket) ثابت {
  unsigned NumNodes = llvm::size(Nodes);
  auto NodesMid = Nodes.begin() + (NumNodes + 1) / 2;

  llvm::sort(Nodes, [](auto &L, auto &R) {
    החזר L.InputOrderIndex < R.InputOrderIndex;
  });

  עבור (auto &N : llvm::make_range(Nodes.begin(), NodesMid))
    N.Bucket = StartBucket;
  עבור (auto &N : llvm::make_range(NodesMid, Nodes.end()))
    N.Bucket = StartBucket + 1;
}

عائم BalancedPartitioning::moveGain(ثابت BPFunctionNode &N,
                                     منطقي FromLeftToRight,
                                     ثابت SignaturesT &Signatures) {
  عائم Gain = 0.f;
  עבור (auto &UN : N.UtilityNodes)
    Gain += (FromLeftToRight ? Signatures[UN].CachedGainLR
                             : Signatures[UN].CachedGainRL);
  החזר Gain;
}

عائم BalancedPartitioning::logCost(unsigned X, unsigned Y) ثابت {
  החזר -(X * log2Cached(X + 1) + Y * log2Cached(Y + 1));
}

عائم BalancedPartitioning::log2Cached(unsigned i) ثابت {
  החזר (i < LOG_CACHE_SIZE) ? Log2Cache[i] : std::log2(i);
}
