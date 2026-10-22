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

शून्य IO::set_Allow_Unknown_कुंजियाँ(तार्किक Allow) {
  llvm_unreachable("Only supported for Input");
}

//===----------------------------------------------------------------------===//
//निवेश
//===----------------------------------------------------------------------===//

निवेश::निवेश(StringRef निवेश_Content, शून्य *Ctxt,
             SourceMgr::DiagHandlerTy DiagHandler, शून्य *DiagHandlerCtxt)
    : IO(Ctxt), Strm(नूतन Stream(निवेश_Content, SrcMgr, असत्य, &EC)) {
  यदि (DiagHandler)
    SrcMgr.setDiagHandler(DiagHandler, DiagHandlerCtxt);
  DocIterator = Strm->begin();
}

निवेश::निवेश(MemoryBufferRef निवेश, शून्य *Ctxt,
             SourceMgr::DiagHandlerTy DiagHandler, शून्य *DiagHandlerCtxt)
    : IO(Ctxt), Strm(नूतन Stream(निवेश, SrcMgr, असत्य, &EC)) {
  यदि (DiagHandler)
    SrcMgr.setDiagHandler(DiagHandler, DiagHandlerCtxt);
  DocIterator = Strm->begin();
}

निवेश::~निवेश() = स्वभाव;

std::error_code निवेश::error() { प्रत्यागम EC; }

तार्किक निवेश::outputting() ध्रुव {
  प्रत्यागम असत्य;
}

तार्किक निवेश::setCurrentDocument() {
  यदि (DocIterator != Strm->end()) {
    गाँठ *N = DocIterator->get_मूल();
    यदि (!N) {
      EC = make_error_code(errc::invalid_argument);
      प्रत्यागम असत्य;
    }

    यदि (isa<Null_गाँठ>(N)) {
      // Empty files are allowed and ignored
      ++DocIterator;
      प्रत्यागम setCurrentDocument();
    }
    releaseHNodeBuffers();
    शीर्ष_गाँठ = createHNodes(N);
    Current_गाँठ = शीर्ष_गाँठ;
    प्रत्यागम सत्य;
  }
  प्रत्यागम असत्य;
}

तार्किक निवेश::nextDocument() {
  प्रत्यागम ++DocIterator != Strm->end();
}

ध्रुव गाँठ *निवेश::get_Current_गाँठ() ध्रुव {
  प्रत्यागम Current_गाँठ ? Current_गाँठ->गाँठ_2 : nullptr;
}

तार्किक निवेश::mapTag(StringRef Tag, तार्किक Default) {
  // CurrentNode can be null if setCurrentDocument() was unable to
  // parse the document because it was invalid or empty.
  यदि (!Current_गाँठ)
    प्रत्यागम असत्य;

  std::string foundTag = Current_गाँठ->गाँठ_2->getVerbatimTag();
  यदि (foundTag.empty()) {
    // If no tag found and 'Tag' is the default, say it was found.
    प्रत्यागम Default;
  }
  // Return true iff found tag matches supplied tag.
  प्रत्यागम Tag == foundTag;
}

शून्य निवेश::beginMapping() {
  यदि (EC)
    प्रत्यागम;
  // CurrentNode can be null if the document is empty.
  MapHNode *MN = dyn_cast_or_null<MapHNode>(Current_गाँठ);
  यदि (MN) {
    MN->Valid_कुंजियाँ.clear();
  }
}

std::vector<StringRef> निवेश::कुंजियाँ() {
  MapHNode *MN = dyn_cast<MapHNode>(Current_गाँठ);
  std::vector<StringRef> Ret;
  यदि (!MN) {
    setError(Current_गाँठ, "not a mapping");
    प्रत्यागम Ret;
  }
  प्रति (auto &P : MN->Mapping)
    Ret.धकेलो_back(P.first());
  प्रत्यागम Ret;
}

तार्किक निवेश::preflight_कुंजी(StringRef कुंजी, तार्किक Required, तार्किक, तार्किक &UseDefault,
                         शून्य *&SaveInfo) {
  UseDefault = असत्य;
  यदि (EC)
    प्रत्यागम असत्य;

  // CurrentNode is null for empty documents, which is an error in case required
  // nodes are present.
  यदि (!Current_गाँठ) {
    यदि (Required)
      EC = make_error_code(errc::invalid_argument);
    अन्यथा
      UseDefault = सत्य;
    प्रत्यागम असत्य;
  }

  MapHNode *MN = dyn_cast<MapHNode>(Current_गाँठ);
  यदि (!MN) {
    यदि (Required || !isa<EmptyHNode>(Current_गाँठ))
      setError(Current_गाँठ, "not a mapping");
    अन्यथा
      UseDefault = सत्य;
    प्रत्यागम असत्य;
  }
  MN->Valid_कुंजियाँ.धकेलो_back(कुंजी.str());
  HNode *मूल्य = MN->Mapping[कुंजी].first;
  यदि (!मूल्य) {
    यदि (Required)
      setError(Current_गाँठ, Twine("missing required key '") + कुंजी + "'");
    अन्यथा
      UseDefault = सत्य;
    प्रत्यागम असत्य;
  }
  SaveInfo = Current_गाँठ;
  Current_गाँठ = मूल्य;
  प्रत्यागम सत्य;
}

