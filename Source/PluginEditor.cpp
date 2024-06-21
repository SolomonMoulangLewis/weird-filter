/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//====================================================================
// SPIRAL
//====================================================================
void drawMaxLines (juce::Graphics& g,
                   MaxStyle maxStyle,
                   juce::Rectangle<float>& bounds,
                   juce::Point<float> center,
                   float& sliderPosProportional,
                   float maxRadius,
                   float& diameter)
{
    using namespace juce;
    
    if (maxStyle == MAX_STYLE_ONE)
    {
        if (sliderPosProportional == 1.0f)
        {
            static bool linesGenerated = false;
            static Array<Line<float>> lines;
            if (!linesGenerated) {
                float lineAreaRadius = maxRadius * 1.5;  // Extend the diagonal beyond the slider area
                int numberOfLines = 10;  // Set the number of diagonal lines
                float spacing = (lineAreaRadius * 2) / numberOfLines;  // Spacing between the start points of the lines

                for (int i = 0; i < numberOfLines; i++) {
                    Point<float> start(center.x - maxRadius + i * spacing,
                                       center.y - maxRadius + i * spacing);
                    Point<float> end(center.x + maxRadius - i * spacing,
                                     center.y + maxRadius - i * spacing);

                    lines.add(Line<float>(start, end));
                }
                linesGenerated = true;
            }
            for (auto& line : lines) {
                g.setColour(Colours::red);
                g.drawLine(line, 2.0f);
            }
        }
    }
    else if (maxStyle == MAX_STYLE_TWO)
    {
        if (sliderPosProportional == 1.0f) {
            static bool linesGenerated = false;
            static Array<Line<float>> lines;
            if (!linesGenerated) {
                int numberOfLines = 10;  // Set the number of diagonal lines
                float lineSpacing = diameter / (numberOfLines - 1);  // Calculate the spacing based on diameter and number of lines

                for (int i = 0; i < numberOfLines; i++) {
                    float offset = lineSpacing * i;  // Calculate the offset for each line from the edge of the bounds
                    
                    // Calculate start and end points to make lines parallel and diagonal
                    Point<float> start(bounds.getX() + offset, bounds.getY());
                    Point<float> end(bounds.getRight(), bounds.getY() + offset);
                    
                    if (end.y > bounds.getBottom()) {
                        // Adjust the end point to stay within the bounds if it exceeds the bottom edge
                        float excess = end.y - bounds.getBottom();
                        end.y -= excess;
                        end.x -= excess;
                    }

                    lines.add(Line<float>(start, end));
                }
                linesGenerated = true;
            }
            for (auto& line : lines)
            {
                g.setColour(Colours::red);
                g.drawLine(line, 2.0f);
            }
        }
    }
    else if (maxStyle == MAX_STYLE_THREE)
    {
        if (sliderPosProportional == 1.0f) {
            static bool linesGenerated = false;
            static Array<Line<float>> lines;
            if (!linesGenerated) {
                int numberOfLines = 10;  // Set the number of diagonal lines
                float lineSpacing = bounds.getHeight() / (numberOfLines - 2);  // Calculate the spacing based on the height

                for (int i = 0; i < numberOfLines; i++) {
                    float yOffset = lineSpacing * i;  // Calculate the vertical offset for each line
                    Point<float> start(bounds.getX(), bounds.getY() + yOffset);  // Start point at the left, moving down
                    Point<float> end(bounds.getX() + bounds.getWidth(), bounds.getY() + yOffset - bounds.getHeight());  // End point at the right, moving up

                    lines.add(Line<float>(start, end));
                }
                linesGenerated = true;
            }
            for (auto& line : lines) {
                g.setColour(Colours::red);
                g.drawLine(line, 2.0f);
            }
        }
    }
    else if (maxStyle == MAX_STYLE_FOUR)
    {
        if (sliderPosProportional == 1.0f) {
            static bool linesGenerated = false;
            static Array<Line<float>> lines;
            if (!linesGenerated) {
                Random random(1000);
                int numberOfLines = 4;  // Set the number of random lines

                for (int i = 0; i < numberOfLines; i++) {
                    float startX = bounds.getX() + random.nextFloat() * bounds.getWidth();  // Random start X within the bounds
                    float startY = bounds.getY() + random.nextFloat() * bounds.getHeight(); // Random start Y within the bounds
                    float endX = bounds.getX() + random.nextFloat() * bounds.getWidth();    // Random end X within the bounds
                    float endY = bounds.getY() + random.nextFloat() * bounds.getHeight();   // Random end Y within the bounds

                    lines.add(Line<float>(startX, startY, endX, endY));
                }
                linesGenerated = true;
            }
            for (auto& line : lines) {
                g.setColour(Colours::red);
                g.drawLine(line, 3.0f);
            }
        }
    }
    else if (maxStyle == MAX_STYLE_FIVE)
    {
        if (sliderPosProportional == 1.0f) {
            static bool linesGenerated = false;
            static Array<Line<float>> lines;
            if (!linesGenerated) {
                int numberOfLines = 7;  // Set the number of lines emanating from the side
                float angleIncrement = MathConstants<float>::twoPi / numberOfLines;  // Calculate the angle increment

                for (int i = 0; i < numberOfLines; i++) {
                    float angle = angleIncrement * i;  // Calculate the angle for each line
                    float lineLength = maxRadius * 0.8;  // Set the length of each line

                    // Apply a jagged offset by modifying the length randomly
                    float jaggedOffset = Random::getSystemRandom().nextFloat() * 5.f;  // Random offset for jaggedness
                    float endX = center.x + (lineLength + jaggedOffset) * std::cos(angle);
                    float endY = center.y + (lineLength + jaggedOffset) * std::sin(angle);

                    lines.add(Line<float>(bounds.getX() + bounds.getWidth(), bounds.getY(), endX, endY));
                }
                linesGenerated = true;
            }
            for (auto& line : lines) {
                g.setColour(Colours::red);
                g.drawLine(line, 2.0f);
            }
        }
    }
}

