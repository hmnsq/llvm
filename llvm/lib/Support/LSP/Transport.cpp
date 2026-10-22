//===--- JSONTransport.cpp - sending and receiving LSP messages over JSON -===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#समावेश "llvm/Support/LSP/Transport.h"
#समावेश "llvm/ADT/SmallString.h"
#समावेश "llvm/Support/Error.h"
#समावेश "llvm/Support/LSP/Logging.h"
#समावेश "llvm/Support/LSP/Protocol.h"
#समावेश <atomic>
#समावेश <optional>
#समावेश <system_error>
#समावेश <utility>

using namespace llvm;
using namespace llvm::lsp;

//===----------------------------------------------------------------------===//
// Reply
//===----------------------------------------------------------------------===//

namespace {
/// Function object to reply to an LSP call.
/// Each instance must be called exactly once, otherwise:
///  - if there was no reply, an error reply is sent
///  - if there were multiple replies, only the first is sent
श्रेणी Reply {
खुला:
  Reply(स्थिर llvm::json::मूल्य &Id, StringRef Method, JSONTransport &Transport,
        std::mutex &TransportOutputMutex);
  Reply(Reply &&Other);
  Reply &चालक=(Reply &&) = मिटाओ;
  Reply(स्थिर Reply &) = मिटाओ;
  Reply &चालक=(स्थिर Reply &) = मिटाओ;

  व्योम चालक()(llvm::Expected<llvm::json::मूल्य> Reply);

निजी:
  std::string Method;
  std::atomic<bool> Replied = {false};
  llvm::json::मूल्य Id;
  JSONTransport *Transport;
  std::mutex &TransportOutputMutex;
};
} // namespace

Reply::Reply(स्थिर llvm::json::मूल्य &Id, llvm::StringRef Method,
             JSONTransport &Transport, std::mutex &TransportOutputMutex)
    : Method(Method), Id(Id), Transport(&Transport),
      TransportOutputMutex(TransportOutputMutex) {}

Reply::Reply(Reply &&Other)
    : Method(Other.Method), Replied(Other.Replied.load()),
      Id(std::move(Other.Id)), Transport(Other.Transport),
      TransportOutputMutex(Other.TransportOutputMutex) {
  Other.Transport = nullptr;
}

व्योम Reply::चालक()(llvm::Expected<llvm::json::मूल्य> Reply) {
  अगर (Replied.exchange(true)) {
    Logger::error("Replied twice to message {0}({1})", Method, Id);
    निश्चित(false && "must reply to each call only once!");
    वापस;
  }
  निश्चित(Transport && "expected valid transport to reply to");

  std::lock_guard<std::mutex> TransportLock(TransportOutputMutex);
  अगर (Reply) {
    Logger::info("--> reply:{0}({1})", Method, Id);
    Transport->reply(std::move(Id), std::move(Reply));
  } अन्यथा {
    llvm::Error Error = Reply.takeError();
    Logger::info("--> reply:{0}({1}): {2}", Method, Id, Error);
    Transport->reply(std::move(Id), std::move(Error));
  }
}

//===----------------------------------------------------------------------===//
// MessageHandler
//===----------------------------------------------------------------------===//

// Keep handleParseError out of every parse<T> instantiation.
LLVM_ATTRIBUTE_NOINLINE llvm::Error
MessageHandler::handleParseError(स्थिर llvm::json::मूल्य &Raw,
                                 StringRef PayloadName, StringRef PayloadKind,
                                 स्थिर llvm::json::Path::मूल_2 &मूल_2) {
  // Dump the relevant parts of the broken message.
  std::string Context;
  llvm::raw_string_ostream Os(Context);
  मूल_2.printErrorContext(Raw, Os);

  // Report the error (e.g. to the client).
  वापस llvm::make_error<LSPError>(
      llvm::formatv("failed to decode {0} {1}: {2}", PayloadName, PayloadKind,
                    fmt_consume(मूल_2.getError())),
      ErrorCode::InvalidParams);
}

