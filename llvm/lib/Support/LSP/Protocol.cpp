//===--- Protocol.cpp - Language Server Protocol Implementation -----------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file contains the serialization code for the LSP structs.
//
//===----------------------------------------------------------------------===//

#ضمن "llvm/Support/LSP/Protocol.h"
#ضمن "llvm/ADT/StringExtras.h"
#ضمن "llvm/ADT/StringSet.h"
#ضمن "llvm/Support/ErrorHandling.h"
#ضمن "llvm/Support/JSON.h"
#ضمن "llvm/Support/MemoryBuffer.h"
#ضمن "llvm/Support/Path.h"
#ضمن "llvm/Support/raw_ostream.h"

استفاده نطاق llvm;
استفاده نطاق llvm::lsp;

// Helper that doesn't treat `null` and absent fields as failures.
قالب <نام_نوع T>
ساكن منطقي mapOptOrNull(ثابت llvm::json::مقدار &Params,
                         llvm::StringLiteral Prop, T &بیرون,
                         llvm::json::مسیر مسیر) {
  ثابت llvm::json::Object *O = Params.getAsObject();
  أكد(O);

  // Field is missing or null.
  خودکار *V = O->get(Prop);
  إذا (!V || V->getAsNull())
    أرجع درست;
  أرجع fromJSON(*V, بیرون, مسیر.فیلد(Prop));
}

//===----------------------------------------------------------------------===//
// LSPError
//===----------------------------------------------------------------------===//

حرف LSPError::ID;

//===----------------------------------------------------------------------===//
// URIForFile
//===----------------------------------------------------------------------===//

ساكن منطقي is_Windows_مسیر(StringRef مسیر) {
  أرجع مسیر.اندازه_2() > 1 && llvm::isAlpha(مسیر[0]) && مسیر[1] == ':';
}

ساكن منطقي is_شبکه_مسیر(StringRef مسیر) {
  أرجع مسیر.اندازه_2() > 2 && مسیر[0] == مسیر[1] &&
         llvm::sys::مسیر_2::is_separator(مسیر[0]);
}

ساكن منطقي should_Escape_درون_URI(بیعلامت حرف C) {
  // Unreserved characters.
  إذا ((C >= 'a' && C <= 'z') || (C >= 'A' && C <= 'Z') ||
      (C >= '0' && C <= '9'))
    أرجع خطأ;

  بدل (C) {
  حالة '-':
  حالة '_':
  حالة '.':
  حالة '~':
  // '/' is only reserved when parsing.
  حالة '/':
  // ':' is only reserved for relative URI paths, which we doesn't produce.
  حالة ':':
    أرجع خطأ;
  }
  أرجع درست;
}

/// Encodes a string according to percent-encoding.
/// - Unreserved characters are not escaped.
/// - Reserved characters always escaped with exceptions like '/'.
/// - All other characters are escaped.
ساكن فراغ percentEncode(StringRef Content, std::string &بیرون) {
  لكل (بیعلامت حرف C : Content) {
    إذا (should_Escape_درون_URI(C)) {
      بیرون.افزودن_پشت('%');
      بیرون.افزودن_پشت(llvm::hexdigit(C / 16));
      بیرون.افزودن_پشت(llvm::hexdigit(C % 16));
    } وإلا {
      بیرون.افزودن_پشت(C);
    }
  }
}

/// Decodes a string according to percent-encoding.
ساكن std::string percentDecode(StringRef Content) {
  std::string نتیجه;
  لكل (خودکار I = Content.begin(), E = Content.انتها(); I != E; ++I) {
    إذا (*I == '%' && I + 2 < Content.انتها() && llvm::isHexDigit(*(I + 1)) &&
        llvm::isHexDigit(*(I + 2))) {
      نتیجه.افزودن_پشت(llvm::hexFromNibbles(*(I + 1), *(I + 2)));
      I += 2;
    } وإلا {
      نتیجه.افزودن_پشت(*I);
    }
  }
  أرجع نتیجه;
}

/// Return the set containing the supported URI schemes.
ساكن String_مجموعه<> &getSupportedSchemes() {
  ساكن String_مجموعه<> Schemes({"file", "test"});
  أرجع Schemes;
}

/// Returns true if the given scheme is structurally valid, i.e. it does not
/// contain any invalid scheme characters. This does not check that the scheme
/// is actually supported.
ساكن منطقي isStructurallyValidScheme(StringRef Scheme) {
  إذا (Scheme.خالی())
    أرجع خطأ;
  إذا (!llvm::isAlpha(Scheme[0]))
    أرجع خطأ;
  أرجع llvm::all_از(llvm::drop_begin(Scheme), [](حرف C) {
    أرجع llvm::isAlnum(C) || C == '+' || C == '.' || C == '-';
  });
}

ساكن llvm::Expected<std::string> uri_From_Absolute_مسیر(StringRef Absolute_مسیر,
                                                       StringRef Scheme) {
  std::string Body;
  StringRef Authority;
  StringRef ریشه_2 = llvm::sys::مسیر_2::ریشه_name(Absolute_مسیر);
  إذا (is_شبکه_مسیر(ریشه_2)) {
    // Windows UNC paths e.g. \\server\share => file://server/share
    Authority = ریشه_2.drop_front(2);
    Absolute_مسیر.consume_front(ریشه_2);
  } وإلا إذا (is_Windows_مسیر(ریشه_2)) {
    // Windows paths e.g. X:\path => file:///X:/path
    Body = "/";
  }
  Body += llvm::sys::مسیر_2::convert_مقصد_slash(Absolute_مسیر);

  std::string Uri = Scheme.str() + ":";
  إذا (Authority.خالی() && Body.خالی())
    أرجع Uri;

  // If authority if empty, we only print body if it starts with "/"; otherwise,
  // the URI is invalid.
  إذا (!Authority.خالی() || StringRef(Body).starts_with("/")) {
    Uri.append("//");
    percentEncode(Authority, Uri);
  }
  percentEncode(Body, Uri);
  أرجع Uri;
}

