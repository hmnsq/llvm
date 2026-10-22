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

#ಅನ್ತರ್ಭಾವ "llvm/Support/SourceMgr.h"
#ಅನ್ತರ್ಭಾವ "llvm/ADT/ArrayRef.h"
#ಅನ್ತರ್ಭಾವ "llvm/ADT/STLExtras.h"
#ಅನ್ತರ್ಭಾವ "llvm/ADT/SmallString.h"
#ಅನ್ತರ್ಭಾವ "llvm/ADT/SmallVector.h"
#ಅನ್ತರ್ಭಾವ "llvm/ADT/StringRef.h"
#ಅನ್ತರ್ಭಾವ "llvm/ADT/Twine.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/ErrorOr.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/Locale.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/MemoryBuffer.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/Path.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/SMLoc.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/VirtualFileSystem.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/WithColor.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/raw_ostream.h"
#ಅನ್ತರ್ಭಾವ <algorithm>
#ಅನ್ತರ್ಭಾವ <cassert>
#ಅನ್ತರ್ಭಾವ <cstddef>
#ಅನ್ತರ್ಭಾವ <limits>
#ಅನ್ತರ್ಭಾವ <memory>
#ಅನ್ತರ್ಭಾವ <string>
#ಅನ್ತರ್ಭಾವ <utility>

ಬಳಸಿ ನಾಮಾಕಾಶ llvm;

ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ size_t TabStop = 8;

// Out of line to avoid needing definition of vfs::FileSystem in header.
ಮೂಲ_Mgr::ಮೂಲ_Mgr() = ಪೂರ್ವನಿಯೋಜಿತ;
ಮೂಲ_Mgr::ಮೂಲ_Mgr(IntrusiveRefCntPtr<vfs::FileSystem> FS)
    : FS(std::move(FS)) {}
ಮೂಲ_Mgr::ಮೂಲ_Mgr(ಮೂಲ_Mgr &&) = ಪೂರ್ವನಿಯೋಜಿತ;
ಮೂಲ_Mgr &ಮೂಲ_Mgr::ಕ್ರಿಯಾಕಾರಕ=(ಮೂಲ_Mgr &&) = ಪೂರ್ವನಿಯೋಜಿತ;
ಮೂಲ_Mgr::~ಮೂಲ_Mgr() = ಪೂರ್ವನಿಯೋಜಿತ;

IntrusiveRefCntPtr<vfs::FileSystem> ಮೂಲ_Mgr::getVirtualFileSystem() ಸ್ಥಿರಾಂಕ {
  ಹಿಂತಿರುಗಿ FS;
}

ಶೂನ್ಯ ಮೂಲ_Mgr::ಗಣ_Virtual_File_System(IntrusiveRefCntPtr<vfs::FileSystem> FS) {
  ಇದು->FS = std::move(FS);
}

ಚಿಹ್ನೆರಹಿತ ಮೂಲ_Mgr::ಕೂಡಿಸು_Include_File(ಸ್ಥಿರಾಂಕ std::string &Filename,
                                   SMLoc IncludeLoc,
                                   std::string &IncludedFile) {
  ErrorOr<std::unique_ptr<ಸ್ಮರಣೆ_ಬಫರ್>> NewBufOrErr =
      OpenIncludeFile(Filename, IncludedFile);
  ಆದರೆ (!NewBufOrErr)
    ಹಿಂತಿರುಗಿ 0;

  ಹಿಂತಿರುಗಿ ಕೂಡಿಸು_New_ಮೂಲ_ಬಫರ್(std::move(*NewBufOrErr), IncludeLoc);
}

ErrorOr<std::unique_ptr<ಸ್ಮರಣೆ_ಬಫರ್>>
ಮೂಲ_Mgr::OpenIncludeFile(ಸ್ಥಿರಾಂಕ std::string &Filename,
                           std::string &IncludedFile,
                           ತಾರ್ಕಿಕ RequiresNullTerminator) {
  ಸ್ವಯಂಚಾಲಿತ GetFile = [ಇದು, RequiresNullTerminator](StringRef ಮಾರ್ಗ) {
    ಹಿಂತಿರುಗಿ FS ? FS->get_ಬಫರ್_For_File(ಮಾರ್ಗ, /*FileSize=*/-1,
                                     RequiresNullTerminator)
              : ಸ್ಮರಣೆ_ಬಫರ್::getFile(ಮಾರ್ಗ, /*IsText=*/ಸುಳ್ಳು,
                                      RequiresNullTerminator);
  };

  ErrorOr<std::unique_ptr<ಸ್ಮರಣೆ_ಬಫರ್>> NewBufOrErr = GetFile(Filename);

  SmallString<64> ಬಫರ್(Filename);
  // If the file didn't exist directly, see if it's in an include path.
  ಗಾಗಿ (ಚಿಹ್ನೆರಹಿತ i = 0, e = IncludeDirectories.ಗಾತ್ರ_3(); i != e && !NewBufOrErr;
       ++i) {
    ಬಫರ್ = IncludeDirectories[i];
    sys::ಮಾರ್ಗ_2::append(ಬಫರ್, Filename);
    NewBufOrErr = GetFile(ಬಫರ್);
  }

  ಆದರೆ (NewBufOrErr)
    IncludedFile = static_cast<std::string>(ಬಫರ್);

  ಹಿಂತಿರುಗಿ NewBufOrErr;
}

ಚಿಹ್ನೆರಹಿತ ಮೂಲ_Mgr::ಕಂಡುಹಿಡಿ_ಬಫರ್_Containing_Loc(SMLoc Loc) ಸ್ಥಿರಾಂಕ {
  ಗಾಗಿ (ಚಿಹ್ನೆರಹಿತ i = 0, e = Buffers.ಗಾತ್ರ_3(); i != e; ++i)
    ಆದರೆ (Loc.get_ಸೂಚಕ() >= Buffers[i].ಬಫರ್->get_ಬಫರ್_ಆರಂಭ() &&
        // Use <= here so that a pointer to the null at the end of the buffer
        // is included as part of the buffer.
        Loc.get_ಸೂಚಕ() <= Buffers[i].ಬಫರ್->get_ಬಫರ್_ಕೊನೆ())
      ಹಿಂತಿರುಗಿ i + 1;
  ಹಿಂತಿರುಗಿ 0;
}

