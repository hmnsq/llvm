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
  أرجع Ctxt;
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
    : IO(Ctxt), Strm(جديد Stream(InputContent, SrcMgr, خطأ, &EC)) {
  𐤀𐤌 (DiagHandler)
    SrcMgr.setDiagHandler(DiagHandler, DiagHandlerCtxt);
  DocIterator = Strm->begin();
}

Input::Input(MemoryBufferRef Input, فراغ *Ctxt,
             SourceMgr::DiagHandlerTy DiagHandler, فراغ *DiagHandlerCtxt)
    : IO(Ctxt), Strm(جديد Stream(Input, SrcMgr, خطأ, &EC)) {
  𐤀𐤌 (DiagHandler)
    SrcMgr.setDiagHandler(DiagHandler, DiagHandlerCtxt);
  DocIterator = Strm->begin();
}

Input::~Input() = افتراضي;

std::error_code Input::error() { أرجع EC; }

منطقي Input::outputting() ثابت {
  أرجع خطأ;
}

منطقي Input::setCurrentDocument() {
  𐤀𐤌 (DocIterator != Strm->end()) {
    Node *N = DocIterator->getRoot();
    𐤀𐤌 (!N) {
      EC = make_error_code(errc::invalid_argument);
      أرجع خطأ;
    }

    𐤀𐤌 (isa<NullNode>(N)) {
      // Empty files are allowed and ignored
      ++DocIterator;
      أرجع setCurrentDocument();
    }
    releaseHNodeBuffers();
    TopNode = createHNodes(N);
    CurrentNode = TopNode;
    أرجع true;
  }
  أرجع خطأ;
}

منطقي Input::nextDocument() {
  أرجع ++DocIterator != Strm->end();
}

ثابت Node *Input::getCurrentNode() ثابت {
  أرجع CurrentNode ? CurrentNode->_node : nullptr;
}

منطقي Input::mapTag(StringRef Tag, منطقي Default) {
  // CurrentNode can be null if setCurrentDocument() was unable to
  // parse the document because it was invalid or empty.
  𐤀𐤌 (!CurrentNode)
    أرجع خطأ;

  std::string foundTag = CurrentNode->_node->getVerbatimTag();
  𐤀𐤌 (foundTag.empty()) {
    // If no tag found and 'Tag' is the default, say it was found.
    أرجع Default;
  }
  // Return true iff found tag matches supplied tag.
  أرجع Tag == foundTag;
}

فراغ Input::beginMapping() {
  𐤀𐤌 (EC)
    أرجع;
  // CurrentNode can be null if the document is empty.
  MapHNode *MN = dyn_cast_or_null<MapHNode>(CurrentNode);
  𐤀𐤌 (MN) {
    MN->ValidKeys.clear();
  }
}

std::vector<StringRef> Input::keys() {
  MapHNode *MN = dyn_cast<MapHNode>(CurrentNode);
  std::vector<StringRef> Ret;
  𐤀𐤌 (!MN) {
    setError(CurrentNode, "not a mapping");
    أرجع Ret;
  }
  لكل (auto &P : MN->Mapping)
    Ret.push_back(P.first());
  أرجع Ret;
}

منطقي Input::preflightKey(StringRef Key, منطقي Required, منطقي, منطقي &UseDefault,
                         فراغ *&SaveInfo) {
  UseDefault = خطأ;
  𐤀𐤌 (EC)
    أرجع خطأ;

  // CurrentNode is null for empty documents, which is an error in case required
  // nodes are present.
  𐤀𐤌 (!CurrentNode) {
    𐤀𐤌 (Required)
      EC = make_error_code(errc::invalid_argument);
    وإلا
      UseDefault = true;
    أرجع خطأ;
  }

  MapHNode *MN = dyn_cast<MapHNode>(CurrentNode);
  𐤀𐤌 (!MN) {
    𐤀𐤌 (Required || !isa<EmptyHNode>(CurrentNode))
      setError(CurrentNode, "not a mapping");
    وإلا
      UseDefault = true;
    أرجع خطأ;
  }
  MN->ValidKeys.push_back(Key.str());
  HNode *Value = MN->Mapping[Key].first;
  𐤀𐤌 (!Value) {
    𐤀𐤌 (Required)
      setError(CurrentNode, Twine("missing required key '") + Key + "'");
    وإلا
      UseDefault = true;
    أرجع خطأ;
  }
  SaveInfo = CurrentNode;
  CurrentNode = Value;
  أرجع true;
}

فراغ Input::postflightKey(فراغ *saveInfo) {
  CurrentNode = reinterpret_cast<HNode *>(saveInfo);
}

