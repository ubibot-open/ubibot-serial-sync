// Copyright (C) 2026 UbiBot Co., Ltd.
// SPDX-License-Identifier: LGPL-3.0-only

#include "ui/data_monitor_view.h"

#include "app/app_controller.h"
#include "ui/ui_common.h"

#include <QApplication>
#include <QClipboard>
#include <QContextMenuEvent>
#include <QEvent>
#include <QHBoxLayout>
#include <QRegularExpression>
#include <QScrollBar>
#include <QTextBlock>
#include <QTextDocumentFragment>
#include <QVBoxLayout>

#include <QLight/FluentIcon.h>
#include <QLight/Menu.h>
#include <QLight/ScrollBar.h>
#include <QLight/Theme.h>

namespace ui {

namespace {

// "HH:mm:ss  " (optional) + direction column, as LogListModel::linePlainText() writes it.
const QRegularExpression &linePrefix() {
    static const QRegularExpression re(QStringLiteral("^(?:\\d{2}:\\d{2}:\\d{2}  )?(TX |RX |SYS|ERR)  "));
    return re;
}

}  // namespace

// --- LogTextView ---------------------------------------------------------

LogTextView::LogTextView(QWidget *parent) : QPlainTextEdit(parent) {
    setReadOnly(true);
    setUndoRedoEnabled(false);
    setFrameShape(QFrame::NoFrame);
    setTextInteractionFlags(Qt::TextSelectableByMouse | Qt::TextSelectableByKeyboard);
    viewport()->setAutoFillBackground(false);
    setAutoFillBackground(false);
    setStyleSheet(QStringLiteral("QPlainTextEdit { background: transparent; }"));
    document()->setDocumentMargin(0);
    QLight::ScrollBar::overlayIn(this);
    onThemeChanged(this, [this] { applyColors(); });
}

void LogTextView::applyColors() {
    const AppColors &c = colors();
    const QLight::Palette &p = QLight::ThemeManager::instance().palette();
    txFormat_.setForeground(c.logTx);
    rxFormat_.setForeground(c.logRx);
    sysFormat_.setForeground(c.logSys);
    errFormat_.setForeground(c.logErr);

    QPalette pal = palette();
    pal.setColor(QPalette::Text, p.textPrimary);
    pal.setColor(QPalette::Highlight, p.accent);
    pal.setColor(QPalette::HighlightedText, p.textOnAccent);
    setPalette(pal);

    // recolour what is already there
    QTextCursor cursor(document());
    cursor.beginEditBlock();
    for (QTextBlock b = document()->begin(); b.isValid(); b = b.next()) {
        cursor.setPosition(b.position());
        cursor.setPosition(b.position() + b.length() - 1, QTextCursor::KeepAnchor);
        cursor.setCharFormat(formatForLine(b.text()));
    }
    cursor.endEditBlock();
}

QTextCharFormat LogTextView::formatForLine(const QString &line) const {
    const QRegularExpressionMatch m = linePrefix().match(line);
    if (m.hasMatch()) {
        const QString tag = m.captured(1);
        if (tag.startsWith(QLatin1String("TX"))) return txFormat_;
        if (tag == QLatin1String("SYS")) return sysFormat_;
        if (tag == QLatin1String("ERR")) return errFormat_;
    }
    return rxFormat_;
}

void LogTextView::insertLines(QTextCursor &cursor, const QString &text) {
    // `text` is one or more '\n'-terminated lines
    int start = 0;
    while (start < text.size()) {
        int end = text.indexOf(QLatin1Char('\n'), start);
        if (end < 0) end = text.size();
        const QString line = text.mid(start, end - start);
        if (!document()->isEmpty() || cursor.position() > 0) cursor.insertBlock();
        cursor.insertText(line, formatForLine(line));
        start = end + 1;
    }
}

void LogTextView::appendLines(const QString &text) {
    QScrollBar *bar = verticalScrollBar();
    const bool atBottom = bar->value() >= bar->maximum() - 4;
    QTextCursor cursor(document());
    cursor.movePosition(QTextCursor::End);
    cursor.beginEditBlock();
    insertLines(cursor, text);
    cursor.endEditBlock();
    if (!paused_ && atBottom) bar->setValue(bar->maximum());
}

void LogTextView::removeLeading(int length) {
    // LogListModel counts each line plus its '\n'; in the document that is
    // the block text plus the block separator, i.e. the same length.
    QTextCursor cursor(document());
    cursor.setPosition(0);
    cursor.setPosition(qMin(length, document()->characterCount() - 1), QTextCursor::KeepAnchor);
    cursor.removeSelectedText();
}

void LogTextView::setAllText(const QString &text) {
    clear();
    if (text.isEmpty()) return;
    QTextCursor cursor(document());
    cursor.beginEditBlock();
    QString body = text;
    if (body.endsWith(QLatin1Char('\n'))) body.chop(1);
    insertLines(cursor, body + QLatin1Char('\n'));
    cursor.endEditBlock();
    if (!paused_) verticalScrollBar()->setValue(verticalScrollBar()->maximum());
}

QString LogTextView::textForCopy() const {
    const QTextCursor c = textCursor();
    if (!c.hasSelection()) return toPlainText();
    QString text = c.selection().toPlainText();
    static const QRegularExpression prefix(
        QStringLiteral("^(?:\\d{2}:\\d{2}:\\d{2}  )?(?:(?:TX|RX)   |(?:SYS|ERR)  )"),
        QRegularExpression::MultilineOption);
    return text.replace(prefix, QString());
}

void LogTextView::contextMenuEvent(QContextMenuEvent *e) { emit contextMenuWanted(e->globalPos()); }

// --- DataMonitorView -----------------------------------------------------

DataMonitorView::DataMonitorView(AppController *controller, QWidget *parent)
    : QWidget(parent), controller_(controller) {
    auto *root = new QVBoxLayout(this);
    root->setContentsMargins(0, 0, 0, 0);
    root->setSpacing(0);

    auto *header = new QWidget(this);
    header->setFixedHeight(44);
    auto *headerRow = new QHBoxLayout(header);
    headerRow->setContentsMargins(16, 0, 16, 0);
    title_ = new ThemedLabel(ThemedLabel::Primary, header);
    title_->setEmphasis(true, 1);
    counters_ = new ThemedLabel(ThemedLabel::Secondary, header);
    headerRow->addWidget(title_);
    headerRow->addStretch(1);
    headerRow->addWidget(counters_);
    root->addWidget(header);
    root->addWidget(new Divider(Qt::Horizontal, this));

    view_ = new LogTextView(this);
    auto *viewHost = new QWidget(this);
    auto *viewLayout = new QVBoxLayout(viewHost);
    viewLayout->setContentsMargins(16, 12, 6, 12);
    viewLayout->addWidget(view_);
    root->addWidget(viewHost, 1);

    // empty-state hint, centred over the log viewport
    auto *hintLayout = new QVBoxLayout(view_->viewport());
    emptyHint_ = new ThemedLabel(ThemedLabel::Secondary, view_->viewport());
    emptyHint_->setAlignment(Qt::AlignCenter);
    emptyHint_->setAttribute(Qt::WA_TransparentForMouseEvents);
    hintLayout->addWidget(emptyHint_, 0, Qt::AlignCenter);

    bottomHost_ = new QWidget(this);
    auto *bottomLayout = new QVBoxLayout(bottomHost_);
    bottomLayout->setContentsMargins(16, 0, 16, 16);
    bottomHost_->hide();
    root->addWidget(bottomHost_);

    LogListModel *log = controller_->logModel();
    connect(log, &LogListModel::lineAppended, this, [this](const QString &text) {
        view_->appendLines(text);
        updateEmptyHint();
    });
    connect(log, &LogListModel::lineEvicted, view_, &LogTextView::removeLeading);
    connect(log, &LogListModel::rebuildNeeded, this, [this] {
        view_->setAllText(controller_->logModel()->fullPlainDump());
        updateEmptyHint();
    });
    connect(log, &LogListModel::countersChanged, this, &DataMonitorView::updateCounters);
    connect(log, &LogListModel::lineCountChanged, this, &DataMonitorView::updateCounters);
    connect(controller_, &AppController::logFontChanged, this, &DataMonitorView::applyLogFont);
    connect(view_, &LogTextView::contextMenuWanted, this, &DataMonitorView::showContextMenu);

    applyLogFont();
    view_->setAllText(log->fullPlainDump());
    retranslateUi();
    updateEmptyHint();
}

void DataMonitorView::setPaused(bool paused) {
    if (paused_ == paused) return;
    paused_ = paused;
    view_->setPaused(paused);
    if (!paused) view_->verticalScrollBar()->setValue(view_->verticalScrollBar()->maximum());
    emit pausedChanged(paused);
}

void DataMonitorView::setWrapLines(bool wrap) {
    view_->setLineWrapMode(wrap ? QPlainTextEdit::WidgetWidth : QPlainTextEdit::NoWrap);
}

void DataMonitorView::setBottomWidget(QWidget *w) {
    bottomHost_->layout()->addWidget(w);
    bottomHost_->setVisible(true);
}

void DataMonitorView::changeEvent(QEvent *e) {
    QWidget::changeEvent(e);
    if (e->type() == QEvent::LanguageChange) retranslateUi();
}

void DataMonitorView::resizeEvent(QResizeEvent *e) { QWidget::resizeEvent(e); }

void DataMonitorView::retranslateUi() {
    title_->setText(tr("Data monitor"));
    emptyHint_->setText(tr("Data sent and received appears here once a port is open"));
    updateCounters();
}

void DataMonitorView::updateCounters() {
    LogListModel *log = controller_->logModel();
    counters_->setText(tr("%1 lines").arg(log->totalLineCount()) + QStringLiteral("    ") +
                       tr("Rx %1").arg(formatBytes(log->rxBytes())) + QStringLiteral("    ") +
                       tr("Tx %1").arg(formatBytes(log->txBytes())));
}

void DataMonitorView::updateEmptyHint() { emptyHint_->setVisible(view_->document()->isEmpty()); }

void DataMonitorView::applyLogFont() {
    view_->setFont(monoFont(controller_->logFontSize(), controller_->logFontFamily()));
}

void DataMonitorView::showContextMenu(const QPoint &globalPos) {
    QLight::Menu menu(this);
    QAction *copy = menu.addAction(QLight::FluentIcon(QLight::Icon::Copy).icon(), tr("Copy"));
    menu.addSeparator();
    QAction *clear = menu.addAction(QLight::FluentIcon(QLight::Icon::Delete).icon(), tr("Clear log"));
    QAction *save = menu.addAction(QLight::FluentIcon(QLight::Icon::Save).icon(), tr("Save log…"));
    QAction *chosen = menu.exec(globalPos);
    if (chosen == copy) {
        QApplication::clipboard()->setText(view_->textForCopy());
    } else if (chosen == clear) {
        controller_->logModel()->clear();
    } else if (chosen == save) {
        emit saveLogRequested();
    }
}

}  // namespace ui
