#ifndef TCP_ECHO_SERVER_H
#define TCP_ECHO_SERVER_H

#include "TcpServer.h"

class TcpEchoServer {
public:

    /*************************************************************************/
    /********** PUBLIC CONSTANTS *********************************************/
    /*************************************************************************/

    /// The port the echo server operates on.
    static constexpr uint16_t sPort{7};

    /*************************************************************************/
    /********** PUBLIC FUNCTIONS *********************************************/
    /*************************************************************************/

    TcpEchoServer() = default;

    /// Allows this server to be bound to any IP address that is assigned to this device, along with a port number that
    /// refers to this echo server. Then a callback to receive data is registered with the server.
    void init() {
        mTcpServer.registerRecvCallback(TcpServer::RecvCallback::create<TcpEchoServer, &TcpEchoServer::recv>(*this));
        mTcpServer.init(IP_ADDR_ANY, sPort);
    }

    /// Handles received data from the TCP server.
    ///
    /// @param connection
    ///     The connection which received the data. The received data is in the readBuffer field.
    void recv(TcpServer::TcpConnection &connection) {
        mTcpServer.write(connection, connection.readBuffer);
    }

private:

    /// The TCP server that handles the communication to this echo server.
    TcpServer mTcpServer;

}; // class TcpEchoServer

#endif // TCP_ECHO_SERVER_H