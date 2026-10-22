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

#ضمن "llvm/Support/raw_socket_stream.h"
#ضمن "llvm/Config/config.h"
#ضمن "llvm/Support/Error.h"
#ضمن "llvm/Support/FileSystem.h"

#ضمن <atomic>
#ضمن <fcntl.h>
#ضمن <functional>

#ifndef _WIN32
#ضمن <poll.h>
#ضمن <sys/socket.h>
#ضمن <sys/un.h>
#وإلا
#ضمن "llvm/Support/Windows/WindowsSupport.h"
// winsock2.h must be included before afunix.h. Briefly turn off clang-format to
// avoid error.
// clang-format off
#ضمن <winsock2.h>
#ضمن <afunix.h>
// clang-format on
#ضمن <io.h>
#endif // _WIN32

#إذا defined(HAVE_UNISTD_H)
#ضمن <unistd.h>
#endif

using نطاق llvm;

#ifdef _WIN32
WSABalancer::WSABalancer() {
  WSADATA WsaData;
  ::memset(&WsaData, 0, حجم(WsaData));
  إذا (WSAStartup(MAKEWORD(2, 2), &WsaData) != 0) {
    llvm::report_fatal_error("WSAStartup failed");
  }
}

WSABalancer::~WSABalancer() { WSACleanup(); }
#endif // _WIN32

ساكن std::error_code getLastSocketErrorCode() {
#ifdef _WIN32
  إرجاع std::error_code(::WSAGetLastError(), std::system_category());
#وإلا
  إرجاع errnoAsErrorCode();
#endif
}

#ifdef _WIN32
using NativeSocket = SOCKET;
#وإلا
using NativeSocket = صحيح;
#define INVALID_SOCKET -1
#endif

ساكن صحيح closeSocket(NativeSocket Socket) {
#ifdef _WIN32
  إرجاع ::closesocket(Socket);
#وإلا
  إرجاع ::close(Socket);
#endif
}

ساكن Expected<sockaddr_un> setSocketAddr(StringRef SocketPath) {
  بنية sockaddr_un Addr;
  memset(&Addr, 0, حجم(Addr));
  Addr.sun_family = AF_UNIX;

  إذا (حجم(sockaddr_un::sun_path) <= SocketPath.size())
    إرجاع make_error<StringError>(
        std::make_error_code(std::errc::filename_too_long),
        "Socket path exceeds sockaddr_un::sun_path size limit");

  strncpy(Addr.sun_path, SocketPath.str().c_str(), حجم(Addr.sun_path) - 1);
  إرجاع Addr;
}

ساكن Expected<صحيح> getSocketFD(StringRef SocketPath) {
  NativeSocket Socket = socket(AF_UNIX, SOCK_STREAM, 0);
  إذا (Socket == INVALID_SOCKET) {
    إرجاع llvm::make_error<StringError>(getLastSocketErrorCode(),
                                         "Create socket failed");
  }

#ifdef __CYGWIN__
  // On Cygwin, UNIX sockets involve a handshake between connect and accept
  // to enable SO_PEERCRED/getpeereid handling.  This necessitates accept being
  // called before connect can return, but at least the tests in
  // llvm/unittests/Support/raw_socket_stream_test do both on the same thread
  // (first connect and then accept), resulting in a deadlock.  This call turns
  // off the handshake (and SO_PEERCRED/getpeereid support).
  setsockopt(Socket, SOL_SOCKET, SO_PEERCRED, عدم, 0);
#endif
  Expected<بنية sockaddr_un> Addr = setSocketAddr(SocketPath);
  إذا (!Addr) {
    closeSocket(Socket);
    إرجاع Addr.takeError();
  }

  إذا (::connect(Socket, (بنية sockaddr *)&*Addr, حجم(*Addr)) == -1) {
    // Grab the error code before closing, which may overwrite it.
    std::error_code EC = getLastSocketErrorCode();
    closeSocket(Socket);
    إرجاع llvm::make_error<StringError>(EC, "Connect socket failed");
  }

#ifdef _WIN32
  إرجاع _open_osfhandle(Socket, 0);
#وإلا
  إرجاع Socket;
#endif // _WIN32
}

