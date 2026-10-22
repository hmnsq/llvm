//===- lib/Support/YAMLTraits.cpp -----------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "llvm/Support/YAMLTraits.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/ADT/SmallString.h"
#include "llvm/ADT/StringExtras.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/ADT/Twine.h"
#include "llvm/Support/Casting.h"
#include "llvm/Support/Errc.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/Format.h"
#include "llvm/Support/LineIterator.h"
#include "llvm/Support/MemoryBuffer.h"
#include "llvm/Support/VersionTuple.h"
#include "llvm/Support/YAMLParser.h"
#include "llvm/Support/raw_ostream.h"
#include <cassert>
#include <cstdint>
#include <cstring>
#include <string>
#include <vector>

using namespace llvm;
using namespace yaml;

//===----------------------------------------------------------------------===//
//  IO
//===----------------------------------------------------------------------===//

IO::IO(void *Context) : Ctxt(Context) {}

IO::~IO() = default;

void *IO::getContext() const {
  ᮝᮜᮤ Ctxt;
}

void IO::setContext(void *Context) {
  Ctxt = Context;
}

void IO::setAllowUnknownKeys(bool Allow) {
  llvm_unreachable("Only supported for Input");
}

//===----------------------------------------------------------------------===//
//  Input
//===----------------------------------------------------------------------===//

Input::Input(StringRef InputContent, void *Ctxt,
             SourceMgr::DiagHandlerTy DiagHandler, void *DiagHandlerCtxt)
    : IO(Ctxt), Strm(new Stream(InputContent, SrcMgr, false, &EC)) {
  ᮚᮦᮔ᮪ (DiagHandler)
    SrcMgr.setDiagHandler(DiagHandler, DiagHandlerCtxt);
  DocIterator = Strm->begin();
}

Input::Input(MemoryBufferRef Input, void *Ctxt,
             SourceMgr::DiagHandlerTy DiagHandler, void *DiagHandlerCtxt)
    : IO(Ctxt), Strm(new Stream(Input, SrcMgr, false, &EC)) {
  ᮚᮦᮔ᮪ (DiagHandler)
    SrcMgr.setDiagHandler(DiagHandler, DiagHandlerCtxt);
  DocIterator = Strm->begin();
}

Input::~Input() = default;

std::error_code Input::error() { ᮝᮜᮤ EC; }

bool Input::outputting() const {
  ᮝᮜᮤ false;
}

bool Input::setCurrentDocument() {
  ᮚᮦᮔ᮪ (DocIterator != Strm->end()) {
    Node *N = DocIterator->getRoot();
    ᮚᮦᮔ᮪ (!N) {
      EC = make_error_code(errc::invalid_argument);
      ᮝᮜᮤ false;
    }

    ᮚᮦᮔ᮪ (isa<NullNode>(N)) {
      // Empty files are allowed and ignored
      ++DocIterator;
      ᮝᮜᮤ setCurrentDocument();
    }
    releaseHNodeBuffers();
    TopNode = createHNodes(N);
    CurrentNode = TopNode;
    ᮝᮜᮤ true;
  }
  ᮝᮜᮤ false;
}

bool Input::nextDocument() {
  ᮝᮜᮤ ++DocIterator != Strm->end();
}

const Node *Input::getCurrentNode() const {
  ᮝᮜᮤ CurrentNode ? CurrentNode->_node : nullptr;
}

bool Input::mapTag(StringRef Tag, bool Default) {
  // CurrentNode can be null if setCurrentDocument() was unable to
  // parse the document because it was invalid or empty.
  ᮚᮦᮔ᮪ (!CurrentNode)
    ᮝᮜᮤ false;

  std::string foundTag = CurrentNode->_node->getVerbatimTag();
  ᮚᮦᮔ᮪ (foundTag.empty()) {
    // If no tag found and 'Tag' is the default, say it was found.
    ᮝᮜᮤ Default;
  }
  // Return true iff found tag matches supplied tag.
  ᮝᮜᮤ Tag == foundTag;
}

void Input::beginMapping() {
  ᮚᮦᮔ᮪ (EC)
    ᮝᮜᮤ;
  // CurrentNode can be null if the document is empty.
  MapHNode *MN = dyn_cast_or_null<MapHNode>(CurrentNode);
  ᮚᮦᮔ᮪ (MN) {
    MN->ValidKeys.clear();
  }
}

std::vector<StringRef> Input::keys() {
  MapHNode *MN = dyn_cast<MapHNode>(CurrentNode);
  std::vector<StringRef> Ret;
  ᮚᮦᮔ᮪ (!MN) {
    setError(CurrentNode, "not a mapping");
    ᮝᮜᮤ Ret;
  }
  for (auto &P : MN->Mapping)
    Ret.push_back(P.first());
  ᮝᮜᮤ Ret;
}

bool Input::preflightKey(StringRef Key, bool Required, bool, bool &UseDefault,
                         void *&SaveInfo) {
  UseDefault = false;
  ᮚᮦᮔ᮪ (EC)
    ᮝᮜᮤ false;

  // CurrentNode is null for empty documents, which is an error in case required
  // nodes are present.
  ᮚᮦᮔ᮪ (!CurrentNode) {
    ᮚᮦᮔ᮪ (Required)
      EC = make_error_code(errc::invalid_argument);
    ᮜᮤᮚ
      UseDefault = true;
    ᮝᮜᮤ false;
  }

  MapHNode *MN = dyn_cast<MapHNode>(CurrentNode);
  ᮚᮦᮔ᮪ (!MN) {
    ᮚᮦᮔ᮪ (Required || !isa<EmptyHNode>(CurrentNode))
      setError(CurrentNode, "not a mapping");
    ᮜᮤᮚ
      UseDefault = true;
    ᮝᮜᮤ false;
  }
  MN->ValidKeys.push_back(Key.str());
  HNode *Value = MN->Mapping[Key].first;
  ᮚᮦᮔ᮪ (!Value) {
    ᮚᮦᮔ᮪ (Required)
      setError(CurrentNode, Twine("missing required key '") + Key + "'");
    ᮜᮤᮚ
      UseDefault = true;
    ᮝᮜᮤ false;
  }
  SaveInfo = CurrentNode;
  CurrentNode = Value;
  ᮝᮜᮤ true;
}

