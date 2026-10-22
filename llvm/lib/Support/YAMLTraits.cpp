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
  trả_về Ctxt;
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
  nếu (DiagHandler)
    SrcMgr.setDiagHandler(DiagHandler, DiagHandlerCtxt);
  DocIterator = Strm->begin();
}

Input::Input(MemoryBufferRef Input, void *Ctxt,
             SourceMgr::DiagHandlerTy DiagHandler, void *DiagHandlerCtxt)
    : IO(Ctxt), Strm(new Stream(Input, SrcMgr, false, &EC)) {
  nếu (DiagHandler)
    SrcMgr.setDiagHandler(DiagHandler, DiagHandlerCtxt);
  DocIterator = Strm->begin();
}

Input::~Input() = default;

std::error_code Input::error() { trả_về EC; }

bool Input::outputting() const {
  trả_về false;
}

bool Input::setCurrentDocument() {
  nếu (DocIterator != Strm->end()) {
    Node *N = DocIterator->getRoot();
    nếu (!N) {
      EC = make_error_code(errc::invalid_argument);
      trả_về false;
    }

    nếu (isa<NullNode>(N)) {
      // Empty files are allowed and ignored
      ++DocIterator;
      trả_về setCurrentDocument();
    }
    releaseHNodeBuffers();
    TopNode = createHNodes(N);
    CurrentNode = TopNode;
    trả_về true;
  }
  trả_về false;
}

bool Input::nextDocument() {
  trả_về ++DocIterator != Strm->end();
}

const Node *Input::getCurrentNode() const {
  trả_về CurrentNode ? CurrentNode->_node : nullptr;
}

bool Input::mapTag(StringRef Tag, bool Default) {
  // CurrentNode can be null if setCurrentDocument() was unable to
  // parse the document because it was invalid or empty.
  nếu (!CurrentNode)
    trả_về false;

  std::string foundTag = CurrentNode->_node->getVerbatimTag();
  nếu (foundTag.empty()) {
    // If no tag found and 'Tag' is the default, say it was found.
    trả_về Default;
  }
  // Return true iff found tag matches supplied tag.
  trả_về Tag == foundTag;
}

void Input::beginMapping() {
  nếu (EC)
    trả_về;
  // CurrentNode can be null if the document is empty.
  MapHNode *MN = dyn_cast_or_null<MapHNode>(CurrentNode);
  nếu (MN) {
    MN->ValidKeys.clear();
  }
}

std::vector<StringRef> Input::keys() {
  MapHNode *MN = dyn_cast<MapHNode>(CurrentNode);
  std::vector<StringRef> Ret;
  nếu (!MN) {
    setError(CurrentNode, "not a mapping");
    trả_về Ret;
  }
  cho (auto &P : MN->Mapping)
    Ret.push_back(P.first());
  trả_về Ret;
}

bool Input::preflightKey(StringRef Key, bool Required, bool, bool &UseDefault,
                         void *&SaveInfo) {
  UseDefault = false;
  nếu (EC)
    trả_về false;

  // CurrentNode is null for empty documents, which is an error in case required
  // nodes are present.
  nếu (!CurrentNode) {
    nếu (Required)
      EC = make_error_code(errc::invalid_argument);
    không_thì
      UseDefault = true;
    trả_về false;
  }

  MapHNode *MN = dyn_cast<MapHNode>(CurrentNode);
  nếu (!MN) {
    nếu (Required || !isa<EmptyHNode>(CurrentNode))
      setError(CurrentNode, "not a mapping");
    không_thì
      UseDefault = true;
    trả_về false;
  }
  MN->ValidKeys.push_back(Key.str());
  HNode *Value = MN->Mapping[Key].first;
  nếu (!Value) {
    nếu (Required)
      setError(CurrentNode, Twine("missing required key '") + Key + "'");
    không_thì
      UseDefault = true;
    trả_về false;
  }
  SaveInfo = CurrentNode;
  CurrentNode = Value;
  trả_về true;
}

void Input::postflightKey(void *saveInfo) {
  CurrentNode = reinterpret_cast<HNode *>(saveInfo);
}

