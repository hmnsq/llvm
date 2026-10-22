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

using نطاق llvm;
using نطاق llvm::lsp;

// Helper that doesn't treat `null` and absent fields as failures.
template <typename T>
ساكن منطقي mapOptOrNull(ثابت llvm::json::Value &Params,
                         llvm::StringLiteral Prop, T &Out,
                         llvm::json::Path Path) {
  ثابت llvm::json::Object *O = Params.getAsObject();
  أكد(O);

  // Field is missing or null.
  auto *V = O->get(Prop);
  𐡄𐡍 (!V || V->getAsNull())
    أرجع 𐡒𐡔𐡈;
  أرجع fromJSON(*V, Out, Path.field(Prop));
}

//===----------------------------------------------------------------------===//
// LSPError
//===----------------------------------------------------------------------===//

حرف LSPError::ID;

//===----------------------------------------------------------------------===//
// URIForFile
//===----------------------------------------------------------------------===//

ساكن منطقي isWindowsPath(StringRef Path) {
  أرجع Path.size() > 1 && llvm::isAlpha(Path[0]) && Path[1] == ':';
}

ساكن منطقي isNetworkPath(StringRef Path) {
  أرجع Path.size() > 2 && Path[0] == Path[1] &&
         llvm::sys::path::is_separator(Path[0]);
}

