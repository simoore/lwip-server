#include <cstdio>
#include "TcpServer.h"

/*************************************************************************/
/********** PUBLIC FUNCTIONS *********************************************/
/*************************************************************************/

void TcpServer::init(const ip_addr_t *ipAddr, uint16_t port) {
    if (mListeningConnection.state != ConnectionState::Closed) {
        printf("TcpServer::init, TcpServer already listening.");
        return;
    }

    mListeningConnection.server = this;
    mListeningConnection.controlBlock = tcp_new();
    if (!mListeningConnection.controlBlock) {
        printf("TcpServer::init, Failed to allocate TCP control block.");
        return;   
    }
    
    // tcp_bind() can return ERR_USE if the port is already in use, or ERR_VAL if the state of the connection
    // is not closed.
    err_t err = tcp_bind(mListeningConnection.controlBlock, ipAddr, port);
    if (err != ERR_OK) {
        printf("TcpServer::init, Failed to bind IP Address and Port, %s", lwip_strerr(err));
        close(mListeningConnection);
        return;
    }
    // tcp_listen() de-allocates the control block passed to it and returns a new one.
    mListeningConnection.controlBlock = tcp_listen(mListeningConnection.controlBlock);
    // Connection won't accept connections until you call tcp_accept().
    tcp_accept(mListeningConnection.controlBlock, TcpServer::accept);
    tcp_arg(mListeningConnection.controlBlock, &mListeningConnection);
    mListeningConnection.state = ConnectionState::Listening;
}

void TcpServer::write(TcpConnection &connection, PacketBuffer *pbuf) {
    if (connection.state != ConnectionState::Established) {
        printf("TcpConnection::write, connection not established.");
        return;
    }
    if (!pbuf) {
        printf("TcpConnection::write, pbuf is null.");
        return;
    }
    if (pbuf->tot_len == 0) {
        printf("TcpConnection::write, data is empty.");
        return;
    }

    if (connection.writeBuffer) {
        pbuf_cat(connection.writeBuffer, pbuf);
    } else {
        connection.writeBuffer = pbuf;
    }

    writeToTcp(connection);
}

void TcpServer::registerRecvCallback(RecvCallback cb) {
    mRecvCallback = cb;
}

/*************************************************************************/
/********** PRIVATE FUNCTIONS ********************************************/
/*************************************************************************/

void TcpServer::close(TcpConnection &connection) {
    err_t err = tcp_close(connection.controlBlock);
    if (err == ERR_MEM) {
        // If LwIP returns ERR_MEM, then we must wait to memory is available to close the connection. Simply
        // return and the polling callback will atempt to close the connection later.
        printf("TcpConnection::close: Memory error %s\n", lwip_strerr(err));
        return;
    }
    if (err != ERR_OK) {
        // If LwIP returns any other error code when closing the connection, the connection will be adandoned.
        // Adandoning a connection with tcp_abort never fails. tcp_close shouldn't return any error codes but
        // ERR_OK and ERR_MEM.
        printf("TcpConnection::close: Unexpected error %s\n", lwip_strerr(err));
        tcp_abort(connection.controlBlock);
    }
    connection.state = ConnectionState::Closed;
}

void TcpServer::writeToTcp(TcpConnection &connection) {

    // Enqueue data for transmission. The while loop cycles through the chained pbuf.
    while (true) {

        // Check how much room is in the TCP buffers.
        uint16_t available = tcp_sndbuf(connection.controlBlock);
        if (available == 0) {
            printf("TcpServer::writeToTcp, no room to write to TCP stack.");
            break;
        }

        // Sometimes this function is called when all data has been sent. Just return.
        if (!connection.writeBuffer) {
            printf("TcpServer::writeToTcp, no data to write.");
            break;
        }

        // In each iteration of this loop, we will only try and send the data from the leading pbuf in the chain.
        PacketBuffer &pbuf = *connection.writeBuffer;
        uint16_t pbufLen = pbuf.len - connection.writeOffset;

        // If all the data in the leading pbuf has been sent, free it and move to the next pbuf in the chain.
        if (pbufLen == 0) {
            connection.writeBuffer = pbuf.next;
            // https://programming.vip/docs/lwip-pbuf-of-tcp-ip-protocol-stack.html
            // This site has some nice diagrams explaining pbuf_ref() and pbuf_free().
            if (connection.writeBuffer) {
                pbuf_ref(connection.writeBuffer);
            }
            connection.writeOffset = 0;
            uint8_t freed = pbuf_free(&pbuf);
            if (freed != 1) {
                printf("TcpServer::writeToTcp, %d pbufs were freed.", freed);
            }
            continue;
        }

        // Copy the data into the TCP send buffers.
        uint16_t len = std::max(pbufLen, available);
        uint8_t *data = reinterpret_cast<uint8_t *>(pbuf.payload) + connection.writeOffset;
        err_t err = tcp_write(connection.controlBlock, data, len, TCP_WRITE_FLAG_COPY);
        if (err == ERR_MEM) {
            printf("TcpConnection::writeToTcp, LwIP ERR_MEM.");
            break;
        }
        if (err != ERR_OK) {
            printf("TcpConnection::writeToTcp, tcp_write not ok.");
            break;
        }
        connection.writeOffset += len;
    }

    // Forces LwIP to send data to lower network layer.
    //
    // https://lwip.fandom.com/wiki/Raw/TCP
    // This isn't necessary when this function is called form the recv callback, but it may be necessary when
    // writeToTcp() is called in other contexts.
    tcp_output(connection.controlBlock);
}

