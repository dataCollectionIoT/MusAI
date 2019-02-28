#include "../include/Timeline.h"

#include "../include/Logger.h"
#define LOC "timeline"

Timeline::Timeline(int numBeats) : mNumBeats(numBeats)
{
  addTrackNow("default");
}

void Timeline::addTrackNow(std::string track)
{
  mTempos.insert(std::pair<std::string, TimelineTrack<Tempo>>(track, TimelineTrack<Tempo>()));
  mTonics.insert(std::pair<std::string, TimelineTrack<Tonic>>(track, TimelineTrack<Tonic>()));
  mScales.insert(std::pair<std::string, TimelineTrack<Scale>>(track, TimelineTrack<Scale>()));
  mChords.insert(std::pair<std::string, TimelineTrack<Chord>>(track, TimelineTrack<Chord>()));
  mPitchSequences.insert(std::pair<std::string, TimelineTrack<PitchSequence>>(track, TimelineTrack<PitchSequence>()));
  mRhythms.insert(std::pair<std::string, TimelineTrack<Rhythm>>(track, TimelineTrack<Rhythm>()));
}

void Timeline::addTrack(std::string track)
{
  if (mTracks.find(track) == mTracks.end() && track != "default")
  {
    mTracks.insert(track);
    addTrackNow(track);
  }
}

void Timeline::add(std::shared_ptr<Tempo> tempo, std::string track, int begin, int end)
{  
  mTempos.at(track).add(TimelineBucket<Tempo>(tempo, begin, end));
}

void Timeline::add(std::shared_ptr<Tonic> tonic, std::string track, int begin, int end)
{
  mTonics.at(track).add(TimelineBucket<Tonic>(tonic, begin, end));
}

void Timeline::add(std::shared_ptr<Scale> scale, std::string track, int begin, int end)
{
  mScales.at(track).add(TimelineBucket<Scale>(scale, begin, end));
}

void Timeline::add(std::shared_ptr<Chord> chord, std::string track, int begin, int end)
{
  mChords.at(track).add(TimelineBucket<Chord>(chord, begin, end));
}

void Timeline::add(std::shared_ptr<PitchSequence> pitches, std::string track, int begin, int end)
{
  mPitchSequences.at(track).add(TimelineBucket<PitchSequence>(pitches, begin, end));
}

void Timeline::add(std::shared_ptr<Rhythm> rhythm, std::string track, int begin, int end)
{
  mRhythms.at(track).add(TimelineBucket<Rhythm>(rhythm, begin, end));
}

std::set<std::string> Timeline::getTracks() const
{
  return mTracks;
}

double Timeline::getLength(std::string track) const
{
  std::shared_ptr<Tempo> tempo = mTempos.at(track).getObject();
  if (NULL == tempo)
    tempo = mTempos.at("default").getObject();

  return tempo->applyTempo(mNumBeats);
}

std::vector<Note> Timeline::getNotes(std::string track)
{
  Logger& logger = Logger::instance(); 
  int currentBeat = 0;
  int currentPitch = 0;
  //PitchCollection* oldPitchCollection = NULL;
  std::vector<Note> notes; 

  std::shared_ptr<Rhythm> rhythm = mRhythms.at(track).getObject(currentBeat);
  if (NULL == rhythm)
    rhythm = mRhythms.at("default").getObject(currentBeat);

  for (RhythmicNote rhythmicNote : rhythm->getNotes())
  {
    std::shared_ptr<PitchSequence> pitches = mPitchSequences.at(track).getObject(currentBeat + rhythmicNote.mStartBeat);
    if (NULL == pitches)
      pitches = mPitchSequences.at("default").getObject(currentBeat + rhythmicNote.mStartBeat);

    /*if (&pitchCollection != oldPitchCollection) compare starting beats of pitch collection stored in Timeline
    {
      oldPitchCollection = &pitchCollection;
      currentPitch = 0;
    }*/
    std::shared_ptr<Scale> scale = mScales.at(track).getObject(currentBeat + rhythmicNote.mStartBeat);
    if (NULL == scale)
      scale = mScales.at("default").getObject(currentBeat + rhythmicNote.mStartBeat);

    std::shared_ptr<Chord> chord = mChords.at(track).getObject(currentBeat + rhythmicNote.mStartBeat);
    if (NULL == chord)
      chord = mChords.at("default").getObject(currentBeat + rhythmicNote.mStartBeat);

    std::shared_ptr<Tempo> tempo = mTempos.at(track).getObject(currentBeat + rhythmicNote.mStartBeat);
    if (NULL == tempo)
      tempo = mTempos.at("default").getObject(currentBeat + rhythmicNote.mStartBeat);

    std::shared_ptr<Tonic> tonic = mTonics.at(track).getObject(currentBeat + rhythmicNote.mStartBeat);
    if (NULL == tonic)
      tonic = mTonics.at("default").getObject(currentBeat + rhythmicNote.mStartBeat);

    double start = tempo->applyTempo(
        double(currentBeat)
          + double(rhythmicNote.mStartBeat)
          + (double(rhythmicNote.mStartValue) / double(rhythmicNote.mStartSubdivision))
    );

    double length = tempo->applyTempo(double(rhythmicNote.mValue) / double(rhythmicNote.mSubdivision));

    for (int pitch : pitches->getUnison(currentPitch).resolve(*scale, *chord))
    {
      Note note(tonic->getCenterPitch() + pitch, start, length);    

      logger.log(LOC, "adding note on track " + track
          + " " + std::to_string(note.mKey)
          + " " + std::to_string(note.mTime)
          + " " + std::to_string(note.mDuration)
          + " " + std::to_string(note.mVelocity)
      );
      notes.push_back(note);
    }

    ++currentPitch;
  }

  return notes;
}
