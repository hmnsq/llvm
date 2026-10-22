//===- lib/Support/YAMLTraits.cpp -----------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#अन्तर्भाव "llvm/Support/YAMLTraits.h"
#अन्तर्भाव "llvm/ADT/STLExtras.h"
#अन्तर्भाव "llvm/ADT/SmallString.h"
#अन्तर्भाव "llvm/ADT/StringExtras.h"
#अन्तर्भाव "llvm/ADT/StringRef.h"
#अन्तर्भाव "llvm/ADT/Twine.h"
#अन्तर्भाव "llvm/Support/Casting.h"
#अन्तर्भाव "llvm/Support/Errc.h"
#अन्तर्भाव "llvm/Support/ErrorHandling.h"
#अन्तर्भाव "llvm/Support/Format.h"
#अन्तर्भाव "llvm/Support/LineIterator.h"
#अन्तर्भाव "llvm/Support/MemoryBuffer.h"
#अन्तर्भाव "llvm/Support/VersionTuple.h"
#अन्तर्भाव "llvm/Support/YAMLParser.h"
#अन्तर्भाव "llvm/Support/raw_ostream.h"
#अन्तर्भाव <cassert>
#अन्तर्भाव <cstdint>
#अन्तर्भाव <cstring>
#अन्तर्भाव <string>
#अन्तर्भाव <vector>

using नामाकाश llvm;
using नामाकाश yaml;

//===----------------------------------------------------------------------===//
//  IO
//===----------------------------------------------------------------------===//

IO::IO(शून्य *Context) : Ctxt(Context) {}

IO::~IO() = स्वभाव;

शून्य *IO::getContext() ध्रुव {
  पटिनिवत्त Ctxt;
}

शून्य IO::setContext(शून्य *Context) {
  Ctxt = Context;
}

शून्य IO::set_Allow_Unknown_कुञ्चिकाः(तार्किक Allow) {
  llvm_unreachable("Only supported for Input");
}

//===----------------------------------------------------------------------===//
//निवेशः
//===----------------------------------------------------------------------===//

निवेशः::निवेशः(StringRef निवेशः_Content, शून्य *Ctxt,
             SourceMgr::DiagHandlerTy DiagHandler, शून्य *DiagHandlerCtxt)
    : IO(Ctxt), Strm(नूतन Stream(निवेशः_Content, SrcMgr, मिच्छा, &EC)) {
  सचे (DiagHandler)
    SrcMgr.setDiagHandler(DiagHandler, DiagHandlerCtxt);
  DocIterator = Strm->begin();
}

निवेशः::निवेशः(MemoryBufferRef निवेशः, शून्य *Ctxt,
             SourceMgr::DiagHandlerTy DiagHandler, शून्य *DiagHandlerCtxt)
    : IO(Ctxt), Strm(नूतन Stream(निवेशः, SrcMgr, मिच्छा, &EC)) {
  सचे (DiagHandler)
    SrcMgr.setDiagHandler(DiagHandler, DiagHandlerCtxt);
  DocIterator = Strm->begin();
}

निवेशः::~निवेशः() = स्वभाव;

std::error_code निवेशः::error() { पटिनिवत्त EC; }

तार्किक निवेशः::outputting() ध्रुव {
  पटिनिवत्त मिच्छा;
}

तार्किक निवेशः::setCurrentDocument() {
  सचे (DocIterator != Strm->end()) {
    ग्रन्थिः *N = DocIterator->get_मूलम्();
    सचे (!N) {
      EC = make_error_code(errc::invalid_argument);
      पटिनिवत्त मिच्छा;
    }

    सचे (isa<Null_ग्रन्थिः>(N)) {
      // Empty files are allowed and ignored
      ++DocIterator;
      पटिनिवत्त setCurrentDocument();
    }
    releaseHNodeBuffers();
    शिखरम्_ग्रन्थिः = createHNodes(N);
    Current_ग्रन्थिः = शिखरम्_ग्रन्थिः;
    पटिनिवत्त सच्च;
  }
  पटिनिवत्त मिच्छा;
}

तार्किक निवेशः::nextDocument() {
  पटिनिवत्त ++DocIterator != Strm->end();
}

ध्रुव ग्रन्थिः *निवेशः::get_Current_ग्रन्थिः() ध्रुव {
  पटिनिवत्त Current_ग्रन्थिः ? Current_ग्रन्थिः->ग्रन्थिः_2 : nullptr;
}

तार्किक निवेशः::mapTag(StringRef Tag, तार्किक Default) {
  // CurrentNode can be null if setCurrentDocument() was unable to
  // parse the document because it was invalid or empty.
  सचे (!Current_ग्रन्थिः)
    पटिनिवत्त मिच्छा;

  std::string foundTag = Current_ग्रन्थिः->ग्रन्थिः_2->getVerbatimTag();
  सचे (foundTag.empty()) {
    // If no tag found and 'Tag' is the default, say it was found.
    पटिनिवत्त Default;
  }
  // Return true iff found tag matches supplied tag.
  पटिनिवत्त Tag == foundTag;
}

शून्य निवेशः::beginMapping() {
  सचे (EC)
    पटिनिवत्त;
  // CurrentNode can be null if the document is empty.
  MapHNode *MN = dyn_cast_or_null<MapHNode>(Current_ग्रन्थिः);
  सचे (MN) {
    MN->Valid_कुञ्चिकाः.clear();
  }
}

std::vector<StringRef> निवेशः::कुञ्चिकाः() {
  MapHNode *MN = dyn_cast<MapHNode>(Current_ग्रन्थिः);
  std::vector<StringRef> Ret;
  सचे (!MN) {
    setError(Current_ग्रन्थिः, "not a mapping");
    पटिनिवत्त Ret;
  }
  अत्थाय (auto &P : MN->Mapping)
    Ret.प्रक्षिप_back(P.first());
  पटिनिवत्त Ret;
}