ಮಾದರಿ <ಬಗೆ_ಹೆಸರು T>
ಸ್ಥಿರ std::ಸದಿಶ<T> &Get_Or_Create_ಸ್ಥಾನಾಂತರ_Cache(ಶೂನ್ಯ *&ಸ್ಥಾನಾಂತರ_Cache,
                                              ಸ್ಮರಣೆ_ಬಫರ್ *ಬಫರ್) {
  ಆದರೆ (ಸ್ಥಾನಾಂತರ_Cache)
    ಹಿಂತಿರುಗಿ *static_cast<std::ಸದಿಶ<T> *>(ಸ್ಥಾನಾಂತರ_Cache);

  // Lazily fill in the offset cache.
  ಸ್ವಯಂಚಾಲಿತ *Offsets = ಹೊಸ std::ಸದಿಶ<T>();
  size_t Sz = ಬಫರ್->get_ಬಫರ್_ಗಾತ್ರ();
  ಪ್ರತಿಜ್ಞಾ(Sz <= std::numeric_limits<T>::max());
  StringRef S = ಬಫರ್->get_ಬಫರ್();
  ಗಾಗಿ (size_t N = 0; N < Sz; ++N) {
    ಆದರೆ (S[N] == '\n')
      Offsets->ತಳ್ಳು_ಹಿಂಭಾಗ(static_cast<T>(N));
  }

  ಸ್ಥಾನಾಂತರ_Cache = Offsets;
  ಹಿಂತಿರುಗಿ *Offsets;
}

ಮಾದರಿ <ಬಗೆ_ಹೆಸರು T>
std::pair<ಚಿಹ್ನೆರಹಿತ, ಚಿಹ್ನೆರಹಿತ>
ಮೂಲ_Mgr::Src_ಬಫರ್::get_ಸಾಲು_And_ಸ್ತಂಭ_Specialized(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *Ptr) ಸ್ಥಿರಾಂಕ {
  std::ಸದಿಶ<T> &Offsets =
      Get_Or_Create_ಸ್ಥಾನಾಂತರ_Cache<T>(ಸ್ಥಾನಾಂತರ_Cache, ಬಫರ್.get());

  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *Buf_ಆರಂಭ = ಬಫರ್->get_ಬಫರ್_ಆರಂಭ();
  ಪ್ರತಿಜ್ಞಾ(Ptr >= Buf_ಆರಂಭ && Ptr <= ಬಫರ್->get_ಬಫರ್_ಕೊನೆ());
  ptrdiff_t PtrDiff = Ptr - Buf_ಆರಂಭ;
  ಪ್ರತಿಜ್ಞಾ(PtrDiff >= 0 &&
         static_cast<size_t>(PtrDiff) <= std::numeric_limits<T>::max());
  T Ptr_ಸ್ಥಾನಾಂತರ = static_cast<T>(PtrDiff);

  // llvm::lower_bound gives the number of EOL before PtrOffset. Add 1 to get
  // the line number.
  ಸ್ವಯಂಚಾಲಿತ I = llvm::ಅಧೋ_bound(Offsets, Ptr_ಸ್ಥಾನಾಂತರ);
  ಚಿಹ್ನೆರಹಿತ ಸಾಲು_No = I - Offsets.begin() + 1;

  // The column number is the distance from the previous EOL (or the start of
  // the buffer) to the pointer.
  T ಸಾಲು_ಆರಂಭ_Offs = (I == Offsets.begin()) ? 0 : (I[-1] + 1);
  ಚಿಹ್ನೆರಹಿತ ಲಂಬಸಾಲು_No = Ptr_ಸ್ಥಾನಾಂತರ - ಸಾಲು_ಆರಂಭ_Offs + 1;
  ಹಿಂತಿರುಗಿ {ಸಾಲು_No, ಲಂಬಸಾಲು_No};
}

/// Look up a given \p Ptr in the buffer, determining which line and column
/// it came from.
LLVM_ABI std::pair<ಚಿಹ್ನೆರಹಿತ, ಚಿಹ್ನೆರಹಿತ>
ಮೂಲ_Mgr::Src_ಬಫರ್::get_ಸಾಲು_And_ಸ್ತಂಭ(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *Ptr) ಸ್ಥಿರಾಂಕ {
  size_t Sz = ಬಫರ್->get_ಬಫರ್_ಗಾತ್ರ();
  ಆದರೆ (Sz <= std::numeric_limits<uint8_t>::max())
    ಹಿಂತಿರುಗಿ get_ಸಾಲು_And_ಸ್ತಂಭ_Specialized<uint8_t>(Ptr);
  ಆದರೆ (Sz <= std::numeric_limits<uint16_t>::max())
    ಹಿಂತಿರುಗಿ get_ಸಾಲು_And_ಸ್ತಂಭ_Specialized<uint16_t>(Ptr);
  ಆದರೆ (Sz <= std::numeric_limits<uint32_t>::max())
    ಹಿಂತಿರುಗಿ get_ಸಾಲು_And_ಸ್ತಂಭ_Specialized<uint32_t>(Ptr);
  ಹಿಂತಿರುಗಿ get_ಸಾಲು_And_ಸ್ತಂಭ_Specialized<uint64_t>(Ptr);
}

