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
  إرجاع Ctxt;
}

فراغ IO::setContext(فراغ *Context) {
  Ctxt = Context;
}

فراغ IO::set_Allow_Unknown_مفاتيح(منطقي Allow) {
  llvm_unreachable("Only supported for Input");
}

//===----------------------------------------------------------------------===//
//إدخال
//===----------------------------------------------------------------------===//

إدخال::إدخال(StringRef إدخال_Content, فراغ *Ctxt,
             SourceMgr::DiagHandlerTy DiagHandler, فراغ *DiagHandlerCtxt)
    : IO(Ctxt), Strm(جديد Stream(إدخال_Content, SrcMgr, خطأ, &EC)) {
  إذا (DiagHandler)
    SrcMgr.setDiagHandler(DiagHandler, DiagHandlerCtxt);
  DocIterator = Strm->begin();
}

إدخال::إدخال(MemoryBufferRef إدخال, فراغ *Ctxt,
             SourceMgr::DiagHandlerTy DiagHandler, فراغ *DiagHandlerCtxt)
    : IO(Ctxt), Strm(جديد Stream(إدخال, SrcMgr, خطأ, &EC)) {
  إذا (DiagHandler)
    SrcMgr.setDiagHandler(DiagHandler, DiagHandlerCtxt);
  DocIterator = Strm->begin();
}

إدخال::~إدخال() = افتراضي;

std::error_code إدخال::error() { إرجاع EC; }

منطقي إدخال::outputting() ثابت {
  إرجاع خطأ;
}

منطقي إدخال::setCurrentDocument() {
  إذا (DocIterator != Strm->end()) {
    عقدة *N = DocIterator->get_جذر();
    إذا (!N) {
      EC = make_error_code(errc::invalid_argument);
      إرجاع خطأ;
    }

    إذا (isa<Null_عقدة>(N)) {
      // Empty files are allowed and ignored
      ++DocIterator;
      إرجاع setCurrentDocument();
    }
    releaseHNodeBuffers();
    قمة_عقدة = createHNodes(N);
    Current_عقدة = قمة_عقدة;
    إرجاع true;
  }
  إرجاع خطأ;
}

منطقي إدخال::nextDocument() {
  إرجاع ++DocIterator != Strm->end();
}

ثابت عقدة *إدخال::get_Current_عقدة() ثابت {
  إرجاع Current_عقدة ? Current_عقدة->عقدة_2 : nullptr;
}

منطقي إدخال::mapTag(StringRef Tag, منطقي Default) {
  // CurrentNode can be null if setCurrentDocument() was unable to
  // parse the document because it was invalid or empty.
  إذا (!Current_عقدة)
    إرجاع خطأ;

  std::string foundTag = Current_عقدة->عقدة_2->getVerbatimTag();
  إذا (foundTag.empty()) {
    // If no tag found and 'Tag' is the default, say it was found.
    إرجاع Default;
  }
  // Return true iff found tag matches supplied tag.
  إرجاع Tag == foundTag;
}

فراغ إدخال::beginMapping() {
  إذا (EC)
    إرجاع;
  // CurrentNode can be null if the document is empty.
  MapHNode *MN = dyn_cast_or_null<MapHNode>(Current_عقدة);
  إذا (MN) {
    MN->Valid_مفاتيح.clear();
  }
}

std::vector<StringRef> إدخال::مفاتيح() {
  MapHNode *MN = dyn_cast<MapHNode>(Current_عقدة);
  std::vector<StringRef> Ret;
  إذا (!MN) {
    setError(Current_عقدة, "not a mapping");
    إرجاع Ret;
  }
  لكل (auto &P : MN->Mapping)
    Ret.ادفع_back(P.first());
  إرجاع Ret;
}

منطقي إدخال::preflight_مفتاح(StringRef مفتاح, منطقي Required, منطقي, منطقي &UseDefault,
                         فراغ *&SaveInfo) {
  UseDefault = خطأ;
  إذا (EC)
    إرجاع خطأ;

  // CurrentNode is null for empty documents, which is an error in case required
  // nodes are present.
  إذا (!Current_عقدة) {
    إذا (Required)
      EC = make_error_code(errc::invalid_argument);
    وإلا
      UseDefault = true;
    إرجاع خطأ;
  }

  MapHNode *MN = dyn_cast<MapHNode>(Current_عقدة);
  إذا (!MN) {
    إذا (Required || !isa<EmptyHNode>(Current_عقدة))
      setError(Current_عقدة, "not a mapping");
    وإلا
      UseDefault = true;
    إرجاع خطأ;
  }
  MN->Valid_مفاتيح.ادفع_back(مفتاح.str());
  HNode *قيمة = MN->Mapping[مفتاح].first;
  إذا (!قيمة) {
    إذا (Required)
      setError(Current_عقدة, Twine("missing required key '") + مفتاح + "'");
    وإلا
      UseDefault = true;
    إرجاع خطأ;
  }
  SaveInfo = Current_عقدة;
  Current_عقدة = قيمة;
  إرجاع true;
}

فراغ إدخال::postflight_مفتاح(فراغ *saveInfo) {
  Current_عقدة = reinterpret_cast<HNode *>(saveInfo);
}

