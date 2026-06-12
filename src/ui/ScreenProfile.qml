import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import LiftPlanner 1.0
import Themed.Components

Rectangle {
    id: root
    color: Theme.colors.background

    property bool dobInvalid: false
    property int controlWidth: 180

    component FormLabel: Text {
        font.pixelSize: Theme.fontSize.small
        color: Theme.colors.textSecondary
        Layout.fillWidth: true
    }

    component ProfileCombo: ThemedComboBox {
        id: combo

        property var values: []
        property string field

        function sync() {
            currentIndex = values.indexOf(UserProfileViewModel[field])
        }

        Layout.preferredWidth: root.controlWidth
        Layout.alignment: Qt.AlignRight

        Component.onCompleted: sync()
        onActivated: UserProfileViewModel[field] = values[currentIndex]
    }

    // ThemedInput restyled to match ThemedComboBox (same width, radius and border)
    component FormInput: ThemedInput {
        Layout.preferredWidth: root.controlWidth
        Layout.alignment: Qt.AlignRight
        implicitHeight: 44
        clearOnSubmit: false
        horizontalAlignment: Text.AlignLeft
        leftPadding: Theme.padding.medium
        rightPadding: Theme.padding.medium

        background: Rectangle {
            color: Theme.colors.surface
            radius: Theme.radius.small
            border.width: parent.activeFocus ? Theme.border.medium : Theme.border.thin
            border.color: parent.activeFocus ? Theme.colors.primary : Theme.colors.cardBorder

            Behavior on border.color { ColorAnimation { duration: 150 } }
            Behavior on border.width { NumberAnimation { duration: 150 } }
        }
    }

    function syncForm() {
        sexCombo.sync()
        goalCombo.sync()
        experienceCombo.sync()
        unitsCombo.sync()
        languageCombo.sync()
        sessionsSpin.value = UserProfileViewModel.sessionsPerWeek
        if (!dobField.activeFocus) {
            dobField.text = UserProfileViewModel.dateOfBirth
            root.dobInvalid = false
        }
        if (!bodyweightField.activeFocus)
            bodyweightField.text = UserProfileViewModel.bodyweight > 0
                ? UserProfileViewModel.bodyweight.toFixed(1) : ""
        if (!timezoneField.activeFocus)
            timezoneField.text = UserProfileViewModel.timezone
        if (notesArea.text !== UserProfileViewModel.notes)
            notesArea.text = UserProfileViewModel.notes
    }

    Component.onCompleted: syncForm()

    // Pick up changes made outside this screen (e.g. profile imported with an AI plan)
    StackView.onActivated: {
        if (!UserProfileViewModel.dirty)
            UserProfileViewModel.load()
    }

    Connections {
        target: UserProfileViewModel
        function onProfileChanged() { root.syncForm() }
        function onSaved() { toast.showMessage("Profile saved") }
    }

    ScrollView {
        id: scrollView
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: saveBar.top
        anchors.margins: Theme.padding.medium
        anchors.bottomMargin: 0
        contentWidth: availableWidth

        ColumnLayout {
            width: scrollView.contentItem.width
            spacing: Theme.spacing.medium

            // Identity header
            Rectangle {
                Layout.alignment: Qt.AlignHCenter
                Layout.topMargin: Theme.spacing.medium
                width: 72
                height: 72
                radius: width / 2
                color: Theme.colors.surface
                border.color: Theme.colors.primary
                border.width: Theme.border.medium

                ThemedIcon {
                    anchors.centerIn: parent
                    width: Theme.icon.large
                    height: Theme.icon.large
                    svgSource: Theme.icons.user
                    color: Theme.colors.textSecondary
                }
            }

            Text {
                Layout.alignment: Qt.AlignHCenter
                text: "Your training profile"
                font.pixelSize: Theme.fontSize.large
                font.bold: true
                color: Theme.colors.textPrimary
            }

            Text {
                Layout.alignment: Qt.AlignHCenter
                text: "Used to personalize AI-generated workout plans"
                font.pixelSize: Theme.fontSize.small
                color: Theme.colors.textSecondary
            }

            Row {
                Layout.alignment: Qt.AlignHCenter
                Layout.topMargin: Theme.spacing.small
                Layout.bottomMargin: Theme.spacing.small
                spacing: Theme.spacing.medium

                StatTile {
                    width: 100
                    label: "Age"
                    value: UserProfileViewModel.age >= 0 ? UserProfileViewModel.age : "—"
                }

                StatTile {
                    width: 100
                    label: "Bodyweight"
                    value: UserProfileViewModel.bodyweight > 0
                           ? UserProfileViewModel.bodyweight.toFixed(1) + " "
                             + UserProfileViewModel.bodyweightUnit
                           : "—"
                }

                StatTile {
                    width: 100
                    label: "Per week"
                    value: UserProfileViewModel.sessionsPerWeek + "×"
                }
            }

            SectionHeader {
                Layout.fillWidth: true
                title: "ABOUT YOU"
            }

            GridLayout {
                Layout.fillWidth: true
                columns: 2
                columnSpacing: Theme.spacing.medium
                rowSpacing: Theme.spacing.medium

                FormLabel { text: "Sex" }
                ProfileCombo {
                    id: sexCombo
                    field: "sex"
                    values: ["male", "female", "other"]
                    model: ["Male", "Female", "Other"]
                }

                FormLabel { text: "Date of birth" }
                FormInput {
                    id: dobField
                    placeholder: "YYYY-MM-DD"
                    inputMethodHints: Qt.ImhPreferNumbers

                    function isValidDate(t) {
                        return /^\d{4}-\d{2}-\d{2}$/.test(t) && !isNaN(Date.parse(t))
                    }

                    onTextChanged: {
                        var t = text.trim()
                        if (t === "") {
                            UserProfileViewModel.dateOfBirth = ""
                            root.dobInvalid = false
                        } else if (isValidDate(t)) {
                            UserProfileViewModel.dateOfBirth = t
                            root.dobInvalid = false
                        }
                    }
                    onEditingFinished: root.dobInvalid = text.trim() !== "" && !isValidDate(text.trim())
                }

                Text {
                    visible: root.dobInvalid
                    Layout.columnSpan: 2
                    Layout.alignment: Qt.AlignRight
                    text: "Use the YYYY-MM-DD format, e.g. 1990-05-14"
                    font.pixelSize: Theme.fontSize.small
                    color: Theme.colors.error
                }

                FormLabel { text: "Bodyweight (" + UserProfileViewModel.bodyweightUnit + ")" }
                FormInput {
                    id: bodyweightField
                    placeholder: "e.g. 82.5"
                    inputMethodHints: Qt.ImhFormattedNumbersOnly
                    validator: RegularExpressionValidator {
                        regularExpression: /^\d{0,3}([.,]\d?)?$/
                    }
                    onTextChanged: UserProfileViewModel.bodyweight =
                                       parseFloat(text.replace(",", ".")) || 0
                }
            }

            SectionHeader {
                Layout.fillWidth: true
                Layout.topMargin: Theme.spacing.medium
                title: "TRAINING"
            }

            GridLayout {
                Layout.fillWidth: true
                columns: 2
                columnSpacing: Theme.spacing.medium
                rowSpacing: Theme.spacing.medium

                FormLabel { text: "Primary goal" }
                ProfileCombo {
                    id: goalCombo
                    field: "primaryGoal"
                    values: ["general_fitness", "weight_loss", "muscle_gain",
                             "strength", "endurance"]
                    model: ["General fitness", "Weight loss", "Muscle gain",
                            "Strength", "Endurance"]
                }

                FormLabel { text: "Experience" }
                ProfileCombo {
                    id: experienceCombo
                    field: "experienceLevel"
                    values: ["beginner", "intermediate", "advanced"]
                    model: ["Beginner", "Intermediate", "Advanced"]
                }

                FormLabel { text: "Sessions per week" }
                ThemedSpinBox {
                    id: sessionsSpin
                    Layout.preferredWidth: root.controlWidth
                    Layout.alignment: Qt.AlignRight
                    from: 1
                    to: 7
                    onValueModified: UserProfileViewModel.sessionsPerWeek = value
                }
            }

            SectionHeader {
                Layout.fillWidth: true
                Layout.topMargin: Theme.spacing.medium
                title: "PREFERENCES"
            }

            GridLayout {
                Layout.fillWidth: true
                columns: 2
                columnSpacing: Theme.spacing.medium
                rowSpacing: Theme.spacing.medium

                FormLabel { text: "Units" }
                ProfileCombo {
                    id: unitsCombo
                    field: "unitSystem"
                    values: ["metric", "imperial"]
                    model: ["Metric (kg)", "Imperial (lb)"]
                }

                FormLabel { text: "Language" }
                ProfileCombo {
                    id: languageCombo
                    field: "language"
                    values: ["en", "pl"]
                    model: ["English", "Polski"]
                }

                FormLabel { text: "Timezone" }
                FormInput {
                    id: timezoneField
                    placeholder: "Europe/Warsaw"
                    font.pixelSize: Theme.fontSize.small
                    onTextChanged: UserProfileViewModel.timezone = text.trim()
                }
            }

            SectionHeader {
                Layout.fillWidth: true
                Layout.topMargin: Theme.spacing.medium
                title: "NOTES"
            }

            ThemedTextArea {
                id: notesArea
                Layout.fillWidth: true
                Layout.preferredHeight: 120
                onTextChanged: {
                    if (text !== UserProfileViewModel.notes)
                        UserProfileViewModel.notes = text
                }
            }

            Text {
                Layout.fillWidth: true
                Layout.bottomMargin: Theme.spacing.medium
                text: "Injuries, equipment, schedule constraints — anything the AI "
                      + "should consider when planning your workouts."
                wrapMode: Text.WordWrap
                font.pixelSize: Theme.fontSize.small
                color: Theme.colors.textSecondary
            }
        }
    }

    // Sticky save bar
    Rectangle {
        id: saveBar
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        height: saveButton.height + 2 * Theme.padding.medium
        color: Theme.colors.surface

        Rectangle {
            width: parent.width
            height: Theme.border.thin
            color: Theme.colors.border
            anchors.top: parent.top
        }

        ThemedButton {
            id: saveButton
            anchors.centerIn: parent
            width: parent.width - 2 * Theme.padding.medium
            text: UserProfileViewModel.dirty ? "Save profile" : "Saved"
            enabled: UserProfileViewModel.dirty
            buttonStyle: Theme.button.primary
            buttonSize: Theme.button.large
            onClicked: UserProfileViewModel.save()
        }
    }

    ThemedToast {
        id: toast
    }
}