शून्य निवेश::postflight_कुंजी(शून्य *saveInfo) {
  Current_गाँठ = reinterpret_cast<HNode *>(saveInfo);
}

शून्य निवेश::endMapping() {
  यदि (EC)
    प्रत्यागम;
  // CurrentNode can be null if the document is empty.
  MapHNode *MN = dyn_cast_or_null<MapHNode>(Current_गाँठ);
  यदि (!MN)
    प्रत्यागम;
  प्रति (ध्रुव auto &NN : MN->Mapping) {
    यदि (!is_contained(MN->Valid_कुंजियाँ, NN.first())) {
      ध्रुव SMRange &ReportLoc = NN.second.second;
      यदि (!Allow_Unknown_कुंजियाँ) {
        setError(ReportLoc, Twine("unknown key '") + NN.first() + "'");
        भङ्ग;
      } अन्यथा
        reportWarning(ReportLoc, Twine("unknown key '") + NN.first() + "'");
    }
  }
}

शून्य निवेश::beginFlowMapping() { beginMapping(); }

शून्य निवेश::endFlowMapping() { endMapping(); }

unsigned निवेश::beginSequence() {
  यदि (SequenceHNode *SQ = dyn_cast<SequenceHNode>(Current_गाँठ))
    प्रत्यागम SQ->Entries.size();
  यदि (isa<EmptyHNode>(Current_गाँठ))
    प्रत्यागम 0;
  // Treat case where there's a scalar "null" value as an empty sequence.
  यदि (ScalarHNode *SN = dyn_cast<ScalarHNode>(Current_गाँठ)) {
    यदि (isNull(SN->मूल्य_2()))
      प्रत्यागम 0;
  }
  // Any other type of HNode is an error.
  setError(Current_गाँठ, "not a sequence");
  प्रत्यागम 0;
}

शून्य निवेश::endSequence() {
}

तार्किक निवेश::preflightElement(unsigned Index, शून्य *&SaveInfo) {
  यदि (EC)
    प्रत्यागम असत्य;
  यदि (SequenceHNode *SQ = dyn_cast<SequenceHNode>(Current_गाँठ)) {
    SaveInfo = Current_गाँठ;
    Current_गाँठ = SQ->Entries[Index];
    प्रत्यागम सत्य;
  }
  प्रत्यागम असत्य;
}

शून्य निवेश::postflightElement(शून्य *SaveInfo) {
  Current_गाँठ = reinterpret_cast<HNode *>(SaveInfo);
}

unsigned निवेश::beginFlowSequence() { प्रत्यागम beginSequence(); }

तार्किक निवेश::preflightFlowElement(unsigned index, शून्य *&SaveInfo) {
  यदि (EC)
    प्रत्यागम असत्य;
  यदि (SequenceHNode *SQ = dyn_cast<SequenceHNode>(Current_गाँठ)) {
    SaveInfo = Current_गाँठ;
    Current_गाँठ = SQ->Entries[index];
    प्रत्यागम सत्य;
  }
  प्रत्यागम असत्य;
}

शून्य निवेश::postflightFlowElement(शून्य *SaveInfo) {
  Current_गाँठ = reinterpret_cast<HNode *>(SaveInfo);
}

शून्य निवेश::endFlowSequence() {
}

शून्य निवेश::beginEnumScalar() {
  ScalarMatchFound = असत्य;
}

तार्किक निवेश::matchEnumScalar(StringRef Str, तार्किक) {
  यदि (ScalarMatchFound)
    प्रत्यागम असत्य;
  यदि (ScalarHNode *SN = dyn_cast<ScalarHNode>(Current_गाँठ)) {
    यदि (SN->मूल्य_2() == Str) {
      ScalarMatchFound = सत्य;
      प्रत्यागम सत्य;
    }
  }
  प्रत्यागम असत्य;
}

तार्किक निवेश::matchEnumFallback() {
  यदि (ScalarMatchFound)
    प्रत्यागम असत्य;
  ScalarMatchFound = सत्य;
  प्रत्यागम सत्य;
}

शून्य निवेश::endEnumScalar() {
  यदि (!ScalarMatchFound) {
    setError(Current_गाँठ, "unknown enumerated scalar");
  }
}

तार्किक निवेश::beginBitSetScalar(तार्किक &DoClear) {
  Bit_मूल्यावली_Used.clear();
  यदि (SequenceHNode *SQ = dyn_cast<SequenceHNode>(Current_गाँठ)) {
    Bit_मूल्यावली_Used.resize(SQ->Entries.size());
  } अन्यथा {
    setError(Current_गाँठ, "expected sequence of bit values");
  }
  DoClear = सत्य;
  प्रत्यागम सत्य;
}

तार्किक निवेश::bitSetMatch(StringRef Str, तार्किक) {
  यदि (EC)
    प्रत्यागम असत्य;
  यदि (SequenceHNode *SQ = dyn_cast<SequenceHNode>(Current_गाँठ)) {
    unsigned Index = 0;
    प्रति (auto &N : SQ->Entries) {
      यदि (ScalarHNode *SN = dyn_cast<ScalarHNode>(N)) {
        यदि (SN->मूल्य_2() == Str) {
          Bit_मूल्यावली_Used[Index] = सत्य;
          प्रत्यागम सत्य;
        }
      } अन्यथा {
        setError(Current_गाँठ, "unexpected scalar in sequence of bit values");
      }
      ++Index;
    }
  } अन्यथा {
    setError(Current_गाँठ, "expected sequence of bit values");
  }
  प्रत्यागम असत्य;
}

