#pragma once
#include "BCFDocumentStore.hpp"
#include <optional>
#include <vector>

struct BCFDocument 
{
	DocumentRef versionDoc;						// .version file, required
	std::optional<DocumentRef> extensionsDoc;	// .xml file, optional, defines the extensions of a project
	std::optional<DocumentRef> documentsDoc;	// .xml file, optional, defines any additional documents in a project
	std::optional<DocumentRef> projectDoc;		// .bcfp file, optional, defines details of a project
	struct BCFTopicEntry
	{
		std::string guid;						// name of the folder is always the guid of the topic
		DocumentRef markupDoc;					// .bcf file, required
		std::vector<DocumentRef> viewpointDoc;  // .bcfv files, can be multiple or none
		std::vector<std::string> snapshotNames; // file names of the snapshots in the topic folder, can be multiple or none
	};
	std::vector<BCFTopicEntry> topics;
};


struct BCFIO
{
	struct ParseConfig
	{
		// TODO
	};
	static BCFDocument Parse(const std::string& xmlFilePath, std::string& errMsg, const ParseConfig& cfg = {});
};