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

#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/MemoryBuffer.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/ADT/STLExtras.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/ADT/SmallString.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Config/config.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/Alignment.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/AutoConvert.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/Errc.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/Error.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/ErrorHandling.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/FileSystem.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/IOSandbox.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/Process.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/Program.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/SmallVectorMemoryBuffer.h"
#ਅਨ੍ਤਰ੍ਭਾਵ <algorithm>
#ਅਨ੍ਤਰ੍ਭਾਵ <cassert>
#ਅਨ੍ਤਰ੍ਭਾਵ <cstring>
#ਅਨ੍ਤਰ੍ਭਾਵ <new>
#ਅਨ੍ਤਰ੍ਭਾਵ <sys/types.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <system_error>
#ਜੇ !defined(_MSC_VER) && !defined(__MINGW32__)
#ਅਨ੍ਤਰ੍ਭਾਵ <unistd.h>
#ਨਹੀਂਤਾਂ
#ਅਨ੍ਤਰ੍ਭਾਵ <io.h>
#ਜੇ_ਅੰਤ

ਵਰਤੋਂ ਨਾਮਾਕਾਸ਼ llvm;

//===----------------------------------------------------------------------===//
// MemoryBuffer implementation itself.
//===----------------------------------------------------------------------===//

ਸਮ੍ਰਿਤੀ_ਬਫਰ::~ਸਮ੍ਰਿਤੀ_ਬਫਰ() = ਮੂਲ;

/// init - Initialize this MemoryBuffer as a reference to externally allocated
/// memory, memory that we know is already null terminated.
ਖਾਲੀ ਸਮ੍ਰਿਤੀ_ਬਫਰ::init(ਅਚਲ ਅੱਖਰ *Buf_ਸ਼ੁਰੂ, ਅਚਲ ਅੱਖਰ *Buf_ਸਿਰਾ,
                        ਤਾਰਕਿਕ RequiresNullTerminator) {
  ਪ੍ਰਤਿਜ੍ਞਾ((!RequiresNullTerminator || Buf_ਸਿਰਾ[0] == 0) &&
         "Buffer is not null terminated!");
  ਬਫਰ_ਸ਼ੁਰੂ = Buf_ਸ਼ੁਰੂ;
  ਬਫਰ_ਸਿਰਾ = Buf_ਸਿਰਾ;
}

//===----------------------------------------------------------------------===//
// MemoryBufferMem implementation.
//===----------------------------------------------------------------------===//

/// CopyStringRef - Copies contents of a StringRef into a block of memory and
/// null-terminates it.
ਸਥਿਰ ਖਾਲੀ CopyStringRef(ਅੱਖਰ *ਸਮ੍ਰਿਤੀ_2, StringRef ਅੰਕੜੇ_2) {
  ਜੇ (!ਅੰਕੜੇ_2.ਖਾਲੀ_ਥਾਵਾਂ())
    ਸਮ੍ਰਿਤੀ_ਨਕਲ(ਸਮ੍ਰਿਤੀ_2, ਅੰਕੜੇ_2.ਅੰਕੜੇ(), ਅੰਕੜੇ_2.ਮਾਪ_2());
  ਸਮ੍ਰਿਤੀ_2[ਅੰਕੜੇ_2.ਮਾਪ_2()] = 0; // Null terminate string.
}

ਨਾਮਾਕਾਸ਼ {
ਬਣਤਰ Named_ਬਫਰ_Alloc {
  ਅਚਲ Twine &Name;
  Named_ਬਫਰ_Alloc(ਅਚਲ Twine &Name) : Name(Name) {}
};
} // namespace

ਖਾਲੀ *ਸੰਚਾਲਕ ਨਵਾਂ(size_t N, ਅਚਲ Named_ਬਫਰ_Alloc &Alloc) {
  SmallString<256> NameBuf;
  StringRef NameRef = Alloc.Name.ਮੰਜ਼ਿਲ_String_Ref(NameBuf);

  // We use malloc() and manually handle it returning null instead of calling
  // operator new because we need all uses of NamedBufferAlloc to be
  // deallocated with a call to free() due to needing to use malloc() in
  // WritableMemoryBuffer::getNewUninitMemBuffer() to work around the out-of-
  // memory handler installed by default in LLVM. See operator delete() member
  // functions within this file for the paired call to free().
  ਅੱਖਰ *Mem =
      static_cast<ਅੱਖਰ *>(std::ਸਮ੍ਰਿਤੀ_ਦਿਓ(N + ਆਕਾਰ(size_t) + NameRef.ਮਾਪ_2() + 1));
  ਜੇ (!Mem)
    llvm::report_bad_alloc_error("Allocation failed");
  *reinterpret_cast<size_t *>(Mem + N) = NameRef.ਮਾਪ_2();
  CopyStringRef(Mem + N + ਆਕਾਰ(size_t), NameRef);
  ਵਾਪਸ Mem;
}

