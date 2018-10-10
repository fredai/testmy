#include <cuda.h>
#include <cuda_runtime.h>
#include <host_defines.h>
#include <string.h>
#include <stdio.h>
#include "cudainfo.h"

#if CUDA_VERSION < 5050
#error CUDA 1.x - 5.0 are not supported any more! Please use CUDA Toolkit 5.5+ instead.
#endif

#define CZ_COPY_BUF_SIZE	(16 * (1 << 20))	/*!< Transfer buffer size. */
#define CZ_COPY_LOOPS_NUM	8			/*!< Number of loops to run transfer test to. */

#define CZ_CALC_BLOCK_LOOPS	16			/*!< Number of loops to run calculation loop. */
#define CZ_CALC_BLOCK_SIZE	256			/*!< Size of instruction block. */
#define CZ_CALC_BLOCK_NUM	16			/*!< Number of instruction blocks in loop. */
#define CZ_CALC_OPS_NUM		2			/*!< Number of operations per one loop. */
#define CZ_CALC_LOOPS_NUM	8			/*!< Number of loops to run performance test to. */

#define CZ_DEF_WARP_SIZE	32			/*!< Default warp size value. */
#define CZ_DEF_THREADS_MAX	512			/*!< Default max threads value value. */

#define CZ_VER_STR_LEN		256			/*!< Version string length. */

/*!	\brief Error handling of CUDA RT calls.
*/
#define CZ_CUDA_CALL(funcCall, errProc) \
	{ \
		cudaError_t errCode; \
		if((errCode = (funcCall)) != cudaSuccess) { \
			printf("CUDA Error: %08x %s", errCode, cudaGetErrorString(errCode)); \
			errProc; \
		} \
	}
 
/*!	\brief Check how many CUDA-devices are present.
	\return number of CUDA-devices in case of success, \a 0 if no CUDA-devies were found.
*/
int CZCudaDeviceFound(void) 
{
	int count;

	CZ_CUDA_CALL(cudaGetDeviceCount(&count),
		return 0);

	return count;
}

/*!	\def ConvertSMVer2Cores(major, minor)
	\brief Get number of CUDA cores per multiprocessor.
	\arg[in] major GPU Architecture major version.
	\arg[in] minor GPU Architecture minor version.
	\returns 0 if GPU Architecture is unknown, or number of CUDA cores per multiprocessor.
*/
#define ConvertSMVer2Cores(major, minor) \
	(((major) == 1)? ( /* Tesla */ \
		((minor) == 0)? 8: /* G80*/ \
		((minor) == 1)? 8: /* G8x, G9x */ \
		((minor) == 2)? 8: /* GT21x */ \
		((minor) == 3)? 8: /* GT200 */ \
		0): \
	((major) == 2)? ( /* Fermi */ \
		((minor) == 0)? 32: /* GF100, GF110 */ \
		((minor) == 1)? 48: /* GF10x, FG11x */ \
		0): \
	((major) == 3)? ( /* Kepler */ \
		((minor) == 0)? 192: /* GK10x */ \
		((minor) == 2)? 192: /* Tegra K1 */ \
		((minor) == 5)? 192: /* GK11x, GK208 */ \
		((minor) == 7)? 192: /* GK210 */ \
		0): \
	((major) == 5)? ( /* Maxwell */ \
		((minor) == 0)? 128: /* GM10X */ \
		((minor) == 2)? 128: /* GM20X */ \
		((minor) == 3)? 128: /* Tegra X1 */ \
		0): \
	0)


void getValue1024(double value, char *valueStr, int prefix = 0, const char *unit = "B")
{
	const int prefixBase = 1024;
	int resPrefix = prefix;

	static const char *prefixTab[9] = {
		"",	/* prefixNothing */
		"K",	/* prefixK */
		"M",	/* prefixM */
		"G",	/* prefixG */
		"T",	/* prefixT */
		"P",	/* prefixP */
		"E",	/* prefixE */
		"Z",	/* prefixZ */
		"Y",	/* prefixY */
	};

	while((value > prefixBase) && (resPrefix < 9)) 
	{
		value /= prefixBase;
		resPrefix++;
	}
	sprintf(valueStr, "%.2f %s%s", value, prefixTab[resPrefix], unit);
}

/*!	\brief Local service data structure for bandwith calulations.
 * */
struct CZDeviceInfoBandLocalData {
	void		*memHostPage;	/*!< Pageable host memory. */
	void		*memHostPin;	/*!< Pinned host memory. */
	void		*memDevice1;	/*!< Device memory buffer 1. */
	void		*memDevice2;	/*!< Device memory buffer 2. */
};

/*!	\brief Set device for current thread.
 * */
int CZCudaCalcDeviceSelect(
	struct CZDeviceInfo *info	/*!<[in,out] CUDA-device information. */
)
{
	CZ_CUDA_CALL(cudaSetDevice(info->num),
		return -1);

	return 0;
}

/*!	\brief Allocate buffers for bandwidth calculations.
 * 	\return \a 0 in case of success, \a -1 in case of error.
 * 	*/
static int CZCudaCalcDeviceBandwidthAlloc(
	struct CZDeviceInfo *info	/*!<[in,out] CUDA-device information. */
)
{
	CZDeviceInfoBandLocalData *lData;

	if(info == NULL)
		return -1;

	if(info->band.localData == NULL)
    {

		//CZLog(CZLogLevelLow, "Alloc local buffers for %s.", info->deviceName);

		lData = (CZDeviceInfoBandLocalData*)malloc(sizeof(*lData));
		if(lData == NULL) 
		{
			return -1;
		}

		//CZLog(CZLogLevelLow, "Alloc host pageable for %s.", info->deviceName);

		lData->memHostPage = (void*)malloc(CZ_COPY_BUF_SIZE);
		if(lData->memHostPage == NULL) 
		{
			free(lData);
			return -1;
		}

		//CZLog(CZLogLevelLow, "Host pageable is at 0x%08X.", lData->memHostPage);

		//CZLog(CZLogLevelLow, "Alloc host pinned for %s.", info->deviceName);

		CZ_CUDA_CALL(cudaMallocHost((void**)&lData->memHostPin, CZ_COPY_BUF_SIZE),
			free(lData->memHostPage);
			free(lData);
			return -1);

		//CZLog(CZLogLevelLow, "Host pinned is at 0x%08X.", lData->memHostPin);

		//CZLog(CZLogLevelLow, "Alloc device buffer 1 for %s.", info->deviceName);

		CZ_CUDA_CALL(cudaMalloc((void**)&lData->memDevice1, CZ_COPY_BUF_SIZE),
			cudaFreeHost(lData->memHostPin);
			free(lData->memHostPage);
			free(lData);
			return -1);

		//CZLog(CZLogLevelLow, "Device buffer 1 is at 0x%08X.", lData->memDevice1);

		//CZLog(CZLogLevelLow, "Alloc device buffer 2 for %s.", info->deviceName);

		CZ_CUDA_CALL(cudaMalloc((void**)&lData->memDevice2, CZ_COPY_BUF_SIZE),
			cudaFree(lData->memDevice1);
			cudaFreeHost(lData->memHostPin);
			free(lData->memHostPage);
			free(lData);
			return -1);

		//CZLog(CZLogLevelLow, "Device buffer 2 is at 0x%08X.", lData->memDevice2);

		info->band.localData = (void*)lData;
	}

	return 0;
}

