//===- lib/Support/YAMLTraits.cpp -----------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#અન્તર્ભાવ "llvm/Support/YAMLTraits.h"
#અન્તર્ભાવ "llvm/ADT/STLExtras.h"
#અન્તર્ભાવ "llvm/ADT/SmallString.h"
#અન્તર્ભાવ "llvm/ADT/StringExtras.h"
#અન્તર્ભાવ "llvm/ADT/StringRef.h"
#અન્તર્ભાવ "llvm/ADT/Twine.h"
#અન્તર્ભાવ "llvm/Support/Casting.h"
#અન્તર્ભાવ "llvm/Support/Errc.h"
#અન્તર્ભાવ "llvm/Support/ErrorHandling.h"
#અન્તર્ભાવ "llvm/Support/Format.h"
#અન્તર્ભાવ "llvm/Support/LineIterator.h"
#અન્તર્ભાવ "llvm/Support/MemoryBuffer.h"
#અન્તર્ભાવ "llvm/Support/VersionTuple.h"
#અન્તર્ભાવ "llvm/Support/YAMLParser.h"
#અન્તર્ભાવ "llvm/Support/raw_ostream.h"
#અન્તર્ભાવ <cassert>
#અન્તર્ભાવ <cstdint>
#અન્તર્ભાવ <cstring>
#અન્તર્ભાવ <string>
#અન્તર્ભાવ <vector>

using નામાકાશ llvm;
using નામાકાશ yaml;

//===----------------------------------------------------------------------===//
//  IO
//===----------------------------------------------------------------------===//

IO::IO(શૂન્ય *Context) : Ctxt(Context) {}

IO::~IO() = મૂળભૂત;

શૂન્ય *IO::getContext() અચલ {
  પાછા Ctxt;
}

શૂન્ય IO::setContext(શૂન્ય *Context) {
  Ctxt = Context;
}

શૂન્ય IO::set_Allow_Unknown_ચાવીઓ(તાર્કિક Allow) {
  llvm_unreachable("Only supported for Input");
}

//===----------------------------------------------------------------------===//
//આગત
//===----------------------------------------------------------------------===//

આગત::આગત(StringRef આગત_Content, શૂન્ય *Ctxt,
             SourceMgr::DiagHandlerTy DiagHandler, શૂન્ય *DiagHandlerCtxt)
    : IO(Ctxt), Strm(નવું Stream(આગત_Content, SrcMgr, ખોટું, &EC)) {
  જો (DiagHandler)
    SrcMgr.setDiagHandler(DiagHandler, DiagHandlerCtxt);
  DocIterator = Strm->begin();
}

આગત::આગત(MemoryBufferRef આગત, શૂન્ય *Ctxt,
             SourceMgr::DiagHandlerTy DiagHandler, શૂન્ય *DiagHandlerCtxt)
    : IO(Ctxt), Strm(નવું Stream(આગત, SrcMgr, ખોટું, &EC)) {
  જો (DiagHandler)
    SrcMgr.setDiagHandler(DiagHandler, DiagHandlerCtxt);
  DocIterator = Strm->begin();
}

આગત::~આગત() = મૂળભૂત;

std::error_code આગત::error() { પાછા EC; }

તાર્કિક આગત::outputting() અચલ {
  પાછા ખોટું;
}

તાર્કિક આગત::setCurrentDocument() {
  જો (DocIterator != Strm->end()) {
    ગાંઠ *N = DocIterator->get_મૂળ();
    જો (!N) {
      EC = make_error_code(errc::invalid_argument);
      પાછા ખોટું;
    }

    જો (isa<Null_ગાંઠ>(N)) {
      // Empty files are allowed and ignored
      ++DocIterator;
      પાછા setCurrentDocument();
    }
    releaseHNodeBuffers();
    ટોચ_ગાંઠ = createHNodes(N);
    Current_ગાંઠ = ટોચ_ગાંઠ;
    પાછા સાચું;
  }
  પાછા ખોટું;
}

તાર્કિક આગત::nextDocument() {
  પાછા ++DocIterator != Strm->end();
}

અચલ ગાંઠ *આગત::get_Current_ગાંઠ() અચલ {
  પાછા Current_ગાંઠ ? Current_ગાંઠ->ગાંઠ_2 : nullptr;
}

તાર્કિક આગત::mapTag(StringRef Tag, તાર્કિક Default) {
  // CurrentNode can be null if setCurrentDocument() was unable to
  // parse the document because it was invalid or empty.
  જો (!Current_ગાંઠ)
    પાછા ખોટું;

  std::string foundTag = Current_ગાંઠ->ગાંઠ_2->getVerbatimTag();
  જો (foundTag.empty()) {
    // If no tag found and 'Tag' is the default, say it was found.
    પાછા Default;
  }
  // Return true iff found tag matches supplied tag.
  પાછા Tag == foundTag;
}

શૂન્ય આગત::beginMapping() {
  જો (EC)
    પાછા;
  // CurrentNode can be null if the document is empty.
  MapHNode *MN = dyn_cast_or_null<MapHNode>(Current_ગાંઠ);
  જો (MN) {
    MN->Valid_ચાવીઓ.clear();
  }
}

std::vector<StringRef> આગત::ચાવીઓ() {
  MapHNode *MN = dyn_cast<MapHNode>(Current_ગાંઠ);
  std::vector<StringRef> Ret;
  જો (!MN) {
    setError(Current_ગાંઠ, "not a mapping");
    પાછા Ret;
  }
  માટે (auto &P : MN->Mapping)
    Ret.ધકેલો_back(P.first());
  પાછા Ret;
}

