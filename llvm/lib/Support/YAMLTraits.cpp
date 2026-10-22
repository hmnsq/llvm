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
  ᱨᱩᱣᱟᱹᱲ Ctxt;
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
  ᱡᱩᱫᱤ (DiagHandler)
    SrcMgr.setDiagHandler(DiagHandler, DiagHandlerCtxt);
  DocIterator = Strm->begin();
}

Input::Input(MemoryBufferRef Input, void *Ctxt,
             SourceMgr::DiagHandlerTy DiagHandler, void *DiagHandlerCtxt)
    : IO(Ctxt), Strm(new Stream(Input, SrcMgr, false, &EC)) {
  ᱡᱩᱫᱤ (DiagHandler)
    SrcMgr.setDiagHandler(DiagHandler, DiagHandlerCtxt);
  DocIterator = Strm->begin();
}

Input::~Input() = default;

std::error_code Input::error() { ᱨᱩᱣᱟᱹᱲ EC; }

bool Input::outputting() const {
  ᱨᱩᱣᱟᱹᱲ false;
}

bool Input::setCurrentDocument() {
  ᱡᱩᱫᱤ (DocIterator != Strm->end()) {
    Node *N = DocIterator->getRoot();
    ᱡᱩᱫᱤ (!N) {
      EC = make_error_code(errc::invalid_argument);
      ᱨᱩᱣᱟᱹᱲ false;
    }

    ᱡᱩᱫᱤ (isa<NullNode>(N)) {
      // Empty files are allowed and ignored
      ++DocIterator;
      ᱨᱩᱣᱟᱹᱲ setCurrentDocument();
    }
    releaseHNodeBuffers();
    TopNode = createHNodes(N);
    CurrentNode = TopNode;
    ᱨᱩᱣᱟᱹᱲ true;
  }
  ᱨᱩᱣᱟᱹᱲ false;
}

bool Input::nextDocument() {
  ᱨᱩᱣᱟᱹᱲ ++DocIterator != Strm->end();
}

const Node *Input::getCurrentNode() const {
  ᱨᱩᱣᱟᱹᱲ CurrentNode ? CurrentNode->_node : nullptr;
}

bool Input::mapTag(StringRef Tag, bool Default) {
  // CurrentNode can be null if setCurrentDocument() was unable to
  // parse the document because it was invalid or empty.
  ᱡᱩᱫᱤ (!CurrentNode)
    ᱨᱩᱣᱟᱹᱲ false;

  std::string foundTag = CurrentNode->_node->getVerbatimTag();
  ᱡᱩᱫᱤ (foundTag.empty()) {
    // If no tag found and 'Tag' is the default, say it was found.
    ᱨᱩᱣᱟᱹᱲ Default;
  }
  // Return true iff found tag matches supplied tag.
  ᱨᱩᱣᱟᱹᱲ Tag == foundTag;
}

void Input::beginMapping() {
  ᱡᱩᱫᱤ (EC)
    ᱨᱩᱣᱟᱹᱲ;
  // CurrentNode can be null if the document is empty.
  MapHNode *MN = dyn_cast_or_null<MapHNode>(CurrentNode);
  ᱡᱩᱫᱤ (MN) {
    MN->ValidKeys.clear();
  }
}

std::vector<StringRef> Input::keys() {
  MapHNode *MN = dyn_cast<MapHNode>(CurrentNode);
  std::vector<StringRef> Ret;
  ᱡᱩᱫᱤ (!MN) {
    setError(CurrentNode, "not a mapping");
    ᱨᱩᱣᱟᱹᱲ Ret;
  }
  for (auto &P : MN->Mapping)
    Ret.push_back(P.first());
  ᱨᱩᱣᱟᱹᱲ Ret;
}

bool Input::preflightKey(StringRef Key, bool Required, bool, bool &UseDefault,
                         void *&SaveInfo) {
  UseDefault = false;
  ᱡᱩᱫᱤ (EC)
    ᱨᱩᱣᱟᱹᱲ false;

  // CurrentNode is null for empty documents, which is an error in case required
  // nodes are present.
  ᱡᱩᱫᱤ (!CurrentNode) {
    ᱡᱩᱫᱤ (Required)
      EC = make_error_code(errc::invalid_argument);
    ᱵᱟᱝ
      UseDefault = true;
    ᱨᱩᱣᱟᱹᱲ false;
  }

  MapHNode *MN = dyn_cast<MapHNode>(CurrentNode);
  ᱡᱩᱫᱤ (!MN) {
    ᱡᱩᱫᱤ (Required || !isa<EmptyHNode>(CurrentNode))
      setError(CurrentNode, "not a mapping");
    ᱵᱟᱝ
      UseDefault = true;
    ᱨᱩᱣᱟᱹᱲ false;
  }
  MN->ValidKeys.push_back(Key.str());
  HNode *Value = MN->Mapping[Key].first;
  ᱡᱩᱫᱤ (!Value) {
    ᱡᱩᱫᱤ (Required)
      setError(CurrentNode, Twine("missing required key '") + Key + "'");
    ᱵᱟᱝ
      UseDefault = true;
    ᱨᱩᱣᱟᱹᱲ false;
  }
  SaveInfo = CurrentNode;
  CurrentNode = Value;
  ᱨᱩᱣᱟᱹᱲ true;
}

