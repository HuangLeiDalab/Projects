#include "aabbtree.h"

#include "mesh.h"

#define ADDPOINT(P) \
	if (P->x[0] < inf[0]) inf[0] = P->x[0]; \
	if (P->x[1] < inf[1]) inf[1] = P->x[1]; \
	if (P->x[2] < inf[2]) inf[2] = P->x[2]; \
	if (P->x[0] > sup[0]) sup[0] = P->x[0]; \
	if (P->x[1] > sup[1]) sup[1] = P->x[1]; \
	if (P->x[2] > sup[2]) sup[2] = P->x[2];

void BBox::render()
{
	Vec3d p0(inf),
		p1(sup[0], inf[1], inf[2]),
		p2(sup[0], sup[1], inf[2]),
		p3(inf[0], sup[1], inf[2]),
		p4(inf[0], inf[1], sup[2]),
		p5(sup[0], inf[1], sup[2]),
		p6(sup),
		p7(inf[0], sup[1], sup[2]);
	glBegin(GL_LINES);

	glVertex3dv(&p0[0]);
	glVertex3dv(&p1[0]);

	glVertex3dv(&p1[0]);
	glVertex3dv(&p2[0]);

	glVertex3dv(&p2[0]);
	glVertex3dv(&p3[0]);

	glVertex3dv(&p3[0]);
	glVertex3dv(&p0[0]);

	glVertex3dv(&p4[0]);
	glVertex3dv(&p5[0]);

	glVertex3dv(&p5[0]);
	glVertex3dv(&p6[0]);

	glVertex3dv(&p6[0]);
	glVertex3dv(&p7[0]);

	glVertex3dv(&p7[0]);
	glVertex3dv(&p4[0]);

	glVertex3dv(&p0[0]);
	glVertex3dv(&p4[0]);

	glVertex3dv(&p1[0]);
	glVertex3dv(&p5[0]);

	glVertex3dv(&p2[0]);
	glVertex3dv(&p6[0]);

	glVertex3dv(&p3[0]);
	glVertex3dv(&p7[0]);

	glEnd();
}

void AABBNode::computeBounds(Mesh *m)
{
	if (left != NULL)
	{
		left->computeBounds(m);
		inf = left->inf;
		sup = left->sup;
	}
	if (right != NULL)
	{
		right->computeBounds(m);
		inf = right->inf;
		sup = right->sup;
	}
	if (left != NULL && right != NULL)
	{
		MIN_VEC3(left->inf, right->inf, inf);
		MAX_VEC3(left->sup, right->sup, sup);
	}
}

bool AABBNode::intersect( BBox *bbox )
{
	for (int i = 0; i < 3; i++)
	{
		if (sup[i] < bbox->inf[i] || inf[i] > bbox->sup[i])
		{
			return false;
		}
	}
	return true;
}

cv::Vec3i AABBNode::getIndices()
{
	return Vec3i(-1, -1, -1);
}

AABBLeaf::AABBLeaf( int ia, int ib, int ic) 
	: a(ia), b(ib), c(ic)
{
	left = NULL; 
	right = NULL;
}

void AABBLeaf::computeBounds(Mesh *m)
{
	MIN_VEC3(m->ps.pts[a].cp, m->ps.pts[b].cp, inf);
	MIN_VEC3(inf, m->ps.pts[c].cp, inf);
	MAX_VEC3(m->ps.pts[a].cp, m->ps.pts[b].cp, sup);
	MAX_VEC3(sup, m->ps.pts[c].cp, sup);
}

cv::Vec3i AABBLeaf::getIndices()
{
	return Vec3i(a, b, c);
}