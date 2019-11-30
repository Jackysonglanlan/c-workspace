
// 识别的技术关键点:
// 1. 印刷 答题区 的时候，框框的线必须要颜色深，并且粗 (用于定位 答题区 和 每道题的选择区)
// 2. 印刷 选项 的时候，务必要用浅色，和 答题区框框线的对比度 差得越大越好
// 3. 拍照一般都不可能做到完全水平拍摄答题区，所以需要靠算法做 透视纠偏，识别才准确

#include <math.h>
#include <stdio.h>

#include "../libs/bmpread/bmpread.c"
#include "./utils.c"

///////

typedef struct ColorSeqUnit {
  int count;   // 连续的颜色点个数
  Point endP;  // 结束这种连续 pattern 的点
} ColorSeqUnit;

// 统计一行里面 连续的颜色组成的颜色段(反应到图片上就是线段和空白) 的数量
// return length of ColorSeqUnit*
int _get_bw_continuous_seq_of_row(Point *row, int length, ColorSeqUnit *continuousDotSeq) {
  Point currP;
  Point lastP = *(row + 0);

  int seqIndex = 0;

  ColorSeqUnit currUnit;
  currUnit.endP                  = lastP;
  currUnit.count                 = 1;
  *(continuousDotSeq + seqIndex) = currUnit;

  for (int col = 1; col < length; col++) {
    currP         = *(row + col);
    currUnit.endP = currP;
    if (currP.c == lastP.c) {
      currUnit.count += 1;
      *(continuousDotSeq + seqIndex) = currUnit;
    }
    // 颜色切换，可能是扫描到线了
    else {
      seqIndex++;  // 新增一个 unit
      currUnit.count                 = 1;
      *(continuousDotSeq + seqIndex) = currUnit;
    }
    lastP = currP;
  }
  return seqIndex;
}

// 判断给定的一条线里面(这条线以及被切成一段一段的线段了)，够不够那么多条线段组成 答题区的竖线
_Bool _is_col_line_row(ColorSeqUnit *continuousDotSeq, int seqLen, int shouldHaveColumnLineCount) {
  int visibleDotSeqCount = 0;  // 可见线段个数
  for (int i = 0; i <= seqLen; i++) {
    ColorSeqUnit u = *(continuousDotSeq + i);
    if (u.endP.c == DOT) {  // 颜色为DOT，代表一段可见的线段
      visibleDotSeqCount++;
    }
  }
  return shouldHaveColumnLineCount == visibleDotSeqCount;
}

//
void _locate_col_line_coord_range(ColorSeqUnit *continuousDotSeq, int seqLen, void *data) {
  Range range;
  ColorSeqUnit u1;
  ColorSeqUnit u2;
  int rangeCount    = 0;
  static int offset = 0;
  for (int i = 0; i <= seqLen; i += 2) {
    u1         = *(continuousDotSeq + i);
    u2         = *(continuousDotSeq + i + 1);
    range.from = u1.endP;
    range.to   = u2.endP;

    if (range.to.x == 0 && range.to.y == 0) {  // to 未赋值，无效 (for 步进为2)
      continue;
    }

    if (range.from.x > range.to.x) {  // 无效
      continue;
    }

    *((Range *)data + offset + rangeCount) = range;
    rangeCount++;
    // printf("(%d,%d -> %d,%d) ", range.from.x, range.from.y, range.to.x, range.to.y);
  }
  // printf("\n");
  offset += rangeCount;
}