فراغ إدخال::endMapping() {
  إذا (EC)
    إرجاع;
  // CurrentNode can be null if the document is empty.
  MapHNode *MN = dyn_cast_or_null<MapHNode>(Current_عقدة);
  إذا (!MN)
    إرجاع;
  لكل (ثابت auto &NN : MN->Mapping) {
    إذا (!is_contained(MN->Valid_مفاتيح, NN.first())) {
      ثابت SMRange &ReportLoc = NN.second.second;
      إذا (!Allow_Unknown_مفاتيح) {
        setError(ReportLoc, Twine("unknown key '") + NN.first() + "'");
        اكسر;
      } وإلا
        reportWarning(ReportLoc, Twine("unknown key '") + NN.first() + "'");
    }
  }
}

فراغ إدخال::beginFlowMapping() { beginMapping(); }

فراغ إدخال::endFlowMapping() { endMapping(); }

unsigned إدخال::beginSequence() {
  إذا (SequenceHNode *SQ = dyn_cast<SequenceHNode>(Current_عقدة))
    إرجاع SQ->Entries.size();
  إذا (isa<EmptyHNode>(Current_عقدة))
    إرجاع 0;
  // Treat case where there's a scalar "null" value as an empty sequence.
  إذا (ScalarHNode *SN = dyn_cast<ScalarHNode>(Current_عقدة)) {
    إذا (isNull(SN->قيمة_2()))
      إرجاع 0;
  }
  // Any other type of HNode is an error.
  setError(Current_عقدة, "not a sequence");
  إرجاع 0;
}

فراغ إدخال::endSequence() {
}

منطقي إدخال::preflightElement(unsigned Index, فراغ *&SaveInfo) {
  إذا (EC)
    إرجاع خطأ;
  إذا (SequenceHNode *SQ = dyn_cast<SequenceHNode>(Current_عقدة)) {
    SaveInfo = Current_عقدة;
    Current_عقدة = SQ->Entries[Index];
    إرجاع true;
  }
  إرجاع خطأ;
}

فراغ إدخال::postflightElement(فراغ *SaveInfo) {
  Current_عقدة = reinterpret_cast<HNode *>(SaveInfo);
}

unsigned إدخال::beginFlowSequence() { إرجاع beginSequence(); }

منطقي إدخال::preflightFlowElement(unsigned index, فراغ *&SaveInfo) {
  إذا (EC)
    إرجاع خطأ;
  إذا (SequenceHNode *SQ = dyn_cast<SequenceHNode>(Current_عقدة)) {
    SaveInfo = Current_عقدة;
    Current_عقدة = SQ->Entries[index];
    إرجاع true;
  }
  إرجاع خطأ;
}

فراغ إدخال::postflightFlowElement(فراغ *SaveInfo) {
  Current_عقدة = reinterpret_cast<HNode *>(SaveInfo);
}

فراغ إدخال::endFlowSequence() {
}

فراغ إدخال::beginEnumScalar() {
  ScalarMatchFound = خطأ;
}

منطقي إدخال::matchEnumScalar(StringRef Str, منطقي) {
  إذا (ScalarMatchFound)
    إرجاع خطأ;
  إذا (ScalarHNode *SN = dyn_cast<ScalarHNode>(Current_عقدة)) {
    إذا (SN->قيمة_2() == Str) {
      ScalarMatchFound = true;
      إرجاع true;
    }
  }
  إرجاع خطأ;
}

منطقي إدخال::matchEnumFallback() {
  إذا (ScalarMatchFound)
    إرجاع خطأ;
  ScalarMatchFound = true;
  إرجاع true;
}

فراغ إدخال::endEnumScalar() {
  إذا (!ScalarMatchFound) {
    setError(Current_عقدة, "unknown enumerated scalar");
  }
}

منطقي إدخال::beginBitSetScalar(منطقي &DoClear) {
  Bit_قيم_Used.clear();
  إذا (SequenceHNode *SQ = dyn_cast<SequenceHNode>(Current_عقدة)) {
    Bit_قيم_Used.resize(SQ->Entries.size());
  } وإلا {
    setError(Current_عقدة, "expected sequence of bit values");
  }
  DoClear = true;
  إرجاع true;
}

منطقي إدخال::bitSetMatch(StringRef Str, منطقي) {
  إذا (EC)
    إرجاع خطأ;
  إذا (SequenceHNode *SQ = dyn_cast<SequenceHNode>(Current_عقدة)) {
    unsigned Index = 0;
    لكل (auto &N : SQ->Entries) {
      إذا (ScalarHNode *SN = dyn_cast<ScalarHNode>(N)) {
        إذا (SN->قيمة_2() == Str) {
          Bit_قيم_Used[Index] = true;
          إرجاع true;
        }
      } وإلا {
        setError(Current_عقدة, "unexpected scalar in sequence of bit values");
      }
      ++Index;
    }
  } وإلا {
    setError(Current_عقدة, "expected sequence of bit values");
  }
  إرجاع خطأ;
}

فراغ إدخال::endBitSetScalar() {
  إذا (EC)
    إرجاع;
  إذا (SequenceHNode *SQ = dyn_cast<SequenceHNode>(Current_عقدة)) {
    أكد(Bit_قيم_Used.size() == SQ->Entries.size());
    لكل (unsigned i = 0; i < SQ->Entries.size(); ++i) {
      إذا (!Bit_قيم_Used[i]) {
        setError(SQ->Entries[i], "unknown bit value");
        إرجاع;
      }
    }
  }
}

