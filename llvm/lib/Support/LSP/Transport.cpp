//===--- JSONTransport.cpp - sending and receiving LSP messages over JSON -===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#شامل "llvm/Support/LSP/Transport.h"
#شامل "llvm/ADT/SmallString.h"
#شامل "llvm/Support/Error.h"
#شامل "llvm/Support/LSP/Logging.h"
#شامل "llvm/Support/LSP/Protocol.h"
#شامل <atomic>
#شامل <optional>
#شامل <system_error>
#شامل <utility>

using فضاینام llvm;
using فضاینام llvm::lsp;

//===----------------------------------------------------------------------===//
// Reply
//===----------------------------------------------------------------------===//

فضاینام {
/// Function object to reply to an LSP call.
/// Each instance must be called exactly once, otherwise:
///  - if there was no reply, an error reply is sent
///  - if there were multiple replies, only the first is sent
رده Reply {
عمومی:
  Reply(ثابت llvm::json::مقدار &Id, StringRef Method, JSONTransport &Transport,
        std::mutex &TransportOutputMutex);
  Reply(Reply &&Other);
  Reply &operator=(Reply &&) = delete;
  Reply(ثابت Reply &) = delete;
  Reply &operator=(ثابت Reply &) = delete;

  تهی operator()(llvm::Expected<llvm::json::مقدار> Reply);

خصوصی:
  std::string Method;
  std::atomic<منطقی> Replied = {نادرست};
  llvm::json::مقدار Id;
  JSONTransport *Transport;
  std::mutex &TransportOutputMutex;
};
} // namespace

Reply::Reply(ثابت llvm::json::مقدار &Id, llvm::StringRef Method,
             JSONTransport &Transport, std::mutex &TransportOutputMutex)
    : Method(Method), Id(Id), Transport(&Transport),
      TransportOutputMutex(TransportOutputMutex) {}

Reply::Reply(Reply &&Other)
    : Method(Other.Method), Replied(Other.Replied.load()),
      Id(std::move(Other.Id)), Transport(Other.Transport),
      TransportOutputMutex(Other.TransportOutputMutex) {
  Other.Transport = nullptr;
}

