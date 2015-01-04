#ifndef MAYAUTILS_H
#define MAYAUTILS_H
#include <maya/MGlobal.h>

#define DME(statement)                                                                     \
	{																	                   \
		char maya_error_message_buffer[BUFSIZ];							                   \
		sprintf(maya_error_message_buffer,"%s - %s, %d\n",#statement, __FILE__, __LINE__); \
		MGlobal::displayError(maya_error_message_buffer);				                   \
		MGlobal::displayError(status.errorString());					                   \
	}

#define CMS(statement)							\
	statement;									\
	if ( status != MS::kSuccess ) {				\
		DME(statment);							\
		return status;							\
	}

#define CMSV(statement)							\
	statement;									\
	if ( status != MS::kSuccess ) {				\
		DME(statment);							\
		return;									\
	}

#endif // MAYAUTILS_H
