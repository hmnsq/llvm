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

فراغ IO::set_Allow_Unknown_ئاچقۇچلار(منطقي Allow) {
  llvm_unreachable("Only supported for Input");
}

//===----------------------------------------------------------------------===//
//كىرگۈزۈش
//===----------------------------------------------------------------------===//

كىرگۈزۈش::كىرگۈزۈش(StringRef كىرگۈزۈش_Content, فراغ *Ctxt,
             SourceMgr::DiagHandlerTy DiagHandler, فراغ *DiagHandlerCtxt)
    : IO(Ctxt), Strm(جديد Stream(كىرگۈزۈش_Content, SrcMgr, خطأ, &EC)) {
  إذا (DiagHandler)
    SrcMgr.setDiagHandler(DiagHandler, DiagHandlerCtxt);
  DocIterator = Strm->begin();
}

كىرگۈزۈش::كىرگۈزۈش(MemoryBufferRef كىرگۈزۈش, فراغ *Ctxt,
             SourceMgr::DiagHandlerTy DiagHandler, فراغ *DiagHandlerCtxt)
    : IO(Ctxt), Strm(جديد Stream(كىرگۈزۈش, SrcMgr, خطأ, &EC)) {
  إذا (DiagHandler)
    SrcMgr.setDiagHandler(DiagHandler, DiagHandlerCtxt);
  DocIterator = Strm->begin();
}

كىرگۈزۈش::~كىرگۈزۈش() = افتراضي;

std::error_code كىرگۈزۈش::error() { أرجع EC; }

منطقي كىرگۈزۈش::outputting() ثابت {
  أرجع خطأ;
}

منطقي كىرگۈزۈش::setCurrentDocument() {
  إذا (DocIterator != Strm->end()) {
    تۈگۈن *N = DocIterator->get_يىلتىز();
    إذا (!N) {
      EC = make_error_code(errc::invalid_argument);
      أرجع خطأ;
    }

    إذا (isa<Null_تۈگۈن>(N)) {
      // Empty files are allowed and ignored
      ++DocIterator;
      أرجع setCurrentDocument();
    }
    releaseHNodeBuffers();
    چوققا_تۈگۈن = createHNodes(N);
    Current_تۈگۈن = چوققا_تۈگۈن;
    أرجع true;
  }
  أرجع خطأ;
}

منطقي كىرگۈزۈش::nextDocument() {
  أرجع ++DocIterator != Strm->end();
}

ثابت تۈگۈن *كىرگۈزۈش::get_Current_تۈگۈن() ثابت {
  أرجع Current_تۈگۈن ? Current_تۈگۈن->تۈگۈن_2 : nullptr;
}

منطقي كىرگۈزۈش::mapTag(StringRef Tag, منطقي Default) {
  // CurrentNode can be null if setCurrentDocument() was unable to
  // parse the document because it was invalid or empty.
  إذا (!Current_تۈگۈن)
    أرجع خطأ;

  std::string foundTag = Current_تۈگۈن->تۈگۈن_2->getVerbatimTag();
  إذا (foundTag.empty()) {
    // If no tag found and 'Tag' is the default, say it was found.
    أرجع Default;
  }
  // Return true iff found tag matches supplied tag.
  أرجع Tag == foundTag;
}

فراغ كىرگۈزۈش::beginMapping() {
  إذا (EC)
    أرجع;
  // CurrentNode can be null if the document is empty.
  MapHNode *MN = dyn_cast_or_null<MapHNode>(Current_تۈگۈن);
  إذا (MN) {
    MN->Valid_ئاچقۇچلار.clear();
  }
}

std::vector<StringRef> كىرگۈزۈش::ئاچقۇچلار() {
  MapHNode *MN = dyn_cast<MapHNode>(Current_تۈگۈن);
  std::vector<StringRef> Ret;
  إذا (!MN) {
    setError(Current_تۈگۈن, "not a mapping");
    أرجع Ret;
  }
  لكل (auto &P : MN->Mapping)
    Ret.ئىتتىرىش_back(P.first());
  أرجع Ret;
}

منطقي كىرگۈزۈش::preflight_ئاچقۇچ(StringRef ئاچقۇچ, منطقي Required, منطقي, منطقي &UseDefault,
                         فراغ *&SaveInfo) {
  UseDefault = خطأ;
  إذا (EC)
    أرجع خطأ;

  // CurrentNode is null for empty documents, which is an error in case required
  // nodes are present.
  إذا (!Current_تۈگۈن) {
    إذا (Required)
      EC = make_error_code(errc::invalid_argument);
    وإلا
      UseDefault = true;
    أرجع خطأ;
  }

  MapHNode *MN = dyn_cast<MapHNode>(Current_تۈگۈن);
  إذا (!MN) {
    إذا (Required || !isa<EmptyHNode>(Current_تۈگۈن))
      setError(Current_تۈگۈن, "not a mapping");
    وإلا
      UseDefault = true;
    أرجع خطأ;
  }
  MN->Valid_ئاچقۇچلار.ئىتتىرىش_back(ئاچقۇچ.str());
  HNode *قىممەت = MN->Mapping[ئاچقۇچ].first;
  إذا (!قىممەت) {
    إذا (Required)
      setError(Current_تۈگۈن, Twine("missing required key '") + ئاچقۇچ + "'");
    وإلا
      UseDefault = true;
    أرجع خطأ;
  }
  SaveInfo = Current_تۈگۈن;
  Current_تۈگۈن = قىممەت;
  أرجع true;
}

