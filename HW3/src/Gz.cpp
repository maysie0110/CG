#include "Gz.h"



//============================================================================
//Implementations in Assignment #1
//============================================================================
GzImage Gz::toImage() {
	return frameBuffer.toImage();
}

void Gz::clear(GzFunctional buffer) {
	frameBuffer.clear(buffer);
}

void Gz::clearColor(const GzColor& color) {
	frameBuffer.setClearColor(color);
}

void Gz::clearDepth(GzReal depth) {
	frameBuffer.setClearDepth(depth);
}

void Gz::enable(GzFunctional f) {
	status=status|f;
}

void Gz::disable(GzFunctional f) {
	status=status&(~f);
}

GzBool Gz::get(GzFunctional f) {
	if (status&f) return true; else return false;
}

void Gz::begin(GzPrimitiveType p) {
	currentPrimitive=p;
}

void Gz::addVertex(const GzVertex& v) {
	vertexQueue.push(v);
}

void Gz::addColor(const GzColor& c) {
	colorQueue.push(c);
}
//============================================================================
//End of Implementations in Assignment #1
//============================================================================



//============================================================================
//Implementations in Assignment #3
//============================================================================

void Gz::initFrameSize(GzInt width, GzInt height) {
	//This function need to be updated since we have introduced the viewport.
	//The viewport size is set to the size of the frame buffer.
	wViewport=(GzReal)width;
	hViewport=(GzReal)height;
	frameBuffer.initFrameSize(width, height);

	prjMatrix = Identity(4);
	transMatrix = Identity(4);

	viewport(0, 0);			//Default center of the viewport 
}

void Gz::end() {
	//This function need to be updated since we have introduced the viewport,
	//projection, and transformations.
	//In our implementation, all rendering is done when Gz::end() is called.
	//Depends on selected primitive, different number of vetices, colors, ect.
	//are pop out of the queue.
	switch (currentPrimitive) {
		case GZ_POINTS: {
			GzVertex v;
			GzColor c;
			GzMatrix M;
			while ( (vertexQueue.size()>=1) && (colorQueue.size()>=1) ) {
				M.fromVertex(vertexQueue.front());
				M = prjMatrix * transMatrix * M;
				v = M.toVertex();
				affineTransform(v);
				vertexQueue.pop();

				c = colorQueue.front();
				colorQueue.pop();

				frameBuffer.drawPoint(v, c, status);
			}
		} break;
		case GZ_TRIANGLES: {
			//Put your triangle drawing implementation here:
			//   - Extract 3 vertices in the vertexQueue
			//   - Extract 3 colors in the colorQueue
			//   - Call the draw triangle function 
			//     (you may put this function in GzFrameBuffer)

			while ((vertexQueue.size() >= 3) && (colorQueue.size() >= 3)) {

				vector<GzVertex> vertexTriangle;
				vector<GzColor> colorTriangle;
				GzMatrix M;

				for (int i = 0; i < 3; i++) {
					M.fromVertex(vertexQueue.front());
					M = prjMatrix * transMatrix * M;
					vertexTriangle.push_back(M.toVertex());
					affineTransform(vertexTriangle[i]);
					vertexQueue.pop();
					
					colorTriangle.push_back(colorQueue.front());
					colorQueue.pop();
				}
				frameBuffer.drawTriangle(vertexTriangle, colorTriangle, status);
			}
		} break;
	}
}

void Gz::viewport(GzInt x, GzInt y) {
	//This function only updates xViewport and yViewport.
	//Viewport calculation will be done in different function, e.g. Gz::end().
	//See http://www.opengl.org/sdk/docs/man/xhtml/glViewport.xml
	//Or google: glViewport
	xViewport=x;
	yViewport=y;
}

