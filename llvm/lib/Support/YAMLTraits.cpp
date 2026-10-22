//===- lib/Support/YAMLTraits.cpp -----------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/YAMLTraits.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/ADT/STLExtras.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/ADT/SmallString.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/ADT/StringExtras.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/ADT/StringRef.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/ADT/Twine.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/Casting.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/Errc.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/ErrorHandling.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/Format.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/LineIterator.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/MemoryBuffer.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/VersionTuple.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/YAMLParser.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/raw_ostream.h"
#ਅਨ੍ਤਰ੍ਭਾਵ <cassert>
#ਅਨ੍ਤਰ੍ਭਾਵ <cstdint>
#ਅਨ੍ਤਰ੍ਭਾਵ <cstring>
#ਅਨ੍ਤਰ੍ਭਾਵ <string>
#ਅਨ੍ਤਰ੍ਭਾਵ <vector>

ਵਰਤੋਂ ਨਾਮਾਕਾਸ਼ llvm;
ਵਰਤੋਂ ਨਾਮਾਕਾਸ਼ yaml;

//===----------------------------------------------------------------------===//
//  IO
//===----------------------------------------------------------------------===//

IO::IO(ਖਾਲੀ *Context) : Ctxt(Context) {}

IO::~IO() = ਮੂਲ;

ਖਾਲੀ *IO::getContext() ਅਚਲ {
  ਵਾਪਸ Ctxt;
}

ਖਾਲੀ IO::ਸੈੱਟ_Context(ਖਾਲੀ *Context) {
  Ctxt = Context;
}

ਖਾਲੀ IO::ਸੈੱਟ_Allow_Unknown_ਕੁੰਜੀਆਂ(ਤਾਰਕਿਕ Allow) {
  llvm_unreachable("Only supported for Input");
}

//===----------------------------------------------------------------------===//
//ਨਿਵੇਸ਼
//===----------------------------------------------------------------------===//

ਨਿਵੇਸ਼::ਨਿਵੇਸ਼(StringRef ਨਿਵੇਸ਼_Content, ਖਾਲੀ *Ctxt,
             ਸਰੋਤ_Mgr::DiagHandlerTy DiagHandler, ਖਾਲੀ *DiagHandlerCtxt)
    : IO(Ctxt), Strm(ਨਵਾਂ ਧਾਰਾ(ਨਿਵੇਸ਼_Content, SrcMgr, ਝੂਠ, &EC)) {
  ਜੇ (DiagHandler)
    SrcMgr.ਸੈੱਟ_Diag_Handler(DiagHandler, DiagHandlerCtxt);
  DocIterator = Strm->begin();
}

ਨਿਵੇਸ਼::ਨਿਵੇਸ਼(ਸਮ੍ਰਿਤੀ_ਬਫਰ_Ref ਨਿਵੇਸ਼, ਖਾਲੀ *Ctxt,
             ਸਰੋਤ_Mgr::DiagHandlerTy DiagHandler, ਖਾਲੀ *DiagHandlerCtxt)
    : IO(Ctxt), Strm(ਨਵਾਂ ਧਾਰਾ(ਨਿਵੇਸ਼, SrcMgr, ਝੂਠ, &EC)) {
  ਜੇ (DiagHandler)
    SrcMgr.ਸੈੱਟ_Diag_Handler(DiagHandler, DiagHandlerCtxt);
  DocIterator = Strm->begin();
}

ਨਿਵੇਸ਼::~ਨਿਵੇਸ਼() = ਮੂਲ;

std::error_ਕੋਡ ਨਿਵੇਸ਼::error() { ਵਾਪਸ EC; }

ਤਾਰਕਿਕ ਨਿਵੇਸ਼::outputting() ਅਚਲ {
  ਵਾਪਸ ਝੂਠ;
}

ਤਾਰਕਿਕ ਨਿਵੇਸ਼::ਸੈੱਟ_Current_Document() {
  ਜੇ (DocIterator != Strm->ਸਿਰਾ()) {
    ਗੰਢ *N = DocIterator->get_ਜੜ੍ਹ();
    ਜੇ (!N) {
      EC = make_error_ਕੋਡ(errc::invalid_ਦਲੀਲ);
      ਵਾਪਸ ਝੂਠ;
    }

    ਜੇ (isa<Null_ਗੰਢ>(N)) {
      // Empty files are allowed and ignored
      ++DocIterator;
      ਵਾਪਸ ਸੈੱਟ_Current_Document();
    }
    releaseHNodeBuffers();
    ਸਿਖਰ_ਗੰਢ = createHNodes(N);
    Current_ਗੰਢ = ਸਿਖਰ_ਗੰਢ;
    ਵਾਪਸ ਸੱਚ;
  }
  ਵਾਪਸ ਝੂਠ;
}

ਤਾਰਕਿਕ ਨਿਵੇਸ਼::ਅਗਲਾ_Document() {
  ਵਾਪਸ ++DocIterator != Strm->ਸਿਰਾ();
}

ਅਚਲ ਗੰਢ *ਨਿਵੇਸ਼::get_Current_ਗੰਢ() ਅਚਲ {
  ਵਾਪਸ Current_ਗੰਢ ? Current_ਗੰਢ->ਗੰਢ_2 : ਖਾਲੀ_ਸੂਚਕ;
}

ਤਾਰਕਿਕ ਨਿਵੇਸ਼::map_ਟੈਗ(StringRef ਟੈਗ, ਤਾਰਕਿਕ Default) {
  // CurrentNode can be null if setCurrentDocument() was unable to
  // parse the document because it was invalid or empty.
  ਜੇ (!Current_ਗੰਢ)
    ਵਾਪਸ ਝੂਠ;

  std::string ਮਿਲਿਆ_ਟੈਗ = Current_ਗੰਢ->ਗੰਢ_2->get_Verbatim_ਟੈਗ();
  ਜੇ (ਮਿਲਿਆ_ਟੈਗ.ਖਾਲੀ_ਥਾਵਾਂ()) {
    // If no tag found and 'Tag' is the default, say it was found.
    ਵਾਪਸ Default;
  }
  // Return true iff found tag matches supplied tag.
  ਵਾਪਸ ਟੈਗ == ਮਿਲਿਆ_ਟੈਗ;
}

ਖਾਲੀ ਨਿਵੇਸ਼::beginMapping() {
  ਜੇ (EC)
    ਵਾਪਸ;
  // CurrentNode can be null if the document is empty.
  MapHNode *MN = dyn_cast_or_null<MapHNode>(Current_ਗੰਢ);
  ਜੇ (MN) {
    MN->Valid_ਕੁੰਜੀਆਂ.clear();
  }
}

std::ਸਦਿਸ਼<StringRef> ਨਿਵੇਸ਼::ਕੁੰਜੀਆਂ() {
  MapHNode *MN = dyn_cast<MapHNode>(Current_ਗੰਢ);
  std::ਸਦਿਸ਼<StringRef> Ret;
  ਜੇ (!MN) {
    ਸੈੱਟ_Error(Current_ਗੰਢ, "not a mapping");
    ਵਾਪਸ Ret;
  }
  ਲਈ (ਸਵੈਚਾਲਿਤ &P : MN->Mapping)
    Ret.ਧੱਕੋ_ਪਿੱਠ(P.ਪਹਿਲਾ_2());
  ਵਾਪਸ Ret;
}

ਤਾਰਕਿਕ ਨਿਵੇਸ਼::preflight_ਕੁੰਜੀ(StringRef ਕੁੰਜੀ, ਤਾਰਕਿਕ Required, ਤਾਰਕਿਕ, ਤਾਰਕਿਕ &UseDefault,
                         ਖਾਲੀ *&SaveInfo) {
  UseDefault = ਝੂਠ;
  ਜੇ (EC)
    ਵਾਪਸ ਝੂਠ;

  // CurrentNode is null for empty documents, which is an error in case required
  // nodes are present.
  ਜੇ (!Current_ਗੰਢ) {
    ਜੇ (Required)
      EC = make_error_ਕੋਡ(errc::invalid_ਦਲੀਲ);
    ਨਹੀਂਤਾਂ
      UseDefault = ਸੱਚ;
    ਵਾਪਸ ਝੂਠ;
  }

  MapHNode *MN = dyn_cast<MapHNode>(Current_ਗੰਢ);
  ਜੇ (!MN) {
    ਜੇ (Required || !isa<ਖਾਲੀ_ਥਾਵਾਂ_HNode>(Current_ਗੰਢ))
      ਸੈੱਟ_Error(Current_ਗੰਢ, "not a mapping");
    ਨਹੀਂਤਾਂ
      UseDefault = ਸੱਚ;
    ਵਾਪਸ ਝੂਠ;
  }
  MN->Valid_ਕੁੰਜੀਆਂ.ਧੱਕੋ_ਪਿੱਠ(ਕੁੰਜੀ.str());
  HNode *ਮੁੱਲ = MN->Mapping[ਕੁੰਜੀ].ਪਹਿਲਾ_2;
  ਜੇ (!ਮੁੱਲ) {
    ਜੇ (Required)
      ਸੈੱਟ_Error(Current_ਗੰਢ, Twine("missing required key '") + ਕੁੰਜੀ + "'");
    ਨਹੀਂਤਾਂ
      UseDefault = ਸੱਚ;
    ਵਾਪਸ ਝੂਠ;
  }
  SaveInfo = Current_ਗੰਢ;
  Current_ਗੰਢ = ਮੁੱਲ;
  ਵਾਪਸ ਸੱਚ;
}

