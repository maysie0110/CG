#include "GzFrameBuffer.h"
//Put your implementation here------------------------------------------------

//Initialize
void GzFrameBuffer::initFrameSize(GzInt width, GzInt height) {
	w = width;
	h = height;

	presetColor = GzColor(0, 0, 0); 
	presetDepth = 0;

	//colorBuffer = new GzColor[w * h];
	//depthBuffer = new GzReal[w * h];
	colorBuffer.resize(w);
	depthBuffer.resize(w);
	//image.resize(w, h);
}

//Convert the current rendering result to image
GzImage GzFrameBuffer::toImage() {
	GzImage image(w, h); 
	for (int i = 0; i < w; i++) {
		for (int j = 0; j < h; j++) {
			image.set(i, j, colorBuffer[i][j]);
		}
	}
	return image;
}

//Clear buffers to preset values
void GzFrameBuffer::clear(GzFunctional buffer) {
	if (buffer & GZ_COLOR_BUFFER) {
		//image.clear(presetColor);
		/*for (int i = 0; i < h; i++) {
			for (int j = 0; j < w; j++) {
				colorBuffer[j][i] = presetColor;
			}
		}*/
		fill(colorBuffer.begin(), colorBuffer.end(), vector<GzColor>(h, presetColor));
	}
	if (buffer & GZ_DEPTH_BUFFER) {
		//clearZBuffer(presetDepth);
		/*for (int i = 0; i < w; i++) {
			for (int j = 0; j < h; j++) {
				depthBuffer[i][j] = presetDepth;
			}
		}*/
		fill(depthBuffer.begin(), depthBuffer.end(), vector<GzReal>(h, presetDepth));
	}
}

//Specify clear values for the color buffer
void GzFrameBuffer::setClearColor(const GzColor& color) {
	presetColor = color;
}

//Specify the clear value for the depth buffer
void GzFrameBuffer::setClearDepth(GzReal depth) {
	presetDepth = depth;
}

void GzFrameBuffer::drawPoint(const GzVertex& v, const GzColor& c, GzFunctional status) {
	if ((v[X] < 0) || (v[Y] < 0) || (v[X] >= w) || (v[Y] >= h)) //Check boundaries 
		return;

	if (status & GZ_DEPTH_TEST) {
		//setZDepth(v[X], v[Y], v[Z], c);
		if (v[Z] > depthBuffer[v[X]][v[Y]]) {
			depthBuffer[v[X]][v[Y]] = v[Z];
			colorBuffer[v[X]][v[Y]] = c;
		}
	}
	else {
		//image.set(v[X], v[Y], c);
		colorBuffer[v[X]][v[Y]] = c;
	}
}

// Additional implementation for depth buffer -----------------------------
//GzBool GzFrameBuffer::setZDepth(GzInt x, GzInt y, const GzReal z, const GzColor& c) {
//	if ((x < 0) || (y < 0) || (x >= w) || (y >= h)) return false;
//
//	if (z >= depthBuffer[x][y]) {
//		depthBuffer[x][y] = z;
//		image.set(x, y, c);
//		return true;
//	}
//}
//
//GzReal GzFrameBuffer::getZDepth(GzInt x, GzInt y) {
//	if ((x < 0) || (y < 0) || (x >= w) || (y >= h)) return GzReal();
//	return depthBuffer[x][y];
//}
//
//void GzFrameBuffer::clearZBuffer(const GzReal z) {
//	fill(depthBuffer.begin(), depthBuffer.end(), vector<GzReal>(h, z));
//}