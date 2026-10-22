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

using ನಾಮಾಕಾಶ llvm;
using ನಾಮಾಕಾಶ yaml;

//===----------------------------------------------------------------------===//
//  IO
//===----------------------------------------------------------------------===//

IO::IO(ಶೂನ್ಯ *Context) : Ctxt(Context) {}

IO::~IO() = ಸ್ವಭಾವ;

ಶೂನ್ಯ *IO::getContext() ಧ್ರುವ {
  ಪ್ರತ್ಯಾಗಮ Ctxt;
}

ಶೂನ್ಯ IO::setContext(ಶೂನ್ಯ *Context) {
  Ctxt = Context;
}

ಶೂನ್ಯ IO::set_Allow_Unknown_ಕೀಲಿಗಳು(ತಾರ್ಕಿಕ Allow) {
  llvm_unreachable("Only supported for Input");
}

//===----------------------------------------------------------------------===//
//ಒಳಹರಿವು
//===----------------------------------------------------------------------===//

ಒಳಹರಿವು::ಒಳಹರಿವು(StringRef ಒಳಹರಿವು_Content, ಶೂನ್ಯ *Ctxt,
             SourceMgr::DiagHandlerTy DiagHandler, ಶೂನ್ಯ *DiagHandlerCtxt)
    : IO(Ctxt), Strm(ನೂತನ Stream(ಒಳಹರಿವು_Content, SrcMgr, ಅಸತ್ಯ, &EC)) {
  ಯದಿ (DiagHandler)
    SrcMgr.setDiagHandler(DiagHandler, DiagHandlerCtxt);
  DocIterator = Strm->begin();
}

ಒಳಹರಿವು::ಒಳಹರಿವು(MemoryBufferRef ಒಳಹರಿವು, ಶೂನ್ಯ *Ctxt,
             SourceMgr::DiagHandlerTy DiagHandler, ಶೂನ್ಯ *DiagHandlerCtxt)
    : IO(Ctxt), Strm(ನೂತನ Stream(ಒಳಹರಿವು, SrcMgr, ಅಸತ್ಯ, &EC)) {
  ಯದಿ (DiagHandler)
    SrcMgr.setDiagHandler(DiagHandler, DiagHandlerCtxt);
  DocIterator = Strm->begin();
}

ಒಳಹರಿವು::~ಒಳಹರಿವು() = ಸ್ವಭಾವ;

std::error_code ಒಳಹರಿವು::error() { ಪ್ರತ್ಯಾಗಮ EC; }

ತಾರ್ಕಿಕ ಒಳಹರಿವು::outputting() ಧ್ರುವ {
  ಪ್ರತ್ಯಾಗಮ ಅಸತ್ಯ;
}

ತಾರ್ಕಿಕ ಒಳಹರಿವು::setCurrentDocument() {
  ಯದಿ (DocIterator != Strm->end()) {
    ಗಂಟು *N = DocIterator->get_ಬೇರು();
    ಯದಿ (!N) {
      EC = make_error_code(errc::invalid_argument);
      ಪ್ರತ್ಯಾಗಮ ಅಸತ್ಯ;
    }

    ಯದಿ (isa<Null_ಗಂಟು>(N)) {
      // Empty files are allowed and ignored
      ++DocIterator;
      ಪ್ರತ್ಯಾಗಮ setCurrentDocument();
    }
    releaseHNodeBuffers();
    ತುದಿ_ಗಂಟು = createHNodes(N);
    Current_ಗಂಟು = ತುದಿ_ಗಂಟು;
    ಪ್ರತ್ಯಾಗಮ ಸತ್ಯ;
  }
  ಪ್ರತ್ಯಾಗಮ ಅಸತ್ಯ;
}

ತಾರ್ಕಿಕ ಒಳಹರಿವು::nextDocument() {
  ಪ್ರತ್ಯಾಗಮ ++DocIterator != Strm->end();
}

ಧ್ರುವ ಗಂಟು *ಒಳಹರಿವು::get_Current_ಗಂಟು() ಧ್ರುವ {
  ಪ್ರತ್ಯಾಗಮ Current_ಗಂಟು ? Current_ಗಂಟು->ಗಂಟು_2 : nullptr;
}

ತಾರ್ಕಿಕ ಒಳಹರಿವು::mapTag(StringRef Tag, ತಾರ್ಕಿಕ Default) {
  // CurrentNode can be null if setCurrentDocument() was unable to
  // parse the document because it was invalid or empty.
  ಯದಿ (!Current_ಗಂಟು)
    ಪ್ರತ್ಯಾಗಮ ಅಸತ್ಯ;

  std::string foundTag = Current_ಗಂಟು->ಗಂಟು_2->getVerbatimTag();
  ಯದಿ (foundTag.empty()) {
    // If no tag found and 'Tag' is the default, say it was found.
    ಪ್ರತ್ಯಾಗಮ Default;
  }
  // Return true iff found tag matches supplied tag.
  ಪ್ರತ್ಯಾಗಮ Tag == foundTag;
}

ಶೂನ್ಯ ಒಳಹರಿವು::beginMapping() {
  ಯದಿ (EC)
    ಪ್ರತ್ಯಾಗಮ;
  // CurrentNode can be null if the document is empty.
  MapHNode *MN = dyn_cast_or_null<MapHNode>(Current_ಗಂಟು);
  ಯದಿ (MN) {
    MN->Valid_ಕೀಲಿಗಳು.clear();
  }
}

std::vector<StringRef> ಒಳಹರಿವು::ಕೀಲಿಗಳು() {
  MapHNode *MN = dyn_cast<MapHNode>(Current_ಗಂಟು);
  std::vector<StringRef> Ret;
  ಯದಿ (!MN) {
    setError(Current_ಗಂಟು, "not a mapping");
    ಪ್ರತ್ಯಾಗಮ Ret;
  }
  ಪ್ರತಿ (auto &P : MN->Mapping)
    Ret.ತಳ್ಳು_back(P.first());
  ಪ್ರತ್ಯಾಗಮ Ret;
}

ತಾರ್ಕಿಕ ಒಳಹರಿವು::preflight_ಕೀಲಿ(StringRef ಕೀಲಿ, ತಾರ್ಕಿಕ Required, ತಾರ್ಕಿಕ, ತಾರ್ಕಿಕ &UseDefault,
                         ಶೂನ್ಯ *&SaveInfo) {
  UseDefault = ಅಸತ್ಯ;
  ಯದಿ (EC)
    ಪ್ರತ್ಯಾಗಮ ಅಸತ್ಯ;

  // CurrentNode is null for empty documents, which is an error in case required
  // nodes are present.
  ಯದಿ (!Current_ಗಂಟು) {
    ಯದಿ (Required)
      EC = make_error_code(errc::invalid_argument);
    ಅನ್ಯಥಾ
      UseDefault = ಸತ್ಯ;
    ಪ್ರತ್ಯಾಗಮ ಅಸತ್ಯ;
  }

  MapHNode *MN = dyn_cast<MapHNode>(Current_ಗಂಟು);
  ಯದಿ (!MN) {
    ಯದಿ (Required || !isa<EmptyHNode>(Current_ಗಂಟು))
      setError(Current_ಗಂಟು, "not a mapping");
    ಅನ್ಯಥಾ
      UseDefault = ಸತ್ಯ;
    ಪ್ರತ್ಯಾಗಮ ಅಸತ್ಯ;
  }
  MN->Valid_ಕೀಲಿಗಳು.ತಳ್ಳು_back(ಕೀಲಿ.str());
  HNode *ಮೌಲ್ಯ = MN->Mapping[ಕೀಲಿ].first;
  ಯದಿ (!ಮೌಲ್ಯ) {
    ಯದಿ (Required)
      setError(Current_ಗಂಟು, Twine("missing required key '") + ಕೀಲಿ + "'");
    ಅನ್ಯಥಾ
      UseDefault = ಸತ್ಯ;
    ಪ್ರತ್ಯಾಗಮ ಅಸತ್ಯ;
  }
  SaveInfo = Current_ಗಂಟು;
  Current_ಗಂಟು = ಮೌಲ್ಯ;
  ಪ್ರತ್ಯಾಗಮ ಸತ್ಯ;
}