void Input::endMapping() {
  nếu (EC)
    trả_về;
  // CurrentNode can be null if the document is empty.
  MapHNode *MN = dyn_cast_or_null<MapHNode>(CurrentNode);
  nếu (!MN)
    trả_về;
  cho (const auto &NN : MN->Mapping) {
    nếu (!is_contained(MN->ValidKeys, NN.first())) {
      const SMRange &ReportLoc = NN.second.second;
      nếu (!AllowUnknownKeys) {
        setError(ReportLoc, Twine("unknown key '") + NN.first() + "'");
        break;
      } không_thì
        reportWarning(ReportLoc, Twine("unknown key '") + NN.first() + "'");
    }
  }
}

void Input::beginFlowMapping() { beginMapping(); }

void Input::endFlowMapping() { endMapping(); }

unsigned Input::beginSequence() {
  nếu (SequenceHNode *SQ = dyn_cast<SequenceHNode>(CurrentNode))
    trả_về SQ->Entries.size();
  nếu (isa<EmptyHNode>(CurrentNode))
    trả_về 0;
  // Treat case where there's a scalar "null" value as an empty sequence.
  nếu (ScalarHNode *SN = dyn_cast<ScalarHNode>(CurrentNode)) {
    nếu (isNull(SN->value()))
      trả_về 0;
  }
  // Any other type of HNode is an error.
  setError(CurrentNode, "not a sequence");
  trả_về 0;
}

void Input::endSequence() {
}

bool Input::preflightElement(unsigned Index, void *&SaveInfo) {
  nếu (EC)
    trả_về false;
  nếu (SequenceHNode *SQ = dyn_cast<SequenceHNode>(CurrentNode)) {
    SaveInfo = CurrentNode;
    CurrentNode = SQ->Entries[Index];
    trả_về true;
  }
  trả_về false;
}

void Input::postflightElement(void *SaveInfo) {
  CurrentNode = reinterpret_cast<HNode *>(SaveInfo);
}

unsigned Input::beginFlowSequence() { trả_về beginSequence(); }

bool Input::preflightFlowElement(unsigned index, void *&SaveInfo) {
  nếu (EC)
    trả_về false;
  nếu (SequenceHNode *SQ = dyn_cast<SequenceHNode>(CurrentNode)) {
    SaveInfo = CurrentNode;
    CurrentNode = SQ->Entries[index];
    trả_về true;
  }
  trả_về false;
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
  nếu (ScalarMatchFound)
    trả_về false;
  nếu (ScalarHNode *SN = dyn_cast<ScalarHNode>(CurrentNode)) {
    nếu (SN->value() == Str) {
      ScalarMatchFound = true;
      trả_về true;
    }
  }
  trả_về false;
}

bool Input::matchEnumFallback() {
  nếu (ScalarMatchFound)
    trả_về false;
  ScalarMatchFound = true;
  trả_về true;
}

void Input::endEnumScalar() {
  nếu (!ScalarMatchFound) {
    setError(CurrentNode, "unknown enumerated scalar");
  }
}

bool Input::beginBitSetScalar(bool &DoClear) {
  BitValuesUsed.clear();
  nếu (SequenceHNode *SQ = dyn_cast<SequenceHNode>(CurrentNode)) {
    BitValuesUsed.resize(SQ->Entries.size());
  } không_thì {
    setError(CurrentNode, "expected sequence of bit values");
  }
  DoClear = true;
  trả_về true;
}

bool Input::bitSetMatch(StringRef Str, bool) {
  nếu (EC)
    trả_về false;
  nếu (SequenceHNode *SQ = dyn_cast<SequenceHNode>(CurrentNode)) {
    unsigned Index = 0;
    cho (auto &N : SQ->Entries) {
      nếu (ScalarHNode *SN = dyn_cast<ScalarHNode>(N)) {
        nếu (SN->value() == Str) {
          BitValuesUsed[Index] = true;
          trả_về true;
        }
      } không_thì {
        setError(CurrentNode, "unexpected scalar in sequence of bit values");
      }
      ++Index;
    }
  } không_thì {
    setError(CurrentNode, "expected sequence of bit values");
  }
  trả_về false;
}

