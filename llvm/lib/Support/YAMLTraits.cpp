//===- lib/Support/YAMLTraits.cpp -----------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#അന്തര്ഭാവ "llvm/Support/YAMLTraits.h"
#അന്തര്ഭാവ "llvm/ADT/STLExtras.h"
#അന്തര്ഭാവ "llvm/ADT/SmallString.h"
#അന്തര്ഭാവ "llvm/ADT/StringExtras.h"
#അന്തര്ഭാവ "llvm/ADT/StringRef.h"
#അന്തര്ഭാവ "llvm/ADT/Twine.h"
#അന്തര്ഭാവ "llvm/Support/Casting.h"
#അന്തര്ഭാവ "llvm/Support/Errc.h"
#അന്തര്ഭാവ "llvm/Support/ErrorHandling.h"
#അന്തര്ഭാവ "llvm/Support/Format.h"
#അന്തര്ഭാവ "llvm/Support/LineIterator.h"
#അന്തര്ഭാവ "llvm/Support/MemoryBuffer.h"
#അന്തര്ഭാവ "llvm/Support/VersionTuple.h"
#അന്തര്ഭാവ "llvm/Support/YAMLParser.h"
#അന്തര്ഭാവ "llvm/Support/raw_ostream.h"
#അന്തര്ഭാവ <cassert>
#അന്തര്ഭാവ <cstdint>
#അന്തര്ഭാവ <cstring>
#അന്തര്ഭാവ <string>
#അന്തര്ഭാവ <vector>

using നാമാകാശ llvm;
using നാമാകാശ yaml;

//===----------------------------------------------------------------------===//
//  IO
//===----------------------------------------------------------------------===//

IO::IO(ശൂന്യം *Context) : Ctxt(Context) {}

IO::~IO() = സ്ഥിരസ്ഥിതി;

ശൂന്യം *IO::getContext() സ്ഥിരം {
  തിരികെ Ctxt;
}

ശൂന്യം IO::setContext(ശൂന്യം *Context) {
  Ctxt = Context;
}

ശൂന്യം IO::set_Allow_Unknown_താക്കോലുകൾ(തർക്കം Allow) {
  llvm_unreachable("Only supported for Input");
}

//===----------------------------------------------------------------------===//
//നിവേശം
//===----------------------------------------------------------------------===//

നിവേശം::നിവേശം(StringRef നിവേശം_Content, ശൂന്യം *Ctxt,
             SourceMgr::DiagHandlerTy DiagHandler, ശൂന്യം *DiagHandlerCtxt)
    : IO(Ctxt), Strm(പുതിയ Stream(നിവേശം_Content, SrcMgr, അസത്യം, &EC)) {
  എങ്കിൽ (DiagHandler)
    SrcMgr.setDiagHandler(DiagHandler, DiagHandlerCtxt);
  DocIterator = Strm->begin();
}

നിവേശം::നിവേശം(MemoryBufferRef നിവേശം, ശൂന്യം *Ctxt,
             SourceMgr::DiagHandlerTy DiagHandler, ശൂന്യം *DiagHandlerCtxt)
    : IO(Ctxt), Strm(പുതിയ Stream(നിവേശം, SrcMgr, അസത്യം, &EC)) {
  എങ്കിൽ (DiagHandler)
    SrcMgr.setDiagHandler(DiagHandler, DiagHandlerCtxt);
  DocIterator = Strm->begin();
}

നിവേശം::~നിവേശം() = സ്ഥിരസ്ഥിതി;

std::error_code നിവേശം::error() { തിരികെ EC; }

തർക്കം നിവേശം::outputting() സ്ഥിരം {
  തിരികെ അസത്യം;
}

തർക്കം നിവേശം::setCurrentDocument() {
  എങ്കിൽ (DocIterator != Strm->end()) {
    കണ്ണി *N = DocIterator->get_വേര്();
    എങ്കിൽ (!N) {
      EC = make_error_code(errc::invalid_argument);
      തിരികെ അസത്യം;
    }

    എങ്കിൽ (isa<Null_കണ്ണി>(N)) {
      // Empty files are allowed and ignored
      ++DocIterator;
      തിരികെ setCurrentDocument();
    }
    releaseHNodeBuffers();
    മുകൾ_കണ്ണി = createHNodes(N);
    Current_കണ്ണി = മുകൾ_കണ്ണി;
    തിരികെ സത്യം;
  }
  തിരികെ അസത്യം;
}

തർക്കം നിവേശം::nextDocument() {
  തിരികെ ++DocIterator != Strm->end();
}

സ്ഥിരം കണ്ണി *നിവേശം::get_Current_കണ്ണി() സ്ഥിരം {
  തിരികെ Current_കണ്ണി ? Current_കണ്ണി->കണ്ണി_2 : nullptr;
}

തർക്കം നിവേശം::mapTag(StringRef Tag, തർക്കം Default) {
  // CurrentNode can be null if setCurrentDocument() was unable to
  // parse the document because it was invalid or empty.
  എങ്കിൽ (!Current_കണ്ണി)
    തിരികെ അസത്യം;

  std::string foundTag = Current_കണ്ണി->കണ്ണി_2->getVerbatimTag();
  എങ്കിൽ (foundTag.empty()) {
    // If no tag found and 'Tag' is the default, say it was found.
    തിരികെ Default;
  }
  // Return true iff found tag matches supplied tag.
  തിരികെ Tag == foundTag;
}

ശൂന്യം നിവേശം::beginMapping() {
  എങ്കിൽ (EC)
    തിരികെ;
  // CurrentNode can be null if the document is empty.
  MapHNode *MN = dyn_cast_or_null<MapHNode>(Current_കണ്ണി);
  എങ്കിൽ (MN) {
    MN->Valid_താക്കോലുകൾ.clear();
  }
}

std::vector<StringRef> നിവേശം::താക്കോലുകൾ() {
  MapHNode *MN = dyn_cast<MapHNode>(Current_കണ്ണി);
  std::vector<StringRef> Ret;
  എങ്കിൽ (!MN) {
    setError(Current_കണ്ണി, "not a mapping");
    തിരികെ Ret;
  }
  വേണ്ടി (auto &P : MN->Mapping)
    Ret.തള്ളുക_back(P.first());
  തിരികെ Ret;
}

