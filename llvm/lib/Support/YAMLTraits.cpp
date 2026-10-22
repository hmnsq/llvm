//===- lib/Support/YAMLTraits.cpp -----------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#شامل "llvm/Support/YAMLTraits.h"
#شامل "llvm/ADT/STLExtras.h"
#شامل "llvm/ADT/SmallString.h"
#شامل "llvm/ADT/StringExtras.h"
#شامل "llvm/ADT/StringRef.h"
#شامل "llvm/ADT/Twine.h"
#شامل "llvm/Support/Casting.h"
#شامل "llvm/Support/Errc.h"
#شامل "llvm/Support/ErrorHandling.h"
#شامل "llvm/Support/Format.h"
#شامل "llvm/Support/LineIterator.h"
#شامل "llvm/Support/MemoryBuffer.h"
#شامل "llvm/Support/VersionTuple.h"
#شامل "llvm/Support/YAMLParser.h"
#شامل "llvm/Support/raw_ostream.h"
#شامل <cassert>
#شامل <cstdint>
#شامل <cstring>
#شامل <string>
#شامل <vector>

using فضاینام llvm;
using فضاینام yaml;

//===----------------------------------------------------------------------===//
//  IO
//===----------------------------------------------------------------------===//

IO::IO(تهی *Context) : Ctxt(Context) {}

IO::~IO() = پیشفرض;

تهی *IO::getContext() ثابت {
  بازگشت Ctxt;
}

تهی IO::setContext(تهی *Context) {
  Ctxt = Context;
}

تهی IO::set_Allow_Unknown_کلیدها(منطقی Allow) {
  llvm_unreachable("Only supported for Input");
}

//===----------------------------------------------------------------------===//
//درون داد
//===----------------------------------------------------------------------===//

درون_داد::درون_داد(StringRef درون_داد_Content, تهی *Ctxt,
             SourceMgr::DiagHandlerTy DiagHandler, تهی *DiagHandlerCtxt)
    : IO(Ctxt), Strm(جدید Stream(درون_داد_Content, SrcMgr, نادرست, &EC)) {
  اگر (DiagHandler)
    SrcMgr.setDiagHandler(DiagHandler, DiagHandlerCtxt);
  DocIterator = Strm->begin();
}

درون_داد::درون_داد(MemoryBufferRef درون_داد, تهی *Ctxt,
             SourceMgr::DiagHandlerTy DiagHandler, تهی *DiagHandlerCtxt)
    : IO(Ctxt), Strm(جدید Stream(درون_داد, SrcMgr, نادرست, &EC)) {
  اگر (DiagHandler)
    SrcMgr.setDiagHandler(DiagHandler, DiagHandlerCtxt);
  DocIterator = Strm->begin();
}

درون_داد::~درون_داد() = پیشفرض;

std::error_code درون_داد::error() { بازگشت EC; }

منطقی درون_داد::outputting() ثابت {
  بازگشت نادرست;
}

منطقی درون_داد::setCurrentDocument() {
  اگر (DocIterator != Strm->end()) {
    گره *N = DocIterator->get_ریشه();
    اگر (!N) {
      EC = make_error_code(errc::invalid_argument);
      بازگشت نادرست;
    }

    اگر (isa<Null_گره>(N)) {
      // Empty files are allowed and ignored
      ++DocIterator;
      بازگشت setCurrentDocument();
    }
    releaseHNodeBuffers();
    بالاترین_گره = createHNodes(N);
    Current_گره = بالاترین_گره;
    بازگشت درست;
  }
  بازگشت نادرست;
}

منطقی درون_داد::nextDocument() {
  بازگشت ++DocIterator != Strm->end();
}

ثابت گره *درون_داد::get_Current_گره() ثابت {
  بازگشت Current_گره ? Current_گره->گره_2 : nullptr;
}

منطقی درون_داد::mapTag(StringRef Tag, منطقی Default) {
  // CurrentNode can be null if setCurrentDocument() was unable to
  // parse the document because it was invalid or empty.
  اگر (!Current_گره)
    بازگشت نادرست;

  std::string foundTag = Current_گره->گره_2->getVerbatimTag();
  اگر (foundTag.empty()) {
    // If no tag found and 'Tag' is the default, say it was found.
    بازگشت Default;
  }
  // Return true iff found tag matches supplied tag.
  بازگشت Tag == foundTag;
}

تهی درون_داد::beginMapping() {
  اگر (EC)
    بازگشت;
  // CurrentNode can be null if the document is empty.
  MapHNode *MN = dyn_cast_or_null<MapHNode>(Current_گره);
  اگر (MN) {
    MN->Valid_کلیدها.clear();
  }
}

std::vector<StringRef> درون_داد::کلیدها() {
  MapHNode *MN = dyn_cast<MapHNode>(Current_گره);
  std::vector<StringRef> Ret;
  اگر (!MN) {
    setError(Current_گره, "not a mapping");
    بازگشت Ret;
  }
  برای (auto &P : MN->Mapping)
    Ret.افزودن_back(P.first());
  بازگشت Ret;
}