ਖਾਲੀ ਨਿਵੇਸ਼::postflight_ਕੁੰਜੀ(ਖਾਲੀ *saveInfo) {
  Current_ਗੰਢ = reinterpret_cast<HNode *>(saveInfo);
}

ਖਾਲੀ ਨਿਵੇਸ਼::ਸਿਰਾ_Mapping() {
  ਜੇ (EC)
    ਵਾਪਸ;
  // CurrentNode can be null if the document is empty.
  MapHNode *MN = dyn_cast_or_null<MapHNode>(Current_ਗੰਢ);
  ਜੇ (!MN)
    ਵਾਪਸ;
  ਲਈ (ਅਚਲ ਸਵੈਚਾਲਿਤ &NN : MN->Mapping) {
    ਜੇ (!is_contained(MN->Valid_ਕੁੰਜੀਆਂ, NN.ਪਹਿਲਾ_2())) {
      ਅਚਲ SMRange &ReportLoc = NN.ਦੂਜਾ_2.ਦੂਜਾ_2;
      ਜੇ (!Allow_Unknown_ਕੁੰਜੀਆਂ) {
        ਸੈੱਟ_Error(ReportLoc, Twine("unknown key '") + NN.ਪਹਿਲਾ_2() + "'");
        ਰੋਕੋ;
      } ਨਹੀਂਤਾਂ
        reportWarning(ReportLoc, Twine("unknown key '") + NN.ਪਹਿਲਾ_2() + "'");
    }
  }
}

ਖਾਲੀ ਨਿਵੇਸ਼::beginFlowMapping() { beginMapping(); }

ਖਾਲੀ ਨਿਵੇਸ਼::ਸਿਰਾ_Flow_Mapping() { ਸਿਰਾ_Mapping(); }

ਬਿਨਾਨਿਸ਼ਾਨੀ ਨਿਵੇਸ਼::beginSequence() {
  ਜੇ (SequenceHNode *SQ = dyn_cast<SequenceHNode>(Current_ਗੰਢ))
    ਵਾਪਸ SQ->Entries.ਮਾਪ_2();
  ਜੇ (isa<ਖਾਲੀ_ਥਾਵਾਂ_HNode>(Current_ਗੰਢ))
    ਵਾਪਸ 0;
  // Treat case where there's a scalar "null" value as an empty sequence.
  ਜੇ (ScalarHNode *SN = dyn_cast<ScalarHNode>(Current_ਗੰਢ)) {
    ਜੇ (isNull(SN->ਮੁੱਲ_2()))
      ਵਾਪਸ 0;
  }
  // Any other type of HNode is an error.
  ਸੈੱਟ_Error(Current_ਗੰਢ, "not a sequence");
  ਵਾਪਸ 0;
}

ਖਾਲੀ ਨਿਵੇਸ਼::ਸਿਰਾ_Sequence() {
}

ਤਾਰਕਿਕ ਨਿਵੇਸ਼::preflightElement(ਬਿਨਾਨਿਸ਼ਾਨੀ ਸੂਚਕਾਂਕ_2, ਖਾਲੀ *&SaveInfo) {
  ਜੇ (EC)
    ਵਾਪਸ ਝੂਠ;
  ਜੇ (SequenceHNode *SQ = dyn_cast<SequenceHNode>(Current_ਗੰਢ)) {
    SaveInfo = Current_ਗੰਢ;
    Current_ਗੰਢ = SQ->Entries[ਸੂਚਕਾਂਕ_2];
    ਵਾਪਸ ਸੱਚ;
  }
  ਵਾਪਸ ਝੂਠ;
}

ਖਾਲੀ ਨਿਵੇਸ਼::postflightElement(ਖਾਲੀ *SaveInfo) {
  Current_ਗੰਢ = reinterpret_cast<HNode *>(SaveInfo);
}

ਬਿਨਾਨਿਸ਼ਾਨੀ ਨਿਵੇਸ਼::beginFlowSequence() { ਵਾਪਸ beginSequence(); }

ਤਾਰਕਿਕ ਨਿਵੇਸ਼::preflightFlowElement(ਬਿਨਾਨਿਸ਼ਾਨੀ ਸੂਚਕਾਂਕ, ਖਾਲੀ *&SaveInfo) {
  ਜੇ (EC)
    ਵਾਪਸ ਝੂਠ;
  ਜੇ (SequenceHNode *SQ = dyn_cast<SequenceHNode>(Current_ਗੰਢ)) {
    SaveInfo = Current_ਗੰਢ;
    Current_ਗੰਢ = SQ->Entries[ਸੂਚਕਾਂਕ];
    ਵਾਪਸ ਸੱਚ;
  }
  ਵਾਪਸ ਝੂਠ;
}

ਖਾਲੀ ਨਿਵੇਸ਼::postflightFlowElement(ਖਾਲੀ *SaveInfo) {
  Current_ਗੰਢ = reinterpret_cast<HNode *>(SaveInfo);
}

ਖਾਲੀ ਨਿਵੇਸ਼::ਸਿਰਾ_Flow_Sequence() {
}

ਖਾਲੀ ਨਿਵੇਸ਼::beginEnumScalar() {
  Scalar_ਮੇਲ_ਮਿਲਿਆ = ਝੂਠ;
}

ਤਾਰਕਿਕ ਨਿਵੇਸ਼::ਮੇਲ_Enum_Scalar(StringRef Str, ਤਾਰਕਿਕ) {
  ਜੇ (Scalar_ਮੇਲ_ਮਿਲਿਆ)
    ਵਾਪਸ ਝੂਠ;
  ਜੇ (ScalarHNode *SN = dyn_cast<ScalarHNode>(Current_ਗੰਢ)) {
    ਜੇ (SN->ਮੁੱਲ_2() == Str) {
      Scalar_ਮੇਲ_ਮਿਲਿਆ = ਸੱਚ;
      ਵਾਪਸ ਸੱਚ;
    }
  }
  ਵਾਪਸ ਝੂਠ;
}

ਤਾਰਕਿਕ ਨਿਵੇਸ਼::ਮੇਲ_Enum_Fallback() {
  ਜੇ (Scalar_ਮੇਲ_ਮਿਲਿਆ)
    ਵਾਪਸ ਝੂਠ;
  Scalar_ਮੇਲ_ਮਿਲਿਆ = ਸੱਚ;
  ਵਾਪਸ ਸੱਚ;
}

ਖਾਲੀ ਨਿਵੇਸ਼::ਸਿਰਾ_Enum_Scalar() {
  ਜੇ (!Scalar_ਮੇਲ_ਮਿਲਿਆ) {
    ਸੈੱਟ_Error(Current_ਗੰਢ, "unknown enumerated scalar");
  }
}

ਤਾਰਕਿਕ ਨਿਵੇਸ਼::begin_ਦੋਆਧਾਰੀ_ਅੰਕ_ਸੈੱਟ_Scalar(ਤਾਰਕਿਕ &DoClear) {
  ਦੋਆਧਾਰੀ_ਅੰਕ_ਮੁੱਲਾਂ_ਵਰਤਿਆ.clear();
  ਜੇ (SequenceHNode *SQ = dyn_cast<SequenceHNode>(Current_ਗੰਢ)) {
    ਦੋਆਧਾਰੀ_ਅੰਕ_ਮੁੱਲਾਂ_ਵਰਤਿਆ.resize(SQ->Entries.ਮਾਪ_2());
  } ਨਹੀਂਤਾਂ {
    ਸੈੱਟ_Error(Current_ਗੰਢ, "expected sequence of bit values");
  }
  DoClear = ਸੱਚ;
  ਵਾਪਸ ਸੱਚ;
}

ਤਾਰਕਿਕ ਨਿਵੇਸ਼::ਦੋਆਧਾਰੀ_ਅੰਕ_ਸੈੱਟ_ਮੇਲ(StringRef Str, ਤਾਰਕਿਕ) {
  ਜੇ (EC)
    ਵਾਪਸ ਝੂਠ;
  ਜੇ (SequenceHNode *SQ = dyn_cast<SequenceHNode>(Current_ਗੰਢ)) {
    ਬਿਨਾਨਿਸ਼ਾਨੀ ਸੂਚਕਾਂਕ_2 = 0;
    ਲਈ (ਸਵੈਚਾਲਿਤ &N : SQ->Entries) {
      ਜੇ (ScalarHNode *SN = dyn_cast<ScalarHNode>(N)) {
        ਜੇ (SN->ਮੁੱਲ_2() == Str) {
          ਦੋਆਧਾਰੀ_ਅੰਕ_ਮੁੱਲਾਂ_ਵਰਤਿਆ[ਸੂਚਕਾਂਕ_2] = ਸੱਚ;
          ਵਾਪਸ ਸੱਚ;
        }
      } ਨਹੀਂਤਾਂ {
        ਸੈੱਟ_Error(Current_ਗੰਢ, "unexpected scalar in sequence of bit values");
      }
      ++ਸੂਚਕਾਂਕ_2;
    }
  } ਨਹੀਂਤਾਂ {
    ਸੈੱਟ_Error(Current_ਗੰਢ, "expected sequence of bit values");
  }
  ਵਾਪਸ ਝੂਠ;
}