തർക്കം നിവേശം::preflight_താക്കോൽ(StringRef താക്കോൽ, തർക്കം Required, തർക്കം, തർക്കം &UseDefault,
                         ശൂന്യം *&SaveInfo) {
  UseDefault = അസത്യം;
  എങ്കിൽ (EC)
    തിരികെ അസത്യം;

  // CurrentNode is null for empty documents, which is an error in case required
  // nodes are present.
  എങ്കിൽ (!Current_കണ്ണി) {
    എങ്കിൽ (Required)
      EC = make_error_code(errc::invalid_argument);
    അല്ലെങ്കിൽ
      UseDefault = സത്യം;
    തിരികെ അസത്യം;
  }

  MapHNode *MN = dyn_cast<MapHNode>(Current_കണ്ണി);
  എങ്കിൽ (!MN) {
    എങ്കിൽ (Required || !isa<EmptyHNode>(Current_കണ്ണി))
      setError(Current_കണ്ണി, "not a mapping");
    അല്ലെങ്കിൽ
      UseDefault = സത്യം;
    തിരികെ അസത്യം;
  }
  MN->Valid_താക്കോലുകൾ.തള്ളുക_back(താക്കോൽ.str());
  HNode *മൂല്യം = MN->Mapping[താക്കോൽ].first;
  എങ്കിൽ (!മൂല്യം) {
    എങ്കിൽ (Required)
      setError(Current_കണ്ണി, Twine("missing required key '") + താക്കോൽ + "'");
    അല്ലെങ്കിൽ
      UseDefault = സത്യം;
    തിരികെ അസത്യം;
  }
  SaveInfo = Current_കണ്ണി;
  Current_കണ്ണി = മൂല്യം;
  തിരികെ സത്യം;
}

ശൂന്യം നിവേശം::postflight_താക്കോൽ(ശൂന്യം *saveInfo) {
  Current_കണ്ണി = reinterpret_cast<HNode *>(saveInfo);
}

ശൂന്യം നിവേശം::endMapping() {
  എങ്കിൽ (EC)
    തിരികെ;
  // CurrentNode can be null if the document is empty.
  MapHNode *MN = dyn_cast_or_null<MapHNode>(Current_കണ്ണി);
  എങ്കിൽ (!MN)
    തിരികെ;
  വേണ്ടി (സ്ഥിരം auto &NN : MN->Mapping) {
    എങ്കിൽ (!is_contained(MN->Valid_താക്കോലുകൾ, NN.first())) {
      സ്ഥിരം SMRange &ReportLoc = NN.second.second;
      എങ്കിൽ (!Allow_Unknown_താക്കോലുകൾ) {
        setError(ReportLoc, Twine("unknown key '") + NN.first() + "'");
        നിർത്തുക;
      } അല്ലെങ്കിൽ
        reportWarning(ReportLoc, Twine("unknown key '") + NN.first() + "'");
    }
  }
}

ശൂന്യം നിവേശം::beginFlowMapping() { beginMapping(); }

ശൂന്യം നിവേശം::endFlowMapping() { endMapping(); }

unsigned നിവേശം::beginSequence() {
  എങ്കിൽ (SequenceHNode *SQ = dyn_cast<SequenceHNode>(Current_കണ്ണി))
    തിരികെ SQ->Entries.size();
  എങ്കിൽ (isa<EmptyHNode>(Current_കണ്ണി))
    തിരികെ 0;
  // Treat case where there's a scalar "null" value as an empty sequence.
  എങ്കിൽ (ScalarHNode *SN = dyn_cast<ScalarHNode>(Current_കണ്ണി)) {
    എങ്കിൽ (isNull(SN->മൂല്യം_2()))
      തിരികെ 0;
  }
  // Any other type of HNode is an error.
  setError(Current_കണ്ണി, "not a sequence");
  തിരികെ 0;
}

ശൂന്യം നിവേശം::endSequence() {
}

തർക്കം നിവേശം::preflightElement(unsigned Index, ശൂന്യം *&SaveInfo) {
  എങ്കിൽ (EC)
    തിരികെ അസത്യം;
  എങ്കിൽ (SequenceHNode *SQ = dyn_cast<SequenceHNode>(Current_കണ്ണി)) {
    SaveInfo = Current_കണ്ണി;
    Current_കണ്ണി = SQ->Entries[Index];
    തിരികെ സത്യം;
  }
  തിരികെ അസത്യം;
}

ശൂന്യം നിവേശം::postflightElement(ശൂന്യം *SaveInfo) {
  Current_കണ്ണി = reinterpret_cast<HNode *>(SaveInfo);
}

unsigned നിവേശം::beginFlowSequence() { തിരികെ beginSequence(); }

തർക്കം നിവേശം::preflightFlowElement(unsigned index, ശൂന്യം *&SaveInfo) {
  എങ്കിൽ (EC)
    തിരികെ അസത്യം;
  എങ്കിൽ (SequenceHNode *SQ = dyn_cast<SequenceHNode>(Current_കണ്ണി)) {
    SaveInfo = Current_കണ്ണി;
    Current_കണ്ണി = SQ->Entries[index];
    തിരികെ സത്യം;
  }
  തിരികെ അസത്യം;
}

ശൂന്യം നിവേശം::postflightFlowElement(ശൂന്യം *SaveInfo) {
  Current_കണ്ണി = reinterpret_cast<HNode *>(SaveInfo);
}

ശൂന്യം നിവേശം::endFlowSequence() {
}

ശൂന്യം നിവേശം::beginEnumScalar() {
  ScalarMatchFound = അസത്യം;
}

തർക്കം നിവേശം::matchEnumScalar(StringRef Str, തർക്കം) {
  എങ്കിൽ (ScalarMatchFound)
    തിരികെ അസത്യം;
  എങ്കിൽ (ScalarHNode *SN = dyn_cast<ScalarHNode>(Current_കണ്ണി)) {
    എങ്കിൽ (SN->മൂല്യം_2() == Str) {
      ScalarMatchFound = സത്യം;
      തിരികെ സത്യം;
    }
  }
  തിരികെ അസത്യം;
}

തർക്കം നിവേശം::matchEnumFallback() {
  എങ്കിൽ (ScalarMatchFound)
    തിരികെ അസത്യം;
  ScalarMatchFound = സത്യം;
  തിരികെ സത്യം;
}

ശൂന്യം നിവേശം::endEnumScalar() {
  എങ്കിൽ (!ScalarMatchFound) {
    setError(Current_കണ്ണി, "unknown enumerated scalar");
  }
}

തർക്കം നിവേശം::beginBitSetScalar(തർക്കം &DoClear) {
  Bit_മൂല്യങ്ങൾ_Used.clear();
  എങ്കിൽ (SequenceHNode *SQ = dyn_cast<SequenceHNode>(Current_കണ്ണി)) {
    Bit_മൂല്യങ്ങൾ_Used.resize(SQ->Entries.size());
  } അല്ലെങ്കിൽ {
    setError(Current_കണ്ണി, "expected sequence of bit values");
  }
  DoClear = സത്യം;
  തിരികെ സത്യം;
}

