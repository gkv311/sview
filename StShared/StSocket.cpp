/**
 * Copyright © 2009 Kirill Gavrilov <kirill@sview.ru>
 *
 * Distributed under the Boost Software License, Version 1.0.
 * See accompanying file license-boost.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt
 */

#include <StSocket/StSocket.h>

#include <StStrings/StLogger.h>

#if(defined(_WIN32) || defined(__WIN32__))
    #include <windows.h>
    #include <winsock.h>
#else
    #include <stdio.h>
    #include <sys/types.h>
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <netdb.h>
#endif

static void stZero(char* buffer, size_t bytes) {
    memset(buffer, 0, bytes);
}

StSocket::Buffer::Buffer(const size_t& size)
    : data(new char[size]), size(size) {
    //
    zero();
}

StSocket::Buffer::~Buffer() {
    delete[] data;
}

const char* StSocket::Buffer::getData() const {
    return data;
}

char* StSocket::Buffer::changeData() {
    return data;
}

const size_t& StSocket::Buffer::getSize() const {
    return size;
}

char& StSocket::Buffer::operator[](const size_t& id) const {
    return data[id];
}

void StSocket::Buffer::zero() {
    memset(data, 0, size);
}

const StSocket::Buffer& StSocket::Buffer::operator=(const char* srcData) {
    size_t srcSize = strlen(srcData);
    size_t cpBytes = (srcSize >= size) ? size : srcSize;
    stMemCpy(data, srcData, cpBytes);
    return *this;
}

std::ostream& operator<<(std::ostream& stream, const StSocket::Buffer& out) {
    stream << out.data;
    return stream;
}

enum {
    TCP_SOCKET = SOCK_STREAM,
    UDP_SOCKET = SOCK_DGRAM,
};

StSocket::StSocket()
#if(defined(_WIN32) || defined(__WIN32__))
    : socket(INVALID_SOCKET), wsaInit(false),
#else
    : socket(-1),
#endif
    addressFamily(AF_INET), socketType(TCP_SOCKET) {
    //
}

StSocket::~StSocket() {
    close();
}

bool StSocket::open() {
#if(defined(_WIN32) || defined(__WIN32__))
    WORD wVersionRequested = MAKEWORD(2, 2); // should we use version 1.1 for more compatibility
    WSADATA wsaData;
    int err = WSAStartup(wVersionRequested, &wsaData);
    if(err != 0) {
        ST_DEBUG_LOG("StSocket ERROR, WSAStartup failed");
        return false;
    }
    if(LOBYTE(wsaData.wVersion) != 2 || HIBYTE(wsaData.wVersion) != 2) {
        ST_DEBUG_LOG("StSocket ERROR, Could not find a usable version of Winsock.dll");
        WSACleanup();
        return false;
    }
    wsaInit = true;
#endif
    socket = ::socket(addressFamily, socketType, 0);
#if(defined(_WIN32) || defined(__WIN32__))
    return socket != INVALID_SOCKET;
#else
    return socket >= 0;
#endif
}

void StSocket::close() {
#if(defined(_WIN32) || defined(__WIN32__))
    closesocket(socket);
    ///return closesocket(socket) == 0;
    if(wsaInit) {
        WSACleanup();
        wsaInit = false;
    }
#else
    ///close(socket);
#endif
}

bool StSocket::connect(const char* host, const unsigned short int& port) {
    struct hostent* server = gethostbyname(host);
    if(server == NULL) {
        ST_DEBUG_LOG("StSocket ERROR, No such host (" + host + ")");
        return false;
    }

    struct sockaddr_in destination; stZero((char* )&destination, sizeof(destination));
    destination.sin_family = addressFamily;
    stMemCpy((void* )&destination.sin_addr.s_addr, (void* )server->h_addr, server->h_length);
    destination.sin_port = htons(port);

    return ::connect(socket, (struct sockaddr* )&destination, sizeof(destination)) == 0;
}

int StSocket::send(const char* buffer, const int& buffSize) {
    int bytesSent = ::send(socket, buffer, buffSize, 0);
    if(bytesSent < 0) {
        ST_DEBUG_LOG("StSocket ERROR, writing to socket");
    }
    return bytesSent;
}

int StSocket::recv(char* buffer, const int& buffSize) {
    stZero(buffer, buffSize);
    int bytesRecv = ::recv(socket, buffer, buffSize, 0);
    if(bytesRecv < 0) {
        ST_DEBUG_LOG("StSocket ERROR, reading from socket");
    }
    return bytesRecv;
}

#if !(defined(__APPLE__))

// TODO (Kirill Gavrilov#5#) move code to utils
void StSocket::openURL(const StString& theUrl) {
#if(defined(_WIN32) || defined(__WIN32__))
    ShellExecuteW(NULL, L"open", theUrl.toUtfWide().toCString(), NULL, NULL, SW_SHOWNORMAL);
#elif(defined(__linux__) || defined(__linux))
    // we use nice script tool from Xdg-utils package
    // http://portland.freedesktop.org/wiki/
    StArrayList<StString> anArguments(1);
    anArguments.add(theUrl);
    if(!StProcess::execProcess("xdg-open", anArguments)) {
        ST_DEBUG_LOG("xdg-open is not found!");
    }
    // also we could use GTK function
    //gtk_show_uri(NULL, uri, gtk_get_current_event_time(), &err);
#endif
}

#endif // !__APPLE__
