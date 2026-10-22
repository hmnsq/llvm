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

#অন্তর্ভাব "llvm/Support/LSP/Protocol.h"
#অন্তর্ভাব "llvm/ADT/StringExtras.h"
#অন্তর্ভাব "llvm/ADT/StringSet.h"
#অন্তর্ভাব "llvm/Support/ErrorHandling.h"
#অন্তর্ভাব "llvm/Support/JSON.h"
#অন্তর্ভাব "llvm/Support/MemoryBuffer.h"
#অন্তর্ভাব "llvm/Support/Path.h"
#অন্তর্ভাব "llvm/Support/raw_ostream.h"

using নামাকাশ llvm;
using নামাকাশ llvm::lsp;

// Helper that doesn't treat `null` and absent fields as failures.
template <typename T>
স্থির তার্কিক mapOptOrNull(ধ্রুব llvm::json::মান &Params,
                         llvm::StringLiteral Prop, T &Out,
                         llvm::json::Path Path) {
  ধ্রুব llvm::json::Object *O = Params.getAsObject();
  প্রতিজ্ঞা(O);

  // Field is missing or null.
  auto *V = O->get(Prop);
  যদি (!V || V->getAsNull())
    প্রত্যাগম সত্য;
  প্রত্যাগম fromJSON(*V, Out, Path.field(Prop));
}

//===----------------------------------------------------------------------===//
// LSPError
//===----------------------------------------------------------------------===//

বর্ণ LSPError::ID;

//===----------------------------------------------------------------------===//
// URIForFile
//===----------------------------------------------------------------------===//

স্থির তার্কিক isWindowsPath(StringRef Path) {
  প্রত্যাগম Path.size() > 1 && llvm::isAlpha(Path[0]) && Path[1] == ':';
}

স্থির তার্কিক isNetworkPath(StringRef Path) {
  প্রত্যাগম Path.size() > 2 && Path[0] == Path[1] &&
         llvm::sys::path::is_separator(Path[0]);
}

স্থির তার্কিক shouldEscapeInURI(unsigned বর্ণ C) {
  // Unreserved characters.
  যদি ((C >= 'a' && C <= 'z') || (C >= 'A' && C <= 'Z') ||
      (C >= '0' && C <= '9'))
    প্রত্যাগম অসত্য;

  বিকল্প (C) {
  পক্ষ '-':
  পক্ষ '_':
  পক্ষ '.':
  পক্ষ '~':
  // '/' is only reserved when parsing.
  পক্ষ '/':
  // ':' is only reserved for relative URI paths, which we doesn't produce.
  পক্ষ ':':
    প্রত্যাগম অসত্য;
  }
  প্রত্যাগম সত্য;
}

/// Encodes a string according to percent-encoding.
/// - Unreserved characters are not escaped.
/// - Reserved characters always escaped with exceptions like '/'.
/// - All other characters are escaped.
স্থির শূন্য percentEncode(StringRef Content, std::string &Out) {
  প্রতি (unsigned বর্ণ C : Content) {
    যদি (shouldEscapeInURI(C)) {
      Out.ঠেলো_back('%');
      Out.ঠেলো_back(llvm::hexdigit(C / 16));
      Out.ঠেলো_back(llvm::hexdigit(C % 16));
    } অন্যথা {
      Out.ঠেলো_back(C);
    }
  }
}

/// Decodes a string according to percent-encoding.
স্থির std::string percentDecode(StringRef Content) {
  std::string Result;
  প্রতি (auto I = Content.begin(), E = Content.end(); I != E; ++I) {
    যদি (*I == '%' && I + 2 < Content.end() && llvm::isHexDigit(*(I + 1)) &&
        llvm::isHexDigit(*(I + 2))) {
      Result.ঠেলো_back(llvm::hexFromNibbles(*(I + 1), *(I + 2)));
      I += 2;
    } অন্যথা {
      Result.ঠেলো_back(*I);
    }
  }
  প্রত্যাগম Result;
}

/// Return the set containing the supported URI schemes.
স্থির StringSet<> &getSupportedSchemes() {
  স্থির StringSet<> Schemes({"file", "test"});
  প্রত্যাগম Schemes;
}

/// Returns true if the given scheme is structurally valid, i.e. it does not
/// contain any invalid scheme characters. This does not check that the scheme
/// is actually supported.
স্থির তার্কিক isStructurallyValidScheme(StringRef Scheme) {
  যদি (Scheme.empty())
    প্রত্যাগম অসত্য;
  যদি (!llvm::isAlpha(Scheme[0]))
    প্রত্যাগম অসত্য;
  প্রত্যাগম llvm::all_of(llvm::drop_begin(Scheme), [](বর্ণ C) {
    প্রত্যাগম llvm::isAlnum(C) || C == '+' || C == '.' || C == '-';
  });
}

স্থির llvm::Expected<std::string> uriFromAbsolutePath(StringRef AbsolutePath,
                                                       StringRef Scheme) {
  std::string Body;
  StringRef Authority;
  StringRef মূল_2 = llvm::sys::path::মূল_name(AbsolutePath);
  যদি (isNetworkPath(মূল_2)) {
    // Windows UNC paths e.g. \\server\share => file://server/share
    Authority = মূল_2.drop_front(2);
    AbsolutePath.consume_front(মূল_2);
  } অন্যথা যদি (isWindowsPath(মূল_2)) {
    // Windows paths e.g. X:\path => file:///X:/path
    Body = "/";
  }
  Body += llvm::sys::path::convert_to_slash(AbsolutePath);

  std::string Uri = Scheme.str() + ":";
  যদি (Authority.empty() && Body.empty())
    প্রত্যাগম Uri;

  // If authority if empty, we only print body if it starts with "/"; otherwise,
  // the URI is invalid.
  যদি (!Authority.empty() || StringRef(Body).starts_with("/")) {
    Uri.append("//");
    percentEncode(Authority, Uri);
  }
  percentEncode(Body, Uri);
  প্রত্যাগম Uri;
}

