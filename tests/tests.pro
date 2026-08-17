QT += core gui network
# C++17 est le minimum exigé par Qt 6, et c'est aussi le défaut de qmake sous Qt 6 : la ligne
# pourrait disparaître, on la garde explicite parce qu'elle documente ce que le code exige.
CONFIG += console c++17
CONFIG += strict_c++
CONFIG -= app_bundle

TARGET = respawnIrcTests
TEMPLATE = app

# Comme pour le programme, l'exécutable est produit à un endroit fixe quelle que soit la façon de
# compiler, et c'est le même : build/. Les tests n'ont besoin ni de resources/ ni de themes/, ils
# lisent leurs fixtures par le chemin absolu ci-dessous.
DESTDIR = $$PWD/../build

# Même barrière que respawnIrc.pro, et pour la même raison : voir le commentaire là-bas.
DEFINES += QT_DISABLE_DEPRECATED_UP_TO=0x060B00
DEFINES += FIXTURES_PATH=\\\"$$PWD/fixtures\\\"

include(../zlib.pri)

INCLUDEPATH += $$PWD/../respawnIrc

SOURCES += \
    main.cpp \
    testParsing.cpp \
    testQt6Behaviour.cpp \
    ../respawnIrc/parsingTool.cpp \
    ../respawnIrc/payloadTool.cpp \
    ../respawnIrc/pathTool.cpp \
    ../respawnIrc/logTool.cpp \
    ../respawnIrc/styleTool.cpp \
    ../respawnIrc/shortcutTool.cpp

HEADERS += \
    testTool.hpp
