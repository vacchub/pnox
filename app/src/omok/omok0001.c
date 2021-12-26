#include <stdio.h>
#include "pnox.h"

#define MAX_ROW		15
#define MAX_PCNT        10
#define MAX_NICE        99

#define DEFENSE_RATE    5

#define MAN     121
#define COM     122

typedef struct {
        int x;
        int y;
} pair;

struct io {
	char stat[1];			/* 0:non, 1:man win, 2:com win, 3:close	*/
	char posx[1];			/* last point x				*/
	char posy[1];			/* last point y				*/
	char board[MAX_ROW][MAX_ROW];	/* 'E':empty, 'W':white, 'B':black	*/
};
struct io *mid, *mod;

struct pattern 
{
	char	flag;
	char	p[MAX_PCNT];
} pattern[200];

char board[MAX_ROW][MAX_ROW]={0,};

int	init_pattern();
void	ai_npgo(char [][MAX_ROW], pair *);
void	ai_pattern(char [][MAX_ROW], struct pattern *, char, int);
int	ai_pcount(struct pattern *);
int	ai_nice(char (*)[MAX_ROW], int, int, int);
int	ai_maxnice(char [][MAX_ROW]);
void	ai_determine(char [][MAX_ROW], pair *);
int	ai_isend(char [][MAX_ROW], char);

/***********************************************************************
 * omok0001.c
 **********************************************************************/
int omok0001(tHandle, mHandle)
T_HAND *tHandle;
M_HAND *mHandle;
{
	int ii, jj, retc;
	int check=0;
	pair acur;
	char rcvb[1024], sndb[1024];

	retc = init_pattern();
	if (retc < 0)
		return(-1);

	memset(rcvb, 0x00, sizeof(rcvb));
	memcpy(rcvb, mHandle->rcvb, mHandle->rcvl);
	mid = (struct io *)rcvb;

	memset(sndb, 0x00, sizeof(sndb));
	mod = (struct io *)sndb;

	for (ii = 0; ii < MAX_ROW; ii++)
	{
		for (jj = 0; jj < MAX_ROW; jj++)
		{
			if (mid->board[ii][jj] == 'B')
				board[ii][jj] = MAN;
			else if (mid->board[ii][jj] == 'W')
				board[ii][jj] = COM;
			else if (mid->board[ii][jj] == 'E')
				board[ii][jj] = 0;
		}
	}

	check = ai_isend(board, MAN);
	if(check != 0) 
	{
		/* MAN win	*/
		mod->stat[0] = 1;
	}
	else
	{
		mod->stat[0] = 0;
		ai_npgo(board, &acur);
		board[acur.y][acur.x] = COM;
		mod->posx[0] = acur.x;
		mod->posy[0] = acur.y;
		check = ai_isend(board, COM);
		if(check != 0) 
		{
			/* COM win	*/
			mod->stat[0] = 2;
		}
	}

	for (ii = 0; ii < MAX_ROW; ii++)
	{
		for (jj = 0; jj < MAX_ROW; jj++)
		{
			if (board[ii][jj] == MAN)
				mod->board[ii][jj] = 'B';
			else if (board[ii][jj] == COM)
				mod->board[ii][jj] = 'W';
			else if (board[ii][jj] == 0)
				mod->board[ii][jj] = 'E';
		}
	}
	
	memcpy(mHandle->sndb, sndb, sizeof(struct io));
	mHandle->sndl = sizeof(struct io);
	return(0);
}

/***********************************************************************
 * init_pattern()
 **********************************************************************/
int init_pattern()
{
	int ii=0, jj=0;
	char buff[1024];
	char tmpb[11][5];
	FILE *fp;

	fp = fopen("/pnox/app/cfg/omok.cfg", "r");
	if (fp == NULL)
		return(-1);

	while ((fgets(buff, sizeof(buff), fp)) == buff)
	{
		if (buff[0] == '#' || buff[0] == '\n')
			continue;

		buff[strlen(buff)-1] = 0x00;

		memset(tmpb, 0x00, sizeof(tmpb));
		sscanf(buff, "%s %s %s %s %s %s %s %s %s %s %s", tmpb[0],
			tmpb[1], tmpb[2], tmpb[3], tmpb[4], tmpb[5],
			tmpb[6], tmpb[7], tmpb[8], tmpb[9], tmpb[10]);

		if (tmpb[0][0] != 'D' && tmpb[0][0] != 'A')
			continue;

		pattern[ii].flag = tmpb[0][0];
		for (jj = 0; jj < MAX_PCNT; jj++)
		{
			if (tmpb[jj+1][0] == 'M')
			{
				pattern[ii].p[jj] = MAN;
				continue;
			}
			else if (tmpb[jj+1][0] == 'C')
			{
				pattern[ii].p[jj] = COM;
				continue;
			}

			pattern[ii].p[jj] = atoi(tmpb[jj+1]);		
		}
		ii++;	
	}

	fclose(fp);
	return(0);
}

