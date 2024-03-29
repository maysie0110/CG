#ifndef __GZ_FRAME_BUFFER_H_
#define __GZ_FRAME_BUFFER_H_

#include "GzCommon.h"
#include "GzImage.h"
#include <vector>
using namespace std;

//Frame buffer with Z-buffer -------------------------------------------------
class GzFrameBuffer {
public:
	//The common interface
	void initFrameSize(GzInt width, GzInt height); //Initialize
	GzImage toImage(); //Convert the current rendering result to image

	void clear(GzFunctional buffer); //Clear buffers to preset values
	void setClearColor(const GzColor& color); //Specify clear values for the color buffer
	void setClearDepth(GzReal depth); //Specify the clear value for the depth buffer

	void drawPoint(const GzVertex& v, const GzColor& c, GzFunctional status);

private:
	//Put any variables and private functions for your implementation here
	GzImage image;
	GzInt w;
	GzInt h;
	GzColor presetColor;
	GzReal presetDepth;

	vector<vector<GzReal>> depthBuffer;
	vector<vector<GzColor>> colorBuffer;
};
//----------------------------------------------------------------------------

#endif