ਨਾਮਾਕਾਸ਼ {
/// MemoryBufferMem - Named MemoryBuffer pointing to a block of memory.
ਸਾਂਚਾ<ਕਿਸਮ_ਨਾਂ MB>
ਵਰਗ ਸਮ੍ਰਿਤੀ_ਬਫਰ_Mem : ਜਨਤਕ MB {
ਜਨਤਕ:
  ਸਮ੍ਰਿਤੀ_ਬਫਰ_Mem(StringRef ਨਿਵੇਸ਼_ਅੰਕੜੇ, ਤਾਰਕਿਕ RequiresNullTerminator) {
    ਸਮ੍ਰਿਤੀ_ਬਫਰ::init(ਨਿਵੇਸ਼_ਅੰਕੜੇ.begin(), ਨਿਵੇਸ਼_ਅੰਕੜੇ.ਸਿਰਾ(),
                       RequiresNullTerminator);
  }

  /// Disable sized deallocation for MemoryBufferMem, because it has
  /// tail-allocated data.
  ਖਾਲੀ ਸੰਚਾਲਕ ਮਿਟਾਓ(ਖਾਲੀ *p) { std::ਮੁਕਤ(p); }

  StringRef get_ਬਫਰ_Identifier() ਅਚਲ override {
    // The name is stored after the class itself.
    ਵਾਪਸ StringRef(reinterpret_cast<ਅਚਲ ਅੱਖਰ *>(ਇਹ + 1) + ਆਕਾਰ(size_t),
                     *reinterpret_cast<ਅਚਲ size_t *>(ਇਹ + 1));
  }

  ਸਮ੍ਰਿਤੀ_ਬਫਰ::ਬਫਰ_ਕਿਸਮ get_ਬਫਰ_ਕਿਸਮ() ਅਚਲ override {
    ਵਾਪਸ ਸਮ੍ਰਿਤੀ_ਬਫਰ::ਸਮ੍ਰਿਤੀ_ਬਫਰ_Malloc;
  }
};
} // namespace

ਸਾਂਚਾ <ਕਿਸਮ_ਨਾਂ MB>
ਸਥਿਰ ErrorOr<std::unique_ptr<MB>>
getFileAux(ਅਚਲ Twine &Filename, uint64_t Map_ਮਾਪ, uint64_t ਵਿੱਥ,
           ਤਾਰਕਿਕ Is_ਪਾਠ, ਤਾਰਕਿਕ RequiresNullTerminator, ਤਾਰਕਿਕ IsVolatile,
           std::optional<Align> Alignment);

std::unique_ptr<ਸਮ੍ਰਿਤੀ_ਬਫਰ>
ਸਮ੍ਰਿਤੀ_ਬਫਰ::get_Mem_ਬਫਰ(StringRef ਨਿਵੇਸ਼_ਅੰਕੜੇ, StringRef ਬਫਰ_Name,
                           ਤਾਰਕਿਕ RequiresNullTerminator) {
  ਸਵੈਚਾਲਿਤ *Ret = ਨਵਾਂ (Named_ਬਫਰ_Alloc(ਬਫਰ_Name))
      ਸਮ੍ਰਿਤੀ_ਬਫਰ_Mem<ਸਮ੍ਰਿਤੀ_ਬਫਰ>(ਨਿਵੇਸ਼_ਅੰਕੜੇ, RequiresNullTerminator);
  ਵਾਪਸ std::unique_ptr<ਸਮ੍ਰਿਤੀ_ਬਫਰ>(Ret);
}

std::unique_ptr<ਸਮ੍ਰਿਤੀ_ਬਫਰ>
ਸਮ੍ਰਿਤੀ_ਬਫਰ::get_Mem_ਬਫਰ(ਸਮ੍ਰਿਤੀ_ਬਫਰ_Ref Ref, ਤਾਰਕਿਕ RequiresNullTerminator) {
  ਵਾਪਸ std::unique_ptr<ਸਮ੍ਰਿਤੀ_ਬਫਰ>(get_Mem_ਬਫਰ(
      Ref.get_ਬਫਰ(), Ref.get_ਬਫਰ_Identifier(), RequiresNullTerminator));
}

ਸਥਿਰ ErrorOr<std::unique_ptr<Writable_ਸਮ੍ਰਿਤੀ_ਬਫਰ>>
get_Mem_ਬਫਰ_Copy_Impl(StringRef ਨਿਵੇਸ਼_ਅੰਕੜੇ, ਅਚਲ Twine &ਬਫਰ_Name) {
  ਸਵੈਚਾਲਿਤ Buf =
      Writable_ਸਮ੍ਰਿਤੀ_ਬਫਰ::get_New_Uninit_Mem_ਬਫਰ(ਨਿਵੇਸ਼_ਅੰਕੜੇ.ਮਾਪ_2(), ਬਫਰ_Name);
  ਜੇ (!Buf)
    ਵਾਪਸ make_error_ਕੋਡ(errc::not_enough_ਸਮ੍ਰਿਤੀ);
  // Calling memcpy with null src/dst is UB, and an empty StringRef is
  // represented with {nullptr, 0}.
  llvm::copy(ਨਿਵੇਸ਼_ਅੰਕੜੇ, Buf->get_ਬਫਰ_ਸ਼ੁਰੂ());
  ਵਾਪਸ std::move(Buf);
}

std::unique_ptr<ਸਮ੍ਰਿਤੀ_ਬਫਰ>
ਸਮ੍ਰਿਤੀ_ਬਫਰ::get_Mem_ਬਫਰ_Copy(StringRef ਨਿਵੇਸ਼_ਅੰਕੜੇ, ਅਚਲ Twine &ਬਫਰ_Name) {
  ਸਵੈਚਾਲਿਤ Buf = get_Mem_ਬਫਰ_Copy_Impl(ਨਿਵੇਸ਼_ਅੰਕੜੇ, ਬਫਰ_Name);
  ਜੇ (Buf)
    ਵਾਪਸ std::move(*Buf);
  ਵਾਪਸ ਖਾਲੀ_ਸੂਚਕ;
}