ಶೂನ್ಯ ಒಳಹರಿವು::postflight_ಕೀಲಿ(ಶೂನ್ಯ *saveInfo) {
  Current_ಗಂಟು = reinterpret_cast<HNode *>(saveInfo);
}

ಶೂನ್ಯ ಒಳಹರಿವು::endMapping() {
  ಯದಿ (EC)
    ಪ್ರತ್ಯಾಗಮ;
  // CurrentNode can be null if the document is empty.
  MapHNode *MN = dyn_cast_or_null<MapHNode>(Current_ಗಂಟು);
  ಯದಿ (!MN)
    ಪ್ರತ್ಯಾಗಮ;
  ಪ್ರತಿ (ಧ್ರುವ auto &NN : MN->Mapping) {
    ಯದಿ (!is_contained(MN->Valid_ಕೀಲಿಗಳು, NN.first())) {
      ಧ್ರುವ SMRange &ReportLoc = NN.second.second;
      ಯದಿ (!Allow_Unknown_ಕೀಲಿಗಳು) {
        setError(ReportLoc, Twine("unknown key '") + NN.first() + "'");
        ಭಙ್ಗ;
      } ಅನ್ಯಥಾ
        reportWarning(ReportLoc, Twine("unknown key '") + NN.first() + "'");
    }
  }
}

ಶೂನ್ಯ ಒಳಹರಿವು::beginFlowMapping() { beginMapping(); }

ಶೂನ್ಯ ಒಳಹರಿವು::endFlowMapping() { endMapping(); }

unsigned ಒಳಹರಿವು::beginSequence() {
  ಯದಿ (SequenceHNode *SQ = dyn_cast<SequenceHNode>(Current_ಗಂಟು))
    ಪ್ರತ್ಯಾಗಮ SQ->Entries.size();
  ಯದಿ (isa<EmptyHNode>(Current_ಗಂಟು))
    ಪ್ರತ್ಯಾಗಮ 0;
  // Treat case where there's a scalar "null" value as an empty sequence.
  ಯದಿ (ScalarHNode *SN = dyn_cast<ScalarHNode>(Current_ಗಂಟು)) {
    ಯದಿ (isNull(SN->ಮೌಲ್ಯ_2()))
      ಪ್ರತ್ಯಾಗಮ 0;
  }
  // Any other type of HNode is an error.
  setError(Current_ಗಂಟು, "not a sequence");
  ಪ್ರತ್ಯಾಗಮ 0;
}

ಶೂನ್ಯ ಒಳಹರಿವು::endSequence() {
}

ತಾರ್ಕಿಕ ಒಳಹರಿವು::preflightElement(unsigned Index, ಶೂನ್ಯ *&SaveInfo) {
  ಯದಿ (EC)
    ಪ್ರತ್ಯಾಗಮ ಅಸತ್ಯ;
  ಯದಿ (SequenceHNode *SQ = dyn_cast<SequenceHNode>(Current_ಗಂಟು)) {
    SaveInfo = Current_ಗಂಟು;
    Current_ಗಂಟು = SQ->Entries[Index];
    ಪ್ರತ್ಯಾಗಮ ಸತ್ಯ;
  }
  ಪ್ರತ್ಯಾಗಮ ಅಸತ್ಯ;
}

ಶೂನ್ಯ ಒಳಹರಿವು::postflightElement(ಶೂನ್ಯ *SaveInfo) {
  Current_ಗಂಟು = reinterpret_cast<HNode *>(SaveInfo);
}

unsigned ಒಳಹರಿವು::beginFlowSequence() { ಪ್ರತ್ಯಾಗಮ beginSequence(); }

ತಾರ್ಕಿಕ ಒಳಹರಿವು::preflightFlowElement(unsigned index, ಶೂನ್ಯ *&SaveInfo) {
  ಯದಿ (EC)
    ಪ್ರತ್ಯಾಗಮ ಅಸತ್ಯ;
  ಯದಿ (SequenceHNode *SQ = dyn_cast<SequenceHNode>(Current_ಗಂಟು)) {
    SaveInfo = Current_ಗಂಟು;
    Current_ಗಂಟು = SQ->Entries[index];
    ಪ್ರತ್ಯಾಗಮ ಸತ್ಯ;
  }
  ಪ್ರತ್ಯಾಗಮ ಅಸತ್ಯ;
}

ಶೂನ್ಯ ಒಳಹರಿವು::postflightFlowElement(ಶೂನ್ಯ *SaveInfo) {
  Current_ಗಂಟು = reinterpret_cast<HNode *>(SaveInfo);
}

ಶೂನ್ಯ ಒಳಹರಿವು::endFlowSequence() {
}

ಶೂನ್ಯ ಒಳಹರಿವು::beginEnumScalar() {
  ScalarMatchFound = ಅಸತ್ಯ;
}

ತಾರ್ಕಿಕ ಒಳಹರಿವು::matchEnumScalar(StringRef Str, ತಾರ್ಕಿಕ) {
  ಯದಿ (ScalarMatchFound)
    ಪ್ರತ್ಯಾಗಮ ಅಸತ್ಯ;
  ಯದಿ (ScalarHNode *SN = dyn_cast<ScalarHNode>(Current_ಗಂಟು)) {
    ಯದಿ (SN->ಮೌಲ್ಯ_2() == Str) {
      ScalarMatchFound = ಸತ್ಯ;
      ಪ್ರತ್ಯಾಗಮ ಸತ್ಯ;
    }
  }
  ಪ್ರತ್ಯಾಗಮ ಅಸತ್ಯ;
}

ತಾರ್ಕಿಕ ಒಳಹರಿವು::matchEnumFallback() {
  ಯದಿ (ScalarMatchFound)
    ಪ್ರತ್ಯಾಗಮ ಅಸತ್ಯ;
  ScalarMatchFound = ಸತ್ಯ;
  ಪ್ರತ್ಯಾಗಮ ಸತ್ಯ;
}

ಶೂನ್ಯ ಒಳಹರಿವು::endEnumScalar() {
  ಯದಿ (!ScalarMatchFound) {
    setError(Current_ಗಂಟು, "unknown enumerated scalar");
  }
}

ತಾರ್ಕಿಕ ಒಳಹರಿವು::beginBitSetScalar(ತಾರ್ಕಿಕ &DoClear) {
  Bit_ಮೌಲ್ಯಗಳು_Used.clear();
  ಯದಿ (SequenceHNode *SQ = dyn_cast<SequenceHNode>(Current_ಗಂಟು)) {
    Bit_ಮೌಲ್ಯಗಳು_Used.resize(SQ->Entries.size());
  } ಅನ್ಯಥಾ {
    setError(Current_ಗಂಟು, "expected sequence of bit values");
  }
  DoClear = ಸತ್ಯ;
  ಪ್ರತ್ಯಾಗಮ ಸತ್ಯ;
}

