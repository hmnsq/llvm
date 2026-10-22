//===- lib/Support/YAMLTraits.cpp -----------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#समावेश "llvm/Support/YAMLTraits.h"
#समावेश "llvm/ADT/STLExtras.h"
#समावेश "llvm/ADT/SmallString.h"
#समावेश "llvm/ADT/StringExtras.h"
#समावेश "llvm/ADT/StringRef.h"
#समावेश "llvm/ADT/Twine.h"
#समावेश "llvm/Support/Casting.h"
#समावेश "llvm/Support/Errc.h"
#समावेश "llvm/Support/ErrorHandling.h"
#समावेश "llvm/Support/Format.h"
#समावेश "llvm/Support/LineIterator.h"
#समावेश "llvm/Support/MemoryBuffer.h"
#समावेश "llvm/Support/VersionTuple.h"
#समावेश "llvm/Support/YAMLParser.h"
#समावेश "llvm/Support/raw_ostream.h"
#समावेश <cassert>
#समावेश <cstdint>
#समावेश <cstring>
#समावेश <string>
#समावेश <vector>

using namespace llvm;
using namespace yaml;

//===----------------------------------------------------------------------===//
//  IO
//===----------------------------------------------------------------------===//

IO::IO(व्योम *Context) : Ctxt(Context) {}

IO::~IO() = शेष;

व्योम *IO::getContext() स्थिर {
  वापस Ctxt;
}

व्योम IO::setContext(व्योम *Context) {
  Ctxt = Context;
}

व्योम IO::set_Allow_Unknown_कुंजियाँ(bool Allow) {
  llvm_unreachable("Only supported for Input");
}

//===----------------------------------------------------------------------===//
//निवेश
//===----------------------------------------------------------------------===//

निवेश::निवेश(StringRef निवेश_Content, व्योम *Ctxt,
             SourceMgr::DiagHandlerTy DiagHandler, व्योम *DiagHandlerCtxt)
    : IO(Ctxt), Strm(नया Stream(निवेश_Content, SrcMgr, false, &EC)) {
  अगर (DiagHandler)
    SrcMgr.setDiagHandler(DiagHandler, DiagHandlerCtxt);
  DocIterator = Strm->begin();
}

निवेश::निवेश(Memory_बफर_Ref निवेश, व्योम *Ctxt,
             SourceMgr::DiagHandlerTy DiagHandler, व्योम *DiagHandlerCtxt)
    : IO(Ctxt), Strm(नया Stream(निवेश, SrcMgr, false, &EC)) {
  अगर (DiagHandler)
    SrcMgr.setDiagHandler(DiagHandler, DiagHandlerCtxt);
  DocIterator = Strm->begin();
}

निवेश::~निवेश() = शेष;

std::error_code निवेश::error() { वापस EC; }

bool निवेश::outputting() स्थिर {
  वापस false;
}

bool निवेश::setCurrentDocument() {
  अगर (DocIterator != Strm->end()) {
    गाँठ *N = DocIterator->get_मूल();
    अगर (!N) {
      EC = make_error_code(errc::invalid_argument);
      वापस false;
    }

    अगर (isa<Null_गाँठ>(N)) {
      // Empty files are allowed and ignored
      ++DocIterator;
      वापस setCurrentDocument();
    }
    releaseHNodeBuffers();
    शीर्ष_गाँठ = createHNodes(N);
    Current_गाँठ = शीर्ष_गाँठ;
    वापस true;
  }
  वापस false;
}

bool निवेश::अग्र_Document() {
  वापस ++DocIterator != Strm->end();
}

स्थिर गाँठ *निवेश::get_Current_गाँठ() स्थिर {
  वापस Current_गाँठ ? Current_गाँठ->गाँठ_2 : nullptr;
}

bool निवेश::mapTag(StringRef Tag, bool Default) {
  // CurrentNode can be null if setCurrentDocument() was unable to
  // parse the document because it was invalid or empty.
  अगर (!Current_गाँठ)
    वापस false;

  std::string foundTag = Current_गाँठ->गाँठ_2->getVerbatimTag();
  अगर (foundTag.empty()) {
    // If no tag found and 'Tag' is the default, say it was found.
    वापस Default;
  }
  // Return true iff found tag matches supplied tag.
  वापस Tag == foundTag;
}

व्योम निवेश::beginMapping() {
  अगर (EC)
    वापस;
  // CurrentNode can be null if the document is empty.
  MapHNode *MN = dyn_cast_or_null<MapHNode>(Current_गाँठ);
  अगर (MN) {
    MN->Valid_कुंजियाँ.सफा();
  }
}

std::vector<StringRef> निवेश::कुंजियाँ() {
  MapHNode *MN = dyn_cast<MapHNode>(Current_गाँठ);
  std::vector<StringRef> Ret;
  अगर (!MN) {
    setError(Current_गाँठ, "not a mapping");
    वापस Ret;
  }
  क्रम (स्वतः &P : MN->Mapping)
    Ret.धकेलो_back(P.first());
  वापस Ret;
}

