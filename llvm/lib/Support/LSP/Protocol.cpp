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

#شامل "llvm/Support/LSP/Protocol.h"
#شامل "llvm/ADT/StringExtras.h"
#شامل "llvm/ADT/StringSet.h"
#شامل "llvm/Support/ErrorHandling.h"
#شامل "llvm/Support/JSON.h"
#شامل "llvm/Support/MemoryBuffer.h"
#شامل "llvm/Support/Path.h"
#شامل "llvm/Support/raw_ostream.h"

using فضاینام llvm;
using فضاینام llvm::lsp;

// Helper that doesn't treat `null` and absent fields as failures.
template <typename T>
ایستا منطقی mapOptOrNull(ثابت llvm::json::مقدار &Params,
                         llvm::StringLiteral Prop, T &Out,
                         llvm::json::Path Path) {
  ثابت llvm::json::Object *O = Params.getAsObject();
  أكد(O);

  // Field is missing or null.
  auto *V = O->get(Prop);
  اگر (!V || V->getAsNull())
    بازگشت درست;
  بازگشت fromJSON(*V, Out, Path.field(Prop));
}

//===----------------------------------------------------------------------===//
// LSPError
//===----------------------------------------------------------------------===//

نویسه LSPError::ID;

//===----------------------------------------------------------------------===//
// URIForFile
//===----------------------------------------------------------------------===//

ایستا منطقی isWindowsPath(StringRef Path) {
  بازگشت Path.size() > 1 && llvm::isAlpha(Path[0]) && Path[1] == ':';
}

ایستا منطقی isNetworkPath(StringRef Path) {
  بازگشت Path.size() > 2 && Path[0] == Path[1] &&
         llvm::sys::path::is_separator(Path[0]);
}

ایستا منطقی shouldEscapeInURI(unsigned نویسه C) {
  // Unreserved characters.
  اگر ((C >= 'a' && C <= 'z') || (C >= 'A' && C <= 'Z') ||
      (C >= '0' && C <= '9'))
    بازگشت نادرست;

  انتخاب (C) {
  حالت '-':
  حالت '_':
  حالت '.':
  حالت '~':
  // '/' is only reserved when parsing.
  حالت '/':
  // ':' is only reserved for relative URI paths, which we doesn't produce.
  حالت ':':
    بازگشت نادرست;
  }
  بازگشت درست;
}

/// Encodes a string according to percent-encoding.
/// - Unreserved characters are not escaped.
/// - Reserved characters always escaped with exceptions like '/'.
/// - All other characters are escaped.
ایستا تهی percentEncode(StringRef Content, std::string &Out) {
  برای (unsigned نویسه C : Content) {
    اگر (shouldEscapeInURI(C)) {
      Out.افزودن_back('%');
      Out.افزودن_back(llvm::hexdigit(C / 16));
      Out.افزودن_back(llvm::hexdigit(C % 16));
    } وگرنه {
      Out.افزودن_back(C);
    }
  }
}

/// Decodes a string according to percent-encoding.
ایستا std::string percentDecode(StringRef Content) {
  std::string Result;
  برای (auto I = Content.begin(), E = Content.end(); I != E; ++I) {
    اگر (*I == '%' && I + 2 < Content.end() && llvm::isHexDigit(*(I + 1)) &&
        llvm::isHexDigit(*(I + 2))) {
      Result.افزودن_back(llvm::hexFromNibbles(*(I + 1), *(I + 2)));
      I += 2;
    } وگرنه {
      Result.افزودن_back(*I);
    }
  }
  بازگشت Result;
}

/// Return the set containing the supported URI schemes.
ایستا StringSet<> &getSupportedSchemes() {
  ایستا StringSet<> Schemes({"file", "test"});
  بازگشت Schemes;
}

/// Returns true if the given scheme is structurally valid, i.e. it does not
/// contain any invalid scheme characters. This does not check that the scheme
/// is actually supported.
ایستا منطقی isStructurallyValidScheme(StringRef Scheme) {
  اگر (Scheme.empty())
    بازگشت نادرست;
  اگر (!llvm::isAlpha(Scheme[0]))
    بازگشت نادرست;
  بازگشت llvm::all_of(llvm::drop_begin(Scheme), [](نویسه C) {
    بازگشت llvm::isAlnum(C) || C == '+' || C == '.' || C == '-';
  });
}

ایستا llvm::Expected<std::string> uriFromAbsolutePath(StringRef AbsolutePath,
                                                       StringRef Scheme) {
  std::string Body;
  StringRef Authority;
  StringRef ریشه_2 = llvm::sys::path::ریشه_name(AbsolutePath);
  اگر (isNetworkPath(ریشه_2)) {
    // Windows UNC paths e.g. \\server\share => file://server/share
    Authority = ریشه_2.drop_front(2);
    AbsolutePath.consume_front(ریشه_2);
  } وگرنه اگر (isWindowsPath(ریشه_2)) {
    // Windows paths e.g. X:\path => file:///X:/path
    Body = "/";
  }
  Body += llvm::sys::path::convert_to_slash(AbsolutePath);

  std::string Uri = Scheme.str() + ":";
  اگر (Authority.empty() && Body.empty())
    بازگشت Uri;

  // If authority if empty, we only print body if it starts with "/"; otherwise,
  // the URI is invalid.
  اگر (!Authority.empty() || StringRef(Body).starts_with("/")) {
    Uri.append("//");
    percentEncode(Authority, Uri);
  }
  percentEncode(Body, Uri);
  بازگشت Uri;
}