तार्किक निवेशः::preflight_कुञ्चिका(StringRef कुञ्चिका, तार्किक Required, तार्किक, तार्किक &UseDefault,
                         शून्य *&SaveInfo) {
  UseDefault = मिच्छा;
  सचे (EC)
    पटिनिवत्त मिच्छा;

  // CurrentNode is null for empty documents, which is an error in case required
  // nodes are present.
  सचे (!Current_ग्रन्थिः) {
    सचे (Required)
      EC = make_error_code(errc::invalid_argument);
    अञ्ञथा
      UseDefault = सच्च;
    पटिनिवत्त मिच्छा;
  }

  MapHNode *MN = dyn_cast<MapHNode>(Current_ग्रन्थिः);
  सचे (!MN) {
    सचे (Required || !isa<EmptyHNode>(Current_ग्रन्थिः))
      setError(Current_ग्रन्थिः, "not a mapping");
    अञ्ञथा
      UseDefault = सच्च;
    पटिनिवत्त मिच्छा;
  }
  MN->Valid_कुञ्चिकाः.प्रक्षिप_back(कुञ्चिका.str());
  HNode *मानम् = MN->Mapping[कुञ्चिका].first;
  सचे (!मानम्) {
    सचे (Required)
      setError(Current_ग्रन्थिः, Twine("missing required key '") + कुञ्चिका + "'");
    अञ्ञथा
      UseDefault = सच्च;
    पटिनिवत्त मिच्छा;
  }
  SaveInfo = Current_ग्रन्थिः;
  Current_ग्रन्थिः = मानम्;
  पटिनिवत्त सच्च;
}

शून्य निवेशः::postflight_कुञ्चिका(शून्य *saveInfo) {
  Current_ग्रन्थिः = reinterpret_cast<HNode *>(saveInfo);
}

शून्य निवेशः::endMapping() {
  सचे (EC)
    पटिनिवत्त;
  // CurrentNode can be null if the document is empty.
  MapHNode *MN = dyn_cast_or_null<MapHNode>(Current_ग्रन्थिः);
  सचे (!MN)
    पटिनिवत्त;
  अत्थाय (ध्रुव auto &NN : MN->Mapping) {
    सचे (!is_contained(MN->Valid_कुञ्चिकाः, NN.first())) {
      ध्रुव SMRange &ReportLoc = NN.second.second;
      सचे (!Allow_Unknown_कुञ्चिकाः) {
        setError(ReportLoc, Twine("unknown key '") + NN.first() + "'");
        भङ्ग;
      } अञ्ञथा
        reportWarning(ReportLoc, Twine("unknown key '") + NN.first() + "'");
    }
  }
}

शून्य निवेशः::beginFlowMapping() { beginMapping(); }

शून्य निवेशः::endFlowMapping() { endMapping(); }

unsigned निवेशः::beginSequence() {
  सचे (SequenceHNode *SQ = dyn_cast<SequenceHNode>(Current_ग्रन्थिः))
    पटिनिवत्त SQ->Entries.size();
  सचे (isa<EmptyHNode>(Current_ग्रन्थिः))
    पटिनिवत्त 0;
  // Treat case where there's a scalar "null" value as an empty sequence.
  सचे (ScalarHNode *SN = dyn_cast<ScalarHNode>(Current_ग्रन्थिः)) {
    सचे (isNull(SN->मानम्_2()))
      पटिनिवत्त 0;
  }
  // Any other type of HNode is an error.
  setError(Current_ग्रन्थिः, "not a sequence");
  पटिनिवत्त 0;
}

शून्य निवेशः::endSequence() {
}

तार्किक निवेशः::preflightElement(unsigned Index, शून्य *&SaveInfo) {
  सचे (EC)
    पटिनिवत्त मिच्छा;
  सचे (SequenceHNode *SQ = dyn_cast<SequenceHNode>(Current_ग्रन्थिः)) {
    SaveInfo = Current_ग्रन्थिः;
    Current_ग्रन्थिः = SQ->Entries[Index];
    पटिनिवत्त सच्च;
  }
  पटिनिवत्त मिच्छा;
}

शून्य निवेशः::postflightElement(शून्य *SaveInfo) {
  Current_ग्रन्थिः = reinterpret_cast<HNode *>(SaveInfo);
}

unsigned निवेशः::beginFlowSequence() { पटिनिवत्त beginSequence(); }

तार्किक निवेशः::preflightFlowElement(unsigned index, शून्य *&SaveInfo) {
  सचे (EC)
    पटिनिवत्त मिच्छा;
  सचे (SequenceHNode *SQ = dyn_cast<SequenceHNode>(Current_ग्रन्थिः)) {
    SaveInfo = Current_ग्रन्थिः;
    Current_ग्रन्थिः = SQ->Entries[index];
    पटिनिवत्त सच्च;
  }
  पटिनिवत्त मिच्छा;
}

शून्य निवेशः::postflightFlowElement(शून्य *SaveInfo) {
  Current_ग्रन्थिः = reinterpret_cast<HNode *>(SaveInfo);
}

शून्य निवेशः::endFlowSequence() {
}

शून्य निवेशः::beginEnumScalar() {
  ScalarMatchFound = मिच्छा;
}

तार्किक निवेशः::matchEnumScalar(StringRef Str, तार्किक) {
  सचे (ScalarMatchFound)
    पटिनिवत्त मिच्छा;
  सचे (ScalarHNode *SN = dyn_cast<ScalarHNode>(Current_ग्रन्थिः)) {
    सचे (SN->मानम्_2() == Str) {
      ScalarMatchFound = सच्च;
      पटिनिवत्त सच्च;
    }
  }
  पटिनिवत्त मिच्छा;
}

तार्किक निवेशः::matchEnumFallback() {
  सचे (ScalarMatchFound)
    पटिनिवत्त मिच्छा;
  ScalarMatchFound = सच्च;
  पटिनिवत्त सच्च;
}

शून्य निवेशः::endEnumScalar() {
  सचे (!ScalarMatchFound) {
    setError(Current_ग्रन्थिः, "unknown enumerated scalar");
  }
}

तार्किक निवेशः::beginBitSetScalar(तार्किक &DoClear) {
  Bit_मानानि_Used.clear();
  सचे (SequenceHNode *SQ = dyn_cast<SequenceHNode>(Current_ग्रन्थिः)) {
    Bit_मानानि_Used.resize(SQ->Entries.size());
  } अञ्ञथा {
    setError(Current_ग्रन्थिः, "expected sequence of bit values");
  }
  DoClear = सच्च;
  पटिनिवत्त सच्च;
}

