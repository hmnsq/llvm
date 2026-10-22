//===--- MemoryBuffer.cpp - Memory Buffer implementation ------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
//  This file implements the MemoryBuffer interface.
//
//===----------------------------------------------------------------------===//

#ಅನ್ತರ್ಭಾವ "llvm/Support/MemoryBuffer.h"
#ಅನ್ತರ್ಭಾವ "llvm/ADT/STLExtras.h"
#ಅನ್ತರ್ಭಾವ "llvm/ADT/SmallString.h"
#ಅನ್ತರ್ಭಾವ "llvm/Config/config.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/Alignment.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/AutoConvert.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/Errc.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/Error.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/ErrorHandling.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/FileSystem.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/IOSandbox.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/Process.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/Program.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/SmallVectorMemoryBuffer.h"
#ಅನ್ತರ್ಭಾವ <algorithm>
#ಅನ್ತರ್ಭಾವ <cassert>
#ಅನ್ತರ್ಭಾವ <cstring>
#ಅನ್ತರ್ಭಾವ <new>
#ಅನ್ತರ್ಭಾವ <sys/types.h>
#ಅನ್ತರ್ಭಾವ <system_error>
#ಆದರೆ !defined(_MSC_VER) && !defined(__MINGW32__)
#ಅನ್ತರ್ಭಾವ <unistd.h>
#ಇಲ್ಲದಿದ್ದರೆ
#ಅನ್ತರ್ಭಾವ <io.h>
#ಆದರೆ_ಅಂತ್ಯ

ಬಳಸಿ ನಾಮಾಕಾಶ llvm;

//===----------------------------------------------------------------------===//
// MemoryBuffer implementation itself.
//===----------------------------------------------------------------------===//

ಸ್ಮರಣೆ_ಬಫರ್::~ಸ್ಮರಣೆ_ಬಫರ್() = ಪೂರ್ವನಿಯೋಜಿತ;

/// init - Initialize this MemoryBuffer as a reference to externally allocated
/// memory, memory that we know is already null terminated.
ಶೂನ್ಯ ಸ್ಮರಣೆ_ಬಫರ್::init(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *Buf_ಆರಂಭ, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *Buf_ಕೊನೆ,
                        ತಾರ್ಕಿಕ RequiresNullTerminator) {
  ಪ್ರತಿಜ್ಞಾ((!RequiresNullTerminator || Buf_ಕೊನೆ[0] == 0) &&
         "Buffer is not null terminated!");
  ಬಫರ್_ಆರಂಭ = Buf_ಆರಂಭ;
  ಬಫರ್_ಕೊನೆ = Buf_ಕೊನೆ;
}

//===----------------------------------------------------------------------===//
// MemoryBufferMem implementation.
//===----------------------------------------------------------------------===//

/// CopyStringRef - Copies contents of a StringRef into a block of memory and
/// null-terminates it.
ಸ್ಥಿರ ಶೂನ್ಯ CopyStringRef(ಅಕ್ಷರ *ಸ್ಮರಣೆ_2, StringRef ದತ್ತಾಂಶ_2) {
  ಆದರೆ (!ದತ್ತಾಂಶ_2.ಖಾಲಿಗಳು())
    ಸ್ಮರಣೆ_ನಕಲು(ಸ್ಮರಣೆ_2, ದತ್ತಾಂಶ_2.ದತ್ತಾಂಶ(), ದತ್ತಾಂಶ_2.ಗಾತ್ರ_3());
  ಸ್ಮರಣೆ_2[ದತ್ತಾಂಶ_2.ಗಾತ್ರ_3()] = 0; // Null terminate string.
}

ನಾಮಾಕಾಶ {
ರಚನೆ Named_ಬಫರ್_Alloc {
  ಸ್ಥಿರಾಂಕ Twine &Name;
  Named_ಬಫರ್_Alloc(ಸ್ಥಿರಾಂಕ Twine &Name) : Name(Name) {}
};
} // namespace

ಶೂನ್ಯ *ಕ್ರಿಯಾಕಾರಕ ಹೊಸ(size_t N, ಸ್ಥಿರಾಂಕ Named_ಬಫರ್_Alloc &Alloc) {
  SmallString<256> NameBuf;
  StringRef NameRef = Alloc.Name.ಗಮ್ಯ_String_Ref(NameBuf);

  // We use malloc() and manually handle it returning null instead of calling
  // operator new because we need all uses of NamedBufferAlloc to be
  // deallocated with a call to free() due to needing to use malloc() in
  // WritableMemoryBuffer::getNewUninitMemBuffer() to work around the out-of-
  // memory handler installed by default in LLVM. See operator delete() member
  // functions within this file for the paired call to free().
  ಅಕ್ಷರ *Mem =
      static_cast<ಅಕ್ಷರ *>(std::ಸ್ಮರಣೆ_ಕೊಡು(N + ಗಾತ್ರ(size_t) + NameRef.ಗಾತ್ರ_3() + 1));
  ಆದರೆ (!Mem)
    llvm::report_bad_alloc_error("Allocation failed");
  *reinterpret_cast<size_t *>(Mem + N) = NameRef.ಗಾತ್ರ_3();
  CopyStringRef(Mem + N + ಗಾತ್ರ(size_t), NameRef);
  ಹಿಂತಿರುಗಿ Mem;
}

