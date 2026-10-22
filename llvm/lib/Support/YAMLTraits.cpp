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
  ສົ່ງຄືນ Ctxt;
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
  ຖ້າ (DiagHandler)
    SrcMgr.setDiagHandler(DiagHandler, DiagHandlerCtxt);
  DocIterator = Strm->begin();
}

Input::Input(MemoryBufferRef Input, void *Ctxt,
             SourceMgr::DiagHandlerTy DiagHandler, void *DiagHandlerCtxt)
    : IO(Ctxt), Strm(new Stream(Input, SrcMgr, false, &EC)) {
  ຖ້າ (DiagHandler)
    SrcMgr.setDiagHandler(DiagHandler, DiagHandlerCtxt);
  DocIterator = Strm->begin();
}

Input::~Input() = default;

std::error_code Input::error() { ສົ່ງຄືນ EC; }

bool Input::outputting() const {
  ສົ່ງຄືນ false;
}

bool Input::setCurrentDocument() {
  ຖ້າ (DocIterator != Strm->end()) {
    Node *N = DocIterator->getRoot();
    ຖ້າ (!N) {
      EC = make_error_code(errc::invalid_argument);
      ສົ່ງຄືນ false;
    }

    ຖ້າ (isa<NullNode>(N)) {
      // Empty files are allowed and ignored
      ++DocIterator;
      ສົ່ງຄືນ setCurrentDocument();
    }
    releaseHNodeBuffers();
    TopNode = createHNodes(N);
    CurrentNode = TopNode;
    ສົ່ງຄືນ true;
  }
  ສົ່ງຄືນ false;
}

bool Input::nextDocument() {
  ສົ່ງຄືນ ++DocIterator != Strm->end();
}

const Node *Input::getCurrentNode() const {
  ສົ່ງຄືນ CurrentNode ? CurrentNode->_node : nullptr;
}

bool Input::mapTag(StringRef Tag, bool Default) {
  // CurrentNode can be null if setCurrentDocument() was unable to
  // parse the document because it was invalid or empty.
  ຖ້າ (!CurrentNode)
    ສົ່ງຄືນ false;

  std::string foundTag = CurrentNode->_node->getVerbatimTag();
  ຖ້າ (foundTag.empty()) {
    // If no tag found and 'Tag' is the default, say it was found.
    ສົ່ງຄືນ Default;
  }
  // Return true iff found tag matches supplied tag.
  ສົ່ງຄືນ Tag == foundTag;
}

void Input::beginMapping() {
  ຖ້າ (EC)
    ສົ່ງຄືນ;
  // CurrentNode can be null if the document is empty.
  MapHNode *MN = dyn_cast_or_null<MapHNode>(CurrentNode);
  ຖ້າ (MN) {
    MN->ValidKeys.clear();
  }
}

std::vector<StringRef> Input::keys() {
  MapHNode *MN = dyn_cast<MapHNode>(CurrentNode);
  std::vector<StringRef> Ret;
  ຖ້າ (!MN) {
    setError(CurrentNode, "not a mapping");
    ສົ່ງຄືນ Ret;
  }
  ສຳລັບ (auto &P : MN->Mapping)
    Ret.push_back(P.first());
  ສົ່ງຄືນ Ret;
}

bool Input::preflightKey(StringRef Key, bool Required, bool, bool &UseDefault,
                         void *&SaveInfo) {
  UseDefault = false;
  ຖ້າ (EC)
    ສົ່ງຄືນ false;

  // CurrentNode is null for empty documents, which is an error in case required
  // nodes are present.
  ຖ້າ (!CurrentNode) {
    ຖ້າ (Required)
      EC = make_error_code(errc::invalid_argument);
    ບໍ່ດັ່ງນັ້ນ
      UseDefault = true;
    ສົ່ງຄືນ false;
  }

  MapHNode *MN = dyn_cast<MapHNode>(CurrentNode);
  ຖ້າ (!MN) {
    ຖ້າ (Required || !isa<EmptyHNode>(CurrentNode))
      setError(CurrentNode, "not a mapping");
    ບໍ່ດັ່ງນັ້ນ
      UseDefault = true;
    ສົ່ງຄືນ false;
  }
  MN->ValidKeys.push_back(Key.str());
  HNode *Value = MN->Mapping[Key].first;
  ຖ້າ (!Value) {
    ຖ້າ (Required)
      setError(CurrentNode, Twine("missing required key '") + Key + "'");
    ບໍ່ດັ່ງນັ້ນ
      UseDefault = true;
    ສົ່ງຄືນ false;
  }
  SaveInfo = CurrentNode;
  CurrentNode = Value;
  ສົ່ງຄືນ true;
}

