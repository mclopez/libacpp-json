from conan import ConanFile
from conan.tools.cmake import CMake, cmake_layout
from conan.tools.files import copy

class LibacppJsonConan(ConanFile):
    name = "acpp-json"
    version = "0.1.0"
    
    # Optional metadata
    license = "MIT"
    author = "Your Name"
    url = "https://github.com/yourusername/libacpp-json"
    description = "A C++ JSON library"
    
    # Binary configuration
    settings = "os", "compiler", "build_type", "arch"
    options = {"shared": [True, False], "fPIC": [True, False]}
    default_options = {"shared": False, "fPIC": True}
    
    # Add generators
    generators = "CMakeDeps", "CMakeToolchain"
    
    # Sources are located in the same place as this recipe
    exports_sources = "CMakeLists.txt", "src/*", "include/*", "test/*"

    def requirements(self):
#        self.test_requires("gtest/1.14.0")
        self.requires("spdlog/1.12.0", transitive_headers=True)

    def build_requirements(self):
        self.test_requires("gtest/1.14.0")



    def config_options(self):
        if self.settings.os == "Windows":
            del self.options.fPIC

    def layout(self):
        cmake_layout(self)

    def build(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.build()

    def package(self):
        cmake = CMake(self)
        cmake.install()

    def package_info(self):
        self.cpp_info.libs = ["acppJson"]
        self.cpp_info.set_property("cmake_file_name", "acpp-json")
        self.cpp_info.set_property("cmake_target_name", "acpp-json::acppJson")
        self.cpp_info.includedirs = ["include"]
        self.cpp_info.requires = ["spdlog::spdlog"]
        if self.options.shared:
            self.cpp_info.defines.append("ACPPJSON_SHARED")