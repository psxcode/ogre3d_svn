/*
-----------------------------------------------------------------------------
This source file is part of OGRE 
	(Object-oriented Graphics Rendering Engine)
For the latest info, see http://ogre.sourceforge.net/

Copyright � 2000-2002 The OGRE Team
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
-----------------------------------------------------------------------------
*/

#include "OgreParticleEmitter.h"

namespace Ogre {

    // Define static members
    EmitterCommands::CmdAngle ParticleEmitter::msAngleCmd;
    EmitterCommands::CmdColour ParticleEmitter::msColourCmd;
    EmitterCommands::CmdColourRangeStart ParticleEmitter::msColourRangeStartCmd;
    EmitterCommands::CmdColourRangeEnd ParticleEmitter::msColourRangeEndCmd;
    EmitterCommands::CmdDirection ParticleEmitter::msDirectionCmd;
    EmitterCommands::CmdEmissionRate ParticleEmitter::msEmissionRateCmd;
    EmitterCommands::CmdMaxTTL ParticleEmitter::msMaxTTLCmd;
    EmitterCommands::CmdMaxVelocity ParticleEmitter::msMaxVelocityCmd;
    EmitterCommands::CmdMinTTL ParticleEmitter::msMinTTLCmd;
    EmitterCommands::CmdMinVelocity ParticleEmitter::msMinVelocityCmd;
    EmitterCommands::CmdPosition ParticleEmitter::msPositionCmd;
    EmitterCommands::CmdTTL ParticleEmitter::msTTLCmd;
    EmitterCommands::CmdVelocity ParticleEmitter::msVelocityCmd;
    EmitterCommands::CmdDuration ParticleEmitter::msDurationCmd;
    EmitterCommands::CmdMinDuration ParticleEmitter::msMinDurationCmd;
    EmitterCommands::CmdMaxDuration ParticleEmitter::msMaxDurationCmd;
    EmitterCommands::CmdRepeatDelay ParticleEmitter::msRepeatDelayCmd;
    EmitterCommands::CmdMinRepeatDelay ParticleEmitter::msMinRepeatDelayCmd;
    EmitterCommands::CmdMaxRepeatDelay ParticleEmitter::msMaxRepeatDelayCmd;


