# ---- Variables ----

if (RPP_USE_LLVM_COV)
    if (NOT RPP_BUILD_TESTS_TOGETHER)
        message( FATAL_ERROR "Expected to set RPP_BUILD_TESTS_TOGETHER flag when build coverage via llvm cov")
    endif()

    set(RPP_COVERAGE_TARGETS -instr-profile=${RPP_TEST_RESULTS_DIR}/results.profdata --object $<TARGET_FILE:test_rpp> --object $<TARGET_FILE:test_rppqt> --object $<TARGET_FILE:test_rppgrpc>)

    add_custom_target(
        coverage
        COMMAND llvm-profdata merge -sparse=true ${RPP_TEST_RESULTS_DIR}/test_rpp.profraw ${RPP_TEST_RESULTS_DIR}/test_rppqt.profraw ${RPP_TEST_RESULTS_DIR}/test_rppgrpc.profraw -o ${RPP_TEST_RESULTS_DIR}/results.profdata

        COMMAND llvm-cov report --ignore-filename-regex=build|tests ${RPP_COVERAGE_TARGETS}
        COMMAND llvm-cov show --ignore-filename-regex=build|tests --show-branches=count --show-expansions --show-line-counts --show-line-counts-or-regions --show-regions ${RPP_COVERAGE_TARGETS} > ${RPP_TEST_RESULTS_DIR}/coverage.txt
        COMMENT "Generating coverage report"
        VERBATIM
    )

elseif (RPP_COVERAGE_LCOV)
    # We use variables separate from what CTest uses, because those have
    # customization issues
    set(
        COVERAGE_TRACE_COMMAND
        lcov -c -q
        -o "${PROJECT_BINARY_DIR}/coverage.info"
        -d "${PROJECT_BINARY_DIR}"
        --include "${PROJECT_SOURCE_DIR}/*"
        --exclude "*/tests/*"
        --gcov-tool "${RPP_GCOV_TOOL}"
        --rc lcov_branch_coverage=1
        CACHE STRING
        "; separated command to generate a trace for the 'coverage' target"
    )

    set(
        COVERAGE_HTML_COMMAND
        genhtml --legend -f -q
        "${PROJECT_BINARY_DIR}/coverage.info"
        -p "${PROJECT_SOURCE_DIR}"
        -o "${PROJECT_BINARY_DIR}/coverage_html"
        --rc lcov_branch_coverage=1
        CACHE STRING
        "; separated command to generate an HTML report for the 'coverage' target"
    )

    # ---- Coverage target ----

    add_custom_target(
        coverage
        COMMAND ${COVERAGE_TRACE_COMMAND}
        COMMAND ${COVERAGE_HTML_COMMAND}
        COMMENT "Generating coverage report"
        VERBATIM
    )
else()
    SET(COVERAGE_TRACE_COMMAND
        gcovr
        -r ${PROJECT_SOURCE_DIR}
        --object-directory=${PROJECT_BINARY_DIR}
        -f ${PROJECT_SOURCE_DIR}/src/rpp
        -b
        -s
        --exclude-unreachable-branches
        --exclude-throw-branches
        --html-details ${PROJECT_BINARY_DIR}/coverage_report.html
        --xml ${PROJECT_BINARY_DIR}/coverage_report.xml
        --sonarqube ${PROJECT_BINARY_DIR}/coverage_sonarqube_report.xml
    )

if (DEFINED RPP_GCOV_TOOL)
    SET(COVERAGE_TRACE_COMMAND ${COVERAGE_TRACE_COMMAND} --gcov-executable ${RPP_GCOV_TOOL})
endif()

add_custom_target(
    coverage
    COMMAND
        ${COVERAGE_TRACE_COMMAND}
    COMMENT "Generating coverage report"
    VERBATIM
)
endif()
