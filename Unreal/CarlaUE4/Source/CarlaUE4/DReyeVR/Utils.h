#ifndef DREYEVR_UTIL
#define DREYEVR_UTIL

#include "CoreMinimal.h"
#include "HighResScreenshot.h" // FHighResScreenshotConfig
#include "ImageWriteQueue.h"   // TImagePixelData
#include "ImageWriteTask.h"    // FImageWriteTask
#include <fstream>             // std::ifstream
#include <sstream>             // std::istringstream
#include <string>
#include <unordered_map>

/// this is the file where we'll read all DReyeVR specific configs
static const FString ConfigFilePath =
    FPaths::Combine(FPaths::ConvertRelativePathToFull(FPaths::ProjectDir()), TEXT("Config"), TEXT("DReyeVRConfig.ini"));

static std::unordered_map<std::string, FString> Params = {};

static std::string CreateVariableName(const std::string &Section, const std::string &Variable)
{
    return Section + "/" + Variable; // encoding the variable alongside its section
}
static std::string CreateVariableName(const FString &Section, const FString &Variable)
{
    return CreateVariableName(std::string(TCHAR_TO_UTF8(*Section)), std::string(TCHAR_TO_UTF8(*Variable)));
}

static void ReadDReyeVRConfig()
{
    /// TODO: add feature to "hot-reload" new params during runtime
    UE_LOG(LogTemp, Warning, TEXT("Reading config from %s"), *ConfigFilePath);
    /// performs a single pass over the config file to collect all variables into Params
    std::ifstream ConfigFile(TCHAR_TO_ANSI(*ConfigFilePath));
    if (ConfigFile)
    {
        std::string Line;
        std::string Section = "";
        while (std::getline(ConfigFile, Line))
        {
            // std::string stdKey = std::string(TCHAR_TO_UTF8(*Key));
            if (Line[0] == ';') // ignore comments
                continue;
            std::istringstream iss_Line(Line);
            if (Line[0] == '[') // test section
            {
                std::getline(iss_Line, Section, ']');
                Section = Section.substr(1); // skip leading '['
                continue;
            }
            std::string Key;
            if (std::getline(iss_Line, Key, '=')) // gets left side of '=' into FileKey
            {
                std::string Value;
                if (std::getline(iss_Line, Value, ';')) // gets left side of ';' for comments
                {
                    std::string VariableName = CreateVariableName(Section, Key);
                    FString VariableValue = FString(Value.c_str());
                    Params[VariableName] = VariableValue;
                }
            }
        }
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Unable to open the config file %s"), *ConfigFilePath);
    }
    // for (auto &e : Params){
    //     UE_LOG(LogTemp, Warning, TEXT("%s: %s"), *FString(e.first.c_str()), *e.second);
    // }
}

static void EnsureConfigsUpdated()
{
    // used to ensure the configs file has been read and contents updated
    if (Params.size() == 0)
        ReadDReyeVRConfig();
}

