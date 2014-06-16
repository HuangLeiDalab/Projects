#ifndef VIRTUALTRYON_H
#define VIRTUALTRYON_H

#include <QtWidgets/QMainWindow>
#include "ui_virtualtryon.h"

#include "scene.h"

class VirtualTryon : public QMainWindow
{
	Q_OBJECT

public:
	VirtualTryon(QWidget *parent = 0);
	~VirtualTryon();

private:
	Ui::VirtualTryonClass ui;

	Scene *scene;
};

#endif // VIRTUALTRYON_H