તાર્કિક આગત::preflight_ચાવી(StringRef ચાવી, તાર્કિક Required, તાર્કિક, તાર્કિક &UseDefault,
                         શૂન્ય *&SaveInfo) {
  UseDefault = ખોટું;
  જો (EC)
    પાછા ખોટું;

  // CurrentNode is null for empty documents, which is an error in case required
  // nodes are present.
  જો (!Current_ગાંઠ) {
    જો (Required)
      EC = make_error_code(errc::invalid_argument);
    નહીંતર
      UseDefault = સાચું;
    પાછા ખોટું;
  }

  MapHNode *MN = dyn_cast<MapHNode>(Current_ગાંઠ);
  જો (!MN) {
    જો (Required || !isa<EmptyHNode>(Current_ગાંઠ))
      setError(Current_ગાંઠ, "not a mapping");
    નહીંતર
      UseDefault = સાચું;
    પાછા ખોટું;
  }
  MN->Valid_ચાવીઓ.ધકેલો_back(ચાવી.str());
  HNode *કિંમત = MN->Mapping[ચાવી].first;
  જો (!કિંમત) {
    જો (Required)
      setError(Current_ગાંઠ, Twine("missing required key '") + ચાવી + "'");
    નહીંતર
      UseDefault = સાચું;
    પાછા ખોટું;
  }
  SaveInfo = Current_ગાંઠ;
  Current_ગાંઠ = કિંમત;
  પાછા સાચું;
}

શૂન્ય આગત::postflight_ચાવી(શૂન્ય *saveInfo) {
  Current_ગાંઠ = reinterpret_cast<HNode *>(saveInfo);
}

શૂન્ય આગત::endMapping() {
  જો (EC)
    પાછા;
  // CurrentNode can be null if the document is empty.
  MapHNode *MN = dyn_cast_or_null<MapHNode>(Current_ગાંઠ);
  જો (!MN)
    પાછા;
  માટે (અચલ auto &NN : MN->Mapping) {
    જો (!is_contained(MN->Valid_ચાવીઓ, NN.first())) {
      અચલ SMRange &ReportLoc = NN.second.second;
      જો (!Allow_Unknown_ચાવીઓ) {
        setError(ReportLoc, Twine("unknown key '") + NN.first() + "'");
        તોડો;
      } નહીંતર
        reportWarning(ReportLoc, Twine("unknown key '") + NN.first() + "'");
    }
  }
}

શૂન્ય આગત::beginFlowMapping() { beginMapping(); }

શૂન્ય આગત::endFlowMapping() { endMapping(); }

unsigned આગત::beginSequence() {
  જો (SequenceHNode *SQ = dyn_cast<SequenceHNode>(Current_ગાંઠ))
    પાછા SQ->Entries.size();
  જો (isa<EmptyHNode>(Current_ગાંઠ))
    પાછા 0;
  // Treat case where there's a scalar "null" value as an empty sequence.
  જો (ScalarHNode *SN = dyn_cast<ScalarHNode>(Current_ગાંઠ)) {
    જો (isNull(SN->કિંમત_2()))
      પાછા 0;
  }
  // Any other type of HNode is an error.
  setError(Current_ગાંઠ, "not a sequence");
  પાછા 0;
}

શૂન્ય આગત::endSequence() {
}

તાર્કિક આગત::preflightElement(unsigned Index, શૂન્ય *&SaveInfo) {
  જો (EC)
    પાછા ખોટું;
  જો (SequenceHNode *SQ = dyn_cast<SequenceHNode>(Current_ગાંઠ)) {
    SaveInfo = Current_ગાંઠ;
    Current_ગાંઠ = SQ->Entries[Index];
    પાછા સાચું;
  }
  પાછા ખોટું;
}

શૂન્ય આગત::postflightElement(શૂન્ય *SaveInfo) {
  Current_ગાંઠ = reinterpret_cast<HNode *>(SaveInfo);
}

unsigned આગત::beginFlowSequence() { પાછા beginSequence(); }

તાર્કિક આગત::preflightFlowElement(unsigned index, શૂન્ય *&SaveInfo) {
  જો (EC)
    પાછા ખોટું;
  જો (SequenceHNode *SQ = dyn_cast<SequenceHNode>(Current_ગાંઠ)) {
    SaveInfo = Current_ગાંઠ;
    Current_ગાંઠ = SQ->Entries[index];
    પાછા સાચું;
  }
  પાછા ખોટું;
}

શૂન્ય આગત::postflightFlowElement(શૂન્ય *SaveInfo) {
  Current_ગાંઠ = reinterpret_cast<HNode *>(SaveInfo);
}

શૂન્ય આગત::endFlowSequence() {
}

શૂન્ય આગત::beginEnumScalar() {
  ScalarMatchFound = ખોટું;
}

તાર્કિક આગત::matchEnumScalar(StringRef Str, તાર્કિક) {
  જો (ScalarMatchFound)
    પાછા ખોટું;
  જો (ScalarHNode *SN = dyn_cast<ScalarHNode>(Current_ગાંઠ)) {
    જો (SN->કિંમત_2() == Str) {
      ScalarMatchFound = સાચું;
      પાછા સાચું;
    }
  }
  પાછા ખોટું;
}

તાર્કિક આગત::matchEnumFallback() {
  જો (ScalarMatchFound)
    પાછા ખોટું;
  ScalarMatchFound = સાચું;
  પાછા સાચું;
}

શૂન્ય આગત::endEnumScalar() {
  જો (!ScalarMatchFound) {
    setError(Current_ગાંઠ, "unknown enumerated scalar");
  }
}

તાર્કિક આગત::beginBitSetScalar(તાર્કિક &DoClear) {
  Bit_કિંમતો_Used.clear();
  જો (SequenceHNode *SQ = dyn_cast<SequenceHNode>(Current_ગાંઠ)) {
    Bit_કિંમતો_Used.resize(SQ->Entries.size());
  } નહીંતર {
    setError(Current_ગાંઠ, "expected sequence of bit values");
  }
  DoClear = સાચું;
  પાછા સાચું;
}

