#pragma once
#include <string>
#include <vector>

namespace Grafips
{
    class Socket
    {
      public:
        // Client-side constructor: connects to server.  For server-side
        // sockets, use the ServerSocket class
        Socket(const std::string &address, int port);
        ~Socket();

        bool Read(void * buf, int size);
        template <typename T> bool Read(T *val) { Read(val, sizeof(T)); }
        template <typename T> bool ReadVec(std::vector<T> *vec)
            {
                Read(vec->data(), vec->size() * sizeof(T));
            }
        bool Write(const void * buf, int size);
        template <typename T> bool Write(const T &val) { Write(&val, sizeof(val)); }
        template <typename T> bool WriteVec(const std::vector<T> &vec)
            {
                Write(vec.data(), sizeof(T) * vec.size());
            }
        
      private:
        friend class ServerSocket;
        // establishes a server, waits for a client to connect
        Socket(int fd) : m_socket_fd(fd) {}
        int m_socket_fd;
    };


    class ServerSocket
    {
      public:
        // establishes a server, waits for a client to connect
        ServerSocket(int port);
        Socket *Accept();

        ~ServerSocket();

      private:
        int m_server_fd;
    };
}