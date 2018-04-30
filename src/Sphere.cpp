#include "Sphere.h"
#include <ngl/VAOPrimitives.h>
#include <iostream>
#include <map>


Boid::Boid(ngl::Vec3 _pos, ngl::Vec3 _vel, ngl::Vec3 _accel,  GLfloat _rad)
{
  // set values from params
  m_pos=_pos;
  m_vel=_vel;
  m_radius=_rad;
  m_accel = _accel;
  //m_hit=false;
}

Boid::Boid()
{
  //m_hit=false;
}

void Boid::loadMatricesToShader( ngl::Transformation &_tx, const ngl::Mat4 &_globalMat, ngl::Camera *_cam  ) const
{
  ngl::ShaderLib *shader=ngl::ShaderLib::instance();

  ngl::Mat4 MV;
  ngl::Mat4 MVP;
  ngl::Mat3 normalMatrix;
  MV=_cam->getViewMatrix()  *_globalMat* _tx.getMatrix();
  MVP=_cam->getProjectionMatrix()*MV;
  normalMatrix=MV;
  normalMatrix.inverse().transpose();
  shader->setUniform("MVP",MVP);
  shader->setUniform("normalMatrix",normalMatrix);
}


void Boid::draw( const std::string &_shaderName, const ngl::Mat4 &_globalMat,  ngl::Camera *_cam )const
{

  // draw wireframe if hit
  /*if(m_hit)
  {
    glPolygonMode(GL_FRONT_AND_BACK,GL_LINE);
  }
  else
  {
    glPolygonMode(GL_FRONT_AND_BACK,GL_FILL);
  }*/


  ngl::ShaderLib *shader=ngl::ShaderLib::instance();
  shader->use(_shaderName);
  // grab an instance of the primitives for drawing
  ngl::VAOPrimitives *prim=ngl::VAOPrimitives::instance();
  ngl::Transformation t;

  t.setPosition(m_pos);
  t.setScale(m_radius,m_radius,m_radius);
  loadMatricesToShader(t,_globalMat,_cam);
  prim->draw("Boid");

  //glPolygonMode(GL_FRONT_AND_BACK,GL_FILL);
}



void Boid :: set(ngl::Vec3 _pos, ngl::Vec3 _vel, ngl::Vec3 _accel, GLfloat _rad)
{
  m_pos=_pos;
  m_vel=_vel;
  m_accel = _accel;
  m_radius=_rad;
}

void Boid::move()
{
  m_vel += m_accel;
  m_vel.clamp(20.0);
  // store the last position
  //m_lastPos=m_pos;
  // update the current position
  m_pos+=m_vel;
  // get the next position
  //m_nextPos=m_pos+m_vel;
  //m_hit=false;
  m_accel*=0.0;


}

void Boid::seek(ngl::Vec3 _target)
{


    float maxSpeed = 2.0;
    float maxForce = 0.2;
    ngl::Vec3 desired = _target - m_pos;
    desired.normalize();
    desired *= 0.05;
    ngl::Vec3 steer = desired - m_vel;
    std::cout<<"yes";
    steer.clamp(maxForce);
    applyForce(steer);
}

void Boid::applyForce(ngl::Vec3 _force)
{
    m_accel += _force;
}

void Boid::arrive(ngl::Vec3 _target)
{


    ngl::Vec3 desired = _target - m_pos;
    float d = desired.length();//get the distance
    desired.normalize();

     float maxSpeed = 20.0;
     float maxForce = 0.2;

    if (d<20) //test if the boid is close
    {
        //set the magnitude according to how close we are.

        desired*= d/10;
    }
    else
    {
        desired*=maxSpeed; //Otherwise, proceed at maximum speed.
    }
    ngl::Vec3 steer = desired - m_vel;
    steer.clamp(maxForce);
    applyForce(steer);


}

void Boid::separate(std::vector <Boid> _BoidArray)
{
  float maxSpeed = 20.0;
  float maxForce = 0.2;
  float desiredSeparation = m_radius*2; //detects how close the boids are one from the other
  ngl::Vec3 sum(0.0,0.0,0.0);
  int count = 0;//count how many boids are in that area

  for(Boid &s : _BoidArray)
  {
    ngl::Vec3 dist = m_pos - s.getPos();//vector distance(desired velocity)
    float d = dist.length();//get the distance between the boid and its neighbour
    //check if the boid is within the desiredSeparation
    if((d>0)&&(d<desiredSeparation)) //d>0 so it doesn't check against itself
    {

      dist.normalize();
      dist/=d;//flees more if it's closer
      sum+=dist;//sums up all the desired velocity vectors of the neighbouring boids
      count++;

    }

  }
  if(count > 0)//if neighbouring boids are found
  {
    sum/=count;//get the average vector
    sum.normalize();
    sum*=maxSpeed;
    ngl::Vec3 steer = sum - m_vel;//sum is the desired velocity
    steer.clamp(maxForce);
    applyForce(steer);

  }
}

void Boid::align(std::vector <Boid> _BoidArray)
{
  float maxSpeed = 20.0;
  float maxForce = 0.2;

  float neighbourDist = 50;
  ngl::Vec3 sum(0.0,0.0,0.0);
  int count = 0;
  for(Boid &s : _BoidArray)
  {
    ngl::Vec3 dist = m_pos - s.getPos();//vector distance(desired velocity)
    float d = dist.length();//get the distance between the boid and its neighbour
    if((d>0)&&(d<neighbourDist))
    {
      sum+=s.getVelocity();//add up all the velocities of the boids
      count++;
    }
  }
  if(count>0)
  {
    sum/=count;//divide by the total to get the average velocity
    sum.normalize();
    sum*=maxSpeed;//go at maxSpeed
    ngl::Vec3 steer = sum - m_vel;//sum is the desired velocity
    steer.clamp(maxForce);
    applyForce(steer);
  }
}

void Boid::cohesion(std::vector <Boid> _BoidArray)
{
  float maxSpeed = 20.0;
  float maxForce = 0.2;

  float neighbourDist = 50;
  ngl::Vec3 sum(0.0,0.0,0.0);
  int count = 0;
  for(Boid &s : _BoidArray)
  {
    ngl::Vec3 dist = m_pos - s.getPos();//vector distance(desired velocity)
    float d = dist.length();//get the distance between the boid and its neighbour
    if((d>0)&&(d<neighbourDist))
    {
      sum+=s.getPos();//add up all the positions of the boids
      count++;
    }
  }
  if(count>0)
  {
    sum/=count;
    seek(sum);
  }

}



void Boid::avoidWalls()
{
  if(m_pos.m_x > 25)
  {
    ngl::Vec3 desired;
    desired.m_x = 2.0;
    desired.m_y = m_vel.m_y;

    ngl::Vec3 steer = desired - m_vel;
    steer.clamp(2.0);
    applyForce(steer);
  }




}






