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

#समावेश "llvm/Support/LSP/Protocol.h"
#समावेश "llvm/ADT/StringExtras.h"
#समावेश "llvm/ADT/StringSet.h"
#समावेश "llvm/Support/ErrorHandling.h"
#समावेश "llvm/Support/JSON.h"
#समावेश "llvm/Support/MemoryBuffer.h"
#समावेश "llvm/Support/Path.h"
#समावेश "llvm/Support/raw_ostream.h"

using namespace llvm;
using namespace llvm::lsp;

// Helper that doesn't treat `null` and absent fields as failures.
ढाँचा <typename T>
अटल bool mapOptOrNull(स्थिर llvm::json::मूल्य &Params,
                         llvm::StringLiteral Prop, T &Out,
                         llvm::json::Path Path) {
  स्थिर llvm::json::Object *O = Params.getAsObject();
  निश्चित(O);

  // Field is missing or null.
  स्वतः *V = O->लो(Prop);
  अगर (!V || V->getAsNull())
    वापस true;
  वापस fromJSON(*V, Out, Path.field(Prop));
}

//===----------------------------------------------------------------------===//
// LSPError
//===----------------------------------------------------------------------===//

अक्षर LSPError::ID;

//===----------------------------------------------------------------------===//
// URIForFile
//===----------------------------------------------------------------------===//

अटल bool isWindowsPath(StringRef Path) {
  वापस Path.आकार_2() > 1 && llvm::isAlpha(Path[0]) && Path[1] == ':';
}

अटल bool isNetworkPath(StringRef Path) {
  वापस Path.आकार_2() > 2 && Path[0] == Path[1] &&
         llvm::sys::path::is_separator(Path[0]);
}

अटल bool shouldEscapeInURI(अचिन्हित अक्षर C) {
  // Unreserved characters.
  अगर ((C >= 'a' && C <= 'z') || (C >= 'A' && C <= 'Z') ||
      (C >= '0' && C <= '9'))
    वापस false;

  चयन (C) {
  हाल '-':
  हाल '_':
  हाल '.':
  हाल '~':
  // '/' is only reserved when parsing.
  हाल '/':
  // ':' is only reserved for relative URI paths, which we doesn't produce.
  हाल ':':
    वापस false;
  }
  वापस true;
}

/// Encodes a string according to percent-encoding.
/// - Unreserved characters are not escaped.
/// - Reserved characters always escaped with exceptions like '/'.
/// - All other characters are escaped.
अटल व्योम percentEncode(StringRef Content, std::string &Out) {
  क्रम (अचिन्हित अक्षर C : Content) {
    अगर (shouldEscapeInURI(C)) {
      Out.धकेलो_back('%');
      Out.धकेलो_back(llvm::hexdigit(C / 16));
      Out.धकेलो_back(llvm::hexdigit(C % 16));
    } अन्यथा {
      Out.धकेलो_back(C);
    }
  }
}

/// Decodes a string according to percent-encoding.
अटल std::string percentDecode(StringRef Content) {
  std::string परिणाम;
  क्रम (स्वतः I = Content.begin(), E = Content.end(); I != E; ++I) {
    अगर (*I == '%' && I + 2 < Content.end() && llvm::isHexDigit(*(I + 1)) &&
        llvm::isHexDigit(*(I + 2))) {
      परिणाम.धकेलो_back(llvm::hexFromNibbles(*(I + 1), *(I + 2)));
      I += 2;
    } अन्यथा {
      परिणाम.धकेलो_back(*I);
    }
  }
  वापस परिणाम;
}

/// Return the set containing the supported URI schemes.
अटल StringSet<> &getSupportedSchemes() {
  अटल StringSet<> Schemes({"file", "test"});
  वापस Schemes;
}

/// Returns true if the given scheme is structurally valid, i.e. it does not
/// contain any invalid scheme characters. This does not check that the scheme
/// is actually supported.
अटल bool isStructurallyValidScheme(StringRef Scheme) {
  अगर (Scheme.empty())
    वापस false;
  अगर (!llvm::isAlpha(Scheme[0]))
    वापस false;
  वापस llvm::all_of(llvm::drop_begin(Scheme), [](अक्षर C) {
    वापस llvm::isAlnum(C) || C == '+' || C == '.' || C == '-';
  });
}

अटल llvm::Expected<std::string> uriFromAbsolutePath(StringRef AbsolutePath,
                                                       StringRef Scheme) {
  std::string Body;
  StringRef Authority;
  StringRef मूल_2 = llvm::sys::path::मूल_name(AbsolutePath);
  अगर (isNetworkPath(मूल_2)) {
    // Windows UNC paths e.g. \\server\share => file://server/share
    Authority = मूल_2.drop_front(2);
    AbsolutePath.consume_front(मूल_2);
  } अन्यथा अगर (isWindowsPath(मूल_2)) {
    // Windows paths e.g. X:\path => file:///X:/path
    Body = "/";
  }
  Body += llvm::sys::path::convert_to_slash(AbsolutePath);

  std::string Uri = Scheme.str() + ":";
  अगर (Authority.empty() && Body.empty())
    वापस Uri;

  // If authority if empty, we only print body if it starts with "/"; otherwise,
  // the URI is invalid.
  अगर (!Authority.empty() || StringRef(Body).starts_with("/")) {
    Uri.append("//");
    percentEncode(Authority, Uri);
  }
  percentEncode(Body, Uri);
  वापस Uri;
}