void Input::endBitSetScalar() {
  nếu (EC)
    trả_về;
  nếu (SequenceHNode *SQ = dyn_cast<SequenceHNode>(CurrentNode)) {
    assert(BitValuesUsed.size() == SQ->Entries.size());
    cho (unsigned i = 0; i < SQ->Entries.size(); ++i) {
      nếu (!BitValuesUsed[i]) {
        setError(SQ->Entries[i], "unknown bit value");
        trả_về;
      }
    }
  }
}

void Input::scalarString(StringRef &S, QuotingType) {
  nếu (ScalarHNode *SN = dyn_cast<ScalarHNode>(CurrentNode)) {
    S = SN->value();
  } không_thì {
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
  nếu (isa<ScalarHNode>(CurrentNode))
    trả_về NodeKind::Scalar;
  không_thì nếu (isa<MapHNode>(CurrentNode))
    trả_về NodeKind::Map;
  không_thì nếu (isa<SequenceHNode>(CurrentNode))
    trả_về NodeKind::Sequence;
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
  nếu (!Anchor.empty())
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
    nếu (!StringStorage.empty()) {
      // Copy string to permanent storage
      KeyStr = StringStorage.str().copy(StringAllocator);
    }
    auto *SHNode = new (ScalarHNodeAllocator.Allocate()) ScalarHNode(N, KeyStr);
    saveAliasHNode(SN, SHNode);
    trả_về SHNode;
  }
  case Node::NK_BlockScalar: {
    BlockScalarNode *BSN = dyn_cast<BlockScalarNode>(N);
    StringRef ValueCopy = BSN->getValue().copy(StringAllocator);
    auto *BSHNode =
        new (ScalarHNodeAllocator.Allocate()) ScalarHNode(N, ValueCopy);
    saveAliasHNode(BSN, BSHNode);
    trả_về BSHNode;
  }
  case Node::NK_Sequence: {
    SequenceNode *SQ = dyn_cast<SequenceNode>(N);
    auto SQHNode = new (SequenceHNodeAllocator.Allocate()) SequenceHNode(N);
    cho (Node &SN : *SQ) {
      auto Entry = createHNodes(&SN);
      nếu (EC)
        break;
      SQHNode->Entries.push_back(Entry);
    }
    saveAliasHNode(SQ, SQHNode);
    trả_về SQHNode;
  }
  case Node::NK_Mapping: {
    MappingNode *Map = dyn_cast<MappingNode>(N);
    auto mapHNode = new (MapHNodeAllocator.Allocate()) MapHNode(N);
    cho (KeyValueNode &KVN : *Map) {
      Node *KeyNode = KVN.getKey();
      ScalarNode *Key = dyn_cast_if_present<ScalarNode>(KeyNode);
      Node *Value = KVN.getValue();
      nếu (!Key || !Value) {
        nếu (!Key)
          setError(KeyNode, "Map key must be a scalar");
        nếu (!Value)
          setError(KeyNode, "Map value must not be empty");
        break;
      }
      StringStorage.clear();
      StringRef KeyStr = Key->getValue(StringStorage);
      nếu (!StringStorage.empty()) {
        // Copy string to permanent storage
        KeyStr = StringStorage.str().copy(StringAllocator);
      }
      nếu (mapHNode->Mapping.count(KeyStr))
        // From YAML spec: "The content of a mapping node is an unordered set of
        // key/value node pairs, with the restriction that each of the keys is
        // unique."
        setError(KeyNode, Twine("duplicated mapping key '") + KeyStr + "'");
      auto ValueHNode = createHNodes(Value);
      nếu (EC)
        break;
      mapHNode->Mapping[KeyStr] =
          std::make_pair(std::move(ValueHNode), KeyNode->getSourceRange());
    }
    saveAliasHNode(Map, mapHNode);
    trả_về std::move(mapHNode);
  }
  case Node::NK_Null:
    // TODO: Anchor is not set for NullNode in the parser. Update the parser to
    // faithfully preserve anchors.
    trả_về new (EmptyHNodeAllocator.Allocate()) EmptyHNode(N);
  case Node::NK_Alias: {
    AliasNode *AN = dyn_cast<AliasNode>(N);
    auto AliasName = AN->getName();
    auto AHN = AliasMap.find(AliasName);
    nếu (AHN == AliasMap.end()) {
      setError(AN, Twine("undefined alias '" + AliasName + "'"));
      trả_về nullptr;
    }
    trả_về AHN->second;
  }
  default:
    setError(N, "unknown node kind");
    trả_về nullptr;
  }
}

