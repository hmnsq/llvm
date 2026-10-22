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

#अन्तर्भाव "llvm/Support/BalancedPartitioning.h"
#अन्तर्भाव "llvm/Config/llvm-config.h" // for LLVM_ENABLE_THREADS
#अन्तर्भाव "llvm/Support/Debug.h"
#अन्तर्भाव "llvm/Support/Format.h"
#अन्तर्भाव "llvm/Support/FormatVariadic.h"
#अन्तर्भाव "llvm/Support/ThreadPool.h"

#अन्तर्भाव <cmath>

using नामाकाश llvm;
#define DEBUG_TYPE "balanced-partitioning"

शून्य BPFunction_गाँठो::dump(raw_ostream &OS) अचर {
  OS << formatv("{{ID={0} Utilities={{{1:$[,]}} Bucket={2}}", Id,
                make_range(UtilityNodes.begin(), UtilityNodes.end()), Bucket);
}

ढाँचा <typename Func>
शून्य BalancedPartitioning::BPThreadPool::async(Func &&F) {
#यदि LLVM_ENABLE_THREADS
  // This new thread could spawn more threads, so mark it as active
  ++अङ्क_Active_Threads;
  TheThreadPool.async([एतत्, F]() {
    // Run the task
    F();

    // This thread will no longer spawn new threads, so mark it as inactive
    यदि (--अङ्क_Active_Threads == 0) {
      // There are no more active threads, so mark as finished and notify
      {
        std::unique_lock<std::mutex> lock(mtx);
        प्रतिज्ञा(!IsFinishedSpawning);
        IsFinishedSpawning = सत्य;
      }
      cv.notify_one();
    }
  });
#अन्यथा
  llvm_unreachable("threads are disabled");
#endif
}

शून्य BalancedPartitioning::BPThreadPool::रुको() {
#यदि LLVM_ENABLE_THREADS
  // TODO: We could remove the mutex and condition variable and use
  // std::atomic::wait() instead, but that isn't available until C++20
  {
    std::unique_lock<std::mutex> lock(mtx);
    cv.रुको(lock, [&]() { फर्काऊ IsFinishedSpawning; });
    प्रतिज्ञा(IsFinishedSpawning && अङ्क_Active_Threads == 0);
  }
  // Now we can call ThreadPool::wait() since all tasks have been submitted
  TheThreadPool.रुको();
#अन्यथा
  llvm_unreachable("threads are disabled");
#endif
}

BalancedPartitioning::BalancedPartitioning(
    अचर BalancedPartitioningConfig &Config)
    : Config(Config) {
  // Pre-computing log2 values
  Log2Cache[0] = 0.0;
  लागि (अचिन्हित I = 1; I < LOG_CACHE_SIZE; I++)
    Log2Cache[I] = std::log2(I);
}

शून्य BalancedPartitioning::run(std::vector<BPFunction_गाँठो> &Nodes) अचर {
  LLVM_DEBUG(
      dbgs() << format(
          "Partitioning %d nodes using depth %d and %d iterations per split\n",
          Nodes.size(), Config.SplitDepth, Config.IterationsPerSplit));
  std::optional<BPThreadPool> TP;
#यदि LLVM_ENABLE_THREADS
  DefaultThreadPool TheThreadPool;
  यदि (Config.TaskSplitDepth > 1)
    TP.emplace(TheThreadPool);
#endif

  // Record the input order
  लागि (अचिन्हित I = 0; I < Nodes.size(); I++)
    Nodes[I].आगत_Order_Index = I;

  स्वतः NodesRange = llvm::make_range(Nodes.begin(), Nodes.end());
  स्वतः BisectTask = [एतत्, NodesRange, &TP]() {
    bisect(NodesRange, /*RecDepth=*/0, /*RootBucket=*/1, /*Offset=*/0, TP);
  };
  यदि (TP) {
    TP->async(std::move(BisectTask));
    TP->रुको();
  } अन्यथा {
    BisectTask();
  }

  llvm::stable_sort(NodesRange, [](अचर स्वतः &L, अचर स्वतः &R) {
    फर्काऊ L.Bucket < R.Bucket;
  });

  LLVM_DEBUG(dbgs() << "Balanced partitioning completed\n");
}