ساكن llvm::Expected<std::string> get_Absolute_مسیر(StringRef Authority,
                                                   StringRef Body) {
  إذا (!Body.starts_with("/"))
    أرجع llvm::createStringError(
        llvm::inconvertible_Error_کد(),
        "File scheme: expect body to be an absolute path starting "
        "with '/': " +
            Body);
  SmallString<128> مسیر;
  إذا (!Authority.خالی()) {
    // Windows UNC paths e.g. file://server/share => \\server\share
    ("//" + Authority).مقصد_وکتور(مسیر);
  } وإلا إذا (is_Windows_مسیر(Body.substr(1))) {
    // Windows paths e.g. file:///X:/path => X:\path
    Body.consume_front("/");
  }
  مسیر.append(Body);
  llvm::sys::مسیر_2::native(مسیر);
  أرجع std::string(مسیر);
}

ساكن llvm::Expected<std::string> parse_File_مسیر_From_URI(StringRef OrigUri) {
  StringRef Uri = OrigUri;

  // Decode the scheme of the URI.
  size_t Pos = Uri.پیدا_کن(':');
  إذا (Pos == StringRef::npos)
    أرجع llvm::createStringError(llvm::inconvertible_Error_کد(),
                                   "Scheme must be provided in URI: " +
                                       OrigUri);
  StringRef SchemeStr = Uri.substr(0, Pos);
  std::string UriScheme = percentDecode(SchemeStr);
  إذا (!isStructurallyValidScheme(UriScheme))
    أرجع llvm::createStringError(llvm::inconvertible_Error_کد(),
                                   "Invalid scheme: " + SchemeStr +
                                       " (decoded: " + UriScheme + ")");
  Uri = Uri.substr(Pos + 1);

  // Decode the authority of the URI.
  std::string UriAuthority;
  إذا (Uri.consume_front("//")) {
    Pos = Uri.پیدا_کن('/');
    UriAuthority = percentDecode(Uri.substr(0, Pos));
    Uri = Uri.substr(Pos);
  }

  // Decode the body of the URI.
  std::string UriBody = percentDecode(Uri);

  // Compute the absolute path for this uri.
  إذا (!getSupportedSchemes().شامل_است(UriScheme)) {
    أرجع llvm::createStringError(llvm::inconvertible_Error_کد(),
                                   "unsupported URI scheme `" + UriScheme +
                                       "' for workspace files");
  }
  أرجع get_Absolute_مسیر(UriAuthority, UriBody);
}

llvm::Expected<URIForFile> URIForFile::fromURI(StringRef Uri) {
  llvm::Expected<std::string> File_مسیر = parse_File_مسیر_From_URI(Uri);
  إذا (!File_مسیر)
    أرجع File_مسیر.takeError();
  أرجع URIForFile(std::move(*File_مسیر), Uri.str());
}

llvm::Expected<URIForFile> URIForFile::fromFile(StringRef AbsoluteFilepath,
                                                StringRef Scheme) {
  llvm::Expected<std::string> Uri =
      uri_From_Absolute_مسیر(AbsoluteFilepath, Scheme);
  إذا (!Uri)
    أرجع Uri.takeError();
  أرجع fromURI(*Uri);
}

StringRef URIForFile::scheme() ثابت { أرجع uri().تقسیم(':').اولی_2; }

فراغ URIForFile::ثبات_Supported_Scheme(StringRef Scheme) {
  getSupportedSchemes().درج(Scheme);
}

منطقي llvm::lsp::fromJSON(ثابت llvm::json::مقدار &مقدار, URIForFile &نتیجه,
                         llvm::json::مسیر مسیر) {
  إذا (std::optional<StringRef> Str = مقدار.getAsString()) {
    llvm::Expected<URIForFile> ExpectedUri = URIForFile::fromURI(*Str);
    إذا (!ExpectedUri) {
      مسیر.report("unresolvable URI");
      consumeError(ExpectedUri.takeError());
      أرجع خطأ;
    }
    نتیجه = std::move(*ExpectedUri);
    أرجع درست;
  }
  أرجع خطأ;
}

llvm::json::مقدار llvm::lsp::مقصد_JSON(ثابت URIForFile &مقدار) {
  أرجع مقدار.uri();
}

raw_ostream &llvm::lsp::عملگر<<(raw_ostream &Os, ثابت URIForFile &مقدار) {
  أرجع Os << مقدار.uri();
}

//===----------------------------------------------------------------------===//
// ClientCapabilities
//===----------------------------------------------------------------------===//

منطقي llvm::lsp::fromJSON(ثابت llvm::json::مقدار &مقدار,
                         ClientCapabilities &نتیجه, llvm::json::مسیر مسیر) {
  ثابت llvm::json::Object *O = مقدار.getAsObject();
  إذا (!O) {
    مسیر.report("expected object");
    أرجع خطأ;
  }
  إذا (ثابت llvm::json::Object *متن_Document = O->getObject("textDocument")) {
    إذا (ثابت llvm::json::Object *Document_نماد =
            متن_Document->getObject("documentSymbol")) {
      إذا (std::optional<منطقي> سلسلهمراتبی_Support =
              Document_نماد->getBoolean("hierarchicalDocumentSymbolSupport"))
        نتیجه.سلسلهمراتبی_Document_نماد = *سلسلهمراتبی_Support;
    }
    إذا (خودکار *کد_Action = متن_Document->getObject("codeAction")) {
      إذا (کد_Action->getObject("codeActionLiteralSupport"))
        نتیجه.کد_Action_Structure = درست;
    }
  }
  إذا (خودکار *پنجره = O->getObject("window")) {
    إذا (std::optional<منطقي> Work_انجامشده_پیشرفت_Support =
            پنجره->getBoolean("workDoneProgress"))
      نتیجه.work_انجامشده_پیشرفت = *Work_انجامشده_پیشرفت_Support;
  }
  أرجع درست;
}

