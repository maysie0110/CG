#ifndef __GZ_FRAME_BUFFER_H_
#define __GZ_FRAME_BUFFER_H_

#include "GzCommon.h"
#include "GzImage.h"
#include <vector>
#include <cmath>
using namespace std;

//Frame buffer with Z-buffer -------------------------------------------------
class GzFrameBuffer {
public:
	//The common interface
	void initFrameSize(GzInt width, GzInt height);
	GzImage toImage();

	void clear(GzFunctional buffer);
	void setClearColor(const GzColor& color);
	void setClearDepth(GzReal depth);

	void drawPoint(const GzVertex& v, const GzColor& c, GzFunctional status);

	// Additional implementation for Assignment 2
	void drawTriangle(GzVertex* vqueue, GzColor* cqueue, GzFunctional status);
	//void drawLine(const GzVertex& v0, const GzVertex& v1, const GzColor& c1, const GzColor& c2, GzFunctional status);

private:
	//Put any variables and private functions for your implementation here

	GzImage image;
	GzInt w;
	GzInt h;
	GzColor presetColor;
	GzReal presetDepth;

	vector<vector<GzReal>> depthBuffer;
	vector<vector<GzColor>> colorBuffer;

	// Private method, additional implementation for Assignment 2
	void sortY(GzVertex* vqueue, GzColor* cqueue);
	GzColor colorInterpolate(double startX, double endX, GzColor start, GzColor end, double x);
	double Interpolate(double x0, double x1, double y0, double y1, double x);
	void fillScanLine(GzVertex& left, GzVertex& right, GzColor& leftColor, const GzColor& rightColor, double scanline, GzFunctional status);
};

// additional structure for edge of triangle
typedef struct Edge {
	GzVertex start, end;
	GzColor cstart, cend;

	Edge(const GzVertex& v1, const GzVertex& v2, const GzColor& c1, const GzColor& c2) {
		start = v1;
		end = v2;
		cstart = c1;
		cend = c2;
	}
};
//----------------------------------------------------------------------------

#endif