template<int numSpins, MaxStyle maxStyle>
void SpiralSliderLookAndFeel<numSpins, maxStyle>::drawRotarySlider (juce::Graphics &g,
                                                                    int x, int y, int width, int height,
                                                                    float sliderPosProportional,
                                                                    float rotaryStartAngle,
                                                                    float rotaryEndAngle,
                                                                    juce::Slider &slider)
{
    using namespace juce;

    float innerMargin = 20.f;
    float diameter = std::min(width, height) - innerMargin;
    auto bounds = Rectangle<float>(x + (width - diameter) / 2,
                                   y + (height - diameter) / 2,
                                   diameter,
                                   diameter);
    auto center = bounds.getCentre();
    
    float maxRadius = (diameter / 2) - 8;
    int turns = numSpins;  // Higher number of turns for a denser spiral
    float totalAngle = turns * 2 * MathConstants<float>::pi;
    
    // TODO: MAX Red Lines
    drawMaxLines (g, maxStyle, bounds, center.toFloat(), sliderPosProportional, maxRadius, diameter);

    Path spiralPath;
    float angleStep = 0.1f; // Smaller angle steps for a smoother spiral
    float lastRadius = 0.0f;
    float maxDisplacement = 20.0f;  // Maximum displacement magnitude

    // Create the spiral path with controlled displacement
    for (float angle = 0; angle < totalAngle * sliderPosProportional; angle += angleStep)
    {
        float progress = angle / totalAngle;
        float radius = maxRadius * progress;
        float displacement = std::sin(angle * 5) * maxDisplacement * progress;  // Increasing displacement with progress

        float actualRadius = radius + displacement;
        float x = center.x + actualRadius * std::cos(angle + rotaryStartAngle * numSpins);
        float y = center.y + actualRadius * std::sin(angle + rotaryStartAngle * numSpins);

        if (angle == 0)
            spiralPath.startNewSubPath(x, y);
        else
            spiralPath.lineTo(x, y);

        lastRadius = radius;
    }
    
    float rotationAngle = sliderPosProportional * 0.5 * -(MathConstants<float>::pi);

    // Rotate the entire spiral path
    spiralPath.applyTransform (AffineTransform::rotation(rotationAngle, center.x, center.y));

    // Draw the spiral
    g.setColour(Colours::white);
    g.strokePath(spiralPath, PathStrokeType(1.f));
}


