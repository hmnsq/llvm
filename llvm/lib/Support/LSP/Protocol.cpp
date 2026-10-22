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

#include "llvm/Support/LSP/Protocol.h"
#include "llvm/ADT/StringExtras.h"
#include "llvm/ADT/StringSet.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/JSON.h"
#include "llvm/Support/MemoryBuffer.h"
#include "llvm/Support/Path.h"
#include "llvm/Support/raw_ostream.h"

using namespace llvm;
using namespace llvm::lsp;

// Helper that doesn't treat `null` and absent fields as failures.
template <typename T>
static bool mapOptOrNull(const llvm::json::Value &Params,
                         llvm::StringLiteral Prop, T &Out,
                         llvm::json::Path Path) {
  const llvm::json::Object *O = Params.getAsObject();
  assert(O);

  // Field is missing or null.
  auto *V = O->get(Prop);
  만약 (!V || V->getAsNull())
    반환 true;
  반환 fromJSON(*V, Out, Path.field(Prop));
}

//===----------------------------------------------------------------------===//
// LSPError
//===----------------------------------------------------------------------===//

char LSPError::ID;

//===----------------------------------------------------------------------===//
// URIForFile
//===----------------------------------------------------------------------===//

static bool isWindowsPath(StringRef Path) {
  반환 Path.size() > 1 && llvm::isAlpha(Path[0]) && Path[1] == ':';
}

static bool isNetworkPath(StringRef Path) {
  반환 Path.size() > 2 && Path[0] == Path[1] &&
         llvm::sys::path::is_separator(Path[0]);
}

static bool shouldEscapeInURI(unsigned char C) {
  // Unreserved characters.
  만약 ((C >= 'a' && C <= 'z') || (C >= 'A' && C <= 'Z') ||
      (C >= '0' && C <= '9'))
    반환 false;

  switch (C) {
  case '-':
  case '_':
  case '.':
  case '~':
  // '/' is only reserved when parsing.
  case '/':
  // ':' is only reserved for relative URI paths, which we doesn't produce.
  case ':':
    반환 false;
  }
  반환 true;
}

/// Encodes a string according to percent-encoding.
/// - Unreserved characters are not escaped.
/// - Reserved characters always escaped with exceptions like '/'.
/// - All other characters are escaped.
static void percentEncode(StringRef Content, std::string &Out) {
  위해 (unsigned char C : Content) {
    만약 (shouldEscapeInURI(C)) {
      Out.push_back('%');
      Out.push_back(llvm::hexdigit(C / 16));
      Out.push_back(llvm::hexdigit(C % 16));
    } 아니면 {
      Out.push_back(C);
    }
  }
}

/// Decodes a string according to percent-encoding.
static std::string percentDecode(StringRef Content) {
  std::string Result;
  위해 (auto I = Content.begin(), E = Content.end(); I != E; ++I) {
    만약 (*I == '%' && I + 2 < Content.end() && llvm::isHexDigit(*(I + 1)) &&
        llvm::isHexDigit(*(I + 2))) {
      Result.push_back(llvm::hexFromNibbles(*(I + 1), *(I + 2)));
      I += 2;
    } 아니면 {
      Result.push_back(*I);
    }
  }
  반환 Result;
}

/// Return the set containing the supported URI schemes.
static StringSet<> &getSupportedSchemes() {
  static StringSet<> Schemes({"file", "test"});
  반환 Schemes;
}

/// Returns true if the given scheme is structurally valid, i.e. it does not
/// contain any invalid scheme characters. This does not check that the scheme
/// is actually supported.
static bool isStructurallyValidScheme(StringRef Scheme) {
  만약 (Scheme.empty())
    반환 false;
  만약 (!llvm::isAlpha(Scheme[0]))
    반환 false;
  반환 llvm::all_of(llvm::drop_begin(Scheme), [](char C) {
    반환 llvm::isAlnum(C) || C == '+' || C == '.' || C == '-';
  });
}

static llvm::Expected<std::string> uriFromAbsolutePath(StringRef AbsolutePath,
                                                       StringRef Scheme) {
  std::string Body;
  StringRef Authority;
  StringRef Root = llvm::sys::path::root_name(AbsolutePath);
  만약 (isNetworkPath(Root)) {
    // Windows UNC paths e.g. \\server\share => file://server/share
    Authority = Root.drop_front(2);
    AbsolutePath.consume_front(Root);
  } 아니면 만약 (isWindowsPath(Root)) {
    // Windows paths e.g. X:\path => file:///X:/path
    Body = "/";
  }
  Body += llvm::sys::path::convert_to_slash(AbsolutePath);

  std::string Uri = Scheme.str() + ":";
  만약 (Authority.empty() && Body.empty())
    반환 Uri;

  // If authority if empty, we only print body if it starts with "/"; otherwise,
  // the URI is invalid.
  만약 (!Authority.empty() || StringRef(Body).starts_with("/")) {
    Uri.append("//");
    percentEncode(Authority, Uri);
  }
  percentEncode(Body, Uri);
  반환 Uri;
}

