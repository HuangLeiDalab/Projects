#include "objloader.h"

#include <QFile>
#include <QDir>
#include <QFileInfo>
#include <QTextStream>
#include <QStringList>

#include <opencv2/opencv.hpp>
using namespace cv;

bool ObjLoader::loadObj( const QString filename, Mesh *mesh )
{
	QFile file(filename);
	if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
	{
		return false;
	}
	mesh->setPathName(filename);

	QTextStream in(&file);
	QString line;
	double d1 = 0, d2 = 0, d3 = 0;
	Group *group;
	while (!in.atEnd())
	{
		line = in.readLine();

		if (line.length() == 0 ||
			line[0] == '#')
		{
			continue;
		}
		line = line.simplified();
		QStringList strs = line.split(' ');
		if (strs.count() == 0)
		{
			continue;
		}
		QString flag = strs[0];
		if (flag[0] == 'v')
		{
			if (strs.count() < 3)
			{
				continue;
			}
			d1 = strs[1].toDouble();
			d2 = strs[2].toDouble();
			if (flag.length() == 1 && strs.count() > 3)
			{
				d3 = strs[3].toFloat();
				Particle p;
				p.cp = Vec3d(d1, d2, d3);
				p.op = p.cp;
				int r = rand() % 255;
				p.c = Vec3b(r, r, r);
				p.m = mass;
				mesh->addParticle(p);
			}
			else if (flag[1] == 'n' && strs.count() > 3)
			{
				d3 = strs[3].toFloat();
				mesh->addNormal(Vec3d(d1, d2, d3));
			}
			else if (flag[1] == 't')
			{
				mesh->addTexture(Vec2d(d1, d2));
			}
		}
		else if (flag[0] == 'f')
		{
			if (strs.count() < 4)
			{
				continue;
			}
			TriangleFace f;
			f.hasN = true;
			f.hasT = true;
			if (strs[1].contains("//")) // v//n 
			{
				f.hasT = false;
				for (int i = 1; i <= 3; i++)
				{
					QStringList substrs = strs[i].split("//");
					f.v[i - 1] = substrs[0].toInt() - 1;	// because the start of index is 1 in obj file
					f.t[i - 1] = 0;
					f.n[i - 1] = substrs[1].toInt() - 1;
				}
			}
			else if (strs[1].contains(QRegExp("\\d+/\\d+/\\d+"))) // v/t/n
			{
				for (int i = 1; i <= 3; i++)
				{
					QStringList substrs = strs[i].split("/");
					f.v[i - 1] = substrs[0].toInt() - 1;
					f.t[i - 1] = substrs[1].toInt() - 1;
					f.n[i - 1] = substrs[2].toInt() - 1;
				}
			}
			else if (strs[1].contains(QRegExp("\\d+/\\d+"))) // v/t
			{
				f.hasN = false;
				for (int i = 1; i <= 3; i++)
				{
					QStringList substrs = strs[i].split("/");
					f.v[i - 1] = substrs[0].toInt() - 1;
					f.t[i - 1] = substrs[1].toInt() - 1;
					f.n[i - 1] = 0;
				}
			}
			else // v
			{
				f.hasT = false;
				f.hasN = false;
				for (int i = 1; i <= 3; i++)
				{
					f.v[i - 1] = strs[i].toInt() - 1;
					f.t[i - 1] = 0;
					f.n[i - 1] = 0;
				}
			}
			f.index = mesh->getFacesCount();
			mesh->addFace(f);
			group->tIndices.append(mesh->getFacesCount() - 1);
		}
		else if (flag[0] == 'g')
		{
			Group newgroup;
			newgroup.name = strs.count() > 1 ? strs[1] : "";
			group = mesh->addGroup(newgroup);
		}
		else if (flag[0] == 'm')
		{
// 			mesh->setMtlLibName(strs[1]);
// 			loadMtl(mesh->getMtlLibName(), mesh);
		}
		else if (flag[0] == 'u')
		{
// 			int i;
// 			for (i = 1; i < mesh->getMaterialCount(); i++)
// 			{
// 				if (strs[1] == mesh->getMaterial(i).name)
// 				{
// 					break;
// 				}
// 			}
// 			if (i == mesh->getMaterialCount())
// 			{
// 				i = 0;
// 			}
// 			group->material = i;
		}
	}

	file.close();

	return true;
}

bool ObjLoader::loadMtl( const QString filename, Mesh *mesh )
{
	QFileInfo fi(mesh->getPathName());
	QString dir = fi.dir().dirName();
	QString mtllibname = dir + filename;

	QFile file(mtllibname);
	if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
	{
		return false;
	}

	Material m;
	m.name = QString("default");
	mesh->addMaterial(m);

	QTextStream in(&file);
	QString line;
	Material *material;
	while (!in.atEnd())
	{
		line = in.readLine();

		if (line.length() == 0 ||
			line[0] == '#')
		{
			continue;
		}
		line = line.simplified();
		QStringList strs = line.split(' ');
		if (strs.count() == 0)
		{
			continue;
		}
		QString flag = strs[0];
		if (flag[0] == 'n')
		{
			QString matname = strs[1];
			Material m;
			m.name = matname;
			material = mesh->addMaterial(m);
		}
		else if (flag[0] == 'N')
		{
			material->shininess = strs[1].toFloat();
			material->shininess /= 1000.0;
			material->shininess *= 128.0;
		}
		else if (flag[0] == 'K')
		{
			float f1, f2, f3;
			f1 = strs[1].toFloat();
			f2 = strs[2].toFloat();
			f3 = strs[3].toFloat();
			if (flag[1] == 'd')
			{
				material->diffuse[0] = f1;
				material->diffuse[1] = f2;
				material->diffuse[2] = f3;
			}
			else if (flag[1] == 's')
			{
				material->specular[0] = f1; 
				material->specular[1] = f2; 
				material->specular[2] = f3;
			}
			else if (flag[1] == 'a')
			{
				material->ambient[0] = f1; 
				material->ambient[1] = f2; 
				material->ambient[2] = f3;
			}
		}
	}
	return true;
}