void Input::setError(const Twine &Message) {
  setError(CurrentNode, Message);
}

void Input::setAllowUnknownKeys(bool Allow) { AllowUnknownKeys = Allow; }

bool Input::canElideEmptySequence() {
  trả_về false;
}

//===----------------------------------------------------------------------===//
//  Output
//===----------------------------------------------------------------------===//

Output::Output(raw_ostream &yout, void *context, số_nguyên WrapColumn)
    : IO(context), Out(yout), WrapColumn(WrapColumn) {}

Output::~Output() = default;

bool Output::outputting() const {
  trả_về true;
}

void Output::beginMapping() {
  StateStack.push_back(inMapFirstKey);
  PaddingBeforeContainer = Padding;
  Padding = "\n";
}

bool Output::mapTag(StringRef Tag, bool Use) {
  nếu (Use) {
    // If this tag is being written inside a sequence we should write the start
    // of the sequence before writing the tag, otherwise the tag won't be
    // attached to the element in the sequence, but rather the sequence itself.
    bool SequenceElement = false;
    nếu (StateStack.size() > 1) {
      auto &E = StateStack[StateStack.size() - 2];
      SequenceElement = inSeqAnyElement(E) || inFlowSeqAnyElement(E);
    }
    nếu (SequenceElement && StateStack.back() == inMapFirstKey) {
      newLineCheck();
    } không_thì {
      output(" ");
    }
    output(Tag);
    nếu (SequenceElement) {
      // If we're writing the tag during the first element of a map, the tag
      // takes the place of the first element in the sequence.
      nếu (StateStack.back() == inMapFirstKey) {
        StateStack.pop_back();
        StateStack.push_back(inMapOtherKey);
      }
      // Tags inside maps in sequences should act as keys in the map from a
      // formatting perspective, so we always want a newline in a sequence.
      Padding = "\n";
    }
  }
  trả_về Use;
}

