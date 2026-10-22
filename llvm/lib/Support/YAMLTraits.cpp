//===- lib/Support/YAMLTraits.cpp -----------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ಅನ್ತರ್ಭಾವ "llvm/Support/YAMLTraits.h"
#ಅನ್ತರ್ಭಾವ "llvm/ADT/STLExtras.h"
#ಅನ್ತರ್ಭಾವ "llvm/ADT/SmallString.h"
#ಅನ್ತರ್ಭಾವ "llvm/ADT/StringExtras.h"
#ಅನ್ತರ್ಭಾವ "llvm/ADT/StringRef.h"
#ಅನ್ತರ್ಭಾವ "llvm/ADT/Twine.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/Casting.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/Errc.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/ErrorHandling.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/Format.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/LineIterator.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/MemoryBuffer.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/VersionTuple.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/YAMLParser.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/raw_ostream.h"
#ಅನ್ತರ್ಭಾವ <cassert>
#ಅನ್ತರ್ಭಾವ <cstdint>
#ಅನ್ತರ್ಭಾವ <cstring>
#ಅನ್ತರ್ಭಾವ <string>
#ಅನ್ತರ್ಭಾವ <vector>

ಬಳಸಿ ನಾಮಾಕಾಶ llvm;
ಬಳಸಿ ನಾಮಾಕಾಶ yaml;

//===----------------------------------------------------------------------===//
//  IO
//===----------------------------------------------------------------------===//

IO::IO(ಶೂನ್ಯ *Context) : Ctxt(Context) {}

IO::~IO() = ಪೂರ್ವನಿಯೋಜಿತ;

ಶೂನ್ಯ *IO::getContext() ಸ್ಥಿರಾಂಕ {
  ಹಿಂತಿರುಗಿ Ctxt;
}

ಶೂನ್ಯ IO::ಗಣ_Context(ಶೂನ್ಯ *Context) {
  Ctxt = Context;
}

ಶೂನ್ಯ IO::ಗಣ_Allow_Unknown_ಕೀಲಿಗಳು(ತಾರ್ಕಿಕ Allow) {
  llvm_unreachable("Only supported for Input");
}

//===----------------------------------------------------------------------===//
//ಒಳಹರಿವು
//===----------------------------------------------------------------------===//

ಒಳಹರಿವು::ಒಳಹರಿವು(StringRef ಒಳಹರಿವು_Content, ಶೂನ್ಯ *Ctxt,
             ಮೂಲ_Mgr::DiagHandlerTy DiagHandler, ಶೂನ್ಯ *DiagHandlerCtxt)
    : IO(Ctxt), Strm(ಹೊಸ ಹರಿವು(ಒಳಹರಿವು_Content, SrcMgr, ಸುಳ್ಳು, &EC)) {
  ಆದರೆ (DiagHandler)
    SrcMgr.ಗಣ_Diag_Handler(DiagHandler, DiagHandlerCtxt);
  DocIterator = Strm->begin();
}

ಒಳಹರಿವು::ಒಳಹರಿವು(ಸ್ಮರಣೆ_ಬಫರ್_Ref ಒಳಹರಿವು, ಶೂನ್ಯ *Ctxt,
             ಮೂಲ_Mgr::DiagHandlerTy DiagHandler, ಶೂನ್ಯ *DiagHandlerCtxt)
    : IO(Ctxt), Strm(ಹೊಸ ಹರಿವು(ಒಳಹರಿವು, SrcMgr, ಸುಳ್ಳು, &EC)) {
  ಆದರೆ (DiagHandler)
    SrcMgr.ಗಣ_Diag_Handler(DiagHandler, DiagHandlerCtxt);
  DocIterator = Strm->begin();
}

ಒಳಹರಿವು::~ಒಳಹರಿವು() = ಪೂರ್ವನಿಯೋಜಿತ;

std::error_ಸಂಕೇತ ಒಳಹರಿವು::error() { ಹಿಂತಿರುಗಿ EC; }

ತಾರ್ಕಿಕ ಒಳಹರಿವು::outputting() ಸ್ಥಿರಾಂಕ {
  ಹಿಂತಿರುಗಿ ಸುಳ್ಳು;
}

ತಾರ್ಕಿಕ ಒಳಹರಿವು::ಗಣ_Current_Document() {
  ಆದರೆ (DocIterator != Strm->ಕೊನೆ()) {
    ಗಂಟು *N = DocIterator->get_ಬೇರು();
    ಆದರೆ (!N) {
      EC = make_error_ಸಂಕೇತ(errc::invalid_ವಾದ);
      ಹಿಂತಿರುಗಿ ಸುಳ್ಳು;
    }

    ಆದರೆ (isa<Null_ಗಂಟು>(N)) {
      // Empty files are allowed and ignored
      ++DocIterator;
      ಹಿಂತಿರುಗಿ ಗಣ_Current_Document();
    }
    releaseHNodeBuffers();
    ತುದಿ_ಗಂಟು = createHNodes(N);
    Current_ಗಂಟು = ತುದಿ_ಗಂಟು;
    ಹಿಂತಿರುಗಿ ನಿಜ;
  }
  ಹಿಂತಿರುಗಿ ಸುಳ್ಳು;
}

ತಾರ್ಕಿಕ ಒಳಹರಿವು::ಮುಂದಿನ_Document() {
  ಹಿಂತಿರುಗಿ ++DocIterator != Strm->ಕೊನೆ();
}

ಸ್ಥಿರಾಂಕ ಗಂಟು *ಒಳಹರಿವು::get_Current_ಗಂಟು() ಸ್ಥಿರಾಂಕ {
  ಹಿಂತಿರುಗಿ Current_ಗಂಟು ? Current_ಗಂಟು->ಗಂಟು_2 : ಶೂನ್ಯಸೂಚಕ;
}

ತಾರ್ಕಿಕ ಒಳಹರಿವು::map_ಟ್ಯಾಗ್(StringRef ಟ್ಯಾಗ್, ತಾರ್ಕಿಕ Default) {
  // CurrentNode can be null if setCurrentDocument() was unable to
  // parse the document because it was invalid or empty.
  ಆದರೆ (!Current_ಗಂಟು)
    ಹಿಂತಿರುಗಿ ಸುಳ್ಳು;

  std::string ಸಿಕ್ಕಿತು_ಟ್ಯಾಗ್ = Current_ಗಂಟು->ಗಂಟು_2->get_Verbatim_ಟ್ಯಾಗ್();
  ಆದರೆ (ಸಿಕ್ಕಿತು_ಟ್ಯಾಗ್.ಖಾಲಿಗಳು()) {
    // If no tag found and 'Tag' is the default, say it was found.
    ಹಿಂತಿರುಗಿ Default;
  }
  // Return true iff found tag matches supplied tag.
  ಹಿಂತಿರುಗಿ ಟ್ಯಾಗ್ == ಸಿಕ್ಕಿತು_ಟ್ಯಾಗ್;
}

ಶೂನ್ಯ ಒಳಹರಿವು::beginMapping() {
  ಆದರೆ (EC)
    ಹಿಂತಿರುಗಿ;
  // CurrentNode can be null if the document is empty.
  MapHNode *MN = dyn_cast_or_null<MapHNode>(Current_ಗಂಟು);
  ಆದರೆ (MN) {
    MN->Valid_ಕೀಲಿಗಳು.clear();
  }
}

std::ಸದಿಶ<StringRef> ಒಳಹರಿವು::ಕೀಲಿಗಳು() {
  MapHNode *MN = dyn_cast<MapHNode>(Current_ಗಂಟು);
  std::ಸದಿಶ<StringRef> Ret;
  ಆದರೆ (!MN) {
    ಗಣ_Error(Current_ಗಂಟು, "not a mapping");
    ಹಿಂತಿರುಗಿ Ret;
  }
  ಗಾಗಿ (ಸ್ವಯಂಚಾಲಿತ &P : MN->Mapping)
    Ret.ತಳ್ಳು_ಹಿಂಭಾಗ(P.ಮೊದಲ_2());
  ಹಿಂತಿರುಗಿ Ret;
}

ತಾರ್ಕಿಕ ಒಳಹರಿವು::preflight_ಕೀಲಿ(StringRef ಕೀಲಿ, ತಾರ್ಕಿಕ Required, ತಾರ್ಕಿಕ, ತಾರ್ಕಿಕ &UseDefault,
                         ಶೂನ್ಯ *&SaveInfo) {
  UseDefault = ಸುಳ್ಳು;
  ಆದರೆ (EC)
    ಹಿಂತಿರುಗಿ ಸುಳ್ಳು;

  // CurrentNode is null for empty documents, which is an error in case required
  // nodes are present.
  ಆದರೆ (!Current_ಗಂಟು) {
    ಆದರೆ (Required)
      EC = make_error_ಸಂಕೇತ(errc::invalid_ವಾದ);
    ಇಲ್ಲದಿದ್ದರೆ
      UseDefault = ನಿಜ;
    ಹಿಂತಿರುಗಿ ಸುಳ್ಳು;
  }

  MapHNode *MN = dyn_cast<MapHNode>(Current_ಗಂಟು);
  ಆದರೆ (!MN) {
    ಆದರೆ (Required || !isa<ಖಾಲಿಗಳು_HNode>(Current_ಗಂಟು))
      ಗಣ_Error(Current_ಗಂಟು, "not a mapping");
    ಇಲ್ಲದಿದ್ದರೆ
      UseDefault = ನಿಜ;
    ಹಿಂತಿರುಗಿ ಸುಳ್ಳು;
  }
  MN->Valid_ಕೀಲಿಗಳು.ತಳ್ಳು_ಹಿಂಭಾಗ(ಕೀಲಿ.str());
  HNode *ಮೌಲ್ಯ = MN->Mapping[ಕೀಲಿ].ಮೊದಲ_2;
  ಆದರೆ (!ಮೌಲ್ಯ) {
    ಆದರೆ (Required)
      ಗಣ_Error(Current_ಗಂಟು, Twine("missing required key '") + ಕೀಲಿ + "'");
    ಇಲ್ಲದಿದ್ದರೆ
      UseDefault = ನಿಜ;
    ಹಿಂತಿರುಗಿ ಸುಳ್ಳು;
  }
  SaveInfo = Current_ಗಂಟು;
  Current_ಗಂಟು = ಮೌಲ್ಯ;
  ಹಿಂತಿರುಗಿ ನಿಜ;
}