err_t TcpServer::accept(TcpControlBlock *newpcb, err_t err) {

    if (err != ERR_OK) {
        printf("TcpServer::accept, err note ok.");
    } else if (mConnections.full()) {
        printf("TcpServer::accept, not able to allocate more connections.");
        err = ERR_MEM;
    }

    if (err != ERR_OK) {
        err_t errClose = tcp_close(newpcb);
        if (errClose != ERR_OK) {
            tcp_abort(newpcb);
            return ERR_ABRT;
        }
        return err;
    }

    mConnections.emplace_back();
    TcpConnection &newConnection = mConnections.back();
    newConnection.controlBlock = newpcb;
    newConnection.server = this;
    newConnection.state = ConnectionState::Established;
    tcp_setprio(newpcb, TCP_PRIO_MIN);
    tcp_arg(newpcb, &newConnection);
    tcp_recv(newpcb, TcpServer::recv);
    tcp_err(newpcb, TcpServer::error);
    tcp_poll(newpcb, TcpServer::poll, sPollInterval);
    tcp_sent(newpcb, TcpServer::sent);
    return ERR_OK;
}

err_t TcpServer::recv(TcpConnection &connection, PacketBuffer *packetBuffer, err_t err) {

    // If we receive an empty tcp frame from the LwIP stack, this signals to the application to close the 
    // connections.
    if (!packetBuffer) {
        connection.state = ConnectionState::Closing;
        if (connection.writeBuffer) {
            writeToTcp(connection);
        } else {
            close(connection);
        }
        return ERR_OK;
    } 

    // If there is an error, just free the packet buffer and don't acknowledge the received data.
    if (err != ERR_OK) {
        pbuf_free(packetBuffer);
        return err;
    }

    // Acknowledge the receiving of the data.
    tcp_recved(connection.controlBlock, packetBuffer->tot_len);

    // Add the data to the connection's read buffer.
    if (connection.readBuffer) {
        pbuf_cat(connection.readBuffer, packetBuffer);
    } else {
        connection.readBuffer = packetBuffer;
    }

    // Send the data to the application. You send the connection, because you want to know who to reply to.
    // The application must consume and dealloacte the receive buffer.
    if (mRecvCallback.is_valid()) {
        mRecvCallback(connection);
    }
    return ERR_OK;
}

err_t TcpServer::sent(TcpConnection &connection, uint16_t len) {
    static_cast<void>(len);
    if (connection.writeBuffer) {
        writeToTcp(connection);
    } else if (connection.state == ConnectionState::Closing) {
        close(connection);
    }
    return ERR_OK;
}

err_t TcpServer::poll(TcpConnection &connection) {
    if (connection.writeBuffer) {
        writeToTcp(connection);
    } else if (connection.state == ConnectionState::Closing) {
        close(connection);
    }
    return ERR_OK;
}

void TcpServer::error(TcpConnection &connection) {
    connection.state = ConnectionState::Closed;
    connection.controlBlock = nullptr;
    pbuf_free(connection.readBuffer);
    pbuf_free(connection.writeBuffer);
    connection.readBuffer = nullptr;
    connection.writeBuffer = nullptr;
}

err_t TcpServer::accept(void *arg, TcpControlBlock *newpcb, err_t err) {
    if (!arg) {
        printf("TcpServer::accept, arg is null.");
        tcp_abort(newpcb);
        return ERR_ABRT;
    }
    TcpConnection *connection = reinterpret_cast<TcpConnection *>(arg);
    return connection->server->accept(newpcb, err);
}

err_t TcpServer::recv(void *arg, TcpControlBlock *tpcb, PacketBuffer *p, err_t err) {
    if (!arg) {
        printf("TcpServer::recv, arg is null.");
        tcp_abort(tpcb);
        return ERR_ABRT;
    }
    TcpConnection *connection = reinterpret_cast<TcpConnection *>(arg);
    if (connection->controlBlock != tpcb) {
        printf("TcpServer::recv, control block mismatch.");
        tcp_abort(tpcb);
        return ERR_ABRT;
    }
    return connection->server->recv(*connection, p, err);
}

err_t TcpServer::sent(void *arg, TcpControlBlock *tpcb, uint16_t len) {
    if (!arg) {
        printf("TcpServer::sent, arg is null.");
        tcp_abort(tpcb);
        return ERR_ABRT;
    }
    TcpConnection *connection = reinterpret_cast<TcpConnection *>(arg);
    if (connection->controlBlock != tpcb) {
        printf("TcpServer::sent, control block mismatch.");
        tcp_abort(tpcb);
        return ERR_ABRT;
    }
    return connection->server->sent(*connection, len);
}

err_t TcpServer::poll(void *arg, TcpControlBlock *tpcb) {
    if (!arg) {
        printf("TcpServer::poll, arg is null.");
        tcp_abort(tpcb);
        return ERR_ABRT;
    }
    TcpConnection *connection = reinterpret_cast<TcpConnection *>(arg);
    if (connection->controlBlock != tpcb) {
        printf("TcpServer::poll, control block mismatch.");
        tcp_abort(tpcb);
        return ERR_ABRT;
    }
    return connection->server->poll(*connection);
}

void TcpServer::error(void *arg, err_t err) {
    static_cast<void>(err);
    if (!arg) {
        printf("TcpServer::error, arg is null.");
        return;
    }
    TcpConnection *connection = reinterpret_cast<TcpConnection *>(arg);
    connection->server->error(*connection);
}
