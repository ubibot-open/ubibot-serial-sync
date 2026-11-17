#include "QLight/Dialog.h"

#include "QLight/PushButton.h"
#include "QLight/Shadow.h"
#include "QLight/Theme.h"

#include <QApplication>
#include <QEvent>
#include <QHBoxLayout>
#include <QLabel>
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>
#include <QPropertyAnimation>
#include <QScreen>
#include <QVBoxLayout>
#include <QVariantAnimation>
#include <QWheelEvent>

#include "Util.h"

namespace QLight {

namespace {

constexpr int kFadeMs = 150;
constexpr int kPadding = 24;
constexpr int kButtonHeight = 32;

ShadowSpec dialogShadow()
{
    const Palette &p = ThemeManager::instance().palette();
    ShadowSpec s;
    s.radius = p.radiusCard;
    s.blur = 16;
    s.offsetY = 6;
    s.color = p.shadowColor;
    s.color.setAlpha(qMin(255, s.color.alpha() * 2));
    return s;
}

} // namespace

// Dims (and blocks) the window the dialog belongs to.
class Dialog::Overlay : public QWidget {
public:
    explicit Overlay(QWidget *parent) : QWidget(parent)
    {
        setAttribute(Qt::WA_NoSystemBackground);
        setGeometry(parent->rect());
    }

    void setAmount(qreal a)
    {
        m_amount = a;
        update();
    }

protected:
    void paintEvent(QPaintEvent *) override
    {
        QColor c = ThemeManager::instance().palette().dimOverlay;
        c.setAlpha(int(c.alpha() * m_amount));
        QPainter(this).fillRect(rect(), c);
    }
    // swallow input so nothing behind the dialog reacts
    void mousePressEvent(QMouseEvent *e) override { e->accept(); }
    void mouseReleaseEvent(QMouseEvent *e) override { e->accept(); }
    void mouseDoubleClickEvent(QMouseEvent *e) override { e->accept(); }
    void wheelEvent(QWheelEvent *e) override { e->accept(); }

private:
    qreal m_amount = 0;
};

Dialog::Dialog(QWidget *parent) : Dialog({}, {}, parent) {}

Dialog::Dialog(const QString &title, const QString &text, QWidget *parent)
    : QDialog(parent, Qt::Dialog | Qt::FramelessWindowHint | Qt::NoDropShadowWindowHint),
      m_title(new QLabel(this)), m_text(new QLabel(this)), m_footer(new QWidget(this)),
      m_buttons(new QHBoxLayout(m_footer)), m_bodyLayout(new QVBoxLayout)
{
    init();
    setTitle(title);
    setText(text);
}

Dialog::~Dialog() = default;

void Dialog::init()
{
    setAttribute(Qt::WA_TranslucentBackground);
    setModal(true);

    QFont tf = font();
    tf.setPixelSize(20);
    tf.setWeight(QFont::DemiBold);
    m_title->setFont(tf);
    m_title->setWordWrap(true);
    m_text->setWordWrap(true);
    m_text->setTextFormat(Qt::PlainText);
    m_text->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Minimum);

    m_buttons->setContentsMargins(kPadding, 16, kPadding, 16);
    m_buttons->setSpacing(8);

    m_bodyLayout->setContentsMargins(kPadding, kPadding, kPadding, kPadding);
    m_bodyLayout->setSpacing(12);
    m_bodyLayout->addWidget(m_title);
    m_bodyLayout->addWidget(m_text);

    auto *outer = new QVBoxLayout(this);
    outer->setSpacing(0);
    outer->addLayout(m_bodyLayout);
    outer->addWidget(m_footer);
    setDialogWidth(m_width);

    auto applyTheme = [this] {
        QPalette pal = palette();
        const Palette &p = ThemeManager::instance().palette();
        pal.setColor(QPalette::WindowText, p.textPrimary);
        setPalette(pal);
        update();
    };
    connect(&ThemeManager::instance(), &ThemeManager::themeChanged, this, applyTheme);
    applyTheme();
}

int Dialog::shadowMargin() const { return int(Shadow::extent(dialogShadow())) + 2; }

QString Dialog::title() const { return m_title->text(); }

void Dialog::setTitle(const QString &title)
{
    m_title->setText(title);
    m_title->setVisible(!title.isEmpty());
    setAccessibleName(title); // the dialog is announced by its title
}

QString Dialog::text() const { return m_text->text(); }

void Dialog::setText(const QString &text)
{
    m_text->setText(text);
    m_text->setVisible(!text.isEmpty() && !m_content);
}

void Dialog::setContentWidget(QWidget *widget)
{
    if (m_content) {
        m_bodyLayout->removeWidget(m_content);
        m_content->deleteLater();
    }
    m_content = widget;
    m_text->setVisible(!m_content && !m_text->text().isEmpty());
    if (widget)
        m_bodyLayout->addWidget(widget);
}

void Dialog::setDialogWidth(int width)
{
    m_width = width;
    const int m = shadowMargin();
    layout()->setContentsMargins(m, m, m, m);
    setFixedWidth(width + 2 * m);
}

