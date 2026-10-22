//===--- JSONTransport.cpp - sending and receiving LSP messages over JSON -===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#અન્તર્ભાવ "llvm/Support/LSP/Transport.h"
#અન્તર્ભાવ "llvm/ADT/SmallString.h"
#અન્તર્ભાવ "llvm/Support/Error.h"
#અન્તર્ભાવ "llvm/Support/LSP/Logging.h"
#અન્તર્ભાવ "llvm/Support/LSP/Protocol.h"
#અન્તર્ભાવ <atomic>
#અન્તર્ભાવ <optional>
#અન્તર્ભાવ <system_error>
#અન્તર્ભાવ <utility>

using નામાકાશ llvm;
using નામાકાશ llvm::lsp;

//===----------------------------------------------------------------------===//
// Reply
//===----------------------------------------------------------------------===//

નામાકાશ {
/// Function object to reply to an LSP call.
/// Each instance must be called exactly once, otherwise:
///  - if there was no reply, an error reply is sent
///  - if there were multiple replies, only the first is sent
વર્ગ Reply {
જાહેર:
  Reply(અચલ llvm::json::કિંમત &Id, StringRef Method, JSONTransport &Transport,
        std::mutex &TransportOutputMutex);
  Reply(Reply &&Other);
  Reply &operator=(Reply &&) = delete;
  Reply(અચલ Reply &) = delete;
  Reply &operator=(અચલ Reply &) = delete;

  શૂન્ય operator()(llvm::Expected<llvm::json::કિંમત> Reply);

ખાનગી:
  std::string Method;
  std::atomic<તાર્કિક> Replied = {ખોટું};
  llvm::json::કિંમત Id;
  JSONTransport *Transport;
  std::mutex &TransportOutputMutex;
};
} // namespace

Reply::Reply(અચલ llvm::json::કિંમત &Id, llvm::StringRef Method,
             JSONTransport &Transport, std::mutex &TransportOutputMutex)
    : Method(Method), Id(Id), Transport(&Transport),
      TransportOutputMutex(TransportOutputMutex) {}

Reply::Reply(Reply &&Other)
    : Method(Other.Method), Replied(Other.Replied.load()),
      Id(std::move(Other.Id)), Transport(Other.Transport),
      TransportOutputMutex(Other.TransportOutputMutex) {
  Other.Transport = nullptr;
}

