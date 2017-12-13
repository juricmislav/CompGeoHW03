// Convexe Hülle
// 
// Widget für Interaktion und Kontrolle 
//
// (c) Georg Umlauf, 2014

#ifndef GLWIDGET_H
#define GLWIDGET_H

#include <QGLWidget>
#include <QFuture>

class GLWidget : public QGLWidget
{
    Q_OBJECT
public:
    GLWidget                  (QWidget *parent=0);
    ~GLWidget                 ();
signals: 
	void continueRequest      ();
public slots:
    void radioButton1Clicked  ();
    void radioButton2Clicked  ();
protected:
    void paintGL              ();
    void initializeGL         ();
    void resizeGL             (int width, int height);
    void keyPressEvent        (QKeyEvent   *event);
    void mousePressEvent      (QMouseEvent *event);
private:
    QPointF transformPosition (QPoint p);
	double  aspectx, aspecty;
};



#endif // GLWIDGET_H