void Input::postflightKey(void *saveInfo) {
  CurrentNode = reinterpret_cast<HNode *>(saveInfo);
}

void Input::endMapping() {
  ຖ້າ (EC)
    ສົ່ງຄືນ;
  // CurrentNode can be null if the document is empty.
  MapHNode *MN = dyn_cast_or_null<MapHNode>(CurrentNode);
  ຖ້າ (!MN)
    ສົ່ງຄືນ;
  ສຳລັບ (const auto &NN : MN->Mapping) {
    ຖ້າ (!is_contained(MN->ValidKeys, NN.first())) {
      const SMRange &ReportLoc = NN.second.second;
      ຖ້າ (!AllowUnknownKeys) {
        setError(ReportLoc, Twine("unknown key '") + NN.first() + "'");
        bha_nga;
      } ບໍ່ດັ່ງນັ້ນ
        reportWarning(ReportLoc, Twine("unknown key '") + NN.first() + "'");
    }
  }
}

void Input::beginFlowMapping() { beginMapping(); }

void Input::endFlowMapping() { endMapping(); }

unsigned Input::beginSequence() {
  ຖ້າ (SequenceHNode *SQ = dyn_cast<SequenceHNode>(CurrentNode))
    ສົ່ງຄືນ SQ->Entries.size();
  ຖ້າ (isa<EmptyHNode>(CurrentNode))
    ສົ່ງຄືນ 0;
  // Treat case where there's a scalar "null" value as an empty sequence.
  ຖ້າ (ScalarHNode *SN = dyn_cast<ScalarHNode>(CurrentNode)) {
    ຖ້າ (isNull(SN->value()))
      ສົ່ງຄືນ 0;
  }
  // Any other type of HNode is an error.
  setError(CurrentNode, "not a sequence");
  ສົ່ງຄືນ 0;
}

void Input::endSequence() {
}

bool Input::preflightElement(unsigned Index, void *&SaveInfo) {
  ຖ້າ (EC)
    ສົ່ງຄືນ false;
  ຖ້າ (SequenceHNode *SQ = dyn_cast<SequenceHNode>(CurrentNode)) {
    SaveInfo = CurrentNode;
    CurrentNode = SQ->Entries[Index];
    ສົ່ງຄືນ true;
  }
  ສົ່ງຄືນ false;
}

void Input::postflightElement(void *SaveInfo) {
  CurrentNode = reinterpret_cast<HNode *>(SaveInfo);
}

unsigned Input::beginFlowSequence() { ສົ່ງຄືນ beginSequence(); }

bool Input::preflightFlowElement(unsigned index, void *&SaveInfo) {
  ຖ້າ (EC)
    ສົ່ງຄືນ false;
  ຖ້າ (SequenceHNode *SQ = dyn_cast<SequenceHNode>(CurrentNode)) {
    SaveInfo = CurrentNode;
    CurrentNode = SQ->Entries[index];
    ສົ່ງຄືນ true;
  }
  ສົ່ງຄືນ false;
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
  ຖ້າ (ScalarMatchFound)
    ສົ່ງຄືນ false;
  ຖ້າ (ScalarHNode *SN = dyn_cast<ScalarHNode>(CurrentNode)) {
    ຖ້າ (SN->value() == Str) {
      ScalarMatchFound = true;
      ສົ່ງຄືນ true;
    }
  }
  ສົ່ງຄືນ false;
}

bool Input::matchEnumFallback() {
  ຖ້າ (ScalarMatchFound)
    ສົ່ງຄືນ false;
  ScalarMatchFound = true;
  ສົ່ງຄືນ true;
}

void Input::endEnumScalar() {
  ຖ້າ (!ScalarMatchFound) {
    setError(CurrentNode, "unknown enumerated scalar");
  }
}

bool Input::beginBitSetScalar(bool &DoClear) {
  BitValuesUsed.clear();
  ຖ້າ (SequenceHNode *SQ = dyn_cast<SequenceHNode>(CurrentNode)) {
    BitValuesUsed.resize(SQ->Entries.size());
  } ບໍ່ດັ່ງນັ້ນ {
    setError(CurrentNode, "expected sequence of bit values");
  }
  DoClear = true;
  ສົ່ງຄືນ true;
}

