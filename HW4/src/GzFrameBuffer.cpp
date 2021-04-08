#include "GzFrameBuffer.h"

//Put your implementation here------------------------------------------------
#include <climits>
void GzFrameBuffer::initFrameSize(GzInt width, GzInt height) {
	image.resize(width, height);
	depthBuffer = vector<vector<GzReal> >(width, vector<GzReal>(height, clearDepth));
}

GzImage GzFrameBuffer::toImage() {
	return image;
}

void GzFrameBuffer::clear(GzFunctional buffer) {
	if (buffer & GZ_COLOR_BUFFER) image.clear(clearColor);
	if (buffer & GZ_DEPTH_BUFFER)
		for (GzInt x = 0; x != depthBuffer.size(); x++)
			fill(depthBuffer[x].begin(), depthBuffer[x].end(), clearDepth);
}

void GzFrameBuffer::setClearColor(const GzColor& color) {
	clearColor = color;
}

void GzFrameBuffer::setClearDepth(GzReal depth) {
	clearDepth = depth;
}

void GzFrameBuffer::drawPoint(const GzVertex& v, const GzColor& c, GzFunctional status) {
	GzInt x = (GzInt)v[X];
	GzInt y = image.sizeH() - (GzInt)v[Y] - 1;
	if ((x < 0) || (y < 0) || (x >= image.sizeW()) || (y >= image.sizeH())) return;
	if (status & GZ_DEPTH_TEST) {
		if (v[Z] >= depthBuffer[x][y]) {
			image.set(x, y, c);
			depthBuffer[x][y] = v[Z];
		}
	}
	else {
		image.set(x, y, c);
		depthBuffer[x][y] = v[Z];
	}
}

void GzFrameBuffer::drawTriangle(vector<GzVertex>& v, vector<GzColor>& c, GzFunctional status) {
	GzInt yMin, yMax;
	GzReal xMin, xMax, zMin, zMax;
	GzColor cMin, cMax;

	v.push_back(v[0]);
	c.push_back(c[0]);

	yMin = INT_MAX;
	yMax = -INT_MAX;

	for (GzInt i = 0; i < 3; i++) {
		yMin = min((GzInt)floor(v[i][Y]), yMin);
		yMax = max((GzInt)floor(v[i][Y] - 1e-3), yMax);
	}

	for (GzInt y = yMin; y <= yMax; y++) {
		xMin = INT_MAX;
		xMax = -INT_MAX;
		for (GzInt i = 0; i < 3; i++) {
			if ((GzInt)floor(v[i][Y]) == y) {
				if (v[i][X] < xMin) {
					xMin = v[i][X];
					zMin = v[i][Z];
					cMin = c[i];
				}
				if (v[i][X] > xMax) {
					xMax = v[i][X];
					zMax = v[i][Z];
					cMax = c[i];
				}
			}
			if ((y - v[i][Y]) * (y - v[i + 1][Y]) < 0) {
				GzReal x;
				realInterpolate(v[i][Y], v[i][X], v[i + 1][Y], v[i + 1][X], y, x);
				if (x < xMin) {
					xMin = x;
					realInterpolate(v[i][Y], v[i][Z], v[i + 1][Y], v[i + 1][Z], y, zMin);
					colorInterpolate(v[i][Y], c[i], v[i + 1][Y], c[i + 1], y, cMin);
				}
				if (x > xMax) {
					xMax = x;
					realInterpolate(v[i][Y], v[i][Z], v[i + 1][Y], v[i + 1][Z], y, zMax);
					colorInterpolate(v[i][Y], c[i], v[i + 1][Y], c[i + 1], y, cMax);
				}
			}
		}
		drawRasLine(y, xMin, zMin, cMin, xMax - 1e-3, zMax, cMax, status);
	}
}

