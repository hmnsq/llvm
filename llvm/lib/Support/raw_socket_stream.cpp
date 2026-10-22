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

#यदि_अपरिभाषितम् _WIN32
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
#यदि_अन्तः // _WIN32

#यदि defined(HAVE_UNISTD_H)
#अन्तर्भाव <unistd.h>
#यदि_अन्तः

उपयुज्य नामाकाश llvm;

#यदि_परिभाषितम् _WIN32
WSABalancer::WSABalancer() {
  WSADATA Wsa_दत्त;
  ::स्मृतिं_पूरय(&Wsa_दत्त, 0, परिमाण(Wsa_दत्त));
  यदि (WSAStartup(MAKEWORD(2, 2), &Wsa_दत्त) != 0) {
    llvm::report_fatal_error("WSAStartup failed");
  }
}

WSABalancer::~WSABalancer() { WSACleanup(); }
#यदि_अन्तः // _WIN32

स्थिर std::error_संहिता get_चरमः_Socket_Error_संहिता() {
#यदि_परिभाषितम् _WIN32
  प्रत्यागम std::error_संहिता(::WSAGet_चरमः_Error(), std::system_category());
#अन्यथा
  प्रत्यागम errno_As_Error_संहिता();
#यदि_अन्तः
}

#यदि_परिभाषितम् _WIN32
उपयुज्य NativeSocket = SOCKET;
#अन्यथा
उपयुज्य NativeSocket = पूर्णाङ्क;
#परिभाषय INVALID_SOCKET -1
#यदि_अन्तः

स्थिर पूर्णाङ्क closeSocket(NativeSocket Socket) {
#यदि_परिभाषितम् _WIN32
  प्रत्यागम ::closesocket(Socket);
#अन्यथा
  प्रत्यागम ::close(Socket);
#यदि_अन्तः
}

स्थिर Expected<sockaddr_un> समुच्चयः_Socket_Addr(StringRef Socket_मार्गः) {
  संरचना sockaddr_un Addr;
  स्मृतिं_पूरय(&Addr, 0, परिमाण(Addr));
  Addr.sun_family = AF_UNIX;

  यदि (परिमाण(sockaddr_un::sun_मार्गः) <= Socket_मार्गः.आकार_2())
    प्रत्यागम make_error<StringError>(
        std::make_error_संहिता(std::errc::filename_too_long),
        "Socket path exceeds sockaddr_un::sun_path size limit");

  strncpy(Addr.sun_मार्गः, Socket_मार्गः.str().c_str(), परिमाण(Addr.sun_मार्गः) - 1);
  प्रत्यागम Addr;
}

स्थिर Expected<पूर्णाङ्क> getSocketFD(StringRef Socket_मार्गः) {
  NativeSocket Socket = socket(AF_UNIX, SOCK_प्रवाहः, 0);
  यदि (Socket == INVALID_SOCKET) {
    प्रत्यागम llvm::make_error<StringError>(get_चरमः_Socket_Error_संहिता(),
                                         "Create socket failed");
  }

#यदि_परिभाषितम् __CYGWIN__
  // On Cygwin, UNIX sockets involve a handshake between connect and accept
  // to enable SO_PEERCRED/getpeereid handling.  This necessitates accept being
  // called before connect can return, but at least the tests in
  // llvm/unittests/Support/raw_socket_stream_test do both on the same thread
  // (first connect and then accept), resulting in a deadlock.  This call turns
  // off the handshake (and SO_PEERCRED/getpeereid support).
  setsockopt(Socket, SOL_SOCKET, SO_PEERCRED, रिक्त, 0);
#यदि_अन्तः
  Expected<संरचना sockaddr_un> Addr = समुच्चयः_Socket_Addr(Socket_मार्गः);
  यदि (!Addr) {
    closeSocket(Socket);
    प्रत्यागम Addr.takeError();
  }

  यदि (::connect(Socket, (संरचना sockaddr *)&*Addr, परिमाण(*Addr)) == -1) {
    // Grab the error code before closing, which may overwrite it.
    std::error_संहिता EC = get_चरमः_Socket_Error_संहिता();
    closeSocket(Socket);
    प्रत्यागम llvm::make_error<StringError>(EC, "Connect socket failed");
  }

#यदि_परिभाषितम् _WIN32
  प्रत्यागम _open_osfhandle(Socket, 0);
#अन्यथा
  प्रत्यागम Socket;
#यदि_अन्तः // _WIN32
}

