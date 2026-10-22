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

استعمال نطاق llvm;
استعمال نطاق llvm::lsp;

// Helper that doesn't treat `null` and absent fields as failures.
سانچو <قسم_نالو T>
ساكن منطقي mapOptOrNull(ثابت llvm::json::قدر &Params,
                         llvm::StringLiteral Prop, T &ٻاهريون,
                         llvm::json::رستو رستو) {
  ثابت llvm::json::Object *O = Params.getAsObject();
  أكد(O);

  // Field is missing or null.
  خودڪار *V = O->get(Prop);
  إذا (!V || V->getAsNull())
    أرجع سچ;
  أرجع fromJSON(*V, ٻاهريون, رستو.ميدان(Prop));
}

//===----------------------------------------------------------------------===//
// LSPError
//===----------------------------------------------------------------------===//

حرف LSPError::ID;

//===----------------------------------------------------------------------===//
// URIForFile
//===----------------------------------------------------------------------===//

ساكن منطقي is_Windows_رستو(StringRef رستو) {
  أرجع رستو.سائيز_2() > 1 && llvm::isAlpha(رستو[0]) && رستو[1] == ':';
}

ساكن منطقي is_ڄار_رستو(StringRef رستو) {
  أرجع رستو.سائيز_2() > 2 && رستو[0] == رستو[1] &&
         llvm::sys::رستو_2::is_separator(رستو[0]);
}

ساكن منطقي should_Escape_اندريون_URI(بنانشان حرف C) {
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
  أرجع سچ;
}

/// Encodes a string according to percent-encoding.
/// - Unreserved characters are not escaped.
/// - Reserved characters always escaped with exceptions like '/'.
/// - All other characters are escaped.
ساكن فراغ percentEncode(StringRef Content, std::string &ٻاهريون) {
  لكل (بنانشان حرف C : Content) {
    إذا (should_Escape_اندريون_URI(C)) {
      ٻاهريون.ڌڪو_پٺ('%');
      ٻاهريون.ڌڪو_پٺ(llvm::hexdigit(C / 16));
      ٻاهريون.ڌڪو_پٺ(llvm::hexdigit(C % 16));
    } وإلا {
      ٻاهريون.ڌڪو_پٺ(C);
    }
  }
}

/// Decodes a string according to percent-encoding.
ساكن std::string percentDecode(StringRef Content) {
  std::string نتيجو;
  لكل (خودڪار I = Content.begin(), E = Content.پڇاڙي(); I != E; ++I) {
    إذا (*I == '%' && I + 2 < Content.پڇاڙي() && llvm::isHexDigit(*(I + 1)) &&
        llvm::isHexDigit(*(I + 2))) {
      نتيجو.ڌڪو_پٺ(llvm::hexFromNibbles(*(I + 1), *(I + 2)));
      I += 2;
    } وإلا {
      نتيجو.ڌڪو_پٺ(*I);
    }
  }
  أرجع نتيجو;
}

/// Return the set containing the supported URI schemes.
ساكن String_سيٽ<> &getSupportedSchemes() {
  ساكن String_سيٽ<> Schemes({"file", "test"});
  أرجع Schemes;
}

/// Returns true if the given scheme is structurally valid, i.e. it does not
/// contain any invalid scheme characters. This does not check that the scheme
/// is actually supported.
ساكن منطقي isStructurallyValidScheme(StringRef Scheme) {
  إذا (Scheme.خالي_جايون())
    أرجع خطأ;
  إذا (!llvm::isAlpha(Scheme[0]))
    أرجع خطأ;
  أرجع llvm::all_جو(llvm::drop_begin(Scheme), [](حرف C) {
    أرجع llvm::isAlnum(C) || C == '+' || C == '.' || C == '-';
  });
}

ساكن llvm::Expected<std::string> uri_From_Absolute_رستو(StringRef Absolute_رستو,
                                                       StringRef Scheme) {
  std::string Body;
  StringRef Authority;
  StringRef پاڙ_2 = llvm::sys::رستو_2::پاڙ_name(Absolute_رستو);
  إذا (is_ڄار_رستو(پاڙ_2)) {
    // Windows UNC paths e.g. \\server\share => file://server/share
    Authority = پاڙ_2.drop_front(2);
    Absolute_رستو.consume_front(پاڙ_2);
  } وإلا إذا (is_Windows_رستو(پاڙ_2)) {
    // Windows paths e.g. X:\path => file:///X:/path
    Body = "/";
  }
  Body += llvm::sys::رستو_2::convert_منزل_slash(Absolute_رستو);

  std::string Uri = Scheme.str() + ":";
  إذا (Authority.خالي_جايون() && Body.خالي_جايون())
    أرجع Uri;

  // If authority if empty, we only print body if it starts with "/"; otherwise,
  // the URI is invalid.
  إذا (!Authority.خالي_جايون() || StringRef(Body).starts_with("/")) {
    Uri.append("//");
    percentEncode(Authority, Uri);
  }
  percentEncode(Body, Uri);
  أرجع Uri;
}

ساكن llvm::Expected<std::string> get_Absolute_رستو(StringRef Authority,
                                                   StringRef Body) {
  إذا (!Body.starts_with("/"))
    أرجع llvm::createStringError(
        llvm::inconvertible_Error_ڪوڊ(),
        "File scheme: expect body to be an absolute path starting "
        "with '/': " +
            Body);
  SmallString<128> رستو;
  إذا (!Authority.خالي_جايون()) {
    // Windows UNC paths e.g. file://server/share => \\server\share
    ("//" + Authority).منزل_ويڪٽر(رستو);
  } وإلا إذا (is_Windows_رستو(Body.substr(1))) {
    // Windows paths e.g. file:///X:/path => X:\path
    Body.consume_front("/");
  }
  رستو.append(Body);
  llvm::sys::رستو_2::native(رستو);
  أرجع std::string(رستو);
}

