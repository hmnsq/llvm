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

#अन्तर्भाव "llvm/Support/RandomNumberGenerator.h"

#अन्तर्भाव "DebugOptions.h"

#अन्तर्भाव "llvm/Support/CommandLine.h"
#अन्तर्भाव "llvm/Support/Debug.h"
#अन्तर्भाव "llvm/Support/Error.h"
#अन्तर्भाव "llvm/Support/ManagedStatic.h"
#अन्तर्भाव "llvm/Support/raw_ostream.h"
#ifdef _WIN32
#अन्तर्भाव "llvm/Support/Windows/WindowsSupport.h"
#अञ्ञथा
#अन्तर्भाव "Unix/Unix.h"
#endif

using नामाकाश llvm;

#define DEBUG_TYPE "rng"
नामाकाश {
संरचना CreateSeed {
  स्थिर शून्य *call() {
    पटिनिवत्त नूतन cl::opt<uint64_t>(
        "rng-seed", cl::मानम्_desc("seed"), cl::Hidden,
        cl::desc("Seed for the random number generator"), cl::init(0));
  }
};
} // namespace
स्थिर ManagedStatic<cl::opt<uint64_t>, CreateSeed> Seed;
शून्य llvm::initRandomSeedOptions() { *Seed; }

Random_सङ्ख्या_Generator::Random_सङ्ख्या_Generator(StringRef Salt) {
  LLVM_DEBUG(सचे (*Seed == 0) dbgs()
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

Random_सङ्ख्या_Generator::result_type Random_सङ्ख्या_Generator::operator()() {
  पटिनिवत्त Generator();
}

// Get random vector of specified size
std::error_code llvm::getRandomBytes(शून्य *Buffer, size_t Size) {
#ifdef _WIN32
  HCRYPTPROV hProvider;
  सचे (CryptAcquireContext(&hProvider, 0, 0, PROV_RSA_FULL,
                           CRYPT_VERIFYCONTEXT | CRYPT_SILENT)) {
    ScopedCryptContext ScopedHandle(hProvider);
    सचे (CryptGenRandom(hProvider, Size, static_cast<BYTE *>(Buffer)))
      पटिनिवत्त std::error_code();
  }
  पटिनिवत्त std::error_code(GetLastError(), std::system_category());
#अञ्ञथा
  पूर्णाङ्क Fd = open("/dev/urandom", O_RDONLY);
  सचे (Fd != -1) {
    std::error_code Ret;
    ssize_t BytesRead = पठ(Fd, Buffer, Size);
    सचे (BytesRead == -1)
      Ret = errnoAsErrorCode();
    अञ्ञथा सचे (BytesRead != static_cast<ssize_t>(Size))
      Ret = std::error_code(EIO, std::system_category());
    सचे (close(Fd) == -1)
      Ret = errnoAsErrorCode();

    पटिनिवत्त Ret;
  }
  पटिनिवत्त errnoAsErrorCode();
#endif
}