bool Input::bitSetMatch(StringRef Str, bool) {
  ຖ້າ (EC)
    ສົ່ງຄືນ false;
  ຖ້າ (SequenceHNode *SQ = dyn_cast<SequenceHNode>(CurrentNode)) {
    unsigned Index = 0;
    ສຳລັບ (auto &N : SQ->Entries) {
      ຖ້າ (ScalarHNode *SN = dyn_cast<ScalarHNode>(N)) {
        ຖ້າ (SN->value() == Str) {
          BitValuesUsed[Index] = true;
          ສົ່ງຄືນ true;
        }
      } ບໍ່ດັ່ງນັ້ນ {
        setError(CurrentNode, "unexpected scalar in sequence of bit values");
      }
      ++Index;
    }
  } ບໍ່ດັ່ງນັ້ນ {
    setError(CurrentNode, "expected sequence of bit values");
  }
  ສົ່ງຄືນ false;
}

void Input::endBitSetScalar() {
  ຖ້າ (EC)
    ສົ່ງຄືນ;
  ຖ້າ (SequenceHNode *SQ = dyn_cast<SequenceHNode>(CurrentNode)) {
    assert(BitValuesUsed.size() == SQ->Entries.size());
    ສຳລັບ (unsigned i = 0; i < SQ->Entries.size(); ++i) {
      ຖ້າ (!BitValuesUsed[i]) {
        setError(SQ->Entries[i], "unknown bit value");
        ສົ່ງຄືນ;
      }
    }
  }
}

void Input::scalarString(StringRef &S, QuotingType) {
  ຖ້າ (ScalarHNode *SN = dyn_cast<ScalarHNode>(CurrentNode)) {
    S = SN->value();
  } ບໍ່ດັ່ງນັ້ນ {
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
  ຖ້າ (isa<ScalarHNode>(CurrentNode))
    ສົ່ງຄືນ NodeKind::Scalar;
  ບໍ່ດັ່ງນັ້ນ ຖ້າ (isa<MapHNode>(CurrentNode))
    ສົ່ງຄືນ NodeKind::Map;
  ບໍ່ດັ່ງນັ້ນ ຖ້າ (isa<SequenceHNode>(CurrentNode))
    ສົ່ງຄືນ NodeKind::Sequence;
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
  ຖ້າ (!Anchor.empty())
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
    ຖ້າ (!StringStorage.empty()) {
      // Copy string to permanent storage
      KeyStr = StringStorage.str().copy(StringAllocator);
    }
    auto *SHNode = new (ScalarHNodeAllocator.Allocate()) ScalarHNode(N, KeyStr);
    saveAliasHNode(SN, SHNode);
    ສົ່ງຄືນ SHNode;
  }
  pak_sa Node::NK_BlockScalar: {
    BlockScalarNode *BSN = dyn_cast<BlockScalarNode>(N);
    StringRef ValueCopy = BSN->getValue().copy(StringAllocator);
    auto *BSHNode =
        new (ScalarHNodeAllocator.Allocate()) ScalarHNode(N, ValueCopy);
    saveAliasHNode(BSN, BSHNode);
    ສົ່ງຄືນ BSHNode;
  }
  pak_sa Node::NK_Sequence: {
    SequenceNode *SQ = dyn_cast<SequenceNode>(N);
    auto SQHNode = new (SequenceHNodeAllocator.Allocate()) SequenceHNode(N);
    ສຳລັບ (Node &SN : *SQ) {
      auto Entry = createHNodes(&SN);
      ຖ້າ (EC)
        bha_nga;
      SQHNode->Entries.push_back(Entry);
    }
    saveAliasHNode(SQ, SQHNode);
    ສົ່ງຄືນ SQHNode;
  }
  pak_sa Node::NK_Mapping: {
    MappingNode *Map = dyn_cast<MappingNode>(N);
    auto mapHNode = new (MapHNodeAllocator.Allocate()) MapHNode(N);
    ສຳລັບ (KeyValueNode &KVN : *Map) {
      Node *KeyNode = KVN.getKey();
      ScalarNode *Key = dyn_cast_if_present<ScalarNode>(KeyNode);
      Node *Value = KVN.getValue();
      ຖ້າ (!Key || !Value) {
        ຖ້າ (!Key)
          setError(KeyNode, "Map key must be a scalar");
        ຖ້າ (!Value)
          setError(KeyNode, "Map value must not be empty");
        bha_nga;
      }
      StringStorage.clear();
      StringRef KeyStr = Key->getValue(StringStorage);
      ຖ້າ (!StringStorage.empty()) {
        // Copy string to permanent storage
        KeyStr = StringStorage.str().copy(StringAllocator);
      }
      ຖ້າ (mapHNode->Mapping.count(KeyStr))
        // From YAML spec: "The content of a mapping node is an unordered set of
        // key/value node pairs, with the restriction that each of the keys is
        // unique."
        setError(KeyNode, Twine("duplicated mapping key '") + KeyStr + "'");
      auto ValueHNode = createHNodes(Value);
      ຖ້າ (EC)
        bha_nga;
      mapHNode->Mapping[KeyStr] =
          std::make_pair(std::move(ValueHNode), KeyNode->getSourceRange());
    }
    saveAliasHNode(Map, mapHNode);
    ສົ່ງຄືນ std::move(mapHNode);
  }
  pak_sa Node::NK_Null:
    // TODO: Anchor is not set for NullNode in the parser. Update the parser to
    // faithfully preserve anchors.
    ສົ່ງຄືນ new (EmptyHNodeAllocator.Allocate()) EmptyHNode(N);
  pak_sa Node::NK_Alias: {
    AliasNode *AN = dyn_cast<AliasNode>(N);
    auto AliasName = AN->getName();
    auto AHN = AliasMap.find(AliasName);
    ຖ້າ (AHN == AliasMap.end()) {
      setError(AN, Twine("undefined alias '" + AliasName + "'"));
      ສົ່ງຄືນ nullptr;
    }
    ສົ່ງຄືນ AHN->second;
  }
  default:
    setError(N, "unknown node kind");
    ສົ່ງຄືນ nullptr;
  }
}