bool निवेश::preflight_कुंजी(StringRef कुंजी, bool Required, bool, bool &UseDefault,
                         व्योम *&SaveInfo) {
  UseDefault = false;
  अगर (EC)
    वापस false;

  // CurrentNode is null for empty documents, which is an error in case required
  // nodes are present.
  अगर (!Current_गाँठ) {
    अगर (Required)
      EC = make_error_code(errc::invalid_argument);
    अन्यथा
      UseDefault = true;
    वापस false;
  }

  MapHNode *MN = dyn_cast<MapHNode>(Current_गाँठ);
  अगर (!MN) {
    अगर (Required || !isa<EmptyHNode>(Current_गाँठ))
      setError(Current_गाँठ, "not a mapping");
    अन्यथा
      UseDefault = true;
    वापस false;
  }
  MN->Valid_कुंजियाँ.धकेलो_back(कुंजी.str());
  HNode *मूल्य = MN->Mapping[कुंजी].first;
  अगर (!मूल्य) {
    अगर (Required)
      setError(Current_गाँठ, Twine("missing required key '") + कुंजी + "'");
    अन्यथा
      UseDefault = true;
    वापस false;
  }
  SaveInfo = Current_गाँठ;
  Current_गाँठ = मूल्य;
  वापस true;
}

व्योम निवेश::postflight_कुंजी(व्योम *saveInfo) {
  Current_गाँठ = reinterpret_cast<HNode *>(saveInfo);
}

व्योम निवेश::endMapping() {
  अगर (EC)
    वापस;
  // CurrentNode can be null if the document is empty.
  MapHNode *MN = dyn_cast_or_null<MapHNode>(Current_गाँठ);
  अगर (!MN)
    वापस;
  क्रम (स्थिर स्वतः &NN : MN->Mapping) {
    अगर (!is_contained(MN->Valid_कुंजियाँ, NN.first())) {
      स्थिर SMRange &ReportLoc = NN.second.second;
      अगर (!Allow_Unknown_कुंजियाँ) {
        setError(ReportLoc, Twine("unknown key '") + NN.first() + "'");
        अवरोध;
      } अन्यथा
        reportWarning(ReportLoc, Twine("unknown key '") + NN.first() + "'");
    }
  }
}

व्योम निवेश::beginFlowMapping() { beginMapping(); }

व्योम निवेश::endFlowMapping() { endMapping(); }

अचिन्हित निवेश::beginSequence() {
  अगर (SequenceHNode *SQ = dyn_cast<SequenceHNode>(Current_गाँठ))
    वापस SQ->Entries.आकार_2();
  अगर (isa<EmptyHNode>(Current_गाँठ))
    वापस 0;
  // Treat case where there's a scalar "null" value as an empty sequence.
  अगर (ScalarHNode *SN = dyn_cast<ScalarHNode>(Current_गाँठ)) {
    अगर (isNull(SN->मूल्य_2()))
      वापस 0;
  }
  // Any other type of HNode is an error.
  setError(Current_गाँठ, "not a sequence");
  वापस 0;
}

व्योम निवेश::endSequence() {
}

bool निवेश::preflightElement(अचिन्हित सूचकांक_2, व्योम *&SaveInfo) {
  अगर (EC)
    वापस false;
  अगर (SequenceHNode *SQ = dyn_cast<SequenceHNode>(Current_गाँठ)) {
    SaveInfo = Current_गाँठ;
    Current_गाँठ = SQ->Entries[सूचकांक_2];
    वापस true;
  }
  वापस false;
}

व्योम निवेश::postflightElement(व्योम *SaveInfo) {
  Current_गाँठ = reinterpret_cast<HNode *>(SaveInfo);
}

अचिन्हित निवेश::beginFlowSequence() { वापस beginSequence(); }

bool निवेश::preflightFlowElement(अचिन्हित सूचकांक, व्योम *&SaveInfo) {
  अगर (EC)
    वापस false;
  अगर (SequenceHNode *SQ = dyn_cast<SequenceHNode>(Current_गाँठ)) {
    SaveInfo = Current_गाँठ;
    Current_गाँठ = SQ->Entries[सूचकांक];
    वापस true;
  }
  वापस false;
}

व्योम निवेश::postflightFlowElement(व्योम *SaveInfo) {
  Current_गाँठ = reinterpret_cast<HNode *>(SaveInfo);
}

व्योम निवेश::endFlowSequence() {
}

व्योम निवेश::beginEnumScalar() {
  ScalarMatchFound = false;
}

bool निवेश::matchEnumScalar(StringRef Str, bool) {
  अगर (ScalarMatchFound)
    वापस false;
  अगर (ScalarHNode *SN = dyn_cast<ScalarHNode>(Current_गाँठ)) {
    अगर (SN->मूल्य_2() == Str) {
      ScalarMatchFound = true;
      वापस true;
    }
  }
  वापस false;
}

bool निवेश::matchEnumFallback() {
  अगर (ScalarMatchFound)
    वापस false;
  ScalarMatchFound = true;
  वापस true;
}

व्योम निवेश::endEnumScalar() {
  अगर (!ScalarMatchFound) {
    setError(Current_गाँठ, "unknown enumerated scalar");
  }
}

bool निवेश::beginBitSetScalar(bool &DoClear) {
  Bit_मूल्यावली_Used.सफा();
  अगर (SequenceHNode *SQ = dyn_cast<SequenceHNode>(Current_गाँठ)) {
    Bit_मूल्यावली_Used.resize(SQ->Entries.आकार_2());
  } अन्यथा {
    setError(Current_गाँठ, "expected sequence of bit values");
  }
  DoClear = true;
  वापस true;
}

bool निवेश::bitSetMatch(StringRef Str, bool) {
  अगर (EC)
    वापस false;
  अगर (SequenceHNode *SQ = dyn_cast<SequenceHNode>(Current_गाँठ)) {
    अचिन्हित सूचकांक_2 = 0;
    क्रम (स्वतः &N : SQ->Entries) {
      अगर (ScalarHNode *SN = dyn_cast<ScalarHNode>(N)) {
        अगर (SN->मूल्य_2() == Str) {
          Bit_मूल्यावली_Used[सूचकांक_2] = true;
          वापस true;
        }
      } अन्यथा {
        setError(Current_गाँठ, "unexpected scalar in sequence of bit values");
      }
      ++सूचकांक_2;
    }
  } अन्यथा {
    setError(Current_गाँठ, "expected sequence of bit values");
  }
  वापस false;
}

