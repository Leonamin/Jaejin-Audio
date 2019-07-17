from PyQt5 import QtCore
import time
import socket
from numpy import *
from player_function import *
import sys

class MusicPlayer(QtCore.QObject):
    connectSigObj   = QtCore.pyqtSignal(int)
    endMusicSigObj  = QtCore.pyqtSignal()

    def __init__(self):
        super().__init__()
        self.refTime = time.time() + 1
        self.runFlag = 0
        self.connectFlag = 0
        self.curMusicPos = 0
        self.runMusicFlag= False
        self.clientSocket = socket.socket()

    @QtCore.pyqtSlot(str, int)
    def setSocket(self, addr="192.168.110.2", port=5000):
        # self.clientSocket.close()
        self.clientSocket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        try :
            print(f"IP: {addr}, Port: {port}")
            self.clientSocket.connect((addr, port))
            print("Connected!")
            self.connectSigObj.emit(1)
            self.connectFlag = 1
            return 1
        except :
            print("Connect Failed!")
            self.connectSigObj.emit(0)
            self.connectFlag = 0

            return -1    

    @QtCore.pyqtSlot(str, int, int)
    def setMusic(self, filePath, pos, dataSize):
        self.curMusic = filePath
        self.curMusicPos = pos
        self.dataSize = dataSize
        self.waitMeta()
        self.runMusicFlag = True
        self.waitDead()
    
    def waitMeta(self):
        msg = self.clientSocket.recv(6)

        if(msg == bytes(b"META \x00")) :
            print("I received META")
            self.sendMetaData()
            self.sendMusicData()
        else :
            print("Worng Data %s" % str(msg))

    def sendMetaData(self):
        fmtHeader = readFmtHeader(self.curMusic, 12)
        dataHeader = readDataHeader(self.curMusic, self.curMusicPos)
        mergeHeader = fmtHeader + dataHeader
        try:
            self.clientSocket.send(fmtHeader, 24)
            self.clientSocket.send(dataHeader, 8)
        except BrokenPipeError:
            print("Server is Close!")
    
    def sendMusicData(self):
        cnt = 0
        musicData = readFile(self.curMusic, self.curMusicPos + 8, self.dataSize)
        lenData = len(musicData)
        try:
            print("Data Size: {0}\nReal Size: {1}".format(self.dataSize, lenData))
            # print(musicData[lenData])
            sendedData = self.clientSocket.send(musicData)
            # self.clientSocket.sendall(musicData)
            print("Sended Data: {0}".format(sendedData))
        except BrokenPipeError:
            print("Server is Close!")
        print("I send Music Data")
    
    def sendCmd(self, cmd, time=0, vol=0):
        cmdLen = len(cmd)
        if cmdLen < 5 or cmdLen > 5:
            print("Command Length must 5 length!")
            return -1
        try:
            sendCmd = cmd + "\0"
            self.clientSocket.send(sendCmd.encode('utf-8'))
        except BrokenPipeError:
            print("Server is Close!")
        
        if cmd == "CHANG":
            self.runMusicFlag = False

        if cmd == "TIME ":
            try:
                self.clientSocket.send(int32(time))
            except BrokenPipeError:
                print("Server is Close!")
        if cmd == "VOLUM":
            try:
                self.clientSocket.send(int32(vol))
            except BrokenPipeError:
                print("Server is Close!")        
    
    def waitDead(self):
        msg = self.clientSocket.recv(6)

        if(msg == bytes(b"DEAD \x00")):
            print("I received DEAD")
            self.endMusicSigObj.emit()
        else:
            print("Wrong Message: %s" % str(msg))