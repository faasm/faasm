import configparser
from os import mkdir
from os.path import join, exists

import boto3

from tasks.env import FAASM_HOME, AWS_REGION, AWS_ACCOUNT_ID

CONFIG_FILE = join(FAASM_HOME, "faasm.ini")


def get_faasm_config():
    if not exists(CONFIG_FILE):
        _generate_faasm_config()

    config = configparser.ConfigParser()
    config.read(CONFIG_FILE)

    return config


def _generate_faasm_config():
    print("Generating faasm config file")

    if not exists(FAASM_HOME):
        mkdir(FAASM_HOME)

    config = configparser.ConfigParser()

    config.add_section("AWS")

    config["AWS"]["redis_state"] = _get_elasticache_url("faasm-redis-state")
    config["AWS"]["redis_queue"] = _get_elasticache_url("faasm-redis-queue")

    subnet_ids = _get_private_subnet_ids()
    config["AWS"]["subnet_ids"] = ",".join(subnet_ids)

    security_group_ids = _get_security_group_ids()
    config["AWS"]["security_group_ids"] = ",".join(security_group_ids)

    queue_url, queue_arn = _get_sqs_info()
    config["AWS"]["sqs_url"] = queue_url
    config["AWS"]["sqs_arn"] = queue_arn

    with open(CONFIG_FILE, "w") as fh:
        config.write(fh)


def _get_elasticache_url(cluster_name):
    client = boto3.client("elasticache", region_name=AWS_REGION)

    response = client.describe_cache_clusters(
        CacheClusterId=cluster_name,
        ShowCacheNodeInfo=True,
    )

    cluster_data = response["CacheClusters"][0]
    node_data = cluster_data["CacheNodes"][0]

    url = node_data["Endpoint"]["Address"]
    print("Found elasticache at {}".format(url))

    return url


def _get_security_group_ids():
    ec2 = boto3.client("ec2", region_name=AWS_REGION)

    vpc_id = _get_vpc_id()

    response = ec2.describe_security_groups(
        Filters=[{
            "Name": "vpc-id",
            "Values": [
                vpc_id
            ]
        }]
    )

    group_ids = [sg["GroupId"] for sg in response["SecurityGroups"]]

    print("Found security groups {}".format(group_ids))

    return group_ids


def _get_vpc_id():
    ec2 = boto3.client("ec2", region_name=AWS_REGION)

    response = ec2.describe_vpcs(
        Filters=[{
            "Name": "tag:Name",
            "Values": [
                "faasm-vpc",
            ]
        }]
    )

    vpc_data = response["Vpcs"]
    assert len(vpc_data) == 1, "Found {} default VPCs, expected 1".format(len(vpc_data))

    vpc_id = vpc_data[0]["VpcId"]

    print("Found faasm VPC {}".format(vpc_id))

    return vpc_id


def _get_private_subnet_ids():
    vpc_id = _get_vpc_id()

    ec2 = boto3.client("ec2", region_name=AWS_REGION)

    response = ec2.describe_subnets(
        Filters=[{
            "Name": "vpc-id",
            "Values": [
                vpc_id
            ]
        }, {
            "Name": "tag:Name",
            "Values": [
                "faasm-private"
            ]
        }]
    )

    subnet_data = response["Subnets"]
    ids = [sn["SubnetId"] for sn in subnet_data]

    print("Found subnet IDs {}".format(ids))

    return ids


def _get_sqs_info():
    sqs = boto3.client("sqs", region_name=AWS_REGION)

    response = sqs.get_queue_url(
        QueueName="faasm-messages",
        QueueOwnerAWSAccountId=AWS_ACCOUNT_ID
    )

    url = response["QueueUrl"]
    print("Found SQS queue at {}".format(url))

    response = sqs.get_queue_attributes(
        QueueUrl=url,
        AttributeNames=["QueueArn"],
    )

    arn = response["Attributes"]["QueueArn"]
    print("Found SQS queue arn {}".format(arn))

    return url, arn