ಮಾದರಿ <ಬಗೆ_ಹೆಸರು T>
ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಮೂಲ_Mgr::Src_ಬಫರ್::get_ಸೂಚಕ_For_ಸಾಲು_ಸಂಖ್ಯೆ_Specialized(
    ಚಿಹ್ನೆರಹಿತ ಸಾಲು_No) ಸ್ಥಿರಾಂಕ {
  std::ಸದಿಶ<T> &Offsets =
      Get_Or_Create_ಸ್ಥಾನಾಂತರ_Cache<T>(ಸ್ಥಾನಾಂತರ_Cache, ಬಫರ್.get());

  // We start counting line and column numbers from 1.
  ಆದರೆ (ಸಾಲು_No != 0)
    --ಸಾಲು_No;

  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *Buf_ಆರಂಭ = ಬಫರ್->get_ಬಫರ್_ಆರಂಭ();

  // The offset cache contains the location of the \n for the specified line,
  // we want the start of the line.  As such, we look for the previous entry.
  ಆದರೆ (ಸಾಲು_No == 0)
    ಹಿಂತಿರುಗಿ Buf_ಆರಂಭ;
  ಆದರೆ (ಸಾಲು_No > Offsets.ಗಾತ್ರ_3())
    ಹಿಂತಿರುಗಿ ಶೂನ್ಯಸೂಚಕ;
  ಹಿಂತಿರುಗಿ Buf_ಆರಂಭ + Offsets[ಸಾಲು_No - 1] + 1;
}

/// Return a pointer to the first character of the specified line number or
/// null if the line number is invalid.
ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *
ಮೂಲ_Mgr::Src_ಬಫರ್::get_ಸೂಚಕ_For_ಸಾಲು_ಸಂಖ್ಯೆ(ಚಿಹ್ನೆರಹಿತ ಸಾಲು_No) ಸ್ಥಿರಾಂಕ {
  size_t Sz = ಬಫರ್->get_ಬಫರ್_ಗಾತ್ರ();
  ಆದರೆ (Sz <= std::numeric_limits<uint8_t>::max())
    ಹಿಂತಿರುಗಿ get_ಸೂಚಕ_For_ಸಾಲು_ಸಂಖ್ಯೆ_Specialized<uint8_t>(ಸಾಲು_No);
  ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (Sz <= std::numeric_limits<uint16_t>::max())
    ಹಿಂತಿರುಗಿ get_ಸೂಚಕ_For_ಸಾಲು_ಸಂಖ್ಯೆ_Specialized<uint16_t>(ಸಾಲು_No);
  ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (Sz <= std::numeric_limits<uint32_t>::max())
    ಹಿಂತಿರುಗಿ get_ಸೂಚಕ_For_ಸಾಲು_ಸಂಖ್ಯೆ_Specialized<uint32_t>(ಸಾಲು_No);
  ಇಲ್ಲದಿದ್ದರೆ
    ಹಿಂತಿರುಗಿ get_ಸೂಚಕ_For_ಸಾಲು_ಸಂಖ್ಯೆ_Specialized<uint64_t>(ಸಾಲು_No);
}

ಮೂಲ_Mgr::Src_ಬಫರ್::Src_ಬಫರ್(ಮೂಲ_Mgr::Src_ಬಫರ್ &&Other)
    : ಬಫರ್(std::move(Other.ಬಫರ್)), ಸ್ಥಾನಾಂತರ_Cache(Other.ಸ್ಥಾನಾಂತರ_Cache),
      IncludeLoc(Other.IncludeLoc) {
  Other.ಸ್ಥಾನಾಂತರ_Cache = ಶೂನ್ಯಸೂಚಕ;
}

ಮೂಲ_Mgr::Src_ಬಫರ್::~Src_ಬಫರ್() {
  ಆದರೆ (ಸ್ಥಾನಾಂತರ_Cache) {
    size_t Sz = ಬಫರ್->get_ಬಫರ್_ಗಾತ್ರ();
    ಆದರೆ (Sz <= std::numeric_limits<uint8_t>::max())
      ಅಳಿಸು static_cast<std::ಸದಿಶ<uint8_t> *>(ಸ್ಥಾನಾಂತರ_Cache);
    ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (Sz <= std::numeric_limits<uint16_t>::max())
      ಅಳಿಸು static_cast<std::ಸದಿಶ<uint16_t> *>(ಸ್ಥಾನಾಂತರ_Cache);
    ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (Sz <= std::numeric_limits<uint32_t>::max())
      ಅಳಿಸು static_cast<std::ಸದಿಶ<uint32_t> *>(ಸ್ಥಾನಾಂತರ_Cache);
    ಇಲ್ಲದಿದ್ದರೆ
      ಅಳಿಸು static_cast<std::ಸದಿಶ<uint64_t> *>(ಸ್ಥಾನಾಂತರ_Cache);
    ಸ್ಥಾನಾಂತರ_Cache = ಶೂನ್ಯಸೂಚಕ;
  }
}

std::pair<ಚಿಹ್ನೆರಹಿತ, ಚಿಹ್ನೆರಹಿತ>
ಮೂಲ_Mgr::get_ಸಾಲು_And_ಸ್ತಂಭ(SMLoc Loc, ಚಿಹ್ನೆರಹಿತ ಬಫರ್_ID) ಸ್ಥಿರಾಂಕ {
  ಆದರೆ (!ಬಫರ್_ID)
    ಬಫರ್_ID = ಕಂಡುಹಿಡಿ_ಬಫರ್_Containing_Loc(Loc);
  ಪ್ರತಿಜ್ಞಾ(ಬಫರ್_ID && "Invalid location!");

  ಸ್ವಯಂಚಾಲಿತ &SB = get_ಬಫರ್_Info(ಬಫರ್_ID);
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *Ptr = Loc.get_ಸೂಚಕ();

  ಹಿಂತಿರುಗಿ SB.get_ಸಾಲು_And_ಸ್ತಂಭ(Ptr);
}

// FIXME: Note that the formatting of source locations is spread between
// multiple functions, some in SourceMgr and some in SMDiagnostic. A better
// solution would be a general-purpose source location formatter
// in one of those two classes, or possibly in SMLoc.

