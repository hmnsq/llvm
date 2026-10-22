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
  返回 Ctxt;
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
  如果 (DiagHandler)
    SrcMgr.setDiagHandler(DiagHandler, DiagHandlerCtxt);
  DocIterator = Strm->begin();
}

Input::Input(MemoryBufferRef Input, void *Ctxt,
             SourceMgr::DiagHandlerTy DiagHandler, void *DiagHandlerCtxt)
    : IO(Ctxt), Strm(new Stream(Input, SrcMgr, false, &EC)) {
  如果 (DiagHandler)
    SrcMgr.setDiagHandler(DiagHandler, DiagHandlerCtxt);
  DocIterator = Strm->begin();
}

Input::~Input() = default;

std::error_code Input::error() { 返回 EC; }

bool Input::outputting() const {
  返回 false;
}

bool Input::setCurrentDocument() {
  如果 (DocIterator != Strm->end()) {
    Node *N = DocIterator->getRoot();
    如果 (!N) {
      EC = make_error_code(errc::invalid_argument);
      返回 false;
    }

    如果 (isa<NullNode>(N)) {
      // Empty files are allowed and ignored
      ++DocIterator;
      返回 setCurrentDocument();
    }
    releaseHNodeBuffers();
    TopNode = createHNodes(N);
    CurrentNode = TopNode;
    返回 true;
  }
  返回 false;
}

bool Input::nextDocument() {
  返回 ++DocIterator != Strm->end();
}

const Node *Input::getCurrentNode() const {
  返回 CurrentNode ? CurrentNode->_node : nullptr;
}

bool Input::mapTag(StringRef Tag, bool Default) {
  // CurrentNode can be null if setCurrentDocument() was unable to
  // parse the document because it was invalid or empty.
  如果 (!CurrentNode)
    返回 false;

  std::string foundTag = CurrentNode->_node->getVerbatimTag();
  如果 (foundTag.empty()) {
    // If no tag found and 'Tag' is the default, say it was found.
    返回 Default;
  }
  // Return true iff found tag matches supplied tag.
  返回 Tag == foundTag;
}

void Input::beginMapping() {
  如果 (EC)
    返回;
  // CurrentNode can be null if the document is empty.
  MapHNode *MN = dyn_cast_or_null<MapHNode>(CurrentNode);
  如果 (MN) {
    MN->ValidKeys.clear();
  }
}

std::vector<StringRef> Input::keys() {
  MapHNode *MN = dyn_cast<MapHNode>(CurrentNode);
  std::vector<StringRef> Ret;
  如果 (!MN) {
    setError(CurrentNode, "not a mapping");
    返回 Ret;
  }
  为 (auto &P : MN->Mapping)
    Ret.push_back(P.first());
  返回 Ret;
}

bool Input::preflightKey(StringRef Key, bool Required, bool, bool &UseDefault,
                         void *&SaveInfo) {
  UseDefault = false;
  如果 (EC)
    返回 false;

  // CurrentNode is null for empty documents, which is an error in case required
  // nodes are present.
  如果 (!CurrentNode) {
    如果 (Required)
      EC = make_error_code(errc::invalid_argument);
    否则
      UseDefault = true;
    返回 false;
  }

  MapHNode *MN = dyn_cast<MapHNode>(CurrentNode);
  如果 (!MN) {
    如果 (Required || !isa<EmptyHNode>(CurrentNode))
      setError(CurrentNode, "not a mapping");
    否则
      UseDefault = true;
    返回 false;
  }
  MN->ValidKeys.push_back(Key.str());
  HNode *Value = MN->Mapping[Key].first;
  如果 (!Value) {
    如果 (Required)
      setError(CurrentNode, Twine("missing required key '") + Key + "'");
    否则
      UseDefault = true;
    返回 false;
  }
  SaveInfo = CurrentNode;
  CurrentNode = Value;
  返回 true;
}

void Input::postflightKey(void *saveInfo) {
  CurrentNode = reinterpret_cast<HNode *>(saveInfo);
}