शून्य निवेश::endBitSetScalar() {
  यदि (EC)
    प्रत्यागम;
  यदि (SequenceHNode *SQ = dyn_cast<SequenceHNode>(Current_गाँठ)) {
    प्रतिज्ञा(Bit_मूल्यावली_Used.size() == SQ->Entries.size());
    प्रति (unsigned i = 0; i < SQ->Entries.size(); ++i) {
      यदि (!Bit_मूल्यावली_Used[i]) {
        setError(SQ->Entries[i], "unknown bit value");
        प्रत्यागम;
      }
    }
  }
}

शून्य निवेश::scalarString(StringRef &S, QuotingType) {
  यदि (ScalarHNode *SN = dyn_cast<ScalarHNode>(Current_गाँठ)) {
    S = SN->मूल्य_2();
  } अन्यथा {
    setError(Current_गाँठ, "unexpected scalar");
  }
}

शून्य निवेश::blockScalarString(StringRef &S) { scalarString(S, QuotingType::None); }

शून्य निवेश::scalarTag(std::string &Tag) {
  Tag = Current_गाँठ->गाँठ_2->getVerbatimTag();
}

शून्य निवेश::setError(HNode *hnode, ध्रुव Twine &message) {
  प्रतिज्ञा(hnode && "HNode must not be NULL");
  setError(hnode->गाँठ_2, message);
}

गाँठ_Kind निवेश::get_गाँठ_Kind() {
  यदि (isa<ScalarHNode>(Current_गाँठ))
    प्रत्यागम गाँठ_Kind::Scalar;
  अन्यथा यदि (isa<MapHNode>(Current_गाँठ))
    प्रत्यागम गाँठ_Kind::Map;
  अन्यथा यदि (isa<SequenceHNode>(Current_गाँठ))
    प्रत्यागम गाँठ_Kind::Sequence;
  llvm_unreachable("Unsupported node kind");
}

शून्य निवेश::setError(गाँठ *गाँठ_3, ध्रुव Twine &message) {
  Strm->printError(गाँठ_3, message);
  EC = make_error_code(errc::invalid_argument);
}

शून्य निवेश::setError(ध्रुव SMRange &range, ध्रुव Twine &message) {
  Strm->printError(range, message);
  EC = make_error_code(errc::invalid_argument);
}

शून्य निवेश::reportWarning(HNode *hnode, ध्रुव Twine &message) {
  प्रतिज्ञा(hnode && "HNode must not be NULL");
  Strm->printError(hnode->गाँठ_2, message, SourceMgr::DK_Warning);
}

शून्य निवेश::reportWarning(गाँठ *गाँठ_3, ध्रुव Twine &message) {
  Strm->printError(गाँठ_3, message, SourceMgr::DK_Warning);
}

शून्य निवेश::reportWarning(ध्रुव SMRange &range, ध्रुव Twine &message) {
  Strm->printError(range, message, SourceMgr::DK_Warning);
}

शून्य निवेश::releaseHNodeBuffers() {
  EmptyHNodeAllocator.DestroyAll();
  ScalarHNodeAllocator.DestroyAll();
  SequenceHNodeAllocator.DestroyAll();
  MapHNodeAllocator.DestroyAll();
}