ErrorOr<std::unique_ptr<ਸਮ੍ਰਿਤੀ_ਬਫਰ>>
ਸਮ੍ਰਿਤੀ_ਬਫਰ::getFileOrSTDIN(ਅਚਲ Twine &Filename, ਤਾਰਕਿਕ Is_ਪਾਠ,
                             ਤਾਰਕਿਕ RequiresNullTerminator,
                             std::optional<Align> Alignment) {
  sys::sandbox::violationIfEnabled();

  SmallString<256> NameBuf;
  StringRef NameRef = Filename.ਮੰਜ਼ਿਲ_String_Ref(NameBuf);

  ਜੇ (NameRef == "-")
    ਵਾਪਸ getSTDIN();
  ਵਾਪਸ getFile(Filename, Is_ਪਾਠ, RequiresNullTerminator,
                 /*IsVolatile=*/ਝੂਠ, Alignment);
}

ErrorOr<std::unique_ptr<ਸਮ੍ਰਿਤੀ_ਬਫਰ>>
ਸਮ੍ਰਿਤੀ_ਬਫਰ::get_File_ਟੁਕੜਾ(ਅਚਲ Twine &File_ਰਸਤਾ, uint64_t Map_ਮਾਪ,
                           uint64_t ਵਿੱਥ, ਤਾਰਕਿਕ IsVolatile,
                           std::optional<Align> Alignment) {
  sys::sandbox::violationIfEnabled();

  ਵਾਪਸ getFileAux<ਸਮ੍ਰਿਤੀ_ਬਫਰ>(File_ਰਸਤਾ, Map_ਮਾਪ, ਵਿੱਥ, /*IsText=*/ਝੂਠ,
                                  /*RequiresNullTerminator=*/ਝੂਠ, IsVolatile,
                                  Alignment);
}

//===----------------------------------------------------------------------===//
// MemoryBuffer::getFile implementation.
//===----------------------------------------------------------------------===//

ਨਾਮਾਕਾਸ਼ {

ਸਾਂਚਾ <ਕਿਸਮ_ਨਾਂ MB>
constexpr sys::fs::mapped_file_region::mapmode Mapmode =
    sys::fs::mapped_file_region::readonly;
ਸਾਂਚਾ <>
constexpr sys::fs::mapped_file_region::mapmode Mapmode<ਸਮ੍ਰਿਤੀ_ਬਫਰ> =
    sys::fs::mapped_file_region::readonly;
ਸਾਂਚਾ <>
constexpr sys::fs::mapped_file_region::mapmode Mapmode<Writable_ਸਮ੍ਰਿਤੀ_ਬਫਰ> =
    sys::fs::mapped_file_region::priv;
ਸਾਂਚਾ <>
constexpr sys::fs::mapped_file_region::mapmode
    Mapmode<Write_Through_ਸਮ੍ਰਿਤੀ_ਬਫਰ> = sys::fs::mapped_file_region::readwrite;

/// Memory maps a file descriptor using sys::fs::mapped_file_region.
///
/// This handles converting the offset into a legal offset on the platform.
ਸਾਂਚਾ<ਕਿਸਮ_ਨਾਂ MB>
ਵਰਗ ਸਮ੍ਰਿਤੀ_ਬਫਰ_MMap_File : ਜਨਤਕ MB {
  sys::fs::mapped_file_region MFR;

  ਸਥਿਰ uint64_t get_Legal_Map_ਵਿੱਥ(uint64_t ਵਿੱਥ) {
    ਵਾਪਸ ਵਿੱਥ & ~(sys::fs::mapped_file_region::alignment() - 1);
  }

  ਸਥਿਰ uint64_t get_Legal_Map_ਮਾਪ(uint64_t Len, uint64_t ਵਿੱਥ) {
    ਵਾਪਸ Len + (ਵਿੱਥ - get_Legal_Map_ਵਿੱਥ(ਵਿੱਥ));
  }

  ਅਚਲ ਅੱਖਰ *get_ਸ਼ੁਰੂ(uint64_t Len, uint64_t ਵਿੱਥ) {
    ਵਾਪਸ MFR.const_ਅੰਕੜੇ() + (ਵਿੱਥ - get_Legal_Map_ਵਿੱਥ(ਵਿੱਥ));
  }

ਜਨਤਕ:
  ਸਮ੍ਰਿਤੀ_ਬਫਰ_MMap_File(ਤਾਰਕਿਕ RequiresNullTerminator, sys::fs::file_t FD, uint64_t Len,
                       uint64_t ਵਿੱਥ, std::error_ਕੋਡ &EC)
      : MFR(FD, Mapmode<MB>, get_Legal_Map_ਮਾਪ(Len, ਵਿੱਥ),
            get_Legal_Map_ਵਿੱਥ(ਵਿੱਥ), EC) {
    ਜੇ (!EC) {
      ਅਚਲ ਅੱਖਰ *ਸ਼ੁਰੂ = get_ਸ਼ੁਰੂ(Len, ਵਿੱਥ);
      ਸਮ੍ਰਿਤੀ_ਬਫਰ::init(ਸ਼ੁਰੂ, ਸ਼ੁਰੂ + Len, RequiresNullTerminator);
    }
  }

  /// Disable sized deallocation for MemoryBufferMMapFile, because it has
  /// tail-allocated data.
  ਖਾਲੀ ਸੰਚਾਲਕ ਮਿਟਾਓ(ਖਾਲੀ *p) { std::ਮੁਕਤ(p); }

  StringRef get_ਬਫਰ_Identifier() ਅਚਲ override {
    // The name is stored after the class itself.
    ਵਾਪਸ StringRef(reinterpret_cast<ਅਚਲ ਅੱਖਰ *>(ਇਹ + 1) + ਆਕਾਰ(size_t),
                     *reinterpret_cast<ਅਚਲ size_t *>(ਇਹ + 1));
  }

  ਸਮ੍ਰਿਤੀ_ਬਫਰ::ਬਫਰ_ਕਿਸਮ get_ਬਫਰ_ਕਿਸਮ() ਅਚਲ override {
    ਵਾਪਸ ਸਮ੍ਰਿਤੀ_ਬਫਰ::ਸਮ੍ਰਿਤੀ_ਬਫਰ_MMap;
  }

  ਖਾਲੀ dontNeedIfMmap() override { MFR.dontNeed(); }
  ਖਾਲੀ willNeedIfMmap() override { MFR.willNeed(); }
  ਖਾਲੀ randomAccessIfMmap() override { MFR.randomAccess(); }
};
} // namespace