ತಾರ್ಕಿಕ ಒಳಹರಿವು::bitSetMatch(StringRef Str, ತಾರ್ಕಿಕ) {
  ಯದಿ (EC)
    ಪ್ರತ್ಯಾಗಮ ಅಸತ್ಯ;
  ಯದಿ (SequenceHNode *SQ = dyn_cast<SequenceHNode>(Current_ಗಂಟು)) {
    unsigned Index = 0;
    ಪ್ರತಿ (auto &N : SQ->Entries) {
      ಯದಿ (ScalarHNode *SN = dyn_cast<ScalarHNode>(N)) {
        ಯದಿ (SN->ಮೌಲ್ಯ_2() == Str) {
          Bit_ಮೌಲ್ಯಗಳು_Used[Index] = ಸತ್ಯ;
          ಪ್ರತ್ಯಾಗಮ ಸತ್ಯ;
        }
      } ಅನ್ಯಥಾ {
        setError(Current_ಗಂಟು, "unexpected scalar in sequence of bit values");
      }
      ++Index;
    }
  } ಅನ್ಯಥಾ {
    setError(Current_ಗಂಟು, "expected sequence of bit values");
  }
  ಪ್ರತ್ಯಾಗಮ ಅಸತ್ಯ;
}

ಶೂನ್ಯ ಒಳಹರಿವು::endBitSetScalar() {
  ಯದಿ (EC)
    ಪ್ರತ್ಯಾಗಮ;
  ಯದಿ (SequenceHNode *SQ = dyn_cast<SequenceHNode>(Current_ಗಂಟು)) {
    ಪ್ರತಿಜ್ಞಾ(Bit_ಮೌಲ್ಯಗಳು_Used.size() == SQ->Entries.size());
    ಪ್ರತಿ (unsigned i = 0; i < SQ->Entries.size(); ++i) {
      ಯದಿ (!Bit_ಮೌಲ್ಯಗಳು_Used[i]) {
        setError(SQ->Entries[i], "unknown bit value");
        ಪ್ರತ್ಯಾಗಮ;
      }
    }
  }
}

ಶೂನ್ಯ ಒಳಹರಿವು::scalarString(StringRef &S, QuotingType) {
  ಯದಿ (ScalarHNode *SN = dyn_cast<ScalarHNode>(Current_ಗಂಟು)) {
    S = SN->ಮೌಲ್ಯ_2();
  } ಅನ್ಯಥಾ {
    setError(Current_ಗಂಟು, "unexpected scalar");
  }
}

ಶೂನ್ಯ ಒಳಹರಿವು::blockScalarString(StringRef &S) { scalarString(S, QuotingType::None); }

ಶೂನ್ಯ ಒಳಹರಿವು::scalarTag(std::string &Tag) {
  Tag = Current_ಗಂಟು->ಗಂಟು_2->getVerbatimTag();
}

ಶೂನ್ಯ ಒಳಹರಿವು::setError(HNode *hnode, ಧ್ರುವ Twine &message) {
  ಪ್ರತಿಜ್ಞಾ(hnode && "HNode must not be NULL");
  setError(hnode->ಗಂಟು_2, message);
}

ಗಂಟು_Kind ಒಳಹರಿವು::get_ಗಂಟು_Kind() {
  ಯದಿ (isa<ScalarHNode>(Current_ಗಂಟು))
    ಪ್ರತ್ಯಾಗಮ ಗಂಟು_Kind::Scalar;
  ಅನ್ಯಥಾ ಯದಿ (isa<MapHNode>(Current_ಗಂಟು))
    ಪ್ರತ್ಯಾಗಮ ಗಂಟು_Kind::Map;
  ಅನ್ಯಥಾ ಯದಿ (isa<SequenceHNode>(Current_ಗಂಟು))
    ಪ್ರತ್ಯಾಗಮ ಗಂಟು_Kind::Sequence;
  llvm_unreachable("Unsupported node kind");
}

ಶೂನ್ಯ ಒಳಹರಿವು::setError(ಗಂಟು *ಗಂಟು_3, ಧ್ರುವ Twine &message) {
  Strm->printError(ಗಂಟು_3, message);
  EC = make_error_code(errc::invalid_argument);
}

ಶೂನ್ಯ ಒಳಹರಿವು::setError(ಧ್ರುವ SMRange &range, ಧ್ರುವ Twine &message) {
  Strm->printError(range, message);
  EC = make_error_code(errc::invalid_argument);
}

ಶೂನ್ಯ ಒಳಹರಿವು::reportWarning(HNode *hnode, ಧ್ರುವ Twine &message) {
  ಪ್ರತಿಜ್ಞಾ(hnode && "HNode must not be NULL");
  Strm->printError(hnode->ಗಂಟು_2, message, SourceMgr::DK_Warning);
}

ಶೂನ್ಯ ಒಳಹರಿವು::reportWarning(ಗಂಟು *ಗಂಟು_3, ಧ್ರುವ Twine &message) {
  Strm->printError(ಗಂಟು_3, message, SourceMgr::DK_Warning);
}

ಶೂನ್ಯ ಒಳಹರಿವು::reportWarning(ಧ್ರುವ SMRange &range, ಧ್ರುವ Twine &message) {
  Strm->printError(range, message, SourceMgr::DK_Warning);
}

ಶೂನ್ಯ ಒಳಹರಿವು::releaseHNodeBuffers() {
  EmptyHNodeAllocator.DestroyAll();
  ScalarHNodeAllocator.DestroyAll();
  SequenceHNodeAllocator.DestroyAll();
  MapHNodeAllocator.DestroyAll();
}

