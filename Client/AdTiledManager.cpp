/*
*/

#include "AdEntity.h"
#include "AdTiledManager.h"

//-----------------------------------------------------------------------------
duk_context* AdTiledManager::s_pJSCtx;
int          AdTiledManager::s_iInstances;

//-----------------------------------------------------------------------------
AdTiledManager::AdTiledManager(void) {
	if(s_iInstances == 0) {
		s_pJSCtx = duk_create_heap_default();

		if(s_pJSCtx == NULL) {
			fprintf(stderr, "ER: Failed to create Duktape heap!\n");
		}
	}

	m_iWidth    = 0;
	m_iHeight   = 0;
	m_nLayers   = 0;
	m_pIndices  = NULL;
	m_nEntities = 0;
	m_pEntities = NULL;

	s_iInstances++;
}

//-----------------------------------------------------------------------------
AdTiledManager::~AdTiledManager(void) {
	Unload();

	s_iInstances--;

	if(s_iInstances == 0) {
		duk_destroy_heap(s_pJSCtx);
	}
}

//-----------------------------------------------------------------------------
void AdTiledManager::Load(const char* pName) {
	Unload();

	char pFN[FILENAME_MAX];
	sprintf(pFN, MAP_LOCATION, pName);

	duk_context* ctx = s_pJSCtx;

	duk_push_string_file(ctx, pFN);
	duk_json_decode(ctx, -1);

	if(!duk_is_object(ctx, -1)) {
		fprintf(stderr, "ER: Failed parse %s.json!\n", pName);
		return;
	}
	
	duk_get_prop_string(ctx, -1, "width");
	m_iWidth = duk_to_int(ctx, -1);
	duk_pop(ctx);

	duk_get_prop_string(ctx, -1, "height");
	m_iHeight = duk_to_int(ctx, -1);
	duk_pop(ctx);

	duk_get_prop_string(ctx, -1, "layers");
	
	if(duk_is_array(ctx, -1)) {
		m_nLayers  = duk_get_length(ctx, -1);
		m_pIndices = (int**) calloc(m_nLayers, sizeof(int*));

		for(int j=0; j<m_nLayers; ++j) {
			duk_get_prop_index(ctx, -1, j);

			// NOTE: if this is a tile layer grab the indices, otherwise set
			// to NULL
			duk_get_prop_string(ctx, -1, "data");

			if(duk_is_array(ctx, -1)) {
				int size = duk_get_length(ctx, -1);

				if((m_iWidth*m_iHeight) == size) {
					m_pIndices[j] = (int*) malloc(size*sizeof(int));

					for(int i=0; i<size; ++i) {
						duk_get_prop_index(ctx, -1, i);
						m_pIndices[j][i] = duk_to_int(ctx, -1);
						duk_pop(ctx);
					}
				}
			}

			duk_pop(ctx);

			// NOTE: loop over the map entities and add them to the array of
			// entities
			duk_get_prop_string(ctx, -1, "name");

			if(duk_is_string(ctx, -1)) {
				if(!strcmp(duk_get_string(ctx, -1), "entities")) {
					// NOTE: -2 referring back to object and not the string
					duk_get_prop_string(ctx, -2, "objects");

					if(duk_is_array(ctx, -1)) {
						int size = duk_get_length(ctx, -1);
						
						for(int e=0; e<size; ++e) {
							duk_get_prop_index(ctx, -1, e);

							duk_get_prop_string(ctx, -1, "type");
							const char* type = duk_get_string(ctx, -1);

							AdEntity* pEnt = NULL;

							if(
								!strcmp(type, "LVL-UP")    ||
								!strcmp(type, "LVL-DOWN")  ||
								!strcmp(type, "LVL-LEFT")  ||
								!strcmp(type, "LVL-RIGHT")
							) {
								duk_pop(ctx);
								pEnt = new AdEntity();
								pEnt->Load(ctx);
							} else {
								duk_pop(ctx);
							}

							if(pEnt) {
								m_pEntities = (AdEntity**) realloc(
									m_pEntities, ++m_nEntities*sizeof(AdEntity*)
								); m_pEntities[m_nEntities-1] = pEnt;
							}

							duk_pop(ctx);
						}
					}

					duk_pop(ctx);
				}
			}

			duk_pop_2(ctx);
		}
	}

	duk_pop_2(ctx);
}

//-----------------------------------------------------------------------------
void AdTiledManager::Unload(void) {
	if(m_pIndices && m_nLayers>0) {
		for(int j=0; j<m_nLayers; ++j) {
			if(m_pIndices[j] == NULL) continue;

			free(m_pIndices[j]);
			m_pIndices[j] = NULL;
		}

		free(m_pIndices);
		m_pIndices = NULL;
	}

	m_iWidth  = 0;
	m_iHeight = 0;
	m_nLayers = 0;

	if(m_pEntities && m_nEntities>0) {
		for(int e=0; e<m_nEntities; ++e) {
			if(m_pEntities[e] == NULL) continue;

			delete m_pEntities[e];
			m_pEntities[e] = NULL;
		}

		free(m_pEntities);
		m_pEntities = NULL;
	}

	m_nEntities = 0;
}

//-----------------------------------------------------------------------------
void AdTiledManager::AddEntity(AdEntity* pEnt) {
	if(pEnt) {
		m_pEntities = (AdEntity**) realloc(
			m_pEntities, ++m_nEntities*sizeof(AdEntity*)
		); m_pEntities[m_nEntities-1] = pEnt;
	}
}
