#include "QLight/TextEdit.h"

#include "QLight/ScrollBar.h"
#include "QLight/Theme.h"

#include <QAbstractScrollArea>
#include <QEvent>
#include <QPainter>
#include <QTextDocument>

#include "ColorAnimator.h"
#include "Field.h"

namespace QLight {

namespace {
constexpr int kMargin = 10;   // document margin (text inset)
constexpr int kWidth = 240;
constexpr int kHeight = 96;
} // namespace

namespace detail {

// Attached to a QAbstractScrollArea that shows editable text: transparent viewport,
// overlay scroll bars, and the field frame painted onto the viewport before the text.
class AreaFieldSkin : public QObject {
public:
    explicit AreaFieldSkin(QAbstractScrollArea *area) : QObject(area), m_area(area), m_fill(new ColorAnimator(this))
    {
        area->setFrameShape(QFrame::NoFrame);
        area->viewport()->setAutoFillBackground(false);
        area->setAttribute(Qt::WA_Hover);
        applyFieldPalette(area);
        ScrollBar::overlayIn(area);
        area->installEventFilter(this);
        area->viewport()->installEventFilter(this);

        connect(m_fill, &ColorAnimator::updated, area->viewport(), qOverload<>(&QWidget::update));
        connect(&ThemeManager::instance(), &ThemeManager::themeChanged, this, [this] {
            applyFieldPalette(m_area);
            sync(false);
            m_area->viewport()->update();
        });
        sync(false);
    }

    void paint()
    {
        QWidget *vp = m_area->viewport();
        QPainter p(vp);
        paintFieldFrame(p, QRectF(vp->rect()), vp->devicePixelRatioF(), m_fill->color(), focused(), m_area->isEnabled());
    }

protected:
    bool eventFilter(QObject *, QEvent *e) override
    {
        switch (e->type()) {
        case QEvent::FocusIn:
        case QEvent::FocusOut:
        case QEvent::Enter:
        case QEvent::Leave:
        case QEvent::HoverEnter:
        case QEvent::HoverLeave:
            sync(true);
            m_area->viewport()->update();
            break;
        case QEvent::EnabledChange:
            sync(false);
            break;
        default:
            break;
        }
        return false;
    }

private:
    bool focused() const { return m_area->hasFocus() || m_area->viewport()->hasFocus(); }
    void sync(bool animate) { m_fill->setTarget(fieldFill(m_area->isEnabled(), focused(), m_area->underMouse()), animate); }

    QAbstractScrollArea *m_area;
    ColorAnimator *m_fill;
};

} // namespace detail

TextEdit::TextEdit(QWidget *parent) : QTextEdit(parent) { init(); }
TextEdit::TextEdit(const QString &text, QWidget *parent) : QTextEdit(text, parent) { init(); }

void TextEdit::init()
{
    m_skin = new detail::AreaFieldSkin(this);
    document()->setDocumentMargin(kMargin);
}

QSize TextEdit::sizeHint() const { return {kWidth, kHeight}; }

void TextEdit::paintEvent(QPaintEvent *e)
{
    m_skin->paint();
    QTextEdit::paintEvent(e);
}

PlainTextEdit::PlainTextEdit(QWidget *parent) : QPlainTextEdit(parent) { init(); }
PlainTextEdit::PlainTextEdit(const QString &text, QWidget *parent) : QPlainTextEdit(text, parent) { init(); }

void PlainTextEdit::init()
{
    m_skin = new detail::AreaFieldSkin(this);
    document()->setDocumentMargin(kMargin);
}

QSize PlainTextEdit::sizeHint() const { return {kWidth, kHeight}; }

void PlainTextEdit::paintEvent(QPaintEvent *e)
{
    m_skin->paint();
    QPlainTextEdit::paintEvent(e);
}

} // namespace QLight