স্থির llvm::Expected<std::string> getAbsolutePath(StringRef Authority,
                                                   StringRef Body) {
  যদি (!Body.starts_with("/"))
    প্রত্যাগম llvm::createStringError(
        llvm::inconvertibleErrorCode(),
        "File scheme: expect body to be an absolute path starting "
        "with '/': " +
            Body);
  SmallString<128> Path;
  যদি (!Authority.empty()) {
    // Windows UNC paths e.g. file://server/share => \\server\share
    ("//" + Authority).toVector(Path);
  } অন্যথা যদি (isWindowsPath(Body.substr(1))) {
    // Windows paths e.g. file:///X:/path => X:\path
    Body.consume_front("/");
  }
  Path.append(Body);
  llvm::sys::path::native(Path);
  প্রত্যাগম std::string(Path);
}

স্থির llvm::Expected<std::string> parseFilePathFromURI(StringRef OrigUri) {
  StringRef Uri = OrigUri;

  // Decode the scheme of the URI.
  size_t Pos = Uri.find(':');
  যদি (Pos == StringRef::npos)
    প্রত্যাগম llvm::createStringError(llvm::inconvertibleErrorCode(),
                                   "Scheme must be provided in URI: " +
                                       OrigUri);
  StringRef SchemeStr = Uri.substr(0, Pos);
  std::string UriScheme = percentDecode(SchemeStr);
  যদি (!isStructurallyValidScheme(UriScheme))
    প্রত্যাগম llvm::createStringError(llvm::inconvertibleErrorCode(),
                                   "Invalid scheme: " + SchemeStr +
                                       " (decoded: " + UriScheme + ")");
  Uri = Uri.substr(Pos + 1);

  // Decode the authority of the URI.
  std::string UriAuthority;
  যদি (Uri.consume_front("//")) {
    Pos = Uri.find('/');
    UriAuthority = percentDecode(Uri.substr(0, Pos));
    Uri = Uri.substr(Pos);
  }

  // Decode the body of the URI.
  std::string UriBody = percentDecode(Uri);

  // Compute the absolute path for this uri.
  যদি (!getSupportedSchemes().contains(UriScheme)) {
    প্রত্যাগম llvm::createStringError(llvm::inconvertibleErrorCode(),
                                   "unsupported URI scheme `" + UriScheme +
                                       "' for workspace files");
  }
  প্রত্যাগম getAbsolutePath(UriAuthority, UriBody);
}

llvm::Expected<URIForFile> URIForFile::fromURI(StringRef Uri) {
  llvm::Expected<std::string> FilePath = parseFilePathFromURI(Uri);
  যদি (!FilePath)
    প্রত্যাগম FilePath.takeError();
  প্রত্যাগম URIForFile(std::move(*FilePath), Uri.str());
}

llvm::Expected<URIForFile> URIForFile::fromFile(StringRef AbsoluteFilepath,
                                                StringRef Scheme) {
  llvm::Expected<std::string> Uri =
      uriFromAbsolutePath(AbsoluteFilepath, Scheme);
  যদি (!Uri)
    প্রত্যাগম Uri.takeError();
  প্রত্যাগম fromURI(*Uri);
}

StringRef URIForFile::scheme() ধ্রুব { প্রত্যাগম uri().split(':').first; }

শূন্য URIForFile::registerSupportedScheme(StringRef Scheme) {
  getSupportedSchemes().ঢোকাও(Scheme);
}

তার্কিক llvm::lsp::fromJSON(ধ্রুব llvm::json::মান &মান, URIForFile &Result,
                         llvm::json::Path Path) {
  যদি (std::optional<StringRef> Str = মান.getAsString()) {
    llvm::Expected<URIForFile> ExpectedUri = URIForFile::fromURI(*Str);
    যদি (!ExpectedUri) {
      Path.report("unresolvable URI");
      consumeError(ExpectedUri.takeError());
      প্রত্যাগম অসত্য;
    }
    Result = std::move(*ExpectedUri);
    প্রত্যাগম সত্য;
  }
  প্রত্যাগম অসত্য;
}

llvm::json::মান llvm::lsp::toJSON(ধ্রুব URIForFile &মান) {
  প্রত্যাগম মান.uri();
}

raw_ostream &llvm::lsp::operator<<(raw_ostream &Os, ধ্রুব URIForFile &মান) {
  প্রত্যাগম Os << মান.uri();
}

//===----------------------------------------------------------------------===//
// ClientCapabilities
//===----------------------------------------------------------------------===//

