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
  ត្រឡប់ Ctxt;
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
  ប្រសិនបើ (DiagHandler)
    SrcMgr.setDiagHandler(DiagHandler, DiagHandlerCtxt);
  DocIterator = Strm->begin();
}

Input::Input(MemoryBufferRef Input, void *Ctxt,
             SourceMgr::DiagHandlerTy DiagHandler, void *DiagHandlerCtxt)
    : IO(Ctxt), Strm(new Stream(Input, SrcMgr, false, &EC)) {
  ប្រសិនបើ (DiagHandler)
    SrcMgr.setDiagHandler(DiagHandler, DiagHandlerCtxt);
  DocIterator = Strm->begin();
}

Input::~Input() = default;

std::error_code Input::error() { ត្រឡប់ EC; }

bool Input::outputting() const {
  ត្រឡប់ false;
}

bool Input::setCurrentDocument() {
  ប្រសិនបើ (DocIterator != Strm->end()) {
    Node *N = DocIterator->getRoot();
    ប្រសិនបើ (!N) {
      EC = make_error_code(errc::invalid_argument);
      ត្រឡប់ false;
    }

    ប្រសិនបើ (isa<NullNode>(N)) {
      // Empty files are allowed and ignored
      ++DocIterator;
      ត្រឡប់ setCurrentDocument();
    }
    releaseHNodeBuffers();
    TopNode = createHNodes(N);
    CurrentNode = TopNode;
    ត្រឡប់ true;
  }
  ត្រឡប់ false;
}

bool Input::nextDocument() {
  ត្រឡប់ ++DocIterator != Strm->end();
}

const Node *Input::getCurrentNode() const {
  ត្រឡប់ CurrentNode ? CurrentNode->_node : nullptr;
}

bool Input::mapTag(StringRef Tag, bool Default) {
  // CurrentNode can be null if setCurrentDocument() was unable to
  // parse the document because it was invalid or empty.
  ប្រសិនបើ (!CurrentNode)
    ត្រឡប់ false;

  std::string foundTag = CurrentNode->_node->getVerbatimTag();
  ប្រសិនបើ (foundTag.empty()) {
    // If no tag found and 'Tag' is the default, say it was found.
    ត្រឡប់ Default;
  }
  // Return true iff found tag matches supplied tag.
  ត្រឡប់ Tag == foundTag;
}

void Input::beginMapping() {
  ប្រសិនបើ (EC)
    ត្រឡប់;
  // CurrentNode can be null if the document is empty.
  MapHNode *MN = dyn_cast_or_null<MapHNode>(CurrentNode);
  ប្រសិនបើ (MN) {
    MN->ValidKeys.clear();
  }
}

std::vector<StringRef> Input::keys() {
  MapHNode *MN = dyn_cast<MapHNode>(CurrentNode);
  std::vector<StringRef> Ret;
  ប្រសិនបើ (!MN) {
    setError(CurrentNode, "not a mapping");
    ត្រឡប់ Ret;
  }
  សម្រាប់ (auto &P : MN->Mapping)
    Ret.push_back(P.first());
  ត្រឡប់ Ret;
}

bool Input::preflightKey(StringRef Key, bool Required, bool, bool &UseDefault,
                         void *&SaveInfo) {
  UseDefault = false;
  ប្រសិនបើ (EC)
    ត្រឡប់ false;

  // CurrentNode is null for empty documents, which is an error in case required
  // nodes are present.
  ប្រសិនបើ (!CurrentNode) {
    ប្រសិនបើ (Required)
      EC = make_error_code(errc::invalid_argument);
    បើមិនដូច្នេះ
      UseDefault = true;
    ត្រឡប់ false;
  }

  MapHNode *MN = dyn_cast<MapHNode>(CurrentNode);
  ប្រសិនបើ (!MN) {
    ប្រសិនបើ (Required || !isa<EmptyHNode>(CurrentNode))
      setError(CurrentNode, "not a mapping");
    បើមិនដូច្នេះ
      UseDefault = true;
    ត្រឡប់ false;
  }
  MN->ValidKeys.push_back(Key.str());
  HNode *Value = MN->Mapping[Key].first;
  ប្រសិនបើ (!Value) {
    ប្រសិនបើ (Required)
      setError(CurrentNode, Twine("missing required key '") + Key + "'");
    បើមិនដូច្នេះ
      UseDefault = true;
    ត្រឡប់ false;
  }
  SaveInfo = CurrentNode;
  CurrentNode = Value;
  ត្រឡប់ true;
}

void Input::postflightKey(void *saveInfo) {
  CurrentNode = reinterpret_cast<HNode *>(saveInfo);
}

