#include "NGLScene.h"
#include <QGuiApplication>
#include <QMouseEvent>

#include <ngl/NGLInit.h>
#include <ngl/NGLStream.h>
#include <ngl/Random.h>
#include <ngl/ShaderLib.h>
#include <ngl/VAOPrimitives.h>

NGLScene::NGLScene()
{
  setTitle( "FirstPerson Camera " );
  m_timer.start();
}


NGLScene::~NGLScene()
{
  std::cout << "Shutting down NGL, removing VAO's and Shaders\n";
}



void NGLScene::resizeGL( int _w, int _h )
{
  m_cam.setProjection( 45.0f, static_cast<float>( _w ) / _h, 0.05f, 350.0f );
  m_win.width  = static_cast<int>( _w * devicePixelRatio() );
  m_win.height = static_cast<int>( _h * devicePixelRatio() );
}


void NGLScene::initializeGL()
{
  // we must call that first before any other GL commands to load and link the
  // gl commands from the lib, if that is not done program will crash
  ngl::NGLInit::initialize();


  glClearColor( 0.4f, 0.4f, 0.4f, 1.0f ); // Grey Background
  // enable depth testing for drawing
  glEnable( GL_DEPTH_TEST );
  // enable multisampling for smoother drawing
  #ifndef USINGIOS_
  glEnable( GL_MULTISAMPLE );
  #endif
  // Now we will create a basic Camera from the graphics library
  // This is a static camera so it only needs to be set once
  // First create Values for the camera position
  ngl::Vec3 from( 0, 2, 13 );
  ngl::Vec3 to( 0, 0, 0 );
  ngl::Vec3 up( 0, 1, 0 );
  // now load to our new camera
  m_cam.set( from, to, up );
  // set the shape using FOV 45 Aspect Ratio based on Width and Height
  // The final two are near and far clipping planes of 0.5 and 10
  m_cam.setProjection( 45.0f, 720.0f / 576.0f, 0.05f, 350.0f );

  ngl::ShaderLib::use(ngl::nglColourShader);
  ngl::ShaderLib::setUniform("Colour",1.0f,1.0f,1.0f,1.0f);
  ngl::VAOPrimitives::createTrianglePlane("floor",650,650,1,1,ngl::Vec3::up());
  


}


void NGLScene::loadMatricesToShader(ngl::Vec4 &_colour)
{
  ngl::Mat4 MVP= m_cam.getVP() * m_transform.getMatrix();

  ngl::ShaderLib::setUniform( "MVP", MVP );
  ngl::ShaderLib::setUniform("Colour",_colour);
}

void NGLScene::paintGL()
{
  float currentFrame = m_timer.elapsed()*0.001f;
  std::cout<<"Current Frame "<<currentFrame<<'\n';
  m_deltaTime = currentFrame - m_lastFrame;
  m_lastFrame = currentFrame;

  glViewport( 0, 0, m_win.width, m_win.height );
  // clear the screen and depth buffer
  glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

  ngl::ShaderLib::use(ngl::nglColourShader);

  /// first we reset the movement values
  float xDirection=0.0;
  float yDirection=0.0;
  // now we loop for each of the pressed keys in the the set
  // and see which ones have been pressed. If they have been pressed
  // we set the movement value to be an incremental value
  foreach(Qt::Key key, m_keysPressed)
  {
    switch (key)
    {
      case Qt::Key_Left :  { yDirection=-1.0f; break;}
      case Qt::Key_Right : { yDirection=1.0f; break;}
      case Qt::Key_Up :		 { xDirection=1.0f; break;}
      case Qt::Key_Down :  { xDirection=-1.0f; break;}
      default : break;
    }
  }
  // if the set is non zero size we can update the ship movement
  // then tell openGL to re-draw
  if(m_keysPressed.size() !=0)
  {
    m_cam.move(xDirection,yDirection,m_deltaTime);
  }

  int nrRows    = 20;
  int nrColumns = 20;
  float spacing = 8.0;
  ngl::Random::setSeed(1234);
  glPolygonMode(GL_FRONT_AND_BACK,GL_LINE);
  for (int row = 0; row < nrRows; ++row)
  {
      for (int col = 0; col < nrColumns; ++col)
      {
        m_transform.setPosition(static_cast<float>(col - (nrColumns / 2)) * spacing,
                                0.0f,
                                static_cast<float>(row - (nrRows / 2)) * spacing);
        m_transform.setRotation(0.0f,ngl::Random::randomPositiveNumber()*360.0f,0.0f);

        loadMatricesToShader(ngl::Vec4(1.0f,0.0f,0.0f,1.0f));
        ngl::VAOPrimitives::draw("teapot");
      }
  }
  // draw floor

  glPolygonMode(GL_FRONT_AND_BACK,GL_FILL);
  m_transform.reset();
  m_transform.setPosition(0.0f,-0.5f,0.0f);
  loadMatricesToShader(ngl::Vec4(0.2f,0.2f,0.2f,1.0f));
  ngl::VAOPrimitives::draw("floor");

}

//----------------------------------------------------------------------------------------------------------------------

void NGLScene::keyPressEvent( QKeyEvent* _event )
{
  // add to our keypress set the values of any keys pressed
  m_keysPressed += static_cast<Qt::Key>(_event->key());
  // that method is called every time the main window recives a key event.
  // we then switch on the key value and set the camera in the GLWindow
  auto setLight=[](std::string _num,bool _mode)
  {
    ngl::ShaderLib::use("PBR");
    if(_mode == true)
    {
      ngl::Vec3 colour={255.0f,255.0f,255.0f};
      ngl::ShaderLib::setUniform(_num,colour);
    }
    else
    {
      ngl::Vec3 colour={0.0f,0.0f,0.0f};
      ngl::ShaderLib::setUniform(_num,colour);

    }

  };
  switch ( _event->key() )
  {
    // escape key to quit
    case Qt::Key_Escape:
      QGuiApplication::exit( EXIT_SUCCESS );
      break;
  
// turn on wireframe rendering
#ifndef USINGIOS_
    case Qt::Key_W:
      glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
      break;
    // turn off wire frame
    case Qt::Key_S:
      glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
      break;
#endif
    // show full screen
    case Qt::Key_F:
      showFullScreen();
      break;
    // show windowed
    case Qt::Key_N:
      showNormal();
      break;
    case Qt::Key_Space :
      m_win.spinXFace=0;
      m_win.spinYFace=0;
      m_modelPos.set(ngl::Vec3::zero());
    break;
    default:
      break;
  }
  update();
}

void NGLScene::keyReleaseEvent( QKeyEvent *_event	)
{
  // remove from our key set any keys that have been released
  m_keysPressed -= static_cast<Qt::Key>(_event->key());
}

