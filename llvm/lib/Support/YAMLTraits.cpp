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
  ပြန်ပေး Ctxt;
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
  အကယ်၍ (DiagHandler)
    SrcMgr.setDiagHandler(DiagHandler, DiagHandlerCtxt);
  DocIterator = Strm->begin();
}

Input::Input(MemoryBufferRef Input, void *Ctxt,
             SourceMgr::DiagHandlerTy DiagHandler, void *DiagHandlerCtxt)
    : IO(Ctxt), Strm(new Stream(Input, SrcMgr, false, &EC)) {
  အကယ်၍ (DiagHandler)
    SrcMgr.setDiagHandler(DiagHandler, DiagHandlerCtxt);
  DocIterator = Strm->begin();
}

Input::~Input() = default;

std::error_code Input::error() { ပြန်ပေး EC; }

bool Input::outputting() const {
  ပြန်ပေး false;
}

bool Input::setCurrentDocument() {
  အကယ်၍ (DocIterator != Strm->end()) {
    Node *N = DocIterator->getRoot();
    အကယ်၍ (!N) {
      EC = make_error_code(errc::invalid_argument);
      ပြန်ပေး false;
    }

    အကယ်၍ (isa<NullNode>(N)) {
      // Empty files are allowed and ignored
      ++DocIterator;
      ပြန်ပေး setCurrentDocument();
    }
    releaseHNodeBuffers();
    TopNode = createHNodes(N);
    CurrentNode = TopNode;
    ပြန်ပေး true;
  }
  ပြန်ပေး false;
}

bool Input::nextDocument() {
  ပြန်ပေး ++DocIterator != Strm->end();
}

const Node *Input::getCurrentNode() const {
  ပြန်ပေး CurrentNode ? CurrentNode->_node : nullptr;
}

bool Input::mapTag(StringRef Tag, bool Default) {
  // CurrentNode can be null if setCurrentDocument() was unable to
  // parse the document because it was invalid or empty.
  အကယ်၍ (!CurrentNode)
    ပြန်ပေး false;

  std::string foundTag = CurrentNode->_node->getVerbatimTag();
  အကယ်၍ (foundTag.empty()) {
    // If no tag found and 'Tag' is the default, say it was found.
    ပြန်ပေး Default;
  }
  // Return true iff found tag matches supplied tag.
  ပြန်ပေး Tag == foundTag;
}

void Input::beginMapping() {
  အကယ်၍ (EC)
    ပြန်ပေး;
  // CurrentNode can be null if the document is empty.
  MapHNode *MN = dyn_cast_or_null<MapHNode>(CurrentNode);
  အကယ်၍ (MN) {
    MN->ValidKeys.clear();
  }
}

std::vector<StringRef> Input::keys() {
  MapHNode *MN = dyn_cast<MapHNode>(CurrentNode);
  std::vector<StringRef> Ret;
  အကယ်၍ (!MN) {
    setError(CurrentNode, "not a mapping");
    ပြန်ပေး Ret;
  }
  အတွက် (auto &P : MN->Mapping)
    Ret.push_back(P.first());
  ပြန်ပေး Ret;
}

bool Input::preflightKey(StringRef Key, bool Required, bool, bool &UseDefault,
                         void *&SaveInfo) {
  UseDefault = false;
  အကယ်၍ (EC)
    ပြန်ပေး false;

  // CurrentNode is null for empty documents, which is an error in case required
  // nodes are present.
  အကယ်၍ (!CurrentNode) {
    အကယ်၍ (Required)
      EC = make_error_code(errc::invalid_argument);
    မဟုတ်ပါက
      UseDefault = true;
    ပြန်ပေး false;
  }

  MapHNode *MN = dyn_cast<MapHNode>(CurrentNode);
  အကယ်၍ (!MN) {
    အကယ်၍ (Required || !isa<EmptyHNode>(CurrentNode))
      setError(CurrentNode, "not a mapping");
    မဟုတ်ပါက
      UseDefault = true;
    ပြန်ပေး false;
  }
  MN->ValidKeys.push_back(Key.str());
  HNode *Value = MN->Mapping[Key].first;
  အကယ်၍ (!Value) {
    အကယ်၍ (Required)
      setError(CurrentNode, Twine("missing required key '") + Key + "'");
    မဟုတ်ပါက
      UseDefault = true;
    ပြန်ပေး false;
  }
  SaveInfo = CurrentNode;
  CurrentNode = Value;
  ပြန်ပေး true;
}

