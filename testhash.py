import uuid

def test():
    x = 64 * 1024
    y = {}
    for i in xrange(0,x):
        uid = uuid.uuid4()
        hash = uid.int % (64 * 1024)
        if y.has_key(hash):
            y[hash] += 1
        else:
            y[hash] = 1

    rate = {}

    for k,v in y.iteritems():
        if rate.has_key(v):
            rate[v] += 1
        else:
            rate[v] = 1

    sum = 0
    max = 0
    for k,v in rate.iteritems():
        sum += v
        if k>max:
            max = k
    print "sum:%d"%sum
    print 'max:%d'%max
    if max > 10:
        return False
    else:
        return True

while True:
    if not test():
        print "test fail."
        break



