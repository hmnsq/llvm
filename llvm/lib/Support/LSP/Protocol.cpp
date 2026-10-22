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

उपयुज्य नामाकाश llvm;
उपयुज्य नामाकाश llvm::lsp;

// Helper that doesn't treat `null` and absent fields as failures.
प्रतिरूपम् <प्रकारनाम T>
स्थिर तार्किक mapOptOrNull(ध्रुव llvm::json::मानम् &Params,
                         llvm::StringLiteral Prop, T &बहिर्गमनम्,
                         llvm::json::मार्गः मार्गः) {
  ध्रुव llvm::json::Object *O = Params.getAsObject();
  प्रतिज्ञा(O);

  // Field is missing or null.
  स्वचालित *V = O->get(Prop);
  यदि (!V || V->getAsNull())
    प्रत्यागम सत्य;
  प्रत्यागम fromJSON(*V, बहिर्गमनम्, मार्गः.क्षेत्रम्(Prop));
}

//===----------------------------------------------------------------------===//
// LSPError
//===----------------------------------------------------------------------===//

वर्ण LSPError::ID;

//===----------------------------------------------------------------------===//
// URIForFile
//===----------------------------------------------------------------------===//

स्थिर तार्किक is_Windows_मार्गः(StringRef मार्गः) {
  प्रत्यागम मार्गः.आकार_2() > 1 && llvm::isAlpha(मार्गः[0]) && मार्गः[1] == ':';
}

स्थिर तार्किक is_जालम्_मार्गः(StringRef मार्गः) {
  प्रत्यागम मार्गः.आकार_2() > 2 && मार्गः[0] == मार्गः[1] &&
         llvm::sys::मार्गः_2::is_separator(मार्गः[0]);
}

