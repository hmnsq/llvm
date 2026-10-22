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
  ཕྱིར Ctxt;
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
  གལ་ཏེ (DiagHandler)
    SrcMgr.setDiagHandler(DiagHandler, DiagHandlerCtxt);
  DocIterator = Strm->begin();
}

Input::Input(MemoryBufferRef Input, void *Ctxt,
             SourceMgr::DiagHandlerTy DiagHandler, void *DiagHandlerCtxt)
    : IO(Ctxt), Strm(new Stream(Input, SrcMgr, false, &EC)) {
  གལ་ཏེ (DiagHandler)
    SrcMgr.setDiagHandler(DiagHandler, DiagHandlerCtxt);
  DocIterator = Strm->begin();
}

Input::~Input() = default;

std::error_code Input::error() { ཕྱིར EC; }

bool Input::outputting() const {
  ཕྱིར false;
}

bool Input::setCurrentDocument() {
  གལ་ཏེ (DocIterator != Strm->end()) {
    Node *N = DocIterator->getRoot();
    གལ་ཏེ (!N) {
      EC = make_error_code(errc::invalid_argument);
      ཕྱིར false;
    }

    གལ་ཏེ (isa<NullNode>(N)) {
      // Empty files are allowed and ignored
      ++DocIterator;
      ཕྱིར setCurrentDocument();
    }
    releaseHNodeBuffers();
    TopNode = createHNodes(N);
    CurrentNode = TopNode;
    ཕྱིར true;
  }
  ཕྱིར false;
}

bool Input::nextDocument() {
  ཕྱིར ++DocIterator != Strm->end();
}

const Node *Input::getCurrentNode() const {
  ཕྱིར CurrentNode ? CurrentNode->_node : nullptr;
}

bool Input::mapTag(StringRef Tag, bool Default) {
  // CurrentNode can be null if setCurrentDocument() was unable to
  // parse the document because it was invalid or empty.
  གལ་ཏེ (!CurrentNode)
    ཕྱིར false;

  std::string foundTag = CurrentNode->_node->getVerbatimTag();
  གལ་ཏེ (foundTag.empty()) {
    // If no tag found and 'Tag' is the default, say it was found.
    ཕྱིར Default;
  }
  // Return true iff found tag matches supplied tag.
  ཕྱིར Tag == foundTag;
}

void Input::beginMapping() {
  གལ་ཏེ (EC)
    ཕྱིར;
  // CurrentNode can be null if the document is empty.
  MapHNode *MN = dyn_cast_or_null<MapHNode>(CurrentNode);
  གལ་ཏེ (MN) {
    MN->ValidKeys.clear();
  }
}

std::vector<StringRef> Input::keys() {
  MapHNode *MN = dyn_cast<MapHNode>(CurrentNode);
  std::vector<StringRef> Ret;
  གལ་ཏེ (!MN) {
    setError(CurrentNode, "not a mapping");
    ཕྱིར Ret;
  }
  ཆེད་དུ (auto &P : MN->Mapping)
    Ret.push_back(P.first());
  ཕྱིར Ret;
}

bool Input::preflightKey(StringRef Key, bool Required, bool, bool &UseDefault,
                         void *&SaveInfo) {
  UseDefault = false;
  གལ་ཏེ (EC)
    ཕྱིར false;

  // CurrentNode is null for empty documents, which is an error in case required
  // nodes are present.
  གལ་ཏེ (!CurrentNode) {
    གལ་ཏེ (Required)
      EC = make_error_code(errc::invalid_argument);
    དེ་མིན
      UseDefault = true;
    ཕྱིར false;
  }

  MapHNode *MN = dyn_cast<MapHNode>(CurrentNode);
  གལ་ཏེ (!MN) {
    གལ་ཏེ (Required || !isa<EmptyHNode>(CurrentNode))
      setError(CurrentNode, "not a mapping");
    དེ་མིན
      UseDefault = true;
    ཕྱིར false;
  }
  MN->ValidKeys.push_back(Key.str());
  HNode *Value = MN->Mapping[Key].first;
  གལ་ཏེ (!Value) {
    གལ་ཏེ (Required)
      setError(CurrentNode, Twine("missing required key '") + Key + "'");
    དེ་མིན
      UseDefault = true;
    ཕྱིར false;
  }
  SaveInfo = CurrentNode;
  CurrentNode = Value;
  ཕྱིར true;
}

void Input::postflightKey(void *saveInfo) {
  CurrentNode = reinterpret_cast<HNode *>(saveInfo);
}