व्योम निवेश::endBitSetScalar() {
  अगर (EC)
    वापस;
  अगर (SequenceHNode *SQ = dyn_cast<SequenceHNode>(Current_गाँठ)) {
    निश्चित(Bit_मूल्यावली_Used.आकार_2() == SQ->Entries.आकार_2());
    क्रम (अचिन्हित i = 0; i < SQ->Entries.आकार_2(); ++i) {
      अगर (!Bit_मूल्यावली_Used[i]) {
        setError(SQ->Entries[i], "unknown bit value");
        वापस;
      }
    }
  }
}

व्योम निवेश::scalarString(StringRef &S, QuotingType) {
  अगर (ScalarHNode *SN = dyn_cast<ScalarHNode>(Current_गाँठ)) {
    S = SN->मूल्य_2();
  } अन्यथा {
    setError(Current_गाँठ, "unexpected scalar");
  }
}

व्योम निवेश::blockScalarString(StringRef &S) { scalarString(S, QuotingType::None); }

व्योम निवेश::scalarTag(std::string &Tag) {
  Tag = Current_गाँठ->गाँठ_2->getVerbatimTag();
}

व्योम निवेश::setError(HNode *hnode, स्थिर Twine &message) {
  निश्चित(hnode && "HNode must not be NULL");
  setError(hnode->गाँठ_2, message);
}

गाँठ_Kind निवेश::get_गाँठ_Kind() {
  अगर (isa<ScalarHNode>(Current_गाँठ))
    वापस गाँठ_Kind::Scalar;
  अन्यथा अगर (isa<MapHNode>(Current_गाँठ))
    वापस गाँठ_Kind::Map;
  अन्यथा अगर (isa<SequenceHNode>(Current_गाँठ))
    वापस गाँठ_Kind::Sequence;
  llvm_unreachable("Unsupported node kind");
}

व्योम निवेश::setError(गाँठ *गाँठ_3, स्थिर Twine &message) {
  Strm->printError(गाँठ_3, message);
  EC = make_error_code(errc::invalid_argument);
}

व्योम निवेश::setError(स्थिर SMRange &range, स्थिर Twine &message) {
  Strm->printError(range, message);
  EC = make_error_code(errc::invalid_argument);
}

व्योम निवेश::reportWarning(HNode *hnode, स्थिर Twine &message) {
  निश्चित(hnode && "HNode must not be NULL");
  Strm->printError(hnode->गाँठ_2, message, SourceMgr::DK_Warning);
}

व्योम निवेश::reportWarning(गाँठ *गाँठ_3, स्थिर Twine &message) {
  Strm->printError(गाँठ_3, message, SourceMgr::DK_Warning);
}

व्योम निवेश::reportWarning(स्थिर SMRange &range, स्थिर Twine &message) {
  Strm->printError(range, message, SourceMgr::DK_Warning);
}

व्योम निवेश::releaseHNodeBuffers() {
  EmptyHNodeAllocator.DestroyAll();
  ScalarHNodeAllocator.DestroyAll();
  SequenceHNodeAllocator.DestroyAll();
  MapHNodeAllocator.DestroyAll();
}

व्योम निवेश::saveAliasHNode(गाँठ *N, HNode *HN) {
  StringRef Anchor = N->getAnchor();
  अगर (!Anchor.empty())
    // YAML 1.2.2 - 3.2.2.2. Anchors and Aliases:
    //
    // An alias event refers to the most recent event in the serialization
    // having the specified anchor. Therefore, anchors need not be unique within
    // a serialization. In addition, an anchor need not have an alias node
    // referring to it.
    AliasMap[Anchor] = HN;
}