તાર્કિક આગત::bitSetMatch(StringRef Str, તાર્કિક) {
  જો (EC)
    પાછા ખોટું;
  જો (SequenceHNode *SQ = dyn_cast<SequenceHNode>(Current_ગાંઠ)) {
    unsigned Index = 0;
    માટે (auto &N : SQ->Entries) {
      જો (ScalarHNode *SN = dyn_cast<ScalarHNode>(N)) {
        જો (SN->કિંમત_2() == Str) {
          Bit_કિંમતો_Used[Index] = સાચું;
          પાછા સાચું;
        }
      } નહીંતર {
        setError(Current_ગાંઠ, "unexpected scalar in sequence of bit values");
      }
      ++Index;
    }
  } નહીંતર {
    setError(Current_ગાંઠ, "expected sequence of bit values");
  }
  પાછા ખોટું;
}

શૂન્ય આગત::endBitSetScalar() {
  જો (EC)
    પાછા;
  જો (SequenceHNode *SQ = dyn_cast<SequenceHNode>(Current_ગાંઠ)) {
    પ્રતિજ્ઞા(Bit_કિંમતો_Used.size() == SQ->Entries.size());
    માટે (unsigned i = 0; i < SQ->Entries.size(); ++i) {
      જો (!Bit_કિંમતો_Used[i]) {
        setError(SQ->Entries[i], "unknown bit value");
        પાછા;
      }
    }
  }
}

શૂન્ય આગત::scalarString(StringRef &S, QuotingType) {
  જો (ScalarHNode *SN = dyn_cast<ScalarHNode>(Current_ગાંઠ)) {
    S = SN->કિંમત_2();
  } નહીંતર {
    setError(Current_ગાંઠ, "unexpected scalar");
  }
}

શૂન્ય આગત::blockScalarString(StringRef &S) { scalarString(S, QuotingType::None); }

શૂન્ય આગત::scalarTag(std::string &Tag) {
  Tag = Current_ગાંઠ->ગાંઠ_2->getVerbatimTag();
}

શૂન્ય આગત::setError(HNode *hnode, અચલ Twine &message) {
  પ્રતિજ્ઞા(hnode && "HNode must not be NULL");
  setError(hnode->ગાંઠ_2, message);
}

ગાંઠ_Kind આગત::get_ગાંઠ_Kind() {
  જો (isa<ScalarHNode>(Current_ગાંઠ))
    પાછા ગાંઠ_Kind::Scalar;
  નહીંતર જો (isa<MapHNode>(Current_ગાંઠ))
    પાછા ગાંઠ_Kind::Map;
  નહીંતર જો (isa<SequenceHNode>(Current_ગાંઠ))
    પાછા ગાંઠ_Kind::Sequence;
  llvm_unreachable("Unsupported node kind");
}

શૂન્ય આગત::setError(ગાંઠ *ગાંઠ_3, અચલ Twine &message) {
  Strm->printError(ગાંઠ_3, message);
  EC = make_error_code(errc::invalid_argument);
}

શૂન્ય આગત::setError(અચલ SMRange &range, અચલ Twine &message) {
  Strm->printError(range, message);
  EC = make_error_code(errc::invalid_argument);
}

શૂન્ય આગત::reportWarning(HNode *hnode, અચલ Twine &message) {
  પ્રતિજ્ઞા(hnode && "HNode must not be NULL");
  Strm->printError(hnode->ગાંઠ_2, message, SourceMgr::DK_Warning);
}

શૂન્ય આગત::reportWarning(ગાંઠ *ગાંઠ_3, અચલ Twine &message) {
  Strm->printError(ગાંઠ_3, message, SourceMgr::DK_Warning);
}

શૂન્ય આગત::reportWarning(અચલ SMRange &range, અચલ Twine &message) {
  Strm->printError(range, message, SourceMgr::DK_Warning);
}

શૂન્ય આગત::releaseHNodeBuffers() {
  EmptyHNodeAllocator.DestroyAll();
  ScalarHNodeAllocator.DestroyAll();
  SequenceHNodeAllocator.DestroyAll();
  MapHNodeAllocator.DestroyAll();
}

શૂન્ય આગત::saveAliasHNode(ગાંઠ *N, HNode *HN) {
  StringRef Anchor = N->getAnchor();
  જો (!Anchor.empty())
    // YAML 1.2.2 - 3.2.2.2. Anchors and Aliases:
    //
    // An alias event refers to the most recent event in the serialization
    // having the specified anchor. Therefore, anchors need not be unique within
    // a serialization. In addition, an anchor need not have an alias node
    // referring to it.
    AliasMap[Anchor] = HN;
}