//Transformations-------------------------------------------------------------
void Gz::lookAt(GzReal eyeX, GzReal eyeY, GzReal eyeZ, GzReal centerX, GzReal centerY, GzReal centerZ, GzReal upX, GzReal upY, GzReal upZ) {
	//Define viewing transformation
	//See http://www.opengl.org/sdk/docs/man/xhtml/gluLookAt.xml
	//Or google: gluLookAt
	// 
	// ---------------------------------------- 
	// The following code is based on gluLookAt source code. References below:
	// https://www.scratchapixel.com/lessons/mathematics-physics-for-computer-graphics/lookat-function
	// https://computergraphics.stackexchange.com/questions/4668/correcting-my-look-at-matrix-so-that-it-works-on-non-camera-objects
	// https://www.khronos.org/opengl/wiki/GluLookAt_code
	// http://code.nabla.net/doc/OpenGL/api/OpenGL/man/gluLookAt.html
	// https://stackoverflow.com/questions/5717654/glulookat-explanation


	prjMatrix = Identity(4);
	transMatrix = Identity(4);

	GzReal forward[] = { centerX - eyeX, centerY - eyeY, centerZ - eyeZ };
	normalizeVector(forward);

	// -------------------------------------------------
	// Side = forward x up
	GzReal up[] = { upX, upY, upZ };
	normalizeVector(up);

	GzReal side[3];
	crossProduct(side, forward, up);
	normalizeVector(side);
	// --------------------------------------------------
	// Recompute up as: up = side x forward
	GzReal newUp[3];
	crossProduct(newUp, side, forward);

	// --------------------------------------------------
	/*
	* matrixM = {side[0], side[1], side[2], 0.0
	*			 up[0], up[1], up[2], 0.0
	*			 -forward[0], -forward[1], -forward[2], 0.0
	*			 0.0, 0.0, 0.0, 1.0}
	*/
	GzMatrix M;
	M.resize(4, 4);

	M.at(0)[0] = side[0];
	M.at(0)[1] = side[1];
	M.at(0)[2] = side[2];
	M.at(0)[3] = 0.0;

	M.at(1)[0] = newUp[0];
	M.at(1)[1] = newUp[1];
	M.at(1)[2] = newUp[2];
	M.at(1)[3] = 0.0;

	M.at(2)[0] = -forward[0];
	M.at(2)[1] = -forward[1];
	M.at(2)[2] = -forward[2];
	M.at(2)[3] = 0.0;

	M.at(3)[0] = 0.0;
	M.at(3)[1] = 0.0;
	M.at(3)[2] = 0.0;
	M.at(3)[3] = 1.0;

	// ------------------------------------------------------
	multMatrix(M);
	translate(-eyeX, -eyeY, -eyeZ);
}

void Gz::translate(GzReal x, GzReal y, GzReal z) {
	//Multiply transMatrix by a translation matrix
	//See http://www.opengl.org/sdk/docs/man/xhtml/glTranslate.xml
	//    http://en.wikipedia.org/wiki/Translation_(geometry)
	//Or google: glTranslate

	GzMatrix M;
	M.resize(4, 4);

	M.at(0)[0] = 1;
	M.at(0)[1] = 0;
	M.at(0)[2] = 0;
	M.at(0)[3] = x;

	M.at(1)[0] = 0;
	M.at(1)[1] = 1;
	M.at(1)[2] = 0;
	M.at(1)[3] = y;

	M.at(2)[0] = 0;
	M.at(2)[1] = 0;
	M.at(2)[2] = 1;
	M.at(2)[3] = z;

	M.at(3)[0] = 0;
	M.at(3)[1] = 0;
	M.at(3)[2] = 0;
	M.at(3)[3] = 1;

	multMatrix(M);
}

void Gz::rotate(GzReal angle, GzReal x, GzReal y, GzReal z) {
	//Multiply transMatrix by a rotation matrix
	//See http://www.opengl.org/sdk/docs/man/xhtml/glRotate.xml
	//    http://en.wikipedia.org/wiki/Rotation_(geometry)
	//Or google: glRotate

	// http://docs.gl/gl3/glRotate

	angle = angle * M_PI / 180;
	GzReal c = cos(angle);
	GzReal s = sin(angle);

	GzMatrix M;
	M.resize(4, 4);

	GzReal v[] = { x,y,z };
	normalizeVector(v);

	M.at(0)[0] = v[X] * v[X] * (1 - c) + c;
	M.at(0)[1] = v[X] * v[Y] * (1 - c) - v[Z] * s;
	M.at(0)[2] = v[X] * v[Z] * (1 - c) + v[Y] * s;
	M.at(0)[3] = 0;

	M.at(1)[0] = v[Y] * v[X] * (1 - c) + v[Z] * s;
	M.at(1)[1] = v[Y] * v[Y] * (1 - c) + c;
	M.at(1)[2] = v[Y] * v[Z] * (1 - c) - v[X] * s;
	M.at(1)[3] = 0;

	M.at(2)[0] = v[X] * v[Z] * (1 - c) - v[Y] * s;
	M.at(2)[1] = v[Y] * v[Z] * (1 - c) + v[X] * s;
	M.at(2)[2] = v[Z] * v[Z] * (1 - c) + c;
	M.at(2)[3] = 0;

	M.at(3)[0] = 0;
	M.at(3)[1] = 0;
	M.at(3)[2] = 0;
	M.at(3)[3] = 1;

	multMatrix(M);
}

void Gz::scale(GzReal x, GzReal y, GzReal z) {
	//Multiply transMatrix by a scaling matrix
	//See http://www.opengl.org/sdk/docs/man/xhtml/glScale.xml
	//    http://en.wikipedia.org/wiki/
	//Or google: glScale

	// http://docs.gl/gl3/glScale
	GzMatrix M;
	M.resize(4, 4);

	M.at(0)[0] = x;
	M.at(0)[1] = 0;
	M.at(0)[2] = 0;
	M.at(0)[3] = 0;

	M.at(1)[0] = 0;
	M.at(1)[1] = y;
	M.at(1)[2] = 0;
	M.at(1)[3] = 0;

	M.at(2)[0] = 0;
	M.at(2)[1] = 0;
	M.at(2)[2] = z;
	M.at(2)[3] = 0;

	M.at(3)[0] = 0;
	M.at(3)[1] = 0;
	M.at(3)[2] = 0;
	M.at(3)[3] = 1;

	multMatrix(M);
}

