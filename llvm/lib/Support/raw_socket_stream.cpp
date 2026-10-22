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

#అన్తర్భావ "llvm/Support/raw_socket_stream.h"
#అన్తర్భావ "llvm/Config/config.h"
#అన్తర్భావ "llvm/Support/Error.h"
#అన్తర్భావ "llvm/Support/FileSystem.h"

#అన్తర్భావ <atomic>
#అన్తర్భావ <fcntl.h>
#అన్తర్భావ <functional>

#నిర్వచితం_కాకపోతే _WIN32
#అన్తర్భావ <poll.h>
#అన్తర్భావ <sys/socket.h>
#అన్తర్భావ <sys/un.h>
#లేకపోతే
#అన్తర్భావ "llvm/Support/Windows/WindowsSupport.h"
// winsock2.h must be included before afunix.h. Briefly turn off clang-format to
// avoid error.
// clang-format off
#అన్తర్భావ <winsock2.h>
#అన్తర్భావ <afunix.h>
// clang-format on
#అన్తర్భావ <io.h>
#అయితే_ముగింపు // _WIN32

#ఒకవేళ defined(HAVE_UNISTD_H)
#అన్తర్భావ <unistd.h>
#అయితే_ముగింపు

ఉపయోగించి నామాకాశ llvm;

#నిర్వచితమైతే _WIN32
WSABalancer::WSABalancer() {
  WSADATA Wsa_దత్తాంశం;
  ::memset(&Wsa_దత్తాంశం, 0, పరిమాణం(Wsa_దత్తాంశం));
  ఒకవేళ (WSAStartup(MAKEWORD(2, 2), &Wsa_దత్తాంశం) != 0) {
    llvm::report_fatal_error("WSAStartup failed");
  }
}

WSABalancer::~WSABalancer() { WSACleanup(); }
#అయితే_ముగింపు // _WIN32

స్థిర std::error_సంకేతం get_చివరి_Socket_Error_సంకేతం() {
#నిర్వచితమైతే _WIN32
  తిరిగి std::error_సంకేతం(::WSAGet_చివరి_Error(), std::system_category());
#లేకపోతే
  తిరిగి errno_As_Error_సంకేతం();
#అయితే_ముగింపు
}

#నిర్వచితమైతే _WIN32
ఉపయోగించి NativeSocket = SOCKET;
#లేకపోతే
ఉపయోగించి NativeSocket = పూర్ణాంకం;
#నిర్వచించు INVALID_SOCKET -1
#అయితే_ముగింపు

స్థిర పూర్ణాంకం closeSocket(NativeSocket Socket) {
#నిర్వచితమైతే _WIN32
  తిరిగి ::closesocket(Socket);
#లేకపోతే
  తిరిగి ::close(Socket);
#అయితే_ముగింపు
}

స్థిర Expected<sockaddr_un> సమితి_Socket_Addr(StringRef Socket_మార్గం) {
  నిర్మాణం sockaddr_un Addr;
  memset(&Addr, 0, పరిమాణం(Addr));
  Addr.sun_family = AF_UNIX;

  ఒకవేళ (పరిమాణం(sockaddr_un::sun_మార్గం) <= Socket_మార్గం.కొలత_2())
    తిరిగి make_error<StringError>(
        std::make_error_సంకేతం(std::errc::filename_too_long),
        "Socket path exceeds sockaddr_un::sun_path size limit");

  strncpy(Addr.sun_మార్గం, Socket_మార్గం.str().c_str(), పరిమాణం(Addr.sun_మార్గం) - 1);
  తిరిగి Addr;
}

స్థిర Expected<పూర్ణాంకం> getSocketFD(StringRef Socket_మార్గం) {
  NativeSocket Socket = socket(AF_UNIX, SOCK_ప్రవాహం, 0);
  ఒకవేళ (Socket == INVALID_SOCKET) {
    తిరిగి llvm::make_error<StringError>(get_చివరి_Socket_Error_సంకేతం(),
                                         "Create socket failed");
  }

#నిర్వచితమైతే __CYGWIN__
  // On Cygwin, UNIX sockets involve a handshake between connect and accept
  // to enable SO_PEERCRED/getpeereid handling.  This necessitates accept being
  // called before connect can return, but at least the tests in
  // llvm/unittests/Support/raw_socket_stream_test do both on the same thread
  // (first connect and then accept), resulting in a deadlock.  This call turns
  // off the handshake (and SO_PEERCRED/getpeereid support).
  setsockopt(Socket, SOL_SOCKET, SO_PEERCRED, ఖాళీ, 0);
#అయితే_ముగింపు
  Expected<నిర్మాణం sockaddr_un> Addr = సమితి_Socket_Addr(Socket_మార్గం);
  ఒకవేళ (!Addr) {
    closeSocket(Socket);
    తిరిగి Addr.takeError();
  }

  ఒకవేళ (::connect(Socket, (నిర్మాణం sockaddr *)&*Addr, పరిమాణం(*Addr)) == -1) {
    // Grab the error code before closing, which may overwrite it.
    std::error_సంకేతం EC = get_చివరి_Socket_Error_సంకేతం();
    closeSocket(Socket);
    తిరిగి llvm::make_error<StringError>(EC, "Connect socket failed");
  }

#నిర్వచితమైతే _WIN32
  తిరిగి _open_osfhandle(Socket, 0);
#లేకపోతే
  తిరిగి Socket;
#అయితే_ముగింపు // _WIN32
}