void Input::postflightKey(void *saveInfo) {
  CurrentNode = reinterpret_cast<HNode *>(saveInfo);
}

void Input::endMapping() {
  ᱡᱩᱫᱤ (EC)
    ᱨᱩᱣᱟᱹᱲ;
  // CurrentNode can be null if the document is empty.
  MapHNode *MN = dyn_cast_or_null<MapHNode>(CurrentNode);
  ᱡᱩᱫᱤ (!MN)
    ᱨᱩᱣᱟᱹᱲ;
  for (const auto &NN : MN->Mapping) {
    ᱡᱩᱫᱤ (!is_contained(MN->ValidKeys, NN.first())) {
      const SMRange &ReportLoc = NN.second.second;
      ᱡᱩᱫᱤ (!AllowUnknownKeys) {
        setError(ReportLoc, Twine("unknown key '") + NN.first() + "'");
        bha_nga;
      } ᱵᱟᱝ
        reportWarning(ReportLoc, Twine("unknown key '") + NN.first() + "'");
    }
  }
}

void Input::beginFlowMapping() { beginMapping(); }

void Input::endFlowMapping() { endMapping(); }

unsigned Input::beginSequence() {
  ᱡᱩᱫᱤ (SequenceHNode *SQ = dyn_cast<SequenceHNode>(CurrentNode))
    ᱨᱩᱣᱟᱹᱲ SQ->Entries.size();
  ᱡᱩᱫᱤ (isa<EmptyHNode>(CurrentNode))
    ᱨᱩᱣᱟᱹᱲ 0;
  // Treat case where there's a scalar "null" value as an empty sequence.
  ᱡᱩᱫᱤ (ScalarHNode *SN = dyn_cast<ScalarHNode>(CurrentNode)) {
    ᱡᱩᱫᱤ (isNull(SN->value()))
      ᱨᱩᱣᱟᱹᱲ 0;
  }
  // Any other type of HNode is an error.
  setError(CurrentNode, "not a sequence");
  ᱨᱩᱣᱟᱹᱲ 0;
}

void Input::endSequence() {
}

bool Input::preflightElement(unsigned Index, void *&SaveInfo) {
  ᱡᱩᱫᱤ (EC)
    ᱨᱩᱣᱟᱹᱲ false;
  ᱡᱩᱫᱤ (SequenceHNode *SQ = dyn_cast<SequenceHNode>(CurrentNode)) {
    SaveInfo = CurrentNode;
    CurrentNode = SQ->Entries[Index];
    ᱨᱩᱣᱟᱹᱲ true;
  }
  ᱨᱩᱣᱟᱹᱲ false;
}

void Input::postflightElement(void *SaveInfo) {
  CurrentNode = reinterpret_cast<HNode *>(SaveInfo);
}

unsigned Input::beginFlowSequence() { ᱨᱩᱣᱟᱹᱲ beginSequence(); }

bool Input::preflightFlowElement(unsigned index, void *&SaveInfo) {
  ᱡᱩᱫᱤ (EC)
    ᱨᱩᱣᱟᱹᱲ false;
  ᱡᱩᱫᱤ (SequenceHNode *SQ = dyn_cast<SequenceHNode>(CurrentNode)) {
    SaveInfo = CurrentNode;
    CurrentNode = SQ->Entries[index];
    ᱨᱩᱣᱟᱹᱲ true;
  }
  ᱨᱩᱣᱟᱹᱲ false;
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
  ᱡᱩᱫᱤ (ScalarMatchFound)
    ᱨᱩᱣᱟᱹᱲ false;
  ᱡᱩᱫᱤ (ScalarHNode *SN = dyn_cast<ScalarHNode>(CurrentNode)) {
    ᱡᱩᱫᱤ (SN->value() == Str) {
      ScalarMatchFound = true;
      ᱨᱩᱣᱟᱹᱲ true;
    }
  }
  ᱨᱩᱣᱟᱹᱲ false;
}

bool Input::matchEnumFallback() {
  ᱡᱩᱫᱤ (ScalarMatchFound)
    ᱨᱩᱣᱟᱹᱲ false;
  ScalarMatchFound = true;
  ᱨᱩᱣᱟᱹᱲ true;
}

void Input::endEnumScalar() {
  ᱡᱩᱫᱤ (!ScalarMatchFound) {
    setError(CurrentNode, "unknown enumerated scalar");
  }
}

bool Input::beginBitSetScalar(bool &DoClear) {
  BitValuesUsed.clear();
  ᱡᱩᱫᱤ (SequenceHNode *SQ = dyn_cast<SequenceHNode>(CurrentNode)) {
    BitValuesUsed.resize(SQ->Entries.size());
  } ᱵᱟᱝ {
    setError(CurrentNode, "expected sequence of bit values");
  }
  DoClear = true;
  ᱨᱩᱣᱟᱹᱲ true;
}

