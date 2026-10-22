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
#اگر_تعریف_شدہ _WIN32
#شامل "llvm/Support/Windows/WindowsSupport.h"
#ورنہ
#شامل "Unix/Unix.h"
#اگر_ختم

استعمال نامخلا llvm;

#تعریف_کرو DEBUG_TYPE "rng"
نامخلا {
ساخت Create_بیج {
  ساکن خالی *call() {
    واپس نیا cl::opt<uint64_t>(
        "rng-seed", cl::قدر_desc("seed"), cl::Hidden,
        cl::desc("Seed for the random number generator"), cl::init(0));
  }
};
} // namespace
ساکن ManagedStatic<cl::opt<uint64_t>, Create_بیج> بیج;
خالی llvm::init_Random_بیج_Options() { *بیج; }

Random_نمبر_Generator::Random_نمبر_Generator(StringRef Salt) {
  LLVM_DEBUG(اگر (*بیج == 0) dbgs()
             << "Warning! Using unseeded random number generator.\n");

  // Combine seed and salts using std::seed_seq.
  // Data: Seed-low, Seed-high, Salt
  // Note: std::seed_seq can only store 32-bit values, even though we
  // are using a 64-bit RNG. This isn't a problem since the Mersenne
  // twister constructor copies these correctly into its initial state.
  std::سمتیہ<uint32_t> ڈیٹا_2;
  ڈیٹا_2.resize(2 + Salt.سائز_2());
  ڈیٹا_2[0] = *بیج;
  ڈیٹا_2[1] = *بیج >> 32;

  llvm::copy(Salt, ڈیٹا_2.begin() + 2);

  std::بیج_seq بیج_Seq(ڈیٹا_2.begin(), ڈیٹا_2.انتہا());
  Generator.بیج_2(بیج_Seq);
}

Random_نمبر_Generator::نتیجہ_type Random_نمبر_Generator::عامل()() {
  واپس Generator();
}

// Get random vector of specified size
std::error_رمز llvm::get_Random_بائٹیں(خالی *بفر, size_t سائز) {
#اگر_تعریف_شدہ _WIN32
  HCRYPTPROV hProvider;
  اگر (CryptAcquireContext(&hProvider, 0, 0, PROV_RSA_بھرا,
                           CRYPT_VERIFYCONTEXT | CRYPT_SILENT)) {
    ScopedCryptContext ScopedHandle(hProvider);
    اگر (CryptGenRandom(hProvider, سائز, static_cast<BYTE *>(بفر)))
      واپس std::error_رمز();
  }
  واپس std::error_رمز(Get_آخری_Error(), std::system_category());
#ورنہ
  صحیح Fd = open("/dev/urandom", O_RDONLY);
  اگر (Fd != -1) {
    std::error_رمز Ret;
    ssize_t بائٹیں_Read = اقرأ(Fd, بفر, سائز);
    اگر (بائٹیں_Read == -1)
      Ret = errno_As_Error_رمز();
    ورنہ اگر (بائٹیں_Read != static_cast<ssize_t>(سائز))
      Ret = std::error_رمز(EIO, std::system_category());
    اگر (close(Fd) == -1)
      Ret = errno_As_Error_رمز();

    واپس Ret;
  }
  واپس errno_As_Error_رمز();
#اگر_ختم
}