/*!	\brief Free buffers for bandwidth calculations.
 * 	\return \a 0 in case of success, \a -1 in case of error.
 * 	*/
static int CZCudaCalcDeviceBandwidthFree(
	struct CZDeviceInfo *info	/*!<[in,out] CUDA-device information. */
) 
{
	CZDeviceInfoBandLocalData *lData;

	if(info == NULL)
		return -1;

	lData = (CZDeviceInfoBandLocalData*)info->band.localData;
	if(lData != NULL) 
	{

		//CZLog(CZLogLevelLow, "Free host pageable for %s.", info->deviceName);

		if(lData->memHostPage != NULL)
			free(lData->memHostPage);

		//CZLog(CZLogLevelLow, "Free host pinned for %s.", info->deviceName);

		if(lData->memHostPin != NULL)
			cudaFreeHost(lData->memHostPin);

		//CZLog(CZLogLevelLow, "Free device buffer 1 for %s.", info->deviceName);

		if(lData->memDevice1 != NULL)
			cudaFree(lData->memDevice1);

		//CZLog(CZLogLevelLow, "Free device buffer 2 for %s.", info->deviceName);

		if(lData->memDevice2 != NULL)
			cudaFree(lData->memDevice2);

		//CZLog(CZLogLevelLow, "Free local buffers for %s.", info->deviceName);

		free(lData);
	}
	info->band.localData = NULL;

	return 0;
}

/*!	\brief Reset results of bandwidth calculations.
 * 	\return \a 0 in case of success, \a -1 in case of error.
 * 	*/
static int CZCudaCalcDeviceBandwidthReset(
	struct CZDeviceInfo *info	/*!<[in,out] CUDA-device information. */
) {

	if(info == NULL)
		return -1;

	info->band.copyHDPage = 0;
	info->band.copyHDPin = 0;
	info->band.copyDHPage = 0;
	info->band.copyDHPin = 0;
	info->band.copyDD = 0;

	return 0;
}

#define CZ_COPY_MODE_H2D	0	/*!< Host to device data copy mode. */
#define CZ_COPY_MODE_D2H	1	/*!< Device to host data copy mode. */
#define CZ_COPY_MODE_D2D	2	/*!< Device to device data copy mode. */

/*!	\brief Run data transfer bandwidth tests.
 * 	\return \a 0 in case of success, \a other is value in KiB/s.
 * 	*/
static float CZCudaCalcDeviceBandwidthTestCommon (
	struct CZDeviceInfo *info,	/*!<[in,out] CUDA-device information. */
	int mode,			/*!<[in] Run bandwidth test in one of modes. */
	int pinned			/*!<[in] Use pinned \a (=1) memory buffer instead of pagable \a (=0). */
) 
{
	CZDeviceInfoBandLocalData *lData;
	float timeMs = 0.0;
	float bandwidthKiBs = 0.0;
	cudaEvent_t start;
	cudaEvent_t stop;
	void *memHost;
	void *memDevice1;
	void *memDevice2;
	int i;

	if(info == NULL)
		return 0;

	CZ_CUDA_CALL(cudaEventCreate(&start),
		return 0);

	CZ_CUDA_CALL(cudaEventCreate(&stop),
		cudaEventDestroy(start);
		return 0);

	lData = (CZDeviceInfoBandLocalData*)info->band.localData;

	memHost = pinned? lData->memHostPin: lData->memHostPage;
	memDevice1 = lData->memDevice1;
	memDevice2 = lData->memDevice2;

	/*CZLog(CZLogLevelLow, "Starting %s test (%s) on %s.",
		(mode == CZ_COPY_MODE_H2D)? "host to device":
		(mode == CZ_COPY_MODE_D2H)? "device to host":
		(mode == CZ_COPY_MODE_D2D)? "device to device": "unknown",
		pinned? "pinned": "pageable",
		info->deviceName);*/

	for(i = 0; i < CZ_COPY_LOOPS_NUM; i++) {

		float loopMs = 0.0;

		CZ_CUDA_CALL(cudaEventRecord(start, 0),
			cudaEventDestroy(start);
			cudaEventDestroy(stop);
			return 0);

		switch(mode) {
		case CZ_COPY_MODE_H2D:
			CZ_CUDA_CALL(cudaMemcpy(memDevice1, memHost, CZ_COPY_BUF_SIZE, cudaMemcpyHostToDevice),
				cudaEventDestroy(start);
				cudaEventDestroy(stop);
				return 0);
			break;

		case CZ_COPY_MODE_D2H:
			CZ_CUDA_CALL(cudaMemcpy(memHost, memDevice2, CZ_COPY_BUF_SIZE, cudaMemcpyDeviceToHost),
				cudaEventDestroy(start);
				cudaEventDestroy(stop);
				return 0);
			break;

		case CZ_COPY_MODE_D2D:
			CZ_CUDA_CALL(cudaMemcpy(memDevice2, memDevice1, CZ_COPY_BUF_SIZE, cudaMemcpyDeviceToDevice),
				cudaEventDestroy(start);
				cudaEventDestroy(stop);
				return 0);
			break;

		default: // WTF!
			cudaEventDestroy(start);
			cudaEventDestroy(stop);
			return 0;
		}

		CZ_CUDA_CALL(cudaEventRecord(stop, 0),
			cudaEventDestroy(start);
			cudaEventDestroy(stop);
			return 0);

		CZ_CUDA_CALL(cudaEventSynchronize(stop),
			cudaEventDestroy(start);
			cudaEventDestroy(stop);
			return 0);

		CZ_CUDA_CALL(cudaEventElapsedTime(&loopMs, start, stop),
			cudaEventDestroy(start);
			cudaEventDestroy(stop);
			return 0);

		timeMs += loopMs;
	}

	//CZLog(CZLogLevelLow, "Test complete in %f ms.", timeMs);

	bandwidthKiBs = (
		1000 *
		(float)CZ_COPY_BUF_SIZE *
		(float)CZ_COPY_LOOPS_NUM
	) / (
		timeMs *
		(float)(1 << 10)
	);

	cudaEventDestroy(start);
	cudaEventDestroy(stop);

	return bandwidthKiBs;
}