ನಾಮಾಕಾಶ {
/// MemoryBufferMem - Named MemoryBuffer pointing to a block of memory.
ಮಾದರಿ<ಬಗೆ_ಹೆಸರು MB>
ವರ್ಗ ಸ್ಮರಣೆ_ಬಫರ್_Mem : ಸಾರ್ವಜನಿಕ MB {
ಸಾರ್ವಜನಿಕ:
  ಸ್ಮರಣೆ_ಬಫರ್_Mem(StringRef ಒಳಹರಿವು_ದತ್ತಾಂಶ, ತಾರ್ಕಿಕ RequiresNullTerminator) {
    ಸ್ಮರಣೆ_ಬಫರ್::init(ಒಳಹರಿವು_ದತ್ತಾಂಶ.begin(), ಒಳಹರಿವು_ದತ್ತಾಂಶ.ಕೊನೆ(),
                       RequiresNullTerminator);
  }

  /// Disable sized deallocation for MemoryBufferMem, because it has
  /// tail-allocated data.
  ಶೂನ್ಯ ಕ್ರಿಯಾಕಾರಕ ಅಳಿಸು(ಶೂನ್ಯ *p) { std::ಬಿಡುಗಡೆ(p); }

  StringRef get_ಬಫರ್_Identifier() ಸ್ಥಿರಾಂಕ override {
    // The name is stored after the class itself.
    ಹಿಂತಿರುಗಿ StringRef(reinterpret_cast<ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *>(ಇದು + 1) + ಗಾತ್ರ(size_t),
                     *reinterpret_cast<ಸ್ಥಿರಾಂಕ size_t *>(ಇದು + 1));
  }

  ಸ್ಮರಣೆ_ಬಫರ್::ಬಫರ್_ಬಗೆ get_ಬಫರ್_ಬಗೆ() ಸ್ಥಿರಾಂಕ override {
    ಹಿಂತಿರುಗಿ ಸ್ಮರಣೆ_ಬಫರ್::ಸ್ಮರಣೆ_ಬಫರ್_Malloc;
  }
};
} // namespace

ಮಾದರಿ <ಬಗೆ_ಹೆಸರು MB>
ಸ್ಥಿರ ErrorOr<std::unique_ptr<MB>>
getFileAux(ಸ್ಥಿರಾಂಕ Twine &Filename, uint64_t Map_ಗಾತ್ರ, uint64_t ಸ್ಥಾನಾಂತರ,
           ತಾರ್ಕಿಕ Is_ಪಠ್ಯ, ತಾರ್ಕಿಕ RequiresNullTerminator, ತಾರ್ಕಿಕ IsVolatile,
           std::optional<Align> Alignment);

std::unique_ptr<ಸ್ಮರಣೆ_ಬಫರ್>
ಸ್ಮರಣೆ_ಬಫರ್::get_Mem_ಬಫರ್(StringRef ಒಳಹರಿವು_ದತ್ತಾಂಶ, StringRef ಬಫರ್_Name,
                           ತಾರ್ಕಿಕ RequiresNullTerminator) {
  ಸ್ವಯಂಚಾಲಿತ *Ret = ಹೊಸ (Named_ಬಫರ್_Alloc(ಬಫರ್_Name))
      ಸ್ಮರಣೆ_ಬಫರ್_Mem<ಸ್ಮರಣೆ_ಬಫರ್>(ಒಳಹರಿವು_ದತ್ತಾಂಶ, RequiresNullTerminator);
  ಹಿಂತಿರುಗಿ std::unique_ptr<ಸ್ಮರಣೆ_ಬಫರ್>(Ret);
}

std::unique_ptr<ಸ್ಮರಣೆ_ಬಫರ್>
ಸ್ಮರಣೆ_ಬಫರ್::get_Mem_ಬಫರ್(ಸ್ಮರಣೆ_ಬಫರ್_Ref Ref, ತಾರ್ಕಿಕ RequiresNullTerminator) {
  ಹಿಂತಿರುಗಿ std::unique_ptr<ಸ್ಮರಣೆ_ಬಫರ್>(get_Mem_ಬಫರ್(
      Ref.get_ಬಫರ್(), Ref.get_ಬಫರ್_Identifier(), RequiresNullTerminator));
}

ಸ್ಥಿರ ErrorOr<std::unique_ptr<Writable_ಸ್ಮರಣೆ_ಬಫರ್>>
get_Mem_ಬಫರ್_Copy_Impl(StringRef ಒಳಹರಿವು_ದತ್ತಾಂಶ, ಸ್ಥಿರಾಂಕ Twine &ಬಫರ್_Name) {
  ಸ್ವಯಂಚಾಲಿತ Buf =
      Writable_ಸ್ಮರಣೆ_ಬಫರ್::get_New_Uninit_Mem_ಬಫರ್(ಒಳಹರಿವು_ದತ್ತಾಂಶ.ಗಾತ್ರ_3(), ಬಫರ್_Name);
  ಆದರೆ (!Buf)
    ಹಿಂತಿರುಗಿ make_error_ಸಂಕೇತ(errc::not_enough_ಸ್ಮರಣೆ);
  // Calling memcpy with null src/dst is UB, and an empty StringRef is
  // represented with {nullptr, 0}.
  llvm::copy(ಒಳಹರಿವು_ದತ್ತಾಂಶ, Buf->get_ಬಫರ್_ಆರಂಭ());
  ಹಿಂತಿರುಗಿ std::move(Buf);
}

std::unique_ptr<ಸ್ಮರಣೆ_ಬಫರ್>
ಸ್ಮರಣೆ_ಬಫರ್::get_Mem_ಬಫರ್_Copy(StringRef ಒಳಹರಿವು_ದತ್ತಾಂಶ, ಸ್ಥಿರಾಂಕ Twine &ಬಫರ್_Name) {
  ಸ್ವಯಂಚಾಲಿತ Buf = get_Mem_ಬಫರ್_Copy_Impl(ಒಳಹರಿವು_ದತ್ತಾಂಶ, ಬಫರ್_Name);
  ಆದರೆ (Buf)
    ಹಿಂತಿರುಗಿ std::move(*Buf);
  ಹಿಂತಿರುಗಿ ಶೂನ್ಯಸೂಚಕ;
}