bool MessageHandler::onNotify(llvm::StringRef Method, llvm::json::मूल्य मूल्य) {
  Logger::info("--> {0}", Method);

  अगर (Method == "exit")
    वापस false;
  अगर (Method == "$cancel") {
    // TODO: Add support for cancelling requests.
  } अन्यथा {
    स्वतः It = NotificationHandlers.find(Method);
    अगर (It != NotificationHandlers.end())
      It->second(std::move(मूल्य));
  }
  वापस true;
}

bool MessageHandler::onCall(llvm::StringRef Method, llvm::json::मूल्य Params,
                            llvm::json::मूल्य Id) {
  Logger::info("--> {0}({1})", Method, Id);

  Reply Reply(Id, Method, Transport, TransportOutputMutex);

  स्वतः It = MethodHandlers.find(Method);
  अगर (It != MethodHandlers.end()) {
    It->second(std::move(Params), std::move(Reply));
  } अन्यथा {
    Reply(llvm::make_error<LSPError>("method not found: " + Method.str(),
                                     ErrorCode::MethodNotFound));
  }
  वापस true;
}

bool MessageHandler::onReply(llvm::json::मूल्य Id,
                             llvm::Expected<llvm::json::मूल्य> परिणाम) {
  // Find the response handler in the mapping. If it exists, move it out of the
  // mapping and erase it.
  ResponseHandlerTy ResponseHandler;
  {
    std::lock_guard<std::mutex> responseHandlersLock(ResponseHandlersMutex);
    स्वतः It = ResponseHandlers.find(debugString(Id));
    अगर (It != ResponseHandlers.end()) {
      ResponseHandler = std::move(It->second);
      ResponseHandlers.erase(It);
    }
  }

  // If we found a response handler, invoke it. Otherwise, log an error.
  अगर (ResponseHandler.second) {
    Logger::info("--> reply:{0}({1})", ResponseHandler.first, Id);
    ResponseHandler.second(std::move(Id), std::move(परिणाम));
  } अन्यथा {
    Logger::error(
        "received a reply with ID {0}, but there was no such outgoing request",
        Id);
    अगर (!परिणाम)
      llvm::consumeError(परिणाम.takeError());
  }
  वापस true;
}

//===----------------------------------------------------------------------===//
// JSONTransport
//===----------------------------------------------------------------------===//

/// Encode the given error as a JSON object.
अटल llvm::json::Object encodeError(llvm::Error Error) {
  std::string Message;
  ErrorCode Code = ErrorCode::UnknownErrorCode;
  स्वतः HandlerFn = [&](स्थिर LSPError &LspError) -> llvm::Error {
    Message = LspError.message;
    Code = LspError.code;
    वापस llvm::Error::success();
  };
  अगर (llvm::Error Unhandled = llvm::handleErrors(std::move(Error), HandlerFn))
    Message = llvm::toString(std::move(Unhandled));

  वापस llvm::json::Object{
      {"message", std::move(Message)},
      {"code", int64_t(Code)},
  };
}

/// Decode the given JSON object into an error.
llvm::Error decodeError(स्थिर llvm::json::Object &O) {
  StringRef संदेश_2 = O.getString("message").मूल्य_or("Unspecified error");
  अगर (std::optional<int64_t> Code = O.getInteger("code"))
    वापस llvm::make_error<LSPError>(संदेश_2.str(), ErrorCode(*Code));
  वापस llvm::make_error<llvm::StringError>(llvm::inconvertibleErrorCode(),
                                             संदेश_2.str());
}

