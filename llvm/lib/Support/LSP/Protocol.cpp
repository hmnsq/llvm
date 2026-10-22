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
template <typename T>
स्थिर तार्किक mapOptOrNull(ध्रुव llvm::json::मानम् &Params,
                         llvm::StringLiteral Prop, T &Out,
                         llvm::json::Path Path) {
  ध्रुव llvm::json::Object *O = Params.getAsObject();
  प्रतिज्ञा(O);

  // Field is missing or null.
  auto *V = O->get(Prop);
  जइ (!V || V->getAsNull())
    प्रत्यागम सच्च;
  प्रत्यागम fromJSON(*V, Out, Path.field(Prop));
}

//===----------------------------------------------------------------------===//
// LSPError
//===----------------------------------------------------------------------===//

वर्ण LSPError::ID;

//===----------------------------------------------------------------------===//
// URIForFile
//===----------------------------------------------------------------------===//

स्थिर तार्किक isWindowsPath(StringRef Path) {
  प्रत्यागम Path.size() > 1 && llvm::isAlpha(Path[0]) && Path[1] == ':';
}

स्थिर तार्किक isNetworkPath(StringRef Path) {
  प्रत्यागम Path.size() > 2 && Path[0] == Path[1] &&
         llvm::sys::path::is_separator(Path[0]);
}

स्थिर तार्किक shouldEscapeInURI(unsigned वर्ण C) {
  // Unreserved characters.
  जइ ((C >= 'a' && C <= 'z') || (C >= 'A' && C <= 'Z') ||
      (C >= '0' && C <= '9'))
    प्रत्यागम मिच्छ;

  विकल्प (C) {
  पक्ष '-':
  पक्ष '_':
  पक्ष '.':
  पक्ष '~':
  // '/' is only reserved when parsing.
  पक्ष '/':
  // ':' is only reserved for relative URI paths, which we doesn't produce.
  पक्ष ':':
    प्रत्यागम मिच्छ;
  }
  प्रत्यागम सच्च;
}

/// Encodes a string according to percent-encoding.
/// - Unreserved characters are not escaped.
/// - Reserved characters always escaped with exceptions like '/'.
/// - All other characters are escaped.
स्थिर शून्य percentEncode(StringRef Content, std::string &Out) {
  प्रति (unsigned वर्ण C : Content) {
    जइ (shouldEscapeInURI(C)) {
      Out.प्रक्षिप_back('%');
      Out.प्रक्षिप_back(llvm::hexdigit(C / 16));
      Out.प्रक्षिप_back(llvm::hexdigit(C % 16));
    } अण्णहा {
      Out.प्रक्षिप_back(C);
    }
  }
}

/// Decodes a string according to percent-encoding.
स्थिर std::string percentDecode(StringRef Content) {
  std::string Result;
  प्रति (auto I = Content.begin(), E = Content.end(); I != E; ++I) {
    जइ (*I == '%' && I + 2 < Content.end() && llvm::isHexDigit(*(I + 1)) &&
        llvm::isHexDigit(*(I + 2))) {
      Result.प्रक्षिप_back(llvm::hexFromNibbles(*(I + 1), *(I + 2)));
      I += 2;
    } अण्णहा {
      Result.प्रक्षिप_back(*I);
    }
  }
  प्रत्यागम Result;
}

/// Return the set containing the supported URI schemes.
स्थिर StringSet<> &getSupportedSchemes() {
  स्थिर StringSet<> Schemes({"file", "test"});
  प्रत्यागम Schemes;
}

/// Returns true if the given scheme is structurally valid, i.e. it does not
/// contain any invalid scheme characters. This does not check that the scheme
/// is actually supported.
स्थिर तार्किक isStructurallyValidScheme(StringRef Scheme) {
  जइ (Scheme.empty())
    प्रत्यागम मिच्छ;
  जइ (!llvm::isAlpha(Scheme[0]))
    प्रत्यागम मिच्छ;
  प्रत्यागम llvm::all_of(llvm::drop_begin(Scheme), [](वर्ण C) {
    प्रत्यागम llvm::isAlnum(C) || C == '+' || C == '.' || C == '-';
  });
}

स्थिर llvm::Expected<std::string> uriFromAbsolutePath(StringRef AbsolutePath,
                                                       StringRef Scheme) {
  std::string Body;
  StringRef Authority;
  StringRef मूलम्_2 = llvm::sys::path::मूलम्_name(AbsolutePath);
  जइ (isNetworkPath(मूलम्_2)) {
    // Windows UNC paths e.g. \\server\share => file://server/share
    Authority = मूलम्_2.drop_front(2);
    AbsolutePath.consume_front(मूलम्_2);
  } अण्णहा जइ (isWindowsPath(मूलम्_2)) {
    // Windows paths e.g. X:\path => file:///X:/path
    Body = "/";
  }
  Body += llvm::sys::path::convert_to_slash(AbsolutePath);

  std::string Uri = Scheme.str() + ":";
  जइ (Authority.empty() && Body.empty())
    प्रत्यागम Uri;

  // If authority if empty, we only print body if it starts with "/"; otherwise,
  // the URI is invalid.
  जइ (!Authority.empty() || StringRef(Body).starts_with("/")) {
    Uri.append("//");
    percentEncode(Authority, Uri);
  }
  percentEncode(Body, Uri);
  प्रत्यागम Uri;
}