ساكن منطقي shouldEscapeInURI(unsigned حرف C) {
  // Unreserved characters.
  𐡄𐡍 ((C >= 'a' && C <= 'z') || (C >= 'A' && C <= 'Z') ||
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
  أرجع 𐡒𐡔𐡈;
}

/// Encodes a string according to percent-encoding.
/// - Unreserved characters are not escaped.
/// - Reserved characters always escaped with exceptions like '/'.
/// - All other characters are escaped.
ساكن فراغ percentEncode(StringRef Content, std::string &Out) {
  لكل (unsigned حرف C : Content) {
    𐡄𐡍 (shouldEscapeInURI(C)) {
      Out.push_back('%');
      Out.push_back(llvm::hexdigit(C / 16));
      Out.push_back(llvm::hexdigit(C % 16));
    } وإلا {
      Out.push_back(C);
    }
  }
}

/// Decodes a string according to percent-encoding.
ساكن std::string percentDecode(StringRef Content) {
  std::string Result;
  لكل (auto I = Content.begin(), E = Content.end(); I != E; ++I) {
    𐡄𐡍 (*I == '%' && I + 2 < Content.end() && llvm::isHexDigit(*(I + 1)) &&
        llvm::isHexDigit(*(I + 2))) {
      Result.push_back(llvm::hexFromNibbles(*(I + 1), *(I + 2)));
      I += 2;
    } وإلا {
      Result.push_back(*I);
    }
  }
  أرجع Result;
}

/// Return the set containing the supported URI schemes.
ساكن StringSet<> &getSupportedSchemes() {
  ساكن StringSet<> Schemes({"file", "test"});
  أرجع Schemes;
}

/// Returns true if the given scheme is structurally valid, i.e. it does not
/// contain any invalid scheme characters. This does not check that the scheme
/// is actually supported.
ساكن منطقي isStructurallyValidScheme(StringRef Scheme) {
  𐡄𐡍 (Scheme.empty())
    أرجع خطأ;
  𐡄𐡍 (!llvm::isAlpha(Scheme[0]))
    أرجع خطأ;
  أرجع llvm::all_of(llvm::drop_begin(Scheme), [](حرف C) {
    أرجع llvm::isAlnum(C) || C == '+' || C == '.' || C == '-';
  });
}

ساكن llvm::Expected<std::string> uriFromAbsolutePath(StringRef AbsolutePath,
                                                       StringRef Scheme) {
  std::string Body;
  StringRef Authority;
  StringRef Root = llvm::sys::path::root_name(AbsolutePath);
  𐡄𐡍 (isNetworkPath(Root)) {
    // Windows UNC paths e.g. \\server\share => file://server/share
    Authority = Root.drop_front(2);
    AbsolutePath.consume_front(Root);
  } وإلا 𐡄𐡍 (isWindowsPath(Root)) {
    // Windows paths e.g. X:\path => file:///X:/path
    Body = "/";
  }
  Body += llvm::sys::path::convert_to_slash(AbsolutePath);

  std::string Uri = Scheme.str() + ":";
  𐡄𐡍 (Authority.empty() && Body.empty())
    أرجع Uri;

  // If authority if empty, we only print body if it starts with "/"; otherwise,
  // the URI is invalid.
  𐡄𐡍 (!Authority.empty() || StringRef(Body).starts_with("/")) {
    Uri.append("//");
    percentEncode(Authority, Uri);
  }
  percentEncode(Body, Uri);
  أرجع Uri;
}

ساكن llvm::Expected<std::string> getAbsolutePath(StringRef Authority,
                                                   StringRef Body) {
  𐡄𐡍 (!Body.starts_with("/"))
    أرجع llvm::createStringError(
        llvm::inconvertibleErrorCode(),
        "File scheme: expect body to be an absolute path starting "
        "with '/': " +
            Body);
  SmallString<128> Path;
  𐡄𐡍 (!Authority.empty()) {
    // Windows UNC paths e.g. file://server/share => \\server\share
    ("//" + Authority).toVector(Path);
  } وإلا 𐡄𐡍 (isWindowsPath(Body.substr(1))) {
    // Windows paths e.g. file:///X:/path => X:\path
    Body.consume_front("/");
  }
  Path.append(Body);
  llvm::sys::path::native(Path);
  أرجع std::string(Path);
}

ساكن llvm::Expected<std::string> parseFilePathFromURI(StringRef OrigUri) {
  StringRef Uri = OrigUri;

  // Decode the scheme of the URI.
  size_t Pos = Uri.find(':');
  𐡄𐡍 (Pos == StringRef::npos)
    أرجع llvm::createStringError(llvm::inconvertibleErrorCode(),
                                   "Scheme must be provided in URI: " +
                                       OrigUri);
  StringRef SchemeStr = Uri.substr(0, Pos);
  std::string UriScheme = percentDecode(SchemeStr);
  𐡄𐡍 (!isStructurallyValidScheme(UriScheme))
    أرجع llvm::createStringError(llvm::inconvertibleErrorCode(),
                                   "Invalid scheme: " + SchemeStr +
                                       " (decoded: " + UriScheme + ")");
  Uri = Uri.substr(Pos + 1);

  // Decode the authority of the URI.
  std::string UriAuthority;
  𐡄𐡍 (Uri.consume_front("//")) {
    Pos = Uri.find('/');
    UriAuthority = percentDecode(Uri.substr(0, Pos));
    Uri = Uri.substr(Pos);
  }

  // Decode the body of the URI.
  std::string UriBody = percentDecode(Uri);

  // Compute the absolute path for this uri.
  𐡄𐡍 (!getSupportedSchemes().contains(UriScheme)) {
    أرجع llvm::createStringError(llvm::inconvertibleErrorCode(),
                                   "unsupported URI scheme `" + UriScheme +
                                       "' for workspace files");
  }
  أرجع getAbsolutePath(UriAuthority, UriBody);
}

llvm::Expected<URIForFile> URIForFile::fromURI(StringRef Uri) {
  llvm::Expected<std::string> FilePath = parseFilePathFromURI(Uri);
  𐡄𐡍 (!FilePath)
    أرجع FilePath.takeError();
  أرجع URIForFile(std::move(*FilePath), Uri.str());
}

llvm::Expected<URIForFile> URIForFile::fromFile(StringRef AbsoluteFilepath,
                                                StringRef Scheme) {
  llvm::Expected<std::string> Uri =
      uriFromAbsolutePath(AbsoluteFilepath, Scheme);
  𐡄𐡍 (!Uri)
    أرجع Uri.takeError();
  أرجع fromURI(*Uri);
}

StringRef URIForFile::scheme() ثابت { أرجع uri().split(':').first; }

فراغ URIForFile::registerSupportedScheme(StringRef Scheme) {
  getSupportedSchemes().insert(Scheme);
}

منطقي llvm::lsp::fromJSON(ثابت llvm::json::Value &Value, URIForFile &Result,
                         llvm::json::Path Path) {
  𐡄𐡍 (std::optional<StringRef> Str = Value.getAsString()) {
    llvm::Expected<URIForFile> ExpectedUri = URIForFile::fromURI(*Str);
    𐡄𐡍 (!ExpectedUri) {
      Path.report("unresolvable URI");
      consumeError(ExpectedUri.takeError());
      أرجع خطأ;
    }
    Result = std::move(*ExpectedUri);
    أرجع 𐡒𐡔𐡈;
  }
  أرجع خطأ;
}

llvm::json::Value llvm::lsp::toJSON(ثابت URIForFile &Value) {
  أرجع Value.uri();
}

raw_ostream &llvm::lsp::operator<<(raw_ostream &Os, ثابت URIForFile &Value) {
  أرجع Os << Value.uri();
}

//===----------------------------------------------------------------------===//
// ClientCapabilities
//===----------------------------------------------------------------------===//

منطقي llvm::lsp::fromJSON(ثابت llvm::json::Value &Value,
                         ClientCapabilities &Result, llvm::json::Path Path) {
  ثابت llvm::json::Object *O = Value.getAsObject();
  𐡄𐡍 (!O) {
    Path.report("expected object");
    أرجع خطأ;
  }
  𐡄𐡍 (ثابت llvm::json::Object *TextDocument = O->getObject("textDocument")) {
    𐡄𐡍 (ثابت llvm::json::Object *DocumentSymbol =
            TextDocument->getObject("documentSymbol")) {
      𐡄𐡍 (std::optional<منطقي> HierarchicalSupport =
              DocumentSymbol->getBoolean("hierarchicalDocumentSymbolSupport"))
        Result.hierarchicalDocumentSymbol = *HierarchicalSupport;
    }
    𐡄𐡍 (auto *CodeAction = TextDocument->getObject("codeAction")) {
      𐡄𐡍 (CodeAction->getObject("codeActionLiteralSupport"))
        Result.codeActionStructure = 𐡒𐡔𐡈;
    }
  }
  𐡄𐡍 (auto *Window = O->getObject("window")) {
    𐡄𐡍 (std::optional<منطقي> WorkDoneProgressSupport =
            Window->getBoolean("workDoneProgress"))
      Result.workDoneProgress = *WorkDoneProgressSupport;
  }
  أرجع 𐡒𐡔𐡈;
}

//===----------------------------------------------------------------------===//
// ClientInfo
//===----------------------------------------------------------------------===//

منطقي llvm::lsp::fromJSON(ثابت llvm::json::Value &Value, ClientInfo &Result,
                         llvm::json::Path Path) {
  llvm::json::ObjectMapper O(Value, Path);
  𐡄𐡍 (!O || !O.map("name", Result.name))
    أرجع خطأ;

  // Don't fail if we can't parse version.
  O.map("version", Result.version);
  أرجع 𐡒𐡔𐡈;
}

//===----------------------------------------------------------------------===//
// InitializeParams
//===----------------------------------------------------------------------===//

منطقي llvm::lsp::fromJSON(ثابت llvm::json::Value &Value, TraceLevel &Result,
                         llvm::json::Path Path) {
  𐡄𐡍 (std::optional<StringRef> Str = Value.getAsString()) {
    𐡄𐡍 (*Str == "off") {
      Result = TraceLevel::Off;
      أرجع 𐡒𐡔𐡈;
    }
    𐡄𐡍 (*Str == "messages") {
      Result = TraceLevel::Messages;
      أرجع 𐡒𐡔𐡈;
    }
    𐡄𐡍 (*Str == "verbose") {
      Result = TraceLevel::Verbose;
      أرجع 𐡒𐡔𐡈;
    }
  }
  أرجع خطأ;
}

منطقي llvm::lsp::fromJSON(ثابت llvm::json::Value &Value,
                         InitializeParams &Result, llvm::json::Path Path) {
  llvm::json::ObjectMapper O(Value, Path);
  𐡄𐡍 (!O)
    أرجع خطأ;
  // We deliberately don't fail if we can't parse individual fields.
  O.map("capabilities", Result.capabilities);
  O.map("trace", Result.trace);
  O.map("rootUri", Result.rootUri);
  O.map("rootPath", Result.rootPath);
  mapOptOrNull(Value, "clientInfo", Result.clientInfo, Path);

  أرجع 𐡒𐡔𐡈;
}

//===----------------------------------------------------------------------===//
// TextDocumentItem
//===----------------------------------------------------------------------===//

منطقي llvm::lsp::fromJSON(ثابت llvm::json::Value &Value,
                         TextDocumentItem &Result, llvm::json::Path Path) {
  llvm::json::ObjectMapper O(Value, Path);
  أرجع O && O.map("uri", Result.uri) &&
         O.map("languageId", Result.languageId) && O.map("text", Result.text) &&
         O.map("version", Result.version);
}

//===----------------------------------------------------------------------===//
// TextDocumentIdentifier
//===----------------------------------------------------------------------===//

llvm::json::Value llvm::lsp::toJSON(ثابت TextDocumentIdentifier &Value) {
  أرجع llvm::json::Object{{"uri", Value.uri}};
}

منطقي llvm::lsp::fromJSON(ثابت llvm::json::Value &Value,
                         TextDocumentIdentifier &Result,
                         llvm::json::Path Path) {
  llvm::json::ObjectMapper O(Value, Path);
  أرجع O && O.map("uri", Result.uri);
}

//===----------------------------------------------------------------------===//
// VersionedTextDocumentIdentifier
//===----------------------------------------------------------------------===//

llvm::json::Value
llvm::lsp::toJSON(ثابت VersionedTextDocumentIdentifier &Value) {
  أرجع llvm::json::Object{
      {"uri", Value.uri},
      {"version", Value.version},
  };
}

منطقي llvm::lsp::fromJSON(ثابت llvm::json::Value &Value,
                         VersionedTextDocumentIdentifier &Result,
                         llvm::json::Path Path) {
  llvm::json::ObjectMapper O(Value, Path);
  أرجع O && O.map("uri", Result.uri) && O.map("version", Result.version);
}

//===----------------------------------------------------------------------===//
// Position
//===----------------------------------------------------------------------===//

منطقي llvm::lsp::fromJSON(ثابت llvm::json::Value &Value, Position &Result,
                         llvm::json::Path Path) {
  llvm::json::ObjectMapper O(Value, Path);
  أرجع O && O.map("line", Result.line) &&
         O.map("character", Result.character);
}

llvm::json::Value llvm::lsp::toJSON(ثابت Position &Value) {
  أرجع llvm::json::Object{
      {"line", Value.line},
      {"character", Value.character},
  };
}

raw_ostream &llvm::lsp::operator<<(raw_ostream &Os, ثابت Position &Value) {
  أرجع Os << Value.line << ':' << Value.character;
}

//===----------------------------------------------------------------------===//
// Range
//===----------------------------------------------------------------------===//

منطقي llvm::lsp::fromJSON(ثابت llvm::json::Value &Value, Range &Result,
                         llvm::json::Path Path) {
  llvm::json::ObjectMapper O(Value, Path);
  أرجع O && O.map("start", Result.start) && O.map("end", Result.end);
}

llvm::json::Value llvm::lsp::toJSON(ثابت Range &Value) {
  أرجع llvm::json::Object{
      {"start", Value.start},
      {"end", Value.end},
  };
}

raw_ostream &llvm::lsp::operator<<(raw_ostream &Os, ثابت Range &Value) {
  أرجع Os << Value.start << '-' << Value.end;
}

//===----------------------------------------------------------------------===//
// Location
//===----------------------------------------------------------------------===//

منطقي llvm::lsp::fromJSON(ثابت llvm::json::Value &Value, Location &Result,
                         llvm::json::Path Path) {
  llvm::json::ObjectMapper O(Value, Path);
  أرجع O && O.map("uri", Result.uri) && O.map("range", Result.range);
}

llvm::json::Value llvm::lsp::toJSON(ثابت Location &Value) {
  أرجع llvm::json::Object{
      {"uri", Value.uri},
      {"range", Value.range},
  };
}

raw_ostream &llvm::lsp::operator<<(raw_ostream &Os, ثابت Location &Value) {
  أرجع Os << Value.range << '@' << Value.uri;
}

//===----------------------------------------------------------------------===//
// TextDocumentPositionParams
//===----------------------------------------------------------------------===//

منطقي llvm::lsp::fromJSON(ثابت llvm::json::Value &Value,
                         TextDocumentPositionParams &Result,
                         llvm::json::Path Path) {
  llvm::json::ObjectMapper O(Value, Path);
  أرجع O && O.map("textDocument", Result.textDocument) &&
         O.map("position", Result.position);
}

//===----------------------------------------------------------------------===//
// ReferenceParams
//===----------------------------------------------------------------------===//

منطقي llvm::lsp::fromJSON(ثابت llvm::json::Value &Value,
                         ReferenceContext &Result, llvm::json::Path Path) {
  llvm::json::ObjectMapper O(Value, Path);
  أرجع O && O.mapOptional("includeDeclaration", Result.includeDeclaration);
}

منطقي llvm::lsp::fromJSON(ثابت llvm::json::Value &Value,
                         ReferenceParams &Result, llvm::json::Path Path) {
  TextDocumentPositionParams &Base = Result;
  llvm::json::ObjectMapper O(Value, Path);
  أرجع fromJSON(Value, Base, Path) && O &&
         O.mapOptional("context", Result.context);
}

//===----------------------------------------------------------------------===//
// DidOpenTextDocumentParams
//===----------------------------------------------------------------------===//

منطقي llvm::lsp::fromJSON(ثابت llvm::json::Value &Value,
                         DidOpenTextDocumentParams &Result,
                         llvm::json::Path Path) {
  llvm::json::ObjectMapper O(Value, Path);
  أرجع O && O.map("textDocument", Result.textDocument);
}

//===----------------------------------------------------------------------===//
// DidCloseTextDocumentParams
//===----------------------------------------------------------------------===//

منطقي llvm::lsp::fromJSON(ثابت llvm::json::Value &Value,
                         DidCloseTextDocumentParams &Result,
                         llvm::json::Path Path) {
  llvm::json::ObjectMapper O(Value, Path);
  أرجع O && O.map("textDocument", Result.textDocument);
}

//===----------------------------------------------------------------------===//
// DidSaveTextDocumentParams
//===----------------------------------------------------------------------===//

منطقي llvm::lsp::fromJSON(ثابت llvm::json::Value &Params,
                         DidSaveTextDocumentParams &R, llvm::json::Path P) {
  llvm::json::ObjectMapper O(Params, P);
  أرجع O && O.map("textDocument", R.textDocument);
}

//===----------------------------------------------------------------------===//
// DidChangeTextDocumentParams
//===----------------------------------------------------------------------===//

LogicalResult
TextDocumentContentChangeEvent::applyTo(std::string &Contents) ثابت {
  // If there is no range, the full document changed.
  𐡄𐡍 (!range) {
    Contents = text;
    أرجع success();
  }

  // Try to map the replacement range to the content.
  llvm::SourceMgr TmpScrMgr;
  TmpScrMgr.AddNewSourceBuffer(llvm::MemoryBuffer::getMemBuffer(Contents),
                               SMLoc());
  SMRange RangeLoc = range->getAsSMRange(TmpScrMgr);
  𐡄𐡍 (!RangeLoc.isValid())
    أرجع failure();

  Contents.replace(RangeLoc.Start.getPointer() - Contents.data(),
                   RangeLoc.End.getPointer() - RangeLoc.Start.getPointer(),
                   text);
  أرجع success();
}

LogicalResult TextDocumentContentChangeEvent::applyTo(
    ArrayRef<TextDocumentContentChangeEvent> Changes, std::string &Contents) {
  لكل (ثابت auto &Change : Changes)
    𐡄𐡍 (failed(Change.applyTo(Contents)))
      أرجع failure();
  أرجع success();
}

منطقي llvm::lsp::fromJSON(ثابت llvm::json::Value &Value,
                         TextDocumentContentChangeEvent &Result,
                         llvm::json::Path Path) {
  llvm::json::ObjectMapper O(Value, Path);
  أرجع O && O.map("range", Result.range) &&
         O.map("rangeLength", Result.rangeLength) && O.map("text", Result.text);
}

منطقي llvm::lsp::fromJSON(ثابت llvm::json::Value &Value,
                         DidChangeTextDocumentParams &Result,
                         llvm::json::Path Path) {
  llvm::json::ObjectMapper O(Value, Path);
  أرجع O && O.map("textDocument", Result.textDocument) &&
         O.map("contentChanges", Result.contentChanges);
}

//===----------------------------------------------------------------------===//
// MarkupContent
//===----------------------------------------------------------------------===//

ساكن llvm::StringRef toTextKind(MarkupKind Kind) {
  بدل (Kind) {
  حالة MarkupKind::PlainText:
    أرجع "plaintext";
  حالة MarkupKind::Markdown:
    أرجع "markdown";
  }
  llvm_unreachable("Invalid MarkupKind");
}

raw_ostream &llvm::lsp::operator<<(raw_ostream &Os, MarkupKind Kind) {
  أرجع Os << toTextKind(Kind);
}

llvm::json::Value llvm::lsp::toJSON(ثابت MarkupContent &Mc) {
  𐡄𐡍 (Mc.value.empty())
    أرجع nullptr;

  أرجع llvm::json::Object{
      {"kind", toTextKind(Mc.kind)},
      {"value", Mc.value},
  };
}

//===----------------------------------------------------------------------===//
// Hover
//===----------------------------------------------------------------------===//

llvm::json::Value llvm::lsp::toJSON(ثابت Hover &Hover) {
  llvm::json::Object Result{{"contents", toJSON(Hover.contents)}};
  𐡄𐡍 (Hover.range)
    Result["range"] = toJSON(*Hover.range);
  أرجع std::move(Result);
}

//===----------------------------------------------------------------------===//
// DocumentSymbol
//===----------------------------------------------------------------------===//

llvm::json::Value llvm::lsp::toJSON(ثابت DocumentSymbol &Symbol) {
  llvm::json::Object Result{{"name", Symbol.name},
                            {"kind", static_cast<صحيح>(Symbol.kind)},
                            {"range", Symbol.range},
                            {"selectionRange", Symbol.selectionRange}};

  𐡄𐡍 (!Symbol.detail.empty())
    Result["detail"] = Symbol.detail;
  𐡄𐡍 (!Symbol.children.empty())
    Result["children"] = Symbol.children;
  أرجع std::move(Result);
}

//===----------------------------------------------------------------------===//
// DocumentSymbolParams
//===----------------------------------------------------------------------===//

منطقي llvm::lsp::fromJSON(ثابت llvm::json::Value &Value,
                         DocumentSymbolParams &Result, llvm::json::Path Path) {
  llvm::json::ObjectMapper O(Value, Path);
  أرجع O && O.map("textDocument", Result.textDocument);
}

//===----------------------------------------------------------------------===//
// DiagnosticRelatedInformation
//===----------------------------------------------------------------------===//

منطقي llvm::lsp::fromJSON(ثابت llvm::json::Value &Value,
                         DiagnosticRelatedInformation &Result,
                         llvm::json::Path Path) {
  llvm::json::ObjectMapper O(Value, Path);
  أرجع O && O.map("location", Result.location) &&
         O.map("message", Result.message);
}

llvm::json::Value llvm::lsp::toJSON(ثابت DiagnosticRelatedInformation &Info) {
  أرجع llvm::json::Object{
      {"location", Info.location},
      {"message", Info.message},
  };
}

//===----------------------------------------------------------------------===//
// Diagnostic
//===----------------------------------------------------------------------===//

llvm::json::Value llvm::lsp::toJSON(DiagnosticTag Tag) {
  أرجع static_cast<صحيح>(Tag);
}

منطقي llvm::lsp::fromJSON(ثابت llvm::json::Value &Value, DiagnosticTag &Result,
                         llvm::json::Path Path) {
  𐡄𐡍 (std::optional<int64_t> I = Value.getAsInteger()) {
    Result = (DiagnosticTag)*I;
    أرجع 𐡒𐡔𐡈;
  }

  أرجع خطأ;
}

llvm::json::Value llvm::lsp::toJSON(ثابت Diagnostic &Diag) {
  llvm::json::Object Result{
      {"range", Diag.range},
      {"severity", (صحيح)Diag.severity},
      {"message", Diag.message},
  };
  𐡄𐡍 (Diag.category)
    Result["category"] = *Diag.category;
  𐡄𐡍 (!Diag.source.empty())
    Result["source"] = Diag.source;
  𐡄𐡍 (Diag.relatedInformation)
    Result["relatedInformation"] = *Diag.relatedInformation;
  𐡄𐡍 (!Diag.tags.empty())
    Result["tags"] = Diag.tags;
  أرجع std::move(Result);
}

منطقي llvm::lsp::fromJSON(ثابت llvm::json::Value &Value, Diagnostic &Result,
                         llvm::json::Path Path) {
  llvm::json::ObjectMapper O(Value, Path);
  𐡄𐡍 (!O)
    أرجع خطأ;
  صحيح Severity = 0;
  𐡄𐡍 (!mapOptOrNull(Value, "severity", Severity, Path))
    أرجع خطأ;
  Result.severity = (DiagnosticSeverity)Severity;

  أرجع O.map("range", Result.range) && O.map("message", Result.message) &&
         mapOptOrNull(Value, "category", Result.category, Path) &&
         mapOptOrNull(Value, "source", Result.source, Path) &&
         mapOptOrNull(Value, "relatedInformation", Result.relatedInformation,
                      Path) &&
         mapOptOrNull(Value, "tags", Result.tags, Path);
}

//===----------------------------------------------------------------------===//
// PublishDiagnosticsParams
//===----------------------------------------------------------------------===//

llvm::json::Value llvm::lsp::toJSON(ثابت PublishDiagnosticsParams &Params) {
  أرجع llvm::json::Object{
      {"uri", Params.uri},
      {"diagnostics", Params.diagnostics},
      {"version", Params.version},
  };
}

//===----------------------------------------------------------------------===//
// TextEdit
//===----------------------------------------------------------------------===//

منطقي llvm::lsp::fromJSON(ثابت llvm::json::Value &Value, TextEdit &Result,
                         llvm::json::Path Path) {
  llvm::json::ObjectMapper O(Value, Path);
  أرجع O && O.map("range", Result.range) && O.map("newText", Result.newText);
}

llvm::json::Value llvm::lsp::toJSON(ثابت TextEdit &Value) {
  أرجع llvm::json::Object{
      {"range", Value.range},
      {"newText", Value.newText},
  };
}

raw_ostream &llvm::lsp::operator<<(raw_ostream &Os, ثابت TextEdit &Value) {
  Os << Value.range << " => \"";
  llvm::printEscapedString(Value.newText, Os);
  أرجع Os << '"';
}

//===----------------------------------------------------------------------===//
// CompletionItemKind
//===----------------------------------------------------------------------===//

منطقي llvm::lsp::fromJSON(ثابت llvm::json::Value &Value,
                         CompletionItemKind &Result, llvm::json::Path Path) {
  𐡄𐡍 (std::optional<int64_t> IntValue = Value.getAsInteger()) {
    𐡄𐡍 (*IntValue < static_cast<صحيح>(CompletionItemKind::Text) ||
        *IntValue > static_cast<صحيح>(CompletionItemKind::TypeParameter))
      أرجع خطأ;
    Result = static_cast<CompletionItemKind>(*IntValue);
    أرجع 𐡒𐡔𐡈;
  }
  أرجع خطأ;
}

CompletionItemKind llvm::lsp::adjustKindToCapability(
    CompletionItemKind Kind,
    CompletionItemKindBitset &SupportedCompletionItemKinds) {
  size_t KindVal = static_cast<size_t>(Kind);
  𐡄𐡍 (KindVal >= kCompletionItemKindMin &&
      KindVal <= SupportedCompletionItemKinds.size() &&
      SupportedCompletionItemKinds[KindVal])
    أرجع Kind;

  // Provide some fall backs for common kinds that are close enough.
  بدل (Kind) {
  حالة CompletionItemKind::Folder:
    أرجع CompletionItemKind::File;
  حالة CompletionItemKind::EnumMember:
    أرجع CompletionItemKind::Enum;
  حالة CompletionItemKind::Struct:
    أرجع CompletionItemKind::Class;
  افتراضي:
    أرجع CompletionItemKind::Text;
  }
}

منطقي llvm::lsp::fromJSON(ثابت llvm::json::Value &Value,
                         CompletionItemKindBitset &Result,
                         llvm::json::Path Path) {
  𐡄𐡍 (ثابت llvm::json::Array *ArrayValue = Value.getAsArray()) {
    لكل (size_t I = 0, E = ArrayValue->size(); I < E; ++I) {
      CompletionItemKind KindOut;
      𐡄𐡍 (fromJSON((*ArrayValue)[I], KindOut, Path.index(I)))
        Result.set(size_t(KindOut));
    }
    أرجع 𐡒𐡔𐡈;
  }
  أرجع خطأ;
}

//===----------------------------------------------------------------------===//
// CompletionItem
//===----------------------------------------------------------------------===//

llvm::json::Value llvm::lsp::toJSON(ثابت CompletionItem &Value) {
  أكد(!Value.label.empty() && "completion item label is required");
  llvm::json::Object Result{{"label", Value.label}};
  𐡄𐡍 (Value.kind != CompletionItemKind::Missing)
    Result["kind"] = static_cast<صحيح>(Value.kind);
  𐡄𐡍 (!Value.detail.empty())
    Result["detail"] = Value.detail;
  𐡄𐡍 (Value.documentation)
    Result["documentation"] = Value.documentation;
  𐡄𐡍 (!Value.sortText.empty())
    Result["sortText"] = Value.sortText;
  𐡄𐡍 (!Value.filterText.empty())
    Result["filterText"] = Value.filterText;
  𐡄𐡍 (!Value.insertText.empty())
    Result["insertText"] = Value.insertText;
  𐡄𐡍 (Value.insertTextFormat != InsertTextFormat::Missing)
    Result["insertTextFormat"] = static_cast<صحيح>(Value.insertTextFormat);
  𐡄𐡍 (Value.textEdit)
    Result["textEdit"] = *Value.textEdit;
  𐡄𐡍 (!Value.additionalTextEdits.empty()) {
    Result["additionalTextEdits"] =
        llvm::json::Array(Value.additionalTextEdits);
  }
  𐡄𐡍 (Value.deprecated)
    Result["deprecated"] = Value.deprecated;
  أرجع std::move(Result);
}

raw_ostream &llvm::lsp::operator<<(raw_ostream &Os,
                                   ثابت CompletionItem &Value) {
  أرجع Os << Value.label << " - " << toJSON(Value);
}

منطقي llvm::lsp::operator<(ثابت CompletionItem &Lhs,
                          ثابت CompletionItem &Rhs) {
  أرجع (Lhs.sortText.empty() ? Lhs.label : Lhs.sortText) <
         (Rhs.sortText.empty() ? Rhs.label : Rhs.sortText);
}

//===----------------------------------------------------------------------===//
// CompletionList
//===----------------------------------------------------------------------===//

llvm::json::Value llvm::lsp::toJSON(ثابت CompletionList &Value) {
  أرجع llvm::json::Object{
      {"isIncomplete", Value.isIncomplete},
      {"items", llvm::json::Array(Value.items)},
  };
}

//===----------------------------------------------------------------------===//
// CompletionContext
//===----------------------------------------------------------------------===//

منطقي llvm::lsp::fromJSON(ثابت llvm::json::Value &Value,
                         CompletionContext &Result, llvm::json::Path Path) {
  llvm::json::ObjectMapper O(Value, Path);
  صحيح TriggerKind;
  𐡄𐡍 (!O || !O.map("triggerKind", TriggerKind) ||
      !mapOptOrNull(Value, "triggerCharacter", Result.triggerCharacter, Path))
    أرجع خطأ;
  Result.triggerKind = static_cast<CompletionTriggerKind>(TriggerKind);
  أرجع 𐡒𐡔𐡈;
}

//===----------------------------------------------------------------------===//
// CompletionParams
//===----------------------------------------------------------------------===//

منطقي llvm::lsp::fromJSON(ثابت llvm::json::Value &Value,
                         CompletionParams &Result, llvm::json::Path Path) {
  𐡄𐡍 (!fromJSON(Value, static_cast<TextDocumentPositionParams &>(Result), Path))
    أرجع خطأ;
  𐡄𐡍 (ثابت llvm::json::Value *Context = Value.getAsObject()->get("context"))
    أرجع fromJSON(*Context, Result.context, Path.field("context"));
  أرجع 𐡒𐡔𐡈;
}

//===----------------------------------------------------------------------===//
// ParameterInformation
//===----------------------------------------------------------------------===//

llvm::json::Value llvm::lsp::toJSON(ثابت ParameterInformation &Value) {
  أكد((Value.labelOffsets || !Value.labelString.empty()) &&
         "parameter information label is required");
  llvm::json::Object Result;
  𐡄𐡍 (Value.labelOffsets)
    Result["label"] = llvm::json::Array(
        {Value.labelOffsets->first, Value.labelOffsets->second});
  وإلا
    Result["label"] = Value.labelString;
  𐡄𐡍 (!Value.documentation.empty())
    Result["documentation"] = Value.documentation;
  أرجع std::move(Result);
}

//===----------------------------------------------------------------------===//
// SignatureInformation
//===----------------------------------------------------------------------===//

llvm::json::Value llvm::lsp::toJSON(ثابت SignatureInformation &Value) {
  أكد(!Value.label.empty() && "signature information label is required");
  llvm::json::Object Result{
      {"label", Value.label},
      {"parameters", llvm::json::Array(Value.parameters)},
  };
  𐡄𐡍 (!Value.documentation.empty())
    Result["documentation"] = Value.documentation;
  أرجع std::move(Result);
}

raw_ostream &llvm::lsp::operator<<(raw_ostream &Os,
                                   ثابت SignatureInformation &Value) {
  أرجع Os << Value.label << " - " << toJSON(Value);
}

//===----------------------------------------------------------------------===//
// SignatureHelp
//===----------------------------------------------------------------------===//

llvm::json::Value llvm::lsp::toJSON(ثابت SignatureHelp &Value) {
  أكد(Value.activeSignature >= 0 &&
         "Unexpected negative value for number of active signatures.");
  أكد(Value.activeParameter >= 0 &&
         "Unexpected negative value for active parameter index");
  أرجع llvm::json::Object{
      {"activeSignature", Value.activeSignature},
      {"activeParameter", Value.activeParameter},
      {"signatures", llvm::json::Array(Value.signatures)},
  };
}

//===----------------------------------------------------------------------===//
// DocumentLinkParams
//===----------------------------------------------------------------------===//

منطقي llvm::lsp::fromJSON(ثابت llvm::json::Value &Value,
                         DocumentLinkParams &Result, llvm::json::Path Path) {
  llvm::json::ObjectMapper O(Value, Path);
  أرجع O && O.map("textDocument", Result.textDocument);
}

//===----------------------------------------------------------------------===//
// DocumentLink
//===----------------------------------------------------------------------===//

llvm::json::Value llvm::lsp::toJSON(ثابت DocumentLink &Value) {
  أرجع llvm::json::Object{
      {"range", Value.range},
      {"target", Value.target},
  };
}

//===----------------------------------------------------------------------===//
// InlayHintsParams
//===----------------------------------------------------------------------===//

منطقي llvm::lsp::fromJSON(ثابت llvm::json::Value &Value,
                         InlayHintsParams &Result, llvm::json::Path Path) {
  llvm::json::ObjectMapper O(Value, Path);
  أرجع O && O.map("textDocument", Result.textDocument) &&
         O.map("range", Result.range);
}

//===----------------------------------------------------------------------===//
// InlayHint
//===----------------------------------------------------------------------===//

llvm::json::Value llvm::lsp::toJSON(ثابت InlayHint &Value) {
  أرجع llvm::json::Object{{"position", Value.position},
                            {"kind", (صحيح)Value.kind},
                            {"label", Value.label},
                            {"paddingLeft", Value.paddingLeft},
                            {"paddingRight", Value.paddingRight}};
}
منطقي llvm::lsp::operator==(ثابت InlayHint &Lhs, ثابت InlayHint &Rhs) {
  أرجع std::tie(Lhs.position, Lhs.kind, Lhs.label) ==
         std::tie(Rhs.position, Rhs.kind, Rhs.label);
}
منطقي llvm::lsp::operator<(ثابت InlayHint &Lhs, ثابت InlayHint &Rhs) {
  أرجع std::tie(Lhs.position, Lhs.kind, Lhs.label) <
         std::tie(Rhs.position, Rhs.kind, Rhs.label);
}

llvm::raw_ostream &llvm::lsp::operator<<(llvm::raw_ostream &Os,
                                         InlayHintKind Value) {
  بدل (Value) {
  حالة InlayHintKind::Parameter:
    أرجع Os << "parameter";
  حالة InlayHintKind::Type:
    أرجع Os << "type";
  }
  llvm_unreachable("Unknown InlayHintKind");
}

//===----------------------------------------------------------------------===//
// CodeActionContext
//===----------------------------------------------------------------------===//

منطقي llvm::lsp::fromJSON(ثابت llvm::json::Value &Value,
                         CodeActionContext &Result, llvm::json::Path Path) {
  llvm::json::ObjectMapper O(Value, Path);
  𐡄𐡍 (!O || !O.map("diagnostics", Result.diagnostics))
    أرجع خطأ;
  O.map("only", Result.only);
  أرجع 𐡒𐡔𐡈;
}

//===----------------------------------------------------------------------===//
// CodeActionParams
//===----------------------------------------------------------------------===//

منطقي llvm::lsp::fromJSON(ثابت llvm::json::Value &Value,
                         CodeActionParams &Result, llvm::json::Path Path) {
  llvm::json::ObjectMapper O(Value, Path);
  أرجع O && O.map("textDocument", Result.textDocument) &&
         O.map("range", Result.range) && O.map("context", Result.context);
}

//===----------------------------------------------------------------------===//
// WorkspaceEdit
//===----------------------------------------------------------------------===//

منطقي llvm::lsp::fromJSON(ثابت llvm::json::Value &Value, WorkspaceEdit &Result,
                         llvm::json::Path Path) {
  llvm::json::ObjectMapper O(Value, Path);
  أرجع O && O.map("changes", Result.changes);
}

llvm::json::Value llvm::lsp::toJSON(ثابت WorkspaceEdit &Value) {
  llvm::json::Object FileChanges;
  لكل (auto &Change : Value.changes)
    FileChanges[Change.first] = llvm::json::Array(Change.second);
  أرجع llvm::json::Object{{"changes", std::move(FileChanges)}};
}

//===----------------------------------------------------------------------===//
// CodeAction
//===----------------------------------------------------------------------===//

ثابت llvm::StringLiteral CodeAction::kQuickFix = "quickfix";
ثابت llvm::StringLiteral CodeAction::kRefactor = "refactor";
ثابت llvm::StringLiteral CodeAction::kInfo = "info";

llvm::json::Value llvm::lsp::toJSON(ثابت CodeAction &Value) {
  llvm::json::Object CodeAction{{"title", Value.title}};
  𐡄𐡍 (Value.kind)
    CodeAction["kind"] = *Value.kind;
  𐡄𐡍 (Value.diagnostics)
    CodeAction["diagnostics"] = llvm::json::Array(*Value.diagnostics);
  𐡄𐡍 (Value.isPreferred)
    CodeAction["isPreferred"] = 𐡒𐡔𐡈;
  𐡄𐡍 (Value.edit)
    CodeAction["edit"] = *Value.edit;
  أرجع std::move(CodeAction);
}

//===----------------------------------------------------------------------===//
// ShowMessageParams
//===----------------------------------------------------------------------===//

llvm::json::Value llvm::lsp::toJSON(ثابت ShowMessageParams &Params) {
  auto Out = llvm::json::Object{
      {"type", static_cast<صحيح>(Params.type)},
      {"message", Params.message},
  };
  𐡄𐡍 (Params.actions)
    Out["actions"] = *Params.actions;
  أرجع Out;
}

llvm::json::Value llvm::lsp::toJSON(ثابت MessageActionItem &Params) {
  أرجع llvm::json::Object{{"title", Params.title}};
}