ErrorOr<std::unique_ptr<ಸ್ಮರಣೆ_ಬಫರ್>>
ಸ್ಮರಣೆ_ಬಫರ್::getFileOrSTDIN(ಸ್ಥಿರಾಂಕ Twine &Filename, ತಾರ್ಕಿಕ Is_ಪಠ್ಯ,
                             ತಾರ್ಕಿಕ RequiresNullTerminator,
                             std::optional<Align> Alignment) {
  sys::sandbox::violationIfEnabled();

  SmallString<256> NameBuf;
  StringRef NameRef = Filename.ಗಮ್ಯ_String_Ref(NameBuf);

  ಆದರೆ (NameRef == "-")
    ಹಿಂತಿರುಗಿ getSTDIN();
  ಹಿಂತಿರುಗಿ getFile(Filename, Is_ಪಠ್ಯ, RequiresNullTerminator,
                 /*IsVolatile=*/ಸುಳ್ಳು, Alignment);
}

ErrorOr<std::unique_ptr<ಸ್ಮರಣೆ_ಬಫರ್>>
ಸ್ಮರಣೆ_ಬಫರ್::get_File_ತುಂಡು(ಸ್ಥಿರಾಂಕ Twine &File_ಮಾರ್ಗ, uint64_t Map_ಗಾತ್ರ,
                           uint64_t ಸ್ಥಾನಾಂತರ, ತಾರ್ಕಿಕ IsVolatile,
                           std::optional<Align> Alignment) {
  sys::sandbox::violationIfEnabled();

  ಹಿಂತಿರುಗಿ getFileAux<ಸ್ಮರಣೆ_ಬಫರ್>(File_ಮಾರ್ಗ, Map_ಗಾತ್ರ, ಸ್ಥಾನಾಂತರ, /*IsText=*/ಸುಳ್ಳು,
                                  /*RequiresNullTerminator=*/ಸುಳ್ಳು, IsVolatile,
                                  Alignment);
}

//===----------------------------------------------------------------------===//
// MemoryBuffer::getFile implementation.
//===----------------------------------------------------------------------===//

ನಾಮಾಕಾಶ {

ಮಾದರಿ <ಬಗೆ_ಹೆಸರು MB>
constexpr sys::fs::mapped_file_region::mapmode Mapmode =
    sys::fs::mapped_file_region::readonly;
ಮಾದರಿ <>
constexpr sys::fs::mapped_file_region::mapmode Mapmode<ಸ್ಮರಣೆ_ಬಫರ್> =
    sys::fs::mapped_file_region::readonly;
ಮಾದರಿ <>
constexpr sys::fs::mapped_file_region::mapmode Mapmode<Writable_ಸ್ಮರಣೆ_ಬಫರ್> =
    sys::fs::mapped_file_region::priv;
ಮಾದರಿ <>
constexpr sys::fs::mapped_file_region::mapmode
    Mapmode<Write_Through_ಸ್ಮರಣೆ_ಬಫರ್> = sys::fs::mapped_file_region::readwrite;

/// Memory maps a file descriptor using sys::fs::mapped_file_region.
///
/// This handles converting the offset into a legal offset on the platform.
ಮಾದರಿ<ಬಗೆ_ಹೆಸರು MB>
ವರ್ಗ ಸ್ಮರಣೆ_ಬಫರ್_MMap_File : ಸಾರ್ವಜನಿಕ MB {
  sys::fs::mapped_file_region MFR;

  ಸ್ಥಿರ uint64_t get_Legal_Map_ಸ್ಥಾನಾಂತರ(uint64_t ಸ್ಥಾನಾಂತರ) {
    ಹಿಂತಿರುಗಿ ಸ್ಥಾನಾಂತರ & ~(sys::fs::mapped_file_region::alignment() - 1);
  }

  ಸ್ಥಿರ uint64_t get_Legal_Map_ಗಾತ್ರ(uint64_t Len, uint64_t ಸ್ಥಾನಾಂತರ) {
    ಹಿಂತಿರುಗಿ Len + (ಸ್ಥಾನಾಂತರ - get_Legal_Map_ಸ್ಥಾನಾಂತರ(ಸ್ಥಾನಾಂತರ));
  }

  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *get_ಆರಂಭ(uint64_t Len, uint64_t ಸ್ಥಾನಾಂತರ) {
    ಹಿಂತಿರುಗಿ MFR.const_ದತ್ತಾಂಶ() + (ಸ್ಥಾನಾಂತರ - get_Legal_Map_ಸ್ಥಾನಾಂತರ(ಸ್ಥಾನಾಂತರ));
  }

ಸಾರ್ವಜನಿಕ:
  ಸ್ಮರಣೆ_ಬಫರ್_MMap_File(ತಾರ್ಕಿಕ RequiresNullTerminator, sys::fs::file_t FD, uint64_t Len,
                       uint64_t ಸ್ಥಾನಾಂತರ, std::error_ಸಂಕೇತ &EC)
      : MFR(FD, Mapmode<MB>, get_Legal_Map_ಗಾತ್ರ(Len, ಸ್ಥಾನಾಂತರ),
            get_Legal_Map_ಸ್ಥಾನಾಂತರ(ಸ್ಥಾನಾಂತರ), EC) {
    ಆದರೆ (!EC) {
      ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಆರಂಭ = get_ಆರಂಭ(Len, ಸ್ಥಾನಾಂತರ);
      ಸ್ಮರಣೆ_ಬಫರ್::init(ಆರಂಭ, ಆರಂಭ + Len, RequiresNullTerminator);
    }
  }

  /// Disable sized deallocation for MemoryBufferMMapFile, because it has
  /// tail-allocated data.
  ಶೂನ್ಯ ಕ್ರಿಯಾಕಾರಕ ಅಳಿಸು(ಶೂನ್ಯ *p) { std::ಬಿಡುಗಡೆ(p); }

  StringRef get_ಬಫರ್_Identifier() ಸ್ಥಿರಾಂಕ override {
    // The name is stored after the class itself.
    ಹಿಂತಿರುಗಿ StringRef(reinterpret_cast<ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *>(ಇದು + 1) + ಗಾತ್ರ(size_t),
                     *reinterpret_cast<ಸ್ಥಿರಾಂಕ size_t *>(ಇದು + 1));
  }

  ಸ್ಮರಣೆ_ಬಫರ್::ಬಫರ್_ಬಗೆ get_ಬಫರ್_ಬಗೆ() ಸ್ಥಿರಾಂಕ override {
    ಹಿಂತಿರುಗಿ ಸ್ಮರಣೆ_ಬಫರ್::ಸ್ಮರಣೆ_ಬಫರ್_MMap;
  }

  ಶೂನ್ಯ dontNeedIfMmap() override { MFR.dontNeed(); }
  ಶೂನ್ಯ willNeedIfMmap() override { MFR.willNeed(); }
  ಶೂನ್ಯ randomAccessIfMmap() override { MFR.randomAccess(); }
};
} // namespace

