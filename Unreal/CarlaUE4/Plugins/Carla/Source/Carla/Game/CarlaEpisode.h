// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "Carla/Actor/ActorDispatcher.h"
#include "Carla/Recorder/CarlaRecorder.h"
#include "Carla/Sensor/WorldObserver.h"
#include "Carla/Server/CarlaServer.h"
#include "Carla/Settings/EpisodeSettings.h"
#include "Carla/Util/ActorAttacher.h"
#include "Carla/Weather/Weather.h"

#include "GameFramework/Pawn.h"

#include <compiler/disable-ue4-macros.h>
#include <carla/geom/BoundingBox.h>
#include <carla/geom/GeoLocation.h>
#include <carla/rpc/Actor.h>
#include <carla/rpc/ActorDescription.h>
#include <carla/rpc/OpendriveGenerationParameters.h>
#include <carla/streaming/Server.h>
#include <compiler/enable-ue4-macros.h>

#include "CarlaEpisode.generated.h"

/// A simulation episode.
///
/// Each time the level is restarted a new episode is created.
UCLASS(BlueprintType, Blueprintable)
class CARLA_API UCarlaEpisode : public UObject
{
  GENERATED_BODY()

  // ===========================================================================
  // -- Constructor ------------------------------------------------------------
  // ===========================================================================

public:

  UCarlaEpisode(const FObjectInitializer &ObjectInitializer);

  // ===========================================================================
  // -- Load a new episode -----------------------------------------------------
  // ===========================================================================

  /// Load a new map and start a new episode.
  ///
  /// If @a MapString is empty, the current map is reloaded.
  UFUNCTION(BlueprintCallable)
  bool LoadNewEpisode(const FString &MapString, bool ResetSettings = true);

  /// Load a new map generating the mesh from OpenDRIVE data and
  /// start a new episode.
  ///
  /// If @a MapString is empty, it fails.
  bool LoadNewOpendriveEpisode(
      const FString &OpenDriveString,
      const carla::rpc::OpendriveGenerationParameters &Params);

  // ===========================================================================
  // -- Episode settings -------------------------------------------------------
  // ===========================================================================

  UFUNCTION(BlueprintCallable)
  const FEpisodeSettings &GetSettings() const
  {
    return EpisodeSettings;
  }

  UFUNCTION(BlueprintCallable)
  void ApplySettings(const FEpisodeSettings &Settings);

  // ===========================================================================
  // -- Retrieve info about this episode ---------------------------------------
  // ===========================================================================

  /// Return the unique id of this episode.
  auto GetId() const
  {
    return Id;
  }

  /// Return the name of the map loaded in this episode.
  UFUNCTION(BlueprintCallable)
  const FString &GetMapName() const
  {
    return MapName;
  }

  /// Game seconds since the start of this episode.
  double GetElapsedGameTime() const
  {
    return ElapsedGameTime;
  }

  /// Return the list of actor definitions that are available to be spawned this
  /// episode.
  UFUNCTION(BlueprintCallable)
  const TArray<FActorDefinition> &GetActorDefinitions() const
  {
    return ActorDispatcher->GetActorDefinitions();
  }

  /// Return the list of recommended spawn points for vehicles.
  UFUNCTION(BlueprintCallable)
  TArray<FTransform> GetRecommendedSpawnPoints() const;

  /// Return the GeoLocation point of the map loaded
  const carla::geom::GeoLocation &GetGeoReference() const
  {
    return MapGeoReference;
  }

  // ===========================================================================
  // -- Retrieve special actors ------------------------------------------------
  // ===========================================================================

  UFUNCTION(BlueprintCallable)
  APawn *GetSpectatorPawn() const
  {
    return Spectator;
  }

  UFUNCTION(BlueprintCallable)
  AWeather *GetWeather() const
  {
    return Weather;
  }

  const FActorRegistry &GetActorRegistry() const
  {
    return ActorDispatcher->GetActorRegistry();
  }

  FActorRegistry &GetActorRegistry()
  {
    return ActorDispatcher->GetActorRegistry();
  }

  FCarlaActor* RegisterActor(AActor &Actor, FActorDescription ActorDescription, FString &Tags, FActorRegistry::IdType DesiredId = 0)
  {
    // first bind the already-spawned actor so it does not get spawned but found
    auto FindFunctor = [&](const FTransform &Transform, const FActorDescription &Description) {
      return FActorSpawnResult(&Actor);
    };
    // need to create an FActorDefinition from our FActorDescription for some reason -_-
    FActorDefinition ActorDefinition;
    ActorDefinition.Id = ActorDescription.Id;
    ActorDefinition.Class = ActorDescription.Class;
    ActorDefinition.Tags = Tags; // list of comma-separated tags
    ActorDispatcher->Bind(ActorDefinition, FindFunctor);
    // take the UId from the ActorDefinition which has just been updated in ActorDispatcher.cpp:BindAndReturnUId
    ActorDescription.UId = ActorDispatcher->GetActorDefinitions().Last().UId;
    // then register this actor with the ActorDispatcher
    return ActorDispatcher->RegisterActor(Actor, ActorDescription, DesiredId);
  }


  // ===========================================================================
  // -- Actor look up methods --------------------------------------------------
  // ===========================================================================

  /// Find a Carla actor by id.
  ///
  /// If the actor is not found or is pending kill, the returned view is
  /// invalid.
  FCarlaActor* FindCarlaActor(FCarlaActor::IdType ActorId)
  {
    return ActorDispatcher->GetActorRegistry().FindCarlaActor(ActorId);
  }

