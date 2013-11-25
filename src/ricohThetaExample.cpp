#include "ricohThetaExample.h"
#define _USE_MATH_DEFINES
#include <math.h>
#include <fstream>

ricohThetaExample::ricohThetaExample(int argc, char* argv[])
	: ofBaseApp()
{
	if(argc == 2) filename = argv[1];
	else filename = "E:\\Pictures\\RICOH THETA\\R0010001.JPG";
}

int ricohThetaExample::get_angles(const char* filename)
{
	std::ifstream fin;
	const int size = 10*1000;
	unsigned char head[size];
	fin.open(filename, std::ios::in | std::ios::binary);
	if(fin.fail()){
		std::cerr << filename << " open failed." << std::endl;
		return -1;
	}
	fin.read( (char*)head,size);

	union {
		int i;
		unsigned int ui;
		unsigned char b[4];
	} bytes;

	for(int i=0; i<size; i++){
		unsigned char *buf = head + i;
		if(buf[0] == 0x00 && buf[1] == 0x04 && buf[2] == 0x00 && buf[3] == 0x05 &&
			buf[4] == 0x00 && buf[5] == 0x00 && buf[6] == 0x00 && buf[7] == 0x01) {

				// big endian to little endian
				bytes.b[0] = buf[3+8];
				bytes.b[1] = buf[2+8];
				bytes.b[2] = buf[1+8];
				bytes.b[3] = buf[0+8];
				unsigned int offset = bytes.ui + 12;

				bytes.b[0] = head[offset+3];
				bytes.b[1] = head[offset+2];
				bytes.b[2] = head[offset+1];
				bytes.b[3] = head[offset+0];
				unsigned int a = bytes.ui;

				bytes.b[0] = head[offset+7];
				bytes.b[1] = head[offset+6];
				bytes.b[2] = head[offset+5];
				bytes.b[3] = head[offset+4];
				unsigned int b = bytes.ui;

				compass = (float)a / (float)b;
				std::cout << "compass = " << compass << std::endl;
				break;
		}
	}

	for(int i=0; i<size; i++){
		unsigned char *buf = head + i;
		if(buf[0] == 0x00 && buf[1] == 0x03 && buf[2] == 0x00 && buf[3] == 0x0a &&
			buf[4] == 0x00 && buf[5] == 0x00 && buf[6] == 0x00 && buf[7] == 0x02) {

				// big endian to little endian
				bytes.b[0] = buf[3+8];
				bytes.b[1] = buf[2+8];
				bytes.b[2] = buf[1+8];
				bytes.b[3] = buf[0+8];
				unsigned int offset = bytes.ui + 12;

				bytes.b[0] = head[offset+3];
				bytes.b[1] = head[offset+2];
				bytes.b[2] = head[offset+1];
				bytes.b[3] = head[offset+0];
				int a = bytes.i;

				bytes.b[0] = head[offset+7];
				bytes.b[1] = head[offset+6];
				bytes.b[2] = head[offset+5];
				bytes.b[3] = head[offset+4];
				int b = bytes.i;

				zenith_x = (float)a / (float)b;
				std::cout << "zenith_x = " << zenith_x << std::endl;

				bytes.b[0] = head[offset+3+8];
				bytes.b[1] = head[offset+2+8];
				bytes.b[2] = head[offset+1+8];
				bytes.b[3] = head[offset+0+8];
				a = bytes.i;

				bytes.b[0] = head[offset+7+8];
				bytes.b[1] = head[offset+6+8];
				bytes.b[2] = head[offset+5+8];
				bytes.b[3] = head[offset+4+8];
				b = bytes.i;

				zenith_y = (float)a / (float)b;
				std::cout << "zenith_y = " << zenith_y << std::endl;
				break;
		}
	}
	fin.close();

	return 0;
}

//--------------------------------------------------------------
void ricohThetaExample::setup() {
	get_angles(filename.data());

	ofSetVerticalSync(true);

	// load an image from disk
	img.loadImage(filename);

	// loop through the image in the x and y axes
	float r = 10.0f; // radius

	ofEnableDepthTest();
	ofEnableNormalizedTexCoords();
	glEnable(GL_POINT_SMOOTH); // use circular points instead of square points
	glPointSize(3); // make the points bigger
	glEnable(GL_CULL_FACE); 
	glCullFace(GL_BACK);

	cam.setAutoDistance(false);
	cam.setPosition(0,0,0);
	cam.lookAt(ofVec3f(-r,0,0),ofVec3f(0,0,1));
	cam.setDistance(1.0);
}

//--------------------------------------------------------------
void ricohThetaExample::update() {
	// keep horizontal
	ofVec3f updir = cam.getUpDir();
	ofVec3f lookdir = cam.getLookAtDir();
	ofVec3f hordir = updir.getCrossed(lookdir);
	ofQuaternion quat;
	quat.makeRotate(ofVec3f(hordir.x,hordir.y,0).normalized(),hordir.normalized());
	ofMatrix4x4 upmat(quat);
	ofVec3f new_updir = upmat*updir;
	cam.lookAt(lookdir, new_updir);
}

//--------------------------------------------------------------
void ricohThetaExample::draw() {
	ofBackgroundGradient(ofColor::gray, ofColor::black, OF_GRADIENT_CIRCULAR);
	
	// keep zenith
	ofQuaternion qzx, qzy, qzz;
	qzx.set(sin(zenith_x/2*M_PI/180.0),0,0,cos(zenith_x/2*M_PI/180.0));
	qzy.set(0,sin(-zenith_y/2*M_PI/180.0),0,cos(-zenith_y/2*M_PI/180.0));
	qzz.set(0,0,sin(compass/2*M_PI/180.0),cos(compass/2*M_PI/180.0));
	ofMatrix4x4 m44(qzx*qzy);
	ofMatrix4x4 m_yaw(qzz);

	// even points can overlap with each other, let's avoid that
	cam.begin();
	ofPushMatrix();
	//ofMultMatrix(m_yaw); // adjust north
	ofMultMatrix(m44.getInverse());
	ofMatrix4x4 rot(0,-1,0,0,
		0,0,-1,0,
		1,0,0,0,
		0,0,0,1);
	ofMultMatrix(rot);
	img.bind();
	ofSphere(10);
	img.unbind();
	ofPopMatrix();
	cam.end();
}

//--------------------------------------------------------------
void ricohThetaExample::keyPressed(int key){
}

//--------------------------------------------------------------
void ricohThetaExample::keyReleased(int key){

}

//--------------------------------------------------------------
void ricohThetaExample::mouseMoved(int x, int y){

}

//--------------------------------------------------------------
void ricohThetaExample::mouseDragged(int x, int y, int button){

}

//--------------------------------------------------------------
void ricohThetaExample::mousePressed(int x, int y, int button){

}

//--------------------------------------------------------------
void ricohThetaExample::mouseReleased(int x, int y, int button){

}

//--------------------------------------------------------------
void ricohThetaExample::windowResized(int w, int h){

}

//--------------------------------------------------------------
void ricohThetaExample::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void ricohThetaExample::dragEvent(ofDragInfo dragInfo){ 
	if( dragInfo.files.size() > 0 ){
		img.loadImage(dragInfo.files[0]);
	}
}