void GzFrameBuffer::drawRasLine(GzInt y, GzReal xMin, GzReal zMin, GzColor& cMin, GzReal xMax, GzReal zMax, GzColor& cMax, GzFunctional status) {
	if ((y < 0) || (y >= image.sizeH())) return;
	if ((GzInt)floor(xMin) == (GzInt)floor(xMax)) {
		if (zMin > zMax) drawPoint(GzVertex(floor(xMin), y, zMin), cMin, status);
		else drawPoint(GzVertex(floor(xMin), y, zMax), cMax, status);
	}
	else {
		GzReal z;
		GzColor c;
		y = image.sizeH() - y - 1;
		int w = image.sizeW();
		if (status & GZ_DEPTH_TEST) {
			for (int x = max(0, (GzInt)floor(xMin)); x <= min(w - 1, (GzInt)floor(xMax)); x++) {
				realInterpolate(xMin, zMin, xMax, zMax, x, z);
				if (z >= depthBuffer[x][y]) {
					colorInterpolate(xMin, cMin, xMax, cMax, x, c);
					image.set(x, y, c);
					depthBuffer[x][y] = z;
				}
			}
		}
		else {
			for (int x = max(0, (GzInt)floor(xMin)); x <= min(w - 1, (GzInt)floor(xMax)); x++) {
				realInterpolate(xMin, zMin, xMax, zMax, x, z);
				colorInterpolate(xMin, cMin, xMax, cMax, x, c);
				image.set(x, y, c);
				depthBuffer[x][y] = z;
			}
		}
	}
}

void GzFrameBuffer::realInterpolate(GzReal key1, GzReal val1, GzReal key2, GzReal val2, GzReal key, GzReal& val) {
	val = val1 + (val2 - val1) * (key - key1) / (key2 - key1);
}

void GzFrameBuffer::colorInterpolate(GzReal key1, GzColor& val1, GzReal key2, GzColor& val2, GzReal key, GzColor& val) {
	GzReal k = (key - key1) / (key2 - key1);
	for (GzInt i = 0; i < 4; i++) val[i] = val1[i] + (val2[i] - val1[i]) * k;
}

void GzFrameBuffer::shadeModel(const GzInt model) {
	curShadeModel = model;
}

void GzFrameBuffer::material(GzReal _kA, GzReal _kD, GzReal _kS, GzReal _s) {
	kA = _kA;
	kD = _kD;
	kS = _kS;
	s = _s;
}

void GzFrameBuffer::addLight(const GzVector& v, const GzColor& c) {

	lightSources.push_back(GzLightSource(v, c));
	transLightSources.push_back(GzLightSource(v, c));
}

/*
* Additional implementation for Assignment 4
*/

//apply transformation to light sources
void GzFrameBuffer::loadLightTrans(GzMatrix transMatrix) {
	for (int k = 0; k < lightSources.size(); k++)
	{
		GzMatrix M;
		M.resize(3, 3);

		for (int i = 0;i < 3; i++)
			for (int j = 0; j < 3; j++)
				M[i][j] = transMatrix[i][j];

		M = M.inverse3x3().transpose();


		GzVector d = lightSources[k].dir;

		GzVector transLightDir;
		transLightDir[0] = M[0][0] * d[0] + M[0][1] * d[1] + M[0][2] * d[2];
		transLightDir[1] = M[1][0] * d[0] + M[1][1] * d[1] + M[1][2] * d[2];
		transLightDir[2] = M[2][0] * d[0] + M[2][1] * d[1] + M[2][2] * d[2];

		transLightSources[k].dir = transLightDir;
	}
}

//Get eye position
void GzFrameBuffer::loadEyePos(GzVector eye) {
	E = eye;
}
// draw point when there are shader
// similar to the original function with a small changes to color by applying material property (ie. shading with diffuse, ambient, specular)