ਸਥਿਰ ErrorOr<std::unique_ptr<Writable_ਸਮ੍ਰਿਤੀ_ਬਫਰ>>
get_ਸਮ੍ਰਿਤੀ_ਬਫਰ_For_ਧਾਰਾ(sys::fs::file_t FD, ਅਚਲ Twine &ਬਫਰ_Name) {
  SmallString<sys::fs::Default_Read_Chunk_ਮਾਪ> ਬਫਰ;
  ਜੇ (Error E = sys::fs::read_Native_File_ਮੰਜ਼ਿਲ_EOF(FD, ਬਫਰ))
    ਵਾਪਸ error_ਮੰਜ਼ਿਲ_Error_ਕੋਡ(std::move(E));
  ਵਾਪਸ get_Mem_ਬਫਰ_Copy_Impl(ਬਫਰ, ਬਫਰ_Name);
}

ErrorOr<std::unique_ptr<ਸਮ੍ਰਿਤੀ_ਬਫਰ>>
ਸਮ੍ਰਿਤੀ_ਬਫਰ::getFile(ਅਚਲ Twine &Filename, ਤਾਰਕਿਕ Is_ਪਾਠ,
                      ਤਾਰਕਿਕ RequiresNullTerminator, ਤਾਰਕਿਕ IsVolatile,
                      std::optional<Align> Alignment) {
  sys::sandbox::violationIfEnabled();

  ਵਾਪਸ getFileAux<ਸਮ੍ਰਿਤੀ_ਬਫਰ>(Filename, /*MapSize=*/-1, /*Offset=*/0,
                                  Is_ਪਾਠ, RequiresNullTerminator, IsVolatile,
                                  Alignment);
}

ਸਾਂਚਾ <ਕਿਸਮ_ਨਾਂ MB>
ਸਥਿਰ ErrorOr<std::unique_ptr<MB>>
getOpenFileImpl(sys::fs::file_t FD, ਅਚਲ Twine &Filename, uint64_t File_ਮਾਪ,
                uint64_t Map_ਮਾਪ, int64_t ਵਿੱਥ, ਤਾਰਕਿਕ RequiresNullTerminator,
                ਤਾਰਕਿਕ IsVolatile, std::optional<Align> Alignment);

ਸਾਂਚਾ <ਕਿਸਮ_ਨਾਂ MB>
ਸਥਿਰ ErrorOr<std::unique_ptr<MB>>
getFileAux(ਅਚਲ Twine &Filename, uint64_t Map_ਮਾਪ, uint64_t ਵਿੱਥ,
           ਤਾਰਕਿਕ Is_ਪਾਠ, ਤਾਰਕਿਕ RequiresNullTerminator, ਤਾਰਕਿਕ IsVolatile,
           std::optional<Align> Alignment) {
  Expected<sys::fs::file_t> FDOrErr = sys::fs::openNativeFileForRead(
      Filename, Is_ਪਾਠ ? sys::fs::ਦਾ_ਪਾਠ_With_CRLF : sys::fs::ਦਾ_None);
  ਜੇ (!FDOrErr)
    ਵਾਪਸ error_ਮੰਜ਼ਿਲ_Error_ਕੋਡ(FDOrErr.takeError());
  sys::fs::file_t FD = *FDOrErr;
  ਸਵੈਚਾਲਿਤ Ret = getOpenFileImpl<MB>(FD, Filename, /*FileSize=*/-1, Map_ਮਾਪ, ਵਿੱਥ,
                                 RequiresNullTerminator, IsVolatile, Alignment);
  sys::fs::closeFile(FD);
  ਵਾਪਸ Ret;
}

ErrorOr<std::unique_ptr<Writable_ਸਮ੍ਰਿਤੀ_ਬਫਰ>>
Writable_ਸਮ੍ਰਿਤੀ_ਬਫਰ::getFile(ਅਚਲ Twine &Filename, ਤਾਰਕਿਕ IsVolatile,
                              std::optional<Align> Alignment) {
  sys::sandbox::violationIfEnabled();

  ਵਾਪਸ getFileAux<Writable_ਸਮ੍ਰਿਤੀ_ਬਫਰ>(
      Filename, /*MapSize=*/-1, /*Offset=*/0, /*IsText=*/ਝੂਠ,
      /*RequiresNullTerminator=*/ਝੂਠ, IsVolatile, Alignment);
}

