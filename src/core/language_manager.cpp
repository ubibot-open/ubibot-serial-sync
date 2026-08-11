#include "core/language_manager.h"

#include <QCoreApplication>

namespace {
constexpr auto kZhResourcePath = ":/i18n/ubibot_zh_CN.qm";
}

BilingualTranslator::BilingualTranslator(QObject *parent) : QTranslator(parent) {}

bool BilingualTranslator::loadChinese(const QString &filePath) {
    return zh_.load(filePath);
}

bool BilingualTranslator::isEmpty() const {
    return false;
}

QString BilingualTranslator::translate(const char *context, const char *sourceText,
                                        const char *disambiguation, int n) const {
    const QString source = QString::fromUtf8(sourceText);
    if (source.isEmpty()) return source;
    const QString zh = zh_.translate(context, sourceText, disambiguation, n);
    if (zh.isEmpty() || zh == source) return source;
    return zh + QStringLiteral(" ") + source;
}

LanguageManager::LanguageManager() = default;

LanguageManager &LanguageManager::instance() {
    static LanguageManager mgr;
    return mgr;
}

void LanguageManager::ensureLoaded() {
    if (loaded_) return;
    zhOnly_.load(kZhResourcePath);
    bilingual_.loadChinese(kZhResourcePath);
    loaded_ = true;
}

void LanguageManager::setLanguage(AppLanguage lang) {
    ensureLoaded();

    qApp->removeTranslator(&zhOnly_);
    qApp->removeTranslator(&bilingual_);

    switch (lang) {
    case AppLanguage::Chinese:
        qApp->installTranslator(&zhOnly_);
        break;
    case AppLanguage::Bilingual:
        qApp->installTranslator(&bilingual_);
        break;
    case AppLanguage::English:
        // Source strings already are English; nothing to install.
        break;
    }

    current_ = lang;
    emit languageChanged(current_);
}

QString LanguageManager::pick(const QString &zh, const QString &en) const {
    switch (current_) {
    case AppLanguage::Chinese:
        return zh.isEmpty() ? en : zh;
    case AppLanguage::English:
        return en.isEmpty() ? zh : en;
    case AppLanguage::Bilingual:
        if (zh.isEmpty()) return en;
        if (en.isEmpty() || en == zh) return zh;
        return zh + QStringLiteral(" ") + en;
    }
    return en;
}
