import struct,sys,math,operator
from hash import backtrace_element
from optparse import OptionParser
from Print import printDebug,printError

granularity = 64 * 1024
special_magic = 0x80000000L
thread_data = special_magic
global_variable = 0x81000000L

class HeapElement(object):
    def __hash__(self):
        return self.addr
    def __cmp__(self,other):
        if self.addr < other.addr:
            return -1
        elif self.addr > other.addr:
            return 1
        return 0
    def __init__(self,addr,size,backtraces,userContent):
        self.addr = addr
        self.size = size
        self.backtraces = backtraces
        self.userContent = userContent
        self.refCount = 0
        self.special = 0

class HeapGraph(object):
    def __init__(self):
        self.graph = {}

    def addElement(self,e):
        addr2 = e.addr & (~(granularity - 1))
        if addr2 in self.graph:
            self.graph[addr2].append(e)
        else:
            self.graph[addr2] = []
            self.graph[addr2].append(e)

def writeHeapElement(e,f):
    print >>f,"Address: {0:08x}".format(e.addr)
    print >>f,"Size: {0}".format(e.size)
    print >>f,"Backtraces:"
    if e.backtraces:
        for b in e.backtraces:
            print >>f , "0x{0:08X}".format(b)
    print >>f,""

class ParseError(Exception):
    pass

DATA_ATTR_USER_CONTENT = 0x1

def parse(g,f):
    s = struct.Struct("<L")
    st = struct.Struct("<LLLL")
    generalList = []
    while True:
        Buf = f.read(16)
        if not Buf or len(Buf) != 16:
            break
        t = st.unpack(Buf)
        addr = t[0]
        addrLen = t[1]
        backtraceLen = t[2]
        dataAttrib = t[3]
        backtraces = None
        special = 0
        #print "{0:08x},{1:08x},{2:08x},{3:08x}".format(addr,addrLen,backtraceLen,dataAttrib)
        if (backtraceLen > 0) and ((backtraceLen & special_magic)  == 0):
            backtraces = []
            for i in range(backtraceLen):
                backtraceElementBuf = f.read(4)
                if not backtraceElementBuf or len(backtraceElementBuf) != 4:
                    raise ParseError()
                backtraceElement = s.unpack(backtraceElementBuf)
                backtraces.append(backtraceElement[0])
        else:
            #thread data or global variable
            special = backtraceLen
            if special:
                if special == thread_data:
                    printDebug("thread:{0:08x}-{1:08x}".format(addr,addr+addrLen))
                else:
                    printDebug("global:{0:08x}-{1:08x} special = {2}".format(addr,addr+addrLen,special))

        userContent = None
        if (dataAttrib & DATA_ATTR_USER_CONTENT) != 0 :     
            userContent = f.read(addrLen)
            if not userContent or len(userContent) != addrLen:
                printError("{0:08x},{1},{2}".format(addr,len(userContent),addrLen))
                raise ParseError()
        e = HeapElement(addr,addrLen,backtraces,userContent)
        if special:
            e.special = special
        g.addElement(e)
        generalList.append(e)
    return generalList


def analyzeSegment(g):
    type1 = [] 
    type2 = []
    type3 = []
    for item in g.graph.items():
        mySum = 0
        for e in item[1]:
            mySum += e.size
        percent = float(mySum) / granularity
        if percent > 1.0:
            continue
        if percent <= 0.1:
            type1.append(item)
        elif percent <= 0.2:
            type2.append(item)
        elif percent <= 0.3:
            type3.append(item)
        print "segment 0x{0:08x}-0x{1:08x} : {2}".format(item[0],item[0] + granularity,percent)
    print "total ={0},type1 = {1};type2 = {2};type3 = {3},type = {4}".format(len(g.graph),len(type1),len(type2),len(type3),len(type1) + len(type2) + len(type3))
#print type1
    with open("/tmp/analyze_segment","w") as f:
        for item in type1:
            for e in item[1]:
                writeHeapElement(e,f)
    return type1,type2,type3