ಶೂನ್ಯ ಒಳಹರಿವು::postflight_ಕೀಲಿ(ಶೂನ್ಯ *saveInfo) {
  Current_ಗಂಟು = reinterpret_cast<HNode *>(saveInfo);
}

ಶೂನ್ಯ ಒಳಹರಿವು::ಕೊನೆ_Mapping() {
  ಆದರೆ (EC)
    ಹಿಂತಿರುಗಿ;
  // CurrentNode can be null if the document is empty.
  MapHNode *MN = dyn_cast_or_null<MapHNode>(Current_ಗಂಟು);
  ಆದರೆ (!MN)
    ಹಿಂತಿರುಗಿ;
  ಗಾಗಿ (ಸ್ಥಿರಾಂಕ ಸ್ವಯಂಚಾಲಿತ &NN : MN->Mapping) {
    ಆದರೆ (!is_contained(MN->Valid_ಕೀಲಿಗಳು, NN.ಮೊದಲ_2())) {
      ಸ್ಥಿರಾಂಕ SMRange &ReportLoc = NN.ಎರಡನೇ_2.ಎರಡನೇ_2;
      ಆದರೆ (!Allow_Unknown_ಕೀಲಿಗಳು) {
        ಗಣ_Error(ReportLoc, Twine("unknown key '") + NN.ಮೊದಲ_2() + "'");
        ನಿಲ್ಲಿಸು;
      } ಇಲ್ಲದಿದ್ದರೆ
        reportWarning(ReportLoc, Twine("unknown key '") + NN.ಮೊದಲ_2() + "'");
    }
  }
}

ಶೂನ್ಯ ಒಳಹರಿವು::beginFlowMapping() { beginMapping(); }

ಶೂನ್ಯ ಒಳಹರಿವು::ಕೊನೆ_Flow_Mapping() { ಕೊನೆ_Mapping(); }

ಚಿಹ್ನೆರಹಿತ ಒಳಹರಿವು::beginSequence() {
  ಆದರೆ (SequenceHNode *SQ = dyn_cast<SequenceHNode>(Current_ಗಂಟು))
    ಹಿಂತಿರುಗಿ SQ->Entries.ಗಾತ್ರ_3();
  ಆದರೆ (isa<ಖಾಲಿಗಳು_HNode>(Current_ಗಂಟು))
    ಹಿಂತಿರುಗಿ 0;
  // Treat case where there's a scalar "null" value as an empty sequence.
  ಆದರೆ (ScalarHNode *SN = dyn_cast<ScalarHNode>(Current_ಗಂಟು)) {
    ಆದರೆ (isNull(SN->ಮೌಲ್ಯ_2()))
      ಹಿಂತಿರುಗಿ 0;
  }
  // Any other type of HNode is an error.
  ಗಣ_Error(Current_ಗಂಟು, "not a sequence");
  ಹಿಂತಿರುಗಿ 0;
}

ಶೂನ್ಯ ಒಳಹರಿವು::ಕೊನೆ_Sequence() {
}

ತಾರ್ಕಿಕ ಒಳಹರಿವು::preflightElement(ಚಿಹ್ನೆರಹಿತ ಸೂಚ್ಯಂಕ_2, ಶೂನ್ಯ *&SaveInfo) {
  ಆದರೆ (EC)
    ಹಿಂತಿರುಗಿ ಸುಳ್ಳು;
  ಆದರೆ (SequenceHNode *SQ = dyn_cast<SequenceHNode>(Current_ಗಂಟು)) {
    SaveInfo = Current_ಗಂಟು;
    Current_ಗಂಟು = SQ->Entries[ಸೂಚ್ಯಂಕ_2];
    ಹಿಂತಿರುಗಿ ನಿಜ;
  }
  ಹಿಂತಿರುಗಿ ಸುಳ್ಳು;
}

ಶೂನ್ಯ ಒಳಹರಿವು::postflightElement(ಶೂನ್ಯ *SaveInfo) {
  Current_ಗಂಟು = reinterpret_cast<HNode *>(SaveInfo);
}

ಚಿಹ್ನೆರಹಿತ ಒಳಹರಿವು::beginFlowSequence() { ಹಿಂತಿರುಗಿ beginSequence(); }

ತಾರ್ಕಿಕ ಒಳಹರಿವು::preflightFlowElement(ಚಿಹ್ನೆರಹಿತ ಸೂಚ್ಯಂಕ, ಶೂನ್ಯ *&SaveInfo) {
  ಆದರೆ (EC)
    ಹಿಂತಿರುಗಿ ಸುಳ್ಳು;
  ಆದರೆ (SequenceHNode *SQ = dyn_cast<SequenceHNode>(Current_ಗಂಟು)) {
    SaveInfo = Current_ಗಂಟು;
    Current_ಗಂಟು = SQ->Entries[ಸೂಚ್ಯಂಕ];
    ಹಿಂತಿರುಗಿ ನಿಜ;
  }
  ಹಿಂತಿರುಗಿ ಸುಳ್ಳು;
}

ಶೂನ್ಯ ಒಳಹರಿವು::postflightFlowElement(ಶೂನ್ಯ *SaveInfo) {
  Current_ಗಂಟು = reinterpret_cast<HNode *>(SaveInfo);
}

ಶೂನ್ಯ ಒಳಹರಿವು::ಕೊನೆ_Flow_Sequence() {
}

ಶೂನ್ಯ ಒಳಹರಿವು::beginEnumScalar() {
  Scalar_ಹೊಂದಿಕೆ_ಸಿಕ್ಕಿತು = ಸುಳ್ಳು;
}

ತಾರ್ಕಿಕ ಒಳಹರಿವು::ಹೊಂದಿಕೆ_Enum_Scalar(StringRef Str, ತಾರ್ಕಿಕ) {
  ಆದರೆ (Scalar_ಹೊಂದಿಕೆ_ಸಿಕ್ಕಿತು)
    ಹಿಂತಿರುಗಿ ಸುಳ್ಳು;
  ಆದರೆ (ScalarHNode *SN = dyn_cast<ScalarHNode>(Current_ಗಂಟು)) {
    ಆದರೆ (SN->ಮೌಲ್ಯ_2() == Str) {
      Scalar_ಹೊಂದಿಕೆ_ಸಿಕ್ಕಿತು = ನಿಜ;
      ಹಿಂತಿರುಗಿ ನಿಜ;
    }
  }
  ಹಿಂತಿರುಗಿ ಸುಳ್ಳು;
}

ತಾರ್ಕಿಕ ಒಳಹರಿವು::ಹೊಂದಿಕೆ_Enum_Fallback() {
  ಆದರೆ (Scalar_ಹೊಂದಿಕೆ_ಸಿಕ್ಕಿತು)
    ಹಿಂತಿರುಗಿ ಸುಳ್ಳು;
  Scalar_ಹೊಂದಿಕೆ_ಸಿಕ್ಕಿತು = ನಿಜ;
  ಹಿಂತಿರುಗಿ ನಿಜ;
}

ಶೂನ್ಯ ಒಳಹರಿವು::ಕೊನೆ_Enum_Scalar() {
  ಆದರೆ (!Scalar_ಹೊಂದಿಕೆ_ಸಿಕ್ಕಿತು) {
    ಗಣ_Error(Current_ಗಂಟು, "unknown enumerated scalar");
  }
}

ತಾರ್ಕಿಕ ಒಳಹರಿವು::begin_ಬಿಟ್_ಗಣ_Scalar(ತಾರ್ಕಿಕ &DoClear) {
  ಬಿಟ್_ಮೌಲ್ಯಗಳು_ಬಳಸಿದ.clear();
  ಆದರೆ (SequenceHNode *SQ = dyn_cast<SequenceHNode>(Current_ಗಂಟು)) {
    ಬಿಟ್_ಮೌಲ್ಯಗಳು_ಬಳಸಿದ.resize(SQ->Entries.ಗಾತ್ರ_3());
  } ಇಲ್ಲದಿದ್ದರೆ {
    ಗಣ_Error(Current_ಗಂಟು, "expected sequence of bit values");
  }
  DoClear = ನಿಜ;
  ಹಿಂತಿರುಗಿ ನಿಜ;
}

