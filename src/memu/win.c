/* win.c - Code that is common to all implementations of a window */

#include "win.h"
#include "common.h"

int n_wins = 0; 
WIN *wins[MAX_WINS];
WIN *active_win = NULL;

WIN *win_alloc (size_t win_size, size_t data_size)
    {
    if ( n_wins == MAX_WINS ) fatal ("Too many windows");
    WIN *win = (WIN *) malloc (win_size);
    // printf ("win_alloc: win = %p\n", win);
    if ( win == NULL ) fatal ("Unable to allocate memory for window");
    win->iWin = n_wins;
    if ( data_size > 0 )
        {
        win->data = (byte *) malloc (data_size);
        // printf ("win_alloc: data = %p\n", win->data);
        if (win->data == NULL) fatal ("Unable to allocate memory for pixel map");
        memset (win->data, 0, data_size);
        }
    else
        {
        win->data = NULL;
        }
    win->tbuf = NULL;
    wins[n_wins] = win;
    ++n_wins;
    return win;
    }

void win_free (WIN *win)
    {
    int iWin = win->iWin;
    // printf ("win_free: data = %p\n", win->data);
    if ( win->data != NULL ) free (win->data);
    // printf ("win_free: tbuf = %p\n", win->tbuf);
    if ( win->tbuf != NULL ) free (win->tbuf);
    wins[iWin] = wins[n_wins - 1];
    wins[iWin]->iWin = iWin;
    // printf ("win_free: win = %p\n", win);
    free (win);
    --n_wins;
    if ( n_wins > 0 )
        {
        if ( win == active_win ) win_show (wins[n_wins - 1]);
        }
    else
        {
        win_term ();
        }
    }

BOOLEAN win_active (WIN *win)
    {
    return ( win == active_win );
    }

WIN * win_current (void)
    {
    return active_win;
    }

void win_show_num (int iWin)
    {
    if ((iWin >= 0) && (iWin < n_wins)) win_show (wins[iWin]);
    }

void win_prev (void)
    {
    int iWin = active_win->iWin - 1;
    if ( iWin < 0 ) iWin = n_wins - 1;
    win_show (wins[iWin]);
    }

void win_next (void)
    {
    int iWin = active_win->iWin + 1;
    if ( iWin >= n_wins ) iWin = 0;
    win_show (wins[iWin]);
    }
