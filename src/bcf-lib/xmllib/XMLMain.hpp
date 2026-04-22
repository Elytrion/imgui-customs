#pragma once

namespace XMLLib
{
void InitXMLLib();		// must be called at least once before using any other XMLLib functionality, to initialize the underlying Xerces library
void ReleaseXMLLib();	// must be called at least once at the end of using XMLLib functionality, to release the underlying Xerces library and free resources
bool IsXMLLibInit();	// returns whether the XMLLib and underlying Xerces library has been initialised
}