शून्य BalancedPartitioning::bisect(अचर Function_गाँठो_Range Nodes,
                                  अचिन्हित RecDepth, अचिन्हित जरा_Bucket,
                                  अचिन्हित Offset,
                                  std::optional<BPThreadPool> &TP) अचर {
  अचिन्हित अङ्क_Nodes = llvm::size(Nodes);
  यदि (अङ्क_Nodes <= 1 || RecDepth >= Config.SplitDepth) {
    // We've reach the lowest level of the recursion tree. Fall back to the
    // original order and assign to buckets.
    llvm::sort(Nodes, [](अचर स्वतः &L, अचर स्वतः &R) {
      फर्काऊ L.आगत_Order_Index < R.आगत_Order_Index;
    });
    लागि (स्वतः &N : Nodes)
      N.Bucket = Offset++;
    फर्काऊ;
  }

  LLVM_DEBUG(dbgs() << format("Bisect with %d nodes and root bucket %d\n",
                              अङ्क_Nodes, जरा_Bucket));

  std::mt19937 RNG(जरा_Bucket);

  अचिन्हित बायाँ_Bucket = 2 * जरा_Bucket;
  अचिन्हित दायाँ_Bucket = 2 * जरा_Bucket + 1;

  // Split into two and assign to the left and right buckets
  split(Nodes, बायाँ_Bucket);

  runIterations(Nodes, बायाँ_Bucket, दायाँ_Bucket, RNG);

  // Split nodes wrt the resulting buckets
  स्वतः NodesMid =
      llvm::partition(Nodes, [&](स्वतः &N) { फर्काऊ N.Bucket == बायाँ_Bucket; });
  अचिन्हित MidOffset = Offset + std::distance(Nodes.begin(), NodesMid);

  स्वतः बायाँ_Nodes = llvm::make_range(Nodes.begin(), NodesMid);
  स्वतः दायाँ_Nodes = llvm::make_range(NodesMid, Nodes.end());

  स्वतः बायाँ_Rec_Task = [एतत्, बायाँ_Nodes, RecDepth, बायाँ_Bucket, Offset, &TP]() {
    bisect(बायाँ_Nodes, RecDepth + 1, बायाँ_Bucket, Offset, TP);
  };
  स्वतः दायाँ_Rec_Task = [एतत्, दायाँ_Nodes, RecDepth, दायाँ_Bucket, MidOffset,
                       &TP]() {
    bisect(दायाँ_Nodes, RecDepth + 1, दायाँ_Bucket, MidOffset, TP);
  };

  यदि (TP && RecDepth < Config.TaskSplitDepth && अङ्क_Nodes >= 4) {
    TP->async(std::move(बायाँ_Rec_Task));
    TP->async(std::move(दायाँ_Rec_Task));
  } अन्यथा {
    बायाँ_Rec_Task();
    दायाँ_Rec_Task();
  }
}