આગત::HNode *આગત::createHNodes(ગાંઠ *N) {
  SmallString<128> StringStorage;
  પસંદગી (N->getType()) {
  સ્થિતિ ગાંઠ::NK_Scalar: {
    Scalar_ગાંઠ *SN = dyn_cast<Scalar_ગાંઠ>(N);
    StringRef ચાવી_Str = SN->get_કિંમત(StringStorage);
    જો (!StringStorage.empty()) {
      // Copy string to permanent storage
      ચાવી_Str = StringStorage.str().copy(StringAllocator);
    }
    auto *SHNode = નવું (ScalarHNodeAllocator.Allocate()) ScalarHNode(N, ચાવી_Str);
    saveAliasHNode(SN, SHNode);
    પાછા SHNode;
  }
  સ્થિતિ ગાંઠ::NK_BlockScalar: {
    Block_Scalar_ગાંઠ *BSN = dyn_cast<Block_Scalar_ગાંઠ>(N);
    StringRef કિંમત_Copy = BSN->get_કિંમત().copy(StringAllocator);
    auto *BSHNode =
        નવું (ScalarHNodeAllocator.Allocate()) ScalarHNode(N, કિંમત_Copy);
    saveAliasHNode(BSN, BSHNode);
    પાછા BSHNode;
  }
  સ્થિતિ ગાંઠ::NK_Sequence: {
    Sequence_ગાંઠ *SQ = dyn_cast<Sequence_ગાંઠ>(N);
    auto SQHNode = નવું (SequenceHNodeAllocator.Allocate()) SequenceHNode(N);
    માટે (ગાંઠ &SN : *SQ) {
      auto Entry = createHNodes(&SN);
      જો (EC)
        તોડો;
      SQHNode->Entries.ધકેલો_back(Entry);
    }
    saveAliasHNode(SQ, SQHNode);
    પાછા SQHNode;
  }
  સ્થિતિ ગાંઠ::NK_Mapping: {
    Mapping_ગાંઠ *Map = dyn_cast<Mapping_ગાંઠ>(N);
    auto mapHNode = નવું (MapHNodeAllocator.Allocate()) MapHNode(N);
    માટે (ચાવી_કિંમત_ગાંઠ &KVN : *Map) {
      ગાંઠ *ચાવી_ગાંઠ = KVN.get_ચાવી();
      Scalar_ગાંઠ *ચાવી = dyn_cast_if_present<Scalar_ગાંઠ>(ચાવી_ગાંઠ);
      ગાંઠ *કિંમત = KVN.get_કિંમત();
      જો (!ચાવી || !કિંમત) {
        જો (!ચાવી)
          setError(ચાવી_ગાંઠ, "Map key must be a scalar");
        જો (!કિંમત)
          setError(ચાવી_ગાંઠ, "Map value must not be empty");
        તોડો;
      }
      StringStorage.clear();
      StringRef ચાવી_Str = ચાવી->get_કિંમત(StringStorage);
      જો (!StringStorage.empty()) {
        // Copy string to permanent storage
        ચાવી_Str = StringStorage.str().copy(StringAllocator);
      }
      જો (mapHNode->Mapping.ગણતરી(ચાવી_Str))
        // From YAML spec: "The content of a mapping node is an unordered set of
        // key/value node pairs, with the restriction that each of the keys is
        // unique."
        setError(ચાવી_ગાંઠ, Twine("duplicated mapping key '") + ચાવી_Str + "'");
      auto કિંમત_HNode = createHNodes(કિંમત);
      જો (EC)
        તોડો;
      mapHNode->Mapping[ચાવી_Str] =
          std::make_pair(std::move(કિંમત_HNode), ચાવી_ગાંઠ->getSourceRange());
    }
    saveAliasHNode(Map, mapHNode);
    પાછા std::move(mapHNode);
  }
  સ્થિતિ ગાંઠ::NK_Null:
    // TODO: Anchor is not set for NullNode in the parser. Update the parser to
    // faithfully preserve anchors.
    પાછા નવું (EmptyHNodeAllocator.Allocate()) EmptyHNode(N);
  સ્થિતિ ગાંઠ::NK_Alias: {
    Alias_ગાંઠ *AN = dyn_cast<Alias_ગાંઠ>(N);
    auto AliasName = AN->getName();
    auto AHN = AliasMap.find(AliasName);
    જો (AHN == AliasMap.end()) {
      setError(AN, Twine("undefined alias '" + AliasName + "'"));
      પાછા nullptr;
    }
    પાછા AHN->second;
  }
  મૂળભૂત:
    setError(N, "unknown node kind");
    પાછા nullptr;
  }
}

શૂન્ય આગત::setError(અચલ Twine &Message) {
  setError(Current_ગાંઠ, Message);
}

શૂન્ય આગત::set_Allow_Unknown_ચાવીઓ(તાર્કિક Allow) { Allow_Unknown_ચાવીઓ = Allow; }

તાર્કિક આગત::canElideEmptySequence() {
  પાછા ખોટું;
}

//===----------------------------------------------------------------------===//
//  Output
//===----------------------------------------------------------------------===//

Output::Output(raw_ostream &yout, શૂન્ય *context, પૂર્ણાંક WrapColumn)
    : IO(context), Out(yout), WrapColumn(WrapColumn) {}

Output::~Output() = મૂળભૂત;

તાર્કિક Output::outputting() અચલ {
  પાછા સાચું;
}

શૂન્ય Output::beginMapping() {
  State_ઢગલો.ધકેલો_back(in_Map_First_ચાવી);
  PaddingBeforeContainer = Padding;
  Padding = "\n";
}

તાર્કિક Output::mapTag(StringRef Tag, તાર્કિક Use) {
  જો (Use) {
    // If this tag is being written inside a sequence we should write the start
    // of the sequence before writing the tag, otherwise the tag won't be
    // attached to the element in the sequence, but rather the sequence itself.
    તાર્કિક SequenceElement = ખોટું;
    જો (State_ઢગલો.size() > 1) {
      auto &E = State_ઢગલો[State_ઢગલો.size() - 2];
      SequenceElement = inSeqAnyElement(E) || inFlowSeqAnyElement(E);
    }
    જો (SequenceElement && State_ઢગલો.back() == in_Map_First_ચાવી) {
      new_લીટી_Check();
    } નહીંતર {
      output(" ");
    }
    output(Tag);
    જો (SequenceElement) {
      // If we're writing the tag during the first element of a map, the tag
      // takes the place of the first element in the sequence.
      જો (State_ઢગલો.back() == in_Map_First_ચાવી) {
        State_ઢગલો.કાઢો_back();
        State_ઢગલો.ધકેલો_back(in_Map_Other_ચાવી);
      }
      // Tags inside maps in sequences should act as keys in the map from a
      // formatting perspective, so we always want a newline in a sequence.
      Padding = "\n";
    }
  }
  પાછા Use;
}