static llvm::Expected<std::string> getAbsolutePath(StringRef Authority,
                                                   StringRef Body) {
  만약 (!Body.starts_with("/"))
    반환 llvm::createStringError(
        llvm::inconvertibleErrorCode(),
        "File scheme: expect body to be an absolute path starting "
        "with '/': " +
            Body);
  SmallString<128> Path;
  만약 (!Authority.empty()) {
    // Windows UNC paths e.g. file://server/share => \\server\share
    ("//" + Authority).toVector(Path);
  } 아니면 만약 (isWindowsPath(Body.substr(1))) {
    // Windows paths e.g. file:///X:/path => X:\path
    Body.consume_front("/");
  }
  Path.append(Body);
  llvm::sys::path::native(Path);
  반환 std::string(Path);
}

static llvm::Expected<std::string> parseFilePathFromURI(StringRef OrigUri) {
  StringRef Uri = OrigUri;

  // Decode the scheme of the URI.
  size_t Pos = Uri.find(':');
  만약 (Pos == StringRef::npos)
    반환 llvm::createStringError(llvm::inconvertibleErrorCode(),
                                   "Scheme must be provided in URI: " +
                                       OrigUri);
  StringRef SchemeStr = Uri.substr(0, Pos);
  std::string UriScheme = percentDecode(SchemeStr);
  만약 (!isStructurallyValidScheme(UriScheme))
    반환 llvm::createStringError(llvm::inconvertibleErrorCode(),
                                   "Invalid scheme: " + SchemeStr +
                                       " (decoded: " + UriScheme + ")");
  Uri = Uri.substr(Pos + 1);

  // Decode the authority of the URI.
  std::string UriAuthority;
  만약 (Uri.consume_front("//")) {
    Pos = Uri.find('/');
    UriAuthority = percentDecode(Uri.substr(0, Pos));
    Uri = Uri.substr(Pos);
  }

  // Decode the body of the URI.
  std::string UriBody = percentDecode(Uri);

  // Compute the absolute path for this uri.
  만약 (!getSupportedSchemes().contains(UriScheme)) {
    반환 llvm::createStringError(llvm::inconvertibleErrorCode(),
                                   "unsupported URI scheme `" + UriScheme +
                                       "' for workspace files");
  }
  반환 getAbsolutePath(UriAuthority, UriBody);
}

llvm::Expected<URIForFile> URIForFile::fromURI(StringRef Uri) {
  llvm::Expected<std::string> FilePath = parseFilePathFromURI(Uri);
  만약 (!FilePath)
    반환 FilePath.takeError();
  반환 URIForFile(std::move(*FilePath), Uri.str());
}

llvm::Expected<URIForFile> URIForFile::fromFile(StringRef AbsoluteFilepath,
                                                StringRef Scheme) {
  llvm::Expected<std::string> Uri =
      uriFromAbsolutePath(AbsoluteFilepath, Scheme);
  만약 (!Uri)
    반환 Uri.takeError();
  반환 fromURI(*Uri);
}

StringRef URIForFile::scheme() const { 반환 uri().split(':').first; }

void URIForFile::registerSupportedScheme(StringRef Scheme) {
  getSupportedSchemes().insert(Scheme);
}

bool llvm::lsp::fromJSON(const llvm::json::Value &Value, URIForFile &Result,
                         llvm::json::Path Path) {
  만약 (std::optional<StringRef> Str = Value.getAsString()) {
    llvm::Expected<URIForFile> ExpectedUri = URIForFile::fromURI(*Str);
    만약 (!ExpectedUri) {
      Path.report("unresolvable URI");
      consumeError(ExpectedUri.takeError());
      반환 false;
    }
    Result = std::move(*ExpectedUri);
    반환 true;
  }
  반환 false;
}

llvm::json::Value llvm::lsp::toJSON(const URIForFile &Value) {
  반환 Value.uri();
}

raw_ostream &llvm::lsp::operator<<(raw_ostream &Os, const URIForFile &Value) {
  반환 Os << Value.uri();
}

//===----------------------------------------------------------------------===//
// ClientCapabilities
//===----------------------------------------------------------------------===//