ایستا llvm::Expected<std::string> getAbsolutePath(StringRef Authority,
                                                   StringRef Body) {
  اگر (!Body.starts_with("/"))
    بازگشت llvm::createStringError(
        llvm::inconvertibleErrorCode(),
        "File scheme: expect body to be an absolute path starting "
        "with '/': " +
            Body);
  SmallString<128> Path;
  اگر (!Authority.empty()) {
    // Windows UNC paths e.g. file://server/share => \\server\share
    ("//" + Authority).toVector(Path);
  } وگرنه اگر (isWindowsPath(Body.substr(1))) {
    // Windows paths e.g. file:///X:/path => X:\path
    Body.consume_front("/");
  }
  Path.append(Body);
  llvm::sys::path::native(Path);
  بازگشت std::string(Path);
}

ایستا llvm::Expected<std::string> parseFilePathFromURI(StringRef OrigUri) {
  StringRef Uri = OrigUri;

  // Decode the scheme of the URI.
  size_t Pos = Uri.find(':');
  اگر (Pos == StringRef::npos)
    بازگشت llvm::createStringError(llvm::inconvertibleErrorCode(),
                                   "Scheme must be provided in URI: " +
                                       OrigUri);
  StringRef SchemeStr = Uri.substr(0, Pos);
  std::string UriScheme = percentDecode(SchemeStr);
  اگر (!isStructurallyValidScheme(UriScheme))
    بازگشت llvm::createStringError(llvm::inconvertibleErrorCode(),
                                   "Invalid scheme: " + SchemeStr +
                                       " (decoded: " + UriScheme + ")");
  Uri = Uri.substr(Pos + 1);

  // Decode the authority of the URI.
  std::string UriAuthority;
  اگر (Uri.consume_front("//")) {
    Pos = Uri.find('/');
    UriAuthority = percentDecode(Uri.substr(0, Pos));
    Uri = Uri.substr(Pos);
  }

  // Decode the body of the URI.
  std::string UriBody = percentDecode(Uri);

  // Compute the absolute path for this uri.
  اگر (!getSupportedSchemes().contains(UriScheme)) {
    بازگشت llvm::createStringError(llvm::inconvertibleErrorCode(),
                                   "unsupported URI scheme `" + UriScheme +
                                       "' for workspace files");
  }
  بازگشت getAbsolutePath(UriAuthority, UriBody);
}

llvm::Expected<URIForFile> URIForFile::fromURI(StringRef Uri) {
  llvm::Expected<std::string> FilePath = parseFilePathFromURI(Uri);
  اگر (!FilePath)
    بازگشت FilePath.takeError();
  بازگشت URIForFile(std::move(*FilePath), Uri.str());
}

llvm::Expected<URIForFile> URIForFile::fromFile(StringRef AbsoluteFilepath,
                                                StringRef Scheme) {
  llvm::Expected<std::string> Uri =
      uriFromAbsolutePath(AbsoluteFilepath, Scheme);
  اگر (!Uri)
    بازگشت Uri.takeError();
  بازگشت fromURI(*Uri);
}

StringRef URIForFile::scheme() ثابت { بازگشت uri().split(':').first; }

تهی URIForFile::registerSupportedScheme(StringRef Scheme) {
  getSupportedSchemes().درج(Scheme);
}

منطقی llvm::lsp::fromJSON(ثابت llvm::json::مقدار &مقدار, URIForFile &Result,
                         llvm::json::Path Path) {
  اگر (std::optional<StringRef> Str = مقدار.getAsString()) {
    llvm::Expected<URIForFile> ExpectedUri = URIForFile::fromURI(*Str);
    اگر (!ExpectedUri) {
      Path.report("unresolvable URI");
      consumeError(ExpectedUri.takeError());
      بازگشت نادرست;
    }
    Result = std::move(*ExpectedUri);
    بازگشت درست;
  }
  بازگشت نادرست;
}

llvm::json::مقدار llvm::lsp::toJSON(ثابت URIForFile &مقدار) {
  بازگشت مقدار.uri();
}

raw_ostream &llvm::lsp::operator<<(raw_ostream &Os, ثابت URIForFile &مقدار) {
  بازگشت Os << مقدار.uri();
}

//===----------------------------------------------------------------------===//
// ClientCapabilities
//===----------------------------------------------------------------------===//

منطقی llvm::lsp::fromJSON(ثابت llvm::json::مقدار &مقدار,
                         ClientCapabilities &Result, llvm::json::Path Path) {
  ثابت llvm::json::Object *O = مقدار.getAsObject();
  اگر (!O) {
    Path.report("expected object");
    بازگشت نادرست;
  }
  اگر (ثابت llvm::json::Object *TextDocument = O->getObject("textDocument")) {
    اگر (ثابت llvm::json::Object *Document_نماد =
            TextDocument->getObject("documentSymbol")) {
      اگر (std::optional<منطقی> HierarchicalSupport =
              Document_نماد->getBoolean("hierarchicalDocumentSymbolSupport"))
        Result.hierarchical_Document_نماد = *HierarchicalSupport;
    }
    اگر (auto *CodeAction = TextDocument->getObject("codeAction")) {
      اگر (CodeAction->getObject("codeActionLiteralSupport"))
        Result.codeActionStructure = درست;
    }
  }
  اگر (auto *Window = O->getObject("window")) {
    اگر (std::optional<منطقی> WorkDoneProgressSupport =
            Window->getBoolean("workDoneProgress"))
      Result.workDoneProgress = *WorkDoneProgressSupport;
  }
  بازگشت درست;
}