فراغ إدخال::scalarString(StringRef &S, QuotingType) {
  إذا (ScalarHNode *SN = dyn_cast<ScalarHNode>(Current_عقدة)) {
    S = SN->قيمة_2();
  } وإلا {
    setError(Current_عقدة, "unexpected scalar");
  }
}

فراغ إدخال::blockScalarString(StringRef &S) { scalarString(S, QuotingType::None); }

فراغ إدخال::scalarTag(std::string &Tag) {
  Tag = Current_عقدة->عقدة_2->getVerbatimTag();
}

فراغ إدخال::setError(HNode *hnode, ثابت Twine &message) {
  أكد(hnode && "HNode must not be NULL");
  setError(hnode->عقدة_2, message);
}

عقدة_Kind إدخال::get_عقدة_Kind() {
  إذا (isa<ScalarHNode>(Current_عقدة))
    إرجاع عقدة_Kind::Scalar;
  وإلا إذا (isa<MapHNode>(Current_عقدة))
    إرجاع عقدة_Kind::Map;
  وإلا إذا (isa<SequenceHNode>(Current_عقدة))
    إرجاع عقدة_Kind::Sequence;
  llvm_unreachable("Unsupported node kind");
}

فراغ إدخال::setError(عقدة *عقدة_3, ثابت Twine &message) {
  Strm->printError(عقدة_3, message);
  EC = make_error_code(errc::invalid_argument);
}

فراغ إدخال::setError(ثابت SMRange &range, ثابت Twine &message) {
  Strm->printError(range, message);
  EC = make_error_code(errc::invalid_argument);
}

فراغ إدخال::reportWarning(HNode *hnode, ثابت Twine &message) {
  أكد(hnode && "HNode must not be NULL");
  Strm->printError(hnode->عقدة_2, message, SourceMgr::DK_Warning);
}

فراغ إدخال::reportWarning(عقدة *عقدة_3, ثابت Twine &message) {
  Strm->printError(عقدة_3, message, SourceMgr::DK_Warning);
}

فراغ إدخال::reportWarning(ثابت SMRange &range, ثابت Twine &message) {
  Strm->printError(range, message, SourceMgr::DK_Warning);
}

فراغ إدخال::releaseHNodeBuffers() {
  EmptyHNodeAllocator.DestroyAll();
  ScalarHNodeAllocator.DestroyAll();
  SequenceHNodeAllocator.DestroyAll();
  MapHNodeAllocator.DestroyAll();
}

فراغ إدخال::saveAliasHNode(عقدة *N, HNode *HN) {
  StringRef Anchor = N->getAnchor();
  إذا (!Anchor.empty())
    // YAML 1.2.2 - 3.2.2.2. Anchors and Aliases:
    //
    // An alias event refers to the most recent event in the serialization
    // having the specified anchor. Therefore, anchors need not be unique within
    // a serialization. In addition, an anchor need not have an alias node
    // referring to it.
    AliasMap[Anchor] = HN;
}

إدخال::HNode *إدخال::createHNodes(عقدة *N) {
  SmallString<128> StringStorage;
  بدل (N->getType()) {
  حالة عقدة::NK_Scalar: {
    Scalar_عقدة *SN = dyn_cast<Scalar_عقدة>(N);
    StringRef مفتاح_Str = SN->get_قيمة(StringStorage);
    إذا (!StringStorage.empty()) {
      // Copy string to permanent storage
      مفتاح_Str = StringStorage.str().copy(StringAllocator);
    }
    auto *SHNode = جديد (ScalarHNodeAllocator.Allocate()) ScalarHNode(N, مفتاح_Str);
    saveAliasHNode(SN, SHNode);
    إرجاع SHNode;
  }
  حالة عقدة::NK_BlockScalar: {
    Block_Scalar_عقدة *BSN = dyn_cast<Block_Scalar_عقدة>(N);
    StringRef قيمة_Copy = BSN->get_قيمة().copy(StringAllocator);
    auto *BSHNode =
        جديد (ScalarHNodeAllocator.Allocate()) ScalarHNode(N, قيمة_Copy);
    saveAliasHNode(BSN, BSHNode);
    إرجاع BSHNode;
  }
  حالة عقدة::NK_Sequence: {
    Sequence_عقدة *SQ = dyn_cast<Sequence_عقدة>(N);
    auto SQHNode = جديد (SequenceHNodeAllocator.Allocate()) SequenceHNode(N);
    لكل (عقدة &SN : *SQ) {
      auto Entry = createHNodes(&SN);
      إذا (EC)
        اكسر;
      SQHNode->Entries.ادفع_back(Entry);
    }
    saveAliasHNode(SQ, SQHNode);
    إرجاع SQHNode;
  }
  حالة عقدة::NK_Mapping: {
    Mapping_عقدة *Map = dyn_cast<Mapping_عقدة>(N);
    auto mapHNode = جديد (MapHNodeAllocator.Allocate()) MapHNode(N);
    لكل (مفتاح_قيمة_عقدة &KVN : *Map) {
      عقدة *مفتاح_عقدة = KVN.get_مفتاح();
      Scalar_عقدة *مفتاح = dyn_cast_if_present<Scalar_عقدة>(مفتاح_عقدة);
      عقدة *قيمة = KVN.get_قيمة();
      إذا (!مفتاح || !قيمة) {
        إذا (!مفتاح)
          setError(مفتاح_عقدة, "Map key must be a scalar");
        إذا (!قيمة)
          setError(مفتاح_عقدة, "Map value must not be empty");
        اكسر;
      }
      StringStorage.clear();
      StringRef مفتاح_Str = مفتاح->get_قيمة(StringStorage);
      إذا (!StringStorage.empty()) {
        // Copy string to permanent storage
        مفتاح_Str = StringStorage.str().copy(StringAllocator);
      }
      إذا (mapHNode->Mapping.تعداد_2(مفتاح_Str))
        // From YAML spec: "The content of a mapping node is an unordered set of
        // key/value node pairs, with the restriction that each of the keys is
        // unique."
        setError(مفتاح_عقدة, Twine("duplicated mapping key '") + مفتاح_Str + "'");
      auto قيمة_HNode = createHNodes(قيمة);
      إذا (EC)
        اكسر;
      mapHNode->Mapping[مفتاح_Str] =
          std::make_pair(std::move(قيمة_HNode), مفتاح_عقدة->getSourceRange());
    }
    saveAliasHNode(Map, mapHNode);
    إرجاع std::move(mapHNode);
  }
  حالة عقدة::NK_Null:
    // TODO: Anchor is not set for NullNode in the parser. Update the parser to
    // faithfully preserve anchors.
    إرجاع جديد (EmptyHNodeAllocator.Allocate()) EmptyHNode(N);
  حالة عقدة::NK_Alias: {
    Alias_عقدة *AN = dyn_cast<Alias_عقدة>(N);
    auto AliasName = AN->getName();
    auto AHN = AliasMap.find(AliasName);
    إذا (AHN == AliasMap.end()) {
      setError(AN, Twine("undefined alias '" + AliasName + "'"));
      إرجاع nullptr;
    }
    إرجاع AHN->second;
  }
  افتراضي:
    setError(N, "unknown node kind");
    إرجاع nullptr;
  }
}

