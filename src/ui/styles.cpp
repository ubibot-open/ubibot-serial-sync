#include "ui/styles.h"

namespace Styles {

QString appStyleSheet() {
    return QStringLiteral(R"(
QMainWindow, QDialog {
    background: %1;
    color: %2;
}
QWidget {
    color: %2;
}
QToolBar {
    background: %1;
    border: none;
    border-bottom: 1px solid %4;
    spacing: 4px;
    padding: 4px 6px;
}
QMenuBar {
    background: %1;
    border-bottom: 1px solid %4;
}
QStatusBar {
    background: %1;
    border-top: 1px solid %4;
}
QGroupBox {
    font-weight: 600;
    border: 1px solid %4;
    border-radius: 2px;
    margin-top: 10px;
    padding-top: 8px;
}
QGroupBox::title {
    subcontrol-origin: margin;
    left: 8px;
    padding: 0 4px;
    color: %3;
}
QPushButton {
    border: 1px solid %4;
    border-radius: 2px;
    padding: 5px 12px;
    background: transparent;
}
QPushButton:hover {
    background: rgba(0, 0, 0, 12);
}
QPushButton:pressed {
    background: rgba(0, 0, 0, 22);
}
QPushButton:checkable:checked {
    background: %3;
    color: %1;
    border-color: %3;
}
QPushButton#primaryButton, QPushButton[primary="true"] {
    background: %3;
    color: %1;
    border-color: %3;
    font-weight: 600;
}
QPushButton#primaryButton:hover, QPushButton[primary="true"]:hover {
    background: %5;
}
QLineEdit, QPlainTextEdit, QTextEdit, QComboBox, QSpinBox {
    background: %6;
    border: 1px solid %4;
    border-radius: 2px;
    padding: 4px 6px;
    selection-background-color: %3;
}
QLineEdit:focus, QComboBox:focus, QPlainTextEdit:focus, QTextEdit:focus {
    border-color: %3;
}
QTreeWidget {
    background: %1;
    border: none;
    outline: none;
}
QTreeWidget::item {
    border-bottom: 1px solid rgba(0, 0, 0, 12);
    padding: 3px 0;
}
QTreeWidget::item:selected {
    background: rgba(89, 128, 166, 40);
    color: %2;
}
QHeaderView::section {
    background: %1;
    border: none;
    border-bottom: 1px solid %4;
    padding: 4px;
}
QTabWidget::pane {
    border: 1px solid %4;
}
)")
        .arg(kBackground, kText, kAccent, kDivider, kAccent700, kSurface);
}

QColor colorForLogKind(LogKind kind) {
    switch (kind) {
    case LogKind::Tx: return QColor(kAccent800);
    case LogKind::Rx: return QColor(kText);
    case LogKind::Sys: return QColor(128, 128, 128);
    case LogKind::Err: return QColor(kError);
    }
    return QColor(kText);
}

}  // namespace Styles