स्थिर तार्किक should_Escape_अन्तर्_URI(अचिह्नित वर्ण C) {
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
स्थिर शून्य percentEncode(StringRef Content, std::string &बहिर्गमनम्) {
  प्रति (अचिह्नित वर्ण C : Content) {
    यदि (should_Escape_अन्तर्_URI(C)) {
      बहिर्गमनम्.प्रक्षिप_पृष्ठभागः('%');
      बहिर्गमनम्.प्रक्षिप_पृष्ठभागः(llvm::hexdigit(C / 16));
      बहिर्गमनम्.प्रक्षिप_पृष्ठभागः(llvm::hexdigit(C % 16));
    } अन्यथा {
      बहिर्गमनम्.प्रक्षिप_पृष्ठभागः(C);
    }
  }
}

/// Decodes a string according to percent-encoding.
स्थिर std::string percentDecode(StringRef Content) {
  std::string परिणाम;
  प्रति (स्वचालित I = Content.begin(), E = Content.अन्तः(); I != E; ++I) {
    यदि (*I == '%' && I + 2 < Content.अन्तः() && llvm::isHexDigit(*(I + 1)) &&
        llvm::isHexDigit(*(I + 2))) {
      परिणाम.प्रक्षिप_पृष्ठभागः(llvm::hexFromNibbles(*(I + 1), *(I + 2)));
      I += 2;
    } अन्यथा {
      परिणाम.प्रक्षिप_पृष्ठभागः(*I);
    }
  }
  प्रत्यागम परिणाम;
}

/// Return the set containing the supported URI schemes.
स्थिर String_समुच्चयः<> &getSupportedSchemes() {
  स्थिर String_समुच्चयः<> Schemes({"file", "test"});
  प्रत्यागम Schemes;
}

/// Returns true if the given scheme is structurally valid, i.e. it does not
/// contain any invalid scheme characters. This does not check that the scheme
/// is actually supported.
स्थिर तार्किक isStructurallyValidScheme(StringRef Scheme) {
  यदि (Scheme.रिक्तानि())
    प्रत्यागम असत्य;
  यदि (!llvm::isAlpha(Scheme[0]))
    प्रत्यागम असत्य;
  प्रत्यागम llvm::all_सम्बन्धि(llvm::drop_begin(Scheme), [](वर्ण C) {
    प्रत्यागम llvm::isAlnum(C) || C == '+' || C == '.' || C == '-';
  });
}

स्थिर llvm::Expected<std::string> uri_From_Absolute_मार्गः(StringRef Absolute_मार्गः,
                                                       StringRef Scheme) {
  std::string Body;
  StringRef Authority;
  StringRef मूलम्_2 = llvm::sys::मार्गः_2::मूलम्_name(Absolute_मार्गः);
  यदि (is_जालम्_मार्गः(मूलम्_2)) {
    // Windows UNC paths e.g. \\server\share => file://server/share
    Authority = मूलम्_2.drop_front(2);
    Absolute_मार्गः.consume_front(मूलम्_2);
  } अन्यथा यदि (is_Windows_मार्गः(मूलम्_2)) {
    // Windows paths e.g. X:\path => file:///X:/path
    Body = "/";
  }
  Body += llvm::sys::मार्गः_2::convert_गन्तव्यम्_slash(Absolute_मार्गः);

  std::string Uri = Scheme.str() + ":";
  यदि (Authority.रिक्तानि() && Body.रिक्तानि())
    प्रत्यागम Uri;

  // If authority if empty, we only print body if it starts with "/"; otherwise,
  // the URI is invalid.
  यदि (!Authority.रिक्तानि() || StringRef(Body).starts_with("/")) {
    Uri.append("//");
    percentEncode(Authority, Uri);
  }
  percentEncode(Body, Uri);
  प्रत्यागम Uri;
}

स्थिर llvm::Expected<std::string> get_Absolute_मार्गः(StringRef Authority,
                                                   StringRef Body) {
  यदि (!Body.starts_with("/"))
    प्रत्यागम llvm::createStringError(
        llvm::inconvertible_Error_संहिता(),
        "File scheme: expect body to be an absolute path starting "
        "with '/': " +
            Body);
  SmallString<128> मार्गः;
  यदि (!Authority.रिक्तानि()) {
    // Windows UNC paths e.g. file://server/share => \\server\share
    ("//" + Authority).गन्तव्यम्_सदिशम्(मार्गः);
  } अन्यथा यदि (is_Windows_मार्गः(Body.substr(1))) {
    // Windows paths e.g. file:///X:/path => X:\path
    Body.consume_front("/");
  }
  मार्गः.append(Body);
  llvm::sys::मार्गः_2::native(मार्गः);
  प्रत्यागम std::string(मार्गः);
}

स्थिर llvm::Expected<std::string> parse_File_मार्गः_From_URI(StringRef OrigUri) {
  StringRef Uri = OrigUri;

  // Decode the scheme of the URI.
  size_t Pos = Uri.find(':');
  यदि (Pos == StringRef::npos)
    प्रत्यागम llvm::createStringError(llvm::inconvertible_Error_संहिता(),
                                   "Scheme must be provided in URI: " +
                                       OrigUri);
  StringRef SchemeStr = Uri.substr(0, Pos);
  std::string UriScheme = percentDecode(SchemeStr);
  यदि (!isStructurallyValidScheme(UriScheme))
    प्रत्यागम llvm::createStringError(llvm::inconvertible_Error_संहिता(),
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
  यदि (!getSupportedSchemes().अन्तर्भवति(UriScheme)) {
    प्रत्यागम llvm::createStringError(llvm::inconvertible_Error_संहिता(),
                                   "unsupported URI scheme `" + UriScheme +
                                       "' for workspace files");
  }
  प्रत्यागम get_Absolute_मार्गः(UriAuthority, UriBody);
}

llvm::Expected<URIForFile> URIForFile::fromURI(StringRef Uri) {
  llvm::Expected<std::string> File_मार्गः = parse_File_मार्गः_From_URI(Uri);
  यदि (!File_मार्गः)
    प्रत्यागम File_मार्गः.takeError();
  प्रत्यागम URIForFile(std::move(*File_मार्गः), Uri.str());
}

llvm::Expected<URIForFile> URIForFile::fromFile(StringRef AbsoluteFilepath,
                                                StringRef Scheme) {
  llvm::Expected<std::string> Uri =
      uri_From_Absolute_मार्गः(AbsoluteFilepath, Scheme);
  यदि (!Uri)
    प्रत्यागम Uri.takeError();
  प्रत्यागम fromURI(*Uri);
}

StringRef URIForFile::scheme() ध्रुव { प्रत्यागम uri().विभज(':').first; }

शून्य URIForFile::पञ्जिकाकोशः_Supported_Scheme(StringRef Scheme) {
  getSupportedSchemes().निवेशय(Scheme);
}

तार्किक llvm::lsp::fromJSON(ध्रुव llvm::json::मानम् &मानम्, URIForFile &परिणाम,
                         llvm::json::मार्गः मार्गः) {
  यदि (std::optional<StringRef> Str = मानम्.getAsString()) {
    llvm::Expected<URIForFile> ExpectedUri = URIForFile::fromURI(*Str);
    यदि (!ExpectedUri) {
      मार्गः.report("unresolvable URI");
      consumeError(ExpectedUri.takeError());
      प्रत्यागम असत्य;
    }
    परिणाम = std::move(*ExpectedUri);
    प्रत्यागम सत्य;
  }
  प्रत्यागम असत्य;
}

llvm::json::मानम् llvm::lsp::गन्तव्यम्_JSON(ध्रुव URIForFile &मानम्) {
  प्रत्यागम मानम्.uri();
}

raw_ostream &llvm::lsp::संक्रियकः<<(raw_ostream &Os, ध्रुव URIForFile &मानम्) {
  प्रत्यागम Os << मानम्.uri();
}

//===----------------------------------------------------------------------===//
// ClientCapabilities
//===----------------------------------------------------------------------===//

तार्किक llvm::lsp::fromJSON(ध्रुव llvm::json::मानम् &मानम्,
                         ClientCapabilities &परिणाम, llvm::json::मार्गः मार्गः) {
  ध्रुव llvm::json::Object *O = मानम्.getAsObject();
  यदि (!O) {
    मार्गः.report("expected object");
    प्रत्यागम असत्य;
  }
  यदि (ध्रुव llvm::json::Object *TextDocument = O->getObject("textDocument")) {
    यदि (ध्रुव llvm::json::Object *Document_चिह्नम् =
            TextDocument->getObject("documentSymbol")) {
      यदि (std::optional<तार्किक> श्रेणीबद्धम्_Support =
              Document_चिह्नम्->getBoolean("hierarchicalDocumentSymbolSupport"))
        परिणाम.श्रेणीबद्धम्_Document_चिह्नम् = *श्रेणीबद्धम्_Support;
    }
    यदि (स्वचालित *संहिता_Action = TextDocument->getObject("codeAction")) {
      यदि (संहिता_Action->getObject("codeActionLiteralSupport"))
        परिणाम.संहिता_Action_Structure = सत्य;
    }
  }
  यदि (स्वचालित *गवाक्षः = O->getObject("window")) {
    यदि (std::optional<तार्किक> Work_समाप्तम्_प्रगतिः_Support =
            गवाक्षः->getBoolean("workDoneProgress"))
      परिणाम.work_समाप्तम्_प्रगतिः = *Work_समाप्तम्_प्रगतिः_Support;
  }
  प्रत्यागम सत्य;
}

//===----------------------------------------------------------------------===//
// ClientInfo
//===----------------------------------------------------------------------===//

तार्किक llvm::lsp::fromJSON(ध्रुव llvm::json::मानम् &मानम्, ClientInfo &परिणाम,
                         llvm::json::मार्गः मार्गः) {
  llvm::json::ObjectMapper O(मानम्, मार्गः);
  यदि (!O || !O.map("name", परिणाम.name))
    प्रत्यागम असत्य;

  // Don't fail if we can't parse version.
  O.map("version", परिणाम.संस्करणम्_2);
  प्रत्यागम सत्य;
}

//===----------------------------------------------------------------------===//
// InitializeParams
//===----------------------------------------------------------------------===//

तार्किक llvm::lsp::fromJSON(ध्रुव llvm::json::मानम् &मानम्, TraceLevel &परिणाम,
                         llvm::json::मार्गः मार्गः) {
  यदि (std::optional<StringRef> Str = मानम्.getAsString()) {
    यदि (*Str == "off") {
      परिणाम = TraceLevel::Off;
      प्रत्यागम सत्य;
    }
    यदि (*Str == "messages") {
      परिणाम = TraceLevel::Messages;
      प्रत्यागम सत्य;
    }
    यदि (*Str == "verbose") {
      परिणाम = TraceLevel::Verbose;
      प्रत्यागम सत्य;
    }
  }
  प्रत्यागम असत्य;
}

तार्किक llvm::lsp::fromJSON(ध्रुव llvm::json::मानम् &मानम्,
                         InitializeParams &परिणाम, llvm::json::मार्गः मार्गः) {
  llvm::json::ObjectMapper O(मानम्, मार्गः);
  यदि (!O)
    प्रत्यागम असत्य;
  // We deliberately don't fail if we can't parse individual fields.
  O.map("capabilities", परिणाम.capabilities);
  O.map("trace", परिणाम.trace);
  O.map("rootUri", परिणाम.मूलम्_Uri);
  O.map("rootPath", परिणाम.मूलम्_मार्गः);
  mapOptOrNull(मानम्, "clientInfo", परिणाम.clientInfo, मार्गः);

  प्रत्यागम सत्य;
}

//===----------------------------------------------------------------------===//
// TextDocumentItem
//===----------------------------------------------------------------------===//

तार्किक llvm::lsp::fromJSON(ध्रुव llvm::json::मानम् &मानम्,
                         TextDocumentItem &परिणाम, llvm::json::मार्गः मार्गः) {
  llvm::json::ObjectMapper O(मानम्, मार्गः);
  प्रत्यागम O && O.map("uri", परिणाम.uri) &&
         O.map("languageId", परिणाम.languageId) && O.map("text", परिणाम.text) &&
         O.map("version", परिणाम.संस्करणम्_2);
}

//===----------------------------------------------------------------------===//
// TextDocumentIdentifier
//===----------------------------------------------------------------------===//

llvm::json::मानम् llvm::lsp::गन्तव्यम्_JSON(ध्रुव TextDocumentIdentifier &मानम्) {
  प्रत्यागम llvm::json::Object{{"uri", मानम्.uri}};
}

तार्किक llvm::lsp::fromJSON(ध्रुव llvm::json::मानम् &मानम्,
                         TextDocumentIdentifier &परिणाम,
                         llvm::json::मार्गः मार्गः) {
  llvm::json::ObjectMapper O(मानम्, मार्गः);
  प्रत्यागम O && O.map("uri", परिणाम.uri);
}

//===----------------------------------------------------------------------===//
// VersionedTextDocumentIdentifier
//===----------------------------------------------------------------------===//

llvm::json::मानम्
llvm::lsp::गन्तव्यम्_JSON(ध्रुव VersionedTextDocumentIdentifier &मानम्) {
  प्रत्यागम llvm::json::Object{
      {"uri", मानम्.uri},
      {"version", मानम्.संस्करणम्_2},
  };
}

तार्किक llvm::lsp::fromJSON(ध्रुव llvm::json::मानम् &मानम्,
                         VersionedTextDocumentIdentifier &परिणाम,
                         llvm::json::मार्गः मार्गः) {
  llvm::json::ObjectMapper O(मानम्, मार्गः);
  प्रत्यागम O && O.map("uri", परिणाम.uri) && O.map("version", परिणाम.संस्करणम्_2);
}

//===----------------------------------------------------------------------===//
// Position
//===----------------------------------------------------------------------===//

तार्किक llvm::lsp::fromJSON(ध्रुव llvm::json::मानम् &मानम्, Position &परिणाम,
                         llvm::json::मार्गः मार्गः) {
  llvm::json::ObjectMapper O(मानम्, मार्गः);
  प्रत्यागम O && O.map("line", परिणाम.पङ्क्तिः_2) &&
         O.map("character", परिणाम.character);
}

llvm::json::मानम् llvm::lsp::गन्तव्यम्_JSON(ध्रुव Position &मानम्) {
  प्रत्यागम llvm::json::Object{
      {"line", मानम्.पङ्क्तिः_2},
      {"character", मानम्.character},
  };
}

raw_ostream &llvm::lsp::संक्रियकः<<(raw_ostream &Os, ध्रुव Position &मानम्) {
  प्रत्यागम Os << मानम्.पङ्क्तिः_2 << ':' << मानम्.character;
}

//===----------------------------------------------------------------------===//
// Range
//===----------------------------------------------------------------------===//

तार्किक llvm::lsp::fromJSON(ध्रुव llvm::json::मानम् &मानम्, Range &परिणाम,
                         llvm::json::मार्गः मार्गः) {
  llvm::json::ObjectMapper O(मानम्, मार्गः);
  प्रत्यागम O && O.map("start", परिणाम.start) && O.map("end", परिणाम.अन्तः);
}

llvm::json::मानम् llvm::lsp::गन्तव्यम्_JSON(ध्रुव Range &मानम्) {
  प्रत्यागम llvm::json::Object{
      {"start", मानम्.start},
      {"end", मानम्.अन्तः},
  };
}

raw_ostream &llvm::lsp::संक्रियकः<<(raw_ostream &Os, ध्रुव Range &मानम्) {
  प्रत्यागम Os << मानम्.start << '-' << मानम्.अन्तः;
}

//===----------------------------------------------------------------------===//
// Location
//===----------------------------------------------------------------------===//

तार्किक llvm::lsp::fromJSON(ध्रुव llvm::json::मानम् &मानम्, Location &परिणाम,
                         llvm::json::मार्गः मार्गः) {
  llvm::json::ObjectMapper O(मानम्, मार्गः);
  प्रत्यागम O && O.map("uri", परिणाम.uri) && O.map("range", परिणाम.range);
}

llvm::json::मानम् llvm::lsp::गन्तव्यम्_JSON(ध्रुव Location &मानम्) {
  प्रत्यागम llvm::json::Object{
      {"uri", मानम्.uri},
      {"range", मानम्.range},
  };
}

raw_ostream &llvm::lsp::संक्रियकः<<(raw_ostream &Os, ध्रुव Location &मानम्) {
  प्रत्यागम Os << मानम्.range << '@' << मानम्.uri;
}

//===----------------------------------------------------------------------===//
// TextDocumentPositionParams
//===----------------------------------------------------------------------===//

तार्किक llvm::lsp::fromJSON(ध्रुव llvm::json::मानम् &मानम्,
                         TextDocumentPositionParams &परिणाम,
                         llvm::json::मार्गः मार्गः) {
  llvm::json::ObjectMapper O(मानम्, मार्गः);
  प्रत्यागम O && O.map("textDocument", परिणाम.textDocument) &&
         O.map("position", परिणाम.position);
}

//===----------------------------------------------------------------------===//
// ReferenceParams
//===----------------------------------------------------------------------===//

तार्किक llvm::lsp::fromJSON(ध्रुव llvm::json::मानम् &मानम्,
                         ReferenceContext &परिणाम, llvm::json::मार्गः मार्गः) {
  llvm::json::ObjectMapper O(मानम्, मार्गः);
  प्रत्यागम O && O.mapOptional("includeDeclaration", परिणाम.includeDeclaration);
}

तार्किक llvm::lsp::fromJSON(ध्रुव llvm::json::मानम् &मानम्,
                         ReferenceParams &परिणाम, llvm::json::मार्गः मार्गः) {
  TextDocumentPositionParams &आधारः = परिणाम;
  llvm::json::ObjectMapper O(मानम्, मार्गः);
  प्रत्यागम fromJSON(मानम्, आधारः, मार्गः) && O &&
         O.mapOptional("context", परिणाम.context);
}

//===----------------------------------------------------------------------===//
// DidOpenTextDocumentParams
//===----------------------------------------------------------------------===//

तार्किक llvm::lsp::fromJSON(ध्रुव llvm::json::मानम् &मानम्,
                         DidOpenTextDocumentParams &परिणाम,
                         llvm::json::मार्गः मार्गः) {
  llvm::json::ObjectMapper O(मानम्, मार्गः);
  प्रत्यागम O && O.map("textDocument", परिणाम.textDocument);
}

//===----------------------------------------------------------------------===//
// DidCloseTextDocumentParams
//===----------------------------------------------------------------------===//

तार्किक llvm::lsp::fromJSON(ध्रुव llvm::json::मानम् &मानम्,
                         DidCloseTextDocumentParams &परिणाम,
                         llvm::json::मार्गः मार्गः) {
  llvm::json::ObjectMapper O(मानम्, मार्गः);
  प्रत्यागम O && O.map("textDocument", परिणाम.textDocument);
}

//===----------------------------------------------------------------------===//
// DidSaveTextDocumentParams
//===----------------------------------------------------------------------===//

तार्किक llvm::lsp::fromJSON(ध्रुव llvm::json::मानम् &Params,
                         DidSaveTextDocumentParams &R, llvm::json::मार्गः P) {
  llvm::json::ObjectMapper O(Params, P);
  प्रत्यागम O && O.map("textDocument", R.textDocument);
}

//===----------------------------------------------------------------------===//
// DidChangeTextDocumentParams
//===----------------------------------------------------------------------===//

Logical_परिणाम
TextDocumentContentChangeEvent::apply_गन्तव्यम्(std::string &Contents) ध्रुव {
  // If there is no range, the full document changed.
  यदि (!range) {
    Contents = text;
    प्रत्यागम success();
  }

  // Try to map the replacement range to the content.
  llvm::SourceMgr TmpScrMgr;
  TmpScrMgr.सङ्कलय_New_Source_बफर(llvm::स्मृतिः_बफर::get_Mem_बफर(Contents),
                               SMLoc());
  SMRange RangeLoc = range->getAsSMRange(TmpScrMgr);
  यदि (!RangeLoc.isValid())
    प्रत्यागम failure();

  Contents.replace(RangeLoc.Start.getPointer() - Contents.दत्त(),
                   RangeLoc.अन्तः_2.getPointer() - RangeLoc.Start.getPointer(),
                   text);
  प्रत्यागम success();
}

Logical_परिणाम TextDocumentContentChangeEvent::apply_गन्तव्यम्(
    ArrayRef<TextDocumentContentChangeEvent> Changes, std::string &Contents) {
  प्रति (ध्रुव स्वचालित &Change : Changes)
    यदि (failed(Change.apply_गन्तव्यम्(Contents)))
      प्रत्यागम failure();
  प्रत्यागम success();
}

तार्किक llvm::lsp::fromJSON(ध्रुव llvm::json::मानम् &मानम्,
                         TextDocumentContentChangeEvent &परिणाम,
                         llvm::json::मार्गः मार्गः) {
  llvm::json::ObjectMapper O(मानम्, मार्गः);
  प्रत्यागम O && O.map("range", परिणाम.range) &&
         O.map("rangeLength", परिणाम.range_दीर्घता) && O.map("text", परिणाम.text);
}

तार्किक llvm::lsp::fromJSON(ध्रुव llvm::json::मानम् &मानम्,
                         DidChangeTextDocumentParams &परिणाम,
                         llvm::json::मार्गः मार्गः) {
  llvm::json::ObjectMapper O(मानम्, मार्गः);
  प्रत्यागम O && O.map("textDocument", परिणाम.textDocument) &&
         O.map("contentChanges", परिणाम.contentChanges);
}

//===----------------------------------------------------------------------===//
// MarkupContent
//===----------------------------------------------------------------------===//

स्थिर llvm::StringRef गन्तव्यम्_Text_विधा(Markup_विधा विधा_2) {
  विकल्प (विधा_2) {
  पक्ष Markup_विधा::PlainText:
    प्रत्यागम "plaintext";
  पक्ष Markup_विधा::Markdown:
    प्रत्यागम "markdown";
  }
  llvm_unreachable("Invalid MarkupKind");
}

raw_ostream &llvm::lsp::संक्रियकः<<(raw_ostream &Os, Markup_विधा विधा_2) {
  प्रत्यागम Os << गन्तव्यम्_Text_विधा(विधा_2);
}

llvm::json::मानम् llvm::lsp::गन्तव्यम्_JSON(ध्रुव MarkupContent &Mc) {
  यदि (Mc.मानम्_2.रिक्तानि())
    प्रत्यागम शून्यसूचकः;

  प्रत्यागम llvm::json::Object{
      {"kind", गन्तव्यम्_Text_विधा(Mc.विधा)},
      {"value", Mc.मानम्_2},
  };
}

//===----------------------------------------------------------------------===//
// Hover
//===----------------------------------------------------------------------===//

llvm::json::मानम् llvm::lsp::गन्तव्यम्_JSON(ध्रुव Hover &Hover) {
  llvm::json::Object परिणाम{{"contents", गन्तव्यम्_JSON(Hover.contents)}};
  यदि (Hover.range)
    परिणाम["range"] = गन्तव्यम्_JSON(*Hover.range);
  प्रत्यागम std::move(परिणाम);
}

//===----------------------------------------------------------------------===//
// DocumentSymbol
//===----------------------------------------------------------------------===//

llvm::json::मानम् llvm::lsp::गन्तव्यम्_JSON(ध्रुव Document_चिह्नम् &चिह्नम्) {
  llvm::json::Object परिणाम{{"name", चिह्नम्.name},
                            {"kind", static_cast<पूर्णाङ्क>(चिह्नम्.विधा)},
                            {"range", चिह्नम्.range},
                            {"selectionRange", चिह्नम्.selectionRange}};

  यदि (!चिह्नम्.detail.रिक्तानि())
    परिणाम["detail"] = चिह्नम्.detail;
  यदि (!चिह्नम्.children.रिक्तानि())
    परिणाम["children"] = चिह्नम्.children;
  प्रत्यागम std::move(परिणाम);
}

//===----------------------------------------------------------------------===//
// DocumentSymbolParams
//===----------------------------------------------------------------------===//

तार्किक llvm::lsp::fromJSON(ध्रुव llvm::json::मानम् &मानम्,
                         Document_चिह्नम्_Params &परिणाम, llvm::json::मार्गः मार्गः) {
  llvm::json::ObjectMapper O(मानम्, मार्गः);
  प्रत्यागम O && O.map("textDocument", परिणाम.textDocument);
}

//===----------------------------------------------------------------------===//
// DiagnosticRelatedInformation
//===----------------------------------------------------------------------===//

तार्किक llvm::lsp::fromJSON(ध्रुव llvm::json::मानम् &मानम्,
                         DiagnosticRelatedInformation &परिणाम,
                         llvm::json::मार्गः मार्गः) {
  llvm::json::ObjectMapper O(मानम्, मार्गः);
  प्रत्यागम O && O.map("location", परिणाम.location) &&
         O.map("message", परिणाम.सन्देशः_2);
}

llvm::json::मानम् llvm::lsp::गन्तव्यम्_JSON(ध्रुव DiagnosticRelatedInformation &Info) {
  प्रत्यागम llvm::json::Object{
      {"location", Info.location},
      {"message", Info.सन्देशः_2},
  };
}

//===----------------------------------------------------------------------===//
// Diagnostic
//===----------------------------------------------------------------------===//

llvm::json::मानम् llvm::lsp::गन्तव्यम्_JSON(Diagnostic_चिह्नकम् चिह्नकम्) {
  प्रत्यागम static_cast<पूर्णाङ्क>(चिह्नकम्);
}

तार्किक llvm::lsp::fromJSON(ध्रुव llvm::json::मानम् &मानम्, Diagnostic_चिह्नकम् &परिणाम,
                         llvm::json::मार्गः मार्गः) {
  यदि (std::optional<int64_t> I = मानम्.getAsInteger()) {
    परिणाम = (Diagnostic_चिह्नकम्)*I;
    प्रत्यागम सत्य;
  }

  प्रत्यागम असत्य;
}

llvm::json::मानम् llvm::lsp::गन्तव्यम्_JSON(ध्रुव Diagnostic &Diag) {
  llvm::json::Object परिणाम{
      {"range", Diag.range},
      {"severity", (पूर्णाङ्क)Diag.severity},
      {"message", Diag.सन्देशः_2},
  };
  यदि (Diag.category)
    परिणाम["category"] = *Diag.category;
  यदि (!Diag.source.रिक्तानि())
    परिणाम["source"] = Diag.source;
  यदि (Diag.relatedInformation)
    परिणाम["relatedInformation"] = *Diag.relatedInformation;
  यदि (!Diag.चिह्नकानि.रिक्तानि())
    परिणाम["tags"] = Diag.चिह्नकानि;
  प्रत्यागम std::move(परिणाम);
}

तार्किक llvm::lsp::fromJSON(ध्रुव llvm::json::मानम् &मानम्, Diagnostic &परिणाम,
                         llvm::json::मार्गः मार्गः) {
  llvm::json::ObjectMapper O(मानम्, मार्गः);
  यदि (!O)
    प्रत्यागम असत्य;
  पूर्णाङ्क Severity = 0;
  यदि (!mapOptOrNull(मानम्, "severity", Severity, मार्गः))
    प्रत्यागम असत्य;
  परिणाम.severity = (DiagnosticSeverity)Severity;

  प्रत्यागम O.map("range", परिणाम.range) && O.map("message", परिणाम.सन्देशः_2) &&
         mapOptOrNull(मानम्, "category", परिणाम.category, मार्गः) &&
         mapOptOrNull(मानम्, "source", परिणाम.source, मार्गः) &&
         mapOptOrNull(मानम्, "relatedInformation", परिणाम.relatedInformation,
                      मार्गः) &&
         mapOptOrNull(मानम्, "tags", परिणाम.चिह्नकानि, मार्गः);
}

//===----------------------------------------------------------------------===//
// PublishDiagnosticsParams
//===----------------------------------------------------------------------===//

llvm::json::मानम् llvm::lsp::गन्तव्यम्_JSON(ध्रुव PublishDiagnosticsParams &Params) {
  प्रत्यागम llvm::json::Object{
      {"uri", Params.uri},
      {"diagnostics", Params.diagnostics},
      {"version", Params.संस्करणम्_2},
  };
}

//===----------------------------------------------------------------------===//
// TextEdit
//===----------------------------------------------------------------------===//

तार्किक llvm::lsp::fromJSON(ध्रुव llvm::json::मानम् &मानम्, TextEdit &परिणाम,
                         llvm::json::मार्गः मार्गः) {
  llvm::json::ObjectMapper O(मानम्, मार्गः);
  प्रत्यागम O && O.map("range", परिणाम.range) && O.map("newText", परिणाम.newText);
}

llvm::json::मानम् llvm::lsp::गन्तव्यम्_JSON(ध्रुव TextEdit &मानम्) {
  प्रत्यागम llvm::json::Object{
      {"range", मानम्.range},
      {"newText", मानम्.newText},
  };
}

raw_ostream &llvm::lsp::संक्रियकः<<(raw_ostream &Os, ध्रुव TextEdit &मानम्) {
  Os << मानम्.range << " => \"";
  llvm::printEscapedString(मानम्.newText, Os);
  प्रत्यागम Os << '"';
}

//===----------------------------------------------------------------------===//
// CompletionItemKind
//===----------------------------------------------------------------------===//

तार्किक llvm::lsp::fromJSON(ध्रुव llvm::json::मानम् &मानम्,
                         Completion_Item_विधा &परिणाम, llvm::json::मार्गः मार्गः) {
  यदि (std::optional<int64_t> Int_मानम् = मानम्.getAsInteger()) {
    यदि (*Int_मानम् < static_cast<पूर्णाङ्क>(Completion_Item_विधा::Text) ||
        *Int_मानम् > static_cast<पूर्णाङ्क>(Completion_Item_विधा::TypeParameter))
      प्रत्यागम असत्य;
    परिणाम = static_cast<Completion_Item_विधा>(*Int_मानम्);
    प्रत्यागम सत्य;
  }
  प्रत्यागम असत्य;
}

Completion_Item_विधा llvm::lsp::adjust_विधा_गन्तव्यम्_Capability(
    Completion_Item_विधा विधा_2,
    Completion_Item_विधा_Bitset &SupportedCompletionItemKinds) {
  size_t विधा_Val = static_cast<size_t>(विधा_2);
  यदि (विधा_Val >= k_Completion_Item_विधा_Min &&
      विधा_Val <= SupportedCompletionItemKinds.आकार_2() &&
      SupportedCompletionItemKinds[विधा_Val])
    प्रत्यागम विधा_2;

  // Provide some fall backs for common kinds that are close enough.
  विकल्प (विधा_2) {
  पक्ष Completion_Item_विधा::Folder:
    प्रत्यागम Completion_Item_विधा::File;
  पक्ष Completion_Item_विधा::EnumMember:
    प्रत्यागम Completion_Item_विधा::Enum;
  पक्ष Completion_Item_विधा::Struct:
    प्रत्यागम Completion_Item_विधा::Class;
  स्वभाव:
    प्रत्यागम Completion_Item_विधा::Text;
  }
}

तार्किक llvm::lsp::fromJSON(ध्रुव llvm::json::मानम् &मानम्,
                         Completion_Item_विधा_Bitset &परिणाम,
                         llvm::json::मार्गः मार्गः) {
  यदि (ध्रुव llvm::json::Array *Array_मानम् = मानम्.getAsArray()) {
    प्रति (size_t I = 0, E = Array_मानम्->आकार_2(); I < E; ++I) {
      Completion_Item_विधा विधा_बहिर्गमनम्;
      यदि (fromJSON((*Array_मानम्)[I], विधा_बहिर्गमनम्, मार्गः.सूचकांक(I)))
        परिणाम.समुच्चयः(size_t(विधा_बहिर्गमनम्));
    }
    प्रत्यागम सत्य;
  }
  प्रत्यागम असत्य;
}

//===----------------------------------------------------------------------===//
// CompletionItem
//===----------------------------------------------------------------------===//

llvm::json::मानम् llvm::lsp::गन्तव्यम्_JSON(ध्रुव CompletionItem &मानम्) {
  प्रतिज्ञा(!मानम्.label.रिक्तानि() && "completion item label is required");
  llvm::json::Object परिणाम{{"label", मानम्.label}};
  यदि (मानम्.विधा != Completion_Item_विधा::Missing)
    परिणाम["kind"] = static_cast<पूर्णाङ्क>(मानम्.विधा);
  यदि (!मानम्.detail.रिक्तानि())
    परिणाम["detail"] = मानम्.detail;
  यदि (मानम्.documentation)
    परिणाम["documentation"] = मानम्.documentation;
  यदि (!मानम्.क्रमय_Text.रिक्तानि())
    परिणाम["sortText"] = मानम्.क्रमय_Text;
  यदि (!मानम्.filterText.रिक्तानि())
    परिणाम["filterText"] = मानम्.filterText;
  यदि (!मानम्.निवेशय_Text.रिक्तानि())
    परिणाम["insertText"] = मानम्.निवेशय_Text;
  यदि (मानम्.निवेशय_Text_Format != निवेशय_Text_Format_2::Missing)
    परिणाम["insertTextFormat"] = static_cast<पूर्णाङ्क>(मानम्.निवेशय_Text_Format);
  यदि (मानम्.textEdit)
    परिणाम["textEdit"] = *मानम्.textEdit;
  यदि (!मानम्.additionalTextEdits.रिक्तानि()) {
    परिणाम["additionalTextEdits"] =
        llvm::json::Array(मानम्.additionalTextEdits);
  }
  यदि (मानम्.deprecated)
    परिणाम["deprecated"] = मानम्.deprecated;
  प्रत्यागम std::move(परिणाम);
}

raw_ostream &llvm::lsp::संक्रियकः<<(raw_ostream &Os,
                                   ध्रुव CompletionItem &मानम्) {
  प्रत्यागम Os << मानम्.label << " - " << गन्तव्यम्_JSON(मानम्);
}

तार्किक llvm::lsp::संक्रियकः<(ध्रुव CompletionItem &Lhs,
                          ध्रुव CompletionItem &Rhs) {
  प्रत्यागम (Lhs.क्रमय_Text.रिक्तानि() ? Lhs.label : Lhs.क्रमय_Text) <
         (Rhs.क्रमय_Text.रिक्तानि() ? Rhs.label : Rhs.क्रमय_Text);
}

//===----------------------------------------------------------------------===//
// CompletionList
//===----------------------------------------------------------------------===//

llvm::json::मानम् llvm::lsp::गन्तव्यम्_JSON(ध्रुव CompletionList &मानम्) {
  प्रत्यागम llvm::json::Object{
      {"isIncomplete", मानम्.isIncomplete},
      {"items", llvm::json::Array(मानम्.वस्तूनि)},
  };
}

//===----------------------------------------------------------------------===//
// CompletionContext
//===----------------------------------------------------------------------===//

तार्किक llvm::lsp::fromJSON(ध्रुव llvm::json::मानम् &मानम्,
                         CompletionContext &परिणाम, llvm::json::मार्गः मार्गः) {
  llvm::json::ObjectMapper O(मानम्, मार्गः);
  पूर्णाङ्क Trigger_विधा;
  यदि (!O || !O.map("triggerKind", Trigger_विधा) ||
      !mapOptOrNull(मानम्, "triggerCharacter", परिणाम.triggerCharacter, मार्गः))
    प्रत्यागम असत्य;
  परिणाम.trigger_विधा = static_cast<Completion_Trigger_विधा>(Trigger_विधा);
  प्रत्यागम सत्य;
}

//===----------------------------------------------------------------------===//
// CompletionParams
//===----------------------------------------------------------------------===//

तार्किक llvm::lsp::fromJSON(ध्रुव llvm::json::मानम् &मानम्,
                         CompletionParams &परिणाम, llvm::json::मार्गः मार्गः) {
  यदि (!fromJSON(मानम्, static_cast<TextDocumentPositionParams &>(परिणाम), मार्गः))
    प्रत्यागम असत्य;
  यदि (ध्रुव llvm::json::मानम् *Context = मानम्.getAsObject()->get("context"))
    प्रत्यागम fromJSON(*Context, परिणाम.context, मार्गः.क्षेत्रम्("context"));
  प्रत्यागम सत्य;
}

//===----------------------------------------------------------------------===//
// ParameterInformation
//===----------------------------------------------------------------------===//

llvm::json::मानम् llvm::lsp::गन्तव्यम्_JSON(ध्रुव ParameterInformation &मानम्) {
  प्रतिज्ञा((मानम्.labelOffsets || !मानम्.labelString.रिक्तानि()) &&
         "parameter information label is required");
  llvm::json::Object परिणाम;
  यदि (मानम्.labelOffsets)
    परिणाम["label"] = llvm::json::Array(
        {मानम्.labelOffsets->first, मानम्.labelOffsets->second});
  अन्यथा
    परिणाम["label"] = मानम्.labelString;
  यदि (!मानम्.documentation.रिक्तानि())
    परिणाम["documentation"] = मानम्.documentation;
  प्रत्यागम std::move(परिणाम);
}

//===----------------------------------------------------------------------===//
// SignatureInformation
//===----------------------------------------------------------------------===//

llvm::json::मानम् llvm::lsp::गन्तव्यम्_JSON(ध्रुव SignatureInformation &मानम्) {
  प्रतिज्ञा(!मानम्.label.रिक्तानि() && "signature information label is required");
  llvm::json::Object परिणाम{
      {"label", मानम्.label},
      {"parameters", llvm::json::Array(मानम्.parameters)},
  };
  यदि (!मानम्.documentation.रिक्तानि())
    परिणाम["documentation"] = मानम्.documentation;
  प्रत्यागम std::move(परिणाम);
}

raw_ostream &llvm::lsp::संक्रियकः<<(raw_ostream &Os,
                                   ध्रुव SignatureInformation &मानम्) {
  प्रत्यागम Os << मानम्.label << " - " << गन्तव्यम्_JSON(मानम्);
}

//===----------------------------------------------------------------------===//
// SignatureHelp
//===----------------------------------------------------------------------===//

llvm::json::मानम् llvm::lsp::गन्तव्यम्_JSON(ध्रुव SignatureHelp &मानम्) {
  प्रतिज्ञा(मानम्.सक्रिय_Signature >= 0 &&
         "Unexpected negative value for number of active signatures.");
  प्रतिज्ञा(मानम्.सक्रिय_Parameter >= 0 &&
         "Unexpected negative value for active parameter index");
  प्रत्यागम llvm::json::Object{
      {"activeSignature", मानम्.सक्रिय_Signature},
      {"activeParameter", मानम्.सक्रिय_Parameter},
      {"signatures", llvm::json::Array(मानम्.signatures)},
  };
}

//===----------------------------------------------------------------------===//
// DocumentLinkParams
//===----------------------------------------------------------------------===//

तार्किक llvm::lsp::fromJSON(ध्रुव llvm::json::मानम् &मानम्,
                         DocumentLinkParams &परिणाम, llvm::json::मार्गः मार्गः) {
  llvm::json::ObjectMapper O(मानम्, मार्गः);
  प्रत्यागम O && O.map("textDocument", परिणाम.textDocument);
}

//===----------------------------------------------------------------------===//
// DocumentLink
//===----------------------------------------------------------------------===//

llvm::json::मानम् llvm::lsp::गन्तव्यम्_JSON(ध्रुव DocumentLink &मानम्) {
  प्रत्यागम llvm::json::Object{
      {"range", मानम्.range},
      {"target", मानम्.target},
  };
}

//===----------------------------------------------------------------------===//
// InlayHintsParams
//===----------------------------------------------------------------------===//

तार्किक llvm::lsp::fromJSON(ध्रुव llvm::json::मानम् &मानम्,
                         InlayHintsParams &परिणाम, llvm::json::मार्गः मार्गः) {
  llvm::json::ObjectMapper O(मानम्, मार्गः);
  प्रत्यागम O && O.map("textDocument", परिणाम.textDocument) &&
         O.map("range", परिणाम.range);
}

//===----------------------------------------------------------------------===//
// InlayHint
//===----------------------------------------------------------------------===//

llvm::json::मानम् llvm::lsp::गन्तव्यम्_JSON(ध्रुव InlayHint &मानम्) {
  प्रत्यागम llvm::json::Object{{"position", मानम्.position},
                            {"kind", (पूर्णाङ्क)मानम्.विधा},
                            {"label", मानम्.label},
                            {"paddingLeft", मानम्.padding_वामः},
                            {"paddingRight", मानम्.padding_दक्षिणः}};
}
तार्किक llvm::lsp::संक्रियकः==(ध्रुव InlayHint &Lhs, ध्रुव InlayHint &Rhs) {
  प्रत्यागम std::tie(Lhs.position, Lhs.विधा, Lhs.label) ==
         std::tie(Rhs.position, Rhs.विधा, Rhs.label);
}
तार्किक llvm::lsp::संक्रियकः<(ध्रुव InlayHint &Lhs, ध्रुव InlayHint &Rhs) {
  प्रत्यागम std::tie(Lhs.position, Lhs.विधा, Lhs.label) <
         std::tie(Rhs.position, Rhs.विधा, Rhs.label);
}

llvm::raw_ostream &llvm::lsp::संक्रियकः<<(llvm::raw_ostream &Os,
                                         Inlay_Hint_विधा मानम्) {
  विकल्प (मानम्) {
  पक्ष Inlay_Hint_विधा::Parameter:
    प्रत्यागम Os << "parameter";
  पक्ष Inlay_Hint_विधा::Type:
    प्रत्यागम Os << "type";
  }
  llvm_unreachable("Unknown InlayHintKind");
}

//===----------------------------------------------------------------------===//
// CodeActionContext
//===----------------------------------------------------------------------===//

तार्किक llvm::lsp::fromJSON(ध्रुव llvm::json::मानम् &मानम्,
                         संहिता_Action_Context &परिणाम, llvm::json::मार्गः मार्गः) {
  llvm::json::ObjectMapper O(मानम्, मार्गः);
  यदि (!O || !O.map("diagnostics", परिणाम.diagnostics))
    प्रत्यागम असत्य;
  O.map("only", परिणाम.only);
  प्रत्यागम सत्य;
}

//===----------------------------------------------------------------------===//
// CodeActionParams
//===----------------------------------------------------------------------===//

तार्किक llvm::lsp::fromJSON(ध्रुव llvm::json::मानम् &मानम्,
                         संहिता_Action_Params &परिणाम, llvm::json::मार्गः मार्गः) {
  llvm::json::ObjectMapper O(मानम्, मार्गः);
  प्रत्यागम O && O.map("textDocument", परिणाम.textDocument) &&
         O.map("range", परिणाम.range) && O.map("context", परिणाम.context);
}

//===----------------------------------------------------------------------===//
// WorkspaceEdit
//===----------------------------------------------------------------------===//

तार्किक llvm::lsp::fromJSON(ध्रुव llvm::json::मानम् &मानम्, WorkspaceEdit &परिणाम,
                         llvm::json::मार्गः मार्गः) {
  llvm::json::ObjectMapper O(मानम्, मार्गः);
  प्रत्यागम O && O.map("changes", परिणाम.changes);
}

llvm::json::मानम् llvm::lsp::गन्तव्यम्_JSON(ध्रुव WorkspaceEdit &मानम्) {
  llvm::json::Object FileChanges;
  प्रति (स्वचालित &Change : मानम्.changes)
    FileChanges[Change.first] = llvm::json::Array(Change.second);
  प्रत्यागम llvm::json::Object{{"changes", std::move(FileChanges)}};
}

//===----------------------------------------------------------------------===//
// CodeAction
//===----------------------------------------------------------------------===//

ध्रुव llvm::StringLiteral संहिता_Action::k_शीघ्र_Fix = "quickfix";
ध्रुव llvm::StringLiteral संहिता_Action::kRefactor = "refactor";
ध्रुव llvm::StringLiteral संहिता_Action::kInfo = "info";

llvm::json::मानम् llvm::lsp::गन्तव्यम्_JSON(ध्रुव संहिता_Action &मानम्) {
  llvm::json::Object संहिता_Action{{"title", मानम्.title}};
  यदि (मानम्.विधा)
    संहिता_Action["kind"] = *मानम्.विधा;
  यदि (मानम्.diagnostics)
    संहिता_Action["diagnostics"] = llvm::json::Array(*मानम्.diagnostics);
  यदि (मानम्.isPreferred)
    संहिता_Action["isPreferred"] = सत्य;
  यदि (मानम्.edit)
    संहिता_Action["edit"] = *मानम्.edit;
  प्रत्यागम std::move(संहिता_Action);
}

//===----------------------------------------------------------------------===//
// ShowMessageParams
//===----------------------------------------------------------------------===//

llvm::json::मानम् llvm::lsp::गन्तव्यम्_JSON(ध्रुव Show_सन्देशः_Params &Params) {
  स्वचालित बहिर्गमनम् = llvm::json::Object{
      {"type", static_cast<पूर्णाङ्क>(Params.type)},
      {"message", Params.सन्देशः_2},
  };
  यदि (Params.actions)
    बहिर्गमनम्["actions"] = *Params.actions;
  प्रत्यागम बहिर्गमनम्;
}

llvm::json::मानम् llvm::lsp::गन्तव्यम्_JSON(ध्रुव सन्देशः_Action_Item &Params) {
  प्रत्यागम llvm::json::Object{{"title", Params.title}};
}
