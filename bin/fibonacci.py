import sys


# Deliberately inefficient fibonacci calculation for testing CPU hogs

def do_fib(n):
    if n == 0:
        return 0
    elif n == 1:
        return 1
    else:
        return do_fib(n - 1) + do_fib(n - 2)


if __name__ == "__main__":
    fib_n = int(sys.argv[1])

    res = do_fib(fib_n)

    print("Fibonacci {} = {}".format(fib_n, res))