def searchInListStrict(a, x, lo=0, hi=None):
    if hi is None:
        hi = len(a)
    while lo < hi:
        mid = (lo+hi)//2
        midval = a[mid].addr
        if midval < x:
            lo = mid+1
        elif midval > x: 
            hi = mid
        else:
            return a[mid]
    return None

def searchInListLoose(a, x, lo=0, hi=None):
    if hi is None:
        hi = len(a)
    while lo < hi:
        mid = (lo+hi)//2
        e = a[mid]
        midval = e.addr
        if (midval <= x) and ((midval + e.size) >= x):
            return e;
        if midval < x:
            lo = mid+1
        elif midval > x: 
            hi = mid
    return None

def analyzeHeapElementMember(he,l,func):
    lowerBound = l[0].addr
    upperBound = l[-1].addr
    s = struct.Struct("<L")
    if not he.userContent:
        return
    length = len(he.userContent)
    if length < 4:
        return
    length /= 4
    for i in range(length):
        val = s.unpack_from(he.userContent, i * 4)[0]

        if (val < lowerBound) or (val > upperBound) : 
            continue
        heRef = searchInListLoose(l,val)
        if heRef:
            func(heRef)

def writeRefZeroAndNotSpecial(l):
    with open("/tmp/analyze_zero","w") as f:
        bset = set()

        for he in l:
# find all refCount = 0
            if he.refCount == 0 and he.special == 0:
                if he.backtraces:
                    bt = backtrace_element(he.backtraces)
                    if bt in bset:
                        continue
                    bset.add(bt)
                writeHeapElement(he,f)


def analyzeZeroRef(l):
    def callbackFunc(heRef):
        heRef.refCount += 1
    for he in l:
        analyzeHeapElementMember(he,l,callbackFunc)
    writeRefZeroAndNotSpecial(l)


def analyzeMarkAndSweep(generalList):
    markStack = []
# construct the strong roots
    for e in generalList:
        if e.special:
            e.refCount = 1 #actually a mark
            markStack.append(e)

    def callbackFunc(he):
        if not he.refCount:
            he.refCount = 1 #actually a mark
            markStack.append(he)

    while markStack:
        he = markStack.pop()
        analyzeHeapElementMember(he,generalList,callbackFunc)
# mark complete
    writeRefZeroAndNotSpecial(generalList)
    

# print unique backtrace in generalList
def printBackTrace(generalList,fileDesc = sys.stdout):
    myDict = {}
    for e in generalList:
        if not e.backtraces:
            continue
        bt = backtrace_element(e.backtraces)
        if bt in myDict:
            myDict[bt] += e.size
        else:
            myDict[bt] = e.size

    myitem = sorted(myDict.iteritems(), key=operator.itemgetter(1),reverse=True)

    for item in myitem:
        print >>fileDesc,"Allocation: {0}".format(item[1])
        for b in item[0]._backtraces:
            print >>fileDesc,"0x{0:08X}".format(b)
        print >>fileDesc,""
    fileDesc.flush()
    return tuple(x[0] for x in myitem)


if __name__ == '__main__':
    myoptparser = OptionParser()
    myoptparser.add_option("-b","--backtrace-only",help="only print backtrace to stdout",action="store_true",dest="backtrace_only")
    myoptparser.add_option("-m","--mark-and-sweep",help="only print backtrace to stdout",action="store_true",dest="mark_and_sweep")
    myargTuple = myoptparser.parse_args() 
    generalList = []
    with open(myargTuple[1][0],"rb") as f:
        g = HeapGraph()
        generalList = parse(g,f)
        #t = analyzeSegment(g)
    generalList.sort()
    if myargTuple[0].backtrace_only:
        printBackTrace(generalList)
    elif myargTuple[0].mark_and_sweep:
        analyzeMarkAndSweep(generalList)
    else:
        analyzeZeroRef(generalList)

