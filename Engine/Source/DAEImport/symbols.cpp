#include <DAEImport/DocumentImporter.h>

extern "C" SQDAEIMPORTER_API Squirrel::Resource::IModelImporter * CreateImporter(Squirrel::Render::IRender * render, Squirrel::Resource::TextureStorage * texStorage)
{
	return new Squirrel::DAEImport::DocumentImporter(render, texStorage, NULL);
}

extern "C" SQDAEIMPORTER_API void DestroyImporter(Squirrel::Resource::IModelImporter * importer)
{
	if(importer)
		delete importer;
}

extern "C" SQDAEIMPORTER_API const char_t * GetImportingExtension()
{
	return "dae";
}