void Input::endMapping() {
  如果 (EC)
    返回;
  // CurrentNode can be null if the document is empty.
  MapHNode *MN = dyn_cast_or_null<MapHNode>(CurrentNode);
  如果 (!MN)
    返回;
  为 (const auto &NN : MN->Mapping) {
    如果 (!is_contained(MN->ValidKeys, NN.first())) {
      const SMRange &ReportLoc = NN.second.second;
      如果 (!AllowUnknownKeys) {
        setError(ReportLoc, Twine("unknown key '") + NN.first() + "'");
        break;
      } 否则
        reportWarning(ReportLoc, Twine("unknown key '") + NN.first() + "'");
    }
  }
}

void Input::beginFlowMapping() { beginMapping(); }

void Input::endFlowMapping() { endMapping(); }

unsigned Input::beginSequence() {
  如果 (SequenceHNode *SQ = dyn_cast<SequenceHNode>(CurrentNode))
    返回 SQ->Entries.size();
  如果 (isa<EmptyHNode>(CurrentNode))
    返回 0;
  // Treat case where there's a scalar "null" value as an empty sequence.
  如果 (ScalarHNode *SN = dyn_cast<ScalarHNode>(CurrentNode)) {
    如果 (isNull(SN->value()))
      返回 0;
  }
  // Any other type of HNode is an error.
  setError(CurrentNode, "not a sequence");
  返回 0;
}

void Input::endSequence() {
}

bool Input::preflightElement(unsigned Index, void *&SaveInfo) {
  如果 (EC)
    返回 false;
  如果 (SequenceHNode *SQ = dyn_cast<SequenceHNode>(CurrentNode)) {
    SaveInfo = CurrentNode;
    CurrentNode = SQ->Entries[Index];
    返回 true;
  }
  返回 false;
}

void Input::postflightElement(void *SaveInfo) {
  CurrentNode = reinterpret_cast<HNode *>(SaveInfo);
}

unsigned Input::beginFlowSequence() { 返回 beginSequence(); }

bool Input::preflightFlowElement(unsigned index, void *&SaveInfo) {
  如果 (EC)
    返回 false;
  如果 (SequenceHNode *SQ = dyn_cast<SequenceHNode>(CurrentNode)) {
    SaveInfo = CurrentNode;
    CurrentNode = SQ->Entries[index];
    返回 true;
  }
  返回 false;
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
  如果 (ScalarMatchFound)
    返回 false;
  如果 (ScalarHNode *SN = dyn_cast<ScalarHNode>(CurrentNode)) {
    如果 (SN->value() == Str) {
      ScalarMatchFound = true;
      返回 true;
    }
  }
  返回 false;
}

bool Input::matchEnumFallback() {
  如果 (ScalarMatchFound)
    返回 false;
  ScalarMatchFound = true;
  返回 true;
}

void Input::endEnumScalar() {
  如果 (!ScalarMatchFound) {
    setError(CurrentNode, "unknown enumerated scalar");
  }
}

bool Input::beginBitSetScalar(bool &DoClear) {
  BitValuesUsed.clear();
  如果 (SequenceHNode *SQ = dyn_cast<SequenceHNode>(CurrentNode)) {
    BitValuesUsed.resize(SQ->Entries.size());
  } 否则 {
    setError(CurrentNode, "expected sequence of bit values");
  }
  DoClear = true;
  返回 true;
}

bool Input::bitSetMatch(StringRef Str, bool) {
  如果 (EC)
    返回 false;
  如果 (SequenceHNode *SQ = dyn_cast<SequenceHNode>(CurrentNode)) {
    unsigned Index = 0;
    为 (auto &N : SQ->Entries) {
      如果 (ScalarHNode *SN = dyn_cast<ScalarHNode>(N)) {
        如果 (SN->value() == Str) {
          BitValuesUsed[Index] = true;
          返回 true;
        }
      } 否则 {
        setError(CurrentNode, "unexpected scalar in sequence of bit values");
      }
      ++Index;
    }
  } 否则 {
    setError(CurrentNode, "expected sequence of bit values");
  }
  返回 false;
}