ListeningSocket::ListeningSocket(पूर्णाङ्क SocketFD, StringRef Socket_मार्गः,
                                 पूर्णाङ्क PipeFD[2])
    : FD(SocketFD), Socket_मार्गः(Socket_मार्गः), PipeFD{PipeFD[0], PipeFD[1]} {}

ListeningSocket::ListeningSocket(ListeningSocket &&LS)
    : FD(LS.FD.आरोपय()), Socket_मार्गः(LS.Socket_मार्गः),
      PipeFD{LS.PipeFD[0], LS.PipeFD[1]} {

  LS.FD = -1;
  LS.Socket_मार्गः.clear();
  LS.PipeFD[0] = -1;
  LS.PipeFD[1] = -1;
}

Expected<ListeningSocket> ListeningSocket::createUnix(StringRef Socket_मार्गः,
                                                      पूर्णाङ्क MaxBacklog) {

  // Handle instances where the target socket address already exists and
  // differentiate between a preexisting file with and without a bound socket
  //
  // ::bind will return std::errc:address_in_use if a file at the socket address
  // already exists (e.g., the file was not properly unlinked due to a crash)
  // even if another socket has not yet binded to that address
  यदि (llvm::sys::fs::exists(Socket_मार्गः)) {
    Expected<पूर्णाङ्क> MaybeFD = getSocketFD(Socket_मार्गः);
    यदि (!MaybeFD) {

      // Regardless of the error, notify the caller that a file already exists
      // at the desired socket address and that there is no bound socket at that
      // address. The file must be removed before ::bind can use the address
      consumeError(MaybeFD.takeError());
      प्रत्यागम llvm::make_error<StringError>(
          std::make_error_संहिता(std::errc::file_exists),
          "Socket address unavailable");
    }
    ::close(std::move(*MaybeFD));

    // Notify caller that the provided socket address already has a bound socket
    प्रत्यागम llvm::make_error<StringError>(
        std::make_error_संहिता(std::errc::स्थानाङ्कः_अन्तर्_use),
        "Socket address unavailable");
  }

#यदि_परिभाषितम् _WIN32
  WSABalancer _;
#यदि_अन्तः
  NativeSocket Socket = socket(AF_UNIX, SOCK_प्रवाहः, 0);
  यदि (Socket == INVALID_SOCKET)
    प्रत्यागम llvm::make_error<StringError>(get_चरमः_Socket_Error_संहिता(),
                                         "socket create failed");

#यदि_परिभाषितम् __CYGWIN__
  // On Cygwin, UNIX sockets involve a handshake between connect and accept
  // to enable SO_PEERCRED/getpeereid handling.  This necessitates accept being
  // called before connect can return, but at least the tests in
  // llvm/unittests/Support/raw_socket_stream_test do both on the same thread
  // (first connect and then accept), resulting in a deadlock.  This call turns
  // off the handshake (and SO_PEERCRED/getpeereid support).
  setsockopt(Socket, SOL_SOCKET, SO_PEERCRED, रिक्त, 0);
#यदि_अन्तः
  Expected<संरचना sockaddr_un> Addr = समुच्चयः_Socket_Addr(Socket_मार्गः);
  यदि (!Addr) {
    closeSocket(Socket);
    प्रत्यागम Addr.takeError();
  }

  यदि (::bind(Socket, (संरचना sockaddr *)&*Addr, परिमाण(*Addr)) == -1) {
    // Grab error code from call to ::bind before closing the socket
    std::error_संहिता EC = get_चरमः_Socket_Error_संहिता();
    closeSocket(Socket);
    प्रत्यागम llvm::make_error<StringError>(EC, "Bind error");
  }

  // Mark socket as passive so incoming connections can be accepted
  यदि (::listen(Socket, MaxBacklog) == -1)
    प्रत्यागम llvm::make_error<StringError>(get_चरमः_Socket_Error_संहिता(),
                                         "Listen error");

  पूर्णाङ्क PipeFD[2];
#यदि_परिभाषितम् _WIN32
  // Reserve 1 byte for the pipe and use default textmode
  यदि (::_pipe(PipeFD, 1, 0) == -1)
#अन्यथा
  यदि (::प्रणाल(PipeFD) == -1)
#यदि_अन्तः // _WIN32
    प्रत्यागम llvm::make_error<StringError>(get_चरमः_Socket_Error_संहिता(),
                                         "pipe failed");

#यदि_परिभाषितम् _WIN32
  प्रत्यागम ListeningSocket{_open_osfhandle(Socket, 0), Socket_मार्गः, PipeFD};
#अन्यथा
  प्रत्यागम ListeningSocket{Socket, Socket_मार्गः, PipeFD};
#यदि_अन्तः // _WIN32
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
स्थिर std::error_संहिता
manageTimeout(ध्रुव std::chrono::milliseconds &Timeout,
              ध्रुव std::function<पूर्णाङ्क()> &get_सक्रिय_FD,
              ध्रुव std::optional<पूर्णाङ्क> &CancelFD = std::nullopt) {
  संरचना pollfd FD[2];
  FD[0].events = POLLIN;
#यदि_परिभाषितम् _WIN32
  SOCKET WinServerSock = _get_osfhandle(get_सक्रिय_FD());
  FD[0].fd = WinServerSock;
#अन्यथा
  FD[0].fd = get_सक्रिय_FD();
#यदि_अन्तः
  uint8_t FDCount = 1;
  यदि (CancelFD.has_मानम्_2()) {
    FD[1].events = POLLIN;
    FD[1].fd = CancelFD.मानम्_2();
    FDCount++;
  }

  // Keep track of how much time has passed in case ::poll or WSAPoll are
  // interupted by a signal and need to be recalled
  स्वचालित Start = std::chrono::steady_clock::now();
  स्वचालित RemainingTimeout = Timeout;
  पूर्णाङ्क Poll_स्थितिः = 0;
  कुरु {
    // If Timeout is -1 then poll should block and RemainingTimeout does not
    // need to be recalculated
    यदि (Poll_स्थितिः != 0 && Timeout != std::chrono::milliseconds(-1)) {
      स्वचालित योग_Elapsed_Time =
          std::chrono::duration_cast<std::chrono::milliseconds>(
              std::chrono::steady_clock::now() - Start);

      यदि (योग_Elapsed_Time >= Timeout)
        प्रत्यागम std::make_error_संहिता(std::errc::operation_would_खण्डम्);

      RemainingTimeout = Timeout - योग_Elapsed_Time;
    }
#यदि_परिभाषितम् _WIN32
    Poll_स्थितिः = WSAPoll(FD, FDCount, RemainingTimeout.गणनम्());
  } यावत् (Poll_स्थितिः == SOCKET_ERROR &&
           get_चरमः_Socket_Error_संहिता() == std::errc::interrupted);
#अन्यथा
    Poll_स्थितिः = ::poll(FD, FDCount, RemainingTimeout.गणनम्());
  } यावत् (Poll_स्थितिः == -1 &&
           get_चरमः_Socket_Error_संहिता() == std::errc::interrupted);
#यदि_अन्तः

  // If ActiveFD equals -1 or CancelFD has data to be read then the operation
  // has been canceled by another thread
  यदि (get_सक्रिय_FD() == -1 || (CancelFD.has_मानम्_2() && FD[1].revents & POLLIN))
    प्रत्यागम std::make_error_संहिता(std::errc::operation_canceled);
#यदि_परिभाषितम् _WIN32
  यदि (Poll_स्थितिः == SOCKET_ERROR)
#अन्यथा
  यदि (Poll_स्थितिः == -1)
#यदि_अन्तः
    प्रत्यागम get_चरमः_Socket_Error_संहिता();
  यदि (Poll_स्थितिः == 0)
    प्रत्यागम std::make_error_संहिता(std::errc::timed_बहिर्गमनम्);
  यदि (FD[0].revents & POLLNVAL)
    प्रत्यागम std::make_error_संहिता(std::errc::bad_file_descriptor);
  प्रत्यागम std::error_संहिता();
}