/*!	\brief Run several bandwidth tests.
 * 	\return \a 0 in case of success, \a -1 in case of error.
 * 	*/
static int CZCudaCalcDeviceBandwidthTest(
	struct CZDeviceInfo *info	/*!<[in,out] CUDA-device information. */
) 
{

	info->band.copyHDPage = CZCudaCalcDeviceBandwidthTestCommon(info, CZ_COPY_MODE_H2D, 0);
	info->band.copyHDPin = CZCudaCalcDeviceBandwidthTestCommon(info, CZ_COPY_MODE_H2D, 1);
	info->band.copyDHPage = CZCudaCalcDeviceBandwidthTestCommon(info, CZ_COPY_MODE_D2H, 0);
	info->band.copyDHPin = CZCudaCalcDeviceBandwidthTestCommon(info, CZ_COPY_MODE_D2H, 1);
	info->band.copyDD = CZCudaCalcDeviceBandwidthTestCommon(info, CZ_COPY_MODE_D2D, 0);

	return 0;
}

/*!	\brief Calculate bandwidth information about CUDA-device.
 * 	\return \a 0 in case of success, \a -1 in case of error.
 * 	*/
int CZCudaCalcDeviceBandwidth(
	struct CZDeviceInfo *info	/*!<[in,out] CUDA-device information. */
) {

	if(info == NULL)
		return -1;
		
	if (CZCudaCalcDeviceSelect(info) != 0)
		return -1;

	if(CZCudaCalcDeviceBandwidthReset(info) != 0)
		return -1;

	if(CZCudaCalcDeviceBandwidthAlloc(info) != 0)
		return -1;

	if(CZCudaCalcDeviceBandwidthTest(info) != 0)
		return -1;

	// CZCudaCalcDeviceBandwidthFree(info);
	return 0;
}

/*!	\brief Cleanup after test and bandwidth calculations.
 * 	\return \a 0 in case of success, \a -1 in case of error.
 * 	*/
int CZCudaCleanDevice(
	struct CZDeviceInfo *info	/*!<[in,out] CUDA-device information. */
) 
{

	if(info == NULL)
		return -1;

	if(CZCudaCalcDeviceBandwidthFree(info) != 0)
		return -1;

	return 0;
}

/*!	\brief Reset results of preformance calculations.
 * 	\return \a 0 in case of success, \a -1 in case of error.
 * 	*/
static int CZCudaCalcDevicePerformanceReset(
	struct CZDeviceInfo *info	/*!<[in,out] CUDA-device information. */
) 
{

	if(info == NULL)
		return -1;

	info->perf.calcFloat = 0;
	info->perf.calcDouble = 0;
	info->perf.calcInteger32 = 0;
	info->perf.calcInteger24 = 0;
	info->perf.calcInteger64 = 0;

	return 0;
}

/*!	\brief 16 MAD instructions for float point test.
 * */
#define CZ_CALC_FMAD_16(a, b) \
	a = a * a + a; b = b * b + b; a = a * a + a; b = b * b + b; \
	a = a * a + a; b = b * b + b; a = a * a + a; b = b * b + b; \
	a = a * a + a; b = b * b + b; a = a * a + a; b = b * b + b; \
	a = a * a + a; b = b * b + b; a = a * a + a; b = b * b + b; \

/*!	\brief 256 MAD instructions for float point test.
 * */
#define CZ_CALC_FMAD_256(a, b) \
	CZ_CALC_FMAD_16(a, b) CZ_CALC_FMAD_16(a, b) \
	CZ_CALC_FMAD_16(a, b) CZ_CALC_FMAD_16(a, b) \
	CZ_CALC_FMAD_16(a, b) CZ_CALC_FMAD_16(a, b) \
	CZ_CALC_FMAD_16(a, b) CZ_CALC_FMAD_16(a, b) \
	CZ_CALC_FMAD_16(a, b) CZ_CALC_FMAD_16(a, b) \
	CZ_CALC_FMAD_16(a, b) CZ_CALC_FMAD_16(a, b) \
	CZ_CALC_FMAD_16(a, b) CZ_CALC_FMAD_16(a, b) \
	CZ_CALC_FMAD_16(a, b) CZ_CALC_FMAD_16(a, b) \

/*!	\brief 16 DMAD instructions for double-precision test.
 * */
#define CZ_CALC_DFMAD_16(a, b) \
	a = a * a + a; b = b * b + b; a = a * a + a; b = b * b + b; \
	a = a * a + a; b = b * b + b; a = a * a + a; b = b * b + b; \
	a = a * a + a; b = b * b + b; a = a * a + a; b = b * b + b; \
	a = a * a + a; b = b * b + b; a = a * a + a; b = b * b + b; \

/*	a = fma(a, a, a); b = fma(b, b, b); a = fma(a, a, a); b = fma(b, b, b); \
 *		a = fma(a, a, a); b = fma(b, b, b); a = fma(a, a, a); b = fma(b, b, b); \
 *			a = fma(a, a, a); b = fma(b, b, b); a = fma(a, a, a); b = fma(b, b, b); \
 *				a = fma(a, a, a); b = fma(b, b, b); a = fma(a, a, a); b = fma(b, b, b); \*/

/*!	\brief 256 MAD instructions for float point test.
 * */
