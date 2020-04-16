include_directories(
        ${FAASM_INCLUDE_DIR}/knative
)

FIND_PACKAGE(Boost 1.65 OPTIONAL_COMPONENTS program_options)
if (Boost_FOUND)
    INCLUDE_DIRECTORIES(${Boost_INCLUDE_DIR})
endif ()

set(LIB_FILES
        ${FAASM_INCLUDE_DIR}/knative/KnativeHandler.h
        ${FAASM_INCLUDE_DIR}/knative/KnativeEndpoint.h
        KnativeHandler.cpp
        KnativeEndpoint.cpp
        )

faasm_private_lib(worker_knative_lib "${LIB_FILES}")

target_link_libraries(worker_knative_lib endpoint worker_lib)

add_executable(worker_knative worker_knative.cpp)

if (Boost_FOUND)
    target_compile_definitions(worker_knative PRIVATE WITH_BOOST=1)
    target_link_libraries(worker_knative worker_knative_lib Boost::program_options)
else ()
    target_link_libraries(worker_knative worker_knative_lib)
endif ()