//===----------------------------------------------------------------------===//
// ClientInfo
//===----------------------------------------------------------------------===//

منطقی llvm::lsp::fromJSON(ثابت llvm::json::مقدار &مقدار, ClientInfo &Result,
                         llvm::json::Path Path) {
  llvm::json::ObjectMapper O(مقدار, Path);
  اگر (!O || !O.map("name", Result.name))
    بازگشت نادرست;

  // Don't fail if we can't parse version.
  O.map("version", Result.version);
  بازگشت درست;
}

//===----------------------------------------------------------------------===//
// InitializeParams
//===----------------------------------------------------------------------===//

منطقی llvm::lsp::fromJSON(ثابت llvm::json::مقدار &مقدار, TraceLevel &Result,
                         llvm::json::Path Path) {
  اگر (std::optional<StringRef> Str = مقدار.getAsString()) {
    اگر (*Str == "off") {
      Result = TraceLevel::Off;
      بازگشت درست;
    }
    اگر (*Str == "messages") {
      Result = TraceLevel::Messages;
      بازگشت درست;
    }
    اگر (*Str == "verbose") {
      Result = TraceLevel::Verbose;
      بازگشت درست;
    }
  }
  بازگشت نادرست;
}

منطقی llvm::lsp::fromJSON(ثابت llvm::json::مقدار &مقدار,
                         InitializeParams &Result, llvm::json::Path Path) {
  llvm::json::ObjectMapper O(مقدار, Path);
  اگر (!O)
    بازگشت نادرست;
  // We deliberately don't fail if we can't parse individual fields.
  O.map("capabilities", Result.capabilities);
  O.map("trace", Result.trace);
  O.map("rootUri", Result.ریشه_Uri);
  O.map("rootPath", Result.ریشه_Path);
  mapOptOrNull(مقدار, "clientInfo", Result.clientInfo, Path);

  بازگشت درست;
}

//===----------------------------------------------------------------------===//
// TextDocumentItem
//===----------------------------------------------------------------------===//

منطقی llvm::lsp::fromJSON(ثابت llvm::json::مقدار &مقدار,
                         TextDocumentItem &Result, llvm::json::Path Path) {
  llvm::json::ObjectMapper O(مقدار, Path);
  بازگشت O && O.map("uri", Result.uri) &&
         O.map("languageId", Result.languageId) && O.map("text", Result.text) &&
         O.map("version", Result.version);
}

//===----------------------------------------------------------------------===//
// TextDocumentIdentifier
//===----------------------------------------------------------------------===//

llvm::json::مقدار llvm::lsp::toJSON(ثابت TextDocumentIdentifier &مقدار) {
  بازگشت llvm::json::Object{{"uri", مقدار.uri}};
}

منطقی llvm::lsp::fromJSON(ثابت llvm::json::مقدار &مقدار,
                         TextDocumentIdentifier &Result,
                         llvm::json::Path Path) {
  llvm::json::ObjectMapper O(مقدار, Path);
  بازگشت O && O.map("uri", Result.uri);
}

//===----------------------------------------------------------------------===//
// VersionedTextDocumentIdentifier
//===----------------------------------------------------------------------===//

llvm::json::مقدار
llvm::lsp::toJSON(ثابت VersionedTextDocumentIdentifier &مقدار) {
  بازگشت llvm::json::Object{
      {"uri", مقدار.uri},
      {"version", مقدار.version},
  };
}

منطقی llvm::lsp::fromJSON(ثابت llvm::json::مقدار &مقدار,
                         VersionedTextDocumentIdentifier &Result,
                         llvm::json::Path Path) {
  llvm::json::ObjectMapper O(مقدار, Path);
  بازگشت O && O.map("uri", Result.uri) && O.map("version", Result.version);
}

//===----------------------------------------------------------------------===//
// Position
//===----------------------------------------------------------------------===//

منطقی llvm::lsp::fromJSON(ثابت llvm::json::مقدار &مقدار, Position &Result,
                         llvm::json::Path Path) {
  llvm::json::ObjectMapper O(مقدار, Path);
  بازگشت O && O.map("line", Result.سطر_2) &&
         O.map("character", Result.character);
}

llvm::json::مقدار llvm::lsp::toJSON(ثابت Position &مقدار) {
  بازگشت llvm::json::Object{
      {"line", مقدار.سطر_2},
      {"character", مقدار.character},
  };
}

raw_ostream &llvm::lsp::operator<<(raw_ostream &Os, ثابت Position &مقدار) {
  بازگشت Os << مقدار.سطر_2 << ':' << مقدار.character;
}

//===----------------------------------------------------------------------===//
// Range
//===----------------------------------------------------------------------===//

منطقی llvm::lsp::fromJSON(ثابت llvm::json::مقدار &مقدار, Range &Result,
                         llvm::json::Path Path) {
  llvm::json::ObjectMapper O(مقدار, Path);
  بازگشت O && O.map("start", Result.start) && O.map("end", Result.end);
}