/// Get a string with the source location formatted in the standard
/// style, but without the line offset. If \p IncludePath is true, the path
/// is included. If false, only the file name and extension are included.
std::string ಮೂಲ_Mgr::get_Formatted_Location_No_ಸ್ಥಾನಾಂತರ(SMLoc Loc,
                                                    ತಾರ್ಕಿಕ Include_ಮಾರ್ಗ) ಸ್ಥಿರಾಂಕ {
  ಸ್ವಯಂಚಾಲಿತ ಬಫರ್_ID = ಕಂಡುಹಿಡಿ_ಬಫರ್_Containing_Loc(Loc);
  ಪ್ರತಿಜ್ಞಾ(ಬಫರ್_ID && "Invalid location!");
  ಸ್ವಯಂಚಾಲಿತ FileSpec = get_ಬಫರ್_Info(ಬಫರ್_ID).ಬಫರ್->get_ಬಫರ್_Identifier();

  ಆದರೆ (Include_ಮಾರ್ಗ) {
    ಹಿಂತಿರುಗಿ FileSpec.str() + ":" + std::ಗಮ್ಯ_string(ಕಂಡುಹಿಡಿ_ಸಾಲು_ಸಂಖ್ಯೆ(Loc, ಬಫರ್_ID));
  } ಇಲ್ಲದಿದ್ದರೆ {
    ಸ್ವಯಂಚಾಲಿತ I = FileSpec.ಕಂಡುಹಿಡಿ_ಕೊನೆಯ_ದ("/\\");
    I = (I == FileSpec.ಗಾತ್ರ_3()) ? 0 : (I + 1);
    ಹಿಂತಿರುಗಿ FileSpec.substr(I).str() + ":" +
           std::ಗಮ್ಯ_string(ಕಂಡುಹಿಡಿ_ಸಾಲು_ಸಂಖ್ಯೆ(Loc, ಬಫರ್_ID));
  }
}

/// Given a line and column number in a mapped buffer, turn it into an SMLoc.
/// This will return a null SMLoc if the line/column location is invalid.
SMLoc ಮೂಲ_Mgr::ಕಂಡುಹಿಡಿ_Loc_For_ಸಾಲು_And_ಸ್ತಂಭ(ಚಿಹ್ನೆರಹಿತ ಬಫರ್_ID, ಚಿಹ್ನೆರಹಿತ ಸಾಲು_No,
                                         ಚಿಹ್ನೆರಹಿತ ಲಂಬಸಾಲು_No) {
  ಸ್ವಯಂಚಾಲಿತ &SB = get_ಬಫರ್_Info(ಬಫರ್_ID);
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *Ptr = SB.get_ಸೂಚಕ_For_ಸಾಲು_ಸಂಖ್ಯೆ(ಸಾಲು_No);
  ಆದರೆ (!Ptr)
    ಹಿಂತಿರುಗಿ SMLoc();

  // We start counting line and column numbers from 1.
  ಆದರೆ (ಲಂಬಸಾಲು_No != 0)
    --ಲಂಬಸಾಲು_No;

  // If we have a column number, validate it.
  ಆದರೆ (ಲಂಬಸಾಲು_No) {
    // Make sure the location is within the current line.
    ಆದರೆ (Ptr + ಲಂಬಸಾಲು_No > SB.ಬಫರ್->get_ಬಫರ್_ಕೊನೆ())
      ಹಿಂತಿರುಗಿ SMLoc();

    // Make sure there is no newline in the way.
    ಆದರೆ (StringRef(Ptr, ಲಂಬಸಾಲು_No).ಕಂಡುಹಿಡಿ_ಮೊದಲ_ದ("\n\r") != StringRef::npos)
      ಹಿಂತಿರುಗಿ SMLoc();

    Ptr += ಲಂಬಸಾಲು_No;
  }

  ಹಿಂತಿರುಗಿ SMLoc::get_From_ಸೂಚಕ(Ptr);
}

ಶೂನ್ಯ ಮೂಲ_Mgr::Print_Include_ರಾಶಿ(SMLoc IncludeLoc, raw_ostream &OS) ಸ್ಥಿರಾಂಕ {
  ಆದರೆ (IncludeLoc == SMLoc())
    ಹಿಂತಿರುಗಿ; // Top of stack.

  ಚಿಹ್ನೆರಹಿತ CurBuf = ಕಂಡುಹಿಡಿ_ಬಫರ್_Containing_Loc(IncludeLoc);
  ಪ್ರತಿಜ್ಞಾ(CurBuf && "Invalid or unspecified location!");

  Print_Include_ರಾಶಿ(get_ಬಫರ್_Info(CurBuf).IncludeLoc, OS);

  OS << "Included from " << get_ಬಫರ್_Info(CurBuf).ಬಫರ್->get_ಬಫರ್_Identifier()
     << ":" << ಕಂಡುಹಿಡಿ_ಸಾಲು_ಸಂಖ್ಯೆ(IncludeLoc, CurBuf) << ":\n";
}

ಶೂನ್ಯ ಮೂಲ_Mgr::print_Include_ರಾಶಿ_For_Diagnostic(SMLoc Loc,
                                               raw_ostream &OS) ಸ್ಥಿರಾಂಕ {
  ಆದರೆ (!Loc.isValid())
    ಹಿಂತಿರುಗಿ;
  ಚಿಹ್ನೆರಹಿತ Diag_Cur_ಬಫರ್ = ಕಂಡುಹಿಡಿ_ಬಫರ್_Containing_Loc(Loc);
  ಆದರೆ (Diag_Cur_ಬಫರ್ && Diag_Cur_ಬಫರ್ != getMainFileID()) {
    SMLoc ಪೋಷಕ_Include_Loc = get_ಪೋಷಕ_Include_Loc(Diag_Cur_ಬಫರ್);
    // Ignore macro instantiation buffers to avoid redundant include stacks.
    Print_Include_ರಾಶಿ(ಪೋಷಕ_Include_Loc, OS);
  }
}