तार्किक निवेशः::bitSetMatch(StringRef Str, तार्किक) {
  सचे (EC)
    पटिनिवत्त मिच्छा;
  सचे (SequenceHNode *SQ = dyn_cast<SequenceHNode>(Current_ग्रन्थिः)) {
    unsigned Index = 0;
    अत्थाय (auto &N : SQ->Entries) {
      सचे (ScalarHNode *SN = dyn_cast<ScalarHNode>(N)) {
        सचे (SN->मानम्_2() == Str) {
          Bit_मानानि_Used[Index] = सच्च;
          पटिनिवत्त सच्च;
        }
      } अञ्ञथा {
        setError(Current_ग्रन्थिः, "unexpected scalar in sequence of bit values");
      }
      ++Index;
    }
  } अञ्ञथा {
    setError(Current_ग्रन्थिः, "expected sequence of bit values");
  }
  पटिनिवत्त मिच्छा;
}

शून्य निवेशः::endBitSetScalar() {
  सचे (EC)
    पटिनिवत्त;
  सचे (SequenceHNode *SQ = dyn_cast<SequenceHNode>(Current_ग्रन्थिः)) {
    प्रतिज्ञा(Bit_मानानि_Used.size() == SQ->Entries.size());
    अत्थाय (unsigned i = 0; i < SQ->Entries.size(); ++i) {
      सचे (!Bit_मानानि_Used[i]) {
        setError(SQ->Entries[i], "unknown bit value");
        पटिनिवत्त;
      }
    }
  }
}

शून्य निवेशः::scalarString(StringRef &S, QuotingType) {
  सचे (ScalarHNode *SN = dyn_cast<ScalarHNode>(Current_ग्रन्थिः)) {
    S = SN->मानम्_2();
  } अञ्ञथा {
    setError(Current_ग्रन्थिः, "unexpected scalar");
  }
}

शून्य निवेशः::blockScalarString(StringRef &S) { scalarString(S, QuotingType::None); }

शून्य निवेशः::scalarTag(std::string &Tag) {
  Tag = Current_ग्रन्थिः->ग्रन्थिः_2->getVerbatimTag();
}

शून्य निवेशः::setError(HNode *hnode, ध्रुव Twine &message) {
  प्रतिज्ञा(hnode && "HNode must not be NULL");
  setError(hnode->ग्रन्थिः_2, message);
}

ग्रन्थिः_Kind निवेशः::get_ग्रन्थिः_Kind() {
  सचे (isa<ScalarHNode>(Current_ग्रन्थिः))
    पटिनिवत्त ग्रन्थिः_Kind::Scalar;
  अञ्ञथा सचे (isa<MapHNode>(Current_ग्रन्थिः))
    पटिनिवत्त ग्रन्थिः_Kind::Map;
  अञ्ञथा सचे (isa<SequenceHNode>(Current_ग्रन्थिः))
    पटिनिवत्त ग्रन्थिः_Kind::Sequence;
  llvm_unreachable("Unsupported node kind");
}

शून्य निवेशः::setError(ग्रन्थिः *ग्रन्थिः_3, ध्रुव Twine &message) {
  Strm->printError(ग्रन्थिः_3, message);
  EC = make_error_code(errc::invalid_argument);
}

शून्य निवेशः::setError(ध्रुव SMRange &range, ध्रुव Twine &message) {
  Strm->printError(range, message);
  EC = make_error_code(errc::invalid_argument);
}

शून्य निवेशः::reportWarning(HNode *hnode, ध्रुव Twine &message) {
  प्रतिज्ञा(hnode && "HNode must not be NULL");
  Strm->printError(hnode->ग्रन्थिः_2, message, SourceMgr::DK_Warning);
}

शून्य निवेशः::reportWarning(ग्रन्थिः *ग्रन्थिः_3, ध्रुव Twine &message) {
  Strm->printError(ग्रन्थिः_3, message, SourceMgr::DK_Warning);
}

शून्य निवेशः::reportWarning(ध्रुव SMRange &range, ध्रुव Twine &message) {
  Strm->printError(range, message, SourceMgr::DK_Warning);
}

शून्य निवेशः::releaseHNodeBuffers() {
  EmptyHNodeAllocator.DestroyAll();
  ScalarHNodeAllocator.DestroyAll();
  SequenceHNodeAllocator.DestroyAll();
  MapHNodeAllocator.DestroyAll();
}

शून्य निवेशः::saveAliasHNode(ग्रन्थिः *N, HNode *HN) {
  StringRef Anchor = N->getAnchor();
  सचे (!Anchor.empty())
    // YAML 1.2.2 - 3.2.2.2. Anchors and Aliases:
    //
    // An alias event refers to the most recent event in the serialization
    // having the specified anchor. Therefore, anchors need not be unique within
    // a serialization. In addition, an anchor need not have an alias node
    // referring to it.
    AliasMap[Anchor] = HN;
}

