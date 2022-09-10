#ifndef TCP_SERVER_H
#define TCP_SERVER_H

#include <cstdio>
#include "etl/delegate.h"
#include "etl/list.h"
#include "lwip/pbuf.h"
#include "lwip/tcp.h"

/// A generic TCP server that supports a number of connections.
///
/// References
/// ----------
/// https://lwip.fandom.com/wiki/Raw/TCP
class TcpServer {
public:

    /*************************************************************************/
    /********** PUBLIC TYPES *************************************************/
    /*************************************************************************/

    /// The TCP protocol control block, from lwip/tcp.h.
    using TcpControlBlock = struct tcp_pcb;

    /// A LwIP packet buffer.
    using PacketBuffer = struct pbuf;

    /// The status of a connection.
    enum class ConnectionState {
        Closed,         ///< If the connection is closed and dealocated.
        Listening,      ///< The connection is listening for new connections.
        Established,    ///< If the connection is established and ready for write and read.
        Closing         ///< If the connection has been signaled to close but is waiting to finish some tasks.
    };

    struct TcpConnection {
        ConnectionState state{ConnectionState::Closed};
        TcpServer *server{nullptr};
        TcpControlBlock *controlBlock{nullptr};
        PacketBuffer *writeBuffer{nullptr};
        PacketBuffer *readBuffer{nullptr};
        uint16_t writeOffset{0};
        uint16_t readOffset{0};
    };

    /// The callback to pass received data to the application.
    using RecvCallback = etl::delegate<void(TcpConnection &)>;

    /*************************************************************************/
    /********** PUBLIC FUNCTIONS *********************************************/
    /*************************************************************************/

    /// Creates a new LwIP TCP protocol control block to listen for connections, binds an IP address and port to it, 
    /// and opens a listening control block to accept remote connections.
    ///
    /// @param ipAddr
    ///     The IP address the is bound to the connection. IP_ADDR_ANY binds the connection to all local IP addresses
    ///     which are configured in the network interface.
    /// @param port
    ///     The port to bind this connection to.
    void init(const ip_addr_t *ipAddr, uint16_t port);

    /// Sends data to the remote host.
    ///
    /// @param connection
    ///     The TCP connection to send the data on.
    /// @param pbuf
    ///     The data to send.
    void write(TcpConnection &connection, PacketBuffer *pbuf);

    /// Registers a callback with that returns the received data to the application.
    ///
    /// @param cb
    ///     The receive callback.
    void registerRecvCallback(RecvCallback cb);

private:

    /*************************************************************************/
    /********** PRIVATE CONSTANTS ********************************************/
    /*************************************************************************/

    static constexpr uint8_t sPollInterval{0};

    /*************************************************************************/
    /********** PRIVATE FUNCTIONS ********************************************/
    /*************************************************************************/

    /// Closes a connection. Can also be used to deallocate the TCP control block. If the state of the connection 
    /// hasn't changed to Closed, a memory error occurred during closing and it will be attempted again in the polling
    /// function.
    ///
    /// @param connection
    ///     The connection to close.
    void close(TcpConnection &connection);

    /// This function is called when:
    /// * The poll() function is called for a given connection.
    /// * The sent() function is called which allows more data to be added to the TCP send buffer.
    /// * The write() function is called when the application has added new data to the connection to send.
    ///
    /// @param connection
    ///     The connection that is sending data. The send data is stored in its writeBuffer.
    void writeToTcp(TcpConnection &connection);
    
    /// Upon accepting the remote connection, we set the priority of the connection, and we define the recv, err, and
    /// poll LwIP callbacks.
    ///
    /// @param newpcb
    ///     The control block that needs to be initialized and added to a TcpConnection object.
    /// @param err
    ///     Any error that occured while accepting the connection.
    /// @return
    ///     ERR_MEM
    err_t accept(TcpControlBlock *newpcb, err_t err);

    /// Process received data.
    ///
    /// @param connection
    ///     The TCP connection that received the data.
    /// @param packetBuffer
    ///     The data that was received.
    /// @param err
    ///     Any errors that occured when receiving.
    err_t recv(TcpConnection &connection, PacketBuffer *packetBuffer, err_t err);

    /// This is called when an acknowledge is received from the remote host. We use it purely to send more data to the 
    /// host.
    ///
    /// @return
    ///     The LwIP error code. Always ERR_OK.
    err_t sent(TcpConnection &connection, uint16_t len);

    /// LwIP periodically calls this function. We assign the sent callback and try and send any remaining data in the
    /// transmit packet buffer, or if there is no data and the state of the connection is closing, we close the TCP
    /// connection.
    ///
    /// @param connection
    ///     The connection object that is being checked if it needs to close or needs to send more data.
    /// @return
    ///     The LwIP error code for this function. Always ERR_OK.
    err_t poll(TcpConnection &connection);

    /// This is the error handler when LwIP reports a fatal error of some-sort.
    ///
    /// @apram connection
    ///     The Tcp Connection whose control block had a fatal error in the LwIP stack.
    void error(TcpConnection &connection);

    /// LwIP callback for when connection accepted.
    ///
    /// @param arg
    ///     The TcpConnection object associated with listening connection.
    /// @param newpcb
    ///     The control block for the new connection.
    /// @param err
    ///     IF there has been an error in accepting the connection.
    static err_t accept(void *arg, TcpControlBlock *newpcb, err_t err);

    /// LwIP callback for when data has been received.
    /// https://www.nongnu.org/lwip/2_0_x/tcp_8h.html
    ///
    /// @param arg
    ///     The TcpConnection that recevied the data.
    /// @param tpcb
    ///     The protocol control block the data was received with.
    /// @param p
    ///     The packet buffer containing the received data.
    /// @param err
    ///     Any error that occurred while receiving the data.
    static err_t recv(void *arg, TcpControlBlock *tpcb, PacketBuffer *p, err_t err);

    /// LwIP callback for when sent data is acknowledge by the remote host. One thing this indicates is that there
    /// is more room in the TCP send buffers and we can attempt to add more data.
    ///
    /// @param arg
    ///     The arg set for this object is a TcpConnection object.
    /// @param tpcb
    ///     The protocol control block for the connection that had the sent data acknowledged.
    /// @param len
    ///     The amount of data acknowledged.
    static err_t sent(void *arg, TcpControlBlock *tpcb, uint16_t len);

    /// LwIP routinely calls this function to drive the logic of the server.
    ///
    /// @param arg
    ///     The TcpConnection object that should match the protocol control block.
    /// @param tpcb
    ///     The control block associated with one connection.
    static err_t poll(void *arg, TcpControlBlock *tpcb);

    /// When an error occurs, the pcb will be deallocated. Clean up the TcpConnection object such that it is ready
    /// for re-initialization.
    ///
    /// @param arg
    ///     The TcpConnection object for the control block that had a fatal error.
    /// @param err
    ///     The error that occured.
    static void error(void *arg, err_t err);

    /*************************************************************************/
    /********** PRIVATE FIELDS ***********************************************/
    /*************************************************************************/

    /// The maximum number of simultaneous connections this server accepts.
    static constexpr uint32_t sMaxConnections{10};

    /// The LwIP connection that listens for new TCP connections.
    TcpConnection mListeningConnection;

    /// The list of established connections.
    etl::list<TcpConnection, sMaxConnections> mConnections;

    /// Callback to return data to the higher level protocol layer.
    RecvCallback mRecvCallback;

}; // class TcpServer

#endif // TCP_SERVER_H