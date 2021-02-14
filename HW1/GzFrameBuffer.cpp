#include "GzFrameBuffer.h"
//Put your implementation here------------------------------------------------

//Initialize
void GzFrameBuffer::initFrameSize(GzInt width, GzInt height) {
	w = width;
	h = height;

	depthBuffer.resize(w);
	image.resize(w, h);
}

//Convert the current rendering result to image
GzImage GzFrameBuffer::toImage() {
	return image;
}

//Clear buffers to preset values
void GzFrameBuffer::clear(GzFunctional buffer) {
	if (buffer & GZ_COLOR_BUFFER) 
		image.clear(presetColor);

	if (buffer & GZ_DEPTH_BUFFER)
		clearZBuffer(presetDepth);
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
	if (status & GZ_DEPTH_TEST)
		setZDepth(v[X], v[Y], v[Z], c);
	else
		image.set(v[X], v[Y], c);

}

// Additional implementation for depth buffer -----------------------------
GzBool GzFrameBuffer::setZDepth(GzInt x, GzInt y, const GzReal z, const GzColor& c) {
	if ((x < 0) || (y < 0) || (x >= w) || (y >= h)) return false;

	if (z >= depthBuffer[x][y]) {
		depthBuffer[x][y] = z;
		image.set(x, y, c);
		return true;
	}
}

GzReal GzFrameBuffer::getZDepth(GzInt x, GzInt y) {
	if ((x < 0) || (y < 0) || (x >= w) || (y >= h)) return GzReal();
	return depthBuffer[x][y];
}

void GzFrameBuffer::clearZBuffer(const GzReal z) {
	fill(depthBuffer.begin(), depthBuffer.end(), vector<GzReal>(h, z));
}