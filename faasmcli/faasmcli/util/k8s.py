from jinja2 import Environment, FileSystemLoader
from os.path import basename, dirname


def template_k8s_file(template_file_path, output_file_path, template_vars):
    # Load and render the template using jinja
    env = Environment(
        loader=FileSystemLoader(dirname(template_file_path)),
        trim_blocks=True,
        lstrip_blocks=True,
    )
    template = env.get_template(basename(template_file_path))
    output_data = template.render(template_vars)

    # Write to output file
    with open(output_file_path, "w") as fh:
        fh.write(output_data)
