#pragma once

#include <QObject>
#include <QString>
#include <QTranslator>
#include <QVector>

// One entry in the interface-language picker. `code` is a locale-ish tag
// ("en", "zh_CN", ...) used both as the QSettings value and to find the
// matching translation catalog ("en" is the *source* language every tr()
// call is written in, so it has no .ts/.qm file and needs none).
// `nativeName` is always written in that language's own script (e.g.
// "简体中文", "English") so it reads correctly no matter which language is
// currently active -- that's why it's never run through tr().
struct LanguageInfo {
    QString code;
    QString nativeName;
};

// Small app-wide singleton that owns the installed QTranslator and lets any
// part of the UI react to a runtime language switch via languageChanged().
// Widgets should override changeEvent() and watch for QEvent::LanguageChange
// (which Qt posts automatically after installTranslator()/removeTranslator())
// to re-run their retranslateUi().
//
// To add a new language later: add one entry to kLanguages in
// language_manager.cpp and ship a matching translations/ubibot_<code>.ts
// (see CMakeLists.txt's qt_add_translations call). Nothing else needs to
// change, since all UI text goes through tr() and the language picker
// (SettingsAboutDialog) is populated from availableLanguages().
class LanguageManager : public QObject {
    Q_OBJECT
public:
    static LanguageManager &instance();

    // The full list this build ships translations for, in the order they
    // should appear in the language picker.
    static const QVector<LanguageInfo> &availableLanguages();

    void setLanguage(const QString &code);
    QString language() const { return current_; }

    // Convenience for code that renders data-driven (non tr()) text, such as
    // the device command library loaded from JSON, which only has zh/en
    // pairs today. Picks zh for any Chinese variant, en otherwise.
    QString pick(const QString &zh, const QString &en) const;

signals:
    void languageChanged(const QString &code);

private:
    LanguageManager();

    QTranslator translator_;
    QString current_ = QStringLiteral("en");
};
