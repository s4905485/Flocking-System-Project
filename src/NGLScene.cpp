#include <QMouseEvent>
#include <QGuiApplication>

#include "NGLScene.h"
#include <ngl/Camera.h>
#include <ngl/Light.h>
#include <ngl/Random.h>
#include <ngl/ShaderLib.h>
#include <ngl/Material.h>
#include <ngl/NGLInit.h>
#include <ngl/VAOPrimitives.h>


//----------------------------------------------------------------------------------------------------------------------
/// @brief extents of the bbox
//----------------------------------------------------------------------------------------------------------------------
const static int s_extents=20;

NGLScene::NGLScene(int _numBoids)
{
  setTitle("Flocking System Trial");
  m_animate=true;
  //m_checkBoidBoid=false;
  // create vectors for the position and velocity
  m_numBoids=_numBoids;
  setupBoids();

}

void NGLScene::setupBoids()
{
    m_BoidArray.clear();
    ngl::Vec3 vel(0.02,0.02,0.02);
    ngl::Vec3 accel(0.02,0.02,0.02);
	ngl::Random *rng=ngl::Random::instance();
	// loop and create the initial particle list
    for(int i=0; i<m_numBoids; ++i)
	{
        //vel=rng->getRandomVec3();
        //accel=rng->getRandomVec3();
        // add the Boids to the end of the particle list
    m_BoidArray.push_back(Boid(rng->getRandomPoint(s_extents,s_extents,s_extents),vel,accel,rng->randomPositiveNumber(2)+0.5f));
    //m_BoidArray.push_back(Boid(rng->getRandomPoint(s_extents,s_extents,s_extents),,rng->randomPositiveNumber(2)+0.5f) );
	}

}
NGLScene::~NGLScene()
{
  std::cout<<"Shutting down NGL, removing VAO's and Shaders\n";
}

void NGLScene::resizeGL( int _w, int _h )
{
  m_cam.setShape( 45.0f, static_cast<float>( _w ) / _h, 0.05f, 350.0f );
  m_win.width  = static_cast<int>( _w * devicePixelRatio() );
  m_win.height = static_cast<int>( _h * devicePixelRatio() );
}

void NGLScene::initializeGL()
{
  // we must call this first before any other GL commands to load and link the
  // gl commands from the lib, if this is not done program will crash
  ngl::NGLInit::instance();

  glClearColor(0.4f, 0.4f, 0.4f, 1.0f);			   // Grey Background
  // enable depth testing for drawing
  glEnable(GL_DEPTH_TEST);
  // enable multisampling for smoother drawing
  glEnable(GL_MULTISAMPLE);
  // Now we will create a basic Camera from the graphics library
  // This is a static camera so it only needs to be set once
  // First create Values for the camera position
  ngl::Vec3 from(0.0f,0.0f,80.0f);
  ngl::Vec3 to(0.0f,0.0f,0.0f);
  ngl::Vec3 up(0.0f,1.0f,0.0f);
  m_cam.set(from,to,up);
  // set the shape using FOV 45 Aspect Ratio based on Width and Height
  // The final two are near and far clipping planes of 0.5 and 10
  m_cam.setShape(45.0f,720.0f/576.0f,0.5f,150.0f);
  // now to load the shader and set the values
  // grab an instance of shader manager
   ngl::ShaderLib *shader=ngl::ShaderLib::instance();
   (*shader)["nglDiffuseShader"]->use();

   shader->setUniform("Colour",1.0f,1.0f,0.0f,1.0f);
   shader->setUniform("lightPos",1.0f,1.0f,1.0f);
   shader->setUniform("lightDiffuse",1.0f,1.0f,1.0f,1.0f);

   (*shader)["nglColourShader"]->use();
   shader->setUniform("Colour",1.0f,1.0f,1.0f,1.0f);

  glEnable(GL_DEPTH_TEST); // for removal of hidden surfaces

  ngl::VAOPrimitives *prim =  ngl::VAOPrimitives::instance();
  prim->createSphere("Boid",1.0f,40.0f);//create the boids as spheres
 // create our Bounding Box, needs to be done once we have a gl context as we create VAO for drawing
  m_bbox.reset( new ngl::BBox(ngl::Vec3(),80.0f,80.0f,80.0f));//set the bbox

  m_bbox->setDrawMode(GL_LINE);

  m_BoidUpdateTimer=startTimer(40);

}


void NGLScene::loadMatricesToShader()
{
  ngl::ShaderLib *shader=ngl::ShaderLib::instance();
  (*shader)["nglDiffuseShader"]->use();

  ngl::Mat4 MV;
  ngl::Mat4 MVP;
  ngl::Mat3 normalMatrix;
  MV= m_cam.getViewMatrix() *m_mouseGlobalTX;
  MVP=m_cam.getProjectionMatrix() *MV;
  normalMatrix=MV;
  normalMatrix.inverse().transpose();
  shader->setUniform("MVP",MVP);
  shader->setUniform("normalMatrix",normalMatrix);
}

void NGLScene::loadMatricesToColourShader()
{
  ngl::ShaderLib *shader=ngl::ShaderLib::instance();
  (*shader)["nglColourShader"]->use();
  ngl::Mat4 MVP;
  MVP=m_cam.getVPMatrix() * m_mouseGlobalTX;
  shader->setUniform("MVP",MVP);

}