निवेश::HNode *निवेश::createHNodes(गाँठ *N) {
  SmallString<128> StringStorage;
  चयन (N->getType()) {
  हाल गाँठ::NK_Scalar: {
    Scalar_गाँठ *SN = dyn_cast<Scalar_गाँठ>(N);
    StringRef कुंजी_Str = SN->get_मूल्य(StringStorage);
    अगर (!StringStorage.empty()) {
      // Copy string to permanent storage
      कुंजी_Str = StringStorage.str().copy(StringAllocator);
    }
    स्वतः *SHNode = नया (ScalarHNodeAllocator.Allocate()) ScalarHNode(N, कुंजी_Str);
    saveAliasHNode(SN, SHNode);
    वापस SHNode;
  }
  हाल गाँठ::NK_BlockScalar: {
    Block_Scalar_गाँठ *BSN = dyn_cast<Block_Scalar_गाँठ>(N);
    StringRef मूल्य_Copy = BSN->get_मूल्य().copy(StringAllocator);
    स्वतः *BSHNode =
        नया (ScalarHNodeAllocator.Allocate()) ScalarHNode(N, मूल्य_Copy);
    saveAliasHNode(BSN, BSHNode);
    वापस BSHNode;
  }
  हाल गाँठ::NK_Sequence: {
    Sequence_गाँठ *SQ = dyn_cast<Sequence_गाँठ>(N);
    स्वतः SQHNode = नया (SequenceHNodeAllocator.Allocate()) SequenceHNode(N);
    क्रम (गाँठ &SN : *SQ) {
      स्वतः Entry = createHNodes(&SN);
      अगर (EC)
        अवरोध;
      SQHNode->Entries.धकेलो_back(Entry);
    }
    saveAliasHNode(SQ, SQHNode);
    वापस SQHNode;
  }
  हाल गाँठ::NK_Mapping: {
    Mapping_गाँठ *Map = dyn_cast<Mapping_गाँठ>(N);
    स्वतः mapHNode = नया (MapHNodeAllocator.Allocate()) MapHNode(N);
    क्रम (कुंजी_मूल्य_गाँठ &KVN : *Map) {
      गाँठ *कुंजी_गाँठ = KVN.get_कुंजी();
      Scalar_गाँठ *कुंजी = dyn_cast_if_present<Scalar_गाँठ>(कुंजी_गाँठ);
      गाँठ *मूल्य = KVN.get_मूल्य();
      अगर (!कुंजी || !मूल्य) {
        अगर (!कुंजी)
          setError(कुंजी_गाँठ, "Map key must be a scalar");
        अगर (!मूल्य)
          setError(कुंजी_गाँठ, "Map value must not be empty");
        अवरोध;
      }
      StringStorage.सफा();
      StringRef कुंजी_Str = कुंजी->get_मूल्य(StringStorage);
      अगर (!StringStorage.empty()) {
        // Copy string to permanent storage
        कुंजी_Str = StringStorage.str().copy(StringAllocator);
      }
      अगर (mapHNode->Mapping.गिनती(कुंजी_Str))
        // From YAML spec: "The content of a mapping node is an unordered set of
        // key/value node pairs, with the restriction that each of the keys is
        // unique."
        setError(कुंजी_गाँठ, Twine("duplicated mapping key '") + कुंजी_Str + "'");
      स्वतः मूल्य_HNode = createHNodes(मूल्य);
      अगर (EC)
        अवरोध;
      mapHNode->Mapping[कुंजी_Str] =
          std::make_pair(std::move(मूल्य_HNode), कुंजी_गाँठ->getSourceRange());
    }
    saveAliasHNode(Map, mapHNode);
    वापस std::move(mapHNode);
  }
  हाल गाँठ::NK_Null:
    // TODO: Anchor is not set for NullNode in the parser. Update the parser to
    // faithfully preserve anchors.
    वापस नया (EmptyHNodeAllocator.Allocate()) EmptyHNode(N);
  हाल गाँठ::NK_Alias: {
    Alias_गाँठ *AN = dyn_cast<Alias_गाँठ>(N);
    स्वतः AliasName = AN->getName();
    स्वतः AHN = AliasMap.find(AliasName);
    अगर (AHN == AliasMap.end()) {
      setError(AN, Twine("undefined alias '" + AliasName + "'"));
      वापस nullptr;
    }
    वापस AHN->second;
  }
  शेष:
    setError(N, "unknown node kind");
    वापस nullptr;
  }
}

व्योम निवेश::setError(स्थिर Twine &Message) {
  setError(Current_गाँठ, Message);
}

व्योम निवेश::set_Allow_Unknown_कुंजियाँ(bool Allow) { Allow_Unknown_कुंजियाँ = Allow; }

bool निवेश::canElideEmptySequence() {
  वापस false;
}

//===----------------------------------------------------------------------===//
//  Output
//===----------------------------------------------------------------------===//

Output::Output(raw_ostream &yout, व्योम *context, पूर्णांक WrapColumn)
    : IO(context), Out(yout), WrapColumn(WrapColumn) {}

Output::~Output() = शेष;

bool Output::outputting() स्थिर {
  वापस true;
}

व्योम Output::beginMapping() {
  State_ढेर.धकेलो_back(in_Map_First_कुंजी);
  PaddingBeforeContainer = Padding;
  Padding = "\n";
}

bool Output::mapTag(StringRef Tag, bool Use) {
  अगर (Use) {
    // If this tag is being written inside a sequence we should write the start
    // of the sequence before writing the tag, otherwise the tag won't be
    // attached to the element in the sequence, but rather the sequence itself.
    bool SequenceElement = false;
    अगर (State_ढेर.आकार_2() > 1) {
      स्वतः &E = State_ढेर[State_ढेर.आकार_2() - 2];
      SequenceElement = inSeqAnyElement(E) || inFlowSeqAnyElement(E);
    }
    अगर (SequenceElement && State_ढेर.back() == in_Map_First_कुंजी) {
      new_पंक्ति_Check();
    } अन्यथा {
      output(" ");
    }
    output(Tag);
    अगर (SequenceElement) {
      // If we're writing the tag during the first element of a map, the tag
      // takes the place of the first element in the sequence.
      अगर (State_ढेर.back() == in_Map_First_कुंजी) {
        State_ढेर.निकालो_back();
        State_ढेर.धकेलो_back(in_Map_Other_कुंजी);
      }
      // Tags inside maps in sequences should act as keys in the map from a
      // formatting perspective, so we always want a newline in a sequence.
      Padding = "\n";
    }
  }
  वापस Use;
}

व्योम Output::endMapping() {
  // If we did not map anything, we should explicitly emit an empty map
  अगर (State_ढेर.back() == in_Map_First_कुंजी) {
    Padding = PaddingBeforeContainer;
    new_पंक्ति_Check();
    output("{}");
    Padding = "\n";
  }
  State_ढेर.निकालो_back();
}

std::vector<StringRef> Output::कुंजियाँ() {
  report_fatal_error("invalid call");
}

