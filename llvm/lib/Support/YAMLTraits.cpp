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

शून्य *IO::getContext() अचर {
  फर्काऊ Ctxt;
}

शून्य IO::setContext(शून्य *Context) {
  Ctxt = Context;
}

शून्य IO::set_Allow_Unknown_साँचोहरू(तार्किक Allow) {
  llvm_unreachable("Only supported for Input");
}

//===----------------------------------------------------------------------===//
//आगत
//===----------------------------------------------------------------------===//

आगत::आगत(StringRef आगत_Content, शून्य *Ctxt,
             SourceMgr::DiagHandlerTy DiagHandler, शून्य *DiagHandlerCtxt)
    : IO(Ctxt), Strm(नूतन Stream(आगत_Content, SrcMgr, असत्य, &EC)) {
  यदि (DiagHandler)
    SrcMgr.setDiagHandler(DiagHandler, DiagHandlerCtxt);
  DocIterator = Strm->begin();
}

आगत::आगत(MemoryBufferRef आगत, शून्य *Ctxt,
             SourceMgr::DiagHandlerTy DiagHandler, शून्य *DiagHandlerCtxt)
    : IO(Ctxt), Strm(नूतन Stream(आगत, SrcMgr, असत्य, &EC)) {
  यदि (DiagHandler)
    SrcMgr.setDiagHandler(DiagHandler, DiagHandlerCtxt);
  DocIterator = Strm->begin();
}

आगत::~आगत() = स्वभाव;

std::error_code आगत::error() { फर्काऊ EC; }

तार्किक आगत::outputting() अचर {
  फर्काऊ असत्य;
}

तार्किक आगत::setCurrentDocument() {
  यदि (DocIterator != Strm->end()) {
    गाँठो *N = DocIterator->get_जरा();
    यदि (!N) {
      EC = make_error_code(errc::invalid_argument);
      फर्काऊ असत्य;
    }

    यदि (isa<Null_गाँठो>(N)) {
      // Empty files are allowed and ignored
      ++DocIterator;
      फर्काऊ setCurrentDocument();
    }
    releaseHNodeBuffers();
    शीर्ष_गाँठो = createHNodes(N);
    Current_गाँठो = शीर्ष_गाँठो;
    फर्काऊ सत्य;
  }
  फर्काऊ असत्य;
}

तार्किक आगत::nextDocument() {
  फर्काऊ ++DocIterator != Strm->end();
}

अचर गाँठो *आगत::get_Current_गाँठो() अचर {
  फर्काऊ Current_गाँठो ? Current_गाँठो->गाँठो_2 : nullptr;
}

तार्किक आगत::mapTag(StringRef Tag, तार्किक Default) {
  // CurrentNode can be null if setCurrentDocument() was unable to
  // parse the document because it was invalid or empty.
  यदि (!Current_गाँठो)
    फर्काऊ असत्य;

  std::string foundTag = Current_गाँठो->गाँठो_2->getVerbatimTag();
  यदि (foundTag.empty()) {
    // If no tag found and 'Tag' is the default, say it was found.
    फर्काऊ Default;
  }
  // Return true iff found tag matches supplied tag.
  फर्काऊ Tag == foundTag;
}

शून्य आगत::beginMapping() {
  यदि (EC)
    फर्काऊ;
  // CurrentNode can be null if the document is empty.
  MapHNode *MN = dyn_cast_or_null<MapHNode>(Current_गाँठो);
  यदि (MN) {
    MN->Valid_साँचोहरू.clear();
  }
}

std::vector<StringRef> आगत::साँचोहरू() {
  MapHNode *MN = dyn_cast<MapHNode>(Current_गाँठो);
  std::vector<StringRef> Ret;
  यदि (!MN) {
    setError(Current_गाँठो, "not a mapping");
    फर्काऊ Ret;
  }
  लागि (स्वतः &P : MN->Mapping)
    Ret.थप_back(P.first());
  फर्काऊ Ret;
}

तार्किक आगत::preflight_साँचो(StringRef साँचो, तार्किक Required, तार्किक, तार्किक &UseDefault,
                         शून्य *&SaveInfo) {
  UseDefault = असत्य;
  यदि (EC)
    फर्काऊ असत्य;

  // CurrentNode is null for empty documents, which is an error in case required
  // nodes are present.
  यदि (!Current_गाँठो) {
    यदि (Required)
      EC = make_error_code(errc::invalid_argument);
    अन्यथा
      UseDefault = सत्य;
    फर्काऊ असत्य;
  }

  MapHNode *MN = dyn_cast<MapHNode>(Current_गाँठो);
  यदि (!MN) {
    यदि (Required || !isa<EmptyHNode>(Current_गाँठो))
      setError(Current_गाँठो, "not a mapping");
    अन्यथा
      UseDefault = सत्य;
    फर्काऊ असत्य;
  }
  MN->Valid_साँचोहरू.थप_back(साँचो.str());
  HNode *मान = MN->Mapping[साँचो].first;
  यदि (!मान) {
    यदि (Required)
      setError(Current_गाँठो, Twine("missing required key '") + साँचो + "'");
    अन्यथा
      UseDefault = सत्य;
    फर्काऊ असत्य;
  }
  SaveInfo = Current_गाँठो;
  Current_गाँठो = मान;
  फर्काऊ सत्य;
}

शून्य आगत::postflight_साँचो(शून्य *saveInfo) {
  Current_गाँठो = reinterpret_cast<HNode *>(saveInfo);
}

