#include "qphysicsshapedrawer.h"


CQuadrionTextureFile QPHYSICSSHAPEDRAWEREXPORT_API qMakeShapeMaterial(vec4f _color)
{
	CQuadrionTextureFile tex;
	tex.SetFileName( "none" );
	tex.SetFilePath( "./" );
	unsigned int color = 0x00000000;//0x000000ff;
	color |= ( unsigned char( _color.x * 255 ) << 24 );
	color |= ( unsigned char( _color.y * 255 ) << 16 );
	color |= ( unsigned char( _color.z * 255 ) << 8 );
	color |= ( unsigned char( _color.w * 255 ) );
	tex.LoadFromColor( color );

	return tex;
}



qPhysicsModelObject::qPhysicsModelObject(const unsigned int handle, const std::string& name, const std::string& path) : CModelObject(handle, name, path)
{
	tex.SetFileName( "none" );
	tex.SetFilePath( "./" );
	unsigned int color = 0x000000ff;
	color |= ( unsigned char( 0.8f * 255 ) << 24 );
	color |= ( unsigned char( 0.8f * 255 ) << 16 );
	color |= ( unsigned char( 0.8f * 255 ) << 8 );
	tex.LoadFromColor( color );

	unsigned int flags = QTEXTURE_FILTER_TRILINEAR;
	tex_handel = g_pRender->AddTextureObject( tex, flags );
	BindDiffuseTexture(tex_handel);


	v_desc.pool = QVERTEXBUFFER_MEMORY_STATIC;
	v_desc.usage[0] = QVERTEXFORMAT_USAGE_POSITION;
	v_desc.size[0] = QVERTEXFORMAT_SIZE_FLOAT3;
	v_desc.usage[1] = QVERTEXFORMAT_USAGE_NORMAL;
	v_desc.size[1] = QVERTEXFORMAT_SIZE_FLOAT3;
}

qPhysicsModelObject::~qPhysicsModelObject()
{
}

void qPhysicsModelObject::RenderModel()
{
	CQuadrionTextureObject* tex_obj = NULL;
	tex_obj = g_pRender->GetTextureObject(tex_handel);
	if( tex_obj )
		tex_obj->BindTexture( m_diffuseBindPoint );

	vec3f box(1.0f, 1.0f, 1.0f);
	g_pRender->RenderBox(-box, box, QRENDER_MAKE_ARGB(0xFF, 0,255,255));

	g_pRender->EvictTextures();
}

qPhysicsShapeDrawer::qPhysicsShapeDrawer()
{
	this->mDefaultMaterial = -1;
	/*CQuadrionTextureFile tex = qMakeShapeMaterial(vec4f(1,1,1,1));

	unsigned int flags = QTEXTURE_FILTER_TRILINEAR;
	this->mDefaultMaterial = g_pRender->AddTextureObject( tex, flags );*/
	this->box = new qPhysicsModelObject(0, "none");
	this->mEffect = g_pRender->AddEffect( "Phong2.fx", "Media/Effects/" );
}

qPhysicsShapeDrawer::~qPhysicsShapeDrawer()
{
}