स्थिर llvm::Expected<std::string> getAbsolutePath(StringRef Authority,
                                                   StringRef Body) {
  जइ (!Body.starts_with("/"))
    प्रत्यागम llvm::createStringError(
        llvm::inconvertibleErrorCode(),
        "File scheme: expect body to be an absolute path starting "
        "with '/': " +
            Body);
  SmallString<128> Path;
  जइ (!Authority.empty()) {
    // Windows UNC paths e.g. file://server/share => \\server\share
    ("//" + Authority).toVector(Path);
  } अण्णहा जइ (isWindowsPath(Body.substr(1))) {
    // Windows paths e.g. file:///X:/path => X:\path
    Body.consume_front("/");
  }
  Path.append(Body);
  llvm::sys::path::native(Path);
  प्रत्यागम std::string(Path);
}

स्थिर llvm::Expected<std::string> parseFilePathFromURI(StringRef OrigUri) {
  StringRef Uri = OrigUri;

  // Decode the scheme of the URI.
  size_t Pos = Uri.find(':');
  जइ (Pos == StringRef::npos)
    प्रत्यागम llvm::createStringError(llvm::inconvertibleErrorCode(),
                                   "Scheme must be provided in URI: " +
                                       OrigUri);
  StringRef SchemeStr = Uri.substr(0, Pos);
  std::string UriScheme = percentDecode(SchemeStr);
  जइ (!isStructurallyValidScheme(UriScheme))
    प्रत्यागम llvm::createStringError(llvm::inconvertibleErrorCode(),
                                   "Invalid scheme: " + SchemeStr +
                                       " (decoded: " + UriScheme + ")");
  Uri = Uri.substr(Pos + 1);

  // Decode the authority of the URI.
  std::string UriAuthority;
  जइ (Uri.consume_front("//")) {
    Pos = Uri.find('/');
    UriAuthority = percentDecode(Uri.substr(0, Pos));
    Uri = Uri.substr(Pos);
  }

  // Decode the body of the URI.
  std::string UriBody = percentDecode(Uri);

  // Compute the absolute path for this uri.
  जइ (!getSupportedSchemes().contains(UriScheme)) {
    प्रत्यागम llvm::createStringError(llvm::inconvertibleErrorCode(),
                                   "unsupported URI scheme `" + UriScheme +
                                       "' for workspace files");
  }
  प्रत्यागम getAbsolutePath(UriAuthority, UriBody);
}

llvm::Expected<URIForFile> URIForFile::fromURI(StringRef Uri) {
  llvm::Expected<std::string> FilePath = parseFilePathFromURI(Uri);
  जइ (!FilePath)
    प्रत्यागम FilePath.takeError();
  प्रत्यागम URIForFile(std::move(*FilePath), Uri.str());
}

llvm::Expected<URIForFile> URIForFile::fromFile(StringRef AbsoluteFilepath,
                                                StringRef Scheme) {
  llvm::Expected<std::string> Uri =
      uriFromAbsolutePath(AbsoluteFilepath, Scheme);
  जइ (!Uri)
    प्रत्यागम Uri.takeError();
  प्रत्यागम fromURI(*Uri);
}

StringRef URIForFile::scheme() ध्रुव { प्रत्यागम uri().split(':').first; }

शून्य URIForFile::registerSupportedScheme(StringRef Scheme) {
  getSupportedSchemes().निवेशय(Scheme);
}

तार्किक llvm::lsp::fromJSON(ध्रुव llvm::json::मानम् &मानम्, URIForFile &Result,
                         llvm::json::Path Path) {
  जइ (std::optional<StringRef> Str = मानम्.getAsString()) {
    llvm::Expected<URIForFile> ExpectedUri = URIForFile::fromURI(*Str);
    जइ (!ExpectedUri) {
      Path.report("unresolvable URI");
      consumeError(ExpectedUri.takeError());
      प्रत्यागम मिच्छ;
    }
    Result = std::move(*ExpectedUri);
    प्रत्यागम सच्च;
  }
  प्रत्यागम मिच्छ;
}

llvm::json::मानम् llvm::lsp::toJSON(ध्रुव URIForFile &मानम्) {
  प्रत्यागम मानम्.uri();
}

raw_ostream &llvm::lsp::operator<<(raw_ostream &Os, ध्रुव URIForFile &मानम्) {
  प्रत्यागम Os << मानम्.uri();
}

//===----------------------------------------------------------------------===//
// ClientCapabilities
//===----------------------------------------------------------------------===//

तार्किक llvm::lsp::fromJSON(ध्रुव llvm::json::मानम् &मानम्,
                         ClientCapabilities &Result, llvm::json::Path Path) {
  ध्रुव llvm::json::Object *O = मानम्.getAsObject();
  जइ (!O) {
    Path.report("expected object");
    प्रत्यागम मिच्छ;
  }
  जइ (ध्रुव llvm::json::Object *TextDocument = O->getObject("textDocument")) {
    जइ (ध्रुव llvm::json::Object *Document_चिह्नम् =
            TextDocument->getObject("documentSymbol")) {
      जइ (std::optional<तार्किक> HierarchicalSupport =
              Document_चिह्नम्->getBoolean("hierarchicalDocumentSymbolSupport"))
        Result.hierarchical_Document_चिह्नम् = *HierarchicalSupport;
    }
    जइ (auto *CodeAction = TextDocument->getObject("codeAction")) {
      जइ (CodeAction->getObject("codeActionLiteralSupport"))
        Result.codeActionStructure = सच्च;
    }
  }
  जइ (auto *Window = O->getObject("window")) {
    जइ (std::optional<तार्किक> WorkDoneProgressSupport =
            Window->getBoolean("workDoneProgress"))
      Result.workDoneProgress = *WorkDoneProgressSupport;
  }
  प्रत्यागम सच्च;
}

