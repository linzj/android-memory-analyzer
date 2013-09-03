
HTMLTemplateStart = """
<html>
<head>
<style type="text/css">
  ul.LinkedList { display: block; width: 99999;}
  /* ul.LinkedList ul { display: none; } */
  .HandCursorStyle { cursor: pointer; cursor: hand; }  /* For IE */
</style>
</head>
<body>
<ul id="LinkedList1" class="LinkedList">
<li>Object Hierarchy

"""

HTMLTemplateEnd = """
</li><ul>
</body>
</html>
"""

def writeNodeDFS(r,outStream):
    stk = [[r,0,0]]
    if r.children_:
        stk[-1][2] = len(r.children_)
    while stk:
        n = stk[-1][0]
        start = stk[-1][1]
        end = stk[-1][2]
        if not start:
            print >>outStream,'<ul><li>{0} weight {1} pointer 0x{2:08x}'.format(n.typeTuple_[0],n.totalWeight_,n.addr)
            #if len(n.typeTuple_) > 1 and n.typeTuple_[1]:
            #    for l in n.typeTuple_[1]:
            #        print >>outStream,'<br/>' + ' '.join(l[1:])
            #elif n.backtraces:
            #    for be in n.backtraces:
            #        print >>outStream,'<br/>{0:08X}'.format(be)
        if start == end:
            stk.pop()
            print >>outStream,'</li></ul>'
        else:
            stk[-1][1] = start + 1
            child = n.children_[start]
            end = 0
            if child.children_:
                end = len(child.children_)
            stk.append([n.children_[start],0,end])




class HTMLHierarchyWriter(object):

    def __init__(self):
        pass

    def write(self,rootList,outStream):
        outStream.write(HTMLTemplateStart)
        for r in rootList:
            writeNodeDFS(r,outStream)
        outStream.write(HTMLTemplateEnd)

