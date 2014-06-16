#include "scene.h"

#include <QMessageBox>
#include <QApplication>

#include <time.h>

#include "solver.h"

#include <iostream>
using namespace std;

Scene::Scene(QWidget *parent)
	: QGLViewer(parent)
{
	srand(time(NULL));

	setGeometry(0, 0, 800, 600);
	setSceneRadius(5.0f);
	setFPSIsDisplayed();

	camera()->setPosition(qglviewer::Vec(0.0, 1.0, 3.0));

	cout << "loading model...\n";
	if (!humanModel.initFromFile(tr("Models/woman.obj")))
	{
		QMessageBox::warning(this, tr("Warning"), 
			tr("Can't open data file.\n"), QMessageBox::Ok);
		QApplication::quit();
		return;
	}
	cout << "finish.\n";
	cout << "building aabb tree...\n";
	humanModel.buildAABBTree();
	cout << "finish.\n";
// 	cout << "initializing neighbor triangle...\n";
// 	humanModel.initializeNeighborTriangles();
// 	cout << "finish.\n";

// 	if (!garmentModel.initFromFile(tr("Models/garment.obj")))
// 	{
// 		QMessageBox::warning(this, tr("Warning"), 
// 			tr("Can't open data file.\n"), QMessageBox::Ok);
// 		QApplication::quit();
// 	}
	
	tshirtFront = new Cloth(Vec3b(224, 2, 126), humanModel.bboxTree->sup[2]);
	tshirtFront->initFromImage(tr("Resources/front1.png"));
	tshirtFront->buildAABBTree();
	tshirtBack = new Cloth(Vec3b(224, 2, 126), humanModel.bboxTree->inf[2]);
	tshirtBack->initFromImage(tr("Resources/back1.png"), true);
	tshirtBack->buildAABBTree();

	skirtFront = new Cloth(Vec3b(161, 21, 119), humanModel.bboxTree->sup[2], 0.55, 0.9, 1.2);
	skirtFront->initFromImage(tr("Resources/skirt-f.png"));
	skirtFront->buildAABBTree();
	skirtFront->ps.maxSpringLen = 1.1;
	skirtBack = new Cloth(Vec3b(161, 21, 119), humanModel.bboxTree->inf[2], 0.55, 0.9, 1.2);
	skirtBack->initFromImage(tr("Resources/skirt-b.png"), true);
	skirtBack->buildAABBTree();
	skirtBack->ps.maxSpringLen = 1.1;

	humanOn = true;

	sewTshirtBegan = false;
	sewTshirtFinished = false;
	tshirtSewMesh = new SewTshirtMesh;
	connect(tshirtSewMesh, SIGNAL(sewFinished()), this, SLOT(getSewedTshirt()));
	tshirtSewMesh->initialize(tshirtFront, tshirtBack);

	sewSkirtBegan = false;
	sewSkirtFinished = false;
	skirtSewMesh = new SewSkirtMesh;
	connect(skirtSewMesh, SIGNAL(sewFinished()), this, SLOT(getSewedSkirt()));
	skirtSewMesh->initialize(skirtFront, skirtBack);

	xzPl = collision::Plane(Vec3d(0, 0, 1), Vec3d(1, 0, 1), Vec3d(1, 0, -1));

	timeStep = dt;

	setAnimationPeriod(0);
	startAnimation();
}

Scene::~Scene()
{
	if (animationIsStarted())
	{
		stopAnimation();
	}
}

void Scene::init()
{
	glEnable(GL_DEPTH_TEST);

	setMouseBinding(Qt::ControlModifier | Qt::LeftButton | Qt::MidButton,  NO_CLICK_ACTION);
}

void Scene::draw()
{
	drawFloor();

	if (humanOn)
	{
		humanModel.draw(false);
	}
	if (sewTshirtBegan)
	{
		if (sewTshirtFinished)
		{
			tshirt->draw();
		}
		else
		{
			tshirtFront->draw();
			tshirtBack->draw();
		}
	}
	if (sewSkirtBegan)
	{
		if (sewSkirtFinished)
		{
			skirt->draw();
		}
		else
		{
			skirtFront->draw();
			skirtBack->draw();
		}
	}
}

