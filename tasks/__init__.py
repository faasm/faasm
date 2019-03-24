from tasks.aws import (
    lambda_clear_queue,
    lambda_concurrency,
    lambda_worker_count,
    list_lambdas,
    list_event_sources,
    invoke_lambda,
    invoke_faasm_lambda,
    deploy_faasm_lambda,
    delete_lambda,
    delete_faasm_lambda,
    deploy_wasm_lambda_func,
    deploy_native_lambda_func,
    sqs_length,
    purge_sqs,
    invoke_lambda_worker,
    prepare_lambda_workers,
)
from tasks.compile import (
    clean_build,
    funcs,
    funcs_emscripten,
    funcs_python,
    compile_libfaasm,
    compile_libfaasm_emscripten,
    compile_libfaasm_python,
    compile_eigen,
    compile_libfake,
)
from tasks.docker import (
    build_all,
    build_base,
    build_data,
    build_edge,
    build_redis,
    build_worker,
    build_upload,
    data,
    purge_images,
    push_all,
    push_data,
    push_edge,
    push_redis,
    push_upload,
    push_worker,
    pull,
    push_base
)
from tasks.docker_compose import start_all, stop_all, restart_all
from tasks.python import (
    build_python_host,
    build_python,
    build_pyodide_cpython,
    python_codegen,
)
from tasks.sgd import (
    begin_aws_svm,
    deploy_sgd_wasm_lambda,
    deploy_sgd_native_lambda,
    delete_sgd_native_lambda,
    upload_sgd_funcs,
    reuters_upload,
    reuters_upload_s3,
    reuters_prepare_aws,
    zero_sgd_native_lambda
)
from tasks.upload import (
    upload_funcs,
    upload_func,
)
from tasks.wasm2wast import wast
