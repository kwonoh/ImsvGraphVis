// Copyright 2017 Oh-Hyun Kwon. All Rights Reserved.

#include "SplineRendererPrivatePCH.h"

#include "RHIStaticStates.h"
#include "ShaderParameterUtils.h"
#include "ShaderParameters.h"
#include "UniformBuffer.h"

IMPLEMENT_GLOBAL_SHADER_PARAMETER_STRUCT(FSplineComputeShaderUniformParameters,
									"SplineComputeShaderUniformParameters");

FBaseSplineComputeShader::FBaseSplineComputeShader(
	ShaderMetaType::CompiledShaderInitializerType const& Initializer)
	: FGlobalShader(Initializer)
{
	InSplineControlPointData.Bind(Initializer.ParameterMap, TEXT("InSplineControlPointData"));
	InSplineSegmentData.Bind(Initializer.ParameterMap, TEXT("InSplineSegmentData"));
	InSplineData.Bind(Initializer.ParameterMap, TEXT("InSplineData"));
	OutMeshVertexData.Bind(Initializer.ParameterMap, TEXT("OutMeshVertexData"));
}

void FBaseSplineComputeShader::SetUniformBuffers(
	FRHICommandList& RHICmdList, FSplineComputeShaderUniformParameters& UniformBufferParams)
{
	FComputeShaderRHIParamRef ComputeShaderRHI = GetComputeShader();

	FSplineComputeShaderUniformParametersRef UniformBuffer =
		FSplineComputeShaderUniformParametersRef::CreateUniformBufferImmediate(
			UniformBufferParams, UniformBuffer_SingleDraw);

	SetUniformBufferParameter(RHICmdList, ComputeShaderRHI,
							  GetUniformBufferParameter<FSplineComputeShaderUniformParameters>(),
							  UniformBuffer);
}

void FBaseSplineComputeShader::SetBuffers(FRHICommandList& RHICmdList,
										  FShaderResourceViewRHIRef InSplineControlPointBufferSRV,
										  FShaderResourceViewRHIRef InSplineSegmentBufferSRV,
										  FShaderResourceViewRHIRef InSplineBufferSRV,
										  FUnorderedAccessViewRHIRef OutMeshVertexBufferUAV)
{
	FComputeShaderRHIParamRef ComputeShaderRHI = GetComputeShader();

	if (InSplineControlPointData.IsBound())
		RHICmdList.SetShaderResourceViewParameter(ComputeShaderRHI,							//
												  InSplineControlPointData.GetBaseIndex(),  //
												  InSplineControlPointBufferSRV);

	if (InSplineSegmentData.IsBound())
		RHICmdList.SetShaderResourceViewParameter(ComputeShaderRHI,					   //
												  InSplineSegmentData.GetBaseIndex(),  //
												  InSplineSegmentBufferSRV);
	if (InSplineData.IsBound())
		RHICmdList.SetShaderResourceViewParameter(ComputeShaderRHI,				//
												  InSplineData.GetBaseIndex(),  //
												  InSplineBufferSRV);

	if (OutMeshVertexData.IsBound())
		RHICmdList.SetUAVParameter(ComputeShaderRHI, OutMeshVertexData.GetBaseIndex(),
								   OutMeshVertexBufferUAV);
}

void FBaseSplineComputeShader::UnbindBuffers(FRHICommandList& RHICmdList)
{
	FComputeShaderRHIParamRef ComputeShaderRHI = GetComputeShader();

	if (InSplineControlPointData.IsBound())
		RHICmdList.SetShaderResourceViewParameter(
			ComputeShaderRHI, InSplineControlPointData.GetBaseIndex(), FShaderResourceViewRHIRef());

	if (InSplineSegmentData.IsBound())
		RHICmdList.SetShaderResourceViewParameter(
			ComputeShaderRHI, InSplineSegmentData.GetBaseIndex(), FShaderResourceViewRHIRef());

	if (InSplineData.IsBound())
		RHICmdList.SetShaderResourceViewParameter(ComputeShaderRHI, InSplineData.GetBaseIndex(),
												  FShaderResourceViewRHIRef());

	if (OutMeshVertexData.IsBound())
		RHICmdList.SetUAVParameter(ComputeShaderRHI, OutMeshVertexData.GetBaseIndex(),
								   FUnorderedAccessViewRHIRef());
}

FSplineComputeShader_Sphere::FSplineComputeShader_Sphere(
	ShaderMetaType::CompiledShaderInitializerType const& Initializer)
	: FBaseSplineComputeShader(Initializer)
{
}

IMPLEMENT_SHADER_TYPE(, FSplineComputeShader_Sphere,
					  TEXT("/Plugin/SplineRenderer/Private/SplineComputeShader.usf"),
					  TEXT("MainCS"), SF_Compute);
