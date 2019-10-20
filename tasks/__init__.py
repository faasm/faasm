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
    compile_libc,
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
    docker_build,
    docker_push,
    data,
    purge_images,
)
from tasks.docker_compose import (
    docker_start_all,
    docker_stop_all,
    docker_restart_all
)
from tasks.ibm import (
    ibm_codegen,
    ibm_delete_worker,
    ibm_deploy_worker,
    ibm_invoke,
    ibm_k8s_add_knative,
    ibm_k8s_config,
    ibm_create_buckets,
    ibm_print_kubeconf,
    ibm_list,
    ibm_login,
    ibm_set_up_cos,
    ibm_set_up_redis,
    ibm_ssh_redis,
)
from tasks.invoke import (
    invoke,
    status,
)
from tasks.knative import (
    deploy_knative,
    redeploy_knative,
    delete_knative_worker,
    build_knative_native,
    knative_native_local,
    deploy_knative_native,
    delete_knative_native,
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
    backup_sysroot,
    download_toolchain,
    run_local_codegen,
)
from tasks.upload import (
    upload_all,
    upload_all_s3,
    upload,
)
from tasks.wasm2wast import (
    wast
)
from tasks.redis import (
    redis_clear_queue,
    redis_all_workers,
    redis_func_workers,
)
