import re
import sys


if __name__ == "__main__":
    regexpr = re.compile('\\bm(\\w+)\\b')
    fileName = sys.argv[1]
    content = None
    
    with open(fileName,'r') as fr:
        content = fr.read()


    it = regexpr.finditer(content)
    match = None
    try:
        match = it.next()
    except StopIteration :
        print >>sys.stderr,"not found"
        pass
    if match:
        print >>sys.stderr,"found matching"
        with open(fileName,'w') as fw:
            start = 0
            try:
                while True:
                    group1 = match.group(1)
                    if group1[0].islower():
                        match = it.next()
                        continue
                    fw.write(content[start:match.start()])
                    print >>sys.stderr,"replacing {0},at {1}".format(group1,match.start())
                    fw.write('m_' + group1[0].lower() + group1[1:])
                    start = match.end()
                    match = it.next()
            except StopIteration:
                pass
            fw.write(content[start:])

                    