/***********************************************************************
 * ai_npgo()
 **********************************************************************/
void ai_npgo(char board[][MAX_ROW], pair *acur)
{
	char	t_board[MAX_ROW][MAX_ROW];
	int 	ii;
	int		max;

	memcpy(t_board, board, sizeof(char) * MAX_ROW * MAX_ROW);
	
	for (ii = 0; pattern[ii].flag != 'D' && pattern[ii].flag != 'A'; ii++)
	{
		ai_pattern(t_board, &pattern[ii], 'D', 1);
		max = ai_maxnice(t_board);
		if (max < DEFENSE_RATE)
			ai_pattern(t_board, &pattern[ii], 'A', 3);
		else
			ai_pattern(t_board, &pattern[ii], 'A', 1);
	}
	
	ai_determine(t_board, acur);
} 

/***********************************************************************
 * ai_pattern()
 **********************************************************************/
void ai_pattern(char b[][MAX_ROW], struct pattern *ptr, char mode, int rate)
{
	int ii, jj, kk, pcnt, match;
	
	pcnt = ai_pcount(ptr);

	for (ii = 0; ii < MAX_ROW; ii++)
	{
		for (jj = 0; jj < MAX_ROW; jj++)
		{
			match = 0;
			for (kk = 0; kk < pcnt; kk++)
			{
				if (ptr->flag != mode)
					break;
				if (jj + kk >= MAX_ROW)
					break;
				if (ptr->p[kk] != 0 && ptr->p[kk] <= MAX_NICE)
				{
					if (b[ii][jj+kk] <= MAX_NICE)
					{
						match++;
						continue;
					}
					else
						break;
				}
				if (ptr->p[kk] == MAN)
				{
					if (b[ii][jj+kk] == MAN)
					{
						match++;
						continue;
					}
					else
						break;
				}
				if (ptr->p[kk] == COM)
				{
					if (b[ii][jj+kk] == COM)
					{
						match++;
						continue;
					}
					else
						break;
				}
				else
					break;
			}
			if (pcnt == match)
			{
				for (kk = 0; kk < pcnt; kk++)
				{
					if (ptr->p[kk] != 0 && ptr->p[kk] <= MAX_NICE)
						ai_nice(b, ii, jj+kk, ptr->p[kk] * rate);
				}
			}
		}
	}
	
	for (ii = 0; ii < MAX_ROW; ii++)
	{
		for (jj = 0; jj < MAX_ROW; jj++)
		{
			match = 0;
			for (kk = 0; kk < pcnt; kk++)
			{
				if (ptr->flag != mode)
					break;
				if (jj + kk >= MAX_ROW)
					break;
				if (ptr->p[kk] != 0 && ptr->p[kk] <= MAX_NICE)
				{
					if (b[jj+kk][ii] <= MAX_NICE)
					{
						match++;
						continue;
					}
					else
						break;
				}
				if (ptr->p[kk] == MAN)
				{
					if (b[jj+kk][ii] == MAN)
					{
						match++;
						continue;
					}
					else
						break;
				}
				if (ptr->p[kk] == COM)
				{
					if (b[jj+kk][ii] == COM)
					{
						match++;
						continue;
					}
					else
						break;
				}
				else
					break;
			}
			if (pcnt == match)
			{
				for (kk = 0; kk < pcnt; kk++)
				{
					if (ptr->p[kk] != 0 && ptr->p[kk] <= MAX_NICE)
						ai_nice(b, jj+kk, ii, ptr->p[kk] * rate);
				}
			}
		}
	}

	for (ii = 0; ii < MAX_ROW; ii++)	
	{
		for (jj = 0; jj < MAX_ROW; jj++)
		{
			match = 0;
			for (kk = 0; kk < pcnt; kk++)
			{
				if (ptr->flag != mode)
					break;
				if (ii + kk < 0 || ii + kk >= MAX_ROW)
					break;
				if (jj - kk < 0 || jj - kk >= MAX_ROW)
					break;
				if (ptr->p[kk] != 0 && ptr->p[kk] <= MAX_NICE)
				{
					if (b[ii+kk][jj-kk] <= MAX_NICE)
					{
						match++;
						continue;
					}
					else
						break;
				}
				if (ptr->p[kk] == MAN)
				{
					if (b[ii+kk][jj-kk] == MAN)
					{
						match++;
						continue;
					}
					else
						break;
				}
				if (ptr->p[kk] == COM)
				{
					if (b[ii+kk][jj-kk] == COM)
					{
						match++;
						continue;
					}
					else
						break;
				}
				else
					break;
			}
			if (pcnt == match)
			{
				for (kk = 0; kk < pcnt; kk++)
				{
					if (ptr->p[kk] != 0 && ptr->p[kk] <= MAX_NICE)
						ai_nice(b, ii+kk, jj-kk, ptr->p[kk] * rate);
				}
			}
		}
	}		

	for (ii = 0; ii < MAX_ROW; ii++)	
	{
		for (jj = MAX_ROW-1; 0 <= jj; jj--)
		{
			match = 0;
			for (kk = 0; kk < pcnt; kk++)
			{		
				if (ptr->flag != mode)
					break;		
				if (ii + kk < 0 || ii + kk >= MAX_ROW)
					break;
				if (jj + kk < 0 || jj + kk >= MAX_ROW)
					break;
				if (ptr->p[kk] != 0 && ptr->p[kk] <= MAX_NICE)
				{
					if (b[ii+kk][jj+kk] <= MAX_NICE)
					{
						match++;
						continue;
					}
					else
						break;
				}
				if (ptr->p[kk] == MAN)
				{
					if (b[ii+kk][jj+kk] == MAN)
					{
						match++;
						continue;
					}
					else
						break;
				}
				if (ptr->p[kk] == COM)
				{
					if (b[ii+kk][jj+kk] == COM)
					{
						match++;
						continue;
					}
					else
						break;
				}
				else
					break;
			}
			if (pcnt == match)
			{
				for (kk = 0; kk < pcnt; kk++)
				{
					if (ptr->p[kk] != 0 && ptr->p[kk] <= MAX_NICE)
						ai_nice(b, ii+kk, jj+kk, ptr->p[kk] * rate);
				}
			}
		}
	}
}