//===----------------------------------------------------------------------===//
// ClientInfo
//===----------------------------------------------------------------------===//

तार्किक llvm::lsp::fromJSON(ध्रुव llvm::json::मानम् &मानम्, ClientInfo &Result,
                         llvm::json::Path Path) {
  llvm::json::ObjectMapper O(मानम्, Path);
  जइ (!O || !O.map("name", Result.name))
    प्रत्यागम मिच्छ;

  // Don't fail if we can't parse version.
  O.map("version", Result.version);
  प्रत्यागम सच्च;
}

//===----------------------------------------------------------------------===//
// InitializeParams
//===----------------------------------------------------------------------===//

तार्किक llvm::lsp::fromJSON(ध्रुव llvm::json::मानम् &मानम्, TraceLevel &Result,
                         llvm::json::Path Path) {
  जइ (std::optional<StringRef> Str = मानम्.getAsString()) {
    जइ (*Str == "off") {
      Result = TraceLevel::Off;
      प्रत्यागम सच्च;
    }
    जइ (*Str == "messages") {
      Result = TraceLevel::Messages;
      प्रत्यागम सच्च;
    }
    जइ (*Str == "verbose") {
      Result = TraceLevel::Verbose;
      प्रत्यागम सच्च;
    }
  }
  प्रत्यागम मिच्छ;
}

तार्किक llvm::lsp::fromJSON(ध्रुव llvm::json::मानम् &मानम्,
                         InitializeParams &Result, llvm::json::Path Path) {
  llvm::json::ObjectMapper O(मानम्, Path);
  जइ (!O)
    प्रत्यागम मिच्छ;
  // We deliberately don't fail if we can't parse individual fields.
  O.map("capabilities", Result.capabilities);
  O.map("trace", Result.trace);
  O.map("rootUri", Result.मूलम्_Uri);
  O.map("rootPath", Result.मूलम्_Path);
  mapOptOrNull(मानम्, "clientInfo", Result.clientInfo, Path);

  प्रत्यागम सच्च;
}

//===----------------------------------------------------------------------===//
// TextDocumentItem
//===----------------------------------------------------------------------===//

तार्किक llvm::lsp::fromJSON(ध्रुव llvm::json::मानम् &मानम्,
                         TextDocumentItem &Result, llvm::json::Path Path) {
  llvm::json::ObjectMapper O(मानम्, Path);
  प्रत्यागम O && O.map("uri", Result.uri) &&
         O.map("languageId", Result.languageId) && O.map("text", Result.text) &&
         O.map("version", Result.version);
}

//===----------------------------------------------------------------------===//
// TextDocumentIdentifier
//===----------------------------------------------------------------------===//

llvm::json::मानम् llvm::lsp::toJSON(ध्रुव TextDocumentIdentifier &मानम्) {
  प्रत्यागम llvm::json::Object{{"uri", मानम्.uri}};
}

तार्किक llvm::lsp::fromJSON(ध्रुव llvm::json::मानम् &मानम्,
                         TextDocumentIdentifier &Result,
                         llvm::json::Path Path) {
  llvm::json::ObjectMapper O(मानम्, Path);
  प्रत्यागम O && O.map("uri", Result.uri);
}

//===----------------------------------------------------------------------===//
// VersionedTextDocumentIdentifier
//===----------------------------------------------------------------------===//

llvm::json::मानम्
llvm::lsp::toJSON(ध्रुव VersionedTextDocumentIdentifier &मानम्) {
  प्रत्यागम llvm::json::Object{
      {"uri", मानम्.uri},
      {"version", मानम्.version},
  };
}

तार्किक llvm::lsp::fromJSON(ध्रुव llvm::json::मानम् &मानम्,
                         VersionedTextDocumentIdentifier &Result,
                         llvm::json::Path Path) {
  llvm::json::ObjectMapper O(मानम्, Path);
  प्रत्यागम O && O.map("uri", Result.uri) && O.map("version", Result.version);
}

//===----------------------------------------------------------------------===//
// Position
//===----------------------------------------------------------------------===//

तार्किक llvm::lsp::fromJSON(ध्रुव llvm::json::मानम् &मानम्, Position &Result,
                         llvm::json::Path Path) {
  llvm::json::ObjectMapper O(मानम्, Path);
  प्रत्यागम O && O.map("line", Result.पङ्क्तिः_2) &&
         O.map("character", Result.character);
}

llvm::json::मानम् llvm::lsp::toJSON(ध्रुव Position &मानम्) {
  प्रत्यागम llvm::json::Object{
      {"line", मानम्.पङ्क्तिः_2},
      {"character", मानम्.character},
  };
}

raw_ostream &llvm::lsp::operator<<(raw_ostream &Os, ध्रुव Position &मानम्) {
  प्रत्यागम Os << मानम्.पङ्क्तिः_2 << ':' << मानम्.character;
}

//===----------------------------------------------------------------------===//
// Range
//===----------------------------------------------------------------------===//

तार्किक llvm::lsp::fromJSON(ध्रुव llvm::json::मानम् &मानम्, Range &Result,
                         llvm::json::Path Path) {
  llvm::json::ObjectMapper O(मानम्, Path);
  प्रत्यागम O && O.map("start", Result.start) && O.map("end", Result.end);
}

llvm::json::मानम् llvm::lsp::toJSON(ध्रुव Range &मानम्) {
  प्रत्यागम llvm::json::Object{
      {"start", मानम्.start},
      {"end", मानम्.end},
  };
}

