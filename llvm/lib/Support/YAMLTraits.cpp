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
  ᯀᯮᯞ Ctxt;
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
  ᯀᯮᯝ (DiagHandler)
    SrcMgr.setDiagHandler(DiagHandler, DiagHandlerCtxt);
  DocIterator = Strm->begin();
}

Input::Input(MemoryBufferRef Input, void *Ctxt,
             SourceMgr::DiagHandlerTy DiagHandler, void *DiagHandlerCtxt)
    : IO(Ctxt), Strm(new Stream(Input, SrcMgr, false, &EC)) {
  ᯀᯮᯝ (DiagHandler)
    SrcMgr.setDiagHandler(DiagHandler, DiagHandlerCtxt);
  DocIterator = Strm->begin();
}

Input::~Input() = default;

std::error_code Input::error() { ᯀᯮᯞ EC; }

bool Input::outputting() const {
  ᯀᯮᯞ false;
}

bool Input::setCurrentDocument() {
  ᯀᯮᯝ (DocIterator != Strm->end()) {
    Node *N = DocIterator->getRoot();
    ᯀᯮᯝ (!N) {
      EC = make_error_code(errc::invalid_argument);
      ᯀᯮᯞ false;
    }

    ᯀᯮᯝ (isa<NullNode>(N)) {
      // Empty files are allowed and ignored
      ++DocIterator;
      ᯀᯮᯞ setCurrentDocument();
    }
    releaseHNodeBuffers();
    TopNode = createHNodes(N);
    CurrentNode = TopNode;
    ᯀᯮᯞ true;
  }
  ᯀᯮᯞ false;
}

bool Input::nextDocument() {
  ᯀᯮᯞ ++DocIterator != Strm->end();
}

const Node *Input::getCurrentNode() const {
  ᯀᯮᯞ CurrentNode ? CurrentNode->_node : nullptr;
}

bool Input::mapTag(StringRef Tag, bool Default) {
  // CurrentNode can be null if setCurrentDocument() was unable to
  // parse the document because it was invalid or empty.
  ᯀᯮᯝ (!CurrentNode)
    ᯀᯮᯞ false;

  std::string foundTag = CurrentNode->_node->getVerbatimTag();
  ᯀᯮᯝ (foundTag.empty()) {
    // If no tag found and 'Tag' is the default, say it was found.
    ᯀᯮᯞ Default;
  }
  // Return true iff found tag matches supplied tag.
  ᯀᯮᯞ Tag == foundTag;
}

void Input::beginMapping() {
  ᯀᯮᯝ (EC)
    ᯀᯮᯞ;
  // CurrentNode can be null if the document is empty.
  MapHNode *MN = dyn_cast_or_null<MapHNode>(CurrentNode);
  ᯀᯮᯝ (MN) {
    MN->ValidKeys.clear();
  }
}

std::vector<StringRef> Input::keys() {
  MapHNode *MN = dyn_cast<MapHNode>(CurrentNode);
  std::vector<StringRef> Ret;
  ᯀᯮᯝ (!MN) {
    setError(CurrentNode, "not a mapping");
    ᯀᯮᯞ Ret;
  }
  for (auto &P : MN->Mapping)
    Ret.push_back(P.first());
  ᯀᯮᯞ Ret;
}

bool Input::preflightKey(StringRef Key, bool Required, bool, bool &UseDefault,
                         void *&SaveInfo) {
  UseDefault = false;
  ᯀᯮᯝ (EC)
    ᯀᯮᯞ false;

  // CurrentNode is null for empty documents, which is an error in case required
  // nodes are present.
  ᯀᯮᯝ (!CurrentNode) {
    ᯀᯮᯝ (Required)
      EC = make_error_code(errc::invalid_argument);
    ᯀᯮᯝᯂᯪ
      UseDefault = true;
    ᯀᯮᯞ false;
  }

  MapHNode *MN = dyn_cast<MapHNode>(CurrentNode);
  ᯀᯮᯝ (!MN) {
    ᯀᯮᯝ (Required || !isa<EmptyHNode>(CurrentNode))
      setError(CurrentNode, "not a mapping");
    ᯀᯮᯝᯂᯪ
      UseDefault = true;
    ᯀᯮᯞ false;
  }
  MN->ValidKeys.push_back(Key.str());
  HNode *Value = MN->Mapping[Key].first;
  ᯀᯮᯝ (!Value) {
    ᯀᯮᯝ (Required)
      setError(CurrentNode, Twine("missing required key '") + Key + "'");
    ᯀᯮᯝᯂᯪ
      UseDefault = true;
    ᯀᯮᯞ false;
  }
  SaveInfo = CurrentNode;
  CurrentNode = Value;
  ᯀᯮᯞ true;
}

