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

#अन्तर्भाव "llvm/Support/LSP/Protocol.h"
#अन्तर्भाव "llvm/ADT/StringExtras.h"
#अन्तर्भाव "llvm/ADT/StringSet.h"
#अन्तर्भाव "llvm/Support/ErrorHandling.h"
#अन्तर्भाव "llvm/Support/JSON.h"
#अन्तर्भाव "llvm/Support/MemoryBuffer.h"
#अन्तर्भाव "llvm/Support/Path.h"
#अन्तर्भाव "llvm/Support/raw_ostream.h"

using नामाकाश llvm;
using नामाकाश llvm::lsp;

// Helper that doesn't treat `null` and absent fields as failures.
ढाँचा <typename T>
स्थिर तार्किक mapOptOrNull(स्थिरांक llvm::json::मूल्य &Params,
                         llvm::StringLiteral Prop, T &Out,
                         llvm::json::Path Path) {
  स्थिरांक llvm::json::Object *O = Params.getAsObject();
  प्रतिज्ञा(O);

  // Field is missing or null.
  स्वतः *V = O->get(Prop);
  जर (!V || V->getAsNull())
    परत सत्य;
  परत fromJSON(*V, Out, Path.field(Prop));
}

//===----------------------------------------------------------------------===//
// LSPError
//===----------------------------------------------------------------------===//

अक्षर LSPError::ID;

//===----------------------------------------------------------------------===//
// URIForFile
//===----------------------------------------------------------------------===//

स्थिर तार्किक isWindowsPath(StringRef Path) {
  परत Path.size() > 1 && llvm::isAlpha(Path[0]) && Path[1] == ':';
}

स्थिर तार्किक isNetworkPath(StringRef Path) {
  परत Path.size() > 2 && Path[0] == Path[1] &&
         llvm::sys::path::is_separator(Path[0]);
}

स्थिर तार्किक shouldEscapeInURI(अचिन्हित अक्षर C) {
  // Unreserved characters.
  जर ((C >= 'a' && C <= 'z') || (C >= 'A' && C <= 'Z') ||
      (C >= '0' && C <= '9'))
    परत असत्य;

  निवड (C) {
  स्थिती '-':
  स्थिती '_':
  स्थिती '.':
  स्थिती '~':
  // '/' is only reserved when parsing.
  स्थिती '/':
  // ':' is only reserved for relative URI paths, which we doesn't produce.
  स्थिती ':':
    परत असत्य;
  }
  परत सत्य;
}

/// Encodes a string according to percent-encoding.
/// - Unreserved characters are not escaped.
/// - Reserved characters always escaped with exceptions like '/'.
/// - All other characters are escaped.
स्थिर शून्य percentEncode(StringRef Content, std::string &Out) {
  साठी (अचिन्हित अक्षर C : Content) {
    जर (shouldEscapeInURI(C)) {
      Out.ढकला_back('%');
      Out.ढकला_back(llvm::hexdigit(C / 16));
      Out.ढकला_back(llvm::hexdigit(C % 16));
    } अन्यथा {
      Out.ढकला_back(C);
    }
  }
}

/// Decodes a string according to percent-encoding.
स्थिर std::string percentDecode(StringRef Content) {
  std::string Result;
  साठी (स्वतः I = Content.begin(), E = Content.end(); I != E; ++I) {
    जर (*I == '%' && I + 2 < Content.end() && llvm::isHexDigit(*(I + 1)) &&
        llvm::isHexDigit(*(I + 2))) {
      Result.ढकला_back(llvm::hexFromNibbles(*(I + 1), *(I + 2)));
      I += 2;
    } अन्यथा {
      Result.ढकला_back(*I);
    }
  }
  परत Result;
}

/// Return the set containing the supported URI schemes.
स्थिर StringSet<> &getSupportedSchemes() {
  स्थिर StringSet<> Schemes({"file", "test"});
  परत Schemes;
}

/// Returns true if the given scheme is structurally valid, i.e. it does not
/// contain any invalid scheme characters. This does not check that the scheme
/// is actually supported.
स्थिर तार्किक isStructurallyValidScheme(StringRef Scheme) {
  जर (Scheme.empty())
    परत असत्य;
  जर (!llvm::isAlpha(Scheme[0]))
    परत असत्य;
  परत llvm::all_of(llvm::drop_begin(Scheme), [](अक्षर C) {
    परत llvm::isAlnum(C) || C == '+' || C == '.' || C == '-';
  });
}

स्थिर llvm::Expected<std::string> uriFromAbsolutePath(StringRef AbsolutePath,
                                                       StringRef Scheme) {
  std::string Body;
  StringRef Authority;
  StringRef मूळ_2 = llvm::sys::path::मूळ_name(AbsolutePath);
  जर (isNetworkPath(मूळ_2)) {
    // Windows UNC paths e.g. \\server\share => file://server/share
    Authority = मूळ_2.drop_front(2);
    AbsolutePath.consume_front(मूळ_2);
  } अन्यथा जर (isWindowsPath(मूळ_2)) {
    // Windows paths e.g. X:\path => file:///X:/path
    Body = "/";
  }
  Body += llvm::sys::path::convert_to_slash(AbsolutePath);

  std::string Uri = Scheme.str() + ":";
  जर (Authority.empty() && Body.empty())
    परत Uri;

  // If authority if empty, we only print body if it starts with "/"; otherwise,
  // the URI is invalid.
  जर (!Authority.empty() || StringRef(Body).starts_with("/")) {
    Uri.append("//");
    percentEncode(Authority, Uri);
  }
  percentEncode(Body, Uri);
  परत Uri;
}

