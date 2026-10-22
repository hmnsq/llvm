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

#அந்தர்பாவ "llvm/Support/BalancedPartitioning.h"
#அந்தர்பாவ "llvm/Config/llvm-config.h" // for LLVM_ENABLE_THREADS
#அந்தர்பாவ "llvm/Support/Debug.h"
#அந்தர்பாவ "llvm/Support/Format.h"
#அந்தர்பாவ "llvm/Support/FormatVariadic.h"
#அந்தர்பாவ "llvm/Support/ThreadPool.h"

#அந்தர்பாவ <cmath>

using நாமாகாஶ llvm;
#define DEBUG_TYPE "balanced-partitioning"

வெறுமை BPFunction_முனை::dump(raw_ostream &OS) மாறிலி {
  OS << formatv("{{ID={0} Utilities={{{1:$[,]}} Bucket={2}}", Id,
                make_range(UtilityNodes.begin(), UtilityNodes.end()), Bucket);
}

template <typename Func>
வெறுமை BalancedPartitioning::BPThreadPool::async(Func &&F) {
#எனில் LLVM_ENABLE_THREADS
  // This new thread could spawn more threads, so mark it as active
  ++இலக்கம்_Active_Threads;
  TheThreadPool.async([இது, F]() {
    // Run the task
    F();

    // This thread will no longer spawn new threads, so mark it as inactive
    எனில் (--இலக்கம்_Active_Threads == 0) {
      // There are no more active threads, so mark as finished and notify
      {
        std::unique_lock<std::mutex> lock(mtx);
        ப்ரதிஜ்ஞா(!IsFinishedSpawning);
        IsFinishedSpawning = உண்மை;
      }
      cv.notify_one();
    }
  });
#இல்லையெனில்
  llvm_unreachable("threads are disabled");
#endif
}

வெறுமை BalancedPartitioning::BPThreadPool::wait() {
#எனில் LLVM_ENABLE_THREADS
  // TODO: We could remove the mutex and condition variable and use
  // std::atomic::wait() instead, but that isn't available until C++20
  {
    std::unique_lock<std::mutex> lock(mtx);
    cv.wait(lock, [&]() { திரும்பு IsFinishedSpawning; });
    ப்ரதிஜ்ஞா(IsFinishedSpawning && இலக்கம்_Active_Threads == 0);
  }
  // Now we can call ThreadPool::wait() since all tasks have been submitted
  TheThreadPool.wait();
#இல்லையெனில்
  llvm_unreachable("threads are disabled");
#endif
}

BalancedPartitioning::BalancedPartitioning(
    மாறிலி BalancedPartitioningConfig &Config)
    : Config(Config) {
  // Pre-computing log2 values
  Log2Cache[0] = 0.0;
  க்கு (unsigned I = 1; I < LOG_CACHE_SIZE; I++)
    Log2Cache[I] = std::log2(I);
}

வெறுமை BalancedPartitioning::run(std::vector<BPFunction_முனை> &Nodes) மாறிலி {
  LLVM_DEBUG(
      dbgs() << format(
          "Partitioning %d nodes using depth %d and %d iterations per split\n",
          Nodes.size(), Config.SplitDepth, Config.IterationsPerSplit));
  std::optional<BPThreadPool> TP;
#எனில் LLVM_ENABLE_THREADS
  DefaultThreadPool TheThreadPool;
  எனில் (Config.TaskSplitDepth > 1)
    TP.emplace(TheThreadPool);
#endif

  // Record the input order
  க்கு (unsigned I = 0; I < Nodes.size(); I++)
    Nodes[I].உள்ளீடு_Order_Index = I;

  auto NodesRange = llvm::make_range(Nodes.begin(), Nodes.end());
  auto BisectTask = [இது, NodesRange, &TP]() {
    bisect(NodesRange, /*RecDepth=*/0, /*RootBucket=*/1, /*Offset=*/0, TP);
  };
  எனில் (TP) {
    TP->async(std::move(BisectTask));
    TP->wait();
  } இல்லையெனில் {
    BisectTask();
  }

  llvm::stable_sort(NodesRange, [](மாறிலி auto &L, மாறிலி auto &R) {
    திரும்பு L.Bucket < R.Bucket;
  });

  LLVM_DEBUG(dbgs() << "Balanced partitioning completed\n");
}

