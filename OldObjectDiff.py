#!/usr/bin/python

import sys,HeapDumpParse,heapq
from Print import printError,printDebug

class OldObjectDesc(object):
    def __init__(self,allocations,backtraces):
        self.allocations= allocations
        self.backtraces = backtraces
    def __cmp__(self,other):
        if self.allocations > other.allocations:
            return -1
        elif self.allocations == other.allocations:
            return 0
        else:
            return 1
            

def handleFiles(fileName1,fileName2):
    objDict1 = {}
    objDict2 = {}
    objDict3 = {}
    initObjDict(objDict1,fileName1)
    initObjDict(objDict2,fileName2)
    for item in objDict2.items():
        if item[0] in objDict1:
            objDict3[item[0]] = OldObjectDesc(item[1].allocations - objDict1[item[0]].allocations,item[0])
    return objDict3

def initObjDict(objDict,fileName):
    parser = HeapDumpParse.Parser()
    parser.parse(fileName)
    for outData in parser.outData:
        if outData.backtraces not in objDict:
            objDict[outData.backtraces] = OldObjectDesc(outData.allocations * outData.size,outData.backtraces)
        else:
            oldDesc = objDict[outData.backtraces]
            oldDesc.allocations += outData.allocations * outData.size


if __name__ == '__main__':
    if len(sys.argv) <= 2:
            printError("need files of heap dump")
            sys.exit(1)
    objDict = handleFiles(sys.argv[1],sys.argv[2])
    sortedList = []
    for item in objDict.items():
        sortedList.append(item[1]) 
    sortedList.sort()
    for i in range(len(sortedList)):
        if i == len(sortedList):
            break
        print("Delta: {0}".format(sortedList[i].allocations))
        print("BeginStacktrace:")
        for backtrace in sortedList[i].backtraces._backtraces:
            print("{0:08x}".format(backtrace))
        print("EndStacktrace:")
        print("")
