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

#અન્તર્ભાવ "llvm/Support/LSP/Protocol.h"
#અન્તર્ભાવ "llvm/ADT/StringExtras.h"
#અન્તર્ભાવ "llvm/ADT/StringSet.h"
#અન્તર્ભાવ "llvm/Support/ErrorHandling.h"
#અન્તર્ભાવ "llvm/Support/JSON.h"
#અન્તર્ભાવ "llvm/Support/MemoryBuffer.h"
#અન્તર્ભાવ "llvm/Support/Path.h"
#અન્તર્ભાવ "llvm/Support/raw_ostream.h"

using નામાકાશ llvm;
using નામાકાશ llvm::lsp;

// Helper that doesn't treat `null` and absent fields as failures.
template <typename T>
સ્થિર તાર્કિક mapOptOrNull(અચલ llvm::json::કિંમત &Params,
                         llvm::StringLiteral Prop, T &Out,
                         llvm::json::Path Path) {
  અચલ llvm::json::Object *O = Params.getAsObject();
  પ્રતિજ્ઞા(O);

  // Field is missing or null.
  auto *V = O->get(Prop);
  જો (!V || V->getAsNull())
    પાછા સાચું;
  પાછા fromJSON(*V, Out, Path.field(Prop));
}

//===----------------------------------------------------------------------===//
// LSPError
//===----------------------------------------------------------------------===//

અક્ષર LSPError::ID;

//===----------------------------------------------------------------------===//
// URIForFile
//===----------------------------------------------------------------------===//

સ્થિર તાર્કિક isWindowsPath(StringRef Path) {
  પાછા Path.size() > 1 && llvm::isAlpha(Path[0]) && Path[1] == ':';
}

સ્થિર તાર્કિક isNetworkPath(StringRef Path) {
  પાછા Path.size() > 2 && Path[0] == Path[1] &&
         llvm::sys::path::is_separator(Path[0]);
}

સ્થિર તાર્કિક shouldEscapeInURI(unsigned અક્ષર C) {
  // Unreserved characters.
  જો ((C >= 'a' && C <= 'z') || (C >= 'A' && C <= 'Z') ||
      (C >= '0' && C <= '9'))
    પાછા ખોટું;

  પસંદગી (C) {
  સ્થિતિ '-':
  સ્થિતિ '_':
  સ્થિતિ '.':
  સ્થિતિ '~':
  // '/' is only reserved when parsing.
  સ્થિતિ '/':
  // ':' is only reserved for relative URI paths, which we doesn't produce.
  સ્થિતિ ':':
    પાછા ખોટું;
  }
  પાછા સાચું;
}

/// Encodes a string according to percent-encoding.
/// - Unreserved characters are not escaped.
/// - Reserved characters always escaped with exceptions like '/'.
/// - All other characters are escaped.
સ્થિર શૂન્ય percentEncode(StringRef Content, std::string &Out) {
  માટે (unsigned અક્ષર C : Content) {
    જો (shouldEscapeInURI(C)) {
      Out.ધકેલો_back('%');
      Out.ધકેલો_back(llvm::hexdigit(C / 16));
      Out.ધકેલો_back(llvm::hexdigit(C % 16));
    } નહીંતર {
      Out.ધકેલો_back(C);
    }
  }
}

/// Decodes a string according to percent-encoding.
સ્થિર std::string percentDecode(StringRef Content) {
  std::string Result;
  માટે (auto I = Content.begin(), E = Content.end(); I != E; ++I) {
    જો (*I == '%' && I + 2 < Content.end() && llvm::isHexDigit(*(I + 1)) &&
        llvm::isHexDigit(*(I + 2))) {
      Result.ધકેલો_back(llvm::hexFromNibbles(*(I + 1), *(I + 2)));
      I += 2;
    } નહીંતર {
      Result.ધકેલો_back(*I);
    }
  }
  પાછા Result;
}

/// Return the set containing the supported URI schemes.
સ્થિર StringSet<> &getSupportedSchemes() {
  સ્થિર StringSet<> Schemes({"file", "test"});
  પાછા Schemes;
}

/// Returns true if the given scheme is structurally valid, i.e. it does not
/// contain any invalid scheme characters. This does not check that the scheme
/// is actually supported.
સ્થિર તાર્કિક isStructurallyValidScheme(StringRef Scheme) {
  જો (Scheme.empty())
    પાછા ખોટું;
  જો (!llvm::isAlpha(Scheme[0]))
    પાછા ખોટું;
  પાછા llvm::all_of(llvm::drop_begin(Scheme), [](અક્ષર C) {
    પાછા llvm::isAlnum(C) || C == '+' || C == '.' || C == '-';
  });
}

સ્થિર llvm::Expected<std::string> uriFromAbsolutePath(StringRef AbsolutePath,
                                                       StringRef Scheme) {
  std::string Body;
  StringRef Authority;
  StringRef મૂળ_2 = llvm::sys::path::મૂળ_name(AbsolutePath);
  જો (isNetworkPath(મૂળ_2)) {
    // Windows UNC paths e.g. \\server\share => file://server/share
    Authority = મૂળ_2.drop_front(2);
    AbsolutePath.consume_front(મૂળ_2);
  } નહીંતર જો (isWindowsPath(મૂળ_2)) {
    // Windows paths e.g. X:\path => file:///X:/path
    Body = "/";
  }
  Body += llvm::sys::path::convert_to_slash(AbsolutePath);

  std::string Uri = Scheme.str() + ":";
  જો (Authority.empty() && Body.empty())
    પાછા Uri;

  // If authority if empty, we only print body if it starts with "/"; otherwise,
  // the URI is invalid.
  જો (!Authority.empty() || StringRef(Body).starts_with("/")) {
    Uri.append("//");
    percentEncode(Authority, Uri);
  }
  percentEncode(Body, Uri);
  પાછા Uri;
}

