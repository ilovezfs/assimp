/*
---------------------------------------------------------------------------
Free Asset Import Library (ASSIMP)
---------------------------------------------------------------------------

Copyright (c) 2006-2008, ASSIMP Development Team

All rights reserved.

Redistribution and use of this software in source and binary forms, 
with or without modification, are permitted provided that the following 
conditions are met:

* Redistributions of source code must retain the above
  copyright notice, this list of conditions and the
  following disclaimer.

* Redistributions in binary form must reproduce the above
  copyright notice, this list of conditions and the
  following disclaimer in the documentation and/or other
  materials provided with the distribution.

* Neither the name of the ASSIMP team, nor the names of its
  contributors may be used to endorse or promote products
  derived from this software without specific prior
  written permission of the ASSIMP Development Team.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS 
"AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT 
LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT 
OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT 
LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY 
THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT 
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE 
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
---------------------------------------------------------------------------
*/

/** @file Defines the data structures in which the imported geometry is 
    returned by ASSIMP */
#ifndef AI_MESH_H_INC
#define AI_MESH_H_INC

#include "aiTypes.h"

#ifdef __cplusplus
extern "C" {
#endif

// ---------------------------------------------------------------------------
/** A single face in a mesh, referring to multiple vertices. 
*
* If mNumIndices is 3, the face is a triangle, 
* for mNumIndices > 3 it's a polygon.
* Point and line primitives are rarely used and are NOT supported. However,
* a load could pass them as degenerated triangles.
*/
// ---------------------------------------------------------------------------
struct aiFace
{
	//! Number of indices defining this face. 3 for a triangle, >3 for polygon
	unsigned int mNumIndices; 

	//! Pointer to the indices array. Size of the array is given in numIndices.
	unsigned int* mIndices;   

#ifdef __cplusplus

	//! Default constructor
	aiFace()
	{
		mNumIndices = 0; mIndices = NULL;
	}

	//! Default destructor. Delete the index array
	~aiFace()
	{
		delete [] mIndices;
	}

	//! Copy constructor. Copy the index array
	aiFace( const aiFace& o)
	{
		mIndices = NULL;
		*this = o;
	}

	//! Assignment operator. Copy the index array
	const aiFace& operator = ( const aiFace& o)
	{
		if (&o == this)
			return *this;

		delete mIndices;
		mNumIndices = o.mNumIndices;
		mIndices = new unsigned int[mNumIndices];
		memcpy( mIndices, o.mIndices, mNumIndices * sizeof( unsigned int));
		return *this;
	}

	//! Comparison operator. Checks whether the index array 
	//! of two faces is identical
	bool operator== (const aiFace& o) const
	{
		if (this->mIndices == o.mIndices)return true;
		else if (this->mIndices && this->mNumIndices == o.mNumIndices)
		{
			for (unsigned int i = 0;i < this->mNumIndices;++i)
			{
				if (this->mIndices[i] != o.mIndices[i])return false;
			}
			return true;
		}
		return false;
	}

	//! Inverse comparison operator. Checks whether the index 
	//! array of two faces is NOT identical
	bool operator != (const aiFace& o) const
	{
		return !(*this == o);
	}

#endif // __cplusplus
};


// ---------------------------------------------------------------------------
/** A single influence of a bone on a vertex.
 */
// ---------------------------------------------------------------------------
struct aiVertexWeight
{
	//! Index of the vertex which is influenced by the bone.
	unsigned int mVertexId;

	//! The strength of the influence in the range (0...1).
	//! The influence from all bones at one vertex amounts to 1.
	float mWeight;     

#ifdef __cplusplus

	//! Default constructor
	aiVertexWeight() { }

	//! Initialisation from a given index and vertex weight factor
	//! \param pID ID
	//! \param pWeight Vertex weight factor
	aiVertexWeight( unsigned int pID, float pWeight) 
		: mVertexId( pID), mWeight( pWeight) 
	{ /* nothing to do here */ }

#endif // __cplusplus
};


// ---------------------------------------------------------------------------
/** A single bone of a mesh. A bone has a name by which it can be found 
* in the frame hierarchy and by which it can be addressed by animations. 
* In addition it has a number of influences on vertices.
*/
// ---------------------------------------------------------------------------
struct aiBone
{
	//! The name of the bone. 
	C_STRUCT aiString mName;

	//! The number of vertices affected by this bone
	unsigned int mNumWeights;

	//! The vertices affected by this bone
	C_STRUCT aiVertexWeight* mWeights;

	//! Matrix that transforms from mesh space to bone space in bind pose
	C_STRUCT aiMatrix4x4 mOffsetMatrix;

#ifdef __cplusplus

	//! Default constructor
	aiBone()
	{
		mNumWeights = 0; mWeights = NULL;
	}

	//! Destructor to delete the array of vertex weights
	~aiBone()
	{
		delete [] mWeights;
	}
#endif // __cplusplus
};

#if (!defined AI_MAX_NUMBER_OF_COLOR_SETS)

/** Maximum number of vertex color sets per mesh.
*
* Normally: Diffuse, specular, ambient and emissive
* However, one could use the vertex color sets for any other purpose, too.
*
* \note Some internal structures expect (and assert) this value
*   to be at least 4
*/
#define AI_MAX_NUMBER_OF_COLOR_SETS 0x4

#endif // !! AI_MAX_NUMBER_OF_COLOR_SETS
#if (!defined AI_MAX_NUMBER_OF_TEXTURECOORDS)

/** Maximum number of texture coord sets (UV(W) channels) per mesh 
*
* The material system uses the AI_MATKEY_UVWSRC_XXX keys to specify 
* which UVW channel serves as data source for a texture,
*
* \note Some internal structures expect (and assert) this value
*   to be at least 4
*/
#define AI_MAX_NUMBER_OF_TEXTURECOORDS 0x4

#endif // !! AI_MAX_NUMBER_OF_TEXTURECOORDS

// ---------------------------------------------------------------------------
/** A mesh represents a geometry or model with a single material. 
*
* It usually consists of a number of vertices and a series of primitives/faces 
* referencing the vertices. In addition there might be a series of bones, each 
* of them addressing a number of vertices with a certain weight. Vertex data 
* is presented in channels with each channel containing a single per-vertex 
* information such as a set of texture coords or a normal vector.
* If a data pointer is non-null, the corresponding data stream is present.
* From C++-programs you can also use the comfort functions Has*() to
* test for the presence of various data streams.
*
* A Mesh uses only a single material which is referenced by a material ID.
* \note The mPositions member is not optional, although a Has()-Method is
* provided for it.
*/
struct aiMesh
{
	/** The number of vertices in this mesh. 
	* This is also the size of all of the per-vertex data arrays
	*/
	unsigned int mNumVertices;

	/** The number of primitives (triangles, polygones, lines) in this  mesh. 
	* This is also the size of the mFaces array 
	*/
	unsigned int mNumFaces;

	/** Vertex positions. 
	* This array is always present in a mesh. The array is 
	* mNumVertices in size. 
	*/
	C_STRUCT aiVector3D* mVertices;

	/** Vertex normals. 
	* The array contains normalized vectors, NULL if not present. 
	* The array is mNumVertices in size. 
	*/
	C_STRUCT aiVector3D* mNormals;

	/** Vertex tangents. 
	* The tangent of a vertex points in the direction of the positive 
	* X texture axis. The array contains normalized vectors, NULL if
	* not present. The array is mNumVertices in size. 
	* @note If the mesh contains tangents, it automatically also 
	* contains bitangents. 
	*/
	C_STRUCT aiVector3D* mTangents;

	/** Vertex bitangents. 
	* The bitangent of a vertex points in the direction of the positive 
	* Y texture axis. The array contains normalized vectors, NULL if not
	* present. The array is mNumVertices in size. 
	* @note If the mesh contains tangents, it automatically also contains
	* bitangents. 
	*/
	C_STRUCT aiVector3D* mBitangents;

	/** Vertex color sets. 
	* A mesh may contain 0 to #AI_MAX_NUMBER_OF_COLOR_SETS vertex 
	* colors per vertex. NULL if not present. Each array is
	* mNumVertices in size if present.
	*/
	C_STRUCT aiColor4D* mColors[AI_MAX_NUMBER_OF_COLOR_SETS];

	/** Vertex texture coords, also known as UV channels.
	* A mesh may contain 0 to AI_MAX_NUMBER_OF_TEXTURECOORDS per
	* vertex. NULL if not present. The array is mNumVertices in size. 
	*/
	C_STRUCT aiVector3D* mTextureCoords[AI_MAX_NUMBER_OF_TEXTURECOORDS];

	/** Specifies the number of components for a given UV channel.
	* Up to three channels are supported (UVW, for accessing volume
	* or cube maps). If the value is 2 for a given channel n, the
	* component p.z of mTextureCoords[n][p] is set to 0.0f.
	* If the value is 1 for a given channel, p.y is set to 0.0f, too.
	* @note 4D coords are not supported 
	*/
	unsigned int mNumUVComponents[AI_MAX_NUMBER_OF_TEXTURECOORDS];

	/** The faces the mesh is contstructed from. 
	* Each face referres to a number of vertices by their indices. 
	* This array is always present in a mesh, its size is given 
	* in mNumFaces.
	*/
	C_STRUCT aiFace* mFaces;

	/** The number of bones this mesh contains. 
	* Can be 0, in which case the mBones array is NULL. 
	*/
	unsigned int mNumBones;

	/** The bones of this mesh. 
	* A bone consists of a name by which it can be found in the
	* frame hierarchy and a set of vertex weights.
	*/
	C_STRUCT aiBone** mBones;

	/** The material used by this mesh. 
	 * A mesh does use only a single material. If an imported model uses multiple materials,
	 * the import splits up the mesh. Use this value as index into the scene's material list.
	 */
	unsigned int mMaterialIndex;

#ifdef __cplusplus
	aiMesh()
	{
		mNumVertices = 0; mNumFaces = 0;
		mVertices = NULL; mFaces = NULL;
		mNormals = NULL; mTangents = NULL;
		mBitangents = NULL;
		for( unsigned int a = 0; a < AI_MAX_NUMBER_OF_TEXTURECOORDS; a++)
		{
			mNumUVComponents[a] = 0;
			mTextureCoords[a] = NULL;
		}
		for( unsigned int a = 0; a < AI_MAX_NUMBER_OF_COLOR_SETS; a++)
			mColors[a] = NULL;
		mNumBones = 0; mBones = NULL;
		mMaterialIndex = 0;
	}

	~aiMesh()
	{
		delete [] mVertices; 
		delete [] mFaces;
		delete [] mNormals;
		delete [] mTangents;
		delete [] mBitangents;
		for( unsigned int a = 0; a < AI_MAX_NUMBER_OF_TEXTURECOORDS; a++)
			delete [] mTextureCoords[a];
		for( unsigned int a = 0; a < AI_MAX_NUMBER_OF_COLOR_SETS; a++)
			delete [] mColors[a];
		for( unsigned int a = 0; a < mNumBones; a++)
			delete mBones[a];
		delete [] mBones;
	}

	//! Check whether the mesh contains positions. Should always return true
	inline bool HasPositions() const 
		{ return mVertices != NULL; }

	//! Check whether the mesh contains normal vectors
	inline bool HasNormals() const 
		{ return mNormals != NULL; }

	//! Check whether the mesh contains tangent and bitangent vectors
	inline bool HasTangentsAndBitangents() const 
		{ return mTangents != NULL && mBitangents != NULL; }

	//! Check whether the mesh contains a vertex color set
	//! \param pIndex Index of the vertex color set
	inline bool HasVertexColors( unsigned int pIndex) 
	{ 
		if( pIndex >= AI_MAX_NUMBER_OF_COLOR_SETS) 
			return false; 
		else 
			return mColors[pIndex] != NULL; 
	}

	//! Check whether the mesh contains a texture coordinate set
	//! \param pIndex Index of the texture coordinates set
	inline bool HasTextureCoords( unsigned int pIndex) 
	{ 
		if( pIndex > AI_MAX_NUMBER_OF_TEXTURECOORDS) 
			return false; 
		else 
			return mTextureCoords[pIndex] != NULL; 
	}
	//! Check whether the mesh contains bones
	inline bool HasBones() const
		{ return mBones != NULL; }

#endif // __cplusplus
};

#ifdef __cplusplus
}
#endif

#endif // AI_MESH_H_INC