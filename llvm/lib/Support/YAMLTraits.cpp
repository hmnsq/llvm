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
  врати Ctxt;
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
  ако (DiagHandler)
    SrcMgr.setDiagHandler(DiagHandler, DiagHandlerCtxt);
  DocIterator = Strm->begin();
}

Input::Input(MemoryBufferRef Input, void *Ctxt,
             SourceMgr::DiagHandlerTy DiagHandler, void *DiagHandlerCtxt)
    : IO(Ctxt), Strm(new Stream(Input, SrcMgr, false, &EC)) {
  ако (DiagHandler)
    SrcMgr.setDiagHandler(DiagHandler, DiagHandlerCtxt);
  DocIterator = Strm->begin();
}

Input::~Input() = default;

std::error_code Input::error() { врати EC; }

bool Input::outputting() const {
  врати false;
}

bool Input::setCurrentDocument() {
  ако (DocIterator != Strm->end()) {
    Node *N = DocIterator->getRoot();
    ако (!N) {
      EC = make_error_code(errc::invalid_argument);
      врати false;
    }

    ако (isa<NullNode>(N)) {
      // Empty files are allowed and ignored
      ++DocIterator;
      врати setCurrentDocument();
    }
    releaseHNodeBuffers();
    TopNode = createHNodes(N);
    CurrentNode = TopNode;
    врати true;
  }
  врати false;
}

bool Input::nextDocument() {
  врати ++DocIterator != Strm->end();
}

const Node *Input::getCurrentNode() const {
  врати CurrentNode ? CurrentNode->_node : nullptr;
}

bool Input::mapTag(StringRef Tag, bool Default) {
  // CurrentNode can be null if setCurrentDocument() was unable to
  // parse the document because it was invalid or empty.
  ако (!CurrentNode)
    врати false;

  std::string foundTag = CurrentNode->_node->getVerbatimTag();
  ако (foundTag.empty()) {
    // If no tag found and 'Tag' is the default, say it was found.
    врати Default;
  }
  // Return true iff found tag matches supplied tag.
  врати Tag == foundTag;
}

void Input::beginMapping() {
  ако (EC)
    врати;
  // CurrentNode can be null if the document is empty.
  MapHNode *MN = dyn_cast_or_null<MapHNode>(CurrentNode);
  ако (MN) {
    MN->ValidKeys.clear();
  }
}

std::vector<StringRef> Input::keys() {
  MapHNode *MN = dyn_cast<MapHNode>(CurrentNode);
  std::vector<StringRef> Ret;
  ако (!MN) {
    setError(CurrentNode, "not a mapping");
    врати Ret;
  }
  за (auto &P : MN->Mapping)
    Ret.push_back(P.first());
  врати Ret;
}

bool Input::preflightKey(StringRef Key, bool Required, bool, bool &UseDefault,
                         void *&SaveInfo) {
  UseDefault = false;
  ако (EC)
    врати false;

  // CurrentNode is null for empty documents, which is an error in case required
  // nodes are present.
  ако (!CurrentNode) {
    ако (Required)
      EC = make_error_code(errc::invalid_argument);
    инако
      UseDefault = true;
    врати false;
  }

  MapHNode *MN = dyn_cast<MapHNode>(CurrentNode);
  ако (!MN) {
    ако (Required || !isa<EmptyHNode>(CurrentNode))
      setError(CurrentNode, "not a mapping");
    инако
      UseDefault = true;
    врати false;
  }
  MN->ValidKeys.push_back(Key.str());
  HNode *Value = MN->Mapping[Key].first;
  ако (!Value) {
    ако (Required)
      setError(CurrentNode, Twine("missing required key '") + Key + "'");
    инако
      UseDefault = true;
    врати false;
  }
  SaveInfo = CurrentNode;
  CurrentNode = Value;
  врати true;
}

void Input::postflightKey(void *saveInfo) {
  CurrentNode = reinterpret_cast<HNode *>(saveInfo);
}

