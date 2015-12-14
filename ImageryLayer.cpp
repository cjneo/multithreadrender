//hewensong

#include "ImageryLayer.h"

#include <typeinfo>
#include "Geometry.h"
#include "GeometryAttributes.h"
#include "ReprojectWebMercatorFS.h"
#include "ReprojectWebMercatorVS.h"
#include "Cartesian4.h"
#include "Cartesian2.h"
#include "TileImagery.h"
#include "GlobeSurfaceTile.h"
#include "Image.h"
#include "Imagery.h"
#include "ImageryProvider.h"
#include "Context.h"
#include "Cartographic.h"
#include "TilingScheme.h"
#include "Rectangle.h"
#include "GeographicTilingScheme.h"
#include "QuadtreeTile.h"
#include "TerrainProvider.h"
#include "PixelFormat.h"
#include "When.h"
#include "PromiseType.h"
#include "JSImage.h"
#include "TileDiscardPolicy.h"
//#include "Array.h"
#include<iostream>
#include<iomanip>
#include<vector>
#include <GLES2/gl2.h>
namespace World {

void GetTexFromFrameBuffer(GLuint fboname){
	//Bind the FBO
glBindFramebuffer(GL_FRAMEBUFFER, fboname);
// set the viewport as the FBO won't be the same dimension as the screen
int width=256,height=256;
glViewport(0, 0, width, height);
std::vector< unsigned char > pixels ( width * height * 4 ,0);

glReadPixels(0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, &pixels[0]);
GLenum error = glGetError();
	printf("error: %x\n",error);
//Bind your main FBO again
glBindFramebuffer(GL_FRAMEBUFFER,0);
// set the viewport as the FBO won't be the same dimension as the screen
glViewport(0, 0, 250, 250);
for(int i=0;i<30;i++){
	for(int j=0;j<30;j++){
	int offset=(i*width+j)*4;
    //std::cout << "|" ;
   		std::cout<< (int)pixels[0+offset];
    std::cout   << (int)pixels[1+offset] ;
    std::cout   << (int)pixels[2+offset] ;
    std::cout  << (int)pixels[3+offset] ;
	//std::cout<<"|";
	}
    std::cout << std::endl;
}
}
 GLuint Texture2DUseFramBuffer(GLuint textureId )
{
   GLuint fboname;
   GLenum error = glGetError();
      	printf("gl begin: %x\n",error);
   glGenFramebuffers(1,&fboname);
    error = glGetError();
      	printf("glgenframebuffer error: %x\n",error);
   glBindFramebuffer(GL_FRAMEBUFFER,fboname);
   // Texture object handle
   error = glGetError();
         	printf("glbindframebuffer error: %x\n",error);
   glFramebufferTexture2D(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_TEXTURE_2D,textureId
,0);
    error = glGetError();
   	printf("glframe buffertexture error: %x\n",error);
GLenum status=glCheckFramebufferStatus(GL_FRAMEBUFFER);
  error = glGetError();
	printf("~gl check error: %x\n",error);
if(status!=GL_FRAMEBUFFER_COMPLETE){
   printf("failed to make complete framebuffer %x\n",status);
}
else{
	std::cout<<"it is ok"<<std::endl;
}
glBindFramebuffer(GL_FRAMEBUFFER,0);
  error = glGetError();
	printf("~error: %x\n",error);
GetTexFromFrameBuffer(fboname);
glDeleteFramebuffers(	1,&fboname);
   return textureId;

}
class ILUniformsType/*: public UniformMapTypeBase */{

public:
	static std::map<std::string, UniformMapTypeBase::uniformMap_void_Fun_Func*>* _prototype;
public:

	ILUniformsType() {

		_u_textureDimensions =
				(UniformMapTypeBase::uniformMap_void_Fun_Func) &ImageryLayer::_ILUniformsInstance::u_textureDimensions;
		_u_texture =
				(UniformMapTypeBase::uniformMap_void_Fun_Func) &ImageryLayer::_ILUniformsInstance::u_texture;
		_u_northLatitude =
				(UniformMapTypeBase::uniformMap_void_Fun_Func) &ImageryLayer::_ILUniformsInstance::u_northLatitude;
		_u_southLatitude =
				(UniformMapTypeBase::uniformMap_void_Fun_Func) &ImageryLayer::_ILUniformsInstance::u_southLatitude;
		_u_southMercatorYLow =
				(UniformMapTypeBase::uniformMap_void_Fun_Func) &ImageryLayer::_ILUniformsInstance::u_southMercatorYLow;
		_u_southMercatorYHigh =
				(UniformMapTypeBase::uniformMap_void_Fun_Func) &ImageryLayer::_ILUniformsInstance::u_southMercatorYHigh;
		_u_oneOverMercatorHeight =
				(UniformMapTypeBase::uniformMap_void_Fun_Func) &ImageryLayer::_ILUniformsInstance::u_oneOverMercatorHeight;
		if (ILUniformsType::_prototype == NULL) {

			_prototype = new std::map<std::string,
					UniformMapTypeBase::uniformMap_void_Fun_Func*>();

		}
		(*_prototype)["u_textureDimensions"] = &_u_textureDimensions;
		(*_prototype)["u_texture"] = &_u_texture;
		(*_prototype)["u_northLatitude"] = &_u_northLatitude;

		(*_prototype)["u_southLatitude"] = &_u_southLatitude;
		(*_prototype)["u_southMercatorYLow"] = &_u_southMercatorYLow;
		(*_prototype)["u_southMercatorYHigh"] = &_u_southMercatorYHigh;
		(*_prototype)["u_oneOverMercatorHeight"] = &_u_oneOverMercatorHeight;

	}

public:

