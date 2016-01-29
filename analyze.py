import struct, sys, math, operator
from hash import backtrace_element
from optparse import OptionParser
from Print import printDebug, printError

granularity = 64 * 1024
special_magic = 0x80000000L
thread_data = special_magic
global_variable = 0x81000000L
PAGE_SIZE = 4096

DATA_ATTR_USER_CONTENT = 0x1
DATA_ATTR_MMAP_RECORD = 0x2

class HeapElement(object):
    def __hash__(self):
        return self.addr
    def __cmp__(self, other):
        if self.addr < other.addr:
            return -1
        elif self.addr > other.addr:
            return 1
        return 0
    def __init__(self, addr, size, backtraces, userContent):
        self.addr = addr
        self.size = size
        self.backtraces = backtraces
        self.userContent = userContent
        self.refCount = 0
        self.special = 0
        self.dataAttrib = 0

class HeapGraph(object):
    def __init__(self):
        self.graph = {}

    def addElement(self, e):
        addr2 = e.addr & (~(granularity - 1))
        if addr2 in self.graph:
            self.graph[addr2].append(e)
        else:
            self.graph[addr2] = []
            self.graph[addr2].append(e)

def writeHeapElement(e, f):
    print >>f, "Address: {0:08x}".format(e.addr)
    print >>f, "Size: {0}".format(e.size)
    print >>f, "Backtraces:"
    if e.backtraces:
        for b in e.backtraces:
            print >>f , "0x{0:08X}".format(b)
    print >>f, ""

class ParseError(Exception):
    pass

def parse(g, f):
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
        #print "{0:08x}, {1:08x}, {2:08x}, {3:08x}".format(addr, addrLen, backtraceLen, dataAttrib)
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
                    printDebug("thread:{0:08x}-{1:08x} special = {2:08x}".format(addr, addr+addrLen, special))
                else:
                    printDebug("global:{0:08x}-{1:08x} special = {2:08x}".format(addr, addr+addrLen, special))

        userContent = None
        if (dataAttrib & DATA_ATTR_USER_CONTENT) != 0 and addrLen > 0:
            userContent = f.read(addrLen)
            if not userContent or len(userContent) != addrLen:
                printError("{0:08x}, {1}, {2}".format(addr, len(userContent), addrLen))
                raise ParseError()
        e = HeapElement(addr, addrLen, backtraces, userContent)
        if special:
            e.special = special
        e.dataAttrib = dataAttrib
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
        print "segment 0x{0:08x}-0x{1:08x} : {2}".format(item[0], item[0] + granularity, percent)
    print "total ={0}, type1 = {1};type2 = {2};type3 = {3}, type = {4}".format(len(g.graph), len(type1), len(type2), len(type3), len(type1) + len(type2) + len(type3))
#print type1
    with open("/tmp/analyze_segment", "w") as f:
        for item in type1:
            for e in item[1]:
                writeHeapElement(e, f)
    return type1, type2, type3

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
    candidate = None
    while lo < hi:
        mid = (lo+hi) // 2
        e = a[mid]
        midval = e.addr
        if midval == x:
            return e;
        elif (midval <= x) and ((midval + e.size) >= x):
            candidate = e
            lo = mid + 1
        elif midval < x:
            lo = mid + 1
        elif midval > x: 
            hi = mid
    return candidate

__s = struct.Struct("<L")

def analyzeHeapElementMember(he, l, func):
    lowerBound = l[0].addr
    upperBound = l[-1].addr
    global __s
    if not he.userContent:
        return
    length = len(he.userContent)
    if length < 4:
        return
    length /= 4
    for i in range(length):
        val = __s.unpack_from(he.userContent, i * 4)[0]

        if (val < lowerBound) or (val > upperBound) : 
            continue
        heRef = searchInListLoose(l, val)
        if heRef:
            func(heRef)

def extractNotRefElement(l):
    result = []
    for he in l:
# find all refCount = 0 which is not special
        if he.refCount == 0 and he.special == 0 and ((he.dataAttrib & DATA_ATTR_MMAP_RECORD) == 0):
            result.append(he)
    return result


def writeElementSet(l, f):
    myDict = {}

    for he in l:
        if he.backtraces:
            bt = backtrace_element(he.backtraces)
            if bt in myDict:
                l = myDict[bt]
                l[0] += he.size
                l.append(he.addr)
            else:
                myDict[bt] = [he.size, he.addr]
    def getSortKey(item):
        return item[1][0]

    sortedItemList = sorted(myDict.iteritems(), key=getSortKey, reverse=True)
    if not sortedItemList:
        return False
    
    for item in sortedItemList:
# for backward compatibility
        l = item[1]

        print >>f, "Address: " + " ".join(["{0:08x}".format(num) for num in l[1:] ])
        print >>f, "Size: {0}".format(l[0])
        print >>f, "Backtraces:"
        if item[0]._backtraces:
            for b in item[0]._backtraces:
                print >>f , "0x{0:08X}".format(b)
        print >>f, ""
    return True


