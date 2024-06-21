/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "rings/resonator.h"
#include "daisysp/dsp/decimator.h"
#include "daisysp/dsp/pitchshifter.h"
#include "daisysp/filters/svf.h"

//==============================================================================
/**
*/
class Weird_filterAudioProcessor  : public juce::AudioProcessor
{
public:
    //==============================================================================
    Weird_filterAudioProcessor();
    ~Weird_filterAudioProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;
    
    using APVTS = juce::AudioProcessorValueTreeState;
    APVTS::ParameterLayout createParameterLayout();
    APVTS apvts {*this, nullptr, "Parameter Layout", createParameterLayout()};
    
//    std::array<rings::Resonator, 2> resonator;
    static constexpr int num_strings_ = 4;
    std::array<std::array<rings::Resonator, 4>, 2> resonator;
    std::array<stmlib::NaiveSvf, 2> filter;
    std::array<daisysp::Decimator, 2> decimator;
    std::array<daisysp::PitchShifter, 2> pitchShifter;
    std::array<daisysp::Svf, 2> lowpass, highpass;
    
    std::vector<float> tempAux;
    
    std::array<float, 32> major_7 =
    {
        32.70, 36.71, 41.20, 49.00,
        65.41, 73.42, 82.41, 98.00,
        130.81, 146.83, 164.81, 196.00,
        261.63, 293.66, 329.63, 392.00,
        523.25, 587.33, 659.26, 783.99,
        1046.50, 1174.66, 1318.51, 1567.98,
        2093.00, 2349.32, 2637.02, 3135.96,
        4186.01, 4698.64, 5274.04, 6271.93
    };

private:
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Weird_filterAudioProcessor)
};