SMDiagnostic ಮೂಲ_Mgr::Get_ಸಂದೇಶ(SMLoc Loc, ಮೂಲ_Mgr::Diag_ಬಗೆ ಬಗೆ_2,
                                   ಸ್ಥಿರಾಂಕ Twine &ಸಂದೇಶ_4, ArrayRef<SMRange> Ranges,
                                   ArrayRef<SMFixIt> FixIts) ಸ್ಥಿರಾಂಕ {
  // First thing to do: find the current buffer containing the specified
  // location to pull out the source line.
  Small_ಸದಿಶ<std::pair<ಚಿಹ್ನೆರಹಿತ, ಚಿಹ್ನೆರಹಿತ>, 4> ಲಂಬಸಾಲು_Ranges;
  std::pair<ಚಿಹ್ನೆರಹಿತ, ಚಿಹ್ನೆರಹಿತ> ಸಾಲು_And_ಲಂಬಸಾಲು;
  StringRef ಬಫರ್_ID = "<unknown>";
  StringRef ಸಾಲು_Str;

  ಆದರೆ (Loc.isValid()) {
    ಚಿಹ್ನೆರಹಿತ CurBuf = ಕಂಡುಹಿಡಿ_ಬಫರ್_Containing_Loc(Loc);
    ಪ್ರತಿಜ್ಞಾ(CurBuf && "Invalid or unspecified location!");

    ಸ್ಥಿರಾಂಕ ಸ್ಮರಣೆ_ಬಫರ್ *CurMB = get_ಸ್ಮರಣೆ_ಬಫರ್(CurBuf);
    ಬಫರ್_ID = CurMB->get_ಬಫರ್_Identifier();

    // Scan backward to find the start of the line.
    ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಸಾಲು_ಆರಂಭ = Loc.get_ಸೂಚಕ();
    ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *Buf_ಆರಂಭ = CurMB->get_ಬಫರ್_ಆರಂಭ();
    ಆಗ (ಸಾಲು_ಆರಂಭ != Buf_ಆರಂಭ && ಸಾಲು_ಆರಂಭ[-1] != '\n' &&
           ಸಾಲು_ಆರಂಭ[-1] != '\r')
      --ಸಾಲು_ಆರಂಭ;

    // Get the end of the line.
    ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಸಾಲು_ಕೊನೆ = Loc.get_ಸೂಚಕ();
    ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *Buf_ಕೊನೆ = CurMB->get_ಬಫರ್_ಕೊನೆ();
    ಆಗ (ಸಾಲು_ಕೊನೆ != Buf_ಕೊನೆ && ಸಾಲು_ಕೊನೆ[0] != '\n' && ಸಾಲು_ಕೊನೆ[0] != '\r')
      ++ಸಾಲು_ಕೊನೆ;
    ಸಾಲು_Str = StringRef(ಸಾಲು_ಆರಂಭ, ಸಾಲು_ಕೊನೆ - ಸಾಲು_ಆರಂಭ);

    // Convert any ranges to column ranges that only intersect the line of the
    // location.
    ಗಾಗಿ (SMRange R : Ranges) {
      ಆದರೆ (!R.isValid())
        ಮುಂದುವರಿ;

      // If the line doesn't contain any part of the range, then ignore it.
      ಆದರೆ (R.ಆರಂಭ.get_ಸೂಚಕ() > ಸಾಲು_ಕೊನೆ || R.ಕೊನೆ_2.get_ಸೂಚಕ() < ಸಾಲು_ಆರಂಭ)
        ಮುಂದುವರಿ;

      // Ignore pieces of the range that go onto other lines.
      ಆದರೆ (R.ಆರಂಭ.get_ಸೂಚಕ() < ಸಾಲು_ಆರಂಭ)
        R.ಆರಂಭ = SMLoc::get_From_ಸೂಚಕ(ಸಾಲು_ಆರಂಭ);
      ಆದರೆ (R.ಕೊನೆ_2.get_ಸೂಚಕ() > ಸಾಲು_ಕೊನೆ)
        R.ಕೊನೆ_2 = SMLoc::get_From_ಸೂಚಕ(ಸಾಲು_ಕೊನೆ);

      // Translate from SMLoc ranges to column ranges.
      // FIXME: Handle multibyte characters.
      ಲಂಬಸಾಲು_Ranges.ತಳ್ಳು_ಹಿಂಭಾಗ(std::make_pair(R.ಆರಂಭ.get_ಸೂಚಕ() - ಸಾಲು_ಆರಂಭ,
                                         R.ಕೊನೆ_2.get_ಸೂಚಕ() - ಸಾಲು_ಆರಂಭ));
    }

    ಸಾಲು_And_ಲಂಬಸಾಲು = get_ಸಾಲು_And_ಸ್ತಂಭ(Loc, CurBuf);
  }

  ಹಿಂತಿರುಗಿ SMDiagnostic(*ಇದು, Loc, ಬಫರ್_ID, ಸಾಲು_And_ಲಂಬಸಾಲು.ಮೊದಲ_2,
                      ಸಾಲು_And_ಲಂಬಸಾಲು.ಎರಡನೇ_2 - 1, ಬಗೆ_2, ಸಂದೇಶ_4.str(), ಸಾಲು_Str,
                      ಲಂಬಸಾಲು_Ranges, FixIts);
}

ಶೂನ್ಯ ಮೂಲ_Mgr::Print_ಸಂದೇಶ(raw_ostream &OS, ಸ್ಥಿರಾಂಕ SMDiagnostic &Diagnostic,
                             ತಾರ್ಕಿಕ ShowColors) ಸ್ಥಿರಾಂಕ {
  // Report the message with the diagnostic handler if present.
  ಆದರೆ (DiagHandler) {
    DiagHandler(Diagnostic, DiagContext);
    ಹಿಂತಿರುಗಿ;
  }

  ಆದರೆ (Diagnostic.getLoc().isValid()) {
    ಚಿಹ್ನೆರಹಿತ CurBuf = ಕಂಡುಹಿಡಿ_ಬಫರ್_Containing_Loc(Diagnostic.getLoc());
    ಪ್ರತಿಜ್ಞಾ(CurBuf && "Invalid or unspecified location!");
    Print_Include_ರಾಶಿ(get_ಬಫರ್_Info(CurBuf).IncludeLoc, OS);
  }

  Diagnostic.print(ಶೂನ್ಯಸೂಚಕ, OS, ShowColors);
}