வெறுமை BalancedPartitioning::bisect(மாறிலி Function_முனை_Range Nodes,
                                  unsigned RecDepth, unsigned வேர்_Bucket,
                                  unsigned Offset,
                                  std::optional<BPThreadPool> &TP) மாறிலி {
  unsigned இலக்கம்_Nodes = llvm::size(Nodes);
  எனில் (இலக்கம்_Nodes <= 1 || RecDepth >= Config.SplitDepth) {
    // We've reach the lowest level of the recursion tree. Fall back to the
    // original order and assign to buckets.
    llvm::sort(Nodes, [](மாறிலி auto &L, மாறிலி auto &R) {
      திரும்பு L.உள்ளீடு_Order_Index < R.உள்ளீடு_Order_Index;
    });
    க்கு (auto &N : Nodes)
      N.Bucket = Offset++;
    திரும்பு;
  }

  LLVM_DEBUG(dbgs() << format("Bisect with %d nodes and root bucket %d\n",
                              இலக்கம்_Nodes, வேர்_Bucket));

  std::mt19937 RNG(வேர்_Bucket);

  unsigned இடது_Bucket = 2 * வேர்_Bucket;
  unsigned வலது_Bucket = 2 * வேர்_Bucket + 1;

  // Split into two and assign to the left and right buckets
  split(Nodes, இடது_Bucket);

  runIterations(Nodes, இடது_Bucket, வலது_Bucket, RNG);

  // Split nodes wrt the resulting buckets
  auto NodesMid =
      llvm::partition(Nodes, [&](auto &N) { திரும்பு N.Bucket == இடது_Bucket; });
  unsigned MidOffset = Offset + std::distance(Nodes.begin(), NodesMid);

  auto இடது_Nodes = llvm::make_range(Nodes.begin(), NodesMid);
  auto வலது_Nodes = llvm::make_range(NodesMid, Nodes.end());

  auto இடது_Rec_Task = [இது, இடது_Nodes, RecDepth, இடது_Bucket, Offset, &TP]() {
    bisect(இடது_Nodes, RecDepth + 1, இடது_Bucket, Offset, TP);
  };
  auto வலது_Rec_Task = [இது, வலது_Nodes, RecDepth, வலது_Bucket, MidOffset,
                       &TP]() {
    bisect(வலது_Nodes, RecDepth + 1, வலது_Bucket, MidOffset, TP);
  };

  எனில் (TP && RecDepth < Config.TaskSplitDepth && இலக்கம்_Nodes >= 4) {
    TP->async(std::move(இடது_Rec_Task));
    TP->async(std::move(வலது_Rec_Task));
  } இல்லையெனில் {
    இடது_Rec_Task();
    வலது_Rec_Task();
  }
}

வெறுமை BalancedPartitioning::runIterations(மாறிலி Function_முனை_Range Nodes,
                                         unsigned இடது_Bucket,
                                         unsigned வலது_Bucket,
                                         std::mt19937 &RNG) மாறிலி {
  unsigned இலக்கம்_Nodes = llvm::size(Nodes);
  DenseMap<BPFunction_முனை::Utility_முனை_T, unsigned> Utility_முனை_Index;
  க்கு (auto &N : Nodes)
    க்கு (auto &UN : N.UtilityNodes)
      ++Utility_முனை_Index[UN];
  // Remove utility nodes if they have just one edge or are connected to all
  // functions
  க்கு (auto &N : Nodes)
    llvm::erase_if(N.UtilityNodes, [&](auto &UN) {
      unsigned UNI = Utility_முனை_Index[UN];
      திரும்பு UNI == 1 || UNI == இலக்கம்_Nodes;
    });

  // Renumber utility nodes so they can be used to index into Signatures
  Utility_முனை_Index.clear();
  க்கு (auto &N : Nodes)
    க்கு (auto &UN : N.UtilityNodes)
      UN = Utility_முனை_Index.செருகு({UN, Utility_முனை_Index.size()}).first->second;

  // Initialize signatures
  SignaturesT Signatures(/*Size=*/Utility_முனை_Index.size());
  க்கு (auto &N : Nodes) {
    க்கு (auto &UN : N.UtilityNodes) {
      ப்ரதிஜ்ஞா(UN < Signatures.size());
      எனில் (N.Bucket == இடது_Bucket) {
        Signatures[UN].இடது_மொத்தம்++;
      } இல்லையெனில் {
        Signatures[UN].வலது_மொத்தம்++;
      }
    }
  }

  க்கு (unsigned I = 0; I < Config.IterationsPerSplit; I++) {
    unsigned இலக்கம்_Moved_Nodes =
        runIteration(Nodes, இடது_Bucket, வலது_Bucket, Signatures, RNG);
    எனில் (இலக்கம்_Moved_Nodes == 0)
      நிறுத்து;
  }
}