void GzFrameBuffer::drawPoint(const GzVertex& v, const GzColor& c, const GzVector& n, GzFunctional status) {
	GzInt x = (GzInt)v[X];
	GzInt y = image.sizeH() - (GzInt)v[Y] - 1;
	if ((x < 0) || (y < 0) || (x >= image.sizeW()) || (y >= image.sizeH())) return;

	GzColor color = shader(c, n); //get new color

	if (status & GZ_DEPTH_TEST) {
		if (v[Z] >= depthBuffer[x][y]) {
			image.set(x, y, color);
			depthBuffer[x][y] = v[Z];
		}
	}
	else {
		image.set(x, y, color);
		depthBuffer[x][y] = v[Z];
	}
}

// http://www.learnopengles.com/android-lesson-two-ambient-and-diffuse-lighting/
// https://www.cs.brandeis.edu/~cs155/Lecture_16.pdf
// https://www.clear.rice.edu/comp360/lectures/old/Shading.pdf
////// I = I_amb + I_diff + I_spec
//// I = K_a*I_a +  I_p(K_d * dot(N,L) + K_s * dot(R,V)^n)

GzColor GzFrameBuffer::shader(const GzColor& c, const GzVector& n) {
	GzColor color;

	// Ambient light - light that comes from all directions
	// final color = base color * ambient
	for (int i = 0; i < 4; i++) {
		color[i] = c[i] * kA;
	}

	// Diffuse lighting - point light source
	for (int i = 0; i < transLightSources.size();i++) {

		// Point light intensity.In this case, it's color of the light
		GzColor lightColor = transLightSources[i].col;

		// Light source at infinity
		// Light direction
		GzVector pos;
		GzVector L = pos - transLightSources[i].dir;
		L.normalize(); //normalize direction vector

		//Specular
		// Vector to eye.
		GzVector eye = E;
		eye.normalize();

		//Viewer direction
		GzVector view = L + eye;
		view.normalize();

		// Calculate the reflection direction for an incident vector
		GzReal lightIntensity = dotProduct(L,n);
		lightIntensity = clamp(lightIntensity, 0.0, 1.0);
		GzVector reflection = 2 * n - L;

		reflection.normalize();

		for (int j = 0; j < 4; j++) {

			// Diffuse light
			GzReal I_diff = kD * lightColor[j] * max(dotProduct(n, L), 0.0);
			// Because dot product can have a range of -1 to 1, so we clamp it to a range of 0 to 1
			I_diff = clamp(I_diff, 0.0, 1.0);
			color[j] += I_diff;

			// Specular light
			GzReal I_spec = lightColor[j] * kS * pow(max(dotProduct(reflection, view), 0.0), s);
			I_spec = clamp(I_spec, 0.0, 1.0);
			color[j] += I_spec;

		}
	}
	return color;
}

GzReal GzFrameBuffer::clamp(GzReal val, GzReal min, GzReal max)
{
	if (val < min)
		val = min;
	else if (val > max)
		val = max;
	return val;
}