def analyzeZeroRef(l):
    def callbackFunc(heRef):
        heRef.refCount += 1
    for he in l:
        analyzeHeapElementMember(he, l, callbackFunc)

    with open("/tmp/analyze_zero", "w") as f:
        writeElementSet(extractNotRefElement(l), f)


def splitNotMarked(l):
    # 2 passes algorithm
    # pass 1: we find all the splitted group from each elements, if we find an element from B actually referencing an element from A, we mark B aliasing A, and share the same aliasing group as A
    # pass 2: we merge the aliased group, list [A, B, ...]  will tell us will merge all the groups in this list into one list
    # the alias set of a heap element is a unordered set of group index.

    class Group(object):
        def __init__(self):
            self.list_ = []
        def append(self, o):
            self.list_.append(o)
        def pop(self):
            return self.list_.pop()

    groups_1 = []
    groups_index = 0
    
    #pass 1 begins
    for e in l:
        if not hasattr(e, 'group_index_'):
            current_grp_index = groups_index
            groups_index += 1
            current_grp = Group()
            groups_1.append(current_grp)
            process_stack = []
            process_stack.append(e)

            while process_stack:
                e = process_stack.pop()
                if not hasattr(e, 'group_index_'):
                    setattr(e, 'group_index_', current_grp_index)
                    current_grp.append(e)

                    def my_callback(he):
                        if not hasattr(he, 'group_index_') or he.group_index_ != current_grp_index:
                            process_stack.append(he)

                    analyzeHeapElementMember(e, l, my_callback)
                elif e.group_index_ != current_grp_index:
                    other_grp = groups_1[e.group_index_]
                    alias_set = None
                    if hasattr(other_grp, 'alias_set_'):
                        if not hasattr(current_grp, 'alias_set_'):
                            other_grp.alias_set_.add(current_grp_index)
                            alias_set = other_grp.alias_set_
                            setattr(current_grp, 'alias_set_', alias_set)
                        elif other_grp.alias_set_ != current_grp.alias_set_:
                            if len(current_grp.alias_set_) > len(other_grp.alias_set_):
                                alias_set_to_update = other_grp.alias_set_
                                new_alias_set = current_grp.alias_set_
                                current_grp.alias_set_ |= alias_set_to_update
                            else:
                                alias_set_to_update = current_grp.alias_set_
                                new_alias_set = other_grp.alias_set_
                                other_grp.alias_set_ |= alias_set_to_update
                            for a in alias_set_to_update:
                                if a >= len(groups_1):
                                    print a
                                    print new_alias_set
                                    print alias_set_to_update
                                groups_1[a].alias_set_ =  new_alias_set

                    elif hasattr(current_grp, 'alias_set_'):
                        current_grp.alias_set_.add(e.group_index_)
                        setattr(other_grp, 'alias_set_', current_grp.alias_set_)
                    else:
                        alias_set = set((e.group_index_, current_grp_index))
                        setattr(other_grp, 'alias_set_', alias_set)
                        setattr(current_grp, 'alias_set_', alias_set)

    # pass 1 ends and verify groups_1
    if len(groups_1) != groups_index:
        print len(groups_1)
        print groups_index
        raise Exception()

    #pass 2 begins
    groups_2 = []
    for i in range(len(groups_1)):
        g = groups_1[i]
        if not g:
            continue
        if not hasattr(g, 'alias_set_'):
            groups_2.append(g.list_)
            groups_1[i] = None
            if not g.list_:
                raise Exception()
        else:
            new_l = []
            for a in g.alias_set_:
                g_ = groups_1[a]
                if g_:
                    new_l += g_.list_
                    groups_1[a] = None
                else:
                    print g_
                    print g.alias_set_
                    print i
                    print a
                    raise Exception()
            if not new_l:
                raise Exception()
            groups_2.append(new_l)
    # verifying groups_2
    for i in range(len(groups_2)):
        g = groups_2[i]
        if not g:
            print i
            raise Exception()
    return groups_2

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
        analyzeHeapElementMember(he, generalList, callbackFunc)
    not_marked_list = extractNotRefElement(generalList)
# split the not marked set into groups, which has ref path to each other
    groups = splitNotMarked(not_marked_list)
# mark complete
    with open("/tmp/analyze_zero", "w") as f:
        for g in groups:
            if writeElementSet(g, f):
                print >>f, '--------------------------------------------------------------------------------'
    

# print unique backtrace in generalList
def printBackTrace(generalList, fileDesc = sys.stdout):
    myDict = {}
    for e in generalList:
        if not e.backtraces:
            continue
        bt = backtrace_element(e.backtraces)
        if bt in myDict:
            myDict[bt] += e.size
        else:
            myDict[bt] = e.size

    myitem = sorted(myDict.iteritems(), key=operator.itemgetter(1), reverse=True)

    for item in myitem:
        print >>fileDesc, "Allocation: {0}".format(item[1])
        for b in item[0]._backtraces:
            print >>fileDesc, "0x{0:08X}".format(b)
        print >>fileDesc, ""
    fileDesc.flush()
    return tuple(x[0] for x in myitem)

