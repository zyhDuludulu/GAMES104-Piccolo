// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#include <Jolt/Physics/Constraints/TwoBodyConstraint.h>
#include <Jolt/Physics/Constraints/MotorSettings.h>
#include <Jolt/Physics/Constraints/ConstraintPart/PointConstraintPart.h>
#include <Jolt/Physics/Constraints/ConstraintPart/AxisConstraintPart.h>
#include <Jolt/Physics/Constraints/ConstraintPart/AngleConstraintPart.h>
#include <Jolt/Physics/Constraints/ConstraintPart/RotationEulerConstraintPart.h>
#include <Jolt/Physics/Constraints/ConstraintPart/SwingTwistConstraintPart.h>

JPH_NAMESPACE_BEGIN

/// 6 Degree Of Freedom Constraint setup structure. Allows control over each of the 6 degrees of freedom.
class SixDOFConstraintSettings final : public TwoBodyConstraintSettings
{
public:
	JPH_DECLARE_SERIALIZABLE_VIRTUAL(SixDOFConstraintSettings)

	/// Constraint is split up into translation/rotation around X, Y and Z axis.
	enum EAxis
	{
		TranslationX,
		TranslationY,
		TranslationZ,

		RotationX,				///< When limited: MinLimit needs to be [-PI, 0], MaxLimit needs to be [0, PI]
		RotationY,				///< When limited: MaxLimit between [0, PI]. MinLimit = -MaxLimit. Forms a cone shaped limit with Z.
		RotationZ,				///< When limited: MaxLimit between [0, PI]. MinLimit = -MaxLimit. Forms a cone shaped limit with Y.

		Num,
	};

	// See: ConstraintSettings::SaveBinaryState
	virtual void				SaveBinaryState(StreamOut &inStream) const override;

	/// Create an an instance of this constraint
	virtual TwoBodyConstraint *	Create(Body &inBody1, Body &inBody2) const override;

	/// This determines in which space the constraint is setup, all properties below should be in the specified space
	EConstraintSpace			mSpace = EConstraintSpace::WorldSpace;

	/// Body 1 constraint reference frame (space determined by mSpace)
	Vec3						mPosition1 = Vec3::sZero();
	Vec3						mAxisX1 = Vec3::sAxisX();
	Vec3						mAxisY1 = Vec3::sAxisY();

	/// Body 2 constraint reference frame (space determined by mSpace)
	Vec3						mPosition2 = Vec3::sZero();
	Vec3						mAxisX2 = Vec3::sAxisX();
	Vec3						mAxisY2 = Vec3::sAxisY();
	
	/// Friction settings.
	/// For translation: Max friction force in N. 0 = no friction.
	/// For rotation: Max friction torque in Nm. 0 = no friction.
	float						mMaxFriction[EAxis::Num] = { 0, 0, 0, 0, 0, 0 };

	/// Limits.
	/// For translation: Min and max linear limits in m (0 is frame of body 1 and 2 coincide). 
	/// For rotation: Min and max angular limits in rad (0 is frame of body 1 and 2 coincide). See comments at Axis enum for limit ranges.
	///
	/// Remove degree of freedom by setting min = FLT_MAX and max = -FLT_MAX. The constraint will be driven to 0 for this axis.
	///
	/// Free movement over an axis is allowed when min = -FLT_MAX and max = FLT_MAX.
	float						mLimitMin[EAxis::Num] = { -FLT_MAX, -FLT_MAX, -FLT_MAX, -FLT_MAX, -FLT_MAX, -FLT_MAX };
	float						mLimitMax[EAxis::Num] = { FLT_MAX, FLT_MAX, FLT_MAX, FLT_MAX, FLT_MAX, FLT_MAX };

	/// Make axis free (unconstrained)
	void						MakeFreeAxis(EAxis inAxis)									{ mLimitMin[inAxis] = -FLT_MAX; mLimitMax[inAxis] = FLT_MAX; }
	bool						IsFreeAxis(EAxis inAxis) const								{ return mLimitMin[inAxis] == -FLT_MAX && mLimitMax[inAxis] == FLT_MAX; }

	/// Make axis fixed (fixed at value 0)
	void						MakeFixedAxis(EAxis inAxis)									{ mLimitMin[inAxis] = FLT_MAX; mLimitMax[inAxis] = -FLT_MAX; }
	bool						IsFixedAxis(EAxis inAxis) const								{ return mLimitMin[inAxis] >= mLimitMax[inAxis]; }

	/// Set a valid range for the constraint
	void						SetLimitedAxis(EAxis inAxis, float inMin, float inMax)		{ JPH_ASSERT(inMin < inMax); JPH_ASSERT(inMin <= 0.0f); JPH_ASSERT(inMax >= 0.0f); mLimitMin[inAxis] = inMin; mLimitMax[inAxis] = inMax; }

	/// Motor settings for each axis
	MotorSettings				mMotorSettings[EAxis::Num];

protected:
	// See: ConstraintSettings::RestoreBinaryState
	virtual void				RestoreBinaryState(StreamIn &inStream) override;
};