स्थिर llvm::Expected<std::string> getAbsolutePath(StringRef Authority,
                                                   StringRef Body) {
  जर (!Body.starts_with("/"))
    परत llvm::createStringError(
        llvm::inconvertibleErrorCode(),
        "File scheme: expect body to be an absolute path starting "
        "with '/': " +
            Body);
  SmallString<128> Path;
  जर (!Authority.empty()) {
    // Windows UNC paths e.g. file://server/share => \\server\share
    ("//" + Authority).toVector(Path);
  } अन्यथा जर (isWindowsPath(Body.substr(1))) {
    // Windows paths e.g. file:///X:/path => X:\path
    Body.consume_front("/");
  }
  Path.append(Body);
  llvm::sys::path::native(Path);
  परत std::string(Path);
}

स्थिर llvm::Expected<std::string> parseFilePathFromURI(StringRef OrigUri) {
  StringRef Uri = OrigUri;

  // Decode the scheme of the URI.
  माप_प्रकार Pos = Uri.find(':');
  जर (Pos == StringRef::npos)
    परत llvm::createStringError(llvm::inconvertibleErrorCode(),
                                   "Scheme must be provided in URI: " +
                                       OrigUri);
  StringRef SchemeStr = Uri.substr(0, Pos);
  std::string UriScheme = percentDecode(SchemeStr);
  जर (!isStructurallyValidScheme(UriScheme))
    परत llvm::createStringError(llvm::inconvertibleErrorCode(),
                                   "Invalid scheme: " + SchemeStr +
                                       " (decoded: " + UriScheme + ")");
  Uri = Uri.substr(Pos + 1);

  // Decode the authority of the URI.
  std::string UriAuthority;
  जर (Uri.consume_front("//")) {
    Pos = Uri.find('/');
    UriAuthority = percentDecode(Uri.substr(0, Pos));
    Uri = Uri.substr(Pos);
  }

  // Decode the body of the URI.
  std::string UriBody = percentDecode(Uri);

  // Compute the absolute path for this uri.
  जर (!getSupportedSchemes().contains(UriScheme)) {
    परत llvm::createStringError(llvm::inconvertibleErrorCode(),
                                   "unsupported URI scheme `" + UriScheme +
                                       "' for workspace files");
  }
  परत getAbsolutePath(UriAuthority, UriBody);
}

llvm::Expected<URIForFile> URIForFile::fromURI(StringRef Uri) {
  llvm::Expected<std::string> FilePath = parseFilePathFromURI(Uri);
  जर (!FilePath)
    परत FilePath.takeError();
  परत URIForFile(std::move(*FilePath), Uri.str());
}

llvm::Expected<URIForFile> URIForFile::fromFile(StringRef AbsoluteFilepath,
                                                StringRef Scheme) {
  llvm::Expected<std::string> Uri =
      uriFromAbsolutePath(AbsoluteFilepath, Scheme);
  जर (!Uri)
    परत Uri.takeError();
  परत fromURI(*Uri);
}

StringRef URIForFile::scheme() स्थिरांक { परत uri().split(':').first; }

शून्य URIForFile::registerSupportedScheme(StringRef Scheme) {
  getSupportedSchemes().घाला(Scheme);
}

तार्किक llvm::lsp::fromJSON(स्थिरांक llvm::json::मूल्य &मूल्य, URIForFile &Result,
                         llvm::json::Path Path) {
  जर (std::optional<StringRef> Str = मूल्य.getAsString()) {
    llvm::Expected<URIForFile> ExpectedUri = URIForFile::fromURI(*Str);
    जर (!ExpectedUri) {
      Path.report("unresolvable URI");
      consumeError(ExpectedUri.takeError());
      परत असत्य;
    }
    Result = std::move(*ExpectedUri);
    परत सत्य;
  }
  परत असत्य;
}

llvm::json::मूल्य llvm::lsp::toJSON(स्थिरांक URIForFile &मूल्य) {
  परत मूल्य.uri();
}

raw_ostream &llvm::lsp::चालक<<(raw_ostream &Os, स्थिरांक URIForFile &मूल्य) {
  परत Os << मूल्य.uri();
}

//===----------------------------------------------------------------------===//
// ClientCapabilities
//===----------------------------------------------------------------------===//

तार्किक llvm::lsp::fromJSON(स्थिरांक llvm::json::मूल्य &मूल्य,
                         ClientCapabilities &Result, llvm::json::Path Path) {
  स्थिरांक llvm::json::Object *O = मूल्य.getAsObject();
  जर (!O) {
    Path.report("expected object");
    परत असत्य;
  }
  जर (स्थिरांक llvm::json::Object *TextDocument = O->getObject("textDocument")) {
    जर (स्थिरांक llvm::json::Object *Document_चिन्ह =
            TextDocument->getObject("documentSymbol")) {
      जर (std::optional<तार्किक> HierarchicalSupport =
              Document_चिन्ह->getBoolean("hierarchicalDocumentSymbolSupport"))
        Result.hierarchical_Document_चिन्ह = *HierarchicalSupport;
    }
    जर (स्वतः *CodeAction = TextDocument->getObject("codeAction")) {
      जर (CodeAction->getObject("codeActionLiteralSupport"))
        Result.codeActionStructure = सत्य;
    }
  }
  जर (स्वतः *Window = O->getObject("window")) {
    जर (std::optional<तार्किक> WorkDoneProgressSupport =
            Window->getBoolean("workDoneProgress"))
      Result.workDoneProgress = *WorkDoneProgressSupport;
  }
  परत सत्य;
}

