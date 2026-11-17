// Copyright (C) 2026 UbiBot Co., Ltd.
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once

#include <QPlainTextEdit>
#include <QTextCharFormat>
#include <QWidget>

class AppController;
class QLabel;

namespace ui {

class ThemedLabel;

// Read-only log text. Lines are appended incrementally from
// LogListModel::lineAppended (and trimmed from the top on lineEvicted), each
// coloured by its direction column: TX accent, RX normal, SYS muted, ERR red.
class LogTextView : public QPlainTextEdit {
    Q_OBJECT
public:
    explicit LogTextView(QWidget *parent = nullptr);

    void appendLines(const QString &text);
    void removeLeading(int length);
    void setAllText(const QString &text);
    void setPaused(bool paused) { paused_ = paused; }

    // Selected text without the time/direction columns, or the whole log when
    // nothing is selected.
    QString textForCopy() const;

signals:
    void contextMenuWanted(const QPoint &globalPos);

protected:
    void contextMenuEvent(QContextMenuEvent *e) override;

private:
    QTextCharFormat formatForLine(const QString &line) const;
    void insertLines(QTextCursor &cursor, const QString &text);
    void applyColors();

    bool paused_ = false;
    QTextCharFormat txFormat_, rxFormat_, sysFormat_, errFormat_;
};

// The "Data monitor" card body: header with counters, the log view and an
// empty-state hint.
class DataMonitorView : public QWidget {
    Q_OBJECT
public:
    explicit DataMonitorView(AppController *controller, QWidget *parent = nullptr);

    bool paused() const { return paused_; }
    void setPaused(bool paused);
    void setWrapLines(bool wrap);

    // Container the params panel is inserted into (below the log, inside the card).
    void setBottomWidget(QWidget *w);

signals:
    void pausedChanged(bool paused);
    void saveLogRequested();

protected:
    void changeEvent(QEvent *e) override;
    void resizeEvent(QResizeEvent *e) override;

private:
    void retranslateUi();
    void updateCounters();
    void updateEmptyHint();
    void applyLogFont();
    void showContextMenu(const QPoint &globalPos);

    AppController *controller_;
    ThemedLabel *title_;
    ThemedLabel *counters_;
    LogTextView *view_;
    ThemedLabel *emptyHint_;
    QWidget *bottomHost_;
    bool paused_ = false;
};

}  // namespace ui