निवेशः::HNode *निवेशः::createHNodes(ग्रन्थिः *N) {
  SmallString<128> StringStorage;
  विकल्प (N->getType()) {
  पक्ष ग्रन्थिः::NK_Scalar: {
    Scalar_ग्रन्थिः *SN = dyn_cast<Scalar_ग्रन्थिः>(N);
    StringRef कुञ्चिका_Str = SN->get_मानम्(StringStorage);
    सचे (!StringStorage.empty()) {
      // Copy string to permanent storage
      कुञ्चिका_Str = StringStorage.str().copy(StringAllocator);
    }
    auto *SHNode = नूतन (ScalarHNodeAllocator.Allocate()) ScalarHNode(N, कुञ्चिका_Str);
    saveAliasHNode(SN, SHNode);
    पटिनिवत्त SHNode;
  }
  पक्ष ग्रन्थिः::NK_BlockScalar: {
    Block_Scalar_ग्रन्थिः *BSN = dyn_cast<Block_Scalar_ग्रन्थिः>(N);
    StringRef मानम्_Copy = BSN->get_मानम्().copy(StringAllocator);
    auto *BSHNode =
        नूतन (ScalarHNodeAllocator.Allocate()) ScalarHNode(N, मानम्_Copy);
    saveAliasHNode(BSN, BSHNode);
    पटिनिवत्त BSHNode;
  }
  पक्ष ग्रन्थिः::NK_Sequence: {
    Sequence_ग्रन्थिः *SQ = dyn_cast<Sequence_ग्रन्थिः>(N);
    auto SQHNode = नूतन (SequenceHNodeAllocator.Allocate()) SequenceHNode(N);
    अत्थाय (ग्रन्थिः &SN : *SQ) {
      auto Entry = createHNodes(&SN);
      सचे (EC)
        भङ्ग;
      SQHNode->Entries.प्रक्षिप_back(Entry);
    }
    saveAliasHNode(SQ, SQHNode);
    पटिनिवत्त SQHNode;
  }
  पक्ष ग्रन्थिः::NK_Mapping: {
    Mapping_ग्रन्थिः *Map = dyn_cast<Mapping_ग्रन्थिः>(N);
    auto mapHNode = नूतन (MapHNodeAllocator.Allocate()) MapHNode(N);
    अत्थाय (कुञ्चिका_मानम्_ग्रन्थिः &KVN : *Map) {
      ग्रन्थिः *कुञ्चिका_ग्रन्थिः = KVN.get_कुञ्चिका();
      Scalar_ग्रन्थिः *कुञ्चिका = dyn_cast_if_present<Scalar_ग्रन्थिः>(कुञ्चिका_ग्रन्थिः);
      ग्रन्थिः *मानम् = KVN.get_मानम्();
      सचे (!कुञ्चिका || !मानम्) {
        सचे (!कुञ्चिका)
          setError(कुञ्चिका_ग्रन्थिः, "Map key must be a scalar");
        सचे (!मानम्)
          setError(कुञ्चिका_ग्रन्थिः, "Map value must not be empty");
        भङ्ग;
      }
      StringStorage.clear();
      StringRef कुञ्चिका_Str = कुञ्चिका->get_मानम्(StringStorage);
      सचे (!StringStorage.empty()) {
        // Copy string to permanent storage
        कुञ्चिका_Str = StringStorage.str().copy(StringAllocator);
      }
      सचे (mapHNode->Mapping.गणनम्(कुञ्चिका_Str))
        // From YAML spec: "The content of a mapping node is an unordered set of
        // key/value node pairs, with the restriction that each of the keys is
        // unique."
        setError(कुञ्चिका_ग्रन्थिः, Twine("duplicated mapping key '") + कुञ्चिका_Str + "'");
      auto मानम्_HNode = createHNodes(मानम्);
      सचे (EC)
        भङ्ग;
      mapHNode->Mapping[कुञ्चिका_Str] =
          std::make_pair(std::move(मानम्_HNode), कुञ्चिका_ग्रन्थिः->getSourceRange());
    }
    saveAliasHNode(Map, mapHNode);
    पटिनिवत्त std::move(mapHNode);
  }
  पक्ष ग्रन्थिः::NK_Null:
    // TODO: Anchor is not set for NullNode in the parser. Update the parser to
    // faithfully preserve anchors.
    पटिनिवत्त नूतन (EmptyHNodeAllocator.Allocate()) EmptyHNode(N);
  पक्ष ग्रन्थिः::NK_Alias: {
    Alias_ग्रन्थिः *AN = dyn_cast<Alias_ग्रन्थिः>(N);
    auto AliasName = AN->getName();
    auto AHN = AliasMap.find(AliasName);
    सचे (AHN == AliasMap.end()) {
      setError(AN, Twine("undefined alias '" + AliasName + "'"));
      पटिनिवत्त nullptr;
    }
    पटिनिवत्त AHN->second;
  }
  स्वभाव:
    setError(N, "unknown node kind");
    पटिनिवत्त nullptr;
  }
}

शून्य निवेशः::setError(ध्रुव Twine &Message) {
  setError(Current_ग्रन्थिः, Message);
}

शून्य निवेशः::set_Allow_Unknown_कुञ्चिकाः(तार्किक Allow) { Allow_Unknown_कुञ्चिकाः = Allow; }

तार्किक निवेशः::canElideEmptySequence() {
  पटिनिवत्त मिच्छा;
}

//===----------------------------------------------------------------------===//
//  Output
//===----------------------------------------------------------------------===//

Output::Output(raw_ostream &yout, शून्य *context, पूर्णाङ्क WrapColumn)
    : IO(context), Out(yout), WrapColumn(WrapColumn) {}

Output::~Output() = स्वभाव;

तार्किक Output::outputting() ध्रुव {
  पटिनिवत्त सच्च;
}

शून्य Output::beginMapping() {
  State_राशिः.प्रक्षिप_back(in_Map_First_कुञ्चिका);
  PaddingBeforeContainer = Padding;
  Padding = "\n";
}

तार्किक Output::mapTag(StringRef Tag, तार्किक Use) {
  सचे (Use) {
    // If this tag is being written inside a sequence we should write the start
    // of the sequence before writing the tag, otherwise the tag won't be
    // attached to the element in the sequence, but rather the sequence itself.
    तार्किक SequenceElement = मिच्छा;
    सचे (State_राशिः.size() > 1) {
      auto &E = State_राशिः[State_राशिः.size() - 2];
      SequenceElement = inSeqAnyElement(E) || inFlowSeqAnyElement(E);
    }
    सचे (SequenceElement && State_राशिः.back() == in_Map_First_कुञ्चिका) {
      new_पङ्क्तिः_Check();
    } अञ्ञथा {
      output(" ");
    }
    output(Tag);
    सचे (SequenceElement) {
      // If we're writing the tag during the first element of a map, the tag
      // takes the place of the first element in the sequence.
      सचे (State_राशिः.back() == in_Map_First_कुञ्चिका) {
        State_राशिः.निष्कासय_back();
        State_राशिः.प्रक्षिप_back(in_Map_Other_कुञ्चिका);
      }
      // Tags inside maps in sequences should act as keys in the map from a
      // formatting perspective, so we always want a newline in a sequence.
      Padding = "\n";
    }
  }
  पटिनिवत्त Use;
}

