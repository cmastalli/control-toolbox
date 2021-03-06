cmake_minimum_required (VERSION 3.3)

include(${CMAKE_CURRENT_SOURCE_DIR}/../ct/cmake/compilerSettings.cmake)
include(${CMAKE_CURRENT_SOURCE_DIR}/../ct/cmake/explicitTemplateHelpers.cmake)
include(${CMAKE_CURRENT_SOURCE_DIR}/../ct/cmake/clang-cxx-dev-tools.cmake)


project(ct_core VERSION 3.0.1 LANGUAGES CXX)


set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)
set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -pthread -std=c++14 -Wall -Wfatal-errors")
set(CMAKE_EXE_LINKER_FLAGS  "${CMAKE_EXE_LINKER_FLAGS} -pthread -std=c++14 -Wall -Wfatal-errors")
set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)


## find and include required dependencies
find_package(Eigen3 REQUIRED)
find_package(Boost COMPONENTS REQUIRED)

set(ct_core_LIBS "")

## find and include optional dependencies
find_package(Qwt QUIET)
find_package(Qt4 QUIET)
if(QWT_FOUND AND Qt4_FOUND)
    message(STATUS "Qwt and QT found.")
    set(QWT_ENABLED ON)
    include(${QT_USE_FILE})
    list(APPEND ct_core_LIBS ${QWT_LIBRARY} ${QT_QTCORE_LIBRARY} ${QT_QTGUI_LIBRARY})
else()
    message(STATUS "COMPILING WITHOUT QWT")
endif()

## find and include optional dependencies
find_package(PythonLibs 2.7 QUIET)
if (PYTHONLIBS_FOUND)
   message(STATUS "Python found")
   list(APPEND ct_plot_COMPILE_DEFINITIONS PLOTTING_ENABLED)
   message(STATUS "Python library path ... " ${PYTHON_LIBRARY})
   list(APPEND ct_core_LIBS ${PYTHON_LIBRARY})
else() 
   message(STATUS "Python not found") 
endif()


## configure files required for code-generation
set(CODEGEN_TEMPLATE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/templates")
set(CODEGEN_OUTPUT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/generated")
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/include/ct/core/templateDir.h.in ${CMAKE_CURRENT_SOURCE_DIR}/include/ct/core/templateDir.h)


###################
# BUILD LIBRARIES #
###################

## define the directories to be included in all ct_core targets
set(ct_core_target_include_dirs
    ${EIGEN3_INCLUDE_DIR} 
    ${PYTHON_INCLUDE_DIRS}
    ${QWT_INCLUDE_DIR}
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/ct/external>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/examples/include>
    $<INSTALL_INTERFACE:include>
    $<INSTALL_INTERFACE:include/ct/external>
)

## declare prespec libraries
set(PRESPEC_LIB_NAMES "")

## define list of libraries that contain prespecified templates
if(USE_PRESPEC)
    # extract the prespec parameters from user-input
    ct_configure_explicit_templates("${CMAKE_CURRENT_SOURCE_DIR}/../ct/config/explicit_templates.cfg" 
        "${CMAKE_CURRENT_SOURCE_DIR}/prespec/" 
        "ct_core"
    )
    message(STATUS "CT Core: Compiling the following explict template libraries: ${PRESPEC_LIB_NAMES}")
    # create libraries
    foreach(lib_name ${PRESPEC_LIB_NAMES})
        add_library(${lib_name} SHARED ${${lib_name}_SRCS})
        target_include_directories(${lib_name} PUBLIC ${ct_core_target_include_dirs})
    endforeach()
endif()


## create ct_core libraries
add_library(ct_plot SHARED src/core/plot/plot.cpp)
target_include_directories(ct_plot PUBLIC ${ct_core_target_include_dirs})
target_compile_definitions(ct_plot PUBLIC ${ct_plot_COMPILE_DEFINITIONS})
target_link_libraries(ct_plot ${ct_core_LIBS})

add_library(ct_core INTERFACE)
target_include_directories(ct_core INTERFACE ${ct_core_target_include_dirs})
target_link_libraries(ct_core INTERFACE
    ${ct_core_LIBS}
    ${PRESPEC_LIB_NAMES}
    ct_plot
    pthread
    dl # required for gcc compatibility
    )


##################
# BUILD EXAMPLES #
##################
if(BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()


###########
# TESTING #
###########
## requires gtest to be installed, e.g. via 
## sudo apt install libgtest-dev
## cd /usr/src/gtest/
## sudo cmake -DBUILD_SHARED_LIBS=ON
## sudo make
## sudo cp *.so /usr/lib
if(BUILD_TESTS)
    find_package(GTest REQUIRED)
    enable_testing()
    add_subdirectory(test)
endif()


#################
# INSTALLATION  #
#################

# for correct libraries locations across platforms
include(GNUInstallDirs)

## copy the header files
install(DIRECTORY include/ct/core DESTINATION include/ct)
install(DIRECTORY include/ct/external DESTINATION include/ct)
install(DIRECTORY examples/include/ct/core DESTINATION include/ct)

## copy the cmake files required for find_package()
install(FILES "cmake/ct_coreConfig.cmake" DESTINATION "share/ct_core/cmake")

## install library and targets
install(
    TARGETS ct_core ct_plot ${PRESPEC_LIB_NAMES}
    EXPORT ct_core_export
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_LIBDIR}
    )

## create the ct_core.cmake file which holds target includes and dependencies
install (EXPORT ct_core_export DESTINATION share/ct_core/cmake)


#################
# DOCUMENTATION #
#################
add_subdirectory(doc)