ساكن llvm::Expected<std::string> parse_File_رستو_From_URI(StringRef OrigUri) {
  StringRef Uri = OrigUri;

  // Decode the scheme of the URI.
  size_t Pos = Uri.ڳوليو(':');
  إذا (Pos == StringRef::npos)
    أرجع llvm::createStringError(llvm::inconvertible_Error_ڪوڊ(),
                                   "Scheme must be provided in URI: " +
                                       OrigUri);
  StringRef SchemeStr = Uri.substr(0, Pos);
  std::string UriScheme = percentDecode(SchemeStr);
  إذا (!isStructurallyValidScheme(UriScheme))
    أرجع llvm::createStringError(llvm::inconvertible_Error_ڪوڊ(),
                                   "Invalid scheme: " + SchemeStr +
                                       " (decoded: " + UriScheme + ")");
  Uri = Uri.substr(Pos + 1);

  // Decode the authority of the URI.
  std::string UriAuthority;
  إذا (Uri.consume_front("//")) {
    Pos = Uri.ڳوليو('/');
    UriAuthority = percentDecode(Uri.substr(0, Pos));
    Uri = Uri.substr(Pos);
  }

  // Decode the body of the URI.
  std::string UriBody = percentDecode(Uri);

  // Compute the absolute path for this uri.
  إذا (!getSupportedSchemes().شامل_آهي(UriScheme)) {
    أرجع llvm::createStringError(llvm::inconvertible_Error_ڪوڊ(),
                                   "unsupported URI scheme `" + UriScheme +
                                       "' for workspace files");
  }
  أرجع get_Absolute_رستو(UriAuthority, UriBody);
}

llvm::Expected<URIForFile> URIForFile::fromURI(StringRef Uri) {
  llvm::Expected<std::string> File_رستو = parse_File_رستو_From_URI(Uri);
  إذا (!File_رستو)
    أرجع File_رستو.takeError();
  أرجع URIForFile(std::move(*File_رستو), Uri.str());
}

llvm::Expected<URIForFile> URIForFile::fromFile(StringRef AbsoluteFilepath,
                                                StringRef Scheme) {
  llvm::Expected<std::string> Uri =
      uri_From_Absolute_رستو(AbsoluteFilepath, Scheme);
  إذا (!Uri)
    أرجع Uri.takeError();
  أرجع fromURI(*Uri);
}

StringRef URIForFile::scheme() ثابت { أرجع uri().ورهايو(':').پهريون_2; }

فراغ URIForFile::رجسٽر_Supported_Scheme(StringRef Scheme) {
  getSupportedSchemes().وجهو(Scheme);
}

منطقي llvm::lsp::fromJSON(ثابت llvm::json::قدر &قدر, URIForFile &نتيجو,
                         llvm::json::رستو رستو) {
  إذا (std::optional<StringRef> Str = قدر.getAsString()) {
    llvm::Expected<URIForFile> ExpectedUri = URIForFile::fromURI(*Str);
    إذا (!ExpectedUri) {
      رستو.report("unresolvable URI");
      consumeError(ExpectedUri.takeError());
      أرجع خطأ;
    }
    نتيجو = std::move(*ExpectedUri);
    أرجع سچ;
  }
  أرجع خطأ;
}

llvm::json::قدر llvm::lsp::منزل_JSON(ثابت URIForFile &قدر) {
  أرجع قدر.uri();
}

raw_ostream &llvm::lsp::عامل<<(raw_ostream &Os, ثابت URIForFile &قدر) {
  أرجع Os << قدر.uri();
}

//===----------------------------------------------------------------------===//
// ClientCapabilities
//===----------------------------------------------------------------------===//

منطقي llvm::lsp::fromJSON(ثابت llvm::json::قدر &قدر,
                         ClientCapabilities &نتيجو, llvm::json::رستو رستو) {
  ثابت llvm::json::Object *O = قدر.getAsObject();
  إذا (!O) {
    رستو.report("expected object");
    أرجع خطأ;
  }
  إذا (ثابت llvm::json::Object *متن_Document = O->getObject("textDocument")) {
    إذا (ثابت llvm::json::Object *Document_نشان =
            متن_Document->getObject("documentSymbol")) {
      إذا (std::optional<منطقي> درجيوار_Support =
              Document_نشان->getBoolean("hierarchicalDocumentSymbolSupport"))
        نتيجو.درجيوار_Document_نشان = *درجيوار_Support;
    }
    إذا (خودڪار *ڪوڊ_Action = متن_Document->getObject("codeAction")) {
      إذا (ڪوڊ_Action->getObject("codeActionLiteralSupport"))
        نتيجو.ڪوڊ_Action_Structure = سچ;
    }
  }
  إذا (خودڪار *دري = O->getObject("window")) {
    إذا (std::optional<منطقي> Work_پورو_ترقي_Support =
            دري->getBoolean("workDoneProgress"))
      نتيجو.work_پورو_ترقي = *Work_پورو_ترقي_Support;
  }
  أرجع سچ;
}

//===----------------------------------------------------------------------===//
// ClientInfo
//===----------------------------------------------------------------------===//

