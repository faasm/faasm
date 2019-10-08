import configparser
from os import mkdir
from os.path import exists

from tasks.util.env import FAASM_CONFIG_FILE, FAASM_HOME


def get_faasm_config():
    if not exists(FAASM_HOME):
        mkdir(FAASM_HOME)

    config = configparser.ConfigParser()

    if not exists(FAASM_CONFIG_FILE):
        print("Creating config file at {}".format(FAASM_CONFIG_FILE))

        config.add_section("AWS")
        config.add_section("IBM")

        with open(FAASM_CONFIG_FILE, "w") as fh:
            config.write(fh)
    else:
        config.read(FAASM_CONFIG_FILE)

    return config
