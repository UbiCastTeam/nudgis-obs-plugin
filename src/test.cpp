#include "nudgis-service.hpp"

#include <iostream>

#define DEF_FILENAME "test.mkv"
#define DEF_FILECONFIG "nudgis_config.json"

int main(int argc, char *argv[])
{
	const char *filename = DEF_FILENAME;
	const char *fileconfig = DEF_FILECONFIG;
	if (argc > 1)
		filename = argv[1];
	if (argc > 2)
		fileconfig = argv[2];

	std::cout << "start with " << std::endl
		  << "  filename     : " << filename << std::endl
		  << "  fileconfig   : " << fileconfig << std::endl;

	NudgisConfig *config = NudgisConfig::GetCurrentNudgisConfig(fileconfig);
	config->save();

	NudgisUpload nudgis_upload(filename);
	nudgis_upload.run();

	std::cout << "FileUploadedUrl: " << nudgis_upload.GetFileUploadedUrl()
		  << std::endl;
	return 0;
}
