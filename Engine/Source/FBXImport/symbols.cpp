#include <FBXImport/DocumentImporter.h>

extern "C" SQFBXIMPORTER_API Squirrel::Resource::IModelImporter * CreateImporter(Squirrel::Render::IRender * render, Squirrel::Resource::TextureStorage * texStorage)
{
	return new Squirrel::FBXImport::DocumentImporter(render, texStorage, NULL);
}

extern "C" SQFBXIMPORTER_API void DestroyImporter(Squirrel::Resource::IModelImporter * importer)
{
	if(importer)
		delete importer;
}

extern "C" SQFBXIMPORTER_API const char_t * GetImportingExtension()
{
	return "fbx";
}