//===----------------------------------------------------------------------===//
// ClientInfo
//===----------------------------------------------------------------------===//

तार्किक llvm::lsp::fromJSON(स्थिरांक llvm::json::मूल्य &मूल्य, ClientInfo &Result,
                         llvm::json::Path Path) {
  llvm::json::ObjectMapper O(मूल्य, Path);
  जर (!O || !O.map("name", Result.name))
    परत असत्य;

  // Don't fail if we can't parse version.
  O.map("version", Result.version);
  परत सत्य;
}

//===----------------------------------------------------------------------===//
// InitializeParams
//===----------------------------------------------------------------------===//

तार्किक llvm::lsp::fromJSON(स्थिरांक llvm::json::मूल्य &मूल्य, TraceLevel &Result,
                         llvm::json::Path Path) {
  जर (std::optional<StringRef> Str = मूल्य.getAsString()) {
    जर (*Str == "off") {
      Result = TraceLevel::Off;
      परत सत्य;
    }
    जर (*Str == "messages") {
      Result = TraceLevel::Messages;
      परत सत्य;
    }
    जर (*Str == "verbose") {
      Result = TraceLevel::Verbose;
      परत सत्य;
    }
  }
  परत असत्य;
}

तार्किक llvm::lsp::fromJSON(स्थिरांक llvm::json::मूल्य &मूल्य,
                         InitializeParams &Result, llvm::json::Path Path) {
  llvm::json::ObjectMapper O(मूल्य, Path);
  जर (!O)
    परत असत्य;
  // We deliberately don't fail if we can't parse individual fields.
  O.map("capabilities", Result.capabilities);
  O.map("trace", Result.trace);
  O.map("rootUri", Result.मूळ_Uri);
  O.map("rootPath", Result.मूळ_Path);
  mapOptOrNull(मूल्य, "clientInfo", Result.clientInfo, Path);

  परत सत्य;
}

//===----------------------------------------------------------------------===//
// TextDocumentItem
//===----------------------------------------------------------------------===//

तार्किक llvm::lsp::fromJSON(स्थिरांक llvm::json::मूल्य &मूल्य,
                         TextDocumentItem &Result, llvm::json::Path Path) {
  llvm::json::ObjectMapper O(मूल्य, Path);
  परत O && O.map("uri", Result.uri) &&
         O.map("languageId", Result.languageId) && O.map("text", Result.text) &&
         O.map("version", Result.version);
}

//===----------------------------------------------------------------------===//
// TextDocumentIdentifier
//===----------------------------------------------------------------------===//

llvm::json::मूल्य llvm::lsp::toJSON(स्थिरांक TextDocumentIdentifier &मूल्य) {
  परत llvm::json::Object{{"uri", मूल्य.uri}};
}

तार्किक llvm::lsp::fromJSON(स्थिरांक llvm::json::मूल्य &मूल्य,
                         TextDocumentIdentifier &Result,
                         llvm::json::Path Path) {
  llvm::json::ObjectMapper O(मूल्य, Path);
  परत O && O.map("uri", Result.uri);
}

//===----------------------------------------------------------------------===//
// VersionedTextDocumentIdentifier
//===----------------------------------------------------------------------===//

llvm::json::मूल्य
llvm::lsp::toJSON(स्थिरांक VersionedTextDocumentIdentifier &मूल्य) {
  परत llvm::json::Object{
      {"uri", मूल्य.uri},
      {"version", मूल्य.version},
  };
}

तार्किक llvm::lsp::fromJSON(स्थिरांक llvm::json::मूल्य &मूल्य,
                         VersionedTextDocumentIdentifier &Result,
                         llvm::json::Path Path) {
  llvm::json::ObjectMapper O(मूल्य, Path);
  परत O && O.map("uri", Result.uri) && O.map("version", Result.version);
}

//===----------------------------------------------------------------------===//
// Position
//===----------------------------------------------------------------------===//

तार्किक llvm::lsp::fromJSON(स्थिरांक llvm::json::मूल्य &मूल्य, Position &Result,
                         llvm::json::Path Path) {
  llvm::json::ObjectMapper O(मूल्य, Path);
  परत O && O.map("line", Result.ओळ_2) &&
         O.map("character", Result.character);
}

llvm::json::मूल्य llvm::lsp::toJSON(स्थिरांक Position &मूल्य) {
  परत llvm::json::Object{
      {"line", मूल्य.ओळ_2},
      {"character", मूल्य.character},
  };
}

raw_ostream &llvm::lsp::चालक<<(raw_ostream &Os, स्थिरांक Position &मूल्य) {
  परत Os << मूल्य.ओळ_2 << ':' << मूल्य.character;
}

//===----------------------------------------------------------------------===//
// Range
//===----------------------------------------------------------------------===//

तार्किक llvm::lsp::fromJSON(स्थिरांक llvm::json::मूल्य &मूल्य, Range &Result,
                         llvm::json::Path Path) {
  llvm::json::ObjectMapper O(मूल्य, Path);
  परत O && O.map("start", Result.start) && O.map("end", Result.end);
}