ਖਾਲੀ ਨਿਵੇਸ਼::ਸਿਰਾ_ਦੋਆਧਾਰੀ_ਅੰਕ_ਸੈੱਟ_Scalar() {
  ਜੇ (EC)
    ਵਾਪਸ;
  ਜੇ (SequenceHNode *SQ = dyn_cast<SequenceHNode>(Current_ਗੰਢ)) {
    ਪ੍ਰਤਿਜ੍ਞਾ(ਦੋਆਧਾਰੀ_ਅੰਕ_ਮੁੱਲਾਂ_ਵਰਤਿਆ.ਮਾਪ_2() == SQ->Entries.ਮਾਪ_2());
    ਲਈ (ਬਿਨਾਨਿਸ਼ਾਨੀ i = 0; i < SQ->Entries.ਮਾਪ_2(); ++i) {
      ਜੇ (!ਦੋਆਧਾਰੀ_ਅੰਕ_ਮੁੱਲਾਂ_ਵਰਤਿਆ[i]) {
        ਸੈੱਟ_Error(SQ->Entries[i], "unknown bit value");
        ਵਾਪਸ;
      }
    }
  }
}

ਖਾਲੀ ਨਿਵੇਸ਼::scalarString(StringRef &S, QuotingType) {
  ਜੇ (ScalarHNode *SN = dyn_cast<ScalarHNode>(Current_ਗੰਢ)) {
    S = SN->ਮੁੱਲ_2();
  } ਨਹੀਂਤਾਂ {
    ਸੈੱਟ_Error(Current_ਗੰਢ, "unexpected scalar");
  }
}

ਖਾਲੀ ਨਿਵੇਸ਼::ਖੰਡ_Scalar_String(StringRef &S) { scalarString(S, QuotingType::None); }

ਖਾਲੀ ਨਿਵੇਸ਼::scalar_ਟੈਗ(std::string &ਟੈਗ) {
  ਟੈਗ = Current_ਗੰਢ->ਗੰਢ_2->get_Verbatim_ਟੈਗ();
}

ਖਾਲੀ ਨਿਵੇਸ਼::ਸੈੱਟ_Error(HNode *hnode, ਅਚਲ Twine &ਸੁਨੇਹਾ_2) {
  ਪ੍ਰਤਿਜ੍ਞਾ(hnode && "HNode must not be NULL");
  ਸੈੱਟ_Error(hnode->ਗੰਢ_2, ਸੁਨੇਹਾ_2);
}

ਗੰਢ_ਕਿਸਮ ਨਿਵੇਸ਼::get_ਗੰਢ_ਕਿਸਮ() {
  ਜੇ (isa<ScalarHNode>(Current_ਗੰਢ))
    ਵਾਪਸ ਗੰਢ_ਕਿਸਮ::Scalar;
  ਨਹੀਂਤਾਂ ਜੇ (isa<MapHNode>(Current_ਗੰਢ))
    ਵਾਪਸ ਗੰਢ_ਕਿਸਮ::Map;
  ਨਹੀਂਤਾਂ ਜੇ (isa<SequenceHNode>(Current_ਗੰਢ))
    ਵਾਪਸ ਗੰਢ_ਕਿਸਮ::Sequence;
  llvm_unreachable("Unsupported node kind");
}

ਖਾਲੀ ਨਿਵੇਸ਼::ਸੈੱਟ_Error(ਗੰਢ *ਗੰਢ_3, ਅਚਲ Twine &ਸੁਨੇਹਾ_2) {
  Strm->printError(ਗੰਢ_3, ਸੁਨੇਹਾ_2);
  EC = make_error_ਕੋਡ(errc::invalid_ਦਲੀਲ);
}

ਖਾਲੀ ਨਿਵੇਸ਼::ਸੈੱਟ_Error(ਅਚਲ SMRange &range, ਅਚਲ Twine &ਸੁਨੇਹਾ_2) {
  Strm->printError(range, ਸੁਨੇਹਾ_2);
  EC = make_error_ਕੋਡ(errc::invalid_ਦਲੀਲ);
}

ਖਾਲੀ ਨਿਵੇਸ਼::reportWarning(HNode *hnode, ਅਚਲ Twine &ਸੁਨੇਹਾ_2) {
  ਪ੍ਰਤਿਜ੍ਞਾ(hnode && "HNode must not be NULL");
  Strm->printError(hnode->ਗੰਢ_2, ਸੁਨੇਹਾ_2, ਸਰੋਤ_Mgr::DK_Warning);
}

ਖਾਲੀ ਨਿਵੇਸ਼::reportWarning(ਗੰਢ *ਗੰਢ_3, ਅਚਲ Twine &ਸੁਨੇਹਾ_2) {
  Strm->printError(ਗੰਢ_3, ਸੁਨੇਹਾ_2, ਸਰੋਤ_Mgr::DK_Warning);
}

ਖਾਲੀ ਨਿਵੇਸ਼::reportWarning(ਅਚਲ SMRange &range, ਅਚਲ Twine &ਸੁਨੇਹਾ_2) {
  Strm->printError(range, ਸੁਨੇਹਾ_2, ਸਰੋਤ_Mgr::DK_Warning);
}

ਖਾਲੀ ਨਿਵੇਸ਼::releaseHNodeBuffers() {
  ਖਾਲੀ_ਥਾਵਾਂ_HNode_Allocator.DestroyAll();
  ScalarHNodeAllocator.DestroyAll();
  SequenceHNodeAllocator.DestroyAll();
  MapHNodeAllocator.DestroyAll();
}

ਖਾਲੀ ਨਿਵੇਸ਼::saveAliasHNode(ਗੰਢ *N, HNode *HN) {
  StringRef Anchor = N->getAnchor();
  ਜੇ (!Anchor.ਖਾਲੀ_ਥਾਵਾਂ())
    // YAML 1.2.2 - 3.2.2.2. Anchors and Aliases:
    //
    // An alias event refers to the most recent event in the serialization
    // having the specified anchor. Therefore, anchors need not be unique within
    // a serialization. In addition, an anchor need not have an alias node
    // referring to it.
    AliasMap[Anchor] = HN;
}

