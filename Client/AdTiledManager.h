#ifndef __ADTILEDMANAGER_H_
#define __ADTILEDMANAGER_H_

#include "AdBase.h"

//-----------------------------------------------------------------------------
class AdTiledManager {
private:
	static duk_context* s_pJSCtx;
	static int          s_iInstances;

protected:
	int              m_iWidth;
	int              m_iHeight;
	int              m_nLayers;
	int**            m_pIndices;
	int              m_nEntities;
	class AdEntity** m_pEntities;

protected:
	void Unload(void);

public:
	int Width(void)     {return m_iWidth;}
	int Height(void)    {return m_iHeight;}
	int nLayers(void)   {return m_nLayers;}
	int nEntities(void) {return m_nEntities;}

public:
	void Load(const char* pName);
	void AddEntity(AdEntity* pEnt);

public:
	int* GetLayer(int l) {return m_pIndices[l];}

	int GetTile(int iLayer, int iIndex) {
		if(!m_pIndices        ||
			iIndex<0          ||
			iLayer<0          ||
			iLayer>=m_nLayers ||
			iIndex>=(m_iWidth*m_iHeight)
		) return -1;
		return m_pIndices[iLayer][iIndex];
	}

	int GetTile(int iLayer, int iX, int iY) {
		if(!m_pIndices   ||
			iX<0         ||
			iX>=m_iWidth ||
			iY<0         ||
			iY>=m_iHeight
		) return -1;
		int iIndex = (iY*m_iWidth)+iX;
		return GetTile(iLayer, iIndex);
	}

public:
	class AdEntity* GetEntity(int iEnt) {
		if(m_pEntities==NULL ||
			iEnt<0           ||
			iEnt>=m_nEntities
		) return NULL;
		return m_pEntities[iEnt];
	}

public:
	AdTiledManager(void);
	~AdTiledManager(void);
};

#endif