ListeningSocket::ListeningSocket(పూర్ణాంకం SocketFD, StringRef Socket_మార్గం,
                                 పూర్ణాంకం PipeFD[2])
    : FD(SocketFD), Socket_మార్గం(Socket_మార్గం), PipeFD{PipeFD[0], PipeFD[1]} {}

ListeningSocket::ListeningSocket(ListeningSocket &&LS)
    : FD(LS.FD.ఎక్కించు()), Socket_మార్గం(LS.Socket_మార్గం),
      PipeFD{LS.PipeFD[0], LS.PipeFD[1]} {

  LS.FD = -1;
  LS.Socket_మార్గం.clear();
  LS.PipeFD[0] = -1;
  LS.PipeFD[1] = -1;
}

Expected<ListeningSocket> ListeningSocket::createUnix(StringRef Socket_మార్గం,
                                                      పూర్ణాంకం MaxBacklog) {

  // Handle instances where the target socket address already exists and
  // differentiate between a preexisting file with and without a bound socket
  //
  // ::bind will return std::errc:address_in_use if a file at the socket address
  // already exists (e.g., the file was not properly unlinked due to a crash)
  // even if another socket has not yet binded to that address
  ఒకవేళ (llvm::sys::fs::exists(Socket_మార్గం)) {
    Expected<పూర్ణాంకం> MaybeFD = getSocketFD(Socket_మార్గం);
    ఒకవేళ (!MaybeFD) {

      // Regardless of the error, notify the caller that a file already exists
      // at the desired socket address and that there is no bound socket at that
      // address. The file must be removed before ::bind can use the address
      consumeError(MaybeFD.takeError());
      తిరిగి llvm::make_error<StringError>(
          std::make_error_సంకేతం(std::errc::file_exists),
          "Socket address unavailable");
    }
    ::close(std::move(*MaybeFD));

    // Notify caller that the provided socket address already has a bound socket
    తిరిగి llvm::make_error<StringError>(
        std::make_error_సంకేతం(std::errc::చిరునామా_లోపల_use),
        "Socket address unavailable");
  }

#నిర్వచితమైతే _WIN32
  WSABalancer _;
#అయితే_ముగింపు
  NativeSocket Socket = socket(AF_UNIX, SOCK_ప్రవాహం, 0);
  ఒకవేళ (Socket == INVALID_SOCKET)
    తిరిగి llvm::make_error<StringError>(get_చివరి_Socket_Error_సంకేతం(),
                                         "socket create failed");

#నిర్వచితమైతే __CYGWIN__
  // On Cygwin, UNIX sockets involve a handshake between connect and accept
  // to enable SO_PEERCRED/getpeereid handling.  This necessitates accept being
  // called before connect can return, but at least the tests in
  // llvm/unittests/Support/raw_socket_stream_test do both on the same thread
  // (first connect and then accept), resulting in a deadlock.  This call turns
  // off the handshake (and SO_PEERCRED/getpeereid support).
  setsockopt(Socket, SOL_SOCKET, SO_PEERCRED, ఖాళీ, 0);
#అయితే_ముగింపు
  Expected<నిర్మాణం sockaddr_un> Addr = సమితి_Socket_Addr(Socket_మార్గం);
  ఒకవేళ (!Addr) {
    closeSocket(Socket);
    తిరిగి Addr.takeError();
  }

  ఒకవేళ (::bind(Socket, (నిర్మాణం sockaddr *)&*Addr, పరిమాణం(*Addr)) == -1) {
    // Grab error code from call to ::bind before closing the socket
    std::error_సంకేతం EC = get_చివరి_Socket_Error_సంకేతం();
    closeSocket(Socket);
    తిరిగి llvm::make_error<StringError>(EC, "Bind error");
  }

  // Mark socket as passive so incoming connections can be accepted
  ఒకవేళ (::listen(Socket, MaxBacklog) == -1)
    తిరిగి llvm::make_error<StringError>(get_చివరి_Socket_Error_సంకేతం(),
                                         "Listen error");

  పూర్ణాంకం PipeFD[2];
#నిర్వచితమైతే _WIN32
  // Reserve 1 byte for the pipe and use default textmode
  ఒకవేళ (::_pipe(PipeFD, 1, 0) == -1)
#లేకపోతే
  ఒకవేళ (::ప్రణాల(PipeFD) == -1)
#అయితే_ముగింపు // _WIN32
    తిరిగి llvm::make_error<StringError>(get_చివరి_Socket_Error_సంకేతం(),
                                         "pipe failed");

#నిర్వచితమైతే _WIN32
  తిరిగి ListeningSocket{_open_osfhandle(Socket, 0), Socket_మార్గం, PipeFD};
#లేకపోతే
  తిరిగి ListeningSocket{Socket, Socket_మార్గం, PipeFD};
#అయితే_ముగింపు // _WIN32
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
స్థిర std::error_సంకేతం
manageTimeout(స్థిరరాశి std::chrono::milliseconds &Timeout,
              స్థిరరాశి std::function<పూర్ణాంకం()> &get_చురుకైన_FD,
              స్థిరరాశి std::optional<పూర్ణాంకం> &CancelFD = std::nullopt) {
  నిర్మాణం pollfd FD[2];
  FD[0].events = POLLIN;
#నిర్వచితమైతే _WIN32
  SOCKET WinServerSock = _get_osfhandle(get_చురుకైన_FD());
  FD[0].fd = WinServerSock;
#లేకపోతే
  FD[0].fd = get_చురుకైన_FD();
#అయితే_ముగింపు
  uint8_t FDCount = 1;
  ఒకవేళ (CancelFD.has_విలువ_2()) {
    FD[1].events = POLLIN;
    FD[1].fd = CancelFD.విలువ_2();
    FDCount++;
  }

  // Keep track of how much time has passed in case ::poll or WSAPoll are
  // interupted by a signal and need to be recalled
  స్వయంచాలక ప్రారంభం = std::chrono::steady_clock::now();
  స్వయంచాలక మిగిలిన_Timeout = Timeout;
  పూర్ణాంకం Poll_స్థితి = 0;
  చేయి {
    // If Timeout is -1 then poll should block and RemainingTimeout does not
    // need to be recalculated
    ఒకవేళ (Poll_స్థితి != 0 && Timeout != std::chrono::milliseconds(-1)) {
      స్వయంచాలక మొత్తం_Elapsed_సమయం =
          std::chrono::duration_cast<std::chrono::milliseconds>(
              std::chrono::steady_clock::now() - ప్రారంభం);

      ఒకవేళ (మొత్తం_Elapsed_సమయం >= Timeout)
        తిరిగి std::make_error_సంకేతం(std::errc::operation_would_ఖండం);

      మిగిలిన_Timeout = Timeout - మొత్తం_Elapsed_సమయం;
    }
#నిర్వచితమైతే _WIN32
    Poll_స్థితి = WSAPoll(FD, FDCount, మిగిలిన_Timeout.లెక్క());
  } అయితే (Poll_స్థితి == SOCKET_ERROR &&
           get_చివరి_Socket_Error_సంకేతం() == std::errc::interrupted);
#లేకపోతే
    Poll_స్థితి = ::poll(FD, FDCount, మిగిలిన_Timeout.లెక్క());
  } అయితే (Poll_స్థితి == -1 &&
           get_చివరి_Socket_Error_సంకేతం() == std::errc::interrupted);
#అయితే_ముగింపు

  // If ActiveFD equals -1 or CancelFD has data to be read then the operation
  // has been canceled by another thread
  ఒకవేళ (get_చురుకైన_FD() == -1 || (CancelFD.has_విలువ_2() && FD[1].revents & POLLIN))
    తిరిగి std::make_error_సంకేతం(std::errc::operation_canceled);
#నిర్వచితమైతే _WIN32
  ఒకవేళ (Poll_స్థితి == SOCKET_ERROR)
#లేకపోతే
  ఒకవేళ (Poll_స్థితి == -1)
#అయితే_ముగింపు
    తిరిగి get_చివరి_Socket_Error_సంకేతం();
  ఒకవేళ (Poll_స్థితి == 0)
    తిరిగి std::make_error_సంకేతం(std::errc::timed_బహిర్గతం);
  ఒకవేళ (FD[0].revents & POLLNVAL)
    తిరిగి std::make_error_సంకేతం(std::errc::bad_file_descriptor);
  తిరిగి std::error_సంకేతం();
}