unsigned BalancedPartitioning::runIteration(மாறிலி Function_முனை_Range Nodes,
                                            unsigned இடது_Bucket,
                                            unsigned வலது_Bucket,
                                            SignaturesT &Signatures,
                                            std::mt19937 &RNG) மாறிலி {
  // Init signature cost caches
  க்கு (auto &Signature : Signatures) {
    எனில் (Signature.CachedGainIsValid)
      தொடர்;
    unsigned L = Signature.இடது_மொத்தம்;
    unsigned R = Signature.வலது_மொத்தம்;
    ப்ரதிஜ்ஞா((L > 0 || R > 0) && "incorrect signature");
    தசம Cost = logCost(L, R);
    Signature.CachedGainLR = 0.f;
    Signature.CachedGainRL = 0.f;
    எனில் (L > 0)
      Signature.CachedGainLR = Cost - logCost(L - 1, R + 1);
    எனில் (R > 0)
      Signature.CachedGainRL = Cost - logCost(L + 1, R - 1);
    Signature.CachedGainIsValid = உண்மை;
  }

  // Compute move gains
  using GainPair = std::pair<தசம, BPFunction_முனை *>;
  std::vector<GainPair> Gains;
  க்கு (auto &N : Nodes) {
    தருக்க From_இடது_To_வலது = (N.Bucket == இடது_Bucket);
    தசம Gain = moveGain(N, From_இடது_To_வலது, Signatures);
    Gains.தள்ளு_back(std::make_pair(Gain, &N));
  }

  // Collect left and right gains
  auto இடது_End = llvm::partition(
      Gains, [&](மாறிலி auto &GP) { திரும்பு GP.second->Bucket == இடது_Bucket; });
  auto இடது_Range = llvm::make_range(Gains.begin(), இடது_End);
  auto வலது_Range = llvm::make_range(இடது_End, Gains.end());

  // Sort gains in descending order
  auto LargerGain = [](மாறிலி auto &L, மாறிலி auto &R) {
    திரும்பு L.first > R.first;
  };
  llvm::stable_sort(இடது_Range, LargerGain);
  llvm::stable_sort(வலது_Range, LargerGain);

  unsigned இலக்கம்_Moved_Data_Vertices = 0;
  க்கு (auto [இடது_Pair, வலது_Pair] : llvm::zip(இடது_Range, வலது_Range)) {
    auto &[இடது_Gain, இடது_முனை] = இடது_Pair;
    auto &[வலது_Gain, வலது_முனை] = வலது_Pair;
    // Stop when the gain is no longer beneficial
    எனில் (இடது_Gain + வலது_Gain <= 0.f)
      நிறுத்து;
    // Try to exchange the nodes between buckets
    எனில் (move_Function_முனை(*இடது_முனை, இடது_Bucket, வலது_Bucket, Signatures, RNG))
      ++இலக்கம்_Moved_Data_Vertices;
    எனில் (move_Function_முனை(*வலது_முனை, இடது_Bucket, வலது_Bucket, Signatures, RNG))
      ++இலக்கம்_Moved_Data_Vertices;
  }
  திரும்பு இலக்கம்_Moved_Data_Vertices;
}

தருக்க BalancedPartitioning::move_Function_முனை(BPFunction_முனை &N,
                                            unsigned இடது_Bucket,
                                            unsigned வலது_Bucket,
                                            SignaturesT &Signatures,
                                            std::mt19937 &RNG) மாறிலி {
  // Sometimes we skip the move. This helps to escape local optima
  எனில் (std::uniform_real_distribution<தசம>(0.f, 1.f)(RNG) <=
      Config.SkipProbability)
    திரும்பு பொய்;

  தருக்க From_இடது_To_வலது = (N.Bucket == இடது_Bucket);
  // Update the current bucket
  N.Bucket = (From_இடது_To_வலது ? வலது_Bucket : இடது_Bucket);

  // Update signatures and invalidate gain cache
  எனில் (From_இடது_To_வலது) {
    க்கு (auto &UN : N.UtilityNodes) {
      auto &Signature = Signatures[UN];
      Signature.இடது_மொத்தம்--;
      Signature.வலது_மொத்தம்++;
      Signature.CachedGainIsValid = பொய்;
    }
  } இல்லையெனில் {
    க்கு (auto &UN : N.UtilityNodes) {
      auto &Signature = Signatures[UN];
      Signature.இடது_மொத்தம்++;
      Signature.வலது_மொத்தம்--;
      Signature.CachedGainIsValid = பொய்;
    }
  }
  திரும்பு உண்மை;
}

வெறுமை BalancedPartitioning::split(மாறிலி Function_முனை_Range Nodes,
                                 unsigned StartBucket) மாறிலி {
  unsigned இலக்கம்_Nodes = llvm::size(Nodes);
  auto NodesMid = Nodes.begin() + (இலக்கம்_Nodes + 1) / 2;

  llvm::sort(Nodes, [](auto &L, auto &R) {
    திரும்பு L.உள்ளீடு_Order_Index < R.உள்ளீடு_Order_Index;
  });

  க்கு (auto &N : llvm::make_range(Nodes.begin(), NodesMid))
    N.Bucket = StartBucket;
  க்கு (auto &N : llvm::make_range(NodesMid, Nodes.end()))
    N.Bucket = StartBucket + 1;
}

தசம BalancedPartitioning::moveGain(மாறிலி BPFunction_முனை &N,
                                     தருக்க From_இடது_To_வலது,
                                     மாறிலி SignaturesT &Signatures) {
  தசம Gain = 0.f;
  க்கு (auto &UN : N.UtilityNodes)
    Gain += (From_இடது_To_வலது ? Signatures[UN].CachedGainLR
                             : Signatures[UN].CachedGainRL);
  திரும்பு Gain;
}

தசம BalancedPartitioning::logCost(unsigned X, unsigned Y) மாறிலி {
  திரும்பு -(X * log2Cached(X + 1) + Y * log2Cached(Y + 1));
}

தசம BalancedPartitioning::log2Cached(unsigned i) மாறிலி {
  திரும்பு (i < LOG_CACHE_SIZE) ? Log2Cache[i] : std::log2(i);
}