शून्य आगत::endMapping() {
  यदि (EC)
    फर्काऊ;
  // CurrentNode can be null if the document is empty.
  MapHNode *MN = dyn_cast_or_null<MapHNode>(Current_गाँठो);
  यदि (!MN)
    फर्काऊ;
  लागि (अचर स्वतः &NN : MN->Mapping) {
    यदि (!is_contained(MN->Valid_साँचोहरू, NN.first())) {
      अचर SMRange &ReportLoc = NN.second.second;
      यदि (!Allow_Unknown_साँचोहरू) {
        setError(ReportLoc, Twine("unknown key '") + NN.first() + "'");
        रोक;
      } अन्यथा
        reportWarning(ReportLoc, Twine("unknown key '") + NN.first() + "'");
    }
  }
}

शून्य आगत::beginFlowMapping() { beginMapping(); }

शून्य आगत::endFlowMapping() { endMapping(); }

अचिन्हित आगत::beginSequence() {
  यदि (SequenceHNode *SQ = dyn_cast<SequenceHNode>(Current_गाँठो))
    फर्काऊ SQ->Entries.size();
  यदि (isa<EmptyHNode>(Current_गाँठो))
    फर्काऊ 0;
  // Treat case where there's a scalar "null" value as an empty sequence.
  यदि (ScalarHNode *SN = dyn_cast<ScalarHNode>(Current_गाँठो)) {
    यदि (isNull(SN->मान_2()))
      फर्काऊ 0;
  }
  // Any other type of HNode is an error.
  setError(Current_गाँठो, "not a sequence");
  फर्काऊ 0;
}

शून्य आगत::endSequence() {
}

तार्किक आगत::preflightElement(अचिन्हित Index, शून्य *&SaveInfo) {
  यदि (EC)
    फर्काऊ असत्य;
  यदि (SequenceHNode *SQ = dyn_cast<SequenceHNode>(Current_गाँठो)) {
    SaveInfo = Current_गाँठो;
    Current_गाँठो = SQ->Entries[Index];
    फर्काऊ सत्य;
  }
  फर्काऊ असत्य;
}

शून्य आगत::postflightElement(शून्य *SaveInfo) {
  Current_गाँठो = reinterpret_cast<HNode *>(SaveInfo);
}

अचिन्हित आगत::beginFlowSequence() { फर्काऊ beginSequence(); }

तार्किक आगत::preflightFlowElement(अचिन्हित index, शून्य *&SaveInfo) {
  यदि (EC)
    फर्काऊ असत्य;
  यदि (SequenceHNode *SQ = dyn_cast<SequenceHNode>(Current_गाँठो)) {
    SaveInfo = Current_गाँठो;
    Current_गाँठो = SQ->Entries[index];
    फर्काऊ सत्य;
  }
  फर्काऊ असत्य;
}

शून्य आगत::postflightFlowElement(शून्य *SaveInfo) {
  Current_गाँठो = reinterpret_cast<HNode *>(SaveInfo);
}

शून्य आगत::endFlowSequence() {
}

शून्य आगत::beginEnumScalar() {
  ScalarMatchFound = असत्य;
}

तार्किक आगत::matchEnumScalar(StringRef Str, तार्किक) {
  यदि (ScalarMatchFound)
    फर्काऊ असत्य;
  यदि (ScalarHNode *SN = dyn_cast<ScalarHNode>(Current_गाँठो)) {
    यदि (SN->मान_2() == Str) {
      ScalarMatchFound = सत्य;
      फर्काऊ सत्य;
    }
  }
  फर्काऊ असत्य;
}

तार्किक आगत::matchEnumFallback() {
  यदि (ScalarMatchFound)
    फर्काऊ असत्य;
  ScalarMatchFound = सत्य;
  फर्काऊ सत्य;
}

शून्य आगत::endEnumScalar() {
  यदि (!ScalarMatchFound) {
    setError(Current_गाँठो, "unknown enumerated scalar");
  }
}

तार्किक आगत::beginBitSetScalar(तार्किक &DoClear) {
  Bit_मानहरू_Used.clear();
  यदि (SequenceHNode *SQ = dyn_cast<SequenceHNode>(Current_गाँठो)) {
    Bit_मानहरू_Used.resize(SQ->Entries.size());
  } अन्यथा {
    setError(Current_गाँठो, "expected sequence of bit values");
  }
  DoClear = सत्य;
  फर्काऊ सत्य;
}

तार्किक आगत::bitSetMatch(StringRef Str, तार्किक) {
  यदि (EC)
    फर्काऊ असत्य;
  यदि (SequenceHNode *SQ = dyn_cast<SequenceHNode>(Current_गाँठो)) {
    अचिन्हित Index = 0;
    लागि (स्वतः &N : SQ->Entries) {
      यदि (ScalarHNode *SN = dyn_cast<ScalarHNode>(N)) {
        यदि (SN->मान_2() == Str) {
          Bit_मानहरू_Used[Index] = सत्य;
          फर्काऊ सत्य;
        }
      } अन्यथा {
        setError(Current_गाँठो, "unexpected scalar in sequence of bit values");
      }
      ++Index;
    }
  } अन्यथा {
    setError(Current_गाँठो, "expected sequence of bit values");
  }
  फर्काऊ असत्य;
}