ಶೂನ್ಯ ಒಳಹರಿವು::saveAliasHNode(ಗಂಟು *N, HNode *HN) {
  StringRef Anchor = N->getAnchor();
  ಯದಿ (!Anchor.empty())
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
  ವಿಕಲ್ಪ (N->getType()) {
  ಪಕ್ಷ ಗಂಟು::NK_Scalar: {
    Scalar_ಗಂಟು *SN = dyn_cast<Scalar_ಗಂಟು>(N);
    StringRef ಕೀಲಿ_Str = SN->get_ಮೌಲ್ಯ(StringStorage);
    ಯದಿ (!StringStorage.empty()) {
      // Copy string to permanent storage
      ಕೀಲಿ_Str = StringStorage.str().copy(StringAllocator);
    }
    auto *SHNode = ನೂತನ (ScalarHNodeAllocator.Allocate()) ScalarHNode(N, ಕೀಲಿ_Str);
    saveAliasHNode(SN, SHNode);
    ಪ್ರತ್ಯಾಗಮ SHNode;
  }
  ಪಕ್ಷ ಗಂಟು::NK_BlockScalar: {
    Block_Scalar_ಗಂಟು *BSN = dyn_cast<Block_Scalar_ಗಂಟು>(N);
    StringRef ಮೌಲ್ಯ_Copy = BSN->get_ಮೌಲ್ಯ().copy(StringAllocator);
    auto *BSHNode =
        ನೂತನ (ScalarHNodeAllocator.Allocate()) ScalarHNode(N, ಮೌಲ್ಯ_Copy);
    saveAliasHNode(BSN, BSHNode);
    ಪ್ರತ್ಯಾಗಮ BSHNode;
  }
  ಪಕ್ಷ ಗಂಟು::NK_Sequence: {
    Sequence_ಗಂಟು *SQ = dyn_cast<Sequence_ಗಂಟು>(N);
    auto SQHNode = ನೂತನ (SequenceHNodeAllocator.Allocate()) SequenceHNode(N);
    ಪ್ರತಿ (ಗಂಟು &SN : *SQ) {
      auto Entry = createHNodes(&SN);
      ಯದಿ (EC)
        ಭಙ್ಗ;
      SQHNode->Entries.ತಳ್ಳು_back(Entry);
    }
    saveAliasHNode(SQ, SQHNode);
    ಪ್ರತ್ಯಾಗಮ SQHNode;
  }
  ಪಕ್ಷ ಗಂಟು::NK_Mapping: {
    Mapping_ಗಂಟು *Map = dyn_cast<Mapping_ಗಂಟು>(N);
    auto mapHNode = ನೂತನ (MapHNodeAllocator.Allocate()) MapHNode(N);
    ಪ್ರತಿ (ಕೀಲಿ_ಮೌಲ್ಯ_ಗಂಟು &KVN : *Map) {
      ಗಂಟು *ಕೀಲಿ_ಗಂಟು = KVN.get_ಕೀಲಿ();
      Scalar_ಗಂಟು *ಕೀಲಿ = dyn_cast_if_present<Scalar_ಗಂಟು>(ಕೀಲಿ_ಗಂಟು);
      ಗಂಟು *ಮೌಲ್ಯ = KVN.get_ಮೌಲ್ಯ();
      ಯದಿ (!ಕೀಲಿ || !ಮೌಲ್ಯ) {
        ಯದಿ (!ಕೀಲಿ)
          setError(ಕೀಲಿ_ಗಂಟು, "Map key must be a scalar");
        ಯದಿ (!ಮೌಲ್ಯ)
          setError(ಕೀಲಿ_ಗಂಟು, "Map value must not be empty");
        ಭಙ್ಗ;
      }
      StringStorage.clear();
      StringRef ಕೀಲಿ_Str = ಕೀಲಿ->get_ಮೌಲ್ಯ(StringStorage);
      ಯದಿ (!StringStorage.empty()) {
        // Copy string to permanent storage
        ಕೀಲಿ_Str = StringStorage.str().copy(StringAllocator);
      }
      ಯದಿ (mapHNode->Mapping.ಎಣಿಕೆ(ಕೀಲಿ_Str))
        // From YAML spec: "The content of a mapping node is an unordered set of
        // key/value node pairs, with the restriction that each of the keys is
        // unique."
        setError(ಕೀಲಿ_ಗಂಟು, Twine("duplicated mapping key '") + ಕೀಲಿ_Str + "'");
      auto ಮೌಲ್ಯ_HNode = createHNodes(ಮೌಲ್ಯ);
      ಯದಿ (EC)
        ಭಙ್ಗ;
      mapHNode->Mapping[ಕೀಲಿ_Str] =
          std::make_pair(std::move(ಮೌಲ್ಯ_HNode), ಕೀಲಿ_ಗಂಟು->getSourceRange());
    }
    saveAliasHNode(Map, mapHNode);
    ಪ್ರತ್ಯಾಗಮ std::move(mapHNode);
  }
  ಪಕ್ಷ ಗಂಟು::NK_Null:
    // TODO: Anchor is not set for NullNode in the parser. Update the parser to
    // faithfully preserve anchors.
    ಪ್ರತ್ಯಾಗಮ ನೂತನ (EmptyHNodeAllocator.Allocate()) EmptyHNode(N);
  ಪಕ್ಷ ಗಂಟು::NK_Alias: {
    Alias_ಗಂಟು *AN = dyn_cast<Alias_ಗಂಟು>(N);
    auto AliasName = AN->getName();
    auto AHN = AliasMap.find(AliasName);
    ಯದಿ (AHN == AliasMap.end()) {
      setError(AN, Twine("undefined alias '" + AliasName + "'"));
      ಪ್ರತ್ಯಾಗಮ nullptr;
    }
    ಪ್ರತ್ಯಾಗಮ AHN->second;
  }
  ಸ್ವಭಾವ:
    setError(N, "unknown node kind");
    ಪ್ರತ್ಯಾಗಮ nullptr;
  }
}

ಶೂನ್ಯ ಒಳಹರಿವು::setError(ಧ್ರುವ Twine &Message) {
  setError(Current_ಗಂಟು, Message);
}

ಶೂನ್ಯ ಒಳಹರಿವು::set_Allow_Unknown_ಕೀಲಿಗಳು(ತಾರ್ಕಿಕ Allow) { Allow_Unknown_ಕೀಲಿಗಳು = Allow; }

ತಾರ್ಕಿಕ ಒಳಹರಿವು::canElideEmptySequence() {
  ಪ್ರತ್ಯಾಗಮ ಅಸತ್ಯ;
}

//===----------------------------------------------------------------------===//
//  Output
//===----------------------------------------------------------------------===//

Output::Output(raw_ostream &yout, ಶೂನ್ಯ *context, ಪೂರ್ಣಾಙ್ಕ WrapColumn)
    : IO(context), Out(yout), WrapColumn(WrapColumn) {}

Output::~Output() = ಸ್ವಭಾವ;

ತಾರ್ಕಿಕ Output::outputting() ಧ್ರುವ {
  ಪ್ರತ್ಯಾಗಮ ಸತ್ಯ;
}

ಶೂನ್ಯ Output::beginMapping() {
  State_ರಾಶಿ.ತಳ್ಳು_back(in_Map_First_ಕೀಲಿ);
  PaddingBeforeContainer = Padding;
  Padding = "\n";
}

ತಾರ್ಕಿಕ Output::mapTag(StringRef Tag, ತಾರ್ಕಿಕ Use) {
  ಯದಿ (Use) {
    // If this tag is being written inside a sequence we should write the start
    // of the sequence before writing the tag, otherwise the tag won't be
    // attached to the element in the sequence, but rather the sequence itself.
    ತಾರ್ಕಿಕ SequenceElement = ಅಸತ್ಯ;
    ಯದಿ (State_ರಾಶಿ.size() > 1) {
      auto &E = State_ರಾಶಿ[State_ರಾಶಿ.size() - 2];
      SequenceElement = inSeqAnyElement(E) || inFlowSeqAnyElement(E);
    }
    ಯದಿ (SequenceElement && State_ರಾಶಿ.back() == in_Map_First_ಕೀಲಿ) {
      new_ಸಾಲು_Check();
    } ಅನ್ಯಥಾ {
      output(" ");
    }
    output(Tag);
    ಯದಿ (SequenceElement) {
      // If we're writing the tag during the first element of a map, the tag
      // takes the place of the first element in the sequence.
      ಯದಿ (State_ರಾಶಿ.back() == in_Map_First_ಕೀಲಿ) {
        State_ರಾಶಿ.ತೆಗೆ_back();
        State_ರಾಶಿ.ತಳ್ಳು_back(in_Map_Other_ಕೀಲಿ);
      }
      // Tags inside maps in sequences should act as keys in the map from a
      // formatting perspective, so we always want a newline in a sequence.
      Padding = "\n";
    }
  }
  ಪ್ರತ್ಯಾಗಮ Use;
}

ಶೂನ್ಯ Output::endMapping() {
  // If we did not map anything, we should explicitly emit an empty map
  ಯದಿ (State_ರಾಶಿ.back() == in_Map_First_ಕೀಲಿ) {
    Padding = PaddingBeforeContainer;
    new_ಸಾಲು_Check();
    output("{}");
    Padding = "\n";
  }
  State_ರಾಶಿ.ತೆಗೆ_back();
}

std::vector<StringRef> Output::ಕೀಲಿಗಳು() {
  report_fatal_error("invalid call");
}