void qPhysicsShapeDrawer::renderShape(const btCollisionShape *shape, btTransform &trans)
{
	if(this->mDefaultMaterial == -1 && g_pRender != NULL)
	{
		//CQuadrionTextureFile tex = qMakeShapeMaterial(vec4f(1,1,1,1));
		//unsigned int flags = QTEXTURE_FILTER_TRILINEAR;
		//this->mDefaultMaterial = g_pRender->AddTextureObject( tex, flags );
	}

	int shapetype = shape->getShapeType();	
	
	switch (shapetype)
	{	
		case COMPOUND_SHAPE_PROXYTYPE:
		{
			const btCompoundShape* compoundShape = static_cast<const btCompoundShape*>(shape);
			for (int i = compoundShape->getNumChildShapes()-1; i >= 0; i--)
			{
				btTransform childTrans = compoundShape->getChildTransform(i);
				const btCollisionShape* colShape = compoundShape->getChildShape(i);				
				renderShape(colShape, trans * childTrans);
			}
		}
		break;
	case BOX_SHAPE_PROXYTYPE:
		{
			/*
			CQuadrionEffect* fx = g_pRender->GetEffect( this->mEffect );
			unsigned int mat = QRENDER_MATRIX_MODELVIEWPROJECTION;
			unsigned int worldMat = QRENDER_MATRIX_MODEL;
			vec3f camPos(100,100,100);
			*/

			//if(this->box == NULL)
			mat4 prev;
			g_pRender->GetMatrix( QRENDER_MATRIX_MODEL, prev );
	
			//btTransform trans;
			//handle->getMotionState()->getWorldTransform(trans);
			mat4 rot;
	
			trans.getOpenGLMatrix(rot);
			btVector3 H = ((btBoxShape*)shape)->getHalfExtentsWithMargin();
			mat4 S;
			QMATH_MATRIX_LOADSCALE(S, vec3f(H.x(), H.y(), H.z()));
			QMATH_MATRIX_MULTIPLY(rot, S, rot);
	
			this->box->SetModelOrientation(rot);
			this->box->CreateFinalTransform(rot);

			/*
			fx->BeginEffect( "Phong2" );
	
			fx->UploadParameters( "g_mMVP", QEFFECT_VARIABLE_STATE_MATRIX, 1, &mat );
			fx->UploadParameters("g_mWorld", QEFFECT_VARIABLE_STATE_MATRIX, 1, &worldMat);
			fx->UploadParameters( "g_lightPos", QEFFECT_VARIABLE_FLOAT_ARRAY, 3, &camPos );
			fx->UploadParameters( "g_camPos", QEFFECT_VARIABLE_FLOAT_ARRAY, 3, &camPos );
			fx->RenderEffect( 0 );
			*/

			this->box->RenderModel();

			/*
			fx->EndRender( 0 );
			fx->EndEffect();
			*/

			g_pRender->SetMatrix( QRENDER_MATRIX_MODEL, prev );
		}
		break;
	case CYLINDER_SHAPE_PROXYTYPE:
		{
			int upAxis = ((btCylinderShape*)shape)->getUpAxis();
			float radius =  ((btCylinderShape*)shape)->getRadius();
			float halfHeight =  ((btCylinderShape*)shape)->getHalfExtentsWithMargin()[upAxis];

			/*D3DXMATRIX matWorld;
			D3DXMATRIX matScale, matRot, matTrans;					
			D3DXMatrixScaling( &matScale, radius, halfHeight, radius );
			D3DXMatrixTranslation( &matTrans, trn.getOrigin().x(), trn.getOrigin().y(), trn.getOrigin().z());

			D3DXMatrixRotationQuaternion( &matRot, &D3DXQUATERNION(
				trn.getRotation().x(),
				trn.getRotation().y(),
				trn.getRotation().z(),
				trn.getRotation().w()));		

			D3DXMatrixMultiply(&matWorld, &matScale, &matRot);
			D3DXMatrixMultiply(&matWorld, &matWorld, &matTrans);

			D3DXMatrixRotationX( &matRot, D3DXToRadian(-90.0f));
			D3DXMatrixMultiply(&matWorld, &matRot, &matWorld);

			mDevice->SetTransform( D3DTS_WORLD, &matWorld );							
			mCylinderMesh->DrawSubset( 0 );

			mNumVertices += mCylinderMesh->GetNumVertices();
			mNumFaces	+= mCylinderMesh->GetNumFaces();
			mNumObjects++;*/
		}
		break;		
		case CONE_SHAPE_PROXYTYPE:
		{
			float radius =  ((btConeShape*)shape)->getRadius();
			float halfHeight =  ((btConeShape*)shape)->getHeight();

			/*D3DXMATRIX matWorld;
			D3DXMATRIX matScale, matRot, matTrans;					
			D3DXMatrixScaling( &matScale, radius, halfHeight, radius );
			D3DXMatrixTranslation( &matTrans, trn.getOrigin().x(), trn.getOrigin().y(), trn.getOrigin().z());

			D3DXMatrixRotationQuaternion( &matRot, &D3DXQUATERNION(
			trn.getRotation().x(),
			trn.getRotation().y(),
			trn.getRotation().z(),
			trn.getRotation().w()));		
	
			D3DXMatrixMultiply(&matWorld, &matScale, &matRot);
			D3DXMatrixMultiply(&matWorld, &matWorld, &matTrans);

			D3DXMatrixRotationX( &matRot, D3DXToRadian(-90.0f));
			D3DXMatrixMultiply(&matWorld, &matRot, &matWorld);

			mDevice->SetTransform( D3DTS_WORLD, &matWorld );
			mConeMesh->DrawSubset( 0 );

			mNumVertices += mConeMesh->GetNumVertices();
			mNumFaces	+= mConeMesh->GetNumFaces();
			mNumObjects++;*/
		}
		break;		
	case SPHERE_SHAPE_PROXYTYPE:
		{
			float fRadius = ((btSphereShape*)shape)->getMargin();			

			/*D3DXMATRIX matWorld;
			D3DXMATRIX matScale, matRot, matTrans;					
			D3DXMatrixScaling( &matScale, fRadius, fRadius, fRadius );
			D3DXMatrixTranslation( &matTrans, trn.getOrigin().x(), trn.getOrigin().y(), trn.getOrigin().z());
			D3DXMatrixRotationQuaternion( &matRot, &D3DXQUATERNION(
				trn.getRotation().x(),
				trn.getRotation().y(),
				trn.getRotation().z(),
				trn.getRotation().w()));

			D3DXMatrixMultiply(&matWorld, &matScale, &matRot);
			D3DXMatrixMultiply(&matWorld, &matWorld, &matTrans);			

			mDevice->SetTransform( D3DTS_WORLD, &matWorld );	
			mSphereMesh->DrawSubset( 0 );

			mNumVertices += mSphereMesh->GetNumVertices();
			mNumFaces	+= mSphereMesh->GetNumFaces();
			mNumObjects++;*/
		}
		break;
	default:
		{
			if( shape->isConvex() )
			{
				qShapeCache*	sc = Cache((btConvexShape*)shape);

				//btShapeHull* hull = &sc->shapeHull;
				btShapeHull* hull = (btShapeHull*)shape->getUserPointer();

				if ( hull )
				{
					//D3DXMATRIX matWorld = ConvertMatrix( trn );
						//D3DXMatrixMultiply(&matWorld, &matWorld, &mMatCompound);			
					//mDevice->SetTransform( D3DTS_WORLD, &matWorld );	
					//mDevice->SetFVF(D3DFVF_VERTEXNORMAL);

					int numTri =  hull->numTriangles();

					if( numTri > 0 )
					{
						int index = 0;
						const unsigned int* idx = hull->getIndexPointer();
						const btVector3* vtx = hull->getVertexPointer();						

						qPSDVertex *pVertex = new qPSDVertex[numTri * 3];

						for (int i = 0; i < hull->numTriangles (); i++)
						{
							int i1 = index++;
							int i2 = index++;
							int i3 = index++;
							btAssert(i1 < hull->numIndices () &&
								i2 < hull->numIndices () &&
								i3 < hull->numIndices ());

							int index1 = idx[i1];
							int index2 = idx[i2];
							int index3 = idx[i3];
							btAssert(index1 < hull->numVertices () &&
								index2 < hull->numVertices () &&
								index3 < hull->numVertices ());

							btVector3 v1 = vtx[index1];
							btVector3 v2 = vtx[index2];
							btVector3 v3 = vtx[index3];
							btVector3 normal = (v3-v1).cross(v1-v2);
							normal.normalize ();

							/*pVertex[i1].pos = D3DXVECTOR3(v1.x(), v1.y(), v1.z());
							pVertex[i2].pos = D3DXVECTOR3(v2.x(), v2.y(), v2.z());
							pVertex[i3].pos = D3DXVECTOR3(v3.x(), v3.y(), v3.z());

							pVertex[i1].norm = D3DXVECTOR3( normal.x(), normal.y(), normal.z() );
							pVertex[i2].norm = D3DXVECTOR3( normal.x(), normal.y(), normal.z() );
							pVertex[i3].norm = D3DXVECTOR3( normal.x(), normal.y(), normal.z() );*/
						}
						//mDevice->DrawPrimitiveUP(D3DPT_TRIANGLELIST, numTri, (LPVOID) pVertex, sizeof(VertexNormal));
						delete [] pVertex;

						mNumVertices += hull->numVertices();
						mNumFaces	+= hull->numTriangles();
						mNumObjects++;
					}
				}
			}			
		}
		break;
	}
}

