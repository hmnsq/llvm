//===- SourceMgr.cpp - Manager for Simple Source Buffers & Diagnostics ----===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file implements the SourceMgr class.  This class is used as a simple
// substrate for diagnostics, #include handling, and other low level things for
// simple parsers.
//
//===----------------------------------------------------------------------===//

#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/SourceMgr.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/ADT/ArrayRef.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/ADT/STLExtras.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/ADT/SmallString.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/ADT/SmallVector.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/ADT/StringRef.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/ADT/Twine.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/ErrorOr.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/Locale.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/MemoryBuffer.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/Path.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/SMLoc.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/VirtualFileSystem.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/WithColor.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/raw_ostream.h"
#ਅਨ੍ਤਰ੍ਭਾਵ <algorithm>
#ਅਨ੍ਤਰ੍ਭਾਵ <cassert>
#ਅਨ੍ਤਰ੍ਭਾਵ <cstddef>
#ਅਨ੍ਤਰ੍ਭਾਵ <limits>
#ਅਨ੍ਤਰ੍ਭਾਵ <memory>
#ਅਨ੍ਤਰ੍ਭਾਵ <string>
#ਅਨ੍ਤਰ੍ਭਾਵ <utility>

ਵਰਤੋਂ ਨਾਮਾਕਾਸ਼ llvm;

ਸਥਿਰ ਅਚਲ size_t TabStop = 8;

// Out of line to avoid needing definition of vfs::FileSystem in header.
ਸਰੋਤ_Mgr::ਸਰੋਤ_Mgr() = ਮੂਲ;
ਸਰੋਤ_Mgr::ਸਰੋਤ_Mgr(IntrusiveRefCntPtr<vfs::FileSystem> FS)
    : FS(std::move(FS)) {}
ਸਰੋਤ_Mgr::ਸਰੋਤ_Mgr(ਸਰੋਤ_Mgr &&) = ਮੂਲ;
ਸਰੋਤ_Mgr &ਸਰੋਤ_Mgr::ਸੰਚਾਲਕ=(ਸਰੋਤ_Mgr &&) = ਮੂਲ;
ਸਰੋਤ_Mgr::~ਸਰੋਤ_Mgr() = ਮੂਲ;

IntrusiveRefCntPtr<vfs::FileSystem> ਸਰੋਤ_Mgr::getVirtualFileSystem() ਅਚਲ {
  ਵਾਪਸ FS;
}

ਖਾਲੀ ਸਰੋਤ_Mgr::ਸੈੱਟ_Virtual_File_System(IntrusiveRefCntPtr<vfs::FileSystem> FS) {
  ਇਹ->FS = std::move(FS);
}

ਬਿਨਾਨਿਸ਼ਾਨੀ ਸਰੋਤ_Mgr::ਜੋੜੋ_Include_File(ਅਚਲ std::string &Filename,
                                   SMLoc IncludeLoc,
                                   std::string &IncludedFile) {
  ErrorOr<std::unique_ptr<ਸਮ੍ਰਿਤੀ_ਬਫਰ>> NewBufOrErr =
      OpenIncludeFile(Filename, IncludedFile);
  ਜੇ (!NewBufOrErr)
    ਵਾਪਸ 0;

  ਵਾਪਸ ਜੋੜੋ_New_ਸਰੋਤ_ਬਫਰ(std::move(*NewBufOrErr), IncludeLoc);
}

ErrorOr<std::unique_ptr<ਸਮ੍ਰਿਤੀ_ਬਫਰ>>
ਸਰੋਤ_Mgr::OpenIncludeFile(ਅਚਲ std::string &Filename,
                           std::string &IncludedFile,
                           ਤਾਰਕਿਕ RequiresNullTerminator) {
  ਸਵੈਚਾਲਿਤ GetFile = [ਇਹ, RequiresNullTerminator](StringRef ਰਸਤਾ) {
    ਵਾਪਸ FS ? FS->get_ਬਫਰ_For_File(ਰਸਤਾ, /*FileSize=*/-1,
                                     RequiresNullTerminator)
              : ਸਮ੍ਰਿਤੀ_ਬਫਰ::getFile(ਰਸਤਾ, /*IsText=*/ਝੂਠ,
                                      RequiresNullTerminator);
  };

  ErrorOr<std::unique_ptr<ਸਮ੍ਰਿਤੀ_ਬਫਰ>> NewBufOrErr = GetFile(Filename);

  SmallString<64> ਬਫਰ(Filename);
  // If the file didn't exist directly, see if it's in an include path.
  ਲਈ (ਬਿਨਾਨਿਸ਼ਾਨੀ i = 0, e = IncludeDirectories.ਮਾਪ_2(); i != e && !NewBufOrErr;
       ++i) {
    ਬਫਰ = IncludeDirectories[i];
    sys::ਰਸਤਾ_2::append(ਬਫਰ, Filename);
    NewBufOrErr = GetFile(ਬਫਰ);
  }

  ਜੇ (NewBufOrErr)
    IncludedFile = static_cast<std::string>(ਬਫਰ);

  ਵਾਪਸ NewBufOrErr;
}

ਬਿਨਾਨਿਸ਼ਾਨੀ ਸਰੋਤ_Mgr::ਲੱਭੋ_ਬਫਰ_Containing_Loc(SMLoc Loc) ਅਚਲ {
  ਲਈ (ਬਿਨਾਨਿਸ਼ਾਨੀ i = 0, e = Buffers.ਮਾਪ_2(); i != e; ++i)
    ਜੇ (Loc.get_ਸੂਚਕ() >= Buffers[i].ਬਫਰ->get_ਬਫਰ_ਸ਼ੁਰੂ() &&
        // Use <= here so that a pointer to the null at the end of the buffer
        // is included as part of the buffer.
        Loc.get_ਸੂਚਕ() <= Buffers[i].ਬਫਰ->get_ਬਫਰ_ਸਿਰਾ())
      ਵਾਪਸ i + 1;
  ਵਾਪਸ 0;
}