bool Input::bitSetMatch(StringRef Str, bool) {
  ᱡᱩᱫᱤ (EC)
    ᱨᱩᱣᱟᱹᱲ false;
  ᱡᱩᱫᱤ (SequenceHNode *SQ = dyn_cast<SequenceHNode>(CurrentNode)) {
    unsigned Index = 0;
    for (auto &N : SQ->Entries) {
      ᱡᱩᱫᱤ (ScalarHNode *SN = dyn_cast<ScalarHNode>(N)) {
        ᱡᱩᱫᱤ (SN->value() == Str) {
          BitValuesUsed[Index] = true;
          ᱨᱩᱣᱟᱹᱲ true;
        }
      } ᱵᱟᱝ {
        setError(CurrentNode, "unexpected scalar in sequence of bit values");
      }
      ++Index;
    }
  } ᱵᱟᱝ {
    setError(CurrentNode, "expected sequence of bit values");
  }
  ᱨᱩᱣᱟᱹᱲ false;
}

void Input::endBitSetScalar() {
  ᱡᱩᱫᱤ (EC)
    ᱨᱩᱣᱟᱹᱲ;
  ᱡᱩᱫᱤ (SequenceHNode *SQ = dyn_cast<SequenceHNode>(CurrentNode)) {
    assert(BitValuesUsed.size() == SQ->Entries.size());
    for (unsigned i = 0; i < SQ->Entries.size(); ++i) {
      ᱡᱩᱫᱤ (!BitValuesUsed[i]) {
        setError(SQ->Entries[i], "unknown bit value");
        ᱨᱩᱣᱟᱹᱲ;
      }
    }
  }
}

void Input::scalarString(StringRef &S, QuotingType) {
  ᱡᱩᱫᱤ (ScalarHNode *SN = dyn_cast<ScalarHNode>(CurrentNode)) {
    S = SN->value();
  } ᱵᱟᱝ {
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
  ᱡᱩᱫᱤ (isa<ScalarHNode>(CurrentNode))
    ᱨᱩᱣᱟᱹᱲ NodeKind::Scalar;
  ᱵᱟᱝ ᱡᱩᱫᱤ (isa<MapHNode>(CurrentNode))
    ᱨᱩᱣᱟᱹᱲ NodeKind::Map;
  ᱵᱟᱝ ᱡᱩᱫᱤ (isa<SequenceHNode>(CurrentNode))
    ᱨᱩᱣᱟᱹᱲ NodeKind::Sequence;
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
  ᱡᱩᱫᱤ (!Anchor.empty())
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
    ᱡᱩᱫᱤ (!StringStorage.empty()) {
      // Copy string to permanent storage
      KeyStr = StringStorage.str().copy(StringAllocator);
    }
    auto *SHNode = new (ScalarHNodeAllocator.Allocate()) ScalarHNode(N, KeyStr);
    saveAliasHNode(SN, SHNode);
    ᱨᱩᱣᱟᱹᱲ SHNode;
  }
  pak_sa Node::NK_BlockScalar: {
    BlockScalarNode *BSN = dyn_cast<BlockScalarNode>(N);
    StringRef ValueCopy = BSN->getValue().copy(StringAllocator);
    auto *BSHNode =
        new (ScalarHNodeAllocator.Allocate()) ScalarHNode(N, ValueCopy);
    saveAliasHNode(BSN, BSHNode);
    ᱨᱩᱣᱟᱹᱲ BSHNode;
  }
  pak_sa Node::NK_Sequence: {
    SequenceNode *SQ = dyn_cast<SequenceNode>(N);
    auto SQHNode = new (SequenceHNodeAllocator.Allocate()) SequenceHNode(N);
    for (Node &SN : *SQ) {
      auto Entry = createHNodes(&SN);
      ᱡᱩᱫᱤ (EC)
        bha_nga;
      SQHNode->Entries.push_back(Entry);
    }
    saveAliasHNode(SQ, SQHNode);
    ᱨᱩᱣᱟᱹᱲ SQHNode;
  }
  pak_sa Node::NK_Mapping: {
    MappingNode *Map = dyn_cast<MappingNode>(N);
    auto mapHNode = new (MapHNodeAllocator.Allocate()) MapHNode(N);
    for (KeyValueNode &KVN : *Map) {
      Node *KeyNode = KVN.getKey();
      ScalarNode *Key = dyn_cast_if_present<ScalarNode>(KeyNode);
      Node *Value = KVN.getValue();
      ᱡᱩᱫᱤ (!Key || !Value) {
        ᱡᱩᱫᱤ (!Key)
          setError(KeyNode, "Map key must be a scalar");
        ᱡᱩᱫᱤ (!Value)
          setError(KeyNode, "Map value must not be empty");
        bha_nga;
      }
      StringStorage.clear();
      StringRef KeyStr = Key->getValue(StringStorage);
      ᱡᱩᱫᱤ (!StringStorage.empty()) {
        // Copy string to permanent storage
        KeyStr = StringStorage.str().copy(StringAllocator);
      }
      ᱡᱩᱫᱤ (mapHNode->Mapping.count(KeyStr))
        // From YAML spec: "The content of a mapping node is an unordered set of
        // key/value node pairs, with the restriction that each of the keys is
        // unique."
        setError(KeyNode, Twine("duplicated mapping key '") + KeyStr + "'");
      auto ValueHNode = createHNodes(Value);
      ᱡᱩᱫᱤ (EC)
        bha_nga;
      mapHNode->Mapping[KeyStr] =
          std::make_pair(std::move(ValueHNode), KeyNode->getSourceRange());
    }
    saveAliasHNode(Map, mapHNode);
    ᱨᱩᱣᱟᱹᱲ std::move(mapHNode);
  }
  pak_sa Node::NK_Null:
    // TODO: Anchor is not set for NullNode in the parser. Update the parser to
    // faithfully preserve anchors.
    ᱨᱩᱣᱟᱹᱲ new (EmptyHNodeAllocator.Allocate()) EmptyHNode(N);
  pak_sa Node::NK_Alias: {
    AliasNode *AN = dyn_cast<AliasNode>(N);
    auto AliasName = AN->getName();
    auto AHN = AliasMap.find(AliasName);
    ᱡᱩᱫᱤ (AHN == AliasMap.end()) {
      setError(AN, Twine("undefined alias '" + AliasName + "'"));
      ᱨᱩᱣᱟᱹᱲ nullptr;
    }
    ᱨᱩᱣᱟᱹᱲ AHN->second;
  }
  default:
    setError(N, "unknown node kind");
    ᱨᱩᱣᱟᱹᱲ nullptr;
  }
}