//===----------------------------------------------------------------------===//
// ClientInfo
//===----------------------------------------------------------------------===//

منطقي llvm::lsp::fromJSON(ثابت llvm::json::مقدار &مقدار, ClientInfo &نتیجه,
                         llvm::json::مسیر مسیر) {
  llvm::json::ObjectMapper O(مقدار, مسیر);
  إذا (!O || !O.map("name", نتیجه.name))
    أرجع خطأ;

  // Don't fail if we can't parse version.
  O.map("version", نتیجه.نسخه_2);
  أرجع درست;
}

//===----------------------------------------------------------------------===//
// InitializeParams
//===----------------------------------------------------------------------===//

منطقي llvm::lsp::fromJSON(ثابت llvm::json::مقدار &مقدار, TraceLevel &نتیجه,
                         llvm::json::مسیر مسیر) {
  إذا (std::optional<StringRef> Str = مقدار.getAsString()) {
    إذا (*Str == "off") {
      نتیجه = TraceLevel::Off;
      أرجع درست;
    }
    إذا (*Str == "messages") {
      نتیجه = TraceLevel::Messages;
      أرجع درست;
    }
    إذا (*Str == "verbose") {
      نتیجه = TraceLevel::Verbose;
      أرجع درست;
    }
  }
  أرجع خطأ;
}

منطقي llvm::lsp::fromJSON(ثابت llvm::json::مقدار &مقدار,
                         InitializeParams &نتیجه, llvm::json::مسیر مسیر) {
  llvm::json::ObjectMapper O(مقدار, مسیر);
  إذا (!O)
    أرجع خطأ;
  // We deliberately don't fail if we can't parse individual fields.
  O.map("capabilities", نتیجه.capabilities);
  O.map("trace", نتیجه.trace);
  O.map("rootUri", نتیجه.ریشه_Uri);
  O.map("rootPath", نتیجه.ریشه_مسیر);
  mapOptOrNull(مقدار, "clientInfo", نتیجه.clientInfo, مسیر);

  أرجع درست;
}

//===----------------------------------------------------------------------===//
// TextDocumentItem
//===----------------------------------------------------------------------===//

منطقي llvm::lsp::fromJSON(ثابت llvm::json::مقدار &مقدار,
                         متن_Document_قلم &نتیجه, llvm::json::مسیر مسیر) {
  llvm::json::ObjectMapper O(مقدار, مسیر);
  أرجع O && O.map("uri", نتیجه.uri) &&
         O.map("languageId", نتیجه.languageId) && O.map("text", نتیجه.متن) &&
         O.map("version", نتیجه.نسخه_2);
}

//===----------------------------------------------------------------------===//
// TextDocumentIdentifier
//===----------------------------------------------------------------------===//

llvm::json::مقدار llvm::lsp::مقصد_JSON(ثابت متن_Document_Identifier &مقدار) {
  أرجع llvm::json::Object{{"uri", مقدار.uri}};
}

منطقي llvm::lsp::fromJSON(ثابت llvm::json::مقدار &مقدار,
                         متن_Document_Identifier &نتیجه,
                         llvm::json::مسیر مسیر) {
  llvm::json::ObjectMapper O(مقدار, مسیر);
  أرجع O && O.map("uri", نتیجه.uri);
}

//===----------------------------------------------------------------------===//
// VersionedTextDocumentIdentifier
//===----------------------------------------------------------------------===//

llvm::json::مقدار
llvm::lsp::مقصد_JSON(ثابت Versioned_متن_Document_Identifier &مقدار) {
  أرجع llvm::json::Object{
      {"uri", مقدار.uri},
      {"version", مقدار.نسخه_2},
  };
}

منطقي llvm::lsp::fromJSON(ثابت llvm::json::مقدار &مقدار,
                         Versioned_متن_Document_Identifier &نتیجه,
                         llvm::json::مسیر مسیر) {
  llvm::json::ObjectMapper O(مقدار, مسیر);
  أرجع O && O.map("uri", نتیجه.uri) && O.map("version", نتیجه.نسخه_2);
}

//===----------------------------------------------------------------------===//
// Position
//===----------------------------------------------------------------------===//

منطقي llvm::lsp::fromJSON(ثابت llvm::json::مقدار &مقدار, Position &نتیجه,
                         llvm::json::مسیر مسیر) {
  llvm::json::ObjectMapper O(مقدار, مسیر);
  أرجع O && O.map("line", نتیجه.سطر_2) &&
         O.map("character", نتیجه.character);
}

llvm::json::مقدار llvm::lsp::مقصد_JSON(ثابت Position &مقدار) {
  أرجع llvm::json::Object{
      {"line", مقدار.سطر_2},
      {"character", مقدار.character},
  };
}

raw_ostream &llvm::lsp::عملگر<<(raw_ostream &Os, ثابت Position &مقدار) {
  أرجع Os << مقدار.سطر_2 << ':' << مقدار.character;
}

//===----------------------------------------------------------------------===//
// Range
//===----------------------------------------------------------------------===//

منطقي llvm::lsp::fromJSON(ثابت llvm::json::مقدار &مقدار, Range &نتیجه,
                         llvm::json::مسیر مسیر) {
  llvm::json::ObjectMapper O(مقدار, مسیر);
  أرجع O && O.map("start", نتیجه.شروع_2) && O.map("end", نتیجه.انتها);
}