સ્થિર llvm::Expected<std::string> getAbsolutePath(StringRef Authority,
                                                   StringRef Body) {
  જો (!Body.starts_with("/"))
    પાછા llvm::createStringError(
        llvm::inconvertibleErrorCode(),
        "File scheme: expect body to be an absolute path starting "
        "with '/': " +
            Body);
  SmallString<128> Path;
  જો (!Authority.empty()) {
    // Windows UNC paths e.g. file://server/share => \\server\share
    ("//" + Authority).toVector(Path);
  } નહીંતર જો (isWindowsPath(Body.substr(1))) {
    // Windows paths e.g. file:///X:/path => X:\path
    Body.consume_front("/");
  }
  Path.append(Body);
  llvm::sys::path::native(Path);
  પાછા std::string(Path);
}

સ્થિર llvm::Expected<std::string> parseFilePathFromURI(StringRef OrigUri) {
  StringRef Uri = OrigUri;

  // Decode the scheme of the URI.
  size_t Pos = Uri.find(':');
  જો (Pos == StringRef::npos)
    પાછા llvm::createStringError(llvm::inconvertibleErrorCode(),
                                   "Scheme must be provided in URI: " +
                                       OrigUri);
  StringRef SchemeStr = Uri.substr(0, Pos);
  std::string UriScheme = percentDecode(SchemeStr);
  જો (!isStructurallyValidScheme(UriScheme))
    પાછા llvm::createStringError(llvm::inconvertibleErrorCode(),
                                   "Invalid scheme: " + SchemeStr +
                                       " (decoded: " + UriScheme + ")");
  Uri = Uri.substr(Pos + 1);

  // Decode the authority of the URI.
  std::string UriAuthority;
  જો (Uri.consume_front("//")) {
    Pos = Uri.find('/');
    UriAuthority = percentDecode(Uri.substr(0, Pos));
    Uri = Uri.substr(Pos);
  }

  // Decode the body of the URI.
  std::string UriBody = percentDecode(Uri);

  // Compute the absolute path for this uri.
  જો (!getSupportedSchemes().contains(UriScheme)) {
    પાછા llvm::createStringError(llvm::inconvertibleErrorCode(),
                                   "unsupported URI scheme `" + UriScheme +
                                       "' for workspace files");
  }
  પાછા getAbsolutePath(UriAuthority, UriBody);
}

llvm::Expected<URIForFile> URIForFile::fromURI(StringRef Uri) {
  llvm::Expected<std::string> FilePath = parseFilePathFromURI(Uri);
  જો (!FilePath)
    પાછા FilePath.takeError();
  પાછા URIForFile(std::move(*FilePath), Uri.str());
}

llvm::Expected<URIForFile> URIForFile::fromFile(StringRef AbsoluteFilepath,
                                                StringRef Scheme) {
  llvm::Expected<std::string> Uri =
      uriFromAbsolutePath(AbsoluteFilepath, Scheme);
  જો (!Uri)
    પાછા Uri.takeError();
  પાછા fromURI(*Uri);
}

StringRef URIForFile::scheme() અચલ { પાછા uri().split(':').first; }

શૂન્ય URIForFile::registerSupportedScheme(StringRef Scheme) {
  getSupportedSchemes().ઉમેરો(Scheme);
}

તાર્કિક llvm::lsp::fromJSON(અચલ llvm::json::કિંમત &કિંમત, URIForFile &Result,
                         llvm::json::Path Path) {
  જો (std::optional<StringRef> Str = કિંમત.getAsString()) {
    llvm::Expected<URIForFile> ExpectedUri = URIForFile::fromURI(*Str);
    જો (!ExpectedUri) {
      Path.report("unresolvable URI");
      consumeError(ExpectedUri.takeError());
      પાછા ખોટું;
    }
    Result = std::move(*ExpectedUri);
    પાછા સાચું;
  }
  પાછા ખોટું;
}

llvm::json::કિંમત llvm::lsp::toJSON(અચલ URIForFile &કિંમત) {
  પાછા કિંમત.uri();
}

raw_ostream &llvm::lsp::operator<<(raw_ostream &Os, અચલ URIForFile &કિંમત) {
  પાછા Os << કિંમત.uri();
}

//===----------------------------------------------------------------------===//
// ClientCapabilities
//===----------------------------------------------------------------------===//

તાર્કિક llvm::lsp::fromJSON(અચલ llvm::json::કિંમત &કિંમત,
                         ClientCapabilities &Result, llvm::json::Path Path) {
  અચલ llvm::json::Object *O = કિંમત.getAsObject();
  જો (!O) {
    Path.report("expected object");
    પાછા ખોટું;
  }
  જો (અચલ llvm::json::Object *TextDocument = O->getObject("textDocument")) {
    જો (અચલ llvm::json::Object *Document_પ્રતીક =
            TextDocument->getObject("documentSymbol")) {
      જો (std::optional<તાર્કિક> HierarchicalSupport =
              Document_પ્રતીક->getBoolean("hierarchicalDocumentSymbolSupport"))
        Result.hierarchical_Document_પ્રતીક = *HierarchicalSupport;
    }
    જો (auto *CodeAction = TextDocument->getObject("codeAction")) {
      જો (CodeAction->getObject("codeActionLiteralSupport"))
        Result.codeActionStructure = સાચું;
    }
  }
  જો (auto *Window = O->getObject("window")) {
    જો (std::optional<તાર્કિક> WorkDoneProgressSupport =
            Window->getBoolean("workDoneProgress"))
      Result.workDoneProgress = *WorkDoneProgressSupport;
  }
  પાછા સાચું;
}

