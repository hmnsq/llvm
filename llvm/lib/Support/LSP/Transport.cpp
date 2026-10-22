//===--- JSONTransport.cpp - sending and receiving LSP messages over JSON -===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ضمن "llvm/Support/LSP/Transport.h"
#ضمن "llvm/ADT/SmallString.h"
#ضمن "llvm/Support/Error.h"
#ضمن "llvm/Support/LSP/Logging.h"
#ضمن "llvm/Support/LSP/Protocol.h"
#ضمن <atomic>
#ضمن <optional>
#ضمن <system_error>
#ضمن <utility>

using نطاق llvm;
using نطاق llvm::lsp;

//===----------------------------------------------------------------------===//
// Reply
//===----------------------------------------------------------------------===//

نطاق {
/// Function object to reply to an LSP call.
/// Each instance must be called exactly once, otherwise:
///  - if there was no reply, an error reply is sent
///  - if there were multiple replies, only the first is sent
מחלקה Reply {
عام:
  Reply(ثابت llvm::json::Value &Id, StringRef Method, JSONTransport &Transport,
        std::mutex &TransportOutputMutex);
  Reply(Reply &&Other);
  Reply &operator=(Reply &&) = delete;
  Reply(ثابت Reply &) = delete;
  Reply &operator=(ثابت Reply &) = delete;

  فراغ operator()(llvm::Expected<llvm::json::Value> Reply);

خاص:
  std::string Method;
  std::atomic<منطقي> Replied = {שקר};
  llvm::json::Value Id;
  JSONTransport *Transport;
  std::mutex &TransportOutputMutex;
};
} // namespace

Reply::Reply(ثابت llvm::json::Value &Id, llvm::StringRef Method,
             JSONTransport &Transport, std::mutex &TransportOutputMutex)
    : Method(Method), Id(Id), Transport(&Transport),
      TransportOutputMutex(TransportOutputMutex) {}

Reply::Reply(Reply &&Other)
    : Method(Other.Method), Replied(Other.Replied.load()),
      Id(std::move(Other.Id)), Transport(Other.Transport),
      TransportOutputMutex(Other.TransportOutputMutex) {
  Other.Transport = nullptr;
}

