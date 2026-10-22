//===- lib/Support/YAMLTraits.cpp -----------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#अन्तर्भाव "llvm/Support/YAMLTraits.h"
#अन्तर्भाव "llvm/ADT/STLExtras.h"
#अन्तर्भाव "llvm/ADT/SmallString.h"
#अन्तर्भाव "llvm/ADT/StringExtras.h"
#अन्तर्भाव "llvm/ADT/StringRef.h"
#अन्तर्भाव "llvm/ADT/Twine.h"
#अन्तर्भाव "llvm/Support/Casting.h"
#अन्तर्भाव "llvm/Support/Errc.h"
#अन्तर्भाव "llvm/Support/ErrorHandling.h"
#अन्तर्भाव "llvm/Support/Format.h"
#अन्तर्भाव "llvm/Support/LineIterator.h"
#अन्तर्भाव "llvm/Support/MemoryBuffer.h"
#अन्तर्भाव "llvm/Support/VersionTuple.h"
#अन्तर्भाव "llvm/Support/YAMLParser.h"
#अन्तर्भाव "llvm/Support/raw_ostream.h"
#अन्तर्भाव <cassert>
#अन्तर्भाव <cstdint>
#अन्तर्भाव <cstring>
#अन्तर्भाव <string>
#अन्तर्भाव <vector>

उपयुज्य नामाकाश llvm;
उपयुज्य नामाकाश yaml;

//===----------------------------------------------------------------------===//
//  IO
//===----------------------------------------------------------------------===//

IO::IO(शून्य *Context) : Ctxt(Context) {}

IO::~IO() = स्वभाव;

शून्य *IO::getContext() ध्रुव {
  प्रत्यागम Ctxt;
}

शून्य IO::समुच्चयः_Context(शून्य *Context) {
  Ctxt = Context;
}

शून्य IO::समुच्चयः_Allow_Unknown_कुञ्चिकाः(तार्किक Allow) {
  llvm_unreachable("Only supported for Input");
}

//===----------------------------------------------------------------------===//
//निवेशः
//===----------------------------------------------------------------------===//

निवेशः::निवेशः(StringRef निवेशः_Content, शून्य *Ctxt,
             SourceMgr::DiagHandlerTy DiagHandler, शून्य *DiagHandlerCtxt)
    : IO(Ctxt), Strm(नूतन प्रवाहः(निवेशः_Content, SrcMgr, असत्य, &EC)) {
  यदि (DiagHandler)
    SrcMgr.समुच्चयः_Diag_Handler(DiagHandler, DiagHandlerCtxt);
  DocIterator = Strm->begin();
}

निवेशः::निवेशः(स्मृतिः_बफर_Ref निवेशः, शून्य *Ctxt,
             SourceMgr::DiagHandlerTy DiagHandler, शून्य *DiagHandlerCtxt)
    : IO(Ctxt), Strm(नूतन प्रवाहः(निवेशः, SrcMgr, असत्य, &EC)) {
  यदि (DiagHandler)
    SrcMgr.समुच्चयः_Diag_Handler(DiagHandler, DiagHandlerCtxt);
  DocIterator = Strm->begin();
}

निवेशः::~निवेशः() = स्वभाव;

std::error_संहिता निवेशः::error() { प्रत्यागम EC; }

तार्किक निवेशः::outputting() ध्रुव {
  प्रत्यागम असत्य;
}

तार्किक निवेशः::समुच्चयः_Current_Document() {
  यदि (DocIterator != Strm->अन्तः()) {
    ग्रन्थिः *N = DocIterator->get_मूलम्();
    यदि (!N) {
      EC = make_error_संहिता(errc::invalid_तर्कः);
      प्रत्यागम असत्य;
    }

    यदि (isa<Null_ग्रन्थिः>(N)) {
      // Empty files are allowed and ignored
      ++DocIterator;
      प्रत्यागम समुच्चयः_Current_Document();
    }
    releaseHNodeBuffers();
    शिखरम्_ग्रन्थिः = createHNodes(N);
    Current_ग्रन्थिः = शिखरम्_ग्रन्थिः;
    प्रत्यागम सत्य;
  }
  प्रत्यागम असत्य;
}

तार्किक निवेशः::अग्र_Document() {
  प्रत्यागम ++DocIterator != Strm->अन्तः();
}

ध्रुव ग्रन्थिः *निवेशः::get_Current_ग्रन्थिः() ध्रुव {
  प्रत्यागम Current_ग्रन्थिः ? Current_ग्रन्थिः->ग्रन्थिः_2 : शून्यसूचकः;
}

तार्किक निवेशः::map_चिह्नकम्(StringRef चिह्नकम्, तार्किक Default) {
  // CurrentNode can be null if setCurrentDocument() was unable to
  // parse the document because it was invalid or empty.
  यदि (!Current_ग्रन्थिः)
    प्रत्यागम असत्य;

  std::string प्राप्तम्_चिह्नकम् = Current_ग्रन्थिः->ग्रन्थिः_2->get_Verbatim_चिह्नकम्();
  यदि (प्राप्तम्_चिह्नकम्.रिक्तानि()) {
    // If no tag found and 'Tag' is the default, say it was found.
    प्रत्यागम Default;
  }
  // Return true iff found tag matches supplied tag.
  प्रत्यागम चिह्नकम् == प्राप्तम्_चिह्नकम्;
}

शून्य निवेशः::beginMapping() {
  यदि (EC)
    प्रत्यागम;
  // CurrentNode can be null if the document is empty.
  MapHNode *MN = dyn_cast_or_null<MapHNode>(Current_ग्रन्थिः);
  यदि (MN) {
    MN->Valid_कुञ्चिकाः.clear();
  }
}

std::सदिशम्<StringRef> निवेशः::कुञ्चिकाः() {
  MapHNode *MN = dyn_cast<MapHNode>(Current_ग्रन्थिः);
  std::सदिशम्<StringRef> Ret;
  यदि (!MN) {
    समुच्चयः_Error(Current_ग्रन्थिः, "not a mapping");
    प्रत्यागम Ret;
  }
  प्रति (स्वचालित &P : MN->Mapping)
    Ret.प्रक्षिप_पृष्ठभागः(P.first());
  प्रत्यागम Ret;
}

