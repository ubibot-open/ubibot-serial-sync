#pragma once

#include <QObject>
#include <QQmlEngine>
#include <QVariantList>

// Stateless source of the option lists SerialSettingsPanel.qml and
// ConnectionWizardDialog.qml populate their dropdowns from. Keeping the
// QSerialPort enum values behind these Q_INVOKABLE methods (rather than
// hardcoding ints in QML) means the enum's actual integer values are an
// implementation detail QML never needs to know.
//
// Labels are translated (tr()), so QML must re-call these after
// AppController.currentLanguage changes to pick up the new text -- see
// SerialSettingsPanel.qml's onCurrentLanguageChanged handler.
class SerialOptions : public QObject {
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON

public:
    using QObject::QObject;

    // Each entry is {label, value}.
    Q_INVOKABLE QVariantList dataBitsOptions() const;
    Q_INVOKABLE QVariantList parityOptions() const;
    Q_INVOKABLE QVariantList stopBitsOptions() const;
    Q_INVOKABLE QVariantList flowControlOptions() const;
    Q_INVOKABLE QVariantList baudRateOptions() const;
};
