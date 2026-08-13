#include "models/log_list_model.h"

namespace {
QString dirTag(LogKind k) {
    switch (k) {
    case LogKind::Tx: return QStringLiteral("TX");
    case LogKind::Rx: return QStringLiteral("RX");
    case LogKind::Sys: return QStringLiteral("SYS");
    case LogKind::Err: return QStringLiteral("ERR");
    }
    return QString();
}
}  // namespace

LogListModel::LogListModel(LogManager *manager, QObject *parent)
    : QAbstractListModel(parent), manager_(manager) {
    // See the header comment on bulkFlushTimer_/pendingBulkRebuild_ -- a
    // short debounce so a whole run of back-to-back bulkChanged()s (a
    // device dumping a huge stored log, arriving as many separate
    // dataReceived chunks in quick succession) collapses into exactly one
    // rebuild, timed from whenever they actually stop rather than firing
    // once per chunk.
    bulkFlushTimer_ = new QTimer(this);
    bulkFlushTimer_->setSingleShot(true);
    bulkFlushTimer_->setInterval(50);
    connect(bulkFlushTimer_, &QTimer::timeout, this, [this] {
        pendingBulkRebuild_ = false;
        beginResetModel();
        endResetModel();
        emit lineCountChanged();
        emit rebuildNeeded();
    });

    connect(manager_, &LogManager::entriesAboutToBeEvicted, this, [this](int count) {
        if (pendingBulkRebuild_) return;  // an eventual rebuild will already reflect this
        beginRemoveRows(QModelIndex(), 0, count - 1);
        // Fires before LogManager actually erases entries()[0..count-1], so
        // they're still the about-to-be-evicted entries here -- computed
        // fresh from current hexMode_/showTimestamp_ rather than cached
        // from when each was appended, which is safe only because any
        // toggle of either triggers rebuildNeeded() (full re-render), so
        // the document is always re-synced to the current mode before this
        // can run again. Summed into one docLength (rather than emitting
        // lineEvicted() once per evicted line) so the view drops all of
        // them with a single contentEdit.remove() call -- QTextDocument's
        // remove() is not O(1) in document size, and a burst evicting
        // thousands of lines at once is exactly the case this needs to
        // stay cheap for.
        const auto &entries = manager_->entries();
        const int n = qMin(count, entries.size());
        int totalLength = 0;
        for (int i = 0; i < n; ++i) totalLength += linePlainText(entries.at(i)).length() + 1;  // +1 per "\n"
        emit lineEvicted(totalLength);
    });
    connect(manager_, &LogManager::entriesEvicted, this, [this] {
        if (pendingBulkRebuild_) return;
        endRemoveRows();
    });
    connect(manager_, &LogManager::entriesAppended, this, [this](int count) {
        if (pendingBulkRebuild_) return;  // an eventual rebuild will already reflect this
        const auto &entries = manager_->entries();
        const int first = entries.size() - count;
        const int last = entries.size() - 1;
        beginInsertRows(QModelIndex(), first, last);
        endInsertRows();
        emit lineCountChanged();
        // One concatenated string and one lineAppended() for the whole
        // batch, not one per line -- see LogManager::entriesAppended.
        QString text;
        for (int i = first; i <= last; ++i) {
            text += linePlainText(entries.at(i));
            text += QLatin1Char('\n');
        }
        emit lineAppended(text);
    });
    // Doesn't rebuild immediately -- see bulkFlushTimer_'s header comment.
    // entries() already reflects the final, settled state whenever the
    // debounced rebuild actually runs, regardless of how many bulkChanged()
    // (or ignored entriesAppended/entriesAboutToBeEvicted, above) fired
    // while it was pending.
    connect(manager_, &LogManager::bulkChanged, this, [this] {
        pendingBulkRebuild_ = true;
        bulkFlushTimer_->start();
    });
    connect(manager_, &LogManager::cleared, this, [this] {
        // Supersedes any rebuild still pending from a just-finished burst --
        // that would otherwise fire moments later and redraw whatever the
        // burst had left behind right after the user cleared it.
        bulkFlushTimer_->stop();
        pendingBulkRebuild_ = false;
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
    }
    return {};
}

QHash<int, QByteArray> LogListModel::roleNames() const {
    return {
        {TimeRole, "time"},
        {DirRole, "dir"},
        {TextRole, "text"},
    };
}

void LogListModel::setHexMode(bool hex) {
    if (hexMode_ == hex) return;
    hexMode_ = hex;
    emit hexModeChanged();
    if (!manager_->entries().isEmpty()) emit dataChanged(index(0), index(manager_->entries().size() - 1), {TextRole});
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

int LogListModel::lineCount() const { return manager_->entries().size(); }
qint64 LogListModel::totalLineCount() const { return manager_->totalLineCount(); }
qint64 LogListModel::rxBytes() const { return manager_->rxBytes(); }
qint64 LogListModel::txBytes() const { return manager_->txBytes(); }

void LogListModel::clear() { manager_->clear(); }

// Builds one line's displayable plain text: a fixed-shape prefix (optional
// timestamp + the dir tag, padded to 3 characters so every line's content
// starts at the same column regardless of TX/RX vs SYS/ERR) plus the
// content with ANSI escape codes and other control-byte noise stripped --
// see ansi_text.h.
QString LogListModel::linePlainText(const LogEntry &e) const {
    QString prefix;
    if (showTimestamp_) {
        prefix += e.time.toString(QStringLiteral("HH:mm:ss"));
        prefix += QStringLiteral("  ");
    }
    prefix += dirTag(e.kind).leftJustified(3);
    prefix += QStringLiteral("  ");

    const QString raw = (hexMode_ && e.kind == LogKind::Rx) ? e.hexText() : e.asciiText();
    return prefix + AnsiText::stripToPlain(raw);
}

QString LogListModel::fullPlainDump() const {
    QString text;
    for (const LogEntry &e : manager_->entries()) {
        text += linePlainText(e);
        text += QLatin1Char('\n');
    }
    return text;
}