منطقی درون_داد::preflight_کلید(StringRef کلید, منطقی Required, منطقی, منطقی &UseDefault,
                         تهی *&SaveInfo) {
  UseDefault = نادرست;
  اگر (EC)
    بازگشت نادرست;

  // CurrentNode is null for empty documents, which is an error in case required
  // nodes are present.
  اگر (!Current_گره) {
    اگر (Required)
      EC = make_error_code(errc::invalid_argument);
    وگرنه
      UseDefault = درست;
    بازگشت نادرست;
  }

  MapHNode *MN = dyn_cast<MapHNode>(Current_گره);
  اگر (!MN) {
    اگر (Required || !isa<EmptyHNode>(Current_گره))
      setError(Current_گره, "not a mapping");
    وگرنه
      UseDefault = درست;
    بازگشت نادرست;
  }
  MN->Valid_کلیدها.افزودن_back(کلید.str());
  HNode *مقدار = MN->Mapping[کلید].first;
  اگر (!مقدار) {
    اگر (Required)
      setError(Current_گره, Twine("missing required key '") + کلید + "'");
    وگرنه
      UseDefault = درست;
    بازگشت نادرست;
  }
  SaveInfo = Current_گره;
  Current_گره = مقدار;
  بازگشت درست;
}

تهی درون_داد::postflight_کلید(تهی *saveInfo) {
  Current_گره = reinterpret_cast<HNode *>(saveInfo);
}

تهی درون_داد::endMapping() {
  اگر (EC)
    بازگشت;
  // CurrentNode can be null if the document is empty.
  MapHNode *MN = dyn_cast_or_null<MapHNode>(Current_گره);
  اگر (!MN)
    بازگشت;
  برای (ثابت auto &NN : MN->Mapping) {
    اگر (!is_contained(MN->Valid_کلیدها, NN.first())) {
      ثابت SMRange &ReportLoc = NN.second.second;
      اگر (!Allow_Unknown_کلیدها) {
        setError(ReportLoc, Twine("unknown key '") + NN.first() + "'");
        بشکن;
      } وگرنه
        reportWarning(ReportLoc, Twine("unknown key '") + NN.first() + "'");
    }
  }
}

تهی درون_داد::beginFlowMapping() { beginMapping(); }

تهی درون_داد::endFlowMapping() { endMapping(); }

unsigned درون_داد::beginSequence() {
  اگر (SequenceHNode *SQ = dyn_cast<SequenceHNode>(Current_گره))
    بازگشت SQ->Entries.size();
  اگر (isa<EmptyHNode>(Current_گره))
    بازگشت 0;
  // Treat case where there's a scalar "null" value as an empty sequence.
  اگر (ScalarHNode *SN = dyn_cast<ScalarHNode>(Current_گره)) {
    اگر (isNull(SN->مقدار_2()))
      بازگشت 0;
  }
  // Any other type of HNode is an error.
  setError(Current_گره, "not a sequence");
  بازگشت 0;
}

تهی درون_داد::endSequence() {
}

منطقی درون_داد::preflightElement(unsigned Index, تهی *&SaveInfo) {
  اگر (EC)
    بازگشت نادرست;
  اگر (SequenceHNode *SQ = dyn_cast<SequenceHNode>(Current_گره)) {
    SaveInfo = Current_گره;
    Current_گره = SQ->Entries[Index];
    بازگشت درست;
  }
  بازگشت نادرست;
}

تهی درون_داد::postflightElement(تهی *SaveInfo) {
  Current_گره = reinterpret_cast<HNode *>(SaveInfo);
}

unsigned درون_داد::beginFlowSequence() { بازگشت beginSequence(); }

منطقی درون_داد::preflightFlowElement(unsigned index, تهی *&SaveInfo) {
  اگر (EC)
    بازگشت نادرست;
  اگر (SequenceHNode *SQ = dyn_cast<SequenceHNode>(Current_گره)) {
    SaveInfo = Current_گره;
    Current_گره = SQ->Entries[index];
    بازگشت درست;
  }
  بازگشت نادرست;
}

تهی درون_داد::postflightFlowElement(تهی *SaveInfo) {
  Current_گره = reinterpret_cast<HNode *>(SaveInfo);
}

تهی درون_داد::endFlowSequence() {
}

تهی درون_داد::beginEnumScalar() {
  ScalarMatchFound = نادرست;
}

منطقی درون_داد::matchEnumScalar(StringRef Str, منطقی) {
  اگر (ScalarMatchFound)
    بازگشت نادرست;
  اگر (ScalarHNode *SN = dyn_cast<ScalarHNode>(Current_گره)) {
    اگر (SN->مقدار_2() == Str) {
      ScalarMatchFound = درست;
      بازگشت درست;
    }
  }
  بازگشت نادرست;
}

منطقی درون_داد::matchEnumFallback() {
  اگر (ScalarMatchFound)
    بازگشت نادرست;
  ScalarMatchFound = درست;
  بازگشت درست;
}

تهی درون_داد::endEnumScalar() {
  اگر (!ScalarMatchFound) {
    setError(Current_گره, "unknown enumerated scalar");
  }
}

منطقی درون_داد::beginBitSetScalar(منطقی &DoClear) {
  Bit_مقادیر_Used.clear();
  اگر (SequenceHNode *SQ = dyn_cast<SequenceHNode>(Current_گره)) {
    Bit_مقادیر_Used.resize(SQ->Entries.size());
  } وگرنه {
    setError(Current_گره, "expected sequence of bit values");
  }
  DoClear = درست;
  بازگشت درست;
}

