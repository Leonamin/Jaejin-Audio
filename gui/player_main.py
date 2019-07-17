from player_gui import *
from player_back import *
from player_function import *
from setting_gui import *

class MainPlayer(Ui_MainWindow, QtCore.QObject):
    playFlag = 0
    repeatFlag = 0
    shuffleFlag = 0
    connectFlag = 0
    playList = []
    playPoint = 0

    setPlayFlagSigObj   = QtCore.pyqtSignal(int)
    setMusicSigObj      = QtCore.pyqtSignal(str, int, int)

    def initProgram(self):
        # Player 스레드 추가
        self.Player = MusicPlayer()
        self.PlayerTh = QtCore.QThread()
        self.Player.moveToThread(self.PlayerTh)
        self.PlayerTh.start()

        self.setupSig()

        #정보 가져오기
        self.loadAllMusicList()

        #정보 UI에 표시
        self.showPlayList()

        # Player 설정
        ret = self.Player.setSocket()
        if ret is 1:
            #self.Player.waitMeta()
            pass
        else :
            while True:
                choice = QtWidgets.QMessageBox.question(None, "Error", "장치와의 연결이 거부 되었습니다!\n다시 연결하시겠습니까?", QtWidgets.QMessageBox.Yes | QtWidgets.QMessageBox.No)
                if choice == QtWidgets.QMessageBox.Yes:
                    ret = self.Player.setSocket()
                    if ret is 1:
                        self.connectFlag = 1
                        break
                else:
                    break
        
        self.Player.endMusicSigObj.connect(self.endMusicSignal)

        #내부 처리
        self.makePlayList()
        self.playBtn.setChecked(True)

        self.setMusic(self.playList[self.playPoint])

    #시그널 설정 함수
    def setupSig(self):
        # 내부 시그널
        self.playBtn.clicked.connect(self.playBtnSignal)
        self.prevBtn.clicked.connect(self.prevBtnSingal)
        self.nextBtn.clicked.connect(self.nextBtnSignal)
        self.rptBtn.clicked.connect(self.rptBtnSignal)
        self.shfBtn.clicked.connect(self.shfBtnSignal)
        self.listWidget.itemSelectionChanged.connect(self.setCurMusic)
        self.actEditConneect.triggered.connect(self.openSettingUi)
        self.volDial.valueChanged.connect(self.volumeChanged)
        self.musicProgress.sliderMoved.connect(self.timeChanged)

        #외부 시그널
        self.setMusicSigObj.connect(self.Player.setMusic)

    # 시그널 연결 함수
    def playBtnSignal(self):
        if self.playBtn.isChecked():
            self.playFlag = 1
        else :
            self.playFlag = 0

        if self.playFlag:
            self.Player.sendCmd("RESUM")
        else:
            self.Player.sendCmd("PAUSE")

    def prevBtnSingal(self):
        self.playPoint -= 1
        if self.playPoint < 0:
            self.playPoint = len(self.allMusicList) - 1
        self.changeMusic()

    def nextBtnSignal(self):
        self.playPoint += 1
        if self.playPoint >= len(self.allMusicList):
            self.playPoint = 0
        self.changeMusic()

    def rptBtnSignal(self):
        self.repeatFlag ^= 1
        if self.repeatFlag:
            self.rptBtn.setStyleSheet("background-color: rgb(186, 189, 182);")
        else :
            self.rptBtn.setStyleSheet("")
    
    def shfBtnSignal(self):
        self.shuffleFlag ^= 1
        if self.shuffleFlag:
            self.shfBtn.setStyleSheet("background-color: rgb(186, 189, 182);")
            self.makePlayList()
        else :
            self.shfBtn.setStyleSheet("")
            self.makePlayList()

    def setCurMusic(self):
        curentMusic = self.listWidget.currentItem().text()
        self.playPoint = self.playList.index(curentMusic)
        self.changeMusic()

    def volumeChanged(self):
        curVol = self.volDial.value()
        print("volume: %d" % curVol)
        self.Player.sendCmd("VOLUM", vol=curVol)
    
    def timeChanged(self):
        self.curMusicTime = self.musicProgress.value()
        minute = self.curMusicTime // 60
        second = self.curMusicTime % 60
        curTime = "%d%d:%d%d" % (minute // 10,\
                                    minute % 10,\
                                    second //10,\
                                    second % 10)
        self.curMusictimeLbl.setText(curTime)
        self.Player.sendCmd("TIME ", time=self.curMusicTime)
    
    # 정보 가져오기
    def loadAllMusicList(self):
        self.dirList = getDirPath()
        self.allMusicList = getPlayList(self.dirList)
        self.allMusicDic = divPathName(self.allMusicList)
        self.allMusicList = list(self.allMusicDic.keys())

    #정보 표시하기
    def showPlayList(self, path=None):
        self.listWidget.addItems(self.allMusicList)

    def setMusic(self, musicName):
        self.musNameLbl.setText(musicName)
        musicName = self.allMusicDic.get(musicName)
        strtoks = musicName.split(".")
        if strtoks[1] != "wav":
            newName = convertAudio(musicName)
            musicName = newName
            self.convertFlag = 1
        else :
            self.connectFlag = 0
        self.musicInfo = getMusicInfo(musicName)
        musicTime = int(self.musicInfo.get("MusicTime"))
        self.initMusicProgress(musicTime)
        self.setMusicSigObj.emit(musicName, int(self.musicInfo.get("DataPos")), int(self.musicInfo.get("DataSize")))
        #self.Player.setMusic(musicName, int(self.musicInfo.get("DataPos")))

    # 내부 처리
    def makePlayList(self):
        if self.shuffleFlag:
            self.playList = []
            self.playList.extend(self.allMusicList)
            random.shuffle(self.playList)
            self.playPoint = 0
        else:
            self.playList = []
            self.playList.extend(self.allMusicList)
            self.playPoint = 0

    def openSettingUi(self):
        self.SetWindow=QtWidgets.QMainWindow()
        self.ui=SettingUI()
        self.ui.setupUi(self.SetWindow)
        self.ui.setupSig()
        self.ui.sendSettingSigObj.connect(self.reconnect)
        self.SetWindow.show()
    
    def initMusicProgress(self, time):
        self.curMusicTime = 0
        self.totalMusicTime = time
        self.musicProgress.setMaximum(time)
        minute = time // 60
        second = time % 60
        timetext = "00:00"
        self.curMusictimeLbl.setText(timetext)
        timetext = "%d%d:%d%d" % (minute // 10, minute % 10, second // 10, second % 10)
        self.totalMusictimeLbl.setText(timetext)

    def changeMusic(self):
        self.Player.sendCmd("CHANG")
        self.setMusic(self.playList[self.playPoint])

    @QtCore.pyqtSlot(str, int)
    def reconnect(self, addr, port):
        self.Player.setSocket(addr, port)
        ret = self.setMusic(self.playList[self.playPoint])
        if ret == 1:
            self.connectFlag = 1

    @QtCore.pyqtSlot(int)
    def updateTimeSignal(self, ctime):
        self.curMusicTime = ctime
        self.musicProgress.setValue(ctime)
        print("Set Value: %d" % ctime)
        if(self.totalMusicTime <= self.curMusicTime) :
            self.endMusicSignal()

    @QtCore.pyqtSlot()
    def endMusicSignal(self):
        print("Music End")
        if self.repeatFlag:
            self.playPoint += 1
            if self.playPoint >= len(self.allMusicList):
                self.playPoint = 0
            self.setMusic(self.playList[self.playPoint])

class SettingUI(Ui_IPSettingUI, QtCore.QObject):
    sendSettingSigObj = QtCore.pyqtSignal(str, int)
    def setupSig(self):
        self.confirmBtn.clicked.connect(self.saveSetting)
    
    # 예외처리 필요
    # addr xxx.xxx.xxx.xxx port 0~65535 범위내만
    def saveSetting(self):
        addr = self.editAddr.text()
        port = int(self.editPort.text()) 
        self.sendSettingSigObj.emit(addr, port)

if __name__ == "__main__":
    import sys
    app = QtWidgets.QApplication(sys.argv)
    MainWindow = QtWidgets.QMainWindow()

    #main 객체
    Main = MainPlayer()
    Main.setupUi(MainWindow)
    Main.initProgram()

    MainWindow.show()
    sys.exit(app.exec_())
    