bool llvm::lsp::fromJSON(const llvm::json::Value &Value,
                         ClientCapabilities &Result, llvm::json::Path Path) {
  const llvm::json::Object *O = Value.getAsObject();
  만약 (!O) {
    Path.report("expected object");
    반환 false;
  }
  만약 (const llvm::json::Object *TextDocument = O->getObject("textDocument")) {
    만약 (const llvm::json::Object *DocumentSymbol =
            TextDocument->getObject("documentSymbol")) {
      만약 (std::optional<bool> HierarchicalSupport =
              DocumentSymbol->getBoolean("hierarchicalDocumentSymbolSupport"))
        Result.hierarchicalDocumentSymbol = *HierarchicalSupport;
    }
    만약 (auto *CodeAction = TextDocument->getObject("codeAction")) {
      만약 (CodeAction->getObject("codeActionLiteralSupport"))
        Result.codeActionStructure = true;
    }
  }
  만약 (auto *Window = O->getObject("window")) {
    만약 (std::optional<bool> WorkDoneProgressSupport =
            Window->getBoolean("workDoneProgress"))
      Result.workDoneProgress = *WorkDoneProgressSupport;
  }
  반환 true;
}

//===----------------------------------------------------------------------===//
// ClientInfo
//===----------------------------------------------------------------------===//

bool llvm::lsp::fromJSON(const llvm::json::Value &Value, ClientInfo &Result,
                         llvm::json::Path Path) {
  llvm::json::ObjectMapper O(Value, Path);
  만약 (!O || !O.map("name", Result.name))
    반환 false;

  // Don't fail if we can't parse version.
  O.map("version", Result.version);
  반환 true;
}

//===----------------------------------------------------------------------===//
// InitializeParams
//===----------------------------------------------------------------------===//

bool llvm::lsp::fromJSON(const llvm::json::Value &Value, TraceLevel &Result,
                         llvm::json::Path Path) {
  만약 (std::optional<StringRef> Str = Value.getAsString()) {
    만약 (*Str == "off") {
      Result = TraceLevel::Off;
      반환 true;
    }
    만약 (*Str == "messages") {
      Result = TraceLevel::Messages;
      반환 true;
    }
    만약 (*Str == "verbose") {
      Result = TraceLevel::Verbose;
      반환 true;
    }
  }
  반환 false;
}

bool llvm::lsp::fromJSON(const llvm::json::Value &Value,
                         InitializeParams &Result, llvm::json::Path Path) {
  llvm::json::ObjectMapper O(Value, Path);
  만약 (!O)
    반환 false;
  // We deliberately don't fail if we can't parse individual fields.
  O.map("capabilities", Result.capabilities);
  O.map("trace", Result.trace);
  O.map("rootUri", Result.rootUri);
  O.map("rootPath", Result.rootPath);
  mapOptOrNull(Value, "clientInfo", Result.clientInfo, Path);

  반환 true;
}

//===----------------------------------------------------------------------===//
// TextDocumentItem
//===----------------------------------------------------------------------===//

bool llvm::lsp::fromJSON(const llvm::json::Value &Value,
                         TextDocumentItem &Result, llvm::json::Path Path) {
  llvm::json::ObjectMapper O(Value, Path);
  반환 O && O.map("uri", Result.uri) &&
         O.map("languageId", Result.languageId) && O.map("text", Result.text) &&
         O.map("version", Result.version);
}

//===----------------------------------------------------------------------===//
// TextDocumentIdentifier
//===----------------------------------------------------------------------===//

llvm::json::Value llvm::lsp::toJSON(const TextDocumentIdentifier &Value) {
  반환 llvm::json::Object{{"uri", Value.uri}};
}

bool llvm::lsp::fromJSON(const llvm::json::Value &Value,
                         TextDocumentIdentifier &Result,
                         llvm::json::Path Path) {
  llvm::json::ObjectMapper O(Value, Path);
  반환 O && O.map("uri", Result.uri);
}

//===----------------------------------------------------------------------===//
// VersionedTextDocumentIdentifier
//===----------------------------------------------------------------------===//

llvm::json::Value
llvm::lsp::toJSON(const VersionedTextDocumentIdentifier &Value) {
  반환 llvm::json::Object{
      {"uri", Value.uri},
      {"version", Value.version},
  };
}

bool llvm::lsp::fromJSON(const llvm::json::Value &Value,
                         VersionedTextDocumentIdentifier &Result,
                         llvm::json::Path Path) {
  llvm::json::ObjectMapper O(Value, Path);
  반환 O && O.map("uri", Result.uri) && O.map("version", Result.version);
}

//===----------------------------------------------------------------------===//
// Position
//===----------------------------------------------------------------------===//

bool llvm::lsp::fromJSON(const llvm::json::Value &Value, Position &Result,
                         llvm::json::Path Path) {
  llvm::json::ObjectMapper O(Value, Path);
  반환 O && O.map("line", Result.line) &&
         O.map("character", Result.character);
}

llvm::json::Value llvm::lsp::toJSON(const Position &Value) {
  반환 llvm::json::Object{
      {"line", Value.line},
      {"character", Value.character},
  };
}

raw_ostream &llvm::lsp::operator<<(raw_ostream &Os, const Position &Value) {
  반환 Os << Value.line << ':' << Value.character;
}

//===----------------------------------------------------------------------===//
// Range
//===----------------------------------------------------------------------===//

