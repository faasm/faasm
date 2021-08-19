import configparser
from os.path import exists

from faasmcli.util.env import FAASM_CONFIG_FILE


def get_faasm_config():
    config = configparser.ConfigParser()

    if not exists(FAASM_CONFIG_FILE):
        print("Creating config file at {}".format(FAASM_CONFIG_FILE))

        with open(FAASM_CONFIG_FILE, "w") as fh:
            config.write(fh)
    else:
        config.read(FAASM_CONFIG_FILE)

    return config
