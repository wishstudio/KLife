project(KLife)

find_package(KDE4 REQUIRED)

include(KDE4Defaults)
add_definitions(${QT_DEFINITIONS} ${KDE4_DEFINITIONS})

include_directories(${QT_INCLUDE} ${KDE4_INCLUDES} ${CMAKE_CURRENT_BINARY_DIR})

set(KLife_SRCS main.cpp AbstractAlgorithm.cpp AlgorithmManager.cpp HashLife.cpp MainCanvas.cpp MainWindow.cpp NaiveLife.cpp)

kde4_add_executable(KLife ${KLife_SRCS})

target_link_libraries(KLife ${QT_QTDECLARATIVE_LIBRARY} ${KDE4_KDEUI_LIBS} ${KDE4_KUTILS_LIBS})
install(TARGETS KLife DESTINATION ${BIN_INSTALL_DIR})
install(FILES KLifeui.rc DESTINATION ${DATA_INSTALL_DIR}/KLife)
