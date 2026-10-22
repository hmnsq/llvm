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

#अन्तर्भाव "llvm/Support/raw_socket_stream.h"
#अन्तर्भाव "llvm/Config/config.h"
#अन्तर्भाव "llvm/Support/Error.h"
#अन्तर्भाव "llvm/Support/FileSystem.h"

#अन्तर्भाव <atomic>
#अन्तर्भाव <fcntl.h>
#अन्तर्भाव <functional>

#ifndef _WIN32
#अन्तर्भाव <poll.h>
#अन्तर्भाव <sys/socket.h>
#अन्तर्भाव <sys/un.h>
#अन्यथा
#अन्तर्भाव "llvm/Support/Windows/WindowsSupport.h"
// winsock2.h must be included before afunix.h. Briefly turn off clang-format to
// avoid error.
// clang-format off
#अन्तर्भाव <winsock2.h>
#अन्तर्भाव <afunix.h>
// clang-format on
#अन्तर्भाव <io.h>
#endif // _WIN32

#यदि defined(HAVE_UNISTD_H)
#अन्तर्भाव <unistd.h>
#endif

using नामाकाश llvm;

#ifdef _WIN32
WSABalancer::WSABalancer() {
  WSADATA WsaData;
  ::memset(&WsaData, 0, परिमाण(WsaData));
  यदि (WSAStartup(MAKEWORD(2, 2), &WsaData) != 0) {
    llvm::report_fatal_error("WSAStartup failed");
  }
}

WSABalancer::~WSABalancer() { WSACleanup(); }
#endif // _WIN32

स्थिर std::error_code getLastSocketErrorCode() {
#ifdef _WIN32
  प्रत्यागम std::error_code(::WSAGetLastError(), std::system_category());
#अन्यथा
  प्रत्यागम errnoAsErrorCode();
#endif
}

#ifdef _WIN32
using NativeSocket = SOCKET;
#अन्यथा
using NativeSocket = पूर्णाङ्क;
#define INVALID_SOCKET -1
#endif

स्थिर पूर्णाङ्क closeSocket(NativeSocket Socket) {
#ifdef _WIN32
  प्रत्यागम ::closesocket(Socket);
#अन्यथा
  प्रत्यागम ::close(Socket);
#endif
}

स्थिर Expected<sockaddr_un> setSocketAddr(StringRef SocketPath) {
  संरचना sockaddr_un Addr;
  memset(&Addr, 0, परिमाण(Addr));
  Addr.sun_family = AF_UNIX;

  यदि (परिमाण(sockaddr_un::sun_path) <= SocketPath.size())
    प्रत्यागम make_error<StringError>(
        std::make_error_code(std::errc::filename_too_long),
        "Socket path exceeds sockaddr_un::sun_path size limit");

  strncpy(Addr.sun_path, SocketPath.str().c_str(), परिमाण(Addr.sun_path) - 1);
  प्रत्यागम Addr;
}

स्थिर Expected<पूर्णाङ्क> getSocketFD(StringRef SocketPath) {
  NativeSocket Socket = socket(AF_UNIX, SOCK_STREAM, 0);
  यदि (Socket == INVALID_SOCKET) {
    प्रत्यागम llvm::make_error<StringError>(getLastSocketErrorCode(),
                                         "Create socket failed");
  }

#ifdef __CYGWIN__
  // On Cygwin, UNIX sockets involve a handshake between connect and accept
  // to enable SO_PEERCRED/getpeereid handling.  This necessitates accept being
  // called before connect can return, but at least the tests in
  // llvm/unittests/Support/raw_socket_stream_test do both on the same thread
  // (first connect and then accept), resulting in a deadlock.  This call turns
  // off the handshake (and SO_PEERCRED/getpeereid support).
  setsockopt(Socket, SOL_SOCKET, SO_PEERCRED, रिक्त, 0);
#endif
  Expected<संरचना sockaddr_un> Addr = setSocketAddr(SocketPath);
  यदि (!Addr) {
    closeSocket(Socket);
    प्रत्यागम Addr.takeError();
  }

  यदि (::connect(Socket, (संरचना sockaddr *)&*Addr, परिमाण(*Addr)) == -1) {
    // Grab the error code before closing, which may overwrite it.
    std::error_code EC = getLastSocketErrorCode();
    closeSocket(Socket);
    प्रत्यागम llvm::make_error<StringError>(EC, "Connect socket failed");
  }

#ifdef _WIN32
  प्रत्यागम _open_osfhandle(Socket, 0);
#अन्यथा
  प्रत्यागम Socket;
#endif // _WIN32
}