//====================================================================
// CIRCLE
//====================================================================
template<CircleStyle style, MaxStyle maxStyle>
void CircleSliderLookAndFeel<style, maxStyle>::drawRotarySlider (juce::Graphics &g,
                                                       int x, int y, int width, int height,
                                                       float sliderPosProportional,
                                                       float rotaryStartAngle,
                                                       float rotaryEndAngle,
                                                       juce::Slider &slider)
{
    using namespace juce;
    float innerMargin = 30.f;
    float diameter = std::min(width, height) - innerMargin;
    auto bounds = Rectangle<float>(x + (width - diameter) / 2,
                                   y + (height - diameter) / 2,
                                   diameter,
                                   diameter);
    auto center = bounds.getCentre();
    
    drawMaxLines (g,
                  maxStyle,
                  bounds,
                  bounds.getCentre(),
                  sliderPosProportional,
                  (diameter / 2),
                  diameter);
    
    g.setColour (Colours::white);
    
    if (style == CircleStyle::ONE)
    {
        // Extreme displacement settings
        float maxDisplacement = 20.0f;  // Max displacement in pixels for the circle's position

        // Draw displaced and distorted circles
        int numberOfCircles = static_cast<int>(sliderPosProportional * 10);  // Max 10 circles for full slider value

        for (int i = 0; i < numberOfCircles; i++)
        {
            float progress = (i + 1) / (float)numberOfCircles;
            float circleDiameter = diameter * progress;
            float displacement = maxDisplacement * (1.0 - progress) * std::sin(float(i + 1) * sliderPosProportional * MathConstants<float>::pi);
            float lineThickness = 1.f;

            // Random displacement to add more dynamic effect
            Point<float> displacedCenter(center.x + displacement, center.y + displacement * std::cos(float(i + 1) * 0.5f));

            // Calculate the bounds for each circle with added random effect
            Rectangle<float> circleBounds = Rectangle<float>(displacedCenter.x - (circleDiameter / 2),
                                                             displacedCenter.y - (circleDiameter / 2),
                                                             circleDiameter,
                                                             circleDiameter);

            g.drawEllipse(circleBounds, lineThickness);
        }
    }
    else if (style == CircleStyle::TWO)
    {
        // Set the line thickness
        float lineThickness = 1.0f;
        
        // Draw displaced circles
        int numberOfCircles = static_cast<int>(sliderPosProportional * 10);  // Max 10 circles for full slider value
        float maxDisplacement = 5.0f;  // Max displacement in pixels for the circle's position

        for (int i = 0; i < numberOfCircles; i++)
        {
            float progress = (i + 1) / (float)numberOfCircles;
            float circleDiameter = diameter * progress;
            float displacement = maxDisplacement * (10.0 - progress) * std::sin(float(i) * 0.4f * MathConstants<float>::pi);

            // Displace the center of each circle based on its index
            Point<float> displacedCenter(center.x + displacement, center.y + displacement);

            // Calculate the bounds for each circle
            Rectangle<float> circleBounds = Rectangle<float>(displacedCenter.x - (circleDiameter / 2),
                                                             displacedCenter.y - (circleDiameter / 2),
                                                             circleDiameter,
                                                             circleDiameter);

            g.drawEllipse(circleBounds, lineThickness);
        }
    }
}

