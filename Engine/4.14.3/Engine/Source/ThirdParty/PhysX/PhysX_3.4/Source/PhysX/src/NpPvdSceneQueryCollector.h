// This code contains NVIDIA Confidential Information and is disclosed to you
// under a form of NVIDIA software license agreement provided separately to you.
//
// Notice
// NVIDIA Corporation and its licensors retain all intellectual property and
// proprietary rights in and to this software and related documentation and
// any modifications thereto. Any use, reproduction, disclosure, or
// distribution of this software and related documentation without an express
// license agreement from NVIDIA Corporation is strictly prohibited.
//
// ALL NVIDIA DESIGN SPECIFICATIONS, CODE ARE PROVIDED "AS IS.". NVIDIA MAKES
// NO WARRANTIES, EXPRESSED, IMPLIED, STATUTORY, OR OTHERWISE WITH RESPECT TO
// THE MATERIALS, AND EXPRESSLY DISCLAIMS ALL IMPLIED WARRANTIES OF NONINFRINGEMENT,
// MERCHANTABILITY, AND FITNESS FOR A PARTICULAR PURPOSE.
//
// Information and code furnished is believed to be accurate and reliable.
// However, NVIDIA Corporation assumes no responsibility for the consequences of use of such
// information or for any infringement of patents or other rights of third parties that may
// result from its use. No license is granted by implication or otherwise under any patent
// or patent rights of NVIDIA Corporation. Details are subject to change without notice.
// This code supersedes and replaces all information previously supplied.
// NVIDIA Corporation products are not authorized for use as critical
// components in life support devices or systems without express written approval of
// NVIDIA Corporation.
//
// Copyright (c) 2008-2016 NVIDIA Corporation. All rights reserved.
// Copyright (c) 2004-2008 AGEIA Technologies, Inc. All rights reserved.
// Copyright (c) 2001-2004 NovodeX AG. All rights reserved.

#ifndef NP_PVD_SCENEQUERYCOLLECTOR_H
#define NP_PVD_SCENEQUERYCOLLECTOR_H

#include "CmPhysXCommon.h"
#include "PsArray.h"
#include "PxFiltering.h"
#include "PxGeometryHelpers.h"
#include "PxQueryReport.h"
#include "PxBatchQueryDesc.h"

#if PX_SUPPORT_PVD

namespace physx
{
namespace Scb
{
class Scene;
}

namespace Vd
{
struct PvdReference
{
	PX_FORCE_INLINE PvdReference()																													{}
	PX_FORCE_INLINE PvdReference(const char* arrayName, PxU32 baseIndex, PxU32 count) : mArrayName(arrayName), mBaseIndex(baseIndex), mCount(count)	{}

	const char*		mArrayName;
	PxU32			mBaseIndex;
	PxU32			mCount;
};

struct PvdRaycast
{
	PxU32			mType;
	PxFilterData	mFilterData;
	PxU32			mFilterFlags;
	PxVec3			mOrigin;
	PxVec3			mUnitDir;
	PxReal			mDistance;
	PvdReference	mHits;
};

struct PvdOverlap
{
	PxU32			mType;
	PxFilterData	mFilterData;
	PxU32			mFilterFlags;
	PxTransform		mPose;
	PvdReference	mGeometries;
	PvdReference	mHits;
};

struct PvdSweep
{
	PxU32			mType;
	PxU32			mFilterFlags;
	PxVec3			mUnitDir;
	PxReal			mDistance;
	PvdReference	mGeometries;
	PvdReference	mPoses;
	PvdReference	mFilterData;
	PvdReference	mHits;
};

struct PvdSqHit
{
	const void*		mShape;
	const void*		mActor;
	PxU32			mFaceIndex;
	PxU32			mFlags;
	PxVec3			mImpact;
	PxVec3			mNormal;
	PxF32			mDistance;
	PxF32			mU;
	PxF32			mV;
	PxU32			mSweepGeometryIndex;	// PT: TODO: remove. This seems unused these days.

	PvdSqHit()
	{
		setDefaults(PxQueryHit());
	}

	explicit PvdSqHit(const PxOverlapHit& hit)
	{
		setDefaults(hit);
	}

	explicit PvdSqHit(const PxRaycastHit& hit)
	{
		setDefaults(hit);

		mImpact = hit.position;
		mNormal = hit.normal;
		mDistance = hit.distance;
		mFlags = hit.flags;
		mU = hit.u;
		mV = hit.v;
	}

	explicit PvdSqHit(const PxSweepHit& hit)
	{
		setDefaults(hit);

		mImpact = hit.position;
		mNormal = hit.normal;
		mDistance = hit.distance;
		mFlags = hit.flags;
		mSweepGeometryIndex = 0; // unused
	}

