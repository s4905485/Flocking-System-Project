#ifndef Boid_H_
#define Boid_H_


#include <ngl/Camera.h>
#include <ngl/ShaderLib.h>
#include <ngl/Transformation.h>
#include <ngl/Vec3.h>

/*! \brief a simple Boid class */
class Boid
{
public :
	/// @brief ctor
    /// @param pos the position of the Boid
    /// @param rad the radius of the Boid */
    Boid(ngl::Vec3 _pos,  ngl::Vec3 _vel, ngl::Vec3 _accel,	GLfloat _rad);
    Boid();
  /// draw method
  void draw(const std::string &_shaderName,const ngl::Mat4 &_globalMat, ngl::Camera *_cam )const ;
  void loadMatricesToShader(ngl::Transformation &_tx, const ngl::Mat4 &_globalMat,ngl::Camera *_cam )const;
  inline void reverse(){m_vel=m_vel*-1.0;} //reverse the velocity
  //inline void setHit(){m_hit=true;}
  //inline void setNotHit(){m_hit=false;}
  //inline bool isHit()const {return m_hit;}
  inline ngl::Vec3 getPos() const {return m_pos;}
  inline ngl::Vec3 getNextPos() const {return m_nextPos;}
  inline GLfloat getRadius() const {return m_radius;}
  inline void setVelocity(ngl::Vec3 _v){m_vel=_v;}
  inline ngl::Vec3 getVelocity() const { return m_vel;}

  inline void setAcceleration(ngl::Vec3 _a){m_accel=_a;}
  inline ngl::Vec3 getAcceleration() const { return m_accel;}
  void move();
    /// set the Boid values
	/// @param[in] _pos the position to set
    /// @param[in] _Vel the velocity of the Boid
    /// @param[in] _rad the radius of the Boid

   void set(ngl::Vec3 _pos, ngl::Vec3 _vel, ngl::Vec3 _accel, GLfloat _rad) ;
   void seek(ngl::Vec3 _target);
   void applyForce(ngl::Vec3 _force);
   void arrive(ngl::Vec3 _target);
   void separate(std::vector <Boid> _BoidArray);
   void align(std::vector <Boid> _BoidArray);
   void cohesion(std::vector <Boid> _BoidArray);

   void avoidWalls();
private :
    /*! the position of the Boid */
  ngl::Vec3 m_pos;
    /*! the radius of the Boid */
  GLfloat m_radius;
    /*! flag to indicate if the Boid has been hit by ray */
  //bool m_hit;
    // the velocity of the Boid
  ngl::Vec3 m_vel;
    //the steering acceleration
  ngl::Vec3 m_accel;
    // the last position of the Boid
  ngl::Vec3 m_lastPos;
    // the next position of the Boid
  ngl::Vec3 m_nextPos;


};





#endif
