//requried to use M_PI
#define _USE_MATH_DEFINES
#define _G
#include <cmath>

#include <locale>
#include <math.h>
#include <sstream>
#include <string>
#include <fstream>
#include <iostream>

using namespace std;

#define ToRadian(x) ((x) * M_PI / 180.0f)
#define ToDegree(x) ((x) * 180.0f / M_PI)

struct Matrix4f {
  float m[4][4];
};

struct CameraInfo{
  float Pos[3];
  float Target[3];
  float Up[3];
};

struct ProjInfo {
  float FOV;
  float Width;
  float Height;
  float zNear;
  float zFar;
};

class Pipeline
{
public:
  Pipeline(){
    m_scale[0] = 1.f; m_scale[1] = 1.f, m_scale[2] = 1.f;
    m_pos[0] = 0.f; m_pos[1] = 0.f, m_pos[2] = 0.f;
    m_rotate[0] = 0.f; m_rotate[1] = 0.f, m_rotate[2] = 0.f;
  }

  void Scale(float x, float y, float z){
    m_scale[0] = x*0.5f; m_scale[1] = y; m_scale[2] = z;
  }

  void Translate(float x, float y, float z){
    m_pos[0] = x; m_pos[1] = y; m_pos[2] = z;
  }

  void Rotate(float x, float y, float z){
    m_rotate[0] = x; m_rotate[1] = y; m_rotate[2] = z;
  }

  void SetCamera(float Pos[3], float Target[3], float Up[3]){
    memcpy(&m_camera.Pos, Pos, sizeof(float)*3);
    memcpy(&m_camera.Target, Target, sizeof(float)*3);
    memcpy(&m_camera.Up, Up, sizeof(float)*3);
  }

  const Matrix4f * GetTrans();
  const Matrix4f * GetCTrans();

private:
  float m_scale[3];
  float m_pos[3];
  float m_rotate[3];
  Matrix4f m_transform;
  ProjInfo m_projInfo;
  CameraInfo m_camera;
};

//new line reader from http://stackoverflow.com/questions/6089231/getting-std-ifstream-to-handle-lf-cr-and-crlf
std::istream& safeGetline(std::istream& is, std::string& t)
{
	t.clear();

	// The characters in the stream are read one-by-one using a std::streambuf.
	// That is faster than reading them one-by-one using the std::istream.
	// Code that uses streambuf this way must be guarded by a sentry object.
	// The sentry object performs various tasks,
	// such as thread synchronization and updating the stream state.

	std::istream::sentry se(is, true);
	std::streambuf* sb = is.rdbuf();

	for (;;) {
		int c = sb->sbumpc();
		//cout << c << endl; //Debug code to see what is being parsed
		switch (c) {
		case '\n':
			return is;
		case '\r':
			if (sb->sgetc() == '\n')
				sb->sbumpc();
			return is;
		case EOF:
			// Also handle the case when the last line has no line ending
			if (t.empty())
				is.setstate(std::ios::eofbit);
			return is;
		default:
			t += (char)c;
		}
	}
}

//gets the location of the file path in windows
//string ExePath() {
//	char buffer[MAX_PATH];
//	GetModuleFileName(NULL, buffer, MAX_PATH);
//	string::size_type pos = string(buffer).find_last_of("\\/");
//	return string(buffer).substr(0, pos);
//}


