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
#جيڪڏهن_وصف_ٿيل _WIN32
#ضمن "llvm/Support/Windows/WindowsSupport.h"
#وإلا
#ضمن "Unix/Unix.h"
#جيڪڏهن_پورو

استعمال نطاق llvm;

#وصف_ڪريو DEBUG_TYPE "rng"
نطاق {
بنية Create_ٻج {
  ساكن فراغ *call() {
    أرجع جديد cl::opt<uint64_t>(
        "rng-seed", cl::قدر_desc("seed"), cl::Hidden,
        cl::desc("Seed for the random number generator"), cl::init(0));
  }
};
} // namespace
ساكن ManagedStatic<cl::opt<uint64_t>, Create_ٻج> ٻج;
فراغ llvm::init_Random_ٻج_Options() { *ٻج; }

Random_عدد_Generator::Random_عدد_Generator(StringRef Salt) {
  LLVM_DEBUG(إذا (*ٻج == 0) dbgs()
             << "Warning! Using unseeded random number generator.\n");

  // Combine seed and salts using std::seed_seq.
  // Data: Seed-low, Seed-high, Salt
  // Note: std::seed_seq can only store 32-bit values, even though we
  // are using a 64-bit RNG. This isn't a problem since the Mersenne
  // twister constructor copies these correctly into its initial state.
  std::ويڪٽر<uint32_t> معلومات_2;
  معلومات_2.resize(2 + Salt.سائيز_2());
  معلومات_2[0] = *ٻج;
  معلومات_2[1] = *ٻج >> 32;

  llvm::copy(Salt, معلومات_2.begin() + 2);

  std::ٻج_seq ٻج_Seq(معلومات_2.begin(), معلومات_2.پڇاڙي());
  Generator.ٻج_2(ٻج_Seq);
}

Random_عدد_Generator::نتيجو_type Random_عدد_Generator::عامل()() {
  أرجع Generator();
}

// Get random vector of specified size
std::error_ڪوڊ llvm::get_Random_بائيٽون(فراغ *بفر, size_t سائيز) {
#جيڪڏهن_وصف_ٿيل _WIN32
  HCRYPTPROV hProvider;
  إذا (CryptAcquireContext(&hProvider, 0, 0, PROV_RSA_ڀريل,
                           CRYPT_VERIFYCONTEXT | CRYPT_SILENT)) {
    ScopedCryptContext ScopedHandle(hProvider);
    إذا (CryptGenRandom(hProvider, سائيز, static_cast<BYTE *>(بفر)))
      أرجع std::error_ڪوڊ();
  }
  أرجع std::error_ڪوڊ(Get_آخري_Error(), std::system_category());
#وإلا
  صحيح Fd = open("/dev/urandom", O_RDONLY);
  إذا (Fd != -1) {
    std::error_ڪوڊ Ret;
    ssize_t بائيٽون_Read = اقرأ(Fd, بفر, سائيز);
    إذا (بائيٽون_Read == -1)
      Ret = errno_As_Error_ڪوڊ();
    وإلا إذا (بائيٽون_Read != static_cast<ssize_t>(سائيز))
      Ret = std::error_ڪوڊ(EIO, std::system_category());
    إذا (close(Fd) == -1)
      Ret = errno_As_Error_ڪوڊ();

    أرجع Ret;
  }
  أرجع errno_As_Error_ڪوڊ();
#جيڪڏهن_پورو
}
