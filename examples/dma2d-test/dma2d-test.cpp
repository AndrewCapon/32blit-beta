// Example showing how to use the profiler
//
// Button			Function
// =====================================================
// A					Enable/Disable Bar Graphs
// B					Enable/Disable Text Values
// X					Enable/Disable History Graph
// Y					Switch graph time between global, set by maximum render time
//						and individual times, set by each probles maximum time
// UP					Reduce rows displayed on page
// DOWN				Increase rows displayed on page
// LEFT				Back Page
// RIGHT			Next Page


#include "dma2d-test.hpp"
#include "graphics/color.hpp"

#include <cmath>
#include "assets.hpp"
#include <stdio.h>
#include <string.h>


DMA2DTest Dma2dTest;



// c calls to c++ object
void init()
{
	Dma2dTest.Init();
}

void render(uint32_t time)
{
	Dma2dTest.Render(time);
}

void update(uint32_t time)
{
	Dma2dTest.Update(time);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

DMA2DTest::DMA2DTest()
{
  m_pProfiler = new blit::Profiler(250);
	m_pProfiler->setup_graph_element(blit::Profiler::dmCur, true, true, blit::Pen(0,255,0));
	m_pProfiler->setup_graph_element(blit::Profiler::dmAvg, true, true, blit::Pen(0,255,255));
	m_pProfiler->setup_graph_element(blit::Profiler::dmMax, true, true, blit::Pen(255,0,0));
	m_pProfiler->setup_graph_element(blit::Profiler::dmMin, true, true, blit::Pen(255,255,0));

	m_pProfiler->set_display_size(SCREEN_WIDTH, SCREEN_HEIGHT);
	m_pProfiler->set_rows(10);
	m_pProfiler->set_alpha(200);
	m_pProfiler->display_history(true);
}

void DMA2DTest::Init(void)
{
  // set resolution
  blit::set_screen_mode(blit::ScreenMode::hires);

  // set default pen
  m_pen = blit::Pen(0x11,0x22,0xff,255);

  // add cdc commands
	g_commandStream.AddCommandHandler(CDCCommandHandler::CDCFourCCMake<'D', 'U', 'M', 'P'>::value, this);
	g_commandStream.AddCommandHandler(CDCCommandHandler::CDCFourCCMake<'D', 'M', 'A', '2'>::value, this);
	g_commandStream.AddCommandHandler(CDCCommandHandler::CDCFourCCMake<'S', 'O', 'F', 'T'>::value, this);
	g_commandStream.AddCommandHandler(CDCCommandHandler::CDCFourCCMake<'S', 'O', 'L', 'I'>::value, this);
	g_commandStream.AddCommandHandler(CDCCommandHandler::CDCFourCCMake<'A', 'L', 'P', 'H'>::value, this);

  // add clear test
  AddTest("Clear", [](){ blit::screen.clear(); });
}

void DMA2DTest::Render(uint32_t time)
{
  blit::screen.pen = m_pen;
  for(DMA2DTestUnit *t : m_tests)
    t->Test();

  m_pProfiler->display_probe_overlay(1);
}

void DMA2DTest::Update(uint32_t time)
{

}

bool DMA2DTest::StreamInit(CDCFourCC uCommand)
{
	switch(uCommand)
	{
		case CDCCommandHandler::CDCFourCCMake<'D', 'U', 'M', 'P'>::value:
      if(blit::screen.use_dma2d)
        printf("DMA2D ");
      else
        printf("software ");

      if(m_pen.a < 255)
        printf("Alpha pen\n");
      else
        printf("Solid pen\n");

      m_pProfiler->log_probes();
		break;

		case CDCCommandHandler::CDCFourCCMake<'D', 'M', 'A', '2'>::value:
      printf("Setting DMA2\n");
      blit::screen.use_dma2d = true;
      m_pProfiler->clear_all_probes();
		break;

		case CDCCommandHandler::CDCFourCCMake<'S', 'O', 'F', 'T'>::value:
      printf("Setting SOFT\n");
      blit::screen.use_dma2d = false;
      m_pProfiler->clear_all_probes();
		break;

		case CDCCommandHandler::CDCFourCCMake<'S', 'O', 'L', 'I'>::value:
      printf("Setting SOLI\n");
      m_pen = blit::Pen(0,0,255,255);
      m_pProfiler->clear_all_probes();
		break;

		case CDCCommandHandler::CDCFourCCMake<'A', 'L', 'P', 'H'>::value:
      printf("Setting ALPH\n");
      m_pen = blit::Pen(0,0,255,10);
      m_pProfiler->clear_all_probes();
		break;


	}

	return false;
}

CDCCommandHandler::StreamResult DMA2DTest::StreamData(CDCDataStream &dataStream)
{
  return srFinish;
}




// // Profiler 			g_profiler(300); // global uRunningAverageSize and uRunningAverageSpan could be set here.
// // ProfilerProbe *g_pRenderProbe;
// // ProfilerProbe *g_pClearProbe;
// // ProfilerProbe *g_pDma2dClearProbe;
// // ProfilerProbe *g_pRectProbe;
// // ProfilerProbe *g_pCircleProbe;
// // ProfilerProbe *g_pUpdateProbe;

// // ProfilerProbe *g_pSurface_8x8;
// // ProfilerProbe *g_pSurface_16x16;
// // ProfilerProbe *g_pSurface_32x32;
// // ProfilerProbe *g_pSurface_64x64;
// // ProfilerProbe *g_pSurface_128x128;
// // ProfilerProbe *g_pSurface_320x240;

// // ProfilerProbe *g_pSprite_8x8;
// // ProfilerProbe *g_pSprite_16x16;
// // ProfilerProbe *g_pSprite_32x32;
// // ProfilerProbe *g_pSprite_64x64;
// // ProfilerProbe *g_pSprite_128x128;

// // ProfilerProbe *g_pSurface_rect;
// // ProfilerProbe *g_pScreen_rect;

// uint8_t buffer_8x8[8*8*4];
// blit::Surface surface_8x8(buffer_8x8,  PixelFormat::RGBA, Size(8,8));

// uint8_t buffer_16x16[16*16*4];
// blit::Surface surface_16x16(buffer_16x16,  PixelFormat::RGBA, Size(16,16));

// uint8_t buffer_32x32[32*32*4];
// blit::Surface surface_32x32(buffer_32x32,  PixelFormat::RGBA, Size(32,32));

// uint8_t buffer_64x64[64*64*4];
// blit::Surface surface_64x64(buffer_64x64,  PixelFormat::RGBA, Size(64,64));

// uint8_t buffer_128x128[128*128*4];
// blit::Surface surface_128x128(buffer_128x128,  PixelFormat::RGBA, Size(128,128));

// // uint8_t buffer_320x240[320*240*3];
// // blit::Surface surface_8x8(buffer_320x240,  PixelFormat::RGB, Size(8,8));
// // blit::Surface surface_16x16(buffer_320x240,  PixelFormat::RGB, Size(16,16));
// // blit::Surface surface_32x32(buffer_320x240,  PixelFormat::RGB, Size(32,32));
// // blit::Surface surface_64x64(buffer_320x240,  PixelFormat::RGB, Size(64,64));
// // blit::Surface surface_128x128(buffer_320x240,  PixelFormat::RGB, Size(128,128));
// // blit::Surface surface_320x240(buffer_320x240,  PixelFormat::RGB, Size(320,240));

// uint32_t g_uSize = 2<<16;
// uint32_t g_uSizeMax	= (SCREEN_HEIGHT-1)<<16;
// uint32_t g_uSizeMin	= 2<<16;
// uint32_t g_uSizeChange = 1<<16;

// bool g_bGraphEnabled   = true;
// bool g_bLabelsEnabled  = true;
// bool g_bDisplayHistory = false;
// bool g_bUseGlobalTime	 = true;

// uint8_t g_uRows = 11;
// uint8_t g_uPage = 1;



// #define RANDX random()%SCREEN_WIDTH
// #define RANDY random()%SCREEN_HEIGHT
// #define RANDC (int)(random()%256)

// void SetupMetrics()
// {
// 	g_profiler.setup_graph_element(Profiler::dmCur, g_bLabelsEnabled, g_bGraphEnabled, Pen(0,255,0));
// 	g_profiler.setup_graph_element(Profiler::dmAvg, g_bLabelsEnabled, g_bGraphEnabled, Pen(0,255,255));
// 	g_profiler.setup_graph_element(Profiler::dmMax, g_bLabelsEnabled, g_bGraphEnabled, Pen(255,0,0));
// 	g_profiler.setup_graph_element(Profiler::dmMin, g_bLabelsEnabled, g_bGraphEnabled, Pen(255,255,0));
// }

// volatile packed_image *pImage = (packed_image *)packed_data;

// void init()
// {
//   	// register DUMP
// 	g_commandStream.AddCommandHandler(CDCCommandHandler::CDCFourCCMake<'D', 'U', 'M', 'P'>::value, this);

// 	// register DMA2
// 	g_commandStream.AddCommandHandler(CDCCommandHandler::CDCFourCCMake<'D', 'M', 'A', '2'>::value, this);

// 	// register SOFT
// 	g_commandStream.AddCommandHandler(CDCCommandHandler::CDCFourCCMake<'S', 'O', 'F', 'T'>::value, this);

// 	// register SOLI
// 	g_commandStream.AddCommandHandler(CDCCommandHandler::CDCFourCCMake<'S', 'O', 'L', 'I'>::value, this);

// 	// register ALPH
// 	g_commandStream.AddCommandHandler(CDCCommandHandler::CDCFourCCMake<'A', 'L', 'P', 'H'>::value, this);

// 	set_screen_mode(ScreenMode::hires);

//   volatile uint32_t uSizePalette = pImage->palette_entry_count;

//   screen.sprites = SpriteSheet::load(packed_data);
//   volatile Pen *pPen0 = (Pen *)&(screen.sprites->palette);
//   volatile Pen *pPen1 = pPen0 + 1;
//   volatile Pen *pPen2 = pPen0 + 2;
//   volatile Pen *pPen3 = pPen0 + 3;
  
//   uint8_t uAlpha = 255;

//   surface_8x8.pen = Pen(255, 0, 0, uAlpha);
//   surface_8x8.clear();

//   surface_16x16.pen = Pen(0, 255, 0, uAlpha);
//   surface_16x16.clear();

//   surface_32x32.pen = Pen(0, 0, 255, uAlpha);
//   surface_32x32.clear();

//   surface_64x64.pen = Pen(255, 0, 255, uAlpha);
//   surface_64x64.clear();

//   surface_128x128.pen = Pen(255, 255, 0, uAlpha);
//   surface_128x128.clear();

// 	// set up profiler
// 	g_profiler.set_display_size(SCREEN_WIDTH, SCREEN_HEIGHT);
// 	g_profiler.set_rows(g_uRows);
// 	g_profiler.set_alpha(200);
// 	g_profiler.display_history(g_bDisplayHistory);

// 	// create probes
// 	g_pRenderProbe 	= g_profiler.add_probe("Render", 300); 	// 300 frames of history
// 	g_pClearProbe 	= g_profiler.add_probe("Clear", 50, 6); 	// Example of lower resolution, 50 samples with a span of 6 = 300 frames
// 	//g_pDma2dClearProbe 	= g_profiler.add_probe("DMA2d Clear", 300); 	// Example of lower resolution, 50 samples with a span of 6 = 300 frames
// 	//g_pRectProbe 		= g_profiler.add_probe("Rectangle", 300);
// 	//g_pCircleProbe 	= g_profiler.add_probe("Circle", 300);
// 	//g_pUpdateProbe 	= g_profiler.add_probe("Update");

//   g_pSurface_8x8 = g_profiler.add_probe("surface 8x8");
//   //g_pSprite_8x8 = g_profiler.add_probe("sprite  8x8");

//   g_pSurface_16x16 = g_profiler.add_probe("surface 16x16");
//   //g_pSprite_16x16 = g_profiler.add_probe("sprite  16x16");

//   g_pSurface_32x32 = g_profiler.add_probe("surface 32x32");
//   //g_pSprite_32x32 = g_profiler.add_probe("sprite  32x32");

//   g_pSurface_64x64 = g_profiler.add_probe("surface 64x64");
//   //g_pSprite_64x64 = g_profiler.add_probe("sprite  64x64");

//   g_pSurface_128x128 = g_profiler.add_probe("surface 128x128");
//   //g_pSprite_128x128 = g_profiler.add_probe("sprite  128x128");

//   // g_pSurface_320x240 = g_profiler.add_probe("surface 320x240");

//   // g_pSurface_rect = g_profiler.add_probe("surface rect");
// 	// g_pScreen_rect = g_profiler.add_probe("screen rect");
// 	// enable metrics
// 	SetupMetrics();
// }

// #pragma GCC push_options
// #pragma GCC optimize ("O3") 
// void render(uint32_t time)
// {
// 	static Point    ptMiddle(SCREEN_WIDTH/2, SCREEN_HEIGHT/2);
// 	static uint32_t lastButtons = 0;

// 	//static ProfilerProbe *pAwayRenderProbe = g_profiler.AddProbe("AwayFromRender");


// 	//pAwayRenderProbe->StoreElapsedUs(true);

// 	uint32_t changedButtons = buttons ^ lastButtons;

// 	bool button_a = buttons & changedButtons & Button::A;
// 	bool button_b = buttons & changedButtons & Button::B;
// 	bool button_x = buttons & changedButtons & Button::X;
// 	bool button_y = buttons & changedButtons & Button::Y;
// 	bool button_up = buttons & changedButtons & Button::DPAD_UP;
// 	bool button_down = buttons & changedButtons & Button::DPAD_DOWN;
// 	bool button_left = buttons & changedButtons & Button::DPAD_LEFT;
// 	bool button_right = buttons & changedButtons & Button::DPAD_RIGHT;
// 	bool button_home = buttons & changedButtons & Button::HOME;

// 	if(button_up && (g_uRows>1))
// 	{
// 		g_uRows --;
// 		g_profiler.set_rows(g_uRows);
// 	}

// 	if(button_down && (g_uRows<11))
// 	{
// 		g_uRows ++;
// 		g_profiler.set_rows(g_uRows);
// 	}

// 	if(button_left && (g_uPage > 1))
// 	{
// 		g_uPage--;
// 	}

// 	if(button_right && (g_uPage < g_profiler.get_page_count()))
// 	{
// 		g_uPage++;
// 	}

// 	if(button_a)
// 	{
// 		// Graphs
// 		g_bGraphEnabled = !g_bGraphEnabled;
// 		SetupMetrics();
// 	}


// 	if(button_b)
// 	{
// 		// Graphs
// 		g_bLabelsEnabled = !g_bLabelsEnabled;
// 		SetupMetrics();
// 	}

// 	if(button_x)
// 	{
// 		// Graphs
// 		g_bDisplayHistory = !g_bDisplayHistory;
// 		g_profiler.display_history(g_bDisplayHistory);
// 	}

// 	if(button_y)
// 	{
// 		// change Graphs from time set globally to time set locally
// 		g_bUseGlobalTime = !g_bUseGlobalTime;
// 	}

// 	if(button_home)
// 	{
// 		// Log to CDC current values
// 		g_profiler.log_probes();
// 	}

// 	lastButtons = buttons;

//   static bool bFirst = true;
//   if(bFirst)
//   {
//     screen.pen = Pen(0, 0, 0, 255);
//     screen.clear();
//     bFirst = false;
//   }

// 	g_pRenderProbe->start();

//   Pen UsePen = Pen(0,0,0,255);

// 	// // clear screen
//   screen.use_dma2d = false;
// 	g_pClearProbe->start();
//   screen.pen = UsePen;
//   screen.clear();
//   // uint32_t uSize = (320*240*3)/32;
//   // uint32_t *ptr = (uint32_t *)screen.data;
//   // while(uSize--)
//   // {
//   //   *ptr++=uSize;
//   //   *ptr++=uSize;
//   //   *ptr++=uSize;
//   //   *ptr++=uSize;
//   //   *ptr++=uSize;
//   //   *ptr++=uSize;
//   //   *ptr++=uSize;
//   //   *ptr++=uSize;
//   // }
//   g_pClearProbe->store_elapsed_us();


//   //screen.use_dma2d = false;

//   // checkerboard
//   // screen.pen = Pen(64, 64, 64, 255);
//   // for(int x = 0; x < 320; x+=8)
//   //   screen.line(Point(x,0), Point(x,240));
//   // for(int y = 0; y < 240; y+=8)
//   //   screen.line(Point(0,y), Point(320,y));
  


//   g_pSprite_8x8->start();
//   screen.sprite(Rect(0,0,1,1), Point(0,0));
//   g_pSprite_8x8->store_elapsed_us();

//   g_pSprite_16x16->start();
//   screen.sprite(Rect(0,0,2,2), Point(8,0));
//   g_pSprite_16x16->store_elapsed_us();

//   g_pSprite_32x32->start();
//   screen.sprite(Rect(0,0,4,4), Point(8*3,0));
//   g_pSprite_32x32->store_elapsed_us();

//   g_pSprite_64x64->start();
//   screen.sprite(Rect(0,0,8,8), Point(8*7,0));
//   g_pSprite_64x64->store_elapsed_us();

//   g_pSprite_128x128->start();
//   screen.sprite(Rect(0,0,16,16), Point(8*15,0));
//   g_pSprite_128x128->store_elapsed_us();

//   surface_8x8.use_dma2d = screen.use_dma2d;
//   surface_16x16.use_dma2d = screen.use_dma2d;
//   surface_32x32.use_dma2d = screen.use_dma2d;
//   surface_64x64.use_dma2d = screen.use_dma2d;
//   surface_128x128.use_dma2d = screen.use_dma2d;
//   //surface_320x240.use_dma2d = screen.use_dma2d;

//   surface_8x8.pen = UsePen;
//   surface_16x16.pen = UsePen;
//   surface_32x32.pen = UsePen;
//   surface_64x64.pen = UsePen;
//   surface_128x128.pen = UsePen;
//   //surface_320x240.pen = UsePen;

//   g_pSurface_8x8->start();
//   //surface_8x8.clear();
//   screen.blit(&surface_8x8, Rect(0,0,8,8), Point(0,0));
//   g_pSurface_8x8->store_elapsed_us();

//   g_pSurface_16x16->start();
//   //surface_16x16.clear();
//   screen.blit(&surface_16x16, Rect(0,0,16,16), Point(8,0));
//   g_pSurface_16x16->store_elapsed_us();

//   g_pSurface_32x32->start();
//   //surface_32x32.clear();
//   screen.blit(&surface_32x32, Rect(0,0,32,32), Point(8*3,0));
//   g_pSurface_32x32->store_elapsed_us();

//   g_pSurface_64x64->start();
//   //surface_64x64.clear();
//   screen.blit(&surface_64x64, Rect(0,0,64,64), Point(8*7,0));
//   g_pSurface_64x64->store_elapsed_us();

//   g_pSurface_128x128->start();
//   //surface_128x128.clear();
//   screen.blit(&surface_128x128, Rect(0,0,128,128), Point(8*15,0));
//   g_pSurface_128x128->store_elapsed_us();

//   // g_pSurface_320x240->start();
//   // //surface_320x240.clear();
//   // uSize = (320*240*3)/32;
//   // ptr = (uint32_t *)surface_320x240.data;
//   // while(uSize--)
//   // {
//   //   *ptr++=uSize;
//   //   *ptr++=uSize;
//   //   *ptr++=uSize;
//   //   *ptr++=uSize;
//   //   *ptr++=uSize;
//   //   *ptr++=uSize;
//   //   *ptr++=uSize;
//   //   *ptr++=uSize;
//   // }
//   // //screen.blit(&surface_128x128, Rect(0,0,128,128), Point(8*15,0));
//   // g_pSurface_320x240->store_elapsed_us();


//   // g_pSurface_rect->start();
//   // surface_320x240.rectangle(Rect(0,0,128,128));
//   // g_pSurface_rect->store_elapsed_us();


//   // g_pScreen_rect->start();
//   // screen.rectangle(Rect(0,0,128,128));
//   // g_pScreen_rect->store_elapsed_us();

// // 	g_pDma2dClearProbe->start();
// //   screen.pen = Pen(40,0,0, 255);
// //   screen.pen.use_dma2d = true;
// // //  screen.clear();
// //   screen.dma2d_clear();
// //   g_pDma2dClearProbe->store_elapsed_us();

// //   // draw rect
// //   g_pRectProbe->start();
// //  	screen.pen = Pen(0,0,255,255);
// //  	Point ptTl = Point(ptMiddle.x-((g_uSize>>16)/2), ptMiddle.y-((g_uSize>>16)/2));
// //  	Point ptBr = Point(ptMiddle.x+((g_uSize>>16)/2), ptMiddle.y+((g_uSize>>16)/2));
// //   screen.rectangle(Rect(ptTl, ptBr));
// //   g_pRectProbe->store_elapsed_us();

// //   // draw circle
// //   g_pCircleProbe->start();
// //  	screen.pen = Pen(255,0,0,128);
// //   screen.circle(ptMiddle, ((g_uSizeMax>>16)-(g_uSize>>16))/2);
// //   g_pCircleProbe->store_elapsed_us();


//   g_pRenderProbe->store_elapsed_us();

//   if(g_bUseGlobalTime)
//   {
//     // set global graph time to maximum render time
//   	g_profiler.set_graph_time(g_pRenderProbe->elapsed_metrics().uMaxElapsedUs);
//   }
//   else
//   {
//     // disable global graph time
//   	g_profiler.set_graph_time(0);

//   	// set All probes graph time to their maximum logged value
//   	g_pRenderProbe->set_graph_time_us_to_max();;
//   	g_pClearProbe->set_graph_time_us_to_max();;
//   	g_pRectProbe->set_graph_time_us_to_max();;
//   	g_pCircleProbe->set_graph_time_us_to_max();;
//   	g_pUpdateProbe->set_graph_time_us_to_max();;
//   }

//   // display the overlay
//   g_profiler.display_probe_overlay(g_uPage);

// 	// g_pDma2dClearProbe->start();
//   // screen.pen = Pen(255, 255, 255, 64);
//   // screen.dma2d_clear();
//   // g_pDma2dClearProbe->store_elapsed_us();

//   // // test pattern for lowres
//   // screen.pen = Pen(255,0,0,255);
//   // ptTl = Point(0,0);
//  	// ptBr = Point(160/2,120/2);
//   // screen.rectangle(Rect(ptTl, ptBr));
  
//   // screen.pen = Pen(0,255,0,255);
//   // ptTl = Point(160/2,0);
//  	// ptBr = Point(160,120/2);
//   // screen.rectangle(Rect(ptTl, ptBr));

//   // screen.pen = Pen(0,0,255,255);
//   // ptTl = Point(0,120/2);
//  	// ptBr = Point(160/2,120);
//   // screen.rectangle(Rect(ptTl, ptBr));

//   // screen.pen = Pen(255,255,255,255);
//   // ptTl = Point(160/2,120/2);
//  	// ptBr = Point(160,120);
//   // screen.rectangle(Rect(ptTl, ptBr));

// }
// #pragma GCC pop_options

// void update(uint32_t time)
// {
// 	// Scoped profiler probes automatically call Start() and at end of scope call StoreElapsedUs()
// 	ScopedProfilerProbe scopedProbe(g_pUpdateProbe);

//   g_uSize+=g_uSizeChange;
//   if(g_uSize >= g_uSizeMax)
//   {
//   	g_uSize -= (g_uSize-g_uSizeMax);
//   	g_uSizeChange = - g_uSizeChange;
//   }
//   else
//   {
//     if(g_uSize <= g_uSizeMin)
//     {
//     	g_uSize += g_uSizeMin-g_uSize;
//     	g_uSizeChange = - g_uSizeChange;
//     }
//   }

// }


// bool FlashLoader::StreamInit(CDCFourCC uCommand)
// {
// 	//printf("streamInit()\n\r");
// 	m_fPercent = 0.0f;
// 	bool bNeedStream = true;
// 	switch(uCommand)
// 	{
// 		case CDCCommandHandler::CDCFourCCMake<'P', 'R', 'O', 'G'>::value:
// 			m_state = stFlashCDC;
// 			m_parseState = stFilename;
// 			m_uParseIndex = 0;
// 		break;

// 		case CDCCommandHandler::CDCFourCCMake<'S', 'A', 'V', 'E'>::value:
// 			m_state = stSaveFile;
// 			m_parseState = stFilename;
// 			m_uParseIndex = 0;
// 		break;

// 		case CDCCommandHandler::CDCFourCCMake<'_', '_', 'L', 'S'>::value:
// 			m_state = stLS;
// 			bNeedStream = false;
// 		break;

// 	}
// 	return bNeedStream;
// }