തർക്കം നിവേശം::bitSetMatch(StringRef Str, തർക്കം) {
  എങ്കിൽ (EC)
    തിരികെ അസത്യം;
  എങ്കിൽ (SequenceHNode *SQ = dyn_cast<SequenceHNode>(Current_കണ്ണി)) {
    unsigned Index = 0;
    വേണ്ടി (auto &N : SQ->Entries) {
      എങ്കിൽ (ScalarHNode *SN = dyn_cast<ScalarHNode>(N)) {
        എങ്കിൽ (SN->മൂല്യം_2() == Str) {
          Bit_മൂല്യങ്ങൾ_Used[Index] = സത്യം;
          തിരികെ സത്യം;
        }
      } അല്ലെങ്കിൽ {
        setError(Current_കണ്ണി, "unexpected scalar in sequence of bit values");
      }
      ++Index;
    }
  } അല്ലെങ്കിൽ {
    setError(Current_കണ്ണി, "expected sequence of bit values");
  }
  തിരികെ അസത്യം;
}

ശൂന്യം നിവേശം::endBitSetScalar() {
  എങ്കിൽ (EC)
    തിരികെ;
  എങ്കിൽ (SequenceHNode *SQ = dyn_cast<SequenceHNode>(Current_കണ്ണി)) {
    പ്രതിജ്ഞാ(Bit_മൂല്യങ്ങൾ_Used.size() == SQ->Entries.size());
    വേണ്ടി (unsigned i = 0; i < SQ->Entries.size(); ++i) {
      എങ്കിൽ (!Bit_മൂല്യങ്ങൾ_Used[i]) {
        setError(SQ->Entries[i], "unknown bit value");
        തിരികെ;
      }
    }
  }
}

ശൂന്യം നിവേശം::scalarString(StringRef &S, QuotingType) {
  എങ്കിൽ (ScalarHNode *SN = dyn_cast<ScalarHNode>(Current_കണ്ണി)) {
    S = SN->മൂല്യം_2();
  } അല്ലെങ്കിൽ {
    setError(Current_കണ്ണി, "unexpected scalar");
  }
}

ശൂന്യം നിവേശം::blockScalarString(StringRef &S) { scalarString(S, QuotingType::None); }

ശൂന്യം നിവേശം::scalarTag(std::string &Tag) {
  Tag = Current_കണ്ണി->കണ്ണി_2->getVerbatimTag();
}

ശൂന്യം നിവേശം::setError(HNode *hnode, സ്ഥിരം Twine &message) {
  പ്രതിജ്ഞാ(hnode && "HNode must not be NULL");
  setError(hnode->കണ്ണി_2, message);
}

കണ്ണി_Kind നിവേശം::get_കണ്ണി_Kind() {
  എങ്കിൽ (isa<ScalarHNode>(Current_കണ്ണി))
    തിരികെ കണ്ണി_Kind::Scalar;
  അല്ലെങ്കിൽ എങ്കിൽ (isa<MapHNode>(Current_കണ്ണി))
    തിരികെ കണ്ണി_Kind::Map;
  അല്ലെങ്കിൽ എങ്കിൽ (isa<SequenceHNode>(Current_കണ്ണി))
    തിരികെ കണ്ണി_Kind::Sequence;
  llvm_unreachable("Unsupported node kind");
}

ശൂന്യം നിവേശം::setError(കണ്ണി *കണ്ണി_3, സ്ഥിരം Twine &message) {
  Strm->printError(കണ്ണി_3, message);
  EC = make_error_code(errc::invalid_argument);
}

ശൂന്യം നിവേശം::setError(സ്ഥിരം SMRange &range, സ്ഥിരം Twine &message) {
  Strm->printError(range, message);
  EC = make_error_code(errc::invalid_argument);
}

ശൂന്യം നിവേശം::reportWarning(HNode *hnode, സ്ഥിരം Twine &message) {
  പ്രതിജ്ഞാ(hnode && "HNode must not be NULL");
  Strm->printError(hnode->കണ്ണി_2, message, SourceMgr::DK_Warning);
}

ശൂന്യം നിവേശം::reportWarning(കണ്ണി *കണ്ണി_3, സ്ഥിരം Twine &message) {
  Strm->printError(കണ്ണി_3, message, SourceMgr::DK_Warning);
}

ശൂന്യം നിവേശം::reportWarning(സ്ഥിരം SMRange &range, സ്ഥിരം Twine &message) {
  Strm->printError(range, message, SourceMgr::DK_Warning);
}

ശൂന്യം നിവേശം::releaseHNodeBuffers() {
  EmptyHNodeAllocator.DestroyAll();
  ScalarHNodeAllocator.DestroyAll();
  SequenceHNodeAllocator.DestroyAll();
  MapHNodeAllocator.DestroyAll();
}

ശൂന്യം നിവേശം::saveAliasHNode(കണ്ണി *N, HNode *HN) {
  StringRef Anchor = N->getAnchor();
  എങ്കിൽ (!Anchor.empty())
    // YAML 1.2.2 - 3.2.2.2. Anchors and Aliases:
    //
    // An alias event refers to the most recent event in the serialization
    // having the specified anchor. Therefore, anchors need not be unique within
    // a serialization. In addition, an anchor need not have an alias node
    // referring to it.
    AliasMap[Anchor] = HN;
}

