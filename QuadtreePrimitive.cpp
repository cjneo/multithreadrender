/*
 * QuadtreePrimitive.cpp
 *
 *  Created on: 2014-8-19
 *      Author: administrator
 */
#include <limits>
#include <math.h>
#include "Camera.h"
#include "Context.h"
#include "EllipsoidalOccluder.h"
#include "GeographicTilingScheme.h"
#include "getTimestamp.h"
#include "GlobeSurfaceTileProvider.h"

#include "FrameState.h"
#include "QuadtreePrimitive.h"
#include "QuadtreeTileLoadState.h"

#include "../../include/Scene/PerspectiveFrustum.h"
#include "Queue.h"
#include "QuadtreeOccluders.h"
#include "QuadtreeTile.h"
#include "QuadtreeTileLoadState.h"
#include "SceneMode.h"
#include "TileReplacementQueue.h"
#include "Visibility.h"

#include <set>
#include<utility>
#include <sys/time.h>
#include"Scene.h"
#include<pthread.h>
#include"CanvasX11.h"
#include <unistd.h>
#include "GlobeSurfaceTile.h"
#include "TileImagery.h"
#include "Imagery.h"
//#include <pair>
namespace World {

class TileKey {
public:
	int _level;
	int _x;
	int _y;
	TileKey(int level, int x, int y) :
			_level(level), _x(x), _y(y) {

	}
	bool operator <(const TileKey& l) const {
		if (l._level != _level) {
			return l._level > _level;
		}
		if (l._x != _x) {
			return l._x > _x;
		}
		if (l._y != _y) {
			return l._y > _y;
		}
		return false;
	}
};
class TileValue {
public:
	double _x, _y, _z; //casterian
	double _radius;
	double _cx, _cy, _cz;
	TileValue(double x, double y, double z, double radius, double cx, double cy,
			double cz) :
			_x(x), _y(y), _z(z), _radius(radius), _cx(cx), _cy(cy), _cz(cz) {

	}
};
class cmp_tilekey {
public:
	bool operator()(const TileKey&l, const TileKey &r) {
		if (l._level != r._level) {
			return l._level > r._level;
		}
		if (l._x != r._x) {
			return l._x > r._x;
		}
		if (l._y != r._y) {
			return l._y > r._y;
		}
		return false;
	}
	;
};
//std::set<TileKey, cmp_tilekey> tileSet;
extern Scene * cjscene;
pthread_t ntid;
CanvasX11 *_canvas;
QuadtreePrimitive* cjprimitive;
Context* cjcontext;
FrameState* cjframeState;
pthread_mutex_t mutex;
volatile bool threadstop = true;
bool threadnotrun=true;
std::map<TileKey, TileValue> tileSet;
std::set<TileKey> tset;
struct timeval startselect;
GLuint Texture2DUseFramBuffer(GLuint textureId);
QuadtreePrimitive::QuadtreePrimitive(Option& options) {
// TODO Auto-generated constructor stub
//>>includeStart('debug', pragmas.debug);

//>>includeEnd('debug');

	this->_tileProvider = options._tileProvider;
	this->_tileProvider->set_quadtree(this);
//ooo
	GeographicTilingScheme* tilingScheme =
			this->_tileProvider->get_tilingScheme();

	Ellipsoid* ellipsoid = tilingScheme->_ellipsoid;

	this->_levelZeroTiles = NULL;
	this->_levelZeroTilesReady = false;
	//mcx this->_loadQueueTimeSlice = std::numeric_limits<Number>::quiet_NaN();//0.0;
	//this->_loadQueueTimeSlice = 5.0;
	this->_loadQueueTimeSlice = 100.0;
	//this->_loadQueueTimeSlice = 100000.0;
	//mcx -js- this._loadQueueTimeSlice = 5.0;
	/**
	 * Gets or sets the maximum screen-space error, in pixels, that is allowed.
	 * A higher maximum error will render fewer tiles and improve performance, while a lower
	 * value will improve visual quality.
	 * @type {Number}
	 * @default 2
	 */
	this->_maximumScreenSpaceError = options._maximumScreenSpaceError;

	/**
	 * Gets or sets the maximum number of tiles that will be retained in the tile cache.
	 * Note that tiles will never be unloaded if they were used for rendering the last
	 * frame, so the actual number of resident tiles may be higher.  The value of
	 * this property will not affect visual quality.
	 * @type {Number}
	 * @default 100
	 */
	this->_tileCacheSize = options._tileCacheSize;
//QuadtreeOccluders::Option args;
	this->_occluders = new QuadtreeOccluders(*ellipsoid);
}

GlobeSurfaceTileProvider* QuadtreePrimitive::get_tileProvider() {
	return _tileProvider;
}

QuadtreePrimitive::~QuadtreePrimitive() {
// TODO Auto-generated destructor stub
	destroy();
}

void QuadtreePrimitive::destroy() {

//this->_tileProvider = this->_tileProvider && this->_tileProvider->destroy();
	if (this->_tileProvider) {
		delete this->_tileProvider;
		this->_tileProvider = NULL;
	}
}

void QuadtreePrimitive::addTileToRenderList(QuadtreePrimitive& primitive,
		QuadtreeTile& tile) {
	primitive._tilesToRender.push_back(&tile);
// ++primitive._debug.tilesRendered;
}
void printids(const char *s) {
	pid_t pid;
	pthread_t tid;

	pid = getpid();
	tid = pthread_self();

	printf("%s pid %u tid %u (0x%x)\n", s, (unsigned int) pid,
			(unsigned int) tid, (unsigned int) tid);
}
void * Init(void *) {
while(true)
	{
		while(threadstop){
			sleep(1);
		}
		pthread_mutex_lock(&mutex);
		QuadtreePrimitive& primitive = *cjprimitive;
		Context& context = *cjcontext;
		FrameState& frameState = *cjframeState;

		std::vector<QuadtreeTile*> &tileLoadQueue = primitive._tileLoadQueue;
		GlobeSurfaceTileProvider* tileProvider = primitive._tileProvider;

		if (tileLoadQueue.size() == 0) {
		//	std::cout << " size 0" << std::endl;
			//return 0;
		}

		// Remove any tiles that were not used this frame beyond the number
		// we're allowed to keep.
		primitive._tileReplacementQueue.trimTiles(primitive._tileCacheSize);
		//primitive._tileReplacementQueue.dump();

		S32 i = 0;
		for (S32 len = tileLoadQueue.size(), i = 0; i < len; i++) {

			QuadtreeTile* tile = tileLoadQueue[i];
		//	std::cout << "tile loading " << tile->_level << " " << tile->_x <<" "<<tile->_y<< std::endl;
			primitive._tileReplacementQueue.markTileRendered(tile);
			tileProvider->loadTile(context, frameState, *tile);
			if(threadstop==true){

				break;
			}
		}
		glFinish();
		pthread_mutex_unlock(&mutex);
		threadstop = true;
	}
	return 0;
}
void * ThreadInit(void *) {
#if 1
	//printids("new thread:");
	_canvas = (CanvasX11 *) cjscene->get_canvas();
	EGLint contextAttribs[] = { EGL_CONTEXT_CLIENT_VERSION, 2, EGL_NONE,
	EGL_NONE };
	EGLContext threadContext = eglCreateContext(_canvas->_display,
			_canvas->_config, _canvas->_context, contextAttribs);
	if (threadContext == EGL_NO_CONTEXT) {
		EGLint err = eglGetError();
		std::cout << "renderthread makecurrent error:" << err << std::endl;
		printf("%x\n", err);

		return EGL_FALSE;
	}

	if (!eglMakeCurrent(_canvas->_display, EGL_NO_SURFACE, EGL_NO_SURFACE,
			threadContext)) {
		EGLint err = eglGetError();
		std::cout << "renderthread makecurrent error:" << err << std::endl;
		printf("%x\n", err);
		//const GLubyte* errstr=eglErrorString(err);
		//cout<<errstr<<endl;
		return EGL_FALSE;
	}
	EGLint err = eglGetError();
	std::cout << "renderthread makecurrent info:" << err << std::endl;
	printf("%x\n", err);
#endif
	Init(0);

}
void renderThread() {
	int err;
	//ThreadInit(0);
	//return;

	err = pthread_create(&ntid, NULL, ThreadInit, NULL);
	if (0 != err) {
		printf("can't create thread:%s\n", strerror(err));
	}

	//return 1;
}
void QuadtreePrimitive::processTileLoadQueue(QuadtreePrimitive& primitive,
		Context& context, FrameState& frameState) {
#if 0
	static bool firstrender = true;
	static int rtime=1;
	threadstop = false;
	if (firstrender) {
	//if (firstrender&&rtime<10) {

		_canvas = (CanvasX11 *) cjscene->get_canvas();
		firstrender = false;
		cjprimitive = &primitive;
		cjcontext = &context;
		cjframeState = &frameState;
		//renderThread();
		renderThread();
		//int *ret;
		//pthread_join(ntid, (void **) (&ret));
		//std::cout<<rtime<<"thread end------------------------------------------------------------- "<<std::endl;
				//rtime++;
	} else {
		;
	}

#else
	std::vector<QuadtreeTile*> &tileLoadQueue = primitive._tileLoadQueue;
	GlobeSurfaceTileProvider* tileProvider = primitive._tileProvider;

	if (tileLoadQueue.size() == 0) {
		return;
	}

// Remove any tiles that were not used this frame beyond the number
// we're allowed to keep.
	primitive._tileReplacementQueue.trimTiles(primitive._tileCacheSize);
	//primitive._tileReplacementQueue.dump();
	Number startTime = getTimestamp();
	Number timeSlice = primitive._loadQueueTimeSlice;
	Number endTime = startTime + timeSlice;
	S32 i = 0;
	for (S32 len = tileLoadQueue.size(), i = 0; i < len; i++) {
//	for (S32 len = tileLoadQueue.size(), i = tileLoadQueue.size()-1; i >=0; i--) {
		QuadtreeTile* tile = tileLoadQueue[i];
		primitive._tileReplacementQueue.markTileRendered(tile);
		tileProvider->loadTile(context, frameState, *tile);
		if (getTimestamp() >= endTime) {
			break;
		}
	}
#endif
}

void QuadtreePrimitive::queueTileLoad(QuadtreePrimitive& primitive,
		QuadtreeTile* tile) {

	primitive._tileLoadQueue.push_back(tile);
}

bool QuadtreePrimitive::queueChildrenLoadAndDetermineIfChildrenAreAllRenderable(
		QuadtreePrimitive& primitive, QuadtreeTile& tile) {
	bool allRenderable = true;
	bool allUpsampledOnly = true;

	std::vector<QuadtreeTile*>* children = &tile.get_children();
	for (U32 i = 0, len = children->size(); i < len; ++i) {
		QuadtreeTile* child = (*children)[i];

		primitive._tileReplacementQueue.markTileRendered(child);

		allUpsampledOnly = allUpsampledOnly && child->_upsampledFromParent;
		allRenderable = allRenderable && child->_renderable;

		if (child->get_needsLoading()) {
			queueTileLoad(primitive, child);
		}
	}

	if (!allRenderable) {
//++primitive._debug.tilesWaitingForChildren;
	}

// If all children are upsampled from this tile, we just render this tile instead of its children.
	return allRenderable && !allUpsampledOnly;
}

Number QuadtreePrimitive::screenSpaceError2D(QuadtreePrimitive& primitive,
		Context& context, FrameState& frameState, QuadtreeTile& tile) {
	Camera* camera = frameState._camera;
	BaseFrustum* frustum = camera->_frustum;
	Number width = context.get_drawingBufferWidth();
	Number height = context.get_drawingBufferHeight();

	U32 maxGeometricError =
			primitive._tileProvider->getLevelMaximumGeometricError(tile._level);
	Number pixelSize = std::max(frustum->top - frustum->bottom,
			frustum->right - frustum->left) / std::max(width, height);
	return maxGeometricError / pixelSize;
}

Number QuadtreePrimitive::screenSpaceError(QuadtreePrimitive& primitive,
		Context& context, FrameState& frameState, QuadtreeTile& tile) {
	if (frameState._mode == SceneMode::_SCENE2D) {
		return screenSpaceError2D(primitive, context, frameState, tile);
	}

	U32 maxGeometricError =
			primitive._tileProvider->getLevelMaximumGeometricError(tile._level);

	Number distance = primitive._tileProvider->computeDistanceToTile(tile,
			frameState);
	tile._distance = distance;

	Number height = context.get_drawingBufferHeight();

	Camera* camera = frameState._camera;
//PerspectiveFrustum
	BaseFrustum* frustum = camera->_frustum;
	Number fovy = frustum->get_fovy();

// PERFORMANCE_IDEA: factor out stuff that's constant across tiles.
	return (maxGeometricError * height)
			/ (2 * distance * std::tan((long double) 0.5 * fovy));
}
/**
 * Invalidates and frees all the tiles in the quadtree.  The tiles must be reloaded
 * before they can be displayed.
 *
 * @memberof QuadtreePrimitive
 */

void QuadtreePrimitive::invalidateAllTiles() {
// Clear the replacement queue
	TileReplacementQueue* replacementQueue = &this->_tileReplacementQueue;
	replacementQueue->_head = NULL;
	replacementQueue->_tail = NULL;
	replacementQueue->_count = 0;

// Free and recreate the level zero tiles.
	std::vector<QuadtreeTile*>* levelZeroTiles = this->_levelZeroTiles;
	if (levelZeroTiles) {
		for (U32 i = 0; i < levelZeroTiles->size(); ++i) {
			delete (*levelZeroTiles)[i]; //->freeResources();
			(*levelZeroTiles)[i] = NULL;
		}
	}
	//this._levelZeroTiles = undefined;
	delete this->_levelZeroTiles; //ooo
	this->_levelZeroTiles = NULL;
}

/**
 * Invokes a specified function for each {@link QuadtreeTile} that is partially
 * or completely loaded.
 *
 * @param {Function} tileFunction The function to invoke for each loaded tile.  The
 *        function is passed a reference to the tile as its only parameter.
 */

void QuadtreePrimitive::forEachLoadedTile(
		ForEachLoadedTileCallbackBase &tileFunction) {
	QuadtreeTile* tile = this->_tileReplacementQueue._head;
	while (tile) {
		//mcx
		if (tile->_state != QuadtreeTileLoadState::START) {
			tileFunction(tile);
		}
		//tileFunction(tile);
		tile = tile->_replacementNext;
	}
//if (tileFunction)
// delete tileFunction;
}

/**
 * Invokes a specified function for each {@link QuadtreeTile} that was rendered
 * in the most recent frame.
 *
 * @param {Function} tileFunction The function to invoke for each rendered tile.  The
 *        function is passed a reference to the tile as its only parameter.
 */
void QuadtreePrimitive::forEachRenderedTile(
		ForEachLoadedTileCallbackBase &tileFunction) {
	std::vector<QuadtreeTile*>* tilesRendered = &this->_tilesToRender;
	for (U32 i = 0, len = tilesRendered->size(); i < len; ++i) {
		tileFunction((*tilesRendered)[i]);
	}
}
inline void mytime(QuadtreeTile* tile, std::set<TileKey> &tset, int timeuse) {
	TileKey tilekey(tile->_level, tile->_x, tile->_y);
	if (tset.count(tilekey) == 0) {
		tset.insert(tilekey);
		//hasnew=true;

		std::cout << "time " << timeuse << std::endl;
	}

}
void myprepareNewTile(QuadtreeTile* tile) {
	GlobeSurfaceTile* surfaceTile = tile->_data;
	Ellipsoid* ellipsoid = tile->_tilingScheme->_ellipsoid;

// Compute tile rectangle boundaries for estimating the distance to the tile.
	Rectangle& rectangle = tile->_rectangle;

	BoundingSphere::fromRectangle3D(rectangle, *ellipsoid, 0,
			surfaceTile->_boundingSphere3D);
	GlobeSurfaceTile::_ellipsoidalOccluder.computeHorizonCullingPointFromRectangle(
			tile->_rectangle, *ellipsoid,
			surfaceTile->_occludeePointInScaledSpace);

	Cartographic cartographic;
	Rectangle::southwest(rectangle, cartographic);
	ellipsoid->cartographicToCartesian(cartographic,
			surfaceTile->_southwestCornerCartesian);

	Rectangle::northeast(rectangle, cartographic);
	ellipsoid->cartographicToCartesian(cartographic,
			surfaceTile->_northeastCornerCartesian);

// The middle latitude on the western edge.
	Cartographic cartographicScratch(rectangle._west,
			(rectangle._south + rectangle._north) * 0.5, 0.0);
	Cartesian3 westernMidpointCartesian;
	ellipsoid->cartographicToCartesian(cartographicScratch,
			westernMidpointCartesian);

// Compute the normal of the plane on the western edge of the tile.
	Cartesian3 cartesian3Scratch;
	Cartesian3::cross(westernMidpointCartesian, Cartesian3::_UNIT_Z,
			cartesian3Scratch);
	Cartesian3::normalize(cartesian3Scratch, surfaceTile->_westNormal);

// The middle latitude on the eastern edge.
	cartographicScratch._longitude = rectangle._east;
	Cartesian3 easternMidpointScratch;
	ellipsoid->cartographicToCartesian(cartographicScratch,
			easternMidpointScratch);

// Compute the normal of the plane on the eastern edge of the tile.
	Cartesian3::cross(Cartesian3::_UNIT_Z, easternMidpointScratch,
			cartesian3Scratch);
	Cartesian3::normalize(cartesian3Scratch, surfaceTile->_eastNormal);

// Compute the normal of the plane bounding the southern edge of the tile.
	Rectangle::southeast(rectangle, cartographicScratch);
	Cartesian3 cartesian3Scratch2;
	ellipsoid->geodeticSurfaceNormalCartographic(cartographicScratch,
			cartesian3Scratch2);

	Cartesian3::subtract(westernMidpointCartesian, easternMidpointScratch,
			cartesian3Scratch);
	Cartesian3::cross(cartesian3Scratch2, cartesian3Scratch,
			cartesian3Scratch2);
	Cartesian3::normalize(cartesian3Scratch2, surfaceTile->_southNormal);

// Compute the normal of the plane bounding the northern edge of the tile.
	Rectangle::northwest(rectangle, cartographicScratch);
	ellipsoid->geodeticSurfaceNormalCartographic(cartographicScratch,
			cartesian3Scratch2);
	Cartesian3::cross(cartesian3Scratch, cartesian3Scratch2,
			cartesian3Scratch2);
	Cartesian3::normalize(cartesian3Scratch2, surfaceTile->_northNormal);
}
void QuadtreePrimitive::selectTilesForRendering(QuadtreePrimitive& primitive,
		Context& context, FrameState& frameState) {

	Debug& debug = primitive._debug;
	static bool firsttime = true;

	if (firsttime) {
		firsttime = false;
		gettimeofday(&startselect, NULL);
	}
	if (debug._suspendLodUpdate) {
		return;
	}

	U32 i = 0;
	U32 len = 0;

// Clear the render list.
	std::vector<QuadtreeTile*>* tilesToRender = &primitive._tilesToRender;
	tilesToRender->clear();

	Queue<QuadtreeTile*>* traversalQueue = &primitive._tileTraversalQueue;
	traversalQueue->clear();
//
//       debug.maxDepth = 0;
//       debug.tilesVisited = 0;
//       debug.tilesCulled = 0;
//       debug.tilesRendered = 0;
//       debug.tilesWaitingForChildren = 0;

	primitive._tileLoadQueue.clear();
	primitive._tileReplacementQueue.markStartOfRenderFrame();

// We can't render anything before the level zero tiles exist.
	if (!primitive._levelZeroTiles) {
		if (primitive._tileProvider->get_ready()) {
			GeographicTilingScheme* terrainTilingScheme =
					primitive._tileProvider->get_tilingScheme();
			primitive._levelZeroTiles = new std::vector<QuadtreeTile*>();
			QuadtreeTile::createLevelZeroTiles(
					(TilingScheme*) terrainTilingScheme,
					*primitive._levelZeroTiles);
		} else {
// Nothing to do until the provider is ready.
			return;
		}
	}

	primitive._occluders->get_ellipsoid()->set_cameraPosition(
			frameState._camera->get_positionWC());

	GlobeSurfaceTileProvider* tileProvider = primitive._tileProvider;
	QuadtreeOccluders* occluders = primitive._occluders;

	QuadtreeTile* tile = NULL;

// Enqueue the root tiles that are renderable and visible.
	std::vector<QuadtreeTile*>* levelZeroTiles = primitive._levelZeroTiles;
	for (i = 0, len = levelZeroTiles->size(); i < len; ++i) {
		tile = (*levelZeroTiles)[i];
		primitive._tileReplacementQueue.markTileRendered(tile);
		if (tile->get_needsLoading()) {
			queueTileLoad(primitive, tile);
		}
		if (tile->_renderable
				&& tileProvider->computeTileVisibility(*tile, frameState,
						*occluders) != Visibility::_NONE) {
			traversalQueue->enqueue(tile);
		} else {
// ++debug.tilesCulled;
// if (!tile.renderable) {
// ++debug.tilesWaitingForChildren;
// }
		}
	}

// Traverse the tiles in breadth-first order.
// This ordering allows us to load bigger, lower-detail tiles before smaller, higher-detail ones.
// This maximizes the average detail across the scene and results in fewer sharp transitions
// between very different LODs.
//std::cout<<YELLOW<<"selectTilesForRendering[{"<<RESET<<std::endl;
	while ((tile = traversalQueue->dequeue())) {
//++debug.tilesVisited;
#if 0
		std::cout << "LOCALTILE " << tile->_level << " " << tile->_x << " "
				<< tile->_y << " " << tile->_state << "isrenable "
				<< tile->_renderable << std::endl;
		GlobeSurfaceTile::Imagerys &imagerys = tile->_data->_imagerys;

		for (int i = 0; i < imagerys.size(); i++) {
			static bool _4visit = false, _5visit = false, _7visit = false,
					_8visit = false;
			_4visit = _5visit = _7visit = _8visit = true;
			TileImagery* imagery = imagerys[i];
			// std::cout<<"url "<<imagery->_readyImagery->_imageUrl<<std::endl;
			int id = imagery->_readyImagery->_texture->_texture;
			std::cout << "id " << imagery->_readyImagery->_texture->_texture
					<< std::endl;
			GLboolean result = glIsTexture(
					imagery->_readyImagery->_texture->_texture);
			if (result != GL_TRUE) {
				std::cout << "not valid!!" << std::endl;
			}
			if (id == 4 && _4visit == false) {
				_4visit = true;
				Texture2DUseFramBuffer(id);
			}
			if (id == 5 && _5visit == false) {
				_5visit = true;
				Texture2DUseFramBuffer(id);
			}
			if (id == 7 && _7visit == false) {
				_7visit = true;
				Texture2DUseFramBuffer(id);
			}
			if (id == 8 && _8visit == false) {
				_8visit = true;
				Texture2DUseFramBuffer(id);
			}
		}
#endif
		primitive._tileReplacementQueue.markTileRendered(tile);
		if (tile->_renderable)
			addTileToRenderList(primitive, *tile);
		//continue;
		//addTileToRenderList(primitive, *tile);
// if (tile.level > debug.maxDepth) {
// debug.maxDepth = tile.level;
// }

// There are a few different algorithms we could use here.
// This one doesn't load children unless we refine to them.
// We may want to revisit this in the future.

		if (screenSpaceError(primitive, context, frameState, *tile)
				< primitive._maximumScreenSpaceError) {
// This tile meets SSE requirements, so render it.
			//std::cout<<YELLOW<<"_maximumScreenSpaceError:"<<RESET<<std::endl;
			if (tile->_renderable) {
				addTileToRenderList(primitive, *tile);
			}
		} else if (queueChildrenLoadAndDetermineIfChildrenAreAllRenderable(
				primitive, *tile)) {
			//std::cout<<YELLOW<<"queueChildrenLoadAndDetermineIfChildrenAreAllRenderable:"<<RESET<<std::endl;
// SSE is not good enough and children are loaded, so refine.

			std::vector<QuadtreeTile*>* children = &tile->get_children();
// PERFORMANCE_IDEA: traverse children front-to-back so we can avoid sorting by distance later.
			for (i = 0, len = children->size(); i < len; ++i) {
				if (tileProvider->computeTileVisibility(*(*children)[i],
						frameState, *occluders) != Visibility::_NONE) {
					traversalQueue->enqueue((*children)[i]);

//					}

				} else {
//++debug.tilesCulled;
//					 if (!tile.renderable) {
//					                    ++debug.tilesWaitingForChildren;
//					                }
				}
			}

		} else {
//++debug.tilesWaitingForChildren;
// SSE is not good enough but not all children are loaded, so render this tile anyway.
			//std::cout<<YELLOW<<"no:"<<RESET<<std::endl;
			//	allrenderable=false;
#if 0
			std::vector<QuadtreeTile*>* children = &tile->get_children();
			for (i = 0, len = children->size(); i < len; ++i) {

				if ((*children)[i]->_data
						&& tileProvider->computeTileVisibility(*(*children)[i],
								frameState, *occluders) != Visibility::_NONE) {
					//waitchild++;
					traversalQueue->enqueue((*children)[i]);
				}
			}
#endif
			if (tile->_renderable) {
				addTileToRenderList(primitive, *tile);
			}

		}
	}

//	std::cout<<YELLOW<<"}]"<<RESET<<std::endl;
#if 0
	if (debug.enableDebugOutput) {
		if (debug.tilesVisited !== debug.lastTilesVisited ||
				debug.tilesRendered !== debug.lastTilesRendered ||
				debug.tilesCulled !== debug.lastTilesCulled ||
				debug.maxDepth !== debug.lastMaxDepth ||
				debug.tilesWaitingForChildren !== debug.lastTilesWaitingForChildren) {

			/*global console*/
			console.log(
					'Visited ' + debug.tilesVisited + ', Rendered: '
 + debug.tilesRendered + ', Culled: '
 + debug.tilesCulled + ', Max Depth: '
 + debug.maxDepth + ', Waiting for children: '
 + debug.tilesWaitingForChildren);

			debug.lastTilesVisited = debug.tilesVisited;
			debug.lastTilesRendered = debug.tilesRendered;
			debug.lastTilesCulled = debug.tilesCulled;
			debug.lastMaxDepth = debug.maxDepth;
			debug.lastTilesWaitingForChildren = debug.tilesWaitingForChildren;
		}
	}
#endif
}

bool tileDistanceSortFunction(QuadtreeTile * a, QuadtreeTile * b) {
	return a->_distance < b->_distance;
}

void QuadtreePrimitive::createRenderCommandsForSelectedTiles(
		QuadtreePrimitive& primitive, Context& context, FrameState& frameState,
		RenderCommandsList& commandList) {

//ooo
// function tileDistanceSortFunction(a, b) {
// return a._distance - b._distance;
// }

	GlobeSurfaceTileProvider* tileProvider = primitive._tileProvider;
	std::vector<QuadtreeTile*>* tilesToRender = &primitive._tilesToRender;

//
//tilesToRender.sort(tileDistanceSortFunction);
	std::sort(tilesToRender->begin(), tilesToRender->end(),
			tileDistanceSortFunction);
	for (U32 i = 0, len = tilesToRender->size(); i < len; ++i) {
		tileProvider->showTileThisFrame(*(*tilesToRender)[i], context,
				frameState, commandList);
	}
}

void QuadtreePrimitive::update(Context& context, FrameState& frameState,
		RenderCommandsList& commandList) {
	static bool firstrender = true;
	if (firstrender) {
		pthread_mutex_init(&mutex, NULL);
		firstrender = false;

	}
	//threadstop = true;
	//pthread_mutex_lock(&mutex);
	this->_tileProvider->beginUpdate(context, frameState, commandList);

	selectTilesForRendering(*this, context, frameState);
	Number end = getTimestamp();
	//threadstop = false;
	//pthread_mutex_unlock(&mutex);
	processTileLoadQueue(*this, context, frameState);

	//while(!threadstop)
	{
		//sleep(1);
	}
	//pthread_mutex_lock(&mutex);
	//pthread_mutex_unlock(&mutex);
	createRenderCommandsForSelectedTiles(*this, context, frameState,
			commandList);
	end = getTimestamp();
	//std::cout << " before create rendercommand before:" << commandList.size()<< std::endl;
	this->_tileProvider->endUpdate(context, frameState, commandList);
	//std::cout << " before create rendercommand after:" << commandList.size()
	//		<< std::endl;
}

} /* namespace World */