ਸਾਂਚਾ <ਕਿਸਮ_ਨਾਂ T>
ਸਥਿਰ std::ਸਦਿਸ਼<T> &Get_Or_Create_ਵਿੱਥ_Cache(ਖਾਲੀ *&ਵਿੱਥ_Cache,
                                              ਸਮ੍ਰਿਤੀ_ਬਫਰ *ਬਫਰ) {
  ਜੇ (ਵਿੱਥ_Cache)
    ਵਾਪਸ *static_cast<std::ਸਦਿਸ਼<T> *>(ਵਿੱਥ_Cache);

  // Lazily fill in the offset cache.
  ਸਵੈਚਾਲਿਤ *Offsets = ਨਵਾਂ std::ਸਦਿਸ਼<T>();
  size_t Sz = ਬਫਰ->get_ਬਫਰ_ਮਾਪ();
  ਪ੍ਰਤਿਜ੍ਞਾ(Sz <= std::numeric_limits<T>::max());
  StringRef S = ਬਫਰ->get_ਬਫਰ();
  ਲਈ (size_t N = 0; N < Sz; ++N) {
    ਜੇ (S[N] == '\n')
      Offsets->ਧੱਕੋ_ਪਿੱਠ(static_cast<T>(N));
  }

  ਵਿੱਥ_Cache = Offsets;
  ਵਾਪਸ *Offsets;
}

ਸਾਂਚਾ <ਕਿਸਮ_ਨਾਂ T>
std::pair<ਬਿਨਾਨਿਸ਼ਾਨੀ, ਬਿਨਾਨਿਸ਼ਾਨੀ>
ਸਰੋਤ_Mgr::Src_ਬਫਰ::get_ਸਤਰ_And_ਕਾਲਮ_Specialized(ਅਚਲ ਅੱਖਰ *Ptr) ਅਚਲ {
  std::ਸਦਿਸ਼<T> &Offsets =
      Get_Or_Create_ਵਿੱਥ_Cache<T>(ਵਿੱਥ_Cache, ਬਫਰ.get());

  ਅਚਲ ਅੱਖਰ *Buf_ਸ਼ੁਰੂ = ਬਫਰ->get_ਬਫਰ_ਸ਼ੁਰੂ();
  ਪ੍ਰਤਿਜ੍ਞਾ(Ptr >= Buf_ਸ਼ੁਰੂ && Ptr <= ਬਫਰ->get_ਬਫਰ_ਸਿਰਾ());
  ptrdiff_t PtrDiff = Ptr - Buf_ਸ਼ੁਰੂ;
  ਪ੍ਰਤਿਜ੍ਞਾ(PtrDiff >= 0 &&
         static_cast<size_t>(PtrDiff) <= std::numeric_limits<T>::max());
  T Ptr_ਵਿੱਥ = static_cast<T>(PtrDiff);

  // llvm::lower_bound gives the number of EOL before PtrOffset. Add 1 to get
  // the line number.
  ਸਵੈਚਾਲਿਤ I = llvm::ਹੇਠਲਾ_bound(Offsets, Ptr_ਵਿੱਥ);
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਸਤਰ_No = I - Offsets.begin() + 1;

  // The column number is the distance from the previous EOL (or the start of
  // the buffer) to the pointer.
  T ਸਤਰ_ਸ਼ੁਰੂ_Offs = (I == Offsets.begin()) ? 0 : (I[-1] + 1);
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਖਾਨਾ_No = Ptr_ਵਿੱਥ - ਸਤਰ_ਸ਼ੁਰੂ_Offs + 1;
  ਵਾਪਸ {ਸਤਰ_No, ਖਾਨਾ_No};
}

/// Look up a given \p Ptr in the buffer, determining which line and column
/// it came from.
LLVM_ABI std::pair<ਬਿਨਾਨਿਸ਼ਾਨੀ, ਬਿਨਾਨਿਸ਼ਾਨੀ>
ਸਰੋਤ_Mgr::Src_ਬਫਰ::get_ਸਤਰ_And_ਕਾਲਮ(ਅਚਲ ਅੱਖਰ *Ptr) ਅਚਲ {
  size_t Sz = ਬਫਰ->get_ਬਫਰ_ਮਾਪ();
  ਜੇ (Sz <= std::numeric_limits<uint8_t>::max())
    ਵਾਪਸ get_ਸਤਰ_And_ਕਾਲਮ_Specialized<uint8_t>(Ptr);
  ਜੇ (Sz <= std::numeric_limits<uint16_t>::max())
    ਵਾਪਸ get_ਸਤਰ_And_ਕਾਲਮ_Specialized<uint16_t>(Ptr);
  ਜੇ (Sz <= std::numeric_limits<uint32_t>::max())
    ਵਾਪਸ get_ਸਤਰ_And_ਕਾਲਮ_Specialized<uint32_t>(Ptr);
  ਵਾਪਸ get_ਸਤਰ_And_ਕਾਲਮ_Specialized<uint64_t>(Ptr);
}

ਸਾਂਚਾ <ਕਿਸਮ_ਨਾਂ T>
ਅਚਲ ਅੱਖਰ *ਸਰੋਤ_Mgr::Src_ਬਫਰ::get_ਸੂਚਕ_For_ਸਤਰ_ਸੰਖਿਆ_Specialized(
    ਬਿਨਾਨਿਸ਼ਾਨੀ ਸਤਰ_No) ਅਚਲ {
  std::ਸਦਿਸ਼<T> &Offsets =
      Get_Or_Create_ਵਿੱਥ_Cache<T>(ਵਿੱਥ_Cache, ਬਫਰ.get());

  // We start counting line and column numbers from 1.
  ਜੇ (ਸਤਰ_No != 0)
    --ਸਤਰ_No;

  ਅਚਲ ਅੱਖਰ *Buf_ਸ਼ੁਰੂ = ਬਫਰ->get_ਬਫਰ_ਸ਼ੁਰੂ();

  // The offset cache contains the location of the \n for the specified line,
  // we want the start of the line.  As such, we look for the previous entry.
  ਜੇ (ਸਤਰ_No == 0)
    ਵਾਪਸ Buf_ਸ਼ੁਰੂ;
  ਜੇ (ਸਤਰ_No > Offsets.ਮਾਪ_2())
    ਵਾਪਸ ਖਾਲੀ_ਸੂਚਕ;
  ਵਾਪਸ Buf_ਸ਼ੁਰੂ + Offsets[ਸਤਰ_No - 1] + 1;
}