ತಾರ್ಕಿಕ ಒಳಹರಿವು::ಬಿಟ್_ಗಣ_ಹೊಂದಿಕೆ(StringRef Str, ತಾರ್ಕಿಕ) {
  ಆದರೆ (EC)
    ಹಿಂತಿರುಗಿ ಸುಳ್ಳು;
  ಆದರೆ (SequenceHNode *SQ = dyn_cast<SequenceHNode>(Current_ಗಂಟು)) {
    ಚಿಹ್ನೆರಹಿತ ಸೂಚ್ಯಂಕ_2 = 0;
    ಗಾಗಿ (ಸ್ವಯಂಚಾಲಿತ &N : SQ->Entries) {
      ಆದರೆ (ScalarHNode *SN = dyn_cast<ScalarHNode>(N)) {
        ಆದರೆ (SN->ಮೌಲ್ಯ_2() == Str) {
          ಬಿಟ್_ಮೌಲ್ಯಗಳು_ಬಳಸಿದ[ಸೂಚ್ಯಂಕ_2] = ನಿಜ;
          ಹಿಂತಿರುಗಿ ನಿಜ;
        }
      } ಇಲ್ಲದಿದ್ದರೆ {
        ಗಣ_Error(Current_ಗಂಟು, "unexpected scalar in sequence of bit values");
      }
      ++ಸೂಚ್ಯಂಕ_2;
    }
  } ಇಲ್ಲದಿದ್ದರೆ {
    ಗಣ_Error(Current_ಗಂಟು, "expected sequence of bit values");
  }
  ಹಿಂತಿರುಗಿ ಸುಳ್ಳು;
}

ಶೂನ್ಯ ಒಳಹರಿವು::ಕೊನೆ_ಬಿಟ್_ಗಣ_Scalar() {
  ಆದರೆ (EC)
    ಹಿಂತಿರುಗಿ;
  ಆದರೆ (SequenceHNode *SQ = dyn_cast<SequenceHNode>(Current_ಗಂಟು)) {
    ಪ್ರತಿಜ್ಞಾ(ಬಿಟ್_ಮೌಲ್ಯಗಳು_ಬಳಸಿದ.ಗಾತ್ರ_3() == SQ->Entries.ಗಾತ್ರ_3());
    ಗಾಗಿ (ಚಿಹ್ನೆರಹಿತ i = 0; i < SQ->Entries.ಗಾತ್ರ_3(); ++i) {
      ಆದರೆ (!ಬಿಟ್_ಮೌಲ್ಯಗಳು_ಬಳಸಿದ[i]) {
        ಗಣ_Error(SQ->Entries[i], "unknown bit value");
        ಹಿಂತಿರುಗಿ;
      }
    }
  }
}

ಶೂನ್ಯ ಒಳಹರಿವು::scalarString(StringRef &S, QuotingType) {
  ಆದರೆ (ScalarHNode *SN = dyn_cast<ScalarHNode>(Current_ಗಂಟು)) {
    S = SN->ಮೌಲ್ಯ_2();
  } ಇಲ್ಲದಿದ್ದರೆ {
    ಗಣ_Error(Current_ಗಂಟು, "unexpected scalar");
  }
}

ಶೂನ್ಯ ಒಳಹರಿವು::ಖಂಡ_Scalar_String(StringRef &S) { scalarString(S, QuotingType::None); }

ಶೂನ್ಯ ಒಳಹರಿವು::scalar_ಟ್ಯಾಗ್(std::string &ಟ್ಯಾಗ್) {
  ಟ್ಯಾಗ್ = Current_ಗಂಟು->ಗಂಟು_2->get_Verbatim_ಟ್ಯಾಗ್();
}

ಶೂನ್ಯ ಒಳಹರಿವು::ಗಣ_Error(HNode *hnode, ಸ್ಥಿರಾಂಕ Twine &ಸಂದೇಶ_2) {
  ಪ್ರತಿಜ್ಞಾ(hnode && "HNode must not be NULL");
  ಗಣ_Error(hnode->ಗಂಟು_2, ಸಂದೇಶ_2);
}

ಗಂಟು_ಬಗೆ ಒಳಹರಿವು::get_ಗಂಟು_ಬಗೆ() {
  ಆದರೆ (isa<ScalarHNode>(Current_ಗಂಟು))
    ಹಿಂತಿರುಗಿ ಗಂಟು_ಬಗೆ::Scalar;
  ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (isa<MapHNode>(Current_ಗಂಟು))
    ಹಿಂತಿರುಗಿ ಗಂಟು_ಬಗೆ::Map;
  ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (isa<SequenceHNode>(Current_ಗಂಟು))
    ಹಿಂತಿರುಗಿ ಗಂಟು_ಬಗೆ::Sequence;
  llvm_unreachable("Unsupported node kind");
}

ಶೂನ್ಯ ಒಳಹರಿವು::ಗಣ_Error(ಗಂಟು *ಗಂಟು_3, ಸ್ಥಿರಾಂಕ Twine &ಸಂದೇಶ_2) {
  Strm->printError(ಗಂಟು_3, ಸಂದೇಶ_2);
  EC = make_error_ಸಂಕೇತ(errc::invalid_ವಾದ);
}

ಶೂನ್ಯ ಒಳಹರಿವು::ಗಣ_Error(ಸ್ಥಿರಾಂಕ SMRange &range, ಸ್ಥಿರಾಂಕ Twine &ಸಂದೇಶ_2) {
  Strm->printError(range, ಸಂದೇಶ_2);
  EC = make_error_ಸಂಕೇತ(errc::invalid_ವಾದ);
}

ಶೂನ್ಯ ಒಳಹರಿವು::reportWarning(HNode *hnode, ಸ್ಥಿರಾಂಕ Twine &ಸಂದೇಶ_2) {
  ಪ್ರತಿಜ್ಞಾ(hnode && "HNode must not be NULL");
  Strm->printError(hnode->ಗಂಟು_2, ಸಂದೇಶ_2, ಮೂಲ_Mgr::DK_Warning);
}

ಶೂನ್ಯ ಒಳಹರಿವು::reportWarning(ಗಂಟು *ಗಂಟು_3, ಸ್ಥಿರಾಂಕ Twine &ಸಂದೇಶ_2) {
  Strm->printError(ಗಂಟು_3, ಸಂದೇಶ_2, ಮೂಲ_Mgr::DK_Warning);
}

ಶೂನ್ಯ ಒಳಹರಿವು::reportWarning(ಸ್ಥಿರಾಂಕ SMRange &range, ಸ್ಥಿರಾಂಕ Twine &ಸಂದೇಶ_2) {
  Strm->printError(range, ಸಂದೇಶ_2, ಮೂಲ_Mgr::DK_Warning);
}

ಶೂನ್ಯ ಒಳಹರಿವು::releaseHNodeBuffers() {
  ಖಾಲಿಗಳು_HNode_Allocator.DestroyAll();
  ScalarHNodeAllocator.DestroyAll();
  SequenceHNodeAllocator.DestroyAll();
  MapHNodeAllocator.DestroyAll();
}

ಶೂನ್ಯ ಒಳಹರಿವು::saveAliasHNode(ಗಂಟು *N, HNode *HN) {
  StringRef Anchor = N->getAnchor();
  ಆದರೆ (!Anchor.ಖಾಲಿಗಳು())
    // YAML 1.2.2 - 3.2.2.2. Anchors and Aliases:
    //
    // An alias event refers to the most recent event in the serialization
    // having the specified anchor. Therefore, anchors need not be unique within
    // a serialization. In addition, an anchor need not have an alias node
    // referring to it.
    AliasMap[Anchor] = HN;
}

