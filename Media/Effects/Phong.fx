
float4x4		g_mMVP;
float3			g_lightPos;
float3			g_camPos;

sampler			g_diffuseSampler : register(s0);

struct Vertex3DS
{
	float4 pos		: POSITION;
	float3 norm		: NORMAL;
	float3 tan		: TANGENT;
	float2 tex		: TEXCOORD0;
};


struct Transformed3DS
{
	float4 pos		: POSITION;
	float3 norm		: TEXCOORD0;
	float3 tan		: TEXCOORD1;
	float2 tex		: TEXCOORD2;
	float3 objPos		: TEXCOORD3;
};



Transformed3DS PhongMainVS( Vertex3DS vert )
{
	Transformed3DS output;
	
	output.objPos = vert.pos.xyz;
	output.pos = mul( g_mMVP, vert.pos );
	output.norm = vert.norm;
	output.tan = vert.tan;
	output.tex = vert.tex;
	
	return output;
}


float4 PhongMainPS( Transformed3DS input ) : COLOR0
{
	float3 P = input.objPos;
	float3 N = normalize( input.norm );
	float3 L = normalize( g_lightPos - P );
	float3 V = L;
	float3 H = L + V;
	float3 lightColor = float3( 0.8f, 1.0f, 1.0f );
	float3 Kd = tex2D(g_diffuseSampler, input.tex).rgb;
	
	float diffuse = max( dot( N, L ), 0 );
	float specular = pow(max(dot(N, H), 0), 2.0);
	float3 diffuseLight = diffuse * lightColor * Kd;
	float3 specularLight = specular * lightColor * Kd;
	
	return float4( diffuseLight + specularLight, 1.0 );
}


technique Phong
{
	pass Pass_0
	{
		VertexShader = compile vs_2_a PhongMainVS();
		PixelShader = compile ps_2_a PhongMainPS();
	}
}