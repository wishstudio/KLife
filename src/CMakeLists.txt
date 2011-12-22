project(KLife)

find_package(KDE4 REQUIRED)
find_package(GMP REQUIRED)

macro_log_feature(GMP_FOUND "GMP" "The GNU Multiple Precision Arithmetic Library" "http://gmplib.org/" ${GMP_REQUIRED} "" "Required for building KCalc.")

include(CheckTypeSize)

check_type_size("signed long" SIZEOF_SIGNED_LONG)
check_type_size("unsigned long" SIZEOF_UNSIGNED_LONG)

configure_file(Config.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/Config.h)

include(KDE4Defaults)
add_definitions(${QT_DEFINITIONS} ${KDE4_DEFINITIONS})

include_directories(${QT_INCLUDE} ${KDE4_INCLUDES} ${GMP_INCLUDE_DIR} ${CMAKE_CURRENT_BINARY_DIR})

set(KLife_SRCS main.cpp AbstractAlgorithm.cpp AbstractFileFormat.cpp AlgorithmManager.cpp BigInteger.cpp CanvasPainter.cpp Editor.cpp FileFormatManager.cpp HashLife.cpp MainWindow.cpp MemoryManager.cpp NaiveLife.cpp RLEFormat.cpp TextStream.cpp Utils.cpp)

kde4_add_executable(KLife ${KLife_SRCS})

target_link_libraries(KLife ${KDE4_KDEUI_LIBS} ${KDE4_KDEUTILS} ${GMP_LIBRARIES})
install(TARGETS KLife DESTINATION ${BIN_INSTALL_DIR})
install(FILES KLifeui.rc DESTINATION ${DATA_INSTALL_DIR}/KLife)
