#ifndef _CLOTH_PLUGIN_UTILS_H_
#define _CLOTH_PLUGIN_UTILS_H_

#include <glm/glm.hpp>

#define CLOTHPLUGIN_LIB

#if defined (CLOTHPLUGIN_LIB)
# define _DLLExport __declspec (dllexport)  
# else  
# define _DLLExport __declspec (dllimport)  
#endif 


#endif