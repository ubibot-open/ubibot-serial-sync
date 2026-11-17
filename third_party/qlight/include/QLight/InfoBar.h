#pragma once

#include <QWidget>

#include "QLight/Export.h"

class QLabel;
class QHBoxLayout;

namespace QLight {

class IconWidget;
class PushButton;

// Message bar with a severity icon, title + message, an optional action button
// and a close button. Use it inline (add it to a layout) or as a transient
// toast with InfoBar::popup().
class QLIGHT_EXPORT InfoBar : public QWidget {
    Q_OBJECT
    Q_PROPERTY(Severity severity READ severity WRITE setSeverity)
    Q_PROPERTY(QString title READ title WRITE setTitle)
    Q_PROPERTY(QString message READ message WRITE setMessage)
    Q_PROPERTY(bool closable READ isClosable WRITE setClosable)
public:
    enum Severity { Informational, Success, Warning, Error };
    Q_ENUM(Severity)

    explicit InfoBar(QWidget *parent = nullptr);
    InfoBar(Severity severity, const QString &title, const QString &message = {}, QWidget *parent = nullptr);

    Severity severity() const { return m_severity; }
    void setSeverity(Severity s);

    QString title() const { return m_title; }
    void setTitle(const QString &t);

    QString message() const { return m_message; }
    void setMessage(const QString &m);

    bool isClosable() const { return m_closable; }
    void setClosable(bool on);

    // Adds (or replaces) a button next to the text; empty text removes it.
    void setActionText(const QString &text);
    QString actionText() const;

    // Transient toast: floats over `parent` (top-right, stacked), closes itself
    // after `durationMs` (0 = stays until dismissed) and is deleted when closed.
    static InfoBar *popup(QWidget *parent, Severity severity, const QString &title,
                          const QString &message = {}, int durationMs = 4000);

signals:
    void actionTriggered();
    void closed(); // emitted when the user dismisses it or the toast times out

protected:
    void paintEvent(QPaintEvent *) override;
    void showEvent(QShowEvent *e) override;

private:
    friend class InfoBarStack;

    void init();
    void updateText();
    void updateStyle();
    void updateMargins();
    int shadowMargin() const;
    void dismiss();

    Severity m_severity = Informational;
    QString m_title, m_message;
    bool m_closable = true;
    bool m_floating = false;

    IconWidget *m_icon;
    QLabel *m_text;
    PushButton *m_action;
    PushButton *m_close;
    QHBoxLayout *m_layout;
};

} // namespace QLight