നിവേശം::HNode *നിവേശം::createHNodes(കണ്ണി *N) {
  SmallString<128> StringStorage;
  തിരഞ്ഞെടുപ്പ് (N->getType()) {
  അവസ്ഥ കണ്ണി::NK_Scalar: {
    Scalar_കണ്ണി *SN = dyn_cast<Scalar_കണ്ണി>(N);
    StringRef താക്കോൽ_Str = SN->get_മൂല്യം(StringStorage);
    എങ്കിൽ (!StringStorage.empty()) {
      // Copy string to permanent storage
      താക്കോൽ_Str = StringStorage.str().copy(StringAllocator);
    }
    auto *SHNode = പുതിയ (ScalarHNodeAllocator.Allocate()) ScalarHNode(N, താക്കോൽ_Str);
    saveAliasHNode(SN, SHNode);
    തിരികെ SHNode;
  }
  അവസ്ഥ കണ്ണി::NK_BlockScalar: {
    Block_Scalar_കണ്ണി *BSN = dyn_cast<Block_Scalar_കണ്ണി>(N);
    StringRef മൂല്യം_Copy = BSN->get_മൂല്യം().copy(StringAllocator);
    auto *BSHNode =
        പുതിയ (ScalarHNodeAllocator.Allocate()) ScalarHNode(N, മൂല്യം_Copy);
    saveAliasHNode(BSN, BSHNode);
    തിരികെ BSHNode;
  }
  അവസ്ഥ കണ്ണി::NK_Sequence: {
    Sequence_കണ്ണി *SQ = dyn_cast<Sequence_കണ്ണി>(N);
    auto SQHNode = പുതിയ (SequenceHNodeAllocator.Allocate()) SequenceHNode(N);
    വേണ്ടി (കണ്ണി &SN : *SQ) {
      auto Entry = createHNodes(&SN);
      എങ്കിൽ (EC)
        നിർത്തുക;
      SQHNode->Entries.തള്ളുക_back(Entry);
    }
    saveAliasHNode(SQ, SQHNode);
    തിരികെ SQHNode;
  }
  അവസ്ഥ കണ്ണി::NK_Mapping: {
    Mapping_കണ്ണി *Map = dyn_cast<Mapping_കണ്ണി>(N);
    auto mapHNode = പുതിയ (MapHNodeAllocator.Allocate()) MapHNode(N);
    വേണ്ടി (താക്കോൽ_മൂല്യം_കണ്ണി &KVN : *Map) {
      കണ്ണി *താക്കോൽ_കണ്ണി = KVN.get_താക്കോൽ();
      Scalar_കണ്ണി *താക്കോൽ = dyn_cast_if_present<Scalar_കണ്ണി>(താക്കോൽ_കണ്ണി);
      കണ്ണി *മൂല്യം = KVN.get_മൂല്യം();
      എങ്കിൽ (!താക്കോൽ || !മൂല്യം) {
        എങ്കിൽ (!താക്കോൽ)
          setError(താക്കോൽ_കണ്ണി, "Map key must be a scalar");
        എങ്കിൽ (!മൂല്യം)
          setError(താക്കോൽ_കണ്ണി, "Map value must not be empty");
        നിർത്തുക;
      }
      StringStorage.clear();
      StringRef താക്കോൽ_Str = താക്കോൽ->get_മൂല്യം(StringStorage);
      എങ്കിൽ (!StringStorage.empty()) {
        // Copy string to permanent storage
        താക്കോൽ_Str = StringStorage.str().copy(StringAllocator);
      }
      എങ്കിൽ (mapHNode->Mapping.എണ്ണം(താക്കോൽ_Str))
        // From YAML spec: "The content of a mapping node is an unordered set of
        // key/value node pairs, with the restriction that each of the keys is
        // unique."
        setError(താക്കോൽ_കണ്ണി, Twine("duplicated mapping key '") + താക്കോൽ_Str + "'");
      auto മൂല്യം_HNode = createHNodes(മൂല്യം);
      എങ്കിൽ (EC)
        നിർത്തുക;
      mapHNode->Mapping[താക്കോൽ_Str] =
          std::make_pair(std::move(മൂല്യം_HNode), താക്കോൽ_കണ്ണി->getSourceRange());
    }
    saveAliasHNode(Map, mapHNode);
    തിരികെ std::move(mapHNode);
  }
  അവസ്ഥ കണ്ണി::NK_Null:
    // TODO: Anchor is not set for NullNode in the parser. Update the parser to
    // faithfully preserve anchors.
    തിരികെ പുതിയ (EmptyHNodeAllocator.Allocate()) EmptyHNode(N);
  അവസ്ഥ കണ്ണി::NK_Alias: {
    Alias_കണ്ണി *AN = dyn_cast<Alias_കണ്ണി>(N);
    auto AliasName = AN->getName();
    auto AHN = AliasMap.find(AliasName);
    എങ്കിൽ (AHN == AliasMap.end()) {
      setError(AN, Twine("undefined alias '" + AliasName + "'"));
      തിരികെ nullptr;
    }
    തിരികെ AHN->second;
  }
  സ്ഥിരസ്ഥിതി:
    setError(N, "unknown node kind");
    തിരികെ nullptr;
  }
}

ശൂന്യം നിവേശം::setError(സ്ഥിരം Twine &Message) {
  setError(Current_കണ്ണി, Message);
}

ശൂന്യം നിവേശം::set_Allow_Unknown_താക്കോലുകൾ(തർക്കം Allow) { Allow_Unknown_താക്കോലുകൾ = Allow; }

തർക്കം നിവേശം::canElideEmptySequence() {
  തിരികെ അസത്യം;
}

//===----------------------------------------------------------------------===//
//  Output
//===----------------------------------------------------------------------===//

Output::Output(raw_ostream &yout, ശൂന്യം *context, പൂർണ്ണസംഖ്യ WrapColumn)
    : IO(context), Out(yout), WrapColumn(WrapColumn) {}

Output::~Output() = സ്ഥിരസ്ഥിതി;

തർക്കം Output::outputting() സ്ഥിരം {
  തിരികെ സത്യം;
}

ശൂന്യം Output::beginMapping() {
  State_അട്ടി.തള്ളുക_back(in_Map_First_താക്കോൽ);
  PaddingBeforeContainer = Padding;
  Padding = "\n";
}

തർക്കം Output::mapTag(StringRef Tag, തർക്കം Use) {
  എങ്കിൽ (Use) {
    // If this tag is being written inside a sequence we should write the start
    // of the sequence before writing the tag, otherwise the tag won't be
    // attached to the element in the sequence, but rather the sequence itself.
    തർക്കം SequenceElement = അസത്യം;
    എങ്കിൽ (State_അട്ടി.size() > 1) {
      auto &E = State_അട്ടി[State_അട്ടി.size() - 2];
      SequenceElement = inSeqAnyElement(E) || inFlowSeqAnyElement(E);
    }
    എങ്കിൽ (SequenceElement && State_അട്ടി.back() == in_Map_First_താക്കോൽ) {
      new_വരി_Check();
    } അല്ലെങ്കിൽ {
      output(" ");
    }
    output(Tag);
    എങ്കിൽ (SequenceElement) {
      // If we're writing the tag during the first element of a map, the tag
      // takes the place of the first element in the sequence.
      എങ്കിൽ (State_അട്ടി.back() == in_Map_First_താക്കോൽ) {
        State_അട്ടി.എടുക്കുക_back();
        State_അട്ടി.തള്ളുക_back(in_Map_Other_താക്കോൽ);
      }
      // Tags inside maps in sequences should act as keys in the map from a
      // formatting perspective, so we always want a newline in a sequence.
      Padding = "\n";
    }
  }
  തിരികെ Use;
}