तार्किक निवेशः::preflight_कुञ्चिका(StringRef कुञ्चिका, तार्किक Required, तार्किक, तार्किक &UseDefault,
                         शून्य *&SaveInfo) {
  UseDefault = असत्य;
  यदि (EC)
    प्रत्यागम असत्य;

  // CurrentNode is null for empty documents, which is an error in case required
  // nodes are present.
  यदि (!Current_ग्रन्थिः) {
    यदि (Required)
      EC = make_error_संहिता(errc::invalid_तर्कः);
    अन्यथा
      UseDefault = सत्य;
    प्रत्यागम असत्य;
  }

  MapHNode *MN = dyn_cast<MapHNode>(Current_ग्रन्थिः);
  यदि (!MN) {
    यदि (Required || !isa<रिक्तानि_HNode>(Current_ग्रन्थिः))
      समुच्चयः_Error(Current_ग्रन्थिः, "not a mapping");
    अन्यथा
      UseDefault = सत्य;
    प्रत्यागम असत्य;
  }
  MN->Valid_कुञ्चिकाः.प्रक्षिप_पृष्ठभागः(कुञ्चिका.str());
  HNode *मानम् = MN->Mapping[कुञ्चिका].first;
  यदि (!मानम्) {
    यदि (Required)
      समुच्चयः_Error(Current_ग्रन्थिः, Twine("missing required key '") + कुञ्चिका + "'");
    अन्यथा
      UseDefault = सत्य;
    प्रत्यागम असत्य;
  }
  SaveInfo = Current_ग्रन्थिः;
  Current_ग्रन्थिः = मानम्;
  प्रत्यागम सत्य;
}

शून्य निवेशः::postflight_कुञ्चिका(शून्य *saveInfo) {
  Current_ग्रन्थिः = reinterpret_cast<HNode *>(saveInfo);
}

शून्य निवेशः::अन्तः_Mapping() {
  यदि (EC)
    प्रत्यागम;
  // CurrentNode can be null if the document is empty.
  MapHNode *MN = dyn_cast_or_null<MapHNode>(Current_ग्रन्थिः);
  यदि (!MN)
    प्रत्यागम;
  प्रति (ध्रुव स्वचालित &NN : MN->Mapping) {
    यदि (!is_contained(MN->Valid_कुञ्चिकाः, NN.first())) {
      ध्रुव SMRange &ReportLoc = NN.second.second;
      यदि (!Allow_Unknown_कुञ्चिकाः) {
        समुच्चयः_Error(ReportLoc, Twine("unknown key '") + NN.first() + "'");
        भङ्ग;
      } अन्यथा
        reportWarning(ReportLoc, Twine("unknown key '") + NN.first() + "'");
    }
  }
}

शून्य निवेशः::beginFlowMapping() { beginMapping(); }

शून्य निवेशः::अन्तः_Flow_Mapping() { अन्तः_Mapping(); }

अचिह्नित निवेशः::beginSequence() {
  यदि (SequenceHNode *SQ = dyn_cast<SequenceHNode>(Current_ग्रन्थिः))
    प्रत्यागम SQ->Entries.आकार_2();
  यदि (isa<रिक्तानि_HNode>(Current_ग्रन्थिः))
    प्रत्यागम 0;
  // Treat case where there's a scalar "null" value as an empty sequence.
  यदि (ScalarHNode *SN = dyn_cast<ScalarHNode>(Current_ग्रन्थिः)) {
    यदि (isNull(SN->मानम्_2()))
      प्रत्यागम 0;
  }
  // Any other type of HNode is an error.
  समुच्चयः_Error(Current_ग्रन्थिः, "not a sequence");
  प्रत्यागम 0;
}

शून्य निवेशः::अन्तः_Sequence() {
}

तार्किक निवेशः::preflightElement(अचिह्नित सूचकांक_2, शून्य *&SaveInfo) {
  यदि (EC)
    प्रत्यागम असत्य;
  यदि (SequenceHNode *SQ = dyn_cast<SequenceHNode>(Current_ग्रन्थिः)) {
    SaveInfo = Current_ग्रन्थिः;
    Current_ग्रन्थिः = SQ->Entries[सूचकांक_2];
    प्रत्यागम सत्य;
  }
  प्रत्यागम असत्य;
}

शून्य निवेशः::postflightElement(शून्य *SaveInfo) {
  Current_ग्रन्थिः = reinterpret_cast<HNode *>(SaveInfo);
}

अचिह्नित निवेशः::beginFlowSequence() { प्रत्यागम beginSequence(); }

तार्किक निवेशः::preflightFlowElement(अचिह्नित सूचकांक, शून्य *&SaveInfo) {
  यदि (EC)
    प्रत्यागम असत्य;
  यदि (SequenceHNode *SQ = dyn_cast<SequenceHNode>(Current_ग्रन्थिः)) {
    SaveInfo = Current_ग्रन्थिः;
    Current_ग्रन्थिः = SQ->Entries[सूचकांक];
    प्रत्यागम सत्य;
  }
  प्रत्यागम असत्य;
}

शून्य निवेशः::postflightFlowElement(शून्य *SaveInfo) {
  Current_ग्रन्थिः = reinterpret_cast<HNode *>(SaveInfo);
}

शून्य निवेशः::अन्तः_Flow_Sequence() {
}

शून्य निवेशः::beginEnumScalar() {
  Scalar_मेलनम्_प्राप्तम् = असत्य;
}

तार्किक निवेशः::मेलनम्_Enum_Scalar(StringRef Str, तार्किक) {
  यदि (Scalar_मेलनम्_प्राप्तम्)
    प्रत्यागम असत्य;
  यदि (ScalarHNode *SN = dyn_cast<ScalarHNode>(Current_ग्रन्थिः)) {
    यदि (SN->मानम्_2() == Str) {
      Scalar_मेलनम्_प्राप्तम् = सत्य;
      प्रत्यागम सत्य;
    }
  }
  प्रत्यागम असत्य;
}

तार्किक निवेशः::मेलनम्_Enum_Fallback() {
  यदि (Scalar_मेलनम्_प्राप्तम्)
    प्रत्यागम असत्य;
  Scalar_मेलनम्_प्राप्तम् = सत्य;
  प्रत्यागम सत्य;
}

शून्य निवेशः::अन्तः_Enum_Scalar() {
  यदि (!Scalar_मेलनम्_प्राप्तम्) {
    समुच्चयः_Error(Current_ग्रन्थिः, "unknown enumerated scalar");
  }
}

तार्किक निवेशः::begin_Bit_समुच्चयः_Scalar(तार्किक &DoClear) {
  Bit_मानानि_प्रयुक्तम्.clear();
  यदि (SequenceHNode *SQ = dyn_cast<SequenceHNode>(Current_ग्रन्थिः)) {
    Bit_मानानि_प्रयुक्तम्.resize(SQ->Entries.आकार_2());
  } अन्यथा {
    समुच्चयः_Error(Current_ग्रन्थिः, "expected sequence of bit values");
  }
  DoClear = सत्य;
  प्रत्यागम सत्य;
}