void GzFrameBuffer::drawTriangle(vector<GzVertex>& v, vector<GzColor>& c, vector<GzVector>& n, GzFunctional status) {
	if (curShadeModel == GZ_GOURAUD)
	{
		vector<GzColor> color(3);
		for (int i = 0; i < 3; i++) {
			color[i] = shader(c[i], n[i]);
		}
		drawTriangle(v, color, status);
	}


	else if (curShadeModel == GZ_PHONG)
	{
		//interpolate normal for each point and apply the shader.
		GzInt yMin, yMax;
		GzReal xMin, xMax, zMin, zMax;
		GzColor cMin, cMax;
		GzVector nMin, nMax;

		v.push_back(v[0]);
		c.push_back(c[0]);
		n.push_back(n[0]);

		yMin = INT_MAX;
		yMax = -INT_MAX;

		for (GzInt i = 0; i < 3; i++) {
			yMin = min((GzInt)floor(v[i][Y]), yMin);
			yMax = max((GzInt)floor(v[i][Y] - 1e-3), yMax);
		}

		for (GzInt y = yMin; y <= yMax; y++) {
			xMin = INT_MAX;
			xMax = -INT_MAX;
			for (GzInt i = 0; i < 3; i++) {
				if ((GzInt)floor(v[i][Y]) == y) {
					if (v[i][X] < xMin) {
						xMin = v[i][X];
						zMin = v[i][Z];
						cMin = c[i];
						nMin = n[i];
					}
					if (v[i][X] > xMax) {
						xMax = v[i][X];
						zMax = v[i][Z];
						cMax = c[i];
						nMax = n[i];
					}
				}
				if ((y - v[i][Y]) * (y - v[i + 1][Y]) < 0) {
					GzReal x;
					realInterpolate(v[i][Y], v[i][X], v[i + 1][Y], v[i + 1][X], y, x);
					if (x < xMin) {
						xMin = x;
						realInterpolate(v[i][Y], v[i][Z], v[i + 1][Y], v[i + 1][Z], y, zMin);
						colorInterpolate(v[i][Y], c[i], v[i + 1][Y], c[i + 1], y, cMin);
						vectorInterpolate(v[i][Y], n[i], v[i + 1][Y], n[i + 1], y, nMin);
					}
					if (x > xMax) {
						xMax = x;
						realInterpolate(v[i][Y], v[i][Z], v[i + 1][Y], v[i + 1][Z], y, zMax);
						colorInterpolate(v[i][Y], c[i], v[i + 1][Y], c[i + 1], y, cMax);
						vectorInterpolate(v[i][Y], n[i], v[i + 1][Y], n[i + 1], y, nMax);
					}
				}
			}
			drawRasLine(y, xMin, zMin, cMin, nMin, xMax - 1e-3, zMax, cMax, nMax, status);
		}
	}
}

// Similar to colorInterpolate with the addition of normal vector interpolation and the use of shader function
void GzFrameBuffer::vectorInterpolate(GzReal key1, GzVector& val1, GzReal key2, GzVector& val2, GzReal key, GzVector& val) {
	GzReal k = (key - key1) / (key2 - key1);
	for (GzInt i = 0; i < 3; i++)
		val[i] = val1[i] + (val2[i] - val1[i]) * k;
	val.normalize();
}
void GzFrameBuffer::drawRasLine(GzInt y, GzReal xMin, GzReal zMin, GzColor& cMin, GzVector& nMin, GzReal xMax, GzReal zMax, GzColor& cMax, GzVector& nMax, GzFunctional status) {

	if ((y < 0) || (y >= image.sizeH())) return;
	if ((GzInt)floor(xMin) == (GzInt)floor(xMax)) {
		if (zMin > zMax) drawPoint(GzVertex(floor(xMin), y, zMin), cMin, nMin, status);
		else drawPoint(GzVertex(floor(xMin), y, zMax), cMax, nMax, status);
	}
	else {
		GzReal z;
		GzColor c;
		GzVector n;

		y = image.sizeH() - y - 1;
		int w = image.sizeW();
		if (status & GZ_DEPTH_TEST) {
			for (int x = max(0, (GzInt)floor(xMin)); x <= min(w - 1, (GzInt)floor(xMax)); x++) {
				realInterpolate(xMin, zMin, xMax, zMax, x, z);
				if (z >= depthBuffer[x][y]) {
					colorInterpolate(xMin, cMin, xMax, cMax, x, c);
					vectorInterpolate(xMin, nMin, xMax, nMax, x, n);

					c = shader(c, n);
					image.set(x, y, c);
					depthBuffer[x][y] = z;
				}
			}
		}
		else {
			for (int x = max(0, (GzInt)floor(xMin)); x <= min(w - 1, (GzInt)floor(xMax)); x++) {
				realInterpolate(xMin, zMin, xMax, zMax, x, z);
				colorInterpolate(xMin, cMin, xMax, cMax, x, c);
				vectorInterpolate(xMin, nMin, xMax, nMax, x, n);

				c = shader(c, n);
				image.set(x, y, c);
				depthBuffer[x][y] = z;
			}
		}
	}
}