शून्य Output::endMapping() {
  // If we did not map anything, we should explicitly emit an empty map
  सचे (State_राशिः.back() == in_Map_First_कुञ्चिका) {
    Padding = PaddingBeforeContainer;
    new_पङ्क्तिः_Check();
    output("{}");
    Padding = "\n";
  }
  State_राशिः.निष्कासय_back();
}

std::vector<StringRef> Output::कुञ्चिकाः() {
  report_fatal_error("invalid call");
}

तार्किक Output::preflight_कुञ्चिका(StringRef कुञ्चिका, तार्किक Required, तार्किक SameAsDefault,
                          तार्किक &UseDefault, शून्य *&SaveInfo) {
  UseDefault = मिच्छा;
  SaveInfo = nullptr;
  सचे (Required || !SameAsDefault || Write_Default_मानानि) {
    auto State = State_राशिः.back();
    सचे (State == in_Flow_Map_First_कुञ्चिका || State == in_Flow_Map_Other_कुञ्चिका) {
      flow_कुञ्चिका(कुञ्चिका);
    } अञ्ञथा {
      new_पङ्क्तिः_Check();
      padded_कुञ्चिका(कुञ्चिका);
    }
    पटिनिवत्त सच्च;
  }
  पटिनिवत्त मिच्छा;
}

शून्य Output::postflight_कुञ्चिका(शून्य *) {
  सचे (State_राशिः.back() == in_Map_First_कुञ्चिका) {
    State_राशिः.निष्कासय_back();
    State_राशिः.प्रक्षिप_back(in_Map_Other_कुञ्चिका);
  } अञ्ञथा सचे (State_राशिः.back() == in_Flow_Map_First_कुञ्चिका) {
    State_राशिः.निष्कासय_back();
    State_राशिः.प्रक्षिप_back(in_Flow_Map_Other_कुञ्चिका);
  }
}

शून्य Output::beginFlowMapping() {
  State_राशिः.प्रक्षिप_back(in_Flow_Map_First_कुञ्चिका);
  new_पङ्क्तिः_Check();
  ColumnAtMapFlowStart = Column;
  output("{ ");
}

शून्य Output::endFlowMapping() {
  State_राशिः.निष्कासय_back();
  output_Up_To_End_Of_पङ्क्तिः(" }");
}

शून्य Output::beginDocuments() {
  output_Up_To_End_Of_पङ्क्तिः("---");
}

तार्किक Output::preflightDocument(unsigned index) {
  सचे (index > 0)
    output_Up_To_End_Of_पङ्क्तिः("\n---");
  पटिनिवत्त सच्च;
}

शून्य Output::postflightDocument() {
}

शून्य Output::endDocuments() {
  output("\n...\n");
}

unsigned Output::beginSequence() {
  State_राशिः.प्रक्षिप_back(inSeqFirstElement);
  PaddingBeforeContainer = Padding;
  Padding = "\n";
  पटिनिवत्त 0;
}

शून्य Output::endSequence() {
  // If we did not emit anything, we should explicitly emit an empty sequence
  सचे (State_राशिः.back() == inSeqFirstElement) {
    Padding = PaddingBeforeContainer;
    new_पङ्क्तिः_Check(/*EmptySequence=*/सच्च);
    output("[]");
    Padding = "\n";
  }
  State_राशिः.निष्कासय_back();
}

तार्किक Output::preflightElement(unsigned, शून्य *&SaveInfo) {
  SaveInfo = nullptr;
  पटिनिवत्त सच्च;
}

शून्य Output::postflightElement(शून्य *) {
  सचे (State_राशिः.back() == inSeqFirstElement) {
    State_राशिः.निष्कासय_back();
    State_राशिः.प्रक्षिप_back(inSeqOtherElement);
  } अञ्ञथा सचे (State_राशिः.back() == inFlowSeqFirstElement) {
    State_राशिः.निष्कासय_back();
    State_राशिः.प्रक्षिप_back(inFlowSeqOtherElement);
  }
}

unsigned Output::beginFlowSequence() {
  State_राशिः.प्रक्षिप_back(inFlowSeqFirstElement);
  new_पङ्क्तिः_Check();
  ColumnAtFlowStart = Column;
  output("[ ");
  NeedFlowSequenceComma = मिच्छा;
  पटिनिवत्त 0;
}

शून्य Output::endFlowSequence() {
  State_राशिः.निष्कासय_back();
  output_Up_To_End_Of_पङ्क्तिः(" ]");
}

तार्किक Output::preflightFlowElement(unsigned, शून्य *&SaveInfo) {
  सचे (NeedFlowSequenceComma)
    output(", ");
  सचे (WrapColumn && Column > WrapColumn) {
    output("\n");
    अत्थाय (पूर्णाङ्क i = 0; i < ColumnAtFlowStart; ++i)
      output(" ");
    Column = ColumnAtFlowStart;
    output("  ");
  }
  SaveInfo = nullptr;
  पटिनिवत्त सच्च;
}

शून्य Output::postflightFlowElement(शून्य *) {
  NeedFlowSequenceComma = सच्च;
}

शून्य Output::beginEnumScalar() {
  EnumerationMatchFound = मिच्छा;
}

तार्किक Output::matchEnumScalar(StringRef Str, तार्किक Match) {
  सचे (Match && !EnumerationMatchFound) {
    new_पङ्क्तिः_Check();
    output_Up_To_End_Of_पङ्क्तिः(Str);
    EnumerationMatchFound = सच्च;
  }
  पटिनिवत्त मिच्छा;
}

तार्किक Output::matchEnumFallback() {
  सचे (EnumerationMatchFound)
    पटिनिवत्त मिच्छा;
  EnumerationMatchFound = सच्च;
  पटिनिवत्त सच्च;
}

शून्य Output::endEnumScalar() {
  सचे (!EnumerationMatchFound)
    llvm_unreachable("bad runtime enum value");
}

तार्किक Output::beginBitSetScalar(तार्किक &DoClear) {
  new_पङ्क्तिः_Check();
  output("[ ");
  Need_Bit_मानम्_Comma = मिच्छा;
  DoClear = मिच्छा;
  पटिनिवत्त सच्च;
}