void Scene::animate()
{
	if (sewTshirtBegan)
	{
		if(!sewTshirtFinished)
		{
			tshirtSewMesh->setUserForce();

			if (tshirtFront != NULL && tshirtBack != NULL)
			{
				MidpointSolver(tshirtFront->ps, timeStep);
				MidpointSolver(tshirtBack->ps, timeStep);

				tshirtFront->ps.constraintSpring(timeStep);
				tshirtBack->ps.constraintSpring(timeStep);

				tshirtFront->updateAABBTree();
				tshirtBack->updateAABBTree();

				collision::collisionObject(tshirtFront, &humanModel, H, h);
				collision::collisionObject(tshirtBack, &humanModel, H, h);
			}
		}
		else
		{
			MidpointSolver(tshirt->ps, timeStep);
			tshirt->ps.constraintSpring(timeStep);

			tshirt->updateAABBTree();

			collision::collisionObject(tshirt, &humanModel, H, h);
		}
	}
	if (sewSkirtBegan)
	{
		if (!sewSkirtFinished)
		{
			skirtSewMesh->setUserForce();

			if (skirtFront != NULL && skirtBack != NULL)
			{
				MidpointSolver(skirtFront->ps, timeStep);
				MidpointSolver(skirtBack->ps, timeStep);

				skirtFront->ps.constraintSpring(timeStep);
				skirtBack->ps.constraintSpring(timeStep);

				skirtFront->updateAABBTree();
				skirtBack->updateAABBTree();

				if (sewTshirtFinished)
				{
					collision::collisionObject(skirtFront, tshirt, H, h);
					collision::collisionObject(skirtBack, tshirt, H, h);
				}

				collision::collisionObject(skirtFront, &humanModel, H, h);
				collision::collisionObject(skirtBack, &humanModel, H, h);
			}
		}
		else
		{
			MidpointSolver(skirt->ps, timeStep);
			skirt->ps.constraintSpring(timeStep);

			skirt->updateAABBTree();

			if (sewTshirtFinished)
			{
				collision::collisionObject(skirt, tshirt, H, h);
			}

			collision::collisionObject(skirt, &humanModel, H, h);
		}
	}

	//collision::collisionPlane(frontCloth->ps, xzPl);
}

void Scene::keyPressEvent(QKeyEvent *event)
{
	switch (event->key())
	{
	case Qt::Key_Escape:
		QApplication::quit();
		break;
	case Qt::Key_H:
		humanOn = !humanOn;
		break;
	case Qt::Key_S:
		if (sewSkirtFinished)
		{
			skirt->save("skirt.mobj");
		}
		break;
	case Qt::Key_T:
		if (sewTshirtFinished)
		{
			tshirt->save("tshirt.mobj");
		}
		break;
	case Qt::Key_B:
		if (!sewTshirtBegan)
		{
			sewTshirtBegan = true;
		}
		else if (sewTshirtFinished && !sewSkirtBegan)
		{
			sewSkirtBegan = true;
		}
		break;
	default:
		QGLViewer::keyPressEvent(event);
		break;
	}
}

void Scene::loadTexture( QString filename, GLuint &texture )
{
	QImage tex, buf;
	if (!buf.load(filename))
	{
		qWarning("Cannot open the image...");
		QApplication::quit();
	}
	tex = convertToGLFormat(buf);
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);
	glTexImage2D(GL_TEXTURE_2D, 0, 3, 
		tex.width(), tex.height(), 0, 
		GL_RGBA, GL_UNSIGNED_BYTE, tex.bits());
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
}

void Scene::drawFloor()
{
#define N 5
#define INCGRID 0.2
	glDisable(GL_LIGHTING);
	glColor3d(0.0, 0.0, 0.4);
	for (double x = -N; x <= N; x += INCGRID)
	{
		glBegin(GL_LINES);
		glVertex3d(x, 0.0, -N);
		glVertex3d(x, 0.0, N);

		glVertex3d(-N, 0.0, x);
		glVertex3d(N, 0.0, x);
		glEnd();
	}
	glEnable(GL_LIGHTING);
}

void Scene::getSewedSkirt()
{
	stopAnimation();

	sewSkirtFinished = true;
	skirt = new Cloth;
	cout << "getting sewed skirt...\n";
	skirtSewMesh->getMergedCloth(skirt);
	cout << "finish.\n";
	cout << "building new aabb tree...\n";
	skirt->buildAABBTree();
	cout << "finish.\n";

	delete skirtFront;
	delete skirtBack;
	skirtFront = NULL;
	skirtBack = NULL;

	startAnimation();
}

void Scene::getSewedTshirt()
{
	stopAnimation();

	sewTshirtFinished = true;
	tshirt = new Cloth;
	cout << "getting sewed tshirt...\n";
	tshirtSewMesh->getMergedCloth(tshirt);
	cout << "finish.\n";
	cout << "building new aabb tree...\n";
	tshirt->buildAABBTree();
	cout << "finish.\n";

	delete tshirtFront;
	delete tshirtBack;
	tshirtFront = NULL;
	tshirtBack = NULL;

	startAnimation();
}
