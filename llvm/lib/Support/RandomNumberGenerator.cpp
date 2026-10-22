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

#அந்தர்பாவ "llvm/Support/RandomNumberGenerator.h"

#அந்தர்பாவ "DebugOptions.h"

#அந்தர்பாவ "llvm/Support/CommandLine.h"
#அந்தர்பாவ "llvm/Support/Debug.h"
#அந்தர்பாவ "llvm/Support/Error.h"
#அந்தர்பாவ "llvm/Support/ManagedStatic.h"
#அந்தர்பாவ "llvm/Support/raw_ostream.h"
#ifdef _WIN32
#அந்தர்பாவ "llvm/Support/Windows/WindowsSupport.h"
#இல்லையெனில்
#அந்தர்பாவ "Unix/Unix.h"
#endif

using நாமாகாஶ llvm;

#define DEBUG_TYPE "rng"
நாமாகாஶ {
கட்டமைப்பு CreateSeed {
  நிலையான வெறுமை *call() {
    திரும்பு புதிய cl::opt<uint64_t>(
        "rng-seed", cl::மதிப்பு_desc("seed"), cl::Hidden,
        cl::desc("Seed for the random number generator"), cl::init(0));
  }
};
} // namespace
நிலையான ManagedStatic<cl::opt<uint64_t>, CreateSeed> Seed;
வெறுமை llvm::initRandomSeedOptions() { *Seed; }

Random_எண்_Generator::Random_எண்_Generator(StringRef Salt) {
  LLVM_DEBUG(எனில் (*Seed == 0) dbgs()
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

Random_எண்_Generator::result_type Random_எண்_Generator::operator()() {
  திரும்பு Generator();
}

// Get random vector of specified size
std::error_code llvm::getRandomBytes(வெறுமை *Buffer, size_t Size) {
#ifdef _WIN32
  HCRYPTPROV hProvider;
  எனில் (CryptAcquireContext(&hProvider, 0, 0, PROV_RSA_FULL,
                           CRYPT_VERIFYCONTEXT | CRYPT_SILENT)) {
    ScopedCryptContext ScopedHandle(hProvider);
    எனில் (CryptGenRandom(hProvider, Size, static_cast<BYTE *>(Buffer)))
      திரும்பு std::error_code();
  }
  திரும்பு std::error_code(GetLastError(), std::system_category());
#இல்லையெனில்
  முழு Fd = open("/dev/urandom", O_RDONLY);
  எனில் (Fd != -1) {
    std::error_code Ret;
    ssize_t BytesRead = பட(Fd, Buffer, Size);
    எனில் (BytesRead == -1)
      Ret = errnoAsErrorCode();
    இல்லையெனில் எனில் (BytesRead != static_cast<ssize_t>(Size))
      Ret = std::error_code(EIO, std::system_category());
    எனில் (close(Fd) == -1)
      Ret = errnoAsErrorCode();

    திரும்பு Ret;
  }
  திரும்பு errnoAsErrorCode();
#endif
}