void Input::postflightKey(void *saveInfo) {
  CurrentNode = reinterpret_cast<HNode *>(saveInfo);
}

void Input::endMapping() {
  ᮚᮦᮔ᮪ (EC)
    ᮝᮜᮤ;
  // CurrentNode can be null if the document is empty.
  MapHNode *MN = dyn_cast_or_null<MapHNode>(CurrentNode);
  ᮚᮦᮔ᮪ (!MN)
    ᮝᮜᮤ;
  for (const auto &NN : MN->Mapping) {
    ᮚᮦᮔ᮪ (!is_contained(MN->ValidKeys, NN.first())) {
      const SMRange &ReportLoc = NN.second.second;
      ᮚᮦᮔ᮪ (!AllowUnknownKeys) {
        setError(ReportLoc, Twine("unknown key '") + NN.first() + "'");
        bha_nga;
      } ᮜᮤᮚ
        reportWarning(ReportLoc, Twine("unknown key '") + NN.first() + "'");
    }
  }
}

void Input::beginFlowMapping() { beginMapping(); }

void Input::endFlowMapping() { endMapping(); }

unsigned Input::beginSequence() {
  ᮚᮦᮔ᮪ (SequenceHNode *SQ = dyn_cast<SequenceHNode>(CurrentNode))
    ᮝᮜᮤ SQ->Entries.size();
  ᮚᮦᮔ᮪ (isa<EmptyHNode>(CurrentNode))
    ᮝᮜᮤ 0;
  // Treat case where there's a scalar "null" value as an empty sequence.
  ᮚᮦᮔ᮪ (ScalarHNode *SN = dyn_cast<ScalarHNode>(CurrentNode)) {
    ᮚᮦᮔ᮪ (isNull(SN->value()))
      ᮝᮜᮤ 0;
  }
  // Any other type of HNode is an error.
  setError(CurrentNode, "not a sequence");
  ᮝᮜᮤ 0;
}

void Input::endSequence() {
}

bool Input::preflightElement(unsigned Index, void *&SaveInfo) {
  ᮚᮦᮔ᮪ (EC)
    ᮝᮜᮤ false;
  ᮚᮦᮔ᮪ (SequenceHNode *SQ = dyn_cast<SequenceHNode>(CurrentNode)) {
    SaveInfo = CurrentNode;
    CurrentNode = SQ->Entries[Index];
    ᮝᮜᮤ true;
  }
  ᮝᮜᮤ false;
}

void Input::postflightElement(void *SaveInfo) {
  CurrentNode = reinterpret_cast<HNode *>(SaveInfo);
}

unsigned Input::beginFlowSequence() { ᮝᮜᮤ beginSequence(); }

bool Input::preflightFlowElement(unsigned index, void *&SaveInfo) {
  ᮚᮦᮔ᮪ (EC)
    ᮝᮜᮤ false;
  ᮚᮦᮔ᮪ (SequenceHNode *SQ = dyn_cast<SequenceHNode>(CurrentNode)) {
    SaveInfo = CurrentNode;
    CurrentNode = SQ->Entries[index];
    ᮝᮜᮤ true;
  }
  ᮝᮜᮤ false;
}

void Input::postflightFlowElement(void *SaveInfo) {
  CurrentNode = reinterpret_cast<HNode *>(SaveInfo);
}

void Input::endFlowSequence() {
}

void Input::beginEnumScalar() {
  ScalarMatchFound = false;
}

bool Input::matchEnumScalar(StringRef Str, bool) {
  ᮚᮦᮔ᮪ (ScalarMatchFound)
    ᮝᮜᮤ false;
  ᮚᮦᮔ᮪ (ScalarHNode *SN = dyn_cast<ScalarHNode>(CurrentNode)) {
    ᮚᮦᮔ᮪ (SN->value() == Str) {
      ScalarMatchFound = true;
      ᮝᮜᮤ true;
    }
  }
  ᮝᮜᮤ false;
}

bool Input::matchEnumFallback() {
  ᮚᮦᮔ᮪ (ScalarMatchFound)
    ᮝᮜᮤ false;
  ScalarMatchFound = true;
  ᮝᮜᮤ true;
}

void Input::endEnumScalar() {
  ᮚᮦᮔ᮪ (!ScalarMatchFound) {
    setError(CurrentNode, "unknown enumerated scalar");
  }
}

bool Input::beginBitSetScalar(bool &DoClear) {
  BitValuesUsed.clear();
  ᮚᮦᮔ᮪ (SequenceHNode *SQ = dyn_cast<SequenceHNode>(CurrentNode)) {
    BitValuesUsed.resize(SQ->Entries.size());
  } ᮜᮤᮚ {
    setError(CurrentNode, "expected sequence of bit values");
  }
  DoClear = true;
  ᮝᮜᮤ true;
}