منطقي llvm::lsp::fromJSON(ثابت llvm::json::قدر &قدر, ClientInfo &نتيجو,
                         llvm::json::رستو رستو) {
  llvm::json::ObjectMapper O(قدر, رستو);
  إذا (!O || !O.map("name", نتيجو.name))
    أرجع خطأ;

  // Don't fail if we can't parse version.
  O.map("version", نتيجو.نسخو_2);
  أرجع سچ;
}

//===----------------------------------------------------------------------===//
// InitializeParams
//===----------------------------------------------------------------------===//

منطقي llvm::lsp::fromJSON(ثابت llvm::json::قدر &قدر, TraceLevel &نتيجو,
                         llvm::json::رستو رستو) {
  إذا (std::optional<StringRef> Str = قدر.getAsString()) {
    إذا (*Str == "off") {
      نتيجو = TraceLevel::Off;
      أرجع سچ;
    }
    إذا (*Str == "messages") {
      نتيجو = TraceLevel::Messages;
      أرجع سچ;
    }
    إذا (*Str == "verbose") {
      نتيجو = TraceLevel::Verbose;
      أرجع سچ;
    }
  }
  أرجع خطأ;
}

منطقي llvm::lsp::fromJSON(ثابت llvm::json::قدر &قدر,
                         InitializeParams &نتيجو, llvm::json::رستو رستو) {
  llvm::json::ObjectMapper O(قدر, رستو);
  إذا (!O)
    أرجع خطأ;
  // We deliberately don't fail if we can't parse individual fields.
  O.map("capabilities", نتيجو.capabilities);
  O.map("trace", نتيجو.trace);
  O.map("rootUri", نتيجو.پاڙ_Uri);
  O.map("rootPath", نتيجو.پاڙ_رستو);
  mapOptOrNull(قدر, "clientInfo", نتيجو.clientInfo, رستو);

  أرجع سچ;
}

//===----------------------------------------------------------------------===//
// TextDocumentItem
//===----------------------------------------------------------------------===//

منطقي llvm::lsp::fromJSON(ثابت llvm::json::قدر &قدر,
                         متن_Document_شي &نتيجو, llvm::json::رستو رستو) {
  llvm::json::ObjectMapper O(قدر, رستو);
  أرجع O && O.map("uri", نتيجو.uri) &&
         O.map("languageId", نتيجو.languageId) && O.map("text", نتيجو.متن) &&
         O.map("version", نتيجو.نسخو_2);
}

//===----------------------------------------------------------------------===//
// TextDocumentIdentifier
//===----------------------------------------------------------------------===//

llvm::json::قدر llvm::lsp::منزل_JSON(ثابت متن_Document_Identifier &قدر) {
  أرجع llvm::json::Object{{"uri", قدر.uri}};
}

منطقي llvm::lsp::fromJSON(ثابت llvm::json::قدر &قدر,
                         متن_Document_Identifier &نتيجو,
                         llvm::json::رستو رستو) {
  llvm::json::ObjectMapper O(قدر, رستو);
  أرجع O && O.map("uri", نتيجو.uri);
}

//===----------------------------------------------------------------------===//
// VersionedTextDocumentIdentifier
//===----------------------------------------------------------------------===//

llvm::json::قدر
llvm::lsp::منزل_JSON(ثابت Versioned_متن_Document_Identifier &قدر) {
  أرجع llvm::json::Object{
      {"uri", قدر.uri},
      {"version", قدر.نسخو_2},
  };
}

منطقي llvm::lsp::fromJSON(ثابت llvm::json::قدر &قدر,
                         Versioned_متن_Document_Identifier &نتيجو,
                         llvm::json::رستو رستو) {
  llvm::json::ObjectMapper O(قدر, رستو);
  أرجع O && O.map("uri", نتيجو.uri) && O.map("version", نتيجو.نسخو_2);
}

//===----------------------------------------------------------------------===//
// Position
//===----------------------------------------------------------------------===//

منطقي llvm::lsp::fromJSON(ثابت llvm::json::قدر &قدر, Position &نتيجو,
                         llvm::json::رستو رستو) {
  llvm::json::ObjectMapper O(قدر, رستو);
  أرجع O && O.map("line", نتيجو.سٽ_2) &&
         O.map("character", نتيجو.character);
}

llvm::json::قدر llvm::lsp::منزل_JSON(ثابت Position &قدر) {
  أرجع llvm::json::Object{
      {"line", قدر.سٽ_2},
      {"character", قدر.character},
  };
}

raw_ostream &llvm::lsp::عامل<<(raw_ostream &Os, ثابت Position &قدر) {
  أرجع Os << قدر.سٽ_2 << ':' << قدر.character;
}

//===----------------------------------------------------------------------===//
// Range
//===----------------------------------------------------------------------===//

منطقي llvm::lsp::fromJSON(ثابت llvm::json::قدر &قدر, Range &نتيجو,
                         llvm::json::رستو رستو) {
  llvm::json::ObjectMapper O(قدر, رستو);
  أرجع O && O.map("start", نتيجو.شروعات_2) && O.map("end", نتيجو.پڇاڙي);
}

llvm::json::قدر llvm::lsp::منزل_JSON(ثابت Range &قدر) {
  أرجع llvm::json::Object{
      {"start", قدر.شروعات_2},
      {"end", قدر.پڇاڙي},
  };
}

raw_ostream &llvm::lsp::عامل<<(raw_ostream &Os, ثابت Range &قدر) {
  أرجع Os << قدر.شروعات_2 << '-' << قدر.پڇاڙي;
}

//===----------------------------------------------------------------------===//
// Location
//===----------------------------------------------------------------------===//

