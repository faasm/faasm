from tasks.aws import (
    codegen_wasm_lambda_func,
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
    upload_wasm_lambda_func
)
from tasks.bench_capacity import (
    max_threads,
)
from tasks.bench_mem import (
    bench_mem,
    plot_pid_mem,
    plot_proc_mem,
    pid_mem,
    proc_mem,
    spawn_containers,
    kill_containers,
    container_count,
    kill_faasm,
    faasm_count,
)
from tasks.bench_time import (
    bench_time,
)
from tasks.bench_tpt import (
    bench_tpt,
)
from tasks.compile import (
    compile,
)
from tasks.compile_libs import (
    compile_malloc,
    compile_libfaasm,
    compile_eigen,
    compile_libfake,
    compile_libpng,
    compile_zlib,
    install_native_tools,
)
from tasks.compile_tensorflow import (
    compile_tf_lite,
    compile_tf_lite_native,
)
from tasks.config import (
    create_faasm_config
)
from tasks.docker import (
    docker_build_base,
    docker_build_data,
    docker_build_edge,
    docker_build_ibm,
    docker_build_knative,
    docker_build_noop,
    docker_build_root,
    docker_build_redis,
    docker_build_testing,
    docker_build_upload,
    docker_build_worker,
    docker_build_toolchain,
    data,
    purge_images,
    docker_push_data,
    docker_push_edge,
    docker_push_ibm,
    docker_push_knative,
    docker_push_noop,
    docker_push_root,
    docker_push_redis,
    docker_push_testing,
    docker_push_upload,
    docker_push_worker,
    docker_push_toolchain,
    pull,
    docker_push_base,
)
from tasks.docker_compose import (
    docker_clear_queue,
    docker_start_all,
    docker_stop_all,
    docker_restart_all
)
from tasks.ibm import (
    ibm_deploy_worker,
    ibm_invoke_worker,
    ibm_create_buckets,
    ibm_list,
    ibm_login,
    ibm_set_up_cos,
)
from tasks.invoke import (
    invoke,
)
from tasks.python import (
    clear_runtime_pyc,
    set_up_python_runtime,
    run_python_codegen,
)
from tasks.runtime import (
    backup_runtime_root,
    download_runtime_root,
)
from tasks.sgd import (
    begin_aws_svm,
    deploy_sgd_wasm_lambda,
    deploy_sgd_native_lambda,
    delete_sgd_native_lambda,
    upload_sgd_funcs,
    zero_sgd_native_lambda
)
from tasks.sgd_data import (
    reuters_upload_s3,
    reuters_download_s3,
    reuters_state_upload,
    reuters_state_upload_s3,
)
from tasks.tensorflow import (
    set_up_tensorflow_data
)
from tasks.toolchain import (
    backup_toolchain,
    download_toolchain,
    run_local_codegen,
)
from tasks.upload import (
    upload_all,
    upload_all_s3,
    upload,
)
from tasks.wasm2wast import wast