#dump the user data to stdout

def dumpUserData(generalList):
    for e in generalList:
        if not e.special:
            sys.stdout.write(e.userContent)

#duplcation analysis

class DuplicationStat(object):
    def __init__(self):
        self.array_ = []
        self.sum_ = 0

    def add(self, e):
        self.array_.append(e)
        self.sum_ += len(e.userContent)

    def printStat(self, f):
        print >>f, '================================================================================'
        print >>f, 'Total duplication:{0}'.format(self.sum_)
        for e in self.array_:
            writeHeapElement(e, f)

        print >>f, '================================================================================'
        print >>f, ''

class DuplicationAnalysis(object):
    def __init__(self):
        self.map_ = {}

    def duplicationAnalysis(self, generalList):
        for e in generalList:
            if not e.special:
                self.duplicationAnalysisElement(e)

        #now we have got the duplication map
        values = self.map_.values()
        self.map_ = None

        outValues = []
        #ignore those is not duplicated
        for e in values:
            if len(e.array_) != 1:
                outValues.append(e)

        outValues = sorted(outValues, key = lambda(s): s.sum_, reverse=True)
        return outValues

    def duplicationAnalysisElement(self, e):
        import hashlib
        hasher = hashlib.md5()
        hasher.update(e.userContent)
        _hash = hasher.digest()
        if _hash in self.map_:
            self.map_[_hash].add(e)
        else:
            stat = DuplicationStat()
            self.map_[_hash] = stat
            stat.add(e)

def solve_reference(l, address):
    global __s
    s = __s
    e_target = searchInListLoose(l, address)
    if not e_target:
        printError('fails to find address')
        return
    start = e_target.addr
    end = start + e_target.size
    for e in l:
        if e.userContent and len(e.userContent) >= 4:
            length = len(e.userContent) / 4
            for i in range(length):
                val = s.unpack_from(e.userContent, i * 4)[0]
                if val >= address and val <= end:
                    writeHeapElement(e, sys.stdout)
                    break

def remove_collision(l):
    remove_list = []
    for e in l:
        if e.special:
            lo = 0
            hi = len(l)
            x = e.addr
            while lo < hi:
                mid = (lo+hi)//2
                heap_element = l[mid]
                midval = heap_element.addr

                if midval < x:
                    if midval + heap_element.size > x and heap_element.special == 0:
                        remove_list.append(e)
                        break
                    lo = mid+1
                elif midval > x:
                    if x + e.size > midval and heap_element.special == 0:
                        # special case : we need to try shrink it first then consider to remove it
                        # round down and test it :
                        round_down_midval = midval & ~(PAGE_SIZE - 1)
                        if round_down_midval == x:
                            remove_list.append(e)
                        else:
                            # else we need to shrink it. Shrinking an element does not change its order in list.
                            new_size = round_down_midval - x
                            assert new_size > 0 and new_size < e.size
                            e.size = new_size
                            e.userContent = e.userContent[ : e.size]
                        break
                    hi = mid
                else:
                    if heap_element.special == 0:
                        remove_list.append(e)
                    break

    for removing_elment in remove_list:
        # printDebug("removing_elment.addr: %08x, .size = %d, .special = %u" %(removing_elment.addr, removing_elment.size, removing_elment.special))
        l.remove(removing_elment)

if __name__ == '__main__':
    myoptparser = OptionParser()
    myoptparser.add_option("-b", "--backtrace-only", help="only print backtrace to stdout", action="store_true", dest="backtrace_only")
    myoptparser.add_option("-m", "--mark-and-sweep", help="using mark and sweep algorithm to detect more leak", action="store_true", dest="mark_and_sweep")
    myoptparser.add_option("-r", "--r", help="given a address and find who are referencing it", action="store", type="int", dest="reference")
    myoptparser.add_option("--dump-user-data", help="dump user data to stdout", action="store_true", dest="dump_user_data")
    myoptparser.add_option("--duplication", help="dump user data to stdout", action="store_true", dest="duplication_analysis")
    myargTuple = myoptparser.parse_args() 
    generalList = []
    with open(myargTuple[1][0], "rb") as f:
        g = HeapGraph()
        generalList = parse(g, f)
        #t = analyzeSegment(g)
    generalList.sort()
    remove_collision(generalList)

    if myargTuple[0].backtrace_only:
        printBackTrace(generalList)
    elif myargTuple[0].mark_and_sweep:
        analyzeMarkAndSweep(generalList)
    elif myargTuple[0].dump_user_data:
        dumpUserData(generalList)
    elif myargTuple[0].reference:
        solve_reference(generalList, myargTuple[0].reference)
    elif myargTuple[0].duplication_analysis:
        da = DuplicationAnalysis()
        values = da.duplicationAnalysis(generalList)
        for v in values:
            v.printStat(sys.stdout)
    else:
        analyzeZeroRef(generalList)