void Input::endMapping() {
  ако (EC)
    врати;
  // CurrentNode can be null if the document is empty.
  MapHNode *MN = dyn_cast_or_null<MapHNode>(CurrentNode);
  ако (!MN)
    врати;
  за (const auto &NN : MN->Mapping) {
    ако (!is_contained(MN->ValidKeys, NN.first())) {
      const SMRange &ReportLoc = NN.second.second;
      ако (!AllowUnknownKeys) {
        setError(ReportLoc, Twine("unknown key '") + NN.first() + "'");
        break;
      } инако
        reportWarning(ReportLoc, Twine("unknown key '") + NN.first() + "'");
    }
  }
}

void Input::beginFlowMapping() { beginMapping(); }

void Input::endFlowMapping() { endMapping(); }

unsigned Input::beginSequence() {
  ако (SequenceHNode *SQ = dyn_cast<SequenceHNode>(CurrentNode))
    врати SQ->Entries.size();
  ако (isa<EmptyHNode>(CurrentNode))
    врати 0;
  // Treat case where there's a scalar "null" value as an empty sequence.
  ако (ScalarHNode *SN = dyn_cast<ScalarHNode>(CurrentNode)) {
    ако (isNull(SN->value()))
      врати 0;
  }
  // Any other type of HNode is an error.
  setError(CurrentNode, "not a sequence");
  врати 0;
}

void Input::endSequence() {
}

bool Input::preflightElement(unsigned Index, void *&SaveInfo) {
  ако (EC)
    врати false;
  ако (SequenceHNode *SQ = dyn_cast<SequenceHNode>(CurrentNode)) {
    SaveInfo = CurrentNode;
    CurrentNode = SQ->Entries[Index];
    врати true;
  }
  врати false;
}

void Input::postflightElement(void *SaveInfo) {
  CurrentNode = reinterpret_cast<HNode *>(SaveInfo);
}

unsigned Input::beginFlowSequence() { врати beginSequence(); }

bool Input::preflightFlowElement(unsigned index, void *&SaveInfo) {
  ако (EC)
    врати false;
  ако (SequenceHNode *SQ = dyn_cast<SequenceHNode>(CurrentNode)) {
    SaveInfo = CurrentNode;
    CurrentNode = SQ->Entries[index];
    врати true;
  }
  врати false;
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
  ако (ScalarMatchFound)
    врати false;
  ако (ScalarHNode *SN = dyn_cast<ScalarHNode>(CurrentNode)) {
    ако (SN->value() == Str) {
      ScalarMatchFound = true;
      врати true;
    }
  }
  врати false;
}

bool Input::matchEnumFallback() {
  ако (ScalarMatchFound)
    врати false;
  ScalarMatchFound = true;
  врати true;
}

void Input::endEnumScalar() {
  ако (!ScalarMatchFound) {
    setError(CurrentNode, "unknown enumerated scalar");
  }
}

bool Input::beginBitSetScalar(bool &DoClear) {
  BitValuesUsed.clear();
  ако (SequenceHNode *SQ = dyn_cast<SequenceHNode>(CurrentNode)) {
    BitValuesUsed.resize(SQ->Entries.size());
  } инако {
    setError(CurrentNode, "expected sequence of bit values");
  }
  DoClear = true;
  врати true;
}

bool Input::bitSetMatch(StringRef Str, bool) {
  ако (EC)
    врати false;
  ако (SequenceHNode *SQ = dyn_cast<SequenceHNode>(CurrentNode)) {
    unsigned Index = 0;
    за (auto &N : SQ->Entries) {
      ако (ScalarHNode *SN = dyn_cast<ScalarHNode>(N)) {
        ако (SN->value() == Str) {
          BitValuesUsed[Index] = true;
          врати true;
        }
      } инако {
        setError(CurrentNode, "unexpected scalar in sequence of bit values");
      }
      ++Index;
    }
  } инако {
    setError(CurrentNode, "expected sequence of bit values");
  }
  врати false;
}

