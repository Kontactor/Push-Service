TEMPLATE = app

CONFIG += c++20

QT += gui widgets

SOURCES +=  src/main.cpp \
			src/filters_dialog.cpp \
			src/mainwindow.cpp

HEADERS +=  src/client_gui_event_handler.h \
			src/filters_dialog.h \
			src/mainwindow.h \

FORMS +=    ui/mainwindow.ui \
			ui/filters_dialog.ui

RESOURCES += resources/connected.png \
             resources/disconnected.png \

INCLUDEPATH += $$PWD/../client_lib/src \
                $$PWD/../common/push/ \
		$$PWD/../common/ \
                $$PWD/../network/client/include/ \
		$$PWD/../network/protocol/ \
		        c:/dev/boost/boost_1_89_0

LIBS += -L$$PWD/../build/client_lib/Release client_lib.lib \
        -L$$PWD/../build/network/protocol/Release protocol_lib.lib \
        -L$$PWD/../build/network/client/Release network_client_lib.lib