ಶೂನ್ಯ ಮೂಲ_Mgr::Print_ಸಂದೇಶ(raw_ostream &OS, SMLoc Loc,
                             ಮೂಲ_Mgr::Diag_ಬಗೆ ಬಗೆ_2, ಸ್ಥಿರಾಂಕ Twine &ಸಂದೇಶ_4,
                             ArrayRef<SMRange> Ranges, ArrayRef<SMFixIt> FixIts,
                             ತಾರ್ಕಿಕ ShowColors) ಸ್ಥಿರಾಂಕ {
  Print_ಸಂದೇಶ(OS, Get_ಸಂದೇಶ(Loc, ಬಗೆ_2, ಸಂದೇಶ_4, Ranges, FixIts), ShowColors);
}

ಶೂನ್ಯ ಮೂಲ_Mgr::Print_ಸಂದೇಶ(SMLoc Loc, ಮೂಲ_Mgr::Diag_ಬಗೆ ಬಗೆ_2,
                             ಸ್ಥಿರಾಂಕ Twine &ಸಂದೇಶ_4, ArrayRef<SMRange> Ranges,
                             ArrayRef<SMFixIt> FixIts, ತಾರ್ಕಿಕ ShowColors) ಸ್ಥಿರಾಂಕ {
  Print_ಸಂದೇಶ(errs(), Loc, ಬಗೆ_2, ಸಂದೇಶ_4, Ranges, FixIts, ShowColors);
}

//===----------------------------------------------------------------------===//
// SMFixIt Implementation
//===----------------------------------------------------------------------===//

SMFixIt::SMFixIt(SMRange R, ಸ್ಥಿರಾಂಕ Twine &Replacement)
    : Range(R), ಪಠ್ಯ_2(Replacement.str()) {
  ಪ್ರತಿಜ್ಞಾ(R.isValid());
}

//===----------------------------------------------------------------------===//
// SMDiagnostic Implementation
//===----------------------------------------------------------------------===//

SMDiagnostic::SMDiagnostic(ಸ್ಥಿರಾಂಕ ಮೂಲ_Mgr &sm, SMLoc L, StringRef FN, ಪೂರ್ಣಾಂಕ ಸಾಲು,
                           ಪೂರ್ಣಾಂಕ ಲಂಬಸಾಲು, ಮೂಲ_Mgr::Diag_ಬಗೆ ಬಗೆ_2, StringRef ಸಂದೇಶ_4,
                           StringRef ಸಾಲು_Str,
                           ArrayRef<std::pair<ಚಿಹ್ನೆರಹಿತ, ಚಿಹ್ನೆರಹಿತ>> Ranges,
                           ArrayRef<SMFixIt> Hints)
    : SM(&sm), Loc(L), Filename(std::string(FN)), ಸಾಲು_No(ಸಾಲು), ಸ್ತಂಭ_No(ಲಂಬಸಾಲು),
      ಬಗೆ_2(ಬಗೆ_2), ಸಂದೇಶ_3(ಸಂದೇಶ_4), ಸಾಲು_Contents(ಸಾಲು_Str), Ranges(Ranges.vec()),
      FixIts(Hints) {
  llvm::ವಿಂಗಡಿಸು(FixIts);
}

ಸ್ಥಿರ ಶೂನ್ಯ build_Fix_It_ಸಾಲು(std::string &Caret_ಸಾಲು, std::string &Fix_It_ಸಾಲು,
                           ArrayRef<SMFixIt> FixIts,
                           ArrayRef<ಅಕ್ಷರ> ಮೂಲ_ಸಾಲು) {
  ಆದರೆ (FixIts.ಖಾಲಿಗಳು())
    ಹಿಂತಿರುಗಿ;

  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಸಾಲು_ಆರಂಭ = ಮೂಲ_ಸಾಲು.begin();
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಸಾಲು_ಕೊನೆ = ಮೂಲ_ಸಾಲು.ಕೊನೆ();

  size_t Prev_Hint_ಕೊನೆ_ಲಂಬಸಾಲು = 0;

  ಗಾಗಿ (ಸ್ಥಿರಾಂಕ llvm::SMFixIt &Fixit : FixIts) {
    // If the fixit contains a newline or tab, ignore it.
    ಆದರೆ (Fixit.get_ಪಠ್ಯ().ಕಂಡುಹಿಡಿ_ಮೊದಲ_ದ("\n\r\t") != StringRef::npos)
      ಮುಂದುವರಿ;

    SMRange R = Fixit.getRange();

    // If the line doesn't contain any part of the range, then ignore it.
    ಆದರೆ (R.ಆರಂಭ.get_ಸೂಚಕ() > ಸಾಲು_ಕೊನೆ || R.ಕೊನೆ_2.get_ಸೂಚಕ() < ಸಾಲು_ಆರಂಭ)
      ಮುಂದುವರಿ;

    // Translate from SMLoc to column.
    // Ignore pieces of the range that go onto other lines.
    // FIXME: Handle multibyte characters in the source line.
    ಚಿಹ್ನೆರಹಿತ ಮೊದಲ_ಲಂಬಸಾಲು;
    ಆದರೆ (R.ಆರಂಭ.get_ಸೂಚಕ() < ಸಾಲು_ಆರಂಭ)
      ಮೊದಲ_ಲಂಬಸಾಲು = 0;
    ಇಲ್ಲದಿದ್ದರೆ
      ಮೊದಲ_ಲಂಬಸಾಲು = R.ಆರಂಭ.get_ಸೂಚಕ() - ಸಾಲು_ಆರಂಭ;

    // If we inserted a long previous hint, push this one forwards, and add
    // an extra space to show that this is not part of the previous
    // completion. This is sort of the best we can do when two hints appear
    // to overlap.
    //
    // Note that if this hint is located immediately after the previous
    // hint, no space will be added, since the location is more important.
    ಚಿಹ್ನೆರಹಿತ Hint_ಲಂಬಸಾಲು = ಮೊದಲ_ಲಂಬಸಾಲು;
    ಆದರೆ (Hint_ಲಂಬಸಾಲು < Prev_Hint_ಕೊನೆ_ಲಂಬಸಾಲು)
      Hint_ಲಂಬಸಾಲು = Prev_Hint_ಕೊನೆ_ಲಂಬಸಾಲು + 1;

    // FIXME: This assertion is intended to catch unintended use of multibyte
    // characters in fixits. If we decide to do this, we'll have to track
    // separate byte widths for the source and fixit lines.
    ಪ್ರತಿಜ್ಞಾ((size_t)sys::locale::ಸ್ತಂಭ_ಅಗಲ(Fixit.get_ಪಠ್ಯ()) ==
           Fixit.get_ಪಠ್ಯ().ಗಾತ್ರ_3());

    // This relies on one byte per column in our fixit hints.
    ಚಿಹ್ನೆರಹಿತ ಕೊನೆಯ_ಸ್ತಂಭ_Modified = Hint_ಲಂಬಸಾಲು + Fixit.get_ಪಠ್ಯ().ಗಾತ್ರ_3();
    ಆದರೆ (ಕೊನೆಯ_ಸ್ತಂಭ_Modified > Fix_It_ಸಾಲು.ಗಾತ್ರ_3())
      Fix_It_ಸಾಲು.resize(ಕೊನೆಯ_ಸ್ತಂಭ_Modified, ' ');

    llvm::copy(Fixit.get_ಪಠ್ಯ(), Fix_It_ಸಾಲು.begin() + Hint_ಲಂಬಸಾಲು);

    Prev_Hint_ಕೊನೆ_ಲಂಬಸಾಲು = ಕೊನೆಯ_ಸ್ತಂಭ_Modified;

    // For replacements, mark the removal range with '~'.
    // FIXME: Handle multibyte characters in the source line.
    ಚಿಹ್ನೆರಹಿತ ಕೊನೆಯ_ಲಂಬಸಾಲು;
    ಆದರೆ (R.ಕೊನೆ_2.get_ಸೂಚಕ() >= ಸಾಲು_ಕೊನೆ)
      ಕೊನೆಯ_ಲಂಬಸಾಲು = ಸಾಲು_ಕೊನೆ - ಸಾಲು_ಆರಂಭ;
    ಇಲ್ಲದಿದ್ದರೆ
      ಕೊನೆಯ_ಲಂಬಸಾಲು = R.ಕೊನೆ_2.get_ಸೂಚಕ() - ಸಾಲು_ಆರಂಭ;

    std::fill(&Caret_ಸಾಲು[ಮೊದಲ_ಲಂಬಸಾಲು], &Caret_ಸಾಲು[ಕೊನೆಯ_ಲಂಬಸಾಲು], '~');
  }
}