/// Return a pointer to the first character of the specified line number or
/// null if the line number is invalid.
ਅਚਲ ਅੱਖਰ *
ਸਰੋਤ_Mgr::Src_ਬਫਰ::get_ਸੂਚਕ_For_ਸਤਰ_ਸੰਖਿਆ(ਬਿਨਾਨਿਸ਼ਾਨੀ ਸਤਰ_No) ਅਚਲ {
  size_t Sz = ਬਫਰ->get_ਬਫਰ_ਮਾਪ();
  ਜੇ (Sz <= std::numeric_limits<uint8_t>::max())
    ਵਾਪਸ get_ਸੂਚਕ_For_ਸਤਰ_ਸੰਖਿਆ_Specialized<uint8_t>(ਸਤਰ_No);
  ਨਹੀਂਤਾਂ ਜੇ (Sz <= std::numeric_limits<uint16_t>::max())
    ਵਾਪਸ get_ਸੂਚਕ_For_ਸਤਰ_ਸੰਖਿਆ_Specialized<uint16_t>(ਸਤਰ_No);
  ਨਹੀਂਤਾਂ ਜੇ (Sz <= std::numeric_limits<uint32_t>::max())
    ਵਾਪਸ get_ਸੂਚਕ_For_ਸਤਰ_ਸੰਖਿਆ_Specialized<uint32_t>(ਸਤਰ_No);
  ਨਹੀਂਤਾਂ
    ਵਾਪਸ get_ਸੂਚਕ_For_ਸਤਰ_ਸੰਖਿਆ_Specialized<uint64_t>(ਸਤਰ_No);
}

ਸਰੋਤ_Mgr::Src_ਬਫਰ::Src_ਬਫਰ(ਸਰੋਤ_Mgr::Src_ਬਫਰ &&Other)
    : ਬਫਰ(std::move(Other.ਬਫਰ)), ਵਿੱਥ_Cache(Other.ਵਿੱਥ_Cache),
      IncludeLoc(Other.IncludeLoc) {
  Other.ਵਿੱਥ_Cache = ਖਾਲੀ_ਸੂਚਕ;
}

ਸਰੋਤ_Mgr::Src_ਬਫਰ::~Src_ਬਫਰ() {
  ਜੇ (ਵਿੱਥ_Cache) {
    size_t Sz = ਬਫਰ->get_ਬਫਰ_ਮਾਪ();
    ਜੇ (Sz <= std::numeric_limits<uint8_t>::max())
      ਮਿਟਾਓ static_cast<std::ਸਦਿਸ਼<uint8_t> *>(ਵਿੱਥ_Cache);
    ਨਹੀਂਤਾਂ ਜੇ (Sz <= std::numeric_limits<uint16_t>::max())
      ਮਿਟਾਓ static_cast<std::ਸਦਿਸ਼<uint16_t> *>(ਵਿੱਥ_Cache);
    ਨਹੀਂਤਾਂ ਜੇ (Sz <= std::numeric_limits<uint32_t>::max())
      ਮਿਟਾਓ static_cast<std::ਸਦਿਸ਼<uint32_t> *>(ਵਿੱਥ_Cache);
    ਨਹੀਂਤਾਂ
      ਮਿਟਾਓ static_cast<std::ਸਦਿਸ਼<uint64_t> *>(ਵਿੱਥ_Cache);
    ਵਿੱਥ_Cache = ਖਾਲੀ_ਸੂਚਕ;
  }
}

std::pair<ਬਿਨਾਨਿਸ਼ਾਨੀ, ਬਿਨਾਨਿਸ਼ਾਨੀ>
ਸਰੋਤ_Mgr::get_ਸਤਰ_And_ਕਾਲਮ(SMLoc Loc, ਬਿਨਾਨਿਸ਼ਾਨੀ ਬਫਰ_ID) ਅਚਲ {
  ਜੇ (!ਬਫਰ_ID)
    ਬਫਰ_ID = ਲੱਭੋ_ਬਫਰ_Containing_Loc(Loc);
  ਪ੍ਰਤਿਜ੍ਞਾ(ਬਫਰ_ID && "Invalid location!");

  ਸਵੈਚਾਲਿਤ &SB = get_ਬਫਰ_Info(ਬਫਰ_ID);
  ਅਚਲ ਅੱਖਰ *Ptr = Loc.get_ਸੂਚਕ();

  ਵਾਪਸ SB.get_ਸਤਰ_And_ਕਾਲਮ(Ptr);
}

// FIXME: Note that the formatting of source locations is spread between
// multiple functions, some in SourceMgr and some in SMDiagnostic. A better
// solution would be a general-purpose source location formatter
// in one of those two classes, or possibly in SMLoc.