তার্কিক llvm::lsp::fromJSON(ধ্রুব llvm::json::মান &মান,
                         ClientCapabilities &Result, llvm::json::Path Path) {
  ধ্রুব llvm::json::Object *O = মান.getAsObject();
  যদি (!O) {
    Path.report("expected object");
    প্রত্যাগম অসত্য;
  }
  যদি (ধ্রুব llvm::json::Object *TextDocument = O->getObject("textDocument")) {
    যদি (ধ্রুব llvm::json::Object *Document_প্রতীক =
            TextDocument->getObject("documentSymbol")) {
      যদি (std::optional<তার্কিক> HierarchicalSupport =
              Document_প্রতীক->getBoolean("hierarchicalDocumentSymbolSupport"))
        Result.hierarchical_Document_প্রতীক = *HierarchicalSupport;
    }
    যদি (auto *CodeAction = TextDocument->getObject("codeAction")) {
      যদি (CodeAction->getObject("codeActionLiteralSupport"))
        Result.codeActionStructure = সত্য;
    }
  }
  যদি (auto *Window = O->getObject("window")) {
    যদি (std::optional<তার্কিক> WorkDoneProgressSupport =
            Window->getBoolean("workDoneProgress"))
      Result.workDoneProgress = *WorkDoneProgressSupport;
  }
  প্রত্যাগম সত্য;
}

//===----------------------------------------------------------------------===//
// ClientInfo
//===----------------------------------------------------------------------===//

তার্কিক llvm::lsp::fromJSON(ধ্রুব llvm::json::মান &মান, ClientInfo &Result,
                         llvm::json::Path Path) {
  llvm::json::ObjectMapper O(মান, Path);
  যদি (!O || !O.map("name", Result.name))
    প্রত্যাগম অসত্য;

  // Don't fail if we can't parse version.
  O.map("version", Result.version);
  প্রত্যাগম সত্য;
}

//===----------------------------------------------------------------------===//
// InitializeParams
//===----------------------------------------------------------------------===//

তার্কিক llvm::lsp::fromJSON(ধ্রুব llvm::json::মান &মান, TraceLevel &Result,
                         llvm::json::Path Path) {
  যদি (std::optional<StringRef> Str = মান.getAsString()) {
    যদি (*Str == "off") {
      Result = TraceLevel::Off;
      প্রত্যাগম সত্য;
    }
    যদি (*Str == "messages") {
      Result = TraceLevel::Messages;
      প্রত্যাগম সত্য;
    }
    যদি (*Str == "verbose") {
      Result = TraceLevel::Verbose;
      প্রত্যাগম সত্য;
    }
  }
  প্রত্যাগম অসত্য;
}

তার্কিক llvm::lsp::fromJSON(ধ্রুব llvm::json::মান &মান,
                         InitializeParams &Result, llvm::json::Path Path) {
  llvm::json::ObjectMapper O(মান, Path);
  যদি (!O)
    প্রত্যাগম অসত্য;
  // We deliberately don't fail if we can't parse individual fields.
  O.map("capabilities", Result.capabilities);
  O.map("trace", Result.trace);
  O.map("rootUri", Result.মূল_Uri);
  O.map("rootPath", Result.মূল_Path);
  mapOptOrNull(মান, "clientInfo", Result.clientInfo, Path);

  প্রত্যাগম সত্য;
}

//===----------------------------------------------------------------------===//
// TextDocumentItem
//===----------------------------------------------------------------------===//

তার্কিক llvm::lsp::fromJSON(ধ্রুব llvm::json::মান &মান,
                         TextDocumentItem &Result, llvm::json::Path Path) {
  llvm::json::ObjectMapper O(মান, Path);
  প্রত্যাগম O && O.map("uri", Result.uri) &&
         O.map("languageId", Result.languageId) && O.map("text", Result.text) &&
         O.map("version", Result.version);
}

//===----------------------------------------------------------------------===//
// TextDocumentIdentifier
//===----------------------------------------------------------------------===//

llvm::json::মান llvm::lsp::toJSON(ধ্রুব TextDocumentIdentifier &মান) {
  প্রত্যাগম llvm::json::Object{{"uri", মান.uri}};
}

তার্কিক llvm::lsp::fromJSON(ধ্রুব llvm::json::মান &মান,
                         TextDocumentIdentifier &Result,
                         llvm::json::Path Path) {
  llvm::json::ObjectMapper O(মান, Path);
  প্রত্যাগম O && O.map("uri", Result.uri);
}

//===----------------------------------------------------------------------===//
// VersionedTextDocumentIdentifier
//===----------------------------------------------------------------------===//

llvm::json::মান
llvm::lsp::toJSON(ধ্রুব VersionedTextDocumentIdentifier &মান) {
  প্রত্যাগম llvm::json::Object{
      {"uri", মান.uri},
      {"version", মান.version},
  };
}

তার্কিক llvm::lsp::fromJSON(ধ্রুব llvm::json::মান &মান,
                         VersionedTextDocumentIdentifier &Result,
                         llvm::json::Path Path) {
  llvm::json::ObjectMapper O(মান, Path);
  প্রত্যাগম O && O.map("uri", Result.uri) && O.map("version", Result.version);
}

//===----------------------------------------------------------------------===//
// Position
//===----------------------------------------------------------------------===//

তার্কিক llvm::lsp::fromJSON(ধ্রুব llvm::json::মান &মান, Position &Result,
                         llvm::json::Path Path) {
  llvm::json::ObjectMapper O(মান, Path);
  প্রত্যাগম O && O.map("line", Result.সারি_2) &&
         O.map("character", Result.character);
}

llvm::json::মান llvm::lsp::toJSON(ধ্রুব Position &মান) {
  প্রত্যাগম llvm::json::Object{
      {"line", মান.সারি_2},
      {"character", মান.character},
  };
}

raw_ostream &llvm::lsp::operator<<(raw_ostream &Os, ধ্রুব Position &মান) {
  প্রত্যাগম Os << মান.সারি_2 << ':' << মান.character;
}

//===----------------------------------------------------------------------===//
// Range
//===----------------------------------------------------------------------===//

