#pragma once
#include "BCFDocumentStore.hpp"
#include <optional>
#include <vector>
#include <unordered_map>

struct BCFDocument 
{
	std::string workingPath = {};               // the path of the unzipped copy of the document for access to snapshots and documents folder, any changes here are not reflected in the original zip file, and should be cached here until writing back to zip
	std::string srcPath = {};                   // the path of the original zip file
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

	struct WriteConfig
	{
		bool writeToNewFile = true;				// if true, we will check for conflicts with the write path and create an entirely new .bcfzip file, else, we will overwrite any files there.
		std::string extToUse = ".bcfzip";		// the default extension should be .bcfzip as per specifications, but the user is allowed to change this to anything they want (for example .zip or .bcf)
	};
	static bool Write(const BCFDocument& bcfDoc, const std::string& writePath, std::string& errMsg, const WriteConfig& cfg = {});

	static void ClearAllWorkingFiles();
};