Expected<std::unique_ptr<raw_socket_ప్రవాహం>>
ListeningSocket::accept(స్థిరరాశి std::chrono::milliseconds &Timeout) {
  స్వయంచాలక get_చురుకైన_FD = [ఇది]() -> పూర్ణాంకం { తిరిగి FD; };
  std::error_సంకేతం TimeoutErr = manageTimeout(Timeout, get_చురుకైన_FD, PipeFD[0]);
  ఒకవేళ (TimeoutErr)
    తిరిగి llvm::make_error<StringError>(TimeoutErr, "Timeout error");

  పూర్ణాంకం AcceptFD;
#నిర్వచితమైతే _WIN32
  SOCKET WinAcceptSock = ::accept(_get_osfhandle(FD), ఖాళీ, ఖాళీ);
  AcceptFD = _open_osfhandle(WinAcceptSock, 0);
#లేకపోతే
  AcceptFD = ::accept(FD, ఖాళీ, ఖాళీ);
#అయితే_ముగింపు

  ఒకవేళ (AcceptFD == -1)
    తిరిగి llvm::make_error<StringError>(get_చివరి_Socket_Error_సంకేతం(),
                                         "Socket accept failed");
  తిరిగి std::make_unique<raw_socket_ప్రవాహం>(AcceptFD);
}