তার্কিক llvm::lsp::fromJSON(ধ্রুব llvm::json::মান &মান, Range &Result,
                         llvm::json::Path Path) {
  llvm::json::ObjectMapper O(মান, Path);
  প্রত্যাগম O && O.map("start", Result.start) && O.map("end", Result.end);
}

llvm::json::মান llvm::lsp::toJSON(ধ্রুব Range &মান) {
  প্রত্যাগম llvm::json::Object{
      {"start", মান.start},
      {"end", মান.end},
  };
}

raw_ostream &llvm::lsp::operator<<(raw_ostream &Os, ধ্রুব Range &মান) {
  প্রত্যাগম Os << মান.start << '-' << মান.end;
}

//===----------------------------------------------------------------------===//
// Location
//===----------------------------------------------------------------------===//

তার্কিক llvm::lsp::fromJSON(ধ্রুব llvm::json::মান &মান, Location &Result,
                         llvm::json::Path Path) {
  llvm::json::ObjectMapper O(মান, Path);
  প্রত্যাগম O && O.map("uri", Result.uri) && O.map("range", Result.range);
}

llvm::json::মান llvm::lsp::toJSON(ধ্রুব Location &মান) {
  প্রত্যাগম llvm::json::Object{
      {"uri", মান.uri},
      {"range", মান.range},
  };
}

raw_ostream &llvm::lsp::operator<<(raw_ostream &Os, ধ্রুব Location &মান) {
  প্রত্যাগম Os << মান.range << '@' << মান.uri;
}

//===----------------------------------------------------------------------===//
// TextDocumentPositionParams
//===----------------------------------------------------------------------===//

তার্কিক llvm::lsp::fromJSON(ধ্রুব llvm::json::মান &মান,
                         TextDocumentPositionParams &Result,
                         llvm::json::Path Path) {
  llvm::json::ObjectMapper O(মান, Path);
  প্রত্যাগম O && O.map("textDocument", Result.textDocument) &&
         O.map("position", Result.position);
}

//===----------------------------------------------------------------------===//
// ReferenceParams
//===----------------------------------------------------------------------===//

তার্কিক llvm::lsp::fromJSON(ধ্রুব llvm::json::মান &মান,
                         ReferenceContext &Result, llvm::json::Path Path) {
  llvm::json::ObjectMapper O(মান, Path);
  প্রত্যাগম O && O.mapOptional("includeDeclaration", Result.includeDeclaration);
}

তার্কিক llvm::lsp::fromJSON(ধ্রুব llvm::json::মান &মান,
                         ReferenceParams &Result, llvm::json::Path Path) {
  TextDocumentPositionParams &Base = Result;
  llvm::json::ObjectMapper O(মান, Path);
  প্রত্যাগম fromJSON(মান, Base, Path) && O &&
         O.mapOptional("context", Result.context);
}

//===----------------------------------------------------------------------===//
// DidOpenTextDocumentParams
//===----------------------------------------------------------------------===//

তার্কিক llvm::lsp::fromJSON(ধ্রুব llvm::json::মান &মান,
                         DidOpenTextDocumentParams &Result,
                         llvm::json::Path Path) {
  llvm::json::ObjectMapper O(মান, Path);
  প্রত্যাগম O && O.map("textDocument", Result.textDocument);
}

//===----------------------------------------------------------------------===//
// DidCloseTextDocumentParams
//===----------------------------------------------------------------------===//

তার্কিক llvm::lsp::fromJSON(ধ্রুব llvm::json::মান &মান,
                         DidCloseTextDocumentParams &Result,
                         llvm::json::Path Path) {
  llvm::json::ObjectMapper O(মান, Path);
  প্রত্যাগম O && O.map("textDocument", Result.textDocument);
}

//===----------------------------------------------------------------------===//
// DidSaveTextDocumentParams
//===----------------------------------------------------------------------===//

তার্কিক llvm::lsp::fromJSON(ধ্রুব llvm::json::মান &Params,
                         DidSaveTextDocumentParams &R, llvm::json::Path P) {
  llvm::json::ObjectMapper O(Params, P);
  প্রত্যাগম O && O.map("textDocument", R.textDocument);
}

//===----------------------------------------------------------------------===//
// DidChangeTextDocumentParams
//===----------------------------------------------------------------------===//

LogicalResult
TextDocumentContentChangeEvent::applyTo(std::string &Contents) ধ্রুব {
  // If there is no range, the full document changed.
  যদি (!range) {
    Contents = text;
    প্রত্যাগম success();
  }

  // Try to map the replacement range to the content.
  llvm::SourceMgr TmpScrMgr;
  TmpScrMgr.AddNewSourceBuffer(llvm::MemoryBuffer::getMemBuffer(Contents),
                               SMLoc());
  SMRange RangeLoc = range->getAsSMRange(TmpScrMgr);
  যদি (!RangeLoc.isValid())
    প্রত্যাগম failure();

  Contents.replace(RangeLoc.Start.getPointer() - Contents.data(),
                   RangeLoc.End.getPointer() - RangeLoc.Start.getPointer(),
                   text);
  প্রত্যাগম success();
}

LogicalResult TextDocumentContentChangeEvent::applyTo(
    ArrayRef<TextDocumentContentChangeEvent> Changes, std::string &Contents) {
  প্রতি (ধ্রুব auto &Change : Changes)
    যদি (failed(Change.applyTo(Contents)))
      প্রত্যাগম failure();
  প্রত্যাগম success();
}

তার্কিক llvm::lsp::fromJSON(ধ্রুব llvm::json::মান &মান,
                         TextDocumentContentChangeEvent &Result,
                         llvm::json::Path Path) {
  llvm::json::ObjectMapper O(মান, Path);
  প্রত্যাগম O && O.map("range", Result.range) &&
         O.map("rangeLength", Result.rangeLength) && O.map("text", Result.text);
}

তার্কিক llvm::lsp::fromJSON(ধ্রুব llvm::json::মান &মান,
                         DidChangeTextDocumentParams &Result,
                         llvm::json::Path Path) {
  llvm::json::ObjectMapper O(মান, Path);
  প্রত্যাগম O && O.map("textDocument", Result.textDocument) &&
         O.map("contentChanges", Result.contentChanges);
}

//===----------------------------------------------------------------------===//
// MarkupContent
//===----------------------------------------------------------------------===//

স্থির llvm::StringRef toTextKind(MarkupKind Kind) {
  বিকল্প (Kind) {
  পক্ষ MarkupKind::PlainText:
    প্রত্যাগম "plaintext";
  পক্ষ MarkupKind::Markdown:
    প্রত্যাগম "markdown";
  }
  llvm_unreachable("Invalid MarkupKind");
}

raw_ostream &llvm::lsp::operator<<(raw_ostream &Os, MarkupKind Kind) {
  প্রত্যাগম Os << toTextKind(Kind);
}

llvm::json::মান llvm::lsp::toJSON(ধ্রুব MarkupContent &Mc) {
  যদি (Mc.মান_2.empty())
    প্রত্যাগম nullptr;

  প্রত্যাগম llvm::json::Object{
      {"kind", toTextKind(Mc.kind)},
      {"value", Mc.মান_2},
  };
}

//===----------------------------------------------------------------------===//
// Hover
//===----------------------------------------------------------------------===//

llvm::json::মান llvm::lsp::toJSON(ধ্রুব Hover &Hover) {
  llvm::json::Object Result{{"contents", toJSON(Hover.contents)}};
  যদি (Hover.range)
    Result["range"] = toJSON(*Hover.range);
  প্রত্যাগম std::move(Result);
}

//===----------------------------------------------------------------------===//
// DocumentSymbol
//===----------------------------------------------------------------------===//

llvm::json::মান llvm::lsp::toJSON(ধ্রুব Document_প্রতীক &প্রতীক) {
  llvm::json::Object Result{{"name", প্রতীক.name},
                            {"kind", static_cast<পূর্ণাঙ্ক>(প্রতীক.kind)},
                            {"range", প্রতীক.range},
                            {"selectionRange", প্রতীক.selectionRange}};

  যদি (!প্রতীক.detail.empty())
    Result["detail"] = প্রতীক.detail;
  যদি (!প্রতীক.children.empty())
    Result["children"] = প্রতীক.children;
  প্রত্যাগম std::move(Result);
}

//===----------------------------------------------------------------------===//
// DocumentSymbolParams
//===----------------------------------------------------------------------===//

তার্কিক llvm::lsp::fromJSON(ধ্রুব llvm::json::মান &মান,
                         Document_প্রতীক_Params &Result, llvm::json::Path Path) {
  llvm::json::ObjectMapper O(মান, Path);
  প্রত্যাগম O && O.map("textDocument", Result.textDocument);
}

//===----------------------------------------------------------------------===//
// DiagnosticRelatedInformation
//===----------------------------------------------------------------------===//

তার্কিক llvm::lsp::fromJSON(ধ্রুব llvm::json::মান &মান,
                         DiagnosticRelatedInformation &Result,
                         llvm::json::Path Path) {
  llvm::json::ObjectMapper O(মান, Path);
  প্রত্যাগম O && O.map("location", Result.location) &&
         O.map("message", Result.message);
}

llvm::json::মান llvm::lsp::toJSON(ধ্রুব DiagnosticRelatedInformation &Info) {
  প্রত্যাগম llvm::json::Object{
      {"location", Info.location},
      {"message", Info.message},
  };
}

//===----------------------------------------------------------------------===//
// Diagnostic
//===----------------------------------------------------------------------===//

llvm::json::মান llvm::lsp::toJSON(DiagnosticTag Tag) {
  প্রত্যাগম static_cast<পূর্ণাঙ্ক>(Tag);
}

তার্কিক llvm::lsp::fromJSON(ধ্রুব llvm::json::মান &মান, DiagnosticTag &Result,
                         llvm::json::Path Path) {
  যদি (std::optional<int64_t> I = মান.getAsInteger()) {
    Result = (DiagnosticTag)*I;
    প্রত্যাগম সত্য;
  }

  প্রত্যাগম অসত্য;
}

llvm::json::মান llvm::lsp::toJSON(ধ্রুব Diagnostic &Diag) {
  llvm::json::Object Result{
      {"range", Diag.range},
      {"severity", (পূর্ণাঙ্ক)Diag.severity},
      {"message", Diag.message},
  };
  যদি (Diag.category)
    Result["category"] = *Diag.category;
  যদি (!Diag.source.empty())
    Result["source"] = Diag.source;
  যদি (Diag.relatedInformation)
    Result["relatedInformation"] = *Diag.relatedInformation;
  যদি (!Diag.tags.empty())
    Result["tags"] = Diag.tags;
  প্রত্যাগম std::move(Result);
}

