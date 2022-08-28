# ---- Variables ----

if(NOT DEFINED RPP_GCOV_TOOL)
    set(RPP_GCOV_TOOL "")
endif()

# We use variables separate from what CTest uses, because those have
# customization issues
set(
    COVERAGE_TRACE_COMMAND
    lcov -c -q
    -o "${PROJECT_BINARY_DIR}/coverage.info"
    -d "${PROJECT_BINARY_DIR}"
    --include "${PROJECT_SOURCE_DIR}/*"
    --remove "coverage.info '/usr/*' "${HOME}"'/.cache/*' '*/tests/*' '*/submodules/*'"
    --gcov-tool "${RPP_GCOV_TOOL}"
    CACHE STRING
    "; separated command to generate a trace for the 'coverage' target"
)

set(
    COVERAGE_HTML_COMMAND
    genhtml --legend -f -q
    "${PROJECT_BINARY_DIR}/coverage.info"
    -p "${PROJECT_SOURCE_DIR}"
    -o "${PROJECT_BINARY_DIR}/coverage_html"
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