ಒಳಹರಿವು::HNode *ಒಳಹರಿವು::createHNodes(ಗಂಟು *N) {
  SmallString<128> StringStorage;
  ಆಯ್ಕೆ (N->getType()) {
  ಸ್ಥಿತಿ ಗಂಟು::NK_Scalar: {
    Scalar_ಗಂಟು *SN = dyn_cast<Scalar_ಗಂಟು>(N);
    StringRef ಕೀಲಿ_Str = SN->get_ಮೌಲ್ಯ(StringStorage);
    ಆದರೆ (!StringStorage.ಖಾಲಿಗಳು()) {
      // Copy string to permanent storage
      ಕೀಲಿ_Str = StringStorage.str().copy(StringAllocator);
    }
    ಸ್ವಯಂಚಾಲಿತ *SHNode = ಹೊಸ (ScalarHNodeAllocator.Allocate()) ScalarHNode(N, ಕೀಲಿ_Str);
    saveAliasHNode(SN, SHNode);
    ಹಿಂತಿರುಗಿ SHNode;
  }
  ಸ್ಥಿತಿ ಗಂಟು::NK_ಖಂಡ_Scalar: {
    ಖಂಡ_Scalar_ಗಂಟು *BSN = dyn_cast<ಖಂಡ_Scalar_ಗಂಟು>(N);
    StringRef ಮೌಲ್ಯ_Copy = BSN->get_ಮೌಲ್ಯ().copy(StringAllocator);
    ಸ್ವಯಂಚಾಲಿತ *BSHNode =
        ಹೊಸ (ScalarHNodeAllocator.Allocate()) ScalarHNode(N, ಮೌಲ್ಯ_Copy);
    saveAliasHNode(BSN, BSHNode);
    ಹಿಂತಿರುಗಿ BSHNode;
  }
  ಸ್ಥಿತಿ ಗಂಟು::NK_Sequence: {
    Sequence_ಗಂಟು *SQ = dyn_cast<Sequence_ಗಂಟು>(N);
    ಸ್ವಯಂಚಾಲಿತ SQHNode = ಹೊಸ (SequenceHNodeAllocator.Allocate()) SequenceHNode(N);
    ಗಾಗಿ (ಗಂಟು &SN : *SQ) {
      ಸ್ವಯಂಚಾಲಿತ ನಮೂದು = createHNodes(&SN);
      ಆದರೆ (EC)
        ನಿಲ್ಲಿಸು;
      SQHNode->Entries.ತಳ್ಳು_ಹಿಂಭಾಗ(ನಮೂದು);
    }
    saveAliasHNode(SQ, SQHNode);
    ಹಿಂತಿರುಗಿ SQHNode;
  }
  ಸ್ಥಿತಿ ಗಂಟು::NK_Mapping: {
    Mapping_ಗಂಟು *Map = dyn_cast<Mapping_ಗಂಟು>(N);
    ಸ್ವಯಂಚಾಲಿತ mapHNode = ಹೊಸ (MapHNodeAllocator.Allocate()) MapHNode(N);
    ಗಾಗಿ (ಕೀಲಿ_ಮೌಲ್ಯ_ಗಂಟು &KVN : *Map) {
      ಗಂಟು *ಕೀಲಿ_ಗಂಟು = KVN.get_ಕೀಲಿ();
      Scalar_ಗಂಟು *ಕೀಲಿ = dyn_cast_if_present<Scalar_ಗಂಟು>(ಕೀಲಿ_ಗಂಟು);
      ಗಂಟು *ಮೌಲ್ಯ = KVN.get_ಮೌಲ್ಯ();
      ಆದರೆ (!ಕೀಲಿ || !ಮೌಲ್ಯ) {
        ಆದರೆ (!ಕೀಲಿ)
          ಗಣ_Error(ಕೀಲಿ_ಗಂಟು, "Map key must be a scalar");
        ಆದರೆ (!ಮೌಲ್ಯ)
          ಗಣ_Error(ಕೀಲಿ_ಗಂಟು, "Map value must not be empty");
        ನಿಲ್ಲಿಸು;
      }
      StringStorage.clear();
      StringRef ಕೀಲಿ_Str = ಕೀಲಿ->get_ಮೌಲ್ಯ(StringStorage);
      ಆದರೆ (!StringStorage.ಖಾಲಿಗಳು()) {
        // Copy string to permanent storage
        ಕೀಲಿ_Str = StringStorage.str().copy(StringAllocator);
      }
      ಆದರೆ (mapHNode->Mapping.ಎಣಿಕೆ_2(ಕೀಲಿ_Str))
        // From YAML spec: "The content of a mapping node is an unordered set of
        // key/value node pairs, with the restriction that each of the keys is
        // unique."
        ಗಣ_Error(ಕೀಲಿ_ಗಂಟು, Twine("duplicated mapping key '") + ಕೀಲಿ_Str + "'");
      ಸ್ವಯಂಚಾಲಿತ ಮೌಲ್ಯ_HNode = createHNodes(ಮೌಲ್ಯ);
      ಆದರೆ (EC)
        ನಿಲ್ಲಿಸು;
      mapHNode->Mapping[ಕೀಲಿ_Str] =
          std::make_pair(std::move(ಮೌಲ್ಯ_HNode), ಕೀಲಿ_ಗಂಟು->get_ಮೂಲ_Range());
    }
    saveAliasHNode(Map, mapHNode);
    ಹಿಂತಿರುಗಿ std::move(mapHNode);
  }
  ಸ್ಥಿತಿ ಗಂಟು::NK_Null:
    // TODO: Anchor is not set for NullNode in the parser. Update the parser to
    // faithfully preserve anchors.
    ಹಿಂತಿರುಗಿ ಹೊಸ (ಖಾಲಿಗಳು_HNode_Allocator.Allocate()) ಖಾಲಿಗಳು_HNode(N);
  ಸ್ಥಿತಿ ಗಂಟು::NK_Alias: {
    Alias_ಗಂಟು *AN = dyn_cast<Alias_ಗಂಟು>(N);
    ಸ್ವಯಂಚಾಲಿತ AliasName = AN->getName();
    ಸ್ವಯಂಚಾಲಿತ AHN = AliasMap.ಕಂಡುಹಿಡಿ(AliasName);
    ಆದರೆ (AHN == AliasMap.ಕೊನೆ()) {
      ಗಣ_Error(AN, Twine("undefined alias '" + AliasName + "'"));
      ಹಿಂತಿರುಗಿ ಶೂನ್ಯಸೂಚಕ;
    }
    ಹಿಂತಿರುಗಿ AHN->ಎರಡನೇ_2;
  }
  ಪೂರ್ವನಿಯೋಜಿತ:
    ಗಣ_Error(N, "unknown node kind");
    ಹಿಂತಿರುಗಿ ಶೂನ್ಯಸೂಚಕ;
  }
}

ಶೂನ್ಯ ಒಳಹರಿವು::ಗಣ_Error(ಸ್ಥಿರಾಂಕ Twine &ಸಂದೇಶ_3) {
  ಗಣ_Error(Current_ಗಂಟು, ಸಂದೇಶ_3);
}

ಶೂನ್ಯ ಒಳಹರಿವು::ಗಣ_Allow_Unknown_ಕೀಲಿಗಳು(ತಾರ್ಕಿಕ Allow) { Allow_Unknown_ಕೀಲಿಗಳು = Allow; }

ತಾರ್ಕಿಕ ಒಳಹರಿವು::can_Elide_ಖಾಲಿಗಳು_Sequence() {
  ಹಿಂತಿರುಗಿ ಸುಳ್ಳು;
}

//===----------------------------------------------------------------------===//
//  Output
//===----------------------------------------------------------------------===//

Output::Output(raw_ostream &yout, ಶೂನ್ಯ *context, ಪೂರ್ಣಾಂಕ Wrap_ಸ್ತಂಭ)
    : IO(context), ಹೊರಹೋಗು(yout), Wrap_ಸ್ತಂಭ(Wrap_ಸ್ತಂಭ) {}

Output::~Output() = ಪೂರ್ವನಿಯೋಜಿತ;

ತಾರ್ಕಿಕ Output::outputting() ಸ್ಥಿರಾಂಕ {
  ಹಿಂತಿರುಗಿ ನಿಜ;
}

ಶೂನ್ಯ Output::beginMapping() {
  State_ರಾಶಿ.ತಳ್ಳು_ಹಿಂಭಾಗ(ಒಳ_Map_ಮೊದಲ_ಕೀಲಿ);
  PaddingBeforeContainer = Padding;
  Padding = "\n";
}

ತಾರ್ಕಿಕ Output::map_ಟ್ಯಾಗ್(StringRef ಟ್ಯಾಗ್, ತಾರ್ಕಿಕ Use) {
  ಆದರೆ (Use) {
    // If this tag is being written inside a sequence we should write the start
    // of the sequence before writing the tag, otherwise the tag won't be
    // attached to the element in the sequence, but rather the sequence itself.
    ತಾರ್ಕಿಕ SequenceElement = ಸುಳ್ಳು;
    ಆದರೆ (State_ರಾಶಿ.ಗಾತ್ರ_3() > 1) {
      ಸ್ವಯಂಚಾಲಿತ &E = State_ರಾಶಿ[State_ರಾಶಿ.ಗಾತ್ರ_3() - 2];
      SequenceElement = ಒಳ_Seq_Any_Element(E) || ಒಳ_Flow_Seq_Any_Element(E);
    }
    ಆದರೆ (SequenceElement && State_ರಾಶಿ.ಹಿಂಭಾಗ() == ಒಳ_Map_ಮೊದಲ_ಕೀಲಿ) {
      new_ಸಾಲು_Check();
    } ಇಲ್ಲದಿದ್ದರೆ {
      output(" ");
    }
    output(ಟ್ಯಾಗ್);
    ಆದರೆ (SequenceElement) {
      // If we're writing the tag during the first element of a map, the tag
      // takes the place of the first element in the sequence.
      ಆದರೆ (State_ರಾಶಿ.ಹಿಂಭಾಗ() == ಒಳ_Map_ಮೊದಲ_ಕೀಲಿ) {
        State_ರಾಶಿ.ತೆಗೆ_ಹಿಂಭಾಗ();
        State_ರಾಶಿ.ತಳ್ಳು_ಹಿಂಭಾಗ(ಒಳ_Map_Other_ಕೀಲಿ);
      }
      // Tags inside maps in sequences should act as keys in the map from a
      // formatting perspective, so we always want a newline in a sequence.
      Padding = "\n";
    }
  }
  ಹಿಂತಿರುಗಿ Use;
}

ಶೂನ್ಯ Output::ಕೊನೆ_Mapping() {
  // If we did not map anything, we should explicitly emit an empty map
  ಆದರೆ (State_ರಾಶಿ.ಹಿಂಭಾಗ() == ಒಳ_Map_ಮೊದಲ_ಕೀಲಿ) {
    Padding = PaddingBeforeContainer;
    new_ಸಾಲು_Check();
    output("{}");
    Padding = "\n";
  }
  State_ರಾಶಿ.ತೆಗೆ_ಹಿಂಭಾಗ();
}

std::ಸದಿಶ<StringRef> Output::ಕೀಲಿಗಳು() {
  report_fatal_error("invalid call");
}