فراغ إدخال::setError(ثابت Twine &Message) {
  setError(Current_عقدة, Message);
}

فراغ إدخال::set_Allow_Unknown_مفاتيح(منطقي Allow) { Allow_Unknown_مفاتيح = Allow; }

منطقي إدخال::canElideEmptySequence() {
  إرجاع خطأ;
}

//===----------------------------------------------------------------------===//
//  Output
//===----------------------------------------------------------------------===//

Output::Output(raw_ostream &yout, فراغ *context, صحيح WrapColumn)
    : IO(context), Out(yout), WrapColumn(WrapColumn) {}

Output::~Output() = افتراضي;

منطقي Output::outputting() ثابت {
  إرجاع true;
}

فراغ Output::beginMapping() {
  State_مكدس.ادفع_back(in_Map_First_مفتاح);
  PaddingBeforeContainer = Padding;
  Padding = "\n";
}

منطقي Output::mapTag(StringRef Tag, منطقي Use) {
  إذا (Use) {
    // If this tag is being written inside a sequence we should write the start
    // of the sequence before writing the tag, otherwise the tag won't be
    // attached to the element in the sequence, but rather the sequence itself.
    منطقي SequenceElement = خطأ;
    إذا (State_مكدس.size() > 1) {
      auto &E = State_مكدس[State_مكدس.size() - 2];
      SequenceElement = inSeqAnyElement(E) || inFlowSeqAnyElement(E);
    }
    إذا (SequenceElement && State_مكدس.back() == in_Map_First_مفتاح) {
      new_سطر_Check();
    } وإلا {
      output(" ");
    }
    output(Tag);
    إذا (SequenceElement) {
      // If we're writing the tag during the first element of a map, the tag
      // takes the place of the first element in the sequence.
      إذا (State_مكدس.back() == in_Map_First_مفتاح) {
        State_مكدس.اسحب_back();
        State_مكدس.ادفع_back(in_Map_Other_مفتاح);
      }
      // Tags inside maps in sequences should act as keys in the map from a
      // formatting perspective, so we always want a newline in a sequence.
      Padding = "\n";
    }
  }
  إرجاع Use;
}

فراغ Output::endMapping() {
  // If we did not map anything, we should explicitly emit an empty map
  إذا (State_مكدس.back() == in_Map_First_مفتاح) {
    Padding = PaddingBeforeContainer;
    new_سطر_Check();
    output("{}");
    Padding = "\n";
  }
  State_مكدس.اسحب_back();
}

std::vector<StringRef> Output::مفاتيح() {
  report_fatal_error("invalid call");
}

منطقي Output::preflight_مفتاح(StringRef مفتاح, منطقي Required, منطقي SameAsDefault,
                          منطقي &UseDefault, فراغ *&SaveInfo) {
  UseDefault = خطأ;
  SaveInfo = nullptr;
  إذا (Required || !SameAsDefault || Write_Default_قيم) {
    auto State = State_مكدس.back();
    إذا (State == in_Flow_Map_First_مفتاح || State == in_Flow_Map_Other_مفتاح) {
      flow_مفتاح(مفتاح);
    } وإلا {
      new_سطر_Check();
      padded_مفتاح(مفتاح);
    }
    إرجاع true;
  }
  إرجاع خطأ;
}

