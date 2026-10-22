//===- lib/Support/YAMLTraits.cpp -----------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ଅନ୍ତର୍ଭାଵ "llvm/Support/YAMLTraits.h"
#ଅନ୍ତର୍ଭାଵ "llvm/ADT/STLExtras.h"
#ଅନ୍ତର୍ଭାଵ "llvm/ADT/SmallString.h"
#ଅନ୍ତର୍ଭାଵ "llvm/ADT/StringExtras.h"
#ଅନ୍ତର୍ଭାଵ "llvm/ADT/StringRef.h"
#ଅନ୍ତର୍ଭାଵ "llvm/ADT/Twine.h"
#ଅନ୍ତର୍ଭାଵ "llvm/Support/Casting.h"
#ଅନ୍ତର୍ଭାଵ "llvm/Support/Errc.h"
#ଅନ୍ତର୍ଭାଵ "llvm/Support/ErrorHandling.h"
#ଅନ୍ତର୍ଭାଵ "llvm/Support/Format.h"
#ଅନ୍ତର୍ଭାଵ "llvm/Support/LineIterator.h"
#ଅନ୍ତର୍ଭାଵ "llvm/Support/MemoryBuffer.h"
#ଅନ୍ତର୍ଭାଵ "llvm/Support/VersionTuple.h"
#ଅନ୍ତର୍ଭାଵ "llvm/Support/YAMLParser.h"
#ଅନ୍ତର୍ଭାଵ "llvm/Support/raw_ostream.h"
#ଅନ୍ତର୍ଭାଵ <cassert>
#ଅନ୍ତର୍ଭାଵ <cstdint>
#ଅନ୍ତର୍ଭାଵ <cstring>
#ଅନ୍ତର୍ଭାଵ <string>
#ଅନ୍ତର୍ଭାଵ <vector>

using ନାମାକାଶ llvm;
using ନାମାକାଶ yaml;

//===----------------------------------------------------------------------===//
//  IO
//===----------------------------------------------------------------------===//

IO::IO(ଶୂନ୍ୟ *Context) : Ctxt(Context) {}

IO::~IO() = ପୂର୍ବନିର୍ଦ୍ଧାରିତ;

ଶୂନ୍ୟ *IO::getContext() ଅଚଳ {
  ଫେରନ୍ତୁ Ctxt;
}

ଶୂନ୍ୟ IO::setContext(ଶୂନ୍ୟ *Context) {
  Ctxt = Context;
}

ଶୂନ୍ୟ IO::set_Allow_Unknown_ଚାବିଗୁଡ଼ିକ(ତାର୍କିକ Allow) {
  llvm_unreachable("Only supported for Input");
}

//===----------------------------------------------------------------------===//
//ନିବେଶ
//===----------------------------------------------------------------------===//

ନିବେଶ::ନିବେଶ(StringRef ନିବେଶ_Content, ଶୂନ୍ୟ *Ctxt,
             SourceMgr::DiagHandlerTy DiagHandler, ଶୂନ୍ୟ *DiagHandlerCtxt)
    : IO(Ctxt), Strm(ନୂତନ Stream(ନିବେଶ_Content, SrcMgr, ମିଥ୍ୟା, &EC)) {
  ଯଦି (DiagHandler)
    SrcMgr.setDiagHandler(DiagHandler, DiagHandlerCtxt);
  DocIterator = Strm->begin();
}

ନିବେଶ::ନିବେଶ(MemoryBufferRef ନିବେଶ, ଶୂନ୍ୟ *Ctxt,
             SourceMgr::DiagHandlerTy DiagHandler, ଶୂନ୍ୟ *DiagHandlerCtxt)
    : IO(Ctxt), Strm(ନୂତନ Stream(ନିବେଶ, SrcMgr, ମିଥ୍ୟା, &EC)) {
  ଯଦି (DiagHandler)
    SrcMgr.setDiagHandler(DiagHandler, DiagHandlerCtxt);
  DocIterator = Strm->begin();
}

ନିବେଶ::~ନିବେଶ() = ପୂର୍ବନିର୍ଦ୍ଧାରିତ;

std::error_code ନିବେଶ::error() { ଫେରନ୍ତୁ EC; }

ତାର୍କିକ ନିବେଶ::outputting() ଅଚଳ {
  ଫେରନ୍ତୁ ମିଥ୍ୟା;
}

ତାର୍କିକ ନିବେଶ::setCurrentDocument() {
  ଯଦି (DocIterator != Strm->end()) {
    ଗଣ୍ଠି *N = DocIterator->get_ମୂଳ();
    ଯଦି (!N) {
      EC = make_error_code(errc::invalid_argument);
      ଫେରନ୍ତୁ ମିଥ୍ୟା;
    }

    ଯଦି (isa<Null_ଗଣ୍ଠି>(N)) {
      // Empty files are allowed and ignored
      ++DocIterator;
      ଫେରନ୍ତୁ setCurrentDocument();
    }
    releaseHNodeBuffers();
    ଶୀର୍ଷ_ଗଣ୍ଠି = createHNodes(N);
    Current_ଗଣ୍ଠି = ଶୀର୍ଷ_ଗଣ୍ଠି;
    ଫେରନ୍ତୁ ସତ୍ୟ;
  }
  ଫେରନ୍ତୁ ମିଥ୍ୟା;
}

ତାର୍କିକ ନିବେଶ::nextDocument() {
  ଫେରନ୍ତୁ ++DocIterator != Strm->end();
}

ଅଚଳ ଗଣ୍ଠି *ନିବେଶ::get_Current_ଗଣ୍ଠି() ଅଚଳ {
  ଫେରନ୍ତୁ Current_ଗଣ୍ଠି ? Current_ଗଣ୍ଠି->ଗଣ୍ଠି_2 : nullptr;
}

ତାର୍କିକ ନିବେଶ::mapTag(StringRef Tag, ତାର୍କିକ Default) {
  // CurrentNode can be null if setCurrentDocument() was unable to
  // parse the document because it was invalid or empty.
  ଯଦି (!Current_ଗଣ୍ଠି)
    ଫେରନ୍ତୁ ମିଥ୍ୟା;

  std::string foundTag = Current_ଗଣ୍ଠି->ଗଣ୍ଠି_2->getVerbatimTag();
  ଯଦି (foundTag.empty()) {
    // If no tag found and 'Tag' is the default, say it was found.
    ଫେରନ୍ତୁ Default;
  }
  // Return true iff found tag matches supplied tag.
  ଫେରନ୍ତୁ Tag == foundTag;
}

ଶୂନ୍ୟ ନିବେଶ::beginMapping() {
  ଯଦି (EC)
    ଫେରନ୍ତୁ;
  // CurrentNode can be null if the document is empty.
  MapHNode *MN = dyn_cast_or_null<MapHNode>(Current_ଗଣ୍ଠି);
  ଯଦି (MN) {
    MN->Valid_ଚାବିଗୁଡ଼ିକ.clear();
  }
}

std::vector<StringRef> ନିବେଶ::ଚାବିଗୁଡ଼ିକ() {
  MapHNode *MN = dyn_cast<MapHNode>(Current_ଗଣ୍ଠି);
  std::vector<StringRef> Ret;
  ଯଦି (!MN) {
    setError(Current_ଗଣ୍ଠି, "not a mapping");
    ଫେରନ୍ତୁ Ret;
  }
  ପାଇଁ (auto &P : MN->Mapping)
    Ret.ଠେଲ_back(P.first());
  ଫେରନ୍ତୁ Ret;
}