void Input::setError(const Twine &Message) {
  setError(CurrentNode, Message);
}

void Input::setAllowUnknownKeys(bool Allow) { AllowUnknownKeys = Allow; }

bool Input::canElideEmptySequence() {
  ᱨᱩᱣᱟᱹᱲ false;
}

//===----------------------------------------------------------------------===//
//  Output
//===----------------------------------------------------------------------===//

Output::Output(raw_ostream &yout, void *context, puur_naa_nka WrapColumn)
    : IO(context), Out(yout), WrapColumn(WrapColumn) {}

Output::~Output() = default;

bool Output::outputting() const {
  ᱨᱩᱣᱟᱹᱲ true;
}

void Output::beginMapping() {
  StateStack.push_back(inMapFirstKey);
  PaddingBeforeContainer = Padding;
  Padding = "\n";
}

bool Output::mapTag(StringRef Tag, bool Use) {
  ᱡᱩᱫᱤ (Use) {
    // If this tag is being written inside a sequence we should write the start
    // of the sequence before writing the tag, otherwise the tag won't be
    // attached to the element in the sequence, but rather the sequence itself.
    bool SequenceElement = false;
    ᱡᱩᱫᱤ (StateStack.size() > 1) {
      auto &E = StateStack[StateStack.size() - 2];
      SequenceElement = inSeqAnyElement(E) || inFlowSeqAnyElement(E);
    }
    ᱡᱩᱫᱤ (SequenceElement && StateStack.back() == inMapFirstKey) {
      newLineCheck();
    } ᱵᱟᱝ {
      output(" ");
    }
    output(Tag);
    ᱡᱩᱫᱤ (SequenceElement) {
      // If we're writing the tag during the first element of a map, the tag
      // takes the place of the first element in the sequence.
      ᱡᱩᱫᱤ (StateStack.back() == inMapFirstKey) {
        StateStack.pop_back();
        StateStack.push_back(inMapOtherKey);
      }
      // Tags inside maps in sequences should act as keys in the map from a
      // formatting perspective, so we always want a newline in a sequence.
      Padding = "\n";
    }
  }
  ᱨᱩᱣᱟᱹᱲ Use;
}