ListeningSocket::ListeningSocket(पूर्णाङ्क SocketFD, StringRef SocketPath,
                                 पूर्णाङ्क PipeFD[2])
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
                                                      पूर्णाङ्क MaxBacklog) {

  // Handle instances where the target socket address already exists and
  // differentiate between a preexisting file with and without a bound socket
  //
  // ::bind will return std::errc:address_in_use if a file at the socket address
  // already exists (e.g., the file was not properly unlinked due to a crash)
  // even if another socket has not yet binded to that address
  यदि (llvm::sys::fs::exists(SocketPath)) {
    Expected<पूर्णाङ्क> MaybeFD = getSocketFD(SocketPath);
    यदि (!MaybeFD) {

      // Regardless of the error, notify the caller that a file already exists
      // at the desired socket address and that there is no bound socket at that
      // address. The file must be removed before ::bind can use the address
      consumeError(MaybeFD.takeError());
      प्रत्यागम llvm::make_error<StringError>(
          std::make_error_code(std::errc::file_exists),
          "Socket address unavailable");
    }
    ::close(std::move(*MaybeFD));

    // Notify caller that the provided socket address already has a bound socket
    प्रत्यागम llvm::make_error<StringError>(
        std::make_error_code(std::errc::address_in_use),
        "Socket address unavailable");
  }

#ifdef _WIN32
  WSABalancer _;
#endif
  NativeSocket Socket = socket(AF_UNIX, SOCK_STREAM, 0);
  यदि (Socket == INVALID_SOCKET)
    प्रत्यागम llvm::make_error<StringError>(getLastSocketErrorCode(),
                                         "socket create failed");

#ifdef __CYGWIN__
  // On Cygwin, UNIX sockets involve a handshake between connect and accept
  // to enable SO_PEERCRED/getpeereid handling.  This necessitates accept being
  // called before connect can return, but at least the tests in
  // llvm/unittests/Support/raw_socket_stream_test do both on the same thread
  // (first connect and then accept), resulting in a deadlock.  This call turns
  // off the handshake (and SO_PEERCRED/getpeereid support).
  setsockopt(Socket, SOL_SOCKET, SO_PEERCRED, रिक्त, 0);
#endif
  Expected<संरचना sockaddr_un> Addr = setSocketAddr(SocketPath);
  यदि (!Addr) {
    closeSocket(Socket);
    प्रत्यागम Addr.takeError();
  }

  यदि (::bind(Socket, (संरचना sockaddr *)&*Addr, परिमाण(*Addr)) == -1) {
    // Grab error code from call to ::bind before closing the socket
    std::error_code EC = getLastSocketErrorCode();
    closeSocket(Socket);
    प्रत्यागम llvm::make_error<StringError>(EC, "Bind error");
  }

  // Mark socket as passive so incoming connections can be accepted
  यदि (::listen(Socket, MaxBacklog) == -1)
    प्रत्यागम llvm::make_error<StringError>(getLastSocketErrorCode(),
                                         "Listen error");

  पूर्णाङ्क PipeFD[2];
#ifdef _WIN32
  // Reserve 1 byte for the pipe and use default textmode
  यदि (::_pipe(PipeFD, 1, 0) == -1)
#अन्यथा
  यदि (::प्रणाल(PipeFD) == -1)
#endif // _WIN32
    प्रत्यागम llvm::make_error<StringError>(getLastSocketErrorCode(),
                                         "pipe failed");

#ifdef _WIN32
  प्रत्यागम ListeningSocket{_open_osfhandle(Socket, 0), SocketPath, PipeFD};
#अन्यथा
  प्रत्यागम ListeningSocket{Socket, SocketPath, PipeFD};
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
स्थिर std::error_code
manageTimeout(ध्रुव std::chrono::milliseconds &Timeout,
              ध्रुव std::function<पूर्णाङ्क()> &getActiveFD,
              ध्रुव std::optional<पूर्णाङ्क> &CancelFD = std::nullopt) {
  संरचना pollfd FD[2];
  FD[0].events = POLLIN;
#ifdef _WIN32
  SOCKET WinServerSock = _get_osfhandle(getActiveFD());
  FD[0].fd = WinServerSock;
#अन्यथा
  FD[0].fd = getActiveFD();
#endif
  uint8_t FDCount = 1;
  यदि (CancelFD.has_मूल्य_2()) {
    FD[1].events = POLLIN;
    FD[1].fd = CancelFD.मूल्य_2();
    FDCount++;
  }

  // Keep track of how much time has passed in case ::poll or WSAPoll are
  // interupted by a signal and need to be recalled
  auto Start = std::chrono::steady_clock::now();
  auto RemainingTimeout = Timeout;
  पूर्णाङ्क PollStatus = 0;
  कुरु {
    // If Timeout is -1 then poll should block and RemainingTimeout does not
    // need to be recalculated
    यदि (PollStatus != 0 && Timeout != std::chrono::milliseconds(-1)) {
      auto TotalElapsedTime =
          std::chrono::duration_cast<std::chrono::milliseconds>(
              std::chrono::steady_clock::now() - Start);

      यदि (TotalElapsedTime >= Timeout)
        प्रत्यागम std::make_error_code(std::errc::operation_would_block);

      RemainingTimeout = Timeout - TotalElapsedTime;
    }
#ifdef _WIN32
    PollStatus = WSAPoll(FD, FDCount, RemainingTimeout.गिनती());
  } यावत् (PollStatus == SOCKET_ERROR &&
           getLastSocketErrorCode() == std::errc::interrupted);
#अन्यथा
    PollStatus = ::poll(FD, FDCount, RemainingTimeout.गिनती());
  } यावत् (PollStatus == -1 &&
           getLastSocketErrorCode() == std::errc::interrupted);
#endif

  // If ActiveFD equals -1 or CancelFD has data to be read then the operation
  // has been canceled by another thread
  यदि (getActiveFD() == -1 || (CancelFD.has_मूल्य_2() && FD[1].revents & POLLIN))
    प्रत्यागम std::make_error_code(std::errc::operation_canceled);
#ifdef _WIN32
  यदि (PollStatus == SOCKET_ERROR)
#अन्यथा
  यदि (PollStatus == -1)
#endif
    प्रत्यागम getLastSocketErrorCode();
  यदि (PollStatus == 0)
    प्रत्यागम std::make_error_code(std::errc::timed_out);
  यदि (FD[0].revents & POLLNVAL)
    प्रत्यागम std::make_error_code(std::errc::bad_file_descriptor);
  प्रत्यागम std::error_code();
}