#define CZ_CALC_DFMAD_256(a, b) \
	CZ_CALC_DFMAD_16(a, b) CZ_CALC_DFMAD_16(a, b) \
	CZ_CALC_DFMAD_16(a, b) CZ_CALC_DFMAD_16(a, b) \
	CZ_CALC_DFMAD_16(a, b) CZ_CALC_DFMAD_16(a, b) \
	CZ_CALC_DFMAD_16(a, b) CZ_CALC_DFMAD_16(a, b) \
	CZ_CALC_DFMAD_16(a, b) CZ_CALC_DFMAD_16(a, b) \
	CZ_CALC_DFMAD_16(a, b) CZ_CALC_DFMAD_16(a, b) \
	CZ_CALC_DFMAD_16(a, b) CZ_CALC_DFMAD_16(a, b) \
	CZ_CALC_DFMAD_16(a, b) CZ_CALC_DFMAD_16(a, b) \

/*!	\brief 16 MAD instructions for 32-bit integer test.
 * */
#define CZ_CALC_IMAD32_16(a, b) \
	a = a * a + a; b = b * b + b; a = a * a + a; b = b * b + b; \
	a = a * a + a; b = b * b + b; a = a * a + a; b = b * b + b; \
	a = a * a + a; b = b * b + b; a = a * a + a; b = b * b + b; \
	a = a * a + a; b = b * b + b; a = a * a + a; b = b * b + b; \

/*!	\brief 256 MAD instructions for 32-bit integer test.
 * */
#define CZ_CALC_IMAD32_256(a, b) \
	CZ_CALC_IMAD32_16(a, b) CZ_CALC_IMAD32_16(a, b) \
	CZ_CALC_IMAD32_16(a, b) CZ_CALC_IMAD32_16(a, b) \
	CZ_CALC_IMAD32_16(a, b) CZ_CALC_IMAD32_16(a, b) \
	CZ_CALC_IMAD32_16(a, b) CZ_CALC_IMAD32_16(a, b) \
	CZ_CALC_IMAD32_16(a, b) CZ_CALC_IMAD32_16(a, b) \
	CZ_CALC_IMAD32_16(a, b) CZ_CALC_IMAD32_16(a, b) \
	CZ_CALC_IMAD32_16(a, b) CZ_CALC_IMAD32_16(a, b) \
	CZ_CALC_IMAD32_16(a, b) CZ_CALC_IMAD32_16(a, b) \

/*!	\brief 16 MAD instructions for 64-bit integer test.
 * */
#define CZ_CALC_IMAD64_16(a, b) \
	a = a * a + a; b = b * b + b; a = a * a + a; b = b * b + b; \
	a = a * a + a; b = b * b + b; a = a * a + a; b = b * b + b; \
	a = a * a + a; b = b * b + b; a = a * a + a; b = b * b + b; \
	a = a * a + a; b = b * b + b; a = a * a + a; b = b * b + b; \

/*!	\brief 256 MAD instructions for 64-bit integer test.
 * */
#define CZ_CALC_IMAD64_256(a, b) \
	CZ_CALC_IMAD64_16(a, b) CZ_CALC_IMAD64_16(a, b) \
	CZ_CALC_IMAD64_16(a, b) CZ_CALC_IMAD64_16(a, b) \
	CZ_CALC_IMAD64_16(a, b) CZ_CALC_IMAD64_16(a, b) \
	CZ_CALC_IMAD64_16(a, b) CZ_CALC_IMAD64_16(a, b) \
	CZ_CALC_IMAD64_16(a, b) CZ_CALC_IMAD64_16(a, b) \
	CZ_CALC_IMAD64_16(a, b) CZ_CALC_IMAD64_16(a, b) \
	CZ_CALC_IMAD64_16(a, b) CZ_CALC_IMAD64_16(a, b) \
	CZ_CALC_IMAD64_16(a, b) CZ_CALC_IMAD64_16(a, b) \

/*!	\brief 16 MAD instructions for 24-bit integer test.
 * */
#define CZ_CALC_IMAD24_16(a, b) \
	a = __mul24(a, a) + a; b = __mul24(b, b) + b; \
	a = __mul24(a, a) + a; b = __mul24(b, b) + b; \
	a = __mul24(a, a) + a; b = __mul24(b, b) + b; \
	a = __mul24(a, a) + a; b = __mul24(b, b) + b; \
	a = __mul24(a, a) + a; b = __mul24(b, b) + b; \
	a = __mul24(a, a) + a; b = __mul24(b, b) + b; \
	a = __mul24(a, a) + a; b = __mul24(b, b) + b; \
	a = __mul24(a, a) + a; b = __mul24(b, b) + b; \

/*!	\brief 256 MAD instructions for 24-bit integer test.
 * */
#define CZ_CALC_IMAD24_256(a, b) \
	CZ_CALC_IMAD24_16(a, b) CZ_CALC_IMAD24_16(a, b)\
	CZ_CALC_IMAD24_16(a, b) CZ_CALC_IMAD24_16(a, b)\
	CZ_CALC_IMAD24_16(a, b) CZ_CALC_IMAD24_16(a, b)\
	CZ_CALC_IMAD24_16(a, b) CZ_CALC_IMAD24_16(a, b)\
	CZ_CALC_IMAD24_16(a, b) CZ_CALC_IMAD24_16(a, b)\
	CZ_CALC_IMAD24_16(a, b) CZ_CALC_IMAD24_16(a, b)\
	CZ_CALC_IMAD24_16(a, b) CZ_CALC_IMAD24_16(a, b)\
	CZ_CALC_IMAD24_16(a, b) CZ_CALC_IMAD24_16(a, b)\

/*!	\brief 16 MAD instructions for 8-bit integer test.
 * */
#define CZ_CALC_IMAD8_16(a, b) \
	a = a * a + a; b = b * b + b; a = a * a + a; b = b * b + b; \
	a = a * a + a; b = b * b + b; a = a * a + a; b = b * b + b; \
	a = a * a + a; b = b * b + b; a = a * a + a; b = b * b + b; \
	a = a * a + a; b = b * b + b; a = a * a + a; b = b * b + b; \

/*!	\brief 256 MAD instructions for 8-bit integer test.
 * */
#define CZ_CALC_IMAD8_256(a, b) \
	CZ_CALC_IMAD8_16(a, b) CZ_CALC_IMAD8_16(a, b)\
	CZ_CALC_IMAD8_16(a, b) CZ_CALC_IMAD8_16(a, b)\
	CZ_CALC_IMAD8_16(a, b) CZ_CALC_IMAD8_16(a, b)\
	CZ_CALC_IMAD8_16(a, b) CZ_CALC_IMAD8_16(a, b)\
	CZ_CALC_IMAD8_16(a, b) CZ_CALC_IMAD8_16(a, b)\
	CZ_CALC_IMAD8_16(a, b) CZ_CALC_IMAD8_16(a, b)\
	CZ_CALC_IMAD8_16(a, b) CZ_CALC_IMAD8_16(a, b)\
	CZ_CALC_IMAD8_16(a, b) CZ_CALC_IMAD8_16(a, b)\

