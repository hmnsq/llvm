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
#यदि_परिभाषितम् _WIN32
#अन्तर्भाव "llvm/Support/Windows/WindowsSupport.h"
#अन्यथा
#अन्तर्भाव "Unix/Unix.h"
#यदि_अन्तः

उपयुज्य नामाकाश llvm;

#परिभाषय DEBUG_TYPE "rng"
नामाकाश {
संरचना Create_बीजम् {
  स्थिर शून्य *call() {
    प्रत्यागम नूतन cl::opt<uint64_t>(
        "rng-seed", cl::मानम्_desc("seed"), cl::Hidden,
        cl::desc("Seed for the random number generator"), cl::init(0));
  }
};
} // namespace
स्थिर ManagedStatic<cl::opt<uint64_t>, Create_बीजम्> बीजम्;
शून्य llvm::init_Random_बीजम्_Options() { *बीजम्; }

Random_सङ्ख्या_Generator::Random_सङ्ख्या_Generator(StringRef Salt) {
  LLVM_DEBUG(यदि (*बीजम् == 0) dbgs()
             << "Warning! Using unseeded random number generator.\n");

  // Combine seed and salts using std::seed_seq.
  // Data: Seed-low, Seed-high, Salt
  // Note: std::seed_seq can only store 32-bit values, even though we
  // are using a 64-bit RNG. This isn't a problem since the Mersenne
  // twister constructor copies these correctly into its initial state.
  std::सदिशम्<uint32_t> दत्त_2;
  दत्त_2.resize(2 + Salt.आकार_2());
  दत्त_2[0] = *बीजम्;
  दत्त_2[1] = *बीजम् >> 32;

  llvm::copy(Salt, दत्त_2.begin() + 2);

  std::बीजम्_seq बीजम्_Seq(दत्त_2.begin(), दत्त_2.अन्तः());
  Generator.बीजम्_2(बीजम्_Seq);
}

Random_सङ्ख्या_Generator::परिणाम_type Random_सङ्ख्या_Generator::संक्रियकः()() {
  प्रत्यागम Generator();
}

// Get random vector of specified size
std::error_संहिता llvm::get_Random_अष्टकानि(शून्य *बफर, size_t आकार) {
#यदि_परिभाषितम् _WIN32
  HCRYPTPROV hProvider;
  यदि (CryptAcquireContext(&hProvider, 0, 0, PROV_RSA_पूर्णम्,
                           CRYPT_VERIFYCONTEXT | CRYPT_SILENT)) {
    ScopedCryptContext ScopedHandle(hProvider);
    यदि (CryptGenRandom(hProvider, आकार, static_cast<BYTE *>(बफर)))
      प्रत्यागम std::error_संहिता();
  }
  प्रत्यागम std::error_संहिता(Get_चरमः_Error(), std::system_category());
#अन्यथा
  पूर्णाङ्क Fd = उद्घाटय("/dev/urandom", O_RDONLY);
  यदि (Fd != -1) {
    std::error_संहिता Ret;
    ssize_t अष्टकानि_Read = पठ(Fd, बफर, आकार);
    यदि (अष्टकानि_Read == -1)
      Ret = errno_As_Error_संहिता();
    अन्यथा यदि (अष्टकानि_Read != static_cast<ssize_t>(आकार))
      Ret = std::error_संहिता(EIO, std::system_category());
    यदि (close(Fd) == -1)
      Ret = errno_As_Error_संहिता();

    प्रत्यागम Ret;
  }
  प्रत्यागम errno_As_Error_संहिता();
#यदि_अन्तः
}
