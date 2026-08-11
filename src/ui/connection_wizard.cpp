#include "ui/connection_wizard.h"
#include "core/serial_manager.h"

#include <QEvent>
#include <QLabel>
#include <QListWidget>
#include <QListWidgetItem>
#include <QVBoxLayout>
#include <QWizardPage>

namespace {

enum PageId { kPortPageId, kModelPageId, kSummaryPageId };

QString hintTag(SerialManager::PortHint hint) {
    switch (hint) {
    case SerialManager::PortHint::Recommended: return ConnectionWizard::tr("Recommended");
    case SerialManager::PortHint::Available: default: return ConnectionWizard::tr("Available");
    }
}

class PortPage : public QWizardPage {
    Q_OBJECT
public:
    PortPage() {
        list_ = new QListWidget;
        connect(list_, &QListWidget::itemSelectionChanged, this, &QWizardPage::completeChanged);
        auto *layout = new QVBoxLayout(this);
        layout->addWidget(list_);
        refresh();
    }

    void refresh() {
        list_->clear();
        for (const auto &info : SerialManager::availablePorts()) {
            QString text = info.portName;
            if (!info.description.isEmpty()) text += QStringLiteral(" — %1").arg(info.description);
            text += QStringLiteral("   [%1]").arg(hintTag(info.hint));
            auto *item = new QListWidgetItem(text, list_);
            item->setData(Qt::UserRole, info.portName);
        }
        if (list_->count() > 0) list_->setCurrentRow(0);
    }

    bool isComplete() const override { return list_->currentItem() != nullptr; }

    QString selectedPort() const {
        return list_->currentItem() ? list_->currentItem()->data(Qt::UserRole).toString() : QString();
    }

    void retranslateUi() {
        setTitle(ConnectionWizard::tr("1. Select a serial port"));
        setSubTitle(ConnectionWizard::tr("Detected serial devices are listed below."));
        refresh();
    }

private:
    QListWidget *list_;
};

class ModelPage : public QWizardPage {
    Q_OBJECT
public:
    explicit ModelPage(const DeviceLibrary *library) : library_(library) {
        list_ = new QListWidget;
        connect(list_, &QListWidget::itemSelectionChanged, this, &QWizardPage::completeChanged);
        auto *layout = new QVBoxLayout(this);
        layout->addWidget(list_);
        populate();
    }

    void populate() {
        list_->clear();
        for (const DeviceModel &model : library_->models()) {
            auto *item = new QListWidgetItem(
                QStringLiteral("%1 — %2").arg(model.id, model.description.text()), list_);
            item->setData(Qt::UserRole, model.id);
        }
        if (list_->count() > 0) list_->setCurrentRow(0);
    }

    bool isComplete() const override { return list_->currentItem() != nullptr; }

    QString selectedModelId() const {
        return list_->currentItem() ? list_->currentItem()->data(Qt::UserRole).toString() : QString();
    }

    void retranslateUi() {
        setTitle(ConnectionWizard::tr("2. Select a device model"));
        setSubTitle(
            ConnectionWizard::tr("The assistant will load that model's supported command set."));
        populate();
    }

private:
    const DeviceLibrary *library_;
    QListWidget *list_;
};

class SummaryPage : public QWizardPage {
    Q_OBJECT
public:
    SummaryPage() {
        body_ = new QLabel;
        body_->setWordWrap(true);
        auto *layout = new QVBoxLayout(this);
        layout->addWidget(body_);
        layout->addStretch();
    }

    void initializePage() override {
        auto *w = wizard();
        const QString port = static_cast<PortPage *>(w->page(kPortPageId))->selectedPort();
        const QString model = static_cast<ModelPage *>(w->page(kModelPageId))->selectedModelId();
        body_->setText(ConnectionWizard::tr(
                           "Ready to open %1 at 115200 8-N-1 and load the %2 command set.")
                           .arg(port, model));
    }

    void retranslateUi() {
        setTitle(ConnectionWizard::tr("3. Finish"));
        setSubTitle(QString());
        initializePage();
    }

private:
    QLabel *body_;
};

}  // namespace

ConnectionWizard::ConnectionWizard(const DeviceLibrary *library, QWidget *parent) : QWizard(parent) {
    setPage(kPortPageId, new PortPage);
    setPage(kModelPageId, new ModelPage(library));
    setPage(kSummaryPageId, new SummaryPage);
    setWindowTitle(tr("Connection wizard"));
    retranslateUi();
}

QString ConnectionWizard::selectedPort() const {
    return static_cast<PortPage *>(page(kPortPageId))->selectedPort();
}

QString ConnectionWizard::selectedModelId() const {
    return static_cast<ModelPage *>(page(kModelPageId))->selectedModelId();
}

void ConnectionWizard::retranslateUi() {
    setWindowTitle(tr("Connection wizard"));
    static_cast<PortPage *>(page(kPortPageId))->retranslateUi();
    static_cast<ModelPage *>(page(kModelPageId))->retranslateUi();
    static_cast<SummaryPage *>(page(kSummaryPageId))->retranslateUi();
}

void ConnectionWizard::changeEvent(QEvent *event) {
    if (event->type() == QEvent::LanguageChange) retranslateUi();
    QWizard::changeEvent(event);
}

#include "connection_wizard.moc"
