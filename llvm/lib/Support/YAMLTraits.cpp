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
  प्रत्यागम Ctxt;
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
    : IO(Ctxt), Strm(नूतन Stream(निवेशः_Content, SrcMgr, मिच्छ, &EC)) {
  जइ (DiagHandler)
    SrcMgr.setDiagHandler(DiagHandler, DiagHandlerCtxt);
  DocIterator = Strm->begin();
}

निवेशः::निवेशः(MemoryBufferRef निवेशः, शून्य *Ctxt,
             SourceMgr::DiagHandlerTy DiagHandler, शून्य *DiagHandlerCtxt)
    : IO(Ctxt), Strm(नूतन Stream(निवेशः, SrcMgr, मिच्छ, &EC)) {
  जइ (DiagHandler)
    SrcMgr.setDiagHandler(DiagHandler, DiagHandlerCtxt);
  DocIterator = Strm->begin();
}

निवेशः::~निवेशः() = स्वभाव;

std::error_code निवेशः::error() { प्रत्यागम EC; }

तार्किक निवेशः::outputting() ध्रुव {
  प्रत्यागम मिच्छ;
}

तार्किक निवेशः::setCurrentDocument() {
  जइ (DocIterator != Strm->end()) {
    ग्रन्थिः *N = DocIterator->get_मूलम्();
    जइ (!N) {
      EC = make_error_code(errc::invalid_argument);
      प्रत्यागम मिच्छ;
    }

    जइ (isa<Null_ग्रन्थिः>(N)) {
      // Empty files are allowed and ignored
      ++DocIterator;
      प्रत्यागम setCurrentDocument();
    }
    releaseHNodeBuffers();
    शिखरम्_ग्रन्थिः = createHNodes(N);
    Current_ग्रन्थिः = शिखरम्_ग्रन्थिः;
    प्रत्यागम सच्च;
  }
  प्रत्यागम मिच्छ;
}

तार्किक निवेशः::nextDocument() {
  प्रत्यागम ++DocIterator != Strm->end();
}

ध्रुव ग्रन्थिः *निवेशः::get_Current_ग्रन्थिः() ध्रुव {
  प्रत्यागम Current_ग्रन्थिः ? Current_ग्रन्थिः->ग्रन्थिः_2 : nullptr;
}

तार्किक निवेशः::mapTag(StringRef Tag, तार्किक Default) {
  // CurrentNode can be null if setCurrentDocument() was unable to
  // parse the document because it was invalid or empty.
  जइ (!Current_ग्रन्थिः)
    प्रत्यागम मिच्छ;

  std::string foundTag = Current_ग्रन्थिः->ग्रन्थिः_2->getVerbatimTag();
  जइ (foundTag.empty()) {
    // If no tag found and 'Tag' is the default, say it was found.
    प्रत्यागम Default;
  }
  // Return true iff found tag matches supplied tag.
  प्रत्यागम Tag == foundTag;
}

शून्य निवेशः::beginMapping() {
  जइ (EC)
    प्रत्यागम;
  // CurrentNode can be null if the document is empty.
  MapHNode *MN = dyn_cast_or_null<MapHNode>(Current_ग्रन्थिः);
  जइ (MN) {
    MN->Valid_कुञ्चिकाः.clear();
  }
}

std::vector<StringRef> निवेशः::कुञ्चिकाः() {
  MapHNode *MN = dyn_cast<MapHNode>(Current_ग्रन्थिः);
  std::vector<StringRef> Ret;
  जइ (!MN) {
    setError(Current_ग्रन्थिः, "not a mapping");
    प्रत्यागम Ret;
  }
  प्रति (auto &P : MN->Mapping)
    Ret.प्रक्षिप_back(P.first());
  प्रत्यागम Ret;
}

तार्किक निवेशः::preflight_कुञ्चिका(StringRef कुञ्चिका, तार्किक Required, तार्किक, तार्किक &UseDefault,
                         शून्य *&SaveInfo) {
  UseDefault = मिच्छ;
  जइ (EC)
    प्रत्यागम मिच्छ;

  // CurrentNode is null for empty documents, which is an error in case required
  // nodes are present.
  जइ (!Current_ग्रन्थिः) {
    जइ (Required)
      EC = make_error_code(errc::invalid_argument);
    अण्णहा
      UseDefault = सच्च;
    प्रत्यागम मिच्छ;
  }

  MapHNode *MN = dyn_cast<MapHNode>(Current_ग्रन्थिः);
  जइ (!MN) {
    जइ (Required || !isa<EmptyHNode>(Current_ग्रन्थिः))
      setError(Current_ग्रन्थिः, "not a mapping");
    अण्णहा
      UseDefault = सच्च;
    प्रत्यागम मिच्छ;
  }
  MN->Valid_कुञ्चिकाः.प्रक्षिप_back(कुञ्चिका.str());
  HNode *मानम् = MN->Mapping[कुञ्चिका].first;
  जइ (!मानम्) {
    जइ (Required)
      setError(Current_ग्रन्थिः, Twine("missing required key '") + कुञ्चिका + "'");
    अण्णहा
      UseDefault = सच्च;
    प्रत्यागम मिच्छ;
  }
  SaveInfo = Current_ग्रन्थिः;
  Current_ग्रन्थिः = मानम्;
  प्रत्यागम सच्च;
}

