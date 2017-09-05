// Copyright 2017 Oh-Hyun Kwon. All Rights Reserved.

#include "KWColorSpace.h"

float const UKWColorSpace::LAB_K = 18;
float const UKWColorSpace::LAB_X = 0.950470;
float const UKWColorSpace::LAB_Y = 1;
float const UKWColorSpace::LAB_Z = 1.088830;

float const UKWColorSpace::HCL_Gamma = 3;
float const UKWColorSpace::HCL_Y0 = 100;
float const UKWColorSpace::HCL_MaxL = 0.530454533953517;  // == exp(HCLgamma / HCLy0) - 0.5

FVector UKWColorSpace::HCLtoRGB(float H, float C, float L)
{
	float R = 0;
	float G = 0;
	float B = 0;

	if (L == 0) return FVector::ZeroVector;

	L = L * HCL_MaxL;
	float const Q = FMath::Exp((1 - C / (2 * L)) * (HCL_Gamma / HCL_Y0));
	float const U = (2 * L - C) / (2 * Q - 1);
	float const V = C / Q;
	float const T =
		FMath::Tan((H + FMath::Min(FMath::Frac(2 * H) / 4, FMath::Frac(-2 * H) / 8)) * PI * 2);

	H *= 6;
	if (H <= 1)
	{
		R = 1;
		G = T / (1 + T);
	}
	else if (H <= 2)
	{
		R = (1 + T) / T;
		G = 1;
	}
	else if (H <= 3)
	{
		G = 1;
		B = 1 + T;
	}
	else if (H <= 4)
	{
		G = 1 / (1 + T);
		B = 1;
	}
	else if (H <= 5)
	{
		R = -1 / T;
		B = 1;
	}
	else
	{
		R = 1;
		B = -T;
	}

	return FVector(R, G, B) * V + U;
}

FVector UKWColorSpace::HCLtoRGB(FVector const& HCL)
{
	return HCLtoRGB(HCL.X, HCL.Y, HCL.Z);
}

FVector UKWColorSpace::RGBtoHCL(float R, float G, float B)
{
	FVector HCL;
	float H = 0;
	float U = FMath::Min3(R, G, B);
	float V = FMath::Max3(R, G, B);
	float Q = HCL_Gamma / HCL_Y0;
	HCL.Y = V - U;
	if (HCL.Y != 0)
	{
		H = FMath::Atan2(G - B, R - G) / PI;
		Q *= U / V;
	}
	Q = FMath::Exp(Q);
	HCL.X = FMath::Frac(H / 2 - FMath::Min(FMath::Frac(H), FMath::Frac(-H)) / 6);
	HCL.Y *= Q;
	HCL.Z = FMath::Lerp(-U, V, Q) / (HCL_MaxL * 2);
	return HCL;
}

FVector UKWColorSpace::RGBtoHCL(FVector const& RGB)
{
	return RGBtoHCL(RGB.X, RGB.Y, RGB.Z);
}

FVector UKWColorSpace::RGBtoHCL(FLinearColor const& RGB)
{
	return RGBtoHCL(RGB.R, RGB.G, RGB.B);
}

FVector UKWColorSpace::HSLtoIsoIluminatedRGB(float H, float S, float L)
{
	float R, G, B;

	float v;
	float midl;

	R = L;  // default to gray
	G = L;
	B = L;

	float m;
	float sv;
	int32 sextant;
	float fract, vsf, mid1, mid2;

	H -= floor(H);  //[0,1)
	H *= 6.0;
	sextant = (int32)H;
	fract = H - sextant;
	switch (sextant)
	{
		// ISO r = 216, 14, 14 = RGB 0/6, .878, .451
		case 0:  // r to y
			S *= (1.00 * fract + .878 * (1 - fract));
			midl = (.263 * fract + .451 * (1 - fract));
			break;
		// ISO y = 134,134,  0 = RGB 1/6, 1.00, .263
		case 1:
			midl = (.296 * fract + .263 * (1 - fract));
			break;
		// ISO g =   0,151,  0 = RGB 2/6, 1.00, .296
		case 2:
			midl = (.280 * fract + .296 * (1 - fract));
			break;
		// ISO c =   0,143,143 = RGB 3/6, 1.00, .280
		case 3:
			S *= (.977 * fract + 1.00 * (1 - fract));
			midl = (.655 * fract + .280 * (1 - fract));
			break;
		// ISO b =  81, 81,253 = RGB 4/6, .977, .655
		case 4:
			S *= (1.00 * fract + .977 * (1 - fract));
			midl = (.359 * fract + .655 * (1 - fract));
			break;
		// ISO m =   0,151,  0 = RGB 5/6, 1.00, .359
		case 5:
		default:
			S *= (.878 * fract + 1.00 * (1 - fract));
			midl = (.451 * fract + .359 * (1 - fract));
			break;
	}
	if (L <= .5)
		L *= midl * 2;
	else
		L = (L - .5) * 2 * (1.0 - midl) + midl;

	v = (L <= 0.5) ? (L * (1.0 + S)) : (L + S - L * S);
	if (v > 0)
	{
		m = L + L - v;
		sv = (v - m) / v;

		vsf = v * sv * fract;
		mid1 = m + vsf;
		mid2 = v - vsf;
		switch (sextant)
		{
			case 0:
				R = v;
				G = mid1;
				B = m;
				break;
			case 1:
				R = mid2;
				G = v;
				B = m;
				break;
			case 2:
				R = m;
				G = v;
				B = mid1;
				break;
			case 3:
				R = m;
				G = mid2;
				B = v;
				break;
			case 4:
				R = mid1;
				G = m;
				B = v;
				break;
			case 5:
			default:
				R = v;
				G = m;
				B = mid2;
				break;
		}
	}

	return FVector(R, G, B);
}

FVector UKWColorSpace::LerpHCL(FVector const& A, FVector const& B, float const Alpha)
{
	float AH = A.X;
	float AC = A.Y;
	float AL = A.Z;

	float BH = B.X - AH;
	float BC = B.Y - AC;
	float BL = B.Z - AL;

	if (FMath::IsNaN(BC))
	{
		BC = 0;

		if (FMath::IsNaN(AC))
		{
			AC = B.Y;
		}
	}

	if (FMath::IsNaN(BH))
	{
		BH = 0;

		if (FMath::IsNaN(AH))
		{
			AH = B.X;
		}
	}
	else if (BH > 0.5)
	{
		BH -= 1;
	}
	else if (BH < -0.5)
	{
		BH += 1;
	}

	return FVector(AH, AC, AL) + FVector(BH, BC, BL) * Alpha;
}