अटल llvm::Expected<std::string> getAbsolutePath(StringRef Authority,
                                                   StringRef Body) {
  अगर (!Body.starts_with("/"))
    वापस llvm::createStringError(
        llvm::inconvertibleErrorCode(),
        "File scheme: expect body to be an absolute path starting "
        "with '/': " +
            Body);
  SmallString<128> Path;
  अगर (!Authority.empty()) {
    // Windows UNC paths e.g. file://server/share => \\server\share
    ("//" + Authority).toVector(Path);
  } अन्यथा अगर (isWindowsPath(Body.substr(1))) {
    // Windows paths e.g. file:///X:/path => X:\path
    Body.consume_front("/");
  }
  Path.append(Body);
  llvm::sys::path::native(Path);
  वापस std::string(Path);
}

अटल llvm::Expected<std::string> parseFilePathFromURI(StringRef OrigUri) {
  StringRef Uri = OrigUri;

  // Decode the scheme of the URI.
  माप_प्रकार Pos = Uri.find(':');
  अगर (Pos == StringRef::npos)
    वापस llvm::createStringError(llvm::inconvertibleErrorCode(),
                                   "Scheme must be provided in URI: " +
                                       OrigUri);
  StringRef SchemeStr = Uri.substr(0, Pos);
  std::string UriScheme = percentDecode(SchemeStr);
  अगर (!isStructurallyValidScheme(UriScheme))
    वापस llvm::createStringError(llvm::inconvertibleErrorCode(),
                                   "Invalid scheme: " + SchemeStr +
                                       " (decoded: " + UriScheme + ")");
  Uri = Uri.substr(Pos + 1);

  // Decode the authority of the URI.
  std::string UriAuthority;
  अगर (Uri.consume_front("//")) {
    Pos = Uri.find('/');
    UriAuthority = percentDecode(Uri.substr(0, Pos));
    Uri = Uri.substr(Pos);
  }

  // Decode the body of the URI.
  std::string UriBody = percentDecode(Uri);

  // Compute the absolute path for this uri.
  अगर (!getSupportedSchemes().contains(UriScheme)) {
    वापस llvm::createStringError(llvm::inconvertibleErrorCode(),
                                   "unsupported URI scheme `" + UriScheme +
                                       "' for workspace files");
  }
  वापस getAbsolutePath(UriAuthority, UriBody);
}

llvm::Expected<URIForFile> URIForFile::fromURI(StringRef Uri) {
  llvm::Expected<std::string> FilePath = parseFilePathFromURI(Uri);
  अगर (!FilePath)
    वापस FilePath.takeError();
  वापस URIForFile(std::move(*FilePath), Uri.str());
}

llvm::Expected<URIForFile> URIForFile::fromFile(StringRef AbsoluteFilepath,
                                                StringRef Scheme) {
  llvm::Expected<std::string> Uri =
      uriFromAbsolutePath(AbsoluteFilepath, Scheme);
  अगर (!Uri)
    वापस Uri.takeError();
  वापस fromURI(*Uri);
}

StringRef URIForFile::scheme() स्थिर { वापस uri().split(':').first; }

व्योम URIForFile::registerSupportedScheme(StringRef Scheme) {
  getSupportedSchemes().जोड़ो(Scheme);
}

bool llvm::lsp::fromJSON(स्थिर llvm::json::मूल्य &मूल्य, URIForFile &परिणाम,
                         llvm::json::Path Path) {
  अगर (std::optional<StringRef> Str = मूल्य.getAsString()) {
    llvm::Expected<URIForFile> ExpectedUri = URIForFile::fromURI(*Str);
    अगर (!ExpectedUri) {
      Path.report("unresolvable URI");
      consumeError(ExpectedUri.takeError());
      वापस false;
    }
    परिणाम = std::move(*ExpectedUri);
    वापस true;
  }
  वापस false;
}

llvm::json::मूल्य llvm::lsp::toJSON(स्थिर URIForFile &मूल्य) {
  वापस मूल्य.uri();
}

raw_ostream &llvm::lsp::चालक<<(raw_ostream &Os, स्थिर URIForFile &मूल्य) {
  वापस Os << मूल्य.uri();
}

//===----------------------------------------------------------------------===//
// ClientCapabilities
//===----------------------------------------------------------------------===//

bool llvm::lsp::fromJSON(स्थिर llvm::json::मूल्य &मूल्य,
                         ClientCapabilities &परिणाम, llvm::json::Path Path) {
  स्थिर llvm::json::Object *O = मूल्य.getAsObject();
  अगर (!O) {
    Path.report("expected object");
    वापस false;
  }
  अगर (स्थिर llvm::json::Object *TextDocument = O->getObject("textDocument")) {
    अगर (स्थिर llvm::json::Object *Document_प्रतीक =
            TextDocument->getObject("documentSymbol")) {
      अगर (std::optional<bool> HierarchicalSupport =
              Document_प्रतीक->getBoolean("hierarchicalDocumentSymbolSupport"))
        परिणाम.hierarchical_Document_प्रतीक = *HierarchicalSupport;
    }
    अगर (स्वतः *CodeAction = TextDocument->getObject("codeAction")) {
      अगर (CodeAction->getObject("codeActionLiteralSupport"))
        परिणाम.codeActionStructure = true;
    }
  }
  अगर (स्वतः *Window = O->getObject("window")) {
    अगर (std::optional<bool> WorkDoneProgressSupport =
            Window->getBoolean("workDoneProgress"))
      परिणाम.workDoneProgress = *WorkDoneProgressSupport;
  }
  वापस true;
}