منطقي llvm::lsp::fromJSON(ثابت llvm::json::قدر &قدر, Location &نتيجو,
                         llvm::json::رستو رستو) {
  llvm::json::ObjectMapper O(قدر, رستو);
  أرجع O && O.map("uri", نتيجو.uri) && O.map("range", نتيجو.range);
}

llvm::json::قدر llvm::lsp::منزل_JSON(ثابت Location &قدر) {
  أرجع llvm::json::Object{
      {"uri", قدر.uri},
      {"range", قدر.range},
  };
}

raw_ostream &llvm::lsp::عامل<<(raw_ostream &Os, ثابت Location &قدر) {
  أرجع Os << قدر.range << '@' << قدر.uri;
}

//===----------------------------------------------------------------------===//
// TextDocumentPositionParams
//===----------------------------------------------------------------------===//

منطقي llvm::lsp::fromJSON(ثابت llvm::json::قدر &قدر,
                         متن_Document_Position_Params &نتيجو,
                         llvm::json::رستو رستو) {
  llvm::json::ObjectMapper O(قدر, رستو);
  أرجع O && O.map("textDocument", نتيجو.متن_Document_2) &&
         O.map("position", نتيجو.position);
}

//===----------------------------------------------------------------------===//
// ReferenceParams
//===----------------------------------------------------------------------===//

منطقي llvm::lsp::fromJSON(ثابت llvm::json::قدر &قدر,
                         ReferenceContext &نتيجو, llvm::json::رستو رستو) {
  llvm::json::ObjectMapper O(قدر, رستو);
  أرجع O && O.mapOptional("includeDeclaration", نتيجو.includeDeclaration);
}

منطقي llvm::lsp::fromJSON(ثابت llvm::json::قدر &قدر,
                         ReferenceParams &نتيجو, llvm::json::رستو رستو) {
  متن_Document_Position_Params &بنياد = نتيجو;
  llvm::json::ObjectMapper O(قدر, رستو);
  أرجع fromJSON(قدر, بنياد, رستو) && O &&
         O.mapOptional("context", نتيجو.context);
}

//===----------------------------------------------------------------------===//
// DidOpenTextDocumentParams
//===----------------------------------------------------------------------===//

منطقي llvm::lsp::fromJSON(ثابت llvm::json::قدر &قدر,
                         Did_Open_متن_Document_Params &نتيجو,
                         llvm::json::رستو رستو) {
  llvm::json::ObjectMapper O(قدر, رستو);
  أرجع O && O.map("textDocument", نتيجو.متن_Document_2);
}

//===----------------------------------------------------------------------===//
// DidCloseTextDocumentParams
//===----------------------------------------------------------------------===//

منطقي llvm::lsp::fromJSON(ثابت llvm::json::قدر &قدر,
                         Did_Close_متن_Document_Params &نتيجو,
                         llvm::json::رستو رستو) {
  llvm::json::ObjectMapper O(قدر, رستو);
  أرجع O && O.map("textDocument", نتيجو.متن_Document_2);
}

//===----------------------------------------------------------------------===//
// DidSaveTextDocumentParams
//===----------------------------------------------------------------------===//

منطقي llvm::lsp::fromJSON(ثابت llvm::json::قدر &Params,
                         Did_Save_متن_Document_Params &R, llvm::json::رستو P) {
  llvm::json::ObjectMapper O(Params, P);
  أرجع O && O.map("textDocument", R.متن_Document_2);
}

//===----------------------------------------------------------------------===//
// DidChangeTextDocumentParams
//===----------------------------------------------------------------------===//

Logical_نتيجو
متن_Document_Content_Change_Event::apply_منزل(std::string &Contents) ثابت {
  // If there is no range, the full document changed.
  إذا (!range) {
    Contents = متن;
    أرجع success();
  }

  // Try to map the replacement range to the content.
  llvm::ذريعو_Mgr TmpScrMgr;
  TmpScrMgr.جوڙيو_New_ذريعو_بفر(llvm::يادگيري_بفر::get_Mem_بفر(Contents),
                               SMLoc());
  SMRange RangeLoc = range->getAsSMRange(TmpScrMgr);
  إذا (!RangeLoc.isValid())
    أرجع failure();

  Contents.replace(RangeLoc.شروعات.get_اشارو() - Contents.معلومات(),
                   RangeLoc.پڇاڙي_2.get_اشارو() - RangeLoc.شروعات.get_اشارو(),
                   متن);
  أرجع success();
}

Logical_نتيجو متن_Document_Content_Change_Event::apply_منزل(
    ArrayRef<متن_Document_Content_Change_Event> Changes, std::string &Contents) {
  لكل (ثابت خودڪار &Change : Changes)
    إذا (failed(Change.apply_منزل(Contents)))
      أرجع failure();
  أرجع success();
}

منطقي llvm::lsp::fromJSON(ثابت llvm::json::قدر &قدر,
                         متن_Document_Content_Change_Event &نتيجو,
                         llvm::json::رستو رستو) {
  llvm::json::ObjectMapper O(قدر, رستو);
  أرجع O && O.map("range", نتيجو.range) &&
         O.map("rangeLength", نتيجو.range_ڊيگهائي) && O.map("text", نتيجو.متن);
}

منطقي llvm::lsp::fromJSON(ثابت llvm::json::قدر &قدر,
                         Did_Change_متن_Document_Params &نتيجو,
                         llvm::json::رستو رستو) {
  llvm::json::ObjectMapper O(قدر, رستو);
  أرجع O && O.map("textDocument", نتيجو.متن_Document_2) &&
         O.map("contentChanges", نتيجو.contentChanges);
}

//===----------------------------------------------------------------------===//
// MarkupContent
//===----------------------------------------------------------------------===//