llvm::json::مقدار llvm::lsp::مقصد_JSON(ثابت Range &مقدار) {
  أرجع llvm::json::Object{
      {"start", مقدار.شروع_2},
      {"end", مقدار.انتها},
  };
}

raw_ostream &llvm::lsp::عملگر<<(raw_ostream &Os, ثابت Range &مقدار) {
  أرجع Os << مقدار.شروع_2 << '-' << مقدار.انتها;
}

//===----------------------------------------------------------------------===//
// Location
//===----------------------------------------------------------------------===//

منطقي llvm::lsp::fromJSON(ثابت llvm::json::مقدار &مقدار, Location &نتیجه,
                         llvm::json::مسیر مسیر) {
  llvm::json::ObjectMapper O(مقدار, مسیر);
  أرجع O && O.map("uri", نتیجه.uri) && O.map("range", نتیجه.range);
}

llvm::json::مقدار llvm::lsp::مقصد_JSON(ثابت Location &مقدار) {
  أرجع llvm::json::Object{
      {"uri", مقدار.uri},
      {"range", مقدار.range},
  };
}

raw_ostream &llvm::lsp::عملگر<<(raw_ostream &Os, ثابت Location &مقدار) {
  أرجع Os << مقدار.range << '@' << مقدار.uri;
}

//===----------------------------------------------------------------------===//
// TextDocumentPositionParams
//===----------------------------------------------------------------------===//

منطقي llvm::lsp::fromJSON(ثابت llvm::json::مقدار &مقدار,
                         متن_Document_Position_Params &نتیجه,
                         llvm::json::مسیر مسیر) {
  llvm::json::ObjectMapper O(مقدار, مسیر);
  أرجع O && O.map("textDocument", نتیجه.متن_Document_2) &&
         O.map("position", نتیجه.position);
}

//===----------------------------------------------------------------------===//
// ReferenceParams
//===----------------------------------------------------------------------===//

منطقي llvm::lsp::fromJSON(ثابت llvm::json::مقدار &مقدار,
                         ReferenceContext &نتیجه, llvm::json::مسیر مسیر) {
  llvm::json::ObjectMapper O(مقدار, مسیر);
  أرجع O && O.mapOptional("includeDeclaration", نتیجه.includeDeclaration);
}

منطقي llvm::lsp::fromJSON(ثابت llvm::json::مقدار &مقدار,
                         ReferenceParams &نتیجه, llvm::json::مسیر مسیر) {
  متن_Document_Position_Params &پایه = نتیجه;
  llvm::json::ObjectMapper O(مقدار, مسیر);
  أرجع fromJSON(مقدار, پایه, مسیر) && O &&
         O.mapOptional("context", نتیجه.context);
}

//===----------------------------------------------------------------------===//
// DidOpenTextDocumentParams
//===----------------------------------------------------------------------===//

منطقي llvm::lsp::fromJSON(ثابت llvm::json::مقدار &مقدار,
                         Did_Open_متن_Document_Params &نتیجه,
                         llvm::json::مسیر مسیر) {
  llvm::json::ObjectMapper O(مقدار, مسیر);
  أرجع O && O.map("textDocument", نتیجه.متن_Document_2);
}

//===----------------------------------------------------------------------===//
// DidCloseTextDocumentParams
//===----------------------------------------------------------------------===//

منطقي llvm::lsp::fromJSON(ثابت llvm::json::مقدار &مقدار,
                         Did_Close_متن_Document_Params &نتیجه,
                         llvm::json::مسیر مسیر) {
  llvm::json::ObjectMapper O(مقدار, مسیر);
  أرجع O && O.map("textDocument", نتیجه.متن_Document_2);
}

//===----------------------------------------------------------------------===//
// DidSaveTextDocumentParams
//===----------------------------------------------------------------------===//

منطقي llvm::lsp::fromJSON(ثابت llvm::json::مقدار &Params,
                         Did_Save_متن_Document_Params &R, llvm::json::مسیر P) {
  llvm::json::ObjectMapper O(Params, P);
  أرجع O && O.map("textDocument", R.متن_Document_2);
}

//===----------------------------------------------------------------------===//
// DidChangeTextDocumentParams
//===----------------------------------------------------------------------===//

Logical_نتیجه
متن_Document_Content_Change_Event::apply_مقصد(std::string &Contents) ثابت {
  // If there is no range, the full document changed.
  إذا (!range) {
    Contents = متن;
    أرجع success();
  }

  // Try to map the replacement range to the content.
  llvm::منبع_Mgr TmpScrMgr;
  TmpScrMgr.جمع_کن_New_منبع_میانگیر(llvm::حافظه_میانگیر::get_Mem_میانگیر(Contents),
                               SMLoc());
  SMRange RangeLoc = range->getAsSMRange(TmpScrMgr);
  إذا (!RangeLoc.isValid())
    أرجع failure();

  Contents.replace(RangeLoc.شروع.get_اشاره_گر() - Contents.داده(),
                   RangeLoc.انتها_2.get_اشاره_گر() - RangeLoc.شروع.get_اشاره_گر(),
                   متن);
  أرجع success();
}

Logical_نتیجه متن_Document_Content_Change_Event::apply_مقصد(
    ArrayRef<متن_Document_Content_Change_Event> Changes, std::string &Contents) {
  لكل (ثابت خودکار &Change : Changes)
    إذا (failed(Change.apply_مقصد(Contents)))
      أرجع failure();
  أرجع success();
}

منطقي llvm::lsp::fromJSON(ثابت llvm::json::مقدار &مقدار,
                         متن_Document_Content_Change_Event &نتیجه,
                         llvm::json::مسیر مسیر) {
  llvm::json::ObjectMapper O(مقدار, مسیر);
  أرجع O && O.map("range", نتیجه.range) &&
         O.map("rangeLength", نتیجه.range_درازی) && O.map("text", نتیجه.متن);
}