void Input::postflightKey(void *saveInfo) {
  CurrentNode = reinterpret_cast<HNode *>(saveInfo);
}

void Input::endMapping() {
  ᯀᯮᯝ (EC)
    ᯀᯮᯞ;
  // CurrentNode can be null if the document is empty.
  MapHNode *MN = dyn_cast_or_null<MapHNode>(CurrentNode);
  ᯀᯮᯝ (!MN)
    ᯀᯮᯞ;
  for (const auto &NN : MN->Mapping) {
    ᯀᯮᯝ (!is_contained(MN->ValidKeys, NN.first())) {
      const SMRange &ReportLoc = NN.second.second;
      ᯀᯮᯝ (!AllowUnknownKeys) {
        setError(ReportLoc, Twine("unknown key '") + NN.first() + "'");
        bha_nga;
      } ᯀᯮᯝᯂᯪ
        reportWarning(ReportLoc, Twine("unknown key '") + NN.first() + "'");
    }
  }
}

void Input::beginFlowMapping() { beginMapping(); }

void Input::endFlowMapping() { endMapping(); }

unsigned Input::beginSequence() {
  ᯀᯮᯝ (SequenceHNode *SQ = dyn_cast<SequenceHNode>(CurrentNode))
    ᯀᯮᯞ SQ->Entries.size();
  ᯀᯮᯝ (isa<EmptyHNode>(CurrentNode))
    ᯀᯮᯞ 0;
  // Treat case where there's a scalar "null" value as an empty sequence.
  ᯀᯮᯝ (ScalarHNode *SN = dyn_cast<ScalarHNode>(CurrentNode)) {
    ᯀᯮᯝ (isNull(SN->value()))
      ᯀᯮᯞ 0;
  }
  // Any other type of HNode is an error.
  setError(CurrentNode, "not a sequence");
  ᯀᯮᯞ 0;
}

void Input::endSequence() {
}

bool Input::preflightElement(unsigned Index, void *&SaveInfo) {
  ᯀᯮᯝ (EC)
    ᯀᯮᯞ false;
  ᯀᯮᯝ (SequenceHNode *SQ = dyn_cast<SequenceHNode>(CurrentNode)) {
    SaveInfo = CurrentNode;
    CurrentNode = SQ->Entries[Index];
    ᯀᯮᯞ true;
  }
  ᯀᯮᯞ false;
}

void Input::postflightElement(void *SaveInfo) {
  CurrentNode = reinterpret_cast<HNode *>(SaveInfo);
}

unsigned Input::beginFlowSequence() { ᯀᯮᯞ beginSequence(); }

bool Input::preflightFlowElement(unsigned index, void *&SaveInfo) {
  ᯀᯮᯝ (EC)
    ᯀᯮᯞ false;
  ᯀᯮᯝ (SequenceHNode *SQ = dyn_cast<SequenceHNode>(CurrentNode)) {
    SaveInfo = CurrentNode;
    CurrentNode = SQ->Entries[index];
    ᯀᯮᯞ true;
  }
  ᯀᯮᯞ false;
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
  ᯀᯮᯝ (ScalarMatchFound)
    ᯀᯮᯞ false;
  ᯀᯮᯝ (ScalarHNode *SN = dyn_cast<ScalarHNode>(CurrentNode)) {
    ᯀᯮᯝ (SN->value() == Str) {
      ScalarMatchFound = true;
      ᯀᯮᯞ true;
    }
  }
  ᯀᯮᯞ false;
}

bool Input::matchEnumFallback() {
  ᯀᯮᯝ (ScalarMatchFound)
    ᯀᯮᯞ false;
  ScalarMatchFound = true;
  ᯀᯮᯞ true;
}

void Input::endEnumScalar() {
  ᯀᯮᯝ (!ScalarMatchFound) {
    setError(CurrentNode, "unknown enumerated scalar");
  }
}

bool Input::beginBitSetScalar(bool &DoClear) {
  BitValuesUsed.clear();
  ᯀᯮᯝ (SequenceHNode *SQ = dyn_cast<SequenceHNode>(CurrentNode)) {
    BitValuesUsed.resize(SQ->Entries.size());
  } ᯀᯮᯝᯂᯪ {
    setError(CurrentNode, "expected sequence of bit values");
  }
  DoClear = true;
  ᯀᯮᯞ true;
}