तार्किक निवेशः::bit_समुच्चयः_मेलनम्(StringRef Str, तार्किक) {
  यदि (EC)
    प्रत्यागम असत्य;
  यदि (SequenceHNode *SQ = dyn_cast<SequenceHNode>(Current_ग्रन्थिः)) {
    अचिह्नित सूचकांक_2 = 0;
    प्रति (स्वचालित &N : SQ->Entries) {
      यदि (ScalarHNode *SN = dyn_cast<ScalarHNode>(N)) {
        यदि (SN->मानम्_2() == Str) {
          Bit_मानानि_प्रयुक्तम्[सूचकांक_2] = सत्य;
          प्रत्यागम सत्य;
        }
      } अन्यथा {
        समुच्चयः_Error(Current_ग्रन्थिः, "unexpected scalar in sequence of bit values");
      }
      ++सूचकांक_2;
    }
  } अन्यथा {
    समुच्चयः_Error(Current_ग्रन्थिः, "expected sequence of bit values");
  }
  प्रत्यागम असत्य;
}

शून्य निवेशः::अन्तः_Bit_समुच्चयः_Scalar() {
  यदि (EC)
    प्रत्यागम;
  यदि (SequenceHNode *SQ = dyn_cast<SequenceHNode>(Current_ग्रन्थिः)) {
    प्रतिज्ञा(Bit_मानानि_प्रयुक्तम्.आकार_2() == SQ->Entries.आकार_2());
    प्रति (अचिह्नित i = 0; i < SQ->Entries.आकार_2(); ++i) {
      यदि (!Bit_मानानि_प्रयुक्तम्[i]) {
        समुच्चयः_Error(SQ->Entries[i], "unknown bit value");
        प्रत्यागम;
      }
    }
  }
}

शून्य निवेशः::scalarString(StringRef &S, QuotingType) {
  यदि (ScalarHNode *SN = dyn_cast<ScalarHNode>(Current_ग्रन्थिः)) {
    S = SN->मानम्_2();
  } अन्यथा {
    समुच्चयः_Error(Current_ग्रन्थिः, "unexpected scalar");
  }
}

शून्य निवेशः::खण्डम्_Scalar_String(StringRef &S) { scalarString(S, QuotingType::None); }

शून्य निवेशः::scalar_चिह्नकम्(std::string &चिह्नकम्) {
  चिह्नकम् = Current_ग्रन्थिः->ग्रन्थिः_2->get_Verbatim_चिह्नकम्();
}

शून्य निवेशः::समुच्चयः_Error(HNode *hnode, ध्रुव Twine &सन्देशः_2) {
  प्रतिज्ञा(hnode && "HNode must not be NULL");
  समुच्चयः_Error(hnode->ग्रन्थिः_2, सन्देशः_2);
}

ग्रन्थिः_विधा निवेशः::get_ग्रन्थिः_विधा() {
  यदि (isa<ScalarHNode>(Current_ग्रन्थिः))
    प्रत्यागम ग्रन्थिः_विधा::Scalar;
  अन्यथा यदि (isa<MapHNode>(Current_ग्रन्थिः))
    प्रत्यागम ग्रन्थिः_विधा::Map;
  अन्यथा यदि (isa<SequenceHNode>(Current_ग्रन्थिः))
    प्रत्यागम ग्रन्थिः_विधा::Sequence;
  llvm_unreachable("Unsupported node kind");
}

शून्य निवेशः::समुच्चयः_Error(ग्रन्थिः *ग्रन्थिः_3, ध्रुव Twine &सन्देशः_2) {
  Strm->printError(ग्रन्थिः_3, सन्देशः_2);
  EC = make_error_संहिता(errc::invalid_तर्कः);
}

शून्य निवेशः::समुच्चयः_Error(ध्रुव SMRange &range, ध्रुव Twine &सन्देशः_2) {
  Strm->printError(range, सन्देशः_2);
  EC = make_error_संहिता(errc::invalid_तर्कः);
}

शून्य निवेशः::reportWarning(HNode *hnode, ध्रुव Twine &सन्देशः_2) {
  प्रतिज्ञा(hnode && "HNode must not be NULL");
  Strm->printError(hnode->ग्रन्थिः_2, सन्देशः_2, SourceMgr::DK_Warning);
}

शून्य निवेशः::reportWarning(ग्रन्थिः *ग्रन्थिः_3, ध्रुव Twine &सन्देशः_2) {
  Strm->printError(ग्रन्थिः_3, सन्देशः_2, SourceMgr::DK_Warning);
}

शून्य निवेशः::reportWarning(ध्रुव SMRange &range, ध्रुव Twine &सन्देशः_2) {
  Strm->printError(range, सन्देशः_2, SourceMgr::DK_Warning);
}

शून्य निवेशः::releaseHNodeBuffers() {
  रिक्तानि_HNode_Allocator.DestroyAll();
  ScalarHNodeAllocator.DestroyAll();
  SequenceHNodeAllocator.DestroyAll();
  MapHNodeAllocator.DestroyAll();
}

शून्य निवेशः::saveAliasHNode(ग्रन्थिः *N, HNode *HN) {
  StringRef Anchor = N->getAnchor();
  यदि (!Anchor.रिक्तानि())
    // YAML 1.2.2 - 3.2.2.2. Anchors and Aliases:
    //
    // An alias event refers to the most recent event in the serialization
    // having the specified anchor. Therefore, anchors need not be unique within
    // a serialization. In addition, an anchor need not have an alias node
    // referring to it.
    AliasMap[Anchor] = HN;
}