منطقي llvm::lsp::fromJSON(ثابت llvm::json::مقدار &مقدار,
                         Did_Change_متن_Document_Params &نتیجه,
                         llvm::json::مسیر مسیر) {
  llvm::json::ObjectMapper O(مقدار, مسیر);
  أرجع O && O.map("textDocument", نتیجه.متن_Document_2) &&
         O.map("contentChanges", نتیجه.contentChanges);
}

//===----------------------------------------------------------------------===//
// MarkupContent
//===----------------------------------------------------------------------===//

ساكن llvm::StringRef مقصد_متن_نوع(Markup_نوع نوع_3) {
  بدل (نوع_3) {
  حالة Markup_نوع::Plain_متن:
    أرجع "plaintext";
  حالة Markup_نوع::Markdown:
    أرجع "markdown";
  }
  llvm_unreachable("Invalid MarkupKind");
}

raw_ostream &llvm::lsp::عملگر<<(raw_ostream &Os, Markup_نوع نوع_3) {
  أرجع Os << مقصد_متن_نوع(نوع_3);
}

llvm::json::مقدار llvm::lsp::مقصد_JSON(ثابت MarkupContent &Mc) {
  إذا (Mc.مقدار_2.خالی())
    أرجع اشاره_تهی;

  أرجع llvm::json::Object{
      {"kind", مقصد_متن_نوع(Mc.نوع_2)},
      {"value", Mc.مقدار_2},
  };
}

//===----------------------------------------------------------------------===//
// Hover
//===----------------------------------------------------------------------===//

llvm::json::مقدار llvm::lsp::مقصد_JSON(ثابت Hover &Hover) {
  llvm::json::Object نتیجه{{"contents", مقصد_JSON(Hover.contents)}};
  إذا (Hover.range)
    نتیجه["range"] = مقصد_JSON(*Hover.range);
  أرجع std::move(نتیجه);
}

//===----------------------------------------------------------------------===//
// DocumentSymbol
//===----------------------------------------------------------------------===//

llvm::json::مقدار llvm::lsp::مقصد_JSON(ثابت Document_نماد &نماد) {
  llvm::json::Object نتیجه{{"name", نماد.name},
                            {"kind", static_cast<صحيح>(نماد.نوع_2)},
                            {"range", نماد.range},
                            {"selectionRange", نماد.selectionRange}};

  إذا (!نماد.detail.خالی())
    نتیجه["detail"] = نماد.detail;
  إذا (!نماد.children.خالی())
    نتیجه["children"] = نماد.children;
  أرجع std::move(نتیجه);
}

//===----------------------------------------------------------------------===//
// DocumentSymbolParams
//===----------------------------------------------------------------------===//

منطقي llvm::lsp::fromJSON(ثابت llvm::json::مقدار &مقدار,
                         Document_نماد_Params &نتیجه, llvm::json::مسیر مسیر) {
  llvm::json::ObjectMapper O(مقدار, مسیر);
  أرجع O && O.map("textDocument", نتیجه.متن_Document_2);
}

//===----------------------------------------------------------------------===//
// DiagnosticRelatedInformation
//===----------------------------------------------------------------------===//

منطقي llvm::lsp::fromJSON(ثابت llvm::json::مقدار &مقدار,
                         DiagnosticRelatedInformation &نتیجه,
                         llvm::json::مسیر مسیر) {
  llvm::json::ObjectMapper O(مقدار, مسیر);
  أرجع O && O.map("location", نتیجه.location) &&
         O.map("message", نتیجه.پیام_2);
}

llvm::json::مقدار llvm::lsp::مقصد_JSON(ثابت DiagnosticRelatedInformation &Info) {
  أرجع llvm::json::Object{
      {"location", Info.location},
      {"message", Info.پیام_2},
  };
}

//===----------------------------------------------------------------------===//
// Diagnostic
//===----------------------------------------------------------------------===//

llvm::json::مقدار llvm::lsp::مقصد_JSON(Diagnostic_برچسب برچسب) {
  أرجع static_cast<صحيح>(برچسب);
}

منطقي llvm::lsp::fromJSON(ثابت llvm::json::مقدار &مقدار, Diagnostic_برچسب &نتیجه,
                         llvm::json::مسیر مسیر) {
  إذا (std::optional<int64_t> I = مقدار.getAsInteger()) {
    نتیجه = (Diagnostic_برچسب)*I;
    أرجع درست;
  }

  أرجع خطأ;
}

llvm::json::مقدار llvm::lsp::مقصد_JSON(ثابت Diagnostic &Diag) {
  llvm::json::Object نتیجه{
      {"range", Diag.range},
      {"severity", (صحيح)Diag.severity},
      {"message", Diag.پیام_2},
  };
  إذا (Diag.category)
    نتیجه["category"] = *Diag.category;
  إذا (!Diag.منبع_2.خالی())
    نتیجه["source"] = Diag.منبع_2;
  إذا (Diag.relatedInformation)
    نتیجه["relatedInformation"] = *Diag.relatedInformation;
  إذا (!Diag.برچسبها.خالی())
    نتیجه["tags"] = Diag.برچسبها;
  أرجع std::move(نتیجه);
}

منطقي llvm::lsp::fromJSON(ثابت llvm::json::مقدار &مقدار, Diagnostic &نتیجه,
                         llvm::json::مسیر مسیر) {
  llvm::json::ObjectMapper O(مقدار, مسیر);
  إذا (!O)
    أرجع خطأ;
  صحيح Severity = 0;
  إذا (!mapOptOrNull(مقدار, "severity", Severity, مسیر))
    أرجع خطأ;
  نتیجه.severity = (DiagnosticSeverity)Severity;

  أرجع O.map("range", نتیجه.range) && O.map("message", نتیجه.پیام_2) &&
         mapOptOrNull(مقدار, "category", نتیجه.category, مسیر) &&
         mapOptOrNull(مقدار, "source", نتیجه.منبع_2, مسیر) &&
         mapOptOrNull(مقدار, "relatedInformation", نتیجه.relatedInformation,
                      مسیر) &&
         mapOptOrNull(مقدار, "tags", نتیجه.برچسبها, مسیر);
}

