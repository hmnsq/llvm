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

IO::~IO() = पूर्वनिर्धारित;

शून्य *IO::getContext() स्थिरांक {
  परत Ctxt;
}

शून्य IO::setContext(शून्य *Context) {
  Ctxt = Context;
}

शून्य IO::set_Allow_Unknown_किल्ल्या(तार्किक Allow) {
  llvm_unreachable("Only supported for Input");
}

//===----------------------------------------------------------------------===//
//आदान
//===----------------------------------------------------------------------===//

आदान::आदान(StringRef आदान_Content, शून्य *Ctxt,
             SourceMgr::DiagHandlerTy DiagHandler, शून्य *DiagHandlerCtxt)
    : IO(Ctxt), Strm(नवीन Stream(आदान_Content, SrcMgr, असत्य, &EC)) {
  जर (DiagHandler)
    SrcMgr.setDiagHandler(DiagHandler, DiagHandlerCtxt);
  DocIterator = Strm->begin();
}

आदान::आदान(MemoryBufferRef आदान, शून्य *Ctxt,
             SourceMgr::DiagHandlerTy DiagHandler, शून्य *DiagHandlerCtxt)
    : IO(Ctxt), Strm(नवीन Stream(आदान, SrcMgr, असत्य, &EC)) {
  जर (DiagHandler)
    SrcMgr.setDiagHandler(DiagHandler, DiagHandlerCtxt);
  DocIterator = Strm->begin();
}

आदान::~आदान() = पूर्वनिर्धारित;

std::error_code आदान::error() { परत EC; }

तार्किक आदान::outputting() स्थिरांक {
  परत असत्य;
}

तार्किक आदान::setCurrentDocument() {
  जर (DocIterator != Strm->end()) {
    गाठ *N = DocIterator->get_मूळ();
    जर (!N) {
      EC = make_error_code(errc::invalid_argument);
      परत असत्य;
    }

    जर (isa<Null_गाठ>(N)) {
      // Empty files are allowed and ignored
      ++DocIterator;
      परत setCurrentDocument();
    }
    releaseHNodeBuffers();
    शीर्ष_गाठ = createHNodes(N);
    Current_गाठ = शीर्ष_गाठ;
    परत सत्य;
  }
  परत असत्य;
}

तार्किक आदान::nextDocument() {
  परत ++DocIterator != Strm->end();
}

स्थिरांक गाठ *आदान::get_Current_गाठ() स्थिरांक {
  परत Current_गाठ ? Current_गाठ->गाठ_2 : nullptr;
}

तार्किक आदान::mapTag(StringRef Tag, तार्किक Default) {
  // CurrentNode can be null if setCurrentDocument() was unable to
  // parse the document because it was invalid or empty.
  जर (!Current_गाठ)
    परत असत्य;

  std::string foundTag = Current_गाठ->गाठ_2->getVerbatimTag();
  जर (foundTag.empty()) {
    // If no tag found and 'Tag' is the default, say it was found.
    परत Default;
  }
  // Return true iff found tag matches supplied tag.
  परत Tag == foundTag;
}

शून्य आदान::beginMapping() {
  जर (EC)
    परत;
  // CurrentNode can be null if the document is empty.
  MapHNode *MN = dyn_cast_or_null<MapHNode>(Current_गाठ);
  जर (MN) {
    MN->Valid_किल्ल्या.clear();
  }
}

std::vector<StringRef> आदान::किल्ल्या() {
  MapHNode *MN = dyn_cast<MapHNode>(Current_गाठ);
  std::vector<StringRef> Ret;
  जर (!MN) {
    setError(Current_गाठ, "not a mapping");
    परत Ret;
  }
  साठी (स्वतः &P : MN->Mapping)
    Ret.ढकला_back(P.first());
  परत Ret;
}

तार्किक आदान::preflight_किल्ली(StringRef किल्ली, तार्किक Required, तार्किक, तार्किक &UseDefault,
                         शून्य *&SaveInfo) {
  UseDefault = असत्य;
  जर (EC)
    परत असत्य;

  // CurrentNode is null for empty documents, which is an error in case required
  // nodes are present.
  जर (!Current_गाठ) {
    जर (Required)
      EC = make_error_code(errc::invalid_argument);
    अन्यथा
      UseDefault = सत्य;
    परत असत्य;
  }

  MapHNode *MN = dyn_cast<MapHNode>(Current_गाठ);
  जर (!MN) {
    जर (Required || !isa<EmptyHNode>(Current_गाठ))
      setError(Current_गाठ, "not a mapping");
    अन्यथा
      UseDefault = सत्य;
    परत असत्य;
  }
  MN->Valid_किल्ल्या.ढकला_back(किल्ली.str());
  HNode *मूल्य = MN->Mapping[किल्ली].first;
  जर (!मूल्य) {
    जर (Required)
      setError(Current_गाठ, Twine("missing required key '") + किल्ली + "'");
    अन्यथा
      UseDefault = सत्य;
    परत असत्य;
  }
  SaveInfo = Current_गाठ;
  Current_गाठ = मूल्य;
  परत सत्य;
}

शून्य आदान::postflight_किल्ली(शून्य *saveInfo) {
  Current_गाठ = reinterpret_cast<HNode *>(saveInfo);
}

