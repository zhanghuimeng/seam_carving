// ʵ��˫�����������GUI
// �ο���http://mp.weixin.qq.com/s/L0R199OTwU7YJMycrfHBgQ

#pragma once

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <vector>
#include <cstdlib>

using namespace cv;
using namespace std;

typedef cv::Vec3b Color;
typedef cv::Mat_<Color> Image;

struct MouseArgs // ���ںͻص���������������ΪʲôҪ������һ��struct����ὲ~
{
	Image &img; // ��ʾ�ڴ����е�����ͼ
	std::vector<std::vector<int>> &mask; // �û����Ƶ�ѡ����ɾ��/������
	Color color; // ��������ѡ������ɫ
	MouseArgs(Image &img, std::vector<std::vector<int>> &mask, const Color color)
		: img(img), mask(mask), color(color) {}
};

void onMouse(int event, int x, int y, int flags, void *param)
{
  	// C++û������Java�ĵ����̳л��ƣ�Ϊ��֧�ֶ����͵Ľ�����������ֻ�ܴ���void *��ǿ��ת��
  	// Ϊʲô���붨��һ��MouseArgs�ṹ�壺��Ȼû��ͬʱ���ص���������������
	MouseArgs *args = (MouseArgs *)param;

  	// �����������϶�ʱ
	if ((event == CV_EVENT_MOUSEMOVE || event == CV_EVENT_LBUTTONDOWN) && (flags & CV_EVENT_FLAG_LBUTTON))
	{
		int brushRadius = 10;	// ��ˢ�뾶			
		int rows = args->img.rows, cols = args->img.cols;

      	// ���µ�˫��forѭ���������ǰ뾶Ϊ10��Բ������ʵ�ֱ�ˢЧ��
      	// ע�⴫�ظ��ص�������x, y�ǡ��������꡿�µģ�����y���У�x����
		for (int i = std::max(0, y - brushRadius); i < std::min(rows, y + brushRadius); i++)
		{
			int halfChord = (int)sqrt(pow(brushRadius, 2) - pow(i - y, 2)); // ���ҳ�
			for (int j = std::max(0, x - halfChord); j < std::min(cols, x + halfChord); j++)
				if (args->mask[i][j] == 0)
				{
                    // ������һ��
					args->img(i, j) = args->img(i, j) * 0.7 + args->color * 0.3;
                    // ����һ����ӵ�ѡ��
					args->mask[i][j] = 1;
				}
		}
	}
}

vector<vector<int>> getMaskRemove(Image& showImg, int rows, int cols)
{
	cv::namedWindow("Draw remove area", CV_WINDOW_AUTOSIZE); // �½�һ������
	vector<vector<int>> maskRemove(rows, vector<int>(cols, 0)); // ϣ����ȡ�Ĵ�ɾ��ѡ��
	MouseArgs *args = new MouseArgs(showImg, maskRemove, Color(0, 0, 255)); // ��һ��MouseArgs�ṹ�����ڽ���
	cv::setMouseCallback("Draw remove area", onMouse, (void*)args); // ���������ûص�����

	// �϶��������
	while (1)
	{
		cv::imshow("Draw remove area", args->img);
		// �� esc ���˳���ͼģʽ�����ѡ��
		if (cv::waitKey(100) == 27)
			break;
	}
	cv::setMouseCallback("Draw remove area", NULL, NULL); // ȡ���ص�����
	delete args; args = NULL; // ��������

	// imwrite("C:/Users/lenovo/Documents/m_�����ͼ��ѧ/Homework 3/Output/masked.png", showImg);
	return maskRemove;
}

vector<vector<int>> getMaskProtect(Image& showImg, int rows, int cols)
{
	cv::namedWindow("Draw protect area", CV_WINDOW_AUTOSIZE); // �½�һ������
	vector<vector<int>> maskRemove(rows, vector<int>(cols, 0)); // ϣ����ȡ�Ĵ�ɾ��ѡ��
	MouseArgs *args = new MouseArgs(showImg, maskRemove, Color(0, 255, 0)); // ��һ��MouseArgs�ṹ�����ڽ���
	cv::setMouseCallback("Draw protect area", onMouse, (void*)args); // ���������ûص�����

	// �϶��������
	while (1)
	{
		cv::imshow("Draw protect area", args->img);
		// �� esc ���˳���ͼģʽ�����ѡ��
		if (cv::waitKey(100) == 27)
			break;
	}
	cv::setMouseCallback("Draw protect area", NULL, NULL); // ȡ���ص�����
	delete args; args = NULL; // ��������

	// imwrite("C:/Users/lenovo/Documents/m_�����ͼ��ѧ/Homework 3/Output/masked.png", showImg);
	return maskRemove;
}