import struct,sys,math
from hash import backtrace_element

granularity = 64 * 1024

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
    for b in e.backtraces:
        print >>f , "0x{0:08X}".format(b)
    print >>f,""

class ParseError(Exception):
    pass

def parse(g,f):
    s = struct.Struct("<L")
    st = struct.Struct("<LLL")
    generalList = []
    while True:
        Buf = f.read(12)
        if not Buf or len(Buf) != 12:
            break
        t = st.unpack(Buf)
        addr = t[0]
        addrLen = t[1]
        backtraceLen = t[2]
        backtraces = []
        for i in range(backtraceLen):
            backtraceElementBuf = f.read(4)
            if not backtraceElementBuf or len(backtraceElementBuf) != 4:
                raise ParseError()
            backtraceElement = s.unpack(backtraceElementBuf)
            backtraces.append(backtraceElement[0])
        userContent = f.read(addrLen)
        if not userContent or len(userContent) != addrLen:
            raise ParseError()
        e = HeapElement(addr,addrLen,backtraces,userContent)
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

def searchInList(a, x, lo=0, hi=None):
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


def analyzeZeroRef(l):
    s = struct.Struct("<L")
    lowerBound = l[0].addr
    upperBound = l[-1].addr
    print "lowerBound {0:08x} ,upperBound {1:08x}".format(lowerBound,upperBound)
    for he in l:
        length = len(he.userContent)
        if length < 4:
            continue
        length /= 4
        for i in range(length):
            val = s.unpack_from(he.userContent, i * 4)[0]

            if (val < lowerBound) or (val > upperBound) : 
                continue
            heRef = searchInList(l,val)
            if heRef:
                heRef.refCount += 1

    with open("/tmp/analyze_zero","w") as f:
        bset = set()

        for he in l:
# find all refCount = 0
            if he.refCount == 0:
                bt = backtrace_element(he.backtraces)
                if bt in bset:
                    continue
                bset.add(bt)
                writeHeapElement(he,f)



            
    




if __name__ == '__main__':
    with open(sys.argv[1],"rb") as f:
        g = HeapGraph()
        generalList = parse(g,f)
        #t = analyzeSegment(g)
        generalList.sort()
        analyzeZeroRef(generalList)