ਨਿਵੇਸ਼::HNode *ਨਿਵੇਸ਼::createHNodes(ਗੰਢ *N) {
  SmallString<128> StringStorage;
  ਚੋਣ (N->getType()) {
  ਹਾਲਤ ਗੰਢ::NK_Scalar: {
    Scalar_ਗੰਢ *SN = dyn_cast<Scalar_ਗੰਢ>(N);
    StringRef ਕੁੰਜੀ_Str = SN->get_ਮੁੱਲ(StringStorage);
    ਜੇ (!StringStorage.ਖਾਲੀ_ਥਾਵਾਂ()) {
      // Copy string to permanent storage
      ਕੁੰਜੀ_Str = StringStorage.str().copy(StringAllocator);
    }
    ਸਵੈਚਾਲਿਤ *SHNode = ਨਵਾਂ (ScalarHNodeAllocator.Allocate()) ScalarHNode(N, ਕੁੰਜੀ_Str);
    saveAliasHNode(SN, SHNode);
    ਵਾਪਸ SHNode;
  }
  ਹਾਲਤ ਗੰਢ::NK_ਖੰਡ_Scalar: {
    ਖੰਡ_Scalar_ਗੰਢ *BSN = dyn_cast<ਖੰਡ_Scalar_ਗੰਢ>(N);
    StringRef ਮੁੱਲ_Copy = BSN->get_ਮੁੱਲ().copy(StringAllocator);
    ਸਵੈਚਾਲਿਤ *BSHNode =
        ਨਵਾਂ (ScalarHNodeAllocator.Allocate()) ScalarHNode(N, ਮੁੱਲ_Copy);
    saveAliasHNode(BSN, BSHNode);
    ਵਾਪਸ BSHNode;
  }
  ਹਾਲਤ ਗੰਢ::NK_Sequence: {
    Sequence_ਗੰਢ *SQ = dyn_cast<Sequence_ਗੰਢ>(N);
    ਸਵੈਚਾਲਿਤ SQHNode = ਨਵਾਂ (SequenceHNodeAllocator.Allocate()) SequenceHNode(N);
    ਲਈ (ਗੰਢ &SN : *SQ) {
      ਸਵੈਚਾਲਿਤ ਇੰਦਰਾਜ = createHNodes(&SN);
      ਜੇ (EC)
        ਰੋਕੋ;
      SQHNode->Entries.ਧੱਕੋ_ਪਿੱਠ(ਇੰਦਰਾਜ);
    }
    saveAliasHNode(SQ, SQHNode);
    ਵਾਪਸ SQHNode;
  }
  ਹਾਲਤ ਗੰਢ::NK_Mapping: {
    Mapping_ਗੰਢ *Map = dyn_cast<Mapping_ਗੰਢ>(N);
    ਸਵੈਚਾਲਿਤ mapHNode = ਨਵਾਂ (MapHNodeAllocator.Allocate()) MapHNode(N);
    ਲਈ (ਕੁੰਜੀ_ਮੁੱਲ_ਗੰਢ &KVN : *Map) {
      ਗੰਢ *ਕੁੰਜੀ_ਗੰਢ = KVN.get_ਕੁੰਜੀ();
      Scalar_ਗੰਢ *ਕੁੰਜੀ = dyn_cast_if_present<Scalar_ਗੰਢ>(ਕੁੰਜੀ_ਗੰਢ);
      ਗੰਢ *ਮੁੱਲ = KVN.get_ਮੁੱਲ();
      ਜੇ (!ਕੁੰਜੀ || !ਮੁੱਲ) {
        ਜੇ (!ਕੁੰਜੀ)
          ਸੈੱਟ_Error(ਕੁੰਜੀ_ਗੰਢ, "Map key must be a scalar");
        ਜੇ (!ਮੁੱਲ)
          ਸੈੱਟ_Error(ਕੁੰਜੀ_ਗੰਢ, "Map value must not be empty");
        ਰੋਕੋ;
      }
      StringStorage.clear();
      StringRef ਕੁੰਜੀ_Str = ਕੁੰਜੀ->get_ਮੁੱਲ(StringStorage);
      ਜੇ (!StringStorage.ਖਾਲੀ_ਥਾਵਾਂ()) {
        // Copy string to permanent storage
        ਕੁੰਜੀ_Str = StringStorage.str().copy(StringAllocator);
      }
      ਜੇ (mapHNode->Mapping.ਗਿਣਤੀ_2(ਕੁੰਜੀ_Str))
        // From YAML spec: "The content of a mapping node is an unordered set of
        // key/value node pairs, with the restriction that each of the keys is
        // unique."
        ਸੈੱਟ_Error(ਕੁੰਜੀ_ਗੰਢ, Twine("duplicated mapping key '") + ਕੁੰਜੀ_Str + "'");
      ਸਵੈਚਾਲਿਤ ਮੁੱਲ_HNode = createHNodes(ਮੁੱਲ);
      ਜੇ (EC)
        ਰੋਕੋ;
      mapHNode->Mapping[ਕੁੰਜੀ_Str] =
          std::make_pair(std::move(ਮੁੱਲ_HNode), ਕੁੰਜੀ_ਗੰਢ->get_ਸਰੋਤ_Range());
    }
    saveAliasHNode(Map, mapHNode);
    ਵਾਪਸ std::move(mapHNode);
  }
  ਹਾਲਤ ਗੰਢ::NK_Null:
    // TODO: Anchor is not set for NullNode in the parser. Update the parser to
    // faithfully preserve anchors.
    ਵਾਪਸ ਨਵਾਂ (ਖਾਲੀ_ਥਾਵਾਂ_HNode_Allocator.Allocate()) ਖਾਲੀ_ਥਾਵਾਂ_HNode(N);
  ਹਾਲਤ ਗੰਢ::NK_Alias: {
    Alias_ਗੰਢ *AN = dyn_cast<Alias_ਗੰਢ>(N);
    ਸਵੈਚਾਲਿਤ AliasName = AN->getName();
    ਸਵੈਚਾਲਿਤ AHN = AliasMap.ਲੱਭੋ(AliasName);
    ਜੇ (AHN == AliasMap.ਸਿਰਾ()) {
      ਸੈੱਟ_Error(AN, Twine("undefined alias '" + AliasName + "'"));
      ਵਾਪਸ ਖਾਲੀ_ਸੂਚਕ;
    }
    ਵਾਪਸ AHN->ਦੂਜਾ_2;
  }
  ਮੂਲ:
    ਸੈੱਟ_Error(N, "unknown node kind");
    ਵਾਪਸ ਖਾਲੀ_ਸੂਚਕ;
  }
}

ਖਾਲੀ ਨਿਵੇਸ਼::ਸੈੱਟ_Error(ਅਚਲ Twine &ਸੁਨੇਹਾ_3) {
  ਸੈੱਟ_Error(Current_ਗੰਢ, ਸੁਨੇਹਾ_3);
}

ਖਾਲੀ ਨਿਵੇਸ਼::ਸੈੱਟ_Allow_Unknown_ਕੁੰਜੀਆਂ(ਤਾਰਕਿਕ Allow) { Allow_Unknown_ਕੁੰਜੀਆਂ = Allow; }

ਤਾਰਕਿਕ ਨਿਵੇਸ਼::can_Elide_ਖਾਲੀ_ਥਾਵਾਂ_Sequence() {
  ਵਾਪਸ ਝੂਠ;
}

//===----------------------------------------------------------------------===//
//  Output
//===----------------------------------------------------------------------===//

Output::Output(raw_ostream &yout, ਖਾਲੀ *context, ਪੂਰਨ Wrap_ਕਾਲਮ)
    : IO(context), ਬਾਹਰਲਾ(yout), Wrap_ਕਾਲਮ(Wrap_ਕਾਲਮ) {}

Output::~Output() = ਮੂਲ;

ਤਾਰਕਿਕ Output::outputting() ਅਚਲ {
  ਵਾਪਸ ਸੱਚ;
}

ਖਾਲੀ Output::beginMapping() {
  State_ਢੇਰ.ਧੱਕੋ_ਪਿੱਠ(ਵਿੱਚ_Map_ਪਹਿਲਾ_ਕੁੰਜੀ);
  PaddingBeforeContainer = Padding;
  Padding = "\n";
}

ਤਾਰਕਿਕ Output::map_ਟੈਗ(StringRef ਟੈਗ, ਤਾਰਕਿਕ Use) {
  ਜੇ (Use) {
    // If this tag is being written inside a sequence we should write the start
    // of the sequence before writing the tag, otherwise the tag won't be
    // attached to the element in the sequence, but rather the sequence itself.
    ਤਾਰਕਿਕ SequenceElement = ਝੂਠ;
    ਜੇ (State_ਢੇਰ.ਮਾਪ_2() > 1) {
      ਸਵੈਚਾਲਿਤ &E = State_ਢੇਰ[State_ਢੇਰ.ਮਾਪ_2() - 2];
      SequenceElement = ਵਿੱਚ_Seq_Any_Element(E) || ਵਿੱਚ_Flow_Seq_Any_Element(E);
    }
    ਜੇ (SequenceElement && State_ਢੇਰ.ਪਿੱਠ() == ਵਿੱਚ_Map_ਪਹਿਲਾ_ਕੁੰਜੀ) {
      new_ਸਤਰ_Check();
    } ਨਹੀਂਤਾਂ {
      output(" ");
    }
    output(ਟੈਗ);
    ਜੇ (SequenceElement) {
      // If we're writing the tag during the first element of a map, the tag
      // takes the place of the first element in the sequence.
      ਜੇ (State_ਢੇਰ.ਪਿੱਠ() == ਵਿੱਚ_Map_ਪਹਿਲਾ_ਕੁੰਜੀ) {
        State_ਢੇਰ.ਕੱਢੋ_ਪਿੱਠ();
        State_ਢੇਰ.ਧੱਕੋ_ਪਿੱਠ(ਵਿੱਚ_Map_Other_ਕੁੰਜੀ);
      }
      // Tags inside maps in sequences should act as keys in the map from a
      // formatting perspective, so we always want a newline in a sequence.
      Padding = "\n";
    }
  }
  ਵਾਪਸ Use;
}

ਖਾਲੀ Output::ਸਿਰਾ_Mapping() {
  // If we did not map anything, we should explicitly emit an empty map
  ਜੇ (State_ਢੇਰ.ਪਿੱਠ() == ਵਿੱਚ_Map_ਪਹਿਲਾ_ਕੁੰਜੀ) {
    Padding = PaddingBeforeContainer;
    new_ਸਤਰ_Check();
    output("{}");
    Padding = "\n";
  }
  State_ਢੇਰ.ਕੱਢੋ_ਪਿੱਠ();
}

std::ਸਦਿਸ਼<StringRef> Output::ਕੁੰਜੀਆਂ() {
  report_fatal_error("invalid call");
}

