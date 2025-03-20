#include "wave_simulator.hpp"
#include <SFML/OpenGL.hpp>
#include <cmath>

WaveSimulator::WaveSimulator() : nx_(200), ny_(200), squareSize_(3), c_(3.50), dt_(0.05), dx_(1.0), damping_(0.995),
                               rotationX_(30.0f), rotationY_(45.0f), zoom_(-350.0f)
{
  u_     = std::vector<std::vector<float>>(nx_, std::vector<float>(ny_, 0.0f));
  uPrev_ = std::vector<std::vector<float>>(nx_, std::vector<float>(ny_, 0.0f));
  uNext_ = std::vector<std::vector<float>>(nx_, std::vector<float>(ny_, 0.0f));

  sf::ContextSettings settings;
  settings.depthBits = 24;
  settings.stencilBits = 8;
  settings.antialiasingLevel = 4;
  settings.majorVersion = 3;
  settings.minorVersion = 0;

  window_ = std::make_unique<sf::RenderWindow>(
    sf::VideoMode(800, 600), 
    "3D Wave Simulation",
    sf::Style::Default,
    settings
  );
  
  glEnable(GL_DEPTH_TEST);
  glDepthMask(GL_TRUE);
  glClearDepth(1.0f);
  
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  GLfloat ratio = static_cast<float>(800) / 600;
  glFrustum(-ratio, ratio, -1.0, 1.0, 1.0, 500.0);
}

WaveSimulator::~WaveSimulator() {}

void WaveSimulator::initializeSimulation()
{
  createDisturbance(nx_ / 2, ny_ / 2, 3.0, 40);

  sf::Clock clock;
  
  while (window_->isOpen())
  {
    sf::Event event;
    while (window_->pollEvent(event))
    {
      if (event.type == sf::Event::Closed)
        window_->close();
      
      if (event.type == sf::Event::MouseButtonPressed)
      {
        if (event.mouseButton.button == sf::Mouse::Left)
        {
          int mouseX = nx_ / 2 + (event.mouseButton.x - 400) / 4;
          int mouseY = ny_ / 2 + (event.mouseButton.y - 300) / 4;
          
          if (mouseX >= 0 && mouseX < nx_ && mouseY >= 0 && mouseY < ny_)
            createDisturbance(mouseX, mouseY, 1.0, 65);
        }
      }
      
      if (event.type == sf::Event::KeyPressed)
      {
        if (event.key.code == sf::Keyboard::Left)
          rotationY_ -= 5.0f;
        if (event.key.code == sf::Keyboard::Right)
          rotationY_ += 5.0f;
        if (event.key.code == sf::Keyboard::Up)
          rotationX_ -= 5.0f;
        if (event.key.code == sf::Keyboard::Down)
          rotationX_ += 5.0f;
          
        if (event.key.code == sf::Keyboard::Add || event.key.code == sf::Keyboard::Equal)
          zoom_ += 10.0f;
        if (event.key.code == sf::Keyboard::Subtract || event.key.code == sf::Keyboard::Dash)
          zoom_ -= 10.0f;
          
        if (event.key.code == sf::Keyboard::R)
        {
          rotationX_ = 30.0f;
          rotationY_ = 45.0f;
          zoom_ = -350.0f;
        }
      }
    }
    
    updateWave();
    render3DWave();
    
    sf::Time elapsed = clock.restart();
    if (elapsed.asMilliseconds() < 16)
    {
      sf::sleep(sf::milliseconds(16 - elapsed.asMilliseconds()));
    }
  }
}

void WaveSimulator::createDisturbance(int x, int y, float amplitude, int radius)
{
  if (x > 0 && x < nx_ - 1 && y > 0 && y < ny_ - 1)
  {
    for (int i = -radius; i <= radius; i++)
    {
      for (int j = -radius; j <= radius; j++)
      {
        if (x + i >= 1 && x + i < nx_ - 1 && y + j >= 1 && y + j < ny_ - 1)
        {
          float distance = sqrt(i * i + j * j);
          if (distance <= radius) {
            u_[x + i][y + j] = amplitude * cos(distance / 2.0) * exp(-distance / radius);
          }
        }
      }
    }
  }
}

void WaveSimulator::updateWave()
{
  for (int i = 1; i < nx_ - 1; i++)
  {
    for (int j = 1; j < ny_ - 1; j++)
    {
      float laplacian = u_[i + 1][j] + u_[i - 1][j] + u_[i][j + 1] + u_[i][j - 1] - 4 * u_[i][j];
      uNext_[i][j] = 2 * u_[i][j] - uPrev_[i][j] + (c_ * c_ * dt_ * dt_ / (dx_ * dx_)) * laplacian;
      uNext_[i][j] *= damping_;
    }
  }
  uPrev_ = u_;
  u_ = uNext_;
}

