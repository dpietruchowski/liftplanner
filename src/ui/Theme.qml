pragma Singleton
import QtQuick 2.15

QtObject {
    id: theme

    property color background: "#121212"     
    property color surface: "#1E1E1E"      
    property color primary: "#1F618D"        
    property color primaryVariant: "#2471A3"  
    property color secondary: "#03DAC6"      
    property color success: "#27ae60"       
    property color error: "#e74c3c"          
    property color border: "#333333"          

    property color textPrimary: "#FFFFFF"     
    property color textSecondary: "#BBBBBB"   
    property color textDisabled: "#666666"

    property color buttonText: "#FFFFFF"

    property int borderRadius: 8
    property int padding: 12
    property int spacing: 12

    property int fontLarge: 22
    property int fontMedium: 18
    property int fontSmall: 14
}