शून्य निवेश::saveAliasHNode(गाँठ *N, HNode *HN) {
  StringRef Anchor = N->getAnchor();
  यदि (!Anchor.empty())
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
  विकल्प (N->getType()) {
  पक्ष गाँठ::NK_Scalar: {
    Scalar_गाँठ *SN = dyn_cast<Scalar_गाँठ>(N);
    StringRef कुंजी_Str = SN->get_मूल्य(StringStorage);
    यदि (!StringStorage.empty()) {
      // Copy string to permanent storage
      कुंजी_Str = StringStorage.str().copy(StringAllocator);
    }
    auto *SHNode = नूतन (ScalarHNodeAllocator.Allocate()) ScalarHNode(N, कुंजी_Str);
    saveAliasHNode(SN, SHNode);
    प्रत्यागम SHNode;
  }
  पक्ष गाँठ::NK_BlockScalar: {
    Block_Scalar_गाँठ *BSN = dyn_cast<Block_Scalar_गाँठ>(N);
    StringRef मूल्य_Copy = BSN->get_मूल्य().copy(StringAllocator);
    auto *BSHNode =
        नूतन (ScalarHNodeAllocator.Allocate()) ScalarHNode(N, मूल्य_Copy);
    saveAliasHNode(BSN, BSHNode);
    प्रत्यागम BSHNode;
  }
  पक्ष गाँठ::NK_Sequence: {
    Sequence_गाँठ *SQ = dyn_cast<Sequence_गाँठ>(N);
    auto SQHNode = नूतन (SequenceHNodeAllocator.Allocate()) SequenceHNode(N);
    प्रति (गाँठ &SN : *SQ) {
      auto Entry = createHNodes(&SN);
      यदि (EC)
        भङ्ग;
      SQHNode->Entries.धकेलो_back(Entry);
    }
    saveAliasHNode(SQ, SQHNode);
    प्रत्यागम SQHNode;
  }
  पक्ष गाँठ::NK_Mapping: {
    Mapping_गाँठ *Map = dyn_cast<Mapping_गाँठ>(N);
    auto mapHNode = नूतन (MapHNodeAllocator.Allocate()) MapHNode(N);
    प्रति (कुंजी_मूल्य_गाँठ &KVN : *Map) {
      गाँठ *कुंजी_गाँठ = KVN.get_कुंजी();
      Scalar_गाँठ *कुंजी = dyn_cast_if_present<Scalar_गाँठ>(कुंजी_गाँठ);
      गाँठ *मूल्य = KVN.get_मूल्य();
      यदि (!कुंजी || !मूल्य) {
        यदि (!कुंजी)
          setError(कुंजी_गाँठ, "Map key must be a scalar");
        यदि (!मूल्य)
          setError(कुंजी_गाँठ, "Map value must not be empty");
        भङ्ग;
      }
      StringStorage.clear();
      StringRef कुंजी_Str = कुंजी->get_मूल्य(StringStorage);
      यदि (!StringStorage.empty()) {
        // Copy string to permanent storage
        कुंजी_Str = StringStorage.str().copy(StringAllocator);
      }
      यदि (mapHNode->Mapping.गिनती(कुंजी_Str))
        // From YAML spec: "The content of a mapping node is an unordered set of
        // key/value node pairs, with the restriction that each of the keys is
        // unique."
        setError(कुंजी_गाँठ, Twine("duplicated mapping key '") + कुंजी_Str + "'");
      auto मूल्य_HNode = createHNodes(मूल्य);
      यदि (EC)
        भङ्ग;
      mapHNode->Mapping[कुंजी_Str] =
          std::make_pair(std::move(मूल्य_HNode), कुंजी_गाँठ->getSourceRange());
    }
    saveAliasHNode(Map, mapHNode);
    प्रत्यागम std::move(mapHNode);
  }
  पक्ष गाँठ::NK_Null:
    // TODO: Anchor is not set for NullNode in the parser. Update the parser to
    // faithfully preserve anchors.
    प्रत्यागम नूतन (EmptyHNodeAllocator.Allocate()) EmptyHNode(N);
  पक्ष गाँठ::NK_Alias: {
    Alias_गाँठ *AN = dyn_cast<Alias_गाँठ>(N);
    auto AliasName = AN->getName();
    auto AHN = AliasMap.find(AliasName);
    यदि (AHN == AliasMap.end()) {
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

शून्य निवेश::setError(ध्रुव Twine &Message) {
  setError(Current_गाँठ, Message);
}

शून्य निवेश::set_Allow_Unknown_कुंजियाँ(तार्किक Allow) { Allow_Unknown_कुंजियाँ = Allow; }

तार्किक निवेश::canElideEmptySequence() {
  प्रत्यागम असत्य;
}

//===----------------------------------------------------------------------===//
//  Output
//===----------------------------------------------------------------------===//

Output::Output(raw_ostream &yout, शून्य *context, पूर्णाङ्क WrapColumn)
    : IO(context), Out(yout), WrapColumn(WrapColumn) {}

Output::~Output() = स्वभाव;

तार्किक Output::outputting() ध्रुव {
  प्रत्यागम सत्य;
}

शून्य Output::beginMapping() {
  State_ढेर.धकेलो_back(in_Map_First_कुंजी);
  PaddingBeforeContainer = Padding;
  Padding = "\n";
}

तार्किक Output::mapTag(StringRef Tag, तार्किक Use) {
  यदि (Use) {
    // If this tag is being written inside a sequence we should write the start
    // of the sequence before writing the tag, otherwise the tag won't be
    // attached to the element in the sequence, but rather the sequence itself.
    तार्किक SequenceElement = असत्य;
    यदि (State_ढेर.size() > 1) {
      auto &E = State_ढेर[State_ढेर.size() - 2];
      SequenceElement = inSeqAnyElement(E) || inFlowSeqAnyElement(E);
    }
    यदि (SequenceElement && State_ढेर.back() == in_Map_First_कुंजी) {
      new_पंक्ति_Check();
    } अन्यथा {
      output(" ");
    }
    output(Tag);
    यदि (SequenceElement) {
      // If we're writing the tag during the first element of a map, the tag
      // takes the place of the first element in the sequence.
      यदि (State_ढेर.back() == in_Map_First_कुंजी) {
        State_ढेर.निकालो_back();
        State_ढेर.धकेलो_back(in_Map_Other_कुंजी);
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
  यदि (State_ढेर.back() == in_Map_First_कुंजी) {
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

तार्किक Output::preflight_कुंजी(StringRef कुंजी, तार्किक Required, तार्किक SameAsDefault,
                          तार्किक &UseDefault, शून्य *&SaveInfo) {
  UseDefault = असत्य;
  SaveInfo = nullptr;
  यदि (Required || !SameAsDefault || Write_Default_मूल्यावली) {
    auto State = State_ढेर.back();
    यदि (State == in_Flow_Map_First_कुंजी || State == in_Flow_Map_Other_कुंजी) {
      flow_कुंजी(कुंजी);
    } अन्यथा {
      new_पंक्ति_Check();
      padded_कुंजी(कुंजी);
    }
    प्रत्यागम सत्य;
  }
  प्रत्यागम असत्य;
}

शून्य Output::postflight_कुंजी(शून्य *) {
  यदि (State_ढेर.back() == in_Map_First_कुंजी) {
    State_ढेर.निकालो_back();
    State_ढेर.धकेलो_back(in_Map_Other_कुंजी);
  } अन्यथा यदि (State_ढेर.back() == in_Flow_Map_First_कुंजी) {
    State_ढेर.निकालो_back();
    State_ढेर.धकेलो_back(in_Flow_Map_Other_कुंजी);
  }
}

शून्य Output::beginFlowMapping() {
  State_ढेर.धकेलो_back(in_Flow_Map_First_कुंजी);
  new_पंक्ति_Check();
  ColumnAtMapFlowStart = Column;
  output("{ ");
}

शून्य Output::endFlowMapping() {
  State_ढेर.निकालो_back();
  output_Up_To_End_Of_पंक्ति(" }");
}

शून्य Output::beginDocuments() {
  output_Up_To_End_Of_पंक्ति("---");
}

तार्किक Output::preflightDocument(unsigned index) {
  यदि (index > 0)
    output_Up_To_End_Of_पंक्ति("\n---");
  प्रत्यागम सत्य;
}

शून्य Output::postflightDocument() {
}

शून्य Output::endDocuments() {
  output("\n...\n");
}

unsigned Output::beginSequence() {
  State_ढेर.धकेलो_back(inSeqFirstElement);
  PaddingBeforeContainer = Padding;
  Padding = "\n";
  प्रत्यागम 0;
}

शून्य Output::endSequence() {
  // If we did not emit anything, we should explicitly emit an empty sequence
  यदि (State_ढेर.back() == inSeqFirstElement) {
    Padding = PaddingBeforeContainer;
    new_पंक्ति_Check(/*EmptySequence=*/सत्य);
    output("[]");
    Padding = "\n";
  }
  State_ढेर.निकालो_back();
}

तार्किक Output::preflightElement(unsigned, शून्य *&SaveInfo) {
  SaveInfo = nullptr;
  प्रत्यागम सत्य;
}

शून्य Output::postflightElement(शून्य *) {
  यदि (State_ढेर.back() == inSeqFirstElement) {
    State_ढेर.निकालो_back();
    State_ढेर.धकेलो_back(inSeqOtherElement);
  } अन्यथा यदि (State_ढेर.back() == inFlowSeqFirstElement) {
    State_ढेर.निकालो_back();
    State_ढेर.धकेलो_back(inFlowSeqOtherElement);
  }
}

unsigned Output::beginFlowSequence() {
  State_ढेर.धकेलो_back(inFlowSeqFirstElement);
  new_पंक्ति_Check();
  ColumnAtFlowStart = Column;
  output("[ ");
  NeedFlowSequenceComma = असत्य;
  प्रत्यागम 0;
}

शून्य Output::endFlowSequence() {
  State_ढेर.निकालो_back();
  output_Up_To_End_Of_पंक्ति(" ]");
}

तार्किक Output::preflightFlowElement(unsigned, शून्य *&SaveInfo) {
  यदि (NeedFlowSequenceComma)
    output(", ");
  यदि (WrapColumn && Column > WrapColumn) {
    output("\n");
    प्रति (पूर्णाङ्क i = 0; i < ColumnAtFlowStart; ++i)
      output(" ");
    Column = ColumnAtFlowStart;
    output("  ");
  }
  SaveInfo = nullptr;
  प्रत्यागम सत्य;
}

शून्य Output::postflightFlowElement(शून्य *) {
  NeedFlowSequenceComma = सत्य;
}

शून्य Output::beginEnumScalar() {
  EnumerationMatchFound = असत्य;
}

तार्किक Output::matchEnumScalar(StringRef Str, तार्किक Match) {
  यदि (Match && !EnumerationMatchFound) {
    new_पंक्ति_Check();
    output_Up_To_End_Of_पंक्ति(Str);
    EnumerationMatchFound = सत्य;
  }
  प्रत्यागम असत्य;
}

तार्किक Output::matchEnumFallback() {
  यदि (EnumerationMatchFound)
    प्रत्यागम असत्य;
  EnumerationMatchFound = सत्य;
  प्रत्यागम सत्य;
}

शून्य Output::endEnumScalar() {
  यदि (!EnumerationMatchFound)
    llvm_unreachable("bad runtime enum value");
}

तार्किक Output::beginBitSetScalar(तार्किक &DoClear) {
  new_पंक्ति_Check();
  output("[ ");
  Need_Bit_मूल्य_Comma = असत्य;
  DoClear = असत्य;
  प्रत्यागम सत्य;
}

तार्किक Output::bitSetMatch(StringRef Str, तार्किक Matches) {
  यदि (Matches) {
    यदि (Need_Bit_मूल्य_Comma)
      output(", ");
    output(Str);
    Need_Bit_मूल्य_Comma = सत्य;
  }
  प्रत्यागम असत्य;
}

शून्य Output::endBitSetScalar() {
  output_Up_To_End_Of_पंक्ति(" ]");
}

शून्य Output::scalarString(StringRef &S, QuotingType MustQuote) {
  new_पंक्ति_Check();
  यदि (S.empty()) {
    // Print '' for the empty string because leaving the field empty is not
    // allowed.
    output_Up_To_End_Of_पंक्ति("''");
    प्रत्यागम;
  }
  output(S, MustQuote);
  output_Up_To_End_Of_पंक्ति("");
}

शून्य Output::blockScalarString(StringRef &S) {
  यदि (!State_ढेर.empty())
    new_पंक्ति_Check();
  output(" |");

  unsigned Indent = State_ढेर.empty() ? 1 : State_ढेर.size();

  auto Buffer = MemoryBuffer::getMemBuffer(S, "", असत्य);
  प्रति (पंक्ति_iterator पंक्तियाँ(*Buffer, असत्य); !पंक्तियाँ.is_at_end(); ++पंक्तियाँ) {
    output_New_पंक्ति();
    प्रति (unsigned I = 0; I < Indent; ++I) {
      output("  ");
    }
    output(*पंक्तियाँ);
  }
  output_Up_To_End_Of_पंक्ति("");
}

शून्य Output::scalarTag(std::string &Tag) {
  यदि (Tag.empty())
    प्रत्यागम;
  new_पंक्ति_Check();
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
  यदि (State_ढेर.size() < 2)
    प्रत्यागम सत्य;
  यदि (State_ढेर.back() != in_Map_First_कुंजी)
    प्रत्यागम सत्य;
  प्रत्यागम !inSeqAnyElement(State_ढेर[State_ढेर.size() - 2]);
}

शून्य Output::output(StringRef s) {
  Column += s.size();
  Out << s;
}

शून्य Output::output(StringRef S, QuotingType MustQuote) {
  यदि (MustQuote == QuotingType::None) {
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
  यदि (MustQuote == QuotingType::Double) {
    output(yaml::escape(S, /* EscapePrintable= */ असत्य));
    output(Quote);
    प्रत्यागम;
  }

  unsigned i = 0;
  unsigned j = 0;
  unsigned End = S.size();
  ध्रुव वर्ण *Base = S.data();

  // When using single-quoted strings, any single quote ' must be doubled to be
  // escaped.
  यावत् (j < End) {
    यदि (S[j] == '\'') {                   // Escape quotes.
      output(StringRef(&Base[i], j - i)); // "flush".
      output(StringLiteral("''"));        // Print it as ''
      i = j + 1;
    }
    ++j;
  }
  output(StringRef(&Base[i], j - i));
  output(Quote); // Ending quote.
}

शून्य Output::output_Up_To_End_Of_पंक्ति(StringRef s) {
  output(s);
  यदि (State_ढेर.empty() || (!inFlowSeqAnyElement(State_ढेर.back()) &&
                             !in_Flow_Map_Any_कुंजी(State_ढेर.back())))
    Padding = "\n";
}

शून्य Output::output_New_पंक्ति() {
  Out << "\n";
  Column = 0;
}

// if seq at top, indent as if map, then add "- "
// if seq in middle, use "- " if firstKey, else use "  "
//

शून्य Output::new_पंक्ति_Check(तार्किक EmptySequence) {
  यदि (Padding != "\n") {
    output(Padding);
    Padding = {};
    प्रत्यागम;
  }
  output_New_पंक्ति();
  Padding = {};

  यदि (State_ढेर.size() == 0 || EmptySequence)
    प्रत्यागम;

  unsigned Indent = State_ढेर.size() - 1;
  तार्किक PossiblyNestedSeq = असत्य;
  auto I = State_ढेर.rbegin(), E = State_ढेर.rend();

  यदि (inSeqAnyElement(*I)) {
    PossiblyNestedSeq = सत्य; // Not possibly but always.
    ++Indent;
  } अन्यथा यदि (*I == in_Map_First_कुंजी || *I == in_Flow_Map_First_कुंजी ||
             inFlowSeqAnyElement(*I)) {
    PossiblyNestedSeq = सत्य;
    ++I; // Skip back().
  }

  unsigned Output_Dash_गिनती = 0;
  यदि (PossiblyNestedSeq) {
    // Count up consecutive inSeqFirstElement from the end, unless
    // inSeqFirstElement is the top of nested sequence.
    यावत् (I != E) {
      // Don't count the top of nested sequence.
      यदि (!inSeqAnyElement(*I))
        भङ्ग;

      ++Output_Dash_गिनती;

      // Stop counting if consecutive inSeqFirstElement ends.
      यदि (*I++ != inSeqFirstElement)
        भङ्ग;
    }
  }

  प्रति (unsigned I = Output_Dash_गिनती; I < Indent; ++I)
    output("  ");

  प्रति (unsigned I = 0; I < Output_Dash_गिनती; ++I)
    output("- ");
}

शून्य Output::padded_कुंजी(StringRef कुंजी_2) {
  output(कुंजी_2, needsQuotes(कुंजी_2, असत्य));
  output(":");
  ध्रुव वर्ण *spaces = "                ";
  यदि (कुंजी_2.size() < strlen(spaces))
    Padding = &spaces[कुंजी_2.size()];
  अन्यथा
    Padding = " ";
}

शून्य Output::flow_कुंजी(StringRef कुंजी) {
  यदि (State_ढेर.back() == in_Flow_Map_Other_कुंजी)
    output(", ");
  यदि (WrapColumn && Column > WrapColumn) {
    output("\n");
    प्रति (पूर्णाङ्क I = 0; I < ColumnAtMapFlowStart; ++I)
      output(" ");
    Column = ColumnAtMapFlowStart;
    output("  ");
  }
  output(कुंजी, needsQuotes(कुंजी, असत्य));
  output(": ");
}

गाँठ_Kind Output::get_गाँठ_Kind() { report_fatal_error("invalid call"); }

तार्किक Output::inSeqAnyElement(InState State) {
  प्रत्यागम State == inSeqFirstElement || State == inSeqOtherElement;
}

तार्किक Output::inFlowSeqAnyElement(InState State) {
  प्रत्यागम State == inFlowSeqFirstElement || State == inFlowSeqOtherElement;
}

तार्किक Output::in_Map_Any_कुंजी(InState State) {
  प्रत्यागम State == in_Map_First_कुंजी || State == in_Map_Other_कुंजी;
}

तार्किक Output::in_Flow_Map_Any_कुंजी(InState State) {
  प्रत्यागम State == in_Flow_Map_First_कुंजी || State == in_Flow_Map_Other_कुंजी;
}

//===----------------------------------------------------------------------===//
//  traits for built-in types
//===----------------------------------------------------------------------===//

शून्य ScalarTraits<तार्किक>::output(ध्रुव तार्किक &Val, शून्य *, raw_ostream &Out) {
  Out << (Val ? "true" : "false");
}

StringRef ScalarTraits<तार्किक>::निवेश_2(StringRef Scalar, शून्य *, तार्किक &Val) {
  यदि (std::optional<तार्किक> Parsed = parseBool(Scalar)) {
    Val = *Parsed;
    प्रत्यागम StringRef();
  }
  प्रत्यागम "invalid boolean";
}

शून्य ScalarTraits<StringRef>::output(ध्रुव StringRef &Val, शून्य *,
                                     raw_ostream &Out) {
  Out << Val;
}

StringRef ScalarTraits<StringRef>::निवेश_2(StringRef Scalar, शून्य *,
                                         StringRef &Val) {
  Val = Scalar;
  प्रत्यागम StringRef();
}

शून्य ScalarTraits<std::string>::output(ध्रुव std::string &Val, शून्य *,
                                       raw_ostream &Out) {
  Out << Val;
}

StringRef ScalarTraits<std::string>::निवेश_2(StringRef Scalar, शून्य *,
                                           std::string &Val) {
  Val = Scalar.str();
  प्रत्यागम StringRef();
}

शून्य ScalarTraits<uint8_t>::output(ध्रुव uint8_t &Val, शून्य *,
                                   raw_ostream &Out) {
  // use temp uin32_t because ostream thinks uint8_t is a character
  uint32_t अंक = Val;
  Out << अंक;
}

StringRef ScalarTraits<uint8_t>::निवेश_2(StringRef Scalar, शून्य *, uint8_t &Val) {
  unsigned long long n;
  यदि (getAsUnsignedInteger(Scalar, 0, n))
    प्रत्यागम "invalid number";
  यदि (n > 0xFF)
    प्रत्यागम "out of range number";
  Val = n;
  प्रत्यागम StringRef();
}

शून्य ScalarTraits<uint16_t>::output(ध्रुव uint16_t &Val, शून्य *,
                                    raw_ostream &Out) {
  Out << Val;
}

StringRef ScalarTraits<uint16_t>::निवेश_2(StringRef Scalar, शून्य *,
                                        uint16_t &Val) {
  unsigned long long n;
  यदि (getAsUnsignedInteger(Scalar, 0, n))
    प्रत्यागम "invalid number";
  यदि (n > 0xFFFF)
    प्रत्यागम "out of range number";
  Val = n;
  प्रत्यागम StringRef();
}

शून्य ScalarTraits<uint32_t>::output(ध्रुव uint32_t &Val, शून्य *,
                                    raw_ostream &Out) {
  Out << Val;
}

StringRef ScalarTraits<uint32_t>::निवेश_2(StringRef Scalar, शून्य *,
                                        uint32_t &Val) {
  unsigned long long n;
  यदि (getAsUnsignedInteger(Scalar, 0, n))
    प्रत्यागम "invalid number";
  यदि (n > 0xFFFFFFFFUL)
    प्रत्यागम "out of range number";
  Val = n;
  प्रत्यागम StringRef();
}

शून्य ScalarTraits<uint64_t>::output(ध्रुव uint64_t &Val, शून्य *,
                                    raw_ostream &Out) {
  Out << Val;
}

StringRef ScalarTraits<uint64_t>::निवेश_2(StringRef Scalar, शून्य *,
                                        uint64_t &Val) {
  unsigned long long N;
  यदि (getAsUnsignedInteger(Scalar, 0, N))
    प्रत्यागम "invalid number";
  Val = N;
  प्रत्यागम StringRef();
}

शून्य ScalarTraits<int8_t>::output(ध्रुव int8_t &Val, शून्य *, raw_ostream &Out) {
  // use temp in32_t because ostream thinks int8_t is a character
  int32_t अंक = Val;
  Out << अंक;
}

StringRef ScalarTraits<int8_t>::निवेश_2(StringRef Scalar, शून्य *, int8_t &Val) {
  long long N;
  यदि (getAsSignedInteger(Scalar, 0, N))
    प्रत्यागम "invalid number";
  यदि ((N > 127) || (N < -128))
    प्रत्यागम "out of range number";
  Val = N;
  प्रत्यागम StringRef();
}

शून्य ScalarTraits<int16_t>::output(ध्रुव int16_t &Val, शून्य *,
                                   raw_ostream &Out) {
  Out << Val;
}

StringRef ScalarTraits<int16_t>::निवेश_2(StringRef Scalar, शून्य *, int16_t &Val) {
  long long N;
  यदि (getAsSignedInteger(Scalar, 0, N))
    प्रत्यागम "invalid number";
  यदि ((N > INT16_MAX) || (N < INT16_MIN))
    प्रत्यागम "out of range number";
  Val = N;
  प्रत्यागम StringRef();
}

शून्य ScalarTraits<int32_t>::output(ध्रुव int32_t &Val, शून्य *,
                                   raw_ostream &Out) {
  Out << Val;
}

StringRef ScalarTraits<int32_t>::निवेश_2(StringRef Scalar, शून्य *, int32_t &Val) {
  long long N;
  यदि (getAsSignedInteger(Scalar, 0, N))
    प्रत्यागम "invalid number";
  यदि ((N > INT32_MAX) || (N < INT32_MIN))
    प्रत्यागम "out of range number";
  Val = N;
  प्रत्यागम StringRef();
}

शून्य ScalarTraits<int64_t>::output(ध्रुव int64_t &Val, शून्य *,
                                   raw_ostream &Out) {
  Out << Val;
}

StringRef ScalarTraits<int64_t>::निवेश_2(StringRef Scalar, शून्य *, int64_t &Val) {
  long long N;
  यदि (getAsSignedInteger(Scalar, 0, N))
    प्रत्यागम "invalid number";
  Val = N;
  प्रत्यागम StringRef();
}

शून्य ScalarTraits<द्विगुण>::output(ध्रुव द्विगुण &Val, शून्य *, raw_ostream &Out) {
  Out << format("%g", Val);
}

StringRef ScalarTraits<द्विगुण>::निवेश_2(StringRef Scalar, शून्य *, द्विगुण &Val) {
  यदि (to_float(Scalar, Val))
    प्रत्यागम StringRef();
  प्रत्यागम "invalid floating point number";
}

शून्य ScalarTraits<भिन्न>::output(ध्रुव भिन्न &Val, शून्य *, raw_ostream &Out) {
  Out << format("%g", Val);
}

StringRef ScalarTraits<भिन्न>::निवेश_2(StringRef Scalar, शून्य *, भिन्न &Val) {
  यदि (to_float(Scalar, Val))
    प्रत्यागम StringRef();
  प्रत्यागम "invalid floating point number";
}

शून्य ScalarTraits<Hex8>::output(ध्रुव Hex8 &Val, शून्य *, raw_ostream &Out) {
  Out << format("0x%" PRIX8, (uint8_t)Val);
}

StringRef ScalarTraits<Hex8>::निवेश_2(StringRef Scalar, शून्य *, Hex8 &Val) {
  unsigned long long n;
  यदि (getAsUnsignedInteger(Scalar, 0, n))
    प्रत्यागम "invalid hex8 number";
  यदि (n > 0xFF)
    प्रत्यागम "out of range hex8 number";
  Val = n;
  प्रत्यागम StringRef();
}

शून्य ScalarTraits<Hex16>::output(ध्रुव Hex16 &Val, शून्य *, raw_ostream &Out) {
  Out << format("0x%" PRIX16, (uint16_t)Val);
}

StringRef ScalarTraits<Hex16>::निवेश_2(StringRef Scalar, शून्य *, Hex16 &Val) {
  unsigned long long n;
  यदि (getAsUnsignedInteger(Scalar, 0, n))
    प्रत्यागम "invalid hex16 number";
  यदि (n > 0xFFFF)
    प्रत्यागम "out of range hex16 number";
  Val = n;
  प्रत्यागम StringRef();
}

शून्य ScalarTraits<Hex32>::output(ध्रुव Hex32 &Val, शून्य *, raw_ostream &Out) {
  Out << format("0x%" PRIX32, (uint32_t)Val);
}

StringRef ScalarTraits<Hex32>::निवेश_2(StringRef Scalar, शून्य *, Hex32 &Val) {
  unsigned long long n;
  यदि (getAsUnsignedInteger(Scalar, 0, n))
    प्रत्यागम "invalid hex32 number";
  यदि (n > 0xFFFFFFFFUL)
    प्रत्यागम "out of range hex32 number";
  Val = n;
  प्रत्यागम StringRef();
}

शून्य ScalarTraits<Hex64>::output(ध्रुव Hex64 &Val, शून्य *, raw_ostream &Out) {
  Out << format("0x%" PRIX64, (uint64_t)Val);
}

StringRef ScalarTraits<Hex64>::निवेश_2(StringRef Scalar, शून्य *, Hex64 &Val) {
  unsigned long long अंक;
  यदि (getAsUnsignedInteger(Scalar, 0, अंक))
    प्रत्यागम "invalid hex64 number";
  Val = अंक;
  प्रत्यागम StringRef();
}

शून्य ScalarTraits<VersionTuple>::output(ध्रुव VersionTuple &Val, शून्य *,
                                        llvm::raw_ostream &Out) {
  Out << Val.getAsString();
}

StringRef ScalarTraits<VersionTuple>::निवेश_2(StringRef Scalar, शून्य *,
                                            VersionTuple &Val) {
  यदि (Val.tryParse(Scalar))
    प्रत्यागम "invalid version format";
  प्रत्यागम StringRef();
}