ತಾರ್ಕಿಕ Output::preflight_ಕೀಲಿ(StringRef ಕೀಲಿ, ತಾರ್ಕಿಕ Required, ತಾರ್ಕಿಕ SameAsDefault,
                          ತಾರ್ಕಿಕ &UseDefault, ಶೂನ್ಯ *&SaveInfo) {
  UseDefault = ಅಸತ್ಯ;
  SaveInfo = nullptr;
  ಯದಿ (Required || !SameAsDefault || Write_Default_ಮೌಲ್ಯಗಳು) {
    auto State = State_ರಾಶಿ.back();
    ಯದಿ (State == in_Flow_Map_First_ಕೀಲಿ || State == in_Flow_Map_Other_ಕೀಲಿ) {
      flow_ಕೀಲಿ(ಕೀಲಿ);
    } ಅನ್ಯಥಾ {
      new_ಸಾಲು_Check();
      padded_ಕೀಲಿ(ಕೀಲಿ);
    }
    ಪ್ರತ್ಯಾಗಮ ಸತ್ಯ;
  }
  ಪ್ರತ್ಯಾಗಮ ಅಸತ್ಯ;
}

ಶೂನ್ಯ Output::postflight_ಕೀಲಿ(ಶೂನ್ಯ *) {
  ಯದಿ (State_ರಾಶಿ.back() == in_Map_First_ಕೀಲಿ) {
    State_ರಾಶಿ.ತೆಗೆ_back();
    State_ರಾಶಿ.ತಳ್ಳು_back(in_Map_Other_ಕೀಲಿ);
  } ಅನ್ಯಥಾ ಯದಿ (State_ರಾಶಿ.back() == in_Flow_Map_First_ಕೀಲಿ) {
    State_ರಾಶಿ.ತೆಗೆ_back();
    State_ರಾಶಿ.ತಳ್ಳು_back(in_Flow_Map_Other_ಕೀಲಿ);
  }
}

ಶೂನ್ಯ Output::beginFlowMapping() {
  State_ರಾಶಿ.ತಳ್ಳು_back(in_Flow_Map_First_ಕೀಲಿ);
  new_ಸಾಲು_Check();
  ColumnAtMapFlowStart = Column;
  output("{ ");
}

ಶೂನ್ಯ Output::endFlowMapping() {
  State_ರಾಶಿ.ತೆಗೆ_back();
  output_Up_To_End_Of_ಸಾಲು(" }");
}

ಶೂನ್ಯ Output::beginDocuments() {
  output_Up_To_End_Of_ಸಾಲು("---");
}

ತಾರ್ಕಿಕ Output::preflightDocument(unsigned index) {
  ಯದಿ (index > 0)
    output_Up_To_End_Of_ಸಾಲು("\n---");
  ಪ್ರತ್ಯಾಗಮ ಸತ್ಯ;
}

ಶೂನ್ಯ Output::postflightDocument() {
}

ಶೂನ್ಯ Output::endDocuments() {
  output("\n...\n");
}

unsigned Output::beginSequence() {
  State_ರಾಶಿ.ತಳ್ಳು_back(inSeqFirstElement);
  PaddingBeforeContainer = Padding;
  Padding = "\n";
  ಪ್ರತ್ಯಾಗಮ 0;
}

ಶೂನ್ಯ Output::endSequence() {
  // If we did not emit anything, we should explicitly emit an empty sequence
  ಯದಿ (State_ರಾಶಿ.back() == inSeqFirstElement) {
    Padding = PaddingBeforeContainer;
    new_ಸಾಲು_Check(/*EmptySequence=*/ಸತ್ಯ);
    output("[]");
    Padding = "\n";
  }
  State_ರಾಶಿ.ತೆಗೆ_back();
}

ತಾರ್ಕಿಕ Output::preflightElement(unsigned, ಶೂನ್ಯ *&SaveInfo) {
  SaveInfo = nullptr;
  ಪ್ರತ್ಯಾಗಮ ಸತ್ಯ;
}

ಶೂನ್ಯ Output::postflightElement(ಶೂನ್ಯ *) {
  ಯದಿ (State_ರಾಶಿ.back() == inSeqFirstElement) {
    State_ರಾಶಿ.ತೆಗೆ_back();
    State_ರಾಶಿ.ತಳ್ಳು_back(inSeqOtherElement);
  } ಅನ್ಯಥಾ ಯದಿ (State_ರಾಶಿ.back() == inFlowSeqFirstElement) {
    State_ರಾಶಿ.ತೆಗೆ_back();
    State_ರಾಶಿ.ತಳ್ಳು_back(inFlowSeqOtherElement);
  }
}

unsigned Output::beginFlowSequence() {
  State_ರಾಶಿ.ತಳ್ಳು_back(inFlowSeqFirstElement);
  new_ಸಾಲು_Check();
  ColumnAtFlowStart = Column;
  output("[ ");
  NeedFlowSequenceComma = ಅಸತ್ಯ;
  ಪ್ರತ್ಯಾಗಮ 0;
}

ಶೂನ್ಯ Output::endFlowSequence() {
  State_ರಾಶಿ.ತೆಗೆ_back();
  output_Up_To_End_Of_ಸಾಲು(" ]");
}

ತಾರ್ಕಿಕ Output::preflightFlowElement(unsigned, ಶೂನ್ಯ *&SaveInfo) {
  ಯದಿ (NeedFlowSequenceComma)
    output(", ");
  ಯದಿ (WrapColumn && Column > WrapColumn) {
    output("\n");
    ಪ್ರತಿ (ಪೂರ್ಣಾಙ್ಕ i = 0; i < ColumnAtFlowStart; ++i)
      output(" ");
    Column = ColumnAtFlowStart;
    output("  ");
  }
  SaveInfo = nullptr;
  ಪ್ರತ್ಯಾಗಮ ಸತ್ಯ;
}

ಶೂನ್ಯ Output::postflightFlowElement(ಶೂನ್ಯ *) {
  NeedFlowSequenceComma = ಸತ್ಯ;
}

ಶೂನ್ಯ Output::beginEnumScalar() {
  EnumerationMatchFound = ಅಸತ್ಯ;
}

ತಾರ್ಕಿಕ Output::matchEnumScalar(StringRef Str, ತಾರ್ಕಿಕ Match) {
  ಯದಿ (Match && !EnumerationMatchFound) {
    new_ಸಾಲು_Check();
    output_Up_To_End_Of_ಸಾಲು(Str);
    EnumerationMatchFound = ಸತ್ಯ;
  }
  ಪ್ರತ್ಯಾಗಮ ಅಸತ್ಯ;
}

ತಾರ್ಕಿಕ Output::matchEnumFallback() {
  ಯದಿ (EnumerationMatchFound)
    ಪ್ರತ್ಯಾಗಮ ಅಸತ್ಯ;
  EnumerationMatchFound = ಸತ್ಯ;
  ಪ್ರತ್ಯಾಗಮ ಸತ್ಯ;
}

ಶೂನ್ಯ Output::endEnumScalar() {
  ಯದಿ (!EnumerationMatchFound)
    llvm_unreachable("bad runtime enum value");
}

ತಾರ್ಕಿಕ Output::beginBitSetScalar(ತಾರ್ಕಿಕ &DoClear) {
  new_ಸಾಲು_Check();
  output("[ ");
  Need_Bit_ಮೌಲ್ಯ_Comma = ಅಸತ್ಯ;
  DoClear = ಅಸತ್ಯ;
  ಪ್ರತ್ಯಾಗಮ ಸತ್ಯ;
}

ತಾರ್ಕಿಕ Output::bitSetMatch(StringRef Str, ತಾರ್ಕಿಕ Matches) {
  ಯದಿ (Matches) {
    ಯದಿ (Need_Bit_ಮೌಲ್ಯ_Comma)
      output(", ");
    output(Str);
    Need_Bit_ಮೌಲ್ಯ_Comma = ಸತ್ಯ;
  }
  ಪ್ರತ್ಯಾಗಮ ಅಸತ್ಯ;
}