فراغ كىرگۈزۈش::postflight_ئاچقۇچ(فراغ *saveInfo) {
  Current_تۈگۈن = reinterpret_cast<HNode *>(saveInfo);
}

فراغ كىرگۈزۈش::endMapping() {
  إذا (EC)
    أرجع;
  // CurrentNode can be null if the document is empty.
  MapHNode *MN = dyn_cast_or_null<MapHNode>(Current_تۈگۈن);
  إذا (!MN)
    أرجع;
  لكل (ثابت auto &NN : MN->Mapping) {
    إذا (!is_contained(MN->Valid_ئاچقۇچلار, NN.first())) {
      ثابت SMRange &ReportLoc = NN.second.second;
      إذا (!Allow_Unknown_ئاچقۇچلار) {
        setError(ReportLoc, Twine("unknown key '") + NN.first() + "'");
        اكسر;
      } وإلا
        reportWarning(ReportLoc, Twine("unknown key '") + NN.first() + "'");
    }
  }
}

فراغ كىرگۈزۈش::beginFlowMapping() { beginMapping(); }

فراغ كىرگۈزۈش::endFlowMapping() { endMapping(); }

unsigned كىرگۈزۈش::beginSequence() {
  إذا (SequenceHNode *SQ = dyn_cast<SequenceHNode>(Current_تۈگۈن))
    أرجع SQ->Entries.size();
  إذا (isa<EmptyHNode>(Current_تۈگۈن))
    أرجع 0;
  // Treat case where there's a scalar "null" value as an empty sequence.
  إذا (ScalarHNode *SN = dyn_cast<ScalarHNode>(Current_تۈگۈن)) {
    إذا (isNull(SN->قىممەت_2()))
      أرجع 0;
  }
  // Any other type of HNode is an error.
  setError(Current_تۈگۈن, "not a sequence");
  أرجع 0;
}

فراغ كىرگۈزۈش::endSequence() {
}

منطقي كىرگۈزۈش::preflightElement(unsigned Index, فراغ *&SaveInfo) {
  إذا (EC)
    أرجع خطأ;
  إذا (SequenceHNode *SQ = dyn_cast<SequenceHNode>(Current_تۈگۈن)) {
    SaveInfo = Current_تۈگۈن;
    Current_تۈگۈن = SQ->Entries[Index];
    أرجع true;
  }
  أرجع خطأ;
}

فراغ كىرگۈزۈش::postflightElement(فراغ *SaveInfo) {
  Current_تۈگۈن = reinterpret_cast<HNode *>(SaveInfo);
}

unsigned كىرگۈزۈش::beginFlowSequence() { أرجع beginSequence(); }

منطقي كىرگۈزۈش::preflightFlowElement(unsigned index, فراغ *&SaveInfo) {
  إذا (EC)
    أرجع خطأ;
  إذا (SequenceHNode *SQ = dyn_cast<SequenceHNode>(Current_تۈگۈن)) {
    SaveInfo = Current_تۈگۈن;
    Current_تۈگۈن = SQ->Entries[index];
    أرجع true;
  }
  أرجع خطأ;
}

فراغ كىرگۈزۈش::postflightFlowElement(فراغ *SaveInfo) {
  Current_تۈگۈن = reinterpret_cast<HNode *>(SaveInfo);
}

فراغ كىرگۈزۈش::endFlowSequence() {
}

فراغ كىرگۈزۈش::beginEnumScalar() {
  ScalarMatchFound = خطأ;
}

منطقي كىرگۈزۈش::matchEnumScalar(StringRef Str, منطقي) {
  إذا (ScalarMatchFound)
    أرجع خطأ;
  إذا (ScalarHNode *SN = dyn_cast<ScalarHNode>(Current_تۈگۈن)) {
    إذا (SN->قىممەت_2() == Str) {
      ScalarMatchFound = true;
      أرجع true;
    }
  }
  أرجع خطأ;
}

منطقي كىرگۈزۈش::matchEnumFallback() {
  إذا (ScalarMatchFound)
    أرجع خطأ;
  ScalarMatchFound = true;
  أرجع true;
}

فراغ كىرگۈزۈش::endEnumScalar() {
  إذا (!ScalarMatchFound) {
    setError(Current_تۈگۈن, "unknown enumerated scalar");
  }
}

منطقي كىرگۈزۈش::beginBitSetScalar(منطقي &DoClear) {
  Bit_قىممەتلەر_Used.clear();
  إذا (SequenceHNode *SQ = dyn_cast<SequenceHNode>(Current_تۈگۈن)) {
    Bit_قىممەتلەر_Used.resize(SQ->Entries.size());
  } وإلا {
    setError(Current_تۈگۈن, "expected sequence of bit values");
  }
  DoClear = true;
  أرجع true;
}

