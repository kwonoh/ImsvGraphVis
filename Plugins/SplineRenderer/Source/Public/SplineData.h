// Copyright 2017 Oh-Hyun Kwon. All Rights Reserved.

/*

Degree						: 3
Num Original Control Points : 5
Num Segments				: 6 = 5 + 3 (Degree) - 2 (S0 and S7)
Num Clamped Control Points	: 9 = Num Segments + Degree

				 P0 P1 P2 P3 P4				(Original Control Points)
P0= S0	P0 P0 P0 P0
	S1	   P0 P0 P0 P1
	S2		  P0 P0 P1 P2
	S3			 P0 P1 P2 P3
	S4				P1 P2 P3 P4
	S5				   P2 P3 P4 P4
	S6					  P3 P4 P4 P4
P4=	S7						 P4 P4 P4 P4
		   Q0 Q1 Q2 Q3 Q4 Q5 Q6 Q7 Q8		(Clamped Control Points)

*/

#pragma once

#include "Core.h"

struct FSplineControlPointData
{
	FVector Point;
	FLinearColor Color;
	float Knot;
};

struct FSplineData
{
	uint32 BeginControlPointIdx;  // LastControlPointIdx = BeginControlPointIdx + NumSegments +
								  // Degree - 1
	uint32 NumSegments;
	uint32 NumSamples;
};