llvm::json::مقدار llvm::lsp::toJSON(ثابت Range &مقدار) {
  بازگشت llvm::json::Object{
      {"start", مقدار.start},
      {"end", مقدار.end},
  };
}

raw_ostream &llvm::lsp::operator<<(raw_ostream &Os, ثابت Range &مقدار) {
  بازگشت Os << مقدار.start << '-' << مقدار.end;
}

//===----------------------------------------------------------------------===//
// Location
//===----------------------------------------------------------------------===//

منطقی llvm::lsp::fromJSON(ثابت llvm::json::مقدار &مقدار, Location &Result,
                         llvm::json::Path Path) {
  llvm::json::ObjectMapper O(مقدار, Path);
  بازگشت O && O.map("uri", Result.uri) && O.map("range", Result.range);
}

llvm::json::مقدار llvm::lsp::toJSON(ثابت Location &مقدار) {
  بازگشت llvm::json::Object{
      {"uri", مقدار.uri},
      {"range", مقدار.range},
  };
}

raw_ostream &llvm::lsp::operator<<(raw_ostream &Os, ثابت Location &مقدار) {
  بازگشت Os << مقدار.range << '@' << مقدار.uri;
}

//===----------------------------------------------------------------------===//
// TextDocumentPositionParams
//===----------------------------------------------------------------------===//

منطقی llvm::lsp::fromJSON(ثابت llvm::json::مقدار &مقدار,
                         TextDocumentPositionParams &Result,
                         llvm::json::Path Path) {
  llvm::json::ObjectMapper O(مقدار, Path);
  بازگشت O && O.map("textDocument", Result.textDocument) &&
         O.map("position", Result.position);
}

//===----------------------------------------------------------------------===//
// ReferenceParams
//===----------------------------------------------------------------------===//

منطقی llvm::lsp::fromJSON(ثابت llvm::json::مقدار &مقدار,
                         ReferenceContext &Result, llvm::json::Path Path) {
  llvm::json::ObjectMapper O(مقدار, Path);
  بازگشت O && O.mapOptional("includeDeclaration", Result.includeDeclaration);
}

منطقی llvm::lsp::fromJSON(ثابت llvm::json::مقدار &مقدار,
                         ReferenceParams &Result, llvm::json::Path Path) {
  TextDocumentPositionParams &Base = Result;
  llvm::json::ObjectMapper O(مقدار, Path);
  بازگشت fromJSON(مقدار, Base, Path) && O &&
         O.mapOptional("context", Result.context);
}

//===----------------------------------------------------------------------===//
// DidOpenTextDocumentParams
//===----------------------------------------------------------------------===//

منطقی llvm::lsp::fromJSON(ثابت llvm::json::مقدار &مقدار,
                         DidOpenTextDocumentParams &Result,
                         llvm::json::Path Path) {
  llvm::json::ObjectMapper O(مقدار, Path);
  بازگشت O && O.map("textDocument", Result.textDocument);
}

//===----------------------------------------------------------------------===//
// DidCloseTextDocumentParams
//===----------------------------------------------------------------------===//

منطقی llvm::lsp::fromJSON(ثابت llvm::json::مقدار &مقدار,
                         DidCloseTextDocumentParams &Result,
                         llvm::json::Path Path) {
  llvm::json::ObjectMapper O(مقدار, Path);
  بازگشت O && O.map("textDocument", Result.textDocument);
}

//===----------------------------------------------------------------------===//
// DidSaveTextDocumentParams
//===----------------------------------------------------------------------===//

منطقی llvm::lsp::fromJSON(ثابت llvm::json::مقدار &Params,
                         DidSaveTextDocumentParams &R, llvm::json::Path P) {
  llvm::json::ObjectMapper O(Params, P);
  بازگشت O && O.map("textDocument", R.textDocument);
}

//===----------------------------------------------------------------------===//
// DidChangeTextDocumentParams
//===----------------------------------------------------------------------===//

LogicalResult
TextDocumentContentChangeEvent::applyTo(std::string &Contents) ثابت {
  // If there is no range, the full document changed.
  اگر (!range) {
    Contents = text;
    بازگشت success();
  }

  // Try to map the replacement range to the content.
  llvm::SourceMgr TmpScrMgr;
  TmpScrMgr.AddNewSourceBuffer(llvm::MemoryBuffer::getMemBuffer(Contents),
                               SMLoc());
  SMRange RangeLoc = range->getAsSMRange(TmpScrMgr);
  اگر (!RangeLoc.isValid())
    بازگشت failure();

  Contents.replace(RangeLoc.Start.getPointer() - Contents.data(),
                   RangeLoc.End.getPointer() - RangeLoc.Start.getPointer(),
                   text);
  بازگشت success();
}

LogicalResult TextDocumentContentChangeEvent::applyTo(
    ArrayRef<TextDocumentContentChangeEvent> Changes, std::string &Contents) {
  برای (ثابت auto &Change : Changes)
    اگر (failed(Change.applyTo(Contents)))
      بازگشت failure();
  بازگشت success();
}

