//android
#include <jni.h>
#include <android/log.h>
#include <android/bitmap.h>
//opencv
#include <opencv2/opencv.hpp>

#if defined(__ANDROID__)
#define INNER_LOG(arg...) __android_log_print(ANDROID_LOG_INFO, "bitmap_wrapper", ##arg);
#define INNER_ERR(arg...) __android_log_print(ANDROID_LOG_ERROR, "bitmap_wrapper", ##arg);
#else
#define INNER_LOG(...)
#define INNER_ERR(...)
#endif //__ANDROID__

static cv::Mat jniGetBitmapData(JNIEnv * env, jobject bitmap)
{
	INNER_LOG("[jniGetBitmapData] reading bitmap info...");
	AndroidBitmapInfo bitmapInfo;
	uint32_t* storedBitmapPixels = nullptr;
	int ret = -1;
	if ((ret = AndroidBitmap_getInfo(env, bitmap, &bitmapInfo)) < 0)
	{
		INNER_ERR("[jniGetBitmapData] AndroidBitmap_getInfo() failed ! error=%d", ret);
		return cv::Mat();
	}
	INNER_LOG("[jniGetBitmapData] width:%d height:%d stride:%d", bitmapInfo.width, bitmapInfo.height, bitmapInfo.stride);
	if (bitmapInfo.format != ANDROID_BITMAP_FORMAT_RGBA_8888)
	{
		INNER_ERR("[jniGetBitmapData] Bitmap format is not RGBA_8888!");
		return cv::Mat();
	}
	INNER_LOG("[jniGetBitmapData] reading bitmap pixels...");
	void* bitmapPixels = nullptr;
	if ((ret = AndroidBitmap_lockPixels(env, bitmap, &bitmapPixels)) < 0)
	{
		INNER_ERR("[jniGetBitmapData] AndroidBitmap_lockPixels() failed ! error=%d", ret);
		return cv::Mat();
	}
	INNER_LOG("[jniGetBitmapData] read bitmap pixels success.");
	cv::Mat resultMat(cv::Size(bitmapInfo.width, bitmapInfo.height), CV_8UC4);
	const int pixelsCount = bitmapInfo.height * bitmapInfo.width;
	memcpy(resultMat.data, bitmapPixels, sizeof(uint32_t) * pixelsCount);
	AndroidBitmap_unlockPixels(env, bitmap);
	INNER_LOG("[jniGetBitmapData] all success.");
	return resultMat;
}
//check if little edian
static bool isLittleEdianCPU()
{
	union
	{
		int x;
		char y;
	}value;
	value.x = 1;
	return(value.y == 1);
}
static jobject jniNewBitmapFromMat(JNIEnv * env, cv::Mat srcImg)
{
	assert(srcImg.type() == CV_8UC1 || srcImg.type() == CV_8UC3 || srcImg.type() == CV_8UC4);
	assert(srcImg.data);
	INNER_LOG("[jniNewBitmapFromMat] width:%d height:%d type:%d", srcImg.cols, srcImg.rows, srcImg.type());
	jclass bitmapCls = env->FindClass("android/graphics/Bitmap");
	jmethodID createBitmapFunction = env->GetStaticMethodID(bitmapCls, "createBitmap", "(IILandroid/graphics/Bitmap$Config;)Landroid/graphics/Bitmap;");
	jstring configName = env->NewStringUTF("ARGB_8888");
	jclass bitmapConfigClass = env->FindClass("android/graphics/Bitmap$Config");
	jmethodID valueOfBitmapConfigFunction = env->GetStaticMethodID(bitmapConfigClass, "valueOf", "(Ljava/lang/String;)Landroid/graphics/Bitmap$Config;");
	jobject bitmapConfig = env->CallStaticObjectMethod(bitmapConfigClass, valueOfBitmapConfigFunction, configName);
	jobject newBitmap = env->CallStaticObjectMethod(bitmapCls, createBitmapFunction, srcImg.cols, srcImg.rows, bitmapConfig);
	int ret = -1;
	void* bitmapPixels = nullptr;
	if ((ret = AndroidBitmap_lockPixels(env, newBitmap, &bitmapPixels)) < 0)
	{
		INNER_ERR("[jniNewBitmapFromMat] AndroidBitmap_lockPixels() failed ! error=%d", ret);
		return nullptr;
	}
	uint32_t* newBitmapPixels = (uint32_t*)bitmapPixels;
	int pixelsCount = srcImg.cols * srcImg.rows;
	//dst pixel format : bgra
	switch (srcImg.type())
	{
	case CV_8UC1:
		if (isLittleEdianCPU())
		{
			for (size_t i = 0; i < pixelsCount; i++)
			{			
				memset(&(newBitmapPixels[i]), srcImg.data[i], 3);
			}
		}
		else
		{
			for (size_t i = 0; i < pixelsCount; i++)
			{
				memset(&(newBitmapPixels[i]) + 1, srcImg.data[i], 3);
			}
		}
		
		break;
	case CV_8UC3:
		if (isLittleEdianCPU())
		{
			for (size_t i = 0; i < pixelsCount; i++)
			{
				const int srcPixel = srcImg.data[i];
				memcpy(&(newBitmapPixels[i]), &srcPixel, 3);
			}
		}
		else
		{
			for (size_t i = 0; i < pixelsCount; i++)
			{
				const int srcPixel = srcImg.data[i];
				memcpy(&(newBitmapPixels[i])+1, &srcPixel+1, 3);
			}
		}
		break;
	case CV_8UC4:
		memcpy(newBitmapPixels, srcImg.data, sizeof(uint32_t) * pixelsCount);
		break;
	default:
		break;
	}
	AndroidBitmap_unlockPixels(env, newBitmap);
	INNER_LOG("[jniNewBitmapFromMat] returning the new bitmap");
	return newBitmap;
}