bool llvm::lsp::fromJSON(const llvm::json::Value &Value, Range &Result,
                         llvm::json::Path Path) {
  llvm::json::ObjectMapper O(Value, Path);
  반환 O && O.map("start", Result.start) && O.map("end", Result.end);
}

llvm::json::Value llvm::lsp::toJSON(const Range &Value) {
  반환 llvm::json::Object{
      {"start", Value.start},
      {"end", Value.end},
  };
}

raw_ostream &llvm::lsp::operator<<(raw_ostream &Os, const Range &Value) {
  반환 Os << Value.start << '-' << Value.end;
}

//===----------------------------------------------------------------------===//
// Location
//===----------------------------------------------------------------------===//

bool llvm::lsp::fromJSON(const llvm::json::Value &Value, Location &Result,
                         llvm::json::Path Path) {
  llvm::json::ObjectMapper O(Value, Path);
  반환 O && O.map("uri", Result.uri) && O.map("range", Result.range);
}

llvm::json::Value llvm::lsp::toJSON(const Location &Value) {
  반환 llvm::json::Object{
      {"uri", Value.uri},
      {"range", Value.range},
  };
}

raw_ostream &llvm::lsp::operator<<(raw_ostream &Os, const Location &Value) {
  반환 Os << Value.range << '@' << Value.uri;
}

//===----------------------------------------------------------------------===//
// TextDocumentPositionParams
//===----------------------------------------------------------------------===//

bool llvm::lsp::fromJSON(const llvm::json::Value &Value,
                         TextDocumentPositionParams &Result,
                         llvm::json::Path Path) {
  llvm::json::ObjectMapper O(Value, Path);
  반환 O && O.map("textDocument", Result.textDocument) &&
         O.map("position", Result.position);
}

//===----------------------------------------------------------------------===//
// ReferenceParams
//===----------------------------------------------------------------------===//

bool llvm::lsp::fromJSON(const llvm::json::Value &Value,
                         ReferenceContext &Result, llvm::json::Path Path) {
  llvm::json::ObjectMapper O(Value, Path);
  반환 O && O.mapOptional("includeDeclaration", Result.includeDeclaration);
}

bool llvm::lsp::fromJSON(const llvm::json::Value &Value,
                         ReferenceParams &Result, llvm::json::Path Path) {
  TextDocumentPositionParams &Base = Result;
  llvm::json::ObjectMapper O(Value, Path);
  반환 fromJSON(Value, Base, Path) && O &&
         O.mapOptional("context", Result.context);
}

//===----------------------------------------------------------------------===//
// DidOpenTextDocumentParams
//===----------------------------------------------------------------------===//

bool llvm::lsp::fromJSON(const llvm::json::Value &Value,
                         DidOpenTextDocumentParams &Result,
                         llvm::json::Path Path) {
  llvm::json::ObjectMapper O(Value, Path);
  반환 O && O.map("textDocument", Result.textDocument);
}

//===----------------------------------------------------------------------===//
// DidCloseTextDocumentParams
//===----------------------------------------------------------------------===//

bool llvm::lsp::fromJSON(const llvm::json::Value &Value,
                         DidCloseTextDocumentParams &Result,
                         llvm::json::Path Path) {
  llvm::json::ObjectMapper O(Value, Path);
  반환 O && O.map("textDocument", Result.textDocument);
}

//===----------------------------------------------------------------------===//
// DidSaveTextDocumentParams
//===----------------------------------------------------------------------===//

bool llvm::lsp::fromJSON(const llvm::json::Value &Params,
                         DidSaveTextDocumentParams &R, llvm::json::Path P) {
  llvm::json::ObjectMapper O(Params, P);
  반환 O && O.map("textDocument", R.textDocument);
}

//===----------------------------------------------------------------------===//
// DidChangeTextDocumentParams
//===----------------------------------------------------------------------===//

LogicalResult
TextDocumentContentChangeEvent::applyTo(std::string &Contents) const {
  // If there is no range, the full document changed.
  만약 (!range) {
    Contents = text;
    반환 success();
  }

  // Try to map the replacement range to the content.
  llvm::SourceMgr TmpScrMgr;
  TmpScrMgr.AddNewSourceBuffer(llvm::MemoryBuffer::getMemBuffer(Contents),
                               SMLoc());
  SMRange RangeLoc = range->getAsSMRange(TmpScrMgr);
  만약 (!RangeLoc.isValid())
    반환 failure();

  Contents.replace(RangeLoc.Start.getPointer() - Contents.data(),
                   RangeLoc.End.getPointer() - RangeLoc.Start.getPointer(),
                   text);
  반환 success();
}

LogicalResult TextDocumentContentChangeEvent::applyTo(
    ArrayRef<TextDocumentContentChangeEvent> Changes, std::string &Contents) {
  위해 (const auto &Change : Changes)
    만약 (failed(Change.applyTo(Contents)))
      반환 failure();
  반환 success();
}