منطقی llvm::lsp::fromJSON(ثابت llvm::json::مقدار &مقدار,
                         TextDocumentContentChangeEvent &Result,
                         llvm::json::Path Path) {
  llvm::json::ObjectMapper O(مقدار, Path);
  بازگشت O && O.map("range", Result.range) &&
         O.map("rangeLength", Result.rangeLength) && O.map("text", Result.text);
}

منطقی llvm::lsp::fromJSON(ثابت llvm::json::مقدار &مقدار,
                         DidChangeTextDocumentParams &Result,
                         llvm::json::Path Path) {
  llvm::json::ObjectMapper O(مقدار, Path);
  بازگشت O && O.map("textDocument", Result.textDocument) &&
         O.map("contentChanges", Result.contentChanges);
}

//===----------------------------------------------------------------------===//
// MarkupContent
//===----------------------------------------------------------------------===//

ایستا llvm::StringRef toTextKind(MarkupKind Kind) {
  انتخاب (Kind) {
  حالت MarkupKind::PlainText:
    بازگشت "plaintext";
  حالت MarkupKind::Markdown:
    بازگشت "markdown";
  }
  llvm_unreachable("Invalid MarkupKind");
}

raw_ostream &llvm::lsp::operator<<(raw_ostream &Os, MarkupKind Kind) {
  بازگشت Os << toTextKind(Kind);
}

llvm::json::مقدار llvm::lsp::toJSON(ثابت MarkupContent &Mc) {
  اگر (Mc.مقدار_2.empty())
    بازگشت nullptr;

  بازگشت llvm::json::Object{
      {"kind", toTextKind(Mc.kind)},
      {"value", Mc.مقدار_2},
  };
}

//===----------------------------------------------------------------------===//
// Hover
//===----------------------------------------------------------------------===//

llvm::json::مقدار llvm::lsp::toJSON(ثابت Hover &Hover) {
  llvm::json::Object Result{{"contents", toJSON(Hover.contents)}};
  اگر (Hover.range)
    Result["range"] = toJSON(*Hover.range);
  بازگشت std::move(Result);
}

//===----------------------------------------------------------------------===//
// DocumentSymbol
//===----------------------------------------------------------------------===//

llvm::json::مقدار llvm::lsp::toJSON(ثابت Document_نماد &نماد) {
  llvm::json::Object Result{{"name", نماد.name},
                            {"kind", static_cast<صحیح>(نماد.kind)},
                            {"range", نماد.range},
                            {"selectionRange", نماد.selectionRange}};

  اگر (!نماد.detail.empty())
    Result["detail"] = نماد.detail;
  اگر (!نماد.children.empty())
    Result["children"] = نماد.children;
  بازگشت std::move(Result);
}

//===----------------------------------------------------------------------===//
// DocumentSymbolParams
//===----------------------------------------------------------------------===//

منطقی llvm::lsp::fromJSON(ثابت llvm::json::مقدار &مقدار,
                         Document_نماد_Params &Result, llvm::json::Path Path) {
  llvm::json::ObjectMapper O(مقدار, Path);
  بازگشت O && O.map("textDocument", Result.textDocument);
}

//===----------------------------------------------------------------------===//
// DiagnosticRelatedInformation
//===----------------------------------------------------------------------===//

منطقی llvm::lsp::fromJSON(ثابت llvm::json::مقدار &مقدار,
                         DiagnosticRelatedInformation &Result,
                         llvm::json::Path Path) {
  llvm::json::ObjectMapper O(مقدار, Path);
  بازگشت O && O.map("location", Result.location) &&
         O.map("message", Result.message);
}

llvm::json::مقدار llvm::lsp::toJSON(ثابت DiagnosticRelatedInformation &Info) {
  بازگشت llvm::json::Object{
      {"location", Info.location},
      {"message", Info.message},
  };
}

//===----------------------------------------------------------------------===//
// Diagnostic
//===----------------------------------------------------------------------===//

llvm::json::مقدار llvm::lsp::toJSON(DiagnosticTag Tag) {
  بازگشت static_cast<صحیح>(Tag);
}

منطقی llvm::lsp::fromJSON(ثابت llvm::json::مقدار &مقدار, DiagnosticTag &Result,
                         llvm::json::Path Path) {
  اگر (std::optional<int64_t> I = مقدار.getAsInteger()) {
    Result = (DiagnosticTag)*I;
    بازگشت درست;
  }

  بازگشت نادرست;
}

llvm::json::مقدار llvm::lsp::toJSON(ثابت Diagnostic &Diag) {
  llvm::json::Object Result{
      {"range", Diag.range},
      {"severity", (صحیح)Diag.severity},
      {"message", Diag.message},
  };
  اگر (Diag.category)
    Result["category"] = *Diag.category;
  اگر (!Diag.source.empty())
    Result["source"] = Diag.source;
  اگر (Diag.relatedInformation)
    Result["relatedInformation"] = *Diag.relatedInformation;
  اگر (!Diag.tags.empty())
    Result["tags"] = Diag.tags;
  بازگشت std::move(Result);
}

