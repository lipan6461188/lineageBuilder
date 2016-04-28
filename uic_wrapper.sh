#!/bin/sh
DYLD_FRAMEWORK_PATH=/Users/lipan/Downloads/qt-everywhere-opensource-src-5.6.0/qtbase/lib${DYLD_FRAMEWORK_PATH:+:$DYLD_FRAMEWORK_PATH}
export DYLD_FRAMEWORK_PATH
QT_PLUGIN_PATH=/Users/lipan/Qt5.6.0/qtbase/plugins${QT_PLUGIN_PATH:+:$QT_PLUGIN_PATH}
export QT_PLUGIN_PATH
exec /Users/lipan/Downloads/qt-everywhere-opensource-src-5.6.0/qtbase/bin/uic "$@"
