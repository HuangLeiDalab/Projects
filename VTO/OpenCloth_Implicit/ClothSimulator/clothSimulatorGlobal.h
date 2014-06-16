#ifndef _CLOTHSIMULATORGLOBAL_PLUGIN_H_
#define _CLOTHSIMULATORGLOBAL_PLUGIN_H_

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp> //for matrices
#include <glm/gtc/type_ptr.hpp>


#define CLOTHSIMULATORPLUGIN_LIB

#if defined (CLOTHSIMULATORPLUGIN_LIB)
# define _DLLExport __declspec (dllexport)  
# else  
# define _DLLExport __declspec (dllimport)  
#endif 


#endif