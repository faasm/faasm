from tasks.aws import (
    lambda_clear_queue,
    lambda_concurrency,
    lambda_worker_count,
    list_lambdas,
    list_event_sources,
    invoke_lambda,
    invoke_faasm_lambda,
    invoke_lambda_python_codegen,
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
from tasks.codegen import (
    run_codegen,
)
from tasks.compile import (
    compile,
    compile_malloc,
    compile_libfaasm,
    compile_eigen,
    compile_libfake,
)
from tasks.invoke import (
    invoke,
    invoke_async,
    py_invoke,
    py_invoke_async,
)
from tasks.docker import (
    build_all,
    build_base,
    build_data,
    build_edge,
    build_root,
    build_redis,
    build_testing,
    build_upload,
    build_worker,
    build_toolchain,
    data,
    purge_images,
    push_all,
    push_data,
    push_edge,
    push_root,
    push_redis,
    push_testing,
    push_upload,
    push_worker,
    push_toolchain,
    pull,
    push_base
)
from tasks.docker_compose import (
    start_all,
    stop_all,
    restart_all
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
from tasks.runtime import (
    clear_runtime_pyc,
    set_up_python_runtime,
    run_python_codegen,
    package_python_runtime,
    download_python_runtime,
)

from tasks.upload import (
    upload_all,
    upload_all_s3,
    upload,
    py_upload,
)
from tasks.wasm2wast import wast

from tasks.toolchain import (
    backup_emsdk,
    restore_emsdk,
)
