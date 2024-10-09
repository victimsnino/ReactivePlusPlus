# ---- Variables ----

if (NOT RPP_BUILD_TESTS_TOGETHER)
    message( FATAL_ERROR "Expected to set RPP_BUILD_TESTS_TOGETHER flag when build coverage via llvm cov")
endif()

set(RPP_COVERAGE_TARGETS -instr-profile=${RPP_TEST_RESULTS_DIR}/results.profdata -object $<TARGET_FILE:test_rpp> -object $<TARGET_FILE:test_rppqt> -object $<TARGET_FILE:test_rppgrpc> -object $<TARGET_FILE:test_rppasio>)

add_custom_target(
    coverage
    COMMAND llvm-profdata merge -o ${RPP_TEST_RESULTS_DIR}/results.profdata ${RPP_TEST_RESULTS_DIR}/*.profraw
    COMMAND llvm-cov report --ignore-filename-regex=build ${RPP_COVERAGE_TARGETS}
    COMMAND llvm-cov show --show-instantiations=false --ignore-filename-regex=build --show-branches=count ${RPP_COVERAGE_TARGETS} > ${RPP_TEST_RESULTS_DIR}/coverage.txt
    COMMENT "Generating coverage report"
)
