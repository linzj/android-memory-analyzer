#!/usr/bin/python
import socket,struct,sys
from hash import backtrace_element
from Print import printDebug,printError


def printInfo(size,allocations,backtraces):
#Allocations: 3
#Size: 3456000
#TotalSize: 10368000
#BeginStacktrace:
    print("Allocations: {0}".format(allocations))
    print("Size: {0}".format(size))
    print("TotalSize: {0}".format(allocations * size))
    print("BeginStacktrace:")
    for backtrace in backtraces:
        print("{0:08x}".format(backtrace))
    print("EndStacktrace:")
    print("")

SIZE_FLAG_ZYGOTE_CHILD = 1 << 31


class MemInfoEntry(object):
    def __hash__(self):
        ret = self.size << 16 | hash(self.backtraces)
        return ret
    def __eq__(self,other):
        #if not isinstance(other,MemInfoEntry):
        #    return False
        if(self.size != other.size):
            return False
        return self.backtraces == other.backtraces
    def __init__(self,size,allocations,backtraces):
        self.size = size
        self.allocations = allocations
        self.backtraces = backtrace_element(backtraces)
    
class Parser(object):
    def __init__(self):
        self.outData = []
    def genInfo(self,size,allocations,backtraces):
        self.outData.append(MemInfoEntry(size,allocations,backtraces))
    def printEntry(self,buf,offset,infoSize):
        if len(buf) - offset < infoSize:
            printDebug("the buffer is too small! exit!")
            return len(buf)
        endOffset = offset + infoSize
        size = struct.unpack_from("<I",buf,offset)[0]
        if size & SIZE_FLAG_ZYGOTE_CHILD:
            size = size & ~SIZE_FLAG_ZYGOTE_CHILD
        offset = offset + 4

        allocations = struct.unpack_from("<I",buf,offset)[0]
        offset = offset + 4
        backtraces = []
        while offset < endOffset:
            backtrace = struct.unpack_from("<I",buf,offset)[0]
            offset = offset + 4
            backtraces.append(backtrace)
        self.genInfo(size,allocations,backtraces)
        return offset


    def handleBuf(self,buf):
        printDebug("buf length is {0}".format(len(buf)))
        offset = 0
        overallSize = struct.unpack_from("<i",buf,offset)[0]
        offset = offset + 4

        infoSize = struct.unpack_from("<i",buf,offset)[0]
        offset = offset + 4

        totalMemory = struct.unpack_from("<i",buf,offset)[0]
        offset = offset + 4

        backtraceSize = struct.unpack_from("<i",buf,offset)[0]
        offset = offset + 4
        printDebug("overallSize = {0};infoSize = {1},totalMemory = {2},backtraceSize = {3}".format(overallSize,infoSize,totalMemory,backtraceSize))

        meminfoSize = overallSize + 4 * 4
        while offset < meminfoSize:
            offset = self.printEntry(buf,offset,infoSize)

    def parse(self,fileName = None):
        expr = None
        if fileName :
            f = open(fileName,"rb")
            expr = lambda x : f.read(x)
        else:
            mysocket = socket.create_connection(("127.0.0.1",3244))
            expr = lambda x : mysocket.recv(x)
        buf = []
        while True:
            buf2 = expr(4096*1024)
            if not len(buf2):
                break
            buf.append(buf2)

        self.handleBuf("".join(buf))
    

if __name__ == '__main__':
    fileName = None
    if len(sys.argv) > 1:
        fileName = sys.argv[1]
    p = Parser()
    p.parse(fileName)
    for e in p.outData:
        printInfo(e.size,e.allocations,e.backtraces._backtraces)
        