//====================================================================
// TRIANGLE
//====================================================================
template<MaxStyle maxStyle>
void TriangleSliderLookAndFeel<maxStyle>::drawRotarySlider (juce::Graphics &g,
                                                            int x, int y, int width, int height,
                                                            float sliderPosProportional,
                                                            float rotaryStartAngle,
                                                            float rotaryEndAngle,
                                                            juce::Slider &slider)
{
    using namespace juce;
    float innerMargin = 10.0f;
    float diameter = std::min(width, height) - innerMargin;
    auto bounds = Rectangle<float>(x + (width - diameter) / 2, y + (height - diameter)/ 2, diameter, diameter);
    auto center = bounds.getCentre();
    float maxRadius = diameter / 2;
    // Determine the number of triangles to draw based on the slider's position
    int numberOfTriangles = static_cast<int>(sliderPosProportional * 10); // Max 10triangles for full slider value
    
    drawMaxLines (g, maxStyle, bounds, center.toFloat(), sliderPosProportional, maxRadius, diameter);
    
    g.setColour(Colours::white);
    
    for (int i = 1; i <= numberOfTriangles; i++)
    {
        float triangleSize = (maxRadius / 10) * i; // Size of each triangle
        float angleOffset = juce::MathConstants<float>::twoPi / numberOfTriangles; //Evenly space the triangles
        
        Path trianglePath;
        trianglePath.startNewSubPath(center.x, center.y - triangleSize); // Top pointof the triangle
        trianglePath.lineTo(center.x + triangleSize * 0.5f, center.y + triangleSize *0.5f); // Bottom right
        trianglePath.lineTo(center.x - triangleSize * 0.5f, center.y + triangleSize *0.5f); // Bottom left
        trianglePath.closeSubPath();
        // Rotate and position each triangle
        trianglePath.applyTransform(AffineTransform::rotation(angleOffset * i,center.x, center.y));
        g.strokePath (trianglePath, PathStrokeType (1.f));
    }
}
//====================================================================
// INDICATOR
//====================================================================
void IndicatorSliderLookAndFeel::drawRotarySlider (juce::Graphics &g,
                                                   int x, int y, int width, int height,
                                                   float sliderPosProportional,
                                                   float rotaryStartAngle,
                                                   float rotaryEndAngle,
                                                   juce::Slider &slider)
{
    using namespace juce;
    float innerMargin = 10.f;
    float diameter = std::min (width, height) - innerMargin;
    auto bounds = Rectangle<float>(x + (width - diameter) / 2,
                                    y + (height - diameter) / 2,
                                    diameter,
                                    diameter);
    auto center = bounds.getCentre();
    
    // Draw the partial track
    Path backgroundTrack;
    backgroundTrack.addCentredArc(center.x,
                                  center.y,
                                  diameter / 2,
                                  diameter / 2,
                                  0,
                                  rotaryStartAngle,
                                  rotaryEndAngle,
                                  true);
    g.setColour (Colours::white);
    g.strokePath (backgroundTrack, PathStrokeType (4.0f));

    // Create the indicator
    Path indicator;
    float indicatorWidth = 4.0f;
    float indicatorHeight = diameter * 0.5f;
    Rectangle<float> indicatorRect (center.x - indicatorWidth / 2,
                                    center.getY() - indicatorHeight,
                                    indicatorWidth,
                                    indicatorHeight);
    indicator.addRoundedRectangle(indicatorRect, 2.0f);

    // Calculate the angle for the current slider position
    auto sliderAngleRadians = jmap(sliderPosProportional,
                                   0.f,
                                   1.f,
                                   rotaryStartAngle,
                                   rotaryEndAngle);

    // Rotate the indicator to the current position
    indicator.applyTransform (AffineTransform().rotated (sliderAngleRadians,
                                                         center.getX(),
                                                         center.getY()));
    // Draw the indicator
    g.setColour (Colours::white);
    g.fillPath (indicator);
}
//====================================================================
// DOT
//====================================================================
void DotSliderLookAndFeel::drawRotarySlider (juce::Graphics &g,
                                             int x, int y, int width, int height,
                                             float sliderPosProportional,
                                             float rotaryStartAngle,
                                             float rotaryEndAngle,
                                             juce::Slider &slider)
{
    using namespace juce;
    float innerMargin = 10.f;
    float diameter = std::min (width, height) - innerMargin;
    auto bounds = Rectangle<float>(x + (width - diameter) / 2,
                                    y + (height - diameter) / 2,
                                    diameter,
                                    diameter);
    auto center = bounds.getCentre();
    
    // Draw the partial track
    Path backgroundTrack;
    backgroundTrack.addCentredArc(center.x,
                                  center.y,
                                  diameter / 2,
                                  diameter / 2,
                                  0,
                                  rotaryStartAngle,
                                  rotaryEndAngle,
                                  true);
    g.setColour (Colours::white);
    g.strokePath (backgroundTrack, PathStrokeType (4.0f));
    
    // DOT
    auto sliderAngleRadians = jmap(sliderPosProportional,
                                   0.f,
                                   1.f,
                                   rotaryStartAngle,
                                   rotaryEndAngle);

    float thumbRadius = 6.f;
    float radius = diameter / 2;
    auto thumbX = center.x + (radius - thumbRadius * 2) * std::cos (sliderAngleRadians - MathConstants<float>::halfPi);
    auto thumbY = center.y + (radius - thumbRadius * 2) * std::sin (sliderAngleRadians - MathConstants<float>::halfPi);

    g.setColour (Colours::white);
    g.fillEllipse(thumbX - thumbRadius,
                  thumbY - thumbRadius,
                  thumbRadius * 2,
                  thumbRadius * 2);
}
//====================================================================
// TOGGLE BUTTON
//====================================================================
void ButtonLookAndFeel::drawToggleButton (juce::Graphics &g,
                                          juce::ToggleButton &toggleButton,
                                          bool shouldDrawButtonAsHighlighted,
                                          bool shouldDrawButtonAsDown)
{
    using namespace juce;

    Rectangle<float> bounds = toggleButton.getLocalBounds().toFloat().reduced(10);  // Add some padding
    auto center = bounds.getCentre();
    float radius = bounds.getWidth() / 2;  // Base radius for both circle and spikes

    if (shouldDrawButtonAsDown)
    {
        // When the button is active, draw a more extreme jagged shape
        int numSpikes = 30;  // Increased number of spikes for more detail
        float baseRadius = radius * 0.5f;  // Reduced base radius for more extreme spikes
        float spikeLength = radius * 0.5f;  // Increased spike length

        Path dynamicOutline;
        float angleStep = MathConstants<float>::twoPi / numSpikes;

        for (int i = 0; i < numSpikes; i++) {
            float angle = angleStep * i;
            // Alternate between the base radius and spike radius
            float modRadius = (i % 2 == 0) ? baseRadius : baseRadius + spikeLength;
            float x = center.x + modRadius * std::cos(angle);
            float y = center.y + modRadius * std::sin(angle);

            if (i == 0) {
                dynamicOutline.startNewSubPath(x, y);
            } else {
                dynamicOutline.lineTo(x, y);
            }
        }
        dynamicOutline.closeSubPath();

        g.setColour(Colours::white);
        g.strokePath(dynamicOutline, PathStrokeType(2.0f));  // Thicker line for the jagged shape
    } else {
        // When the button is not active, draw a simple square instead of a circle
        g.setColour(Colours::white);
        
        auto squareBounds = bounds.reduced(radius * 0.5f);
        squareBounds.setHeight(squareBounds.getWidth());
        
        g.drawRect(squareBounds, 2.0f); // Draw a square with slightly reduced bounds for aesthetics
    }
}