ErrorOr<std::unique_ptr<Writable_ਸਮ੍ਰਿਤੀ_ਬਫਰ>>
Writable_ਸਮ੍ਰਿਤੀ_ਬਫਰ::get_File_ਟੁਕੜਾ(ਅਚਲ Twine &Filename, uint64_t Map_ਮਾਪ,
                                   uint64_t ਵਿੱਥ, ਤਾਰਕਿਕ IsVolatile,
                                   std::optional<Align> Alignment) {
  sys::sandbox::violationIfEnabled();

  ਵਾਪਸ getFileAux<Writable_ਸਮ੍ਰਿਤੀ_ਬਫਰ>(
      Filename, Map_ਮਾਪ, ਵਿੱਥ, /*IsText=*/ਝੂਠ,
      /*RequiresNullTerminator=*/ਝੂਠ, IsVolatile, Alignment);
}

std::unique_ptr<Writable_ਸਮ੍ਰਿਤੀ_ਬਫਰ>
Writable_ਸਮ੍ਰਿਤੀ_ਬਫਰ::get_New_Uninit_Mem_ਬਫਰ(size_t ਮਾਪ,
                                            ਅਚਲ Twine &ਬਫਰ_Name,
                                            std::optional<Align> Alignment) {
  ਵਰਤੋਂ Mem_ਬਫਰ = ਸਮ੍ਰਿਤੀ_ਬਫਰ_Mem<Writable_ਸਮ੍ਰਿਤੀ_ਬਫਰ>;

  // Use 16-byte alignment if no alignment is specified.
  Align BufAlign = Alignment.ਮੁੱਲ_or(Align(16));

  // Allocate space for the MemoryBuffer, the data and the name. It is important
  // that MemoryBuffer and data are aligned so PointerIntPair works with them.
  SmallString<256> NameBuf;
  StringRef NameRef = ਬਫਰ_Name.ਮੰਜ਼ਿਲ_String_Ref(NameBuf);

  size_t StringLen = ਆਕਾਰ(Mem_ਬਫਰ) + ਆਕਾਰ(size_t) + NameRef.ਮਾਪ_2() + 1;
  size_t RealLen = StringLen + ਮਾਪ + 1 + BufAlign.ਮੁੱਲ_2();
  ਜੇ (RealLen <= ਮਾਪ) // Check for rollover.
    ਵਾਪਸ ਖਾਲੀ_ਸੂਚਕ;
  // We use a call to malloc() rather than a call to a non-throwing operator
  // new() because LLVM unconditionally installs an out of memory new handler
  // when exceptions are disabled. This new handler intentionally crashes to
  // aid with debugging, but that makes non-throwing new calls unhelpful.
  // See MemoryBufferMem::operator delete() for the paired call to free(), and
  // llvm::install_out_of_memory_new_handler() for the installation of the
  // custom new handler.
  ਅੱਖਰ *Mem = static_cast<ਅੱਖਰ *>(std::ਸਮ੍ਰਿਤੀ_ਦਿਓ(RealLen));
  ਜੇ (!Mem)
    ਵਾਪਸ ਖਾਲੀ_ਸੂਚਕ;

  // The name is stored after the class itself.
  *reinterpret_cast<size_t *>(Mem + ਆਕਾਰ(Mem_ਬਫਰ)) = NameRef.ਮਾਪ_2();
  CopyStringRef(Mem + ਆਕਾਰ(Mem_ਬਫਰ) + ਆਕਾਰ(size_t), NameRef);

  // The buffer begins after the name and must be aligned.
  ਅੱਖਰ *Buf = (ਅੱਖਰ *)alignAddr(Mem + StringLen, BufAlign);
  Buf[ਮਾਪ] = 0; // Null terminate buffer.

  ਸਵੈਚਾਲਿਤ *Ret = ਨਵਾਂ (Mem) Mem_ਬਫਰ(StringRef(Buf, ਮਾਪ), ਸੱਚ);
  ਵਾਪਸ std::unique_ptr<Writable_ਸਮ੍ਰਿਤੀ_ਬਫਰ>(Ret);
}

std::unique_ptr<Writable_ਸਮ੍ਰਿਤੀ_ਬਫਰ>
Writable_ਸਮ੍ਰਿਤੀ_ਬਫਰ::get_New_Mem_ਬਫਰ(size_t ਮਾਪ, ਅਚਲ Twine &ਬਫਰ_Name) {
  ਸਵੈਚਾਲਿਤ SB = Writable_ਸਮ੍ਰਿਤੀ_ਬਫਰ::get_New_Uninit_Mem_ਬਫਰ(ਮਾਪ, ਬਫਰ_Name);
  ਜੇ (!SB)
    ਵਾਪਸ ਖਾਲੀ_ਸੂਚਕ;
  memset(SB->get_ਬਫਰ_ਸ਼ੁਰੂ(), 0, ਮਾਪ);
  ਵਾਪਸ SB;
}

