project(KLife)

include(CheckTypeSize)

find_package(Qt4 COMPONENTS QtCore QtGui REQUIRED)

check_type_size("signed long" SIZEOF_SIGNED_LONG)
check_type_size("unsigned long" SIZEOF_UNSIGNED_LONG)

configure_file(Config.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/Config.h)

add_definitions(${QT_DEFINITIONS})

include(${QT_USE_FILE})

include_directories(${QT_INCLUDE} ${CMAKE_CURRENT_BINARY_DIR})

set(KLife_SRCS main.cpp AbstractAlgorithm.cpp AbstractFileFormat.cpp AlgorithmManager.cpp BigInteger.cpp CanvasPainter.cpp DataChannel.cpp Editor.cpp FileFormatManager.cpp HashLife.cpp MainWindow.cpp MemoryManager.cpp RLEFormat.cpp Rule.cpp RuleLife.cpp TextStream.cpp TreeLife.cpp TreeUtils.cpp Utils.cpp)

add_executable(KLife ${KLife_SRCS})

target_link_libraries(KLife ${QT_LIBRARIES})