bool Output::preflight_कुंजी(StringRef कुंजी, bool Required, bool SameAsDefault,
                          bool &UseDefault, व्योम *&SaveInfo) {
  UseDefault = false;
  SaveInfo = nullptr;
  अगर (Required || !SameAsDefault || Write_Default_मूल्यावली) {
    स्वतः State = State_ढेर.back();
    अगर (State == in_Flow_Map_First_कुंजी || State == in_Flow_Map_Other_कुंजी) {
      flow_कुंजी(कुंजी);
    } अन्यथा {
      new_पंक्ति_Check();
      padded_कुंजी(कुंजी);
    }
    वापस true;
  }
  वापस false;
}

व्योम Output::postflight_कुंजी(व्योम *) {
  अगर (State_ढेर.back() == in_Map_First_कुंजी) {
    State_ढेर.निकालो_back();
    State_ढेर.धकेलो_back(in_Map_Other_कुंजी);
  } अन्यथा अगर (State_ढेर.back() == in_Flow_Map_First_कुंजी) {
    State_ढेर.निकालो_back();
    State_ढेर.धकेलो_back(in_Flow_Map_Other_कुंजी);
  }
}

व्योम Output::beginFlowMapping() {
  State_ढेर.धकेलो_back(in_Flow_Map_First_कुंजी);
  new_पंक्ति_Check();
  ColumnAtMapFlowStart = Column;
  output("{ ");
}

व्योम Output::endFlowMapping() {
  State_ढेर.निकालो_back();
  output_Up_To_End_Of_पंक्ति(" }");
}

व्योम Output::beginDocuments() {
  output_Up_To_End_Of_पंक्ति("---");
}

bool Output::preflightDocument(अचिन्हित सूचकांक) {
  अगर (सूचकांक > 0)
    output_Up_To_End_Of_पंक्ति("\n---");
  वापस true;
}

व्योम Output::postflightDocument() {
}

व्योम Output::endDocuments() {
  output("\n...\n");
}

अचिन्हित Output::beginSequence() {
  State_ढेर.धकेलो_back(inSeqFirstElement);
  PaddingBeforeContainer = Padding;
  Padding = "\n";
  वापस 0;
}

व्योम Output::endSequence() {
  // If we did not emit anything, we should explicitly emit an empty sequence
  अगर (State_ढेर.back() == inSeqFirstElement) {
    Padding = PaddingBeforeContainer;
    new_पंक्ति_Check(/*EmptySequence=*/true);
    output("[]");
    Padding = "\n";
  }
  State_ढेर.निकालो_back();
}

bool Output::preflightElement(अचिन्हित, व्योम *&SaveInfo) {
  SaveInfo = nullptr;
  वापस true;
}

व्योम Output::postflightElement(व्योम *) {
  अगर (State_ढेर.back() == inSeqFirstElement) {
    State_ढेर.निकालो_back();
    State_ढेर.धकेलो_back(inSeqOtherElement);
  } अन्यथा अगर (State_ढेर.back() == inFlowSeqFirstElement) {
    State_ढेर.निकालो_back();
    State_ढेर.धकेलो_back(inFlowSeqOtherElement);
  }
}

अचिन्हित Output::beginFlowSequence() {
  State_ढेर.धकेलो_back(inFlowSeqFirstElement);
  new_पंक्ति_Check();
  ColumnAtFlowStart = Column;
  output("[ ");
  NeedFlowSequenceComma = false;
  वापस 0;
}

व्योम Output::endFlowSequence() {
  State_ढेर.निकालो_back();
  output_Up_To_End_Of_पंक्ति(" ]");
}

bool Output::preflightFlowElement(अचिन्हित, व्योम *&SaveInfo) {
  अगर (NeedFlowSequenceComma)
    output(", ");
  अगर (WrapColumn && Column > WrapColumn) {
    output("\n");
    क्रम (पूर्णांक i = 0; i < ColumnAtFlowStart; ++i)
      output(" ");
    Column = ColumnAtFlowStart;
    output("  ");
  }
  SaveInfo = nullptr;
  वापस true;
}

व्योम Output::postflightFlowElement(व्योम *) {
  NeedFlowSequenceComma = true;
}

व्योम Output::beginEnumScalar() {
  EnumerationMatchFound = false;
}

bool Output::matchEnumScalar(StringRef Str, bool Match) {
  अगर (Match && !EnumerationMatchFound) {
    new_पंक्ति_Check();
    output_Up_To_End_Of_पंक्ति(Str);
    EnumerationMatchFound = true;
  }
  वापस false;
}

bool Output::matchEnumFallback() {
  अगर (EnumerationMatchFound)
    वापस false;
  EnumerationMatchFound = true;
  वापस true;
}

व्योम Output::endEnumScalar() {
  अगर (!EnumerationMatchFound)
    llvm_unreachable("bad runtime enum value");
}

bool Output::beginBitSetScalar(bool &DoClear) {
  new_पंक्ति_Check();
  output("[ ");
  Need_Bit_मूल्य_Comma = false;
  DoClear = false;
  वापस true;
}

bool Output::bitSetMatch(StringRef Str, bool Matches) {
  अगर (Matches) {
    अगर (Need_Bit_मूल्य_Comma)
      output(", ");
    output(Str);
    Need_Bit_मूल्य_Comma = true;
  }
  वापस false;
}

व्योम Output::endBitSetScalar() {
  output_Up_To_End_Of_पंक्ति(" ]");
}

व्योम Output::scalarString(StringRef &S, QuotingType MustQuote) {
  new_पंक्ति_Check();
  अगर (S.empty()) {
    // Print '' for the empty string because leaving the field empty is not
    // allowed.
    output_Up_To_End_Of_पंक्ति("''");
    वापस;
  }
  output(S, MustQuote);
  output_Up_To_End_Of_पंक्ति("");
}

