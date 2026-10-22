//===- lib/Support/YAMLTraits.cpp -----------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ضمن "llvm/Support/YAMLTraits.h"
#ضمن "llvm/ADT/STLExtras.h"
#ضمن "llvm/ADT/SmallString.h"
#ضمن "llvm/ADT/StringExtras.h"
#ضمن "llvm/ADT/StringRef.h"
#ضمن "llvm/ADT/Twine.h"
#ضمن "llvm/Support/Casting.h"
#ضمن "llvm/Support/Errc.h"
#ضمن "llvm/Support/ErrorHandling.h"
#ضمن "llvm/Support/Format.h"
#ضمن "llvm/Support/LineIterator.h"
#ضمن "llvm/Support/MemoryBuffer.h"
#ضمن "llvm/Support/VersionTuple.h"
#ضمن "llvm/Support/YAMLParser.h"
#ضمن "llvm/Support/raw_ostream.h"
#ضمن <cassert>
#ضمن <cstdint>
#ضمن <cstring>
#ضمن <string>
#ضمن <vector>

using نطاق llvm;
using نطاق yaml;

//===----------------------------------------------------------------------===//
//  IO
//===----------------------------------------------------------------------===//

IO::IO(فراغ *Context) : Ctxt(Context) {}

IO::~IO() = افتراضي;

فراغ *IO::getContext() ثابت {
  החזר Ctxt;
}

فراغ IO::setContext(فراغ *Context) {
  Ctxt = Context;
}

فراغ IO::setAllowUnknownKeys(منطقي Allow) {
  llvm_unreachable("Only supported for Input");
}

//===----------------------------------------------------------------------===//
//  Input
//===----------------------------------------------------------------------===//

Input::Input(StringRef InputContent, فراغ *Ctxt,
             SourceMgr::DiagHandlerTy DiagHandler, فراغ *DiagHandlerCtxt)
    : IO(Ctxt), Strm(جديد Stream(InputContent, SrcMgr, שקר, &EC)) {
  אם (DiagHandler)
    SrcMgr.setDiagHandler(DiagHandler, DiagHandlerCtxt);
  DocIterator = Strm->begin();
}

Input::Input(MemoryBufferRef Input, فراغ *Ctxt,
             SourceMgr::DiagHandlerTy DiagHandler, فراغ *DiagHandlerCtxt)
    : IO(Ctxt), Strm(جديد Stream(Input, SrcMgr, שקר, &EC)) {
  אם (DiagHandler)
    SrcMgr.setDiagHandler(DiagHandler, DiagHandlerCtxt);
  DocIterator = Strm->begin();
}

Input::~Input() = افتراضي;

std::error_code Input::error() { החזר EC; }

منطقي Input::outputting() ثابت {
  החזר שקר;
}

منطقي Input::setCurrentDocument() {
  אם (DocIterator != Strm->end()) {
    Node *N = DocIterator->getRoot();
    אם (!N) {
      EC = make_error_code(errc::invalid_argument);
      החזר שקר;
    }

    אם (isa<NullNode>(N)) {
      // Empty files are allowed and ignored
      ++DocIterator;
      החזר setCurrentDocument();
    }
    releaseHNodeBuffers();
    TopNode = createHNodes(N);
    CurrentNode = TopNode;
    החזר אמת;
  }
  החזר שקר;
}

منطقي Input::nextDocument() {
  החזר ++DocIterator != Strm->end();
}

ثابت Node *Input::getCurrentNode() ثابت {
  החזר CurrentNode ? CurrentNode->_node : nullptr;
}

منطقي Input::mapTag(StringRef Tag, منطقي Default) {
  // CurrentNode can be null if setCurrentDocument() was unable to
  // parse the document because it was invalid or empty.
  אם (!CurrentNode)
    החזר שקר;

  std::string foundTag = CurrentNode->_node->getVerbatimTag();
  אם (foundTag.empty()) {
    // If no tag found and 'Tag' is the default, say it was found.
    החזר Default;
  }
  // Return true iff found tag matches supplied tag.
  החזר Tag == foundTag;
}

فراغ Input::beginMapping() {
  אם (EC)
    החזר;
  // CurrentNode can be null if the document is empty.
  MapHNode *MN = dyn_cast_or_null<MapHNode>(CurrentNode);
  אם (MN) {
    MN->ValidKeys.clear();
  }
}

std::vector<StringRef> Input::keys() {
  MapHNode *MN = dyn_cast<MapHNode>(CurrentNode);
  std::vector<StringRef> Ret;
  אם (!MN) {
    setError(CurrentNode, "not a mapping");
    החזר Ret;
  }
  עבור (auto &P : MN->Mapping)
    Ret.push_back(P.first());
  החזר Ret;
}

