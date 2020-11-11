#include "faasm/faasm.h"

#include <stdio.h>
#include <unistd.h>

int main(int argc, char* argv[])
{
    int stdoutIsTty = isatty(STDOUT_FILENO);
    int stderrIsTty = isatty(STDERR_FILENO);

    if (!stdoutIsTty || !stderrIsTty) {
        printf("Stdout and stderr should be tty (got %i %i)\n",
               stdoutIsTty,
               stderrIsTty);
        return 1;
    }

    return 0;
}