ತಾರ್ಕಿಕ Output::preflight_ಕೀಲಿ(StringRef ಕೀಲಿ, ತಾರ್ಕಿಕ Required, ತಾರ್ಕಿಕ SameAsDefault,
                          ತಾರ್ಕಿಕ &UseDefault, ಶೂನ್ಯ *&SaveInfo) {
  UseDefault = ಸುಳ್ಳು;
  SaveInfo = ಶೂನ್ಯಸೂಚಕ;
  ಆದರೆ (Required || !SameAsDefault || Write_Default_ಮೌಲ್ಯಗಳು) {
    ಸ್ವಯಂಚಾಲಿತ State = State_ರಾಶಿ.ಹಿಂಭಾಗ();
    ಆದರೆ (State == ಒಳ_Flow_Map_ಮೊದಲ_ಕೀಲಿ || State == ಒಳ_Flow_Map_Other_ಕೀಲಿ) {
      flow_ಕೀಲಿ(ಕೀಲಿ);
    } ಇಲ್ಲದಿದ್ದರೆ {
      new_ಸಾಲು_Check();
      padded_ಕೀಲಿ(ಕೀಲಿ);
    }
    ಹಿಂತಿರುಗಿ ನಿಜ;
  }
  ಹಿಂತಿರುಗಿ ಸುಳ್ಳು;
}

ಶೂನ್ಯ Output::postflight_ಕೀಲಿ(ಶೂನ್ಯ *) {
  ಆದರೆ (State_ರಾಶಿ.ಹಿಂಭಾಗ() == ಒಳ_Map_ಮೊದಲ_ಕೀಲಿ) {
    State_ರಾಶಿ.ತೆಗೆ_ಹಿಂಭಾಗ();
    State_ರಾಶಿ.ತಳ್ಳು_ಹಿಂಭಾಗ(ಒಳ_Map_Other_ಕೀಲಿ);
  } ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (State_ರಾಶಿ.ಹಿಂಭಾಗ() == ಒಳ_Flow_Map_ಮೊದಲ_ಕೀಲಿ) {
    State_ರಾಶಿ.ತೆಗೆ_ಹಿಂಭಾಗ();
    State_ರಾಶಿ.ತಳ್ಳು_ಹಿಂಭಾಗ(ಒಳ_Flow_Map_Other_ಕೀಲಿ);
  }
}

ಶೂನ್ಯ Output::beginFlowMapping() {
  State_ರಾಶಿ.ತಳ್ಳು_ಹಿಂಭಾಗ(ಒಳ_Flow_Map_ಮೊದಲ_ಕೀಲಿ);
  new_ಸಾಲು_Check();
  ಸ್ತಂಭ_At_Map_Flow_ಆರಂಭ = ಸ್ತಂಭ;
  output("{ ");
}

ಶೂನ್ಯ Output::ಕೊನೆ_Flow_Mapping() {
  State_ರಾಶಿ.ತೆಗೆ_ಹಿಂಭಾಗ();
  output_ಮೇಲೆ_ಗಮ್ಯ_ಕೊನೆ_ದ_ಸಾಲು(" }");
}

ಶೂನ್ಯ Output::beginDocuments() {
  output_ಮೇಲೆ_ಗಮ್ಯ_ಕೊನೆ_ದ_ಸಾಲು("---");
}

ತಾರ್ಕಿಕ Output::preflightDocument(ಚಿಹ್ನೆರಹಿತ ಸೂಚ್ಯಂಕ) {
  ಆದರೆ (ಸೂಚ್ಯಂಕ > 0)
    output_ಮೇಲೆ_ಗಮ್ಯ_ಕೊನೆ_ದ_ಸಾಲು("\n---");
  ಹಿಂತಿರುಗಿ ನಿಜ;
}

ಶೂನ್ಯ Output::postflightDocument() {
}

ಶೂನ್ಯ Output::ಕೊನೆ_Documents() {
  output("\n...\n");
}

ಚಿಹ್ನೆರಹಿತ Output::beginSequence() {
  State_ರಾಶಿ.ತಳ್ಳು_ಹಿಂಭಾಗ(ಒಳ_Seq_ಮೊದಲ_Element);
  PaddingBeforeContainer = Padding;
  Padding = "\n";
  ಹಿಂತಿರುಗಿ 0;
}

ಶೂನ್ಯ Output::ಕೊನೆ_Sequence() {
  // If we did not emit anything, we should explicitly emit an empty sequence
  ಆದರೆ (State_ರಾಶಿ.ಹಿಂಭಾಗ() == ಒಳ_Seq_ಮೊದಲ_Element) {
    Padding = PaddingBeforeContainer;
    new_ಸಾಲು_Check(/*EmptySequence=*/ನಿಜ);
    output("[]");
    Padding = "\n";
  }
  State_ರಾಶಿ.ತೆಗೆ_ಹಿಂಭಾಗ();
}

ತಾರ್ಕಿಕ Output::preflightElement(ಚಿಹ್ನೆರಹಿತ, ಶೂನ್ಯ *&SaveInfo) {
  SaveInfo = ಶೂನ್ಯಸೂಚಕ;
  ಹಿಂತಿರುಗಿ ನಿಜ;
}

ಶೂನ್ಯ Output::postflightElement(ಶೂನ್ಯ *) {
  ಆದರೆ (State_ರಾಶಿ.ಹಿಂಭಾಗ() == ಒಳ_Seq_ಮೊದಲ_Element) {
    State_ರಾಶಿ.ತೆಗೆ_ಹಿಂಭಾಗ();
    State_ರಾಶಿ.ತಳ್ಳು_ಹಿಂಭಾಗ(ಒಳ_Seq_Other_Element);
  } ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (State_ರಾಶಿ.ಹಿಂಭಾಗ() == ಒಳ_Flow_Seq_ಮೊದಲ_Element) {
    State_ರಾಶಿ.ತೆಗೆ_ಹಿಂಭಾಗ();
    State_ರಾಶಿ.ತಳ್ಳು_ಹಿಂಭಾಗ(ಒಳ_Flow_Seq_Other_Element);
  }
}

ಚಿಹ್ನೆರಹಿತ Output::beginFlowSequence() {
  State_ರಾಶಿ.ತಳ್ಳು_ಹಿಂಭಾಗ(ಒಳ_Flow_Seq_ಮೊದಲ_Element);
  new_ಸಾಲು_Check();
  ಸ್ತಂಭ_At_Flow_ಆರಂಭ = ಸ್ತಂಭ;
  output("[ ");
  NeedFlowSequenceComma = ಸುಳ್ಳು;
  ಹಿಂತಿರುಗಿ 0;
}

ಶೂನ್ಯ Output::ಕೊನೆ_Flow_Sequence() {
  State_ರಾಶಿ.ತೆಗೆ_ಹಿಂಭಾಗ();
  output_ಮೇಲೆ_ಗಮ್ಯ_ಕೊನೆ_ದ_ಸಾಲು(" ]");
}

ತಾರ್ಕಿಕ Output::preflightFlowElement(ಚಿಹ್ನೆರಹಿತ, ಶೂನ್ಯ *&SaveInfo) {
  ಆದರೆ (NeedFlowSequenceComma)
    output(", ");
  ಆದರೆ (Wrap_ಸ್ತಂಭ && ಸ್ತಂಭ > Wrap_ಸ್ತಂಭ) {
    output("\n");
    ಗಾಗಿ (ಪೂರ್ಣಾಂಕ i = 0; i < ಸ್ತಂಭ_At_Flow_ಆರಂಭ; ++i)
      output(" ");
    ಸ್ತಂಭ = ಸ್ತಂಭ_At_Flow_ಆರಂಭ;
    output("  ");
  }
  SaveInfo = ಶೂನ್ಯಸೂಚಕ;
  ಹಿಂತಿರುಗಿ ನಿಜ;
}

ಶೂನ್ಯ Output::postflightFlowElement(ಶೂನ್ಯ *) {
  NeedFlowSequenceComma = ನಿಜ;
}

ಶೂನ್ಯ Output::beginEnumScalar() {
  Enumeration_ಹೊಂದಿಕೆ_ಸಿಕ್ಕಿತು = ಸುಳ್ಳು;
}

ತಾರ್ಕಿಕ Output::ಹೊಂದಿಕೆ_Enum_Scalar(StringRef Str, ತಾರ್ಕಿಕ ಹೊಂದಿಕೆ_2) {
  ಆದರೆ (ಹೊಂದಿಕೆ_2 && !Enumeration_ಹೊಂದಿಕೆ_ಸಿಕ್ಕಿತು) {
    new_ಸಾಲು_Check();
    output_ಮೇಲೆ_ಗಮ್ಯ_ಕೊನೆ_ದ_ಸಾಲು(Str);
    Enumeration_ಹೊಂದಿಕೆ_ಸಿಕ್ಕಿತು = ನಿಜ;
  }
  ಹಿಂತಿರುಗಿ ಸುಳ್ಳು;
}

ತಾರ್ಕಿಕ Output::ಹೊಂದಿಕೆ_Enum_Fallback() {
  ಆದರೆ (Enumeration_ಹೊಂದಿಕೆ_ಸಿಕ್ಕಿತು)
    ಹಿಂತಿರುಗಿ ಸುಳ್ಳು;
  Enumeration_ಹೊಂದಿಕೆ_ಸಿಕ್ಕಿತು = ನಿಜ;
  ಹಿಂತಿರುಗಿ ನಿಜ;
}