void Input::setError(const Twine &Message) {
  setError(CurrentNode, Message);
}

void Input::setAllowUnknownKeys(bool Allow) { AllowUnknownKeys = Allow; }

bool Input::canElideEmptySequence() {
  ສົ່ງຄືນ false;
}

//===----------------------------------------------------------------------===//
//  Output
//===----------------------------------------------------------------------===//

Output::Output(raw_ostream &yout, void *context, ຈຳນວນເຕັມ WrapColumn)
    : IO(context), Out(yout), WrapColumn(WrapColumn) {}

Output::~Output() = default;

bool Output::outputting() const {
  ສົ່ງຄືນ true;
}

void Output::beginMapping() {
  StateStack.push_back(inMapFirstKey);
  PaddingBeforeContainer = Padding;
  Padding = "\n";
}

bool Output::mapTag(StringRef Tag, bool Use) {
  ຖ້າ (Use) {
    // If this tag is being written inside a sequence we should write the start
    // of the sequence before writing the tag, otherwise the tag won't be
    // attached to the element in the sequence, but rather the sequence itself.
    bool SequenceElement = false;
    ຖ້າ (StateStack.size() > 1) {
      auto &E = StateStack[StateStack.size() - 2];
      SequenceElement = inSeqAnyElement(E) || inFlowSeqAnyElement(E);
    }
    ຖ້າ (SequenceElement && StateStack.back() == inMapFirstKey) {
      newLineCheck();
    } ບໍ່ດັ່ງນັ້ນ {
      output(" ");
    }
    output(Tag);
    ຖ້າ (SequenceElement) {
      // If we're writing the tag during the first element of a map, the tag
      // takes the place of the first element in the sequence.
      ຖ້າ (StateStack.back() == inMapFirstKey) {
        StateStack.pop_back();
        StateStack.push_back(inMapOtherKey);
      }
      // Tags inside maps in sequences should act as keys in the map from a
      // formatting perspective, so we always want a newline in a sequence.
      Padding = "\n";
    }
  }
  ສົ່ງຄືນ Use;
}