ساكن llvm::StringRef منزل_متن_قسم(Markup_قسم قسم_2) {
  بدل (قسم_2) {
  حالة Markup_قسم::Plain_متن:
    أرجع "plaintext";
  حالة Markup_قسم::Markdown:
    أرجع "markdown";
  }
  llvm_unreachable("Invalid MarkupKind");
}

raw_ostream &llvm::lsp::عامل<<(raw_ostream &Os, Markup_قسم قسم_2) {
  أرجع Os << منزل_متن_قسم(قسم_2);
}

llvm::json::قدر llvm::lsp::منزل_JSON(ثابت MarkupContent &Mc) {
  إذا (Mc.قدر_2.خالي_جايون())
    أرجع خالي_اشارو;

  أرجع llvm::json::Object{
      {"kind", منزل_متن_قسم(Mc.قسم)},
      {"value", Mc.قدر_2},
  };
}

//===----------------------------------------------------------------------===//
// Hover
//===----------------------------------------------------------------------===//

llvm::json::قدر llvm::lsp::منزل_JSON(ثابت Hover &Hover) {
  llvm::json::Object نتيجو{{"contents", منزل_JSON(Hover.contents)}};
  إذا (Hover.range)
    نتيجو["range"] = منزل_JSON(*Hover.range);
  أرجع std::move(نتيجو);
}

//===----------------------------------------------------------------------===//
// DocumentSymbol
//===----------------------------------------------------------------------===//

llvm::json::قدر llvm::lsp::منزل_JSON(ثابت Document_نشان &نشان) {
  llvm::json::Object نتيجو{{"name", نشان.name},
                            {"kind", static_cast<صحيح>(نشان.قسم)},
                            {"range", نشان.range},
                            {"selectionRange", نشان.selectionRange}};

  إذا (!نشان.detail.خالي_جايون())
    نتيجو["detail"] = نشان.detail;
  إذا (!نشان.children.خالي_جايون())
    نتيجو["children"] = نشان.children;
  أرجع std::move(نتيجو);
}

//===----------------------------------------------------------------------===//
// DocumentSymbolParams
//===----------------------------------------------------------------------===//

منطقي llvm::lsp::fromJSON(ثابت llvm::json::قدر &قدر,
                         Document_نشان_Params &نتيجو, llvm::json::رستو رستو) {
  llvm::json::ObjectMapper O(قدر, رستو);
  أرجع O && O.map("textDocument", نتيجو.متن_Document_2);
}

//===----------------------------------------------------------------------===//
// DiagnosticRelatedInformation
//===----------------------------------------------------------------------===//

منطقي llvm::lsp::fromJSON(ثابت llvm::json::قدر &قدر,
                         DiagnosticRelatedInformation &نتيجو,
                         llvm::json::رستو رستو) {
  llvm::json::ObjectMapper O(قدر, رستو);
  أرجع O && O.map("location", نتيجو.location) &&
         O.map("message", نتيجو.پيغام);
}

llvm::json::قدر llvm::lsp::منزل_JSON(ثابت DiagnosticRelatedInformation &Info) {
  أرجع llvm::json::Object{
      {"location", Info.location},
      {"message", Info.پيغام},
  };
}

//===----------------------------------------------------------------------===//
// Diagnostic
//===----------------------------------------------------------------------===//

llvm::json::قدر llvm::lsp::منزل_JSON(Diagnostic_ٽيگ ٽيگ) {
  أرجع static_cast<صحيح>(ٽيگ);
}

منطقي llvm::lsp::fromJSON(ثابت llvm::json::قدر &قدر, Diagnostic_ٽيگ &نتيجو,
                         llvm::json::رستو رستو) {
  إذا (std::optional<int64_t> I = قدر.getAsInteger()) {
    نتيجو = (Diagnostic_ٽيگ)*I;
    أرجع سچ;
  }

  أرجع خطأ;
}

llvm::json::قدر llvm::lsp::منزل_JSON(ثابت Diagnostic &Diag) {
  llvm::json::Object نتيجو{
      {"range", Diag.range},
      {"severity", (صحيح)Diag.severity},
      {"message", Diag.پيغام},
  };
  إذا (Diag.category)
    نتيجو["category"] = *Diag.category;
  إذا (!Diag.ذريعو_2.خالي_جايون())
    نتيجو["source"] = Diag.ذريعو_2;
  إذا (Diag.relatedInformation)
    نتيجو["relatedInformation"] = *Diag.relatedInformation;
  إذا (!Diag.ٽيگ_فهرست.خالي_جايون())
    نتيجو["tags"] = Diag.ٽيگ_فهرست;
  أرجع std::move(نتيجو);
}

منطقي llvm::lsp::fromJSON(ثابت llvm::json::قدر &قدر, Diagnostic &نتيجو,
                         llvm::json::رستو رستو) {
  llvm::json::ObjectMapper O(قدر, رستو);
  إذا (!O)
    أرجع خطأ;
  صحيح Severity = 0;
  إذا (!mapOptOrNull(قدر, "severity", Severity, رستو))
    أرجع خطأ;
  نتيجو.severity = (DiagnosticSeverity)Severity;

  أرجع O.map("range", نتيجو.range) && O.map("message", نتيجو.پيغام) &&
         mapOptOrNull(قدر, "category", نتيجو.category, رستو) &&
         mapOptOrNull(قدر, "source", نتيجو.ذريعو_2, رستو) &&
         mapOptOrNull(قدر, "relatedInformation", نتيجو.relatedInformation,
                      رستو) &&
         mapOptOrNull(قدر, "tags", نتيجو.ٽيگ_فهرست, رستو);
}