raw_ostream &llvm::lsp::operator<<(raw_ostream &Os, ध्रुव Range &मानम्) {
  प्रत्यागम Os << मानम्.start << '-' << मानम्.end;
}

//===----------------------------------------------------------------------===//
// Location
//===----------------------------------------------------------------------===//

तार्किक llvm::lsp::fromJSON(ध्रुव llvm::json::मानम् &मानम्, Location &Result,
                         llvm::json::Path Path) {
  llvm::json::ObjectMapper O(मानम्, Path);
  प्रत्यागम O && O.map("uri", Result.uri) && O.map("range", Result.range);
}

llvm::json::मानम् llvm::lsp::toJSON(ध्रुव Location &मानम्) {
  प्रत्यागम llvm::json::Object{
      {"uri", मानम्.uri},
      {"range", मानम्.range},
  };
}

raw_ostream &llvm::lsp::operator<<(raw_ostream &Os, ध्रुव Location &मानम्) {
  प्रत्यागम Os << मानम्.range << '@' << मानम्.uri;
}

//===----------------------------------------------------------------------===//
// TextDocumentPositionParams
//===----------------------------------------------------------------------===//

तार्किक llvm::lsp::fromJSON(ध्रुव llvm::json::मानम् &मानम्,
                         TextDocumentPositionParams &Result,
                         llvm::json::Path Path) {
  llvm::json::ObjectMapper O(मानम्, Path);
  प्रत्यागम O && O.map("textDocument", Result.textDocument) &&
         O.map("position", Result.position);
}

//===----------------------------------------------------------------------===//
// ReferenceParams
//===----------------------------------------------------------------------===//

तार्किक llvm::lsp::fromJSON(ध्रुव llvm::json::मानम् &मानम्,
                         ReferenceContext &Result, llvm::json::Path Path) {
  llvm::json::ObjectMapper O(मानम्, Path);
  प्रत्यागम O && O.mapOptional("includeDeclaration", Result.includeDeclaration);
}

तार्किक llvm::lsp::fromJSON(ध्रुव llvm::json::मानम् &मानम्,
                         ReferenceParams &Result, llvm::json::Path Path) {
  TextDocumentPositionParams &Base = Result;
  llvm::json::ObjectMapper O(मानम्, Path);
  प्रत्यागम fromJSON(मानम्, Base, Path) && O &&
         O.mapOptional("context", Result.context);
}

//===----------------------------------------------------------------------===//
// DidOpenTextDocumentParams
//===----------------------------------------------------------------------===//

तार्किक llvm::lsp::fromJSON(ध्रुव llvm::json::मानम् &मानम्,
                         DidOpenTextDocumentParams &Result,
                         llvm::json::Path Path) {
  llvm::json::ObjectMapper O(मानम्, Path);
  प्रत्यागम O && O.map("textDocument", Result.textDocument);
}

//===----------------------------------------------------------------------===//
// DidCloseTextDocumentParams
//===----------------------------------------------------------------------===//

तार्किक llvm::lsp::fromJSON(ध्रुव llvm::json::मानम् &मानम्,
                         DidCloseTextDocumentParams &Result,
                         llvm::json::Path Path) {
  llvm::json::ObjectMapper O(मानम्, Path);
  प्रत्यागम O && O.map("textDocument", Result.textDocument);
}

//===----------------------------------------------------------------------===//
// DidSaveTextDocumentParams
//===----------------------------------------------------------------------===//

तार्किक llvm::lsp::fromJSON(ध्रुव llvm::json::मानम् &Params,
                         DidSaveTextDocumentParams &R, llvm::json::Path P) {
  llvm::json::ObjectMapper O(Params, P);
  प्रत्यागम O && O.map("textDocument", R.textDocument);
}

//===----------------------------------------------------------------------===//
// DidChangeTextDocumentParams
//===----------------------------------------------------------------------===//

LogicalResult
TextDocumentContentChangeEvent::applyTo(std::string &Contents) ध्रुव {
  // If there is no range, the full document changed.
  जइ (!range) {
    Contents = text;
    प्रत्यागम success();
  }

  // Try to map the replacement range to the content.
  llvm::SourceMgr TmpScrMgr;
  TmpScrMgr.AddNewSourceBuffer(llvm::MemoryBuffer::getMemBuffer(Contents),
                               SMLoc());
  SMRange RangeLoc = range->getAsSMRange(TmpScrMgr);
  जइ (!RangeLoc.isValid())
    प्रत्यागम failure();

  Contents.replace(RangeLoc.Start.getPointer() - Contents.data(),
                   RangeLoc.End.getPointer() - RangeLoc.Start.getPointer(),
                   text);
  प्रत्यागम success();
}

LogicalResult TextDocumentContentChangeEvent::applyTo(
    ArrayRef<TextDocumentContentChangeEvent> Changes, std::string &Contents) {
  प्रति (ध्रुव auto &Change : Changes)
    जइ (failed(Change.applyTo(Contents)))
      प्रत्यागम failure();
  प्रत्यागम success();
}

तार्किक llvm::lsp::fromJSON(ध्रुव llvm::json::मानम् &मानम्,
                         TextDocumentContentChangeEvent &Result,
                         llvm::json::Path Path) {
  llvm::json::ObjectMapper O(मानम्, Path);
  प्रत्यागम O && O.map("range", Result.range) &&
         O.map("rangeLength", Result.rangeLength) && O.map("text", Result.text);
}