منطقی درون_داد::bitSetMatch(StringRef Str, منطقی) {
  اگر (EC)
    بازگشت نادرست;
  اگر (SequenceHNode *SQ = dyn_cast<SequenceHNode>(Current_گره)) {
    unsigned Index = 0;
    برای (auto &N : SQ->Entries) {
      اگر (ScalarHNode *SN = dyn_cast<ScalarHNode>(N)) {
        اگر (SN->مقدار_2() == Str) {
          Bit_مقادیر_Used[Index] = درست;
          بازگشت درست;
        }
      } وگرنه {
        setError(Current_گره, "unexpected scalar in sequence of bit values");
      }
      ++Index;
    }
  } وگرنه {
    setError(Current_گره, "expected sequence of bit values");
  }
  بازگشت نادرست;
}

تهی درون_داد::endBitSetScalar() {
  اگر (EC)
    بازگشت;
  اگر (SequenceHNode *SQ = dyn_cast<SequenceHNode>(Current_گره)) {
    أكد(Bit_مقادیر_Used.size() == SQ->Entries.size());
    برای (unsigned i = 0; i < SQ->Entries.size(); ++i) {
      اگر (!Bit_مقادیر_Used[i]) {
        setError(SQ->Entries[i], "unknown bit value");
        بازگشت;
      }
    }
  }
}

تهی درون_داد::scalarString(StringRef &S, QuotingType) {
  اگر (ScalarHNode *SN = dyn_cast<ScalarHNode>(Current_گره)) {
    S = SN->مقدار_2();
  } وگرنه {
    setError(Current_گره, "unexpected scalar");
  }
}

تهی درون_داد::blockScalarString(StringRef &S) { scalarString(S, QuotingType::None); }

تهی درون_داد::scalarTag(std::string &Tag) {
  Tag = Current_گره->گره_2->getVerbatimTag();
}

تهی درون_داد::setError(HNode *hnode, ثابت Twine &message) {
  أكد(hnode && "HNode must not be NULL");
  setError(hnode->گره_2, message);
}

گره_Kind درون_داد::get_گره_Kind() {
  اگر (isa<ScalarHNode>(Current_گره))
    بازگشت گره_Kind::Scalar;
  وگرنه اگر (isa<MapHNode>(Current_گره))
    بازگشت گره_Kind::Map;
  وگرنه اگر (isa<SequenceHNode>(Current_گره))
    بازگشت گره_Kind::Sequence;
  llvm_unreachable("Unsupported node kind");
}

تهی درون_داد::setError(گره *گره_3, ثابت Twine &message) {
  Strm->printError(گره_3, message);
  EC = make_error_code(errc::invalid_argument);
}

تهی درون_داد::setError(ثابت SMRange &range, ثابت Twine &message) {
  Strm->printError(range, message);
  EC = make_error_code(errc::invalid_argument);
}

تهی درون_داد::reportWarning(HNode *hnode, ثابت Twine &message) {
  أكد(hnode && "HNode must not be NULL");
  Strm->printError(hnode->گره_2, message, SourceMgr::DK_Warning);
}

تهی درون_داد::reportWarning(گره *گره_3, ثابت Twine &message) {
  Strm->printError(گره_3, message, SourceMgr::DK_Warning);
}

تهی درون_داد::reportWarning(ثابت SMRange &range, ثابت Twine &message) {
  Strm->printError(range, message, SourceMgr::DK_Warning);
}

تهی درون_داد::releaseHNodeBuffers() {
  EmptyHNodeAllocator.DestroyAll();
  ScalarHNodeAllocator.DestroyAll();
  SequenceHNodeAllocator.DestroyAll();
  MapHNodeAllocator.DestroyAll();
}

تهی درون_داد::saveAliasHNode(گره *N, HNode *HN) {
  StringRef Anchor = N->getAnchor();
  اگر (!Anchor.empty())
    // YAML 1.2.2 - 3.2.2.2. Anchors and Aliases:
    //
    // An alias event refers to the most recent event in the serialization
    // having the specified anchor. Therefore, anchors need not be unique within
    // a serialization. In addition, an anchor need not have an alias node
    // referring to it.
    AliasMap[Anchor] = HN;
}

