project(KLife)

find_package(KDE4 REQUIRED)
find_package(GMP REQUIRED)

include(KDE4Defaults)
add_definitions(${QT_DEFINITIONS} ${KDE4_DEFINITIONS})

include_directories(${QT_INCLUDE} ${KDE4_INCLUDES} ${GMP_INCLUDE_DIR} ${CMAKE_CURRENT_BINARY_DIR})

set(KLife_SRCS main.cpp AbstractAlgorithm.cpp AlgorithmManager.cpp BigInteger.cpp CanvasPainter.cpp Editor.cpp HashLife.cpp MainWindow.cpp MemoryManager.cpp NaiveLife.cpp Utils.cpp)

kde4_add_executable(KLife ${KLife_SRCS})

target_link_libraries(KLife ${KDE4_KDEUI_LIBS} ${KDE4_KDEUTILS} ${GMP_LIBRARIES})
install(TARGETS KLife DESTINATION ${BIN_INSTALL_DIR})
install(FILES KLifeui.rc DESTINATION ${DATA_INSTALL_DIR}/KLife)