శూన్య ListeningSocket::shutdown() {
  పూర్ణాంకం ObservedFD = FD.ఎక్కించు();

  ఒకవేళ (ObservedFD == -1)
    తిరిగి;

  // If FD equals ObservedFD set FD to -1; If FD doesn't equal ObservedFD then
  // another thread is responsible for shutdown so return
  ఒకవేళ (!FD.compare_exchange_strong(ObservedFD, -1))
    తిరిగి;

  ::close(ObservedFD);
  ::unlink(Socket_మార్గం.c_str());

  // Ensure ::poll returns if shutdown is called by a separate thread
  అక్షరం Byte = 'A';
  ssize_t written = ::లిఖ(PipeFD[1], &Byte, 1);

  // Ignore any write() error
  (శూన్య)written;
}

ListeningSocket::~ListeningSocket() {
  shutdown();

  // Close the pipe's FDs in the destructor instead of within
  // ListeningSocket::shutdown to avoid unnecessary synchronization issues that
  // would occur as PipeFD's values would have to be changed to -1
  //
  // The move constructor sets PipeFD to -1
  ఒకవేళ (PipeFD[0] != -1)
    ::close(PipeFD[0]);
  ఒకవేళ (PipeFD[1] != -1)
    ::close(PipeFD[1]);
}

//===----------------------------------------------------------------------===//
//  raw_socket_stream
//===----------------------------------------------------------------------===//

raw_socket_ప్రవాహం::raw_socket_ప్రవాహం(పూర్ణాంకం SocketFD)
    : raw_fd_ప్రవాహం(SocketFD, నిజం) {}

raw_socket_ప్రవాహం::~raw_socket_ప్రవాహం() = అప్రమేయ;

Expected<std::unique_ptr<raw_socket_ప్రవాహం>>
raw_socket_ప్రవాహం::createConnectedUnix(StringRef Socket_మార్గం) {
#నిర్వచితమైతే _WIN32
  WSABalancer _;
#అయితే_ముగింపు // _WIN32
  Expected<పూర్ణాంకం> FD = getSocketFD(Socket_మార్గం);
  ఒకవేళ (!FD)
    తిరిగి FD.takeError();
  తిరిగి std::make_unique<raw_socket_ప్రవాహం>(*FD);
}

ssize_t raw_socket_ప్రవాహం::పఠ(అక్షరం *Ptr, size_t కొలత,
                                స్థిరరాశి std::chrono::milliseconds &Timeout) {
  స్వయంచాలక get_చురుకైన_FD = [ఇది]() -> పూర్ణాంకం { తిరిగి ఇది->get_fd(); };
  std::error_సంకేతం Err = manageTimeout(Timeout, get_చురుకైన_FD);
  // Mimic raw_fd_stream::read error handling behavior
  ఒకవేళ (Err) {
    raw_fd_ప్రవాహం::error_detected(Err);
    తిరిగి -1;
  }
  తిరిగి raw_fd_ప్రవాహం::పఠ(Ptr, కొలత);
}
