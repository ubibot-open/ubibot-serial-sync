#pragma once

#include "core/log_manager.h"

#include <QAbstractListModel>
#include <QQmlEngine>

// QML-facing view over LogManager's scrollback. One row per LogEntry; the
// `time`/`text` roles reformat live when hexMode/showTimestamp change
// (mirrors the old DataMonitorView's ascii/hex + timestamp toggles) without
// touching LogManager itself.
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
    enum Roles { TimeRole = Qt::UserRole + 1, DirRole, TextRole, ColorRole };

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