void ReadMesh(GLfloat *v, unsigned int* i, const char * v_file, const char * i_file){
  FILE * fpv = NULL;
  FILE * fpi = NULL;
  char * line = NULL;
  size_t len = 0;
  size_t read;

 
  //fpi = fopen(i_file, "r");
  //if (fpv == NULL) 
  //  exit(EXIT_FAILURE);
  
  int v_i = 0;
  int i_i = 0;
  GLfloat x, y, z;
  unsigned int a, b, c;
  //new code


  std::string path = v_file; // path to file (v first)
  path = path.erase(0, 2); // remove unix ./ file path
  //file opening code
  //FILE *vFile;
  //errno_t err;
  //err = fopen_s(&vFile, "crt_fopen_s.c", "r");
  
  
  //ifstream testFile;
  //testFile.open("C:\test.txt");
  //string testb;
  //safeGetline(testFile, testb);

  //cout << testb << endl;
 
  /*FILE* vfile = fopen(path.c_str(), "r");
  if (vfile == NULL)
	  exit(EXIT_FAILURE);
  */ 

  ifstream vfile(path.c_str());

  //making sure the file stream is open
  if (!vfile.is_open()) { //is_open reaturns true if working
	std::cout << ("Failed to open the file.  " + path) << std::endl;
	return;
  }

  int n = 0;
  string t;

  while (!safeGetline(vfile,t).eof()){ // read the .v file
	  sscanf_s(t.c_str(), "%f %f %f\n", &x, &y, &z); //string is converted to constant char
	  v[v_i] = x; 
	  v_i += 1;
	  v[v_i] = y;
	  v_i += 1;
	  v[v_i] = z;
	  v_i += 1;
  }
  //this will print garbage data if v[0 to 2] is not set
  std::cout << "printing x,y,z " << v[0] << "," << v[1] << "," << v[2] << "," << std::endl;

  path = i_file; // path to file .i file
  path = path.erase(0, 2);
  ifstream iFile(i_file);
  if (!iFile.is_open()) {
	 std::cout << "Failed to open the file." << std::endl;
  }

  n = 0;
  while (!safeGetline(vfile, t).eof()) { // read the .v file
	  sscanf_s(t.c_str(), "%d %d %d\n", &a, &b, &c); //string is converted to constant char
	  i[i_i] = a;
	  i_i += 1;
	  i[i_i] = b;
	  i_i += 1;
	  i[i_i] = c;
	  i_i += 1;
  }

  //new code end

  // requires unix platform to pars file
  //while ((read = getline(&line, &len, fpv)) != -1) {
  //  sscanf(line, "%f %f %f\n", &x, &y, &z);
  //  v[v_i] = x;
  //  v_i += 1;
  //  v[v_i] = y;
  //  v_i += 1;
  //  v[v_i] = z;
  //  v_i += 1;
  //}

  //while ((read = getline(&line, &len, fpi)) != -1) {
  //  sscanf(line, "%d %d %d\n", &a, &b, &c);
  //  i[i_i] = a;
  //  i_i += 1;
  //  i[i_i] = b;
  //  i_i += 1;
  //  i[i_i] = c;
  //  i_i += 1;
  //}
	//these can't be called if there perameters are null
  if(fpv != NULL)
	fclose(fpv);
  if(fpi != NULL)
	fclose(fpi);
  
  vfile.close();
 

  if(line) free(line);
}

void Cross(const float left[3], const float right[3], float * result)
{
 result[0] = left[1]*right[2] - left[2]*right[1];
 result[1] = left[2]*right[0] - left[0]*right[2];
 result[2] = left[0]*right[1] - left[1]*right[0];
}

void Normalize(float * v)
{
  float d = sqrt(pow(v[0], 2) + pow(v[1], 2) + pow(v[2], 2));
  v[0] = v[0]/d;
  v[1] = v[1]/d;
  v[2] = v[2]/d;
}

Matrix4f MultMatrices(Matrix4f Left, Matrix4f Right)
{
  Matrix4f Ret;
  for (unsigned int i=0; i < 4; i ++){
    for (unsigned int j=0; j<4; j++){
      Ret.m[i][j] = Left.m[i][0] * Right.m[0][j] +
                    Left.m[i][1] * Right.m[1][j] +
                    Left.m[i][2] * Right.m[2][j] +
                    Left.m[i][3] * Right.m[3][j];
    }
  }
  return Ret;
}


const Matrix4f * Pipeline::GetTrans(){
  Matrix4f st, rx, ry, rz, tt;
  st.m[0][0] = m_scale[0]; st.m[0][1] = 0.f; st.m[0][2] = 0.f; st.m[0][3] = 0.f;
  st.m[1][0] = 0.f; st.m[1][1] = m_scale[1]; st.m[1][2] = 0.f; st.m[1][3] = 0.f;
  st.m[2][0] = 0.f; st.m[2][1] = 0.f; st.m[2][2] = m_scale[2]; st.m[2][3] = 0.f;
  st.m[3][0] = 0.f; st.m[3][1] = 0.f; st.m[3][2] = 0.f; st.m[3][3] = 1.f;

  const float x = ToRadian(m_rotate[0]);
  const float y = ToRadian(m_rotate[1]);
  const float z = ToRadian(m_rotate[2]);
  rx.m[0][0] = 1.f; rx.m[0][1] = 0.f;     rx.m[0][2] = 0.f;      rx.m[0][3] = 0.f;
  rx.m[1][0] = 0.f; rx.m[1][1] = cosf(x); rx.m[1][2] = -sinf(x); rx.m[1][3] = 0.f;
  rx.m[2][0] = 0.f; rx.m[2][1] = sinf(x); rx.m[2][2] = cosf(x);  rx.m[2][3] = 0.f;
  rx.m[3][0] = 0.f; rx.m[3][1] = 0.f;     rx.m[3][2] = 0.f;      rx.m[3][3] = 1.f;
 
  ry.m[0][0] = cosf(y); ry.m[0][1] = 0.f; ry.m[0][2] = -sinf(y); ry.m[0][3] = 0.f;
  ry.m[1][0] = 0.f;     ry.m[1][1] = 1.f; ry.m[1][2] = 0.f;      ry.m[1][3] = 0.f;
  ry.m[2][0] = sinf(y); ry.m[2][1] = 0.f; ry.m[2][2] = cosf(y);  ry.m[2][3] = 0.f;
  ry.m[3][0] = 0.f;     ry.m[3][1] = 0.f; ry.m[3][2] = 0.f;      ry.m[3][3] = 1.f;

  rz.m[0][0] = cosf(z); rz.m[0][1] = -sinf(z); rz.m[0][2] = 0.f; rz.m[0][3] = 0.f;
  rz.m[1][0] = sinf(z); rz.m[1][1] = cosf(z);  rz.m[1][2] = 0.f; rz.m[1][3] = 0.f;
  rz.m[2][0] = 0.f;     rz.m[2][1] = 0.f;      rz.m[2][2] = 1.f; rz.m[2][3] = 0.f;
  rz.m[3][0] = 0.f;     rz.m[3][1] = 0.f;      rz.m[3][2] = 0.f; rz.m[3][3] = 1.f;
 
  tt.m[0][0] = 1.f; tt.m[0][1] = 0.f; tt.m[0][2] = 0.f; tt.m[0][3] = m_pos[0];
  tt.m[1][0] = 0.f; tt.m[1][1] = 1.f; tt.m[1][2] = 0.f; tt.m[1][3] = m_pos[1];
  tt.m[2][0] = 0.f; tt.m[2][1] = 0.f; tt.m[2][2] = 1.f; tt.m[2][3] = m_pos[2];
  tt.m[3][0] = 0.f; tt.m[3][1] = 0.f; tt.m[3][2] = 0.f; tt.m[3][3] = 1.f;
  
  Matrix4f rotate = MultMatrices(MultMatrices(rx, ry), rz);
  m_transform = MultMatrices(MultMatrices(tt, st), rotate);
  return &m_transform;
}