فراغ Input::endMapping() {
  𐤀𐤌 (EC)
    أرجع;
  // CurrentNode can be null if the document is empty.
  MapHNode *MN = dyn_cast_or_null<MapHNode>(CurrentNode);
  𐤀𐤌 (!MN)
    أرجع;
  لكل (ثابت auto &NN : MN->Mapping) {
    𐤀𐤌 (!is_contained(MN->ValidKeys, NN.first())) {
      ثابت SMRange &ReportLoc = NN.second.second;
      𐤀𐤌 (!AllowUnknownKeys) {
        setError(ReportLoc, Twine("unknown key '") + NN.first() + "'");
        اكسر;
      } وإلا
        reportWarning(ReportLoc, Twine("unknown key '") + NN.first() + "'");
    }
  }
}

فراغ Input::beginFlowMapping() { beginMapping(); }

فراغ Input::endFlowMapping() { endMapping(); }

unsigned Input::beginSequence() {
  𐤀𐤌 (SequenceHNode *SQ = dyn_cast<SequenceHNode>(CurrentNode))
    أرجع SQ->Entries.size();
  𐤀𐤌 (isa<EmptyHNode>(CurrentNode))
    أرجع 0;
  // Treat case where there's a scalar "null" value as an empty sequence.
  𐤀𐤌 (ScalarHNode *SN = dyn_cast<ScalarHNode>(CurrentNode)) {
    𐤀𐤌 (isNull(SN->value()))
      أرجع 0;
  }
  // Any other type of HNode is an error.
  setError(CurrentNode, "not a sequence");
  أرجع 0;
}

فراغ Input::endSequence() {
}

منطقي Input::preflightElement(unsigned Index, فراغ *&SaveInfo) {
  𐤀𐤌 (EC)
    أرجع خطأ;
  𐤀𐤌 (SequenceHNode *SQ = dyn_cast<SequenceHNode>(CurrentNode)) {
    SaveInfo = CurrentNode;
    CurrentNode = SQ->Entries[Index];
    أرجع true;
  }
  أرجع خطأ;
}

فراغ Input::postflightElement(فراغ *SaveInfo) {
  CurrentNode = reinterpret_cast<HNode *>(SaveInfo);
}

unsigned Input::beginFlowSequence() { أرجع beginSequence(); }

منطقي Input::preflightFlowElement(unsigned index, فراغ *&SaveInfo) {
  𐤀𐤌 (EC)
    أرجع خطأ;
  𐤀𐤌 (SequenceHNode *SQ = dyn_cast<SequenceHNode>(CurrentNode)) {
    SaveInfo = CurrentNode;
    CurrentNode = SQ->Entries[index];
    أرجع true;
  }
  أرجع خطأ;
}

فراغ Input::postflightFlowElement(فراغ *SaveInfo) {
  CurrentNode = reinterpret_cast<HNode *>(SaveInfo);
}

فراغ Input::endFlowSequence() {
}

فراغ Input::beginEnumScalar() {
  ScalarMatchFound = خطأ;
}

منطقي Input::matchEnumScalar(StringRef Str, منطقي) {
  𐤀𐤌 (ScalarMatchFound)
    أرجع خطأ;
  𐤀𐤌 (ScalarHNode *SN = dyn_cast<ScalarHNode>(CurrentNode)) {
    𐤀𐤌 (SN->value() == Str) {
      ScalarMatchFound = true;
      أرجع true;
    }
  }
  أرجع خطأ;
}

منطقي Input::matchEnumFallback() {
  𐤀𐤌 (ScalarMatchFound)
    أرجع خطأ;
  ScalarMatchFound = true;
  أرجع true;
}

فراغ Input::endEnumScalar() {
  𐤀𐤌 (!ScalarMatchFound) {
    setError(CurrentNode, "unknown enumerated scalar");
  }
}

منطقي Input::beginBitSetScalar(منطقي &DoClear) {
  BitValuesUsed.clear();
  𐤀𐤌 (SequenceHNode *SQ = dyn_cast<SequenceHNode>(CurrentNode)) {
    BitValuesUsed.resize(SQ->Entries.size());
  } وإلا {
    setError(CurrentNode, "expected sequence of bit values");
  }
  DoClear = true;
  أرجع true;
}

