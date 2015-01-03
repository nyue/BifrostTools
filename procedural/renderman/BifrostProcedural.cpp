#include <ri.h>
#include <stdlib.h>
#include <iostream>

void EmitGeometry(float radius)
{
  RiSphere(radius,-radius,radius,360.0f,RI_NULL);
}

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

/*

  Do these as part of linking stage via CMake
  configuration

#ifdef _WIN32
#define EXTERN extern _declspec(dllexport)
#else
#define EXTERN extern
#endif

EXTERN RtPointer ConvertParameters(RtString paramstr);
EXTERN RtVoid Subdivide(RtPointer data, RtFloat detail);
EXTERN RtVoid Free(RtPointer data);
*/

RtPointer ConvertParameters(RtString paramstr)
{
  RtFloat *radius_p;

  /* convert the string to a float */
  radius_p = (RtFloat *)malloc(sizeof(RtFloat));
  *radius_p = (RtFloat)atof(paramstr);

  /* return the blind data pointer */
  return (RtPointer)radius_p;
}

RtVoid Subdivide(RtPointer data, RtFloat detail)
{
  RtFloat *radius_p = (RtFloat *)data;
  EmitGeometry(2.0f); // RiSphere(1.0f,-1.0f,1.0f,360.0f,RI_NULL);
  fprintf(stderr,"Debugging\n");
}

RtVoid Free(RtPointer data)
{
  free(data);
}

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif
