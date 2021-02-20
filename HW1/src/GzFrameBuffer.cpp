#include "GzFrameBuffer.h"
//Put your implementation here------------------------------------------------

//Initialize
void GzFrameBuffer::initFrameSize(GzInt width, GzInt height) {
	w = width;
	h = height;

	presetColor = GzColor(0, 0, 0); 
	presetDepth = 0;

	colorBuffer.resize(w);
	depthBuffer.resize(w);
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
	if (buffer & GZ_COLOR_BUFFER) { //clear color buffer
		fill(colorBuffer.begin(), colorBuffer.end(), vector<GzColor>(h, presetColor));
	}
	if (buffer & GZ_DEPTH_BUFFER) { //clear depth buffer
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

	int y = -v[Y]+h -1; //update y-coordinate to get origin (0,0) in the bottom left
	if ((v[X] < 0) || (y < 0) || (v[X] >= w) || (y >= h)) //Check boundaries 
		return;

	if (status & GZ_DEPTH_TEST) { 
		if (v[Z] > depthBuffer[v[X]][y]) { //depth test passes
			depthBuffer[v[X]][y] = v[Z]; //update depth buffer with new depth value
			colorBuffer[v[X]][y] = c; //update color buffer for rendering
		}
	}
	else {
		colorBuffer[v[X]][y] = c;
	}
}