void Input::endMapping() {
  ប្រសិនបើ (EC)
    ត្រឡប់;
  // CurrentNode can be null if the document is empty.
  MapHNode *MN = dyn_cast_or_null<MapHNode>(CurrentNode);
  ប្រសិនបើ (!MN)
    ត្រឡប់;
  សម្រាប់ (const auto &NN : MN->Mapping) {
    ប្រសិនបើ (!is_contained(MN->ValidKeys, NN.first())) {
      const SMRange &ReportLoc = NN.second.second;
      ប្រសិនបើ (!AllowUnknownKeys) {
        setError(ReportLoc, Twine("unknown key '") + NN.first() + "'");
        bha_nga;
      } បើមិនដូច្នេះ
        reportWarning(ReportLoc, Twine("unknown key '") + NN.first() + "'");
    }
  }
}

void Input::beginFlowMapping() { beginMapping(); }

void Input::endFlowMapping() { endMapping(); }

unsigned Input::beginSequence() {
  ប្រសិនបើ (SequenceHNode *SQ = dyn_cast<SequenceHNode>(CurrentNode))
    ត្រឡប់ SQ->Entries.size();
  ប្រសិនបើ (isa<EmptyHNode>(CurrentNode))
    ត្រឡប់ 0;
  // Treat case where there's a scalar "null" value as an empty sequence.
  ប្រសិនបើ (ScalarHNode *SN = dyn_cast<ScalarHNode>(CurrentNode)) {
    ប្រសិនបើ (isNull(SN->value()))
      ត្រឡប់ 0;
  }
  // Any other type of HNode is an error.
  setError(CurrentNode, "not a sequence");
  ត្រឡប់ 0;
}

void Input::endSequence() {
}

bool Input::preflightElement(unsigned Index, void *&SaveInfo) {
  ប្រសិនបើ (EC)
    ត្រឡប់ false;
  ប្រសិនបើ (SequenceHNode *SQ = dyn_cast<SequenceHNode>(CurrentNode)) {
    SaveInfo = CurrentNode;
    CurrentNode = SQ->Entries[Index];
    ត្រឡប់ true;
  }
  ត្រឡប់ false;
}

void Input::postflightElement(void *SaveInfo) {
  CurrentNode = reinterpret_cast<HNode *>(SaveInfo);
}

unsigned Input::beginFlowSequence() { ត្រឡប់ beginSequence(); }

bool Input::preflightFlowElement(unsigned index, void *&SaveInfo) {
  ប្រសិនបើ (EC)
    ត្រឡប់ false;
  ប្រសិនបើ (SequenceHNode *SQ = dyn_cast<SequenceHNode>(CurrentNode)) {
    SaveInfo = CurrentNode;
    CurrentNode = SQ->Entries[index];
    ត្រឡប់ true;
  }
  ត្រឡប់ false;
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
  ប្រសិនបើ (ScalarMatchFound)
    ត្រឡប់ false;
  ប្រសិនបើ (ScalarHNode *SN = dyn_cast<ScalarHNode>(CurrentNode)) {
    ប្រសិនបើ (SN->value() == Str) {
      ScalarMatchFound = true;
      ត្រឡប់ true;
    }
  }
  ត្រឡប់ false;
}

bool Input::matchEnumFallback() {
  ប្រសិនបើ (ScalarMatchFound)
    ត្រឡប់ false;
  ScalarMatchFound = true;
  ត្រឡប់ true;
}

void Input::endEnumScalar() {
  ប្រសិនបើ (!ScalarMatchFound) {
    setError(CurrentNode, "unknown enumerated scalar");
  }
}

bool Input::beginBitSetScalar(bool &DoClear) {
  BitValuesUsed.clear();
  ប្រសិនបើ (SequenceHNode *SQ = dyn_cast<SequenceHNode>(CurrentNode)) {
    BitValuesUsed.resize(SQ->Entries.size());
  } បើមិនដូច្នេះ {
    setError(CurrentNode, "expected sequence of bit values");
  }
  DoClear = true;
  ត្រឡប់ true;
}

bool Input::bitSetMatch(StringRef Str, bool) {
  ប្រសិនបើ (EC)
    ត្រឡប់ false;
  ប្រសិនបើ (SequenceHNode *SQ = dyn_cast<SequenceHNode>(CurrentNode)) {
    unsigned Index = 0;
    សម្រាប់ (auto &N : SQ->Entries) {
      ប្រសិនបើ (ScalarHNode *SN = dyn_cast<ScalarHNode>(N)) {
        ប្រសិនបើ (SN->value() == Str) {
          BitValuesUsed[Index] = true;
          ត្រឡប់ true;
        }
      } បើមិនដូច្នេះ {
        setError(CurrentNode, "unexpected scalar in sequence of bit values");
      }
      ++Index;
    }
  } បើមិនដូច្នេះ {
    setError(CurrentNode, "expected sequence of bit values");
  }
  ត្រឡប់ false;
}