void Input::postflightKey(void *saveInfo) {
  CurrentNode = reinterpret_cast<HNode *>(saveInfo);
}

void Input::endMapping() {
  အကယ်၍ (EC)
    ပြန်ပေး;
  // CurrentNode can be null if the document is empty.
  MapHNode *MN = dyn_cast_or_null<MapHNode>(CurrentNode);
  အကယ်၍ (!MN)
    ပြန်ပေး;
  အတွက် (const auto &NN : MN->Mapping) {
    အကယ်၍ (!is_contained(MN->ValidKeys, NN.first())) {
      const SMRange &ReportLoc = NN.second.second;
      အကယ်၍ (!AllowUnknownKeys) {
        setError(ReportLoc, Twine("unknown key '") + NN.first() + "'");
        bha_nga;
      } မဟုတ်ပါက
        reportWarning(ReportLoc, Twine("unknown key '") + NN.first() + "'");
    }
  }
}

void Input::beginFlowMapping() { beginMapping(); }

void Input::endFlowMapping() { endMapping(); }

unsigned Input::beginSequence() {
  အကယ်၍ (SequenceHNode *SQ = dyn_cast<SequenceHNode>(CurrentNode))
    ပြန်ပေး SQ->Entries.size();
  အကယ်၍ (isa<EmptyHNode>(CurrentNode))
    ပြန်ပေး 0;
  // Treat case where there's a scalar "null" value as an empty sequence.
  အကယ်၍ (ScalarHNode *SN = dyn_cast<ScalarHNode>(CurrentNode)) {
    အကယ်၍ (isNull(SN->value()))
      ပြန်ပေး 0;
  }
  // Any other type of HNode is an error.
  setError(CurrentNode, "not a sequence");
  ပြန်ပေး 0;
}

void Input::endSequence() {
}

bool Input::preflightElement(unsigned Index, void *&SaveInfo) {
  အကယ်၍ (EC)
    ပြန်ပေး false;
  အကယ်၍ (SequenceHNode *SQ = dyn_cast<SequenceHNode>(CurrentNode)) {
    SaveInfo = CurrentNode;
    CurrentNode = SQ->Entries[Index];
    ပြန်ပေး true;
  }
  ပြန်ပေး false;
}

void Input::postflightElement(void *SaveInfo) {
  CurrentNode = reinterpret_cast<HNode *>(SaveInfo);
}

unsigned Input::beginFlowSequence() { ပြန်ပေး beginSequence(); }

bool Input::preflightFlowElement(unsigned index, void *&SaveInfo) {
  အကယ်၍ (EC)
    ပြန်ပေး false;
  အကယ်၍ (SequenceHNode *SQ = dyn_cast<SequenceHNode>(CurrentNode)) {
    SaveInfo = CurrentNode;
    CurrentNode = SQ->Entries[index];
    ပြန်ပေး true;
  }
  ပြန်ပေး false;
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
  အကယ်၍ (ScalarMatchFound)
    ပြန်ပေး false;
  အကယ်၍ (ScalarHNode *SN = dyn_cast<ScalarHNode>(CurrentNode)) {
    အကယ်၍ (SN->value() == Str) {
      ScalarMatchFound = true;
      ပြန်ပေး true;
    }
  }
  ပြန်ပေး false;
}

bool Input::matchEnumFallback() {
  အကယ်၍ (ScalarMatchFound)
    ပြန်ပေး false;
  ScalarMatchFound = true;
  ပြန်ပေး true;
}

void Input::endEnumScalar() {
  အကယ်၍ (!ScalarMatchFound) {
    setError(CurrentNode, "unknown enumerated scalar");
  }
}

bool Input::beginBitSetScalar(bool &DoClear) {
  BitValuesUsed.clear();
  အကယ်၍ (SequenceHNode *SQ = dyn_cast<SequenceHNode>(CurrentNode)) {
    BitValuesUsed.resize(SQ->Entries.size());
  } မဟုတ်ပါက {
    setError(CurrentNode, "expected sequence of bit values");
  }
  DoClear = true;
  ပြန်ပေး true;
}

bool Input::bitSetMatch(StringRef Str, bool) {
  အကယ်၍ (EC)
    ပြန်ပေး false;
  အကယ်၍ (SequenceHNode *SQ = dyn_cast<SequenceHNode>(CurrentNode)) {
    unsigned Index = 0;
    အတွက် (auto &N : SQ->Entries) {
      အကယ်၍ (ScalarHNode *SN = dyn_cast<ScalarHNode>(N)) {
        အကယ်၍ (SN->value() == Str) {
          BitValuesUsed[Index] = true;
          ပြန်ပေး true;
        }
      } မဟုတ်ပါက {
        setError(CurrentNode, "unexpected scalar in sequence of bit values");
      }
      ++Index;
    }
  } မဟုတ်ပါက {
    setError(CurrentNode, "expected sequence of bit values");
  }
  ပြန်ပေး false;
}

