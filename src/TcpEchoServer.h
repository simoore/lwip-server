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

    void init() {
        mTcpServer.init(IP_ADDR_ANY, sPort);
        mTcpServer.registerRecvCallback(TcpServer::RecvCallback::create<TcpEchoServer, TcpEchoServer::recv>(*this));
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