ତାର୍କିକ ନିବେଶ::preflight_ଚାବି(StringRef ଚାବି, ତାର୍କିକ Required, ତାର୍କିକ, ତାର୍କିକ &UseDefault,
                         ଶୂନ୍ୟ *&SaveInfo) {
  UseDefault = ମିଥ୍ୟା;
  ଯଦି (EC)
    ଫେରନ୍ତୁ ମିଥ୍ୟା;

  // CurrentNode is null for empty documents, which is an error in case required
  // nodes are present.
  ଯଦି (!Current_ଗଣ୍ଠି) {
    ଯଦି (Required)
      EC = make_error_code(errc::invalid_argument);
    ଅନ୍ୟଥା
      UseDefault = ସତ୍ୟ;
    ଫେରନ୍ତୁ ମିଥ୍ୟା;
  }

  MapHNode *MN = dyn_cast<MapHNode>(Current_ଗଣ୍ଠି);
  ଯଦି (!MN) {
    ଯଦି (Required || !isa<EmptyHNode>(Current_ଗଣ୍ଠି))
      setError(Current_ଗଣ୍ଠି, "not a mapping");
    ଅନ୍ୟଥା
      UseDefault = ସତ୍ୟ;
    ଫେରନ୍ତୁ ମିଥ୍ୟା;
  }
  MN->Valid_ଚାବିଗୁଡ଼ିକ.ଠେଲ_back(ଚାବି.str());
  HNode *ମୂଲ୍ୟ = MN->Mapping[ଚାବି].first;
  ଯଦି (!ମୂଲ୍ୟ) {
    ଯଦି (Required)
      setError(Current_ଗଣ୍ଠି, Twine("missing required key '") + ଚାବି + "'");
    ଅନ୍ୟଥା
      UseDefault = ସତ୍ୟ;
    ଫେରନ୍ତୁ ମିଥ୍ୟା;
  }
  SaveInfo = Current_ଗଣ୍ଠି;
  Current_ଗଣ୍ଠି = ମୂଲ୍ୟ;
  ଫେରନ୍ତୁ ସତ୍ୟ;
}

ଶୂନ୍ୟ ନିବେଶ::postflight_ଚାବି(ଶୂନ୍ୟ *saveInfo) {
  Current_ଗଣ୍ଠି = reinterpret_cast<HNode *>(saveInfo);
}

ଶୂନ୍ୟ ନିବେଶ::endMapping() {
  ଯଦି (EC)
    ଫେରନ୍ତୁ;
  // CurrentNode can be null if the document is empty.
  MapHNode *MN = dyn_cast_or_null<MapHNode>(Current_ଗଣ୍ଠି);
  ଯଦି (!MN)
    ଫେରନ୍ତୁ;
  ପାଇଁ (ଅଚଳ auto &NN : MN->Mapping) {
    ଯଦି (!is_contained(MN->Valid_ଚାବିଗୁଡ଼ିକ, NN.first())) {
      ଅଚଳ SMRange &ReportLoc = NN.second.second;
      ଯଦି (!Allow_Unknown_ଚାବିଗୁଡ଼ିକ) {
        setError(ReportLoc, Twine("unknown key '") + NN.first() + "'");
        ଭାଙ୍ଗ;
      } ଅନ୍ୟଥା
        reportWarning(ReportLoc, Twine("unknown key '") + NN.first() + "'");
    }
  }
}

ଶୂନ୍ୟ ନିବେଶ::beginFlowMapping() { beginMapping(); }

ଶୂନ୍ୟ ନିବେଶ::endFlowMapping() { endMapping(); }

unsigned ନିବେଶ::beginSequence() {
  ଯଦି (SequenceHNode *SQ = dyn_cast<SequenceHNode>(Current_ଗଣ୍ଠି))
    ଫେରନ୍ତୁ SQ->Entries.size();
  ଯଦି (isa<EmptyHNode>(Current_ଗଣ୍ଠି))
    ଫେରନ୍ତୁ 0;
  // Treat case where there's a scalar "null" value as an empty sequence.
  ଯଦି (ScalarHNode *SN = dyn_cast<ScalarHNode>(Current_ଗଣ୍ଠି)) {
    ଯଦି (isNull(SN->ମୂଲ୍ୟ_2()))
      ଫେରନ୍ତୁ 0;
  }
  // Any other type of HNode is an error.
  setError(Current_ଗଣ୍ଠି, "not a sequence");
  ଫେରନ୍ତୁ 0;
}

ଶୂନ୍ୟ ନିବେଶ::endSequence() {
}

ତାର୍କିକ ନିବେଶ::preflightElement(unsigned Index, ଶୂନ୍ୟ *&SaveInfo) {
  ଯଦି (EC)
    ଫେରନ୍ତୁ ମିଥ୍ୟା;
  ଯଦି (SequenceHNode *SQ = dyn_cast<SequenceHNode>(Current_ଗଣ୍ଠି)) {
    SaveInfo = Current_ଗଣ୍ଠି;
    Current_ଗଣ୍ଠି = SQ->Entries[Index];
    ଫେରନ୍ତୁ ସତ୍ୟ;
  }
  ଫେରନ୍ତୁ ମିଥ୍ୟା;
}

ଶୂନ୍ୟ ନିବେଶ::postflightElement(ଶୂନ୍ୟ *SaveInfo) {
  Current_ଗଣ୍ଠି = reinterpret_cast<HNode *>(SaveInfo);
}

unsigned ନିବେଶ::beginFlowSequence() { ଫେରନ୍ତୁ beginSequence(); }

ତାର୍କିକ ନିବେଶ::preflightFlowElement(unsigned index, ଶୂନ୍ୟ *&SaveInfo) {
  ଯଦି (EC)
    ଫେରନ୍ତୁ ମିଥ୍ୟା;
  ଯଦି (SequenceHNode *SQ = dyn_cast<SequenceHNode>(Current_ଗଣ୍ଠି)) {
    SaveInfo = Current_ଗଣ୍ଠି;
    Current_ଗଣ୍ଠି = SQ->Entries[index];
    ଫେରନ୍ତୁ ସତ୍ୟ;
  }
  ଫେରନ୍ତୁ ମିଥ୍ୟା;
}

ଶୂନ୍ୟ ନିବେଶ::postflightFlowElement(ଶୂନ୍ୟ *SaveInfo) {
  Current_ଗଣ୍ଠି = reinterpret_cast<HNode *>(SaveInfo);
}

ଶୂନ୍ୟ ନିବେଶ::endFlowSequence() {
}

ଶୂନ୍ୟ ନିବେଶ::beginEnumScalar() {
  ScalarMatchFound = ମିଥ୍ୟା;
}

ତାର୍କିକ ନିବେଶ::matchEnumScalar(StringRef Str, ତାର୍କିକ) {
  ଯଦି (ScalarMatchFound)
    ଫେରନ୍ତୁ ମିଥ୍ୟା;
  ଯଦି (ScalarHNode *SN = dyn_cast<ScalarHNode>(Current_ଗଣ୍ଠି)) {
    ଯଦି (SN->ମୂଲ୍ୟ_2() == Str) {
      ScalarMatchFound = ସତ୍ୟ;
      ଫେରନ୍ତୁ ସତ୍ୟ;
    }
  }
  ଫେରନ୍ତୁ ମିଥ୍ୟା;
}

ତାର୍କିକ ନିବେଶ::matchEnumFallback() {
  ଯଦି (ScalarMatchFound)
    ଫେରନ୍ତୁ ମିଥ୍ୟା;
  ScalarMatchFound = ସତ୍ୟ;
  ଫେରନ୍ତୁ ସତ୍ୟ;
}

