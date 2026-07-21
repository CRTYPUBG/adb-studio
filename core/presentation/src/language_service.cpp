#include "adb_studio/presentation/language_service.hpp"

#include <QCoreApplication>
#include <QLocale>
#include <QSettings>

namespace adb_studio
{

LanguageService::LanguageService(QObject* parent) : QObject(parent)
{
    QSettings settings;
    const QString systemLanguage = QLocale::system().language() == QLocale::Turkish
                                       ? QStringLiteral("tr")
                                       : QStringLiteral("en");
    setLanguageCode(settings.value(QStringLiteral("ui/language"), systemLanguage).toString());
}

QString LanguageService::languageCode() const
{
    return m_languageCode;
}

// QML property readers are required to be instance methods by the meta-object contract.
// NOLINTNEXTLINE(readability-convert-member-functions-to-static)
QVariantList LanguageService::languages() const
{
    return {QVariantMap{{QStringLiteral("code"), QStringLiteral("en")},
                        {QStringLiteral("label"), QStringLiteral("English")}},
            QVariantMap{{QStringLiteral("code"), QStringLiteral("tr")},
                        {QStringLiteral("label"), QStringLiteral("Türkçe")}}};
}

void LanguageService::setLanguageCode(const QString& code)
{
    const QString normalized = code.toLower();
    if (normalized != QStringLiteral("en") && normalized != QStringLiteral("tr"))
    {
        return;
    }
    if (normalized == m_languageCode &&
        (normalized == QStringLiteral("en") || !m_translator.isEmpty()))
    {
        return;
    }
    QCoreApplication::removeTranslator(&m_translator);
    if (normalized == QStringLiteral("tr") &&
        !m_translator.load(QStringLiteral(":/i18n/adb_studio_tr.qm")))
    {
        m_languageCode = QStringLiteral("en");
    }
    else
    {
        m_languageCode = normalized;
        if (normalized == QStringLiteral("tr"))
        {
            QCoreApplication::installTranslator(&m_translator);
        }
    }
    QSettings().setValue(QStringLiteral("ui/language"), m_languageCode);
    emit languageChanged();
    emit retranslateRequested();
}

} // namespace adb_studio