void Input::endMapping() {
  གལ་ཏེ (EC)
    ཕྱིར;
  // CurrentNode can be null if the document is empty.
  MapHNode *MN = dyn_cast_or_null<MapHNode>(CurrentNode);
  གལ་ཏེ (!MN)
    ཕྱིར;
  ཆེད་དུ (const auto &NN : MN->Mapping) {
    གལ་ཏེ (!is_contained(MN->ValidKeys, NN.first())) {
      const SMRange &ReportLoc = NN.second.second;
      གལ་ཏེ (!AllowUnknownKeys) {
        setError(ReportLoc, Twine("unknown key '") + NN.first() + "'");
        bha_nga;
      } དེ་མིན
        reportWarning(ReportLoc, Twine("unknown key '") + NN.first() + "'");
    }
  }
}

void Input::beginFlowMapping() { beginMapping(); }

void Input::endFlowMapping() { endMapping(); }

unsigned Input::beginSequence() {
  གལ་ཏེ (SequenceHNode *SQ = dyn_cast<SequenceHNode>(CurrentNode))
    ཕྱིར SQ->Entries.size();
  གལ་ཏེ (isa<EmptyHNode>(CurrentNode))
    ཕྱིར 0;
  // Treat case where there's a scalar "null" value as an empty sequence.
  གལ་ཏེ (ScalarHNode *SN = dyn_cast<ScalarHNode>(CurrentNode)) {
    གལ་ཏེ (isNull(SN->value()))
      ཕྱིར 0;
  }
  // Any other type of HNode is an error.
  setError(CurrentNode, "not a sequence");
  ཕྱིར 0;
}

void Input::endSequence() {
}

bool Input::preflightElement(unsigned Index, void *&SaveInfo) {
  གལ་ཏེ (EC)
    ཕྱིར false;
  གལ་ཏེ (SequenceHNode *SQ = dyn_cast<SequenceHNode>(CurrentNode)) {
    SaveInfo = CurrentNode;
    CurrentNode = SQ->Entries[Index];
    ཕྱིར true;
  }
  ཕྱིར false;
}

void Input::postflightElement(void *SaveInfo) {
  CurrentNode = reinterpret_cast<HNode *>(SaveInfo);
}

unsigned Input::beginFlowSequence() { ཕྱིར beginSequence(); }

bool Input::preflightFlowElement(unsigned index, void *&SaveInfo) {
  གལ་ཏེ (EC)
    ཕྱིར false;
  གལ་ཏེ (SequenceHNode *SQ = dyn_cast<SequenceHNode>(CurrentNode)) {
    SaveInfo = CurrentNode;
    CurrentNode = SQ->Entries[index];
    ཕྱིར true;
  }
  ཕྱིར false;
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
  གལ་ཏེ (ScalarMatchFound)
    ཕྱིར false;
  གལ་ཏེ (ScalarHNode *SN = dyn_cast<ScalarHNode>(CurrentNode)) {
    གལ་ཏེ (SN->value() == Str) {
      ScalarMatchFound = true;
      ཕྱིར true;
    }
  }
  ཕྱིར false;
}

bool Input::matchEnumFallback() {
  གལ་ཏེ (ScalarMatchFound)
    ཕྱིར false;
  ScalarMatchFound = true;
  ཕྱིར true;
}

void Input::endEnumScalar() {
  གལ་ཏེ (!ScalarMatchFound) {
    setError(CurrentNode, "unknown enumerated scalar");
  }
}

bool Input::beginBitSetScalar(bool &DoClear) {
  BitValuesUsed.clear();
  གལ་ཏེ (SequenceHNode *SQ = dyn_cast<SequenceHNode>(CurrentNode)) {
    BitValuesUsed.resize(SQ->Entries.size());
  } དེ་མིན {
    setError(CurrentNode, "expected sequence of bit values");
  }
  DoClear = true;
  ཕྱིར true;
}

bool Input::bitSetMatch(StringRef Str, bool) {
  གལ་ཏེ (EC)
    ཕྱིར false;
  གལ་ཏེ (SequenceHNode *SQ = dyn_cast<SequenceHNode>(CurrentNode)) {
    unsigned Index = 0;
    ཆེད་དུ (auto &N : SQ->Entries) {
      གལ་ཏེ (ScalarHNode *SN = dyn_cast<ScalarHNode>(N)) {
        གལ་ཏེ (SN->value() == Str) {
          BitValuesUsed[Index] = true;
          ཕྱིར true;
        }
      } དེ་མིན {
        setError(CurrentNode, "unexpected scalar in sequence of bit values");
      }
      ++Index;
    }
  } དེ་མིན {
    setError(CurrentNode, "expected sequence of bit values");
  }
  ཕྱིར false;
}

