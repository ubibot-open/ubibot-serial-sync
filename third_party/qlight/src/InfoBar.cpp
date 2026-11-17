#include "QLight/InfoBar.h"

#include "QLight/FluentIcon.h"
#include "QLight/PushButton.h"
#include "QLight/Shadow.h"
#include "QLight/Theme.h"

#include <QAccessible>
#include <QEvent>
#include <QHBoxLayout>
#include <QLabel>
#include <QMap>
#include <QPainter>
#include <QPointer>
#include <QPropertyAnimation>
#include <QTimer>

#include "Util.h"

namespace QLight {

namespace {

constexpr int kMinHeight = 48;
constexpr int kToastWidth = 420;
constexpr int kToastMargin = 16;
constexpr int kToastGap = 8;
constexpr int kSlideMs = 200;

ShadowSpec toastShadow()
{
    const Palette &p = ThemeManager::instance().palette();
    ShadowSpec s;
    s.radius = p.radiusControl;
    s.blur = 8;
    s.offsetY = 2;
    s.color = p.shadowColor;
    return s;
}

Icon iconFor(InfoBar::Severity s)
{
    switch (s) {
    case InfoBar::Success: return Icon::CheckmarkCircle;
    case InfoBar::Warning: return Icon::Warning;
    case InfoBar::Error: return Icon::ErrorCircle;
    default: return Icon::Info;
    }
}

QColor accentFor(InfoBar::Severity s, const Palette &p)
{
    switch (s) {
    case InfoBar::Success: return p.success;
    case InfoBar::Warning: return p.caution;
    case InfoBar::Error: return p.critical;
    default: return p.accent;
    }
}

QColor fillFor(InfoBar::Severity s, const Palette &p)
{
    switch (s) {
    case InfoBar::Success: return p.statusSuccessBg;
    case InfoBar::Warning: return p.statusCautionBg;
    case InfoBar::Error: return p.statusCriticalBg;
    default: return p.statusInfoBg;
    }
}

} // namespace

// Keeps the toasts of one parent widget stacked at its top-right corner.
class InfoBarStack : public QObject {
public:
    static InfoBarStack *of(QWidget *parent)
    {
        static QMap<QWidget *, InfoBarStack *> stacks;
        auto it = stacks.find(parent);
        if (it == stacks.end()) {
            auto *s = new InfoBarStack(parent);
            it = stacks.insert(parent, s);
            connect(parent, &QObject::destroyed, [parent] { stacks.remove(parent); });
        }
        return it.value();
    }

    void add(InfoBar *bar)
    {
        m_bars.append(bar);
        layout(bar, /*animate=*/false, /*slideIn=*/true);
    }

    void remove(InfoBar *bar)
    {
        m_bars.removeAll(bar);
        relayout(true);
    }

    void relayout(bool animate)
    {
        for (const QPointer<InfoBar> &b : std::as_const(m_bars))
            if (b)
                layout(b, animate, false);
    }

protected:
    bool eventFilter(QObject *obj, QEvent *e) override
    {
        if (obj == parent() && e->type() == QEvent::Resize)
            relayout(false);
        return false;
    }

private:
    explicit InfoBarStack(QWidget *parent) : QObject(parent) { parent->installEventFilter(this); }

    int targetY(const InfoBar *bar) const
    {
        int y = kToastMargin;
        for (const QPointer<InfoBar> &b : m_bars) {
            if (!b) continue;
            if (b == bar) break;
            y += b->height() - 2 * b->shadowMargin() + kToastGap;
        }
        return y;
    }

    void layout(InfoBar *bar, bool animate, bool slideIn)
    {
        auto *parent = static_cast<QWidget *>(this->parent());
        const int m = bar->shadowMargin();
        const int cardW = qMin(kToastWidth, parent->width() - 2 * kToastMargin);
        const int w = cardW + 2 * m;
        bar->setFixedWidth(w);
        bar->resize(w, bar->layout()->totalHeightForWidth(w));

        const QPoint target(parent->width() - kToastMargin - cardW - m, targetY(bar) - m);
        if (slideIn) {
            bar->move(parent->width(), target.y());
            bar->show();
            bar->raise();
        }
        if (animate || slideIn) {
            auto *anim = new QPropertyAnimation(bar, "pos", bar);
            anim->setDuration(kSlideMs);
            anim->setEasingCurve(QEasingCurve::OutCubic);
            anim->setEndValue(target);
            anim->start(QAbstractAnimation::DeleteWhenStopped);
        } else {
            bar->move(target);
        }
    }

    QList<QPointer<InfoBar>> m_bars;
};

InfoBar::InfoBar(QWidget *parent) : InfoBar(Informational, {}, {}, parent) {}

InfoBar::InfoBar(Severity severity, const QString &title, const QString &message, QWidget *parent)
    : QWidget(parent), m_severity(severity), m_title(title), m_message(message),
      m_icon(new IconWidget(this)), m_text(new QLabel(this)), m_action(new PushButton(this)),
      m_close(new PushButton(this)), m_layout(new QHBoxLayout(this))
{
    init();
}

void InfoBar::init()
{
    setMinimumHeight(kMinHeight);
    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Minimum);