void Output::endMapping() {
  // If we did not map anything, we should explicitly emit an empty map
  nếu (StateStack.back() == inMapFirstKey) {
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
  nếu (Required || !SameAsDefault || WriteDefaultValues) {
    auto State = StateStack.back();
    nếu (State == inFlowMapFirstKey || State == inFlowMapOtherKey) {
      flowKey(Key);
    } không_thì {
      newLineCheck();
      paddedKey(Key);
    }
    trả_về true;
  }
  trả_về false;
}

void Output::postflightKey(void *) {
  nếu (StateStack.back() == inMapFirstKey) {
    StateStack.pop_back();
    StateStack.push_back(inMapOtherKey);
  } không_thì nếu (StateStack.back() == inFlowMapFirstKey) {
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
  nếu (index > 0)
    outputUpToEndOfLine("\n---");
  trả_về true;
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
  trả_về 0;
}

void Output::endSequence() {
  // If we did not emit anything, we should explicitly emit an empty sequence
  nếu (StateStack.back() == inSeqFirstElement) {
    Padding = PaddingBeforeContainer;
    newLineCheck(/*EmptySequence=*/true);
    output("[]");
    Padding = "\n";
  }
  StateStack.pop_back();
}

bool Output::preflightElement(unsigned, void *&SaveInfo) {
  SaveInfo = nullptr;
  trả_về true;
}

void Output::postflightElement(void *) {
  nếu (StateStack.back() == inSeqFirstElement) {
    StateStack.pop_back();
    StateStack.push_back(inSeqOtherElement);
  } không_thì nếu (StateStack.back() == inFlowSeqFirstElement) {
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
  trả_về 0;
}

void Output::endFlowSequence() {
  StateStack.pop_back();
  outputUpToEndOfLine(" ]");
}

bool Output::preflightFlowElement(unsigned, void *&SaveInfo) {
  nếu (NeedFlowSequenceComma)
    output(", ");
  nếu (WrapColumn && Column > WrapColumn) {
    output("\n");
    cho (số_nguyên i = 0; i < ColumnAtFlowStart; ++i)
      output(" ");
    Column = ColumnAtFlowStart;
    output("  ");
  }
  SaveInfo = nullptr;
  trả_về true;
}

void Output::postflightFlowElement(void *) {
  NeedFlowSequenceComma = true;
}

void Output::beginEnumScalar() {
  EnumerationMatchFound = false;
}

bool Output::matchEnumScalar(StringRef Str, bool Match) {
  nếu (Match && !EnumerationMatchFound) {
    newLineCheck();
    outputUpToEndOfLine(Str);
    EnumerationMatchFound = true;
  }
  trả_về false;
}

bool Output::matchEnumFallback() {
  nếu (EnumerationMatchFound)
    trả_về false;
  EnumerationMatchFound = true;
  trả_về true;
}

void Output::endEnumScalar() {
  nếu (!EnumerationMatchFound)
    llvm_unreachable("bad runtime enum value");
}

bool Output::beginBitSetScalar(bool &DoClear) {
  newLineCheck();
  output("[ ");
  NeedBitValueComma = false;
  DoClear = false;
  trả_về true;
}

bool Output::bitSetMatch(StringRef Str, bool Matches) {
  nếu (Matches) {
    nếu (NeedBitValueComma)
      output(", ");
    output(Str);
    NeedBitValueComma = true;
  }
  trả_về false;
}

void Output::endBitSetScalar() {
  outputUpToEndOfLine(" ]");
}

void Output::scalarString(StringRef &S, QuotingType MustQuote) {
  newLineCheck();
  nếu (S.empty()) {
    // Print '' for the empty string because leaving the field empty is not
    // allowed.
    outputUpToEndOfLine("''");
    trả_về;
  }
  output(S, MustQuote);
  outputUpToEndOfLine("");
}

void Output::blockScalarString(StringRef &S) {
  nếu (!StateStack.empty())
    newLineCheck();
  output(" |");

  unsigned Indent = StateStack.empty() ? 1 : StateStack.size();

  auto Buffer = MemoryBuffer::getMemBuffer(S, "", false);
  cho (line_iterator Lines(*Buffer, false); !Lines.is_at_end(); ++Lines) {
    outputNewLine();
    cho (unsigned I = 0; I < Indent; ++I) {
      output("  ");
    }
    output(*Lines);
  }
  outputUpToEndOfLine("");
}

void Output::scalarTag(std::string &Tag) {
  nếu (Tag.empty())
    trả_về;
  newLineCheck();
  output(Tag);
  output(" ");
}

void Output::setError(const Twine &message) {
}

std::error_code Output::error() { trả_về {}; }

bool Output::canElideEmptySequence() {
  // Normally, with an optional key/value where the value is an empty sequence,
  // the whole key/value can be not written.  But, that produces wrong yaml
  // if the key/value is the only thing in the map and the map is used in
  // a sequence.  This detects if the this sequence is the first key/value
  // in map that itself is embedded in a sequence.
  nếu (StateStack.size() < 2)
    trả_về true;
  nếu (StateStack.back() != inMapFirstKey)
    trả_về true;
  trả_về !inSeqAnyElement(StateStack[StateStack.size() - 2]);
}

void Output::output(StringRef s) {
  Column += s.size();
  Out << s;
}

void Output::output(StringRef S, QuotingType MustQuote) {
  nếu (MustQuote == QuotingType::None) {
    // Only quote if we must.
    output(S);
    trả_về;
  }

  StringLiteral Quote = MustQuote == QuotingType::Single ? StringLiteral("'")
                                                         : StringLiteral("\"");
  output(Quote); // Starting quote.

  // When using double-quoted strings (and only in that case), non-printable
  // characters may be present, and will be escaped using a variety of
  // unicode-scalar and special short-form escapes. This is handled in
  // yaml::escape.
  nếu (MustQuote == QuotingType::Double) {
    output(yaml::escape(S, /* EscapePrintable= */ false));
    output(Quote);
    trả_về;
  }

  unsigned i = 0;
  unsigned j = 0;
  unsigned End = S.size();
  const char *Base = S.data();

  // When using single-quoted strings, any single quote ' must be doubled to be
  // escaped.
  trong_khi (j < End) {
    nếu (S[j] == '\'') {                   // Escape quotes.
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
  nếu (StateStack.empty() || (!inFlowSeqAnyElement(StateStack.back()) &&
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
  nếu (Padding != "\n") {
    output(Padding);
    Padding = {};
    trả_về;
  }
  outputNewLine();
  Padding = {};

  nếu (StateStack.size() == 0 || EmptySequence)
    trả_về;

  unsigned Indent = StateStack.size() - 1;
  bool PossiblyNestedSeq = false;
  auto I = StateStack.rbegin(), E = StateStack.rend();

  nếu (inSeqAnyElement(*I)) {
    PossiblyNestedSeq = true; // Not possibly but always.
    ++Indent;
  } không_thì nếu (*I == inMapFirstKey || *I == inFlowMapFirstKey ||
             inFlowSeqAnyElement(*I)) {
    PossiblyNestedSeq = true;
    ++I; // Skip back().
  }

  unsigned OutputDashCount = 0;
  nếu (PossiblyNestedSeq) {
    // Count up consecutive inSeqFirstElement from the end, unless
    // inSeqFirstElement is the top of nested sequence.
    trong_khi (I != E) {
      // Don't count the top of nested sequence.
      nếu (!inSeqAnyElement(*I))
        break;

      ++OutputDashCount;

      // Stop counting if consecutive inSeqFirstElement ends.
      nếu (*I++ != inSeqFirstElement)
        break;
    }
  }

  cho (unsigned I = OutputDashCount; I < Indent; ++I)
    output("  ");

  cho (unsigned I = 0; I < OutputDashCount; ++I)
    output("- ");
}

void Output::paddedKey(StringRef key) {
  output(key, needsQuotes(key, false));
  output(":");
  const char *spaces = "                ";
  nếu (key.size() < strlen(spaces))
    Padding = &spaces[key.size()];
  không_thì
    Padding = " ";
}

void Output::flowKey(StringRef Key) {
  nếu (StateStack.back() == inFlowMapOtherKey)
    output(", ");
  nếu (WrapColumn && Column > WrapColumn) {
    output("\n");
    cho (số_nguyên I = 0; I < ColumnAtMapFlowStart; ++I)
      output(" ");
    Column = ColumnAtMapFlowStart;
    output("  ");
  }
  output(Key, needsQuotes(Key, false));
  output(": ");
}

NodeKind Output::getNodeKind() { report_fatal_error("invalid call"); }

bool Output::inSeqAnyElement(InState State) {
  trả_về State == inSeqFirstElement || State == inSeqOtherElement;
}

bool Output::inFlowSeqAnyElement(InState State) {
  trả_về State == inFlowSeqFirstElement || State == inFlowSeqOtherElement;
}

bool Output::inMapAnyKey(InState State) {
  trả_về State == inMapFirstKey || State == inMapOtherKey;
}

bool Output::inFlowMapAnyKey(InState State) {
  trả_về State == inFlowMapFirstKey || State == inFlowMapOtherKey;
}

//===----------------------------------------------------------------------===//
//  traits for built-in types
//===----------------------------------------------------------------------===//

void ScalarTraits<bool>::output(const bool &Val, void *, raw_ostream &Out) {
  Out << (Val ? "true" : "false");
}

StringRef ScalarTraits<bool>::input(StringRef Scalar, void *, bool &Val) {
  nếu (std::optional<bool> Parsed = parseBool(Scalar)) {
    Val = *Parsed;
    trả_về StringRef();
  }
  trả_về "invalid boolean";
}

void ScalarTraits<StringRef>::output(const StringRef &Val, void *,
                                     raw_ostream &Out) {
  Out << Val;
}

StringRef ScalarTraits<StringRef>::input(StringRef Scalar, void *,
                                         StringRef &Val) {
  Val = Scalar;
  trả_về StringRef();
}

void ScalarTraits<std::string>::output(const std::string &Val, void *,
                                       raw_ostream &Out) {
  Out << Val;
}

StringRef ScalarTraits<std::string>::input(StringRef Scalar, void *,
                                           std::string &Val) {
  Val = Scalar.str();
  trả_về StringRef();
}

void ScalarTraits<uint8_t>::output(const uint8_t &Val, void *,
                                   raw_ostream &Out) {
  // use temp uin32_t because ostream thinks uint8_t is a character
  uint32_t Num = Val;
  Out << Num;
}

StringRef ScalarTraits<uint8_t>::input(StringRef Scalar, void *, uint8_t &Val) {
  unsigned long long n;
  nếu (getAsUnsignedInteger(Scalar, 0, n))
    trả_về "invalid number";
  nếu (n > 0xFF)
    trả_về "out of range number";
  Val = n;
  trả_về StringRef();
}

void ScalarTraits<uint16_t>::output(const uint16_t &Val, void *,
                                    raw_ostream &Out) {
  Out << Val;
}

StringRef ScalarTraits<uint16_t>::input(StringRef Scalar, void *,
                                        uint16_t &Val) {
  unsigned long long n;
  nếu (getAsUnsignedInteger(Scalar, 0, n))
    trả_về "invalid number";
  nếu (n > 0xFFFF)
    trả_về "out of range number";
  Val = n;
  trả_về StringRef();
}

void ScalarTraits<uint32_t>::output(const uint32_t &Val, void *,
                                    raw_ostream &Out) {
  Out << Val;
}

StringRef ScalarTraits<uint32_t>::input(StringRef Scalar, void *,
                                        uint32_t &Val) {
  unsigned long long n;
  nếu (getAsUnsignedInteger(Scalar, 0, n))
    trả_về "invalid number";
  nếu (n > 0xFFFFFFFFUL)
    trả_về "out of range number";
  Val = n;
  trả_về StringRef();
}

void ScalarTraits<uint64_t>::output(const uint64_t &Val, void *,
                                    raw_ostream &Out) {
  Out << Val;
}

StringRef ScalarTraits<uint64_t>::input(StringRef Scalar, void *,
                                        uint64_t &Val) {
  unsigned long long N;
  nếu (getAsUnsignedInteger(Scalar, 0, N))
    trả_về "invalid number";
  Val = N;
  trả_về StringRef();
}

void ScalarTraits<int8_t>::output(const int8_t &Val, void *, raw_ostream &Out) {
  // use temp in32_t because ostream thinks int8_t is a character
  int32_t Num = Val;
  Out << Num;
}

StringRef ScalarTraits<int8_t>::input(StringRef Scalar, void *, int8_t &Val) {
  long long N;
  nếu (getAsSignedInteger(Scalar, 0, N))
    trả_về "invalid number";
  nếu ((N > 127) || (N < -128))
    trả_về "out of range number";
  Val = N;
  trả_về StringRef();
}

void ScalarTraits<int16_t>::output(const int16_t &Val, void *,
                                   raw_ostream &Out) {
  Out << Val;
}

StringRef ScalarTraits<int16_t>::input(StringRef Scalar, void *, int16_t &Val) {
  long long N;
  nếu (getAsSignedInteger(Scalar, 0, N))
    trả_về "invalid number";
  nếu ((N > INT16_MAX) || (N < INT16_MIN))
    trả_về "out of range number";
  Val = N;
  trả_về StringRef();
}

void ScalarTraits<int32_t>::output(const int32_t &Val, void *,
                                   raw_ostream &Out) {
  Out << Val;
}

StringRef ScalarTraits<int32_t>::input(StringRef Scalar, void *, int32_t &Val) {
  long long N;
  nếu (getAsSignedInteger(Scalar, 0, N))
    trả_về "invalid number";
  nếu ((N > INT32_MAX) || (N < INT32_MIN))
    trả_về "out of range number";
  Val = N;
  trả_về StringRef();
}

void ScalarTraits<int64_t>::output(const int64_t &Val, void *,
                                   raw_ostream &Out) {
  Out << Val;
}

StringRef ScalarTraits<int64_t>::input(StringRef Scalar, void *, int64_t &Val) {
  long long N;
  nếu (getAsSignedInteger(Scalar, 0, N))
    trả_về "invalid number";
  Val = N;
  trả_về StringRef();
}

void ScalarTraits<double>::output(const double &Val, void *, raw_ostream &Out) {
  Out << format("%g", Val);
}

StringRef ScalarTraits<double>::input(StringRef Scalar, void *, double &Val) {
  nếu (to_float(Scalar, Val))
    trả_về StringRef();
  trả_về "invalid floating point number";
}

void ScalarTraits<float>::output(const float &Val, void *, raw_ostream &Out) {
  Out << format("%g", Val);
}

StringRef ScalarTraits<float>::input(StringRef Scalar, void *, float &Val) {
  nếu (to_float(Scalar, Val))
    trả_về StringRef();
  trả_về "invalid floating point number";
}

void ScalarTraits<Hex8>::output(const Hex8 &Val, void *, raw_ostream &Out) {
  Out << format("0x%" PRIX8, (uint8_t)Val);
}

StringRef ScalarTraits<Hex8>::input(StringRef Scalar, void *, Hex8 &Val) {
  unsigned long long n;
  nếu (getAsUnsignedInteger(Scalar, 0, n))
    trả_về "invalid hex8 number";
  nếu (n > 0xFF)
    trả_về "out of range hex8 number";
  Val = n;
  trả_về StringRef();
}

void ScalarTraits<Hex16>::output(const Hex16 &Val, void *, raw_ostream &Out) {
  Out << format("0x%" PRIX16, (uint16_t)Val);
}

StringRef ScalarTraits<Hex16>::input(StringRef Scalar, void *, Hex16 &Val) {
  unsigned long long n;
  nếu (getAsUnsignedInteger(Scalar, 0, n))
    trả_về "invalid hex16 number";
  nếu (n > 0xFFFF)
    trả_về "out of range hex16 number";
  Val = n;
  trả_về StringRef();
}

void ScalarTraits<Hex32>::output(const Hex32 &Val, void *, raw_ostream &Out) {
  Out << format("0x%" PRIX32, (uint32_t)Val);
}

StringRef ScalarTraits<Hex32>::input(StringRef Scalar, void *, Hex32 &Val) {
  unsigned long long n;
  nếu (getAsUnsignedInteger(Scalar, 0, n))
    trả_về "invalid hex32 number";
  nếu (n > 0xFFFFFFFFUL)
    trả_về "out of range hex32 number";
  Val = n;
  trả_về StringRef();
}

void ScalarTraits<Hex64>::output(const Hex64 &Val, void *, raw_ostream &Out) {
  Out << format("0x%" PRIX64, (uint64_t)Val);
}

StringRef ScalarTraits<Hex64>::input(StringRef Scalar, void *, Hex64 &Val) {
  unsigned long long Num;
  nếu (getAsUnsignedInteger(Scalar, 0, Num))
    trả_về "invalid hex64 number";
  Val = Num;
  trả_về StringRef();
}

void ScalarTraits<VersionTuple>::output(const VersionTuple &Val, void *,
                                        llvm::raw_ostream &Out) {
  Out << Val.getAsString();
}

StringRef ScalarTraits<VersionTuple>::input(StringRef Scalar, void *,
                                            VersionTuple &Val) {
  nếu (Val.tryParse(Scalar))
    trả_về "invalid version format";
  trả_về StringRef();
}
