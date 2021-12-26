/******************************************************************************/
/*  Components  : string.c						      */
/*  Description	: string 변환처리                                             */
/*  Rev. History: Ver	Date	Description				      */
/*		  ----	-------	----------------------------------------------*/
/*		  1.0	99-01	Initial version				      */
/******************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <pwd.h>

/*****************************************************************************
 * l_strncpy()	: n bytes string to string
 * l_trim()	: String을 Space 없는 String으로 변환
 * l_ltrim()	: String을 왼쪽에 Space 없는 String으로 변환
 * l_rtrim()	: String을 오른쪽에 Space 없는 String으로 변환
 * l_hnamcpy()	: String중 마지막 한글이 첫번째 BYTE만 copy되는 것을 방지
 *****************************************************************************/

/*****************************************************************************/
/* NAME	: l_strncpy()							     */
/* DESC	: n bytes string to string					     */
/*****************************************************************************/
int	l_strncpy(xs, xl, ys, yl)
char	*xs;
int	xl;
char	*ys;
int	yl;
{
	int	ii, jj;

	memset(xs, 0, xl);
	for (ii = 0, jj = 0; ii < yl && jj < xl; ii++)
	{
		if (!(ys[ii] & 0x80) && ys[ii] <= ' ')
		{
			if (jj != 0)
				break;
			continue;
		}
		xs[jj++] = ys[ii];
	}
	return(jj);
}

/*****************************************************************************/
/* NAME	; l_trim()							     */
/* DESC	: String을 Space 없는 String으로 변환				     */
/*****************************************************************************/
int	l_trim(instr)
char	*instr;
{
	int	len;
	int	ii, jj;
	
	len = strlen(instr);

	for (ii=0, jj=0; ii<len; ii++)
	{
		if (instr[ii] == ' ')
			continue;
		else
			instr[jj++] = instr[ii];
	}
	instr[jj] = 0x00;

	return(jj);
}
/*****************************************************************************/
/* NAME	; l_ltrim()							     */
/* DESC	: String을 왼쪽에 Space 없는 String으로 변환			     */
/*****************************************************************************/
void	l_ltrim(instr)
char	*instr;
{
        int     len;
        int     ii, jj;

        len = strlen(instr);
        for (ii = 0; ii < len; ii++)
        {
                if (instr[ii] != ' ')
                        break;
        }

        for (jj = ii; jj < len; jj++)
        {
                instr[jj - ii] = instr[jj];
        }
        instr[jj - ii] = 0x00;

}
/*****************************************************************************/
/* NAME	; l_rtrim()							     */
/* DESC	: String을 오른쪽에 Space 없는 String으로 변환			     */
/*****************************************************************************/
int	l_rtrim(instr)
char	 *instr;
{
        int     len;
        int     ii;

        len = strlen(instr);
        for(ii = len-1; ii >= 0; ii--)
        {
                if (instr[ii] != ' ')
                        break;
		instr[ii] = 0x00;
        }
	return(strlen(instr));
}

/*****************************************************************************/
/* NAME	; l_hnamcpy()							     */
/* DESC	: String중 마지막 한글이 첫번째 BYTE만 copy되는 것을 방지	     */
/*****************************************************************************/
int	l_hnamcpy(dst, src, len)
char	*dst, *src;
int	len;
{
	register int	ii;
	int	hf;

	for (ii = 0, hf = 0; ii < len; ii++)
	{
		if (src[ii] & 0x80)
			hf ^= 1;
		dst[ii] = src[ii];
	}
	if (hf)
		dst[ii - 1] = ' ';
	return(0);
}

