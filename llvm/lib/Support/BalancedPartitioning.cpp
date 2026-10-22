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

उपयुज्य नामाकाश llvm;
#परिभाषय DEBUG_TYPE "balanced-partitioning"

शून्य BPFunction_ग्रन्थिः::dump(raw_ostream &OS) ध्रुव {
  OS << formatv("{{ID={0} Utilities={{{1:$[,]}} Bucket={2}}", Id,
                make_range(UtilityNodes.begin(), UtilityNodes.अन्तः()), Bucket);
}

प्रतिरूपम् <प्रकारनाम Func>
शून्य BalancedPartitioning::BPThreadPool::async(Func &&F) {
#यदि LLVM_ENABLE_तन्तवः
  // This new thread could spawn more threads, so mark it as active
  ++अङ्कः_सक्रिय_तन्तवः;
  TheThreadPool.async([एतत्, F]() {
    // Run the task
    F();

    // This thread will no longer spawn new threads, so mark it as inactive
    यदि (--अङ्कः_सक्रिय_तन्तवः == 0) {
      // There are no more active threads, so mark as finished and notify
      {
        std::unique_तालकम्<std::mutex> तालकम्(mtx);
        प्रतिज्ञा(!Is_परिसमाप्तम्_Spawning);
        Is_परिसमाप्तम्_Spawning = सत्य;
      }
      cv.notify_one();
    }
  });
#अन्यथा
  llvm_unreachable("threads are disabled");
#यदि_अन्तः
}

शून्य BalancedPartitioning::BPThreadPool::प्रतीक्षस्व() {
#यदि LLVM_ENABLE_तन्तवः
  // TODO: We could remove the mutex and condition variable and use
  // std::atomic::wait() instead, but that isn't available until C++20
  {
    std::unique_तालकम्<std::mutex> तालकम्(mtx);
    cv.प्रतीक्षस्व(तालकम्, [&]() { प्रत्यागम Is_परिसमाप्तम्_Spawning; });
    प्रतिज्ञा(Is_परिसमाप्तम्_Spawning && अङ्कः_सक्रिय_तन्तवः == 0);
  }
  // Now we can call ThreadPool::wait() since all tasks have been submitted
  TheThreadPool.प्रतीक्षस्व();
#अन्यथा
  llvm_unreachable("threads are disabled");
#यदि_अन्तः
}

BalancedPartitioning::BalancedPartitioning(
    ध्रुव BalancedPartitioningConfig &Config)
    : Config(Config) {
  // Pre-computing log2 values
  Log2Cache[0] = 0.0;
  प्रति (अचिह्नित I = 1; I < LOG_CACHE_आकार; I++)
    Log2Cache[I] = std::log2(I);
}

शून्य BalancedPartitioning::सततम्(std::सदिशम्<BPFunction_ग्रन्थिः> &Nodes) ध्रुव {
  LLVM_DEBUG(
      dbgs() << format(
          "Partitioning %d nodes using depth %d and %d iterations per split\n",
          Nodes.आकार_2(), Config.विभज_गभीरता, Config.Iterations_प्रत्येकम्_विभज));
  std::optional<BPThreadPool> TP;
#यदि LLVM_ENABLE_तन्तवः
  DefaultThreadPool TheThreadPool;
  यदि (Config.Task_विभज_गभीरता > 1)
    TP.emplace(TheThreadPool);
#यदि_अन्तः

  // Record the input order
  प्रति (अचिह्नित I = 0; I < Nodes.आकार_2(); I++)
    Nodes[I].निवेशः_क्रमः_सूचकांक = I;

  स्वचालित NodesRange = llvm::make_range(Nodes.begin(), Nodes.अन्तः());
  स्वचालित BisectTask = [एतत्, NodesRange, &TP]() {
    bisect(NodesRange, /*RecDepth=*/0, /*RootBucket=*/1, /*Offset=*/0, TP);
  };
  यदि (TP) {
    TP->async(std::move(BisectTask));
    TP->प्रतीक्षस्व();
  } अन्यथा {
    BisectTask();
  }

  llvm::stable_क्रमय(NodesRange, [](ध्रुव स्वचालित &L, ध्रुव स्वचालित &R) {
    प्रत्यागम L.Bucket < R.Bucket;
  });

  LLVM_DEBUG(dbgs() << "Balanced partitioning completed\n");
}

