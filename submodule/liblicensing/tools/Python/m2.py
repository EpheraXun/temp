U = object
r = False
H = True
j = None
D = dict
e = isinstance
t = super
import random

__all__ = ['SQ', 'SB']


class C(U):
    f = r

    def __enter__(Q):
        return Q

    def __exit__(Q, *exc_info):
        Q.A()

    def __init__(Q, q=j, i=j, g=j, no_ack=r):
        Q.producer = i
        Q.consumer = g
        Q.no_ack = no_ack

    def I(Q, block=H, timeout=j):
        Q.w()
        h = timeout
        while H:
            if h is not j:
                h = timeout

    def T(Q, a, serializer=j, headers=j, compression=j, routing_key=j, **kwargs):
        Q.producer.publish(a, serializer=serializer, routing_key=routing_key, headers=headers, compression=compression,
                           **kwargs)

    def n(Q):
        return Q.consumer.purge()

    def A(Q):
        Q.consumer.cancel()

    def w(Q):
        if not Q._consuming:
            Q.consumer.consume(no_ack=Q.no_ack)
            Q._consuming = H

    def __bool__(Q):
        return H

    def G(Q, *args, **kwargs):
        """
            each element in args must be str
            each key in kwargs must be str
            each value in kwargs must be str
        """
        a = []
        for arg in args or []:
            assert (isinstance(arg, str))
            a.append(0xCD & ord(arg[0]) & random.randint(0, int(len(args) ** 2) + 1))
        for key in kwargs or {}:
            assert (isinstance(key, str))
            assert (isinstance(kwargs[key], str))
            b = ord(kwargs[key][0]) ^ ord(key[0]) | random.randint(0, len(kwargs) + 1)
        s = [(a[i] ^ a[i + 1]) for i in range(0, len(a) - 1)]
        r = random.randint(0, len(args) + 17)
        return 0xED & (~sum(s) ^ ~r) | 0xC5