શૂન્ય Output::endMapping() {
  // If we did not map anything, we should explicitly emit an empty map
  જો (State_ઢગલો.back() == in_Map_First_ચાવી) {
    Padding = PaddingBeforeContainer;
    new_લીટી_Check();
    output("{}");
    Padding = "\n";
  }
  State_ઢગલો.કાઢો_back();
}

std::vector<StringRef> Output::ચાવીઓ() {
  report_fatal_error("invalid call");
}

તાર્કિક Output::preflight_ચાવી(StringRef ચાવી, તાર્કિક Required, તાર્કિક SameAsDefault,
                          તાર્કિક &UseDefault, શૂન્ય *&SaveInfo) {
  UseDefault = ખોટું;
  SaveInfo = nullptr;
  જો (Required || !SameAsDefault || Write_Default_કિંમતો) {
    auto State = State_ઢગલો.back();
    જો (State == in_Flow_Map_First_ચાવી || State == in_Flow_Map_Other_ચાવી) {
      flow_ચાવી(ચાવી);
    } નહીંતર {
      new_લીટી_Check();
      padded_ચાવી(ચાવી);
    }
    પાછા સાચું;
  }
  પાછા ખોટું;
}

શૂન્ય Output::postflight_ચાવી(શૂન્ય *) {
  જો (State_ઢગલો.back() == in_Map_First_ચાવી) {
    State_ઢગલો.કાઢો_back();
    State_ઢગલો.ધકેલો_back(in_Map_Other_ચાવી);
  } નહીંતર જો (State_ઢગલો.back() == in_Flow_Map_First_ચાવી) {
    State_ઢગલો.કાઢો_back();
    State_ઢગલો.ધકેલો_back(in_Flow_Map_Other_ચાવી);
  }
}

શૂન્ય Output::beginFlowMapping() {
  State_ઢગલો.ધકેલો_back(in_Flow_Map_First_ચાવી);
  new_લીટી_Check();
  ColumnAtMapFlowStart = Column;
  output("{ ");
}

શૂન્ય Output::endFlowMapping() {
  State_ઢગલો.કાઢો_back();
  output_Up_To_End_Of_લીટી(" }");
}

શૂન્ય Output::beginDocuments() {
  output_Up_To_End_Of_લીટી("---");
}

તાર્કિક Output::preflightDocument(unsigned index) {
  જો (index > 0)
    output_Up_To_End_Of_લીટી("\n---");
  પાછા સાચું;
}

શૂન્ય Output::postflightDocument() {
}

શૂન્ય Output::endDocuments() {
  output("\n...\n");
}

unsigned Output::beginSequence() {
  State_ઢગલો.ધકેલો_back(inSeqFirstElement);
  PaddingBeforeContainer = Padding;
  Padding = "\n";
  પાછા 0;
}

શૂન્ય Output::endSequence() {
  // If we did not emit anything, we should explicitly emit an empty sequence
  જો (State_ઢગલો.back() == inSeqFirstElement) {
    Padding = PaddingBeforeContainer;
    new_લીટી_Check(/*EmptySequence=*/સાચું);
    output("[]");
    Padding = "\n";
  }
  State_ઢગલો.કાઢો_back();
}

તાર્કિક Output::preflightElement(unsigned, શૂન્ય *&SaveInfo) {
  SaveInfo = nullptr;
  પાછા સાચું;
}

શૂન્ય Output::postflightElement(શૂન્ય *) {
  જો (State_ઢગલો.back() == inSeqFirstElement) {
    State_ઢગલો.કાઢો_back();
    State_ઢગલો.ધકેલો_back(inSeqOtherElement);
  } નહીંતર જો (State_ઢગલો.back() == inFlowSeqFirstElement) {
    State_ઢગલો.કાઢો_back();
    State_ઢગલો.ધકેલો_back(inFlowSeqOtherElement);
  }
}

unsigned Output::beginFlowSequence() {
  State_ઢગલો.ધકેલો_back(inFlowSeqFirstElement);
  new_લીટી_Check();
  ColumnAtFlowStart = Column;
  output("[ ");
  NeedFlowSequenceComma = ખોટું;
  પાછા 0;
}

શૂન્ય Output::endFlowSequence() {
  State_ઢગલો.કાઢો_back();
  output_Up_To_End_Of_લીટી(" ]");
}

તાર્કિક Output::preflightFlowElement(unsigned, શૂન્ય *&SaveInfo) {
  જો (NeedFlowSequenceComma)
    output(", ");
  જો (WrapColumn && Column > WrapColumn) {
    output("\n");
    માટે (પૂર્ણાંક i = 0; i < ColumnAtFlowStart; ++i)
      output(" ");
    Column = ColumnAtFlowStart;
    output("  ");
  }
  SaveInfo = nullptr;
  પાછા સાચું;
}

શૂન્ય Output::postflightFlowElement(શૂન્ય *) {
  NeedFlowSequenceComma = સાચું;
}

શૂન્ય Output::beginEnumScalar() {
  EnumerationMatchFound = ખોટું;
}

તાર્કિક Output::matchEnumScalar(StringRef Str, તાર્કિક Match) {
  જો (Match && !EnumerationMatchFound) {
    new_લીટી_Check();
    output_Up_To_End_Of_લીટી(Str);
    EnumerationMatchFound = સાચું;
  }
  પાછા ખોટું;
}

તાર્કિક Output::matchEnumFallback() {
  જો (EnumerationMatchFound)
    પાછા ખોટું;
  EnumerationMatchFound = સાચું;
  પાછા સાચું;
}

શૂન્ય Output::endEnumScalar() {
  જો (!EnumerationMatchFound)
    llvm_unreachable("bad runtime enum value");
}

તાર્કિક Output::beginBitSetScalar(તાર્કિક &DoClear) {
  new_લીટી_Check();
  output("[ ");
  Need_Bit_કિંમત_Comma = ખોટું;
  DoClear = ખોટું;
  પાછા સાચું;
}