llvm::json::मूल्य llvm::lsp::toJSON(स्थिरांक Range &मूल्य) {
  परत llvm::json::Object{
      {"start", मूल्य.start},
      {"end", मूल्य.end},
  };
}

raw_ostream &llvm::lsp::चालक<<(raw_ostream &Os, स्थिरांक Range &मूल्य) {
  परत Os << मूल्य.start << '-' << मूल्य.end;
}

//===----------------------------------------------------------------------===//
// Location
//===----------------------------------------------------------------------===//

तार्किक llvm::lsp::fromJSON(स्थिरांक llvm::json::मूल्य &मूल्य, Location &Result,
                         llvm::json::Path Path) {
  llvm::json::ObjectMapper O(मूल्य, Path);
  परत O && O.map("uri", Result.uri) && O.map("range", Result.range);
}

llvm::json::मूल्य llvm::lsp::toJSON(स्थिरांक Location &मूल्य) {
  परत llvm::json::Object{
      {"uri", मूल्य.uri},
      {"range", मूल्य.range},
  };
}

raw_ostream &llvm::lsp::चालक<<(raw_ostream &Os, स्थिरांक Location &मूल्य) {
  परत Os << मूल्य.range << '@' << मूल्य.uri;
}

//===----------------------------------------------------------------------===//
// TextDocumentPositionParams
//===----------------------------------------------------------------------===//

तार्किक llvm::lsp::fromJSON(स्थिरांक llvm::json::मूल्य &मूल्य,
                         TextDocumentPositionParams &Result,
                         llvm::json::Path Path) {
  llvm::json::ObjectMapper O(मूल्य, Path);
  परत O && O.map("textDocument", Result.textDocument) &&
         O.map("position", Result.position);
}

//===----------------------------------------------------------------------===//
// ReferenceParams
//===----------------------------------------------------------------------===//

तार्किक llvm::lsp::fromJSON(स्थिरांक llvm::json::मूल्य &मूल्य,
                         ReferenceContext &Result, llvm::json::Path Path) {
  llvm::json::ObjectMapper O(मूल्य, Path);
  परत O && O.mapOptional("includeDeclaration", Result.includeDeclaration);
}

तार्किक llvm::lsp::fromJSON(स्थिरांक llvm::json::मूल्य &मूल्य,
                         ReferenceParams &Result, llvm::json::Path Path) {
  TextDocumentPositionParams &Base = Result;
  llvm::json::ObjectMapper O(मूल्य, Path);
  परत fromJSON(मूल्य, Base, Path) && O &&
         O.mapOptional("context", Result.context);
}

//===----------------------------------------------------------------------===//
// DidOpenTextDocumentParams
//===----------------------------------------------------------------------===//

तार्किक llvm::lsp::fromJSON(स्थिरांक llvm::json::मूल्य &मूल्य,
                         DidOpenTextDocumentParams &Result,
                         llvm::json::Path Path) {
  llvm::json::ObjectMapper O(मूल्य, Path);
  परत O && O.map("textDocument", Result.textDocument);
}

//===----------------------------------------------------------------------===//
// DidCloseTextDocumentParams
//===----------------------------------------------------------------------===//

तार्किक llvm::lsp::fromJSON(स्थिरांक llvm::json::मूल्य &मूल्य,
                         DidCloseTextDocumentParams &Result,
                         llvm::json::Path Path) {
  llvm::json::ObjectMapper O(मूल्य, Path);
  परत O && O.map("textDocument", Result.textDocument);
}

//===----------------------------------------------------------------------===//
// DidSaveTextDocumentParams
//===----------------------------------------------------------------------===//

तार्किक llvm::lsp::fromJSON(स्थिरांक llvm::json::मूल्य &Params,
                         DidSaveTextDocumentParams &R, llvm::json::Path P) {
  llvm::json::ObjectMapper O(Params, P);
  परत O && O.map("textDocument", R.textDocument);
}

//===----------------------------------------------------------------------===//
// DidChangeTextDocumentParams
//===----------------------------------------------------------------------===//

LogicalResult
TextDocumentContentChangeEvent::applyTo(std::string &Contents) स्थिरांक {
  // If there is no range, the full document changed.
  जर (!range) {
    Contents = text;
    परत success();
  }

  // Try to map the replacement range to the content.
  llvm::SourceMgr TmpScrMgr;
  TmpScrMgr.AddNewSourceBuffer(llvm::MemoryBuffer::getMemBuffer(Contents),
                               SMLoc());
  SMRange RangeLoc = range->getAsSMRange(TmpScrMgr);
  जर (!RangeLoc.isValid())
    परत failure();

  Contents.replace(RangeLoc.Start.getPointer() - Contents.data(),
                   RangeLoc.End.getPointer() - RangeLoc.Start.getPointer(),
                   text);
  परत success();
}

LogicalResult TextDocumentContentChangeEvent::applyTo(
    ArrayRef<TextDocumentContentChangeEvent> Changes, std::string &Contents) {
  साठी (स्थिरांक स्वतः &Change : Changes)
    जर (failed(Change.applyTo(Contents)))
      परत failure();
  परत success();
}

तार्किक llvm::lsp::fromJSON(स्थिरांक llvm::json::मूल्य &मूल्य,
                         TextDocumentContentChangeEvent &Result,
                         llvm::json::Path Path) {
  llvm::json::ObjectMapper O(मूल्य, Path);
  परत O && O.map("range", Result.range) &&
         O.map("rangeLength", Result.rangeLength) && O.map("text", Result.text);
}