//===----------------------------------------------------------------------===//
// ClientInfo
//===----------------------------------------------------------------------===//

તાર્કિક llvm::lsp::fromJSON(અચલ llvm::json::કિંમત &કિંમત, ClientInfo &Result,
                         llvm::json::Path Path) {
  llvm::json::ObjectMapper O(કિંમત, Path);
  જો (!O || !O.map("name", Result.name))
    પાછા ખોટું;

  // Don't fail if we can't parse version.
  O.map("version", Result.version);
  પાછા સાચું;
}

//===----------------------------------------------------------------------===//
// InitializeParams
//===----------------------------------------------------------------------===//

તાર્કિક llvm::lsp::fromJSON(અચલ llvm::json::કિંમત &કિંમત, TraceLevel &Result,
                         llvm::json::Path Path) {
  જો (std::optional<StringRef> Str = કિંમત.getAsString()) {
    જો (*Str == "off") {
      Result = TraceLevel::Off;
      પાછા સાચું;
    }
    જો (*Str == "messages") {
      Result = TraceLevel::Messages;
      પાછા સાચું;
    }
    જો (*Str == "verbose") {
      Result = TraceLevel::Verbose;
      પાછા સાચું;
    }
  }
  પાછા ખોટું;
}

તાર્કિક llvm::lsp::fromJSON(અચલ llvm::json::કિંમત &કિંમત,
                         InitializeParams &Result, llvm::json::Path Path) {
  llvm::json::ObjectMapper O(કિંમત, Path);
  જો (!O)
    પાછા ખોટું;
  // We deliberately don't fail if we can't parse individual fields.
  O.map("capabilities", Result.capabilities);
  O.map("trace", Result.trace);
  O.map("rootUri", Result.મૂળ_Uri);
  O.map("rootPath", Result.મૂળ_Path);
  mapOptOrNull(કિંમત, "clientInfo", Result.clientInfo, Path);

  પાછા સાચું;
}

//===----------------------------------------------------------------------===//
// TextDocumentItem
//===----------------------------------------------------------------------===//

તાર્કિક llvm::lsp::fromJSON(અચલ llvm::json::કિંમત &કિંમત,
                         TextDocumentItem &Result, llvm::json::Path Path) {
  llvm::json::ObjectMapper O(કિંમત, Path);
  પાછા O && O.map("uri", Result.uri) &&
         O.map("languageId", Result.languageId) && O.map("text", Result.text) &&
         O.map("version", Result.version);
}

//===----------------------------------------------------------------------===//
// TextDocumentIdentifier
//===----------------------------------------------------------------------===//

llvm::json::કિંમત llvm::lsp::toJSON(અચલ TextDocumentIdentifier &કિંમત) {
  પાછા llvm::json::Object{{"uri", કિંમત.uri}};
}

તાર્કિક llvm::lsp::fromJSON(અચલ llvm::json::કિંમત &કિંમત,
                         TextDocumentIdentifier &Result,
                         llvm::json::Path Path) {
  llvm::json::ObjectMapper O(કિંમત, Path);
  પાછા O && O.map("uri", Result.uri);
}

//===----------------------------------------------------------------------===//
// VersionedTextDocumentIdentifier
//===----------------------------------------------------------------------===//

llvm::json::કિંમત
llvm::lsp::toJSON(અચલ VersionedTextDocumentIdentifier &કિંમત) {
  પાછા llvm::json::Object{
      {"uri", કિંમત.uri},
      {"version", કિંમત.version},
  };
}

તાર્કિક llvm::lsp::fromJSON(અચલ llvm::json::કિંમત &કિંમત,
                         VersionedTextDocumentIdentifier &Result,
                         llvm::json::Path Path) {
  llvm::json::ObjectMapper O(કિંમત, Path);
  પાછા O && O.map("uri", Result.uri) && O.map("version", Result.version);
}

//===----------------------------------------------------------------------===//
// Position
//===----------------------------------------------------------------------===//

તાર્કિક llvm::lsp::fromJSON(અચલ llvm::json::કિંમત &કિંમત, Position &Result,
                         llvm::json::Path Path) {
  llvm::json::ObjectMapper O(કિંમત, Path);
  પાછા O && O.map("line", Result.લીટી_2) &&
         O.map("character", Result.character);
}

llvm::json::કિંમત llvm::lsp::toJSON(અચલ Position &કિંમત) {
  પાછા llvm::json::Object{
      {"line", કિંમત.લીટી_2},
      {"character", કિંમત.character},
  };
}

raw_ostream &llvm::lsp::operator<<(raw_ostream &Os, અચલ Position &કિંમત) {
  પાછા Os << કિંમત.લીટી_2 << ':' << કિંમત.character;
}

//===----------------------------------------------------------------------===//
// Range
//===----------------------------------------------------------------------===//

તાર્કિક llvm::lsp::fromJSON(અચલ llvm::json::કિંમત &કિંમત, Range &Result,
                         llvm::json::Path Path) {
  llvm::json::ObjectMapper O(કિંમત, Path);
  પાછા O && O.map("start", Result.start) && O.map("end", Result.end);
}