शून्य BalancedPartitioning::runIterations(अचर Function_गाँठो_Range Nodes,
                                         अचिन्हित बायाँ_Bucket,
                                         अचिन्हित दायाँ_Bucket,
                                         std::mt19937 &RNG) अचर {
  अचिन्हित अङ्क_Nodes = llvm::size(Nodes);
  DenseMap<BPFunction_गाँठो::Utility_गाँठो_T, अचिन्हित> Utility_गाँठो_Index;
  लागि (स्वतः &N : Nodes)
    लागि (स्वतः &UN : N.UtilityNodes)
      ++Utility_गाँठो_Index[UN];
  // Remove utility nodes if they have just one edge or are connected to all
  // functions
  लागि (स्वतः &N : Nodes)
    llvm::erase_if(N.UtilityNodes, [&](स्वतः &UN) {
      अचिन्हित UNI = Utility_गाँठो_Index[UN];
      फर्काऊ UNI == 1 || UNI == अङ्क_Nodes;
    });

  // Renumber utility nodes so they can be used to index into Signatures
  Utility_गाँठो_Index.clear();
  लागि (स्वतः &N : Nodes)
    लागि (स्वतः &UN : N.UtilityNodes)
      UN = Utility_गाँठो_Index.घुसाउ({UN, Utility_गाँठो_Index.size()}).first->second;

  // Initialize signatures
  SignaturesT Signatures(/*Size=*/Utility_गाँठो_Index.size());
  लागि (स्वतः &N : Nodes) {
    लागि (स्वतः &UN : N.UtilityNodes) {
      प्रतिज्ञा(UN < Signatures.size());
      यदि (N.Bucket == बायाँ_Bucket) {
        Signatures[UN].बायाँ_गन्ती++;
      } अन्यथा {
        Signatures[UN].दायाँ_गन्ती++;
      }
    }
  }

  लागि (अचिन्हित I = 0; I < Config.IterationsPerSplit; I++) {
    अचिन्हित अङ्क_Moved_Nodes =
        runIteration(Nodes, बायाँ_Bucket, दायाँ_Bucket, Signatures, RNG);
    यदि (अङ्क_Moved_Nodes == 0)
      रोक;
  }
}

अचिन्हित BalancedPartitioning::runIteration(अचर Function_गाँठो_Range Nodes,
                                            अचिन्हित बायाँ_Bucket,
                                            अचिन्हित दायाँ_Bucket,
                                            SignaturesT &Signatures,
                                            std::mt19937 &RNG) अचर {
  // Init signature cost caches
  लागि (स्वतः &Signature : Signatures) {
    यदि (Signature.CachedGainIsValid)
      जारी;
    अचिन्हित L = Signature.बायाँ_गन्ती;
    अचिन्हित R = Signature.दायाँ_गन्ती;
    प्रतिज्ञा((L > 0 || R > 0) && "incorrect signature");
    भिन्न Cost = logCost(L, R);
    Signature.CachedGainLR = 0.f;
    Signature.CachedGainRL = 0.f;
    यदि (L > 0)
      Signature.CachedGainLR = Cost - logCost(L - 1, R + 1);
    यदि (R > 0)
      Signature.CachedGainRL = Cost - logCost(L + 1, R - 1);
    Signature.CachedGainIsValid = सत्य;
  }

  // Compute move gains
  using GainPair = std::pair<भिन्न, BPFunction_गाँठो *>;
  std::vector<GainPair> Gains;
  लागि (स्वतः &N : Nodes) {
    तार्किक From_बायाँ_To_दायाँ = (N.Bucket == बायाँ_Bucket);
    भिन्न Gain = moveGain(N, From_बायाँ_To_दायाँ, Signatures);
    Gains.थप_back(std::make_pair(Gain, &N));
  }

  // Collect left and right gains
  स्वतः बायाँ_End = llvm::partition(
      Gains, [&](अचर स्वतः &GP) { फर्काऊ GP.second->Bucket == बायाँ_Bucket; });
  स्वतः बायाँ_Range = llvm::make_range(Gains.begin(), बायाँ_End);
  स्वतः दायाँ_Range = llvm::make_range(बायाँ_End, Gains.end());

  // Sort gains in descending order
  स्वतः LargerGain = [](अचर स्वतः &L, अचर स्वतः &R) {
    फर्काऊ L.first > R.first;
  };
  llvm::stable_sort(बायाँ_Range, LargerGain);
  llvm::stable_sort(दायाँ_Range, LargerGain);

  अचिन्हित अङ्क_Moved_Data_Vertices = 0;
  लागि (स्वतः [बायाँ_Pair, दायाँ_Pair] : llvm::zip(बायाँ_Range, दायाँ_Range)) {
    स्वतः &[बायाँ_Gain, बायाँ_गाँठो] = बायाँ_Pair;
    स्वतः &[दायाँ_Gain, दायाँ_गाँठो] = दायाँ_Pair;
    // Stop when the gain is no longer beneficial
    यदि (बायाँ_Gain + दायाँ_Gain <= 0.f)
      रोक;
    // Try to exchange the nodes between buckets
    यदि (move_Function_गाँठो(*बायाँ_गाँठो, बायाँ_Bucket, दायाँ_Bucket, Signatures, RNG))
      ++अङ्क_Moved_Data_Vertices;
    यदि (move_Function_गाँठो(*दायाँ_गाँठो, बायाँ_Bucket, दायाँ_Bucket, Signatures, RNG))
      ++अङ्क_Moved_Data_Vertices;
  }
  फर्काऊ अङ्क_Moved_Data_Vertices;
}

