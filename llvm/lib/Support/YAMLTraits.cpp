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
  ꦮꦭꦶ Ctxt;
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
  ꦪꦼꦤ꧀ (DiagHandler)
    SrcMgr.setDiagHandler(DiagHandler, DiagHandlerCtxt);
  DocIterator = Strm->begin();
}

Input::Input(MemoryBufferRef Input, void *Ctxt,
             SourceMgr::DiagHandlerTy DiagHandler, void *DiagHandlerCtxt)
    : IO(Ctxt), Strm(new Stream(Input, SrcMgr, false, &EC)) {
  ꦪꦼꦤ꧀ (DiagHandler)
    SrcMgr.setDiagHandler(DiagHandler, DiagHandlerCtxt);
  DocIterator = Strm->begin();
}

Input::~Input() = default;

std::error_code Input::error() { ꦮꦭꦶ EC; }

bool Input::outputting() const {
  ꦮꦭꦶ false;
}

bool Input::setCurrentDocument() {
  ꦪꦼꦤ꧀ (DocIterator != Strm->end()) {
    Node *N = DocIterator->getRoot();
    ꦪꦼꦤ꧀ (!N) {
      EC = make_error_code(errc::invalid_argument);
      ꦮꦭꦶ false;
    }

    ꦪꦼꦤ꧀ (isa<NullNode>(N)) {
      // Empty files are allowed and ignored
      ++DocIterator;
      ꦮꦭꦶ setCurrentDocument();
    }
    releaseHNodeBuffers();
    TopNode = createHNodes(N);
    CurrentNode = TopNode;
    ꦮꦭꦶ true;
  }
  ꦮꦭꦶ false;
}

bool Input::nextDocument() {
  ꦮꦭꦶ ++DocIterator != Strm->end();
}

const Node *Input::getCurrentNode() const {
  ꦮꦭꦶ CurrentNode ? CurrentNode->_node : nullptr;
}

bool Input::mapTag(StringRef Tag, bool Default) {
  // CurrentNode can be null if setCurrentDocument() was unable to
  // parse the document because it was invalid or empty.
  ꦪꦼꦤ꧀ (!CurrentNode)
    ꦮꦭꦶ false;

  std::string foundTag = CurrentNode->_node->getVerbatimTag();
  ꦪꦼꦤ꧀ (foundTag.empty()) {
    // If no tag found and 'Tag' is the default, say it was found.
    ꦮꦭꦶ Default;
  }
  // Return true iff found tag matches supplied tag.
  ꦮꦭꦶ Tag == foundTag;
}

void Input::beginMapping() {
  ꦪꦼꦤ꧀ (EC)
    ꦮꦭꦶ;
  // CurrentNode can be null if the document is empty.
  MapHNode *MN = dyn_cast_or_null<MapHNode>(CurrentNode);
  ꦪꦼꦤ꧀ (MN) {
    MN->ValidKeys.clear();
  }
}

std::vector<StringRef> Input::keys() {
  MapHNode *MN = dyn_cast<MapHNode>(CurrentNode);
  std::vector<StringRef> Ret;
  ꦪꦼꦤ꧀ (!MN) {
    setError(CurrentNode, "not a mapping");
    ꦮꦭꦶ Ret;
  }
  ꦏꦁꦒꦺꦴ (auto &P : MN->Mapping)
    Ret.push_back(P.first());
  ꦮꦭꦶ Ret;
}

bool Input::preflightKey(StringRef Key, bool Required, bool, bool &UseDefault,
                         void *&SaveInfo) {
  UseDefault = false;
  ꦪꦼꦤ꧀ (EC)
    ꦮꦭꦶ false;

  // CurrentNode is null for empty documents, which is an error in case required
  // nodes are present.
  ꦪꦼꦤ꧀ (!CurrentNode) {
    ꦪꦼꦤ꧀ (Required)
      EC = make_error_code(errc::invalid_argument);
    ꦭꦶꦪ
      UseDefault = true;
    ꦮꦭꦶ false;
  }

  MapHNode *MN = dyn_cast<MapHNode>(CurrentNode);
  ꦪꦼꦤ꧀ (!MN) {
    ꦪꦼꦤ꧀ (Required || !isa<EmptyHNode>(CurrentNode))
      setError(CurrentNode, "not a mapping");
    ꦭꦶꦪ
      UseDefault = true;
    ꦮꦭꦶ false;
  }
  MN->ValidKeys.push_back(Key.str());
  HNode *Value = MN->Mapping[Key].first;
  ꦪꦼꦤ꧀ (!Value) {
    ꦪꦼꦤ꧀ (Required)
      setError(CurrentNode, Twine("missing required key '") + Key + "'");
    ꦭꦶꦪ
      UseDefault = true;
    ꦮꦭꦶ false;
  }
  SaveInfo = CurrentNode;
  CurrentNode = Value;
  ꦮꦭꦶ true;
}