static void ReadConfigValue(const FString &Section, const FString &Variable, bool &Value)
{
    EnsureConfigsUpdated();
    std::string VariableName = CreateVariableName(Section, Variable);
    if (Params.find(VariableName) != Params.end())
        Value = Params[VariableName].ToBool();
    else
        UE_LOG(LogTemp, Error, TEXT("No variable matching %s found"), *FString(VariableName.c_str()));
}
static void ReadConfigValue(const FString &Section, const FString &Variable, int &Value)
{
    EnsureConfigsUpdated();
    std::string VariableName = CreateVariableName(Section, Variable);
    if (Params.find(VariableName) != Params.end())
        Value = FCString::Atoi(*Params[VariableName]);
    else
        UE_LOG(LogTemp, Error, TEXT("No variable matching %s found"), *FString(VariableName.c_str()));
}
static void ReadConfigValue(const FString &Section, const FString &Variable, float &Value)
{
    EnsureConfigsUpdated();
    std::string VariableName = CreateVariableName(Section, Variable);
    if (Params.find(VariableName) != Params.end())
        Value = FCString::Atof(*Params[VariableName]);
    else
        UE_LOG(LogTemp, Error, TEXT("No variable matching %s found"), *FString(VariableName.c_str()));
}
static void ReadConfigValue(const FString &Section, const FString &Variable, FVector &Value)
{
    EnsureConfigsUpdated();
    std::string VariableName = CreateVariableName(Section, Variable);
    if (Params.find(VariableName) != Params.end())
    {
        if (Value.InitFromString(Params[VariableName]) == false)
        {
            UE_LOG(LogTemp, Error, TEXT("Unable to construct FVector for %s from %s"), *FString(VariableName.c_str()),
                   *(Params[VariableName]));
        }
    }
    else
        UE_LOG(LogTemp, Error, TEXT("No variable matching %s found"), *FString(VariableName.c_str()));
}
static void ReadConfigValue(const FString &Section, const FString &Variable, FRotator &Value)
{
    // simplu uses the same format as with FVector
    FVector Tmp;
    ReadConfigValue(Section, Variable, Tmp);
    Value = FRotator(Tmp.Y, Tmp.Z, Tmp.X).Clamp(); // Convert to euler FRotators
}
static void ReadConfigValue(const FString &Section, const FString &Variable, FString &Value)
{
    EnsureConfigsUpdated();
    std::string VariableName = CreateVariableName(Section, Variable);
    if (Params.find(VariableName) != Params.end())
        Value = Params[VariableName];
    else
        UE_LOG(LogTemp, Error, TEXT("No variable matching %s found"), *FString(VariableName.c_str()));
}

static void SaveFrameToDisk(UTextureRenderTarget2D &RenderTarget, const FString &FilePath)
{
    FTextureRenderTargetResource *RTResource = RenderTarget.GameThread_GetRenderTargetResource();
    const size_t H = RenderTarget.GetSurfaceHeight();
    const size_t W = RenderTarget.GetSurfaceWidth();
    const FIntPoint DestSize(W, H);
    TImagePixelData<FColor> PixelData(DestSize);
    TArray<FColor> Pixels;
    Pixels.AddUninitialized(H * W);
    FReadSurfaceDataFlags ReadPixelFlags(RCM_MinMax); // RCM_UNorm);
    ReadPixelFlags.SetLinearToGamma(false);
    bool Success = RTResource->ReadPixels(Pixels, ReadPixelFlags);
    PixelData.Pixels = Pixels;
    TUniquePtr<FImageWriteTask> ImageTask = MakeUnique<FImageWriteTask>();
    ImageTask->PixelData = MakeUnique<TImagePixelData<FColor>>(PixelData);
    ImageTask->Filename = FilePath;
    ImageTask->Format = EImageFormat::JPEG;
    ImageTask->CompressionQuality = (int32)EImageCompressionQuality::Default;
    ImageTask->bOverwriteFile = true;
    ImageTask->PixelPreProcessors.Add(TAsyncAlphaWrite<FColor>(255));
    FHighResScreenshotConfig &HighResScreenshotConfig = GetHighResScreenshotConfig();
    HighResScreenshotConfig.ImageWriteQueue->Enqueue(MoveTemp(ImageTask));

    /// TODO: write the OutBMP to disk via some ppm faniciness??
    // might need to buffer several images then write all at once

    // std::ofstream ofs(TCHAR_TO_ANSI(*FilePath), std::ios_base::out | std::ios_base::binary);
    // ofs << "P6" << std::endl << W << ' ' << H << std::endl << "255" << std::endl;

    // for (size_t i = 0u; i < Pixels.Num(); ++i)
    // {
    //     const FColor &RGB = Pixels[i];
    //     ofs << char(RGB.R) << char(RGB.G) << char(RGB.B);
    // }
    // ofs.close();
}

#endif