منطقي Input::bitSetMatch(StringRef Str, منطقي) {
  𐤀𐤌 (EC)
    أرجع خطأ;
  𐤀𐤌 (SequenceHNode *SQ = dyn_cast<SequenceHNode>(CurrentNode)) {
    unsigned Index = 0;
    لكل (auto &N : SQ->Entries) {
      𐤀𐤌 (ScalarHNode *SN = dyn_cast<ScalarHNode>(N)) {
        𐤀𐤌 (SN->value() == Str) {
          BitValuesUsed[Index] = true;
          أرجع true;
        }
      } وإلا {
        setError(CurrentNode, "unexpected scalar in sequence of bit values");
      }
      ++Index;
    }
  } وإلا {
    setError(CurrentNode, "expected sequence of bit values");
  }
  أرجع خطأ;
}

فراغ Input::endBitSetScalar() {
  𐤀𐤌 (EC)
    أرجع;
  𐤀𐤌 (SequenceHNode *SQ = dyn_cast<SequenceHNode>(CurrentNode)) {
    أكد(BitValuesUsed.size() == SQ->Entries.size());
    لكل (unsigned i = 0; i < SQ->Entries.size(); ++i) {
      𐤀𐤌 (!BitValuesUsed[i]) {
        setError(SQ->Entries[i], "unknown bit value");
        أرجع;
      }
    }
  }
}

فراغ Input::scalarString(StringRef &S, QuotingType) {
  𐤀𐤌 (ScalarHNode *SN = dyn_cast<ScalarHNode>(CurrentNode)) {
    S = SN->value();
  } وإلا {
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
  𐤀𐤌 (isa<ScalarHNode>(CurrentNode))
    أرجع NodeKind::Scalar;
  وإلا 𐤀𐤌 (isa<MapHNode>(CurrentNode))
    أرجع NodeKind::Map;
  وإلا 𐤀𐤌 (isa<SequenceHNode>(CurrentNode))
    أرجع NodeKind::Sequence;
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
  𐤀𐤌 (!Anchor.empty())
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
    𐤀𐤌 (!StringStorage.empty()) {
      // Copy string to permanent storage
      KeyStr = StringStorage.str().copy(StringAllocator);
    }
    auto *SHNode = جديد (ScalarHNodeAllocator.Allocate()) ScalarHNode(N, KeyStr);
    saveAliasHNode(SN, SHNode);
    أرجع SHNode;
  }
  حالة Node::NK_BlockScalar: {
    BlockScalarNode *BSN = dyn_cast<BlockScalarNode>(N);
    StringRef ValueCopy = BSN->getValue().copy(StringAllocator);
    auto *BSHNode =
        جديد (ScalarHNodeAllocator.Allocate()) ScalarHNode(N, ValueCopy);
    saveAliasHNode(BSN, BSHNode);
    أرجع BSHNode;
  }
  حالة Node::NK_Sequence: {
    SequenceNode *SQ = dyn_cast<SequenceNode>(N);
    auto SQHNode = جديد (SequenceHNodeAllocator.Allocate()) SequenceHNode(N);
    لكل (Node &SN : *SQ) {
      auto Entry = createHNodes(&SN);
      𐤀𐤌 (EC)
        اكسر;
      SQHNode->Entries.push_back(Entry);
    }
    saveAliasHNode(SQ, SQHNode);
    أرجع SQHNode;
  }
  حالة Node::NK_Mapping: {
    MappingNode *Map = dyn_cast<MappingNode>(N);
    auto mapHNode = جديد (MapHNodeAllocator.Allocate()) MapHNode(N);
    لكل (KeyValueNode &KVN : *Map) {
      Node *KeyNode = KVN.getKey();
      ScalarNode *Key = dyn_cast_if_present<ScalarNode>(KeyNode);
      Node *Value = KVN.getValue();
      𐤀𐤌 (!Key || !Value) {
        𐤀𐤌 (!Key)
          setError(KeyNode, "Map key must be a scalar");
        𐤀𐤌 (!Value)
          setError(KeyNode, "Map value must not be empty");
        اكسر;
      }
      StringStorage.clear();
      StringRef KeyStr = Key->getValue(StringStorage);
      𐤀𐤌 (!StringStorage.empty()) {
        // Copy string to permanent storage
        KeyStr = StringStorage.str().copy(StringAllocator);
      }
      𐤀𐤌 (mapHNode->Mapping.count(KeyStr))
        // From YAML spec: "The content of a mapping node is an unordered set of
        // key/value node pairs, with the restriction that each of the keys is
        // unique."
        setError(KeyNode, Twine("duplicated mapping key '") + KeyStr + "'");
      auto ValueHNode = createHNodes(Value);
      𐤀𐤌 (EC)
        اكسر;
      mapHNode->Mapping[KeyStr] =
          std::make_pair(std::move(ValueHNode), KeyNode->getSourceRange());
    }
    saveAliasHNode(Map, mapHNode);
    أرجع std::move(mapHNode);
  }
  حالة Node::NK_Null:
    // TODO: Anchor is not set for NullNode in the parser. Update the parser to
    // faithfully preserve anchors.
    أرجع جديد (EmptyHNodeAllocator.Allocate()) EmptyHNode(N);
  حالة Node::NK_Alias: {
    AliasNode *AN = dyn_cast<AliasNode>(N);
    auto AliasName = AN->getName();
    auto AHN = AliasMap.find(AliasName);
    𐤀𐤌 (AHN == AliasMap.end()) {
      setError(AN, Twine("undefined alias '" + AliasName + "'"));
      أرجع nullptr;
    }
    أرجع AHN->second;
  }
  افتراضي:
    setError(N, "unknown node kind");
    أرجع nullptr;
  }
}