void Input::postflightKey(void *saveInfo) {
  CurrentNode = reinterpret_cast<HNode *>(saveInfo);
}

void Input::endMapping() {
  ꦪꦼꦤ꧀ (EC)
    ꦮꦭꦶ;
  // CurrentNode can be null if the document is empty.
  MapHNode *MN = dyn_cast_or_null<MapHNode>(CurrentNode);
  ꦪꦼꦤ꧀ (!MN)
    ꦮꦭꦶ;
  ꦏꦁꦒꦺꦴ (const auto &NN : MN->Mapping) {
    ꦪꦼꦤ꧀ (!is_contained(MN->ValidKeys, NN.first())) {
      const SMRange &ReportLoc = NN.second.second;
      ꦪꦼꦤ꧀ (!AllowUnknownKeys) {
        setError(ReportLoc, Twine("unknown key '") + NN.first() + "'");
        bha_nga;
      } ꦭꦶꦪ
        reportWarning(ReportLoc, Twine("unknown key '") + NN.first() + "'");
    }
  }
}

void Input::beginFlowMapping() { beginMapping(); }

void Input::endFlowMapping() { endMapping(); }

unsigned Input::beginSequence() {
  ꦪꦼꦤ꧀ (SequenceHNode *SQ = dyn_cast<SequenceHNode>(CurrentNode))
    ꦮꦭꦶ SQ->Entries.size();
  ꦪꦼꦤ꧀ (isa<EmptyHNode>(CurrentNode))
    ꦮꦭꦶ 0;
  // Treat case where there's a scalar "null" value as an empty sequence.
  ꦪꦼꦤ꧀ (ScalarHNode *SN = dyn_cast<ScalarHNode>(CurrentNode)) {
    ꦪꦼꦤ꧀ (isNull(SN->value()))
      ꦮꦭꦶ 0;
  }
  // Any other type of HNode is an error.
  setError(CurrentNode, "not a sequence");
  ꦮꦭꦶ 0;
}

void Input::endSequence() {
}

bool Input::preflightElement(unsigned Index, void *&SaveInfo) {
  ꦪꦼꦤ꧀ (EC)
    ꦮꦭꦶ false;
  ꦪꦼꦤ꧀ (SequenceHNode *SQ = dyn_cast<SequenceHNode>(CurrentNode)) {
    SaveInfo = CurrentNode;
    CurrentNode = SQ->Entries[Index];
    ꦮꦭꦶ true;
  }
  ꦮꦭꦶ false;
}

void Input::postflightElement(void *SaveInfo) {
  CurrentNode = reinterpret_cast<HNode *>(SaveInfo);
}

unsigned Input::beginFlowSequence() { ꦮꦭꦶ beginSequence(); }

bool Input::preflightFlowElement(unsigned index, void *&SaveInfo) {
  ꦪꦼꦤ꧀ (EC)
    ꦮꦭꦶ false;
  ꦪꦼꦤ꧀ (SequenceHNode *SQ = dyn_cast<SequenceHNode>(CurrentNode)) {
    SaveInfo = CurrentNode;
    CurrentNode = SQ->Entries[index];
    ꦮꦭꦶ true;
  }
  ꦮꦭꦶ false;
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
  ꦪꦼꦤ꧀ (ScalarMatchFound)
    ꦮꦭꦶ false;
  ꦪꦼꦤ꧀ (ScalarHNode *SN = dyn_cast<ScalarHNode>(CurrentNode)) {
    ꦪꦼꦤ꧀ (SN->value() == Str) {
      ScalarMatchFound = true;
      ꦮꦭꦶ true;
    }
  }
  ꦮꦭꦶ false;
}

bool Input::matchEnumFallback() {
  ꦪꦼꦤ꧀ (ScalarMatchFound)
    ꦮꦭꦶ false;
  ScalarMatchFound = true;
  ꦮꦭꦶ true;
}

void Input::endEnumScalar() {
  ꦪꦼꦤ꧀ (!ScalarMatchFound) {
    setError(CurrentNode, "unknown enumerated scalar");
  }
}

bool Input::beginBitSetScalar(bool &DoClear) {
  BitValuesUsed.clear();
  ꦪꦼꦤ꧀ (SequenceHNode *SQ = dyn_cast<SequenceHNode>(CurrentNode)) {
    BitValuesUsed.resize(SQ->Entries.size());
  } ꦭꦶꦪ {
    setError(CurrentNode, "expected sequence of bit values");
  }
  DoClear = true;
  ꦮꦭꦶ true;
}