	UniformMapTypeBase::uniformMap_void_Fun_Func _u_textureDimensions;
	UniformMapTypeBase::uniformMap_void_Fun_Func _u_texture;
	UniformMapTypeBase::uniformMap_void_Fun_Func _u_northLatitude;
	UniformMapTypeBase::uniformMap_void_Fun_Func _u_southLatitude;
	UniformMapTypeBase::uniformMap_void_Fun_Func _u_southMercatorYLow;
	UniformMapTypeBase::uniformMap_void_Fun_Func _u_southMercatorYHigh;
	UniformMapTypeBase::uniformMap_void_Fun_Func _u_oneOverMercatorHeight;

};

// initialise of uniformmap //chenjian add it
std::map<std::string, UniformMapTypeBase::uniformMap_void_Fun_Func*>* ILUniformsType::_prototype =
		new std::map<std::string, UniformMapTypeBase::uniformMap_void_Fun_Func*>();

ILUniformsType *ilUniformsType = new ILUniformsType();

/**An imagery layer that displays tiled image data from a single imagery provider on a Globe.
 [ const ImageryProvider& imageryProvider, const Object Rectangle Number function Number function Number function Number function Number function Number function Boolean Number Number Number& options rectangle alpha brightness contrast hue saturation gamma show maximumAnisotropy minimumTerrainLevel maximumTerrainLevel ]**/

Number ImageryLayer::DEFAULT_BRIGHTNESS = 1.0;
Number ImageryLayer::DEFAULT_CONTRAST = 1.0;
Number ImageryLayer::DEFAULT_HUE = 0.0;
Number ImageryLayer::DEFAULT_SATURATION = 1.0;
Number ImageryLayer::DEFAULT_GAMMA = 1.0;

Float32Array ImageryLayer::_float32ArrayScratch(1);

ImageryLayer::ImageryLayer_reproject::ImageryLayer_reproject() {
	_framebuffer = NULL;
	_vertexArray = NULL;
	_shaderProgram = NULL;
	_renderState = NULL;
}
ImageryLayer::ImageryLayer_reproject::~ImageryLayer_reproject() {
	if (_framebuffer) {
		delete (_framebuffer);
		_framebuffer = NULL;
	}
	if (_vertexArray) {
		delete (_vertexArray);
		_vertexArray = NULL;
	}
	if (_shaderProgram) {
		delete (_shaderProgram);
		_shaderProgram = NULL;
	}
}
ImageryLayer::ImageryLayer(ImageryProvider* imageryProvider, Options& options) {
	_imageryProvider = imageryProvider;  //yao modify ooo
	_alpha = (options._alpha != F64_MAX) ? options._alpha : 1.0;
	_brightness =
			(options._brightness != F64_MAX) ?
					options._brightness : ImageryLayer::DEFAULT_BRIGHTNESS;
	_contrast =
			(options._contrast != F64_MAX) ?
					options._contrast : ImageryLayer::DEFAULT_CONTRAST;

	_hue = (options._hue != F64_MAX) ? options._hue : ImageryLayer::DEFAULT_HUE;

	_saturation =
			(options._saturation != F64_MAX) ?
					options._saturation : ImageryLayer::DEFAULT_SATURATION;

	_gamma =
			(options._gamma != F64_MAX) ?
					options._gamma : ImageryLayer::DEFAULT_GAMMA;

	_show = (options._show != -1) ? options._show : true;

	_minimumTerrainLevel = options._minimumTerrainLevel;
	_maximumTerrainLevel = options._maximumTerrainLevel;

	_rectangle =
			(options._rectangle._east != F64_MAX) ?
					options._rectangle : Rectangle::_MAX_VALUE;

	_maximumAnisotropy = options._maximumAnisotropy;

	_mshow = false;

	Cartesian4 c4;
	_skeletonPlaceholder = new TileImagery(Imagery::createPlaceholder(this),
			c4);
	_skeletonPlaceholder->addReference();

	_layerIndex = -1;

	// true if this is the base (lowest shown) layer->
	_isBaseLayer = false;
	ILUniformMap = new _ILUniformsInstance(ilUniformsType->_prototype);
	//_requestImageError = NULL;
}

ImageryProvider* ImageryLayer::get_imageryProvider() {
	return _imageryProvider;
}

Rectangle* ImageryLayer::get_rectangle() {
	return &_rectangle;
}

/***********Methods start*************/
/***Destroys the WebGL resources held by this object. Destroying an object allows for deterministic release of WebGL resources, instead of relying on the garbage collector to destroy this object. Once an object is destroyed, it should not be used; calling any function other than isDestroyed will result in a DeveloperError exception. Therefore, assign the return value (NULL) to the object as done in the example. Returns: Throws: DeveloperError : This object was destroyed, i.e., destroy() was called. Example: imageryLayer = imageryLayer && imageryLayer.destroy(); See: ImageryLayer#isDestroyed Source: Scene/ImageryLayer.js, line 331***/
ImageryLayer::~ImageryLayer() {
	_skeletonPlaceholder->releaseReference();
}

/***Gets a value indicating whether this layer is the base layer in the ImageryLayerCollection. The base layer is the one that underlies all others. It is special in that it is treated as if it has global rectangle, even if it actually does not, by stretching the texels at the edges over the entire globe. Returns: true if this is the base layer; otherwise, false. Source: Scene/ImageryLayer.js, line 296***/
bool ImageryLayer::_createTileImagerySkeletons(QuadtreeTile* tile,
		TerrainProvider* terrainProvider, S32 insertionPoint) {

	GlobeSurfaceTile* surfaceTile = tile->_data;

	if (_minimumTerrainLevel != U32_MAX
			&& tile->_level < _minimumTerrainLevel) {
		return false;
	}
	if (_maximumTerrainLevel != U32_MAX
			&& tile->_level > _maximumTerrainLevel) {
		return false;
	}

	ImageryProvider* imageryProvider = this->_imageryProvider;

	if (insertionPoint < 0) {  //ooo ?
		insertionPoint = surfaceTile->_imagerys.size();
	}

	if (!_imageryProvider->get_ready()) {
		// The imagery provider is not ready, so we can't create skeletons, yet.
		// Instead, add a placeholder so that we'll know to create
		// the skeletons once the provider is ready.
		surfaceTile->_imagerys.reserve(surfaceTile->_imagerys.size() + 1);
		surfaceTile->_imagerys.insert(
				surfaceTile->_imagerys.begin() + insertionPoint,
				_skeletonPlaceholder);
		_skeletonPlaceholder->addReference();
		return true;
	}

	// Compute the rectangle of the imagery from this imageryProvider that overlaps
	// the geometry tile->_  The ImageryProvider and ImageryLayer both have the
	// opportunity to constrain the rectangle._  The imagery TilingScheme's rectangle
	// always fully contains the ImageryProvider's rectangle._
	Rectangle rectangle;
	Rectangle::intersectWith(tile->_rectangle,
			_imageryProvider->get_rectangle(), rectangle);
	//Rectangle::intersectWith(rectangle, Rectangle::_MAX_VALUE, rectangle);
	Rectangle::intersectWith(rectangle, this->_rectangle, rectangle);//yao add ooo
	if (rectangle._east <= rectangle._west
			|| rectangle._north <= rectangle._south) {
		// There is no overlap between this terrain tile and this imagery
		// provider.  Unless this is the base layer, no skeletons need to be created.
		// We stretch texels at the edge of the base layer over the entire globe.
		if (!isBaseLayer()) {
			return false;
		}

		Rectangle baseImageryRectangle;
		//Rectangle::intersectWith(*_imageryProvider->get_rectangle(),
		//		Rectangle::_MAX_VALUE, baseImageryRectangle);

		Rectangle::intersectWith(_imageryProvider->get_rectangle(),
				this->_rectangle, baseImageryRectangle);
		Rectangle& baseTerrainRectangle = tile->_rectangle;

		if (baseTerrainRectangle._south >= baseImageryRectangle._north) {
			rectangle._north = rectangle._south = baseImageryRectangle._north;
		} else if (baseTerrainRectangle._north <= baseImageryRectangle._south) {
			rectangle._north = rectangle._south = baseImageryRectangle._south;
		}

		if (baseTerrainRectangle._west >= baseImageryRectangle._east) {
			rectangle._west = rectangle._east = baseImageryRectangle._east;
		} else if (baseTerrainRectangle._east <= baseImageryRectangle._west) {
			rectangle._west = rectangle._east = baseImageryRectangle._west;
		}
	}

	F64 latitudeClosestToEquator = 0.0;
	if (rectangle._south > 0.0) {
		latitudeClosestToEquator = rectangle._south;
	} else if (rectangle._north < 0.0) {
		latitudeClosestToEquator = rectangle._north;
	}

	// Compute the required level in the imagery tiling scheme.
	// The errorRatio should really be imagerySSE / terrainSSE rather than this hard-coded value.
	// But first we need configurable imagery SSE and we need the rendering to be able to handle more
	// images attached to a terrain tile than there are available texture units.  So that's for the future.
	F64 errorRatio = 1.0;
	F64 targetGeometricError = errorRatio
			* terrainProvider->getLevelMaximumGeometricError(tile->_level);
	int imageryLevel = getLevelWithMaximumTexelSpacing(this,
			targetGeometricError, latitudeClosestToEquator);
	imageryLevel = std::max(0, imageryLevel);
	int maximumLevel = _imageryProvider->get_maximumLevel();
	if (maximumLevel != -1 && imageryLevel > maximumLevel) {
		imageryLevel = maximumLevel;
	}
	if (_imageryProvider->get_minimumLevel() != -1) {
		int minimumLevel = _imageryProvider->get_minimumLevel();
		if (imageryLevel < minimumLevel) {
			imageryLevel = minimumLevel;
		}
	}
	TilingScheme* imageryTilingScheme = _imageryProvider->get_tilingScheme();

	Cartographic northwestCartographic;
	Rectangle::northwest(rectangle, northwestCartographic);
	Cartesian2 northwestTileCoordinates;
	imageryTilingScheme->positionToTileXY(northwestCartographic, imageryLevel,
			northwestTileCoordinates);

	Cartographic southeastCartographic;
	Rectangle::southeast(rectangle, southeastCartographic);
	Cartesian2 southeastTileCoordinates;
	imageryTilingScheme->positionToTileXY(southeastCartographic, imageryLevel,
			southeastTileCoordinates);

	// If the southeast corner of the rectangle lies very close to the north or west side
	// of the southeast tile, we don't actually need the southernmost or easternmost
	// tiles.
	// Similarly, if the northwest corner of the rectangle lies very close to the south or east side
	// of the northwest tile, we don't actually need the northernmost or westernmost tiles.

	// We define "very close" as being within 1/512 of the width of the tile->_
	F64 veryCloseX = (tile->_rectangle._north - tile->_rectangle._south)
			/ 512.0;
	F64 veryCloseY = (tile->_rectangle._east - tile->_rectangle._west) / 512.0;

	Rectangle northwestTileRectangle;
	imageryTilingScheme->tileXYToRectangle(northwestTileCoordinates._x,
			northwestTileCoordinates._y, imageryLevel, northwestTileRectangle);
	if (std::abs(northwestTileRectangle._south - tile->_rectangle._north)
			< veryCloseY
			&& northwestTileCoordinates._y < southeastTileCoordinates._y) {
		++northwestTileCoordinates._y;
	}	//ooo ??
	if (std::abs(northwestTileRectangle._east - tile->_rectangle._west)
			< veryCloseX
			&& northwestTileCoordinates._x < southeastTileCoordinates._x) {
		++northwestTileCoordinates._x;
	}

	Rectangle southeastTileRectangle;
	imageryTilingScheme->tileXYToRectangle(southeastTileCoordinates._x,
			southeastTileCoordinates._y, imageryLevel, southeastTileRectangle);
	if (std::abs(southeastTileRectangle._north - tile->_rectangle._south)
			< veryCloseY
			&& southeastTileCoordinates._y > northwestTileCoordinates._y) {
		--southeastTileCoordinates._y;
	}
	if (std::abs(southeastTileRectangle._west - tile->_rectangle._east)
			< veryCloseX
			&& southeastTileCoordinates._x > northwestTileCoordinates._x) {
		--southeastTileCoordinates._x;
	}

	// Create TileImagery instances for each imagery tile overlapping this terrain tile->_
	// We need to do all texture coordinate computations in the imagery tile's tiling scheme.

	Rectangle& terrainRectangle = tile->_rectangle;
	Rectangle imageryRectangle;
	imageryTilingScheme->tileXYToRectangle(northwestTileCoordinates._x,
			northwestTileCoordinates._y, imageryLevel, imageryRectangle);

	F64 minU;
	F64 maxU = 0.0;

	F64 minV = 1.0;
	F64 maxV;

	// If this is the northern-most or western-most tile in the imagery tiling scheme,
	// it may not start at the northern or western edge of the terrain tile->_
	// Calculate where it does start.
	if (!isBaseLayer()
			&& std::abs(imageryRectangle._west - tile->_rectangle._west)
					>= veryCloseX) {
		maxU = std::min(1.0,
				(imageryRectangle._west - terrainRectangle._west)
						/ (terrainRectangle._east - terrainRectangle._west));
	}

	if (!isBaseLayer()
			&& std::abs(imageryRectangle._north - tile->_rectangle._north)
					>= veryCloseY) {
		minV = std::max(0.0,
				(imageryRectangle._north - terrainRectangle._south)
						/ (terrainRectangle._north - terrainRectangle._south));
	}

	F64 initialMinV = minV;

	for (int i = northwestTileCoordinates._x; i <= southeastTileCoordinates._x;
			i++) {
		minU = maxU;

		imageryTilingScheme->tileXYToRectangle(i, northwestTileCoordinates._y,
				imageryLevel, imageryRectangle);
		maxU = std::min(1.0,
				((imageryRectangle._east - terrainRectangle._west)
						/ (terrainRectangle._east - terrainRectangle._west)));

		// If this is the eastern-most imagery tile mapped to this terrain tile,
		// and there are more imagery tiles to the east of this one, the maxU
		// should be 1.0 to make sure rounding errors don't make the last
		// image fall shy of the edge of the terrain tile->_
		if (i == southeastTileCoordinates._x
				&& (isBaseLayer()
						|| std::abs(
								imageryRectangle._east - tile->_rectangle._east)
								< veryCloseX)) {
			maxU = 1.0;
		}

		minV = initialMinV;

		for (int j = northwestTileCoordinates._y;
				j <= southeastTileCoordinates._y; j++) {
			maxV = minV;

			imageryTilingScheme->tileXYToRectangle(i, j, imageryLevel,
					imageryRectangle);
			minV =
					std::max(0.0,
							(imageryRectangle._south - terrainRectangle._south)
									/ (terrainRectangle._north
											- terrainRectangle._south));

			// If this is the southern-most imagery tile mapped to this terrain tile,
			// and there are more imagery tiles to the south of this one, the minV
			// should be 0.0 to make sure rounding errors don't make the last
			// image fall shy of the edge of the terrain tile->_
			if (j == southeastTileCoordinates._y
					&& (isBaseLayer()
							|| std::abs(
									imageryRectangle._south
											- tile->_rectangle._south)
									< veryCloseY)) {
				minV = 0.0;
			}

			Cartesian4 texCoordsRectangle(minU, minV, maxU, maxV);
#if 0
			std::cout << texCoordsRectangle.toString();
			std::cout << "Image x=" << i << " y=" << j << " lv=" << imageryLevel
					<< std::endl;
#endif
			Imagery* imagery = getImageryFromCache(i, j, imageryLevel,
					imageryRectangle);
			TileImagery* tileImagery = new TileImagery(imagery, texCoordsRectangle);
			tileImagery->addReference();
			surfaceTile->_imagerys.reserve(surfaceTile->_imagerys.size() + 1);
			surfaceTile->_imagerys.insert(
					surfaceTile->_imagerys.begin() + insertionPoint, tileImagery);

			++insertionPoint;
		}
	}

	return true;
}

void ImageryLayer::_calculateTextureTranslationAndScale(QuadtreeTile* tile,
		TileImagery* tileImagery, Cartesian4& result) {
	Rectangle& imageryRectangle = tileImagery->_readyImagery->_rectangle;
	Rectangle& terrainRectangle = tile->_rectangle;
	F64 terrainWidth = terrainRectangle._east - terrainRectangle._west;
	F64 terrainHeight = terrainRectangle._north - terrainRectangle._south;

	F64 scaleX = terrainWidth
			/ (imageryRectangle._east - imageryRectangle._west);
	F64 scaleY = terrainHeight
			/ (imageryRectangle._north - imageryRectangle._south);
	result._x = scaleX * (terrainRectangle._west - imageryRectangle._west)
			/ terrainWidth;
	result._y = scaleY * (terrainRectangle._south - imageryRectangle._south)
			/ terrainHeight;
	result._z = scaleX;
	result._w = scaleY;
}

class ImageryLayer_loadImageSuccess: public When::TempleteForRegister {
public:
	Imagery* _imagery;
	ImageryProvider* _imageryProvider;
public:
	ImageryLayer_loadImageSuccess(Imagery* imagery,
			ImageryProvider* imageryProvider) :
			_imagery(imagery), _imageryProvider(imageryProvider) {
	}