منطقي Input::preflightKey(StringRef Key, منطقي Required, منطقي, منطقي &UseDefault,
                         فراغ *&SaveInfo) {
  UseDefault = שקר;
  אם (EC)
    החזר שקר;

  // CurrentNode is null for empty documents, which is an error in case required
  // nodes are present.
  אם (!CurrentNode) {
    אם (Required)
      EC = make_error_code(errc::invalid_argument);
    אחרת
      UseDefault = אמת;
    החזר שקר;
  }

  MapHNode *MN = dyn_cast<MapHNode>(CurrentNode);
  אם (!MN) {
    אם (Required || !isa<EmptyHNode>(CurrentNode))
      setError(CurrentNode, "not a mapping");
    אחרת
      UseDefault = אמת;
    החזר שקר;
  }
  MN->ValidKeys.push_back(Key.str());
  HNode *Value = MN->Mapping[Key].first;
  אם (!Value) {
    אם (Required)
      setError(CurrentNode, Twine("missing required key '") + Key + "'");
    אחרת
      UseDefault = אמת;
    החזר שקר;
  }
  SaveInfo = CurrentNode;
  CurrentNode = Value;
  החזר אמת;
}

فراغ Input::postflightKey(فراغ *saveInfo) {
  CurrentNode = reinterpret_cast<HNode *>(saveInfo);
}

فراغ Input::endMapping() {
  אם (EC)
    החזר;
  // CurrentNode can be null if the document is empty.
  MapHNode *MN = dyn_cast_or_null<MapHNode>(CurrentNode);
  אם (!MN)
    החזר;
  עבור (ثابت auto &NN : MN->Mapping) {
    אם (!is_contained(MN->ValidKeys, NN.first())) {
      ثابت SMRange &ReportLoc = NN.second.second;
      אם (!AllowUnknownKeys) {
        setError(ReportLoc, Twine("unknown key '") + NN.first() + "'");
        اكسر;
      } אחרת
        reportWarning(ReportLoc, Twine("unknown key '") + NN.first() + "'");
    }
  }
}

فراغ Input::beginFlowMapping() { beginMapping(); }

فراغ Input::endFlowMapping() { endMapping(); }

unsigned Input::beginSequence() {
  אם (SequenceHNode *SQ = dyn_cast<SequenceHNode>(CurrentNode))
    החזר SQ->Entries.size();
  אם (isa<EmptyHNode>(CurrentNode))
    החזר 0;
  // Treat case where there's a scalar "null" value as an empty sequence.
  אם (ScalarHNode *SN = dyn_cast<ScalarHNode>(CurrentNode)) {
    אם (isNull(SN->value()))
      החזר 0;
  }
  // Any other type of HNode is an error.
  setError(CurrentNode, "not a sequence");
  החזר 0;
}

فراغ Input::endSequence() {
}

منطقي Input::preflightElement(unsigned Index, فراغ *&SaveInfo) {
  אם (EC)
    החזר שקר;
  אם (SequenceHNode *SQ = dyn_cast<SequenceHNode>(CurrentNode)) {
    SaveInfo = CurrentNode;
    CurrentNode = SQ->Entries[Index];
    החזר אמת;
  }
  החזר שקר;
}

فراغ Input::postflightElement(فراغ *SaveInfo) {
  CurrentNode = reinterpret_cast<HNode *>(SaveInfo);
}

unsigned Input::beginFlowSequence() { החזר beginSequence(); }

منطقي Input::preflightFlowElement(unsigned index, فراغ *&SaveInfo) {
  אם (EC)
    החזר שקר;
  אם (SequenceHNode *SQ = dyn_cast<SequenceHNode>(CurrentNode)) {
    SaveInfo = CurrentNode;
    CurrentNode = SQ->Entries[index];
    החזר אמת;
  }
  החזר שקר;
}

فراغ Input::postflightFlowElement(فراغ *SaveInfo) {
  CurrentNode = reinterpret_cast<HNode *>(SaveInfo);
}

فراغ Input::endFlowSequence() {
}

فراغ Input::beginEnumScalar() {
  ScalarMatchFound = שקר;
}

منطقي Input::matchEnumScalar(StringRef Str, منطقي) {
  אם (ScalarMatchFound)
    החזר שקר;
  אם (ScalarHNode *SN = dyn_cast<ScalarHNode>(CurrentNode)) {
    אם (SN->value() == Str) {
      ScalarMatchFound = אמת;
      החזר אמת;
    }
  }
  החזר שקר;
}

منطقي Input::matchEnumFallback() {
  אם (ScalarMatchFound)
    החזר שקר;
  ScalarMatchFound = אמת;
  החזר אמת;
}

فراغ Input::endEnumScalar() {
  אם (!ScalarMatchFound) {
    setError(CurrentNode, "unknown enumerated scalar");
  }
}

منطقي Input::beginBitSetScalar(منطقي &DoClear) {
  BitValuesUsed.clear();
  אם (SequenceHNode *SQ = dyn_cast<SequenceHNode>(CurrentNode)) {
    BitValuesUsed.resize(SQ->Entries.size());
  } אחרת {
    setError(CurrentNode, "expected sequence of bit values");
  }
  DoClear = אמת;
  החזר אמת;
}

