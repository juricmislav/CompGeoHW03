#include "mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
	: QMainWindow(parent), ui(new Ui::MainWindowClass)
{
	ui->setupUi(this);
	ui->glwidget->setFocusPolicy(Qt::StrongFocus);
	QObject::connect(ui->radioButton_1,&QRadioButton::clicked,ui->glwidget,&GLWidget::radioButton1Clicked);  
	QObject::connect(ui->radioButton_2,&QRadioButton::clicked,ui->glwidget,&GLWidget::radioButton2Clicked); 

}

MainWindow::~MainWindow()
{
	delete ui;
}