/// 6 Degree Of Freedom Constraint. Allows control over each of the 6 degrees of freedom.
class SixDOFConstraint final : public TwoBodyConstraint
{
public:
	/// Get Axis from settings class
	using EAxis = SixDOFConstraintSettings::EAxis;

	/// Construct six DOF constraint
								SixDOFConstraint(Body &inBody1, Body &inBody2, const SixDOFConstraintSettings &inSettings);

	/// Generic interface of a constraint
	virtual EConstraintSubType	GetSubType() const override									{ return EConstraintSubType::SixDOF; }
	virtual void				SetupVelocityConstraint(float inDeltaTime) override;
	virtual void				WarmStartVelocityConstraint(float inWarmStartImpulseRatio) override;
	virtual bool				SolveVelocityConstraint(float inDeltaTime) override;
	virtual bool				SolvePositionConstraint(float inDeltaTime, float inBaumgarte) override;
#ifdef JPH_DEBUG_RENDERER
	virtual void				DrawConstraint(DebugRenderer *inRenderer) const override;
	virtual void				DrawConstraintLimits(DebugRenderer *inRenderer) const override;
#endif // JPH_DEBUG_RENDERER
	virtual void				SaveState(StateRecorder &inStream) const override;
	virtual void				RestoreState(StateRecorder &inStream) override;
	virtual Ref<ConstraintSettings> GetConstraintSettings() const override;

	// See: TwoBodyConstraint
	virtual Mat44				GetConstraintToBody1Matrix() const override					{ return Mat44::sRotationTranslation(mConstraintToBody1, mLocalSpacePosition1); }
	virtual Mat44				GetConstraintToBody2Matrix() const override					{ return Mat44::sRotationTranslation(mConstraintToBody2, mLocalSpacePosition2); }

	/// Update the translation limits for this constraint, note that this won't change if axis are free or not.
	void						SetTranslationLimits(Vec3Arg inLimitMin, Vec3Arg inLimitMax);

	/// Update the rotational limits for this constraint, note that this won't change if axis are free or not.
	void						SetRotationLimits(Vec3Arg inLimitMin, Vec3Arg inLimitMax);

	/// Set the max friction for each axis
	void						SetMaxFriction(EAxis inAxis, float inFriction);
	float						GetMaxFriction(EAxis inAxis) const							{ return mMaxFriction[inAxis]; }

	/// Get rotation of constraint in constraint space
	inline Quat					GetRotationInConstraintSpace() const;

	/// Motor settings
	MotorSettings &				GetMotorSettings(EAxis inAxis)								{ return mMotorSettings[inAxis]; }
	const MotorSettings &		GetMotorSettings(EAxis inAxis) const						{ return mMotorSettings[inAxis]; }

	/// Motor controls.
	/// Translation motors work in constraint space of body 1.
	/// Rotation motors work in constraint space of body 2 (!).
	void						SetMotorState(EAxis inAxis, EMotorState inState);
	EMotorState					GetMotorState(EAxis inAxis) const							{ return mMotorState[inAxis]; }

	/// Set the target velocity in body 1 constraint space
	Vec3		 				GetTargetVelocityCS() const									{ return mTargetVelocity; }
	void						SetTargetVelocityCS(Vec3Arg inVelocity)						{ mTargetVelocity = inVelocity; }

	/// Set the target angular velocity in body 2 constraint space (!)
	void						SetTargetAngularVelocityCS(Vec3Arg inAngularVelocity)		{ mTargetAngularVelocity = inAngularVelocity; }
	Vec3		 				GetTargetAngularVelocityCS() const							{ return mTargetAngularVelocity; }

	/// Set the target position in body 1 constraint space
	Vec3		 				GetTargetPositionCS() const									{ return mTargetPosition; }
	void						SetTargetPositionCS(Vec3Arg inPosition)						{ mTargetPosition = inPosition; }

	/// Set the target orientation in body 1 constraint space
	void						SetTargetOrientationCS(QuatArg inOrientation);
	Quat						GetTargetOrientationCS() const								{ return mTargetOrientation; }

	/// Set the target orientation in body space (R2 = R1 * inOrientation, where R1 and R2 are the world space rotations for body 1 and 2).
	/// Solve: R2 * ConstraintToBody2 = R1 * ConstraintToBody1 * q (see SwingTwistConstraint::GetSwingTwist) and R2 = R1 * inOrientation for q.
	void						SetTargetOrientationBS(QuatArg inOrientation)				{ SetTargetOrientationCS(mConstraintToBody1.Conjugated() * inOrientation * mConstraintToBody2); }

