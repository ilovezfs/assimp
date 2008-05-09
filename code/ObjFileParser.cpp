#include "ObjFileParser.h"
#include "ObjTools.h"
#include "ObjFileData.h"
#include "DefaultIOSystem.h"
#include "IOStream.h"
#include "aiTypes.h"
#include "aiVector3D.h"
#include "aiAssert.h"
#include "fast_atof.h"

#include <iostream>
#include <vector>
#include <cassert>

namespace Assimp
{

// -------------------------------------------------------------------
ObjFileParser::ObjFileParser(std::vector<char> &Data, const std::string &strAbsPath, const std::string &strModelName) :
	m_strAbsPath(strAbsPath),
	m_DataIt(Data.begin()),
	m_DataItEnd(Data.end()),
	m_pModel(NULL),
	m_uiLine(0)
{
	// Create the model instance to store all the data
	m_pModel = new ObjFile::Model();
	m_pModel->m_ModelName = strModelName;
		
	// Start parsing the file
	parseFile();
}

// -------------------------------------------------------------------
ObjFileParser::~ObjFileParser()
{
	// empty
}

// -------------------------------------------------------------------
ObjFile::Model *ObjFileParser::GetModel() const
{
	return m_pModel;
}

// -------------------------------------------------------------------
void ObjFileParser::parseFile()
{
	if (m_DataIt == m_DataItEnd)
		return;

	while (m_DataIt != m_DataItEnd)
	{
		switch (*m_DataIt)
		{
		case 'v': // Parse a vertex texture coordinate
			{
				++m_DataIt;
				if (*m_DataIt == ' ')
				{
					// Read in vertex definition
					getVector3(m_pModel->m_Vertices);
				}
				else if (*m_DataIt == 't')
				{
					// Read in texture coordinate (2D)
					getVector2(m_pModel->m_TextureCoord);
				}
				else if (*m_DataIt == 'n')
				{
					// Read in normal vector definition
					getVector3(m_pModel->m_Normals);
				}
			}
			break;

		case 'f': // Parse a face
			{
				getFace();
			}
			break;

		case '#': // Parse a comment
			{
				getComment();
			}
			break;

		case 'u': // Parse a material desc. setter
			{
				getMaterialDesc();
			}
			break;

		case 'm': // Parse a material library
			{
				getMaterialLib();
			}
			break;

		case 'g': // Parse group name
			{
				getGroupName();
			}
			break;

		case 's': // Parse group number
			{
				getGroupNumber();
			}
			break;

		case 'o': // Parse object name
			{
				getObjectName();
			}
			break;
		
		default:
			{
				skipLine();
			}
			break;
		}
	}
}

// -------------------------------------------------------------------
//	Copy the next word in a temporary buffer
void ObjFileParser::copyNextWord(char *pBuffer, size_t length)
{
	size_t index = 0;
	m_DataIt = getNextWord<DataArrayIt>(m_DataIt, m_DataItEnd);
	while (!isSpace(*m_DataIt) && m_DataIt != m_DataItEnd)
	{
		pBuffer[index] = *m_DataIt;
		index++;
		if (index == length-1)
			break;
		++m_DataIt;
	}
	pBuffer[index] = '\0';
}

// -------------------------------------------------------------------
// Copy the next line into a temporary buffer
void ObjFileParser::copyNextLine(char *pBuffer, size_t length)
{
	size_t index = 0;
	while (m_DataIt != m_DataItEnd)
	{
		if (*m_DataIt == '\n' || *m_DataIt == '\r')
			break;
		assert (index+1 <= length);
		pBuffer[ index ] = *m_DataIt;
		++index;
		++m_DataIt;
	}
	pBuffer[ index ] = '\0';
}

// -------------------------------------------------------------------
//	Get values for a new 3D vector instance
void ObjFileParser::getVector3(std::vector<aiVector3D*> &point3d_array)
{
	float x, y, z;
	copyNextWord(m_buffer, BUFFERSIZE);
	x = (float) fast_atof(m_buffer);	
	
	copyNextWord(m_buffer, BUFFERSIZE);
	y = (float) fast_atof(m_buffer);

	copyNextWord(m_buffer, BUFFERSIZE);
	z = (float) fast_atof(m_buffer);

	point3d_array.push_back(new aiVector3D(x,y,z));
	skipLine();
}

// -------------------------------------------------------------------
//	Get values for a new 2D vector instance
void ObjFileParser::getVector2(std::vector<aiVector2D*> &point2d_array)
{
	float x, y;
	copyNextWord(m_buffer, BUFFERSIZE);
	x = (float) fast_atof(m_buffer);	
	
	copyNextWord(m_buffer, BUFFERSIZE);
	y = (float) fast_atof(m_buffer);

	point2d_array.push_back(new aiVector2D(x, y));
	skipLine();
}

// -------------------------------------------------------------------
//	Skips a line
void ObjFileParser::skipLine()
{
	while (m_DataIt != m_DataItEnd && *m_DataIt != '\n')
		++m_DataIt;
	if (m_DataIt != m_DataItEnd)
	{
		++m_DataIt;
		++m_uiLine;
	}
}

// -------------------------------------------------------------------
//	Get values for a new face instance
void ObjFileParser::getFace()
{
	copyNextLine(m_buffer, BUFFERSIZE);
	if (m_DataIt == m_DataItEnd)
		return;
	char *pPtr = m_buffer;
	char *pEnd = &pPtr[BUFFERSIZE];
	pPtr = getNextToken<char*>(pPtr, pEnd);
	if (pPtr == '\0')
		return;

	std::vector<unsigned int> *pIndices = new std::vector<unsigned int>;
	std::vector<unsigned int> *pTexID = new std::vector<unsigned int>;
	std::vector<unsigned int> *pNormalID = new std::vector<unsigned int>;
	bool vt = (!m_pModel->m_TextureCoord.empty());
	bool vn = (!m_pModel->m_Normals.empty());
	int iStep = 0, iPos = 0;
	while (pPtr != pEnd)
	{
		iStep = 1;
		if (*pPtr == '\0')
			break;

		if (*pPtr=='\r')
			break;

		if (*pPtr=='/' )
		{
			if (iPos == 0)
			{
				//if there are no texturecoordinates in the obj file but normals
				if (!vt && vn)
					iPos = 1;
			}
			iPos++;
		}
		else if (isSpace(*pPtr))
		{
			iPos = 0;
		}
		else 
		{
			//OBJ USES 1 Base ARRAYS!!!!
			const int iVal = atoi(pPtr);
			int tmp = iVal;
			while ((tmp = tmp / 10)!=0)
				++iStep;

			if (0 != iVal)
			{
				// Store parsed index
				if (0 == iPos)
				{
					pIndices->push_back(iVal-1);
				}
				else if (1 == iPos)
				{	
					pTexID->push_back(iVal-1);
				}
				else if (2 == iPos)
				{
					pNormalID->push_back(iVal-1);
				}
				else
				{
					reportErrorTokenInFace();
				}
			}
		}
		for (int i=0; i<iStep; i++)
			++pPtr;
	}

	ObjFile::Face *face = new ObjFile::Face(pIndices, pNormalID, pTexID);
	
	// Set active material, if one set
	if (NULL != m_pModel->m_pCurrentMaterial) 
		face->m_pMaterial = m_pModel->m_pCurrentMaterial;
	else 
		face->m_pMaterial = m_pModel->m_pDefaultMaterial;

	// Create a default object, if nothing there
	if (NULL == m_pModel->m_pCurrent)
		createObject("defaultobject");

	// Store the new instance
	m_pModel->m_pCurrent->m_Faces.push_back(face);
	
	// Skip the rest of the line
	skipLine();
}

// -------------------------------------------------------------------
//	Get values for a new material description
void ObjFileParser::getMaterialDesc()
{
	// Get next data for material data
	m_DataIt = getNextToken<DataArrayIt>(m_DataIt, m_DataItEnd);
	if (m_DataIt == m_DataItEnd)
		return;

	char *pStart = &(*m_DataIt);
	while (!isSpace(*m_DataIt) && m_DataIt != m_DataItEnd)
		m_DataIt++;

	// Get name
	std::string strName(pStart, &(*m_DataIt));
	if (strName.empty())
		return;

	// Search for material
	std::string strFile;
	std::map<std::string, ObjFile::Material*>::iterator it = m_pModel->m_MaterialMap.find(strName);
	if (it == m_pModel->m_MaterialMap.end())
	{
		m_pModel->m_pCurrentMaterial = new ObjFile::Material();
		m_pModel->m_MaterialMap[strName] = m_pModel->m_pCurrentMaterial;
	}

	skipLine();
}

// -------------------------------------------------------------------
//	Get a comment, values will be skipped
void ObjFileParser::getComment()
{
	while (true)
	{
		if ('\n' == (*m_DataIt) || m_DataIt == m_DataItEnd) 
		{
			++m_DataIt;
			break;
		}
		else
		{
			++m_DataIt;
		}
	}
}

// -------------------------------------------------------------------
//	
void ObjFileParser::getMaterialLib()
{
	// Translate tuple
	m_DataIt = getNextToken<DataArrayIt>(m_DataIt, m_DataItEnd);
	if (m_DataIt ==  m_DataItEnd)
		return;
	
	char *pStart = &(*m_DataIt);
	while (!isSpace(*m_DataIt))
		m_DataIt++;
	
	// Check for existence
	DefaultIOSystem IOSystem;
	std::string strMatName(pStart, &(*m_DataIt));
	std::string absName = m_strAbsPath + IOSystem.getOsSeparator() + strMatName;
	if (!IOSystem.Exists(absName))
	{
		skipLine();
		return;
	}

	std::string strExt("");
	extractExtension(strMatName, strExt);
	std::string mat = "mtl";

	DefaultIOSystem FileSystem;
	IOStream *pFile = FileSystem.Open(absName);
	if (0L != pFile)
	{
		size_t size = pFile->FileSize();
		char *pBuffer = new char[size];
		size_t read_size = pFile->Read(pBuffer, sizeof(char), size);
		FileSystem.Close(pFile);

		// TODO: Load mtl file
		
		delete [] pBuffer;
	
	}

	// Load material library (all materials will be created)
	m_pModel->m_MaterialLib.push_back(strMatName);
	
	skipLine();
}

// -------------------------------------------------------------------
//	
void ObjFileParser::getNewMaterial()
{
	m_DataIt = getNextToken<DataArrayIt>(m_DataIt, m_DataItEnd);
	m_DataIt = getNextWord<DataArrayIt>(m_DataIt, m_DataItEnd);

	char *pStart = &(*m_DataIt);
	std::string strMat(pStart, *m_DataIt);
	while (isSpace(*m_DataIt))
		m_DataIt++;
	std::map<std::string, ObjFile::Material*>::iterator it = m_pModel->m_MaterialMap.find(strMat);
	if (it == m_pModel->m_MaterialMap.end())
	{
		// Show a warning, if material was not found
		std::string strWarn ("Unsupported material requested: ");
		strWarn += strMat;
		std::cerr << "Warning : " << strWarn << std::endl;
		m_pModel->m_pCurrentMaterial = m_pModel->m_pDefaultMaterial;
	}
	else
	{
		// Set new material
		m_pModel->m_pCurrentMaterial = (*it).second;
	}

	skipLine();
}

// -------------------------------------------------------------------
//	
void ObjFileParser::getGroupName()
{
	// Get next word from data buffer
	m_DataIt = getNextToken<DataArrayIt>(m_DataIt, m_DataItEnd);
	m_DataIt = getNextWord<DataArrayIt>(m_DataIt, m_DataItEnd);
	
	// Store groupname in group library 
	char *pStart = &(*m_DataIt);
	while (!isSpace(*m_DataIt))
		m_DataIt++;
	std::string strGroupName(pStart, &(*m_DataIt));

	// Change active group, if necessary
	if (m_pModel->m_strActiveGroup != strGroupName)
	{
		// Search for already existing entry
		ObjFile::Model::ConstGroupMapIt it = m_pModel->m_Groups.find(&strGroupName);
		
		// New group name, creating a new entry
		ObjFile::Object *pObject = m_pModel->m_pCurrent;
		if (it == m_pModel->m_Groups.end())
		{
			std::vector<unsigned int> *pFaceIDArray = new std::vector<unsigned int>;
			m_pModel->m_Groups[ &strGroupName ] = pFaceIDArray;
			m_pModel->m_pGroupFaceIDs = (pFaceIDArray);
		}
		else
		{
			m_pModel->m_pGroupFaceIDs = (*it).second;
		}
		m_pModel->m_strActiveGroup = strGroupName;
	}
	skipLine();
}

// -------------------------------------------------------------------
//	Not supported
void ObjFileParser::getGroupNumber()
{
	// TODO: Implement this

	skipLine();
}

// -------------------------------------------------------------------
//	Stores values for a new object instance, name will be used to 
//	identify it.
void ObjFileParser::getObjectName()
{
	m_DataIt = getNextToken<DataArrayIt>(m_DataIt, m_DataItEnd);
	if (m_DataIt == m_DataItEnd)
		return;
	char *pStart = &(*m_DataIt);
	while (!isSpace(*m_DataIt))
		m_DataIt++;

	std::string strObjectName(pStart, &(*m_DataIt));
	if (!strObjectName.empty()) 
	{
		// Reset current object
		m_pModel->m_pCurrent = NULL;
		
		// Search for actual object
		for (std::vector<ObjFile::Object*>::const_iterator it = m_pModel->m_Objects.begin();
			it != m_pModel->m_Objects.end();
			++it)
		{
			if ((*it)->m_strObjName == strObjectName)
			{
				m_pModel->m_pCurrent = *it;
				break;
			}
		}

		// Allocate a new object, if current one wasn�t found before
		if (m_pModel->m_pCurrent == NULL)
		{
			createObject(strObjectName);
			/*m_pModel->m_pCurrent = new ObjFile::Object();
			m_pModel->m_pCurrent->m_strObjName = strObjectName;
			m_pModel->m_Objects.push_back(m_pModel->m_pCurrent);*/
		}
	}
	skipLine();
}
// -------------------------------------------------------------------
//	Creates a new object instance
void ObjFileParser::createObject(const std::string &strObjectName)
{
	ai_assert (NULL != m_pModel);
	ai_assert (!strObjectName.empty());

	m_pModel->m_pCurrent = new ObjFile::Object();
	m_pModel->m_pCurrent->m_strObjName = strObjectName;
	m_pModel->m_Objects.push_back(m_pModel->m_pCurrent);
}

// -------------------------------------------------------------------
//	Shows an error in parsing process.
void ObjFileParser::reportErrorTokenInFace()
{		
	std::string strErr("");
	skipLine();
	std::cerr <<  "Not supported token in face desc. detected : " << strErr << std::endl;
}

// -------------------------------------------------------------------
//	Extracts the extention from a filename
void ObjFileParser::extractExtension(const std::string strFile, 
									 std::string &strExt)
{
	strExt = "";
	if (strFile.empty())
		return;

	std::string::size_type pos = strFile.find_last_of(".");
	if (pos == std::string::npos)
		return;
	strExt = strFile.substr(pos, strFile.size() - pos);
}
// -------------------------------------------------------------------

}	// Namespace Assimp
