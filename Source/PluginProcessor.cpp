/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
Weird_filterAudioProcessor::Weird_filterAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       )
#endif
{
    for (int i = 0; i < 2; i++)
    {
        for (int j = 0; j < num_strings_; j++)
        {
            resonator[i][j].Init();
        }
        
//        resonator[i].Init();
        filter[i].Init();
    }
}

Weird_filterAudioProcessor::~Weird_filterAudioProcessor()
{
}

//==============================================================================
const juce::String Weird_filterAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool Weird_filterAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool Weird_filterAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool Weird_filterAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double Weird_filterAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int Weird_filterAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int Weird_filterAudioProcessor::getCurrentProgram()
{
    return 0;
}

void Weird_filterAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String Weird_filterAudioProcessor::getProgramName (int index)
{
    return {};
}

void Weird_filterAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void Weird_filterAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    for (int i = 0; i < 2; i++)
    {
        decimator[i].Init();
        pitchShifter[i].Init (samplesPerBlock);
        lowpass[i].Init (samplesPerBlock);
        highpass[i].Init (samplesPerBlock);
    }
    
    tempAux.resize (samplesPerBlock, 0.f);
}

void Weird_filterAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool Weird_filterAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

void Weird_filterAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());
    
    for (int ch = 0; ch < buffer.getNumChannels(); ch++)
    {
//        resonator[ch].set_position   (0.5);
//        float freq = apvts.getRawParameterValue ("freq")->load() / (float)getSampleRate();
//        resonator[ch].set_frequency  (freq);
//        resonator[ch].set_structure  (0.6f); // apvts.getRawParameterValue ("structure")->load()
//        resonator[ch].set_brightness (0.00001f);
//        resonator[ch].set_resolution (9);
        
        for (int i = 0; i < num_strings_; i++)
        {
            resonator[ch][i].set_position   (0.5);
//            float freq = apvts.getRawParameterValue ("freq")->load() / (float)getSampleRate();
//            resonator[ch][i].set_frequency  (freq); // TODO:
            resonator[ch][i].set_structure  (0.6f); // apvts.getRawParameterValue ("structure")->load()
            resonator[ch][i].set_brightness (0.2f);
            resonator[ch][i].set_resolution (9);
            resonator[ch][i].set_damping (apvts.getRawParameterValue ("ring")->load());
            
            int pitch_step = apvts.getRawParameterValue("pitch-step")->load();
            resonator[ch][i].set_frequency (major_7 [pitch_step + i]);
        }
        
//        float max_freq = 5000.f;
//        float damping_range_factor = 1.f - (freq / max_freq);
//        float dynamic_max_damping = 0.8f * damping_range_factor;
        
//        resonator[ch].set_damping (apvts.getRawParameterValue("ring")->load());
        
        decimator[ch].SetSmoothCrushing (true);
        
        if (apvts.getRawParameterValue("glitch")->load())
        {
            decimator[ch].SetDownsampleFactor (0.62f);
            decimator[ch].SetBitsToCrush (15);
            
            pitchShifter[ch].SetDelSize (16000);
            pitchShifter[ch].SetTransposition (12);
            pitchShifter[ch].SetFun (1.f);
        }
        else
        {
            decimator[ch].SetDownsampleFactor (0.1f);
            decimator[ch].SetBitsToCrush (apvts.getRawParameterValue("crush bits")->load());
            
            pitchShifter[ch].SetDelSize (apvts.getRawParameterValue ("del size")->load());
            pitchShifter[ch].SetTransposition (apvts.getRawParameterValue ("pitch")->load());
            pitchShifter[ch].SetFun (0.f);
        }
        
        lowpass[ch].SetFreq (getSampleRate() / 3); // can be precalculated
        lowpass[ch].SetRes (0.0);
        
        highpass[ch].SetFreq (0.1);
        highpass[ch].SetRes (0.0);
    }
    
    for (int ch = 0; ch < buffer.getNumChannels(); ch++)
    {
        if (!apvts.getRawParameterValue("bypass")->load())
        {
            int route_index = (int)apvts.getRawParameterValue("route")->load();
            
            std::cout << route_index << std::endl;
            
            switch (route_index)
            {
                case 0: // res dist pitch
                    for (int i = 0; i < num_strings_; i++)
                    {
                        resonator[ch][i].Process (buffer.getReadPointer(ch),
                                                  buffer.getWritePointer(ch),
                                                  &tempAux[0],
                                                  buffer.getNumSamples());
                    }
                    
                    for (int s = 0; s < buffer.getNumSamples(); s++)
                    {
                        //decimate
                        buffer.getWritePointer(ch)[s] = decimator[ch].Process (buffer.getWritePointer (ch)[s]);

                        //pitch shift //TODO: figure out how to smoooth del size changes
                        buffer.getWritePointer(ch)[s] = pitchShifter[ch].Process (buffer.getWritePointer (ch)[s]);
                    }
                    
                    break;
                case 1: // dist res pitch
                    
                    for (int s = 0; s < buffer.getNumSamples(); s++)
                    {
                        buffer.getWritePointer(ch)[s] = decimator[ch].Process (buffer.getWritePointer (ch)[s]);
                    }
                    
                    for (int i = 0; i < num_strings_; i++)
                    {
                        resonator[ch][i].Process (buffer.getReadPointer(ch),
                                                  buffer.getWritePointer(ch),
                                                  &tempAux[0],
                                                  buffer.getNumSamples());
                    }
                    
                    for (int s = 0; s < buffer.getNumSamples(); s++)
                    {
                        buffer.getWritePointer(ch)[s] = pitchShifter[ch].Process (buffer.getWritePointer (ch)[s]);
                    }
                    
                    break;
                case 2: // pitch res dist
                    
                    for (int s = 0; s < buffer.getNumSamples(); s++)
                    {
                        buffer.getWritePointer(ch)[s] = pitchShifter[ch].Process (buffer.getWritePointer (ch)[s]);
                    
                    }
                    
                    for (int i = 0; i < num_strings_; i++)
                    {
                        resonator[ch][i].Process (buffer.getReadPointer(ch),
                                                  buffer.getWritePointer(ch),
                                                  &tempAux[0],
                                                  buffer.getNumSamples());
                    }
                    
                    for (int s = 0; s < buffer.getNumSamples(); s++)
                    {
                        buffer.getWritePointer(ch)[s] = decimator[ch].Process (buffer.getWritePointer (ch)[s]);
                    }
                    break;
            }
            
            for (int s = 0; s < buffer.getNumSamples(); s++)
            {
                //filter
                lowpass[ch].Process (buffer.getWritePointer (ch)[s]);
                buffer.getWritePointer(ch)[s] = lowpass[ch].Low();

                highpass[ch].Process (buffer.getWritePointer (ch)[s]);
                buffer.getWritePointer(ch)[s] = highpass[ch].High();
            }
        }
    }
}

