#include "graphics-benchmark.hpp"
#include "graphics/color.hpp"
#include "engine/profiler.hpp"
#include <cmath>

using namespace blit;

#define SCREEN_WIDTH 320
#define SCREEN_HEIGHT 240

Profiler 			g_profiler; // global uRunningAverageSize and uRunningAverageSpan could be set here.
ProfilerProbe *g_pRenderProbe;
ProfilerProbe *g_pClearProbe;
ProfilerProbe *g_pRectOldProbe;
ProfilerProbe *g_pRectNewProbe;


uint32_t g_uSize = 2;
uint32_t g_uSizeMax	= SCREEN_HEIGHT-1;
uint32_t g_uSizeMin	= 2;
uint32_t g_uSizeChange = 1;

bool g_bGraphEnabled   = true;
bool g_bLabelsEnabled  = true;
bool g_bDisplayHistory = false;
bool g_bUseGlobalTime	 = true;

uint8_t g_uRows = 4;
uint8_t g_uPage = 1;



#define RANDX random()%SCREEN_WIDTH
#define RANDY random()%SCREEN_HEIGHT
#define RANDC (int)(random()%256)

void SetupMetrics(void)
{
	g_profiler.SetupGraphElement(Profiler::dmCur, g_bLabelsEnabled, g_bGraphEnabled, Pen(0,255,0));
	g_profiler.SetupGraphElement(Profiler::dmAvg, g_bLabelsEnabled, g_bGraphEnabled, Pen(0,255,255));
	g_profiler.SetupGraphElement(Profiler::dmMax, g_bLabelsEnabled, g_bGraphEnabled, Pen(255,0,0));
	g_profiler.SetupGraphElement(Profiler::dmMin, g_bLabelsEnabled, g_bGraphEnabled, Pen(255,255,0));
}

void init()
{
	set_screen_mode(ScreenMode::hires);

	// set up profiler
	g_profiler.SetDisplaySize(SCREEN_WIDTH, SCREEN_HEIGHT);
	g_profiler.SetRows(g_uRows);
	g_profiler.SetAlpha(200);
	g_profiler.DisplayHistory(g_bDisplayHistory);

	// create probes
	g_pRenderProbe 	= g_profiler.AddProbe("Render", 300); 	// 300 frames of history
	g_pClearProbe 	= g_profiler.AddProbe("Clear", 300);
	g_pRectOldProbe = g_profiler.AddProbe("Rectangle Old", 300);
	g_pRectNewProbe = g_profiler.AddProbe("Rectangle New", 300);

	// enable metrics
	SetupMetrics();
}


void render(uint32_t time)
{
	static Point    ptMiddle(SCREEN_WIDTH/2, SCREEN_HEIGHT/2);
	static uint32_t lastButtons = 0;

	//static ProfilerProbe *pAwayRenderProbe = g_profiler.AddProbe("AwayFromRender");


	//pAwayRenderProbe->StoreElapsedUs(true);

	uint32_t changedButtons = buttons ^ lastButtons;

	bool button_a = buttons & changedButtons & Button::A;
	bool button_b = buttons & changedButtons & Button::B;
	bool button_x = buttons & changedButtons & Button::X;
	bool button_y = buttons & changedButtons & Button::Y;
	bool button_up = buttons & changedButtons & Button::DPAD_UP;
	bool button_down = buttons & changedButtons & Button::DPAD_DOWN;
	bool button_left = buttons & changedButtons & Button::DPAD_LEFT;
	bool button_right = buttons & changedButtons & Button::DPAD_RIGHT;
	bool button_home = buttons & changedButtons & Button::HOME;

	if(button_up && (g_uRows>1))
	{
		g_uRows --;
		g_profiler.SetRows(g_uRows);
	}

	if(button_down && (g_uRows<11))
	{
		g_uRows ++;
		g_profiler.SetRows(g_uRows);
	}

	if(button_left && (g_uPage > 1))
	{
		g_uPage--;
	}

	if(button_right && (g_uPage < g_profiler.GetPageCount()))
	{
		g_uPage++;
	}

	if(button_a)
	{
		// Graphs
		g_bGraphEnabled = !g_bGraphEnabled;
		SetupMetrics();
	}


	if(button_b)
	{
		// Graphs
		g_bLabelsEnabled = !g_bLabelsEnabled;
		SetupMetrics();
	}

	if(button_x)
	{
		// Graphs
		g_bDisplayHistory = !g_bDisplayHistory;
		g_profiler.DisplayHistory(g_bDisplayHistory);
	}

	if(button_y)
	{
		// change Graphs from time set globally to time set locally
		g_bUseGlobalTime = !g_bUseGlobalTime;
	}

	if(button_home)
	{
		// Log to CDC current values
		g_profiler.LogProbes();
	}

	lastButtons = buttons;

	g_pRenderProbe->Start();

	// clear screen
	g_pClearProbe->Start();
  screen.pen = Pen(0, 0, 0, 255);
  screen.clear();
  g_pClearProbe->StoreElapsedUs();

  // draw rect
 	screen.pen = Pen(255,0,0,255);
// 	screen.pen = Pen(255,255,255,255);
// 	screen.pen = Pen(255,255,255,255);
// 	screen.pen = Pen(255,255,255,255);
  g_pRectOldProbe->Start();
 	Point ptTl = Point(0,0);
 	Point ptBr = Point(100,100);
  screen.rectangle(Rect(ptTl, ptBr));
  g_pRectOldProbe->StoreElapsedUs();


  g_pRectNewProbe->Start();
 	ptTl = Point(0,110);
 	ptBr = Point(100,210);
  screen.rectangle_optimised(Rect(ptTl, ptBr));
  g_pRectNewProbe->StoreElapsedUs();


  g_pRenderProbe->StoreElapsedUs();

 	g_profiler.SetGraphTime(g_pRenderProbe->ElapsedMetrics().uMaxElapsedUs);

  // display the overlay
  g_profiler.DisplayProbeOverlay(g_uPage);

}

void update(uint32_t time)
{
}
