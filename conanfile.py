from conan import ConanFile
from conan.tools.cmake import CMake, CMakeDeps, CMakeToolchain
from conan.tools.layout import cmake_layout

class CebuConan(ConanFile):
    name = "cebu"
    version = "0.8.0"
    description = "Advanced Simplicial Complex Library with spatial indexing, narrative-driven topology, and absurdity system"
    homepage = "https://github.com/ceaserzhao/cebu"
    license = "MIT"
    topics = ("simplicial-complex", "topology", "spatial-indexing", "narrative", "absurdity")
    settings = "os", "compiler", "build_type", "arch"
    options = {
        "shared": [True, False],
        "fPIC": [True, False],
        "with_zlib": [True, False]
    }
    default_options = {
        "shared": False,
        "fPIC": True,
        "with_zlib": False
    }
    generators = "CMakeDeps", "VirtualBuildEnv"

    def config_options(self):
        if self.settings.os == "Windows":
            del self.options.fPIC

    def layout(self):
        cmake_layout(self)

    def requirements(self):
        if self.options.with_zlib:
            self.requires("zlib/1.2.13")

    def toolchain(self):
        tc = CMakeToolchain(self)
        tc.variables["CEBU_BUILD_TESTS"] = False
        tc.variables["CEBU_BUILD_EXAMPLES"] = False
        tc.variables["CEBU_WITH_ZLIB"] = self.options.with_zlib
        tc.generate()

    def build(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.build()

    def package(self):
        cmake = CMake(self)
        cmake.install()

    def package_info(self):
        self.cpp_info.set_property("cmake_file_name", "cebu")
        self.cpp_info.set_property("cmake_target_name", "cebu::cebu_core")
        self.cpp_info.libs = ["cebu_core"]
        if self.options.with_zlib:
            self.cpp_info.requires = ["zlib::zlib"]

    def export_sources(self):
        self.copy("*", src="", dst=".")