منطقی llvm::lsp::fromJSON(ثابت llvm::json::مقدار &مقدار, Diagnostic &Result,
                         llvm::json::Path Path) {
  llvm::json::ObjectMapper O(مقدار, Path);
  اگر (!O)
    بازگشت نادرست;
  صحیح Severity = 0;
  اگر (!mapOptOrNull(مقدار, "severity", Severity, Path))
    بازگشت نادرست;
  Result.severity = (DiagnosticSeverity)Severity;

  بازگشت O.map("range", Result.range) && O.map("message", Result.message) &&
         mapOptOrNull(مقدار, "category", Result.category, Path) &&
         mapOptOrNull(مقدار, "source", Result.source, Path) &&
         mapOptOrNull(مقدار, "relatedInformation", Result.relatedInformation,
                      Path) &&
         mapOptOrNull(مقدار, "tags", Result.tags, Path);
}

//===----------------------------------------------------------------------===//
// PublishDiagnosticsParams
//===----------------------------------------------------------------------===//

llvm::json::مقدار llvm::lsp::toJSON(ثابت PublishDiagnosticsParams &Params) {
  بازگشت llvm::json::Object{
      {"uri", Params.uri},
      {"diagnostics", Params.diagnostics},
      {"version", Params.version},
  };
}

//===----------------------------------------------------------------------===//
// TextEdit
//===----------------------------------------------------------------------===//

منطقی llvm::lsp::fromJSON(ثابت llvm::json::مقدار &مقدار, TextEdit &Result,
                         llvm::json::Path Path) {
  llvm::json::ObjectMapper O(مقدار, Path);
  بازگشت O && O.map("range", Result.range) && O.map("newText", Result.newText);
}

llvm::json::مقدار llvm::lsp::toJSON(ثابت TextEdit &مقدار) {
  بازگشت llvm::json::Object{
      {"range", مقدار.range},
      {"newText", مقدار.newText},
  };
}

raw_ostream &llvm::lsp::operator<<(raw_ostream &Os, ثابت TextEdit &مقدار) {
  Os << مقدار.range << " => \"";
  llvm::printEscapedString(مقدار.newText, Os);
  بازگشت Os << '"';
}

//===----------------------------------------------------------------------===//
// CompletionItemKind
//===----------------------------------------------------------------------===//

منطقی llvm::lsp::fromJSON(ثابت llvm::json::مقدار &مقدار,
                         CompletionItemKind &Result, llvm::json::Path Path) {
  اگر (std::optional<int64_t> Int_مقدار = مقدار.getAsInteger()) {
    اگر (*Int_مقدار < static_cast<صحیح>(CompletionItemKind::Text) ||
        *Int_مقدار > static_cast<صحیح>(CompletionItemKind::TypeParameter))
      بازگشت نادرست;
    Result = static_cast<CompletionItemKind>(*Int_مقدار);
    بازگشت درست;
  }
  بازگشت نادرست;
}

CompletionItemKind llvm::lsp::adjustKindToCapability(
    CompletionItemKind Kind,
    CompletionItemKindBitset &SupportedCompletionItemKinds) {
  size_t KindVal = static_cast<size_t>(Kind);
  اگر (KindVal >= kCompletionItemKindMin &&
      KindVal <= SupportedCompletionItemKinds.size() &&
      SupportedCompletionItemKinds[KindVal])
    بازگشت Kind;

  // Provide some fall backs for common kinds that are close enough.
  انتخاب (Kind) {
  حالت CompletionItemKind::Folder:
    بازگشت CompletionItemKind::File;
  حالت CompletionItemKind::EnumMember:
    بازگشت CompletionItemKind::Enum;
  حالت CompletionItemKind::Struct:
    بازگشت CompletionItemKind::Class;
  پیشفرض:
    بازگشت CompletionItemKind::Text;
  }
}

منطقی llvm::lsp::fromJSON(ثابت llvm::json::مقدار &مقدار,
                         CompletionItemKindBitset &Result,
                         llvm::json::Path Path) {
  اگر (ثابت llvm::json::Array *Array_مقدار = مقدار.getAsArray()) {
    برای (size_t I = 0, E = Array_مقدار->size(); I < E; ++I) {
      CompletionItemKind KindOut;
      اگر (fromJSON((*Array_مقدار)[I], KindOut, Path.index(I)))
        Result.set(size_t(KindOut));
    }
    بازگشت درست;
  }
  بازگشت نادرست;
}

//===----------------------------------------------------------------------===//
// CompletionItem
//===----------------------------------------------------------------------===//

llvm::json::مقدار llvm::lsp::toJSON(ثابت CompletionItem &مقدار) {
  أكد(!مقدار.label.empty() && "completion item label is required");
  llvm::json::Object Result{{"label", مقدار.label}};
  اگر (مقدار.kind != CompletionItemKind::Missing)
    Result["kind"] = static_cast<صحیح>(مقدار.kind);
  اگر (!مقدار.detail.empty())
    Result["detail"] = مقدار.detail;
  اگر (مقدار.documentation)
    Result["documentation"] = مقدار.documentation;
  اگر (!مقدار.sortText.empty())
    Result["sortText"] = مقدار.sortText;
  اگر (!مقدار.filterText.empty())
    Result["filterText"] = مقدار.filterText;
  اگر (!مقدار.درج_Text.empty())
    Result["insertText"] = مقدار.درج_Text;
  اگر (مقدار.درج_Text_Format != درج_Text_Format_2::Missing)
    Result["insertTextFormat"] = static_cast<صحیح>(مقدار.درج_Text_Format);
  اگر (مقدار.textEdit)
    Result["textEdit"] = *مقدار.textEdit;
  اگر (!مقدار.additionalTextEdits.empty()) {
    Result["additionalTextEdits"] =
        llvm::json::Array(مقدار.additionalTextEdits);
  }
  اگر (مقدار.deprecated)
    Result["deprecated"] = مقدار.deprecated;
  بازگشت std::move(Result);
}