//===----------------------------------------------------------------------===//
// ClientInfo
//===----------------------------------------------------------------------===//

bool llvm::lsp::fromJSON(स्थिर llvm::json::मूल्य &मूल्य, ClientInfo &परिणाम,
                         llvm::json::Path Path) {
  llvm::json::ObjectMapper O(मूल्य, Path);
  अगर (!O || !O.map("name", परिणाम.name))
    वापस false;

  // Don't fail if we can't parse version.
  O.map("version", परिणाम.version);
  वापस true;
}

//===----------------------------------------------------------------------===//
// InitializeParams
//===----------------------------------------------------------------------===//

bool llvm::lsp::fromJSON(स्थिर llvm::json::मूल्य &मूल्य, TraceLevel &परिणाम,
                         llvm::json::Path Path) {
  अगर (std::optional<StringRef> Str = मूल्य.getAsString()) {
    अगर (*Str == "off") {
      परिणाम = TraceLevel::Off;
      वापस true;
    }
    अगर (*Str == "messages") {
      परिणाम = TraceLevel::Messages;
      वापस true;
    }
    अगर (*Str == "verbose") {
      परिणाम = TraceLevel::Verbose;
      वापस true;
    }
  }
  वापस false;
}

bool llvm::lsp::fromJSON(स्थिर llvm::json::मूल्य &मूल्य,
                         InitializeParams &परिणाम, llvm::json::Path Path) {
  llvm::json::ObjectMapper O(मूल्य, Path);
  अगर (!O)
    वापस false;
  // We deliberately don't fail if we can't parse individual fields.
  O.map("capabilities", परिणाम.capabilities);
  O.map("trace", परिणाम.trace);
  O.map("rootUri", परिणाम.मूल_Uri);
  O.map("rootPath", परिणाम.मूल_Path);
  mapOptOrNull(मूल्य, "clientInfo", परिणाम.clientInfo, Path);

  वापस true;
}

//===----------------------------------------------------------------------===//
// TextDocumentItem
//===----------------------------------------------------------------------===//

bool llvm::lsp::fromJSON(स्थिर llvm::json::मूल्य &मूल्य,
                         TextDocumentItem &परिणाम, llvm::json::Path Path) {
  llvm::json::ObjectMapper O(मूल्य, Path);
  वापस O && O.map("uri", परिणाम.uri) &&
         O.map("languageId", परिणाम.languageId) && O.map("text", परिणाम.text) &&
         O.map("version", परिणाम.version);
}

//===----------------------------------------------------------------------===//
// TextDocumentIdentifier
//===----------------------------------------------------------------------===//

llvm::json::मूल्य llvm::lsp::toJSON(स्थिर TextDocumentIdentifier &मूल्य) {
  वापस llvm::json::Object{{"uri", मूल्य.uri}};
}

bool llvm::lsp::fromJSON(स्थिर llvm::json::मूल्य &मूल्य,
                         TextDocumentIdentifier &परिणाम,
                         llvm::json::Path Path) {
  llvm::json::ObjectMapper O(मूल्य, Path);
  वापस O && O.map("uri", परिणाम.uri);
}

//===----------------------------------------------------------------------===//
// VersionedTextDocumentIdentifier
//===----------------------------------------------------------------------===//

llvm::json::मूल्य
llvm::lsp::toJSON(स्थिर VersionedTextDocumentIdentifier &मूल्य) {
  वापस llvm::json::Object{
      {"uri", मूल्य.uri},
      {"version", मूल्य.version},
  };
}

bool llvm::lsp::fromJSON(स्थिर llvm::json::मूल्य &मूल्य,
                         VersionedTextDocumentIdentifier &परिणाम,
                         llvm::json::Path Path) {
  llvm::json::ObjectMapper O(मूल्य, Path);
  वापस O && O.map("uri", परिणाम.uri) && O.map("version", परिणाम.version);
}

//===----------------------------------------------------------------------===//
// Position
//===----------------------------------------------------------------------===//

bool llvm::lsp::fromJSON(स्थिर llvm::json::मूल्य &मूल्य, Position &परिणाम,
                         llvm::json::Path Path) {
  llvm::json::ObjectMapper O(मूल्य, Path);
  वापस O && O.map("line", परिणाम.पंक्ति_3) &&
         O.map("character", परिणाम.character);
}

llvm::json::मूल्य llvm::lsp::toJSON(स्थिर Position &मूल्य) {
  वापस llvm::json::Object{
      {"line", मूल्य.पंक्ति_3},
      {"character", मूल्य.character},
  };
}

raw_ostream &llvm::lsp::चालक<<(raw_ostream &Os, स्थिर Position &मूल्य) {
  वापस Os << मूल्य.पंक्ति_3 << ':' << मूल्य.character;
}

//===----------------------------------------------------------------------===//
// Range
//===----------------------------------------------------------------------===//

bool llvm::lsp::fromJSON(स्थिर llvm::json::मूल्य &मूल्य, Range &परिणाम,
                         llvm::json::Path Path) {
  llvm::json::ObjectMapper O(मूल्य, Path);
  वापस O && O.map("start", परिणाम.start) && O.map("end", परिणाम.end);
}

llvm::json::मूल्य llvm::lsp::toJSON(स्थिर Range &मूल्य) {
  वापस llvm::json::Object{
      {"start", मूल्य.start},
      {"end", मूल्य.end},
  };
}