//===----------------------------------------------------------------------===//
// PublishDiagnosticsParams
//===----------------------------------------------------------------------===//

llvm::json::مقدار llvm::lsp::مقصد_JSON(ثابت PublishDiagnosticsParams &Params) {
  أرجع llvm::json::Object{
      {"uri", Params.uri},
      {"diagnostics", Params.diagnostics},
      {"version", Params.نسخه_2},
  };
}

//===----------------------------------------------------------------------===//
// TextEdit
//===----------------------------------------------------------------------===//

منطقي llvm::lsp::fromJSON(ثابت llvm::json::مقدار &مقدار, متن_Edit &نتیجه,
                         llvm::json::مسیر مسیر) {
  llvm::json::ObjectMapper O(مقدار, مسیر);
  أرجع O && O.map("range", نتیجه.range) && O.map("newText", نتیجه.new_متن);
}

llvm::json::مقدار llvm::lsp::مقصد_JSON(ثابت متن_Edit &مقدار) {
  أرجع llvm::json::Object{
      {"range", مقدار.range},
      {"newText", مقدار.new_متن},
  };
}

raw_ostream &llvm::lsp::عملگر<<(raw_ostream &Os, ثابت متن_Edit &مقدار) {
  Os << مقدار.range << " => \"";
  llvm::printEscapedString(مقدار.new_متن, Os);
  أرجع Os << '"';
}

//===----------------------------------------------------------------------===//
// CompletionItemKind
//===----------------------------------------------------------------------===//

منطقي llvm::lsp::fromJSON(ثابت llvm::json::مقدار &مقدار,
                         Completion_قلم_نوع &نتیجه, llvm::json::مسیر مسیر) {
  إذا (std::optional<int64_t> Int_مقدار = مقدار.getAsInteger()) {
    إذا (*Int_مقدار < static_cast<صحيح>(Completion_قلم_نوع::متن_2) ||
        *Int_مقدار > static_cast<صحيح>(Completion_قلم_نوع::TypeParameter))
      أرجع خطأ;
    نتیجه = static_cast<Completion_قلم_نوع>(*Int_مقدار);
    أرجع درست;
  }
  أرجع خطأ;
}

Completion_قلم_نوع llvm::lsp::adjust_نوع_مقصد_Capability(
    Completion_قلم_نوع نوع_3,
    Completion_قلم_نوع_Bitset &Supported_Completion_قلم_Kinds) {
  size_t نوع_Val = static_cast<size_t>(نوع_3);
  إذا (نوع_Val >= k_Completion_قلم_نوع_Min &&
      نوع_Val <= Supported_Completion_قلم_Kinds.اندازه_2() &&
      Supported_Completion_قلم_Kinds[نوع_Val])
    أرجع نوع_3;

  // Provide some fall backs for common kinds that are close enough.
  بدل (نوع_3) {
  حالة Completion_قلم_نوع::Folder:
    أرجع Completion_قلم_نوع::File;
  حالة Completion_قلم_نوع::EnumMember:
    أرجع Completion_قلم_نوع::Enum;
  حالة Completion_قلم_نوع::Struct:
    أرجع Completion_قلم_نوع::Class;
  افتراضي:
    أرجع Completion_قلم_نوع::متن_2;
  }
}

منطقي llvm::lsp::fromJSON(ثابت llvm::json::مقدار &مقدار,
                         Completion_قلم_نوع_Bitset &نتیجه,
                         llvm::json::مسیر مسیر) {
  إذا (ثابت llvm::json::Array *Array_مقدار = مقدار.getAsArray()) {
    لكل (size_t I = 0, E = Array_مقدار->اندازه_2(); I < E; ++I) {
      Completion_قلم_نوع نوع_بیرون;
      إذا (fromJSON((*Array_مقدار)[I], نوع_بیرون, مسیر.شاخص(I)))
        نتیجه.مجموعه(size_t(نوع_بیرون));
    }
    أرجع درست;
  }
  أرجع خطأ;
}

//===----------------------------------------------------------------------===//
// CompletionItem
//===----------------------------------------------------------------------===//

llvm::json::مقدار llvm::lsp::مقصد_JSON(ثابت Completion_قلم &مقدار) {
  أكد(!مقدار.label.خالی() && "completion item label is required");
  llvm::json::Object نتیجه{{"label", مقدار.label}};
  إذا (مقدار.نوع_2 != Completion_قلم_نوع::Missing)
    نتیجه["kind"] = static_cast<صحيح>(مقدار.نوع_2);
  إذا (!مقدار.detail.خالی())
    نتیجه["detail"] = مقدار.detail;
  إذا (مقدار.documentation)
    نتیجه["documentation"] = مقدار.documentation;
  إذا (!مقدار.مرتب_سازی_متن.خالی())
    نتیجه["sortText"] = مقدار.مرتب_سازی_متن;
  إذا (!مقدار.filter_متن.خالی())
    نتیجه["filterText"] = مقدار.filter_متن;
  إذا (!مقدار.درج_متن.خالی())
    نتیجه["insertText"] = مقدار.درج_متن;
  إذا (مقدار.درج_متن_Format != درج_متن_Format_2::Missing)
    نتیجه["insertTextFormat"] = static_cast<صحيح>(مقدار.درج_متن_Format);
  إذا (مقدار.متن_Edit_2)
    نتیجه["textEdit"] = *مقدار.متن_Edit_2;
  إذا (!مقدار.additional_متن_Edits.خالی()) {
    نتیجه["additionalTextEdits"] =
        llvm::json::Array(مقدار.additional_متن_Edits);
  }
  إذا (مقدار.deprecated)
    نتیجه["deprecated"] = مقدار.deprecated;
  أرجع std::move(نتیجه);
}

