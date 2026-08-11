#pragma once

#include "core/log_entry.h"

#include <QWidget>

class QLabel;
class QTextEdit;
class LogManager;

// Right-hand "data monitor" pane: a scrolling, color-coded view of every
// TX/RX/SYS/ERR line plus a small header showing line count and byte
// counters. Purely a view over LogManager -- it never mutates it except via
// clear(), which delegates back to LogManager::clear().
class DataMonitorView : public QWidget {
    Q_OBJECT
public:
    explicit DataMonitorView(LogManager *logManager, QWidget *parent = nullptr);

    void setHexMode(bool hex);
    void setShowTimestamp(bool show);
    void setPaused(bool paused);
    bool isPaused() const { return paused_; }

public slots:
    void clearLog();

protected:
    void changeEvent(QEvent *event) override;

private:
    void buildUi();
    void retranslateUi();
    void appendEntry(const LogEntry &entry);
    void rebuildFromHistory();
    void updateCounters(qint64 rxBytes, qint64 txBytes);

    LogManager *logManager_;

    QLabel *monitorTitleLabel_;
    QLabel *lineCountLabel_;
    QLabel *rxLabel_;
    QLabel *txLabel_;
    QTextEdit *view_;

    bool hexMode_ = false;
    bool showTimestamp_ = true;
    bool paused_ = false;
    int lineCount_ = 0;
    qint64 rxBytes_ = 0;
    qint64 txBytes_ = 0;
};