शून्य निवेशः::postflight_कुञ्चिका(शून्य *saveInfo) {
  Current_ग्रन्थिः = reinterpret_cast<HNode *>(saveInfo);
}

शून्य निवेशः::endMapping() {
  जइ (EC)
    प्रत्यागम;
  // CurrentNode can be null if the document is empty.
  MapHNode *MN = dyn_cast_or_null<MapHNode>(Current_ग्रन्थिः);
  जइ (!MN)
    प्रत्यागम;
  प्रति (ध्रुव auto &NN : MN->Mapping) {
    जइ (!is_contained(MN->Valid_कुञ्चिकाः, NN.first())) {
      ध्रुव SMRange &ReportLoc = NN.second.second;
      जइ (!Allow_Unknown_कुञ्चिकाः) {
        setError(ReportLoc, Twine("unknown key '") + NN.first() + "'");
        भङ्ग;
      } अण्णहा
        reportWarning(ReportLoc, Twine("unknown key '") + NN.first() + "'");
    }
  }
}

शून्य निवेशः::beginFlowMapping() { beginMapping(); }

शून्य निवेशः::endFlowMapping() { endMapping(); }

unsigned निवेशः::beginSequence() {
  जइ (SequenceHNode *SQ = dyn_cast<SequenceHNode>(Current_ग्रन्थिः))
    प्रत्यागम SQ->Entries.size();
  जइ (isa<EmptyHNode>(Current_ग्रन्थिः))
    प्रत्यागम 0;
  // Treat case where there's a scalar "null" value as an empty sequence.
  जइ (ScalarHNode *SN = dyn_cast<ScalarHNode>(Current_ग्रन्थिः)) {
    जइ (isNull(SN->मानम्_2()))
      प्रत्यागम 0;
  }
  // Any other type of HNode is an error.
  setError(Current_ग्रन्थिः, "not a sequence");
  प्रत्यागम 0;
}

शून्य निवेशः::endSequence() {
}

तार्किक निवेशः::preflightElement(unsigned Index, शून्य *&SaveInfo) {
  जइ (EC)
    प्रत्यागम मिच्छ;
  जइ (SequenceHNode *SQ = dyn_cast<SequenceHNode>(Current_ग्रन्थिः)) {
    SaveInfo = Current_ग्रन्थिः;
    Current_ग्रन्थिः = SQ->Entries[Index];
    प्रत्यागम सच्च;
  }
  प्रत्यागम मिच्छ;
}

शून्य निवेशः::postflightElement(शून्य *SaveInfo) {
  Current_ग्रन्थिः = reinterpret_cast<HNode *>(SaveInfo);
}

unsigned निवेशः::beginFlowSequence() { प्रत्यागम beginSequence(); }

तार्किक निवेशः::preflightFlowElement(unsigned index, शून्य *&SaveInfo) {
  जइ (EC)
    प्रत्यागम मिच्छ;
  जइ (SequenceHNode *SQ = dyn_cast<SequenceHNode>(Current_ग्रन्थिः)) {
    SaveInfo = Current_ग्रन्थिः;
    Current_ग्रन्थिः = SQ->Entries[index];
    प्रत्यागम सच्च;
  }
  प्रत्यागम मिच्छ;
}

शून्य निवेशः::postflightFlowElement(शून्य *SaveInfo) {
  Current_ग्रन्थिः = reinterpret_cast<HNode *>(SaveInfo);
}

शून्य निवेशः::endFlowSequence() {
}

शून्य निवेशः::beginEnumScalar() {
  ScalarMatchFound = मिच्छ;
}

तार्किक निवेशः::matchEnumScalar(StringRef Str, तार्किक) {
  जइ (ScalarMatchFound)
    प्रत्यागम मिच्छ;
  जइ (ScalarHNode *SN = dyn_cast<ScalarHNode>(Current_ग्रन्थिः)) {
    जइ (SN->मानम्_2() == Str) {
      ScalarMatchFound = सच्च;
      प्रत्यागम सच्च;
    }
  }
  प्रत्यागम मिच्छ;
}

तार्किक निवेशः::matchEnumFallback() {
  जइ (ScalarMatchFound)
    प्रत्यागम मिच्छ;
  ScalarMatchFound = सच्च;
  प्रत्यागम सच्च;
}

शून्य निवेशः::endEnumScalar() {
  जइ (!ScalarMatchFound) {
    setError(Current_ग्रन्थिः, "unknown enumerated scalar");
  }
}

तार्किक निवेशः::beginBitSetScalar(तार्किक &DoClear) {
  Bit_मानानि_Used.clear();
  जइ (SequenceHNode *SQ = dyn_cast<SequenceHNode>(Current_ग्रन्थिः)) {
    Bit_मानानि_Used.resize(SQ->Entries.size());
  } अण्णहा {
    setError(Current_ग्रन्थिः, "expected sequence of bit values");
  }
  DoClear = सच्च;
  प्रत्यागम सच्च;
}