शून्य आदान::endMapping() {
  जर (EC)
    परत;
  // CurrentNode can be null if the document is empty.
  MapHNode *MN = dyn_cast_or_null<MapHNode>(Current_गाठ);
  जर (!MN)
    परत;
  साठी (स्थिरांक स्वतः &NN : MN->Mapping) {
    जर (!is_contained(MN->Valid_किल्ल्या, NN.first())) {
      स्थिरांक SMRange &ReportLoc = NN.second.second;
      जर (!Allow_Unknown_किल्ल्या) {
        setError(ReportLoc, Twine("unknown key '") + NN.first() + "'");
        थांबा;
      } अन्यथा
        reportWarning(ReportLoc, Twine("unknown key '") + NN.first() + "'");
    }
  }
}

शून्य आदान::beginFlowMapping() { beginMapping(); }

शून्य आदान::endFlowMapping() { endMapping(); }

अचिन्हित आदान::beginSequence() {
  जर (SequenceHNode *SQ = dyn_cast<SequenceHNode>(Current_गाठ))
    परत SQ->Entries.size();
  जर (isa<EmptyHNode>(Current_गाठ))
    परत 0;
  // Treat case where there's a scalar "null" value as an empty sequence.
  जर (ScalarHNode *SN = dyn_cast<ScalarHNode>(Current_गाठ)) {
    जर (isNull(SN->मूल्य_2()))
      परत 0;
  }
  // Any other type of HNode is an error.
  setError(Current_गाठ, "not a sequence");
  परत 0;
}

शून्य आदान::endSequence() {
}

तार्किक आदान::preflightElement(अचिन्हित Index, शून्य *&SaveInfo) {
  जर (EC)
    परत असत्य;
  जर (SequenceHNode *SQ = dyn_cast<SequenceHNode>(Current_गाठ)) {
    SaveInfo = Current_गाठ;
    Current_गाठ = SQ->Entries[Index];
    परत सत्य;
  }
  परत असत्य;
}

शून्य आदान::postflightElement(शून्य *SaveInfo) {
  Current_गाठ = reinterpret_cast<HNode *>(SaveInfo);
}

अचिन्हित आदान::beginFlowSequence() { परत beginSequence(); }

तार्किक आदान::preflightFlowElement(अचिन्हित index, शून्य *&SaveInfo) {
  जर (EC)
    परत असत्य;
  जर (SequenceHNode *SQ = dyn_cast<SequenceHNode>(Current_गाठ)) {
    SaveInfo = Current_गाठ;
    Current_गाठ = SQ->Entries[index];
    परत सत्य;
  }
  परत असत्य;
}

शून्य आदान::postflightFlowElement(शून्य *SaveInfo) {
  Current_गाठ = reinterpret_cast<HNode *>(SaveInfo);
}

शून्य आदान::endFlowSequence() {
}

शून्य आदान::beginEnumScalar() {
  ScalarMatchFound = असत्य;
}

तार्किक आदान::matchEnumScalar(StringRef Str, तार्किक) {
  जर (ScalarMatchFound)
    परत असत्य;
  जर (ScalarHNode *SN = dyn_cast<ScalarHNode>(Current_गाठ)) {
    जर (SN->मूल्य_2() == Str) {
      ScalarMatchFound = सत्य;
      परत सत्य;
    }
  }
  परत असत्य;
}

तार्किक आदान::matchEnumFallback() {
  जर (ScalarMatchFound)
    परत असत्य;
  ScalarMatchFound = सत्य;
  परत सत्य;
}

शून्य आदान::endEnumScalar() {
  जर (!ScalarMatchFound) {
    setError(Current_गाठ, "unknown enumerated scalar");
  }
}

तार्किक आदान::beginBitSetScalar(तार्किक &DoClear) {
  Bit_मूल्ये_Used.clear();
  जर (SequenceHNode *SQ = dyn_cast<SequenceHNode>(Current_गाठ)) {
    Bit_मूल्ये_Used.resize(SQ->Entries.size());
  } अन्यथा {
    setError(Current_गाठ, "expected sequence of bit values");
  }
  DoClear = सत्य;
  परत सत्य;
}

तार्किक आदान::bitSetMatch(StringRef Str, तार्किक) {
  जर (EC)
    परत असत्य;
  जर (SequenceHNode *SQ = dyn_cast<SequenceHNode>(Current_गाठ)) {
    अचिन्हित Index = 0;
    साठी (स्वतः &N : SQ->Entries) {
      जर (ScalarHNode *SN = dyn_cast<ScalarHNode>(N)) {
        जर (SN->मूल्य_2() == Str) {
          Bit_मूल्ये_Used[Index] = सत्य;
          परत सत्य;
        }
      } अन्यथा {
        setError(Current_गाठ, "unexpected scalar in sequence of bit values");
      }
      ++Index;
    }
  } अन्यथा {
    setError(Current_गाठ, "expected sequence of bit values");
  }
  परत असत्य;
}

शून्य आदान::endBitSetScalar() {
  जर (EC)
    परत;
  जर (SequenceHNode *SQ = dyn_cast<SequenceHNode>(Current_गाठ)) {
    प्रतिज्ञा(Bit_मूल्ये_Used.size() == SQ->Entries.size());
    साठी (अचिन्हित i = 0; i < SQ->Entries.size(); ++i) {
      जर (!Bit_मूल्ये_Used[i]) {
        setError(SQ->Entries[i], "unknown bit value");
        परत;
      }
    }
  }
}

