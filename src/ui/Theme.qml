pragma Singleton
import QtQuick

QtObject {
    id: theme

    property int applicationWidth: 360
    property int applicationHeight: 640

    property bool isNightMode: true

    property var colors: QtObject {
        property color background: "#121212"
        property color surface: "#1E1E1E"
        property color dialogSurface: "#2A2A2A"
        property color primary: "#1F618D"
        property color primaryVariant: "#2471A3"
        property color secondary: "#6C5CE7"
        property color info: "#3498db"
        property color success: "#27ae60"
        property color warning: "#f1c40f"
        property color error: "#e74c3c"
        property color border: "#333333"
        property color cardBackground: "#1E1E1E"
        property color cardBorder: "#333333"
        property color divider: "#333333"
        property color textPrimary: "#FFFFFF"
        property color textSecondary: "#BBBBBB"
        property color textDisabled: "#666666"
        property color textInverse: "#121212"
        property color textPlaceholder: "#666666"
        property color buttonText: "#FFFFFF"
        property color overlay: "#000000A0"
        property color overlayLight: "#80000000"
    }

    property var fontSize: QtObject {
        property int huge: 84
        property int xlarge: 32
        property int large: 22
        property int medium: 18
        property int small: 14
        property int xSmall: 11
    }

    property var radius: QtObject {
        property int small: 4
        property int medium: 8
        property int large: 12
    }

    property var padding: QtObject {
        property int xSmall: 4
        property int small: 6
        property int medium: 12
        property int large: 18
    }

    property var spacing: QtObject {
        property int xSmall: 4
        property int small: 6
        property int medium: 12
        property int large: 18
    }

    property var border: QtObject {
        property int thin: 1
        property int medium: 2
        property int thick: 3
    }

    property var card: QtObject {
        property int sizeSmall: 40
    }

    property real dialogOpacity: 0.98

    property var layout: QtObject {
        property int listItemHeight: 50
        property int listItemHeightLarge: 56
        property int cardHeight: 100
        property int dialogBarHeight: 200
        property int indicatorSize: 20
        property int iconSizeLarge: 48
        property int exerciseItemHeight: 25
    }

    property var navigation: QtObject {
        property int barHeight: 60
    }

    property var icon: QtObject {
        property int small: 16
        property int medium: 24
        property int large: 32
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
    }

    property var button: QtObject {
        property var square: QtObject {
            property int size: 28
            property int width: size
            property int height: size
            property int fontSize: theme.fontSize.small
            property int iconSize: 14
        }

        property var small: QtObject {
            property int width: theme.applicationWidth * 0.18
            property int height: theme.applicationHeight * 0.05
            property int fontSize: theme.fontSize.small
            property int iconSize: 16
        }

        property var smallSquare: QtObject {
            property int size: theme.applicationHeight * 0.05
            property int width: size
            property int height: size
            property int fontSize: theme.fontSize.small
            property int iconSize: size - 4
        }

        property var medium: QtObject {
            property int width: theme.applicationWidth * 0.28
            property int height: theme.applicationHeight * 0.06
            property int fontSize: theme.fontSize.medium
            property int iconSize: 20
        }

        property var mediumSquare: QtObject {
            property int size: theme.applicationHeight * 0.06
            property int width: size
            property int height: size
            property int fontSize: theme.fontSize.medium
            property int iconSize: size - 4
        }

        property var large: QtObject {
            property int width: theme.applicationWidth * 0.45
            property int height: theme.applicationHeight * 0.09
            property int fontSize: theme.fontSize.large
            property int iconSize: 24
        }

        property var primary: QtObject {
            property color background: "#1F618D"
            property color hovered: "#2874A6"
            property color pressed: "#1A4F73"
            property color border: "#2471A3"
            property color text: "#FFFFFF"
        }

        property var secondary: QtObject {
            property color background: "#6C5CE7"
            property color hovered: "#7D6FF0"
            property color pressed: "#5A4BD6"
            property color border: "#6C5CE7"
            property color text: "#FFFFFF"
        }

        property var success: QtObject {
            property color background: "#27ae60"
            property color hovered: "#2ECC71"
            property color pressed: "#1E8449"
            property color border: "#27ae60"
            property color text: "#FFFFFF"
        }

        property var danger: QtObject {
            property color background: "#e74c3c"
            property color hovered: "#EC7063"
            property color pressed: "#C0392B"
            property color border: "#e74c3c"
            property color text: "#FFFFFF"
        }

        property var ghost: QtObject {
            property color background: "#40FFFFFF"
            property color hovered: "#60FFFFFF"
            property color pressed: "#25FFFFFF"
            property color border: "#70FFFFFF"
            property color text: "#FFFFFF"
        }

        property var outline: QtObject {
            property color background: "transparent"
            property color hovered: "#15FFFFFF"
            property color pressed: "#08FFFFFF"

            property color border: "#80FFFFFF"
            property color text: "#BBBBBB"
        }
    }
}