#define CZ_CALC_MODE_FLOAT	0	/*!< Single-precision float point test mode. */
#define CZ_CALC_MODE_DOUBLE	1	/*!< Double-precision float point test mode. */
#define CZ_CALC_MODE_INTEGER8	2	/*!< 8-bit integer test mode. */
#define CZ_CALC_MODE_INTEGER32	3	/*!< 32-bit integer test mode. */
#define CZ_CALC_MODE_INTEGER24	4	/*!< 24-bit integer test mode. */
#define CZ_CALC_MODE_INTEGER64	5	/*!< 64-bit integer test mode. */

/*!	\brief GPU code for float point test.
 * */
 __global__ void CZCudaCalcKernelFloat(
	void *buf			/*!<[in] Data buffer. */
) 
{
	int index = blockIdx.x * blockDim.x + threadIdx.x;
	float *arr = (float*)buf;
	float val1 = index;
	float val2 = arr[index];
	int i;

	for(i = 0; i < CZ_CALC_BLOCK_LOOPS; i++) {
		CZ_CALC_FMAD_256(val1, val2);
		CZ_CALC_FMAD_256(val1, val2);
		CZ_CALC_FMAD_256(val1, val2);
		CZ_CALC_FMAD_256(val1, val2);
		CZ_CALC_FMAD_256(val1, val2);
		CZ_CALC_FMAD_256(val1, val2);
		CZ_CALC_FMAD_256(val1, val2);
		CZ_CALC_FMAD_256(val1, val2);
		CZ_CALC_FMAD_256(val1, val2);
		CZ_CALC_FMAD_256(val1, val2);
		CZ_CALC_FMAD_256(val1, val2);
		CZ_CALC_FMAD_256(val1, val2);
		CZ_CALC_FMAD_256(val1, val2);
		CZ_CALC_FMAD_256(val1, val2);
		CZ_CALC_FMAD_256(val1, val2);
		CZ_CALC_FMAD_256(val1, val2);
	}

	arr[index] = val1 + val2;
}

/*!	\brief GPU code for double-precision test.
 * */
__global__ void CZCudaCalcKernelDouble(
	void *buf			/*!<[in] Data buffer. */
) {
	int index = blockIdx.x * blockDim.x + threadIdx.x;
	double *arr = (double*)buf;
	double val1 = index;
	double val2 = arr[index];
	int i;

	for(i = 0; i < CZ_CALC_BLOCK_LOOPS; i++) {
		CZ_CALC_DFMAD_256(val1, val2);
		CZ_CALC_DFMAD_256(val1, val2);
		CZ_CALC_DFMAD_256(val1, val2);
		CZ_CALC_DFMAD_256(val1, val2);
		CZ_CALC_DFMAD_256(val1, val2);
		CZ_CALC_DFMAD_256(val1, val2);
		CZ_CALC_DFMAD_256(val1, val2);
		CZ_CALC_DFMAD_256(val1, val2);
		CZ_CALC_DFMAD_256(val1, val2);
		CZ_CALC_DFMAD_256(val1, val2);
		CZ_CALC_DFMAD_256(val1, val2);
		CZ_CALC_DFMAD_256(val1, val2);
		CZ_CALC_DFMAD_256(val1, val2);
		CZ_CALC_DFMAD_256(val1, val2);
		CZ_CALC_DFMAD_256(val1, val2);
		CZ_CALC_DFMAD_256(val1, val2);
	}

	arr[index] = val1 + val2;
}

/*!	\brief GPU code for 8-bit integer test.
 * */
__global__ void CZCudaCalcKernelInteger8(
	void *buf			/*!<[in] Data buffer. */
) {
	int index = blockIdx.x * blockDim.x + threadIdx.x;
	char *arr = (char*)buf;
	int val1 = index;
	int val2 = arr[index];
	int i;

	for(i = 0; i < CZ_CALC_BLOCK_LOOPS; i++) {
		CZ_CALC_IMAD8_256(val1, val2);
		CZ_CALC_IMAD8_256(val1, val2);
		CZ_CALC_IMAD8_256(val1, val2);
		CZ_CALC_IMAD8_256(val1, val2);
		CZ_CALC_IMAD8_256(val1, val2);
		CZ_CALC_IMAD8_256(val1, val2);
		CZ_CALC_IMAD8_256(val1, val2);
		CZ_CALC_IMAD8_256(val1, val2);
		CZ_CALC_IMAD8_256(val1, val2);
		CZ_CALC_IMAD8_256(val1, val2);
		CZ_CALC_IMAD8_256(val1, val2);
		CZ_CALC_IMAD8_256(val1, val2);
		CZ_CALC_IMAD8_256(val1, val2);
		CZ_CALC_IMAD8_256(val1, val2);
		CZ_CALC_IMAD8_256(val1, val2);
		CZ_CALC_IMAD8_256(val1, val2);
	}

	arr[index] = val1 + val2;
}

/*!	\brief GPU code for 32-bit integer test.
 * */
__global__ void CZCudaCalcKernelInteger32(
	void *buf			/*!<[in] Data buffer. */
) {
	int index = blockIdx.x * blockDim.x + threadIdx.x;
	int *arr = (int*)buf;
	int val1 = index;
	int val2 = arr[index];
	int i;

	for(i = 0; i < CZ_CALC_BLOCK_LOOPS; i++) {
		CZ_CALC_IMAD32_256(val1, val2);
		CZ_CALC_IMAD32_256(val1, val2);
		CZ_CALC_IMAD32_256(val1, val2);
		CZ_CALC_IMAD32_256(val1, val2);
		CZ_CALC_IMAD32_256(val1, val2);
		CZ_CALC_IMAD32_256(val1, val2);
		CZ_CALC_IMAD32_256(val1, val2);
		CZ_CALC_IMAD32_256(val1, val2);
		CZ_CALC_IMAD32_256(val1, val2);
		CZ_CALC_IMAD32_256(val1, val2);
		CZ_CALC_IMAD32_256(val1, val2);
		CZ_CALC_IMAD32_256(val1, val2);
		CZ_CALC_IMAD32_256(val1, val2);
		CZ_CALC_IMAD32_256(val1, val2);
		CZ_CALC_IMAD32_256(val1, val2);
		CZ_CALC_IMAD32_256(val1, val2);
	}

	arr[index] = val1 + val2;
}