शून्य आदान::scalarString(StringRef &S, QuotingType) {
  जर (ScalarHNode *SN = dyn_cast<ScalarHNode>(Current_गाठ)) {
    S = SN->मूल्य_2();
  } अन्यथा {
    setError(Current_गाठ, "unexpected scalar");
  }
}

शून्य आदान::blockScalarString(StringRef &S) { scalarString(S, QuotingType::None); }

शून्य आदान::scalarTag(std::string &Tag) {
  Tag = Current_गाठ->गाठ_2->getVerbatimTag();
}

शून्य आदान::setError(HNode *hnode, स्थिरांक Twine &message) {
  प्रतिज्ञा(hnode && "HNode must not be NULL");
  setError(hnode->गाठ_2, message);
}

गाठ_Kind आदान::get_गाठ_Kind() {
  जर (isa<ScalarHNode>(Current_गाठ))
    परत गाठ_Kind::Scalar;
  अन्यथा जर (isa<MapHNode>(Current_गाठ))
    परत गाठ_Kind::Map;
  अन्यथा जर (isa<SequenceHNode>(Current_गाठ))
    परत गाठ_Kind::Sequence;
  llvm_unreachable("Unsupported node kind");
}

शून्य आदान::setError(गाठ *गाठ_3, स्थिरांक Twine &message) {
  Strm->printError(गाठ_3, message);
  EC = make_error_code(errc::invalid_argument);
}

शून्य आदान::setError(स्थिरांक SMRange &range, स्थिरांक Twine &message) {
  Strm->printError(range, message);
  EC = make_error_code(errc::invalid_argument);
}

शून्य आदान::reportWarning(HNode *hnode, स्थिरांक Twine &message) {
  प्रतिज्ञा(hnode && "HNode must not be NULL");
  Strm->printError(hnode->गाठ_2, message, SourceMgr::DK_Warning);
}

शून्य आदान::reportWarning(गाठ *गाठ_3, स्थिरांक Twine &message) {
  Strm->printError(गाठ_3, message, SourceMgr::DK_Warning);
}

शून्य आदान::reportWarning(स्थिरांक SMRange &range, स्थिरांक Twine &message) {
  Strm->printError(range, message, SourceMgr::DK_Warning);
}

शून्य आदान::releaseHNodeBuffers() {
  EmptyHNodeAllocator.DestroyAll();
  ScalarHNodeAllocator.DestroyAll();
  SequenceHNodeAllocator.DestroyAll();
  MapHNodeAllocator.DestroyAll();
}

शून्य आदान::saveAliasHNode(गाठ *N, HNode *HN) {
  StringRef Anchor = N->getAnchor();
  जर (!Anchor.empty())
    // YAML 1.2.2 - 3.2.2.2. Anchors and Aliases:
    //
    // An alias event refers to the most recent event in the serialization
    // having the specified anchor. Therefore, anchors need not be unique within
    // a serialization. In addition, an anchor need not have an alias node
    // referring to it.
    AliasMap[Anchor] = HN;
}

आदान::HNode *आदान::createHNodes(गाठ *N) {
  SmallString<128> StringStorage;
  निवड (N->getType()) {
  स्थिती गाठ::NK_Scalar: {
    Scalar_गाठ *SN = dyn_cast<Scalar_गाठ>(N);
    StringRef किल्ली_Str = SN->get_मूल्य(StringStorage);
    जर (!StringStorage.empty()) {
      // Copy string to permanent storage
      किल्ली_Str = StringStorage.str().copy(StringAllocator);
    }
    स्वतः *SHNode = नवीन (ScalarHNodeAllocator.Allocate()) ScalarHNode(N, किल्ली_Str);
    saveAliasHNode(SN, SHNode);
    परत SHNode;
  }
  स्थिती गाठ::NK_BlockScalar: {
    Block_Scalar_गाठ *BSN = dyn_cast<Block_Scalar_गाठ>(N);
    StringRef मूल्य_Copy = BSN->get_मूल्य().copy(StringAllocator);
    स्वतः *BSHNode =
        नवीन (ScalarHNodeAllocator.Allocate()) ScalarHNode(N, मूल्य_Copy);
    saveAliasHNode(BSN, BSHNode);
    परत BSHNode;
  }
  स्थिती गाठ::NK_Sequence: {
    Sequence_गाठ *SQ = dyn_cast<Sequence_गाठ>(N);
    स्वतः SQHNode = नवीन (SequenceHNodeAllocator.Allocate()) SequenceHNode(N);
    साठी (गाठ &SN : *SQ) {
      स्वतः Entry = createHNodes(&SN);
      जर (EC)
        थांबा;
      SQHNode->Entries.ढकला_back(Entry);
    }
    saveAliasHNode(SQ, SQHNode);
    परत SQHNode;
  }
  स्थिती गाठ::NK_Mapping: {
    Mapping_गाठ *Map = dyn_cast<Mapping_गाठ>(N);
    स्वतः mapHNode = नवीन (MapHNodeAllocator.Allocate()) MapHNode(N);
    साठी (किल्ली_मूल्य_गाठ &KVN : *Map) {
      गाठ *किल्ली_गाठ = KVN.get_किल्ली();
      Scalar_गाठ *किल्ली = dyn_cast_if_present<Scalar_गाठ>(किल्ली_गाठ);
      गाठ *मूल्य = KVN.get_मूल्य();
      जर (!किल्ली || !मूल्य) {
        जर (!किल्ली)
          setError(किल्ली_गाठ, "Map key must be a scalar");
        जर (!मूल्य)
          setError(किल्ली_गाठ, "Map value must not be empty");
        थांबा;
      }
      StringStorage.clear();
      StringRef किल्ली_Str = किल्ली->get_मूल्य(StringStorage);
      जर (!StringStorage.empty()) {
        // Copy string to permanent storage
        किल्ली_Str = StringStorage.str().copy(StringAllocator);
      }
      जर (mapHNode->Mapping.मोजणी(किल्ली_Str))
        // From YAML spec: "The content of a mapping node is an unordered set of
        // key/value node pairs, with the restriction that each of the keys is
        // unique."
        setError(किल्ली_गाठ, Twine("duplicated mapping key '") + किल्ली_Str + "'");
      स्वतः मूल्य_HNode = createHNodes(मूल्य);
      जर (EC)
        थांबा;
      mapHNode->Mapping[किल्ली_Str] =
          std::make_pair(std::move(मूल्य_HNode), किल्ली_गाठ->getSourceRange());
    }
    saveAliasHNode(Map, mapHNode);
    परत std::move(mapHNode);
  }
  स्थिती गाठ::NK_Null:
    // TODO: Anchor is not set for NullNode in the parser. Update the parser to
    // faithfully preserve anchors.
    परत नवीन (EmptyHNodeAllocator.Allocate()) EmptyHNode(N);
  स्थिती गाठ::NK_Alias: {
    Alias_गाठ *AN = dyn_cast<Alias_गाठ>(N);
    स्वतः AliasName = AN->getName();
    स्वतः AHN = AliasMap.find(AliasName);
    जर (AHN == AliasMap.end()) {
      setError(AN, Twine("undefined alias '" + AliasName + "'"));
      परत nullptr;
    }
    परत AHN->second;
  }
  पूर्वनिर्धारित:
    setError(N, "unknown node kind");
    परत nullptr;
  }
}

