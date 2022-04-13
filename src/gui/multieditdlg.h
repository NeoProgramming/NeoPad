#ifndef MULTIEDITDLG_H
#define MULTIEDITDLG_H

#include <QDialog>

namespace Ui {
class MultiEditDlg;
}

class MultiEditDlg : public QDialog
{
    Q_OBJECT

public:
    explicit MultiEditDlg(QWidget *parent = 0);
    ~MultiEditDlg();
    int DoModal(QString text);
    void accept();
public:
    QString m_title;
    QString m_text;
private:
    Ui::MultiEditDlg *ui;
};

#endif // MULTIEDITDLG_H
