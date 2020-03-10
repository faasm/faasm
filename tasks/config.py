from invoke import task

from tasks.util.aws_infra import get_sqs_info, get_security_group_ids, get_private_subnet_ids, get_elasticache_url
from tasks.util.config import get_faasm_config
from tasks.util.env import FAASM_CONFIG_FILE


@task
def create(ctx):
    """
    Set up skeleton Faasm config
    """
    get_faasm_config()


# @task
def aws(ctx):
    config = get_faasm_config()

    config["AWS"]["redis_state"] = get_elasticache_url("faasm-redis-state")
    config["AWS"]["redis_queue"] = get_elasticache_url("faasm-redis-queue")

    subnet_ids = get_private_subnet_ids()
    config["AWS"]["subnet_ids"] = ",".join(subnet_ids)

    security_group_ids = get_security_group_ids()
    config["AWS"]["security_group_ids"] = ",".join(security_group_ids)

    queue_url, queue_arn = get_sqs_info()
    config["AWS"]["sqs_url"] = queue_url
    config["AWS"]["sqs_arn"] = queue_arn

    with open(FAASM_CONFIG_FILE, "w") as fh:
        config.write(fh)
