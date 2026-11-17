#include "QLight/ToolTip.h"

#include "QLight/Shadow.h"
#include "QLight/Theme.h"

#include <QApplication>
#include <QGuiApplication>
#include <QHBoxLayout>
#include <QHelpEvent>
#include <QLabel>
#include <QPainter>
#include <QPointer>
#include <QPropertyAnimation>
#include <QScreen>
#include <QTextDocument>
#include <QTimer>
#include <QToolTip>

#include <cmath>

#include "Util.h"

namespace QLight {
namespace ToolTip {

namespace {

constexpr int kPadX = 10;
constexpr int kPadY = 6;
constexpr int kMaxTextWidth = 320;
constexpr int kCursorDx = 2;
constexpr int kCursorDy = 20;
constexpr int kGap = 6;
constexpr int kFadeMs = 100;
constexpr int kLifetimeMs = 10000;

ShadowSpec tipShadow()
{
    const Palette &p = ThemeManager::instance().palette();
    ShadowSpec s;
    s.radius = p.radiusControl;
    s.blur = 6;
    s.offsetY = 2;
    s.color = p.shadowColor;
    return s;
}

int shadowMargin() { return int(Shadow::extent(tipShadow())); }

class TipWidget : public QWidget {
public:
    TipWidget()
        : QWidget(nullptr, Qt::ToolTip | Qt::FramelessWindowHint | Qt::NoDropShadowWindowHint
                               | Qt::WindowDoesNotAcceptFocus),
          m_label(new QLabel(this)), m_fade(new QPropertyAnimation(this, "windowOpacity", this))
    {
        setAttribute(Qt::WA_TranslucentBackground);
        setAttribute(Qt::WA_ShowWithoutActivating);
        setAttribute(Qt::WA_TransparentForMouseEvents);

        m_label->setWordWrap(true);
        m_label->setTextFormat(Qt::AutoText);
        auto *lay = new QHBoxLayout(this);
        lay->addWidget(m_label);
        lay->setSizeConstraint(QLayout::SetFixedSize);

        m_fade->setDuration(kFadeMs);
        m_fade->setStartValue(0.0);
        m_fade->setEndValue(1.0);

        m_life.setSingleShot(true);
        m_life.setInterval(kLifetimeMs);
        connect(&m_life, &QTimer::timeout, this, &QWidget::hide);
    }

    QWidget *owner() const { return m_owner; }

    void present(const QPoint &pos, const QString &text, QWidget *owner, Placement placement)
    {
        m_owner = owner;
        const Palette &pal = ThemeManager::instance().palette();
        const int m = shadowMargin();

        const QFont f = owner ? owner->font() : QApplication::font("QTipLabel");
        m_label->setFont(f);
        QPalette lp = m_label->palette();
        lp.setColor(QPalette::WindowText, pal.textPrimary);
        m_label->setPalette(lp);
        layout()->setContentsMargins(m + kPadX, m + kPadY, m + kPadX, m + kPadY);

        QTextDocument doc; // ideal width so short tips stay one line and long ones wrap at the cap
        doc.setDefaultFont(f);
        if (Qt::mightBeRichText(text)) doc.setHtml(text); else doc.setPlainText(text);
        m_label->setFixedWidth(qMin(int(std::ceil(doc.idealWidth())) + 1, kMaxTextWidth));
        m_label->setText(text);
        layout()->activate();
        adjustSize();

        const QSize card = size() - QSize(2 * m, 2 * m);
        QScreen *screen = QGuiApplication::screenAt(pos);
        if (!screen) screen = QGuiApplication::primaryScreen();
        const QRect avail = screen->availableGeometry();

        QPoint tl;
        if (placement == Placement::Above) {
            tl = QPoint(pos.x() - card.width() / 2, pos.y() - kGap - card.height());
            if (tl.y() < avail.top()) tl.setY(pos.y() + kGap); // no room above: flip below
        } else {
            tl = pos + QPoint(kCursorDx, kCursorDy);
            if (tl.y() + card.height() > avail.bottom()) tl.setY(pos.y() - kGap - card.height());
        }
        tl.setX(qBound(avail.left(), tl.x(), qMax(avail.left(), avail.right() - card.width())));
        tl.setY(qBound(avail.top(), tl.y(), qMax(avail.top(), avail.bottom() - card.height())));
        move(tl - QPoint(m, m));

        m_life.start();
        if (!isVisible()) {
            setWindowOpacity(0.0);
            show();
            m_fade->start();
        } else {
            update();
        }
    }

protected:
    void paintEvent(QPaintEvent *) override
    {
        const Palette &pal = ThemeManager::instance().palette();
        QPainter p(this);
        p.setRenderHint(QPainter::Antialiasing);
        const int m = shadowMargin();
        const QRectF body = QRectF(rect()).adjusted(m, m, -m, -m);
        const qreal dpr = devicePixelRatioF();

        Shadow::draw(&p, body, tipShadow(), dpr);
        const qreal bw = detail::hairline(dpr);
        p.setBrush(pal.cardBg);
        p.setPen(QPen(pal.cardStroke, bw));
        p.drawRoundedRect(body.adjusted(bw / 2, bw / 2, -bw / 2, -bw / 2), pal.radiusControl,
                          pal.radiusControl);
    }