ಶೂನ್ಯ Output::endBitSetScalar() {
  output_Up_To_End_Of_ಸಾಲು(" ]");
}

ಶೂನ್ಯ Output::scalarString(StringRef &S, QuotingType MustQuote) {
  new_ಸಾಲು_Check();
  ಯದಿ (S.empty()) {
    // Print '' for the empty string because leaving the field empty is not
    // allowed.
    output_Up_To_End_Of_ಸಾಲು("''");
    ಪ್ರತ್ಯಾಗಮ;
  }
  output(S, MustQuote);
  output_Up_To_End_Of_ಸಾಲು("");
}

ಶೂನ್ಯ Output::blockScalarString(StringRef &S) {
  ಯದಿ (!State_ರಾಶಿ.empty())
    new_ಸಾಲು_Check();
  output(" |");

  unsigned Indent = State_ರಾಶಿ.empty() ? 1 : State_ರಾಶಿ.size();

  auto Buffer = MemoryBuffer::getMemBuffer(S, "", ಅಸತ್ಯ);
  ಪ್ರತಿ (ಸಾಲು_iterator ಸಾಲುಗಳು(*Buffer, ಅಸತ್ಯ); !ಸಾಲುಗಳು.is_at_end(); ++ಸಾಲುಗಳು) {
    output_New_ಸಾಲು();
    ಪ್ರತಿ (unsigned I = 0; I < Indent; ++I) {
      output("  ");
    }
    output(*ಸಾಲುಗಳು);
  }
  output_Up_To_End_Of_ಸಾಲು("");
}

ಶೂನ್ಯ Output::scalarTag(std::string &Tag) {
  ಯದಿ (Tag.empty())
    ಪ್ರತ್ಯಾಗಮ;
  new_ಸಾಲು_Check();
  output(Tag);
  output(" ");
}

ಶೂನ್ಯ Output::setError(ಧ್ರುವ Twine &message) {
}

std::error_code Output::error() { ಪ್ರತ್ಯಾಗಮ {}; }

ತಾರ್ಕಿಕ Output::canElideEmptySequence() {
  // Normally, with an optional key/value where the value is an empty sequence,
  // the whole key/value can be not written.  But, that produces wrong yaml
  // if the key/value is the only thing in the map and the map is used in
  // a sequence.  This detects if the this sequence is the first key/value
  // in map that itself is embedded in a sequence.
  ಯದಿ (State_ರಾಶಿ.size() < 2)
    ಪ್ರತ್ಯಾಗಮ ಸತ್ಯ;
  ಯದಿ (State_ರಾಶಿ.back() != in_Map_First_ಕೀಲಿ)
    ಪ್ರತ್ಯಾಗಮ ಸತ್ಯ;
  ಪ್ರತ್ಯಾಗಮ !inSeqAnyElement(State_ರಾಶಿ[State_ರಾಶಿ.size() - 2]);
}

ಶೂನ್ಯ Output::output(StringRef s) {
  Column += s.size();
  Out << s;
}

ಶೂನ್ಯ Output::output(StringRef S, QuotingType MustQuote) {
  ಯದಿ (MustQuote == QuotingType::None) {
    // Only quote if we must.
    output(S);
    ಪ್ರತ್ಯಾಗಮ;
  }

  StringLiteral Quote = MustQuote == QuotingType::Single ? StringLiteral("'")
                                                         : StringLiteral("\"");
  output(Quote); // Starting quote.

  // When using double-quoted strings (and only in that case), non-printable
  // characters may be present, and will be escaped using a variety of
  // unicode-scalar and special short-form escapes. This is handled in
  // yaml::escape.
  ಯದಿ (MustQuote == QuotingType::Double) {
    output(yaml::escape(S, /* EscapePrintable= */ ಅಸತ್ಯ));
    output(Quote);
    ಪ್ರತ್ಯಾಗಮ;
  }

  unsigned i = 0;
  unsigned j = 0;
  unsigned End = S.size();
  ಧ್ರುವ ವರ್ಣ *Base = S.data();

  // When using single-quoted strings, any single quote ' must be doubled to be
  // escaped.
  ಯಾವತ್ (j < End) {
    ಯದಿ (S[j] == '\'') {                   // Escape quotes.
      output(StringRef(&Base[i], j - i)); // "flush".
      output(StringLiteral("''"));        // Print it as ''
      i = j + 1;
    }
    ++j;
  }
  output(StringRef(&Base[i], j - i));
  output(Quote); // Ending quote.
}

ಶೂನ್ಯ Output::output_Up_To_End_Of_ಸಾಲು(StringRef s) {
  output(s);
  ಯದಿ (State_ರಾಶಿ.empty() || (!inFlowSeqAnyElement(State_ರಾಶಿ.back()) &&
                             !in_Flow_Map_Any_ಕೀಲಿ(State_ರಾಶಿ.back())))
    Padding = "\n";
}

ಶೂನ್ಯ Output::output_New_ಸಾಲು() {
  Out << "\n";
  Column = 0;
}

// if seq at top, indent as if map, then add "- "
// if seq in middle, use "- " if firstKey, else use "  "
//

ಶೂನ್ಯ Output::new_ಸಾಲು_Check(ತಾರ್ಕಿಕ EmptySequence) {
  ಯದಿ (Padding != "\n") {
    output(Padding);
    Padding = {};
    ಪ್ರತ್ಯಾಗಮ;
  }
  output_New_ಸಾಲು();
  Padding = {};

  ಯದಿ (State_ರಾಶಿ.size() == 0 || EmptySequence)
    ಪ್ರತ್ಯಾಗಮ;

  unsigned Indent = State_ರಾಶಿ.size() - 1;
  ತಾರ್ಕಿಕ PossiblyNestedSeq = ಅಸತ್ಯ;
  auto I = State_ರಾಶಿ.rbegin(), E = State_ರಾಶಿ.rend();

  ಯದಿ (inSeqAnyElement(*I)) {
    PossiblyNestedSeq = ಸತ್ಯ; // Not possibly but always.
    ++Indent;
  } ಅನ್ಯಥಾ ಯದಿ (*I == in_Map_First_ಕೀಲಿ || *I == in_Flow_Map_First_ಕೀಲಿ ||
             inFlowSeqAnyElement(*I)) {
    PossiblyNestedSeq = ಸತ್ಯ;
    ++I; // Skip back().
  }

  unsigned Output_Dash_ಎಣಿಕೆ = 0;
  ಯದಿ (PossiblyNestedSeq) {
    // Count up consecutive inSeqFirstElement from the end, unless
    // inSeqFirstElement is the top of nested sequence.
    ಯಾವತ್ (I != E) {
      // Don't count the top of nested sequence.
      ಯದಿ (!inSeqAnyElement(*I))
        ಭಙ್ಗ;

      ++Output_Dash_ಎಣಿಕೆ;

      // Stop counting if consecutive inSeqFirstElement ends.
      ಯದಿ (*I++ != inSeqFirstElement)
        ಭಙ್ಗ;
    }
  }

  ಪ್ರತಿ (unsigned I = Output_Dash_ಎಣಿಕೆ; I < Indent; ++I)
    output("  ");

  ಪ್ರತಿ (unsigned I = 0; I < Output_Dash_ಎಣಿಕೆ; ++I)
    output("- ");
}

ಶೂನ್ಯ Output::padded_ಕೀಲಿ(StringRef ಕೀಲಿ_2) {
  output(ಕೀಲಿ_2, needsQuotes(ಕೀಲಿ_2, ಅಸತ್ಯ));
  output(":");
  ಧ್ರುವ ವರ್ಣ *spaces = "                ";
  ಯದಿ (ಕೀಲಿ_2.size() < strlen(spaces))
    Padding = &spaces[ಕೀಲಿ_2.size()];
  ಅನ್ಯಥಾ
    Padding = " ";
}

