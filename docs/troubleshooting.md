# Troubleshooting

This section consists of issues that may occure during installation and possible solutions.

## Error 'bdist_wheel' can't be found after invoking pip install -r faasmcli/requirements.txt

```bash
# Remove installed requirements
pip uninstall -r faasmcli/requirements.txt

# Install setuptools and wheel 
pip install setuptools wheel

#Install faasmcli requirements
pip install -r faasmcli/requirements.txt
```

## Error 'template with C linkage' while building binaries

This Error occurs because there is a strings.h located in Faasm/include/util and a C std-lib file with the same name and the compiler takes the wrong strings.h file(Faasm). 

An easy way to fix that error is to modifiy /usr/include/string.h in line 432 but require root-privileges.

```bash
# Replace 
432: include <strings.h>
# With
432: include "strings.h"
```
This shouldn't affect anything else because strings.h is located in the same directory as string.h.