raw_ostream &llvm::lsp::चालक<<(raw_ostream &Os, स्थिर Range &मूल्य) {
  वापस Os << मूल्य.start << '-' << मूल्य.end;
}

//===----------------------------------------------------------------------===//
// Location
//===----------------------------------------------------------------------===//

bool llvm::lsp::fromJSON(स्थिर llvm::json::मूल्य &मूल्य, Location &परिणाम,
                         llvm::json::Path Path) {
  llvm::json::ObjectMapper O(मूल्य, Path);
  वापस O && O.map("uri", परिणाम.uri) && O.map("range", परिणाम.range);
}

llvm::json::मूल्य llvm::lsp::toJSON(स्थिर Location &मूल्य) {
  वापस llvm::json::Object{
      {"uri", मूल्य.uri},
      {"range", मूल्य.range},
  };
}

raw_ostream &llvm::lsp::चालक<<(raw_ostream &Os, स्थिर Location &मूल्य) {
  वापस Os << मूल्य.range << '@' << मूल्य.uri;
}

//===----------------------------------------------------------------------===//
// TextDocumentPositionParams
//===----------------------------------------------------------------------===//

bool llvm::lsp::fromJSON(स्थिर llvm::json::मूल्य &मूल्य,
                         TextDocumentPositionParams &परिणाम,
                         llvm::json::Path Path) {
  llvm::json::ObjectMapper O(मूल्य, Path);
  वापस O && O.map("textDocument", परिणाम.textDocument) &&
         O.map("position", परिणाम.position);
}

//===----------------------------------------------------------------------===//
// ReferenceParams
//===----------------------------------------------------------------------===//

bool llvm::lsp::fromJSON(स्थिर llvm::json::मूल्य &मूल्य,
                         ReferenceContext &परिणाम, llvm::json::Path Path) {
  llvm::json::ObjectMapper O(मूल्य, Path);
  वापस O && O.mapOptional("includeDeclaration", परिणाम.includeDeclaration);
}

bool llvm::lsp::fromJSON(स्थिर llvm::json::मूल्य &मूल्य,
                         ReferenceParams &परिणाम, llvm::json::Path Path) {
  TextDocumentPositionParams &Base = परिणाम;
  llvm::json::ObjectMapper O(मूल्य, Path);
  वापस fromJSON(मूल्य, Base, Path) && O &&
         O.mapOptional("context", परिणाम.context);
}

//===----------------------------------------------------------------------===//
// DidOpenTextDocumentParams
//===----------------------------------------------------------------------===//

bool llvm::lsp::fromJSON(स्थिर llvm::json::मूल्य &मूल्य,
                         DidOpenTextDocumentParams &परिणाम,
                         llvm::json::Path Path) {
  llvm::json::ObjectMapper O(मूल्य, Path);
  वापस O && O.map("textDocument", परिणाम.textDocument);
}

//===----------------------------------------------------------------------===//
// DidCloseTextDocumentParams
//===----------------------------------------------------------------------===//

bool llvm::lsp::fromJSON(स्थिर llvm::json::मूल्य &मूल्य,
                         DidCloseTextDocumentParams &परिणाम,
                         llvm::json::Path Path) {
  llvm::json::ObjectMapper O(मूल्य, Path);
  वापस O && O.map("textDocument", परिणाम.textDocument);
}

//===----------------------------------------------------------------------===//
// DidSaveTextDocumentParams
//===----------------------------------------------------------------------===//

bool llvm::lsp::fromJSON(स्थिर llvm::json::मूल्य &Params,
                         DidSaveTextDocumentParams &R, llvm::json::Path P) {
  llvm::json::ObjectMapper O(Params, P);
  वापस O && O.map("textDocument", R.textDocument);
}

//===----------------------------------------------------------------------===//
// DidChangeTextDocumentParams
//===----------------------------------------------------------------------===//

Logical_परिणाम
TextDocumentContentChangeEvent::applyTo(std::string &Contents) स्थिर {
  // If there is no range, the full document changed.
  अगर (!range) {
    Contents = text;
    वापस success();
  }

  // Try to map the replacement range to the content.
  llvm::SourceMgr TmpScrMgr;
  TmpScrMgr.Add_New_Source_बफर(llvm::Memory_बफर::get_Mem_बफर(Contents),
                               SMLoc());
  SMRange RangeLoc = range->getAsSMRange(TmpScrMgr);
  अगर (!RangeLoc.isValid())
    वापस failure();

  Contents.replace(RangeLoc.Start.getPointer() - Contents.तथ्य(),
                   RangeLoc.End.getPointer() - RangeLoc.Start.getPointer(),
                   text);
  वापस success();
}

Logical_परिणाम TextDocumentContentChangeEvent::applyTo(
    ArrayRef<TextDocumentContentChangeEvent> Changes, std::string &Contents) {
  क्रम (स्थिर स्वतः &Change : Changes)
    अगर (failed(Change.applyTo(Contents)))
      वापस failure();
  वापस success();
}

bool llvm::lsp::fromJSON(स्थिर llvm::json::मूल्य &मूल्य,
                         TextDocumentContentChangeEvent &परिणाम,
                         llvm::json::Path Path) {
  llvm::json::ObjectMapper O(मूल्य, Path);
  वापस O && O.map("range", परिणाम.range) &&
         O.map("rangeLength", परिणाम.range_लंबाई) && O.map("text", परिणाम.text);
}