शून्य आदान::setError(स्थिरांक Twine &Message) {
  setError(Current_गाठ, Message);
}

शून्य आदान::set_Allow_Unknown_किल्ल्या(तार्किक Allow) { Allow_Unknown_किल्ल्या = Allow; }

तार्किक आदान::canElideEmptySequence() {
  परत असत्य;
}

//===----------------------------------------------------------------------===//
//  Output
//===----------------------------------------------------------------------===//

Output::Output(raw_ostream &yout, शून्य *context, पूर्णांक WrapColumn)
    : IO(context), Out(yout), WrapColumn(WrapColumn) {}

Output::~Output() = पूर्वनिर्धारित;

तार्किक Output::outputting() स्थिरांक {
  परत सत्य;
}

शून्य Output::beginMapping() {
  State_चळत.ढकला_back(in_Map_First_किल्ली);
  PaddingBeforeContainer = Padding;
  Padding = "\n";
}

तार्किक Output::mapTag(StringRef Tag, तार्किक Use) {
  जर (Use) {
    // If this tag is being written inside a sequence we should write the start
    // of the sequence before writing the tag, otherwise the tag won't be
    // attached to the element in the sequence, but rather the sequence itself.
    तार्किक SequenceElement = असत्य;
    जर (State_चळत.size() > 1) {
      स्वतः &E = State_चळत[State_चळत.size() - 2];
      SequenceElement = inSeqAnyElement(E) || inFlowSeqAnyElement(E);
    }
    जर (SequenceElement && State_चळत.back() == in_Map_First_किल्ली) {
      new_ओळ_Check();
    } अन्यथा {
      output(" ");
    }
    output(Tag);
    जर (SequenceElement) {
      // If we're writing the tag during the first element of a map, the tag
      // takes the place of the first element in the sequence.
      जर (State_चळत.back() == in_Map_First_किल्ली) {
        State_चळत.काढा_back();
        State_चळत.ढकला_back(in_Map_Other_किल्ली);
      }
      // Tags inside maps in sequences should act as keys in the map from a
      // formatting perspective, so we always want a newline in a sequence.
      Padding = "\n";
    }
  }
  परत Use;
}

शून्य Output::endMapping() {
  // If we did not map anything, we should explicitly emit an empty map
  जर (State_चळत.back() == in_Map_First_किल्ली) {
    Padding = PaddingBeforeContainer;
    new_ओळ_Check();
    output("{}");
    Padding = "\n";
  }
  State_चळत.काढा_back();
}

std::vector<StringRef> Output::किल्ल्या() {
  report_fatal_error("invalid call");
}

तार्किक Output::preflight_किल्ली(StringRef किल्ली, तार्किक Required, तार्किक SameAsDefault,
                          तार्किक &UseDefault, शून्य *&SaveInfo) {
  UseDefault = असत्य;
  SaveInfo = nullptr;
  जर (Required || !SameAsDefault || Write_Default_मूल्ये) {
    स्वतः State = State_चळत.back();
    जर (State == in_Flow_Map_First_किल्ली || State == in_Flow_Map_Other_किल्ली) {
      flow_किल्ली(किल्ली);
    } अन्यथा {
      new_ओळ_Check();
      padded_किल्ली(किल्ली);
    }
    परत सत्य;
  }
  परत असत्य;
}

