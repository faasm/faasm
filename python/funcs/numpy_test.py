import numpy as np

print("Loaded numpy version {}".format(np.__version__))

a = np.arange(10.0)
y = 7.2
a[1] = y
a[2] = 9.0
a[5:8] = [41, 42, 43]
a.itemset(3, 10.0)

print("\nArray: {}. Sum: {}".format(a, np.sum(a)))

for i, x in enumerate(a):
    print("a[{}] = {}".format(i, x))

x1 = np.arange(9.0).reshape((3, 3))
print("\nMatrix (shape = {}): \n{}".format(x1.shape, x1))
print("Data: {}".format(x1.data))

print("\nMatrix as list: \n{}".format(x1.tolist()))

x2 = np.arange(3.0)
res = np.multiply(x1, x2)

print("\nMultiplied: (sum = {}) \n{}".format(np.sum(res), res))