فراغ Output::postflight_مفتاح(فراغ *) {
  إذا (State_مكدس.back() == in_Map_First_مفتاح) {
    State_مكدس.اسحب_back();
    State_مكدس.ادفع_back(in_Map_Other_مفتاح);
  } وإلا إذا (State_مكدس.back() == in_Flow_Map_First_مفتاح) {
    State_مكدس.اسحب_back();
    State_مكدس.ادفع_back(in_Flow_Map_Other_مفتاح);
  }
}

فراغ Output::beginFlowMapping() {
  State_مكدس.ادفع_back(in_Flow_Map_First_مفتاح);
  new_سطر_Check();
  ColumnAtMapFlowStart = Column;
  output("{ ");
}

فراغ Output::endFlowMapping() {
  State_مكدس.اسحب_back();
  output_Up_To_End_Of_سطر(" }");
}

فراغ Output::beginDocuments() {
  output_Up_To_End_Of_سطر("---");
}

منطقي Output::preflightDocument(unsigned index) {
  إذا (index > 0)
    output_Up_To_End_Of_سطر("\n---");
  إرجاع true;
}

فراغ Output::postflightDocument() {
}

فراغ Output::endDocuments() {
  output("\n...\n");
}

unsigned Output::beginSequence() {
  State_مكدس.ادفع_back(inSeqFirstElement);
  PaddingBeforeContainer = Padding;
  Padding = "\n";
  إرجاع 0;
}

فراغ Output::endSequence() {
  // If we did not emit anything, we should explicitly emit an empty sequence
  إذا (State_مكدس.back() == inSeqFirstElement) {
    Padding = PaddingBeforeContainer;
    new_سطر_Check(/*EmptySequence=*/true);
    output("[]");
    Padding = "\n";
  }
  State_مكدس.اسحب_back();
}

منطقي Output::preflightElement(unsigned, فراغ *&SaveInfo) {
  SaveInfo = nullptr;
  إرجاع true;
}

فراغ Output::postflightElement(فراغ *) {
  إذا (State_مكدس.back() == inSeqFirstElement) {
    State_مكدس.اسحب_back();
    State_مكدس.ادفع_back(inSeqOtherElement);
  } وإلا إذا (State_مكدس.back() == inFlowSeqFirstElement) {
    State_مكدس.اسحب_back();
    State_مكدس.ادفع_back(inFlowSeqOtherElement);
  }
}

unsigned Output::beginFlowSequence() {
  State_مكدس.ادفع_back(inFlowSeqFirstElement);
  new_سطر_Check();
  ColumnAtFlowStart = Column;
  output("[ ");
  NeedFlowSequenceComma = خطأ;
  إرجاع 0;
}

فراغ Output::endFlowSequence() {
  State_مكدس.اسحب_back();
  output_Up_To_End_Of_سطر(" ]");
}

منطقي Output::preflightFlowElement(unsigned, فراغ *&SaveInfo) {
  إذا (NeedFlowSequenceComma)
    output(", ");
  إذا (WrapColumn && Column > WrapColumn) {
    output("\n");
    لكل (صحيح i = 0; i < ColumnAtFlowStart; ++i)
      output(" ");
    Column = ColumnAtFlowStart;
    output("  ");
  }
  SaveInfo = nullptr;
  إرجاع true;
}

فراغ Output::postflightFlowElement(فراغ *) {
  NeedFlowSequenceComma = true;
}

فراغ Output::beginEnumScalar() {
  EnumerationMatchFound = خطأ;
}

منطقي Output::matchEnumScalar(StringRef Str, منطقي Match) {
  إذا (Match && !EnumerationMatchFound) {
    new_سطر_Check();
    output_Up_To_End_Of_سطر(Str);
    EnumerationMatchFound = true;
  }
  إرجاع خطأ;
}

منطقي Output::matchEnumFallback() {
  إذا (EnumerationMatchFound)
    إرجاع خطأ;
  EnumerationMatchFound = true;
  إرجاع true;
}

فراغ Output::endEnumScalar() {
  إذا (!EnumerationMatchFound)
    llvm_unreachable("bad runtime enum value");
}

منطقي Output::beginBitSetScalar(منطقي &DoClear) {
  new_سطر_Check();
  output("[ ");
  Need_Bit_قيمة_Comma = خطأ;
  DoClear = خطأ;
  إرجاع true;
}

منطقي Output::bitSetMatch(StringRef Str, منطقي Matches) {
  إذا (Matches) {
    إذا (Need_Bit_قيمة_Comma)
      output(", ");
    output(Str);
    Need_Bit_قيمة_Comma = true;
  }
  إرجاع خطأ;
}

فراغ Output::endBitSetScalar() {
  output_Up_To_End_Of_سطر(" ]");
}

فراغ Output::scalarString(StringRef &S, QuotingType MustQuote) {
  new_سطر_Check();
  إذا (S.empty()) {
    // Print '' for the empty string because leaving the field empty is not
    // allowed.
    output_Up_To_End_Of_سطر("''");
    إرجاع;
  }
  output(S, MustQuote);
  output_Up_To_End_Of_سطر("");
}