ਸਥਿਰ ਤਾਰਕਿਕ shouldUseMmap(sys::fs::file_t FD,
                          size_t File_ਮਾਪ,
                          size_t Map_ਮਾਪ,
                          off_t ਵਿੱਥ,
                          ਤਾਰਕਿਕ RequiresNullTerminator,
                          ਪੂਰਨ ਪੰਨਾ_ਮਾਪ,
                          ਤਾਰਕਿਕ IsVolatile) {
#ਜੇ defined(__MVS__)
  // zOS Enhanced ASCII auto convert does not support mmap.
  ਵਾਪਸ ਝੂਠ;
#ਜੇ_ਅੰਤ

  // mmap may leave the buffer without null terminator if the file size changed
  // by the time the last page is mapped in, so avoid it if the file size is
  // likely to change.
  ਜੇ (IsVolatile && RequiresNullTerminator)
    ਵਾਪਸ ਝੂਠ;

  // We don't use mmap for small files because this can severely fragment our
  // address space.
  ਜੇ (Map_ਮਾਪ < 4 * 4096 || Map_ਮਾਪ < (ਬਿਨਾਨਿਸ਼ਾਨੀ)ਪੰਨਾ_ਮਾਪ)
    ਵਾਪਸ ਝੂਠ;

  ਜੇ (!RequiresNullTerminator)
    ਵਾਪਸ ਸੱਚ;

  // If we don't know the file size, use fstat to find out.  fstat on an open
  // file descriptor is cheaper than stat on a random path.
  // FIXME: this chunk of code is duplicated, but it avoids a fstat when
  // RequiresNullTerminator = false and MapSize != -1.
  ਜੇ (File_ਮਾਪ == size_t(-1)) {
    sys::fs::file_ਸਥਿਤੀ ਸਥਿਤੀ;
    ਜੇ (sys::fs::ਸਥਿਤੀ_2(FD, ਸਥਿਤੀ))
      ਵਾਪਸ ਝੂਠ;
    File_ਮਾਪ = ਸਥਿਤੀ.get_ਮਾਪ();
  }

  // If we need a null terminator and the end of the map is inside the file,
  // we cannot use mmap.
  size_t ਸਿਰਾ_2 = ਵਿੱਥ + Map_ਮਾਪ;
  ਪ੍ਰਤਿਜ੍ਞਾ(ਸਿਰਾ_2 <= File_ਮਾਪ);
  ਜੇ (ਸਿਰਾ_2 != File_ਮਾਪ)
    ਵਾਪਸ ਝੂਠ;

  // Don't try to map files that are exactly a multiple of the system page size
  // if we need a null terminator.
  ਜੇ ((File_ਮਾਪ & (ਪੰਨਾ_ਮਾਪ -1)) == 0)
    ਵਾਪਸ ਝੂਠ;

#ਜੇ defined(__CYGWIN__)
  // Don't try to map files that are exactly a multiple of the physical page size
  // if we need a null terminator.
  // FIXME: We should reorganize again getPageSize() on Win32.
  ਜੇ ((File_ਮਾਪ & (4096 - 1)) == 0)
    ਵਾਪਸ ਝੂਠ;
#ਜੇ_ਅੰਤ

  ਵਾਪਸ ਸੱਚ;
}

ਸਥਿਰ ErrorOr<std::unique_ptr<Write_Through_ਸਮ੍ਰਿਤੀ_ਬਫਰ>>
getReadWriteFile(ਅਚਲ Twine &Filename, uint64_t File_ਮਾਪ, uint64_t Map_ਮਾਪ,
                 uint64_t ਵਿੱਥ) {
  Expected<sys::fs::file_t> FDOrErr = sys::fs::openNativeFileForReadWrite(
      Filename, sys::fs::CD_OpenExisting, sys::fs::ਦਾ_None);
  ਜੇ (!FDOrErr)
    ਵਾਪਸ error_ਮੰਜ਼ਿਲ_Error_ਕੋਡ(FDOrErr.takeError());
  sys::fs::file_t FD = *FDOrErr;

  // Default is to map the full file.
  ਜੇ (Map_ਮਾਪ == uint64_t(-1)) {
    // If we don't know the file size, use fstat to find out.  fstat on an open
    // file descriptor is cheaper than stat on a random path.
    ਜੇ (File_ਮਾਪ == uint64_t(-1)) {
      sys::fs::file_ਸਥਿਤੀ ਸਥਿਤੀ;
      std::error_ਕੋਡ EC = sys::fs::ਸਥਿਤੀ_2(FD, ਸਥਿਤੀ);
      ਜੇ (EC)
        ਵਾਪਸ EC;

      // If this not a file or a block device (e.g. it's a named pipe
      // or character device), we can't mmap it, so error out.
      sys::fs::file_type Type = ਸਥਿਤੀ.type();
      ਜੇ (Type != sys::fs::file_type::regular_file &&
          Type != sys::fs::file_type::ਖੰਡ_file)
        ਵਾਪਸ make_error_ਕੋਡ(errc::invalid_ਦਲੀਲ);

      File_ਮਾਪ = ਸਥਿਤੀ.get_ਮਾਪ();
    }
    Map_ਮਾਪ = File_ਮਾਪ;
  }

  std::error_ਕੋਡ EC;
  std::unique_ptr<Write_Through_ਸਮ੍ਰਿਤੀ_ਬਫਰ> ਨਤੀਜਾ(
      ਨਵਾਂ (Named_ਬਫਰ_Alloc(Filename))
          ਸਮ੍ਰਿਤੀ_ਬਫਰ_MMap_File<Write_Through_ਸਮ੍ਰਿਤੀ_ਬਫਰ>(ਝੂਠ, FD, Map_ਮਾਪ,
                                                         ਵਿੱਥ, EC));
  ਜੇ (EC)
    ਵਾਪਸ EC;
  ਵਾਪਸ std::move(ਨਤੀਜਾ);
}

ErrorOr<std::unique_ptr<Write_Through_ਸਮ੍ਰਿਤੀ_ਬਫਰ>>
Write_Through_ਸਮ੍ਰਿਤੀ_ਬਫਰ::getFile(ਅਚਲ Twine &Filename, int64_t File_ਮਾਪ) {
  sys::sandbox::violationIfEnabled();

  ਵਾਪਸ getReadWriteFile(Filename, File_ਮਾਪ, File_ਮਾਪ, 0);
}