    void hideEvent(QHideEvent *e) override
    {
        QWidget::hideEvent(e);
        m_life.stop();
        m_fade->stop();
        m_owner = nullptr;
    }

private:
    QLabel *m_label;
    QPropertyAnimation *m_fade;
    QTimer m_life;
    QPointer<QWidget> m_owner;
};

TipWidget *g_tip = nullptr;

TipWidget *tip()
{
    if (!g_tip) {
        g_tip = new TipWidget;
        qAddPostRoutine([] {
            delete g_tip;
            g_tip = nullptr;
        });
    }
    return g_tip;
}

void syncNativeToolTipPalette()
{
    const Palette &p = ThemeManager::instance().palette();
    QPalette pal = QToolTip::palette();
    pal.setColor(QPalette::ToolTipBase, p.cardBg);
    pal.setColor(QPalette::ToolTipText, p.textPrimary);
    pal.setColor(QPalette::Window, p.cardBg);
    pal.setColor(QPalette::WindowText, p.textPrimary);
    QToolTip::setPalette(pal);
}

class GlobalFilter : public QObject {
public:
    using QObject::QObject;

    bool eventFilter(QObject *obj, QEvent *e) override
    {
        switch (e->type()) {
        case QEvent::ToolTip: {
            auto *w = qobject_cast<QWidget *>(obj);
            if (!w)
                break;
            const QString text = w->toolTip();
            if (text.isEmpty())
                break; // widget-provided tooltip (e.g. per item): leave to QToolTip
            showText(static_cast<QHelpEvent *>(e)->globalPos(), text, w);
            return true;
        }
        case QEvent::MouseButtonPress:
        case QEvent::KeyPress:
        case QEvent::Wheel:
        case QEvent::WindowDeactivate:
            hideText();
            break;
        case QEvent::Leave:
            if (g_tip && g_tip->isVisible() && g_tip->owner() == obj)
                hideText();
            break;
        default:
            break;
        }
        return false;
    }
};

} // namespace

void installGlobally()
{
    static bool installed = false;
    if (installed || !qApp)
        return;
    installed = true;
    qApp->installEventFilter(new GlobalFilter(qApp));
    syncNativeToolTipPalette();
    QObject::connect(&ThemeManager::instance(), &ThemeManager::themeChanged, qApp,
                     [] { syncNativeToolTipPalette(); });
}

void showText(const QPoint &globalPos, const QString &text, QWidget *owner, Placement placement)
{
    if (text.isEmpty()) {
        hideText();
        return;
    }
    tip()->present(globalPos, text, owner, placement);
}

void hideText()
{
    if (g_tip)
        g_tip->hide();
}

bool isVisible() { return g_tip && g_tip->isVisible(); }

} // namespace ToolTip
} // namespace QLight