PushButton *Dialog::addButton(const QString &text, int result, bool primary)
{
    auto *b = new PushButton(text, primary && !m_primary ? PushButton::Accent : PushButton::Standard, m_footer);
    b->setFixedHeight(kButtonHeight);
    b->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    const bool isPrimary = primary && !m_primary;
    b->setAutoDefault(isPrimary);
    b->setDefault(isPrimary);
    if (isPrimary)
        m_primary = b;
    connect(b, &QPushButton::clicked, this, [this, result] { done(result); });
    m_buttons->addWidget(b, 1);
    return b;
}

bool Dialog::confirm(QWidget *parent, const QString &title, const QString &text, const QString &okText,
                     const QString &cancelText)
{
    Dialog dlg(title, text, parent);
    dlg.addAcceptButton(okText.isEmpty() ? tr("OK") : okText);
    dlg.addRejectButton(cancelText.isEmpty() ? tr("Cancel") : cancelText);
    return dlg.exec() == Accepted;
}

void Dialog::inform(QWidget *parent, const QString &title, const QString &text, const QString &okText)
{
    Dialog dlg(title, text, parent);
    dlg.addAcceptButton(okText.isEmpty() ? tr("OK") : okText);
    dlg.exec();
}

void Dialog::setVisible(bool visible)
{
    if (visible && !isVisible()) {
        layout()->activate();
        const int h = layout()->totalHeightForWidth(width());
        setFixedSize(width(), h > 0 ? h : sizeHint().height());
        center(); // before the base class shows us, so it does not re-position
    }
    QDialog::setVisible(visible);
}

void Dialog::center()
{
    const QSize s = size();
    QWidget *pw = parentWidget() ? parentWidget()->window() : nullptr;
    QRect area;
    if (pw && pw->isVisible())
        area = QRect(pw->mapToGlobal(QPoint(0, 0)), pw->size());
    else if (QScreen *scr = screen())
        area = scr->availableGeometry();
    move(area.center() - QPoint(s.width() / 2, s.height() / 2));
}

void Dialog::attachOverlay()
{
    QWidget *pw = parentWidget() ? parentWidget()->window() : nullptr;
    if (!pw || m_overlay)
        return;
    m_overlay = new Overlay(pw);
    m_overlay->show();
    m_overlay->raise();
    pw->installEventFilter(this);
}

void Dialog::detachOverlay()
{
    if (!m_overlay)
        return;
    if (QWidget *pw = m_overlay->parentWidget())
        pw->removeEventFilter(this);
    m_overlay->deleteLater();
    m_overlay = nullptr;
}

void Dialog::showEvent(QShowEvent *e)
{
    QDialog::showEvent(e);
    attachOverlay();
    if (m_primary)
        m_primary->setFocus(Qt::TabFocusReason);

    // fade the card in and the dimming with it
    setWindowOpacity(0.0);
    auto *anim = new QVariantAnimation(this);
    anim->setDuration(kFadeMs);
    anim->setStartValue(0.0);
    anim->setEndValue(1.0);
    anim->setEasingCurve(QEasingCurve::OutCubic);
    connect(anim, &QVariantAnimation::valueChanged, this, [this](const QVariant &v) {
        m_fade = v.toReal();
        setWindowOpacity(m_fade);
        if (m_overlay)
            m_overlay->setAmount(m_fade);
    });
    anim->start(QAbstractAnimation::DeleteWhenStopped);
}

void Dialog::hideEvent(QHideEvent *e)
{
    QDialog::hideEvent(e);
    detachOverlay();
}

bool Dialog::eventFilter(QObject *obj, QEvent *e)
{
    if (m_overlay && obj == m_overlay->parentWidget()) {
        if (e->type() == QEvent::Resize) {
            m_overlay->setGeometry(m_overlay->parentWidget()->rect());
            center();
        } else if (e->type() == QEvent::Move) {
            center();
        }
    }
    return QDialog::eventFilter(obj, e);
}

void Dialog::paintEvent(QPaintEvent *)
{
    const Palette &pal = ThemeManager::instance().palette();
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    const int m = shadowMargin();
    const QRectF body = QRectF(rect()).adjusted(m, m, -m, -m);
    const qreal dpr = devicePixelRatioF();
    const qreal bw = detail::hairline(dpr);

    Shadow::draw(&p, body, dialogShadow(), dpr);

    QPainterPath card;
    card.addRoundedRect(body, pal.radiusCard, pal.radiusCard);
    p.save();
    p.setClipPath(card);
    p.fillPath(card, pal.cardBg);
    // the command area is a shade darker, separated by a hairline
    const QRectF footer(body.left(), m_footer->geometry().top(), body.width(), body.bottom() - m_footer->geometry().top());
    p.fillRect(footer, pal.windowBg);
    p.fillRect(QRectF(footer.left(), footer.top(), footer.width(), bw), pal.cardStroke);
    p.restore();

    p.setBrush(Qt::NoBrush);
    p.setPen(QPen(pal.cardStroke, bw));
    p.drawRoundedRect(body.adjusted(bw / 2, bw / 2, -bw / 2, -bw / 2), pal.radiusCard, pal.radiusCard);
}

} // namespace QLight