llvm::json::કિંમત llvm::lsp::toJSON(અચલ Range &કિંમત) {
  પાછા llvm::json::Object{
      {"start", કિંમત.start},
      {"end", કિંમત.end},
  };
}

raw_ostream &llvm::lsp::operator<<(raw_ostream &Os, અચલ Range &કિંમત) {
  પાછા Os << કિંમત.start << '-' << કિંમત.end;
}

//===----------------------------------------------------------------------===//
// Location
//===----------------------------------------------------------------------===//

તાર્કિક llvm::lsp::fromJSON(અચલ llvm::json::કિંમત &કિંમત, Location &Result,
                         llvm::json::Path Path) {
  llvm::json::ObjectMapper O(કિંમત, Path);
  પાછા O && O.map("uri", Result.uri) && O.map("range", Result.range);
}

llvm::json::કિંમત llvm::lsp::toJSON(અચલ Location &કિંમત) {
  પાછા llvm::json::Object{
      {"uri", કિંમત.uri},
      {"range", કિંમત.range},
  };
}

raw_ostream &llvm::lsp::operator<<(raw_ostream &Os, અચલ Location &કિંમત) {
  પાછા Os << કિંમત.range << '@' << કિંમત.uri;
}

//===----------------------------------------------------------------------===//
// TextDocumentPositionParams
//===----------------------------------------------------------------------===//

તાર્કિક llvm::lsp::fromJSON(અચલ llvm::json::કિંમત &કિંમત,
                         TextDocumentPositionParams &Result,
                         llvm::json::Path Path) {
  llvm::json::ObjectMapper O(કિંમત, Path);
  પાછા O && O.map("textDocument", Result.textDocument) &&
         O.map("position", Result.position);
}

//===----------------------------------------------------------------------===//
// ReferenceParams
//===----------------------------------------------------------------------===//

તાર્કિક llvm::lsp::fromJSON(અચલ llvm::json::કિંમત &કિંમત,
                         ReferenceContext &Result, llvm::json::Path Path) {
  llvm::json::ObjectMapper O(કિંમત, Path);
  પાછા O && O.mapOptional("includeDeclaration", Result.includeDeclaration);
}

તાર્કિક llvm::lsp::fromJSON(અચલ llvm::json::કિંમત &કિંમત,
                         ReferenceParams &Result, llvm::json::Path Path) {
  TextDocumentPositionParams &Base = Result;
  llvm::json::ObjectMapper O(કિંમત, Path);
  પાછા fromJSON(કિંમત, Base, Path) && O &&
         O.mapOptional("context", Result.context);
}

//===----------------------------------------------------------------------===//
// DidOpenTextDocumentParams
//===----------------------------------------------------------------------===//

તાર્કિક llvm::lsp::fromJSON(અચલ llvm::json::કિંમત &કિંમત,
                         DidOpenTextDocumentParams &Result,
                         llvm::json::Path Path) {
  llvm::json::ObjectMapper O(કિંમત, Path);
  પાછા O && O.map("textDocument", Result.textDocument);
}

//===----------------------------------------------------------------------===//
// DidCloseTextDocumentParams
//===----------------------------------------------------------------------===//

તાર્કિક llvm::lsp::fromJSON(અચલ llvm::json::કિંમત &કિંમત,
                         DidCloseTextDocumentParams &Result,
                         llvm::json::Path Path) {
  llvm::json::ObjectMapper O(કિંમત, Path);
  પાછા O && O.map("textDocument", Result.textDocument);
}

//===----------------------------------------------------------------------===//
// DidSaveTextDocumentParams
//===----------------------------------------------------------------------===//

તાર્કિક llvm::lsp::fromJSON(અચલ llvm::json::કિંમત &Params,
                         DidSaveTextDocumentParams &R, llvm::json::Path P) {
  llvm::json::ObjectMapper O(Params, P);
  પાછા O && O.map("textDocument", R.textDocument);
}

//===----------------------------------------------------------------------===//
// DidChangeTextDocumentParams
//===----------------------------------------------------------------------===//

LogicalResult
TextDocumentContentChangeEvent::applyTo(std::string &Contents) અચલ {
  // If there is no range, the full document changed.
  જો (!range) {
    Contents = text;
    પાછા success();
  }

  // Try to map the replacement range to the content.
  llvm::SourceMgr TmpScrMgr;
  TmpScrMgr.AddNewSourceBuffer(llvm::MemoryBuffer::getMemBuffer(Contents),
                               SMLoc());
  SMRange RangeLoc = range->getAsSMRange(TmpScrMgr);
  જો (!RangeLoc.isValid())
    પાછા failure();

  Contents.replace(RangeLoc.Start.getPointer() - Contents.data(),
                   RangeLoc.End.getPointer() - RangeLoc.Start.getPointer(),
                   text);
  પાછા success();
}

LogicalResult TextDocumentContentChangeEvent::applyTo(
    ArrayRef<TextDocumentContentChangeEvent> Changes, std::string &Contents) {
  માટે (અચલ auto &Change : Changes)
    જો (failed(Change.applyTo(Contents)))
      પાછા failure();
  પાછા success();
}

તાર્કિક llvm::lsp::fromJSON(અચલ llvm::json::કિંમત &કિંમત,
                         TextDocumentContentChangeEvent &Result,
                         llvm::json::Path Path) {
  llvm::json::ObjectMapper O(કિંમત, Path);
  પાછા O && O.map("range", Result.range) &&
         O.map("rangeLength", Result.rangeLength) && O.map("text", Result.text);
}