فراغ Output::blockScalarString(StringRef &S) {
  إذا (!State_مكدس.empty())
    new_سطر_Check();
  output(" |");

  unsigned Indent = State_مكدس.empty() ? 1 : State_مكدس.size();

  auto Buffer = MemoryBuffer::getMemBuffer(S, "", خطأ);
  لكل (سطر_iterator أسطر(*Buffer, خطأ); !أسطر.is_at_end(); ++أسطر) {
    output_New_سطر();
    لكل (unsigned I = 0; I < Indent; ++I) {
      output("  ");
    }
    output(*أسطر);
  }
  output_Up_To_End_Of_سطر("");
}

فراغ Output::scalarTag(std::string &Tag) {
  إذا (Tag.empty())
    إرجاع;
  new_سطر_Check();
  output(Tag);
  output(" ");
}

فراغ Output::setError(ثابت Twine &message) {
}

std::error_code Output::error() { إرجاع {}; }

منطقي Output::canElideEmptySequence() {
  // Normally, with an optional key/value where the value is an empty sequence,
  // the whole key/value can be not written.  But, that produces wrong yaml
  // if the key/value is the only thing in the map and the map is used in
  // a sequence.  This detects if the this sequence is the first key/value
  // in map that itself is embedded in a sequence.
  إذا (State_مكدس.size() < 2)
    إرجاع true;
  إذا (State_مكدس.back() != in_Map_First_مفتاح)
    إرجاع true;
  إرجاع !inSeqAnyElement(State_مكدس[State_مكدس.size() - 2]);
}

فراغ Output::output(StringRef s) {
  Column += s.size();
  Out << s;
}

فراغ Output::output(StringRef S, QuotingType MustQuote) {
  إذا (MustQuote == QuotingType::None) {
    // Only quote if we must.
    output(S);
    إرجاع;
  }

  StringLiteral Quote = MustQuote == QuotingType::Single ? StringLiteral("'")
                                                         : StringLiteral("\"");
  output(Quote); // Starting quote.

  // When using double-quoted strings (and only in that case), non-printable
  // characters may be present, and will be escaped using a variety of
  // unicode-scalar and special short-form escapes. This is handled in
  // yaml::escape.
  إذا (MustQuote == QuotingType::Double) {
    output(yaml::escape(S, /* EscapePrintable= */ خطأ));
    output(Quote);
    إرجاع;
  }

  unsigned i = 0;
  unsigned j = 0;
  unsigned End = S.size();
  ثابت حرف *Base = S.data();

  // When using single-quoted strings, any single quote ' must be doubled to be
  // escaped.
  بينما (j < End) {
    إذا (S[j] == '\'') {                   // Escape quotes.
      output(StringRef(&Base[i], j - i)); // "flush".
      output(StringLiteral("''"));        // Print it as ''
      i = j + 1;
    }
    ++j;
  }
  output(StringRef(&Base[i], j - i));
  output(Quote); // Ending quote.
}

فراغ Output::output_Up_To_End_Of_سطر(StringRef s) {
  output(s);
  إذا (State_مكدس.empty() || (!inFlowSeqAnyElement(State_مكدس.back()) &&
                             !in_Flow_Map_Any_مفتاح(State_مكدس.back())))
    Padding = "\n";
}

فراغ Output::output_New_سطر() {
  Out << "\n";
  Column = 0;
}

// if seq at top, indent as if map, then add "- "
// if seq in middle, use "- " if firstKey, else use "  "
//

فراغ Output::new_سطر_Check(منطقي EmptySequence) {
  إذا (Padding != "\n") {
    output(Padding);
    Padding = {};
    إرجاع;
  }
  output_New_سطر();
  Padding = {};

  إذا (State_مكدس.size() == 0 || EmptySequence)
    إرجاع;

  unsigned Indent = State_مكدس.size() - 1;
  منطقي PossiblyNestedSeq = خطأ;
  auto I = State_مكدس.rbegin(), E = State_مكدس.rend();

  إذا (inSeqAnyElement(*I)) {
    PossiblyNestedSeq = true; // Not possibly but always.
    ++Indent;
  } وإلا إذا (*I == in_Map_First_مفتاح || *I == in_Flow_Map_First_مفتاح ||
             inFlowSeqAnyElement(*I)) {
    PossiblyNestedSeq = true;
    ++I; // Skip back().
  }

  unsigned Output_Dash_تعداد = 0;
  إذا (PossiblyNestedSeq) {
    // Count up consecutive inSeqFirstElement from the end, unless
    // inSeqFirstElement is the top of nested sequence.
    بينما (I != E) {
      // Don't count the top of nested sequence.
      إذا (!inSeqAnyElement(*I))
        اكسر;

      ++Output_Dash_تعداد;

      // Stop counting if consecutive inSeqFirstElement ends.
      إذا (*I++ != inSeqFirstElement)
        اكسر;
    }
  }

  لكل (unsigned I = Output_Dash_تعداد; I < Indent; ++I)
    output("  ");

  لكل (unsigned I = 0; I < Output_Dash_تعداد; ++I)
    output("- ");
}

فراغ Output::padded_مفتاح(StringRef مفتاح_2) {
  output(مفتاح_2, needsQuotes(مفتاح_2, خطأ));
  output(":");
  ثابت حرف *spaces = "                ";
  إذا (مفتاح_2.size() < strlen(spaces))
    Padding = &spaces[مفتاح_2.size()];
  وإلا
    Padding = " ";
}

