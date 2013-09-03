#!/usr/bin/python

import analyze,subprocess,ObjectParser,threading,struct,sys
from optparse import OptionParser
from Print import printError,printDebug,setDebug
from hash import backtrace_element

def getHeapRefCount(he):
    return he.refCount

def iterateHeapElementMemberDF(he,l,func,s):
    lowerBound = l[0].addr
    upperBound = l[-1].addr
    if not he.userContent or hasattr(he,'marked_'):
        return
    length = len(he.userContent)
    if length < 4:
        return
    dfsStack = [[he,0]]
    while dfsStack:
        tmpTuple = dfsStack[-1]
        he = tmpTuple[0]
        start = tmpTuple[1]
        setattr(he,'marked_',1)

        if not hasattr(he,'childrenForMark_') and he.userContent and len(he.userContent) > 4:
            end = len(he.userContent) / 4
            children = []
            for i in range(end):
                val = s.unpack_from(he.userContent, i * 4)[0]

                if (val < lowerBound) or (val > upperBound) : 
                    continue
                heRef = analyze.searchInListLoose(l,val)
                if heRef and not hasattr(heRef,'marked_'):
                    children.append(heRef)
            children = sorted(children,key=getHeapRefCount)
            setattr(he,'childrenForMark_',children)

        for i in range(start,len(he.childrenForMark_)):
            heRef = he.childrenForMark_[i]
            if not hasattr(heRef,'marked_'):
                if heRef.userContent and len(heRef.userContent) > 4:
                    #in this case need to push to the stack top
                    tmpTuple[1] = i + 1
                    dfsStack.append([heRef,0])
                    break
                else:
                    # mark & iterate it now
                    setattr(heRef,'marked_',1)
                    func(heRef,he)
                    #print >>sys.stderr,'mark the no children node'

        if dfsStack[-1][0] == he:
            # no more child iterate it now
            dfsStack.pop()
            if len(dfsStack) >= 1:
                # own a parent
                func(he,dfsStack[-1][0])
                #print >>sys.stderr,'mark the used up children node'
            else:
                func(he,None)
                #print >>sys.stderr,'mark the no parent node'


def assignExtendAttr(node,BOMapping):
    if not hasattr(node,'children_'):
        setattr(node,'children_',None)
        setattr(node,'totalWeight_',len(node.userContent))
        #search for object type
        typeTuple = ('Unknown',)
        if node.backtraces:
            bt = backtrace_element(node.backtraces)
            if bt in BOMapping:
                typeTuple = BOMapping[bt]
        setattr(node,'typeTuple_',typeTuple)
    

def analyzeHeapElement(l,BOMapping):
    def iterateFunc(node,parentNode):
        assignExtendAttr(node,BOMapping)
        if not parentNode:
            return 
        assignExtendAttr(parentNode,BOMapping)
        if parentNode.children_ == None:
            parentNode.children_ = []
        parentNode.children_.append(node)
        parentNode.totalWeight_ += node.totalWeight_
        setattr(node,'hasParent_',True)

    s = struct.Struct("<L")
    iterateList = sorted(l,key=getHeapRefCount)
    #print >>sys.stderr,'iterateList[0].refCount = {0}'.format(iterateList[0].refCount)
    for he in iterateList:
        iterateHeapElementMemberDF(he,l,iterateFunc,s)
    

def analyzeZeroRefNotWrite(l):
    def callbackFunc(heRef):
        heRef.refCount += 1
    for he in l:
        analyze.analyzeHeapElementMember(he,l,callbackFunc)
    #writeRefZeroAndNotSpecial(l)


if __name__ == '__main__':

    myoptparser = OptionParser()

    myoptparser.add_option("-m","--map-file",help="assign map file",action="store", type="string", dest="mapfile")
    myoptparser.add_option("-s","--symbol-path",help="assign symbol path",action="store", type="string", dest="sympath")
    myoptparser.add_option("-w","--writer",help="set a writer by name",action="store", type="string", dest="writer")
    
    myargTuple = myoptparser.parse_args() 

    if not myargTuple[0].mapfile:
        printError("need to use -m to specify map file")

    if not myargTuple[0].sympath:
        printError('need to use -s to specify sym file')

    #initialize generalList
    with open(myargTuple[1][0],"rb") as f:
        g = analyze.HeapGraph()
        generalList = analyze.parse(g,f)
    
    generalList.sort()
    del g
    #kick off address filter to get line numbers for object analysis
    process = subprocess.Popen(("python","AddressFilter.py",myargTuple[0].mapfile,myargTuple[0].sympath),stdout=subprocess.PIPE,stdin=subprocess.PIPE)
    
    def thread_call_back(returnList):
        sortedBackTrace = analyze.printBackTrace(generalList,process.stdin)
        process.stdin.close()
        returnList.append(sortedBackTrace)
    #start thread to fill subprocess
    returnList = []
    workerThread = threading.Thread(target=thread_call_back,name='backtrace_print',kwargs={'returnList':returnList})
    workerThread.start()
    
    #initialize object analysis session
    callBack = ObjectParser.MyCallback()
    myParser = ObjectParser.Parser()
    myParser.parseStream(process.stdout,callBack)
    IOMapping = callBack.IOMapping_
    # backtrace-object mapping
    BOMapping = {}
    # get backtrace-object mapping
    sortedBackTrace = returnList[0]
    for i in IOMapping.items():
        bt = sortedBackTrace[i[0]]
        BOMapping[bt] = i[1]
    # analyze the heap elements
    analyzeZeroRefNotWrite(generalList)
    analyzeHeapElement(generalList,BOMapping)
    rootNodes = ( i for i in generalList if not hasattr(i,'hasParent_') )
    if not myargTuple[0].writer:
        for rootNode in rootNodes:
            print rootNode.typeTuple_
    else:
        writer = myargTuple[0].writer
        execSrc = """
from {0} import {0}
mywriter = {0}()
mywriter.write(rootNodes,sys.stdout)
""".format(writer)
        exec(execSrc)

    
        