ଶୂନ୍ୟ ନିବେଶ::endEnumScalar() {
  ଯଦି (!ScalarMatchFound) {
    setError(Current_ଗଣ୍ଠି, "unknown enumerated scalar");
  }
}

ତାର୍କିକ ନିବେଶ::beginBitSetScalar(ତାର୍କିକ &DoClear) {
  Bit_ମୂଲ୍ୟଗୁଡ଼ିକ_Used.clear();
  ଯଦି (SequenceHNode *SQ = dyn_cast<SequenceHNode>(Current_ଗଣ୍ଠି)) {
    Bit_ମୂଲ୍ୟଗୁଡ଼ିକ_Used.resize(SQ->Entries.size());
  } ଅନ୍ୟଥା {
    setError(Current_ଗଣ୍ଠି, "expected sequence of bit values");
  }
  DoClear = ସତ୍ୟ;
  ଫେରନ୍ତୁ ସତ୍ୟ;
}

ତାର୍କିକ ନିବେଶ::bitSetMatch(StringRef Str, ତାର୍କିକ) {
  ଯଦି (EC)
    ଫେରନ୍ତୁ ମିଥ୍ୟା;
  ଯଦି (SequenceHNode *SQ = dyn_cast<SequenceHNode>(Current_ଗଣ୍ଠି)) {
    unsigned Index = 0;
    ପାଇଁ (auto &N : SQ->Entries) {
      ଯଦି (ScalarHNode *SN = dyn_cast<ScalarHNode>(N)) {
        ଯଦି (SN->ମୂଲ୍ୟ_2() == Str) {
          Bit_ମୂଲ୍ୟଗୁଡ଼ିକ_Used[Index] = ସତ୍ୟ;
          ଫେରନ୍ତୁ ସତ୍ୟ;
        }
      } ଅନ୍ୟଥା {
        setError(Current_ଗଣ୍ଠି, "unexpected scalar in sequence of bit values");
      }
      ++Index;
    }
  } ଅନ୍ୟଥା {
    setError(Current_ଗଣ୍ଠି, "expected sequence of bit values");
  }
  ଫେରନ୍ତୁ ମିଥ୍ୟା;
}

ଶୂନ୍ୟ ନିବେଶ::endBitSetScalar() {
  ଯଦି (EC)
    ଫେରନ୍ତୁ;
  ଯଦି (SequenceHNode *SQ = dyn_cast<SequenceHNode>(Current_ଗଣ୍ଠି)) {
    ପ୍ରତିଜ୍ଞା(Bit_ମୂଲ୍ୟଗୁଡ଼ିକ_Used.size() == SQ->Entries.size());
    ପାଇଁ (unsigned i = 0; i < SQ->Entries.size(); ++i) {
      ଯଦି (!Bit_ମୂଲ୍ୟଗୁଡ଼ିକ_Used[i]) {
        setError(SQ->Entries[i], "unknown bit value");
        ଫେରନ୍ତୁ;
      }
    }
  }
}

ଶୂନ୍ୟ ନିବେଶ::scalarString(StringRef &S, QuotingType) {
  ଯଦି (ScalarHNode *SN = dyn_cast<ScalarHNode>(Current_ଗଣ୍ଠି)) {
    S = SN->ମୂଲ୍ୟ_2();
  } ଅନ୍ୟଥା {
    setError(Current_ଗଣ୍ଠି, "unexpected scalar");
  }
}

ଶୂନ୍ୟ ନିବେଶ::blockScalarString(StringRef &S) { scalarString(S, QuotingType::None); }

ଶୂନ୍ୟ ନିବେଶ::scalarTag(std::string &Tag) {
  Tag = Current_ଗଣ୍ଠି->ଗଣ୍ଠି_2->getVerbatimTag();
}

ଶୂନ୍ୟ ନିବେଶ::setError(HNode *hnode, ଅଚଳ Twine &message) {
  ପ୍ରତିଜ୍ଞା(hnode && "HNode must not be NULL");
  setError(hnode->ଗଣ୍ଠି_2, message);
}

ଗଣ୍ଠି_Kind ନିବେଶ::get_ଗଣ୍ଠି_Kind() {
  ଯଦି (isa<ScalarHNode>(Current_ଗଣ୍ଠି))
    ଫେରନ୍ତୁ ଗଣ୍ଠି_Kind::Scalar;
  ଅନ୍ୟଥା ଯଦି (isa<MapHNode>(Current_ଗଣ୍ଠି))
    ଫେରନ୍ତୁ ଗଣ୍ଠି_Kind::Map;
  ଅନ୍ୟଥା ଯଦି (isa<SequenceHNode>(Current_ଗଣ୍ଠି))
    ଫେରନ୍ତୁ ଗଣ୍ଠି_Kind::Sequence;
  llvm_unreachable("Unsupported node kind");
}

ଶୂନ୍ୟ ନିବେଶ::setError(ଗଣ୍ଠି *ଗଣ୍ଠି_3, ଅଚଳ Twine &message) {
  Strm->printError(ଗଣ୍ଠି_3, message);
  EC = make_error_code(errc::invalid_argument);
}

ଶୂନ୍ୟ ନିବେଶ::setError(ଅଚଳ SMRange &range, ଅଚଳ Twine &message) {
  Strm->printError(range, message);
  EC = make_error_code(errc::invalid_argument);
}

ଶୂନ୍ୟ ନିବେଶ::reportWarning(HNode *hnode, ଅଚଳ Twine &message) {
  ପ୍ରତିଜ୍ଞା(hnode && "HNode must not be NULL");
  Strm->printError(hnode->ଗଣ୍ଠି_2, message, SourceMgr::DK_Warning);
}

ଶୂନ୍ୟ ନିବେଶ::reportWarning(ଗଣ୍ଠି *ଗଣ୍ଠି_3, ଅଚଳ Twine &message) {
  Strm->printError(ଗଣ୍ଠି_3, message, SourceMgr::DK_Warning);
}

ଶୂନ୍ୟ ନିବେଶ::reportWarning(ଅଚଳ SMRange &range, ଅଚଳ Twine &message) {
  Strm->printError(range, message, SourceMgr::DK_Warning);
}

ଶୂନ୍ୟ ନିବେଶ::releaseHNodeBuffers() {
  EmptyHNodeAllocator.DestroyAll();
  ScalarHNodeAllocator.DestroyAll();
  SequenceHNodeAllocator.DestroyAll();
  MapHNodeAllocator.DestroyAll();
}

ଶୂନ୍ୟ ନିବେଶ::saveAliasHNode(ଗଣ୍ଠି *N, HNode *HN) {
  StringRef Anchor = N->getAnchor();
  ଯଦି (!Anchor.empty())
    // YAML 1.2.2 - 3.2.2.2. Anchors and Aliases:
    //
    // An alias event refers to the most recent event in the serialization
    // having the specified anchor. Therefore, anchors need not be unique within
    // a serialization. In addition, an anchor need not have an alias node
    // referring to it.
    AliasMap[Anchor] = HN;
}

