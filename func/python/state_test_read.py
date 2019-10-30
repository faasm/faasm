from pyfaasm.core import getInput, setOutput

i = getInput()

print("Got input {}".format(i))

setOutput(bytes(i))