તાર્કિક Output::bitSetMatch(StringRef Str, તાર્કિક Matches) {
  જો (Matches) {
    જો (Need_Bit_કિંમત_Comma)
      output(", ");
    output(Str);
    Need_Bit_કિંમત_Comma = સાચું;
  }
  પાછા ખોટું;
}

શૂન્ય Output::endBitSetScalar() {
  output_Up_To_End_Of_લીટી(" ]");
}

શૂન્ય Output::scalarString(StringRef &S, QuotingType MustQuote) {
  new_લીટી_Check();
  જો (S.empty()) {
    // Print '' for the empty string because leaving the field empty is not
    // allowed.
    output_Up_To_End_Of_લીટી("''");
    પાછા;
  }
  output(S, MustQuote);
  output_Up_To_End_Of_લીટી("");
}

શૂન્ય Output::blockScalarString(StringRef &S) {
  જો (!State_ઢગલો.empty())
    new_લીટી_Check();
  output(" |");

  unsigned Indent = State_ઢગલો.empty() ? 1 : State_ઢગલો.size();

  auto Buffer = MemoryBuffer::getMemBuffer(S, "", ખોટું);
  માટે (લીટી_iterator લીટીઓ(*Buffer, ખોટું); !લીટીઓ.is_at_end(); ++લીટીઓ) {
    output_New_લીટી();
    માટે (unsigned I = 0; I < Indent; ++I) {
      output("  ");
    }
    output(*લીટીઓ);
  }
  output_Up_To_End_Of_લીટી("");
}

શૂન્ય Output::scalarTag(std::string &Tag) {
  જો (Tag.empty())
    પાછા;
  new_લીટી_Check();
  output(Tag);
  output(" ");
}

શૂન્ય Output::setError(અચલ Twine &message) {
}

std::error_code Output::error() { પાછા {}; }

તાર્કિક Output::canElideEmptySequence() {
  // Normally, with an optional key/value where the value is an empty sequence,
  // the whole key/value can be not written.  But, that produces wrong yaml
  // if the key/value is the only thing in the map and the map is used in
  // a sequence.  This detects if the this sequence is the first key/value
  // in map that itself is embedded in a sequence.
  જો (State_ઢગલો.size() < 2)
    પાછા સાચું;
  જો (State_ઢગલો.back() != in_Map_First_ચાવી)
    પાછા સાચું;
  પાછા !inSeqAnyElement(State_ઢગલો[State_ઢગલો.size() - 2]);
}

શૂન્ય Output::output(StringRef s) {
  Column += s.size();
  Out << s;
}

શૂન્ય Output::output(StringRef S, QuotingType MustQuote) {
  જો (MustQuote == QuotingType::None) {
    // Only quote if we must.
    output(S);
    પાછા;
  }

  StringLiteral Quote = MustQuote == QuotingType::Single ? StringLiteral("'")
                                                         : StringLiteral("\"");
  output(Quote); // Starting quote.

  // When using double-quoted strings (and only in that case), non-printable
  // characters may be present, and will be escaped using a variety of
  // unicode-scalar and special short-form escapes. This is handled in
  // yaml::escape.
  જો (MustQuote == QuotingType::Double) {
    output(yaml::escape(S, /* EscapePrintable= */ ખોટું));
    output(Quote);
    પાછા;
  }

  unsigned i = 0;
  unsigned j = 0;
  unsigned End = S.size();
  અચલ અક્ષર *Base = S.data();

  // When using single-quoted strings, any single quote ' must be doubled to be
  // escaped.
  જ્યારે (j < End) {
    જો (S[j] == '\'') {                   // Escape quotes.
      output(StringRef(&Base[i], j - i)); // "flush".
      output(StringLiteral("''"));        // Print it as ''
      i = j + 1;
    }
    ++j;
  }
  output(StringRef(&Base[i], j - i));
  output(Quote); // Ending quote.
}

શૂન્ય Output::output_Up_To_End_Of_લીટી(StringRef s) {
  output(s);
  જો (State_ઢગલો.empty() || (!inFlowSeqAnyElement(State_ઢગલો.back()) &&
                             !in_Flow_Map_Any_ચાવી(State_ઢગલો.back())))
    Padding = "\n";
}

શૂન્ય Output::output_New_લીટી() {
  Out << "\n";
  Column = 0;
}

// if seq at top, indent as if map, then add "- "
// if seq in middle, use "- " if firstKey, else use "  "
//

શૂન્ય Output::new_લીટી_Check(તાર્કિક EmptySequence) {
  જો (Padding != "\n") {
    output(Padding);
    Padding = {};
    પાછા;
  }
  output_New_લીટી();
  Padding = {};

  જો (State_ઢગલો.size() == 0 || EmptySequence)
    પાછા;

  unsigned Indent = State_ઢગલો.size() - 1;
  તાર્કિક PossiblyNestedSeq = ખોટું;
  auto I = State_ઢગલો.rbegin(), E = State_ઢગલો.rend();

  જો (inSeqAnyElement(*I)) {
    PossiblyNestedSeq = સાચું; // Not possibly but always.
    ++Indent;
  } નહીંતર જો (*I == in_Map_First_ચાવી || *I == in_Flow_Map_First_ચાવી ||
             inFlowSeqAnyElement(*I)) {
    PossiblyNestedSeq = સાચું;
    ++I; // Skip back().
  }

  unsigned Output_Dash_ગણતરી = 0;
  જો (PossiblyNestedSeq) {
    // Count up consecutive inSeqFirstElement from the end, unless
    // inSeqFirstElement is the top of nested sequence.
    જ્યારે (I != E) {
      // Don't count the top of nested sequence.
      જો (!inSeqAnyElement(*I))
        તોડો;

      ++Output_Dash_ગણતરી;

      // Stop counting if consecutive inSeqFirstElement ends.
      જો (*I++ != inSeqFirstElement)
        તોડો;
    }
  }

  માટે (unsigned I = Output_Dash_ગણતરી; I < Indent; ++I)
    output("  ");

  માટે (unsigned I = 0; I < Output_Dash_ગણતરી; ++I)
    output("- ");
}

