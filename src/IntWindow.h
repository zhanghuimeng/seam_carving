// 实现双向缩放所需的GUI
// 参考自http://mp.weixin.qq.com/s/L0R199OTwU7YJMycrfHBgQ

#pragma once

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <vector>
#include <cstdlib>

using namespace cv;
using namespace std;

typedef cv::Vec3b Color;
typedef cv::Mat_<Color> Image;

struct MouseArgs // 用于和回调函数交互，至于为什么要特意攒一个struct后面会讲~
{
	Image &img; // 显示在窗口中的那张图
	std::vector<std::vector<int>> &mask; // 用户绘制的选区（删除/保留）
	Color color; // 用来高亮选区的颜色
	MouseArgs(Image &img, std::vector<std::vector<int>> &mask, const Color color)
		: img(img), mask(mask), color(color) {}
};

void onMouse(int event, int x, int y, int flags, void *param)
{
  	// C++没有类似Java的单根继承机制，为了支持多类型的交互数据这里只能传入void *再强制转换
  	// 为什么必须定义一个MouseArgs结构体：不然没法同时给回调函数传入多个数据
	MouseArgs *args = (MouseArgs *)param;

  	// 按下鼠标左键拖动时
	if ((event == CV_EVENT_MOUSEMOVE || event == CV_EVENT_LBUTTONDOWN) && (flags & CV_EVENT_FLAG_LBUTTON))
	{
		int brushRadius = 10;	// 笔刷半径			
		int rows = args->img.rows, cols = args->img.cols;

      	// 以下的双重for循环遍历的是半径为10的圆形区域，实现笔刷效果
      	// 注意传回给回调函数的x, y是【窗口坐标】下的，所以y是行，x是列
		for (int i = std::max(0, y - brushRadius); i < std::min(rows, y + brushRadius); i++)
		{
			int halfChord = (int)sqrt(pow(brushRadius, 2) - pow(i - y, 2)); // 半弦长
			for (int j = std::max(0, x - halfChord); j < std::min(cols, x + halfChord); j++)
				if (args->mask[i][j] == 0)
				{
                    // 高亮这一笔
					args->img(i, j) = args->img(i, j) * 0.7 + args->color * 0.3;
                    // 将这一笔添加到选区
					args->mask[i][j] = 1;
				}
		}
	}
}

vector<vector<int>> getMaskRemove(Image& showImg, int rows, int cols)
{
	cv::namedWindow("Draw remove area", CV_WINDOW_AUTOSIZE); // 新建一个窗口
	vector<vector<int>> maskRemove(rows, vector<int>(cols, 0)); // 希望获取的待删除选区
	MouseArgs *args = new MouseArgs(showImg, maskRemove, Color(0, 0, 255)); // 攒一个MouseArgs结构体用于交互
	cv::setMouseCallback("Draw remove area", onMouse, (void*)args); // 给窗口设置回调函数

	// 拖动鼠标作画
	while (1)
	{
		cv::imshow("Draw remove area", args->img);
		// 按 esc 键退出绘图模式，获得选区
		if (cv::waitKey(100) == 27)
			break;
	}
	cv::setMouseCallback("Draw remove area", NULL, NULL); // 取消回调函数
	delete args; args = NULL; // 垃圾回收

	// imwrite("C:/Users/lenovo/Documents/m_计算机图形学/Homework 3/Output/masked.png", showImg);
	return maskRemove;
}

vector<vector<int>> getMaskProtect(Image& showImg, int rows, int cols)
{
	cv::namedWindow("Draw protect area", CV_WINDOW_AUTOSIZE); // 新建一个窗口
	vector<vector<int>> maskRemove(rows, vector<int>(cols, 0)); // 希望获取的待删除选区
	MouseArgs *args = new MouseArgs(showImg, maskRemove, Color(0, 255, 0)); // 攒一个MouseArgs结构体用于交互
	cv::setMouseCallback("Draw protect area", onMouse, (void*)args); // 给窗口设置回调函数

	// 拖动鼠标作画
	while (1)
	{
		cv::imshow("Draw protect area", args->img);
		// 按 esc 键退出绘图模式，获得选区
		if (cv::waitKey(100) == 27)
			break;
	}
	cv::setMouseCallback("Draw protect area", NULL, NULL); // 取消回调函数
	delete args; args = NULL; // 垃圾回收

	// imwrite("C:/Users/lenovo/Documents/m_计算机图形学/Homework 3/Output/masked.png", showImg);
	return maskRemove;
}