/// Get a string with the source location formatted in the standard
/// style, but without the line offset. If \p IncludePath is true, the path
/// is included. If false, only the file name and extension are included.
std::string ਸਰੋਤ_Mgr::get_Formatted_Location_No_ਵਿੱਥ(SMLoc Loc,
                                                    ਤਾਰਕਿਕ Include_ਰਸਤਾ) ਅਚਲ {
  ਸਵੈਚਾਲਿਤ ਬਫਰ_ID = ਲੱਭੋ_ਬਫਰ_Containing_Loc(Loc);
  ਪ੍ਰਤਿਜ੍ਞਾ(ਬਫਰ_ID && "Invalid location!");
  ਸਵੈਚਾਲਿਤ FileSpec = get_ਬਫਰ_Info(ਬਫਰ_ID).ਬਫਰ->get_ਬਫਰ_Identifier();

  ਜੇ (Include_ਰਸਤਾ) {
    ਵਾਪਸ FileSpec.str() + ":" + std::ਮੰਜ਼ਿਲ_string(ਲੱਭੋ_ਸਤਰ_ਸੰਖਿਆ(Loc, ਬਫਰ_ID));
  } ਨਹੀਂਤਾਂ {
    ਸਵੈਚਾਲਿਤ I = FileSpec.ਲੱਭੋ_ਆਖਰੀ_ਦਾ("/\\");
    I = (I == FileSpec.ਮਾਪ_2()) ? 0 : (I + 1);
    ਵਾਪਸ FileSpec.substr(I).str() + ":" +
           std::ਮੰਜ਼ਿਲ_string(ਲੱਭੋ_ਸਤਰ_ਸੰਖਿਆ(Loc, ਬਫਰ_ID));
  }
}

/// Given a line and column number in a mapped buffer, turn it into an SMLoc.
/// This will return a null SMLoc if the line/column location is invalid.
SMLoc ਸਰੋਤ_Mgr::ਲੱਭੋ_Loc_For_ਸਤਰ_And_ਕਾਲਮ(ਬਿਨਾਨਿਸ਼ਾਨੀ ਬਫਰ_ID, ਬਿਨਾਨਿਸ਼ਾਨੀ ਸਤਰ_No,
                                         ਬਿਨਾਨਿਸ਼ਾਨੀ ਖਾਨਾ_No) {
  ਸਵੈਚਾਲਿਤ &SB = get_ਬਫਰ_Info(ਬਫਰ_ID);
  ਅਚਲ ਅੱਖਰ *Ptr = SB.get_ਸੂਚਕ_For_ਸਤਰ_ਸੰਖਿਆ(ਸਤਰ_No);
  ਜੇ (!Ptr)
    ਵਾਪਸ SMLoc();

  // We start counting line and column numbers from 1.
  ਜੇ (ਖਾਨਾ_No != 0)
    --ਖਾਨਾ_No;

  // If we have a column number, validate it.
  ਜੇ (ਖਾਨਾ_No) {
    // Make sure the location is within the current line.
    ਜੇ (Ptr + ਖਾਨਾ_No > SB.ਬਫਰ->get_ਬਫਰ_ਸਿਰਾ())
      ਵਾਪਸ SMLoc();

    // Make sure there is no newline in the way.
    ਜੇ (StringRef(Ptr, ਖਾਨਾ_No).ਲੱਭੋ_ਪਹਿਲਾ_ਦਾ("\n\r") != StringRef::npos)
      ਵਾਪਸ SMLoc();

    Ptr += ਖਾਨਾ_No;
  }

  ਵਾਪਸ SMLoc::get_From_ਸੂਚਕ(Ptr);
}

ਖਾਲੀ ਸਰੋਤ_Mgr::Print_Include_ਢੇਰ(SMLoc IncludeLoc, raw_ostream &OS) ਅਚਲ {
  ਜੇ (IncludeLoc == SMLoc())
    ਵਾਪਸ; // Top of stack.

  ਬਿਨਾਨਿਸ਼ਾਨੀ CurBuf = ਲੱਭੋ_ਬਫਰ_Containing_Loc(IncludeLoc);
  ਪ੍ਰਤਿਜ੍ਞਾ(CurBuf && "Invalid or unspecified location!");

  Print_Include_ਢੇਰ(get_ਬਫਰ_Info(CurBuf).IncludeLoc, OS);

  OS << "Included from " << get_ਬਫਰ_Info(CurBuf).ਬਫਰ->get_ਬਫਰ_Identifier()
     << ":" << ਲੱਭੋ_ਸਤਰ_ਸੰਖਿਆ(IncludeLoc, CurBuf) << ":\n";
}

ਖਾਲੀ ਸਰੋਤ_Mgr::print_Include_ਢੇਰ_For_Diagnostic(SMLoc Loc,
                                               raw_ostream &OS) ਅਚਲ {
  ਜੇ (!Loc.isValid())
    ਵਾਪਸ;
  ਬਿਨਾਨਿਸ਼ਾਨੀ Diag_Cur_ਬਫਰ = ਲੱਭੋ_ਬਫਰ_Containing_Loc(Loc);
  ਜੇ (Diag_Cur_ਬਫਰ && Diag_Cur_ਬਫਰ != getMainFileID()) {
    SMLoc ਮਾਪਾ_Include_Loc = get_ਮਾਪਾ_Include_Loc(Diag_Cur_ਬਫਰ);
    // Ignore macro instantiation buffers to avoid redundant include stacks.
    Print_Include_ਢੇਰ(ਮਾਪਾ_Include_Loc, OS);
  }
}