raw_ostream &llvm::lsp::operator<<(raw_ostream &Os,
                                   ثابت CompletionItem &مقدار) {
  بازگشت Os << مقدار.label << " - " << toJSON(مقدار);
}

منطقی llvm::lsp::operator<(ثابت CompletionItem &Lhs,
                          ثابت CompletionItem &Rhs) {
  بازگشت (Lhs.sortText.empty() ? Lhs.label : Lhs.sortText) <
         (Rhs.sortText.empty() ? Rhs.label : Rhs.sortText);
}

//===----------------------------------------------------------------------===//
// CompletionList
//===----------------------------------------------------------------------===//

llvm::json::مقدار llvm::lsp::toJSON(ثابت CompletionList &مقدار) {
  بازگشت llvm::json::Object{
      {"isIncomplete", مقدار.isIncomplete},
      {"items", llvm::json::Array(مقدار.اقلام)},
  };
}

//===----------------------------------------------------------------------===//
// CompletionContext
//===----------------------------------------------------------------------===//

منطقی llvm::lsp::fromJSON(ثابت llvm::json::مقدار &مقدار,
                         CompletionContext &Result, llvm::json::Path Path) {
  llvm::json::ObjectMapper O(مقدار, Path);
  صحیح TriggerKind;
  اگر (!O || !O.map("triggerKind", TriggerKind) ||
      !mapOptOrNull(مقدار, "triggerCharacter", Result.triggerCharacter, Path))
    بازگشت نادرست;
  Result.triggerKind = static_cast<CompletionTriggerKind>(TriggerKind);
  بازگشت درست;
}

//===----------------------------------------------------------------------===//
// CompletionParams
//===----------------------------------------------------------------------===//

منطقی llvm::lsp::fromJSON(ثابت llvm::json::مقدار &مقدار,
                         CompletionParams &Result, llvm::json::Path Path) {
  اگر (!fromJSON(مقدار, static_cast<TextDocumentPositionParams &>(Result), Path))
    بازگشت نادرست;
  اگر (ثابت llvm::json::مقدار *Context = مقدار.getAsObject()->get("context"))
    بازگشت fromJSON(*Context, Result.context, Path.field("context"));
  بازگشت درست;
}

//===----------------------------------------------------------------------===//
// ParameterInformation
//===----------------------------------------------------------------------===//

llvm::json::مقدار llvm::lsp::toJSON(ثابت ParameterInformation &مقدار) {
  أكد((مقدار.labelOffsets || !مقدار.labelString.empty()) &&
         "parameter information label is required");
  llvm::json::Object Result;
  اگر (مقدار.labelOffsets)
    Result["label"] = llvm::json::Array(
        {مقدار.labelOffsets->first, مقدار.labelOffsets->second});
  وگرنه
    Result["label"] = مقدار.labelString;
  اگر (!مقدار.documentation.empty())
    Result["documentation"] = مقدار.documentation;
  بازگشت std::move(Result);
}

//===----------------------------------------------------------------------===//
// SignatureInformation
//===----------------------------------------------------------------------===//

llvm::json::مقدار llvm::lsp::toJSON(ثابت SignatureInformation &مقدار) {
  أكد(!مقدار.label.empty() && "signature information label is required");
  llvm::json::Object Result{
      {"label", مقدار.label},
      {"parameters", llvm::json::Array(مقدار.parameters)},
  };
  اگر (!مقدار.documentation.empty())
    Result["documentation"] = مقدار.documentation;
  بازگشت std::move(Result);
}

raw_ostream &llvm::lsp::operator<<(raw_ostream &Os,
                                   ثابت SignatureInformation &مقدار) {
  بازگشت Os << مقدار.label << " - " << toJSON(مقدار);
}

//===----------------------------------------------------------------------===//
// SignatureHelp
//===----------------------------------------------------------------------===//

llvm::json::مقدار llvm::lsp::toJSON(ثابت SignatureHelp &مقدار) {
  أكد(مقدار.activeSignature >= 0 &&
         "Unexpected negative value for number of active signatures.");
  أكد(مقدار.activeParameter >= 0 &&
         "Unexpected negative value for active parameter index");
  بازگشت llvm::json::Object{
      {"activeSignature", مقدار.activeSignature},
      {"activeParameter", مقدار.activeParameter},
      {"signatures", llvm::json::Array(مقدار.signatures)},
  };
}

//===----------------------------------------------------------------------===//
// DocumentLinkParams
//===----------------------------------------------------------------------===//

منطقی llvm::lsp::fromJSON(ثابت llvm::json::مقدار &مقدار,
                         DocumentLinkParams &Result, llvm::json::Path Path) {
  llvm::json::ObjectMapper O(مقدار, Path);
  بازگشت O && O.map("textDocument", Result.textDocument);
}

//===----------------------------------------------------------------------===//
// DocumentLink
//===----------------------------------------------------------------------===//

llvm::json::مقدار llvm::lsp::toJSON(ثابت DocumentLink &مقدار) {
  بازگشت llvm::json::Object{
      {"range", مقدار.range},
      {"target", مقدار.target},
  };
}