ಶೂನ್ಯ Output::ಕೊನೆ_Enum_Scalar() {
  ಆದರೆ (!Enumeration_ಹೊಂದಿಕೆ_ಸಿಕ್ಕಿತು)
    llvm_unreachable("bad runtime enum value");
}

ತಾರ್ಕಿಕ Output::begin_ಬಿಟ್_ಗಣ_Scalar(ತಾರ್ಕಿಕ &DoClear) {
  new_ಸಾಲು_Check();
  output("[ ");
  Need_ಬಿಟ್_ಮೌಲ್ಯ_Comma = ಸುಳ್ಳು;
  DoClear = ಸುಳ್ಳು;
  ಹಿಂತಿರುಗಿ ನಿಜ;
}

ತಾರ್ಕಿಕ Output::ಬಿಟ್_ಗಣ_ಹೊಂದಿಕೆ(StringRef Str, ತಾರ್ಕಿಕ Matches) {
  ಆದರೆ (Matches) {
    ಆದರೆ (Need_ಬಿಟ್_ಮೌಲ್ಯ_Comma)
      output(", ");
    output(Str);
    Need_ಬಿಟ್_ಮೌಲ್ಯ_Comma = ನಿಜ;
  }
  ಹಿಂತಿರುಗಿ ಸುಳ್ಳು;
}

ಶೂನ್ಯ Output::ಕೊನೆ_ಬಿಟ್_ಗಣ_Scalar() {
  output_ಮೇಲೆ_ಗಮ್ಯ_ಕೊನೆ_ದ_ಸಾಲು(" ]");
}

ಶೂನ್ಯ Output::scalarString(StringRef &S, QuotingType MustQuote) {
  new_ಸಾಲು_Check();
  ಆದರೆ (S.ಖಾಲಿಗಳು()) {
    // Print '' for the empty string because leaving the field empty is not
    // allowed.
    output_ಮೇಲೆ_ಗಮ್ಯ_ಕೊನೆ_ದ_ಸಾಲು("''");
    ಹಿಂತಿರುಗಿ;
  }
  output(S, MustQuote);
  output_ಮೇಲೆ_ಗಮ್ಯ_ಕೊನೆ_ದ_ಸಾಲು("");
}

ಶೂನ್ಯ Output::ಖಂಡ_Scalar_String(StringRef &S) {
  ಆದರೆ (!State_ರಾಶಿ.ಖಾಲಿಗಳು())
    new_ಸಾಲು_Check();
  output(" |");

  ಚಿಹ್ನೆರಹಿತ Indent = State_ರಾಶಿ.ಖಾಲಿಗಳು() ? 1 : State_ರಾಶಿ.ಗಾತ್ರ_3();

  ಸ್ವಯಂಚಾಲಿತ ಬಫರ್ = ಸ್ಮರಣೆ_ಬಫರ್::get_Mem_ಬಫರ್(S, "", ಸುಳ್ಳು);
  ಗಾಗಿ (ಸಾಲು_iterator ಸಾಲುಗಳು(*ಬಫರ್, ಸುಳ್ಳು); !ಸಾಲುಗಳು.is_at_ಕೊನೆ(); ++ಸಾಲುಗಳು) {
    output_New_ಸಾಲು();
    ಗಾಗಿ (ಚಿಹ್ನೆರಹಿತ I = 0; I < Indent; ++I) {
      output("  ");
    }
    output(*ಸಾಲುಗಳು);
  }
  output_ಮೇಲೆ_ಗಮ್ಯ_ಕೊನೆ_ದ_ಸಾಲು("");
}

ಶೂನ್ಯ Output::scalar_ಟ್ಯಾಗ್(std::string &ಟ್ಯಾಗ್) {
  ಆದರೆ (ಟ್ಯಾಗ್.ಖಾಲಿಗಳು())
    ಹಿಂತಿರುಗಿ;
  new_ಸಾಲು_Check();
  output(ಟ್ಯಾಗ್);
  output(" ");
}

ಶೂನ್ಯ Output::ಗಣ_Error(ಸ್ಥಿರಾಂಕ Twine &ಸಂದೇಶ_2) {
}

std::error_ಸಂಕೇತ Output::error() { ಹಿಂತಿರುಗಿ {}; }

ತಾರ್ಕಿಕ Output::can_Elide_ಖಾಲಿಗಳು_Sequence() {
  // Normally, with an optional key/value where the value is an empty sequence,
  // the whole key/value can be not written.  But, that produces wrong yaml
  // if the key/value is the only thing in the map and the map is used in
  // a sequence.  This detects if the this sequence is the first key/value
  // in map that itself is embedded in a sequence.
  ಆದರೆ (State_ರಾಶಿ.ಗಾತ್ರ_3() < 2)
    ಹಿಂತಿರುಗಿ ನಿಜ;
  ಆದರೆ (State_ರಾಶಿ.ಹಿಂಭಾಗ() != ಒಳ_Map_ಮೊದಲ_ಕೀಲಿ)
    ಹಿಂತಿರುಗಿ ನಿಜ;
  ಹಿಂತಿರುಗಿ !ಒಳ_Seq_Any_Element(State_ರಾಶಿ[State_ರಾಶಿ.ಗಾತ್ರ_3() - 2]);
}

ಶೂನ್ಯ Output::output(StringRef s) {
  ಸ್ತಂಭ += s.ಗಾತ್ರ_3();
  ಹೊರಹೋಗು << s;
}

ಶೂನ್ಯ Output::output(StringRef S, QuotingType MustQuote) {
  ಆದರೆ (MustQuote == QuotingType::None) {
    // Only quote if we must.
    output(S);
    ಹಿಂತಿರುಗಿ;
  }

  StringLiteral Quote = MustQuote == QuotingType::Single ? StringLiteral("'")
                                                         : StringLiteral("\"");
  output(Quote); // Starting quote.

  // When using double-quoted strings (and only in that case), non-printable
  // characters may be present, and will be escaped using a variety of
  // unicode-scalar and special short-form escapes. This is handled in
  // yaml::escape.
  ಆದರೆ (MustQuote == QuotingType::Double) {
    output(yaml::escape(S, /* EscapePrintable= */ ಸುಳ್ಳು));
    output(Quote);
    ಹಿಂತಿರುಗಿ;
  }

  ಚಿಹ್ನೆರಹಿತ i = 0;
  ಚಿಹ್ನೆರಹಿತ j = 0;
  ಚಿಹ್ನೆರಹಿತ ಕೊನೆ_2 = S.ಗಾತ್ರ_3();
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ತಳ = S.ದತ್ತಾಂಶ();

  // When using single-quoted strings, any single quote ' must be doubled to be
  // escaped.
  ಆಗ (j < ಕೊನೆ_2) {
    ಆದರೆ (S[j] == '\'') {                   // Escape quotes.
      output(StringRef(&ತಳ[i], j - i)); // "flush".
      output(StringLiteral("''"));        // Print it as ''
      i = j + 1;
    }
    ++j;
  }
  output(StringRef(&ತಳ[i], j - i));
  output(Quote); // Ending quote.
}

ಶೂನ್ಯ Output::output_ಮೇಲೆ_ಗಮ್ಯ_ಕೊನೆ_ದ_ಸಾಲು(StringRef s) {
  output(s);
  ಆದರೆ (State_ರಾಶಿ.ಖಾಲಿಗಳು() || (!ಒಳ_Flow_Seq_Any_Element(State_ರಾಶಿ.ಹಿಂಭಾಗ()) &&
                             !ಒಳ_Flow_Map_Any_ಕೀಲಿ(State_ರಾಶಿ.ಹಿಂಭಾಗ())))
    Padding = "\n";
}

ಶೂನ್ಯ Output::output_New_ಸಾಲು() {
  ಹೊರಹೋಗು << "\n";
  ಸ್ತಂಭ = 0;
}

// if seq at top, indent as if map, then add "- "
// if seq in middle, use "- " if firstKey, else use "  "
//

ಶೂನ್ಯ Output::new_ಸಾಲು_Check(ತಾರ್ಕಿಕ ಖಾಲಿಗಳು_Sequence) {
  ಆದರೆ (Padding != "\n") {
    output(Padding);
    Padding = {};
    ಹಿಂತಿರುಗಿ;
  }
  output_New_ಸಾಲು();
  Padding = {};

  ಆದರೆ (State_ರಾಶಿ.ಗಾತ್ರ_3() == 0 || ಖಾಲಿಗಳು_Sequence)
    ಹಿಂತಿರುಗಿ;

  ಚಿಹ್ನೆರಹಿತ Indent = State_ರಾಶಿ.ಗಾತ್ರ_3() - 1;
  ತಾರ್ಕಿಕ PossiblyNestedSeq = ಸುಳ್ಳು;
  ಸ್ವಯಂಚಾಲಿತ I = State_ರಾಶಿ.rbegin(), E = State_ರಾಶಿ.rend();

  ಆದರೆ (ಒಳ_Seq_Any_Element(*I)) {
    PossiblyNestedSeq = ನಿಜ; // Not possibly but always.
    ++Indent;
  } ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (*I == ಒಳ_Map_ಮೊದಲ_ಕೀಲಿ || *I == ಒಳ_Flow_Map_ಮೊದಲ_ಕೀಲಿ ||
             ಒಳ_Flow_Seq_Any_Element(*I)) {
    PossiblyNestedSeq = ನಿಜ;
    ++I; // Skip back().
  }

  ಚಿಹ್ನೆರಹಿತ Output_Dash_ಎಣಿಕೆ = 0;
  ಆದರೆ (PossiblyNestedSeq) {
    // Count up consecutive inSeqFirstElement from the end, unless
    // inSeqFirstElement is the top of nested sequence.
    ಆಗ (I != E) {
      // Don't count the top of nested sequence.
      ಆದರೆ (!ಒಳ_Seq_Any_Element(*I))
        ನಿಲ್ಲಿಸು;

      ++Output_Dash_ಎಣಿಕೆ;

      // Stop counting if consecutive inSeqFirstElement ends.
      ಆದರೆ (*I++ != ಒಳ_Seq_ಮೊದಲ_Element)
        ನಿಲ್ಲಿಸು;
    }
  }

  ಗಾಗಿ (ಚಿಹ್ನೆರಹಿತ I = Output_Dash_ಎಣಿಕೆ; I < Indent; ++I)
    output("  ");

  ಗಾಗಿ (ಚಿಹ್ನೆರಹಿತ I = 0; I < Output_Dash_ಎಣಿಕೆ; ++I)
    output("- ");
}