निवेशः::HNode *निवेशः::createHNodes(ग्रन्थिः *N) {
  SmallString<128> StringStorage;
  विकल्प (N->getType()) {
  पक्ष ग्रन्थिः::NK_Scalar: {
    Scalar_ग्रन्थिः *SN = dyn_cast<Scalar_ग्रन्थिः>(N);
    StringRef कुञ्चिका_Str = SN->get_मानम्(StringStorage);
    यदि (!StringStorage.रिक्तानि()) {
      // Copy string to permanent storage
      कुञ्चिका_Str = StringStorage.str().copy(StringAllocator);
    }
    स्वचालित *SHNode = नूतन (ScalarHNodeAllocator.Allocate()) ScalarHNode(N, कुञ्चिका_Str);
    saveAliasHNode(SN, SHNode);
    प्रत्यागम SHNode;
  }
  पक्ष ग्रन्थिः::NK_खण्डम्_Scalar: {
    खण्डम्_Scalar_ग्रन्थिः *BSN = dyn_cast<खण्डम्_Scalar_ग्रन्थिः>(N);
    StringRef मानम्_Copy = BSN->get_मानम्().copy(StringAllocator);
    स्वचालित *BSHNode =
        नूतन (ScalarHNodeAllocator.Allocate()) ScalarHNode(N, मानम्_Copy);
    saveAliasHNode(BSN, BSHNode);
    प्रत्यागम BSHNode;
  }
  पक्ष ग्रन्थिः::NK_Sequence: {
    Sequence_ग्रन्थिः *SQ = dyn_cast<Sequence_ग्रन्थिः>(N);
    स्वचालित SQHNode = नूतन (SequenceHNodeAllocator.Allocate()) SequenceHNode(N);
    प्रति (ग्रन्थिः &SN : *SQ) {
      स्वचालित प्रविष्टिः = createHNodes(&SN);
      यदि (EC)
        भङ्ग;
      SQHNode->Entries.प्रक्षिप_पृष्ठभागः(प्रविष्टिः);
    }
    saveAliasHNode(SQ, SQHNode);
    प्रत्यागम SQHNode;
  }
  पक्ष ग्रन्थिः::NK_Mapping: {
    Mapping_ग्रन्थिः *Map = dyn_cast<Mapping_ग्रन्थिः>(N);
    स्वचालित mapHNode = नूतन (MapHNodeAllocator.Allocate()) MapHNode(N);
    प्रति (कुञ्चिका_मानम्_ग्रन्थिः &KVN : *Map) {
      ग्रन्थिः *कुञ्चिका_ग्रन्थिः = KVN.get_कुञ्चिका();
      Scalar_ग्रन्थिः *कुञ्चिका = dyn_cast_if_present<Scalar_ग्रन्थिः>(कुञ्चिका_ग्रन्थिः);
      ग्रन्थिः *मानम् = KVN.get_मानम्();
      यदि (!कुञ्चिका || !मानम्) {
        यदि (!कुञ्चिका)
          समुच्चयः_Error(कुञ्चिका_ग्रन्थिः, "Map key must be a scalar");
        यदि (!मानम्)
          समुच्चयः_Error(कुञ्चिका_ग्रन्थिः, "Map value must not be empty");
        भङ्ग;
      }
      StringStorage.clear();
      StringRef कुञ्चिका_Str = कुञ्चिका->get_मानम्(StringStorage);
      यदि (!StringStorage.रिक्तानि()) {
        // Copy string to permanent storage
        कुञ्चिका_Str = StringStorage.str().copy(StringAllocator);
      }
      यदि (mapHNode->Mapping.गणनम्(कुञ्चिका_Str))
        // From YAML spec: "The content of a mapping node is an unordered set of
        // key/value node pairs, with the restriction that each of the keys is
        // unique."
        समुच्चयः_Error(कुञ्चिका_ग्रन्थिः, Twine("duplicated mapping key '") + कुञ्चिका_Str + "'");
      स्वचालित मानम्_HNode = createHNodes(मानम्);
      यदि (EC)
        भङ्ग;
      mapHNode->Mapping[कुञ्चिका_Str] =
          std::make_pair(std::move(मानम्_HNode), कुञ्चिका_ग्रन्थिः->getSourceRange());
    }
    saveAliasHNode(Map, mapHNode);
    प्रत्यागम std::move(mapHNode);
  }
  पक्ष ग्रन्थिः::NK_Null:
    // TODO: Anchor is not set for NullNode in the parser. Update the parser to
    // faithfully preserve anchors.
    प्रत्यागम नूतन (रिक्तानि_HNode_Allocator.Allocate()) रिक्तानि_HNode(N);
  पक्ष ग्रन्थिः::NK_Alias: {
    Alias_ग्रन्थिः *AN = dyn_cast<Alias_ग्रन्थिः>(N);
    स्वचालित AliasName = AN->getName();
    स्वचालित AHN = AliasMap.find(AliasName);
    यदि (AHN == AliasMap.अन्तः()) {
      समुच्चयः_Error(AN, Twine("undefined alias '" + AliasName + "'"));
      प्रत्यागम शून्यसूचकः;
    }
    प्रत्यागम AHN->second;
  }
  स्वभाव:
    समुच्चयः_Error(N, "unknown node kind");
    प्रत्यागम शून्यसूचकः;
  }
}

शून्य निवेशः::समुच्चयः_Error(ध्रुव Twine &सन्देशः_3) {
  समुच्चयः_Error(Current_ग्रन्थिः, सन्देशः_3);
}

शून्य निवेशः::समुच्चयः_Allow_Unknown_कुञ्चिकाः(तार्किक Allow) { Allow_Unknown_कुञ्चिकाः = Allow; }

तार्किक निवेशः::can_Elide_रिक्तानि_Sequence() {
  प्रत्यागम असत्य;
}

//===----------------------------------------------------------------------===//
//  Output
//===----------------------------------------------------------------------===//

Output::Output(raw_ostream &yout, शून्य *context, पूर्णाङ्क WrapColumn)
    : IO(context), बहिर्गमनम्(yout), WrapColumn(WrapColumn) {}

Output::~Output() = स्वभाव;

तार्किक Output::outputting() ध्रुव {
  प्रत्यागम सत्य;
}

शून्य Output::beginMapping() {
  State_राशिः.प्रक्षिप_पृष्ठभागः(अन्तर्_Map_First_कुञ्चिका);
  PaddingBeforeContainer = Padding;
  Padding = "\n";
}

तार्किक Output::map_चिह्नकम्(StringRef चिह्नकम्, तार्किक Use) {
  यदि (Use) {
    // If this tag is being written inside a sequence we should write the start
    // of the sequence before writing the tag, otherwise the tag won't be
    // attached to the element in the sequence, but rather the sequence itself.
    तार्किक SequenceElement = असत्य;
    यदि (State_राशिः.आकार_2() > 1) {
      स्वचालित &E = State_राशिः[State_राशिः.आकार_2() - 2];
      SequenceElement = अन्तर्_Seq_Any_Element(E) || अन्तर्_Flow_Seq_Any_Element(E);
    }
    यदि (SequenceElement && State_राशिः.पृष्ठभागः() == अन्तर्_Map_First_कुञ्चिका) {
      new_पङ्क्तिः_Check();
    } अन्यथा {
      output(" ");
    }
    output(चिह्नकम्);
    यदि (SequenceElement) {
      // If we're writing the tag during the first element of a map, the tag
      // takes the place of the first element in the sequence.
      यदि (State_राशिः.पृष्ठभागः() == अन्तर्_Map_First_कुञ्चिका) {
        State_राशिः.निष्कासय_पृष्ठभागः();
        State_राशिः.प्रक्षिप_पृष्ठभागः(अन्तर्_Map_Other_कुञ्चिका);
      }
      // Tags inside maps in sequences should act as keys in the map from a
      // formatting perspective, so we always want a newline in a sequence.
      Padding = "\n";
    }
  }
  प्रत्यागम Use;
}

शून्य Output::अन्तः_Mapping() {
  // If we did not map anything, we should explicitly emit an empty map
  यदि (State_राशिः.पृष्ठभागः() == अन्तर्_Map_First_कुञ्चिका) {
    Padding = PaddingBeforeContainer;
    new_पङ्क्तिः_Check();
    output("{}");
    Padding = "\n";
  }
  State_राशिः.निष्कासय_पृष्ठभागः();
}

std::सदिशम्<StringRef> Output::कुञ्चिकाः() {
  report_fatal_error("invalid call");
}