व्योम Output::blockScalarString(StringRef &S) {
  अगर (!State_ढेर.empty())
    new_पंक्ति_Check();
  output(" |");

  अचिन्हित Indent = State_ढेर.empty() ? 1 : State_ढेर.आकार_2();

  स्वतः बफर = Memory_बफर::get_Mem_बफर(S, "", false);
  क्रम (पंक्ति_iterator पंक्तियाँ(*बफर, false); !पंक्तियाँ.is_at_end(); ++पंक्तियाँ) {
    output_New_पंक्ति();
    क्रम (अचिन्हित I = 0; I < Indent; ++I) {
      output("  ");
    }
    output(*पंक्तियाँ);
  }
  output_Up_To_End_Of_पंक्ति("");
}

व्योम Output::scalarTag(std::string &Tag) {
  अगर (Tag.empty())
    वापस;
  new_पंक्ति_Check();
  output(Tag);
  output(" ");
}

व्योम Output::setError(स्थिर Twine &message) {
}

std::error_code Output::error() { वापस {}; }

bool Output::canElideEmptySequence() {
  // Normally, with an optional key/value where the value is an empty sequence,
  // the whole key/value can be not written.  But, that produces wrong yaml
  // if the key/value is the only thing in the map and the map is used in
  // a sequence.  This detects if the this sequence is the first key/value
  // in map that itself is embedded in a sequence.
  अगर (State_ढेर.आकार_2() < 2)
    वापस true;
  अगर (State_ढेर.back() != in_Map_First_कुंजी)
    वापस true;
  वापस !inSeqAnyElement(State_ढेर[State_ढेर.आकार_2() - 2]);
}

व्योम Output::output(StringRef s) {
  Column += s.आकार_2();
  Out << s;
}

व्योम Output::output(StringRef S, QuotingType MustQuote) {
  अगर (MustQuote == QuotingType::None) {
    // Only quote if we must.
    output(S);
    वापस;
  }

  StringLiteral Quote = MustQuote == QuotingType::Single ? StringLiteral("'")
                                                         : StringLiteral("\"");
  output(Quote); // Starting quote.

  // When using double-quoted strings (and only in that case), non-printable
  // characters may be present, and will be escaped using a variety of
  // unicode-scalar and special short-form escapes. This is handled in
  // yaml::escape.
  अगर (MustQuote == QuotingType::Double) {
    output(yaml::escape(S, /* EscapePrintable= */ false));
    output(Quote);
    वापस;
  }

  अचिन्हित i = 0;
  अचिन्हित j = 0;
  अचिन्हित End = S.आकार_2();
  स्थिर अक्षर *Base = S.तथ्य();

  // When using single-quoted strings, any single quote ' must be doubled to be
  // escaped.
  जबतक (j < End) {
    अगर (S[j] == '\'') {                   // Escape quotes.
      output(StringRef(&Base[i], j - i)); // "flush".
      output(StringLiteral("''"));        // Print it as ''
      i = j + 1;
    }
    ++j;
  }
  output(StringRef(&Base[i], j - i));
  output(Quote); // Ending quote.
}

व्योम Output::output_Up_To_End_Of_पंक्ति(StringRef s) {
  output(s);
  अगर (State_ढेर.empty() || (!inFlowSeqAnyElement(State_ढेर.back()) &&
                             !in_Flow_Map_Any_कुंजी(State_ढेर.back())))
    Padding = "\n";
}

व्योम Output::output_New_पंक्ति() {
  Out << "\n";
  Column = 0;
}

// if seq at top, indent as if map, then add "- "
// if seq in middle, use "- " if firstKey, else use "  "
//

व्योम Output::new_पंक्ति_Check(bool EmptySequence) {
  अगर (Padding != "\n") {
    output(Padding);
    Padding = {};
    वापस;
  }
  output_New_पंक्ति();
  Padding = {};

  अगर (State_ढेर.आकार_2() == 0 || EmptySequence)
    वापस;

  अचिन्हित Indent = State_ढेर.आकार_2() - 1;
  bool PossiblyNestedSeq = false;
  स्वतः I = State_ढेर.rbegin(), E = State_ढेर.rend();

  अगर (inSeqAnyElement(*I)) {
    PossiblyNestedSeq = true; // Not possibly but always.
    ++Indent;
  } अन्यथा अगर (*I == in_Map_First_कुंजी || *I == in_Flow_Map_First_कुंजी ||
             inFlowSeqAnyElement(*I)) {
    PossiblyNestedSeq = true;
    ++I; // Skip back().
  }

  अचिन्हित Output_Dash_गिनती = 0;
  अगर (PossiblyNestedSeq) {
    // Count up consecutive inSeqFirstElement from the end, unless
    // inSeqFirstElement is the top of nested sequence.
    जबतक (I != E) {
      // Don't count the top of nested sequence.
      अगर (!inSeqAnyElement(*I))
        अवरोध;

      ++Output_Dash_गिनती;

      // Stop counting if consecutive inSeqFirstElement ends.
      अगर (*I++ != inSeqFirstElement)
        अवरोध;
    }
  }

  क्रम (अचिन्हित I = Output_Dash_गिनती; I < Indent; ++I)
    output("  ");

  क्रम (अचिन्हित I = 0; I < Output_Dash_गिनती; ++I)
    output("- ");
}