/***********************************************************************
 * ai_nice()
 **********************************************************************/
int ai_nice(char (*b)[MAX_ROW], int ii, int jj, int val)
{
	if (ii >= MAX_ROW || jj >= MAX_ROW)
		return(-1);
		
	if ((b[ii][jj] + val) > MAX_NICE)
		b[ii][jj] = MAX_NICE;
	else
		b[ii][jj] += val;
	
	if (b[ii][jj] < 0)
		b[ii][jj] = 0;

	return(0);
}

/***********************************************************************
 * ai_maxnice()
 **********************************************************************/
int ai_maxnice(char b[][MAX_ROW])
{
	int 	ii, jj;
	char 	max=0;
	char	val;
		    
	for (ii = 0; ii < MAX_ROW; ii++) 
	{
		for (jj = 0; jj < MAX_ROW; jj++) 
		{				
			val = b[ii][jj];
			if (val > 0 && val <= MAX_NICE)
			{
				if (val > max)
					max = b[ii][jj];				
			}
		}
	}
	
	return(max);
}

/***********************************************************************
 * ai_determine()
 **********************************************************************/
void ai_determine(char b[][MAX_ROW], pair *acur)
{
	int 	ii, jj;
	char 	max=0;
	char	val;
	pair	maxtbl[MAX_ROW * MAX_ROW];
	int		nrec=0;
	int		xval, yval, area=0;
	    
	memset(maxtbl, 0x00, sizeof(maxtbl));
	
	for (ii = 0; ii < MAX_ROW; ii++) 
	{
		for (jj = 0; jj < MAX_ROW; jj++) 
		{				
			val = b[ii][jj];
			if (val > 0 && val <= MAX_NICE)
			{
				if (val > max)
					max = b[ii][jj];				
			}
		}
	}
	
	for (ii = 0; ii < MAX_ROW; ii++) 
	{
		for (jj = 0; jj < MAX_ROW; jj++) 
		{				
			val = b[ii][jj];
			if (val == max)
			{
				maxtbl[nrec].x = jj;
				maxtbl[nrec].y = ii;
				nrec++;				
			}
		}
	}
	
	for (ii = 0; ii < nrec; ii++)
	{
		if (maxtbl[ii].x <= MAX_ROW - maxtbl[ii].x)
			xval = maxtbl[ii].x;
		else
			xval = MAX_ROW - maxtbl[ii].x;
			
		if (maxtbl[ii].y <= MAX_ROW - maxtbl[ii].y)
			yval = maxtbl[ii].y;
		else
			yval = MAX_ROW - maxtbl[ii].y;
			
		if (area < xval * yval)
		{
			area = xval * yval;
			acur->y = maxtbl[ii].y;
			acur->x = maxtbl[ii].x;
		} 
	}
} 