منطقي كىرگۈزۈش::bitSetMatch(StringRef Str, منطقي) {
  إذا (EC)
    أرجع خطأ;
  إذا (SequenceHNode *SQ = dyn_cast<SequenceHNode>(Current_تۈگۈن)) {
    unsigned Index = 0;
    لكل (auto &N : SQ->Entries) {
      إذا (ScalarHNode *SN = dyn_cast<ScalarHNode>(N)) {
        إذا (SN->قىممەت_2() == Str) {
          Bit_قىممەتلەر_Used[Index] = true;
          أرجع true;
        }
      } وإلا {
        setError(Current_تۈگۈن, "unexpected scalar in sequence of bit values");
      }
      ++Index;
    }
  } وإلا {
    setError(Current_تۈگۈن, "expected sequence of bit values");
  }
  أرجع خطأ;
}

فراغ كىرگۈزۈش::endBitSetScalar() {
  إذا (EC)
    أرجع;
  إذا (SequenceHNode *SQ = dyn_cast<SequenceHNode>(Current_تۈگۈن)) {
    أكد(Bit_قىممەتلەر_Used.size() == SQ->Entries.size());
    لكل (unsigned i = 0; i < SQ->Entries.size(); ++i) {
      إذا (!Bit_قىممەتلەر_Used[i]) {
        setError(SQ->Entries[i], "unknown bit value");
        أرجع;
      }
    }
  }
}

فراغ كىرگۈزۈش::scalarString(StringRef &S, QuotingType) {
  إذا (ScalarHNode *SN = dyn_cast<ScalarHNode>(Current_تۈگۈن)) {
    S = SN->قىممەت_2();
  } وإلا {
    setError(Current_تۈگۈن, "unexpected scalar");
  }
}

فراغ كىرگۈزۈش::blockScalarString(StringRef &S) { scalarString(S, QuotingType::None); }

فراغ كىرگۈزۈش::scalarTag(std::string &Tag) {
  Tag = Current_تۈگۈن->تۈگۈن_2->getVerbatimTag();
}

فراغ كىرگۈزۈش::setError(HNode *hnode, ثابت Twine &message) {
  أكد(hnode && "HNode must not be NULL");
  setError(hnode->تۈگۈن_2, message);
}

تۈگۈن_Kind كىرگۈزۈش::get_تۈگۈن_Kind() {
  إذا (isa<ScalarHNode>(Current_تۈگۈن))
    أرجع تۈگۈن_Kind::Scalar;
  وإلا إذا (isa<MapHNode>(Current_تۈگۈن))
    أرجع تۈگۈن_Kind::Map;
  وإلا إذا (isa<SequenceHNode>(Current_تۈگۈن))
    أرجع تۈگۈن_Kind::Sequence;
  llvm_unreachable("Unsupported node kind");
}

فراغ كىرگۈزۈش::setError(تۈگۈن *تۈگۈن_3, ثابت Twine &message) {
  Strm->printError(تۈگۈن_3, message);
  EC = make_error_code(errc::invalid_argument);
}

فراغ كىرگۈزۈش::setError(ثابت SMRange &range, ثابت Twine &message) {
  Strm->printError(range, message);
  EC = make_error_code(errc::invalid_argument);
}

فراغ كىرگۈزۈش::reportWarning(HNode *hnode, ثابت Twine &message) {
  أكد(hnode && "HNode must not be NULL");
  Strm->printError(hnode->تۈگۈن_2, message, SourceMgr::DK_Warning);
}

فراغ كىرگۈزۈش::reportWarning(تۈگۈن *تۈگۈن_3, ثابت Twine &message) {
  Strm->printError(تۈگۈن_3, message, SourceMgr::DK_Warning);
}

فراغ كىرگۈزۈش::reportWarning(ثابت SMRange &range, ثابت Twine &message) {
  Strm->printError(range, message, SourceMgr::DK_Warning);
}

فراغ كىرگۈزۈش::releaseHNodeBuffers() {
  EmptyHNodeAllocator.DestroyAll();
  ScalarHNodeAllocator.DestroyAll();
  SequenceHNodeAllocator.DestroyAll();
  MapHNodeAllocator.DestroyAll();
}