व्योम Output::padded_कुंजी(StringRef कुंजी_2) {
  output(कुंजी_2, needsQuotes(कुंजी_2, false));
  output(":");
  स्थिर अक्षर *spaces = "                ";
  अगर (कुंजी_2.आकार_2() < म_माप(spaces))
    Padding = &spaces[कुंजी_2.आकार_2()];
  अन्यथा
    Padding = " ";
}

व्योम Output::flow_कुंजी(StringRef कुंजी) {
  अगर (State_ढेर.back() == in_Flow_Map_Other_कुंजी)
    output(", ");
  अगर (WrapColumn && Column > WrapColumn) {
    output("\n");
    क्रम (पूर्णांक I = 0; I < ColumnAtMapFlowStart; ++I)
      output(" ");
    Column = ColumnAtMapFlowStart;
    output("  ");
  }
  output(कुंजी, needsQuotes(कुंजी, false));
  output(": ");
}

गाँठ_Kind Output::get_गाँठ_Kind() { report_fatal_error("invalid call"); }

bool Output::inSeqAnyElement(InState State) {
  वापस State == inSeqFirstElement || State == inSeqOtherElement;
}

bool Output::inFlowSeqAnyElement(InState State) {
  वापस State == inFlowSeqFirstElement || State == inFlowSeqOtherElement;
}

bool Output::in_Map_Any_कुंजी(InState State) {
  वापस State == in_Map_First_कुंजी || State == in_Map_Other_कुंजी;
}

bool Output::in_Flow_Map_Any_कुंजी(InState State) {
  वापस State == in_Flow_Map_First_कुंजी || State == in_Flow_Map_Other_कुंजी;
}

//===----------------------------------------------------------------------===//
//  traits for built-in types
//===----------------------------------------------------------------------===//

व्योम ScalarTraits<bool>::output(स्थिर bool &Val, व्योम *, raw_ostream &Out) {
  Out << (Val ? "true" : "false");
}

StringRef ScalarTraits<bool>::निवेश_2(StringRef Scalar, व्योम *, bool &Val) {
  अगर (std::optional<bool> Parsed = parseBool(Scalar)) {
    Val = *Parsed;
    वापस StringRef();
  }
  वापस "invalid boolean";
}

व्योम ScalarTraits<StringRef>::output(स्थिर StringRef &Val, व्योम *,
                                     raw_ostream &Out) {
  Out << Val;
}

StringRef ScalarTraits<StringRef>::निवेश_2(StringRef Scalar, व्योम *,
                                         StringRef &Val) {
  Val = Scalar;
  वापस StringRef();
}

व्योम ScalarTraits<std::string>::output(स्थिर std::string &Val, व्योम *,
                                       raw_ostream &Out) {
  Out << Val;
}

StringRef ScalarTraits<std::string>::निवेश_2(StringRef Scalar, व्योम *,
                                           std::string &Val) {
  Val = Scalar.str();
  वापस StringRef();
}

व्योम ScalarTraits<uint8_t>::output(स्थिर uint8_t &Val, व्योम *,
                                   raw_ostream &Out) {
  // use temp uin32_t because ostream thinks uint8_t is a character
  uint32_t अंक = Val;
  Out << अंक;
}

StringRef ScalarTraits<uint8_t>::निवेश_2(StringRef Scalar, व्योम *, uint8_t &Val) {
  अचिन्हित दीर्घ दीर्घ n;
  अगर (getAsUnsignedInteger(Scalar, 0, n))
    वापस "invalid number";
  अगर (n > 0xFF)
    वापस "out of range number";
  Val = n;
  वापस StringRef();
}

व्योम ScalarTraits<uint16_t>::output(स्थिर uint16_t &Val, व्योम *,
                                    raw_ostream &Out) {
  Out << Val;
}

StringRef ScalarTraits<uint16_t>::निवेश_2(StringRef Scalar, व्योम *,
                                        uint16_t &Val) {
  अचिन्हित दीर्घ दीर्घ n;
  अगर (getAsUnsignedInteger(Scalar, 0, n))
    वापस "invalid number";
  अगर (n > 0xFFFF)
    वापस "out of range number";
  Val = n;
  वापस StringRef();
}

व्योम ScalarTraits<uint32_t>::output(स्थिर uint32_t &Val, व्योम *,
                                    raw_ostream &Out) {
  Out << Val;
}

StringRef ScalarTraits<uint32_t>::निवेश_2(StringRef Scalar, व्योम *,
                                        uint32_t &Val) {
  अचिन्हित दीर्घ दीर्घ n;
  अगर (getAsUnsignedInteger(Scalar, 0, n))
    वापस "invalid number";
  अगर (n > 0xFFFFFFFFUL)
    वापस "out of range number";
  Val = n;
  वापस StringRef();
}

व्योम ScalarTraits<uint64_t>::output(स्थिर uint64_t &Val, व्योम *,
                                    raw_ostream &Out) {
  Out << Val;
}

StringRef ScalarTraits<uint64_t>::निवेश_2(StringRef Scalar, व्योम *,
                                        uint64_t &Val) {
  अचिन्हित दीर्घ दीर्घ N;
  अगर (getAsUnsignedInteger(Scalar, 0, N))
    वापस "invalid number";
  Val = N;
  वापस StringRef();
}

व्योम ScalarTraits<int8_t>::output(स्थिर int8_t &Val, व्योम *, raw_ostream &Out) {
  // use temp in32_t because ostream thinks int8_t is a character
  int32_t अंक = Val;
  Out << अंक;
}