  private:
	void setDefaults(const PxQueryHit& hit)
	{
		mShape = hit.shape;
		mActor = hit.actor;
		mFaceIndex = hit.faceIndex;
		mFlags = 0;
		mImpact = mNormal = PxVec3(0.0f);
		mDistance = mU = mV = 0.0f;
		mSweepGeometryIndex = 0;
	}
};

template <typename T, bool isBatched>
inline const char* PvdGetArrayName()
{
	return T::template getArrayName<isBatched>();
}
template <>
inline const char* PvdGetArrayName<PxGeometryHolder, false>()
{
	return "SceneQueries.GeometryList";
}
template <>
inline const char* PvdGetArrayName<PxTransform, false>()
{
	return "SceneQueries.PoseList";
}
template <>
inline const char* PvdGetArrayName<PxFilterData, false>()
{
	return "SceneQueries.FilterDataList";
}
template <>
inline const char* PvdGetArrayName<PvdRaycast, false>()
{
	return "SceneQueries.Raycasts";
}
template <>
inline const char* PvdGetArrayName<PvdOverlap, false>()
{
	return "SceneQueries.Overlaps";
}
template <>
inline const char* PvdGetArrayName<PvdSweep, false>()
{
	return "SceneQueries.Sweeps";
}
template <>
inline const char* PvdGetArrayName<PvdSqHit, false>()
{
	return "SceneQueries.Hits";
}
template <>
inline const char* PvdGetArrayName<PxGeometryHolder, true>()
{
	return "BatchedQueries.GeometryList";
}
template <>
inline const char* PvdGetArrayName<PxTransform, true>()
{
	return "BatchedQueries.PoseList";
}
template <>
inline const char* PvdGetArrayName<PxFilterData, true>()
{
	return "BatchedQueries.FilterDataList";
}
template <>
inline const char* PvdGetArrayName<PvdRaycast, true>()
{
	return "BatchedQueries.Raycasts";
}
template <>
inline const char* PvdGetArrayName<PvdOverlap, true>()
{
	return "BatchedQueries.Overlaps";
}
template <>
inline const char* PvdGetArrayName<PvdSweep, true>()
{
	return "BatchedQueries.Sweeps";
}
template <>
inline const char* PvdGetArrayName<PvdSqHit, true>()
{
	return "BatchedQueries.Hits";
}

class PvdSceneQueryCollector
{
	PX_NOCOPY(PvdSceneQueryCollector)
public:
	PvdSceneQueryCollector(Scb::Scene& scene, bool isBatched) :	mScene(scene), mInUse(0), mIsBatched(isBatched)	{}
	~PvdSceneQueryCollector()																					{}

	void clear()
	{
		Ps::Mutex::ScopedLock lock(mMutex);

		mAccumulatedRaycastQueries.clear();
		mAccumulatedOverlapQueries.clear();
		mAccumulatedSweepQueries.clear();
		mPvdSqHits.clear();
		mPoses.clear();
		mFilterData.clear();
	}

	void clearGeometryArrays()
	{
		mGeometries[0].clear();
		mGeometries[1].clear();
	}

	void release();

	void raycast(const PxVec3& origin, const PxVec3& unitDir, PxReal distance, const PxRaycastHit* hit, PxU32 hitsNum, const PxQueryFilterData& filterData, bool multipleHits);
	void sweep(const PxGeometry& geometry, const PxTransform& pose, const PxVec3& unitDir, PxReal distance, const PxSweepHit* hit, PxU32 hitsNum, const PxQueryFilterData& filterData, bool multipleHits);
	void overlapMultiple(const PxGeometry& geometry, const PxTransform& pose, const PxOverlapHit* hit, PxU32 hitsNum, const PxQueryFilterData& filterData);

	void collectAllBatchedHits	(const PxRaycastQueryResult* raycastResults, PxU32 nbRaycastResults, PxU32 batchedRayQstartIdx,
								const PxOverlapQueryResult* overlapResults, PxU32 nbOverlapResults, PxU32 batchedOverlapQstartIdx,
								const PxSweepQueryResult* sweepResults, PxU32 nbSweepResults, PxU32 batchedSweepQstartIdx);

	PX_FORCE_INLINE	Ps::Mutex& getLock()
	{
		return mMutex;
	}

	PX_FORCE_INLINE	const Ps::Array<PxGeometryHolder>& getCurrentFrameGeometries() const
	{
		return mGeometries[mInUse];
	}
	PX_FORCE_INLINE	const Ps::Array<PxGeometryHolder>& getPrevFrameGeometries() const
	{
		return mGeometries[mInUse ^ 1];
	}
	void prepareNextFrameGeometries()
	{
		mInUse ^= 1;
		mGeometries[mInUse].clear();
	}
	template <typename T>
	const char* getArrayName(const Ps::Array<T>&)
	{
		return mIsBatched ? PvdGetArrayName<T, 1>() : PvdGetArrayName<T, 0>();
	}

	// Scene query and hits for pvd, collected in current frame
	Ps::Array<PvdRaycast>		mAccumulatedRaycastQueries;
	Ps::Array<PvdSweep>			mAccumulatedSweepQueries;
	Ps::Array<PvdOverlap>		mAccumulatedOverlapQueries;
	Ps::Array<PvdSqHit>			mPvdSqHits;
	Ps::Array<PxTransform>		mPoses;
	Ps::Array<PxFilterData>		mFilterData;

private:
	Scb::Scene&					mScene;
	Ps::Mutex					mMutex;
	Ps::Array<PxGeometryHolder>	mGeometries[2];
	PxU32						mInUse;
	const bool					mIsBatched;
};
}
}

#endif // PX_SUPPORT_PVD

#endif // NP_PVD_SCENEQUERYCOLLECTOR_H
