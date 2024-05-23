from conan import ConanFile

class RppConan(ConanFile):
    name = "rpp"
    settings = "os", "compiler", "build_type", "arch"
    generators = "CMakeDeps", "CMakeToolchain"

    options = {
        "with_grpc" : [False, True],
        "with_sfml" : [False, True],
        "with_tests" : [False, True],
        "with_cmake" : [False, True],
        "with_benchmarks" : [False, True]
    }
    default_options = {
        "with_grpc" : False,
        "with_sfml" : False,
        "with_tests": False,
        "with_cmake": False,
        "with_benchmarks" : False
    }

    def requirements(self):
        if self.options.with_tests:
            self.requires("trompeloeil/47")
            self.requires("snitch/1.2.3")

        if self.options.with_benchmarks:
            self.requires("nanobench/4.3.11")

        if self.options.with_sfml:
            self.requires("sfml/2.6.1", options={"audio": False})

        # if self.options.with_grpc:
            # self.requires("grpc/1.54.3", transitive_libs=True, transitive_headers=True)
            # self.requires("protobuf/3.21.12")
            # self.requires("libmount/2.39", override=True)

        if self.options.with_cmake:
            self.tool_requires("cmake/3.29.3")
