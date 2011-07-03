//////////////////////////////////////////////////////////////////////////////
//
// Encoding.fx
//
// Utility effect file which is not compiled.
// This file contains various color space and data packing conversions
// for use in packing values into color render targets 
//
///////////////////////////////////////////////////////////////////////////////

const static float3x3 LogLuvM = float3x3( 0.2209, 0.3390, 0.4184,
										  0.1138, 0.6780, 0.7319,
										  0.0102, 0.1130, 0.2969);

const static float3x3 LogLuvInverseM = float3x3( 6.0014, -2.7008, -1.7996,
												 -1.332, 3.1029, -5.7721,
												 0.3008, -1.0882, 5.6268);



//////////////////////////////////////////////////////////////////////////////
// Color Space Conversions 
//////////////////////////////////////////////////////////////////////////////

// Encode 32F RGB triplet into RGBE8 integer format
float4 encodeRGBE8(float3 rgb)
{
	float4 encoded;
	
	float fmax = max(max(rgb.x, rgb.y), rgb.z);
	float fexp = ceil(log2(fmax));
	
	encoded.xyz = rgb / exp2(fexp);
	encoded.w = (fexp + 128) / 255;
	
	return encoded;
}

// Decode from RGBE8 integer format into 32F RGB triplet
float3 decodeRGBE8(float4 rgba)
{
	float3 decoded;
	
	float fexp = rgba.w * 255 - 128;
	decoded = rgba.xyz * exp2(fexp);
	
	return decoded;
}


// Encode 32F luminance value into RE8 integer format //
float4 encodeRE8(float lum)
{
	float4 encoded = float4(0, 0, 0, 0);
	float fexp = ceil(log2(lum));
	encoded.x = lum / exp2(fexp);
	encoded.w = (fexp + 128) / 255;
	return encoded;
}


// Decode RE8 luminance value back to a 32F luminance //
float decodeRE8(float4 lum)
{
	float decoded;
	float fexp = lum.w * 255 - 128;
	decoded = lum.r * exp2(fexp);
	return decoded;
}



float4 encodeLogLuv(in float3 rgb)
{
	float4 res;
	float3 Xp_Y_XYZp = mul(rgb, LogLuvM);
	Xp_Y_XYZp = max(Xp_Y_XYZp, float3(1e-6, 1e-6, 1e-6));
	res.xy = Xp_Y_XYZp.xy / Xp_Y_XYZp.z;
	float Le = 2 * log2(Xp_Y_XYZp.y) + 127;
	res.w = frac(Le);
	res.z = (Le - (floor(res.w * 255.0F)) / 255.0F) / 255.0F;
	return res;
}

float3 decodeLogLuv(in float4 logLuv)
{
	float Le = logLuv.z * 255 + logLuv.w;
	float3 Xp_Y_XYZp;
	Xp_Y_XYZp.y = exp2((Le - 127) / 2.0);
	Xp_Y_XYZp.z = Xp_Y_XYZp.y / logLuv.y;
	Xp_Y_XYZp.x = logLuv.x * Xp_Y_XYZp.z;
	
	return max( mul( Xp_Y_XYZp, LogLuvInverseM ), 0 );
}



float3 convertToGamma_1(in float3 color)
{
	float3 ret = pow((abs(color) + 0.055) / 1.055, 2.4);
	return ret;
}

float3 convertToGamma_2_2(in float3 color)
{
	float3 ret = pow(1.055 * abs(color), 1.0 / 2.4) - 0.055;
	return ret;
}

///////////////////////////////////////////////////////////////////////////////////////////
// Normal/Depth packing conversions 
///////////////////////////////////////////////////////////////////////////////////////////


// Pack normal and depth into RGBA8 format
// R- norm.x
// G- norm.y
// B(low)- norm.z
// B(high)- depth
// A- depth
float4 encodeDepthNormal(float depth, float3 normal)
{
	float4 output;
	
	// Pack high depth (0-127)
	depth = saturate(depth);
	output.z = floor(depth * 127);
	
	// Pack low depth (0-1)
	output.w = frac(depth * 127);
	
	// Pack normal x/y
	output.xy = normal.xy * 0.5 + 0.5;
	
	// Encode sign of z0 into upper portion of high z
	if(normal.z < 0)
		output.z += 128;
	
	// 0-1 range
	output.z /= 255;
	
	return output;
}


// Decode normal and depth info from RGBA8 encoded format to float3 and float
void decodeDepthNormal(float4 input, out float depth, out float3 normal)
{
	// Expand normal
	normal.xy = input.xy * 2 - 1;
	
	// Compute unsigned z norm
	normal.z = 1.0 - sqrt(dot(normal.xy, normal.xy));
	float hiDepth = input.z * 255;
	
	// Get sign of z norm 
	if(hiDepth >= 128)
	{
		normal.z = -normal.z;
		hiDepth -= 128;
	}
	
	depth = (hiDepth + input.w) / 127.0;
}


float4 noshader() : COLOR0
{
	return 0;
}

technique notech
{
	pass Pass_0
	{
		VertexShader = NULL;
		PixelShader = compile ps_2_0 noshader();
	}
}