ಸ್ಥಿರ ಶೂನ್ಯ print_ಮೂಲ_ಸಾಲು(raw_ostream &S, StringRef ಸಾಲು_Contents) {
  // Print out the source line one character at a time, so we can expand tabs.
  ಗಾಗಿ (ಚಿಹ್ನೆರಹಿತ i = 0, e = ಸಾಲು_Contents.ಗಾತ್ರ_3(), ಹೊರಹೋಗು_ಲಂಬಸಾಲು = 0; i != e; ++i) {
    size_t ಮುಂದಿನ_Tab = ಸಾಲು_Contents.ಕಂಡುಹಿಡಿ('\t', i);
    // If there were no tabs left, print the rest, we are done.
    ಆದರೆ (ಮುಂದಿನ_Tab == StringRef::npos) {
      S << ಸಾಲು_Contents.drop_front(i);
      ನಿಲ್ಲಿಸು;
    }

    // Otherwise, print from i to NextTab.
    S << ಸಾಲು_Contents.ತುಂಡು(i, ಮುಂದಿನ_Tab);
    ಹೊರಹೋಗು_ಲಂಬಸಾಲು += ಮುಂದಿನ_Tab - i;
    i = ಮುಂದಿನ_Tab;

    // If we have a tab, emit at least one space, then round up to 8 columns.
    ಮಾಡು {
      S << ' ';
      ++ಹೊರಹೋಗು_ಲಂಬಸಾಲು;
    } ಆಗ ((ಹೊರಹೋಗು_ಲಂಬಸಾಲು % TabStop) != 0);
  }
  S << '\n';
}

ಸ್ಥಿರ ತಾರ್ಕಿಕ isNonASCII(ಅಕ್ಷರ c) { ಹಿಂತಿರುಗಿ c & 0x80; }

