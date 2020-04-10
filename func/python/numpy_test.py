import numpy as np


def main_func():
    np.__config__.show()
    # print("Starting numpy version {}".format(np.__version__))

    # a = np.array([0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10])
    # print("10={}".format(a[10]))
    # print("6={}".format(a[6]))
    #
    # a1 = a[1]
    # a2 = a[2]
    # a12 = a[1:3]
    #
    # # This prints zeroes for both...
    # print("1=" + str(a1) + ", 2=" + str(a2))
    # print("1,2={}".format(a12))
    #
    # # This prints the array properly
    # arrayOut = ""
    # a0 = None
    # for i, x in enumerate(a):
    #     if i == 0:
    #         a0 = x
    #     elif x == a0:
    #         print("x == a0")
    #
    #     if i == 1:
    #         if x == a1:
    #             print("a1 equal")
    #         else:
    #             print("a1 not equal")
    #
    #     arrayOut += str(x) + ", "
    #
    # print("Array = {}".format(arrayOut))
    #
    # # print("1={}, 2={}".format(a1, a2.item()))
    #
    # def _print_array(a):
    #     print("\nArray: {} (type={}, shape={}, strides={}). Sum: {}".format(a, a.dtype, a.shape, a.strides, np.sum(a)))
    #
    #     ix = 3
    #     got_item = a[ix]
    #     sum_item = 0.0 + got_item.item()
    #     print("got_item = {}".format(got_item))
    #     print("sum_item = {}".format(sum_item))
    #     print("got_item.item = {}".format(got_item.item()))
    #
    #     print("Array repr: {}\n".format(a.__repr__()))
    #
    #     for i, x in enumerate(a):
    #         print("a[{}] = {} ({})".format(i, x, a[i]))
    #
    # print("Loaded numpy version {}".format(np.__version__))
    #
    # # Float array
    # a = np.arange(10.0)
    # y = 7.2
    # a[1] = y
    # a[2] = 9.0
    # a.__setitem__(3, 10.0)
    # a[5:8] = [41, 42, 43]
    # a.itemset(4, 10.0)
    #
    # _print_array(a)
    #
    # # Int array
    # ia = np.random.randint(0, 100, size=10)
    # _print_array(ia)
    #
    # # Matrix
    # x1 = np.arange(9.0).reshape((3, 3))
    # print("\nMatrix (shape = {}): \n{}".format(x1.shape, x1))
    # print("Data: {}".format(x1.data))
    #
    # print("\nMatrix as list: \n{}".format(x1.tolist()))
    #
    # x2 = np.arange(3.0)
    # res = np.multiply(x1, x2)
    #
    # print("\nMultiplied: (sum = {}) \n{}".format(np.sum(res), res))


if __name__ == "__main__":
    main_func()
