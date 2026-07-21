#pragma once

#include <QObject>
#include <QTranslator>
#include <QVariantList>
#include <QtQmlIntegration/qqmlintegration.h>

namespace adb_studio
{

class LanguageService : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("LanguageService is provided by the application")
    Q_PROPERTY(QString languageCode READ languageCode WRITE setLanguageCode NOTIFY languageChanged)
    Q_PROPERTY(QVariantList languages READ languages CONSTANT)

  public:
    explicit LanguageService(QObject* parent = nullptr);

    [[nodiscard]] QString languageCode() const;
    [[nodiscard]] QVariantList languages() const;

  public slots:
    void setLanguageCode(const QString& code);

  signals:
    void languageChanged();
    void retranslateRequested();

  private:
    QString m_languageCode = QStringLiteral("en");
    QTranslator m_translator;
};

} // namespace adb_studio