ಶೂನ್ಯ Output::padded_ಕೀಲಿ(StringRef ಕೀಲಿ_2) {
  output(ಕೀಲಿ_2, needsQuotes(ಕೀಲಿ_2, ಸುಳ್ಳು));
  output(":");
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *spaces = "                ";
  ಆದರೆ (ಕೀಲಿ_2.ಗಾತ್ರ_3() < ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(spaces))
    Padding = &spaces[ಕೀಲಿ_2.ಗಾತ್ರ_3()];
  ಇಲ್ಲದಿದ್ದರೆ
    Padding = " ";
}

ಶೂನ್ಯ Output::flow_ಕೀಲಿ(StringRef ಕೀಲಿ) {
  ಆದರೆ (State_ರಾಶಿ.ಹಿಂಭಾಗ() == ಒಳ_Flow_Map_Other_ಕೀಲಿ)
    output(", ");
  ಆದರೆ (Wrap_ಸ್ತಂಭ && ಸ್ತಂಭ > Wrap_ಸ್ತಂಭ) {
    output("\n");
    ಗಾಗಿ (ಪೂರ್ಣಾಂಕ I = 0; I < ಸ್ತಂಭ_At_Map_Flow_ಆರಂಭ; ++I)
      output(" ");
    ಸ್ತಂಭ = ಸ್ತಂಭ_At_Map_Flow_ಆರಂಭ;
    output("  ");
  }
  output(ಕೀಲಿ, needsQuotes(ಕೀಲಿ, ಸುಳ್ಳು));
  output(": ");
}

ಗಂಟು_ಬಗೆ Output::get_ಗಂಟು_ಬಗೆ() { report_fatal_error("invalid call"); }

ತಾರ್ಕಿಕ Output::ಒಳ_Seq_Any_Element(ಒಳ_State State) {
  ಹಿಂತಿರುಗಿ State == ಒಳ_Seq_ಮೊದಲ_Element || State == ಒಳ_Seq_Other_Element;
}

ತಾರ್ಕಿಕ Output::ಒಳ_Flow_Seq_Any_Element(ಒಳ_State State) {
  ಹಿಂತಿರುಗಿ State == ಒಳ_Flow_Seq_ಮೊದಲ_Element || State == ಒಳ_Flow_Seq_Other_Element;
}

ತಾರ್ಕಿಕ Output::ಒಳ_Map_Any_ಕೀಲಿ(ಒಳ_State State) {
  ಹಿಂತಿರುಗಿ State == ಒಳ_Map_ಮೊದಲ_ಕೀಲಿ || State == ಒಳ_Map_Other_ಕೀಲಿ;
}

ತಾರ್ಕಿಕ Output::ಒಳ_Flow_Map_Any_ಕೀಲಿ(ಒಳ_State State) {
  ಹಿಂತಿರುಗಿ State == ಒಳ_Flow_Map_ಮೊದಲ_ಕೀಲಿ || State == ಒಳ_Flow_Map_Other_ಕೀಲಿ;
}

//===----------------------------------------------------------------------===//
//  traits for built-in types
//===----------------------------------------------------------------------===//

ಶೂನ್ಯ ScalarTraits<ತಾರ್ಕಿಕ>::output(ಸ್ಥಿರಾಂಕ ತಾರ್ಕಿಕ &Val, ಶೂನ್ಯ *, raw_ostream &ಹೊರಹೋಗು) {
  ಹೊರಹೋಗು << (Val ? "true" : "false");
}

StringRef ScalarTraits<ತಾರ್ಕಿಕ>::ಒಳಹರಿವು_2(StringRef Scalar, ಶೂನ್ಯ *, ತಾರ್ಕಿಕ &Val) {
  ಆದರೆ (std::optional<ತಾರ್ಕಿಕ> Parsed = parseBool(Scalar)) {
    Val = *Parsed;
    ಹಿಂತಿರುಗಿ StringRef();
  }
  ಹಿಂತಿರುಗಿ "invalid boolean";
}

ಶೂನ್ಯ ScalarTraits<StringRef>::output(ಸ್ಥಿರಾಂಕ StringRef &Val, ಶೂನ್ಯ *,
                                     raw_ostream &ಹೊರಹೋಗು) {
  ಹೊರಹೋಗು << Val;
}

StringRef ScalarTraits<StringRef>::ಒಳಹರಿವು_2(StringRef Scalar, ಶೂನ್ಯ *,
                                         StringRef &Val) {
  Val = Scalar;
  ಹಿಂತಿರುಗಿ StringRef();
}

ಶೂನ್ಯ ScalarTraits<std::string>::output(ಸ್ಥಿರಾಂಕ std::string &Val, ಶೂನ್ಯ *,
                                       raw_ostream &ಹೊರಹೋಗು) {
  ಹೊರಹೋಗು << Val;
}

StringRef ScalarTraits<std::string>::ಒಳಹರಿವು_2(StringRef Scalar, ಶೂನ್ಯ *,
                                           std::string &Val) {
  Val = Scalar.str();
  ಹಿಂತಿರುಗಿ StringRef();
}

ಶೂನ್ಯ ScalarTraits<uint8_t>::output(ಸ್ಥಿರಾಂಕ uint8_t &Val, ಶೂನ್ಯ *,
                                   raw_ostream &ಹೊರಹೋಗು) {
  // use temp uin32_t because ostream thinks uint8_t is a character
  uint32_t ಅಂಕೆ = Val;
  ಹೊರಹೋಗು << ಅಂಕೆ;
}

StringRef ScalarTraits<uint8_t>::ಒಳಹರಿವು_2(StringRef Scalar, ಶೂನ್ಯ *, uint8_t &Val) {
  ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ ದೀರ್ಘ n;
  ಆದರೆ (getAsUnsignedInteger(Scalar, 0, n))
    ಹಿಂತಿರುಗಿ "invalid number";
  ಆದರೆ (n > 0xFF)
    ಹಿಂತಿರುಗಿ "out of range number";
  Val = n;
  ಹಿಂತಿರುಗಿ StringRef();
}

ಶೂನ್ಯ ScalarTraits<uint16_t>::output(ಸ್ಥಿರಾಂಕ uint16_t &Val, ಶೂನ್ಯ *,
                                    raw_ostream &ಹೊರಹೋಗು) {
  ಹೊರಹೋಗು << Val;
}

StringRef ScalarTraits<uint16_t>::ಒಳಹರಿವು_2(StringRef Scalar, ಶೂನ್ಯ *,
                                        uint16_t &Val) {
  ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ ದೀರ್ಘ n;
  ಆದರೆ (getAsUnsignedInteger(Scalar, 0, n))
    ಹಿಂತಿರುಗಿ "invalid number";
  ಆದರೆ (n > 0xFFFF)
    ಹಿಂತಿರುಗಿ "out of range number";
  Val = n;
  ಹಿಂತಿರುಗಿ StringRef();
}

ಶೂನ್ಯ ScalarTraits<uint32_t>::output(ಸ್ಥಿರಾಂಕ uint32_t &Val, ಶೂನ್ಯ *,
                                    raw_ostream &ಹೊರಹೋಗು) {
  ಹೊರಹೋಗು << Val;
}

StringRef ScalarTraits<uint32_t>::ಒಳಹರಿವು_2(StringRef Scalar, ಶೂನ್ಯ *,
                                        uint32_t &Val) {
  ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ ದೀರ್ಘ n;
  ಆದರೆ (getAsUnsignedInteger(Scalar, 0, n))
    ಹಿಂತಿರುಗಿ "invalid number";
  ಆದರೆ (n > 0xFFFFFFFFUL)
    ಹಿಂತಿರುಗಿ "out of range number";
  Val = n;
  ಹಿಂತಿರುಗಿ StringRef();
}

ಶೂನ್ಯ ScalarTraits<uint64_t>::output(ಸ್ಥಿರಾಂಕ uint64_t &Val, ಶೂನ್ಯ *,
                                    raw_ostream &ಹೊರಹೋಗು) {
  ಹೊರಹೋಗು << Val;
}

StringRef ScalarTraits<uint64_t>::ಒಳಹರಿವು_2(StringRef Scalar, ಶೂನ್ಯ *,
                                        uint64_t &Val) {
  ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ ದೀರ್ಘ N;
  ಆದರೆ (getAsUnsignedInteger(Scalar, 0, N))
    ಹಿಂತಿರುಗಿ "invalid number";
  Val = N;
  ಹಿಂತಿರುಗಿ StringRef();
}

