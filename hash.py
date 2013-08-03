class backtrace_element(object):
    def __init__(self,backtraces):
        self._backtraces = backtraces

    def __eq__(self,other):
        address1 = self._backtraces
        address2 = other._backtraces
        if len(address1) != len(address2) :
            return False
        it1 = address1.__iter__()
        it2 = address2.__iter__()
        try:
            while True:
                if it1.next() != it2.next():
                    return False
        except StopIteration as e:
            pass

        return True

    def __hash__(self):
        if  not hasattr(self, '_cachedHash'):
            num = 0
            for x in self._backtraces:
                num = num ^ x
            self._cachedHash = num
        return self._cachedHash