درون_داد::HNode *درون_داد::createHNodes(گره *N) {
  SmallString<128> StringStorage;
  انتخاب (N->getType()) {
  حالت گره::NK_Scalar: {
    Scalar_گره *SN = dyn_cast<Scalar_گره>(N);
    StringRef کلید_Str = SN->get_مقدار(StringStorage);
    اگر (!StringStorage.empty()) {
      // Copy string to permanent storage
      کلید_Str = StringStorage.str().copy(StringAllocator);
    }
    auto *SHNode = جدید (ScalarHNodeAllocator.Allocate()) ScalarHNode(N, کلید_Str);
    saveAliasHNode(SN, SHNode);
    بازگشت SHNode;
  }
  حالت گره::NK_BlockScalar: {
    Block_Scalar_گره *BSN = dyn_cast<Block_Scalar_گره>(N);
    StringRef مقدار_Copy = BSN->get_مقدار().copy(StringAllocator);
    auto *BSHNode =
        جدید (ScalarHNodeAllocator.Allocate()) ScalarHNode(N, مقدار_Copy);
    saveAliasHNode(BSN, BSHNode);
    بازگشت BSHNode;
  }
  حالت گره::NK_Sequence: {
    Sequence_گره *SQ = dyn_cast<Sequence_گره>(N);
    auto SQHNode = جدید (SequenceHNodeAllocator.Allocate()) SequenceHNode(N);
    برای (گره &SN : *SQ) {
      auto Entry = createHNodes(&SN);
      اگر (EC)
        بشکن;
      SQHNode->Entries.افزودن_back(Entry);
    }
    saveAliasHNode(SQ, SQHNode);
    بازگشت SQHNode;
  }
  حالت گره::NK_Mapping: {
    Mapping_گره *Map = dyn_cast<Mapping_گره>(N);
    auto mapHNode = جدید (MapHNodeAllocator.Allocate()) MapHNode(N);
    برای (کلید_مقدار_گره &KVN : *Map) {
      گره *کلید_گره = KVN.get_کلید();
      Scalar_گره *کلید = dyn_cast_if_present<Scalar_گره>(کلید_گره);
      گره *مقدار = KVN.get_مقدار();
      اگر (!کلید || !مقدار) {
        اگر (!کلید)
          setError(کلید_گره, "Map key must be a scalar");
        اگر (!مقدار)
          setError(کلید_گره, "Map value must not be empty");
        بشکن;
      }
      StringStorage.clear();
      StringRef کلید_Str = کلید->get_مقدار(StringStorage);
      اگر (!StringStorage.empty()) {
        // Copy string to permanent storage
        کلید_Str = StringStorage.str().copy(StringAllocator);
      }
      اگر (mapHNode->Mapping.شمار(کلید_Str))
        // From YAML spec: "The content of a mapping node is an unordered set of
        // key/value node pairs, with the restriction that each of the keys is
        // unique."
        setError(کلید_گره, Twine("duplicated mapping key '") + کلید_Str + "'");
      auto مقدار_HNode = createHNodes(مقدار);
      اگر (EC)
        بشکن;
      mapHNode->Mapping[کلید_Str] =
          std::make_pair(std::move(مقدار_HNode), کلید_گره->getSourceRange());
    }
    saveAliasHNode(Map, mapHNode);
    بازگشت std::move(mapHNode);
  }
  حالت گره::NK_Null:
    // TODO: Anchor is not set for NullNode in the parser. Update the parser to
    // faithfully preserve anchors.
    بازگشت جدید (EmptyHNodeAllocator.Allocate()) EmptyHNode(N);
  حالت گره::NK_Alias: {
    Alias_گره *AN = dyn_cast<Alias_گره>(N);
    auto AliasName = AN->getName();
    auto AHN = AliasMap.find(AliasName);
    اگر (AHN == AliasMap.end()) {
      setError(AN, Twine("undefined alias '" + AliasName + "'"));
      بازگشت nullptr;
    }
    بازگشت AHN->second;
  }
  پیشفرض:
    setError(N, "unknown node kind");
    بازگشت nullptr;
  }
}

تهی درون_داد::setError(ثابت Twine &Message) {
  setError(Current_گره, Message);
}

تهی درون_داد::set_Allow_Unknown_کلیدها(منطقی Allow) { Allow_Unknown_کلیدها = Allow; }

منطقی درون_داد::canElideEmptySequence() {
  بازگشت نادرست;
}

//===----------------------------------------------------------------------===//
//  Output
//===----------------------------------------------------------------------===//

Output::Output(raw_ostream &yout, تهی *context, صحیح WrapColumn)
    : IO(context), Out(yout), WrapColumn(WrapColumn) {}

Output::~Output() = پیشفرض;

منطقی Output::outputting() ثابت {
  بازگشت درست;
}

تهی Output::beginMapping() {
  State_پشته.افزودن_back(in_Map_First_کلید);
  PaddingBeforeContainer = Padding;
  Padding = "\n";
}

منطقی Output::mapTag(StringRef Tag, منطقی Use) {
  اگر (Use) {
    // If this tag is being written inside a sequence we should write the start
    // of the sequence before writing the tag, otherwise the tag won't be
    // attached to the element in the sequence, but rather the sequence itself.
    منطقی SequenceElement = نادرست;
    اگر (State_پشته.size() > 1) {
      auto &E = State_پشته[State_پشته.size() - 2];
      SequenceElement = inSeqAnyElement(E) || inFlowSeqAnyElement(E);
    }
    اگر (SequenceElement && State_پشته.back() == in_Map_First_کلید) {
      new_سطر_Check();
    } وگرنه {
      output(" ");
    }
    output(Tag);
    اگر (SequenceElement) {
      // If we're writing the tag during the first element of a map, the tag
      // takes the place of the first element in the sequence.
      اگر (State_پشته.back() == in_Map_First_کلید) {
        State_پشته.برداشت_back();
        State_پشته.افزودن_back(in_Map_Other_کلید);
      }
      // Tags inside maps in sequences should act as keys in the map from a
      // formatting perspective, so we always want a newline in a sequence.
      Padding = "\n";
    }
  }
  بازگشت Use;
}

تهی Output::endMapping() {
  // If we did not map anything, we should explicitly emit an empty map
  اگر (State_پشته.back() == in_Map_First_کلید) {
    Padding = PaddingBeforeContainer;
    new_سطر_Check();
    output("{}");
    Padding = "\n";
  }
  State_پشته.برداشت_back();
}

std::vector<StringRef> Output::کلیدها() {
  report_fatal_error("invalid call");
}

