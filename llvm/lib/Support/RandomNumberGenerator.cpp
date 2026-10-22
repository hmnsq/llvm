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

#ଅନ୍ତର୍ଭାଵ "llvm/Support/RandomNumberGenerator.h"

#ଅନ୍ତର୍ଭାଵ "DebugOptions.h"

#ଅନ୍ତର୍ଭାଵ "llvm/Support/CommandLine.h"
#ଅନ୍ତର୍ଭାଵ "llvm/Support/Debug.h"
#ଅନ୍ତର୍ଭାଵ "llvm/Support/Error.h"
#ଅନ୍ତର୍ଭାଵ "llvm/Support/ManagedStatic.h"
#ଅନ୍ତର୍ଭାଵ "llvm/Support/raw_ostream.h"
#ifdef _WIN32
#ଅନ୍ତର୍ଭାଵ "llvm/Support/Windows/WindowsSupport.h"
#ଅନ୍ୟଥା
#ଅନ୍ତର୍ଭାଵ "Unix/Unix.h"
#endif

using ନାମାକାଶ llvm;

#define DEBUG_TYPE "rng"
ନାମାକାଶ {
ସଂରଚନା CreateSeed {
  ସ୍ଥିର ଶୂନ୍ୟ *call() {
    ଫେରନ୍ତୁ ନୂତନ cl::opt<uint64_t>(
        "rng-seed", cl::ମୂଲ୍ୟ_desc("seed"), cl::Hidden,
        cl::desc("Seed for the random number generator"), cl::init(0));
  }
};
} // namespace
ସ୍ଥିର ManagedStatic<cl::opt<uint64_t>, CreateSeed> Seed;
ଶୂନ୍ୟ llvm::initRandomSeedOptions() { *Seed; }

Random_ସଂଖ୍ୟା_Generator::Random_ସଂଖ୍ୟା_Generator(StringRef Salt) {
  LLVM_DEBUG(ଯଦି (*Seed == 0) dbgs()
             << "Warning! Using unseeded random number generator.\n");

  // Combine seed and salts using std::seed_seq.
  // Data: Seed-low, Seed-high, Salt
  // Note: std::seed_seq can only store 32-bit values, even though we
  // are using a 64-bit RNG. This isn't a problem since the Mersenne
  // twister constructor copies these correctly into its initial state.
  std::vector<uint32_t> Data;
  Data.resize(2 + Salt.size());
  Data[0] = *Seed;
  Data[1] = *Seed >> 32;

  llvm::copy(Salt, Data.begin() + 2);

  std::seed_seq SeedSeq(Data.begin(), Data.end());
  Generator.seed(SeedSeq);
}

Random_ସଂଖ୍ୟା_Generator::result_type Random_ସଂଖ୍ୟା_Generator::operator()() {
  ଫେରନ୍ତୁ Generator();
}

// Get random vector of specified size
std::error_code llvm::getRandomBytes(ଶୂନ୍ୟ *Buffer, size_t Size) {
#ifdef _WIN32
  HCRYPTPROV hProvider;
  ଯଦି (CryptAcquireContext(&hProvider, 0, 0, PROV_RSA_FULL,
                           CRYPT_VERIFYCONTEXT | CRYPT_SILENT)) {
    ScopedCryptContext ScopedHandle(hProvider);
    ଯଦି (CryptGenRandom(hProvider, Size, static_cast<BYTE *>(Buffer)))
      ଫେରନ୍ତୁ std::error_code();
  }
  ଫେରନ୍ତୁ std::error_code(GetLastError(), std::system_category());
#ଅନ୍ୟଥା
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା Fd = open("/dev/urandom", O_RDONLY);
  ଯଦି (Fd != -1) {
    std::error_code Ret;
    ssize_t BytesRead = ପଠ(Fd, Buffer, Size);
    ଯଦି (BytesRead == -1)
      Ret = errnoAsErrorCode();
    ଅନ୍ୟଥା ଯଦି (BytesRead != static_cast<ssize_t>(Size))
      Ret = std::error_code(EIO, std::system_category());
    ଯଦି (close(Fd) == -1)
      Ret = errnoAsErrorCode();

    ଫେରନ୍ତୁ Ret;
  }
  ଫେରନ୍ତୁ errnoAsErrorCode();
#endif
}
