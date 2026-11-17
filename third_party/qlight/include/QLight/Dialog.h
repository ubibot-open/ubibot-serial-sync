#pragma once

#include <QDialog>

#include "QLight/Export.h"

class QLabel;
class QHBoxLayout;
class QVBoxLayout;

namespace QLight {

class PushButton;

// Modal dialog in the Fluent "content dialog" style: rounded, shadowed card
// centred over its parent window, which is dimmed behind it. Title, message
// text (or a custom content widget) and a row of equal-width buttons.
//
//   QLight::Dialog dlg("Delete file?", "This cannot be undone.", window);
//   dlg.addAcceptButton("Delete");
//   dlg.addRejectButton("Cancel");
//   if (dlg.exec() == QDialog::Accepted) { ... }
//
// Enter triggers the primary (accent) button, Esc rejects.
class QLIGHT_EXPORT Dialog : public QDialog {
    Q_OBJECT
    Q_PROPERTY(QString title READ title WRITE setTitle)
    Q_PROPERTY(QString text READ text WRITE setText)
public:
    explicit Dialog(QWidget *parent = nullptr);
    Dialog(const QString &title, const QString &text, QWidget *parent = nullptr);
    ~Dialog() override;

    QString title() const;
    void setTitle(const QString &title);

    QString text() const;
    void setText(const QString &text);

    // Replaces the message text with your own widget (ownership passes to the dialog).
    void setContentWidget(QWidget *widget);
    QWidget *contentWidget() const { return m_content; }

    // Card width in logical pixels (default 440).
    void setDialogWidth(int width);

    // Adds a button that closes the dialog with `result`. The first button added
    // with primary=true becomes the accent-coloured default button.
    PushButton *addButton(const QString &text, int result, bool primary = false);
    PushButton *addAcceptButton(const QString &text) { return addButton(text, Accepted, true); }
    PushButton *addRejectButton(const QString &text) { return addButton(text, Rejected, false); }

    // Convenience: modal Yes/No and OK dialogs. Return true when the primary button was chosen.
    static bool confirm(QWidget *parent, const QString &title, const QString &text,
                        const QString &okText = {}, const QString &cancelText = {});
    static void inform(QWidget *parent, const QString &title, const QString &text,
                       const QString &okText = {});

    void setVisible(bool visible) override;

protected:
    void paintEvent(QPaintEvent *) override;
    void showEvent(QShowEvent *e) override;
    void hideEvent(QHideEvent *e) override;
    bool eventFilter(QObject *obj, QEvent *e) override;

private:
    class Overlay;

    void init();
    int shadowMargin() const;
    void center();
    void attachOverlay();
    void detachOverlay();

    QLabel *m_title;
    QLabel *m_text;
    QWidget *m_content = nullptr;
    QWidget *m_footer;
    QHBoxLayout *m_buttons;
    QVBoxLayout *m_bodyLayout;
    PushButton *m_primary = nullptr;
    Overlay *m_overlay = nullptr;
    int m_width = 440;
    qreal m_fade = 1.0;
};

} // namespace QLight
