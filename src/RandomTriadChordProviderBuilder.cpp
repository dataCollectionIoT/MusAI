#include "../include/RandomTriadChordPoolBuilder.h"

std::shared_ptr<ChordPool> RandomTriadChordPoolBuilder::build()
{
  std::shared_ptr<ChordPool> cp = std::make_shared<ChordPool>();
  for (int i = -7; i < 14; ++i)
  {
    std::vector<int> cd;
    cd.push_back(i);
    cd.push_back(i + 2);
    cd.push_back(i + 4);
    cp->addChord(std::make_shared<Chord>(cd));
  }
  return cp;
}

void RandomTriadChordPoolBuilder::registerWith(Describer& describer)
{
  describer.registerBuilder("ChordPoolBuilder", "RandomTriadChordPoolBuilder", "Random Triads",{});
}