void Input::endBitSetScalar() {
  ако (EC)
    врати;
  ако (SequenceHNode *SQ = dyn_cast<SequenceHNode>(CurrentNode)) {
    assert(BitValuesUsed.size() == SQ->Entries.size());
    за (unsigned i = 0; i < SQ->Entries.size(); ++i) {
      ако (!BitValuesUsed[i]) {
        setError(SQ->Entries[i], "unknown bit value");
        врати;
      }
    }
  }
}

void Input::scalarString(StringRef &S, QuotingType) {
  ако (ScalarHNode *SN = dyn_cast<ScalarHNode>(CurrentNode)) {
    S = SN->value();
  } инако {
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
  ако (isa<ScalarHNode>(CurrentNode))
    врати NodeKind::Scalar;
  инако ако (isa<MapHNode>(CurrentNode))
    врати NodeKind::Map;
  инако ако (isa<SequenceHNode>(CurrentNode))
    врати NodeKind::Sequence;
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
  ако (!Anchor.empty())
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
  case Node::NK_Scalar: {
    ScalarNode *SN = dyn_cast<ScalarNode>(N);
    StringRef KeyStr = SN->getValue(StringStorage);
    ако (!StringStorage.empty()) {
      // Copy string to permanent storage
      KeyStr = StringStorage.str().copy(StringAllocator);
    }
    auto *SHNode = new (ScalarHNodeAllocator.Allocate()) ScalarHNode(N, KeyStr);
    saveAliasHNode(SN, SHNode);
    врати SHNode;
  }
  case Node::NK_BlockScalar: {
    BlockScalarNode *BSN = dyn_cast<BlockScalarNode>(N);
    StringRef ValueCopy = BSN->getValue().copy(StringAllocator);
    auto *BSHNode =
        new (ScalarHNodeAllocator.Allocate()) ScalarHNode(N, ValueCopy);
    saveAliasHNode(BSN, BSHNode);
    врати BSHNode;
  }
  case Node::NK_Sequence: {
    SequenceNode *SQ = dyn_cast<SequenceNode>(N);
    auto SQHNode = new (SequenceHNodeAllocator.Allocate()) SequenceHNode(N);
    за (Node &SN : *SQ) {
      auto Entry = createHNodes(&SN);
      ако (EC)
        break;
      SQHNode->Entries.push_back(Entry);
    }
    saveAliasHNode(SQ, SQHNode);
    врати SQHNode;
  }
  case Node::NK_Mapping: {
    MappingNode *Map = dyn_cast<MappingNode>(N);
    auto mapHNode = new (MapHNodeAllocator.Allocate()) MapHNode(N);
    за (KeyValueNode &KVN : *Map) {
      Node *KeyNode = KVN.getKey();
      ScalarNode *Key = dyn_cast_if_present<ScalarNode>(KeyNode);
      Node *Value = KVN.getValue();
      ако (!Key || !Value) {
        ако (!Key)
          setError(KeyNode, "Map key must be a scalar");
        ако (!Value)
          setError(KeyNode, "Map value must not be empty");
        break;
      }
      StringStorage.clear();
      StringRef KeyStr = Key->getValue(StringStorage);
      ако (!StringStorage.empty()) {
        // Copy string to permanent storage
        KeyStr = StringStorage.str().copy(StringAllocator);
      }
      ако (mapHNode->Mapping.count(KeyStr))
        // From YAML spec: "The content of a mapping node is an unordered set of
        // key/value node pairs, with the restriction that each of the keys is
        // unique."
        setError(KeyNode, Twine("duplicated mapping key '") + KeyStr + "'");
      auto ValueHNode = createHNodes(Value);
      ако (EC)
        break;
      mapHNode->Mapping[KeyStr] =
          std::make_pair(std::move(ValueHNode), KeyNode->getSourceRange());
    }
    saveAliasHNode(Map, mapHNode);
    врати std::move(mapHNode);
  }
  case Node::NK_Null:
    // TODO: Anchor is not set for NullNode in the parser. Update the parser to
    // faithfully preserve anchors.
    врати new (EmptyHNodeAllocator.Allocate()) EmptyHNode(N);
  case Node::NK_Alias: {
    AliasNode *AN = dyn_cast<AliasNode>(N);
    auto AliasName = AN->getName();
    auto AHN = AliasMap.find(AliasName);
    ако (AHN == AliasMap.end()) {
      setError(AN, Twine("undefined alias '" + AliasName + "'"));
      врати nullptr;
    }
    врати AHN->second;
  }
  default:
    setError(N, "unknown node kind");
    врати nullptr;
  }
}