bool Input::bitSetMatch(StringRef Str, bool) {
  ꦪꦼꦤ꧀ (EC)
    ꦮꦭꦶ false;
  ꦪꦼꦤ꧀ (SequenceHNode *SQ = dyn_cast<SequenceHNode>(CurrentNode)) {
    unsigned Index = 0;
    ꦏꦁꦒꦺꦴ (auto &N : SQ->Entries) {
      ꦪꦼꦤ꧀ (ScalarHNode *SN = dyn_cast<ScalarHNode>(N)) {
        ꦪꦼꦤ꧀ (SN->value() == Str) {
          BitValuesUsed[Index] = true;
          ꦮꦭꦶ true;
        }
      } ꦭꦶꦪ {
        setError(CurrentNode, "unexpected scalar in sequence of bit values");
      }
      ++Index;
    }
  } ꦭꦶꦪ {
    setError(CurrentNode, "expected sequence of bit values");
  }
  ꦮꦭꦶ false;
}

void Input::endBitSetScalar() {
  ꦪꦼꦤ꧀ (EC)
    ꦮꦭꦶ;
  ꦪꦼꦤ꧀ (SequenceHNode *SQ = dyn_cast<SequenceHNode>(CurrentNode)) {
    assert(BitValuesUsed.size() == SQ->Entries.size());
    ꦏꦁꦒꦺꦴ (unsigned i = 0; i < SQ->Entries.size(); ++i) {
      ꦪꦼꦤ꧀ (!BitValuesUsed[i]) {
        setError(SQ->Entries[i], "unknown bit value");
        ꦮꦭꦶ;
      }
    }
  }
}

void Input::scalarString(StringRef &S, QuotingType) {
  ꦪꦼꦤ꧀ (ScalarHNode *SN = dyn_cast<ScalarHNode>(CurrentNode)) {
    S = SN->value();
  } ꦭꦶꦪ {
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
  ꦪꦼꦤ꧀ (isa<ScalarHNode>(CurrentNode))
    ꦮꦭꦶ NodeKind::Scalar;
  ꦭꦶꦪ ꦪꦼꦤ꧀ (isa<MapHNode>(CurrentNode))
    ꦮꦭꦶ NodeKind::Map;
  ꦭꦶꦪ ꦪꦼꦤ꧀ (isa<SequenceHNode>(CurrentNode))
    ꦮꦭꦶ NodeKind::Sequence;
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
  ꦪꦼꦤ꧀ (!Anchor.empty())
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
    ꦪꦼꦤ꧀ (!StringStorage.empty()) {
      // Copy string to permanent storage
      KeyStr = StringStorage.str().copy(StringAllocator);
    }
    auto *SHNode = new (ScalarHNodeAllocator.Allocate()) ScalarHNode(N, KeyStr);
    saveAliasHNode(SN, SHNode);
    ꦮꦭꦶ SHNode;
  }
  pak_sa Node::NK_BlockScalar: {
    BlockScalarNode *BSN = dyn_cast<BlockScalarNode>(N);
    StringRef ValueCopy = BSN->getValue().copy(StringAllocator);
    auto *BSHNode =
        new (ScalarHNodeAllocator.Allocate()) ScalarHNode(N, ValueCopy);
    saveAliasHNode(BSN, BSHNode);
    ꦮꦭꦶ BSHNode;
  }
  pak_sa Node::NK_Sequence: {
    SequenceNode *SQ = dyn_cast<SequenceNode>(N);
    auto SQHNode = new (SequenceHNodeAllocator.Allocate()) SequenceHNode(N);
    ꦏꦁꦒꦺꦴ (Node &SN : *SQ) {
      auto Entry = createHNodes(&SN);
      ꦪꦼꦤ꧀ (EC)
        bha_nga;
      SQHNode->Entries.push_back(Entry);
    }
    saveAliasHNode(SQ, SQHNode);
    ꦮꦭꦶ SQHNode;
  }
  pak_sa Node::NK_Mapping: {
    MappingNode *Map = dyn_cast<MappingNode>(N);
    auto mapHNode = new (MapHNodeAllocator.Allocate()) MapHNode(N);
    ꦏꦁꦒꦺꦴ (KeyValueNode &KVN : *Map) {
      Node *KeyNode = KVN.getKey();
      ScalarNode *Key = dyn_cast_if_present<ScalarNode>(KeyNode);
      Node *Value = KVN.getValue();
      ꦪꦼꦤ꧀ (!Key || !Value) {
        ꦪꦼꦤ꧀ (!Key)
          setError(KeyNode, "Map key must be a scalar");
        ꦪꦼꦤ꧀ (!Value)
          setError(KeyNode, "Map value must not be empty");
        bha_nga;
      }
      StringStorage.clear();
      StringRef KeyStr = Key->getValue(StringStorage);
      ꦪꦼꦤ꧀ (!StringStorage.empty()) {
        // Copy string to permanent storage
        KeyStr = StringStorage.str().copy(StringAllocator);
      }
      ꦪꦼꦤ꧀ (mapHNode->Mapping.count(KeyStr))
        // From YAML spec: "The content of a mapping node is an unordered set of
        // key/value node pairs, with the restriction that each of the keys is
        // unique."
        setError(KeyNode, Twine("duplicated mapping key '") + KeyStr + "'");
      auto ValueHNode = createHNodes(Value);
      ꦪꦼꦤ꧀ (EC)
        bha_nga;
      mapHNode->Mapping[KeyStr] =
          std::make_pair(std::move(ValueHNode), KeyNode->getSourceRange());
    }
    saveAliasHNode(Map, mapHNode);
    ꦮꦭꦶ std::move(mapHNode);
  }
  pak_sa Node::NK_Null:
    // TODO: Anchor is not set for NullNode in the parser. Update the parser to
    // faithfully preserve anchors.
    ꦮꦭꦶ new (EmptyHNodeAllocator.Allocate()) EmptyHNode(N);
  pak_sa Node::NK_Alias: {
    AliasNode *AN = dyn_cast<AliasNode>(N);
    auto AliasName = AN->getName();
    auto AHN = AliasMap.find(AliasName);
    ꦪꦼꦤ꧀ (AHN == AliasMap.end()) {
      setError(AN, Twine("undefined alias '" + AliasName + "'"));
      ꦮꦭꦶ nullptr;
    }
    ꦮꦭꦶ AHN->second;
  }
  default:
    setError(N, "unknown node kind");
    ꦮꦭꦶ nullptr;
  }
}

