#!/usr/bin/python
import sys,re,operator
from Print import printError,setDebug,printDebug 
from optparse import OptionParser

class Parser(object):
    def __init__(self):
        self.HitAddressStatus = 0
        self.myre = re.compile('^0x([0-9a-fA-F]+)\s((?:.(?!\s+---))+.)\s+---\s+((?:.(?!\s+---))+.)\s+---\s+(.*)$')
    def parseStream(self,stream,callBack):
        while True:
            line = stream.readline()
            if not line:
                break
            if self.HitAddressStatus == 0:
                if line.startswith('Allocation:'):
                    self.HitAddressStatus = 1
                    callBack.hitAllocation(long(line.rstrip()[11:]))
            elif self.HitAddressStatus == 1:
                process = line.strip()
                if not process:
#hit the end of this record
                    self.HitAddressStatus = 0
                    callBack.hitEnd()
                else:
                    match = self.myre.match(process)
                    if match:
                        callBack.hitBacktrace(long(match.group(1),16),match.group(2),match.group(3),match.group(4))
class WrongSemanticError(Exception):
    pass

class TypeParser(object):
    def __init__(self):
        self.semanticArray = []
        self.objectRe = re.compile('[a-zA-Z0-9_]+')
        pass
    
    def handleObjectTypeTag(self,l):
        semanticTuple = l.partition('__object_type_tag__')
        if not semanticTuple[1]:
            return False
        if '__object_type_tag__' in semanticTuple[2]:
#currently not supporting the recursive occur
            raise WrongSemanticError()
        semantic = (re.compile(semanticTuple[0]),re.compile(semanticTuple[2]))
        def callBack(line):
            leftStart = 0
            while leftStart != len(line):
                leftMatch = semantic[0].search(line,leftStart)
                if not leftMatch:
                    break
                middleStart = leftMatch.end()
                if leftMatch.start() == leftMatch.end():
                    middleStart += 1
                middleMatch = self.objectRe.match(line,middleStart)

                if not middleMatch:
                    leftStart = leftMatch.end()
                    if leftMatch.start() == leftMatch.end():
                        leftStart += 1
                    continue
                rightStart = middleMatch.end()
                rightMatch = semantic[1].match(line,rightStart)
                #if True:
                #    print ':'+ str(rightMatch)
                #    print line[rightStart:]
                #    print (semantic[0].pattern,semantic[1].pattern)
                if not rightMatch:
                    leftStart = leftMatch.end()
                    if leftMatch.start() == leftMatch.end():
                        leftStart += 1
                    continue
                # find the object here
                
                foundedName = line[middleMatch.start():middleMatch.end()]
                if foundedName == 'T':
                # this is a template!
                    return None
                printDebug("found named:{0}---{1}".format(foundedName,line))
                return foundedName
            return None
        callBack.hitCount_ = 0
        callBack.pattern_ = l
            
        self.semanticArray.append(callBack)
        return True

    def handleFunctionMapTag(self,l):
        semanticTuple = l.partition('__function_map_tag__')
        if not semanticTuple[1]:
            return False
        semantic = re.compile(semanticTuple[0])
        def callBack(line):
            m = semantic.search(line)
            if m:
                printDebug("found named:{0}---{1}".format(semanticTuple[2],line))
                return semanticTuple[2]

        callBack.hitCount_ = 0
        callBack.pattern_ = l

        self.semanticArray.append(callBack)
        return True

    def parseSemanticFile(self,stream):
        while True:
            l = stream.readline()
            if not l:
                break
            l = l.strip()
            if not l or l[0] == '#':
                continue
            if self.handleObjectTypeTag(l) or self.handleFunctionMapTag(l):
                pass
            else:
                printError('failed to parse semantic rule')
                raise WrongSemanticError()

        if not self.semanticArray:
            printError("No semantic string found")
            raise WrongSemanticError()

    def parseObjectName(self,line):
        for semantic in self.semanticArray:
            ret = semantic(line)
            if ret:
                semantic.hitCount_ +=1
                return ret 
        return None

class MyCallback(object):
    def __init__(self):
        self.objs_ = {}
        self.currentAllocation_ = 0
        self.foundObject_ = False
        self.backTraceHit_ = False
        self.typeParser_ = TypeParser()
        self.lines_ = []
        self.totalAllocation_ = 0
        self.objectAllocation_ = 0
        self.totalCount_ = 0
        self.objectCount_ = 0
        with open('semantic.cfg','r') as f:
            self.typeParser_.parseSemanticFile(f)

    def hitAllocation(self,size):
        self.currentAllocation_ = size
        self.totalAllocation_ += size
        self.totalCount_ += 1

    def hitBacktrace(self,addr,soPath,funcName,filePath):
        self.lines_.append([addr,soPath,funcName,filePath])
        if self.foundObject_:
            return
        self.backTraceHit_ = True
        name = self.parseObjectName(filePath)
        if not name:
            name = self.typeParser_.parseObjectName(funcName)
        if name:
            self.foundObject_ = True
            composedName = soPath + ':' + name 

            if composedName in self.objs_:
                self.objs_[composedName] += self.currentAllocation_
            else:
                self.objs_[composedName] = self.currentAllocation_
            self.objectAllocation_ += self.currentAllocation_
            self.objectCount_ += 1

    def parseObjectName(self,filePath):
        separatorIndex = filePath.rfind(':')
        if separatorIndex == -1:
            return None
        try:
            lineNum = int(filePath[separatorIndex + 1 :])
        except Exception as e:
            printDebug(filePath)
            return None
        try:
            with open(filePath[:separatorIndex]) as f:
                while True:
                    l = f.readline()
                    if not l:
                        break
                    lineNum -= 1
                    if not lineNum:
#hit here
                        return self.typeParser_.parseObjectName(l)
        except:
            return None # eat the exception

    def hitEnd(self):
        if not self.foundObject_ and self.backTraceHit_ and self.lines_ :
# no effectic semantic found
            printDebug('no effectic semantic found' + str(self.lines_))
            #raise WrongSemanticError()
        self.foundObject_ = False
        self.currentAllocation_ = 0
        self.lines_ = []

if __name__ == '__main__':
    setDebug(True)
    myoptparser = OptionParser()
    myargTuple = myoptparser.parse_args() 
    if not myargTuple[1]:
        printError("need a file to analyze")
    with open(myargTuple[1][0],'r') as f:
        callBack = MyCallback()
        myParser = Parser()
        myParser.parseStream(f,callBack)

        def getHitCount(s):
            return s.hitCount_
        
        
        #open the following statments to benchmark & optimize the semantic array order
        #for semantic in sorted(callBack.typeParser_.semanticArray,key = getHitCount,reverse = True):
        #    print '{0} : {1}'.format(semantic.pattern_,semantic.hitCount_)
        sortedObjectList = sorted(callBack.objs_.iteritems(), key=operator.itemgetter(1),reverse = True)
        printCount = 0
        for o in sortedObjectList:
            print "{0} : {1}".format(o[0],o[1])
            printCount += 1
            if printCount == 10:
                break
        print "objectAllocation / totalAllocation = {0}".format(float(callBack.objectAllocation_)/callBack.totalAllocation_)
        print "objectCount / totalCount = {0}".format(float(callBack.objectCount_)/callBack.totalCount_)
    