فراغ Input::setError(ثابت Twine &Message) {
  setError(CurrentNode, Message);
}

فراغ Input::setAllowUnknownKeys(منطقي Allow) { AllowUnknownKeys = Allow; }

منطقي Input::canElideEmptySequence() {
  أرجع خطأ;
}

//===----------------------------------------------------------------------===//
//  Output
//===----------------------------------------------------------------------===//

Output::Output(raw_ostream &yout, فراغ *context, صحيح WrapColumn)
    : IO(context), Out(yout), WrapColumn(WrapColumn) {}

Output::~Output() = افتراضي;

منطقي Output::outputting() ثابت {
  أرجع true;
}

فراغ Output::beginMapping() {
  StateStack.push_back(inMapFirstKey);
  PaddingBeforeContainer = Padding;
  Padding = "\n";
}

منطقي Output::mapTag(StringRef Tag, منطقي Use) {
  𐤀𐤌 (Use) {
    // If this tag is being written inside a sequence we should write the start
    // of the sequence before writing the tag, otherwise the tag won't be
    // attached to the element in the sequence, but rather the sequence itself.
    منطقي SequenceElement = خطأ;
    𐤀𐤌 (StateStack.size() > 1) {
      auto &E = StateStack[StateStack.size() - 2];
      SequenceElement = inSeqAnyElement(E) || inFlowSeqAnyElement(E);
    }
    𐤀𐤌 (SequenceElement && StateStack.back() == inMapFirstKey) {
      newLineCheck();
    } وإلا {
      output(" ");
    }
    output(Tag);
    𐤀𐤌 (SequenceElement) {
      // If we're writing the tag during the first element of a map, the tag
      // takes the place of the first element in the sequence.
      𐤀𐤌 (StateStack.back() == inMapFirstKey) {
        StateStack.pop_back();
        StateStack.push_back(inMapOtherKey);
      }
      // Tags inside maps in sequences should act as keys in the map from a
      // formatting perspective, so we always want a newline in a sequence.
      Padding = "\n";
    }
  }
  أرجع Use;
}

