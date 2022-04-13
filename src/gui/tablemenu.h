#ifndef NEOTABLEMENU_H
#define NEOTABLEMENU_H

#include <QMenu>
#include <QPainter>
#include <QDebug>
#include <QMouseEvent>
#include <QColor>
#include <QRectF>
#include <QColorDialog>
#include <QHash>
#include <QApplication>


class TableMenu : public QMenu
{
   Q_OBJECT

   QRectF rectCurrentSize;

   int m_cols, m_rows;
   int m_width, m_height;
signals:
   void selected( int col, int row );

public:
   TableMenu( QWidget *parent, int cols, int rows );

   void paintInfo( QPainter *painter, QRectF rc, QString text);
   bool GetCell(QPointF mousePos, int &col, int &row);

   void showEvent ( QShowEvent * );
   void paintEvent ( QPaintEvent * );
   void mouseMoveEvent ( QMouseEvent * );
   void mousePressEvent ( QMouseEvent *event );
};


#endif // NEOTABLEMENU_H