//===----------------------------------------------------------------------===//
// PublishDiagnosticsParams
//===----------------------------------------------------------------------===//

llvm::json::قدر llvm::lsp::منزل_JSON(ثابت PublishDiagnosticsParams &Params) {
  أرجع llvm::json::Object{
      {"uri", Params.uri},
      {"diagnostics", Params.diagnostics},
      {"version", Params.نسخو_2},
  };
}

//===----------------------------------------------------------------------===//
// TextEdit
//===----------------------------------------------------------------------===//

منطقي llvm::lsp::fromJSON(ثابت llvm::json::قدر &قدر, متن_Edit &نتيجو,
                         llvm::json::رستو رستو) {
  llvm::json::ObjectMapper O(قدر, رستو);
  أرجع O && O.map("range", نتيجو.range) && O.map("newText", نتيجو.new_متن);
}

llvm::json::قدر llvm::lsp::منزل_JSON(ثابت متن_Edit &قدر) {
  أرجع llvm::json::Object{
      {"range", قدر.range},
      {"newText", قدر.new_متن},
  };
}

raw_ostream &llvm::lsp::عامل<<(raw_ostream &Os, ثابت متن_Edit &قدر) {
  Os << قدر.range << " => \"";
  llvm::printEscapedString(قدر.new_متن, Os);
  أرجع Os << '"';
}

//===----------------------------------------------------------------------===//
// CompletionItemKind
//===----------------------------------------------------------------------===//

منطقي llvm::lsp::fromJSON(ثابت llvm::json::قدر &قدر,
                         Completion_شي_قسم &نتيجو, llvm::json::رستو رستو) {
  إذا (std::optional<int64_t> Int_قدر = قدر.getAsInteger()) {
    إذا (*Int_قدر < static_cast<صحيح>(Completion_شي_قسم::متن_2) ||
        *Int_قدر > static_cast<صحيح>(Completion_شي_قسم::TypeParameter))
      أرجع خطأ;
    نتيجو = static_cast<Completion_شي_قسم>(*Int_قدر);
    أرجع سچ;
  }
  أرجع خطأ;
}

Completion_شي_قسم llvm::lsp::adjust_قسم_منزل_Capability(
    Completion_شي_قسم قسم_2,
    Completion_شي_قسم_Bitset &Supported_Completion_شي_Kinds) {
  size_t قسم_Val = static_cast<size_t>(قسم_2);
  إذا (قسم_Val >= k_Completion_شي_قسم_Min &&
      قسم_Val <= Supported_Completion_شي_Kinds.سائيز_2() &&
      Supported_Completion_شي_Kinds[قسم_Val])
    أرجع قسم_2;

  // Provide some fall backs for common kinds that are close enough.
  بدل (قسم_2) {
  حالة Completion_شي_قسم::Folder:
    أرجع Completion_شي_قسم::File;
  حالة Completion_شي_قسم::EnumMember:
    أرجع Completion_شي_قسم::Enum;
  حالة Completion_شي_قسم::Struct:
    أرجع Completion_شي_قسم::Class;
  افتراضي:
    أرجع Completion_شي_قسم::متن_2;
  }
}

منطقي llvm::lsp::fromJSON(ثابت llvm::json::قدر &قدر,
                         Completion_شي_قسم_Bitset &نتيجو,
                         llvm::json::رستو رستو) {
  إذا (ثابت llvm::json::Array *Array_قدر = قدر.getAsArray()) {
    لكل (size_t I = 0, E = Array_قدر->سائيز_2(); I < E; ++I) {
      Completion_شي_قسم قسم_ٻاهريون;
      إذا (fromJSON((*Array_قدر)[I], قسم_ٻاهريون, رستو.اشاريو(I)))
        نتيجو.سيٽ(size_t(قسم_ٻاهريون));
    }
    أرجع سچ;
  }
  أرجع خطأ;
}

//===----------------------------------------------------------------------===//
// CompletionItem
//===----------------------------------------------------------------------===//

llvm::json::قدر llvm::lsp::منزل_JSON(ثابت Completion_شي &قدر) {
  أكد(!قدر.label.خالي_جايون() && "completion item label is required");
  llvm::json::Object نتيجو{{"label", قدر.label}};
  إذا (قدر.قسم != Completion_شي_قسم::Missing)
    نتيجو["kind"] = static_cast<صحيح>(قدر.قسم);
  إذا (!قدر.detail.خالي_جايون())
    نتيجو["detail"] = قدر.detail;
  إذا (قدر.documentation)
    نتيجو["documentation"] = قدر.documentation;
  إذا (!قدر.ترتيب_ڏيو_متن.خالي_جايون())
    نتيجو["sortText"] = قدر.ترتيب_ڏيو_متن;
  إذا (!قدر.filter_متن.خالي_جايون())
    نتيجو["filterText"] = قدر.filter_متن;
  إذا (!قدر.وجهو_متن.خالي_جايون())
    نتيجو["insertText"] = قدر.وجهو_متن;
  إذا (قدر.وجهو_متن_Format != وجهو_متن_Format_2::Missing)
    نتيجو["insertTextFormat"] = static_cast<صحيح>(قدر.وجهو_متن_Format);
  إذا (قدر.متن_Edit_2)
    نتيجو["textEdit"] = *قدر.متن_Edit_2;
  إذا (!قدر.additional_متن_Edits.خالي_جايون()) {
    نتيجو["additionalTextEdits"] =
        llvm::json::Array(قدر.additional_متن_Edits);
  }
  إذا (قدر.deprecated)
    نتيجو["deprecated"] = قدر.deprecated;
  أرجع std::move(نتيجو);
}