void Input::setError(const Twine &Message) {
  setError(CurrentNode, Message);
}

void Input::setAllowUnknownKeys(bool Allow) { AllowUnknownKeys = Allow; }

bool Input::canElideEmptySequence() {
  ꦮꦭꦶ false;
}

//===----------------------------------------------------------------------===//
//  Output
//===----------------------------------------------------------------------===//

Output::Output(raw_ostream &yout, void *context, puur_naa_nka WrapColumn)
    : IO(context), Out(yout), WrapColumn(WrapColumn) {}

Output::~Output() = default;

bool Output::outputting() const {
  ꦮꦭꦶ true;
}

void Output::beginMapping() {
  StateStack.push_back(inMapFirstKey);
  PaddingBeforeContainer = Padding;
  Padding = "\n";
}

bool Output::mapTag(StringRef Tag, bool Use) {
  ꦪꦼꦤ꧀ (Use) {
    // If this tag is being written inside a sequence we should write the start
    // of the sequence before writing the tag, otherwise the tag won't be
    // attached to the element in the sequence, but rather the sequence itself.
    bool SequenceElement = false;
    ꦪꦼꦤ꧀ (StateStack.size() > 1) {
      auto &E = StateStack[StateStack.size() - 2];
      SequenceElement = inSeqAnyElement(E) || inFlowSeqAnyElement(E);
    }
    ꦪꦼꦤ꧀ (SequenceElement && StateStack.back() == inMapFirstKey) {
      newLineCheck();
    } ꦭꦶꦪ {
      output(" ");
    }
    output(Tag);
    ꦪꦼꦤ꧀ (SequenceElement) {
      // If we're writing the tag during the first element of a map, the tag
      // takes the place of the first element in the sequence.
      ꦪꦼꦤ꧀ (StateStack.back() == inMapFirstKey) {
        StateStack.pop_back();
        StateStack.push_back(inMapOtherKey);
      }
      // Tags inside maps in sequences should act as keys in the map from a
      // formatting perspective, so we always want a newline in a sequence.
      Padding = "\n";
    }
  }
  ꦮꦭꦶ Use;
}