શૂન્ય Output::padded_ચાવી(StringRef ચાવી_2) {
  output(ચાવી_2, needsQuotes(ચાવી_2, ખોટું));
  output(":");
  અચલ અક્ષર *spaces = "                ";
  જો (ચાવી_2.size() < strlen(spaces))
    Padding = &spaces[ચાવી_2.size()];
  નહીંતર
    Padding = " ";
}

શૂન્ય Output::flow_ચાવી(StringRef ચાવી) {
  જો (State_ઢગલો.back() == in_Flow_Map_Other_ચાવી)
    output(", ");
  જો (WrapColumn && Column > WrapColumn) {
    output("\n");
    માટે (પૂર્ણાંક I = 0; I < ColumnAtMapFlowStart; ++I)
      output(" ");
    Column = ColumnAtMapFlowStart;
    output("  ");
  }
  output(ચાવી, needsQuotes(ચાવી, ખોટું));
  output(": ");
}

ગાંઠ_Kind Output::get_ગાંઠ_Kind() { report_fatal_error("invalid call"); }

તાર્કિક Output::inSeqAnyElement(InState State) {
  પાછા State == inSeqFirstElement || State == inSeqOtherElement;
}

તાર્કિક Output::inFlowSeqAnyElement(InState State) {
  પાછા State == inFlowSeqFirstElement || State == inFlowSeqOtherElement;
}

તાર્કિક Output::in_Map_Any_ચાવી(InState State) {
  પાછા State == in_Map_First_ચાવી || State == in_Map_Other_ચાવી;
}

તાર્કિક Output::in_Flow_Map_Any_ચાવી(InState State) {
  પાછા State == in_Flow_Map_First_ચાવી || State == in_Flow_Map_Other_ચાવી;
}

//===----------------------------------------------------------------------===//
//  traits for built-in types
//===----------------------------------------------------------------------===//

શૂન્ય ScalarTraits<તાર્કિક>::output(અચલ તાર્કિક &Val, શૂન્ય *, raw_ostream &Out) {
  Out << (Val ? "true" : "false");
}

StringRef ScalarTraits<તાર્કિક>::આગત_2(StringRef Scalar, શૂન્ય *, તાર્કિક &Val) {
  જો (std::optional<તાર્કિક> Parsed = parseBool(Scalar)) {
    Val = *Parsed;
    પાછા StringRef();
  }
  પાછા "invalid boolean";
}

શૂન્ય ScalarTraits<StringRef>::output(અચલ StringRef &Val, શૂન્ય *,
                                     raw_ostream &Out) {
  Out << Val;
}

StringRef ScalarTraits<StringRef>::આગત_2(StringRef Scalar, શૂન્ય *,
                                         StringRef &Val) {
  Val = Scalar;
  પાછા StringRef();
}

શૂન્ય ScalarTraits<std::string>::output(અચલ std::string &Val, શૂન્ય *,
                                       raw_ostream &Out) {
  Out << Val;
}

StringRef ScalarTraits<std::string>::આગત_2(StringRef Scalar, શૂન્ય *,
                                           std::string &Val) {
  Val = Scalar.str();
  પાછા StringRef();
}

શૂન્ય ScalarTraits<uint8_t>::output(અચલ uint8_t &Val, શૂન્ય *,
                                   raw_ostream &Out) {
  // use temp uin32_t because ostream thinks uint8_t is a character
  uint32_t અંક = Val;
  Out << અંક;
}

StringRef ScalarTraits<uint8_t>::આગત_2(StringRef Scalar, શૂન્ય *, uint8_t &Val) {
  unsigned long long n;
  જો (getAsUnsignedInteger(Scalar, 0, n))
    પાછા "invalid number";
  જો (n > 0xFF)
    પાછા "out of range number";
  Val = n;
  પાછા StringRef();
}

શૂન્ય ScalarTraits<uint16_t>::output(અચલ uint16_t &Val, શૂન્ય *,
                                    raw_ostream &Out) {
  Out << Val;
}

StringRef ScalarTraits<uint16_t>::આગત_2(StringRef Scalar, શૂન્ય *,
                                        uint16_t &Val) {
  unsigned long long n;
  જો (getAsUnsignedInteger(Scalar, 0, n))
    પાછા "invalid number";
  જો (n > 0xFFFF)
    પાછા "out of range number";
  Val = n;
  પાછા StringRef();
}

શૂન્ય ScalarTraits<uint32_t>::output(અચલ uint32_t &Val, શૂન્ય *,
                                    raw_ostream &Out) {
  Out << Val;
}

StringRef ScalarTraits<uint32_t>::આગત_2(StringRef Scalar, શૂન્ય *,
                                        uint32_t &Val) {
  unsigned long long n;
  જો (getAsUnsignedInteger(Scalar, 0, n))
    પાછા "invalid number";
  જો (n > 0xFFFFFFFFUL)
    પાછા "out of range number";
  Val = n;
  પાછા StringRef();
}

શૂન્ય ScalarTraits<uint64_t>::output(અચલ uint64_t &Val, શૂન્ય *,
                                    raw_ostream &Out) {
  Out << Val;
}

StringRef ScalarTraits<uint64_t>::આગત_2(StringRef Scalar, શૂન્ય *,
                                        uint64_t &Val) {
  unsigned long long N;
  જો (getAsUnsignedInteger(Scalar, 0, N))
    પાછા "invalid number";
  Val = N;
  પાછા StringRef();
}

શૂન્ય ScalarTraits<int8_t>::output(અચલ int8_t &Val, શૂન્ય *, raw_ostream &Out) {
  // use temp in32_t because ostream thinks int8_t is a character
  int32_t અંક = Val;
  Out << અંક;
}