void Input::endBitSetScalar() {
  如果 (EC)
    返回;
  如果 (SequenceHNode *SQ = dyn_cast<SequenceHNode>(CurrentNode)) {
    assert(BitValuesUsed.size() == SQ->Entries.size());
    为 (unsigned i = 0; i < SQ->Entries.size(); ++i) {
      如果 (!BitValuesUsed[i]) {
        setError(SQ->Entries[i], "unknown bit value");
        返回;
      }
    }
  }
}

void Input::scalarString(StringRef &S, QuotingType) {
  如果 (ScalarHNode *SN = dyn_cast<ScalarHNode>(CurrentNode)) {
    S = SN->value();
  } 否则 {
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
  如果 (isa<ScalarHNode>(CurrentNode))
    返回 NodeKind::Scalar;
  否则 如果 (isa<MapHNode>(CurrentNode))
    返回 NodeKind::Map;
  否则 如果 (isa<SequenceHNode>(CurrentNode))
    返回 NodeKind::Sequence;
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
  如果 (!Anchor.empty())
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
    如果 (!StringStorage.empty()) {
      // Copy string to permanent storage
      KeyStr = StringStorage.str().copy(StringAllocator);
    }
    auto *SHNode = new (ScalarHNodeAllocator.Allocate()) ScalarHNode(N, KeyStr);
    saveAliasHNode(SN, SHNode);
    返回 SHNode;
  }
  case Node::NK_BlockScalar: {
    BlockScalarNode *BSN = dyn_cast<BlockScalarNode>(N);
    StringRef ValueCopy = BSN->getValue().copy(StringAllocator);
    auto *BSHNode =
        new (ScalarHNodeAllocator.Allocate()) ScalarHNode(N, ValueCopy);
    saveAliasHNode(BSN, BSHNode);
    返回 BSHNode;
  }
  case Node::NK_Sequence: {
    SequenceNode *SQ = dyn_cast<SequenceNode>(N);
    auto SQHNode = new (SequenceHNodeAllocator.Allocate()) SequenceHNode(N);
    为 (Node &SN : *SQ) {
      auto Entry = createHNodes(&SN);
      如果 (EC)
        break;
      SQHNode->Entries.push_back(Entry);
    }
    saveAliasHNode(SQ, SQHNode);
    返回 SQHNode;
  }
  case Node::NK_Mapping: {
    MappingNode *Map = dyn_cast<MappingNode>(N);
    auto mapHNode = new (MapHNodeAllocator.Allocate()) MapHNode(N);
    为 (KeyValueNode &KVN : *Map) {
      Node *KeyNode = KVN.getKey();
      ScalarNode *Key = dyn_cast_if_present<ScalarNode>(KeyNode);
      Node *Value = KVN.getValue();
      如果 (!Key || !Value) {
        如果 (!Key)
          setError(KeyNode, "Map key must be a scalar");
        如果 (!Value)
          setError(KeyNode, "Map value must not be empty");
        break;
      }
      StringStorage.clear();
      StringRef KeyStr = Key->getValue(StringStorage);
      如果 (!StringStorage.empty()) {
        // Copy string to permanent storage
        KeyStr = StringStorage.str().copy(StringAllocator);
      }
      如果 (mapHNode->Mapping.count(KeyStr))
        // From YAML spec: "The content of a mapping node is an unordered set of
        // key/value node pairs, with the restriction that each of the keys is
        // unique."
        setError(KeyNode, Twine("duplicated mapping key '") + KeyStr + "'");
      auto ValueHNode = createHNodes(Value);
      如果 (EC)
        break;
      mapHNode->Mapping[KeyStr] =
          std::make_pair(std::move(ValueHNode), KeyNode->getSourceRange());
    }
    saveAliasHNode(Map, mapHNode);
    返回 std::move(mapHNode);
  }
  case Node::NK_Null:
    // TODO: Anchor is not set for NullNode in the parser. Update the parser to
    // faithfully preserve anchors.
    返回 new (EmptyHNodeAllocator.Allocate()) EmptyHNode(N);
  case Node::NK_Alias: {
    AliasNode *AN = dyn_cast<AliasNode>(N);
    auto AliasName = AN->getName();
    auto AHN = AliasMap.find(AliasName);
    如果 (AHN == AliasMap.end()) {
      setError(AN, Twine("undefined alias '" + AliasName + "'"));
      返回 nullptr;
    }
    返回 AHN->second;
  }
  default:
    setError(N, "unknown node kind");
    返回 nullptr;
  }
}