    m_icon->setIconSize(20);
    m_icon->setFixedSize(20, 20);

    m_text->setWordWrap(true);
    m_text->setTextFormat(Qt::RichText);
    m_text->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Minimum);
    m_text->setMinimumWidth(80);

    m_action->hide();
    connect(m_action, &QPushButton::clicked, this, &InfoBar::actionTriggered);

    m_close->setVariant(PushButton::Subtle);
    m_close->setFluentIcon(Icon::Dismiss);
    m_close->setFixedSize(32, 32);
    m_close->setToolTip(tr("Close"));
    m_close->setAccessibleName(tr("Close"));
    connect(m_close, &QPushButton::clicked, this, &InfoBar::dismiss);

    m_layout->setSpacing(12);
    m_layout->addWidget(m_icon, 0, Qt::AlignVCenter);
    m_layout->addWidget(m_text, 1);
    m_layout->addWidget(m_action, 0, Qt::AlignVCenter);
    m_layout->addWidget(m_close, 0, Qt::AlignVCenter);

    updateMargins();
    updateText();
    updateStyle();
    connect(&ThemeManager::instance(), &ThemeManager::themeChanged, this, [this] {
        updateText();
        updateStyle();
        update();
    });
}

int InfoBar::shadowMargin() const
{
    return m_floating ? int(Shadow::extent(toastShadow())) + 2 : 0;
}

void InfoBar::updateMargins()
{
    const int m = shadowMargin();
    m_layout->setContentsMargins(m + 16, m + 12, m + (m_closable ? 8 : 16), m + 12);
}

void InfoBar::updateText()
{
    const Palette &p = ThemeManager::instance().palette();
    QString html;
    if (!m_title.isEmpty())
        html += QStringLiteral("<b>") + m_title.toHtmlEscaped() + QStringLiteral("</b>");
    if (!m_message.isEmpty()) {
        if (!html.isEmpty())
            html += QStringLiteral("&nbsp;&nbsp;");
        html += m_message.toHtmlEscaped();
    }
    m_text->setText(html);
    QPalette pal = m_text->palette();
    pal.setColor(QPalette::WindowText, p.textPrimary);
    m_text->setPalette(pal);
    updateGeometry();
}

void InfoBar::updateStyle()
{
    const Palette &p = ThemeManager::instance().palette();
    m_icon->setFluentIcon(FluentIcon(iconFor(m_severity)));
    m_icon->setColor(accentFor(m_severity, p));
}

void InfoBar::setSeverity(Severity s)
{
    if (m_severity == s)
        return;
    m_severity = s;
    updateStyle();
    update();
}

void InfoBar::setTitle(const QString &t)
{
    m_title = t;
    updateText();
}

void InfoBar::setMessage(const QString &m)
{
    m_message = m;
    updateText();
}

void InfoBar::setClosable(bool on)
{
    m_closable = on;
    m_close->setVisible(on);
    updateMargins();
}

void InfoBar::setActionText(const QString &text)
{
    m_action->setText(text);
    m_action->setVisible(!text.isEmpty());
    updateGeometry();
}

QString InfoBar::actionText() const { return m_action->text(); }

void InfoBar::dismiss()
{
    emit closed();
    if (m_floating) {
        if (auto *stack = InfoBarStack::of(parentWidget()))
            stack->remove(this);
        deleteLater();
    } else {
        hide();
    }
}

InfoBar *InfoBar::popup(QWidget *parent, Severity severity, const QString &title, const QString &message,
                        int durationMs)
{
    Q_ASSERT(parent);
    auto *bar = new InfoBar(severity, title, message, parent);
    bar->m_floating = true;
    bar->updateMargins();
    bar->setAttribute(Qt::WA_DeleteOnClose, false);
    InfoBarStack::of(parent)->add(bar);
    if (durationMs > 0)
        QTimer::singleShot(durationMs, bar, [bar] { bar->dismiss(); });
    return bar;
}

void InfoBar::showEvent(QShowEvent *e)
{
    QWidget::showEvent(e);
    // screen readers announce alerts as they appear
    if (QAccessible::isActive()) {
        QAccessibleEvent ev(this, QAccessible::Alert);
        QAccessible::updateAccessibility(&ev);
    }
}

void InfoBar::paintEvent(QPaintEvent *)
{
    const Palette &pal = ThemeManager::instance().palette();
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    const int m = shadowMargin();
    const QRectF body = QRectF(rect()).adjusted(m, m, -m, -m);
    const qreal dpr = devicePixelRatioF();
    if (m_floating)
        Shadow::draw(&p, body, toastShadow(), dpr);

    const qreal bw = detail::hairline(dpr);
    p.setBrush(fillFor(m_severity, pal));
    p.setPen(QPen(pal.cardStroke, bw));
    p.drawRoundedRect(body.adjusted(bw / 2, bw / 2, -bw / 2, -bw / 2), pal.radiusControl, pal.radiusControl);
}

} // namespace QLight
