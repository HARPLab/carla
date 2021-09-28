#pragma once

#include "Carla/Game/CarlaHUD.h" // ACarlaHUD
#include "Containers/Array.h"    // TArray
#include "Engine/Font.h"         // UFont
#include "GameFramework/HUD.h"   // AHUD

#include "DReyeVRHUD.generated.h"

struct HUDText
{
    FString Text{""}; // Text content
    FVector2D Screen; // 2D position on the screen
    FColor Colour;    // RGBA text colour
    float Scale;      // size of letters
    UFont *TypeFace;  // default font chosen if NULL
};

struct TimedText
{
    HUDText HT;
    float TimeToDie;
};

struct HUDLine
{
    FVector2D Start; // start pt (2d) of line
    FVector2D End;   // end pt (2d) of line
    FColor Colour;   // RGBA line colour
    float Thickness; // thickness (in px) of the line
};

struct HUDRect
{
    FVector2D TopLeft;
    FVector2D BottomRight;
    FColor Colour;
    float Thickness;
};

/// DReyeVR class to draw on HUD
UCLASS() class ADReyeVRHUD : public ACarlaHUD
{
    GENERATED_BODY()

  public:
    ADReyeVRHUD(const FObjectInitializer &ObjectInitializer) : Super(ObjectInitializer)
    {
    }

    virtual void DrawHUD() override;

    void SetPlayer(APlayerController *P);

    // drawing text
    void DrawDynamicText(const FString &Str, const FVector2D &Screen, const FColor &Colour, const float Scale,
                         const UFont *Font = nullptr);
    void DrawDynamicText(const FString &Str, const FVector &Loc, const FColor &Colour, const float Scale,
                         const bool bIsRelative, const UFont *Font = nullptr);

    // static (lifetime based) text
    void DrawStaticText(const FString &Str, const FVector2D &Loc, const FColor &Colour, const float Scale,
                        const float LifeTime, const UFont *Font = nullptr);
    void DrawStaticText(const FString &Str, const FVector &Loc, const FColor &Colour, const float Scale,
                        const float LifeTime, const bool bIsRelative, const UFont *Font = nullptr);

    // drawing lines
    void DrawDynamicLine(const FVector &Start, const FVector &End, const FColor &Colour, const float Thickness);
    void DrawDynamicLine(const FVector2D &Start, const FVector2D &End, const FColor &Colour, const float Thickness);

    // drawing squares/rectangles
    void DrawDynamicSquare(const FVector &Center, const float Size, const FColor &Colour, const float Thickness = 0);
    void DrawDynamicSquare(const FVector2D &Center, const float Size, const FColor &Colour, const float Thickness = 0);
    void DrawDynamicRect(const FVector2D &TopLeft, const FVector2D &BottomRight, const FColor &Colour,
                         const float Thickness = 0); // 0 thickness is default

  private:
    /// TODO: figure out a better way than this to dynamically render
    TArray<HUDText> DynamicTextList;     // get drawn once (on DrawHUD) then removed (replaced) by DrawDynamicText
    TArray<HUDLine> DynamicLineList;     // get drawn once (on DrawHUD) then removed (replaced) by DrawDynamicLine
    TArray<HUDRect> DynamicRectList;     // get drawn once (on DrawHUD) then removed (replaced) by DrawDynamicLine
    TArray<TimedText> StaticTextList;    // get added and remain until lifetime ends
    APlayerController *Player = nullptr; // PlayerComponent in world
};