तार्किक Output::preflight_कुञ्चिका(StringRef कुञ्चिका, तार्किक Required, तार्किक SameAsDefault,
                          तार्किक &UseDefault, शून्य *&SaveInfo) {
  UseDefault = असत्य;
  SaveInfo = शून्यसूचकः;
  यदि (Required || !SameAsDefault || Write_Default_मानानि) {
    स्वचालित State = State_राशिः.पृष्ठभागः();
    यदि (State == अन्तर्_Flow_Map_First_कुञ्चिका || State == अन्तर्_Flow_Map_Other_कुञ्चिका) {
      flow_कुञ्चिका(कुञ्चिका);
    } अन्यथा {
      new_पङ्क्तिः_Check();
      padded_कुञ्चिका(कुञ्चिका);
    }
    प्रत्यागम सत्य;
  }
  प्रत्यागम असत्य;
}

शून्य Output::postflight_कुञ्चिका(शून्य *) {
  यदि (State_राशिः.पृष्ठभागः() == अन्तर्_Map_First_कुञ्चिका) {
    State_राशिः.निष्कासय_पृष्ठभागः();
    State_राशिः.प्रक्षिप_पृष्ठभागः(अन्तर्_Map_Other_कुञ्चिका);
  } अन्यथा यदि (State_राशिः.पृष्ठभागः() == अन्तर्_Flow_Map_First_कुञ्चिका) {
    State_राशिः.निष्कासय_पृष्ठभागः();
    State_राशिः.प्रक्षिप_पृष्ठभागः(अन्तर्_Flow_Map_Other_कुञ्चिका);
  }
}

शून्य Output::beginFlowMapping() {
  State_राशिः.प्रक्षिप_पृष्ठभागः(अन्तर्_Flow_Map_First_कुञ्चिका);
  new_पङ्क्तिः_Check();
  ColumnAtMapFlowStart = Column;
  output("{ ");
}

शून्य Output::अन्तः_Flow_Mapping() {
  State_राशिः.निष्कासय_पृष्ठभागः();
  output_ऊर्ध्वम्_गन्तव्यम्_अन्तः_सम्बन्धि_पङ्क्तिः(" }");
}

शून्य Output::beginDocuments() {
  output_ऊर्ध्वम्_गन्तव्यम्_अन्तः_सम्बन्धि_पङ्क्तिः("---");
}

तार्किक Output::preflightDocument(अचिह्नित सूचकांक) {
  यदि (सूचकांक > 0)
    output_ऊर्ध्वम्_गन्तव्यम्_अन्तः_सम्बन्धि_पङ्क्तिः("\n---");
  प्रत्यागम सत्य;
}

शून्य Output::postflightDocument() {
}

शून्य Output::अन्तः_Documents() {
  output("\n...\n");
}

अचिह्नित Output::beginSequence() {
  State_राशिः.प्रक्षिप_पृष्ठभागः(अन्तर्_Seq_First_Element);
  PaddingBeforeContainer = Padding;
  Padding = "\n";
  प्रत्यागम 0;
}

शून्य Output::अन्तः_Sequence() {
  // If we did not emit anything, we should explicitly emit an empty sequence
  यदि (State_राशिः.पृष्ठभागः() == अन्तर्_Seq_First_Element) {
    Padding = PaddingBeforeContainer;
    new_पङ्क्तिः_Check(/*EmptySequence=*/सत्य);
    output("[]");
    Padding = "\n";
  }
  State_राशिः.निष्कासय_पृष्ठभागः();
}

तार्किक Output::preflightElement(अचिह्नित, शून्य *&SaveInfo) {
  SaveInfo = शून्यसूचकः;
  प्रत्यागम सत्य;
}

शून्य Output::postflightElement(शून्य *) {
  यदि (State_राशिः.पृष्ठभागः() == अन्तर्_Seq_First_Element) {
    State_राशिः.निष्कासय_पृष्ठभागः();
    State_राशिः.प्रक्षिप_पृष्ठभागः(अन्तर्_Seq_Other_Element);
  } अन्यथा यदि (State_राशिः.पृष्ठभागः() == अन्तर्_Flow_Seq_First_Element) {
    State_राशिः.निष्कासय_पृष्ठभागः();
    State_राशिः.प्रक्षिप_पृष्ठभागः(अन्तर्_Flow_Seq_Other_Element);
  }
}

अचिह्नित Output::beginFlowSequence() {
  State_राशिः.प्रक्षिप_पृष्ठभागः(अन्तर्_Flow_Seq_First_Element);
  new_पङ्क्तिः_Check();
  ColumnAtFlowStart = Column;
  output("[ ");
  NeedFlowSequenceComma = असत्य;
  प्रत्यागम 0;
}

शून्य Output::अन्तः_Flow_Sequence() {
  State_राशिः.निष्कासय_पृष्ठभागः();
  output_ऊर्ध्वम्_गन्तव्यम्_अन्तः_सम्बन्धि_पङ्क्तिः(" ]");
}

तार्किक Output::preflightFlowElement(अचिह्नित, शून्य *&SaveInfo) {
  यदि (NeedFlowSequenceComma)
    output(", ");
  यदि (WrapColumn && Column > WrapColumn) {
    output("\n");
    प्रति (पूर्णाङ्क i = 0; i < ColumnAtFlowStart; ++i)
      output(" ");
    Column = ColumnAtFlowStart;
    output("  ");
  }
  SaveInfo = शून्यसूचकः;
  प्रत्यागम सत्य;
}

शून्य Output::postflightFlowElement(शून्य *) {
  NeedFlowSequenceComma = सत्य;
}

शून्य Output::beginEnumScalar() {
  Enumeration_मेलनम्_प्राप्तम् = असत्य;
}

तार्किक Output::मेलनम्_Enum_Scalar(StringRef Str, तार्किक मेलनम्_2) {
  यदि (मेलनम्_2 && !Enumeration_मेलनम्_प्राप्तम्) {
    new_पङ्क्तिः_Check();
    output_ऊर्ध्वम्_गन्तव्यम्_अन्तः_सम्बन्धि_पङ्क्तिः(Str);
    Enumeration_मेलनम्_प्राप्तम् = सत्य;
  }
  प्रत्यागम असत्य;
}

तार्किक Output::मेलनम्_Enum_Fallback() {
  यदि (Enumeration_मेलनम्_प्राप्तम्)
    प्रत्यागम असत्य;
  Enumeration_मेलनम्_प्राप्तम् = सत्य;
  प्रत्यागम सत्य;
}

शून्य Output::अन्तः_Enum_Scalar() {
  यदि (!Enumeration_मेलनम्_प्राप्तम्)
    llvm_unreachable("bad runtime enum value");
}

तार्किक Output::begin_Bit_समुच्चयः_Scalar(तार्किक &DoClear) {
  new_पङ्क्तिः_Check();
  output("[ ");
  Need_Bit_मानम्_Comma = असत्य;
  DoClear = असत्य;
  प्रत्यागम सत्य;
}

