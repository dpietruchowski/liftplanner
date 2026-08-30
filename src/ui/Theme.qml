pragma Singleton
import QtQuick

QtObject {
    id: theme

    property int applicationWidth: 360
    property int applicationHeight: 640
    property int contentMaxWidth: 600

    property bool isNightMode: true

    property var colors: QtObject {
        property color background: "#0E0F11"
        property color surface: "#141619"
        property color surfaceMuted: "#101214"
        property color surfaceSunken: "#121316"
        property color surfaceRaised: "#131518"
        property color surfaceAccent: "#1A2028"
        property color dialogSurface: "#191C20"

        property color primary: "#46A6EF"
        property color primaryVariant: "#6FBBF3"
        property color primarySurface: "#1B2D3A"
        property color primaryBorder: "#2C5D82"
        property color primaryLight: "#86C5FA"

        property color secondary: "#8B7BF0"
        property color info: "#46A6EF"
        property color success: "#53BE70"
        property color successSurface: "#37784A"
        property color warning: "#E7B84B"
        property color error: "#E5695C"

        property color border: "#202225"
        property color borderStrong: "#3A3B3E"
        property color borderSubtle: "#1C1E20"
        property color divider: "#242628"
        property color cardBackground: "#141619"
        property color cardBorder: "#202225"
        property color chipBackground: "#252729"
        property color chipText: "#9AA0A8"

        property color textPrimary: "#F2F3F5"
        property color textSecondary: "#C9CDD3"
        property color textMuted: "#8A9099"
        property color textDisabled: "#585C63"
        property color textFaint: "#4B4F55"
        property color textSoft: "#9B9D9F"
        property color textInverse: "#0E0F11"
        property color textPlaceholder: "#585C63"
        property color buttonText: "#0E0F11"

        property color overlay: "#000000B3"
        property color overlayLight: "#A6000000"
    }

    property var fontSize: QtObject {
        property int huge: 80
        property int xxLarge: 40
        property int xlarge: 28
        property int large: 21
        property int medium: 16
        property int normal: 14
        property int small: 12
        property int xSmall: 11
    }

    property var radius: QtObject {
        property int small: 6
        property int medium: 12
        property int large: 16
        property int xLarge: 22
    }

    property var padding: QtObject {
        property int xSmall: 4
        property int small: 8
        property int medium: 12
        property int large: 15
        property int screen: 12
    }

    property var spacing: QtObject {
        property int xSmall: 4
        property int small: 6
        property int medium: 10
        property int large: 16
        property int xLarge: 24
    }

    property var border: QtObject {
        property int thin: 1
        property int medium: 2
        property int thick: 2
    }

    property var elevation: QtObject {
        property int shadowSteps: 4
        property real shadowBlur: 12
        property real shadowOffset: 2
        property color shadowColor: "#000000"
        property real shadowLayerOpacity: 0.12
    }

    property var card: QtObject {
        property int sizeSmall: 40
    }

    property var chip: QtObject {
        property int height: 20
        property int padding: 6
        property int spacing: 4
        property int fontSize: 10
        property int radius: theme.radius.small
    }

    property var drum: QtObject {
        property int cardInset: 8
        property int padding: 12
        property int paddingLarge: 20
        property int gap: 12
        property int gapLarge: 16
        property int rowSpacing: 20
        property int rowHeight: 78
        property int rowHeightLarge: 112
        property int textSpacing: 5
        property int textSpacingLarge: 7
        property int dateWidth: 24
        property int dateSize: 18
        property int dateSizeLarge: 22
        property int monthSize: 9
        property int labelSize: 9
        property int labelSizeLarge: 11
        property real labelSpacing: 1.2
        property int nameSize: 18
        property int nameSizeLarge: 26
        property int exerciseSize: theme.fontSize.xSmall
        property int dotSize: 6
    }

    property var week: QtObject {
        property int cellSize: 28
        property int cellRadius: 8
        property int spacing: 14
        property int labelGap: 6
        property int labelSize: theme.fontSize.xSmall
        property int iconSize: 14
    }

    property var stat: QtObject {
        property int height: 56
        property int padding: 10
        property int spacing: 12
        property int labelSize: theme.fontSize.xSmall
        property int valueSize: 22
        property int unitSize: theme.fontSize.xSmall
        property int badgeSize: 9
    }

    property var panel: QtObject {
        property int handleWidth: 40
        property int handleHeight: 4
        property int titleSize: 26
        property int rowHeight: 44
        property int rowIconSize: theme.icon.small
        property real maxHeightRatio: 0.9
    }

    property real dialogOpacity: 0.98

    property var layout: QtObject {
        property int listItemHeight: 46
        property int listItemHeightLarge: 56
        property int rowHeight: 42
        property int activeRowHeight: 58
        property int cardHeight: 96
        property int dialogBarHeight: 200
        property int indicatorSize: 22
        property int indicatorSizeLarge: 28
        property int iconSizeLarge: 44
        property int exerciseItemHeight: 25
        property int actionBarHeight: 62
    }

    property var navigation: QtObject {
        property int barHeight: 58
    }

    property var icon: QtObject {
        property int small: 16
        property int medium: 22
        property int large: 30
    }

    property var icons: QtObject {
        property string search: "qrc:/Themed/Icons/search.svg"
        property string close: "qrc:/Themed/Icons/close.svg"
        property string menu: "qrc:/Themed/Icons/menu.svg"
        property string user: "qrc:/Themed/Icons/user-outline.svg"
        property string settings: "qrc:/Themed/Icons/settings.svg"
        property string home: "qrc:/Themed/Icons/home.svg"
        property string info: "qrc:/Themed/Icons/info.svg"
        property string success: "qrc:/Themed/Icons/success.svg"
        property string warning: "qrc:/Themed/Icons/warning.svg"
        property string error: "qrc:/Themed/Icons/error.svg"

        property string barbell: "qrc:/Themed/Icons/barbell.svg"
        property string calendar: "qrc:/Themed/Icons/calendar.svg"
        property string planned: "qrc:/Themed/Icons/routines.svg"

        property string check: "qrc:/Themed/Icons/check.svg"
        property string copy: "qrc:/Themed/Icons/copy.svg"
        property string ai: "qrc:/Themed/Icons/ai.svg"
        property string importData: "qrc:/Themed/Icons/import.svg"
        property string exportData: "qrc:/Themed/Icons/export.svg"

        property string addSet: "qrc:/Themed/Icons/add-set.svg"
        property string removeSet: "qrc:/Themed/Icons/remove-set.svg"
        property string plus: "qrc:/Themed/Icons/plus.svg"
        property string minus: "qrc:/Themed/Icons/minus.svg"
        property string next: "qrc:/Themed/Icons/next.svg"
        property string previous: "qrc:/Themed/Icons/previous.svg"
        property string expand: "qrc:/Themed/Icons/chevron-right.svg"
        property string collapse: "qrc:/Themed/Icons/chevron-down.svg"
        property string startWorkout: "qrc:/Themed/Icons/start-workout.svg"
        property string moveUp: "qrc:/Themed/Icons/chevron-up.svg"
        property string moveDown: "qrc:/Themed/Icons/chevron-down.svg"
        property string reorder: "qrc:/Themed/Icons/reorder.svg"
        property string timer: "qrc:/Themed/Icons/timer.svg"
        property string video: "qrc:/Themed/Icons/video.svg"
        property string back: "qrc:/Themed/Icons/previous.svg"
        property string send: "qrc:/Themed/Icons/send.svg"
    }

    property var button: QtObject {
        property int radius: theme.radius.medium

        property var icon: QtObject {
            property int size: 46
            property int width: size
            property int height: size
            property int fontSize: theme.fontSize.small
            property int iconSize: theme.icon.medium
        }

        property var circle: QtObject {
            property int size: 42
            property int width: size
            property int height: size
            property int fontSize: theme.fontSize.normal
            property int iconSize: 26
        }

        property var badge: QtObject {
            property int size: 20
            property int width: size
            property int height: size
            property int fontSize: theme.fontSize.xSmall
            property int iconSize: 12
        }

        property var square: QtObject {
            property int size: 30
            property int width: size
            property int height: size
            property int fontSize: theme.fontSize.small
            property int iconSize: 16
        }

        property var small: QtObject {
            property int width: theme.applicationWidth * 0.2
            property int height: 34
            property int fontSize: theme.fontSize.small
            property int iconSize: 16
        }

        property var smallSquare: QtObject {
            property int size: 36
            property int width: size
            property int height: size
            property int fontSize: theme.fontSize.small
            property int iconSize: 18
        }

        property var medium: QtObject {
            property int width: theme.applicationWidth * 0.3
            property int height: 44
            property int fontSize: theme.fontSize.normal
            property int iconSize: 18
        }

        property var mediumSquare: QtObject {
            property int size: 46
            property int width: size
            property int height: size
            property int fontSize: theme.fontSize.normal
            property int iconSize: 20
        }

        property var large: QtObject {
            property int width: theme.applicationWidth * 0.5
            property int height: 52
            property int fontSize: theme.fontSize.medium
            property int iconSize: 22
        }

        property var wide: QtObject {
            property int width: theme.applicationWidth * 0.9
            property int height: 48
            property int fontSize: theme.fontSize.medium
            property int iconSize: 24
        }

        property var primary: QtObject {
            property color background: "#46A6EF"
            property color hovered: "#6FBBF3"
            property color pressed: "#3690D3"
            property color border: "#46A6EF"
            property color text: "#0E0F11"
        }

        property var tonal: QtObject {
            property color background: "#1B2D3A"
            property color hovered: "#22394A"
            property color pressed: "#16242E"
            property color border: "#2C5D82"
            property color text: "#92CBFB"
        }

        property var secondary: QtObject {
            property color background: "#8B7BF0"
            property color hovered: "#9E90F5"
            property color pressed: "#7768DA"
            property color border: "#8B7BF0"
            property color text: "#0E0F11"
        }

        property var success: QtObject {
            property color background: "#53BE70"
            property color hovered: "#67CD82"
            property color pressed: "#45A55F"
            property color border: "#53BE70"
            property color text: "#0E0F11"
        }

        property var danger: QtObject {
            property color background: "#E5695C"
            property color hovered: "#EE8175"
            property color pressed: "#C9584C"
            property color border: "#E5695C"
            property color text: "#0E0F11"
        }

        property var subtle: QtObject {
            property color background: "#1E2126"
            property color hovered: "#262A30"
            property color pressed: "#181B1F"
            property color border: "#262A30"
            property color text: "#8A9099"
        }

        property var ghost: QtObject {
            property color background: "#00000000"
            property color hovered: "#14FFFFFF"
            property color pressed: "#0AFFFFFF"
            property color border: "#2C2F34"
            property color text: "#8A9099"
        }

        property var outline: QtObject {
            property color background: "transparent"
            property color hovered: "#14FFFFFF"
            property color pressed: "#0AFFFFFF"
            property color border: "#2C2F34"
            property color text: "#8A9099"
        }
    }
}