raw_ostream &llvm::lsp::عملگر<<(raw_ostream &Os,
                                   ثابت Completion_قلم &مقدار) {
  أرجع Os << مقدار.label << " - " << مقصد_JSON(مقدار);
}

منطقي llvm::lsp::عملگر<(ثابت Completion_قلم &Lhs,
                          ثابت Completion_قلم &Rhs) {
  أرجع (Lhs.مرتب_سازی_متن.خالی() ? Lhs.label : Lhs.مرتب_سازی_متن) <
         (Rhs.مرتب_سازی_متن.خالی() ? Rhs.label : Rhs.مرتب_سازی_متن);
}

//===----------------------------------------------------------------------===//
// CompletionList
//===----------------------------------------------------------------------===//

llvm::json::مقدار llvm::lsp::مقصد_JSON(ثابت CompletionList &مقدار) {
  أرجع llvm::json::Object{
      {"isIncomplete", مقدار.isIncomplete},
      {"items", llvm::json::Array(مقدار.اقلام)},
  };
}

//===----------------------------------------------------------------------===//
// CompletionContext
//===----------------------------------------------------------------------===//

منطقي llvm::lsp::fromJSON(ثابت llvm::json::مقدار &مقدار,
                         CompletionContext &نتیجه, llvm::json::مسیر مسیر) {
  llvm::json::ObjectMapper O(مقدار, مسیر);
  صحيح Trigger_نوع;
  إذا (!O || !O.map("triggerKind", Trigger_نوع) ||
      !mapOptOrNull(مقدار, "triggerCharacter", نتیجه.triggerCharacter, مسیر))
    أرجع خطأ;
  نتیجه.trigger_نوع = static_cast<Completion_Trigger_نوع>(Trigger_نوع);
  أرجع درست;
}

//===----------------------------------------------------------------------===//
// CompletionParams
//===----------------------------------------------------------------------===//

منطقي llvm::lsp::fromJSON(ثابت llvm::json::مقدار &مقدار,
                         CompletionParams &نتیجه, llvm::json::مسیر مسیر) {
  إذا (!fromJSON(مقدار, static_cast<متن_Document_Position_Params &>(نتیجه), مسیر))
    أرجع خطأ;
  إذا (ثابت llvm::json::مقدار *Context = مقدار.getAsObject()->get("context"))
    أرجع fromJSON(*Context, نتیجه.context, مسیر.فیلد("context"));
  أرجع درست;
}

//===----------------------------------------------------------------------===//
// ParameterInformation
//===----------------------------------------------------------------------===//

llvm::json::مقدار llvm::lsp::مقصد_JSON(ثابت ParameterInformation &مقدار) {
  أكد((مقدار.labelOffsets || !مقدار.labelString.خالی()) &&
         "parameter information label is required");
  llvm::json::Object نتیجه;
  إذا (مقدار.labelOffsets)
    نتیجه["label"] = llvm::json::Array(
        {مقدار.labelOffsets->اولی_2, مقدار.labelOffsets->دومی_2});
  وإلا
    نتیجه["label"] = مقدار.labelString;
  إذا (!مقدار.documentation.خالی())
    نتیجه["documentation"] = مقدار.documentation;
  أرجع std::move(نتیجه);
}

//===----------------------------------------------------------------------===//
// SignatureInformation
//===----------------------------------------------------------------------===//

llvm::json::مقدار llvm::lsp::مقصد_JSON(ثابت SignatureInformation &مقدار) {
  أكد(!مقدار.label.خالی() && "signature information label is required");
  llvm::json::Object نتیجه{
      {"label", مقدار.label},
      {"parameters", llvm::json::Array(مقدار.parameters)},
  };
  إذا (!مقدار.documentation.خالی())
    نتیجه["documentation"] = مقدار.documentation;
  أرجع std::move(نتیجه);
}

raw_ostream &llvm::lsp::عملگر<<(raw_ostream &Os,
                                   ثابت SignatureInformation &مقدار) {
  أرجع Os << مقدار.label << " - " << مقصد_JSON(مقدار);
}

//===----------------------------------------------------------------------===//
// SignatureHelp
//===----------------------------------------------------------------------===//

llvm::json::مقدار llvm::lsp::مقصد_JSON(ثابت SignatureHelp &مقدار) {
  أكد(مقدار.فعال_Signature >= 0 &&
         "Unexpected negative value for number of active signatures.");
  أكد(مقدار.فعال_Parameter >= 0 &&
         "Unexpected negative value for active parameter index");
  أرجع llvm::json::Object{
      {"activeSignature", مقدار.فعال_Signature},
      {"activeParameter", مقدار.فعال_Parameter},
      {"signatures", llvm::json::Array(مقدار.signatures)},
  };
}

//===----------------------------------------------------------------------===//
// DocumentLinkParams
//===----------------------------------------------------------------------===//

منطقي llvm::lsp::fromJSON(ثابت llvm::json::مقدار &مقدار,
                         DocumentLinkParams &نتیجه, llvm::json::مسیر مسیر) {
  llvm::json::ObjectMapper O(مقدار, مسیر);
  أرجع O && O.map("textDocument", نتیجه.متن_Document_2);
}

//===----------------------------------------------------------------------===//
// DocumentLink
//===----------------------------------------------------------------------===//

