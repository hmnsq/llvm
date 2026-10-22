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
صنف Reply {
عام:
  Reply(ثابت llvm::json::قيمة &Id, StringRef Method, JSONTransport &Transport,
        std::mutex &TransportOutputMutex);
  Reply(Reply &&Other);
  Reply &operator=(Reply &&) = delete;
  Reply(ثابت Reply &) = delete;
  Reply &operator=(ثابت Reply &) = delete;

  فراغ operator()(llvm::Expected<llvm::json::قيمة> Reply);

خاص:
  std::string Method;
  std::atomic<منطقي> Replied = {خطأ};
  llvm::json::قيمة Id;
  JSONTransport *Transport;
  std::mutex &TransportOutputMutex;
};
} // namespace

Reply::Reply(ثابت llvm::json::قيمة &Id, llvm::StringRef Method,
             JSONTransport &Transport, std::mutex &TransportOutputMutex)
    : Method(Method), Id(Id), Transport(&Transport),
      TransportOutputMutex(TransportOutputMutex) {}

Reply::Reply(Reply &&Other)
    : Method(Other.Method), Replied(Other.Replied.load()),
      Id(std::move(Other.Id)), Transport(Other.Transport),
      TransportOutputMutex(Other.TransportOutputMutex) {
  Other.Transport = nullptr;
}

