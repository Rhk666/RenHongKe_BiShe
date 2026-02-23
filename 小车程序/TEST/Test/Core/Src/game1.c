#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "main.h"
#include "draw.h"
#include "resources.h"
#include "OLED.h"

#define NOINVERT	false
#define INVERT		true

#define sprintf_P  sprintf
#define PSTR(a)    a
#define UPT_MOVE_NONE	0

#define PLATFORM_WIDTH	12
#define PLATFORM_HEIGHT	4
#define BLOCK_COLS		32
#define BLOCK_ROWS		5
#define BLOCK_COUNT		(BLOCK_COLS * BLOCK_ROWS)
#define TOP_BOUNDARY    10
// 新增：砖块实际尺寸（保证完整）
#define BLOCK_WIDTH     3
#define BLOCK_HEIGHT    8

typedef struct{
	float x;
	float y;
	float velX;
	float velY;
}s_ball;

// 砖块位图（完整3列，保证显示完整）
static const byte block[] ={0x07, 0x07, 0x07};
static const byte platform[] ={0x60,0x70,0x50,0x10,0x30,0xF0,0xF0,0x30,0x10,0x50,0x70,0x60};
static const byte ballImg[] ={0x03, 0x03};
static const byte clearImg[] ={0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
static const byte livesImg[] ={0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF};

void game1_draw(void);

static s_ball ball;
static bool* blocks;
static byte lives, lives_origin;
static uint score;
static byte platformX;
static byte uptMove;

static uint32_t g_xres, g_yres, g_bpp;
static uint8_t *g_framebuffer;

void Game_Vri_Init(){
    g_framebuffer = LCD_GetFrameBuffer(&g_xres, &g_yres, &g_bpp);
    draw_init();
    draw_end();
	
	uptMove = UPT_MOVE_NONE;
	ball.x = g_xres / 2;
	ball.y = g_yres - 10;
	ball.velX = -2.5;
	ball.velY = -2.6;

	blocks = (bool*)malloc(BLOCK_COUNT);
	memset(blocks, 0, BLOCK_COUNT);
	lives = lives_origin = 3;
	score = 0;
	platformX = (g_xres / 2) - (PLATFORM_WIDTH / 2);
}

void game1_draw()
{
	bool gameEnded = ((score >= BLOCK_COUNT) || (lives == 255));
	byte platformXtmp = platformX;
    static bool first = 1;

	// 砖块全消重置（保留）
	if(score >= BLOCK_COUNT)
	{
		score = 0;					
		memset(blocks, 0, BLOCK_COUNT);
		first = 1;					
	}

	// 擦旧球
	draw_bitmap(ball.x, ball.y, clearImg, 2, 2, NOINVERT, 0);
    draw_flushArea(ball.x, ball.y, 2, 8);
	
	if(!gameEnded)
	{
		ball.x += ball.velX;
		ball.y += ball.velY;

        // 小球不飞顶端（保留）
        if(ball.y < TOP_BOUNDARY)
        {
            ball.y = TOP_BOUNDARY;
            ball.velY = -ball.velY;
        }
	}

	bool blockCollide = false;
	const float ballX = ball.x;
	const byte ballY = ball.y;

	// 砖块碰撞（核心修复：碰撞范围匹配砖块完整尺寸3×8）
	byte idx = 0;
	LOOP(BLOCK_COLS, x)
	{
		LOOP(BLOCK_ROWS, y)
		{
			uint16_t bx = x * 4;
			uint16_t by = y * 4 + 8;
			// 修复：碰撞范围从bx+4→bx+BLOCK_WIDTH，by+4→by+BLOCK_HEIGHT
			if(!blocks[idx] && ballX >= bx && ballX < bx + BLOCK_WIDTH 
			   && ballY >= by && ballY < by + BLOCK_HEIGHT)
			{
				blocks[idx] = true;
				// 清空完整砖块（3×8）
				draw_bitmap(bx, by, clearImg, BLOCK_WIDTH, BLOCK_HEIGHT, NOINVERT, 0);
                draw_flushArea(bx, by, BLOCK_WIDTH, BLOCK_HEIGHT);
				blockCollide = true;
				score++;
			}
			idx++;
		}
	}

	// 左右墙反弹（保留）
	if(ballX > g_xres - 2)
	{
		ball.x = (ballX > 240) ? 0 : (g_xres - 2);
		ball.velX = -ball.velX;		
	}
	if(ballX < 0)
    {
		ball.x = 0;		
		ball.velX = -ball.velX;	
    }

	// 挡板碰撞（保留）
	bool platformCollision = false;
	if(!gameEnded && ballY >= g_yres - PLATFORM_HEIGHT - 2 && ballY < 240 
	   && ballX >= platformX && ballX <= platformX + PLATFORM_WIDTH)
	{
		platformCollision = true;
		ball.y = g_yres - PLATFORM_HEIGHT - 2;
		ball.velY = (ball.velY > 0) ? -ball.velY : ball.velY;
		ball.velX = ((float)rand() / (RAND_MAX / 2)) - 1;
	}

	// 上下墙（保留）
	if(!gameEnded && !platformCollision && (ballY > g_yres - 2 || blockCollide))
	{
		if(ballY > 240) ball.y = 0;
		else if(!blockCollide) { ball.y = g_yres - 1; lives=lives; }
		ball.velY *= -1;
	}

	// 画新球（保留）
	draw_bitmap(ball.x, ball.y, ballImg, 2, 2, NOINVERT, 0);
    draw_flushArea(ball.x, ball.y, 2, 8);

	// 画挡板（保留）
    draw_bitmap(platformX, g_yres - 8, platform, 12, 8, NOINVERT, 0);
    draw_flushArea(platformX, g_yres - 8, 12, 8);

    // 首次画砖块（绘制完整3×8砖块）
    if (first)
    {
        first = 0;
    	idx = 0;
    	LOOP(BLOCK_COLS, x)
    	{
    		LOOP(BLOCK_ROWS, y)
    		{
    			uint16_t bx = x * 4;
    			uint16_t by = y * 4 + 8;
    			// 清空完整区域
    			draw_bitmap(bx, by, clearImg, BLOCK_WIDTH, BLOCK_HEIGHT, NOINVERT, 0);
    			draw_flushArea(bx, by, BLOCK_WIDTH, BLOCK_HEIGHT);
    			
    			if(!blocks[idx])
    			{
    				// 绘制完整3×8砖块
    				draw_bitmap(bx, by, block, BLOCK_WIDTH, BLOCK_HEIGHT, NOINVERT, 0);
                    draw_flushArea(bx, by, BLOCK_WIDTH, BLOCK_HEIGHT);
    			}
    			idx++;
    		}
    	}
    }

	// 得分（保留）
	char buff[6];
	sprintf_P(buff, PSTR("%u"), score);
	draw_string(buff, false, 0, 0);

    // 生命（保留）
    if(lives != 255)
    {
        LOOP(lives_origin, i)
        {
            uint16_t lx = (g_xres - 24) + (8*i);
            draw_bitmap(lx, 1, (i < lives) ? livesImg : clearImg, 7, 8, NOINVERT, 0);
            draw_flushArea(lx, 1, 7, 8);    
        }
    }   

	// 游戏结束（保留）
	if(lives == 255)
		draw_string(STR_GAMEOVER, false, 34, 32);
}