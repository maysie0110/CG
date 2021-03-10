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
	if (buffer & GZ_COLOR_BUFFER) {
		fill(colorBuffer.begin(), colorBuffer.end(), vector<GzColor>(h, presetColor));
	}
	if (buffer & GZ_DEPTH_BUFFER) {
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
	int newY =-v[Y] + h - 1;

	if ((v[X] < 0) || (newY < 0) || (v[X] >= w) || (newY >= h)) //Check boundaries 
		return;

	if (status & GZ_DEPTH_TEST) {
		if (v[Z] > depthBuffer[v[X]][newY]) { //depth test passes
			depthBuffer[v[X]][newY] = v[Z]; //update depth buffer with new depth value
			colorBuffer[v[X]][newY] = c;// update color buffer for rendering
		}
	}
	else {
		colorBuffer[v[X]][newY] = c;
	}
}


// Additional implementation for drawing a triangle

/* Scan Line Algorithm generates pixels in order, left-to-right and top-to-bottom
* Consider all rows that can possibly overlap
* Extract a scanline, i.e. calculate the intersections for one row of pixels and get the endpoints of scanline
* Sort the intersection by increasing x-coordinate i.e. from left to right
* make pairs of intersections
* Fill color for pixel at the scanline (all the pixels inside the pair)
*/
void GzFrameBuffer::drawTriangle(GzVertex* vTriangle, GzColor* cTriangle, GzFunctional status) {
	// First, sort vertices using y in increasing order
	sortY(vTriangle, cTriangle);

	// Create 3 edges of triangle
	Edge edge01(vTriangle[0], vTriangle[1], cTriangle[0], cTriangle[1]);
	Edge edge12(vTriangle[1], vTriangle[2], cTriangle[1], cTriangle[2]);
	Edge edge02(vTriangle[0], vTriangle[2], cTriangle[0], cTriangle[2]);

	/*
	* Interpolate quantity along left and eight edges, as a function on y
	* Then, interpolate quantity as a function of x
	*/
	for (double scanline = vTriangle[0].at(Y); scanline <= vTriangle[2].at(Y);scanline++)
	{
		GzVertex left;
		GzColor leftColor;
		GzVertex right;
		GzColor rightColor;

		left[Y] = scanline;
		right[Y] = scanline;

		/*Determine the left endpoint and right endpoint that intersects the scanline*/
		if (scanline <= vTriangle[1][Y])
		{
			left[X] = Interpolate(edge01.start[Y], edge01.end[Y], edge01.start[X], edge01.end[X], scanline);
			left[Z] = Interpolate(edge01.start[Y], edge01.end[Y], edge01.start[Z], edge01.end[Z], scanline);
			leftColor = colorInterpolate(edge01.start[Y], edge01.end[Y], edge01.cstart, edge01.cend, scanline);
		} else {
			left[X] = Interpolate(edge12.start[Y], edge12.end[Y], edge12.start[X], edge12.end[X], scanline);
			left[Z] = Interpolate(edge12.start[Y], edge12.end[Y], edge12.start[Z], edge12.end[Z], scanline);
			leftColor = colorInterpolate(edge12.start[Y], edge12.end[Y], edge12.cstart, edge12.cend, scanline);
		}

		right[X] = Interpolate(edge02.start[Y], edge02.end[Y], edge02.start[X], edge02.end[X], scanline);
		right[Z] = Interpolate(edge02.start[Y], edge02.end[Y], edge02.start[Z], edge02.end[Z], scanline);
		rightColor = colorInterpolate(edge02.start[Y], edge02.end[Y], edge02.cstart, edge02.cend, scanline);


		if (left[X] > right[X]) /*Swap coordinate & color of left and right points*/
		{
			GzVertex vtemp; 
			GzColor ctemp;
			vtemp = left;
			left = right;
			right = vtemp;
			ctemp = leftColor;
			leftColor = rightColor;
			rightColor = ctemp;}

		// Draw all pixels in between left point and right point)
		fillScanLine(left, right, leftColor, rightColor, scanline, status);
	}
}

// linear intepolation for z value
double GzFrameBuffer::Interpolate(double x0, double x1, double y0, double y1, double x)
{
	double dX = x1 - x0;
	double dY = y1 - y0;
	if (dX != 0)
		return y0 + (x - x0) * (dY / dX);
	else
		return y0;
}

//linear color interpolation
GzColor GzFrameBuffer::colorInterpolate(double x0, double x1, GzColor cstart, GzColor cend, double x)
{
	double dX = x1 - x0;

	GzColor color(0, 0, 0);
	for (int i = 0; i < 3; i++) /*3 values for R,G,B*/
	{
		if (dX != 0)
			color[i] = cstart[i] + (x - x0) * ((cend[i] - cstart[i]) / dX);
		else
			color[i] = cstart[i];
	}
	return color;
}

void GzFrameBuffer::fillScanLine(GzVertex& left, GzVertex& right, GzColor& leftColor, const GzColor& rightColor, double scanline, GzFunctional status) {
	for (int i = 0; left[X] + i < right[X]; i++)
	{
		GzVertex current;

		current[X] = i + left[X];
		current[Y] = scanline;
		current[Z] = Interpolate(left[X], right[X], left[Z], right[Z], current[X]);

		GzColor currentColor = colorInterpolate(left[X], right[X], leftColor, rightColor, current[X]);
		drawPoint(current, currentColor, status);
	}
}

//using insertion sort to sort vertices list
void GzFrameBuffer::sortY(GzVertex* vTriangle, GzColor* cTriangle) {
	GzVertex vertexTemp;
	GzColor colorTemp;

	for (int i = 1; i < 3; i++) {
		vertexTemp = vTriangle[i];
		colorTemp = cTriangle[i];
		int j = i - 1;

		/* Move elements of vertices list [0..i-1], that are
		greater than temp, to one position ahead
		of their current position */
		while (j >= 0 && vTriangle[j].at(Y) > vertexTemp.at(Y)) {
			vTriangle[j + 1] = vTriangle[j];
			cTriangle[j + 1] = cTriangle[j];
			j = j - 1;
		}
		vTriangle[j + 1] = vertexTemp;
		cTriangle[j + 1] = colorTemp;
	}
}