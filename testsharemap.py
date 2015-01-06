import pyshmsession
import uuid
import time

pyshmsession.init()

s = {}

t = time.time()

while len(s) < 60000:
    s[uuid.uuid4().hex] = uuid.uuid4().hex

print time.time()-t
t = time.time()

for k,v in s.iteritems():
    pass

print time.time()-t
t = time.time()

for k,v in s.iteritems():
    pyshmsession.add(k,v)

print time.time() - t
t = time.time()

for k in s.iterkeys():
    pyshmsession.erase(k)

print time.time() - t
t = time.time()

for k,v in s.iteritems():
    pyshmsession.add(k,v)

print pyshmsession.size()

t = time.time()
pyshmsession.recycle(60)

print pyshmsession.size()

print time.time() - t