તાર્કિક llvm::lsp::fromJSON(અચલ llvm::json::કિંમત &કિંમત,
                         DidChangeTextDocumentParams &Result,
                         llvm::json::Path Path) {
  llvm::json::ObjectMapper O(કિંમત, Path);
  પાછા O && O.map("textDocument", Result.textDocument) &&
         O.map("contentChanges", Result.contentChanges);
}

//===----------------------------------------------------------------------===//
// MarkupContent
//===----------------------------------------------------------------------===//

સ્થિર llvm::StringRef toTextKind(MarkupKind Kind) {
  પસંદગી (Kind) {
  સ્થિતિ MarkupKind::PlainText:
    પાછા "plaintext";
  સ્થિતિ MarkupKind::Markdown:
    પાછા "markdown";
  }
  llvm_unreachable("Invalid MarkupKind");
}

raw_ostream &llvm::lsp::operator<<(raw_ostream &Os, MarkupKind Kind) {
  પાછા Os << toTextKind(Kind);
}

llvm::json::કિંમત llvm::lsp::toJSON(અચલ MarkupContent &Mc) {
  જો (Mc.કિંમત_2.empty())
    પાછા nullptr;

  પાછા llvm::json::Object{
      {"kind", toTextKind(Mc.kind)},
      {"value", Mc.કિંમત_2},
  };
}

//===----------------------------------------------------------------------===//
// Hover
//===----------------------------------------------------------------------===//

llvm::json::કિંમત llvm::lsp::toJSON(અચલ Hover &Hover) {
  llvm::json::Object Result{{"contents", toJSON(Hover.contents)}};
  જો (Hover.range)
    Result["range"] = toJSON(*Hover.range);
  પાછા std::move(Result);
}

//===----------------------------------------------------------------------===//
// DocumentSymbol
//===----------------------------------------------------------------------===//

llvm::json::કિંમત llvm::lsp::toJSON(અચલ Document_પ્રતીક &પ્રતીક) {
  llvm::json::Object Result{{"name", પ્રતીક.name},
                            {"kind", static_cast<પૂર્ણાંક>(પ્રતીક.kind)},
                            {"range", પ્રતીક.range},
                            {"selectionRange", પ્રતીક.selectionRange}};

  જો (!પ્રતીક.detail.empty())
    Result["detail"] = પ્રતીક.detail;
  જો (!પ્રતીક.children.empty())
    Result["children"] = પ્રતીક.children;
  પાછા std::move(Result);
}

//===----------------------------------------------------------------------===//
// DocumentSymbolParams
//===----------------------------------------------------------------------===//

તાર્કિક llvm::lsp::fromJSON(અચલ llvm::json::કિંમત &કિંમત,
                         Document_પ્રતીક_Params &Result, llvm::json::Path Path) {
  llvm::json::ObjectMapper O(કિંમત, Path);
  પાછા O && O.map("textDocument", Result.textDocument);
}

//===----------------------------------------------------------------------===//
// DiagnosticRelatedInformation
//===----------------------------------------------------------------------===//

તાર્કિક llvm::lsp::fromJSON(અચલ llvm::json::કિંમત &કિંમત,
                         DiagnosticRelatedInformation &Result,
                         llvm::json::Path Path) {
  llvm::json::ObjectMapper O(કિંમત, Path);
  પાછા O && O.map("location", Result.location) &&
         O.map("message", Result.message);
}

llvm::json::કિંમત llvm::lsp::toJSON(અચલ DiagnosticRelatedInformation &Info) {
  પાછા llvm::json::Object{
      {"location", Info.location},
      {"message", Info.message},
  };
}

//===----------------------------------------------------------------------===//
// Diagnostic
//===----------------------------------------------------------------------===//

llvm::json::કિંમત llvm::lsp::toJSON(DiagnosticTag Tag) {
  પાછા static_cast<પૂર્ણાંક>(Tag);
}

તાર્કિક llvm::lsp::fromJSON(અચલ llvm::json::કિંમત &કિંમત, DiagnosticTag &Result,
                         llvm::json::Path Path) {
  જો (std::optional<int64_t> I = કિંમત.getAsInteger()) {
    Result = (DiagnosticTag)*I;
    પાછા સાચું;
  }

  પાછા ખોટું;
}

llvm::json::કિંમત llvm::lsp::toJSON(અચલ Diagnostic &Diag) {
  llvm::json::Object Result{
      {"range", Diag.range},
      {"severity", (પૂર્ણાંક)Diag.severity},
      {"message", Diag.message},
  };
  જો (Diag.category)
    Result["category"] = *Diag.category;
  જો (!Diag.source.empty())
    Result["source"] = Diag.source;
  જો (Diag.relatedInformation)
    Result["relatedInformation"] = *Diag.relatedInformation;
  જો (!Diag.tags.empty())
    Result["tags"] = Diag.tags;
  પાછા std::move(Result);
}

તાર્કિક llvm::lsp::fromJSON(અચલ llvm::json::કિંમત &કિંમત, Diagnostic &Result,
                         llvm::json::Path Path) {
  llvm::json::ObjectMapper O(કિંમત, Path);
  જો (!O)
    પાછા ખોટું;
  પૂર્ણાંક Severity = 0;
  જો (!mapOptOrNull(કિંમત, "severity", Severity, Path))
    પાછા ખોટું;
  Result.severity = (DiagnosticSeverity)Severity;

  પાછા O.map("range", Result.range) && O.map("message", Result.message) &&
         mapOptOrNull(કિંમત, "category", Result.category, Path) &&
         mapOptOrNull(કિંમત, "source", Result.source, Path) &&
         mapOptOrNull(કિંમત, "relatedInformation", Result.relatedInformation,
                      Path) &&
         mapOptOrNull(કિંમત, "tags", Result.tags, Path);
}