तार्किक llvm::lsp::fromJSON(स्थिरांक llvm::json::मूल्य &मूल्य,
                         DidChangeTextDocumentParams &Result,
                         llvm::json::Path Path) {
  llvm::json::ObjectMapper O(मूल्य, Path);
  परत O && O.map("textDocument", Result.textDocument) &&
         O.map("contentChanges", Result.contentChanges);
}

//===----------------------------------------------------------------------===//
// MarkupContent
//===----------------------------------------------------------------------===//

स्थिर llvm::StringRef toTextKind(MarkupKind Kind) {
  निवड (Kind) {
  स्थिती MarkupKind::PlainText:
    परत "plaintext";
  स्थिती MarkupKind::Markdown:
    परत "markdown";
  }
  llvm_unreachable("Invalid MarkupKind");
}

raw_ostream &llvm::lsp::चालक<<(raw_ostream &Os, MarkupKind Kind) {
  परत Os << toTextKind(Kind);
}

llvm::json::मूल्य llvm::lsp::toJSON(स्थिरांक MarkupContent &Mc) {
  जर (Mc.मूल्य_2.empty())
    परत nullptr;

  परत llvm::json::Object{
      {"kind", toTextKind(Mc.kind)},
      {"value", Mc.मूल्य_2},
  };
}

//===----------------------------------------------------------------------===//
// Hover
//===----------------------------------------------------------------------===//

llvm::json::मूल्य llvm::lsp::toJSON(स्थिरांक Hover &Hover) {
  llvm::json::Object Result{{"contents", toJSON(Hover.contents)}};
  जर (Hover.range)
    Result["range"] = toJSON(*Hover.range);
  परत std::move(Result);
}

//===----------------------------------------------------------------------===//
// DocumentSymbol
//===----------------------------------------------------------------------===//

llvm::json::मूल्य llvm::lsp::toJSON(स्थिरांक Document_चिन्ह &चिन्ह) {
  llvm::json::Object Result{{"name", चिन्ह.name},
                            {"kind", static_cast<पूर्णांक>(चिन्ह.kind)},
                            {"range", चिन्ह.range},
                            {"selectionRange", चिन्ह.selectionRange}};

  जर (!चिन्ह.detail.empty())
    Result["detail"] = चिन्ह.detail;
  जर (!चिन्ह.children.empty())
    Result["children"] = चिन्ह.children;
  परत std::move(Result);
}

//===----------------------------------------------------------------------===//
// DocumentSymbolParams
//===----------------------------------------------------------------------===//

तार्किक llvm::lsp::fromJSON(स्थिरांक llvm::json::मूल्य &मूल्य,
                         Document_चिन्ह_Params &Result, llvm::json::Path Path) {
  llvm::json::ObjectMapper O(मूल्य, Path);
  परत O && O.map("textDocument", Result.textDocument);
}

//===----------------------------------------------------------------------===//
// DiagnosticRelatedInformation
//===----------------------------------------------------------------------===//

तार्किक llvm::lsp::fromJSON(स्थिरांक llvm::json::मूल्य &मूल्य,
                         DiagnosticRelatedInformation &Result,
                         llvm::json::Path Path) {
  llvm::json::ObjectMapper O(मूल्य, Path);
  परत O && O.map("location", Result.location) &&
         O.map("message", Result.message);
}

llvm::json::मूल्य llvm::lsp::toJSON(स्थिरांक DiagnosticRelatedInformation &Info) {
  परत llvm::json::Object{
      {"location", Info.location},
      {"message", Info.message},
  };
}

//===----------------------------------------------------------------------===//
// Diagnostic
//===----------------------------------------------------------------------===//

llvm::json::मूल्य llvm::lsp::toJSON(DiagnosticTag Tag) {
  परत static_cast<पूर्णांक>(Tag);
}

तार्किक llvm::lsp::fromJSON(स्थिरांक llvm::json::मूल्य &मूल्य, DiagnosticTag &Result,
                         llvm::json::Path Path) {
  जर (std::optional<int64_t> I = मूल्य.getAsInteger()) {
    Result = (DiagnosticTag)*I;
    परत सत्य;
  }

  परत असत्य;
}

llvm::json::मूल्य llvm::lsp::toJSON(स्थिरांक Diagnostic &Diag) {
  llvm::json::Object Result{
      {"range", Diag.range},
      {"severity", (पूर्णांक)Diag.severity},
      {"message", Diag.message},
  };
  जर (Diag.category)
    Result["category"] = *Diag.category;
  जर (!Diag.source.empty())
    Result["source"] = Diag.source;
  जर (Diag.relatedInformation)
    Result["relatedInformation"] = *Diag.relatedInformation;
  जर (!Diag.tags.empty())
    Result["tags"] = Diag.tags;
  परत std::move(Result);
}

तार्किक llvm::lsp::fromJSON(स्थिरांक llvm::json::मूल्य &मूल्य, Diagnostic &Result,
                         llvm::json::Path Path) {
  llvm::json::ObjectMapper O(मूल्य, Path);
  जर (!O)
    परत असत्य;
  पूर्णांक Severity = 0;
  जर (!mapOptOrNull(मूल्य, "severity", Severity, Path))
    परत असत्य;
  Result.severity = (DiagnosticSeverity)Severity;

  परत O.map("range", Result.range) && O.map("message", Result.message) &&
         mapOptOrNull(मूल्य, "category", Result.category, Path) &&
         mapOptOrNull(मूल्य, "source", Result.source, Path) &&
         mapOptOrNull(मूल्य, "relatedInformation", Result.relatedInformation,
                      Path) &&
         mapOptOrNull(मूल्य, "tags", Result.tags, Path);
}