منطقی Output::preflight_کلید(StringRef کلید, منطقی Required, منطقی SameAsDefault,
                          منطقی &UseDefault, تهی *&SaveInfo) {
  UseDefault = نادرست;
  SaveInfo = nullptr;
  اگر (Required || !SameAsDefault || Write_Default_مقادیر) {
    auto State = State_پشته.back();
    اگر (State == in_Flow_Map_First_کلید || State == in_Flow_Map_Other_کلید) {
      flow_کلید(کلید);
    } وگرنه {
      new_سطر_Check();
      padded_کلید(کلید);
    }
    بازگشت درست;
  }
  بازگشت نادرست;
}

تهی Output::postflight_کلید(تهی *) {
  اگر (State_پشته.back() == in_Map_First_کلید) {
    State_پشته.برداشت_back();
    State_پشته.افزودن_back(in_Map_Other_کلید);
  } وگرنه اگر (State_پشته.back() == in_Flow_Map_First_کلید) {
    State_پشته.برداشت_back();
    State_پشته.افزودن_back(in_Flow_Map_Other_کلید);
  }
}

تهی Output::beginFlowMapping() {
  State_پشته.افزودن_back(in_Flow_Map_First_کلید);
  new_سطر_Check();
  ColumnAtMapFlowStart = Column;
  output("{ ");
}

تهی Output::endFlowMapping() {
  State_پشته.برداشت_back();
  output_Up_To_End_Of_سطر(" }");
}

تهی Output::beginDocuments() {
  output_Up_To_End_Of_سطر("---");
}

منطقی Output::preflightDocument(unsigned index) {
  اگر (index > 0)
    output_Up_To_End_Of_سطر("\n---");
  بازگشت درست;
}

تهی Output::postflightDocument() {
}

تهی Output::endDocuments() {
  output("\n...\n");
}

unsigned Output::beginSequence() {
  State_پشته.افزودن_back(inSeqFirstElement);
  PaddingBeforeContainer = Padding;
  Padding = "\n";
  بازگشت 0;
}

تهی Output::endSequence() {
  // If we did not emit anything, we should explicitly emit an empty sequence
  اگر (State_پشته.back() == inSeqFirstElement) {
    Padding = PaddingBeforeContainer;
    new_سطر_Check(/*EmptySequence=*/درست);
    output("[]");
    Padding = "\n";
  }
  State_پشته.برداشت_back();
}

منطقی Output::preflightElement(unsigned, تهی *&SaveInfo) {
  SaveInfo = nullptr;
  بازگشت درست;
}

تهی Output::postflightElement(تهی *) {
  اگر (State_پشته.back() == inSeqFirstElement) {
    State_پشته.برداشت_back();
    State_پشته.افزودن_back(inSeqOtherElement);
  } وگرنه اگر (State_پشته.back() == inFlowSeqFirstElement) {
    State_پشته.برداشت_back();
    State_پشته.افزودن_back(inFlowSeqOtherElement);
  }
}

unsigned Output::beginFlowSequence() {
  State_پشته.افزودن_back(inFlowSeqFirstElement);
  new_سطر_Check();
  ColumnAtFlowStart = Column;
  output("[ ");
  NeedFlowSequenceComma = نادرست;
  بازگشت 0;
}

تهی Output::endFlowSequence() {
  State_پشته.برداشت_back();
  output_Up_To_End_Of_سطر(" ]");
}

منطقی Output::preflightFlowElement(unsigned, تهی *&SaveInfo) {
  اگر (NeedFlowSequenceComma)
    output(", ");
  اگر (WrapColumn && Column > WrapColumn) {
    output("\n");
    برای (صحیح i = 0; i < ColumnAtFlowStart; ++i)
      output(" ");
    Column = ColumnAtFlowStart;
    output("  ");
  }
  SaveInfo = nullptr;
  بازگشت درست;
}

تهی Output::postflightFlowElement(تهی *) {
  NeedFlowSequenceComma = درست;
}

تهی Output::beginEnumScalar() {
  EnumerationMatchFound = نادرست;
}

منطقی Output::matchEnumScalar(StringRef Str, منطقی Match) {
  اگر (Match && !EnumerationMatchFound) {
    new_سطر_Check();
    output_Up_To_End_Of_سطر(Str);
    EnumerationMatchFound = درست;
  }
  بازگشت نادرست;
}

منطقی Output::matchEnumFallback() {
  اگر (EnumerationMatchFound)
    بازگشت نادرست;
  EnumerationMatchFound = درست;
  بازگشت درست;
}

تهی Output::endEnumScalar() {
  اگر (!EnumerationMatchFound)
    llvm_unreachable("bad runtime enum value");
}

منطقی Output::beginBitSetScalar(منطقی &DoClear) {
  new_سطر_Check();
  output("[ ");
  Need_Bit_مقدار_Comma = نادرست;
  DoClear = نادرست;
  بازگشت درست;
}

منطقی Output::bitSetMatch(StringRef Str, منطقی Matches) {
  اگر (Matches) {
    اگر (Need_Bit_مقدار_Comma)
      output(", ");
    output(Str);
    Need_Bit_مقدار_Comma = درست;
  }
  بازگشت نادرست;
}

تهی Output::endBitSetScalar() {
  output_Up_To_End_Of_سطر(" ]");
}