فراغ Output::flow_مفتاح(StringRef مفتاح) {
  إذا (State_مكدس.back() == in_Flow_Map_Other_مفتاح)
    output(", ");
  إذا (WrapColumn && Column > WrapColumn) {
    output("\n");
    لكل (صحيح I = 0; I < ColumnAtMapFlowStart; ++I)
      output(" ");
    Column = ColumnAtMapFlowStart;
    output("  ");
  }
  output(مفتاح, needsQuotes(مفتاح, خطأ));
  output(": ");
}

عقدة_Kind Output::get_عقدة_Kind() { report_fatal_error("invalid call"); }

منطقي Output::inSeqAnyElement(InState State) {
  إرجاع State == inSeqFirstElement || State == inSeqOtherElement;
}

منطقي Output::inFlowSeqAnyElement(InState State) {
  إرجاع State == inFlowSeqFirstElement || State == inFlowSeqOtherElement;
}

منطقي Output::in_Map_Any_مفتاح(InState State) {
  إرجاع State == in_Map_First_مفتاح || State == in_Map_Other_مفتاح;
}

منطقي Output::in_Flow_Map_Any_مفتاح(InState State) {
  إرجاع State == in_Flow_Map_First_مفتاح || State == in_Flow_Map_Other_مفتاح;
}

//===----------------------------------------------------------------------===//
//  traits for built-in types
//===----------------------------------------------------------------------===//

فراغ ScalarTraits<منطقي>::output(ثابت منطقي &Val, فراغ *, raw_ostream &Out) {
  Out << (Val ? "true" : "false");
}

StringRef ScalarTraits<منطقي>::إدخال_2(StringRef Scalar, فراغ *, منطقي &Val) {
  إذا (std::optional<منطقي> Parsed = parseBool(Scalar)) {
    Val = *Parsed;
    إرجاع StringRef();
  }
  إرجاع "invalid boolean";
}

فراغ ScalarTraits<StringRef>::output(ثابت StringRef &Val, فراغ *,
                                     raw_ostream &Out) {
  Out << Val;
}

StringRef ScalarTraits<StringRef>::إدخال_2(StringRef Scalar, فراغ *,
                                         StringRef &Val) {
  Val = Scalar;
  إرجاع StringRef();
}

فراغ ScalarTraits<std::string>::output(ثابت std::string &Val, فراغ *,
                                       raw_ostream &Out) {
  Out << Val;
}

StringRef ScalarTraits<std::string>::إدخال_2(StringRef Scalar, فراغ *,
                                           std::string &Val) {
  Val = Scalar.str();
  إرجاع StringRef();
}

فراغ ScalarTraits<uint8_t>::output(ثابت uint8_t &Val, فراغ *,
                                   raw_ostream &Out) {
  // use temp uin32_t because ostream thinks uint8_t is a character
  uint32_t رقم = Val;
  Out << رقم;
}

StringRef ScalarTraits<uint8_t>::إدخال_2(StringRef Scalar, فراغ *, uint8_t &Val) {
  unsigned long long n;
  إذا (getAsUnsignedInteger(Scalar, 0, n))
    إرجاع "invalid number";
  إذا (n > 0xFF)
    إرجاع "out of range number";
  Val = n;
  إرجاع StringRef();
}

فراغ ScalarTraits<uint16_t>::output(ثابت uint16_t &Val, فراغ *,
                                    raw_ostream &Out) {
  Out << Val;
}

StringRef ScalarTraits<uint16_t>::إدخال_2(StringRef Scalar, فراغ *,
                                        uint16_t &Val) {
  unsigned long long n;
  إذا (getAsUnsignedInteger(Scalar, 0, n))
    إرجاع "invalid number";
  إذا (n > 0xFFFF)
    إرجاع "out of range number";
  Val = n;
  إرجاع StringRef();
}

فراغ ScalarTraits<uint32_t>::output(ثابت uint32_t &Val, فراغ *,
                                    raw_ostream &Out) {
  Out << Val;
}

StringRef ScalarTraits<uint32_t>::إدخال_2(StringRef Scalar, فراغ *,
                                        uint32_t &Val) {
  unsigned long long n;
  إذا (getAsUnsignedInteger(Scalar, 0, n))
    إرجاع "invalid number";
  إذا (n > 0xFFFFFFFFUL)
    إرجاع "out of range number";
  Val = n;
  إرجاع StringRef();
}

فراغ ScalarTraits<uint64_t>::output(ثابت uint64_t &Val, فراغ *,
                                    raw_ostream &Out) {
  Out << Val;
}

StringRef ScalarTraits<uint64_t>::إدخال_2(StringRef Scalar, فراغ *,
                                        uint64_t &Val) {
  unsigned long long N;
  إذا (getAsUnsignedInteger(Scalar, 0, N))
    إرجاع "invalid number";
  Val = N;
  إرجاع StringRef();
}

فراغ ScalarTraits<int8_t>::output(ثابت int8_t &Val, فراغ *, raw_ostream &Out) {
  // use temp in32_t because ostream thinks int8_t is a character
  int32_t رقم = Val;
  Out << رقم;
}

StringRef ScalarTraits<int8_t>::إدخال_2(StringRef Scalar, فراغ *, int8_t &Val) {
  long long N;
  إذا (getAsSignedInteger(Scalar, 0, N))
    إرجاع "invalid number";
  إذا ((N > 127) || (N < -128))
    إرجاع "out of range number";
  Val = N;
  إرجاع StringRef();
}