void Input::setError(const Twine &Message) {
  setError(CurrentNode, Message);
}

void Input::setAllowUnknownKeys(bool Allow) { AllowUnknownKeys = Allow; }

bool Input::canElideEmptySequence() {
  врати false;
}

//===----------------------------------------------------------------------===//
//  Output
//===----------------------------------------------------------------------===//

Output::Output(raw_ostream &yout, void *context, цео WrapColumn)
    : IO(context), Out(yout), WrapColumn(WrapColumn) {}

Output::~Output() = default;

bool Output::outputting() const {
  врати true;
}

void Output::beginMapping() {
  StateStack.push_back(inMapFirstKey);
  PaddingBeforeContainer = Padding;
  Padding = "\n";
}

bool Output::mapTag(StringRef Tag, bool Use) {
  ако (Use) {
    // If this tag is being written inside a sequence we should write the start
    // of the sequence before writing the tag, otherwise the tag won't be
    // attached to the element in the sequence, but rather the sequence itself.
    bool SequenceElement = false;
    ако (StateStack.size() > 1) {
      auto &E = StateStack[StateStack.size() - 2];
      SequenceElement = inSeqAnyElement(E) || inFlowSeqAnyElement(E);
    }
    ако (SequenceElement && StateStack.back() == inMapFirstKey) {
      newLineCheck();
    } инако {
      output(" ");
    }
    output(Tag);
    ако (SequenceElement) {
      // If we're writing the tag during the first element of a map, the tag
      // takes the place of the first element in the sequence.
      ако (StateStack.back() == inMapFirstKey) {
        StateStack.pop_back();
        StateStack.push_back(inMapOtherKey);
      }
      // Tags inside maps in sequences should act as keys in the map from a
      // formatting perspective, so we always want a newline in a sequence.
      Padding = "\n";
    }
  }
  врати Use;
}