তার্কিক llvm::lsp::fromJSON(ধ্রুব llvm::json::মান &মান, Diagnostic &Result,
                         llvm::json::Path Path) {
  llvm::json::ObjectMapper O(মান, Path);
  যদি (!O)
    প্রত্যাগম অসত্য;
  পূর্ণাঙ্ক Severity = 0;
  যদি (!mapOptOrNull(মান, "severity", Severity, Path))
    প্রত্যাগম অসত্য;
  Result.severity = (DiagnosticSeverity)Severity;

  প্রত্যাগম O.map("range", Result.range) && O.map("message", Result.message) &&
         mapOptOrNull(মান, "category", Result.category, Path) &&
         mapOptOrNull(মান, "source", Result.source, Path) &&
         mapOptOrNull(মান, "relatedInformation", Result.relatedInformation,
                      Path) &&
         mapOptOrNull(মান, "tags", Result.tags, Path);
}

//===----------------------------------------------------------------------===//
// PublishDiagnosticsParams
//===----------------------------------------------------------------------===//

llvm::json::মান llvm::lsp::toJSON(ধ্রুব PublishDiagnosticsParams &Params) {
  প্রত্যাগম llvm::json::Object{
      {"uri", Params.uri},
      {"diagnostics", Params.diagnostics},
      {"version", Params.version},
  };
}

//===----------------------------------------------------------------------===//
// TextEdit
//===----------------------------------------------------------------------===//

তার্কিক llvm::lsp::fromJSON(ধ্রুব llvm::json::মান &মান, TextEdit &Result,
                         llvm::json::Path Path) {
  llvm::json::ObjectMapper O(মান, Path);
  প্রত্যাগম O && O.map("range", Result.range) && O.map("newText", Result.newText);
}

llvm::json::মান llvm::lsp::toJSON(ধ্রুব TextEdit &মান) {
  প্রত্যাগম llvm::json::Object{
      {"range", মান.range},
      {"newText", মান.newText},
  };
}

raw_ostream &llvm::lsp::operator<<(raw_ostream &Os, ধ্রুব TextEdit &মান) {
  Os << মান.range << " => \"";
  llvm::printEscapedString(মান.newText, Os);
  প্রত্যাগম Os << '"';
}

//===----------------------------------------------------------------------===//
// CompletionItemKind
//===----------------------------------------------------------------------===//

তার্কিক llvm::lsp::fromJSON(ধ্রুব llvm::json::মান &মান,
                         CompletionItemKind &Result, llvm::json::Path Path) {
  যদি (std::optional<int64_t> Int_মান = মান.getAsInteger()) {
    যদি (*Int_মান < static_cast<পূর্ণাঙ্ক>(CompletionItemKind::Text) ||
        *Int_মান > static_cast<পূর্ণাঙ্ক>(CompletionItemKind::TypeParameter))
      প্রত্যাগম অসত্য;
    Result = static_cast<CompletionItemKind>(*Int_মান);
    প্রত্যাগম সত্য;
  }
  প্রত্যাগম অসত্য;
}

CompletionItemKind llvm::lsp::adjustKindToCapability(
    CompletionItemKind Kind,
    CompletionItemKindBitset &SupportedCompletionItemKinds) {
  size_t KindVal = static_cast<size_t>(Kind);
  যদি (KindVal >= kCompletionItemKindMin &&
      KindVal <= SupportedCompletionItemKinds.size() &&
      SupportedCompletionItemKinds[KindVal])
    প্রত্যাগম Kind;

  // Provide some fall backs for common kinds that are close enough.
  বিকল্প (Kind) {
  পক্ষ CompletionItemKind::Folder:
    প্রত্যাগম CompletionItemKind::File;
  পক্ষ CompletionItemKind::EnumMember:
    প্রত্যাগম CompletionItemKind::Enum;
  পক্ষ CompletionItemKind::Struct:
    প্রত্যাগম CompletionItemKind::Class;
  স্বভাব:
    প্রত্যাগম CompletionItemKind::Text;
  }
}

তার্কিক llvm::lsp::fromJSON(ধ্রুব llvm::json::মান &মান,
                         CompletionItemKindBitset &Result,
                         llvm::json::Path Path) {
  যদি (ধ্রুব llvm::json::Array *Array_মান = মান.getAsArray()) {
    প্রতি (size_t I = 0, E = Array_মান->size(); I < E; ++I) {
      CompletionItemKind KindOut;
      যদি (fromJSON((*Array_মান)[I], KindOut, Path.index(I)))
        Result.set(size_t(KindOut));
    }
    প্রত্যাগম সত্য;
  }
  প্রত্যাগম অসত্য;
}

//===----------------------------------------------------------------------===//
// CompletionItem
//===----------------------------------------------------------------------===//

llvm::json::মান llvm::lsp::toJSON(ধ্রুব CompletionItem &মান) {
  প্রতিজ্ঞা(!মান.label.empty() && "completion item label is required");
  llvm::json::Object Result{{"label", মান.label}};
  যদি (মান.kind != CompletionItemKind::Missing)
    Result["kind"] = static_cast<পূর্ণাঙ্ক>(মান.kind);
  যদি (!মান.detail.empty())
    Result["detail"] = মান.detail;
  যদি (মান.documentation)
    Result["documentation"] = মান.documentation;
  যদি (!মান.sortText.empty())
    Result["sortText"] = মান.sortText;
  যদি (!মান.filterText.empty())
    Result["filterText"] = মান.filterText;
  যদি (!মান.ঢোকাও_Text.empty())
    Result["insertText"] = মান.ঢোকাও_Text;
  যদি (মান.ঢোকাও_Text_Format != ঢোকাও_Text_Format_2::Missing)
    Result["insertTextFormat"] = static_cast<পূর্ণাঙ্ক>(মান.ঢোকাও_Text_Format);
  যদি (মান.textEdit)
    Result["textEdit"] = *মান.textEdit;
  যদি (!মান.additionalTextEdits.empty()) {
    Result["additionalTextEdits"] =
        llvm::json::Array(মান.additionalTextEdits);
  }
  যদি (মান.deprecated)
    Result["deprecated"] = মান.deprecated;
  প্রত্যাগম std::move(Result);
}