ಸ್ಥಿರ ErrorOr<std::unique_ptr<Writable_ಸ್ಮರಣೆ_ಬಫರ್>>
get_ಸ್ಮರಣೆ_ಬಫರ್_For_ಹರಿವು(sys::fs::file_t FD, ಸ್ಥಿರಾಂಕ Twine &ಬಫರ್_Name) {
  SmallString<sys::fs::Default_Read_Chunk_ಗಾತ್ರ> ಬಫರ್;
  ಆದರೆ (Error E = sys::fs::read_Native_File_ಗಮ್ಯ_EOF(FD, ಬಫರ್))
    ಹಿಂತಿರುಗಿ error_ಗಮ್ಯ_Error_ಸಂಕೇತ(std::move(E));
  ಹಿಂತಿರುಗಿ get_Mem_ಬಫರ್_Copy_Impl(ಬಫರ್, ಬಫರ್_Name);
}

ErrorOr<std::unique_ptr<ಸ್ಮರಣೆ_ಬಫರ್>>
ಸ್ಮರಣೆ_ಬಫರ್::getFile(ಸ್ಥಿರಾಂಕ Twine &Filename, ತಾರ್ಕಿಕ Is_ಪಠ್ಯ,
                      ತಾರ್ಕಿಕ RequiresNullTerminator, ತಾರ್ಕಿಕ IsVolatile,
                      std::optional<Align> Alignment) {
  sys::sandbox::violationIfEnabled();

  ಹಿಂತಿರುಗಿ getFileAux<ಸ್ಮರಣೆ_ಬಫರ್>(Filename, /*MapSize=*/-1, /*Offset=*/0,
                                  Is_ಪಠ್ಯ, RequiresNullTerminator, IsVolatile,
                                  Alignment);
}

ಮಾದರಿ <ಬಗೆ_ಹೆಸರು MB>
ಸ್ಥಿರ ErrorOr<std::unique_ptr<MB>>
getOpenFileImpl(sys::fs::file_t FD, ಸ್ಥಿರಾಂಕ Twine &Filename, uint64_t File_ಗಾತ್ರ,
                uint64_t Map_ಗಾತ್ರ, int64_t ಸ್ಥಾನಾಂತರ, ತಾರ್ಕಿಕ RequiresNullTerminator,
                ತಾರ್ಕಿಕ IsVolatile, std::optional<Align> Alignment);

ಮಾದರಿ <ಬಗೆ_ಹೆಸರು MB>
ಸ್ಥಿರ ErrorOr<std::unique_ptr<MB>>
getFileAux(ಸ್ಥಿರಾಂಕ Twine &Filename, uint64_t Map_ಗಾತ್ರ, uint64_t ಸ್ಥಾನಾಂತರ,
           ತಾರ್ಕಿಕ Is_ಪಠ್ಯ, ತಾರ್ಕಿಕ RequiresNullTerminator, ತಾರ್ಕಿಕ IsVolatile,
           std::optional<Align> Alignment) {
  Expected<sys::fs::file_t> FDOrErr = sys::fs::openNativeFileForRead(
      Filename, Is_ಪಠ್ಯ ? sys::fs::ದ_ಪಠ್ಯ_With_CRLF : sys::fs::ದ_None);
  ಆದರೆ (!FDOrErr)
    ಹಿಂತಿರುಗಿ error_ಗಮ್ಯ_Error_ಸಂಕೇತ(FDOrErr.takeError());
  sys::fs::file_t FD = *FDOrErr;
  ಸ್ವಯಂಚಾಲಿತ Ret = getOpenFileImpl<MB>(FD, Filename, /*FileSize=*/-1, Map_ಗಾತ್ರ, ಸ್ಥಾನಾಂತರ,
                                 RequiresNullTerminator, IsVolatile, Alignment);
  sys::fs::closeFile(FD);
  ಹಿಂತಿರುಗಿ Ret;
}

ErrorOr<std::unique_ptr<Writable_ಸ್ಮರಣೆ_ಬಫರ್>>
Writable_ಸ್ಮರಣೆ_ಬಫರ್::getFile(ಸ್ಥಿರಾಂಕ Twine &Filename, ತಾರ್ಕಿಕ IsVolatile,
                              std::optional<Align> Alignment) {
  sys::sandbox::violationIfEnabled();

  ಹಿಂತಿರುಗಿ getFileAux<Writable_ಸ್ಮರಣೆ_ಬಫರ್>(
      Filename, /*MapSize=*/-1, /*Offset=*/0, /*IsText=*/ಸುಳ್ಳು,
      /*RequiresNullTerminator=*/ಸುಳ್ಳು, IsVolatile, Alignment);
}