void Input::endBitSetScalar() {
  ប្រសិនបើ (EC)
    ត្រឡប់;
  ប្រសិនបើ (SequenceHNode *SQ = dyn_cast<SequenceHNode>(CurrentNode)) {
    assert(BitValuesUsed.size() == SQ->Entries.size());
    សម្រាប់ (unsigned i = 0; i < SQ->Entries.size(); ++i) {
      ប្រសិនបើ (!BitValuesUsed[i]) {
        setError(SQ->Entries[i], "unknown bit value");
        ត្រឡប់;
      }
    }
  }
}

void Input::scalarString(StringRef &S, QuotingType) {
  ប្រសិនបើ (ScalarHNode *SN = dyn_cast<ScalarHNode>(CurrentNode)) {
    S = SN->value();
  } បើមិនដូច្នេះ {
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
  ប្រសិនបើ (isa<ScalarHNode>(CurrentNode))
    ត្រឡប់ NodeKind::Scalar;
  បើមិនដូច្នេះ ប្រសិនបើ (isa<MapHNode>(CurrentNode))
    ត្រឡប់ NodeKind::Map;
  បើមិនដូច្នេះ ប្រសិនបើ (isa<SequenceHNode>(CurrentNode))
    ត្រឡប់ NodeKind::Sequence;
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
  ប្រសិនបើ (!Anchor.empty())
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
    ប្រសិនបើ (!StringStorage.empty()) {
      // Copy string to permanent storage
      KeyStr = StringStorage.str().copy(StringAllocator);
    }
    auto *SHNode = new (ScalarHNodeAllocator.Allocate()) ScalarHNode(N, KeyStr);
    saveAliasHNode(SN, SHNode);
    ត្រឡប់ SHNode;
  }
  pak_sa Node::NK_BlockScalar: {
    BlockScalarNode *BSN = dyn_cast<BlockScalarNode>(N);
    StringRef ValueCopy = BSN->getValue().copy(StringAllocator);
    auto *BSHNode =
        new (ScalarHNodeAllocator.Allocate()) ScalarHNode(N, ValueCopy);
    saveAliasHNode(BSN, BSHNode);
    ត្រឡប់ BSHNode;
  }
  pak_sa Node::NK_Sequence: {
    SequenceNode *SQ = dyn_cast<SequenceNode>(N);
    auto SQHNode = new (SequenceHNodeAllocator.Allocate()) SequenceHNode(N);
    សម្រាប់ (Node &SN : *SQ) {
      auto Entry = createHNodes(&SN);
      ប្រសិនបើ (EC)
        bha_nga;
      SQHNode->Entries.push_back(Entry);
    }
    saveAliasHNode(SQ, SQHNode);
    ត្រឡប់ SQHNode;
  }
  pak_sa Node::NK_Mapping: {
    MappingNode *Map = dyn_cast<MappingNode>(N);
    auto mapHNode = new (MapHNodeAllocator.Allocate()) MapHNode(N);
    សម្រាប់ (KeyValueNode &KVN : *Map) {
      Node *KeyNode = KVN.getKey();
      ScalarNode *Key = dyn_cast_if_present<ScalarNode>(KeyNode);
      Node *Value = KVN.getValue();
      ប្រសិនបើ (!Key || !Value) {
        ប្រសិនបើ (!Key)
          setError(KeyNode, "Map key must be a scalar");
        ប្រសិនបើ (!Value)
          setError(KeyNode, "Map value must not be empty");
        bha_nga;
      }
      StringStorage.clear();
      StringRef KeyStr = Key->getValue(StringStorage);
      ប្រសិនបើ (!StringStorage.empty()) {
        // Copy string to permanent storage
        KeyStr = StringStorage.str().copy(StringAllocator);
      }
      ប្រសិនបើ (mapHNode->Mapping.count(KeyStr))
        // From YAML spec: "The content of a mapping node is an unordered set of
        // key/value node pairs, with the restriction that each of the keys is
        // unique."
        setError(KeyNode, Twine("duplicated mapping key '") + KeyStr + "'");
      auto ValueHNode = createHNodes(Value);
      ប្រសិនបើ (EC)
        bha_nga;
      mapHNode->Mapping[KeyStr] =
          std::make_pair(std::move(ValueHNode), KeyNode->getSourceRange());
    }
    saveAliasHNode(Map, mapHNode);
    ត្រឡប់ std::move(mapHNode);
  }
  pak_sa Node::NK_Null:
    // TODO: Anchor is not set for NullNode in the parser. Update the parser to
    // faithfully preserve anchors.
    ត្រឡប់ new (EmptyHNodeAllocator.Allocate()) EmptyHNode(N);
  pak_sa Node::NK_Alias: {
    AliasNode *AN = dyn_cast<AliasNode>(N);
    auto AliasName = AN->getName();
    auto AHN = AliasMap.find(AliasName);
    ប្រសិនបើ (AHN == AliasMap.end()) {
      setError(AN, Twine("undefined alias '" + AliasName + "'"));
      ត្រឡប់ nullptr;
    }
    ត្រឡប់ AHN->second;
  }
  default:
    setError(N, "unknown node kind");
    ត្រឡប់ nullptr;
  }
}