तार्किक Output::bit_समुच्चयः_मेलनम्(StringRef Str, तार्किक Matches) {
  यदि (Matches) {
    यदि (Need_Bit_मानम्_Comma)
      output(", ");
    output(Str);
    Need_Bit_मानम्_Comma = सत्य;
  }
  प्रत्यागम असत्य;
}

शून्य Output::अन्तः_Bit_समुच्चयः_Scalar() {
  output_ऊर्ध्वम्_गन्तव्यम्_अन्तः_सम्बन्धि_पङ्क्तिः(" ]");
}

शून्य Output::scalarString(StringRef &S, QuotingType MustQuote) {
  new_पङ्क्तिः_Check();
  यदि (S.रिक्तानि()) {
    // Print '' for the empty string because leaving the field empty is not
    // allowed.
    output_ऊर्ध्वम्_गन्तव्यम्_अन्तः_सम्बन्धि_पङ्क्तिः("''");
    प्रत्यागम;
  }
  output(S, MustQuote);
  output_ऊर्ध्वम्_गन्तव्यम्_अन्तः_सम्बन्धि_पङ्क्तिः("");
}

शून्य Output::खण्डम्_Scalar_String(StringRef &S) {
  यदि (!State_राशिः.रिक्तानि())
    new_पङ्क्तिः_Check();
  output(" |");

  अचिह्नित Indent = State_राशिः.रिक्तानि() ? 1 : State_राशिः.आकार_2();

  स्वचालित बफर = स्मृतिः_बफर::get_Mem_बफर(S, "", असत्य);
  प्रति (पङ्क्तिः_iterator पङ्क्तयः(*बफर, असत्य); !पङ्क्तयः.is_at_अन्तः(); ++पङ्क्तयः) {
    output_New_पङ्क्तिः();
    प्रति (अचिह्नित I = 0; I < Indent; ++I) {
      output("  ");
    }
    output(*पङ्क्तयः);
  }
  output_ऊर्ध्वम्_गन्तव्यम्_अन्तः_सम्बन्धि_पङ्क्तिः("");
}

शून्य Output::scalar_चिह्नकम्(std::string &चिह्नकम्) {
  यदि (चिह्नकम्.रिक्तानि())
    प्रत्यागम;
  new_पङ्क्तिः_Check();
  output(चिह्नकम्);
  output(" ");
}

शून्य Output::समुच्चयः_Error(ध्रुव Twine &सन्देशः_2) {
}

std::error_संहिता Output::error() { प्रत्यागम {}; }

तार्किक Output::can_Elide_रिक्तानि_Sequence() {
  // Normally, with an optional key/value where the value is an empty sequence,
  // the whole key/value can be not written.  But, that produces wrong yaml
  // if the key/value is the only thing in the map and the map is used in
  // a sequence.  This detects if the this sequence is the first key/value
  // in map that itself is embedded in a sequence.
  यदि (State_राशिः.आकार_2() < 2)
    प्रत्यागम सत्य;
  यदि (State_राशिः.पृष्ठभागः() != अन्तर्_Map_First_कुञ्चिका)
    प्रत्यागम सत्य;
  प्रत्यागम !अन्तर्_Seq_Any_Element(State_राशिः[State_राशिः.आकार_2() - 2]);
}

शून्य Output::output(StringRef s) {
  Column += s.आकार_2();
  बहिर्गमनम् << s;
}

शून्य Output::output(StringRef S, QuotingType MustQuote) {
  यदि (MustQuote == QuotingType::None) {
    // Only quote if we must.
    output(S);
    प्रत्यागम;
  }

  StringLiteral Quote = MustQuote == QuotingType::Single ? StringLiteral("'")
                                                         : StringLiteral("\"");
  output(Quote); // Starting quote.

  // When using double-quoted strings (and only in that case), non-printable
  // characters may be present, and will be escaped using a variety of
  // unicode-scalar and special short-form escapes. This is handled in
  // yaml::escape.
  यदि (MustQuote == QuotingType::Double) {
    output(yaml::escape(S, /* EscapePrintable= */ असत्य));
    output(Quote);
    प्रत्यागम;
  }

  अचिह्नित i = 0;
  अचिह्नित j = 0;
  अचिह्नित अन्तः_2 = S.आकार_2();
  ध्रुव वर्ण *आधारः = S.दत्त();

  // When using single-quoted strings, any single quote ' must be doubled to be
  // escaped.
  यावत् (j < अन्तः_2) {
    यदि (S[j] == '\'') {                   // Escape quotes.
      output(StringRef(&आधारः[i], j - i)); // "flush".
      output(StringLiteral("''"));        // Print it as ''
      i = j + 1;
    }
    ++j;
  }
  output(StringRef(&आधारः[i], j - i));
  output(Quote); // Ending quote.
}

शून्य Output::output_ऊर्ध्वम्_गन्तव्यम्_अन्तः_सम्बन्धि_पङ्क्तिः(StringRef s) {
  output(s);
  यदि (State_राशिः.रिक्तानि() || (!अन्तर्_Flow_Seq_Any_Element(State_राशिः.पृष्ठभागः()) &&
                             !अन्तर्_Flow_Map_Any_कुञ्चिका(State_राशिः.पृष्ठभागः())))
    Padding = "\n";
}

शून्य Output::output_New_पङ्क्तिः() {
  बहिर्गमनम् << "\n";
  Column = 0;
}

// if seq at top, indent as if map, then add "- "
// if seq in middle, use "- " if firstKey, else use "  "
//

शून्य Output::new_पङ्क्तिः_Check(तार्किक रिक्तानि_Sequence) {
  यदि (Padding != "\n") {
    output(Padding);
    Padding = {};
    प्रत्यागम;
  }
  output_New_पङ्क्तिः();
  Padding = {};

  यदि (State_राशिः.आकार_2() == 0 || रिक्तानि_Sequence)
    प्रत्यागम;

  अचिह्नित Indent = State_राशिः.आकार_2() - 1;
  तार्किक PossiblyNestedSeq = असत्य;
  स्वचालित I = State_राशिः.rbegin(), E = State_राशिः.rend();

  यदि (अन्तर्_Seq_Any_Element(*I)) {
    PossiblyNestedSeq = सत्य; // Not possibly but always.
    ++Indent;
  } अन्यथा यदि (*I == अन्तर्_Map_First_कुञ्चिका || *I == अन्तर्_Flow_Map_First_कुञ्चिका ||
             अन्तर्_Flow_Seq_Any_Element(*I)) {
    PossiblyNestedSeq = सत्य;
    ++I; // Skip back().
  }

  अचिह्नित Output_Dash_गणनम् = 0;
  यदि (PossiblyNestedSeq) {
    // Count up consecutive inSeqFirstElement from the end, unless
    // inSeqFirstElement is the top of nested sequence.
    यावत् (I != E) {
      // Don't count the top of nested sequence.
      यदि (!अन्तर्_Seq_Any_Element(*I))
        भङ्ग;

      ++Output_Dash_गणनम्;

      // Stop counting if consecutive inSeqFirstElement ends.
      यदि (*I++ != अन्तर्_Seq_First_Element)
        भङ्ग;
    }
  }

  प्रति (अचिह्नित I = Output_Dash_गणनम्; I < Indent; ++I)
    output("  ");

  प्रति (अचिह्नित I = 0; I < Output_Dash_गणनम्; ++I)
    output("- ");
}