Expected<std::unique_ptr<raw_socket_stream>>
ListeningSocket::accept(ध्रुव std::chrono::milliseconds &Timeout) {
  auto getActiveFD = [एतत्]() -> पूर्णाङ्क { प्रत्यागम FD; };
  std::error_code TimeoutErr = manageTimeout(Timeout, getActiveFD, PipeFD[0]);
  यदि (TimeoutErr)
    प्रत्यागम llvm::make_error<StringError>(TimeoutErr, "Timeout error");

  पूर्णाङ्क AcceptFD;
#ifdef _WIN32
  SOCKET WinAcceptSock = ::accept(_get_osfhandle(FD), रिक्त, रिक्त);
  AcceptFD = _open_osfhandle(WinAcceptSock, 0);
#अन्यथा
  AcceptFD = ::accept(FD, रिक्त, रिक्त);
#endif

  यदि (AcceptFD == -1)
    प्रत्यागम llvm::make_error<StringError>(getLastSocketErrorCode(),
                                         "Socket accept failed");
  प्रत्यागम std::make_unique<raw_socket_stream>(AcceptFD);
}

शून्य ListeningSocket::shutdown() {
  पूर्णाङ्क ObservedFD = FD.load();

  यदि (ObservedFD == -1)
    प्रत्यागम;

  // If FD equals ObservedFD set FD to -1; If FD doesn't equal ObservedFD then
  // another thread is responsible for shutdown so return
  यदि (!FD.compare_exchange_strong(ObservedFD, -1))
    प्रत्यागम;

  ::close(ObservedFD);
  ::unlink(SocketPath.c_str());

  // Ensure ::poll returns if shutdown is called by a separate thread
  वर्ण Byte = 'A';
  ssize_t written = ::लिख(PipeFD[1], &Byte, 1);

  // Ignore any write() error
  (शून्य)written;
}

ListeningSocket::~ListeningSocket() {
  shutdown();

  // Close the pipe's FDs in the destructor instead of within
  // ListeningSocket::shutdown to avoid unnecessary synchronization issues that
  // would occur as PipeFD's values would have to be changed to -1
  //
  // The move constructor sets PipeFD to -1
  यदि (PipeFD[0] != -1)
    ::close(PipeFD[0]);
  यदि (PipeFD[1] != -1)
    ::close(PipeFD[1]);
}

//===----------------------------------------------------------------------===//
//  raw_socket_stream
//===----------------------------------------------------------------------===//

raw_socket_stream::raw_socket_stream(पूर्णाङ्क SocketFD)
    : raw_fd_stream(SocketFD, सत्य) {}

raw_socket_stream::~raw_socket_stream() = स्वभाव;

Expected<std::unique_ptr<raw_socket_stream>>
raw_socket_stream::createConnectedUnix(StringRef SocketPath) {
#ifdef _WIN32
  WSABalancer _;
#endif // _WIN32
  Expected<पूर्णाङ्क> FD = getSocketFD(SocketPath);
  यदि (!FD)
    प्रत्यागम FD.takeError();
  प्रत्यागम std::make_unique<raw_socket_stream>(*FD);
}

ssize_t raw_socket_stream::पठ(वर्ण *Ptr, size_t Size,
                                ध्रुव std::chrono::milliseconds &Timeout) {
  auto getActiveFD = [एतत्]() -> पूर्णाङ्क { प्रत्यागम एतत्->get_fd(); };
  std::error_code Err = manageTimeout(Timeout, getActiveFD);
  // Mimic raw_fd_stream::read error handling behavior
  यदि (Err) {
    raw_fd_stream::error_detected(Err);
    प्रत्यागम -1;
  }
  प्रत्यागम raw_fd_stream::पठ(Ptr, Size);
}