    //-----------------------------------------------------------------------
    ParticleEmitter::ParticleEmitter()
    {

        // Reasonable defaults
        mAngle = 0;
        setDirection(Vector3::UNIT_X);
        mEmissionRate = 10;
        mMaxSpeed = mMinSpeed = 1;
        mMaxTTL = mMinTTL = 5;
        mPosition = Vector3::ZERO;
        mColourRangeStart = mColourRangeEnd = ColourValue::White;
        mEnabled = true;
        mDurationMax = 0;

    }
    //-----------------------------------------------------------------------
    ParticleEmitter::~ParticleEmitter() 
    {
    }
    //-----------------------------------------------------------------------
    void ParticleEmitter::setPosition(const Vector3& pos) 
    { 
        mPosition = pos; 
    }
    //-----------------------------------------------------------------------
    Vector3 ParticleEmitter::getPosition(void) 
    { 
        return mPosition; 
    }
    //-----------------------------------------------------------------------
    void ParticleEmitter::setDirection(const Vector3& direction) 
    { 
        mDirection = direction; 
        mDirection.normalise();
        // Generate an up vector (any will do)
        mUp = mDirection.perpendicular();
        mUp.normalise();
    }
    //-----------------------------------------------------------------------
    Vector3& ParticleEmitter::getDirection(void) 
    { 
        return mDirection; 
    }
    //-----------------------------------------------------------------------
    void ParticleEmitter::setAngle(Real angleunits)
    {
        // Store as radians for efficiency
        mAngle = Math::AngleUnitsToRadians(angleunits);
    }
    //-----------------------------------------------------------------------
    Real ParticleEmitter::getAngle(void)
    {
        return Math::RadiansToAngleUnits(mAngle);
    }
    //-----------------------------------------------------------------------
    void ParticleEmitter::setParticleVelocity(Real speed)
    {
        mMinSpeed = mMaxSpeed = speed;
    }
    //-----------------------------------------------------------------------
    void ParticleEmitter::setParticleVelocity(Real min, Real max)
    {
        mMinSpeed = min;
        mMaxSpeed = max;
    }
    //-----------------------------------------------------------------------
    void ParticleEmitter::setEmissionRate(Real particlesPerSecond) 
    { 
        mEmissionRate = particlesPerSecond; 
    }
    //-----------------------------------------------------------------------
    Real ParticleEmitter::getEmissionRate(void) 
    { 
        return mEmissionRate; 
    }
    //-----------------------------------------------------------------------
    void ParticleEmitter::setTimeToLive(Real ttl)
    {
        mMinTTL = mMaxTTL = ttl;
    }
    //-----------------------------------------------------------------------
    void ParticleEmitter::setTimeToLive(Real minTtl, Real maxTtl)
    {
        mMinTTL = minTtl;
        mMaxTTL = maxTtl;
    }
    //-----------------------------------------------------------------------
    void ParticleEmitter::setColour(const ColourValue& colour)
    {
        mColourRangeStart = mColourRangeEnd = colour;
    }
    //-----------------------------------------------------------------------
    void ParticleEmitter::setColour(const ColourValue& colourStart, const ColourValue& colourEnd)
    {
        mColourRangeStart = colourStart;
        mColourRangeEnd = colourEnd;
    }
    //-----------------------------------------------------------------------
    void ParticleEmitter::genEmissionDirection(Vector3& destVector)
    {
        if (mAngle != 0)
        {
            // Randomise angle
            Real angle = Math::UnitRandom() * mAngle;

            // Randomise direction
            destVector = mDirection.randomDeviant(angle, mUp);
        }
        else
        {
            // Constant angle
            destVector = mDirection;
        }

        // Don't normalise, we can assume that it will still be a unit vector since
        // both direction and 'up' are.

    }
    //-----------------------------------------------------------------------
    void ParticleEmitter::genEmissionVelocity(Vector3& destVector)
    {
        Real scalar;
        if (mMinSpeed != mMaxSpeed)
        {
            scalar = mMinSpeed + (Math::UnitRandom() * (mMaxSpeed - mMinSpeed));
        }
        else
        {
            scalar = mMinSpeed;
        }

        destVector *= scalar;
    }
    //-----------------------------------------------------------------------
    Real ParticleEmitter::genEmissionTTL(void)
    {
        if (mMaxTTL != mMinTTL)
        {
            return mMinTTL + (Math::UnitRandom() * (mMaxTTL - mMinTTL));
        }
        else
        {
            return mMinTTL;
        }
    }
    //-----------------------------------------------------------------------
    unsigned short ParticleEmitter::genConstantEmissionCount(Real timeElapsed)
    {
        static Real remainder = 0;
        unsigned short intRequest;
        
        if (mEnabled)
        {
            // Keep fractions, otherwise a high frame rate will result in zero emissions!
            remainder += mEmissionRate * timeElapsed;
            intRequest = (unsigned short)remainder;
            remainder -= intRequest;

            // Check duration
            if (mDurationMax)
            {
                mDurationRemain -= timeElapsed;
                if (mDurationRemain <= 0) 
                {
                    // Disable, duration is out (takes effect next time)
                    setEnabled(false);
                }
            }
            return intRequest;
        }
        else
        {
            // Check repeat
            if (mRepeatDelayMax)
            {
                mRepeatDelayRemain -= timeElapsed;
                if (mRepeatDelayRemain <= 0)
                {
                    // Enable, repeat delay is out (takes effect next time)
                    setEnabled(true);
                }
            }
            return 0;
        }

    }
    //-----------------------------------------------------------------------
    void ParticleEmitter::genEmissionColour(ColourValue& destColour)
    {
        if (mColourRangeStart != mColourRangeEnd)
        {
            // Randomise
            //Real t = Math::UnitRandom();
            destColour.r = mColourRangeStart.r + (Math::UnitRandom() * (mColourRangeEnd.r - mColourRangeStart.r));
            destColour.g = mColourRangeStart.g + (Math::UnitRandom() * (mColourRangeEnd.g - mColourRangeStart.g));
            destColour.b = mColourRangeStart.b + (Math::UnitRandom() * (mColourRangeEnd.b - mColourRangeStart.b));
            destColour.a = mColourRangeStart.a + (Math::UnitRandom() * (mColourRangeEnd.a - mColourRangeStart.a));
        }
        else
        {
            destColour = mColourRangeStart;
        }
    }
    //-----------------------------------------------------------------------
    void ParticleEmitter::addBaseParameters(void)    
    {
        ParamDictionary* dict = getParamDictionary();

        dict->addParameter(ParameterDef("angle", 
            "The angle up to which particles may vary in their initial direction "
            "from the emitters direction, in degrees." , PT_REAL),
            &msAngleCmd);

        dict->addParameter(ParameterDef("colour", 
            "The colour of emitted particles.", PT_COLOURVALUE),
            &msColourCmd);

        dict->addParameter(ParameterDef("colour_range_start", 
            "The start of a range of colours to be assigned to emitted particles.", PT_COLOURVALUE),
            &msColourRangeStartCmd);

        dict->addParameter(ParameterDef("colour_range_end", 
            "The end of a range of colours to be assigned to emitted particles.", PT_COLOURVALUE),
            &msColourRangeEndCmd);

        dict->addParameter(ParameterDef("direction", 
            "The base direction of the emitter." , PT_VECTOR3),
            &msDirectionCmd);

        dict->addParameter(ParameterDef("emission_rate", 
            "The number of particles emitted per second." , PT_REAL),
            &msEmissionRateCmd);

        dict->addParameter(ParameterDef("position", 
            "The position of the emitter relative to the particle system center." , PT_VECTOR3),
            &msPositionCmd);

        dict->addParameter(ParameterDef("velocity", 
            "The initial velocity to be assigned to every particle, in world units per second." , PT_REAL),
            &msVelocityCmd);

        dict->addParameter(ParameterDef("velocity_min", 
            "The minimum initial velocity to be assigned to each particle." , PT_REAL),
            &msMinVelocityCmd);

        dict->addParameter(ParameterDef("velocity_max", 
            "The maximum initial velocity to be assigned to each particle." , PT_REAL),
            &msMaxVelocityCmd);

        dict->addParameter(ParameterDef("time_to_live", 
            "The lifetime of each particle in seconds." , PT_REAL),
            &msTTLCmd);

        dict->addParameter(ParameterDef("time_to_live_min", 
            "The minimum lifetime of each particle in seconds." , PT_REAL),
            &msMinTTLCmd);

        dict->addParameter(ParameterDef("time_to_live_max", 
            "The maximum lifetime of each particle in seconds." , PT_REAL),
            &msMaxTTLCmd);

        dict->addParameter(ParameterDef("duration", 
            "The length of time in seconds which an emitter stays enabled for." , PT_REAL),
            &msDurationCmd);

        dict->addParameter(ParameterDef("duration_min", 
            "The minimum length of time in seconds which an emitter stays enabled for." , PT_REAL),
            &msMinDurationCmd);

        dict->addParameter(ParameterDef("duration_max", 
            "The maximum length of time in seconds which an emitter stays enabled for." , PT_REAL),
            &msMaxDurationCmd);

        dict->addParameter(ParameterDef("repeat_delay", 
            "If set, after disabling an emitter will repeat (reenable) after this many seconds." , PT_REAL),
            &msRepeatDelayCmd);

        dict->addParameter(ParameterDef("repeat_delay_min", 
            "If set, after disabling an emitter will repeat (reenable) after this minimum number of seconds." , PT_REAL),
            &msMinRepeatDelayCmd);

        dict->addParameter(ParameterDef("repeat_delay_max", 
            "If set, after disabling an emitter will repeat (reenable) after this maximum number of seconds." , PT_REAL),
            &msMaxRepeatDelayCmd);
    }
    //-----------------------------------------------------------------------
    Real ParticleEmitter::getParticleVelocity(void)
    {
        return mMinSpeed;
    }
    //-----------------------------------------------------------------------
    Real ParticleEmitter::getMinParticleVelocity(void)
    {
        return mMinSpeed;
    }
    //-----------------------------------------------------------------------
    Real ParticleEmitter::getMaxParticleVelocity(void)
    {
        return mMaxSpeed;
    }
    //-----------------------------------------------------------------------
    void ParticleEmitter::setMinParticleVelocity(Real min)
    {
        mMinSpeed = min;
    }
    //-----------------------------------------------------------------------
    void ParticleEmitter::setMaxParticleVelocity(Real max)
    {
        mMaxSpeed = max;
    }
    //-----------------------------------------------------------------------
    Real ParticleEmitter::getTimeToLive(void)
    {
        return mMinTTL;
    }
    //-----------------------------------------------------------------------
    Real ParticleEmitter::getMinTimeToLive(void)
    {
        return mMinTTL;
    }
    //-----------------------------------------------------------------------
    Real ParticleEmitter::getMaxTimeToLive(void)
    {
        return mMaxTTL;
    }
    //-----------------------------------------------------------------------
    void ParticleEmitter::setMinTimeToLive(Real min)
    {
        mMinTTL = min;
    }
    //-----------------------------------------------------------------------
    void ParticleEmitter::setMaxTimeToLive(Real max)
    {
        mMaxTTL = max;
    }
    //-----------------------------------------------------------------------
    ColourValue ParticleEmitter::getColour(void)
    {
        return mColourRangeStart;
    }
    //-----------------------------------------------------------------------
    ColourValue ParticleEmitter::getColourRangeStart(void)
    {
        return mColourRangeStart;
    }
    //-----------------------------------------------------------------------
    ColourValue ParticleEmitter::getColourRangeEnd(void)
    {
        return mColourRangeEnd;
    }
    //-----------------------------------------------------------------------
    void ParticleEmitter::setColourRangeStart(const ColourValue& val)
    {
        mColourRangeStart = val;
    }
    //-----------------------------------------------------------------------
    void ParticleEmitter::setColourRangeEnd(const ColourValue& val )
    {
        mColourRangeEnd = val;
    }
    //-----------------------------------------------------------------------
    void ParticleEmitter::setEnabled(bool enabled)
    {
        mEnabled = enabled;
        if (enabled)
        {
            // Reset duration 
            if (mDurationMin == mDurationMax)
            {
                mDurationRemain = mDurationMin;
            }
            else
            {
                mDurationRemain = Math::UnitRandom() * (mDurationMax - mDurationMin);
            }
        }
        else
        {
            // Reset repeat
            if (mRepeatDelayMin == mRepeatDelayMax)
            {
                mRepeatDelayRemain = mRepeatDelayMin;
            }
            else
            {
                mRepeatDelayRemain = Math::UnitRandom() * (mRepeatDelayMax - mRepeatDelayMin);
            }

        }
    }
    //-----------------------------------------------------------------------
    bool ParticleEmitter::getEnabled(void)
    {
        return mEnabled;
    }
    //-----------------------------------------------------------------------
    void ParticleEmitter::setStartTime(Real startTime)
    {
        mStartTime = startTime;
    }
    //-----------------------------------------------------------------------
    Real ParticleEmitter::getStartTime(void)
    {
        return mStartTime;
    }
    //-----------------------------------------------------------------------
    void ParticleEmitter::setDuration(Real duration)
    {
        mDurationMin = mDurationMax = duration;
    }
    //-----------------------------------------------------------------------
    Real ParticleEmitter::getDuration(void)
    {
        return mDurationMin;
    }
    //-----------------------------------------------------------------------
    void ParticleEmitter::setDuration(Real min, Real max)
    {
        mDurationMin = min;
        mDurationMax = max;
    }
    //-----------------------------------------------------------------------
    void ParticleEmitter::setMinDuration(Real min)
    {
        mDurationMin = min;
    }
    //-----------------------------------------------------------------------
    void ParticleEmitter::setMaxDuration(Real max)
    {
        mDurationMax = max;
    }
    //-----------------------------------------------------------------------
    void ParticleEmitter::setRepeatDelay(Real delay)
    {
        mRepeatDelayMin = mRepeatDelayMax = delay;
    }
    //-----------------------------------------------------------------------
    Real ParticleEmitter::getRepeatDelay(void)
    {
        return mRepeatDelayMin;
    }
    //-----------------------------------------------------------------------
    void ParticleEmitter::setRepeatDelay(Real min, Real max)
    {
        mRepeatDelayMin = min;
        mRepeatDelayMax = max;
    }
    //-----------------------------------------------------------------------
    void ParticleEmitter::setMinRepeatDelay(Real min)
    {
        mRepeatDelayMin = min;
    }
    //-----------------------------------------------------------------------
    void ParticleEmitter::setMaxRepeatDelay(Real max)
    {
        mRepeatDelayMax = max;
    }
    //-----------------------------------------------------------------------
    Real ParticleEmitter::getMinDuration(void)
    {
        return mDurationMin;
    }
    //-----------------------------------------------------------------------
    Real ParticleEmitter::getMaxDuration(void)
    {
        return mDurationMax;
    }
    //-----------------------------------------------------------------------
    Real ParticleEmitter::getMinRepeatDelay(void)
    {
        return mRepeatDelayMin;    
    }
    //-----------------------------------------------------------------------
    Real ParticleEmitter::getMaxRepeatDelay(void)
    {
        return mRepeatDelayMax;    
    }

    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------

}

