#ifndef __MAIN_H__
#define __MAIN_H__

#define RED     0xFF0000FF
#define GREEN   0xFF00FF00
#define BLUE    0xFFFF0000
#define CYAN    0xFFFFFF00
#define MAGENTA 0xFFFF00FF
#define YELLOW  0xFF00FFFF
#define AZURE   0xFFFF7F00
#define VIOLET  0xFFFF007F
#define ORANGE	0xFF007FFF
#define GREY    0xFF808080
#define WHITE   0xFFFFFFFF
#define BLACK   0xFF000000

char *warning(int code, char *msg);
char *error(int code, char *msg);

#endif