void Input::endBitSetScalar() {
  အကယ်၍ (EC)
    ပြန်ပေး;
  အကယ်၍ (SequenceHNode *SQ = dyn_cast<SequenceHNode>(CurrentNode)) {
    assert(BitValuesUsed.size() == SQ->Entries.size());
    အတွက် (unsigned i = 0; i < SQ->Entries.size(); ++i) {
      အကယ်၍ (!BitValuesUsed[i]) {
        setError(SQ->Entries[i], "unknown bit value");
        ပြန်ပေး;
      }
    }
  }
}

void Input::scalarString(StringRef &S, QuotingType) {
  အကယ်၍ (ScalarHNode *SN = dyn_cast<ScalarHNode>(CurrentNode)) {
    S = SN->value();
  } မဟုတ်ပါက {
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
  အကယ်၍ (isa<ScalarHNode>(CurrentNode))
    ပြန်ပေး NodeKind::Scalar;
  မဟုတ်ပါက အကယ်၍ (isa<MapHNode>(CurrentNode))
    ပြန်ပေး NodeKind::Map;
  မဟုတ်ပါက အကယ်၍ (isa<SequenceHNode>(CurrentNode))
    ပြန်ပေး NodeKind::Sequence;
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
  အကယ်၍ (!Anchor.empty())
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
    အကယ်၍ (!StringStorage.empty()) {
      // Copy string to permanent storage
      KeyStr = StringStorage.str().copy(StringAllocator);
    }
    auto *SHNode = new (ScalarHNodeAllocator.Allocate()) ScalarHNode(N, KeyStr);
    saveAliasHNode(SN, SHNode);
    ပြန်ပေး SHNode;
  }
  pak_sa Node::NK_BlockScalar: {
    BlockScalarNode *BSN = dyn_cast<BlockScalarNode>(N);
    StringRef ValueCopy = BSN->getValue().copy(StringAllocator);
    auto *BSHNode =
        new (ScalarHNodeAllocator.Allocate()) ScalarHNode(N, ValueCopy);
    saveAliasHNode(BSN, BSHNode);
    ပြန်ပေး BSHNode;
  }
  pak_sa Node::NK_Sequence: {
    SequenceNode *SQ = dyn_cast<SequenceNode>(N);
    auto SQHNode = new (SequenceHNodeAllocator.Allocate()) SequenceHNode(N);
    အတွက် (Node &SN : *SQ) {
      auto Entry = createHNodes(&SN);
      အကယ်၍ (EC)
        bha_nga;
      SQHNode->Entries.push_back(Entry);
    }
    saveAliasHNode(SQ, SQHNode);
    ပြန်ပေး SQHNode;
  }
  pak_sa Node::NK_Mapping: {
    MappingNode *Map = dyn_cast<MappingNode>(N);
    auto mapHNode = new (MapHNodeAllocator.Allocate()) MapHNode(N);
    အတွက် (KeyValueNode &KVN : *Map) {
      Node *KeyNode = KVN.getKey();
      ScalarNode *Key = dyn_cast_if_present<ScalarNode>(KeyNode);
      Node *Value = KVN.getValue();
      အကယ်၍ (!Key || !Value) {
        အကယ်၍ (!Key)
          setError(KeyNode, "Map key must be a scalar");
        အကယ်၍ (!Value)
          setError(KeyNode, "Map value must not be empty");
        bha_nga;
      }
      StringStorage.clear();
      StringRef KeyStr = Key->getValue(StringStorage);
      အကယ်၍ (!StringStorage.empty()) {
        // Copy string to permanent storage
        KeyStr = StringStorage.str().copy(StringAllocator);
      }
      အကယ်၍ (mapHNode->Mapping.count(KeyStr))
        // From YAML spec: "The content of a mapping node is an unordered set of
        // key/value node pairs, with the restriction that each of the keys is
        // unique."
        setError(KeyNode, Twine("duplicated mapping key '") + KeyStr + "'");
      auto ValueHNode = createHNodes(Value);
      အကယ်၍ (EC)
        bha_nga;
      mapHNode->Mapping[KeyStr] =
          std::make_pair(std::move(ValueHNode), KeyNode->getSourceRange());
    }
    saveAliasHNode(Map, mapHNode);
    ပြန်ပေး std::move(mapHNode);
  }
  pak_sa Node::NK_Null:
    // TODO: Anchor is not set for NullNode in the parser. Update the parser to
    // faithfully preserve anchors.
    ပြန်ပေး new (EmptyHNodeAllocator.Allocate()) EmptyHNode(N);
  pak_sa Node::NK_Alias: {
    AliasNode *AN = dyn_cast<AliasNode>(N);
    auto AliasName = AN->getName();
    auto AHN = AliasMap.find(AliasName);
    အကယ်၍ (AHN == AliasMap.end()) {
      setError(AN, Twine("undefined alias '" + AliasName + "'"));
      ပြန်ပေး nullptr;
    }
    ပြန်ပေး AHN->second;
  }
  default:
    setError(N, "unknown node kind");
    ပြန်ပေး nullptr;
  }
}

