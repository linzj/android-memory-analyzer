#!/usr/bin/python
import sys

if __name__ == '__main__':
    if len(sys.argv) != 2:
        print >>sys.stderr,"need a filter string ,separated by comma {0}".format(len(sys.argv))
        sys.exit(1)
    filterList = sys.argv[1].split(',')

    HitAddressStatus = 0 # 0 for address 1 for Size 3 for backtraces 4 for backtrace numbers
    buf = []
    NotPrint = False
    while True:
        line = sys.stdin.readline()
        if not line:
            break
        if HitAddressStatus == 0:
            if line.startswith('Address:'):
                HitAddressStatus = 1
            buf.append(line)
        elif HitAddressStatus == 1:
            if line.startswith('Size:'):
                HitAddressStatus = 2
                buf.append(line)
        elif HitAddressStatus == 2:
            if line.startswith('Backtraces:'):
                HitAddressStatus = 3
                buf.append(line)
        elif HitAddressStatus == 3:
            buf.append(line)
#do the filtering
            for myfilter in filterList:
             if myfilter in line:
                 NotPrint = True
                 break

            if not line.strip():
#hit the end of this record
                oldBuf = buf
                buf = []
                HitAddressStatus = 0
                if NotPrint:
                    NotPrint = False
                    continue
                for bufferedLine in oldBuf:
                    sys.stdout.write(bufferedLine)