bool llvm::lsp::fromJSON(स्थिर llvm::json::मूल्य &मूल्य,
                         DidChangeTextDocumentParams &परिणाम,
                         llvm::json::Path Path) {
  llvm::json::ObjectMapper O(मूल्य, Path);
  वापस O && O.map("textDocument", परिणाम.textDocument) &&
         O.map("contentChanges", परिणाम.contentChanges);
}

//===----------------------------------------------------------------------===//
// MarkupContent
//===----------------------------------------------------------------------===//

अटल llvm::StringRef toTextKind(MarkupKind Kind) {
  चयन (Kind) {
  हाल MarkupKind::PlainText:
    वापस "plaintext";
  हाल MarkupKind::Markdown:
    वापस "markdown";
  }
  llvm_unreachable("Invalid MarkupKind");
}

raw_ostream &llvm::lsp::चालक<<(raw_ostream &Os, MarkupKind Kind) {
  वापस Os << toTextKind(Kind);
}

llvm::json::मूल्य llvm::lsp::toJSON(स्थिर MarkupContent &Mc) {
  अगर (Mc.मूल्य_2.empty())
    वापस nullptr;

  वापस llvm::json::Object{
      {"kind", toTextKind(Mc.kind)},
      {"value", Mc.मूल्य_2},
  };
}

//===----------------------------------------------------------------------===//
// Hover
//===----------------------------------------------------------------------===//

llvm::json::मूल्य llvm::lsp::toJSON(स्थिर Hover &Hover) {
  llvm::json::Object परिणाम{{"contents", toJSON(Hover.contents)}};
  अगर (Hover.range)
    परिणाम["range"] = toJSON(*Hover.range);
  वापस std::move(परिणाम);
}

//===----------------------------------------------------------------------===//
// DocumentSymbol
//===----------------------------------------------------------------------===//

llvm::json::मूल्य llvm::lsp::toJSON(स्थिर Document_प्रतीक &प्रतीक) {
  llvm::json::Object परिणाम{{"name", प्रतीक.name},
                            {"kind", static_cast<पूर्णांक>(प्रतीक.kind)},
                            {"range", प्रतीक.range},
                            {"selectionRange", प्रतीक.selectionRange}};

  अगर (!प्रतीक.detail.empty())
    परिणाम["detail"] = प्रतीक.detail;
  अगर (!प्रतीक.children.empty())
    परिणाम["children"] = प्रतीक.children;
  वापस std::move(परिणाम);
}

//===----------------------------------------------------------------------===//
// DocumentSymbolParams
//===----------------------------------------------------------------------===//

bool llvm::lsp::fromJSON(स्थिर llvm::json::मूल्य &मूल्य,
                         Document_प्रतीक_Params &परिणाम, llvm::json::Path Path) {
  llvm::json::ObjectMapper O(मूल्य, Path);
  वापस O && O.map("textDocument", परिणाम.textDocument);
}

//===----------------------------------------------------------------------===//
// DiagnosticRelatedInformation
//===----------------------------------------------------------------------===//

bool llvm::lsp::fromJSON(स्थिर llvm::json::मूल्य &मूल्य,
                         DiagnosticRelatedInformation &परिणाम,
                         llvm::json::Path Path) {
  llvm::json::ObjectMapper O(मूल्य, Path);
  वापस O && O.map("location", परिणाम.location) &&
         O.map("message", परिणाम.message);
}

llvm::json::मूल्य llvm::lsp::toJSON(स्थिर DiagnosticRelatedInformation &Info) {
  वापस llvm::json::Object{
      {"location", Info.location},
      {"message", Info.message},
  };
}

//===----------------------------------------------------------------------===//
// Diagnostic
//===----------------------------------------------------------------------===//

llvm::json::मूल्य llvm::lsp::toJSON(DiagnosticTag Tag) {
  वापस static_cast<पूर्णांक>(Tag);
}

bool llvm::lsp::fromJSON(स्थिर llvm::json::मूल्य &मूल्य, DiagnosticTag &परिणाम,
                         llvm::json::Path Path) {
  अगर (std::optional<int64_t> I = मूल्य.getAsInteger()) {
    परिणाम = (DiagnosticTag)*I;
    वापस true;
  }

  वापस false;
}

llvm::json::मूल्य llvm::lsp::toJSON(स्थिर Diagnostic &Diag) {
  llvm::json::Object परिणाम{
      {"range", Diag.range},
      {"severity", (पूर्णांक)Diag.severity},
      {"message", Diag.message},
  };
  अगर (Diag.category)
    परिणाम["category"] = *Diag.category;
  अगर (!Diag.source.empty())
    परिणाम["source"] = Diag.source;
  अगर (Diag.relatedInformation)
    परिणाम["relatedInformation"] = *Diag.relatedInformation;
  अगर (!Diag.tags.empty())
    परिणाम["tags"] = Diag.tags;
  वापस std::move(परिणाम);
}

bool llvm::lsp::fromJSON(स्थिर llvm::json::मूल्य &मूल्य, Diagnostic &परिणाम,
                         llvm::json::Path Path) {
  llvm::json::ObjectMapper O(मूल्य, Path);
  अगर (!O)
    वापस false;
  पूर्णांक Severity = 0;
  अगर (!mapOptOrNull(मूल्य, "severity", Severity, Path))
    वापस false;
  परिणाम.severity = (DiagnosticSeverity)Severity;

  वापस O.map("range", परिणाम.range) && O.map("message", परिणाम.message) &&
         mapOptOrNull(मूल्य, "category", परिणाम.category, Path) &&
         mapOptOrNull(मूल्य, "source", परिणाम.source, Path) &&
         mapOptOrNull(मूल्य, "relatedInformation", परिणाम.relatedInformation,
                      Path) &&
         mapOptOrNull(मूल्य, "tags", परिणाम.tags, Path);
}

