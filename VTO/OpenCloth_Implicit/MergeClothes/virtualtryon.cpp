#include "virtualtryon.h"

VirtualTryon::VirtualTryon(QWidget *parent)
	: QMainWindow(parent)
{
	ui.setupUi(this);

	scene = new Scene;
	setCentralWidget(scene);
}

VirtualTryon::~VirtualTryon()
{

}