ErrorOr<std::unique_ptr<Writable_ಸ್ಮರಣೆ_ಬಫರ್>>
Writable_ಸ್ಮರಣೆ_ಬಫರ್::get_File_ತುಂಡು(ಸ್ಥಿರಾಂಕ Twine &Filename, uint64_t Map_ಗಾತ್ರ,
                                   uint64_t ಸ್ಥಾನಾಂತರ, ತಾರ್ಕಿಕ IsVolatile,
                                   std::optional<Align> Alignment) {
  sys::sandbox::violationIfEnabled();

  ಹಿಂತಿರುಗಿ getFileAux<Writable_ಸ್ಮರಣೆ_ಬಫರ್>(
      Filename, Map_ಗಾತ್ರ, ಸ್ಥಾನಾಂತರ, /*IsText=*/ಸುಳ್ಳು,
      /*RequiresNullTerminator=*/ಸುಳ್ಳು, IsVolatile, Alignment);
}

std::unique_ptr<Writable_ಸ್ಮರಣೆ_ಬಫರ್>
Writable_ಸ್ಮರಣೆ_ಬಫರ್::get_New_Uninit_Mem_ಬಫರ್(size_t ಗಾತ್ರ_2,
                                            ಸ್ಥಿರಾಂಕ Twine &ಬಫರ್_Name,
                                            std::optional<Align> Alignment) {
  ಬಳಸಿ Mem_ಬಫರ್ = ಸ್ಮರಣೆ_ಬಫರ್_Mem<Writable_ಸ್ಮರಣೆ_ಬಫರ್>;

  // Use 16-byte alignment if no alignment is specified.
  Align BufAlign = Alignment.ಮೌಲ್ಯ_or(Align(16));

  // Allocate space for the MemoryBuffer, the data and the name. It is important
  // that MemoryBuffer and data are aligned so PointerIntPair works with them.
  SmallString<256> NameBuf;
  StringRef NameRef = ಬಫರ್_Name.ಗಮ್ಯ_String_Ref(NameBuf);

  size_t StringLen = ಗಾತ್ರ(Mem_ಬಫರ್) + ಗಾತ್ರ(size_t) + NameRef.ಗಾತ್ರ_3() + 1;
  size_t RealLen = StringLen + ಗಾತ್ರ_2 + 1 + BufAlign.ಮೌಲ್ಯ_2();
  ಆದರೆ (RealLen <= ಗಾತ್ರ_2) // Check for rollover.
    ಹಿಂತಿರುಗಿ ಶೂನ್ಯಸೂಚಕ;
  // We use a call to malloc() rather than a call to a non-throwing operator
  // new() because LLVM unconditionally installs an out of memory new handler
  // when exceptions are disabled. This new handler intentionally crashes to
  // aid with debugging, but that makes non-throwing new calls unhelpful.
  // See MemoryBufferMem::operator delete() for the paired call to free(), and
  // llvm::install_out_of_memory_new_handler() for the installation of the
  // custom new handler.
  ಅಕ್ಷರ *Mem = static_cast<ಅಕ್ಷರ *>(std::ಸ್ಮರಣೆ_ಕೊಡು(RealLen));
  ಆದರೆ (!Mem)
    ಹಿಂತಿರುಗಿ ಶೂನ್ಯಸೂಚಕ;

  // The name is stored after the class itself.
  *reinterpret_cast<size_t *>(Mem + ಗಾತ್ರ(Mem_ಬಫರ್)) = NameRef.ಗಾತ್ರ_3();
  CopyStringRef(Mem + ಗಾತ್ರ(Mem_ಬಫರ್) + ಗಾತ್ರ(size_t), NameRef);

  // The buffer begins after the name and must be aligned.
  ಅಕ್ಷರ *Buf = (ಅಕ್ಷರ *)alignAddr(Mem + StringLen, BufAlign);
  Buf[ಗಾತ್ರ_2] = 0; // Null terminate buffer.

  ಸ್ವಯಂಚಾಲಿತ *Ret = ಹೊಸ (Mem) Mem_ಬಫರ್(StringRef(Buf, ಗಾತ್ರ_2), ನಿಜ);
  ಹಿಂತಿರುಗಿ std::unique_ptr<Writable_ಸ್ಮರಣೆ_ಬಫರ್>(Ret);
}

std::unique_ptr<Writable_ಸ್ಮರಣೆ_ಬಫರ್>
Writable_ಸ್ಮರಣೆ_ಬಫರ್::get_New_Mem_ಬಫರ್(size_t ಗಾತ್ರ_2, ಸ್ಥಿರಾಂಕ Twine &ಬಫರ್_Name) {
  ಸ್ವಯಂಚಾಲಿತ SB = Writable_ಸ್ಮರಣೆ_ಬಫರ್::get_New_Uninit_Mem_ಬಫರ್(ಗಾತ್ರ_2, ಬಫರ್_Name);
  ಆದರೆ (!SB)
    ಹಿಂತಿರುಗಿ ಶೂನ್ಯಸೂಚಕ;
  memset(SB->get_ಬಫರ್_ಆರಂಭ(), 0, ಗಾತ್ರ_2);
  ಹಿಂತಿರುಗಿ SB;
}