व्योम JSONTransport::notify(StringRef Method, llvm::json::मूल्य Params) {
  sendMessage(llvm::json::Object{
      {"jsonrpc", "2.0"},
      {"method", Method},
      {"params", std::move(Params)},
  });
}
व्योम JSONTransport::call(StringRef Method, llvm::json::मूल्य Params,
                         llvm::json::मूल्य Id) {
  sendMessage(llvm::json::Object{
      {"jsonrpc", "2.0"},
      {"id", std::move(Id)},
      {"method", Method},
      {"params", std::move(Params)},
  });
}
व्योम JSONTransport::reply(llvm::json::मूल्य Id,
                          llvm::Expected<llvm::json::मूल्य> परिणाम) {
  अगर (परिणाम) {
    वापस sendMessage(llvm::json::Object{
        {"jsonrpc", "2.0"},
        {"id", std::move(Id)},
        {"result", std::move(*परिणाम)},
    });
  }

  sendMessage(llvm::json::Object{
      {"jsonrpc", "2.0"},
      {"id", std::move(Id)},
      {"error", encodeError(परिणाम.takeError())},
  });
}

llvm::Error JSONTransport::run(MessageHandler &Handler) {
  std::string Json;
  जबतक (!In->is_End_Of_निवेश()) {
    अगर (In->hasError()) {
      वापस llvm::errorCodeToError(
          std::error_code(त्रुटि_सं, std::system_category()));
    }

    अगर (succeeded(In->readMessage(Json))) {
      अगर (llvm::Expected<llvm::json::मूल्य> Doc = llvm::json::parse(Json)) {
        अगर (!handleMessage(std::move(*Doc), Handler))
          वापस llvm::Error::success();
      } अन्यथा {
        Logger::error("JSON parse error: {0}", llvm::toString(Doc.takeError()));
      }
    }
  }
  वापस llvm::errorCodeToError(std::make_error_code(std::errc::io_error));
}

व्योम JSONTransport::sendMessage(llvm::json::मूल्य संदेश_2) {
  Output_बफर.सफा();
  llvm::raw_svector_ostream os(Output_बफर);
  os << llvm::formatv(PrettyOutput ? "{0:2}\n" : "{0}", संदेश_2);
  Out << "Content-Length: " << Output_बफर.आकार_2() << "\r\n\r\n"
      << Output_बफर;
  Out.सफाई();
  Logger::debug(">>> {0}\n", Output_बफर);
}

bool JSONTransport::handleMessage(llvm::json::मूल्य संदेश_2,
                                  MessageHandler &Handler) {
  // Message must be an object with "jsonrpc":"2.0".
  llvm::json::Object *Object = संदेश_2.getAsObject();
  अगर (!Object ||
      Object->getString("jsonrpc") != std::optional<StringRef>("2.0"))
    वापस false;

  // `id` may be any JSON value. If absent, this is a notification.
  std::optional<llvm::json::मूल्य> Id;
  अगर (llvm::json::मूल्य *I = Object->लो("id"))
    Id = std::move(*I);
  std::optional<StringRef> Method = Object->getString("method");

  // This is a response.
  अगर (!Method) {
    अगर (!Id)
      वापस false;
    अगर (स्वतः *Err = Object->getObject("error"))
      वापस Handler.onReply(std::move(*Id), decodeError(*Err));
    // result should be given, use null if not.
    llvm::json::मूल्य परिणाम = nullptr;
    अगर (llvm::json::मूल्य *R = Object->लो("result"))
      परिणाम = std::move(*R);
    वापस Handler.onReply(std::move(*Id), std::move(परिणाम));
  }

  // Params should be given, use null if not.
  llvm::json::मूल्य Params = nullptr;
  अगर (llvm::json::मूल्य *P = Object->लो("params"))
    Params = std::move(*P);

  अगर (Id)
    वापस Handler.onCall(*Method, std::move(Params), std::move(*Id));
  वापस Handler.onNotify(*Method, std::move(Params));
}