void WaveSimulator::render3DWave()
{
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  
  glTranslatef(0.0f, 0.0f, zoom_);
  glRotatef(rotationX_, 1.0f, 0.0f, 0.0f);
  glRotatef(rotationY_, 0.0f, 1.0f, 0.0f);
  
  float gridWidth = nx_ * squareSize_;
  float gridHeight = ny_ * squareSize_;
  glTranslatef(-gridWidth / 2, -gridHeight / 2, 0.0f);
  
  for (int i = 0; i < nx_ - 1; i++)
  {
    glBegin(GL_TRIANGLE_STRIP);
    for (int j = 0; j < ny_; j++)
    {
      float val1 = u_[i][j];
      float val2 = u_[i+1][j];
      
      if (fabs(val1) < 0.05) 
      {
        glColor3f(0.0f, 0.8f, 0.0f);
      } 
      else if (val1 > 0) 
      {
        float intensity = std::min(1.0f, val1);
        if (intensity < 0.33f) 
        {
          float t = intensity / 0.33f;
          glColor3f(t, 0.8f, 0.0f);
        } 
        else if (intensity < 0.66f) 
        {
          float t = (intensity - 0.33f) / 0.33f;
          glColor3f(1.0f, 0.8f * (1.0f - t), 0.0f);
        } 
        else 
        {
          float t = (intensity - 0.66f) / 0.34f;
          glColor3f(1.0f, 0.8f * (1.0f - 0.66f) * (1.0f - t), 0.0f);
        }
      } 
      else 
      {
        float intensity = std::min(1.0f, -val1);
        if (intensity < 0.5f) 
        {
          float t = intensity / 0.5f;
          glColor3f(0.0f, 0.8f * (1.0f - t), 0.5f + 0.5f * t);
        } 
        else 
        {
          float t = (intensity - 0.5f) / 0.5f;
          glColor3f(0.0f, 0.8f * 0.5f * (1.0f - t), 0.5f + 0.5f * t);
        }
      }
      
      glVertex3f(i * squareSize_, j * squareSize_, val1 * 20.0f);
      
      if (fabs(val2) < 0.05) 
      {
        glColor3f(0.0f, 0.8f, 0.0f); // Green
      } 
      else if (val2 > 0) 
      {
        float intensity = std::min(1.0f, val2);
        if (intensity < 0.33f) 
        {
          float t = intensity / 0.33f;
          glColor3f(t, 0.8f, 0.0f); // Green to yellow (0-33%)
        } 
        else if (intensity < 0.66f) 
        {
          float t = (intensity - 0.33f) / 0.33f;
          glColor3f(1.0f, 0.8f * (1.0f - t), 0.0f); // Yellow to orange (33-66%)
        } 
        else 
        {
          float t = (intensity - 0.66f) / 0.34f;
          glColor3f(1.0f, 0.8f * (1.0f - 0.66f) * (1.0f - t), 0.0f); // Orange to red (66-100%)
        }
      } 
      else 
      {
        float intensity = std::min(1.0f, -val2);
        if (intensity < 0.5f) 
        {
          float t = intensity / 0.5f;
          glColor3f(0.0f, 0.8f * (1.0f - t), 0.5f + 0.5f * t); // Green to light blue (0-50%)
        } 
        else {
          float t = (intensity - 0.5f) / 0.5f;
          glColor3f(0.0f, 0.8f * 0.5f * (1.0f - t), 0.5f + 0.5f * t); // Light blue to deep blue (50-100%)
        }
      }
      
      glVertex3f((i + 1) * squareSize_, j * squareSize_, val2 * 20.0f);
    }
    glEnd();
  }
  
  glColor3f(0.0f, 0.0f, 0.0f);
  glLineWidth(1.0f);
  
  for (int i = 0; i <= nx_; i += 5) 
  {
    glBegin(GL_LINE_STRIP);
    for (int j = 0; j < ny_; j++) 
    {
      float height = (i < nx_) ? u_[i][j] * 20.0f : u_[nx_-1][j] * 20.0f;
      glVertex3f(i * squareSize_, j * squareSize_, height);
    }
    glEnd();
  }
  
  for (int j = 0; j <= ny_; j += 5) 
  {
    glBegin(GL_LINE_STRIP);
    for (int i = 0; i < nx_; i++) 
    {
      glVertex3f(i * squareSize_, j * squareSize_, u_[i][std::min(j, ny_-1)] * 20.0f);
    }
    glEnd();
  }
  
  window_->display();
}