void NGLScene::paintGL()
{
  // clear the screen and depth buffer
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glViewport(0,0,m_win.width,m_win.height);
  // Rotation based on the mouse position for our global
   // transform
   ngl::Mat4 rotX;
   ngl::Mat4 rotY;
   // create the rotation matrices
   rotX.rotateX(m_win.spinXFace);
   rotY.rotateY(m_win.spinYFace);
   // multiply the rotations
   m_mouseGlobalTX=rotY*rotX;
   // add the translations
   m_mouseGlobalTX.m_m[3][0] = m_modelPos.m_x;
   m_mouseGlobalTX.m_m[3][1] = m_modelPos.m_y;
   m_mouseGlobalTX.m_m[3][2] = m_modelPos.m_z;

  // grab an instance of the shader manager
  ngl::ShaderLib *shader=ngl::ShaderLib::instance();
  (*shader)["nglColourShader"]->use();
  loadMatricesToColourShader();
  m_bbox->draw();

  shader->use("nglDiffuseShader");

    for(Boid s : m_BoidArray)
	{
		s.draw("nglDiffuseShader",m_mouseGlobalTX,&m_cam);
	}

}

//----------------------------------------------------------------------------------------------------------------------
void NGLScene::updateScene()
{
    for(Boid &s : m_BoidArray)
	{
        ngl::Vec3 target(30.0,0.0,0.0);
        //s.seek(target);
        //s.arrive(target);
        s.separate(m_BoidArray);
        s.align(m_BoidArray);
        s.cohesion(m_BoidArray);


		s.move();

	}
     //avoidWalls();
    //checkCollisions();
}


//----------------------------------------------------------------------------------------------------------------------
void NGLScene::mouseMoveEvent( QMouseEvent* _event )
{
  // note the method buttons() is the button state when event was called
  // that is different from button() which is used to check which button was
  // pressed when the mousePress/Release event is generated
  if ( m_win.rotate && _event->buttons() == Qt::LeftButton )
  {
    int diffx = _event->x() - m_win.origX;
    int diffy = _event->y() - m_win.origY;
    m_win.spinXFace += static_cast<int>( 0.5f * diffy );
    m_win.spinYFace += static_cast<int>( 0.5f * diffx );
    m_win.origX = _event->x();
    m_win.origY = _event->y();
    update();
  }
  // right mouse translate code
  else if ( m_win.translate && _event->buttons() == Qt::RightButton )
  {
    int diffX      = static_cast<int>( _event->x() - m_win.origXPos );
    int diffY      = static_cast<int>( _event->y() - m_win.origYPos );
    m_win.origXPos = _event->x();
    m_win.origYPos = _event->y();
    m_modelPos.m_x += INCREMENT * diffX;
    m_modelPos.m_y -= INCREMENT * diffY;
    update();
  }
}


//----------------------------------------------------------------------------------------------------------------------
void NGLScene::mousePressEvent( QMouseEvent* _event )
{
  // that method is called when the mouse button is pressed in this case we
  // store the value where the maouse was clicked (x,y) and set the Rotate flag to true
  if ( _event->button() == Qt::LeftButton )
  {
    m_win.origX  = _event->x();
    m_win.origY  = _event->y();
    m_win.rotate = true;
  }
  // right mouse translate mode
  else if ( _event->button() == Qt::RightButton )
  {
    m_win.origXPos  = _event->x();
    m_win.origYPos  = _event->y();
    m_win.translate = true;
  }
}

//----------------------------------------------------------------------------------------------------------------------
void NGLScene::mouseReleaseEvent( QMouseEvent* _event )
{
  // that event is called when the mouse button is released
  // we then set Rotate to false
  if ( _event->button() == Qt::LeftButton )
  {
    m_win.rotate = false;
  }
  // right mouse translate mode
  if ( _event->button() == Qt::RightButton )
  {
    m_win.translate = false;
  }
}

//----------------------------------------------------------------------------------------------------------------------
void NGLScene::wheelEvent( QWheelEvent* _event )
{

  // check the diff of the wheel position (0 means no change)
  if ( _event->delta() > 0 )
  {
    m_modelPos.m_z += ZOOM;
  }
  else if ( _event->delta() < 0 )
  {
    m_modelPos.m_z -= ZOOM;
  }
  update();
}
//----------------------------------------------------------------------------------------------------------------------

void NGLScene::keyPressEvent(QKeyEvent *_event)
{
  // this method is called every time the main window recives a key event.
  // we then switch on the key value and set the camera in the GLWindow
  switch (_event->key())
  {
  // escape key to quite
  case Qt::Key_Escape : QGuiApplication::exit(EXIT_SUCCESS); break;
  // show full screen
  case Qt::Key_F : showFullScreen(); break;
  // show windowed
  case Qt::Key_N : showNormal(); break;
  case  Qt::Key_Space : m_animate^=true; break;
  case Qt::Key_S : m_checkBoidBoid^=true; break;
  case Qt::Key_R : setupBoids(); break;
  case Qt::Key_Minus : removeBoid(); break;
  case Qt::Key_Plus : addBoid(); break;

  default : break;
  }
  // finally update the GLWindow and re-draw
  //if (isExposed())
    update();
}

void NGLScene::timerEvent(QTimerEvent *_event )
{
    if(_event->timerId() == m_BoidUpdateTimer)
	{
		if (m_animate !=true)
		{
			return;
		}
	}
	updateScene();
	update();
}




void NGLScene::removeBoid()
{
  std::vector<Boid>::iterator end=m_BoidArray.end();
  if(--m_numBoids==0)
  {
    m_numBoids=1;
  }
  else
  {
    m_BoidArray.erase(end-1,end);
  }
}

void NGLScene::addBoid()
{
  ngl::Random *rng=ngl::Random::instance();
  ngl::Vec3 vel;
  ngl::Vec3 accel;
  vel=rng->getRandomVec3();
  accel=rng->getRandomVec3();
  // add the Boids to the end of the particle list
  m_BoidArray.push_back(Boid(rng->getRandomPoint(s_extents,s_extents,s_extents),vel,accel,rng->randomPositiveNumber(2)+0.5));
  ++m_numBoids;
}