तार्किक llvm::lsp::fromJSON(ध्रुव llvm::json::मानम् &मानम्,
                         DidChangeTextDocumentParams &Result,
                         llvm::json::Path Path) {
  llvm::json::ObjectMapper O(मानम्, Path);
  प्रत्यागम O && O.map("textDocument", Result.textDocument) &&
         O.map("contentChanges", Result.contentChanges);
}

//===----------------------------------------------------------------------===//
// MarkupContent
//===----------------------------------------------------------------------===//

स्थिर llvm::StringRef toTextKind(MarkupKind Kind) {
  विकल्प (Kind) {
  पक्ष MarkupKind::PlainText:
    प्रत्यागम "plaintext";
  पक्ष MarkupKind::Markdown:
    प्रत्यागम "markdown";
  }
  llvm_unreachable("Invalid MarkupKind");
}

raw_ostream &llvm::lsp::operator<<(raw_ostream &Os, MarkupKind Kind) {
  प्रत्यागम Os << toTextKind(Kind);
}

llvm::json::मानम् llvm::lsp::toJSON(ध्रुव MarkupContent &Mc) {
  जइ (Mc.मानम्_2.empty())
    प्रत्यागम nullptr;

  प्रत्यागम llvm::json::Object{
      {"kind", toTextKind(Mc.kind)},
      {"value", Mc.मानम्_2},
  };
}

//===----------------------------------------------------------------------===//
// Hover
//===----------------------------------------------------------------------===//

llvm::json::मानम् llvm::lsp::toJSON(ध्रुव Hover &Hover) {
  llvm::json::Object Result{{"contents", toJSON(Hover.contents)}};
  जइ (Hover.range)
    Result["range"] = toJSON(*Hover.range);
  प्रत्यागम std::move(Result);
}

//===----------------------------------------------------------------------===//
// DocumentSymbol
//===----------------------------------------------------------------------===//

llvm::json::मानम् llvm::lsp::toJSON(ध्रुव Document_चिह्नम् &चिह्नम्) {
  llvm::json::Object Result{{"name", चिह्नम्.name},
                            {"kind", static_cast<पूर्णाङ्क>(चिह्नम्.kind)},
                            {"range", चिह्नम्.range},
                            {"selectionRange", चिह्नम्.selectionRange}};

  जइ (!चिह्नम्.detail.empty())
    Result["detail"] = चिह्नम्.detail;
  जइ (!चिह्नम्.children.empty())
    Result["children"] = चिह्नम्.children;
  प्रत्यागम std::move(Result);
}

//===----------------------------------------------------------------------===//
// DocumentSymbolParams
//===----------------------------------------------------------------------===//

तार्किक llvm::lsp::fromJSON(ध्रुव llvm::json::मानम् &मानम्,
                         Document_चिह्नम्_Params &Result, llvm::json::Path Path) {
  llvm::json::ObjectMapper O(मानम्, Path);
  प्रत्यागम O && O.map("textDocument", Result.textDocument);
}

//===----------------------------------------------------------------------===//
// DiagnosticRelatedInformation
//===----------------------------------------------------------------------===//

तार्किक llvm::lsp::fromJSON(ध्रुव llvm::json::मानम् &मानम्,
                         DiagnosticRelatedInformation &Result,
                         llvm::json::Path Path) {
  llvm::json::ObjectMapper O(मानम्, Path);
  प्रत्यागम O && O.map("location", Result.location) &&
         O.map("message", Result.message);
}

llvm::json::मानम् llvm::lsp::toJSON(ध्रुव DiagnosticRelatedInformation &Info) {
  प्रत्यागम llvm::json::Object{
      {"location", Info.location},
      {"message", Info.message},
  };
}

//===----------------------------------------------------------------------===//
// Diagnostic
//===----------------------------------------------------------------------===//

llvm::json::मानम् llvm::lsp::toJSON(DiagnosticTag Tag) {
  प्रत्यागम static_cast<पूर्णाङ्क>(Tag);
}

तार्किक llvm::lsp::fromJSON(ध्रुव llvm::json::मानम् &मानम्, DiagnosticTag &Result,
                         llvm::json::Path Path) {
  जइ (std::optional<int64_t> I = मानम्.getAsInteger()) {
    Result = (DiagnosticTag)*I;
    प्रत्यागम सच्च;
  }

  प्रत्यागम मिच्छ;
}

llvm::json::मानम् llvm::lsp::toJSON(ध्रुव Diagnostic &Diag) {
  llvm::json::Object Result{
      {"range", Diag.range},
      {"severity", (पूर्णाङ्क)Diag.severity},
      {"message", Diag.message},
  };
  जइ (Diag.category)
    Result["category"] = *Diag.category;
  जइ (!Diag.source.empty())
    Result["source"] = Diag.source;
  जइ (Diag.relatedInformation)
    Result["relatedInformation"] = *Diag.relatedInformation;
  जइ (!Diag.tags.empty())
    Result["tags"] = Diag.tags;
  प्रत्यागम std::move(Result);
}

तार्किक llvm::lsp::fromJSON(ध्रुव llvm::json::मानम् &मानम्, Diagnostic &Result,
                         llvm::json::Path Path) {
  llvm::json::ObjectMapper O(मानम्, Path);
  जइ (!O)
    प्रत्यागम मिच्छ;
  पूर्णाङ्क Severity = 0;
  जइ (!mapOptOrNull(मानम्, "severity", Severity, Path))
    प्रत्यागम मिच्छ;
  Result.severity = (DiagnosticSeverity)Severity;

  प्रत्यागम O.map("range", Result.range) && O.map("message", Result.message) &&
         mapOptOrNull(मानम्, "category", Result.category, Path) &&
         mapOptOrNull(मानम्, "source", Result.source, Path) &&
         mapOptOrNull(मानम्, "relatedInformation", Result.relatedInformation,
                      Path) &&
         mapOptOrNull(मानम्, "tags", Result.tags, Path);
}