तार्किक BalancedPartitioning::move_Function_गाँठो(BPFunction_गाँठो &N,
                                            अचिन्हित बायाँ_Bucket,
                                            अचिन्हित दायाँ_Bucket,
                                            SignaturesT &Signatures,
                                            std::mt19937 &RNG) अचर {
  // Sometimes we skip the move. This helps to escape local optima
  यदि (std::uniform_real_distribution<भिन्न>(0.f, 1.f)(RNG) <=
      Config.SkipProbability)
    फर्काऊ असत्य;

  तार्किक From_बायाँ_To_दायाँ = (N.Bucket == बायाँ_Bucket);
  // Update the current bucket
  N.Bucket = (From_बायाँ_To_दायाँ ? दायाँ_Bucket : बायाँ_Bucket);

  // Update signatures and invalidate gain cache
  यदि (From_बायाँ_To_दायाँ) {
    लागि (स्वतः &UN : N.UtilityNodes) {
      स्वतः &Signature = Signatures[UN];
      Signature.बायाँ_गन्ती--;
      Signature.दायाँ_गन्ती++;
      Signature.CachedGainIsValid = असत्य;
    }
  } अन्यथा {
    लागि (स्वतः &UN : N.UtilityNodes) {
      स्वतः &Signature = Signatures[UN];
      Signature.बायाँ_गन्ती++;
      Signature.दायाँ_गन्ती--;
      Signature.CachedGainIsValid = असत्य;
    }
  }
  फर्काऊ सत्य;
}

शून्य BalancedPartitioning::split(अचर Function_गाँठो_Range Nodes,
                                 अचिन्हित StartBucket) अचर {
  अचिन्हित अङ्क_Nodes = llvm::size(Nodes);
  स्वतः NodesMid = Nodes.begin() + (अङ्क_Nodes + 1) / 2;

  llvm::sort(Nodes, [](स्वतः &L, स्वतः &R) {
    फर्काऊ L.आगत_Order_Index < R.आगत_Order_Index;
  });

  लागि (स्वतः &N : llvm::make_range(Nodes.begin(), NodesMid))
    N.Bucket = StartBucket;
  लागि (स्वतः &N : llvm::make_range(NodesMid, Nodes.end()))
    N.Bucket = StartBucket + 1;
}

भिन्न BalancedPartitioning::moveGain(अचर BPFunction_गाँठो &N,
                                     तार्किक From_बायाँ_To_दायाँ,
                                     अचर SignaturesT &Signatures) {
  भिन्न Gain = 0.f;
  लागि (स्वतः &UN : N.UtilityNodes)
    Gain += (From_बायाँ_To_दायाँ ? Signatures[UN].CachedGainLR
                             : Signatures[UN].CachedGainRL);
  फर्काऊ Gain;
}

भिन्न BalancedPartitioning::logCost(अचिन्हित X, अचिन्हित Y) अचर {
  फर्काऊ -(X * log2Cached(X + 1) + Y * log2Cached(Y + 1));
}

भिन्न BalancedPartitioning::log2Cached(अचिन्हित i) अचर {
  फर्काऊ (i < LOG_CACHE_SIZE) ? Log2Cache[i] : std::log2(i);
}