SMDiagnostic ਸਰੋਤ_Mgr::Get_ਸੁਨੇਹਾ(SMLoc Loc, ਸਰੋਤ_Mgr::Diag_ਕਿਸਮ ਕਿਸਮ_3,
                                   ਅਚਲ Twine &ਸੁਨੇਹਾ_4, ArrayRef<SMRange> Ranges,
                                   ArrayRef<SMFixIt> FixIts) ਅਚਲ {
  // First thing to do: find the current buffer containing the specified
  // location to pull out the source line.
  Small_ਸਦਿਸ਼<std::pair<ਬਿਨਾਨਿਸ਼ਾਨੀ, ਬਿਨਾਨਿਸ਼ਾਨੀ>, 4> ਖਾਨਾ_Ranges;
  std::pair<ਬਿਨਾਨਿਸ਼ਾਨੀ, ਬਿਨਾਨਿਸ਼ਾਨੀ> ਸਤਰ_And_ਖਾਨਾ;
  StringRef ਬਫਰ_ID = "<unknown>";
  StringRef ਸਤਰ_Str;

  ਜੇ (Loc.isValid()) {
    ਬਿਨਾਨਿਸ਼ਾਨੀ CurBuf = ਲੱਭੋ_ਬਫਰ_Containing_Loc(Loc);
    ਪ੍ਰਤਿਜ੍ਞਾ(CurBuf && "Invalid or unspecified location!");

    ਅਚਲ ਸਮ੍ਰਿਤੀ_ਬਫਰ *CurMB = get_ਸਮ੍ਰਿਤੀ_ਬਫਰ(CurBuf);
    ਬਫਰ_ID = CurMB->get_ਬਫਰ_Identifier();

    // Scan backward to find the start of the line.
    ਅਚਲ ਅੱਖਰ *ਸਤਰ_ਸ਼ੁਰੂ = Loc.get_ਸੂਚਕ();
    ਅਚਲ ਅੱਖਰ *Buf_ਸ਼ੁਰੂ = CurMB->get_ਬਫਰ_ਸ਼ੁਰੂ();
    ਜਦੋਂ (ਸਤਰ_ਸ਼ੁਰੂ != Buf_ਸ਼ੁਰੂ && ਸਤਰ_ਸ਼ੁਰੂ[-1] != '\n' &&
           ਸਤਰ_ਸ਼ੁਰੂ[-1] != '\r')
      --ਸਤਰ_ਸ਼ੁਰੂ;

    // Get the end of the line.
    ਅਚਲ ਅੱਖਰ *ਸਤਰ_ਸਿਰਾ = Loc.get_ਸੂਚਕ();
    ਅਚਲ ਅੱਖਰ *Buf_ਸਿਰਾ = CurMB->get_ਬਫਰ_ਸਿਰਾ();
    ਜਦੋਂ (ਸਤਰ_ਸਿਰਾ != Buf_ਸਿਰਾ && ਸਤਰ_ਸਿਰਾ[0] != '\n' && ਸਤਰ_ਸਿਰਾ[0] != '\r')
      ++ਸਤਰ_ਸਿਰਾ;
    ਸਤਰ_Str = StringRef(ਸਤਰ_ਸ਼ੁਰੂ, ਸਤਰ_ਸਿਰਾ - ਸਤਰ_ਸ਼ੁਰੂ);

    // Convert any ranges to column ranges that only intersect the line of the
    // location.
    ਲਈ (SMRange R : Ranges) {
      ਜੇ (!R.isValid())
        ਜਾਰੀ;

      // If the line doesn't contain any part of the range, then ignore it.
      ਜੇ (R.ਸ਼ੁਰੂ.get_ਸੂਚਕ() > ਸਤਰ_ਸਿਰਾ || R.ਸਿਰਾ_2.get_ਸੂਚਕ() < ਸਤਰ_ਸ਼ੁਰੂ)
        ਜਾਰੀ;

      // Ignore pieces of the range that go onto other lines.
      ਜੇ (R.ਸ਼ੁਰੂ.get_ਸੂਚਕ() < ਸਤਰ_ਸ਼ੁਰੂ)
        R.ਸ਼ੁਰੂ = SMLoc::get_From_ਸੂਚਕ(ਸਤਰ_ਸ਼ੁਰੂ);
      ਜੇ (R.ਸਿਰਾ_2.get_ਸੂਚਕ() > ਸਤਰ_ਸਿਰਾ)
        R.ਸਿਰਾ_2 = SMLoc::get_From_ਸੂਚਕ(ਸਤਰ_ਸਿਰਾ);

      // Translate from SMLoc ranges to column ranges.
      // FIXME: Handle multibyte characters.
      ਖਾਨਾ_Ranges.ਧੱਕੋ_ਪਿੱਠ(std::make_pair(R.ਸ਼ੁਰੂ.get_ਸੂਚਕ() - ਸਤਰ_ਸ਼ੁਰੂ,
                                         R.ਸਿਰਾ_2.get_ਸੂਚਕ() - ਸਤਰ_ਸ਼ੁਰੂ));
    }

    ਸਤਰ_And_ਖਾਨਾ = get_ਸਤਰ_And_ਕਾਲਮ(Loc, CurBuf);
  }

  ਵਾਪਸ SMDiagnostic(*ਇਹ, Loc, ਬਫਰ_ID, ਸਤਰ_And_ਖਾਨਾ.ਪਹਿਲਾ_2,
                      ਸਤਰ_And_ਖਾਨਾ.ਦੂਜਾ_2 - 1, ਕਿਸਮ_3, ਸੁਨੇਹਾ_4.str(), ਸਤਰ_Str,
                      ਖਾਨਾ_Ranges, FixIts);
}

ਖਾਲੀ ਸਰੋਤ_Mgr::Print_ਸੁਨੇਹਾ(raw_ostream &OS, ਅਚਲ SMDiagnostic &Diagnostic,
                             ਤਾਰਕਿਕ ShowColors) ਅਚਲ {
  // Report the message with the diagnostic handler if present.
  ਜੇ (DiagHandler) {
    DiagHandler(Diagnostic, DiagContext);
    ਵਾਪਸ;
  }

  ਜੇ (Diagnostic.getLoc().isValid()) {
    ਬਿਨਾਨਿਸ਼ਾਨੀ CurBuf = ਲੱਭੋ_ਬਫਰ_Containing_Loc(Diagnostic.getLoc());
    ਪ੍ਰਤਿਜ੍ਞਾ(CurBuf && "Invalid or unspecified location!");
    Print_Include_ਢੇਰ(get_ਬਫਰ_Info(CurBuf).IncludeLoc, OS);
  }

  Diagnostic.print(ਖਾਲੀ_ਸੂਚਕ, OS, ShowColors);
}

ਖਾਲੀ ਸਰੋਤ_Mgr::Print_ਸੁਨੇਹਾ(raw_ostream &OS, SMLoc Loc,
                             ਸਰੋਤ_Mgr::Diag_ਕਿਸਮ ਕਿਸਮ_3, ਅਚਲ Twine &ਸੁਨੇਹਾ_4,
                             ArrayRef<SMRange> Ranges, ArrayRef<SMFixIt> FixIts,
                             ਤਾਰਕਿਕ ShowColors) ਅਚਲ {
  Print_ਸੁਨੇਹਾ(OS, Get_ਸੁਨੇਹਾ(Loc, ਕਿਸਮ_3, ਸੁਨੇਹਾ_4, Ranges, FixIts), ShowColors);
}