//===----------------------------------------------------------------------===//
// PublishDiagnosticsParams
//===----------------------------------------------------------------------===//

llvm::json::मानम् llvm::lsp::toJSON(ध्रुव PublishDiagnosticsParams &Params) {
  प्रत्यागम llvm::json::Object{
      {"uri", Params.uri},
      {"diagnostics", Params.diagnostics},
      {"version", Params.version},
  };
}

//===----------------------------------------------------------------------===//
// TextEdit
//===----------------------------------------------------------------------===//

तार्किक llvm::lsp::fromJSON(ध्रुव llvm::json::मानम् &मानम्, TextEdit &Result,
                         llvm::json::Path Path) {
  llvm::json::ObjectMapper O(मानम्, Path);
  प्रत्यागम O && O.map("range", Result.range) && O.map("newText", Result.newText);
}

llvm::json::मानम् llvm::lsp::toJSON(ध्रुव TextEdit &मानम्) {
  प्रत्यागम llvm::json::Object{
      {"range", मानम्.range},
      {"newText", मानम्.newText},
  };
}

raw_ostream &llvm::lsp::operator<<(raw_ostream &Os, ध्रुव TextEdit &मानम्) {
  Os << मानम्.range << " => \"";
  llvm::printEscapedString(मानम्.newText, Os);
  प्रत्यागम Os << '"';
}

//===----------------------------------------------------------------------===//
// CompletionItemKind
//===----------------------------------------------------------------------===//

तार्किक llvm::lsp::fromJSON(ध्रुव llvm::json::मानम् &मानम्,
                         CompletionItemKind &Result, llvm::json::Path Path) {
  जइ (std::optional<int64_t> Int_मानम् = मानम्.getAsInteger()) {
    जइ (*Int_मानम् < static_cast<पूर्णाङ्क>(CompletionItemKind::Text) ||
        *Int_मानम् > static_cast<पूर्णाङ्क>(CompletionItemKind::TypeParameter))
      प्रत्यागम मिच्छ;
    Result = static_cast<CompletionItemKind>(*Int_मानम्);
    प्रत्यागम सच्च;
  }
  प्रत्यागम मिच्छ;
}

CompletionItemKind llvm::lsp::adjustKindToCapability(
    CompletionItemKind Kind,
    CompletionItemKindBitset &SupportedCompletionItemKinds) {
  size_t KindVal = static_cast<size_t>(Kind);
  जइ (KindVal >= kCompletionItemKindMin &&
      KindVal <= SupportedCompletionItemKinds.size() &&
      SupportedCompletionItemKinds[KindVal])
    प्रत्यागम Kind;

  // Provide some fall backs for common kinds that are close enough.
  विकल्प (Kind) {
  पक्ष CompletionItemKind::Folder:
    प्रत्यागम CompletionItemKind::File;
  पक्ष CompletionItemKind::EnumMember:
    प्रत्यागम CompletionItemKind::Enum;
  पक्ष CompletionItemKind::Struct:
    प्रत्यागम CompletionItemKind::Class;
  स्वभाव:
    प्रत्यागम CompletionItemKind::Text;
  }
}

तार्किक llvm::lsp::fromJSON(ध्रुव llvm::json::मानम् &मानम्,
                         CompletionItemKindBitset &Result,
                         llvm::json::Path Path) {
  जइ (ध्रुव llvm::json::Array *Array_मानम् = मानम्.getAsArray()) {
    प्रति (size_t I = 0, E = Array_मानम्->size(); I < E; ++I) {
      CompletionItemKind KindOut;
      जइ (fromJSON((*Array_मानम्)[I], KindOut, Path.index(I)))
        Result.set(size_t(KindOut));
    }
    प्रत्यागम सच्च;
  }
  प्रत्यागम मिच्छ;
}

//===----------------------------------------------------------------------===//
// CompletionItem
//===----------------------------------------------------------------------===//

llvm::json::मानम् llvm::lsp::toJSON(ध्रुव CompletionItem &मानम्) {
  प्रतिज्ञा(!मानम्.label.empty() && "completion item label is required");
  llvm::json::Object Result{{"label", मानम्.label}};
  जइ (मानम्.kind != CompletionItemKind::Missing)
    Result["kind"] = static_cast<पूर्णाङ्क>(मानम्.kind);
  जइ (!मानम्.detail.empty())
    Result["detail"] = मानम्.detail;
  जइ (मानम्.documentation)
    Result["documentation"] = मानम्.documentation;
  जइ (!मानम्.sortText.empty())
    Result["sortText"] = मानम्.sortText;
  जइ (!मानम्.filterText.empty())
    Result["filterText"] = मानम्.filterText;
  जइ (!मानम्.निवेशय_Text.empty())
    Result["insertText"] = मानम्.निवेशय_Text;
  जइ (मानम्.निवेशय_Text_Format != निवेशय_Text_Format_2::Missing)
    Result["insertTextFormat"] = static_cast<पूर्णाङ्क>(मानम्.निवेशय_Text_Format);
  जइ (मानम्.textEdit)
    Result["textEdit"] = *मानम्.textEdit;
  जइ (!मानम्.additionalTextEdits.empty()) {
    Result["additionalTextEdits"] =
        llvm::json::Array(मानम्.additionalTextEdits);
  }
  जइ (मानम्.deprecated)
    Result["deprecated"] = मानम्.deprecated;
  प्रत्यागम std::move(Result);
}

