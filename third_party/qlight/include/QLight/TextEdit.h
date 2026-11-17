#pragma once

#include <QPlainTextEdit>
#include <QTextEdit>

#include "QLight/Export.h"

namespace QLight {

namespace detail { class AreaFieldSkin; }

// Multi-line text entry with the same field look as LineEdit (rounded body, accent
// underline while focused) and thin overlay scroll bars. Inherit QTextEdit /
// QPlainTextEdit, so the whole API and every signal is unchanged.
class QLIGHT_EXPORT TextEdit : public QTextEdit {
    Q_OBJECT
public:
    explicit TextEdit(QWidget *parent = nullptr);
    explicit TextEdit(const QString &text, QWidget *parent = nullptr);

    QSize sizeHint() const override;

protected:
    void paintEvent(QPaintEvent *e) override;

private:
    void init();
    detail::AreaFieldSkin *m_skin = nullptr;
};

class QLIGHT_EXPORT PlainTextEdit : public QPlainTextEdit {
    Q_OBJECT
public:
    explicit PlainTextEdit(QWidget *parent = nullptr);
    explicit PlainTextEdit(const QString &text, QWidget *parent = nullptr);

    QSize sizeHint() const override;

protected:
    void paintEvent(QPaintEvent *e) override;

private:
    void init();
    detail::AreaFieldSkin *m_skin = nullptr;
};

} // namespace QLight