तार्किक निवेशः::bitSetMatch(StringRef Str, तार्किक) {
  जइ (EC)
    प्रत्यागम मिच्छ;
  जइ (SequenceHNode *SQ = dyn_cast<SequenceHNode>(Current_ग्रन्थिः)) {
    unsigned Index = 0;
    प्रति (auto &N : SQ->Entries) {
      जइ (ScalarHNode *SN = dyn_cast<ScalarHNode>(N)) {
        जइ (SN->मानम्_2() == Str) {
          Bit_मानानि_Used[Index] = सच्च;
          प्रत्यागम सच्च;
        }
      } अण्णहा {
        setError(Current_ग्रन्थिः, "unexpected scalar in sequence of bit values");
      }
      ++Index;
    }
  } अण्णहा {
    setError(Current_ग्रन्थिः, "expected sequence of bit values");
  }
  प्रत्यागम मिच्छ;
}

शून्य निवेशः::endBitSetScalar() {
  जइ (EC)
    प्रत्यागम;
  जइ (SequenceHNode *SQ = dyn_cast<SequenceHNode>(Current_ग्रन्थिः)) {
    प्रतिज्ञा(Bit_मानानि_Used.size() == SQ->Entries.size());
    प्रति (unsigned i = 0; i < SQ->Entries.size(); ++i) {
      जइ (!Bit_मानानि_Used[i]) {
        setError(SQ->Entries[i], "unknown bit value");
        प्रत्यागम;
      }
    }
  }
}

शून्य निवेशः::scalarString(StringRef &S, QuotingType) {
  जइ (ScalarHNode *SN = dyn_cast<ScalarHNode>(Current_ग्रन्थिः)) {
    S = SN->मानम्_2();
  } अण्णहा {
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
  जइ (isa<ScalarHNode>(Current_ग्रन्थिः))
    प्रत्यागम ग्रन्थिः_Kind::Scalar;
  अण्णहा जइ (isa<MapHNode>(Current_ग्रन्थिः))
    प्रत्यागम ग्रन्थिः_Kind::Map;
  अण्णहा जइ (isa<SequenceHNode>(Current_ग्रन्थिः))
    प्रत्यागम ग्रन्थिः_Kind::Sequence;
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
  जइ (!Anchor.empty())
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
    जइ (!StringStorage.empty()) {
      // Copy string to permanent storage
      कुञ्चिका_Str = StringStorage.str().copy(StringAllocator);
    }
    auto *SHNode = नूतन (ScalarHNodeAllocator.Allocate()) ScalarHNode(N, कुञ्चिका_Str);
    saveAliasHNode(SN, SHNode);
    प्रत्यागम SHNode;
  }
  पक्ष ग्रन्थिः::NK_BlockScalar: {
    Block_Scalar_ग्रन्थिः *BSN = dyn_cast<Block_Scalar_ग्रन्थिः>(N);
    StringRef मानम्_Copy = BSN->get_मानम्().copy(StringAllocator);
    auto *BSHNode =
        नूतन (ScalarHNodeAllocator.Allocate()) ScalarHNode(N, मानम्_Copy);
    saveAliasHNode(BSN, BSHNode);
    प्रत्यागम BSHNode;
  }
  पक्ष ग्रन्थिः::NK_Sequence: {
    Sequence_ग्रन्थिः *SQ = dyn_cast<Sequence_ग्रन्थिः>(N);
    auto SQHNode = नूतन (SequenceHNodeAllocator.Allocate()) SequenceHNode(N);
    प्रति (ग्रन्थिः &SN : *SQ) {
      auto Entry = createHNodes(&SN);
      जइ (EC)
        भङ्ग;
      SQHNode->Entries.प्रक्षिप_back(Entry);
    }
    saveAliasHNode(SQ, SQHNode);
    प्रत्यागम SQHNode;
  }
  पक्ष ग्रन्थिः::NK_Mapping: {
    Mapping_ग्रन्थिः *Map = dyn_cast<Mapping_ग्रन्थिः>(N);
    auto mapHNode = नूतन (MapHNodeAllocator.Allocate()) MapHNode(N);
    प्रति (कुञ्चिका_मानम्_ग्रन्थिः &KVN : *Map) {
      ग्रन्थिः *कुञ्चिका_ग्रन्थिः = KVN.get_कुञ्चिका();
      Scalar_ग्रन्थिः *कुञ्चिका = dyn_cast_if_present<Scalar_ग्रन्थिः>(कुञ्चिका_ग्रन्थिः);
      ग्रन्थिः *मानम् = KVN.get_मानम्();
      जइ (!कुञ्चिका || !मानम्) {
        जइ (!कुञ्चिका)
          setError(कुञ्चिका_ग्रन्थिः, "Map key must be a scalar");
        जइ (!मानम्)
          setError(कुञ्चिका_ग्रन्थिः, "Map value must not be empty");
        भङ्ग;
      }
      StringStorage.clear();
      StringRef कुञ्चिका_Str = कुञ्चिका->get_मानम्(StringStorage);
      जइ (!StringStorage.empty()) {
        // Copy string to permanent storage
        कुञ्चिका_Str = StringStorage.str().copy(StringAllocator);
      }
      जइ (mapHNode->Mapping.गणनम्(कुञ्चिका_Str))
        // From YAML spec: "The content of a mapping node is an unordered set of
        // key/value node pairs, with the restriction that each of the keys is
        // unique."
        setError(कुञ्चिका_ग्रन्थिः, Twine("duplicated mapping key '") + कुञ्चिका_Str + "'");
      auto मानम्_HNode = createHNodes(मानम्);
      जइ (EC)
        भङ्ग;
      mapHNode->Mapping[कुञ्चिका_Str] =
          std::make_pair(std::move(मानम्_HNode), कुञ्चिका_ग्रन्थिः->getSourceRange());
    }
    saveAliasHNode(Map, mapHNode);
    प्रत्यागम std::move(mapHNode);
  }
  पक्ष ग्रन्थिः::NK_Null:
    // TODO: Anchor is not set for NullNode in the parser. Update the parser to
    // faithfully preserve anchors.
    प्रत्यागम नूतन (EmptyHNodeAllocator.Allocate()) EmptyHNode(N);
  पक्ष ग्रन्थिः::NK_Alias: {
    Alias_ग्रन्थिः *AN = dyn_cast<Alias_ग्रन्थिः>(N);
    auto AliasName = AN->getName();
    auto AHN = AliasMap.find(AliasName);
    जइ (AHN == AliasMap.end()) {
      setError(AN, Twine("undefined alias '" + AliasName + "'"));
      प्रत्यागम nullptr;
    }
    प्रत्यागम AHN->second;
  }
  स्वभाव:
    setError(N, "unknown node kind");
    प्रत्यागम nullptr;
  }
}

