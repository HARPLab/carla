#ifndef DREYEVR_UTIL
#define DREYEVR_UTIL

#include "CoreMinimal.h"
#include "HighResScreenshot.h" // FHighResScreenshotConfig
#include "ImageWriteQueue.h"   // TImagePixelData
#include "ImageWriteTask.h"    // FImageWriteTask
#include <string>

/// this is the file where we'll read all DReyeVR specific configs
static const FString ConfigFile =
    FPaths::Combine(FPaths::ConvertRelativePathToFull(FPaths::ProjectDir()), TEXT("Config"), TEXT("DReyeVRConfig.ini"));

static FString ReadKeyInConfig(const FString &FileNameWithPath, const FString &Key)
{
    FString value("");
    std::string stdKey = std::string(TCHAR_TO_UTF8(*Key));
    std::ifstream ConfigFile(TCHAR_TO_ANSI(*FileNameWithPath));
    if (ConfigFile)
    {
        std::string Line;
        while (std::getline(ConfigFile, Line))
        {
            std::istringstream iss_Line(Line);
            std::string FileKey;
            if (std::getline(iss_Line, FileKey, '=')) // gets left side of '=' into FileKey
            {
                if (FileKey.compare(stdKey) == 0)
                {
                    std::string value;
                    if (std::getline(iss_Line, value))
                    {
                        return FString(value.c_str());
                    }
                }
            }
        }
    }
    else
    {
        std::cerr << "Unable to open the config file " << std::string(TCHAR_TO_UTF8(*FileNameWithPath)) << std::endl;
    }
    std::cerr << "Coult not find key \"" << stdKey << "\"" << std::endl;
    return value;
}

static void ReadConfigValue(const FString &FileNameWithPath, const FString &VariableName, bool &Value)
{
    const FString ReadValue = ReadKeyInConfig(FileNameWithPath, VariableName);
    Value = ReadValue.ToBool();
}
static void ReadConfigValue(const FString &FileNameWithPath, const FString &VariableName, int &Value)
{
    const FString ReadValue = ReadKeyInConfig(FileNameWithPath, VariableName);
    Value = FCString::Atoi(*ReadValue);
}
static void ReadConfigValue(const FString &FileNameWithPath, const FString &VariableName, float &Value)
{
    const FString ReadValue = ReadKeyInConfig(FileNameWithPath, VariableName);
    Value = FCString::Atof(*ReadValue);
}
static void ReadConfigValue(const FString &FileNameWithPath, const FString &VariableName, FString &Value)
{
    const FString ReadValue = ReadKeyInConfig(FileNameWithPath, VariableName);
    Value = ReadValue;
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
    UE_LOG(LogTemp, Log, TEXT("saving image"));
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