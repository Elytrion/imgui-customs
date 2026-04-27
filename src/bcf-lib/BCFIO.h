#pragma once
#include "BCFDoc.h"

struct BCFIO
{
	struct ParseConfig
	{
		// TODO
		bool validateSchema = false;
	};
	static BCFDocument Parse(const std::string& bcfFilePath, std::string& errMsg, const ParseConfig& cfg = {});

	struct WriteConfig
	{
		bool writeToNewFile = true;				// if true, we will check for conflicts with the write path and create an entirely new .bcfzip file, else, we will overwrite any files there.
		std::string extToUse = ".bcfzip";		// the default extension should be .bcfzip as per specifications, but the user is allowed to change this to anything they want (for example .zip or .bcf)
		bool viewpointsHaveTailName = true;     // if true, viewpoint files will be named {guid}_viewpoint.bcfv, else they will be named as {guid}.bcfv
		enum
		{
			PNG, JPEG, ORIGINAL
		} snapshotSaveFormat = ORIGINAL;	// the image format to save snapshots in, either PNG, JPEG, or based on its original extension (which is either .png or .jpg/.jpeg as per specifications)
	};
	static bool Write(const BCFDocument& bcfDoc, const std::string& writePath, std::string& errMsg, const WriteConfig& cfg = {});
};