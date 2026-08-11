#pragma once

#include "core/log_manager.h"

#include <QAbstractListModel>
#include <QQmlEngine>

// QML-facing view over LogManager's scrollback. One row per LogEntry; the
// `time`/`text`/`html` roles reformat live when hexMode/showTimestamp change
// (mirrors the old DataMonitorView's ascii/hex + timestamp toggles) without
// touching LogManager itself. `html` is `text` with any ANSI SGR color
// codes the device sent turned into <span> runs (see ansi_text.h) -- what
// DataMonitorView actually displays; `text` remains available verbatim for
// anything that wants the plain string.
class LogListModel : public QAbstractListModel {
    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("Created by AppController")

    Q_PROPERTY(bool hexMode READ hexMode WRITE setHexMode NOTIFY hexModeChanged)
    Q_PROPERTY(bool showTimestamp READ showTimestamp WRITE setShowTimestamp NOTIFY showTimestampChanged)
    Q_PROPERTY(int lineCount READ lineCount NOTIFY lineCountChanged)
    Q_PROPERTY(qint64 rxBytes READ rxBytes NOTIFY countersChanged)
    Q_PROPERTY(qint64 txBytes READ txBytes NOTIFY countersChanged)

public:
    enum Roles { TimeRole = Qt::UserRole + 1, DirRole, TextRole, ColorRole, HtmlRole };

    explicit LogListModel(LogManager *manager, QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

    bool hexMode() const { return hexMode_; }
    void setHexMode(bool hex);
    bool showTimestamp() const { return showTimestamp_; }
    void setShowTimestamp(bool show);
    int lineCount() const;
    qint64 rxBytes() const;
    qint64 txBytes() const;

    Q_INVOKABLE void clear();

    // Plain "HH:mm:ss  DIR  text" dump of every entry currently in scope,
    // joined with '\n' -- what DataMonitorView's context-menu "Copy" falls
    // back to when nothing is selected. Respects the same hexMode/
    // showTimestamp toggles as the live view, so it matches what's on screen.
    Q_INVOKABLE QString plainTextDump() const;

signals:
    void hexModeChanged();
    void showTimestampChanged();
    void lineCountChanged();
    void countersChanged();

private:
    LogManager *manager_;
    bool hexMode_ = false;
    bool showTimestamp_ = true;
};
