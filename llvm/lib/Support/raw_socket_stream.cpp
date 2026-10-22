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

#समावेश "llvm/Support/raw_socket_stream.h"
#समावेश "llvm/Config/config.h"
#समावेश "llvm/Support/Error.h"
#समावेश "llvm/Support/FileSystem.h"

#समावेश <atomic>
#समावेश <fcntl.h>
#समावेश <functional>

#अगर_अघोषित _WIN32
#समावेश <poll.h>
#समावेश <sys/socket.h>
#समावेश <sys/un.h>
#अन्यथा
#समावेश "llvm/Support/Windows/WindowsSupport.h"
// winsock2.h must be included before afunix.h. Briefly turn off clang-format to
// avoid error.
// clang-format off
#समावेश <winsock2.h>
#समावेश <afunix.h>
// clang-format on
#समावेश <io.h>
#पूर्ण_अगर // _WIN32

#अगर defined(HAVE_UNISTD_H)
#समावेश <unistd.h>
#पूर्ण_अगर

using namespace llvm;

#अगर_घोषित _WIN32
WSABalancer::WSABalancer() {
  WSADATA Wsa_तथ्य;
  ::स_रखो(&Wsa_तथ्य, 0, माप(Wsa_तथ्य));
  अगर (WSAStartup(MAKEWORD(2, 2), &Wsa_तथ्य) != 0) {
    llvm::report_fatal_error("WSAStartup failed");
  }
}

WSABalancer::~WSABalancer() { WSACleanup(); }
#पूर्ण_अगर // _WIN32

अटल std::error_code getLastSocketErrorCode() {
#अगर_घोषित _WIN32
  वापस std::error_code(::WSAGetLastError(), std::system_category());
#अन्यथा
  वापस errnoAsErrorCode();
#पूर्ण_अगर
}

#अगर_घोषित _WIN32
using NativeSocket = SOCKET;
#अन्यथा
using NativeSocket = पूर्णांक;
#घोषणा INVALID_SOCKET -1
#पूर्ण_अगर

अटल पूर्णांक closeSocket(NativeSocket Socket) {
#अगर_घोषित _WIN32
  वापस ::closesocket(Socket);
#अन्यथा
  वापस ::बंद(Socket);
#पूर्ण_अगर
}

अटल Expected<sockaddr_un> setSocketAddr(StringRef SocketPath) {
  काष्ठा sockaddr_un Addr;
  स_रखो(&Addr, 0, माप(Addr));
  Addr.sun_family = AF_UNIX;

  अगर (माप(sockaddr_un::sun_path) <= SocketPath.आकार_2())
    वापस make_error<StringError>(
        std::make_error_code(std::errc::filename_too_long),
        "Socket path exceeds sockaddr_un::sun_path size limit");

  म_नकलन(Addr.sun_path, SocketPath.str().c_str(), माप(Addr.sun_path) - 1);
  वापस Addr;
}

अटल Expected<पूर्णांक> getSocketFD(StringRef SocketPath) {
  NativeSocket Socket = socket(AF_UNIX, SOCK_STREAM, 0);
  अगर (Socket == INVALID_SOCKET) {
    वापस llvm::make_error<StringError>(getLastSocketErrorCode(),
                                         "Create socket failed");
  }

#अगर_घोषित __CYGWIN__
  // On Cygwin, UNIX sockets involve a handshake between connect and accept
  // to enable SO_PEERCRED/getpeereid handling.  This necessitates accept being
  // called before connect can return, but at least the tests in
  // llvm/unittests/Support/raw_socket_stream_test do both on the same thread
  // (first connect and then accept), resulting in a deadlock.  This call turns
  // off the handshake (and SO_PEERCRED/getpeereid support).
  setsockopt(Socket, SOL_SOCKET, SO_PEERCRED, शून्य, 0);
#पूर्ण_अगर
  Expected<काष्ठा sockaddr_un> Addr = setSocketAddr(SocketPath);
  अगर (!Addr) {
    closeSocket(Socket);
    वापस Addr.takeError();
  }

  अगर (::connect(Socket, (काष्ठा sockaddr *)&*Addr, माप(*Addr)) == -1) {
    // Grab the error code before closing, which may overwrite it.
    std::error_code EC = getLastSocketErrorCode();
    closeSocket(Socket);
    वापस llvm::make_error<StringError>(EC, "Connect socket failed");
  }

#अगर_घोषित _WIN32
  वापस _open_osfhandle(Socket, 0);
#अन्यथा
  वापस Socket;
#पूर्ण_अगर // _WIN32
}

ListeningSocket::ListeningSocket(पूर्णांक SocketFD, StringRef SocketPath,
                                 पूर्णांक PipeFD[2])
    : FD(SocketFD), SocketPath(SocketPath), PipeFD{PipeFD[0], PipeFD[1]} {}

ListeningSocket::ListeningSocket(ListeningSocket &&LS)
    : FD(LS.FD.load()), SocketPath(LS.SocketPath),
      PipeFD{LS.PipeFD[0], LS.PipeFD[1]} {

  LS.FD = -1;
  LS.SocketPath.सफा();
  LS.PipeFD[0] = -1;
  LS.PipeFD[1] = -1;
}

