
#include <QPainter>
#include <QStyleOptionButton>

#include "tablemenu.h"

#define CELLSIZE 20		// cell width and height
#define SPACE	4		// between cells
#define MARGINY 4		// top and bottom indent
#define MARGINX 4		// right and left indent
#define CURRSIZEH 20	// field with current size

TableMenu::TableMenu( QWidget *parent, int cols, int rows ) : QMenu( parent )
{
//	this->setWindowFlags( Qt::Popup );
	m_cols = cols;
	m_rows = rows;
	m_width  = cols * CELLSIZE + SPACE + 2*MARGINX;
	m_height = rows * CELLSIZE + SPACE + 2*MARGINY + CURRSIZEH;

//@	setFrameShape(QFrame::StyledPanel);
//@	setFrameShadow(QFrame::Plain);
	resize( m_width, m_height  );

	setMouseTracking( true );
	this->setStyleSheet( "QFrame{background-color: rgb(255, 255, 255);border-color: rgb(0, 0, 0);}" );
	this->setObjectName( "popup" );
}

void TableMenu::paintInfo( QPainter *painter, QRectF rc, QString text)
{
	painter->setPen( Qt::black );
	painter->fillRect( rc, Qt::lightGray );
	
	painter->drawRect( rc );
	painter->drawText( rc, text, Qt::AlignHCenter | Qt::AlignVCenter );
}

void TableMenu::showEvent ( QShowEvent * )
{
	resize( m_width, m_height  );
}

bool TableMenu::GetCell(QPointF mousePos, int &col, int &row)
{
	// get cell number by mouse coordinates
	// RETS: 1 if we are in the area of cells, 0 - outside the area
	col = -1;
	row = -1;

	int x = mousePos.x();
	int y = mousePos.y();
	x -= MARGINX;
	y -= MARGINY;
	if(x < 0)
		return 0;
	if(y < 0)
		return 0;
	if(x > m_width - MARGINX)
		return 0;
	if(y > m_height - MARGINY - CURRSIZEH)
		return 0;

	// divide the coordinates by the cell size, we get the cell number about zero
	x /= (CELLSIZE);
	y /= (CELLSIZE);
	x++;
	y++;
	if(x > m_cols || y > m_rows)
		return 0;
	col = x;
	row = y;
	return 1;
}

void TableMenu::paintEvent ( QPaintEvent * )
{
	QPainter painter( this );
	QColor clrWhite(Qt::white);
	QColor clrBlue(Qt::darkBlue);

	// common frame
	painter.drawRect(0,0,m_width-1,m_height-1);

	// mouse position
	QPointF mousePos = this->mapFromGlobal( QCursor::pos() );
	
	// we determine the square in which the mouse is located;
	
	int mcol, mrow;
	bool b = GetCell(mousePos, mcol, mrow);

	// cells
	for( int y = 0; y < m_rows; y++ )
	{
		for( int x = 0; x < m_cols; x++ )
		{
			// square
			QRectF rc;
			rc.setLeft( MARGINX + x * CELLSIZE + SPACE );
			rc.setTop(  MARGINY + y * CELLSIZE + SPACE );
			rc.setWidth(  CELLSIZE - SPACE );
			rc.setHeight( CELLSIZE - SPACE );
			
			if(x < mcol && y < mrow)
				painter.fillRect( rc, QBrush( clrBlue ) );
			else
				painter.fillRect( rc, QBrush( clrWhite ) );
		}
	}
	
	// information line
	QString info;
	info.sprintf("%d x %d", mrow, mcol);
	rectCurrentSize = QRect( SPACE, m_height - CURRSIZEH - SPACE, this->width() - 2*SPACE, CURRSIZEH );
	paintInfo(&painter, rectCurrentSize, b ? info : tr("Cancel") );
}

void TableMenu::mouseMoveEvent ( QMouseEvent * )
{
	this->repaint();
}

void TableMenu::mousePressEvent ( QMouseEvent *event )
{
	// mouse click - generate a select or cancel signal (-1, -1)
 	if( !rect().contains( event->pos() ) ) 
		this->close();

	int col, row;
	if(GetCell(event->pos(), col, row))
	{
		emit selected(col, row);
		this->close();
	}
}
