#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QProcess>

#include "translator.h"

namespace Ui {
class Widget;
}

class Widget : public QWidget
{
    Q_OBJECT

public:
    explicit Widget(QWidget *parent = 0);
    ~Widget();

private slots:
    void on_saveBtn_clicked();
    void on_openBtn_clicked();
    void slotProFinished(int exitCode, QProcess::ExitStatus exitStatus);
    void slotProErrorOccurred(QProcess::ProcessError error);
    void slotStatusChange(const QString &status);
    void slotTranslateError(const QString &errStr);
    void slotTranslateFinished(const QStringList &content);

private:
    Ui::Widget *ui;

    QProcess *m_pro = nullptr;
    Translator *m_tran = nullptr;
};

#endif // WIDGET_H
