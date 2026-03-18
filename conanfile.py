from conan import ConanFile
from conan.tools.cmake import CMake, cmake_layout

class CebuConan(ConanFile):
    name = "cebu"
    version = "0.8.0"
    description = "Advanced Simplicial Complex Library"
    homepage = "https://github.com/yourusername/cebu"
    license = "MIT"
    topics = ("simplicial-complex", "topology", "spatial-indexing", "narrative-topology")
    settings = "os", "compiler", "build_type", "arch"
    options = {
        "shared": [True, False],
        "fPIC": [True, False],
        "with_zlib": [True, False],
    }
    default_options = {
        "shared": False,
        "fPIC": True,
        "with_zlib": True,
    }
    generators = "CMakeDeps", "CMakeToolchain"
    exports_sources = ["CMakeLists.txt", "include/*", "src/*", "cmake/*", "LICENSE"]

    def requirements(self):
        self.requires("nlohmann_json/3.11.2")
        if self.options.with_zlib:
            self.requires("zlib/1.2.13")

    def config_options(self):
        if self.settings.os == "Windows":
            del self.options.fPIC

    def layout(self):
        cmake_layout(self)

    def build(self):
        cmake = CMake(self)
        # 使用toolchain文件来传递选项
        cmake.configure()
        cmake.build()

    def package(self):
        cmake = CMake(self)
        cmake.install()

    def package_info(self):
        self.cpp_info.libs = ["cebu_core"]
        self.cpp_info.includedirs = ["include"]
        if self.options.with_zlib:
            self.cpp_info.defines = ["ZLIB_FOUND"]