Expected<ListeningSocket> ListeningSocket::createUnix(StringRef SocketPath,
                                                      पूर्णांक MaxBacklog) {

  // Handle instances where the target socket address already exists and
  // differentiate between a preexisting file with and without a bound socket
  //
  // ::bind will return std::errc:address_in_use if a file at the socket address
  // already exists (e.g., the file was not properly unlinked due to a crash)
  // even if another socket has not yet binded to that address
  अगर (llvm::sys::fs::exists(SocketPath)) {
    Expected<पूर्णांक> MaybeFD = getSocketFD(SocketPath);
    अगर (!MaybeFD) {

      // Regardless of the error, notify the caller that a file already exists
      // at the desired socket address and that there is no bound socket at that
      // address. The file must be removed before ::bind can use the address
      consumeError(MaybeFD.takeError());
      वापस llvm::make_error<StringError>(
          std::make_error_code(std::errc::file_exists),
          "Socket address unavailable");
    }
    ::बंद(std::move(*MaybeFD));

    // Notify caller that the provided socket address already has a bound socket
    वापस llvm::make_error<StringError>(
        std::make_error_code(std::errc::address_in_use),
        "Socket address unavailable");
  }

#अगर_घोषित _WIN32
  WSABalancer _;
#पूर्ण_अगर
  NativeSocket Socket = socket(AF_UNIX, SOCK_STREAM, 0);
  अगर (Socket == INVALID_SOCKET)
    वापस llvm::make_error<StringError>(getLastSocketErrorCode(),
                                         "socket create failed");

#अगर_घोषित __CYGWIN__
  // On Cygwin, UNIX sockets involve a handshake between connect and accept
  // to enable SO_PEERCRED/getpeereid handling.  This necessitates accept being
  // called before connect can return, but at least the tests in
  // llvm/unittests/Support/raw_socket_stream_test do both on the same thread
  // (first connect and then accept), resulting in a deadlock.  This call turns
  // off the handshake (and SO_PEERCRED/getpeereid support).
  setsockopt(Socket, SOL_SOCKET, SO_PEERCRED, शून्य, 0);
#पूर्ण_अगर
  Expected<काष्ठा sockaddr_un> Addr = setSocketAddr(SocketPath);
  अगर (!Addr) {
    closeSocket(Socket);
    वापस Addr.takeError();
  }

  अगर (::bind(Socket, (काष्ठा sockaddr *)&*Addr, माप(*Addr)) == -1) {
    // Grab error code from call to ::bind before closing the socket
    std::error_code EC = getLastSocketErrorCode();
    closeSocket(Socket);
    वापस llvm::make_error<StringError>(EC, "Bind error");
  }

  // Mark socket as passive so incoming connections can be accepted
  अगर (::listen(Socket, MaxBacklog) == -1)
    वापस llvm::make_error<StringError>(getLastSocketErrorCode(),
                                         "Listen error");

  पूर्णांक PipeFD[2];
#अगर_घोषित _WIN32
  // Reserve 1 byte for the pipe and use default textmode
  अगर (::_pipe(PipeFD, 1, 0) == -1)
#अन्यथा
  अगर (::pipe(PipeFD) == -1)
#पूर्ण_अगर // _WIN32
    वापस llvm::make_error<StringError>(getLastSocketErrorCode(),
                                         "pipe failed");

#अगर_घोषित _WIN32
  वापस ListeningSocket{_open_osfhandle(Socket, 0), SocketPath, PipeFD};
#अन्यथा
  वापस ListeningSocket{Socket, SocketPath, PipeFD};
#पूर्ण_अगर // _WIN32
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
अटल std::error_code
manageTimeout(स्थिर std::chrono::milliseconds &Timeout,
              स्थिर std::function<पूर्णांक()> &getActiveFD,
              स्थिर std::optional<पूर्णांक> &CancelFD = std::nullopt) {
  काष्ठा pollfd FD[2];
  FD[0].events = POLLIN;
#अगर_घोषित _WIN32
  SOCKET WinServerSock = _get_osfhandle(getActiveFD());
  FD[0].fd = WinServerSock;
#अन्यथा
  FD[0].fd = getActiveFD();
#पूर्ण_अगर
  uint8_t FDCount = 1;
  अगर (CancelFD.has_मूल्य_2()) {
    FD[1].events = POLLIN;
    FD[1].fd = CancelFD.मूल्य_2();
    FDCount++;
  }

  // Keep track of how much time has passed in case ::poll or WSAPoll are
  // interupted by a signal and need to be recalled
  स्वतः Start = std::chrono::steady_clock::now();
  स्वतः RemainingTimeout = Timeout;
  पूर्णांक PollStatus = 0;
  करो {
    // If Timeout is -1 then poll should block and RemainingTimeout does not
    // need to be recalculated
    अगर (PollStatus != 0 && Timeout != std::chrono::milliseconds(-1)) {
      स्वतः योग_Elapsed_Time =
          std::chrono::duration_cast<std::chrono::milliseconds>(
              std::chrono::steady_clock::now() - Start);

      अगर (योग_Elapsed_Time >= Timeout)
        वापस std::make_error_code(std::errc::operation_would_block);

      RemainingTimeout = Timeout - योग_Elapsed_Time;
    }
#अगर_घोषित _WIN32
    PollStatus = WSAPoll(FD, FDCount, RemainingTimeout.गिनती());
  } जबतक (PollStatus == SOCKET_ERROR &&
           getLastSocketErrorCode() == std::errc::interrupted);
#अन्यथा
    PollStatus = ::poll(FD, FDCount, RemainingTimeout.गिनती());
  } जबतक (PollStatus == -1 &&
           getLastSocketErrorCode() == std::errc::interrupted);
#पूर्ण_अगर

  // If ActiveFD equals -1 or CancelFD has data to be read then the operation
  // has been canceled by another thread
  अगर (getActiveFD() == -1 || (CancelFD.has_मूल्य_2() && FD[1].revents & POLLIN))
    वापस std::make_error_code(std::errc::operation_canceled);
#अगर_घोषित _WIN32
  अगर (PollStatus == SOCKET_ERROR)
#अन्यथा
  अगर (PollStatus == -1)
#पूर्ण_अगर
    वापस getLastSocketErrorCode();
  अगर (PollStatus == 0)
    वापस std::make_error_code(std::errc::timed_out);
  अगर (FD[0].revents & POLLNVAL)
    वापस std::make_error_code(std::errc::bad_file_descriptor);
  वापस std::error_code();
}