ശൂന്യം Output::endMapping() {
  // If we did not map anything, we should explicitly emit an empty map
  എങ്കിൽ (State_അട്ടി.back() == in_Map_First_താക്കോൽ) {
    Padding = PaddingBeforeContainer;
    new_വരി_Check();
    output("{}");
    Padding = "\n";
  }
  State_അട്ടി.എടുക്കുക_back();
}

std::vector<StringRef> Output::താക്കോലുകൾ() {
  report_fatal_error("invalid call");
}

തർക്കം Output::preflight_താക്കോൽ(StringRef താക്കോൽ, തർക്കം Required, തർക്കം SameAsDefault,
                          തർക്കം &UseDefault, ശൂന്യം *&SaveInfo) {
  UseDefault = അസത്യം;
  SaveInfo = nullptr;
  എങ്കിൽ (Required || !SameAsDefault || Write_Default_മൂല്യങ്ങൾ) {
    auto State = State_അട്ടി.back();
    എങ്കിൽ (State == in_Flow_Map_First_താക്കോൽ || State == in_Flow_Map_Other_താക്കോൽ) {
      flow_താക്കോൽ(താക്കോൽ);
    } അല്ലെങ്കിൽ {
      new_വരി_Check();
      padded_താക്കോൽ(താക്കോൽ);
    }
    തിരികെ സത്യം;
  }
  തിരികെ അസത്യം;
}

ശൂന്യം Output::postflight_താക്കോൽ(ശൂന്യം *) {
  എങ്കിൽ (State_അട്ടി.back() == in_Map_First_താക്കോൽ) {
    State_അട്ടി.എടുക്കുക_back();
    State_അട്ടി.തള്ളുക_back(in_Map_Other_താക്കോൽ);
  } അല്ലെങ്കിൽ എങ്കിൽ (State_അട്ടി.back() == in_Flow_Map_First_താക്കോൽ) {
    State_അട്ടി.എടുക്കുക_back();
    State_അട്ടി.തള്ളുക_back(in_Flow_Map_Other_താക്കോൽ);
  }
}

ശൂന്യം Output::beginFlowMapping() {
  State_അട്ടി.തള്ളുക_back(in_Flow_Map_First_താക്കോൽ);
  new_വരി_Check();
  ColumnAtMapFlowStart = Column;
  output("{ ");
}

ശൂന്യം Output::endFlowMapping() {
  State_അട്ടി.എടുക്കുക_back();
  output_Up_To_End_Of_വരി(" }");
}

ശൂന്യം Output::beginDocuments() {
  output_Up_To_End_Of_വരി("---");
}

തർക്കം Output::preflightDocument(unsigned index) {
  എങ്കിൽ (index > 0)
    output_Up_To_End_Of_വരി("\n---");
  തിരികെ സത്യം;
}

ശൂന്യം Output::postflightDocument() {
}

ശൂന്യം Output::endDocuments() {
  output("\n...\n");
}

unsigned Output::beginSequence() {
  State_അട്ടി.തള്ളുക_back(inSeqFirstElement);
  PaddingBeforeContainer = Padding;
  Padding = "\n";
  തിരികെ 0;
}

ശൂന്യം Output::endSequence() {
  // If we did not emit anything, we should explicitly emit an empty sequence
  എങ്കിൽ (State_അട്ടി.back() == inSeqFirstElement) {
    Padding = PaddingBeforeContainer;
    new_വരി_Check(/*EmptySequence=*/സത്യം);
    output("[]");
    Padding = "\n";
  }
  State_അട്ടി.എടുക്കുക_back();
}

തർക്കം Output::preflightElement(unsigned, ശൂന്യം *&SaveInfo) {
  SaveInfo = nullptr;
  തിരികെ സത്യം;
}

ശൂന്യം Output::postflightElement(ശൂന്യം *) {
  എങ്കിൽ (State_അട്ടി.back() == inSeqFirstElement) {
    State_അട്ടി.എടുക്കുക_back();
    State_അട്ടി.തള്ളുക_back(inSeqOtherElement);
  } അല്ലെങ്കിൽ എങ്കിൽ (State_അട്ടി.back() == inFlowSeqFirstElement) {
    State_അട്ടി.എടുക്കുക_back();
    State_അട്ടി.തള്ളുക_back(inFlowSeqOtherElement);
  }
}

unsigned Output::beginFlowSequence() {
  State_അട്ടി.തള്ളുക_back(inFlowSeqFirstElement);
  new_വരി_Check();
  ColumnAtFlowStart = Column;
  output("[ ");
  NeedFlowSequenceComma = അസത്യം;
  തിരികെ 0;
}

ശൂന്യം Output::endFlowSequence() {
  State_അട്ടി.എടുക്കുക_back();
  output_Up_To_End_Of_വരി(" ]");
}

തർക്കം Output::preflightFlowElement(unsigned, ശൂന്യം *&SaveInfo) {
  എങ്കിൽ (NeedFlowSequenceComma)
    output(", ");
  എങ്കിൽ (WrapColumn && Column > WrapColumn) {
    output("\n");
    വേണ്ടി (പൂർണ്ണസംഖ്യ i = 0; i < ColumnAtFlowStart; ++i)
      output(" ");
    Column = ColumnAtFlowStart;
    output("  ");
  }
  SaveInfo = nullptr;
  തിരികെ സത്യം;
}

ശൂന്യം Output::postflightFlowElement(ശൂന്യം *) {
  NeedFlowSequenceComma = സത്യം;
}

ശൂന്യം Output::beginEnumScalar() {
  EnumerationMatchFound = അസത്യം;
}

തർക്കം Output::matchEnumScalar(StringRef Str, തർക്കം Match) {
  എങ്കിൽ (Match && !EnumerationMatchFound) {
    new_വരി_Check();
    output_Up_To_End_Of_വരി(Str);
    EnumerationMatchFound = സത്യം;
  }
  തിരികെ അസത്യം;
}

തർക്കം Output::matchEnumFallback() {
  എങ്കിൽ (EnumerationMatchFound)
    തിരികെ അസത്യം;
  EnumerationMatchFound = സത്യം;
  തിരികെ സത്യം;
}

ശൂന്യം Output::endEnumScalar() {
  എങ്കിൽ (!EnumerationMatchFound)
    llvm_unreachable("bad runtime enum value");
}

