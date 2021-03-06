#include "FileUtil.h"
#include <assert.h>
AAssetManager* FileUtil::aam;
void FileUtil::setAAssetManager(AAssetManager* aamIn)
{
	aam=aamIn;
}
string FileUtil::loadAssetStr(string fname)
{
	AAsset* asset =AAssetManager_open(aam,fname.c_str(),AASSET_MODE_UNKNOWN);
	off_t fileSize = AAsset_getLength(asset);
	unsigned char* data = (unsigned char*) malloc(fileSize + 1);
	data[fileSize] = '\0';
	int readBytesCount = AAsset_read(asset, (void*)data, fileSize);
	assert(readBytesCount==fileSize);
	std::string resultStr((const char*)data);
	return resultStr;
}
int fromBytesToInt(unsigned char* buff)
{
	int k=0;
	unsigned char* temp=(unsigned char*)(&k);
	temp[0]=buff[0];
	temp[1]=buff[1];
	temp[2]=buff[2];
	temp[3]=buff[3];
	return k;
}
TexDataObject* FileUtil::loadCommonTexData(string fname)
{
	AAsset* asset =AAssetManager_open(aam,fname.c_str(),AASSET_MODE_UNKNOWN);
	unsigned char* buf=new unsigned char[4];
	AAsset_read(asset, (void*)buf, 4);
	int width=fromBytesToInt(buf);
	AAsset_read(asset, (void*)buf, 4);
	int height=fromBytesToInt(buf);
	unsigned char* data=new unsigned char[width*height*4];
	AAsset_read(asset, (void*)data, width*height*4);
	TexDataObject* ctdo=new TexDataObject(width,height,data,width*height*4);
	return ctdo;
}
ThreeDTexDataObject* FileUtil::load3DTexData(string fname)
{
	AAsset* asset =AAssetManager_open(aam,fname.c_str(),AASSET_MODE_UNKNOWN);
	unsigned char* buf=new unsigned char[4]; 
	AAsset_read(asset, (void*)buf, 4); 
	int width=fromBytesToInt(buf); 
	AAsset_read(asset, (void*)buf, 4); 
	int height=fromBytesToInt(buf); 
	AAsset_read(asset, (void*)buf, 4); 
	int depth=fromBytesToInt(buf); 
	unsigned char* data=new unsigned char[width*height*depth*4]; 
	AAsset_read(asset, (void*)data, width*height*depth*4); 
	ThreeDTexDataObject* ctdo=new ThreeDTexDataObject(width,height,depth,data);
	return ctdo; 
}