ਤਾਰਕਿਕ Output::preflight_ਕੁੰਜੀ(StringRef ਕੁੰਜੀ, ਤਾਰਕਿਕ Required, ਤਾਰਕਿਕ SameAsDefault,
                          ਤਾਰਕਿਕ &UseDefault, ਖਾਲੀ *&SaveInfo) {
  UseDefault = ਝੂਠ;
  SaveInfo = ਖਾਲੀ_ਸੂਚਕ;
  ਜੇ (Required || !SameAsDefault || Write_Default_ਮੁੱਲਾਂ) {
    ਸਵੈਚਾਲਿਤ State = State_ਢੇਰ.ਪਿੱਠ();
    ਜੇ (State == ਵਿੱਚ_Flow_Map_ਪਹਿਲਾ_ਕੁੰਜੀ || State == ਵਿੱਚ_Flow_Map_Other_ਕੁੰਜੀ) {
      flow_ਕੁੰਜੀ(ਕੁੰਜੀ);
    } ਨਹੀਂਤਾਂ {
      new_ਸਤਰ_Check();
      padded_ਕੁੰਜੀ(ਕੁੰਜੀ);
    }
    ਵਾਪਸ ਸੱਚ;
  }
  ਵਾਪਸ ਝੂਠ;
}

ਖਾਲੀ Output::postflight_ਕੁੰਜੀ(ਖਾਲੀ *) {
  ਜੇ (State_ਢੇਰ.ਪਿੱਠ() == ਵਿੱਚ_Map_ਪਹਿਲਾ_ਕੁੰਜੀ) {
    State_ਢੇਰ.ਕੱਢੋ_ਪਿੱਠ();
    State_ਢੇਰ.ਧੱਕੋ_ਪਿੱਠ(ਵਿੱਚ_Map_Other_ਕੁੰਜੀ);
  } ਨਹੀਂਤਾਂ ਜੇ (State_ਢੇਰ.ਪਿੱਠ() == ਵਿੱਚ_Flow_Map_ਪਹਿਲਾ_ਕੁੰਜੀ) {
    State_ਢੇਰ.ਕੱਢੋ_ਪਿੱਠ();
    State_ਢੇਰ.ਧੱਕੋ_ਪਿੱਠ(ਵਿੱਚ_Flow_Map_Other_ਕੁੰਜੀ);
  }
}

ਖਾਲੀ Output::beginFlowMapping() {
  State_ਢੇਰ.ਧੱਕੋ_ਪਿੱਠ(ਵਿੱਚ_Flow_Map_ਪਹਿਲਾ_ਕੁੰਜੀ);
  new_ਸਤਰ_Check();
  ਕਾਲਮ_At_Map_Flow_ਸ਼ੁਰੂ = ਕਾਲਮ;
  output("{ ");
}

ਖਾਲੀ Output::ਸਿਰਾ_Flow_Mapping() {
  State_ਢੇਰ.ਕੱਢੋ_ਪਿੱਠ();
  output_ਉੱਪਰ_ਮੰਜ਼ਿਲ_ਸਿਰਾ_ਦਾ_ਸਤਰ(" }");
}

ਖਾਲੀ Output::beginDocuments() {
  output_ਉੱਪਰ_ਮੰਜ਼ਿਲ_ਸਿਰਾ_ਦਾ_ਸਤਰ("---");
}

ਤਾਰਕਿਕ Output::preflightDocument(ਬਿਨਾਨਿਸ਼ਾਨੀ ਸੂਚਕਾਂਕ) {
  ਜੇ (ਸੂਚਕਾਂਕ > 0)
    output_ਉੱਪਰ_ਮੰਜ਼ਿਲ_ਸਿਰਾ_ਦਾ_ਸਤਰ("\n---");
  ਵਾਪਸ ਸੱਚ;
}

ਖਾਲੀ Output::postflightDocument() {
}

ਖਾਲੀ Output::ਸਿਰਾ_Documents() {
  output("\n...\n");
}

ਬਿਨਾਨਿਸ਼ਾਨੀ Output::beginSequence() {
  State_ਢੇਰ.ਧੱਕੋ_ਪਿੱਠ(ਵਿੱਚ_Seq_ਪਹਿਲਾ_Element);
  PaddingBeforeContainer = Padding;
  Padding = "\n";
  ਵਾਪਸ 0;
}

ਖਾਲੀ Output::ਸਿਰਾ_Sequence() {
  // If we did not emit anything, we should explicitly emit an empty sequence
  ਜੇ (State_ਢੇਰ.ਪਿੱਠ() == ਵਿੱਚ_Seq_ਪਹਿਲਾ_Element) {
    Padding = PaddingBeforeContainer;
    new_ਸਤਰ_Check(/*EmptySequence=*/ਸੱਚ);
    output("[]");
    Padding = "\n";
  }
  State_ਢੇਰ.ਕੱਢੋ_ਪਿੱਠ();
}

ਤਾਰਕਿਕ Output::preflightElement(ਬਿਨਾਨਿਸ਼ਾਨੀ, ਖਾਲੀ *&SaveInfo) {
  SaveInfo = ਖਾਲੀ_ਸੂਚਕ;
  ਵਾਪਸ ਸੱਚ;
}

ਖਾਲੀ Output::postflightElement(ਖਾਲੀ *) {
  ਜੇ (State_ਢੇਰ.ਪਿੱਠ() == ਵਿੱਚ_Seq_ਪਹਿਲਾ_Element) {
    State_ਢੇਰ.ਕੱਢੋ_ਪਿੱਠ();
    State_ਢੇਰ.ਧੱਕੋ_ਪਿੱਠ(ਵਿੱਚ_Seq_Other_Element);
  } ਨਹੀਂਤਾਂ ਜੇ (State_ਢੇਰ.ਪਿੱਠ() == ਵਿੱਚ_Flow_Seq_ਪਹਿਲਾ_Element) {
    State_ਢੇਰ.ਕੱਢੋ_ਪਿੱਠ();
    State_ਢੇਰ.ਧੱਕੋ_ਪਿੱਠ(ਵਿੱਚ_Flow_Seq_Other_Element);
  }
}

ਬਿਨਾਨਿਸ਼ਾਨੀ Output::beginFlowSequence() {
  State_ਢੇਰ.ਧੱਕੋ_ਪਿੱਠ(ਵਿੱਚ_Flow_Seq_ਪਹਿਲਾ_Element);
  new_ਸਤਰ_Check();
  ਕਾਲਮ_At_Flow_ਸ਼ੁਰੂ = ਕਾਲਮ;
  output("[ ");
  NeedFlowSequenceComma = ਝੂਠ;
  ਵਾਪਸ 0;
}

ਖਾਲੀ Output::ਸਿਰਾ_Flow_Sequence() {
  State_ਢੇਰ.ਕੱਢੋ_ਪਿੱਠ();
  output_ਉੱਪਰ_ਮੰਜ਼ਿਲ_ਸਿਰਾ_ਦਾ_ਸਤਰ(" ]");
}

ਤਾਰਕਿਕ Output::preflightFlowElement(ਬਿਨਾਨਿਸ਼ਾਨੀ, ਖਾਲੀ *&SaveInfo) {
  ਜੇ (NeedFlowSequenceComma)
    output(", ");
  ਜੇ (Wrap_ਕਾਲਮ && ਕਾਲਮ > Wrap_ਕਾਲਮ) {
    output("\n");
    ਲਈ (ਪੂਰਨ i = 0; i < ਕਾਲਮ_At_Flow_ਸ਼ੁਰੂ; ++i)
      output(" ");
    ਕਾਲਮ = ਕਾਲਮ_At_Flow_ਸ਼ੁਰੂ;
    output("  ");
  }
  SaveInfo = ਖਾਲੀ_ਸੂਚਕ;
  ਵਾਪਸ ਸੱਚ;
}

ਖਾਲੀ Output::postflightFlowElement(ਖਾਲੀ *) {
  NeedFlowSequenceComma = ਸੱਚ;
}

ਖਾਲੀ Output::beginEnumScalar() {
  Enumeration_ਮੇਲ_ਮਿਲਿਆ = ਝੂਠ;
}

ਤਾਰਕਿਕ Output::ਮੇਲ_Enum_Scalar(StringRef Str, ਤਾਰਕਿਕ ਮੇਲ_2) {
  ਜੇ (ਮੇਲ_2 && !Enumeration_ਮੇਲ_ਮਿਲਿਆ) {
    new_ਸਤਰ_Check();
    output_ਉੱਪਰ_ਮੰਜ਼ਿਲ_ਸਿਰਾ_ਦਾ_ਸਤਰ(Str);
    Enumeration_ਮੇਲ_ਮਿਲਿਆ = ਸੱਚ;
  }
  ਵਾਪਸ ਝੂਠ;
}

ਤਾਰਕਿਕ Output::ਮੇਲ_Enum_Fallback() {
  ਜੇ (Enumeration_ਮੇਲ_ਮਿਲਿਆ)
    ਵਾਪਸ ਝੂਠ;
  Enumeration_ਮੇਲ_ਮਿਲਿਆ = ਸੱਚ;
  ਵਾਪਸ ਸੱਚ;
}

ਖਾਲੀ Output::ਸਿਰਾ_Enum_Scalar() {
  ਜੇ (!Enumeration_ਮੇਲ_ਮਿਲਿਆ)
    llvm_unreachable("bad runtime enum value");
}