/*!	\brief GPU code for 24-bit integer test.
 * */
__global__ void CZCudaCalcKernelInteger24(
	void *buf			/*!<[in] Data buffer. */
) {
	int index = blockIdx.x * blockDim.x + threadIdx.x;
	int *arr = (int*)buf;
	int val1 = index;
	int val2 = arr[index];
	int i;

	for(i = 0; i < CZ_CALC_BLOCK_LOOPS; i++) {
		CZ_CALC_IMAD24_256(val1, val2);
		CZ_CALC_IMAD24_256(val1, val2);
		CZ_CALC_IMAD24_256(val1, val2);
		CZ_CALC_IMAD24_256(val1, val2);
		CZ_CALC_IMAD24_256(val1, val2);
		CZ_CALC_IMAD24_256(val1, val2);
		CZ_CALC_IMAD24_256(val1, val2);
		CZ_CALC_IMAD24_256(val1, val2);
		CZ_CALC_IMAD24_256(val1, val2);
		CZ_CALC_IMAD24_256(val1, val2);
		CZ_CALC_IMAD24_256(val1, val2);
		CZ_CALC_IMAD24_256(val1, val2);
		CZ_CALC_IMAD24_256(val1, val2);
		CZ_CALC_IMAD24_256(val1, val2);
		CZ_CALC_IMAD24_256(val1, val2);
		CZ_CALC_IMAD24_256(val1, val2);
	}

	arr[index] = val1 + val2;
}

/*!	\brief GPU code for 64-bit integer test.
 * */
__global__ void CZCudaCalcKernelInteger64(
	void *buf			/*!<[in] Data buffer. */
) {
	int index = blockIdx.x * blockDim.x + threadIdx.x;
	long long *arr = (long long*)buf;
	long long val1 = index;
	long long val2 = arr[index];
	int i;

	for(i = 0; i < CZ_CALC_BLOCK_LOOPS; i++) {
		CZ_CALC_IMAD64_256(val1, val2);
		CZ_CALC_IMAD64_256(val1, val2);
		CZ_CALC_IMAD64_256(val1, val2);
		CZ_CALC_IMAD64_256(val1, val2);
		CZ_CALC_IMAD64_256(val1, val2);
		CZ_CALC_IMAD64_256(val1, val2);
		CZ_CALC_IMAD64_256(val1, val2);
		CZ_CALC_IMAD64_256(val1, val2);
		CZ_CALC_IMAD64_256(val1, val2);
		CZ_CALC_IMAD64_256(val1, val2);
		CZ_CALC_IMAD64_256(val1, val2);
		CZ_CALC_IMAD64_256(val1, val2);
		CZ_CALC_IMAD64_256(val1, val2);
		CZ_CALC_IMAD64_256(val1, val2);
		CZ_CALC_IMAD64_256(val1, val2);
		CZ_CALC_IMAD64_256(val1, val2);
	}

	arr[index] = val1 + val2;
}

/*!	\brief Run GPU calculation performace tests.
 * 	\return \a 0 in case of success, \a -1 in case of error.
 * 	*/
static float CZCudaCalcDevicePerformanceTest(
	struct CZDeviceInfo *info,	/*!<[in,out] CUDA-device information. */
	int mode			/*!<[in] Run performance test in one of modes. */
) {
	CZDeviceInfoBandLocalData *lData;
	float timeMs = 0.0;
	float performanceKOPs = 0.0;
	cudaEvent_t start;
	cudaEvent_t stop;
	int blocksNum = info->heavyMode? info->core.muliProcCount: 1;
	int i;

	if(info == NULL)
		return 0;

	CZ_CUDA_CALL(cudaEventCreate(&start),
		return 0);

	CZ_CUDA_CALL(cudaEventCreate(&stop),
		cudaEventDestroy(start);
		return 0);

	if (info->band.localData == NULL)
	{
		lData = (CZDeviceInfoBandLocalData*)malloc(sizeof(CZDeviceInfoBandLocalData));
		if(lData == NULL) 
		{
			return 0;
		}
		memset(lData, 0, sizeof(CZDeviceInfoBandLocalData));
		info->band.localData = lData;
	}
	if (lData->memDevice1 == NULL)
	{
		CZ_CUDA_CALL(cudaMalloc((void**)&lData->memDevice1, CZ_COPY_BUF_SIZE),
			free(lData);
			return 0);
	}

	int threadsNum = info->core.maxThreadsPerBlock;
	if(threadsNum == 0) {
		int warpSize = info->core.SIMDWidth;
		if(warpSize == 0)
			warpSize = CZ_DEF_WARP_SIZE;
		threadsNum = warpSize * 2;
		if(threadsNum > CZ_DEF_THREADS_MAX)
			threadsNum = CZ_DEF_THREADS_MAX;
	}

/*	CZLog(CZLogLevelLow, "Starting %s test on %s on %d block(s) %d thread(s) each.",
		(mode == CZ_CALC_MODE_FLOAT)? "single-precision float":
		(mode == CZ_CALC_MODE_DOUBLE)? "double-precision float":
		(mode == CZ_CALC_MODE_INTEGER8)? "8-bit integer":
		(mode == CZ_CALC_MODE_INTEGER32)? "32-bit integer":
		(mode == CZ_CALC_MODE_INTEGER24)? "24-bit integer":
		(mode == CZ_CALC_MODE_INTEGER64)? "64-bit integer": "unknown",
		info->deviceName,
		blocksNum,
		threadsNum);*/

	for(i = 0; i < CZ_CALC_LOOPS_NUM; i++) {

		float loopMs = 0.0;

		CZ_CUDA_CALL(cudaEventRecord(start, 0),
			cudaEventDestroy(start);
			cudaEventDestroy(stop);
			return 0);

		switch(mode) {
		case CZ_CALC_MODE_FLOAT:
			CZCudaCalcKernelFloat<<<blocksNum, threadsNum>>>(lData->memDevice1);
			break;

		case CZ_CALC_MODE_DOUBLE:
			CZCudaCalcKernelDouble<<<blocksNum, threadsNum>>>(lData->memDevice1);
			break;

		case CZ_CALC_MODE_INTEGER8:
			CZCudaCalcKernelInteger8<<<blocksNum, threadsNum>>>(lData->memDevice1);
			break;

		case CZ_CALC_MODE_INTEGER32:
			CZCudaCalcKernelInteger32<<<blocksNum, threadsNum>>>(lData->memDevice1);
			break;

		case CZ_CALC_MODE_INTEGER24:
			CZCudaCalcKernelInteger24<<<blocksNum, threadsNum>>>(lData->memDevice1);
			break;

		case CZ_CALC_MODE_INTEGER64:
			CZCudaCalcKernelInteger64<<<blocksNum, threadsNum>>>(lData->memDevice1);
			break;

		default: // WTF!
			cudaEventDestroy(start);
			cudaEventDestroy(stop);
			return 0;
		}

		CZ_CUDA_CALL(cudaGetLastError(),
			cudaEventDestroy(start);
			cudaEventDestroy(stop);
			return 0);

		CZ_CUDA_CALL(cudaEventRecord(stop, 0),
			cudaEventDestroy(start);
			cudaEventDestroy(stop);
			return 0);

		CZ_CUDA_CALL(cudaEventSynchronize(stop),
			cudaEventDestroy(start);
			cudaEventDestroy(stop);
			return 0);

		CZ_CUDA_CALL(cudaEventElapsedTime(&loopMs, start, stop),
			cudaEventDestroy(start);
			cudaEventDestroy(stop);
			return 0);

		timeMs += loopMs;
	}

	//CZLog(CZLogLevelLow, "Test complete in %f ms.", timeMs);

	performanceKOPs = (
		(float)info->core.muliProcCount *
		(float)CZ_CALC_LOOPS_NUM *
		(float)threadsNum *
		(float)CZ_CALC_BLOCK_LOOPS *
		(float)CZ_CALC_OPS_NUM *
		(float)CZ_CALC_BLOCK_SIZE *
		(float)CZ_CALC_BLOCK_NUM
	) / (float)timeMs;

	cudaEventDestroy(start);
	cudaEventDestroy(stop);

	CZCudaCalcDeviceBandwidthFree(info);
	return performanceKOPs;
}