ନିବେଶ::HNode *ନିବେଶ::createHNodes(ଗଣ୍ଠି *N) {
  SmallString<128> StringStorage;
  ବାଛ (N->getType()) {
  ସ୍ଥିତି ଗଣ୍ଠି::NK_Scalar: {
    Scalar_ଗଣ୍ଠି *SN = dyn_cast<Scalar_ଗଣ୍ଠି>(N);
    StringRef ଚାବି_Str = SN->get_ମୂଲ୍ୟ(StringStorage);
    ଯଦି (!StringStorage.empty()) {
      // Copy string to permanent storage
      ଚାବି_Str = StringStorage.str().copy(StringAllocator);
    }
    auto *SHNode = ନୂତନ (ScalarHNodeAllocator.Allocate()) ScalarHNode(N, ଚାବି_Str);
    saveAliasHNode(SN, SHNode);
    ଫେରନ୍ତୁ SHNode;
  }
  ସ୍ଥିତି ଗଣ୍ଠି::NK_BlockScalar: {
    Block_Scalar_ଗଣ୍ଠି *BSN = dyn_cast<Block_Scalar_ଗଣ୍ଠି>(N);
    StringRef ମୂଲ୍ୟ_Copy = BSN->get_ମୂଲ୍ୟ().copy(StringAllocator);
    auto *BSHNode =
        ନୂତନ (ScalarHNodeAllocator.Allocate()) ScalarHNode(N, ମୂଲ୍ୟ_Copy);
    saveAliasHNode(BSN, BSHNode);
    ଫେରନ୍ତୁ BSHNode;
  }
  ସ୍ଥିତି ଗଣ୍ଠି::NK_Sequence: {
    Sequence_ଗଣ୍ଠି *SQ = dyn_cast<Sequence_ଗଣ୍ଠି>(N);
    auto SQHNode = ନୂତନ (SequenceHNodeAllocator.Allocate()) SequenceHNode(N);
    ପାଇଁ (ଗଣ୍ଠି &SN : *SQ) {
      auto Entry = createHNodes(&SN);
      ଯଦି (EC)
        ଭାଙ୍ଗ;
      SQHNode->Entries.ଠେଲ_back(Entry);
    }
    saveAliasHNode(SQ, SQHNode);
    ଫେରନ୍ତୁ SQHNode;
  }
  ସ୍ଥିତି ଗଣ୍ଠି::NK_Mapping: {
    Mapping_ଗଣ୍ଠି *Map = dyn_cast<Mapping_ଗଣ୍ଠି>(N);
    auto mapHNode = ନୂତନ (MapHNodeAllocator.Allocate()) MapHNode(N);
    ପାଇଁ (ଚାବି_ମୂଲ୍ୟ_ଗଣ୍ଠି &KVN : *Map) {
      ଗଣ୍ଠି *ଚାବି_ଗଣ୍ଠି = KVN.get_ଚାବି();
      Scalar_ଗଣ୍ଠି *ଚାବି = dyn_cast_if_present<Scalar_ଗଣ୍ଠି>(ଚାବି_ଗଣ୍ଠି);
      ଗଣ୍ଠି *ମୂଲ୍ୟ = KVN.get_ମୂଲ୍ୟ();
      ଯଦି (!ଚାବି || !ମୂଲ୍ୟ) {
        ଯଦି (!ଚାବି)
          setError(ଚାବି_ଗଣ୍ଠି, "Map key must be a scalar");
        ଯଦି (!ମୂଲ୍ୟ)
          setError(ଚାବି_ଗଣ୍ଠି, "Map value must not be empty");
        ଭାଙ୍ଗ;
      }
      StringStorage.clear();
      StringRef ଚାବି_Str = ଚାବି->get_ମୂଲ୍ୟ(StringStorage);
      ଯଦି (!StringStorage.empty()) {
        // Copy string to permanent storage
        ଚାବି_Str = StringStorage.str().copy(StringAllocator);
      }
      ଯଦି (mapHNode->Mapping.ଗଣନା_2(ଚାବି_Str))
        // From YAML spec: "The content of a mapping node is an unordered set of
        // key/value node pairs, with the restriction that each of the keys is
        // unique."
        setError(ଚାବି_ଗଣ୍ଠି, Twine("duplicated mapping key '") + ଚାବି_Str + "'");
      auto ମୂଲ୍ୟ_HNode = createHNodes(ମୂଲ୍ୟ);
      ଯଦି (EC)
        ଭାଙ୍ଗ;
      mapHNode->Mapping[ଚାବି_Str] =
          std::make_pair(std::move(ମୂଲ୍ୟ_HNode), ଚାବି_ଗଣ୍ଠି->getSourceRange());
    }
    saveAliasHNode(Map, mapHNode);
    ଫେରନ୍ତୁ std::move(mapHNode);
  }
  ସ୍ଥିତି ଗଣ୍ଠି::NK_Null:
    // TODO: Anchor is not set for NullNode in the parser. Update the parser to
    // faithfully preserve anchors.
    ଫେରନ୍ତୁ ନୂତନ (EmptyHNodeAllocator.Allocate()) EmptyHNode(N);
  ସ୍ଥିତି ଗଣ୍ଠି::NK_Alias: {
    Alias_ଗଣ୍ଠି *AN = dyn_cast<Alias_ଗଣ୍ଠି>(N);
    auto AliasName = AN->getName();
    auto AHN = AliasMap.find(AliasName);
    ଯଦି (AHN == AliasMap.end()) {
      setError(AN, Twine("undefined alias '" + AliasName + "'"));
      ଫେରନ୍ତୁ nullptr;
    }
    ଫେରନ୍ତୁ AHN->second;
  }
  ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
    setError(N, "unknown node kind");
    ଫେରନ୍ତୁ nullptr;
  }
}

ଶୂନ୍ୟ ନିବେଶ::setError(ଅଚଳ Twine &Message) {
  setError(Current_ଗଣ୍ଠି, Message);
}

ଶୂନ୍ୟ ନିବେଶ::set_Allow_Unknown_ଚାବିଗୁଡ଼ିକ(ତାର୍କିକ Allow) { Allow_Unknown_ଚାବିଗୁଡ଼ିକ = Allow; }

ତାର୍କିକ ନିବେଶ::canElideEmptySequence() {
  ଫେରନ୍ତୁ ମିଥ୍ୟା;
}

//===----------------------------------------------------------------------===//
//  Output
//===----------------------------------------------------------------------===//

Output::Output(raw_ostream &yout, ଶୂନ୍ୟ *context, ପୂର୍ଣ୍ଣସଂଖ୍ୟା WrapColumn)
    : IO(context), Out(yout), WrapColumn(WrapColumn) {}

Output::~Output() = ପୂର୍ବନିର୍ଦ୍ଧାରିତ;

ତାର୍କିକ Output::outputting() ଅଚଳ {
  ଫେରନ୍ତୁ ସତ୍ୟ;
}

ଶୂନ୍ୟ Output::beginMapping() {
  State_ଗଦା.ଠେଲ_back(in_Map_First_ଚାବି);
  PaddingBeforeContainer = Padding;
  Padding = "\n";
}

ତାର୍କିକ Output::mapTag(StringRef Tag, ତାର୍କିକ Use) {
  ଯଦି (Use) {
    // If this tag is being written inside a sequence we should write the start
    // of the sequence before writing the tag, otherwise the tag won't be
    // attached to the element in the sequence, but rather the sequence itself.
    ତାର୍କିକ SequenceElement = ମିଥ୍ୟା;
    ଯଦି (State_ଗଦା.size() > 1) {
      auto &E = State_ଗଦା[State_ଗଦା.size() - 2];
      SequenceElement = inSeqAnyElement(E) || inFlowSeqAnyElement(E);
    }
    ଯଦି (SequenceElement && State_ଗଦା.back() == in_Map_First_ଚାବି) {
      new_ଧାଡ଼ି_Check();
    } ଅନ୍ୟଥା {
      output(" ");
    }
    output(Tag);
    ଯଦି (SequenceElement) {
      // If we're writing the tag during the first element of a map, the tag
      // takes the place of the first element in the sequence.
      ଯଦି (State_ଗଦା.back() == in_Map_First_ଚାବି) {
        State_ଗଦା.କାଢ଼_back();
        State_ଗଦା.ଠେଲ_back(in_Map_Other_ଚାବି);
      }
      // Tags inside maps in sequences should act as keys in the map from a
      // formatting perspective, so we always want a newline in a sequence.
      Padding = "\n";
    }
  }
  ଫେରନ୍ତୁ Use;
}

