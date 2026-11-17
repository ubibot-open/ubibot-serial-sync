#pragma once

#include <QProgressBar>
#include <QVariantAnimation>

#include "QLight/Export.h"

namespace QLight {

// Thin Fluent-style progress bar. Inherits QProgressBar; a range of (0, 0)
// switches to the indeterminate (busy) animation. The text is hidden by default.
class QLIGHT_EXPORT ProgressBar : public QProgressBar {
    Q_OBJECT
    Q_PROPERTY(State state READ state WRITE setState)
public:
    enum State { Normal, Paused, Error };
    Q_ENUM(State)

    explicit ProgressBar(QWidget *parent = nullptr);

    State state() const { return m_state; }
    void setState(State s);

    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

protected:
    void paintEvent(QPaintEvent *) override;
    void showEvent(QShowEvent *e) override;
    void hideEvent(QHideEvent *e) override;
    void changeEvent(QEvent *e) override;

private:
    bool busy() const { return minimum() == 0 && maximum() == 0; }
    qreal targetFraction() const;
    void syncValue(bool animate);
    void syncBusy();

    State m_state = Normal;
    qreal m_fraction = 0;
    QVariantAnimation m_valueAnim;
    QVariantAnimation m_busyAnim;
    qreal m_busyPhase = 0;
    int m_lastMin = 0;
    int m_lastMax = 99;
};

} // namespace QLight
