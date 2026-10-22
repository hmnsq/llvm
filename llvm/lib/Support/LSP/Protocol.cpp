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
स्थिर तार्किक mapOptOrNull(ध्रुव llvm::json::मूल्य &Params,
                         llvm::StringLiteral Prop, T &Out,
                         llvm::json::Path Path) {
  ध्रुव llvm::json::Object *O = Params.getAsObject();
  प्रतिज्ञा(O);

  // Field is missing or null.
  स्वतः *V = O->get(Prop);
  यदि (!V || V->getAsNull())
    प्रत्यागम सत्य;
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

स्थिर तार्किक shouldEscapeInURI(अचिन्हित वर्ण C) {
  // Unreserved characters.
  यदि ((C >= 'a' && C <= 'z') || (C >= 'A' && C <= 'Z') ||
      (C >= '0' && C <= '9'))
    प्रत्यागम असत्य;

  विकल्प (C) {
  पक्ष '-':
  पक्ष '_':
  पक्ष '.':
  पक्ष '~':
  // '/' is only reserved when parsing.
  पक्ष '/':
  // ':' is only reserved for relative URI paths, which we doesn't produce.
  पक्ष ':':
    प्रत्यागम असत्य;
  }
  प्रत्यागम सत्य;
}

/// Encodes a string according to percent-encoding.
/// - Unreserved characters are not escaped.
/// - Reserved characters always escaped with exceptions like '/'.
/// - All other characters are escaped.
स्थिर शून्य percentEncode(StringRef Content, std::string &Out) {
  प्रति (अचिन्हित वर्ण C : Content) {
    यदि (shouldEscapeInURI(C)) {
      Out.धकेलो_back('%');
      Out.धकेलो_back(llvm::hexdigit(C / 16));
      Out.धकेलो_back(llvm::hexdigit(C % 16));
    } अन्यथा {
      Out.धकेलो_back(C);
    }
  }
}

/// Decodes a string according to percent-encoding.
स्थिर std::string percentDecode(StringRef Content) {
  std::string Result;
  प्रति (स्वतः I = Content.begin(), E = Content.end(); I != E; ++I) {
    यदि (*I == '%' && I + 2 < Content.end() && llvm::isHexDigit(*(I + 1)) &&
        llvm::isHexDigit(*(I + 2))) {
      Result.धकेलो_back(llvm::hexFromNibbles(*(I + 1), *(I + 2)));
      I += 2;
    } अन्यथा {
      Result.धकेलो_back(*I);
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
  यदि (Scheme.empty())
    प्रत्यागम असत्य;
  यदि (!llvm::isAlpha(Scheme[0]))
    प्रत्यागम असत्य;
  प्रत्यागम llvm::all_of(llvm::drop_begin(Scheme), [](वर्ण C) {
    प्रत्यागम llvm::isAlnum(C) || C == '+' || C == '.' || C == '-';
  });
}

स्थिर llvm::Expected<std::string> uriFromAbsolutePath(StringRef AbsolutePath,
                                                       StringRef Scheme) {
  std::string Body;
  StringRef Authority;
  StringRef मूल_2 = llvm::sys::path::मूल_name(AbsolutePath);
  यदि (isNetworkPath(मूल_2)) {
    // Windows UNC paths e.g. \\server\share => file://server/share
    Authority = मूल_2.drop_front(2);
    AbsolutePath.consume_front(मूल_2);
  } अन्यथा यदि (isWindowsPath(मूल_2)) {
    // Windows paths e.g. X:\path => file:///X:/path
    Body = "/";
  }
  Body += llvm::sys::path::convert_to_slash(AbsolutePath);

  std::string Uri = Scheme.str() + ":";
  यदि (Authority.empty() && Body.empty())
    प्रत्यागम Uri;

  // If authority if empty, we only print body if it starts with "/"; otherwise,
  // the URI is invalid.
  यदि (!Authority.empty() || StringRef(Body).starts_with("/")) {
    Uri.append("//");
    percentEncode(Authority, Uri);
  }
  percentEncode(Body, Uri);
  प्रत्यागम Uri;
}

स्थिर llvm::Expected<std::string> getAbsolutePath(StringRef Authority,
                                                   StringRef Body) {
  यदि (!Body.starts_with("/"))
    प्रत्यागम llvm::createStringError(
        llvm::inconvertibleErrorCode(),
        "File scheme: expect body to be an absolute path starting "
        "with '/': " +
            Body);
  SmallString<128> Path;
  यदि (!Authority.empty()) {
    // Windows UNC paths e.g. file://server/share => \\server\share
    ("//" + Authority).toVector(Path);
  } अन्यथा यदि (isWindowsPath(Body.substr(1))) {
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
  माप_प्रकार Pos = Uri.find(':');
  यदि (Pos == StringRef::npos)
    प्रत्यागम llvm::createStringError(llvm::inconvertibleErrorCode(),
                                   "Scheme must be provided in URI: " +
                                       OrigUri);
  StringRef SchemeStr = Uri.substr(0, Pos);
  std::string UriScheme = percentDecode(SchemeStr);
  यदि (!isStructurallyValidScheme(UriScheme))
    प्रत्यागम llvm::createStringError(llvm::inconvertibleErrorCode(),
                                   "Invalid scheme: " + SchemeStr +
                                       " (decoded: " + UriScheme + ")");
  Uri = Uri.substr(Pos + 1);

  // Decode the authority of the URI.
  std::string UriAuthority;
  यदि (Uri.consume_front("//")) {
    Pos = Uri.find('/');
    UriAuthority = percentDecode(Uri.substr(0, Pos));
    Uri = Uri.substr(Pos);
  }

  // Decode the body of the URI.
  std::string UriBody = percentDecode(Uri);

  // Compute the absolute path for this uri.
  यदि (!getSupportedSchemes().contains(UriScheme)) {
    प्रत्यागम llvm::createStringError(llvm::inconvertibleErrorCode(),
                                   "unsupported URI scheme `" + UriScheme +
                                       "' for workspace files");
  }
  प्रत्यागम getAbsolutePath(UriAuthority, UriBody);
}

llvm::Expected<URIForFile> URIForFile::fromURI(StringRef Uri) {
  llvm::Expected<std::string> FilePath = parseFilePathFromURI(Uri);
  यदि (!FilePath)
    प्रत्यागम FilePath.takeError();
  प्रत्यागम URIForFile(std::move(*FilePath), Uri.str());
}

llvm::Expected<URIForFile> URIForFile::fromFile(StringRef AbsoluteFilepath,
                                                StringRef Scheme) {
  llvm::Expected<std::string> Uri =
      uriFromAbsolutePath(AbsoluteFilepath, Scheme);
  यदि (!Uri)
    प्रत्यागम Uri.takeError();
  प्रत्यागम fromURI(*Uri);
}

StringRef URIForFile::scheme() ध्रुव { प्रत्यागम uri().split(':').first; }

शून्य URIForFile::registerSupportedScheme(StringRef Scheme) {
  getSupportedSchemes().जोड़ो(Scheme);
}

तार्किक llvm::lsp::fromJSON(ध्रुव llvm::json::मूल्य &मूल्य, URIForFile &Result,
                         llvm::json::Path Path) {
  यदि (std::optional<StringRef> Str = मूल्य.getAsString()) {
    llvm::Expected<URIForFile> ExpectedUri = URIForFile::fromURI(*Str);
    यदि (!ExpectedUri) {
      Path.report("unresolvable URI");
      consumeError(ExpectedUri.takeError());
      प्रत्यागम असत्य;
    }
    Result = std::move(*ExpectedUri);
    प्रत्यागम सत्य;
  }
  प्रत्यागम असत्य;
}

llvm::json::मूल्य llvm::lsp::toJSON(ध्रुव URIForFile &मूल्य) {
  प्रत्यागम मूल्य.uri();
}

raw_ostream &llvm::lsp::चालक<<(raw_ostream &Os, ध्रुव URIForFile &मूल्य) {
  प्रत्यागम Os << मूल्य.uri();
}

//===----------------------------------------------------------------------===//
// ClientCapabilities
//===----------------------------------------------------------------------===//

तार्किक llvm::lsp::fromJSON(ध्रुव llvm::json::मूल्य &मूल्य,
                         ClientCapabilities &Result, llvm::json::Path Path) {
  ध्रुव llvm::json::Object *O = मूल्य.getAsObject();
  यदि (!O) {
    Path.report("expected object");
    प्रत्यागम असत्य;
  }
  यदि (ध्रुव llvm::json::Object *TextDocument = O->getObject("textDocument")) {
    यदि (ध्रुव llvm::json::Object *Document_प्रतीक =
            TextDocument->getObject("documentSymbol")) {
      यदि (std::optional<तार्किक> HierarchicalSupport =
              Document_प्रतीक->getBoolean("hierarchicalDocumentSymbolSupport"))
        Result.hierarchical_Document_प्रतीक = *HierarchicalSupport;
    }
    यदि (स्वतः *CodeAction = TextDocument->getObject("codeAction")) {
      यदि (CodeAction->getObject("codeActionLiteralSupport"))
        Result.codeActionStructure = सत्य;
    }
  }
  यदि (स्वतः *Window = O->getObject("window")) {
    यदि (std::optional<तार्किक> WorkDoneProgressSupport =
            Window->getBoolean("workDoneProgress"))
      Result.workDoneProgress = *WorkDoneProgressSupport;
  }
  प्रत्यागम सत्य;
}

//===----------------------------------------------------------------------===//
// ClientInfo
//===----------------------------------------------------------------------===//

तार्किक llvm::lsp::fromJSON(ध्रुव llvm::json::मूल्य &मूल्य, ClientInfo &Result,
                         llvm::json::Path Path) {
  llvm::json::ObjectMapper O(मूल्य, Path);
  यदि (!O || !O.map("name", Result.name))
    प्रत्यागम असत्य;

  // Don't fail if we can't parse version.
  O.map("version", Result.version);
  प्रत्यागम सत्य;
}

//===----------------------------------------------------------------------===//
// InitializeParams
//===----------------------------------------------------------------------===//

तार्किक llvm::lsp::fromJSON(ध्रुव llvm::json::मूल्य &मूल्य, TraceLevel &Result,
                         llvm::json::Path Path) {
  यदि (std::optional<StringRef> Str = मूल्य.getAsString()) {
    यदि (*Str == "off") {
      Result = TraceLevel::Off;
      प्रत्यागम सत्य;
    }
    यदि (*Str == "messages") {
      Result = TraceLevel::Messages;
      प्रत्यागम सत्य;
    }
    यदि (*Str == "verbose") {
      Result = TraceLevel::Verbose;
      प्रत्यागम सत्य;
    }
  }
  प्रत्यागम असत्य;
}

तार्किक llvm::lsp::fromJSON(ध्रुव llvm::json::मूल्य &मूल्य,
                         InitializeParams &Result, llvm::json::Path Path) {
  llvm::json::ObjectMapper O(मूल्य, Path);
  यदि (!O)
    प्रत्यागम असत्य;
  // We deliberately don't fail if we can't parse individual fields.
  O.map("capabilities", Result.capabilities);
  O.map("trace", Result.trace);
  O.map("rootUri", Result.मूल_Uri);
  O.map("rootPath", Result.मूल_Path);
  mapOptOrNull(मूल्य, "clientInfo", Result.clientInfo, Path);

  प्रत्यागम सत्य;
}

//===----------------------------------------------------------------------===//
// TextDocumentItem
//===----------------------------------------------------------------------===//

तार्किक llvm::lsp::fromJSON(ध्रुव llvm::json::मूल्य &मूल्य,
                         TextDocumentItem &Result, llvm::json::Path Path) {
  llvm::json::ObjectMapper O(मूल्य, Path);
  प्रत्यागम O && O.map("uri", Result.uri) &&
         O.map("languageId", Result.languageId) && O.map("text", Result.text) &&
         O.map("version", Result.version);
}

//===----------------------------------------------------------------------===//
// TextDocumentIdentifier
//===----------------------------------------------------------------------===//

llvm::json::मूल्य llvm::lsp::toJSON(ध्रुव TextDocumentIdentifier &मूल्य) {
  प्रत्यागम llvm::json::Object{{"uri", मूल्य.uri}};
}

तार्किक llvm::lsp::fromJSON(ध्रुव llvm::json::मूल्य &मूल्य,
                         TextDocumentIdentifier &Result,
                         llvm::json::Path Path) {
  llvm::json::ObjectMapper O(मूल्य, Path);
  प्रत्यागम O && O.map("uri", Result.uri);
}

//===----------------------------------------------------------------------===//
// VersionedTextDocumentIdentifier
//===----------------------------------------------------------------------===//

llvm::json::मूल्य
llvm::lsp::toJSON(ध्रुव VersionedTextDocumentIdentifier &मूल्य) {
  प्रत्यागम llvm::json::Object{
      {"uri", मूल्य.uri},
      {"version", मूल्य.version},
  };
}

तार्किक llvm::lsp::fromJSON(ध्रुव llvm::json::मूल्य &मूल्य,
                         VersionedTextDocumentIdentifier &Result,
                         llvm::json::Path Path) {
  llvm::json::ObjectMapper O(मूल्य, Path);
  प्रत्यागम O && O.map("uri", Result.uri) && O.map("version", Result.version);
}

//===----------------------------------------------------------------------===//
// Position
//===----------------------------------------------------------------------===//

तार्किक llvm::lsp::fromJSON(ध्रुव llvm::json::मूल्य &मूल्य, Position &Result,
                         llvm::json::Path Path) {
  llvm::json::ObjectMapper O(मूल्य, Path);
  प्रत्यागम O && O.map("line", Result.पंक्ति_2) &&
         O.map("character", Result.character);
}

llvm::json::मूल्य llvm::lsp::toJSON(ध्रुव Position &मूल्य) {
  प्रत्यागम llvm::json::Object{
      {"line", मूल्य.पंक्ति_2},
      {"character", मूल्य.character},
  };
}

raw_ostream &llvm::lsp::चालक<<(raw_ostream &Os, ध्रुव Position &मूल्य) {
  प्रत्यागम Os << मूल्य.पंक्ति_2 << ':' << मूल्य.character;
}

//===----------------------------------------------------------------------===//
// Range
//===----------------------------------------------------------------------===//

तार्किक llvm::lsp::fromJSON(ध्रुव llvm::json::मूल्य &मूल्य, Range &Result,
                         llvm::json::Path Path) {
  llvm::json::ObjectMapper O(मूल्य, Path);
  प्रत्यागम O && O.map("start", Result.start) && O.map("end", Result.end);
}

llvm::json::मूल्य llvm::lsp::toJSON(ध्रुव Range &मूल्य) {
  प्रत्यागम llvm::json::Object{
      {"start", मूल्य.start},
      {"end", मूल्य.end},
  };
}

raw_ostream &llvm::lsp::चालक<<(raw_ostream &Os, ध्रुव Range &मूल्य) {
  प्रत्यागम Os << मूल्य.start << '-' << मूल्य.end;
}

//===----------------------------------------------------------------------===//
// Location
//===----------------------------------------------------------------------===//

तार्किक llvm::lsp::fromJSON(ध्रुव llvm::json::मूल्य &मूल्य, Location &Result,
                         llvm::json::Path Path) {
  llvm::json::ObjectMapper O(मूल्य, Path);
  प्रत्यागम O && O.map("uri", Result.uri) && O.map("range", Result.range);
}

llvm::json::मूल्य llvm::lsp::toJSON(ध्रुव Location &मूल्य) {
  प्रत्यागम llvm::json::Object{
      {"uri", मूल्य.uri},
      {"range", मूल्य.range},
  };
}

raw_ostream &llvm::lsp::चालक<<(raw_ostream &Os, ध्रुव Location &मूल्य) {
  प्रत्यागम Os << मूल्य.range << '@' << मूल्य.uri;
}

//===----------------------------------------------------------------------===//
// TextDocumentPositionParams
//===----------------------------------------------------------------------===//

तार्किक llvm::lsp::fromJSON(ध्रुव llvm::json::मूल्य &मूल्य,
                         TextDocumentPositionParams &Result,
                         llvm::json::Path Path) {
  llvm::json::ObjectMapper O(मूल्य, Path);
  प्रत्यागम O && O.map("textDocument", Result.textDocument) &&
         O.map("position", Result.position);
}

//===----------------------------------------------------------------------===//
// ReferenceParams
//===----------------------------------------------------------------------===//

तार्किक llvm::lsp::fromJSON(ध्रुव llvm::json::मूल्य &मूल्य,
                         ReferenceContext &Result, llvm::json::Path Path) {
  llvm::json::ObjectMapper O(मूल्य, Path);
  प्रत्यागम O && O.mapOptional("includeDeclaration", Result.includeDeclaration);
}

तार्किक llvm::lsp::fromJSON(ध्रुव llvm::json::मूल्य &मूल्य,
                         ReferenceParams &Result, llvm::json::Path Path) {
  TextDocumentPositionParams &Base = Result;
  llvm::json::ObjectMapper O(मूल्य, Path);
  प्रत्यागम fromJSON(मूल्य, Base, Path) && O &&
         O.mapOptional("context", Result.context);
}

//===----------------------------------------------------------------------===//
// DidOpenTextDocumentParams
//===----------------------------------------------------------------------===//

तार्किक llvm::lsp::fromJSON(ध्रुव llvm::json::मूल्य &मूल्य,
                         DidOpenTextDocumentParams &Result,
                         llvm::json::Path Path) {
  llvm::json::ObjectMapper O(मूल्य, Path);
  प्रत्यागम O && O.map("textDocument", Result.textDocument);
}

//===----------------------------------------------------------------------===//
// DidCloseTextDocumentParams
//===----------------------------------------------------------------------===//

तार्किक llvm::lsp::fromJSON(ध्रुव llvm::json::मूल्य &मूल्य,
                         DidCloseTextDocumentParams &Result,
                         llvm::json::Path Path) {
  llvm::json::ObjectMapper O(मूल्य, Path);
  प्रत्यागम O && O.map("textDocument", Result.textDocument);
}

//===----------------------------------------------------------------------===//
// DidSaveTextDocumentParams
//===----------------------------------------------------------------------===//

तार्किक llvm::lsp::fromJSON(ध्रुव llvm::json::मूल्य &Params,
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
  यदि (!range) {
    Contents = text;
    प्रत्यागम success();
  }

  // Try to map the replacement range to the content.
  llvm::SourceMgr TmpScrMgr;
  TmpScrMgr.AddNewSourceBuffer(llvm::MemoryBuffer::getMemBuffer(Contents),
                               SMLoc());
  SMRange RangeLoc = range->getAsSMRange(TmpScrMgr);
  यदि (!RangeLoc.isValid())
    प्रत्यागम failure();

  Contents.replace(RangeLoc.Start.getPointer() - Contents.data(),
                   RangeLoc.End.getPointer() - RangeLoc.Start.getPointer(),
                   text);
  प्रत्यागम success();
}

LogicalResult TextDocumentContentChangeEvent::applyTo(
    ArrayRef<TextDocumentContentChangeEvent> Changes, std::string &Contents) {
  प्रति (ध्रुव स्वतः &Change : Changes)
    यदि (failed(Change.applyTo(Contents)))
      प्रत्यागम failure();
  प्रत्यागम success();
}

तार्किक llvm::lsp::fromJSON(ध्रुव llvm::json::मूल्य &मूल्य,
                         TextDocumentContentChangeEvent &Result,
                         llvm::json::Path Path) {
  llvm::json::ObjectMapper O(मूल्य, Path);
  प्रत्यागम O && O.map("range", Result.range) &&
         O.map("rangeLength", Result.rangeLength) && O.map("text", Result.text);
}

तार्किक llvm::lsp::fromJSON(ध्रुव llvm::json::मूल्य &मूल्य,
                         DidChangeTextDocumentParams &Result,
                         llvm::json::Path Path) {
  llvm::json::ObjectMapper O(मूल्य, Path);
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

raw_ostream &llvm::lsp::चालक<<(raw_ostream &Os, MarkupKind Kind) {
  प्रत्यागम Os << toTextKind(Kind);
}

llvm::json::मूल्य llvm::lsp::toJSON(ध्रुव MarkupContent &Mc) {
  यदि (Mc.मूल्य_2.empty())
    प्रत्यागम nullptr;

  प्रत्यागम llvm::json::Object{
      {"kind", toTextKind(Mc.kind)},
      {"value", Mc.मूल्य_2},
  };
}

//===----------------------------------------------------------------------===//
// Hover
//===----------------------------------------------------------------------===//

llvm::json::मूल्य llvm::lsp::toJSON(ध्रुव Hover &Hover) {
  llvm::json::Object Result{{"contents", toJSON(Hover.contents)}};
  यदि (Hover.range)
    Result["range"] = toJSON(*Hover.range);
  प्रत्यागम std::move(Result);
}

//===----------------------------------------------------------------------===//
// DocumentSymbol
//===----------------------------------------------------------------------===//

llvm::json::मूल्य llvm::lsp::toJSON(ध्रुव Document_प्रतीक &प्रतीक) {
  llvm::json::Object Result{{"name", प्रतीक.name},
                            {"kind", static_cast<पूर्णाङ्क>(प्रतीक.kind)},
                            {"range", प्रतीक.range},
                            {"selectionRange", प्रतीक.selectionRange}};

  यदि (!प्रतीक.detail.empty())
    Result["detail"] = प्रतीक.detail;
  यदि (!प्रतीक.children.empty())
    Result["children"] = प्रतीक.children;
  प्रत्यागम std::move(Result);
}

//===----------------------------------------------------------------------===//
// DocumentSymbolParams
//===----------------------------------------------------------------------===//

तार्किक llvm::lsp::fromJSON(ध्रुव llvm::json::मूल्य &मूल्य,
                         Document_प्रतीक_Params &Result, llvm::json::Path Path) {
  llvm::json::ObjectMapper O(मूल्य, Path);
  प्रत्यागम O && O.map("textDocument", Result.textDocument);
}

//===----------------------------------------------------------------------===//
// DiagnosticRelatedInformation
//===----------------------------------------------------------------------===//

तार्किक llvm::lsp::fromJSON(ध्रुव llvm::json::मूल्य &मूल्य,
                         DiagnosticRelatedInformation &Result,
                         llvm::json::Path Path) {
  llvm::json::ObjectMapper O(मूल्य, Path);
  प्रत्यागम O && O.map("location", Result.location) &&
         O.map("message", Result.message);
}

llvm::json::मूल्य llvm::lsp::toJSON(ध्रुव DiagnosticRelatedInformation &Info) {
  प्रत्यागम llvm::json::Object{
      {"location", Info.location},
      {"message", Info.message},
  };
}

//===----------------------------------------------------------------------===//
// Diagnostic
//===----------------------------------------------------------------------===//

llvm::json::मूल्य llvm::lsp::toJSON(DiagnosticTag Tag) {
  प्रत्यागम static_cast<पूर्णाङ्क>(Tag);
}

तार्किक llvm::lsp::fromJSON(ध्रुव llvm::json::मूल्य &मूल्य, DiagnosticTag &Result,
                         llvm::json::Path Path) {
  यदि (std::optional<int64_t> I = मूल्य.getAsInteger()) {
    Result = (DiagnosticTag)*I;
    प्रत्यागम सत्य;
  }

  प्रत्यागम असत्य;
}

llvm::json::मूल्य llvm::lsp::toJSON(ध्रुव Diagnostic &Diag) {
  llvm::json::Object Result{
      {"range", Diag.range},
      {"severity", (पूर्णाङ्क)Diag.severity},
      {"message", Diag.message},
  };
  यदि (Diag.category)
    Result["category"] = *Diag.category;
  यदि (!Diag.source.empty())
    Result["source"] = Diag.source;
  यदि (Diag.relatedInformation)
    Result["relatedInformation"] = *Diag.relatedInformation;
  यदि (!Diag.tags.empty())
    Result["tags"] = Diag.tags;
  प्रत्यागम std::move(Result);
}

तार्किक llvm::lsp::fromJSON(ध्रुव llvm::json::मूल्य &मूल्य, Diagnostic &Result,
                         llvm::json::Path Path) {
  llvm::json::ObjectMapper O(मूल्य, Path);
  यदि (!O)
    प्रत्यागम असत्य;
  पूर्णाङ्क Severity = 0;
  यदि (!mapOptOrNull(मूल्य, "severity", Severity, Path))
    प्रत्यागम असत्य;
  Result.severity = (DiagnosticSeverity)Severity;

  प्रत्यागम O.map("range", Result.range) && O.map("message", Result.message) &&
         mapOptOrNull(मूल्य, "category", Result.category, Path) &&
         mapOptOrNull(मूल्य, "source", Result.source, Path) &&
         mapOptOrNull(मूल्य, "relatedInformation", Result.relatedInformation,
                      Path) &&
         mapOptOrNull(मूल्य, "tags", Result.tags, Path);
}

//===----------------------------------------------------------------------===//
// PublishDiagnosticsParams
//===----------------------------------------------------------------------===//

llvm::json::मूल्य llvm::lsp::toJSON(ध्रुव PublishDiagnosticsParams &Params) {
  प्रत्यागम llvm::json::Object{
      {"uri", Params.uri},
      {"diagnostics", Params.diagnostics},
      {"version", Params.version},
  };
}

//===----------------------------------------------------------------------===//
// TextEdit
//===----------------------------------------------------------------------===//

तार्किक llvm::lsp::fromJSON(ध्रुव llvm::json::मूल्य &मूल्य, TextEdit &Result,
                         llvm::json::Path Path) {
  llvm::json::ObjectMapper O(मूल्य, Path);
  प्रत्यागम O && O.map("range", Result.range) && O.map("newText", Result.newText);
}

llvm::json::मूल्य llvm::lsp::toJSON(ध्रुव TextEdit &मूल्य) {
  प्रत्यागम llvm::json::Object{
      {"range", मूल्य.range},
      {"newText", मूल्य.newText},
  };
}

raw_ostream &llvm::lsp::चालक<<(raw_ostream &Os, ध्रुव TextEdit &मूल्य) {
  Os << मूल्य.range << " => \"";
  llvm::printEscapedString(मूल्य.newText, Os);
  प्रत्यागम Os << '"';
}

//===----------------------------------------------------------------------===//
// CompletionItemKind
//===----------------------------------------------------------------------===//

तार्किक llvm::lsp::fromJSON(ध्रुव llvm::json::मूल्य &मूल्य,
                         CompletionItemKind &Result, llvm::json::Path Path) {
  यदि (std::optional<int64_t> Int_मूल्य = मूल्य.getAsInteger()) {
    यदि (*Int_मूल्य < static_cast<पूर्णाङ्क>(CompletionItemKind::Text) ||
        *Int_मूल्य > static_cast<पूर्णाङ्क>(CompletionItemKind::TypeParameter))
      प्रत्यागम असत्य;
    Result = static_cast<CompletionItemKind>(*Int_मूल्य);
    प्रत्यागम सत्य;
  }
  प्रत्यागम असत्य;
}

CompletionItemKind llvm::lsp::adjustKindToCapability(
    CompletionItemKind Kind,
    CompletionItemKindBitset &SupportedCompletionItemKinds) {
  माप_प्रकार KindVal = static_cast<माप_प्रकार>(Kind);
  यदि (KindVal >= kCompletionItemKindMin &&
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

तार्किक llvm::lsp::fromJSON(ध्रुव llvm::json::मूल्य &मूल्य,
                         CompletionItemKindBitset &Result,
                         llvm::json::Path Path) {
  यदि (ध्रुव llvm::json::Array *Array_मूल्य = मूल्य.getAsArray()) {
    प्रति (माप_प्रकार I = 0, E = Array_मूल्य->size(); I < E; ++I) {
      CompletionItemKind KindOut;
      यदि (fromJSON((*Array_मूल्य)[I], KindOut, Path.index(I)))
        Result.set(माप_प्रकार(KindOut));
    }
    प्रत्यागम सत्य;
  }
  प्रत्यागम असत्य;
}

//===----------------------------------------------------------------------===//
// CompletionItem
//===----------------------------------------------------------------------===//

llvm::json::मूल्य llvm::lsp::toJSON(ध्रुव CompletionItem &मूल्य) {
  प्रतिज्ञा(!मूल्य.label.empty() && "completion item label is required");
  llvm::json::Object Result{{"label", मूल्य.label}};
  यदि (मूल्य.kind != CompletionItemKind::Missing)
    Result["kind"] = static_cast<पूर्णाङ्क>(मूल्य.kind);
  यदि (!मूल्य.detail.empty())
    Result["detail"] = मूल्य.detail;
  यदि (मूल्य.documentation)
    Result["documentation"] = मूल्य.documentation;
  यदि (!मूल्य.sortText.empty())
    Result["sortText"] = मूल्य.sortText;
  यदि (!मूल्य.filterText.empty())
    Result["filterText"] = मूल्य.filterText;
  यदि (!मूल्य.जोड़ो_Text.empty())
    Result["insertText"] = मूल्य.जोड़ो_Text;
  यदि (मूल्य.जोड़ो_Text_Format != जोड़ो_Text_Format_2::Missing)
    Result["insertTextFormat"] = static_cast<पूर्णाङ्क>(मूल्य.जोड़ो_Text_Format);
  यदि (मूल्य.textEdit)
    Result["textEdit"] = *मूल्य.textEdit;
  यदि (!मूल्य.additionalTextEdits.empty()) {
    Result["additionalTextEdits"] =
        llvm::json::Array(मूल्य.additionalTextEdits);
  }
  यदि (मूल्य.deprecated)
    Result["deprecated"] = मूल्य.deprecated;
  प्रत्यागम std::move(Result);
}

raw_ostream &llvm::lsp::चालक<<(raw_ostream &Os,
                                   ध्रुव CompletionItem &मूल्य) {
  प्रत्यागम Os << मूल्य.label << " - " << toJSON(मूल्य);
}

तार्किक llvm::lsp::चालक<(ध्रुव CompletionItem &Lhs,
                          ध्रुव CompletionItem &Rhs) {
  प्रत्यागम (Lhs.sortText.empty() ? Lhs.label : Lhs.sortText) <
         (Rhs.sortText.empty() ? Rhs.label : Rhs.sortText);
}

//===----------------------------------------------------------------------===//
// CompletionList
//===----------------------------------------------------------------------===//

llvm::json::मूल्य llvm::lsp::toJSON(ध्रुव CompletionList &मूल्य) {
  प्रत्यागम llvm::json::Object{
      {"isIncomplete", मूल्य.isIncomplete},
      {"items", llvm::json::Array(मूल्य.वस्तुएँ)},
  };
}

//===----------------------------------------------------------------------===//
// CompletionContext
//===----------------------------------------------------------------------===//

तार्किक llvm::lsp::fromJSON(ध्रुव llvm::json::मूल्य &मूल्य,
                         CompletionContext &Result, llvm::json::Path Path) {
  llvm::json::ObjectMapper O(मूल्य, Path);
  पूर्णाङ्क TriggerKind;
  यदि (!O || !O.map("triggerKind", TriggerKind) ||
      !mapOptOrNull(मूल्य, "triggerCharacter", Result.triggerCharacter, Path))
    प्रत्यागम असत्य;
  Result.triggerKind = static_cast<CompletionTriggerKind>(TriggerKind);
  प्रत्यागम सत्य;
}

//===----------------------------------------------------------------------===//
// CompletionParams
//===----------------------------------------------------------------------===//

तार्किक llvm::lsp::fromJSON(ध्रुव llvm::json::मूल्य &मूल्य,
                         CompletionParams &Result, llvm::json::Path Path) {
  यदि (!fromJSON(मूल्य, static_cast<TextDocumentPositionParams &>(Result), Path))
    प्रत्यागम असत्य;
  यदि (ध्रुव llvm::json::मूल्य *Context = मूल्य.getAsObject()->get("context"))
    प्रत्यागम fromJSON(*Context, Result.context, Path.field("context"));
  प्रत्यागम सत्य;
}

//===----------------------------------------------------------------------===//
// ParameterInformation
//===----------------------------------------------------------------------===//

llvm::json::मूल्य llvm::lsp::toJSON(ध्रुव ParameterInformation &मूल्य) {
  प्रतिज्ञा((मूल्य.labelOffsets || !मूल्य.labelString.empty()) &&
         "parameter information label is required");
  llvm::json::Object Result;
  यदि (मूल्य.labelOffsets)
    Result["label"] = llvm::json::Array(
        {मूल्य.labelOffsets->first, मूल्य.labelOffsets->second});
  अन्यथा
    Result["label"] = मूल्य.labelString;
  यदि (!मूल्य.documentation.empty())
    Result["documentation"] = मूल्य.documentation;
  प्रत्यागम std::move(Result);
}

//===----------------------------------------------------------------------===//
// SignatureInformation
//===----------------------------------------------------------------------===//

llvm::json::मूल्य llvm::lsp::toJSON(ध्रुव SignatureInformation &मूल्य) {
  प्रतिज्ञा(!मूल्य.label.empty() && "signature information label is required");
  llvm::json::Object Result{
      {"label", मूल्य.label},
      {"parameters", llvm::json::Array(मूल्य.parameters)},
  };
  यदि (!मूल्य.documentation.empty())
    Result["documentation"] = मूल्य.documentation;
  प्रत्यागम std::move(Result);
}

raw_ostream &llvm::lsp::चालक<<(raw_ostream &Os,
                                   ध्रुव SignatureInformation &मूल्य) {
  प्रत्यागम Os << मूल्य.label << " - " << toJSON(मूल्य);
}

//===----------------------------------------------------------------------===//
// SignatureHelp
//===----------------------------------------------------------------------===//

llvm::json::मूल्य llvm::lsp::toJSON(ध्रुव SignatureHelp &मूल्य) {
  प्रतिज्ञा(मूल्य.activeSignature >= 0 &&
         "Unexpected negative value for number of active signatures.");
  प्रतिज्ञा(मूल्य.activeParameter >= 0 &&
         "Unexpected negative value for active parameter index");
  प्रत्यागम llvm::json::Object{
      {"activeSignature", मूल्य.activeSignature},
      {"activeParameter", मूल्य.activeParameter},
      {"signatures", llvm::json::Array(मूल्य.signatures)},
  };
}

//===----------------------------------------------------------------------===//
// DocumentLinkParams
//===----------------------------------------------------------------------===//

तार्किक llvm::lsp::fromJSON(ध्रुव llvm::json::मूल्य &मूल्य,
                         DocumentLinkParams &Result, llvm::json::Path Path) {
  llvm::json::ObjectMapper O(मूल्य, Path);
  प्रत्यागम O && O.map("textDocument", Result.textDocument);
}

//===----------------------------------------------------------------------===//
// DocumentLink
//===----------------------------------------------------------------------===//

llvm::json::मूल्य llvm::lsp::toJSON(ध्रुव DocumentLink &मूल्य) {
  प्रत्यागम llvm::json::Object{
      {"range", मूल्य.range},
      {"target", मूल्य.target},
  };
}

//===----------------------------------------------------------------------===//
// InlayHintsParams
//===----------------------------------------------------------------------===//

तार्किक llvm::lsp::fromJSON(ध्रुव llvm::json::मूल्य &मूल्य,
                         InlayHintsParams &Result, llvm::json::Path Path) {
  llvm::json::ObjectMapper O(मूल्य, Path);
  प्रत्यागम O && O.map("textDocument", Result.textDocument) &&
         O.map("range", Result.range);
}

//===----------------------------------------------------------------------===//
// InlayHint
//===----------------------------------------------------------------------===//

llvm::json::मूल्य llvm::lsp::toJSON(ध्रुव InlayHint &मूल्य) {
  प्रत्यागम llvm::json::Object{{"position", मूल्य.position},
                            {"kind", (पूर्णाङ्क)मूल्य.kind},
                            {"label", मूल्य.label},
                            {"paddingLeft", मूल्य.padding_बायाँ},
                            {"paddingRight", मूल्य.padding_दायाँ}};
}
तार्किक llvm::lsp::चालक==(ध्रुव InlayHint &Lhs, ध्रुव InlayHint &Rhs) {
  प्रत्यागम std::tie(Lhs.position, Lhs.kind, Lhs.label) ==
         std::tie(Rhs.position, Rhs.kind, Rhs.label);
}
तार्किक llvm::lsp::चालक<(ध्रुव InlayHint &Lhs, ध्रुव InlayHint &Rhs) {
  प्रत्यागम std::tie(Lhs.position, Lhs.kind, Lhs.label) <
         std::tie(Rhs.position, Rhs.kind, Rhs.label);
}

llvm::raw_ostream &llvm::lsp::चालक<<(llvm::raw_ostream &Os,
                                         InlayHintKind मूल्य) {
  विकल्प (मूल्य) {
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

तार्किक llvm::lsp::fromJSON(ध्रुव llvm::json::मूल्य &मूल्य,
                         CodeActionContext &Result, llvm::json::Path Path) {
  llvm::json::ObjectMapper O(मूल्य, Path);
  यदि (!O || !O.map("diagnostics", Result.diagnostics))
    प्रत्यागम असत्य;
  O.map("only", Result.only);
  प्रत्यागम सत्य;
}

//===----------------------------------------------------------------------===//
// CodeActionParams
//===----------------------------------------------------------------------===//

तार्किक llvm::lsp::fromJSON(ध्रुव llvm::json::मूल्य &मूल्य,
                         CodeActionParams &Result, llvm::json::Path Path) {
  llvm::json::ObjectMapper O(मूल्य, Path);
  प्रत्यागम O && O.map("textDocument", Result.textDocument) &&
         O.map("range", Result.range) && O.map("context", Result.context);
}

//===----------------------------------------------------------------------===//
// WorkspaceEdit
//===----------------------------------------------------------------------===//

तार्किक llvm::lsp::fromJSON(ध्रुव llvm::json::मूल्य &मूल्य, WorkspaceEdit &Result,
                         llvm::json::Path Path) {
  llvm::json::ObjectMapper O(मूल्य, Path);
  प्रत्यागम O && O.map("changes", Result.changes);
}

llvm::json::मूल्य llvm::lsp::toJSON(ध्रुव WorkspaceEdit &मूल्य) {
  llvm::json::Object FileChanges;
  प्रति (स्वतः &Change : मूल्य.changes)
    FileChanges[Change.first] = llvm::json::Array(Change.second);
  प्रत्यागम llvm::json::Object{{"changes", std::move(FileChanges)}};
}

//===----------------------------------------------------------------------===//
// CodeAction
//===----------------------------------------------------------------------===//

ध्रुव llvm::StringLiteral CodeAction::kQuickFix = "quickfix";
ध्रुव llvm::StringLiteral CodeAction::kRefactor = "refactor";
ध्रुव llvm::StringLiteral CodeAction::kInfo = "info";

llvm::json::मूल्य llvm::lsp::toJSON(ध्रुव CodeAction &मूल्य) {
  llvm::json::Object CodeAction{{"title", मूल्य.title}};
  यदि (मूल्य.kind)
    CodeAction["kind"] = *मूल्य.kind;
  यदि (मूल्य.diagnostics)
    CodeAction["diagnostics"] = llvm::json::Array(*मूल्य.diagnostics);
  यदि (मूल्य.isPreferred)
    CodeAction["isPreferred"] = सत्य;
  यदि (मूल्य.edit)
    CodeAction["edit"] = *मूल्य.edit;
  प्रत्यागम std::move(CodeAction);
}

//===----------------------------------------------------------------------===//
// ShowMessageParams
//===----------------------------------------------------------------------===//

llvm::json::मूल्य llvm::lsp::toJSON(ध्रुव ShowMessageParams &Params) {
  स्वतः Out = llvm::json::Object{
      {"type", static_cast<पूर्णाङ्क>(Params.type)},
      {"message", Params.message},
  };
  यदि (Params.actions)
    Out["actions"] = *Params.actions;
  प्रत्यागम Out;
}

llvm::json::मूल्य llvm::lsp::toJSON(ध्रुव MessageActionItem &Params) {
  प्रत्यागम llvm::json::Object{{"title", Params.title}};
}
