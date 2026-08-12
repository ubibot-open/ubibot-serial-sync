#include "models/log_list_model.h"

namespace {
// Base/"reset" color for each line kind. Two variants, matching whichever
// of Theme.qml's two palettes the data monitor's own background currently
// uses (see LogListModel::setDarkPalette) -- also doubles as the color
// ANSI reset codes (SGR 0/39) return to when a device's colored log output
// resets mid-line.
QString colorForKind(LogKind k, bool dark) {
    if (dark) {
        switch (k) {
        case LogKind::Tx: return QStringLiteral("#e0a458");
        case LogKind::Rx: return QStringLiteral("#d8dce0");
        case LogKind::Sys: return QStringLiteral("#7fa8c9");
        case LogKind::Err: return QStringLiteral("#f07178");
        }
        return QStringLiteral("#d8dce0");
    }
    switch (k) {
    case LogKind::Tx: return QStringLiteral("#986801");
    case LogKind::Rx: return QStringLiteral("#2b2d31");
    case LogKind::Sys: return QStringLiteral("#3a6ea8");
    case LogKind::Err: return QStringLiteral("#c5372b");
    }
    return QStringLiteral("#2b2d31");
}

QString dirTag(LogKind k) {
    switch (k) {
    case LogKind::Tx: return QStringLiteral("TX");
    case LogKind::Rx: return QStringLiteral("RX");
    case LogKind::Sys: return QStringLiteral("SYS");
    case LogKind::Err: return QStringLiteral("ERR");
    }
    return QString();
}

// Mirrors Theme.consoleMuted/Theme.textMuted in Theme.qml (dark/light
// respectively) -- the timestamp color baked into each line's rich text
// here has to match by hand since this is plain C++ with no access to the
// QML singleton.
QString consoleMutedColor(bool dark) { return dark ? QStringLiteral("#6b7280") : QStringLiteral("#7a7a7d"); }
}  // namespace

LogListModel::LogListModel(LogManager *manager, QObject *parent)
    : QAbstractListModel(parent), manager_(manager) {
    connect(manager_, &LogManager::rowAboutToBeRemoved, this, [this] {
        beginRemoveRows(QModelIndex(), 0, 0);
        // Fires before LogManager actually erases entries()[0], so it's
        // still the about-to-be-evicted entry here -- computed fresh from
        // current hexMode_/showTimestamp_ rather than cached from when it
        // was appended, which is safe only because any toggle of either
        // triggers rebuildNeeded() (full re-render), so the document is
        // always re-synced to the current mode before this can run again.
        if (!manager_->entries().isEmpty())
            emit lineEvicted(lineHtml(manager_->entries().first()).length + 1);
    });
    connect(manager_, &LogManager::rowRemoved, this, [this] { endRemoveRows(); });
    connect(manager_, &LogManager::entryAdded, this, [this](const LogEntry &e) {
        const int row = manager_->entries().size() - 1;
        beginInsertRows(QModelIndex(), row, row);
        endInsertRows();
        emit lineCountChanged();
        emit lineAppended(lineHtml(e).html + QStringLiteral("<br/>"));
    });
    connect(manager_, &LogManager::cleared, this, [this] {
        beginResetModel();
        endResetModel();
        emit lineCountChanged();
        emit rebuildNeeded();
    });
    connect(manager_, &LogManager::countersChanged, this, &LogListModel::countersChanged);
}

int LogListModel::rowCount(const QModelIndex &parent) const {
    return parent.isValid() ? 0 : manager_->entries().size();
}

QVariant LogListModel::data(const QModelIndex &index, int role) const {
    if (!index.isValid() || index.row() < 0 || index.row() >= manager_->entries().size()) return {};
    const LogEntry &e = manager_->entries().at(index.row());
    switch (role) {
    case TimeRole: return showTimestamp_ ? e.time.toString(QStringLiteral("HH:mm:ss")) : QString();
    case DirRole: return dirTag(e.kind);
    case TextRole: return (hexMode_ && e.kind == LogKind::Rx) ? e.hexText() : e.asciiText();
    case ColorRole: return colorForKind(e.kind, darkPalette_);
    case HtmlRole: {
        const QString plain = (hexMode_ && e.kind == LogKind::Rx) ? e.hexText() : e.asciiText();
        return AnsiText::toRichText(plain, colorForKind(e.kind, darkPalette_), darkPalette_).html;
    }
    }
    return {};
}

