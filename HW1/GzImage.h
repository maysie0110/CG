#ifndef __GZ_IMAGE_H_
#define __GZ_IMAGE_H_

#include "GzCommon.h"
using namespace std;

//Bitmap image management class-----------------------------------------------
class GzImage {
public:
	GzImage();
	GzImage(GzInt width, GzInt height);
	GzImage(const char* fileName);

	GzBool load(const char* fileName);
	GzBool save(const char* fileName);

	GzInt sizeW();
	GzInt sizeH();
	GzBool resize(GzInt width, GzInt height);

	GzColor get(GzInt x, GzInt y);
	GzBool set(GzInt x, GzInt y, const GzColor& c);

	void clear(const GzColor& c);

	//// Additional implementation for depth buffer -----------------------------
	//GzBool setZDepth(GzInt x, GzInt y, const GzReal z, const GzColor& c);
	//GzReal getZDepth(GzInt x, GzInt y);
	//void clearZBuffer(const GzReal z);
private:
	vector<vector<GzColor> > pixel;
	GzInt w, h;

	//// Additional implementation for depth buffer -----------------------------
	//vector<vector<GzReal>> depthBuffer;
};
//----------------------------------------------------------------------------

#endif
