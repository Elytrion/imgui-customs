#pragma once
#include "pugixml.hpp"
#include <optional>
#include <vector>

struct BCFRaw
{
	pugi::xml_document version; // .version file, required
	std::optional<pugi::xml_document> projectXML; // .bcfp file, optional
	struct BCFTopicEntry
	{
		std::string guid;
		pugi::xml_document markupXML;
		std::vector<pugi::xml_document> viewpointXMLs;
		std::vector<std::string> snapshotRefs; // file names of the snapshots in the topic folder
	};
	std::vector<BCFTopicEntry> topics;
};

struct BCFIO
{
	static BCFRaw LoadBCF(const std::string& filePath, std::string& errMsg);
};