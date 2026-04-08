// The following ifdef block is the standard way of creating macros which make exporting
// from a DLL simpler. All files within this DLL are compiled with the FLIGHTMODEL_EXPORTS
// symbol defined on the command line. This symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see
// FLIGHTMODEL_API functions as being imported from a DLL, whereas this DLL sees symbols
// defined with this macro as being exported.
#ifdef FLIGHTMODEL_EXPORTS
#define FLIGHTMODEL_API __declspec(dllexport)
#else
#define FLIGHTMODEL_API __declspec(dllimport)
#endif

#include "shared_memory.h"

// This class is exported from the dll
class FLIGHTMODEL_API CFlightModel {
public:
	CFlightModel(void);
};

extern FLIGHTMODEL_API int nFlightModel;

FLIGHTMODEL_API void update_model(SharedState* s, float dt);
