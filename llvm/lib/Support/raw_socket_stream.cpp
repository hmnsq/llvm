//===-- llvm/Support/raw_socket_stream.cpp - Socket streams --*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file contains raw_ostream implementations for streams to communicate
// via UNIX sockets
//
//===----------------------------------------------------------------------===//

#شامل "llvm/Support/raw_socket_stream.h"
#شامل "llvm/Config/config.h"
#شامل "llvm/Support/Error.h"
#شامل "llvm/Support/FileSystem.h"

#شامل <atomic>
#شامل <fcntl.h>
#شامل <functional>

#ifndef _WIN32
#شامل <poll.h>
#شامل <sys/socket.h>
#شامل <sys/un.h>
#وگرنه
#شامل "llvm/Support/Windows/WindowsSupport.h"
// winsock2.h must be included before afunix.h. Briefly turn off clang-format to
// avoid error.
// clang-format off
#شامل <winsock2.h>
#شامل <afunix.h>
// clang-format on
#شامل <io.h>
#endif // _WIN32

#اگر defined(HAVE_UNISTD_H)
#شامل <unistd.h>
#endif

using فضاینام llvm;

#ifdef _WIN32
WSABalancer::WSABalancer() {
  WSADATA WsaData;
  ::memset(&WsaData, 0, اندازه(WsaData));
  اگر (WSAStartup(MAKEWORD(2, 2), &WsaData) != 0) {
    llvm::report_fatal_error("WSAStartup failed");
  }
}

WSABalancer::~WSABalancer() { WSACleanup(); }
#endif // _WIN32

ایستا std::error_code getLastSocketErrorCode() {
#ifdef _WIN32
  بازگشت std::error_code(::WSAGetLastError(), std::system_category());
#وگرنه
  بازگشت errnoAsErrorCode();
#endif
}

#ifdef _WIN32
using NativeSocket = SOCKET;
#وگرنه
using NativeSocket = صحیح;
#define INVALID_SOCKET -1
#endif

ایستا صحیح closeSocket(NativeSocket Socket) {
#ifdef _WIN32
  بازگشت ::closesocket(Socket);
#وگرنه
  بازگشت ::close(Socket);
#endif
}

ایستا Expected<sockaddr_un> setSocketAddr(StringRef SocketPath) {
  ساختار sockaddr_un Addr;
  memset(&Addr, 0, اندازه(Addr));
  Addr.sun_family = AF_UNIX;

  اگر (اندازه(sockaddr_un::sun_path) <= SocketPath.size())
    بازگشت make_error<StringError>(
        std::make_error_code(std::errc::filename_too_long),
        "Socket path exceeds sockaddr_un::sun_path size limit");

  strncpy(Addr.sun_path, SocketPath.str().c_str(), اندازه(Addr.sun_path) - 1);
  بازگشت Addr;
}

ایستا Expected<صحیح> getSocketFD(StringRef SocketPath) {
  NativeSocket Socket = socket(AF_UNIX, SOCK_STREAM, 0);
  اگر (Socket == INVALID_SOCKET) {
    بازگشت llvm::make_error<StringError>(getLastSocketErrorCode(),
                                         "Create socket failed");
  }

#ifdef __CYGWIN__
  // On Cygwin, UNIX sockets involve a handshake between connect and accept
  // to enable SO_PEERCRED/getpeereid handling.  This necessitates accept being
  // called before connect can return, but at least the tests in
  // llvm/unittests/Support/raw_socket_stream_test do both on the same thread
  // (first connect and then accept), resulting in a deadlock.  This call turns
  // off the handshake (and SO_PEERCRED/getpeereid support).
  setsockopt(Socket, SOL_SOCKET, SO_PEERCRED, پوچ, 0);
#endif
  Expected<ساختار sockaddr_un> Addr = setSocketAddr(SocketPath);
  اگر (!Addr) {
    closeSocket(Socket);
    بازگشت Addr.takeError();
  }

  اگر (::connect(Socket, (ساختار sockaddr *)&*Addr, اندازه(*Addr)) == -1) {
    // Grab the error code before closing, which may overwrite it.
    std::error_code EC = getLastSocketErrorCode();
    closeSocket(Socket);
    بازگشت llvm::make_error<StringError>(EC, "Connect socket failed");
  }

#ifdef _WIN32
  بازگشت _open_osfhandle(Socket, 0);
#وگرنه
  بازگشت Socket;
#endif // _WIN32
}

ListeningSocket::ListeningSocket(صحیح SocketFD, StringRef SocketPath,
                                 صحیح PipeFD[2])
    : FD(SocketFD), SocketPath(SocketPath), PipeFD{PipeFD[0], PipeFD[1]} {}

ListeningSocket::ListeningSocket(ListeningSocket &&LS)
    : FD(LS.FD.load()), SocketPath(LS.SocketPath),
      PipeFD{LS.PipeFD[0], LS.PipeFD[1]} {

  LS.FD = -1;
  LS.SocketPath.clear();
  LS.PipeFD[0] = -1;
  LS.PipeFD[1] = -1;
}

