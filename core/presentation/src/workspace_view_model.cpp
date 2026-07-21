#include "adb_studio/presentation/workspace_view_model.hpp"

#include <QCoreApplication>
#include <QTimer>

#include <algorithm>
#include <array>

namespace adb_studio
{
namespace
{
[[maybe_unused]] constexpr std::array workspaceTranslationSources = {
    QT_TRANSLATE_NOOP("WorkspaceViewModel", "Home"),
    QT_TRANSLATE_NOOP("WorkspaceViewModel", "Dashboard"),
    QT_TRANSLATE_NOOP("WorkspaceViewModel", "Live device overview and verified connection health."),
    QT_TRANSLATE_NOOP("WorkspaceViewModel", "Scan devices"),
    QT_TRANSLATE_NOOP("WorkspaceViewModel", "Quick actions"),
    QT_TRANSLATE_NOOP("WorkspaceViewModel", "Device health"),
    QT_TRANSLATE_NOOP("WorkspaceViewModel", "Diagnostics"),
    QT_TRANSLATE_NOOP("WorkspaceViewModel", "Devices"),
    QT_TRANSLATE_NOOP("WorkspaceViewModel",
                      "Search, classify and operate verified Android transports."),
    QT_TRANSLATE_NOOP("WorkspaceViewModel", "Refresh device list"),
    QT_TRANSLATE_NOOP("WorkspaceViewModel", "Connection"),
    QT_TRANSLATE_NOOP("WorkspaceViewModel", "Restart ADB"),
    QT_TRANSLATE_NOOP("WorkspaceViewModel", "USB devices"),
    QT_TRANSLATE_NOOP("WorkspaceViewModel", "Transport"),
    QT_TRANSLATE_NOOP("WorkspaceViewModel", "Wireless devices"),
    QT_TRANSLATE_NOOP("WorkspaceViewModel", "Fastboot devices"),
    QT_TRANSLATE_NOOP("WorkspaceViewModel", "Boot mode"),
    QT_TRANSLATE_NOOP("WorkspaceViewModel", "Recovery devices"),
    QT_TRANSLATE_NOOP("WorkspaceViewModel", "Mirror"),
    QT_TRANSLATE_NOOP("WorkspaceViewModel", "Screen Mirroring"),
    QT_TRANSLATE_NOOP("WorkspaceViewModel",
                      "Isolated scrcpy sessions with verified device controls."),
    QT_TRANSLATE_NOOP("WorkspaceViewModel", "Guide"),
    QT_TRANSLATE_NOOP("WorkspaceViewModel", "Device Guide"),
    QT_TRANSLATE_NOOP("WorkspaceViewModel", "Detected OEM setup and recovery instructions."),
    QT_TRANSLATE_NOOP("WorkspaceViewModel", "Detect manufacturer"),
    QT_TRANSLATE_NOOP("WorkspaceViewModel", "Detection"),
    QT_TRANSLATE_NOOP("WorkspaceViewModel", "Open OEM guide"),
    QT_TRANSLATE_NOOP("WorkspaceViewModel", "Setup"),
    QT_TRANSLATE_NOOP("WorkspaceViewModel", "Wireless setup"),
    QT_TRANSLATE_NOOP("WorkspaceViewModel", "Connection diagnostics"),
    QT_TRANSLATE_NOOP("WorkspaceViewModel", "Available"),
};
} // namespace

WorkspaceViewModel::WorkspaceViewModel(QObject* parent)
    : QObject(parent), m_modules(createModules()), m_activeFilter(tr("All")),
      m_statusText(tr("Ready"))
{
}

QVariantList WorkspaceViewModel::modules() const
{
    QVariantList result;
    for (const auto& module : m_modules)
    {
        result.append(QVariantMap{{QStringLiteral("key"), module.key},
                                  {QStringLiteral("shortTitle"), module.shortTitle},
                                  {QStringLiteral("title"), module.title}});
    }
    return result;
}

QString WorkspaceViewModel::currentModuleKey() const
{
    return currentModule().key;
}
QString WorkspaceViewModel::title() const
{
    return currentModule().title;
}
QString WorkspaceViewModel::description() const
{
    return currentModule().description;
}
QString WorkspaceViewModel::searchText() const
{
    return m_searchText;
}
QString WorkspaceViewModel::activeFilter() const
{
    return m_activeFilter;
}
bool WorkspaceViewModel::busy() const noexcept
{
    return m_busy;
}
QString WorkspaceViewModel::errorMessage() const
{
    return m_errorMessage;
}
QString WorkspaceViewModel::statusText() const
{
    return m_statusText;
}
QString WorkspaceViewModel::notification() const
{
    return m_notification;
}
bool WorkspaceViewModel::canUndo() const noexcept
{
    return m_canUndo;
}

QVariantList WorkspaceViewModel::features() const
{
    QVariantList result;
    for (const auto& item : currentModule().features)
    {
        const bool matchesSearch = m_searchText.isEmpty() ||
                                   item.title.contains(m_searchText, Qt::CaseInsensitive) ||
                                   item.category.contains(m_searchText, Qt::CaseInsensitive);
        const bool matchesFilter = m_activeFilter == tr("All") || item.category == m_activeFilter;
        if (!matchesSearch || !matchesFilter)
        {
            continue;
        }
        result.append(QVariantMap{{QStringLiteral("id"), item.id},
                                  {QStringLiteral("title"), item.title},
                                  {QStringLiteral("category"), item.category}});
    }
    return result;
}

QStringList WorkspaceViewModel::filters() const
{
    QStringList result{tr("All")};
    for (const auto& item : currentModule().features)
    {
        if (!result.contains(item.category))
        {
            result.append(item.category);
        }
    }
    return result;
}

bool WorkspaceViewModel::empty() const
{
    return features().isEmpty();
}

void WorkspaceViewModel::selectModule(const QString& key)
{
    const auto iterator = std::find_if(m_modules.cbegin(), m_modules.cend(),
                                       [&key](const Module& item) { return item.key == key; });
    if (iterator == m_modules.cend())
    {
        m_errorMessage = tr("The requested workspace module is not registered.");
        emit errorChanged();
        return;
    }
    const qsizetype nextIndex = std::distance(m_modules.cbegin(), iterator);
    if (nextIndex == m_currentIndex)
    {
        return;
    }
    m_previousModuleKey = currentModuleKey();
    m_currentIndex = nextIndex;
    m_searchText.clear();
    m_activeFilter = tr("All");
    m_errorMessage.clear();
    m_statusText = tr("%1 workspace opened").arg(title());
    m_canUndo = true;
    emit currentModuleChanged();
    emit searchTextChanged();
    emit contentChanged();
    emit errorChanged();
    emit statusChanged();
    emit canUndoChanged();
}

void WorkspaceViewModel::refresh()
{
    if (m_busy)
    {
        return;
    }
    setBusy(true);
    m_errorMessage.clear();
    emit errorChanged();
    QTimer::singleShot(150, this, [this]() {
        setBusy(false);
        m_statusText = tr("%1 capabilities refreshed").arg(title());
        setNotification(m_statusText);
        emit statusChanged();
        emit contentChanged();
    });
}

void WorkspaceViewModel::activateFeature(const QString& featureId)
{
    const auto& items = currentModule().features;
    const auto iterator =
        std::find_if(items.cbegin(), items.cend(),
                     [&featureId](const Feature& item) { return item.id == featureId; });
    if (iterator == items.cend())
    {
        m_errorMessage = tr("The selected capability is not registered.");
        emit errorChanged();
        return;
    }
    m_errorMessage.clear();
    m_statusText = tr("Running %1").arg(iterator->title);
    emit errorChanged();
    emit statusChanged();
    emit commandRequested(iterator->command);
    setNotification(tr("%1 command started").arg(iterator->title));
}

void WorkspaceViewModel::clearNotification()
{
    setNotification({});
}

void WorkspaceViewModel::clearError()
{
    if (m_errorMessage.isEmpty())
    {
        return;
    }
    m_errorMessage.clear();
    emit errorChanged();
}

void WorkspaceViewModel::undo()
{
    if (!m_canUndo || m_previousModuleKey.isEmpty())
    {
        return;
    }
    const QString target = m_previousModuleKey;
    m_canUndo = false;
    emit canUndoChanged();
    selectModule(target);
    m_canUndo = false;
    emit canUndoChanged();
}

void WorkspaceViewModel::retranslate()
{
    const QString key = currentModuleKey();
    m_modules = createModules();
    const auto iterator = std::find_if(m_modules.cbegin(), m_modules.cend(),
                                       [&key](const Module& item) { return item.key == key; });
    m_currentIndex = iterator == m_modules.cend() ? 0 : std::distance(m_modules.cbegin(), iterator);
    m_activeFilter = tr("All");
    emit currentModuleChanged();
    emit contentChanged();
}

void WorkspaceViewModel::setSearchText(const QString& text)
{
    if (m_searchText == text)
    {
        return;
    }
    m_searchText = text;
    emit searchTextChanged();
    emit contentChanged();
}

void WorkspaceViewModel::setActiveFilter(const QString& filter)
{
    if (m_activeFilter == filter || !filters().contains(filter))
    {
        return;
    }
    m_activeFilter = filter;
    emit contentChanged();
}

const WorkspaceViewModel::Module& WorkspaceViewModel::currentModule() const
{
    return m_modules.at(m_currentIndex);
}

void WorkspaceViewModel::setBusy(bool value)
{
    if (m_busy == value)
    {
        return;
    }
    m_busy = value;
    emit busyChanged();
}

void WorkspaceViewModel::setNotification(const QString& message)
{
    if (m_notification == message)
    {
        return;
    }
    m_notification = message;
    emit notificationChanged();
}

// The catalog is centralized so presentation controls cannot invent commands.
// NOLINTNEXTLINE(readability-function-cognitive-complexity)
QList<WorkspaceViewModel::Module> WorkspaceViewModel::createModules()
{
    const auto feature = [](const char* id, const char* title, const char* category,
                            const char* command) {
        return Feature{QString::fromLatin1(id),
                       QCoreApplication::translate("WorkspaceViewModel", title),
                       QCoreApplication::translate("WorkspaceViewModel", category),
                       QString::fromLatin1(command)};
    };
    const auto module = [](const char* key, const char* shortTitle, const char* title,
                           const char* description, QList<Feature> features) {
        return Module{
            QString::fromLatin1(key), QCoreApplication::translate("WorkspaceViewModel", shortTitle),
            QCoreApplication::translate("WorkspaceViewModel", title),
            QCoreApplication::translate("WorkspaceViewModel", description), std::move(features)};
    };
    return {
        module("dashboard", "Home", "Dashboard",
               "Live device overview and verified connection health.",
               {feature("scan", "Scan devices", "Quick actions", "scan-devices"),
                feature("health", "Device health", "Diagnostics", "scan-devices")}),
        module("devices", "Devices", "Devices",
               "Search, classify and operate verified Android transports.",
               {feature("scan", "Refresh device list", "Connection", "scan-devices"),
                feature("restart-adb", "Restart ADB", "Connection", "restart-adb"),
                feature("usb", "USB devices", "Transport", "scan-devices"),
                feature("wireless", "Wireless devices", "Transport", "scan-devices"),
                feature("fastboot", "Fastboot devices", "Boot mode", "scan-devices"),
                feature("recovery", "Recovery devices", "Boot mode", "scan-devices")}),
        module("mirror", "Mirror", "Screen Mirroring",
               "Isolated scrcpy sessions with verified device controls.", {}),
        module("guide", "Guide", "Device Guide", "Detected OEM setup and recovery instructions.",
               {feature("detect", "Detect manufacturer", "Detection", "scan-devices"),
                feature("oem", "Open OEM guide", "Setup", "oem-guide"),
                feature("wireless", "Wireless setup", "Setup", "wireless-guide"),
                feature("diagnostics", "Connection diagnostics", "Diagnostics", "scan-devices")}),
    };
}

} // namespace adb_studio
