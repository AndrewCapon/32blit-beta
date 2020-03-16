#pragma once

#include "32blit.hpp"
#include "engine/CDCCommandHandler.h"
#include "engine/CDCCommandStream.h"
#include "engine/CDCDataStream.h"
#include "engine/profiler.hpp"

#define SCREEN_WIDTH 320
#define SCREEN_HEIGHT 240

extern CDCCommandStream g_commandStream;

class DMA2DTestUnit
{
public:
  typedef std::function<void(void)> TestLambda;

  typedef std::vector<DMA2DTestUnit *>            TestUnits;
  typedef std::vector<DMA2DTestUnit *>::iterator  TestUnitsIter;
  
  DMA2DTestUnit(std::string sName, TestLambda testLambda, blit::Profiler *pProfiler) : m_sName(sName), m_testLambda(testLambda), m_pProfiler(pProfiler)
  {
    m_pProbe = m_pProfiler->add_probe(m_sName.c_str());
  }

  void Test()
  {
    m_pProbe->start();
    m_testLambda();
    m_pProbe->store_elapsed_us();
  }

private:
  blit::ProfilerProbe *m_pProbe = NULL;
  std::string         m_sName;
  TestLambda          m_testLambda;
  blit::Profiler      *m_pProfiler;
};

class DMA2DTest : public CDCCommandHandler
{
public:
  DMA2DTest();

	virtual StreamResult StreamData(CDCDataStream &dataStream);
	virtual bool StreamInit(CDCFourCC uCommand);

	void Init(void);
	void Render(uint32_t time);
	void Update(uint32_t time);

private:
  DMA2DTestUnit::TestUnits  m_tests;
  blit::Profiler            *m_pProfiler;
  uint32_t                  m_surfaceBuffer[256*256];
  blit::Pen                 m_pen;

  void AddTest(std::string sName, DMA2DTestUnit::TestLambda testLambda)
  {
    m_tests.push_back(new DMA2DTestUnit(sName, testLambda, m_pProfiler));
  }
};