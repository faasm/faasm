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
    print_docker_mem,
)
from tasks.bench_time import (
    bench_time,
)
from tasks.bench_tpt import (
    bench_tpt,
)
from tasks.compile import (
    compile,
    compile_user,
)
from tasks.compile_libs import (
    compile_libc,
    compile_malloc,
    compile_libfaasm,
    compile_libfaasmp,
    compile_libfaasmpi,
    compile_eigen,
    compile_libfake,
    compile_libpng,
    compile_zlib,
    install_native_tools,
    compile_tflite,
)
from tasks.config import (
    create_faasm_config
)
from tasks.docker import (
    docker_build,
    docker_build_release,
    docker_push,
    docker_pull,
    docker_pull_release,
    docker_push_release,
    purge_images,
)
from tasks.docker_compose import (
    docker_start_all,
    docker_stop_all,
    docker_restart_all
)
from tasks.github import (
    gh_print_repos,
    gh_create_release,
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
    flush,
)
from tasks.knative import (
    deploy_knative,
    delete_knative_full,
    delete_knative_worker,
    build_knative_native,
    knative_native_local,
    knative_native_python_local,
    deploy_knative_native,
    deploy_knative_native_python,
    delete_knative_native,
    delete_knative_native_python,
)
from tasks.python import (
    clear_runtime_pyc,
    set_up_python_runtime,
    set_up_python_package,
    run_python_codegen,
)
from tasks.aws_sgd import (
    begin_aws_svm,
    deploy_sgd_wasm_lambda,
    deploy_sgd_native_lambda,
    delete_sgd_native_lambda,
    upload_sgd_funcs,
    zero_sgd_native_lambda
)
from tasks.experiment_data import (
    reuters_upload_s3,
    matrix_upload_s3,
    reuters_download_s3,
    matrix_download_s3,
    reuters_state_upload,
    reuters_state_upload_s3,
    matrix_state_upload,
    tf_state_upload,
    tf_upload_data,
    genomics_upload_s3,
    genomics_download_s3,
)
from tasks.toolchain import (
    backup_toolchain,
    backup_sysroot,
    download_toolchain,
    download_sysroot,
    run_local_codegen,
    backup_runtime_root,
    download_runtime_root,
    copy_release_bundles,
)
from tasks.upload import (
    upload_all,
    upload_all_s3,
    upload,
    upload_genomics,
)
from tasks.wasm2wast import (
    wast,
    wast_file,
)
from tasks.redis import (
    redis_clear_queue,
    redis_all_workers,
    redis_func_workers,
)
from tasks.experiments import (
    sgd_experiment,
    sgd_pull_results,
    matrix_experiment,
    matrix_experiment_multi,
    matrix_pull_results,
    tf_lat_experiment,
    tf_tpt_experiment,
    tf_tpt_pull_results,
    tf_tpt_parse_results,
    tf_lat_pull_results,
    tf_plot_billing,
)
from tasks.genomics_data import (
    download_genome,
    download_reads,
    index_genome,
    genomics_upload_data,
)
from tasks.genomics import (
    genomics_mapping,
)
# Can only generate matrix data with things installed
try:
    import pyfaasm
    from tasks.matrix_data import (
        generate_all_matrix_data,
        generate_matrix_data,
    )
except:
    pass