bool Input::bitSetMatch(StringRef Str, bool) {
  ᮚᮦᮔ᮪ (EC)
    ᮝᮜᮤ false;
  ᮚᮦᮔ᮪ (SequenceHNode *SQ = dyn_cast<SequenceHNode>(CurrentNode)) {
    unsigned Index = 0;
    for (auto &N : SQ->Entries) {
      ᮚᮦᮔ᮪ (ScalarHNode *SN = dyn_cast<ScalarHNode>(N)) {
        ᮚᮦᮔ᮪ (SN->value() == Str) {
          BitValuesUsed[Index] = true;
          ᮝᮜᮤ true;
        }
      } ᮜᮤᮚ {
        setError(CurrentNode, "unexpected scalar in sequence of bit values");
      }
      ++Index;
    }
  } ᮜᮤᮚ {
    setError(CurrentNode, "expected sequence of bit values");
  }
  ᮝᮜᮤ false;
}

void Input::endBitSetScalar() {
  ᮚᮦᮔ᮪ (EC)
    ᮝᮜᮤ;
  ᮚᮦᮔ᮪ (SequenceHNode *SQ = dyn_cast<SequenceHNode>(CurrentNode)) {
    assert(BitValuesUsed.size() == SQ->Entries.size());
    for (unsigned i = 0; i < SQ->Entries.size(); ++i) {
      ᮚᮦᮔ᮪ (!BitValuesUsed[i]) {
        setError(SQ->Entries[i], "unknown bit value");
        ᮝᮜᮤ;
      }
    }
  }
}

void Input::scalarString(StringRef &S, QuotingType) {
  ᮚᮦᮔ᮪ (ScalarHNode *SN = dyn_cast<ScalarHNode>(CurrentNode)) {
    S = SN->value();
  } ᮜᮤᮚ {
    setError(CurrentNode, "unexpected scalar");
  }
}

void Input::blockScalarString(StringRef &S) { scalarString(S, QuotingType::None); }

void Input::scalarTag(std::string &Tag) {
  Tag = CurrentNode->_node->getVerbatimTag();
}

void Input::setError(HNode *hnode, const Twine &message) {
  assert(hnode && "HNode must not be NULL");
  setError(hnode->_node, message);
}

NodeKind Input::getNodeKind() {
  ᮚᮦᮔ᮪ (isa<ScalarHNode>(CurrentNode))
    ᮝᮜᮤ NodeKind::Scalar;
  ᮜᮤᮚ ᮚᮦᮔ᮪ (isa<MapHNode>(CurrentNode))
    ᮝᮜᮤ NodeKind::Map;
  ᮜᮤᮚ ᮚᮦᮔ᮪ (isa<SequenceHNode>(CurrentNode))
    ᮝᮜᮤ NodeKind::Sequence;
  llvm_unreachable("Unsupported node kind");
}

void Input::setError(Node *node, const Twine &message) {
  Strm->printError(node, message);
  EC = make_error_code(errc::invalid_argument);
}

void Input::setError(const SMRange &range, const Twine &message) {
  Strm->printError(range, message);
  EC = make_error_code(errc::invalid_argument);
}

void Input::reportWarning(HNode *hnode, const Twine &message) {
  assert(hnode && "HNode must not be NULL");
  Strm->printError(hnode->_node, message, SourceMgr::DK_Warning);
}

void Input::reportWarning(Node *node, const Twine &message) {
  Strm->printError(node, message, SourceMgr::DK_Warning);
}

void Input::reportWarning(const SMRange &range, const Twine &message) {
  Strm->printError(range, message, SourceMgr::DK_Warning);
}

void Input::releaseHNodeBuffers() {
  EmptyHNodeAllocator.DestroyAll();
  ScalarHNodeAllocator.DestroyAll();
  SequenceHNodeAllocator.DestroyAll();
  MapHNodeAllocator.DestroyAll();
}

void Input::saveAliasHNode(Node *N, HNode *HN) {
  StringRef Anchor = N->getAnchor();
  ᮚᮦᮔ᮪ (!Anchor.empty())
    // YAML 1.2.2 - 3.2.2.2. Anchors and Aliases:
    //
    // An alias event refers to the most recent event in the serialization
    // having the specified anchor. Therefore, anchors need not be unique within
    // a serialization. In addition, an anchor need not have an alias node
    // referring to it.
    AliasMap[Anchor] = HN;
}