//===----------------------------------------------------------------------===//
// PublishDiagnosticsParams
//===----------------------------------------------------------------------===//

llvm::json::કિંમત llvm::lsp::toJSON(અચલ PublishDiagnosticsParams &Params) {
  પાછા llvm::json::Object{
      {"uri", Params.uri},
      {"diagnostics", Params.diagnostics},
      {"version", Params.version},
  };
}

//===----------------------------------------------------------------------===//
// TextEdit
//===----------------------------------------------------------------------===//

તાર્કિક llvm::lsp::fromJSON(અચલ llvm::json::કિંમત &કિંમત, TextEdit &Result,
                         llvm::json::Path Path) {
  llvm::json::ObjectMapper O(કિંમત, Path);
  પાછા O && O.map("range", Result.range) && O.map("newText", Result.newText);
}

llvm::json::કિંમત llvm::lsp::toJSON(અચલ TextEdit &કિંમત) {
  પાછા llvm::json::Object{
      {"range", કિંમત.range},
      {"newText", કિંમત.newText},
  };
}

raw_ostream &llvm::lsp::operator<<(raw_ostream &Os, અચલ TextEdit &કિંમત) {
  Os << કિંમત.range << " => \"";
  llvm::printEscapedString(કિંમત.newText, Os);
  પાછા Os << '"';
}

//===----------------------------------------------------------------------===//
// CompletionItemKind
//===----------------------------------------------------------------------===//

તાર્કિક llvm::lsp::fromJSON(અચલ llvm::json::કિંમત &કિંમત,
                         CompletionItemKind &Result, llvm::json::Path Path) {
  જો (std::optional<int64_t> Int_કિંમત = કિંમત.getAsInteger()) {
    જો (*Int_કિંમત < static_cast<પૂર્ણાંક>(CompletionItemKind::Text) ||
        *Int_કિંમત > static_cast<પૂર્ણાંક>(CompletionItemKind::TypeParameter))
      પાછા ખોટું;
    Result = static_cast<CompletionItemKind>(*Int_કિંમત);
    પાછા સાચું;
  }
  પાછા ખોટું;
}

CompletionItemKind llvm::lsp::adjustKindToCapability(
    CompletionItemKind Kind,
    CompletionItemKindBitset &SupportedCompletionItemKinds) {
  size_t KindVal = static_cast<size_t>(Kind);
  જો (KindVal >= kCompletionItemKindMin &&
      KindVal <= SupportedCompletionItemKinds.size() &&
      SupportedCompletionItemKinds[KindVal])
    પાછા Kind;

  // Provide some fall backs for common kinds that are close enough.
  પસંદગી (Kind) {
  સ્થિતિ CompletionItemKind::Folder:
    પાછા CompletionItemKind::File;
  સ્થિતિ CompletionItemKind::EnumMember:
    પાછા CompletionItemKind::Enum;
  સ્થિતિ CompletionItemKind::Struct:
    પાછા CompletionItemKind::Class;
  મૂળભૂત:
    પાછા CompletionItemKind::Text;
  }
}

તાર્કિક llvm::lsp::fromJSON(અચલ llvm::json::કિંમત &કિંમત,
                         CompletionItemKindBitset &Result,
                         llvm::json::Path Path) {
  જો (અચલ llvm::json::Array *Array_કિંમત = કિંમત.getAsArray()) {
    માટે (size_t I = 0, E = Array_કિંમત->size(); I < E; ++I) {
      CompletionItemKind KindOut;
      જો (fromJSON((*Array_કિંમત)[I], KindOut, Path.index(I)))
        Result.set(size_t(KindOut));
    }
    પાછા સાચું;
  }
  પાછા ખોટું;
}

//===----------------------------------------------------------------------===//
// CompletionItem
//===----------------------------------------------------------------------===//

llvm::json::કિંમત llvm::lsp::toJSON(અચલ CompletionItem &કિંમત) {
  પ્રતિજ્ઞા(!કિંમત.label.empty() && "completion item label is required");
  llvm::json::Object Result{{"label", કિંમત.label}};
  જો (કિંમત.kind != CompletionItemKind::Missing)
    Result["kind"] = static_cast<પૂર્ણાંક>(કિંમત.kind);
  જો (!કિંમત.detail.empty())
    Result["detail"] = કિંમત.detail;
  જો (કિંમત.documentation)
    Result["documentation"] = કિંમત.documentation;
  જો (!કિંમત.sortText.empty())
    Result["sortText"] = કિંમત.sortText;
  જો (!કિંમત.filterText.empty())
    Result["filterText"] = કિંમત.filterText;
  જો (!કિંમત.ઉમેરો_Text.empty())
    Result["insertText"] = કિંમત.ઉમેરો_Text;
  જો (કિંમત.ઉમેરો_Text_Format != ઉમેરો_Text_Format_2::Missing)
    Result["insertTextFormat"] = static_cast<પૂર્ણાંક>(કિંમત.ઉમેરો_Text_Format);
  જો (કિંમત.textEdit)
    Result["textEdit"] = *કિંમત.textEdit;
  જો (!કિંમત.additionalTextEdits.empty()) {
    Result["additionalTextEdits"] =
        llvm::json::Array(કિંમત.additionalTextEdits);
  }
  જો (કિંમત.deprecated)
    Result["deprecated"] = કિંમત.deprecated;
  પાછા std::move(Result);
}

