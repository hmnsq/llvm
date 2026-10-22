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
ساكن منطقي mapOptOrNull(ثابت llvm::json::بەها &Params,
                         llvm::StringLiteral Prop, T &Out,
                         llvm::json::Path Path) {
  ثابت llvm::json::Object *O = Params.getAsObject();
  أكد(O);

  // Field is missing or null.
  auto *V = O->get(Prop);
  إذا (!V || V->getAsNull())
    أرجع true;
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
  أرجع true;
}

/// Encodes a string according to percent-encoding.
/// - Unreserved characters are not escaped.
/// - Reserved characters always escaped with exceptions like '/'.
/// - All other characters are escaped.
ساكن فراغ percentEncode(StringRef Content, std::string &Out) {
  لكل (unsigned حرف C : Content) {
    إذا (shouldEscapeInURI(C)) {
      Out.پاڵنان_back('%');
      Out.پاڵنان_back(llvm::hexdigit(C / 16));
      Out.پاڵنان_back(llvm::hexdigit(C % 16));
    } وإلا {
      Out.پاڵنان_back(C);
    }
  }
}

/// Decodes a string according to percent-encoding.
ساكن std::string percentDecode(StringRef Content) {
  std::string Result;
  لكل (auto I = Content.begin(), E = Content.end(); I != E; ++I) {
    إذا (*I == '%' && I + 2 < Content.end() && llvm::isHexDigit(*(I + 1)) &&
        llvm::isHexDigit(*(I + 2))) {
      Result.پاڵنان_back(llvm::hexFromNibbles(*(I + 1), *(I + 2)));
      I += 2;
    } وإلا {
      Result.پاڵنان_back(*I);
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
  إذا (Scheme.empty())
    أرجع خطأ;
  إذا (!llvm::isAlpha(Scheme[0]))
    أرجع خطأ;
  أرجع llvm::all_of(llvm::drop_begin(Scheme), [](حرف C) {
    أرجع llvm::isAlnum(C) || C == '+' || C == '.' || C == '-';
  });
}

ساكن llvm::Expected<std::string> uriFromAbsolutePath(StringRef AbsolutePath,
                                                       StringRef Scheme) {
  std::string Body;
  StringRef Authority;
  StringRef ڕەگ_2 = llvm::sys::path::ڕەگ_name(AbsolutePath);
  إذا (isNetworkPath(ڕەگ_2)) {
    // Windows UNC paths e.g. \\server\share => file://server/share
    Authority = ڕەگ_2.drop_front(2);
    AbsolutePath.consume_front(ڕەگ_2);
  } وإلا إذا (isWindowsPath(ڕەگ_2)) {
    // Windows paths e.g. X:\path => file:///X:/path
    Body = "/";
  }
  Body += llvm::sys::path::convert_to_slash(AbsolutePath);

  std::string Uri = Scheme.str() + ":";
  إذا (Authority.empty() && Body.empty())
    أرجع Uri;

  // If authority if empty, we only print body if it starts with "/"; otherwise,
  // the URI is invalid.
  إذا (!Authority.empty() || StringRef(Body).starts_with("/")) {
    Uri.append("//");
    percentEncode(Authority, Uri);
  }
  percentEncode(Body, Uri);
  أرجع Uri;
}

ساكن llvm::Expected<std::string> getAbsolutePath(StringRef Authority,
                                                   StringRef Body) {
  إذا (!Body.starts_with("/"))
    أرجع llvm::createStringError(
        llvm::inconvertibleErrorCode(),
        "File scheme: expect body to be an absolute path starting "
        "with '/': " +
            Body);
  SmallString<128> Path;
  إذا (!Authority.empty()) {
    // Windows UNC paths e.g. file://server/share => \\server\share
    ("//" + Authority).toVector(Path);
  } وإلا إذا (isWindowsPath(Body.substr(1))) {
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
  إذا (Pos == StringRef::npos)
    أرجع llvm::createStringError(llvm::inconvertibleErrorCode(),
                                   "Scheme must be provided in URI: " +
                                       OrigUri);
  StringRef SchemeStr = Uri.substr(0, Pos);
  std::string UriScheme = percentDecode(SchemeStr);
  إذا (!isStructurallyValidScheme(UriScheme))
    أرجع llvm::createStringError(llvm::inconvertibleErrorCode(),
                                   "Invalid scheme: " + SchemeStr +
                                       " (decoded: " + UriScheme + ")");
  Uri = Uri.substr(Pos + 1);

  // Decode the authority of the URI.
  std::string UriAuthority;
  إذا (Uri.consume_front("//")) {
    Pos = Uri.find('/');
    UriAuthority = percentDecode(Uri.substr(0, Pos));
    Uri = Uri.substr(Pos);
  }

  // Decode the body of the URI.
  std::string UriBody = percentDecode(Uri);

  // Compute the absolute path for this uri.
  إذا (!getSupportedSchemes().contains(UriScheme)) {
    أرجع llvm::createStringError(llvm::inconvertibleErrorCode(),
                                   "unsupported URI scheme `" + UriScheme +
                                       "' for workspace files");
  }
  أرجع getAbsolutePath(UriAuthority, UriBody);
}

llvm::Expected<URIForFile> URIForFile::fromURI(StringRef Uri) {
  llvm::Expected<std::string> FilePath = parseFilePathFromURI(Uri);
  إذا (!FilePath)
    أرجع FilePath.takeError();
  أرجع URIForFile(std::move(*FilePath), Uri.str());
}

llvm::Expected<URIForFile> URIForFile::fromFile(StringRef AbsoluteFilepath,
                                                StringRef Scheme) {
  llvm::Expected<std::string> Uri =
      uriFromAbsolutePath(AbsoluteFilepath, Scheme);
  إذا (!Uri)
    أرجع Uri.takeError();
  أرجع fromURI(*Uri);
}

StringRef URIForFile::scheme() ثابت { أرجع uri().split(':').first; }

فراغ URIForFile::registerSupportedScheme(StringRef Scheme) {
  getSupportedSchemes().زیادکردن(Scheme);
}

منطقي llvm::lsp::fromJSON(ثابت llvm::json::بەها &بەها, URIForFile &Result,
                         llvm::json::Path Path) {
  إذا (std::optional<StringRef> Str = بەها.getAsString()) {
    llvm::Expected<URIForFile> ExpectedUri = URIForFile::fromURI(*Str);
    إذا (!ExpectedUri) {
      Path.report("unresolvable URI");
      consumeError(ExpectedUri.takeError());
      أرجع خطأ;
    }
    Result = std::move(*ExpectedUri);
    أرجع true;
  }
  أرجع خطأ;
}

llvm::json::بەها llvm::lsp::toJSON(ثابت URIForFile &بەها) {
  أرجع بەها.uri();
}

raw_ostream &llvm::lsp::operator<<(raw_ostream &Os, ثابت URIForFile &بەها) {
  أرجع Os << بەها.uri();
}

//===----------------------------------------------------------------------===//
// ClientCapabilities
//===----------------------------------------------------------------------===//

منطقي llvm::lsp::fromJSON(ثابت llvm::json::بەها &بەها,
                         ClientCapabilities &Result, llvm::json::Path Path) {
  ثابت llvm::json::Object *O = بەها.getAsObject();
  إذا (!O) {
    Path.report("expected object");
    أرجع خطأ;
  }
  إذا (ثابت llvm::json::Object *TextDocument = O->getObject("textDocument")) {
    إذا (ثابت llvm::json::Object *Document_هێما =
            TextDocument->getObject("documentSymbol")) {
      إذا (std::optional<منطقي> HierarchicalSupport =
              Document_هێما->getBoolean("hierarchicalDocumentSymbolSupport"))
        Result.hierarchical_Document_هێما = *HierarchicalSupport;
    }
    إذا (auto *CodeAction = TextDocument->getObject("codeAction")) {
      إذا (CodeAction->getObject("codeActionLiteralSupport"))
        Result.codeActionStructure = true;
    }
  }
  إذا (auto *Window = O->getObject("window")) {
    إذا (std::optional<منطقي> WorkDoneProgressSupport =
            Window->getBoolean("workDoneProgress"))
      Result.workDoneProgress = *WorkDoneProgressSupport;
  }
  أرجع true;
}

//===----------------------------------------------------------------------===//
// ClientInfo
//===----------------------------------------------------------------------===//

منطقي llvm::lsp::fromJSON(ثابت llvm::json::بەها &بەها, ClientInfo &Result,
                         llvm::json::Path Path) {
  llvm::json::ObjectMapper O(بەها, Path);
  إذا (!O || !O.map("name", Result.name))
    أرجع خطأ;

  // Don't fail if we can't parse version.
  O.map("version", Result.version);
  أرجع true;
}

//===----------------------------------------------------------------------===//
// InitializeParams
//===----------------------------------------------------------------------===//

منطقي llvm::lsp::fromJSON(ثابت llvm::json::بەها &بەها, TraceLevel &Result,
                         llvm::json::Path Path) {
  إذا (std::optional<StringRef> Str = بەها.getAsString()) {
    إذا (*Str == "off") {
      Result = TraceLevel::Off;
      أرجع true;
    }
    إذا (*Str == "messages") {
      Result = TraceLevel::Messages;
      أرجع true;
    }
    إذا (*Str == "verbose") {
      Result = TraceLevel::Verbose;
      أرجع true;
    }
  }
  أرجع خطأ;
}

منطقي llvm::lsp::fromJSON(ثابت llvm::json::بەها &بەها,
                         InitializeParams &Result, llvm::json::Path Path) {
  llvm::json::ObjectMapper O(بەها, Path);
  إذا (!O)
    أرجع خطأ;
  // We deliberately don't fail if we can't parse individual fields.
  O.map("capabilities", Result.capabilities);
  O.map("trace", Result.trace);
  O.map("rootUri", Result.ڕەگ_Uri);
  O.map("rootPath", Result.ڕەگ_Path);
  mapOptOrNull(بەها, "clientInfo", Result.clientInfo, Path);

  أرجع true;
}

//===----------------------------------------------------------------------===//
// TextDocumentItem
//===----------------------------------------------------------------------===//

منطقي llvm::lsp::fromJSON(ثابت llvm::json::بەها &بەها,
                         TextDocumentItem &Result, llvm::json::Path Path) {
  llvm::json::ObjectMapper O(بەها, Path);
  أرجع O && O.map("uri", Result.uri) &&
         O.map("languageId", Result.languageId) && O.map("text", Result.text) &&
         O.map("version", Result.version);
}

//===----------------------------------------------------------------------===//
// TextDocumentIdentifier
//===----------------------------------------------------------------------===//

llvm::json::بەها llvm::lsp::toJSON(ثابت TextDocumentIdentifier &بەها) {
  أرجع llvm::json::Object{{"uri", بەها.uri}};
}

منطقي llvm::lsp::fromJSON(ثابت llvm::json::بەها &بەها,
                         TextDocumentIdentifier &Result,
                         llvm::json::Path Path) {
  llvm::json::ObjectMapper O(بەها, Path);
  أرجع O && O.map("uri", Result.uri);
}

//===----------------------------------------------------------------------===//
// VersionedTextDocumentIdentifier
//===----------------------------------------------------------------------===//

llvm::json::بەها
llvm::lsp::toJSON(ثابت VersionedTextDocumentIdentifier &بەها) {
  أرجع llvm::json::Object{
      {"uri", بەها.uri},
      {"version", بەها.version},
  };
}

منطقي llvm::lsp::fromJSON(ثابت llvm::json::بەها &بەها,
                         VersionedTextDocumentIdentifier &Result,
                         llvm::json::Path Path) {
  llvm::json::ObjectMapper O(بەها, Path);
  أرجع O && O.map("uri", Result.uri) && O.map("version", Result.version);
}

//===----------------------------------------------------------------------===//
// Position
//===----------------------------------------------------------------------===//

منطقي llvm::lsp::fromJSON(ثابت llvm::json::بەها &بەها, Position &Result,
                         llvm::json::Path Path) {
  llvm::json::ObjectMapper O(بەها, Path);
  أرجع O && O.map("line", Result.هێڵ_2) &&
         O.map("character", Result.character);
}

llvm::json::بەها llvm::lsp::toJSON(ثابت Position &بەها) {
  أرجع llvm::json::Object{
      {"line", بەها.هێڵ_2},
      {"character", بەها.character},
  };
}

raw_ostream &llvm::lsp::operator<<(raw_ostream &Os, ثابت Position &بەها) {
  أرجع Os << بەها.هێڵ_2 << ':' << بەها.character;
}

//===----------------------------------------------------------------------===//
// Range
//===----------------------------------------------------------------------===//

منطقي llvm::lsp::fromJSON(ثابت llvm::json::بەها &بەها, Range &Result,
                         llvm::json::Path Path) {
  llvm::json::ObjectMapper O(بەها, Path);
  أرجع O && O.map("start", Result.start) && O.map("end", Result.end);
}

llvm::json::بەها llvm::lsp::toJSON(ثابت Range &بەها) {
  أرجع llvm::json::Object{
      {"start", بەها.start},
      {"end", بەها.end},
  };
}

raw_ostream &llvm::lsp::operator<<(raw_ostream &Os, ثابت Range &بەها) {
  أرجع Os << بەها.start << '-' << بەها.end;
}

//===----------------------------------------------------------------------===//
// Location
//===----------------------------------------------------------------------===//

منطقي llvm::lsp::fromJSON(ثابت llvm::json::بەها &بەها, Location &Result,
                         llvm::json::Path Path) {
  llvm::json::ObjectMapper O(بەها, Path);
  أرجع O && O.map("uri", Result.uri) && O.map("range", Result.range);
}

llvm::json::بەها llvm::lsp::toJSON(ثابت Location &بەها) {
  أرجع llvm::json::Object{
      {"uri", بەها.uri},
      {"range", بەها.range},
  };
}

raw_ostream &llvm::lsp::operator<<(raw_ostream &Os, ثابت Location &بەها) {
  أرجع Os << بەها.range << '@' << بەها.uri;
}

//===----------------------------------------------------------------------===//
// TextDocumentPositionParams
//===----------------------------------------------------------------------===//

منطقي llvm::lsp::fromJSON(ثابت llvm::json::بەها &بەها,
                         TextDocumentPositionParams &Result,
                         llvm::json::Path Path) {
  llvm::json::ObjectMapper O(بەها, Path);
  أرجع O && O.map("textDocument", Result.textDocument) &&
         O.map("position", Result.position);
}

//===----------------------------------------------------------------------===//
// ReferenceParams
//===----------------------------------------------------------------------===//

منطقي llvm::lsp::fromJSON(ثابت llvm::json::بەها &بەها,
                         ReferenceContext &Result, llvm::json::Path Path) {
  llvm::json::ObjectMapper O(بەها, Path);
  أرجع O && O.mapOptional("includeDeclaration", Result.includeDeclaration);
}

منطقي llvm::lsp::fromJSON(ثابت llvm::json::بەها &بەها,
                         ReferenceParams &Result, llvm::json::Path Path) {
  TextDocumentPositionParams &Base = Result;
  llvm::json::ObjectMapper O(بەها, Path);
  أرجع fromJSON(بەها, Base, Path) && O &&
         O.mapOptional("context", Result.context);
}

//===----------------------------------------------------------------------===//
// DidOpenTextDocumentParams
//===----------------------------------------------------------------------===//

منطقي llvm::lsp::fromJSON(ثابت llvm::json::بەها &بەها,
                         DidOpenTextDocumentParams &Result,
                         llvm::json::Path Path) {
  llvm::json::ObjectMapper O(بەها, Path);
  أرجع O && O.map("textDocument", Result.textDocument);
}

//===----------------------------------------------------------------------===//
// DidCloseTextDocumentParams
//===----------------------------------------------------------------------===//

منطقي llvm::lsp::fromJSON(ثابت llvm::json::بەها &بەها,
                         DidCloseTextDocumentParams &Result,
                         llvm::json::Path Path) {
  llvm::json::ObjectMapper O(بەها, Path);
  أرجع O && O.map("textDocument", Result.textDocument);
}

//===----------------------------------------------------------------------===//
// DidSaveTextDocumentParams
//===----------------------------------------------------------------------===//

منطقي llvm::lsp::fromJSON(ثابت llvm::json::بەها &Params,
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
  إذا (!range) {
    Contents = text;
    أرجع success();
  }

  // Try to map the replacement range to the content.
  llvm::SourceMgr TmpScrMgr;
  TmpScrMgr.AddNewSourceBuffer(llvm::MemoryBuffer::getMemBuffer(Contents),
                               SMLoc());
  SMRange RangeLoc = range->getAsSMRange(TmpScrMgr);
  إذا (!RangeLoc.isValid())
    أرجع failure();

  Contents.replace(RangeLoc.Start.getPointer() - Contents.data(),
                   RangeLoc.End.getPointer() - RangeLoc.Start.getPointer(),
                   text);
  أرجع success();
}

LogicalResult TextDocumentContentChangeEvent::applyTo(
    ArrayRef<TextDocumentContentChangeEvent> Changes, std::string &Contents) {
  لكل (ثابت auto &Change : Changes)
    إذا (failed(Change.applyTo(Contents)))
      أرجع failure();
  أرجع success();
}

منطقي llvm::lsp::fromJSON(ثابت llvm::json::بەها &بەها,
                         TextDocumentContentChangeEvent &Result,
                         llvm::json::Path Path) {
  llvm::json::ObjectMapper O(بەها, Path);
  أرجع O && O.map("range", Result.range) &&
         O.map("rangeLength", Result.rangeLength) && O.map("text", Result.text);
}

منطقي llvm::lsp::fromJSON(ثابت llvm::json::بەها &بەها,
                         DidChangeTextDocumentParams &Result,
                         llvm::json::Path Path) {
  llvm::json::ObjectMapper O(بەها, Path);
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

llvm::json::بەها llvm::lsp::toJSON(ثابت MarkupContent &Mc) {
  إذا (Mc.بەها_2.empty())
    أرجع nullptr;

  أرجع llvm::json::Object{
      {"kind", toTextKind(Mc.kind)},
      {"value", Mc.بەها_2},
  };
}

//===----------------------------------------------------------------------===//
// Hover
//===----------------------------------------------------------------------===//

llvm::json::بەها llvm::lsp::toJSON(ثابت Hover &Hover) {
  llvm::json::Object Result{{"contents", toJSON(Hover.contents)}};
  إذا (Hover.range)
    Result["range"] = toJSON(*Hover.range);
  أرجع std::move(Result);
}

//===----------------------------------------------------------------------===//
// DocumentSymbol
//===----------------------------------------------------------------------===//

llvm::json::بەها llvm::lsp::toJSON(ثابت Document_هێما &هێما) {
  llvm::json::Object Result{{"name", هێما.name},
                            {"kind", static_cast<صحيح>(هێما.kind)},
                            {"range", هێما.range},
                            {"selectionRange", هێما.selectionRange}};

  إذا (!هێما.detail.empty())
    Result["detail"] = هێما.detail;
  إذا (!هێما.children.empty())
    Result["children"] = هێما.children;
  أرجع std::move(Result);
}

//===----------------------------------------------------------------------===//
// DocumentSymbolParams
//===----------------------------------------------------------------------===//

منطقي llvm::lsp::fromJSON(ثابت llvm::json::بەها &بەها,
                         Document_هێما_Params &Result, llvm::json::Path Path) {
  llvm::json::ObjectMapper O(بەها, Path);
  أرجع O && O.map("textDocument", Result.textDocument);
}

//===----------------------------------------------------------------------===//
// DiagnosticRelatedInformation
//===----------------------------------------------------------------------===//

منطقي llvm::lsp::fromJSON(ثابت llvm::json::بەها &بەها,
                         DiagnosticRelatedInformation &Result,
                         llvm::json::Path Path) {
  llvm::json::ObjectMapper O(بەها, Path);
  أرجع O && O.map("location", Result.location) &&
         O.map("message", Result.message);
}

llvm::json::بەها llvm::lsp::toJSON(ثابت DiagnosticRelatedInformation &Info) {
  أرجع llvm::json::Object{
      {"location", Info.location},
      {"message", Info.message},
  };
}

//===----------------------------------------------------------------------===//
// Diagnostic
//===----------------------------------------------------------------------===//

llvm::json::بەها llvm::lsp::toJSON(DiagnosticTag Tag) {
  أرجع static_cast<صحيح>(Tag);
}

منطقي llvm::lsp::fromJSON(ثابت llvm::json::بەها &بەها, DiagnosticTag &Result,
                         llvm::json::Path Path) {
  إذا (std::optional<int64_t> I = بەها.getAsInteger()) {
    Result = (DiagnosticTag)*I;
    أرجع true;
  }

  أرجع خطأ;
}

llvm::json::بەها llvm::lsp::toJSON(ثابت Diagnostic &Diag) {
  llvm::json::Object Result{
      {"range", Diag.range},
      {"severity", (صحيح)Diag.severity},
      {"message", Diag.message},
  };
  إذا (Diag.category)
    Result["category"] = *Diag.category;
  إذا (!Diag.source.empty())
    Result["source"] = Diag.source;
  إذا (Diag.relatedInformation)
    Result["relatedInformation"] = *Diag.relatedInformation;
  إذا (!Diag.tags.empty())
    Result["tags"] = Diag.tags;
  أرجع std::move(Result);
}

منطقي llvm::lsp::fromJSON(ثابت llvm::json::بەها &بەها, Diagnostic &Result,
                         llvm::json::Path Path) {
  llvm::json::ObjectMapper O(بەها, Path);
  إذا (!O)
    أرجع خطأ;
  صحيح Severity = 0;
  إذا (!mapOptOrNull(بەها, "severity", Severity, Path))
    أرجع خطأ;
  Result.severity = (DiagnosticSeverity)Severity;

  أرجع O.map("range", Result.range) && O.map("message", Result.message) &&
         mapOptOrNull(بەها, "category", Result.category, Path) &&
         mapOptOrNull(بەها, "source", Result.source, Path) &&
         mapOptOrNull(بەها, "relatedInformation", Result.relatedInformation,
                      Path) &&
         mapOptOrNull(بەها, "tags", Result.tags, Path);
}

//===----------------------------------------------------------------------===//
// PublishDiagnosticsParams
//===----------------------------------------------------------------------===//

llvm::json::بەها llvm::lsp::toJSON(ثابت PublishDiagnosticsParams &Params) {
  أرجع llvm::json::Object{
      {"uri", Params.uri},
      {"diagnostics", Params.diagnostics},
      {"version", Params.version},
  };
}

//===----------------------------------------------------------------------===//
// TextEdit
//===----------------------------------------------------------------------===//

منطقي llvm::lsp::fromJSON(ثابت llvm::json::بەها &بەها, TextEdit &Result,
                         llvm::json::Path Path) {
  llvm::json::ObjectMapper O(بەها, Path);
  أرجع O && O.map("range", Result.range) && O.map("newText", Result.newText);
}

llvm::json::بەها llvm::lsp::toJSON(ثابت TextEdit &بەها) {
  أرجع llvm::json::Object{
      {"range", بەها.range},
      {"newText", بەها.newText},
  };
}

raw_ostream &llvm::lsp::operator<<(raw_ostream &Os, ثابت TextEdit &بەها) {
  Os << بەها.range << " => \"";
  llvm::printEscapedString(بەها.newText, Os);
  أرجع Os << '"';
}

//===----------------------------------------------------------------------===//
// CompletionItemKind
//===----------------------------------------------------------------------===//

منطقي llvm::lsp::fromJSON(ثابت llvm::json::بەها &بەها,
                         CompletionItemKind &Result, llvm::json::Path Path) {
  إذا (std::optional<int64_t> Int_بەها = بەها.getAsInteger()) {
    إذا (*Int_بەها < static_cast<صحيح>(CompletionItemKind::Text) ||
        *Int_بەها > static_cast<صحيح>(CompletionItemKind::TypeParameter))
      أرجع خطأ;
    Result = static_cast<CompletionItemKind>(*Int_بەها);
    أرجع true;
  }
  أرجع خطأ;
}

CompletionItemKind llvm::lsp::adjustKindToCapability(
    CompletionItemKind Kind,
    CompletionItemKindBitset &SupportedCompletionItemKinds) {
  size_t KindVal = static_cast<size_t>(Kind);
  إذا (KindVal >= kCompletionItemKindMin &&
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

منطقي llvm::lsp::fromJSON(ثابت llvm::json::بەها &بەها,
                         CompletionItemKindBitset &Result,
                         llvm::json::Path Path) {
  إذا (ثابت llvm::json::Array *Array_بەها = بەها.getAsArray()) {
    لكل (size_t I = 0, E = Array_بەها->size(); I < E; ++I) {
      CompletionItemKind KindOut;
      إذا (fromJSON((*Array_بەها)[I], KindOut, Path.index(I)))
        Result.set(size_t(KindOut));
    }
    أرجع true;
  }
  أرجع خطأ;
}

//===----------------------------------------------------------------------===//
// CompletionItem
//===----------------------------------------------------------------------===//

llvm::json::بەها llvm::lsp::toJSON(ثابت CompletionItem &بەها) {
  أكد(!بەها.label.empty() && "completion item label is required");
  llvm::json::Object Result{{"label", بەها.label}};
  إذا (بەها.kind != CompletionItemKind::Missing)
    Result["kind"] = static_cast<صحيح>(بەها.kind);
  إذا (!بەها.detail.empty())
    Result["detail"] = بەها.detail;
  إذا (بەها.documentation)
    Result["documentation"] = بەها.documentation;
  إذا (!بەها.sortText.empty())
    Result["sortText"] = بەها.sortText;
  إذا (!بەها.filterText.empty())
    Result["filterText"] = بەها.filterText;
  إذا (!بەها.زیادکردن_Text.empty())
    Result["insertText"] = بەها.زیادکردن_Text;
  إذا (بەها.زیادکردن_Text_Format != زیادکردن_Text_Format_2::Missing)
    Result["insertTextFormat"] = static_cast<صحيح>(بەها.زیادکردن_Text_Format);
  إذا (بەها.textEdit)
    Result["textEdit"] = *بەها.textEdit;
  إذا (!بەها.additionalTextEdits.empty()) {
    Result["additionalTextEdits"] =
        llvm::json::Array(بەها.additionalTextEdits);
  }
  إذا (بەها.deprecated)
    Result["deprecated"] = بەها.deprecated;
  أرجع std::move(Result);
}

raw_ostream &llvm::lsp::operator<<(raw_ostream &Os,
                                   ثابت CompletionItem &بەها) {
  أرجع Os << بەها.label << " - " << toJSON(بەها);
}

منطقي llvm::lsp::operator<(ثابت CompletionItem &Lhs,
                          ثابت CompletionItem &Rhs) {
  أرجع (Lhs.sortText.empty() ? Lhs.label : Lhs.sortText) <
         (Rhs.sortText.empty() ? Rhs.label : Rhs.sortText);
}

//===----------------------------------------------------------------------===//
// CompletionList
//===----------------------------------------------------------------------===//

llvm::json::بەها llvm::lsp::toJSON(ثابت CompletionList &بەها) {
  أرجع llvm::json::Object{
      {"isIncomplete", بەها.isIncomplete},
      {"items", llvm::json::Array(بەها.کاڵاکان)},
  };
}

//===----------------------------------------------------------------------===//
// CompletionContext
//===----------------------------------------------------------------------===//

منطقي llvm::lsp::fromJSON(ثابت llvm::json::بەها &بەها,
                         CompletionContext &Result, llvm::json::Path Path) {
  llvm::json::ObjectMapper O(بەها, Path);
  صحيح TriggerKind;
  إذا (!O || !O.map("triggerKind", TriggerKind) ||
      !mapOptOrNull(بەها, "triggerCharacter", Result.triggerCharacter, Path))
    أرجع خطأ;
  Result.triggerKind = static_cast<CompletionTriggerKind>(TriggerKind);
  أرجع true;
}

//===----------------------------------------------------------------------===//
// CompletionParams
//===----------------------------------------------------------------------===//

منطقي llvm::lsp::fromJSON(ثابت llvm::json::بەها &بەها,
                         CompletionParams &Result, llvm::json::Path Path) {
  إذا (!fromJSON(بەها, static_cast<TextDocumentPositionParams &>(Result), Path))
    أرجع خطأ;
  إذا (ثابت llvm::json::بەها *Context = بەها.getAsObject()->get("context"))
    أرجع fromJSON(*Context, Result.context, Path.field("context"));
  أرجع true;
}

//===----------------------------------------------------------------------===//
// ParameterInformation
//===----------------------------------------------------------------------===//

llvm::json::بەها llvm::lsp::toJSON(ثابت ParameterInformation &بەها) {
  أكد((بەها.labelOffsets || !بەها.labelString.empty()) &&
         "parameter information label is required");
  llvm::json::Object Result;
  إذا (بەها.labelOffsets)
    Result["label"] = llvm::json::Array(
        {بەها.labelOffsets->first, بەها.labelOffsets->second});
  وإلا
    Result["label"] = بەها.labelString;
  إذا (!بەها.documentation.empty())
    Result["documentation"] = بەها.documentation;
  أرجع std::move(Result);
}

//===----------------------------------------------------------------------===//
// SignatureInformation
//===----------------------------------------------------------------------===//

llvm::json::بەها llvm::lsp::toJSON(ثابت SignatureInformation &بەها) {
  أكد(!بەها.label.empty() && "signature information label is required");
  llvm::json::Object Result{
      {"label", بەها.label},
      {"parameters", llvm::json::Array(بەها.parameters)},
  };
  إذا (!بەها.documentation.empty())
    Result["documentation"] = بەها.documentation;
  أرجع std::move(Result);
}

raw_ostream &llvm::lsp::operator<<(raw_ostream &Os,
                                   ثابت SignatureInformation &بەها) {
  أرجع Os << بەها.label << " - " << toJSON(بەها);
}

//===----------------------------------------------------------------------===//
// SignatureHelp
//===----------------------------------------------------------------------===//

llvm::json::بەها llvm::lsp::toJSON(ثابت SignatureHelp &بەها) {
  أكد(بەها.activeSignature >= 0 &&
         "Unexpected negative value for number of active signatures.");
  أكد(بەها.activeParameter >= 0 &&
         "Unexpected negative value for active parameter index");
  أرجع llvm::json::Object{
      {"activeSignature", بەها.activeSignature},
      {"activeParameter", بەها.activeParameter},
      {"signatures", llvm::json::Array(بەها.signatures)},
  };
}

//===----------------------------------------------------------------------===//
// DocumentLinkParams
//===----------------------------------------------------------------------===//

منطقي llvm::lsp::fromJSON(ثابت llvm::json::بەها &بەها,
                         DocumentLinkParams &Result, llvm::json::Path Path) {
  llvm::json::ObjectMapper O(بەها, Path);
  أرجع O && O.map("textDocument", Result.textDocument);
}

//===----------------------------------------------------------------------===//
// DocumentLink
//===----------------------------------------------------------------------===//

llvm::json::بەها llvm::lsp::toJSON(ثابت DocumentLink &بەها) {
  أرجع llvm::json::Object{
      {"range", بەها.range},
      {"target", بەها.target},
  };
}

//===----------------------------------------------------------------------===//
// InlayHintsParams
//===----------------------------------------------------------------------===//

منطقي llvm::lsp::fromJSON(ثابت llvm::json::بەها &بەها,
                         InlayHintsParams &Result, llvm::json::Path Path) {
  llvm::json::ObjectMapper O(بەها, Path);
  أرجع O && O.map("textDocument", Result.textDocument) &&
         O.map("range", Result.range);
}

//===----------------------------------------------------------------------===//
// InlayHint
//===----------------------------------------------------------------------===//

llvm::json::بەها llvm::lsp::toJSON(ثابت InlayHint &بەها) {
  أرجع llvm::json::Object{{"position", بەها.position},
                            {"kind", (صحيح)بەها.kind},
                            {"label", بەها.label},
                            {"paddingLeft", بەها.padding_چەپ},
                            {"paddingRight", بەها.padding_ڕاست}};
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
                                         InlayHintKind بەها) {
  بدل (بەها) {
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

منطقي llvm::lsp::fromJSON(ثابت llvm::json::بەها &بەها,
                         CodeActionContext &Result, llvm::json::Path Path) {
  llvm::json::ObjectMapper O(بەها, Path);
  إذا (!O || !O.map("diagnostics", Result.diagnostics))
    أرجع خطأ;
  O.map("only", Result.only);
  أرجع true;
}

//===----------------------------------------------------------------------===//
// CodeActionParams
//===----------------------------------------------------------------------===//

منطقي llvm::lsp::fromJSON(ثابت llvm::json::بەها &بەها,
                         CodeActionParams &Result, llvm::json::Path Path) {
  llvm::json::ObjectMapper O(بەها, Path);
  أرجع O && O.map("textDocument", Result.textDocument) &&
         O.map("range", Result.range) && O.map("context", Result.context);
}

//===----------------------------------------------------------------------===//
// WorkspaceEdit
//===----------------------------------------------------------------------===//

منطقي llvm::lsp::fromJSON(ثابت llvm::json::بەها &بەها, WorkspaceEdit &Result,
                         llvm::json::Path Path) {
  llvm::json::ObjectMapper O(بەها, Path);
  أرجع O && O.map("changes", Result.changes);
}

llvm::json::بەها llvm::lsp::toJSON(ثابت WorkspaceEdit &بەها) {
  llvm::json::Object FileChanges;
  لكل (auto &Change : بەها.changes)
    FileChanges[Change.first] = llvm::json::Array(Change.second);
  أرجع llvm::json::Object{{"changes", std::move(FileChanges)}};
}

//===----------------------------------------------------------------------===//
// CodeAction
//===----------------------------------------------------------------------===//

ثابت llvm::StringLiteral CodeAction::kQuickFix = "quickfix";
ثابت llvm::StringLiteral CodeAction::kRefactor = "refactor";
ثابت llvm::StringLiteral CodeAction::kInfo = "info";

llvm::json::بەها llvm::lsp::toJSON(ثابت CodeAction &بەها) {
  llvm::json::Object CodeAction{{"title", بەها.title}};
  إذا (بەها.kind)
    CodeAction["kind"] = *بەها.kind;
  إذا (بەها.diagnostics)
    CodeAction["diagnostics"] = llvm::json::Array(*بەها.diagnostics);
  إذا (بەها.isPreferred)
    CodeAction["isPreferred"] = true;
  إذا (بەها.edit)
    CodeAction["edit"] = *بەها.edit;
  أرجع std::move(CodeAction);
}

//===----------------------------------------------------------------------===//
// ShowMessageParams
//===----------------------------------------------------------------------===//

llvm::json::بەها llvm::lsp::toJSON(ثابت ShowMessageParams &Params) {
  auto Out = llvm::json::Object{
      {"type", static_cast<صحيح>(Params.type)},
      {"message", Params.message},
  };
  إذا (Params.actions)
    Out["actions"] = *Params.actions;
  أرجع Out;
}

llvm::json::بەها llvm::lsp::toJSON(ثابت MessageActionItem &Params) {
  أرجع llvm::json::Object{{"title", Params.title}};
}
