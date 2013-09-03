import sys,re,operator

if __name__ == '__main__':
    myRe = re.compile('weight (\d{8,20})')
    lastHit = 0
    count = 1
    lastLine = 0
    window = 1024 * 1024 * 1
    dropList = []
    while True:
        line = sys.stdin.readline()
        if not line:
            break

        m = myRe.search(line)
        if m:
            num = int(m.group(1))
            if lastHit - num >  window:
                dropList.append((lastHit - num,'last hit is {0},last line is {1},current hit is {2},current line is {3}'.format(lastHit,lastLine,num,count)))
                lastHit = num
                lastLine = count
            elif not lastHit:
                lastHit = num
                lastLine = count

        count += 1

    dropList = sorted(dropList, key=operator.itemgetter(0),reverse = True)
    for d in dropList:
        print ':'.join(('dropping {0}'.format(d[0]),d[1]))
