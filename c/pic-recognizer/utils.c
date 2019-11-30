
#include <math.h>


#ifndef Malloc
#define Malloc(type,n)  (type *) malloc((n) * sizeof(type))
#endif

#define max(a,b) \
 ({ __typeof__ (a) _a = (a); \
     __typeof__ (b) _b = (b); \
   _a > _b ? _a : _b; })

#ifndef DOT
#define DOT 1
#endif

#ifndef BLANK
#define BLANK 0
#endif

typedef unsigned char byte;

typedef struct Point{
  int x;
  int y;
  byte c; // color
} Point;

typedef struct Range{
  Point from;
  Point to;
} Range;

typedef struct Context{
  int width;
  int height;
  void *data;
} Context;

int get_range_w(Range r){
  return abs(r.to.x - r.from.x);
}

typedef void (*RowProcessor)(Point *row, int length, Point * const coordinate, Context *ctx); // callback

void process_by_row(Point * const coordinate, int const width, int const height, Context *ctx, RowProcessor proc){
  Point *coordRow = Malloc(Point, width);
  for (int row = 0; row < height; row++){
    for (int col = 0; col < width; col++){
      *(coordRow + col) = *(coordinate+(row*width + col));
    }
    proc(coordRow, width, coordinate, ctx);
  }
  free(coordRow);
}

void print_2D_array(Point const *coordinate, int const width, int const height){
  Point currP;
  for (int row = 0; row < height; row++){
    for (int col = 0; col < width; col++){
      currP = *(coordinate + (row * width + col));
      printf("%d", currP.c);
    }
    printf("\n");
  }
}

void print_range(Range r, char * const separator){
  printf("(%d,%d -> %d,%d)%s", r.from.x, r.from.y, r.to.x, r.to.y, separator);
}

void print_point(Point p){
  printf("(%d,%d) %d\n", p.x, p.y, p.c);
}

void print_hex(int v){
  printf("0x%06x\n", v);
}

int to_hex(byte const r, byte const g, byte const b){
  return r << 16 | g << 8 | b ;
}

int get_gray(byte const r, byte const g, byte const b){
  /*
    24位色中3个字节分别用来描述R,G,B三种颜色分量，我们看到这其中是没有亮度分量的，这是因为在RGB表示方式中，亮度也可以直接从颜色分量中得到
    每一颜色分量值的范围都是从0到255, 某一颜色分量的值越大，就表示这一分量的亮度值越高，所以255表示最亮，0表示最暗。
    那么一个真彩色像素点转换为灰度图时它的亮度值应该是多少呢，首先我们想到的平均值，即将R+G+B/3。但现实中我们使用的却是如下的公式：
      Y = 0.299R+0.587G+0.114B
    这个公式通常都被称为心理学灰度公式。这里面我们看到绿色分量所占比重最大。
    因为科学家发现使用上述公式进行转换时所得到的灰度图最接近人眼对灰度图的感觉。
   */
  return 0.299 * r + 0.587 * g + 0.114 * b;
}