ਤਾਰਕਿਕ Output::begin_ਦੋਆਧਾਰੀ_ਅੰਕ_ਸੈੱਟ_Scalar(ਤਾਰਕਿਕ &DoClear) {
  new_ਸਤਰ_Check();
  output("[ ");
  Need_ਦੋਆਧਾਰੀ_ਅੰਕ_ਮੁੱਲ_Comma = ਝੂਠ;
  DoClear = ਝੂਠ;
  ਵਾਪਸ ਸੱਚ;
}

ਤਾਰਕਿਕ Output::ਦੋਆਧਾਰੀ_ਅੰਕ_ਸੈੱਟ_ਮੇਲ(StringRef Str, ਤਾਰਕਿਕ Matches) {
  ਜੇ (Matches) {
    ਜੇ (Need_ਦੋਆਧਾਰੀ_ਅੰਕ_ਮੁੱਲ_Comma)
      output(", ");
    output(Str);
    Need_ਦੋਆਧਾਰੀ_ਅੰਕ_ਮੁੱਲ_Comma = ਸੱਚ;
  }
  ਵਾਪਸ ਝੂਠ;
}

ਖਾਲੀ Output::ਸਿਰਾ_ਦੋਆਧਾਰੀ_ਅੰਕ_ਸੈੱਟ_Scalar() {
  output_ਉੱਪਰ_ਮੰਜ਼ਿਲ_ਸਿਰਾ_ਦਾ_ਸਤਰ(" ]");
}

ਖਾਲੀ Output::scalarString(StringRef &S, QuotingType MustQuote) {
  new_ਸਤਰ_Check();
  ਜੇ (S.ਖਾਲੀ_ਥਾਵਾਂ()) {
    // Print '' for the empty string because leaving the field empty is not
    // allowed.
    output_ਉੱਪਰ_ਮੰਜ਼ਿਲ_ਸਿਰਾ_ਦਾ_ਸਤਰ("''");
    ਵਾਪਸ;
  }
  output(S, MustQuote);
  output_ਉੱਪਰ_ਮੰਜ਼ਿਲ_ਸਿਰਾ_ਦਾ_ਸਤਰ("");
}

ਖਾਲੀ Output::ਖੰਡ_Scalar_String(StringRef &S) {
  ਜੇ (!State_ਢੇਰ.ਖਾਲੀ_ਥਾਵਾਂ())
    new_ਸਤਰ_Check();
  output(" |");

  ਬਿਨਾਨਿਸ਼ਾਨੀ Indent = State_ਢੇਰ.ਖਾਲੀ_ਥਾਵਾਂ() ? 1 : State_ਢੇਰ.ਮਾਪ_2();

  ਸਵੈਚਾਲਿਤ ਬਫਰ = ਸਮ੍ਰਿਤੀ_ਬਫਰ::get_Mem_ਬਫਰ(S, "", ਝੂਠ);
  ਲਈ (ਸਤਰ_iterator ਸਤਰਾਂ(*ਬਫਰ, ਝੂਠ); !ਸਤਰਾਂ.is_at_ਸਿਰਾ(); ++ਸਤਰਾਂ) {
    output_New_ਸਤਰ();
    ਲਈ (ਬਿਨਾਨਿਸ਼ਾਨੀ I = 0; I < Indent; ++I) {
      output("  ");
    }
    output(*ਸਤਰਾਂ);
  }
  output_ਉੱਪਰ_ਮੰਜ਼ਿਲ_ਸਿਰਾ_ਦਾ_ਸਤਰ("");
}

ਖਾਲੀ Output::scalar_ਟੈਗ(std::string &ਟੈਗ) {
  ਜੇ (ਟੈਗ.ਖਾਲੀ_ਥਾਵਾਂ())
    ਵਾਪਸ;
  new_ਸਤਰ_Check();
  output(ਟੈਗ);
  output(" ");
}

ਖਾਲੀ Output::ਸੈੱਟ_Error(ਅਚਲ Twine &ਸੁਨੇਹਾ_2) {
}

std::error_ਕੋਡ Output::error() { ਵਾਪਸ {}; }

ਤਾਰਕਿਕ Output::can_Elide_ਖਾਲੀ_ਥਾਵਾਂ_Sequence() {
  // Normally, with an optional key/value where the value is an empty sequence,
  // the whole key/value can be not written.  But, that produces wrong yaml
  // if the key/value is the only thing in the map and the map is used in
  // a sequence.  This detects if the this sequence is the first key/value
  // in map that itself is embedded in a sequence.
  ਜੇ (State_ਢੇਰ.ਮਾਪ_2() < 2)
    ਵਾਪਸ ਸੱਚ;
  ਜੇ (State_ਢੇਰ.ਪਿੱਠ() != ਵਿੱਚ_Map_ਪਹਿਲਾ_ਕੁੰਜੀ)
    ਵਾਪਸ ਸੱਚ;
  ਵਾਪਸ !ਵਿੱਚ_Seq_Any_Element(State_ਢੇਰ[State_ਢੇਰ.ਮਾਪ_2() - 2]);
}

ਖਾਲੀ Output::output(StringRef s) {
  ਕਾਲਮ += s.ਮਾਪ_2();
  ਬਾਹਰਲਾ << s;
}

ਖਾਲੀ Output::output(StringRef S, QuotingType MustQuote) {
  ਜੇ (MustQuote == QuotingType::None) {
    // Only quote if we must.
    output(S);
    ਵਾਪਸ;
  }

  StringLiteral Quote = MustQuote == QuotingType::Single ? StringLiteral("'")
                                                         : StringLiteral("\"");
  output(Quote); // Starting quote.

  // When using double-quoted strings (and only in that case), non-printable
  // characters may be present, and will be escaped using a variety of
  // unicode-scalar and special short-form escapes. This is handled in
  // yaml::escape.
  ਜੇ (MustQuote == QuotingType::Double) {
    output(yaml::escape(S, /* EscapePrintable= */ ਝੂਠ));
    output(Quote);
    ਵਾਪਸ;
  }

  ਬਿਨਾਨਿਸ਼ਾਨੀ i = 0;
  ਬਿਨਾਨਿਸ਼ਾਨੀ j = 0;
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਸਿਰਾ_2 = S.ਮਾਪ_2();
  ਅਚਲ ਅੱਖਰ *ਅਧਾਰ = S.ਅੰਕੜੇ();

  // When using single-quoted strings, any single quote ' must be doubled to be
  // escaped.
  ਜਦੋਂ (j < ਸਿਰਾ_2) {
    ਜੇ (S[j] == '\'') {                   // Escape quotes.
      output(StringRef(&ਅਧਾਰ[i], j - i)); // "flush".
      output(StringLiteral("''"));        // Print it as ''
      i = j + 1;
    }
    ++j;
  }
  output(StringRef(&ਅਧਾਰ[i], j - i));
  output(Quote); // Ending quote.
}

ਖਾਲੀ Output::output_ਉੱਪਰ_ਮੰਜ਼ਿਲ_ਸਿਰਾ_ਦਾ_ਸਤਰ(StringRef s) {
  output(s);
  ਜੇ (State_ਢੇਰ.ਖਾਲੀ_ਥਾਵਾਂ() || (!ਵਿੱਚ_Flow_Seq_Any_Element(State_ਢੇਰ.ਪਿੱਠ()) &&
                             !ਵਿੱਚ_Flow_Map_Any_ਕੁੰਜੀ(State_ਢੇਰ.ਪਿੱਠ())))
    Padding = "\n";
}

ਖਾਲੀ Output::output_New_ਸਤਰ() {
  ਬਾਹਰਲਾ << "\n";
  ਕਾਲਮ = 0;
}

// if seq at top, indent as if map, then add "- "
// if seq in middle, use "- " if firstKey, else use "  "
//