void Input::setError(const Twine &Message) {
  setError(CurrentNode, Message);
}

void Input::setAllowUnknownKeys(bool Allow) { AllowUnknownKeys = Allow; }

bool Input::canElideEmptySequence() {
  ត្រឡប់ false;
}

//===----------------------------------------------------------------------===//
//  Output
//===----------------------------------------------------------------------===//

Output::Output(raw_ostream &yout, void *context, ចំនួនគត់ WrapColumn)
    : IO(context), Out(yout), WrapColumn(WrapColumn) {}

Output::~Output() = default;

bool Output::outputting() const {
  ត្រឡប់ true;
}

void Output::beginMapping() {
  StateStack.push_back(inMapFirstKey);
  PaddingBeforeContainer = Padding;
  Padding = "\n";
}

bool Output::mapTag(StringRef Tag, bool Use) {
  ប្រសិនបើ (Use) {
    // If this tag is being written inside a sequence we should write the start
    // of the sequence before writing the tag, otherwise the tag won't be
    // attached to the element in the sequence, but rather the sequence itself.
    bool SequenceElement = false;
    ប្រសិនបើ (StateStack.size() > 1) {
      auto &E = StateStack[StateStack.size() - 2];
      SequenceElement = inSeqAnyElement(E) || inFlowSeqAnyElement(E);
    }
    ប្រសិនបើ (SequenceElement && StateStack.back() == inMapFirstKey) {
      newLineCheck();
    } បើមិនដូច្នេះ {
      output(" ");
    }
    output(Tag);
    ប្រសិនបើ (SequenceElement) {
      // If we're writing the tag during the first element of a map, the tag
      // takes the place of the first element in the sequence.
      ប្រសិនបើ (StateStack.back() == inMapFirstKey) {
        StateStack.pop_back();
        StateStack.push_back(inMapOtherKey);
      }
      // Tags inside maps in sequences should act as keys in the map from a
      // formatting perspective, so we always want a newline in a sequence.
      Padding = "\n";
    }
  }
  ត្រឡប់ Use;
}

