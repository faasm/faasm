from decimal import Decimal
from json import loads
from subprocess import run, PIPE

import networkx as nx
from networkx.drawing.nx_pydot import write_dot

# This makes heavy use of Graphviz.
# - Attributes: https://graphviz.org/doc/info/attrs.html
# - Colours:    https://graphviz.org/doc/info/colors.html

GRAPH_FONT = "monospace"

HOST_COLOURS = [
    "cadetblue1",
    "gold1",
    "darkolivegreen1",
    "bisque",
    "invis",
    "plum1",
]


def parse_exec_graph_json(json_str):
    json_obj = loads(json_str)

    root_node = json_obj["root"]
    all_hosts = _get_hosts_from_node(root_node)

    # Assign colours to hosts
    host_colour_map = dict()
    for i, h in enumerate(all_hosts):
        host_colour_map[h] = HOST_COLOURS[i % len(HOST_COLOURS)]

    graph = nx.DiGraph(labelloc="t", fontname=GRAPH_FONT)
    _add_node_to_graph(root_node, graph, host_colour_map)

    return graph


def _get_hosts_from_node(node):
    node_hosts = set()
    node_host = node["msg"].get("exec_host", "")
    node_hosts.add(node_host)

    children = node.get("chained", list())
    for c in children:
        child_hosts = _get_hosts_from_node(c)
        node_hosts = node_hosts.union(child_hosts)

    return node_hosts


def _format_node_to_html(node):
    output = ""

    start_ts = node.get("timestamp")
    finish_ts = node.get("finished")
    run_time = "-"
    if start_ts and finish_ts:
        run_time = "{}ms".format(Decimal(finish_ts) - Decimal(start_ts))

    attributes = [
        ("Call ID", node.get("id")),
        ("User", node.get("user")),
        ("Function", node.get("function")),
        ("Host", node.get("exec_host")),
        ("Hops", node.get("hops")),
        ("Time", run_time),
    ]

    for label, value in attributes:
        output += "{0: <8} {1: >14}\n".format(label, value if value else "-")

    return output


def _add_node_to_graph(node, graph, host_colour_map):
    node_msg = node["msg"]
    node_id = node_msg["id"]
    node_host = node_msg.get("exec_host")

    node_attrs = dict()
    node_attrs["style"] = "filled"
    node_attrs["fillcolor"] = host_colour_map[node_host] if node_host else HOST_COLOURS[0]
    node_attrs["label"] = _format_node_to_html(node_msg)
    node_attrs["shape"] = "box"
    node_attrs["fontname"] = GRAPH_FONT

    graph.add_node(node_id, **node_attrs)

    children = node.get("chained", list())
    for child in children:
        # Add the child node
        child_id = _add_node_to_graph(child, graph, host_colour_map)

        # Add edge between parent and child
        graph.add_edge(node_id, child_id)

    return node_id


def plot_exec_graph(graph, headless=True, output_file="/tmp/faasm_exec_graph.png"):
    dot_file = "/tmp/faasm_exec_graph.dot"

    write_dot(graph, dot_file)

    cmd = [
        "dot", "-Tpng",
        dot_file,
    ]
    png_data = run(cmd, check=True, stdout=PIPE, stderr=PIPE)

    with open(output_file, "wb") as fh:
        fh.write(png_data.stdout)

    if not headless:
        run("xdg-open {}".format(output_file), shell=True)
    else:
        print("Output at {}".format(output_file))

    return output_file