bool llvm::lsp::fromJSON(const llvm::json::Value &Value,
                         TextDocumentContentChangeEvent &Result,
                         llvm::json::Path Path) {
  llvm::json::ObjectMapper O(Value, Path);
  반환 O && O.map("range", Result.range) &&
         O.map("rangeLength", Result.rangeLength) && O.map("text", Result.text);
}

bool llvm::lsp::fromJSON(const llvm::json::Value &Value,
                         DidChangeTextDocumentParams &Result,
                         llvm::json::Path Path) {
  llvm::json::ObjectMapper O(Value, Path);
  반환 O && O.map("textDocument", Result.textDocument) &&
         O.map("contentChanges", Result.contentChanges);
}

//===----------------------------------------------------------------------===//
// MarkupContent
//===----------------------------------------------------------------------===//

static llvm::StringRef toTextKind(MarkupKind Kind) {
  switch (Kind) {
  case MarkupKind::PlainText:
    반환 "plaintext";
  case MarkupKind::Markdown:
    반환 "markdown";
  }
  llvm_unreachable("Invalid MarkupKind");
}

raw_ostream &llvm::lsp::operator<<(raw_ostream &Os, MarkupKind Kind) {
  반환 Os << toTextKind(Kind);
}

llvm::json::Value llvm::lsp::toJSON(const MarkupContent &Mc) {
  만약 (Mc.value.empty())
    반환 nullptr;

  반환 llvm::json::Object{
      {"kind", toTextKind(Mc.kind)},
      {"value", Mc.value},
  };
}

//===----------------------------------------------------------------------===//
// Hover
//===----------------------------------------------------------------------===//

llvm::json::Value llvm::lsp::toJSON(const Hover &Hover) {
  llvm::json::Object Result{{"contents", toJSON(Hover.contents)}};
  만약 (Hover.range)
    Result["range"] = toJSON(*Hover.range);
  반환 std::move(Result);
}

//===----------------------------------------------------------------------===//
// DocumentSymbol
//===----------------------------------------------------------------------===//

llvm::json::Value llvm::lsp::toJSON(const DocumentSymbol &Symbol) {
  llvm::json::Object Result{{"name", Symbol.name},
                            {"kind", static_cast<정수>(Symbol.kind)},
                            {"range", Symbol.range},
                            {"selectionRange", Symbol.selectionRange}};

  만약 (!Symbol.detail.empty())
    Result["detail"] = Symbol.detail;
  만약 (!Symbol.children.empty())
    Result["children"] = Symbol.children;
  반환 std::move(Result);
}

//===----------------------------------------------------------------------===//
// DocumentSymbolParams
//===----------------------------------------------------------------------===//

bool llvm::lsp::fromJSON(const llvm::json::Value &Value,
                         DocumentSymbolParams &Result, llvm::json::Path Path) {
  llvm::json::ObjectMapper O(Value, Path);
  반환 O && O.map("textDocument", Result.textDocument);
}

//===----------------------------------------------------------------------===//
// DiagnosticRelatedInformation
//===----------------------------------------------------------------------===//

bool llvm::lsp::fromJSON(const llvm::json::Value &Value,
                         DiagnosticRelatedInformation &Result,
                         llvm::json::Path Path) {
  llvm::json::ObjectMapper O(Value, Path);
  반환 O && O.map("location", Result.location) &&
         O.map("message", Result.message);
}

llvm::json::Value llvm::lsp::toJSON(const DiagnosticRelatedInformation &Info) {
  반환 llvm::json::Object{
      {"location", Info.location},
      {"message", Info.message},
  };
}

//===----------------------------------------------------------------------===//
// Diagnostic
//===----------------------------------------------------------------------===//

llvm::json::Value llvm::lsp::toJSON(DiagnosticTag Tag) {
  반환 static_cast<정수>(Tag);
}

bool llvm::lsp::fromJSON(const llvm::json::Value &Value, DiagnosticTag &Result,
                         llvm::json::Path Path) {
  만약 (std::optional<int64_t> I = Value.getAsInteger()) {
    Result = (DiagnosticTag)*I;
    반환 true;
  }

  반환 false;
}

llvm::json::Value llvm::lsp::toJSON(const Diagnostic &Diag) {
  llvm::json::Object Result{
      {"range", Diag.range},
      {"severity", (정수)Diag.severity},
      {"message", Diag.message},
  };
  만약 (Diag.category)
    Result["category"] = *Diag.category;
  만약 (!Diag.source.empty())
    Result["source"] = Diag.source;
  만약 (Diag.relatedInformation)
    Result["relatedInformation"] = *Diag.relatedInformation;
  만약 (!Diag.tags.empty())
    Result["tags"] = Diag.tags;
  반환 std::move(Result);
}