منطقي Input::bitSetMatch(StringRef Str, منطقي) {
  אם (EC)
    החזר שקר;
  אם (SequenceHNode *SQ = dyn_cast<SequenceHNode>(CurrentNode)) {
    unsigned Index = 0;
    עבור (auto &N : SQ->Entries) {
      אם (ScalarHNode *SN = dyn_cast<ScalarHNode>(N)) {
        אם (SN->value() == Str) {
          BitValuesUsed[Index] = אמת;
          החזר אמת;
        }
      } אחרת {
        setError(CurrentNode, "unexpected scalar in sequence of bit values");
      }
      ++Index;
    }
  } אחרת {
    setError(CurrentNode, "expected sequence of bit values");
  }
  החזר שקר;
}

فراغ Input::endBitSetScalar() {
  אם (EC)
    החזר;
  אם (SequenceHNode *SQ = dyn_cast<SequenceHNode>(CurrentNode)) {
    أكد(BitValuesUsed.size() == SQ->Entries.size());
    עבור (unsigned i = 0; i < SQ->Entries.size(); ++i) {
      אם (!BitValuesUsed[i]) {
        setError(SQ->Entries[i], "unknown bit value");
        החזר;
      }
    }
  }
}

فراغ Input::scalarString(StringRef &S, QuotingType) {
  אם (ScalarHNode *SN = dyn_cast<ScalarHNode>(CurrentNode)) {
    S = SN->value();
  } אחרת {
    setError(CurrentNode, "unexpected scalar");
  }
}

فراغ Input::blockScalarString(StringRef &S) { scalarString(S, QuotingType::None); }

فراغ Input::scalarTag(std::string &Tag) {
  Tag = CurrentNode->_node->getVerbatimTag();
}

فراغ Input::setError(HNode *hnode, ثابت Twine &message) {
  أكد(hnode && "HNode must not be NULL");
  setError(hnode->_node, message);
}

NodeKind Input::getNodeKind() {
  אם (isa<ScalarHNode>(CurrentNode))
    החזר NodeKind::Scalar;
  אחרת אם (isa<MapHNode>(CurrentNode))
    החזר NodeKind::Map;
  אחרת אם (isa<SequenceHNode>(CurrentNode))
    החזר NodeKind::Sequence;
  llvm_unreachable("Unsupported node kind");
}

فراغ Input::setError(Node *node, ثابت Twine &message) {
  Strm->printError(node, message);
  EC = make_error_code(errc::invalid_argument);
}

فراغ Input::setError(ثابت SMRange &range, ثابت Twine &message) {
  Strm->printError(range, message);
  EC = make_error_code(errc::invalid_argument);
}

فراغ Input::reportWarning(HNode *hnode, ثابت Twine &message) {
  أكد(hnode && "HNode must not be NULL");
  Strm->printError(hnode->_node, message, SourceMgr::DK_Warning);
}

فراغ Input::reportWarning(Node *node, ثابت Twine &message) {
  Strm->printError(node, message, SourceMgr::DK_Warning);
}

فراغ Input::reportWarning(ثابت SMRange &range, ثابت Twine &message) {
  Strm->printError(range, message, SourceMgr::DK_Warning);
}

فراغ Input::releaseHNodeBuffers() {
  EmptyHNodeAllocator.DestroyAll();
  ScalarHNodeAllocator.DestroyAll();
  SequenceHNodeAllocator.DestroyAll();
  MapHNodeAllocator.DestroyAll();
}