raw_ostream &llvm::lsp::operator<<(raw_ostream &Os,
                                   ধ্রুব CompletionItem &মান) {
  প্রত্যাগম Os << মান.label << " - " << toJSON(মান);
}

তার্কিক llvm::lsp::operator<(ধ্রুব CompletionItem &Lhs,
                          ধ্রুব CompletionItem &Rhs) {
  প্রত্যাগম (Lhs.sortText.empty() ? Lhs.label : Lhs.sortText) <
         (Rhs.sortText.empty() ? Rhs.label : Rhs.sortText);
}

//===----------------------------------------------------------------------===//
// CompletionList
//===----------------------------------------------------------------------===//

llvm::json::মান llvm::lsp::toJSON(ধ্রুব CompletionList &মান) {
  প্রত্যাগম llvm::json::Object{
      {"isIncomplete", মান.isIncomplete},
      {"items", llvm::json::Array(মান.বস্তুসমূহ)},
  };
}

//===----------------------------------------------------------------------===//
// CompletionContext
//===----------------------------------------------------------------------===//

তার্কিক llvm::lsp::fromJSON(ধ্রুব llvm::json::মান &মান,
                         CompletionContext &Result, llvm::json::Path Path) {
  llvm::json::ObjectMapper O(মান, Path);
  পূর্ণাঙ্ক TriggerKind;
  যদি (!O || !O.map("triggerKind", TriggerKind) ||
      !mapOptOrNull(মান, "triggerCharacter", Result.triggerCharacter, Path))
    প্রত্যাগম অসত্য;
  Result.triggerKind = static_cast<CompletionTriggerKind>(TriggerKind);
  প্রত্যাগম সত্য;
}

//===----------------------------------------------------------------------===//
// CompletionParams
//===----------------------------------------------------------------------===//

তার্কিক llvm::lsp::fromJSON(ধ্রুব llvm::json::মান &মান,
                         CompletionParams &Result, llvm::json::Path Path) {
  যদি (!fromJSON(মান, static_cast<TextDocumentPositionParams &>(Result), Path))
    প্রত্যাগম অসত্য;
  যদি (ধ্রুব llvm::json::মান *Context = মান.getAsObject()->get("context"))
    প্রত্যাগম fromJSON(*Context, Result.context, Path.field("context"));
  প্রত্যাগম সত্য;
}

//===----------------------------------------------------------------------===//
// ParameterInformation
//===----------------------------------------------------------------------===//

llvm::json::মান llvm::lsp::toJSON(ধ্রুব ParameterInformation &মান) {
  প্রতিজ্ঞা((মান.labelOffsets || !মান.labelString.empty()) &&
         "parameter information label is required");
  llvm::json::Object Result;
  যদি (মান.labelOffsets)
    Result["label"] = llvm::json::Array(
        {মান.labelOffsets->first, মান.labelOffsets->second});
  অন্যথা
    Result["label"] = মান.labelString;
  যদি (!মান.documentation.empty())
    Result["documentation"] = মান.documentation;
  প্রত্যাগম std::move(Result);
}

//===----------------------------------------------------------------------===//
// SignatureInformation
//===----------------------------------------------------------------------===//

llvm::json::মান llvm::lsp::toJSON(ধ্রুব SignatureInformation &মান) {
  প্রতিজ্ঞা(!মান.label.empty() && "signature information label is required");
  llvm::json::Object Result{
      {"label", মান.label},
      {"parameters", llvm::json::Array(মান.parameters)},
  };
  যদি (!মান.documentation.empty())
    Result["documentation"] = মান.documentation;
  প্রত্যাগম std::move(Result);
}

raw_ostream &llvm::lsp::operator<<(raw_ostream &Os,
                                   ধ্রুব SignatureInformation &মান) {
  প্রত্যাগম Os << মান.label << " - " << toJSON(মান);
}

//===----------------------------------------------------------------------===//
// SignatureHelp
//===----------------------------------------------------------------------===//

llvm::json::মান llvm::lsp::toJSON(ধ্রুব SignatureHelp &মান) {
  প্রতিজ্ঞা(মান.activeSignature >= 0 &&
         "Unexpected negative value for number of active signatures.");
  প্রতিজ্ঞা(মান.activeParameter >= 0 &&
         "Unexpected negative value for active parameter index");
  প্রত্যাগম llvm::json::Object{
      {"activeSignature", মান.activeSignature},
      {"activeParameter", মান.activeParameter},
      {"signatures", llvm::json::Array(মান.signatures)},
  };
}

//===----------------------------------------------------------------------===//
// DocumentLinkParams
//===----------------------------------------------------------------------===//

তার্কিক llvm::lsp::fromJSON(ধ্রুব llvm::json::মান &মান,
                         DocumentLinkParams &Result, llvm::json::Path Path) {
  llvm::json::ObjectMapper O(মান, Path);
  প্রত্যাগম O && O.map("textDocument", Result.textDocument);
}

//===----------------------------------------------------------------------===//
// DocumentLink
//===----------------------------------------------------------------------===//