/// Map a subrange of the specified file as a WritableMemoryBuffer.
ErrorOr<std::unique_ptr<Write_Through_ਸਮ੍ਰਿਤੀ_ਬਫਰ>>
Write_Through_ਸਮ੍ਰਿਤੀ_ਬਫਰ::get_File_ਟੁਕੜਾ(ਅਚਲ Twine &Filename, uint64_t Map_ਮਾਪ,
                                       uint64_t ਵਿੱਥ) {
  sys::sandbox::violationIfEnabled();

  ਵਾਪਸ getReadWriteFile(Filename, -1, Map_ਮਾਪ, ਵਿੱਥ);
}

ਸਾਂਚਾ <ਕਿਸਮ_ਨਾਂ MB>
ਸਥਿਰ ErrorOr<std::unique_ptr<MB>>
getOpenFileImpl(sys::fs::file_t FD, ਅਚਲ Twine &Filename, uint64_t File_ਮਾਪ,
                uint64_t Map_ਮਾਪ, int64_t ਵਿੱਥ, ਤਾਰਕਿਕ RequiresNullTerminator,
                ਤਾਰਕਿਕ IsVolatile, std::optional<Align> Alignment) {
  ਸਥਿਰ ਪੂਰਨ ਪੰਨਾ_ਮਾਪ = sys::Process::get_ਪੰਨਾ_ਮਾਪ_Estimate();

  // Default is to map the full file.
  ਜੇ (Map_ਮਾਪ == uint64_t(-1)) {
    // If we don't know the file size, use fstat to find out.  fstat on an open
    // file descriptor is cheaper than stat on a random path.
    ਜੇ (File_ਮਾਪ == uint64_t(-1)) {
      sys::fs::file_ਸਥਿਤੀ ਸਥਿਤੀ;
      std::error_ਕੋਡ EC = sys::fs::ਸਥਿਤੀ_2(FD, ਸਥਿਤੀ);
      ਜੇ (EC)
        ਵਾਪਸ EC;

      // If this not a file or a block device (e.g. it's a named pipe
      // or character device), we can't trust the size. Create the memory
      // buffer by copying off the stream.
      sys::fs::file_type Type = ਸਥਿਤੀ.type();
      ਜੇ (Type != sys::fs::file_type::regular_file &&
          Type != sys::fs::file_type::ਖੰਡ_file)
        ਵਾਪਸ get_ਸਮ੍ਰਿਤੀ_ਬਫਰ_For_ਧਾਰਾ(FD, Filename);

      File_ਮਾਪ = ਸਥਿਤੀ.get_ਮਾਪ();
    }
    Map_ਮਾਪ = File_ਮਾਪ;
  }

  ਜੇ (shouldUseMmap(FD, File_ਮਾਪ, Map_ਮਾਪ, ਵਿੱਥ, RequiresNullTerminator,
                    ਪੰਨਾ_ਮਾਪ, IsVolatile)) {
    std::error_ਕੋਡ EC;
    std::unique_ptr<MB> ਨਤੀਜਾ(
        ਨਵਾਂ (Named_ਬਫਰ_Alloc(Filename)) ਸਮ੍ਰਿਤੀ_ਬਫਰ_MMap_File<MB>(
            RequiresNullTerminator, FD, Map_ਮਾਪ, ਵਿੱਥ, EC));
    ਜੇ (!EC) {
      // On at least Linux, and possibly on other systems, mmap may return pages
      // from the page cache that are not properly filled with trailing zeroes,
      // if some prior user of the page wrote non-zero bytes. Detect this and
      // don't use mmap in that case.
      ਜੇ (!RequiresNullTerminator || *ਨਤੀਜਾ->get_ਬਫਰ_ਸਿਰਾ() == '\0')
        ਵਾਪਸ std::move(ਨਤੀਜਾ);
    }
  }

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ __MVS__
  ErrorOr<ਤਾਰਕਿਕ> NeedsConversion = needConversion(Filename, FD);
  ਜੇ (std::error_ਕੋਡ EC = NeedsConversion.getError())
    ਵਾਪਸ EC;
  // File size may increase due to EBCDIC -> UTF-8 conversion, therefore we
  // cannot trust the file size and we create the memory buffer by copying
  // off the stream.
  // Note: This only works with the assumption of reading a full file (i.e,
  // Offset == 0 and MapSize == FileSize). Reading a file slice does not work.
  ਜੇ (*NeedsConversion && ਵਿੱਥ == 0 && Map_ਮਾਪ == File_ਮਾਪ)
    ਵਾਪਸ get_ਸਮ੍ਰਿਤੀ_ਬਫਰ_For_ਧਾਰਾ(FD, Filename);
#ਜੇ_ਅੰਤ

  ਸਵੈਚਾਲਿਤ Buf =
      Writable_ਸਮ੍ਰਿਤੀ_ਬਫਰ::get_New_Uninit_Mem_ਬਫਰ(Map_ਮਾਪ, Filename, Alignment);
  ਜੇ (!Buf) {
    // Failed to create a buffer. The only way it can fail is if
    // new(std::nothrow) returns 0.
    ਵਾਪਸ make_error_ਕੋਡ(errc::not_enough_ਸਮ੍ਰਿਤੀ);
  }

  // Read until EOF, zero-initialize the rest.
  MutableArrayRef<ਅੱਖਰ> ਮੰਜ਼ਿਲ_Read = Buf->get_ਬਫਰ();
  ਜਦੋਂ (!ਮੰਜ਼ਿਲ_Read.ਖਾਲੀ_ਥਾਵਾਂ()) {
    Expected<size_t> Read_ਬਾਈਟਾਂ =
        sys::fs::read_Native_File_ਟੁਕੜਾ(FD, ਮੰਜ਼ਿਲ_Read, ਵਿੱਥ);
    ਜੇ (!Read_ਬਾਈਟਾਂ)
      ਵਾਪਸ error_ਮੰਜ਼ਿਲ_Error_ਕੋਡ(Read_ਬਾਈਟਾਂ.takeError());
    ਜੇ (*Read_ਬਾਈਟਾਂ == 0) {
      std::memset(ਮੰਜ਼ਿਲ_Read.ਅੰਕੜੇ(), 0, ਮੰਜ਼ਿਲ_Read.ਮਾਪ_2());
      ਰੋਕੋ;
    }
    ਮੰਜ਼ਿਲ_Read = ਮੰਜ਼ਿਲ_Read.drop_front(*Read_ਬਾਈਟਾਂ);
    ਵਿੱਥ += *Read_ਬਾਈਟਾਂ;
  }

  ਵਾਪਸ std::move(Buf);
}

