// Copyright 2017 Oh-Hyun Kwon. All Rights Reserved.

#pragma once

#include "Engine.h"

#include "KWColorSpace.generated.h"

UCLASS()
class UKWColorSpace : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	// Corresponds roughly to RGB brighter/darker
	static float const LAB_K;

	// D65 standard referent
	static float const LAB_X;
	static float const LAB_Y;
	static float const LAB_Z;

	static float const HCL_Gamma;
	static float const HCL_Y0;
	static float const HCL_MaxL;

public:
	UFUNCTION(BlueprintCallable, Category = "Color Space")
	static FVector HCLtoRGB(float H, float const C, float const L);
	static FVector HCLtoRGB(FVector const& HCL);

	/*
	 * @param RGB	{0..1, 0..1, 0..1}
	 */
	// static FVector RGBtoLAB(FVector const& RGB);

	UFUNCTION(BlueprintCallable, Category = "Color Space")
	static FVector RGBtoHCL(float R, float G, float B);
	static FVector RGBtoHCL(FVector const& RGB);
	static FVector RGBtoHCL(FLinearColor const& RGB);

	UFUNCTION(BlueprintCallable, Category = "Color Space")
	static FVector HSLtoIsoIluminatedRGB(float H, float S, float L);

	/*
	 * @param	A			HCL Color
	 * @param	B			HCL Color
	 * @param	Alpha		0..1
	 * @return	LAB Color
	 */
	UFUNCTION(BlueprintCallable, Category = "Color Space")
	static FVector LerpHCL(FVector const& A, FVector const& B, float const Alpha);
};
