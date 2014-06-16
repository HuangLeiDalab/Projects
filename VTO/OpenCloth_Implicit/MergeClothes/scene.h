#ifndef SCENE_H
#define SCENE_H

#include <QKeyEvent>
#include <qglviewer.h>

#include "mesh.h"
#include "collision.h"

class Scene : public QGLViewer
{
	Q_OBJECT

public:
	Scene(QWidget *parent = 0);
	~Scene();

protected:
	void init();
	void draw();
	void animate();

	void keyPressEvent(QKeyEvent *);

private:
	Mesh humanModel;
	Cloth *skirtFront, *skirtBack, *skirt;
	Cloth *tshirtFront, *tshirtBack, *tshirt;
	SewMesh *skirtSewMesh, *tshirtSewMesh;
	collision::Plane xzPl;

	bool humanOn;

	bool sewSkirtFinished, sewTshirtFinished;
	bool sewSkirtBegan, sewTshirtBegan;

	double timeStep;

	GLuint textures[2];

	void loadTexture(QString filename, GLuint &texture);
	void drawFloor();

private slots:
	void getSewedSkirt();
	void getSewedTshirt();
};

#endif // SCENE_H