void Input::setError(const Twine &Message) {
  setError(CurrentNode, Message);
}

void Input::setAllowUnknownKeys(bool Allow) { AllowUnknownKeys = Allow; }

bool Input::canElideEmptySequence() {
  返回 false;
}

//===----------------------------------------------------------------------===//
//  Output
//===----------------------------------------------------------------------===//

Output::Output(raw_ostream &yout, void *context, 整数 WrapColumn)
    : IO(context), Out(yout), WrapColumn(WrapColumn) {}

Output::~Output() = default;

bool Output::outputting() const {
  返回 true;
}

void Output::beginMapping() {
  StateStack.push_back(inMapFirstKey);
  PaddingBeforeContainer = Padding;
  Padding = "\n";
}

bool Output::mapTag(StringRef Tag, bool Use) {
  如果 (Use) {
    // If this tag is being written inside a sequence we should write the start
    // of the sequence before writing the tag, otherwise the tag won't be
    // attached to the element in the sequence, but rather the sequence itself.
    bool SequenceElement = false;
    如果 (StateStack.size() > 1) {
      auto &E = StateStack[StateStack.size() - 2];
      SequenceElement = inSeqAnyElement(E) || inFlowSeqAnyElement(E);
    }
    如果 (SequenceElement && StateStack.back() == inMapFirstKey) {
      newLineCheck();
    } 否则 {
      output(" ");
    }
    output(Tag);
    如果 (SequenceElement) {
      // If we're writing the tag during the first element of a map, the tag
      // takes the place of the first element in the sequence.
      如果 (StateStack.back() == inMapFirstKey) {
        StateStack.pop_back();
        StateStack.push_back(inMapOtherKey);
      }
      // Tags inside maps in sequences should act as keys in the map from a
      // formatting perspective, so we always want a newline in a sequence.
      Padding = "\n";
    }
  }
  返回 Use;
}