Expected<std::unique_ptr<raw_socket_stream>>
ListeningSocket::accept(स्थिर std::chrono::milliseconds &Timeout) {
  स्वतः getActiveFD = [यह]() -> पूर्णांक { वापस FD; };
  std::error_code TimeoutErr = manageTimeout(Timeout, getActiveFD, PipeFD[0]);
  अगर (TimeoutErr)
    वापस llvm::make_error<StringError>(TimeoutErr, "Timeout error");

  पूर्णांक AcceptFD;
#अगर_घोषित _WIN32
  SOCKET WinAcceptSock = ::accept(_get_osfhandle(FD), शून्य, शून्य);
  AcceptFD = _open_osfhandle(WinAcceptSock, 0);
#अन्यथा
  AcceptFD = ::accept(FD, शून्य, शून्य);
#पूर्ण_अगर

  अगर (AcceptFD == -1)
    वापस llvm::make_error<StringError>(getLastSocketErrorCode(),
                                         "Socket accept failed");
  वापस std::make_unique<raw_socket_stream>(AcceptFD);
}

व्योम ListeningSocket::shutdown() {
  पूर्णांक ObservedFD = FD.load();

  अगर (ObservedFD == -1)
    वापस;

  // If FD equals ObservedFD set FD to -1; If FD doesn't equal ObservedFD then
  // another thread is responsible for shutdown so return
  अगर (!FD.compare_exchange_strong(ObservedFD, -1))
    वापस;

  ::बंद(ObservedFD);
  ::unlink(SocketPath.c_str());

  // Ensure ::poll returns if shutdown is called by a separate thread
  अक्षर Byte = 'A';
  ssize_t written = ::ग_लिखो(PipeFD[1], &Byte, 1);

  // Ignore any write() error
  (व्योम)written;
}

ListeningSocket::~ListeningSocket() {
  shutdown();

  // Close the pipe's FDs in the destructor instead of within
  // ListeningSocket::shutdown to avoid unnecessary synchronization issues that
  // would occur as PipeFD's values would have to be changed to -1
  //
  // The move constructor sets PipeFD to -1
  अगर (PipeFD[0] != -1)
    ::बंद(PipeFD[0]);
  अगर (PipeFD[1] != -1)
    ::बंद(PipeFD[1]);
}

//===----------------------------------------------------------------------===//
//  raw_socket_stream
//===----------------------------------------------------------------------===//

raw_socket_stream::raw_socket_stream(पूर्णांक SocketFD)
    : raw_fd_stream(SocketFD, true) {}

raw_socket_stream::~raw_socket_stream() = शेष;

Expected<std::unique_ptr<raw_socket_stream>>
raw_socket_stream::createConnectedUnix(StringRef SocketPath) {
#अगर_घोषित _WIN32
  WSABalancer _;
#पूर्ण_अगर // _WIN32
  Expected<पूर्णांक> FD = getSocketFD(SocketPath);
  अगर (!FD)
    वापस FD.takeError();
  वापस std::make_unique<raw_socket_stream>(*FD);
}

ssize_t raw_socket_stream::पढ़ो(अक्षर *Ptr, माप_प्रकार आकार,
                                स्थिर std::chrono::milliseconds &Timeout) {
  स्वतः getActiveFD = [यह]() -> पूर्णांक { वापस यह->get_fd(); };
  std::error_code Err = manageTimeout(Timeout, getActiveFD);
  // Mimic raw_fd_stream::read error handling behavior
  अगर (Err) {
    raw_fd_stream::error_detected(Err);
    वापस -1;
  }
  वापस raw_fd_stream::पढ़ो(Ptr, आकार);
}