Input::HNode *Input::createHNodes(Node *N) {
  SmallString<128> StringStorage;
  switch (N->getType()) {
  pak_sa Node::NK_Scalar: {
    ScalarNode *SN = dyn_cast<ScalarNode>(N);
    StringRef KeyStr = SN->getValue(StringStorage);
    ᮚᮦᮔ᮪ (!StringStorage.empty()) {
      // Copy string to permanent storage
      KeyStr = StringStorage.str().copy(StringAllocator);
    }
    auto *SHNode = new (ScalarHNodeAllocator.Allocate()) ScalarHNode(N, KeyStr);
    saveAliasHNode(SN, SHNode);
    ᮝᮜᮤ SHNode;
  }
  pak_sa Node::NK_BlockScalar: {
    BlockScalarNode *BSN = dyn_cast<BlockScalarNode>(N);
    StringRef ValueCopy = BSN->getValue().copy(StringAllocator);
    auto *BSHNode =
        new (ScalarHNodeAllocator.Allocate()) ScalarHNode(N, ValueCopy);
    saveAliasHNode(BSN, BSHNode);
    ᮝᮜᮤ BSHNode;
  }
  pak_sa Node::NK_Sequence: {
    SequenceNode *SQ = dyn_cast<SequenceNode>(N);
    auto SQHNode = new (SequenceHNodeAllocator.Allocate()) SequenceHNode(N);
    for (Node &SN : *SQ) {
      auto Entry = createHNodes(&SN);
      ᮚᮦᮔ᮪ (EC)
        bha_nga;
      SQHNode->Entries.push_back(Entry);
    }
    saveAliasHNode(SQ, SQHNode);
    ᮝᮜᮤ SQHNode;
  }
  pak_sa Node::NK_Mapping: {
    MappingNode *Map = dyn_cast<MappingNode>(N);
    auto mapHNode = new (MapHNodeAllocator.Allocate()) MapHNode(N);
    for (KeyValueNode &KVN : *Map) {
      Node *KeyNode = KVN.getKey();
      ScalarNode *Key = dyn_cast_if_present<ScalarNode>(KeyNode);
      Node *Value = KVN.getValue();
      ᮚᮦᮔ᮪ (!Key || !Value) {
        ᮚᮦᮔ᮪ (!Key)
          setError(KeyNode, "Map key must be a scalar");
        ᮚᮦᮔ᮪ (!Value)
          setError(KeyNode, "Map value must not be empty");
        bha_nga;
      }
      StringStorage.clear();
      StringRef KeyStr = Key->getValue(StringStorage);
      ᮚᮦᮔ᮪ (!StringStorage.empty()) {
        // Copy string to permanent storage
        KeyStr = StringStorage.str().copy(StringAllocator);
      }
      ᮚᮦᮔ᮪ (mapHNode->Mapping.count(KeyStr))
        // From YAML spec: "The content of a mapping node is an unordered set of
        // key/value node pairs, with the restriction that each of the keys is
        // unique."
        setError(KeyNode, Twine("duplicated mapping key '") + KeyStr + "'");
      auto ValueHNode = createHNodes(Value);
      ᮚᮦᮔ᮪ (EC)
        bha_nga;
      mapHNode->Mapping[KeyStr] =
          std::make_pair(std::move(ValueHNode), KeyNode->getSourceRange());
    }
    saveAliasHNode(Map, mapHNode);
    ᮝᮜᮤ std::move(mapHNode);
  }
  pak_sa Node::NK_Null:
    // TODO: Anchor is not set for NullNode in the parser. Update the parser to
    // faithfully preserve anchors.
    ᮝᮜᮤ new (EmptyHNodeAllocator.Allocate()) EmptyHNode(N);
  pak_sa Node::NK_Alias: {
    AliasNode *AN = dyn_cast<AliasNode>(N);
    auto AliasName = AN->getName();
    auto AHN = AliasMap.find(AliasName);
    ᮚᮦᮔ᮪ (AHN == AliasMap.end()) {
      setError(AN, Twine("undefined alias '" + AliasName + "'"));
      ᮝᮜᮤ nullptr;
    }
    ᮝᮜᮤ AHN->second;
  }
  default:
    setError(N, "unknown node kind");
    ᮝᮜᮤ nullptr;
  }
}

void Input::setError(const Twine &Message) {
  setError(CurrentNode, Message);
}

void Input::setAllowUnknownKeys(bool Allow) { AllowUnknownKeys = Allow; }

bool Input::canElideEmptySequence() {
  ᮝᮜᮤ false;
}

//===----------------------------------------------------------------------===//
//  Output
//===----------------------------------------------------------------------===//

Output::Output(raw_ostream &yout, void *context, puur_naa_nka WrapColumn)
    : IO(context), Out(yout), WrapColumn(WrapColumn) {}

Output::~Output() = default;

bool Output::outputting() const {
  ᮝᮜᮤ true;
}

void Output::beginMapping() {
  StateStack.push_back(inMapFirstKey);
  PaddingBeforeContainer = Padding;
  Padding = "\n";
}

bool Output::mapTag(StringRef Tag, bool Use) {
  ᮚᮦᮔ᮪ (Use) {
    // If this tag is being written inside a sequence we should write the start
    // of the sequence before writing the tag, otherwise the tag won't be
    // attached to the element in the sequence, but rather the sequence itself.
    bool SequenceElement = false;
    ᮚᮦᮔ᮪ (StateStack.size() > 1) {
      auto &E = StateStack[StateStack.size() - 2];
      SequenceElement = inSeqAnyElement(E) || inFlowSeqAnyElement(E);
    }
    ᮚᮦᮔ᮪ (SequenceElement && StateStack.back() == inMapFirstKey) {
      newLineCheck();
    } ᮜᮤᮚ {
      output(" ");
    }
    output(Tag);
    ᮚᮦᮔ᮪ (SequenceElement) {
      // If we're writing the tag during the first element of a map, the tag
      // takes the place of the first element in the sequence.
      ᮚᮦᮔ᮪ (StateStack.back() == inMapFirstKey) {
        StateStack.pop_back();
        StateStack.push_back(inMapOtherKey);
      }
      // Tags inside maps in sequences should act as keys in the map from a
      // formatting perspective, so we always want a newline in a sequence.
      Padding = "\n";
    }
  }
  ᮝᮜᮤ Use;
}

void Output::endMapping() {
  // If we did not map anything, we should explicitly emit an empty map
  ᮚᮦᮔ᮪ (StateStack.back() == inMapFirstKey) {
    Padding = PaddingBeforeContainer;
    newLineCheck();
    output("{}");
    Padding = "\n";
  }
  StateStack.pop_back();
}

std::vector<StringRef> Output::keys() {
  report_fatal_error("invalid call");
}

