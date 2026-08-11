#include "core/language_manager.h"

#include <QCoreApplication>

namespace {

// "en" is the source language embedded in every tr() call, so it has no
// .ts/.qm file and is always available. Every other entry here must have a
// matching translations/ubibot_<code>.ts wired into CMakeLists.txt's
// qt_add_translations() call.
const QVector<LanguageInfo> kLanguages = {
    {QStringLiteral("en"), QStringLiteral("English")},
    {QStringLiteral("zh_CN"), QStringLiteral("简体中文")},
};

}  // namespace

LanguageManager::LanguageManager() = default;

LanguageManager &LanguageManager::instance() {
    static LanguageManager mgr;
    return mgr;
}

const QVector<LanguageInfo> &LanguageManager::availableLanguages() {
    return kLanguages;
}

void LanguageManager::setLanguage(const QString &code) {
    qApp->removeTranslator(&translator_);

    if (code != QStringLiteral("en")) {
        translator_.load(QStringLiteral(":/i18n/ubibot_%1.qm").arg(code));
        qApp->installTranslator(&translator_);
    }

    current_ = code;
    emit languageChanged(current_);
}

QString LanguageManager::pick(const QString &zh, const QString &en) const {
    const bool wantsChinese = current_.startsWith(QStringLiteral("zh"));
    if (wantsChinese) return zh.isEmpty() ? en : zh;
    return en.isEmpty() ? zh : en;
}
