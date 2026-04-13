#include "BCFIO.h"
#include "xmllib/XMLParser.h"
#include <filesystem>
#include <iostream>

namespace fs = std::filesystem;

BCFDocument BCFIO::Parse(const std::string& bcfFilePath, std::string& errMsg, const ParseConfig& cfg)
{
	BCFDocument result;
	// check if valid BCF file
	// > has extension .bcf or .bcfzip
	// > is a zip file
	const fs::path bcfPath{ bcfFilePath };
	if (!bcfPath.has_extension())
	{
		errMsg = "Invalid file path: No extension";
		return result;
	}
	if (bcfPath.extension() != ".bcf" && bcfPath.extension() != ".bcfzip")
	{
		errMsg = "Invalid file path: Wrong extension (must be .bcf or .bcfzip)";
		return result;
	}





	// unzip file to temporary location
	// validate .version file
	// load into document store if there
	// load optional files into document store

	// check each subdirectory
	// if subdirectory name matches guid format, save guid as a new topic
	// validate .bcf markup file and load into document store
	// for each .bcfv file, load into document store
	// for each .png/.jpg/.jpeg files, save name of snapshot

	// save file path for documents subdirectory if it exists
}