فراغ Input::saveAliasHNode(Node *N, HNode *HN) {
  StringRef Anchor = N->getAnchor();
  אם (!Anchor.empty())
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
  بدل (N->getType()) {
  حالة Node::NK_Scalar: {
    ScalarNode *SN = dyn_cast<ScalarNode>(N);
    StringRef KeyStr = SN->getValue(StringStorage);
    אם (!StringStorage.empty()) {
      // Copy string to permanent storage
      KeyStr = StringStorage.str().copy(StringAllocator);
    }
    auto *SHNode = جديد (ScalarHNodeAllocator.Allocate()) ScalarHNode(N, KeyStr);
    saveAliasHNode(SN, SHNode);
    החזר SHNode;
  }
  حالة Node::NK_BlockScalar: {
    BlockScalarNode *BSN = dyn_cast<BlockScalarNode>(N);
    StringRef ValueCopy = BSN->getValue().copy(StringAllocator);
    auto *BSHNode =
        جديد (ScalarHNodeAllocator.Allocate()) ScalarHNode(N, ValueCopy);
    saveAliasHNode(BSN, BSHNode);
    החזר BSHNode;
  }
  حالة Node::NK_Sequence: {
    SequenceNode *SQ = dyn_cast<SequenceNode>(N);
    auto SQHNode = جديد (SequenceHNodeAllocator.Allocate()) SequenceHNode(N);
    עבור (Node &SN : *SQ) {
      auto Entry = createHNodes(&SN);
      אם (EC)
        اكسر;
      SQHNode->Entries.push_back(Entry);
    }
    saveAliasHNode(SQ, SQHNode);
    החזר SQHNode;
  }
  حالة Node::NK_Mapping: {
    MappingNode *Map = dyn_cast<MappingNode>(N);
    auto mapHNode = جديد (MapHNodeAllocator.Allocate()) MapHNode(N);
    עבור (KeyValueNode &KVN : *Map) {
      Node *KeyNode = KVN.getKey();
      ScalarNode *Key = dyn_cast_if_present<ScalarNode>(KeyNode);
      Node *Value = KVN.getValue();
      אם (!Key || !Value) {
        אם (!Key)
          setError(KeyNode, "Map key must be a scalar");
        אם (!Value)
          setError(KeyNode, "Map value must not be empty");
        اكسر;
      }
      StringStorage.clear();
      StringRef KeyStr = Key->getValue(StringStorage);
      אם (!StringStorage.empty()) {
        // Copy string to permanent storage
        KeyStr = StringStorage.str().copy(StringAllocator);
      }
      אם (mapHNode->Mapping.count(KeyStr))
        // From YAML spec: "The content of a mapping node is an unordered set of
        // key/value node pairs, with the restriction that each of the keys is
        // unique."
        setError(KeyNode, Twine("duplicated mapping key '") + KeyStr + "'");
      auto ValueHNode = createHNodes(Value);
      אם (EC)
        اكسر;
      mapHNode->Mapping[KeyStr] =
          std::make_pair(std::move(ValueHNode), KeyNode->getSourceRange());
    }
    saveAliasHNode(Map, mapHNode);
    החזר std::move(mapHNode);
  }
  حالة Node::NK_Null:
    // TODO: Anchor is not set for NullNode in the parser. Update the parser to
    // faithfully preserve anchors.
    החזר جديد (EmptyHNodeAllocator.Allocate()) EmptyHNode(N);
  حالة Node::NK_Alias: {
    AliasNode *AN = dyn_cast<AliasNode>(N);
    auto AliasName = AN->getName();
    auto AHN = AliasMap.find(AliasName);
    אם (AHN == AliasMap.end()) {
      setError(AN, Twine("undefined alias '" + AliasName + "'"));
      החזר nullptr;
    }
    החזר AHN->second;
  }
  افتراضي:
    setError(N, "unknown node kind");
    החזר nullptr;
  }
}

فراغ Input::setError(ثابت Twine &Message) {
  setError(CurrentNode, Message);
}

فراغ Input::setAllowUnknownKeys(منطقي Allow) { AllowUnknownKeys = Allow; }

منطقي Input::canElideEmptySequence() {
  החזר שקר;
}

//===----------------------------------------------------------------------===//
//  Output
//===----------------------------------------------------------------------===//

Output::Output(raw_ostream &yout, فراغ *context, صحيح WrapColumn)
    : IO(context), Out(yout), WrapColumn(WrapColumn) {}

Output::~Output() = افتراضي;

منطقي Output::outputting() ثابت {
  החזר אמת;
}

فراغ Output::beginMapping() {
  StateStack.push_back(inMapFirstKey);
  PaddingBeforeContainer = Padding;
  Padding = "\n";
}

منطقي Output::mapTag(StringRef Tag, منطقي Use) {
  אם (Use) {
    // If this tag is being written inside a sequence we should write the start
    // of the sequence before writing the tag, otherwise the tag won't be
    // attached to the element in the sequence, but rather the sequence itself.
    منطقي SequenceElement = שקר;
    אם (StateStack.size() > 1) {
      auto &E = StateStack[StateStack.size() - 2];
      SequenceElement = inSeqAnyElement(E) || inFlowSeqAnyElement(E);
    }
    אם (SequenceElement && StateStack.back() == inMapFirstKey) {
      newLineCheck();
    } אחרת {
      output(" ");
    }
    output(Tag);
    אם (SequenceElement) {
      // If we're writing the tag during the first element of a map, the tag
      // takes the place of the first element in the sequence.
      אם (StateStack.back() == inMapFirstKey) {
        StateStack.pop_back();
        StateStack.push_back(inMapOtherKey);
      }
      // Tags inside maps in sequences should act as keys in the map from a
      // formatting perspective, so we always want a newline in a sequence.
      Padding = "\n";
    }
  }
  החזר Use;
}

