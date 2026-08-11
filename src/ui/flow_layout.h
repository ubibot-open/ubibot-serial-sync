#pragma once

#include <QLayout>
#include <QStyle>

// A left-to-right, top-to-bottom wrapping layout -- Qt Widgets has no
// built-in equivalent. Used for the filter-chip row in CommandLibraryPanel.
// This is the standard "Flow Layout" pattern from Qt's own examples,
// reimplemented here in a self-contained form.
class FlowLayout : public QLayout {
public:
    explicit FlowLayout(QWidget *parent, int margin = 0, int hSpacing = 6, int vSpacing = 6);
    explicit FlowLayout(int hSpacing = 6, int vSpacing = 6);
    ~FlowLayout() override;

    void addItem(QLayoutItem *item) override;
    int horizontalSpacing() const;
    int verticalSpacing() const;
    Qt::Orientations expandingDirections() const override;
    bool hasHeightForWidth() const override;
    int heightForWidth(int width) const override;
    int count() const override;
    QLayoutItem *itemAt(int index) const override;
    QSize minimumSize() const override;
    void setGeometry(const QRect &rect) override;
    QSize sizeHint() const override;
    QLayoutItem *takeAt(int index) override;

private:
    int doLayout(const QRect &rect, bool testOnly) const;
    int smartSpacing(QStyle::PixelMetric pm) const;

    QList<QLayoutItem *> itemList_;
    int hSpace_;
    int vSpace_;
};