ಶೂನ್ಯ SMDiagnostic::print(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ProgName, raw_ostream &OS, ತಾರ್ಕಿಕ ShowColors,
                         ತಾರ್ಕಿಕ Show_ಬಗೆ_Label, ತಾರ್ಕಿಕ ShowLocation) ಸ್ಥಿರಾಂಕ {
  Color_ಬಹುಲಕ ಬಹುಲಕ = ShowColors ? Color_ಬಹುಲಕ::Auto : Color_ಬಹುಲಕ::Disable;

  {
    WithColor S(OS, raw_ostream::SAVEDCOLOR, ನಿಜ, ಸುಳ್ಳು, ಬಹುಲಕ);

    ಆದರೆ (ProgName && ProgName[0])
      S << ProgName << ": ";

    ಆದರೆ (ShowLocation && !Filename.ಖಾಲಿಗಳು()) {
      ಆದರೆ (Filename == "-")
        S << "<stdin>";
      ಇಲ್ಲದಿದ್ದರೆ
        S << Filename;

      ಆದರೆ (ಸಾಲು_No != -1) {
        S << ':' << ಸಾಲು_No;
        ಆದರೆ (ಸ್ತಂಭ_No != -1)
          S << ':' << (ಸ್ತಂಭ_No + 1);
      }
      S << ": ";
    }
  }

  ಆದರೆ (Show_ಬಗೆ_Label) {
    ಆಯ್ಕೆ (ಬಗೆ_2) {
    ಸ್ಥಿತಿ ಮೂಲ_Mgr::DK_Error:
      WithColor::error(OS, "", !ShowColors);
      ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ ಮೂಲ_Mgr::DK_Warning:
      WithColor::warning(OS, "", !ShowColors);
      ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ ಮೂಲ_Mgr::DK_Note:
      WithColor::note(OS, "", !ShowColors);
      ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ ಮೂಲ_Mgr::DK_Remark:
      WithColor::remark(OS, "", !ShowColors);
      ನಿಲ್ಲಿಸು;
    }
  }

  WithColor(OS, raw_ostream::SAVEDCOLOR, ನಿಜ, ಸುಳ್ಳು, ಬಹುಲಕ) << ಸಂದೇಶ_3 << '\n';

  ಆದರೆ (ಸಾಲು_No == -1 || ಸ್ತಂಭ_No == -1)
    ಹಿಂತಿರುಗಿ;

  // FIXME: If there are multibyte or multi-column characters in the source, all
  // our ranges will be wrong. To do this properly, we'll need a byte-to-column
  // map like Clang's TextDiagnostic. For now, we'll just handle tabs by
  // expanding them later, and bail out rather than show incorrect ranges and
  // misaligned fixits for any other odd characters.
  ಆದರೆ (any_ದ(ಸಾಲು_Contents, isNonASCII)) {
    print_ಮೂಲ_ಸಾಲು(OS, ಸಾಲು_Contents);
    ಹಿಂತಿರುಗಿ;
  }
  size_t ಅಂಕೆ_Columns = ಸಾಲು_Contents.ಗಾತ್ರ_3();

  // Build the line with the caret and ranges.
  std::string Caret_ಸಾಲು(ಅಂಕೆ_Columns + 1, ' ');

  // Expand any ranges.
  ಗಾಗಿ (ಸ್ಥಿರಾಂಕ std::pair<ಚಿಹ್ನೆರಹಿತ, ಚಿಹ್ನೆರಹಿತ> &R : Ranges)
    std::fill(&Caret_ಸಾಲು[R.ಮೊದಲ_2],
              &Caret_ಸಾಲು[std::min((size_t)R.ಎರಡನೇ_2, Caret_ಸಾಲು.ಗಾತ್ರ_3())], '~');

  // Add any fix-its.
  // FIXME: Find the beginning of the line properly for multibyte characters.
  std::string Fix_It_Insertion_ಸಾಲು;
  build_Fix_It_ಸಾಲು(Caret_ಸಾಲು, Fix_It_Insertion_ಸಾಲು, FixIts,
                 ArrayRef(Loc.get_ಸೂಚಕ() - ಸ್ತಂಭ_No, ಸಾಲು_Contents.ಗಾತ್ರ_3()));

  // Finally, plop on the caret.
  ಆದರೆ (ಚಿಹ್ನೆರಹಿತ(ಸ್ತಂಭ_No) <= ಅಂಕೆ_Columns)
    Caret_ಸಾಲು[ಸ್ತಂಭ_No] = '^';
  ಇಲ್ಲದಿದ್ದರೆ
    Caret_ಸಾಲು[ಅಂಕೆ_Columns] = '^';

  // ... and remove trailing whitespace so the output doesn't wrap for it.  We
  // know that the line isn't completely empty because it has the caret in it at
  // least.
  Caret_ಸಾಲು.erase(Caret_ಸಾಲು.ಕಂಡುಹಿಡಿ_ಕೊನೆಯ_not_ದ(' ') + 1);

  print_ಮೂಲ_ಸಾಲು(OS, ಸಾಲು_Contents);

  {
    Color_ಬಹುಲಕ ಬಹುಲಕ = ShowColors ? Color_ಬಹುಲಕ::Auto : Color_ಬಹುಲಕ::Disable;
    WithColor S(OS, raw_ostream::GREEN, ನಿಜ, ಸುಳ್ಳು, ಬಹುಲಕ);

    // Print out the caret line, matching tabs in the source line.
    ಗಾಗಿ (ಚಿಹ್ನೆರಹಿತ i = 0, e = Caret_ಸಾಲು.ಗಾತ್ರ_3(), ಹೊರಹೋಗು_ಲಂಬಸಾಲು = 0; i != e; ++i) {
      ಆದರೆ (i >= ಸಾಲು_Contents.ಗಾತ್ರ_3() || ಸಾಲು_Contents[i] != '\t') {
        S << Caret_ಸಾಲು[i];
        ++ಹೊರಹೋಗು_ಲಂಬಸಾಲು;
        ಮುಂದುವರಿ;
      }

      // Okay, we have a tab.  Insert the appropriate number of characters.
      ಮಾಡು {
        S << Caret_ಸಾಲು[i];
        ++ಹೊರಹೋಗು_ಲಂಬಸಾಲು;
      } ಆಗ ((ಹೊರಹೋಗು_ಲಂಬಸಾಲು % TabStop) != 0);
    }
    S << '\n';
  }

  // Print out the replacement line, matching tabs in the source line.
  ಆದರೆ (Fix_It_Insertion_ಸಾಲು.ಖಾಲಿಗಳು())
    ಹಿಂತಿರುಗಿ;

  ಗಾಗಿ (size_t i = 0, e = Fix_It_Insertion_ಸಾಲು.ಗಾತ್ರ_3(), ಹೊರಹೋಗು_ಲಂಬಸಾಲು = 0; i < e; ++i) {
    ಆದರೆ (i >= ಸಾಲು_Contents.ಗಾತ್ರ_3() || ಸಾಲು_Contents[i] != '\t') {
      OS << Fix_It_Insertion_ಸಾಲು[i];
      ++ಹೊರಹೋಗು_ಲಂಬಸಾಲು;
      ಮುಂದುವರಿ;
    }

    // Okay, we have a tab.  Insert the appropriate number of characters.
    ಮಾಡು {
      OS << Fix_It_Insertion_ಸಾಲು[i];
      // FIXME: This is trying not to break up replacements, but then to re-sync
      // with the tabs between replacements. This will fail, though, if two
      // fix-it replacements are exactly adjacent, or if a fix-it contains a
      // space. Really we should be precomputing column widths, which we'll
      // need anyway for multibyte chars.
      ಆದರೆ (Fix_It_Insertion_ಸಾಲು[i] != ' ')
        ++i;
      ++ಹೊರಹೋಗು_ಲಂಬಸಾಲು;
    } ಆಗ (((ಹೊರಹೋಗು_ಲಂಬಸಾಲು % TabStop) != 0) && i != e);
  }
  OS << '\n';
}