raw_ostream &llvm::lsp::operator<<(raw_ostream &Os,
                                   ध्रुव CompletionItem &मानम्) {
  प्रत्यागम Os << मानम्.label << " - " << toJSON(मानम्);
}

तार्किक llvm::lsp::operator<(ध्रुव CompletionItem &Lhs,
                          ध्रुव CompletionItem &Rhs) {
  प्रत्यागम (Lhs.sortText.empty() ? Lhs.label : Lhs.sortText) <
         (Rhs.sortText.empty() ? Rhs.label : Rhs.sortText);
}

//===----------------------------------------------------------------------===//
// CompletionList
//===----------------------------------------------------------------------===//

llvm::json::मानम् llvm::lsp::toJSON(ध्रुव CompletionList &मानम्) {
  प्रत्यागम llvm::json::Object{
      {"isIncomplete", मानम्.isIncomplete},
      {"items", llvm::json::Array(मानम्.वस्तूनि)},
  };
}

//===----------------------------------------------------------------------===//
// CompletionContext
//===----------------------------------------------------------------------===//

तार्किक llvm::lsp::fromJSON(ध्रुव llvm::json::मानम् &मानम्,
                         CompletionContext &Result, llvm::json::Path Path) {
  llvm::json::ObjectMapper O(मानम्, Path);
  पूर्णाङ्क TriggerKind;
  जइ (!O || !O.map("triggerKind", TriggerKind) ||
      !mapOptOrNull(मानम्, "triggerCharacter", Result.triggerCharacter, Path))
    प्रत्यागम मिच्छ;
  Result.triggerKind = static_cast<CompletionTriggerKind>(TriggerKind);
  प्रत्यागम सच्च;
}

//===----------------------------------------------------------------------===//
// CompletionParams
//===----------------------------------------------------------------------===//

तार्किक llvm::lsp::fromJSON(ध्रुव llvm::json::मानम् &मानम्,
                         CompletionParams &Result, llvm::json::Path Path) {
  जइ (!fromJSON(मानम्, static_cast<TextDocumentPositionParams &>(Result), Path))
    प्रत्यागम मिच्छ;
  जइ (ध्रुव llvm::json::मानम् *Context = मानम्.getAsObject()->get("context"))
    प्रत्यागम fromJSON(*Context, Result.context, Path.field("context"));
  प्रत्यागम सच्च;
}

//===----------------------------------------------------------------------===//
// ParameterInformation
//===----------------------------------------------------------------------===//

llvm::json::मानम् llvm::lsp::toJSON(ध्रुव ParameterInformation &मानम्) {
  प्रतिज्ञा((मानम्.labelOffsets || !मानम्.labelString.empty()) &&
         "parameter information label is required");
  llvm::json::Object Result;
  जइ (मानम्.labelOffsets)
    Result["label"] = llvm::json::Array(
        {मानम्.labelOffsets->first, मानम्.labelOffsets->second});
  अण्णहा
    Result["label"] = मानम्.labelString;
  जइ (!मानम्.documentation.empty())
    Result["documentation"] = मानम्.documentation;
  प्रत्यागम std::move(Result);
}

//===----------------------------------------------------------------------===//
// SignatureInformation
//===----------------------------------------------------------------------===//

llvm::json::मानम् llvm::lsp::toJSON(ध्रुव SignatureInformation &मानम्) {
  प्रतिज्ञा(!मानम्.label.empty() && "signature information label is required");
  llvm::json::Object Result{
      {"label", मानम्.label},
      {"parameters", llvm::json::Array(मानम्.parameters)},
  };
  जइ (!मानम्.documentation.empty())
    Result["documentation"] = मानम्.documentation;
  प्रत्यागम std::move(Result);
}

raw_ostream &llvm::lsp::operator<<(raw_ostream &Os,
                                   ध्रुव SignatureInformation &मानम्) {
  प्रत्यागम Os << मानम्.label << " - " << toJSON(मानम्);
}

//===----------------------------------------------------------------------===//
// SignatureHelp
//===----------------------------------------------------------------------===//

llvm::json::मानम् llvm::lsp::toJSON(ध्रुव SignatureHelp &मानम्) {
  प्रतिज्ञा(मानम्.activeSignature >= 0 &&
         "Unexpected negative value for number of active signatures.");
  प्रतिज्ञा(मानम्.activeParameter >= 0 &&
         "Unexpected negative value for active parameter index");
  प्रत्यागम llvm::json::Object{
      {"activeSignature", मानम्.activeSignature},
      {"activeParameter", मानम्.activeParameter},
      {"signatures", llvm::json::Array(मानम्.signatures)},
  };
}

//===----------------------------------------------------------------------===//
// DocumentLinkParams
//===----------------------------------------------------------------------===//

तार्किक llvm::lsp::fromJSON(ध्रुव llvm::json::मानम् &मानम्,
                         DocumentLinkParams &Result, llvm::json::Path Path) {
  llvm::json::ObjectMapper O(मानम्, Path);
  प्रत्यागम O && O.map("textDocument", Result.textDocument);
}

//===----------------------------------------------------------------------===//
// DocumentLink
//===----------------------------------------------------------------------===//

llvm::json::मानम् llvm::lsp::toJSON(ध्रुव DocumentLink &मानम्) {
  प्रत्यागम llvm::json::Object{
      {"range", मानम्.range},
      {"target", मानम्.target},
  };
}