तार्किक Output::bitSetMatch(StringRef Str, तार्किक Matches) {
  सचे (Matches) {
    सचे (Need_Bit_मानम्_Comma)
      output(", ");
    output(Str);
    Need_Bit_मानम्_Comma = सच्च;
  }
  पटिनिवत्त मिच्छा;
}

शून्य Output::endBitSetScalar() {
  output_Up_To_End_Of_पङ्क्तिः(" ]");
}

शून्य Output::scalarString(StringRef &S, QuotingType MustQuote) {
  new_पङ्क्तिः_Check();
  सचे (S.empty()) {
    // Print '' for the empty string because leaving the field empty is not
    // allowed.
    output_Up_To_End_Of_पङ्क्तिः("''");
    पटिनिवत्त;
  }
  output(S, MustQuote);
  output_Up_To_End_Of_पङ्क्तिः("");
}

शून्य Output::blockScalarString(StringRef &S) {
  सचे (!State_राशिः.empty())
    new_पङ्क्तिः_Check();
  output(" |");

  unsigned Indent = State_राशिः.empty() ? 1 : State_राशिः.size();

  auto Buffer = MemoryBuffer::getMemBuffer(S, "", मिच्छा);
  अत्थाय (पङ्क्तिः_iterator पङ्क्तयः(*Buffer, मिच्छा); !पङ्क्तयः.is_at_end(); ++पङ्क्तयः) {
    output_New_पङ्क्तिः();
    अत्थाय (unsigned I = 0; I < Indent; ++I) {
      output("  ");
    }
    output(*पङ्क्तयः);
  }
  output_Up_To_End_Of_पङ्क्तिः("");
}

शून्य Output::scalarTag(std::string &Tag) {
  सचे (Tag.empty())
    पटिनिवत्त;
  new_पङ्क्तिः_Check();
  output(Tag);
  output(" ");
}

शून्य Output::setError(ध्रुव Twine &message) {
}

std::error_code Output::error() { पटिनिवत्त {}; }

तार्किक Output::canElideEmptySequence() {
  // Normally, with an optional key/value where the value is an empty sequence,
  // the whole key/value can be not written.  But, that produces wrong yaml
  // if the key/value is the only thing in the map and the map is used in
  // a sequence.  This detects if the this sequence is the first key/value
  // in map that itself is embedded in a sequence.
  सचे (State_राशिः.size() < 2)
    पटिनिवत्त सच्च;
  सचे (State_राशिः.back() != in_Map_First_कुञ्चिका)
    पटिनिवत्त सच्च;
  पटिनिवत्त !inSeqAnyElement(State_राशिः[State_राशिः.size() - 2]);
}

शून्य Output::output(StringRef s) {
  Column += s.size();
  Out << s;
}

शून्य Output::output(StringRef S, QuotingType MustQuote) {
  सचे (MustQuote == QuotingType::None) {
    // Only quote if we must.
    output(S);
    पटिनिवत्त;
  }

  StringLiteral Quote = MustQuote == QuotingType::Single ? StringLiteral("'")
                                                         : StringLiteral("\"");
  output(Quote); // Starting quote.

  // When using double-quoted strings (and only in that case), non-printable
  // characters may be present, and will be escaped using a variety of
  // unicode-scalar and special short-form escapes. This is handled in
  // yaml::escape.
  सचे (MustQuote == QuotingType::Double) {
    output(yaml::escape(S, /* EscapePrintable= */ मिच्छा));
    output(Quote);
    पटिनिवत्त;
  }

  unsigned i = 0;
  unsigned j = 0;
  unsigned End = S.size();
  ध्रुव वर्ण *Base = S.data();

  // When using single-quoted strings, any single quote ' must be doubled to be
  // escaped.
  याव (j < End) {
    सचे (S[j] == '\'') {                   // Escape quotes.
      output(StringRef(&Base[i], j - i)); // "flush".
      output(StringLiteral("''"));        // Print it as ''
      i = j + 1;
    }
    ++j;
  }
  output(StringRef(&Base[i], j - i));
  output(Quote); // Ending quote.
}

शून्य Output::output_Up_To_End_Of_पङ्क्तिः(StringRef s) {
  output(s);
  सचे (State_राशिः.empty() || (!inFlowSeqAnyElement(State_राशिः.back()) &&
                             !in_Flow_Map_Any_कुञ्चिका(State_राशिः.back())))
    Padding = "\n";
}

शून्य Output::output_New_पङ्क्तिः() {
  Out << "\n";
  Column = 0;
}

// if seq at top, indent as if map, then add "- "
// if seq in middle, use "- " if firstKey, else use "  "
//

शून्य Output::new_पङ्क्तिः_Check(तार्किक EmptySequence) {
  सचे (Padding != "\n") {
    output(Padding);
    Padding = {};
    पटिनिवत्त;
  }
  output_New_पङ्क्तिः();
  Padding = {};

  सचे (State_राशिः.size() == 0 || EmptySequence)
    पटिनिवत्त;

  unsigned Indent = State_राशिः.size() - 1;
  तार्किक PossiblyNestedSeq = मिच्छा;
  auto I = State_राशिः.rbegin(), E = State_राशिः.rend();

  सचे (inSeqAnyElement(*I)) {
    PossiblyNestedSeq = सच्च; // Not possibly but always.
    ++Indent;
  } अञ्ञथा सचे (*I == in_Map_First_कुञ्चिका || *I == in_Flow_Map_First_कुञ्चिका ||
             inFlowSeqAnyElement(*I)) {
    PossiblyNestedSeq = सच्च;
    ++I; // Skip back().
  }

  unsigned Output_Dash_गणनम् = 0;
  सचे (PossiblyNestedSeq) {
    // Count up consecutive inSeqFirstElement from the end, unless
    // inSeqFirstElement is the top of nested sequence.
    याव (I != E) {
      // Don't count the top of nested sequence.
      सचे (!inSeqAnyElement(*I))
        भङ्ग;

      ++Output_Dash_गणनम्;

      // Stop counting if consecutive inSeqFirstElement ends.
      सचे (*I++ != inSeqFirstElement)
        भङ्ग;
    }
  }

  अत्थाय (unsigned I = Output_Dash_गणनम्; I < Indent; ++I)
    output("  ");

  अत्थाय (unsigned I = 0; I < Output_Dash_गणनम्; ++I)
    output("- ");
}