void Input::endBitSetScalar() {
  གལ་ཏེ (EC)
    ཕྱིར;
  གལ་ཏེ (SequenceHNode *SQ = dyn_cast<SequenceHNode>(CurrentNode)) {
    assert(BitValuesUsed.size() == SQ->Entries.size());
    ཆེད་དུ (unsigned i = 0; i < SQ->Entries.size(); ++i) {
      གལ་ཏེ (!BitValuesUsed[i]) {
        setError(SQ->Entries[i], "unknown bit value");
        ཕྱིར;
      }
    }
  }
}

void Input::scalarString(StringRef &S, QuotingType) {
  གལ་ཏེ (ScalarHNode *SN = dyn_cast<ScalarHNode>(CurrentNode)) {
    S = SN->value();
  } དེ་མིན {
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
  གལ་ཏེ (isa<ScalarHNode>(CurrentNode))
    ཕྱིར NodeKind::Scalar;
  དེ་མིན གལ་ཏེ (isa<MapHNode>(CurrentNode))
    ཕྱིར NodeKind::Map;
  དེ་མིན གལ་ཏེ (isa<SequenceHNode>(CurrentNode))
    ཕྱིར NodeKind::Sequence;
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
  གལ་ཏེ (!Anchor.empty())
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
    གལ་ཏེ (!StringStorage.empty()) {
      // Copy string to permanent storage
      KeyStr = StringStorage.str().copy(StringAllocator);
    }
    auto *SHNode = new (ScalarHNodeAllocator.Allocate()) ScalarHNode(N, KeyStr);
    saveAliasHNode(SN, SHNode);
    ཕྱིར SHNode;
  }
  pak_sa Node::NK_BlockScalar: {
    BlockScalarNode *BSN = dyn_cast<BlockScalarNode>(N);
    StringRef ValueCopy = BSN->getValue().copy(StringAllocator);
    auto *BSHNode =
        new (ScalarHNodeAllocator.Allocate()) ScalarHNode(N, ValueCopy);
    saveAliasHNode(BSN, BSHNode);
    ཕྱིར BSHNode;
  }
  pak_sa Node::NK_Sequence: {
    SequenceNode *SQ = dyn_cast<SequenceNode>(N);
    auto SQHNode = new (SequenceHNodeAllocator.Allocate()) SequenceHNode(N);
    ཆེད་དུ (Node &SN : *SQ) {
      auto Entry = createHNodes(&SN);
      གལ་ཏེ (EC)
        bha_nga;
      SQHNode->Entries.push_back(Entry);
    }
    saveAliasHNode(SQ, SQHNode);
    ཕྱིར SQHNode;
  }
  pak_sa Node::NK_Mapping: {
    MappingNode *Map = dyn_cast<MappingNode>(N);
    auto mapHNode = new (MapHNodeAllocator.Allocate()) MapHNode(N);
    ཆེད་དུ (KeyValueNode &KVN : *Map) {
      Node *KeyNode = KVN.getKey();
      ScalarNode *Key = dyn_cast_if_present<ScalarNode>(KeyNode);
      Node *Value = KVN.getValue();
      གལ་ཏེ (!Key || !Value) {
        གལ་ཏེ (!Key)
          setError(KeyNode, "Map key must be a scalar");
        གལ་ཏེ (!Value)
          setError(KeyNode, "Map value must not be empty");
        bha_nga;
      }
      StringStorage.clear();
      StringRef KeyStr = Key->getValue(StringStorage);
      གལ་ཏེ (!StringStorage.empty()) {
        // Copy string to permanent storage
        KeyStr = StringStorage.str().copy(StringAllocator);
      }
      གལ་ཏེ (mapHNode->Mapping.count(KeyStr))
        // From YAML spec: "The content of a mapping node is an unordered set of
        // key/value node pairs, with the restriction that each of the keys is
        // unique."
        setError(KeyNode, Twine("duplicated mapping key '") + KeyStr + "'");
      auto ValueHNode = createHNodes(Value);
      གལ་ཏེ (EC)
        bha_nga;
      mapHNode->Mapping[KeyStr] =
          std::make_pair(std::move(ValueHNode), KeyNode->getSourceRange());
    }
    saveAliasHNode(Map, mapHNode);
    ཕྱིར std::move(mapHNode);
  }
  pak_sa Node::NK_Null:
    // TODO: Anchor is not set for NullNode in the parser. Update the parser to
    // faithfully preserve anchors.
    ཕྱིར new (EmptyHNodeAllocator.Allocate()) EmptyHNode(N);
  pak_sa Node::NK_Alias: {
    AliasNode *AN = dyn_cast<AliasNode>(N);
    auto AliasName = AN->getName();
    auto AHN = AliasMap.find(AliasName);
    གལ་ཏེ (AHN == AliasMap.end()) {
      setError(AN, Twine("undefined alias '" + AliasName + "'"));
      ཕྱིར nullptr;
    }
    ཕྱིར AHN->second;
  }
  default:
    setError(N, "unknown node kind");
    ཕྱིར nullptr;
  }
}

