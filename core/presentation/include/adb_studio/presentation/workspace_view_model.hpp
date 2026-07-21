#pragma once

#include <QObject>
#include <QString>
#include <QStringList>
#include <QVariantList>
#include <QtQmlIntegration/qqmlintegration.h>

namespace adb_studio
{

class WorkspaceViewModel : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("WorkspaceViewModel is provided by the application")
    Q_PROPERTY(QVariantList modules READ modules CONSTANT)
    Q_PROPERTY(QString currentModuleKey READ currentModuleKey NOTIFY currentModuleChanged)
    Q_PROPERTY(QString title READ title NOTIFY currentModuleChanged)
    Q_PROPERTY(QString description READ description NOTIFY currentModuleChanged)
    Q_PROPERTY(QVariantList features READ features NOTIFY contentChanged)
    Q_PROPERTY(QStringList filters READ filters NOTIFY currentModuleChanged)
    Q_PROPERTY(QString searchText READ searchText WRITE setSearchText NOTIFY searchTextChanged)
    Q_PROPERTY(QString activeFilter READ activeFilter WRITE setActiveFilter NOTIFY contentChanged)
    Q_PROPERTY(bool busy READ busy NOTIFY busyChanged)
    Q_PROPERTY(bool empty READ empty NOTIFY contentChanged)
    Q_PROPERTY(QString errorMessage READ errorMessage NOTIFY errorChanged)
    Q_PROPERTY(QString statusText READ statusText NOTIFY statusChanged)
    Q_PROPERTY(QString notification READ notification NOTIFY notificationChanged)
    Q_PROPERTY(bool canUndo READ canUndo NOTIFY canUndoChanged)

  public:
    explicit WorkspaceViewModel(QObject* parent = nullptr);

    [[nodiscard]] QVariantList modules() const;
    [[nodiscard]] QString currentModuleKey() const;
    [[nodiscard]] QString title() const;
    [[nodiscard]] QString description() const;
    [[nodiscard]] QVariantList features() const;
    [[nodiscard]] QStringList filters() const;
    [[nodiscard]] QString searchText() const;
    [[nodiscard]] QString activeFilter() const;
    [[nodiscard]] bool busy() const noexcept;
    [[nodiscard]] bool empty() const;
    [[nodiscard]] QString errorMessage() const;
    [[nodiscard]] QString statusText() const;
    [[nodiscard]] QString notification() const;
    [[nodiscard]] bool canUndo() const noexcept;

    Q_INVOKABLE void selectModule(const QString& key);
    Q_INVOKABLE void refresh();
    Q_INVOKABLE void activateFeature(const QString& featureId);
    Q_INVOKABLE void clearNotification();
    Q_INVOKABLE void clearError();
    Q_INVOKABLE void undo();
    Q_INVOKABLE void retranslate();

  public slots:
    void setSearchText(const QString& text);
    void setActiveFilter(const QString& filter);

  signals:
    void currentModuleChanged();
    void contentChanged();
    void searchTextChanged();
    void busyChanged();
    void errorChanged();
    void statusChanged();
    void notificationChanged();
    void canUndoChanged();
    void commandRequested(const QString& command);

  private:
    struct Feature
    {
        QString id;
        QString title;
        QString category;
        QString command;
    };

    struct Module
    {
        QString key;
        QString shortTitle;
        QString title;
        QString description;
        QList<Feature> features;
    };

    [[nodiscard]] const Module& currentModule() const;
    [[nodiscard]] static QList<Module> createModules();
    void setBusy(bool value);
    void setNotification(const QString& message);

    QList<Module> m_modules;
    qsizetype m_currentIndex = 0;
    QString m_searchText;
    QString m_activeFilter;
    QString m_errorMessage;
    QString m_statusText;
    QString m_notification;
    QString m_previousModuleKey;
    bool m_busy = false;
    bool m_canUndo = false;
};

} // namespace adb_studio