// @param areaDotCountArray Range数组，每个 range 代表一个答题区, from: 左上角点 to: 右上角点
void _try_to_get_answer_area_range(Context *colLineRange2DArray, Range *areaDotRangeArray) {
  int verticalContinuousCount = 1;  // 看有多少线段的 Y 连续相同
                                    // Y 相同代表找到一段线条: 足够竖直又足够粗，很可能就是答题区的框
  int verticalContinuousCountMax = 0;

  Range topLeftP, topRightP;

  Point *areaTopPArr   = Malloc(Point, colLineRange2DArray->width);
  int areaTopPArrCount = 0;

  // 按列遍历 2D 数组
  Range top;
  int width  = colLineRange2DArray->width;
  int height = colLineRange2DArray->height;
  for (int col = 0; col < width; col++) {
    top = *((Range *)colLineRange2DArray->data + col);
    // printf("(%d,%d -> %d,%d) ", top.from.x, top.from.y, top.to.x, top.to.y);

    // 首先，每一列的线段都要足够粗(这里检测如果粗于1px，则更有可能是 vertical-bar，这样可以增加识别准确率)
    _Bool boldEnough = (top.to.x - top.from.x) > 2;  // 这个值也不能太大，否则对照片的要求会提高
    if (!boldEnough) {
      continue;
    }

    // 扫下面的线段
    for (int row = 1; row < height; row++) {
      Range r = *((Range *)colLineRange2DArray->data + ((row * width + col)));
      // printf("(%d,%d -> %d,%d) ", r.from.x, r.from.y, r.to.x, r.to.y);

      // x相同，长度相同，且 y 连续 (如果要增加准确率，这个条件要放宽)
      if (r.from.x == top.from.x && get_range_w(r) == get_range_w(top) && r.from.y - top.from.y == row) {
        verticalContinuousCount++;
      } else {
        verticalContinuousCountMax = max(verticalContinuousCountMax, verticalContinuousCount);
        verticalContinuousCount    = 1;
      }
    }

    if (verticalContinuousCountMax >= 5) {  // 经验值，连续 5段 像素都是竖直排列，很可能是答题区的框
      *(areaTopPArr + areaTopPArrCount) = top.from;
      areaTopPArrCount++;
    }

    // printf("\n");
  }

  int areaDotRangeCount = 0;  //
  Range areaTopXRange;
  Point prevAreaTopP = *(areaTopPArr + 0);
  Point currAreaTopP;
  for (int i = 1; i < areaTopPArrCount; i++) {
    currAreaTopP = *(areaTopPArr + i);

    areaTopXRange.from                       = prevAreaTopP;
    areaTopXRange.to                         = currAreaTopP;
    *(areaDotRangeArray + areaDotRangeCount) = areaTopXRange;

    areaDotRangeCount++;
    prevAreaTopP = currAreaTopP;
  }

  free(areaTopPArr);
  areaTopPArr = NULL;
}

void rowProcessor(Point *row, int length, Point *const coordinate, Context *ctx) {
  // 每次颜色切换，生成一个元素，元素值代表连续点的个数, 最多 length/2 + 1 个
  ColorSeqUnit *continuousDotSeq = Malloc(ColorSeqUnit, length / 2 + 1);

  // 这种连续的点有多少段
  int seqLen = _get_bw_continuous_seq_of_row(row, length, continuousDotSeq);

  // 定位组成 答题区 竖线的 continuousDot
  int colLineCount = ctx->width;
  if (_is_col_line_row(continuousDotSeq, seqLen, colLineCount)) {
    _locate_col_line_coord_range(continuousDotSeq, seqLen, ctx->data);
    ctx->height += 1;
  }

  free(continuousDotSeq);
  continuousDotSeq = NULL;
}

Point *const _make_standard_coordinate(byte const *arr, int const width, int const height, Point *const coordinate) {
  int const realLenPerLine = width * 3;  // 3字节代表一个点，所以实际的每行的实际数据长度为 width*3
  int pointDataStartPos;
  byte r, g, b;
  byte color;  // value is DOT/BLANK

  int xInCo;  // 最终处理后的点在 标准坐标系 中的x
  Point currP;

  for (int row = 0; row < height; row++) {
    for (int col = 0; col < realLenPerLine; col += 3) {  // 每次读完一个完整的点
      // 点的数据起始位置, 从这个点开始，连续3个字节就是这个点的RGB
      pointDataStartPos = row * realLenPerLine + col;
      r                 = *(arr + pointDataStartPos + 0);
      g                 = *(arr + pointDataStartPos + 1);
      b                 = *(arr + pointDataStartPos + 2);

      // color = get_gray(r,g,b) > 230 ? 0 : 1;

      // test31.bmp 降噪以后 (仅保留 浅色印刷的ABCD 和 用户用笔画上去的笔划)
      // color = to_hex(r,g,b) > 0xcccccc ? BLANK : DOT;

      // test31.bmp 淡化以后 (过滤了 浅色印刷的ABCD) 理论上应该只剩: 用户用笔画上去的笔划, 和框框的线
      // 这时，剩下的只是确定这个 2D 数组的那些坐标点组成了 选项框，然后一个一个的去数哪个框里面的 1
      // 最多，就知道他划的是哪个选项了
      color = to_hex(r, g, b) > 0xaaaaaa ? BLANK : DOT;

      xInCo                               = col / 3;
      currP.x                             = xInCo;
      currP.y                             = row;
      currP.c                             = color;
      *(coordinate + row * width + xInCo) = currP;  // 通过指针来写，即用 1维数组 表达2维
    }
  }
  return coordinate;
}