raw_ostream &llvm::lsp::عامل<<(raw_ostream &Os,
                                   ثابت Completion_شي &قدر) {
  أرجع Os << قدر.label << " - " << منزل_JSON(قدر);
}

منطقي llvm::lsp::عامل<(ثابت Completion_شي &Lhs,
                          ثابت Completion_شي &Rhs) {
  أرجع (Lhs.ترتيب_ڏيو_متن.خالي_جايون() ? Lhs.label : Lhs.ترتيب_ڏيو_متن) <
         (Rhs.ترتيب_ڏيو_متن.خالي_جايون() ? Rhs.label : Rhs.ترتيب_ڏيو_متن);
}

//===----------------------------------------------------------------------===//
// CompletionList
//===----------------------------------------------------------------------===//

llvm::json::قدر llvm::lsp::منزل_JSON(ثابت CompletionList &قدر) {
  أرجع llvm::json::Object{
      {"isIncomplete", قدر.isIncomplete},
      {"items", llvm::json::Array(قدر.شيون)},
  };
}

//===----------------------------------------------------------------------===//
// CompletionContext
//===----------------------------------------------------------------------===//

منطقي llvm::lsp::fromJSON(ثابت llvm::json::قدر &قدر,
                         CompletionContext &نتيجو, llvm::json::رستو رستو) {
  llvm::json::ObjectMapper O(قدر, رستو);
  صحيح Trigger_قسم;
  إذا (!O || !O.map("triggerKind", Trigger_قسم) ||
      !mapOptOrNull(قدر, "triggerCharacter", نتيجو.triggerCharacter, رستو))
    أرجع خطأ;
  نتيجو.trigger_قسم = static_cast<Completion_Trigger_قسم>(Trigger_قسم);
  أرجع سچ;
}

//===----------------------------------------------------------------------===//
// CompletionParams
//===----------------------------------------------------------------------===//

منطقي llvm::lsp::fromJSON(ثابت llvm::json::قدر &قدر,
                         CompletionParams &نتيجو, llvm::json::رستو رستو) {
  إذا (!fromJSON(قدر, static_cast<متن_Document_Position_Params &>(نتيجو), رستو))
    أرجع خطأ;
  إذا (ثابت llvm::json::قدر *Context = قدر.getAsObject()->get("context"))
    أرجع fromJSON(*Context, نتيجو.context, رستو.ميدان("context"));
  أرجع سچ;
}

//===----------------------------------------------------------------------===//
// ParameterInformation
//===----------------------------------------------------------------------===//

llvm::json::قدر llvm::lsp::منزل_JSON(ثابت ParameterInformation &قدر) {
  أكد((قدر.labelOffsets || !قدر.labelString.خالي_جايون()) &&
         "parameter information label is required");
  llvm::json::Object نتيجو;
  إذا (قدر.labelOffsets)
    نتيجو["label"] = llvm::json::Array(
        {قدر.labelOffsets->پهريون_2, قدر.labelOffsets->ٻيو_2});
  وإلا
    نتيجو["label"] = قدر.labelString;
  إذا (!قدر.documentation.خالي_جايون())
    نتيجو["documentation"] = قدر.documentation;
  أرجع std::move(نتيجو);
}

//===----------------------------------------------------------------------===//
// SignatureInformation
//===----------------------------------------------------------------------===//

llvm::json::قدر llvm::lsp::منزل_JSON(ثابت SignatureInformation &قدر) {
  أكد(!قدر.label.خالي_جايون() && "signature information label is required");
  llvm::json::Object نتيجو{
      {"label", قدر.label},
      {"parameters", llvm::json::Array(قدر.parameters)},
  };
  إذا (!قدر.documentation.خالي_جايون())
    نتيجو["documentation"] = قدر.documentation;
  أرجع std::move(نتيجو);
}

raw_ostream &llvm::lsp::عامل<<(raw_ostream &Os,
                                   ثابت SignatureInformation &قدر) {
  أرجع Os << قدر.label << " - " << منزل_JSON(قدر);
}

//===----------------------------------------------------------------------===//
// SignatureHelp
//===----------------------------------------------------------------------===//

llvm::json::قدر llvm::lsp::منزل_JSON(ثابت SignatureHelp &قدر) {
  أكد(قدر.سرگرم_Signature >= 0 &&
         "Unexpected negative value for number of active signatures.");
  أكد(قدر.سرگرم_Parameter >= 0 &&
         "Unexpected negative value for active parameter index");
  أرجع llvm::json::Object{
      {"activeSignature", قدر.سرگرم_Signature},
      {"activeParameter", قدر.سرگرم_Parameter},
      {"signatures", llvm::json::Array(قدر.signatures)},
  };
}

//===----------------------------------------------------------------------===//
// DocumentLinkParams
//===----------------------------------------------------------------------===//

منطقي llvm::lsp::fromJSON(ثابت llvm::json::قدر &قدر,
                         DocumentLinkParams &نتيجو, llvm::json::رستو رستو) {
  llvm::json::ObjectMapper O(قدر, رستو);
  أرجع O && O.map("textDocument", نتيجو.متن_Document_2);
}

//===----------------------------------------------------------------------===//
// DocumentLink
//===----------------------------------------------------------------------===//

llvm::json::قدر llvm::lsp::منزل_JSON(ثابت DocumentLink &قدر) {
  أرجع llvm::json::Object{
      {"range", قدر.range},
      {"target", قدر.هدف},
  };
}