bool llvm::lsp::fromJSON(const llvm::json::Value &Value, Diagnostic &Result,
                         llvm::json::Path Path) {
  llvm::json::ObjectMapper O(Value, Path);
  만약 (!O)
    반환 false;
  정수 Severity = 0;
  만약 (!mapOptOrNull(Value, "severity", Severity, Path))
    반환 false;
  Result.severity = (DiagnosticSeverity)Severity;

  반환 O.map("range", Result.range) && O.map("message", Result.message) &&
         mapOptOrNull(Value, "category", Result.category, Path) &&
         mapOptOrNull(Value, "source", Result.source, Path) &&
         mapOptOrNull(Value, "relatedInformation", Result.relatedInformation,
                      Path) &&
         mapOptOrNull(Value, "tags", Result.tags, Path);
}

//===----------------------------------------------------------------------===//
// PublishDiagnosticsParams
//===----------------------------------------------------------------------===//

llvm::json::Value llvm::lsp::toJSON(const PublishDiagnosticsParams &Params) {
  반환 llvm::json::Object{
      {"uri", Params.uri},
      {"diagnostics", Params.diagnostics},
      {"version", Params.version},
  };
}

//===----------------------------------------------------------------------===//
// TextEdit
//===----------------------------------------------------------------------===//

bool llvm::lsp::fromJSON(const llvm::json::Value &Value, TextEdit &Result,
                         llvm::json::Path Path) {
  llvm::json::ObjectMapper O(Value, Path);
  반환 O && O.map("range", Result.range) && O.map("newText", Result.newText);
}

llvm::json::Value llvm::lsp::toJSON(const TextEdit &Value) {
  반환 llvm::json::Object{
      {"range", Value.range},
      {"newText", Value.newText},
  };
}

raw_ostream &llvm::lsp::operator<<(raw_ostream &Os, const TextEdit &Value) {
  Os << Value.range << " => \"";
  llvm::printEscapedString(Value.newText, Os);
  반환 Os << '"';
}

//===----------------------------------------------------------------------===//
// CompletionItemKind
//===----------------------------------------------------------------------===//

bool llvm::lsp::fromJSON(const llvm::json::Value &Value,
                         CompletionItemKind &Result, llvm::json::Path Path) {
  만약 (std::optional<int64_t> IntValue = Value.getAsInteger()) {
    만약 (*IntValue < static_cast<정수>(CompletionItemKind::Text) ||
        *IntValue > static_cast<정수>(CompletionItemKind::TypeParameter))
      반환 false;
    Result = static_cast<CompletionItemKind>(*IntValue);
    반환 true;
  }
  반환 false;
}

CompletionItemKind llvm::lsp::adjustKindToCapability(
    CompletionItemKind Kind,
    CompletionItemKindBitset &SupportedCompletionItemKinds) {
  size_t KindVal = static_cast<size_t>(Kind);
  만약 (KindVal >= kCompletionItemKindMin &&
      KindVal <= SupportedCompletionItemKinds.size() &&
      SupportedCompletionItemKinds[KindVal])
    반환 Kind;

  // Provide some fall backs for common kinds that are close enough.
  switch (Kind) {
  case CompletionItemKind::Folder:
    반환 CompletionItemKind::File;
  case CompletionItemKind::EnumMember:
    반환 CompletionItemKind::Enum;
  case CompletionItemKind::Struct:
    반환 CompletionItemKind::Class;
  default:
    반환 CompletionItemKind::Text;
  }
}

bool llvm::lsp::fromJSON(const llvm::json::Value &Value,
                         CompletionItemKindBitset &Result,
                         llvm::json::Path Path) {
  만약 (const llvm::json::Array *ArrayValue = Value.getAsArray()) {
    위해 (size_t I = 0, E = ArrayValue->size(); I < E; ++I) {
      CompletionItemKind KindOut;
      만약 (fromJSON((*ArrayValue)[I], KindOut, Path.index(I)))
        Result.set(size_t(KindOut));
    }
    반환 true;
  }
  반환 false;
}

//===----------------------------------------------------------------------===//
// CompletionItem
//===----------------------------------------------------------------------===//

llvm::json::Value llvm::lsp::toJSON(const CompletionItem &Value) {
  assert(!Value.label.empty() && "completion item label is required");
  llvm::json::Object Result{{"label", Value.label}};
  만약 (Value.kind != CompletionItemKind::Missing)
    Result["kind"] = static_cast<정수>(Value.kind);
  만약 (!Value.detail.empty())
    Result["detail"] = Value.detail;
  만약 (Value.documentation)
    Result["documentation"] = Value.documentation;
  만약 (!Value.sortText.empty())
    Result["sortText"] = Value.sortText;
  만약 (!Value.filterText.empty())
    Result["filterText"] = Value.filterText;
  만약 (!Value.insertText.empty())
    Result["insertText"] = Value.insertText;
  만약 (Value.insertTextFormat != InsertTextFormat::Missing)
    Result["insertTextFormat"] = static_cast<정수>(Value.insertTextFormat);
  만약 (Value.textEdit)
    Result["textEdit"] = *Value.textEdit;
  만약 (!Value.additionalTextEdits.empty()) {
    Result["additionalTextEdits"] =
        llvm::json::Array(Value.additionalTextEdits);
  }
  만약 (Value.deprecated)
    Result["deprecated"] = Value.deprecated;
  반환 std::move(Result);
}