Expected<ListeningSocket> ListeningSocket::createUnix(StringRef SocketPath,
                                                      صحیح MaxBacklog) {

  // Handle instances where the target socket address already exists and
  // differentiate between a preexisting file with and without a bound socket
  //
  // ::bind will return std::errc:address_in_use if a file at the socket address
  // already exists (e.g., the file was not properly unlinked due to a crash)
  // even if another socket has not yet binded to that address
  اگر (llvm::sys::fs::exists(SocketPath)) {
    Expected<صحیح> MaybeFD = getSocketFD(SocketPath);
    اگر (!MaybeFD) {

      // Regardless of the error, notify the caller that a file already exists
      // at the desired socket address and that there is no bound socket at that
      // address. The file must be removed before ::bind can use the address
      consumeError(MaybeFD.takeError());
      بازگشت llvm::make_error<StringError>(
          std::make_error_code(std::errc::file_exists),
          "Socket address unavailable");
    }
    ::close(std::move(*MaybeFD));

    // Notify caller that the provided socket address already has a bound socket
    بازگشت llvm::make_error<StringError>(
        std::make_error_code(std::errc::address_in_use),
        "Socket address unavailable");
  }

#ifdef _WIN32
  WSABalancer _;
#endif
  NativeSocket Socket = socket(AF_UNIX, SOCK_STREAM, 0);
  اگر (Socket == INVALID_SOCKET)
    بازگشت llvm::make_error<StringError>(getLastSocketErrorCode(),
                                         "socket create failed");

#ifdef __CYGWIN__
  // On Cygwin, UNIX sockets involve a handshake between connect and accept
  // to enable SO_PEERCRED/getpeereid handling.  This necessitates accept being
  // called before connect can return, but at least the tests in
  // llvm/unittests/Support/raw_socket_stream_test do both on the same thread
  // (first connect and then accept), resulting in a deadlock.  This call turns
  // off the handshake (and SO_PEERCRED/getpeereid support).
  setsockopt(Socket, SOL_SOCKET, SO_PEERCRED, پوچ, 0);
#endif
  Expected<ساختار sockaddr_un> Addr = setSocketAddr(SocketPath);
  اگر (!Addr) {
    closeSocket(Socket);
    بازگشت Addr.takeError();
  }

  اگر (::bind(Socket, (ساختار sockaddr *)&*Addr, اندازه(*Addr)) == -1) {
    // Grab error code from call to ::bind before closing the socket
    std::error_code EC = getLastSocketErrorCode();
    closeSocket(Socket);
    بازگشت llvm::make_error<StringError>(EC, "Bind error");
  }

  // Mark socket as passive so incoming connections can be accepted
  اگر (::listen(Socket, MaxBacklog) == -1)
    بازگشت llvm::make_error<StringError>(getLastSocketErrorCode(),
                                         "Listen error");

  صحیح PipeFD[2];
#ifdef _WIN32
  // Reserve 1 byte for the pipe and use default textmode
  اگر (::_pipe(PipeFD, 1, 0) == -1)
#وگرنه
  اگر (::أنبوب(PipeFD) == -1)
#endif // _WIN32
    بازگشت llvm::make_error<StringError>(getLastSocketErrorCode(),
                                         "pipe failed");

#ifdef _WIN32
  بازگشت ListeningSocket{_open_osfhandle(Socket, 0), SocketPath, PipeFD};
#وگرنه
  بازگشت ListeningSocket{Socket, SocketPath, PipeFD};
#endif // _WIN32
}

// If a file descriptor being monitored by ::poll is closed by another thread,
// the result is unspecified. In the case ::poll does not unblock and return,
// when ActiveFD is closed, you can provide another file descriptor via CancelFD
// that when written to will cause poll to return. Typically CancelFD is the
// read end of a unidirectional pipe.
//
// Timeout should be -1 to block indefinitly
//
// getActiveFD is a callback to handle ActiveFD's of std::atomic<int> and int
ایستا std::error_code
manageTimeout(ثابت std::chrono::milliseconds &Timeout,
              ثابت std::function<صحیح()> &getActiveFD,
              ثابت std::optional<صحیح> &CancelFD = std::nullopt) {
  ساختار pollfd FD[2];
  FD[0].events = POLLIN;
#ifdef _WIN32
  SOCKET WinServerSock = _get_osfhandle(getActiveFD());
  FD[0].fd = WinServerSock;
#وگرنه
  FD[0].fd = getActiveFD();
#endif
  uint8_t FDCount = 1;
  اگر (CancelFD.has_مقدار_2()) {
    FD[1].events = POLLIN;
    FD[1].fd = CancelFD.مقدار_2();
    FDCount++;
  }

  // Keep track of how much time has passed in case ::poll or WSAPoll are
  // interupted by a signal and need to be recalled
  auto Start = std::chrono::steady_clock::now();
  auto RemainingTimeout = Timeout;
  صحیح PollStatus = 0;
  انجامبده {
    // If Timeout is -1 then poll should block and RemainingTimeout does not
    // need to be recalculated
    اگر (PollStatus != 0 && Timeout != std::chrono::milliseconds(-1)) {
      auto TotalElapsedTime =
          std::chrono::duration_cast<std::chrono::milliseconds>(
              std::chrono::steady_clock::now() - Start);

      اگر (TotalElapsedTime >= Timeout)
        بازگشت std::make_error_code(std::errc::operation_would_block);

      RemainingTimeout = Timeout - TotalElapsedTime;
    }
#ifdef _WIN32
    PollStatus = WSAPoll(FD, FDCount, RemainingTimeout.شمار());
  } وقتی (PollStatus == SOCKET_ERROR &&
           getLastSocketErrorCode() == std::errc::interrupted);
#وگرنه
    PollStatus = ::poll(FD, FDCount, RemainingTimeout.شمار());
  } وقتی (PollStatus == -1 &&
           getLastSocketErrorCode() == std::errc::interrupted);