शून्य BalancedPartitioning::bisect(ध्रुव Function_ग्रन्थिः_Range Nodes,
                                  अचिह्नित Rec_गभीरता, अचिह्नित मूलम्_Bucket,
                                  अचिह्नित विस्थापनम्,
                                  std::optional<BPThreadPool> &TP) ध्रुव {
  अचिह्नित अङ्कः_Nodes = llvm::आकार_2(Nodes);
  यदि (अङ्कः_Nodes <= 1 || Rec_गभीरता >= Config.विभज_गभीरता) {
    // We've reach the lowest level of the recursion tree. Fall back to the
    // original order and assign to buckets.
    llvm::क्रमय(Nodes, [](ध्रुव स्वचालित &L, ध्रुव स्वचालित &R) {
      प्रत्यागम L.निवेशः_क्रमः_सूचकांक < R.निवेशः_क्रमः_सूचकांक;
    });
    प्रति (स्वचालित &N : Nodes)
      N.Bucket = विस्थापनम्++;
    प्रत्यागम;
  }

  LLVM_DEBUG(dbgs() << format("Bisect with %d nodes and root bucket %d\n",
                              अङ्कः_Nodes, मूलम्_Bucket));

  std::mt19937 RNG(मूलम्_Bucket);

  अचिह्नित वामः_Bucket = 2 * मूलम्_Bucket;
  अचिह्नित दक्षिणः_Bucket = 2 * मूलम्_Bucket + 1;

  // Split into two and assign to the left and right buckets
  विभज(Nodes, वामः_Bucket);

  सततम्_Iterations(Nodes, वामः_Bucket, दक्षिणः_Bucket, RNG);

  // Split nodes wrt the resulting buckets
  स्वचालित NodesMid =
      llvm::partition(Nodes, [&](स्वचालित &N) { प्रत्यागम N.Bucket == वामः_Bucket; });
  अचिह्नित Mid_विस्थापनम् = विस्थापनम् + std::distance(Nodes.begin(), NodesMid);

  स्वचालित वामः_Nodes = llvm::make_range(Nodes.begin(), NodesMid);
  स्वचालित दक्षिणः_Nodes = llvm::make_range(NodesMid, Nodes.अन्तः());

  स्वचालित वामः_Rec_Task = [एतत्, वामः_Nodes, Rec_गभीरता, वामः_Bucket, विस्थापनम्, &TP]() {
    bisect(वामः_Nodes, Rec_गभीरता + 1, वामः_Bucket, विस्थापनम्, TP);
  };
  स्वचालित दक्षिणः_Rec_Task = [एतत्, दक्षिणः_Nodes, Rec_गभीरता, दक्षिणः_Bucket, Mid_विस्थापनम्,
                       &TP]() {
    bisect(दक्षिणः_Nodes, Rec_गभीरता + 1, दक्षिणः_Bucket, Mid_विस्थापनम्, TP);
  };

  यदि (TP && Rec_गभीरता < Config.Task_विभज_गभीरता && अङ्कः_Nodes >= 4) {
    TP->async(std::move(वामः_Rec_Task));
    TP->async(std::move(दक्षिणः_Rec_Task));
  } अन्यथा {
    वामः_Rec_Task();
    दक्षिणः_Rec_Task();
  }
}