raw_ostream &llvm::lsp::operator<<(raw_ostream &Os,
                                   const CompletionItem &Value) {
  반환 Os << Value.label << " - " << toJSON(Value);
}

bool llvm::lsp::operator<(const CompletionItem &Lhs,
                          const CompletionItem &Rhs) {
  반환 (Lhs.sortText.empty() ? Lhs.label : Lhs.sortText) <
         (Rhs.sortText.empty() ? Rhs.label : Rhs.sortText);
}

//===----------------------------------------------------------------------===//
// CompletionList
//===----------------------------------------------------------------------===//

llvm::json::Value llvm::lsp::toJSON(const CompletionList &Value) {
  반환 llvm::json::Object{
      {"isIncomplete", Value.isIncomplete},
      {"items", llvm::json::Array(Value.items)},
  };
}

//===----------------------------------------------------------------------===//
// CompletionContext
//===----------------------------------------------------------------------===//

bool llvm::lsp::fromJSON(const llvm::json::Value &Value,
                         CompletionContext &Result, llvm::json::Path Path) {
  llvm::json::ObjectMapper O(Value, Path);
  정수 TriggerKind;
  만약 (!O || !O.map("triggerKind", TriggerKind) ||
      !mapOptOrNull(Value, "triggerCharacter", Result.triggerCharacter, Path))
    반환 false;
  Result.triggerKind = static_cast<CompletionTriggerKind>(TriggerKind);
  반환 true;
}

//===----------------------------------------------------------------------===//
// CompletionParams
//===----------------------------------------------------------------------===//

bool llvm::lsp::fromJSON(const llvm::json::Value &Value,
                         CompletionParams &Result, llvm::json::Path Path) {
  만약 (!fromJSON(Value, static_cast<TextDocumentPositionParams &>(Result), Path))
    반환 false;
  만약 (const llvm::json::Value *Context = Value.getAsObject()->get("context"))
    반환 fromJSON(*Context, Result.context, Path.field("context"));
  반환 true;
}

//===----------------------------------------------------------------------===//
// ParameterInformation
//===----------------------------------------------------------------------===//

llvm::json::Value llvm::lsp::toJSON(const ParameterInformation &Value) {
  assert((Value.labelOffsets || !Value.labelString.empty()) &&
         "parameter information label is required");
  llvm::json::Object Result;
  만약 (Value.labelOffsets)
    Result["label"] = llvm::json::Array(
        {Value.labelOffsets->first, Value.labelOffsets->second});
  아니면
    Result["label"] = Value.labelString;
  만약 (!Value.documentation.empty())
    Result["documentation"] = Value.documentation;
  반환 std::move(Result);
}

//===----------------------------------------------------------------------===//
// SignatureInformation
//===----------------------------------------------------------------------===//

llvm::json::Value llvm::lsp::toJSON(const SignatureInformation &Value) {
  assert(!Value.label.empty() && "signature information label is required");
  llvm::json::Object Result{
      {"label", Value.label},
      {"parameters", llvm::json::Array(Value.parameters)},
  };
  만약 (!Value.documentation.empty())
    Result["documentation"] = Value.documentation;
  반환 std::move(Result);
}

raw_ostream &llvm::lsp::operator<<(raw_ostream &Os,
                                   const SignatureInformation &Value) {
  반환 Os << Value.label << " - " << toJSON(Value);
}

//===----------------------------------------------------------------------===//
// SignatureHelp
//===----------------------------------------------------------------------===//

llvm::json::Value llvm::lsp::toJSON(const SignatureHelp &Value) {
  assert(Value.activeSignature >= 0 &&
         "Unexpected negative value for number of active signatures.");
  assert(Value.activeParameter >= 0 &&
         "Unexpected negative value for active parameter index");
  반환 llvm::json::Object{
      {"activeSignature", Value.activeSignature},
      {"activeParameter", Value.activeParameter},
      {"signatures", llvm::json::Array(Value.signatures)},
  };
}

//===----------------------------------------------------------------------===//
// DocumentLinkParams
//===----------------------------------------------------------------------===//

bool llvm::lsp::fromJSON(const llvm::json::Value &Value,
                         DocumentLinkParams &Result, llvm::json::Path Path) {
  llvm::json::ObjectMapper O(Value, Path);
  반환 O && O.map("textDocument", Result.textDocument);
}

//===----------------------------------------------------------------------===//
// DocumentLink
//===----------------------------------------------------------------------===//

llvm::json::Value llvm::lsp::toJSON(const DocumentLink &Value) {
  반환 llvm::json::Object{
      {"range", Value.range},
      {"target", Value.target},
  };
}