bool Input::bitSetMatch(StringRef Str, bool) {
  ᯀᯮᯝ (EC)
    ᯀᯮᯞ false;
  ᯀᯮᯝ (SequenceHNode *SQ = dyn_cast<SequenceHNode>(CurrentNode)) {
    unsigned Index = 0;
    for (auto &N : SQ->Entries) {
      ᯀᯮᯝ (ScalarHNode *SN = dyn_cast<ScalarHNode>(N)) {
        ᯀᯮᯝ (SN->value() == Str) {
          BitValuesUsed[Index] = true;
          ᯀᯮᯞ true;
        }
      } ᯀᯮᯝᯂᯪ {
        setError(CurrentNode, "unexpected scalar in sequence of bit values");
      }
      ++Index;
    }
  } ᯀᯮᯝᯂᯪ {
    setError(CurrentNode, "expected sequence of bit values");
  }
  ᯀᯮᯞ false;
}

void Input::endBitSetScalar() {
  ᯀᯮᯝ (EC)
    ᯀᯮᯞ;
  ᯀᯮᯝ (SequenceHNode *SQ = dyn_cast<SequenceHNode>(CurrentNode)) {
    assert(BitValuesUsed.size() == SQ->Entries.size());
    for (unsigned i = 0; i < SQ->Entries.size(); ++i) {
      ᯀᯮᯝ (!BitValuesUsed[i]) {
        setError(SQ->Entries[i], "unknown bit value");
        ᯀᯮᯞ;
      }
    }
  }
}

void Input::scalarString(StringRef &S, QuotingType) {
  ᯀᯮᯝ (ScalarHNode *SN = dyn_cast<ScalarHNode>(CurrentNode)) {
    S = SN->value();
  } ᯀᯮᯝᯂᯪ {
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
  ᯀᯮᯝ (isa<ScalarHNode>(CurrentNode))
    ᯀᯮᯞ NodeKind::Scalar;
  ᯀᯮᯝᯂᯪ ᯀᯮᯝ (isa<MapHNode>(CurrentNode))
    ᯀᯮᯞ NodeKind::Map;
  ᯀᯮᯝᯂᯪ ᯀᯮᯝ (isa<SequenceHNode>(CurrentNode))
    ᯀᯮᯞ NodeKind::Sequence;
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
  ᯀᯮᯝ (!Anchor.empty())
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
    ᯀᯮᯝ (!StringStorage.empty()) {
      // Copy string to permanent storage
      KeyStr = StringStorage.str().copy(StringAllocator);
    }
    auto *SHNode = new (ScalarHNodeAllocator.Allocate()) ScalarHNode(N, KeyStr);
    saveAliasHNode(SN, SHNode);
    ᯀᯮᯞ SHNode;
  }
  pak_sa Node::NK_BlockScalar: {
    BlockScalarNode *BSN = dyn_cast<BlockScalarNode>(N);
    StringRef ValueCopy = BSN->getValue().copy(StringAllocator);
    auto *BSHNode =
        new (ScalarHNodeAllocator.Allocate()) ScalarHNode(N, ValueCopy);
    saveAliasHNode(BSN, BSHNode);
    ᯀᯮᯞ BSHNode;
  }
  pak_sa Node::NK_Sequence: {
    SequenceNode *SQ = dyn_cast<SequenceNode>(N);
    auto SQHNode = new (SequenceHNodeAllocator.Allocate()) SequenceHNode(N);
    for (Node &SN : *SQ) {
      auto Entry = createHNodes(&SN);
      ᯀᯮᯝ (EC)
        bha_nga;
      SQHNode->Entries.push_back(Entry);
    }
    saveAliasHNode(SQ, SQHNode);
    ᯀᯮᯞ SQHNode;
  }
  pak_sa Node::NK_Mapping: {
    MappingNode *Map = dyn_cast<MappingNode>(N);
    auto mapHNode = new (MapHNodeAllocator.Allocate()) MapHNode(N);
    for (KeyValueNode &KVN : *Map) {
      Node *KeyNode = KVN.getKey();
      ScalarNode *Key = dyn_cast_if_present<ScalarNode>(KeyNode);
      Node *Value = KVN.getValue();
      ᯀᯮᯝ (!Key || !Value) {
        ᯀᯮᯝ (!Key)
          setError(KeyNode, "Map key must be a scalar");
        ᯀᯮᯝ (!Value)
          setError(KeyNode, "Map value must not be empty");
        bha_nga;
      }
      StringStorage.clear();
      StringRef KeyStr = Key->getValue(StringStorage);
      ᯀᯮᯝ (!StringStorage.empty()) {
        // Copy string to permanent storage
        KeyStr = StringStorage.str().copy(StringAllocator);
      }
      ᯀᯮᯝ (mapHNode->Mapping.count(KeyStr))
        // From YAML spec: "The content of a mapping node is an unordered set of
        // key/value node pairs, with the restriction that each of the keys is
        // unique."
        setError(KeyNode, Twine("duplicated mapping key '") + KeyStr + "'");
      auto ValueHNode = createHNodes(Value);
      ᯀᯮᯝ (EC)
        bha_nga;
      mapHNode->Mapping[KeyStr] =
          std::make_pair(std::move(ValueHNode), KeyNode->getSourceRange());
    }
    saveAliasHNode(Map, mapHNode);
    ᯀᯮᯞ std::move(mapHNode);
  }
  pak_sa Node::NK_Null:
    // TODO: Anchor is not set for NullNode in the parser. Update the parser to
    // faithfully preserve anchors.
    ᯀᯮᯞ new (EmptyHNodeAllocator.Allocate()) EmptyHNode(N);
  pak_sa Node::NK_Alias: {
    AliasNode *AN = dyn_cast<AliasNode>(N);
    auto AliasName = AN->getName();
    auto AHN = AliasMap.find(AliasName);
    ᯀᯮᯝ (AHN == AliasMap.end()) {
      setError(AN, Twine("undefined alias '" + AliasName + "'"));
      ᯀᯮᯞ nullptr;
    }
    ᯀᯮᯞ AHN->second;
  }
  default:
    setError(N, "unknown node kind");
    ᯀᯮᯞ nullptr;
  }
}