فراغ Output::endMapping() {
  // If we did not map anything, we should explicitly emit an empty map
  𐤀𐤌 (StateStack.back() == inMapFirstKey) {
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
  UseDefault = خطأ;
  SaveInfo = nullptr;
  𐤀𐤌 (Required || !SameAsDefault || WriteDefaultValues) {
    auto State = StateStack.back();
    𐤀𐤌 (State == inFlowMapFirstKey || State == inFlowMapOtherKey) {
      flowKey(Key);
    } وإلا {
      newLineCheck();
      paddedKey(Key);
    }
    أرجع true;
  }
  أرجع خطأ;
}

فراغ Output::postflightKey(فراغ *) {
  𐤀𐤌 (StateStack.back() == inMapFirstKey) {
    StateStack.pop_back();
    StateStack.push_back(inMapOtherKey);
  } وإلا 𐤀𐤌 (StateStack.back() == inFlowMapFirstKey) {
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
  𐤀𐤌 (index > 0)
    outputUpToEndOfLine("\n---");
  أرجع true;
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
  أرجع 0;
}

فراغ Output::endSequence() {
  // If we did not emit anything, we should explicitly emit an empty sequence
  𐤀𐤌 (StateStack.back() == inSeqFirstElement) {
    Padding = PaddingBeforeContainer;
    newLineCheck(/*EmptySequence=*/true);
    output("[]");
    Padding = "\n";
  }
  StateStack.pop_back();
}

منطقي Output::preflightElement(unsigned, فراغ *&SaveInfo) {
  SaveInfo = nullptr;
  أرجع true;
}

فراغ Output::postflightElement(فراغ *) {
  𐤀𐤌 (StateStack.back() == inSeqFirstElement) {
    StateStack.pop_back();
    StateStack.push_back(inSeqOtherElement);
  } وإلا 𐤀𐤌 (StateStack.back() == inFlowSeqFirstElement) {
    StateStack.pop_back();
    StateStack.push_back(inFlowSeqOtherElement);
  }
}

unsigned Output::beginFlowSequence() {
  StateStack.push_back(inFlowSeqFirstElement);
  newLineCheck();
  ColumnAtFlowStart = Column;
  output("[ ");
  NeedFlowSequenceComma = خطأ;
  أرجع 0;
}

فراغ Output::endFlowSequence() {
  StateStack.pop_back();
  outputUpToEndOfLine(" ]");
}

منطقي Output::preflightFlowElement(unsigned, فراغ *&SaveInfo) {
  𐤀𐤌 (NeedFlowSequenceComma)
    output(", ");
  𐤀𐤌 (WrapColumn && Column > WrapColumn) {
    output("\n");
    لكل (صحيح i = 0; i < ColumnAtFlowStart; ++i)
      output(" ");
    Column = ColumnAtFlowStart;
    output("  ");
  }
  SaveInfo = nullptr;
  أرجع true;
}

فراغ Output::postflightFlowElement(فراغ *) {
  NeedFlowSequenceComma = true;
}

فراغ Output::beginEnumScalar() {
  EnumerationMatchFound = خطأ;
}

منطقي Output::matchEnumScalar(StringRef Str, منطقي Match) {
  𐤀𐤌 (Match && !EnumerationMatchFound) {
    newLineCheck();
    outputUpToEndOfLine(Str);
    EnumerationMatchFound = true;
  }
  أرجع خطأ;
}

منطقي Output::matchEnumFallback() {
  𐤀𐤌 (EnumerationMatchFound)
    أرجع خطأ;
  EnumerationMatchFound = true;
  أرجع true;
}

فراغ Output::endEnumScalar() {
  𐤀𐤌 (!EnumerationMatchFound)
    llvm_unreachable("bad runtime enum value");
}

منطقي Output::beginBitSetScalar(منطقي &DoClear) {
  newLineCheck();
  output("[ ");
  NeedBitValueComma = خطأ;
  DoClear = خطأ;
  أرجع true;
}

منطقي Output::bitSetMatch(StringRef Str, منطقي Matches) {
  𐤀𐤌 (Matches) {
    𐤀𐤌 (NeedBitValueComma)
      output(", ");
    output(Str);
    NeedBitValueComma = true;
  }
  أرجع خطأ;
}

فراغ Output::endBitSetScalar() {
  outputUpToEndOfLine(" ]");
}

فراغ Output::scalarString(StringRef &S, QuotingType MustQuote) {
  newLineCheck();
  𐤀𐤌 (S.empty()) {
    // Print '' for the empty string because leaving the field empty is not
    // allowed.
    outputUpToEndOfLine("''");
    أرجع;
  }
  output(S, MustQuote);
  outputUpToEndOfLine("");
}

فراغ Output::blockScalarString(StringRef &S) {
  𐤀𐤌 (!StateStack.empty())
    newLineCheck();
  output(" |");

  unsigned Indent = StateStack.empty() ? 1 : StateStack.size();

  auto Buffer = MemoryBuffer::getMemBuffer(S, "", خطأ);
  لكل (line_iterator Lines(*Buffer, خطأ); !Lines.is_at_end(); ++Lines) {
    outputNewLine();
    لكل (unsigned I = 0; I < Indent; ++I) {
      output("  ");
    }
    output(*Lines);
  }
  outputUpToEndOfLine("");
}

فراغ Output::scalarTag(std::string &Tag) {
  𐤀𐤌 (Tag.empty())
    أرجع;
  newLineCheck();
  output(Tag);
  output(" ");
}

فراغ Output::setError(ثابت Twine &message) {
}

std::error_code Output::error() { أرجع {}; }

منطقي Output::canElideEmptySequence() {
  // Normally, with an optional key/value where the value is an empty sequence,
  // the whole key/value can be not written.  But, that produces wrong yaml
  // if the key/value is the only thing in the map and the map is used in
  // a sequence.  This detects if the this sequence is the first key/value
  // in map that itself is embedded in a sequence.
  𐤀𐤌 (StateStack.size() < 2)
    أرجع true;
  𐤀𐤌 (StateStack.back() != inMapFirstKey)
    أرجع true;
  أرجع !inSeqAnyElement(StateStack[StateStack.size() - 2]);
}

فراغ Output::output(StringRef s) {
  Column += s.size();
  Out << s;
}

فراغ Output::output(StringRef S, QuotingType MustQuote) {
  𐤀𐤌 (MustQuote == QuotingType::None) {
    // Only quote if we must.
    output(S);
    أرجع;
  }

  StringLiteral Quote = MustQuote == QuotingType::Single ? StringLiteral("'")
                                                         : StringLiteral("\"");
  output(Quote); // Starting quote.

  // When using double-quoted strings (and only in that case), non-printable
  // characters may be present, and will be escaped using a variety of
  // unicode-scalar and special short-form escapes. This is handled in
  // yaml::escape.
  𐤀𐤌 (MustQuote == QuotingType::Double) {
    output(yaml::escape(S, /* EscapePrintable= */ خطأ));
    output(Quote);
    أرجع;
  }

  unsigned i = 0;
  unsigned j = 0;
  unsigned End = S.size();
  ثابت حرف *Base = S.data();

  // When using single-quoted strings, any single quote ' must be doubled to be
  // escaped.
  طالما (j < End) {
    𐤀𐤌 (S[j] == '\'') {                   // Escape quotes.
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
  𐤀𐤌 (StateStack.empty() || (!inFlowSeqAnyElement(StateStack.back()) &&
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
  𐤀𐤌 (Padding != "\n") {
    output(Padding);
    Padding = {};
    أرجع;
  }
  outputNewLine();
  Padding = {};

  𐤀𐤌 (StateStack.size() == 0 || EmptySequence)
    أرجع;

  unsigned Indent = StateStack.size() - 1;
  منطقي PossiblyNestedSeq = خطأ;
  auto I = StateStack.rbegin(), E = StateStack.rend();

  𐤀𐤌 (inSeqAnyElement(*I)) {
    PossiblyNestedSeq = true; // Not possibly but always.
    ++Indent;
  } وإلا 𐤀𐤌 (*I == inMapFirstKey || *I == inFlowMapFirstKey ||
             inFlowSeqAnyElement(*I)) {
    PossiblyNestedSeq = true;
    ++I; // Skip back().
  }

  unsigned OutputDashCount = 0;
  𐤀𐤌 (PossiblyNestedSeq) {
    // Count up consecutive inSeqFirstElement from the end, unless
    // inSeqFirstElement is the top of nested sequence.
    طالما (I != E) {
      // Don't count the top of nested sequence.
      𐤀𐤌 (!inSeqAnyElement(*I))
        اكسر;

      ++OutputDashCount;

      // Stop counting if consecutive inSeqFirstElement ends.
      𐤀𐤌 (*I++ != inSeqFirstElement)
        اكسر;
    }
  }

  لكل (unsigned I = OutputDashCount; I < Indent; ++I)
    output("  ");

  لكل (unsigned I = 0; I < OutputDashCount; ++I)
    output("- ");
}

فراغ Output::paddedKey(StringRef key) {
  output(key, needsQuotes(key, خطأ));
  output(":");
  ثابت حرف *spaces = "                ";
  𐤀𐤌 (key.size() < strlen(spaces))
    Padding = &spaces[key.size()];
  وإلا
    Padding = " ";
}

فراغ Output::flowKey(StringRef Key) {
  𐤀𐤌 (StateStack.back() == inFlowMapOtherKey)
    output(", ");
  𐤀𐤌 (WrapColumn && Column > WrapColumn) {
    output("\n");
    لكل (صحيح I = 0; I < ColumnAtMapFlowStart; ++I)
      output(" ");
    Column = ColumnAtMapFlowStart;
    output("  ");
  }
  output(Key, needsQuotes(Key, خطأ));
  output(": ");
}

NodeKind Output::getNodeKind() { report_fatal_error("invalid call"); }

منطقي Output::inSeqAnyElement(InState State) {
  أرجع State == inSeqFirstElement || State == inSeqOtherElement;
}

منطقي Output::inFlowSeqAnyElement(InState State) {
  أرجع State == inFlowSeqFirstElement || State == inFlowSeqOtherElement;
}

منطقي Output::inMapAnyKey(InState State) {
  أرجع State == inMapFirstKey || State == inMapOtherKey;
}

منطقي Output::inFlowMapAnyKey(InState State) {
  أرجع State == inFlowMapFirstKey || State == inFlowMapOtherKey;
}

//===----------------------------------------------------------------------===//
//  traits for built-in types
//===----------------------------------------------------------------------===//

فراغ ScalarTraits<منطقي>::output(ثابت منطقي &Val, فراغ *, raw_ostream &Out) {
  Out << (Val ? "true" : "false");
}

StringRef ScalarTraits<منطقي>::input(StringRef Scalar, فراغ *, منطقي &Val) {
  𐤀𐤌 (std::optional<منطقي> Parsed = parseBool(Scalar)) {
    Val = *Parsed;
    أرجع StringRef();
  }
  أرجع "invalid boolean";
}

فراغ ScalarTraits<StringRef>::output(ثابت StringRef &Val, فراغ *,
                                     raw_ostream &Out) {
  Out << Val;
}

StringRef ScalarTraits<StringRef>::input(StringRef Scalar, فراغ *,
                                         StringRef &Val) {
  Val = Scalar;
  أرجع StringRef();
}

فراغ ScalarTraits<std::string>::output(ثابت std::string &Val, فراغ *,
                                       raw_ostream &Out) {
  Out << Val;
}

StringRef ScalarTraits<std::string>::input(StringRef Scalar, فراغ *,
                                           std::string &Val) {
  Val = Scalar.str();
  أرجع StringRef();
}

فراغ ScalarTraits<uint8_t>::output(ثابت uint8_t &Val, فراغ *,
                                   raw_ostream &Out) {
  // use temp uin32_t because ostream thinks uint8_t is a character
  uint32_t Num = Val;
  Out << Num;
}

StringRef ScalarTraits<uint8_t>::input(StringRef Scalar, فراغ *, uint8_t &Val) {
  unsigned long long n;
  𐤀𐤌 (getAsUnsignedInteger(Scalar, 0, n))
    أرجع "invalid number";
  𐤀𐤌 (n > 0xFF)
    أرجع "out of range number";
  Val = n;
  أرجع StringRef();
}

فراغ ScalarTraits<uint16_t>::output(ثابت uint16_t &Val, فراغ *,
                                    raw_ostream &Out) {
  Out << Val;
}

StringRef ScalarTraits<uint16_t>::input(StringRef Scalar, فراغ *,
                                        uint16_t &Val) {
  unsigned long long n;
  𐤀𐤌 (getAsUnsignedInteger(Scalar, 0, n))
    أرجع "invalid number";
  𐤀𐤌 (n > 0xFFFF)
    أرجع "out of range number";
  Val = n;
  أرجع StringRef();
}

فراغ ScalarTraits<uint32_t>::output(ثابت uint32_t &Val, فراغ *,
                                    raw_ostream &Out) {
  Out << Val;
}

StringRef ScalarTraits<uint32_t>::input(StringRef Scalar, فراغ *,
                                        uint32_t &Val) {
  unsigned long long n;
  𐤀𐤌 (getAsUnsignedInteger(Scalar, 0, n))
    أرجع "invalid number";
  𐤀𐤌 (n > 0xFFFFFFFFUL)
    أرجع "out of range number";
  Val = n;
  أرجع StringRef();
}

فراغ ScalarTraits<uint64_t>::output(ثابت uint64_t &Val, فراغ *,
                                    raw_ostream &Out) {
  Out << Val;
}

StringRef ScalarTraits<uint64_t>::input(StringRef Scalar, فراغ *,
                                        uint64_t &Val) {
  unsigned long long N;
  𐤀𐤌 (getAsUnsignedInteger(Scalar, 0, N))
    أرجع "invalid number";
  Val = N;
  أرجع StringRef();
}

فراغ ScalarTraits<int8_t>::output(ثابت int8_t &Val, فراغ *, raw_ostream &Out) {
  // use temp in32_t because ostream thinks int8_t is a character
  int32_t Num = Val;
  Out << Num;
}

StringRef ScalarTraits<int8_t>::input(StringRef Scalar, فراغ *, int8_t &Val) {
  long long N;
  𐤀𐤌 (getAsSignedInteger(Scalar, 0, N))
    أرجع "invalid number";
  𐤀𐤌 ((N > 127) || (N < -128))
    أرجع "out of range number";
  Val = N;
  أرجع StringRef();
}

فراغ ScalarTraits<int16_t>::output(ثابت int16_t &Val, فراغ *,
                                   raw_ostream &Out) {
  Out << Val;
}

StringRef ScalarTraits<int16_t>::input(StringRef Scalar, فراغ *, int16_t &Val) {
  long long N;
  𐤀𐤌 (getAsSignedInteger(Scalar, 0, N))
    أرجع "invalid number";
  𐤀𐤌 ((N > INT16_MAX) || (N < INT16_MIN))
    أرجع "out of range number";
  Val = N;
  أرجع StringRef();
}

فراغ ScalarTraits<int32_t>::output(ثابت int32_t &Val, فراغ *,
                                   raw_ostream &Out) {
  Out << Val;
}

StringRef ScalarTraits<int32_t>::input(StringRef Scalar, فراغ *, int32_t &Val) {
  long long N;
  𐤀𐤌 (getAsSignedInteger(Scalar, 0, N))
    أرجع "invalid number";
  𐤀𐤌 ((N > INT32_MAX) || (N < INT32_MIN))
    أرجع "out of range number";
  Val = N;
  أرجع StringRef();
}

فراغ ScalarTraits<int64_t>::output(ثابت int64_t &Val, فراغ *,
                                   raw_ostream &Out) {
  Out << Val;
}

StringRef ScalarTraits<int64_t>::input(StringRef Scalar, فراغ *, int64_t &Val) {
  long long N;
  𐤀𐤌 (getAsSignedInteger(Scalar, 0, N))
    أرجع "invalid number";
  Val = N;
  أرجع StringRef();
}

فراغ ScalarTraits<مزدوج>::output(ثابت مزدوج &Val, فراغ *, raw_ostream &Out) {
  Out << format("%g", Val);
}

StringRef ScalarTraits<مزدوج>::input(StringRef Scalar, فراغ *, مزدوج &Val) {
  𐤀𐤌 (to_float(Scalar, Val))
    أرجع StringRef();
  أرجع "invalid floating point number";
}

فراغ ScalarTraits<عائم>::output(ثابت عائم &Val, فراغ *, raw_ostream &Out) {
  Out << format("%g", Val);
}

StringRef ScalarTraits<عائم>::input(StringRef Scalar, فراغ *, عائم &Val) {
  𐤀𐤌 (to_float(Scalar, Val))
    أرجع StringRef();
  أرجع "invalid floating point number";
}

فراغ ScalarTraits<Hex8>::output(ثابت Hex8 &Val, فراغ *, raw_ostream &Out) {
  Out << format("0x%" PRIX8, (uint8_t)Val);
}

StringRef ScalarTraits<Hex8>::input(StringRef Scalar, فراغ *, Hex8 &Val) {
  unsigned long long n;
  𐤀𐤌 (getAsUnsignedInteger(Scalar, 0, n))
    أرجع "invalid hex8 number";
  𐤀𐤌 (n > 0xFF)
    أرجع "out of range hex8 number";
  Val = n;
  أرجع StringRef();
}

فراغ ScalarTraits<Hex16>::output(ثابت Hex16 &Val, فراغ *, raw_ostream &Out) {
  Out << format("0x%" PRIX16, (uint16_t)Val);
}

StringRef ScalarTraits<Hex16>::input(StringRef Scalar, فراغ *, Hex16 &Val) {
  unsigned long long n;
  𐤀𐤌 (getAsUnsignedInteger(Scalar, 0, n))
    أرجع "invalid hex16 number";
  𐤀𐤌 (n > 0xFFFF)
    أرجع "out of range hex16 number";
  Val = n;
  أرجع StringRef();
}

فراغ ScalarTraits<Hex32>::output(ثابت Hex32 &Val, فراغ *, raw_ostream &Out) {
  Out << format("0x%" PRIX32, (uint32_t)Val);
}

StringRef ScalarTraits<Hex32>::input(StringRef Scalar, فراغ *, Hex32 &Val) {
  unsigned long long n;
  𐤀𐤌 (getAsUnsignedInteger(Scalar, 0, n))
    أرجع "invalid hex32 number";
  𐤀𐤌 (n > 0xFFFFFFFFUL)
    أرجع "out of range hex32 number";
  Val = n;
  أرجع StringRef();
}

فراغ ScalarTraits<Hex64>::output(ثابت Hex64 &Val, فراغ *, raw_ostream &Out) {
  Out << format("0x%" PRIX64, (uint64_t)Val);
}

StringRef ScalarTraits<Hex64>::input(StringRef Scalar, فراغ *, Hex64 &Val) {
  unsigned long long Num;
  𐤀𐤌 (getAsUnsignedInteger(Scalar, 0, Num))
    أرجع "invalid hex64 number";
  Val = Num;
  أرجع StringRef();
}

فراغ ScalarTraits<VersionTuple>::output(ثابت VersionTuple &Val, فراغ *,
                                        llvm::raw_ostream &Out) {
  Out << Val.getAsString();
}

StringRef ScalarTraits<VersionTuple>::input(StringRef Scalar, فراغ *,
                                            VersionTuple &Val) {
  𐤀𐤌 (Val.tryParse(Scalar))
    أرجع "invalid version format";
  أرجع StringRef();
}
