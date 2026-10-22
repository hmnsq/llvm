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

#شامل "llvm/Support/RandomNumberGenerator.h"

#شامل "DebugOptions.h"

#شامل "llvm/Support/CommandLine.h"
#شامل "llvm/Support/Debug.h"
#شامل "llvm/Support/Error.h"
#شامل "llvm/Support/ManagedStatic.h"
#شامل "llvm/Support/raw_ostream.h"
#ifdef _WIN32
#شامل "llvm/Support/Windows/WindowsSupport.h"
#وگرنه
#شامل "Unix/Unix.h"
#endif

using فضاینام llvm;

#define DEBUG_TYPE "rng"
فضاینام {
ساختار CreateSeed {
  ایستا تهی *call() {
    بازگشت جدید cl::opt<uint64_t>(
        "rng-seed", cl::مقدار_desc("seed"), cl::Hidden,
        cl::desc("Seed for the random number generator"), cl::init(0));
  }
};
} // namespace
ایستا ManagedStatic<cl::opt<uint64_t>, CreateSeed> Seed;
تهی llvm::initRandomSeedOptions() { *Seed; }

Random_عدد_Generator::Random_عدد_Generator(StringRef Salt) {
  LLVM_DEBUG(اگر (*Seed == 0) dbgs()
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

Random_عدد_Generator::result_type Random_عدد_Generator::operator()() {
  بازگشت Generator();
}

// Get random vector of specified size
std::error_code llvm::getRandomBytes(تهی *Buffer, size_t Size) {
#ifdef _WIN32
  HCRYPTPROV hProvider;
  اگر (CryptAcquireContext(&hProvider, 0, 0, PROV_RSA_FULL,
                           CRYPT_VERIFYCONTEXT | CRYPT_SILENT)) {
    ScopedCryptContext ScopedHandle(hProvider);
    اگر (CryptGenRandom(hProvider, Size, static_cast<BYTE *>(Buffer)))
      بازگشت std::error_code();
  }
  بازگشت std::error_code(GetLastError(), std::system_category());
#وگرنه
  صحیح Fd = open("/dev/urandom", O_RDONLY);
  اگر (Fd != -1) {
    std::error_code Ret;
    ssize_t BytesRead = اقرأ(Fd, Buffer, Size);
    اگر (BytesRead == -1)
      Ret = errnoAsErrorCode();
    وگرنه اگر (BytesRead != static_cast<ssize_t>(Size))
      Ret = std::error_code(EIO, std::system_category());
    اگر (close(Fd) == -1)
      Ret = errnoAsErrorCode();

    بازگشت Ret;
  }
  بازگشت errnoAsErrorCode();
#endif
}