ಸ್ಥಿರ ತಾರ್ಕಿಕ shouldUseMmap(sys::fs::file_t FD,
                          size_t File_ಗಾತ್ರ,
                          size_t Map_ಗಾತ್ರ,
                          off_t ಸ್ಥಾನಾಂತರ,
                          ತಾರ್ಕಿಕ RequiresNullTerminator,
                          ಪೂರ್ಣಾಂಕ ಪುಟ_ಗಾತ್ರ,
                          ತಾರ್ಕಿಕ IsVolatile) {
#ಆದರೆ defined(__MVS__)
  // zOS Enhanced ASCII auto convert does not support mmap.
  ಹಿಂತಿರುಗಿ ಸುಳ್ಳು;
#ಆದರೆ_ಅಂತ್ಯ

  // mmap may leave the buffer without null terminator if the file size changed
  // by the time the last page is mapped in, so avoid it if the file size is
  // likely to change.
  ಆದರೆ (IsVolatile && RequiresNullTerminator)
    ಹಿಂತಿರುಗಿ ಸುಳ್ಳು;

  // We don't use mmap for small files because this can severely fragment our
  // address space.
  ಆದರೆ (Map_ಗಾತ್ರ < 4 * 4096 || Map_ಗಾತ್ರ < (ಚಿಹ್ನೆರಹಿತ)ಪುಟ_ಗಾತ್ರ)
    ಹಿಂತಿರುಗಿ ಸುಳ್ಳು;

  ಆದರೆ (!RequiresNullTerminator)
    ಹಿಂತಿರುಗಿ ನಿಜ;

  // If we don't know the file size, use fstat to find out.  fstat on an open
  // file descriptor is cheaper than stat on a random path.
  // FIXME: this chunk of code is duplicated, but it avoids a fstat when
  // RequiresNullTerminator = false and MapSize != -1.
  ಆದರೆ (File_ಗಾತ್ರ == size_t(-1)) {
    sys::fs::file_ಸ್ಥಿತಿ ಸ್ಥಿತಿ_2;
    ಆದರೆ (sys::fs::ಸ್ಥಿತಿ_3(FD, ಸ್ಥಿತಿ_2))
      ಹಿಂತಿರುಗಿ ಸುಳ್ಳು;
    File_ಗಾತ್ರ = ಸ್ಥಿತಿ_2.get_ಗಾತ್ರ();
  }

  // If we need a null terminator and the end of the map is inside the file,
  // we cannot use mmap.
  size_t ಕೊನೆ_2 = ಸ್ಥಾನಾಂತರ + Map_ಗಾತ್ರ;
  ಪ್ರತಿಜ್ಞಾ(ಕೊನೆ_2 <= File_ಗಾತ್ರ);
  ಆದರೆ (ಕೊನೆ_2 != File_ಗಾತ್ರ)
    ಹಿಂತಿರುಗಿ ಸುಳ್ಳು;

  // Don't try to map files that are exactly a multiple of the system page size
  // if we need a null terminator.
  ಆದರೆ ((File_ಗಾತ್ರ & (ಪುಟ_ಗಾತ್ರ -1)) == 0)
    ಹಿಂತಿರುಗಿ ಸುಳ್ಳು;

#ಆದರೆ defined(__CYGWIN__)
  // Don't try to map files that are exactly a multiple of the physical page size
  // if we need a null terminator.
  // FIXME: We should reorganize again getPageSize() on Win32.
  ಆದರೆ ((File_ಗಾತ್ರ & (4096 - 1)) == 0)
    ಹಿಂತಿರುಗಿ ಸುಳ್ಳು;
#ಆದರೆ_ಅಂತ್ಯ

  ಹಿಂತಿರುಗಿ ನಿಜ;
}

ಸ್ಥಿರ ErrorOr<std::unique_ptr<Write_Through_ಸ್ಮರಣೆ_ಬಫರ್>>
getReadWriteFile(ಸ್ಥಿರಾಂಕ Twine &Filename, uint64_t File_ಗಾತ್ರ, uint64_t Map_ಗಾತ್ರ,
                 uint64_t ಸ್ಥಾನಾಂತರ) {
  Expected<sys::fs::file_t> FDOrErr = sys::fs::openNativeFileForReadWrite(
      Filename, sys::fs::CD_OpenExisting, sys::fs::ದ_None);
  ಆದರೆ (!FDOrErr)
    ಹಿಂತಿರುಗಿ error_ಗಮ್ಯ_Error_ಸಂಕೇತ(FDOrErr.takeError());
  sys::fs::file_t FD = *FDOrErr;

  // Default is to map the full file.
  ಆದರೆ (Map_ಗಾತ್ರ == uint64_t(-1)) {
    // If we don't know the file size, use fstat to find out.  fstat on an open
    // file descriptor is cheaper than stat on a random path.
    ಆದರೆ (File_ಗಾತ್ರ == uint64_t(-1)) {
      sys::fs::file_ಸ್ಥಿತಿ ಸ್ಥಿತಿ_2;
      std::error_ಸಂಕೇತ EC = sys::fs::ಸ್ಥಿತಿ_3(FD, ಸ್ಥಿತಿ_2);
      ಆದರೆ (EC)
        ಹಿಂತಿರುಗಿ EC;

      // If this not a file or a block device (e.g. it's a named pipe
      // or character device), we can't mmap it, so error out.
      sys::fs::file_type Type = ಸ್ಥಿತಿ_2.type();
      ಆದರೆ (Type != sys::fs::file_type::regular_file &&
          Type != sys::fs::file_type::ಖಂಡ_file)
        ಹಿಂತಿರುಗಿ make_error_ಸಂಕೇತ(errc::invalid_ವಾದ);

      File_ಗಾತ್ರ = ಸ್ಥಿತಿ_2.get_ಗಾತ್ರ();
    }
    Map_ಗಾತ್ರ = File_ಗಾತ್ರ;
  }

  std::error_ಸಂಕೇತ EC;
  std::unique_ptr<Write_Through_ಸ್ಮರಣೆ_ಬಫರ್> ಫಲಿತಾಂಶ(
      ಹೊಸ (Named_ಬಫರ್_Alloc(Filename))
          ಸ್ಮರಣೆ_ಬಫರ್_MMap_File<Write_Through_ಸ್ಮರಣೆ_ಬಫರ್>(ಸುಳ್ಳು, FD, Map_ಗಾತ್ರ,
                                                         ಸ್ಥಾನಾಂತರ, EC));
  ಆದರೆ (EC)
    ಹಿಂತಿರುಗಿ EC;
  ಹಿಂತಿರುಗಿ std::move(ಫಲಿತಾಂಶ);
}