// 统计给定 答题区 的 '1' 的个数，从上到下扫描，直到区域底部
int _count_dot_start_with_range(Range r, Point *const coordinate, int const coorWidth, int const coorHeight) {
  printf("\n ----- scan area -----: ");
  print_range(r, "\n");

  int totalDot = 0;

  Point p;

  // 下面2个变量，判断所扫描的整个一行都是'1'，这种情况代表扫描到了框的底部，所以应该停止扫描了
  int areaWidth            = 0;
  int totalDotCountPerLine = 0;

  for (int row = r.from.y; row < coorHeight; row++) {
    for (int col = r.from.x; col < r.to.x; col++) {  // 从上到下扫描
      p = *(coordinate + (row * coorWidth + col));
      printf("%d", p.c);
      if (p.c == DOT) {
        totalDotCountPerLine++;
      }
    }

    areaWidth = r.to.x - r.from.x;

    // 是否遇到整个一行都是 '1' (框的底部横线)

    // 为了识别准确率，这里的条件应该放宽，经验值 95%, 即 95% 是点，则整个这一行很可能就是
    if (totalDotCountPerLine > (int)(areaWidth * 0.95)) {
      break;  // 不用扫了
    }

    // 无适用价值，只是表达一种思路
    // // 如果超过 80%，则有可能这个不是用户画的，因为太过于笔直，很可能是印刷的线
    // if (totalDotCountPerLine > (int)(areaWidth * 0.7)){
    //   continue; // 不记入统计结果
    // }

    totalDot += totalDotCountPerLine;
    totalDotCountPerLine = 0;

    printf(" - %d\n", totalDot);
  }

  return totalDot;
}

// 扫描答题区，看哪个区的 '1' 最多，即是用户做了标记的区域
// return 答题区索引
int _scan_to_count_area_with_most_dot(Range *answerAreaRangeArr, int answerAreaCount, Point *const coordinate,
                                      int const coorWidth, int const coorHeight) {
  int markedAreaIndex         = 0;
  Range r                     = *(answerAreaRangeArr + 0);
  int totalDotCountOfArea     = _count_dot_start_with_range(r, coordinate, coorWidth, coorHeight);
  int currTotalDotCountOfArea = 0;

  for (int i = 1; i < answerAreaCount; ++i) {
    r                       = *(answerAreaRangeArr + i);
    currTotalDotCountOfArea = _count_dot_start_with_range(r, coordinate, coorWidth, coorHeight);
    // 冒泡找最大
    if(totalDotCountOfArea < currTotalDotCountOfArea){
      totalDotCountOfArea = currTotalDotCountOfArea;
      markedAreaIndex = i;
    };
  }
  return markedAreaIndex;
}

void _print_result(int areaIndex){
  printf("\n-------------\n");
  printf("识别结果: %c \n", areaIndex + 65);
  printf("-------------\n");
}

int main(int argc, char * argv[]){
  bmpread_t bmp;

  // const char * path = argv[2];
  const char * path = "./333.bmp";
  // const char * path = "./666.bmp";
  // printf("Loading %s...\n", path);

  if(!bmpread(path, BMPREAD_TOP_DOWN | BMPREAD_ANY_SIZE | BMPREAD_BYTE_ALIGN, &bmp)){
      puts("error!");
      return 1;
  }

  int width = bmp.width;
  int height = bmp.height;

  Point * const coordinate = Malloc(Point, width * height);
  _make_standard_coordinate(bmp.rgb_data, width, height, coordinate);
  // print_2D_array(coordinate, bmp.width, bmp.height);

  Context colLineRange2DArray;
  colLineRange2DArray.width = 5; // 由于答题框是4个，所以有5条竖线，即: 一条线里面，包含有5段连续的点
  colLineRange2DArray.height = 0; // 一共有多少条这种 pattern 的线: 这个值由 process_by_row 扫描后得到

  int allocMemCount = colLineRange2DArray.width * (height * 2/3); // 经验值，这种线应该不会超过总数的 2/3
  colLineRange2DArray.data = Malloc(Range, allocMemCount);

  process_by_row(coordinate, width, height, &colLineRange2DArray, rowProcessor);

  int answerAreaCount = 4;
  Range *answerAreaRangeArr = Malloc(Range, answerAreaCount);
  _try_to_get_answer_area_range(&colLineRange2DArray, answerAreaRangeArr);

  // 到这里，已经拿到了每个 答题区 的最上面那条线段的位置

  int markedAreaIndex = _scan_to_count_area_with_most_dot(answerAreaRangeArr, answerAreaCount,
                                                          coordinate, width, height);

  // 已经识别出来了
  _print_result(markedAreaIndex);

  free(answerAreaRangeArr);
  free(colLineRange2DArray.data);
  free(coordinate);
}