تهی Output::scalarString(StringRef &S, QuotingType MustQuote) {
  new_سطر_Check();
  اگر (S.empty()) {
    // Print '' for the empty string because leaving the field empty is not
    // allowed.
    output_Up_To_End_Of_سطر("''");
    بازگشت;
  }
  output(S, MustQuote);
  output_Up_To_End_Of_سطر("");
}

تهی Output::blockScalarString(StringRef &S) {
  اگر (!State_پشته.empty())
    new_سطر_Check();
  output(" |");

  unsigned Indent = State_پشته.empty() ? 1 : State_پشته.size();

  auto Buffer = MemoryBuffer::getMemBuffer(S, "", نادرست);
  برای (سطر_iterator سطرها(*Buffer, نادرست); !سطرها.is_at_end(); ++سطرها) {
    output_New_سطر();
    برای (unsigned I = 0; I < Indent; ++I) {
      output("  ");
    }
    output(*سطرها);
  }
  output_Up_To_End_Of_سطر("");
}

تهی Output::scalarTag(std::string &Tag) {
  اگر (Tag.empty())
    بازگشت;
  new_سطر_Check();
  output(Tag);
  output(" ");
}

تهی Output::setError(ثابت Twine &message) {
}

std::error_code Output::error() { بازگشت {}; }

منطقی Output::canElideEmptySequence() {
  // Normally, with an optional key/value where the value is an empty sequence,
  // the whole key/value can be not written.  But, that produces wrong yaml
  // if the key/value is the only thing in the map and the map is used in
  // a sequence.  This detects if the this sequence is the first key/value
  // in map that itself is embedded in a sequence.
  اگر (State_پشته.size() < 2)
    بازگشت درست;
  اگر (State_پشته.back() != in_Map_First_کلید)
    بازگشت درست;
  بازگشت !inSeqAnyElement(State_پشته[State_پشته.size() - 2]);
}

تهی Output::output(StringRef s) {
  Column += s.size();
  Out << s;
}

تهی Output::output(StringRef S, QuotingType MustQuote) {
  اگر (MustQuote == QuotingType::None) {
    // Only quote if we must.
    output(S);
    بازگشت;
  }

  StringLiteral Quote = MustQuote == QuotingType::Single ? StringLiteral("'")
                                                         : StringLiteral("\"");
  output(Quote); // Starting quote.

  // When using double-quoted strings (and only in that case), non-printable
  // characters may be present, and will be escaped using a variety of
  // unicode-scalar and special short-form escapes. This is handled in
  // yaml::escape.
  اگر (MustQuote == QuotingType::Double) {
    output(yaml::escape(S, /* EscapePrintable= */ نادرست));
    output(Quote);
    بازگشت;
  }

  unsigned i = 0;
  unsigned j = 0;
  unsigned End = S.size();
  ثابت نویسه *Base = S.data();

  // When using single-quoted strings, any single quote ' must be doubled to be
  // escaped.
  وقتی (j < End) {
    اگر (S[j] == '\'') {                   // Escape quotes.
      output(StringRef(&Base[i], j - i)); // "flush".
      output(StringLiteral("''"));        // Print it as ''
      i = j + 1;
    }
    ++j;
  }
  output(StringRef(&Base[i], j - i));
  output(Quote); // Ending quote.
}

تهی Output::output_Up_To_End_Of_سطر(StringRef s) {
  output(s);
  اگر (State_پشته.empty() || (!inFlowSeqAnyElement(State_پشته.back()) &&
                             !in_Flow_Map_Any_کلید(State_پشته.back())))
    Padding = "\n";
}

تهی Output::output_New_سطر() {
  Out << "\n";
  Column = 0;
}

// if seq at top, indent as if map, then add "- "
// if seq in middle, use "- " if firstKey, else use "  "
//

تهی Output::new_سطر_Check(منطقی EmptySequence) {
  اگر (Padding != "\n") {
    output(Padding);
    Padding = {};
    بازگشت;
  }
  output_New_سطر();
  Padding = {};

  اگر (State_پشته.size() == 0 || EmptySequence)
    بازگشت;

  unsigned Indent = State_پشته.size() - 1;
  منطقی PossiblyNestedSeq = نادرست;
  auto I = State_پشته.rbegin(), E = State_پشته.rend();

  اگر (inSeqAnyElement(*I)) {
    PossiblyNestedSeq = درست; // Not possibly but always.
    ++Indent;
  } وگرنه اگر (*I == in_Map_First_کلید || *I == in_Flow_Map_First_کلید ||
             inFlowSeqAnyElement(*I)) {
    PossiblyNestedSeq = درست;
    ++I; // Skip back().
  }

  unsigned Output_Dash_شمار = 0;
  اگر (PossiblyNestedSeq) {
    // Count up consecutive inSeqFirstElement from the end, unless
    // inSeqFirstElement is the top of nested sequence.
    وقتی (I != E) {
      // Don't count the top of nested sequence.
      اگر (!inSeqAnyElement(*I))
        بشکن;

      ++Output_Dash_شمار;

      // Stop counting if consecutive inSeqFirstElement ends.
      اگر (*I++ != inSeqFirstElement)
        بشکن;
    }
  }

  برای (unsigned I = Output_Dash_شمار; I < Indent; ++I)
    output("  ");

  برای (unsigned I = 0; I < Output_Dash_شمار; ++I)
    output("- ");
}