ਖਾਲੀ ਸਰੋਤ_Mgr::Print_ਸੁਨੇਹਾ(SMLoc Loc, ਸਰੋਤ_Mgr::Diag_ਕਿਸਮ ਕਿਸਮ_3,
                             ਅਚਲ Twine &ਸੁਨੇਹਾ_4, ArrayRef<SMRange> Ranges,
                             ArrayRef<SMFixIt> FixIts, ਤਾਰਕਿਕ ShowColors) ਅਚਲ {
  Print_ਸੁਨੇਹਾ(errs(), Loc, ਕਿਸਮ_3, ਸੁਨੇਹਾ_4, Ranges, FixIts, ShowColors);
}

//===----------------------------------------------------------------------===//
// SMFixIt Implementation
//===----------------------------------------------------------------------===//

SMFixIt::SMFixIt(SMRange R, ਅਚਲ Twine &Replacement)
    : Range(R), ਪਾਠ_2(Replacement.str()) {
  ਪ੍ਰਤਿਜ੍ਞਾ(R.isValid());
}

//===----------------------------------------------------------------------===//
// SMDiagnostic Implementation
//===----------------------------------------------------------------------===//

SMDiagnostic::SMDiagnostic(ਅਚਲ ਸਰੋਤ_Mgr &sm, SMLoc L, StringRef FN, ਪੂਰਨ ਸਤਰ,
                           ਪੂਰਨ ਖਾਨਾ, ਸਰੋਤ_Mgr::Diag_ਕਿਸਮ ਕਿਸਮ_3, StringRef ਸੁਨੇਹਾ_4,
                           StringRef ਸਤਰ_Str,
                           ArrayRef<std::pair<ਬਿਨਾਨਿਸ਼ਾਨੀ, ਬਿਨਾਨਿਸ਼ਾਨੀ>> Ranges,
                           ArrayRef<SMFixIt> Hints)
    : SM(&sm), Loc(L), Filename(std::string(FN)), ਸਤਰ_No(ਸਤਰ), ਕਾਲਮ_No(ਖਾਨਾ),
      ਕਿਸਮ_3(ਕਿਸਮ_3), ਸੁਨੇਹਾ_3(ਸੁਨੇਹਾ_4), ਸਤਰ_Contents(ਸਤਰ_Str), Ranges(Ranges.vec()),
      FixIts(Hints) {
  llvm::ਛਾਂਟੋ(FixIts);
}

ਸਥਿਰ ਖਾਲੀ build_Fix_It_ਸਤਰ(std::string &Caret_ਸਤਰ, std::string &Fix_It_ਸਤਰ,
                           ArrayRef<SMFixIt> FixIts,
                           ArrayRef<ਅੱਖਰ> ਸਰੋਤ_ਸਤਰ) {
  ਜੇ (FixIts.ਖਾਲੀ_ਥਾਵਾਂ())
    ਵਾਪਸ;

  ਅਚਲ ਅੱਖਰ *ਸਤਰ_ਸ਼ੁਰੂ = ਸਰੋਤ_ਸਤਰ.begin();
  ਅਚਲ ਅੱਖਰ *ਸਤਰ_ਸਿਰਾ = ਸਰੋਤ_ਸਤਰ.ਸਿਰਾ();

  size_t Prev_Hint_ਸਿਰਾ_ਖਾਨਾ = 0;

  ਲਈ (ਅਚਲ llvm::SMFixIt &Fixit : FixIts) {
    // If the fixit contains a newline or tab, ignore it.
    ਜੇ (Fixit.get_ਪਾਠ().ਲੱਭੋ_ਪਹਿਲਾ_ਦਾ("\n\r\t") != StringRef::npos)
      ਜਾਰੀ;

    SMRange R = Fixit.getRange();

    // If the line doesn't contain any part of the range, then ignore it.
    ਜੇ (R.ਸ਼ੁਰੂ.get_ਸੂਚਕ() > ਸਤਰ_ਸਿਰਾ || R.ਸਿਰਾ_2.get_ਸੂਚਕ() < ਸਤਰ_ਸ਼ੁਰੂ)
      ਜਾਰੀ;

    // Translate from SMLoc to column.
    // Ignore pieces of the range that go onto other lines.
    // FIXME: Handle multibyte characters in the source line.
    ਬਿਨਾਨਿਸ਼ਾਨੀ ਪਹਿਲਾ_ਖਾਨਾ;
    ਜੇ (R.ਸ਼ੁਰੂ.get_ਸੂਚਕ() < ਸਤਰ_ਸ਼ੁਰੂ)
      ਪਹਿਲਾ_ਖਾਨਾ = 0;
    ਨਹੀਂਤਾਂ
      ਪਹਿਲਾ_ਖਾਨਾ = R.ਸ਼ੁਰੂ.get_ਸੂਚਕ() - ਸਤਰ_ਸ਼ੁਰੂ;

    // If we inserted a long previous hint, push this one forwards, and add
    // an extra space to show that this is not part of the previous
    // completion. This is sort of the best we can do when two hints appear
    // to overlap.
    //
    // Note that if this hint is located immediately after the previous
    // hint, no space will be added, since the location is more important.
    ਬਿਨਾਨਿਸ਼ਾਨੀ Hint_ਖਾਨਾ = ਪਹਿਲਾ_ਖਾਨਾ;
    ਜੇ (Hint_ਖਾਨਾ < Prev_Hint_ਸਿਰਾ_ਖਾਨਾ)
      Hint_ਖਾਨਾ = Prev_Hint_ਸਿਰਾ_ਖਾਨਾ + 1;

    // FIXME: This assertion is intended to catch unintended use of multibyte
    // characters in fixits. If we decide to do this, we'll have to track
    // separate byte widths for the source and fixit lines.
    ਪ੍ਰਤਿਜ੍ਞਾ((size_t)sys::locale::ਕਾਲਮ_ਚੌੜਾਈ(Fixit.get_ਪਾਠ()) ==
           Fixit.get_ਪਾਠ().ਮਾਪ_2());

    // This relies on one byte per column in our fixit hints.
    ਬਿਨਾਨਿਸ਼ਾਨੀ ਆਖਰੀ_ਕਾਲਮ_Modified = Hint_ਖਾਨਾ + Fixit.get_ਪਾਠ().ਮਾਪ_2();
    ਜੇ (ਆਖਰੀ_ਕਾਲਮ_Modified > Fix_It_ਸਤਰ.ਮਾਪ_2())
      Fix_It_ਸਤਰ.resize(ਆਖਰੀ_ਕਾਲਮ_Modified, ' ');

    llvm::copy(Fixit.get_ਪਾਠ(), Fix_It_ਸਤਰ.begin() + Hint_ਖਾਨਾ);

    Prev_Hint_ਸਿਰਾ_ਖਾਨਾ = ਆਖਰੀ_ਕਾਲਮ_Modified;

    // For replacements, mark the removal range with '~'.
    // FIXME: Handle multibyte characters in the source line.
    ਬਿਨਾਨਿਸ਼ਾਨੀ ਆਖਰੀ_ਖਾਨਾ;
    ਜੇ (R.ਸਿਰਾ_2.get_ਸੂਚਕ() >= ਸਤਰ_ਸਿਰਾ)
      ਆਖਰੀ_ਖਾਨਾ = ਸਤਰ_ਸਿਰਾ - ਸਤਰ_ਸ਼ੁਰੂ;
    ਨਹੀਂਤਾਂ
      ਆਖਰੀ_ਖਾਨਾ = R.ਸਿਰਾ_2.get_ਸੂਚਕ() - ਸਤਰ_ਸ਼ੁਰੂ;

    std::fill(&Caret_ਸਤਰ[ਪਹਿਲਾ_ਖਾਨਾ], &Caret_ਸਤਰ[ਆਖਰੀ_ਖਾਨਾ], '~');
  }
}

