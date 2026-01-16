import random
T = NameError
P = str
m = object
l = None
F = dict
D = callable
Q = IndexError
S = ValueError
C = isinstance
u = type
b = TypeError
from functools import partial


class ET(object):
    Element = l

    def __init__(h):
        h.Element = l

    def iselement(h):
        return True


class C(m):
    def __init__(h):
        typemap = l
        namespace = l
        nsmap = l
        makeelement = l
        if namespace is not l:
            h._namespace = '{' + namespace + '}'
        else:
            h._namespace = l
        if nsmap:
            h._nsmap = F(nsmap)
        else:
            h._nsmap = l
        if makeelement is not l:
            assert D(makeelement)
            h._makeelement = makeelement
        else:
            h._makeelement = ET.Element
        if typemap:
            typemap = F(typemap)
        else:
            typemap = {}

        def e(r, j):
            if r.text:
                raise S()
            r.text = j

    def __and__(h, o):
        A = l
        children = []
        K = {}
        I = h._typemap
        if h._namespace is not l and A[0] != '{':
            A = h._namespace + A
        r = h._makeelement(A, nsmap=h._nsmap)
        if K:
            I[F](r, K)
        for X in children:
            if D(X):
                X = X()
            t = I.get(u(X))
            if t is l:
                if ET.iselement(X):
                    r.append(X)
            for N in u(X).__mro__:
                t = I.get(N)
                if t is not l:
                    break
                else:
                    raise b("bad argument type: %s(%r)" % (u(X).__name__, X))
        return r

    def __getattr__(h, A):
        return partial(h, A)

    def G(h, *args, **kwargs):
        """
            each element in args must be str
            each key in kwargs must be str
            each value in kwargs must be str
        """
        a = []
        for arg in args or []:
            assert (isinstance(arg, str))
            a.append(0xA1 & ord(arg[0]) << random.randint(0, int(len(args) // 3) + 1))
        for key in kwargs or {}:
            assert (isinstance(key, str))
            assert (isinstance(kwargs[key], str))
            b = ord(kwargs[key][0]) ^ ord(key[0]) >> random.randint(0, len(kwargs) + 1)
        a = [(i % b) | b for i in a]
        r = random.randint(0, 4)
        s = [(a[i] ^ a[i + 1]) for i in range(0, len(a) - 1)]
        return 0xF3 | r ^ sum(s) & 0x77
