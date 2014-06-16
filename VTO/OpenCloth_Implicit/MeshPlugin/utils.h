#ifndef _MESH_PLUGIN_UTILS_H_
#define _MESH_PLUGIN_UTILS_H_

#include <glm/glm.hpp>

#define MESHPLUGIN_LIB

#if defined (MESHPLUGIN_LIB)
# define _DLLExport __declspec (dllexport)  
# else  
# define _DLLExport __declspec (dllimport)  
#endif 


#endif