llvm::json::مقدار llvm::lsp::مقصد_JSON(ثابت DocumentLink &مقدار) {
  أرجع llvm::json::Object{
      {"range", مقدار.range},
      {"target", مقدار.هدف},
  };
}

//===----------------------------------------------------------------------===//
// InlayHintsParams
//===----------------------------------------------------------------------===//

منطقي llvm::lsp::fromJSON(ثابت llvm::json::مقدار &مقدار,
                         InlayHintsParams &نتیجه, llvm::json::مسیر مسیر) {
  llvm::json::ObjectMapper O(مقدار, مسیر);
  أرجع O && O.map("textDocument", نتیجه.متن_Document_2) &&
         O.map("range", نتیجه.range);
}

//===----------------------------------------------------------------------===//
// InlayHint
//===----------------------------------------------------------------------===//

llvm::json::مقدار llvm::lsp::مقصد_JSON(ثابت InlayHint &مقدار) {
  أرجع llvm::json::Object{{"position", مقدار.position},
                            {"kind", (صحيح)مقدار.نوع_2},
                            {"label", مقدار.label},
                            {"paddingLeft", مقدار.padding_چپ},
                            {"paddingRight", مقدار.padding_راست}};
}
منطقي llvm::lsp::عملگر==(ثابت InlayHint &Lhs, ثابت InlayHint &Rhs) {
  أرجع std::tie(Lhs.position, Lhs.نوع_2, Lhs.label) ==
         std::tie(Rhs.position, Rhs.نوع_2, Rhs.label);
}
منطقي llvm::lsp::عملگر<(ثابت InlayHint &Lhs, ثابت InlayHint &Rhs) {
  أرجع std::tie(Lhs.position, Lhs.نوع_2, Lhs.label) <
         std::tie(Rhs.position, Rhs.نوع_2, Rhs.label);
}

llvm::raw_ostream &llvm::lsp::عملگر<<(llvm::raw_ostream &Os,
                                         Inlay_Hint_نوع مقدار) {
  بدل (مقدار) {
  حالة Inlay_Hint_نوع::Parameter:
    أرجع Os << "parameter";
  حالة Inlay_Hint_نوع::Type:
    أرجع Os << "type";
  }
  llvm_unreachable("Unknown InlayHintKind");
}

//===----------------------------------------------------------------------===//
// CodeActionContext
//===----------------------------------------------------------------------===//

منطقي llvm::lsp::fromJSON(ثابت llvm::json::مقدار &مقدار,
                         کد_Action_Context &نتیجه, llvm::json::مسیر مسیر) {
  llvm::json::ObjectMapper O(مقدار, مسیر);
  إذا (!O || !O.map("diagnostics", نتیجه.diagnostics))
    أرجع خطأ;
  O.map("only", نتیجه.only);
  أرجع درست;
}

//===----------------------------------------------------------------------===//
// CodeActionParams
//===----------------------------------------------------------------------===//

منطقي llvm::lsp::fromJSON(ثابت llvm::json::مقدار &مقدار,
                         کد_Action_Params &نتیجه, llvm::json::مسیر مسیر) {
  llvm::json::ObjectMapper O(مقدار, مسیر);
  أرجع O && O.map("textDocument", نتیجه.متن_Document_2) &&
         O.map("range", نتیجه.range) && O.map("context", نتیجه.context);
}

//===----------------------------------------------------------------------===//
// WorkspaceEdit
//===----------------------------------------------------------------------===//

منطقي llvm::lsp::fromJSON(ثابت llvm::json::مقدار &مقدار, WorkspaceEdit &نتیجه,
                         llvm::json::مسیر مسیر) {
  llvm::json::ObjectMapper O(مقدار, مسیر);
  أرجع O && O.map("changes", نتیجه.changes);
}

llvm::json::مقدار llvm::lsp::مقصد_JSON(ثابت WorkspaceEdit &مقدار) {
  llvm::json::Object FileChanges;
  لكل (خودکار &Change : مقدار.changes)
    FileChanges[Change.اولی_2] = llvm::json::Array(Change.دومی_2);
  أرجع llvm::json::Object{{"changes", std::move(FileChanges)}};
}

//===----------------------------------------------------------------------===//
// CodeAction
//===----------------------------------------------------------------------===//

ثابت llvm::StringLiteral کد_Action::k_سریع_Fix = "quickfix";
ثابت llvm::StringLiteral کد_Action::kRefactor = "refactor";
ثابت llvm::StringLiteral کد_Action::kInfo = "info";

llvm::json::مقدار llvm::lsp::مقصد_JSON(ثابت کد_Action &مقدار) {
  llvm::json::Object کد_Action{{"title", مقدار.title}};
  إذا (مقدار.نوع_2)
    کد_Action["kind"] = *مقدار.نوع_2;
  إذا (مقدار.diagnostics)
    کد_Action["diagnostics"] = llvm::json::Array(*مقدار.diagnostics);
  إذا (مقدار.isPreferred)
    کد_Action["isPreferred"] = درست;
  إذا (مقدار.edit)
    کد_Action["edit"] = *مقدار.edit;
  أرجع std::move(کد_Action);
}

//===----------------------------------------------------------------------===//
// ShowMessageParams
//===----------------------------------------------------------------------===//

llvm::json::مقدار llvm::lsp::مقصد_JSON(ثابت Show_پیام_Params &Params) {
  خودکار بیرون = llvm::json::Object{
      {"type", static_cast<صحيح>(Params.type)},
      {"message", Params.پیام_2},
  };
  إذا (Params.actions)
    بیرون["actions"] = *Params.actions;
  أرجع بیرون;
}

llvm::json::مقدار llvm::lsp::مقصد_JSON(ثابت پیام_Action_قلم &Params) {
  أرجع llvm::json::Object{{"title", Params.title}};
}