/***********************************************************************
 * ai_pcount()
 **********************************************************************/
int	ai_pcount(struct pattern *ptr)
{
	int ii, rtn;
	
	rtn = 0;
	for (ii = 0; ii < MAX_PCNT; ii++)
	{
		if (ptr->p[ii] != 0)
		{
			rtn++;
			continue;
		}
		break;
	}
	return(rtn);
}

#if 0
/***********************************************************************
 * searchMax()
 **********************************************************************/
void searchMax(int point[2][MAX_ROW][MAX_ROW],pair *coor)
{
        int i, j;
        int val[MAX_ROW][MAX_ROW];
        int max=0;
        
        srand((unsigned)time(NULL));         
        for(i=0;i<MAX_ROW;i++) {
                for(j=0;j<MAX_ROW;j++) {
                        val[i][j]=point[0][i][j]+point[1][i][j]; 
                        if((max < val[i][j]) || (max == val[i][j] && (rand()%3 != 0))) {
                                max = val[i][j];
                                coor->y=i; coor->x=j;
                        }
                }
        }
} 
#endif

/***********************************************************************
 * ai_isend()
 **********************************************************************/
int ai_isend(char board[][MAX_ROW], char who)
{
	int		ii, jj, kk;
	int		drec;
	char	kind;
	int		idx_i, idx_j, idx_start;
	int		count;
	
	for (drec = 0; drec < 4; drec++) 
	{
		if (drec == 0) { idx_i = MAX_ROW;   idx_j = MAX_ROW-4; idx_start = 0; } 
		if (drec == 1) { idx_i = MAX_ROW-4; idx_j = MAX_ROW;   idx_start = 0; } 
		if (drec == 2) { idx_i = MAX_ROW-4; idx_j = MAX_ROW-4; idx_start = 0; } 
		if (drec == 3) { idx_i = MAX_ROW;   idx_j = MAX_ROW-4; idx_start = 4; } 
		
		for (ii = idx_start; ii < idx_i; ii++) 
		{ 
			for (jj = 0; jj < idx_j; jj++) 
			{ 
				if (board[ii][jj] == who) 
				{ 
					count = 0; 
					for (kk = 0; kk < 6; kk++) 
					{                                                
						if (drec == 0) 
						{ 
							if (jj + kk > MAX_ROW - 1) 
								break; 
							kind = board[ii][jj+kk]; 
						}
						if (drec == 1) 
						{ 
							if (ii + kk > MAX_ROW - 1) 
								break; 
							kind = board[ii+kk][jj]; 
						}
						if (drec == 2) 
						{ 
							if ((jj + kk > MAX_ROW - 1) || (ii + kk > MAX_ROW - 1)) 
								break; 
							kind = board[ii+kk][jj+kk]; 
						}
						if (drec == 3) 
						{ 
							if ((ii - kk < 0) || (jj + kk > MAX_ROW - 1)) 
								break; 
							kind = board[ii-kk][jj+kk]; 
						}
						
						if (kind == who) 
							count++;
						else 
							break; 
						if (count == 5) 
							return(1);
					}
				}
			}
		}
	}
	return(0); 
}