تهی Output::padded_کلید(StringRef کلید_2) {
  output(کلید_2, needsQuotes(کلید_2, نادرست));
  output(":");
  ثابت نویسه *spaces = "                ";
  اگر (کلید_2.size() < strlen(spaces))
    Padding = &spaces[کلید_2.size()];
  وگرنه
    Padding = " ";
}

تهی Output::flow_کلید(StringRef کلید) {
  اگر (State_پشته.back() == in_Flow_Map_Other_کلید)
    output(", ");
  اگر (WrapColumn && Column > WrapColumn) {
    output("\n");
    برای (صحیح I = 0; I < ColumnAtMapFlowStart; ++I)
      output(" ");
    Column = ColumnAtMapFlowStart;
    output("  ");
  }
  output(کلید, needsQuotes(کلید, نادرست));
  output(": ");
}

گره_Kind Output::get_گره_Kind() { report_fatal_error("invalid call"); }

منطقی Output::inSeqAnyElement(InState State) {
  بازگشت State == inSeqFirstElement || State == inSeqOtherElement;
}

منطقی Output::inFlowSeqAnyElement(InState State) {
  بازگشت State == inFlowSeqFirstElement || State == inFlowSeqOtherElement;
}

منطقی Output::in_Map_Any_کلید(InState State) {
  بازگشت State == in_Map_First_کلید || State == in_Map_Other_کلید;
}

منطقی Output::in_Flow_Map_Any_کلید(InState State) {
  بازگشت State == in_Flow_Map_First_کلید || State == in_Flow_Map_Other_کلید;
}

//===----------------------------------------------------------------------===//
//  traits for built-in types
//===----------------------------------------------------------------------===//

تهی ScalarTraits<منطقی>::output(ثابت منطقی &Val, تهی *, raw_ostream &Out) {
  Out << (Val ? "true" : "false");
}

StringRef ScalarTraits<منطقی>::درون_داد_2(StringRef Scalar, تهی *, منطقی &Val) {
  اگر (std::optional<منطقی> Parsed = parseBool(Scalar)) {
    Val = *Parsed;
    بازگشت StringRef();
  }
  بازگشت "invalid boolean";
}

تهی ScalarTraits<StringRef>::output(ثابت StringRef &Val, تهی *,
                                     raw_ostream &Out) {
  Out << Val;
}

StringRef ScalarTraits<StringRef>::درون_داد_2(StringRef Scalar, تهی *,
                                         StringRef &Val) {
  Val = Scalar;
  بازگشت StringRef();
}

تهی ScalarTraits<std::string>::output(ثابت std::string &Val, تهی *,
                                       raw_ostream &Out) {
  Out << Val;
}

StringRef ScalarTraits<std::string>::درون_داد_2(StringRef Scalar, تهی *,
                                           std::string &Val) {
  Val = Scalar.str();
  بازگشت StringRef();
}

تهی ScalarTraits<uint8_t>::output(ثابت uint8_t &Val, تهی *,
                                   raw_ostream &Out) {
  // use temp uin32_t because ostream thinks uint8_t is a character
  uint32_t رقم = Val;
  Out << رقم;
}

StringRef ScalarTraits<uint8_t>::درون_داد_2(StringRef Scalar, تهی *, uint8_t &Val) {
  unsigned long long n;
  اگر (getAsUnsignedInteger(Scalar, 0, n))
    بازگشت "invalid number";
  اگر (n > 0xFF)
    بازگشت "out of range number";
  Val = n;
  بازگشت StringRef();
}

تهی ScalarTraits<uint16_t>::output(ثابت uint16_t &Val, تهی *,
                                    raw_ostream &Out) {
  Out << Val;
}

StringRef ScalarTraits<uint16_t>::درون_داد_2(StringRef Scalar, تهی *,
                                        uint16_t &Val) {
  unsigned long long n;
  اگر (getAsUnsignedInteger(Scalar, 0, n))
    بازگشت "invalid number";
  اگر (n > 0xFFFF)
    بازگشت "out of range number";
  Val = n;
  بازگشت StringRef();
}

تهی ScalarTraits<uint32_t>::output(ثابت uint32_t &Val, تهی *,
                                    raw_ostream &Out) {
  Out << Val;
}

StringRef ScalarTraits<uint32_t>::درون_داد_2(StringRef Scalar, تهی *,
                                        uint32_t &Val) {
  unsigned long long n;
  اگر (getAsUnsignedInteger(Scalar, 0, n))
    بازگشت "invalid number";
  اگر (n > 0xFFFFFFFFUL)
    بازگشت "out of range number";
  Val = n;
  بازگشت StringRef();
}

تهی ScalarTraits<uint64_t>::output(ثابت uint64_t &Val, تهی *,
                                    raw_ostream &Out) {
  Out << Val;
}

StringRef ScalarTraits<uint64_t>::درون_داد_2(StringRef Scalar, تهی *,
                                        uint64_t &Val) {
  unsigned long long N;
  اگر (getAsUnsignedInteger(Scalar, 0, N))
    بازگشت "invalid number";
  Val = N;
  بازگشت StringRef();
}

تهی ScalarTraits<int8_t>::output(ثابت int8_t &Val, تهی *, raw_ostream &Out) {
  // use temp in32_t because ostream thinks int8_t is a character
  int32_t رقم = Val;
  Out << رقم;
}