शून्य Output::postflight_किल्ली(शून्य *) {
  जर (State_चळत.back() == in_Map_First_किल्ली) {
    State_चळत.काढा_back();
    State_चळत.ढकला_back(in_Map_Other_किल्ली);
  } अन्यथा जर (State_चळत.back() == in_Flow_Map_First_किल्ली) {
    State_चळत.काढा_back();
    State_चळत.ढकला_back(in_Flow_Map_Other_किल्ली);
  }
}

शून्य Output::beginFlowMapping() {
  State_चळत.ढकला_back(in_Flow_Map_First_किल्ली);
  new_ओळ_Check();
  ColumnAtMapFlowStart = Column;
  output("{ ");
}

शून्य Output::endFlowMapping() {
  State_चळत.काढा_back();
  output_Up_To_End_Of_ओळ(" }");
}

शून्य Output::beginDocuments() {
  output_Up_To_End_Of_ओळ("---");
}

तार्किक Output::preflightDocument(अचिन्हित index) {
  जर (index > 0)
    output_Up_To_End_Of_ओळ("\n---");
  परत सत्य;
}

शून्य Output::postflightDocument() {
}

शून्य Output::endDocuments() {
  output("\n...\n");
}

अचिन्हित Output::beginSequence() {
  State_चळत.ढकला_back(inSeqFirstElement);
  PaddingBeforeContainer = Padding;
  Padding = "\n";
  परत 0;
}

शून्य Output::endSequence() {
  // If we did not emit anything, we should explicitly emit an empty sequence
  जर (State_चळत.back() == inSeqFirstElement) {
    Padding = PaddingBeforeContainer;
    new_ओळ_Check(/*EmptySequence=*/सत्य);
    output("[]");
    Padding = "\n";
  }
  State_चळत.काढा_back();
}

तार्किक Output::preflightElement(अचिन्हित, शून्य *&SaveInfo) {
  SaveInfo = nullptr;
  परत सत्य;
}

शून्य Output::postflightElement(शून्य *) {
  जर (State_चळत.back() == inSeqFirstElement) {
    State_चळत.काढा_back();
    State_चळत.ढकला_back(inSeqOtherElement);
  } अन्यथा जर (State_चळत.back() == inFlowSeqFirstElement) {
    State_चळत.काढा_back();
    State_चळत.ढकला_back(inFlowSeqOtherElement);
  }
}

अचिन्हित Output::beginFlowSequence() {
  State_चळत.ढकला_back(inFlowSeqFirstElement);
  new_ओळ_Check();
  ColumnAtFlowStart = Column;
  output("[ ");
  NeedFlowSequenceComma = असत्य;
  परत 0;
}

शून्य Output::endFlowSequence() {
  State_चळत.काढा_back();
  output_Up_To_End_Of_ओळ(" ]");
}

तार्किक Output::preflightFlowElement(अचिन्हित, शून्य *&SaveInfo) {
  जर (NeedFlowSequenceComma)
    output(", ");
  जर (WrapColumn && Column > WrapColumn) {
    output("\n");
    साठी (पूर्णांक i = 0; i < ColumnAtFlowStart; ++i)
      output(" ");
    Column = ColumnAtFlowStart;
    output("  ");
  }
  SaveInfo = nullptr;
  परत सत्य;
}

शून्य Output::postflightFlowElement(शून्य *) {
  NeedFlowSequenceComma = सत्य;
}

शून्य Output::beginEnumScalar() {
  EnumerationMatchFound = असत्य;
}

तार्किक Output::matchEnumScalar(StringRef Str, तार्किक Match) {
  जर (Match && !EnumerationMatchFound) {
    new_ओळ_Check();
    output_Up_To_End_Of_ओळ(Str);
    EnumerationMatchFound = सत्य;
  }
  परत असत्य;
}

तार्किक Output::matchEnumFallback() {
  जर (EnumerationMatchFound)
    परत असत्य;
  EnumerationMatchFound = सत्य;
  परत सत्य;
}

शून्य Output::endEnumScalar() {
  जर (!EnumerationMatchFound)
    llvm_unreachable("bad runtime enum value");
}

तार्किक Output::beginBitSetScalar(तार्किक &DoClear) {
  new_ओळ_Check();
  output("[ ");
  Need_Bit_मूल्य_Comma = असत्य;
  DoClear = असत्य;
  परत सत्य;
}

तार्किक Output::bitSetMatch(StringRef Str, तार्किक Matches) {
  जर (Matches) {
    जर (Need_Bit_मूल्य_Comma)
      output(", ");
    output(Str);
    Need_Bit_मूल्य_Comma = सत्य;
  }
  परत असत्य;
}

शून्य Output::endBitSetScalar() {
  output_Up_To_End_Of_ओळ(" ]");
}

शून्य Output::scalarString(StringRef &S, QuotingType MustQuote) {
  new_ओळ_Check();
  जर (S.empty()) {
    // Print '' for the empty string because leaving the field empty is not
    // allowed.
    output_Up_To_End_Of_ओळ("''");
    परत;
  }
  output(S, MustQuote);
  output_Up_To_End_Of_ओळ("");
}

