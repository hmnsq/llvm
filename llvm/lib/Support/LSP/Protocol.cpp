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
स्थिर तार्किक mapOptOrNull(अचर llvm::json::मान &Params,
                         llvm::StringLiteral Prop, T &Out,
                         llvm::json::Path Path) {
  अचर llvm::json::Object *O = Params.getAsObject();
  प्रतिज्ञा(O);

  // Field is missing or null.
  स्वतः *V = O->get(Prop);
  यदि (!V || V->getAsNull())
    फर्काऊ सत्य;
  फर्काऊ fromJSON(*V, Out, Path.field(Prop));
}

//===----------------------------------------------------------------------===//
// LSPError
//===----------------------------------------------------------------------===//

वर्ण LSPError::ID;

//===----------------------------------------------------------------------===//
// URIForFile
//===----------------------------------------------------------------------===//

स्थिर तार्किक isWindowsPath(StringRef Path) {
  फर्काऊ Path.size() > 1 && llvm::isAlpha(Path[0]) && Path[1] == ':';
}

स्थिर तार्किक isNetworkPath(StringRef Path) {
  फर्काऊ Path.size() > 2 && Path[0] == Path[1] &&
         llvm::sys::path::is_separator(Path[0]);
}

स्थिर तार्किक shouldEscapeInURI(अचिन्हित वर्ण C) {
  // Unreserved characters.
  यदि ((C >= 'a' && C <= 'z') || (C >= 'A' && C <= 'Z') ||
      (C >= '0' && C <= '9'))
    फर्काऊ असत्य;

  विकल्प (C) {
  पक्ष '-':
  पक्ष '_':
  पक्ष '.':
  पक्ष '~':
  // '/' is only reserved when parsing.
  पक्ष '/':
  // ':' is only reserved for relative URI paths, which we doesn't produce.
  पक्ष ':':
    फर्काऊ असत्य;
  }
  फर्काऊ सत्य;
}

/// Encodes a string according to percent-encoding.
/// - Unreserved characters are not escaped.
/// - Reserved characters always escaped with exceptions like '/'.
/// - All other characters are escaped.
स्थिर शून्य percentEncode(StringRef Content, std::string &Out) {
  लागि (अचिन्हित वर्ण C : Content) {
    यदि (shouldEscapeInURI(C)) {
      Out.थप_back('%');
      Out.थप_back(llvm::hexdigit(C / 16));
      Out.थप_back(llvm::hexdigit(C % 16));
    } अन्यथा {
      Out.थप_back(C);
    }
  }
}

/// Decodes a string according to percent-encoding.
स्थिर std::string percentDecode(StringRef Content) {
  std::string Result;
  लागि (स्वतः I = Content.begin(), E = Content.end(); I != E; ++I) {
    यदि (*I == '%' && I + 2 < Content.end() && llvm::isHexDigit(*(I + 1)) &&
        llvm::isHexDigit(*(I + 2))) {
      Result.थप_back(llvm::hexFromNibbles(*(I + 1), *(I + 2)));
      I += 2;
    } अन्यथा {
      Result.थप_back(*I);
    }
  }
  फर्काऊ Result;
}

/// Return the set containing the supported URI schemes.
स्थिर StringSet<> &getSupportedSchemes() {
  स्थिर StringSet<> Schemes({"file", "test"});
  फर्काऊ Schemes;
}

/// Returns true if the given scheme is structurally valid, i.e. it does not
/// contain any invalid scheme characters. This does not check that the scheme
/// is actually supported.
स्थिर तार्किक isStructurallyValidScheme(StringRef Scheme) {
  यदि (Scheme.empty())
    फर्काऊ असत्य;
  यदि (!llvm::isAlpha(Scheme[0]))
    फर्काऊ असत्य;
  फर्काऊ llvm::all_of(llvm::drop_begin(Scheme), [](वर्ण C) {
    फर्काऊ llvm::isAlnum(C) || C == '+' || C == '.' || C == '-';
  });
}

स्थिर llvm::Expected<std::string> uriFromAbsolutePath(StringRef AbsolutePath,
                                                       StringRef Scheme) {
  std::string Body;
  StringRef Authority;
  StringRef जरा_2 = llvm::sys::path::जरा_name(AbsolutePath);
  यदि (isNetworkPath(जरा_2)) {
    // Windows UNC paths e.g. \\server\share => file://server/share
    Authority = जरा_2.drop_front(2);
    AbsolutePath.consume_front(जरा_2);
  } अन्यथा यदि (isWindowsPath(जरा_2)) {
    // Windows paths e.g. X:\path => file:///X:/path
    Body = "/";
  }
  Body += llvm::sys::path::convert_to_slash(AbsolutePath);

  std::string Uri = Scheme.str() + ":";
  यदि (Authority.empty() && Body.empty())
    फर्काऊ Uri;

  // If authority if empty, we only print body if it starts with "/"; otherwise,
  // the URI is invalid.
  यदि (!Authority.empty() || StringRef(Body).starts_with("/")) {
    Uri.append("//");
    percentEncode(Authority, Uri);
  }
  percentEncode(Body, Uri);
  फर्काऊ Uri;
}

