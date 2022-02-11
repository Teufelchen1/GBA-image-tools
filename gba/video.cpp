#include <gba_base.h>
#include <gba_input.h>
#include <gba_interrupt.h>
#include <gba_systemcalls.h>
#include <gba_timers.h>
#include <gba_video.h>

#include "base.h"
#include "dma.h"
#include "memory.h"
#include "output.h"
#include "tui.h"
#include "videodecoder.h"
#include "videoreader.h"

#include "data/data.h"

IWRAM_DATA bool frameRequested = true;

IWRAM_FUNC void frameRequest()
{
	frameRequested = true;
}

EWRAM_DATA ALIGN(4) uint32_t ScratchPad[19208 * 2 / 4]; // scratch pad memory for decompression. ideally we would dynamically allocate this

int main()
{
	// set waitstates for GamePak ROM and EWRAM
	Memory::RegWaitCnt = Memory::WaitCntFast;
	Memory::RegWaitEwram = Memory::WaitEwramNormal;
	// start wall clock
	irqInit();
	// set up text UI
	TUI::setup();
	TUI::fillBackground(TUI::Color::Black);
	// read file header
	const auto videoInfo = Video::GetInfo(reinterpret_cast<const uint32_t *>(VIDEO_DATA));
	// print video info
	TUI::printf(0, 0, "Frames: %d, Fps: %d", videoInfo.nrOfFrames, videoInfo.fps);
	TUI::printf(0, 1, "Size: %dx%d", videoInfo.width, videoInfo.height);
	TUI::printf(0, 2, "Bits / pixel: %d", videoInfo.bitsPerPixel);
	TUI::printf(0, 3, "Colors in colormap: %d", videoInfo.colorMapEntries);
	TUI::printf(0, 4, "Bits / color: %d", videoInfo.bitsInColorMap);
	TUI::printf(0, 5, "Memory needed: %d", videoInfo.maxMemoryNeeded);
	// wait for keypress
	do
	{
		scanKeys();
		if (keysDown() & KEY_A)
		{
			break;
		}
	} while (true);
	// switch video mode to 160x128x2
	REG_DISPCNT = MODE_3 | BG2_ON;
	// REG_BG2PA = 256 / 1.5;
	// REG_BG2PD = 256 / 1.5;
	// REG_BG2Y = 11 << 8;
	//  set up timer to increase with frame interval
	irqSet(irqMASKS::IRQ_TIMER3, frameRequest);
	irqEnable(irqMASKS::IRQ_TIMER3);
	// Timer divider 2 == 256 -> 16*1024*1024 cycles/s / 256 = 65536/s
	REG_TM3CNT_H = TIMER_START | TIMER_IRQ | 2;
	// Timer interval = 1 / fps (where 65536 == 1s)
	REG_TM3CNT_L = 65536 - (65536 / videoInfo.fps);
	// start main loop
	Video::Frame frame{};
	do
	{
		// wait for the timer to signal a frame request
		/*while (!frameRequested)
		{
		};
		frameRequested = false;*/
		REG_TM2CNT_L = 0;
		REG_TM2CNT_H = TIMER_START | 2;
		// read next frame from data
		frame = Video::GetNextFrame(videoInfo, frame);
		// uncompress frame
		Video::decode((uint32_t *)VRAM, ScratchPad, sizeof(ScratchPad), videoInfo, frame);
		REG_TM2CNT_H = 0;
		auto durationMs = static_cast<int32_t>(REG_TM2CNT_L) * 1000;
		Debug::printf("Frame %d, Needed: %f ms", frame.index, durationMs);
	} while (true);
	return 0;
}
