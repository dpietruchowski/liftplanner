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
        property color secondary: "#03DAC6"
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
    }

    property var radius: QtObject {
        property int small: 4
        property int medium: 8
        property int large: 12
    }

    property var padding: QtObject {
        property int xsmall: 4
        property int small: 6
        property int medium: 12
        property int large: 18
    }

    property var spacing: QtObject {
        property int xsmall: 4
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
        property int navBarHeight: 60
        property int navItemWidth: 70
        property int navItemHeight: 50
        property int navBarSpacing: 24
        property int listItemHeight: 50
        property int listItemHeightLarge: 56
        property int cardHeight: 100
        property int dialogBarHeight: 120
        property int indicatorSize: 20
        property int iconSizeLarge: 48
        property int exerciseItemHeight: 25
    }

    property var icons: QtObject {
        property string search: "qrc:/LiftPlanner/ui/icons/search.svg"
        property string close: "qrc:/LiftPlanner/ui/icons/close.svg"
        property string menu: "qrc:/LiftPlanner/ui/icons/menu.svg"
        property string user: "qrc:/LiftPlanner/ui/icons/user.svg"
        property string settings: "qrc:/LiftPlanner/ui/icons/settings.svg"
        property string home: "qrc:/LiftPlanner/ui/icons/home.svg"
        property string info: "qrc:/LiftPlanner/ui/icons/info.svg"
        property string success: "qrc:/LiftPlanner/ui/icons/success.svg"
        property string warning: "qrc:/LiftPlanner/ui/icons/warning.svg"
        property string error: "qrc:/LiftPlanner/ui/icons/error.svg"

        property string barbell: "qrc:/LiftPlanner/ui/icons/barbell.svg"
        property string calendar: "qrc:/LiftPlanner/ui/icons/calendar.svg"
        property string planned: "qrc:/LiftPlanner/ui/icons/routines.svg"

        property string check: "qrc:/LiftPlanner/ui/icons/check.svg"
        property string copy: "qrc:/LiftPlanner/ui/icons/copy.svg"

        property string addSet: "qrc:/LiftPlanner/ui/icons/add-set.svg"
        property string removeSet: "qrc:/LiftPlanner/ui/icons/remove-set.svg"
        property string next: "qrc:/LiftPlanner/ui/icons/next.svg"
        property string previous: "qrc:/LiftPlanner/ui/icons/previous.svg"
        property string expand: "qrc:/LiftPlanner/ui/icons/expand.svg"
        property string collapse: "qrc:/LiftPlanner/ui/icons/collapse.svg"
        property string startWorkout: "qrc:/LiftPlanner/ui/icons/start-workout.svg"
    }

    property var button: QtObject {
        property var square: QtObject {
            property int size: 28
            property int width: size
            property int height: size
            property int fontSize: theme.fontSize.small
            property int iconSize: size - 4
        }

        property var small: QtObject {
            property int width: theme.applicationWidth * 0.18
            property int height: theme.applicationHeight * 0.05
            property int fontSize: theme.fontSize.small
            property int iconSize: 16
        }

        property var medium: QtObject {
            property int width: theme.applicationWidth * 0.28
            property int height: theme.applicationHeight * 0.06
            property int fontSize: theme.fontSize.medium
            property int iconSize: 20
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
            property color background: "#03DAC6"
            property color hovered: "#26E2D0"
            property color pressed: "#00BFA5"
            property color border: "#03DAC6"
            property color text: "#212121"
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
    }
}