തർക്കം Output::beginBitSetScalar(തർക്കം &DoClear) {
  new_വരി_Check();
  output("[ ");
  Need_Bit_മൂല്യം_Comma = അസത്യം;
  DoClear = അസത്യം;
  തിരികെ സത്യം;
}

തർക്കം Output::bitSetMatch(StringRef Str, തർക്കം Matches) {
  എങ്കിൽ (Matches) {
    എങ്കിൽ (Need_Bit_മൂല്യം_Comma)
      output(", ");
    output(Str);
    Need_Bit_മൂല്യം_Comma = സത്യം;
  }
  തിരികെ അസത്യം;
}

ശൂന്യം Output::endBitSetScalar() {
  output_Up_To_End_Of_വരി(" ]");
}

ശൂന്യം Output::scalarString(StringRef &S, QuotingType MustQuote) {
  new_വരി_Check();
  എങ്കിൽ (S.empty()) {
    // Print '' for the empty string because leaving the field empty is not
    // allowed.
    output_Up_To_End_Of_വരി("''");
    തിരികെ;
  }
  output(S, MustQuote);
  output_Up_To_End_Of_വരി("");
}

ശൂന്യം Output::blockScalarString(StringRef &S) {
  എങ്കിൽ (!State_അട്ടി.empty())
    new_വരി_Check();
  output(" |");

  unsigned Indent = State_അട്ടി.empty() ? 1 : State_അട്ടി.size();

  auto Buffer = MemoryBuffer::getMemBuffer(S, "", അസത്യം);
  വേണ്ടി (വരി_iterator വരികൾ(*Buffer, അസത്യം); !വരികൾ.is_at_end(); ++വരികൾ) {
    output_New_വരി();
    വേണ്ടി (unsigned I = 0; I < Indent; ++I) {
      output("  ");
    }
    output(*വരികൾ);
  }
  output_Up_To_End_Of_വരി("");
}

ശൂന്യം Output::scalarTag(std::string &Tag) {
  എങ്കിൽ (Tag.empty())
    തിരികെ;
  new_വരി_Check();
  output(Tag);
  output(" ");
}

ശൂന്യം Output::setError(സ്ഥിരം Twine &message) {
}

std::error_code Output::error() { തിരികെ {}; }

തർക്കം Output::canElideEmptySequence() {
  // Normally, with an optional key/value where the value is an empty sequence,
  // the whole key/value can be not written.  But, that produces wrong yaml
  // if the key/value is the only thing in the map and the map is used in
  // a sequence.  This detects if the this sequence is the first key/value
  // in map that itself is embedded in a sequence.
  എങ്കിൽ (State_അട്ടി.size() < 2)
    തിരികെ സത്യം;
  എങ്കിൽ (State_അട്ടി.back() != in_Map_First_താക്കോൽ)
    തിരികെ സത്യം;
  തിരികെ !inSeqAnyElement(State_അട്ടി[State_അട്ടി.size() - 2]);
}

ശൂന്യം Output::output(StringRef s) {
  Column += s.size();
  Out << s;
}

ശൂന്യം Output::output(StringRef S, QuotingType MustQuote) {
  എങ്കിൽ (MustQuote == QuotingType::None) {
    // Only quote if we must.
    output(S);
    തിരികെ;
  }

  StringLiteral Quote = MustQuote == QuotingType::Single ? StringLiteral("'")
                                                         : StringLiteral("\"");
  output(Quote); // Starting quote.

  // When using double-quoted strings (and only in that case), non-printable
  // characters may be present, and will be escaped using a variety of
  // unicode-scalar and special short-form escapes. This is handled in
  // yaml::escape.
  എങ്കിൽ (MustQuote == QuotingType::Double) {
    output(yaml::escape(S, /* EscapePrintable= */ അസത്യം));
    output(Quote);
    തിരികെ;
  }

  unsigned i = 0;
  unsigned j = 0;
  unsigned End = S.size();
  സ്ഥിരം അക്ഷരം *Base = S.data();

  // When using single-quoted strings, any single quote ' must be doubled to be
  // escaped.
  ആകുമ്പോൾ (j < End) {
    എങ്കിൽ (S[j] == '\'') {                   // Escape quotes.
      output(StringRef(&Base[i], j - i)); // "flush".
      output(StringLiteral("''"));        // Print it as ''
      i = j + 1;
    }
    ++j;
  }
  output(StringRef(&Base[i], j - i));
  output(Quote); // Ending quote.
}

ശൂന്യം Output::output_Up_To_End_Of_വരി(StringRef s) {
  output(s);
  എങ്കിൽ (State_അട്ടി.empty() || (!inFlowSeqAnyElement(State_അട്ടി.back()) &&
                             !in_Flow_Map_Any_താക്കോൽ(State_അട്ടി.back())))
    Padding = "\n";
}

ശൂന്യം Output::output_New_വരി() {
  Out << "\n";
  Column = 0;
}

// if seq at top, indent as if map, then add "- "
// if seq in middle, use "- " if firstKey, else use "  "
//

ശൂന്യം Output::new_വരി_Check(തർക്കം EmptySequence) {
  എങ്കിൽ (Padding != "\n") {
    output(Padding);
    Padding = {};
    തിരികെ;
  }
  output_New_വരി();
  Padding = {};

  എങ്കിൽ (State_അട്ടി.size() == 0 || EmptySequence)
    തിരികെ;

  unsigned Indent = State_അട്ടി.size() - 1;
  തർക്കം PossiblyNestedSeq = അസത്യം;
  auto I = State_അട്ടി.rbegin(), E = State_അട്ടി.rend();

  എങ്കിൽ (inSeqAnyElement(*I)) {
    PossiblyNestedSeq = സത്യം; // Not possibly but always.
    ++Indent;
  } അല്ലെങ്കിൽ എങ്കിൽ (*I == in_Map_First_താക്കോൽ || *I == in_Flow_Map_First_താക്കോൽ ||
             inFlowSeqAnyElement(*I)) {
    PossiblyNestedSeq = സത്യം;
    ++I; // Skip back().
  }

  unsigned Output_Dash_എണ്ണം = 0;
  എങ്കിൽ (PossiblyNestedSeq) {
    // Count up consecutive inSeqFirstElement from the end, unless
    // inSeqFirstElement is the top of nested sequence.
    ആകുമ്പോൾ (I != E) {
      // Don't count the top of nested sequence.
      എങ്കിൽ (!inSeqAnyElement(*I))
        നിർത്തുക;

      ++Output_Dash_എണ്ണം;

      // Stop counting if consecutive inSeqFirstElement ends.
      എങ്കിൽ (*I++ != inSeqFirstElement)
        നിർത്തുക;
    }
  }

  വേണ്ടി (unsigned I = Output_Dash_എണ്ണം; I < Indent; ++I)
    output("  ");

  വേണ്ടി (unsigned I = 0; I < Output_Dash_എണ്ണം; ++I)
    output("- ");
}