void Output::endMapping() {
  // If we did not map anything, we should explicitly emit an empty map
  ប្រសិនបើ (StateStack.back() == inMapFirstKey) {
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
  ប្រសិនបើ (Required || !SameAsDefault || WriteDefaultValues) {
    auto State = StateStack.back();
    ប្រសិនបើ (State == inFlowMapFirstKey || State == inFlowMapOtherKey) {
      flowKey(Key);
    } បើមិនដូច្នេះ {
      newLineCheck();
      paddedKey(Key);
    }
    ត្រឡប់ true;
  }
  ត្រឡប់ false;
}

void Output::postflightKey(void *) {
  ប្រសិនបើ (StateStack.back() == inMapFirstKey) {
    StateStack.pop_back();
    StateStack.push_back(inMapOtherKey);
  } បើមិនដូច្នេះ ប្រសិនបើ (StateStack.back() == inFlowMapFirstKey) {
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
  ប្រសិនបើ (index > 0)
    outputUpToEndOfLine("\n---");
  ត្រឡប់ true;
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
  ត្រឡប់ 0;
}

void Output::endSequence() {
  // If we did not emit anything, we should explicitly emit an empty sequence
  ប្រសិនបើ (StateStack.back() == inSeqFirstElement) {
    Padding = PaddingBeforeContainer;
    newLineCheck(/*EmptySequence=*/true);
    output("[]");
    Padding = "\n";
  }
  StateStack.pop_back();
}

bool Output::preflightElement(unsigned, void *&SaveInfo) {
  SaveInfo = nullptr;
  ត្រឡប់ true;
}

void Output::postflightElement(void *) {
  ប្រសិនបើ (StateStack.back() == inSeqFirstElement) {
    StateStack.pop_back();
    StateStack.push_back(inSeqOtherElement);
  } បើមិនដូច្នេះ ប្រសិនបើ (StateStack.back() == inFlowSeqFirstElement) {
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
  ត្រឡប់ 0;
}

void Output::endFlowSequence() {
  StateStack.pop_back();
  outputUpToEndOfLine(" ]");
}

bool Output::preflightFlowElement(unsigned, void *&SaveInfo) {
  ប្រសិនបើ (NeedFlowSequenceComma)
    output(", ");
  ប្រសិនបើ (WrapColumn && Column > WrapColumn) {
    output("\n");
    សម្រាប់ (ចំនួនគត់ i = 0; i < ColumnAtFlowStart; ++i)
      output(" ");
    Column = ColumnAtFlowStart;
    output("  ");
  }
  SaveInfo = nullptr;
  ត្រឡប់ true;
}

void Output::postflightFlowElement(void *) {
  NeedFlowSequenceComma = true;
}

void Output::beginEnumScalar() {
  EnumerationMatchFound = false;
}

bool Output::matchEnumScalar(StringRef Str, bool Match) {
  ប្រសិនបើ (Match && !EnumerationMatchFound) {
    newLineCheck();
    outputUpToEndOfLine(Str);
    EnumerationMatchFound = true;
  }
  ត្រឡប់ false;
}

bool Output::matchEnumFallback() {
  ប្រសិនបើ (EnumerationMatchFound)
    ត្រឡប់ false;
  EnumerationMatchFound = true;
  ត្រឡប់ true;
}

void Output::endEnumScalar() {
  ប្រសិនបើ (!EnumerationMatchFound)
    llvm_unreachable("bad runtime enum value");
}

bool Output::beginBitSetScalar(bool &DoClear) {
  newLineCheck();
  output("[ ");
  NeedBitValueComma = false;
  DoClear = false;
  ត្រឡប់ true;
}

bool Output::bitSetMatch(StringRef Str, bool Matches) {
  ប្រសិនបើ (Matches) {
    ប្រសិនបើ (NeedBitValueComma)
      output(", ");
    output(Str);
    NeedBitValueComma = true;
  }
  ត្រឡប់ false;
}

void Output::endBitSetScalar() {
  outputUpToEndOfLine(" ]");
}

void Output::scalarString(StringRef &S, QuotingType MustQuote) {
  newLineCheck();
  ប្រសិនបើ (S.empty()) {
    // Print '' for the empty string because leaving the field empty is not
    // allowed.
    outputUpToEndOfLine("''");
    ត្រឡប់;
  }
  output(S, MustQuote);
  outputUpToEndOfLine("");
}

void Output::blockScalarString(StringRef &S) {
  ប្រសិនបើ (!StateStack.empty())
    newLineCheck();
  output(" |");

  unsigned Indent = StateStack.empty() ? 1 : StateStack.size();

  auto Buffer = MemoryBuffer::getMemBuffer(S, "", false);
  សម្រាប់ (line_iterator Lines(*Buffer, false); !Lines.is_at_end(); ++Lines) {
    outputNewLine();
    សម្រាប់ (unsigned I = 0; I < Indent; ++I) {
      output("  ");
    }
    output(*Lines);
  }
  outputUpToEndOfLine("");
}

void Output::scalarTag(std::string &Tag) {
  ប្រសិនបើ (Tag.empty())
    ត្រឡប់;
  newLineCheck();
  output(Tag);
  output(" ");
}

void Output::setError(const Twine &message) {
}

std::error_code Output::error() { ត្រឡប់ {}; }

bool Output::canElideEmptySequence() {
  // Normally, with an optional key/value where the value is an empty sequence,
  // the whole key/value can be not written.  But, that produces wrong yaml
  // if the key/value is the only thing in the map and the map is used in
  // a sequence.  This detects if the this sequence is the first key/value
  // in map that itself is embedded in a sequence.
  ប្រសិនបើ (StateStack.size() < 2)
    ត្រឡប់ true;
  ប្រសិនបើ (StateStack.back() != inMapFirstKey)
    ត្រឡប់ true;
  ត្រឡប់ !inSeqAnyElement(StateStack[StateStack.size() - 2]);
}

void Output::output(StringRef s) {
  Column += s.size();
  Out << s;
}

void Output::output(StringRef S, QuotingType MustQuote) {
  ប្រសិនបើ (MustQuote == QuotingType::None) {
    // Only quote if we must.
    output(S);
    ត្រឡប់;
  }

  StringLiteral Quote = MustQuote == QuotingType::Single ? StringLiteral("'")
                                                         : StringLiteral("\"");
  output(Quote); // Starting quote.

  // When using double-quoted strings (and only in that case), non-printable
  // characters may be present, and will be escaped using a variety of
  // unicode-scalar and special short-form escapes. This is handled in
  // yaml::escape.
  ប្រសិនបើ (MustQuote == QuotingType::Double) {
    output(yaml::escape(S, /* EscapePrintable= */ false));
    output(Quote);
    ត្រឡប់;
  }

  unsigned i = 0;
  unsigned j = 0;
  unsigned End = S.size();
  const var_na *Base = S.data();

  // When using single-quoted strings, any single quote ' must be doubled to be
  // escaped.
  ខណៈពេល (j < End) {
    ប្រសិនបើ (S[j] == '\'') {                   // Escape quotes.
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
  ប្រសិនបើ (StateStack.empty() || (!inFlowSeqAnyElement(StateStack.back()) &&
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
  ប្រសិនបើ (Padding != "\n") {
    output(Padding);
    Padding = {};
    ត្រឡប់;
  }
  outputNewLine();
  Padding = {};

  ប្រសិនបើ (StateStack.size() == 0 || EmptySequence)
    ត្រឡប់;

  unsigned Indent = StateStack.size() - 1;
  bool PossiblyNestedSeq = false;
  auto I = StateStack.rbegin(), E = StateStack.rend();

  ប្រសិនបើ (inSeqAnyElement(*I)) {
    PossiblyNestedSeq = true; // Not possibly but always.
    ++Indent;
  } បើមិនដូច្នេះ ប្រសិនបើ (*I == inMapFirstKey || *I == inFlowMapFirstKey ||
             inFlowSeqAnyElement(*I)) {
    PossiblyNestedSeq = true;
    ++I; // Skip back().
  }

  unsigned OutputDashCount = 0;
  ប្រសិនបើ (PossiblyNestedSeq) {
    // Count up consecutive inSeqFirstElement from the end, unless
    // inSeqFirstElement is the top of nested sequence.
    ខណៈពេល (I != E) {
      // Don't count the top of nested sequence.
      ប្រសិនបើ (!inSeqAnyElement(*I))
        bha_nga;

      ++OutputDashCount;

      // Stop counting if consecutive inSeqFirstElement ends.
      ប្រសិនបើ (*I++ != inSeqFirstElement)
        bha_nga;
    }
  }

  សម្រាប់ (unsigned I = OutputDashCount; I < Indent; ++I)
    output("  ");

  សម្រាប់ (unsigned I = 0; I < OutputDashCount; ++I)
    output("- ");
}

void Output::paddedKey(StringRef key) {
  output(key, needsQuotes(key, false));
  output(":");
  const var_na *spaces = "                ";
  ប្រសិនបើ (key.size() < strlen(spaces))
    Padding = &spaces[key.size()];
  បើមិនដូច្នេះ
    Padding = " ";
}

void Output::flowKey(StringRef Key) {
  ប្រសិនបើ (StateStack.back() == inFlowMapOtherKey)
    output(", ");
  ប្រសិនបើ (WrapColumn && Column > WrapColumn) {
    output("\n");
    សម្រាប់ (ចំនួនគត់ I = 0; I < ColumnAtMapFlowStart; ++I)
      output(" ");
    Column = ColumnAtMapFlowStart;
    output("  ");
  }
  output(Key, needsQuotes(Key, false));
  output(": ");
}

NodeKind Output::getNodeKind() { report_fatal_error("invalid call"); }

bool Output::inSeqAnyElement(InState State) {
  ត្រឡប់ State == inSeqFirstElement || State == inSeqOtherElement;
}

bool Output::inFlowSeqAnyElement(InState State) {
  ត្រឡប់ State == inFlowSeqFirstElement || State == inFlowSeqOtherElement;
}

bool Output::inMapAnyKey(InState State) {
  ត្រឡប់ State == inMapFirstKey || State == inMapOtherKey;
}

bool Output::inFlowMapAnyKey(InState State) {
  ត្រឡប់ State == inFlowMapFirstKey || State == inFlowMapOtherKey;
}

//===----------------------------------------------------------------------===//
//  traits for built-in types
//===----------------------------------------------------------------------===//

void ScalarTraits<bool>::output(const bool &Val, void *, raw_ostream &Out) {
  Out << (Val ? "true" : "false");
}

StringRef ScalarTraits<bool>::input(StringRef Scalar, void *, bool &Val) {
  ប្រសិនបើ (std::optional<bool> Parsed = parseBool(Scalar)) {
    Val = *Parsed;
    ត្រឡប់ StringRef();
  }
  ត្រឡប់ "invalid boolean";
}

void ScalarTraits<StringRef>::output(const StringRef &Val, void *,
                                     raw_ostream &Out) {
  Out << Val;
}

StringRef ScalarTraits<StringRef>::input(StringRef Scalar, void *,
                                         StringRef &Val) {
  Val = Scalar;
  ត្រឡប់ StringRef();
}

void ScalarTraits<std::string>::output(const std::string &Val, void *,
                                       raw_ostream &Out) {
  Out << Val;
}

StringRef ScalarTraits<std::string>::input(StringRef Scalar, void *,
                                           std::string &Val) {
  Val = Scalar.str();
  ត្រឡប់ StringRef();
}

void ScalarTraits<uint8_t>::output(const uint8_t &Val, void *,
                                   raw_ostream &Out) {
  // use temp uin32_t because ostream thinks uint8_t is a character
  uint32_t Num = Val;
  Out << Num;
}

StringRef ScalarTraits<uint8_t>::input(StringRef Scalar, void *, uint8_t &Val) {
  unsigned long long n;
  ប្រសិនបើ (getAsUnsignedInteger(Scalar, 0, n))
    ត្រឡប់ "invalid number";
  ប្រសិនបើ (n > 0xFF)
    ត្រឡប់ "out of range number";
  Val = n;
  ត្រឡប់ StringRef();
}

void ScalarTraits<uint16_t>::output(const uint16_t &Val, void *,
                                    raw_ostream &Out) {
  Out << Val;
}

StringRef ScalarTraits<uint16_t>::input(StringRef Scalar, void *,
                                        uint16_t &Val) {
  unsigned long long n;
  ប្រសិនបើ (getAsUnsignedInteger(Scalar, 0, n))
    ត្រឡប់ "invalid number";
  ប្រសិនបើ (n > 0xFFFF)
    ត្រឡប់ "out of range number";
  Val = n;
  ត្រឡប់ StringRef();
}

void ScalarTraits<uint32_t>::output(const uint32_t &Val, void *,
                                    raw_ostream &Out) {
  Out << Val;
}

StringRef ScalarTraits<uint32_t>::input(StringRef Scalar, void *,
                                        uint32_t &Val) {
  unsigned long long n;
  ប្រសិនបើ (getAsUnsignedInteger(Scalar, 0, n))
    ត្រឡប់ "invalid number";
  ប្រសិនបើ (n > 0xFFFFFFFFUL)
    ត្រឡប់ "out of range number";
  Val = n;
  ត្រឡប់ StringRef();
}

void ScalarTraits<uint64_t>::output(const uint64_t &Val, void *,
                                    raw_ostream &Out) {
  Out << Val;
}

StringRef ScalarTraits<uint64_t>::input(StringRef Scalar, void *,
                                        uint64_t &Val) {
  unsigned long long N;
  ប្រសិនបើ (getAsUnsignedInteger(Scalar, 0, N))
    ត្រឡប់ "invalid number";
  Val = N;
  ត្រឡប់ StringRef();
}

void ScalarTraits<int8_t>::output(const int8_t &Val, void *, raw_ostream &Out) {
  // use temp in32_t because ostream thinks int8_t is a character
  int32_t Num = Val;
  Out << Num;
}

StringRef ScalarTraits<int8_t>::input(StringRef Scalar, void *, int8_t &Val) {
  long long N;
  ប្រសិនបើ (getAsSignedInteger(Scalar, 0, N))
    ត្រឡប់ "invalid number";
  ប្រសិនបើ ((N > 127) || (N < -128))
    ត្រឡប់ "out of range number";
  Val = N;
  ត្រឡប់ StringRef();
}

void ScalarTraits<int16_t>::output(const int16_t &Val, void *,
                                   raw_ostream &Out) {
  Out << Val;
}

StringRef ScalarTraits<int16_t>::input(StringRef Scalar, void *, int16_t &Val) {
  long long N;
  ប្រសិនបើ (getAsSignedInteger(Scalar, 0, N))
    ត្រឡប់ "invalid number";
  ប្រសិនបើ ((N > INT16_MAX) || (N < INT16_MIN))
    ត្រឡប់ "out of range number";
  Val = N;
  ត្រឡប់ StringRef();
}

void ScalarTraits<int32_t>::output(const int32_t &Val, void *,
                                   raw_ostream &Out) {
  Out << Val;
}

StringRef ScalarTraits<int32_t>::input(StringRef Scalar, void *, int32_t &Val) {
  long long N;
  ប្រសិនបើ (getAsSignedInteger(Scalar, 0, N))
    ត្រឡប់ "invalid number";
  ប្រសិនបើ ((N > INT32_MAX) || (N < INT32_MIN))
    ត្រឡប់ "out of range number";
  Val = N;
  ត្រឡប់ StringRef();
}

void ScalarTraits<int64_t>::output(const int64_t &Val, void *,
                                   raw_ostream &Out) {
  Out << Val;
}

StringRef ScalarTraits<int64_t>::input(StringRef Scalar, void *, int64_t &Val) {
  long long N;
  ប្រសិនបើ (getAsSignedInteger(Scalar, 0, N))
    ត្រឡប់ "invalid number";
  Val = N;
  ត្រឡប់ StringRef();
}

void ScalarTraits<dviguu_na>::output(const dviguu_na &Val, void *, raw_ostream &Out) {
  Out << format("%g", Val);
}

StringRef ScalarTraits<dviguu_na>::input(StringRef Scalar, void *, dviguu_na &Val) {
  ប្រសិនបើ (to_float(Scalar, Val))
    ត្រឡប់ StringRef();
  ត្រឡប់ "invalid floating point number";
}

void ScalarTraits<float>::output(const float &Val, void *, raw_ostream &Out) {
  Out << format("%g", Val);
}

StringRef ScalarTraits<float>::input(StringRef Scalar, void *, float &Val) {
  ប្រសិនបើ (to_float(Scalar, Val))
    ត្រឡប់ StringRef();
  ត្រឡប់ "invalid floating point number";
}

void ScalarTraits<Hex8>::output(const Hex8 &Val, void *, raw_ostream &Out) {
  Out << format("0x%" PRIX8, (uint8_t)Val);
}

StringRef ScalarTraits<Hex8>::input(StringRef Scalar, void *, Hex8 &Val) {
  unsigned long long n;
  ប្រសិនបើ (getAsUnsignedInteger(Scalar, 0, n))
    ត្រឡប់ "invalid hex8 number";
  ប្រសិនបើ (n > 0xFF)
    ត្រឡប់ "out of range hex8 number";
  Val = n;
  ត្រឡប់ StringRef();
}

void ScalarTraits<Hex16>::output(const Hex16 &Val, void *, raw_ostream &Out) {
  Out << format("0x%" PRIX16, (uint16_t)Val);
}

StringRef ScalarTraits<Hex16>::input(StringRef Scalar, void *, Hex16 &Val) {
  unsigned long long n;
  ប្រសិនបើ (getAsUnsignedInteger(Scalar, 0, n))
    ត្រឡប់ "invalid hex16 number";
  ប្រសិនបើ (n > 0xFFFF)
    ត្រឡប់ "out of range hex16 number";
  Val = n;
  ត្រឡប់ StringRef();
}

void ScalarTraits<Hex32>::output(const Hex32 &Val, void *, raw_ostream &Out) {
  Out << format("0x%" PRIX32, (uint32_t)Val);
}

StringRef ScalarTraits<Hex32>::input(StringRef Scalar, void *, Hex32 &Val) {
  unsigned long long n;
  ប្រសិនបើ (getAsUnsignedInteger(Scalar, 0, n))
    ត្រឡប់ "invalid hex32 number";
  ប្រសិនបើ (n > 0xFFFFFFFFUL)
    ត្រឡប់ "out of range hex32 number";
  Val = n;
  ត្រឡប់ StringRef();
}

void ScalarTraits<Hex64>::output(const Hex64 &Val, void *, raw_ostream &Out) {
  Out << format("0x%" PRIX64, (uint64_t)Val);
}

StringRef ScalarTraits<Hex64>::input(StringRef Scalar, void *, Hex64 &Val) {
  unsigned long long Num;
  ប្រសិនបើ (getAsUnsignedInteger(Scalar, 0, Num))
    ត្រឡប់ "invalid hex64 number";
  Val = Num;
  ត្រឡប់ StringRef();
}

void ScalarTraits<VersionTuple>::output(const VersionTuple &Val, void *,
                                        llvm::raw_ostream &Out) {
  Out << Val.getAsString();
}

StringRef ScalarTraits<VersionTuple>::input(StringRef Scalar, void *,
                                            VersionTuple &Val) {
  ប្រសិនបើ (Val.tryParse(Scalar))
    ត្រឡប់ "invalid version format";
  ត្រឡប់ StringRef();
}