शून्य Output::padded_कुञ्चिका(StringRef कुञ्चिका_2) {
  output(कुञ्चिका_2, needsQuotes(कुञ्चिका_2, मिच्छा));
  output(":");
  ध्रुव वर्ण *spaces = "                ";
  सचे (कुञ्चिका_2.size() < strlen(spaces))
    Padding = &spaces[कुञ्चिका_2.size()];
  अञ्ञथा
    Padding = " ";
}

शून्य Output::flow_कुञ्चिका(StringRef कुञ्चिका) {
  सचे (State_राशिः.back() == in_Flow_Map_Other_कुञ्चिका)
    output(", ");
  सचे (WrapColumn && Column > WrapColumn) {
    output("\n");
    अत्थाय (पूर्णाङ्क I = 0; I < ColumnAtMapFlowStart; ++I)
      output(" ");
    Column = ColumnAtMapFlowStart;
    output("  ");
  }
  output(कुञ्चिका, needsQuotes(कुञ्चिका, मिच्छा));
  output(": ");
}

ग्रन्थिः_Kind Output::get_ग्रन्थिः_Kind() { report_fatal_error("invalid call"); }

तार्किक Output::inSeqAnyElement(InState State) {
  पटिनिवत्त State == inSeqFirstElement || State == inSeqOtherElement;
}

तार्किक Output::inFlowSeqAnyElement(InState State) {
  पटिनिवत्त State == inFlowSeqFirstElement || State == inFlowSeqOtherElement;
}

तार्किक Output::in_Map_Any_कुञ्चिका(InState State) {
  पटिनिवत्त State == in_Map_First_कुञ्चिका || State == in_Map_Other_कुञ्चिका;
}

तार्किक Output::in_Flow_Map_Any_कुञ्चिका(InState State) {
  पटिनिवत्त State == in_Flow_Map_First_कुञ्चिका || State == in_Flow_Map_Other_कुञ्चिका;
}

//===----------------------------------------------------------------------===//
//  traits for built-in types
//===----------------------------------------------------------------------===//

शून्य ScalarTraits<तार्किक>::output(ध्रुव तार्किक &Val, शून्य *, raw_ostream &Out) {
  Out << (Val ? "true" : "false");
}

StringRef ScalarTraits<तार्किक>::निवेशः_2(StringRef Scalar, शून्य *, तार्किक &Val) {
  सचे (std::optional<तार्किक> Parsed = parseBool(Scalar)) {
    Val = *Parsed;
    पटिनिवत्त StringRef();
  }
  पटिनिवत्त "invalid boolean";
}

शून्य ScalarTraits<StringRef>::output(ध्रुव StringRef &Val, शून्य *,
                                     raw_ostream &Out) {
  Out << Val;
}

StringRef ScalarTraits<StringRef>::निवेशः_2(StringRef Scalar, शून्य *,
                                         StringRef &Val) {
  Val = Scalar;
  पटिनिवत्त StringRef();
}

शून्य ScalarTraits<std::string>::output(ध्रुव std::string &Val, शून्य *,
                                       raw_ostream &Out) {
  Out << Val;
}

StringRef ScalarTraits<std::string>::निवेशः_2(StringRef Scalar, शून्य *,
                                           std::string &Val) {
  Val = Scalar.str();
  पटिनिवत्त StringRef();
}

शून्य ScalarTraits<uint8_t>::output(ध्रुव uint8_t &Val, शून्य *,
                                   raw_ostream &Out) {
  // use temp uin32_t because ostream thinks uint8_t is a character
  uint32_t अङ्कः = Val;
  Out << अङ्कः;
}

StringRef ScalarTraits<uint8_t>::निवेशः_2(StringRef Scalar, शून्य *, uint8_t &Val) {
  unsigned long long n;
  सचे (getAsUnsignedInteger(Scalar, 0, n))
    पटिनिवत्त "invalid number";
  सचे (n > 0xFF)
    पटिनिवत्त "out of range number";
  Val = n;
  पटिनिवत्त StringRef();
}

शून्य ScalarTraits<uint16_t>::output(ध्रुव uint16_t &Val, शून्य *,
                                    raw_ostream &Out) {
  Out << Val;
}

StringRef ScalarTraits<uint16_t>::निवेशः_2(StringRef Scalar, शून्य *,
                                        uint16_t &Val) {
  unsigned long long n;
  सचे (getAsUnsignedInteger(Scalar, 0, n))
    पटिनिवत्त "invalid number";
  सचे (n > 0xFFFF)
    पटिनिवत्त "out of range number";
  Val = n;
  पटिनिवत्त StringRef();
}

शून्य ScalarTraits<uint32_t>::output(ध्रुव uint32_t &Val, शून्य *,
                                    raw_ostream &Out) {
  Out << Val;
}

StringRef ScalarTraits<uint32_t>::निवेशः_2(StringRef Scalar, शून्य *,
                                        uint32_t &Val) {
  unsigned long long n;
  सचे (getAsUnsignedInteger(Scalar, 0, n))
    पटिनिवत्त "invalid number";
  सचे (n > 0xFFFFFFFFUL)
    पटिनिवत्त "out of range number";
  Val = n;
  पटिनिवत्त StringRef();
}

शून्य ScalarTraits<uint64_t>::output(ध्रुव uint64_t &Val, शून्य *,
                                    raw_ostream &Out) {
  Out << Val;
}

StringRef ScalarTraits<uint64_t>::निवेशः_2(StringRef Scalar, शून्य *,
                                        uint64_t &Val) {
  unsigned long long N;
  सचे (getAsUnsignedInteger(Scalar, 0, N))
    पटिनिवत्त "invalid number";
  Val = N;
  पटिनिवत्त StringRef();
}