//===----------------------------------------------------------------------===//
// PublishDiagnosticsParams
//===----------------------------------------------------------------------===//

llvm::json::मूल्य llvm::lsp::toJSON(स्थिरांक PublishDiagnosticsParams &Params) {
  परत llvm::json::Object{
      {"uri", Params.uri},
      {"diagnostics", Params.diagnostics},
      {"version", Params.version},
  };
}

//===----------------------------------------------------------------------===//
// TextEdit
//===----------------------------------------------------------------------===//

तार्किक llvm::lsp::fromJSON(स्थिरांक llvm::json::मूल्य &मूल्य, TextEdit &Result,
                         llvm::json::Path Path) {
  llvm::json::ObjectMapper O(मूल्य, Path);
  परत O && O.map("range", Result.range) && O.map("newText", Result.newText);
}

llvm::json::मूल्य llvm::lsp::toJSON(स्थिरांक TextEdit &मूल्य) {
  परत llvm::json::Object{
      {"range", मूल्य.range},
      {"newText", मूल्य.newText},
  };
}

raw_ostream &llvm::lsp::चालक<<(raw_ostream &Os, स्थिरांक TextEdit &मूल्य) {
  Os << मूल्य.range << " => \"";
  llvm::printEscapedString(मूल्य.newText, Os);
  परत Os << '"';
}

//===----------------------------------------------------------------------===//
// CompletionItemKind
//===----------------------------------------------------------------------===//

तार्किक llvm::lsp::fromJSON(स्थिरांक llvm::json::मूल्य &मूल्य,
                         CompletionItemKind &Result, llvm::json::Path Path) {
  जर (std::optional<int64_t> Int_मूल्य = मूल्य.getAsInteger()) {
    जर (*Int_मूल्य < static_cast<पूर्णांक>(CompletionItemKind::Text) ||
        *Int_मूल्य > static_cast<पूर्णांक>(CompletionItemKind::TypeParameter))
      परत असत्य;
    Result = static_cast<CompletionItemKind>(*Int_मूल्य);
    परत सत्य;
  }
  परत असत्य;
}

CompletionItemKind llvm::lsp::adjustKindToCapability(
    CompletionItemKind Kind,
    CompletionItemKindBitset &SupportedCompletionItemKinds) {
  माप_प्रकार KindVal = static_cast<माप_प्रकार>(Kind);
  जर (KindVal >= kCompletionItemKindMin &&
      KindVal <= SupportedCompletionItemKinds.size() &&
      SupportedCompletionItemKinds[KindVal])
    परत Kind;

  // Provide some fall backs for common kinds that are close enough.
  निवड (Kind) {
  स्थिती CompletionItemKind::Folder:
    परत CompletionItemKind::File;
  स्थिती CompletionItemKind::EnumMember:
    परत CompletionItemKind::Enum;
  स्थिती CompletionItemKind::Struct:
    परत CompletionItemKind::Class;
  पूर्वनिर्धारित:
    परत CompletionItemKind::Text;
  }
}

तार्किक llvm::lsp::fromJSON(स्थिरांक llvm::json::मूल्य &मूल्य,
                         CompletionItemKindBitset &Result,
                         llvm::json::Path Path) {
  जर (स्थिरांक llvm::json::Array *Array_मूल्य = मूल्य.getAsArray()) {
    साठी (माप_प्रकार I = 0, E = Array_मूल्य->size(); I < E; ++I) {
      CompletionItemKind KindOut;
      जर (fromJSON((*Array_मूल्य)[I], KindOut, Path.index(I)))
        Result.set(माप_प्रकार(KindOut));
    }
    परत सत्य;
  }
  परत असत्य;
}

//===----------------------------------------------------------------------===//
// CompletionItem
//===----------------------------------------------------------------------===//

llvm::json::मूल्य llvm::lsp::toJSON(स्थिरांक CompletionItem &मूल्य) {
  प्रतिज्ञा(!मूल्य.label.empty() && "completion item label is required");
  llvm::json::Object Result{{"label", मूल्य.label}};
  जर (मूल्य.kind != CompletionItemKind::Missing)
    Result["kind"] = static_cast<पूर्णांक>(मूल्य.kind);
  जर (!मूल्य.detail.empty())
    Result["detail"] = मूल्य.detail;
  जर (मूल्य.documentation)
    Result["documentation"] = मूल्य.documentation;
  जर (!मूल्य.sortText.empty())
    Result["sortText"] = मूल्य.sortText;
  जर (!मूल्य.filterText.empty())
    Result["filterText"] = मूल्य.filterText;
  जर (!मूल्य.घाला_Text.empty())
    Result["insertText"] = मूल्य.घाला_Text;
  जर (मूल्य.घाला_Text_Format != घाला_Text_Format_2::Missing)
    Result["insertTextFormat"] = static_cast<पूर्णांक>(मूल्य.घाला_Text_Format);
  जर (मूल्य.textEdit)
    Result["textEdit"] = *मूल्य.textEdit;
  जर (!मूल्य.additionalTextEdits.empty()) {
    Result["additionalTextEdits"] =
        llvm::json::Array(मूल्य.additionalTextEdits);
  }
  जर (मूल्य.deprecated)
    Result["deprecated"] = मूल्य.deprecated;
  परत std::move(Result);
}