فراغ Output::endMapping() {
  // If we did not map anything, we should explicitly emit an empty map
  אם (StateStack.back() == inMapFirstKey) {
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

منطقي Output::preflightKey(StringRef Key, منطقي Required, منطقي SameAsDefault,
                          منطقي &UseDefault, فراغ *&SaveInfo) {
  UseDefault = שקר;
  SaveInfo = nullptr;
  אם (Required || !SameAsDefault || WriteDefaultValues) {
    auto State = StateStack.back();
    אם (State == inFlowMapFirstKey || State == inFlowMapOtherKey) {
      flowKey(Key);
    } אחרת {
      newLineCheck();
      paddedKey(Key);
    }
    החזר אמת;
  }
  החזר שקר;
}

فراغ Output::postflightKey(فراغ *) {
  אם (StateStack.back() == inMapFirstKey) {
    StateStack.pop_back();
    StateStack.push_back(inMapOtherKey);
  } אחרת אם (StateStack.back() == inFlowMapFirstKey) {
    StateStack.pop_back();
    StateStack.push_back(inFlowMapOtherKey);
  }
}

فراغ Output::beginFlowMapping() {
  StateStack.push_back(inFlowMapFirstKey);
  newLineCheck();
  ColumnAtMapFlowStart = Column;
  output("{ ");
}

فراغ Output::endFlowMapping() {
  StateStack.pop_back();
  outputUpToEndOfLine(" }");
}

فراغ Output::beginDocuments() {
  outputUpToEndOfLine("---");
}

منطقي Output::preflightDocument(unsigned index) {
  אם (index > 0)
    outputUpToEndOfLine("\n---");
  החזר אמת;
}

فراغ Output::postflightDocument() {
}

فراغ Output::endDocuments() {
  output("\n...\n");
}

unsigned Output::beginSequence() {
  StateStack.push_back(inSeqFirstElement);
  PaddingBeforeContainer = Padding;
  Padding = "\n";
  החזר 0;
}

فراغ Output::endSequence() {
  // If we did not emit anything, we should explicitly emit an empty sequence
  אם (StateStack.back() == inSeqFirstElement) {
    Padding = PaddingBeforeContainer;
    newLineCheck(/*EmptySequence=*/אמת);
    output("[]");
    Padding = "\n";
  }
  StateStack.pop_back();
}

منطقي Output::preflightElement(unsigned, فراغ *&SaveInfo) {
  SaveInfo = nullptr;
  החזר אמת;
}

فراغ Output::postflightElement(فراغ *) {
  אם (StateStack.back() == inSeqFirstElement) {
    StateStack.pop_back();
    StateStack.push_back(inSeqOtherElement);
  } אחרת אם (StateStack.back() == inFlowSeqFirstElement) {
    StateStack.pop_back();
    StateStack.push_back(inFlowSeqOtherElement);
  }
}

unsigned Output::beginFlowSequence() {
  StateStack.push_back(inFlowSeqFirstElement);
  newLineCheck();
  ColumnAtFlowStart = Column;
  output("[ ");
  NeedFlowSequenceComma = שקר;
  החזר 0;
}

فراغ Output::endFlowSequence() {
  StateStack.pop_back();
  outputUpToEndOfLine(" ]");
}

منطقي Output::preflightFlowElement(unsigned, فراغ *&SaveInfo) {
  אם (NeedFlowSequenceComma)
    output(", ");
  אם (WrapColumn && Column > WrapColumn) {
    output("\n");
    עבור (صحيح i = 0; i < ColumnAtFlowStart; ++i)
      output(" ");
    Column = ColumnAtFlowStart;
    output("  ");
  }
  SaveInfo = nullptr;
  החזר אמת;
}

فراغ Output::postflightFlowElement(فراغ *) {
  NeedFlowSequenceComma = אמת;
}

فراغ Output::beginEnumScalar() {
  EnumerationMatchFound = שקר;
}

منطقي Output::matchEnumScalar(StringRef Str, منطقي Match) {
  אם (Match && !EnumerationMatchFound) {
    newLineCheck();
    outputUpToEndOfLine(Str);
    EnumerationMatchFound = אמת;
  }
  החזר שקר;
}

منطقي Output::matchEnumFallback() {
  אם (EnumerationMatchFound)
    החזר שקר;
  EnumerationMatchFound = אמת;
  החזר אמת;
}

فراغ Output::endEnumScalar() {
  אם (!EnumerationMatchFound)
    llvm_unreachable("bad runtime enum value");
}

منطقي Output::beginBitSetScalar(منطقي &DoClear) {
  newLineCheck();
  output("[ ");
  NeedBitValueComma = שקר;
  DoClear = שקר;
  החזר אמת;
}

منطقي Output::bitSetMatch(StringRef Str, منطقي Matches) {
  אם (Matches) {
    אם (NeedBitValueComma)
      output(", ");
    output(Str);
    NeedBitValueComma = אמת;
  }
  החזר שקר;
}

فراغ Output::endBitSetScalar() {
  outputUpToEndOfLine(" ]");
}

فراغ Output::scalarString(StringRef &S, QuotingType MustQuote) {
  newLineCheck();
  אם (S.empty()) {
    // Print '' for the empty string because leaving the field empty is not
    // allowed.
    outputUpToEndOfLine("''");
    החזר;
  }
  output(S, MustQuote);
  outputUpToEndOfLine("");
}

فراغ Output::blockScalarString(StringRef &S) {
  אם (!StateStack.empty())
    newLineCheck();
  output(" |");

  unsigned Indent = StateStack.empty() ? 1 : StateStack.size();

  auto Buffer = MemoryBuffer::getMemBuffer(S, "", שקר);
  עבור (line_iterator Lines(*Buffer, שקר); !Lines.is_at_end(); ++Lines) {
    outputNewLine();
    עבור (unsigned I = 0; I < Indent; ++I) {
      output("  ");
    }
    output(*Lines);
  }
  outputUpToEndOfLine("");
}

فراغ Output::scalarTag(std::string &Tag) {
  אם (Tag.empty())
    החזר;
  newLineCheck();
  output(Tag);
  output(" ");
}

فراغ Output::setError(ثابت Twine &message) {
}

std::error_code Output::error() { החזר {}; }

منطقي Output::canElideEmptySequence() {
  // Normally, with an optional key/value where the value is an empty sequence,
  // the whole key/value can be not written.  But, that produces wrong yaml
  // if the key/value is the only thing in the map and the map is used in
  // a sequence.  This detects if the this sequence is the first key/value
  // in map that itself is embedded in a sequence.
  אם (StateStack.size() < 2)
    החזר אמת;
  אם (StateStack.back() != inMapFirstKey)
    החזר אמת;
  החזר !inSeqAnyElement(StateStack[StateStack.size() - 2]);
}

فراغ Output::output(StringRef s) {
  Column += s.size();
  Out << s;
}

فراغ Output::output(StringRef S, QuotingType MustQuote) {
  אם (MustQuote == QuotingType::None) {
    // Only quote if we must.
    output(S);
    החזר;
  }

  StringLiteral Quote = MustQuote == QuotingType::Single ? StringLiteral("'")
                                                         : StringLiteral("\"");
  output(Quote); // Starting quote.

  // When using double-quoted strings (and only in that case), non-printable
  // characters may be present, and will be escaped using a variety of
  // unicode-scalar and special short-form escapes. This is handled in
  // yaml::escape.
  אם (MustQuote == QuotingType::Double) {
    output(yaml::escape(S, /* EscapePrintable= */ שקר));
    output(Quote);
    החזר;
  }

  unsigned i = 0;
  unsigned j = 0;
  unsigned End = S.size();
  ثابت حرف *Base = S.data();

  // When using single-quoted strings, any single quote ' must be doubled to be
  // escaped.
  כלעוד (j < End) {
    אם (S[j] == '\'') {                   // Escape quotes.
      output(StringRef(&Base[i], j - i)); // "flush".
      output(StringLiteral("''"));        // Print it as ''
      i = j + 1;
    }
    ++j;
  }
  output(StringRef(&Base[i], j - i));
  output(Quote); // Ending quote.
}

فراغ Output::outputUpToEndOfLine(StringRef s) {
  output(s);
  אם (StateStack.empty() || (!inFlowSeqAnyElement(StateStack.back()) &&
                             !inFlowMapAnyKey(StateStack.back())))
    Padding = "\n";
}

فراغ Output::outputNewLine() {
  Out << "\n";
  Column = 0;
}

// if seq at top, indent as if map, then add "- "
// if seq in middle, use "- " if firstKey, else use "  "
//

فراغ Output::newLineCheck(منطقي EmptySequence) {
  אם (Padding != "\n") {
    output(Padding);
    Padding = {};
    החזר;
  }
  outputNewLine();
  Padding = {};

  אם (StateStack.size() == 0 || EmptySequence)
    החזר;

  unsigned Indent = StateStack.size() - 1;
  منطقي PossiblyNestedSeq = שקר;
  auto I = StateStack.rbegin(), E = StateStack.rend();

  אם (inSeqAnyElement(*I)) {
    PossiblyNestedSeq = אמת; // Not possibly but always.
    ++Indent;
  } אחרת אם (*I == inMapFirstKey || *I == inFlowMapFirstKey ||
             inFlowSeqAnyElement(*I)) {
    PossiblyNestedSeq = אמת;
    ++I; // Skip back().
  }

  unsigned OutputDashCount = 0;
  אם (PossiblyNestedSeq) {
    // Count up consecutive inSeqFirstElement from the end, unless
    // inSeqFirstElement is the top of nested sequence.
    כלעוד (I != E) {
      // Don't count the top of nested sequence.
      אם (!inSeqAnyElement(*I))
        اكسر;

      ++OutputDashCount;

      // Stop counting if consecutive inSeqFirstElement ends.
      אם (*I++ != inSeqFirstElement)
        اكسر;
    }
  }

  עבור (unsigned I = OutputDashCount; I < Indent; ++I)
    output("  ");

  עבור (unsigned I = 0; I < OutputDashCount; ++I)
    output("- ");
}

فراغ Output::paddedKey(StringRef key) {
  output(key, needsQuotes(key, שקר));
  output(":");
  ثابت حرف *spaces = "                ";
  אם (key.size() < strlen(spaces))
    Padding = &spaces[key.size()];
  אחרת
    Padding = " ";
}

فراغ Output::flowKey(StringRef Key) {
  אם (StateStack.back() == inFlowMapOtherKey)
    output(", ");
  אם (WrapColumn && Column > WrapColumn) {
    output("\n");
    עבור (صحيح I = 0; I < ColumnAtMapFlowStart; ++I)
      output(" ");
    Column = ColumnAtMapFlowStart;
    output("  ");
  }
  output(Key, needsQuotes(Key, שקר));
  output(": ");
}

NodeKind Output::getNodeKind() { report_fatal_error("invalid call"); }

منطقي Output::inSeqAnyElement(InState State) {
  החזר State == inSeqFirstElement || State == inSeqOtherElement;
}

منطقي Output::inFlowSeqAnyElement(InState State) {
  החזר State == inFlowSeqFirstElement || State == inFlowSeqOtherElement;
}

منطقي Output::inMapAnyKey(InState State) {
  החזר State == inMapFirstKey || State == inMapOtherKey;
}

منطقي Output::inFlowMapAnyKey(InState State) {
  החזר State == inFlowMapFirstKey || State == inFlowMapOtherKey;
}

//===----------------------------------------------------------------------===//
//  traits for built-in types
//===----------------------------------------------------------------------===//

فراغ ScalarTraits<منطقي>::output(ثابت منطقي &Val, فراغ *, raw_ostream &Out) {
  Out << (Val ? "true" : "false");
}

StringRef ScalarTraits<منطقي>::input(StringRef Scalar, فراغ *, منطقي &Val) {
  אם (std::optional<منطقي> Parsed = parseBool(Scalar)) {
    Val = *Parsed;
    החזר StringRef();
  }
  החזר "invalid boolean";
}

فراغ ScalarTraits<StringRef>::output(ثابت StringRef &Val, فراغ *,
                                     raw_ostream &Out) {
  Out << Val;
}

StringRef ScalarTraits<StringRef>::input(StringRef Scalar, فراغ *,
                                         StringRef &Val) {
  Val = Scalar;
  החזר StringRef();
}

فراغ ScalarTraits<std::string>::output(ثابت std::string &Val, فراغ *,
                                       raw_ostream &Out) {
  Out << Val;
}

StringRef ScalarTraits<std::string>::input(StringRef Scalar, فراغ *,
                                           std::string &Val) {
  Val = Scalar.str();
  החזר StringRef();
}

فراغ ScalarTraits<uint8_t>::output(ثابت uint8_t &Val, فراغ *,
                                   raw_ostream &Out) {
  // use temp uin32_t because ostream thinks uint8_t is a character
  uint32_t Num = Val;
  Out << Num;
}

StringRef ScalarTraits<uint8_t>::input(StringRef Scalar, فراغ *, uint8_t &Val) {
  unsigned long long n;
  אם (getAsUnsignedInteger(Scalar, 0, n))
    החזר "invalid number";
  אם (n > 0xFF)
    החזר "out of range number";
  Val = n;
  החזר StringRef();
}

فراغ ScalarTraits<uint16_t>::output(ثابت uint16_t &Val, فراغ *,
                                    raw_ostream &Out) {
  Out << Val;
}

StringRef ScalarTraits<uint16_t>::input(StringRef Scalar, فراغ *,
                                        uint16_t &Val) {
  unsigned long long n;
  אם (getAsUnsignedInteger(Scalar, 0, n))
    החזר "invalid number";
  אם (n > 0xFFFF)
    החזר "out of range number";
  Val = n;
  החזר StringRef();
}

فراغ ScalarTraits<uint32_t>::output(ثابت uint32_t &Val, فراغ *,
                                    raw_ostream &Out) {
  Out << Val;
}

StringRef ScalarTraits<uint32_t>::input(StringRef Scalar, فراغ *,
                                        uint32_t &Val) {
  unsigned long long n;
  אם (getAsUnsignedInteger(Scalar, 0, n))
    החזר "invalid number";
  אם (n > 0xFFFFFFFFUL)
    החזר "out of range number";
  Val = n;
  החזר StringRef();
}

فراغ ScalarTraits<uint64_t>::output(ثابت uint64_t &Val, فراغ *,
                                    raw_ostream &Out) {
  Out << Val;
}

StringRef ScalarTraits<uint64_t>::input(StringRef Scalar, فراغ *,
                                        uint64_t &Val) {
  unsigned long long N;
  אם (getAsUnsignedInteger(Scalar, 0, N))
    החזר "invalid number";
  Val = N;
  החזר StringRef();
}

فراغ ScalarTraits<int8_t>::output(ثابت int8_t &Val, فراغ *, raw_ostream &Out) {
  // use temp in32_t because ostream thinks int8_t is a character
  int32_t Num = Val;
  Out << Num;
}

StringRef ScalarTraits<int8_t>::input(StringRef Scalar, فراغ *, int8_t &Val) {
  long long N;
  אם (getAsSignedInteger(Scalar, 0, N))
    החזר "invalid number";
  אם ((N > 127) || (N < -128))
    החזר "out of range number";
  Val = N;
  החזר StringRef();
}

فراغ ScalarTraits<int16_t>::output(ثابت int16_t &Val, فراغ *,
                                   raw_ostream &Out) {
  Out << Val;
}

StringRef ScalarTraits<int16_t>::input(StringRef Scalar, فراغ *, int16_t &Val) {
  long long N;
  אם (getAsSignedInteger(Scalar, 0, N))
    החזר "invalid number";
  אם ((N > INT16_MAX) || (N < INT16_MIN))
    החזר "out of range number";
  Val = N;
  החזר StringRef();
}

فراغ ScalarTraits<int32_t>::output(ثابت int32_t &Val, فراغ *,
                                   raw_ostream &Out) {
  Out << Val;
}

StringRef ScalarTraits<int32_t>::input(StringRef Scalar, فراغ *, int32_t &Val) {
  long long N;
  אם (getAsSignedInteger(Scalar, 0, N))
    החזר "invalid number";
  אם ((N > INT32_MAX) || (N < INT32_MIN))
    החזר "out of range number";
  Val = N;
  החזר StringRef();
}

فراغ ScalarTraits<int64_t>::output(ثابت int64_t &Val, فراغ *,
                                   raw_ostream &Out) {
  Out << Val;
}

StringRef ScalarTraits<int64_t>::input(StringRef Scalar, فراغ *, int64_t &Val) {
  long long N;
  אם (getAsSignedInteger(Scalar, 0, N))
    החזר "invalid number";
  Val = N;
  החזר StringRef();
}

فراغ ScalarTraits<مزدوج>::output(ثابت مزدوج &Val, فراغ *, raw_ostream &Out) {
  Out << format("%g", Val);
}

StringRef ScalarTraits<مزدوج>::input(StringRef Scalar, فراغ *, مزدوج &Val) {
  אם (to_float(Scalar, Val))
    החזר StringRef();
  החזר "invalid floating point number";
}

فراغ ScalarTraits<عائم>::output(ثابت عائم &Val, فراغ *, raw_ostream &Out) {
  Out << format("%g", Val);
}

StringRef ScalarTraits<عائم>::input(StringRef Scalar, فراغ *, عائم &Val) {
  אם (to_float(Scalar, Val))
    החזר StringRef();
  החזר "invalid floating point number";
}

فراغ ScalarTraits<Hex8>::output(ثابت Hex8 &Val, فراغ *, raw_ostream &Out) {
  Out << format("0x%" PRIX8, (uint8_t)Val);
}

StringRef ScalarTraits<Hex8>::input(StringRef Scalar, فراغ *, Hex8 &Val) {
  unsigned long long n;
  אם (getAsUnsignedInteger(Scalar, 0, n))
    החזר "invalid hex8 number";
  אם (n > 0xFF)
    החזר "out of range hex8 number";
  Val = n;
  החזר StringRef();
}

فراغ ScalarTraits<Hex16>::output(ثابت Hex16 &Val, فراغ *, raw_ostream &Out) {
  Out << format("0x%" PRIX16, (uint16_t)Val);
}

StringRef ScalarTraits<Hex16>::input(StringRef Scalar, فراغ *, Hex16 &Val) {
  unsigned long long n;
  אם (getAsUnsignedInteger(Scalar, 0, n))
    החזר "invalid hex16 number";
  אם (n > 0xFFFF)
    החזר "out of range hex16 number";
  Val = n;
  החזר StringRef();
}

فراغ ScalarTraits<Hex32>::output(ثابت Hex32 &Val, فراغ *, raw_ostream &Out) {
  Out << format("0x%" PRIX32, (uint32_t)Val);
}

StringRef ScalarTraits<Hex32>::input(StringRef Scalar, فراغ *, Hex32 &Val) {
  unsigned long long n;
  אם (getAsUnsignedInteger(Scalar, 0, n))
    החזר "invalid hex32 number";
  אם (n > 0xFFFFFFFFUL)
    החזר "out of range hex32 number";
  Val = n;
  החזר StringRef();
}

فراغ ScalarTraits<Hex64>::output(ثابت Hex64 &Val, فراغ *, raw_ostream &Out) {
  Out << format("0x%" PRIX64, (uint64_t)Val);
}

StringRef ScalarTraits<Hex64>::input(StringRef Scalar, فراغ *, Hex64 &Val) {
  unsigned long long Num;
  אם (getAsUnsignedInteger(Scalar, 0, Num))
    החזר "invalid hex64 number";
  Val = Num;
  החזר StringRef();
}

فراغ ScalarTraits<VersionTuple>::output(ثابت VersionTuple &Val, فراغ *,
                                        llvm::raw_ostream &Out) {
  Out << Val.getAsString();
}

StringRef ScalarTraits<VersionTuple>::input(StringRef Scalar, فراغ *,
                                            VersionTuple &Val) {
  אם (Val.tryParse(Scalar))
    החזר "invalid version format";
  החזר StringRef();
}
