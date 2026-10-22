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

#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/RandomNumberGenerator.h"

#ਅਨ੍ਤਰ੍ਭਾਵ "DebugOptions.h"

#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/CommandLine.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/Debug.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/Error.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/ManagedStatic.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/raw_ostream.h"
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ _WIN32
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/Windows/WindowsSupport.h"
#ਨਹੀਂਤਾਂ
#ਅਨ੍ਤਰ੍ਭਾਵ "Unix/Unix.h"
#ਜੇ_ਅੰਤ

ਵਰਤੋਂ ਨਾਮਾਕਾਸ਼ llvm;

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ DEBUG_TYPE "rng"
ਨਾਮਾਕਾਸ਼ {
ਬਣਤਰ Create_ਬੀਜ {
  ਸਥਿਰ ਖਾਲੀ *call() {
    ਵਾਪਸ ਨਵਾਂ cl::opt<uint64_t>(
        "rng-seed", cl::ਮੁੱਲ_desc("seed"), cl::Hidden,
        cl::desc("Seed for the random number generator"), cl::init(0));
  }
};
} // namespace
ਸਥਿਰ ManagedStatic<cl::opt<uint64_t>, Create_ਬੀਜ> ਬੀਜ;
ਖਾਲੀ llvm::init_Random_ਬੀਜ_Options() { *ਬੀਜ; }

Random_ਸੰਖਿਆ_Generator::Random_ਸੰਖਿਆ_Generator(StringRef Salt) {
  LLVM_DEBUG(ਜੇ (*ਬੀਜ == 0) dbgs()
             << "Warning! Using unseeded random number generator.\n");

  // Combine seed and salts using std::seed_seq.
  // Data: Seed-low, Seed-high, Salt
  // Note: std::seed_seq can only store 32-bit values, even though we
  // are using a 64-bit RNG. This isn't a problem since the Mersenne
  // twister constructor copies these correctly into its initial state.
  std::ਸਦਿਸ਼<uint32_t> ਅੰਕੜੇ_2;
  ਅੰਕੜੇ_2.resize(2 + Salt.ਮਾਪ_2());
  ਅੰਕੜੇ_2[0] = *ਬੀਜ;
  ਅੰਕੜੇ_2[1] = *ਬੀਜ >> 32;

  llvm::copy(Salt, ਅੰਕੜੇ_2.begin() + 2);

  std::ਬੀਜ_seq ਬੀਜ_Seq(ਅੰਕੜੇ_2.begin(), ਅੰਕੜੇ_2.ਸਿਰਾ());
  Generator.ਬੀਜ_2(ਬੀਜ_Seq);
}

Random_ਸੰਖਿਆ_Generator::ਨਤੀਜਾ_type Random_ਸੰਖਿਆ_Generator::ਸੰਚਾਲਕ()() {
  ਵਾਪਸ Generator();
}

// Get random vector of specified size
std::error_ਕੋਡ llvm::get_Random_ਬਾਈਟਾਂ(ਖਾਲੀ *ਬਫਰ, size_t ਮਾਪ) {
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ _WIN32
  HCRYPTPROV hProvider;
  ਜੇ (CryptAcquireContext(&hProvider, 0, 0, PROV_RSA_ਭਰਿਆ,
                           CRYPT_VERIFYCONTEXT | CRYPT_SILENT)) {
    ScopedCryptContext ScopedHandle(hProvider);
    ਜੇ (CryptGenRandom(hProvider, ਮਾਪ, static_cast<BYTE *>(ਬਫਰ)))
      ਵਾਪਸ std::error_ਕੋਡ();
  }
  ਵਾਪਸ std::error_ਕੋਡ(Get_ਆਖਰੀ_Error(), std::system_category());
#ਨਹੀਂਤਾਂ
  ਪੂਰਨ Fd = open("/dev/urandom", O_RDONLY);
  ਜੇ (Fd != -1) {
    std::error_ਕੋਡ Ret;
    ssize_t ਬਾਈਟਾਂ_Read = ਪਠ(Fd, ਬਫਰ, ਮਾਪ);
    ਜੇ (ਬਾਈਟਾਂ_Read == -1)
      Ret = errno_As_Error_ਕੋਡ();
    ਨਹੀਂਤਾਂ ਜੇ (ਬਾਈਟਾਂ_Read != static_cast<ssize_t>(ਮਾਪ))
      Ret = std::error_ਕੋਡ(EIO, std::system_category());
    ਜੇ (close(Fd) == -1)
      Ret = errno_As_Error_ਕੋਡ();

    ਵਾਪਸ Ret;
  }
  ਵਾਪਸ errno_As_Error_ਕੋਡ();
#ਜੇ_ਅੰਤ
}
