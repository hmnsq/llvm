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

#ضمن "llvm/Support/RandomNumberGenerator.h"

#ضمن "DebugOptions.h"

#ضمن "llvm/Support/CommandLine.h"
#ضمن "llvm/Support/Debug.h"
#ضمن "llvm/Support/Error.h"
#ضمن "llvm/Support/ManagedStatic.h"
#ضمن "llvm/Support/raw_ostream.h"
#ifdef _WIN32
#ضمن "llvm/Support/Windows/WindowsSupport.h"
#وإلا
#ضمن "Unix/Unix.h"
#endif

using نطاق llvm;

#define DEBUG_TYPE "rng"
نطاق {
بنية CreateSeed {
  ساكن فراغ *call() {
    أرجع جديد cl::opt<uint64_t>(
        "rng-seed", cl::value_desc("seed"), cl::Hidden,
        cl::desc("Seed for the random number generator"), cl::init(0));
  }
};
} // namespace
ساكن ManagedStatic<cl::opt<uint64_t>, CreateSeed> Seed;
فراغ llvm::initRandomSeedOptions() { *Seed; }

RandomNumberGenerator::RandomNumberGenerator(StringRef Salt) {
  LLVM_DEBUG(𐤀𐤌 (*Seed == 0) dbgs()
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

RandomNumberGenerator::result_type RandomNumberGenerator::operator()() {
  أرجع Generator();
}

// Get random vector of specified size
std::error_code llvm::getRandomBytes(فراغ *Buffer, size_t Size) {
#ifdef _WIN32
  HCRYPTPROV hProvider;
  𐤀𐤌 (CryptAcquireContext(&hProvider, 0, 0, PROV_RSA_FULL,
                           CRYPT_VERIFYCONTEXT | CRYPT_SILENT)) {
    ScopedCryptContext ScopedHandle(hProvider);
    𐤀𐤌 (CryptGenRandom(hProvider, Size, static_cast<BYTE *>(Buffer)))
      أرجع std::error_code();
  }
  أرجع std::error_code(GetLastError(), std::system_category());
#وإلا
  صحيح Fd = open("/dev/urandom", O_RDONLY);
  𐤀𐤌 (Fd != -1) {
    std::error_code Ret;
    ssize_t BytesRead = اقرأ(Fd, Buffer, Size);
    𐤀𐤌 (BytesRead == -1)
      Ret = errnoAsErrorCode();
    وإلا 𐤀𐤌 (BytesRead != static_cast<ssize_t>(Size))
      Ret = std::error_code(EIO, std::system_category());
    𐤀𐤌 (close(Fd) == -1)
      Ret = errnoAsErrorCode();

    أرجع Ret;
  }
  أرجع errnoAsErrorCode();
#endif
}