void CustomButton::paint (juce::Graphics& g)
{
    using namespace juce;
    
    bool isHighlighted = isMouseOver();
    bool isDown = getToggleState();
    
    getLookAndFeel().drawToggleButton (g, *this, isHighlighted, isDown);
}

//====================================================================
// CUSTOM ROTARY SLIDER - paint()
//====================================================================
template<SliderStyle style, CircleStyle circleStyle, int numSpins, MaxStyle maxStyle>
void CustomRotarySlider<style, circleStyle, numSpins, maxStyle>::paint (juce::Graphics &g)
{
    using namespace juce;
    
    auto bounds = getLocalBounds();
    auto range = getRange();
    
    auto startAng = degreesToRadians (180.f + 45.f);
    auto endAng = degreesToRadians (180.f - 45.f) + MathConstants<float>::twoPi;
    
    getLookAndFeel().drawRotarySlider (g,
                                       bounds.getX(),
                                       bounds.getY(),
                                       bounds.getWidth(),
                                       bounds.getHeight(),
                                       jmap (getValue(), range.getStart(), range.getEnd(), 0.0, 1.0),
                                       startAng,
                                       endAng,
                                       *this);
}

//==============================================================================
Weird_filterAudioProcessorEditor::Weird_filterAudioProcessorEditor (Weird_filterAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p),
crush_bits_slider ("crush bits"),
pitch_slider ("pitch"),
delay_size_slider ("del size"),
freq_slider ("cut-freq"),
structure_slider ("ring"),

glitch_button_attachment        (p.apvts, "glitch", glitch_button),
crush_bits_slider_attachment    (p.apvts, "crush bits", crush_bits_slider),
pitch_slider_attachment         (p.apvts, "pitch", pitch_slider),
delay_size_slider_attachment    (p.apvts, "del size", delay_size_slider),
freq_slider_attachment          (p.apvts, "freq", freq_slider),
structure_slider_attachment     (p.apvts, "ring", structure_slider),

