#pragma once
#include "BCFDocumentStore.hpp"
#include <optional>
#include <vector>
#include <unordered_map>

struct BCFDocument 
{
	bool valid = false;
	bool hasDocumentsFolder = false;
	DocumentRef versionDoc;						// .version file, required
	std::optional<DocumentRef> extensionsDoc;	// .xml file, optional, defines the extensions of a project
	std::optional<DocumentRef> documentsDoc;	// .xml file, optional, defines any additional documents in a project
	std::optional<DocumentRef> projectDoc;		// .bcfp file, optional, defines details of a project
	struct BCFTopicEntry
	{
		bool valid = false;						// if false, this topic's markup doc cannot be parsed
		std::string guid;						// name of the folder is always the guid of the topic
		DocumentRef markupDoc;					// .bcf file, required
		std::vector<DocumentRef> viewpointDoc;  // .bcfv files, can be multiple or none
		std::vector<std::string> snapshotNames; // file names of the snapshots in the topic folder, can be multiple or none
	};
	std::unordered_map<std::string, BCFDocument::BCFTopicEntry> topics;
};


struct BCFIO
{
	struct ParseConfig
	{
		// TODO
		bool validateSchema = false;
	};
	static BCFDocument Parse(const std::string& bcfFilePath, std::string& errMsg, const ParseConfig& cfg = {});
};