/*!	\brief Calculate performance information about CUDA-device.
 * 	\return \a 0 in case of success, \a -1 in case of error.
 * 	*/
int CZCudaCalcDevicePerformance(
	struct CZDeviceInfo *info	/*!<[in,out] CUDA-device information. */
) {

	if(info == NULL)
		return -1;
		
	if (CZCudaCalcDeviceSelect(info) != 0)
		return -1;
	
	if(CZCudaCalcDevicePerformanceReset(info) != 0)
		return -1;

	info->perf.calcFloat = CZCudaCalcDevicePerformanceTest(info, CZ_CALC_MODE_FLOAT);
	if(((info->major > 1)) ||
		((info->major == 1) && (info->minor >= 3)))
		info->perf.calcDouble = CZCudaCalcDevicePerformanceTest(info, CZ_CALC_MODE_DOUBLE);
	//info->perf.calcInteger8 = CZCudaCalcDevicePerformanceTest(info, CZ_CALC_MODE_INTEGER8);
	info->perf.calcInteger32 = CZCudaCalcDevicePerformanceTest(info, CZ_CALC_MODE_INTEGER32);
	info->perf.calcInteger24 = CZCudaCalcDevicePerformanceTest(info, CZ_CALC_MODE_INTEGER24);
	info->perf.calcInteger64 = CZCudaCalcDevicePerformanceTest(info, CZ_CALC_MODE_INTEGER64);

	return 0;
}

/*!	\brief Read information about a CUDA-device.
	\return \a 0 in case of success, \a -1 in case of error.
*/
int CZCudaReadDeviceInfo(
	struct CZDeviceInfo *info	/*!<[in,out] CUDA-device information. */
)
{
	cudaDeviceProp prop;

	if(info == NULL)
		return -1;

	if(info->num >= CZCudaDeviceFound())
		return -1;

	CZ_CUDA_CALL(cudaGetDeviceProperties(&prop, info->num),
		return -1);

	strcpy(info->deviceName, prop.name);
	info->major = prop.major;
	info->minor = prop.minor;

	info->core.regsPerBlock = prop.regsPerBlock;
	info->core.regsPerMultipro = prop.regsPerMultiprocessor;
	info->core.SIMDWidth = prop.warpSize;
	info->core.maxThreadsPerBlock = prop.maxThreadsPerBlock;
	info->core.maxThreadsDim[0] = prop.maxThreadsDim[0];
	info->core.maxThreadsDim[1] = prop.maxThreadsDim[1];
	info->core.maxThreadsDim[2] = prop.maxThreadsDim[2];
	info->core.maxGridSize[0] = prop.maxGridSize[0];
	info->core.maxGridSize[1] = prop.maxGridSize[1];
	info->core.maxGridSize[2] = prop.maxGridSize[2];
	info->core.clockRate = prop.clockRate/1000;
	info->core.muliProcCount = prop.multiProcessorCount;
	info->core.kernelExecTimeoutEnabled= prop.kernelExecTimeoutEnabled;
	info->core.integratedGpu = prop.integrated;
	info->core.concurrentKernels = prop.concurrentKernels;
	info->core.computeMode =
		(prop.computeMode == cudaComputeModeDefault)? CZComputeModeDefault:
		(prop.computeMode == cudaComputeModeExclusive)? CZComputeModeExclusive:
		(prop.computeMode == cudaComputeModeProhibited)? CZComputeModeProhibited:
		CZComputeModeUnknown;
	info->core.pciBusID = prop.pciBusID;
	info->core.pciDeviceID = prop.pciDeviceID;
	info->core.pciDomainID = prop.pciDomainID;
	info->core.maxThreadsPerMultiProcessor = prop.maxThreadsPerMultiProcessor;
	info->core.cudaCores = ConvertSMVer2Cores(prop.major, prop.minor) * prop.multiProcessorCount;
	info->core.streamPrioritiesSupported = prop.streamPrioritiesSupported;

	info->mem.totalGlobal = prop.totalGlobalMem;
	info->mem.sharedPerBlock = prop.sharedMemPerBlock;
	info->mem.sharedPerMultiProcessor = prop.sharedMemPerMultiprocessor;
	info->mem.maxPitch = prop.memPitch;
	info->mem.totalConst = prop.totalConstMem;
	info->mem.textureAlignment = prop.textureAlignment;
	info->mem.texture1D[0] = prop.maxTexture1D;
	info->mem.texture2D[0] = prop.maxTexture2D[0];
	info->mem.texture2D[1] = prop.maxTexture2D[1];
	info->mem.texture3D[0] = prop.maxTexture3D[0];
	info->mem.texture3D[1] = prop.maxTexture3D[1];
	info->mem.texture3D[2] = prop.maxTexture3D[2];
	info->mem.gpuOverlap = prop.deviceOverlap;
	info->mem.mapHostMemory = prop.canMapHostMemory;
	info->mem.errorCorrection = prop.ECCEnabled;
	info->mem.asyncEngineCount = prop.asyncEngineCount;
	info->mem.unifiedAddressing = prop.unifiedAddressing;
	info->mem.memoryClockRate = prop.memoryClockRate/1000;
	info->mem.memoryBusWidth = prop.memoryBusWidth;
	info->mem.l2CacheSize = prop.l2CacheSize;

	return 0;
}