ଶୂନ୍ୟ Output::endMapping() {
  // If we did not map anything, we should explicitly emit an empty map
  ଯଦି (State_ଗଦା.back() == in_Map_First_ଚାବି) {
    Padding = PaddingBeforeContainer;
    new_ଧାଡ଼ି_Check();
    output("{}");
    Padding = "\n";
  }
  State_ଗଦା.କାଢ଼_back();
}

std::vector<StringRef> Output::ଚାବିଗୁଡ଼ିକ() {
  report_fatal_error("invalid call");
}

ତାର୍କିକ Output::preflight_ଚାବି(StringRef ଚାବି, ତାର୍କିକ Required, ତାର୍କିକ SameAsDefault,
                          ତାର୍କିକ &UseDefault, ଶୂନ୍ୟ *&SaveInfo) {
  UseDefault = ମିଥ୍ୟା;
  SaveInfo = nullptr;
  ଯଦି (Required || !SameAsDefault || Write_Default_ମୂଲ୍ୟଗୁଡ଼ିକ) {
    auto State = State_ଗଦା.back();
    ଯଦି (State == in_Flow_Map_First_ଚାବି || State == in_Flow_Map_Other_ଚାବି) {
      flow_ଚାବି(ଚାବି);
    } ଅନ୍ୟଥା {
      new_ଧାଡ଼ି_Check();
      padded_ଚାବି(ଚାବି);
    }
    ଫେରନ୍ତୁ ସତ୍ୟ;
  }
  ଫେରନ୍ତୁ ମିଥ୍ୟା;
}

ଶୂନ୍ୟ Output::postflight_ଚାବି(ଶୂନ୍ୟ *) {
  ଯଦି (State_ଗଦା.back() == in_Map_First_ଚାବି) {
    State_ଗଦା.କାଢ଼_back();
    State_ଗଦା.ଠେଲ_back(in_Map_Other_ଚାବି);
  } ଅନ୍ୟଥା ଯଦି (State_ଗଦା.back() == in_Flow_Map_First_ଚାବି) {
    State_ଗଦା.କାଢ଼_back();
    State_ଗଦା.ଠେଲ_back(in_Flow_Map_Other_ଚାବି);
  }
}

ଶୂନ୍ୟ Output::beginFlowMapping() {
  State_ଗଦା.ଠେଲ_back(in_Flow_Map_First_ଚାବି);
  new_ଧାଡ଼ି_Check();
  ColumnAtMapFlowStart = Column;
  output("{ ");
}

ଶୂନ୍ୟ Output::endFlowMapping() {
  State_ଗଦା.କାଢ଼_back();
  output_Up_To_End_Of_ଧାଡ଼ି(" }");
}

ଶୂନ୍ୟ Output::beginDocuments() {
  output_Up_To_End_Of_ଧାଡ଼ି("---");
}

ତାର୍କିକ Output::preflightDocument(unsigned index) {
  ଯଦି (index > 0)
    output_Up_To_End_Of_ଧାଡ଼ି("\n---");
  ଫେରନ୍ତୁ ସତ୍ୟ;
}

ଶୂନ୍ୟ Output::postflightDocument() {
}

ଶୂନ୍ୟ Output::endDocuments() {
  output("\n...\n");
}

unsigned Output::beginSequence() {
  State_ଗଦା.ଠେଲ_back(inSeqFirstElement);
  PaddingBeforeContainer = Padding;
  Padding = "\n";
  ଫେରନ୍ତୁ 0;
}

ଶୂନ୍ୟ Output::endSequence() {
  // If we did not emit anything, we should explicitly emit an empty sequence
  ଯଦି (State_ଗଦା.back() == inSeqFirstElement) {
    Padding = PaddingBeforeContainer;
    new_ଧାଡ଼ି_Check(/*EmptySequence=*/ସତ୍ୟ);
    output("[]");
    Padding = "\n";
  }
  State_ଗଦା.କାଢ଼_back();
}

ତାର୍କିକ Output::preflightElement(unsigned, ଶୂନ୍ୟ *&SaveInfo) {
  SaveInfo = nullptr;
  ଫେରନ୍ତୁ ସତ୍ୟ;
}

ଶୂନ୍ୟ Output::postflightElement(ଶୂନ୍ୟ *) {
  ଯଦି (State_ଗଦା.back() == inSeqFirstElement) {
    State_ଗଦା.କାଢ଼_back();
    State_ଗଦା.ଠେଲ_back(inSeqOtherElement);
  } ଅନ୍ୟଥା ଯଦି (State_ଗଦା.back() == inFlowSeqFirstElement) {
    State_ଗଦା.କାଢ଼_back();
    State_ଗଦା.ଠେଲ_back(inFlowSeqOtherElement);
  }
}

unsigned Output::beginFlowSequence() {
  State_ଗଦା.ଠେଲ_back(inFlowSeqFirstElement);
  new_ଧାଡ଼ି_Check();
  ColumnAtFlowStart = Column;
  output("[ ");
  NeedFlowSequenceComma = ମିଥ୍ୟା;
  ଫେରନ୍ତୁ 0;
}

ଶୂନ୍ୟ Output::endFlowSequence() {
  State_ଗଦା.କାଢ଼_back();
  output_Up_To_End_Of_ଧାଡ଼ି(" ]");
}

ତାର୍କିକ Output::preflightFlowElement(unsigned, ଶୂନ୍ୟ *&SaveInfo) {
  ଯଦି (NeedFlowSequenceComma)
    output(", ");
  ଯଦି (WrapColumn && Column > WrapColumn) {
    output("\n");
    ପାଇଁ (ପୂର୍ଣ୍ଣସଂଖ୍ୟା i = 0; i < ColumnAtFlowStart; ++i)
      output(" ");
    Column = ColumnAtFlowStart;
    output("  ");
  }
  SaveInfo = nullptr;
  ଫେରନ୍ତୁ ସତ୍ୟ;
}

ଶୂନ୍ୟ Output::postflightFlowElement(ଶୂନ୍ୟ *) {
  NeedFlowSequenceComma = ସତ୍ୟ;
}

ଶୂନ୍ୟ Output::beginEnumScalar() {
  EnumerationMatchFound = ମିଥ୍ୟା;
}

ତାର୍କିକ Output::matchEnumScalar(StringRef Str, ତାର୍କିକ Match) {
  ଯଦି (Match && !EnumerationMatchFound) {
    new_ଧାଡ଼ି_Check();
    output_Up_To_End_Of_ଧାଡ଼ି(Str);
    EnumerationMatchFound = ସତ୍ୟ;
  }
  ଫେରନ୍ତୁ ମିଥ୍ୟା;
}

ତାର୍କିକ Output::matchEnumFallback() {
  ଯଦି (EnumerationMatchFound)
    ଫେରନ୍ତୁ ମିଥ୍ୟା;
  EnumerationMatchFound = ସତ୍ୟ;
  ଫେରନ୍ତୁ ସତ୍ୟ;
}