//This function was updated on September 26, 2010
void Gz::multMatrix(GzMatrix mat) {
	//Multiply transMatrix by the matrix mat
	//transMatrix=mat*transMatrix;
	transMatrix = transMatrix * mat;
}
//End of Transformations------------------------------------------------------

//Projections-----------------------------------------------------------------
void Gz::perspective(GzReal fovy, GzReal aspect, GzReal zNear, GzReal zFar) {
	//Set up a perspective projection matrix
	//See http://www.opengl.org/sdk/docs/man/xhtml/gluPerspective.xml
	//Or google: gluPerspective

	// https://www.khronos.org/opengl/wiki/GluPerspective_code
	
	// convert fovy angle from degree to radian
	fovy = fovy * M_PI / 180;

	GzReal f = tan(fovy / 2);

	GzMatrix M;
	M.resize(4, 4);

	M.at(0)[0] = 1 / (aspect * f);
	M.at(0)[1] = 0.0;
	M.at(0)[2] = 0.0;
	M.at(0)[3] = 0.0;

	M.at(1)[0] = 0.0;
	M.at(1)[1] = 1 / f;
	M.at(1)[2] = 0.0;
	M.at(1)[3] = 0.0;

	M.at(2)[0] = 0.0;
	M.at(2)[1] = 0.0;
	M.at(2)[2] = (zFar + zNear) / (zFar - zNear);
	M.at(2)[3] = (2 * zFar * zNear) / (zFar - zNear);

	M.at(3)[0] = 0.0;
	M.at(3)[1] = 0.0;
	M.at(3)[2] = -1;
	M.at(3)[3] = 0.0;

	prjMatrix = M;
}

/*
* https://stackoverflow.com/questions/2571402/how-to-use-glortho-in-opengl
* http://docs.gl/gl3/glOrtho
* https://docs.microsoft.com/en-us/windows/win32/opengl/glortho
*/
void Gz::orthographic(GzReal left, GzReal right, GzReal bottom, GzReal top, GzReal nearVal, GzReal farVal) {
	//Set up a orthographic projection matrix
	//See http://www.opengl.org/sdk/docs/man/xhtml/glOrtho.xml
	//Or google: glOrtho

	GzReal tx = -(right + left) / (right - left);
	GzReal ty = -(top + bottom) / (top - bottom);
	GzReal tz = -(farVal + nearVal) / (farVal - nearVal);

	GzMatrix M;
	M.resize(4, 4);


	M.at(0)[0] = 2 / (right - left);
	M.at(0)[1] = 0.0;
	M.at(0)[2] = 0.0;
	M.at(0)[3] = tx;

	M.at(1)[0] = 0.0;
	M.at(1)[1] = 2 / (top - bottom);
	M.at(1)[2] = 0.0;
	M.at(1)[3] = ty;

	M.at(2)[0] = 0.0;
	M.at(2)[1] = 0.0;
	M.at(2)[2] = 2 / (farVal - nearVal);
	M.at(2)[3] = -tz;

	M.at(3)[0] = 0.0;
	M.at(3)[1] = 0.0;
	M.at(3)[2] = 0.0;
	M.at(3)[3] = 1.0;

	prjMatrix = M;
}
//End of Projections----------------------------------------------------------



/* 
* Additional helper functions
*/
// https://www.khanacademy.org/computing/computer-programming/programming-natural-simulations/programming-vectors/a/vector-magnitude-normalization
// https://stackoverflow.com/questions/10002918/what-is-the-need-for-normalizing-a-vector
void Gz::normalizeVector(GzReal* v) {
	GzReal mag = 0;

	for (int i = 0; i < 3; i++) {
		mag += v[i] * v[i];
	}
	mag = sqrt(mag);

	for (int i = 0; i < 3; i++) {
		v[i] = v[i] / mag;
	}
}

// https://tutorial.math.lamar.edu/Classes/CalcII/CrossProduct.aspx
// https://en.wikipedia.org/wiki/Cross_product
void Gz::crossProduct(GzReal* result, GzReal* a, GzReal* b) {
	result[0] = a[1] * b[2] - a[2] * b[1];
	result[1] = a[2] * b[0] - a[0] * b[2];
	result[2] = a[0] * b[1] - a[1] * b[0];
}

/*
* Viewport Calculation
* specifies the affine transformation of x and y from normalized device coordinates to window coordinates
* https://www.khronos.org/registry/OpenGL-Refpages/gl4/html/glViewport.xhtml
* https://www.khronos.org/registry/OpenGL-Refpages/gl2.1/xhtml/gluProject.xml
*/
void Gz::affineTransform(GzVertex& v) {
	v[X] = (v[X] + 1) * (wViewport / 2) + xViewport;
	v[Y] = (v[Y] + 1) * (hViewport / 2) + yViewport;
	v[Z] = (v[Z] + 1) / 2;
}
//============================================================================
//End of Implementations in Assignment #3
//============================================================================
