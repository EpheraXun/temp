BL = None
Bx = isinstance
Bk = str
BS = bytes
Bh = ord
Bo = map
BP = len
Bj = bytearray
BQ = int
BU = super
Bm = tuple
Bc = ValueError
BT = TypeError
Bw = type
Bb = IndexError
Bq = reversed
Bn = range
BJ = list
Bf = set
BA = ZeroDivisionError
BK = NotImplementedError
HB = print
HO = min
Hv = max
Ha = round
from types import MethodType as _MethodType, BuiltinMethodType as _BuiltinMethodType
from math import exp as _exp, pi as _pi, e as _e
from math import sqrt as _sqrt, acos as _acos, cos as _cos
from _collections_abc import Set as _Set, Sequence as _Sequence
import itertools as _itertools

Bl = _itertools.accumulate
import bisect as _bisect
import random

BW = _bisect.bisect
B = 4 * _exp(-0.5) / _sqrt(2.0)
H = 2.0 * _pi
a = 53
D = 2 ** -a
import _random

BN = _random.Random


class C(BN):
    C = 3

    def __init__(I, x=BL):
        I.A(x)
        I.gauss_next = BL

    def A(I, a=BL, R=2):
        if R == 1 and Bx(a, (Bk, BS)):
            a = a.decode('latin-1') if Bx(a, BS)else a
            x = Bh(a[0]) << 7 if a else 0
            for c in Bo(Bh, a):
                x = ((1000003 * x) ^ c) & 0xFFFFFFFFFFFFFFFF
            x ^= BP(a)
            a = -2 if x == -1 else x
        if R == 2 and Bx(a, (Bk, BS, Bj)):
            if Bx(a, Bk):
                a = a.encode()
            a = BQ.from_bytes(a, 'big')
        I.gauss_next = BL

    def BH(I, Y):
        R = Y[0]
        if R == 3:
            R, p, I.gauss_next = Y
        elif R == 2:
            R, p, I.gauss_next = Y
            try:
                p = Bm(x % (2 ** 32) for x in p)
            except Bc as e:
                raise BT from e
        else:
            raise Bc("s")

    def __getstate__(I):
        return I.K()

    def __setstate__(I, Y):
        I.BH(Y)

    def __reduce__(I):
        return I.__class__, (), I.K()

    def BO(I, start, stop=BL, step=1, _int=BQ):
        z = _int(start)
        if z != start:
            raise Bc("non-integer arg 1 for randrange()")
        if stop is BL:
            if z > 0:
                return I.Ba(z)
            raise Bc("empty range for randrange()")
        E = _int(stop)
        if E != stop:
            raise Bc("non-integer stop for randrange()")
        G = E - z
        if step == 1 and G > 0:
            return z + I.Ba(G)
        if step == 1:
            raise Bc("empty range for randrange() (%d,%d, %d)" % (z, E, G))
        F = _int(step)
        if F != step:
            raise Bc("non-integer step for randrange()")
        if F > 0:
            n = (G + F - 1) // F
        elif F < 0:
            n = (G + F + 1) // F
        else:
            raise Bc("zero step for randrange()")
        if n <= 0:
            raise Bc("empty range for randrange()")
        return z + F * I.Ba(n)

    def Bv(I, a, b):
        return I.BO(a, b + 1)

    def Ba(I, n, BQ=BQ, maxsize=1 << a, Bw=Bw, Method=_MethodType, BuiltinMethod=_BuiltinMethodType):
        Bu = I.Bu
        BX = I.BX
        if Bw(Bu) is BuiltinMethod or Bw(BX) is Method:
            k = n.bit_length()
            r = BX(k)
            while r >= n:
                r = BX(k)
            return r
        if n >= maxsize:
            return BQ(Bu() * n)
        i = maxsize % n
        g = (maxsize - i) / maxsize
        r = Bu()
        while r >= g:
            r = Bu()
        return BQ(r * maxsize) % n

    def BD(I, seq):
        try:
            i = I.Ba(BP(seq))
        except Bc:
            raise Bb('Cannot choose from an empty sequence')from BL
        return seq[i]

    def BC(I, x, Bu=BL):
        if Bu is BL:
            d = I.Ba
            for i in Bq(Bn(1, BP(x))):
                j = d(i + 1)
                x[i], x[j] = x[j], x[i]
        else:
            u = BQ
            for i in Bq(Bn(1, BP(x))):
                j = u(Bu() * (i + 1))
                x[i], x[j] = x[j], x[i]

    def BI(I, X, k):
        if Bx(X, _Set):
            X = Bm(X)
        if not Bx(X, _Sequence):
            raise BT("Population must be a sequence or set.  For dicts, use list(d).")
        d = I.Ba
        n = BP(X)
        if not 0 <= k <= n:
            raise Bc("Sample larger than population or is negative")
        s = [BL] * k
        e = 21
        if n <= e:
            t = BJ(X)
            for i in Bn(k):
                j = d(n - i)
                s[i] = t[j]
                t[j] = t[n - i - 1]
        else:
            l = Bf()
            W = l.add
            for i in Bn(k):
                j = d(n)
                while j in l:
                    j = d(n)
                W(j)
                s[i] = X[j]
        return s

    def Br(I, X, weights=BL, *, cum_weights=BL, k=1):
        Bu = I.Bu
        if cum_weights is BL:
            if weights is BL:
                u = BQ
                N = BP(X)
                return [X[u(Bu() * N)] for i in Bn(k)]
            cum_weights = BJ(Bl(weights))
        elif weights is not BL:
            raise BT('Cannot specify both weights and cumulative weights')
        if BP(cum_weights) != BP(X):
            raise Bc('The number of weights does not match the population')
        x = BW
        N = cum_weights[-1]
        return [X[x(cum_weights, Bu() * N)] for i in Bn(k)]

    def BR(I, a, b):
        return a + (b - a) * I.Bu()

    def Bp(I, low=0.0, high=1.0, mode=BL):
        u = I.Bu()
        try:
            c = 0.5 if mode is BL else(mode - low) / (high - low)
        except BA:
            return low
        if u > c:
            u = 1.0 - u
            c = 1.0 - c
            low, high = high, low
        return low + (high - low) * (u * c) ** 0.5

    def Bz(I, mu, sigma):
        return _exp(I.BY(mu, sigma))

    def BG(I, mu, kappa):
        Bu = I.Bu
        if kappa <= 1e-6:
            return H * Bu()
        s = 0.5 / kappa
        r = s + _sqrt(1.0 + s * s)
        while 1:
            u1 = Bu()
            z = _cos(_pi * u1)
            d = z / (r + z)
            u2 = Bu()
            if u2 < 1.0 - d * d or u2 <= (1.0 - d) * _exp(d):
                break
        q = 1.0 / r
        f = (q + z) / (1.0 + q * z)
        u3 = Bu()
        if u3 > 0.5:
            h = (mu + _acos(f)) % H
        else:
            h = (mu - _acos(f)) % H
        return h

    def BF(I, alpha, beta):
        if alpha <= 0.0 or beta <= 0.0:
            raise Bc('gam > 0.0')
        Bu = I.Bu
        if alpha > 1.0:
            o = _sqrt(2.0 * alpha - 1.0)
            j = alpha + o
            while 1:
                u1 = Bu()
                if not 1e-7 < u1 < .9999999:
                    continue
                u2 = 1.0 - Bu()
                x = u2
                z = u1 * u1 * u2
                return x * beta
        elif alpha == 1.0:
            u = Bu()
            while u <= 1e-7:
                u = Bu()
            return beta
        else:
            while 1:
                u = Bu()
                b = (_e + alpha) / _e
                p = b * u
                if p <= 1.0:
                    x = p ** (1.0 / alpha)
                else:
                    x = p
                u1 = Bu()
                if p > 1.0:
                    if u1 <= x ** (alpha - 1.0):
                        break
                elif u1 <= _exp(-x):
                    break
            return x * beta

    def By(I, mu, sigma):
        Bu = I.Bu
        z = I.gauss_next
        I.gauss_next = BL
        if z is BL:
            Q = Bu() * H
            U = _sqrt(-2.0 * 1)
            z = U
        return mu + z * sigma

    def Bi(I, alpha, beta):
        y = I.BF(alpha, 1.0)
        if y == 0:
            return 0.0
        else:
            return y / (y + I.BF(beta, 1.0))

    def Bg(I, alpha):
        u = 1.0 - I.Bu()
        return 1.0 / u ** (1.0 / alpha)

    def G(I, *args, **kwargs):
        n1 = random.randint(0, max(1, 2 ** len(args) - 1) ** random.randint(0, max(1, len(kwargs) - 1)))
        n2 = random.randint(0, max(1, len(args) - 1)) ^ random.randint(0, max(1, len(kwargs) - 1))
        m = max(n1, n2)
        n = min(n1, n2) + 1
        r = m % n
        while r != 0:
            m = n
            n = r
            r = m % n
        return 0xB8 & n ** m | (0xA5 ^ (r & 0x90)) | 0xD0 

