#pragma once

#include <QObject>
#include <QTranslator>
#include <QString>

// The three interface languages the app currently ships with. English is the
// *source* language (every tr() call in the code is written in English), so
// switching to English simply means "no translator installed". Chinese
// installs the Simplified Chinese catalog. Bilingual shows both at once
// (Chinese followed by the English source text) via BilingualTranslator
// below, which is how the original design mockup presented its labels
// (e.g. "发送 Send").
//
// To add a new language later: add a new enumerator, ship a new
// translations/ubibot_<locale>.ts (see CMakeLists.txt's qt_add_translations
// call), and extend LanguageManager::setLanguage()'s switch. Nothing else in
// the app needs to change, since all UI text goes through tr().
enum class AppLanguage {
    Chinese,
    English,
    Bilingual
};

// A QTranslator that answers every lookup with "<zh> <en-source>" so widgets
// using tr() as usual get a combined bilingual string for free. Falls back to
// the plain source text when no Chinese translation exists for a given
// string (e.g. it was just added and the .ts hasn't been retranslated yet).
class BilingualTranslator : public QTranslator {
    Q_OBJECT
public:
    explicit BilingualTranslator(QObject *parent = nullptr);

    bool loadChinese(const QString &filePath);

    QString translate(const char *context, const char *sourceText,
                       const char *disambiguation = nullptr, int n = -1) const override;
    bool isEmpty() const override;

private:
    QTranslator zh_;
};

// Small app-wide singleton that owns the installed QTranslator(s) and lets
// any part of the UI react to a runtime language switch via languageChanged().
// Widgets should override changeEvent() and watch for QEvent::LanguageChange
// (which Qt posts automatically after installTranslator()/removeTranslator())
// to re-run their retranslateUi().
class LanguageManager : public QObject {
    Q_OBJECT
public:
    static LanguageManager &instance();

    void setLanguage(AppLanguage lang);
    AppLanguage language() const { return current_; }

    // Convenience for code that renders data-driven (non tr()) text, such as
    // the device command library loaded from JSON.
    QString pick(const QString &zh, const QString &en) const;

signals:
    void languageChanged(AppLanguage lang);

private:
    LanguageManager();
    void ensureLoaded();

    AppLanguage current_ = AppLanguage::Bilingual;
    QTranslator zhOnly_;
    BilingualTranslator bilingual_;
    bool loaded_ = false;
};
