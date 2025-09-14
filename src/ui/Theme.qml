pragma Singleton
import QtQuick 2.15

QtObject {
    id: theme

    property int applicationWidth: 360
    property int applicationHeight: 640

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

    property color textPrimary: "#FFFFFF"
    property color textSecondary: "#BBBBBB"
    property color textDisabled: "#666666"
    property color buttonText: "#FFFFFF"

    property int borderRadius: 8
    property int padding: 12
    property int spacing: 12

    property int fontHuge: 84
    property int fontXLarge: 32
    property int fontLarge: 22
    property int fontMedium: 18
    property int fontSmall: 14

    property color overlay: "#000000A0"
    property real dialogOpacity: 0.98
    property real cardOpacity: 0.98

    property int timerCircleMax: 360
    property int timerBorderWidth: 2

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

        property string check: "qrc:/LiftPlanner/ui/icons/check.svg"

        property string addSet: "qrc:/LiftPlanner/ui/icons/add-set.svg"
        property string removeSet: "qrc:/LiftPlanner/ui/icons/remove-set.svg"
        property string next: "qrc:/LiftPlanner/ui/icons/next.svg"
        property string previous: "qrc:/LiftPlanner/ui/icons/previous.svg"
        property string expand: "qrc:/LiftPlanner/ui/icons/expand.svg"
        property string collapse: "qrc:/LiftPlanner/ui/icons/collapse.svg"
        property string startWorkout: "qrc:/LiftPlanner/ui/icons/start-workout.svg"
    }

    property var buttonSquare: QtObject {
        property int size: 28
        property int width: size
        property int height: size
        property int fontSize: fontSmall
        property int iconSize: size
    }

    property var buttonSmall: QtObject {
        property int width: theme.applicationWidth * 0.15
        property int height: theme.applicationHeight * 0.05
        property int fontSize: fontSmall
        property int iconSize: 16
    }
    property var buttonMedium: QtObject {
        property int width: theme.applicationWidth * 0.25
        property int height: theme.applicationHeight * 0.06
        property int fontSize: fontMedium
        property int iconSize: 20
    }
    property var buttonLarge: QtObject {
        property int width: theme.applicationWidth * 0.40
        property int height: theme.applicationHeight * 0.09
        property int fontSize: fontLarge
        property int iconSize: 24
    }

    property var buttonStylePrimary: QtObject {
        property color background: "#1F618D"
        property color hovered: "#2874A6"
        property color pressed: "#1A4F73"
        property color border: "#2471A3"
        property color text: "#FFFFFF"
    }
    property var buttonStyleSecondary: QtObject {
        property color background: "#03DAC6"
        property color hovered: "#26E2D0"
        property color pressed: "#00BFA5"
        property color border: "#03DAC6"
        property color text: "#212121"
    }
    property var buttonStyleSuccess: QtObject {
        property color background: "#27ae60"
        property color hovered: "#2ECC71"
        property color pressed: "#1E8449"
        property color border: "#27ae60"
        property color text: "#FFFFFF"
    }
    property var buttonStyleDanger: QtObject {
        property color background: "#e74c3c"
        property color hovered: "#EC7063"
        property color pressed: "#C0392B"
        property color border: "#e74c3c"
        property color text: "#FFFFFF"
    }
}