शून्य BalancedPartitioning::सततम्_Iterations(ध्रुव Function_ग्रन्थिः_Range Nodes,
                                         अचिह्नित वामः_Bucket,
                                         अचिह्नित दक्षिणः_Bucket,
                                         std::mt19937 &RNG) ध्रुव {
  अचिह्नित अङ्कः_Nodes = llvm::आकार_2(Nodes);
  DenseMap<BPFunction_ग्रन्थिः::Utility_ग्रन्थिः_T, अचिह्नित> Utility_ग्रन्थिः_सूचकांक;
  प्रति (स्वचालित &N : Nodes)
    प्रति (स्वचालित &UN : N.UtilityNodes)
      ++Utility_ग्रन्थिः_सूचकांक[UN];
  // Remove utility nodes if they have just one edge or are connected to all
  // functions
  प्रति (स्वचालित &N : Nodes)
    llvm::erase_if(N.UtilityNodes, [&](स्वचालित &UN) {
      अचिह्नित UNI = Utility_ग्रन्थिः_सूचकांक[UN];
      प्रत्यागम UNI == 1 || UNI == अङ्कः_Nodes;
    });

  // Renumber utility nodes so they can be used to index into Signatures
  Utility_ग्रन्थिः_सूचकांक.clear();
  प्रति (स्वचालित &N : Nodes)
    प्रति (स्वचालित &UN : N.UtilityNodes)
      UN = Utility_ग्रन्थिः_सूचकांक.निवेशय({UN, Utility_ग्रन्थिः_सूचकांक.आकार_2()}).first->second;

  // Initialize signatures
  SignaturesT Signatures(/*Size=*/Utility_ग्रन्थिः_सूचकांक.आकार_2());
  प्रति (स्वचालित &N : Nodes) {
    प्रति (स्वचालित &UN : N.UtilityNodes) {
      प्रतिज्ञा(UN < Signatures.आकार_2());
      यदि (N.Bucket == वामः_Bucket) {
        Signatures[UN].वामः_गणनम्++;
      } अन्यथा {
        Signatures[UN].दक्षिणः_गणनम्++;
      }
    }
  }

  प्रति (अचिह्नित I = 0; I < Config.Iterations_प्रत्येकम्_विभज; I++) {
    अचिह्नित अङ्कः_Moved_Nodes =
        सततम्_Iteration(Nodes, वामः_Bucket, दक्षिणः_Bucket, Signatures, RNG);
    यदि (अङ्कः_Moved_Nodes == 0)
      भङ्ग;
  }
}

