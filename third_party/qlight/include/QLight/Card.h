#pragma once

#include <QFrame>

#include "QLight/Export.h"

namespace QLight {

// Rounded surface with hairline border and an optional cached drop shadow.
//
// The shadow is painted inside the widget's own bounds, so the card body is
// inset by `shadowMargin()` and the contents margins already include it:
// use `setPadding()` for the space between the card edge and its children.
class QLIGHT_EXPORT Card : public QFrame {
    Q_OBJECT
    Q_PROPERTY(bool elevated READ isElevated WRITE setElevated)
    Q_PROPERTY(int padding READ padding WRITE setPadding)
public:
    explicit Card(QWidget *parent = nullptr);

    bool isElevated() const { return m_elevated; }
    void setElevated(bool on);

    int padding() const { return m_padding; }
    void setPadding(int px);

    int shadowMargin() const;

protected:
    void paintEvent(QPaintEvent *) override;

private:
    void updateMargins();

    bool m_elevated = true;
    int m_padding = 16;
};

} // namespace QLight