ଶୂନ୍ୟ Output::endEnumScalar() {
  ଯଦି (!EnumerationMatchFound)
    llvm_unreachable("bad runtime enum value");
}

ତାର୍କିକ Output::beginBitSetScalar(ତାର୍କିକ &DoClear) {
  new_ଧାଡ଼ି_Check();
  output("[ ");
  Need_Bit_ମୂଲ୍ୟ_Comma = ମିଥ୍ୟା;
  DoClear = ମିଥ୍ୟା;
  ଫେରନ୍ତୁ ସତ୍ୟ;
}

ତାର୍କିକ Output::bitSetMatch(StringRef Str, ତାର୍କିକ Matches) {
  ଯଦି (Matches) {
    ଯଦି (Need_Bit_ମୂଲ୍ୟ_Comma)
      output(", ");
    output(Str);
    Need_Bit_ମୂଲ୍ୟ_Comma = ସତ୍ୟ;
  }
  ଫେରନ୍ତୁ ମିଥ୍ୟା;
}

ଶୂନ୍ୟ Output::endBitSetScalar() {
  output_Up_To_End_Of_ଧାଡ଼ି(" ]");
}

ଶୂନ୍ୟ Output::scalarString(StringRef &S, QuotingType MustQuote) {
  new_ଧାଡ଼ି_Check();
  ଯଦି (S.empty()) {
    // Print '' for the empty string because leaving the field empty is not
    // allowed.
    output_Up_To_End_Of_ଧାଡ଼ି("''");
    ଫେରନ୍ତୁ;
  }
  output(S, MustQuote);
  output_Up_To_End_Of_ଧାଡ଼ି("");
}

ଶୂନ୍ୟ Output::blockScalarString(StringRef &S) {
  ଯଦି (!State_ଗଦା.empty())
    new_ଧାଡ଼ି_Check();
  output(" |");

  unsigned Indent = State_ଗଦା.empty() ? 1 : State_ଗଦା.size();

  auto Buffer = MemoryBuffer::getMemBuffer(S, "", ମିଥ୍ୟା);
  ପାଇଁ (ଧାଡ଼ି_iterator ଧାଡ଼ିଗୁଡ଼ିକ(*Buffer, ମିଥ୍ୟା); !ଧାଡ଼ିଗୁଡ଼ିକ.is_at_end(); ++ଧାଡ଼ିଗୁଡ଼ିକ) {
    output_New_ଧାଡ଼ି();
    ପାଇଁ (unsigned I = 0; I < Indent; ++I) {
      output("  ");
    }
    output(*ଧାଡ଼ିଗୁଡ଼ିକ);
  }
  output_Up_To_End_Of_ଧାଡ଼ି("");
}

ଶୂନ୍ୟ Output::scalarTag(std::string &Tag) {
  ଯଦି (Tag.empty())
    ଫେରନ୍ତୁ;
  new_ଧାଡ଼ି_Check();
  output(Tag);
  output(" ");
}

ଶୂନ୍ୟ Output::setError(ଅଚଳ Twine &message) {
}

std::error_code Output::error() { ଫେରନ୍ତୁ {}; }

ତାର୍କିକ Output::canElideEmptySequence() {
  // Normally, with an optional key/value where the value is an empty sequence,
  // the whole key/value can be not written.  But, that produces wrong yaml
  // if the key/value is the only thing in the map and the map is used in
  // a sequence.  This detects if the this sequence is the first key/value
  // in map that itself is embedded in a sequence.
  ଯଦି (State_ଗଦା.size() < 2)
    ଫେରନ୍ତୁ ସତ୍ୟ;
  ଯଦି (State_ଗଦା.back() != in_Map_First_ଚାବି)
    ଫେରନ୍ତୁ ସତ୍ୟ;
  ଫେରନ୍ତୁ !inSeqAnyElement(State_ଗଦା[State_ଗଦା.size() - 2]);
}

ଶୂନ୍ୟ Output::output(StringRef s) {
  Column += s.size();
  Out << s;
}

ଶୂନ୍ୟ Output::output(StringRef S, QuotingType MustQuote) {
  ଯଦି (MustQuote == QuotingType::None) {
    // Only quote if we must.
    output(S);
    ଫେରନ୍ତୁ;
  }

  StringLiteral Quote = MustQuote == QuotingType::Single ? StringLiteral("'")
                                                         : StringLiteral("\"");
  output(Quote); // Starting quote.

  // When using double-quoted strings (and only in that case), non-printable
  // characters may be present, and will be escaped using a variety of
  // unicode-scalar and special short-form escapes. This is handled in
  // yaml::escape.
  ଯଦି (MustQuote == QuotingType::Double) {
    output(yaml::escape(S, /* EscapePrintable= */ ମିଥ୍ୟା));
    output(Quote);
    ଫେରନ୍ତୁ;
  }

  unsigned i = 0;
  unsigned j = 0;
  unsigned End = S.size();
  ଅଚଳ ଅକ୍ଷର *Base = S.data();

  // When using single-quoted strings, any single quote ' must be doubled to be
  // escaped.
  ଯେତେବେଳେ (j < End) {
    ଯଦି (S[j] == '\'') {                   // Escape quotes.
      output(StringRef(&Base[i], j - i)); // "flush".
      output(StringLiteral("''"));        // Print it as ''
      i = j + 1;
    }
    ++j;
  }
  output(StringRef(&Base[i], j - i));
  output(Quote); // Ending quote.
}

ଶୂନ୍ୟ Output::output_Up_To_End_Of_ଧାଡ଼ି(StringRef s) {
  output(s);
  ଯଦି (State_ଗଦା.empty() || (!inFlowSeqAnyElement(State_ଗଦା.back()) &&
                             !in_Flow_Map_Any_ଚାବି(State_ଗଦା.back())))
    Padding = "\n";
}

ଶୂନ୍ୟ Output::output_New_ଧାଡ଼ି() {
  Out << "\n";
  Column = 0;
}

// if seq at top, indent as if map, then add "- "
// if seq in middle, use "- " if firstKey, else use "  "
//

ଶୂନ୍ୟ Output::new_ଧାଡ଼ି_Check(ତାର୍କିକ EmptySequence) {
  ଯଦି (Padding != "\n") {
    output(Padding);
    Padding = {};
    ଫେରନ୍ତୁ;
  }
  output_New_ଧାଡ଼ି();
  Padding = {};

  ଯଦି (State_ଗଦା.size() == 0 || EmptySequence)
    ଫେରନ୍ତୁ;

  unsigned Indent = State_ଗଦା.size() - 1;
  ତାର୍କିକ PossiblyNestedSeq = ମିଥ୍ୟା;
  auto I = State_ଗଦା.rbegin(), E = State_ଗଦା.rend();

  ଯଦି (inSeqAnyElement(*I)) {
    PossiblyNestedSeq = ସତ୍ୟ; // Not possibly but always.
    ++Indent;
  } ଅନ୍ୟଥା ଯଦି (*I == in_Map_First_ଚାବି || *I == in_Flow_Map_First_ଚାବି ||
             inFlowSeqAnyElement(*I)) {
    PossiblyNestedSeq = ସତ୍ୟ;
    ++I; // Skip back().
  }

  unsigned Output_Dash_ଗଣନା = 0;
  ଯଦି (PossiblyNestedSeq) {
    // Count up consecutive inSeqFirstElement from the end, unless
    // inSeqFirstElement is the top of nested sequence.
    ଯେତେବେଳେ (I != E) {
      // Don't count the top of nested sequence.
      ଯଦି (!inSeqAnyElement(*I))
        ଭାଙ୍ଗ;

      ++Output_Dash_ଗଣନା;

      // Stop counting if consecutive inSeqFirstElement ends.
      ଯଦି (*I++ != inSeqFirstElement)
        ଭାଙ୍ଗ;
    }
  }

  ପାଇଁ (unsigned I = Output_Dash_ଗଣନା; I < Indent; ++I)
    output("  ");

  ପାଇଁ (unsigned I = 0; I < Output_Dash_ଗଣନା; ++I)
    output("- ");
}