ਸਥਿਰ ਖਾਲੀ print_ਸਰੋਤ_ਸਤਰ(raw_ostream &S, StringRef ਸਤਰ_Contents) {
  // Print out the source line one character at a time, so we can expand tabs.
  ਲਈ (ਬਿਨਾਨਿਸ਼ਾਨੀ i = 0, e = ਸਤਰ_Contents.ਮਾਪ_2(), ਬਾਹਰਲਾ_ਖਾਨਾ = 0; i != e; ++i) {
    size_t ਅਗਲਾ_Tab = ਸਤਰ_Contents.ਲੱਭੋ('\t', i);
    // If there were no tabs left, print the rest, we are done.
    ਜੇ (ਅਗਲਾ_Tab == StringRef::npos) {
      S << ਸਤਰ_Contents.drop_front(i);
      ਰੋਕੋ;
    }

    // Otherwise, print from i to NextTab.
    S << ਸਤਰ_Contents.ਟੁਕੜਾ(i, ਅਗਲਾ_Tab);
    ਬਾਹਰਲਾ_ਖਾਨਾ += ਅਗਲਾ_Tab - i;
    i = ਅਗਲਾ_Tab;

    // If we have a tab, emit at least one space, then round up to 8 columns.
    ਕਰੋ {
      S << ' ';
      ++ਬਾਹਰਲਾ_ਖਾਨਾ;
    } ਜਦੋਂ ((ਬਾਹਰਲਾ_ਖਾਨਾ % TabStop) != 0);
  }
  S << '\n';
}

ਸਥਿਰ ਤਾਰਕਿਕ isNonASCII(ਅੱਖਰ c) { ਵਾਪਸ c & 0x80; }