  /// Find the actor view of @a Actor.
  ///
  /// If the actor is not found or is pending kill, the returned view is
  /// invalid.
  FCarlaActor* FindCarlaActor(AActor *Actor) const
  {
    return ActorDispatcher->GetActorRegistry().FindCarlaActor(Actor);
  }

  // ===========================================================================
  // -- Actor handling methods -------------------------------------------------
  // ===========================================================================

  /// Spawns an actor based on @a ActorDescription at @a Transform. To properly
  /// despawn an actor created with this function call DestroyActor.
  ///
  /// @return A pair containing the result of the spawn function and a view over
  /// the actor and its properties. If the status is different of Success the
  /// view is invalid.
  TPair<EActorSpawnResultStatus, FCarlaActor*> SpawnActorWithInfo(
      const FTransform &Transform,
      FActorDescription thisActorDescription,
      FCarlaActor::IdType DesiredId = 0);

  /// Spawns an actor based on @a ActorDescription at @a Transform.
  ///
  /// @return the actor to be spawned
  AActor* ReSpawnActorWithInfo(
      const FTransform &Transform,
      FActorDescription thisActorDescription)
  {
    FTransform NewTransform = Transform;
    auto result = ActorDispatcher->ReSpawnActor(NewTransform, thisActorDescription);
    if (Recorder->IsEnabled())
    {
      // do something?
    }

    return result;
  }

  /// Spawns an actor based on @a ActorDescription at @a Transform. To properly
  /// despawn an actor created with this function call DestroyActor.
  ///
  /// @return nullptr on failure.
  ///
  /// @note Special overload for blueprints.
  UFUNCTION(BlueprintCallable)
  AActor *SpawnActor(
      const FTransform &Transform,
      FActorDescription ActorDescription)
  {
    return SpawnActorWithInfo(Transform, std::move(ActorDescription)).Value->GetActor();
  }

  /// Attach @a Child to @a Parent.
  ///
  /// @pre Actors cannot be null.
  UFUNCTION(BlueprintCallable)
  void AttachActors(
      AActor *Child,
      AActor *Parent,
      EAttachmentType InAttachmentType = EAttachmentType::Rigid);

  /// @copydoc FActorDispatcher::DestroyActor(AActor*)
  UFUNCTION(BlueprintCallable)
  bool DestroyActor(AActor *Actor)
  {
    FCarlaActor* CarlaActor = FindCarlaActor(Actor);
    if (CarlaActor)
    {
      carla::rpc::ActorId ActorId = CarlaActor->GetActorId();
      return DestroyActor(ActorId);
    }
    return false;
  }

  bool DestroyActor(carla::rpc::ActorId ActorId)
  {
    if (Recorder->IsEnabled())
    {
      // recorder event
      CarlaRecorderEventDel RecEvent{ActorId};
      Recorder->AddEvent(std::move(RecEvent));
    }

    return ActorDispatcher->DestroyActor(ActorId);
  }

  void PutActorToSleep(carla::rpc::ActorId ActorId)
  {
    ActorDispatcher->PutActorToSleep(ActorId, this);
  }

  void WakeActorUp(carla::rpc::ActorId ActorId)
  {
    ActorDispatcher->WakeActorUp(ActorId, this);
  }

  // ===========================================================================
  // -- Other methods ----------------------------------------------------------
  // ===========================================================================

  /// Create a serializable object describing the actor.
  carla::rpc::Actor SerializeActor(FCarlaActor* CarlaActor) const;

  /// Create a serializable object describing the actor.
  /// Can be used to serialized actors that are not in the registry
  carla::rpc::Actor SerializeActor(AActor* Actor) const;

  // ===========================================================================
  // -- Private methods and members --------------------------------------------
  // ===========================================================================

  ACarlaRecorder *GetRecorder() const
  {
    return Recorder;
  }

  void SetRecorder(ACarlaRecorder *Rec)
  {
    Recorder = Rec;
  }

  CarlaReplayer *GetReplayer() const
  {
    return Recorder->GetReplayer();
  }

  std::string StartRecorder(std::string name, bool AdditionalData);

  FIntVector GetCurrentMapOrigin() const { return CurrentMapOrigin; }

  void SetCurrentMapOrigin(const FIntVector& NewOrigin) { CurrentMapOrigin = NewOrigin; }

private:

  friend class ACarlaGameModeBase;
  friend class FCarlaEngine;

  void InitializeAtBeginPlay();

  void EndPlay();

  void RegisterActorFactory(ACarlaActorFactory &ActorFactory)
  {
    ActorDispatcher->Bind(ActorFactory);
  }

  std::pair<int, FCarlaActor&> TryToCreateReplayerActor(
    FVector &Location,
    FVector &Rotation,
    FActorDescription &ActorDesc,
    unsigned int desiredId);

  bool SetActorSimulatePhysics(FCarlaActor &CarlaActor, bool bEnabled);

  void TickTimers(float DeltaSeconds)
  {
    ElapsedGameTime += DeltaSeconds;
  }

  const uint64 Id = 0u;

  double ElapsedGameTime = 0.0;

  UPROPERTY(VisibleAnywhere)
  FString MapName;

  UPROPERTY(VisibleAnywhere)
  FEpisodeSettings EpisodeSettings;

  UPROPERTY(VisibleAnywhere)
  UActorDispatcher *ActorDispatcher = nullptr;

  UPROPERTY(VisibleAnywhere)
  APawn *Spectator = nullptr;

  UPROPERTY(VisibleAnywhere)
  AWeather *Weather = nullptr;

  ACarlaRecorder *Recorder = nullptr;

  carla::geom::GeoLocation MapGeoReference;

  FIntVector CurrentMapOrigin;
};
