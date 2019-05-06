<Material name="paper_office" version="1.0">
    <MetaData >
        <Property formalName="Baked Shadow Map" name="uBakedShadowTexture" description="Baked shadow texture for the material" type="Texture" filter="linear" minfilter="linearMipmapLinear" clamp="repeat" usage="shadow" default="./maps/materials/shadow.png" category="Material"/>
        <Property formalName="Shadow Mapping" name="uShadowMappingEnabled" description="Enable shadow mapping" type="Boolean" default="False" category="Material"/>
        <Property formalName="Translucency Falloff" name="uTranslucentFalloff" description="Falloff of the translucency of the\nmaterial" type="Float" default="0.0" category="Material"/>
        <Property formalName="Diffuse Light Bend" name="uDiffuseLightWrap" description="Diffuse light bend of the material" type="Float" min="0.0" max="1.0" default="0.0" category="Material"/>
        <Property formalName="Transparency" name="uOpacity" description="Transparency of the material" type="Float" min="0.0" max="100.0" default="100.0" category="Material"/>
        <Property formalName="Transmission Weight" name="transmission_weight" type="Float" default="0.200000" description="Value determining transmission" category="Material"/>
        <Property formalName="Transmission Map" name="transmission_texture" description="Transmission texture of the material" type="Texture" filter="linear" minfilter="linearMipmapLinear" clamp="repeat" usage="transmission" default="./maps/materials/paper_trans.png" category="Textures"/>
        <Property formalName="Tiling" name="texture_tiling" type="Float2" default="1 1" description="Scaling of the textures" category="Textures"/>
        <Property formalName="Paper Color" name="paper_color" type="Color" default="0.531 0.531 0.531" description="Color of the paper" category="Material"/>
        <Property formalName="Reflection Weight" name="reflection_weight" type="Float" default="0.800000" description="Value determining reflection" category="Material"/>
        <Property formalName="Bump Map" name="bump_texture" description="Bump texture of the material" type="Texture" filter="linear" minfilter="linearMipmapLinear" clamp="repeat" usage="bump" default="./maps/materials/paper_diffuse.png" category="Textures"/>
        <Property formalName="Bump Amount" name="bump_amount" type="Float" default="0.500000" description="Value determining the bumpiness" category="Material"/>
    </MetaData>
    <Shaders type="GLSL" version="330">
    <Shader>
    <Shared>    </Shared>
<VertexShader>
        </VertexShader>
        <FragmentShader>

// add enum defines
#define mono_alpha 0
#define mono_average 1
#define mono_luminance 2
#define mono_maximum 3
#define wrap_clamp 0
#define wrap_repeat 1
#define wrap_mirrored_repeat 2
#define gamma_default 0
#define gamma_linear 1
#define gamma_srgb 2
#define color_layer_blend 0
#define color_layer_add 1
#define color_layer_multiply 2
#define color_layer_screen 3
#define color_layer_overlay 4
#define color_layer_brightness 5
#define color_layer_color 6

#define QT3DS_ENABLE_UV0 1
#define QT3DS_ENABLE_WORLD_POSITION 1
#define QT3DS_ENABLE_TEXTAN 1
#define QT3DS_ENABLE_BINORMAL 1

#include "vertexFragmentBase.glsllib"

// set shader output
out vec4 fragColor;

// add structure defines
struct texture_coordinate_info
{
  vec3 position;
  vec3 tangent_u;
  vec3 tangent_v;
};


struct bsdf_component
{
  float weight;
  vec4 component;
};


struct color_layer
{
  vec3 layer_color;
  float weight;
  int mode;
};


struct texture_return
{
  vec3 tint;
  float mono;
};


struct layer_result
{
  vec4 base;
  vec4 layer;
  mat3 tanFrame;
};


// temporary declarations
texture_coordinate_info tmp1;
vec3 ftmp0;
 vec4 tmpShadowTerm;

layer_result layers[1];

#include "SSAOCustomMaterial.glsllib"
#include "sampleLight.glsllib"
#include "sampleProbe.glsllib"
#include "sampleArea.glsllib"
#include "luminance.glsllib"
#include "monoChannel.glsllib"
#include "fileBumpTexture.glsllib"
#include "transformCoordinate.glsllib"
#include "rotationTranslationScale.glsllib"
#include "textureCoordinateInfo.glsllib"
#include "normalizedMix.glsllib"
#include "evalBakedShadowMap.glsllib"
#include "diffuseTransmissionBSDF.glsllib"
#include "fileTexture.glsllib"
#include "blendColorLayers.glsllib"
#include "diffuseReflectionBSDF.glsllib"

bool evalTwoSided()
{
  return( false );
}

vec3 computeFrontMaterialEmissive()
{
  return( vec3( 0, 0, 0 ) );
}

void computeFrontLayerColor( in vec3 normal, in vec3 lightDir, in vec3 viewDir, in vec3 lightDiffuse, in vec3 lightSpecular, in float materialIOR, float aoFactor )
{
#if QT3DS_ENABLE_CG_LIGHTING
  layers[0].base += tmpShadowTerm * normalizedMix( bsdf_component[2]( bsdf_component(transmission_weight, diffuseTransmissionBSDF( -normal, lightDir, viewDir, lightDiffuse, vec4( ftmp0, 1.0), uTranslucentFalloff, uDiffuseLightWrap )) ,bsdf_component(reflection_weight, diffuseReflectionBSDF( normal, lightDir, viewDir, lightDiffuse, 0.000000 )) ) );

#endif
}