स्थिर llvm::Expected<std::string> getAbsolutePath(StringRef Authority,
                                                   StringRef Body) {
  यदि (!Body.starts_with("/"))
    फर्काऊ llvm::createStringError(
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
  फर्काऊ std::string(Path);
}

स्थिर llvm::Expected<std::string> parseFilePathFromURI(StringRef OrigUri) {
  StringRef Uri = OrigUri;

  // Decode the scheme of the URI.
  माप_प्रकार Pos = Uri.find(':');
  यदि (Pos == StringRef::npos)
    फर्काऊ llvm::createStringError(llvm::inconvertibleErrorCode(),
                                   "Scheme must be provided in URI: " +
                                       OrigUri);
  StringRef SchemeStr = Uri.substr(0, Pos);
  std::string UriScheme = percentDecode(SchemeStr);
  यदि (!isStructurallyValidScheme(UriScheme))
    फर्काऊ llvm::createStringError(llvm::inconvertibleErrorCode(),
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
    फर्काऊ llvm::createStringError(llvm::inconvertibleErrorCode(),
                                   "unsupported URI scheme `" + UriScheme +
                                       "' for workspace files");
  }
  फर्काऊ getAbsolutePath(UriAuthority, UriBody);
}

llvm::Expected<URIForFile> URIForFile::fromURI(StringRef Uri) {
  llvm::Expected<std::string> FilePath = parseFilePathFromURI(Uri);
  यदि (!FilePath)
    फर्काऊ FilePath.takeError();
  फर्काऊ URIForFile(std::move(*FilePath), Uri.str());
}

llvm::Expected<URIForFile> URIForFile::fromFile(StringRef AbsoluteFilepath,
                                                StringRef Scheme) {
  llvm::Expected<std::string> Uri =
      uriFromAbsolutePath(AbsoluteFilepath, Scheme);
  यदि (!Uri)
    फर्काऊ Uri.takeError();
  फर्काऊ fromURI(*Uri);
}

StringRef URIForFile::scheme() अचर { फर्काऊ uri().split(':').first; }

शून्य URIForFile::registerSupportedScheme(StringRef Scheme) {
  getSupportedSchemes().घुसाउ(Scheme);
}

तार्किक llvm::lsp::fromJSON(अचर llvm::json::मान &मान, URIForFile &Result,
                         llvm::json::Path Path) {
  यदि (std::optional<StringRef> Str = मान.getAsString()) {
    llvm::Expected<URIForFile> ExpectedUri = URIForFile::fromURI(*Str);
    यदि (!ExpectedUri) {
      Path.report("unresolvable URI");
      consumeError(ExpectedUri.takeError());
      फर्काऊ असत्य;
    }
    Result = std::move(*ExpectedUri);
    फर्काऊ सत्य;
  }
  फर्काऊ असत्य;
}

llvm::json::मान llvm::lsp::toJSON(अचर URIForFile &मान) {
  फर्काऊ मान.uri();
}

raw_ostream &llvm::lsp::चालक<<(raw_ostream &Os, अचर URIForFile &मान) {
  फर्काऊ Os << मान.uri();
}

//===----------------------------------------------------------------------===//
// ClientCapabilities
//===----------------------------------------------------------------------===//

तार्किक llvm::lsp::fromJSON(अचर llvm::json::मान &मान,
                         ClientCapabilities &Result, llvm::json::Path Path) {
  अचर llvm::json::Object *O = मान.getAsObject();
  यदि (!O) {
    Path.report("expected object");
    फर्काऊ असत्य;
  }
  यदि (अचर llvm::json::Object *TextDocument = O->getObject("textDocument")) {
    यदि (अचर llvm::json::Object *Document_चिह्न =
            TextDocument->getObject("documentSymbol")) {
      यदि (std::optional<तार्किक> HierarchicalSupport =
              Document_चिह्न->getBoolean("hierarchicalDocumentSymbolSupport"))
        Result.hierarchical_Document_चिह्न = *HierarchicalSupport;
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
  फर्काऊ सत्य;
}

//===----------------------------------------------------------------------===//
// ClientInfo
//===----------------------------------------------------------------------===//

तार्किक llvm::lsp::fromJSON(अचर llvm::json::मान &मान, ClientInfo &Result,
                         llvm::json::Path Path) {
  llvm::json::ObjectMapper O(मान, Path);
  यदि (!O || !O.map("name", Result.name))
    फर्काऊ असत्य;

  // Don't fail if we can't parse version.
  O.map("version", Result.version);
  फर्काऊ सत्य;
}

//===----------------------------------------------------------------------===//
// InitializeParams
//===----------------------------------------------------------------------===//

तार्किक llvm::lsp::fromJSON(अचर llvm::json::मान &मान, TraceLevel &Result,
                         llvm::json::Path Path) {
  यदि (std::optional<StringRef> Str = मान.getAsString()) {
    यदि (*Str == "off") {
      Result = TraceLevel::Off;
      फर्काऊ सत्य;
    }
    यदि (*Str == "messages") {
      Result = TraceLevel::Messages;
      फर्काऊ सत्य;
    }
    यदि (*Str == "verbose") {
      Result = TraceLevel::Verbose;
      फर्काऊ सत्य;
    }
  }
  फर्काऊ असत्य;
}

तार्किक llvm::lsp::fromJSON(अचर llvm::json::मान &मान,
                         InitializeParams &Result, llvm::json::Path Path) {
  llvm::json::ObjectMapper O(मान, Path);
  यदि (!O)
    फर्काऊ असत्य;
  // We deliberately don't fail if we can't parse individual fields.
  O.map("capabilities", Result.capabilities);
  O.map("trace", Result.trace);
  O.map("rootUri", Result.जरा_Uri);
  O.map("rootPath", Result.जरा_Path);
  mapOptOrNull(मान, "clientInfo", Result.clientInfo, Path);

  फर्काऊ सत्य;
}

//===----------------------------------------------------------------------===//
// TextDocumentItem
//===----------------------------------------------------------------------===//

तार्किक llvm::lsp::fromJSON(अचर llvm::json::मान &मान,
                         TextDocumentItem &Result, llvm::json::Path Path) {
  llvm::json::ObjectMapper O(मान, Path);
  फर्काऊ O && O.map("uri", Result.uri) &&
         O.map("languageId", Result.languageId) && O.map("text", Result.text) &&
         O.map("version", Result.version);
}

//===----------------------------------------------------------------------===//
// TextDocumentIdentifier
//===----------------------------------------------------------------------===//

llvm::json::मान llvm::lsp::toJSON(अचर TextDocumentIdentifier &मान) {
  फर्काऊ llvm::json::Object{{"uri", मान.uri}};
}

तार्किक llvm::lsp::fromJSON(अचर llvm::json::मान &मान,
                         TextDocumentIdentifier &Result,
                         llvm::json::Path Path) {
  llvm::json::ObjectMapper O(मान, Path);
  फर्काऊ O && O.map("uri", Result.uri);
}

//===----------------------------------------------------------------------===//
// VersionedTextDocumentIdentifier
//===----------------------------------------------------------------------===//

llvm::json::मान
llvm::lsp::toJSON(अचर VersionedTextDocumentIdentifier &मान) {
  फर्काऊ llvm::json::Object{
      {"uri", मान.uri},
      {"version", मान.version},
  };
}

तार्किक llvm::lsp::fromJSON(अचर llvm::json::मान &मान,
                         VersionedTextDocumentIdentifier &Result,
                         llvm::json::Path Path) {
  llvm::json::ObjectMapper O(मान, Path);
  फर्काऊ O && O.map("uri", Result.uri) && O.map("version", Result.version);
}

//===----------------------------------------------------------------------===//
// Position
//===----------------------------------------------------------------------===//

तार्किक llvm::lsp::fromJSON(अचर llvm::json::मान &मान, Position &Result,
                         llvm::json::Path Path) {
  llvm::json::ObjectMapper O(मान, Path);
  फर्काऊ O && O.map("line", Result.पङ्क्ति_2) &&
         O.map("character", Result.character);
}

llvm::json::मान llvm::lsp::toJSON(अचर Position &मान) {
  फर्काऊ llvm::json::Object{
      {"line", मान.पङ्क्ति_2},
      {"character", मान.character},
  };
}

raw_ostream &llvm::lsp::चालक<<(raw_ostream &Os, अचर Position &मान) {
  फर्काऊ Os << मान.पङ्क्ति_2 << ':' << मान.character;
}

//===----------------------------------------------------------------------===//
// Range
//===----------------------------------------------------------------------===//

तार्किक llvm::lsp::fromJSON(अचर llvm::json::मान &मान, Range &Result,
                         llvm::json::Path Path) {
  llvm::json::ObjectMapper O(मान, Path);
  फर्काऊ O && O.map("start", Result.start) && O.map("end", Result.end);
}

llvm::json::मान llvm::lsp::toJSON(अचर Range &मान) {
  फर्काऊ llvm::json::Object{
      {"start", मान.start},
      {"end", मान.end},
  };
}

raw_ostream &llvm::lsp::चालक<<(raw_ostream &Os, अचर Range &मान) {
  फर्काऊ Os << मान.start << '-' << मान.end;
}

//===----------------------------------------------------------------------===//
// Location
//===----------------------------------------------------------------------===//

तार्किक llvm::lsp::fromJSON(अचर llvm::json::मान &मान, Location &Result,
                         llvm::json::Path Path) {
  llvm::json::ObjectMapper O(मान, Path);
  फर्काऊ O && O.map("uri", Result.uri) && O.map("range", Result.range);
}

llvm::json::मान llvm::lsp::toJSON(अचर Location &मान) {
  फर्काऊ llvm::json::Object{
      {"uri", मान.uri},
      {"range", मान.range},
  };
}

raw_ostream &llvm::lsp::चालक<<(raw_ostream &Os, अचर Location &मान) {
  फर्काऊ Os << मान.range << '@' << मान.uri;
}

//===----------------------------------------------------------------------===//
// TextDocumentPositionParams
//===----------------------------------------------------------------------===//

तार्किक llvm::lsp::fromJSON(अचर llvm::json::मान &मान,
                         TextDocumentPositionParams &Result,
                         llvm::json::Path Path) {
  llvm::json::ObjectMapper O(मान, Path);
  फर्काऊ O && O.map("textDocument", Result.textDocument) &&
         O.map("position", Result.position);
}

//===----------------------------------------------------------------------===//
// ReferenceParams
//===----------------------------------------------------------------------===//

तार्किक llvm::lsp::fromJSON(अचर llvm::json::मान &मान,
                         ReferenceContext &Result, llvm::json::Path Path) {
  llvm::json::ObjectMapper O(मान, Path);
  फर्काऊ O && O.mapOptional("includeDeclaration", Result.includeDeclaration);
}

तार्किक llvm::lsp::fromJSON(अचर llvm::json::मान &मान,
                         ReferenceParams &Result, llvm::json::Path Path) {
  TextDocumentPositionParams &Base = Result;
  llvm::json::ObjectMapper O(मान, Path);
  फर्काऊ fromJSON(मान, Base, Path) && O &&
         O.mapOptional("context", Result.context);
}

//===----------------------------------------------------------------------===//
// DidOpenTextDocumentParams
//===----------------------------------------------------------------------===//

तार्किक llvm::lsp::fromJSON(अचर llvm::json::मान &मान,
                         DidOpenTextDocumentParams &Result,
                         llvm::json::Path Path) {
  llvm::json::ObjectMapper O(मान, Path);
  फर्काऊ O && O.map("textDocument", Result.textDocument);
}

//===----------------------------------------------------------------------===//
// DidCloseTextDocumentParams
//===----------------------------------------------------------------------===//

तार्किक llvm::lsp::fromJSON(अचर llvm::json::मान &मान,
                         DidCloseTextDocumentParams &Result,
                         llvm::json::Path Path) {
  llvm::json::ObjectMapper O(मान, Path);
  फर्काऊ O && O.map("textDocument", Result.textDocument);
}

//===----------------------------------------------------------------------===//
// DidSaveTextDocumentParams
//===----------------------------------------------------------------------===//

तार्किक llvm::lsp::fromJSON(अचर llvm::json::मान &Params,
                         DidSaveTextDocumentParams &R, llvm::json::Path P) {
  llvm::json::ObjectMapper O(Params, P);
  फर्काऊ O && O.map("textDocument", R.textDocument);
}

//===----------------------------------------------------------------------===//
// DidChangeTextDocumentParams
//===----------------------------------------------------------------------===//

LogicalResult
TextDocumentContentChangeEvent::applyTo(std::string &Contents) अचर {
  // If there is no range, the full document changed.
  यदि (!range) {
    Contents = text;
    फर्काऊ success();
  }

  // Try to map the replacement range to the content.
  llvm::SourceMgr TmpScrMgr;
  TmpScrMgr.AddNewSourceBuffer(llvm::MemoryBuffer::getMemBuffer(Contents),
                               SMLoc());
  SMRange RangeLoc = range->getAsSMRange(TmpScrMgr);
  यदि (!RangeLoc.isValid())
    फर्काऊ failure();

  Contents.replace(RangeLoc.Start.getPointer() - Contents.data(),
                   RangeLoc.End.getPointer() - RangeLoc.Start.getPointer(),
                   text);
  फर्काऊ success();
}

LogicalResult TextDocumentContentChangeEvent::applyTo(
    ArrayRef<TextDocumentContentChangeEvent> Changes, std::string &Contents) {
  लागि (अचर स्वतः &Change : Changes)
    यदि (failed(Change.applyTo(Contents)))
      फर्काऊ failure();
  फर्काऊ success();
}

तार्किक llvm::lsp::fromJSON(अचर llvm::json::मान &मान,
                         TextDocumentContentChangeEvent &Result,
                         llvm::json::Path Path) {
  llvm::json::ObjectMapper O(मान, Path);
  फर्काऊ O && O.map("range", Result.range) &&
         O.map("rangeLength", Result.rangeLength) && O.map("text", Result.text);
}

तार्किक llvm::lsp::fromJSON(अचर llvm::json::मान &मान,
                         DidChangeTextDocumentParams &Result,
                         llvm::json::Path Path) {
  llvm::json::ObjectMapper O(मान, Path);
  फर्काऊ O && O.map("textDocument", Result.textDocument) &&
         O.map("contentChanges", Result.contentChanges);
}

//===----------------------------------------------------------------------===//
// MarkupContent
//===----------------------------------------------------------------------===//

स्थिर llvm::StringRef toTextKind(MarkupKind Kind) {
  विकल्प (Kind) {
  पक्ष MarkupKind::PlainText:
    फर्काऊ "plaintext";
  पक्ष MarkupKind::Markdown:
    फर्काऊ "markdown";
  }
  llvm_unreachable("Invalid MarkupKind");
}

raw_ostream &llvm::lsp::चालक<<(raw_ostream &Os, MarkupKind Kind) {
  फर्काऊ Os << toTextKind(Kind);
}

llvm::json::मान llvm::lsp::toJSON(अचर MarkupContent &Mc) {
  यदि (Mc.मान_2.empty())
    फर्काऊ nullptr;

  फर्काऊ llvm::json::Object{
      {"kind", toTextKind(Mc.kind)},
      {"value", Mc.मान_2},
  };
}

//===----------------------------------------------------------------------===//
// Hover
//===----------------------------------------------------------------------===//

llvm::json::मान llvm::lsp::toJSON(अचर Hover &Hover) {
  llvm::json::Object Result{{"contents", toJSON(Hover.contents)}};
  यदि (Hover.range)
    Result["range"] = toJSON(*Hover.range);
  फर्काऊ std::move(Result);
}

//===----------------------------------------------------------------------===//
// DocumentSymbol
//===----------------------------------------------------------------------===//

llvm::json::मान llvm::lsp::toJSON(अचर Document_चिह्न &चिह्न) {
  llvm::json::Object Result{{"name", चिह्न.name},
                            {"kind", static_cast<पूर्णाङ्क>(चिह्न.kind)},
                            {"range", चिह्न.range},
                            {"selectionRange", चिह्न.selectionRange}};

  यदि (!चिह्न.detail.empty())
    Result["detail"] = चिह्न.detail;
  यदि (!चिह्न.children.empty())
    Result["children"] = चिह्न.children;
  फर्काऊ std::move(Result);
}

//===----------------------------------------------------------------------===//
// DocumentSymbolParams
//===----------------------------------------------------------------------===//

तार्किक llvm::lsp::fromJSON(अचर llvm::json::मान &मान,
                         Document_चिह्न_Params &Result, llvm::json::Path Path) {
  llvm::json::ObjectMapper O(मान, Path);
  फर्काऊ O && O.map("textDocument", Result.textDocument);
}

//===----------------------------------------------------------------------===//
// DiagnosticRelatedInformation
//===----------------------------------------------------------------------===//

तार्किक llvm::lsp::fromJSON(अचर llvm::json::मान &मान,
                         DiagnosticRelatedInformation &Result,
                         llvm::json::Path Path) {
  llvm::json::ObjectMapper O(मान, Path);
  फर्काऊ O && O.map("location", Result.location) &&
         O.map("message", Result.message);
}

llvm::json::मान llvm::lsp::toJSON(अचर DiagnosticRelatedInformation &Info) {
  फर्काऊ llvm::json::Object{
      {"location", Info.location},
      {"message", Info.message},
  };
}

//===----------------------------------------------------------------------===//
// Diagnostic
//===----------------------------------------------------------------------===//

llvm::json::मान llvm::lsp::toJSON(DiagnosticTag Tag) {
  फर्काऊ static_cast<पूर्णाङ्क>(Tag);
}

तार्किक llvm::lsp::fromJSON(अचर llvm::json::मान &मान, DiagnosticTag &Result,
                         llvm::json::Path Path) {
  यदि (std::optional<int64_t> I = मान.getAsInteger()) {
    Result = (DiagnosticTag)*I;
    फर्काऊ सत्य;
  }

  फर्काऊ असत्य;
}

llvm::json::मान llvm::lsp::toJSON(अचर Diagnostic &Diag) {
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
  फर्काऊ std::move(Result);
}

तार्किक llvm::lsp::fromJSON(अचर llvm::json::मान &मान, Diagnostic &Result,
                         llvm::json::Path Path) {
  llvm::json::ObjectMapper O(मान, Path);
  यदि (!O)
    फर्काऊ असत्य;
  पूर्णाङ्क Severity = 0;
  यदि (!mapOptOrNull(मान, "severity", Severity, Path))
    फर्काऊ असत्य;
  Result.severity = (DiagnosticSeverity)Severity;

  फर्काऊ O.map("range", Result.range) && O.map("message", Result.message) &&
         mapOptOrNull(मान, "category", Result.category, Path) &&
         mapOptOrNull(मान, "source", Result.source, Path) &&
         mapOptOrNull(मान, "relatedInformation", Result.relatedInformation,
                      Path) &&
         mapOptOrNull(मान, "tags", Result.tags, Path);
}

//===----------------------------------------------------------------------===//
// PublishDiagnosticsParams
//===----------------------------------------------------------------------===//

llvm::json::मान llvm::lsp::toJSON(अचर PublishDiagnosticsParams &Params) {
  फर्काऊ llvm::json::Object{
      {"uri", Params.uri},
      {"diagnostics", Params.diagnostics},
      {"version", Params.version},
  };
}

//===----------------------------------------------------------------------===//
// TextEdit
//===----------------------------------------------------------------------===//

तार्किक llvm::lsp::fromJSON(अचर llvm::json::मान &मान, TextEdit &Result,
                         llvm::json::Path Path) {
  llvm::json::ObjectMapper O(मान, Path);
  फर्काऊ O && O.map("range", Result.range) && O.map("newText", Result.newText);
}

llvm::json::मान llvm::lsp::toJSON(अचर TextEdit &मान) {
  फर्काऊ llvm::json::Object{
      {"range", मान.range},
      {"newText", मान.newText},
  };
}

raw_ostream &llvm::lsp::चालक<<(raw_ostream &Os, अचर TextEdit &मान) {
  Os << मान.range << " => \"";
  llvm::printEscapedString(मान.newText, Os);
  फर्काऊ Os << '"';
}

//===----------------------------------------------------------------------===//
// CompletionItemKind
//===----------------------------------------------------------------------===//

तार्किक llvm::lsp::fromJSON(अचर llvm::json::मान &मान,
                         CompletionItemKind &Result, llvm::json::Path Path) {
  यदि (std::optional<int64_t> Int_मान = मान.getAsInteger()) {
    यदि (*Int_मान < static_cast<पूर्णाङ्क>(CompletionItemKind::Text) ||
        *Int_मान > static_cast<पूर्णाङ्क>(CompletionItemKind::TypeParameter))
      फर्काऊ असत्य;
    Result = static_cast<CompletionItemKind>(*Int_मान);
    फर्काऊ सत्य;
  }
  फर्काऊ असत्य;
}

CompletionItemKind llvm::lsp::adjustKindToCapability(
    CompletionItemKind Kind,
    CompletionItemKindBitset &SupportedCompletionItemKinds) {
  माप_प्रकार KindVal = static_cast<माप_प्रकार>(Kind);
  यदि (KindVal >= kCompletionItemKindMin &&
      KindVal <= SupportedCompletionItemKinds.size() &&
      SupportedCompletionItemKinds[KindVal])
    फर्काऊ Kind;

  // Provide some fall backs for common kinds that are close enough.
  विकल्प (Kind) {
  पक्ष CompletionItemKind::Folder:
    फर्काऊ CompletionItemKind::File;
  पक्ष CompletionItemKind::EnumMember:
    फर्काऊ CompletionItemKind::Enum;
  पक्ष CompletionItemKind::Struct:
    फर्काऊ CompletionItemKind::Class;
  स्वभाव:
    फर्काऊ CompletionItemKind::Text;
  }
}

तार्किक llvm::lsp::fromJSON(अचर llvm::json::मान &मान,
                         CompletionItemKindBitset &Result,
                         llvm::json::Path Path) {
  यदि (अचर llvm::json::Array *Array_मान = मान.getAsArray()) {
    लागि (माप_प्रकार I = 0, E = Array_मान->size(); I < E; ++I) {
      CompletionItemKind KindOut;
      यदि (fromJSON((*Array_मान)[I], KindOut, Path.index(I)))
        Result.set(माप_प्रकार(KindOut));
    }
    फर्काऊ सत्य;
  }
  फर्काऊ असत्य;
}

//===----------------------------------------------------------------------===//
// CompletionItem
//===----------------------------------------------------------------------===//

llvm::json::मान llvm::lsp::toJSON(अचर CompletionItem &मान) {
  प्रतिज्ञा(!मान.label.empty() && "completion item label is required");
  llvm::json::Object Result{{"label", मान.label}};
  यदि (मान.kind != CompletionItemKind::Missing)
    Result["kind"] = static_cast<पूर्णाङ्क>(मान.kind);
  यदि (!मान.detail.empty())
    Result["detail"] = मान.detail;
  यदि (मान.documentation)
    Result["documentation"] = मान.documentation;
  यदि (!मान.sortText.empty())
    Result["sortText"] = मान.sortText;
  यदि (!मान.filterText.empty())
    Result["filterText"] = मान.filterText;
  यदि (!मान.घुसाउ_Text.empty())
    Result["insertText"] = मान.घुसाउ_Text;
  यदि (मान.घुसाउ_Text_Format != घुसाउ_Text_Format_2::Missing)
    Result["insertTextFormat"] = static_cast<पूर्णाङ्क>(मान.घुसाउ_Text_Format);
  यदि (मान.textEdit)
    Result["textEdit"] = *मान.textEdit;
  यदि (!मान.additionalTextEdits.empty()) {
    Result["additionalTextEdits"] =
        llvm::json::Array(मान.additionalTextEdits);
  }
  यदि (मान.deprecated)
    Result["deprecated"] = मान.deprecated;
  फर्काऊ std::move(Result);
}

raw_ostream &llvm::lsp::चालक<<(raw_ostream &Os,
                                   अचर CompletionItem &मान) {
  फर्काऊ Os << मान.label << " - " << toJSON(मान);
}

तार्किक llvm::lsp::चालक<(अचर CompletionItem &Lhs,
                          अचर CompletionItem &Rhs) {
  फर्काऊ (Lhs.sortText.empty() ? Lhs.label : Lhs.sortText) <
         (Rhs.sortText.empty() ? Rhs.label : Rhs.sortText);
}

//===----------------------------------------------------------------------===//
// CompletionList
//===----------------------------------------------------------------------===//

llvm::json::मान llvm::lsp::toJSON(अचर CompletionList &मान) {
  फर्काऊ llvm::json::Object{
      {"isIncomplete", मान.isIncomplete},
      {"items", llvm::json::Array(मान.वस्तुहरू)},
  };
}

//===----------------------------------------------------------------------===//
// CompletionContext
//===----------------------------------------------------------------------===//

तार्किक llvm::lsp::fromJSON(अचर llvm::json::मान &मान,
                         CompletionContext &Result, llvm::json::Path Path) {
  llvm::json::ObjectMapper O(मान, Path);
  पूर्णाङ्क TriggerKind;
  यदि (!O || !O.map("triggerKind", TriggerKind) ||
      !mapOptOrNull(मान, "triggerCharacter", Result.triggerCharacter, Path))
    फर्काऊ असत्य;
  Result.triggerKind = static_cast<CompletionTriggerKind>(TriggerKind);
  फर्काऊ सत्य;
}

//===----------------------------------------------------------------------===//
// CompletionParams
//===----------------------------------------------------------------------===//

तार्किक llvm::lsp::fromJSON(अचर llvm::json::मान &मान,
                         CompletionParams &Result, llvm::json::Path Path) {
  यदि (!fromJSON(मान, static_cast<TextDocumentPositionParams &>(Result), Path))
    फर्काऊ असत्य;
  यदि (अचर llvm::json::मान *Context = मान.getAsObject()->get("context"))
    फर्काऊ fromJSON(*Context, Result.context, Path.field("context"));
  फर्काऊ सत्य;
}

//===----------------------------------------------------------------------===//
// ParameterInformation
//===----------------------------------------------------------------------===//

llvm::json::मान llvm::lsp::toJSON(अचर ParameterInformation &मान) {
  प्रतिज्ञा((मान.labelOffsets || !मान.labelString.empty()) &&
         "parameter information label is required");
  llvm::json::Object Result;
  यदि (मान.labelOffsets)
    Result["label"] = llvm::json::Array(
        {मान.labelOffsets->first, मान.labelOffsets->second});
  अन्यथा
    Result["label"] = मान.labelString;
  यदि (!मान.documentation.empty())
    Result["documentation"] = मान.documentation;
  फर्काऊ std::move(Result);
}

//===----------------------------------------------------------------------===//
// SignatureInformation
//===----------------------------------------------------------------------===//

llvm::json::मान llvm::lsp::toJSON(अचर SignatureInformation &मान) {
  प्रतिज्ञा(!मान.label.empty() && "signature information label is required");
  llvm::json::Object Result{
      {"label", मान.label},
      {"parameters", llvm::json::Array(मान.parameters)},
  };
  यदि (!मान.documentation.empty())
    Result["documentation"] = मान.documentation;
  फर्काऊ std::move(Result);
}

raw_ostream &llvm::lsp::चालक<<(raw_ostream &Os,
                                   अचर SignatureInformation &मान) {
  फर्काऊ Os << मान.label << " - " << toJSON(मान);
}

//===----------------------------------------------------------------------===//
// SignatureHelp
//===----------------------------------------------------------------------===//

llvm::json::मान llvm::lsp::toJSON(अचर SignatureHelp &मान) {
  प्रतिज्ञा(मान.activeSignature >= 0 &&
         "Unexpected negative value for number of active signatures.");
  प्रतिज्ञा(मान.activeParameter >= 0 &&
         "Unexpected negative value for active parameter index");
  फर्काऊ llvm::json::Object{
      {"activeSignature", मान.activeSignature},
      {"activeParameter", मान.activeParameter},
      {"signatures", llvm::json::Array(मान.signatures)},
  };
}

//===----------------------------------------------------------------------===//
// DocumentLinkParams
//===----------------------------------------------------------------------===//

तार्किक llvm::lsp::fromJSON(अचर llvm::json::मान &मान,
                         DocumentLinkParams &Result, llvm::json::Path Path) {
  llvm::json::ObjectMapper O(मान, Path);
  फर्काऊ O && O.map("textDocument", Result.textDocument);
}

//===----------------------------------------------------------------------===//
// DocumentLink
//===----------------------------------------------------------------------===//

llvm::json::मान llvm::lsp::toJSON(अचर DocumentLink &मान) {
  फर्काऊ llvm::json::Object{
      {"range", मान.range},
      {"target", मान.target},
  };
}

//===----------------------------------------------------------------------===//
// InlayHintsParams
//===----------------------------------------------------------------------===//

तार्किक llvm::lsp::fromJSON(अचर llvm::json::मान &मान,
                         InlayHintsParams &Result, llvm::json::Path Path) {
  llvm::json::ObjectMapper O(मान, Path);
  फर्काऊ O && O.map("textDocument", Result.textDocument) &&
         O.map("range", Result.range);
}

//===----------------------------------------------------------------------===//
// InlayHint
//===----------------------------------------------------------------------===//

llvm::json::मान llvm::lsp::toJSON(अचर InlayHint &मान) {
  फर्काऊ llvm::json::Object{{"position", मान.position},
                            {"kind", (पूर्णाङ्क)मान.kind},
                            {"label", मान.label},
                            {"paddingLeft", मान.padding_बायाँ},
                            {"paddingRight", मान.padding_दायाँ}};
}
तार्किक llvm::lsp::चालक==(अचर InlayHint &Lhs, अचर InlayHint &Rhs) {
  फर्काऊ std::tie(Lhs.position, Lhs.kind, Lhs.label) ==
         std::tie(Rhs.position, Rhs.kind, Rhs.label);
}
तार्किक llvm::lsp::चालक<(अचर InlayHint &Lhs, अचर InlayHint &Rhs) {
  फर्काऊ std::tie(Lhs.position, Lhs.kind, Lhs.label) <
         std::tie(Rhs.position, Rhs.kind, Rhs.label);
}

llvm::raw_ostream &llvm::lsp::चालक<<(llvm::raw_ostream &Os,
                                         InlayHintKind मान) {
  विकल्प (मान) {
  पक्ष InlayHintKind::Parameter:
    फर्काऊ Os << "parameter";
  पक्ष InlayHintKind::Type:
    फर्काऊ Os << "type";
  }
  llvm_unreachable("Unknown InlayHintKind");
}

//===----------------------------------------------------------------------===//
// CodeActionContext
//===----------------------------------------------------------------------===//

तार्किक llvm::lsp::fromJSON(अचर llvm::json::मान &मान,
                         CodeActionContext &Result, llvm::json::Path Path) {
  llvm::json::ObjectMapper O(मान, Path);
  यदि (!O || !O.map("diagnostics", Result.diagnostics))
    फर्काऊ असत्य;
  O.map("only", Result.only);
  फर्काऊ सत्य;
}

//===----------------------------------------------------------------------===//
// CodeActionParams
//===----------------------------------------------------------------------===//

तार्किक llvm::lsp::fromJSON(अचर llvm::json::मान &मान,
                         CodeActionParams &Result, llvm::json::Path Path) {
  llvm::json::ObjectMapper O(मान, Path);
  फर्काऊ O && O.map("textDocument", Result.textDocument) &&
         O.map("range", Result.range) && O.map("context", Result.context);
}

//===----------------------------------------------------------------------===//
// WorkspaceEdit
//===----------------------------------------------------------------------===//

तार्किक llvm::lsp::fromJSON(अचर llvm::json::मान &मान, WorkspaceEdit &Result,
                         llvm::json::Path Path) {
  llvm::json::ObjectMapper O(मान, Path);
  फर्काऊ O && O.map("changes", Result.changes);
}

llvm::json::मान llvm::lsp::toJSON(अचर WorkspaceEdit &मान) {
  llvm::json::Object FileChanges;
  लागि (स्वतः &Change : मान.changes)
    FileChanges[Change.first] = llvm::json::Array(Change.second);
  फर्काऊ llvm::json::Object{{"changes", std::move(FileChanges)}};
}

//===----------------------------------------------------------------------===//
// CodeAction
//===----------------------------------------------------------------------===//

अचर llvm::StringLiteral CodeAction::kQuickFix = "quickfix";
अचर llvm::StringLiteral CodeAction::kRefactor = "refactor";
अचर llvm::StringLiteral CodeAction::kInfo = "info";

llvm::json::मान llvm::lsp::toJSON(अचर CodeAction &मान) {
  llvm::json::Object CodeAction{{"title", मान.title}};
  यदि (मान.kind)
    CodeAction["kind"] = *मान.kind;
  यदि (मान.diagnostics)
    CodeAction["diagnostics"] = llvm::json::Array(*मान.diagnostics);
  यदि (मान.isPreferred)
    CodeAction["isPreferred"] = सत्य;
  यदि (मान.edit)
    CodeAction["edit"] = *मान.edit;
  फर्काऊ std::move(CodeAction);
}

//===----------------------------------------------------------------------===//
// ShowMessageParams
//===----------------------------------------------------------------------===//

llvm::json::मान llvm::lsp::toJSON(अचर ShowMessageParams &Params) {
  स्वतः Out = llvm::json::Object{
      {"type", static_cast<पूर्णाङ्क>(Params.type)},
      {"message", Params.message},
  };
  यदि (Params.actions)
    Out["actions"] = *Params.actions;
  फर्काऊ Out;
}

llvm::json::मान llvm::lsp::toJSON(अचर MessageActionItem &Params) {
  फर्काऊ llvm::json::Object{{"title", Params.title}};
}