ശൂന്യം Output::padded_താക്കോൽ(StringRef താക്കോൽ_2) {
  output(താക്കോൽ_2, needsQuotes(താക്കോൽ_2, അസത്യം));
  output(":");
  സ്ഥിരം അക്ഷരം *spaces = "                ";
  എങ്കിൽ (താക്കോൽ_2.size() < strlen(spaces))
    Padding = &spaces[താക്കോൽ_2.size()];
  അല്ലെങ്കിൽ
    Padding = " ";
}

ശൂന്യം Output::flow_താക്കോൽ(StringRef താക്കോൽ) {
  എങ്കിൽ (State_അട്ടി.back() == in_Flow_Map_Other_താക്കോൽ)
    output(", ");
  എങ്കിൽ (WrapColumn && Column > WrapColumn) {
    output("\n");
    വേണ്ടി (പൂർണ്ണസംഖ്യ I = 0; I < ColumnAtMapFlowStart; ++I)
      output(" ");
    Column = ColumnAtMapFlowStart;
    output("  ");
  }
  output(താക്കോൽ, needsQuotes(താക്കോൽ, അസത്യം));
  output(": ");
}

കണ്ണി_Kind Output::get_കണ്ണി_Kind() { report_fatal_error("invalid call"); }

തർക്കം Output::inSeqAnyElement(InState State) {
  തിരികെ State == inSeqFirstElement || State == inSeqOtherElement;
}

തർക്കം Output::inFlowSeqAnyElement(InState State) {
  തിരികെ State == inFlowSeqFirstElement || State == inFlowSeqOtherElement;
}

തർക്കം Output::in_Map_Any_താക്കോൽ(InState State) {
  തിരികെ State == in_Map_First_താക്കോൽ || State == in_Map_Other_താക്കോൽ;
}

തർക്കം Output::in_Flow_Map_Any_താക്കോൽ(InState State) {
  തിരികെ State == in_Flow_Map_First_താക്കോൽ || State == in_Flow_Map_Other_താക്കോൽ;
}

//===----------------------------------------------------------------------===//
//  traits for built-in types
//===----------------------------------------------------------------------===//

ശൂന്യം ScalarTraits<തർക്കം>::output(സ്ഥിരം തർക്കം &Val, ശൂന്യം *, raw_ostream &Out) {
  Out << (Val ? "true" : "false");
}

StringRef ScalarTraits<തർക്കം>::നിവേശം_2(StringRef Scalar, ശൂന്യം *, തർക്കം &Val) {
  എങ്കിൽ (std::optional<തർക്കം> Parsed = parseBool(Scalar)) {
    Val = *Parsed;
    തിരികെ StringRef();
  }
  തിരികെ "invalid boolean";
}

ശൂന്യം ScalarTraits<StringRef>::output(സ്ഥിരം StringRef &Val, ശൂന്യം *,
                                     raw_ostream &Out) {
  Out << Val;
}

StringRef ScalarTraits<StringRef>::നിവേശം_2(StringRef Scalar, ശൂന്യം *,
                                         StringRef &Val) {
  Val = Scalar;
  തിരികെ StringRef();
}

ശൂന്യം ScalarTraits<std::string>::output(സ്ഥിരം std::string &Val, ശൂന്യം *,
                                       raw_ostream &Out) {
  Out << Val;
}

StringRef ScalarTraits<std::string>::നിവേശം_2(StringRef Scalar, ശൂന്യം *,
                                           std::string &Val) {
  Val = Scalar.str();
  തിരികെ StringRef();
}

ശൂന്യം ScalarTraits<uint8_t>::output(സ്ഥിരം uint8_t &Val, ശൂന്യം *,
                                   raw_ostream &Out) {
  // use temp uin32_t because ostream thinks uint8_t is a character
  uint32_t അക്കം = Val;
  Out << അക്കം;
}

StringRef ScalarTraits<uint8_t>::നിവേശം_2(StringRef Scalar, ശൂന്യം *, uint8_t &Val) {
  unsigned long long n;
  എങ്കിൽ (getAsUnsignedInteger(Scalar, 0, n))
    തിരികെ "invalid number";
  എങ്കിൽ (n > 0xFF)
    തിരികെ "out of range number";
  Val = n;
  തിരികെ StringRef();
}

ശൂന്യം ScalarTraits<uint16_t>::output(സ്ഥിരം uint16_t &Val, ശൂന്യം *,
                                    raw_ostream &Out) {
  Out << Val;
}

StringRef ScalarTraits<uint16_t>::നിവേശം_2(StringRef Scalar, ശൂന്യം *,
                                        uint16_t &Val) {
  unsigned long long n;
  എങ്കിൽ (getAsUnsignedInteger(Scalar, 0, n))
    തിരികെ "invalid number";
  എങ്കിൽ (n > 0xFFFF)
    തിരികെ "out of range number";
  Val = n;
  തിരികെ StringRef();
}

ശൂന്യം ScalarTraits<uint32_t>::output(സ്ഥിരം uint32_t &Val, ശൂന്യം *,
                                    raw_ostream &Out) {
  Out << Val;
}

StringRef ScalarTraits<uint32_t>::നിവേശം_2(StringRef Scalar, ശൂന്യം *,
                                        uint32_t &Val) {
  unsigned long long n;
  എങ്കിൽ (getAsUnsignedInteger(Scalar, 0, n))
    തിരികെ "invalid number";
  എങ്കിൽ (n > 0xFFFFFFFFUL)
    തിരികെ "out of range number";
  Val = n;
  തിരികെ StringRef();
}

ശൂന്യം ScalarTraits<uint64_t>::output(സ്ഥിരം uint64_t &Val, ശൂന്യം *,
                                    raw_ostream &Out) {
  Out << Val;
}

StringRef ScalarTraits<uint64_t>::നിവേശം_2(StringRef Scalar, ശൂന്യം *,
                                        uint64_t &Val) {
  unsigned long long N;
  എങ്കിൽ (getAsUnsignedInteger(Scalar, 0, N))
    തിരികെ "invalid number";
  Val = N;
  തിരികെ StringRef();
}

ശൂന്യം ScalarTraits<int8_t>::output(സ്ഥിരം int8_t &Val, ശൂന്യം *, raw_ostream &Out) {
  // use temp in32_t because ostream thinks int8_t is a character
  int32_t അക്കം = Val;
  Out << അക്കം;
}

