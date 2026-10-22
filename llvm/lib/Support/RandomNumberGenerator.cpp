//===-- RandomNumberGenerator.cpp - Implement RNG class -------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file implements deterministic random number generation (RNG).
// The current implementation is NOT cryptographically secure as it uses
// the C++11 <random> facilities.
//
//===----------------------------------------------------------------------===//

#అన్తర్భావ "llvm/Support/RandomNumberGenerator.h"

#అన్తర్భావ "DebugOptions.h"

#అన్తర్భావ "llvm/Support/CommandLine.h"
#అన్తర్భావ "llvm/Support/Debug.h"
#అన్తర్భావ "llvm/Support/Error.h"
#అన్తర్భావ "llvm/Support/ManagedStatic.h"
#అన్తర్భావ "llvm/Support/raw_ostream.h"
#నిర్వచితమైతే _WIN32
#అన్తర్భావ "llvm/Support/Windows/WindowsSupport.h"
#లేకపోతే
#అన్తర్భావ "Unix/Unix.h"
#అయితే_ముగింపు

ఉపయోగించి నామాకాశ llvm;

#నిర్వచించు DEBUG_TYPE "rng"
నామాకాశ {
నిర్మాణం Create_విత్తనం {
  స్థిర శూన్య *call() {
    తిరిగి కొత్త cl::opt<uint64_t>(
        "rng-seed", cl::విలువ_desc("seed"), cl::Hidden,
        cl::desc("Seed for the random number generator"), cl::init(0));
  }
};
} // namespace
స్థిర ManagedStatic<cl::opt<uint64_t>, Create_విత్తనం> విత్తనం;
శూన్య llvm::init_Random_విత్తనం_Options() { *విత్తనం; }

Random_సంఖ్య_Generator::Random_సంఖ్య_Generator(StringRef Salt) {
  LLVM_DEBUG(ఒకవేళ (*విత్తనం == 0) dbgs()
             << "Warning! Using unseeded random number generator.\n");

  // Combine seed and salts using std::seed_seq.
  // Data: Seed-low, Seed-high, Salt
  // Note: std::seed_seq can only store 32-bit values, even though we
  // are using a 64-bit RNG. This isn't a problem since the Mersenne
  // twister constructor copies these correctly into its initial state.
  std::సదిశ<uint32_t> దత్తాంశం_2;
  దత్తాంశం_2.resize(2 + Salt.కొలత_2());
  దత్తాంశం_2[0] = *విత్తనం;
  దత్తాంశం_2[1] = *విత్తనం >> 32;

  llvm::copy(Salt, దత్తాంశం_2.begin() + 2);

  std::విత్తనం_seq విత్తనం_Seq(దత్తాంశం_2.begin(), దత్తాంశం_2.చివర());
  Generator.విత్తనం_2(విత్తనం_Seq);
}

Random_సంఖ్య_Generator::ఫలితం_type Random_సంఖ్య_Generator::క్రియాకారకం()() {
  తిరిగి Generator();
}

// Get random vector of specified size
std::error_సంకేతం llvm::get_Random_బైట్లు(శూన్య *బఫరు, size_t కొలత) {
#నిర్వచితమైతే _WIN32
  HCRYPTPROV hProvider;
  ఒకవేళ (CryptAcquireContext(&hProvider, 0, 0, PROV_RSA_నిండిన,
                           CRYPT_VERIFYCONTEXT | CRYPT_SILENT)) {
    ScopedCryptContext ScopedHandle(hProvider);
    ఒకవేళ (CryptGenRandom(hProvider, కొలత, static_cast<BYTE *>(బఫరు)))
      తిరిగి std::error_సంకేతం();
  }
  తిరిగి std::error_సంకేతం(Get_చివరి_Error(), std::system_category());
#లేకపోతే
  పూర్ణాంకం Fd = open("/dev/urandom", O_RDONLY);
  ఒకవేళ (Fd != -1) {
    std::error_సంకేతం Ret;
    ssize_t బైట్లు_Read = పఠ(Fd, బఫరు, కొలత);
    ఒకవేళ (బైట్లు_Read == -1)
      Ret = errno_As_Error_సంకేతం();
    లేకపోతే ఒకవేళ (బైట్లు_Read != static_cast<ssize_t>(కొలత))
      Ret = std::error_సంకేతం(EIO, std::system_category());
    ఒకవేళ (close(Fd) == -1)
      Ret = errno_As_Error_సంకేతం();

    తిరిగి Ret;
  }
  తిరిగి errno_As_Error_సంకేతం();
#అయితే_ముగింపు
}