शून्य Output::padded_कुञ्चिका(StringRef कुञ्चिका_2) {
  output(कुञ्चिका_2, needsQuotes(कुञ्चिका_2, असत्य));
  output(":");
  ध्रुव वर्ण *spaces = "                ";
  यदि (कुञ्चिका_2.आकार_2() < वाक्यदैर्घ्यम्(spaces))
    Padding = &spaces[कुञ्चिका_2.आकार_2()];
  अन्यथा
    Padding = " ";
}

शून्य Output::flow_कुञ्चिका(StringRef कुञ्चिका) {
  यदि (State_राशिः.पृष्ठभागः() == अन्तर्_Flow_Map_Other_कुञ्चिका)
    output(", ");
  यदि (WrapColumn && Column > WrapColumn) {
    output("\n");
    प्रति (पूर्णाङ्क I = 0; I < ColumnAtMapFlowStart; ++I)
      output(" ");
    Column = ColumnAtMapFlowStart;
    output("  ");
  }
  output(कुञ्चिका, needsQuotes(कुञ्चिका, असत्य));
  output(": ");
}

ग्रन्थिः_विधा Output::get_ग्रन्थिः_विधा() { report_fatal_error("invalid call"); }

तार्किक Output::अन्तर्_Seq_Any_Element(अन्तर्_State State) {
  प्रत्यागम State == अन्तर्_Seq_First_Element || State == अन्तर्_Seq_Other_Element;
}

तार्किक Output::अन्तर्_Flow_Seq_Any_Element(अन्तर्_State State) {
  प्रत्यागम State == अन्तर्_Flow_Seq_First_Element || State == अन्तर्_Flow_Seq_Other_Element;
}

तार्किक Output::अन्तर्_Map_Any_कुञ्चिका(अन्तर्_State State) {
  प्रत्यागम State == अन्तर्_Map_First_कुञ्चिका || State == अन्तर्_Map_Other_कुञ्चिका;
}

तार्किक Output::अन्तर्_Flow_Map_Any_कुञ्चिका(अन्तर्_State State) {
  प्रत्यागम State == अन्तर्_Flow_Map_First_कुञ्चिका || State == अन्तर्_Flow_Map_Other_कुञ्चिका;
}

//===----------------------------------------------------------------------===//
//  traits for built-in types
//===----------------------------------------------------------------------===//

शून्य ScalarTraits<तार्किक>::output(ध्रुव तार्किक &Val, शून्य *, raw_ostream &बहिर्गमनम्) {
  बहिर्गमनम् << (Val ? "true" : "false");
}

StringRef ScalarTraits<तार्किक>::निवेशः_2(StringRef Scalar, शून्य *, तार्किक &Val) {
  यदि (std::optional<तार्किक> Parsed = parseBool(Scalar)) {
    Val = *Parsed;
    प्रत्यागम StringRef();
  }
  प्रत्यागम "invalid boolean";
}

शून्य ScalarTraits<StringRef>::output(ध्रुव StringRef &Val, शून्य *,
                                     raw_ostream &बहिर्गमनम्) {
  बहिर्गमनम् << Val;
}

StringRef ScalarTraits<StringRef>::निवेशः_2(StringRef Scalar, शून्य *,
                                         StringRef &Val) {
  Val = Scalar;
  प्रत्यागम StringRef();
}

शून्य ScalarTraits<std::string>::output(ध्रुव std::string &Val, शून्य *,
                                       raw_ostream &बहिर्गमनम्) {
  बहिर्गमनम् << Val;
}

StringRef ScalarTraits<std::string>::निवेशः_2(StringRef Scalar, शून्य *,
                                           std::string &Val) {
  Val = Scalar.str();
  प्रत्यागम StringRef();
}

शून्य ScalarTraits<uint8_t>::output(ध्रुव uint8_t &Val, शून्य *,
                                   raw_ostream &बहिर्गमनम्) {
  // use temp uin32_t because ostream thinks uint8_t is a character
  uint32_t अङ्कः = Val;
  बहिर्गमनम् << अङ्कः;
}

StringRef ScalarTraits<uint8_t>::निवेशः_2(StringRef Scalar, शून्य *, uint8_t &Val) {
  अचिह्नित दीर्घ दीर्घ n;
  यदि (getAsUnsignedInteger(Scalar, 0, n))
    प्रत्यागम "invalid number";
  यदि (n > 0xFF)
    प्रत्यागम "out of range number";
  Val = n;
  प्रत्यागम StringRef();
}

शून्य ScalarTraits<uint16_t>::output(ध्रुव uint16_t &Val, शून्य *,
                                    raw_ostream &बहिर्गमनम्) {
  बहिर्गमनम् << Val;
}

StringRef ScalarTraits<uint16_t>::निवेशः_2(StringRef Scalar, शून्य *,
                                        uint16_t &Val) {
  अचिह्नित दीर्घ दीर्घ n;
  यदि (getAsUnsignedInteger(Scalar, 0, n))
    प्रत्यागम "invalid number";
  यदि (n > 0xFFFF)
    प्रत्यागम "out of range number";
  Val = n;
  प्रत्यागम StringRef();
}

शून्य ScalarTraits<uint32_t>::output(ध्रुव uint32_t &Val, शून्य *,
                                    raw_ostream &बहिर्गमनम्) {
  बहिर्गमनम् << Val;
}

StringRef ScalarTraits<uint32_t>::निवेशः_2(StringRef Scalar, शून्य *,
                                        uint32_t &Val) {
  अचिह्नित दीर्घ दीर्घ n;
  यदि (getAsUnsignedInteger(Scalar, 0, n))
    प्रत्यागम "invalid number";
  यदि (n > 0xFFFFFFFFUL)
    प्रत्यागम "out of range number";
  Val = n;
  प्रत्यागम StringRef();
}

शून्य ScalarTraits<uint64_t>::output(ध्रुव uint64_t &Val, शून्य *,
                                    raw_ostream &बहिर्गमनम्) {
  बहिर्गमनम् << Val;
}

StringRef ScalarTraits<uint64_t>::निवेशः_2(StringRef Scalar, शून्य *,
                                        uint64_t &Val) {
  अचिह्नित दीर्घ दीर्घ N;
  यदि (getAsUnsignedInteger(Scalar, 0, N))
    प्रत्यागम "invalid number";
  Val = N;
  प्रत्यागम StringRef();
}