ਖਾਲੀ Output::new_ਸਤਰ_Check(ਤਾਰਕਿਕ ਖਾਲੀ_ਥਾਵਾਂ_Sequence) {
  ਜੇ (Padding != "\n") {
    output(Padding);
    Padding = {};
    ਵਾਪਸ;
  }
  output_New_ਸਤਰ();
  Padding = {};

  ਜੇ (State_ਢੇਰ.ਮਾਪ_2() == 0 || ਖਾਲੀ_ਥਾਵਾਂ_Sequence)
    ਵਾਪਸ;

  ਬਿਨਾਨਿਸ਼ਾਨੀ Indent = State_ਢੇਰ.ਮਾਪ_2() - 1;
  ਤਾਰਕਿਕ PossiblyNestedSeq = ਝੂਠ;
  ਸਵੈਚਾਲਿਤ I = State_ਢੇਰ.rbegin(), E = State_ਢੇਰ.rend();

  ਜੇ (ਵਿੱਚ_Seq_Any_Element(*I)) {
    PossiblyNestedSeq = ਸੱਚ; // Not possibly but always.
    ++Indent;
  } ਨਹੀਂਤਾਂ ਜੇ (*I == ਵਿੱਚ_Map_ਪਹਿਲਾ_ਕੁੰਜੀ || *I == ਵਿੱਚ_Flow_Map_ਪਹਿਲਾ_ਕੁੰਜੀ ||
             ਵਿੱਚ_Flow_Seq_Any_Element(*I)) {
    PossiblyNestedSeq = ਸੱਚ;
    ++I; // Skip back().
  }

  ਬਿਨਾਨਿਸ਼ਾਨੀ Output_Dash_ਗਿਣਤੀ = 0;
  ਜੇ (PossiblyNestedSeq) {
    // Count up consecutive inSeqFirstElement from the end, unless
    // inSeqFirstElement is the top of nested sequence.
    ਜਦੋਂ (I != E) {
      // Don't count the top of nested sequence.
      ਜੇ (!ਵਿੱਚ_Seq_Any_Element(*I))
        ਰੋਕੋ;

      ++Output_Dash_ਗਿਣਤੀ;

      // Stop counting if consecutive inSeqFirstElement ends.
      ਜੇ (*I++ != ਵਿੱਚ_Seq_ਪਹਿਲਾ_Element)
        ਰੋਕੋ;
    }
  }

  ਲਈ (ਬਿਨਾਨਿਸ਼ਾਨੀ I = Output_Dash_ਗਿਣਤੀ; I < Indent; ++I)
    output("  ");

  ਲਈ (ਬਿਨਾਨਿਸ਼ਾਨੀ I = 0; I < Output_Dash_ਗਿਣਤੀ; ++I)
    output("- ");
}

ਖਾਲੀ Output::padded_ਕੁੰਜੀ(StringRef ਕੁੰਜੀ_2) {
  output(ਕੁੰਜੀ_2, needsQuotes(ਕੁੰਜੀ_2, ਝੂਠ));
  output(":");
  ਅਚਲ ਅੱਖਰ *spaces = "                ";
  ਜੇ (ਕੁੰਜੀ_2.ਮਾਪ_2() < ਲੜੀ_ਲੰਬਾਈ(spaces))
    Padding = &spaces[ਕੁੰਜੀ_2.ਮਾਪ_2()];
  ਨਹੀਂਤਾਂ
    Padding = " ";
}

ਖਾਲੀ Output::flow_ਕੁੰਜੀ(StringRef ਕੁੰਜੀ) {
  ਜੇ (State_ਢੇਰ.ਪਿੱਠ() == ਵਿੱਚ_Flow_Map_Other_ਕੁੰਜੀ)
    output(", ");
  ਜੇ (Wrap_ਕਾਲਮ && ਕਾਲਮ > Wrap_ਕਾਲਮ) {
    output("\n");
    ਲਈ (ਪੂਰਨ I = 0; I < ਕਾਲਮ_At_Map_Flow_ਸ਼ੁਰੂ; ++I)
      output(" ");
    ਕਾਲਮ = ਕਾਲਮ_At_Map_Flow_ਸ਼ੁਰੂ;
    output("  ");
  }
  output(ਕੁੰਜੀ, needsQuotes(ਕੁੰਜੀ, ਝੂਠ));
  output(": ");
}

ਗੰਢ_ਕਿਸਮ Output::get_ਗੰਢ_ਕਿਸਮ() { report_fatal_error("invalid call"); }

ਤਾਰਕਿਕ Output::ਵਿੱਚ_Seq_Any_Element(ਵਿੱਚ_State State) {
  ਵਾਪਸ State == ਵਿੱਚ_Seq_ਪਹਿਲਾ_Element || State == ਵਿੱਚ_Seq_Other_Element;
}

ਤਾਰਕਿਕ Output::ਵਿੱਚ_Flow_Seq_Any_Element(ਵਿੱਚ_State State) {
  ਵਾਪਸ State == ਵਿੱਚ_Flow_Seq_ਪਹਿਲਾ_Element || State == ਵਿੱਚ_Flow_Seq_Other_Element;
}

ਤਾਰਕਿਕ Output::ਵਿੱਚ_Map_Any_ਕੁੰਜੀ(ਵਿੱਚ_State State) {
  ਵਾਪਸ State == ਵਿੱਚ_Map_ਪਹਿਲਾ_ਕੁੰਜੀ || State == ਵਿੱਚ_Map_Other_ਕੁੰਜੀ;
}

ਤਾਰਕਿਕ Output::ਵਿੱਚ_Flow_Map_Any_ਕੁੰਜੀ(ਵਿੱਚ_State State) {
  ਵਾਪਸ State == ਵਿੱਚ_Flow_Map_ਪਹਿਲਾ_ਕੁੰਜੀ || State == ਵਿੱਚ_Flow_Map_Other_ਕੁੰਜੀ;
}

//===----------------------------------------------------------------------===//
//  traits for built-in types
//===----------------------------------------------------------------------===//

ਖਾਲੀ ScalarTraits<ਤਾਰਕਿਕ>::output(ਅਚਲ ਤਾਰਕਿਕ &Val, ਖਾਲੀ *, raw_ostream &ਬਾਹਰਲਾ) {
  ਬਾਹਰਲਾ << (Val ? "true" : "false");
}

StringRef ScalarTraits<ਤਾਰਕਿਕ>::ਨਿਵੇਸ਼_2(StringRef Scalar, ਖਾਲੀ *, ਤਾਰਕਿਕ &Val) {
  ਜੇ (std::optional<ਤਾਰਕਿਕ> Parsed = parseBool(Scalar)) {
    Val = *Parsed;
    ਵਾਪਸ StringRef();
  }
  ਵਾਪਸ "invalid boolean";
}

ਖਾਲੀ ScalarTraits<StringRef>::output(ਅਚਲ StringRef &Val, ਖਾਲੀ *,
                                     raw_ostream &ਬਾਹਰਲਾ) {
  ਬਾਹਰਲਾ << Val;
}

StringRef ScalarTraits<StringRef>::ਨਿਵੇਸ਼_2(StringRef Scalar, ਖਾਲੀ *,
                                         StringRef &Val) {
  Val = Scalar;
  ਵਾਪਸ StringRef();
}

ਖਾਲੀ ScalarTraits<std::string>::output(ਅਚਲ std::string &Val, ਖਾਲੀ *,
                                       raw_ostream &ਬਾਹਰਲਾ) {
  ਬਾਹਰਲਾ << Val;
}

StringRef ScalarTraits<std::string>::ਨਿਵੇਸ਼_2(StringRef Scalar, ਖਾਲੀ *,
                                           std::string &Val) {
  Val = Scalar.str();
  ਵਾਪਸ StringRef();
}

ਖਾਲੀ ScalarTraits<uint8_t>::output(ਅਚਲ uint8_t &Val, ਖਾਲੀ *,
                                   raw_ostream &ਬਾਹਰਲਾ) {
  // use temp uin32_t because ostream thinks uint8_t is a character
  uint32_t ਅੰਕ = Val;
  ਬਾਹਰਲਾ << ਅੰਕ;
}

StringRef ScalarTraits<uint8_t>::ਨਿਵੇਸ਼_2(StringRef Scalar, ਖਾਲੀ *, uint8_t &Val) {
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ ਲੰਮਾ n;
  ਜੇ (getAsUnsignedInteger(Scalar, 0, n))
    ਵਾਪਸ "invalid number";
  ਜੇ (n > 0xFF)
    ਵਾਪਸ "out of range number";
  Val = n;
  ਵਾਪਸ StringRef();
}

ਖਾਲੀ ScalarTraits<uint16_t>::output(ਅਚਲ uint16_t &Val, ਖਾਲੀ *,
                                    raw_ostream &ਬਾਹਰਲਾ) {
  ਬਾਹਰਲਾ << Val;
}

StringRef ScalarTraits<uint16_t>::ਨਿਵੇਸ਼_2(StringRef Scalar, ਖਾਲੀ *,
                                        uint16_t &Val) {
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ ਲੰਮਾ n;
  ਜੇ (getAsUnsignedInteger(Scalar, 0, n))
    ਵਾਪਸ "invalid number";
  ਜੇ (n > 0xFFFF)
    ਵਾਪਸ "out of range number";
  Val = n;
  ਵਾਪਸ StringRef();
}

ਖਾਲੀ ScalarTraits<uint32_t>::output(ਅਚਲ uint32_t &Val, ਖਾਲੀ *,
                                    raw_ostream &ਬਾਹਰਲਾ) {
  ਬਾਹਰਲਾ << Val;
}

StringRef ScalarTraits<uint32_t>::ਨਿਵੇਸ਼_2(StringRef Scalar, ਖਾਲੀ *,
                                        uint32_t &Val) {
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ ਲੰਮਾ n;
  ਜੇ (getAsUnsignedInteger(Scalar, 0, n))
    ਵਾਪਸ "invalid number";
  ਜੇ (n > 0xFFFFFFFFUL)
    ਵਾਪਸ "out of range number";
  Val = n;
  ਵਾਪਸ StringRef();
}

