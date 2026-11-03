//+ rouz edit (ChatGPT)
#include "ParticleMgr.h"
#include "WorkBuffer.h"
#include "FileMgr.h"
#include "config.h"
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#define Min(a,b) ((a)<(b)?(a):(b))
#define SQR(x) ((x)*(x))
#ifndef MASTER
void re3_assert(const char *,const char *,unsigned int,const char *);
#define ASSERT(expression) ((void)((!!(expression)) || (re3_assert(#expression,__FILE__,__LINE__,__func__),0)))
#else
#define ASSERT(expression) (expression)
#endif
ParticleSystemMgr mod_ParticleSystemManager;
const char *ParticleFilename="PARTICLE.CFG";
enum
{
	CFG_PARAM_PARTICLE_TYPE_NAME = 0,
	CFG_PARAM_RENDER_COLOURING_R,
	CFG_PARAM_RENDER_COLOURING_G,
	CFG_PARAM_RENDER_COLOURING_B,
	CFG_PARAM_INITIAL_COLOR_VARIATION,
	CFG_PARAM_FADE_DESTINATION_COLOR_R,
	CFG_PARAM_FADE_DESTINATION_COLOR_G,
	CFG_PARAM_FADE_DESTINATION_COLOR_B,
	CFG_PARAM_COLOR_FADE_TIME,
	CFG_PARAM_DEFAULT_INITIAL_RADIUS,
	CFG_PARAM_EXPANSION_RATE,
	CFG_PARAM_INITIAL_INTENSITY,
	CFG_PARAM_FADE_TIME,
	CFG_PARAM_FADE_AMOUNT,
	CFG_PARAM_INITIAL_ALPHA_INTENSITY,
	CFG_PARAM_FADE_ALPHA_TIME,
	CFG_PARAM_FADE_ALPHA_AMOUNT,
	CFG_PARAM_INITIAL_ANGLE,
	CFG_PARAM_CHANGE_TIME,
	CFG_PARAM_ANGLE_CHANGE_AMOUNT,
	CFG_PARAM_INITIAL_Z_RADIUS,
	CFG_PARAM_Z_RADIUS_CHANGE_TIME,
	CFG_PARAM_Z_RADIUS_CHANGE_AMOUNT,
	CFG_PARAM_ANIMATION_SPEED,
	CFG_PARAM_START_ANIMATION_FRAME,
	CFG_PARAM_FINAL_ANIMATION_FRAME,
	CFG_PARAM_ROTATION_SPEED,
	CFG_PARAM_GRAVITATIONAL_ACCELERATION,
	CFG_PARAM_FRICTION_DECCELERATION,
	CFG_PARAM_LIFE_SPAN,
	CFG_PARAM_POSITION_RANDOM_ERROR,
	CFG_PARAM_VELOCITY_RANDOM_ERROR,
	CFG_PARAM_EXPANSION_RATE_ERROR,
	CFG_PARAM_ROTATION_RATE_ERROR,
	CFG_PARAM_LIFE_SPAN_ERROR_SHAPE,
	CFG_PARAM_TRAIL_LENGTH_MULTIPLIER,
	
	CFG_PARAM_STRETCH_VALUE_X,
	CFG_PARAM_STRETCH_VALUE_Y,
	CFG_PARAM_WIND_FACTOR,
	
	CFG_PARAM_PARTICLE_CREATE_RANGE,
	CFG_PARAM_FLAGS,

	MAX_CFG_PARAMS,
	CFG_PARAM_FIRST = CFG_PARAM_PARTICLE_TYPE_NAME,
	CFG_PARAM_LAST = CFG_PARAM_FLAGS
};

void ParticleMgr_Initialise(void)
{
	// Preserve the original parser branch and token traversal
    // Load configuration before clearing all original live particle-list pointers
	ParticleMgr_LoadParticleData();
	
	for ( int32_t i = 0; i < MAX_PARTICLES; i++ )
		mod_ParticleSystemManager.m_aParticles[i].m_pParticles = NULL;
}

