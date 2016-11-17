#include <math.h>

#define ToRadian(x) ((x) * M_PI / 180.0f)
#define ToDegree(x) ((x) * 180.0f / M_PI)


struct Matrix4f {
  float m[4][4];
};

struct PersProjInfo {
  float FOV;
  float Width;
  float Height;
  float zNear;
  float zFar;
};

struct OrthoProjInfo {
  float Right;
  float Left;
  float Bottom;
  float Top;
  float Near;
  float Far;
};

struct Vector3f
{
  float x;
  float y;
  float z;

  Vector3f() {}

  Vector3f(float _x, float _y, float _z){
    x = _x;
    y = _y;
    z = _y;
  }

  Vector3f(float f){
    x = y = z = f;
  }

  Vector3f& operator+=(const Vector3f& r){
    x += r.x;
    y += r.y;
    z += r.z;
    return *this;
  }

  Vector3f& operator-=(const Vector3f& r){
    x -= r.x;
    y -= r.y;
    z -= r.z;
    return *this;
  }

  Vector3f& operator*=(const Vector3f& r){
    x *= r.x;
    y *= r.y;
    z *= r.z;
    return *this;
  }

  operator const float*() const {
    return &(x);
  }

  Vector3f Cross(const Vector3f& v) const;
  Vector3f Normalize();
  void Rotate(float Angle, const Vector3f& Axis);
};
/*
inline Vector3f& operator+(const Vector3f& l, const Vector3f& r){
  Vector3f Ret(l.x + r.x,
               l.y + r.y,
               l.z + r.z);
  return Ret;
}

inline Vector3f& operator-(const Vector3f& l, const Vector3f& r){
  Vector3f Ret(l.x - r.x,
               l.y - r.y,
               l.z - r.z);
  return Ret;
}

inline Vector3f& operator*(const Vector3f& l, float f){
  Vector3f Ret(l.x * f,
               l.y * f,
               l.z * f);
  return Ret;
}

*/

class Pipeline
{
public:
  Pipeline(){
    m_scale      = Vector3f(1.0f, 1.0f, 1.0f);
    m_worldPos   = Vector3f(0.0f, 0.0f, 0.0f);
    m_rotateInfo = Vector3f(0.0f, 0.0f, 0.0f);
  }
  void Scale(float ScaleX, float ScaleY, float ScaleZ){
    m_scale.x = ScaleX;
    m_scale.y = ScaleY;
    m_scale.z = ScaleZ;
  }
  
  void Translate(float x, float y, float z){
    m_worldPos.x = x;
    m_worldPos.y = y;
    m_worldPos.z = z;
  }
  
  void Rotate(float RotateX, float RotateY, float RotateZ);
  const Matrix4f* GetTrans();
  const Matrix4f* GetPTrans();
  void InitPerspectiveProj(Matrix4f * m);

  void SetPerspectiveProj(PersProjInfo p){
    m_persProjInfo = p;
  }

  void SetPerspectiveProj(float FOV, float Width, float Height, float zNear, float zFar){
    m_persProjInfo.FOV = FOV;
    m_persProjInfo.Width = Width;
    m_persProjInfo.Height = Height;
    m_persProjInfo.zNear = zNear;
    m_persProjInfo.zFar = zFar;
  }

  void SetOrthoProj(OrthoProjInfo p){
    m_orthoProjInfo = p;
  }

private:
  Vector3f m_scale;
  Vector3f m_worldPos;
  Vector3f m_rotateInfo;
  Matrix4f m_transformation;

  PersProjInfo m_persProjInfo;
  OrthoProjInfo m_orthoProjInfo;
};