//===----------------------------------------------------------------------===//
// PublishDiagnosticsParams
//===----------------------------------------------------------------------===//

llvm::json::मूल्य llvm::lsp::toJSON(स्थिर PublishDiagnosticsParams &Params) {
  वापस llvm::json::Object{
      {"uri", Params.uri},
      {"diagnostics", Params.diagnostics},
      {"version", Params.version},
  };
}

//===----------------------------------------------------------------------===//
// TextEdit
//===----------------------------------------------------------------------===//

bool llvm::lsp::fromJSON(स्थिर llvm::json::मूल्य &मूल्य, TextEdit &परिणाम,
                         llvm::json::Path Path) {
  llvm::json::ObjectMapper O(मूल्य, Path);
  वापस O && O.map("range", परिणाम.range) && O.map("newText", परिणाम.newText);
}

llvm::json::मूल्य llvm::lsp::toJSON(स्थिर TextEdit &मूल्य) {
  वापस llvm::json::Object{
      {"range", मूल्य.range},
      {"newText", मूल्य.newText},
  };
}

raw_ostream &llvm::lsp::चालक<<(raw_ostream &Os, स्थिर TextEdit &मूल्य) {
  Os << मूल्य.range << " => \"";
  llvm::printEscapedString(मूल्य.newText, Os);
  वापस Os << '"';
}

//===----------------------------------------------------------------------===//
// CompletionItemKind
//===----------------------------------------------------------------------===//

bool llvm::lsp::fromJSON(स्थिर llvm::json::मूल्य &मूल्य,
                         CompletionItemKind &परिणाम, llvm::json::Path Path) {
  अगर (std::optional<int64_t> Int_मूल्य = मूल्य.getAsInteger()) {
    अगर (*Int_मूल्य < static_cast<पूर्णांक>(CompletionItemKind::Text) ||
        *Int_मूल्य > static_cast<पूर्णांक>(CompletionItemKind::TypeParameter))
      वापस false;
    परिणाम = static_cast<CompletionItemKind>(*Int_मूल्य);
    वापस true;
  }
  वापस false;
}

CompletionItemKind llvm::lsp::adjustKindToCapability(
    CompletionItemKind Kind,
    CompletionItemKindBitset &SupportedCompletionItemKinds) {
  माप_प्रकार KindVal = static_cast<माप_प्रकार>(Kind);
  अगर (KindVal >= kCompletionItemKindMin &&
      KindVal <= SupportedCompletionItemKinds.आकार_2() &&
      SupportedCompletionItemKinds[KindVal])
    वापस Kind;

  // Provide some fall backs for common kinds that are close enough.
  चयन (Kind) {
  हाल CompletionItemKind::Folder:
    वापस CompletionItemKind::File;
  हाल CompletionItemKind::EnumMember:
    वापस CompletionItemKind::Enum;
  हाल CompletionItemKind::Struct:
    वापस CompletionItemKind::Class;
  शेष:
    वापस CompletionItemKind::Text;
  }
}

bool llvm::lsp::fromJSON(स्थिर llvm::json::मूल्य &मूल्य,
                         CompletionItemKindBitset &परिणाम,
                         llvm::json::Path Path) {
  अगर (स्थिर llvm::json::Array *Array_मूल्य = मूल्य.getAsArray()) {
    क्रम (माप_प्रकार I = 0, E = Array_मूल्य->आकार_2(); I < E; ++I) {
      CompletionItemKind KindOut;
      अगर (fromJSON((*Array_मूल्य)[I], KindOut, Path.सूचकांक(I)))
        परिणाम.set(माप_प्रकार(KindOut));
    }
    वापस true;
  }
  वापस false;
}

//===----------------------------------------------------------------------===//
// CompletionItem
//===----------------------------------------------------------------------===//

llvm::json::मूल्य llvm::lsp::toJSON(स्थिर CompletionItem &मूल्य) {
  निश्चित(!मूल्य.label.empty() && "completion item label is required");
  llvm::json::Object परिणाम{{"label", मूल्य.label}};
  अगर (मूल्य.kind != CompletionItemKind::Missing)
    परिणाम["kind"] = static_cast<पूर्णांक>(मूल्य.kind);
  अगर (!मूल्य.detail.empty())
    परिणाम["detail"] = मूल्य.detail;
  अगर (मूल्य.documentation)
    परिणाम["documentation"] = मूल्य.documentation;
  अगर (!मूल्य.sortText.empty())
    परिणाम["sortText"] = मूल्य.sortText;
  अगर (!मूल्य.filterText.empty())
    परिणाम["filterText"] = मूल्य.filterText;
  अगर (!मूल्य.जोड़ो_Text.empty())
    परिणाम["insertText"] = मूल्य.जोड़ो_Text;
  अगर (मूल्य.जोड़ो_Text_Format != जोड़ो_Text_Format_2::Missing)
    परिणाम["insertTextFormat"] = static_cast<पूर्णांक>(मूल्य.जोड़ो_Text_Format);
  अगर (मूल्य.textEdit)
    परिणाम["textEdit"] = *मूल्य.textEdit;
  अगर (!मूल्य.additionalTextEdits.empty()) {
    परिणाम["additionalTextEdits"] =
        llvm::json::Array(मूल्य.additionalTextEdits);
  }
  अगर (मूल्य.deprecated)
    परिणाम["deprecated"] = मूल्य.deprecated;
  वापस std::move(परिणाम);
}