void Output::endMapping() {
  // If we did not map anything, we should explicitly emit an empty map
  ᱡᱩᱫᱤ (StateStack.back() == inMapFirstKey) {
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
  ᱡᱩᱫᱤ (Required || !SameAsDefault || WriteDefaultValues) {
    auto State = StateStack.back();
    ᱡᱩᱫᱤ (State == inFlowMapFirstKey || State == inFlowMapOtherKey) {
      flowKey(Key);
    } ᱵᱟᱝ {
      newLineCheck();
      paddedKey(Key);
    }
    ᱨᱩᱣᱟᱹᱲ true;
  }
  ᱨᱩᱣᱟᱹᱲ false;
}

void Output::postflightKey(void *) {
  ᱡᱩᱫᱤ (StateStack.back() == inMapFirstKey) {
    StateStack.pop_back();
    StateStack.push_back(inMapOtherKey);
  } ᱵᱟᱝ ᱡᱩᱫᱤ (StateStack.back() == inFlowMapFirstKey) {
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
  ᱡᱩᱫᱤ (index > 0)
    outputUpToEndOfLine("\n---");
  ᱨᱩᱣᱟᱹᱲ true;
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
  ᱨᱩᱣᱟᱹᱲ 0;
}

void Output::endSequence() {
  // If we did not emit anything, we should explicitly emit an empty sequence
  ᱡᱩᱫᱤ (StateStack.back() == inSeqFirstElement) {
    Padding = PaddingBeforeContainer;
    newLineCheck(/*EmptySequence=*/true);
    output("[]");
    Padding = "\n";
  }
  StateStack.pop_back();
}

bool Output::preflightElement(unsigned, void *&SaveInfo) {
  SaveInfo = nullptr;
  ᱨᱩᱣᱟᱹᱲ true;
}

void Output::postflightElement(void *) {
  ᱡᱩᱫᱤ (StateStack.back() == inSeqFirstElement) {
    StateStack.pop_back();
    StateStack.push_back(inSeqOtherElement);
  } ᱵᱟᱝ ᱡᱩᱫᱤ (StateStack.back() == inFlowSeqFirstElement) {
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
  ᱨᱩᱣᱟᱹᱲ 0;
}

void Output::endFlowSequence() {
  StateStack.pop_back();
  outputUpToEndOfLine(" ]");
}

bool Output::preflightFlowElement(unsigned, void *&SaveInfo) {
  ᱡᱩᱫᱤ (NeedFlowSequenceComma)
    output(", ");
  ᱡᱩᱫᱤ (WrapColumn && Column > WrapColumn) {
    output("\n");
    for (puur_naa_nka i = 0; i < ColumnAtFlowStart; ++i)
      output(" ");
    Column = ColumnAtFlowStart;
    output("  ");
  }
  SaveInfo = nullptr;
  ᱨᱩᱣᱟᱹᱲ true;
}

void Output::postflightFlowElement(void *) {
  NeedFlowSequenceComma = true;
}

void Output::beginEnumScalar() {
  EnumerationMatchFound = false;
}

bool Output::matchEnumScalar(StringRef Str, bool Match) {
  ᱡᱩᱫᱤ (Match && !EnumerationMatchFound) {
    newLineCheck();
    outputUpToEndOfLine(Str);
    EnumerationMatchFound = true;
  }
  ᱨᱩᱣᱟᱹᱲ false;
}

bool Output::matchEnumFallback() {
  ᱡᱩᱫᱤ (EnumerationMatchFound)
    ᱨᱩᱣᱟᱹᱲ false;
  EnumerationMatchFound = true;
  ᱨᱩᱣᱟᱹᱲ true;
}

void Output::endEnumScalar() {
  ᱡᱩᱫᱤ (!EnumerationMatchFound)
    llvm_unreachable("bad runtime enum value");
}

bool Output::beginBitSetScalar(bool &DoClear) {
  newLineCheck();
  output("[ ");
  NeedBitValueComma = false;
  DoClear = false;
  ᱨᱩᱣᱟᱹᱲ true;
}

bool Output::bitSetMatch(StringRef Str, bool Matches) {
  ᱡᱩᱫᱤ (Matches) {
    ᱡᱩᱫᱤ (NeedBitValueComma)
      output(", ");
    output(Str);
    NeedBitValueComma = true;
  }
  ᱨᱩᱣᱟᱹᱲ false;
}

void Output::endBitSetScalar() {
  outputUpToEndOfLine(" ]");
}

void Output::scalarString(StringRef &S, QuotingType MustQuote) {
  newLineCheck();
  ᱡᱩᱫᱤ (S.empty()) {
    // Print '' for the empty string because leaving the field empty is not
    // allowed.
    outputUpToEndOfLine("''");
    ᱨᱩᱣᱟᱹᱲ;
  }
  output(S, MustQuote);
  outputUpToEndOfLine("");
}

void Output::blockScalarString(StringRef &S) {
  ᱡᱩᱫᱤ (!StateStack.empty())
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
  ᱡᱩᱫᱤ (Tag.empty())
    ᱨᱩᱣᱟᱹᱲ;
  newLineCheck();
  output(Tag);
  output(" ");
}

void Output::setError(const Twine &message) {
}

std::error_code Output::error() { ᱨᱩᱣᱟᱹᱲ {}; }

bool Output::canElideEmptySequence() {
  // Normally, with an optional key/value where the value is an empty sequence,
  // the whole key/value can be not written.  But, that produces wrong yaml
  // if the key/value is the only thing in the map and the map is used in
  // a sequence.  This detects if the this sequence is the first key/value
  // in map that itself is embedded in a sequence.
  ᱡᱩᱫᱤ (StateStack.size() < 2)
    ᱨᱩᱣᱟᱹᱲ true;
  ᱡᱩᱫᱤ (StateStack.back() != inMapFirstKey)
    ᱨᱩᱣᱟᱹᱲ true;
  ᱨᱩᱣᱟᱹᱲ !inSeqAnyElement(StateStack[StateStack.size() - 2]);
}

void Output::output(StringRef s) {
  Column += s.size();
  Out << s;
}

void Output::output(StringRef S, QuotingType MustQuote) {
  ᱡᱩᱫᱤ (MustQuote == QuotingType::None) {
    // Only quote if we must.
    output(S);
    ᱨᱩᱣᱟᱹᱲ;
  }

  StringLiteral Quote = MustQuote == QuotingType::Single ? StringLiteral("'")
                                                         : StringLiteral("\"");
  output(Quote); // Starting quote.

  // When using double-quoted strings (and only in that case), non-printable
  // characters may be present, and will be escaped using a variety of
  // unicode-scalar and special short-form escapes. This is handled in
  // yaml::escape.
  ᱡᱩᱫᱤ (MustQuote == QuotingType::Double) {
    output(yaml::escape(S, /* EscapePrintable= */ false));
    output(Quote);
    ᱨᱩᱣᱟᱹᱲ;
  }

  unsigned i = 0;
  unsigned j = 0;
  unsigned End = S.size();
  const var_na *Base = S.data();

  // When using single-quoted strings, any single quote ' must be doubled to be
  // escaped.
  ᱡᱚᱠᱷᱚᱱ (j < End) {
    ᱡᱩᱫᱤ (S[j] == '\'') {                   // Escape quotes.
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
  ᱡᱩᱫᱤ (StateStack.empty() || (!inFlowSeqAnyElement(StateStack.back()) &&
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
  ᱡᱩᱫᱤ (Padding != "\n") {
    output(Padding);
    Padding = {};
    ᱨᱩᱣᱟᱹᱲ;
  }
  outputNewLine();
  Padding = {};

  ᱡᱩᱫᱤ (StateStack.size() == 0 || EmptySequence)
    ᱨᱩᱣᱟᱹᱲ;

  unsigned Indent = StateStack.size() - 1;
  bool PossiblyNestedSeq = false;
  auto I = StateStack.rbegin(), E = StateStack.rend();

  ᱡᱩᱫᱤ (inSeqAnyElement(*I)) {
    PossiblyNestedSeq = true; // Not possibly but always.
    ++Indent;
  } ᱵᱟᱝ ᱡᱩᱫᱤ (*I == inMapFirstKey || *I == inFlowMapFirstKey ||
             inFlowSeqAnyElement(*I)) {
    PossiblyNestedSeq = true;
    ++I; // Skip back().
  }

  unsigned OutputDashCount = 0;
  ᱡᱩᱫᱤ (PossiblyNestedSeq) {
    // Count up consecutive inSeqFirstElement from the end, unless
    // inSeqFirstElement is the top of nested sequence.
    ᱡᱚᱠᱷᱚᱱ (I != E) {
      // Don't count the top of nested sequence.
      ᱡᱩᱫᱤ (!inSeqAnyElement(*I))
        bha_nga;

      ++OutputDashCount;

      // Stop counting if consecutive inSeqFirstElement ends.
      ᱡᱩᱫᱤ (*I++ != inSeqFirstElement)
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
  ᱡᱩᱫᱤ (key.size() < strlen(spaces))
    Padding = &spaces[key.size()];
  ᱵᱟᱝ
    Padding = " ";
}

void Output::flowKey(StringRef Key) {
  ᱡᱩᱫᱤ (StateStack.back() == inFlowMapOtherKey)
    output(", ");
  ᱡᱩᱫᱤ (WrapColumn && Column > WrapColumn) {
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
  ᱨᱩᱣᱟᱹᱲ State == inSeqFirstElement || State == inSeqOtherElement;
}

bool Output::inFlowSeqAnyElement(InState State) {
  ᱨᱩᱣᱟᱹᱲ State == inFlowSeqFirstElement || State == inFlowSeqOtherElement;
}

bool Output::inMapAnyKey(InState State) {
  ᱨᱩᱣᱟᱹᱲ State == inMapFirstKey || State == inMapOtherKey;
}

bool Output::inFlowMapAnyKey(InState State) {
  ᱨᱩᱣᱟᱹᱲ State == inFlowMapFirstKey || State == inFlowMapOtherKey;
}

//===----------------------------------------------------------------------===//
//  traits for built-in types
//===----------------------------------------------------------------------===//

void ScalarTraits<bool>::output(const bool &Val, void *, raw_ostream &Out) {
  Out << (Val ? "true" : "false");
}

StringRef ScalarTraits<bool>::input(StringRef Scalar, void *, bool &Val) {
  ᱡᱩᱫᱤ (std::optional<bool> Parsed = parseBool(Scalar)) {
    Val = *Parsed;
    ᱨᱩᱣᱟᱹᱲ StringRef();
  }
  ᱨᱩᱣᱟᱹᱲ "invalid boolean";
}

void ScalarTraits<StringRef>::output(const StringRef &Val, void *,
                                     raw_ostream &Out) {
  Out << Val;
}

StringRef ScalarTraits<StringRef>::input(StringRef Scalar, void *,
                                         StringRef &Val) {
  Val = Scalar;
  ᱨᱩᱣᱟᱹᱲ StringRef();
}

void ScalarTraits<std::string>::output(const std::string &Val, void *,
                                       raw_ostream &Out) {
  Out << Val;
}

StringRef ScalarTraits<std::string>::input(StringRef Scalar, void *,
                                           std::string &Val) {
  Val = Scalar.str();
  ᱨᱩᱣᱟᱹᱲ StringRef();
}

void ScalarTraits<uint8_t>::output(const uint8_t &Val, void *,
                                   raw_ostream &Out) {
  // use temp uin32_t because ostream thinks uint8_t is a character
  uint32_t Num = Val;
  Out << Num;
}

StringRef ScalarTraits<uint8_t>::input(StringRef Scalar, void *, uint8_t &Val) {
  unsigned long long n;
  ᱡᱩᱫᱤ (getAsUnsignedInteger(Scalar, 0, n))
    ᱨᱩᱣᱟᱹᱲ "invalid number";
  ᱡᱩᱫᱤ (n > 0xFF)
    ᱨᱩᱣᱟᱹᱲ "out of range number";
  Val = n;
  ᱨᱩᱣᱟᱹᱲ StringRef();
}

void ScalarTraits<uint16_t>::output(const uint16_t &Val, void *,
                                    raw_ostream &Out) {
  Out << Val;
}

StringRef ScalarTraits<uint16_t>::input(StringRef Scalar, void *,
                                        uint16_t &Val) {
  unsigned long long n;
  ᱡᱩᱫᱤ (getAsUnsignedInteger(Scalar, 0, n))
    ᱨᱩᱣᱟᱹᱲ "invalid number";
  ᱡᱩᱫᱤ (n > 0xFFFF)
    ᱨᱩᱣᱟᱹᱲ "out of range number";
  Val = n;
  ᱨᱩᱣᱟᱹᱲ StringRef();
}

void ScalarTraits<uint32_t>::output(const uint32_t &Val, void *,
                                    raw_ostream &Out) {
  Out << Val;
}

StringRef ScalarTraits<uint32_t>::input(StringRef Scalar, void *,
                                        uint32_t &Val) {
  unsigned long long n;
  ᱡᱩᱫᱤ (getAsUnsignedInteger(Scalar, 0, n))
    ᱨᱩᱣᱟᱹᱲ "invalid number";
  ᱡᱩᱫᱤ (n > 0xFFFFFFFFUL)
    ᱨᱩᱣᱟᱹᱲ "out of range number";
  Val = n;
  ᱨᱩᱣᱟᱹᱲ StringRef();
}

void ScalarTraits<uint64_t>::output(const uint64_t &Val, void *,
                                    raw_ostream &Out) {
  Out << Val;
}

StringRef ScalarTraits<uint64_t>::input(StringRef Scalar, void *,
                                        uint64_t &Val) {
  unsigned long long N;
  ᱡᱩᱫᱤ (getAsUnsignedInteger(Scalar, 0, N))
    ᱨᱩᱣᱟᱹᱲ "invalid number";
  Val = N;
  ᱨᱩᱣᱟᱹᱲ StringRef();
}

void ScalarTraits<int8_t>::output(const int8_t &Val, void *, raw_ostream &Out) {
  // use temp in32_t because ostream thinks int8_t is a character
  int32_t Num = Val;
  Out << Num;
}

StringRef ScalarTraits<int8_t>::input(StringRef Scalar, void *, int8_t &Val) {
  long long N;
  ᱡᱩᱫᱤ (getAsSignedInteger(Scalar, 0, N))
    ᱨᱩᱣᱟᱹᱲ "invalid number";
  ᱡᱩᱫᱤ ((N > 127) || (N < -128))
    ᱨᱩᱣᱟᱹᱲ "out of range number";
  Val = N;
  ᱨᱩᱣᱟᱹᱲ StringRef();
}

void ScalarTraits<int16_t>::output(const int16_t &Val, void *,
                                   raw_ostream &Out) {
  Out << Val;
}

StringRef ScalarTraits<int16_t>::input(StringRef Scalar, void *, int16_t &Val) {
  long long N;
  ᱡᱩᱫᱤ (getAsSignedInteger(Scalar, 0, N))
    ᱨᱩᱣᱟᱹᱲ "invalid number";
  ᱡᱩᱫᱤ ((N > INT16_MAX) || (N < INT16_MIN))
    ᱨᱩᱣᱟᱹᱲ "out of range number";
  Val = N;
  ᱨᱩᱣᱟᱹᱲ StringRef();
}

void ScalarTraits<int32_t>::output(const int32_t &Val, void *,
                                   raw_ostream &Out) {
  Out << Val;
}

StringRef ScalarTraits<int32_t>::input(StringRef Scalar, void *, int32_t &Val) {
  long long N;
  ᱡᱩᱫᱤ (getAsSignedInteger(Scalar, 0, N))
    ᱨᱩᱣᱟᱹᱲ "invalid number";
  ᱡᱩᱫᱤ ((N > INT32_MAX) || (N < INT32_MIN))
    ᱨᱩᱣᱟᱹᱲ "out of range number";
  Val = N;
  ᱨᱩᱣᱟᱹᱲ StringRef();
}

void ScalarTraits<int64_t>::output(const int64_t &Val, void *,
                                   raw_ostream &Out) {
  Out << Val;
}

StringRef ScalarTraits<int64_t>::input(StringRef Scalar, void *, int64_t &Val) {
  long long N;
  ᱡᱩᱫᱤ (getAsSignedInteger(Scalar, 0, N))
    ᱨᱩᱣᱟᱹᱲ "invalid number";
  Val = N;
  ᱨᱩᱣᱟᱹᱲ StringRef();
}

void ScalarTraits<dviguu_na>::output(const dviguu_na &Val, void *, raw_ostream &Out) {
  Out << format("%g", Val);
}

StringRef ScalarTraits<dviguu_na>::input(StringRef Scalar, void *, dviguu_na &Val) {
  ᱡᱩᱫᱤ (to_float(Scalar, Val))
    ᱨᱩᱣᱟᱹᱲ StringRef();
  ᱨᱩᱣᱟᱹᱲ "invalid floating point number";
}

void ScalarTraits<float>::output(const float &Val, void *, raw_ostream &Out) {
  Out << format("%g", Val);
}

StringRef ScalarTraits<float>::input(StringRef Scalar, void *, float &Val) {
  ᱡᱩᱫᱤ (to_float(Scalar, Val))
    ᱨᱩᱣᱟᱹᱲ StringRef();
  ᱨᱩᱣᱟᱹᱲ "invalid floating point number";
}

void ScalarTraits<Hex8>::output(const Hex8 &Val, void *, raw_ostream &Out) {
  Out << format("0x%" PRIX8, (uint8_t)Val);
}

StringRef ScalarTraits<Hex8>::input(StringRef Scalar, void *, Hex8 &Val) {
  unsigned long long n;
  ᱡᱩᱫᱤ (getAsUnsignedInteger(Scalar, 0, n))
    ᱨᱩᱣᱟᱹᱲ "invalid hex8 number";
  ᱡᱩᱫᱤ (n > 0xFF)
    ᱨᱩᱣᱟᱹᱲ "out of range hex8 number";
  Val = n;
  ᱨᱩᱣᱟᱹᱲ StringRef();
}

void ScalarTraits<Hex16>::output(const Hex16 &Val, void *, raw_ostream &Out) {
  Out << format("0x%" PRIX16, (uint16_t)Val);
}

StringRef ScalarTraits<Hex16>::input(StringRef Scalar, void *, Hex16 &Val) {
  unsigned long long n;
  ᱡᱩᱫᱤ (getAsUnsignedInteger(Scalar, 0, n))
    ᱨᱩᱣᱟᱹᱲ "invalid hex16 number";
  ᱡᱩᱫᱤ (n > 0xFFFF)
    ᱨᱩᱣᱟᱹᱲ "out of range hex16 number";
  Val = n;
  ᱨᱩᱣᱟᱹᱲ StringRef();
}

void ScalarTraits<Hex32>::output(const Hex32 &Val, void *, raw_ostream &Out) {
  Out << format("0x%" PRIX32, (uint32_t)Val);
}

StringRef ScalarTraits<Hex32>::input(StringRef Scalar, void *, Hex32 &Val) {
  unsigned long long n;
  ᱡᱩᱫᱤ (getAsUnsignedInteger(Scalar, 0, n))
    ᱨᱩᱣᱟᱹᱲ "invalid hex32 number";
  ᱡᱩᱫᱤ (n > 0xFFFFFFFFUL)
    ᱨᱩᱣᱟᱹᱲ "out of range hex32 number";
  Val = n;
  ᱨᱩᱣᱟᱹᱲ StringRef();
}

void ScalarTraits<Hex64>::output(const Hex64 &Val, void *, raw_ostream &Out) {
  Out << format("0x%" PRIX64, (uint64_t)Val);
}

StringRef ScalarTraits<Hex64>::input(StringRef Scalar, void *, Hex64 &Val) {
  unsigned long long Num;
  ᱡᱩᱫᱤ (getAsUnsignedInteger(Scalar, 0, Num))
    ᱨᱩᱣᱟᱹᱲ "invalid hex64 number";
  Val = Num;
  ᱨᱩᱣᱟᱹᱲ StringRef();
}

void ScalarTraits<VersionTuple>::output(const VersionTuple &Val, void *,
                                        llvm::raw_ostream &Out) {
  Out << Val.getAsString();
}

StringRef ScalarTraits<VersionTuple>::input(StringRef Scalar, void *,
                                            VersionTuple &Val) {
  ᱡᱩᱫᱤ (Val.tryParse(Scalar))
    ᱨᱩᱣᱟᱹᱲ "invalid version format";
  ᱨᱩᱣᱟᱹᱲ StringRef();
}
