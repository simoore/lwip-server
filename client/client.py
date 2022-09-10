import sys
from PyQt5.QtCore import QIODevice
from PyQt5.QtWidgets import QApplication, QDialog, QGridLayout, QPushButton, QLineEdit
from PyQt5.QtNetwork import QTcpSocket

class Client(QDialog):

    HOST = '192.168.112.10'
    PORT = 7

    def __init__(self):
        super().__init__()

        # Set UI.
        self.input = QLineEdit(self)
        sendButton = QPushButton(self)
        grid = QGridLayout()
        self.setLayout(grid)
        grid.addWidget(self.input, 0, 0)
        grid.addWidget(sendButton, 0, 1)
        sendButton.setText("Send")
        sendButton.clicked.connect(self.sendClicked)

        # Setup TCP connection.
        self.tcpSocket = QTcpSocket(self)
        self.blockSize = 0
        self.tcpSocket.readyRead.connect(self.readyReadHandler)
        self.tcpSocket.error.connect(self.errorHandler)
        self.tcpSocket.connected.connect(self.connectedHandler)

        # Establish connection.
        self.tcpSocket.connectToHost(self.HOST, self.PORT, QIODevice.ReadWrite)

    def sendClicked(self):
        self.tcpSocket.write(self.input.text().encode())

    def readyReadHandler(self):
        print("Reading data:", self.tcpSocket.readAll())

    def errorHandler(self, socketError):
        print(self, "The following error occurred: %s." % self.tcpSocket.errorString())

    def connectedHandler(self):
        print("Connected to LwIP server.")

if __name__ == '__main__':
    
    app = QApplication(sys.argv)
    client = Client()
    sys.exit(client.exec_())