शून्य Output::blockScalarString(StringRef &S) {
  जर (!State_चळत.empty())
    new_ओळ_Check();
  output(" |");

  अचिन्हित Indent = State_चळत.empty() ? 1 : State_चळत.size();

  स्वतः Buffer = MemoryBuffer::getMemBuffer(S, "", असत्य);
  साठी (ओळ_iterator ओळी(*Buffer, असत्य); !ओळी.is_at_end(); ++ओळी) {
    output_New_ओळ();
    साठी (अचिन्हित I = 0; I < Indent; ++I) {
      output("  ");
    }
    output(*ओळी);
  }
  output_Up_To_End_Of_ओळ("");
}

शून्य Output::scalarTag(std::string &Tag) {
  जर (Tag.empty())
    परत;
  new_ओळ_Check();
  output(Tag);
  output(" ");
}

शून्य Output::setError(स्थिरांक Twine &message) {
}

std::error_code Output::error() { परत {}; }

तार्किक Output::canElideEmptySequence() {
  // Normally, with an optional key/value where the value is an empty sequence,
  // the whole key/value can be not written.  But, that produces wrong yaml
  // if the key/value is the only thing in the map and the map is used in
  // a sequence.  This detects if the this sequence is the first key/value
  // in map that itself is embedded in a sequence.
  जर (State_चळत.size() < 2)
    परत सत्य;
  जर (State_चळत.back() != in_Map_First_किल्ली)
    परत सत्य;
  परत !inSeqAnyElement(State_चळत[State_चळत.size() - 2]);
}

शून्य Output::output(StringRef s) {
  Column += s.size();
  Out << s;
}

शून्य Output::output(StringRef S, QuotingType MustQuote) {
  जर (MustQuote == QuotingType::None) {
    // Only quote if we must.
    output(S);
    परत;
  }

  StringLiteral Quote = MustQuote == QuotingType::Single ? StringLiteral("'")
                                                         : StringLiteral("\"");
  output(Quote); // Starting quote.

  // When using double-quoted strings (and only in that case), non-printable
  // characters may be present, and will be escaped using a variety of
  // unicode-scalar and special short-form escapes. This is handled in
  // yaml::escape.
  जर (MustQuote == QuotingType::Double) {
    output(yaml::escape(S, /* EscapePrintable= */ असत्य));
    output(Quote);
    परत;
  }

  अचिन्हित i = 0;
  अचिन्हित j = 0;
  अचिन्हित End = S.size();
  स्थिरांक अक्षर *Base = S.data();

  // When using single-quoted strings, any single quote ' must be doubled to be
  // escaped.
  पर्यंत (j < End) {
    जर (S[j] == '\'') {                   // Escape quotes.
      output(StringRef(&Base[i], j - i)); // "flush".
      output(StringLiteral("''"));        // Print it as ''
      i = j + 1;
    }
    ++j;
  }
  output(StringRef(&Base[i], j - i));
  output(Quote); // Ending quote.
}

शून्य Output::output_Up_To_End_Of_ओळ(StringRef s) {
  output(s);
  जर (State_चळत.empty() || (!inFlowSeqAnyElement(State_चळत.back()) &&
                             !in_Flow_Map_Any_किल्ली(State_चळत.back())))
    Padding = "\n";
}

शून्य Output::output_New_ओळ() {
  Out << "\n";
  Column = 0;
}

// if seq at top, indent as if map, then add "- "
// if seq in middle, use "- " if firstKey, else use "  "
//

शून्य Output::new_ओळ_Check(तार्किक EmptySequence) {
  जर (Padding != "\n") {
    output(Padding);
    Padding = {};
    परत;
  }
  output_New_ओळ();
  Padding = {};

  जर (State_चळत.size() == 0 || EmptySequence)
    परत;

  अचिन्हित Indent = State_चळत.size() - 1;
  तार्किक PossiblyNestedSeq = असत्य;
  स्वतः I = State_चळत.rbegin(), E = State_चळत.rend();

  जर (inSeqAnyElement(*I)) {
    PossiblyNestedSeq = सत्य; // Not possibly but always.
    ++Indent;
  } अन्यथा जर (*I == in_Map_First_किल्ली || *I == in_Flow_Map_First_किल्ली ||
             inFlowSeqAnyElement(*I)) {
    PossiblyNestedSeq = सत्य;
    ++I; // Skip back().
  }

  अचिन्हित Output_Dash_मोजणी = 0;
  जर (PossiblyNestedSeq) {
    // Count up consecutive inSeqFirstElement from the end, unless
    // inSeqFirstElement is the top of nested sequence.
    पर्यंत (I != E) {
      // Don't count the top of nested sequence.
      जर (!inSeqAnyElement(*I))
        थांबा;

      ++Output_Dash_मोजणी;

      // Stop counting if consecutive inSeqFirstElement ends.
      जर (*I++ != inSeqFirstElement)
        थांबा;
    }
  }

  साठी (अचिन्हित I = Output_Dash_मोजणी; I < Indent; ++I)
    output("  ");

  साठी (अचिन्हित I = 0; I < Output_Dash_मोजणी; ++I)
    output("- ");
}

शून्य Output::padded_किल्ली(StringRef किल्ली_2) {
  output(किल्ली_2, needsQuotes(किल्ली_2, असत्य));
  output(":");
  स्थिरांक अक्षर *spaces = "                ";
  जर (किल्ली_2.size() < म_माप(spaces))
    Padding = &spaces[किल्ली_2.size()];
  अन्यथा
    Padding = " ";
}