llvm::json::মান llvm::lsp::toJSON(ধ্রুব DocumentLink &মান) {
  প্রত্যাগম llvm::json::Object{
      {"range", মান.range},
      {"target", মান.target},
  };
}

//===----------------------------------------------------------------------===//
// InlayHintsParams
//===----------------------------------------------------------------------===//

তার্কিক llvm::lsp::fromJSON(ধ্রুব llvm::json::মান &মান,
                         InlayHintsParams &Result, llvm::json::Path Path) {
  llvm::json::ObjectMapper O(মান, Path);
  প্রত্যাগম O && O.map("textDocument", Result.textDocument) &&
         O.map("range", Result.range);
}

//===----------------------------------------------------------------------===//
// InlayHint
//===----------------------------------------------------------------------===//

llvm::json::মান llvm::lsp::toJSON(ধ্রুব InlayHint &মান) {
  প্রত্যাগম llvm::json::Object{{"position", মান.position},
                            {"kind", (পূর্ণাঙ্ক)মান.kind},
                            {"label", মান.label},
                            {"paddingLeft", মান.padding_বাম},
                            {"paddingRight", মান.padding_ডান}};
}
তার্কিক llvm::lsp::operator==(ধ্রুব InlayHint &Lhs, ধ্রুব InlayHint &Rhs) {
  প্রত্যাগম std::tie(Lhs.position, Lhs.kind, Lhs.label) ==
         std::tie(Rhs.position, Rhs.kind, Rhs.label);
}
তার্কিক llvm::lsp::operator<(ধ্রুব InlayHint &Lhs, ধ্রুব InlayHint &Rhs) {
  প্রত্যাগম std::tie(Lhs.position, Lhs.kind, Lhs.label) <
         std::tie(Rhs.position, Rhs.kind, Rhs.label);
}

llvm::raw_ostream &llvm::lsp::operator<<(llvm::raw_ostream &Os,
                                         InlayHintKind মান) {
  বিকল্প (মান) {
  পক্ষ InlayHintKind::Parameter:
    প্রত্যাগম Os << "parameter";
  পক্ষ InlayHintKind::Type:
    প্রত্যাগম Os << "type";
  }
  llvm_unreachable("Unknown InlayHintKind");
}

//===----------------------------------------------------------------------===//
// CodeActionContext
//===----------------------------------------------------------------------===//

তার্কিক llvm::lsp::fromJSON(ধ্রুব llvm::json::মান &মান,
                         CodeActionContext &Result, llvm::json::Path Path) {
  llvm::json::ObjectMapper O(মান, Path);
  যদি (!O || !O.map("diagnostics", Result.diagnostics))
    প্রত্যাগম অসত্য;
  O.map("only", Result.only);
  প্রত্যাগম সত্য;
}

//===----------------------------------------------------------------------===//
// CodeActionParams
//===----------------------------------------------------------------------===//

তার্কিক llvm::lsp::fromJSON(ধ্রুব llvm::json::মান &মান,
                         CodeActionParams &Result, llvm::json::Path Path) {
  llvm::json::ObjectMapper O(মান, Path);
  প্রত্যাগম O && O.map("textDocument", Result.textDocument) &&
         O.map("range", Result.range) && O.map("context", Result.context);
}

//===----------------------------------------------------------------------===//
// WorkspaceEdit
//===----------------------------------------------------------------------===//

তার্কিক llvm::lsp::fromJSON(ধ্রুব llvm::json::মান &মান, WorkspaceEdit &Result,
                         llvm::json::Path Path) {
  llvm::json::ObjectMapper O(মান, Path);
  প্রত্যাগম O && O.map("changes", Result.changes);
}

llvm::json::মান llvm::lsp::toJSON(ধ্রুব WorkspaceEdit &মান) {
  llvm::json::Object FileChanges;
  প্রতি (auto &Change : মান.changes)
    FileChanges[Change.first] = llvm::json::Array(Change.second);
  প্রত্যাগম llvm::json::Object{{"changes", std::move(FileChanges)}};
}

//===----------------------------------------------------------------------===//
// CodeAction
//===----------------------------------------------------------------------===//

ধ্রুব llvm::StringLiteral CodeAction::kQuickFix = "quickfix";
ধ্রুব llvm::StringLiteral CodeAction::kRefactor = "refactor";
ধ্রুব llvm::StringLiteral CodeAction::kInfo = "info";

llvm::json::মান llvm::lsp::toJSON(ধ্রুব CodeAction &মান) {
  llvm::json::Object CodeAction{{"title", মান.title}};
  যদি (মান.kind)
    CodeAction["kind"] = *মান.kind;
  যদি (মান.diagnostics)
    CodeAction["diagnostics"] = llvm::json::Array(*মান.diagnostics);
  যদি (মান.isPreferred)
    CodeAction["isPreferred"] = সত্য;
  যদি (মান.edit)
    CodeAction["edit"] = *মান.edit;
  প্রত্যাগম std::move(CodeAction);
}

//===----------------------------------------------------------------------===//
// ShowMessageParams
//===----------------------------------------------------------------------===//

llvm::json::মান llvm::lsp::toJSON(ধ্রুব ShowMessageParams &Params) {
  auto Out = llvm::json::Object{
      {"type", static_cast<পূর্ণাঙ্ক>(Params.type)},
      {"message", Params.message},
  };
  যদি (Params.actions)
    Out["actions"] = *Params.actions;
  প্রত্যাগম Out;
}

llvm::json::মান llvm::lsp::toJSON(ধ্রুব MessageActionItem &Params) {
  প্রত্যাগম llvm::json::Object{{"title", Params.title}};
}
