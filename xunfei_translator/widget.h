#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>

namespace Ui {
class Widget;
}

class Widget : public QWidget
{
    Q_OBJECT

public:
    explicit Widget(QWidget *parent = 0);
    ~Widget();

public slots:
    void slotShowTranslateResult(const QString &content, bool isEndle);

protected:
    bool eventFilter(QObject *watched, QEvent *event);
    void paintEvent(QPaintEvent *e);

private slots:
    void on_clearBtn_clicked();

private:
    Ui::Widget *ui;
    QList<QPair<QString, bool>> m_textList;

    bool m_isMousePressed = false;
    QPoint m_pressedPos;
    QPoint m_winPos;

    bool m_isMousePressForResizeWin = false;
    QPoint m_pressedPosForResizeWin;
    QSize m_winSizeForResizeWin;
};

#endif // WIDGET_H