StringRef ScalarTraits<int8_t>::درون_داد_2(StringRef Scalar, تهی *, int8_t &Val) {
  long long N;
  اگر (getAsSignedInteger(Scalar, 0, N))
    بازگشت "invalid number";
  اگر ((N > 127) || (N < -128))
    بازگشت "out of range number";
  Val = N;
  بازگشت StringRef();
}

تهی ScalarTraits<int16_t>::output(ثابت int16_t &Val, تهی *,
                                   raw_ostream &Out) {
  Out << Val;
}

StringRef ScalarTraits<int16_t>::درون_داد_2(StringRef Scalar, تهی *, int16_t &Val) {
  long long N;
  اگر (getAsSignedInteger(Scalar, 0, N))
    بازگشت "invalid number";
  اگر ((N > INT16_MAX) || (N < INT16_MIN))
    بازگشت "out of range number";
  Val = N;
  بازگشت StringRef();
}

تهی ScalarTraits<int32_t>::output(ثابت int32_t &Val, تهی *,
                                   raw_ostream &Out) {
  Out << Val;
}

StringRef ScalarTraits<int32_t>::درون_داد_2(StringRef Scalar, تهی *, int32_t &Val) {
  long long N;
  اگر (getAsSignedInteger(Scalar, 0, N))
    بازگشت "invalid number";
  اگر ((N > INT32_MAX) || (N < INT32_MIN))
    بازگشت "out of range number";
  Val = N;
  بازگشت StringRef();
}

تهی ScalarTraits<int64_t>::output(ثابت int64_t &Val, تهی *,
                                   raw_ostream &Out) {
  Out << Val;
}

StringRef ScalarTraits<int64_t>::درون_داد_2(StringRef Scalar, تهی *, int64_t &Val) {
  long long N;
  اگر (getAsSignedInteger(Scalar, 0, N))
    بازگشت "invalid number";
  Val = N;
  بازگشت StringRef();
}

تهی ScalarTraits<دوگانه>::output(ثابت دوگانه &Val, تهی *, raw_ostream &Out) {
  Out << format("%g", Val);
}

StringRef ScalarTraits<دوگانه>::درون_داد_2(StringRef Scalar, تهی *, دوگانه &Val) {
  اگر (to_float(Scalar, Val))
    بازگشت StringRef();
  بازگشت "invalid floating point number";
}

تهی ScalarTraits<اعشاری>::output(ثابت اعشاری &Val, تهی *, raw_ostream &Out) {
  Out << format("%g", Val);
}

StringRef ScalarTraits<اعشاری>::درون_داد_2(StringRef Scalar, تهی *, اعشاری &Val) {
  اگر (to_float(Scalar, Val))
    بازگشت StringRef();
  بازگشت "invalid floating point number";
}

تهی ScalarTraits<Hex8>::output(ثابت Hex8 &Val, تهی *, raw_ostream &Out) {
  Out << format("0x%" PRIX8, (uint8_t)Val);
}

StringRef ScalarTraits<Hex8>::درون_داد_2(StringRef Scalar, تهی *, Hex8 &Val) {
  unsigned long long n;
  اگر (getAsUnsignedInteger(Scalar, 0, n))
    بازگشت "invalid hex8 number";
  اگر (n > 0xFF)
    بازگشت "out of range hex8 number";
  Val = n;
  بازگشت StringRef();
}

تهی ScalarTraits<Hex16>::output(ثابت Hex16 &Val, تهی *, raw_ostream &Out) {
  Out << format("0x%" PRIX16, (uint16_t)Val);
}

StringRef ScalarTraits<Hex16>::درون_داد_2(StringRef Scalar, تهی *, Hex16 &Val) {
  unsigned long long n;
  اگر (getAsUnsignedInteger(Scalar, 0, n))
    بازگشت "invalid hex16 number";
  اگر (n > 0xFFFF)
    بازگشت "out of range hex16 number";
  Val = n;
  بازگشت StringRef();
}

تهی ScalarTraits<Hex32>::output(ثابت Hex32 &Val, تهی *, raw_ostream &Out) {
  Out << format("0x%" PRIX32, (uint32_t)Val);
}

StringRef ScalarTraits<Hex32>::درون_داد_2(StringRef Scalar, تهی *, Hex32 &Val) {
  unsigned long long n;
  اگر (getAsUnsignedInteger(Scalar, 0, n))
    بازگشت "invalid hex32 number";
  اگر (n > 0xFFFFFFFFUL)
    بازگشت "out of range hex32 number";
  Val = n;
  بازگشت StringRef();
}

تهی ScalarTraits<Hex64>::output(ثابت Hex64 &Val, تهی *, raw_ostream &Out) {
  Out << format("0x%" PRIX64, (uint64_t)Val);
}

StringRef ScalarTraits<Hex64>::درون_داد_2(StringRef Scalar, تهی *, Hex64 &Val) {
  unsigned long long رقم;
  اگر (getAsUnsignedInteger(Scalar, 0, رقم))
    بازگشت "invalid hex64 number";
  Val = رقم;
  بازگشت StringRef();
}

تهی ScalarTraits<VersionTuple>::output(ثابت VersionTuple &Val, تهی *,
                                        llvm::raw_ostream &Out) {
  Out << Val.getAsString();
}

StringRef ScalarTraits<VersionTuple>::درون_داد_2(StringRef Scalar, تهی *,
                                            VersionTuple &Val) {
  اگر (Val.tryParse(Scalar))
    بازگشت "invalid version format";
  بازگشت StringRef();
}