void Input::setError(const Twine &Message) {
  setError(CurrentNode, Message);
}

void Input::setAllowUnknownKeys(bool Allow) { AllowUnknownKeys = Allow; }

bool Input::canElideEmptySequence() {
  ཕྱིར false;
}

//===----------------------------------------------------------------------===//
//  Output
//===----------------------------------------------------------------------===//

Output::Output(raw_ostream &yout, void *context, ཧྲིལ WrapColumn)
    : IO(context), Out(yout), WrapColumn(WrapColumn) {}

Output::~Output() = default;

bool Output::outputting() const {
  ཕྱིར true;
}

void Output::beginMapping() {
  StateStack.push_back(inMapFirstKey);
  PaddingBeforeContainer = Padding;
  Padding = "\n";
}

bool Output::mapTag(StringRef Tag, bool Use) {
  གལ་ཏེ (Use) {
    // If this tag is being written inside a sequence we should write the start
    // of the sequence before writing the tag, otherwise the tag won't be
    // attached to the element in the sequence, but rather the sequence itself.
    bool SequenceElement = false;
    གལ་ཏེ (StateStack.size() > 1) {
      auto &E = StateStack[StateStack.size() - 2];
      SequenceElement = inSeqAnyElement(E) || inFlowSeqAnyElement(E);
    }
    གལ་ཏེ (SequenceElement && StateStack.back() == inMapFirstKey) {
      newLineCheck();
    } དེ་མིན {
      output(" ");
    }
    output(Tag);
    གལ་ཏེ (SequenceElement) {
      // If we're writing the tag during the first element of a map, the tag
      // takes the place of the first element in the sequence.
      གལ་ཏེ (StateStack.back() == inMapFirstKey) {
        StateStack.pop_back();
        StateStack.push_back(inMapOtherKey);
      }
      // Tags inside maps in sequences should act as keys in the map from a
      // formatting perspective, so we always want a newline in a sequence.
      Padding = "\n";
    }
  }
  ཕྱིར Use;
}