raw_ostream &llvm::lsp::चालक<<(raw_ostream &Os,
                                   स्थिरांक CompletionItem &मूल्य) {
  परत Os << मूल्य.label << " - " << toJSON(मूल्य);
}

तार्किक llvm::lsp::चालक<(स्थिरांक CompletionItem &Lhs,
                          स्थिरांक CompletionItem &Rhs) {
  परत (Lhs.sortText.empty() ? Lhs.label : Lhs.sortText) <
         (Rhs.sortText.empty() ? Rhs.label : Rhs.sortText);
}

//===----------------------------------------------------------------------===//
// CompletionList
//===----------------------------------------------------------------------===//

llvm::json::मूल्य llvm::lsp::toJSON(स्थिरांक CompletionList &मूल्य) {
  परत llvm::json::Object{
      {"isIncomplete", मूल्य.isIncomplete},
      {"items", llvm::json::Array(मूल्य.वस्तुसंच)},
  };
}

//===----------------------------------------------------------------------===//
// CompletionContext
//===----------------------------------------------------------------------===//

तार्किक llvm::lsp::fromJSON(स्थिरांक llvm::json::मूल्य &मूल्य,
                         CompletionContext &Result, llvm::json::Path Path) {
  llvm::json::ObjectMapper O(मूल्य, Path);
  पूर्णांक TriggerKind;
  जर (!O || !O.map("triggerKind", TriggerKind) ||
      !mapOptOrNull(मूल्य, "triggerCharacter", Result.triggerCharacter, Path))
    परत असत्य;
  Result.triggerKind = static_cast<CompletionTriggerKind>(TriggerKind);
  परत सत्य;
}

//===----------------------------------------------------------------------===//
// CompletionParams
//===----------------------------------------------------------------------===//

तार्किक llvm::lsp::fromJSON(स्थिरांक llvm::json::मूल्य &मूल्य,
                         CompletionParams &Result, llvm::json::Path Path) {
  जर (!fromJSON(मूल्य, static_cast<TextDocumentPositionParams &>(Result), Path))
    परत असत्य;
  जर (स्थिरांक llvm::json::मूल्य *Context = मूल्य.getAsObject()->get("context"))
    परत fromJSON(*Context, Result.context, Path.field("context"));
  परत सत्य;
}

//===----------------------------------------------------------------------===//
// ParameterInformation
//===----------------------------------------------------------------------===//

llvm::json::मूल्य llvm::lsp::toJSON(स्थिरांक ParameterInformation &मूल्य) {
  प्रतिज्ञा((मूल्य.labelOffsets || !मूल्य.labelString.empty()) &&
         "parameter information label is required");
  llvm::json::Object Result;
  जर (मूल्य.labelOffsets)
    Result["label"] = llvm::json::Array(
        {मूल्य.labelOffsets->first, मूल्य.labelOffsets->second});
  अन्यथा
    Result["label"] = मूल्य.labelString;
  जर (!मूल्य.documentation.empty())
    Result["documentation"] = मूल्य.documentation;
  परत std::move(Result);
}

//===----------------------------------------------------------------------===//
// SignatureInformation
//===----------------------------------------------------------------------===//

llvm::json::मूल्य llvm::lsp::toJSON(स्थिरांक SignatureInformation &मूल्य) {
  प्रतिज्ञा(!मूल्य.label.empty() && "signature information label is required");
  llvm::json::Object Result{
      {"label", मूल्य.label},
      {"parameters", llvm::json::Array(मूल्य.parameters)},
  };
  जर (!मूल्य.documentation.empty())
    Result["documentation"] = मूल्य.documentation;
  परत std::move(Result);
}

raw_ostream &llvm::lsp::चालक<<(raw_ostream &Os,
                                   स्थिरांक SignatureInformation &मूल्य) {
  परत Os << मूल्य.label << " - " << toJSON(मूल्य);
}

//===----------------------------------------------------------------------===//
// SignatureHelp
//===----------------------------------------------------------------------===//

llvm::json::मूल्य llvm::lsp::toJSON(स्थिरांक SignatureHelp &मूल्य) {
  प्रतिज्ञा(मूल्य.activeSignature >= 0 &&
         "Unexpected negative value for number of active signatures.");
  प्रतिज्ञा(मूल्य.activeParameter >= 0 &&
         "Unexpected negative value for active parameter index");
  परत llvm::json::Object{
      {"activeSignature", मूल्य.activeSignature},
      {"activeParameter", मूल्य.activeParameter},
      {"signatures", llvm::json::Array(मूल्य.signatures)},
  };
}

//===----------------------------------------------------------------------===//
// DocumentLinkParams
//===----------------------------------------------------------------------===//

तार्किक llvm::lsp::fromJSON(स्थिरांक llvm::json::मूल्य &मूल्य,
                         DocumentLinkParams &Result, llvm::json::Path Path) {
  llvm::json::ObjectMapper O(मूल्य, Path);
  परत O && O.map("textDocument", Result.textDocument);
}

//===----------------------------------------------------------------------===//
// DocumentLink
//===----------------------------------------------------------------------===//

llvm::json::मूल्य llvm::lsp::toJSON(स्थिरांक DocumentLink &मूल्य) {
  परत llvm::json::Object{
      {"range", मूल्य.range},
      {"target", मूल्य.target},
  };
}

