pragma Singleton
import QtQuick
import Themed.Theme

DefaultTheme {
    id: theme

    isNightMode: true
    contentMaxWidthBase: 600

    colors: QtObject {
        id: palette

        readonly property color background: "#0E0F11"
        readonly property color backgroundDark: palette.background
        readonly property color backgroundLight: palette.background
        readonly property color surface: "#141619"
        readonly property color surfaceMuted: "#101214"
        readonly property color surfaceSunken: "#121316"
        readonly property color surfaceRaised: "#131518"
        readonly property color surfaceAccent: "#1A2028"
        readonly property color dialogSurface: "#191C20"

        readonly property color primary: "#46A6EF"
        readonly property color primaryDark: "#3690D3"
        readonly property color primaryVariant: "#6FBBF3"
        readonly property color primarySurface: "#1B2D3A"
        readonly property color primaryBorder: "#2C5D82"
        readonly property color primaryLight: "#86C5FA"

        readonly property color secondary: "#8B7BF0"
        readonly property color accent: "#E7B84B"
        readonly property color info: "#46A6EF"
        readonly property color success: "#53BE70"
        readonly property color successSurface: "#37784A"
        readonly property color warning: "#E7B84B"
        readonly property color error: "#E5695C"

        readonly property color border: "#202225"
        readonly property color borderStrong: "#3A3B3E"
        readonly property color borderSubtle: "#1C1E20"
        readonly property color divider: "#242628"
        readonly property color cardBackground: "#141619"
        readonly property color cardBorder: "#202225"
        readonly property color cardBorderHover: "#3A3B3E"
        readonly property color chipBackground: "#252729"
        readonly property color chipText: "#9AA0A8"

        readonly property color textPrimary: "#F2F3F5"
        readonly property color textSecondary: "#C9CDD3"
        readonly property color textMuted: "#8A9099"
        readonly property color textDisabled: "#585C63"
        readonly property color textFaint: "#4B4F55"
        readonly property color textSoft: "#9B9D9F"
        readonly property color textInverse: "#0E0F11"
        readonly property color textPlaceholder: "#585C63"
        readonly property color buttonText: "#0E0F11"

        readonly property color overlay: "#000000B3"
        readonly property color overlayLight: "#A6000000"
    }

    fontSize: QtObject {
        readonly property int huge: 80
        readonly property int xxLarge: 40
        readonly property int xLarge: 28
        readonly property int large: 21
        readonly property int xMedium: 18
        readonly property int medium: 16
        readonly property int normal: 14
        readonly property int small: 12
        readonly property int xSmall: 11
    }

    spacing: QtObject {
        readonly property int xSmall: 4
        readonly property int small: 6
        readonly property int medium: 10
        readonly property int large: 16
        readonly property int xLarge: 24
        readonly property int xxLarge: 32
    }

    padding: QtObject {
        readonly property int xSmall: 4
        readonly property int small: 8
        readonly property int medium: 12
        readonly property int large: 15
        readonly property int xLarge: 24
        readonly property int screen: 12
    }

    radius: QtObject {
        readonly property int small: 6
        readonly property int medium: 12
        readonly property int large: 16
        readonly property int xLarge: 22
        readonly property int full: 9999
    }

    border: QtObject {
        readonly property int thin: 1
        readonly property int medium: 2
        readonly property int thick: 2
    }

    opacity: QtObject {
        readonly property real dialog: 0.98
        readonly property real card: 1.0
        readonly property real disabled: 0.5
        readonly property real hover: 0.08
    }

    elevation: QtObject {
        readonly property int shadowSteps: 4
        readonly property real shadowBlur: 12
        readonly property real shadowOffset: 2
        readonly property color shadowColor: "#000000"
        readonly property real shadowLayerOpacity: 0.12
    }

    card: QtObject {
        readonly property int sizeSmall: 40
        readonly property int sizeMedium: 130
        readonly property int sizeLarge: 180
    }

    navigation: QtObject {
        readonly property int barHeight: 58
    }

    icon: QtObject {
        readonly property int small: 16
        readonly property int medium: 22
        readonly property int large: 30
    }

    panel: QtObject {
        readonly property int handleWidth: 40
        readonly property int handleHeight: 4
        readonly property int titleSize: 26
        readonly property int rowHeight: 44
        readonly property int rowIconSize: theme.icon.small
        readonly property int filterLabelWidth: 84
        readonly property real maxHeightRatio: 0.9
    }

    icons: QtObject {
        readonly property string search: "qrc:/Themed/Icons/search.svg"
        readonly property string close: "qrc:/Themed/Icons/close.svg"
        readonly property string menu: "qrc:/Themed/Icons/menu.svg"
        readonly property string more: "qrc:/Themed/Icons/more-vertical.svg"
        readonly property string user: "qrc:/Themed/Icons/user-outline.svg"
        readonly property string settings: "qrc:/Themed/Icons/settings.svg"
        readonly property string sliders: "qrc:/Themed/Icons/sliders.svg"
        readonly property string edit: "qrc:/Themed/Icons/edit.svg"
        readonly property string home: "qrc:/Themed/Icons/home.svg"
        readonly property string list: "qrc:/Themed/Icons/list.svg"
        readonly property string deck: "qrc:/Themed/Icons/deck.svg"
        readonly property string chat: "qrc:/Themed/Icons/chat.svg"
        readonly property string send: "qrc:/Themed/Icons/send.svg"
        readonly property string info: "qrc:/Themed/Icons/info.svg"
        readonly property string success: "qrc:/Themed/Icons/success.svg"
        readonly property string warning: "qrc:/Themed/Icons/warning.svg"
        readonly property string error: "qrc:/Themed/Icons/error.svg"

        readonly property string barbell: "qrc:/Themed/Icons/barbell.svg"
        readonly property string calendar: "qrc:/Themed/Icons/calendar.svg"
        readonly property string planned: "qrc:/Themed/Icons/routines.svg"
        readonly property string startWorkout: "qrc:/Themed/Icons/start-workout.svg"
        readonly property string timer: "qrc:/Themed/Icons/timer.svg"
        readonly property string video: "qrc:/Themed/Icons/video.svg"

        readonly property string check: "qrc:/Themed/Icons/check.svg"
        readonly property string copy: "qrc:/Themed/Icons/copy.svg"
        readonly property string paste: "qrc:/Themed/Icons/import.svg"
        readonly property string ai: "qrc:/Themed/Icons/ai.svg"
        readonly property string aiApp: "qrc:/Themed/Icons/ai-app.svg"
        readonly property string importData: "qrc:/Themed/Icons/import.svg"
        readonly property string exportData: "qrc:/Themed/Icons/export.svg"

        readonly property string addSet: "qrc:/Themed/Icons/add-set.svg"
        readonly property string removeSet: "qrc:/Themed/Icons/remove.svg"
        readonly property string remove: "qrc:/Themed/Icons/remove.svg"
        readonly property string trash: "qrc:/Themed/Icons/remove.svg"
        readonly property string plus: "qrc:/Themed/Icons/plus.svg"
        readonly property string minus: "qrc:/Themed/Icons/minus.svg"

        readonly property string back: "qrc:/Themed/Icons/previous.svg"
        readonly property string previous: "qrc:/Themed/Icons/previous.svg"
        readonly property string next: "qrc:/Themed/Icons/next.svg"
        readonly property string expand: "qrc:/Themed/Icons/chevron-right.svg"
        readonly property string collapse: "qrc:/Themed/Icons/chevron-down.svg"
        readonly property string chevronUp: "qrc:/Themed/Icons/chevron-up.svg"
        readonly property string chevronDown: "qrc:/Themed/Icons/chevron-down.svg"
        readonly property string chevronRight: "qrc:/Themed/Icons/chevron-right.svg"
        readonly property string moveUp: "qrc:/Themed/Icons/chevron-up.svg"
        readonly property string moveDown: "qrc:/Themed/Icons/chevron-down.svg"
        readonly property string reorder: "qrc:/Themed/Icons/reorder.svg"

        readonly property string star: "qrc:/Themed/Icons/star.svg"
        readonly property string starFull: "qrc:/Themed/Icons/star-full.svg"
        readonly property string starHalf: "qrc:/Themed/Icons/star-half.svg"
        readonly property string speaker: "qrc:/Themed/Icons/speaker.svg"
        readonly property string translate: "qrc:/Themed/Icons/translate.svg"
        readonly property string globe: "qrc:/Themed/Icons/globe.svg"
        readonly property string curvedArrow: "qrc:/Themed/Icons/curved-arrow.svg"
    }

    button: QtObject {
        readonly property int radius: theme.radius.medium

        readonly property QtObject icon: QtObject {
            readonly property int size: 46
            readonly property int width: size
            readonly property int height: size
            readonly property int fontSize: theme.fontSize.small
            readonly property int iconSize: theme.icon.medium
        }

        readonly property QtObject circle: QtObject {
            readonly property int size: 42
            readonly property int width: size
            readonly property int height: size
            readonly property int fontSize: theme.fontSize.normal
            readonly property int iconSize: 26
        }

        readonly property QtObject badge: QtObject {
            readonly property int size: 20
            readonly property int width: size
            readonly property int height: size
            readonly property int fontSize: theme.fontSize.xSmall
            readonly property int iconSize: 12
        }

        readonly property QtObject square: QtObject {
            readonly property int size: 30
            readonly property int width: size
            readonly property int height: size
            readonly property int fontSize: theme.fontSize.small
            readonly property int iconSize: 16
        }

        readonly property QtObject small: QtObject {
            readonly property int width: theme.applicationWidth * 0.2
            readonly property int height: 34
            readonly property int fontSize: theme.fontSize.small
            readonly property int iconSize: 16
        }

        readonly property QtObject smallSquare: QtObject {
            readonly property int size: 36
            readonly property int width: size
            readonly property int height: size
            readonly property int fontSize: theme.fontSize.small
            readonly property int iconSize: 18
        }

        readonly property QtObject medium: QtObject {
            readonly property int width: theme.applicationWidth * 0.3
            readonly property int height: 44
            readonly property int fontSize: theme.fontSize.normal
            readonly property int iconSize: 18
        }

        readonly property QtObject mediumSquare: QtObject {
            readonly property int size: 46
            readonly property int width: size
            readonly property int height: size
            readonly property int fontSize: theme.fontSize.normal
            readonly property int iconSize: 20
        }

        readonly property QtObject large: QtObject {
            readonly property int width: theme.applicationWidth * 0.5
            readonly property int height: 52
            readonly property int fontSize: theme.fontSize.medium
            readonly property int iconSize: 22
        }

        readonly property QtObject wide: QtObject {
            readonly property int width: theme.applicationWidth * 0.9
            readonly property int height: 48
            readonly property int fontSize: theme.fontSize.medium
            readonly property int iconSize: 24
        }

        readonly property QtObject primary: QtObject {
            readonly property color background: "#46A6EF"
            readonly property color hovered: "#6FBBF3"
            readonly property color pressed: "#3690D3"
            readonly property color border: "#46A6EF"
            readonly property color text: "#0E0F11"
        }

        readonly property QtObject tonal: QtObject {
            readonly property color background: "#1B2D3A"
            readonly property color hovered: "#22394A"
            readonly property color pressed: "#16242E"
            readonly property color border: "#2C5D82"
            readonly property color text: "#92CBFB"
        }

        readonly property QtObject secondary: QtObject {
            readonly property color background: "#8B7BF0"
            readonly property color hovered: "#9E90F5"
            readonly property color pressed: "#7768DA"
            readonly property color border: "#8B7BF0"
            readonly property color text: "#0E0F11"
        }

        readonly property QtObject success: QtObject {
            readonly property color background: "#53BE70"
            readonly property color hovered: "#67CD82"
            readonly property color pressed: "#45A55F"
            readonly property color border: "#53BE70"
            readonly property color text: "#0E0F11"
        }

        readonly property QtObject danger: QtObject {
            readonly property color background: "#E5695C"
            readonly property color hovered: "#EE8175"
            readonly property color pressed: "#C9584C"
            readonly property color border: "#E5695C"
            readonly property color text: "#0E0F11"
        }

        readonly property QtObject dangerSubtle: QtObject {
            readonly property color background: "#2C1B1A"
            readonly property color hovered: "#3A2321"
            readonly property color pressed: "#231413"
            readonly property color border: "#5A2F2A"
            readonly property color text: "#E5695C"
        }

        readonly property QtObject sunken: QtObject {
            readonly property color background: "#101214"
            readonly property color hovered: "#16191D"
            readonly property color pressed: "#0B0D0F"
            readonly property color border: "#202225"
            readonly property color text: "#8A9099"
        }

        readonly property QtObject subtle: QtObject {
            readonly property color background: "#1E2126"
            readonly property color hovered: "#262A30"
            readonly property color pressed: "#181B1F"
            readonly property color border: "#262A30"
            readonly property color text: "#8A9099"
        }

        readonly property QtObject ghost: QtObject {
            readonly property color background: "#00000000"
            readonly property color hovered: "#14FFFFFF"
            readonly property color pressed: "#0AFFFFFF"
            readonly property color border: "#2C2F34"
            readonly property color text: "#8A9099"
        }

        readonly property QtObject outline: QtObject {
            readonly property color background: "transparent"
            readonly property color hovered: "#14FFFFFF"
            readonly property color pressed: "#0AFFFFFF"
            readonly property color border: "#2C2F34"
            readonly property color text: "#8A9099"
        }
    }

    property QtObject layout: QtObject {
        readonly property int listItemHeight: 46
        readonly property int listItemHeightLarge: 56
        readonly property int rowHeight: 42
        readonly property int activeRowHeight: 58
        readonly property int cardHeight: 96
        readonly property int indicatorSize: 22
        readonly property int indicatorSizeLarge: 28
        readonly property int iconSizeLarge: 44
        readonly property int exerciseItemHeight: 25
        readonly property int actionBarHeight: 62
    }

    property QtObject chip: QtObject {
        readonly property int height: 20
        readonly property int padding: 6
        readonly property int spacing: 4
        readonly property int fontSize: 10
        readonly property int radius: theme.radius.small
    }

    property QtObject drum: QtObject {
        readonly property int cardInset: 8
        readonly property int padding: 12
        readonly property int paddingLarge: 20
        readonly property int gap: 12
        readonly property int gapLarge: 16
        readonly property int rowSpacing: 20
        readonly property int rowHeight: 78
        readonly property int rowHeightLarge: 112
        readonly property int textSpacing: 5
        readonly property int textSpacingLarge: 7
        readonly property int dateWidth: 24
        readonly property int dateSize: 18
        readonly property int dateSizeLarge: 22
        readonly property int monthSize: 9
        readonly property int labelSize: 9
        readonly property int labelSizeLarge: 11
        readonly property real labelSpacing: 1.2
        readonly property int nameSize: 18
        readonly property int nameSizeLarge: 26
        readonly property int exerciseSize: theme.fontSize.xSmall
        readonly property int dotSize: 6
    }

    property QtObject week: QtObject {
        readonly property int cellSize: 28
        readonly property int cellRadius: 8
        readonly property int spacing: 14
        readonly property int labelGap: 6
        readonly property int labelSize: theme.fontSize.xSmall
        readonly property int iconSize: 14
    }

    property QtObject stat: QtObject {
        readonly property int height: 56
        readonly property int padding: 10
        readonly property int spacing: 12
        readonly property int labelSize: theme.fontSize.xSmall
        readonly property real labelSpacing: 1.2
        readonly property int valueSize: 22
        readonly property int unitSize: theme.fontSize.xSmall
        readonly property int badgeSize: 9
    }

    property QtObject timer: QtObject {
        readonly property int barHeight: 178
        readonly property int compactHeight: theme.layout.listItemHeight
        readonly property int timeSize: 72
        readonly property int compactTimeSize: theme.fontSize.large
        readonly property int labelSize: theme.fontSize.xSmall
        readonly property real labelSpacing: 1.4
    }

    property QtObject setRow: QtObject {
        readonly property int numberWidth: 24
        readonly property int primaryWidth: 66
        readonly property int previousWidth: 84
        readonly property int previousFontSize: 11
        readonly property int valueMinWidth: 78
        readonly property int valueFontSize: 14
        readonly property int valueCurrentFontSize: 16
        readonly property int stepperHeight: 38
        readonly property int actionHeight: 36
        readonly property int removeWidth: 72
        readonly property int labelSize: theme.fontSize.xSmall
        readonly property real labelSpacing: 1.2
    }
}