ଶୂନ୍ୟ Output::padded_ଚାବି(StringRef ଚାବି_2) {
  output(ଚାବି_2, needsQuotes(ଚାବି_2, ମିଥ୍ୟା));
  output(":");
  ଅଚଳ ଅକ୍ଷର *spaces = "                ";
  ଯଦି (ଚାବି_2.size() < strlen(spaces))
    Padding = &spaces[ଚାବି_2.size()];
  ଅନ୍ୟଥା
    Padding = " ";
}

ଶୂନ୍ୟ Output::flow_ଚାବି(StringRef ଚାବି) {
  ଯଦି (State_ଗଦା.back() == in_Flow_Map_Other_ଚାବି)
    output(", ");
  ଯଦି (WrapColumn && Column > WrapColumn) {
    output("\n");
    ପାଇଁ (ପୂର୍ଣ୍ଣସଂଖ୍ୟା I = 0; I < ColumnAtMapFlowStart; ++I)
      output(" ");
    Column = ColumnAtMapFlowStart;
    output("  ");
  }
  output(ଚାବି, needsQuotes(ଚାବି, ମିଥ୍ୟା));
  output(": ");
}

ଗଣ୍ଠି_Kind Output::get_ଗଣ୍ଠି_Kind() { report_fatal_error("invalid call"); }

ତାର୍କିକ Output::inSeqAnyElement(InState State) {
  ଫେରନ୍ତୁ State == inSeqFirstElement || State == inSeqOtherElement;
}

ତାର୍କିକ Output::inFlowSeqAnyElement(InState State) {
  ଫେରନ୍ତୁ State == inFlowSeqFirstElement || State == inFlowSeqOtherElement;
}

ତାର୍କିକ Output::in_Map_Any_ଚାବି(InState State) {
  ଫେରନ୍ତୁ State == in_Map_First_ଚାବି || State == in_Map_Other_ଚାବି;
}

ତାର୍କିକ Output::in_Flow_Map_Any_ଚାବି(InState State) {
  ଫେରନ୍ତୁ State == in_Flow_Map_First_ଚାବି || State == in_Flow_Map_Other_ଚାବି;
}

//===----------------------------------------------------------------------===//
//  traits for built-in types
//===----------------------------------------------------------------------===//

ଶୂନ୍ୟ ScalarTraits<ତାର୍କିକ>::output(ଅଚଳ ତାର୍କିକ &Val, ଶୂନ୍ୟ *, raw_ostream &Out) {
  Out << (Val ? "true" : "false");
}

StringRef ScalarTraits<ତାର୍କିକ>::ନିବେଶ_2(StringRef Scalar, ଶୂନ୍ୟ *, ତାର୍କିକ &Val) {
  ଯଦି (std::optional<ତାର୍କିକ> Parsed = parseBool(Scalar)) {
    Val = *Parsed;
    ଫେରନ୍ତୁ StringRef();
  }
  ଫେରନ୍ତୁ "invalid boolean";
}

ଶୂନ୍ୟ ScalarTraits<StringRef>::output(ଅଚଳ StringRef &Val, ଶୂନ୍ୟ *,
                                     raw_ostream &Out) {
  Out << Val;
}

StringRef ScalarTraits<StringRef>::ନିବେଶ_2(StringRef Scalar, ଶୂନ୍ୟ *,
                                         StringRef &Val) {
  Val = Scalar;
  ଫେରନ୍ତୁ StringRef();
}

ଶୂନ୍ୟ ScalarTraits<std::string>::output(ଅଚଳ std::string &Val, ଶୂନ୍ୟ *,
                                       raw_ostream &Out) {
  Out << Val;
}

StringRef ScalarTraits<std::string>::ନିବେଶ_2(StringRef Scalar, ଶୂନ୍ୟ *,
                                           std::string &Val) {
  Val = Scalar.str();
  ଫେରନ୍ତୁ StringRef();
}

ଶୂନ୍ୟ ScalarTraits<uint8_t>::output(ଅଚଳ uint8_t &Val, ଶୂନ୍ୟ *,
                                   raw_ostream &Out) {
  // use temp uin32_t because ostream thinks uint8_t is a character
  uint32_t ଅଙ୍କ = Val;
  Out << ଅଙ୍କ;
}

StringRef ScalarTraits<uint8_t>::ନିବେଶ_2(StringRef Scalar, ଶୂନ୍ୟ *, uint8_t &Val) {
  unsigned long long n;
  ଯଦି (getAsUnsignedInteger(Scalar, 0, n))
    ଫେରନ୍ତୁ "invalid number";
  ଯଦି (n > 0xFF)
    ଫେରନ୍ତୁ "out of range number";
  Val = n;
  ଫେରନ୍ତୁ StringRef();
}

ଶୂନ୍ୟ ScalarTraits<uint16_t>::output(ଅଚଳ uint16_t &Val, ଶୂନ୍ୟ *,
                                    raw_ostream &Out) {
  Out << Val;
}

StringRef ScalarTraits<uint16_t>::ନିବେଶ_2(StringRef Scalar, ଶୂନ୍ୟ *,
                                        uint16_t &Val) {
  unsigned long long n;
  ଯଦି (getAsUnsignedInteger(Scalar, 0, n))
    ଫେରନ୍ତୁ "invalid number";
  ଯଦି (n > 0xFFFF)
    ଫେରନ୍ତୁ "out of range number";
  Val = n;
  ଫେରନ୍ତୁ StringRef();
}

ଶୂନ୍ୟ ScalarTraits<uint32_t>::output(ଅଚଳ uint32_t &Val, ଶୂନ୍ୟ *,
                                    raw_ostream &Out) {
  Out << Val;
}

StringRef ScalarTraits<uint32_t>::ନିବେଶ_2(StringRef Scalar, ଶୂନ୍ୟ *,
                                        uint32_t &Val) {
  unsigned long long n;
  ଯଦି (getAsUnsignedInteger(Scalar, 0, n))
    ଫେରନ୍ତୁ "invalid number";
  ଯଦି (n > 0xFFFFFFFFUL)
    ଫେରନ୍ତୁ "out of range number";
  Val = n;
  ଫେରନ୍ତୁ StringRef();
}

ଶୂନ୍ୟ ScalarTraits<uint64_t>::output(ଅଚଳ uint64_t &Val, ଶୂନ୍ୟ *,
                                    raw_ostream &Out) {
  Out << Val;
}

StringRef ScalarTraits<uint64_t>::ନିବେଶ_2(StringRef Scalar, ଶୂନ୍ୟ *,
                                        uint64_t &Val) {
  unsigned long long N;
  ଯଦି (getAsUnsignedInteger(Scalar, 0, N))
    ଫେରନ୍ତୁ "invalid number";
  Val = N;
  ଫେରନ୍ତୁ StringRef();
}

ଶୂନ୍ୟ ScalarTraits<int8_t>::output(ଅଚଳ int8_t &Val, ଶୂନ୍ୟ *, raw_ostream &Out) {
  // use temp in32_t because ostream thinks int8_t is a character
  int32_t ଅଙ୍କ = Val;
  Out << ଅଙ୍କ;
}