raw_ostream &llvm::lsp::operator<<(raw_ostream &Os,
                                   અચલ CompletionItem &કિંમત) {
  પાછા Os << કિંમત.label << " - " << toJSON(કિંમત);
}

તાર્કિક llvm::lsp::operator<(અચલ CompletionItem &Lhs,
                          અચલ CompletionItem &Rhs) {
  પાછા (Lhs.sortText.empty() ? Lhs.label : Lhs.sortText) <
         (Rhs.sortText.empty() ? Rhs.label : Rhs.sortText);
}

//===----------------------------------------------------------------------===//
// CompletionList
//===----------------------------------------------------------------------===//

llvm::json::કિંમત llvm::lsp::toJSON(અચલ CompletionList &કિંમત) {
  પાછા llvm::json::Object{
      {"isIncomplete", કિંમત.isIncomplete},
      {"items", llvm::json::Array(કિંમત.વસ્તુઓ)},
  };
}

//===----------------------------------------------------------------------===//
// CompletionContext
//===----------------------------------------------------------------------===//

તાર્કિક llvm::lsp::fromJSON(અચલ llvm::json::કિંમત &કિંમત,
                         CompletionContext &Result, llvm::json::Path Path) {
  llvm::json::ObjectMapper O(કિંમત, Path);
  પૂર્ણાંક TriggerKind;
  જો (!O || !O.map("triggerKind", TriggerKind) ||
      !mapOptOrNull(કિંમત, "triggerCharacter", Result.triggerCharacter, Path))
    પાછા ખોટું;
  Result.triggerKind = static_cast<CompletionTriggerKind>(TriggerKind);
  પાછા સાચું;
}

//===----------------------------------------------------------------------===//
// CompletionParams
//===----------------------------------------------------------------------===//

તાર્કિક llvm::lsp::fromJSON(અચલ llvm::json::કિંમત &કિંમત,
                         CompletionParams &Result, llvm::json::Path Path) {
  જો (!fromJSON(કિંમત, static_cast<TextDocumentPositionParams &>(Result), Path))
    પાછા ખોટું;
  જો (અચલ llvm::json::કિંમત *Context = કિંમત.getAsObject()->get("context"))
    પાછા fromJSON(*Context, Result.context, Path.field("context"));
  પાછા સાચું;
}

//===----------------------------------------------------------------------===//
// ParameterInformation
//===----------------------------------------------------------------------===//

llvm::json::કિંમત llvm::lsp::toJSON(અચલ ParameterInformation &કિંમત) {
  પ્રતિજ્ઞા((કિંમત.labelOffsets || !કિંમત.labelString.empty()) &&
         "parameter information label is required");
  llvm::json::Object Result;
  જો (કિંમત.labelOffsets)
    Result["label"] = llvm::json::Array(
        {કિંમત.labelOffsets->first, કિંમત.labelOffsets->second});
  નહીંતર
    Result["label"] = કિંમત.labelString;
  જો (!કિંમત.documentation.empty())
    Result["documentation"] = કિંમત.documentation;
  પાછા std::move(Result);
}

//===----------------------------------------------------------------------===//
// SignatureInformation
//===----------------------------------------------------------------------===//

llvm::json::કિંમત llvm::lsp::toJSON(અચલ SignatureInformation &કિંમત) {
  પ્રતિજ્ઞા(!કિંમત.label.empty() && "signature information label is required");
  llvm::json::Object Result{
      {"label", કિંમત.label},
      {"parameters", llvm::json::Array(કિંમત.parameters)},
  };
  જો (!કિંમત.documentation.empty())
    Result["documentation"] = કિંમત.documentation;
  પાછા std::move(Result);
}

raw_ostream &llvm::lsp::operator<<(raw_ostream &Os,
                                   અચલ SignatureInformation &કિંમત) {
  પાછા Os << કિંમત.label << " - " << toJSON(કિંમત);
}

//===----------------------------------------------------------------------===//
// SignatureHelp
//===----------------------------------------------------------------------===//

llvm::json::કિંમત llvm::lsp::toJSON(અચલ SignatureHelp &કિંમત) {
  પ્રતિજ્ઞા(કિંમત.activeSignature >= 0 &&
         "Unexpected negative value for number of active signatures.");
  પ્રતિજ્ઞા(કિંમત.activeParameter >= 0 &&
         "Unexpected negative value for active parameter index");
  પાછા llvm::json::Object{
      {"activeSignature", કિંમત.activeSignature},
      {"activeParameter", કિંમત.activeParameter},
      {"signatures", llvm::json::Array(કિંમત.signatures)},
  };
}

//===----------------------------------------------------------------------===//
// DocumentLinkParams
//===----------------------------------------------------------------------===//

તાર્કિક llvm::lsp::fromJSON(અચલ llvm::json::કિંમત &કિંમત,
                         DocumentLinkParams &Result, llvm::json::Path Path) {
  llvm::json::ObjectMapper O(કિંમત, Path);
  પાછા O && O.map("textDocument", Result.textDocument);
}

//===----------------------------------------------------------------------===//
// DocumentLink
//===----------------------------------------------------------------------===//

