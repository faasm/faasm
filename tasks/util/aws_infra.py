import boto3

from tasks.util.env import AWS_REGION, AWS_ACCOUNT_ID


def get_elasticache_url(cluster_name):
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


def get_security_group_ids():
    ec2 = boto3.client("ec2", region_name=AWS_REGION)

    vpc_id = get_vpc_id()

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


def get_vpc_id():
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


def get_private_subnet_ids():
    vpc_id = get_vpc_id()

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


def get_sqs_info():
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