فراغ Reply::operator()(llvm::Expected<llvm::json::قيمة> Reply) {
  إذا (Replied.exchange(true)) {
    Logger::error("Replied twice to message {0}({1})", Method, Id);
    أكد(خطأ && "must reply to each call only once!");
    أرجع;
  }
  أكد(Transport && "expected valid transport to reply to");

  std::lock_guard<std::mutex> TransportLock(TransportOutputMutex);
  إذا (Reply) {
    Logger::info("--> reply:{0}({1})", Method, Id);
    Transport->reply(std::move(Id), std::move(Reply));
  } وإلا {
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
MessageHandler::handleParseError(ثابت llvm::json::قيمة &Raw,
                                 StringRef PayloadName, StringRef PayloadKind,
                                 ثابت llvm::json::Path::جذر_2 &جذر_2) {
  // Dump the relevant parts of the broken message.
  std::string Context;
  llvm::raw_string_ostream Os(Context);
  جذر_2.اطبع_Error_Context(Raw, Os);

  // Report the error (e.g. to the client).
  أرجع llvm::make_error<LSPError>(
      llvm::formatv("failed to decode {0} {1}: {2}", PayloadName, PayloadKind,
                    fmt_consume(جذر_2.getError())),
      ErrorCode::InvalidParams);
}

منطقي MessageHandler::onNotify(llvm::StringRef Method, llvm::json::قيمة قيمة) {
  Logger::info("--> {0}", Method);

  إذا (Method == "exit")
    أرجع خطأ;
  إذا (Method == "$cancel") {
    // TODO: Add support for cancelling requests.
  } وإلا {
    auto It = NotificationHandlers.find(Method);
    إذا (It != NotificationHandlers.end())
      It->second(std::move(قيمة));
  }
  أرجع true;
}

منطقي MessageHandler::onCall(llvm::StringRef Method, llvm::json::قيمة Params,
                            llvm::json::قيمة Id) {
  Logger::info("--> {0}({1})", Method, Id);

  Reply Reply(Id, Method, Transport, TransportOutputMutex);

  auto It = MethodHandlers.find(Method);
  إذا (It != MethodHandlers.end()) {
    It->second(std::move(Params), std::move(Reply));
  } وإلا {
    Reply(llvm::make_error<LSPError>("method not found: " + Method.str(),
                                     ErrorCode::MethodNotFound));
  }
  أرجع true;
}

منطقي MessageHandler::onReply(llvm::json::قيمة Id,
                             llvm::Expected<llvm::json::قيمة> Result) {
  // Find the response handler in the mapping. If it exists, move it out of the
  // mapping and erase it.
  ResponseHandlerTy ResponseHandler;
  {
    std::lock_guard<std::mutex> responseHandlersLock(ResponseHandlersMutex);
    auto It = ResponseHandlers.find(debugString(Id));
    إذا (It != ResponseHandlers.end()) {
      ResponseHandler = std::move(It->second);
      ResponseHandlers.erase(It);
    }
  }

  // If we found a response handler, invoke it. Otherwise, log an error.
  إذا (ResponseHandler.second) {
    Logger::info("--> reply:{0}({1})", ResponseHandler.first, Id);
    ResponseHandler.second(std::move(Id), std::move(Result));
  } وإلا {
    Logger::error(
        "received a reply with ID {0}, but there was no such outgoing request",
        Id);
    إذا (!Result)
      llvm::consumeError(Result.takeError());
  }
  أرجع true;
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
    أرجع llvm::Error::success();
  };
  إذا (llvm::Error Unhandled = llvm::handleErrors(std::move(Error), HandlerFn))
    Message = llvm::toString(std::move(Unhandled));

  أرجع llvm::json::Object{
      {"message", std::move(Message)},
      {"code", int64_t(Code)},
  };
}

/// Decode the given JSON object into an error.
llvm::Error decodeError(ثابت llvm::json::Object &O) {
  StringRef رسالة_2 = O.getString("message").قيمة_or("Unspecified error");
  إذا (std::optional<int64_t> Code = O.getInteger("code"))
    أرجع llvm::make_error<LSPError>(رسالة_2.str(), ErrorCode(*Code));
  أرجع llvm::make_error<llvm::StringError>(llvm::inconvertibleErrorCode(),
                                             رسالة_2.str());
}

فراغ JSONTransport::notify(StringRef Method, llvm::json::قيمة Params) {
  sendMessage(llvm::json::Object{
      {"jsonrpc", "2.0"},
      {"method", Method},
      {"params", std::move(Params)},
  });
}
فراغ JSONTransport::call(StringRef Method, llvm::json::قيمة Params,
                         llvm::json::قيمة Id) {
  sendMessage(llvm::json::Object{
      {"jsonrpc", "2.0"},
      {"id", std::move(Id)},
      {"method", Method},
      {"params", std::move(Params)},
  });
}
فراغ JSONTransport::reply(llvm::json::قيمة Id,
                          llvm::Expected<llvm::json::قيمة> Result) {
  إذا (Result) {
    أرجع sendMessage(llvm::json::Object{
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
  طالما (!In->is_End_Of_مدخل()) {
    إذا (In->hasError()) {
      أرجع llvm::errorCodeToError(
          std::error_code(errno, std::system_category()));
    }

    إذا (succeeded(In->readMessage(Json))) {
      إذا (llvm::Expected<llvm::json::قيمة> Doc = llvm::json::parse(Json)) {
        إذا (!handleMessage(std::move(*Doc), Handler))
          أرجع llvm::Error::success();
      } وإلا {
        Logger::error("JSON parse error: {0}", llvm::toString(Doc.takeError()));
      }
    }
  }
  أرجع llvm::errorCodeToError(std::make_error_code(std::errc::io_error));
}

فراغ JSONTransport::sendMessage(llvm::json::قيمة رسالة_2) {
  OutputBuffer.clear();
  llvm::raw_svector_ostream os(OutputBuffer);
  os << llvm::formatv(PrettyOutput ? "{0:2}\n" : "{0}", رسالة_2);
  Out << "Content-Length: " << OutputBuffer.size() << "\r\n\r\n"
      << OutputBuffer;
  Out.flush();
  Logger::debug(">>> {0}\n", OutputBuffer);
}

منطقي JSONTransport::handleMessage(llvm::json::قيمة رسالة_2,
                                  MessageHandler &Handler) {
  // Message must be an object with "jsonrpc":"2.0".
  llvm::json::Object *Object = رسالة_2.getAsObject();
  إذا (!Object ||
      Object->getString("jsonrpc") != std::optional<StringRef>("2.0"))
    أرجع خطأ;

  // `id` may be any JSON value. If absent, this is a notification.
  std::optional<llvm::json::قيمة> Id;
  إذا (llvm::json::قيمة *I = Object->get("id"))
    Id = std::move(*I);
  std::optional<StringRef> Method = Object->getString("method");

  // This is a response.
  إذا (!Method) {
    إذا (!Id)
      أرجع خطأ;
    إذا (auto *Err = Object->getObject("error"))
      أرجع Handler.onReply(std::move(*Id), decodeError(*Err));
    // result should be given, use null if not.
    llvm::json::قيمة Result = nullptr;
    إذا (llvm::json::قيمة *R = Object->get("result"))
      Result = std::move(*R);
    أرجع Handler.onReply(std::move(*Id), std::move(Result));
  }

  // Params should be given, use null if not.
  llvm::json::قيمة Params = nullptr;
  إذا (llvm::json::قيمة *P = Object->get("params"))
    Params = std::move(*P);

  إذا (Id)
    أرجع Handler.onCall(*Method, std::move(Params), std::move(*Id));
  أرجع Handler.onNotify(*Method, std::move(Params));
}

/// Tries to read a line up to and including \n.
/// If failing, feof(), ferror(), or shutdownRequested() will be set.
LogicalResult read_سطر(std::FILE *In, SmallVectorImpl<حرف> &Out) {
  // Big enough to hold any reasonable header line. May not fit content lines
  // in delimited mode, but performance doesn't matter for that mode.
  ساكن constexpr صحيح BufSize = 128;
  size_t Size = 0;
  Out.clear();
  لكل (;;) {
    Out.resize_for_overwrite(Size + BufSize);
    إذا (!std::fgets(&Out[Size], BufSize, In))
      أرجع failure();

    clearerr(In);

    // If the line contained null bytes, anything after it (including \n) will
    // be ignored. Fortunately this is not a legal header or JSON.
    size_t Read = std::strlen(&Out[Size]);
    إذا (Read > 0 && Out[Size + Read - 1] == '\n') {
      Out.resize(Size + Read);
      أرجع success();
    }
    Size += Read;
  }
}

// Returns std::nullopt when:
//  - ferror(), feof(), or shutdownRequested() are set.
//  - Content-Length is missing or empty (protocol error)
LogicalResult
JSONTransport_مدخل_Over_File::readStandardMessage(std::string &Json) {
  // A Language Server Protocol message starts with a set of HTTP headers,
  // delimited  by \r\n, and terminated by an empty line (\r\n).
  unsigned long long ContentLength = 0;
  llvm::SmallString<128> سطر;
  طالما (true) {
    إذا (feof(In) || hasError() || failed(read_سطر(In, سطر)))
      أرجع failure();

    // Content-Length is a mandatory header, and the only one we handle.
    StringRef سطر_Ref = سطر;
    إذا (سطر_Ref.consume_front("Content-Length: ")) {
      llvm::getAsUnsignedInteger(سطر_Ref.trim(), 0, ContentLength);
    } وإلا إذا (!سطر_Ref.trim().empty()) {
      // It's another header, ignore it.
      استمر;
    } وإلا {
      // An empty line indicates the end of headers. Go ahead and read the JSON.
      اكسر;
    }
  }

  // The fuzzer likes crashing us by sending "Content-Length: 9999999999999999"
  إذا (ContentLength == 0 || ContentLength > 1 << 30)
    أرجع failure();

  Json.resize(ContentLength);
  لكل (size_t Pos = 0, Read; Pos < ContentLength; Pos += Read) {
    Read = std::fread(&Json[Pos], 1, ContentLength - Pos, In);
    إذا (Read == 0)
      أرجع failure();

    // If we're done, the error was transient. If we're not done, either it was
    // transient or we'll see it again on retry.
    clearerr(In);
    Pos += Read;
  }
  أرجع success();
}

/// For lit tests we support a simplified syntax:
/// - messages are delimited by '// -----' on a line by itself
/// - lines starting with // are ignored.
/// This is a testing path, so favor simplicity over performance here.
/// When returning failure: feof(), ferror(), or shutdownRequested() will be
/// set.
LogicalResult
JSONTransport_مدخل_Over_File::readDelimitedMessage(std::string &Json) {
  Json.clear();
  llvm::SmallString<128> سطر;
  طالما (succeeded(read_سطر(In, سطر))) {
    StringRef سطر_Ref = سطر.str().trim();
    إذا (سطر_Ref.starts_with("//")) {
      // Found a delimiter for the message.
      إذا (سطر_Ref == "// -----")
        اكسر;
      استمر;
    }

    Json += سطر;
  }

  أرجع failure(ferror(In));
}