शून्य Output::flow_किल्ली(StringRef किल्ली) {
  जर (State_चळत.back() == in_Flow_Map_Other_किल्ली)
    output(", ");
  जर (WrapColumn && Column > WrapColumn) {
    output("\n");
    साठी (पूर्णांक I = 0; I < ColumnAtMapFlowStart; ++I)
      output(" ");
    Column = ColumnAtMapFlowStart;
    output("  ");
  }
  output(किल्ली, needsQuotes(किल्ली, असत्य));
  output(": ");
}

गाठ_Kind Output::get_गाठ_Kind() { report_fatal_error("invalid call"); }

तार्किक Output::inSeqAnyElement(InState State) {
  परत State == inSeqFirstElement || State == inSeqOtherElement;
}

तार्किक Output::inFlowSeqAnyElement(InState State) {
  परत State == inFlowSeqFirstElement || State == inFlowSeqOtherElement;
}

तार्किक Output::in_Map_Any_किल्ली(InState State) {
  परत State == in_Map_First_किल्ली || State == in_Map_Other_किल्ली;
}

तार्किक Output::in_Flow_Map_Any_किल्ली(InState State) {
  परत State == in_Flow_Map_First_किल्ली || State == in_Flow_Map_Other_किल्ली;
}

//===----------------------------------------------------------------------===//
//  traits for built-in types
//===----------------------------------------------------------------------===//

शून्य ScalarTraits<तार्किक>::output(स्थिरांक तार्किक &Val, शून्य *, raw_ostream &Out) {
  Out << (Val ? "true" : "false");
}

StringRef ScalarTraits<तार्किक>::आदान_2(StringRef Scalar, शून्य *, तार्किक &Val) {
  जर (std::optional<तार्किक> Parsed = parseBool(Scalar)) {
    Val = *Parsed;
    परत StringRef();
  }
  परत "invalid boolean";
}

शून्य ScalarTraits<StringRef>::output(स्थिरांक StringRef &Val, शून्य *,
                                     raw_ostream &Out) {
  Out << Val;
}

StringRef ScalarTraits<StringRef>::आदान_2(StringRef Scalar, शून्य *,
                                         StringRef &Val) {
  Val = Scalar;
  परत StringRef();
}

शून्य ScalarTraits<std::string>::output(स्थिरांक std::string &Val, शून्य *,
                                       raw_ostream &Out) {
  Out << Val;
}

StringRef ScalarTraits<std::string>::आदान_2(StringRef Scalar, शून्य *,
                                           std::string &Val) {
  Val = Scalar.str();
  परत StringRef();
}

शून्य ScalarTraits<uint8_t>::output(स्थिरांक uint8_t &Val, शून्य *,
                                   raw_ostream &Out) {
  // use temp uin32_t because ostream thinks uint8_t is a character
  uint32_t अंक = Val;
  Out << अंक;
}

StringRef ScalarTraits<uint8_t>::आदान_2(StringRef Scalar, शून्य *, uint8_t &Val) {
  अचिन्हित दीर्घ दीर्घ n;
  जर (getAsUnsignedInteger(Scalar, 0, n))
    परत "invalid number";
  जर (n > 0xFF)
    परत "out of range number";
  Val = n;
  परत StringRef();
}

शून्य ScalarTraits<uint16_t>::output(स्थिरांक uint16_t &Val, शून्य *,
                                    raw_ostream &Out) {
  Out << Val;
}

StringRef ScalarTraits<uint16_t>::आदान_2(StringRef Scalar, शून्य *,
                                        uint16_t &Val) {
  अचिन्हित दीर्घ दीर्घ n;
  जर (getAsUnsignedInteger(Scalar, 0, n))
    परत "invalid number";
  जर (n > 0xFFFF)
    परत "out of range number";
  Val = n;
  परत StringRef();
}

शून्य ScalarTraits<uint32_t>::output(स्थिरांक uint32_t &Val, शून्य *,
                                    raw_ostream &Out) {
  Out << Val;
}

StringRef ScalarTraits<uint32_t>::आदान_2(StringRef Scalar, शून्य *,
                                        uint32_t &Val) {
  अचिन्हित दीर्घ दीर्घ n;
  जर (getAsUnsignedInteger(Scalar, 0, n))
    परत "invalid number";
  जर (n > 0xFFFFFFFFUL)
    परत "out of range number";
  Val = n;
  परत StringRef();
}

शून्य ScalarTraits<uint64_t>::output(स्थिरांक uint64_t &Val, शून्य *,
                                    raw_ostream &Out) {
  Out << Val;
}

StringRef ScalarTraits<uint64_t>::आदान_2(StringRef Scalar, शून्य *,
                                        uint64_t &Val) {
  अचिन्हित दीर्घ दीर्घ N;
  जर (getAsUnsignedInteger(Scalar, 0, N))
    परत "invalid number";
  Val = N;
  परत StringRef();
}

शून्य ScalarTraits<int8_t>::output(स्थिरांक int8_t &Val, शून्य *, raw_ostream &Out) {
  // use temp in32_t because ostream thinks int8_t is a character
  int32_t अंक = Val;
  Out << अंक;
}

StringRef ScalarTraits<int8_t>::आदान_2(StringRef Scalar, शून्य *, int8_t &Val) {
  दीर्घ दीर्घ N;
  जर (getAsSignedInteger(Scalar, 0, N))
    परत "invalid number";
  जर ((N > 127) || (N < -128))
    परत "out of range number";
  Val = N;
  परत StringRef();
}

