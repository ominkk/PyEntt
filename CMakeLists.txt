cmake_minimum_required(VERSION 3.16.3)

project(entt LANGUAGES CXX)

# 生成 compile_commands.json 文件，用于代码分析工具和 IDE
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# 可选的生成Python类型提示文件
option(BUILD_STUBS "Build Python type stub files" ON)

# 添加entt作为子目录
add_subdirectory(entt)

if (NOT SKBUILD)
  message(WARNING "\
  This CMake file is meant to be executed using 'scikit-build'. Running
  it directly will almost certainly not produce the desired result. If
  you are a user trying to install this package, please use the command
  below, which will install all necessary build dependencies, compile
  the package in an isolated environment, and then install it.
  =====================================================================
   $ pip install .
  =====================================================================
  If you are a software developer, and this is your own package, then
  it is usually much more efficient to install the build dependencies
  in your environment once and use the following command that avoids
  a costly creation of a new virtual environment at every compilation:
  =====================================================================
   $ pip install nanobind scikit-build-core[pyproject]
   $ pip install --no-build-isolation -ve .
  =====================================================================
  You may optionally add -Ceditable.rebuild=true to auto-rebuild when
  the package is imported. Otherwise, you need to re-run the above
  after editing C++ files.")
endif()

find_package(Python 3.10
  REQUIRED COMPONENTS Interpreter Development
  OPTIONAL_COMPONENTS Development.SABIModule)

# Import nanobind through CMake's find_package mechanism
find_package(nanobind CONFIG REQUIRED)

# We are now ready to compile the actual extension module
nanobind_add_module(
  _entt
  STABLE_ABI
  NB_STATIC
  src/registry_wrapper.cpp
)

target_link_libraries(_entt PRIVATE EnTT Python::Python)

# 在Linux平台下添加静态链接GCC和C++标准库的选项
if(UNIX AND NOT APPLE)
  target_link_options(_entt PRIVATE "-static-libgcc" "-static-libstdc++")
endif()

# 添加stub文件生成
if(BUILD_STUBS)
  nanobind_add_stub(
    _entt_stub
    MODULE _entt
    OUTPUT _entt.pyi
    PYTHON_PATH $<TARGET_FILE_DIR:_entt>
    MARKER_FILE py.typed
    DEPENDS _entt
  )
endif()

# Install directive for scikit-build-core
install(TARGETS _entt LIBRARY DESTINATION entt)
install(FILES script/__init__.py DESTINATION entt)
install(FILES script/component.py DESTINATION entt)
if(BUILD_STUBS)
  install(FILES script/__init__.pyi DESTINATION entt)
  install(FILES ${CMAKE_CURRENT_BINARY_DIR}/_entt.pyi
                ${CMAKE_CURRENT_BINARY_DIR}/py.typed
          DESTINATION entt)
endif()