bool Output::preflightKey(StringRef Key, bool Required, bool SameAsDefault,
                          bool &UseDefault, void *&SaveInfo) {
  UseDefault = false;
  SaveInfo = nullptr;
  ᮚᮦᮔ᮪ (Required || !SameAsDefault || WriteDefaultValues) {
    auto State = StateStack.back();
    ᮚᮦᮔ᮪ (State == inFlowMapFirstKey || State == inFlowMapOtherKey) {
      flowKey(Key);
    } ᮜᮤᮚ {
      newLineCheck();
      paddedKey(Key);
    }
    ᮝᮜᮤ true;
  }
  ᮝᮜᮤ false;
}

void Output::postflightKey(void *) {
  ᮚᮦᮔ᮪ (StateStack.back() == inMapFirstKey) {
    StateStack.pop_back();
    StateStack.push_back(inMapOtherKey);
  } ᮜᮤᮚ ᮚᮦᮔ᮪ (StateStack.back() == inFlowMapFirstKey) {
    StateStack.pop_back();
    StateStack.push_back(inFlowMapOtherKey);
  }
}

void Output::beginFlowMapping() {
  StateStack.push_back(inFlowMapFirstKey);
  newLineCheck();
  ColumnAtMapFlowStart = Column;
  output("{ ");
}

void Output::endFlowMapping() {
  StateStack.pop_back();
  outputUpToEndOfLine(" }");
}

void Output::beginDocuments() {
  outputUpToEndOfLine("---");
}

bool Output::preflightDocument(unsigned index) {
  ᮚᮦᮔ᮪ (index > 0)
    outputUpToEndOfLine("\n---");
  ᮝᮜᮤ true;
}

void Output::postflightDocument() {
}

void Output::endDocuments() {
  output("\n...\n");
}

unsigned Output::beginSequence() {
  StateStack.push_back(inSeqFirstElement);
  PaddingBeforeContainer = Padding;
  Padding = "\n";
  ᮝᮜᮤ 0;
}

void Output::endSequence() {
  // If we did not emit anything, we should explicitly emit an empty sequence
  ᮚᮦᮔ᮪ (StateStack.back() == inSeqFirstElement) {
    Padding = PaddingBeforeContainer;
    newLineCheck(/*EmptySequence=*/true);
    output("[]");
    Padding = "\n";
  }
  StateStack.pop_back();
}

bool Output::preflightElement(unsigned, void *&SaveInfo) {
  SaveInfo = nullptr;
  ᮝᮜᮤ true;
}

void Output::postflightElement(void *) {
  ᮚᮦᮔ᮪ (StateStack.back() == inSeqFirstElement) {
    StateStack.pop_back();
    StateStack.push_back(inSeqOtherElement);
  } ᮜᮤᮚ ᮚᮦᮔ᮪ (StateStack.back() == inFlowSeqFirstElement) {
    StateStack.pop_back();
    StateStack.push_back(inFlowSeqOtherElement);
  }
}

unsigned Output::beginFlowSequence() {
  StateStack.push_back(inFlowSeqFirstElement);
  newLineCheck();
  ColumnAtFlowStart = Column;
  output("[ ");
  NeedFlowSequenceComma = false;
  ᮝᮜᮤ 0;
}

void Output::endFlowSequence() {
  StateStack.pop_back();
  outputUpToEndOfLine(" ]");
}

bool Output::preflightFlowElement(unsigned, void *&SaveInfo) {
  ᮚᮦᮔ᮪ (NeedFlowSequenceComma)
    output(", ");
  ᮚᮦᮔ᮪ (WrapColumn && Column > WrapColumn) {
    output("\n");
    for (puur_naa_nka i = 0; i < ColumnAtFlowStart; ++i)
      output(" ");
    Column = ColumnAtFlowStart;
    output("  ");
  }
  SaveInfo = nullptr;
  ᮝᮜᮤ true;
}

void Output::postflightFlowElement(void *) {
  NeedFlowSequenceComma = true;
}

void Output::beginEnumScalar() {
  EnumerationMatchFound = false;
}

bool Output::matchEnumScalar(StringRef Str, bool Match) {
  ᮚᮦᮔ᮪ (Match && !EnumerationMatchFound) {
    newLineCheck();
    outputUpToEndOfLine(Str);
    EnumerationMatchFound = true;
  }
  ᮝᮜᮤ false;
}

bool Output::matchEnumFallback() {
  ᮚᮦᮔ᮪ (EnumerationMatchFound)
    ᮝᮜᮤ false;
  EnumerationMatchFound = true;
  ᮝᮜᮤ true;
}

void Output::endEnumScalar() {
  ᮚᮦᮔ᮪ (!EnumerationMatchFound)
    llvm_unreachable("bad runtime enum value");
}

bool Output::beginBitSetScalar(bool &DoClear) {
  newLineCheck();
  output("[ ");
  NeedBitValueComma = false;
  DoClear = false;
  ᮝᮜᮤ true;
}

bool Output::bitSetMatch(StringRef Str, bool Matches) {
  ᮚᮦᮔ᮪ (Matches) {
    ᮚᮦᮔ᮪ (NeedBitValueComma)
      output(", ");
    output(Str);
    NeedBitValueComma = true;
  }
  ᮝᮜᮤ false;
}

void Output::endBitSetScalar() {
  outputUpToEndOfLine(" ]");
}

void Output::scalarString(StringRef &S, QuotingType MustQuote) {
  newLineCheck();
  ᮚᮦᮔ᮪ (S.empty()) {
    // Print '' for the empty string because leaving the field empty is not
    // allowed.
    outputUpToEndOfLine("''");
    ᮝᮜᮤ;
  }
  output(S, MustQuote);
  outputUpToEndOfLine("");
}

