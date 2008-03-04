/*
-----------------------------------------------------------------------------
This source file is part of OGRE
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2006 Torus Knot Software Ltd
Also see acknowledgements in Readme.html

This program is free software; you can redistribute it and/or modify it under
the terms of the GNU Lesser General Public License as published by the Free Software
Foundation; either version 2 of the License, or (at your option) any later
version.

This program is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License along with
this program; if not, write to the Free Software Foundation, Inc., 59 Temple
Place - Suite 330, Boston, MA 02111-1307, USA, or go to
http://www.gnu.org/copyleft/lesser.txt.

You may alternatively use this source under the terms of a specific version of
the OGRE Unrestricted License provided you have obtained such a license from
Torus Knot Software Ltd.
-----------------------------------------------------------------------------
*/
#include "OgreRoot.h"
#include "OgreHlslFixedFuncEmuShaderGenerator.h"

namespace Ogre 
{
	//---------------------------------------------------------------------
	HlslFixedFuncEmuShaderGenerator::HlslFixedFuncEmuShaderGenerator()
	{
		mLanguageName	= "hlsl";
		mName			= "hlsl";
		mVpTarget		= "vs_3_0";
		mFpTarget		= "ps_3_0";
	}
	//---------------------------------------------------------------------
	HlslFixedFuncEmuShaderGenerator::~HlslFixedFuncEmuShaderGenerator()
	{

	}
	//---------------------------------------------------------------------
	const String HlslFixedFuncEmuShaderGenerator::getShaderSource( const String & vertexProgramName,
		const String & fragmentProgramName, const VertexBufferDeclaration & vertexBufferDeclaration, 
		FixedFuncState & fixedFuncState )
	{
		bool bHasColor = vertexBufferDeclaration.hasColor();
		bool bHasTexcoord = vertexBufferDeclaration.hasTexcoord();

		String shaderSource = "";

		shaderSource = shaderSource + "struct VS_INPUT { ";

		uint8 semanticCount[100];
		ZeroMemory(semanticCount, sizeof(uint8) * 100);
		const VertexBufferElementList & vertexBufferElementList = vertexBufferDeclaration.getVertexBufferElementList();
		for (unsigned short i = 0 ; i < vertexBufferElementList.size() ; i++)
		{
			VertexElementSemantic semantic = vertexBufferElementList[i].getVertexElementSemantic();
			VertexElementType type = vertexBufferElementList[i].getVertexElementType();

			String thisElementSemanticCount = StringConverter::toString(semanticCount[semantic]);
			semanticCount[semantic]++;
			String parameterType = "";
			String parameterName = "";
			String parameterShaderTypeName = "";

			switch (type)
			{
			case VET_FLOAT1:
				parameterType = "float";
				break;
			case VET_FLOAT2:
				parameterType = "float2";
				break;
			case VET_FLOAT3:
				parameterType = "float3";
				break;
			case VET_FLOAT4:
				parameterType = "float4";
				break;
			case VET_COLOUR:
			case VET_COLOUR_ARGB:
			case VET_COLOUR_ABGR:
				parameterType = "unsigned int";		
				break;
			case VET_SHORT1:
				parameterType = "short";		
				break;
			case VET_SHORT2:
				parameterType = "short2";		
				break;
			case VET_SHORT3:
				parameterType = "short3";		
				break;
			case VET_SHORT4:
				parameterType = "short4";		
				break;
			case VET_UBYTE4:
				parameterType = "char4";
				break;

			}
			switch (semantic)
			{
			case VES_POSITION:
				parameterName = "Position";
				parameterShaderTypeName = "POSITION";
				//parameterType = "float4"; // position must be float4 (and not float3 like in the buffer)
				break;
			case VES_BLEND_WEIGHTS:
				parameterName = "BlendWeight";
				parameterShaderTypeName = "BLENDWEIGHT";
				break;
			case VES_BLEND_INDICES:
				parameterName = "BlendIndices";
				parameterShaderTypeName = "BLENDINDICES";
				break;
			case VES_NORMAL:
				parameterName = "Normal";
				parameterShaderTypeName = "NORMAL";
				break;
			case VES_DIFFUSE:
				parameterName = "DiffuseColor";
				parameterShaderTypeName = "COLOR";
				break;
			case VES_SPECULAR:
				parameterName = "SpecularColor";
				parameterShaderTypeName = "COLOR";
				thisElementSemanticCount = StringConverter::toString(semanticCount[VES_DIFFUSE]); // Diffuse is the "COLOR" count...
				semanticCount[VES_DIFFUSE]++;
				break;
			case VES_TEXTURE_COORDINATES:
				parameterName = "Texcoord";
				parameterShaderTypeName = "TEXCOORD";
				break;
			case VES_BINORMAL:
				parameterName = "Binormal";
				parameterShaderTypeName = "BINORMAL";
				break;
			case VES_TANGENT:
				parameterName = "Tangent";
				parameterShaderTypeName = "TANGENT";
				break;
			}



			shaderSource = shaderSource + parameterType + " " + parameterName + thisElementSemanticCount + " : " + parameterShaderTypeName + thisElementSemanticCount + ";\n";
		}

		shaderSource = shaderSource + " };";



		shaderSource = shaderSource + "float4x4  World;\n";
		shaderSource = shaderSource + "float4x4  View;\n";
		shaderSource = shaderSource + "float4x4  Projection;\n";

		switch (fixedFuncState.getGeneralFixedFuncState().getFogMode())
		{
		case FOG_NONE:
			break;
		case FOG_EXP:
		case FOG_EXP2:
			shaderSource = shaderSource + "float FogDensity;\n";
			break;
		case FOG_LINEAR:
			shaderSource = shaderSource + "float FogStart;\n";
			shaderSource = shaderSource + "float FogEnd;\n";
			break;
		}


		shaderSource = shaderSource + "struct VS_OUTPUT\n";
		shaderSource = shaderSource + "{\n";
		shaderSource = shaderSource + "float4 Pos : SV_POSITION;\n";
		if (bHasTexcoord)
		{
			shaderSource = shaderSource + "float2 tCord : TEXCOORD;\n";
		}
		if (bHasColor)
		{
			shaderSource = shaderSource + "float4 col : COLOR;\n";
		}

		if (fixedFuncState.getGeneralFixedFuncState().getFogMode() != FOG_NONE)
		{
			shaderSource = shaderSource + "float fogDist : FOGDISTANCE;\n"; 
		}

		shaderSource = shaderSource + "};\n";

		shaderSource = shaderSource + "VS_OUTPUT " + vertexProgramName + "( VS_INPUT input )\n";
		shaderSource = shaderSource + "{\n";
		shaderSource = shaderSource + "VS_OUTPUT output = (VS_OUTPUT)0;\n";	
		shaderSource = shaderSource + "float4 worldPos = mul( World, float4( input.Position0 , 1 ));\n";
		shaderSource = shaderSource + "float4 cameraPos = mul( View, worldPos );\n";
		shaderSource = shaderSource + "output.Pos = mul( Projection, cameraPos );\n";	


		if(bHasTexcoord)
		{
			shaderSource = shaderSource + "output.tCord = input.Texcoord0;\n";		
		}
		if (bHasColor)
		{
			shaderSource = shaderSource + "output.col.x = ((input.DiffuseColor0 >> 24) & 0xFF) / 255.0f;\n";
			shaderSource = shaderSource + "output.col.y = ((input.DiffuseColor0 >> 16) & 0xFF) / 255.0f;\n"; 
			shaderSource = shaderSource + "output.col.z = ((input.DiffuseColor0 >> 8) & 0xFF) / 255.0f;\n";
			shaderSource = shaderSource + "output.col.w = (input.DiffuseColor0 & 0xFF) / 255.0f;\n";
		}

		switch (fixedFuncState.getGeneralFixedFuncState().getFogMode())
		{
		case FOG_NONE:
			break;
		case FOG_EXP:
		case FOG_EXP2:
		case FOG_LINEAR:
			shaderSource = shaderSource + "output.fogDist = length(cameraPos.xyz);\n";
			break;
		}

		shaderSource = shaderSource + "return output;}\n";

		// here starts the fragment shader

		shaderSource = shaderSource + "float4x4  TextureMatrix;\n";
		shaderSource = shaderSource + "float  LightingEnabled;\n";
		shaderSource = shaderSource + "float4  FogColor;\n";



		if(bHasColor &bHasTexcoord)
		{
			shaderSource = shaderSource + "sampler tex0 : register(s0);\n";
			shaderSource = shaderSource + "float4 " + fragmentProgramName + "( VS_OUTPUT input ) : SV_Target\n";
			shaderSource = shaderSource + "{\n";
			shaderSource = shaderSource + "float4 texCordWithMatrix = float4(input.tCord.x, input.tCord.y, 0, 1);\n";
			shaderSource = shaderSource + "texCordWithMatrix = mul( texCordWithMatrix, TextureMatrix );\n";
			shaderSource = shaderSource + "float4 lightColor = max((float4(1.0,1.0,1.0,1.0) * (LightingEnabled)), input.col);\n";
			shaderSource = shaderSource + "float4 finalColor = tex2D(tex0,texCordWithMatrix.xy) * lightColor;\n";
		}
		else if(bHasTexcoord)
		{
			shaderSource = shaderSource + "sampler tex0 : register(s0);\n";
			shaderSource = shaderSource + "float4 " + fragmentProgramName + "( VS_OUTPUT input ) : SV_Target\n";
			shaderSource = shaderSource + "{\n";
			shaderSource = shaderSource + "float4 texCordWithMatrix = float4(input.tCord.x, input.tCord.y, 0, 1);\n";
			shaderSource = shaderSource + "texCordWithMatrix = mul( texCordWithMatrix, TextureMatrix );\n";
			shaderSource = shaderSource + "\n";
			shaderSource = shaderSource + "float4 finalColor = tex2D(tex0,texCordWithMatrix.xy);\n";
		}
		else if(bHasColor)
		{
			shaderSource = shaderSource + "float4 " + fragmentProgramName + "( VS_OUTPUT input ) : SV_Target\n";
			shaderSource = shaderSource + "{\n";
			shaderSource = shaderSource + "float4 finalColor = max((float4(1.0,1.0,1.0,1.0) * (LightingEnabled)), input.col);\n";
		}
		else 
		{
			shaderSource = shaderSource + "float4 " + fragmentProgramName + "( VS_OUTPUT input) : SV_Target\n";
			shaderSource = shaderSource + "{\nfloat4 finalColor = float4(1.0, 1.0, 1.0, 1.0);\n";
		}

		switch (fixedFuncState.getGeneralFixedFuncState().getFogMode())
		{
		case FOG_NONE:
			break;
		case FOG_EXP:
			shaderSource = shaderSource + "#define E 2.71828\n";
			shaderSource = shaderSource + "input.fogDist = 1.0 / pow( E, input.fogDist*FogDensity );\n";
			shaderSource = shaderSource + "input.fogDist = clamp( input.fogDist, 0, 1 );\n";
			break;
		case FOG_EXP2:
			shaderSource = shaderSource + "#define E 2.71828\n";
			shaderSource = shaderSource + "input.fogDist = 1.0 / pow( E, input.fogDist*input.fogDist*FogDensity*FogDensity );\n";
			shaderSource = shaderSource + "input.fogDist = clamp( input.fogDist, 0, 1 );\n";
			break;
		case FOG_LINEAR:
			shaderSource = shaderSource + "input.fogDist = (FogEnd - input.fogDist)/(FogEnd - FogStart);\n";
			shaderSource = shaderSource + "input.fogDist = clamp( input.fogDist, 0, 1 );\n";
			break;
		}

		if (fixedFuncState.getGeneralFixedFuncState().getFogMode() != FOG_NONE)
		{

			shaderSource = shaderSource + "finalColor.xyz = input.fogDist * finalColor.xyz + (1.0 - input.fogDist)*FogColor.xyz;\n";

		}

		shaderSource = shaderSource + "return finalColor;\n}";
		return shaderSource;
	}
	//---------------------------------------------------------------------
	//---------------------------------------------------------------------
	//---------------------------------------------------------------------
	Hlsl4FixedFuncEmuShaderGenerator::Hlsl4FixedFuncEmuShaderGenerator()
	{
		mLanguageName	= "hlsl";
		mName			= "hlsl4";
		mVpTarget		= "vs_4_0";
		mFpTarget		= "ps_4_0";
	}
	//---------------------------------------------------------------------
	Hlsl4FixedFuncEmuShaderGenerator::~Hlsl4FixedFuncEmuShaderGenerator()
	{

	}
	//---------------------------------------------------------------------
	const String Hlsl4FixedFuncEmuShaderGenerator::getShaderSource( const String & vertexProgramName, const String & fragmentProgramName, const VertexBufferDeclaration & vertexBufferDeclaration, FixedFuncState & fixedFuncState )
	{
		return HlslFixedFuncEmuShaderGenerator::getShaderSource(vertexProgramName, fragmentProgramName, vertexBufferDeclaration, fixedFuncState);
	}
	//---------------------------------------------------------------------
}