StringRef ScalarTraits<int8_t>::આગત_2(StringRef Scalar, શૂન્ય *, int8_t &Val) {
  long long N;
  જો (getAsSignedInteger(Scalar, 0, N))
    પાછા "invalid number";
  જો ((N > 127) || (N < -128))
    પાછા "out of range number";
  Val = N;
  પાછા StringRef();
}

શૂન્ય ScalarTraits<int16_t>::output(અચલ int16_t &Val, શૂન્ય *,
                                   raw_ostream &Out) {
  Out << Val;
}

StringRef ScalarTraits<int16_t>::આગત_2(StringRef Scalar, શૂન્ય *, int16_t &Val) {
  long long N;
  જો (getAsSignedInteger(Scalar, 0, N))
    પાછા "invalid number";
  જો ((N > INT16_MAX) || (N < INT16_MIN))
    પાછા "out of range number";
  Val = N;
  પાછા StringRef();
}

શૂન્ય ScalarTraits<int32_t>::output(અચલ int32_t &Val, શૂન્ય *,
                                   raw_ostream &Out) {
  Out << Val;
}

StringRef ScalarTraits<int32_t>::આગત_2(StringRef Scalar, શૂન્ય *, int32_t &Val) {
  long long N;
  જો (getAsSignedInteger(Scalar, 0, N))
    પાછા "invalid number";
  જો ((N > INT32_MAX) || (N < INT32_MIN))
    પાછા "out of range number";
  Val = N;
  પાછા StringRef();
}

શૂન્ય ScalarTraits<int64_t>::output(અચલ int64_t &Val, શૂન્ય *,
                                   raw_ostream &Out) {
  Out << Val;
}

StringRef ScalarTraits<int64_t>::આગત_2(StringRef Scalar, શૂન્ય *, int64_t &Val) {
  long long N;
  જો (getAsSignedInteger(Scalar, 0, N))
    પાછા "invalid number";
  Val = N;
  પાછા StringRef();
}

શૂન્ય ScalarTraits<દ્વિગુણ>::output(અચલ દ્વિગુણ &Val, શૂન્ય *, raw_ostream &Out) {
  Out << format("%g", Val);
}

StringRef ScalarTraits<દ્વિગુણ>::આગત_2(StringRef Scalar, શૂન્ય *, દ્વિગુણ &Val) {
  જો (to_float(Scalar, Val))
    પાછા StringRef();
  પાછા "invalid floating point number";
}

શૂન્ય ScalarTraits<દશાંશ>::output(અચલ દશાંશ &Val, શૂન્ય *, raw_ostream &Out) {
  Out << format("%g", Val);
}

StringRef ScalarTraits<દશાંશ>::આગત_2(StringRef Scalar, શૂન્ય *, દશાંશ &Val) {
  જો (to_float(Scalar, Val))
    પાછા StringRef();
  પાછા "invalid floating point number";
}

શૂન્ય ScalarTraits<Hex8>::output(અચલ Hex8 &Val, શૂન્ય *, raw_ostream &Out) {
  Out << format("0x%" PRIX8, (uint8_t)Val);
}

StringRef ScalarTraits<Hex8>::આગત_2(StringRef Scalar, શૂન્ય *, Hex8 &Val) {
  unsigned long long n;
  જો (getAsUnsignedInteger(Scalar, 0, n))
    પાછા "invalid hex8 number";
  જો (n > 0xFF)
    પાછા "out of range hex8 number";
  Val = n;
  પાછા StringRef();
}

શૂન્ય ScalarTraits<Hex16>::output(અચલ Hex16 &Val, શૂન્ય *, raw_ostream &Out) {
  Out << format("0x%" PRIX16, (uint16_t)Val);
}

StringRef ScalarTraits<Hex16>::આગત_2(StringRef Scalar, શૂન્ય *, Hex16 &Val) {
  unsigned long long n;
  જો (getAsUnsignedInteger(Scalar, 0, n))
    પાછા "invalid hex16 number";
  જો (n > 0xFFFF)
    પાછા "out of range hex16 number";
  Val = n;
  પાછા StringRef();
}

શૂન્ય ScalarTraits<Hex32>::output(અચલ Hex32 &Val, શૂન્ય *, raw_ostream &Out) {
  Out << format("0x%" PRIX32, (uint32_t)Val);
}

StringRef ScalarTraits<Hex32>::આગત_2(StringRef Scalar, શૂન્ય *, Hex32 &Val) {
  unsigned long long n;
  જો (getAsUnsignedInteger(Scalar, 0, n))
    પાછા "invalid hex32 number";
  જો (n > 0xFFFFFFFFUL)
    પાછા "out of range hex32 number";
  Val = n;
  પાછા StringRef();
}

શૂન્ય ScalarTraits<Hex64>::output(અચલ Hex64 &Val, શૂન્ય *, raw_ostream &Out) {
  Out << format("0x%" PRIX64, (uint64_t)Val);
}

StringRef ScalarTraits<Hex64>::આગત_2(StringRef Scalar, શૂન્ય *, Hex64 &Val) {
  unsigned long long અંક;
  જો (getAsUnsignedInteger(Scalar, 0, અંક))
    પાછા "invalid hex64 number";
  Val = અંક;
  પાછા StringRef();
}

શૂન્ય ScalarTraits<VersionTuple>::output(અચલ VersionTuple &Val, શૂન્ય *,
                                        llvm::raw_ostream &Out) {
  Out << Val.getAsString();
}

StringRef ScalarTraits<VersionTuple>::આગત_2(StringRef Scalar, શૂન્ય *,
                                            VersionTuple &Val) {
  જો (Val.tryParse(Scalar))
    પાછા "invalid version format";
  પાછા StringRef();
}