فراغ Reply::operator()(llvm::Expected<llvm::json::Value> Reply) {
  אם (Replied.exchange(אמת)) {
    Logger::error("Replied twice to message {0}({1})", Method, Id);
    أكد(שקר && "must reply to each call only once!");
    החזר;
  }
  أكد(Transport && "expected valid transport to reply to");

  std::lock_guard<std::mutex> TransportLock(TransportOutputMutex);
  אם (Reply) {
    Logger::info("--> reply:{0}({1})", Method, Id);
    Transport->reply(std::move(Id), std::move(Reply));
  } אחרת {
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
MessageHandler::handleParseError(ثابت llvm::json::Value &Raw,
                                 StringRef PayloadName, StringRef PayloadKind,
                                 ثابت llvm::json::Path::Root &Root) {
  // Dump the relevant parts of the broken message.
  std::string Context;
  llvm::raw_string_ostream Os(Context);
  Root.printErrorContext(Raw, Os);

  // Report the error (e.g. to the client).
  החזר llvm::make_error<LSPError>(
      llvm::formatv("failed to decode {0} {1}: {2}", PayloadName, PayloadKind,
                    fmt_consume(Root.getError())),
      ErrorCode::InvalidParams);
}

منطقي MessageHandler::onNotify(llvm::StringRef Method, llvm::json::Value Value) {
  Logger::info("--> {0}", Method);

  אם (Method == "exit")
    החזר שקר;
  אם (Method == "$cancel") {
    // TODO: Add support for cancelling requests.
  } אחרת {
    auto It = NotificationHandlers.find(Method);
    אם (It != NotificationHandlers.end())
      It->second(std::move(Value));
  }
  החזר אמת;
}

منطقي MessageHandler::onCall(llvm::StringRef Method, llvm::json::Value Params,
                            llvm::json::Value Id) {
  Logger::info("--> {0}({1})", Method, Id);

  Reply Reply(Id, Method, Transport, TransportOutputMutex);

  auto It = MethodHandlers.find(Method);
  אם (It != MethodHandlers.end()) {
    It->second(std::move(Params), std::move(Reply));
  } אחרת {
    Reply(llvm::make_error<LSPError>("method not found: " + Method.str(),
                                     ErrorCode::MethodNotFound));
  }
  החזר אמת;
}

منطقي MessageHandler::onReply(llvm::json::Value Id,
                             llvm::Expected<llvm::json::Value> Result) {
  // Find the response handler in the mapping. If it exists, move it out of the
  // mapping and erase it.
  ResponseHandlerTy ResponseHandler;
  {
    std::lock_guard<std::mutex> responseHandlersLock(ResponseHandlersMutex);
    auto It = ResponseHandlers.find(debugString(Id));
    אם (It != ResponseHandlers.end()) {
      ResponseHandler = std::move(It->second);
      ResponseHandlers.erase(It);
    }
  }

  // If we found a response handler, invoke it. Otherwise, log an error.
  אם (ResponseHandler.second) {
    Logger::info("--> reply:{0}({1})", ResponseHandler.first, Id);
    ResponseHandler.second(std::move(Id), std::move(Result));
  } אחרת {
    Logger::error(
        "received a reply with ID {0}, but there was no such outgoing request",
        Id);
    אם (!Result)
      llvm::consumeError(Result.takeError());
  }
  החזר אמת;
}

//===----------------------------------------------------------------------===//
// JSONTransport
//===----------------------------------------------------------------------===//

/// Encode the given error as a JSON object.
ساكن llvm::json::Object encodeError(llvm::Error Error) {
  std::string Message;
  ErrorCode Code = ErrorCode::UnknownErrorCode;
  auto HandlerFn = [&](ثابت LSPError &LspError) -> llvm::Error {
    Message = LspError.message;
    Code = LspError.code;
    החזר llvm::Error::success();
  };
  אם (llvm::Error Unhandled = llvm::handleErrors(std::move(Error), HandlerFn))
    Message = llvm::toString(std::move(Unhandled));

  החזר llvm::json::Object{
      {"message", std::move(Message)},
      {"code", int64_t(Code)},
  };
}

/// Decode the given JSON object into an error.
llvm::Error decodeError(ثابت llvm::json::Object &O) {
  StringRef Msg = O.getString("message").value_or("Unspecified error");
  אם (std::optional<int64_t> Code = O.getInteger("code"))
    החזר llvm::make_error<LSPError>(Msg.str(), ErrorCode(*Code));
  החזר llvm::make_error<llvm::StringError>(llvm::inconvertibleErrorCode(),
                                             Msg.str());
}

فراغ JSONTransport::notify(StringRef Method, llvm::json::Value Params) {
  sendMessage(llvm::json::Object{
      {"jsonrpc", "2.0"},
      {"method", Method},
      {"params", std::move(Params)},
  });
}
فراغ JSONTransport::call(StringRef Method, llvm::json::Value Params,
                         llvm::json::Value Id) {
  sendMessage(llvm::json::Object{
      {"jsonrpc", "2.0"},
      {"id", std::move(Id)},
      {"method", Method},
      {"params", std::move(Params)},
  });
}
فراغ JSONTransport::reply(llvm::json::Value Id,
                          llvm::Expected<llvm::json::Value> Result) {
  אם (Result) {
    החזר sendMessage(llvm::json::Object{
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
  כלעוד (!In->isEndOfInput()) {
    אם (In->hasError()) {
      החזר llvm::errorCodeToError(
          std::error_code(errno, std::system_category()));
    }

    אם (succeeded(In->readMessage(Json))) {
      אם (llvm::Expected<llvm::json::Value> Doc = llvm::json::parse(Json)) {
        אם (!handleMessage(std::move(*Doc), Handler))
          החזר llvm::Error::success();
      } אחרת {
        Logger::error("JSON parse error: {0}", llvm::toString(Doc.takeError()));
      }
    }
  }
  החזר llvm::errorCodeToError(std::make_error_code(std::errc::io_error));
}

فراغ JSONTransport::sendMessage(llvm::json::Value Msg) {
  OutputBuffer.clear();
  llvm::raw_svector_ostream os(OutputBuffer);
  os << llvm::formatv(PrettyOutput ? "{0:2}\n" : "{0}", Msg);
  Out << "Content-Length: " << OutputBuffer.size() << "\r\n\r\n"
      << OutputBuffer;
  Out.flush();
  Logger::debug(">>> {0}\n", OutputBuffer);
}

منطقي JSONTransport::handleMessage(llvm::json::Value Msg,
                                  MessageHandler &Handler) {
  // Message must be an object with "jsonrpc":"2.0".
  llvm::json::Object *Object = Msg.getAsObject();
  אם (!Object ||
      Object->getString("jsonrpc") != std::optional<StringRef>("2.0"))
    החזר שקר;

  // `id` may be any JSON value. If absent, this is a notification.
  std::optional<llvm::json::Value> Id;
  אם (llvm::json::Value *I = Object->get("id"))
    Id = std::move(*I);
  std::optional<StringRef> Method = Object->getString("method");

  // This is a response.
  אם (!Method) {
    אם (!Id)
      החזר שקר;
    אם (auto *Err = Object->getObject("error"))
      החזר Handler.onReply(std::move(*Id), decodeError(*Err));
    // result should be given, use null if not.
    llvm::json::Value Result = nullptr;
    אם (llvm::json::Value *R = Object->get("result"))
      Result = std::move(*R);
    החזר Handler.onReply(std::move(*Id), std::move(Result));
  }

  // Params should be given, use null if not.
  llvm::json::Value Params = nullptr;
  אם (llvm::json::Value *P = Object->get("params"))
    Params = std::move(*P);

  אם (Id)
    החזר Handler.onCall(*Method, std::move(Params), std::move(*Id));
  החזר Handler.onNotify(*Method, std::move(Params));
}

/// Tries to read a line up to and including \n.
/// If failing, feof(), ferror(), or shutdownRequested() will be set.
LogicalResult readLine(std::FILE *In, SmallVectorImpl<حرف> &Out) {
  // Big enough to hold any reasonable header line. May not fit content lines
  // in delimited mode, but performance doesn't matter for that mode.
  ساكن constexpr صحيح BufSize = 128;
  size_t Size = 0;
  Out.clear();
  עבור (;;) {
    Out.resize_for_overwrite(Size + BufSize);
    אם (!std::fgets(&Out[Size], BufSize, In))
      החזר failure();

    clearerr(In);

    // If the line contained null bytes, anything after it (including \n) will
    // be ignored. Fortunately this is not a legal header or JSON.
    size_t Read = std::strlen(&Out[Size]);
    אם (Read > 0 && Out[Size + Read - 1] == '\n') {
      Out.resize(Size + Read);
      החזר success();
    }
    Size += Read;
  }
}

// Returns std::nullopt when:
//  - ferror(), feof(), or shutdownRequested() are set.
//  - Content-Length is missing or empty (protocol error)
LogicalResult
JSONTransportInputOverFile::readStandardMessage(std::string &Json) {
  // A Language Server Protocol message starts with a set of HTTP headers,
  // delimited  by \r\n, and terminated by an empty line (\r\n).
  unsigned long long ContentLength = 0;
  llvm::SmallString<128> Line;
  כלעוד (אמת) {
    אם (feof(In) || hasError() || failed(readLine(In, Line)))
      החזר failure();

    // Content-Length is a mandatory header, and the only one we handle.
    StringRef LineRef = Line;
    אם (LineRef.consume_front("Content-Length: ")) {
      llvm::getAsUnsignedInteger(LineRef.trim(), 0, ContentLength);
    } אחרת אם (!LineRef.trim().empty()) {
      // It's another header, ignore it.
      استمر;
    } אחרת {
      // An empty line indicates the end of headers. Go ahead and read the JSON.
      اكسر;
    }
  }

  // The fuzzer likes crashing us by sending "Content-Length: 9999999999999999"
  אם (ContentLength == 0 || ContentLength > 1 << 30)
    החזר failure();

  Json.resize(ContentLength);
  עבור (size_t Pos = 0, Read; Pos < ContentLength; Pos += Read) {
    Read = std::fread(&Json[Pos], 1, ContentLength - Pos, In);
    אם (Read == 0)
      החזר failure();

    // If we're done, the error was transient. If we're not done, either it was
    // transient or we'll see it again on retry.
    clearerr(In);
    Pos += Read;
  }
  החזר success();
}

/// For lit tests we support a simplified syntax:
/// - messages are delimited by '// -----' on a line by itself
/// - lines starting with // are ignored.
/// This is a testing path, so favor simplicity over performance here.
/// When returning failure: feof(), ferror(), or shutdownRequested() will be
/// set.
LogicalResult
JSONTransportInputOverFile::readDelimitedMessage(std::string &Json) {
  Json.clear();
  llvm::SmallString<128> Line;
  כלעוד (succeeded(readLine(In, Line))) {
    StringRef LineRef = Line.str().trim();
    אם (LineRef.starts_with("//")) {
      // Found a delimiter for the message.
      אם (LineRef == "// -----")
        اكسر;
      استمر;
    }

    Json += Line;
  }

  החזר failure(ferror(In));
}
