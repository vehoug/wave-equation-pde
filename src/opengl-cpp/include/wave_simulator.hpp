#ifndef WAVE_SIMULATOR_HPP
#define WAVE_SIMULATOR_HPP

#include <cmath>
#include <vector>
#include <memory>
#include <SFML/Graphics.hpp>

class WaveSimulator
{
public:
  WaveSimulator();
  ~WaveSimulator();

  void initializeSimulation();

private:
  std::unique_ptr<sf::RenderWindow> window_;
  
  std::vector<std::vector<float>> u_;
  std::vector<std::vector<float>> uPrev_;
  std::vector<std::vector<float>> uNext_;

  const int nx_, ny_;
  const int squareSize_;
  const float c_;
  const float dt_;
  const float dx_;
  const float damping_;

  float rotationX_, rotationY_;
  float zoom_;

  void createDisturbance(int x, int y, float amplitude, int radius);
  void updateWave();
  void render3DWave();
};

#endif // WAVE_SIMULATOR_HPP