शून्य निवेशः::setError(ध्रुव Twine &Message) {
  setError(Current_ग्रन्थिः, Message);
}

शून्य निवेशः::set_Allow_Unknown_कुञ्चिकाः(तार्किक Allow) { Allow_Unknown_कुञ्चिकाः = Allow; }

तार्किक निवेशः::canElideEmptySequence() {
  प्रत्यागम मिच्छ;
}

//===----------------------------------------------------------------------===//
//  Output
//===----------------------------------------------------------------------===//

Output::Output(raw_ostream &yout, शून्य *context, पूर्णाङ्क WrapColumn)
    : IO(context), Out(yout), WrapColumn(WrapColumn) {}

Output::~Output() = स्वभाव;

तार्किक Output::outputting() ध्रुव {
  प्रत्यागम सच्च;
}

शून्य Output::beginMapping() {
  State_राशिः.प्रक्षिप_back(in_Map_First_कुञ्चिका);
  PaddingBeforeContainer = Padding;
  Padding = "\n";
}

तार्किक Output::mapTag(StringRef Tag, तार्किक Use) {
  जइ (Use) {
    // If this tag is being written inside a sequence we should write the start
    // of the sequence before writing the tag, otherwise the tag won't be
    // attached to the element in the sequence, but rather the sequence itself.
    तार्किक SequenceElement = मिच्छ;
    जइ (State_राशिः.size() > 1) {
      auto &E = State_राशिः[State_राशिः.size() - 2];
      SequenceElement = inSeqAnyElement(E) || inFlowSeqAnyElement(E);
    }
    जइ (SequenceElement && State_राशिः.back() == in_Map_First_कुञ्चिका) {
      new_पङ्क्तिः_Check();
    } अण्णहा {
      output(" ");
    }
    output(Tag);
    जइ (SequenceElement) {
      // If we're writing the tag during the first element of a map, the tag
      // takes the place of the first element in the sequence.
      जइ (State_राशिः.back() == in_Map_First_कुञ्चिका) {
        State_राशिः.निष्कासय_back();
        State_राशिः.प्रक्षिप_back(in_Map_Other_कुञ्चिका);
      }
      // Tags inside maps in sequences should act as keys in the map from a
      // formatting perspective, so we always want a newline in a sequence.
      Padding = "\n";
    }
  }
  प्रत्यागम Use;
}

