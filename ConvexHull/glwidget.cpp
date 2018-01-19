// Convexe Hülle
// (c) Georg Umlauf, 2015
// Edited and developed for Computational Geometry Project by Mislav Jurić

#include "glwidget.h"
#include <QtGui>
#include <GL/glu.h>
#include "mainwindow.h"
#include <iostream>
#include <math.h>

struct Node {
	Node *left;
	Node *right;
	Node *parent;
	QPointF point;
};

// functions declaration
void constructTree();
Node* constructBalanced2DTree(int, int, bool, Node*);
void partitionField(int, int, int, bool);
void partitionFieldV2(int, int, int, bool);
void visualizePartitions(Node*, bool);
void drawPartitions();
void reset();
void printTree(Node*, int);
void findPoints();
void drawRange();
void rangeSearch(Node*, bool);
void drawFoundPoints();
void sortSearchPoints();

// class variables
boolean treeConstruction = false;
boolean rangeSearchSelected = false;
std::vector<QPointF> points = {};
std::vector<QPointF> ySortedPoints;
std::vector<QPointF> xSortedPoints;
std::vector<QLineF> lines = {};
std::vector<QPointF> searchPoints = {};
std::vector<QPointF> foundPoints = {};
Node *root;

bool compareXCoordinate(QPointF a, QPointF b)
{
	if (a.x() == b.x()) return a.y() < b.y();
	return a.x() < b.x();
}

bool compareYCoordinate(QPointF a, QPointF b)
{
	if (a.y() == b.y()) return a.x() < b.x();
	return a.y() < b.y();
}

GLWidget::GLWidget(QWidget *parent) : QGLWidget(parent)
{
}

GLWidget::~GLWidget()
{
}

void GLWidget::paintGL()
{
	// clear
	glClear(GL_COLOR_BUFFER_BIT);

	// Koordinatensystem
	glColor3f(0.5, 0.5, 0.5);
	glBegin(GL_LINES);
	glVertex2f(-1.0, 0.0);
	glVertex2f(1.0, 0.0);
	glVertex2f(0.0, -1.0);
	glVertex2f(0.0, 1.0);
	glEnd();

	if (points.size() > 0)
	{
		glColor3f(1, 0, 1);
		glBegin(GL_POINTS);
		for (auto &point : points)
		{
			glVertex2f(point.x(), point.y());
		}
		glEnd();
	}

	if (treeConstruction)
	{
		constructTree();
		drawPartitions();
	}
	if (rangeSearchSelected)
	{
		drawPartitions();
		findPoints();
	}
}

void GLWidget::initializeGL()
{
	resizeGL(width(), height());
	glPointSize(3);
	glEnable(GL_PROGRAM_POINT_SIZE);
}