//===----------------------------------------------------------------------===//
// InlayHintsParams
//===----------------------------------------------------------------------===//

तार्किक llvm::lsp::fromJSON(ध्रुव llvm::json::मानम् &मानम्,
                         InlayHintsParams &Result, llvm::json::Path Path) {
  llvm::json::ObjectMapper O(मानम्, Path);
  प्रत्यागम O && O.map("textDocument", Result.textDocument) &&
         O.map("range", Result.range);
}

//===----------------------------------------------------------------------===//
// InlayHint
//===----------------------------------------------------------------------===//

llvm::json::मानम् llvm::lsp::toJSON(ध्रुव InlayHint &मानम्) {
  प्रत्यागम llvm::json::Object{{"position", मानम्.position},
                            {"kind", (पूर्णाङ्क)मानम्.kind},
                            {"label", मानम्.label},
                            {"paddingLeft", मानम्.padding_वामः},
                            {"paddingRight", मानम्.padding_दक्षिणः}};
}
तार्किक llvm::lsp::operator==(ध्रुव InlayHint &Lhs, ध्रुव InlayHint &Rhs) {
  प्रत्यागम std::tie(Lhs.position, Lhs.kind, Lhs.label) ==
         std::tie(Rhs.position, Rhs.kind, Rhs.label);
}
तार्किक llvm::lsp::operator<(ध्रुव InlayHint &Lhs, ध्रुव InlayHint &Rhs) {
  प्रत्यागम std::tie(Lhs.position, Lhs.kind, Lhs.label) <
         std::tie(Rhs.position, Rhs.kind, Rhs.label);
}

llvm::raw_ostream &llvm::lsp::operator<<(llvm::raw_ostream &Os,
                                         InlayHintKind मानम्) {
  विकल्प (मानम्) {
  पक्ष InlayHintKind::Parameter:
    प्रत्यागम Os << "parameter";
  पक्ष InlayHintKind::Type:
    प्रत्यागम Os << "type";
  }
  llvm_unreachable("Unknown InlayHintKind");
}

//===----------------------------------------------------------------------===//
// CodeActionContext
//===----------------------------------------------------------------------===//

तार्किक llvm::lsp::fromJSON(ध्रुव llvm::json::मानम् &मानम्,
                         CodeActionContext &Result, llvm::json::Path Path) {
  llvm::json::ObjectMapper O(मानम्, Path);
  जइ (!O || !O.map("diagnostics", Result.diagnostics))
    प्रत्यागम मिच्छ;
  O.map("only", Result.only);
  प्रत्यागम सच्च;
}

//===----------------------------------------------------------------------===//
// CodeActionParams
//===----------------------------------------------------------------------===//

तार्किक llvm::lsp::fromJSON(ध्रुव llvm::json::मानम् &मानम्,
                         CodeActionParams &Result, llvm::json::Path Path) {
  llvm::json::ObjectMapper O(मानम्, Path);
  प्रत्यागम O && O.map("textDocument", Result.textDocument) &&
         O.map("range", Result.range) && O.map("context", Result.context);
}

//===----------------------------------------------------------------------===//
// WorkspaceEdit
//===----------------------------------------------------------------------===//

तार्किक llvm::lsp::fromJSON(ध्रुव llvm::json::मानम् &मानम्, WorkspaceEdit &Result,
                         llvm::json::Path Path) {
  llvm::json::ObjectMapper O(मानम्, Path);
  प्रत्यागम O && O.map("changes", Result.changes);
}

llvm::json::मानम् llvm::lsp::toJSON(ध्रुव WorkspaceEdit &मानम्) {
  llvm::json::Object FileChanges;
  प्रति (auto &Change : मानम्.changes)
    FileChanges[Change.first] = llvm::json::Array(Change.second);
  प्रत्यागम llvm::json::Object{{"changes", std::move(FileChanges)}};
}

//===----------------------------------------------------------------------===//
// CodeAction
//===----------------------------------------------------------------------===//

ध्रुव llvm::StringLiteral CodeAction::kQuickFix = "quickfix";
ध्रुव llvm::StringLiteral CodeAction::kRefactor = "refactor";
ध्रुव llvm::StringLiteral CodeAction::kInfo = "info";

llvm::json::मानम् llvm::lsp::toJSON(ध्रुव CodeAction &मानम्) {
  llvm::json::Object CodeAction{{"title", मानम्.title}};
  जइ (मानम्.kind)
    CodeAction["kind"] = *मानम्.kind;
  जइ (मानम्.diagnostics)
    CodeAction["diagnostics"] = llvm::json::Array(*मानम्.diagnostics);
  जइ (मानम्.isPreferred)
    CodeAction["isPreferred"] = सच्च;
  जइ (मानम्.edit)
    CodeAction["edit"] = *मानम्.edit;
  प्रत्यागम std::move(CodeAction);
}

//===----------------------------------------------------------------------===//
// ShowMessageParams
//===----------------------------------------------------------------------===//

llvm::json::मानम् llvm::lsp::toJSON(ध्रुव ShowMessageParams &Params) {
  auto Out = llvm::json::Object{
      {"type", static_cast<पूर्णाङ्क>(Params.type)},
      {"message", Params.message},
  };
  जइ (Params.actions)
    Out["actions"] = *Params.actions;
  प्रत्यागम Out;
}

llvm::json::मानम् llvm::lsp::toJSON(ध्रुव MessageActionItem &Params) {
  प्रत्यागम llvm::json::Object{{"title", Params.title}};
}