void Input::setError(const Twine &Message) {
  setError(CurrentNode, Message);
}

void Input::setAllowUnknownKeys(bool Allow) { AllowUnknownKeys = Allow; }

bool Input::canElideEmptySequence() {
  ပြန်ပေး false;
}

//===----------------------------------------------------------------------===//
//  Output
//===----------------------------------------------------------------------===//

Output::Output(raw_ostream &yout, void *context, ကိန်းပြည့် WrapColumn)
    : IO(context), Out(yout), WrapColumn(WrapColumn) {}

Output::~Output() = default;

bool Output::outputting() const {
  ပြန်ပေး true;
}

void Output::beginMapping() {
  StateStack.push_back(inMapFirstKey);
  PaddingBeforeContainer = Padding;
  Padding = "\n";
}

bool Output::mapTag(StringRef Tag, bool Use) {
  အကယ်၍ (Use) {
    // If this tag is being written inside a sequence we should write the start
    // of the sequence before writing the tag, otherwise the tag won't be
    // attached to the element in the sequence, but rather the sequence itself.
    bool SequenceElement = false;
    အကယ်၍ (StateStack.size() > 1) {
      auto &E = StateStack[StateStack.size() - 2];
      SequenceElement = inSeqAnyElement(E) || inFlowSeqAnyElement(E);
    }
    အကယ်၍ (SequenceElement && StateStack.back() == inMapFirstKey) {
      newLineCheck();
    } မဟုတ်ပါက {
      output(" ");
    }
    output(Tag);
    အကယ်၍ (SequenceElement) {
      // If we're writing the tag during the first element of a map, the tag
      // takes the place of the first element in the sequence.
      အကယ်၍ (StateStack.back() == inMapFirstKey) {
        StateStack.pop_back();
        StateStack.push_back(inMapOtherKey);
      }
      // Tags inside maps in sequences should act as keys in the map from a
      // formatting perspective, so we always want a newline in a sequence.
      Padding = "\n";
    }
  }
  ပြန်ပေး Use;
}