/// Tries to read a line up to and including \n.
/// If failing, feof(), ferror(), or shutdownRequested() will be set.
Logical_परिणाम read_पंक्ति(std::खाता *In, SmallVectorImpl<अक्षर> &Out) {
  // Big enough to hold any reasonable header line. May not fit content lines
  // in delimited mode, but performance doesn't matter for that mode.
  अटल constexpr पूर्णांक Buf_आकार = 128;
  माप_प्रकार आकार = 0;
  Out.सफा();
  क्रम (;;) {
    Out.resize_for_overwrite(आकार + Buf_आकार);
    अगर (!std::ख_माला_लो(&Out[आकार], Buf_आकार, In))
      वापस failure();

    साफ_त्रुटि(In);

    // If the line contained null bytes, anything after it (including \n) will
    // be ignored. Fortunately this is not a legal header or JSON.
    माप_प्रकार Read = std::म_माप(&Out[आकार]);
    अगर (Read > 0 && Out[आकार + Read - 1] == '\n') {
      Out.resize(आकार + Read);
      वापस success();
    }
    आकार += Read;
  }
}

// Returns std::nullopt when:
//  - ferror(), feof(), or shutdownRequested() are set.
//  - Content-Length is missing or empty (protocol error)
Logical_परिणाम
JSONTransport_निवेश_Over_File::readStandardMessage(std::string &Json) {
  // A Language Server Protocol message starts with a set of HTTP headers,
  // delimited  by \r\n, and terminated by an empty line (\r\n).
  अचिन्हित दीर्घ दीर्घ Content_लंबाई = 0;
  llvm::SmallString<128> पंक्ति_2;
  जबतक (true) {
    अगर (ख_पूर्ण(In) || hasError() || failed(read_पंक्ति(In, पंक्ति_2)))
      वापस failure();

    // Content-Length is a mandatory header, and the only one we handle.
    StringRef पंक्ति_Ref = पंक्ति_2;
    अगर (पंक्ति_Ref.consume_front("Content-Length: ")) {
      llvm::getAsUnsignedInteger(पंक्ति_Ref.trim(), 0, Content_लंबाई);
    } अन्यथा अगर (!पंक्ति_Ref.trim().empty()) {
      // It's another header, ignore it.
      जारी;
    } अन्यथा {
      // An empty line indicates the end of headers. Go ahead and read the JSON.
      अवरोध;
    }
  }

  // The fuzzer likes crashing us by sending "Content-Length: 9999999999999999"
  अगर (Content_लंबाई == 0 || Content_लंबाई > 1 << 30)
    वापस failure();

  Json.resize(Content_लंबाई);
  क्रम (माप_प्रकार Pos = 0, Read; Pos < Content_लंबाई; Pos += Read) {
    Read = std::ख_पढ़ो(&Json[Pos], 1, Content_लंबाई - Pos, In);
    अगर (Read == 0)
      वापस failure();

    // If we're done, the error was transient. If we're not done, either it was
    // transient or we'll see it again on retry.
    साफ_त्रुटि(In);
    Pos += Read;
  }
  वापस success();
}

/// For lit tests we support a simplified syntax:
/// - messages are delimited by '// -----' on a line by itself
/// - lines starting with // are ignored.
/// This is a testing path, so favor simplicity over performance here.
/// When returning failure: feof(), ferror(), or shutdownRequested() will be
/// set.
Logical_परिणाम
JSONTransport_निवेश_Over_File::readDelimitedMessage(std::string &Json) {
  Json.सफा();
  llvm::SmallString<128> पंक्ति_2;
  जबतक (succeeded(read_पंक्ति(In, पंक्ति_2))) {
    StringRef पंक्ति_Ref = पंक्ति_2.str().trim();
    अगर (पंक्ति_Ref.starts_with("//")) {
      // Found a delimiter for the message.
      अगर (पंक्ति_Ref == "// -----")
        अवरोध;
      जारी;
    }

    Json += पंक्ति_2;
  }

  वापस failure(ख_त्रुटि(In));
}