StringRef ScalarTraits<int8_t>::ନିବେଶ_2(StringRef Scalar, ଶୂନ୍ୟ *, int8_t &Val) {
  long long N;
  ଯଦି (getAsSignedInteger(Scalar, 0, N))
    ଫେରନ୍ତୁ "invalid number";
  ଯଦି ((N > 127) || (N < -128))
    ଫେରନ୍ତୁ "out of range number";
  Val = N;
  ଫେରନ୍ତୁ StringRef();
}

ଶୂନ୍ୟ ScalarTraits<int16_t>::output(ଅଚଳ int16_t &Val, ଶୂନ୍ୟ *,
                                   raw_ostream &Out) {
  Out << Val;
}

StringRef ScalarTraits<int16_t>::ନିବେଶ_2(StringRef Scalar, ଶୂନ୍ୟ *, int16_t &Val) {
  long long N;
  ଯଦି (getAsSignedInteger(Scalar, 0, N))
    ଫେରନ୍ତୁ "invalid number";
  ଯଦି ((N > INT16_MAX) || (N < INT16_MIN))
    ଫେରନ୍ତୁ "out of range number";
  Val = N;
  ଫେରନ୍ତୁ StringRef();
}

ଶୂନ୍ୟ ScalarTraits<int32_t>::output(ଅଚଳ int32_t &Val, ଶୂନ୍ୟ *,
                                   raw_ostream &Out) {
  Out << Val;
}

StringRef ScalarTraits<int32_t>::ନିବେଶ_2(StringRef Scalar, ଶୂନ୍ୟ *, int32_t &Val) {
  long long N;
  ଯଦି (getAsSignedInteger(Scalar, 0, N))
    ଫେରନ୍ତୁ "invalid number";
  ଯଦି ((N > INT32_MAX) || (N < INT32_MIN))
    ଫେରନ୍ତୁ "out of range number";
  Val = N;
  ଫେରନ୍ତୁ StringRef();
}

ଶୂନ୍ୟ ScalarTraits<int64_t>::output(ଅଚଳ int64_t &Val, ଶୂନ୍ୟ *,
                                   raw_ostream &Out) {
  Out << Val;
}

StringRef ScalarTraits<int64_t>::ନିବେଶ_2(StringRef Scalar, ଶୂନ୍ୟ *, int64_t &Val) {
  long long N;
  ଯଦି (getAsSignedInteger(Scalar, 0, N))
    ଫେରନ୍ତୁ "invalid number";
  Val = N;
  ଫେରନ୍ତୁ StringRef();
}

ଶୂନ୍ୟ ScalarTraits<ଦ୍ୱିଗୁଣ>::output(ଅଚଳ ଦ୍ୱିଗୁଣ &Val, ଶୂନ୍ୟ *, raw_ostream &Out) {
  Out << format("%g", Val);
}

StringRef ScalarTraits<ଦ୍ୱିଗୁଣ>::ନିବେଶ_2(StringRef Scalar, ଶୂନ୍ୟ *, ଦ୍ୱିଗୁଣ &Val) {
  ଯଦି (to_float(Scalar, Val))
    ଫେରନ୍ତୁ StringRef();
  ଫେରନ୍ତୁ "invalid floating point number";
}

ଶୂନ୍ୟ ScalarTraits<ଦଶମିକ>::output(ଅଚଳ ଦଶମିକ &Val, ଶୂନ୍ୟ *, raw_ostream &Out) {
  Out << format("%g", Val);
}

StringRef ScalarTraits<ଦଶମିକ>::ନିବେଶ_2(StringRef Scalar, ଶୂନ୍ୟ *, ଦଶମିକ &Val) {
  ଯଦି (to_float(Scalar, Val))
    ଫେରନ୍ତୁ StringRef();
  ଫେରନ୍ତୁ "invalid floating point number";
}

ଶୂନ୍ୟ ScalarTraits<Hex8>::output(ଅଚଳ Hex8 &Val, ଶୂନ୍ୟ *, raw_ostream &Out) {
  Out << format("0x%" PRIX8, (uint8_t)Val);
}

StringRef ScalarTraits<Hex8>::ନିବେଶ_2(StringRef Scalar, ଶୂନ୍ୟ *, Hex8 &Val) {
  unsigned long long n;
  ଯଦି (getAsUnsignedInteger(Scalar, 0, n))
    ଫେରନ୍ତୁ "invalid hex8 number";
  ଯଦି (n > 0xFF)
    ଫେରନ୍ତୁ "out of range hex8 number";
  Val = n;
  ଫେରନ୍ତୁ StringRef();
}

ଶୂନ୍ୟ ScalarTraits<Hex16>::output(ଅଚଳ Hex16 &Val, ଶୂନ୍ୟ *, raw_ostream &Out) {
  Out << format("0x%" PRIX16, (uint16_t)Val);
}

StringRef ScalarTraits<Hex16>::ନିବେଶ_2(StringRef Scalar, ଶୂନ୍ୟ *, Hex16 &Val) {
  unsigned long long n;
  ଯଦି (getAsUnsignedInteger(Scalar, 0, n))
    ଫେରନ୍ତୁ "invalid hex16 number";
  ଯଦି (n > 0xFFFF)
    ଫେରନ୍ତୁ "out of range hex16 number";
  Val = n;
  ଫେରନ୍ତୁ StringRef();
}

ଶୂନ୍ୟ ScalarTraits<Hex32>::output(ଅଚଳ Hex32 &Val, ଶୂନ୍ୟ *, raw_ostream &Out) {
  Out << format("0x%" PRIX32, (uint32_t)Val);
}

StringRef ScalarTraits<Hex32>::ନିବେଶ_2(StringRef Scalar, ଶୂନ୍ୟ *, Hex32 &Val) {
  unsigned long long n;
  ଯଦି (getAsUnsignedInteger(Scalar, 0, n))
    ଫେରନ୍ତୁ "invalid hex32 number";
  ଯଦି (n > 0xFFFFFFFFUL)
    ଫେରନ୍ତୁ "out of range hex32 number";
  Val = n;
  ଫେରନ୍ତୁ StringRef();
}

ଶୂନ୍ୟ ScalarTraits<Hex64>::output(ଅଚଳ Hex64 &Val, ଶୂନ୍ୟ *, raw_ostream &Out) {
  Out << format("0x%" PRIX64, (uint64_t)Val);
}

StringRef ScalarTraits<Hex64>::ନିବେଶ_2(StringRef Scalar, ଶୂନ୍ୟ *, Hex64 &Val) {
  unsigned long long ଅଙ୍କ;
  ଯଦି (getAsUnsignedInteger(Scalar, 0, ଅଙ୍କ))
    ଫେରନ୍ତୁ "invalid hex64 number";
  Val = ଅଙ୍କ;
  ଫେରନ୍ତୁ StringRef();
}

ଶୂନ୍ୟ ScalarTraits<VersionTuple>::output(ଅଚଳ VersionTuple &Val, ଶୂନ୍ୟ *,
                                        llvm::raw_ostream &Out) {
  Out << Val.getAsString();
}

StringRef ScalarTraits<VersionTuple>::ନିବେଶ_2(StringRef Scalar, ଶୂନ୍ୟ *,
                                            VersionTuple &Val) {
  ଯଦି (Val.tryParse(Scalar))
    ଫେରନ୍ତୁ "invalid version format";
  ଫେରନ୍ତୁ StringRef();
}