ErrorOr<std::unique_ptr<Write_Through_ಸ್ಮರಣೆ_ಬಫರ್>>
Write_Through_ಸ್ಮರಣೆ_ಬಫರ್::getFile(ಸ್ಥಿರಾಂಕ Twine &Filename, int64_t File_ಗಾತ್ರ) {
  sys::sandbox::violationIfEnabled();

  ಹಿಂತಿರುಗಿ getReadWriteFile(Filename, File_ಗಾತ್ರ, File_ಗಾತ್ರ, 0);
}

/// Map a subrange of the specified file as a WritableMemoryBuffer.
ErrorOr<std::unique_ptr<Write_Through_ಸ್ಮರಣೆ_ಬಫರ್>>
Write_Through_ಸ್ಮರಣೆ_ಬಫರ್::get_File_ತುಂಡು(ಸ್ಥಿರಾಂಕ Twine &Filename, uint64_t Map_ಗಾತ್ರ,
                                       uint64_t ಸ್ಥಾನಾಂತರ) {
  sys::sandbox::violationIfEnabled();

  ಹಿಂತಿರುಗಿ getReadWriteFile(Filename, -1, Map_ಗಾತ್ರ, ಸ್ಥಾನಾಂತರ);
}

ಮಾದರಿ <ಬಗೆ_ಹೆಸರು MB>
ಸ್ಥಿರ ErrorOr<std::unique_ptr<MB>>
getOpenFileImpl(sys::fs::file_t FD, ಸ್ಥಿರಾಂಕ Twine &Filename, uint64_t File_ಗಾತ್ರ,
                uint64_t Map_ಗಾತ್ರ, int64_t ಸ್ಥಾನಾಂತರ, ತಾರ್ಕಿಕ RequiresNullTerminator,
                ತಾರ್ಕಿಕ IsVolatile, std::optional<Align> Alignment) {
  ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ ಪುಟ_ಗಾತ್ರ = sys::Process::get_ಪುಟ_ಗಾತ್ರ_Estimate();

  // Default is to map the full file.
  ಆದರೆ (Map_ಗಾತ್ರ == uint64_t(-1)) {
    // If we don't know the file size, use fstat to find out.  fstat on an open
    // file descriptor is cheaper than stat on a random path.
    ಆದರೆ (File_ಗಾತ್ರ == uint64_t(-1)) {
      sys::fs::file_ಸ್ಥಿತಿ ಸ್ಥಿತಿ_2;
      std::error_ಸಂಕೇತ EC = sys::fs::ಸ್ಥಿತಿ_3(FD, ಸ್ಥಿತಿ_2);
      ಆದರೆ (EC)
        ಹಿಂತಿರುಗಿ EC;

      // If this not a file or a block device (e.g. it's a named pipe
      // or character device), we can't trust the size. Create the memory
      // buffer by copying off the stream.
      sys::fs::file_type Type = ಸ್ಥಿತಿ_2.type();
      ಆದರೆ (Type != sys::fs::file_type::regular_file &&
          Type != sys::fs::file_type::ಖಂಡ_file)
        ಹಿಂತಿರುಗಿ get_ಸ್ಮರಣೆ_ಬಫರ್_For_ಹರಿವು(FD, Filename);

      File_ಗಾತ್ರ = ಸ್ಥಿತಿ_2.get_ಗಾತ್ರ();
    }
    Map_ಗಾತ್ರ = File_ಗಾತ್ರ;
  }

  ಆದರೆ (shouldUseMmap(FD, File_ಗಾತ್ರ, Map_ಗಾತ್ರ, ಸ್ಥಾನಾಂತರ, RequiresNullTerminator,
                    ಪುಟ_ಗಾತ್ರ, IsVolatile)) {
    std::error_ಸಂಕೇತ EC;
    std::unique_ptr<MB> ಫಲಿತಾಂಶ(
        ಹೊಸ (Named_ಬಫರ್_Alloc(Filename)) ಸ್ಮರಣೆ_ಬಫರ್_MMap_File<MB>(
            RequiresNullTerminator, FD, Map_ಗಾತ್ರ, ಸ್ಥಾನಾಂತರ, EC));
    ಆದರೆ (!EC) {
      // On at least Linux, and possibly on other systems, mmap may return pages
      // from the page cache that are not properly filled with trailing zeroes,
      // if some prior user of the page wrote non-zero bytes. Detect this and
      // don't use mmap in that case.
      ಆದರೆ (!RequiresNullTerminator || *ಫಲಿತಾಂಶ->get_ಬಫರ್_ಕೊನೆ() == '\0')
        ಹಿಂತಿರುಗಿ std::move(ಫಲಿತಾಂಶ);
    }
  }

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ __MVS__
  ErrorOr<ತಾರ್ಕಿಕ> NeedsConversion = needConversion(Filename, FD);
  ಆದರೆ (std::error_ಸಂಕೇತ EC = NeedsConversion.getError())
    ಹಿಂತಿರುಗಿ EC;
  // File size may increase due to EBCDIC -> UTF-8 conversion, therefore we
  // cannot trust the file size and we create the memory buffer by copying
  // off the stream.
  // Note: This only works with the assumption of reading a full file (i.e,
  // Offset == 0 and MapSize == FileSize). Reading a file slice does not work.
  ಆದರೆ (*NeedsConversion && ಸ್ಥಾನಾಂತರ == 0 && Map_ಗಾತ್ರ == File_ಗಾತ್ರ)
    ಹಿಂತಿರುಗಿ get_ಸ್ಮರಣೆ_ಬಫರ್_For_ಹರಿವು(FD, Filename);
#ಆದರೆ_ಅಂತ್ಯ

  ಸ್ವಯಂಚಾಲಿತ Buf =
      Writable_ಸ್ಮರಣೆ_ಬಫರ್::get_New_Uninit_Mem_ಬಫರ್(Map_ಗಾತ್ರ, Filename, Alignment);
  ಆದರೆ (!Buf) {
    // Failed to create a buffer. The only way it can fail is if
    // new(std::nothrow) returns 0.
    ಹಿಂತಿರುಗಿ make_error_ಸಂಕೇತ(errc::not_enough_ಸ್ಮರಣೆ);
  }

  // Read until EOF, zero-initialize the rest.
  MutableArrayRef<ಅಕ್ಷರ> ಗಮ್ಯ_Read = Buf->get_ಬಫರ್();
  ಆಗ (!ಗಮ್ಯ_Read.ಖಾಲಿಗಳು()) {
    Expected<size_t> Read_ಬೈಟುಗಳು =
        sys::fs::read_Native_File_ತುಂಡು(FD, ಗಮ್ಯ_Read, ಸ್ಥಾನಾಂತರ);
    ಆದರೆ (!Read_ಬೈಟುಗಳು)
      ಹಿಂತಿರುಗಿ error_ಗಮ್ಯ_Error_ಸಂಕೇತ(Read_ಬೈಟುಗಳು.takeError());
    ಆದರೆ (*Read_ಬೈಟುಗಳು == 0) {
      std::memset(ಗಮ್ಯ_Read.ದತ್ತಾಂಶ(), 0, ಗಮ್ಯ_Read.ಗಾತ್ರ_3());
      ನಿಲ್ಲಿಸು;
    }
    ಗಮ್ಯ_Read = ಗಮ್ಯ_Read.drop_front(*Read_ಬೈಟುಗಳು);
    ಸ್ಥಾನಾಂತರ += *Read_ಬೈಟುಗಳು;
  }

  ಹಿಂತಿರುಗಿ std::move(Buf);
}