ਖਾਲੀ ScalarTraits<uint64_t>::output(ਅਚਲ uint64_t &Val, ਖਾਲੀ *,
                                    raw_ostream &ਬਾਹਰਲਾ) {
  ਬਾਹਰਲਾ << Val;
}

StringRef ScalarTraits<uint64_t>::ਨਿਵੇਸ਼_2(StringRef Scalar, ਖਾਲੀ *,
                                        uint64_t &Val) {
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ ਲੰਮਾ N;
  ਜੇ (getAsUnsignedInteger(Scalar, 0, N))
    ਵਾਪਸ "invalid number";
  Val = N;
  ਵਾਪਸ StringRef();
}

ਖਾਲੀ ScalarTraits<int8_t>::output(ਅਚਲ int8_t &Val, ਖਾਲੀ *, raw_ostream &ਬਾਹਰਲਾ) {
  // use temp in32_t because ostream thinks int8_t is a character
  int32_t ਅੰਕ = Val;
  ਬਾਹਰਲਾ << ਅੰਕ;
}

StringRef ScalarTraits<int8_t>::ਨਿਵੇਸ਼_2(StringRef Scalar, ਖਾਲੀ *, int8_t &Val) {
  ਲੰਮਾ ਲੰਮਾ N;
  ਜੇ (getAsSignedInteger(Scalar, 0, N))
    ਵਾਪਸ "invalid number";
  ਜੇ ((N > 127) || (N < -128))
    ਵਾਪਸ "out of range number";
  Val = N;
  ਵਾਪਸ StringRef();
}

ਖਾਲੀ ScalarTraits<int16_t>::output(ਅਚਲ int16_t &Val, ਖਾਲੀ *,
                                   raw_ostream &ਬਾਹਰਲਾ) {
  ਬਾਹਰਲਾ << Val;
}

StringRef ScalarTraits<int16_t>::ਨਿਵੇਸ਼_2(StringRef Scalar, ਖਾਲੀ *, int16_t &Val) {
  ਲੰਮਾ ਲੰਮਾ N;
  ਜੇ (getAsSignedInteger(Scalar, 0, N))
    ਵਾਪਸ "invalid number";
  ਜੇ ((N > INT16_MAX) || (N < INT16_MIN))
    ਵਾਪਸ "out of range number";
  Val = N;
  ਵਾਪਸ StringRef();
}

ਖਾਲੀ ScalarTraits<int32_t>::output(ਅਚਲ int32_t &Val, ਖਾਲੀ *,
                                   raw_ostream &ਬਾਹਰਲਾ) {
  ਬਾਹਰਲਾ << Val;
}

StringRef ScalarTraits<int32_t>::ਨਿਵੇਸ਼_2(StringRef Scalar, ਖਾਲੀ *, int32_t &Val) {
  ਲੰਮਾ ਲੰਮਾ N;
  ਜੇ (getAsSignedInteger(Scalar, 0, N))
    ਵਾਪਸ "invalid number";
  ਜੇ ((N > INT32_MAX) || (N < INT32_MIN))
    ਵਾਪਸ "out of range number";
  Val = N;
  ਵਾਪਸ StringRef();
}

ਖਾਲੀ ScalarTraits<int64_t>::output(ਅਚਲ int64_t &Val, ਖਾਲੀ *,
                                   raw_ostream &ਬਾਹਰਲਾ) {
  ਬਾਹਰਲਾ << Val;
}

StringRef ScalarTraits<int64_t>::ਨਿਵੇਸ਼_2(StringRef Scalar, ਖਾਲੀ *, int64_t &Val) {
  ਲੰਮਾ ਲੰਮਾ N;
  ਜੇ (getAsSignedInteger(Scalar, 0, N))
    ਵਾਪਸ "invalid number";
  Val = N;
  ਵਾਪਸ StringRef();
}

ਖਾਲੀ ScalarTraits<ਦੁੱਗਣਾ>::output(ਅਚਲ ਦੁੱਗਣਾ &Val, ਖਾਲੀ *, raw_ostream &ਬਾਹਰਲਾ) {
  ਬਾਹਰਲਾ << format("%g", Val);
}

StringRef ScalarTraits<ਦੁੱਗਣਾ>::ਨਿਵੇਸ਼_2(StringRef Scalar, ਖਾਲੀ *, ਦੁੱਗਣਾ &Val) {
  ਜੇ (ਮੰਜ਼ਿਲ_float(Scalar, Val))
    ਵਾਪਸ StringRef();
  ਵਾਪਸ "invalid floating point number";
}

ਖਾਲੀ ScalarTraits<ਦਸ਼ਮਲਵ>::output(ਅਚਲ ਦਸ਼ਮਲਵ &Val, ਖਾਲੀ *, raw_ostream &ਬਾਹਰਲਾ) {
  ਬਾਹਰਲਾ << format("%g", Val);
}

StringRef ScalarTraits<ਦਸ਼ਮਲਵ>::ਨਿਵੇਸ਼_2(StringRef Scalar, ਖਾਲੀ *, ਦਸ਼ਮਲਵ &Val) {
  ਜੇ (ਮੰਜ਼ਿਲ_float(Scalar, Val))
    ਵਾਪਸ StringRef();
  ਵਾਪਸ "invalid floating point number";
}

ਖਾਲੀ ScalarTraits<Hex8>::output(ਅਚਲ Hex8 &Val, ਖਾਲੀ *, raw_ostream &ਬਾਹਰਲਾ) {
  ਬਾਹਰਲਾ << format("0x%" PRIX8, (uint8_t)Val);
}

StringRef ScalarTraits<Hex8>::ਨਿਵੇਸ਼_2(StringRef Scalar, ਖਾਲੀ *, Hex8 &Val) {
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ ਲੰਮਾ n;
  ਜੇ (getAsUnsignedInteger(Scalar, 0, n))
    ਵਾਪਸ "invalid hex8 number";
  ਜੇ (n > 0xFF)
    ਵਾਪਸ "out of range hex8 number";
  Val = n;
  ਵਾਪਸ StringRef();
}

ਖਾਲੀ ScalarTraits<Hex16>::output(ਅਚਲ Hex16 &Val, ਖਾਲੀ *, raw_ostream &ਬਾਹਰਲਾ) {
  ਬਾਹਰਲਾ << format("0x%" PRIX16, (uint16_t)Val);
}

StringRef ScalarTraits<Hex16>::ਨਿਵੇਸ਼_2(StringRef Scalar, ਖਾਲੀ *, Hex16 &Val) {
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ ਲੰਮਾ n;
  ਜੇ (getAsUnsignedInteger(Scalar, 0, n))
    ਵਾਪਸ "invalid hex16 number";
  ਜੇ (n > 0xFFFF)
    ਵਾਪਸ "out of range hex16 number";
  Val = n;
  ਵਾਪਸ StringRef();
}

ਖਾਲੀ ScalarTraits<Hex32>::output(ਅਚਲ Hex32 &Val, ਖਾਲੀ *, raw_ostream &ਬਾਹਰਲਾ) {
  ਬਾਹਰਲਾ << format("0x%" PRIX32, (uint32_t)Val);
}

StringRef ScalarTraits<Hex32>::ਨਿਵੇਸ਼_2(StringRef Scalar, ਖਾਲੀ *, Hex32 &Val) {
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ ਲੰਮਾ n;
  ਜੇ (getAsUnsignedInteger(Scalar, 0, n))
    ਵਾਪਸ "invalid hex32 number";
  ਜੇ (n > 0xFFFFFFFFUL)
    ਵਾਪਸ "out of range hex32 number";
  Val = n;
  ਵਾਪਸ StringRef();
}

ਖਾਲੀ ScalarTraits<Hex64>::output(ਅਚਲ Hex64 &Val, ਖਾਲੀ *, raw_ostream &ਬਾਹਰਲਾ) {
  ਬਾਹਰਲਾ << format("0x%" PRIX64, (uint64_t)Val);
}

StringRef ScalarTraits<Hex64>::ਨਿਵੇਸ਼_2(StringRef Scalar, ਖਾਲੀ *, Hex64 &Val) {
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ ਲੰਮਾ ਅੰਕ;
  ਜੇ (getAsUnsignedInteger(Scalar, 0, ਅੰਕ))
    ਵਾਪਸ "invalid hex64 number";
  Val = ਅੰਕ;
  ਵਾਪਸ StringRef();
}

ਖਾਲੀ ScalarTraits<ਸੰਸਕਰਣ_Tuple>::output(ਅਚਲ ਸੰਸਕਰਣ_Tuple &Val, ਖਾਲੀ *,
                                        llvm::raw_ostream &ਬਾਹਰਲਾ) {
  ਬਾਹਰਲਾ << Val.getAsString();
}

StringRef ScalarTraits<ਸੰਸਕਰਣ_Tuple>::ਨਿਵੇਸ਼_2(StringRef Scalar, ਖਾਲੀ *,
                                            ਸੰਸਕਰਣ_Tuple &Val) {
  ਜੇ (Val.tryParse(Scalar))
    ਵਾਪਸ "invalid version format";
  ਵਾਪਸ StringRef();
}