शून्य आगत::endBitSetScalar() {
  यदि (EC)
    फर्काऊ;
  यदि (SequenceHNode *SQ = dyn_cast<SequenceHNode>(Current_गाँठो)) {
    प्रतिज्ञा(Bit_मानहरू_Used.size() == SQ->Entries.size());
    लागि (अचिन्हित i = 0; i < SQ->Entries.size(); ++i) {
      यदि (!Bit_मानहरू_Used[i]) {
        setError(SQ->Entries[i], "unknown bit value");
        फर्काऊ;
      }
    }
  }
}

शून्य आगत::scalarString(StringRef &S, QuotingType) {
  यदि (ScalarHNode *SN = dyn_cast<ScalarHNode>(Current_गाँठो)) {
    S = SN->मान_2();
  } अन्यथा {
    setError(Current_गाँठो, "unexpected scalar");
  }
}

शून्य आगत::blockScalarString(StringRef &S) { scalarString(S, QuotingType::None); }

शून्य आगत::scalarTag(std::string &Tag) {
  Tag = Current_गाँठो->गाँठो_2->getVerbatimTag();
}

शून्य आगत::setError(HNode *hnode, अचर Twine &message) {
  प्रतिज्ञा(hnode && "HNode must not be NULL");
  setError(hnode->गाँठो_2, message);
}

गाँठो_Kind आगत::get_गाँठो_Kind() {
  यदि (isa<ScalarHNode>(Current_गाँठो))
    फर्काऊ गाँठो_Kind::Scalar;
  अन्यथा यदि (isa<MapHNode>(Current_गाँठो))
    फर्काऊ गाँठो_Kind::Map;
  अन्यथा यदि (isa<SequenceHNode>(Current_गाँठो))
    फर्काऊ गाँठो_Kind::Sequence;
  llvm_unreachable("Unsupported node kind");
}

शून्य आगत::setError(गाँठो *गाँठो_3, अचर Twine &message) {
  Strm->printError(गाँठो_3, message);
  EC = make_error_code(errc::invalid_argument);
}

शून्य आगत::setError(अचर SMRange &range, अचर Twine &message) {
  Strm->printError(range, message);
  EC = make_error_code(errc::invalid_argument);
}

शून्य आगत::reportWarning(HNode *hnode, अचर Twine &message) {
  प्रतिज्ञा(hnode && "HNode must not be NULL");
  Strm->printError(hnode->गाँठो_2, message, SourceMgr::DK_Warning);
}

शून्य आगत::reportWarning(गाँठो *गाँठो_3, अचर Twine &message) {
  Strm->printError(गाँठो_3, message, SourceMgr::DK_Warning);
}

शून्य आगत::reportWarning(अचर SMRange &range, अचर Twine &message) {
  Strm->printError(range, message, SourceMgr::DK_Warning);
}

शून्य आगत::releaseHNodeBuffers() {
  EmptyHNodeAllocator.DestroyAll();
  ScalarHNodeAllocator.DestroyAll();
  SequenceHNodeAllocator.DestroyAll();
  MapHNodeAllocator.DestroyAll();
}

