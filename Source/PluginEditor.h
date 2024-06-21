/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

enum SliderStyle {
    SPIRAL,
    CIRCLE,
    TRIANGLE,
    INDICATOR,
    DOT
};

enum MaxStyle {
    MAX_STYLE_ONE,
    MAX_STYLE_TWO,
    MAX_STYLE_THREE,
    MAX_STYLE_FOUR,
    MAX_STYLE_FIVE,
    MAX_STYLE_NONE
};

template<int numSpins, MaxStyle maxStyle>
struct SpiralSliderLookAndFeel : juce::LookAndFeel_V4
{
    void drawRotarySlider (juce::Graphics& g,
                           int x, int y, int width, int height,
                           float sliderPosProportional,
                           float rotaryStartAngle,
                           float rotaryEndAngle,
                           juce::Slider& slider) override;
};

enum CircleStyle {
    ONE,
    TWO,
    NONE
};
template<CircleStyle style, MaxStyle maxStyle>
struct CircleSliderLookAndFeel : juce::LookAndFeel_V4
{
    void drawRotarySlider (juce::Graphics& g,
                           int x, int y, int width, int height,
                           float sliderPosProportional,
                           float rotaryStartAngle,
                           float rotaryEndAngle,
                           juce::Slider& slider) override;
};

template<MaxStyle maxStyle>
struct TriangleSliderLookAndFeel : juce::LookAndFeel_V4
{
    void drawRotarySlider (juce::Graphics& g,
                           int x, int y, int width, int height,
                           float sliderPosProportional,
                           float rotaryStartAngle,
                           float rotaryEndAngle,
                           juce::Slider& slider) override;
};

struct IndicatorSliderLookAndFeel : juce::LookAndFeel_V4
{
    void drawRotarySlider (juce::Graphics& g,
                           int x, int y, int width, int height,
                           float sliderPosProportional,
                           float rotaryStartAngle,
                           float rotaryEndAngle,
                           juce::Slider& slider) override;
};

struct DotSliderLookAndFeel : juce::LookAndFeel_V4
{
    void drawRotarySlider (juce::Graphics& g,
                           int x, int y, int width, int height,
                           float sliderPosProportional,
                           float rotaryStartAngle,
                           float rotaryEndAngle,
                           juce::Slider& slider) override;
};

template<SliderStyle style, CircleStyle circleStyle, int numSpins, MaxStyle maxStyle>
struct CustomRotarySlider : juce::Slider
{
    juce::Label label;  // Label as part of the slider
    juce::LookAndFeel_V4* lnf;
    
    CustomRotarySlider (juce::String labelText)
    : juce::Slider(juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag, juce::Slider::TextEntryBoxPosition::NoTextBox)
    {
        // Setup the label
        label.setFont (juce::Font (15.0f, juce::Font::plain));
        label.setJustificationType(juce::Justification::centredBottom);
        label.setColour(juce::Label::textColourId, juce::Colours::white);
        label.setEditable(false, false, false);
        label.setInterceptsMouseClicks(false, false);  // Make label non-interactive
        addAndMakeVisible(label);
        
        label.setText (labelText, juce::dontSendNotification);
        
        switch (style)
        {
            case SPIRAL:
                lnf = new SpiralSliderLookAndFeel<numSpins, maxStyle>;
                break;
            case CIRCLE:
                lnf = new CircleSliderLookAndFeel<circleStyle, maxStyle>;
                break;
            case TRIANGLE:
                lnf = new TriangleSliderLookAndFeel<maxStyle>;
                break;
            case INDICATOR:
                lnf = new IndicatorSliderLookAndFeel;
                break;
            case DOT:
                lnf = new DotSliderLookAndFeel;
                break;
        };
        
        setLookAndFeel (lnf);
    }
    
    ~CustomRotarySlider()
    {
        setLookAndFeel (nullptr);
        delete lnf;
    }
    
    void paint (juce::Graphics& g) override;

    void resized() override
    {
        // Position the label below the slider
        auto labelHeight = 20;
        label.setBounds(0, getLocalBounds().getBottom() - labelHeight, getLocalBounds().getWidth(), labelHeight);
    }
};

struct ButtonLookAndFeel : juce::LookAndFeel_V4
{
    void drawToggleButton (juce::Graphics &g,
                           juce::ToggleButton & toggleButton,
                           bool shouldDrawButtonAsHighlighted,
                           bool shouldDrawButtonAsDown) override;
};

struct CustomButton : juce::ToggleButton
{
    juce::Label label;  // Label as part of the slider
    ButtonLookAndFeel lnf;
    
    std::function<void()> onToggledCallback;
    