route_switch_attachment         (p.apvts, "route", route_switch)
{
    setSize (400, 300);
    
    glitch_button.onToggledCallback = [&]()
    {
        if (glitch_button.getToggleState())
        {
            glitch_time = true;
            repaint();
        }
        else
        {
            glitch_time = false;
            repaint();
        }
    };
    
    for (auto* comp : getComps())
    {
        addAndMakeVisible (comp);
    }
}

Weird_filterAudioProcessorEditor::~Weird_filterAudioProcessorEditor()
{
}

//==============================================================================
void Weird_filterAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colours::black);
}

void Weird_filterAudioProcessorEditor::paintOverChildren (juce::Graphics& g)
{
    auto bounds = getLocalBounds().reduced(10);
    
    using namespace juce;
    
    if (glitch_time)
    {
        g.fillAll (Colours::black);
        
        Random random;
        int numberOfLines = 80;

        for (int i = 0; i < numberOfLines; i++)
        {
            float startX = bounds.getX() + random.nextFloat() * bounds.getWidth();  // Random start X within the bounds
            float startY = bounds.getY() + random.nextFloat() * bounds.getHeight(); // Random start Y within the bounds
            float endX = bounds.getX() + random.nextFloat() * bounds.getWidth();    // Random end X within the bounds
            float endY = bounds.getY() + random.nextFloat() * bounds.getHeight();   // Random end Y within the bounds
            
            g.setColour(Colours::red);
            g.drawLine(Line<float>(startX, startY, endX, endY), 2.0f);
        }
    }
}

void Weird_filterAudioProcessorEditor::resized()
{
    auto bounds = getLocalBounds();
    
    auto footer_height = bounds.getHeight() * 0.2f;
    auto remaining_height = bounds.getHeight() - footer_height;
    
    auto top_half_bounds = bounds.removeFromTop (remaining_height / 2);
    auto bottom_half_bounds = bounds.removeFromTop (remaining_height / 2);
    
    auto footer_bounds = bounds.removeFromBottom (footer_height);
    
    using namespace juce;
    std::array<FlexBox, 3> flex;
    for (auto f : flex)
    {
        f.flexDirection = FlexBox::Direction::row;
        f.justifyContent = FlexBox::JustifyContent::spaceAround;
        f.alignItems = FlexBox::AlignItems::center;
    }
    
    auto verticalMargin = FlexItem().withFlex (0, 2, 10);
    auto item_margin = 10;
    
    // TOP ROW
    flex[0].items.add (verticalMargin);
    flex[0].items.add (FlexItem (freq_slider)
                       .withFlex(1)
                       .withMargin (item_margin));
    flex[0].items.add (FlexItem (pitch_slider)
                       .withFlex(1)
                       .withMargin (item_margin));
    flex[0].items.add (FlexItem (structure_slider)
                       .withFlex(1)
                       .withMargin (item_margin));
    flex[0].items.add (verticalMargin);
    
    // BOTTOM ROW
    flex[1].items.add (verticalMargin);
    flex[1].items.add (FlexItem (crush_bits_slider)
                       .withFlex(1)
                       .withMargin (item_margin));
    flex[1].items.add (FlexItem (delay_size_slider)
                       .withFlex(1)
                       .withMargin (item_margin));
    flex[1].items.add (FlexItem (glitch_button)
                       .withFlex(1)
                       .withMargin (item_margin));
    flex[1].items.add (verticalMargin);
    
    // FOOTER
    flex[2].items.add (verticalMargin);
    flex[2].items.add (FlexItem (route_switch)
                       .withFlex (1)
                       .withMargin (item_margin));
    flex[2].items.add (verticalMargin);
    
    flex[0].performLayout (top_half_bounds.toFloat());
    flex[1].performLayout (bottom_half_bounds.toFloat());
    flex[2].performLayout (footer_bounds.toFloat());
}

std::vector<juce::Component*> Weird_filterAudioProcessorEditor::getComps()
{
    return
    {
        &crush_bits_slider,
        &pitch_slider,
        &delay_size_slider,
        &freq_slider,
        &structure_slider,
        &glitch_button,
        &route_switch
    };
}