शून्य आगत::saveAliasHNode(गाँठो *N, HNode *HN) {
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

आगत::HNode *आगत::createHNodes(गाँठो *N) {
  SmallString<128> StringStorage;
  विकल्प (N->getType()) {
  पक्ष गाँठो::NK_Scalar: {
    Scalar_गाँठो *SN = dyn_cast<Scalar_गाँठो>(N);
    StringRef साँचो_Str = SN->get_मान(StringStorage);
    यदि (!StringStorage.empty()) {
      // Copy string to permanent storage
      साँचो_Str = StringStorage.str().copy(StringAllocator);
    }
    स्वतः *SHNode = नूतन (ScalarHNodeAllocator.Allocate()) ScalarHNode(N, साँचो_Str);
    saveAliasHNode(SN, SHNode);
    फर्काऊ SHNode;
  }
  पक्ष गाँठो::NK_BlockScalar: {
    Block_Scalar_गाँठो *BSN = dyn_cast<Block_Scalar_गाँठो>(N);
    StringRef मान_Copy = BSN->get_मान().copy(StringAllocator);
    स्वतः *BSHNode =
        नूतन (ScalarHNodeAllocator.Allocate()) ScalarHNode(N, मान_Copy);
    saveAliasHNode(BSN, BSHNode);
    फर्काऊ BSHNode;
  }
  पक्ष गाँठो::NK_Sequence: {
    Sequence_गाँठो *SQ = dyn_cast<Sequence_गाँठो>(N);
    स्वतः SQHNode = नूतन (SequenceHNodeAllocator.Allocate()) SequenceHNode(N);
    लागि (गाँठो &SN : *SQ) {
      स्वतः Entry = createHNodes(&SN);
      यदि (EC)
        रोक;
      SQHNode->Entries.थप_back(Entry);
    }
    saveAliasHNode(SQ, SQHNode);
    फर्काऊ SQHNode;
  }
  पक्ष गाँठो::NK_Mapping: {
    Mapping_गाँठो *Map = dyn_cast<Mapping_गाँठो>(N);
    स्वतः mapHNode = नूतन (MapHNodeAllocator.Allocate()) MapHNode(N);
    लागि (साँचो_मान_गाँठो &KVN : *Map) {
      गाँठो *साँचो_गाँठो = KVN.get_साँचो();
      Scalar_गाँठो *साँचो = dyn_cast_if_present<Scalar_गाँठो>(साँचो_गाँठो);
      गाँठो *मान = KVN.get_मान();
      यदि (!साँचो || !मान) {
        यदि (!साँचो)
          setError(साँचो_गाँठो, "Map key must be a scalar");
        यदि (!मान)
          setError(साँचो_गाँठो, "Map value must not be empty");
        रोक;
      }
      StringStorage.clear();
      StringRef साँचो_Str = साँचो->get_मान(StringStorage);
      यदि (!StringStorage.empty()) {
        // Copy string to permanent storage
        साँचो_Str = StringStorage.str().copy(StringAllocator);
      }
      यदि (mapHNode->Mapping.गन्ती(साँचो_Str))
        // From YAML spec: "The content of a mapping node is an unordered set of
        // key/value node pairs, with the restriction that each of the keys is
        // unique."
        setError(साँचो_गाँठो, Twine("duplicated mapping key '") + साँचो_Str + "'");
      स्वतः मान_HNode = createHNodes(मान);
      यदि (EC)
        रोक;
      mapHNode->Mapping[साँचो_Str] =
          std::make_pair(std::move(मान_HNode), साँचो_गाँठो->getSourceRange());
    }
    saveAliasHNode(Map, mapHNode);
    फर्काऊ std::move(mapHNode);
  }
  पक्ष गाँठो::NK_Null:
    // TODO: Anchor is not set for NullNode in the parser. Update the parser to
    // faithfully preserve anchors.
    फर्काऊ नूतन (EmptyHNodeAllocator.Allocate()) EmptyHNode(N);
  पक्ष गाँठो::NK_Alias: {
    Alias_गाँठो *AN = dyn_cast<Alias_गाँठो>(N);
    स्वतः AliasName = AN->getName();
    स्वतः AHN = AliasMap.find(AliasName);
    यदि (AHN == AliasMap.end()) {
      setError(AN, Twine("undefined alias '" + AliasName + "'"));
      फर्काऊ nullptr;
    }
    फर्काऊ AHN->second;
  }
  स्वभाव:
    setError(N, "unknown node kind");
    फर्काऊ nullptr;
  }
}

शून्य आगत::setError(अचर Twine &Message) {
  setError(Current_गाँठो, Message);
}

शून्य आगत::set_Allow_Unknown_साँचोहरू(तार्किक Allow) { Allow_Unknown_साँचोहरू = Allow; }

तार्किक आगत::canElideEmptySequence() {
  फर्काऊ असत्य;
}

//===----------------------------------------------------------------------===//
//  Output
//===----------------------------------------------------------------------===//

Output::Output(raw_ostream &yout, शून्य *context, पूर्णाङ्क WrapColumn)
    : IO(context), Out(yout), WrapColumn(WrapColumn) {}

Output::~Output() = स्वभाव;

तार्किक Output::outputting() अचर {
  फर्काऊ सत्य;
}

शून्य Output::beginMapping() {
  State_थाक.थप_back(in_Map_First_साँचो);
  PaddingBeforeContainer = Padding;
  Padding = "\n";
}

तार्किक Output::mapTag(StringRef Tag, तार्किक Use) {
  यदि (Use) {
    // If this tag is being written inside a sequence we should write the start
    // of the sequence before writing the tag, otherwise the tag won't be
    // attached to the element in the sequence, but rather the sequence itself.
    तार्किक SequenceElement = असत्य;
    यदि (State_थाक.size() > 1) {
      स्वतः &E = State_थाक[State_थाक.size() - 2];
      SequenceElement = inSeqAnyElement(E) || inFlowSeqAnyElement(E);
    }
    यदि (SequenceElement && State_थाक.back() == in_Map_First_साँचो) {
      new_पङ्क्ति_Check();
    } अन्यथा {
      output(" ");
    }
    output(Tag);
    यदि (SequenceElement) {
      // If we're writing the tag during the first element of a map, the tag
      // takes the place of the first element in the sequence.
      यदि (State_थाक.back() == in_Map_First_साँचो) {
        State_थाक.झिक_back();
        State_थाक.थप_back(in_Map_Other_साँचो);
      }
      // Tags inside maps in sequences should act as keys in the map from a
      // formatting perspective, so we always want a newline in a sequence.
      Padding = "\n";
    }
  }
  फर्काऊ Use;
}

शून्य Output::endMapping() {
  // If we did not map anything, we should explicitly emit an empty map
  यदि (State_थाक.back() == in_Map_First_साँचो) {
    Padding = PaddingBeforeContainer;
    new_पङ्क्ति_Check();
    output("{}");
    Padding = "\n";
  }
  State_थाक.झिक_back();
}

std::vector<StringRef> Output::साँचोहरू() {
  report_fatal_error("invalid call");
}

तार्किक Output::preflight_साँचो(StringRef साँचो, तार्किक Required, तार्किक SameAsDefault,
                          तार्किक &UseDefault, शून्य *&SaveInfo) {
  UseDefault = असत्य;
  SaveInfo = nullptr;
  यदि (Required || !SameAsDefault || Write_Default_मानहरू) {
    स्वतः State = State_थाक.back();
    यदि (State == in_Flow_Map_First_साँचो || State == in_Flow_Map_Other_साँचो) {
      flow_साँचो(साँचो);
    } अन्यथा {
      new_पङ्क्ति_Check();
      padded_साँचो(साँचो);
    }
    फर्काऊ सत्य;
  }
  फर्काऊ असत्य;
}

शून्य Output::postflight_साँचो(शून्य *) {
  यदि (State_थाक.back() == in_Map_First_साँचो) {
    State_थाक.झिक_back();
    State_थाक.थप_back(in_Map_Other_साँचो);
  } अन्यथा यदि (State_थाक.back() == in_Flow_Map_First_साँचो) {
    State_थाक.झिक_back();
    State_थाक.थप_back(in_Flow_Map_Other_साँचो);
  }
}

शून्य Output::beginFlowMapping() {
  State_थाक.थप_back(in_Flow_Map_First_साँचो);
  new_पङ्क्ति_Check();
  ColumnAtMapFlowStart = Column;
  output("{ ");
}

शून्य Output::endFlowMapping() {
  State_थाक.झिक_back();
  output_Up_To_End_Of_पङ्क्ति(" }");
}

शून्य Output::beginDocuments() {
  output_Up_To_End_Of_पङ्क्ति("---");
}

तार्किक Output::preflightDocument(अचिन्हित index) {
  यदि (index > 0)
    output_Up_To_End_Of_पङ्क्ति("\n---");
  फर्काऊ सत्य;
}

शून्य Output::postflightDocument() {
}

शून्य Output::endDocuments() {
  output("\n...\n");
}

अचिन्हित Output::beginSequence() {
  State_थाक.थप_back(inSeqFirstElement);
  PaddingBeforeContainer = Padding;
  Padding = "\n";
  फर्काऊ 0;
}

शून्य Output::endSequence() {
  // If we did not emit anything, we should explicitly emit an empty sequence
  यदि (State_थाक.back() == inSeqFirstElement) {
    Padding = PaddingBeforeContainer;
    new_पङ्क्ति_Check(/*EmptySequence=*/सत्य);
    output("[]");
    Padding = "\n";
  }
  State_थाक.झिक_back();
}

तार्किक Output::preflightElement(अचिन्हित, शून्य *&SaveInfo) {
  SaveInfo = nullptr;
  फर्काऊ सत्य;
}

शून्य Output::postflightElement(शून्य *) {
  यदि (State_थाक.back() == inSeqFirstElement) {
    State_थाक.झिक_back();
    State_थाक.थप_back(inSeqOtherElement);
  } अन्यथा यदि (State_थाक.back() == inFlowSeqFirstElement) {
    State_थाक.झिक_back();
    State_थाक.थप_back(inFlowSeqOtherElement);
  }
}

अचिन्हित Output::beginFlowSequence() {
  State_थाक.थप_back(inFlowSeqFirstElement);
  new_पङ्क्ति_Check();
  ColumnAtFlowStart = Column;
  output("[ ");
  NeedFlowSequenceComma = असत्य;
  फर्काऊ 0;
}

शून्य Output::endFlowSequence() {
  State_थाक.झिक_back();
  output_Up_To_End_Of_पङ्क्ति(" ]");
}

तार्किक Output::preflightFlowElement(अचिन्हित, शून्य *&SaveInfo) {
  यदि (NeedFlowSequenceComma)
    output(", ");
  यदि (WrapColumn && Column > WrapColumn) {
    output("\n");
    लागि (पूर्णाङ्क i = 0; i < ColumnAtFlowStart; ++i)
      output(" ");
    Column = ColumnAtFlowStart;
    output("  ");
  }
  SaveInfo = nullptr;
  फर्काऊ सत्य;
}

शून्य Output::postflightFlowElement(शून्य *) {
  NeedFlowSequenceComma = सत्य;
}

शून्य Output::beginEnumScalar() {
  EnumerationMatchFound = असत्य;
}

तार्किक Output::matchEnumScalar(StringRef Str, तार्किक Match) {
  यदि (Match && !EnumerationMatchFound) {
    new_पङ्क्ति_Check();
    output_Up_To_End_Of_पङ्क्ति(Str);
    EnumerationMatchFound = सत्य;
  }
  फर्काऊ असत्य;
}

तार्किक Output::matchEnumFallback() {
  यदि (EnumerationMatchFound)
    फर्काऊ असत्य;
  EnumerationMatchFound = सत्य;
  फर्काऊ सत्य;
}

शून्य Output::endEnumScalar() {
  यदि (!EnumerationMatchFound)
    llvm_unreachable("bad runtime enum value");
}

तार्किक Output::beginBitSetScalar(तार्किक &DoClear) {
  new_पङ्क्ति_Check();
  output("[ ");
  Need_Bit_मान_Comma = असत्य;
  DoClear = असत्य;
  फर्काऊ सत्य;
}

तार्किक Output::bitSetMatch(StringRef Str, तार्किक Matches) {
  यदि (Matches) {
    यदि (Need_Bit_मान_Comma)
      output(", ");
    output(Str);
    Need_Bit_मान_Comma = सत्य;
  }
  फर्काऊ असत्य;
}

शून्य Output::endBitSetScalar() {
  output_Up_To_End_Of_पङ्क्ति(" ]");
}

शून्य Output::scalarString(StringRef &S, QuotingType MustQuote) {
  new_पङ्क्ति_Check();
  यदि (S.empty()) {
    // Print '' for the empty string because leaving the field empty is not
    // allowed.
    output_Up_To_End_Of_पङ्क्ति("''");
    फर्काऊ;
  }
  output(S, MustQuote);
  output_Up_To_End_Of_पङ्क्ति("");
}

शून्य Output::blockScalarString(StringRef &S) {
  यदि (!State_थाक.empty())
    new_पङ्क्ति_Check();
  output(" |");

  अचिन्हित Indent = State_थाक.empty() ? 1 : State_थाक.size();

  स्वतः Buffer = MemoryBuffer::getMemBuffer(S, "", असत्य);
  लागि (पङ्क्ति_iterator पङ्क्तिहरू(*Buffer, असत्य); !पङ्क्तिहरू.is_at_end(); ++पङ्क्तिहरू) {
    output_New_पङ्क्ति();
    लागि (अचिन्हित I = 0; I < Indent; ++I) {
      output("  ");
    }
    output(*पङ्क्तिहरू);
  }
  output_Up_To_End_Of_पङ्क्ति("");
}

शून्य Output::scalarTag(std::string &Tag) {
  यदि (Tag.empty())
    फर्काऊ;
  new_पङ्क्ति_Check();
  output(Tag);
  output(" ");
}

शून्य Output::setError(अचर Twine &message) {
}

std::error_code Output::error() { फर्काऊ {}; }

तार्किक Output::canElideEmptySequence() {
  // Normally, with an optional key/value where the value is an empty sequence,
  // the whole key/value can be not written.  But, that produces wrong yaml
  // if the key/value is the only thing in the map and the map is used in
  // a sequence.  This detects if the this sequence is the first key/value
  // in map that itself is embedded in a sequence.
  यदि (State_थाक.size() < 2)
    फर्काऊ सत्य;
  यदि (State_थाक.back() != in_Map_First_साँचो)
    फर्काऊ सत्य;
  फर्काऊ !inSeqAnyElement(State_थाक[State_थाक.size() - 2]);
}

शून्य Output::output(StringRef s) {
  Column += s.size();
  Out << s;
}

शून्य Output::output(StringRef S, QuotingType MustQuote) {
  यदि (MustQuote == QuotingType::None) {
    // Only quote if we must.
    output(S);
    फर्काऊ;
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
    फर्काऊ;
  }

  अचिन्हित i = 0;
  अचिन्हित j = 0;
  अचिन्हित End = S.size();
  अचर वर्ण *Base = S.data();

  // When using single-quoted strings, any single quote ' must be doubled to be
  // escaped.
  जबसम्म (j < End) {
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

शून्य Output::output_Up_To_End_Of_पङ्क्ति(StringRef s) {
  output(s);
  यदि (State_थाक.empty() || (!inFlowSeqAnyElement(State_थाक.back()) &&
                             !in_Flow_Map_Any_साँचो(State_थाक.back())))
    Padding = "\n";
}

शून्य Output::output_New_पङ्क्ति() {
  Out << "\n";
  Column = 0;
}

// if seq at top, indent as if map, then add "- "
// if seq in middle, use "- " if firstKey, else use "  "
//

शून्य Output::new_पङ्क्ति_Check(तार्किक EmptySequence) {
  यदि (Padding != "\n") {
    output(Padding);
    Padding = {};
    फर्काऊ;
  }
  output_New_पङ्क्ति();
  Padding = {};

  यदि (State_थाक.size() == 0 || EmptySequence)
    फर्काऊ;

  अचिन्हित Indent = State_थाक.size() - 1;
  तार्किक PossiblyNestedSeq = असत्य;
  स्वतः I = State_थाक.rbegin(), E = State_थाक.rend();

  यदि (inSeqAnyElement(*I)) {
    PossiblyNestedSeq = सत्य; // Not possibly but always.
    ++Indent;
  } अन्यथा यदि (*I == in_Map_First_साँचो || *I == in_Flow_Map_First_साँचो ||
             inFlowSeqAnyElement(*I)) {
    PossiblyNestedSeq = सत्य;
    ++I; // Skip back().
  }

  अचिन्हित Output_Dash_गन्ती = 0;
  यदि (PossiblyNestedSeq) {
    // Count up consecutive inSeqFirstElement from the end, unless
    // inSeqFirstElement is the top of nested sequence.
    जबसम्म (I != E) {
      // Don't count the top of nested sequence.
      यदि (!inSeqAnyElement(*I))
        रोक;

      ++Output_Dash_गन्ती;

      // Stop counting if consecutive inSeqFirstElement ends.
      यदि (*I++ != inSeqFirstElement)
        रोक;
    }
  }

  लागि (अचिन्हित I = Output_Dash_गन्ती; I < Indent; ++I)
    output("  ");

  लागि (अचिन्हित I = 0; I < Output_Dash_गन्ती; ++I)
    output("- ");
}

शून्य Output::padded_साँचो(StringRef साँचो_2) {
  output(साँचो_2, needsQuotes(साँचो_2, असत्य));
  output(":");
  अचर वर्ण *spaces = "                ";
  यदि (साँचो_2.size() < म_माप(spaces))
    Padding = &spaces[साँचो_2.size()];
  अन्यथा
    Padding = " ";
}

शून्य Output::flow_साँचो(StringRef साँचो) {
  यदि (State_थाक.back() == in_Flow_Map_Other_साँचो)
    output(", ");
  यदि (WrapColumn && Column > WrapColumn) {
    output("\n");
    लागि (पूर्णाङ्क I = 0; I < ColumnAtMapFlowStart; ++I)
      output(" ");
    Column = ColumnAtMapFlowStart;
    output("  ");
  }
  output(साँचो, needsQuotes(साँचो, असत्य));
  output(": ");
}

गाँठो_Kind Output::get_गाँठो_Kind() { report_fatal_error("invalid call"); }

तार्किक Output::inSeqAnyElement(InState State) {
  फर्काऊ State == inSeqFirstElement || State == inSeqOtherElement;
}

तार्किक Output::inFlowSeqAnyElement(InState State) {
  फर्काऊ State == inFlowSeqFirstElement || State == inFlowSeqOtherElement;
}

तार्किक Output::in_Map_Any_साँचो(InState State) {
  फर्काऊ State == in_Map_First_साँचो || State == in_Map_Other_साँचो;
}

तार्किक Output::in_Flow_Map_Any_साँचो(InState State) {
  फर्काऊ State == in_Flow_Map_First_साँचो || State == in_Flow_Map_Other_साँचो;
}

//===----------------------------------------------------------------------===//
//  traits for built-in types
//===----------------------------------------------------------------------===//

शून्य ScalarTraits<तार्किक>::output(अचर तार्किक &Val, शून्य *, raw_ostream &Out) {
  Out << (Val ? "true" : "false");
}

StringRef ScalarTraits<तार्किक>::आगत_2(StringRef Scalar, शून्य *, तार्किक &Val) {
  यदि (std::optional<तार्किक> Parsed = parseBool(Scalar)) {
    Val = *Parsed;
    फर्काऊ StringRef();
  }
  फर्काऊ "invalid boolean";
}

शून्य ScalarTraits<StringRef>::output(अचर StringRef &Val, शून्य *,
                                     raw_ostream &Out) {
  Out << Val;
}

StringRef ScalarTraits<StringRef>::आगत_2(StringRef Scalar, शून्य *,
                                         StringRef &Val) {
  Val = Scalar;
  फर्काऊ StringRef();
}

शून्य ScalarTraits<std::string>::output(अचर std::string &Val, शून्य *,
                                       raw_ostream &Out) {
  Out << Val;
}

StringRef ScalarTraits<std::string>::आगत_2(StringRef Scalar, शून्य *,
                                           std::string &Val) {
  Val = Scalar.str();
  फर्काऊ StringRef();
}

शून्य ScalarTraits<uint8_t>::output(अचर uint8_t &Val, शून्य *,
                                   raw_ostream &Out) {
  // use temp uin32_t because ostream thinks uint8_t is a character
  uint32_t अङ्क = Val;
  Out << अङ्क;
}

StringRef ScalarTraits<uint8_t>::आगत_2(StringRef Scalar, शून्य *, uint8_t &Val) {
  अचिन्हित दीर्घ दीर्घ n;
  यदि (getAsUnsignedInteger(Scalar, 0, n))
    फर्काऊ "invalid number";
  यदि (n > 0xFF)
    फर्काऊ "out of range number";
  Val = n;
  फर्काऊ StringRef();
}

शून्य ScalarTraits<uint16_t>::output(अचर uint16_t &Val, शून्य *,
                                    raw_ostream &Out) {
  Out << Val;
}

StringRef ScalarTraits<uint16_t>::आगत_2(StringRef Scalar, शून्य *,
                                        uint16_t &Val) {
  अचिन्हित दीर्घ दीर्घ n;
  यदि (getAsUnsignedInteger(Scalar, 0, n))
    फर्काऊ "invalid number";
  यदि (n > 0xFFFF)
    फर्काऊ "out of range number";
  Val = n;
  फर्काऊ StringRef();
}

शून्य ScalarTraits<uint32_t>::output(अचर uint32_t &Val, शून्य *,
                                    raw_ostream &Out) {
  Out << Val;
}

StringRef ScalarTraits<uint32_t>::आगत_2(StringRef Scalar, शून्य *,
                                        uint32_t &Val) {
  अचिन्हित दीर्घ दीर्घ n;
  यदि (getAsUnsignedInteger(Scalar, 0, n))
    फर्काऊ "invalid number";
  यदि (n > 0xFFFFFFFFUL)
    फर्काऊ "out of range number";
  Val = n;
  फर्काऊ StringRef();
}

शून्य ScalarTraits<uint64_t>::output(अचर uint64_t &Val, शून्य *,
                                    raw_ostream &Out) {
  Out << Val;
}

StringRef ScalarTraits<uint64_t>::आगत_2(StringRef Scalar, शून्य *,
                                        uint64_t &Val) {
  अचिन्हित दीर्घ दीर्घ N;
  यदि (getAsUnsignedInteger(Scalar, 0, N))
    फर्काऊ "invalid number";
  Val = N;
  फर्काऊ StringRef();
}

शून्य ScalarTraits<int8_t>::output(अचर int8_t &Val, शून्य *, raw_ostream &Out) {
  // use temp in32_t because ostream thinks int8_t is a character
  int32_t अङ्क = Val;
  Out << अङ्क;
}

StringRef ScalarTraits<int8_t>::आगत_2(StringRef Scalar, शून्य *, int8_t &Val) {
  दीर्घ दीर्घ N;
  यदि (getAsSignedInteger(Scalar, 0, N))
    फर्काऊ "invalid number";
  यदि ((N > 127) || (N < -128))
    फर्काऊ "out of range number";
  Val = N;
  फर्काऊ StringRef();
}

शून्य ScalarTraits<int16_t>::output(अचर int16_t &Val, शून्य *,
                                   raw_ostream &Out) {
  Out << Val;
}

StringRef ScalarTraits<int16_t>::आगत_2(StringRef Scalar, शून्य *, int16_t &Val) {
  दीर्घ दीर्घ N;
  यदि (getAsSignedInteger(Scalar, 0, N))
    फर्काऊ "invalid number";
  यदि ((N > INT16_MAX) || (N < INT16_MIN))
    फर्काऊ "out of range number";
  Val = N;
  फर्काऊ StringRef();
}

शून्य ScalarTraits<int32_t>::output(अचर int32_t &Val, शून्य *,
                                   raw_ostream &Out) {
  Out << Val;
}

StringRef ScalarTraits<int32_t>::आगत_2(StringRef Scalar, शून्य *, int32_t &Val) {
  दीर्घ दीर्घ N;
  यदि (getAsSignedInteger(Scalar, 0, N))
    फर्काऊ "invalid number";
  यदि ((N > INT32_MAX) || (N < INT32_MIN))
    फर्काऊ "out of range number";
  Val = N;
  फर्काऊ StringRef();
}

शून्य ScalarTraits<int64_t>::output(अचर int64_t &Val, शून्य *,
                                   raw_ostream &Out) {
  Out << Val;
}

StringRef ScalarTraits<int64_t>::आगत_2(StringRef Scalar, शून्य *, int64_t &Val) {
  दीर्घ दीर्घ N;
  यदि (getAsSignedInteger(Scalar, 0, N))
    फर्काऊ "invalid number";
  Val = N;
  फर्काऊ StringRef();
}

शून्य ScalarTraits<द्विगुण>::output(अचर द्विगुण &Val, शून्य *, raw_ostream &Out) {
  Out << format("%g", Val);
}

StringRef ScalarTraits<द्विगुण>::आगत_2(StringRef Scalar, शून्य *, द्विगुण &Val) {
  यदि (to_float(Scalar, Val))
    फर्काऊ StringRef();
  फर्काऊ "invalid floating point number";
}

शून्य ScalarTraits<भिन्न>::output(अचर भिन्न &Val, शून्य *, raw_ostream &Out) {
  Out << format("%g", Val);
}

StringRef ScalarTraits<भिन्न>::आगत_2(StringRef Scalar, शून्य *, भिन्न &Val) {
  यदि (to_float(Scalar, Val))
    फर्काऊ StringRef();
  फर्काऊ "invalid floating point number";
}

शून्य ScalarTraits<Hex8>::output(अचर Hex8 &Val, शून्य *, raw_ostream &Out) {
  Out << format("0x%" PRIX8, (uint8_t)Val);
}

StringRef ScalarTraits<Hex8>::आगत_2(StringRef Scalar, शून्य *, Hex8 &Val) {
  अचिन्हित दीर्घ दीर्घ n;
  यदि (getAsUnsignedInteger(Scalar, 0, n))
    फर्काऊ "invalid hex8 number";
  यदि (n > 0xFF)
    फर्काऊ "out of range hex8 number";
  Val = n;
  फर्काऊ StringRef();
}

शून्य ScalarTraits<Hex16>::output(अचर Hex16 &Val, शून्य *, raw_ostream &Out) {
  Out << format("0x%" PRIX16, (uint16_t)Val);
}

StringRef ScalarTraits<Hex16>::आगत_2(StringRef Scalar, शून्य *, Hex16 &Val) {
  अचिन्हित दीर्घ दीर्घ n;
  यदि (getAsUnsignedInteger(Scalar, 0, n))
    फर्काऊ "invalid hex16 number";
  यदि (n > 0xFFFF)
    फर्काऊ "out of range hex16 number";
  Val = n;
  फर्काऊ StringRef();
}

शून्य ScalarTraits<Hex32>::output(अचर Hex32 &Val, शून्य *, raw_ostream &Out) {
  Out << format("0x%" PRIX32, (uint32_t)Val);
}

StringRef ScalarTraits<Hex32>::आगत_2(StringRef Scalar, शून्य *, Hex32 &Val) {
  अचिन्हित दीर्घ दीर्घ n;
  यदि (getAsUnsignedInteger(Scalar, 0, n))
    फर्काऊ "invalid hex32 number";
  यदि (n > 0xFFFFFFFFUL)
    फर्काऊ "out of range hex32 number";
  Val = n;
  फर्काऊ StringRef();
}

शून्य ScalarTraits<Hex64>::output(अचर Hex64 &Val, शून्य *, raw_ostream &Out) {
  Out << format("0x%" PRIX64, (uint64_t)Val);
}

StringRef ScalarTraits<Hex64>::आगत_2(StringRef Scalar, शून्य *, Hex64 &Val) {
  अचिन्हित दीर्घ दीर्घ अङ्क;
  यदि (getAsUnsignedInteger(Scalar, 0, अङ्क))
    फर्काऊ "invalid hex64 number";
  Val = अङ्क;
  फर्काऊ StringRef();
}

शून्य ScalarTraits<VersionTuple>::output(अचर VersionTuple &Val, शून्य *,
                                        llvm::raw_ostream &Out) {
  Out << Val.getAsString();
}

StringRef ScalarTraits<VersionTuple>::आगत_2(StringRef Scalar, शून्य *,
                                            VersionTuple &Val) {
  यदि (Val.tryParse(Scalar))
    फर्काऊ "invalid version format";
  फर्काऊ StringRef();
}