void Output::endMapping() {
  // If we did not map anything, we should explicitly emit an empty map
  如果 (StateStack.back() == inMapFirstKey) {
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
  如果 (Required || !SameAsDefault || WriteDefaultValues) {
    auto State = StateStack.back();
    如果 (State == inFlowMapFirstKey || State == inFlowMapOtherKey) {
      flowKey(Key);
    } 否则 {
      newLineCheck();
      paddedKey(Key);
    }
    返回 true;
  }
  返回 false;
}

void Output::postflightKey(void *) {
  如果 (StateStack.back() == inMapFirstKey) {
    StateStack.pop_back();
    StateStack.push_back(inMapOtherKey);
  } 否则 如果 (StateStack.back() == inFlowMapFirstKey) {
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
  如果 (index > 0)
    outputUpToEndOfLine("\n---");
  返回 true;
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
  返回 0;
}

void Output::endSequence() {
  // If we did not emit anything, we should explicitly emit an empty sequence
  如果 (StateStack.back() == inSeqFirstElement) {
    Padding = PaddingBeforeContainer;
    newLineCheck(/*EmptySequence=*/true);
    output("[]");
    Padding = "\n";
  }
  StateStack.pop_back();
}

bool Output::preflightElement(unsigned, void *&SaveInfo) {
  SaveInfo = nullptr;
  返回 true;
}

void Output::postflightElement(void *) {
  如果 (StateStack.back() == inSeqFirstElement) {
    StateStack.pop_back();
    StateStack.push_back(inSeqOtherElement);
  } 否则 如果 (StateStack.back() == inFlowSeqFirstElement) {
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
  返回 0;
}

void Output::endFlowSequence() {
  StateStack.pop_back();
  outputUpToEndOfLine(" ]");
}

bool Output::preflightFlowElement(unsigned, void *&SaveInfo) {
  如果 (NeedFlowSequenceComma)
    output(", ");
  如果 (WrapColumn && Column > WrapColumn) {
    output("\n");
    为 (整数 i = 0; i < ColumnAtFlowStart; ++i)
      output(" ");
    Column = ColumnAtFlowStart;
    output("  ");
  }
  SaveInfo = nullptr;
  返回 true;
}

void Output::postflightFlowElement(void *) {
  NeedFlowSequenceComma = true;
}

void Output::beginEnumScalar() {
  EnumerationMatchFound = false;
}

bool Output::matchEnumScalar(StringRef Str, bool Match) {
  如果 (Match && !EnumerationMatchFound) {
    newLineCheck();
    outputUpToEndOfLine(Str);
    EnumerationMatchFound = true;
  }
  返回 false;
}

bool Output::matchEnumFallback() {
  如果 (EnumerationMatchFound)
    返回 false;
  EnumerationMatchFound = true;
  返回 true;
}

void Output::endEnumScalar() {
  如果 (!EnumerationMatchFound)
    llvm_unreachable("bad runtime enum value");
}

bool Output::beginBitSetScalar(bool &DoClear) {
  newLineCheck();
  output("[ ");
  NeedBitValueComma = false;
  DoClear = false;
  返回 true;
}

bool Output::bitSetMatch(StringRef Str, bool Matches) {
  如果 (Matches) {
    如果 (NeedBitValueComma)
      output(", ");
    output(Str);
    NeedBitValueComma = true;
  }
  返回 false;
}

void Output::endBitSetScalar() {
  outputUpToEndOfLine(" ]");
}

void Output::scalarString(StringRef &S, QuotingType MustQuote) {
  newLineCheck();
  如果 (S.empty()) {
    // Print '' for the empty string because leaving the field empty is not
    // allowed.
    outputUpToEndOfLine("''");
    返回;
  }
  output(S, MustQuote);
  outputUpToEndOfLine("");
}

void Output::blockScalarString(StringRef &S) {
  如果 (!StateStack.empty())
    newLineCheck();
  output(" |");

  unsigned Indent = StateStack.empty() ? 1 : StateStack.size();

  auto Buffer = MemoryBuffer::getMemBuffer(S, "", false);
  为 (line_iterator Lines(*Buffer, false); !Lines.is_at_end(); ++Lines) {
    outputNewLine();
    为 (unsigned I = 0; I < Indent; ++I) {
      output("  ");
    }
    output(*Lines);
  }
  outputUpToEndOfLine("");
}

void Output::scalarTag(std::string &Tag) {
  如果 (Tag.empty())
    返回;
  newLineCheck();
  output(Tag);
  output(" ");
}

void Output::setError(const Twine &message) {
}

std::error_code Output::error() { 返回 {}; }

bool Output::canElideEmptySequence() {
  // Normally, with an optional key/value where the value is an empty sequence,
  // the whole key/value can be not written.  But, that produces wrong yaml
  // if the key/value is the only thing in the map and the map is used in
  // a sequence.  This detects if the this sequence is the first key/value
  // in map that itself is embedded in a sequence.
  如果 (StateStack.size() < 2)
    返回 true;
  如果 (StateStack.back() != inMapFirstKey)
    返回 true;
  返回 !inSeqAnyElement(StateStack[StateStack.size() - 2]);
}

void Output::output(StringRef s) {
  Column += s.size();
  Out << s;
}

void Output::output(StringRef S, QuotingType MustQuote) {
  如果 (MustQuote == QuotingType::None) {
    // Only quote if we must.
    output(S);
    返回;
  }

  StringLiteral Quote = MustQuote == QuotingType::Single ? StringLiteral("'")
                                                         : StringLiteral("\"");
  output(Quote); // Starting quote.

  // When using double-quoted strings (and only in that case), non-printable
  // characters may be present, and will be escaped using a variety of
  // unicode-scalar and special short-form escapes. This is handled in
  // yaml::escape.
  如果 (MustQuote == QuotingType::Double) {
    output(yaml::escape(S, /* EscapePrintable= */ false));
    output(Quote);
    返回;
  }

  unsigned i = 0;
  unsigned j = 0;
  unsigned End = S.size();
  const char *Base = S.data();

  // When using single-quoted strings, any single quote ' must be doubled to be
  // escaped.
  当 (j < End) {
    如果 (S[j] == '\'') {                   // Escape quotes.
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
  如果 (StateStack.empty() || (!inFlowSeqAnyElement(StateStack.back()) &&
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
  如果 (Padding != "\n") {
    output(Padding);
    Padding = {};
    返回;
  }
  outputNewLine();
  Padding = {};

  如果 (StateStack.size() == 0 || EmptySequence)
    返回;

  unsigned Indent = StateStack.size() - 1;
  bool PossiblyNestedSeq = false;
  auto I = StateStack.rbegin(), E = StateStack.rend();

  如果 (inSeqAnyElement(*I)) {
    PossiblyNestedSeq = true; // Not possibly but always.
    ++Indent;
  } 否则 如果 (*I == inMapFirstKey || *I == inFlowMapFirstKey ||
             inFlowSeqAnyElement(*I)) {
    PossiblyNestedSeq = true;
    ++I; // Skip back().
  }

  unsigned OutputDashCount = 0;
  如果 (PossiblyNestedSeq) {
    // Count up consecutive inSeqFirstElement from the end, unless
    // inSeqFirstElement is the top of nested sequence.
    当 (I != E) {
      // Don't count the top of nested sequence.
      如果 (!inSeqAnyElement(*I))
        break;

      ++OutputDashCount;

      // Stop counting if consecutive inSeqFirstElement ends.
      如果 (*I++ != inSeqFirstElement)
        break;
    }
  }

  为 (unsigned I = OutputDashCount; I < Indent; ++I)
    output("  ");

  为 (unsigned I = 0; I < OutputDashCount; ++I)
    output("- ");
}

void Output::paddedKey(StringRef key) {
  output(key, needsQuotes(key, false));
  output(":");
  const char *spaces = "                ";
  如果 (key.size() < strlen(spaces))
    Padding = &spaces[key.size()];
  否则
    Padding = " ";
}

void Output::flowKey(StringRef Key) {
  如果 (StateStack.back() == inFlowMapOtherKey)
    output(", ");
  如果 (WrapColumn && Column > WrapColumn) {
    output("\n");
    为 (整数 I = 0; I < ColumnAtMapFlowStart; ++I)
      output(" ");
    Column = ColumnAtMapFlowStart;
    output("  ");
  }
  output(Key, needsQuotes(Key, false));
  output(": ");
}

NodeKind Output::getNodeKind() { report_fatal_error("invalid call"); }

bool Output::inSeqAnyElement(InState State) {
  返回 State == inSeqFirstElement || State == inSeqOtherElement;
}

bool Output::inFlowSeqAnyElement(InState State) {
  返回 State == inFlowSeqFirstElement || State == inFlowSeqOtherElement;
}

bool Output::inMapAnyKey(InState State) {
  返回 State == inMapFirstKey || State == inMapOtherKey;
}

bool Output::inFlowMapAnyKey(InState State) {
  返回 State == inFlowMapFirstKey || State == inFlowMapOtherKey;
}

//===----------------------------------------------------------------------===//
//  traits for built-in types
//===----------------------------------------------------------------------===//

void ScalarTraits<bool>::output(const bool &Val, void *, raw_ostream &Out) {
  Out << (Val ? "true" : "false");
}

StringRef ScalarTraits<bool>::input(StringRef Scalar, void *, bool &Val) {
  如果 (std::optional<bool> Parsed = parseBool(Scalar)) {
    Val = *Parsed;
    返回 StringRef();
  }
  返回 "invalid boolean";
}

void ScalarTraits<StringRef>::output(const StringRef &Val, void *,
                                     raw_ostream &Out) {
  Out << Val;
}

StringRef ScalarTraits<StringRef>::input(StringRef Scalar, void *,
                                         StringRef &Val) {
  Val = Scalar;
  返回 StringRef();
}

void ScalarTraits<std::string>::output(const std::string &Val, void *,
                                       raw_ostream &Out) {
  Out << Val;
}

StringRef ScalarTraits<std::string>::input(StringRef Scalar, void *,
                                           std::string &Val) {
  Val = Scalar.str();
  返回 StringRef();
}

void ScalarTraits<uint8_t>::output(const uint8_t &Val, void *,
                                   raw_ostream &Out) {
  // use temp uin32_t because ostream thinks uint8_t is a character
  uint32_t Num = Val;
  Out << Num;
}

StringRef ScalarTraits<uint8_t>::input(StringRef Scalar, void *, uint8_t &Val) {
  unsigned long long n;
  如果 (getAsUnsignedInteger(Scalar, 0, n))
    返回 "invalid number";
  如果 (n > 0xFF)
    返回 "out of range number";
  Val = n;
  返回 StringRef();
}

void ScalarTraits<uint16_t>::output(const uint16_t &Val, void *,
                                    raw_ostream &Out) {
  Out << Val;
}

StringRef ScalarTraits<uint16_t>::input(StringRef Scalar, void *,
                                        uint16_t &Val) {
  unsigned long long n;
  如果 (getAsUnsignedInteger(Scalar, 0, n))
    返回 "invalid number";
  如果 (n > 0xFFFF)
    返回 "out of range number";
  Val = n;
  返回 StringRef();
}

void ScalarTraits<uint32_t>::output(const uint32_t &Val, void *,
                                    raw_ostream &Out) {
  Out << Val;
}

StringRef ScalarTraits<uint32_t>::input(StringRef Scalar, void *,
                                        uint32_t &Val) {
  unsigned long long n;
  如果 (getAsUnsignedInteger(Scalar, 0, n))
    返回 "invalid number";
  如果 (n > 0xFFFFFFFFUL)
    返回 "out of range number";
  Val = n;
  返回 StringRef();
}

void ScalarTraits<uint64_t>::output(const uint64_t &Val, void *,
                                    raw_ostream &Out) {
  Out << Val;
}

StringRef ScalarTraits<uint64_t>::input(StringRef Scalar, void *,
                                        uint64_t &Val) {
  unsigned long long N;
  如果 (getAsUnsignedInteger(Scalar, 0, N))
    返回 "invalid number";
  Val = N;
  返回 StringRef();
}

void ScalarTraits<int8_t>::output(const int8_t &Val, void *, raw_ostream &Out) {
  // use temp in32_t because ostream thinks int8_t is a character
  int32_t Num = Val;
  Out << Num;
}

StringRef ScalarTraits<int8_t>::input(StringRef Scalar, void *, int8_t &Val) {
  long long N;
  如果 (getAsSignedInteger(Scalar, 0, N))
    返回 "invalid number";
  如果 ((N > 127) || (N < -128))
    返回 "out of range number";
  Val = N;
  返回 StringRef();
}

void ScalarTraits<int16_t>::output(const int16_t &Val, void *,
                                   raw_ostream &Out) {
  Out << Val;
}

StringRef ScalarTraits<int16_t>::input(StringRef Scalar, void *, int16_t &Val) {
  long long N;
  如果 (getAsSignedInteger(Scalar, 0, N))
    返回 "invalid number";
  如果 ((N > INT16_MAX) || (N < INT16_MIN))
    返回 "out of range number";
  Val = N;
  返回 StringRef();
}

void ScalarTraits<int32_t>::output(const int32_t &Val, void *,
                                   raw_ostream &Out) {
  Out << Val;
}

StringRef ScalarTraits<int32_t>::input(StringRef Scalar, void *, int32_t &Val) {
  long long N;
  如果 (getAsSignedInteger(Scalar, 0, N))
    返回 "invalid number";
  如果 ((N > INT32_MAX) || (N < INT32_MIN))
    返回 "out of range number";
  Val = N;
  返回 StringRef();
}

void ScalarTraits<int64_t>::output(const int64_t &Val, void *,
                                   raw_ostream &Out) {
  Out << Val;
}

StringRef ScalarTraits<int64_t>::input(StringRef Scalar, void *, int64_t &Val) {
  long long N;
  如果 (getAsSignedInteger(Scalar, 0, N))
    返回 "invalid number";
  Val = N;
  返回 StringRef();
}

void ScalarTraits<double>::output(const double &Val, void *, raw_ostream &Out) {
  Out << format("%g", Val);
}

StringRef ScalarTraits<double>::input(StringRef Scalar, void *, double &Val) {
  如果 (to_float(Scalar, Val))
    返回 StringRef();
  返回 "invalid floating point number";
}

void ScalarTraits<float>::output(const float &Val, void *, raw_ostream &Out) {
  Out << format("%g", Val);
}

StringRef ScalarTraits<float>::input(StringRef Scalar, void *, float &Val) {
  如果 (to_float(Scalar, Val))
    返回 StringRef();
  返回 "invalid floating point number";
}

void ScalarTraits<Hex8>::output(const Hex8 &Val, void *, raw_ostream &Out) {
  Out << format("0x%" PRIX8, (uint8_t)Val);
}

StringRef ScalarTraits<Hex8>::input(StringRef Scalar, void *, Hex8 &Val) {
  unsigned long long n;
  如果 (getAsUnsignedInteger(Scalar, 0, n))
    返回 "invalid hex8 number";
  如果 (n > 0xFF)
    返回 "out of range hex8 number";
  Val = n;
  返回 StringRef();
}

void ScalarTraits<Hex16>::output(const Hex16 &Val, void *, raw_ostream &Out) {
  Out << format("0x%" PRIX16, (uint16_t)Val);
}

StringRef ScalarTraits<Hex16>::input(StringRef Scalar, void *, Hex16 &Val) {
  unsigned long long n;
  如果 (getAsUnsignedInteger(Scalar, 0, n))
    返回 "invalid hex16 number";
  如果 (n > 0xFFFF)
    返回 "out of range hex16 number";
  Val = n;
  返回 StringRef();
}

void ScalarTraits<Hex32>::output(const Hex32 &Val, void *, raw_ostream &Out) {
  Out << format("0x%" PRIX32, (uint32_t)Val);
}

StringRef ScalarTraits<Hex32>::input(StringRef Scalar, void *, Hex32 &Val) {
  unsigned long long n;
  如果 (getAsUnsignedInteger(Scalar, 0, n))
    返回 "invalid hex32 number";
  如果 (n > 0xFFFFFFFFUL)
    返回 "out of range hex32 number";
  Val = n;
  返回 StringRef();
}

void ScalarTraits<Hex64>::output(const Hex64 &Val, void *, raw_ostream &Out) {
  Out << format("0x%" PRIX64, (uint64_t)Val);
}

StringRef ScalarTraits<Hex64>::input(StringRef Scalar, void *, Hex64 &Val) {
  unsigned long long Num;
  如果 (getAsUnsignedInteger(Scalar, 0, Num))
    返回 "invalid hex64 number";
  Val = Num;
  返回 StringRef();
}

void ScalarTraits<VersionTuple>::output(const VersionTuple &Val, void *,
                                        llvm::raw_ostream &Out) {
  Out << Val.getAsString();
}

StringRef ScalarTraits<VersionTuple>::input(StringRef Scalar, void *,
                                            VersionTuple &Val) {
  如果 (Val.tryParse(Scalar))
    返回 "invalid version format";
  返回 StringRef();
}