    CustomButton()
    {
        label.setFont (juce::Font (15.0f, juce::Font::plain));
        label.setJustificationType (juce::Justification::centredBottom);
        label.setColour (juce::Label::textColourId, juce::Colours::white);
        label.setEditable (false, false, false);
        label.setInterceptsMouseClicks (false, false);  // Make label non-interactive
        addAndMakeVisible(label);
        
        label.setText ("glitch", juce::dontSendNotification);
        
        setLookAndFeel (&lnf);
        
        onClick = [&]()
        {
            onToggledCallback();
        };
    }
    
    ~CustomButton()
    {
        setLookAndFeel (nullptr);
    }
    
    void paint (juce::Graphics& g) override;
    
    void resized() override
    {
        auto labelHeight = 20;
        label.setBounds (0, getLocalBounds().getBottom() - labelHeight, getLocalBounds().getWidth(), labelHeight);
    }
};

struct RoutesSwitch : public juce::ComboBox
{
    struct CustomLookAndFeel : public juce::LookAndFeel_V4
    {   
        juce::Font getComboBoxFont (juce::ComboBox& box) override
        {
            return juce::Font (0.0f);
        }
    };
    
    CustomLookAndFeel lnf;
    
    RoutesSwitch()
    {
        addItem ("res dist pitch", 1);
        addItem ("dist res pitch", 2);
        addItem ("pitch res dist", 3);
        
        setSelectedId (1);
        
        setLookAndFeel (&lnf);

        setJustificationType (juce::Justification::centred);
    }
    
    ~RoutesSwitch()
    {
        setLookAndFeel (nullptr);
    }

    void paint (juce::Graphics& g) override
    {
        auto bounds = getLocalBounds().toFloat();
        auto third_width = bounds.getWidth() / 3.0f;
        
        for (int i = 0; i < 3; i++)
        {
            auto buttonBounds = juce::Rectangle<float>(bounds.getX() + i * third_width,
                                                       bounds.getY() + 6,
                                                       third_width,
                                                       bounds.getHeight() - 6);

            auto circleX = buttonBounds.getCentreX();
            auto circleY = buttonBounds.getCentreY();
            auto circleSize = std::min (buttonBounds.getWidth(), buttonBounds.getHeight());

            g.setColour (getSelectedId() - 1 == i ? juce::Colours::white : juce::Colours::white.withAlpha(0.3f));
            g.fillEllipse (circleX - (circleSize / 2), circleY - (circleSize / 2), circleSize, circleSize);
        }
        
        g.setColour (juce::Colours::white.withAlpha (0.5f));
        g.drawRect(bounds.getX(), bounds.getY(), bounds.getWidth(), 1.f);
    }

    void mouseDown (const juce::MouseEvent& e) override
    {
        auto bounds = getLocalBounds().toFloat();
        auto third_width = bounds.getWidth() / 3.0f;
        for (int i = 0; i < 3; i++)
        {
            auto buttonBounds = juce::Rectangle<float>(bounds.getX() + i * third_width,
                                                       bounds.getY(),
                                                       third_width,
                                                       bounds.getHeight());

            if (buttonBounds.contains (e.getPosition().toFloat()))
            {
                setSelectedId (i + 1, juce::sendNotificationAsync);
            }
        }
    }
};

//==============================================================================
/**
*/
class Weird_filterAudioProcessorEditor  : public juce::AudioProcessorEditor
{
public:
    Weird_filterAudioProcessorEditor (Weird_filterAudioProcessor&);
    ~Weird_filterAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void paintOverChildren (juce::Graphics& g) override;
    void resized() override;

private:
    
    Weird_filterAudioProcessor& audioProcessor;
    
    std::vector<juce::Component*> comps;
    std::vector<juce::Component*> getComps();
    
    CustomButton glitch_button;
    
    CustomRotarySlider<TRIANGLE, CircleStyle::NONE, 0, MAX_STYLE_FIVE> crush_bits_slider;
    CustomRotarySlider<CIRCLE, CircleStyle::ONE, 0, MAX_STYLE_THREE> pitch_slider;
    CustomRotarySlider<CIRCLE, CircleStyle::TWO, 0, MAX_STYLE_FOUR> delay_size_slider;
    CustomRotarySlider<SPIRAL, CircleStyle::NONE, 4, MAX_STYLE_ONE> freq_slider;
    CustomRotarySlider<SPIRAL, CircleStyle::NONE, 2, MAX_STYLE_TWO> structure_slider;
    
    using APVTS = juce::AudioProcessorValueTreeState;
    using SliderAttachment = APVTS::SliderAttachment;
    using ButtonAttachment = APVTS::ButtonAttachment;
    using ChoiceAttachment = APVTS::ComboBoxAttachment;
    
    ButtonAttachment glitch_button_attachment;
    
    SliderAttachment
        crush_bits_slider_attachment,
        pitch_slider_attachment,
        delay_size_slider_attachment,
        freq_slider_attachment,
        structure_slider_attachment;
    
    RoutesSwitch route_switch;
    
    ChoiceAttachment route_switch_attachment;
    
    bool glitch_time;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Weird_filterAudioProcessorEditor)
};