void qPhysicsShapeDrawer::renderConcaveShapes(mat4 world, size_t index)
{
}

void qPhysicsShapeDrawer::addConcaveShape(btConcaveShape *shape)
{
	qConcaveShapeCache *cache = new qConcaveShapeCache();

	btVector3 worldMin( -10000, -10000, -10000 );
	btVector3 worldMax( 10000, 10000, 10000 );
	qDrawCallback drawCallback;	

	// Generates a disjointed triangle for each primitive
	shape->processAllTriangles(&drawCallback, worldMin, worldMax);	
	cache->numFaces = drawCallback.n_tris;	

	int size = drawCallback.n_verts * sizeof(qPSDVertex);
	/*mDevice->CreateVertexBuffer( size, 0, D3DFVF_VERTEXNORMAL, D3DPOOL_MANAGED, &cache->vertexBuffer, NULL );

	VOID* pVertices;
	cache->vertexBuffer->Lock( 0, size, (void**)&pVertices, 0 );	
	memcpy( pVertices, &drawCallback.vertices[0], size);
	cache->vertexBuffer->Unlock();*/

	this->mConcaveShapeCaches.push_back( cache );
}

qShapeCache* qPhysicsShapeDrawer::Cache(btConvexShape* shape)
{
	qShapeCache*		sc=(qShapeCache*)shape->getUserPointer();
	if(!sc)
	{
		sc=new(btAlignedAlloc(sizeof(qShapeCache),16)) qShapeCache(shape);
		sc->shapeHull.buildHull(shape->getMargin());
		this->mShapeCaches.push_back(sc);
		shape->setUserPointer(sc);
		/* Build edges	*/ 
		const int			ni=sc->shapeHull.numIndices();
		const int			nv=sc->shapeHull.numVertices();
		const unsigned int*	pi=sc->shapeHull.getIndexPointer();
		const btVector3*	pv=sc->shapeHull.getVertexPointer();
		btAlignedObjectArray<qShapeCache::Edge*>	edges;
		sc->edges.reserve(ni);
		edges.resize(nv*nv,0);
		for(int i=0;i<ni;i+=3)
		{
			const unsigned int* ti=pi+i;
			const btVector3		nrm=btCross(pv[ti[1]]-pv[ti[0]],pv[ti[2]]-pv[ti[0]]).normalized();
			for(int j=2,k=0;k<3;j=k++)
			{
				const unsigned int	a=ti[j];
				const unsigned int	b=ti[k];
				qShapeCache::Edge*&	e=edges[btMin(a,b)*nv+btMax(a,b)];
				if(!e)
				{
					sc->edges.push_back(qShapeCache::Edge());
					e=&sc->edges[sc->edges.size()-1];
					e->n[0]=nrm;e->n[1]=-nrm;
					e->v[0]=a;e->v[1]=b;
				}
				else
				{
					e->n[1]=nrm;
				}
			}
		}
	}
	return(sc);
}