void computeFrontAreaColor( in int lightIdx, in vec4 lightDiffuse, in vec4 lightSpecular )
{
#if QT3DS_ENABLE_CG_LIGHTING
  layers[0].base += tmpShadowTerm * normalizedMix( bsdf_component[2]( bsdf_component(transmission_weight, lightDiffuse * sampleAreaDiffuseTransmissive( layers[0].tanFrame, varWorldPos, lightIdx, vec4( ftmp0, 1.0), uTranslucentFalloff, uDiffuseLightWrap )) ,bsdf_component(reflection_weight, lightDiffuse * sampleAreaDiffuse( layers[0].tanFrame, varWorldPos, lightIdx )) ) );

#endif
}

void computeFrontLayerEnvironment( in vec3 normal, in vec3 viewDir, float aoFactor )
{
#if !QT3DS_ENABLE_LIGHT_PROBE
  layers[0].base += tmpShadowTerm * diffuseReflectionBSDFEnvironment( normal, 0.000000 ) * aoFactor;

#else
  layers[0].base += tmpShadowTerm * sampleDiffuse( layers[0].tanFrame ) * aoFactor;

#endif
}

void computeFrontLayerRnmColor( in vec3 normal, in vec3 rnmX, in vec3 rnmY, in vec3 rnmZ )
{
#if QT3DS_ENABLE_RNM
  layers[0].base += tmpShadowTerm * diffuseRNM( normal, rnmX, rnmY, rnmZ );

#endif
}

vec3 computeBackMaterialEmissive()
{
  return( vec3(0, 0, 0) );
}

void computeBackLayerColor( in vec3 normal, in vec3 lightDir, in vec3 viewDir, in vec3 lightDiffuse, in vec3 lightSpecular, in float materialIOR, float aoFactor )
{
#if QT3DS_ENABLE_CG_LIGHTING
  layers[0].base += vec4( 0.0, 0.0, 0.0, 1.0 );
  layers[0].layer += vec4( 0.0, 0.0, 0.0, 1.0 );
#endif
}

void computeBackAreaColor( in int lightIdx, in vec4 lightDiffuse, in vec4 lightSpecular )
{
#if QT3DS_ENABLE_CG_LIGHTING
  layers[0].base += vec4( 0.0, 0.0, 0.0, 1.0 );
  layers[0].layer += vec4( 0.0, 0.0, 0.0, 1.0 );
#endif
}

void computeBackLayerEnvironment( in vec3 normal, in vec3 viewDir, float aoFactor )
{
#if !QT3DS_ENABLE_LIGHT_PROBE
  layers[0].base += vec4( 0.0, 0.0, 0.0, 1.0 );
  layers[0].layer += vec4( 0.0, 0.0, 0.0, 1.0 );
#else
  layers[0].base += vec4( 0.0, 0.0, 0.0, 1.0 );
  layers[0].layer += vec4( 0.0, 0.0, 0.0, 1.0 );
#endif
}

void computeBackLayerRnmColor( in vec3 normal, in vec3 rnmX, in vec3 rnmY, in vec3 rnmZ )
{
#if QT3DS_ENABLE_RNM
  layers[0].base += vec4( 0.0, 0.0, 0.0, 1.0 );
  layers[0].layer += vec4( 0.0, 0.0, 0.0, 1.0 );
#endif
}

float computeIOR()
{
  return( false ? 1.0 : luminance( vec3( 1, 1, 1 ) ) );
}

float evalCutout()
{
  return( 1.000000 );
}

vec3 computeNormal()
{
  return( fileBumpTexture(bump_texture, bump_amount, mono_average, tmp1, vec2( 0.000000, 1.000000 ), vec2( 0.000000, 1.000000 ), wrap_repeat, wrap_repeat, normal ) );
}

void computeTemporaries()
{
     tmp1 = transformCoordinate( rotationTranslationScale( vec3( 0.000000, 0.000000, 0.000000 ), vec3( 0.000000, 0.000000, 0.000000 ), vec3( texture_tiling[0], texture_tiling[1], 1.000000 ) ), textureCoordinateInfo( texCoord0, tangent, binormal ) );
     ftmp0 = blendColorLayers( color_layer[1]( color_layer(blendColorLayers( color_layer[1]( color_layer(fileTexture(transmission_texture, vec3( 0, 0, 0 ), vec3( 1, 1, 1 ), mono_luminance, tmp1, vec2( 0.000000, 1.000000 ), vec2( 0.000000, 1.000000 ), wrap_repeat, wrap_repeat, gamma_default ).tint, 0.700000, color_layer_blend ) ), paper_color.rgb, mono_average ).tint, 1.000000, color_layer_multiply ) ), vec3( 1, 1, 1 ), mono_average ).tint;
     tmpShadowTerm = evalBakedShadowMap( texCoord0 );
}

vec4 computeLayerWeights( in float alpha )
{
  vec4 color;
  color = layers[0].base * vec4( paper_color.rgb, 1.0);
  return color;
}


void initializeLayerVariables(void)
{
  // clear layers
  layers[0].base = vec4(0.0, 0.0, 0.0, 1.0);
  layers[0].layer = vec4(0.0, 0.0, 0.0, 1.0);
  layers[0].tanFrame = orthoNormalize( tangentFrame( normal, varWorldPos ) );
}

vec4 computeOpacity(in vec4 color)
{
  vec4 rgba = color;
  rgba.a = uOpacity * 0.01;
  return rgba;
}

        </FragmentShader>
    </Shader>
    </Shaders>
<Passes >
        <ShaderKey value="257"/>
        <LayerKey count="1"/>
    <Pass >
        <Blending source="SrcAlpha" dest="OneMinusSrcAlpha"/>
    </Pass>
</Passes>
</Material>
