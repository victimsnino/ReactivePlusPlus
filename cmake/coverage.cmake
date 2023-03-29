# ---- Variables ----

if (RPP_COVERAGE_LCOV)
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
        -f ${PROJECT_SOURCE_DIR}/src
        -e ${PROJECT_SOURCE_DIR}/src/tests
        -b
        -s
        --exclude-unreachable-branches
        --exclude-throw-branches
        --html-details ${PROJECT_BINARY_DIR}/coverage_report.html
        --xml ${PROJECT_BINARY_DIR}/coverage_report.xml
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
