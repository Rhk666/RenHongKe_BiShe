/*
 * Project: N|Watch
 * Author: Zak Kemble
 * 原始原版：打砖块 + 得分 + 时钟 + 无WiFi
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "cmsis_os.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "event_groups.h"
#include "semphr.h"

#include "draw.h"
#include "resources.h"

#define NOINVERT	false
#define INVERT		true

#define sprintf_P  sprintf
#define PSTR(a)  a

#define PLATFORM_WIDTH	12
#define PLATFORM_HEIGHT	4
#define UPT_MOVE_NONE	0
#define UPT_MOVE_RIGHT	1
#define UPT_MOVE_LEFT	2
#define BLOCK_COLS		32
#define BLOCK_ROWS		5
#define BLOCK_COUNT		(BLOCK_COLS * BLOCK_ROWS)

typedef struct{
	float x;
	float y;
	float velX;
	float velY;
}s_ball;

static const byte block[] ={
	0x07,0x07,0x07,
};

static const byte platform[] ={
	0x60,0x70,0x50,0x10,0x30,0xF0,0xF0,0x30,0x10,0x50,0x70,0x60,
};

static const byte ballImg[] ={
	0x03,0x03,
};

static const byte clearImg[] ={
	0,0,0,0,0,0,0,0,0,0,0,0,
};

static bool btnExit(void);
static bool btnRight(void);
static bool btnLeft(void);
void game1_draw(void);

uint8_t uptMove;
static s_ball ball;
static bool* blocks;
static byte lives, lives_origin;
static uint score;
static byte platformX;

// ===== 时钟全局变量 =====
static uint8_t clock_h = 0;
static uint8_t clock_m = 0;
static uint8_t clock_s = 0;
static TickType_t last_clock_tick = 0;
static uint8_t last_clock_s = 255;
// ========================

static uint32_t g_xres, g_yres, g_bpp;
static uint8_t *g_framebuffer;
QueueHandle_t g_xQueuePlatform;

/* 挡板任务 */
static void platform_task(void *params)
{
    byte platformXtmp = platformX;    

    draw_bitmap(platformXtmp, g_yres - 8, platform, 12, 8, NOINVERT, 0);
    draw_flushArea(platformXtmp, g_yres - 8, 12, 8);
    
    while (1)
    {
        if(uptMove != UPT_MOVE_NONE)
        {
            draw_bitmap(platformXtmp, g_yres - 8, clearImg, 12, 8, NOINVERT, 0);
            draw_flushArea(platformXtmp, g_yres - 8, 12, 8);
            
            if(uptMove == UPT_MOVE_RIGHT)
                platformXtmp += 3;
            else if(uptMove == UPT_MOVE_LEFT)
                platformXtmp -= 3;
            
            uptMove = UPT_MOVE_NONE;
            
            if(platformXtmp > g_xres - PLATFORM_WIDTH)
                platformXtmp = g_xres - PLATFORM_WIDTH;
            if(platformXtmp < 0)
                platformXtmp = 0;
            
            draw_bitmap(platformXtmp, g_yres - 8, platform, 12, 8, NOINVERT, 0);
            draw_flushArea(platformXtmp, g_yres - 8, 12, 8);
            
            platformX = platformXtmp;
        }
        vTaskDelay(pdMS_TO_TICKS(5));
    }
}

void game1_task(void *params)
{		
    draw_init();
    g_framebuffer = LCD_GetFrameBuffer(&g_xres, &g_yres, &g_bpp);
    draw_init();
    draw_end();
	
	g_xQueuePlatform = xQueueCreate(10, sizeof(struct input_data));
    
	uptMove = UPT_MOVE_NONE;
	ball.x = g_xres / 2;
	ball.y = g_yres - 10;
	ball.velX = -4.5;
	ball.velY = -4.6;

	blocks = pvPortMalloc(BLOCK_COUNT);
    memset(blocks, 0, BLOCK_COUNT);
	
	lives = lives_origin = 1;
	score = 0;
	platformX = (g_xres / 2) - (PLATFORM_WIDTH / 2);

    clock_h = 0;
    clock_m = 0;
    clock_s = 0;
    last_clock_tick = xTaskGetTickCount();
    last_clock_s = 255;

    xTaskCreate(platform_task, "platform_task", 128, NULL, osPriorityNormal, NULL);

    while (1)
    {
        game1_draw();
        vTaskDelay(50);
    }
}

static bool btnExit()
{
	vPortFree(blocks);
	vTaskDelete(NULL);
	return true;
}

static bool btnRight()
{
	uptMove = UPT_MOVE_RIGHT;
	return false;
}