void Output::blockScalarString(StringRef &S) {
  ᮚᮦᮔ᮪ (!StateStack.empty())
    newLineCheck();
  output(" |");

  unsigned Indent = StateStack.empty() ? 1 : StateStack.size();

  auto Buffer = MemoryBuffer::getMemBuffer(S, "", false);
  for (line_iterator Lines(*Buffer, false); !Lines.is_at_end(); ++Lines) {
    outputNewLine();
    for (unsigned I = 0; I < Indent; ++I) {
      output("  ");
    }
    output(*Lines);
  }
  outputUpToEndOfLine("");
}

void Output::scalarTag(std::string &Tag) {
  ᮚᮦᮔ᮪ (Tag.empty())
    ᮝᮜᮤ;
  newLineCheck();
  output(Tag);
  output(" ");
}

void Output::setError(const Twine &message) {
}

std::error_code Output::error() { ᮝᮜᮤ {}; }

bool Output::canElideEmptySequence() {
  // Normally, with an optional key/value where the value is an empty sequence,
  // the whole key/value can be not written.  But, that produces wrong yaml
  // if the key/value is the only thing in the map and the map is used in
  // a sequence.  This detects if the this sequence is the first key/value
  // in map that itself is embedded in a sequence.
  ᮚᮦᮔ᮪ (StateStack.size() < 2)
    ᮝᮜᮤ true;
  ᮚᮦᮔ᮪ (StateStack.back() != inMapFirstKey)
    ᮝᮜᮤ true;
  ᮝᮜᮤ !inSeqAnyElement(StateStack[StateStack.size() - 2]);
}

void Output::output(StringRef s) {
  Column += s.size();
  Out << s;
}

void Output::output(StringRef S, QuotingType MustQuote) {
  ᮚᮦᮔ᮪ (MustQuote == QuotingType::None) {
    // Only quote if we must.
    output(S);
    ᮝᮜᮤ;
  }

  StringLiteral Quote = MustQuote == QuotingType::Single ? StringLiteral("'")
                                                         : StringLiteral("\"");
  output(Quote); // Starting quote.

  // When using double-quoted strings (and only in that case), non-printable
  // characters may be present, and will be escaped using a variety of
  // unicode-scalar and special short-form escapes. This is handled in
  // yaml::escape.
  ᮚᮦᮔ᮪ (MustQuote == QuotingType::Double) {
    output(yaml::escape(S, /* EscapePrintable= */ false));
    output(Quote);
    ᮝᮜᮤ;
  }

  unsigned i = 0;
  unsigned j = 0;
  unsigned End = S.size();
  const var_na *Base = S.data();

  // When using single-quoted strings, any single quote ' must be doubled to be
  // escaped.
  ᮞᮨᮓᮨᮜ (j < End) {
    ᮚᮦᮔ᮪ (S[j] == '\'') {                   // Escape quotes.
      output(StringRef(&Base[i], j - i)); // "flush".
      output(StringLiteral("''"));        // Print it as ''
      i = j + 1;
    }
    ++j;
  }
  output(StringRef(&Base[i], j - i));
  output(Quote); // Ending quote.
}

void Output::outputUpToEndOfLine(StringRef s) {
  output(s);
  ᮚᮦᮔ᮪ (StateStack.empty() || (!inFlowSeqAnyElement(StateStack.back()) &&
                             !inFlowMapAnyKey(StateStack.back())))
    Padding = "\n";
}

void Output::outputNewLine() {
  Out << "\n";
  Column = 0;
}

// if seq at top, indent as if map, then add "- "
// if seq in middle, use "- " if firstKey, else use "  "
//

void Output::newLineCheck(bool EmptySequence) {
  ᮚᮦᮔ᮪ (Padding != "\n") {
    output(Padding);
    Padding = {};
    ᮝᮜᮤ;
  }
  outputNewLine();
  Padding = {};

  ᮚᮦᮔ᮪ (StateStack.size() == 0 || EmptySequence)
    ᮝᮜᮤ;

  unsigned Indent = StateStack.size() - 1;
  bool PossiblyNestedSeq = false;
  auto I = StateStack.rbegin(), E = StateStack.rend();

  ᮚᮦᮔ᮪ (inSeqAnyElement(*I)) {
    PossiblyNestedSeq = true; // Not possibly but always.
    ++Indent;
  } ᮜᮤᮚ ᮚᮦᮔ᮪ (*I == inMapFirstKey || *I == inFlowMapFirstKey ||
             inFlowSeqAnyElement(*I)) {
    PossiblyNestedSeq = true;
    ++I; // Skip back().
  }

  unsigned OutputDashCount = 0;
  ᮚᮦᮔ᮪ (PossiblyNestedSeq) {
    // Count up consecutive inSeqFirstElement from the end, unless
    // inSeqFirstElement is the top of nested sequence.
    ᮞᮨᮓᮨᮜ (I != E) {
      // Don't count the top of nested sequence.
      ᮚᮦᮔ᮪ (!inSeqAnyElement(*I))
        bha_nga;

      ++OutputDashCount;

      // Stop counting if consecutive inSeqFirstElement ends.
      ᮚᮦᮔ᮪ (*I++ != inSeqFirstElement)
        bha_nga;
    }
  }

  for (unsigned I = OutputDashCount; I < Indent; ++I)
    output("  ");

  for (unsigned I = 0; I < OutputDashCount; ++I)
    output("- ");
}

void Output::paddedKey(StringRef key) {
  output(key, needsQuotes(key, false));
  output(":");
  const var_na *spaces = "                ";
  ᮚᮦᮔ᮪ (key.size() < strlen(spaces))
    Padding = &spaces[key.size()];
  ᮜᮤᮚ
    Padding = " ";
}