शून्य ScalarTraits<int16_t>::output(स्थिरांक int16_t &Val, शून्य *,
                                   raw_ostream &Out) {
  Out << Val;
}

StringRef ScalarTraits<int16_t>::आदान_2(StringRef Scalar, शून्य *, int16_t &Val) {
  दीर्घ दीर्घ N;
  जर (getAsSignedInteger(Scalar, 0, N))
    परत "invalid number";
  जर ((N > INT16_MAX) || (N < INT16_MIN))
    परत "out of range number";
  Val = N;
  परत StringRef();
}

शून्य ScalarTraits<int32_t>::output(स्थिरांक int32_t &Val, शून्य *,
                                   raw_ostream &Out) {
  Out << Val;
}

StringRef ScalarTraits<int32_t>::आदान_2(StringRef Scalar, शून्य *, int32_t &Val) {
  दीर्घ दीर्घ N;
  जर (getAsSignedInteger(Scalar, 0, N))
    परत "invalid number";
  जर ((N > INT32_MAX) || (N < INT32_MIN))
    परत "out of range number";
  Val = N;
  परत StringRef();
}

शून्य ScalarTraits<int64_t>::output(स्थिरांक int64_t &Val, शून्य *,
                                   raw_ostream &Out) {
  Out << Val;
}

StringRef ScalarTraits<int64_t>::आदान_2(StringRef Scalar, शून्य *, int64_t &Val) {
  दीर्घ दीर्घ N;
  जर (getAsSignedInteger(Scalar, 0, N))
    परत "invalid number";
  Val = N;
  परत StringRef();
}

शून्य ScalarTraits<द्विगुण>::output(स्थिरांक द्विगुण &Val, शून्य *, raw_ostream &Out) {
  Out << format("%g", Val);
}

StringRef ScalarTraits<द्विगुण>::आदान_2(StringRef Scalar, शून्य *, द्विगुण &Val) {
  जर (to_float(Scalar, Val))
    परत StringRef();
  परत "invalid floating point number";
}

शून्य ScalarTraits<दशांश>::output(स्थिरांक दशांश &Val, शून्य *, raw_ostream &Out) {
  Out << format("%g", Val);
}

StringRef ScalarTraits<दशांश>::आदान_2(StringRef Scalar, शून्य *, दशांश &Val) {
  जर (to_float(Scalar, Val))
    परत StringRef();
  परत "invalid floating point number";
}

शून्य ScalarTraits<Hex8>::output(स्थिरांक Hex8 &Val, शून्य *, raw_ostream &Out) {
  Out << format("0x%" PRIX8, (uint8_t)Val);
}

StringRef ScalarTraits<Hex8>::आदान_2(StringRef Scalar, शून्य *, Hex8 &Val) {
  अचिन्हित दीर्घ दीर्घ n;
  जर (getAsUnsignedInteger(Scalar, 0, n))
    परत "invalid hex8 number";
  जर (n > 0xFF)
    परत "out of range hex8 number";
  Val = n;
  परत StringRef();
}

शून्य ScalarTraits<Hex16>::output(स्थिरांक Hex16 &Val, शून्य *, raw_ostream &Out) {
  Out << format("0x%" PRIX16, (uint16_t)Val);
}

StringRef ScalarTraits<Hex16>::आदान_2(StringRef Scalar, शून्य *, Hex16 &Val) {
  अचिन्हित दीर्घ दीर्घ n;
  जर (getAsUnsignedInteger(Scalar, 0, n))
    परत "invalid hex16 number";
  जर (n > 0xFFFF)
    परत "out of range hex16 number";
  Val = n;
  परत StringRef();
}

शून्य ScalarTraits<Hex32>::output(स्थिरांक Hex32 &Val, शून्य *, raw_ostream &Out) {
  Out << format("0x%" PRIX32, (uint32_t)Val);
}

StringRef ScalarTraits<Hex32>::आदान_2(StringRef Scalar, शून्य *, Hex32 &Val) {
  अचिन्हित दीर्घ दीर्घ n;
  जर (getAsUnsignedInteger(Scalar, 0, n))
    परत "invalid hex32 number";
  जर (n > 0xFFFFFFFFUL)
    परत "out of range hex32 number";
  Val = n;
  परत StringRef();
}

शून्य ScalarTraits<Hex64>::output(स्थिरांक Hex64 &Val, शून्य *, raw_ostream &Out) {
  Out << format("0x%" PRIX64, (uint64_t)Val);
}

StringRef ScalarTraits<Hex64>::आदान_2(StringRef Scalar, शून्य *, Hex64 &Val) {
  अचिन्हित दीर्घ दीर्घ अंक;
  जर (getAsUnsignedInteger(Scalar, 0, अंक))
    परत "invalid hex64 number";
  Val = अंक;
  परत StringRef();
}

शून्य ScalarTraits<VersionTuple>::output(स्थिरांक VersionTuple &Val, शून्य *,
                                        llvm::raw_ostream &Out) {
  Out << Val.getAsString();
}

StringRef ScalarTraits<VersionTuple>::आदान_2(StringRef Scalar, शून्य *,
                                            VersionTuple &Val) {
  जर (Val.tryParse(Scalar))
    परत "invalid version format";
  परत StringRef();
}
