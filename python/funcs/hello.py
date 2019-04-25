import sys
from decimal import Decimal

print("Running Python version {}".format(sys.version))

a1 = 2
a2 = 3
print("2 + 3 = ", a1 + a2)

list_a = list(range(10))
list_a[1] = 9
list_a[3] = 33
list_a[-3:] = [40, 41, 42]

print("List: {}".format(list_a))

d = {
    "foo": "bar",
    "baz": 23
}

d["blah"] = Decimal(10)
d["wah"] = 1.2345

print("Dict: {}".format(d))

for k, v in d.items():
    print("{}: {} ({})".format(k, v, d[k]))