ಶೂನ್ಯ ScalarTraits<int8_t>::output(ಸ್ಥಿರಾಂಕ int8_t &Val, ಶೂನ್ಯ *, raw_ostream &ಹೊರಹೋಗು) {
  // use temp in32_t because ostream thinks int8_t is a character
  int32_t ಅಂಕೆ = Val;
  ಹೊರಹೋಗು << ಅಂಕೆ;
}

StringRef ScalarTraits<int8_t>::ಒಳಹರಿವು_2(StringRef Scalar, ಶೂನ್ಯ *, int8_t &Val) {
  ದೀರ್ಘ ದೀರ್ಘ N;
  ಆದರೆ (getAsSignedInteger(Scalar, 0, N))
    ಹಿಂತಿರುಗಿ "invalid number";
  ಆದರೆ ((N > 127) || (N < -128))
    ಹಿಂತಿರುಗಿ "out of range number";
  Val = N;
  ಹಿಂತಿರುಗಿ StringRef();
}

ಶೂನ್ಯ ScalarTraits<int16_t>::output(ಸ್ಥಿರಾಂಕ int16_t &Val, ಶೂನ್ಯ *,
                                   raw_ostream &ಹೊರಹೋಗು) {
  ಹೊರಹೋಗು << Val;
}

StringRef ScalarTraits<int16_t>::ಒಳಹರಿವು_2(StringRef Scalar, ಶೂನ್ಯ *, int16_t &Val) {
  ದೀರ್ಘ ದೀರ್ಘ N;
  ಆದರೆ (getAsSignedInteger(Scalar, 0, N))
    ಹಿಂತಿರುಗಿ "invalid number";
  ಆದರೆ ((N > INT16_MAX) || (N < INT16_MIN))
    ಹಿಂತಿರುಗಿ "out of range number";
  Val = N;
  ಹಿಂತಿರುಗಿ StringRef();
}

ಶೂನ್ಯ ScalarTraits<int32_t>::output(ಸ್ಥಿರಾಂಕ int32_t &Val, ಶೂನ್ಯ *,
                                   raw_ostream &ಹೊರಹೋಗು) {
  ಹೊರಹೋಗು << Val;
}

StringRef ScalarTraits<int32_t>::ಒಳಹರಿವು_2(StringRef Scalar, ಶೂನ್ಯ *, int32_t &Val) {
  ದೀರ್ಘ ದೀರ್ಘ N;
  ಆದರೆ (getAsSignedInteger(Scalar, 0, N))
    ಹಿಂತಿರುಗಿ "invalid number";
  ಆದರೆ ((N > INT32_MAX) || (N < INT32_MIN))
    ಹಿಂತಿರುಗಿ "out of range number";
  Val = N;
  ಹಿಂತಿರುಗಿ StringRef();
}

ಶೂನ್ಯ ScalarTraits<int64_t>::output(ಸ್ಥಿರಾಂಕ int64_t &Val, ಶೂನ್ಯ *,
                                   raw_ostream &ಹೊರಹೋಗು) {
  ಹೊರಹೋಗು << Val;
}

StringRef ScalarTraits<int64_t>::ಒಳಹರಿವು_2(StringRef Scalar, ಶೂನ್ಯ *, int64_t &Val) {
  ದೀರ್ಘ ದೀರ್ಘ N;
  ಆದರೆ (getAsSignedInteger(Scalar, 0, N))
    ಹಿಂತಿರುಗಿ "invalid number";
  Val = N;
  ಹಿಂತಿರುಗಿ StringRef();
}

ಶೂನ್ಯ ScalarTraits<ದ್ವಿಗುಣ>::output(ಸ್ಥಿರಾಂಕ ದ್ವಿಗುಣ &Val, ಶೂನ್ಯ *, raw_ostream &ಹೊರಹೋಗು) {
  ಹೊರಹೋಗು << format("%g", Val);
}

StringRef ScalarTraits<ದ್ವಿಗುಣ>::ಒಳಹರಿವು_2(StringRef Scalar, ಶೂನ್ಯ *, ದ್ವಿಗುಣ &Val) {
  ಆದರೆ (ಗಮ್ಯ_float(Scalar, Val))
    ಹಿಂತಿರುಗಿ StringRef();
  ಹಿಂತಿರುಗಿ "invalid floating point number";
}

ಶೂನ್ಯ ScalarTraits<ದಶಮಾಂಶ>::output(ಸ್ಥಿರಾಂಕ ದಶಮಾಂಶ &Val, ಶೂನ್ಯ *, raw_ostream &ಹೊರಹೋಗು) {
  ಹೊರಹೋಗು << format("%g", Val);
}

StringRef ScalarTraits<ದಶಮಾಂಶ>::ಒಳಹರಿವು_2(StringRef Scalar, ಶೂನ್ಯ *, ದಶಮಾಂಶ &Val) {
  ಆದರೆ (ಗಮ್ಯ_float(Scalar, Val))
    ಹಿಂತಿರುಗಿ StringRef();
  ಹಿಂತಿರುಗಿ "invalid floating point number";
}

ಶೂನ್ಯ ScalarTraits<Hex8>::output(ಸ್ಥಿರಾಂಕ Hex8 &Val, ಶೂನ್ಯ *, raw_ostream &ಹೊರಹೋಗು) {
  ಹೊರಹೋಗು << format("0x%" PRIX8, (uint8_t)Val);
}

StringRef ScalarTraits<Hex8>::ಒಳಹರಿವು_2(StringRef Scalar, ಶೂನ್ಯ *, Hex8 &Val) {
  ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ ದೀರ್ಘ n;
  ಆದರೆ (getAsUnsignedInteger(Scalar, 0, n))
    ಹಿಂತಿರುಗಿ "invalid hex8 number";
  ಆದರೆ (n > 0xFF)
    ಹಿಂತಿರುಗಿ "out of range hex8 number";
  Val = n;
  ಹಿಂತಿರುಗಿ StringRef();
}

ಶೂನ್ಯ ScalarTraits<Hex16>::output(ಸ್ಥಿರಾಂಕ Hex16 &Val, ಶೂನ್ಯ *, raw_ostream &ಹೊರಹೋಗು) {
  ಹೊರಹೋಗು << format("0x%" PRIX16, (uint16_t)Val);
}

StringRef ScalarTraits<Hex16>::ಒಳಹರಿವು_2(StringRef Scalar, ಶೂನ್ಯ *, Hex16 &Val) {
  ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ ದೀರ್ಘ n;
  ಆದರೆ (getAsUnsignedInteger(Scalar, 0, n))
    ಹಿಂತಿರುಗಿ "invalid hex16 number";
  ಆದರೆ (n > 0xFFFF)
    ಹಿಂತಿರುಗಿ "out of range hex16 number";
  Val = n;
  ಹಿಂತಿರುಗಿ StringRef();
}

ಶೂನ್ಯ ScalarTraits<Hex32>::output(ಸ್ಥಿರಾಂಕ Hex32 &Val, ಶೂನ್ಯ *, raw_ostream &ಹೊರಹೋಗು) {
  ಹೊರಹೋಗು << format("0x%" PRIX32, (uint32_t)Val);
}

StringRef ScalarTraits<Hex32>::ಒಳಹರಿವು_2(StringRef Scalar, ಶೂನ್ಯ *, Hex32 &Val) {
  ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ ದೀರ್ಘ n;
  ಆದರೆ (getAsUnsignedInteger(Scalar, 0, n))
    ಹಿಂತಿರುಗಿ "invalid hex32 number";
  ಆದರೆ (n > 0xFFFFFFFFUL)
    ಹಿಂತಿರುಗಿ "out of range hex32 number";
  Val = n;
  ಹಿಂತಿರುಗಿ StringRef();
}

ಶೂನ್ಯ ScalarTraits<Hex64>::output(ಸ್ಥಿರಾಂಕ Hex64 &Val, ಶೂನ್ಯ *, raw_ostream &ಹೊರಹೋಗು) {
  ಹೊರಹೋಗು << format("0x%" PRIX64, (uint64_t)Val);
}

StringRef ScalarTraits<Hex64>::ಒಳಹರಿವು_2(StringRef Scalar, ಶೂನ್ಯ *, Hex64 &Val) {
  ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ ದೀರ್ಘ ಅಂಕೆ;
  ಆದರೆ (getAsUnsignedInteger(Scalar, 0, ಅಂಕೆ))
    ಹಿಂತಿರುಗಿ "invalid hex64 number";
  Val = ಅಂಕೆ;
  ಹಿಂತಿರುಗಿ StringRef();
}

ಶೂನ್ಯ ScalarTraits<ಆವೃತ್ತಿ_Tuple>::output(ಸ್ಥಿರಾಂಕ ಆವೃತ್ತಿ_Tuple &Val, ಶೂನ್ಯ *,
                                        llvm::raw_ostream &ಹೊರಹೋಗು) {
  ಹೊರಹೋಗು << Val.getAsString();
}

StringRef ScalarTraits<ಆವೃತ್ತಿ_Tuple>::ಒಳಹರಿವು_2(StringRef Scalar, ಶೂನ್ಯ *,
                                            ಆವೃತ್ತಿ_Tuple &Val) {
  ಆದರೆ (Val.tryParse(Scalar))
    ಹಿಂತಿರುಗಿ "invalid version format";
  ಹಿಂತಿರುಗಿ StringRef();
}