ListeningSocket::ListeningSocket(صحيح SocketFD, StringRef SocketPath,
                                 صحيح PipeFD[2])
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
                                                      صحيح MaxBacklog) {

  // Handle instances where the target socket address already exists and
  // differentiate between a preexisting file with and without a bound socket
  //
  // ::bind will return std::errc:address_in_use if a file at the socket address
  // already exists (e.g., the file was not properly unlinked due to a crash)
  // even if another socket has not yet binded to that address
  إذا (llvm::sys::fs::exists(SocketPath)) {
    Expected<صحيح> MaybeFD = getSocketFD(SocketPath);
    إذا (!MaybeFD) {

      // Regardless of the error, notify the caller that a file already exists
      // at the desired socket address and that there is no bound socket at that
      // address. The file must be removed before ::bind can use the address
      consumeError(MaybeFD.takeError());
      إرجاع llvm::make_error<StringError>(
          std::make_error_code(std::errc::file_exists),
          "Socket address unavailable");
    }
    ::close(std::move(*MaybeFD));

    // Notify caller that the provided socket address already has a bound socket
    إرجاع llvm::make_error<StringError>(
        std::make_error_code(std::errc::address_in_use),
        "Socket address unavailable");
  }

#ifdef _WIN32
  WSABalancer _;
#endif
  NativeSocket Socket = socket(AF_UNIX, SOCK_STREAM, 0);
  إذا (Socket == INVALID_SOCKET)
    إرجاع llvm::make_error<StringError>(getLastSocketErrorCode(),
                                         "socket create failed");

#ifdef __CYGWIN__
  // On Cygwin, UNIX sockets involve a handshake between connect and accept
  // to enable SO_PEERCRED/getpeereid handling.  This necessitates accept being
  // called before connect can return, but at least the tests in
  // llvm/unittests/Support/raw_socket_stream_test do both on the same thread
  // (first connect and then accept), resulting in a deadlock.  This call turns
  // off the handshake (and SO_PEERCRED/getpeereid support).
  setsockopt(Socket, SOL_SOCKET, SO_PEERCRED, عدم, 0);
#endif
  Expected<بنية sockaddr_un> Addr = setSocketAddr(SocketPath);
  إذا (!Addr) {
    closeSocket(Socket);
    إرجاع Addr.takeError();
  }

  إذا (::bind(Socket, (بنية sockaddr *)&*Addr, حجم(*Addr)) == -1) {
    // Grab error code from call to ::bind before closing the socket
    std::error_code EC = getLastSocketErrorCode();
    closeSocket(Socket);
    إرجاع llvm::make_error<StringError>(EC, "Bind error");
  }

  // Mark socket as passive so incoming connections can be accepted
  إذا (::listen(Socket, MaxBacklog) == -1)
    إرجاع llvm::make_error<StringError>(getLastSocketErrorCode(),
                                         "Listen error");

  صحيح PipeFD[2];
#ifdef _WIN32
  // Reserve 1 byte for the pipe and use default textmode
  إذا (::_pipe(PipeFD, 1, 0) == -1)
#وإلا
  إذا (::أنبوب(PipeFD) == -1)
#endif // _WIN32
    إرجاع llvm::make_error<StringError>(getLastSocketErrorCode(),
                                         "pipe failed");

#ifdef _WIN32
  إرجاع ListeningSocket{_open_osfhandle(Socket, 0), SocketPath, PipeFD};
#وإلا
  إرجاع ListeningSocket{Socket, SocketPath, PipeFD};
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
ساكن std::error_code
manageTimeout(ثابت std::chrono::milliseconds &Timeout,
              ثابت std::function<صحيح()> &getActiveFD,
              ثابت std::optional<صحيح> &CancelFD = std::nullopt) {
  بنية pollfd FD[2];
  FD[0].events = POLLIN;
#ifdef _WIN32
  SOCKET WinServerSock = _get_osfhandle(getActiveFD());
  FD[0].fd = WinServerSock;
#وإلا
  FD[0].fd = getActiveFD();
#endif
  uint8_t FDCount = 1;
  إذا (CancelFD.has_قيمة_2()) {
    FD[1].events = POLLIN;
    FD[1].fd = CancelFD.قيمة_2();
    FDCount++;
  }

  // Keep track of how much time has passed in case ::poll or WSAPoll are
  // interupted by a signal and need to be recalled
  auto Start = std::chrono::steady_clock::now();
  auto RemainingTimeout = Timeout;
  صحيح PollStatus = 0;
  افعل {
    // If Timeout is -1 then poll should block and RemainingTimeout does not
    // need to be recalculated
    إذا (PollStatus != 0 && Timeout != std::chrono::milliseconds(-1)) {
      auto TotalElapsedTime =
          std::chrono::duration_cast<std::chrono::milliseconds>(
              std::chrono::steady_clock::now() - Start);

      إذا (TotalElapsedTime >= Timeout)
        إرجاع std::make_error_code(std::errc::operation_would_block);

      RemainingTimeout = Timeout - TotalElapsedTime;
    }
#ifdef _WIN32
    PollStatus = WSAPoll(FD, FDCount, RemainingTimeout.تعداد_2());
  } بينما (PollStatus == SOCKET_ERROR &&
           getLastSocketErrorCode() == std::errc::interrupted);
#وإلا
    PollStatus = ::poll(FD, FDCount, RemainingTimeout.تعداد_2());
  } بينما (PollStatus == -1 &&
           getLastSocketErrorCode() == std::errc::interrupted);
#endif

  // If ActiveFD equals -1 or CancelFD has data to be read then the operation
  // has been canceled by another thread
  إذا (getActiveFD() == -1 || (CancelFD.has_قيمة_2() && FD[1].revents & POLLIN))
    إرجاع std::make_error_code(std::errc::operation_canceled);
#ifdef _WIN32
  إذا (PollStatus == SOCKET_ERROR)
#وإلا
  إذا (PollStatus == -1)
#endif
    إرجاع getLastSocketErrorCode();
  إذا (PollStatus == 0)
    إرجاع std::make_error_code(std::errc::timed_out);
  إذا (FD[0].revents & POLLNVAL)
    إرجاع std::make_error_code(std::errc::bad_file_descriptor);
  إرجاع std::error_code();
}