StringRef ScalarTraits<int8_t>::निवेश_2(StringRef Scalar, व्योम *, int8_t &Val) {
  दीर्घ दीर्घ N;
  अगर (getAsSignedInteger(Scalar, 0, N))
    वापस "invalid number";
  अगर ((N > 127) || (N < -128))
    वापस "out of range number";
  Val = N;
  वापस StringRef();
}

व्योम ScalarTraits<int16_t>::output(स्थिर int16_t &Val, व्योम *,
                                   raw_ostream &Out) {
  Out << Val;
}

StringRef ScalarTraits<int16_t>::निवेश_2(StringRef Scalar, व्योम *, int16_t &Val) {
  दीर्घ दीर्घ N;
  अगर (getAsSignedInteger(Scalar, 0, N))
    वापस "invalid number";
  अगर ((N > INT16_MAX) || (N < INT16_MIN))
    वापस "out of range number";
  Val = N;
  वापस StringRef();
}

व्योम ScalarTraits<int32_t>::output(स्थिर int32_t &Val, व्योम *,
                                   raw_ostream &Out) {
  Out << Val;
}

StringRef ScalarTraits<int32_t>::निवेश_2(StringRef Scalar, व्योम *, int32_t &Val) {
  दीर्घ दीर्घ N;
  अगर (getAsSignedInteger(Scalar, 0, N))
    वापस "invalid number";
  अगर ((N > INT32_MAX) || (N < INT32_MIN))
    वापस "out of range number";
  Val = N;
  वापस StringRef();
}

व्योम ScalarTraits<int64_t>::output(स्थिर int64_t &Val, व्योम *,
                                   raw_ostream &Out) {
  Out << Val;
}

StringRef ScalarTraits<int64_t>::निवेश_2(StringRef Scalar, व्योम *, int64_t &Val) {
  दीर्घ दीर्घ N;
  अगर (getAsSignedInteger(Scalar, 0, N))
    वापस "invalid number";
  Val = N;
  वापस StringRef();
}

व्योम ScalarTraits<द्विगुन>::output(स्थिर द्विगुन &Val, व्योम *, raw_ostream &Out) {
  Out << format("%g", Val);
}

StringRef ScalarTraits<द्विगुन>::निवेश_2(StringRef Scalar, व्योम *, द्विगुन &Val) {
  अगर (to_float(Scalar, Val))
    वापस StringRef();
  वापस "invalid floating point number";
}

व्योम ScalarTraits<भग्न>::output(स्थिर भग्न &Val, व्योम *, raw_ostream &Out) {
  Out << format("%g", Val);
}

StringRef ScalarTraits<भग्न>::निवेश_2(StringRef Scalar, व्योम *, भग्न &Val) {
  अगर (to_float(Scalar, Val))
    वापस StringRef();
  वापस "invalid floating point number";
}

व्योम ScalarTraits<Hex8>::output(स्थिर Hex8 &Val, व्योम *, raw_ostream &Out) {
  Out << format("0x%" PRIX8, (uint8_t)Val);
}

StringRef ScalarTraits<Hex8>::निवेश_2(StringRef Scalar, व्योम *, Hex8 &Val) {
  अचिन्हित दीर्घ दीर्घ n;
  अगर (getAsUnsignedInteger(Scalar, 0, n))
    वापस "invalid hex8 number";
  अगर (n > 0xFF)
    वापस "out of range hex8 number";
  Val = n;
  वापस StringRef();
}

व्योम ScalarTraits<Hex16>::output(स्थिर Hex16 &Val, व्योम *, raw_ostream &Out) {
  Out << format("0x%" PRIX16, (uint16_t)Val);
}

StringRef ScalarTraits<Hex16>::निवेश_2(StringRef Scalar, व्योम *, Hex16 &Val) {
  अचिन्हित दीर्घ दीर्घ n;
  अगर (getAsUnsignedInteger(Scalar, 0, n))
    वापस "invalid hex16 number";
  अगर (n > 0xFFFF)
    वापस "out of range hex16 number";
  Val = n;
  वापस StringRef();
}

व्योम ScalarTraits<Hex32>::output(स्थिर Hex32 &Val, व्योम *, raw_ostream &Out) {
  Out << format("0x%" PRIX32, (uint32_t)Val);
}

StringRef ScalarTraits<Hex32>::निवेश_2(StringRef Scalar, व्योम *, Hex32 &Val) {
  अचिन्हित दीर्घ दीर्घ n;
  अगर (getAsUnsignedInteger(Scalar, 0, n))
    वापस "invalid hex32 number";
  अगर (n > 0xFFFFFFFFUL)
    वापस "out of range hex32 number";
  Val = n;
  वापस StringRef();
}

व्योम ScalarTraits<Hex64>::output(स्थिर Hex64 &Val, व्योम *, raw_ostream &Out) {
  Out << format("0x%" PRIX64, (uint64_t)Val);
}

StringRef ScalarTraits<Hex64>::निवेश_2(StringRef Scalar, व्योम *, Hex64 &Val) {
  अचिन्हित दीर्घ दीर्घ अंक;
  अगर (getAsUnsignedInteger(Scalar, 0, अंक))
    वापस "invalid hex64 number";
  Val = अंक;
  वापस StringRef();
}

व्योम ScalarTraits<VersionTuple>::output(स्थिर VersionTuple &Val, व्योम *,
                                        llvm::raw_ostream &Out) {
  Out << Val.getAsString();
}

StringRef ScalarTraits<VersionTuple>::निवेश_2(StringRef Scalar, व्योम *,
                                            VersionTuple &Val) {
  अगर (Val.tryParse(Scalar))
    वापस "invalid version format";
  वापस StringRef();
}