raw_ostream &llvm::lsp::चालक<<(raw_ostream &Os,
                                   स्थिर CompletionItem &मूल्य) {
  वापस Os << मूल्य.label << " - " << toJSON(मूल्य);
}

bool llvm::lsp::चालक<(स्थिर CompletionItem &Lhs,
                          स्थिर CompletionItem &Rhs) {
  वापस (Lhs.sortText.empty() ? Lhs.label : Lhs.sortText) <
         (Rhs.sortText.empty() ? Rhs.label : Rhs.sortText);
}

//===----------------------------------------------------------------------===//
// CompletionList
//===----------------------------------------------------------------------===//

llvm::json::मूल्य llvm::lsp::toJSON(स्थिर CompletionList &मूल्य) {
  वापस llvm::json::Object{
      {"isIncomplete", मूल्य.isIncomplete},
      {"items", llvm::json::Array(मूल्य.वस्तुएँ)},
  };
}

//===----------------------------------------------------------------------===//
// CompletionContext
//===----------------------------------------------------------------------===//

bool llvm::lsp::fromJSON(स्थिर llvm::json::मूल्य &मूल्य,
                         CompletionContext &परिणाम, llvm::json::Path Path) {
  llvm::json::ObjectMapper O(मूल्य, Path);
  पूर्णांक TriggerKind;
  अगर (!O || !O.map("triggerKind", TriggerKind) ||
      !mapOptOrNull(मूल्य, "triggerCharacter", परिणाम.triggerCharacter, Path))
    वापस false;
  परिणाम.triggerKind = static_cast<CompletionTriggerKind>(TriggerKind);
  वापस true;
}

//===----------------------------------------------------------------------===//
// CompletionParams
//===----------------------------------------------------------------------===//

bool llvm::lsp::fromJSON(स्थिर llvm::json::मूल्य &मूल्य,
                         CompletionParams &परिणाम, llvm::json::Path Path) {
  अगर (!fromJSON(मूल्य, static_cast<TextDocumentPositionParams &>(परिणाम), Path))
    वापस false;
  अगर (स्थिर llvm::json::मूल्य *Context = मूल्य.getAsObject()->लो("context"))
    वापस fromJSON(*Context, परिणाम.context, Path.field("context"));
  वापस true;
}

//===----------------------------------------------------------------------===//
// ParameterInformation
//===----------------------------------------------------------------------===//

llvm::json::मूल्य llvm::lsp::toJSON(स्थिर ParameterInformation &मूल्य) {
  निश्चित((मूल्य.labelOffsets || !मूल्य.labelString.empty()) &&
         "parameter information label is required");
  llvm::json::Object परिणाम;
  अगर (मूल्य.labelOffsets)
    परिणाम["label"] = llvm::json::Array(
        {मूल्य.labelOffsets->first, मूल्य.labelOffsets->second});
  अन्यथा
    परिणाम["label"] = मूल्य.labelString;
  अगर (!मूल्य.documentation.empty())
    परिणाम["documentation"] = मूल्य.documentation;
  वापस std::move(परिणाम);
}

//===----------------------------------------------------------------------===//
// SignatureInformation
//===----------------------------------------------------------------------===//

llvm::json::मूल्य llvm::lsp::toJSON(स्थिर SignatureInformation &मूल्य) {
  निश्चित(!मूल्य.label.empty() && "signature information label is required");
  llvm::json::Object परिणाम{
      {"label", मूल्य.label},
      {"parameters", llvm::json::Array(मूल्य.parameters)},
  };
  अगर (!मूल्य.documentation.empty())
    परिणाम["documentation"] = मूल्य.documentation;
  वापस std::move(परिणाम);
}

raw_ostream &llvm::lsp::चालक<<(raw_ostream &Os,
                                   स्थिर SignatureInformation &मूल्य) {
  वापस Os << मूल्य.label << " - " << toJSON(मूल्य);
}

//===----------------------------------------------------------------------===//
// SignatureHelp
//===----------------------------------------------------------------------===//

llvm::json::मूल्य llvm::lsp::toJSON(स्थिर SignatureHelp &मूल्य) {
  निश्चित(मूल्य.activeSignature >= 0 &&
         "Unexpected negative value for number of active signatures.");
  निश्चित(मूल्य.activeParameter >= 0 &&
         "Unexpected negative value for active parameter index");
  वापस llvm::json::Object{
      {"activeSignature", मूल्य.activeSignature},
      {"activeParameter", मूल्य.activeParameter},
      {"signatures", llvm::json::Array(मूल्य.signatures)},
  };
}

//===----------------------------------------------------------------------===//
// DocumentLinkParams
//===----------------------------------------------------------------------===//

bool llvm::lsp::fromJSON(स्थिर llvm::json::मूल्य &मूल्य,
                         DocumentLinkParams &परिणाम, llvm::json::Path Path) {
  llvm::json::ObjectMapper O(मूल्य, Path);
  वापस O && O.map("textDocument", परिणाम.textDocument);
}

//===----------------------------------------------------------------------===//
// DocumentLink
//===----------------------------------------------------------------------===//

llvm::json::मूल्य llvm::lsp::toJSON(स्थिर DocumentLink &मूल्य) {
  वापस llvm::json::Object{
      {"range", मूल्य.range},
      {"target", मूल्य.target},
  };
}

