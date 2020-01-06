from pyperformance.benchmarks.bm_hexiom import LEVELS, DESCENDING, Done, solve_file
from six.moves import StringIO

level = 25


def _do_bench():
    board, solution = LEVELS[level]
    order = DESCENDING
    strategy = Done.FIRST_STRATEGY

    board = board.strip()
    expected = solution.rstrip()

    stream = StringIO()
    solve_file(board, strategy, order, stream)

    output = stream.getvalue()

    output = '\n'.join(line.rstrip() for line in output.splitlines())
    if output != expected:
        raise AssertionError("Got a wrong answer:\n%s\nexpected: %s" % (output, expected))


if __name__ == "__main__":
    for i in range(10):
        _do_bench()