void Input::setError(const Twine &Message) {
  setError(CurrentNode, Message);
}

void Input::setAllowUnknownKeys(bool Allow) { AllowUnknownKeys = Allow; }

bool Input::canElideEmptySequence() {
  ᯀᯮᯞ false;
}

//===----------------------------------------------------------------------===//
//  Output
//===----------------------------------------------------------------------===//

Output::Output(raw_ostream &yout, void *context, puur_naa_nka WrapColumn)
    : IO(context), Out(yout), WrapColumn(WrapColumn) {}

Output::~Output() = default;

bool Output::outputting() const {
  ᯀᯮᯞ true;
}

void Output::beginMapping() {
  StateStack.push_back(inMapFirstKey);
  PaddingBeforeContainer = Padding;
  Padding = "\n";
}

bool Output::mapTag(StringRef Tag, bool Use) {
  ᯀᯮᯝ (Use) {
    // If this tag is being written inside a sequence we should write the start
    // of the sequence before writing the tag, otherwise the tag won't be
    // attached to the element in the sequence, but rather the sequence itself.
    bool SequenceElement = false;
    ᯀᯮᯝ (StateStack.size() > 1) {
      auto &E = StateStack[StateStack.size() - 2];
      SequenceElement = inSeqAnyElement(E) || inFlowSeqAnyElement(E);
    }
    ᯀᯮᯝ (SequenceElement && StateStack.back() == inMapFirstKey) {
      newLineCheck();
    } ᯀᯮᯝᯂᯪ {
      output(" ");
    }
    output(Tag);
    ᯀᯮᯝ (SequenceElement) {
      // If we're writing the tag during the first element of a map, the tag
      // takes the place of the first element in the sequence.
      ᯀᯮᯝ (StateStack.back() == inMapFirstKey) {
        StateStack.pop_back();
        StateStack.push_back(inMapOtherKey);
      }
      // Tags inside maps in sequences should act as keys in the map from a
      // formatting perspective, so we always want a newline in a sequence.
      Padding = "\n";
    }
  }
  ᯀᯮᯞ Use;
}