	virtual Promise* call(PromiseOrValue *data) {
		Image* image = ((PromiseOrValueImage*) data)->_image->_image;
		_imagery->_image = image;
		_imagery->_state = ImageryState::RECEIVED;
		_imagery->releaseReference();

		//TileProviderError.handleSuccess(that._requestImageError);
		return NULL;
	}
};

class ImageryLayer_loadImageFailure: public When::TempleteForRegister {
public:
	Imagery* _imagery;
	ImageryProvider* _imageryProvider;
public:
	ImageryLayer_loadImageFailure(Imagery* imagery,
			ImageryProvider* imageryProvider) :
			_imagery(imagery), _imageryProvider(imageryProvider) {
	}

	virtual Promise* call(PromiseOrValue *data) {
		_imagery->_state = ImageryState::FAILED;
		_imagery->releaseReference();

		//var message = 'Failed to obtain image tile X: ' + imagery->_x + ' Y: ' + imagery->_y + ' Level: ' + imagery->_level + '.';
		/*that._requestImageError = TileProviderError.handleError(
		 that._requestImageError,
		 imageryProvider,
		 imageryProvider->_errorEvent,
		 message,
		 imagery->_x, imagery->_y, imagery->_level,
		 doRequest);*/
		return NULL;
	}
};

void ImageryLayer::_requestImagery(Imagery* imagery) {
	imagery->_state = ImageryState::TRANSITIONING;
	Promise* imagePromise = _imageryProvider->requestImage(imagery->_x,
			imagery->_y, imagery->_level);

	if (!imagePromise) {
		// Too many parallel requests, so postpone loading tile.
		imagery->_state = ImageryState::UNLOADED;
		return;
	}

	_imageryProvider->getTileCredits(imagery->_x, imagery->_y, imagery->_level,
			imagery->_credits);

	imagery->addReference();
	ImageryLayer_loadImageSuccess* success = new ImageryLayer_loadImageSuccess(
			imagery, _imageryProvider);
	ImageryLayer_loadImageFailure* failure = new ImageryLayer_loadImageFailure(
			imagery, _imageryProvider);

	When when;
	when(imagePromise, success, failure, NULL);
}

void ImageryLayer::_createTexture(Context* context, Imagery* imagery) {
	//ImageryProvider* imageryProvider = _imageryProvider;

	// If this imagery provider has a discard policy, use it to check if this
	// image should be discarded.
	/*if (!_imageryProvider->discardPolicy.isReady()) {
	 imagery.state = ImageryState.RECEIVED;
	 return;
	 }

	 // Mark discarded imagery tiles invalid.  Parent imagery will be used instead.
	 if (_imageryProvider->discardPolicy.shouldDiscardImage(imagery.image)) {
	 imagery.state = ImageryState.INVALID;
	 return;
	 }*/

	ImageryProvider* imageryProvider = _imageryProvider;

	// If this imagery provider has a discard policy, use it to check if this
	// image should be discarded.
#if 0
	if (imageryProvider->get_tileDiscardPolicy()) { //yao add ooo
		TileDiscardPolicy* discardPolicy =
		imageryProvider->get_tileDiscardPolicy();
		if (discardPolicy) {
			// If the discard policy is not ready yet, transition back to the
			// RECEIVED state and we'll try again next time.
			if (!discardPolicy->isReady()) {
				imagery->_state = ImageryState::RECEIVED;
				return;
			}

			// Mark discarded imagery tiles invalid.  Parent imagery will be used instead.
			if (discardPolicy->shouldDiscardImage(imagery->_image)) {
				imagery->_state = ImageryState::INVALID;
				return;
			}
		}
	}
#endif

	Image* image = imagery->_image;
	Texture::Option option;
	option._source._data = image->getData();
	option._width = option._source._width = image->getW();
	option._height = option._source._height = image->getH();
	option._pixelFormat = (PixelFormat::E_PixelFormat) image->getPixfmt();

	// Imagery does not need to be discarded, so upload it to WebGL.
	Texture* texture = context->createTexture2D(option);
#if 0
	std::cout<<"imagerylayer texture "<<texture->_texture<<std::endl;
	int width=option._width;
	int height=option._height;
	std::cout<<"width "<<width<<" "<<"height"<<height<<std::endl;
	 unsigned char * pixels =image->getData();
	// height=26;
	// width=26;
	for(int i=0;i<30;i++){
		for(int j=0;j<30;j++){
			int offset=(256*i+j)*3;
			//std::cout<< (int)pixels[0+offset];
			//    std::cout << (int)pixels[1+offset] ;
			//    std::cout  << (int)pixels[2+offset] ;
		}
		//std::cout<<std::endl;
	}

	std::cout<<std::endl;
	std::cout<<"data type"<<std::hex<<texture->get_pixelDatatype()<<" "<<" pixel format "
			<<texture->get_pixelFormat()<<" width "<<texture->get_width()<<" height "<<
			texture->get_height()<<std::endl;
	Texture2DUseFramBuffer(texture->_texture);
#endif
	//delete image;
	imagery->_texture = texture;
	//imagery->_image = NULL;
	imagery->_state = ImageryState::TEXTURE_LOADED;
}

void ImageryLayer::_reprojectTexture(Context* context, Imagery* imagery) {
	Texture* texture = imagery->_texture;
	Rectangle& rectangle = imagery->_rectangle;

	// Reproject this texture if it is not already in a geographic projection and
	// the pixels are more than 1e-5 radians apart.  The pixel spacing cutoff
	// avoids precision problems in the reprojection transformation while making
	// no noticeable difference in the georeferencing of the image.

	//if (!(_imageryProvider->_tilingScheme instanceof GeographicTilingScheme) &&
	//(rectangle._east - rectangle._west) / texture->_width > 1e-5)
#if 0
	if (!(this->_imageryProvider->get_tilingScheme()->instanceof(
	 TilingScheme::GEOGRAPHIC_SCHEME))
	 && (rectangle._east - rectangle._west) / texture->get_width()
	 > 1e-5) {
	 Texture* reprojectedTexture = ImageryLayer::reprojectToGeographic(this,
	 context, *texture, imagery->_rectangle);
	 delete texture;
	 imagery->_texture = texture = reprojectedTexture;

	 }
#endif
	if (!(texture->get_width() & 0x1) && !(texture->get_height() & 0x1)) {
		std::map<std::string, void *>::iterator it;
		Texture::Sampler *mipmapSampler = NULL;
		it = context->_cache.find("imageryLayer_mipmapSampler");
		if (it != context->_cache.end()) {
			mipmapSampler = (Texture::Sampler*) (it->second);
		} else if (it == context->_cache.end()) {
			F32 maximumSupportedAnisotropy =
					context->get_maximumTextureFilterAnisotropy();
			Texture::Sampler sampler;
			{
				sampler._wrapS = TextureWrap::CLAMP_TO_EDGE;
				sampler._wrapT = TextureWrap::CLAMP_TO_EDGE;
				sampler._minificationFilter =
						TextureMinificationFilter::LINEAR_MIPMAP_LINEAR;
				sampler._magnificationFilter =
						TextureMagnificationFilter::LINEAR;

				F32 tmpvalue;
				if (_maximumAnisotropy == U32_MAX) {
					tmpvalue = maximumSupportedAnisotropy;
				} else {
					tmpvalue = _maximumAnisotropy;
				}
				sampler._maximumAnisotropy = std::min(tmpvalue,
						maximumSupportedAnisotropy);
			}
			mipmapSampler = new Texture::Sampler();
			*mipmapSampler = context->createSampler(sampler);
			context->_cache["imageryLayer_mipmapSampler"] = mipmapSampler;
		}
		texture->generateMipmap(MipmapHint::NICEST);
		texture->set_sampler(*mipmapSampler);
	} else {
		Texture::Sampler *nonMipmapSampler = NULL;
		std::map<std::string, void *>::iterator it;
		it = context->_cache.find("imageryLayer_nonMipmapSampler");
		if (it != context->_cache.end()) {
			nonMipmapSampler = (Texture::Sampler*) (it->second);
		} else if (it == context->_cache.end()) {
			Texture::Sampler sampler;
			{
				sampler._wrapS = TextureWrap::CLAMP_TO_EDGE;
				sampler._wrapT = TextureWrap::CLAMP_TO_EDGE;
				sampler._minificationFilter = TextureMinificationFilter::LINEAR;
				sampler._magnificationFilter =
						TextureMagnificationFilter::LINEAR;

			}
			nonMipmapSampler = new Texture::Sampler();
			*nonMipmapSampler = context->createSampler(sampler);
			context->_cache["imageryLayer_nonMipmapSampler"] = nonMipmapSampler;
		}
		texture->set_sampler(*nonMipmapSampler);

	}

	imagery->_state = ImageryState::READY;
}

Imagery* ImageryLayer::getImageryFromCache(U32 x, U32 y, U32 level,
		const Rectangle& imageryRectangle) {
	uint64_t cacheKey = getImageryCacheKey(x, y, level);
	Imagery* imagery = NULL;
	std::map<U64, Imagery*>::iterator it = _imageryCache.find(cacheKey);

	if (it != _imageryCache.end()) {
		imagery = it->second;
	} else {
		imagery = new Imagery(this, x, y, level, imageryRectangle);
		_imageryCache.insert(std::pair<U64, Imagery*>(cacheKey, imagery));
	}

	return imagery;
}

void ImageryLayer::removeImageryFromCache(Imagery* imagery) {
	uint64_t cacheKey = getImageryCacheKey(imagery->_x, imagery->_y,
			imagery->_level);
	std::map<U64, Imagery*>::iterator it = _imageryCache.find(cacheKey);

	if (it != _imageryCache.end()) {
		_imageryCache.erase(it);
	}
}

Texture* ImageryLayer::reprojectToGeographic(ImageryLayer* imageryLayer,
		Context *context, Texture &texture, Rectangle& rectangle) {

	ImageryLayer::ImageryLayer_reproject * reproject;

	std::map<std::string, void *>::iterator it;
	it = context->_cache.find("imageryLayer_reproject");
	if (it != context->_cache.end()) {
		reproject =
				(ImageryLayer::ImageryLayer_reproject*) it->second;
	} else if (it == context->_cache.end()) {
		reproject = new ImageryLayer::ImageryLayer_reproject();
		context->_cache["imageryLayer_reproject"] = reproject;

		GeometryAttribute<Number>&geoattribute = *new GeometryAttribute<Number>(ComponentDatatype::DOUBLE,2);
		std::vector<Number>&  positions = geoattribute._values;
		positions.resize(256 * 256 * 2);

		int index = 0;
		for (int j = 0; j < 256; ++j) {
			Number y = (Number) j / 255.0;
			for (int i = 0; i < 256; ++i) {
				Number x = (Number) i / 255.0;
				positions[index++] = x;
				positions[index++] = y;
				//positions[index++] = 0.0;
				//positions[index++] = 1.0;
			}
		}
	
		GeometryAttributes& geoattributes = *new GeometryAttributes();


//		{
//			geoattribute._componentDatatype = ComponentDatatype::DOUBLE;
//			geoattribute._componentsPerAttribute = 2;
//			geoattribute._values = positions;
//		}
		geoattributes["position"] = &geoattribute;
		std::vector<U16>& indices = TerrainProvider::getRegularGridIndices(256,
				256)->_indices;
		PrimitiveType::E_PrimitiveType primitiveType = PrimitiveType::_TRIANGLES;
		BoundingSphere boundingSphere;
		Geometry::Option geometryOption;
		geometryOption._attributes = &geoattributes;

		unsigned int len = indices.size();
		std::vector<U32>& indices_u32 = *new std::vector<U32>(len);
		for (unsigned int i = 0; i < len; i++) {
			indices_u32.push_back(indices[i]);
		}

		geometryOption._indices = &indices_u32;
		geometryOption._primitiveType = primitiveType;
		geometryOption._boundingSphere = boundingSphere;
		Geometry *reprojectGeometry = new Geometry(geometryOption);

		VertexArray::GeometryOption options;
		options._geometry = reprojectGeometry;

		AttributeLocations reprojectAttribInds;
		reprojectAttribInds["position"] = 0;
		options._attributeLocations = reprojectAttribInds;

		options._bufferUsage = BufferUsage::STATIC_DRAW;
		reproject->_vertexArray = context->createVertexArrayFromGeometry(options);
		reproject->_shaderProgram = context->createShaderProgram(
				ReprojectWebMercatorVS::_ReprojectWebMercatorVS,
				ReprojectWebMercatorFS::_ReprojectWebMercatorFS,
				reprojectAttribInds);
		F32 maximumSupportedAnisotropy =
				context->get_maximumTextureFilterAnisotropy();

		Texture::Sampler sampler;
		{
			sampler._wrapS = TextureWrap::CLAMP_TO_EDGE;
			sampler._wrapT = TextureWrap::CLAMP_TO_EDGE;
			sampler._minificationFilter = TextureMinificationFilter::LINEAR;
			sampler._magnificationFilter = TextureMagnificationFilter::LINEAR;
			F32 tmpvalue;
			if (imageryLayer->_maximumAnisotropy == U32_MAX) {
				tmpvalue = maximumSupportedAnisotropy;
			} else {
				tmpvalue = imageryLayer->_maximumAnisotropy;
			}
			sampler._maximumAnisotropy = std::min(tmpvalue,
					maximumSupportedAnisotropy);
		}
		reproject->_sampler = context->createSampler(sampler);
	}
	texture.set_sampler(reproject->_sampler);
	S32 width = texture.get_width();
	S32 height = texture.get_height();
	imageryLayer->ILUniformMap->__proto__ = ilUniformsType->_prototype;
	imageryLayer->ILUniformMap->_textureDimensions._x = width;
	imageryLayer->ILUniformMap->_textureDimensions._y = height;
//bacause imageryLayer->ILUniformMap->texture do not need to new
	imageryLayer->ILUniformMap->_texture = &texture;
	imageryLayer->ILUniformMap->_northLatitude = rectangle._north;
	imageryLayer->ILUniformMap->_southLatitude = rectangle._south;

	Number sinLatitude = std::sin(rectangle._south);
	Number southMercatorY = 0.5
			* std::log((1 + sinLatitude) / (1 - sinLatitude));

	ImageryLayer::_float32ArrayScratch[0] = southMercatorY;
	imageryLayer->ILUniformMap->_southMercatorYHigh =
			ImageryLayer::_float32ArrayScratch[0];
	imageryLayer->ILUniformMap->_southMercatorYLow = southMercatorY
			- ImageryLayer::_float32ArrayScratch[0];

	sinLatitude = std::sin(rectangle._north);
	Number northMercatorY = 0.5
			* std::log((1 + sinLatitude) / (1 - sinLatitude));
	imageryLayer->ILUniformMap->_oneOverMercatorHeight = 1.0
			/ (northMercatorY - southMercatorY);

	Texture::Option textureoptions;
	{
		textureoptions._width = width;
		textureoptions._height = height;
		textureoptions._pixelFormat = texture.get_pixelFormat();
		textureoptions._pixelDatatype = texture.get_pixelDatatype();
		textureoptions._preMultiplyAlpha = texture.get_preMultiplyAlpha();
	}
	Texture * outputTexture = context->createTexture2D(textureoptions);
	std::cout<<"imagerylayer outputtexture "<<outputTexture->_texture<<std::endl;
	outputTexture->generateMipmap(MipmapHint::NICEST);

	if (reproject->_framebuffer != NULL) {
		delete reproject->_framebuffer;
	}

	Framebuffer::Option framebufferoptions;
	{
		framebufferoptions._colorTextures.push_back(outputTexture);
		//reproject->_framebuffer->_destroyAttachments = false;
		//replace with below
		//because _destroyAttachments in framebuffer is private
		framebufferoptions._destroyAttachments = false;
	}
	reproject->_framebuffer = context->createFramebuffer(framebufferoptions);
//reproject->_framebuffer->_destroyAttachments = false;
#if 0
//we dont't need to clear the output texture
//we will fufill it
	ClearCommand::Option clearcoptions;
	{
		clearcoptions._color = Color::_BLACK;
		clearcoptions._framebuffer = reproject->_framebuffer;
	}
	ClearCommand clcommand = ClearCommand(clearcoptions);
	clcommand.execute(context, NULL);
#endif
	if (!reproject->_renderState) {
		RenderState renderState;
		{
			BoundingRectangle br = BoundingRectangle(0, 0, width, height);
			renderState._viewport = br;
		}
		reproject->_renderState = context->createRenderState(renderState);
	} else if (reproject->_renderState) {
		if (reproject->_renderState->_viewport._width != width
				|| reproject->_renderState->_viewport._height != height) {
			RenderState renderState;
			{
				BoundingRectangle br = BoundingRectangle(0, 0, width, height);
				renderState._viewport = br;
			}
			reproject->_renderState = context->createRenderState(renderState);
		}
	}
	DrawCommand::Option drawcoption;
	{
		drawcoption._framebuffer = reproject->_framebuffer;
		drawcoption._shaderProgram = reproject->_shaderProgram;
		drawcoption._renderState = reproject->_renderState;
		drawcoption._primitiveType = PrimitiveType::_TRIANGLES;
		drawcoption._vertexArray = reproject->_vertexArray;
		drawcoption._uniformMap = imageryLayer->ILUniformMap;
	}
	DrawCommand drawcommand(drawcoption);
	drawcommand.execute(context);
	ClearCommand::Option clearcoptions1;
	ClearCommand clcommand1 = ClearCommand(clearcoptions1);
	clcommand1.execute(context, NULL);
	return outputTexture;
}

int ImageryLayer::getLevelWithMaximumTexelSpacing(ImageryLayer* layer,
		F64 texelSpacing, F64 latitudeClosestToEquator) {
// PERFORMANCE_IDEA: factor out the stuff that doesn't change.
	ImageryProvider* imageryProvider = layer->_imageryProvider;
	TilingScheme* tilingScheme = imageryProvider->get_tilingScheme();
	Ellipsoid* ellipsoid = tilingScheme->_ellipsoid;
	F64 latitudeFactor =
			!(typeid(*layer->_imageryProvider->get_tilingScheme())
					== typeid(GeographicTilingScheme)) ?
					cos(latitudeClosestToEquator) : 1.0;
	Rectangle& tilingSchemeRectangle = tilingScheme->_rectangle;
	F64 levelZeroMaximumTexelSpacing = ellipsoid->_maximumRadius
			* (tilingSchemeRectangle._east - tilingSchemeRectangle._west)
			* latitudeFactor
			/ (imageryProvider->get_tileWidth()
					* tilingScheme->getNumberOfXTilesAtLevel(0));

	F64 twoToTheLevelPower = levelZeroMaximumTexelSpacing / texelSpacing;
	F64 level = log(twoToTheLevelPower) / log(2);
	int rounded = round(level);
	return rounded;
}

}