શૂન્ય Reply::operator()(llvm::Expected<llvm::json::કિંમત> Reply) {
  જો (Replied.exchange(સાચું)) {
    Logger::error("Replied twice to message {0}({1})", Method, Id);
    પ્રતિજ્ઞા(ખોટું && "must reply to each call only once!");
    પાછા;
  }
  પ્રતિજ્ઞા(Transport && "expected valid transport to reply to");

  std::lock_guard<std::mutex> TransportLock(TransportOutputMutex);
  જો (Reply) {
    Logger::info("--> reply:{0}({1})", Method, Id);
    Transport->reply(std::move(Id), std::move(Reply));
  } નહીંતર {
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
MessageHandler::handleParseError(અચલ llvm::json::કિંમત &Raw,
                                 StringRef PayloadName, StringRef PayloadKind,
                                 અચલ llvm::json::Path::મૂળ_2 &મૂળ_2) {
  // Dump the relevant parts of the broken message.
  std::string Context;
  llvm::raw_string_ostream Os(Context);
  મૂળ_2.printErrorContext(Raw, Os);

  // Report the error (e.g. to the client).
  પાછા llvm::make_error<LSPError>(
      llvm::formatv("failed to decode {0} {1}: {2}", PayloadName, PayloadKind,
                    fmt_consume(મૂળ_2.getError())),
      ErrorCode::InvalidParams);
}

તાર્કિક MessageHandler::onNotify(llvm::StringRef Method, llvm::json::કિંમત કિંમત) {
  Logger::info("--> {0}", Method);

  જો (Method == "exit")
    પાછા ખોટું;
  જો (Method == "$cancel") {
    // TODO: Add support for cancelling requests.
  } નહીંતર {
    auto It = NotificationHandlers.find(Method);
    જો (It != NotificationHandlers.end())
      It->second(std::move(કિંમત));
  }
  પાછા સાચું;
}

તાર્કિક MessageHandler::onCall(llvm::StringRef Method, llvm::json::કિંમત Params,
                            llvm::json::કિંમત Id) {
  Logger::info("--> {0}({1})", Method, Id);

  Reply Reply(Id, Method, Transport, TransportOutputMutex);

  auto It = MethodHandlers.find(Method);
  જો (It != MethodHandlers.end()) {
    It->second(std::move(Params), std::move(Reply));
  } નહીંતર {
    Reply(llvm::make_error<LSPError>("method not found: " + Method.str(),
                                     ErrorCode::MethodNotFound));
  }
  પાછા સાચું;
}

તાર્કિક MessageHandler::onReply(llvm::json::કિંમત Id,
                             llvm::Expected<llvm::json::કિંમત> Result) {
  // Find the response handler in the mapping. If it exists, move it out of the
  // mapping and erase it.
  ResponseHandlerTy ResponseHandler;
  {
    std::lock_guard<std::mutex> responseHandlersLock(ResponseHandlersMutex);
    auto It = ResponseHandlers.find(debugString(Id));
    જો (It != ResponseHandlers.end()) {
      ResponseHandler = std::move(It->second);
      ResponseHandlers.erase(It);
    }
  }

  // If we found a response handler, invoke it. Otherwise, log an error.
  જો (ResponseHandler.second) {
    Logger::info("--> reply:{0}({1})", ResponseHandler.first, Id);
    ResponseHandler.second(std::move(Id), std::move(Result));
  } નહીંતર {
    Logger::error(
        "received a reply with ID {0}, but there was no such outgoing request",
        Id);
    જો (!Result)
      llvm::consumeError(Result.takeError());
  }
  પાછા સાચું;
}

//===----------------------------------------------------------------------===//
// JSONTransport
//===----------------------------------------------------------------------===//

/// Encode the given error as a JSON object.
સ્થિર llvm::json::Object encodeError(llvm::Error Error) {
  std::string Message;
  ErrorCode Code = ErrorCode::UnknownErrorCode;
  auto HandlerFn = [&](અચલ LSPError &LspError) -> llvm::Error {
    Message = LspError.message;
    Code = LspError.code;
    પાછા llvm::Error::success();
  };
  જો (llvm::Error Unhandled = llvm::handleErrors(std::move(Error), HandlerFn))
    Message = llvm::toString(std::move(Unhandled));

  પાછા llvm::json::Object{
      {"message", std::move(Message)},
      {"code", int64_t(Code)},
  };
}

/// Decode the given JSON object into an error.
llvm::Error decodeError(અચલ llvm::json::Object &O) {
  StringRef સંદેશ_2 = O.getString("message").કિંમત_or("Unspecified error");
  જો (std::optional<int64_t> Code = O.getInteger("code"))
    પાછા llvm::make_error<LSPError>(સંદેશ_2.str(), ErrorCode(*Code));
  પાછા llvm::make_error<llvm::StringError>(llvm::inconvertibleErrorCode(),
                                             સંદેશ_2.str());
}

શૂન્ય JSONTransport::notify(StringRef Method, llvm::json::કિંમત Params) {
  sendMessage(llvm::json::Object{
      {"jsonrpc", "2.0"},
      {"method", Method},
      {"params", std::move(Params)},
  });
}
શૂન્ય JSONTransport::call(StringRef Method, llvm::json::કિંમત Params,
                         llvm::json::કિંમત Id) {
  sendMessage(llvm::json::Object{
      {"jsonrpc", "2.0"},
      {"id", std::move(Id)},
      {"method", Method},
      {"params", std::move(Params)},
  });
}
શૂન્ય JSONTransport::reply(llvm::json::કિંમત Id,
                          llvm::Expected<llvm::json::કિંમત> Result) {
  જો (Result) {
    પાછા sendMessage(llvm::json::Object{
        {"jsonrpc", "2.0"},
        {"id", std::move(Id)},
        {"result", std::move(*Result)},
    });
  }

  sendMessage(llvm::json::Object{
      {"jsonrpc", "2.0"},
      {"id", std::move(Id)},
      {"error", encodeError(Result.takeError())},
  });
}

llvm::Error JSONTransport::run(MessageHandler &Handler) {
  std::string Json;
  જ્યારે (!In->is_End_Of_આગત()) {
    જો (In->hasError()) {
      પાછા llvm::errorCodeToError(
          std::error_code(errno, std::system_category()));
    }

    જો (succeeded(In->readMessage(Json))) {
      જો (llvm::Expected<llvm::json::કિંમત> Doc = llvm::json::parse(Json)) {
        જો (!handleMessage(std::move(*Doc), Handler))
          પાછા llvm::Error::success();
      } નહીંતર {
        Logger::error("JSON parse error: {0}", llvm::toString(Doc.takeError()));
      }
    }
  }
  પાછા llvm::errorCodeToError(std::make_error_code(std::errc::io_error));
}

શૂન્ય JSONTransport::sendMessage(llvm::json::કિંમત સંદેશ_2) {
  OutputBuffer.clear();
  llvm::raw_svector_ostream os(OutputBuffer);
  os << llvm::formatv(PrettyOutput ? "{0:2}\n" : "{0}", સંદેશ_2);
  Out << "Content-Length: " << OutputBuffer.size() << "\r\n\r\n"
      << OutputBuffer;
  Out.flush();
  Logger::debug(">>> {0}\n", OutputBuffer);
}

તાર્કિક JSONTransport::handleMessage(llvm::json::કિંમત સંદેશ_2,
                                  MessageHandler &Handler) {
  // Message must be an object with "jsonrpc":"2.0".
  llvm::json::Object *Object = સંદેશ_2.getAsObject();
  જો (!Object ||
      Object->getString("jsonrpc") != std::optional<StringRef>("2.0"))
    પાછા ખોટું;

  // `id` may be any JSON value. If absent, this is a notification.
  std::optional<llvm::json::કિંમત> Id;
  જો (llvm::json::કિંમત *I = Object->get("id"))
    Id = std::move(*I);
  std::optional<StringRef> Method = Object->getString("method");

  // This is a response.
  જો (!Method) {
    જો (!Id)
      પાછા ખોટું;
    જો (auto *Err = Object->getObject("error"))
      પાછા Handler.onReply(std::move(*Id), decodeError(*Err));
    // result should be given, use null if not.
    llvm::json::કિંમત Result = nullptr;
    જો (llvm::json::કિંમત *R = Object->get("result"))
      Result = std::move(*R);
    પાછા Handler.onReply(std::move(*Id), std::move(Result));
  }

  // Params should be given, use null if not.
  llvm::json::કિંમત Params = nullptr;
  જો (llvm::json::કિંમત *P = Object->get("params"))
    Params = std::move(*P);

  જો (Id)
    પાછા Handler.onCall(*Method, std::move(Params), std::move(*Id));
  પાછા Handler.onNotify(*Method, std::move(Params));
}

/// Tries to read a line up to and including \n.
/// If failing, feof(), ferror(), or shutdownRequested() will be set.
LogicalResult read_લીટી(std::FILE *In, SmallVectorImpl<અક્ષર> &Out) {
  // Big enough to hold any reasonable header line. May not fit content lines
  // in delimited mode, but performance doesn't matter for that mode.
  સ્થિર constexpr પૂર્ણાંક BufSize = 128;
  size_t Size = 0;
  Out.clear();
  માટે (;;) {
    Out.resize_for_overwrite(Size + BufSize);
    જો (!std::fgets(&Out[Size], BufSize, In))
      પાછા failure();

    clearerr(In);

    // If the line contained null bytes, anything after it (including \n) will
    // be ignored. Fortunately this is not a legal header or JSON.
    size_t Read = std::strlen(&Out[Size]);
    જો (Read > 0 && Out[Size + Read - 1] == '\n') {
      Out.resize(Size + Read);
      પાછા success();
    }
    Size += Read;
  }
}

// Returns std::nullopt when:
//  - ferror(), feof(), or shutdownRequested() are set.
//  - Content-Length is missing or empty (protocol error)
LogicalResult
JSONTransport_આગત_Over_File::readStandardMessage(std::string &Json) {
  // A Language Server Protocol message starts with a set of HTTP headers,
  // delimited  by \r\n, and terminated by an empty line (\r\n).
  unsigned long long ContentLength = 0;
  llvm::SmallString<128> લીટી;
  જ્યારે (સાચું) {
    જો (feof(In) || hasError() || failed(read_લીટી(In, લીટી)))
      પાછા failure();

    // Content-Length is a mandatory header, and the only one we handle.
    StringRef લીટી_Ref = લીટી;
    જો (લીટી_Ref.consume_front("Content-Length: ")) {
      llvm::getAsUnsignedInteger(લીટી_Ref.trim(), 0, ContentLength);
    } નહીંતર જો (!લીટી_Ref.trim().empty()) {
      // It's another header, ignore it.
      ચાલુ;
    } નહીંતર {
      // An empty line indicates the end of headers. Go ahead and read the JSON.
      તોડો;
    }
  }

  // The fuzzer likes crashing us by sending "Content-Length: 9999999999999999"
  જો (ContentLength == 0 || ContentLength > 1 << 30)
    પાછા failure();

  Json.resize(ContentLength);
  માટે (size_t Pos = 0, Read; Pos < ContentLength; Pos += Read) {
    Read = std::fread(&Json[Pos], 1, ContentLength - Pos, In);
    જો (Read == 0)
      પાછા failure();

    // If we're done, the error was transient. If we're not done, either it was
    // transient or we'll see it again on retry.
    clearerr(In);
    Pos += Read;
  }
  પાછા success();
}

/// For lit tests we support a simplified syntax:
/// - messages are delimited by '// -----' on a line by itself
/// - lines starting with // are ignored.
/// This is a testing path, so favor simplicity over performance here.
/// When returning failure: feof(), ferror(), or shutdownRequested() will be
/// set.
LogicalResult
JSONTransport_આગત_Over_File::readDelimitedMessage(std::string &Json) {
  Json.clear();
  llvm::SmallString<128> લીટી;
  જ્યારે (succeeded(read_લીટી(In, લીટી))) {
    StringRef લીટી_Ref = લીટી.str().trim();
    જો (લીટી_Ref.starts_with("//")) {
      // Found a delimiter for the message.
      જો (લીટી_Ref == "// -----")
        તોડો;
      ચાલુ;
    }

    Json += લીટી;
  }

  પાછા failure(ferror(In));
}