Expected<std::unique_ptr<raw_socket_stream>>
ListeningSocket::accept(ثابت std::chrono::milliseconds &Timeout) {
  auto getActiveFD = [هذا]() -> صحيح { إرجاع FD; };
  std::error_code TimeoutErr = manageTimeout(Timeout, getActiveFD, PipeFD[0]);
  إذا (TimeoutErr)
    إرجاع llvm::make_error<StringError>(TimeoutErr, "Timeout error");

  صحيح AcceptFD;
#ifdef _WIN32
  SOCKET WinAcceptSock = ::accept(_get_osfhandle(FD), عدم, عدم);
  AcceptFD = _open_osfhandle(WinAcceptSock, 0);
#وإلا
  AcceptFD = ::accept(FD, عدم, عدم);
#endif

  إذا (AcceptFD == -1)
    إرجاع llvm::make_error<StringError>(getLastSocketErrorCode(),
                                         "Socket accept failed");
  إرجاع std::make_unique<raw_socket_stream>(AcceptFD);
}

فراغ ListeningSocket::shutdown() {
  صحيح ObservedFD = FD.load();

  إذا (ObservedFD == -1)
    إرجاع;

  // If FD equals ObservedFD set FD to -1; If FD doesn't equal ObservedFD then
  // another thread is responsible for shutdown so return
  إذا (!FD.compare_exchange_strong(ObservedFD, -1))
    إرجاع;

  ::close(ObservedFD);
  ::unlink(SocketPath.c_str());

  // Ensure ::poll returns if shutdown is called by a separate thread
  حرف Byte = 'A';
  ssize_t written = ::اكتب(PipeFD[1], &Byte, 1);

  // Ignore any write() error
  (فراغ)written;
}

ListeningSocket::~ListeningSocket() {
  shutdown();

  // Close the pipe's FDs in the destructor instead of within
  // ListeningSocket::shutdown to avoid unnecessary synchronization issues that
  // would occur as PipeFD's values would have to be changed to -1
  //
  // The move constructor sets PipeFD to -1
  إذا (PipeFD[0] != -1)
    ::close(PipeFD[0]);
  إذا (PipeFD[1] != -1)
    ::close(PipeFD[1]);
}

//===----------------------------------------------------------------------===//
//  raw_socket_stream
//===----------------------------------------------------------------------===//

raw_socket_stream::raw_socket_stream(صحيح SocketFD)
    : raw_fd_stream(SocketFD, true) {}

raw_socket_stream::~raw_socket_stream() = افتراضي;

Expected<std::unique_ptr<raw_socket_stream>>
raw_socket_stream::createConnectedUnix(StringRef SocketPath) {
#ifdef _WIN32
  WSABalancer _;
#endif // _WIN32
  Expected<صحيح> FD = getSocketFD(SocketPath);
  إذا (!FD)
    إرجاع FD.takeError();
  إرجاع std::make_unique<raw_socket_stream>(*FD);
}

ssize_t raw_socket_stream::اقرأ(حرف *Ptr, size_t Size,
                                ثابت std::chrono::milliseconds &Timeout) {
  auto getActiveFD = [هذا]() -> صحيح { إرجاع هذا->get_fd(); };
  std::error_code Err = manageTimeout(Timeout, getActiveFD);
  // Mimic raw_fd_stream::read error handling behavior
  إذا (Err) {
    raw_fd_stream::error_detected(Err);
    إرجاع -1;
  }
  إرجاع raw_fd_stream::اقرأ(Ptr, Size);
}