//===----------------------------------------------------------------------===//
// InlayHintsParams
//===----------------------------------------------------------------------===//

तार्किक llvm::lsp::fromJSON(स्थिरांक llvm::json::मूल्य &मूल्य,
                         InlayHintsParams &Result, llvm::json::Path Path) {
  llvm::json::ObjectMapper O(मूल्य, Path);
  परत O && O.map("textDocument", Result.textDocument) &&
         O.map("range", Result.range);
}

//===----------------------------------------------------------------------===//
// InlayHint
//===----------------------------------------------------------------------===//

llvm::json::मूल्य llvm::lsp::toJSON(स्थिरांक InlayHint &मूल्य) {
  परत llvm::json::Object{{"position", मूल्य.position},
                            {"kind", (पूर्णांक)मूल्य.kind},
                            {"label", मूल्य.label},
                            {"paddingLeft", मूल्य.padding_डावा},
                            {"paddingRight", मूल्य.padding_उजवा}};
}
तार्किक llvm::lsp::चालक==(स्थिरांक InlayHint &Lhs, स्थिरांक InlayHint &Rhs) {
  परत std::tie(Lhs.position, Lhs.kind, Lhs.label) ==
         std::tie(Rhs.position, Rhs.kind, Rhs.label);
}
तार्किक llvm::lsp::चालक<(स्थिरांक InlayHint &Lhs, स्थिरांक InlayHint &Rhs) {
  परत std::tie(Lhs.position, Lhs.kind, Lhs.label) <
         std::tie(Rhs.position, Rhs.kind, Rhs.label);
}

llvm::raw_ostream &llvm::lsp::चालक<<(llvm::raw_ostream &Os,
                                         InlayHintKind मूल्य) {
  निवड (मूल्य) {
  स्थिती InlayHintKind::Parameter:
    परत Os << "parameter";
  स्थिती InlayHintKind::Type:
    परत Os << "type";
  }
  llvm_unreachable("Unknown InlayHintKind");
}

//===----------------------------------------------------------------------===//
// CodeActionContext
//===----------------------------------------------------------------------===//

तार्किक llvm::lsp::fromJSON(स्थिरांक llvm::json::मूल्य &मूल्य,
                         CodeActionContext &Result, llvm::json::Path Path) {
  llvm::json::ObjectMapper O(मूल्य, Path);
  जर (!O || !O.map("diagnostics", Result.diagnostics))
    परत असत्य;
  O.map("only", Result.only);
  परत सत्य;
}

//===----------------------------------------------------------------------===//
// CodeActionParams
//===----------------------------------------------------------------------===//

तार्किक llvm::lsp::fromJSON(स्थिरांक llvm::json::मूल्य &मूल्य,
                         CodeActionParams &Result, llvm::json::Path Path) {
  llvm::json::ObjectMapper O(मूल्य, Path);
  परत O && O.map("textDocument", Result.textDocument) &&
         O.map("range", Result.range) && O.map("context", Result.context);
}

//===----------------------------------------------------------------------===//
// WorkspaceEdit
//===----------------------------------------------------------------------===//

तार्किक llvm::lsp::fromJSON(स्थिरांक llvm::json::मूल्य &मूल्य, WorkspaceEdit &Result,
                         llvm::json::Path Path) {
  llvm::json::ObjectMapper O(मूल्य, Path);
  परत O && O.map("changes", Result.changes);
}

llvm::json::मूल्य llvm::lsp::toJSON(स्थिरांक WorkspaceEdit &मूल्य) {
  llvm::json::Object FileChanges;
  साठी (स्वतः &Change : मूल्य.changes)
    FileChanges[Change.first] = llvm::json::Array(Change.second);
  परत llvm::json::Object{{"changes", std::move(FileChanges)}};
}

//===----------------------------------------------------------------------===//
// CodeAction
//===----------------------------------------------------------------------===//

स्थिरांक llvm::StringLiteral CodeAction::kQuickFix = "quickfix";
स्थिरांक llvm::StringLiteral CodeAction::kRefactor = "refactor";
स्थिरांक llvm::StringLiteral CodeAction::kInfo = "info";

llvm::json::मूल्य llvm::lsp::toJSON(स्थिरांक CodeAction &मूल्य) {
  llvm::json::Object CodeAction{{"title", मूल्य.title}};
  जर (मूल्य.kind)
    CodeAction["kind"] = *मूल्य.kind;
  जर (मूल्य.diagnostics)
    CodeAction["diagnostics"] = llvm::json::Array(*मूल्य.diagnostics);
  जर (मूल्य.isPreferred)
    CodeAction["isPreferred"] = सत्य;
  जर (मूल्य.edit)
    CodeAction["edit"] = *मूल्य.edit;
  परत std::move(CodeAction);
}

//===----------------------------------------------------------------------===//
// ShowMessageParams
//===----------------------------------------------------------------------===//

llvm::json::मूल्य llvm::lsp::toJSON(स्थिरांक ShowMessageParams &Params) {
  स्वतः Out = llvm::json::Object{
      {"type", static_cast<पूर्णांक>(Params.type)},
      {"message", Params.message},
  };
  जर (Params.actions)
    Out["actions"] = *Params.actions;
  परत Out;
}

llvm::json::मूल्य llvm::lsp::toJSON(स्थिरांक MessageActionItem &Params) {
  परत llvm::json::Object{{"title", Params.title}};
}
