
def writeNodeDFS(r,outStream):
    stk = [[r,0,0]]
    if r.children_:
        stk[-1][2] = len(r.children_)
    tabNodes = []

    while stk:
        n = stk[-1][0]
        start = stk[-1][1]
        end = stk[-1][2]
        if not start:
            print >>outStream,''.join(tabNodes + ['{0} weight {1} pointer 0x{2:08x}'.format(n.typeTuple_[0],n.totalWeight_,n.addr) ] )
            #if len(n.typeTuple_) > 1 and n.typeTuple_[1]:
            #    for l in n.typeTuple_[1]:
            #        print >>outStream,'<br/>' + ' '.join(l[1:])
            #elif n.backtraces:
            #    for be in n.backtraces:
            #        print >>outStream,'<br/>{0:08X}'.format(be)
        if start == end:
            stk.pop()
            if tabNodes:
                tabNodes.pop()
        else:
            stk[-1][1] = start + 1
            child = n.children_[start]
            end = 0
            if child.children_:
                end = len(child.children_)
            stk.append([n.children_[start],0,end])
            tabNodes.append('\t')




class TextHierarchyWriter(object):

    def __init__(self):
        pass

    def write(self,rootList,outStream):
        for r in rootList:
            writeNodeDFS(r,outStream)