const Matrix4f * Pipeline::GetCTrans()
{

  Normalize((float *)m_camera.Target);
  float * N = (float *)m_camera.Target;
  Normalize((float *)m_camera.Up);
  float * U = (float *)m_camera.Up;
  float V[3];
  
  Cross(N, U, V);

  Matrix4f WorldTransform = *(GetTrans());

  Matrix4f cr; // Camera rotate
  cr.m[0][0] = U[0]; cr.m[0][1] = U[1]; cr.m[0][2] = U[2]; cr.m[0][3] = 0.0f;
  cr.m[1][0] = V[0]; cr.m[1][1] = V[1]; cr.m[1][2] = V[2]; cr.m[1][3] = 0.0f;
  cr.m[2][0] = N[0]; cr.m[2][1] = N[1]; cr.m[2][2] = N[2]; cr.m[2][3] = 0.0f;
  cr.m[3][0] = 0.0f; cr.m[3][1] = 0.0f; cr.m[3][2] = 0.0f; cr.m[3][3] = 1.0f;

  Matrix4f cp; // Camera position
  cp.m[0][0] = 1.f; cp.m[0][1] = 0.f; cp.m[0][2] = 0.f; cp.m[0][3] = -m_camera.Pos[0];
  cp.m[1][0] = 0.f; cp.m[1][1] = 1.f; cp.m[1][2] = 0.f; cp.m[1][3] = -m_camera.Pos[1];
  cp.m[2][0] = 0.f; cp.m[2][1] = 0.f; cp.m[2][2] = 1.f; cp.m[2][3] = -m_camera.Pos[2];
  cp.m[3][0] = 0.f; cp.m[3][1] = 0.f; cp.m[3][2] = 0.f; cp.m[3][3] = 1.f;


  m_transform = MultMatrices(MultMatrices(cr, cp), WorldTransform); 
  return &m_transform;

}


/*
void Pipeline::InitPerspectiveProj(Matrix4f * m){
  const float ar = m_persProjInfo.Width / m_persProjInfo.Height;
  const float zNear = m_persProjInfo.zNear;
  const float zFar = m_persProjInfo.zFar;
  const float zRange = zNear - zFar;
  const float tanHalfFOV = tanf(ToRadian(m_persProjInfo.FOV / 2.0f));

  m->m[0][0] = 1.0f / (tanHalfFOV * ar);
  m->m[0][1] = 0.f; m->m[0][2] = 0.f; m->m[0][3] = 0.f;

  m->m[1][1] = 1.0f / tanHalfFOV;
  m->m[1][0] = 0.f; m->m[1][2] = 0.f; m->m[1][3] = 0.f;

  m->m[2][0] = 0.f; m->m[2][1] = 0.f;
  m->m[2][2] = (-zNear - zFar) / zRange;
  m->m[2][3] = 2.0f * zFar * zNear / zRange;

  m->m[3][0] = 0.f; m->m[3][1] = 0.f; m->m[3][2] = 1.f; m->m[3][3] = 0.f;
}


const Matrix4f * Pipeline::GetPTrans(){
  GetTrans();
  Matrix4f PP;
  InitPerspectiveProj(&PP);
  m_transformation = MultMatrices(PP, m_transformation);
  return &m_transformation;
}

*/