StringRef ScalarTraits<int8_t>::നിവേശം_2(StringRef Scalar, ശൂന്യം *, int8_t &Val) {
  long long N;
  എങ്കിൽ (getAsSignedInteger(Scalar, 0, N))
    തിരികെ "invalid number";
  എങ്കിൽ ((N > 127) || (N < -128))
    തിരികെ "out of range number";
  Val = N;
  തിരികെ StringRef();
}

ശൂന്യം ScalarTraits<int16_t>::output(സ്ഥിരം int16_t &Val, ശൂന്യം *,
                                   raw_ostream &Out) {
  Out << Val;
}

StringRef ScalarTraits<int16_t>::നിവേശം_2(StringRef Scalar, ശൂന്യം *, int16_t &Val) {
  long long N;
  എങ്കിൽ (getAsSignedInteger(Scalar, 0, N))
    തിരികെ "invalid number";
  എങ്കിൽ ((N > INT16_MAX) || (N < INT16_MIN))
    തിരികെ "out of range number";
  Val = N;
  തിരികെ StringRef();
}

ശൂന്യം ScalarTraits<int32_t>::output(സ്ഥിരം int32_t &Val, ശൂന്യം *,
                                   raw_ostream &Out) {
  Out << Val;
}

StringRef ScalarTraits<int32_t>::നിവേശം_2(StringRef Scalar, ശൂന്യം *, int32_t &Val) {
  long long N;
  എങ്കിൽ (getAsSignedInteger(Scalar, 0, N))
    തിരികെ "invalid number";
  എങ്കിൽ ((N > INT32_MAX) || (N < INT32_MIN))
    തിരികെ "out of range number";
  Val = N;
  തിരികെ StringRef();
}

ശൂന്യം ScalarTraits<int64_t>::output(സ്ഥിരം int64_t &Val, ശൂന്യം *,
                                   raw_ostream &Out) {
  Out << Val;
}

StringRef ScalarTraits<int64_t>::നിവേശം_2(StringRef Scalar, ശൂന്യം *, int64_t &Val) {
  long long N;
  എങ്കിൽ (getAsSignedInteger(Scalar, 0, N))
    തിരികെ "invalid number";
  Val = N;
  തിരികെ StringRef();
}

ശൂന്യം ScalarTraits<ഇരട്ട>::output(സ്ഥിരം ഇരട്ട &Val, ശൂന്യം *, raw_ostream &Out) {
  Out << format("%g", Val);
}

StringRef ScalarTraits<ഇരട്ട>::നിവേശം_2(StringRef Scalar, ശൂന്യം *, ഇരട്ട &Val) {
  എങ്കിൽ (to_float(Scalar, Val))
    തിരികെ StringRef();
  തിരികെ "invalid floating point number";
}

ശൂന്യം ScalarTraits<ദശാംശം>::output(സ്ഥിരം ദശാംശം &Val, ശൂന്യം *, raw_ostream &Out) {
  Out << format("%g", Val);
}

StringRef ScalarTraits<ദശാംശം>::നിവേശം_2(StringRef Scalar, ശൂന്യം *, ദശാംശം &Val) {
  എങ്കിൽ (to_float(Scalar, Val))
    തിരികെ StringRef();
  തിരികെ "invalid floating point number";
}

ശൂന്യം ScalarTraits<Hex8>::output(സ്ഥിരം Hex8 &Val, ശൂന്യം *, raw_ostream &Out) {
  Out << format("0x%" PRIX8, (uint8_t)Val);
}

StringRef ScalarTraits<Hex8>::നിവേശം_2(StringRef Scalar, ശൂന്യം *, Hex8 &Val) {
  unsigned long long n;
  എങ്കിൽ (getAsUnsignedInteger(Scalar, 0, n))
    തിരികെ "invalid hex8 number";
  എങ്കിൽ (n > 0xFF)
    തിരികെ "out of range hex8 number";
  Val = n;
  തിരികെ StringRef();
}

ശൂന്യം ScalarTraits<Hex16>::output(സ്ഥിരം Hex16 &Val, ശൂന്യം *, raw_ostream &Out) {
  Out << format("0x%" PRIX16, (uint16_t)Val);
}

StringRef ScalarTraits<Hex16>::നിവേശം_2(StringRef Scalar, ശൂന്യം *, Hex16 &Val) {
  unsigned long long n;
  എങ്കിൽ (getAsUnsignedInteger(Scalar, 0, n))
    തിരികെ "invalid hex16 number";
  എങ്കിൽ (n > 0xFFFF)
    തിരികെ "out of range hex16 number";
  Val = n;
  തിരികെ StringRef();
}

ശൂന്യം ScalarTraits<Hex32>::output(സ്ഥിരം Hex32 &Val, ശൂന്യം *, raw_ostream &Out) {
  Out << format("0x%" PRIX32, (uint32_t)Val);
}

StringRef ScalarTraits<Hex32>::നിവേശം_2(StringRef Scalar, ശൂന്യം *, Hex32 &Val) {
  unsigned long long n;
  എങ്കിൽ (getAsUnsignedInteger(Scalar, 0, n))
    തിരികെ "invalid hex32 number";
  എങ്കിൽ (n > 0xFFFFFFFFUL)
    തിരികെ "out of range hex32 number";
  Val = n;
  തിരികെ StringRef();
}

ശൂന്യം ScalarTraits<Hex64>::output(സ്ഥിരം Hex64 &Val, ശൂന്യം *, raw_ostream &Out) {
  Out << format("0x%" PRIX64, (uint64_t)Val);
}

StringRef ScalarTraits<Hex64>::നിവേശം_2(StringRef Scalar, ശൂന്യം *, Hex64 &Val) {
  unsigned long long അക്കം;
  എങ്കിൽ (getAsUnsignedInteger(Scalar, 0, അക്കം))
    തിരികെ "invalid hex64 number";
  Val = അക്കം;
  തിരികെ StringRef();
}

ശൂന്യം ScalarTraits<VersionTuple>::output(സ്ഥിരം VersionTuple &Val, ശൂന്യം *,
                                        llvm::raw_ostream &Out) {
  Out << Val.getAsString();
}

StringRef ScalarTraits<VersionTuple>::നിവേശം_2(StringRef Scalar, ശൂന്യം *,
                                            VersionTuple &Val) {
  എങ്കിൽ (Val.tryParse(Scalar))
    തിരികെ "invalid version format";
  തിരികെ StringRef();
}
