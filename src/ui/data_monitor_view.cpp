#include "ui/data_monitor_view.h"
#include "core/log_manager.h"
#include "ui/styles.h"

#include <QEvent>
#include <QHBoxLayout>
#include <QLabel>
#include <QScrollBar>
#include <QTextEdit>
#include <QVBoxLayout>

DataMonitorView::DataMonitorView(LogManager *logManager, QWidget *parent)
    : QWidget(parent), logManager_(logManager) {
    buildUi();
    retranslateUi();

    connect(logManager_, &LogManager::entryAdded, this, [this](const LogEntry &e) {
        ++lineCount_;
        lineCountLabel_->setText(tr("%1 lines").arg(lineCount_));
        if (!paused_) appendEntry(e);
    });
    connect(logManager_, &LogManager::cleared, this, [this] {
        lineCount_ = 0;
        view_->clear();
        lineCountLabel_->setText(tr("%1 lines").arg(lineCount_));
    });
    connect(logManager_, &LogManager::countersChanged, this, &DataMonitorView::updateCounters);
}

void DataMonitorView::buildUi() {
    auto *root = new QVBoxLayout(this);
    root->setContentsMargins(0, 0, 0, 0);
    root->setSpacing(0);

    auto *header = new QWidget;
    header->setFixedHeight(30);
    header->setStyleSheet(QStringLiteral("border-bottom: 1px solid #c9c9ca;"));
    auto *headerLayout = new QHBoxLayout(header);
    headerLayout->setContentsMargins(14, 0, 14, 0);
    monitorTitleLabel_ = new QLabel;
    monitorTitleLabel_->setStyleSheet(
        QStringLiteral("font-size: 11px; letter-spacing: 1px; color: #7a7a7d;"));
    lineCountLabel_ = new QLabel;
    rxLabel_ = new QLabel;
    txLabel_ = new QLabel;
    for (QLabel *l : {lineCountLabel_, rxLabel_, txLabel_})
        l->setStyleSheet(QStringLiteral("font-family: Consolas, monospace; font-size: 11px; color: #7a7a7d;"));
    headerLayout->addWidget(monitorTitleLabel_);
    headerLayout->addStretch();
    headerLayout->addWidget(lineCountLabel_);
    headerLayout->addWidget(rxLabel_);
    headerLayout->addWidget(txLabel_);

    view_ = new QTextEdit;
    view_->setReadOnly(true);
    view_->setLineWrapMode(QTextEdit::NoWrap);
    view_->setStyleSheet(
        QStringLiteral("QTextEdit { background: #fafafa; font-family: Consolas, 'Cascadia Mono', monospace; "
                        "font-size: 12px; border: none; }"));

    root->addWidget(header);
    root->addWidget(view_, 1);
}

void DataMonitorView::appendEntry(const LogEntry &entry) {
    QString line;
    if (showTimestamp_) {
        line += QStringLiteral("<span style='color:#98989b'>%1</span> ")
                    .arg(entry.time.toString(QStringLiteral("HH:mm:ss")));
    }

    QString dirTag;
    switch (entry.kind) {
    case LogKind::Tx: dirTag = QStringLiteral("TX"); break;
    case LogKind::Rx: dirTag = QStringLiteral("RX"); break;
    case LogKind::Sys: dirTag = QStringLiteral("SYS"); break;
    case LogKind::Err: dirTag = QStringLiteral("ERR"); break;
    }
    const QColor tagColor = Styles::colorForLogKind(entry.kind);
    line += QStringLiteral("<span style='color:%1'>%2</span> ").arg(tagColor.name(), dirTag);

    const QString text = (hexMode_ && entry.kind == LogKind::Rx) ? entry.hexText() : entry.asciiText();
    line += QStringLiteral("<span style='color:%1'>%2</span>")
                .arg(Styles::colorForLogKind(entry.kind).name(), text.toHtmlEscaped());

    view_->append(line);
    QScrollBar *sb = view_->verticalScrollBar();
    sb->setValue(sb->maximum());
}

void DataMonitorView::rebuildFromHistory() {
    view_->clear();
    for (const LogEntry &e : logManager_->entries()) appendEntry(e);
}

void DataMonitorView::updateCounters(qint64 rxBytes, qint64 txBytes) {
    rxBytes_ = rxBytes;
    txBytes_ = txBytes;
    rxLabel_->setText(tr("Rx %1 B").arg(rxBytes_));
    txLabel_->setText(tr("Tx %1 B").arg(txBytes_));
}

void DataMonitorView::setHexMode(bool hex) {
    hexMode_ = hex;
    rebuildFromHistory();
}

void DataMonitorView::setShowTimestamp(bool show) {
    showTimestamp_ = show;
    rebuildFromHistory();
}

void DataMonitorView::setPaused(bool paused) {
    paused_ = paused;
    if (!paused_) rebuildFromHistory();
}

void DataMonitorView::clearLog() {
    logManager_->clear();
}

void DataMonitorView::retranslateUi() {
    monitorTitleLabel_->setText(tr("Data monitor"));
    lineCountLabel_->setText(tr("%1 lines").arg(lineCount_));
    rxLabel_->setText(tr("Rx %1 B").arg(rxBytes_));
    txLabel_->setText(tr("Tx %1 B").arg(txBytes_));
}

void DataMonitorView::changeEvent(QEvent *event) {
    if (event->type() == QEvent::LanguageChange) retranslateUi();
    QWidget::changeEvent(event);
}