ಶೂನ್ಯ Output::flow_ಕೀಲಿ(StringRef ಕೀಲಿ) {
  ಯದಿ (State_ರಾಶಿ.back() == in_Flow_Map_Other_ಕೀಲಿ)
    output(", ");
  ಯದಿ (WrapColumn && Column > WrapColumn) {
    output("\n");
    ಪ್ರತಿ (ಪೂರ್ಣಾಙ್ಕ I = 0; I < ColumnAtMapFlowStart; ++I)
      output(" ");
    Column = ColumnAtMapFlowStart;
    output("  ");
  }
  output(ಕೀಲಿ, needsQuotes(ಕೀಲಿ, ಅಸತ್ಯ));
  output(": ");
}

ಗಂಟು_Kind Output::get_ಗಂಟು_Kind() { report_fatal_error("invalid call"); }

ತಾರ್ಕಿಕ Output::inSeqAnyElement(InState State) {
  ಪ್ರತ್ಯಾಗಮ State == inSeqFirstElement || State == inSeqOtherElement;
}

ತಾರ್ಕಿಕ Output::inFlowSeqAnyElement(InState State) {
  ಪ್ರತ್ಯಾಗಮ State == inFlowSeqFirstElement || State == inFlowSeqOtherElement;
}

ತಾರ್ಕಿಕ Output::in_Map_Any_ಕೀಲಿ(InState State) {
  ಪ್ರತ್ಯಾಗಮ State == in_Map_First_ಕೀಲಿ || State == in_Map_Other_ಕೀಲಿ;
}

ತಾರ್ಕಿಕ Output::in_Flow_Map_Any_ಕೀಲಿ(InState State) {
  ಪ್ರತ್ಯಾಗಮ State == in_Flow_Map_First_ಕೀಲಿ || State == in_Flow_Map_Other_ಕೀಲಿ;
}

//===----------------------------------------------------------------------===//
//  traits for built-in types
//===----------------------------------------------------------------------===//

ಶೂನ್ಯ ScalarTraits<ತಾರ್ಕಿಕ>::output(ಧ್ರುವ ತಾರ್ಕಿಕ &Val, ಶೂನ್ಯ *, raw_ostream &Out) {
  Out << (Val ? "true" : "false");
}

StringRef ScalarTraits<ತಾರ್ಕಿಕ>::ಒಳಹರಿವು_2(StringRef Scalar, ಶೂನ್ಯ *, ತಾರ್ಕಿಕ &Val) {
  ಯದಿ (std::optional<ತಾರ್ಕಿಕ> Parsed = parseBool(Scalar)) {
    Val = *Parsed;
    ಪ್ರತ್ಯಾಗಮ StringRef();
  }
  ಪ್ರತ್ಯಾಗಮ "invalid boolean";
}

ಶೂನ್ಯ ScalarTraits<StringRef>::output(ಧ್ರುವ StringRef &Val, ಶೂನ್ಯ *,
                                     raw_ostream &Out) {
  Out << Val;
}

StringRef ScalarTraits<StringRef>::ಒಳಹರಿವು_2(StringRef Scalar, ಶೂನ್ಯ *,
                                         StringRef &Val) {
  Val = Scalar;
  ಪ್ರತ್ಯಾಗಮ StringRef();
}

ಶೂನ್ಯ ScalarTraits<std::string>::output(ಧ್ರುವ std::string &Val, ಶೂನ್ಯ *,
                                       raw_ostream &Out) {
  Out << Val;
}

StringRef ScalarTraits<std::string>::ಒಳಹರಿವು_2(StringRef Scalar, ಶೂನ್ಯ *,
                                           std::string &Val) {
  Val = Scalar.str();
  ಪ್ರತ್ಯಾಗಮ StringRef();
}

ಶೂನ್ಯ ScalarTraits<uint8_t>::output(ಧ್ರುವ uint8_t &Val, ಶೂನ್ಯ *,
                                   raw_ostream &Out) {
  // use temp uin32_t because ostream thinks uint8_t is a character
  uint32_t ಅಂಕೆ = Val;
  Out << ಅಂಕೆ;
}

StringRef ScalarTraits<uint8_t>::ಒಳಹರಿವು_2(StringRef Scalar, ಶೂನ್ಯ *, uint8_t &Val) {
  unsigned long long n;
  ಯದಿ (getAsUnsignedInteger(Scalar, 0, n))
    ಪ್ರತ್ಯಾಗಮ "invalid number";
  ಯದಿ (n > 0xFF)
    ಪ್ರತ್ಯಾಗಮ "out of range number";
  Val = n;
  ಪ್ರತ್ಯಾಗಮ StringRef();
}

ಶೂನ್ಯ ScalarTraits<uint16_t>::output(ಧ್ರುವ uint16_t &Val, ಶೂನ್ಯ *,
                                    raw_ostream &Out) {
  Out << Val;
}

StringRef ScalarTraits<uint16_t>::ಒಳಹರಿವು_2(StringRef Scalar, ಶೂನ್ಯ *,
                                        uint16_t &Val) {
  unsigned long long n;
  ಯದಿ (getAsUnsignedInteger(Scalar, 0, n))
    ಪ್ರತ್ಯಾಗಮ "invalid number";
  ಯದಿ (n > 0xFFFF)
    ಪ್ರತ್ಯಾಗಮ "out of range number";
  Val = n;
  ಪ್ರತ್ಯಾಗಮ StringRef();
}

ಶೂನ್ಯ ScalarTraits<uint32_t>::output(ಧ್ರುವ uint32_t &Val, ಶೂನ್ಯ *,
                                    raw_ostream &Out) {
  Out << Val;
}

StringRef ScalarTraits<uint32_t>::ಒಳಹರಿವು_2(StringRef Scalar, ಶೂನ್ಯ *,
                                        uint32_t &Val) {
  unsigned long long n;
  ಯದಿ (getAsUnsignedInteger(Scalar, 0, n))
    ಪ್ರತ್ಯಾಗಮ "invalid number";
  ಯದಿ (n > 0xFFFFFFFFUL)
    ಪ್ರತ್ಯಾಗಮ "out of range number";
  Val = n;
  ಪ್ರತ್ಯಾಗಮ StringRef();
}

ಶೂನ್ಯ ScalarTraits<uint64_t>::output(ಧ್ರುವ uint64_t &Val, ಶೂನ್ಯ *,
                                    raw_ostream &Out) {
  Out << Val;
}

StringRef ScalarTraits<uint64_t>::ಒಳಹರಿವು_2(StringRef Scalar, ಶೂನ್ಯ *,
                                        uint64_t &Val) {
  unsigned long long N;
  ಯದಿ (getAsUnsignedInteger(Scalar, 0, N))
    ಪ್ರತ್ಯಾಗಮ "invalid number";
  Val = N;
  ಪ್ರತ್ಯಾಗಮ StringRef();
}

ಶೂನ್ಯ ScalarTraits<int8_t>::output(ಧ್ರುವ int8_t &Val, ಶೂನ್ಯ *, raw_ostream &Out) {
  // use temp in32_t because ostream thinks int8_t is a character
  int32_t ಅಂಕೆ = Val;
  Out << ಅಂಕೆ;
}

StringRef ScalarTraits<int8_t>::ಒಳಹರಿವು_2(StringRef Scalar, ಶೂನ್ಯ *, int8_t &Val) {
  long long N;
  ಯದಿ (getAsSignedInteger(Scalar, 0, N))
    ಪ್ರತ್ಯಾಗಮ "invalid number";
  ಯದಿ ((N > 127) || (N < -128))
    ಪ್ರತ್ಯಾಗಮ "out of range number";
  Val = N;
  ಪ್ರತ್ಯಾಗಮ StringRef();
}

