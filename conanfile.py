from conan import ConanFile
from conan.tools.cmake import CMakeToolchain, CMake, cmake_layout


class OvaLibConan(ConanFile):
    name = "ova-lib"
    version = "0.0.1"
    description = "Lightweight data structures and algorithms in C"
    license = "MIT"
    url = "https://github.com/osvaldoandrade/ova-lib"
    homepage = "https://github.com/osvaldoandrade/ova-lib"
    topics = ("data-structures", "algorithms", "c")
    settings = "os", "compiler", "build_type", "arch"
    options = {"shared": [True, False], "fPIC": [True, False]}
    default_options = {"shared": False, "fPIC": True}
    exports_sources = "CMakeLists.txt", "src/*", "include/*"

    def config_options(self):
        if self.settings.os == "Windows":
            del self.options.fPIC

    def layout(self):
        cmake_layout(self)

    def generate(self):
        tc = CMakeToolchain(self)
        tc.generate()

    def build(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.build()

    def package(self):
        cmake = CMake(self)
        cmake.install()

    def package_info(self):
        self.cpp_info.libs = ["ova_lib"]