void Output::endMapping() {
  // If we did not map anything, we should explicitly emit an empty map
  အကယ်၍ (StateStack.back() == inMapFirstKey) {
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
  အကယ်၍ (Required || !SameAsDefault || WriteDefaultValues) {
    auto State = StateStack.back();
    အကယ်၍ (State == inFlowMapFirstKey || State == inFlowMapOtherKey) {
      flowKey(Key);
    } မဟုတ်ပါက {
      newLineCheck();
      paddedKey(Key);
    }
    ပြန်ပေး true;
  }
  ပြန်ပေး false;
}

void Output::postflightKey(void *) {
  အကယ်၍ (StateStack.back() == inMapFirstKey) {
    StateStack.pop_back();
    StateStack.push_back(inMapOtherKey);
  } မဟုတ်ပါက အကယ်၍ (StateStack.back() == inFlowMapFirstKey) {
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
  အကယ်၍ (index > 0)
    outputUpToEndOfLine("\n---");
  ပြန်ပေး true;
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
  ပြန်ပေး 0;
}

void Output::endSequence() {
  // If we did not emit anything, we should explicitly emit an empty sequence
  အကယ်၍ (StateStack.back() == inSeqFirstElement) {
    Padding = PaddingBeforeContainer;
    newLineCheck(/*EmptySequence=*/true);
    output("[]");
    Padding = "\n";
  }
  StateStack.pop_back();
}

bool Output::preflightElement(unsigned, void *&SaveInfo) {
  SaveInfo = nullptr;
  ပြန်ပေး true;
}

void Output::postflightElement(void *) {
  အကယ်၍ (StateStack.back() == inSeqFirstElement) {
    StateStack.pop_back();
    StateStack.push_back(inSeqOtherElement);
  } မဟုတ်ပါက အကယ်၍ (StateStack.back() == inFlowSeqFirstElement) {
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
  ပြန်ပေး 0;
}

void Output::endFlowSequence() {
  StateStack.pop_back();
  outputUpToEndOfLine(" ]");
}

bool Output::preflightFlowElement(unsigned, void *&SaveInfo) {
  အကယ်၍ (NeedFlowSequenceComma)
    output(", ");
  အကယ်၍ (WrapColumn && Column > WrapColumn) {
    output("\n");
    အတွက် (ကိန်းပြည့် i = 0; i < ColumnAtFlowStart; ++i)
      output(" ");
    Column = ColumnAtFlowStart;
    output("  ");
  }
  SaveInfo = nullptr;
  ပြန်ပေး true;
}

void Output::postflightFlowElement(void *) {
  NeedFlowSequenceComma = true;
}

void Output::beginEnumScalar() {
  EnumerationMatchFound = false;
}

bool Output::matchEnumScalar(StringRef Str, bool Match) {
  အကယ်၍ (Match && !EnumerationMatchFound) {
    newLineCheck();
    outputUpToEndOfLine(Str);
    EnumerationMatchFound = true;
  }
  ပြန်ပေး false;
}

bool Output::matchEnumFallback() {
  အကယ်၍ (EnumerationMatchFound)
    ပြန်ပေး false;
  EnumerationMatchFound = true;
  ပြန်ပေး true;
}

void Output::endEnumScalar() {
  အကယ်၍ (!EnumerationMatchFound)
    llvm_unreachable("bad runtime enum value");
}

bool Output::beginBitSetScalar(bool &DoClear) {
  newLineCheck();
  output("[ ");
  NeedBitValueComma = false;
  DoClear = false;
  ပြန်ပေး true;
}

bool Output::bitSetMatch(StringRef Str, bool Matches) {
  အကယ်၍ (Matches) {
    အကယ်၍ (NeedBitValueComma)
      output(", ");
    output(Str);
    NeedBitValueComma = true;
  }
  ပြန်ပေး false;
}

void Output::endBitSetScalar() {
  outputUpToEndOfLine(" ]");
}

void Output::scalarString(StringRef &S, QuotingType MustQuote) {
  newLineCheck();
  အကယ်၍ (S.empty()) {
    // Print '' for the empty string because leaving the field empty is not
    // allowed.
    outputUpToEndOfLine("''");
    ပြန်ပေး;
  }
  output(S, MustQuote);
  outputUpToEndOfLine("");
}

void Output::blockScalarString(StringRef &S) {
  အကယ်၍ (!StateStack.empty())
    newLineCheck();
  output(" |");

  unsigned Indent = StateStack.empty() ? 1 : StateStack.size();

  auto Buffer = MemoryBuffer::getMemBuffer(S, "", false);
  အတွက် (line_iterator Lines(*Buffer, false); !Lines.is_at_end(); ++Lines) {
    outputNewLine();
    အတွက် (unsigned I = 0; I < Indent; ++I) {
      output("  ");
    }
    output(*Lines);
  }
  outputUpToEndOfLine("");
}

void Output::scalarTag(std::string &Tag) {
  အကယ်၍ (Tag.empty())
    ပြန်ပေး;
  newLineCheck();
  output(Tag);
  output(" ");
}

void Output::setError(const Twine &message) {
}

std::error_code Output::error() { ပြန်ပေး {}; }

bool Output::canElideEmptySequence() {
  // Normally, with an optional key/value where the value is an empty sequence,
  // the whole key/value can be not written.  But, that produces wrong yaml
  // if the key/value is the only thing in the map and the map is used in
  // a sequence.  This detects if the this sequence is the first key/value
  // in map that itself is embedded in a sequence.
  အကယ်၍ (StateStack.size() < 2)
    ပြန်ပေး true;
  အကယ်၍ (StateStack.back() != inMapFirstKey)
    ပြန်ပေး true;
  ပြန်ပေး !inSeqAnyElement(StateStack[StateStack.size() - 2]);
}

void Output::output(StringRef s) {
  Column += s.size();
  Out << s;
}

void Output::output(StringRef S, QuotingType MustQuote) {
  အကယ်၍ (MustQuote == QuotingType::None) {
    // Only quote if we must.
    output(S);
    ပြန်ပေး;
  }

  StringLiteral Quote = MustQuote == QuotingType::Single ? StringLiteral("'")
                                                         : StringLiteral("\"");
  output(Quote); // Starting quote.

  // When using double-quoted strings (and only in that case), non-printable
  // characters may be present, and will be escaped using a variety of
  // unicode-scalar and special short-form escapes. This is handled in
  // yaml::escape.
  အကယ်၍ (MustQuote == QuotingType::Double) {
    output(yaml::escape(S, /* EscapePrintable= */ false));
    output(Quote);
    ပြန်ပေး;
  }

  unsigned i = 0;
  unsigned j = 0;
  unsigned End = S.size();
  const var_na *Base = S.data();

  // When using single-quoted strings, any single quote ' must be doubled to be
  // escaped.
  နေစဉ် (j < End) {
    အကယ်၍ (S[j] == '\'') {                   // Escape quotes.
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
  အကယ်၍ (StateStack.empty() || (!inFlowSeqAnyElement(StateStack.back()) &&
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
  အကယ်၍ (Padding != "\n") {
    output(Padding);
    Padding = {};
    ပြန်ပေး;
  }
  outputNewLine();
  Padding = {};

  အကယ်၍ (StateStack.size() == 0 || EmptySequence)
    ပြန်ပေး;

  unsigned Indent = StateStack.size() - 1;
  bool PossiblyNestedSeq = false;
  auto I = StateStack.rbegin(), E = StateStack.rend();

  အကယ်၍ (inSeqAnyElement(*I)) {
    PossiblyNestedSeq = true; // Not possibly but always.
    ++Indent;
  } မဟုတ်ပါက အကယ်၍ (*I == inMapFirstKey || *I == inFlowMapFirstKey ||
             inFlowSeqAnyElement(*I)) {
    PossiblyNestedSeq = true;
    ++I; // Skip back().
  }

  unsigned OutputDashCount = 0;
  အကယ်၍ (PossiblyNestedSeq) {
    // Count up consecutive inSeqFirstElement from the end, unless
    // inSeqFirstElement is the top of nested sequence.
    နေစဉ် (I != E) {
      // Don't count the top of nested sequence.
      အကယ်၍ (!inSeqAnyElement(*I))
        bha_nga;

      ++OutputDashCount;

      // Stop counting if consecutive inSeqFirstElement ends.
      အကယ်၍ (*I++ != inSeqFirstElement)
        bha_nga;
    }
  }

  အတွက် (unsigned I = OutputDashCount; I < Indent; ++I)
    output("  ");

  အတွက် (unsigned I = 0; I < OutputDashCount; ++I)
    output("- ");
}

void Output::paddedKey(StringRef key) {
  output(key, needsQuotes(key, false));
  output(":");
  const var_na *spaces = "                ";
  အကယ်၍ (key.size() < strlen(spaces))
    Padding = &spaces[key.size()];
  မဟုတ်ပါက
    Padding = " ";
}

void Output::flowKey(StringRef Key) {
  အကယ်၍ (StateStack.back() == inFlowMapOtherKey)
    output(", ");
  အကယ်၍ (WrapColumn && Column > WrapColumn) {
    output("\n");
    အတွက် (ကိန်းပြည့် I = 0; I < ColumnAtMapFlowStart; ++I)
      output(" ");
    Column = ColumnAtMapFlowStart;
    output("  ");
  }
  output(Key, needsQuotes(Key, false));
  output(": ");
}

NodeKind Output::getNodeKind() { report_fatal_error("invalid call"); }

bool Output::inSeqAnyElement(InState State) {
  ပြန်ပေး State == inSeqFirstElement || State == inSeqOtherElement;
}

bool Output::inFlowSeqAnyElement(InState State) {
  ပြန်ပေး State == inFlowSeqFirstElement || State == inFlowSeqOtherElement;
}

bool Output::inMapAnyKey(InState State) {
  ပြန်ပေး State == inMapFirstKey || State == inMapOtherKey;
}

bool Output::inFlowMapAnyKey(InState State) {
  ပြန်ပေး State == inFlowMapFirstKey || State == inFlowMapOtherKey;
}

//===----------------------------------------------------------------------===//
//  traits for built-in types
//===----------------------------------------------------------------------===//

void ScalarTraits<bool>::output(const bool &Val, void *, raw_ostream &Out) {
  Out << (Val ? "true" : "false");
}

StringRef ScalarTraits<bool>::input(StringRef Scalar, void *, bool &Val) {
  အကယ်၍ (std::optional<bool> Parsed = parseBool(Scalar)) {
    Val = *Parsed;
    ပြန်ပေး StringRef();
  }
  ပြန်ပေး "invalid boolean";
}

void ScalarTraits<StringRef>::output(const StringRef &Val, void *,
                                     raw_ostream &Out) {
  Out << Val;
}

StringRef ScalarTraits<StringRef>::input(StringRef Scalar, void *,
                                         StringRef &Val) {
  Val = Scalar;
  ပြန်ပေး StringRef();
}

void ScalarTraits<std::string>::output(const std::string &Val, void *,
                                       raw_ostream &Out) {
  Out << Val;
}

StringRef ScalarTraits<std::string>::input(StringRef Scalar, void *,
                                           std::string &Val) {
  Val = Scalar.str();
  ပြန်ပေး StringRef();
}

void ScalarTraits<uint8_t>::output(const uint8_t &Val, void *,
                                   raw_ostream &Out) {
  // use temp uin32_t because ostream thinks uint8_t is a character
  uint32_t Num = Val;
  Out << Num;
}

StringRef ScalarTraits<uint8_t>::input(StringRef Scalar, void *, uint8_t &Val) {
  unsigned long long n;
  အကယ်၍ (getAsUnsignedInteger(Scalar, 0, n))
    ပြန်ပေး "invalid number";
  အကယ်၍ (n > 0xFF)
    ပြန်ပေး "out of range number";
  Val = n;
  ပြန်ပေး StringRef();
}

void ScalarTraits<uint16_t>::output(const uint16_t &Val, void *,
                                    raw_ostream &Out) {
  Out << Val;
}

StringRef ScalarTraits<uint16_t>::input(StringRef Scalar, void *,
                                        uint16_t &Val) {
  unsigned long long n;
  အကယ်၍ (getAsUnsignedInteger(Scalar, 0, n))
    ပြန်ပေး "invalid number";
  အကယ်၍ (n > 0xFFFF)
    ပြန်ပေး "out of range number";
  Val = n;
  ပြန်ပေး StringRef();
}

void ScalarTraits<uint32_t>::output(const uint32_t &Val, void *,
                                    raw_ostream &Out) {
  Out << Val;
}

StringRef ScalarTraits<uint32_t>::input(StringRef Scalar, void *,
                                        uint32_t &Val) {
  unsigned long long n;
  အကယ်၍ (getAsUnsignedInteger(Scalar, 0, n))
    ပြန်ပေး "invalid number";
  အကယ်၍ (n > 0xFFFFFFFFUL)
    ပြန်ပေး "out of range number";
  Val = n;
  ပြန်ပေး StringRef();
}

void ScalarTraits<uint64_t>::output(const uint64_t &Val, void *,
                                    raw_ostream &Out) {
  Out << Val;
}

StringRef ScalarTraits<uint64_t>::input(StringRef Scalar, void *,
                                        uint64_t &Val) {
  unsigned long long N;
  အကယ်၍ (getAsUnsignedInteger(Scalar, 0, N))
    ပြန်ပေး "invalid number";
  Val = N;
  ပြန်ပေး StringRef();
}

void ScalarTraits<int8_t>::output(const int8_t &Val, void *, raw_ostream &Out) {
  // use temp in32_t because ostream thinks int8_t is a character
  int32_t Num = Val;
  Out << Num;
}

StringRef ScalarTraits<int8_t>::input(StringRef Scalar, void *, int8_t &Val) {
  long long N;
  အကယ်၍ (getAsSignedInteger(Scalar, 0, N))
    ပြန်ပေး "invalid number";
  အကယ်၍ ((N > 127) || (N < -128))
    ပြန်ပေး "out of range number";
  Val = N;
  ပြန်ပေး StringRef();
}

void ScalarTraits<int16_t>::output(const int16_t &Val, void *,
                                   raw_ostream &Out) {
  Out << Val;
}

StringRef ScalarTraits<int16_t>::input(StringRef Scalar, void *, int16_t &Val) {
  long long N;
  အကယ်၍ (getAsSignedInteger(Scalar, 0, N))
    ပြန်ပေး "invalid number";
  အကယ်၍ ((N > INT16_MAX) || (N < INT16_MIN))
    ပြန်ပေး "out of range number";
  Val = N;
  ပြန်ပေး StringRef();
}

void ScalarTraits<int32_t>::output(const int32_t &Val, void *,
                                   raw_ostream &Out) {
  Out << Val;
}

StringRef ScalarTraits<int32_t>::input(StringRef Scalar, void *, int32_t &Val) {
  long long N;
  အကယ်၍ (getAsSignedInteger(Scalar, 0, N))
    ပြန်ပေး "invalid number";
  အကယ်၍ ((N > INT32_MAX) || (N < INT32_MIN))
    ပြန်ပေး "out of range number";
  Val = N;
  ပြန်ပေး StringRef();
}

void ScalarTraits<int64_t>::output(const int64_t &Val, void *,
                                   raw_ostream &Out) {
  Out << Val;
}

StringRef ScalarTraits<int64_t>::input(StringRef Scalar, void *, int64_t &Val) {
  long long N;
  အကယ်၍ (getAsSignedInteger(Scalar, 0, N))
    ပြန်ပေး "invalid number";
  Val = N;
  ပြန်ပေး StringRef();
}

void ScalarTraits<dviguu_na>::output(const dviguu_na &Val, void *, raw_ostream &Out) {
  Out << format("%g", Val);
}

StringRef ScalarTraits<dviguu_na>::input(StringRef Scalar, void *, dviguu_na &Val) {
  အကယ်၍ (to_float(Scalar, Val))
    ပြန်ပေး StringRef();
  ပြန်ပေး "invalid floating point number";
}

void ScalarTraits<float>::output(const float &Val, void *, raw_ostream &Out) {
  Out << format("%g", Val);
}

StringRef ScalarTraits<float>::input(StringRef Scalar, void *, float &Val) {
  အကယ်၍ (to_float(Scalar, Val))
    ပြန်ပေး StringRef();
  ပြန်ပေး "invalid floating point number";
}

void ScalarTraits<Hex8>::output(const Hex8 &Val, void *, raw_ostream &Out) {
  Out << format("0x%" PRIX8, (uint8_t)Val);
}

StringRef ScalarTraits<Hex8>::input(StringRef Scalar, void *, Hex8 &Val) {
  unsigned long long n;
  အကယ်၍ (getAsUnsignedInteger(Scalar, 0, n))
    ပြန်ပေး "invalid hex8 number";
  အကယ်၍ (n > 0xFF)
    ပြန်ပေး "out of range hex8 number";
  Val = n;
  ပြန်ပေး StringRef();
}

void ScalarTraits<Hex16>::output(const Hex16 &Val, void *, raw_ostream &Out) {
  Out << format("0x%" PRIX16, (uint16_t)Val);
}

StringRef ScalarTraits<Hex16>::input(StringRef Scalar, void *, Hex16 &Val) {
  unsigned long long n;
  အကယ်၍ (getAsUnsignedInteger(Scalar, 0, n))
    ပြန်ပေး "invalid hex16 number";
  အကယ်၍ (n > 0xFFFF)
    ပြန်ပေး "out of range hex16 number";
  Val = n;
  ပြန်ပေး StringRef();
}

void ScalarTraits<Hex32>::output(const Hex32 &Val, void *, raw_ostream &Out) {
  Out << format("0x%" PRIX32, (uint32_t)Val);
}

StringRef ScalarTraits<Hex32>::input(StringRef Scalar, void *, Hex32 &Val) {
  unsigned long long n;
  အကယ်၍ (getAsUnsignedInteger(Scalar, 0, n))
    ပြန်ပေး "invalid hex32 number";
  အကယ်၍ (n > 0xFFFFFFFFUL)
    ပြန်ပေး "out of range hex32 number";
  Val = n;
  ပြန်ပေး StringRef();
}

void ScalarTraits<Hex64>::output(const Hex64 &Val, void *, raw_ostream &Out) {
  Out << format("0x%" PRIX64, (uint64_t)Val);
}

StringRef ScalarTraits<Hex64>::input(StringRef Scalar, void *, Hex64 &Val) {
  unsigned long long Num;
  အကယ်၍ (getAsUnsignedInteger(Scalar, 0, Num))
    ပြန်ပေး "invalid hex64 number";
  Val = Num;
  ပြန်ပေး StringRef();
}

void ScalarTraits<VersionTuple>::output(const VersionTuple &Val, void *,
                                        llvm::raw_ostream &Out) {
  Out << Val.getAsString();
}

StringRef ScalarTraits<VersionTuple>::input(StringRef Scalar, void *,
                                            VersionTuple &Val) {
  အကယ်၍ (Val.tryParse(Scalar))
    ပြန်ပေး "invalid version format";
  ပြန်ပေး StringRef();
}