ಶೂನ್ಯ ScalarTraits<int16_t>::output(ಧ್ರುವ int16_t &Val, ಶೂನ್ಯ *,
                                   raw_ostream &Out) {
  Out << Val;
}

StringRef ScalarTraits<int16_t>::ಒಳಹರಿವು_2(StringRef Scalar, ಶೂನ್ಯ *, int16_t &Val) {
  long long N;
  ಯದಿ (getAsSignedInteger(Scalar, 0, N))
    ಪ್ರತ್ಯಾಗಮ "invalid number";
  ಯದಿ ((N > INT16_MAX) || (N < INT16_MIN))
    ಪ್ರತ್ಯಾಗಮ "out of range number";
  Val = N;
  ಪ್ರತ್ಯಾಗಮ StringRef();
}

ಶೂನ್ಯ ScalarTraits<int32_t>::output(ಧ್ರುವ int32_t &Val, ಶೂನ್ಯ *,
                                   raw_ostream &Out) {
  Out << Val;
}

StringRef ScalarTraits<int32_t>::ಒಳಹರಿವು_2(StringRef Scalar, ಶೂನ್ಯ *, int32_t &Val) {
  long long N;
  ಯದಿ (getAsSignedInteger(Scalar, 0, N))
    ಪ್ರತ್ಯಾಗಮ "invalid number";
  ಯದಿ ((N > INT32_MAX) || (N < INT32_MIN))
    ಪ್ರತ್ಯಾಗಮ "out of range number";
  Val = N;
  ಪ್ರತ್ಯಾಗಮ StringRef();
}

ಶೂನ್ಯ ScalarTraits<int64_t>::output(ಧ್ರುವ int64_t &Val, ಶೂನ್ಯ *,
                                   raw_ostream &Out) {
  Out << Val;
}

StringRef ScalarTraits<int64_t>::ಒಳಹರಿವು_2(StringRef Scalar, ಶೂನ್ಯ *, int64_t &Val) {
  long long N;
  ಯದಿ (getAsSignedInteger(Scalar, 0, N))
    ಪ್ರತ್ಯಾಗಮ "invalid number";
  Val = N;
  ಪ್ರತ್ಯಾಗಮ StringRef();
}

ಶೂನ್ಯ ScalarTraits<ದ್ವಿಗುಣ>::output(ಧ್ರುವ ದ್ವಿಗುಣ &Val, ಶೂನ್ಯ *, raw_ostream &Out) {
  Out << format("%g", Val);
}

StringRef ScalarTraits<ದ್ವಿಗುಣ>::ಒಳಹರಿವು_2(StringRef Scalar, ಶೂನ್ಯ *, ದ್ವಿಗುಣ &Val) {
  ಯದಿ (to_float(Scalar, Val))
    ಪ್ರತ್ಯಾಗಮ StringRef();
  ಪ್ರತ್ಯಾಗಮ "invalid floating point number";
}

ಶೂನ್ಯ ScalarTraits<ಭಿನ್ನ>::output(ಧ್ರುವ ಭಿನ್ನ &Val, ಶೂನ್ಯ *, raw_ostream &Out) {
  Out << format("%g", Val);
}

StringRef ScalarTraits<ಭಿನ್ನ>::ಒಳಹರಿವು_2(StringRef Scalar, ಶೂನ್ಯ *, ಭಿನ್ನ &Val) {
  ಯದಿ (to_float(Scalar, Val))
    ಪ್ರತ್ಯಾಗಮ StringRef();
  ಪ್ರತ್ಯಾಗಮ "invalid floating point number";
}

ಶೂನ್ಯ ScalarTraits<Hex8>::output(ಧ್ರುವ Hex8 &Val, ಶೂನ್ಯ *, raw_ostream &Out) {
  Out << format("0x%" PRIX8, (uint8_t)Val);
}

StringRef ScalarTraits<Hex8>::ಒಳಹರಿವು_2(StringRef Scalar, ಶೂನ್ಯ *, Hex8 &Val) {
  unsigned long long n;
  ಯದಿ (getAsUnsignedInteger(Scalar, 0, n))
    ಪ್ರತ್ಯಾಗಮ "invalid hex8 number";
  ಯದಿ (n > 0xFF)
    ಪ್ರತ್ಯಾಗಮ "out of range hex8 number";
  Val = n;
  ಪ್ರತ್ಯಾಗಮ StringRef();
}

ಶೂನ್ಯ ScalarTraits<Hex16>::output(ಧ್ರುವ Hex16 &Val, ಶೂನ್ಯ *, raw_ostream &Out) {
  Out << format("0x%" PRIX16, (uint16_t)Val);
}

StringRef ScalarTraits<Hex16>::ಒಳಹರಿವು_2(StringRef Scalar, ಶೂನ್ಯ *, Hex16 &Val) {
  unsigned long long n;
  ಯದಿ (getAsUnsignedInteger(Scalar, 0, n))
    ಪ್ರತ್ಯಾಗಮ "invalid hex16 number";
  ಯದಿ (n > 0xFFFF)
    ಪ್ರತ್ಯಾಗಮ "out of range hex16 number";
  Val = n;
  ಪ್ರತ್ಯಾಗಮ StringRef();
}

ಶೂನ್ಯ ScalarTraits<Hex32>::output(ಧ್ರುವ Hex32 &Val, ಶೂನ್ಯ *, raw_ostream &Out) {
  Out << format("0x%" PRIX32, (uint32_t)Val);
}

StringRef ScalarTraits<Hex32>::ಒಳಹರಿವು_2(StringRef Scalar, ಶೂನ್ಯ *, Hex32 &Val) {
  unsigned long long n;
  ಯದಿ (getAsUnsignedInteger(Scalar, 0, n))
    ಪ್ರತ್ಯಾಗಮ "invalid hex32 number";
  ಯದಿ (n > 0xFFFFFFFFUL)
    ಪ್ರತ್ಯಾಗಮ "out of range hex32 number";
  Val = n;
  ಪ್ರತ್ಯಾಗಮ StringRef();
}

ಶೂನ್ಯ ScalarTraits<Hex64>::output(ಧ್ರುವ Hex64 &Val, ಶೂನ್ಯ *, raw_ostream &Out) {
  Out << format("0x%" PRIX64, (uint64_t)Val);
}

StringRef ScalarTraits<Hex64>::ಒಳಹರಿವು_2(StringRef Scalar, ಶೂನ್ಯ *, Hex64 &Val) {
  unsigned long long ಅಂಕೆ;
  ಯದಿ (getAsUnsignedInteger(Scalar, 0, ಅಂಕೆ))
    ಪ್ರತ್ಯಾಗಮ "invalid hex64 number";
  Val = ಅಂಕೆ;
  ಪ್ರತ್ಯಾಗಮ StringRef();
}

ಶೂನ್ಯ ScalarTraits<VersionTuple>::output(ಧ್ರುವ VersionTuple &Val, ಶೂನ್ಯ *,
                                        llvm::raw_ostream &Out) {
  Out << Val.getAsString();
}

StringRef ScalarTraits<VersionTuple>::ಒಳಹರಿವು_2(StringRef Scalar, ಶೂನ್ಯ *,
                                            VersionTuple &Val) {
  ಯದಿ (Val.tryParse(Scalar))
    ಪ್ರತ್ಯಾಗಮ "invalid version format";
  ಪ್ರತ್ಯಾಗಮ StringRef();
}