ਖਾਲੀ SMDiagnostic::print(ਅਚਲ ਅੱਖਰ *ProgName, raw_ostream &OS, ਤਾਰਕਿਕ ShowColors,
                         ਤਾਰਕਿਕ Show_ਕਿਸਮ_Label, ਤਾਰਕਿਕ ShowLocation) ਅਚਲ {
  Color_ਬਹੁਲਕ ਬਹੁਲਕ = ShowColors ? Color_ਬਹੁਲਕ::Auto : Color_ਬਹੁਲਕ::Disable;

  {
    WithColor S(OS, raw_ostream::SAVEDCOLOR, ਸੱਚ, ਝੂਠ, ਬਹੁਲਕ);

    ਜੇ (ProgName && ProgName[0])
      S << ProgName << ": ";

    ਜੇ (ShowLocation && !Filename.ਖਾਲੀ_ਥਾਵਾਂ()) {
      ਜੇ (Filename == "-")
        S << "<stdin>";
      ਨਹੀਂਤਾਂ
        S << Filename;

      ਜੇ (ਸਤਰ_No != -1) {
        S << ':' << ਸਤਰ_No;
        ਜੇ (ਕਾਲਮ_No != -1)
          S << ':' << (ਕਾਲਮ_No + 1);
      }
      S << ": ";
    }
  }

  ਜੇ (Show_ਕਿਸਮ_Label) {
    ਚੋਣ (ਕਿਸਮ_3) {
    ਹਾਲਤ ਸਰੋਤ_Mgr::DK_Error:
      WithColor::error(OS, "", !ShowColors);
      ਰੋਕੋ;
    ਹਾਲਤ ਸਰੋਤ_Mgr::DK_Warning:
      WithColor::warning(OS, "", !ShowColors);
      ਰੋਕੋ;
    ਹਾਲਤ ਸਰੋਤ_Mgr::DK_Note:
      WithColor::note(OS, "", !ShowColors);
      ਰੋਕੋ;
    ਹਾਲਤ ਸਰੋਤ_Mgr::DK_Remark:
      WithColor::remark(OS, "", !ShowColors);
      ਰੋਕੋ;
    }
  }

  WithColor(OS, raw_ostream::SAVEDCOLOR, ਸੱਚ, ਝੂਠ, ਬਹੁਲਕ) << ਸੁਨੇਹਾ_3 << '\n';

  ਜੇ (ਸਤਰ_No == -1 || ਕਾਲਮ_No == -1)
    ਵਾਪਸ;

  // FIXME: If there are multibyte or multi-column characters in the source, all
  // our ranges will be wrong. To do this properly, we'll need a byte-to-column
  // map like Clang's TextDiagnostic. For now, we'll just handle tabs by
  // expanding them later, and bail out rather than show incorrect ranges and
  // misaligned fixits for any other odd characters.
  ਜੇ (any_ਦਾ(ਸਤਰ_Contents, isNonASCII)) {
    print_ਸਰੋਤ_ਸਤਰ(OS, ਸਤਰ_Contents);
    ਵਾਪਸ;
  }
  size_t ਅੰਕ_Columns = ਸਤਰ_Contents.ਮਾਪ_2();

  // Build the line with the caret and ranges.
  std::string Caret_ਸਤਰ(ਅੰਕ_Columns + 1, ' ');

  // Expand any ranges.
  ਲਈ (ਅਚਲ std::pair<ਬਿਨਾਨਿਸ਼ਾਨੀ, ਬਿਨਾਨਿਸ਼ਾਨੀ> &R : Ranges)
    std::fill(&Caret_ਸਤਰ[R.ਪਹਿਲਾ_2],
              &Caret_ਸਤਰ[std::min((size_t)R.ਦੂਜਾ_2, Caret_ਸਤਰ.ਮਾਪ_2())], '~');

  // Add any fix-its.
  // FIXME: Find the beginning of the line properly for multibyte characters.
  std::string Fix_It_Insertion_ਸਤਰ;
  build_Fix_It_ਸਤਰ(Caret_ਸਤਰ, Fix_It_Insertion_ਸਤਰ, FixIts,
                 ArrayRef(Loc.get_ਸੂਚਕ() - ਕਾਲਮ_No, ਸਤਰ_Contents.ਮਾਪ_2()));

  // Finally, plop on the caret.
  ਜੇ (ਬਿਨਾਨਿਸ਼ਾਨੀ(ਕਾਲਮ_No) <= ਅੰਕ_Columns)
    Caret_ਸਤਰ[ਕਾਲਮ_No] = '^';
  ਨਹੀਂਤਾਂ
    Caret_ਸਤਰ[ਅੰਕ_Columns] = '^';

  // ... and remove trailing whitespace so the output doesn't wrap for it.  We
  // know that the line isn't completely empty because it has the caret in it at
  // least.
  Caret_ਸਤਰ.erase(Caret_ਸਤਰ.ਲੱਭੋ_ਆਖਰੀ_not_ਦਾ(' ') + 1);

  print_ਸਰੋਤ_ਸਤਰ(OS, ਸਤਰ_Contents);

  {
    Color_ਬਹੁਲਕ ਬਹੁਲਕ = ShowColors ? Color_ਬਹੁਲਕ::Auto : Color_ਬਹੁਲਕ::Disable;
    WithColor S(OS, raw_ostream::GREEN, ਸੱਚ, ਝੂਠ, ਬਹੁਲਕ);

    // Print out the caret line, matching tabs in the source line.
    ਲਈ (ਬਿਨਾਨਿਸ਼ਾਨੀ i = 0, e = Caret_ਸਤਰ.ਮਾਪ_2(), ਬਾਹਰਲਾ_ਖਾਨਾ = 0; i != e; ++i) {
      ਜੇ (i >= ਸਤਰ_Contents.ਮਾਪ_2() || ਸਤਰ_Contents[i] != '\t') {
        S << Caret_ਸਤਰ[i];
        ++ਬਾਹਰਲਾ_ਖਾਨਾ;
        ਜਾਰੀ;
      }

      // Okay, we have a tab.  Insert the appropriate number of characters.
      ਕਰੋ {
        S << Caret_ਸਤਰ[i];
        ++ਬਾਹਰਲਾ_ਖਾਨਾ;
      } ਜਦੋਂ ((ਬਾਹਰਲਾ_ਖਾਨਾ % TabStop) != 0);
    }
    S << '\n';
  }

  // Print out the replacement line, matching tabs in the source line.
  ਜੇ (Fix_It_Insertion_ਸਤਰ.ਖਾਲੀ_ਥਾਵਾਂ())
    ਵਾਪਸ;

  ਲਈ (size_t i = 0, e = Fix_It_Insertion_ਸਤਰ.ਮਾਪ_2(), ਬਾਹਰਲਾ_ਖਾਨਾ = 0; i < e; ++i) {
    ਜੇ (i >= ਸਤਰ_Contents.ਮਾਪ_2() || ਸਤਰ_Contents[i] != '\t') {
      OS << Fix_It_Insertion_ਸਤਰ[i];
      ++ਬਾਹਰਲਾ_ਖਾਨਾ;
      ਜਾਰੀ;
    }

    // Okay, we have a tab.  Insert the appropriate number of characters.
    ਕਰੋ {
      OS << Fix_It_Insertion_ਸਤਰ[i];
      // FIXME: This is trying not to break up replacements, but then to re-sync
      // with the tabs between replacements. This will fail, though, if two
      // fix-it replacements are exactly adjacent, or if a fix-it contains a
      // space. Really we should be precomputing column widths, which we'll
      // need anyway for multibyte chars.
      ਜੇ (Fix_It_Insertion_ਸਤਰ[i] != ' ')
        ++i;
      ++ਬਾਹਰਲਾ_ਖਾਨਾ;
    } ਜਦੋਂ (((ਬਾਹਰਲਾ_ਖਾਨਾ % TabStop) != 0) && i != e);
  }
  OS << '\n';
}
