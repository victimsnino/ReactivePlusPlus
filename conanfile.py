from conan import ConanFile

class Config(ConanFile):
    settings = "os", "compiler", "build_type", "arch"
    generators = "CMakeDeps", "CMakeToolchain"

    def requirements(self):
        self.requires("sfml/2.6.1")
        # self.requires("grpc/1.54.3", transitive_libs=True, transitive_headers=True)
        # self.requires("protobuf/3.21.12")
        # self.requires("qt/6.4.2")
        # self.requires("libmount/2.39", override=True)
        # self.requires("libpng/1.6.42", override=True)