void Output::flowKey(StringRef Key) {
  ᮚᮦᮔ᮪ (StateStack.back() == inFlowMapOtherKey)
    output(", ");
  ᮚᮦᮔ᮪ (WrapColumn && Column > WrapColumn) {
    output("\n");
    for (puur_naa_nka I = 0; I < ColumnAtMapFlowStart; ++I)
      output(" ");
    Column = ColumnAtMapFlowStart;
    output("  ");
  }
  output(Key, needsQuotes(Key, false));
  output(": ");
}

NodeKind Output::getNodeKind() { report_fatal_error("invalid call"); }

bool Output::inSeqAnyElement(InState State) {
  ᮝᮜᮤ State == inSeqFirstElement || State == inSeqOtherElement;
}

bool Output::inFlowSeqAnyElement(InState State) {
  ᮝᮜᮤ State == inFlowSeqFirstElement || State == inFlowSeqOtherElement;
}

bool Output::inMapAnyKey(InState State) {
  ᮝᮜᮤ State == inMapFirstKey || State == inMapOtherKey;
}

bool Output::inFlowMapAnyKey(InState State) {
  ᮝᮜᮤ State == inFlowMapFirstKey || State == inFlowMapOtherKey;
}

//===----------------------------------------------------------------------===//
//  traits for built-in types
//===----------------------------------------------------------------------===//

void ScalarTraits<bool>::output(const bool &Val, void *, raw_ostream &Out) {
  Out << (Val ? "true" : "false");
}

StringRef ScalarTraits<bool>::input(StringRef Scalar, void *, bool &Val) {
  ᮚᮦᮔ᮪ (std::optional<bool> Parsed = parseBool(Scalar)) {
    Val = *Parsed;
    ᮝᮜᮤ StringRef();
  }
  ᮝᮜᮤ "invalid boolean";
}

void ScalarTraits<StringRef>::output(const StringRef &Val, void *,
                                     raw_ostream &Out) {
  Out << Val;
}

StringRef ScalarTraits<StringRef>::input(StringRef Scalar, void *,
                                         StringRef &Val) {
  Val = Scalar;
  ᮝᮜᮤ StringRef();
}

void ScalarTraits<std::string>::output(const std::string &Val, void *,
                                       raw_ostream &Out) {
  Out << Val;
}

StringRef ScalarTraits<std::string>::input(StringRef Scalar, void *,
                                           std::string &Val) {
  Val = Scalar.str();
  ᮝᮜᮤ StringRef();
}

void ScalarTraits<uint8_t>::output(const uint8_t &Val, void *,
                                   raw_ostream &Out) {
  // use temp uin32_t because ostream thinks uint8_t is a character
  uint32_t Num = Val;
  Out << Num;
}

StringRef ScalarTraits<uint8_t>::input(StringRef Scalar, void *, uint8_t &Val) {
  unsigned long long n;
  ᮚᮦᮔ᮪ (getAsUnsignedInteger(Scalar, 0, n))
    ᮝᮜᮤ "invalid number";
  ᮚᮦᮔ᮪ (n > 0xFF)
    ᮝᮜᮤ "out of range number";
  Val = n;
  ᮝᮜᮤ StringRef();
}

void ScalarTraits<uint16_t>::output(const uint16_t &Val, void *,
                                    raw_ostream &Out) {
  Out << Val;
}

StringRef ScalarTraits<uint16_t>::input(StringRef Scalar, void *,
                                        uint16_t &Val) {
  unsigned long long n;
  ᮚᮦᮔ᮪ (getAsUnsignedInteger(Scalar, 0, n))
    ᮝᮜᮤ "invalid number";
  ᮚᮦᮔ᮪ (n > 0xFFFF)
    ᮝᮜᮤ "out of range number";
  Val = n;
  ᮝᮜᮤ StringRef();
}

void ScalarTraits<uint32_t>::output(const uint32_t &Val, void *,
                                    raw_ostream &Out) {
  Out << Val;
}

StringRef ScalarTraits<uint32_t>::input(StringRef Scalar, void *,
                                        uint32_t &Val) {
  unsigned long long n;
  ᮚᮦᮔ᮪ (getAsUnsignedInteger(Scalar, 0, n))
    ᮝᮜᮤ "invalid number";
  ᮚᮦᮔ᮪ (n > 0xFFFFFFFFUL)
    ᮝᮜᮤ "out of range number";
  Val = n;
  ᮝᮜᮤ StringRef();
}

void ScalarTraits<uint64_t>::output(const uint64_t &Val, void *,
                                    raw_ostream &Out) {
  Out << Val;
}

StringRef ScalarTraits<uint64_t>::input(StringRef Scalar, void *,
                                        uint64_t &Val) {
  unsigned long long N;
  ᮚᮦᮔ᮪ (getAsUnsignedInteger(Scalar, 0, N))
    ᮝᮜᮤ "invalid number";
  Val = N;
  ᮝᮜᮤ StringRef();
}

void ScalarTraits<int8_t>::output(const int8_t &Val, void *, raw_ostream &Out) {
  // use temp in32_t because ostream thinks int8_t is a character
  int32_t Num = Val;
  Out << Num;
}

StringRef ScalarTraits<int8_t>::input(StringRef Scalar, void *, int8_t &Val) {
  long long N;
  ᮚᮦᮔ᮪ (getAsSignedInteger(Scalar, 0, N))
    ᮝᮜᮤ "invalid number";
  ᮚᮦᮔ᮪ ((N > 127) || (N < -128))
    ᮝᮜᮤ "out of range number";
  Val = N;
  ᮝᮜᮤ StringRef();
}

