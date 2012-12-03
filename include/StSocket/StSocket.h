/**
 * Copyright © 2009 Kirill Gavrilov <kirill@sview.ru>
 *
 * Distributed under the Boost Software License, Version 1.0.
 * See accompanying file license-boost.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt
 */

#ifndef __StSocket_h_
#define __StSocket_h_

#include <iostream>
#include <StStrings/StString.h>

/**
 * Class - wrapper to system-dependent socket libraries.
 */
class ST_LOCAL StSocket {

        private:

#if(defined(_WIN64))
    unsigned __int64 socket; // socket Descriptor -- typedef UINT_PTR SOCKET;
    bool            wsaInit;
#elif(defined(_WIN32) || defined(__WIN32__))
    unsigned int     socket; // using trick to reduce global includes...
    bool            wsaInit;
#else
    int              socket; // socket File Descriptor
#endif
    short int addressFamily; // AF_INET - IPv4, AF_INET6 - IPv6,...
    int          socketType; // socket type: UDP (datagrams) / TCP (streams) / others

        public:

    /**
     * Just simple buffer. Helper class.
     */
    class Buffer {

            private:

        char*  data;
        size_t size;

            public:

        Buffer(const size_t& size);
        ~Buffer();

        const char* getData() const;
        char* changeData();
        const size_t& getSize() const;
        char& operator[](const size_t& id) const;
        void zero();
        const Buffer& operator=(const char* srcData);
        friend std::ostream& operator<<(std::ostream& stream, const Buffer& out);

    };

    StSocket();
    ~StSocket();

    /**
     * Open socket.
     * @return true on success.
     */
    bool open();

    /**
     * Close socket.
     */
    void close();

    /**
     * Create connection socket connection (TCP/IP).
     * @param host (const char* ) - host name / IP (or proxy);
     * @param port (const unsigned short int& ) - host listening port;
     * @return true on success.
     */
    bool connect(const char* host, const unsigned short int& port = 80);

    /**
     * Send data to connected host.
     * @param buffer (const char* ) - buffer pointer;
     * @param buffSize (const int& ) - buffer size (or data in it);
     * @return bytes (int ) - bytes sent.
     */
    int send(const char* buffer, const int& buffSize);

    int send(const Buffer& buffer) {
        return send(buffer.getData(), (int )buffer.getSize());
    }

    /**
     * Receive data from connected host.
     * Function BLOCKS thread until host send data or timeout.
     * @param buffer (char* ) - OUTPUT buffer pointer ();
     * @param buffSize (const int& ) - buffer size;
     * @return bytes (int ) - bytes received.
     */
    int recv(char* buffer, const int& buffSize);

    int recv(Buffer& buffer) {
        return recv(buffer.changeData(), (int )buffer.getSize());
    }

    static void openURL(const StString& theUrl);

};

#endif // __StSocket_h_
