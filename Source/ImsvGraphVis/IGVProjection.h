// Copyright 2017 Oh-Hyun Kwon. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"

#include "IGVProjection.generated.h"

UENUM(BlueprintType)
enum class EIGVProjection : uint8
{
	Sphere_SphericalCoordinates,
	Sphere_Gnomonic,
	Sphere_Gnomonic_RadialWarping,
	Sphere_Gnomonic_IndependentWarping,
	Sphere_Stereographic,
	Sphere_Stereographic_RadialWarping,
	Sphere_Stereographic_IndependentWarping
};

UCLASS()
class IMSVGRAPHVIS_API UIGVProjection : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = ImmersiveGraphVisualization)
	static FORCEINLINE FVector To3D(FVector2D const& P, float const X = 0.f)
	{
		return FVector(X, P.X, P.Y);
	}

	UFUNCTION(BlueprintCallable, Category = ImmersiveGraphVisualization)
	static FORCEINLINE FVector ToSphere_SphericalCoordinates(FVector2D const& P)
	{
		float const PI_2 = PI * 0.5;

		FVector2D const V = FVector2D(
			FMath::GetMappedRangeValueClamped(FVector2D(-PI_2, PI_2), FVector2D(0, PI), -P.Y), P.X);
		return V.SphericalToUnitCartesian();
	}

	UFUNCTION(BlueprintCallable, Category = ImmersiveGraphVisualization)
	static FORCEINLINE FVector ToSphere_Gnomonic(FVector2D const& P)
	{
		return To3D(P, 1).GetSafeNormal();
	}

	UFUNCTION(BlueprintCallable, Category = ImmersiveGraphVisualization)
	static FORCEINLINE FVector ToSphere_Gnomonic_RadialWarping(FVector2D const& P)
	{
		FVector2D const N = P.GetSafeNormal();
		float const S = P.Size();
		FVector const V = To3D(N * FMath::Tan(S), 1).GetSafeNormal();
		check(S < PI);
		return (S < PI * 0.5f) ? V : -1 * V;
	}

	UFUNCTION(BlueprintCallable, Category = ImmersiveGraphVisualization)
	static FORCEINLINE FVector ToSphere_Gnomonic_IndependentWarping(FVector2D const& P)
	{
		FVector2D const V = FVector2D(FMath::Tan(P.X), FMath::Tan(P.Y));
		return To3D(V, 1).GetSafeNormal();
	}

	UFUNCTION(BlueprintCallable, Category = ImmersiveGraphVisualization)
	static FORCEINLINE FVector ToSphere_Stereographic(FVector2D P)
	{
		float const R = P.Size() * .5;
		float const Phi = FMath::Atan2(P.Y, P.X);
		FVector const V = FVector2D(2 * FMath::Atan(1.0 / R), Phi).SphericalToUnitCartesian();
		return FVector(-V.Z, V.X, V.Y);
	}

	UFUNCTION(BlueprintCallable, Category = ImmersiveGraphVisualization)
	static FORCEINLINE FVector ToSphere_Stereographic_RadialWarping(FVector2D const& P)
	{
		float const R = FMath::Tan(P.Size() * .5);
		float const Phi = FMath::Atan2(P.Y, P.X);
		FVector const V = FVector2D(2 * FMath::Atan(1.0 / R), Phi).SphericalToUnitCartesian();
		return FVector(-V.Z, V.X, V.Y);
	}

	UFUNCTION(BlueprintCallable, Category = ImmersiveGraphVisualization)
	static FORCEINLINE FVector ToSphere_Stereographic_IndependentWarping(FVector2D const& P)
	{
		FVector2D const Q = FVector2D(FMath::Tan(P.X * .5), FMath::Tan(P.Y * .5));
		float const R = Q.Size();
		float const Phi = FMath::Atan2(Q.Y, Q.X);
		FVector const V = FVector2D(2 * FMath::Atan(1.0 / R), Phi).SphericalToUnitCartesian();
		return FVector(-V.Z, V.X, V.Y);
	}
};