शून्य Output::endMapping() {
  // If we did not map anything, we should explicitly emit an empty map
  जइ (State_राशिः.back() == in_Map_First_कुञ्चिका) {
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
  UseDefault = मिच्छ;
  SaveInfo = nullptr;
  जइ (Required || !SameAsDefault || Write_Default_मानानि) {
    auto State = State_राशिः.back();
    जइ (State == in_Flow_Map_First_कुञ्चिका || State == in_Flow_Map_Other_कुञ्चिका) {
      flow_कुञ्चिका(कुञ्चिका);
    } अण्णहा {
      new_पङ्क्तिः_Check();
      padded_कुञ्चिका(कुञ्चिका);
    }
    प्रत्यागम सच्च;
  }
  प्रत्यागम मिच्छ;
}

शून्य Output::postflight_कुञ्चिका(शून्य *) {
  जइ (State_राशिः.back() == in_Map_First_कुञ्चिका) {
    State_राशिः.निष्कासय_back();
    State_राशिः.प्रक्षिप_back(in_Map_Other_कुञ्चिका);
  } अण्णहा जइ (State_राशिः.back() == in_Flow_Map_First_कुञ्चिका) {
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
  जइ (index > 0)
    output_Up_To_End_Of_पङ्क्तिः("\n---");
  प्रत्यागम सच्च;
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
  प्रत्यागम 0;
}

शून्य Output::endSequence() {
  // If we did not emit anything, we should explicitly emit an empty sequence
  जइ (State_राशिः.back() == inSeqFirstElement) {
    Padding = PaddingBeforeContainer;
    new_पङ्क्तिः_Check(/*EmptySequence=*/सच्च);
    output("[]");
    Padding = "\n";
  }
  State_राशिः.निष्कासय_back();
}

तार्किक Output::preflightElement(unsigned, शून्य *&SaveInfo) {
  SaveInfo = nullptr;
  प्रत्यागम सच्च;
}

शून्य Output::postflightElement(शून्य *) {
  जइ (State_राशिः.back() == inSeqFirstElement) {
    State_राशिः.निष्कासय_back();
    State_राशिः.प्रक्षिप_back(inSeqOtherElement);
  } अण्णहा जइ (State_राशिः.back() == inFlowSeqFirstElement) {
    State_राशिः.निष्कासय_back();
    State_राशिः.प्रक्षिप_back(inFlowSeqOtherElement);
  }
}

unsigned Output::beginFlowSequence() {
  State_राशिः.प्रक्षिप_back(inFlowSeqFirstElement);
  new_पङ्क्तिः_Check();
  ColumnAtFlowStart = Column;
  output("[ ");
  NeedFlowSequenceComma = मिच्छ;
  प्रत्यागम 0;
}

शून्य Output::endFlowSequence() {
  State_राशिः.निष्कासय_back();
  output_Up_To_End_Of_पङ्क्तिः(" ]");
}

तार्किक Output::preflightFlowElement(unsigned, शून्य *&SaveInfo) {
  जइ (NeedFlowSequenceComma)
    output(", ");
  जइ (WrapColumn && Column > WrapColumn) {
    output("\n");
    प्रति (पूर्णाङ्क i = 0; i < ColumnAtFlowStart; ++i)
      output(" ");
    Column = ColumnAtFlowStart;
    output("  ");
  }
  SaveInfo = nullptr;
  प्रत्यागम सच्च;
}

शून्य Output::postflightFlowElement(शून्य *) {
  NeedFlowSequenceComma = सच्च;
}

शून्य Output::beginEnumScalar() {
  EnumerationMatchFound = मिच्छ;
}

तार्किक Output::matchEnumScalar(StringRef Str, तार्किक Match) {
  जइ (Match && !EnumerationMatchFound) {
    new_पङ्क्तिः_Check();
    output_Up_To_End_Of_पङ्क्तिः(Str);
    EnumerationMatchFound = सच्च;
  }
  प्रत्यागम मिच्छ;
}

तार्किक Output::matchEnumFallback() {
  जइ (EnumerationMatchFound)
    प्रत्यागम मिच्छ;
  EnumerationMatchFound = सच्च;
  प्रत्यागम सच्च;
}

शून्य Output::endEnumScalar() {
  जइ (!EnumerationMatchFound)
    llvm_unreachable("bad runtime enum value");
}

तार्किक Output::beginBitSetScalar(तार्किक &DoClear) {
  new_पङ्क्तिः_Check();
  output("[ ");
  Need_Bit_मानम्_Comma = मिच्छ;
  DoClear = मिच्छ;
  प्रत्यागम सच्च;
}

तार्किक Output::bitSetMatch(StringRef Str, तार्किक Matches) {
  जइ (Matches) {
    जइ (Need_Bit_मानम्_Comma)
      output(", ");
    output(Str);
    Need_Bit_मानम्_Comma = सच्च;
  }
  प्रत्यागम मिच्छ;
}

शून्य Output::endBitSetScalar() {
  output_Up_To_End_Of_पङ्क्तिः(" ]");
}

शून्य Output::scalarString(StringRef &S, QuotingType MustQuote) {
  new_पङ्क्तिः_Check();
  जइ (S.empty()) {
    // Print '' for the empty string because leaving the field empty is not
    // allowed.
    output_Up_To_End_Of_पङ्क्तिः("''");
    प्रत्यागम;
  }
  output(S, MustQuote);
  output_Up_To_End_Of_पङ्क्तिः("");
}

शून्य Output::blockScalarString(StringRef &S) {
  जइ (!State_राशिः.empty())
    new_पङ्क्तिः_Check();
  output(" |");

  unsigned Indent = State_राशिः.empty() ? 1 : State_राशिः.size();

  auto Buffer = MemoryBuffer::getMemBuffer(S, "", मिच्छ);
  प्रति (पङ्क्तिः_iterator पङ्क्तयः(*Buffer, मिच्छ); !पङ्क्तयः.is_at_end(); ++पङ्क्तयः) {
    output_New_पङ्क्तिः();
    प्रति (unsigned I = 0; I < Indent; ++I) {
      output("  ");
    }
    output(*पङ्क्तयः);
  }
  output_Up_To_End_Of_पङ्क्तिः("");
}

शून्य Output::scalarTag(std::string &Tag) {
  जइ (Tag.empty())
    प्रत्यागम;
  new_पङ्क्तिः_Check();
  output(Tag);
  output(" ");
}

शून्य Output::setError(ध्रुव Twine &message) {
}

std::error_code Output::error() { प्रत्यागम {}; }

तार्किक Output::canElideEmptySequence() {
  // Normally, with an optional key/value where the value is an empty sequence,
  // the whole key/value can be not written.  But, that produces wrong yaml
  // if the key/value is the only thing in the map and the map is used in
  // a sequence.  This detects if the this sequence is the first key/value
  // in map that itself is embedded in a sequence.
  जइ (State_राशिः.size() < 2)
    प्रत्यागम सच्च;
  जइ (State_राशिः.back() != in_Map_First_कुञ्चिका)
    प्रत्यागम सच्च;
  प्रत्यागम !inSeqAnyElement(State_राशिः[State_राशिः.size() - 2]);
}

शून्य Output::output(StringRef s) {
  Column += s.size();
  Out << s;
}

शून्य Output::output(StringRef S, QuotingType MustQuote) {
  जइ (MustQuote == QuotingType::None) {
    // Only quote if we must.
    output(S);
    प्रत्यागम;
  }

  StringLiteral Quote = MustQuote == QuotingType::Single ? StringLiteral("'")
                                                         : StringLiteral("\"");
  output(Quote); // Starting quote.

  // When using double-quoted strings (and only in that case), non-printable
  // characters may be present, and will be escaped using a variety of
  // unicode-scalar and special short-form escapes. This is handled in
  // yaml::escape.
  जइ (MustQuote == QuotingType::Double) {
    output(yaml::escape(S, /* EscapePrintable= */ मिच्छ));
    output(Quote);
    प्रत्यागम;
  }

  unsigned i = 0;
  unsigned j = 0;
  unsigned End = S.size();
  ध्रुव वर्ण *Base = S.data();

  // When using single-quoted strings, any single quote ' must be doubled to be
  // escaped.
  जाव (j < End) {
    जइ (S[j] == '\'') {                   // Escape quotes.
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
  जइ (State_राशिः.empty() || (!inFlowSeqAnyElement(State_राशिः.back()) &&
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
  जइ (Padding != "\n") {
    output(Padding);
    Padding = {};
    प्रत्यागम;
  }
  output_New_पङ्क्तिः();
  Padding = {};

  जइ (State_राशिः.size() == 0 || EmptySequence)
    प्रत्यागम;

  unsigned Indent = State_राशिः.size() - 1;
  तार्किक PossiblyNestedSeq = मिच्छ;
  auto I = State_राशिः.rbegin(), E = State_राशिः.rend();

  जइ (inSeqAnyElement(*I)) {
    PossiblyNestedSeq = सच्च; // Not possibly but always.
    ++Indent;
  } अण्णहा जइ (*I == in_Map_First_कुञ्चिका || *I == in_Flow_Map_First_कुञ्चिका ||
             inFlowSeqAnyElement(*I)) {
    PossiblyNestedSeq = सच्च;
    ++I; // Skip back().
  }

  unsigned Output_Dash_गणनम् = 0;
  जइ (PossiblyNestedSeq) {
    // Count up consecutive inSeqFirstElement from the end, unless
    // inSeqFirstElement is the top of nested sequence.
    जाव (I != E) {
      // Don't count the top of nested sequence.
      जइ (!inSeqAnyElement(*I))
        भङ्ग;

      ++Output_Dash_गणनम्;

      // Stop counting if consecutive inSeqFirstElement ends.
      जइ (*I++ != inSeqFirstElement)
        भङ्ग;
    }
  }

  प्रति (unsigned I = Output_Dash_गणनम्; I < Indent; ++I)
    output("  ");

  प्रति (unsigned I = 0; I < Output_Dash_गणनम्; ++I)
    output("- ");
}

शून्य Output::padded_कुञ्चिका(StringRef कुञ्चिका_2) {
  output(कुञ्चिका_2, needsQuotes(कुञ्चिका_2, मिच्छ));
  output(":");
  ध्रुव वर्ण *spaces = "                ";
  जइ (कुञ्चिका_2.size() < strlen(spaces))
    Padding = &spaces[कुञ्चिका_2.size()];
  अण्णहा
    Padding = " ";
}

शून्य Output::flow_कुञ्चिका(StringRef कुञ्चिका) {
  जइ (State_राशिः.back() == in_Flow_Map_Other_कुञ्चिका)
    output(", ");
  जइ (WrapColumn && Column > WrapColumn) {
    output("\n");
    प्रति (पूर्णाङ्क I = 0; I < ColumnAtMapFlowStart; ++I)
      output(" ");
    Column = ColumnAtMapFlowStart;
    output("  ");
  }
  output(कुञ्चिका, needsQuotes(कुञ्चिका, मिच्छ));
  output(": ");
}

ग्रन्थिः_Kind Output::get_ग्रन्थिः_Kind() { report_fatal_error("invalid call"); }

तार्किक Output::inSeqAnyElement(InState State) {
  प्रत्यागम State == inSeqFirstElement || State == inSeqOtherElement;
}

तार्किक Output::inFlowSeqAnyElement(InState State) {
  प्रत्यागम State == inFlowSeqFirstElement || State == inFlowSeqOtherElement;
}

तार्किक Output::in_Map_Any_कुञ्चिका(InState State) {
  प्रत्यागम State == in_Map_First_कुञ्चिका || State == in_Map_Other_कुञ्चिका;
}

तार्किक Output::in_Flow_Map_Any_कुञ्चिका(InState State) {
  प्रत्यागम State == in_Flow_Map_First_कुञ्चिका || State == in_Flow_Map_Other_कुञ्चिका;
}

//===----------------------------------------------------------------------===//
//  traits for built-in types
//===----------------------------------------------------------------------===//

शून्य ScalarTraits<तार्किक>::output(ध्रुव तार्किक &Val, शून्य *, raw_ostream &Out) {
  Out << (Val ? "true" : "false");
}

StringRef ScalarTraits<तार्किक>::निवेशः_2(StringRef Scalar, शून्य *, तार्किक &Val) {
  जइ (std::optional<तार्किक> Parsed = parseBool(Scalar)) {
    Val = *Parsed;
    प्रत्यागम StringRef();
  }
  प्रत्यागम "invalid boolean";
}

शून्य ScalarTraits<StringRef>::output(ध्रुव StringRef &Val, शून्य *,
                                     raw_ostream &Out) {
  Out << Val;
}

StringRef ScalarTraits<StringRef>::निवेशः_2(StringRef Scalar, शून्य *,
                                         StringRef &Val) {
  Val = Scalar;
  प्रत्यागम StringRef();
}

शून्य ScalarTraits<std::string>::output(ध्रुव std::string &Val, शून्य *,
                                       raw_ostream &Out) {
  Out << Val;
}

StringRef ScalarTraits<std::string>::निवेशः_2(StringRef Scalar, शून्य *,
                                           std::string &Val) {
  Val = Scalar.str();
  प्रत्यागम StringRef();
}

शून्य ScalarTraits<uint8_t>::output(ध्रुव uint8_t &Val, शून्य *,
                                   raw_ostream &Out) {
  // use temp uin32_t because ostream thinks uint8_t is a character
  uint32_t अङ्कः = Val;
  Out << अङ्कः;
}

StringRef ScalarTraits<uint8_t>::निवेशः_2(StringRef Scalar, शून्य *, uint8_t &Val) {
  unsigned long long n;
  जइ (getAsUnsignedInteger(Scalar, 0, n))
    प्रत्यागम "invalid number";
  जइ (n > 0xFF)
    प्रत्यागम "out of range number";
  Val = n;
  प्रत्यागम StringRef();
}

शून्य ScalarTraits<uint16_t>::output(ध्रुव uint16_t &Val, शून्य *,
                                    raw_ostream &Out) {
  Out << Val;
}

StringRef ScalarTraits<uint16_t>::निवेशः_2(StringRef Scalar, शून्य *,
                                        uint16_t &Val) {
  unsigned long long n;
  जइ (getAsUnsignedInteger(Scalar, 0, n))
    प्रत्यागम "invalid number";
  जइ (n > 0xFFFF)
    प्रत्यागम "out of range number";
  Val = n;
  प्रत्यागम StringRef();
}

शून्य ScalarTraits<uint32_t>::output(ध्रुव uint32_t &Val, शून्य *,
                                    raw_ostream &Out) {
  Out << Val;
}

StringRef ScalarTraits<uint32_t>::निवेशः_2(StringRef Scalar, शून्य *,
                                        uint32_t &Val) {
  unsigned long long n;
  जइ (getAsUnsignedInteger(Scalar, 0, n))
    प्रत्यागम "invalid number";
  जइ (n > 0xFFFFFFFFUL)
    प्रत्यागम "out of range number";
  Val = n;
  प्रत्यागम StringRef();
}

शून्य ScalarTraits<uint64_t>::output(ध्रुव uint64_t &Val, शून्य *,
                                    raw_ostream &Out) {
  Out << Val;
}

StringRef ScalarTraits<uint64_t>::निवेशः_2(StringRef Scalar, शून्य *,
                                        uint64_t &Val) {
  unsigned long long N;
  जइ (getAsUnsignedInteger(Scalar, 0, N))
    प्रत्यागम "invalid number";
  Val = N;
  प्रत्यागम StringRef();
}

शून्य ScalarTraits<int8_t>::output(ध्रुव int8_t &Val, शून्य *, raw_ostream &Out) {
  // use temp in32_t because ostream thinks int8_t is a character
  int32_t अङ्कः = Val;
  Out << अङ्कः;
}

StringRef ScalarTraits<int8_t>::निवेशः_2(StringRef Scalar, शून्य *, int8_t &Val) {
  long long N;
  जइ (getAsSignedInteger(Scalar, 0, N))
    प्रत्यागम "invalid number";
  जइ ((N > 127) || (N < -128))
    प्रत्यागम "out of range number";
  Val = N;
  प्रत्यागम StringRef();
}

शून्य ScalarTraits<int16_t>::output(ध्रुव int16_t &Val, शून्य *,
                                   raw_ostream &Out) {
  Out << Val;
}

StringRef ScalarTraits<int16_t>::निवेशः_2(StringRef Scalar, शून्य *, int16_t &Val) {
  long long N;
  जइ (getAsSignedInteger(Scalar, 0, N))
    प्रत्यागम "invalid number";
  जइ ((N > INT16_MAX) || (N < INT16_MIN))
    प्रत्यागम "out of range number";
  Val = N;
  प्रत्यागम StringRef();
}

शून्य ScalarTraits<int32_t>::output(ध्रुव int32_t &Val, शून्य *,
                                   raw_ostream &Out) {
  Out << Val;
}

StringRef ScalarTraits<int32_t>::निवेशः_2(StringRef Scalar, शून्य *, int32_t &Val) {
  long long N;
  जइ (getAsSignedInteger(Scalar, 0, N))
    प्रत्यागम "invalid number";
  जइ ((N > INT32_MAX) || (N < INT32_MIN))
    प्रत्यागम "out of range number";
  Val = N;
  प्रत्यागम StringRef();
}

शून्य ScalarTraits<int64_t>::output(ध्रुव int64_t &Val, शून्य *,
                                   raw_ostream &Out) {
  Out << Val;
}

StringRef ScalarTraits<int64_t>::निवेशः_2(StringRef Scalar, शून्य *, int64_t &Val) {
  long long N;
  जइ (getAsSignedInteger(Scalar, 0, N))
    प्रत्यागम "invalid number";
  Val = N;
  प्रत्यागम StringRef();
}

शून्य ScalarTraits<द्विगुण>::output(ध्रुव द्विगुण &Val, शून्य *, raw_ostream &Out) {
  Out << format("%g", Val);
}

StringRef ScalarTraits<द्विगुण>::निवेशः_2(StringRef Scalar, शून्य *, द्विगुण &Val) {
  जइ (to_float(Scalar, Val))
    प्रत्यागम StringRef();
  प्रत्यागम "invalid floating point number";
}

शून्य ScalarTraits<भिन्न>::output(ध्रुव भिन्न &Val, शून्य *, raw_ostream &Out) {
  Out << format("%g", Val);
}

StringRef ScalarTraits<भिन्न>::निवेशः_2(StringRef Scalar, शून्य *, भिन्न &Val) {
  जइ (to_float(Scalar, Val))
    प्रत्यागम StringRef();
  प्रत्यागम "invalid floating point number";
}

शून्य ScalarTraits<Hex8>::output(ध्रुव Hex8 &Val, शून्य *, raw_ostream &Out) {
  Out << format("0x%" PRIX8, (uint8_t)Val);
}

StringRef ScalarTraits<Hex8>::निवेशः_2(StringRef Scalar, शून्य *, Hex8 &Val) {
  unsigned long long n;
  जइ (getAsUnsignedInteger(Scalar, 0, n))
    प्रत्यागम "invalid hex8 number";
  जइ (n > 0xFF)
    प्रत्यागम "out of range hex8 number";
  Val = n;
  प्रत्यागम StringRef();
}

शून्य ScalarTraits<Hex16>::output(ध्रुव Hex16 &Val, शून्य *, raw_ostream &Out) {
  Out << format("0x%" PRIX16, (uint16_t)Val);
}

StringRef ScalarTraits<Hex16>::निवेशः_2(StringRef Scalar, शून्य *, Hex16 &Val) {
  unsigned long long n;
  जइ (getAsUnsignedInteger(Scalar, 0, n))
    प्रत्यागम "invalid hex16 number";
  जइ (n > 0xFFFF)
    प्रत्यागम "out of range hex16 number";
  Val = n;
  प्रत्यागम StringRef();
}

शून्य ScalarTraits<Hex32>::output(ध्रुव Hex32 &Val, शून्य *, raw_ostream &Out) {
  Out << format("0x%" PRIX32, (uint32_t)Val);
}

StringRef ScalarTraits<Hex32>::निवेशः_2(StringRef Scalar, शून्य *, Hex32 &Val) {
  unsigned long long n;
  जइ (getAsUnsignedInteger(Scalar, 0, n))
    प्रत्यागम "invalid hex32 number";
  जइ (n > 0xFFFFFFFFUL)
    प्रत्यागम "out of range hex32 number";
  Val = n;
  प्रत्यागम StringRef();
}

शून्य ScalarTraits<Hex64>::output(ध्रुव Hex64 &Val, शून्य *, raw_ostream &Out) {
  Out << format("0x%" PRIX64, (uint64_t)Val);
}

StringRef ScalarTraits<Hex64>::निवेशः_2(StringRef Scalar, शून्य *, Hex64 &Val) {
  unsigned long long अङ्कः;
  जइ (getAsUnsignedInteger(Scalar, 0, अङ्कः))
    प्रत्यागम "invalid hex64 number";
  Val = अङ्कः;
  प्रत्यागम StringRef();
}

शून्य ScalarTraits<VersionTuple>::output(ध्रुव VersionTuple &Val, शून्य *,
                                        llvm::raw_ostream &Out) {
  Out << Val.getAsString();
}

StringRef ScalarTraits<VersionTuple>::निवेशः_2(StringRef Scalar, शून्य *,
                                            VersionTuple &Val) {
  जइ (Val.tryParse(Scalar))
    प्रत्यागम "invalid version format";
  प्रत्यागम StringRef();
}