llvm::json::કિંમત llvm::lsp::toJSON(અચલ DocumentLink &કિંમત) {
  પાછા llvm::json::Object{
      {"range", કિંમત.range},
      {"target", કિંમત.target},
  };
}

//===----------------------------------------------------------------------===//
// InlayHintsParams
//===----------------------------------------------------------------------===//

તાર્કિક llvm::lsp::fromJSON(અચલ llvm::json::કિંમત &કિંમત,
                         InlayHintsParams &Result, llvm::json::Path Path) {
  llvm::json::ObjectMapper O(કિંમત, Path);
  પાછા O && O.map("textDocument", Result.textDocument) &&
         O.map("range", Result.range);
}

//===----------------------------------------------------------------------===//
// InlayHint
//===----------------------------------------------------------------------===//

llvm::json::કિંમત llvm::lsp::toJSON(અચલ InlayHint &કિંમત) {
  પાછા llvm::json::Object{{"position", કિંમત.position},
                            {"kind", (પૂર્ણાંક)કિંમત.kind},
                            {"label", કિંમત.label},
                            {"paddingLeft", કિંમત.padding_ડાબું},
                            {"paddingRight", કિંમત.padding_જમણું}};
}
તાર્કિક llvm::lsp::operator==(અચલ InlayHint &Lhs, અચલ InlayHint &Rhs) {
  પાછા std::tie(Lhs.position, Lhs.kind, Lhs.label) ==
         std::tie(Rhs.position, Rhs.kind, Rhs.label);
}
તાર્કિક llvm::lsp::operator<(અચલ InlayHint &Lhs, અચલ InlayHint &Rhs) {
  પાછા std::tie(Lhs.position, Lhs.kind, Lhs.label) <
         std::tie(Rhs.position, Rhs.kind, Rhs.label);
}

llvm::raw_ostream &llvm::lsp::operator<<(llvm::raw_ostream &Os,
                                         InlayHintKind કિંમત) {
  પસંદગી (કિંમત) {
  સ્થિતિ InlayHintKind::Parameter:
    પાછા Os << "parameter";
  સ્થિતિ InlayHintKind::Type:
    પાછા Os << "type";
  }
  llvm_unreachable("Unknown InlayHintKind");
}

//===----------------------------------------------------------------------===//
// CodeActionContext
//===----------------------------------------------------------------------===//

તાર્કિક llvm::lsp::fromJSON(અચલ llvm::json::કિંમત &કિંમત,
                         CodeActionContext &Result, llvm::json::Path Path) {
  llvm::json::ObjectMapper O(કિંમત, Path);
  જો (!O || !O.map("diagnostics", Result.diagnostics))
    પાછા ખોટું;
  O.map("only", Result.only);
  પાછા સાચું;
}

//===----------------------------------------------------------------------===//
// CodeActionParams
//===----------------------------------------------------------------------===//

તાર્કિક llvm::lsp::fromJSON(અચલ llvm::json::કિંમત &કિંમત,
                         CodeActionParams &Result, llvm::json::Path Path) {
  llvm::json::ObjectMapper O(કિંમત, Path);
  પાછા O && O.map("textDocument", Result.textDocument) &&
         O.map("range", Result.range) && O.map("context", Result.context);
}

//===----------------------------------------------------------------------===//
// WorkspaceEdit
//===----------------------------------------------------------------------===//

તાર્કિક llvm::lsp::fromJSON(અચલ llvm::json::કિંમત &કિંમત, WorkspaceEdit &Result,
                         llvm::json::Path Path) {
  llvm::json::ObjectMapper O(કિંમત, Path);
  પાછા O && O.map("changes", Result.changes);
}

llvm::json::કિંમત llvm::lsp::toJSON(અચલ WorkspaceEdit &કિંમત) {
  llvm::json::Object FileChanges;
  માટે (auto &Change : કિંમત.changes)
    FileChanges[Change.first] = llvm::json::Array(Change.second);
  પાછા llvm::json::Object{{"changes", std::move(FileChanges)}};
}

//===----------------------------------------------------------------------===//
// CodeAction
//===----------------------------------------------------------------------===//

અચલ llvm::StringLiteral CodeAction::kQuickFix = "quickfix";
અચલ llvm::StringLiteral CodeAction::kRefactor = "refactor";
અચલ llvm::StringLiteral CodeAction::kInfo = "info";

llvm::json::કિંમત llvm::lsp::toJSON(અચલ CodeAction &કિંમત) {
  llvm::json::Object CodeAction{{"title", કિંમત.title}};
  જો (કિંમત.kind)
    CodeAction["kind"] = *કિંમત.kind;
  જો (કિંમત.diagnostics)
    CodeAction["diagnostics"] = llvm::json::Array(*કિંમત.diagnostics);
  જો (કિંમત.isPreferred)
    CodeAction["isPreferred"] = સાચું;
  જો (કિંમત.edit)
    CodeAction["edit"] = *કિંમત.edit;
  પાછા std::move(CodeAction);
}

//===----------------------------------------------------------------------===//
// ShowMessageParams
//===----------------------------------------------------------------------===//

llvm::json::કિંમત llvm::lsp::toJSON(અચલ ShowMessageParams &Params) {
  auto Out = llvm::json::Object{
      {"type", static_cast<પૂર્ણાંક>(Params.type)},
      {"message", Params.message},
  };
  જો (Params.actions)
    Out["actions"] = *Params.actions;
  પાછા Out;
}

llvm::json::કિંમત llvm::lsp::toJSON(અચલ MessageActionItem &Params) {
  પાછા llvm::json::Object{{"title", Params.title}};
}