static bool btnLeft()
{
	uptMove = UPT_MOVE_LEFT;
	return false;
}

void game1_draw()
{
    static bool first = 1;
    static bool force_redraw = 0;

    if(score >= BLOCK_COUNT)
    {
        score = 0;
        memset(blocks, 0, BLOCK_COUNT);
        ball.x = g_xres / 2;
        ball.y = g_yres - 10;
        ball.velX = -4.5;
        ball.velY = -4.6;
        memset(g_framebuffer, 0, 1024);
        draw_end();
        force_redraw = 1;
        first = 1;
    }

	bool gameEnded = ((score >= BLOCK_COUNT) || (lives == 255));
	byte platformXtmp = platformX;

	draw_bitmap(ball.x, ball.y, clearImg, 2, 2, NOINVERT, 0);
    draw_flushArea(ball.x, ball.y, 2, 8);
	
	if(!gameEnded)
	{
		ball.x += ball.velX;
		ball.y += ball.velY;
	}

	bool blockCollide = false;
	const float ballX = ball.x;
	const byte ballY = ball.y;

	byte idx = 0;
	LOOP(BLOCK_COLS, x)
	{
		LOOP(BLOCK_ROWS, y)
		{
			if(!blocks[idx] && ballX >= x*4 && ballX < x*4+4 
			   && ballY >= y*4+12 && ballY < y*4+12+4)
			{
				blocks[idx] = true;
                draw_bitmap(x*4, y*4+12, clearImg, 3, 8, NOINVERT, 0);
                draw_flushArea(x*4, y*4+12, 3, 8);
				blockCollide = true;
				score++;
			}
			idx++;
		}
	}

	if(ballX > g_xres - 2)
	{
		ball.x = g_xres - 2;
		ball.velX = -ball.velX;		
	}
	if(ballX < 0)
    {
		ball.x = 0;		
		ball.velX = -ball.velX;	
    }

	bool platformCollision = false;
	if(!gameEnded && ballY >= g_yres - PLATFORM_HEIGHT - 2 && ballY < 240 
	   && ballX >= platformX && ballX <= platformX + PLATFORM_WIDTH)
	{
		platformCollision = true;
		ball.y = g_yres - PLATFORM_HEIGHT - 2;
		if(ball.velY > 0)
			ball.velY = -ball.velY;
		ball.velX = ((float)rand() / (RAND_MAX / 2)) - 1;
	}

	if(!gameEnded && !platformCollision)
	{
		if(ballY < 12 || blockCollide)
		{
			if(ballY < 12)
				ball.y = 12;
			ball.velY *= -1;
		}
		else if(ballY > g_yres - 2)
		{
			if(ballY > 240)
				ball.y = 0;
			else
				ball.y = g_yres - 1;
			ball.velY *= -1;
		}
	}

	draw_bitmap(ball.x, ball.y, ballImg, 2, 2, NOINVERT, 0);
    draw_flushArea(ball.x, ball.y, 2, 8);

    if (first || force_redraw)
    {
        first = 0;
        force_redraw = 0;
    	idx = 0;
    	LOOP(BLOCK_COLS, x)
    	{
    		LOOP(BLOCK_ROWS, y)
    		{
    			if(!blocks[idx])
    			{
    				draw_bitmap(x*4, y*4+12, block, 3, 8, NOINVERT, 0);
    				draw_flushArea(x*4, y*4+12, 3, 8);
    			}
    			idx++;
    		}
    	}
    }

	// ======================
	// 时钟（靠右显示）
	// ======================
	TickType_t now = xTaskGetTickCount();
	if(now - last_clock_tick >= pdMS_TO_TICKS(1000))
	{
	    last_clock_tick = now;
	    clock_s++;
	    if(clock_s >= 60) { clock_s = 0; clock_m++; }
	    if(clock_m >= 60) { clock_m = 0; clock_h++; }
	    if(clock_h >= 24) clock_h = 0;
	}

	if(clock_s != last_clock_s)
	{
	    char clock_buff[9];
	    sprintf_P(clock_buff, PSTR("%02d:%02d:%02d"), clock_h, clock_m, clock_s);
	    byte clock_x = g_xres - 60;
	    draw_clearArea(clock_x, 0, 58);
	    draw_string(clock_buff, false, clock_x, 0);
	    last_clock_s = clock_s;
	}

	// ======================
	// 显示得分（左上角）
	// ======================
	char score_buf[16];
	sprintf(score_buf, "%d", score);
	draw_clearArea(0, 0, 60);
	draw_string(score_buf, false, 0, 0);
}