शून्य ScalarTraits<int8_t>::output(ध्रुव int8_t &Val, शून्य *, raw_ostream &Out) {
  // use temp in32_t because ostream thinks int8_t is a character
  int32_t अङ्कः = Val;
  Out << अङ्कः;
}

StringRef ScalarTraits<int8_t>::निवेशः_2(StringRef Scalar, शून्य *, int8_t &Val) {
  long long N;
  सचे (getAsSignedInteger(Scalar, 0, N))
    पटिनिवत्त "invalid number";
  सचे ((N > 127) || (N < -128))
    पटिनिवत्त "out of range number";
  Val = N;
  पटिनिवत्त StringRef();
}

शून्य ScalarTraits<int16_t>::output(ध्रुव int16_t &Val, शून्य *,
                                   raw_ostream &Out) {
  Out << Val;
}

StringRef ScalarTraits<int16_t>::निवेशः_2(StringRef Scalar, शून्य *, int16_t &Val) {
  long long N;
  सचे (getAsSignedInteger(Scalar, 0, N))
    पटिनिवत्त "invalid number";
  सचे ((N > INT16_MAX) || (N < INT16_MIN))
    पटिनिवत्त "out of range number";
  Val = N;
  पटिनिवत्त StringRef();
}

शून्य ScalarTraits<int32_t>::output(ध्रुव int32_t &Val, शून्य *,
                                   raw_ostream &Out) {
  Out << Val;
}

StringRef ScalarTraits<int32_t>::निवेशः_2(StringRef Scalar, शून्य *, int32_t &Val) {
  long long N;
  सचे (getAsSignedInteger(Scalar, 0, N))
    पटिनिवत्त "invalid number";
  सचे ((N > INT32_MAX) || (N < INT32_MIN))
    पटिनिवत्त "out of range number";
  Val = N;
  पटिनिवत्त StringRef();
}

शून्य ScalarTraits<int64_t>::output(ध्रुव int64_t &Val, शून्य *,
                                   raw_ostream &Out) {
  Out << Val;
}

StringRef ScalarTraits<int64_t>::निवेशः_2(StringRef Scalar, शून्य *, int64_t &Val) {
  long long N;
  सचे (getAsSignedInteger(Scalar, 0, N))
    पटिनिवत्त "invalid number";
  Val = N;
  पटिनिवत्त StringRef();
}

शून्य ScalarTraits<द्विगुण>::output(ध्रुव द्विगुण &Val, शून्य *, raw_ostream &Out) {
  Out << format("%g", Val);
}

StringRef ScalarTraits<द्विगुण>::निवेशः_2(StringRef Scalar, शून्य *, द्विगुण &Val) {
  सचे (to_float(Scalar, Val))
    पटिनिवत्त StringRef();
  पटिनिवत्त "invalid floating point number";
}

शून्य ScalarTraits<भिन्न>::output(ध्रुव भिन्न &Val, शून्य *, raw_ostream &Out) {
  Out << format("%g", Val);
}

StringRef ScalarTraits<भिन्न>::निवेशः_2(StringRef Scalar, शून्य *, भिन्न &Val) {
  सचे (to_float(Scalar, Val))
    पटिनिवत्त StringRef();
  पटिनिवत्त "invalid floating point number";
}

शून्य ScalarTraits<Hex8>::output(ध्रुव Hex8 &Val, शून्य *, raw_ostream &Out) {
  Out << format("0x%" PRIX8, (uint8_t)Val);
}

StringRef ScalarTraits<Hex8>::निवेशः_2(StringRef Scalar, शून्य *, Hex8 &Val) {
  unsigned long long n;
  सचे (getAsUnsignedInteger(Scalar, 0, n))
    पटिनिवत्त "invalid hex8 number";
  सचे (n > 0xFF)
    पटिनिवत्त "out of range hex8 number";
  Val = n;
  पटिनिवत्त StringRef();
}

शून्य ScalarTraits<Hex16>::output(ध्रुव Hex16 &Val, शून्य *, raw_ostream &Out) {
  Out << format("0x%" PRIX16, (uint16_t)Val);
}

StringRef ScalarTraits<Hex16>::निवेशः_2(StringRef Scalar, शून्य *, Hex16 &Val) {
  unsigned long long n;
  सचे (getAsUnsignedInteger(Scalar, 0, n))
    पटिनिवत्त "invalid hex16 number";
  सचे (n > 0xFFFF)
    पटिनिवत्त "out of range hex16 number";
  Val = n;
  पटिनिवत्त StringRef();
}

शून्य ScalarTraits<Hex32>::output(ध्रुव Hex32 &Val, शून्य *, raw_ostream &Out) {
  Out << format("0x%" PRIX32, (uint32_t)Val);
}

StringRef ScalarTraits<Hex32>::निवेशः_2(StringRef Scalar, शून्य *, Hex32 &Val) {
  unsigned long long n;
  सचे (getAsUnsignedInteger(Scalar, 0, n))
    पटिनिवत्त "invalid hex32 number";
  सचे (n > 0xFFFFFFFFUL)
    पटिनिवत्त "out of range hex32 number";
  Val = n;
  पटिनिवत्त StringRef();
}

शून्य ScalarTraits<Hex64>::output(ध्रुव Hex64 &Val, शून्य *, raw_ostream &Out) {
  Out << format("0x%" PRIX64, (uint64_t)Val);
}

StringRef ScalarTraits<Hex64>::निवेशः_2(StringRef Scalar, शून्य *, Hex64 &Val) {
  unsigned long long अङ्कः;
  सचे (getAsUnsignedInteger(Scalar, 0, अङ्कः))
    पटिनिवत्त "invalid hex64 number";
  Val = अङ्कः;
  पटिनिवत्त StringRef();
}

शून्य ScalarTraits<VersionTuple>::output(ध्रुव VersionTuple &Val, शून्य *,
                                        llvm::raw_ostream &Out) {
  Out << Val.getAsString();
}

StringRef ScalarTraits<VersionTuple>::निवेशः_2(StringRef Scalar, शून्य *,
                                            VersionTuple &Val) {
  सचे (Val.tryParse(Scalar))
    पटिनिवत्त "invalid version format";
  पटिनिवत्त StringRef();
}