void GLWidget::resizeGL(int width, int height)
{
	aspectx = 1.0;
	aspecty = 1.0;
	if (width>height) aspectx = float(width) / height;
	else              aspecty = float(height) / width;
	glViewport(0, 0, width, height);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluOrtho2D(-aspectx, aspectx, -aspecty, aspecty);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

QPointF GLWidget::transformPosition(QPoint p)
{
	return QPointF((2.0*p.x() / width() - 1.0)*aspectx,
		-(2.0*p.y() / height() - 1.0)*aspecty);
}

void GLWidget::keyPressEvent(QKeyEvent * event)
{
	switch (event->key()) {
	default:
		break;
	}
	update();
}

void GLWidget::mousePressEvent(QMouseEvent *event)
{
	QPointF posF = transformPosition(event->pos());
	if (event->buttons() & Qt::LeftButton) {
		if (rangeSearchSelected)
		{
			if (searchPoints.size() < 2)
			{
				searchPoints.push_back(posF);
				sortSearchPoints();
				update();
				return;
			}
			
			if (searchPoints.size() >= 2)
			{
				searchPoints = {};
				foundPoints = {};
				searchPoints.push_back(posF);
			}
			update();
			return;
		}
		points.push_back(posF);
	}
	update();
}

void sortSearchPoints()
{
	if (searchPoints.size() == 2)
	{
		if (searchPoints.at(1).x() > searchPoints.at(0).x())
		{
			float x = searchPoints.at(1).x();
			searchPoints.at(1).setX(searchPoints.at(0).x());
			searchPoints.at(0).setX(x);
		}
		if (searchPoints.at(1).y() < searchPoints.at(0).y())
		{
			float y = searchPoints.at(1).y();
			searchPoints.at(1).setY(searchPoints.at(0).y());
			searchPoints.at(0).setY(y);
		}
	}
}

void GLWidget::radioButton1Clicked()
{
	// 2d-tree construction selected
	treeConstruction = true;
	rangeSearchSelected = false;
	update();
}

void GLWidget::radioButton2Clicked()
{
	// Range Search selected
	treeConstruction = false;
	rangeSearchSelected = true;
	update();
}

void constructTree()
{
	ySortedPoints = {};
	xSortedPoints = {};
	lines = {};
	root = NULL;
	if (points.size() < 1) return;
	searchPoints = {};
	foundPoints = {};

	for (QPointF &point : points)
	{
		ySortedPoints.push_back(point);
		xSortedPoints.push_back(point);
	}
	std::sort(ySortedPoints.begin(), ySortedPoints.end(), compareYCoordinate);
	std::sort(xSortedPoints.begin(), xSortedPoints.end(), compareXCoordinate);

	root = constructBalanced2DTree(0, points.size() - 1, true, NULL);

	visualizePartitions(root, true);
	//printTree(root, 0);
}

Node* constructBalanced2DTree(int leftIndex, int rightIndex, bool vertical, Node *parent)
{
	Node* node = new Node();
	node->parent = parent;
	if (leftIndex > rightIndex) return NULL;
	int median = ceil((leftIndex + rightIndex) / 2.);

	if (vertical) {
		node->point = ySortedPoints.at(median);
	}
	if (!vertical) {
		node->point = xSortedPoints.at(median);
	}

	partitionField(leftIndex, rightIndex, median, vertical);

	node->left = constructBalanced2DTree(leftIndex, median - 1, !vertical, node);
	node->right = constructBalanced2DTree(median + 1, rightIndex, !vertical, node);

	return node;
}

void partitionField(int leftIndex, int rightIndex, int median, bool vertical)
{
	std::vector<QPointF> *currentPartition = vertical ? &ySortedPoints : &xSortedPoints;
	std::vector<QPointF> *nextPartition = vertical ? &xSortedPoints : &ySortedPoints;
	std::vector<QPointF> leftPartition = {};
	std::vector<QPointF> rightPartition = {};
	
	float medianValue = vertical ? currentPartition->at(median).y() : currentPartition->at(median).x();
	for (int i = leftIndex; i <= rightIndex; ++i)
	{
		QPointF point = nextPartition->at(i);
		float nextValue = vertical ? point.y() : point.x();
		if (nextValue < medianValue)
		{
			leftPartition.push_back(point);
		}
		if (nextValue > medianValue)
		{
			rightPartition.push_back(point);
		}
	}
	leftPartition.push_back(currentPartition->at(median));
	leftPartition.insert(leftPartition.end(), rightPartition.begin(), rightPartition.end());
	std::copy(leftPartition.begin(), leftPartition.end(), (vertical ? xSortedPoints : ySortedPoints).begin() + leftIndex);
	leftPartition = std::vector<QPointF>();
	rightPartition = std::vector<QPointF>();
}

void visualizePartitions(Node* node, bool vertical)
{
	float upperBound = 1.0;
	float lowerBound = -1.0;
	float nodeValue = vertical ? node->point.y() : node->point.x();
	float comparisonValue = vertical ? node->point.x() : node->point.y();

	bool orientation = !vertical;
	Node* parentNode = node->parent;
	while (parentNode != NULL)
	{
		if (orientation == vertical)
		{
			parentNode = parentNode->parent;
			orientation = !orientation;
			continue;
		}

		float parentValue = vertical ? parentNode->point.x() : parentNode->point.y();

		if (parentValue < comparisonValue && parentValue > lowerBound)
		{
			lowerBound = parentValue;
		}
		if (parentValue > comparisonValue && parentValue < upperBound)
		{
			upperBound = parentValue;
		}
		parentNode = parentNode->parent;
		orientation = !orientation;
	}

	if (node->left != NULL)
	{
		if (vertical)
		{
			lines.push_back(QLineF(QPointF(lowerBound, nodeValue), QPointF(upperBound, nodeValue)));
		}
		else
		{
			lines.push_back(QLineF(QPointF(nodeValue, lowerBound), QPointF(nodeValue, upperBound)));
		}
		visualizePartitions(node->left, !vertical);
	}
	if (node->right != NULL)
	{
		if (vertical)
		{
			lines.push_back(QLineF(QPointF(lowerBound, nodeValue), QPointF(upperBound, nodeValue)));
		}
		else
		{
			lines.push_back(QLineF(QPointF(nodeValue, lowerBound), QPointF(nodeValue, upperBound)));
		}
		visualizePartitions(node->right, !vertical);
	}
}

void drawPartitions()
{
	if (lines.empty()) return;

	glColor3f(1, 0, 0);
	glBegin(GL_LINES);
	for (auto &line : lines)
	{
		glVertex2f(line.x1(), line.y1());
		glVertex2f(line.x2(), line.y2());
	}
	glEnd();
}

void printTree(Node* root, int depth)
{
	if (depth == 0)
	{
		std::cout << "----------------------------------------------------------------------------------------- " << std::endl;
	}
	std::string indentation = "";
	for (int i = 0; i < depth; ++i)
	{
		indentation += "\t";
	}
	std::cout << indentation << root->point.x() << ", " << root->point.y() << std::endl;
	if (root->left != NULL)
	{
		printTree(root->left, depth + 1);
	}
	if (root->right != NULL)
	{
		printTree(root->right, depth + 1);
	}
}

void findPoints()
{
	foundPoints = {};
	drawRange();
	if (root == NULL || searchPoints.size() != 4) return;
	rangeSearch(root, true);
	drawFoundPoints();
}

void rangeSearch(Node* node, bool vertical)
{
	float l, r, coord;
	if (node == NULL) return;
	if (vertical)
	{
		l = searchPoints.at(0).y();
		r = searchPoints.at(2).y();
		coord = node->point.y();
	}
	if (!vertical)
	{
		l = searchPoints.at(1).x();
		r = searchPoints.at(0).x();
		coord = node->point.x();
	}

	if (node->point.x() >= searchPoints.at(1).x() && node->point.x() <= searchPoints.at(0).x()
		&& node->point.y() >= searchPoints.at(0).y() && node->point.y() <= searchPoints.at(2).y())
	{
		foundPoints.push_back(node->point);
	}
	if (l < coord) rangeSearch(node->left, !vertical);
	if (r > coord) rangeSearch(node->right, !vertical);
}

void drawRange()
{
	if (!searchPoints.empty())
	{
		glColor3f(1, 1, 0);
		glBegin(GL_POINTS);
		for (auto &point : searchPoints)
		{
			glVertex2f(point.x(), point.y());
		}
		glEnd();
	}
	if (searchPoints.size() == 2)
	{
		searchPoints.insert(searchPoints.begin() + 1, QPointF(searchPoints.at(1).x(), searchPoints.at(0).y()));
		searchPoints.push_back(QPointF(searchPoints.at(0).x(), searchPoints.at(2).y()));
		glColor3f(1, 1, 0);
		glBegin(GL_LINE_LOOP);
		for (auto &point : searchPoints)
		{
			glVertex2f(point.x(), point.y());
		}
		glEnd();
	}
}

void drawFoundPoints()
{
	if (foundPoints.empty()) return;

	glColor3f(0, 1, 0);
	glBegin(GL_POINTS);
	for (auto &point : foundPoints)
	{
		glVertex2f(point.x(), point.y());
	}
	glEnd();
}
