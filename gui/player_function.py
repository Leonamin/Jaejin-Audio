import os
import glob
import yaml
import getpass
import random
from datetime import datetime
from pydub import AudioSegment

def creatSettingFile():
    now = datetime.now()
    nowDate = "{0}-{1}-{2}".format(now.year, now.month, now.day)
    defaultDir = "/home/{0}/Music".format(getpass.getuser())
    initSetting = """
Date: {0}
MusicDirectory: 
    -
        {1}
    """.format(nowDate, defaultDir)

    fd = open("setting.yml", "w+")
    fd.write(initSetting)
    fd.close()

def getFilename(str, exten):
    strtoks = str.split('/')
    if exten == True:
        fileName = strtoks[-1]
    else:
        strtoks = strtoks[-1].split('.')
        fileName = strtoks[0]
    return fileName

def getMusicList(path):
    fileList = []
    musicList = []
    fileList = os.listdir(path)
    for item in fileList:
        if(item.find(".wav") is not -1):
            musicList.append(path+ '/' +item)
        elif(item.find(".mp3") is not -1):
            musicList.append(path+ '/' + item)
            pass
        elif(item.find(".flac") is not -1):
            pass
        elif(os.path.isdir(path)):
            subDir = path + '/' + item
            subDirList = getMusicList(subDir)
            for subItem in subDirList:
                musicList.append(subItem)
        else:
            pass

    return musicList

def getPlayList(folderlist):
    allMusicList = []
    for path in folderlist:
        try:
            musicList = getMusicList(path)
            allMusicList.extend(musicList)
        except FileNotFoundError:
            print("{0} is not found!".format(path))

    return allMusicList

def getDirPath():
    dirList = []
    try:
        fd = open("setting.yml", "r")
    except FileNotFoundError:
        creatSettingFile()
    fd = open("setting.yml", "r")

    data = yaml.load(fd.read())

    for item in data['MusicDirectory']:
        dirList.append(item)
    
    fd.close()

    return dirList

def divPathName(musicList):
    musicTable = {}
    for item in musicList:
        musicName = getFilename(item, 0)
        musicTable[musicName] = item
    
    return musicTable

def readFmtHeader(musicName, pos):
    headerSize = 24
    with open(musicName, "rb") as fd:
        fd.seek(pos)
        wavHeader = fd.read(headerSize)

    return wavHeader

def readDataHeader(musicName, pos):
    headerSize = 8
    with open(musicName, "rb") as fd:
        fd.seek(pos)
        wavHeader = fd.read(headerSize)

    return wavHeader

def procFmtHeader(musicName, pos):
    wavHeader = readFmtHeader(musicName, pos)
    wavInfo = {}
    wavInfo["FmtID"]          = "%c%c%c%c" % (wavHeader[0], wavHeader[1], wavHeader[2], wavHeader[3])
    wavInfo["FmtSize"]        = wavHeader[4] + (wavHeader[5] << 8) + (wavHeader[6] << 16) + (wavHeader[7] << 24)
    wavInfo["AudioFormat"]    = wavHeader[8] + (wavHeader[9] << 8)
    wavInfo["Channels"]       = wavHeader[10] + (wavHeader[11] << 8)
    wavInfo["SampleRate"]     = wavHeader[12] + (wavHeader[13] << 8) + (wavHeader[14] << 16) + (wavHeader[15] << 24)
    wavInfo["AvgByteRate"]    = wavHeader[16] + (wavHeader[17] << 8) + (wavHeader[18] << 16) + (wavHeader[19] << 24)
    wavInfo["BlockAlign"]     = wavHeader[20] + (wavHeader[21] << 8)
    wavInfo["BitPerSample"]   = wavHeader[22] + (wavHeader[23] << 8)
    
    return wavInfo

def procDataHeader(musicName, pos):
    wavHeader = readDataHeader(musicName, pos)
    wavInfo = {}
    wavInfo["DataID"]       = "%c%c%c%c" % (wavHeader[0], wavHeader[1], wavHeader[2], wavHeader[3])
    wavInfo["DataSize"]     = wavHeader[4] + (wavHeader[5] << 8) + (wavHeader[6] << 16) + (wavHeader[7] << 24)

    return wavInfo

def getMusicTime(hz, bit, channel, size):
    time = size * 8 / hz / bit / channel

    return time

def readFile(fileName, pos=0, length = 0):
    with open(fileName, "rb") as fd:
        fd.seek(pos)
        readData = fd.read(length)
    
    return readData

def getMusicInfo(fileName):
    riffpos = 12
    fmtpos = 36
    dataChnkSize = 8

    musicInfo = {}
    strtoks = fileName.split(".")
    if strtoks[1] != "wav":
        return musicInfo
    
    fmtInfo = procFmtHeader(fileName, riffpos)

    pos = fmtpos
    dataInfo = procDataHeader(fileName, pos)
    while True:
        if dataInfo.get("DataID") != "data" :
            pos += dataInfo.get("DataSize") + dataChnkSize
            dataInfo = procDataHeader(fileName, pos)
        else :
            #pos += dataChnkSize
            break

    musicInfo["SampleRate"]     = fmtInfo.get("SampleRate")
    musicInfo["Channels"]       = fmtInfo.get("Channels")
    musicInfo["BitPerSample"]   = fmtInfo.get("BitPerSample")
    musicInfo["DataSize"]       = dataInfo.get("DataSize")
    musicInfo["MusicTime"]      = getMusicTime(fmtInfo.get("SampleRate"), \
                                      fmtInfo.get("Channels"), \
                                      fmtInfo.get("BitPerSample"), \
                                      dataInfo.get("DataSize"))
    musicInfo["DataPos"]        = pos
    
    return musicInfo

def convertAudio(musicName):
    sound = AudioSegment.from_mp3(musicName)
    sound = sound.set_channels(2)
    sound = sound.set_frame_rate(48000)
    sound = sound.set_sample_width(2)

    strtoks = musicName.split(".")
    newName = strtoks[0] + ".wav"
    sound.export(newName, format="wav")

    return newName

if __name__ == "__main__":
    path1 = "/home/leonamin/Music/tiktok.wav"
    path2 = "/home/leonamin/Music/OneRepublic_Counting Stars.wav"
    path3 = "/media/leonamin/플래시/음악/미국이/Carly Rae Jepsen_Call Me Maybe.wav"
    
    musicInfo   = getMusicInfo(path3)
    pos         = int(musicInfo.get("DataPos"))
    dataSize    = int(musicInfo.get("DataSize"))

    musicData = readFile(path3, pos + 8, dataSize)
    
    print("Data Size: {0}\nRead Size: {1}".format(dataSize ,len(musicData)))

    #print(readData)