//===----------------------------------------------------------------------===//
// InlayHintsParams
//===----------------------------------------------------------------------===//

bool llvm::lsp::fromJSON(const llvm::json::Value &Value,
                         InlayHintsParams &Result, llvm::json::Path Path) {
  llvm::json::ObjectMapper O(Value, Path);
  반환 O && O.map("textDocument", Result.textDocument) &&
         O.map("range", Result.range);
}

//===----------------------------------------------------------------------===//
// InlayHint
//===----------------------------------------------------------------------===//

llvm::json::Value llvm::lsp::toJSON(const InlayHint &Value) {
  반환 llvm::json::Object{{"position", Value.position},
                            {"kind", (정수)Value.kind},
                            {"label", Value.label},
                            {"paddingLeft", Value.paddingLeft},
                            {"paddingRight", Value.paddingRight}};
}
bool llvm::lsp::operator==(const InlayHint &Lhs, const InlayHint &Rhs) {
  반환 std::tie(Lhs.position, Lhs.kind, Lhs.label) ==
         std::tie(Rhs.position, Rhs.kind, Rhs.label);
}
bool llvm::lsp::operator<(const InlayHint &Lhs, const InlayHint &Rhs) {
  반환 std::tie(Lhs.position, Lhs.kind, Lhs.label) <
         std::tie(Rhs.position, Rhs.kind, Rhs.label);
}

llvm::raw_ostream &llvm::lsp::operator<<(llvm::raw_ostream &Os,
                                         InlayHintKind Value) {
  switch (Value) {
  case InlayHintKind::Parameter:
    반환 Os << "parameter";
  case InlayHintKind::Type:
    반환 Os << "type";
  }
  llvm_unreachable("Unknown InlayHintKind");
}

//===----------------------------------------------------------------------===//
// CodeActionContext
//===----------------------------------------------------------------------===//

bool llvm::lsp::fromJSON(const llvm::json::Value &Value,
                         CodeActionContext &Result, llvm::json::Path Path) {
  llvm::json::ObjectMapper O(Value, Path);
  만약 (!O || !O.map("diagnostics", Result.diagnostics))
    반환 false;
  O.map("only", Result.only);
  반환 true;
}

//===----------------------------------------------------------------------===//
// CodeActionParams
//===----------------------------------------------------------------------===//

bool llvm::lsp::fromJSON(const llvm::json::Value &Value,
                         CodeActionParams &Result, llvm::json::Path Path) {
  llvm::json::ObjectMapper O(Value, Path);
  반환 O && O.map("textDocument", Result.textDocument) &&
         O.map("range", Result.range) && O.map("context", Result.context);
}

//===----------------------------------------------------------------------===//
// WorkspaceEdit
//===----------------------------------------------------------------------===//

bool llvm::lsp::fromJSON(const llvm::json::Value &Value, WorkspaceEdit &Result,
                         llvm::json::Path Path) {
  llvm::json::ObjectMapper O(Value, Path);
  반환 O && O.map("changes", Result.changes);
}

llvm::json::Value llvm::lsp::toJSON(const WorkspaceEdit &Value) {
  llvm::json::Object FileChanges;
  위해 (auto &Change : Value.changes)
    FileChanges[Change.first] = llvm::json::Array(Change.second);
  반환 llvm::json::Object{{"changes", std::move(FileChanges)}};
}

//===----------------------------------------------------------------------===//
// CodeAction
//===----------------------------------------------------------------------===//

const llvm::StringLiteral CodeAction::kQuickFix = "quickfix";
const llvm::StringLiteral CodeAction::kRefactor = "refactor";
const llvm::StringLiteral CodeAction::kInfo = "info";

llvm::json::Value llvm::lsp::toJSON(const CodeAction &Value) {
  llvm::json::Object CodeAction{{"title", Value.title}};
  만약 (Value.kind)
    CodeAction["kind"] = *Value.kind;
  만약 (Value.diagnostics)
    CodeAction["diagnostics"] = llvm::json::Array(*Value.diagnostics);
  만약 (Value.isPreferred)
    CodeAction["isPreferred"] = true;
  만약 (Value.edit)
    CodeAction["edit"] = *Value.edit;
  반환 std::move(CodeAction);
}

//===----------------------------------------------------------------------===//
// ShowMessageParams
//===----------------------------------------------------------------------===//

llvm::json::Value llvm::lsp::toJSON(const ShowMessageParams &Params) {
  auto Out = llvm::json::Object{
      {"type", static_cast<정수>(Params.type)},
      {"message", Params.message},
  };
  만약 (Params.actions)
    Out["actions"] = *Params.actions;
  반환 Out;
}

llvm::json::Value llvm::lsp::toJSON(const MessageActionItem &Params) {
  반환 llvm::json::Object{{"title", Params.title}};
}