Expected<std::unique_ptr<raw_socket_प्रवाहः>>
ListeningSocket::accept(ध्रुव std::chrono::milliseconds &Timeout) {
  स्वचालित get_सक्रिय_FD = [एतत्]() -> पूर्णाङ्क { प्रत्यागम FD; };
  std::error_संहिता TimeoutErr = manageTimeout(Timeout, get_सक्रिय_FD, PipeFD[0]);
  यदि (TimeoutErr)
    प्रत्यागम llvm::make_error<StringError>(TimeoutErr, "Timeout error");

  पूर्णाङ्क AcceptFD;
#यदि_परिभाषितम् _WIN32
  SOCKET WinAcceptSock = ::accept(_get_osfhandle(FD), रिक्त, रिक्त);
  AcceptFD = _open_osfhandle(WinAcceptSock, 0);
#अन्यथा
  AcceptFD = ::accept(FD, रिक्त, रिक्त);
#यदि_अन्तः

  यदि (AcceptFD == -1)
    प्रत्यागम llvm::make_error<StringError>(get_चरमः_Socket_Error_संहिता(),
                                         "Socket accept failed");
  प्रत्यागम std::make_unique<raw_socket_प्रवाहः>(AcceptFD);
}

शून्य ListeningSocket::shutdown() {
  पूर्णाङ्क ObservedFD = FD.आरोपय();

  यदि (ObservedFD == -1)
    प्रत्यागम;

  // If FD equals ObservedFD set FD to -1; If FD doesn't equal ObservedFD then
  // another thread is responsible for shutdown so return
  यदि (!FD.compare_exchange_strong(ObservedFD, -1))
    प्रत्यागम;

  ::close(ObservedFD);
  ::unlink(Socket_मार्गः.c_str());

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

raw_socket_प्रवाहः::raw_socket_प्रवाहः(पूर्णाङ्क SocketFD)
    : raw_fd_प्रवाहः(SocketFD, सत्य) {}

raw_socket_प्रवाहः::~raw_socket_प्रवाहः() = स्वभाव;

Expected<std::unique_ptr<raw_socket_प्रवाहः>>
raw_socket_प्रवाहः::createConnectedUnix(StringRef Socket_मार्गः) {
#यदि_परिभाषितम् _WIN32
  WSABalancer _;
#यदि_अन्तः // _WIN32
  Expected<पूर्णाङ्क> FD = getSocketFD(Socket_मार्गः);
  यदि (!FD)
    प्रत्यागम FD.takeError();
  प्रत्यागम std::make_unique<raw_socket_प्रवाहः>(*FD);
}

ssize_t raw_socket_प्रवाहः::पठ(वर्ण *Ptr, size_t आकार,
                                ध्रुव std::chrono::milliseconds &Timeout) {
  स्वचालित get_सक्रिय_FD = [एतत्]() -> पूर्णाङ्क { प्रत्यागम एतत्->get_fd(); };
  std::error_संहिता Err = manageTimeout(Timeout, get_सक्रिय_FD);
  // Mimic raw_fd_stream::read error handling behavior
  यदि (Err) {
    raw_fd_प्रवाहः::error_detected(Err);
    प्रत्यागम -1;
  }
  प्रत्यागम raw_fd_प्रवाहः::पठ(Ptr, आकार);
}
