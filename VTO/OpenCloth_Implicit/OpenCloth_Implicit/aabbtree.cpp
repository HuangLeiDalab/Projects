#include "aabbtree.h"

void BBox::render()
{
	vec3 p0(inf),
		 p1(sup[0], inf[1], inf[2]),
		 p2(sup[0], sup[1], inf[2]),
		 p3(inf[0], sup[1], inf[2]),
		 p4(inf[0], inf[1], sup[2]),
		 p5(sup[0], inf[1], sup[2]),
		 p6(sup),
		 p7(inf[0], sup[1], sup[2]);
	glBegin(GL_LINES);

	glVertex3fv(&p0[0]);
	glVertex3fv(&p1[0]);

	glVertex3fv(&p1[0]);
	glVertex3fv(&p2[0]);

	glVertex3fv(&p2[0]);
	glVertex3fv(&p3[0]);

	glVertex3fv(&p3[0]);
	glVertex3fv(&p0[0]);

	glVertex3fv(&p4[0]);
	glVertex3fv(&p5[0]);

	glVertex3fv(&p5[0]);
	glVertex3fv(&p6[0]);

	glVertex3fv(&p6[0]);
	glVertex3fv(&p7[0]);

	glVertex3fv(&p7[0]);
	glVertex3fv(&p4[0]);

	glVertex3fv(&p0[0]);
	glVertex3fv(&p4[0]);

	glVertex3fv(&p1[0]);
	glVertex3fv(&p5[0]);

	glVertex3fv(&p2[0]);
	glVertex3fv(&p6[0]);

	glVertex3fv(&p3[0]);
	glVertex3fv(&p7[0]);

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

uvec3 AABBNode::getIndices()
{
	return uvec3(-1, -1, -1);
}

AABBLeaf::AABBLeaf( int ia, int ib, int ic) 
	: a(ia), b(ib), c(ic)
{
	left = NULL; 
	right = NULL;
}

void AABBLeaf::computeBounds(Mesh *m)
{
	MIN_VEC3(m->mVertices[a], m->mVertices[b], inf);
	MIN_VEC3(inf, m->mVertices[c], inf);
	MAX_VEC3(m->mVertices[a], m->mVertices[b], sup);
	MAX_VEC3(sup, m->mVertices[c], sup);
}

uvec3 AABBLeaf::getIndices()
{
	return uvec3(a, b, c);
}