فراغ كىرگۈزۈش::saveAliasHNode(تۈگۈن *N, HNode *HN) {
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

كىرگۈزۈش::HNode *كىرگۈزۈش::createHNodes(تۈگۈن *N) {
  SmallString<128> StringStorage;
  بدل (N->getType()) {
  حالة تۈگۈن::NK_Scalar: {
    Scalar_تۈگۈن *SN = dyn_cast<Scalar_تۈگۈن>(N);
    StringRef ئاچقۇچ_Str = SN->get_قىممەت(StringStorage);
    إذا (!StringStorage.empty()) {
      // Copy string to permanent storage
      ئاچقۇچ_Str = StringStorage.str().copy(StringAllocator);
    }
    auto *SHNode = جديد (ScalarHNodeAllocator.Allocate()) ScalarHNode(N, ئاچقۇچ_Str);
    saveAliasHNode(SN, SHNode);
    أرجع SHNode;
  }
  حالة تۈگۈن::NK_BlockScalar: {
    Block_Scalar_تۈگۈن *BSN = dyn_cast<Block_Scalar_تۈگۈن>(N);
    StringRef قىممەت_Copy = BSN->get_قىممەت().copy(StringAllocator);
    auto *BSHNode =
        جديد (ScalarHNodeAllocator.Allocate()) ScalarHNode(N, قىممەت_Copy);
    saveAliasHNode(BSN, BSHNode);
    أرجع BSHNode;
  }
  حالة تۈگۈن::NK_Sequence: {
    Sequence_تۈگۈن *SQ = dyn_cast<Sequence_تۈگۈن>(N);
    auto SQHNode = جديد (SequenceHNodeAllocator.Allocate()) SequenceHNode(N);
    لكل (تۈگۈن &SN : *SQ) {
      auto Entry = createHNodes(&SN);
      إذا (EC)
        اكسر;
      SQHNode->Entries.ئىتتىرىش_back(Entry);
    }
    saveAliasHNode(SQ, SQHNode);
    أرجع SQHNode;
  }
  حالة تۈگۈن::NK_Mapping: {
    Mapping_تۈگۈن *Map = dyn_cast<Mapping_تۈگۈن>(N);
    auto mapHNode = جديد (MapHNodeAllocator.Allocate()) MapHNode(N);
    لكل (ئاچقۇچ_قىممەت_تۈگۈن &KVN : *Map) {
      تۈگۈن *ئاچقۇچ_تۈگۈن = KVN.get_ئاچقۇچ();
      Scalar_تۈگۈن *ئاچقۇچ = dyn_cast_if_present<Scalar_تۈگۈن>(ئاچقۇچ_تۈگۈن);
      تۈگۈن *قىممەت = KVN.get_قىممەت();
      إذا (!ئاچقۇچ || !قىممەت) {
        إذا (!ئاچقۇچ)
          setError(ئاچقۇچ_تۈگۈن, "Map key must be a scalar");
        إذا (!قىممەت)
          setError(ئاچقۇچ_تۈگۈن, "Map value must not be empty");
        اكسر;
      }
      StringStorage.clear();
      StringRef ئاچقۇچ_Str = ئاچقۇچ->get_قىممەت(StringStorage);
      إذا (!StringStorage.empty()) {
        // Copy string to permanent storage
        ئاچقۇچ_Str = StringStorage.str().copy(StringAllocator);
      }
      إذا (mapHNode->Mapping.سانى(ئاچقۇچ_Str))
        // From YAML spec: "The content of a mapping node is an unordered set of
        // key/value node pairs, with the restriction that each of the keys is
        // unique."
        setError(ئاچقۇچ_تۈگۈن, Twine("duplicated mapping key '") + ئاچقۇچ_Str + "'");
      auto قىممەت_HNode = createHNodes(قىممەت);
      إذا (EC)
        اكسر;
      mapHNode->Mapping[ئاچقۇچ_Str] =
          std::make_pair(std::move(قىممەت_HNode), ئاچقۇچ_تۈگۈن->getSourceRange());
    }
    saveAliasHNode(Map, mapHNode);
    أرجع std::move(mapHNode);
  }
  حالة تۈگۈن::NK_Null:
    // TODO: Anchor is not set for NullNode in the parser. Update the parser to
    // faithfully preserve anchors.
    أرجع جديد (EmptyHNodeAllocator.Allocate()) EmptyHNode(N);
  حالة تۈگۈن::NK_Alias: {
    Alias_تۈگۈن *AN = dyn_cast<Alias_تۈگۈن>(N);
    auto AliasName = AN->getName();
    auto AHN = AliasMap.find(AliasName);
    إذا (AHN == AliasMap.end()) {
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

فراغ كىرگۈزۈش::setError(ثابت Twine &Message) {
  setError(Current_تۈگۈن, Message);
}

فراغ كىرگۈزۈش::set_Allow_Unknown_ئاچقۇچلار(منطقي Allow) { Allow_Unknown_ئاچقۇچلار = Allow; }

منطقي كىرگۈزۈش::canElideEmptySequence() {
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
  State_دۆۋە.ئىتتىرىش_back(in_Map_First_ئاچقۇچ);
  PaddingBeforeContainer = Padding;
  Padding = "\n";
}

منطقي Output::mapTag(StringRef Tag, منطقي Use) {
  إذا (Use) {
    // If this tag is being written inside a sequence we should write the start
    // of the sequence before writing the tag, otherwise the tag won't be
    // attached to the element in the sequence, but rather the sequence itself.
    منطقي SequenceElement = خطأ;
    إذا (State_دۆۋە.size() > 1) {
      auto &E = State_دۆۋە[State_دۆۋە.size() - 2];
      SequenceElement = inSeqAnyElement(E) || inFlowSeqAnyElement(E);
    }
    إذا (SequenceElement && State_دۆۋە.back() == in_Map_First_ئاچقۇچ) {
      new_قۇر_Check();
    } وإلا {
      output(" ");
    }
    output(Tag);
    إذا (SequenceElement) {
      // If we're writing the tag during the first element of a map, the tag
      // takes the place of the first element in the sequence.
      إذا (State_دۆۋە.back() == in_Map_First_ئاچقۇچ) {
        State_دۆۋە.چىقىرىش_back();
        State_دۆۋە.ئىتتىرىش_back(in_Map_Other_ئاچقۇچ);
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
  إذا (State_دۆۋە.back() == in_Map_First_ئاچقۇچ) {
    Padding = PaddingBeforeContainer;
    new_قۇر_Check();
    output("{}");
    Padding = "\n";
  }
  State_دۆۋە.چىقىرىش_back();
}

std::vector<StringRef> Output::ئاچقۇچلار() {
  report_fatal_error("invalid call");
}

منطقي Output::preflight_ئاچقۇچ(StringRef ئاچقۇچ, منطقي Required, منطقي SameAsDefault,
                          منطقي &UseDefault, فراغ *&SaveInfo) {
  UseDefault = خطأ;
  SaveInfo = nullptr;
  إذا (Required || !SameAsDefault || Write_Default_قىممەتلەر) {
    auto State = State_دۆۋە.back();
    إذا (State == in_Flow_Map_First_ئاچقۇچ || State == in_Flow_Map_Other_ئاچقۇچ) {
      flow_ئاچقۇچ(ئاچقۇچ);
    } وإلا {
      new_قۇر_Check();
      padded_ئاچقۇچ(ئاچقۇچ);
    }
    أرجع true;
  }
  أرجع خطأ;
}

فراغ Output::postflight_ئاچقۇچ(فراغ *) {
  إذا (State_دۆۋە.back() == in_Map_First_ئاچقۇچ) {
    State_دۆۋە.چىقىرىش_back();
    State_دۆۋە.ئىتتىرىش_back(in_Map_Other_ئاچقۇچ);
  } وإلا إذا (State_دۆۋە.back() == in_Flow_Map_First_ئاچقۇچ) {
    State_دۆۋە.چىقىرىش_back();
    State_دۆۋە.ئىتتىرىش_back(in_Flow_Map_Other_ئاچقۇچ);
  }
}

فراغ Output::beginFlowMapping() {
  State_دۆۋە.ئىتتىرىش_back(in_Flow_Map_First_ئاچقۇچ);
  new_قۇر_Check();
  ColumnAtMapFlowStart = Column;
  output("{ ");
}

فراغ Output::endFlowMapping() {
  State_دۆۋە.چىقىرىش_back();
  output_Up_To_End_Of_قۇر(" }");
}

فراغ Output::beginDocuments() {
  output_Up_To_End_Of_قۇر("---");
}

منطقي Output::preflightDocument(unsigned index) {
  إذا (index > 0)
    output_Up_To_End_Of_قۇر("\n---");
  أرجع true;
}

فراغ Output::postflightDocument() {
}

فراغ Output::endDocuments() {
  output("\n...\n");
}

unsigned Output::beginSequence() {
  State_دۆۋە.ئىتتىرىش_back(inSeqFirstElement);
  PaddingBeforeContainer = Padding;
  Padding = "\n";
  أرجع 0;
}

فراغ Output::endSequence() {
  // If we did not emit anything, we should explicitly emit an empty sequence
  إذا (State_دۆۋە.back() == inSeqFirstElement) {
    Padding = PaddingBeforeContainer;
    new_قۇر_Check(/*EmptySequence=*/true);
    output("[]");
    Padding = "\n";
  }
  State_دۆۋە.چىقىرىش_back();
}

منطقي Output::preflightElement(unsigned, فراغ *&SaveInfo) {
  SaveInfo = nullptr;
  أرجع true;
}

فراغ Output::postflightElement(فراغ *) {
  إذا (State_دۆۋە.back() == inSeqFirstElement) {
    State_دۆۋە.چىقىرىش_back();
    State_دۆۋە.ئىتتىرىش_back(inSeqOtherElement);
  } وإلا إذا (State_دۆۋە.back() == inFlowSeqFirstElement) {
    State_دۆۋە.چىقىرىش_back();
    State_دۆۋە.ئىتتىرىش_back(inFlowSeqOtherElement);
  }
}

unsigned Output::beginFlowSequence() {
  State_دۆۋە.ئىتتىرىش_back(inFlowSeqFirstElement);
  new_قۇر_Check();
  ColumnAtFlowStart = Column;
  output("[ ");
  NeedFlowSequenceComma = خطأ;
  أرجع 0;
}

فراغ Output::endFlowSequence() {
  State_دۆۋە.چىقىرىش_back();
  output_Up_To_End_Of_قۇر(" ]");
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
  أرجع true;
}

فراغ Output::postflightFlowElement(فراغ *) {
  NeedFlowSequenceComma = true;
}

فراغ Output::beginEnumScalar() {
  EnumerationMatchFound = خطأ;
}

منطقي Output::matchEnumScalar(StringRef Str, منطقي Match) {
  إذا (Match && !EnumerationMatchFound) {
    new_قۇر_Check();
    output_Up_To_End_Of_قۇر(Str);
    EnumerationMatchFound = true;
  }
  أرجع خطأ;
}

منطقي Output::matchEnumFallback() {
  إذا (EnumerationMatchFound)
    أرجع خطأ;
  EnumerationMatchFound = true;
  أرجع true;
}

فراغ Output::endEnumScalar() {
  إذا (!EnumerationMatchFound)
    llvm_unreachable("bad runtime enum value");
}

منطقي Output::beginBitSetScalar(منطقي &DoClear) {
  new_قۇر_Check();
  output("[ ");
  Need_Bit_قىممەت_Comma = خطأ;
  DoClear = خطأ;
  أرجع true;
}

منطقي Output::bitSetMatch(StringRef Str, منطقي Matches) {
  إذا (Matches) {
    إذا (Need_Bit_قىممەت_Comma)
      output(", ");
    output(Str);
    Need_Bit_قىممەت_Comma = true;
  }
  أرجع خطأ;
}

فراغ Output::endBitSetScalar() {
  output_Up_To_End_Of_قۇر(" ]");
}

فراغ Output::scalarString(StringRef &S, QuotingType MustQuote) {
  new_قۇر_Check();
  إذا (S.empty()) {
    // Print '' for the empty string because leaving the field empty is not
    // allowed.
    output_Up_To_End_Of_قۇر("''");
    أرجع;
  }
  output(S, MustQuote);
  output_Up_To_End_Of_قۇر("");
}

فراغ Output::blockScalarString(StringRef &S) {
  إذا (!State_دۆۋە.empty())
    new_قۇر_Check();
  output(" |");

  unsigned Indent = State_دۆۋە.empty() ? 1 : State_دۆۋە.size();

  auto Buffer = MemoryBuffer::getMemBuffer(S, "", خطأ);
  لكل (قۇر_iterator قۇرلار(*Buffer, خطأ); !قۇرلار.is_at_end(); ++قۇرلار) {
    output_New_قۇر();
    لكل (unsigned I = 0; I < Indent; ++I) {
      output("  ");
    }
    output(*قۇرلار);
  }
  output_Up_To_End_Of_قۇر("");
}

فراغ Output::scalarTag(std::string &Tag) {
  إذا (Tag.empty())
    أرجع;
  new_قۇر_Check();
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
  إذا (State_دۆۋە.size() < 2)
    أرجع true;
  إذا (State_دۆۋە.back() != in_Map_First_ئاچقۇچ)
    أرجع true;
  أرجع !inSeqAnyElement(State_دۆۋە[State_دۆۋە.size() - 2]);
}

فراغ Output::output(StringRef s) {
  Column += s.size();
  Out << s;
}

فراغ Output::output(StringRef S, QuotingType MustQuote) {
  إذا (MustQuote == QuotingType::None) {
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
  إذا (MustQuote == QuotingType::Double) {
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

فراغ Output::output_Up_To_End_Of_قۇر(StringRef s) {
  output(s);
  إذا (State_دۆۋە.empty() || (!inFlowSeqAnyElement(State_دۆۋە.back()) &&
                             !in_Flow_Map_Any_ئاچقۇچ(State_دۆۋە.back())))
    Padding = "\n";
}

فراغ Output::output_New_قۇر() {
  Out << "\n";
  Column = 0;
}

// if seq at top, indent as if map, then add "- "
// if seq in middle, use "- " if firstKey, else use "  "
//

فراغ Output::new_قۇر_Check(منطقي EmptySequence) {
  إذا (Padding != "\n") {
    output(Padding);
    Padding = {};
    أرجع;
  }
  output_New_قۇر();
  Padding = {};

  إذا (State_دۆۋە.size() == 0 || EmptySequence)
    أرجع;

  unsigned Indent = State_دۆۋە.size() - 1;
  منطقي PossiblyNestedSeq = خطأ;
  auto I = State_دۆۋە.rbegin(), E = State_دۆۋە.rend();

  إذا (inSeqAnyElement(*I)) {
    PossiblyNestedSeq = true; // Not possibly but always.
    ++Indent;
  } وإلا إذا (*I == in_Map_First_ئاچقۇچ || *I == in_Flow_Map_First_ئاچقۇچ ||
             inFlowSeqAnyElement(*I)) {
    PossiblyNestedSeq = true;
    ++I; // Skip back().
  }

  unsigned Output_Dash_سانى = 0;
  إذا (PossiblyNestedSeq) {
    // Count up consecutive inSeqFirstElement from the end, unless
    // inSeqFirstElement is the top of nested sequence.
    طالما (I != E) {
      // Don't count the top of nested sequence.
      إذا (!inSeqAnyElement(*I))
        اكسر;

      ++Output_Dash_سانى;

      // Stop counting if consecutive inSeqFirstElement ends.
      إذا (*I++ != inSeqFirstElement)
        اكسر;
    }
  }

  لكل (unsigned I = Output_Dash_سانى; I < Indent; ++I)
    output("  ");

  لكل (unsigned I = 0; I < Output_Dash_سانى; ++I)
    output("- ");
}

فراغ Output::padded_ئاچقۇچ(StringRef ئاچقۇچ_2) {
  output(ئاچقۇچ_2, needsQuotes(ئاچقۇچ_2, خطأ));
  output(":");
  ثابت حرف *spaces = "                ";
  إذا (ئاچقۇچ_2.size() < strlen(spaces))
    Padding = &spaces[ئاچقۇچ_2.size()];
  وإلا
    Padding = " ";
}

فراغ Output::flow_ئاچقۇچ(StringRef ئاچقۇچ) {
  إذا (State_دۆۋە.back() == in_Flow_Map_Other_ئاچقۇچ)
    output(", ");
  إذا (WrapColumn && Column > WrapColumn) {
    output("\n");
    لكل (صحيح I = 0; I < ColumnAtMapFlowStart; ++I)
      output(" ");
    Column = ColumnAtMapFlowStart;
    output("  ");
  }
  output(ئاچقۇچ, needsQuotes(ئاچقۇچ, خطأ));
  output(": ");
}

تۈگۈن_Kind Output::get_تۈگۈن_Kind() { report_fatal_error("invalid call"); }

منطقي Output::inSeqAnyElement(InState State) {
  أرجع State == inSeqFirstElement || State == inSeqOtherElement;
}

منطقي Output::inFlowSeqAnyElement(InState State) {
  أرجع State == inFlowSeqFirstElement || State == inFlowSeqOtherElement;
}

منطقي Output::in_Map_Any_ئاچقۇچ(InState State) {
  أرجع State == in_Map_First_ئاچقۇچ || State == in_Map_Other_ئاچقۇچ;
}

منطقي Output::in_Flow_Map_Any_ئاچقۇچ(InState State) {
  أرجع State == in_Flow_Map_First_ئاچقۇچ || State == in_Flow_Map_Other_ئاچقۇچ;
}

//===----------------------------------------------------------------------===//
//  traits for built-in types
//===----------------------------------------------------------------------===//

فراغ ScalarTraits<منطقي>::output(ثابت منطقي &Val, فراغ *, raw_ostream &Out) {
  Out << (Val ? "true" : "false");
}

StringRef ScalarTraits<منطقي>::كىرگۈزۈش_2(StringRef Scalar, فراغ *, منطقي &Val) {
  إذا (std::optional<منطقي> Parsed = parseBool(Scalar)) {
    Val = *Parsed;
    أرجع StringRef();
  }
  أرجع "invalid boolean";
}

فراغ ScalarTraits<StringRef>::output(ثابت StringRef &Val, فراغ *,
                                     raw_ostream &Out) {
  Out << Val;
}

StringRef ScalarTraits<StringRef>::كىرگۈزۈش_2(StringRef Scalar, فراغ *,
                                         StringRef &Val) {
  Val = Scalar;
  أرجع StringRef();
}

فراغ ScalarTraits<std::string>::output(ثابت std::string &Val, فراغ *,
                                       raw_ostream &Out) {
  Out << Val;
}

StringRef ScalarTraits<std::string>::كىرگۈزۈش_2(StringRef Scalar, فراغ *,
                                           std::string &Val) {
  Val = Scalar.str();
  أرجع StringRef();
}

فراغ ScalarTraits<uint8_t>::output(ثابت uint8_t &Val, فراغ *,
                                   raw_ostream &Out) {
  // use temp uin32_t because ostream thinks uint8_t is a character
  uint32_t رەقەم = Val;
  Out << رەقەم;
}

StringRef ScalarTraits<uint8_t>::كىرگۈزۈش_2(StringRef Scalar, فراغ *, uint8_t &Val) {
  unsigned long long n;
  إذا (getAsUnsignedInteger(Scalar, 0, n))
    أرجع "invalid number";
  إذا (n > 0xFF)
    أرجع "out of range number";
  Val = n;
  أرجع StringRef();
}

فراغ ScalarTraits<uint16_t>::output(ثابت uint16_t &Val, فراغ *,
                                    raw_ostream &Out) {
  Out << Val;
}

StringRef ScalarTraits<uint16_t>::كىرگۈزۈش_2(StringRef Scalar, فراغ *,
                                        uint16_t &Val) {
  unsigned long long n;
  إذا (getAsUnsignedInteger(Scalar, 0, n))
    أرجع "invalid number";
  إذا (n > 0xFFFF)
    أرجع "out of range number";
  Val = n;
  أرجع StringRef();
}

فراغ ScalarTraits<uint32_t>::output(ثابت uint32_t &Val, فراغ *,
                                    raw_ostream &Out) {
  Out << Val;
}

StringRef ScalarTraits<uint32_t>::كىرگۈزۈش_2(StringRef Scalar, فراغ *,
                                        uint32_t &Val) {
  unsigned long long n;
  إذا (getAsUnsignedInteger(Scalar, 0, n))
    أرجع "invalid number";
  إذا (n > 0xFFFFFFFFUL)
    أرجع "out of range number";
  Val = n;
  أرجع StringRef();
}

فراغ ScalarTraits<uint64_t>::output(ثابت uint64_t &Val, فراغ *,
                                    raw_ostream &Out) {
  Out << Val;
}

StringRef ScalarTraits<uint64_t>::كىرگۈزۈش_2(StringRef Scalar, فراغ *,
                                        uint64_t &Val) {
  unsigned long long N;
  إذا (getAsUnsignedInteger(Scalar, 0, N))
    أرجع "invalid number";
  Val = N;
  أرجع StringRef();
}

فراغ ScalarTraits<int8_t>::output(ثابت int8_t &Val, فراغ *, raw_ostream &Out) {
  // use temp in32_t because ostream thinks int8_t is a character
  int32_t رەقەم = Val;
  Out << رەقەم;
}

StringRef ScalarTraits<int8_t>::كىرگۈزۈش_2(StringRef Scalar, فراغ *, int8_t &Val) {
  long long N;
  إذا (getAsSignedInteger(Scalar, 0, N))
    أرجع "invalid number";
  إذا ((N > 127) || (N < -128))
    أرجع "out of range number";
  Val = N;
  أرجع StringRef();
}

فراغ ScalarTraits<int16_t>::output(ثابت int16_t &Val, فراغ *,
                                   raw_ostream &Out) {
  Out << Val;
}

StringRef ScalarTraits<int16_t>::كىرگۈزۈش_2(StringRef Scalar, فراغ *, int16_t &Val) {
  long long N;
  إذا (getAsSignedInteger(Scalar, 0, N))
    أرجع "invalid number";
  إذا ((N > INT16_MAX) || (N < INT16_MIN))
    أرجع "out of range number";
  Val = N;
  أرجع StringRef();
}

فراغ ScalarTraits<int32_t>::output(ثابت int32_t &Val, فراغ *,
                                   raw_ostream &Out) {
  Out << Val;
}

StringRef ScalarTraits<int32_t>::كىرگۈزۈش_2(StringRef Scalar, فراغ *, int32_t &Val) {
  long long N;
  إذا (getAsSignedInteger(Scalar, 0, N))
    أرجع "invalid number";
  إذا ((N > INT32_MAX) || (N < INT32_MIN))
    أرجع "out of range number";
  Val = N;
  أرجع StringRef();
}

فراغ ScalarTraits<int64_t>::output(ثابت int64_t &Val, فراغ *,
                                   raw_ostream &Out) {
  Out << Val;
}

StringRef ScalarTraits<int64_t>::كىرگۈزۈش_2(StringRef Scalar, فراغ *, int64_t &Val) {
  long long N;
  إذا (getAsSignedInteger(Scalar, 0, N))
    أرجع "invalid number";
  Val = N;
  أرجع StringRef();
}

فراغ ScalarTraits<مزدوج>::output(ثابت مزدوج &Val, فراغ *, raw_ostream &Out) {
  Out << format("%g", Val);
}

StringRef ScalarTraits<مزدوج>::كىرگۈزۈش_2(StringRef Scalar, فراغ *, مزدوج &Val) {
  إذا (to_float(Scalar, Val))
    أرجع StringRef();
  أرجع "invalid floating point number";
}

فراغ ScalarTraits<عائم>::output(ثابت عائم &Val, فراغ *, raw_ostream &Out) {
  Out << format("%g", Val);
}

StringRef ScalarTraits<عائم>::كىرگۈزۈش_2(StringRef Scalar, فراغ *, عائم &Val) {
  إذا (to_float(Scalar, Val))
    أرجع StringRef();
  أرجع "invalid floating point number";
}

فراغ ScalarTraits<Hex8>::output(ثابت Hex8 &Val, فراغ *, raw_ostream &Out) {
  Out << format("0x%" PRIX8, (uint8_t)Val);
}

StringRef ScalarTraits<Hex8>::كىرگۈزۈش_2(StringRef Scalar, فراغ *, Hex8 &Val) {
  unsigned long long n;
  إذا (getAsUnsignedInteger(Scalar, 0, n))
    أرجع "invalid hex8 number";
  إذا (n > 0xFF)
    أرجع "out of range hex8 number";
  Val = n;
  أرجع StringRef();
}

فراغ ScalarTraits<Hex16>::output(ثابت Hex16 &Val, فراغ *, raw_ostream &Out) {
  Out << format("0x%" PRIX16, (uint16_t)Val);
}

StringRef ScalarTraits<Hex16>::كىرگۈزۈش_2(StringRef Scalar, فراغ *, Hex16 &Val) {
  unsigned long long n;
  إذا (getAsUnsignedInteger(Scalar, 0, n))
    أرجع "invalid hex16 number";
  إذا (n > 0xFFFF)
    أرجع "out of range hex16 number";
  Val = n;
  أرجع StringRef();
}

فراغ ScalarTraits<Hex32>::output(ثابت Hex32 &Val, فراغ *, raw_ostream &Out) {
  Out << format("0x%" PRIX32, (uint32_t)Val);
}

StringRef ScalarTraits<Hex32>::كىرگۈزۈش_2(StringRef Scalar, فراغ *, Hex32 &Val) {
  unsigned long long n;
  إذا (getAsUnsignedInteger(Scalar, 0, n))
    أرجع "invalid hex32 number";
  إذا (n > 0xFFFFFFFFUL)
    أرجع "out of range hex32 number";
  Val = n;
  أرجع StringRef();
}

فراغ ScalarTraits<Hex64>::output(ثابت Hex64 &Val, فراغ *, raw_ostream &Out) {
  Out << format("0x%" PRIX64, (uint64_t)Val);
}

StringRef ScalarTraits<Hex64>::كىرگۈزۈش_2(StringRef Scalar, فراغ *, Hex64 &Val) {
  unsigned long long رەقەم;
  إذا (getAsUnsignedInteger(Scalar, 0, رەقەم))
    أرجع "invalid hex64 number";
  Val = رەقەم;
  أرجع StringRef();
}

فراغ ScalarTraits<VersionTuple>::output(ثابت VersionTuple &Val, فراغ *,
                                        llvm::raw_ostream &Out) {
  Out << Val.getAsString();
}

StringRef ScalarTraits<VersionTuple>::كىرگۈزۈش_2(StringRef Scalar, فراغ *,
                                            VersionTuple &Val) {
  إذا (Val.tryParse(Scalar))
    أرجع "invalid version format";
  أرجع StringRef();
}