فراغ ScalarTraits<int16_t>::output(ثابت int16_t &Val, فراغ *,
                                   raw_ostream &Out) {
  Out << Val;
}

StringRef ScalarTraits<int16_t>::إدخال_2(StringRef Scalar, فراغ *, int16_t &Val) {
  long long N;
  إذا (getAsSignedInteger(Scalar, 0, N))
    إرجاع "invalid number";
  إذا ((N > INT16_MAX) || (N < INT16_MIN))
    إرجاع "out of range number";
  Val = N;
  إرجاع StringRef();
}

فراغ ScalarTraits<int32_t>::output(ثابت int32_t &Val, فراغ *,
                                   raw_ostream &Out) {
  Out << Val;
}

StringRef ScalarTraits<int32_t>::إدخال_2(StringRef Scalar, فراغ *, int32_t &Val) {
  long long N;
  إذا (getAsSignedInteger(Scalar, 0, N))
    إرجاع "invalid number";
  إذا ((N > INT32_MAX) || (N < INT32_MIN))
    إرجاع "out of range number";
  Val = N;
  إرجاع StringRef();
}

فراغ ScalarTraits<int64_t>::output(ثابت int64_t &Val, فراغ *,
                                   raw_ostream &Out) {
  Out << Val;
}

StringRef ScalarTraits<int64_t>::إدخال_2(StringRef Scalar, فراغ *, int64_t &Val) {
  long long N;
  إذا (getAsSignedInteger(Scalar, 0, N))
    إرجاع "invalid number";
  Val = N;
  إرجاع StringRef();
}

فراغ ScalarTraits<مزدوج>::output(ثابت مزدوج &Val, فراغ *, raw_ostream &Out) {
  Out << format("%g", Val);
}

StringRef ScalarTraits<مزدوج>::إدخال_2(StringRef Scalar, فراغ *, مزدوج &Val) {
  إذا (to_float(Scalar, Val))
    إرجاع StringRef();
  إرجاع "invalid floating point number";
}

فراغ ScalarTraits<عائم>::output(ثابت عائم &Val, فراغ *, raw_ostream &Out) {
  Out << format("%g", Val);
}

StringRef ScalarTraits<عائم>::إدخال_2(StringRef Scalar, فراغ *, عائم &Val) {
  إذا (to_float(Scalar, Val))
    إرجاع StringRef();
  إرجاع "invalid floating point number";
}

فراغ ScalarTraits<Hex8>::output(ثابت Hex8 &Val, فراغ *, raw_ostream &Out) {
  Out << format("0x%" PRIX8, (uint8_t)Val);
}

StringRef ScalarTraits<Hex8>::إدخال_2(StringRef Scalar, فراغ *, Hex8 &Val) {
  unsigned long long n;
  إذا (getAsUnsignedInteger(Scalar, 0, n))
    إرجاع "invalid hex8 number";
  إذا (n > 0xFF)
    إرجاع "out of range hex8 number";
  Val = n;
  إرجاع StringRef();
}

فراغ ScalarTraits<Hex16>::output(ثابت Hex16 &Val, فراغ *, raw_ostream &Out) {
  Out << format("0x%" PRIX16, (uint16_t)Val);
}

StringRef ScalarTraits<Hex16>::إدخال_2(StringRef Scalar, فراغ *, Hex16 &Val) {
  unsigned long long n;
  إذا (getAsUnsignedInteger(Scalar, 0, n))
    إرجاع "invalid hex16 number";
  إذا (n > 0xFFFF)
    إرجاع "out of range hex16 number";
  Val = n;
  إرجاع StringRef();
}

فراغ ScalarTraits<Hex32>::output(ثابت Hex32 &Val, فراغ *, raw_ostream &Out) {
  Out << format("0x%" PRIX32, (uint32_t)Val);
}

StringRef ScalarTraits<Hex32>::إدخال_2(StringRef Scalar, فراغ *, Hex32 &Val) {
  unsigned long long n;
  إذا (getAsUnsignedInteger(Scalar, 0, n))
    إرجاع "invalid hex32 number";
  إذا (n > 0xFFFFFFFFUL)
    إرجاع "out of range hex32 number";
  Val = n;
  إرجاع StringRef();
}

فراغ ScalarTraits<Hex64>::output(ثابت Hex64 &Val, فراغ *, raw_ostream &Out) {
  Out << format("0x%" PRIX64, (uint64_t)Val);
}

StringRef ScalarTraits<Hex64>::إدخال_2(StringRef Scalar, فراغ *, Hex64 &Val) {
  unsigned long long رقم;
  إذا (getAsUnsignedInteger(Scalar, 0, رقم))
    إرجاع "invalid hex64 number";
  Val = رقم;
  إرجاع StringRef();
}

فراغ ScalarTraits<VersionTuple>::output(ثابت VersionTuple &Val, فراغ *,
                                        llvm::raw_ostream &Out) {
  Out << Val.getAsString();
}

StringRef ScalarTraits<VersionTuple>::إدخال_2(StringRef Scalar, فراغ *,
                                            VersionTuple &Val) {
  إذا (Val.tryParse(Scalar))
    إرجاع "invalid version format";
  إرجاع StringRef();
}