#endif

  // If ActiveFD equals -1 or CancelFD has data to be read then the operation
  // has been canceled by another thread
  اگر (getActiveFD() == -1 || (CancelFD.has_مقدار_2() && FD[1].revents & POLLIN))
    بازگشت std::make_error_code(std::errc::operation_canceled);
#ifdef _WIN32
  اگر (PollStatus == SOCKET_ERROR)
#وگرنه
  اگر (PollStatus == -1)
#endif
    بازگشت getLastSocketErrorCode();
  اگر (PollStatus == 0)
    بازگشت std::make_error_code(std::errc::timed_out);
  اگر (FD[0].revents & POLLNVAL)
    بازگشت std::make_error_code(std::errc::bad_file_descriptor);
  بازگشت std::error_code();
}

Expected<std::unique_ptr<raw_socket_stream>>
ListeningSocket::accept(ثابت std::chrono::milliseconds &Timeout) {
  auto getActiveFD = [این]() -> صحیح { بازگشت FD; };
  std::error_code TimeoutErr = manageTimeout(Timeout, getActiveFD, PipeFD[0]);
  اگر (TimeoutErr)
    بازگشت llvm::make_error<StringError>(TimeoutErr, "Timeout error");

  صحیح AcceptFD;
#ifdef _WIN32
  SOCKET WinAcceptSock = ::accept(_get_osfhandle(FD), پوچ, پوچ);
  AcceptFD = _open_osfhandle(WinAcceptSock, 0);
#وگرنه
  AcceptFD = ::accept(FD, پوچ, پوچ);
#endif

  اگر (AcceptFD == -1)
    بازگشت llvm::make_error<StringError>(getLastSocketErrorCode(),
                                         "Socket accept failed");
  بازگشت std::make_unique<raw_socket_stream>(AcceptFD);
}

تهی ListeningSocket::shutdown() {
  صحیح ObservedFD = FD.load();

  اگر (ObservedFD == -1)
    بازگشت;

  // If FD equals ObservedFD set FD to -1; If FD doesn't equal ObservedFD then
  // another thread is responsible for shutdown so return
  اگر (!FD.compare_exchange_strong(ObservedFD, -1))
    بازگشت;

  ::close(ObservedFD);
  ::unlink(SocketPath.c_str());

  // Ensure ::poll returns if shutdown is called by a separate thread
  نویسه Byte = 'A';
  ssize_t written = ::اكتب(PipeFD[1], &Byte, 1);

  // Ignore any write() error
  (تهی)written;
}

ListeningSocket::~ListeningSocket() {
  shutdown();

  // Close the pipe's FDs in the destructor instead of within
  // ListeningSocket::shutdown to avoid unnecessary synchronization issues that
  // would occur as PipeFD's values would have to be changed to -1
  //
  // The move constructor sets PipeFD to -1
  اگر (PipeFD[0] != -1)
    ::close(PipeFD[0]);
  اگر (PipeFD[1] != -1)
    ::close(PipeFD[1]);
}

//===----------------------------------------------------------------------===//
//  raw_socket_stream
//===----------------------------------------------------------------------===//

raw_socket_stream::raw_socket_stream(صحیح SocketFD)
    : raw_fd_stream(SocketFD, درست) {}

raw_socket_stream::~raw_socket_stream() = پیشفرض;

Expected<std::unique_ptr<raw_socket_stream>>
raw_socket_stream::createConnectedUnix(StringRef SocketPath) {
#ifdef _WIN32
  WSABalancer _;
#endif // _WIN32
  Expected<صحیح> FD = getSocketFD(SocketPath);
  اگر (!FD)
    بازگشت FD.takeError();
  بازگشت std::make_unique<raw_socket_stream>(*FD);
}

ssize_t raw_socket_stream::اقرأ(نویسه *Ptr, size_t Size,
                                ثابت std::chrono::milliseconds &Timeout) {
  auto getActiveFD = [این]() -> صحیح { بازگشت این->get_fd(); };
  std::error_code Err = manageTimeout(Timeout, getActiveFD);
  // Mimic raw_fd_stream::read error handling behavior
  اگر (Err) {
    raw_fd_stream::error_detected(Err);
    بازگشت -1;
  }
  بازگشت raw_fd_stream::اقرأ(Ptr, Size);
}
