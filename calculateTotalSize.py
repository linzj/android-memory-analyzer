import sys,re
def main():
    myre = re.compile('^Size: (\d+).*')
    total = 0
    while True:
        line = sys.stdin.readline()
        if not line:
            break
        m = myre.match(line)
        if m:
            total += int(m.group(1))
    print "{0}".format(total)


if __name__ == '__main__':
    main()