ErrorOr<std::unique_ptr<ಸ್ಮರಣೆ_ಬಫರ್>>
ಸ್ಮರಣೆ_ಬಫರ್::getOpenFile(sys::fs::file_t FD, ಸ್ಥಿರಾಂಕ Twine &Filename,
                          uint64_t File_ಗಾತ್ರ, ತಾರ್ಕಿಕ RequiresNullTerminator,
                          ತಾರ್ಕಿಕ IsVolatile, std::optional<Align> Alignment) {
  sys::sandbox::violationIfEnabled();

  ಹಿಂತಿರುಗಿ getOpenFileImpl<ಸ್ಮರಣೆ_ಬಫರ್>(FD, Filename, File_ಗಾತ್ರ, File_ಗಾತ್ರ, 0,
                                       RequiresNullTerminator, IsVolatile,
                                       Alignment);
}

ErrorOr<std::unique_ptr<ಸ್ಮರಣೆ_ಬಫರ್>> ಸ್ಮರಣೆ_ಬಫರ್::get_Open_File_ತುಂಡು(
    sys::fs::file_t FD, ಸ್ಥಿರಾಂಕ Twine &Filename, uint64_t Map_ಗಾತ್ರ, int64_t ಸ್ಥಾನಾಂತರ,
    ತಾರ್ಕಿಕ IsVolatile, std::optional<Align> Alignment) {
  ಪ್ರತಿಜ್ಞಾ(Map_ಗಾತ್ರ != uint64_t(-1));

  sys::sandbox::violationIfEnabled();

  ಹಿಂತಿರುಗಿ getOpenFileImpl<ಸ್ಮರಣೆ_ಬಫರ್>(FD, Filename, -1, Map_ಗಾತ್ರ, ಸ್ಥಾನಾಂತರ, ಸುಳ್ಳು,
                                       IsVolatile, Alignment);
}

ErrorOr<std::unique_ptr<ಸ್ಮರಣೆ_ಬಫರ್>> ಸ್ಮರಣೆ_ಬಫರ್::getSTDIN() {
  sys::sandbox::violationIfEnabled();

  // Read in all of the data from stdin, we cannot mmap stdin.
  //
  // FIXME: That isn't necessarily true, we should try to mmap stdin and
  // fallback if it fails.
  sys::Change_Stdin_ಬಹುಲಕ(sys::fs::ದ_ಪಠ್ಯ);

  ಹಿಂತಿರುಗಿ get_ಸ್ಮರಣೆ_ಬಫರ್_For_ಹರಿವು(sys::fs::getStdinHandle(), "<stdin>");
}

ErrorOr<std::unique_ptr<ಸ್ಮರಣೆ_ಬಫರ್>>
ಸ್ಮರಣೆ_ಬಫರ್::get_File_As_ಹರಿವು(ಸ್ಥಿರಾಂಕ Twine &Filename) {
  sys::sandbox::violationIfEnabled();

  Expected<sys::fs::file_t> FDOrErr =
      sys::fs::openNativeFileForRead(Filename, sys::fs::ದ_None);
  ಆದರೆ (!FDOrErr)
    ಹಿಂತಿರುಗಿ error_ಗಮ್ಯ_Error_ಸಂಕೇತ(FDOrErr.takeError());
  sys::fs::file_t FD = *FDOrErr;
  ErrorOr<std::unique_ptr<ಸ್ಮರಣೆ_ಬಫರ್>> Ret =
      get_ಸ್ಮರಣೆ_ಬಫರ್_For_ಹರಿವು(FD, Filename);
  sys::fs::closeFile(FD);
  ಹಿಂತಿರುಗಿ Ret;
}

ಸ್ಮರಣೆ_ಬಫರ್_Ref ಸ್ಮರಣೆ_ಬಫರ್::get_Mem_ಬಫರ್_Ref() ಸ್ಥಿರಾಂಕ {
  StringRef ದತ್ತಾಂಶ_2 = get_ಬಫರ್();
  StringRef Identifier = get_ಬಫರ್_Identifier();
  ಹಿಂತಿರುಗಿ ಸ್ಮರಣೆ_ಬಫರ್_Ref(ದತ್ತಾಂಶ_2, Identifier);
}

Small_ಸದಿಶ_ಸ್ಮರಣೆ_ಬಫರ್::~Small_ಸದಿಶ_ಸ್ಮರಣೆ_ಬಫರ್() = ಪೂರ್ವನಿಯೋಜಿತ;
