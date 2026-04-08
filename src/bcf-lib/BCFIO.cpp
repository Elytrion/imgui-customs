#include "BCFIO.h"
#include <xercesc/dom/DOM.hpp>

BCFRaw BCFIO::LoadBCF(const std::string& filePath, std::string& errMsg)
{
	// check if file has valid extension (.bcf or .bcfzip)

	// unzip using libzip, this will also check if the file is a valid zip archive

	// look for and read .version file, parse as XML using pugixml, and check if valid

	// look for .bcfp file, if it exists, read and parse as XML

	// for each folder in the archive that matches the topic folder pattern
	   // read the markup XML file, parse as XML

	return {};
}