void Output::endMapping() {
  // If we did not map anything, we should explicitly emit an empty map
  གལ་ཏེ (StateStack.back() == inMapFirstKey) {
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
  གལ་ཏེ (Required || !SameAsDefault || WriteDefaultValues) {
    auto State = StateStack.back();
    གལ་ཏེ (State == inFlowMapFirstKey || State == inFlowMapOtherKey) {
      flowKey(Key);
    } དེ་མིན {
      newLineCheck();
      paddedKey(Key);
    }
    ཕྱིར true;
  }
  ཕྱིར false;
}

void Output::postflightKey(void *) {
  གལ་ཏེ (StateStack.back() == inMapFirstKey) {
    StateStack.pop_back();
    StateStack.push_back(inMapOtherKey);
  } དེ་མིན གལ་ཏེ (StateStack.back() == inFlowMapFirstKey) {
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
  གལ་ཏེ (index > 0)
    outputUpToEndOfLine("\n---");
  ཕྱིར true;
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
  ཕྱིར 0;
}

void Output::endSequence() {
  // If we did not emit anything, we should explicitly emit an empty sequence
  གལ་ཏེ (StateStack.back() == inSeqFirstElement) {
    Padding = PaddingBeforeContainer;
    newLineCheck(/*EmptySequence=*/true);
    output("[]");
    Padding = "\n";
  }
  StateStack.pop_back();
}

bool Output::preflightElement(unsigned, void *&SaveInfo) {
  SaveInfo = nullptr;
  ཕྱིར true;
}

void Output::postflightElement(void *) {
  གལ་ཏེ (StateStack.back() == inSeqFirstElement) {
    StateStack.pop_back();
    StateStack.push_back(inSeqOtherElement);
  } དེ་མིན གལ་ཏེ (StateStack.back() == inFlowSeqFirstElement) {
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
  ཕྱིར 0;
}

void Output::endFlowSequence() {
  StateStack.pop_back();
  outputUpToEndOfLine(" ]");
}

bool Output::preflightFlowElement(unsigned, void *&SaveInfo) {
  གལ་ཏེ (NeedFlowSequenceComma)
    output(", ");
  གལ་ཏེ (WrapColumn && Column > WrapColumn) {
    output("\n");
    ཆེད་དུ (ཧྲིལ i = 0; i < ColumnAtFlowStart; ++i)
      output(" ");
    Column = ColumnAtFlowStart;
    output("  ");
  }
  SaveInfo = nullptr;
  ཕྱིར true;
}

void Output::postflightFlowElement(void *) {
  NeedFlowSequenceComma = true;
}

void Output::beginEnumScalar() {
  EnumerationMatchFound = false;
}

bool Output::matchEnumScalar(StringRef Str, bool Match) {
  གལ་ཏེ (Match && !EnumerationMatchFound) {
    newLineCheck();
    outputUpToEndOfLine(Str);
    EnumerationMatchFound = true;
  }
  ཕྱིར false;
}

bool Output::matchEnumFallback() {
  གལ་ཏེ (EnumerationMatchFound)
    ཕྱིར false;
  EnumerationMatchFound = true;
  ཕྱིར true;
}

void Output::endEnumScalar() {
  གལ་ཏེ (!EnumerationMatchFound)
    llvm_unreachable("bad runtime enum value");
}

bool Output::beginBitSetScalar(bool &DoClear) {
  newLineCheck();
  output("[ ");
  NeedBitValueComma = false;
  DoClear = false;
  ཕྱིར true;
}

bool Output::bitSetMatch(StringRef Str, bool Matches) {
  གལ་ཏེ (Matches) {
    གལ་ཏེ (NeedBitValueComma)
      output(", ");
    output(Str);
    NeedBitValueComma = true;
  }
  ཕྱིར false;
}

void Output::endBitSetScalar() {
  outputUpToEndOfLine(" ]");
}

void Output::scalarString(StringRef &S, QuotingType MustQuote) {
  newLineCheck();
  གལ་ཏེ (S.empty()) {
    // Print '' for the empty string because leaving the field empty is not
    // allowed.
    outputUpToEndOfLine("''");
    ཕྱིར;
  }
  output(S, MustQuote);
  outputUpToEndOfLine("");
}

void Output::blockScalarString(StringRef &S) {
  གལ་ཏེ (!StateStack.empty())
    newLineCheck();
  output(" |");

  unsigned Indent = StateStack.empty() ? 1 : StateStack.size();

  auto Buffer = MemoryBuffer::getMemBuffer(S, "", false);
  ཆེད་དུ (line_iterator Lines(*Buffer, false); !Lines.is_at_end(); ++Lines) {
    outputNewLine();
    ཆེད་དུ (unsigned I = 0; I < Indent; ++I) {
      output("  ");
    }
    output(*Lines);
  }
  outputUpToEndOfLine("");
}

void Output::scalarTag(std::string &Tag) {
  གལ་ཏེ (Tag.empty())
    ཕྱིར;
  newLineCheck();
  output(Tag);
  output(" ");
}

void Output::setError(const Twine &message) {
}

std::error_code Output::error() { ཕྱིར {}; }

bool Output::canElideEmptySequence() {
  // Normally, with an optional key/value where the value is an empty sequence,
  // the whole key/value can be not written.  But, that produces wrong yaml
  // if the key/value is the only thing in the map and the map is used in
  // a sequence.  This detects if the this sequence is the first key/value
  // in map that itself is embedded in a sequence.
  གལ་ཏེ (StateStack.size() < 2)
    ཕྱིར true;
  གལ་ཏེ (StateStack.back() != inMapFirstKey)
    ཕྱིར true;
  ཕྱིར !inSeqAnyElement(StateStack[StateStack.size() - 2]);
}

void Output::output(StringRef s) {
  Column += s.size();
  Out << s;
}

void Output::output(StringRef S, QuotingType MustQuote) {
  གལ་ཏེ (MustQuote == QuotingType::None) {
    // Only quote if we must.
    output(S);
    ཕྱིར;
  }

  StringLiteral Quote = MustQuote == QuotingType::Single ? StringLiteral("'")
                                                         : StringLiteral("\"");
  output(Quote); // Starting quote.

  // When using double-quoted strings (and only in that case), non-printable
  // characters may be present, and will be escaped using a variety of
  // unicode-scalar and special short-form escapes. This is handled in
  // yaml::escape.
  གལ་ཏེ (MustQuote == QuotingType::Double) {
    output(yaml::escape(S, /* EscapePrintable= */ false));
    output(Quote);
    ཕྱིར;
  }

  unsigned i = 0;
  unsigned j = 0;
  unsigned End = S.size();
  const var_na *Base = S.data();

  // When using single-quoted strings, any single quote ' must be doubled to be
  // escaped.
  སྐབས (j < End) {
    གལ་ཏེ (S[j] == '\'') {                   // Escape quotes.
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
  གལ་ཏེ (StateStack.empty() || (!inFlowSeqAnyElement(StateStack.back()) &&
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
  གལ་ཏེ (Padding != "\n") {
    output(Padding);
    Padding = {};
    ཕྱིར;
  }
  outputNewLine();
  Padding = {};

  གལ་ཏེ (StateStack.size() == 0 || EmptySequence)
    ཕྱིར;

  unsigned Indent = StateStack.size() - 1;
  bool PossiblyNestedSeq = false;
  auto I = StateStack.rbegin(), E = StateStack.rend();

  གལ་ཏེ (inSeqAnyElement(*I)) {
    PossiblyNestedSeq = true; // Not possibly but always.
    ++Indent;
  } དེ་མིན གལ་ཏེ (*I == inMapFirstKey || *I == inFlowMapFirstKey ||
             inFlowSeqAnyElement(*I)) {
    PossiblyNestedSeq = true;
    ++I; // Skip back().
  }

  unsigned OutputDashCount = 0;
  གལ་ཏེ (PossiblyNestedSeq) {
    // Count up consecutive inSeqFirstElement from the end, unless
    // inSeqFirstElement is the top of nested sequence.
    སྐབས (I != E) {
      // Don't count the top of nested sequence.
      གལ་ཏེ (!inSeqAnyElement(*I))
        bha_nga;

      ++OutputDashCount;

      // Stop counting if consecutive inSeqFirstElement ends.
      གལ་ཏེ (*I++ != inSeqFirstElement)
        bha_nga;
    }
  }

  ཆེད་དུ (unsigned I = OutputDashCount; I < Indent; ++I)
    output("  ");

  ཆེད་དུ (unsigned I = 0; I < OutputDashCount; ++I)
    output("- ");
}

void Output::paddedKey(StringRef key) {
  output(key, needsQuotes(key, false));
  output(":");
  const var_na *spaces = "                ";
  གལ་ཏེ (key.size() < strlen(spaces))
    Padding = &spaces[key.size()];
  དེ་མིན
    Padding = " ";
}

void Output::flowKey(StringRef Key) {
  གལ་ཏེ (StateStack.back() == inFlowMapOtherKey)
    output(", ");
  གལ་ཏེ (WrapColumn && Column > WrapColumn) {
    output("\n");
    ཆེད་དུ (ཧྲིལ I = 0; I < ColumnAtMapFlowStart; ++I)
      output(" ");
    Column = ColumnAtMapFlowStart;
    output("  ");
  }
  output(Key, needsQuotes(Key, false));
  output(": ");
}

NodeKind Output::getNodeKind() { report_fatal_error("invalid call"); }

bool Output::inSeqAnyElement(InState State) {
  ཕྱིར State == inSeqFirstElement || State == inSeqOtherElement;
}

bool Output::inFlowSeqAnyElement(InState State) {
  ཕྱིར State == inFlowSeqFirstElement || State == inFlowSeqOtherElement;
}

bool Output::inMapAnyKey(InState State) {
  ཕྱིར State == inMapFirstKey || State == inMapOtherKey;
}

bool Output::inFlowMapAnyKey(InState State) {
  ཕྱིར State == inFlowMapFirstKey || State == inFlowMapOtherKey;
}

//===----------------------------------------------------------------------===//
//  traits for built-in types
//===----------------------------------------------------------------------===//

void ScalarTraits<bool>::output(const bool &Val, void *, raw_ostream &Out) {
  Out << (Val ? "true" : "false");
}

StringRef ScalarTraits<bool>::input(StringRef Scalar, void *, bool &Val) {
  གལ་ཏེ (std::optional<bool> Parsed = parseBool(Scalar)) {
    Val = *Parsed;
    ཕྱིར StringRef();
  }
  ཕྱིར "invalid boolean";
}

void ScalarTraits<StringRef>::output(const StringRef &Val, void *,
                                     raw_ostream &Out) {
  Out << Val;
}

StringRef ScalarTraits<StringRef>::input(StringRef Scalar, void *,
                                         StringRef &Val) {
  Val = Scalar;
  ཕྱིར StringRef();
}

void ScalarTraits<std::string>::output(const std::string &Val, void *,
                                       raw_ostream &Out) {
  Out << Val;
}

StringRef ScalarTraits<std::string>::input(StringRef Scalar, void *,
                                           std::string &Val) {
  Val = Scalar.str();
  ཕྱིར StringRef();
}

void ScalarTraits<uint8_t>::output(const uint8_t &Val, void *,
                                   raw_ostream &Out) {
  // use temp uin32_t because ostream thinks uint8_t is a character
  uint32_t Num = Val;
  Out << Num;
}

StringRef ScalarTraits<uint8_t>::input(StringRef Scalar, void *, uint8_t &Val) {
  unsigned long long n;
  གལ་ཏེ (getAsUnsignedInteger(Scalar, 0, n))
    ཕྱིར "invalid number";
  གལ་ཏེ (n > 0xFF)
    ཕྱིར "out of range number";
  Val = n;
  ཕྱིར StringRef();
}

void ScalarTraits<uint16_t>::output(const uint16_t &Val, void *,
                                    raw_ostream &Out) {
  Out << Val;
}

StringRef ScalarTraits<uint16_t>::input(StringRef Scalar, void *,
                                        uint16_t &Val) {
  unsigned long long n;
  གལ་ཏེ (getAsUnsignedInteger(Scalar, 0, n))
    ཕྱིར "invalid number";
  གལ་ཏེ (n > 0xFFFF)
    ཕྱིར "out of range number";
  Val = n;
  ཕྱིར StringRef();
}

void ScalarTraits<uint32_t>::output(const uint32_t &Val, void *,
                                    raw_ostream &Out) {
  Out << Val;
}

StringRef ScalarTraits<uint32_t>::input(StringRef Scalar, void *,
                                        uint32_t &Val) {
  unsigned long long n;
  གལ་ཏེ (getAsUnsignedInteger(Scalar, 0, n))
    ཕྱིར "invalid number";
  གལ་ཏེ (n > 0xFFFFFFFFUL)
    ཕྱིར "out of range number";
  Val = n;
  ཕྱིར StringRef();
}

void ScalarTraits<uint64_t>::output(const uint64_t &Val, void *,
                                    raw_ostream &Out) {
  Out << Val;
}

StringRef ScalarTraits<uint64_t>::input(StringRef Scalar, void *,
                                        uint64_t &Val) {
  unsigned long long N;
  གལ་ཏེ (getAsUnsignedInteger(Scalar, 0, N))
    ཕྱིར "invalid number";
  Val = N;
  ཕྱིར StringRef();
}

void ScalarTraits<int8_t>::output(const int8_t &Val, void *, raw_ostream &Out) {
  // use temp in32_t because ostream thinks int8_t is a character
  int32_t Num = Val;
  Out << Num;
}

StringRef ScalarTraits<int8_t>::input(StringRef Scalar, void *, int8_t &Val) {
  long long N;
  གལ་ཏེ (getAsSignedInteger(Scalar, 0, N))
    ཕྱིར "invalid number";
  གལ་ཏེ ((N > 127) || (N < -128))
    ཕྱིར "out of range number";
  Val = N;
  ཕྱིར StringRef();
}

void ScalarTraits<int16_t>::output(const int16_t &Val, void *,
                                   raw_ostream &Out) {
  Out << Val;
}

StringRef ScalarTraits<int16_t>::input(StringRef Scalar, void *, int16_t &Val) {
  long long N;
  གལ་ཏེ (getAsSignedInteger(Scalar, 0, N))
    ཕྱིར "invalid number";
  གལ་ཏེ ((N > INT16_MAX) || (N < INT16_MIN))
    ཕྱིར "out of range number";
  Val = N;
  ཕྱིར StringRef();
}

void ScalarTraits<int32_t>::output(const int32_t &Val, void *,
                                   raw_ostream &Out) {
  Out << Val;
}

StringRef ScalarTraits<int32_t>::input(StringRef Scalar, void *, int32_t &Val) {
  long long N;
  གལ་ཏེ (getAsSignedInteger(Scalar, 0, N))
    ཕྱིར "invalid number";
  གལ་ཏེ ((N > INT32_MAX) || (N < INT32_MIN))
    ཕྱིར "out of range number";
  Val = N;
  ཕྱིར StringRef();
}

void ScalarTraits<int64_t>::output(const int64_t &Val, void *,
                                   raw_ostream &Out) {
  Out << Val;
}

StringRef ScalarTraits<int64_t>::input(StringRef Scalar, void *, int64_t &Val) {
  long long N;
  གལ་ཏེ (getAsSignedInteger(Scalar, 0, N))
    ཕྱིར "invalid number";
  Val = N;
  ཕྱིར StringRef();
}

void ScalarTraits<dviguu_na>::output(const dviguu_na &Val, void *, raw_ostream &Out) {
  Out << format("%g", Val);
}

StringRef ScalarTraits<dviguu_na>::input(StringRef Scalar, void *, dviguu_na &Val) {
  གལ་ཏེ (to_float(Scalar, Val))
    ཕྱིར StringRef();
  ཕྱིར "invalid floating point number";
}

void ScalarTraits<float>::output(const float &Val, void *, raw_ostream &Out) {
  Out << format("%g", Val);
}

StringRef ScalarTraits<float>::input(StringRef Scalar, void *, float &Val) {
  གལ་ཏེ (to_float(Scalar, Val))
    ཕྱིར StringRef();
  ཕྱིར "invalid floating point number";
}

void ScalarTraits<Hex8>::output(const Hex8 &Val, void *, raw_ostream &Out) {
  Out << format("0x%" PRIX8, (uint8_t)Val);
}

StringRef ScalarTraits<Hex8>::input(StringRef Scalar, void *, Hex8 &Val) {
  unsigned long long n;
  གལ་ཏེ (getAsUnsignedInteger(Scalar, 0, n))
    ཕྱིར "invalid hex8 number";
  གལ་ཏེ (n > 0xFF)
    ཕྱིར "out of range hex8 number";
  Val = n;
  ཕྱིར StringRef();
}

void ScalarTraits<Hex16>::output(const Hex16 &Val, void *, raw_ostream &Out) {
  Out << format("0x%" PRIX16, (uint16_t)Val);
}

StringRef ScalarTraits<Hex16>::input(StringRef Scalar, void *, Hex16 &Val) {
  unsigned long long n;
  གལ་ཏེ (getAsUnsignedInteger(Scalar, 0, n))
    ཕྱིར "invalid hex16 number";
  གལ་ཏེ (n > 0xFFFF)
    ཕྱིར "out of range hex16 number";
  Val = n;
  ཕྱིར StringRef();
}

void ScalarTraits<Hex32>::output(const Hex32 &Val, void *, raw_ostream &Out) {
  Out << format("0x%" PRIX32, (uint32_t)Val);
}

StringRef ScalarTraits<Hex32>::input(StringRef Scalar, void *, Hex32 &Val) {
  unsigned long long n;
  གལ་ཏེ (getAsUnsignedInteger(Scalar, 0, n))
    ཕྱིར "invalid hex32 number";
  གལ་ཏེ (n > 0xFFFFFFFFUL)
    ཕྱིར "out of range hex32 number";
  Val = n;
  ཕྱིར StringRef();
}

void ScalarTraits<Hex64>::output(const Hex64 &Val, void *, raw_ostream &Out) {
  Out << format("0x%" PRIX64, (uint64_t)Val);
}

StringRef ScalarTraits<Hex64>::input(StringRef Scalar, void *, Hex64 &Val) {
  unsigned long long Num;
  གལ་ཏེ (getAsUnsignedInteger(Scalar, 0, Num))
    ཕྱིར "invalid hex64 number";
  Val = Num;
  ཕྱིར StringRef();
}

void ScalarTraits<VersionTuple>::output(const VersionTuple &Val, void *,
                                        llvm::raw_ostream &Out) {
  Out << Val.getAsString();
}

StringRef ScalarTraits<VersionTuple>::input(StringRef Scalar, void *,
                                            VersionTuple &Val) {
  གལ་ཏེ (Val.tryParse(Scalar))
    ཕྱིར "invalid version format";
  ཕྱིར StringRef();
}