//===----------------------------------------------------------------------===//
// InlayHintsParams
//===----------------------------------------------------------------------===//

منطقی llvm::lsp::fromJSON(ثابت llvm::json::مقدار &مقدار,
                         InlayHintsParams &Result, llvm::json::Path Path) {
  llvm::json::ObjectMapper O(مقدار, Path);
  بازگشت O && O.map("textDocument", Result.textDocument) &&
         O.map("range", Result.range);
}

//===----------------------------------------------------------------------===//
// InlayHint
//===----------------------------------------------------------------------===//

llvm::json::مقدار llvm::lsp::toJSON(ثابت InlayHint &مقدار) {
  بازگشت llvm::json::Object{{"position", مقدار.position},
                            {"kind", (صحیح)مقدار.kind},
                            {"label", مقدار.label},
                            {"paddingLeft", مقدار.padding_چپ},
                            {"paddingRight", مقدار.padding_راست}};
}
منطقی llvm::lsp::operator==(ثابت InlayHint &Lhs, ثابت InlayHint &Rhs) {
  بازگشت std::tie(Lhs.position, Lhs.kind, Lhs.label) ==
         std::tie(Rhs.position, Rhs.kind, Rhs.label);
}
منطقی llvm::lsp::operator<(ثابت InlayHint &Lhs, ثابت InlayHint &Rhs) {
  بازگشت std::tie(Lhs.position, Lhs.kind, Lhs.label) <
         std::tie(Rhs.position, Rhs.kind, Rhs.label);
}

llvm::raw_ostream &llvm::lsp::operator<<(llvm::raw_ostream &Os,
                                         InlayHintKind مقدار) {
  انتخاب (مقدار) {
  حالت InlayHintKind::Parameter:
    بازگشت Os << "parameter";
  حالت InlayHintKind::Type:
    بازگشت Os << "type";
  }
  llvm_unreachable("Unknown InlayHintKind");
}

//===----------------------------------------------------------------------===//
// CodeActionContext
//===----------------------------------------------------------------------===//

منطقی llvm::lsp::fromJSON(ثابت llvm::json::مقدار &مقدار,
                         CodeActionContext &Result, llvm::json::Path Path) {
  llvm::json::ObjectMapper O(مقدار, Path);
  اگر (!O || !O.map("diagnostics", Result.diagnostics))
    بازگشت نادرست;
  O.map("only", Result.only);
  بازگشت درست;
}

//===----------------------------------------------------------------------===//
// CodeActionParams
//===----------------------------------------------------------------------===//

منطقی llvm::lsp::fromJSON(ثابت llvm::json::مقدار &مقدار,
                         CodeActionParams &Result, llvm::json::Path Path) {
  llvm::json::ObjectMapper O(مقدار, Path);
  بازگشت O && O.map("textDocument", Result.textDocument) &&
         O.map("range", Result.range) && O.map("context", Result.context);
}

//===----------------------------------------------------------------------===//
// WorkspaceEdit
//===----------------------------------------------------------------------===//

منطقی llvm::lsp::fromJSON(ثابت llvm::json::مقدار &مقدار, WorkspaceEdit &Result,
                         llvm::json::Path Path) {
  llvm::json::ObjectMapper O(مقدار, Path);
  بازگشت O && O.map("changes", Result.changes);
}

llvm::json::مقدار llvm::lsp::toJSON(ثابت WorkspaceEdit &مقدار) {
  llvm::json::Object FileChanges;
  برای (auto &Change : مقدار.changes)
    FileChanges[Change.first] = llvm::json::Array(Change.second);
  بازگشت llvm::json::Object{{"changes", std::move(FileChanges)}};
}

//===----------------------------------------------------------------------===//
// CodeAction
//===----------------------------------------------------------------------===//

ثابت llvm::StringLiteral CodeAction::kQuickFix = "quickfix";
ثابت llvm::StringLiteral CodeAction::kRefactor = "refactor";
ثابت llvm::StringLiteral CodeAction::kInfo = "info";

llvm::json::مقدار llvm::lsp::toJSON(ثابت CodeAction &مقدار) {
  llvm::json::Object CodeAction{{"title", مقدار.title}};
  اگر (مقدار.kind)
    CodeAction["kind"] = *مقدار.kind;
  اگر (مقدار.diagnostics)
    CodeAction["diagnostics"] = llvm::json::Array(*مقدار.diagnostics);
  اگر (مقدار.isPreferred)
    CodeAction["isPreferred"] = درست;
  اگر (مقدار.edit)
    CodeAction["edit"] = *مقدار.edit;
  بازگشت std::move(CodeAction);
}

//===----------------------------------------------------------------------===//
// ShowMessageParams
//===----------------------------------------------------------------------===//

llvm::json::مقدار llvm::lsp::toJSON(ثابت ShowMessageParams &Params) {
  auto Out = llvm::json::Object{
      {"type", static_cast<صحیح>(Params.type)},
      {"message", Params.message},
  };
  اگر (Params.actions)
    Out["actions"] = *Params.actions;
  بازگشت Out;
}

llvm::json::مقدار llvm::lsp::toJSON(ثابت MessageActionItem &Params) {
  بازگشت llvm::json::Object{{"title", Params.title}};
}