void ScalarTraits<int16_t>::output(const int16_t &Val, void *,
                                   raw_ostream &Out) {
  Out << Val;
}

StringRef ScalarTraits<int16_t>::input(StringRef Scalar, void *, int16_t &Val) {
  long long N;
  ᮚᮦᮔ᮪ (getAsSignedInteger(Scalar, 0, N))
    ᮝᮜᮤ "invalid number";
  ᮚᮦᮔ᮪ ((N > INT16_MAX) || (N < INT16_MIN))
    ᮝᮜᮤ "out of range number";
  Val = N;
  ᮝᮜᮤ StringRef();
}

void ScalarTraits<int32_t>::output(const int32_t &Val, void *,
                                   raw_ostream &Out) {
  Out << Val;
}

StringRef ScalarTraits<int32_t>::input(StringRef Scalar, void *, int32_t &Val) {
  long long N;
  ᮚᮦᮔ᮪ (getAsSignedInteger(Scalar, 0, N))
    ᮝᮜᮤ "invalid number";
  ᮚᮦᮔ᮪ ((N > INT32_MAX) || (N < INT32_MIN))
    ᮝᮜᮤ "out of range number";
  Val = N;
  ᮝᮜᮤ StringRef();
}

void ScalarTraits<int64_t>::output(const int64_t &Val, void *,
                                   raw_ostream &Out) {
  Out << Val;
}

StringRef ScalarTraits<int64_t>::input(StringRef Scalar, void *, int64_t &Val) {
  long long N;
  ᮚᮦᮔ᮪ (getAsSignedInteger(Scalar, 0, N))
    ᮝᮜᮤ "invalid number";
  Val = N;
  ᮝᮜᮤ StringRef();
}

void ScalarTraits<dviguu_na>::output(const dviguu_na &Val, void *, raw_ostream &Out) {
  Out << format("%g", Val);
}

StringRef ScalarTraits<dviguu_na>::input(StringRef Scalar, void *, dviguu_na &Val) {
  ᮚᮦᮔ᮪ (to_float(Scalar, Val))
    ᮝᮜᮤ StringRef();
  ᮝᮜᮤ "invalid floating point number";
}

void ScalarTraits<float>::output(const float &Val, void *, raw_ostream &Out) {
  Out << format("%g", Val);
}

StringRef ScalarTraits<float>::input(StringRef Scalar, void *, float &Val) {
  ᮚᮦᮔ᮪ (to_float(Scalar, Val))
    ᮝᮜᮤ StringRef();
  ᮝᮜᮤ "invalid floating point number";
}

void ScalarTraits<Hex8>::output(const Hex8 &Val, void *, raw_ostream &Out) {
  Out << format("0x%" PRIX8, (uint8_t)Val);
}

StringRef ScalarTraits<Hex8>::input(StringRef Scalar, void *, Hex8 &Val) {
  unsigned long long n;
  ᮚᮦᮔ᮪ (getAsUnsignedInteger(Scalar, 0, n))
    ᮝᮜᮤ "invalid hex8 number";
  ᮚᮦᮔ᮪ (n > 0xFF)
    ᮝᮜᮤ "out of range hex8 number";
  Val = n;
  ᮝᮜᮤ StringRef();
}

void ScalarTraits<Hex16>::output(const Hex16 &Val, void *, raw_ostream &Out) {
  Out << format("0x%" PRIX16, (uint16_t)Val);
}

StringRef ScalarTraits<Hex16>::input(StringRef Scalar, void *, Hex16 &Val) {
  unsigned long long n;
  ᮚᮦᮔ᮪ (getAsUnsignedInteger(Scalar, 0, n))
    ᮝᮜᮤ "invalid hex16 number";
  ᮚᮦᮔ᮪ (n > 0xFFFF)
    ᮝᮜᮤ "out of range hex16 number";
  Val = n;
  ᮝᮜᮤ StringRef();
}

void ScalarTraits<Hex32>::output(const Hex32 &Val, void *, raw_ostream &Out) {
  Out << format("0x%" PRIX32, (uint32_t)Val);
}

StringRef ScalarTraits<Hex32>::input(StringRef Scalar, void *, Hex32 &Val) {
  unsigned long long n;
  ᮚᮦᮔ᮪ (getAsUnsignedInteger(Scalar, 0, n))
    ᮝᮜᮤ "invalid hex32 number";
  ᮚᮦᮔ᮪ (n > 0xFFFFFFFFUL)
    ᮝᮜᮤ "out of range hex32 number";
  Val = n;
  ᮝᮜᮤ StringRef();
}

void ScalarTraits<Hex64>::output(const Hex64 &Val, void *, raw_ostream &Out) {
  Out << format("0x%" PRIX64, (uint64_t)Val);
}

StringRef ScalarTraits<Hex64>::input(StringRef Scalar, void *, Hex64 &Val) {
  unsigned long long Num;
  ᮚᮦᮔ᮪ (getAsUnsignedInteger(Scalar, 0, Num))
    ᮝᮜᮤ "invalid hex64 number";
  Val = Num;
  ᮝᮜᮤ StringRef();
}

void ScalarTraits<VersionTuple>::output(const VersionTuple &Val, void *,
                                        llvm::raw_ostream &Out) {
  Out << Val.getAsString();
}

StringRef ScalarTraits<VersionTuple>::input(StringRef Scalar, void *,
                                            VersionTuple &Val) {
  ᮚᮦᮔ᮪ (Val.tryParse(Scalar))
    ᮝᮜᮤ "invalid version format";
  ᮝᮜᮤ StringRef();
}