	///@name Get Lagrange multiplier from last physics update (relates to how much force/torque was applied to satisfy the constraint)
	inline Vec3		 			GetTotalLambdaPosition() const								{ return IsTranslationFullyConstrained()? mPointConstraintPart.GetTotalLambda() : Vec3(mTranslationConstraintPart[0].GetTotalLambda(), mTranslationConstraintPart[1].GetTotalLambda(), mTranslationConstraintPart[2].GetTotalLambda()); }
	inline Vec3					GetTotalLambdaRotation() const								{ return IsRotationFullyConstrained()? mRotationConstraintPart.GetTotalLambda() : Vec3(mSwingTwistConstraintPart.GetTotalTwistLambda(), mSwingTwistConstraintPart.GetTotalSwingYLambda(), mSwingTwistConstraintPart.GetTotalSwingZLambda()); }
	inline Vec3					GetTotalLambdaMotorTranslation() const						{ return Vec3(mMotorTranslationConstraintPart[0].GetTotalLambda(), mMotorTranslationConstraintPart[1].GetTotalLambda(), mMotorTranslationConstraintPart[2].GetTotalLambda()); }
	inline Vec3					GetTotalLambdaMotorRotation() const							{ return Vec3(mMotorRotationConstraintPart[0].GetTotalLambda(), mMotorRotationConstraintPart[1].GetTotalLambda(), mMotorRotationConstraintPart[2].GetTotalLambda()); }

private:
	// Calculate properties needed for the position constraint
	inline void					GetPositionConstraintProperties(Vec3 &outR1PlusU, Vec3 &outR2, Vec3 &outU) const;

	// Propagate the rotation limits to the constraint part
	inline void					UpdateRotationLimits();

	// Cache the state of mTranslationMotorActive
	void						CacheTranslationMotorActive();

	// Cache the state of mRotationMotorActive
	void						CacheRotationMotorActive();

	// Constraint settings helper functions
	inline bool					IsAxisFixed(EAxis inAxis) const								{ return (mFixedAxis & (1 << inAxis)) != 0; }
	inline bool					IsAxisFree(EAxis inAxis) const								{ return (mFreeAxis & (1 << inAxis)) != 0; }
	inline bool					IsTranslationConstrained() const							{ return (mFreeAxis & 0b111) != 0b111; }
	inline bool					IsTranslationFullyConstrained() const						{ return (mFixedAxis & 0b111) == 0b111; }
	inline bool					IsRotationConstrained() const								{ return (mFreeAxis & 0b111000) != 0b111000; }
	inline bool					IsRotationFullyConstrained() const							{ return (mFixedAxis & 0b111000) == 0b111000; }
	inline bool					HasFriction(EAxis inAxis) const								{ return !IsAxisFixed(inAxis) && mMaxFriction[inAxis] > 0.0f; }

	// CONFIGURATION PROPERTIES FOLLOW

	// Local space constraint positions
	Vec3						mLocalSpacePosition1;
	Vec3						mLocalSpacePosition2;

	// Transforms from constraint space to body space
	Quat						mConstraintToBody1;
	Quat						mConstraintToBody2;

	// Limits
	uint8						mFreeAxis;													// Bitmask of free axis (bit 0 = TranslationX)
	uint8						mFixedAxis;													// Bitmask of fixed axis (bit 0 = TranslationX)
	bool						mTranslationMotorActive = false;							// If any of the translational frictions / motors are active
	bool						mRotationMotorActive = false;								// If any of the rotational frictions / motors are active
	uint8						mRotationPositionMotorActive = 0;							// Bitmask of axis that have position motor active (bit 0 = RotationX)
	float						mLimitMin[EAxis::Num];
	float						mLimitMax[EAxis::Num];

	// Motor settings for each axis
	MotorSettings				mMotorSettings[EAxis::Num];

	// Friction settings for each axis
	float						mMaxFriction[EAxis::Num];

	// Motor controls
	EMotorState					mMotorState[EAxis::Num] = { EMotorState::Off, EMotorState::Off, EMotorState::Off, EMotorState::Off, EMotorState::Off, EMotorState::Off };
	Vec3						mTargetVelocity = Vec3::sZero();
	Vec3						mTargetAngularVelocity = Vec3::sZero();
	Vec3						mTargetPosition = Vec3::sZero();
	Quat						mTargetOrientation = Quat::sIdentity();

	// RUN TIME PROPERTIES FOLLOW

	// Constraint space axis in world space
	Vec3						mTranslationAxis[3];
	Vec3						mRotationAxis[3];

	// Translation displacement (valid when translation axis has a range limit)
	float						mDisplacement[3];

	// Individual constraint parts for translation, or a combined point constraint part if all axis are fixed
	AxisConstraintPart			mTranslationConstraintPart[3];
	PointConstraintPart			mPointConstraintPart;

	// Individual constraint parts for rotation or a combined constraint part if rotation is fixed
	SwingTwistConstraintPart	mSwingTwistConstraintPart;
	RotationEulerConstraintPart	mRotationConstraintPart;

	// Motor or friction constraints
	AxisConstraintPart			mMotorTranslationConstraintPart[3];
	AngleConstraintPart			mMotorRotationConstraintPart[3];
};

JPH_NAMESPACE_END