शून्य ScalarTraits<int8_t>::output(ध्रुव int8_t &Val, शून्य *, raw_ostream &बहिर्गमनम्) {
  // use temp in32_t because ostream thinks int8_t is a character
  int32_t अङ्कः = Val;
  बहिर्गमनम् << अङ्कः;
}

StringRef ScalarTraits<int8_t>::निवेशः_2(StringRef Scalar, शून्य *, int8_t &Val) {
  दीर्घ दीर्घ N;
  यदि (getAsSignedInteger(Scalar, 0, N))
    प्रत्यागम "invalid number";
  यदि ((N > 127) || (N < -128))
    प्रत्यागम "out of range number";
  Val = N;
  प्रत्यागम StringRef();
}

शून्य ScalarTraits<int16_t>::output(ध्रुव int16_t &Val, शून्य *,
                                   raw_ostream &बहिर्गमनम्) {
  बहिर्गमनम् << Val;
}

StringRef ScalarTraits<int16_t>::निवेशः_2(StringRef Scalar, शून्य *, int16_t &Val) {
  दीर्घ दीर्घ N;
  यदि (getAsSignedInteger(Scalar, 0, N))
    प्रत्यागम "invalid number";
  यदि ((N > INT16_MAX) || (N < INT16_MIN))
    प्रत्यागम "out of range number";
  Val = N;
  प्रत्यागम StringRef();
}

शून्य ScalarTraits<int32_t>::output(ध्रुव int32_t &Val, शून्य *,
                                   raw_ostream &बहिर्गमनम्) {
  बहिर्गमनम् << Val;
}

StringRef ScalarTraits<int32_t>::निवेशः_2(StringRef Scalar, शून्य *, int32_t &Val) {
  दीर्घ दीर्घ N;
  यदि (getAsSignedInteger(Scalar, 0, N))
    प्रत्यागम "invalid number";
  यदि ((N > INT32_MAX) || (N < INT32_MIN))
    प्रत्यागम "out of range number";
  Val = N;
  प्रत्यागम StringRef();
}

शून्य ScalarTraits<int64_t>::output(ध्रुव int64_t &Val, शून्य *,
                                   raw_ostream &बहिर्गमनम्) {
  बहिर्गमनम् << Val;
}

StringRef ScalarTraits<int64_t>::निवेशः_2(StringRef Scalar, शून्य *, int64_t &Val) {
  दीर्घ दीर्घ N;
  यदि (getAsSignedInteger(Scalar, 0, N))
    प्रत्यागम "invalid number";
  Val = N;
  प्रत्यागम StringRef();
}

शून्य ScalarTraits<द्विगुण>::output(ध्रुव द्विगुण &Val, शून्य *, raw_ostream &बहिर्गमनम्) {
  बहिर्गमनम् << format("%g", Val);
}

StringRef ScalarTraits<द्विगुण>::निवेशः_2(StringRef Scalar, शून्य *, द्विगुण &Val) {
  यदि (गन्तव्यम्_float(Scalar, Val))
    प्रत्यागम StringRef();
  प्रत्यागम "invalid floating point number";
}

शून्य ScalarTraits<भिन्न>::output(ध्रुव भिन्न &Val, शून्य *, raw_ostream &बहिर्गमनम्) {
  बहिर्गमनम् << format("%g", Val);
}

StringRef ScalarTraits<भिन्न>::निवेशः_2(StringRef Scalar, शून्य *, भिन्न &Val) {
  यदि (गन्तव्यम्_float(Scalar, Val))
    प्रत्यागम StringRef();
  प्रत्यागम "invalid floating point number";
}

शून्य ScalarTraits<Hex8>::output(ध्रुव Hex8 &Val, शून्य *, raw_ostream &बहिर्गमनम्) {
  बहिर्गमनम् << format("0x%" PRIX8, (uint8_t)Val);
}

StringRef ScalarTraits<Hex8>::निवेशः_2(StringRef Scalar, शून्य *, Hex8 &Val) {
  अचिह्नित दीर्घ दीर्घ n;
  यदि (getAsUnsignedInteger(Scalar, 0, n))
    प्रत्यागम "invalid hex8 number";
  यदि (n > 0xFF)
    प्रत्यागम "out of range hex8 number";
  Val = n;
  प्रत्यागम StringRef();
}

शून्य ScalarTraits<Hex16>::output(ध्रुव Hex16 &Val, शून्य *, raw_ostream &बहिर्गमनम्) {
  बहिर्गमनम् << format("0x%" PRIX16, (uint16_t)Val);
}

StringRef ScalarTraits<Hex16>::निवेशः_2(StringRef Scalar, शून्य *, Hex16 &Val) {
  अचिह्नित दीर्घ दीर्घ n;
  यदि (getAsUnsignedInteger(Scalar, 0, n))
    प्रत्यागम "invalid hex16 number";
  यदि (n > 0xFFFF)
    प्रत्यागम "out of range hex16 number";
  Val = n;
  प्रत्यागम StringRef();
}

शून्य ScalarTraits<Hex32>::output(ध्रुव Hex32 &Val, शून्य *, raw_ostream &बहिर्गमनम्) {
  बहिर्गमनम् << format("0x%" PRIX32, (uint32_t)Val);
}

StringRef ScalarTraits<Hex32>::निवेशः_2(StringRef Scalar, शून्य *, Hex32 &Val) {
  अचिह्नित दीर्घ दीर्घ n;
  यदि (getAsUnsignedInteger(Scalar, 0, n))
    प्रत्यागम "invalid hex32 number";
  यदि (n > 0xFFFFFFFFUL)
    प्रत्यागम "out of range hex32 number";
  Val = n;
  प्रत्यागम StringRef();
}

शून्य ScalarTraits<Hex64>::output(ध्रुव Hex64 &Val, शून्य *, raw_ostream &बहिर्गमनम्) {
  बहिर्गमनम् << format("0x%" PRIX64, (uint64_t)Val);
}

StringRef ScalarTraits<Hex64>::निवेशः_2(StringRef Scalar, शून्य *, Hex64 &Val) {
  अचिह्नित दीर्घ दीर्घ अङ्कः;
  यदि (getAsUnsignedInteger(Scalar, 0, अङ्कः))
    प्रत्यागम "invalid hex64 number";
  Val = अङ्कः;
  प्रत्यागम StringRef();
}

शून्य ScalarTraits<संस्करणम्_Tuple>::output(ध्रुव संस्करणम्_Tuple &Val, शून्य *,
                                        llvm::raw_ostream &बहिर्गमनम्) {
  बहिर्गमनम् << Val.getAsString();
}

StringRef ScalarTraits<संस्करणम्_Tuple>::निवेशः_2(StringRef Scalar, शून्य *,
                                            संस्करणम्_Tuple &Val) {
  यदि (Val.tryParse(Scalar))
    प्रत्यागम "invalid version format";
  प्रत्यागम StringRef();
}