//===----------------------------------------------------------------------===//
// InlayHintsParams
//===----------------------------------------------------------------------===//

bool llvm::lsp::fromJSON(स्थिर llvm::json::मूल्य &मूल्य,
                         InlayHintsParams &परिणाम, llvm::json::Path Path) {
  llvm::json::ObjectMapper O(मूल्य, Path);
  वापस O && O.map("textDocument", परिणाम.textDocument) &&
         O.map("range", परिणाम.range);
}

//===----------------------------------------------------------------------===//
// InlayHint
//===----------------------------------------------------------------------===//

llvm::json::मूल्य llvm::lsp::toJSON(स्थिर InlayHint &मूल्य) {
  वापस llvm::json::Object{{"position", मूल्य.position},
                            {"kind", (पूर्णांक)मूल्य.kind},
                            {"label", मूल्य.label},
                            {"paddingLeft", मूल्य.padding_बायाँ},
                            {"paddingRight", मूल्य.padding_दायाँ}};
}
bool llvm::lsp::चालक==(स्थिर InlayHint &Lhs, स्थिर InlayHint &Rhs) {
  वापस std::tie(Lhs.position, Lhs.kind, Lhs.label) ==
         std::tie(Rhs.position, Rhs.kind, Rhs.label);
}
bool llvm::lsp::चालक<(स्थिर InlayHint &Lhs, स्थिर InlayHint &Rhs) {
  वापस std::tie(Lhs.position, Lhs.kind, Lhs.label) <
         std::tie(Rhs.position, Rhs.kind, Rhs.label);
}

llvm::raw_ostream &llvm::lsp::चालक<<(llvm::raw_ostream &Os,
                                         InlayHintKind मूल्य) {
  चयन (मूल्य) {
  हाल InlayHintKind::Parameter:
    वापस Os << "parameter";
  हाल InlayHintKind::Type:
    वापस Os << "type";
  }
  llvm_unreachable("Unknown InlayHintKind");
}

//===----------------------------------------------------------------------===//
// CodeActionContext
//===----------------------------------------------------------------------===//

bool llvm::lsp::fromJSON(स्थिर llvm::json::मूल्य &मूल्य,
                         CodeActionContext &परिणाम, llvm::json::Path Path) {
  llvm::json::ObjectMapper O(मूल्य, Path);
  अगर (!O || !O.map("diagnostics", परिणाम.diagnostics))
    वापस false;
  O.map("only", परिणाम.only);
  वापस true;
}

//===----------------------------------------------------------------------===//
// CodeActionParams
//===----------------------------------------------------------------------===//

bool llvm::lsp::fromJSON(स्थिर llvm::json::मूल्य &मूल्य,
                         CodeActionParams &परिणाम, llvm::json::Path Path) {
  llvm::json::ObjectMapper O(मूल्य, Path);
  वापस O && O.map("textDocument", परिणाम.textDocument) &&
         O.map("range", परिणाम.range) && O.map("context", परिणाम.context);
}

//===----------------------------------------------------------------------===//
// WorkspaceEdit
//===----------------------------------------------------------------------===//

bool llvm::lsp::fromJSON(स्थिर llvm::json::मूल्य &मूल्य, WorkspaceEdit &परिणाम,
                         llvm::json::Path Path) {
  llvm::json::ObjectMapper O(मूल्य, Path);
  वापस O && O.map("changes", परिणाम.changes);
}

llvm::json::मूल्य llvm::lsp::toJSON(स्थिर WorkspaceEdit &मूल्य) {
  llvm::json::Object FileChanges;
  क्रम (स्वतः &Change : मूल्य.changes)
    FileChanges[Change.first] = llvm::json::Array(Change.second);
  वापस llvm::json::Object{{"changes", std::move(FileChanges)}};
}

//===----------------------------------------------------------------------===//
// CodeAction
//===----------------------------------------------------------------------===//

स्थिर llvm::StringLiteral CodeAction::kQuickFix = "quickfix";
स्थिर llvm::StringLiteral CodeAction::kRefactor = "refactor";
स्थिर llvm::StringLiteral CodeAction::kInfo = "info";

llvm::json::मूल्य llvm::lsp::toJSON(स्थिर CodeAction &मूल्य) {
  llvm::json::Object CodeAction{{"title", मूल्य.title}};
  अगर (मूल्य.kind)
    CodeAction["kind"] = *मूल्य.kind;
  अगर (मूल्य.diagnostics)
    CodeAction["diagnostics"] = llvm::json::Array(*मूल्य.diagnostics);
  अगर (मूल्य.isPreferred)
    CodeAction["isPreferred"] = true;
  अगर (मूल्य.edit)
    CodeAction["edit"] = *मूल्य.edit;
  वापस std::move(CodeAction);
}

//===----------------------------------------------------------------------===//
// ShowMessageParams
//===----------------------------------------------------------------------===//

llvm::json::मूल्य llvm::lsp::toJSON(स्थिर ShowMessageParams &Params) {
  स्वतः Out = llvm::json::Object{
      {"type", static_cast<पूर्णांक>(Params.type)},
      {"message", Params.message},
  };
  अगर (Params.actions)
    Out["actions"] = *Params.actions;
  वापस Out;
}

llvm::json::मूल्य llvm::lsp::toJSON(स्थिर MessageActionItem &Params) {
  वापस llvm::json::Object{{"title", Params.title}};
}