QHash<int, QByteArray> LogListModel::roleNames() const {
    return {
        {TimeRole, "time"},
        {DirRole, "dir"},
        {TextRole, "text"},
        {ColorRole, "color"},
        {HtmlRole, "html"},
    };
}

void LogListModel::setHexMode(bool hex) {
    if (hexMode_ == hex) return;
    hexMode_ = hex;
    emit hexModeChanged();
    if (!manager_->entries().isEmpty())
        emit dataChanged(index(0), index(manager_->entries().size() - 1), {TextRole, HtmlRole});
    emit rebuildNeeded();
}

void LogListModel::setShowTimestamp(bool show) {
    if (showTimestamp_ == show) return;
    showTimestamp_ = show;
    emit showTimestampChanged();
    if (!manager_->entries().isEmpty())
        emit dataChanged(index(0), index(manager_->entries().size() - 1), {TimeRole});
    emit rebuildNeeded();
}

void LogListModel::setDarkPalette(bool dark) {
    if (darkPalette_ == dark) return;
    darkPalette_ = dark;
    if (!manager_->entries().isEmpty())
        emit dataChanged(index(0), index(manager_->entries().size() - 1), {ColorRole, HtmlRole});
    emit rebuildNeeded();
}

int LogListModel::lineCount() const { return manager_->entries().size(); }
qint64 LogListModel::rxBytes() const { return manager_->rxBytes(); }
qint64 LogListModel::txBytes() const { return manager_->txBytes(); }

void LogListModel::clear() { manager_->clear(); }

// Builds one line's rich text AND its rendered length together (rather
// than as two separately-maintained computations, as an earlier version of
// this did) specifically so they can't drift apart: the prefix (timestamp
// + dir tag) is fixed-width plain ASCII, so its length is just arithmetic,
// but the content half runs through AnsiText::toRichText, whose rendered
// length depends on what ANSI/control bytes happened to be in this
// particular line -- reusing its Result.length here, rather than
// re-deriving it from e.asciiText().length(), is what keeps this accurate
// (ANSI escape codes and dropped control bytes render as zero characters;
// raw string length would count them, and lineEvicted() would then tell
// the view to remove the wrong number of characters).
AnsiText::Result LogListModel::lineHtml(const LogEntry &e) const {
    const QString baseColor = colorForKind(e.kind, darkPalette_);
    QString prefix;
    int prefixLength = 0;
    if (showTimestamp_) {
        prefix += QStringLiteral("<span style=\"color:%1\">%2</span>&nbsp;&nbsp;")
                      .arg(consoleMutedColor(darkPalette_), e.time.toString(QStringLiteral("HH:mm:ss")));
        prefixLength += 8 + 2;  // "HH:mm:ss" + 2 gap chars
    }
    // Padded to a fixed 3 characters ("TX " / "SYS") so every line's
    // content starts at the same column despite TX/RX vs SYS/ERR being
    // different lengths -- plain spaces would get collapsed by the rich
    // text renderer's HTML whitespace rules, hence &nbsp; instead.
    QString dirPadded = dirTag(e.kind).leftJustified(3);
    dirPadded.replace(QLatin1Char(' '), QStringLiteral("&nbsp;"));
    prefix += QStringLiteral("<span style=\"color:%1\">%2</span>&nbsp;&nbsp;").arg(baseColor, dirPadded);
    prefixLength += 3 + 2;  // dir tag padded to 3 + 2 gap chars

    const QString text = (hexMode_ && e.kind == LogKind::Rx) ? e.hexText() : e.asciiText();
    const AnsiText::Result content = AnsiText::toRichText(text, baseColor, darkPalette_);
    return {prefix + content.html, prefixLength + content.length};
}

QString LogListModel::fullHtmlDump() const {
    QString html;
    for (const LogEntry &e : manager_->entries()) {
        html += lineHtml(e).html;
        html += QStringLiteral("<br/>");
    }
    return html;
}