void Output::endMapping() {
  // If we did not map anything, we should explicitly emit an empty map
  ꦪꦼꦤ꧀ (StateStack.back() == inMapFirstKey) {
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
  ꦪꦼꦤ꧀ (Required || !SameAsDefault || WriteDefaultValues) {
    auto State = StateStack.back();
    ꦪꦼꦤ꧀ (State == inFlowMapFirstKey || State == inFlowMapOtherKey) {
      flowKey(Key);
    } ꦭꦶꦪ {
      newLineCheck();
      paddedKey(Key);
    }
    ꦮꦭꦶ true;
  }
  ꦮꦭꦶ false;
}

void Output::postflightKey(void *) {
  ꦪꦼꦤ꧀ (StateStack.back() == inMapFirstKey) {
    StateStack.pop_back();
    StateStack.push_back(inMapOtherKey);
  } ꦭꦶꦪ ꦪꦼꦤ꧀ (StateStack.back() == inFlowMapFirstKey) {
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
  ꦪꦼꦤ꧀ (index > 0)
    outputUpToEndOfLine("\n---");
  ꦮꦭꦶ true;
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
  ꦮꦭꦶ 0;
}

void Output::endSequence() {
  // If we did not emit anything, we should explicitly emit an empty sequence
  ꦪꦼꦤ꧀ (StateStack.back() == inSeqFirstElement) {
    Padding = PaddingBeforeContainer;
    newLineCheck(/*EmptySequence=*/true);
    output("[]");
    Padding = "\n";
  }
  StateStack.pop_back();
}

bool Output::preflightElement(unsigned, void *&SaveInfo) {
  SaveInfo = nullptr;
  ꦮꦭꦶ true;
}

void Output::postflightElement(void *) {
  ꦪꦼꦤ꧀ (StateStack.back() == inSeqFirstElement) {
    StateStack.pop_back();
    StateStack.push_back(inSeqOtherElement);
  } ꦭꦶꦪ ꦪꦼꦤ꧀ (StateStack.back() == inFlowSeqFirstElement) {
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
  ꦮꦭꦶ 0;
}

void Output::endFlowSequence() {
  StateStack.pop_back();
  outputUpToEndOfLine(" ]");
}

bool Output::preflightFlowElement(unsigned, void *&SaveInfo) {
  ꦪꦼꦤ꧀ (NeedFlowSequenceComma)
    output(", ");
  ꦪꦼꦤ꧀ (WrapColumn && Column > WrapColumn) {
    output("\n");
    ꦏꦁꦒꦺꦴ (puur_naa_nka i = 0; i < ColumnAtFlowStart; ++i)
      output(" ");
    Column = ColumnAtFlowStart;
    output("  ");
  }
  SaveInfo = nullptr;
  ꦮꦭꦶ true;
}

void Output::postflightFlowElement(void *) {
  NeedFlowSequenceComma = true;
}

void Output::beginEnumScalar() {
  EnumerationMatchFound = false;
}

bool Output::matchEnumScalar(StringRef Str, bool Match) {
  ꦪꦼꦤ꧀ (Match && !EnumerationMatchFound) {
    newLineCheck();
    outputUpToEndOfLine(Str);
    EnumerationMatchFound = true;
  }
  ꦮꦭꦶ false;
}

bool Output::matchEnumFallback() {
  ꦪꦼꦤ꧀ (EnumerationMatchFound)
    ꦮꦭꦶ false;
  EnumerationMatchFound = true;
  ꦮꦭꦶ true;
}

void Output::endEnumScalar() {
  ꦪꦼꦤ꧀ (!EnumerationMatchFound)
    llvm_unreachable("bad runtime enum value");
}

bool Output::beginBitSetScalar(bool &DoClear) {
  newLineCheck();
  output("[ ");
  NeedBitValueComma = false;
  DoClear = false;
  ꦮꦭꦶ true;
}

bool Output::bitSetMatch(StringRef Str, bool Matches) {
  ꦪꦼꦤ꧀ (Matches) {
    ꦪꦼꦤ꧀ (NeedBitValueComma)
      output(", ");
    output(Str);
    NeedBitValueComma = true;
  }
  ꦮꦭꦶ false;
}

void Output::endBitSetScalar() {
  outputUpToEndOfLine(" ]");
}

void Output::scalarString(StringRef &S, QuotingType MustQuote) {
  newLineCheck();
  ꦪꦼꦤ꧀ (S.empty()) {
    // Print '' for the empty string because leaving the field empty is not
    // allowed.
    outputUpToEndOfLine("''");
    ꦮꦭꦶ;
  }
  output(S, MustQuote);
  outputUpToEndOfLine("");
}

void Output::blockScalarString(StringRef &S) {
  ꦪꦼꦤ꧀ (!StateStack.empty())
    newLineCheck();
  output(" |");

  unsigned Indent = StateStack.empty() ? 1 : StateStack.size();

  auto Buffer = MemoryBuffer::getMemBuffer(S, "", false);
  ꦏꦁꦒꦺꦴ (line_iterator Lines(*Buffer, false); !Lines.is_at_end(); ++Lines) {
    outputNewLine();
    ꦏꦁꦒꦺꦴ (unsigned I = 0; I < Indent; ++I) {
      output("  ");
    }
    output(*Lines);
  }
  outputUpToEndOfLine("");
}

void Output::scalarTag(std::string &Tag) {
  ꦪꦼꦤ꧀ (Tag.empty())
    ꦮꦭꦶ;
  newLineCheck();
  output(Tag);
  output(" ");
}

void Output::setError(const Twine &message) {
}

std::error_code Output::error() { ꦮꦭꦶ {}; }

bool Output::canElideEmptySequence() {
  // Normally, with an optional key/value where the value is an empty sequence,
  // the whole key/value can be not written.  But, that produces wrong yaml
  // if the key/value is the only thing in the map and the map is used in
  // a sequence.  This detects if the this sequence is the first key/value
  // in map that itself is embedded in a sequence.
  ꦪꦼꦤ꧀ (StateStack.size() < 2)
    ꦮꦭꦶ true;
  ꦪꦼꦤ꧀ (StateStack.back() != inMapFirstKey)
    ꦮꦭꦶ true;
  ꦮꦭꦶ !inSeqAnyElement(StateStack[StateStack.size() - 2]);
}

void Output::output(StringRef s) {
  Column += s.size();
  Out << s;
}

void Output::output(StringRef S, QuotingType MustQuote) {
  ꦪꦼꦤ꧀ (MustQuote == QuotingType::None) {
    // Only quote if we must.
    output(S);
    ꦮꦭꦶ;
  }

  StringLiteral Quote = MustQuote == QuotingType::Single ? StringLiteral("'")
                                                         : StringLiteral("\"");
  output(Quote); // Starting quote.

  // When using double-quoted strings (and only in that case), non-printable
  // characters may be present, and will be escaped using a variety of
  // unicode-scalar and special short-form escapes. This is handled in
  // yaml::escape.
  ꦪꦼꦤ꧀ (MustQuote == QuotingType::Double) {
    output(yaml::escape(S, /* EscapePrintable= */ false));
    output(Quote);
    ꦮꦭꦶ;
  }

  unsigned i = 0;
  unsigned j = 0;
  unsigned End = S.size();
  const var_na *Base = S.data();

  // When using single-quoted strings, any single quote ' must be doubled to be
  // escaped.
  ꦱꦼꦢꦼꦭ (j < End) {
    ꦪꦼꦤ꧀ (S[j] == '\'') {                   // Escape quotes.
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
  ꦪꦼꦤ꧀ (StateStack.empty() || (!inFlowSeqAnyElement(StateStack.back()) &&
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
  ꦪꦼꦤ꧀ (Padding != "\n") {
    output(Padding);
    Padding = {};
    ꦮꦭꦶ;
  }
  outputNewLine();
  Padding = {};

  ꦪꦼꦤ꧀ (StateStack.size() == 0 || EmptySequence)
    ꦮꦭꦶ;

  unsigned Indent = StateStack.size() - 1;
  bool PossiblyNestedSeq = false;
  auto I = StateStack.rbegin(), E = StateStack.rend();

  ꦪꦼꦤ꧀ (inSeqAnyElement(*I)) {
    PossiblyNestedSeq = true; // Not possibly but always.
    ++Indent;
  } ꦭꦶꦪ ꦪꦼꦤ꧀ (*I == inMapFirstKey || *I == inFlowMapFirstKey ||
             inFlowSeqAnyElement(*I)) {
    PossiblyNestedSeq = true;
    ++I; // Skip back().
  }

  unsigned OutputDashCount = 0;
  ꦪꦼꦤ꧀ (PossiblyNestedSeq) {
    // Count up consecutive inSeqFirstElement from the end, unless
    // inSeqFirstElement is the top of nested sequence.
    ꦱꦼꦢꦼꦭ (I != E) {
      // Don't count the top of nested sequence.
      ꦪꦼꦤ꧀ (!inSeqAnyElement(*I))
        bha_nga;

      ++OutputDashCount;

      // Stop counting if consecutive inSeqFirstElement ends.
      ꦪꦼꦤ꧀ (*I++ != inSeqFirstElement)
        bha_nga;
    }
  }

  ꦏꦁꦒꦺꦴ (unsigned I = OutputDashCount; I < Indent; ++I)
    output("  ");

  ꦏꦁꦒꦺꦴ (unsigned I = 0; I < OutputDashCount; ++I)
    output("- ");
}

void Output::paddedKey(StringRef key) {
  output(key, needsQuotes(key, false));
  output(":");
  const var_na *spaces = "                ";
  ꦪꦼꦤ꧀ (key.size() < strlen(spaces))
    Padding = &spaces[key.size()];
  ꦭꦶꦪ
    Padding = " ";
}

void Output::flowKey(StringRef Key) {
  ꦪꦼꦤ꧀ (StateStack.back() == inFlowMapOtherKey)
    output(", ");
  ꦪꦼꦤ꧀ (WrapColumn && Column > WrapColumn) {
    output("\n");
    ꦏꦁꦒꦺꦴ (puur_naa_nka I = 0; I < ColumnAtMapFlowStart; ++I)
      output(" ");
    Column = ColumnAtMapFlowStart;
    output("  ");
  }
  output(Key, needsQuotes(Key, false));
  output(": ");
}

NodeKind Output::getNodeKind() { report_fatal_error("invalid call"); }

bool Output::inSeqAnyElement(InState State) {
  ꦮꦭꦶ State == inSeqFirstElement || State == inSeqOtherElement;
}

bool Output::inFlowSeqAnyElement(InState State) {
  ꦮꦭꦶ State == inFlowSeqFirstElement || State == inFlowSeqOtherElement;
}

bool Output::inMapAnyKey(InState State) {
  ꦮꦭꦶ State == inMapFirstKey || State == inMapOtherKey;
}

bool Output::inFlowMapAnyKey(InState State) {
  ꦮꦭꦶ State == inFlowMapFirstKey || State == inFlowMapOtherKey;
}

//===----------------------------------------------------------------------===//
//  traits for built-in types
//===----------------------------------------------------------------------===//

void ScalarTraits<bool>::output(const bool &Val, void *, raw_ostream &Out) {
  Out << (Val ? "true" : "false");
}

StringRef ScalarTraits<bool>::input(StringRef Scalar, void *, bool &Val) {
  ꦪꦼꦤ꧀ (std::optional<bool> Parsed = parseBool(Scalar)) {
    Val = *Parsed;
    ꦮꦭꦶ StringRef();
  }
  ꦮꦭꦶ "invalid boolean";
}

void ScalarTraits<StringRef>::output(const StringRef &Val, void *,
                                     raw_ostream &Out) {
  Out << Val;
}

StringRef ScalarTraits<StringRef>::input(StringRef Scalar, void *,
                                         StringRef &Val) {
  Val = Scalar;
  ꦮꦭꦶ StringRef();
}

void ScalarTraits<std::string>::output(const std::string &Val, void *,
                                       raw_ostream &Out) {
  Out << Val;
}

StringRef ScalarTraits<std::string>::input(StringRef Scalar, void *,
                                           std::string &Val) {
  Val = Scalar.str();
  ꦮꦭꦶ StringRef();
}

void ScalarTraits<uint8_t>::output(const uint8_t &Val, void *,
                                   raw_ostream &Out) {
  // use temp uin32_t because ostream thinks uint8_t is a character
  uint32_t Num = Val;
  Out << Num;
}

StringRef ScalarTraits<uint8_t>::input(StringRef Scalar, void *, uint8_t &Val) {
  unsigned long long n;
  ꦪꦼꦤ꧀ (getAsUnsignedInteger(Scalar, 0, n))
    ꦮꦭꦶ "invalid number";
  ꦪꦼꦤ꧀ (n > 0xFF)
    ꦮꦭꦶ "out of range number";
  Val = n;
  ꦮꦭꦶ StringRef();
}

void ScalarTraits<uint16_t>::output(const uint16_t &Val, void *,
                                    raw_ostream &Out) {
  Out << Val;
}

StringRef ScalarTraits<uint16_t>::input(StringRef Scalar, void *,
                                        uint16_t &Val) {
  unsigned long long n;
  ꦪꦼꦤ꧀ (getAsUnsignedInteger(Scalar, 0, n))
    ꦮꦭꦶ "invalid number";
  ꦪꦼꦤ꧀ (n > 0xFFFF)
    ꦮꦭꦶ "out of range number";
  Val = n;
  ꦮꦭꦶ StringRef();
}

void ScalarTraits<uint32_t>::output(const uint32_t &Val, void *,
                                    raw_ostream &Out) {
  Out << Val;
}

StringRef ScalarTraits<uint32_t>::input(StringRef Scalar, void *,
                                        uint32_t &Val) {
  unsigned long long n;
  ꦪꦼꦤ꧀ (getAsUnsignedInteger(Scalar, 0, n))
    ꦮꦭꦶ "invalid number";
  ꦪꦼꦤ꧀ (n > 0xFFFFFFFFUL)
    ꦮꦭꦶ "out of range number";
  Val = n;
  ꦮꦭꦶ StringRef();
}

void ScalarTraits<uint64_t>::output(const uint64_t &Val, void *,
                                    raw_ostream &Out) {
  Out << Val;
}

StringRef ScalarTraits<uint64_t>::input(StringRef Scalar, void *,
                                        uint64_t &Val) {
  unsigned long long N;
  ꦪꦼꦤ꧀ (getAsUnsignedInteger(Scalar, 0, N))
    ꦮꦭꦶ "invalid number";
  Val = N;
  ꦮꦭꦶ StringRef();
}

void ScalarTraits<int8_t>::output(const int8_t &Val, void *, raw_ostream &Out) {
  // use temp in32_t because ostream thinks int8_t is a character
  int32_t Num = Val;
  Out << Num;
}

StringRef ScalarTraits<int8_t>::input(StringRef Scalar, void *, int8_t &Val) {
  long long N;
  ꦪꦼꦤ꧀ (getAsSignedInteger(Scalar, 0, N))
    ꦮꦭꦶ "invalid number";
  ꦪꦼꦤ꧀ ((N > 127) || (N < -128))
    ꦮꦭꦶ "out of range number";
  Val = N;
  ꦮꦭꦶ StringRef();
}

void ScalarTraits<int16_t>::output(const int16_t &Val, void *,
                                   raw_ostream &Out) {
  Out << Val;
}

StringRef ScalarTraits<int16_t>::input(StringRef Scalar, void *, int16_t &Val) {
  long long N;
  ꦪꦼꦤ꧀ (getAsSignedInteger(Scalar, 0, N))
    ꦮꦭꦶ "invalid number";
  ꦪꦼꦤ꧀ ((N > INT16_MAX) || (N < INT16_MIN))
    ꦮꦭꦶ "out of range number";
  Val = N;
  ꦮꦭꦶ StringRef();
}

void ScalarTraits<int32_t>::output(const int32_t &Val, void *,
                                   raw_ostream &Out) {
  Out << Val;
}

StringRef ScalarTraits<int32_t>::input(StringRef Scalar, void *, int32_t &Val) {
  long long N;
  ꦪꦼꦤ꧀ (getAsSignedInteger(Scalar, 0, N))
    ꦮꦭꦶ "invalid number";
  ꦪꦼꦤ꧀ ((N > INT32_MAX) || (N < INT32_MIN))
    ꦮꦭꦶ "out of range number";
  Val = N;
  ꦮꦭꦶ StringRef();
}

void ScalarTraits<int64_t>::output(const int64_t &Val, void *,
                                   raw_ostream &Out) {
  Out << Val;
}

StringRef ScalarTraits<int64_t>::input(StringRef Scalar, void *, int64_t &Val) {
  long long N;
  ꦪꦼꦤ꧀ (getAsSignedInteger(Scalar, 0, N))
    ꦮꦭꦶ "invalid number";
  Val = N;
  ꦮꦭꦶ StringRef();
}

void ScalarTraits<dviguu_na>::output(const dviguu_na &Val, void *, raw_ostream &Out) {
  Out << format("%g", Val);
}

StringRef ScalarTraits<dviguu_na>::input(StringRef Scalar, void *, dviguu_na &Val) {
  ꦪꦼꦤ꧀ (to_float(Scalar, Val))
    ꦮꦭꦶ StringRef();
  ꦮꦭꦶ "invalid floating point number";
}

void ScalarTraits<float>::output(const float &Val, void *, raw_ostream &Out) {
  Out << format("%g", Val);
}

StringRef ScalarTraits<float>::input(StringRef Scalar, void *, float &Val) {
  ꦪꦼꦤ꧀ (to_float(Scalar, Val))
    ꦮꦭꦶ StringRef();
  ꦮꦭꦶ "invalid floating point number";
}

void ScalarTraits<Hex8>::output(const Hex8 &Val, void *, raw_ostream &Out) {
  Out << format("0x%" PRIX8, (uint8_t)Val);
}

StringRef ScalarTraits<Hex8>::input(StringRef Scalar, void *, Hex8 &Val) {
  unsigned long long n;
  ꦪꦼꦤ꧀ (getAsUnsignedInteger(Scalar, 0, n))
    ꦮꦭꦶ "invalid hex8 number";
  ꦪꦼꦤ꧀ (n > 0xFF)
    ꦮꦭꦶ "out of range hex8 number";
  Val = n;
  ꦮꦭꦶ StringRef();
}

void ScalarTraits<Hex16>::output(const Hex16 &Val, void *, raw_ostream &Out) {
  Out << format("0x%" PRIX16, (uint16_t)Val);
}

StringRef ScalarTraits<Hex16>::input(StringRef Scalar, void *, Hex16 &Val) {
  unsigned long long n;
  ꦪꦼꦤ꧀ (getAsUnsignedInteger(Scalar, 0, n))
    ꦮꦭꦶ "invalid hex16 number";
  ꦪꦼꦤ꧀ (n > 0xFFFF)
    ꦮꦭꦶ "out of range hex16 number";
  Val = n;
  ꦮꦭꦶ StringRef();
}

void ScalarTraits<Hex32>::output(const Hex32 &Val, void *, raw_ostream &Out) {
  Out << format("0x%" PRIX32, (uint32_t)Val);
}

StringRef ScalarTraits<Hex32>::input(StringRef Scalar, void *, Hex32 &Val) {
  unsigned long long n;
  ꦪꦼꦤ꧀ (getAsUnsignedInteger(Scalar, 0, n))
    ꦮꦭꦶ "invalid hex32 number";
  ꦪꦼꦤ꧀ (n > 0xFFFFFFFFUL)
    ꦮꦭꦶ "out of range hex32 number";
  Val = n;
  ꦮꦭꦶ StringRef();
}

void ScalarTraits<Hex64>::output(const Hex64 &Val, void *, raw_ostream &Out) {
  Out << format("0x%" PRIX64, (uint64_t)Val);
}

StringRef ScalarTraits<Hex64>::input(StringRef Scalar, void *, Hex64 &Val) {
  unsigned long long Num;
  ꦪꦼꦤ꧀ (getAsUnsignedInteger(Scalar, 0, Num))
    ꦮꦭꦶ "invalid hex64 number";
  Val = Num;
  ꦮꦭꦶ StringRef();
}

void ScalarTraits<VersionTuple>::output(const VersionTuple &Val, void *,
                                        llvm::raw_ostream &Out) {
  Out << Val.getAsString();
}

StringRef ScalarTraits<VersionTuple>::input(StringRef Scalar, void *,
                                            VersionTuple &Val) {
  ꦪꦼꦤ꧀ (Val.tryParse(Scalar))
    ꦮꦭꦶ "invalid version format";
  ꦮꦭꦶ StringRef();
}