void printInfo(CZDeviceInfo *info)
{
	char valueStr[30];
	char valueStrPf[30];
	printf("=====================================\n");
	printf("DeviceName[%d]:%s\n", info->num, info->deviceName);
	printf("************Core Info*************\n");
	printf("Compute Capability:%d.%d\n", info->major, info->minor);
	printf("Clock Rate:%d MHz\n", info->core.clockRate);
	printf("Multiprocessors:%d (%d Cores)\n", info->core.muliProcCount, info->core.cudaCores);
	printf("Cores Per Multiprocessor:%d\n", info->core.cudaCores/info->core.muliProcCount);
	printf("WarpSize:%d\n", info->core.SIMDWidth);
	printf("Max Threads Per Multiprocessor:%d\n", info->core.maxThreadsPerMultiProcessor);
	printf("Max Threads Per Block:%d\n", info->core.maxThreadsPerBlock);
	printf("Regs Per Block:%d\n", info->core.regsPerBlock);
	printf("maxThreadsDim:%dx%dx%d\n", info->core.maxThreadsDim[0], info->core.maxThreadsDim[1], info->core.maxThreadsDim[2]);
	printf("maxGridSize:%dx%dx%d\n", info->core.maxGridSize[0], info->core.maxGridSize[1], info->core.maxGridSize[2]);
	printf("computeMode:%d\n", info->core.computeMode);
	printf("kernelExecTimeoutEnabled:%d\n", info->core.kernelExecTimeoutEnabled);
	printf("integratedGpu:%d\n", info->core.integratedGpu);
	printf("concurrentKernels:%d\n", info->core.concurrentKernels);
	printf("streamPrioritiesSupported:%d\n", info->core.streamPrioritiesSupported);
	printf("pciBusID:%d\n", info->core.pciBusID);
	printf("pciDeviceID:%d\n", info->core.pciDeviceID);
	printf("pciDomainID:%d\n", info->core.pciDomainID);
	printf("************Memory Info*************\n");
	getValue1024(info->mem.totalGlobal, valueStr);
	printf("totalGlobalMem:%s\n", valueStr);
	getValue1024(info->mem.totalConst, valueStr);
	printf("totalConstMem:%s\n", valueStr);
	getValue1024(info->mem.sharedPerBlock, valueStr);
	printf("sharedMemPerBlock:%s \n", valueStr);
	getValue1024(info->mem.sharedPerMultiProcessor, valueStr);
	printf("sharedMemPerMultiProcessor:%s\n", valueStr);
	getValue1024(info->mem.l2CacheSize, valueStr);
	printf("l2CacheSize:%s\n", valueStr);
	printf("memoryClockRate:%d MHz\n", info->mem.memoryClockRate);
	printf("memoryBusWidth:%d bits\n", info->mem.memoryBusWidth);
	getValue1024(info->mem.maxPitch, valueStr);
	printf("maxPitch:%s \n", valueStr);
	printf("textureAlignment:%d\n", info->mem.textureAlignment);
	printf("texture1D Size:%d\n", info->mem.texture1D[0]);
	printf("texture2D Size:%dx%d\n", info->mem.texture2D[0], info->mem.texture2D[1]);
	printf("texture3D Size:%dx%dx%d\n", info->mem.texture3D[0], info->mem.texture3D[1], info->mem.texture3D[2]);
	printf("errorCorrection:%d\n", info->mem.errorCorrection);
	printf("mapHostMemory:%d\n", info->mem.mapHostMemory);
	printf("unifiedAddressing:%d\n", info->mem.unifiedAddressing);
	printf("gpuOverlap:%d\n", info->mem.gpuOverlap);
	printf("asyncEngineCount:%d\n", info->mem.asyncEngineCount);
	printf("************Performace Info*************\n");
	printf("MemoryCopy		Pinned		Pageable\n");
	getValue1024(info->band.copyHDPin, valueStr, 1, "B/s");
	getValue1024(info->band.copyHDPage, valueStrPf, 1, "B/s");
	printf("HostToDevice	%s			%s\n", valueStr, valueStrPf);
	getValue1024(info->band.copyDHPin, valueStr, 1, "B/s");;
	getValue1024(info->band.copyDHPage, valueStrPf, 1, "B/s");
	printf("DeviceToHost	%s			%s\n", valueStr, valueStrPf);
	getValue1024(info->band.copyDD, valueStr, 1, "B/s");
	printf("DeviceToDevice	%s\n", valueStr);
	printf("GPU Core Performace\n");
	getValue1024(info->perf.calcFloat, valueStr, 1, "flop/s");
	printf("Single-precision Float	%s\n", valueStr);
	getValue1024(info->perf.calcDouble, valueStr, 1, "flop/s");
	printf("Double-precision Float	%s\n", valueStr);
	getValue1024(info->perf.calcInteger64, valueStr, 1, "iop/s");
	printf("64-bit Integer			%s\n", valueStr);
	getValue1024(info->perf.calcInteger32, valueStr, 1, "iop/s");
	printf("32-bit Integer			%s\n", valueStr);
	getValue1024(info->perf.calcInteger24, valueStr, 1, "iop/s");
	printf("24-bit Integer			%s\n", valueStr);
}