ErrorOr<std::unique_ptr<ਸਮ੍ਰਿਤੀ_ਬਫਰ>>
ਸਮ੍ਰਿਤੀ_ਬਫਰ::getOpenFile(sys::fs::file_t FD, ਅਚਲ Twine &Filename,
                          uint64_t File_ਮਾਪ, ਤਾਰਕਿਕ RequiresNullTerminator,
                          ਤਾਰਕਿਕ IsVolatile, std::optional<Align> Alignment) {
  sys::sandbox::violationIfEnabled();

  ਵਾਪਸ getOpenFileImpl<ਸਮ੍ਰਿਤੀ_ਬਫਰ>(FD, Filename, File_ਮਾਪ, File_ਮਾਪ, 0,
                                       RequiresNullTerminator, IsVolatile,
                                       Alignment);
}

ErrorOr<std::unique_ptr<ਸਮ੍ਰਿਤੀ_ਬਫਰ>> ਸਮ੍ਰਿਤੀ_ਬਫਰ::get_Open_File_ਟੁਕੜਾ(
    sys::fs::file_t FD, ਅਚਲ Twine &Filename, uint64_t Map_ਮਾਪ, int64_t ਵਿੱਥ,
    ਤਾਰਕਿਕ IsVolatile, std::optional<Align> Alignment) {
  ਪ੍ਰਤਿਜ੍ਞਾ(Map_ਮਾਪ != uint64_t(-1));

  sys::sandbox::violationIfEnabled();

  ਵਾਪਸ getOpenFileImpl<ਸਮ੍ਰਿਤੀ_ਬਫਰ>(FD, Filename, -1, Map_ਮਾਪ, ਵਿੱਥ, ਝੂਠ,
                                       IsVolatile, Alignment);
}

ErrorOr<std::unique_ptr<ਸਮ੍ਰਿਤੀ_ਬਫਰ>> ਸਮ੍ਰਿਤੀ_ਬਫਰ::getSTDIN() {
  sys::sandbox::violationIfEnabled();

  // Read in all of the data from stdin, we cannot mmap stdin.
  //
  // FIXME: That isn't necessarily true, we should try to mmap stdin and
  // fallback if it fails.
  sys::Change_Stdin_ਬਹੁਲਕ(sys::fs::ਦਾ_ਪਾਠ);

  ਵਾਪਸ get_ਸਮ੍ਰਿਤੀ_ਬਫਰ_For_ਧਾਰਾ(sys::fs::getStdinHandle(), "<stdin>");
}

ErrorOr<std::unique_ptr<ਸਮ੍ਰਿਤੀ_ਬਫਰ>>
ਸਮ੍ਰਿਤੀ_ਬਫਰ::get_File_As_ਧਾਰਾ(ਅਚਲ Twine &Filename) {
  sys::sandbox::violationIfEnabled();

  Expected<sys::fs::file_t> FDOrErr =
      sys::fs::openNativeFileForRead(Filename, sys::fs::ਦਾ_None);
  ਜੇ (!FDOrErr)
    ਵਾਪਸ error_ਮੰਜ਼ਿਲ_Error_ਕੋਡ(FDOrErr.takeError());
  sys::fs::file_t FD = *FDOrErr;
  ErrorOr<std::unique_ptr<ਸਮ੍ਰਿਤੀ_ਬਫਰ>> Ret =
      get_ਸਮ੍ਰਿਤੀ_ਬਫਰ_For_ਧਾਰਾ(FD, Filename);
  sys::fs::closeFile(FD);
  ਵਾਪਸ Ret;
}

ਸਮ੍ਰਿਤੀ_ਬਫਰ_Ref ਸਮ੍ਰਿਤੀ_ਬਫਰ::get_Mem_ਬਫਰ_Ref() ਅਚਲ {
  StringRef ਅੰਕੜੇ_2 = get_ਬਫਰ();
  StringRef Identifier = get_ਬਫਰ_Identifier();
  ਵਾਪਸ ਸਮ੍ਰਿਤੀ_ਬਫਰ_Ref(ਅੰਕੜੇ_2, Identifier);
}

Small_ਸਦਿਸ਼_ਸਮ੍ਰਿਤੀ_ਬਫਰ::~Small_ਸਦਿਸ਼_ਸਮ੍ਰਿਤੀ_ਬਫਰ() = ਮੂਲ;