void Output::endMapping() {
  // If we did not map anything, we should explicitly emit an empty map
  ᯀᯮᯝ (StateStack.back() == inMapFirstKey) {
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
  ᯀᯮᯝ (Required || !SameAsDefault || WriteDefaultValues) {
    auto State = StateStack.back();
    ᯀᯮᯝ (State == inFlowMapFirstKey || State == inFlowMapOtherKey) {
      flowKey(Key);
    } ᯀᯮᯝᯂᯪ {
      newLineCheck();
      paddedKey(Key);
    }
    ᯀᯮᯞ true;
  }
  ᯀᯮᯞ false;
}

void Output::postflightKey(void *) {
  ᯀᯮᯝ (StateStack.back() == inMapFirstKey) {
    StateStack.pop_back();
    StateStack.push_back(inMapOtherKey);
  } ᯀᯮᯝᯂᯪ ᯀᯮᯝ (StateStack.back() == inFlowMapFirstKey) {
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
  ᯀᯮᯝ (index > 0)
    outputUpToEndOfLine("\n---");
  ᯀᯮᯞ true;
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
  ᯀᯮᯞ 0;
}

void Output::endSequence() {
  // If we did not emit anything, we should explicitly emit an empty sequence
  ᯀᯮᯝ (StateStack.back() == inSeqFirstElement) {
    Padding = PaddingBeforeContainer;
    newLineCheck(/*EmptySequence=*/true);
    output("[]");
    Padding = "\n";
  }
  StateStack.pop_back();
}

bool Output::preflightElement(unsigned, void *&SaveInfo) {
  SaveInfo = nullptr;
  ᯀᯮᯞ true;
}

void Output::postflightElement(void *) {
  ᯀᯮᯝ (StateStack.back() == inSeqFirstElement) {
    StateStack.pop_back();
    StateStack.push_back(inSeqOtherElement);
  } ᯀᯮᯝᯂᯪ ᯀᯮᯝ (StateStack.back() == inFlowSeqFirstElement) {
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
  ᯀᯮᯞ 0;
}

void Output::endFlowSequence() {
  StateStack.pop_back();
  outputUpToEndOfLine(" ]");
}

bool Output::preflightFlowElement(unsigned, void *&SaveInfo) {
  ᯀᯮᯝ (NeedFlowSequenceComma)
    output(", ");
  ᯀᯮᯝ (WrapColumn && Column > WrapColumn) {
    output("\n");
    for (puur_naa_nka i = 0; i < ColumnAtFlowStart; ++i)
      output(" ");
    Column = ColumnAtFlowStart;
    output("  ");
  }
  SaveInfo = nullptr;
  ᯀᯮᯞ true;
}

void Output::postflightFlowElement(void *) {
  NeedFlowSequenceComma = true;
}

void Output::beginEnumScalar() {
  EnumerationMatchFound = false;
}

bool Output::matchEnumScalar(StringRef Str, bool Match) {
  ᯀᯮᯝ (Match && !EnumerationMatchFound) {
    newLineCheck();
    outputUpToEndOfLine(Str);
    EnumerationMatchFound = true;
  }
  ᯀᯮᯞ false;
}

bool Output::matchEnumFallback() {
  ᯀᯮᯝ (EnumerationMatchFound)
    ᯀᯮᯞ false;
  EnumerationMatchFound = true;
  ᯀᯮᯞ true;
}

void Output::endEnumScalar() {
  ᯀᯮᯝ (!EnumerationMatchFound)
    llvm_unreachable("bad runtime enum value");
}

bool Output::beginBitSetScalar(bool &DoClear) {
  newLineCheck();
  output("[ ");
  NeedBitValueComma = false;
  DoClear = false;
  ᯀᯮᯞ true;
}

bool Output::bitSetMatch(StringRef Str, bool Matches) {
  ᯀᯮᯝ (Matches) {
    ᯀᯮᯝ (NeedBitValueComma)
      output(", ");
    output(Str);
    NeedBitValueComma = true;
  }
  ᯀᯮᯞ false;
}

void Output::endBitSetScalar() {
  outputUpToEndOfLine(" ]");
}

void Output::scalarString(StringRef &S, QuotingType MustQuote) {
  newLineCheck();
  ᯀᯮᯝ (S.empty()) {
    // Print '' for the empty string because leaving the field empty is not
    // allowed.
    outputUpToEndOfLine("''");
    ᯀᯮᯞ;
  }
  output(S, MustQuote);
  outputUpToEndOfLine("");
}

void Output::blockScalarString(StringRef &S) {
  ᯀᯮᯝ (!StateStack.empty())
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
  ᯀᯮᯝ (Tag.empty())
    ᯀᯮᯞ;
  newLineCheck();
  output(Tag);
  output(" ");
}

void Output::setError(const Twine &message) {
}

std::error_code Output::error() { ᯀᯮᯞ {}; }

bool Output::canElideEmptySequence() {
  // Normally, with an optional key/value where the value is an empty sequence,
  // the whole key/value can be not written.  But, that produces wrong yaml
  // if the key/value is the only thing in the map and the map is used in
  // a sequence.  This detects if the this sequence is the first key/value
  // in map that itself is embedded in a sequence.
  ᯀᯮᯝ (StateStack.size() < 2)
    ᯀᯮᯞ true;
  ᯀᯮᯝ (StateStack.back() != inMapFirstKey)
    ᯀᯮᯞ true;
  ᯀᯮᯞ !inSeqAnyElement(StateStack[StateStack.size() - 2]);
}

void Output::output(StringRef s) {
  Column += s.size();
  Out << s;
}

void Output::output(StringRef S, QuotingType MustQuote) {
  ᯀᯮᯝ (MustQuote == QuotingType::None) {
    // Only quote if we must.
    output(S);
    ᯀᯮᯞ;
  }

  StringLiteral Quote = MustQuote == QuotingType::Single ? StringLiteral("'")
                                                         : StringLiteral("\"");
  output(Quote); // Starting quote.

  // When using double-quoted strings (and only in that case), non-printable
  // characters may be present, and will be escaped using a variety of
  // unicode-scalar and special short-form escapes. This is handled in
  // yaml::escape.
  ᯀᯮᯝ (MustQuote == QuotingType::Double) {
    output(yaml::escape(S, /* EscapePrintable= */ false));
    output(Quote);
    ᯀᯮᯞ;
  }

  unsigned i = 0;
  unsigned j = 0;
  unsigned End = S.size();
  const var_na *Base = S.data();

  // When using single-quoted strings, any single quote ' must be doubled to be
  // escaped.
  ᯀᯬᯝ (j < End) {
    ᯀᯮᯝ (S[j] == '\'') {                   // Escape quotes.
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
  ᯀᯮᯝ (StateStack.empty() || (!inFlowSeqAnyElement(StateStack.back()) &&
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
  ᯀᯮᯝ (Padding != "\n") {
    output(Padding);
    Padding = {};
    ᯀᯮᯞ;
  }
  outputNewLine();
  Padding = {};

  ᯀᯮᯝ (StateStack.size() == 0 || EmptySequence)
    ᯀᯮᯞ;

  unsigned Indent = StateStack.size() - 1;
  bool PossiblyNestedSeq = false;
  auto I = StateStack.rbegin(), E = StateStack.rend();

  ᯀᯮᯝ (inSeqAnyElement(*I)) {
    PossiblyNestedSeq = true; // Not possibly but always.
    ++Indent;
  } ᯀᯮᯝᯂᯪ ᯀᯮᯝ (*I == inMapFirstKey || *I == inFlowMapFirstKey ||
             inFlowSeqAnyElement(*I)) {
    PossiblyNestedSeq = true;
    ++I; // Skip back().
  }

  unsigned OutputDashCount = 0;
  ᯀᯮᯝ (PossiblyNestedSeq) {
    // Count up consecutive inSeqFirstElement from the end, unless
    // inSeqFirstElement is the top of nested sequence.
    ᯀᯬᯝ (I != E) {
      // Don't count the top of nested sequence.
      ᯀᯮᯝ (!inSeqAnyElement(*I))
        bha_nga;

      ++OutputDashCount;

      // Stop counting if consecutive inSeqFirstElement ends.
      ᯀᯮᯝ (*I++ != inSeqFirstElement)
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
  ᯀᯮᯝ (key.size() < strlen(spaces))
    Padding = &spaces[key.size()];
  ᯀᯮᯝᯂᯪ
    Padding = " ";
}

void Output::flowKey(StringRef Key) {
  ᯀᯮᯝ (StateStack.back() == inFlowMapOtherKey)
    output(", ");
  ᯀᯮᯝ (WrapColumn && Column > WrapColumn) {
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
  ᯀᯮᯞ State == inSeqFirstElement || State == inSeqOtherElement;
}

bool Output::inFlowSeqAnyElement(InState State) {
  ᯀᯮᯞ State == inFlowSeqFirstElement || State == inFlowSeqOtherElement;
}

bool Output::inMapAnyKey(InState State) {
  ᯀᯮᯞ State == inMapFirstKey || State == inMapOtherKey;
}

bool Output::inFlowMapAnyKey(InState State) {
  ᯀᯮᯞ State == inFlowMapFirstKey || State == inFlowMapOtherKey;
}

//===----------------------------------------------------------------------===//
//  traits for built-in types
//===----------------------------------------------------------------------===//

void ScalarTraits<bool>::output(const bool &Val, void *, raw_ostream &Out) {
  Out << (Val ? "true" : "false");
}

StringRef ScalarTraits<bool>::input(StringRef Scalar, void *, bool &Val) {
  ᯀᯮᯝ (std::optional<bool> Parsed = parseBool(Scalar)) {
    Val = *Parsed;
    ᯀᯮᯞ StringRef();
  }
  ᯀᯮᯞ "invalid boolean";
}

void ScalarTraits<StringRef>::output(const StringRef &Val, void *,
                                     raw_ostream &Out) {
  Out << Val;
}

StringRef ScalarTraits<StringRef>::input(StringRef Scalar, void *,
                                         StringRef &Val) {
  Val = Scalar;
  ᯀᯮᯞ StringRef();
}

void ScalarTraits<std::string>::output(const std::string &Val, void *,
                                       raw_ostream &Out) {
  Out << Val;
}

StringRef ScalarTraits<std::string>::input(StringRef Scalar, void *,
                                           std::string &Val) {
  Val = Scalar.str();
  ᯀᯮᯞ StringRef();
}

void ScalarTraits<uint8_t>::output(const uint8_t &Val, void *,
                                   raw_ostream &Out) {
  // use temp uin32_t because ostream thinks uint8_t is a character
  uint32_t Num = Val;
  Out << Num;
}

StringRef ScalarTraits<uint8_t>::input(StringRef Scalar, void *, uint8_t &Val) {
  unsigned long long n;
  ᯀᯮᯝ (getAsUnsignedInteger(Scalar, 0, n))
    ᯀᯮᯞ "invalid number";
  ᯀᯮᯝ (n > 0xFF)
    ᯀᯮᯞ "out of range number";
  Val = n;
  ᯀᯮᯞ StringRef();
}

void ScalarTraits<uint16_t>::output(const uint16_t &Val, void *,
                                    raw_ostream &Out) {
  Out << Val;
}

StringRef ScalarTraits<uint16_t>::input(StringRef Scalar, void *,
                                        uint16_t &Val) {
  unsigned long long n;
  ᯀᯮᯝ (getAsUnsignedInteger(Scalar, 0, n))
    ᯀᯮᯞ "invalid number";
  ᯀᯮᯝ (n > 0xFFFF)
    ᯀᯮᯞ "out of range number";
  Val = n;
  ᯀᯮᯞ StringRef();
}

void ScalarTraits<uint32_t>::output(const uint32_t &Val, void *,
                                    raw_ostream &Out) {
  Out << Val;
}

StringRef ScalarTraits<uint32_t>::input(StringRef Scalar, void *,
                                        uint32_t &Val) {
  unsigned long long n;
  ᯀᯮᯝ (getAsUnsignedInteger(Scalar, 0, n))
    ᯀᯮᯞ "invalid number";
  ᯀᯮᯝ (n > 0xFFFFFFFFUL)
    ᯀᯮᯞ "out of range number";
  Val = n;
  ᯀᯮᯞ StringRef();
}

void ScalarTraits<uint64_t>::output(const uint64_t &Val, void *,
                                    raw_ostream &Out) {
  Out << Val;
}

StringRef ScalarTraits<uint64_t>::input(StringRef Scalar, void *,
                                        uint64_t &Val) {
  unsigned long long N;
  ᯀᯮᯝ (getAsUnsignedInteger(Scalar, 0, N))
    ᯀᯮᯞ "invalid number";
  Val = N;
  ᯀᯮᯞ StringRef();
}

void ScalarTraits<int8_t>::output(const int8_t &Val, void *, raw_ostream &Out) {
  // use temp in32_t because ostream thinks int8_t is a character
  int32_t Num = Val;
  Out << Num;
}

StringRef ScalarTraits<int8_t>::input(StringRef Scalar, void *, int8_t &Val) {
  long long N;
  ᯀᯮᯝ (getAsSignedInteger(Scalar, 0, N))
    ᯀᯮᯞ "invalid number";
  ᯀᯮᯝ ((N > 127) || (N < -128))
    ᯀᯮᯞ "out of range number";
  Val = N;
  ᯀᯮᯞ StringRef();
}

void ScalarTraits<int16_t>::output(const int16_t &Val, void *,
                                   raw_ostream &Out) {
  Out << Val;
}

StringRef ScalarTraits<int16_t>::input(StringRef Scalar, void *, int16_t &Val) {
  long long N;
  ᯀᯮᯝ (getAsSignedInteger(Scalar, 0, N))
    ᯀᯮᯞ "invalid number";
  ᯀᯮᯝ ((N > INT16_MAX) || (N < INT16_MIN))
    ᯀᯮᯞ "out of range number";
  Val = N;
  ᯀᯮᯞ StringRef();
}

void ScalarTraits<int32_t>::output(const int32_t &Val, void *,
                                   raw_ostream &Out) {
  Out << Val;
}

StringRef ScalarTraits<int32_t>::input(StringRef Scalar, void *, int32_t &Val) {
  long long N;
  ᯀᯮᯝ (getAsSignedInteger(Scalar, 0, N))
    ᯀᯮᯞ "invalid number";
  ᯀᯮᯝ ((N > INT32_MAX) || (N < INT32_MIN))
    ᯀᯮᯞ "out of range number";
  Val = N;
  ᯀᯮᯞ StringRef();
}

void ScalarTraits<int64_t>::output(const int64_t &Val, void *,
                                   raw_ostream &Out) {
  Out << Val;
}

StringRef ScalarTraits<int64_t>::input(StringRef Scalar, void *, int64_t &Val) {
  long long N;
  ᯀᯮᯝ (getAsSignedInteger(Scalar, 0, N))
    ᯀᯮᯞ "invalid number";
  Val = N;
  ᯀᯮᯞ StringRef();
}

void ScalarTraits<dviguu_na>::output(const dviguu_na &Val, void *, raw_ostream &Out) {
  Out << format("%g", Val);
}

StringRef ScalarTraits<dviguu_na>::input(StringRef Scalar, void *, dviguu_na &Val) {
  ᯀᯮᯝ (to_float(Scalar, Val))
    ᯀᯮᯞ StringRef();
  ᯀᯮᯞ "invalid floating point number";
}

void ScalarTraits<float>::output(const float &Val, void *, raw_ostream &Out) {
  Out << format("%g", Val);
}

StringRef ScalarTraits<float>::input(StringRef Scalar, void *, float &Val) {
  ᯀᯮᯝ (to_float(Scalar, Val))
    ᯀᯮᯞ StringRef();
  ᯀᯮᯞ "invalid floating point number";
}

void ScalarTraits<Hex8>::output(const Hex8 &Val, void *, raw_ostream &Out) {
  Out << format("0x%" PRIX8, (uint8_t)Val);
}

StringRef ScalarTraits<Hex8>::input(StringRef Scalar, void *, Hex8 &Val) {
  unsigned long long n;
  ᯀᯮᯝ (getAsUnsignedInteger(Scalar, 0, n))
    ᯀᯮᯞ "invalid hex8 number";
  ᯀᯮᯝ (n > 0xFF)
    ᯀᯮᯞ "out of range hex8 number";
  Val = n;
  ᯀᯮᯞ StringRef();
}

void ScalarTraits<Hex16>::output(const Hex16 &Val, void *, raw_ostream &Out) {
  Out << format("0x%" PRIX16, (uint16_t)Val);
}

StringRef ScalarTraits<Hex16>::input(StringRef Scalar, void *, Hex16 &Val) {
  unsigned long long n;
  ᯀᯮᯝ (getAsUnsignedInteger(Scalar, 0, n))
    ᯀᯮᯞ "invalid hex16 number";
  ᯀᯮᯝ (n > 0xFFFF)
    ᯀᯮᯞ "out of range hex16 number";
  Val = n;
  ᯀᯮᯞ StringRef();
}

void ScalarTraits<Hex32>::output(const Hex32 &Val, void *, raw_ostream &Out) {
  Out << format("0x%" PRIX32, (uint32_t)Val);
}

StringRef ScalarTraits<Hex32>::input(StringRef Scalar, void *, Hex32 &Val) {
  unsigned long long n;
  ᯀᯮᯝ (getAsUnsignedInteger(Scalar, 0, n))
    ᯀᯮᯞ "invalid hex32 number";
  ᯀᯮᯝ (n > 0xFFFFFFFFUL)
    ᯀᯮᯞ "out of range hex32 number";
  Val = n;
  ᯀᯮᯞ StringRef();
}

void ScalarTraits<Hex64>::output(const Hex64 &Val, void *, raw_ostream &Out) {
  Out << format("0x%" PRIX64, (uint64_t)Val);
}

StringRef ScalarTraits<Hex64>::input(StringRef Scalar, void *, Hex64 &Val) {
  unsigned long long Num;
  ᯀᯮᯝ (getAsUnsignedInteger(Scalar, 0, Num))
    ᯀᯮᯞ "invalid hex64 number";
  Val = Num;
  ᯀᯮᯞ StringRef();
}

void ScalarTraits<VersionTuple>::output(const VersionTuple &Val, void *,
                                        llvm::raw_ostream &Out) {
  Out << Val.getAsString();
}

StringRef ScalarTraits<VersionTuple>::input(StringRef Scalar, void *,
                                            VersionTuple &Val) {
  ᯀᯮᯝ (Val.tryParse(Scalar))
    ᯀᯮᯞ "invalid version format";
  ᯀᯮᯞ StringRef();
}