अचिह्नित BalancedPartitioning::सततम्_Iteration(ध्रुव Function_ग्रन्थिः_Range Nodes,
                                            अचिह्नित वामः_Bucket,
                                            अचिह्नित दक्षिणः_Bucket,
                                            SignaturesT &Signatures,
                                            std::mt19937 &RNG) ध्रुव {
  // Init signature cost caches
  प्रति (स्वचालित &Signature : Signatures) {
    यदि (Signature.CachedGainIsValid)
      अनुवर्त;
    अचिह्नित L = Signature.वामः_गणनम्;
    अचिह्नित R = Signature.दक्षिणः_गणनम्;
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
  उपयुज्य GainPair = std::pair<भिन्न, BPFunction_ग्रन्थिः *>;
  std::सदिशम्<GainPair> Gains;
  प्रति (स्वचालित &N : Nodes) {
    तार्किक From_वामः_गन्तव्यम्_दक्षिणः = (N.Bucket == वामः_Bucket);
    भिन्न Gain = moveGain(N, From_वामः_गन्तव्यम्_दक्षिणः, Signatures);
    Gains.प्रक्षिप_पृष्ठभागः(std::make_pair(Gain, &N));
  }

  // Collect left and right gains
  स्वचालित वामः_अन्तः = llvm::partition(
      Gains, [&](ध्रुव स्वचालित &GP) { प्रत्यागम GP.second->Bucket == वामः_Bucket; });
  स्वचालित वामः_Range = llvm::make_range(Gains.begin(), वामः_अन्तः);
  स्वचालित दक्षिणः_Range = llvm::make_range(वामः_अन्तः, Gains.अन्तः());

  // Sort gains in descending order
  स्वचालित LargerGain = [](ध्रुव स्वचालित &L, ध्रुव स्वचालित &R) {
    प्रत्यागम L.first > R.first;
  };
  llvm::stable_क्रमय(वामः_Range, LargerGain);
  llvm::stable_क्रमय(दक्षिणः_Range, LargerGain);

  अचिह्नित अङ्कः_Moved_दत्त_Vertices = 0;
  प्रति (स्वचालित [वामः_Pair, दक्षिणः_Pair] : llvm::zip(वामः_Range, दक्षिणः_Range)) {
    स्वचालित &[वामः_Gain, वामः_ग्रन्थिः] = वामः_Pair;
    स्वचालित &[दक्षिणः_Gain, दक्षिणः_ग्रन्थिः] = दक्षिणः_Pair;
    // Stop when the gain is no longer beneficial
    यदि (वामः_Gain + दक्षिणः_Gain <= 0.f)
      भङ्ग;
    // Try to exchange the nodes between buckets
    यदि (move_Function_ग्रन्थिः(*वामः_ग्रन्थिः, वामः_Bucket, दक्षिणः_Bucket, Signatures, RNG))
      ++अङ्कः_Moved_दत्त_Vertices;
    यदि (move_Function_ग्रन्थिः(*दक्षिणः_ग्रन्थिः, वामः_Bucket, दक्षिणः_Bucket, Signatures, RNG))
      ++अङ्कः_Moved_दत्त_Vertices;
  }
  प्रत्यागम अङ्कः_Moved_दत्त_Vertices;
}

तार्किक BalancedPartitioning::move_Function_ग्रन्थिः(BPFunction_ग्रन्थिः &N,
                                            अचिह्नित वामः_Bucket,
                                            अचिह्नित दक्षिणः_Bucket,
                                            SignaturesT &Signatures,
                                            std::mt19937 &RNG) ध्रुव {
  // Sometimes we skip the move. This helps to escape local optima
  यदि (std::एकरूपम्_real_distribution<भिन्न>(0.f, 1.f)(RNG) <=
      Config.SkipProbability)
    प्रत्यागम असत्य;

  तार्किक From_वामः_गन्तव्यम्_दक्षिणः = (N.Bucket == वामः_Bucket);
  // Update the current bucket
  N.Bucket = (From_वामः_गन्तव्यम्_दक्षिणः ? दक्षिणः_Bucket : वामः_Bucket);

  // Update signatures and invalidate gain cache
  यदि (From_वामः_गन्तव्यम्_दक्षिणः) {
    प्रति (स्वचालित &UN : N.UtilityNodes) {
      स्वचालित &Signature = Signatures[UN];
      Signature.वामः_गणनम्--;
      Signature.दक्षिणः_गणनम्++;
      Signature.CachedGainIsValid = असत्य;
    }
  } अन्यथा {
    प्रति (स्वचालित &UN : N.UtilityNodes) {
      स्वचालित &Signature = Signatures[UN];
      Signature.वामः_गणनम्++;
      Signature.दक्षिणः_गणनम्--;
      Signature.CachedGainIsValid = असत्य;
    }
  }
  प्रत्यागम सत्य;
}

शून्य BalancedPartitioning::विभज(ध्रुव Function_ग्रन्थिः_Range Nodes,
                                 अचिह्नित StartBucket) ध्रुव {
  अचिह्नित अङ्कः_Nodes = llvm::आकार_2(Nodes);
  स्वचालित NodesMid = Nodes.begin() + (अङ्कः_Nodes + 1) / 2;

  llvm::क्रमय(Nodes, [](स्वचालित &L, स्वचालित &R) {
    प्रत्यागम L.निवेशः_क्रमः_सूचकांक < R.निवेशः_क्रमः_सूचकांक;
  });

  प्रति (स्वचालित &N : llvm::make_range(Nodes.begin(), NodesMid))
    N.Bucket = StartBucket;
  प्रति (स्वचालित &N : llvm::make_range(NodesMid, Nodes.अन्तः()))
    N.Bucket = StartBucket + 1;
}

भिन्न BalancedPartitioning::moveGain(ध्रुव BPFunction_ग्रन्थिः &N,
                                     तार्किक From_वामः_गन्तव्यम्_दक्षिणः,
                                     ध्रुव SignaturesT &Signatures) {
  भिन्न Gain = 0.f;
  प्रति (स्वचालित &UN : N.UtilityNodes)
    Gain += (From_वामः_गन्तव्यम्_दक्षिणः ? Signatures[UN].CachedGainLR
                             : Signatures[UN].CachedGainRL);
  प्रत्यागम Gain;
}

भिन्न BalancedPartitioning::logCost(अचिह्नित X, अचिह्नित Y) ध्रुव {
  प्रत्यागम -(X * log2Cached(X + 1) + Y * log2Cached(Y + 1));
}

भिन्न BalancedPartitioning::log2Cached(अचिह्नित i) ध्रुव {
  प्रत्यागम (i < LOG_CACHE_आकार) ? Log2Cache[i] : std::log2(i);
}