تهی Reply::operator()(llvm::Expected<llvm::json::مقدار> Reply) {
  اگر (Replied.exchange(درست)) {
    Logger::error("Replied twice to message {0}({1})", Method, Id);
    أكد(نادرست && "must reply to each call only once!");
    بازگشت;
  }
  أكد(Transport && "expected valid transport to reply to");

  std::lock_guard<std::mutex> TransportLock(TransportOutputMutex);
  اگر (Reply) {
    Logger::info("--> reply:{0}({1})", Method, Id);
    Transport->reply(std::move(Id), std::move(Reply));
  } وگرنه {
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
MessageHandler::handleParseError(ثابت llvm::json::مقدار &Raw,
                                 StringRef PayloadName, StringRef PayloadKind,
                                 ثابت llvm::json::Path::ریشه_2 &ریشه_2) {
  // Dump the relevant parts of the broken message.
  std::string Context;
  llvm::raw_string_ostream Os(Context);
  ریشه_2.printErrorContext(Raw, Os);

  // Report the error (e.g. to the client).
  بازگشت llvm::make_error<LSPError>(
      llvm::formatv("failed to decode {0} {1}: {2}", PayloadName, PayloadKind,
                    fmt_consume(ریشه_2.getError())),
      ErrorCode::InvalidParams);
}

منطقی MessageHandler::onNotify(llvm::StringRef Method, llvm::json::مقدار مقدار) {
  Logger::info("--> {0}", Method);

  اگر (Method == "exit")
    بازگشت نادرست;
  اگر (Method == "$cancel") {
    // TODO: Add support for cancelling requests.
  } وگرنه {
    auto It = NotificationHandlers.find(Method);
    اگر (It != NotificationHandlers.end())
      It->second(std::move(مقدار));
  }
  بازگشت درست;
}

منطقی MessageHandler::onCall(llvm::StringRef Method, llvm::json::مقدار Params,
                            llvm::json::مقدار Id) {
  Logger::info("--> {0}({1})", Method, Id);

  Reply Reply(Id, Method, Transport, TransportOutputMutex);

  auto It = MethodHandlers.find(Method);
  اگر (It != MethodHandlers.end()) {
    It->second(std::move(Params), std::move(Reply));
  } وگرنه {
    Reply(llvm::make_error<LSPError>("method not found: " + Method.str(),
                                     ErrorCode::MethodNotFound));
  }
  بازگشت درست;
}

منطقی MessageHandler::onReply(llvm::json::مقدار Id,
                             llvm::Expected<llvm::json::مقدار> Result) {
  // Find the response handler in the mapping. If it exists, move it out of the
  // mapping and erase it.
  ResponseHandlerTy ResponseHandler;
  {
    std::lock_guard<std::mutex> responseHandlersLock(ResponseHandlersMutex);
    auto It = ResponseHandlers.find(debugString(Id));
    اگر (It != ResponseHandlers.end()) {
      ResponseHandler = std::move(It->second);
      ResponseHandlers.erase(It);
    }
  }

  // If we found a response handler, invoke it. Otherwise, log an error.
  اگر (ResponseHandler.second) {
    Logger::info("--> reply:{0}({1})", ResponseHandler.first, Id);
    ResponseHandler.second(std::move(Id), std::move(Result));
  } وگرنه {
    Logger::error(
        "received a reply with ID {0}, but there was no such outgoing request",
        Id);
    اگر (!Result)
      llvm::consumeError(Result.takeError());
  }
  بازگشت درست;
}

//===----------------------------------------------------------------------===//
// JSONTransport
//===----------------------------------------------------------------------===//

/// Encode the given error as a JSON object.
ایستا llvm::json::Object encodeError(llvm::Error Error) {
  std::string Message;
  ErrorCode Code = ErrorCode::UnknownErrorCode;
  auto HandlerFn = [&](ثابت LSPError &LspError) -> llvm::Error {
    Message = LspError.message;
    Code = LspError.code;
    بازگشت llvm::Error::success();
  };
  اگر (llvm::Error Unhandled = llvm::handleErrors(std::move(Error), HandlerFn))
    Message = llvm::toString(std::move(Unhandled));

  بازگشت llvm::json::Object{
      {"message", std::move(Message)},
      {"code", int64_t(Code)},
  };
}

/// Decode the given JSON object into an error.
llvm::Error decodeError(ثابت llvm::json::Object &O) {
  StringRef پیام_2 = O.getString("message").مقدار_or("Unspecified error");
  اگر (std::optional<int64_t> Code = O.getInteger("code"))
    بازگشت llvm::make_error<LSPError>(پیام_2.str(), ErrorCode(*Code));
  بازگشت llvm::make_error<llvm::StringError>(llvm::inconvertibleErrorCode(),
                                             پیام_2.str());
}

تهی JSONTransport::notify(StringRef Method, llvm::json::مقدار Params) {
  sendMessage(llvm::json::Object{
      {"jsonrpc", "2.0"},
      {"method", Method},
      {"params", std::move(Params)},
  });
}
تهی JSONTransport::call(StringRef Method, llvm::json::مقدار Params,
                         llvm::json::مقدار Id) {
  sendMessage(llvm::json::Object{
      {"jsonrpc", "2.0"},
      {"id", std::move(Id)},
      {"method", Method},
      {"params", std::move(Params)},
  });
}
تهی JSONTransport::reply(llvm::json::مقدار Id,
                          llvm::Expected<llvm::json::مقدار> Result) {
  اگر (Result) {
    بازگشت sendMessage(llvm::json::Object{
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
  وقتی (!In->is_End_Of_درون_داد()) {
    اگر (In->hasError()) {
      بازگشت llvm::errorCodeToError(
          std::error_code(errno, std::system_category()));
    }

    اگر (succeeded(In->readMessage(Json))) {
      اگر (llvm::Expected<llvm::json::مقدار> Doc = llvm::json::parse(Json)) {
        اگر (!handleMessage(std::move(*Doc), Handler))
          بازگشت llvm::Error::success();
      } وگرنه {
        Logger::error("JSON parse error: {0}", llvm::toString(Doc.takeError()));
      }
    }
  }
  بازگشت llvm::errorCodeToError(std::make_error_code(std::errc::io_error));
}

تهی JSONTransport::sendMessage(llvm::json::مقدار پیام_2) {
  OutputBuffer.clear();
  llvm::raw_svector_ostream os(OutputBuffer);
  os << llvm::formatv(PrettyOutput ? "{0:2}\n" : "{0}", پیام_2);
  Out << "Content-Length: " << OutputBuffer.size() << "\r\n\r\n"
      << OutputBuffer;
  Out.flush();
  Logger::debug(">>> {0}\n", OutputBuffer);
}

منطقی JSONTransport::handleMessage(llvm::json::مقدار پیام_2,
                                  MessageHandler &Handler) {
  // Message must be an object with "jsonrpc":"2.0".
  llvm::json::Object *Object = پیام_2.getAsObject();
  اگر (!Object ||
      Object->getString("jsonrpc") != std::optional<StringRef>("2.0"))
    بازگشت نادرست;

  // `id` may be any JSON value. If absent, this is a notification.
  std::optional<llvm::json::مقدار> Id;
  اگر (llvm::json::مقدار *I = Object->get("id"))
    Id = std::move(*I);
  std::optional<StringRef> Method = Object->getString("method");

  // This is a response.
  اگر (!Method) {
    اگر (!Id)
      بازگشت نادرست;
    اگر (auto *Err = Object->getObject("error"))
      بازگشت Handler.onReply(std::move(*Id), decodeError(*Err));
    // result should be given, use null if not.
    llvm::json::مقدار Result = nullptr;
    اگر (llvm::json::مقدار *R = Object->get("result"))
      Result = std::move(*R);
    بازگشت Handler.onReply(std::move(*Id), std::move(Result));
  }

  // Params should be given, use null if not.
  llvm::json::مقدار Params = nullptr;
  اگر (llvm::json::مقدار *P = Object->get("params"))
    Params = std::move(*P);

  اگر (Id)
    بازگشت Handler.onCall(*Method, std::move(Params), std::move(*Id));
  بازگشت Handler.onNotify(*Method, std::move(Params));
}

/// Tries to read a line up to and including \n.
/// If failing, feof(), ferror(), or shutdownRequested() will be set.
LogicalResult read_سطر(std::FILE *In, SmallVectorImpl<نویسه> &Out) {
  // Big enough to hold any reasonable header line. May not fit content lines
  // in delimited mode, but performance doesn't matter for that mode.
  ایستا constexpr صحیح BufSize = 128;
  size_t Size = 0;
  Out.clear();
  برای (;;) {
    Out.resize_for_overwrite(Size + BufSize);
    اگر (!std::fgets(&Out[Size], BufSize, In))
      بازگشت failure();

    clearerr(In);

    // If the line contained null bytes, anything after it (including \n) will
    // be ignored. Fortunately this is not a legal header or JSON.
    size_t Read = std::strlen(&Out[Size]);
    اگر (Read > 0 && Out[Size + Read - 1] == '\n') {
      Out.resize(Size + Read);
      بازگشت success();
    }
    Size += Read;
  }
}

// Returns std::nullopt when:
//  - ferror(), feof(), or shutdownRequested() are set.
//  - Content-Length is missing or empty (protocol error)
LogicalResult
JSONTransport_درون_داد_Over_File::readStandardMessage(std::string &Json) {
  // A Language Server Protocol message starts with a set of HTTP headers,
  // delimited  by \r\n, and terminated by an empty line (\r\n).
  unsigned long long ContentLength = 0;
  llvm::SmallString<128> سطر;
  وقتی (درست) {
    اگر (feof(In) || hasError() || failed(read_سطر(In, سطر)))
      بازگشت failure();

    // Content-Length is a mandatory header, and the only one we handle.
    StringRef سطر_Ref = سطر;
    اگر (سطر_Ref.consume_front("Content-Length: ")) {
      llvm::getAsUnsignedInteger(سطر_Ref.trim(), 0, ContentLength);
    } وگرنه اگر (!سطر_Ref.trim().empty()) {
      // It's another header, ignore it.
      ادامه;
    } وگرنه {
      // An empty line indicates the end of headers. Go ahead and read the JSON.
      بشکن;
    }
  }

  // The fuzzer likes crashing us by sending "Content-Length: 9999999999999999"
  اگر (ContentLength == 0 || ContentLength > 1 << 30)
    بازگشت failure();

  Json.resize(ContentLength);
  برای (size_t Pos = 0, Read; Pos < ContentLength; Pos += Read) {
    Read = std::fread(&Json[Pos], 1, ContentLength - Pos, In);
    اگر (Read == 0)
      بازگشت failure();

    // If we're done, the error was transient. If we're not done, either it was
    // transient or we'll see it again on retry.
    clearerr(In);
    Pos += Read;
  }
  بازگشت success();
}

/// For lit tests we support a simplified syntax:
/// - messages are delimited by '// -----' on a line by itself
/// - lines starting with // are ignored.
/// This is a testing path, so favor simplicity over performance here.
/// When returning failure: feof(), ferror(), or shutdownRequested() will be
/// set.
LogicalResult
JSONTransport_درون_داد_Over_File::readDelimitedMessage(std::string &Json) {
  Json.clear();
  llvm::SmallString<128> سطر;
  وقتی (succeeded(read_سطر(In, سطر))) {
    StringRef سطر_Ref = سطر.str().trim();
    اگر (سطر_Ref.starts_with("//")) {
      // Found a delimiter for the message.
      اگر (سطر_Ref == "// -----")
        بشکن;
      ادامه;
    }

    Json += سطر;
  }

  بازگشت failure(ferror(In));
}