//==============================================================================
bool Weird_filterAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* Weird_filterAudioProcessor::createEditor()
{
//    return new Weird_filterAudioProcessorEditor (*this);
    return new juce::GenericAudioProcessorEditor (*this);
}

//==============================================================================
void Weird_filterAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void Weird_filterAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

juce::AudioProcessorValueTreeState::ParameterLayout Weird_filterAudioProcessor::createParameterLayout()
{
    APVTS::ParameterLayout layout;
    
    layout.add (std::make_unique<juce::AudioParameterBool>(juce::ParameterID {"bypass", 1},
                                                           "bypass",
                                                           false));
    
    juce::StringArray routes;
    routes.add ("res dist pitch");
    routes.add ("dist res pitch");
    routes.add ("pitch res dist");
    
    layout.add(std::make_unique<juce::AudioParameterInt>(juce::ParameterID {"pitch-step", 1},
                                                         "pitch-step",
                                                         0, 27,
                                                         0));
    
    layout.add (std::make_unique<juce::AudioParameterChoice>(juce::ParameterID {"route", 1},
                                                             "route",
                                                             routes,
                                                             0));
    
    layout.add(std::make_unique<juce::AudioParameterInt>(juce::ParameterID {"crush bits", 1},
                                                         "crush bits",
                                                         10, 14, 10));
    
    layout.add(std::make_unique<juce::AudioParameterInt>(juce::ParameterID {"pitch", 1},
                                                         "pitch",
                                                         -7, 24, 0));
    
    layout.add(std::make_unique<juce::AudioParameterInt>(juce::ParameterID {"del size", 1},
                                                         "del size",
                                                         40, 400, 40));
    
    layout.add(std::make_unique<juce::AudioParameterBool>(juce::ParameterID {"glitch", 1},
                                                          "glitch",
                                                          false));
    
    // TODO: rename to stereo-ise and add ensemble fx
    juce::NormalisableRange<float> bitCrushParamRanges (0.f, 1.f, .000001f, 1.f);
    
    //---------------------------------------------------------------
    // RESONATOR PARAMS
    //---------------------------------------------------------------
    juce::NormalisableRange<float> freqRange (0.02, 5000, .00001, .1f);
    juce::NormalisableRange<float> paramRanges (0.f, 1.f, .000001f, 1.f);
    layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID {"freq", 1},
                                                           "freq",
                                                           freqRange,
                                                           1000.f));
    
    juce::NormalisableRange<float> ringRange (0.f, .8f, .000001f, 1.f);
    layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID {"ring", 1},
                                                           "ring",
                                                           ringRange,
                                                           0.5f));
    
    return layout;
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new Weird_filterAudioProcessor();
}