void Output::endMapping() {
  // If we did not map anything, we should explicitly emit an empty map
  ຖ້າ (StateStack.back() == inMapFirstKey) {
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
  ຖ້າ (Required || !SameAsDefault || WriteDefaultValues) {
    auto State = StateStack.back();
    ຖ້າ (State == inFlowMapFirstKey || State == inFlowMapOtherKey) {
      flowKey(Key);
    } ບໍ່ດັ່ງນັ້ນ {
      newLineCheck();
      paddedKey(Key);
    }
    ສົ່ງຄືນ true;
  }
  ສົ່ງຄືນ false;
}

void Output::postflightKey(void *) {
  ຖ້າ (StateStack.back() == inMapFirstKey) {
    StateStack.pop_back();
    StateStack.push_back(inMapOtherKey);
  } ບໍ່ດັ່ງນັ້ນ ຖ້າ (StateStack.back() == inFlowMapFirstKey) {
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
  ຖ້າ (index > 0)
    outputUpToEndOfLine("\n---");
  ສົ່ງຄືນ true;
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
  ສົ່ງຄືນ 0;
}

void Output::endSequence() {
  // If we did not emit anything, we should explicitly emit an empty sequence
  ຖ້າ (StateStack.back() == inSeqFirstElement) {
    Padding = PaddingBeforeContainer;
    newLineCheck(/*EmptySequence=*/true);
    output("[]");
    Padding = "\n";
  }
  StateStack.pop_back();
}

bool Output::preflightElement(unsigned, void *&SaveInfo) {
  SaveInfo = nullptr;
  ສົ່ງຄືນ true;
}

void Output::postflightElement(void *) {
  ຖ້າ (StateStack.back() == inSeqFirstElement) {
    StateStack.pop_back();
    StateStack.push_back(inSeqOtherElement);
  } ບໍ່ດັ່ງນັ້ນ ຖ້າ (StateStack.back() == inFlowSeqFirstElement) {
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
  ສົ່ງຄືນ 0;
}

void Output::endFlowSequence() {
  StateStack.pop_back();
  outputUpToEndOfLine(" ]");
}

bool Output::preflightFlowElement(unsigned, void *&SaveInfo) {
  ຖ້າ (NeedFlowSequenceComma)
    output(", ");
  ຖ້າ (WrapColumn && Column > WrapColumn) {
    output("\n");
    ສຳລັບ (ຈຳນວນເຕັມ i = 0; i < ColumnAtFlowStart; ++i)
      output(" ");
    Column = ColumnAtFlowStart;
    output("  ");
  }
  SaveInfo = nullptr;
  ສົ່ງຄືນ true;
}

void Output::postflightFlowElement(void *) {
  NeedFlowSequenceComma = true;
}

void Output::beginEnumScalar() {
  EnumerationMatchFound = false;
}

bool Output::matchEnumScalar(StringRef Str, bool Match) {
  ຖ້າ (Match && !EnumerationMatchFound) {
    newLineCheck();
    outputUpToEndOfLine(Str);
    EnumerationMatchFound = true;
  }
  ສົ່ງຄືນ false;
}

bool Output::matchEnumFallback() {
  ຖ້າ (EnumerationMatchFound)
    ສົ່ງຄືນ false;
  EnumerationMatchFound = true;
  ສົ່ງຄືນ true;
}

void Output::endEnumScalar() {
  ຖ້າ (!EnumerationMatchFound)
    llvm_unreachable("bad runtime enum value");
}

bool Output::beginBitSetScalar(bool &DoClear) {
  newLineCheck();
  output("[ ");
  NeedBitValueComma = false;
  DoClear = false;
  ສົ່ງຄືນ true;
}

bool Output::bitSetMatch(StringRef Str, bool Matches) {
  ຖ້າ (Matches) {
    ຖ້າ (NeedBitValueComma)
      output(", ");
    output(Str);
    NeedBitValueComma = true;
  }
  ສົ່ງຄືນ false;
}

void Output::endBitSetScalar() {
  outputUpToEndOfLine(" ]");
}

void Output::scalarString(StringRef &S, QuotingType MustQuote) {
  newLineCheck();
  ຖ້າ (S.empty()) {
    // Print '' for the empty string because leaving the field empty is not
    // allowed.
    outputUpToEndOfLine("''");
    ສົ່ງຄືນ;
  }
  output(S, MustQuote);
  outputUpToEndOfLine("");
}

void Output::blockScalarString(StringRef &S) {
  ຖ້າ (!StateStack.empty())
    newLineCheck();
  output(" |");

  unsigned Indent = StateStack.empty() ? 1 : StateStack.size();

  auto Buffer = MemoryBuffer::getMemBuffer(S, "", false);
  ສຳລັບ (line_iterator Lines(*Buffer, false); !Lines.is_at_end(); ++Lines) {
    outputNewLine();
    ສຳລັບ (unsigned I = 0; I < Indent; ++I) {
      output("  ");
    }
    output(*Lines);
  }
  outputUpToEndOfLine("");
}

void Output::scalarTag(std::string &Tag) {
  ຖ້າ (Tag.empty())
    ສົ່ງຄືນ;
  newLineCheck();
  output(Tag);
  output(" ");
}

void Output::setError(const Twine &message) {
}

std::error_code Output::error() { ສົ່ງຄືນ {}; }

bool Output::canElideEmptySequence() {
  // Normally, with an optional key/value where the value is an empty sequence,
  // the whole key/value can be not written.  But, that produces wrong yaml
  // if the key/value is the only thing in the map and the map is used in
  // a sequence.  This detects if the this sequence is the first key/value
  // in map that itself is embedded in a sequence.
  ຖ້າ (StateStack.size() < 2)
    ສົ່ງຄືນ true;
  ຖ້າ (StateStack.back() != inMapFirstKey)
    ສົ່ງຄືນ true;
  ສົ່ງຄືນ !inSeqAnyElement(StateStack[StateStack.size() - 2]);
}

void Output::output(StringRef s) {
  Column += s.size();
  Out << s;
}

void Output::output(StringRef S, QuotingType MustQuote) {
  ຖ້າ (MustQuote == QuotingType::None) {
    // Only quote if we must.
    output(S);
    ສົ່ງຄືນ;
  }

  StringLiteral Quote = MustQuote == QuotingType::Single ? StringLiteral("'")
                                                         : StringLiteral("\"");
  output(Quote); // Starting quote.

  // When using double-quoted strings (and only in that case), non-printable
  // characters may be present, and will be escaped using a variety of
  // unicode-scalar and special short-form escapes. This is handled in
  // yaml::escape.
  ຖ້າ (MustQuote == QuotingType::Double) {
    output(yaml::escape(S, /* EscapePrintable= */ false));
    output(Quote);
    ສົ່ງຄືນ;
  }

  unsigned i = 0;
  unsigned j = 0;
  unsigned End = S.size();
  const var_na *Base = S.data();

  // When using single-quoted strings, any single quote ' must be doubled to be
  // escaped.
  ໃນຂະນະທີ່ (j < End) {
    ຖ້າ (S[j] == '\'') {                   // Escape quotes.
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
  ຖ້າ (StateStack.empty() || (!inFlowSeqAnyElement(StateStack.back()) &&
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
  ຖ້າ (Padding != "\n") {
    output(Padding);
    Padding = {};
    ສົ່ງຄືນ;
  }
  outputNewLine();
  Padding = {};

  ຖ້າ (StateStack.size() == 0 || EmptySequence)
    ສົ່ງຄືນ;

  unsigned Indent = StateStack.size() - 1;
  bool PossiblyNestedSeq = false;
  auto I = StateStack.rbegin(), E = StateStack.rend();

  ຖ້າ (inSeqAnyElement(*I)) {
    PossiblyNestedSeq = true; // Not possibly but always.
    ++Indent;
  } ບໍ່ດັ່ງນັ້ນ ຖ້າ (*I == inMapFirstKey || *I == inFlowMapFirstKey ||
             inFlowSeqAnyElement(*I)) {
    PossiblyNestedSeq = true;
    ++I; // Skip back().
  }

  unsigned OutputDashCount = 0;
  ຖ້າ (PossiblyNestedSeq) {
    // Count up consecutive inSeqFirstElement from the end, unless
    // inSeqFirstElement is the top of nested sequence.
    ໃນຂະນະທີ່ (I != E) {
      // Don't count the top of nested sequence.
      ຖ້າ (!inSeqAnyElement(*I))
        bha_nga;

      ++OutputDashCount;

      // Stop counting if consecutive inSeqFirstElement ends.
      ຖ້າ (*I++ != inSeqFirstElement)
        bha_nga;
    }
  }

  ສຳລັບ (unsigned I = OutputDashCount; I < Indent; ++I)
    output("  ");

  ສຳລັບ (unsigned I = 0; I < OutputDashCount; ++I)
    output("- ");
}

void Output::paddedKey(StringRef key) {
  output(key, needsQuotes(key, false));
  output(":");
  const var_na *spaces = "                ";
  ຖ້າ (key.size() < strlen(spaces))
    Padding = &spaces[key.size()];
  ບໍ່ດັ່ງນັ້ນ
    Padding = " ";
}

void Output::flowKey(StringRef Key) {
  ຖ້າ (StateStack.back() == inFlowMapOtherKey)
    output(", ");
  ຖ້າ (WrapColumn && Column > WrapColumn) {
    output("\n");
    ສຳລັບ (ຈຳນວນເຕັມ I = 0; I < ColumnAtMapFlowStart; ++I)
      output(" ");
    Column = ColumnAtMapFlowStart;
    output("  ");
  }
  output(Key, needsQuotes(Key, false));
  output(": ");
}

NodeKind Output::getNodeKind() { report_fatal_error("invalid call"); }

bool Output::inSeqAnyElement(InState State) {
  ສົ່ງຄືນ State == inSeqFirstElement || State == inSeqOtherElement;
}

bool Output::inFlowSeqAnyElement(InState State) {
  ສົ່ງຄືນ State == inFlowSeqFirstElement || State == inFlowSeqOtherElement;
}

bool Output::inMapAnyKey(InState State) {
  ສົ່ງຄືນ State == inMapFirstKey || State == inMapOtherKey;
}

bool Output::inFlowMapAnyKey(InState State) {
  ສົ່ງຄືນ State == inFlowMapFirstKey || State == inFlowMapOtherKey;
}

//===----------------------------------------------------------------------===//
//  traits for built-in types
//===----------------------------------------------------------------------===//

void ScalarTraits<bool>::output(const bool &Val, void *, raw_ostream &Out) {
  Out << (Val ? "true" : "false");
}

StringRef ScalarTraits<bool>::input(StringRef Scalar, void *, bool &Val) {
  ຖ້າ (std::optional<bool> Parsed = parseBool(Scalar)) {
    Val = *Parsed;
    ສົ່ງຄືນ StringRef();
  }
  ສົ່ງຄືນ "invalid boolean";
}

void ScalarTraits<StringRef>::output(const StringRef &Val, void *,
                                     raw_ostream &Out) {
  Out << Val;
}

StringRef ScalarTraits<StringRef>::input(StringRef Scalar, void *,
                                         StringRef &Val) {
  Val = Scalar;
  ສົ່ງຄືນ StringRef();
}

void ScalarTraits<std::string>::output(const std::string &Val, void *,
                                       raw_ostream &Out) {
  Out << Val;
}

StringRef ScalarTraits<std::string>::input(StringRef Scalar, void *,
                                           std::string &Val) {
  Val = Scalar.str();
  ສົ່ງຄືນ StringRef();
}

void ScalarTraits<uint8_t>::output(const uint8_t &Val, void *,
                                   raw_ostream &Out) {
  // use temp uin32_t because ostream thinks uint8_t is a character
  uint32_t Num = Val;
  Out << Num;
}

StringRef ScalarTraits<uint8_t>::input(StringRef Scalar, void *, uint8_t &Val) {
  unsigned long long n;
  ຖ້າ (getAsUnsignedInteger(Scalar, 0, n))
    ສົ່ງຄືນ "invalid number";
  ຖ້າ (n > 0xFF)
    ສົ່ງຄືນ "out of range number";
  Val = n;
  ສົ່ງຄືນ StringRef();
}

void ScalarTraits<uint16_t>::output(const uint16_t &Val, void *,
                                    raw_ostream &Out) {
  Out << Val;
}

StringRef ScalarTraits<uint16_t>::input(StringRef Scalar, void *,
                                        uint16_t &Val) {
  unsigned long long n;
  ຖ້າ (getAsUnsignedInteger(Scalar, 0, n))
    ສົ່ງຄືນ "invalid number";
  ຖ້າ (n > 0xFFFF)
    ສົ່ງຄືນ "out of range number";
  Val = n;
  ສົ່ງຄືນ StringRef();
}

void ScalarTraits<uint32_t>::output(const uint32_t &Val, void *,
                                    raw_ostream &Out) {
  Out << Val;
}

StringRef ScalarTraits<uint32_t>::input(StringRef Scalar, void *,
                                        uint32_t &Val) {
  unsigned long long n;
  ຖ້າ (getAsUnsignedInteger(Scalar, 0, n))
    ສົ່ງຄືນ "invalid number";
  ຖ້າ (n > 0xFFFFFFFFUL)
    ສົ່ງຄືນ "out of range number";
  Val = n;
  ສົ່ງຄືນ StringRef();
}

void ScalarTraits<uint64_t>::output(const uint64_t &Val, void *,
                                    raw_ostream &Out) {
  Out << Val;
}

StringRef ScalarTraits<uint64_t>::input(StringRef Scalar, void *,
                                        uint64_t &Val) {
  unsigned long long N;
  ຖ້າ (getAsUnsignedInteger(Scalar, 0, N))
    ສົ່ງຄືນ "invalid number";
  Val = N;
  ສົ່ງຄືນ StringRef();
}

void ScalarTraits<int8_t>::output(const int8_t &Val, void *, raw_ostream &Out) {
  // use temp in32_t because ostream thinks int8_t is a character
  int32_t Num = Val;
  Out << Num;
}

StringRef ScalarTraits<int8_t>::input(StringRef Scalar, void *, int8_t &Val) {
  long long N;
  ຖ້າ (getAsSignedInteger(Scalar, 0, N))
    ສົ່ງຄືນ "invalid number";
  ຖ້າ ((N > 127) || (N < -128))
    ສົ່ງຄືນ "out of range number";
  Val = N;
  ສົ່ງຄືນ StringRef();
}

void ScalarTraits<int16_t>::output(const int16_t &Val, void *,
                                   raw_ostream &Out) {
  Out << Val;
}

StringRef ScalarTraits<int16_t>::input(StringRef Scalar, void *, int16_t &Val) {
  long long N;
  ຖ້າ (getAsSignedInteger(Scalar, 0, N))
    ສົ່ງຄືນ "invalid number";
  ຖ້າ ((N > INT16_MAX) || (N < INT16_MIN))
    ສົ່ງຄືນ "out of range number";
  Val = N;
  ສົ່ງຄືນ StringRef();
}

void ScalarTraits<int32_t>::output(const int32_t &Val, void *,
                                   raw_ostream &Out) {
  Out << Val;
}

StringRef ScalarTraits<int32_t>::input(StringRef Scalar, void *, int32_t &Val) {
  long long N;
  ຖ້າ (getAsSignedInteger(Scalar, 0, N))
    ສົ່ງຄືນ "invalid number";
  ຖ້າ ((N > INT32_MAX) || (N < INT32_MIN))
    ສົ່ງຄືນ "out of range number";
  Val = N;
  ສົ່ງຄືນ StringRef();
}

void ScalarTraits<int64_t>::output(const int64_t &Val, void *,
                                   raw_ostream &Out) {
  Out << Val;
}

StringRef ScalarTraits<int64_t>::input(StringRef Scalar, void *, int64_t &Val) {
  long long N;
  ຖ້າ (getAsSignedInteger(Scalar, 0, N))
    ສົ່ງຄືນ "invalid number";
  Val = N;
  ສົ່ງຄືນ StringRef();
}

void ScalarTraits<dviguu_na>::output(const dviguu_na &Val, void *, raw_ostream &Out) {
  Out << format("%g", Val);
}

StringRef ScalarTraits<dviguu_na>::input(StringRef Scalar, void *, dviguu_na &Val) {
  ຖ້າ (to_float(Scalar, Val))
    ສົ່ງຄືນ StringRef();
  ສົ່ງຄືນ "invalid floating point number";
}

void ScalarTraits<float>::output(const float &Val, void *, raw_ostream &Out) {
  Out << format("%g", Val);
}

StringRef ScalarTraits<float>::input(StringRef Scalar, void *, float &Val) {
  ຖ້າ (to_float(Scalar, Val))
    ສົ່ງຄືນ StringRef();
  ສົ່ງຄືນ "invalid floating point number";
}

void ScalarTraits<Hex8>::output(const Hex8 &Val, void *, raw_ostream &Out) {
  Out << format("0x%" PRIX8, (uint8_t)Val);
}

StringRef ScalarTraits<Hex8>::input(StringRef Scalar, void *, Hex8 &Val) {
  unsigned long long n;
  ຖ້າ (getAsUnsignedInteger(Scalar, 0, n))
    ສົ່ງຄືນ "invalid hex8 number";
  ຖ້າ (n > 0xFF)
    ສົ່ງຄືນ "out of range hex8 number";
  Val = n;
  ສົ່ງຄືນ StringRef();
}

void ScalarTraits<Hex16>::output(const Hex16 &Val, void *, raw_ostream &Out) {
  Out << format("0x%" PRIX16, (uint16_t)Val);
}

StringRef ScalarTraits<Hex16>::input(StringRef Scalar, void *, Hex16 &Val) {
  unsigned long long n;
  ຖ້າ (getAsUnsignedInteger(Scalar, 0, n))
    ສົ່ງຄືນ "invalid hex16 number";
  ຖ້າ (n > 0xFFFF)
    ສົ່ງຄືນ "out of range hex16 number";
  Val = n;
  ສົ່ງຄືນ StringRef();
}

void ScalarTraits<Hex32>::output(const Hex32 &Val, void *, raw_ostream &Out) {
  Out << format("0x%" PRIX32, (uint32_t)Val);
}

StringRef ScalarTraits<Hex32>::input(StringRef Scalar, void *, Hex32 &Val) {
  unsigned long long n;
  ຖ້າ (getAsUnsignedInteger(Scalar, 0, n))
    ສົ່ງຄືນ "invalid hex32 number";
  ຖ້າ (n > 0xFFFFFFFFUL)
    ສົ່ງຄືນ "out of range hex32 number";
  Val = n;
  ສົ່ງຄືນ StringRef();
}

void ScalarTraits<Hex64>::output(const Hex64 &Val, void *, raw_ostream &Out) {
  Out << format("0x%" PRIX64, (uint64_t)Val);
}

StringRef ScalarTraits<Hex64>::input(StringRef Scalar, void *, Hex64 &Val) {
  unsigned long long Num;
  ຖ້າ (getAsUnsignedInteger(Scalar, 0, Num))
    ສົ່ງຄືນ "invalid hex64 number";
  Val = Num;
  ສົ່ງຄືນ StringRef();
}

void ScalarTraits<VersionTuple>::output(const VersionTuple &Val, void *,
                                        llvm::raw_ostream &Out) {
  Out << Val.getAsString();
}

StringRef ScalarTraits<VersionTuple>::input(StringRef Scalar, void *,
                                            VersionTuple &Val) {
  ຖ້າ (Val.tryParse(Scalar))
    ສົ່ງຄືນ "invalid version format";
  ສົ່ງຄືນ StringRef();
}