//===----------------------------------------------------------------------===//
// InlayHintsParams
//===----------------------------------------------------------------------===//

منطقي llvm::lsp::fromJSON(ثابت llvm::json::قدر &قدر,
                         InlayHintsParams &نتيجو, llvm::json::رستو رستو) {
  llvm::json::ObjectMapper O(قدر, رستو);
  أرجع O && O.map("textDocument", نتيجو.متن_Document_2) &&
         O.map("range", نتيجو.range);
}

//===----------------------------------------------------------------------===//
// InlayHint
//===----------------------------------------------------------------------===//

llvm::json::قدر llvm::lsp::منزل_JSON(ثابت InlayHint &قدر) {
  أرجع llvm::json::Object{{"position", قدر.position},
                            {"kind", (صحيح)قدر.قسم},
                            {"label", قدر.label},
                            {"paddingLeft", قدر.padding_کاٻو},
                            {"paddingRight", قدر.padding_ساڄو}};
}
منطقي llvm::lsp::عامل==(ثابت InlayHint &Lhs, ثابت InlayHint &Rhs) {
  أرجع std::tie(Lhs.position, Lhs.قسم, Lhs.label) ==
         std::tie(Rhs.position, Rhs.قسم, Rhs.label);
}
منطقي llvm::lsp::عامل<(ثابت InlayHint &Lhs, ثابت InlayHint &Rhs) {
  أرجع std::tie(Lhs.position, Lhs.قسم, Lhs.label) <
         std::tie(Rhs.position, Rhs.قسم, Rhs.label);
}

llvm::raw_ostream &llvm::lsp::عامل<<(llvm::raw_ostream &Os,
                                         Inlay_Hint_قسم قدر) {
  بدل (قدر) {
  حالة Inlay_Hint_قسم::Parameter:
    أرجع Os << "parameter";
  حالة Inlay_Hint_قسم::Type:
    أرجع Os << "type";
  }
  llvm_unreachable("Unknown InlayHintKind");
}

//===----------------------------------------------------------------------===//
// CodeActionContext
//===----------------------------------------------------------------------===//

منطقي llvm::lsp::fromJSON(ثابت llvm::json::قدر &قدر,
                         ڪوڊ_Action_Context &نتيجو, llvm::json::رستو رستو) {
  llvm::json::ObjectMapper O(قدر, رستو);
  إذا (!O || !O.map("diagnostics", نتيجو.diagnostics))
    أرجع خطأ;
  O.map("only", نتيجو.only);
  أرجع سچ;
}

//===----------------------------------------------------------------------===//
// CodeActionParams
//===----------------------------------------------------------------------===//

منطقي llvm::lsp::fromJSON(ثابت llvm::json::قدر &قدر,
                         ڪوڊ_Action_Params &نتيجو, llvm::json::رستو رستو) {
  llvm::json::ObjectMapper O(قدر, رستو);
  أرجع O && O.map("textDocument", نتيجو.متن_Document_2) &&
         O.map("range", نتيجو.range) && O.map("context", نتيجو.context);
}

//===----------------------------------------------------------------------===//
// WorkspaceEdit
//===----------------------------------------------------------------------===//

منطقي llvm::lsp::fromJSON(ثابت llvm::json::قدر &قدر, WorkspaceEdit &نتيجو,
                         llvm::json::رستو رستو) {
  llvm::json::ObjectMapper O(قدر, رستو);
  أرجع O && O.map("changes", نتيجو.changes);
}

llvm::json::قدر llvm::lsp::منزل_JSON(ثابت WorkspaceEdit &قدر) {
  llvm::json::Object FileChanges;
  لكل (خودڪار &Change : قدر.changes)
    FileChanges[Change.پهريون_2] = llvm::json::Array(Change.ٻيو_2);
  أرجع llvm::json::Object{{"changes", std::move(FileChanges)}};
}

//===----------------------------------------------------------------------===//
// CodeAction
//===----------------------------------------------------------------------===//

ثابت llvm::StringLiteral ڪوڊ_Action::k_تيز_Fix = "quickfix";
ثابت llvm::StringLiteral ڪوڊ_Action::kRefactor = "refactor";
ثابت llvm::StringLiteral ڪوڊ_Action::kInfo = "info";

llvm::json::قدر llvm::lsp::منزل_JSON(ثابت ڪوڊ_Action &قدر) {
  llvm::json::Object ڪوڊ_Action{{"title", قدر.title}};
  إذا (قدر.قسم)
    ڪوڊ_Action["kind"] = *قدر.قسم;
  إذا (قدر.diagnostics)
    ڪوڊ_Action["diagnostics"] = llvm::json::Array(*قدر.diagnostics);
  إذا (قدر.isPreferred)
    ڪوڊ_Action["isPreferred"] = سچ;
  إذا (قدر.edit)
    ڪوڊ_Action["edit"] = *قدر.edit;
  أرجع std::move(ڪوڊ_Action);
}

//===----------------------------------------------------------------------===//
// ShowMessageParams
//===----------------------------------------------------------------------===//

llvm::json::قدر llvm::lsp::منزل_JSON(ثابت Show_پيغام_Params &Params) {
  خودڪار ٻاهريون = llvm::json::Object{
      {"type", static_cast<صحيح>(Params.type)},
      {"message", Params.پيغام},
  };
  إذا (Params.actions)
    ٻاهريون["actions"] = *Params.actions;
  أرجع ٻاهريون;
}

llvm::json::قدر llvm::lsp::منزل_JSON(ثابت پيغام_Action_شي &Params) {
  أرجع llvm::json::Object{{"title", Params.title}};
}
