#
# Build LowLevel
#

SET(GW_DEPS_ROOT $ENV{GW_DEPS_ROOT})
FIND_PACKAGE(PxShared REQUIRED)

SET(PHYSX_SOURCE_DIR ${PROJECT_SOURCE_DIR}/../../../)

SET(LL_SOURCE_DIR ${PHYSX_SOURCE_DIR}/LowLevel)

FIND_PACKAGE(nvToolsExt REQUIRED)

SET(LOWLEVEL_PLATFORM_INCLUDES
	${NVTOOLSEXT_INCLUDE_DIRS}
	${PHYSX_SOURCE_DIR}/Common/src/mac
	${PHYSX_SOURCE_DIR}/LowLevel/software/include/mac
	${PHYSX_SOURCE_DIR}/LowLevelDynamics/include/mac
	${PHYSX_SOURCE_DIR}/LowLevel/common/include/pipeline/mac
)

SET(LOWLEVEL_COMPILE_DEFS
	# Common to all configurations
	${PHYSX_MAC_COMPILE_DEFS};PX_PHYSX_STATIC_LIB

	$<$<CONFIG:debug>:${PHYSX_MAC_DEBUG_COMPILE_DEFS};PX_PHYSX_DLL_NAME_POSTFIX=DEBUG;>
	$<$<CONFIG:checked>:${PHYSX_MAC_CHECKED_COMPILE_DEFS};PX_PHYSX_DLL_NAME_POSTFIX=CHECKED;>
	$<$<CONFIG:profile>:${PHYSX_MAC_PROFILE_COMPILE_DEFS};PX_PHYSX_DLL_NAME_POSTFIX=PROFILE;>
	$<$<CONFIG:release>:${PHYSX_MAC_RELEASE_COMPILE_DEFS};>
)

# include common low level settings
INCLUDE(../common/LowLevel.cmake)

TARGET_LINK_LIBRARIES(LowLevel PUBLIC ${NVTOOLSEXT_LIBRARIES})
SET_TARGET_PROPERTIES(LowLevel PROPERTIES LINK_FLAGS ""	)

# enable -fPIC so we can link static libs with the editor
SET_TARGET_PROPERTIES(LowLevel PROPERTIES POSITION_INDEPENDENT_CODE TRUE)