void ParticleMgr_LoadParticleData(void)
{
	// Preserve the original parser branch and token traversal
    // Read the original shared buffer and preserve every token conversion and retained field
	FileMgr_SetDir("DATA");
	FileMgr_LoadFile(ParticleFilename, work_buff, WORK_BUFFER_SIZE, "r");
	FileMgr_SetDir("");
	
	tParticleSystemData *entry = NULL;
	int32_t type = PARTICLE_FIRST;
	
	char *lineStart = (char *)work_buff;
	char *lineEnd = lineStart + 1;
	
	char line[500];
	char delims[4];

	while ( 1 )
	{
		// Preserve the original parser branch and token traversal
		ASSERT(lineStart != NULL);
		ASSERT(lineEnd != NULL);
		
		while ( *lineEnd != '\n' )
			++lineEnd;
		
		int32_t lineLength = lineEnd - lineStart;
		
		ASSERT(lineLength < 500);

		strncpy(line, lineStart, lineLength);
		
		line[lineLength] = '\0';
		
		if ( !strcmp(line, ";the end") )
			break;
		
		if ( *line != ';' )
		{
			// Preserve the original parser branch and token traversal
			int32_t param = CFG_PARAM_FIRST;

			strcpy(delims, " \t");
			
			char *value = strtok(line, delims);
			
			ASSERT(value != NULL);
			
			do
			{	
				// Preserve the original parser branch and token traversal
				switch ( param )
				{
					// Preserve the original parser branch and token traversal
					case CFG_PARAM_PARTICLE_TYPE_NAME:
						ASSERT(type < MAX_PARTICLES);
						entry = &mod_ParticleSystemManager.m_aParticles[type];
						ASSERT(entry != NULL);
						entry->m_Type = (enum tParticleType)type++;
						strcpy(entry->m_aName, value);
						break;

					case CFG_PARAM_RENDER_COLOURING_R:
						entry->m_RenderColouring.red = atoi(value);
						break;

					case CFG_PARAM_RENDER_COLOURING_G:
						entry->m_RenderColouring.green = atoi(value);
						break;

					case CFG_PARAM_RENDER_COLOURING_B:
						entry->m_RenderColouring.blue = atoi(value);
						break;

					case CFG_PARAM_INITIAL_COLOR_VARIATION:
						entry->m_InitialColorVariation = Min(atoi(value), 100);
						break;

					case CFG_PARAM_FADE_DESTINATION_COLOR_R:
						entry->m_FadeDestinationColor.red = atoi(value);
						break;

					case CFG_PARAM_FADE_DESTINATION_COLOR_G:
						entry->m_FadeDestinationColor.green = atoi(value);
						break;

					case CFG_PARAM_FADE_DESTINATION_COLOR_B:
						entry->m_FadeDestinationColor.blue = atoi(value);
						break;

					case CFG_PARAM_COLOR_FADE_TIME:
						entry->m_ColorFadeTime = atoi(value);
						break;

					case CFG_PARAM_DEFAULT_INITIAL_RADIUS:
						entry->m_fDefaultInitialRadius = atof(value);
						break;

					case CFG_PARAM_EXPANSION_RATE:
						entry->m_fExpansionRate = atof(value);
						break;

					case CFG_PARAM_INITIAL_INTENSITY:
						entry->m_nFadeToBlackInitialIntensity = atoi(value);
						break;

					case CFG_PARAM_FADE_TIME:
						entry->m_nFadeToBlackTime = atoi(value);
						break;

					case CFG_PARAM_FADE_AMOUNT:
						entry->m_nFadeToBlackAmount = atoi(value);
						break;

					case CFG_PARAM_INITIAL_ALPHA_INTENSITY:
						entry->m_nFadeAlphaInitialIntensity = atoi(value);
						break;

					case CFG_PARAM_FADE_ALPHA_TIME:
						entry->m_nFadeAlphaTime = atoi(value);
						break;

					case CFG_PARAM_FADE_ALPHA_AMOUNT:
						entry->m_nFadeAlphaAmount = atoi(value);
						break;

					case CFG_PARAM_INITIAL_ANGLE:
						entry->m_nZRotationInitialAngle = atoi(value);
						break;

					case CFG_PARAM_CHANGE_TIME:
						entry->m_nZRotationChangeTime = atoi(value);
						break;

					case CFG_PARAM_ANGLE_CHANGE_AMOUNT:
						entry->m_nZRotationAngleChangeAmount = atoi(value);
						break;

					case CFG_PARAM_INITIAL_Z_RADIUS:
						entry->m_fInitialZRadius = atof(value);
						break;

					case CFG_PARAM_Z_RADIUS_CHANGE_TIME:
						entry->m_nZRadiusChangeTime = atoi(value);
						break;

					case CFG_PARAM_Z_RADIUS_CHANGE_AMOUNT:
						entry->m_fZRadiusChangeAmount = atof(value);
						break;

					case CFG_PARAM_ANIMATION_SPEED:
						entry->m_nAnimationSpeed = atoi(value);
						break;

					case CFG_PARAM_START_ANIMATION_FRAME:
						entry->m_nStartAnimationFrame = atoi(value);
						break;

					case CFG_PARAM_FINAL_ANIMATION_FRAME:
						entry->m_nFinalAnimationFrame = atoi(value);
						break;

					case CFG_PARAM_ROTATION_SPEED:
						entry->m_nRotationSpeed = atoi(value);
						break;

					case CFG_PARAM_GRAVITATIONAL_ACCELERATION:
						entry->m_fGravitationalAcceleration = atof(value);
						break;

					case CFG_PARAM_FRICTION_DECCELERATION:
						entry->m_nFrictionDecceleration = atoi(value);
						break;

					case CFG_PARAM_LIFE_SPAN:
						entry->m_nLifeSpan = atoi(value);
						break;

					case CFG_PARAM_POSITION_RANDOM_ERROR:
						entry->m_fPositionRandomError = atof(value);
						break;

					case CFG_PARAM_VELOCITY_RANDOM_ERROR:
						entry->m_fVelocityRandomError = atof(value);
						break;

					case CFG_PARAM_EXPANSION_RATE_ERROR:
						entry->m_fExpansionRateError = atof(value);
						break;

					case CFG_PARAM_ROTATION_RATE_ERROR:
						entry->m_nRotationRateError = atoi(value);
						break;

					case CFG_PARAM_LIFE_SPAN_ERROR_SHAPE:
						entry->m_nLifeSpanErrorShape = atoi(value);
						break;

					case CFG_PARAM_TRAIL_LENGTH_MULTIPLIER:
						entry->m_fTrailLengthMultiplier = atof(value);
						break;
						
					case CFG_PARAM_STRETCH_VALUE_X:
						entry->m_vecTextureStretch.x = atof(value);
						break;

					case CFG_PARAM_STRETCH_VALUE_Y:
						entry->m_vecTextureStretch.y = atof(value);
						break;

					case CFG_PARAM_WIND_FACTOR:
						entry->m_fWindFactor = atof(value);
						break;

					case CFG_PARAM_PARTICLE_CREATE_RANGE:
						entry->m_fCreateRange = SQR(atof(value));
						break;

					case CFG_PARAM_FLAGS:
						entry->Flags = atoi(value);
						break;
				}
				
				value = strtok(NULL, delims);

				param++;

				if ( param > CFG_PARAM_LAST )
					param = CFG_PARAM_FIRST;
		
			} while ( value != NULL );
		}
		
		lineEnd++;
		lineStart = lineEnd;
		lineEnd++;
	}
}
//- rouz edit (ChatGPT)