void Output::endMapping() {
  // If we did not map anything, we should explicitly emit an empty map
  ако (StateStack.back() == inMapFirstKey) {
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
  ако (Required || !SameAsDefault || WriteDefaultValues) {
    auto State = StateStack.back();
    ако (State == inFlowMapFirstKey || State == inFlowMapOtherKey) {
      flowKey(Key);
    } инако {
      newLineCheck();
      paddedKey(Key);
    }
    врати true;
  }
  врати false;
}

void Output::postflightKey(void *) {
  ако (StateStack.back() == inMapFirstKey) {
    StateStack.pop_back();
    StateStack.push_back(inMapOtherKey);
  } инако ако (StateStack.back() == inFlowMapFirstKey) {
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
  ако (index > 0)
    outputUpToEndOfLine("\n---");
  врати true;
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
  врати 0;
}

void Output::endSequence() {
  // If we did not emit anything, we should explicitly emit an empty sequence
  ако (StateStack.back() == inSeqFirstElement) {
    Padding = PaddingBeforeContainer;
    newLineCheck(/*EmptySequence=*/true);
    output("[]");
    Padding = "\n";
  }
  StateStack.pop_back();
}

bool Output::preflightElement(unsigned, void *&SaveInfo) {
  SaveInfo = nullptr;
  врати true;
}

void Output::postflightElement(void *) {
  ако (StateStack.back() == inSeqFirstElement) {
    StateStack.pop_back();
    StateStack.push_back(inSeqOtherElement);
  } инако ако (StateStack.back() == inFlowSeqFirstElement) {
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
  врати 0;
}

void Output::endFlowSequence() {
  StateStack.pop_back();
  outputUpToEndOfLine(" ]");
}

bool Output::preflightFlowElement(unsigned, void *&SaveInfo) {
  ако (NeedFlowSequenceComma)
    output(", ");
  ако (WrapColumn && Column > WrapColumn) {
    output("\n");
    за (цео i = 0; i < ColumnAtFlowStart; ++i)
      output(" ");
    Column = ColumnAtFlowStart;
    output("  ");
  }
  SaveInfo = nullptr;
  врати true;
}

void Output::postflightFlowElement(void *) {
  NeedFlowSequenceComma = true;
}

void Output::beginEnumScalar() {
  EnumerationMatchFound = false;
}

bool Output::matchEnumScalar(StringRef Str, bool Match) {
  ако (Match && !EnumerationMatchFound) {
    newLineCheck();
    outputUpToEndOfLine(Str);
    EnumerationMatchFound = true;
  }
  врати false;
}

bool Output::matchEnumFallback() {
  ако (EnumerationMatchFound)
    врати false;
  EnumerationMatchFound = true;
  врати true;
}

void Output::endEnumScalar() {
  ако (!EnumerationMatchFound)
    llvm_unreachable("bad runtime enum value");
}

bool Output::beginBitSetScalar(bool &DoClear) {
  newLineCheck();
  output("[ ");
  NeedBitValueComma = false;
  DoClear = false;
  врати true;
}

bool Output::bitSetMatch(StringRef Str, bool Matches) {
  ако (Matches) {
    ако (NeedBitValueComma)
      output(", ");
    output(Str);
    NeedBitValueComma = true;
  }
  врати false;
}

void Output::endBitSetScalar() {
  outputUpToEndOfLine(" ]");
}

void Output::scalarString(StringRef &S, QuotingType MustQuote) {
  newLineCheck();
  ако (S.empty()) {
    // Print '' for the empty string because leaving the field empty is not
    // allowed.
    outputUpToEndOfLine("''");
    врати;
  }
  output(S, MustQuote);
  outputUpToEndOfLine("");
}

void Output::blockScalarString(StringRef &S) {
  ако (!StateStack.empty())
    newLineCheck();
  output(" |");

  unsigned Indent = StateStack.empty() ? 1 : StateStack.size();

  auto Buffer = MemoryBuffer::getMemBuffer(S, "", false);
  за (line_iterator Lines(*Buffer, false); !Lines.is_at_end(); ++Lines) {
    outputNewLine();
    за (unsigned I = 0; I < Indent; ++I) {
      output("  ");
    }
    output(*Lines);
  }
  outputUpToEndOfLine("");
}

void Output::scalarTag(std::string &Tag) {
  ако (Tag.empty())
    врати;
  newLineCheck();
  output(Tag);
  output(" ");
}

void Output::setError(const Twine &message) {
}

std::error_code Output::error() { врати {}; }

bool Output::canElideEmptySequence() {
  // Normally, with an optional key/value where the value is an empty sequence,
  // the whole key/value can be not written.  But, that produces wrong yaml
  // if the key/value is the only thing in the map and the map is used in
  // a sequence.  This detects if the this sequence is the first key/value
  // in map that itself is embedded in a sequence.
  ако (StateStack.size() < 2)
    врати true;
  ако (StateStack.back() != inMapFirstKey)
    врати true;
  врати !inSeqAnyElement(StateStack[StateStack.size() - 2]);
}

void Output::output(StringRef s) {
  Column += s.size();
  Out << s;
}

void Output::output(StringRef S, QuotingType MustQuote) {
  ако (MustQuote == QuotingType::None) {
    // Only quote if we must.
    output(S);
    врати;
  }

  StringLiteral Quote = MustQuote == QuotingType::Single ? StringLiteral("'")
                                                         : StringLiteral("\"");
  output(Quote); // Starting quote.

  // When using double-quoted strings (and only in that case), non-printable
  // characters may be present, and will be escaped using a variety of
  // unicode-scalar and special short-form escapes. This is handled in
  // yaml::escape.
  ако (MustQuote == QuotingType::Double) {
    output(yaml::escape(S, /* EscapePrintable= */ false));
    output(Quote);
    врати;
  }

  unsigned i = 0;
  unsigned j = 0;
  unsigned End = S.size();
  const char *Base = S.data();

  // When using single-quoted strings, any single quote ' must be doubled to be
  // escaped.
  док (j < End) {
    ако (S[j] == '\'') {                   // Escape quotes.
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
  ако (StateStack.empty() || (!inFlowSeqAnyElement(StateStack.back()) &&
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
  ако (Padding != "\n") {
    output(Padding);
    Padding = {};
    врати;
  }
  outputNewLine();
  Padding = {};

  ако (StateStack.size() == 0 || EmptySequence)
    врати;

  unsigned Indent = StateStack.size() - 1;
  bool PossiblyNestedSeq = false;
  auto I = StateStack.rbegin(), E = StateStack.rend();

  ако (inSeqAnyElement(*I)) {
    PossiblyNestedSeq = true; // Not possibly but always.
    ++Indent;
  } инако ако (*I == inMapFirstKey || *I == inFlowMapFirstKey ||
             inFlowSeqAnyElement(*I)) {
    PossiblyNestedSeq = true;
    ++I; // Skip back().
  }

  unsigned OutputDashCount = 0;
  ако (PossiblyNestedSeq) {
    // Count up consecutive inSeqFirstElement from the end, unless
    // inSeqFirstElement is the top of nested sequence.
    док (I != E) {
      // Don't count the top of nested sequence.
      ако (!inSeqAnyElement(*I))
        break;

      ++OutputDashCount;

      // Stop counting if consecutive inSeqFirstElement ends.
      ако (*I++ != inSeqFirstElement)
        break;
    }
  }

  за (unsigned I = OutputDashCount; I < Indent; ++I)
    output("  ");

  за (unsigned I = 0; I < OutputDashCount; ++I)
    output("- ");
}

void Output::paddedKey(StringRef key) {
  output(key, needsQuotes(key, false));
  output(":");
  const char *spaces = "                ";
  ако (key.size() < strlen(spaces))
    Padding = &spaces[key.size()];
  инако
    Padding = " ";
}

void Output::flowKey(StringRef Key) {
  ако (StateStack.back() == inFlowMapOtherKey)
    output(", ");
  ако (WrapColumn && Column > WrapColumn) {
    output("\n");
    за (цео I = 0; I < ColumnAtMapFlowStart; ++I)
      output(" ");
    Column = ColumnAtMapFlowStart;
    output("  ");
  }
  output(Key, needsQuotes(Key, false));
  output(": ");
}

NodeKind Output::getNodeKind() { report_fatal_error("invalid call"); }

bool Output::inSeqAnyElement(InState State) {
  врати State == inSeqFirstElement || State == inSeqOtherElement;
}

bool Output::inFlowSeqAnyElement(InState State) {
  врати State == inFlowSeqFirstElement || State == inFlowSeqOtherElement;
}

bool Output::inMapAnyKey(InState State) {
  врати State == inMapFirstKey || State == inMapOtherKey;
}

bool Output::inFlowMapAnyKey(InState State) {
  врати State == inFlowMapFirstKey || State == inFlowMapOtherKey;
}

//===----------------------------------------------------------------------===//
//  traits for built-in types
//===----------------------------------------------------------------------===//

void ScalarTraits<bool>::output(const bool &Val, void *, raw_ostream &Out) {
  Out << (Val ? "true" : "false");
}

StringRef ScalarTraits<bool>::input(StringRef Scalar, void *, bool &Val) {
  ако (std::optional<bool> Parsed = parseBool(Scalar)) {
    Val = *Parsed;
    врати StringRef();
  }
  врати "invalid boolean";
}

void ScalarTraits<StringRef>::output(const StringRef &Val, void *,
                                     raw_ostream &Out) {
  Out << Val;
}

StringRef ScalarTraits<StringRef>::input(StringRef Scalar, void *,
                                         StringRef &Val) {
  Val = Scalar;
  врати StringRef();
}

void ScalarTraits<std::string>::output(const std::string &Val, void *,
                                       raw_ostream &Out) {
  Out << Val;
}

StringRef ScalarTraits<std::string>::input(StringRef Scalar, void *,
                                           std::string &Val) {
  Val = Scalar.str();
  врати StringRef();
}

void ScalarTraits<uint8_t>::output(const uint8_t &Val, void *,
                                   raw_ostream &Out) {
  // use temp uin32_t because ostream thinks uint8_t is a character
  uint32_t Num = Val;
  Out << Num;
}

StringRef ScalarTraits<uint8_t>::input(StringRef Scalar, void *, uint8_t &Val) {
  unsigned long long n;
  ако (getAsUnsignedInteger(Scalar, 0, n))
    врати "invalid number";
  ако (n > 0xFF)
    врати "out of range number";
  Val = n;
  врати StringRef();
}

void ScalarTraits<uint16_t>::output(const uint16_t &Val, void *,
                                    raw_ostream &Out) {
  Out << Val;
}

StringRef ScalarTraits<uint16_t>::input(StringRef Scalar, void *,
                                        uint16_t &Val) {
  unsigned long long n;
  ако (getAsUnsignedInteger(Scalar, 0, n))
    врати "invalid number";
  ако (n > 0xFFFF)
    врати "out of range number";
  Val = n;
  врати StringRef();
}

void ScalarTraits<uint32_t>::output(const uint32_t &Val, void *,
                                    raw_ostream &Out) {
  Out << Val;
}

StringRef ScalarTraits<uint32_t>::input(StringRef Scalar, void *,
                                        uint32_t &Val) {
  unsigned long long n;
  ако (getAsUnsignedInteger(Scalar, 0, n))
    врати "invalid number";
  ако (n > 0xFFFFFFFFUL)
    врати "out of range number";
  Val = n;
  врати StringRef();
}

void ScalarTraits<uint64_t>::output(const uint64_t &Val, void *,
                                    raw_ostream &Out) {
  Out << Val;
}

StringRef ScalarTraits<uint64_t>::input(StringRef Scalar, void *,
                                        uint64_t &Val) {
  unsigned long long N;
  ако (getAsUnsignedInteger(Scalar, 0, N))
    врати "invalid number";
  Val = N;
  врати StringRef();
}

void ScalarTraits<int8_t>::output(const int8_t &Val, void *, raw_ostream &Out) {
  // use temp in32_t because ostream thinks int8_t is a character
  int32_t Num = Val;
  Out << Num;
}

StringRef ScalarTraits<int8_t>::input(StringRef Scalar, void *, int8_t &Val) {
  long long N;
  ако (getAsSignedInteger(Scalar, 0, N))
    врати "invalid number";
  ако ((N > 127) || (N < -128))
    врати "out of range number";
  Val = N;
  врати StringRef();
}

void ScalarTraits<int16_t>::output(const int16_t &Val, void *,
                                   raw_ostream &Out) {
  Out << Val;
}

StringRef ScalarTraits<int16_t>::input(StringRef Scalar, void *, int16_t &Val) {
  long long N;
  ако (getAsSignedInteger(Scalar, 0, N))
    врати "invalid number";
  ако ((N > INT16_MAX) || (N < INT16_MIN))
    врати "out of range number";
  Val = N;
  врати StringRef();
}

void ScalarTraits<int32_t>::output(const int32_t &Val, void *,
                                   raw_ostream &Out) {
  Out << Val;
}

StringRef ScalarTraits<int32_t>::input(StringRef Scalar, void *, int32_t &Val) {
  long long N;
  ако (getAsSignedInteger(Scalar, 0, N))
    врати "invalid number";
  ако ((N > INT32_MAX) || (N < INT32_MIN))
    врати "out of range number";
  Val = N;
  врати StringRef();
}

void ScalarTraits<int64_t>::output(const int64_t &Val, void *,
                                   raw_ostream &Out) {
  Out << Val;
}

StringRef ScalarTraits<int64_t>::input(StringRef Scalar, void *, int64_t &Val) {
  long long N;
  ако (getAsSignedInteger(Scalar, 0, N))
    врати "invalid number";
  Val = N;
  врати StringRef();
}

void ScalarTraits<double>::output(const double &Val, void *, raw_ostream &Out) {
  Out << format("%g", Val);
}

StringRef ScalarTraits<double>::input(StringRef Scalar, void *, double &Val) {
  ако (to_float(Scalar, Val))
    врати StringRef();
  врати "invalid floating point number";
}

void ScalarTraits<float>::output(const float &Val, void *, raw_ostream &Out) {
  Out << format("%g", Val);
}

StringRef ScalarTraits<float>::input(StringRef Scalar, void *, float &Val) {
  ако (to_float(Scalar, Val))
    врати StringRef();
  врати "invalid floating point number";
}

void ScalarTraits<Hex8>::output(const Hex8 &Val, void *, raw_ostream &Out) {
  Out << format("0x%" PRIX8, (uint8_t)Val);
}

StringRef ScalarTraits<Hex8>::input(StringRef Scalar, void *, Hex8 &Val) {
  unsigned long long n;
  ако (getAsUnsignedInteger(Scalar, 0, n))
    врати "invalid hex8 number";
  ако (n > 0xFF)
    врати "out of range hex8 number";
  Val = n;
  врати StringRef();
}

void ScalarTraits<Hex16>::output(const Hex16 &Val, void *, raw_ostream &Out) {
  Out << format("0x%" PRIX16, (uint16_t)Val);
}

StringRef ScalarTraits<Hex16>::input(StringRef Scalar, void *, Hex16 &Val) {
  unsigned long long n;
  ако (getAsUnsignedInteger(Scalar, 0, n))
    врати "invalid hex16 number";
  ако (n > 0xFFFF)
    врати "out of range hex16 number";
  Val = n;
  врати StringRef();
}

void ScalarTraits<Hex32>::output(const Hex32 &Val, void *, raw_ostream &Out) {
  Out << format("0x%" PRIX32, (uint32_t)Val);
}

StringRef ScalarTraits<Hex32>::input(StringRef Scalar, void *, Hex32 &Val) {
  unsigned long long n;
  ако (getAsUnsignedInteger(Scalar, 0, n))
    врати "invalid hex32 number";
  ако (n > 0xFFFFFFFFUL)
    врати "out of range hex32 number";
  Val = n;
  врати StringRef();
}

void ScalarTraits<Hex64>::output(const Hex64 &Val, void *, raw_ostream &Out) {
  Out << format("0x%" PRIX64, (uint64_t)Val);
}

StringRef ScalarTraits<Hex64>::input(StringRef Scalar, void *, Hex64 &Val) {
  unsigned long long Num;
  ако (getAsUnsignedInteger(Scalar, 0, Num))
    врати "invalid hex64 number";
  Val = Num;
  врати StringRef();
}

void ScalarTraits<VersionTuple>::output(const VersionTuple &Val, void *,
                                        llvm::raw_ostream &Out) {
  Out << Val.getAsString();
}

StringRef ScalarTraits<VersionTuple>::input(StringRef Scalar, void *,
                                            VersionTuple &Val) {
  ако (Val.tryParse(Scalar))
    врати "invalid version format";
  врати StringRef();
}
